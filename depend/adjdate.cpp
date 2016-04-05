/*! \file    adjdate.cpp
    \brief   Function that cleans up the __DATE__ macro.
    \author  Peter C. Chapin <PChapin@vtc.vsc.edu>

This file contains a function which adjusts the format of __DATE__. It puts a comma after the
day of the month and purges leading zeros or spaces from the day of the month.
*/

#include <cstring>
#include "misc.hpp"

using namespace std;

char *adjust_date( const char *ANSI_date )
{
    static char  buffer[13];
           char *buffer_pointer;

    // Make a working copy of the date as from the ANSI __DATE__ macro.
    strcpy( buffer, ANSI_date );

    // Open up space for the comma.
    for( buffer_pointer  = strchr( buffer,'\0' );
         buffer_pointer >= &buffer[6];
         buffer_pointer-- ) {
        *( buffer_pointer + 1 ) = *buffer_pointer;
    }

    // Put the comma in.
    buffer[6] = ',';

    // If this is a date from 1 to 9, close up the extra space.
    if( buffer[4] == '0' || buffer[4] == ' ' ) {
        for( buffer_pointer = &buffer[4]; *buffer_pointer; buffer_pointer++ ) {
            *buffer_pointer = *( buffer_pointer + 1 );
        }
    }

    // Return are result.
    return buffer;
}
