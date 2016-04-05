/*****************************************************************************
FILE         : roff.c
LAST REVISED : January, 1990
PROGRAMMER   : Peter Chapin

	This program implements an nroff style text formatter.  It is in
the public domain.  The original was rather horrible, so I did alot of
restructuring and reformatting.  I also added some new features and
redid the documentation. 

******************************************************************************/

/* #define  DEBUG */

#define TRUE  1
#define FALSE 0
#define D() 

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*=================================*/
/*           Global Data           */
/*=================================*/

#define PAGEWIDTH  60	     /* Default page width.	 */
#define PAGELEN    66	     /* Default page length.     */
#define MAXLINE    256
#define PAGECHAR   '#'	     /* Page number escape char. */
#define OK	   1

int bFill_Mode = TRUE;   /* In fill mode if TRUE. */

int    iLine_Spacing      = 1,	        /* Current line spacing.           */
       iIndent_Level      = 0,	        /* Current indent; >= 0.           */
       iRight_Margin      = PAGEWIDTH,  /* Current right margin.           */
       iTemp_Indent_Level = 0,	        /* Current temporary indent.       */
       iCenter_Lines      = 0,	        /* Number of lines to center.      */
       iNext_Page_Nmbr    = 1,	        /* Next output page number.	   */
       iNext_Line_Nmbr    = 1,	        /* Next line to be printed.	   */
       iPage_Length       = PAGELEN,    /* Page length in lines.           */
       iTop_Margin        = 2,	        /* Top margin, including header.   */
       iTop_Space         = 3,	        /* Margin after header.	           */
       iBottom_Space      = 2,	        /* Margin after last text line.    */
       iBottom_Margin     = 3,	        /* Bottom margin, including footer.*/
       iLast_Line;		        /* Last live line on page:	   */
			                /* = iPage_Length - iBottom_Space - iBottom_Margin */
       
long lSource_Line_Nmbr = 0L;            /* Keeps track of input line nmbrs.*/

char acHeader[MAXLINE+2],               /* Top of page title.	           */
     acFooter[MAXLINE+2],               /* Bottom of page title.	   */
     acOutput_Buffer[MAXLINE+2];        /* Holds formatted lines of output.*/
     
FILE *Input_File;                       /* Points at document source.      */

/*-----------------------------------------*/
/*           Function Prototypes           */
/*-----------------------------------------*/

void vInit(void);
void vRoff(void);
void vCommand(char *);
int  iGet_Number(char *, char *);
void vSet(int *, int, char *, int, int, int);
void vText(char *);
void vLeading_Blanks(char *);
void vPrint_Line(char *);
void vPrint_Header(void);
void vPrint_Footer(void);
void vPrint_Title(char *, int);
void vSpace(int);
void vSkip(int);
int  iGet_Word(char *, int *, char *);
void vPut_Word(char *, int);
void vSpread(char *, int);
int  iWidth(char *);
void vBreak(void);
void vCenter_Line(char *);

/*==================================*/
/*           Main Program           */
/*==================================*/

int main(int iArgc, char *apcArgv[])
  {
    int i;

    D( dbg_init(0); )
    vInit();

    if (iArgc == 1) {
      Input_File = stdin;
      vRoff();
    }
    else {
      for (i = 1; i < iArgc; i++) {
        if ((Input_File = fopen(apcArgv[i], "r")) == NULL) {
          fprintf(stderr, "Error: Can't open %s\n", apcArgv[i]);
        }
        else {
          vRoff();
          fclose(Input_File);
        }
      }
    }
            
    D( dbg_terminate(); )
    return 0;
  }

/*==========================================*/
/*           Function Definitions           */
/*==========================================*/

/*****************************************************************************
	The following function initializes the line number of the last
line on the page which is used for printing.  It also sets the default
strings for the header and footer. 
******************************************************************************/

void vInit(void)
  {
    iLast_Line = iPage_Length - iBottom_Space - iBottom_Margin;
    strcpy(acHeader, "\n");
    strcpy(acFooter, "\n");
    return;
  }

/*****************************************************************************
	The following function formats the file currently at stdin.  It
reads lines of text until the EOF (or error) is reached.  If the line
begins with a '.', it is handled as a command line.  Otherwise it is
handled as a line of text. 
	When the last line of the file is read, the last page is ejected
(unless the last page is just filled). 
******************************************************************************/

