############################################################################
# FILE        : makefile
# LAST REVISED: 2005-08-22
# AUTHOR      : Peter C. Chapin
#
# This is the makefile for the makebig program.
############################################################################

CC       = g++
COPTIONS = -c -Wall


############################################################################

# Object file list.
OBJS = makebig.o get_switch.o

# The overall target and how to make it.
makebig:    $(OBJS)
	g++ -o makebig $(OBJS)

############################################################################
# Module dependencies

makebig.o:    makebig.cpp get_switch.h

get_switch.o:   get_switch.cpp get_switch.h
