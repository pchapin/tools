/*****************************************************************************
FILE	      : scr.c
LAST REVISION : March 1991
SUBJECT	      : Screen handling.

     (C) Copyright 1991 by Peter Chapin

This file contains the implementation of SCR screen handling module. This
module does direct video access on the IBM PC. It uses, naturally, IBM PC
specific code. The interface is portable.

     Please send comments and bug reports to

	 Peter Chapin
	 P.O. Box 317
	 Randolph Center, VT 05061

*****************************************************************************/

#include <stdlib.h>   /* Standard library headers.  */
#include <stddef.h>
#include <string.h>
#include <dos.h>      /* IBM PC specific materials. */
#include "local.h"    /* Local symbols.		    */
#include "scr.h"      /* Module specific materials. */

/*=================================*/
/*	     Global Data	   */
/*=================================*/

#define BYTES_PER_ROW	   160	/* Number of bytes in one row of video RAM. */
#define BYTES_PER_PAGE	  4096	/* Number of bytes in one video page.	    */

#define VIDEO_MONO	     0	/* Symbolic names for the only two video modes supported. */
#define VIDEO_COLOR	     1

#define BIOS_VIDEO	  0x10	/* INT number for BIOS.	  */
#define SET_CRTMODE	     0	/* BIOS function numbers. */
#define SET_CURSORTYPE	     1
#define SET_CURSORPOS	     2
#define GET_CURSORPOS	     3
#define SET_PAGE	     5
#define SCROLL_UP	     6
#define SCROLL_DOWN	     7
#define GET_CRTMODE	    15

PRIVATE unsigned  Scr_Segment = 0xB800;	       /* Segment address of video RAM.	  */
PRIVATE int	  Video_Mode  = VIDEO_COLOR;   /* Current video mode.		  */
PRIVATE int	  Is_Init     = NO;	       /* =YES if ScrInitialize() called. */
PRIVATE int	  Page_Nmbr   = 0;	       /* Currently active video page.	  */

/*======================================*/
/*	     Public Functions		*/
/*======================================*/

/*----------------------------------------------------------------------------
void ScrInitialize(void);

The following function initializes this module. It should be called before
any other functions from this module are called. It can be called several
times with no ill effects.

This function will force the video mode to an appropriate one if necessary.
It should remember the old video mode for later restoration, but it
currently doesn't do that.
----------------------------------------------------------------------------*/

PUBLIC
void ScrInitialize(void)
  {
    union REGS	r;

    /* Return at once if already initialized. */
    if (Is_Init) return;

    /* Learn what video mode is currently being used. */
    r.h.ah = (byte) GET_CRTMODE;
    int86(BIOS_VIDEO, &r, &r);

    /* If it's mode 7 (monochrome), adjust our records. */
    if (r.h.al == 7) {
      Scr_Segment = 0xB000;
      Video_Mode = VIDEO_MONO;
    }

    /* Otherwise, force mode 3 (25x80 color). */
    else if (r.h.al != 3) {
      r.h.ah = (byte) SET_CRTMODE;
      r.h.al = 3;
      int86(BIOS_VIDEO, &r, &r);
    }

    /* In any case, clear the screen and home the cursor. */
    ScrClear(1, 1, 80, 25, SCR_WHITE);
    ScrSetCursorPos(1, 1);

    /* Adjust our records. */
    Is_Init = YES;
    return;
  }

/*----------------------------------------------------------------------------
void ScrTerminate(void);

The following function should be called whenever the module is no longer
going to be used.
----------------------------------------------------------------------------*/

PUBLIC
void ScrTerminate(void)
  {
    /* Do nothing if ScrInitialize() hasn't been called. */
    if (!Is_Init) return;

    /* Clear the screen and home the cursor. */
    ScrClear(1, 1, 80, 25, SCR_WHITE);
    ScrSetCursorPos(1, 1);

    /* Adjust our records. */
    Is_Init = NO;
    return;
  }

/*----------------------------------------------------------------------------
boolean ScrIsMonochrome(void);

The following function returns TRUE if the monitor is a monochrome monitor.
Otherwise, it returns FALSE (ie for color monitors). Monochrome monitors
will have/need attribute conversions and can only support 1 video page.
This function must be called after a call to ScrInitialize().
----------------------------------------------------------------------------*/

PUBLIC
boolean ScrIsMonochrome(void)
  {
    return Video_Mode == VIDEO_MONO;
  }

