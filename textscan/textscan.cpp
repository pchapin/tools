/*========================================================================*/
/*                             textscan.cpp                               */
/*                                                                        */
/*      This  program  scans  the  text  files  specified  and reports on */
/* various unusual aspects of  those files.  In particular,  it notes odd */
/* control characters and non ASCII characters, white space at the end of */
/* lines, and an "unterminated" line at the end of the file.              */
/*      This program assumes that  a  "proper"  text  file  contains only */
/* ASCII 0x20 -> 0x7E, '\n', and '\t' characters. If any other characters */
/* appear in the file, they are  noted. The  program takes  no corrective */
/* action of any kind.                                                    */
/*      The program  was designed  to scan whole groups of files. Thus it */
/* accepts wildcards on the command line. The name of each file is echoed */
/* to the  output so  that a meaningful report can be generated using I/O */
/* redirection.                                                           */
/*                                                                        */
/*      Usage:                                                            */
/*	textscan infile [infile...]					  */
/*									  */
/* Peter Chapin 							  */
/* P.O. Box 317 							  */
/* Randolph Ctr, VT 05061						  */
/*========================================================================*/

#include <stdio.h>

/*=================================*/
/*	     Global Data	   */
/*=================================*/

long ccount[256];	 /* Array holds counts for each character. */

/*==========================================*/
/*	     Function Definitions	    */
/*==========================================*/

/*------------------------------------------------------------------------*/
/* int unknown_cntrl(int c);                                              */
/*                                                                        */
/*      This function  returns  true  if  the character  c  is  a control */
/*      character that  is not  a member  of C language escape sequences. */
/*      Such a control character does not  have an  easily understandable */
/*      name, and is thus called an "odd" control character.              */
/*------------------------------------------------------------------------*/

int unknown_cntrl(int c)
  {
    int ret_value=false;

    if (c  < 0x20  &&
	c != '\a'  &&
	c != '\b'  &&
	c != '\f'  &&
	c != '\n'  &&
	c != '\r'  &&
	c != '\t'  &&
	c != '\v') ret_value = true;
    return ret_value;
  }

/*------------------------------------------------------------------------*/
/* void print_report(long counts[]);                                      */
/*                                                                        */
/*      This function  prints the  summary report  at the  end of a scan. */
/*      Counts is  an array  with 256  elements containing  the number of */
/*      occurances of  each character.  Although the current version does */
/*	not use all of this information, future versions may.		  */
/*------------------------------------------------------------------------*/

void print_report(long counts[])
  {
    long sumup=0L;
    long bad_cntrl=0L;
    long non_ascii=0L;
    int  i;

    for (i=0; i<256; i++) sumup += counts[i];
    for (i=0; i<0x20; i++) {
      if (i != '\t' && i != '\n') bad_cntrl += counts[i];
    }
    bad_cntrl += counts[127];
    for (i=128; i<256; i++) non_ascii += counts[i];

    printf("\n\nSummary:\n\n"
	   "Total number of characters found.................: %ld\n"
	   "Total number of terminated lines found...........: %ld\n"
	   "Total number of spaces found.....................: %ld\n"
	   "Total number of tabs found.......................: %ld\n"
	   "Total number of other control characters found...: %ld\n"
	   "Total number of non ASCII characters found.......: %ld\n",
	   sumup,
	   counts['\n'],
	   counts[' '],
	   counts['\t'],
	   bad_cntrl,
	   non_ascii );
    return;
  }

/*==================================*/
/*	     Main Program	    */
/*==================================*/

int main(int argc, char *argv[])
  {
    int 	c;		/* Current character from file.      */
    int 	trail_white;	/* =true if there is trailing white. */
    int 	terminated;	/* =false when inside a line.	     */
    char       *file_name;	/* Actual filename.		     */
    FILE       *infile; 	/* Points to the input file.	     */
    int 	i;		/* Index into argv[].		     */
    int 	j;		/* Used to clear out ccount[].	     */

    fprintf(stderr, "TEXTSCAN (May 24, 2000)\n"
		    "Public Domain Software by Peter Chapin\n");

    if (argc == 1)
      fprintf(stderr, "\nUsage: textscan infile [infile...]\n\n"
		      "Program scans the input files and reports on unusual\n"
		      "characters and other odd characteristics.  The input\n"
		      "files are  assumed to  be text files.  Wildcards are\n"
		      "allowed.\n");

    /*------------------*/
    /* Read input file. */
    /*------------------*/
    for (i=1; i<argc; i++) {
      while ((file_name=*++argv) != NULL) {
	if ((infile=fopen(file_name, "r")) == NULL) {
	  fprintf(stderr, "\n\nCan't open %s; ignoring.\n\n", file_name);
	}
	else {
	  printf("\n\n********** FILE: %s **********\n\n", file_name);
	  trail_white = false;
	  terminated = true;
	  for (j=0; j<256; j++) ccount[j] = 0L;
	  while ((c=getc(infile)) != EOF) {
	    ccount[c]++;
	    if (c == '\n'  &&  trail_white) {
	      printf("\nLine %5ld: Trailing white space", ccount['\n']);
	      trail_white = false;
	    }
	    if (c == '\a')
	      printf("\nLine %5ld: Bell character found", ccount['\n']+1);
	    if (c == '\b')
	      printf("\nLine %5ld: Backspace found", ccount['\n']+1);
	    if (c == '\f')
	      printf("\nLine %5ld: Form feed found", ccount['\n']+1);
	    if (c == '\r')
	      printf("\nLine %5ld: Carriage return found", ccount['\n']+1);
	    if (c == '\v')
	      printf("\nLine %5ld: Line feed found", ccount['\n']+1);
	    if (c == 127)
	      printf("\nLine %5ld: Delete character found", ccount['\n']+1);
	    if (unknown_cntrl(c)) {
	      printf("\nLine %5ld: Odd control character found: 0x%02X: ^%c",
		      ccount['\n']+1, (unsigned) c, c+0x40);
	    }
	    if ((unsigned) c > 127) {
	      printf("\nLine %5ld: Non ASCII character found: 0x%02X",
		      ccount['\n']+1, (unsigned) c);
	    }
	    if (c==' '	||  c=='\t') trail_white=true; else trail_white=false;
	    if (c == '\n') terminated = true; else terminated = false;
	  }
	  if (!terminated) {
	    printf("\nLast line in the file does not end with the \'\\n\' character");
	  }
	  fclose(infile);
	  print_report(ccount);
	} /* End of if (can't open file) ... else ...      */
      }   /* End of while loop that processes each file.   */
    }	  /* End of for loop that processes each argument. */

    return 0;
  }
