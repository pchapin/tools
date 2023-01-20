/*! \file    filename.hpp
 *  \brief   Implementation of the filename handling functions.
 *  \author  Peter Chapin <chapinp@proton.me>
 */

#include "environ.hpp"

#include <list>
#include <stdio.h>
#include <string.h>
#include <string>

#if eOPSYS == ePOSIX
#include <sys/stat.h>
#include <unistd.h>
#elif eOPSYS == eWIN32
#include <windows.h>
#else
#include <dos.h>
#endif

#include "filename.hpp"

using namespace std;

/*=================================*/
/*           Global Data           */
/*=================================*/

static list<string> directory_list;

/*==========================================*/
/*           Function Definitions           */
/*==========================================*/

// The following function returns true if the given string does not end with a directory
// delimiter character. Otherwise it returns false.

bool no_trail( char *buffer )
{
    if( strlen( buffer ) == 0 ) return true;

    char *end_pointer = strchr( buffer, '\0' ) - 1;

    #if eOPSYS == pPOSIX
      if( *end_pointer != '/' ) return true;
    #else
      if( *end_pointer != '\\' ) return true;
    #endif
    return false;
}

// The following function takes a semicolon delimited list of directory names and puts the names
// into the String_List named directory_list above. This function does not append the
// directories to the list. If there were already names in the list, they are erased first. This
// function also inserts a null name as the first entry. This function modifies the string given
// to it as an argument.

void set_directory_list( const char *new_directory_list )
{
    // Make sure the list is empty.
    directory_list.clear( );

    // Install a null directory name (makes logic of Match_Name easier).
    directory_list.push_back( "" );

    // If we really didn't get anything in new_directory_list, then we are done.
    if( new_directory_list == NULL ) return;

    // Install ";" delimited directory names into the list.
    char *temporary_directory_list = new char[strlen( new_directory_list ) + 1];
    strcpy( temporary_directory_list, new_directory_list );
    char *directory_name = strtok( temporary_directory_list, ";" );
    while( directory_name != NULL ) {
        directory_list.push_back( directory_name );
        directory_name = strtok( NULL, ";" );
    }
    delete [] temporary_directory_list;
    return;
}

// The following function takes the name given as a parameter and returns the full path of an
// existing file with that name. The only directory paths used in the test are the ones in the
// directory list. If no file exists with the given name, the orignal string is returned. If the
// given name starts with a backslash, the directory list is not used.

char *match_name( char *name )
  {
    // Holds final matched name.
    static char buffer[FILENAME_LENGTH+1];

    // If name starts with a directory delimiter character, don't try to append directory names
    // on it.
    #if eOPSYS == ePOSIX
      if( *name == '/' ) {
          strcpy( buffer, name );
          return buffer;
      }
    #else
      if( *name == '\\' ) {
          strcpy( buffer, name );
          return buffer;
      }
    #endif

    // If name starts with a drive specifier, don't try to append directory names on it.
    if( name[0] != '\0' && name[1] == ':' ) {
        strcpy( buffer, name );
        return buffer;
    }

    // Loop through all the directory names to see if an existing file name can be found.
    bool match_found = false;
    for( list<string>::const_iterator current_directory = directory_list.begin();
         current_directory != directory_list.end();
         ++current_directory ) {

      #if eOPSYS == ePOSIX
        struct stat file_info;
      #elif eOPSYS != eWIN32
        struct find_t file_info;
      #endif
      

        // Copy directory prefix into buffer. Append a directory delimiter only if there's
        // something there and only if there isn't a trailing directory delimiter allready.
        //
        strcpy(buffer, current_directory->c_str( ) );
      #if eOPSYS == ePOSIX
        if( strlen( buffer ) != 0 && no_trail( buffer ) ) strcat( buffer, "/" );
      #else
        if( strlen( buffer ) != 0 && no_trail( buffer ) ) strcat( buffer, "\\" );
      #endif
        strcat( buffer, name );

        // See if the file exists.
        #if eOPSYS == ePOSIX
        if( stat( buffer, &file_info ) == 0 && S_ISREG( file_info.st_mode ) ) {
            match_found = true;
            break;
        }
        #elif eOPSYS == eWIN32
        if( GetFileAttributes( buffer ) != 0xFFFFFFFFU ) {
            match_found = true;
            break;
        }
        #else
        if( _dos_findfirst( buffer, _A_NORMAL, &file_info ) == 0 ) {
            match_found = true;
            break;
        }
        #endif
    }

    // If we found a match return it, otherwise return the original.
    if( !match_found ) strcpy( buffer, name );

    return buffer;
}

