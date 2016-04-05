#
# Makefile for the depend project.
#

CXX=g++
CXXFLAGS=-std=c++11 -c -O -I../../Spica -I../../Scr
LINK=g++
LINKFLAGS=
SOURCES=adjdate.cpp   \
	depend.cpp    \
	filename.cpp  \
	filescan.cpp  \
	linescan.cpp  \
        output.cpp    \
	record_f.cpp  \
	splits.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=depend
LIBSPICA=../../Spica/libSpicaCpp.a
LIBSCR=../../Scr/libScr.a

%.o:	%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

$(EXECUTABLE):	$(OBJECTS)
	$(LINK) $(OBJECTS) $(LINKFLAGS) $(LIBSPICA) $(LIBSCR) -o $@

# File Dependencies
###################

# Module dependencies -- Produced with 'depend' on Sun Jul 14 15:27:27 2013


adjdate.o:	adjdate.cpp misc.hpp 

depend.o:	depend.cpp ../../Spica/environ.hpp ../../Scr/ansiscr.hpp filename.hpp filescan.hpp \
	../../Spica/get_switch.hpp misc.hpp output.hpp record_f.hpp 

filename.o:	filename.cpp ../../Spica/environ.hpp filename.hpp 

filescan.o:	filescan.cpp ../../Spica/environ.hpp ../../Scr/ansiscr.hpp filename.hpp filescan.hpp \
	linescan.hpp 

linescan.o:	linescan.cpp ../../Spica/environ.hpp filename.hpp filescan.hpp linescan.hpp output.hpp \
	

output.o:	output.cpp filename.hpp output.hpp 

record_f.o:	record_f.cpp ../../Spica/environ.hpp misc.hpp record_f.hpp 

splits.o:	splits.cpp ../../Spica/environ.hpp misc.hpp 


# Additional Rules
##################
clean:
	rm -f *.bc *.bc1 *.bc2 *.o $(EXECUTABLE) *.s *.ll *~
