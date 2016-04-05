/****************************************************************************
FILE          : ktour.c
LAST REVISION : October 1998
AUTHOR        : Peter Chapin
SUBJECT       : Knight's Tour solving program.

This program uses a number of techniques to obtain soltions quickly. It
tends to investigate moves first that have a minimum number of additional
follow up moves. This encourages the knight to enter corners early and
hug the edges of the board. This simple device greatly reduces the time
required to find the first solution.

The program also uses some involved board analysis to cut the number of
positions that need to be searched. If isolated squares or groups of square
that are connected but are isolated from the other squares develop, the
program will reject any further investigation in that direction. Although
this analysis is somewhat time consuming it significantly enhanced
performance (in earlier version of the program at least).

In addition, the program "precalculates" the legal knight moves. Thus the
program only needs to read these moves out of a table during the run rather
than compute them fresh at each step.

The program is not, probably, optimized to the utmost. But, hey, I need to
leave something for version 2!

The program has a number of features: It can handle different size boards,
including rectanglar boards; print solutions to a file; count solutions;
etc. Invoke the program with no arguments for a complete usage description.

                                                     peterc (December 1988)

Please send comments or bug reports to

     Peter Chapin
     Electrical and Computer Engineering Technology
     Vermont Technical College
     Randolph Center, VT 05061
     Peter.Chapin@vtc.vsc.edu
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "local.h"
#include "getswtch.h"

/*=================================*/
/*           Global Data           */
/*=================================*/

int   f_switch  = FALSE;    /* Default values for the option switches */
int   c_switch  = FALSE;
int   p_switch  = FALSE;
int   i_switch  = 0;
char *w_switch  = NULL;
int   q_switch  = FALSE;

/*--------------------------------------------------*/
/* Table to inform the  switch handling  functions  */
/* which switches are legal and where to find them. */
/*--------------------------------------------------*/
SWTCH sw_table[]={
  { 'f', BIN_SWTCH, &f_switch, NULL,
         "Find first solution and exit."                                },
  { 'c', BIN_SWTCH, &c_switch, NULL,
         "Display the total number of solutions found."                 },
  { 'p', BIN_SWTCH, &p_switch, NULL,
         "Disable the printing of solutions."                           },
  { 'i', INT_SWTCH, &i_switch, NULL,
         "Inform when the specified <num> of pos. have been considered."},
  { 'w', STR_SWTCH, NULL, &w_switch,
         "Write solution(s) to the file named by <str>."                },
  { '?', BIN_SWTCH, &q_switch, NULL,
         "Print Author's address."                                      }
  };

#define NUM_SWITCHS  6     /* Number of entries in the above table. */

#define MAX_BSIZE   16     /* Largest board that can be handled.    */

short board[MAX_BSIZE+1][MAX_BSIZE+1];  /* Indicies are 1 relative.       */
int   bisize;        /* Current board row dimension.                      */
int   bjsize;        /* Current board col dimension.                      */
int   move_nmbr=0;   /* Current move number.                              */
int   max_move_nmbr; /* Used to check for solution.                       */
long  nmbr_leaves=0; /* Number of leaves encountered in the search tree.  */
long  nmbr_sols=0;   /* Number of solutions encountered.                  */
long  nmbr_pos=0;    /* Number of positions considered.                   */

FILE  *out_file;     /* Output file where solutions are written. */

#define MARKER    -1      /* Used by markit() during connection analysis. */

typedef struct {
  int x,y;                /* x => row number, y => col number */
} move;

/*-------------------------------------------------------------------------*/
/* The following 3D array holds lists of the legal  moves from each square */
/* on the board.  There are 9 elements in each list  eventhough the great- */
/* est  number of moves  is only 8.  This  allows  for a list  terminating */
/* element.  NOTE:  This array requires  2*9*(MAX_BSIZE+1)^2 * sizeof(int) */
/* bytes of memory.                                                        */
/*-------------------------------------------------------------------------*/
move move_table[MAX_BSIZE+1][MAX_BSIZE+1][9];

/*=========================================*/
/*           Function Prototypes           */
/*=========================================*/

void prnt_stuff(void);               /* Prints my intro, etc.           */
void get_moves(move *, int, int);    /* Calculates legal moves.         */
void init_moves(void);               /* Fills in move_table[].          */
int  compare_moves(const void *, const void *);  /* Used when moves are sorted.     */
void markit(int, int);               /* Used to determine connectivity. */
void print_sol(void);                /* Prints solutions.               */
int  isconnect(void);                /* Determines connectivity.        */
void search(int, int);               /* Searches from given location.   */


