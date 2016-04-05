/*****************************************************************************
FILE          : cmtscan.c
LAST REVISION : December 1990
SUBJECT       : C/C++ comment stripping functions.
PROGRAMMER    : Peter Chapin

This module scans C/C++ source text and recognizes comments, string
literals, and character literals. This process is a bit more complicated
than it sounds because comment delimiters found inside string literals (or
character literals) must be ignored. Escape sequences inside string
literals must be noted so that the end of the literal is properly detected.

To see how this module is used, I recommend looking at the file cmttst.c. I
will note some of the important points below.

1.   CmtScanInit()  must  be  called  before  the module is used. This
     function  registers  the  character  getting  function  with  the
     module. CmtScanGetChar()  will use the function pointed at by the
     parameter to actually get characters out of the source file.

2.   CmtScanGetChar() will return EOF when the  end of  the input file
     is  reached.  At  that  time the global variable OpenCommentError
     will be set to  YES if  the input  file has  an unclosed comment.
     Note that if the last character of the file is a /, this does not
     constitute an OpenCommentError.

3.   The global variable Comment will be set to YES if  the characters
     that are  being returned  are part  of a  comment. the  /* and ?/
     sequences are considered part of a comment. Also, the // and new-
     line characters are considered part of a C++ style comment.

4.   The global  variables DoubleQuote  and SingleQuote will be set to
     YES if the characters that  are  being  returned  are  part  of a
     string literal  or character constant respectively. Note that the
     " or ' characters themselves are  considered part  of the literal
     or constant.

This module understands and correctly handles C and C++ style comments.

This module was intended to be used in the processing of ANSI conforming
source code. Strictly speaking, the input stream that this module processes
should already be free of trigraphs and backslash spliced lines. This
module should be used before any preprocessing actions are done. Also this
module provides a way to detect unclosed comments in a file (as per ANSI).

This module contains static variables. It is, therefor, not re-entrant and
cannot be used recursively (ie to process include files). Future versions
may fix this problem.

     Please send comments and bug reports to

     Peter Chapin
     P.O. Box 317
     Randolph Center, VT 05061

*****************************************************************************/

#include "local.h"
#include <stdio.h>
#include "standard.h"
#include "scanners.h"

/*=================================*/
/*           Global Data           */
/*=================================*/

#define BOGUS_VALUE  '\0'

PRIVATE int (*GetChar)(void);   /* Points at function that gets characters. */

enum state_values {
  Code, Start_Cmt, C_Cmt, Cpp_Cmt, End_Cmt, D_Quote, S_Quote, D_Esc, S_Esc
  };

PRIVATE enum state_values state;

PRIVATE      char         buffer[2];        /* Holds chars when '/' found.  */
PRIVATE      int          index;            /* Index into buffer[].         */
PRIVATE      int          buffer_count;     /* Number of characters in buf. */
PRIVATE      boolean      use_buffer=NO;    /* =YES when buffer active.     */

PUBLIC       boolean      Comment=NO;       /* =YES when inside comments.   */
PUBLIC       boolean      DoubleQuote=NO;   /* =YES when inside string.     */
PUBLIC       boolean      SingleQuote=NO;   /* =YES when inside char const. */
PUBLIC       boolean      OpenCommentError=NO;  /* =YES if unclosed cmts.   */

/*==========================================*/
/*           Function Definitions           */
/*==========================================*/

/*------------------------------------------------------------------------*/
/* int handle_eof(void);                                                  */
/*                                                                        */
/*           This function is called when  EOF  is  detected  in the      */
/*      input  stream.  It  returns  the  character  that is in turn      */
/*      returned by CmtScanGetChar().                                     */
/*           The main complication this  function deals  with occurs      */
/*      when the EOF is immediately after a '/' character. In such a      */
/*      case, the EOF is placed into the buffer and CmtScanGetChar()      */
/*      is instructed to take its characters from the buffer.             */
/*           Note that  repeated calls to CmtScanGetChar() after the      */
/*      end of the file is reached  will not  cause probles provided      */
/*      the  low  level  character  producing  function used by this      */
/*      module returns EOF consistantly.                                  */
/*------------------------------------------------------------------------*/

