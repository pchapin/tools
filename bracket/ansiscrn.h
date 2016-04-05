/*****************************************************************************
FILE          : ansiscrn.h
LAST REVISION : September 1991
PROGRAMMER    : Peter Chapin

This file contains the interface to a simple screen handling module.

I welcome comments and bug reports. I can be reached as in:

     Peter Chapin
     P.O. Box 317
     Randolph Center, VT 05061
*****************************************************************************/

#ifndef ANSISCRN_H
#define ANSISCRN_H

#ifdef __cplusplus
extern "C" {
#endif

/* General screen handling */
extern void Clear_Screen(void);
extern void Clear_To_EOL(void);

/* Nice names for the colors */
#define F_BLACK     30
#define F_RED       31
#define F_GREEN     32
#define F_YELLOW    33
#define F_BLUE      34
#define F_MAGENTA   35
#define F_CYAN      36
#define F_WHITE     37
#define B_BLACK     40
#define B_RED       41
#define B_GREEN     42
#define B_YELLOW    43
#define B_BLUE      44
#define B_MAGENTA   45
#define B_CYAN      46
#define B_WHITE     47
extern void Set_Color(int Color);

/* Special effects */
extern void Bold_On(void);
extern void Blink_On(void);
extern void Reverse_On(void);
extern void Reset_Screen(void);

/* Cursor handling */
extern void Position_Cursor(int Row, int Column);
extern void Cursor_Up(int Count);
extern void Cursor_Down(int Count);
extern void Cursor_Forward(int Count);
extern void Cursor_Backward(int Count);
extern void Save_Cursor_Position(void);
extern void Restore_Cursor_Position(void);

#ifdef __cplusplus
}
#endif

#endif
