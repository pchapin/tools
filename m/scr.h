/*****************************************************************************
FILE	      : scr.h
LAST REVISION : February 1991
SUBJECT	      : Public interface to low level screen handling functions.

     (C) Copyright 1991 by Peter Chapin

This file contains the public interface to a fast screen handling module.
Many of the functions in this module to direct video access. See the file
SCR.DOC for more information.

     Please send comments and bug reports to

	 Peter Chapin
	 P.O. Box 317
	 Randolph Center, VT 05061

*****************************************************************************/

#ifndef SCR_H
#define SCR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Start up and clean up. */
extern void ScrInitialize(void);
extern void ScrTerminate(void);

/* Informational. */
extern boolean	ScrIsMonochrome(void);
extern unsigned ScrSegmentAddress(void);

/* Attribute manipulation. */
extern int ScrCvtAttr(int Attr);
extern int ScrReverseAttr(int Attr);

/* Fast transfer of material to and from screen. */
extern void ScrRead(int Row, int Col, int Width, int Heigth, char *Buf);
extern void ScrWrite(int Row, int Col, int Width, int Heigth, char *Buf);
extern void ScrReadText(int Row, int Col, int Width, int Heigth, char *Buf);
extern void ScrWriteText(int Row, int Col, int Width, int Heigth, char *Buf);
extern void ScrPrint(int Row, int Col, char *String, int Attr, int Count);
extern void ScrPrintText(int Row, int Col, char *String, int Count);

/* Manipulation of regions via BIOS calls. */
extern void ScrClear(int Row, int Col, int Width, int Heigth, int Attr);
extern void ScrScroll(
  int Direction,
  int Row, int Col, int Width, int Heigth,
  int Nmbr_Of_Rows, int Attr
  );

/* Manipulating video pages. */
extern int  ScrNmbrPages(void);
extern int  ScrGetPageNmbr(void);
extern void ScrSetPageNmbr(int New_Nmbr);

/* Cursor manipulations via BIOS calls. */
extern void ScrSetCursorPos(int Row, int Col);
extern void ScrGetCursorPos(int *Row, int *Col);
extern void ScrSetCursorSize(int Start_Line, int End_Line);

#ifdef __cplusplus
}
#endif

#define SCR_WHITE	0x07
#define SCR_BLACK	0x00
#define SCR_BLUE	0x01
#define SCR_GREEN	0x02
#define SCR_RED		0x04
#define SCR_CYAN	0x03
#define SCR_MAGENTA	0x05
#define SCR_BROWN	0x06

#define SCR_REV_WHITE	0x70
#define SCR_REV_BLACK	0x00
#define SCR_REV_BLUE	0x10
#define SCR_REV_GREEN	0x20
#define SCR_REV_RED	0x40
#define SCR_REV_CYAN	0x30
#define SCR_REV_MAGENTA 0x50
#define SCR_REV_BROWN	0x60

#define SCR_BRIGHT	0x08
#define SCR_BLINK	0x80

/* Arbitrary values to denote scroll directions. */
#define SCR_UP	  1
#define SCR_DOWN  2

#endif
