/*****************************************************************************
FILE          : get_key.c
LAST REVISION : December 1990
SUBJECT       : General purpose keystroke reading for the IBM PC.

     (C) Copyright 1990 by Peter Chapin

     This file contains a function which can read any key on the IBM PC
keyboard. It returns values greater than 255 for the special keys on the
keyboard. This function does not echo keystrokes. If ^C is pressed while
this function is waiting for a keystroke, it simply returns CNTRL_C (No
signal generated).

     Please send comments and bug reports to

         Peter Chapin
         P.O. Box 317
         Randolph Center, VT 05061

*****************************************************************************/

#include <dos.h>
#include "get_key.h"

int get_key()
  {
    int ch;

    /* Normal key codes */
    if ((ch = bdos(0x7, 0, 0) & 0x00ff) != '\0') return (ch);
    return ((bdos(0x7, 0, 0) & 0x00ff) + XF);
  }