/*----------------------------------------------------------------------------
unsigned ScrSegmentAddress(void);

This function returns the segment address of the start of the currently
active video RAM. It must be called after a call to ScrInitialize(). It
returns the correct number even if the currently active video page is not
page 0.
----------------------------------------------------------------------------*/

PUBLIC
unsigned ScrSegmentAddress(void)
  {
    return Scr_Segment + Page_Nmbr * (BYTES_PER_PAGE >> 4);
  }

/*----------------------------------------------------------------------------
int ScrCvtAttr(int Attribute);

The following function adjusts the given color attribute so that it is
usable on a monochrome screen. It returns the adjusted attribute. If the
color attribute has a black background, this function forces a white
foreground. Otherwise this function returns a reverse video attribute.
Blink and bold attributes are unchanged.
----------------------------------------------------------------------------*/

PUBLIC
int ScrCvtAttr(int Attribute)
  {
    /* Return at once with the attribute unchanged if not the monochrome mode. */
    if (Video_Mode != VIDEO_MONO) return Attribute;

    /* If black background, force white foreground. */
    if ((Attribute & 0x70) == SCR_REV_BLACK) {
      Attribute |= SCR_WHITE;
    }

    /* Otherwise there's a colored background, force reverse video. */
    else {
      Attribute |= SCR_REV_WHITE;
      Attribute &= 0xfff8;	    /* Zero out (black) foreground only. */
    }

    return Attribute;
  }

/*----------------------------------------------------------------------------
int ScrReverseAttr(int Attribute);

The following function reverses the foreground and background colors in the
given attribute. Blink and bold are not changed. It returns the result.
----------------------------------------------------------------------------*/

PUBLIC
int ScrReverseAttr(int Attribute)
  {
    /* Extract the foreground and background colors. */
    int First  = Attribute & 0x07;
    int Second = (Attribute & 0x70) >> 4;

    /* Erase the colors in the original attribute. */
    Attribute &= 0xfff8;
    Attribute &= 0xff8f;

    /* Reinstall the colors in the opposite places. */
    Attribute |= First << 4;
    Attribute |= Second;

    return Attribute;
  }

/*----------------------------------------------------------------------------
void ScrRead(int Row, int Column, int Width, int Height, char *Buffer);

The following function copies a section of the screen to the specified
buffer. Both the text and the attributes are copied.
----------------------------------------------------------------------------*/

PUBLIC
void ScrRead(int Row, int Column, int Width, int Height, char *Buffer)
  {
    register unsigned  Nmbr_Of_Rows;	/* Loop index.			     */
    register unsigned  Row_Length;	/* Number of bytes in row of region. */
    auto     unsigned  Scr_Offset;	/* Offset into video RAM.	     */
    auto     unsigned  Buffer_Segment;	/* Far address of specified buffer.  */
    auto     unsigned  Buffer_Offset;	/*   etc...			     */
    auto     char far *Pntr_FarBuffer;	/* Far pointer to buffer.	     */

    Row_Length	= 2 * Width;
    Scr_Offset	= ((Row - 1) * BYTES_PER_ROW) + (Column - 1) * 2 + (Page_Nmbr * BYTES_PER_PAGE);

    /* Get Segment:Offset of specified buffer. */
    Pntr_FarBuffer = (char far *) Buffer;
    Buffer_Segment = FP_SEG(Pntr_FarBuffer);
    Buffer_Offset  = FP_OFF(Pntr_FarBuffer);

    /* Loop over all rows in the region. */
    for (Nmbr_Of_Rows = Height; Nmbr_Of_Rows > 0; Nmbr_Of_Rows--) {

      /* Copy data from buffer to video RAM and adjust offsets. */
      movedata(Scr_Segment, Scr_Offset, Buffer_Segment, Buffer_Offset, Row_Length);
      Scr_Offset    += BYTES_PER_ROW;
      Buffer_Offset += Row_Length;
    }

    return;
  }

/*----------------------------------------------------------------------------
void ScrReadText(int Row, int Column, int Width, int Height, char *Buffer);

The following function copies a section of the screen to the specified
buffer. Only the text is copied.
----------------------------------------------------------------------------*/

