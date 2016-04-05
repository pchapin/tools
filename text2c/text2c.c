/*=====================================================================*/
/* txttoc.c							       */
/*								       */
/* This  program reads a  text file and produces a  file that contains */
/* the declaration of an array of character pointers. The pointers are */
/* initialized to point at the strings provided in the text file.  The */
/* program  quotes all  relevant  characters (ANSI)  and even provides */
/* escape  sequences  for  non-printing  characters.  Note that the \n */
/* character  at the end of  each line	is not	included in the  array */
/* declaration. 						       */
/*=====================================================================*/

#include <stdio.h>     /* Standard Library */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*========================*/
/* Global Variables, Etc. */
/*========================*/

#define YES 1
#define NO  0

#define LINE_BUF_SIZE	256    /* Max line length that can be handled.	     */
#define INDENT_LEVEL	2      /* Number of columns each string is indented. */

/*------------------------------------------------------------------------*/
/* The following macro is used to get the indentation right. The paramter */
/* named 'x' is used to specify an offset above the user indent level.    */
/*------------------------------------------------------------------------*/
#define ALIGN(x)  for (i=1; i<column_no+x; i++) putc(' ', outfile)

/*======================*/
/* Function Definitions */
/*======================*/

/*----------------------------------------------------------------------*/
/* The following function verifies that the string pointed at by "name" */
/* is a valid C identifier name. Note that it does not check the length */
/* of the string, since identifiers can be of any length.		*/
/*----------------------------------------------------------------------*/

int name_ok(char *name)
  {
    int    ret_code=YES;	 /* Returns NO if name is no good. */
    size_t length=strlen(name);
    static char legal_chars[]="abcdefghijklmnopqrstuvwxyz"
			      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			      "1234567890_";

    if	    (!isalpha(name[0]) && name[0] != '_') ret_code = NO;
    else if (strspn(name, legal_chars) != length) ret_code = NO;
    return (ret_code);
  }

/*===============*/
/* Main function */
/*===============*/

main(int argc, char *argv[])
  {
    int  exit_code=0;	 /* returned to operating system.		    */
    int  column_no;	 /* =1 when array declaration starts in column one. */
    FILE *infile;	 /* Input file contains text.			    */
    FILE *outfile;	 /* Output file contains char *[] declaration.	    */
    char buffer[LINE_BUF_SIZE+2];  /* Holds lines from the input file.	    */
    char *buf_ptr;	 /* Points into buffer[] during processing.	    */
    char c;		 /* A character out of buffer[].		    */
    int  i;		 /* Used for loop indicies (see macro ALIGN()).     */

    long nmbr_lines=0;	 /* Number of lines found in the input file.	    */
    long nmbr_chars=0;	 /* Number of characters that will be compiled.     */
    long nmbr_norms=0;	 /* Number of "normal" characters found.	    */

    int  column_width;	      /* Number of characters on one line of C.     */
    int  max_column_width=0;  /* Longest line produced by program.	    */

    fprintf(stderr, "TXTTOC  Version 1.0  (February 1989)\n");
    fprintf(stderr, "Public Domain Software by Peter Chapin\n\n");

    if (argc != 5) {
      fprintf(stderr, "Usage  : txttoc a_name column infile.txt otfile.c\n\n"
		      "a_name : Name of the array to be declared.\n"
		      "column : Column number at which declaration starts.\n"
		      "infile : Filename of file containing raw text.\n"
		      "otfile : Filename where declaration will be placed.\n");
      exit_code = 4;
    }
    else if (!name_ok(argv[1])) {
      fprintf(stderr, "The name \"%s\" is not a legal C identifier name.\n",
	      argv[1]);
      exit_code = 4;
    }
    else if ((column_no=atoi(argv[2])) < 1  ||	column_no > 40) {
      fprintf(stderr, "The column number %d is illegal.\n", column_no);
      exit_code = 4;
    }
    else if ((infile=fopen(argv[3], "r")) == NULL) {
      fprintf(stderr, "Can't open the file %s for input.\n", argv[3]);
      exit_code = 2;
    }
    else if ((outfile=fopen(argv[4], "w")) == NULL) {
      fprintf(stderr, "Can't open the file %s for output.\n", argv[4]);
      exit_code = 2;
    }
    else {
      ALIGN(0);
      fprintf(outfile, "char *%s[]={\n", argv[1]);
      while (fgets(buffer, LINE_BUF_SIZE+2, infile) != NULL) {
	if ((buf_ptr=strchr(buffer,'\n')) != NULL) *buf_ptr = '\0';
	++nmbr_lines;
	ALIGN(INDENT_LEVEL);
	fprintf(outfile, "\"");
	column_width = column_no + INDENT_LEVEL + 1;
	for (buf_ptr=buffer; c = *buf_ptr; buf_ptr++) {
	  ++nmbr_chars;
	  column_width += 2;
	  if	  (c == '\a') fprintf(outfile, "\\a");
	  else if (c == '\b') fprintf(outfile, "\\b");
	  else if (c == '\f') fprintf(outfile, "\\f");
	  else if (c == '\r') fprintf(outfile, "\\r");
	  else if (c == '\t') fprintf(outfile, "\\t");
	  else if (c == '\v') fprintf(outfile, "\\v");
	  else if (c == '\\') fprintf(outfile, "\\\\");
	  else if (c == '\"') fprintf(outfile, "\\\"");
	  else if (c == '\'') fprintf(outfile, "\\\'");
	  else if (c == '\?') fprintf(outfile, "\\\?");
	  else if (!isprint(c)) {
	    column_width += 2;
	    fprintf(outfile, "\\x%02X", (unsigned) c);
	  }
	  else {
	    ++nmbr_norms;
	    --column_width;
	    putc((int)c, outfile);
	  }
	}
	fprintf(outfile, "\",\n");
	column_width += 2;
	if (column_width > max_column_width) max_column_width = column_width;
      }
      ALIGN(INDENT_LEVEL);
      fprintf(outfile, "NULL\n");
      ALIGN(0);
      fprintf(outfile, "};\n");
      printf("\nProcessing of the file %s complete.\n\n", argv[3]);
      printf("\tNumber of lines found in input file: %ld lines.\n",
	     nmbr_lines);
      printf("\tNumber of characters to be compiled: %ld characters.\n",
	     nmbr_chars);
      printf("\tNumber of special  characters found: %ld characters.\n",
	     nmbr_chars - nmbr_norms);
      printf("\tLongest  line in  array declaration: %d columns.\n",
	     max_column_width - 1);
    }
    exit(exit_code);
  }
  
