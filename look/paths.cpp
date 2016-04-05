/****************************************************************************
FILE          : paths.cpp
LAST REVISION : 2005-08-15
SUBJECT       : Functions for file name path manipulation.
PROGRAMMER    : (C) Copyright 2005 by Peter C. Chapin

Please send comments or bug reports to

     Peter C. Chapin
     Department of Electrical and Computer Engineering Technology
     Vermont Technical College
     Randolph Center, VT 05061
     pchapin@ecet.vtc.edu
****************************************************************************/

#include <cstdlib>

#include <direct.h>
#include "paths.h"

std::string find_absolute_path( const char *p )
{
    std::string path( p );

    // If it's a relative path we have to do some work.
    if( path.size( ) > 0 && path[0] != '\\' ) {
        char *temp = getcwd( NULL, 0 );
        std::string workspace( temp );
        std::free( temp );

        path = workspace;
    }
    return( path );    
}

