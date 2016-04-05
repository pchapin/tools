/*****************************************************************************
FILE          : menu.c
LAST REVISION : May 1992
SUBJECT       : Menu managment program.

     (C) Copyright 1992 by Peter Chapin

This program reads a "menu definition file" and then presents the user with
a menu of options based on what it finds in the file. The user selects an
option by pressing a function key (there can at most be MENU_SIZE options
on a menu). Some of the options invoke other menus (at most N_MENUS can be
defined in the file) while other options are true commands. If a command is
selected it is executed by COMMAND.COM and thus can be an internal command,
an external command, a batch file or a normal program.

This file allows you to create two different variations of the program. One
variation supports larger menus than the other.

For more information about using this program and the format of the menu
definition file, consult the documentation in the file menu.doc.

The executable program and the accompaning user documentation is placed
into the public domain by me, the author. The source code is, however,
copyrighted. Please send any comments you may have about this program to
me. I can be reached as in

     Peter Chapin
     P.O. Box 317
     Randolph Center, VT 05061
     chapinp@vscnet.bitnet
*****************************************************************************/

/* Define 'LONG' below to get the long menu display format. */
/* #define LONG 1 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "local.h"
#include "scr.h"
#include "scrtools.h"
#include "get_key.h"

/*=================================*/
/*           Global Data           */
/*=================================*/

#ifdef LONG
unsigned _heaplen = 6144;      /* Reserve this amount for the heap.  */
unsigned _stklen  = 2048;      /* Reserve this amount for the stack. */
#else
unsigned _heaplen = 4096;      /* Reserve this amount for the heap.  */
unsigned _stklen  = 2048;      /* Reserve this amount for the stack. */
#endif

#define  LINE_BUF_SIZE 128     /* Holds line from menu file. */

#ifdef LONG
#define  N_MENUS        1      /* Total number of menus allowed. (<=10)      */
#define  MENU_SIZE      23     /* Nonscrollable list less header and footer. */
#else
#define  N_MENUS        10     /* Total number of menus allowed. (<=10) */
#define  MENU_SIZE      10     /* Maximum number of items allowed.      */
#endif

struct Menu {
  char *Title;                 /* Title bar that appears on top of menu. */
  int   N_Items;               /* Number of items in following array.    */
  char *Descrip[MENU_SIZE];    /* Descriptions that appear in menu.      */
  char *Action[MENU_SIZE];     /* Commands passed to COMMAND.COM.        */
  int   Color_Attrib;          /* Color attribute used to print text.    */
  };

struct Menu Menu_List[N_MENUS];

int Line_No = 0;               /* Line number of current line from file. */

/*==========================================*/
/*           Function Definitions           */
/*==========================================*/

/*------------------------------------------------------------------------
void Credits(void);

This function is registered with exit() using the atexit() function. This
insures that this all important information is displayed when the program
terminates for any reason.
------------------------------------------------------------------------*/

void Credits(void)
  {
#ifndef LONG
    printf("\nMENU (Version 2.5) May 1992\n"
             "Public Domain Software by Peter Chapin\n\n");
#endif
  }

/*------------------------------------------------------------------------
char *Chk_Strtok(char *S1, char *S2);
char *Chk_Fgets(char *Buffer, int Length, FILE *Stream);
char *Chk_Malloc(int N_Bytes);

These functions simplify the programming below. They perform the same basic
action as the related function from the standard library, but they include
some extra checking. If they detect an error, they terminate the program
via exit(). This program thus exit()'s in a total of four places.

Chk_strtok() terminates if there is no token to find. Chk_fgets()
terminates if the end of the file is reached. Chk_fgets() also removes the
'\n' character (if there is one) from the end of the line. Chk_malloc()
terminates if it runs out of heap space.
------------------------------------------------------------------------*/

char *Chk_Strtok(char *S1, char *S2)
  {
    char *Result;

    if ((Result=strtok(S1, S2)) == NULL) {
      fprintf(stderr, "\nError: Line number %d in the menu definition "
                      "file is bad.\n", Line_No);
      exit(1);
    }
    return Result;
  }

/*------------------------------------------------------------------------*/

char *Chk_Fgets(char *Buffer, int Length, FILE *Stream)
  {
    if (fgets(Buffer, Length, Stream) == NULL) {
      fprintf(stderr, "\nError: Unexpected <EOF> in menu definition file.\n");
      exit(1);
    }
    Line_No++;
    return strtok(Buffer, "\n");
  }

