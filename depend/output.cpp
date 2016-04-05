/*! \file    output.cpp
    \brief   Output handling functions.
    \author  Peter C. Chapin <PChapin@vtc.vsc.edu>

*/

#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <string>

#include "filename.hpp"
#include "output.hpp"

using namespace std;

/*=================================*/
/*           Global Data           */
/*=================================*/

static ofstream      output_file;       // File where dependencies are written.
static list<string> *name_list = NULL;  // Contains list of dependent filenames.
static int           column_count;      // Counts characters on current line of output.
static const char   *preamble =         // Printed at top of dependencies.
  "# Module dependencies -- Produced with \'depend\' on ";

/*==========================================*/
/*           Function Definitions           */
/*==========================================*/

/*----------------------------------------------------------------------------
The following function opens the file that will contain the dependency
list. The file will be suitable for cut and paste into a makefile.
----------------------------------------------------------------------------*/

bool open( char *name )
{
    time_t now = time(NULL);

    output_file.open( name );
    if( !output_file ) return false;
    output_file << preamble << ctime( &now ) << endl;
    return true;
}

/*----------------------------------------------------------------------------
The following function is called whenever a new primary source file is
scanned. It prints the start of the dependency list. In particular, the
object file name and the source file name itself. This function also
initializes the dependency list to an empty state.
----------------------------------------------------------------------------*/

void start( char *name )
{
    char base[FILENAME_LENGTH + 1];
    char extension[3 + 1];

    // Locate the extension part of the filename.
    char *end_pointer = strchr( name, '\0' );
    while( end_pointer != name && *end_pointer != '.' ) end_pointer--;
    strcpy( extension, end_pointer + 1 );

    // Find the base part of the filename.
    char *start_pointer = end_pointer - 1;
    #if eOPSYS == ePOSIX
      while( start_pointer != name && *start_pointer != '/' && *start_pointer != ':' ) start_pointer--;
    #else
      while( start_pointer != name && *start_pointer != '\\' && *start_pointer != ':' ) start_pointer--;
    #endif
    *end_pointer = '\0';
    strcpy( base, start_pointer );
    *end_pointer = '.';

    // Print out object file name and source file name.
    #if eOPSYS == ePOSIX
      output_file << "\n" << base << ".o:\t" << base << "." << extension << " ";
    #else
      output_file << "\n" << base << ".obj:\t" << base << "." << extension << " ";
    #endif
    column_count = 16 + strlen( base ) + strlen( extension );

    // Prepare list for filenames.
    name_list = new list<string>;
}

/*----------------------------------------------------------------------------
This function returns YES if the given filename is already in the
dependency list. Checking for this allows the program to skip redundant
file reads.
----------------------------------------------------------------------------*/

bool already_scanned( char *name )
{
    // Skip out if there's no name list.
    if( name_list == NULL ) return false;

    // Search the current list of filenames. Return true if already on list.
    for( const string &current_name : *name_list ) {
        if( current_name == name ) return true;
    }

    return false;
}

/*----------------------------------------------------------------------------
The following function adds the given name (assumed to be complete) to the
list of dependent files. It first searchs the list to see if the name is
already on it. Names are not added to the list twice.
----------------------------------------------------------------------------*/

void emit( char *name )
{
    // Skip out if there's no name list.
    if( name_list == NULL ) return;

    // Search the current list of filenames. Return at once if already on list.
    if( already_scanned( name ) ) return;

    // There must have been no match. Add the new name.
    name_list->push_back( name );
    return;
}

/*----------------------------------------------------------------------------
The following function writes the current dependency list to the output
file. This writing is postponed to this time (rather than being done in
emit()) so that multiple copies of the same file are not written.
----------------------------------------------------------------------------*/

void flush( char continuation )
{
    // Skip out if there's no list.
    if( name_list == NULL ) return;

    // Scan over list printing the names as they are found.
    for( const string &name : *name_list ) {

        // Output name and advance counter.
        output_file << name << " ";
        column_count += name.length( ) + 1;

        // Adjust column count, wrapping line if necessary.
        if( column_count > 95 ) {
            output_file << continuation << "\n\t";
            column_count = 8;
        }
    }

    // Be sure we're starting on a fresh line for the next dependency list.
    output_file << "\n";

    // Erase the current list.
    delete name_list;
    name_list = NULL;

    return;
}