/*==============================================*/
/*           Function Implementations           */
/*==============================================*/

void prnt_stuff(void)
  {
    static char *message[]={
      "This program and it's source code is placed into the Public Domain",
      "by me, its author.  Feel free to do  whatever you want with it.  I",
      "do ask, however,  that you  delete my name from the program if you",
      "make extensive or significant modifications.",
      "",
      "The source code (in C) consists of the following files:",
      "",
      "KTOUR.C       (The primary file -- this contains most of the program)",
      "LOCAL.H       (Header that has misc. #defines I like to use)",
      "GETSWTCH.C    (Functions that process command line switches)",
      "GETSWTCH.H    (Header that goes with GETSWTCH.C)",
      "",
      "Note that GETSWTCH.C is an adaptation of a module presented in the",
      "book  \"C Chest  and Other C  Treasures\"  by Allen Holub.  Read the",
      "note at the top of GETSWTCH.C",
      "",
      "Compiling should be straight forwared as the code does not do any-",
      "thing  unusual.  You may have to  request extra stack  space.  The",
      "program is highly recursive",
      "",
      "I can be reached at the address below. I welcome comments.",
      "",
      "     Peter Chapin",
      "     PO Box 317",
      "     Randolph Center VT 05061",
      "",
      NULL
      };
    int i;

    for (i=0; message[i] != NULL; i++)
      printf("%s\n", message[i]);
    return;
  }

/*-----------------------------------------------------------------------*/
/* The following function calculates what square can be reached from the */
/* given square. It fills the array list[] with the computed moves. Note */
/* that list[]'s  indicies are 1 relative.  Also note that no attempt is */
/* made here to  check for off  the board  conditions.  This function is */
/* used during the initialization.  It is never called once move_table[] */
/* is set up.                                                            */
/*-----------------------------------------------------------------------*/

void get_moves(move *list, int x, int y)
  {
    register int temp;

    temp = x-2;          /* This rather strange method is a remanant from  */
    list[5].x = temp;    /* earlier optimizations. The use of move_table[] */
    list[6].x = temp;    /* renders this approach unnecessary.             */
    temp++;
    list[4].x = temp;
    list[7].x = temp;
    temp += 2;
    list[3].x = temp;
    list[8].x = temp;
    temp++;
    list[1].x = temp;
    list[2].x = temp;
    temp = y-2;
    list[7].y = temp;
    list[8].y = temp;
    temp++;
    list[1].y = temp;
    list[6].y = temp;
    temp += 2;
    list[2].y = temp;
    list[5].y = temp;
    temp++;
    list[3].y = temp;
    list[4].y = temp;
    return;
  }

/*------------------------------------------------------------------------*/
/* The following  function  calculates the  entries of  move_table[].  It */
/* only bothers to  calculate moves for  the square inside  the requested */
/* board dimensions. Note that moves that go off the board are not placed */
/* in the table. This function is only called during the initialization.  */
/*------------------------------------------------------------------------*/

void init_moves(void)
  {
    int  i,j,k;
    int  index;
    move move_list[8+1];  /* One extra because of get_moves() */

    for (i=1; i<=bisize; i++) {
      for (j=1; j<=bjsize; j++) {
        /*----------------------------------------------------------*/
        /* Load up the list for square (i,j) with legal moves. Note */
        /* that move_table[] is  assumed  initialized to zero (used */
        /* later as a list terminater.                              */
        /*----------------------------------------------------------*/
        get_moves(move_list, i, j);
        for (k=1, index=0; k<=8; k++) {
          if (!(move_list[k].x > bisize  ||  move_list[k].x < 1  ||
                move_list[k].y > bjsize  ||  move_list[k].y < 1)    ) {
            move_table[i][j][index++] = move_list[k];
          }
        }
      }
    }
    for (i=1; i<=bisize; i++)
      for (j=1; j<=bjsize; j++)
        /*--------------------------------------------------------------*/
        /* Sort  the list of moves for square  (i,j)  so that the first */
        /* moves investigated are the ones which have the fewest number */
        /* of choices in the next step.  This greatly speeds arrival at */
        /* the  first  solution.  Note:  different  implementations  of */
        /* qsort() give different performance results.                  */
        /*--------------------------------------------------------------*/
        qsort((void *)move_table[i][j], 9, sizeof(move), compare_moves);
    return;
  }