/*------------------------------------------------------------------------*/

char *Chk_Malloc(int N_Bytes)
  {
    char *Return_Value;

    if ((Return_Value=malloc(N_Bytes)) == NULL) {
      fprintf(stderr, "\nError: Can't allocate memory for menu definitions.\n");
      exit(1);
    }
    return Return_Value;
  }

/*------------------------------------------------------------------------
void Set_Menu_Color(struct Menu *Menu, char *Color_Def);

This function parses the string pointed at by Color_Def and composes the
color attribute for the struct Menu pointed at by menu. Note the following
things:

This function uses the nonstandard function strupr() to convert all the
characters in Color_Def to upper case.

The default color attribute is "black rev_black".

Unrecognized colors are ignored.

The order in which the colors appear in Color_Def is not important.
------------------------------------------------------------------------*/

void Set_Menu_Color(struct Menu *Menu, char *Color_Def)
  {
    auto   char  *Color;   /* Points at each color word in Color_Def. */
    auto   char **p;       /* Used to scan through array Color_Names. */
    auto   int    i;       /* Index into the Color arrays.            */
    auto   int    Found;   /* =YES if Color points at something.      */

    static char  *Color_Names[]={
      "BLACK", "BLUE",    "GREEN",  "CYAN",
      "RED",   "MAGENTA", "BROWN",  "WHITE",
      "REV_BLACK", "REV_BLUE",    "REV_GREEN",  "REV_CYAN",
      "REV_RED",   "REV_MAGENTA", "REV_BROWN",  "REV_WHITE",
      "BRIGHT",
      "BLINKING",
      NULL
      };
    static short Color_Values[]={
      SCR_BLACK, SCR_BLUE,    SCR_GREEN,  SCR_CYAN,
      SCR_RED,   SCR_MAGENTA, SCR_BROWN,  SCR_WHITE,
      SCR_REV_BLACK, SCR_REV_BLUE,    SCR_REV_GREEN,  SCR_REV_CYAN,
      SCR_REV_RED,   SCR_REV_MAGENTA, SCR_REV_BROWN,  SCR_REV_WHITE,
      SCR_BRIGHT,
      SCR_BLINK,
      0
      };

    Menu->Color_Attrib = 0;
    strupr(Color_Def);   /* Converts lower case letters to upper case. */
    Color = strtok(Color_Def, " +|");
    while (Color != NULL) {
      Found = NO;
      for (p = Color_Names, i = 0; *p != NULL  &&  !Found; p++, i++)
        if (strcmp(*p, Color) == 0) {
          Menu->Color_Attrib |= Color_Values[i];
          Found = YES;
        }
      Color = strtok(NULL, " +|");
    }
  }

/*------------------------------------------------------------------------
int Get_Menu_Def(FILE *infile, struct Menu *Menu)

This function reads one menu definition from the file pointed at by infile
(previously opened). Note that this function assumes that an entire menu
definition exists in the file (ending with a '#' as the first character on
a line). Chk_Fgets() will terminate the program with an error message if
this is not the case. Note also the extensive use of the Chk_... functions
in general to verify correctness of the menu definition file.

This function returns ERROR if there are too many options in the menu
definition. It does NOT advance the file pointer past the definition in
such a case, however.
------------------------------------------------------------------------*/

