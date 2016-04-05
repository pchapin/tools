/*****************************************************************************
FILE     : scanners.h
CONTENTS : C/C++ comment stripping functions.
PROGRAMMER    : Peter Chapin

This file contains the interface to a module that recognizes comments and
quoted strings in C or C++ source code. To use it, you must first send
CmtScanInit() a pointer to a function that reads the source. Then, calls to
CmtScanGetChar() will return characters from the file, but it will set some
global variables to indicate what that character is part of. See CMTSCAN.C
for more information.

*****************************************************************************/

#ifndef SCANNERS_H
#define SCANNERS_H

#ifdef __cplusplus
extern "C" {
#endif

extern void CmtScanInit(int (*)(void));
extern int  CmtScanGetChar(void);

#ifdef __cplusplus
}
#endif

extern boolean Comment;
extern boolean DoubleQuote;
extern boolean SingleQuote;
extern boolean OpenCommentError;

#endif

