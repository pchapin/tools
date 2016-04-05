/*****************************************************************************
FILE          : scrdrawb.c
LAST REVISION : December 1990
SUBJECT       : Function which draws a box on the screen.

     (C) Copyright 1990 by Peter Chapin

     Please send comments and bug reports to

         Peter Chapin
         P.O. Box 317
         Randolph Center, VT 05061

*****************************************************************************/

#include "local.h"
#include "scrtools.h"

struct boxchars {
  char horz;
  char vert;
  char upper_left;
  char upper_right;
  char lower_left;
  char lower_right;
  };

PRIVATE
struct boxchars box_defs[]={
  { 205, 186, 201, 187, 200, 188 },  /* Double lines.  */
  { 196, 179, 218, 191, 192, 217 },  /* Single lines.  */
  { 177, 177, 177, 177, 177, 177 },  /* Dark graphic.  */
  { 176, 176, 176, 176, 176, 176 },  /* Light graphic. */
  { 219, 219, 219, 219, 219, 219 },  /* Solid.         */
  {  45, 124,  43,  43,  43,  43 },  /* ASCII.         */
  {  32,  32,  32,  32,  32,  32 }   /* Blank.         */
  };

PUBLIC
void ScrDrawBox(
    int row, int col, int width, int height, int box_type, int attr
    )
  {
    int i;

    for (i=col+1; i<col+width-1; i++) {
      ScrPrint(row, i, &box_defs[box_type].horz, attr, 1);
      ScrPrint(row+height-1, i, &box_defs[box_type].horz, attr, 1);
    }
    for (i=row+1; i<row+height-1; i++) {
      ScrPrint(i, col, &box_defs[box_type].vert, attr, 1);
      ScrPrint(i, col+width-1, &box_defs[box_type].vert, attr, 1);
    }
    ScrPrint(row, col,
             &box_defs[box_type].upper_left, attr, 1);
    ScrPrint(row, col+width-1,
             &box_defs[box_type].upper_right, attr, 1);
    ScrPrint(row+height-1, col,
             &box_defs[box_type].lower_left, attr, 1);
    ScrPrint(row+height-1, col+width-1,
             &box_defs[box_type].lower_right, attr, 1);
    return;
  }
