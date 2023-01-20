/****************************************************************************
FILE          : makedirlist
LAST REVISION : January 2023
SUBJECT       : Program to make 00_index.txt files in a directory.
PROGRAMMER    : (C) Copyright 2023 by Peter Chapin

This program scans a directory and synchronizes it with the file 00_index.txt. If the index file
does not exist, it is created. This program will remove references in 00_index.txt to files that
no longer exist in the directory. It will add references for files that do exist but that are
not already in 00_index.txt.

The file 00_index.txt is intended to store short, textual descriptions of the files. The format
is one file on each line. The name of the file is first, followed by the desciption. Lines can
be as long as needed, but excessively long lines are discouraged. This program will never write
a file description. It is up to some person to do that. However, this program will retain
descriptions that are already in 00_index.txt when it updates that file.

File names are sorted in 00_index.txt.

Usage: makedirlist [-r] dir_name [dir_name ... ]

The -r option causes the program to recursively scan the specified directories. If present the
-r option must be first. Command line parameters that are not directory names are ignored.

BUGS

The current version of this program doesn't deal with owner/group issues on the 00_index.txt
file that it creates. Probably some command line options should be defined that allow the user
to set those things. Should the behavior differ for index files that already exist as compared
to index files that are created? In addition, this program forces the permissions on the index
files to be 664. Again, a command line option should probably be defined to allow the user to
set different permissions.

Please send comments or bug reports to

     Peter Chapin
     chapinp@proton.me
****************************************************************************/

#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <set>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

using namespace std;

extern int optind;
  // Used by getopt() to specify where the real arguments start.

bool recurse = false;
  // =true when recursive directory scanning requested.


//------------------------------
//           Functions
//------------------------------

void read_index( const string &path, map<string, string, less<string>> &index );
void get_files( const string &path, set<string, less<string>> &names );
void process_subdirectories( const string &path );
void scan_directory( const string &path );


//
// read_index
//
// The following function reads 00_index.txt from the directory specified by path. It loads up
// the map with the information from that file. The keys of the map are the file names. The
// values are the file descriptions.
//
void read_index( const string &path, map<string, string, less<string>> &index )
{
    string file_name( path );
    file_name.append( "/" );
    file_name.append( "00_index.txt" );

    ifstream index_file( file_name.c_str( ) );
    string   line;

    // If the file could not be open, that's not an error. It does mean that there is no
    // information to put into the index map.
    if( !index_file ) return;

    // Read the file a line at a time.
    while( getline( index_file, line ) ) {

        // Locate the positions of the end of the name and the start of the description.
        string::size_type end_of_name = line.find_first_of( " \t" );
        if( end_of_name == string::npos ) continue;

        string::size_type start_of_description = line.find_first_not_of( " \t", end_of_name );
        if( start_of_description == string::npos ) continue;

        // Get the substrings.
        string name( line.substr( 0, end_of_name ) );
        string description( line.substr( start_of_description ) );

        // Store them into the index map.
        index[name] = description;
    }
}


//
// get_files
//
// The following function scans the directory specified by path and gets a listing of all files
// and directories in that directory. It will insert all those names into the specified set.
// This function will ignore names that start with a '.' character as well as the two names
// 00_index.txt and 00_index.txt~.
//
void get_files( const string &path, set<string, less<string>> &names )
{
    DIR    *directory_handle = opendir( path.c_str( ) );
    dirent *directory_entry;

    if( directory_handle == 0 ) return;
    while( ( directory_entry = readdir( directory_handle ) ) != 0 ) {

        // Ignore files with names starting with '.' and ignore the name 00_index.txt.
        if( directory_entry->d_name[0] == '.' ) continue;
        if( strcmp(directory_entry->d_name, "00_index.txt"  ) == 0 ) continue;
        if( strcmp(directory_entry->d_name, "00_index.txt~" ) == 0 ) continue;

        // Also ignore emacs any odd file with a '~' in its name. (Such as emacs backup files).
        if( strchr( directory_entry->d_name, '~' ) != 0 ) continue;

        // Otherwise compose the full file name and stat() it to figure out what it is.
        string full_name( path );
        full_name.append( "/" );
        full_name.append( directory_entry->d_name );

        // If it's an ordinary file or a directory, remember its name.
        struct stat file_info;
        if( stat( full_name.c_str( ), &file_info ) != 0 ) continue;
        if( S_ISREG( file_info.st_mode ) || S_ISDIR( file_info.st_mode ) ) {
            //      string short_name( directory_entry->d_name );
            names.insert( directory_entry->d_name );
        }
    }
    closedir( directory_handle );
}


