/*! \file    splits.cpp
 *  \brief   String splitting functions.
 *  \author  Peter Chapin <chapinp@proton.me>
 */

#include "environ.hpp"

#include <stdio.h>
#include <string.h>

#include "misc.hpp"

/*!
 * This function accepts a string and breaks into all of its substrings with one call. It loads
 * up an array of char * with pointers to the substrings. The original string is modified by the
 * operation.
 */
bool split_string(
    char *buffer,                   // String to be split.
    const char *delimiters,         // String of delimiter characters.
    char *strings[],                // Array to be filled with pntrs to substrings.
    int   max_number_of_strings,    // Maximum number of substrings allowed.
    int  *actual_number_of_strings  // Actual number of substrings found.
  )
{
    char *buffer_pointer;         // Temporary pointer into buffer.
    bool  in_string = false;      // =false when *buf_ptr not pointing at str.
    bool  return_code = true;     // Returns false if too many substrings.

    // Initialize result and check for out of bounds input.
    *actual_number_of_strings = 0;
    if( max_number_of_strings < 0 ) return_code = false;
    else {

        // Sweep down string until null byte reached.
        buffer_pointer = buffer;
        while( *buffer_pointer != '\0' ) {

            // Try to process the current character only if no error so far.
            if( return_code == true ) {

                // If delimiter found while scanning an argument...
                if( ( strchr( delimiters, *buffer_pointer ) != NULL ) && ( in_string == true ) ) {
                    *buffer_pointer = '\0';
                    in_string = false;
                }

                // If non delimiter found while scanning "white" space...
                else if( ( strchr( delimiters, *buffer_pointer ) == NULL ) && ( in_string == false ) ) {

                    // Install only if space. If no space left set error flag.
                    if( *actual_number_of_strings < max_number_of_strings ) {
                        strings[*actual_number_of_strings] = buffer_pointer;
                        ( *actual_number_of_strings )++;
                        in_string = true;
                    }
                    else return_code = false;
                }
            }
            buffer_pointer++;
        }   // End of while loop.
    }       // End of overall if... else...

    return return_code;
}
