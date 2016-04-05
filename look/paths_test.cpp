/****************************************************************************
FILE          : paths_test.cpp
LAST REVISION : 2005-08-15
SUBJECT       : Test program for functions in paths.cpp
PROGRAMMER    : (C) Copyright 2005 by Peter C. Chapin

Please send comments or bug reports to

     Peter C. Chapin
     Department of Electrical and Computer Engineering Technology
     Vermont Technical College
     Randolph Center, VT 05061
     pchapin@ecet.vtc.edu
****************************************************************************/

#include <iostream>

#include "paths.h"

int main( )
{
    std::string result = find_absolute_path( "." );
    std::cout << result.c_str( ) << "\n";
    return( 0 );
}
