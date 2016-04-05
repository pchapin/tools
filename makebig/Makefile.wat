############################################################################
# FILE        : makefile
# LAST REVISED: 2005-07-31
# AUTHOR      : Peter C. Chapin
#
# This is the makefile for the makebig program. This file uses Open Watcom
# syntax (in cases where it makes a difference).
############################################################################

CC      = wpp386
OPTIONS = -zq -wx

# Implicit rules for converting source to object.
.cpp.obj:
        $(CC) $(OPTIONS) $[*

############################################################################

# Object file list.
OBJS = makebig.obj get_switch.obj

# The overall target and how to make it.
makebig.exe:    $(OBJS)
        wlink @link.rsp

############################################################################
# Module dependencies

makebig.obj:    makebig.cpp get_switch.h

get_switch.obj:   get_switch.cpp get_switch.h
