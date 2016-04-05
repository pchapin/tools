/*! \file    record_f.hpp
    \brief   Record file handling class.
    \author  Peter C. Chapin <PChapin@vtc.vsc.edu>

*/

#ifndef RECORD_F_HPP
#define RECORD_F_HPP

#include <fstream>

/*!
 * This class supports easy reading of text files which are organized as several lines where
 * each line contains a record consisting of several fields. The record files supported can have
 * a variable number of fields per line and can contain comments.
 */
class RecordFile {

  public:
    RecordFile(             // Opens file and allocates space for buffer.
      const char *file_name,
      int   search_method,
      int   length,
      char  comment,
      const char *delimit );
   ~RecordFile( );         // Closes file and frees buffer.

    char  **get_line( );   // Reads a line and breaks it into fields.
    int     get_length( ); // Returns the number of fields.
    bool    is_ok;         // =true if constructor works.
    bool    too_long;      // =false if too many fields in line.

    enum { DEFAULT };      // Search methods.

  private:
    std::ifstream the_file;   // Refers to actual file.
    char     comment_char;    // Defines comment character in file.
    int      line_length;     // Longest allowed line length in file.
    char    *buffer;          // Points at buffer to hold each raw line.
    const char *delimiters;   // String of delimiters for fields in the file.
    char    *parts[16];       // Points at each field (note fixed size).
    int      number_of_parts; // Number of fields.

  };

#endif
