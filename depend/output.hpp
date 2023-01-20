/*! \file    output.hpp
 *  \brief   Output handling functions.
 *  \author  Peter Chapin <chapinp@proton.me>
 */

#ifndef OUTPUT_HPP
#define OUTPUT_HPP

bool open( char *name );
  // Opens the named output file and writes preamble.

void start( char *name );
  // Prepares a dependency list.

bool already_scanned( char *name );
  // Returns YES if this file already on current dependency list.

void emit( char *name );
  // Installs a name in the dependency list.

void flush( char continuation );
  // Writes the dependency list. The character argument is the line continuation required in the
  // MakeFile.

#endif