/*--------------------------------------------------------------------*/
/* The following function  is used by qsort()  to order the  moves in */
/* move_table[].  It returns 1  if the  square refered to by "a"  has */
/* more legal moves  from it than the square  refered to by  "b".  It */
/* returns -1 if the role of "a" and "b" are reversed and 0 if square */
/* "a" has the same number of moves from it as square "b".            */
/*--------------------------------------------------------------------*/

int compare_moves(const void *a_void, const void *b_void)
  {
    int Anum_moves=9;   /* More than the max # of moves for square "a" */
    int Bnum_moves=9;   /* etc                                         */
    int ret_value;

    const move *a = (const move *)a_void;
    const move *b = (const move *)b_void;

    /*-----------------------------------------------------------------*/
    /* The checking for "a->x != 0", etc is so qsort() won't  sort the */
    /* list terminators into the list! We want list terminators to act */
    /* like they are "very large"  to force them all to the end of the */
    /* list.                                                           */
    /*-----------------------------------------------------------------*/
    if (a->x != 0)
      for (Anum_moves=0;
           move_table[a->x][a->y][Anum_moves].x != 0;
           Anum_moves++) ;
    if (b->x != 0)
      for (Bnum_moves=0;
           move_table[b->x][b->y][Bnum_moves].x != 0;
           Bnum_moves++) ;
    /*-----------------------------------------------*/
    /* Set ret_value according to the above results. */
    /*-----------------------------------------------*/
    if (Anum_moves == Bnum_moves) ret_value = 0;
    else if (Anum_moves > Bnum_moves) ret_value = 1;
    else ret_value = -1;
    return (ret_value);
  }

/*----------------------------------------------------------------------*/
/* The following function places markers on the board at every location */
/* reachable  by the knight  from the  given location.  It calls itself */
/* recursively to descend a tree of possible knight moves.              */
/*----------------------------------------------------------------------*/

void markit(int x,int y)
  {
    register move *trial_move;

    board[x][y] = MARKER;  /* Place a marker on current square. */
    for (trial_move=move_table[x][y]; trial_move->x != 0; trial_move++) {
      if (board[trial_move->x][trial_move->y] == 0) {
        markit(trial_move->x, trial_move->y);
      }
    }
    return;
  }

/*---------------------------------------------------------------*/
/* The following function prints the current board. It is called */
/* when a solution is found to print that solution.              */
/*---------------------------------------------------------------*/

void print_sol(void)
  {
    int i,j;

    fprintf(out_file,"\n\nSolution #%ld:\n",nmbr_sols);
    fprintf(out_file,"    Number of positions : %10ld\n",nmbr_pos);
    fprintf(out_file,"    Number of leaves    : %10ld\n",nmbr_leaves);
    for (i=1; i<=bisize; i++) {
      for (j=1; j<=bjsize; j++) {
        fprintf(out_file,"%3d ",board[i][j]);
      }
      fprintf(out_file,"\n\n");
    }
    fprintf(out_file,"\n");
    return;
  }

/*------------------------------------------------------------------------*/
/* The following  function  determines if the unused squares on the board */
/* are "connected." If there are two  (or more)  groups of unused squares */
/* that are unconnected in the sense that a knight could not move between */
/* them, there is  no solution.  Isconnect()  returns NO in that case The */
/* method catches isolated squares as a special case.                     */
/*------------------------------------------------------------------------*/

int isconnect(void)
  {
    register int i,j;
    register int free=NO;

    /*-----------------------------------------------------------*/
    /* Search the board for an unused square. There must be one. */
    /*-----------------------------------------------------------*/
    for (i=1; i<=bisize; i++)
      for (j=1; j<=bjsize; j++)
        if (board[i][j] == 0) goto fndit;

fndit:
    /*-----------------------------------------------------------------*/
    /* Place markers on the board on every square a knight could reach */
    /* from the unused square just found.                              */
    /*-----------------------------------------------------------------*/
    markit(i,j);

    /*---------------------------------------------------------------*/
    /* Scan the board to see if there are any unused square that are */
    /* not marked. Clean up the markers as we go.                    */
    /*---------------------------------------------------------------*/
    for (i=1; i<=bisize; i++)
      for (j=1; j<=bjsize; j++) {
        if (board[i][j] == 0) free = YES;
        if (board[i][j] == MARKER) board[i][j] = 0;
      }
    if (free == YES) return (NO); else return (YES);
  }