void vRoff(void)
  {
    char acInput_Buffer[MAXLINE+2];

    /* Process lines from input file. */
    while (fgets(acInput_Buffer, MAXLINE+2, Input_File) != NULL) {
      lSource_Line_Nmbr++;
      if (acInput_Buffer[0] == '.') vCommand(acInput_Buffer);
        else vText(acInput_Buffer);
    }
    
    /* Eject last page. */
    vBreak();
    if (iNext_Line_Nmbr != 1) {
      vSpace(INT_MAX);
      vPrint_Footer();
    }
    
    return;
  }
  
/*--------------------------------------------------*/
/*           Command Processing Functions           */
/*--------------------------------------------------*/

/*****************************************************************************
	The following function accepts a format command string and deals
with it.  It determines what the command was and then sets the
approprate parameters. 
******************************************************************************/

void vCommand(char *pcBuffer)
  {
    int  iNumeric_Value;
    int  iSpace_Value;
    char cArgument_Type;

    iNumeric_Value = iGet_Number(pcBuffer, &cArgument_Type);
    
    if (strncmp(pcBuffer, ".bp", 3) == 0) {
      vBreak();
      if (iNext_Line_Nmbr != 1) {
        vSpace(INT_MAX);
        vPrint_Footer();
      }
    }
    
    else if (strncmp(pcBuffer, ".br", 3) == 0) {
      vBreak();
    }
    
    else if (strncmp(pcBuffer, ".ce", 3) == 0) {
      vBreak();
      vSet(
        &iCenter_Lines,
         iNumeric_Value,
        &cArgument_Type,
         1,
         0, INT_MAX
      );
    }
    
    else if (strncmp(pcBuffer, ".fi", 3) == 0) {
      vBreak();
      bFill_Mode = TRUE;
    }
    
    else if (strncmp(pcBuffer, ".fo", 3) == 0) {
      strcpy(acFooter, pcBuffer+3);
    }
    
    else if (strncmp(pcBuffer, ".he", 3) == 0) {
      strcpy(acHeader, pcBuffer+3);
    }
    
    else if (strncmp(pcBuffer, ".in", 3) == 0) {
      vSet(
        &iIndent_Level,
         iNumeric_Value,
        &cArgument_Type,
         0,
         0, iRight_Margin-1
      );
      iTemp_Indent_Level = iIndent_Level;
    }
    
    else if (strncmp(pcBuffer, ".ls", 3) == 0) {
      vSet(
        &iLine_Spacing,
         iNumeric_Value,
        &cArgument_Type,
         1,
         1, INT_MAX
      );
    }
    
    else if (strncmp(pcBuffer, ".nf", 3) == 0) {
      bFill_Mode = FALSE;
    }
    
    else if (strncmp(pcBuffer, ".pl", 3) == 0) {
      vSet(
        &iPage_Length,
         iNumeric_Value,
        &cArgument_Type,
         PAGELEN,
         iTop_Margin + iTop_Space + iBottom_Space + iBottom_Margin + 1, INT_MAX
      );
      iLast_Line = iPage_Length - iBottom_Space - iBottom_Margin;
    }
    
    else if (strncmp(pcBuffer, ".rm", 3) == 0) {
      vSet(
        &iRight_Margin,
         iNumeric_Value,
        &cArgument_Type,
         PAGEWIDTH,
         iTemp_Indent_Level + 1, INT_MAX
      );
    }
    
    /* Symmetry is broken here: This function handles the .sp command. */
    else if (strncmp(pcBuffer, ".sp", 3) == 0) {
      vBreak();
      vSet(
        &iSpace_Value,
         iNumeric_Value,
        &cArgument_Type,
         1,
         0, INT_MAX
      );
      vSpace(iSpace_Value);
      if (iNext_Line_Nmbr > iLast_Line) vPrint_Footer();
    }
    
    else if (strncmp(pcBuffer, ".ti", 3) == 0) {
      vBreak();
      vSet(
        &iTemp_Indent_Level,
         iNumeric_Value,
        &cArgument_Type,
         0,
         0, iRight_Margin
      );
    }
    
    else {
        return;    /* Ignore unknown commands */
    }
    return;
  }

