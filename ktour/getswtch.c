/****************************************************************************
FILE          : getswtch.c
LAST REVISION : May 1993
AUTHOR        : Peter Chapin
SUBJECT       : Functions to read command line switches.

(C) Copyright 1985, 1987 Allen I Holub. All rights reserved. This program
may be copied for personal, non-profit use only.

Information on this program can be found in the book "C chest and Other C
Treasures" by Allen Holub. (C) 1987. M & T Books. Pages 41-51.

------------- THESE CHANGES ARE NOT IN THE TEXT -------------

November 22, 1987

Extensive modifications. I removed some features and simplified others. I
also changed many of the identifier names to reflect the usage better (in
my opinion).
                                                               Peter Chapin

Please send comments or bug reports to

     Peter Chapin
     Electrical and Computer Engineering Technology
     Vermont Technical College
     Randolph Center, VT 05061
     Peter.Chapin@vtc.vsc.edu
****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "local.h"
#include "getswtch.h"

#define SWTCHAR '-'
#define ERRMSG  "Illegal switch <%c>. Legal switchs are:\n\n"

/*=========================================*/
/*           Function Prototypes           */
/*=========================================*/

PRIVATE char  *get_value(SWTCH *, char *);
PRIVATE SWTCH *find_swtch(char, SWTCH *, int);

/*----------------------------------------------------------*/

PRIVATE
char *get_value(SWTCH *swtchp, char *linep)
  {
    ++linep;
    switch (swtchp->type) {

      case INT_SWTCH:
        *swtchp->value = atoi(linep);
        while (isdigit(*linep)) linep++;
        break;

      case BIN_SWTCH:
        *swtchp->value = 1;
        break;

      case CHR_SWTCH:
        *swtchp->value = (int) *linep++;
        break;

      case STR_SWTCH:
        *(swtchp->pvalue) = linep;
        linep = "";
        break;

      default:
        fprintf(stderr,
          "INTERNAL ERROR: get_swtchs() is using a bad switch type.\n");
        break;


    }
    return (linep);
  }

/*----------------------------------------------------------*/

PRIVATE
SWTCH *find_swtch(char c, SWTCH *tabp, int tabsize)
  {
    for (; --tabsize>=0; tabp++)
      if (tabp->name == c)
        return (tabp);
    return (NULL);
  }

/*----------------------------------------------------------*/

PUBLIC
void pr_usage(SWTCH *tabp, int tabsize, FILE *out_file)
  {
    for (; --tabsize>=0; tabp++) {
      switch (tabp->type) {

        case INT_SWTCH:
          fprintf(out_file,
            "-%c<num> %-40s\n",tabp->name,tabp->hlpmsg);
          break;

        case BIN_SWTCH:
          fprintf(out_file,
            "-%c      %-40s\n",tabp->name,tabp->hlpmsg);
          break;

        case CHR_SWTCH:
          fprintf(out_file,
            "-%c<c>   %-40s\n",tabp->name,tabp->hlpmsg);
          break;

        case STR_SWTCH:
          fprintf(out_file,
            "-%c<str> %-40s\n",tabp->name,tabp->hlpmsg);
          break;

        default:
          fprintf(stderr,
            "INTERNAL ERROR: Bad switch type passed to pr_usage().\n");
          break;
      }
    }
    return;
  }

/*----------------------------------------------------------*/

PUBLIC
int get_swtchs(int argc, char **argv, SWTCH *tabp, int tabsize)
  {
    register int   new_argc;
    register char  **new_argv;
    register char  *p;
    register SWTCH *swtchp;

    new_argc = 1;
    for (new_argv=++argv; --argc>0; argv++) {
      if (**argv != SWTCHAR) {
        *new_argv++ = *argv;
        new_argc++;
      }
      else {
       p = (*argv) + 1;
       while (*p) {
         if ((swtchp=find_swtch(*p, tabp, tabsize))!=NULL)
           p = get_value(swtchp, p);
         else {
           fprintf(stderr,ERRMSG,*p);
           pr_usage(tabp, tabsize, stderr);
           exit(EBADSW);
         }
       }
     }
   }
   return (new_argc);
 }
