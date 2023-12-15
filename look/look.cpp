/*! \file   look.cpp
 *  \brief  Program to facilitate reading text files.
 *  \author Peter Chapin <spicacality@kelseymountain.org>
 * 
 * This program keeps a simple bookmark for a file so that when I try to read it again, it jumps
 * to the bookmark. This program is very simple and very limited, but it is surprisingly useful!
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string.h>

#include "paths.hpp"  // Currently not used.

const int LINE_BUFFER_SIZE = 256 + 1;  // Used for lines from the text file.
const int BUFFER_SIZE = 128 + 1;       // Used for other input.
const int SCREEN_SIZE = 25;            // Number of rows on the screen.

int main( int argc, char **argv )
{
    int line_number  = 1;
    int line_count   = 1;
    int screen_count = 0;

    // Check the command line.
    if( argc != 2 ) {
        std::cerr << "usage: look [filename]" << std::endl;
        return EXIT_FAILURE;
    }

    // What's the name of the bookmark file?
    std::string bookmark_name( argv[1] );
    bookmark_name.append( ".bmrk" );

    // Open it and (if we did) read in the current bookmark value.
    std::ifstream bookmark_in( bookmark_name );
    if( bookmark_in ) {
        bookmark_in >> line_number;

        // Back up the bookmark a bit so the user isn't too confused.
        line_number -= SCREEN_SIZE - 2;
        bookmark_in.close( );
    }
    if( line_number < 1 ) line_number = 1;
  
    // Open the main file.
    std::ifstream data_file( argv[1] );
    if( !data_file ) {
        std::cerr << "Unable to open " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    char line_buffer[LINE_BUFFER_SIZE];
    char input_buffer[BUFFER_SIZE];

    // Advance the file to where we left off.
    while( line_count < line_number && data_file.getline( line_buffer, LINE_BUFFER_SIZE ) ) {
        line_count++;
    }

    // Now read the file a screen at a time.
    while( data_file.getline( line_buffer, LINE_BUFFER_SIZE ) ) {
        std::cout << line_buffer << std::endl;

        // Advance our records. Should we pause now?
        screen_count++;
        line_number++;
        if( screen_count == SCREEN_SIZE - 2 ) {

            // Wait for the user to do something.
            screen_count = 0;
            std::cout << ":";
            std::cin.getline( input_buffer, BUFFER_SIZE );
            
            // TODO: Check that the input buffer has something in it.
            switch( input_buffer[0] ) {

                // If the user wants to abort, don't save the bookmark file.
            case 'A':
            case 'a':
                return 0;

                // If the user wants to quit, rewrite the bookmark file.
            case 'Q':
            case 'q': {
                std::ofstream bookmark_out( bookmark_name );
                if (bookmark_out) {
                    bookmark_out << line_number << std::endl;
                    bookmark_out.close( );
                }
                return EXIT_SUCCESS;
            }

                // Give the user help.
            case 'H':
            case 'h':
                std::cout << "q => quit (save bookmark)" << std::endl;
                std::cout << "a => abort (do not save bookmark)" << std::endl;
                std::cout << "RETURN => next page" << std::endl;
                std::cout << std::endl;
                std::cin.getline( input_buffer, BUFFER_SIZE );
                break;
            }
        }
    }
    return 0;
}
