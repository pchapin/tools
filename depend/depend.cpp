/*! \file    depend.cpp
 *  \brief   Program that computes header dependencies for makefiles.
 *  \author  Peter Chapin <chapinp@proton.me>
 */

#include "environ.hpp"

#include <iostream>
#include <cstdlib>
#include <cstring>

#include "filename.hpp"
#include "filescan.hpp"
#include "get_switch.hpp"
#include "misc.hpp"
#include "output.hpp"
#include "record_f.hpp"

using namespace std;

/*=================================*/
/*           Global Data           */
/*=================================*/

const int BUFFER_SIZE = 80;

static int continuation_character = '\\';
static const char *include_list = NULL;
// static char *object_extension = "obj";

static SwitchInfo switch_table[] = {
  { 'c', chr_switch, &continuation_character, NULL,
    "Continuation character used in makefile (default = '\\')" },
  { 'I', str_switch, NULL, &include_list,
    "Semicolon delimited list of directory names for include files" }
};
static int switch_table_size = sizeof( switch_table )/sizeof( SwitchInfo );

/*==================================*/
/*           Main Program           */
/*==================================*/

/*----------------------------------------------------------------------------
The following function loops over all lines in the dependency file emiting
a dependency list for each one.
----------------------------------------------------------------------------*/

int main( int argc, char *argv[] )
{
    int exit_code = 0;  // =1 if error.

    argc = get_switchs( argc, argv, switch_table, switch_table_size );

    // Print credits.
    #if eOPSYS == eOS2
    cerr << "DEPEND for OS/2 (Version 2.3c) " << adjust_date( __DATE__ ) << '\n' <<
            "Public Domain Software by Peter Chapin\n" << endl;
    #endif
    #if eOPSYS == eWIN32
    cerr << "DEPEND for Win32 (Version 2.3c) " << adjust_date( __DATE__ ) << '\n' <<
            "Public Domain Software by Peter Chapin\n" << endl;
    #endif
    #if eOPSYS == ePOSIX
    cerr << "DEPEND for Unix (Version 2.3c) " << adjust_date( __DATE__ ) << '\n' <<
            "Public Domain Software by Peter Chapin\n" << endl;
    #endif

    // Check usage.
    if( argc != 3 ) {
        cerr <<
            "Wrong number of arguments.\n"
            "\n"
            "Usage: DEPEND [switches] lst_file out_file\n"
            "  Where lst_file is the name of a file contain source names and\n"
            "        out_file is the name of the file to write." << endl;
        cerr << "\nLegal switches are:" << endl;
        print_usage(switch_table, switch_table_size, cerr);
        exit_code = 1;
    }

    // Try to open the output file.
    else if( !open( argv[2] ) ) {
        cerr << "Error: Can't open file " << argv[2] << " for output." << endl;
        exit_code = 1;
    }

    else {
        char **fields;

        // Register the include file names with module that handles such things.
        set_directory_list( include_list );

        // Open the master input file.
        RecordFile list_file( argv[1], RecordFile::DEFAULT, BUFFER_SIZE, '#', " \t" );
        if( list_file.is_ok ) {

            // Read lines from the dependency file and handle each one.
            while( ( fields = list_file.get_line( ) ) != NULL ) {

                // Skip blank lines.
                if( list_file.get_length( ) != 0 ) {

                    // Write out the full dependency list for this file.
                    start( fields[0] );
                    handle_file( fields[0] );
                    flush( continuation_character );
                }
            }
        }
    }
    return exit_code;
}