/*****************************************************************************
	The following function returns the value of the numeric argument
on a format command line.  This function assumes that the command is the
first three characters of the line (only two letter commands allowed). 
This function will also copy the first character of the numeric argument
into *pcArgument_Type.  If this character is a '+' or '-', this function
skips it before trying to do the conversion. 
******************************************************************************/

int iGet_Number(char *pcBuffer,char *pcArgument_Type)
  {
    int i;

    i = 3;
    
    /* Find argument. */
    while (pcBuffer[i] == ' '  ||  pcBuffer[i] == '\t') i++;
    
    /* Save first character as argument type. */
    *pcArgument_Type = pcBuffer[i];
    
    /* If argument type was one of the known types, advance index. */
    if (*pcArgument_Type == '+'  ||  *pcArgument_Type == '-') i++;

    /* Return converted integer. Note that this ignores extra charcters on line. */
    return atoi(pcBuffer + i);
  }


/*****************************************************************************
	The following function sets an integer parameter.  The arguments
specify the new value and how the new value is to be interpreted.  They
also specify the legal range of value.  The function verifies that the
new value is in range.  If the new value is not in range, the function
forces the max or min value as appropriate. 
******************************************************************************/

void vSet(
  int  *piParameter,          /* Points to integer parameter to set.      */
  int    iNumeric_Value,      /* Desired value of increment.              */
  char *pcArgument_Type,      /* Specifies what to do with iNumeric_Type. */
  int    iDefault_Value,      /* Default value.                           */
  int    iMin_Value,          /* Minimum legal value.                     */
  int    iMax_Value           /* Maximum legal value.                     */
  )
  {
    switch (*pcArgument_Type) {
      
      case '\n':
        *piParameter = iDefault_Value;
        break;
         
      case  '+':
        *piParameter += iNumeric_Value;
        break;
         
      case  '-':
        *piParameter -= iNumeric_Value;
        break;
         
      default:
        *piParameter = iNumeric_Value;
        break;
    }

    *piParameter = min(*piParameter, iMax_Value);
    *piParameter = max(*piParameter, iMin_Value);
    return;
  }
  
/*-----------------------------------------------*/
/*           Text Processing Functions           */
/*-----------------------------------------------*/

/*****************************************************************************
	The following function handles each input line of text which is
not a formatting command.  The parameter points at the input line. 
******************************************************************************/

void vText(char *pcInput_Buffer)
  {
    char acWord_Buffer[MAXLINE+2];
    int    i;

    D( dbg("vTextT", 1, "Source line = %s", pcInput_Buffer); )
    
    /* If first character on line is a space or tab... */
    if (*pcInput_Buffer == ' '  ||  *pcInput_Buffer == '\t') {
    
      /* Do a break when a line with leading blanks is found. */
      vBreak();
      
      /* Adjusts iTemp_Indent_Level. */
      vLeading_Blanks(pcInput_Buffer);
    }

    /* If we are to center this line, do it. */
    if (iCenter_Lines > 0) {
      vCenter_Line(pcInput_Buffer);
      vPrint_Line(pcInput_Buffer);
      iCenter_Lines--;
    }
    
    /* If this line is blank, print it. */
    else if (*pcInput_Buffer == '\n') {
      vBreak();
      vPrint_Line(pcInput_Buffer);
    }
    
    /* If we are not filling, print line. */
    else if (bFill_Mode == FALSE) {
      vPrint_Line(pcInput_Buffer);
    }
    
    /* Copy words into buffer. vPut_Word will print the line when it needs to. */
    else {
      i = 0;
      while (iGet_Word(pcInput_Buffer, &i, acWord_Buffer) > 0)
        vPut_Word(acWord_Buffer, iRight_Margin - iTemp_Indent_Level);
    }
    
    D( dbg("vTextB", 1, NULL); )
    return;
  }

/*****************************************************************************
	Delete leading blanks.  Set iTemp_Indent_Level.
******************************************************************************/

