#
# Makefile for the depend project.
#

CXX=g++
CXXFLAGS=-std=c++98 -c -Wall -O -I../../../Spica/Cpp
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
LIBSPICA=../../../Spica/Cpp/libSpicaCpp.a

%.o:	%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

$(EXECUTABLE):	$(OBJECTS)
	$(LINK) $(OBJECTS) $(LINKFLAGS) $(LIBSPICA) -o $@

# File Dependencies
###################

# Module dependencies -- Produced with 'depend' on Sun Dec 23 11:11:47 2018


adjdate.o:	adjdate.cpp misc.hpp 

depend.o:	depend.cpp ../../../Spica/Cpp/environ.hpp filename.hpp filescan.hpp \
	../../../Spica/Cpp/get_switch.hpp misc.hpp output.hpp record_f.hpp 

filename.o:	filename.cpp ../../../Spica/Cpp/environ.hpp filename.hpp 

filescan.o:	filescan.cpp ../../../Spica/Cpp/environ.hpp filename.hpp \
	filescan.hpp linescan.hpp 

linescan.o:	linescan.cpp ../../../Spica/Cpp/environ.hpp filename.hpp filescan.hpp linescan.hpp \
	output.hpp 

output.o:	output.cpp filename.hpp output.hpp 

record_f.o:	record_f.cpp ../../../Spica/Cpp/environ.hpp misc.hpp record_f.hpp 

splits.o:	splits.cpp ../../../Spica/Cpp/environ.hpp misc.hpp 


# Additional Rules
##################
clean:
	rm -f *.bc *.bc1 *.bc2 *.o $(EXECUTABLE) *.s *.ll *~