PRIVATE
int handle_eof(void)
  {
    int ret_value=EOF;

    if (state == Start_Cmt) {
      buffer[index] = EOF;
      buffer_count++;
      index = 0;
      use_buffer = YES;
      ret_value = BOGUS_VALUE;
    }
    else if (state == C_Cmt) {
      OpenCommentError = YES;
    }
    return ret_value;
  }

/*------------------------------------------------------------------------*/
/* void CmtScanInit(int (*input_fun)(void));                              */
/*                                                                        */
/*           This  function  is  used  to  register  the  low  level      */
/*      character producing function with the cmtscan module.             */
/*------------------------------------------------------------------------*/

PUBLIC
void CmtScanInit(int (*input_fun)(void))
  {
    GetChar = input_fun;
    return;
  }

/*------------------------------------------------------------------------*/
/* int CmtScanGetChar(void);                                              */
/*                                                                        */
/*           This function  implements a  finite state machine which      */
/*      recognizes C/C++ comments,  string  literals,  and character      */
/*      constants. The states are enumerated in enum state_values. A      */
/*      switch statement is used in connection with a state variable      */
/*      to control the transitions of the FSM.                            */
/*           The look  ahead problem  (due to the fact that comments      */
/*      are introduced  and closed  with two  characters) is handled      */
/*      with a  buffer. When  a potential  opening comment is found,      */
/*      this function loops again to try the next  character. If the      */
/*      slash is not due to a comment, characters are extracted from      */
/*      the buffer until the  buffer  is  exhausted.  Note  that the      */
/*      buffer will  never be  more than  two characters deep (first      */
/*      character always a '/'). This technique allows the EOF to be      */
/*      handled more gracefully if it arrives at an unexpected time.      */
/*------------------------------------------------------------------------*/

PUBLIC
int CmtScanGetChar(void)
  {
    int c;
    int ret_value=BOGUS_VALUE;

    do {
      if (use_buffer) {
        ret_value = buffer[index++];
        if (--buffer_count == 0) use_buffer = NO;
      }
      else if ((c=GetChar()) == EOF) {
        ret_value = handle_eof();
      }
      else switch (state) {
        case Code:              /* 'Normal' state. */
          Comment = NO;
          DoubleQuote = NO;
          SingleQuote = NO;
          if (c == '/') {
            index = buffer_count = 0;
            buffer[index++] = c;
            buffer_count++;
            state = Start_Cmt;
          }
          else if (c == '\"') {
            DoubleQuote = YES;
            ret_value = c;
            state = D_Quote;
          }
          else if (c == '\'') {
            SingleQuote = YES;
            ret_value = c;
            state = S_Quote;
          }
          else ret_value = c;
          break;

        case Start_Cmt:         /* A '/' has already been found. */
          Comment = YES;
          if (c == '/') state = Cpp_Cmt;
          else if (c == '*') state = C_Cmt;
          else {
            state = Code;
            Comment = NO;
          }
          buffer[index] = c;
          buffer_count++;
          index = 0;
          use_buffer = YES;
          break;

        case C_Cmt:             /* A '/*' has been found. */
          ret_value = c;
          if (c == '*') state = End_Cmt;
          break;

        case Cpp_Cmt:           /* A '//' has been found. */
          ret_value = c;
          if (c == '\n') state = Code;
          break;

        case End_Cmt:           /* A '*' has been found inside a C_Cmt. */
          ret_value = c;
          if (c == '/') state = Code;
            else if (c != '*') state = C_Cmt;
          break;

        case D_Quote:           /* A '"' has been found. */
          ret_value = c;
          if (c == '\\') state = D_Esc;
          else if (c == '\"') state = Code;
          break;

        case S_Quote:           /* A '\'' has been found. */
          ret_value = c;
          if (c == '\\') state = S_Esc;
          else if (c == '\'') state = Code;
          break;

        case D_Esc:             /* A '\\' has been found inside a string. */
          ret_value = c;
          state = D_Quote;
          break;

        case S_Esc:             /* A '\\' has been found inside char const. */
          ret_value = c;
          state = S_Quote;
          break;

        default:                /* Something went wrong. */
          fprintf(stderr,
            "\n\nINTERNAL ERROR: Bad state in CmtScanGetChar(): %d\n\n",
            (int) state
          );
          state = Code;
          ret_value = c;
          break;
      }
    } while (ret_value == BOGUS_VALUE);
    return ret_value;
  }
