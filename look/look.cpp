/****************************************************************************
FILE          : look.cpp
LAST REVISION : 2003-12-21
SUBJECT       : Program to facilitate reading text files on-line.
PROGRAMMER    : (C) Copyright 2003 by Peter C. Chapin

This program keeps a simple bookmark for a file so that when I try to
read it again, it jumps to the bookmark.

Please send comments or bug reports to

     Peter C. Chapin
     Vermont Technical College
     Randolph Center, VT 05061
     pchapin@ecet.vsc.edu
****************************************************************************/

#include <iostream>
#include <fstream>
#include <string.h>

#include "paths.h"

using namespace std;

const int LINE_BUFFER_SIZE = 256 + 1;  // Used for lines from the text file.
const int BUFFER_SIZE = 128 + 1;       // Used for other input.
const int SCREEN_SIZE = 25;            // Number of rows on the screen.

int main(int argc, char **argv)
{
  int line_number  = 1;
  int line_count   = 1;
  int screen_count = 0;

  // Check the command line.
  if (argc != 2) {
    cerr << "usage: look: filename\n";
    return 1;
  }

  // What's the name of the bookmark file?
  char bookmark_name[BUFFER_SIZE];
  strcpy(bookmark_name, argv[1]);
  strcat(bookmark_name, ".bmrk");

  // Open it and (if we did) read in the current bookmark value.
  ifstream bookmark_in(bookmark_name);
  if (bookmark_in) {
    bookmark_in >> line_number;

    // Back up the bookmark a bit so the user isn't too confused.
    line_number -= SCREEN_SIZE - 2;
    bookmark_in.close();
  }
  if (line_number < 1) line_number = 1;
  
  // Open the main file.
  ifstream data_file(argv[1]);
  if (!data_file) {
    cerr << "Unable to open " << argv[1] << "\n";
    return 1;
  }

  char line_buffer[LINE_BUFFER_SIZE];
  char input_buffer[BUFFER_SIZE];

  // Advance the file to where we left off.
  while (line_count < line_number &&
         data_file.getline(line_buffer, LINE_BUFFER_SIZE)) {
    line_count++;
  }

  // Now read the file a screen at a time.
  while (data_file.getline(line_buffer, LINE_BUFFER_SIZE)) {
    cout << line_buffer << "\n";

    // Advance our records. Should we pause now?
    screen_count++;
    line_number++;
    if (screen_count == SCREEN_SIZE - 2) {

      // Wait for the user to do something.
      screen_count = 0;
      cout << ":";
      cin.getline(input_buffer, BUFFER_SIZE);

      switch (input_buffer[0]) {

      // If the user wants to abort, don't save the bookmark file.
      case 'A':
      case 'a':
        return 0;

      // If the user wants to quit, rewrite the bookmark file.
      case 'Q':
      case 'q': {
        ofstream bookmark_out(bookmark_name);
        if (bookmark_out) {
          bookmark_out << line_number;
          bookmark_out.close();
        }
        return 0;
      }

      // Give the user help.
      case 'H':
      case 'h':
        cout << "q => Quit (save bookmark).\n";
        cout << "a => Abort (do not save bookmark).\n";
        cout << "RETURN => Next page.\n";
        cout << "\n";
        cin.getline(input_buffer, BUFFER_SIZE);
        break;
      }
    }
  }
  return 0;
}
