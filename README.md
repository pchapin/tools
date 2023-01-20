
Tools
=====

This folder contains tools and supplementary programs of interest to both developers and users.
The tools are in various states of completion. Most compile and work, but some are unfinished.
Many are quite old and could stand reformatting and updating. Many could use additional
functionality (some can only process C programs, for example, and not C++ programs). Most could
use cross platform build control. Probably a single build system for all tools should be
created.

+   bracket

    Scans a C program looking for mismatched brackets of various kinds.

+   cyclo

    Measures the cyclomatic complexity of functions in a C program. This program could and
    should be upgraded to handle C++ methods. Note that cyclomatic complexity is a measure of
    the control flow complexity of a function. It can be used to guide testing and to guide
    decisions about when it is appropriate to break a large function into smaller pieces.

+   depend

    This tool scans over the source files of your program and builds a list of header file
    dependencies for each source file. The output is in a form suitable for inclusion in a
    makefile.

+   ktour

    A sample program that solves the knight's tour chess problem. This isn't exactly a tool, but
    it's a fun toy.

+   look

    A text file reading tool that saves current locations of files being read in a bookmark
    file. It is intended to be a standalone tool, but its functionality might one day be good to
    incorporate into Y. This tool is incomplete.

+   m

    A simple text mode menu manager.

+   makebig

    A program that makes large text files on demand. Useful to testing programs that process
    text files (such as a text editor).

+   makedirlist

    Creates an index file for a directory hierarchy.

+   makepass

    A simple password generator. The passwords it generates are not particularly secure but they
    are intended to be easy to remember.

+   retab

    A simple program that changes tabs to reflect different tab stop settings. It can also add
    and remove tabs from source files.

+   roff

    A very simple text formatting tool for use with writing text documentation.

+   softlock

    An MS-DOS TSR program that locks the keyboard after a certain period of inactivity.

+   text2c

    A tool that reads a text file and writes a C declaration of an array of pointers containing
    that text. This is useful for loading arbitrary text segments into a C program. Y uses this
    for building its help screens.

+   textscan

    A simple tool that scans a text file and reports some interesting statistics about it. The
    tool can be used to locate "unusual" characters in a text file that are invisible and that
    might be causing trouble for other text processing programs.

+   upgrade

    A simple file synchronization tool. It copies newer files (as based on timestamps) from a
    "library" directory into a specified target directory.