void vLeading_Blanks(char *pcBuffer)
  {
    int i,j;
    int iDisplay_Offset = 0;

    D( dbg("vLeading_BlanksT", 2, "Working with: %s", pcBuffer); )
    
    /* Find first non-blank character. Compute offset to use during printing. */
    for (i = 0; pcBuffer[i] == ' '  ||  pcBuffer[i] == '\t'; i++) {
      if (pcBuffer[i] == '\t') iDisplay_Offset += 8;
      else iDisplay_Offset++;
    }

    /* iTemp_Indent_Level will == iIndent_Level at this point. */
    if (pcBuffer[i] != '\n') iTemp_Indent_Level += iDisplay_Offset;

    /* Move line to left */
    j = 0;
    while ((pcBuffer[j++] = pcBuffer[i++]) != '\0') ;
    
    D( dbg("vLeading_BlanksB", 2, "Now have: %s", pcBuffer); )
    return;
  }

/*****************************************************************************
	Put out line with proper spacing and indenting.
******************************************************************************/

void vPrint_Line(char *pcBuffer)
  {
    int i;

    /* If at top of page, print the header. */
    if (iNext_Line_Nmbr == 1) vPrint_Header();

    /* Reset iTemp_Indent_Level for next line. */
    for (i = 1; i <= iTemp_Indent_Level; i++) putchar(' ');
    iTemp_Indent_Level = iIndent_Level;

    /* Do the actual output. */    
    fputs(pcBuffer, stdout);
    
    /* Skip for next line. Don't go past the end of the page! */
    vSpace(iLine_Spacing-1);
    iNext_Line_Nmbr += iLine_Spacing;
    
    /* If we are at the end of the current page... */
    if (iNext_Line_Nmbr > iLast_Line) {
      vPrint_Footer();
    }
    
    return;
  }

/*****************************************************************************
	The following function handles the page header.
******************************************************************************/

void vPrint_Header(void)
  {
    /* Header disabled if iTop_Margin == 0 */
    if (iTop_Margin > 0) {
      vSkip(iTop_Margin);
      vPrint_Title(acHeader, iNext_Page_Nmbr);
    }
    
    /* Adjust iNext_Line_Nmbr to reflect the new position. */
    vSkip(iTop_Space);
    iNext_Line_Nmbr = iTop_Margin + iTop_Space + 1;
    return;
  }

/*****************************************************************************
	The following function handles the page footer.
******************************************************************************/

void vPrint_Footer(void)
  {
    vSkip(iBottom_Space);
    
    /* iBottom_Margin == 0 will disable the footer. */
    if (iBottom_Margin > 0) {
      vPrint_Title(acFooter, iNext_Page_Nmbr);
    }
    iNext_Page_Nmbr++;
    iNext_Line_Nmbr = 1;
    putchar('\f');
    
    return;
  }

/*****************************************************************************
	The following function emits a header or footer with optional
page number. 
******************************************************************************/

void vPrint_Title(char *pcBuffer, int iPage_Nmbr)
  {
    int   i;
    char *pcEnd_Pntr;

    /* Trash '\n' if there is one. */
    if ((pcEnd_Pntr = strchr(pcBuffer, '\n')) != NULL) *pcEnd_Pntr = '\0';
    
    /* Print enough space to flush title to right margin. */
    for (i = 0; i < iRight_Margin-strlen(pcBuffer); i++) putchar(' ');
    
    /* Print the title itself. */
    for (i = 0; pcBuffer[i] != '\0'; i++)
      if (pcBuffer[i] == PAGECHAR)
        printf("%d", iPage_Nmbr);
      else
        putchar(pcBuffer[i]);
        
    putchar('\n');
    return;
  }

/*****************************************************************************
	The following function spaces the specified number of lines or
until the end of the page. Note that a parameter of zero is acceptable.
******************************************************************************/

void vSpace(int iNmbr_Lines)
  {
    int iSkip_Distance;
    
    /* Ignore if already at the bottom of the page. */
    if (iNext_Line_Nmbr > iLast_Line) return;

    /* Skip the required number of lines. Don't go off of page. */
    iSkip_Distance = min(iNmbr_Lines, iLast_Line - iNext_Line_Nmbr);
    vSkip(iSkip_Distance);
    
    /* Adjust next line number. */
    iNext_Line_Nmbr += iSkip_Distance;
    
    return;
  }