PUBLIC
void ScrReadText(int Row, int Column, int Width, int Height, char *Buffer)
  {
    unsigned   Scr_Offset;
    char far  *Source;
    unsigned   i;

    Scr_Offset = ((Row - 1) * BYTES_PER_ROW) + (Column - 1) * 2 + (Page_Nmbr * BYTES_PER_PAGE);
    for ( ; Height>0; Height--) {
      Source = MK_FP(Scr_Segment, Scr_Offset);
      for (i = 0; i < Width; i++, Source++) *Buffer++ = *Source++;
      Scr_Offset += BYTES_PER_ROW;
    }
    return;
  }

/*----------------------------------------------------------------------------
void ScrWrite(int Row, int Column, int Width, int Height, char *Buffer);

The following function copies bytes from the specifed buffer to the screen.
Both text and attributes are copied.
----------------------------------------------------------------------------*/

PUBLIC
void ScrWrite(int Row, int Column, int Width, int Height, char *Buffer)
  {
    register unsigned  Nmbr_Of_Rows;
    register unsigned  Row_Length;
    auto     unsigned  Scr_Offset;
    auto     unsigned  Buffer_Segment;
    auto     unsigned  Buffer_Offset;
    auto     char far *Pntr_FarBuffer;

    Row_Length = 2 * Width;
    Scr_Offset = ((Row - 1) * BYTES_PER_ROW) + (Column - 1) * 2 + (Page_Nmbr * BYTES_PER_PAGE);
    Pntr_FarBuffer   = (char far *) Buffer;
    Buffer_Segment = FP_SEG(Pntr_FarBuffer);
    Buffer_Offset = FP_OFF(Pntr_FarBuffer);
    for ( Nmbr_Of_Rows = Height; Nmbr_Of_Rows > 0; Nmbr_Of_Rows--) {
      movedata(Buffer_Segment, Buffer_Offset, Scr_Segment, Scr_Offset, Row_Length);
      Scr_Offset += BYTES_PER_ROW;
      Buffer_Offset += Row_Length;
    }
    return;
  }

/*----------------------------------------------------------------------------
void ScrWriteText(int Row, int Column, int Width, int Height, char *Buffer);

The following function copies bytes from the specifed buffer to the screen.
Both text and attributes are copied.
----------------------------------------------------------------------------*/

PUBLIC
void ScrWriteText(int Row, int Column, int Width, int Height, char *Buffer)
  {
    unsigned  Scr_Offset;
    char far *Dest;
    unsigned  i;

    Scr_Offset = ((Row - 1) * BYTES_PER_ROW) + (Column - 1) * 2 + (Page_Nmbr * BYTES_PER_PAGE);
    for ( ; Height > 0; Height--) {
      Dest = MK_FP(Scr_Segment, Scr_Offset);
      for (i=0; i<Width; i++, Dest++) *Dest++ = *Buffer++;
      Scr_Offset += BYTES_PER_ROW;
    }
    return;
  }

/*----------------------------------------------------------------------------
void ScrPrint(int Row, int Column, char *String, int Attribute, int Count);

The following function prints the specified string starting at the specifed
row and column. The specified attribute is used. The entire string is
printed or Count characters are printed, whichever comes first. Use Count
to limit the string so that there will be no attempt to print off screen.
----------------------------------------------------------------------------*/

PUBLIC
void ScrPrint(int Row, int Column, char *String, int Attribute, int Count)
  {
    unsigned  Scr_Offset;
    char far *Dest;

    Scr_Offset = ((Row - 1) * BYTES_PER_ROW) + (Column - 1) * 2 + (Page_Nmbr * BYTES_PER_PAGE);
    Dest = MK_FP(Scr_Segment, Scr_Offset);
    while (*String && Count--) {
      *Dest++ = *String++;
      *Dest++ = (byte) ScrCvtAttr(Attribute);
    }
    return;
  }

/*----------------------------------------------------------------------------
void ScrPrintText(int Row, int Column, char *String, int Count);

The following function performs the same action as ScrPrint() above except
that there is no attribute byte specified. Instead whatever attribute that
is found on the screen is used.
----------------------------------------------------------------------------*/

PUBLIC
void ScrPrintText(int Row, int Column, char *String, int Count)
  {
    unsigned  Scr_Offset;
    char far *Dest;

    Scr_Offset = ((Row - 1) * BYTES_PER_ROW) + (Column - 1) * 2 + (Page_Nmbr * BYTES_PER_PAGE);
    Dest = MK_FP(Scr_Segment, Scr_Offset);
    while (*String && Count--) {
      *Dest++ = *String++;
      Dest++;
    }
    return;
  }

