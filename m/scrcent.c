/*****************************************************************************
FILE          : scrcent.c
LAST REVISION : December 1990
SUBJECT       : Function to center text on a row of the screen.

     (C) Copyright 1990 by Peter Chapin

     Please send comments and bug reports to

	 Peter Chapin
	 P.O. Box 317
	 Randolph Center, VT 05061

*****************************************************************************/

#include <string.h>
#include "local.h"
#include "scrtools.h"

PUBLIC
void ScrCenter(int row, int col, int width, char *text, int attr)
  {
    int offset;

    if (width < strlen(text)) {
      ScrPrint(row, col, text, attr, width);
    }
    else {
      offset = (width - strlen(text))/2;
      ScrPrint(row, col+offset, text, attr, strlen(text));
    }
    return;
  }