/*****************************************************************************
	The following function emits the specified number of '\n'
characters.  It does not check for end of page conditions, etc. 
******************************************************************************/

void vSkip(int iNmbr_Lines)
  {
    while ( iNmbr_Lines-- > 0) putchar('\n');
      
    return;
  }

/*****************************************************************************
	The following function copies a word form the input string to
the output string.  *piInput_Index contains the index into the input
string where the search is to start.  The function skips blanks, and
then copies the first word it finds into the output string.  It returns
the length.  Additionally *piInput_Index is updated past the word.  The
function returns zero if there are no words found. 
******************************************************************************/

int iGet_Word(char *pcIn, int *piInput_Index, char *pcOut)
  {
    int i, j;
    
    D( dbg("iGet_WordT", 2, "Working with: %s", &pcIn[*piInput_Index]); )

    /* Skip over blanks. */
    i = *piInput_Index;
    while (isspace(pcIn[i])) i++;

    /* Copy word into pcOut. */
    j = 0;
    while (pcIn[i] != '\0'  &&  !isspace(pcIn[i])) pcOut[j++] = pcIn[i++];
    pcOut[j] = '\0';
    
    *piInput_Index = i;     /* Return index in iInput_Index.  */
    
    D( dbg("iGet_WordB", 2, "Now have: %s", &pcIn[*piInput_Index]); )
    return j;               /* Return length of word.         */
  }

/*****************************************************************************
	Put a word in acOutput_Buffer; includes margin justification. 
The input parameter points at a null terminated word. 
******************************************************************************/

void vPut_Word(char *pcWord_Buffer, int iTotal_Length)
  {
    auto   char *pcEnd_Pntr = pcWord_Buffer;
    auto   int     iLength;
    
    D( dbg("vPut_WordT", 2, "acOutput_Buffer = %s", acOutput_Buffer); )

    /* Get length of input word. */
    iLength = strlen(pcWord_Buffer);
    
    /* Point at terminating null character of current word buffer. */
    pcEnd_Pntr = strchr(acOutput_Buffer, '\0');
    
    /* Is there space to put the new word in the output buffer? */    
    if ((pcEnd_Pntr-acOutput_Buffer) + iLength + 1 > iTotal_Length) {
      
      /* There was not; add spaces to justify both margins. */
      vSpread(acOutput_Buffer, iTotal_Length);
      
      /* Output the line. */
      strcat(acOutput_Buffer, "\n");
      vPrint_Line(acOutput_Buffer);
      acOutput_Buffer[0] = '\0';
    }

    /* Place the word onto the  end of the accumulating buffer. */    
    strcat(acOutput_Buffer, pcWord_Buffer);
    strcat(acOutput_Buffer, " ");
    
    D(dbg("vPut_WordB", 2, "acOutput_Buffer = %s", acOutput_Buffer); )
    
    return;
  }

/*****************************************************************************
	Spread words to justify right margin.  This function assumes
that the string in pcBuffer contains single spaces separating words.  It
will add additional spaces so that the total length is iTotal_Length
(not including the null character.  It adds extra spaces first from the
left and then from the right, alternating with each call.  Note that
this function assumes that the buffer pointed at by pcBuffer is large
enough to hold the result. 
******************************************************************************/

