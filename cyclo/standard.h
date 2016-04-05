/****************************************************************************
FILE          : standard.h
LAST REVISION : May 1993
SUBJECT       : Standard declarations.
PROGRAMMER    : Peter Chapin

This file introduces a number of standard declarations and macros that can
apply to all programs. If you #include this file into every program, you
can use these symbols as if they were built into the language. In general
the order of #includes should be:

#include "local.h"    // To get at OS and compiler vendor symbols.
#include <stdlib.h>   // The standard library.
#include <other.h>    // Third party libraries.
#include "standard.h" // So all other headers can use these symbols.
#include "project.h"  // All headers produced locally.

Please send comments or bug reports to

     VTC^3
     c/o Peter Chapin
     Vermont Technical College
     Randolph Center, VT 05061
     chapinp@vscnet.bitnet
****************************************************************************/

#ifndef STANDARD_H
#define STANDARD_H

#include <stddef.h>

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

/* Better names for controlling scope. Use PUBLIC or PRIVATE on all
     global data objects and functions. */
#define PUBLIC
#define PRIVATE static

/* The following are relavant on MS-DOS machines. Other machines make
     no distinction. */
#ifdef  MSDOS
#define NEAR  near
#define FAR   far
#define HUGE  huge
#else
#define NEAR
#define FAR
#define HUGE
#endif

/* New control structures. */
#define loop     for (;;)
#define repeat   do
#define until(x) while (!(x))

/* Control for debugging. */
#ifdef  DEBUG
#undef  NDEBUG  /* Be sure assertions are in force. */
#define D(x) x
#else
#define NDEBUG  /* Strip out assertions. */
#define D(x)
#endif
#include <assert.h>

/* Control for testing. */
#ifdef  TEST
#define T(x) x
#else
#define T(x)
#endif

/* The following stuff pertains to memory management. */
#ifdef MYMALLOC

/* These functions have the same semantics as the standard library functions
     except that they never return an error indication. Instead, if they
     can't find memory they call the function void Memory_Panic(void). */

#ifdef __cplusplus
extern "C" {
#endif
extern void *My_Malloc(size_t);
extern void *My_Calloc(size_t, size_t);
extern void *My_Realloc(void *, size_t);
extern void  My_Free(void *);
extern void  Memory_Panic(void);
#ifdef __cplusplus
}
#endif

/* Macros that redirect calls to memory management functions to my code. */
#define malloc(size)              My_Malloc(size)
#define calloc(size1, size2)      My_Calloc(size1, size2)
#define realloc(pointer, size)    My_Realloc(pointer, size)
#define free(pointer)             My_Free(pointer)

#ifdef __cplusplus
/* If this is a C++ compile, let's override the global operator new() so
     that it will also call Memory_Panic() if it can't find the memory. */
void *operator new(size_t);
void  operator delete(void *);
#endif

#endif

#endif

