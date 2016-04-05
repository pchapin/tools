/*! \file    filename.hpp
    \brief   Declaration of file name handling functions.
    \author  Peter C. Chapin <PChapin@vtc.vsc.edu>

*/

#ifndef FILENAME_HPP
#define FILENAME_HPP

const int FILENAME_LENGTH = 256;

void set_directory_list( const char *new_directory_list );
  // This function takes a semicolon delimited list of directory names and inserts the names
  // into an internal list for later use.

char *match_name( char *name );
  // This function takes a simple filename and returns either the name it's been given or the
  // "true" filename with the directory path prepended. The prepending of a directory path
  // occurs if the file resides in one of the directories in the current directory list (see
  // above). The pointer returned by this function points at internal, static storage.

#endif


