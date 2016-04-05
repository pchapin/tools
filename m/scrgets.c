/*****************************************************************************
FILE          : scrgets.c
LAST REVISION : December 1990
SUBJECT       : Editing gets()

     (C) Copyright 1990 by Peter Chapin

     This file contains a function which allows for flexible user input.
The user is presented with a buffer to into which s/he can input
characters. The characters can be edited according to the special keys
intercepted below. If the user presses ESC, the original buffer contents
are restored. The function returns the key code of the key used to
terminate input.

     Please send comments and bug reports to

         Peter Chapin
         P.O. Box 317
         Randolph Center, VT 05061

*****************************************************************************/

#include <ctype.h>
#include <string.h>
#include "local.h"
#include "scr.h"
#include "scrtools.h"
#include "get_key.h"

PUBLIC
int ScrGets(int row, int col, char *buffer, int length, int attr)
  {
    static char    holding[128+1];
    auto   int     buffer_offset;
    auto   int     key;
    auto   boolean first_key = YES;
    auto   int     i;

    buffer[length] = '\0';              /* In case initial string too long.  */
    strcpy(holding, buffer);            /* Save in case user wants to abort. */
    buffer_offset = strlen(buffer);

    do {
      /* Print as much from the buffer as there is right now and position cursor. */
      ScrPrint(row, col, buffer, attr, strlen(buffer));
      ScrSetCursorPos(row, col+buffer_offset);

      /* Clear the rest (to show size of buffer in attr). */
      if (length-strlen(buffer) != 0) {
        ScrClear(row, col+strlen(buffer), length-strlen(buffer), 1, attr);
      }

      /* Handle editing. */
      switch (key = get_key()) {

        case K_ESC:
          strcpy(buffer, holding);      /* Restore original string. */
          break;

        case K_HOME:
          buffer_offset = 0;
          break;

        case K_END:
          buffer_offset = strlen(buffer);
          break;

        case K_RIGHT:
          if (buffer_offset != strlen(buffer)) buffer_offset++;
          break;

        case K_LEFT:
          if (buffer_offset != 0) buffer_offset--;
          break;

        case K_BACKSPACE:
          if (buffer_offset != 0) {
            for (i=buffer_offset; (buffer[i-1] = buffer[i]) != '\0'; i++) ;
            buffer_offset--;
          }
          break;

        case K_DEL:
          if (buffer_offset != strlen(buffer)) {
            for (i=buffer_offset; (buffer[i] = buffer[i+1]) != '\0'; i++) ;
          }
          break;

        default:

          /* Ignore other special keys. */
          if (key < 128  &&  isprint(key)) {

            /* If first key is a letter, erase any initial buffer contents. */
            if (first_key) {
              *buffer = '\0';
              buffer_offset = 0;
            }

            /* Do stuff only if there's space. */
            if (strlen(buffer) < length) {
              i = strlen(buffer);
              do {
                buffer[i+1] = buffer[i];
                i--;
              } while (i >= buffer_offset);
              buffer[buffer_offset++] = key;
            }
          }
          break;

      }   /* End of switch */

      /* Turn off first key flag. */
      first_key = NO;

    } while (key != K_RETURN  &&  key != K_ESC  &&  key != K_TAB);

    /* Move cursor to start of where buffer was printed. */
    ScrSetCursorPos(row, col);

    /* Return either K_RETURN or K_ESC or K_TAB. */
    return key;
  }