int Get_Menu_Def(FILE *infile, struct Menu *Menu)
  {
    char  Line_Buffer[LINE_BUF_SIZE+2];
    char *Pntr;
    char *p;
    int   Return_Value = NO_ERROR;
    int   Process_Flag = YES;

    /* Get the menu title. */
    Chk_Fgets(Line_Buffer, LINE_BUF_SIZE+2, infile);
    Menu->Title = Chk_Malloc(strlen(Line_Buffer)+1);
    strcpy(Menu->Title, Line_Buffer);

    /* Get the menu color. */
    Chk_Fgets(Line_Buffer, LINE_BUF_SIZE+2, infile);
    Set_Menu_Color(Menu, Line_Buffer);

    /* Get the menu items. */
    Menu->N_Items = 0;
    while ( Return_Value == NO_ERROR  &&
           *Chk_Fgets(Line_Buffer, LINE_BUF_SIZE+2, infile) != '#') {

      /* See if the first character on the line is a MENUTYPE conditional directive.*/
      if (Line_Buffer[0] == '=') {
        if (Line_Buffer[1] == '\0') {
          Process_Flag = YES;
          continue;
        }
        else if (strcmp(
          &Line_Buffer[1],
          (p = getenv("MENUTYPE")) == NULL ? "NULL" : p) == 0
        ) {
          Process_Flag = YES;
          continue;
        }
        else Process_Flag = NO;
      }

      /* See if the first character on the line is a MENUENV conditional directive.*/
      if (Line_Buffer[0] == '$') {
        if (Line_Buffer[1] == '\0') {
          Process_Flag = YES;
          continue;
        }
        else if (strcmp(
          &Line_Buffer[1],
          (p = getenv("MENUENV")) == NULL ? "NULL" : p) == 0
        ) {
          Process_Flag = YES;
          continue;
        }
        else Process_Flag = NO;
      }

      /* If we are not to process this line, skip it. */
      if (Process_Flag == NO) continue;

      if (Menu->N_Items == MENU_SIZE)    /* Trying to read too much? */
        Return_Value = ERROR;
      else {
        Pntr = Chk_Malloc(strlen(Line_Buffer)+1);  /* Break down string. */
        strcpy(Pntr, Line_Buffer);
        Pntr = Chk_Strtok(Pntr, "!");
        Menu->Descrip[Menu->N_Items] = Pntr;
        Pntr = Chk_Strtok(NULL, "");
        Menu->Action[Menu->N_Items] = Pntr;
        Menu->N_Items++;
      }

    }
    return Return_Value;
  }

/*------------------------------------------------------------------------
int Read_Menu_File(FILE *infile, struct Menu *Menu);

This function reads the entire menu definition file pointed at by infile
(previously opened). This function calls Get_Menu_Def() to read each menu
definition. It is primarly concerned with finding the end of the file.
After each menu definition is obtained, this function checks the next
character in the file looking for a '*'. If it does not find one, it puts
it back and uses Get_Menu_Def() to read the next menu (assumed to be
present).

This function returns ERROR if there are too many menu definitions in the
file or if Get_Menu_Def() returns ERROR.
------------------------------------------------------------------------*/

int Read_Menu_File(FILE *infile, struct Menu *Menu)
  {
    int   Return_Value;    /* =ERROR if too many menus or menu too large. */
    int   Max = N_MENUS;   /* Maximum number of menus possible.           */
    int   Next;            /* Look ahead character. Used to look for '*'. */
    int   End_Of_File=NO;  /* =YES when '*' found after menu definition.  */

    while (End_Of_File == NO && Max > 0) {
      if ((Return_Value=Get_Menu_Def(infile, Menu++)) == NO_ERROR) {
        Next = fgetc(infile);
        if (Next == '*')
          End_Of_File = YES;
        else
          ungetc(Next, infile);
      }
      Max--;
    }
    if (!End_Of_File) Return_Value = ERROR;

    return Return_Value;
  }

#ifdef LONG

/*------------------------------------------------------------------------
void Form_Menu(struct Menu *Menu);

This is the function that is used to display a menu in the long format. The
function below this one displays a menu in the short format.
------------------------------------------------------------------------*/

void Form_Menu(struct Menu *Menu)
  {
    int  Count;
    char Buffer[128+1];

    ScrClear(1, 1, 80,  1, SCR_REV_WHITE);
    ScrClear(2, 1, 80, 23, Menu->Color_Attrib);
    sprintf(Buffer, "%s %s", Menu->Title,
      (Menu == &Menu_List[0]) ? "(ESC to Return to Short Menus)" : "(ESC for Top Level Dense Menu)"
    );
    ScrPrintText(1, 1, Buffer, 80);
    for (Count = 0; Count < Menu->N_Items; Count++) {
      sprintf(Buffer, "%2d. %s%s",
        Count+1,
        Menu->Descrip[Count],
        (*Menu->Action[Count] == '@') ? "..." : ""
      );
      ScrPrintText(Count+2, 1, Buffer, 80);
    }
    ScrSetCursorPos(1, 1);
  }

#else

