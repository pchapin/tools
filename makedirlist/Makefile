############################################################################
# FILE        : makefile
# LAST REVISED: January 2023
# AUTHOR      : Peter Chapin
#
# This is the makefile for the makedirlist program.
############################################################################

CC      = g++
OPTIONS = -c -Wall -std=c++11
LIBS    = 

# Implicit rules for converting source to object.
%.o:	%.cpp
	$(CC) $(OPTIONS) $<

############################################################################

# Object file list.
OBJS = makedirlist.o


# The overall target and how to make it.
makedirlist:	$(OBJS) 
	$(CC) -o makedirlist $(OBJS) $(LIBS)

# The usual nicities
clean:
	rm -f *.o core makedirlist

############################################################################
# Module dependencies

makedirlist.o:	makedirlist.cpp
