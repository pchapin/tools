/*****************************************************************************
FILE          : scrtools.h
LAST REVISION : February 1991
SUBJECT       : Public interface to screen tools package.

     (C) Copyright 1991 by Peter Chapin

     Please send comments and bug reports to

         Peter Chapin
         P.O. Box 317
         Randolph Center, VT 05061

*****************************************************************************/

#ifndef SCRTOOLS_H
#define SCRTOOLS_H

#include "scr.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------*/
/*           Function Prototypes           */
/*-----------------------------------------*/
extern void ScrCenter(int r, int c, int w, char *text, int attr);
extern void ScrDrawBox(int r, int c, int w, int h, int box_type, int attr);
extern int  ScrGets(int row, int col, char *buffer, int length, int attr);

#ifdef __cplusplus
}
#endif

/*---------------------------------------------------*/
/*           Values for Manifest Constants           */
/*---------------------------------------------------*/
#define SCR_DBL_LINE        0   /* Box types for ScrDrawBox() */
#define SCR_SNGL_LINE       1
#define SCR_DARK_GRAPHIC    2
#define SCR_LIGHT_GRAPHIC   3
#define SCR_SOLID           4
#define SCR_ASCII           5
#define SCR_BLANK_BOX       6

#endif
