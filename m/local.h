/*****************************************************************************
FILE          : local.h
LAST REVISION : January 1991
SUBJECT       : Local standard language environment.

     (C) Copyright 1990 by Peter Chapin

     This file contains a number of typedefs, #defines, etc that are useful
in any program. These symbols are maximally useful if they are #included in
every file of a project. This allows you to use them as if they were built
into the language. Be conservative when adding things to this file!

     Please send comments and bug reports to

         Peter Chapin
         P.O. Box 317
         Randolph Center, VT 05061

*****************************************************************************/

#ifndef LOCAL_H
#define LOCAL_H

/* Define the boolean type. */
typedef enum {
  TRUE     = 1,
  FALSE    = 0,
  YES      = 1,
  NO       = 0,
  ON       = 1,
  OFF      = 0,
  NO_ERROR = 1,
  ERROR    = 0
  } boolean;

/* Better names for controlling scope. */
#define PUBLIC
#define PRIVATE static

/* The following are relavant on MS-DOS machines. */
#define NEAR  near
#define FAR   far
#define HUGE  huge

/* New types that are the specified size. */
typedef unsigned char byte;         /*  8 bits.            */
typedef unsigned int  word;         /* 16 bits.            */
typedef unsigned long long_word;    /* 32 bits (Motorola). */
typedef unsigned long double_word;  /* 32 bits (Intel).    */

/* New control structures. */
#define loop for (;;)

#endif