/*------------------------------------------------------------------------
void Form_Menu(struct Menu *Menu);

This function draws the specified menu on the screen. Note that this
function automatically sizes and centers the menu taking into account
trailing "...", if any, and the length of the menu title and menu footer.
Note also the use of the SCR_BLINK mask to insure that menu text never
blinks.
------------------------------------------------------------------------*/

void Form_Menu(struct Menu *Menu)
  {
    int   N_Lines=2*Menu->N_Items+3; /* Number of lines inside window.        */
    int   N_Cols=20;                 /* Number of columns inside window.      */
                                     /* Must be large enough for Exit_String. */
    int   Start_Row;                 /* Row on screen for top of window.      */
    int   Start_Column;              /* Column on screen for left of window.  */
    int   Border_Type;               /* What kind of window for this window.  */
    int   Row;                       /* Current row of window cursor.         */
    int   i;                         /* Generic loop index.                   */
    char  Buffer[10];                /* Buffer for preparing line headers.    */
    int   Length;                    /* Temp for holding length of string.    */
    char *Exit_String;               /* Instructions for exiting menu.        */

    ScrClear(1, 1, 80, 25, SCR_WHITE);

    /* Determine the window width needed. */
    if ((Length = strlen(Menu->Title)) > N_Cols) N_Cols = Length;
    for (i = 0; i < Menu->N_Items; i++) {
      Length = strlen(Menu->Descrip[i]);
      if (*Menu->Action[i] == '@') Length += 3;
      if (Length > N_Cols) N_Cols = Length;
    }
    N_Cols += 7;
    if (N_Cols > 78) N_Cols = 78;

    /* Create window. */
    if (Menu == &Menu_List[0]) Border_Type = SCR_DBL_LINE;
      else Border_Type = SCR_SNGL_LINE;

    Start_Row = 13 - (N_Lines+2)/2;
    Start_Column = 41 - (N_Cols+2)/2;

    ScrClear(Start_Row, Start_Column, N_Cols+2, N_Lines+2, Menu->Color_Attrib & ~SCR_BLINK);
    ScrDrawBox(Start_Row, Start_Column, N_Cols+2, N_Lines+2, Border_Type, Menu->Color_Attrib);

    /* Print header and footer into window. */
    ScrCenter(Start_Row + 1, Start_Column + 1, N_Cols,
              Menu->Title, Menu->Color_Attrib & ~SCR_BLINK);
    if (Menu == &Menu_List[0])
      Exit_String = "ESC to Return to DOS";
    else
      Exit_String = "ESC for Main Menu";
    ScrCenter(Start_Row + N_Lines, Start_Column + 1, N_Cols,
              Exit_String, Menu->Color_Attrib & ~SCR_BLINK);

    /* Print menu into window. */
    for (i = 0, Row = 3; i < Menu->N_Items; i++, Row += 2) {
      sprintf(Buffer,"F%d: ",i+1);
      ScrPrintText(Start_Row + Row, Start_Column + 2, Buffer, N_Cols - 1);
      ScrPrintText(Start_Row + Row, Start_Column + 7, Menu->Descrip[i], N_Cols - 7);
      if (*Menu->Action[i] == '@') {
        int Descriptor_Length = strlen(Menu->Descrip[i]);
        ScrPrintText(
          Start_Row + Row, Start_Column + 7 + Descriptor_Length,
          "...", N_Cols - 7 - Descriptor_Length
        );
      }
    }
    ScrSetCursorPos(1, 1);
  }

#endif

/*==================================*/
/*           Main Program           */
/*==================================*/

/*------------------------------------------------------------------------
main(int Argc, char *Argv[]);

The main function is responsible for opening, reading, and closing the menu
definition file. The main loop, a do... while loop, is the primary control
structure of the program. Note that, except for the ESC key, the keys are
handled by scanning an array of keycodes. This makes it easy to add other
key combos later. Note also that the processing of at-actions is done
inside the main loop. Probably some additional function should be used to
simplify things.
------------------------------------------------------------------------*/

