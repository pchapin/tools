/*! \file    record_f.cpp
    \brief   Record file handling class.
    \author  Peter C. Chapin <PChapin@vtc.vsc.edu>

*/

#include "environ.hpp"

#include <string.h>

#include "misc.hpp"
#include "record_f.hpp"

/*----------------------------------------------------------------------------
The constructor opens the file and allocates memory for a working buffer. The
search method specified tell the object where the record file might be
found. Currently only DEFAULT is supported; record files must be in the
default directory.

The comment character is the character used to mark the beginning of an
end-of-line style comment in the record file. All text after the first
occurance of this character is ignored. It can be '\0' if there are to be no
comments in the record file.
----------------------------------------------------------------------------*/

RecordFile::RecordFile(
  const char *file_name,     // File name. Can contain paths, etc.
  int   /* search_method*/,  // Where to find file. Must be DEFAULT (for now).
  int   length,              // Maximum length of line in record file.
  char  comment,             // Character which starts end-of-line comments.
  const char *delimit )      // String of characters which serve to delimit fields.
{
    // Initialize important members.
    buffer          = NULL;
    number_of_parts = 0;

    // Open the file and allocate space.
    the_file.open( file_name );
    if( !the_file ) {
        is_ok = false;
    }
    else if( ( buffer = new char[length + 2]) == NULL ) {
        is_ok = false;
    }
    else {

        // Save desired specs.
        delimiters   = delimit;
        comment_char = comment;
        line_length  = length;
        is_ok        = true;
    }
}

/*----------------------------------------------------------------------------
The destructor closes the file and frees the buffer.
----------------------------------------------------------------------------*/

RecordFile::~RecordFile( )
{
    if( buffer != NULL ) delete buffer;
    return;
}

/*----------------------------------------------------------------------------
The following function returns an array of character pointers. Each pointer in
the array points to a field from the next line of the record file. To determin
the number of fields, see the next function. Comments in the record file and
trailing white space (regardless of the delimiter characters) are already
stripped. The function returns NULL if there is nothing else in the file.
----------------------------------------------------------------------------*/

char **RecordFile::get_line( )
{
    if( !is_ok ) return NULL;

    char **return_value = NULL;
    char  *end_pointer;

    // Try to read a line from the file.
    if( the_file.getline( buffer, line_length + 1 ) ) {

        // Kill comments.
        if( ( end_pointer = strchr( buffer, comment_char ) ) != NULL ) *end_pointer = '\0';

        // Remove trailing white space.
        for( end_pointer = buffer; *end_pointer; end_pointer++ ) /* Null */ ;
        for( --end_pointer;
             end_pointer >= buffer && ( *end_pointer == ' ' || *end_pointer == '\t' );
             --end_pointer ) /* Null */ ;
        *++end_pointer = '\0';

        // Split the string into fields.
        too_long = split_string( buffer, delimiters, parts, 16, &number_of_parts );
        return_value = parts;
    }

    // Return pointer to fields or NULL.
    return return_value;
}

/*----------------------------------------------------------------------------
The following function returns the number of fields in the last line read from
the file.
----------------------------------------------------------------------------*/

int RecordFile::get_length( )
{
    if( !is_ok ) return 0;
    return number_of_parts;
}