/*----------------------------------------------------------------------------
void ScrClear(int Row, int Column, int Width, int Height, int Attribute);

The following function clears a region of the screen by scrolling it up
zero rows. The works because of the way IBM BIOS operates.
----------------------------------------------------------------------------*/

PUBLIC
void ScrClear(int Row, int Column, int Width, int Height, int Attribute)
  {
    ScrScroll(SCR_UP, Row, Column, Width, Height, 0, Attribute);
    return;
  }

/*----------------------------------------------------------------------------
void ScrScroll(
  int Direction, int Row, int Column, int Width, int Height, int Nmbr_Of_Rows, int Attribute
  );

The following function scrolls the indicated region. It uses IBM BIOS
function to do the job. If the number of rows to scroll is greater than the
height, the region is cleared by setting the number of rows to zero. This
was explicitly checked for due to misbehavior of some BIOS implementations.
----------------------------------------------------------------------------*/

PUBLIC
void ScrScroll(
  int Direction, int Row, int Column, int Width, int Height, int Nmbr_Of_Rows, int Attribute
  )
  {
    union REGS r;

    Attribute = ScrCvtAttr(Attribute);
    if (Nmbr_Of_Rows >= Height) Nmbr_Of_Rows = 0;
    if (Direction == SCR_DOWN) r.h.ah = (byte) SCROLL_DOWN;
      else r.h.ah = (byte) SCROLL_UP;
    r.h.al = (byte) Nmbr_Of_Rows;
    r.h.bh = (byte) Attribute;
    r.h.ch = (byte) (Row - 1);
    r.h.cl = (byte) (Column - 1);
    r.h.dh = (byte) (Row + Height - 2);
    r.h.dl = (byte) (Column + Width - 2);
    int86(BIOS_VIDEO, &r, &r);
    return;
  }

/*----------------------------------------------------------------------------
int  ScrNmbrPages(void);
int  ScrGetPageNmbr(void);
void ScrSetPageNmbr(int New_Nmbr);

The following functions do simple video page manipulations. They use the
BIOS.
----------------------------------------------------------------------------*/

PUBLIC
int ScrNmbrPages(void)
  {
    if (Video_Mode == VIDEO_MONO) return 1;
      else return 4;
  }

PUBLIC
int ScrGetPageNmbr(void)
  {
    return Page_Nmbr;
  }

PUBLIC
void ScrSetPageNmbr(int New_Nmbr)
  {
    union REGS r;

    if (New_Nmbr < 0 || New_Nmbr >= ((Video_Mode == VIDEO_MONO) ? 1 : 4) ) return;
    r.h.ah = (byte) SET_PAGE;
    r.h.al = (byte) New_Nmbr;
    int86(BIOS_VIDEO, &r, &r);
    Page_Nmbr = New_Nmbr;
    return;
  }

/*----------------------------------------------------------------------------
void ScrSetCursorPos(int Row, int Column);

The following function uses the BIOS to set the cursor position.
----------------------------------------------------------------------------*/

PUBLIC
void ScrSetCursorPos(int Row, int Column)
  {
    union REGS r;

    r.h.ah = (byte) SET_CURSORPOS;
    r.h.dh = (byte) (Row - 1);
    r.h.dl = (byte) (Column - 1);
    r.h.bh = (byte) Page_Nmbr;
    int86(BIOS_VIDEO, &r, &r);
    return;
  }

/*----------------------------------------------------------------------------
void ScrGetCursorPos(int *Row, int *Column);

The following function uses the BIOS to learn what the current cursor
position is.
----------------------------------------------------------------------------*/

PUBLIC
void ScrGetCursorPos(int *Row, int *Column)
  {
    union REGS r;

    r.h.ah = (byte) GET_CURSORPOS;
    r.h.bh = (byte) Page_Nmbr;
    int86(BIOS_VIDEO, &r, &r);
    *Row  = r.h.dh + 1;
    *Column  = r.h.dl + 1;
    return;
  }

/*----------------------------------------------------------------------------
void ScrSetCursorSize(int Start_Line, int End_Line);

The following function lets you adjust the size of the cursor.
----------------------------------------------------------------------------*/

PUBLIC
void ScrSetCursorSize(int Start_Line, int End_Line)
  {
    union REGS r;

    r.h.ah = (byte) SET_CURSORTYPE;
    r.h.ch = (byte) (0x000F & Start_Line);
    r.h.cl = (byte) (0x000F & End_Line);
    int86(BIOS_VIDEO, &r, &r);
    return;
  }
