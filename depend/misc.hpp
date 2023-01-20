/*! \file    misc.hpp
 *  \brief   Declarations of various utility functions.
 *  \author  Peter Chapin <chapinp@proton.me>
 */

#ifndef MISC_HPP
#define MISC_HPP

extern bool split_string(
    char *buffer,                   // String to be split.
    const char *delimiters,         // String of delimiter characters.
    char *strings[],                // Array to be filled with pntrs to substrings.
    int   max_number_of_strings,    // Maximum number of substrings allowed.
    int  *actual_number_of_strings  // Actual number of substrings found.
);

extern char *adjust_date( const char * );

#endif
