/****************************************************************************
FILE          : makebig.cpp
SUBJECT       : Program to create very large text files.
PROGRAMMER    : (C) Copyright 2005 by Peter C. Chapin

To Do:

+ Purge the get_switches module. It isn't open source and it is therefor nasty (well, in this
  context at least). Write a version of getopt for the Open Watcom library?

+ BUG: If we run out of bytes in the middle of printing the line header we will output a file
  that is too large since the entire line header is printed regardless (should prepare line
  header in an internal buffer and only print the part that is allowed).

+ Add an option for randomizing line widths to simulate semi-realistic text files.

+ Add an option for outputing the entire file as a single line.

+ Use 64 bit counters so that files >4 GBytes can be made. Add a corresponding -g option.

+ Add options for supporting binary data: random data, all bytes equal data, etc.

Please send comments or bug reports to

     Peter C. Chapin
     Vermont Technical College
     Randolph Center, VT 05061
     PChapin@vtc.vsc.edu
****************************************************************************/

#include <iostream>
#include <stdio.h>

#include "environ.hpp"
#include "get_switch.hpp"

#define K 1024

//
// Global data that can be changed with command line switches.
//
int n_lines    =  0;
int n_bytes    =  0;
int n_kbytes   =  0;
int n_mbytes   =  0;
int line_width = 50;

//
// Allowed command line switches.
//
SwitchInfo my_switches[] = {
  { 'l', int_switch, &n_lines, NULL,
    "Number of lines desired (no default)" },

  { 'b', int_switch, &n_bytes, NULL,
    "Number of bytes desired (no default)" },

  { 'k', int_switch, &n_kbytes, NULL,
    "Number of kilobytes desired (no default)" },

  { 'm', int_switch, &n_mbytes, NULL,
    "Number of megabytes desired (no default)" },

  { 'w', int_switch, &line_width, NULL,
    "Number of characters per line desired (default = 50)" }
};
const int switch_count = sizeof( my_switches ) / sizeof( SwitchInfo );

// +++++++++++++++++++++++++++++++++++++++++
// This function actually prints the results
// +++++++++++++++++++++++++++++++++++++++++

void output_text( )
{
    char letter = '!';  // The first printing character in the ASCII set.

    // If we are trying to print by line count...
    if( n_lines != 0 ) {

        // For each line...
        for( int line_counter = 1; line_counter <= n_lines; line_counter++ ) {

            // Print a line header of eight characters that gives the line number and then the
            // line itself.
            // 
            printf( "%06d: ", line_counter );
            for( int char_counter = 1; char_counter <= line_width - 8; char_counter++ ) {
                putchar( letter );
            }
            printf( "\n" );

            // Advance to the next letter in sequence.
            letter++;
            if( letter > '~' ) letter = '!';
        }
    }

    // Otherwise we are trying to print by byte count...
    else {
        bool line_start   = true; // =false if we are not at the start of a line.
        int  line_counter = 1;    // The line we are to print next.
        int  char_counter = 1;    // The character we are to print next.

        // For all characters in the file...
        for( int byte_counter = 1; byte_counter <= n_bytes; byte_counter++ ) {

            // If this is the start of a line, print the line header.
            if( line_start ) {
                printf( "%06d: ", line_counter );
                line_counter++;
                line_start    = false;
                byte_counter += 8;
            }

            // Print the letter and advance after each letter.
            putchar( letter );
            char_counter++;
            letter++;
            if( letter > '~' ) letter = '!';

            // If we've printed everything that's supposed to be on this line, go to the next
            // line.
            // 
            if( char_counter > line_width - 8 ) {
                printf( "\n" );

                #if eOPSYS == eWIN32
                byte_counter += 2;    // This is because '\n' is CRLF.
                #endif

                #if eOPSYS == ePOSIX
                byte_counter += 1;    // This is because '\n' is LF.
                #endif

                char_counter  = 1;
                line_start    = true;
            }
        }
    }
}

// ++++++++++++
// Main Program
// ++++++++++++

int main( int argc, char **argv )
{
    fprintf( stderr, "makebig (v1.0b) 2005-08-11\n"
                     "(C) Copyright 2005 by Peter C. Chapin\n\n" );

    // Extract the switch information from the command line.
    argc = get_switchs( argc, argv, my_switches, switch_count );

    // Compute total number of bytes requested.
    n_bytes += K * n_kbytes;
    n_bytes += K * K * n_mbytes;

    // Print error messages if necessary.
    if( n_lines == 0 && n_bytes == 0 ) {
        fprintf( stderr,
                 "ERROR: You must specify the desired number of lines or bytes!\n\n" );
        print_usage( my_switches, switch_count, std::cerr );
        return 1;
    }
    if ( n_lines != 0 && n_bytes != 0 ) {
        fprintf( stderr,
                 "WARNING: Lines overrides bytes.\n"
                 "         The number of bytes you specified will be ignored.\n" );
    }
    if( line_width <= 8 ) {
        fprintf( stderr, "ERROR: You must use a line width greater than 8.\n" );
        return 1;
    }

    output_text( );
    return 0;
}