/*----------------------------------------------------------------------*/
/* The following function conducts the actual search for solutions.  It */
/* places  the  knight on  the  given square  and analyzes  the current */
/* board position. If the current position is tree node, search() calls */
/* itself to try  each legal  move that can  be made  from the  current */
/* move.  When the top level search()  returns all  possible moves have */
/* been  considered  (the tree has been  completely searched). Thus all */
/* solutions will have been found. This can take awhile!                */
/*----------------------------------------------------------------------*/

void search(int x, int y)
  {
    register move *trial_move;

    board[x][y] = ++move_nmbr;    /* Place knight */
    ++nmbr_pos;
    if ((i_switch != 0) && (nmbr_pos % i_switch == 0)) {
      fprintf(stderr,"\rNumber of positions considered = %10ld",nmbr_pos);
    }

    if (move_nmbr == max_move_nmbr) {
      nmbr_leaves++;
      nmbr_sols++;
      if (!p_switch) print_sol();
      if (f_switch)  exit(0);
    }
    else if (isconnect() == NO) {
      nmbr_leaves++;
    }
    else {
      for (trial_move=move_table[x][y]; trial_move->x != 0; trial_move++) {
        if (board[trial_move->x][trial_move->y] == 0) {
          search(trial_move->x, trial_move->y);
        }
      }
    }
    --move_nmbr;            /* We're backing up; retract move. */
    board[x][y] = 0;
    return;
  }

/*==================================*/
/*           Main Program           */
/*==================================*/

int main(int argc, char *argv[])
  {
    int exit_code = 0;
    int ipos_x, ipos_y;

    out_file = stdout;

    /*--------------------------------------------------*/
    /* Print header and look for command line switches. */
    /*--------------------------------------------------*/

    fprintf(stderr,"Knight's Tour, Version 1.0 (December 1988)\n");
    fprintf(stderr,"Public Domain Software by Peter Chapin\n\n");

    argc = get_swtchs(argc, argv, sw_table, NUM_SWITCHS);

    /*-----------------------------------------------*/
    /* Print info screen or usage message if needed. */
    /*-----------------------------------------------*/
    if (q_switch) {
      prnt_stuff();
    }
    else if (argc != 5) {
      fprintf(stderr,
        "Usage: KTOUR  [options]  nrows  ncols  start_row  start_col\n\n");
      pr_usage(sw_table, NUM_SWITCHS, stderr);
      exit_code = 1;
    }
    /*----------------------------------------------------------*/
    /* Extract the parameters of the run from the command line. */
    /*----------------------------------------------------------*/
    else if ( (bisize=atoi(argv[1])) > MAX_BSIZE  ||  bisize < 3) {
      fprintf(stderr,
              "ERROR: Board size must be between 3 and %d.\n",MAX_BSIZE);
      exit_code = 2;
    }
    else if ( (bjsize=atoi(argv[2])) > MAX_BSIZE  ||  bjsize < 3) {
      fprintf(stderr,
              "ERROR: Board size must be between 3 and %d.\n",MAX_BSIZE);
      exit_code = 2;
    }
    else if ( (ipos_x=atoi(argv[3])) > bisize  ||  ipos_x < 1) {
      fprintf(stderr,
              "ERROR: Initial x position is not on the board.\n");
      exit_code = 2;
    }
    else if ( (ipos_y=atoi(argv[4])) > bjsize  ||  ipos_y < 1) {
      fprintf(stderr,
              "ERROR: Initial y position is not on the board.\n");
      exit_code = 2;
    }
    else if (w_switch != NULL  &&  (out_file=fopen(w_switch,"w")) == NULL) {
      fprintf(stderr,
              "ERROR: Can't open the file %s; aborting run.\n",w_switch);
      exit_code = 3;
    }
    else {
      init_moves();                   /* Set up move_table[]     */
      max_move_nmbr = bisize*bjsize;  /* Calculate this (once).  */
      search(ipos_x, ipos_y);         /* Find all solutions.     */
      if (c_switch)
        fprintf(out_file,
                "\nThe total number of solutions found = %8ld\n",nmbr_sols);
      else
        fprintf(out_file,"\nDone\n");
    }
    return exit_code;
  }
