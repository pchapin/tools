/*****************************************************************************
FILE          : get_key.h
LAST REVISION : February 1991
SUBJECT       : Public interface to general purpose MS-DOS keystroke function.

     (C) Copyright 1991 by Peter Chapin

     This file contains the prototype for get_key() as well as the #defined
constants returned by get_key(). This function allows any keystroke on an
IBM keyboard to be read easily. If the key is a special key, this function
handles the issue of reading the special two character sequence. It returns
a unique integer for every special key.

     Cntrl-C is will NOT abort the program while this function waits for a
keystroke. The keystroke is not echoed to the screen.

     Please send comments and bug reports to

         Peter Chapin
         P.O. Box 317
         Randolph Center, VT 05061

*****************************************************************************/

#ifndef GET_KEY_H
#define GET_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

extern int get_key(void);

#ifdef __cplusplus
}
#endif

#define XF  0x100  /* Extended flag. Special keys have codes > XF. */

#define K_F1  (59+XF)      /* Function keys. */
#define K_F2  (60+XF)
#define K_F3  (61+XF)
#define K_F4  (62+XF)
#define K_F5  (63+XF)
#define K_F6  (64+XF)
#define K_F7  (65+XF)
#define K_F8  (66+XF)
#define K_F9  (67+XF)
#define K_F10 (68+XF)

#define K_SF1  (84+XF)     /* Shifted function keys. */
#define K_SF2  (85+XF)
#define K_SF3  (86+XF)
#define K_SF4  (87+XF)
#define K_SF5  (88+XF)
#define K_SF6  (89+XF)
#define K_SF7  (90+XF)
#define K_SF8  (91+XF)
#define K_SF9  (92+XF)
#define K_SF10 (93+XF)

#define K_CF1  (94+XF)     /* Control + function keys. */
#define K_CF2  (95+XF)
#define K_CF3  (96+XF)
#define K_CF4  (97+XF)
#define K_CF5  (98+XF)
#define K_CF6  (99+XF)
#define K_CF7  (100+XF)
#define K_CF8  (101+XF)
#define K_CF9  (102+XF)
#define K_CF10 (103+XF)

#define K_AF1  (104+XF)    /* Alt + function keys. */
#define K_AF2  (105+XF)
#define K_AF3  (106+XF)
#define K_AF4  (107+XF)
#define K_AF5  (108+XF)
#define K_AF6  (109+XF)
#define K_AF7  (110+XF)
#define K_AF8  (111+XF)
#define K_AF9  (112+XF)
#define K_AF10 (113+XF)

#define K_HOME  (71+XF)    /* Misc special keys. */
#define K_END   (79+XF)
#define K_PGUP  (73+XF)
#define K_PGDN  (81+XF)
#define K_LEFT  (75+XF)
#define K_RIGHT (77+XF)
#define K_UP    (72+XF)
#define K_DOWN  (80+XF)
#define K_INS   (82+XF)
#define K_DEL   (83+XF)

#define K_CHOME  (119+XF)  /* Control + misc special keys. */
#define K_CEND   (117+XF)
#define K_CPGUP  (132+XF)
#define K_CPGDN  (118+XF)
#define K_CLEFT  (115+XF)
#define K_CRIGHT (116+XF)

#define K_CTRLA  1         /* Nice names for control characters. */
#define K_CTRLB  2
#define K_CTRLC  3
#define K_CTRLD  4
#define K_CTRLE  5
#define K_CTRLF  6
#define K_CTRLG  7
#define K_CTRLH  8
#define K_CTRLI  9
#define K_CTRLJ  10
#define K_CTRLK  11
#define K_CTRLL  12
#define K_CTRLM  13
#define K_CTRLN  14
#define K_CTRLO  15
#define K_CTRLP  16
#define K_CTRLQ  17
#define K_CTRLR  18
#define K_CTRLS  19
#define K_CTRLT  20
#define K_CTRLU  21
#define K_CTRLV  22
#define K_CTRLW  23
#define K_CTRLX  24
#define K_CTRLY  25
#define K_CTRLZ  26

#define K_ESC        27    /* Nice names for special ascii keys. */
#define K_SPACE      32
#define K_TAB        K_CTRLI
#define K_BACKSPACE  K_CTRLH
#define K_RETURN     13
#define K_CRETURN    10

#define K_ALTA  (30+XF)    /* Alt + letter keys. */
#define K_ALTB  (48+XF)
#define K_ALTC  (46+XF)
#define K_ALTD  (32+XF)
#define K_ALTE  (18+XF)
#define K_ALTF  (33+XF)
#define K_ALTG  (34+XF)
#define K_ALTH  (35+XF)
#define K_ALTI  (23+XF)
#define K_ALTJ  (36+XF)
#define K_ALTK  (37+XF)
#define K_ALTL  (38+XF)
#define K_ALTM  (50+XF)
#define K_ALTN  (49+XF)
#define K_ALTO  (24+XF)
#define K_ALTP  (25+XF)
#define K_ALTQ  (16+XF)
#define K_ALTR  (19+XF)
#define K_ALTS  (31+XF)
#define K_ALTT  (20+XF)
#define K_ALTU  (22+XF)
#define K_ALTV  (47+XF)
#define K_ALTW  (17+XF)
#define K_ALTX  (45+XF)
#define K_ALTY  (21+XF)
#define K_ALTZ  (44+XF)

#define K_ALT1    (120+XF) /* Alt + number keys. */
#define K_ALT2    (121+XF)
#define K_ALT3    (122+XF)
#define K_ALT4    (123+XF)
#define K_ALT5    (124+XF)
#define K_ALT6    (125+XF)
#define K_ALT7    (126+XF)
#define K_ALT8    (127+XF)
#define K_ALT9    (128+XF)
#define K_ALT0    (129+XF)
#define K_ALTDASH (130+XF)
#define K_ALTEQU  (131+XF)

#define K_CTRL_PRTSC (114+XF)

#endif

