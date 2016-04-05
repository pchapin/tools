###########################################################################
# FILE          : makefile
# LAST REVISION : August 1991
# SUBJECT       : Makefile for MENU.EXE
#
#       (C) Copyright 1991 by Peter Chapin
###########################################################################

# Implicit rule for creating .obj files from .c files.
.c.obj:
        bcc -c {$< }

OBJS = menu.obj get_key.obj scr.obj scrcent.obj scrdrawb.obj scrgets.obj

menu.com:  $(OBJS)
        tlink /t /m @&&!
p:\bcpp30\lib\c0t.obj $(OBJS)
$*
$*
p:\bcpp30\lib\cs.lib
!

#############################################################################
# Module dependencies -- Produced with DEPEND.EXE on Wed May 20 15:46:03 1992


menu.obj:       menu.c local.h scr.h scrtools.h get_key.h

get_key.obj:    get_key.c get_key.h

scr.obj:        scr.c local.h scr.h

scrcent.obj:    scrcent.c local.h scrtools.h scr.h

scrdrawb.obj:   scrdrawb.c local.h scrtools.h scr.h

scrgets.obj:    scrgets.c local.h scr.h scrtools.h get_key.h
