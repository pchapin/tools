
Upgrade
=======

This program simplifies the problem of upgrading the files in a directory with new versions of
those files as they exist in a library directory. Type 'upgrade' at the prompt for a usage
message. Keep in mind that the phrase "library directory" refers to an archive where potentially
new versions of a file exist. Upgrade will always use the current directory to locate files to
perhaps upgrade. In addition, upgrade will look at every file in the current directory to see if
a match exists in the library directory.

For example:

upgrade \prog\mylib

This command will see if a new copy of each .cpp file in the current directory exists in the
\prog\mylib directory. For each new version found, upgrade will copy that file into the current
directory. It will prompt for confirmation before each copy operation. You can disable the
prompting with the '-i' command line option.


Revision History
----------------

+   Version 1.0 (1993):

    Works and is useful.

+   Version 1.1 (December 28, 1996):

    Converted the entire program to C++. Upgraded internal modules to their most recent
    versions. Compiled for Win32 console mode. Fixed annoying bug: Since I was scanning the
    target directory while coping files into it from the source directory, I would sometimes
    consider the same file more than once (the original and the copy). I fixed this by scanning
    the target directory completely before starting any copy operations. The program now uses
    STL lists.

    I also changed the sense of the -i command line option. In version 1.0 you had to specify -i
    to get interactive operation. In this version you have to specify -i to *avoid* getting
    interactive operation. I found that I always used the program interactively so it seemed to
    make sense to make that behavior the default.

+   Version 1.2 (Early 1998):

    Ported the program to Unix. DOS support has been dropped. Changed the command line syntax so
    that you can no longer supply a wildcard spec to limit the files considered. Upgrade will
    now consider *all* files in the current directory. This was done in part because
    implementing the wild card spec on Unix was annoying and because I've found that I always
    use "*.*" anyway.

    Fixed internal bug with the directory scans under Win32 and OS/2. It now properly closes the
    scans.

+   Version 1.2a (April 2009):

    Updated the code to a more modern style. Ported to Visual Studio 2008.