//
// sync
//
// The following function does the work of synchronizing the index file with what is actually in
// the directory. No files are manipulated here. That is done in other functions.
//
void sync( const set<string, less<string>> &real_names, map<string, string, less<string>> &index )
{
    map<string, string, less<string>> temp_index;

    // Remove entries from the index that are for files that are gone.
    map<string, string, less<string>>::iterator index_scan;
    for( index_scan = index.begin( ); index_scan != index.end( ); index_scan++ ) {

        if( real_names.find( ( *index_scan ).first) != real_names.end( ) )
            temp_index[( *index_scan ).first] = ( *index_scan ).second;
    }
    index = temp_index;

    // Add new entries for files in the real directory that are not yet in the index.
    set<string, less<string>>::iterator real_scan;
    for( real_scan = real_names.begin( ); real_scan != real_names.end( ); real_scan++ ) {
        if( index.find( *real_scan ) == index.end( ) )
            index[*real_scan] = "?";
    }
}


//
// write_index
//
// The following function writes the file 00_index.txt into the directory specified by path.
//
void write_index( const string &path, const map<string, string, less<string>> &index )
{
    string file_name( path );
    file_name.append( "/" );
    file_name.append( "00_index.txt" );

    ofstream index_file( file_name.c_str( ) );

    // If the file could not be open, blow out of here. (Permission problem? Path not really a
    // directory?)
    if( !index_file ) return;

    // Visit every element in the map.
    map<string, string, less<string>>::const_iterator p;
    for( p = index.begin( ); p!= index.end( ); ++p ) {

        // This hack is because gcc's string class doesn't honor the iostream manipulators.
        const string &name = ( *p ).first;
        int   length = name.size( );
        int   extra  = ( length < 20 ) ? 20 - length : 1;
        index_file << name << setw( extra ) << " " << ( *p ).second << endl;
    }

    // Set permissions on the file.
    chmod( file_name.c_str( ), 0664 );
}


//
// process_subdirectories
//
// The following function scans the directory specified by path and for each subdirectory found
// calls scan_directory to handle 00_index.txt in that subdirectory. It is used to recursively
// descend the directory tree when the user requests that action.
//
void process_subdirectories( const string &path )
{
    DIR    *directory_handle = opendir( path.c_str( ) );
    dirent *directory_entry;

    if( directory_handle == 0 ) return;
    while( ( directory_entry = readdir( directory_handle ) ) != 0 ) {

        // Ignore files with names starting with '.'
        if( directory_entry->d_name[0] == '.' ) continue;

        // Otherwise compose the full file name.
        string full_name( path );
        full_name.append( "/" );
        full_name.append( directory_entry->d_name );

        // If it's a directory call scan_directory (recursively).
        struct stat file_info;
        if( stat( full_name.c_str( ), &file_info ) != 0 ) continue;
        if(S_ISDIR( file_info.st_mode ) ) {
            scan_directory( full_name );
        }
    }
    closedir( directory_handle );
}


//
// scan_directory
//
// This function does the main work of the program. It scans the directory named in its
// parameter and sychronizes 00_index.txt. It then processes subdirectories recursively if that
// has been requested.
//
void scan_directory( const string &path )
{
    set<string, less<string>>         real_names;  // Names from the directory listing.
    map<string, string, less<string>> index;       // Digested form of 00_index.txt.

    // Read in the current 00_index.txt if there is one.
    read_index( path, index );

#ifdef NEVER
    // This is for testing/debugging purposes.
    cout << "Contents of 00_index.txt for " << path << endl;

    map<string, string, less<string>>::iterator p;
    for( p = index.begin( ); p != index.end( ); ++p ) {
        cout << "File: " << ( *p ).first << ", has description: " << ( *p ).second << endl;
    }
#endif

    // Scan directory for all ordinary files and subdirectories.
    get_files( path, real_names );

#ifdef NEVER
    // This is for testing/debugging purposes.
    cout << "List of file/directories in " << path << endl;

    set< string, less<string> >::iterator p;
    for( p = real_names.begin( ); p != real_names.end( ); ++p ) {
        cout << "\t" << *p << endl;
    }
#endif

    // Synchronize 00_index.txt with the directory.
    sync( real_names, index );

    // Write out the new 00_index.txt
    write_index( path, index );

    // Scan directory for all subdirectories if requested.
    if( recurse ) process_subdirectories( path );
}


//---------------------------------
//           Main Program
//---------------------------------

int main( int argc, char **argv )
{
    int    ch;
    // string group_name;      // The name of the group for 00_index.txt
    // bool   group_specified; // =true if group_name is to be used.

    // Scan the command line looking for options.
    while( ( ch = getopt( argc, argv, "r" ) ) != EOF ) {
        switch( ch ) {

            // They want to recurse. Remember that for later.
        case 'r': recurse = true; break;

            // Unknown option encountered. Message already printed by getopt().
        case '?': exit( 1 ); break;
        } 
    }

    // Step down the command line getting directory names as we go.
    for( int i = optind; i < argc; i++ ) {
        string path( "." );
        path = argv[i];

        // Process this directory. We should verify that it is really a directory first.
        scan_directory( path );
    }

    return 0;
}