int main(int Argc, char *Argv[])
  {
    auto          int   Exit_Code=0;  /* =1 if problem during program.   */
    auto          int   Quit=NO;      /* =YES when program is to stop.   */
    auto          FILE *Menu_File;    /* Pointer to menu file.           */
    auto   struct Menu *Current_Menu; /* Points at menu to display.      */
    auto          char  Command_Buffer[80+1]; /* Holds the command.      */

#ifndef LONG
    auto          int   c;            /* Command character from user.    */
    static const  int   Key_Codes[]={ /* Values returned by get_key().   */
      K_F1, K_F2, K_F3, K_F4, K_F5,
      K_F6, K_F7, K_F8, K_F9, K_F10,
      0
      };
#endif

    auto          int   Key_Number;   /* Index into Key_Codes.           */

    atexit(Credits);
    if (Argc < 2) {
      if ((Argv[1] = getenv("MDF")) == NULL) {
        printf("\nError: Menu definition file must be specified.\n");
        return 1;
      }
    }

    if ((Menu_File=fopen(Argv[1],"r")) == NULL) {
      printf("\nError: Can't open the menu definition file: %s\n",Argv[1]);
      Exit_Code = 1;
    }
    else if (Read_Menu_File(Menu_File, Menu_List) == ERROR) {
      printf("\nError: Too many menus or a menu was too large.\n");
      Exit_Code = 1;
    }
    else {
      fclose(Menu_File);
      ScrInitialize();
      Current_Menu = &Menu_List[0];

      /* Execute main loop until the user wants to quit. */
      loop {
        Form_Menu(Current_Menu);

#ifdef LONG

        Command_Buffer[0] = '\0';
        ScrPrint(25, 1, "Enter Choice: ", SCR_REV_WHITE, 14);
        if (ScrGets(25, 15, Command_Buffer, 80-15+1, SCR_REV_WHITE) == K_ESC) {
          if (Current_Menu == &Menu_List[0]) break;
          else {
            Current_Menu = &Menu_List[0];
            continue;
          }
        }
        Key_Number = atoi(Command_Buffer);

        if (Key_Number != 0 && Key_Number <= Current_Menu->N_Items) {
          Key_Number--;

#else

        c = get_key();

        /* Handle the escape key. */
        if (c == K_ESC) {
          if (Current_Menu == &Menu_List[0]) break;
            else Current_Menu = &Menu_List[0];
          continue;
        }

        /* Scan key_code array to see if the key pressed is a hot key. */
        for (Key_Number=0;
             Key_Codes[Key_Number] != 0  &&  c != Key_Codes[Key_Number];
             Key_Number++) /* Null */ ;

        /* Perform the body of the 'if' if a legit hot key was pressed. */
        if (Key_Codes[Key_Number] != 0  &&
            Key_Number < Current_Menu->N_Items) {

#endif

          ScrClear(1, 1, 80, 25, SCR_WHITE);
          ScrSetCursorPos(1, 1);

          /* The following will insure that the subshell has a large
             environment. I found that with MS-DOS version 3.21, the
             subshell only inherited an environment big enough to hold
             the current parent environment. The subshell could not add
             new items to the environment very well. This was the case
             even if the parent shell had a large environemt specified
             in CONFIG.SYS. */
          strcpy(Command_Buffer, "command /e:1024 /c ");

          /* Check to see what type of action is requested. */
          switch (*Current_Menu->Action[Key_Number]) {

            case '@': {
                struct Menu *Search;    /* Used to scan list of menus. */
                       int   Abort=NO;  /* =YES when new menu found.   */

                for (Search = Menu_List;
                     Abort == NO  &&  Search->Title != NULL;
                     Search++) {
                  if (strcmp(Search->Title,
                             &Current_Menu->Action[Key_Number][1]) == 0) {
                    Current_Menu = Search;
                    Abort = YES;
                  }
                }
              }
              break;

            case '*':
              Quit = YES;
              strcat(Command_Buffer, &Current_Menu->Action[Key_Number][1]);
              system(Command_Buffer);
              break;

            case '+':
              strcat(Command_Buffer, &Current_Menu->Action[Key_Number][1]);
              system(Command_Buffer);
              printf("\nStrike any key...");
              get_key();
              break;

            default:
              strcat(Command_Buffer, &Current_Menu->Action[Key_Number][0]);
              system(Command_Buffer);
              break;
          }

          ScrClear(1, 1, 80, 25, SCR_WHITE);
          ScrSetCursorPos(1, 1);
          Key_Number = 0;
        }

        /* Die if we've been told to quit. */
        if (Quit) break;
      }
      ScrTerminate();
    }
    return Exit_Code;
  }
