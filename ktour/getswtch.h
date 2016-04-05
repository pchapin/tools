/****************************************************************************
FILE          : getswtch.h
LAST REVISION : May 1993
AUTHOR        : Peter Chapin
SUBJECT       : Module for reading command line switches.

This header file is used in connection with getswtch.c. Getswtch.c is a
command line switch processor copyrighted by Allen I Holub. See page 41 in
"C Chest" for more details.

Please send comments or bug reports to

     Peter Chapin
     Electrical and Computer Engineering Technology
     Vermont Technical College
     Randolph Center, VT 05061
     Peter.Chapin@vtc.vsc.edu
****************************************************************************/

#ifndef GETSWTCH_H
#define GETSWTCH_H

#define INT_SWTCH  0
#define BIN_SWTCH  1
#define CHR_SWTCH  2
#define STR_SWTCH  3

#define EBADSW     1  /* Errorlevel for bad switch found. */

typedef struct {
  char name;       /* Name of the command line switch.    */
  int  type;       /* Switch type (see above).            */
  int  *value;     /* Pointer to variable.                */
  char **pvalue;   /* Pointer to variable for STR_SWTCH.  */
  char *hlpmsg;    /* Pointer to help message.            */
  } SWTCH;

#ifdef __cplusplus
extern "C" {
#endif

extern void pr_usage(SWTCH *, int, FILE *);
extern int  get_swtchs(int, char **, SWTCH *, int);

#ifdef __cplusplus
}
#endif

#endif

