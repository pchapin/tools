/*! \file    linescan.hpp
 *  \brief   Declaration of functions that scan lines looking for #include.
 *  \author  Peter Chapin <chapinp@proton.me>
 */

#ifndef LINESCAN_HPP
#define LINESCAN_HPP

extern void handle_line( char *line );
  // This function figures out if the given line is a #include and, if so, it writes information
  // into the current dependency list as well as calls handle_file( ) so that this include file
  // gets scanned.

#endif
