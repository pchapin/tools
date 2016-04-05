/*! \file    upgrade.cpp
    \brief   Simple directory synchronization program.
    \author  Peter C. Chapin <PChapin@vtc.vsc.edu>

This program can be used to upgrades files in a work directory. It scans a specified library
directory for newer versions of files in the work directory and copies the newer files into the
work directory.

USAGE: upgrade [switches] library_directory

I found that under Unix I needed to use curses to get getch(). While that could be made to work,
curses is overkill for a program of this nature. Thus I decided to go with a simpler interface
where the user must enter a response for each question (no hitting return to get the default)
and where the user must hit return after each response.
*/

#include "environ.hpp"

#if !(eOPSYS == eOS2 || eOPSYS == eWIN32 || eOPSYS == ePOSIX)
#error Only OS/2, Win32, or Unix supported!
#endif

#if eGUI != eNONE
#error No GUI support!
#endif

// Needed to implement stricmp().
#if eCOMPILER == eCODEWARRIOR
#include <cctype>
#endif

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>

#if eOPSYS == eWIN32
#include <windows.h>
#undef min
#undef max
#endif

#if eOPSYS == eOS2
#include <strstream.h>
#define INCL_DOSFILEMGR
#include <os2.h>
#endif

#if eOPSYS == ePOSIX
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#undef clear
#undef erase
#endif

#include "get_switch.hpp"

using namespace std;

// The following structure contains the information about a file that interests us: the name of
// the file and the date/time of its last modification. Note that the name does not contain any
// path part.
//
struct FileInfo {
    #if eOPSYS == eWIN32
    char     name[256+1];
    FILETIME modify_time;
    DWORD    file_size;
    #endif

    #if eOPSYS == eOS2
    char     name[256+1];
    FTIME    modify_time;
    FDATE    modify_date;
    ULONG    file_size;
    #endif

    #if eOPSYS == ePOSIX
    char     name[256+1];
    time_t   modify_time;
    long     file_size;
    #endif
};

int disable_interactive_mode = false;
  // =true when we want to disable interactive mode. Interactive mode is on by default (this is
  // a change from version 1.0).

// The command line switches this program understands.
SwitchInfo the_switches[] = {
  { 'i', bin_switch, &disable_interactive_mode, NULL, "Disable confirmation before each operation" }
  };
const int switch_count = sizeof(the_switches)/sizeof(SwitchInfo);


#if eOPSYS == eOS2

// The following overloaded operators facilitate the comparison of file date/time stamps. This
// is how it might be done in OS/2.

bool operator>( const FDATE &left, const FDATE &right )
{
    if( left.year >  right.year ) return true;
    if( left.year == right.year ) {
        if( left.month >  right.month ) return true;
        if( left.month == right.month ) {
            if( left.day > right.day ) return true;
        }
    }
    return false;
}

bool operator==( const FDATE &left, const FDATE &right )
{
  return left.year  == right.year  &&
         left.month == right.month &&
	 left.day   == right.day;
}

bool operator>( const FTIME &left, const FTIME &right )
{
    if( left.hours >  right.hours ) return true;
    if( left.hours == right.hours ) {
        if( left.minutes >  right.minutes ) return true;
        if( left.minutes == right.minutes ) {
            if( left.twosecs > right.twosecs ) return true;
        }
    }
    return false;
}

bool operator==( const FTIME &left, const FTIME &right )
{
    return left.hours   == right.hours   &&
           left.minutes == right.minutes &&
           left.twosecs == right.twosecs;
}
#endif

#if eOPSYS == eWIN32

// The following overloaded operators facilitate the comparison of file date/time stamps. This
// is how it might be done in Win32.

bool operator>( const FILETIME &left, const FILETIME &right )
{
    return CompareFileTime( &left, &right ) == 1;
}

bool operator==( const FILETIME &left, const FILETIME &right )
{
    return CompareFileTime( &left, &right ) == 0;
}
#endif

#if eCOMPILER == eCODEWARRIOR

// CodeWarrior does not supply stricmp(). Bummer. This version returns zero if the strings agree
// (without regard to case) and non-zero if they do not agree. This version does not attempt to
// return +/- 1 in the manner of strcmp(). I don't need that functionality here anyway.

int stricmp( const char *s1, const char *s2 )
{
  while( *s1 ) {
    if( toupper( *s1 ) != toupper( *s2 ) ) return 1;
    s1++;
    s2++;
  }
  if( *s2 == '\0' ) return 0;
  return 1;
}
#endif


char *adjust_date( char *ANSI_date )
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


