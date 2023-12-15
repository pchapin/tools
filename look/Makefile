#
# A simple Makefile for the `look` tool.
#

# The default target (it is the default because it is first).
all:	look

# How to make the executable.
look:	look.o paths.o
	g++ -g -o look look.o paths.o
# How to make the object file required by the executable.
#   -c        : Compile only, do not link.
#   -std=c++20: Use the C++ 2020 standard.
#   -Wall     : Activate "all" (easy to fix) warnings.
#   -DDEBUG   : Define a DEBUG macro (value set to 1) for conditional compliation.
#   -O0       : Disable the optimizer (makes the code easier to debug, but slower).
#   -g        : Add debugging information to the object file.
#
look.o:	look.cpp environ.hpp paths.hpp
	g++ -c -std=c++20 -Wall -DDEBUG -O0 -g look.cpp

paths.o: paths.cpp environ.hpp paths.hpp
	g++ -c -std=c++20 -Wall -DDEBUG -O0 -g paths.cpp

doc:
	doxygen

clean:
	rm -f look *.o
	rm -rf doc/HTML
