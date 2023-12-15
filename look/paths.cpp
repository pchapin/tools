/*! \file   paths.cpp
 *  \brief  Functions for file name path manipulation.
 *  \author Peter Chapin <spicacality@kelseymountain.org>
 */

#include <string>
#include "environ.hpp"

#if eOPSYS == ePOSIX
#include <unistd.h>
#else
#include <direct.h>
#endif
#include "paths.hpp"

namespace {
    // This is probably in the library somewhere.
    constexpr char path_separator = ( eOPSYS == ePOSIX ) ? '/' : '\\';
}

std::string find_absolute_path( const char *p )
{
    std::string path( p );

    // If it's a relative path we have to do some work.
    if( path.size( ) > 0 && path[0] != path_separator ) {
        char *temp = getcwd( nullptr, 0 );
        std::string workspace( temp );
        std::free( temp );
        path = workspace;
    }
    return( path );    
}

