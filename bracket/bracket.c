/****************************************************************************
FILE          : bracket.c
LAST REVISION : July 1993
SUBJECT       : Program to check brace/bracket matching in C/C++ source code.
PROGRAMMER    : Peter Chapin

This program accepts a C/C++ language source file through its standard
input and examins it for problems with {}, (), and []. It will print the
program, with messages, to its standard output.

The program prints a message after each '}' that closes off a function (or
file scoped struct declaration or initialized array). It prints error
messages for several type of mismatched {}, (), and [] errors. The messages
are printed as close to the source of the error as possible (the actual
error will always be above the message).

The program notices unclosed comments, literals, or {...} pairs at the end
of the file. The message produced by the program may be more useful than
the "Unexpected EOF" message produced by many compilers.

The program correctly disregards material inside of comments or literals.

Please send comments or bug reports to

     Peter Chapin
     P.O. Box 375
     Randolph Center, VT 05061
     chapinp@vscnet.bitnet
****************************************************************************/

#include "local.h"
#include <stdio.h>
#include "standard.h"
#include "ansiscrn.h"
#include "scanners.h"

/*=========================================*/
/*           Function Defintions           */
/*=========================================*/

/*--------------------------------------------------------------------------
int grab_char(void);

This function is used by the CmtScan module to extract characters from the
input file. In this program, characters are simply taken from stdin.
--------------------------------------------------------------------------*/

int grab_char(void)
  {
    return getchar();
  }

/*--------------------------------------------------------------------------
The error handling functions print the error messages of the program. The
messages are printed "in line" with the outputed material so that they are
close to the source of the error. ANSI escape sequences are used to display
the messages in interesting attributes so they stand out.
--------------------------------------------------------------------------*/

void error(char *message)
  {
    Bold_On(); Blink_On();
    printf(" <= ERROR: %s ", message);
    Reset_Screen();
  }

void warning(char *message)
  {
    Bold_On();
    printf(" <= WARNING: %s ", message);
    Reset_Screen();
  }

void eof_error(char *message)
  {
    Bold_On(); Blink_On();
    printf("\n\nERROR: %s", message);
    Reset_Screen();
  }

/*==================================*/
/*           Main Program           */
/*==================================*/

/*--------------------------------------------------------------------------
int main(void);

The main function extracts characters from the CmtScan module (which
analysizes the input stream for C/C++ comments and literals). If the
character is in the normal code region, it is checked for the various
interesting characters ("{}()[]"). If an error is found, corrective action
is often taken to prevent a "cascade" of error messages.
--------------------------------------------------------------------------*/

int main(void)
  {
    int ch;             /* Character obtained after filtering. */
    int brace = 0;      /* {...} level.                        */
    int parens = 0;     /* (...) level.                        */
    int bracket = 0;    /* [...] level.                        */

    CmtScanInit(grab_char);
    while ((ch=CmtScanGetChar()) != EOF) {
      putchar(ch);
      if (!Comment && !DoubleQuote && !SingleQuote) {
        switch (ch) {

          case '{':
            brace++;
            if (parens > 0) {
              error("Extra (");
              parens = 0;
            }
            if (bracket > 0) {
              error("Extra [");
              bracket = 0;
            }
            break;

          case '}':
            brace--;
            if (brace == -1) {
              error("Extra }");
              brace = 0;
            }
            if (parens > 0) {
              error("Extra (");
              parens = 0;
            }
            if (bracket > 0) {
              error("Extra [");
              bracket = 0;
            }
            if (brace == 0) warning("Brace level now at zero");
            break;

          case '(':
            parens++;
            break;

          case ')':
            parens--;
            if (parens == -1) {
              error("Extra )");
              parens = 0;
            }
            break;

          case '[':
            bracket++;
            break;

          case ']':
            bracket--;
            if (bracket == -1) {
              error("Extra ]");
              bracket = 0;
            }
            break;
        }
      }
    }

    if (brace > 0)        eof_error("Unclosed {...}");
    if (OpenCommentError) eof_error("Unclosed comments");
    if (DoubleQuote)      eof_error("Unclosed string literal");
    if (SingleQuote)      eof_error("Unclosed character literal");

    return 0;
  }
