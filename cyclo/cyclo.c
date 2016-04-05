/*==============================================================*/
/* cyclo.c                                                      */
/*                                                              */
/* Program to measure the cyclomatic complexity of a C program. */
/*==============================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "standard.h"
#include "scanners.h"

/*=================================*/
/*	     Global Data	   */
/*=================================*/

FILE	*infile;
boolean  in_funct;
int	 lines_in_funct=0;

/*==========================================*/
/*	     Function Definitions	    */
/*==========================================*/

int scanstr(char *buffer, char *search)
  {
    char *buf_ptr=buffer;
    int  n_times=0;

    while ((buf_ptr=strstr(buf_ptr, search)) != NULL) {
      n_times++;
      buf_ptr++;   /* Bump up so the next match is different. */
    }
    return (n_times);
  }

/*--------------------------*/

#define DUMP_STATS   257  /* Outside the range of anybody's character */
#define MAX_BUF_SIZE 256

void compile_stats(int ch)
  {
    static char buffer[MAX_BUF_SIZE+1];
    static char *buf_ptr=buffer;
    static int  buf_length=0;
    static int	cyclo_number=1;
    static int	n_cases=0;
    
    switch(ch) {
      case DUMP_STATS:
	printf("\n\n/********** End of Function **********\n");
	printf("\nFunction contains %d lines.", lines_in_funct);
	printf("\nTotal cyclomatic number = %d.", cyclo_number);
	printf("\nNumber of \"case\" statements = %d.\n", n_cases);
	printf("\n*************************************/\n");
	lines_in_funct = 0;
	cyclo_number = 1;
	n_cases = 0;
        break;
        
      default:
        if (ch != '\n') {
          if (buf_length < MAX_BUF_SIZE) {
            *buf_ptr++ = ch;
            buf_length++;
          }
        }
        else {
          *buf_ptr = '\0';
	  cyclo_number += scanstr(buffer, "if ");
	  cyclo_number += scanstr(buffer, "if(");
	  cyclo_number += scanstr(buffer, "for ");
	  cyclo_number += scanstr(buffer, "for(");
	  cyclo_number += scanstr(buffer, "while ");
	  cyclo_number += scanstr(buffer, "while(");
	  cyclo_number += scanstr(buffer, "case ");
	  n_cases      += scanstr(buffer, "case ");
	  cyclo_number += scanstr(buffer, "&&");
	  cyclo_number += scanstr(buffer, "||");
	  buf_ptr = buffer;
	  buf_length = 0;
        }
        break;
    }
    return;
  }
  
/*--------------------------*/

void find_funct(int ch)
  {
    static int bracket_level=0;
    static int looking_for_semi=NO;
    static char s[]="struct";
    static char *sptr=s;

    if (bracket_level == 0  &&	ch == *sptr) {
      sptr++;
      if (*sptr == '\0') looking_for_semi = YES;
    }
    else if (bracket_level == 0 && ch != *sptr) sptr = s;

    if (bracket_level == 0  &&	ch == '=') looking_for_semi = YES;
    if (bracket_level == 0  &&	ch == ';') looking_for_semi = NO;

    if (bracket_level >= 1  &&	looking_for_semi == NO) {
      in_funct = YES;
      compile_stats(ch);
    }
        
    if (ch == '{') {
      bracket_level++;
    }
    else if (ch == '}') {
      bracket_level--;
      if (bracket_level == 0  &&  looking_for_semi == NO) {
	in_funct = NO;
	compile_stats(DUMP_STATS);
      }
    }
    return;
  }

/*--------------------------*/

int grab_char(void)
  {
    int ch;

    ch = getc(infile);
    if (ch == '\n'  &&	in_funct) lines_in_funct++;
    if (ch != EOF) putchar(ch);
    return ch;
  }

/*==================================*/
/*	     Main Program	    */
/*==================================*/

main(int argc, char *argv[])
  {
    int   inchar;
    int   exit_code=0;

    fprintf(stderr, "CYCLO  Version 1.0  (%s)\n", __DATE__);
    fprintf(stderr, "Public Domain software by Peter Chapin\n\n");
    
    if (argc != 2) {
      fprintf(stderr, "Measures the cyclomatic complexity of C programs.\n");
      fprintf(stderr, "Usage: CYCLO infile.c [>outfile.txt]\n");
      exit_code = 2;
    }
    else if ((infile=fopen(argv[1], "r")) == NULL) {
      fprintf(stderr, "ERROR: Can't open %s for input.\n", argv[1]);
      exit_code = 1;
    }
    else {
      CmtScanInit(grab_char);
      while ((inchar=CmtScanGetChar()) != EOF) {
	if (!DoubleQuote && !SingleQuote) find_funct(inchar);
      }
    }
    return exit_code;
  }
