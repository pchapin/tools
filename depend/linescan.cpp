/*! \file    linescan.hpp
    \brief   Definition of line scanning functions.
    \author  Peter C. Chapin <PChapin@vtc.vsc.edu>

*/
#include "environ.hpp"

#include <ctype.h>
#include <string.h>

#include "filename.hpp"
#include "filescan.hpp"
#include "linescan.hpp"
#include "output.hpp"

#if eOPSYS == ePOSIX

/*---------------------------------------------------------------------------
The following function converts a string to lowercase.
---------------------------------------------------------------------------*/

static char *_strlwr(char *line)
{
    char *start = line;

    while( *line ) {
        *line = tolower( *line );
        line++;
    }
    return start;
}

#endif

/*----------------------------------------------------------------------------
This function skips leading white space on the string pointed at by 'line'.
It then checks for the presence of "#include". If it finds it, the function
returns the address of the first non-white space character after the
"#include". Otherwise, the function returns NULL.
----------------------------------------------------------------------------*/

static char *skip_include( char *line )
{
    char *return_value = NULL;

    // Scan line, skipping spaces and tabs.
    while( *line  &&  ( *line == ' '  ||  *line == '\t' ) ) line++;

    // See if first interesting thing on the line is #include.
    if( strncmp( line, "#include", 8 ) == 0 ) {

        // If so, skip to first non space or tab on line.
        line += 8;
        while( *line  &&  ( *line == ' '  ||  *line == '\t' ) ) line++;
        return_value = line;
    }
    return return_value;
}

/*----------------------------------------------------------------------------
This function orchestrates the action of the program for each line from
each file.
----------------------------------------------------------------------------*/

void handle_line( char *line )
{
    char *line_pointer;
    char *end_pointer;
    char  file_name[FILENAME_LENGTH + 1];

    // Do nothing if this line has no #include statement on it.
    if( ( line_pointer = skip_include( line ) ) != NULL ) {

        // If this is a '<...>' enclosed #include, ignore it.
        if( *line_pointer == '<' ) return;

        // Advance past the '\"' character.
        if( *line_pointer ) line_pointer++;

        // Chop off next '\"' character.
        if( ( end_pointer = strchr( line_pointer, '\"' ) ) != NULL ) *end_pointer = '\0';

        // Why was this being done? It's clearly wrong on Unix systems.
        // _strlwr( line_pointer );

        // Match name to that of an existing file.
        strcpy( file_name, match_name( line_pointer ) );

        if( !already_scanned( file_name ) ) {

            // Emit the name into the output file.
            emit( file_name );

            // See what headers this #included file further includes.
            handle_file( file_name );
        }
    }
    return;
}