void vSpread(char *pcBuffer, int iTotal_Length)
  {
    typedef enum { FORWARD, BACKWARD } Directions;
    
    static Directions  Direction_Flag = BACKWARD;
    auto   char      acWorking_Buffer[MAXLINE+2];
    auto   char     *pcBuffer_Pntr;     /* Points into the parameter.      */
    auto   char     *pcWorking_Pntr;    /* Points into the working buffer. */
    auto   char     *pcEnd_Pntr;        /* Points at end of parameter.     */
    auto   int        iNmbr_Holes = 0;  /* Number of spaces in parameter.  */
    auto   int        iNmbr_Extra;      /* Number of spaces to add.        */
    auto   int        iNmbr_Blanks;     /* Spaces/space to add.            */
    auto   int        i;                /* Index to loop which adds spaces.*/
    
    D( dbg("vSpreadT", 3, "Buffer = %s", pcBuffer); )

    /* Reverse direction. */
    Direction_Flag = (Direction_Flag == FORWARD) ? BACKWARD : FORWARD;

    /* Point at the very end of the string. */
    pcEnd_Pntr = strchr(pcBuffer, '\0');
    
    /* Trash the extra space at the end and adjust pcEnd_Pntr. */
    *--pcEnd_Pntr = '\0';

    /* Count the number of spaces in the string. */
    for (pcBuffer_Pntr = pcBuffer; *pcBuffer_Pntr; pcBuffer_Pntr++) {
      if (*pcBuffer_Pntr == ' ') iNmbr_Holes++;
    }
    
    /* Compute the number of spaces that need to be introduced. */
    iNmbr_Extra = iTotal_Length - (pcEnd_Pntr - pcBuffer);
    iNmbr_Blanks = iNmbr_Extra/iNmbr_Holes + 1;

    /* Copy parameter into working buffer. Add extra spaces as necessary. */ 
    switch (Direction_Flag) {
    	
      case FORWARD:
        
        /* Point at start of working buffer and scan over parameter. */
        pcWorking_Pntr = acWorking_Buffer;
        for (pcBuffer_Pntr = pcBuffer; *pcBuffer_Pntr; pcBuffer_Pntr++) {
          
          /* Copy each character. */
          *pcWorking_Pntr++ = *pcBuffer_Pntr;
          
          /* If it was a space and we still need to expand the string... */
          if (*pcBuffer_Pntr == ' '  &&  iNmbr_Extra > 0) {

            /* Insert the appropriate number of extra spaces. */
            /* The last time we do this, we may not need iNmbr_Blank spaces. */
            int iNmbr_This_Time;
            iNmbr_This_Time = min(iNmbr_Blanks, iNmbr_Extra);
            for (i = 0; i < iNmbr_This_Time; i++) *pcWorking_Pntr++ = ' ';
            iNmbr_Extra -= iNmbr_This_Time;
          }
        }
        *pcWorking_Pntr++ = '\0';
        break;
        
      case BACKWARD:
        pcWorking_Pntr = acWorking_Buffer + iTotal_Length;
        for (pcBuffer_Pntr = pcEnd_Pntr; pcBuffer_Pntr >= pcBuffer; pcBuffer_Pntr--) {
          *pcWorking_Pntr-- = *pcBuffer_Pntr;
          if (*pcBuffer_Pntr == ' '  &&  iNmbr_Extra > 0) {
            
            int iNmbr_This_Time;
            iNmbr_This_Time = min(iNmbr_Blanks, iNmbr_Extra);
            for (i = 0; i < iNmbr_This_Time; i++) *pcWorking_Pntr-- = ' ';
            iNmbr_Extra -= iNmbr_This_Time;
          }
        }
        break;
        
    }

    /* Copy result over parameter. */
    strcpy(pcBuffer, acWorking_Buffer);
    
    D( dbg("vSpreadB", 3, "Buffer = %s", pcBuffer); )
    return;
  }

/*****************************************************************************
	The following function computes the width of a string.  It is
similar to strlen() except that some special characters are handled as
desired. 
******************************************************************************/

int iWidth(char *pcBuffer)
  {
    int i;
    int iResult;

    for (iResult = i = 0; pcBuffer[i] != '\0'; i++)
      if (pcBuffer[i] == '\b')
        iResult--;
      else if (pcBuffer[i] != '\n')
        iResult++;
        
    return iResult;
  }


/*****************************************************************************
	The following function dumps the current partially filled line. 
It must be called whenever there is a break in the input stream. 
******************************************************************************/

void vBreak(void)
  {
    if (acOutput_Buffer[0] != '\0') {
      strcat(acOutput_Buffer, "\n");
      vPrint_Line(acOutput_Buffer);
    }
    acOutput_Buffer[0] = '\0';
    return;
  }

/*****************************************************************************
	The following function will cause the next line which is output
to be centered.  This is done by adjusting iTemp_Indent_Level. 
******************************************************************************/

void vCenter_Line(char *pcBuffer)
  {
    iTemp_Indent_Level =
      max((iRight_Margin + iTemp_Indent_Level - iWidth(pcBuffer))/2, 0);
    return;
  }

