/*! \file    filescan.hpp
    \brief   Implementation of file scanning functions.
    \author  Peter C. Chapin <PChapin@vtc.vsc.edu>

*/

#include "environ.hpp"

#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>

#include "ansiscr.hpp"
#include "filename.hpp"
#include "filescan.hpp"
#include "linescan.hpp"

using namespace std;
using namespace scr::ansi;

static int nesting_level = 0;

/*----------------------------------------------------------------------------
The following function prints the name of the file which is currently being
scanned. To make things interesting, it uses bold for primary source files.
----------------------------------------------------------------------------*/

static void print(char *name)
{
    assert(nesting_level > 0);

    bool flag = false;

    // Set flag to true if this file is a primary source file.
    char *end_pointer = strchr( name, '\0' );
    while( end_pointer != name && *end_pointer != '.' ) end_pointer--;
    #if eOPSYS == ePOSIX
        if( strcmp( end_pointer, ".c"   ) == 0 ||
            strcmp( end_pointer, ".cpp" ) == 0 ||
            strcmp( end_pointer, ".C"   ) == 0 ) flag = true;
    #else
        if( _stricmp( end_pointer, ".c"   ) == 0 ||
            _stricmp( end_pointer, ".cpp" ) == 0 ) flag = true;
    #endif

    // Print the name with special effect if necessary.
    if( flag ) bold_on( );
    for( int i = 0; i < nesting_level; i++ ) cout << "  " << flush;
    cout << "Scanning " << name << "..." << flush;
    if( flag ) reset_screen( );
    cout << endl;

    return;
}

/*----------------------------------------------------------------------------
The following function reads all the lines out of the specified input file
and calls handle_line() to deal with each. This function is called inside
of handle_line() to deal with include files which #include other files. In
other words, this function is indirectly recursive.
----------------------------------------------------------------------------*/

void handle_file( char *name )
{
    char     file_name[FILENAME_LENGTH+1];
    ifstream input_file;

    // Try to open this guy. Must copy name out of static buffer in match_name() because of
    // recursion.
    //
    // strcpy( file_name, match_name( name ) );
    //
    // The above call to match_name() is spurious. match_name() is called inside of
    // handle_line() just before calling handle_file(). Thus the name given to this function has
    // already been "matched." By removing the call here, I just disallow name matching on the
    // top level names -- not a big deal. The user will probably specify those names fully
    // anyway.
    //
    // I also found that commenting out the call above allowed this program to work on the Y
    // source code under OS/2. It was not doing so before, perhaps because of an excessive
    // number of calls to _dos_findfirst() inside of match_name(). There also may have been some
    // sort of strange interaction.

    strcpy( file_name, name );
    input_file.open( file_name );
    if( !input_file ) {

        // Print error message with special effect. Notice that we have to indent an amount of
        // nesting_level + 1 since we want the error message to appear where the name should go
        // and we haven't incremented the nesting_level to that point yet.
        //
        set_color( F_YELLOW );
        for( int i = 0; i < nesting_level + 1; i++ ) cout << "  " << flush;
        cout << "!!! Can't open " << file_name << " for input. Skipping..." << flush;
        reset_screen( );
        cout << endl;
    }

    // Ok, read it.
    else {
        char buffer[256+2];
        
        nesting_level++;
        print( file_name );
        while( input_file.getline( buffer, 256 + 2 ) ) {
            handle_line( buffer );
        }
        nesting_level--;
    }
    return;
}
