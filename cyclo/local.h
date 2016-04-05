/*****************************************************************************
FILE          : local.h
LAST REVISION : May 1993
SUBJECT       : Local compilation settings.
PROGRAMMER    : Peter Chapin

This file contains local settings that apply to the current program. If the
program is moved to a different implementation, these settings must be
reviewed and changed if necessary. Ideally, this is the only file that
would need to be modified when a program is moved to a new implementation.
Conditional compilation directives based on the symbols defined in this
file (and in standard.h) should allow all other source files to be
implementation independent.

See standard.doc for more information.

     Please send comments and bug reports to

     VTC^3
     c/o Peter Chapin
     Vermont Technical College
     Randolph Center, VT 05061
     chapinp@vscnet.bitnet
*****************************************************************************/

#ifndef LOCAL_H
#define LOCAL_H

/* Choose your operating system. Select only one. */
#define MSDOS   1
#define UNIX    0
#define VMS     0
#define OS2     0

/* Choose your compiler. */
#define BORLAND 1

/* Verify that the following types have the right sizes on your system. */
typedef unsigned char  byte;         /*  8 bits.            */
typedef unsigned short word;         /* 16 bits.            */
typedef unsigned long  long_word;    /* 32 bits (Motorola). */
typedef unsigned long  double_word;  /* 32 bits (Intel).    */

/* Set the following flags to 1 to activate the following features. */
/* #define DEBUG    0 */  /* Compile in debug code and assertions. */
/* #define TEST     0 */  /* Compile in test code. */
/* #define MYMALLOC 0 */  /* Compile in the My_Malloc() family of functions
                             so calls to malloc(), etc, cannot fail. */

#endif

