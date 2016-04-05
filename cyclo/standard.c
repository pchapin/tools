/****************************************************************************
FILE          : standard.c/standard.cpp
LAST REVISION : May 1993
SUBJECT       : Implementation of standard support functions.
PROGRAMMER    : Peter Chapin

These functions are below all local libraries (those compiled from source
code), but above third party libraries (those provided in object code) and
the standard library in the chain of abstraction. They are really language
support functions.

This file can be compiled in either C or C++. That is, by simply renaming
it to STANDARD.C, it can be used to support C programs. Note that
STANDARD.H can appear in either C or C++ code. All functions (except
::operator new()) are explicitly declared extern "C" during a C++ compile.
This file has to be compiled as a C++ file in a C++ program so that
::operator new() is suitably handled.

The function Memory_Panic(), which must be supplied elsewhere, must be a C
function.

Please send comments or bug reports to

     VTC^3
     c/o Peter Chapin
     Vermont Technical College
     Randolph Center, VT 05061
     chapinp@vscnet.bitnet
****************************************************************************/

#include "local.h"
#include <stdlib.h>   /* Needed for declarations of memory managment functions. */
#include <string.h>
#include "standard.h"

/* To prevent recursive calls to My_Malloc(), etc below, I must undefine
     the following symbols that are #defined in standard.h. */
#undef malloc
#undef calloc
#undef realloc
#undef free

#ifdef __cplusplus
#define EXTERNAL extern "C"
#else
#define EXTERNAL extern
#endif

EXTERNAL void Memory_Panic(void);
  /* The function above must be written by the application programmer. It
       must NOT do anything, directly or indirectly, that would cause for
       additional memory to be allocated. If this function returns, the
       memory allocation will be attempted again. Thus Memory_Panic() could
       attempt to free up some memory. */

/* My_Malloc() never returns NULL. If it fails to find memory on the malloc()
     heap, it calls Memory_Panic(), an application call-back function. */

#ifndef MYMALLOC

/* If the caller isn't using My_Malloc(), etc, compile in this version of
     Memory_Panic(). This is so there are no link errors when the functions
     below are compiled in (even if not needed). If the caller is using
     My_Malloc(), this function won't be compiled in, but the caller should
     be providing their own... */

void Memory_Panic(void)
  {
    return;
  }

#endif

EXTERNAL void *My_Malloc(size_t Size)
  {
    void *New_Space;

    do {
      New_Space = malloc(Size);
      if (!New_Space) Memory_Panic();
    } while (!New_Space);

    return New_Space;
  }

/* My_Calloc() has the same semantics as calloc() except that it cannot
     fail from the point of view of the application. */

EXTERNAL void *My_Calloc(size_t Nmbr_Items, size_t Size)
  {
    size_t  Bytes     = Nmbr_Items * Size;
    void   *New_Space = My_Malloc(Bytes);

    memset(New_Space, 0, Bytes);
    return New_Space;
  }

/* And so forth for My_Realloc(). */

EXTERNAL void *My_Realloc(void *Old_Space, size_t New_Size)
  {
    void *New_Space;

    do {
      New_Space = realloc(Old_Space, New_Size);
      if (!New_Space) Memory_Panic();
    } while (!New_Space);

    return New_Space;
  }

/* There is no compelling reason to provide my own free(). However, for
     symmetery, I will do so. Besides, by putting counters in here and
     in My_Malloc(), I should be able to do some rudimentary checking for
     memory leaks. (Number of My_Free() calls should equal the number of
     My_Malloc() calls. */

EXTERNAL void My_Free(void *p)
  {
    free(p);
  }

#ifdef __cplusplus

/* Let's be sure ::operator new() is implemented in terms of My_Malloc()
     so that new expressions cannot fail. */

void *operator new(size_t Size)
  {
    return My_Malloc(Size);
  }

/* And ::operator delete() is here for the same reasons as My_Free(). */

void operator delete(void *p)
  {
    My_Free(p);
  }

#endif