//
// scan_directory( char *, list<FileInfo> & )
//
// The following function scans the named directory (no trailing directory delimiter) and
// produces a list of all files in that directory with the information on each file that
// interests us.
//
void scan_directory( char *directory_name, list<FileInfo> &the_list )
  {
      // Make sure we don't have anything in our list.
      the_list.erase( the_list.begin( ), the_list.end( ) );


      // Scan the directory the Win32 way.
      #if eOPSYS == eWIN32
      char            file_name[256+1];
      WIN32_FIND_DATA file_information;
      HANDLE          search_handle;
      bool            done = false;

      strcpy( file_name, directory_name );
      strcat( file_name, "\\*.*" );

      search_handle = FindFirstFile( file_name, &file_information );
      if( search_handle == INVALID_HANDLE_VALUE ) return;

      while( !done ) {
        
          // Skip directories. I suppose we should also disregard hidden files, etc.
          if( !( file_information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {
              FileInfo new_item;

              strcpy( new_item.name, file_information.cFileName );
              new_item.modify_time = file_information.ftLastWriteTime;
              new_item.file_size   = file_information.nFileSizeLow;
              the_list.push_back( new_item );
          }

          if( FindNextFile( search_handle, &file_information ) == FALSE ) done = true;
      }
      FindClose( search_handle );
      #endif


      // Scan the directory the OS/2 way.
      #if eOPSYS == eOS2
      char          file_name[256+1];
      HDIR          search_handle = HDIR_CREATE;
      FILEFINDBUF3  file_information;
      ULONG         search_count  = 1;
      bool          done          = false;

      strcpy( file_name, directory_name );
      strcat( file_name, "\\*.*" );

      // Locate the first match.
      if( DosFindFirst(
          file_name,                      // The wildcard spec.
          &search_handle,                 // Place to put the search handle.
          FILE_ARCHIVED | FILE_READONLY,  // Includes files without these attribs too.
          &file_information,              // Where to put the goods.
          sizeof( file_information ),
          &search_count,                  // Number of file informations we want now.
          FIL_STANDARD ) !=0 ) return;    // We want level 1 information (no EAs).

      // Did we find anything?
      if( search_count == 0 ) done = true;

      while( !done ) {
          FileInfo new_item;

          strcpy( new_item.name, file_information.achName );
          new_item.modify_time = file_information.ftimeLastWrite;
          new_item.modify_date = file_information.fdateLastWrite;
          new_item.file_size   = file_information.cbFile;
          the_list.push_back( new_item );

          // Get the next file.
          search_count = 1;
          if( DosFindNext(
              search_handle,
              &file_information,
              sizeof( file_information ),
              &search_count ) != 0 ) done = true;

          if( search_count == 0 ) done = true;
      }

      DosFindClose( search_handle );
      #endif


      // Scan the directory using the Unix way. In this case, the information in a FileInfo
      // object has to be collected from two different sources.
      //
      #if eOPSYS == ePOSIX
      char    file_name[256+1];
      char   *end_pointer;
      DIR    *directory_file;
      struct dirent *directory_entry;
      struct stat    file_status;

      // Get the name buffer ready for the file names.
      strcpy( file_name, directory_name );
      strcat( file_name, "/" );
      end_pointer = strchr( file_name, '\0' );

      if( ( directory_file = opendir( directory_name ) ) == 0 ) return;
      while( ( directory_entry = readdir( directory_file ) ) != 0 ) {

          // Is this a normal file?
          strcat( file_name, directory_entry->d_name );
          stat( file_name, &file_status );
          if( S_ISREG( file_status.st_mode ) ) {
              FileInfo new_item;

              strcpy( new_item.name, directory_entry->d_name );
              new_item.modify_time  = file_status.st_mtime;
              new_item.file_size = file_status.st_size;
              the_list.push_back( new_item );
          }
          *end_pointer = '\0';
      }
      closedir( directory_file );
      #endif
  }


// The following function compares the date/time on two files and figures out which is the newer
// one. If the file in the library is newer it returns true. Otherwise it returns false.
//
bool library_newer( const FileInfo &file_status, const FileInfo &lib_status )
{
      
    #if eOPSYS == eOS2
    if( lib_status.modify_date > file_status.modify_date ) return true;

    if( lib_status.modify_date == file_status.modify_date &&
        lib_status.modify_time >  file_status.modify_time ) return true;

    if( lib_status.modify_date  == file_status.modify_date &&
        lib_status.modify_time  == file_status.modify_time &&
        lib_status.file_size != file_status.file_size ) {

        cout << "\n  WARNING! Date/time stamps are the same, but sizes differ! NOT UPGRADING!"
             << endl;
    }
    return false;
    #endif

    #if eOPSYS == ePOSIX || eOPSYS == eWIN32
    if( lib_status.modify_time  >  file_status.modify_time ) return true;
    if( lib_status.modify_time  == file_status.modify_time &&
        lib_status.file_size != file_status.file_size ) {

        cout << "\n  WARNING! Date/time stamps are the same, but sizes differ! NOT UPGRADING!"
             << endl;
    }
    return false;
    #endif
  }


// The following function copies a file out of the library into the current directory.
//
void copy(char *library_file_name)
{
    char ch;
    
    // If we're doing interactive mode, ask the user if this is okay first.
    if( !disable_interactive_mode ) {
        cout << " Needs upgrading. Do it? y/n " << flush;

        cin >> ch;
        if( ch == 'n' || ch == 'N' ) {
            cout << "  (skipped)" << endl;
            return;
        }
    }

    #if eOPSYS == eWIN32
    char  target_name[256+1];
    char *directory_delimiter = strrchr( library_file_name, '\\' );

    strcpy( target_name, ".\\" );
    strcat( target_name, directory_delimiter + 1 );
    CopyFile( library_file_name, target_name, FALSE );
    #endif

    #if eOPSYS == eOS2
    char       command_buffer[256+1];
    ostrstream command_formatter( command_buffer, 257 );

    command_formatter << "copy " << library_file_name << " . >nul\n" << ends;
    system( command_buffer );
    #endif


    #if eOPSYS == ePOSIX
    ostringstream command_formatter;

    // The -p option causes the date/time stamp to get copied as well. How universal is this
    // option?
    //
    command_formatter << "cp -p " << library_file_name << " .";
    system( command_formatter.str( ).c_str( ) );
    #endif

    cout << "  (upgraded)" << endl;
}


//
// Main Program
//
int main( int argc, char **argv )
{
    list<FileInfo> file_list;
    list<FileInfo> library_list;

    #if eOPSYS == eOS2
    cout << "\n"
            "UPGRADE for OS/2 (version 1.2a) " << adjust_date( __DATE__ ) << "\n"
            "Public Domain Software by Peter Chapin\n" << endl;
    #endif

    #if eOPSYS == eWIN32
    cout << "\n"
            "UPGRADE for Win32 (version 1.2a) " << adjust_date( __DATE__ ) << "\n"
            "Public Domain Software by Peter Chapin\n" << endl;
    #endif

    #if eOPSYS == ePOSIX
    cout << "\n"
            "UPGRADE for Unix (version 1.2a) " << adjust_date( __DATE__ ) << "\n"
            "Public Domain Software by Peter Chapin\n" << endl;
    #endif

    // Analyze command line looking for options switches.
    argc = get_switchs( argc, argv, the_switches, switch_count );

    if( argc != 2 ) {
        cout << "USAGE: upgrade [switches] library_dir\n\n"
                "Where 'library_dir' => Directory where potentially new versions exist.\n\n"
                "Legal switches are:\n" << endl;
        print_usage( the_switches, switch_count, cout );
        return 1;
    }


    // Get lists of files in the current directory and in the lib directory.
    scan_directory( ".", file_list );
    scan_directory( argv[1], library_list );

    char  library_file_name[256+1];
    char *end_pointer;

    // Get ready to construct full library file names. We need them below.
    #if eOPSYS == eWIN32 || eOPSYS == eOS2
    strcpy( library_file_name, argv[1] );
    strcat( library_file_name, "\\" );
    end_pointer = strchr( library_file_name, '\0' );
    #endif

    #if eOPSYS == ePOSIX
    strcpy( library_file_name, argv[1]);
    strcat( library_file_name, "/" );
    end_pointer = strchr( library_file_name, '\0' );
    #endif

    // Now step down the list of files in the current directory.
    list<FileInfo>::iterator file_stepper;
    for( file_stepper = file_list.begin( ); file_stepper != file_list.end( ); file_stepper++ ) {

        // If we've got a file, let's see if there's a version in the lib directory.
        list<FileInfo>::iterator library_stepper;
        for( library_stepper = library_list.begin( );
             library_stepper != library_list.end( );
             library_stepper++ ) {

            #if eOPSYS == ePOSIX
            if( strcmp( file_stepper->name, library_stepper->name ) == 0 ) break;
            #endif

            #if eOPSYS == eWIN32 || eOPSYS == eOS2
            if ( _stricmp( file_stepper->name, library_stepper->name ) == 0 ) break;
            #endif
        }

        // There is!
        if( library_stepper != library_list.end( ) ) {
            cout << "Considering file: " << file_stepper->name << "." << flush;
            if( library_newer( *file_stepper, *library_stepper ) ) {
                strcat( library_file_name, file_stepper->name );
                copy( library_file_name );
            }
            cout << endl;
        }

        // Delete the file name off the path in the library_file_name array.
        *end_pointer = '\0';
    }
    return 0;
}
