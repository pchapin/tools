/*****************************************************************************
FILE          : ansiscrn.c
LAST REVISION : September 1991
PROGRAMMER    : Peter Chapin

This file contains the implementation of a simple screen handling module.
The module uses ANSI escape sequences. It is therefore highly reliable and
portable, but slow.

I welcome comments and bug reports. I can be reached as in:

     Peter Chapin
     P.O. Box 317
     Randolph Center, VT 05061
*****************************************************************************/

#include "local.h"
#include <stdio.h>
#include "standard.h"
#include "ansiscrn.h"

#define ESC "\033"    /* ASCII escape character */

void Clear_Screen (void)           { printf(ESC "[2J");            }
void Clear_To_EOL (void)           { printf(ESC "[K");             }
void Set_Color    (int Color)      { printf(ESC "[%dm", Color);    }
void Bold_On      (void)           { printf(ESC "[1m");            }
void Blink_On     (void)           { printf(ESC "[5m");            }
void Reverse_On   (void)           { printf(ESC "[7m");            }
void Reset_Screen (void)           { printf(ESC "[0m");            }

void Position_Cursor(int Row, int Column)  { printf(ESC "[%d;%dH", Row, Column); }

void Cursor_Up       (int Count)   { printf(ESC "[%dA", Count);    }
void Cursor_Down     (int Count)   { printf(ESC "[%dB", Count);    }
void Cursor_Forward  (int Count)   { printf(ESC "[%dC", Count);    }
void Cursor_Backward (int Count)   { printf(ESC "[%dD", Count);    }

void Save_Cursor_Position   (void) { printf(ESC "[s");             }
void Restore_Cursor_Position(void) { printf(ESC "[u");             }
