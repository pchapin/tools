/****************************************************************************
FILE          : makedirlist
LAST REVISION : March 1999
SUBJECT       : Program to make 00_index.txt files in a directory.
PROGRAMMER    : (C) Copyright 1999 by Peter Chapin

This program scans a directory and synchronizes it with the file
00_index.txt. If the index file does not exist, it is created. This
program will remove references in 00_index.txt to files that no longer
exist in the directory. It will add references for files that do exist
but that are not already in 00_index.txt.

The file 00_index.txt is intended to store short, textual descriptions
of the files. The format is one file on each line. The name of the
file is first, followed by the desciption. Lines can be as long as
needed, but excessively long lines are discouraged. This program will
never write a file description.  It is up to some person to do
that. However, this program will retain descriptions that are already
in 00_index.txt when it updates that file.

File names are sorted in 00_index.txt.

Usage: makedirlist [-r] dir_name [dir_name ... ]

The -r option causes the program to recursively scan the specified
directories. If present the -r option must be first. Command line
parameters that are not directory names are ignored.

BUGS

The current version of this program doesn't deal with owner/group
issues on the 00_index.txt file that it creates. Probably some command
line options should be defined that allow the user to set those
things. (Should the behavior differ for index files that already exist
as compared to index files that are created?) In addition, this
program forces the permissions on the index files to be 664. Again, a
command line option should probably be defined to allow the user to
set different permissions.

Please send comments or bug reports to

     Peter Chapin
     P.O. Box 317
     Randolph Center, VT 05061
     pchapin@twilight.vtc.vsc.edu
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <set>
#include <string>

using namespace std;

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

extern int optind;
  // Used by getopt() to specify where the real arguments start.

bool Recurse = false;
  // =true when recursive directory scanning requested.


//------------------------------
//           Functions
//------------------------------

void Read_Index(const string &Path, map< string, string, less<string> > &Index);
void Get_Files(const string &Path, set< string, less<string> > &Names);
void Process_Subdirectories(const string &Path);
void Scan_Directory(const string &Path);


//
// Read_Index
//
// The following function reads 00_index.txt from the directory specified by
//   Path. It loads up the map with the information from that file. The keys
//   of the map are the file names. The values are the file descriptions.
//
void Read_Index(const string &Path, map< string, string, less<string> > &Index)
{
  string File_Name(Path);
  File_Name.append("/");
  File_Name.append("00_index.txt");

  ifstream Index_File(File_Name.c_str());
  string   Line;

  // If the file could not be open, that's not an error. It does mean that there
  //   is no information to put into the Index map.
  if (!Index_File) return;

  // Read the file a line at a time.
  while (getline(Index_File, Line)) {

    // Locate the positions of the end of the name and the start of the description.
    int End_Of_Name = Line.find_first_of(" \t");
    if (End_Of_Name == string::npos) continue;

    int Start_Of_Description = Line.find_first_not_of(" \t", End_Of_Name);
    if (Start_Of_Description == string::npos) continue;

    // Get the substrings.
    string Name(Line.substr(0, End_Of_Name));
    string Description(Line.substr(Start_Of_Description));

    // Store them into the index map.
    Index[Name] = Description;
  }
}


//
// Get_Files
//
// The following function scans the directory specified by Path and gets
//   a listing of all files and directories in that directory. It will
//   insert all those names into the specified set. This function will
//   ignore names that start with a '.' character as well as the two names
//   00_index.txt and 00_index.txt~.
//
void Get_Files(const string &Path, set< string, less<string> > &Names)
{
  DIR    *Directory_Handle = opendir(Path.c_str());
  dirent *Directory_Entry;

  if (Directory_Handle == 0) return;
  while ((Directory_Entry = readdir(Directory_Handle)) != 0) {

    // Ignore files with names starting with '.' and ignore the name 00_index.txt.
    if (Directory_Entry->d_name[0] == '.') continue;
    if (strcmp(Directory_Entry->d_name, "00_index.txt" ) == 0) continue;
    if (strcmp(Directory_Entry->d_name, "00_index.txt~") == 0) continue;

    // Also ignore emacs any odd file with a '~' in its name. (Such as emacs backup files).
    if (strchr(Directory_Entry->d_name, '~') != 0) continue;

    // Otherwise compose the full file name and stat() it to figure out what it is.
    string Full_Name(Path);
    Full_Name.append("/");
    Full_Name.append(Directory_Entry->d_name);

    // If it's an ordinary file or a directory, remember its name.
    struct stat File_Info;
    if (stat(Full_Name.c_str(), &File_Info) != 0) continue;
    if (S_ISREG(File_Info.st_mode) || S_ISDIR(File_Info.st_mode)) {
      //      string Short_Name(Directory_Entry->d_name);
      Names.insert(Directory_Entry->d_name);
    }
  }
  closedir(Directory_Handle);
}


//
// Sync
//
// The following function does the work of synchronizing the index file with what
//   is actually in the directory. No files are manipulated here. That is done in
//   other functions.
//
void Sync(const set< string, less<string> > &Real_Names, map< string, string, less<string> > &Index)
{
  map< string, string, less<string> > Temp_Index;

  // Remove entries from the index that are for files that are gone.
  map< string, string, less<string> >::iterator Index_Scan;
  for (Index_Scan = Index.begin(); Index_Scan != Index.end(); Index_Scan++) {

    if (Real_Names.find((*Index_Scan).first) != Real_Names.end())
      Temp_Index[(*Index_Scan).first] = (*Index_Scan).second;
  }
  Index = Temp_Index;

  // Add new entries for files in the real directory that are not yet in the index.
  set< string, less<string> >::iterator Real_Scan;
  for (Real_Scan = Real_Names.begin(); Real_Scan != Real_Names.end(); Real_Scan++) {

    if (Index.find(*Real_Scan) == Index.end())
      Index[*Real_Scan] = "?";
  }
}


//
// Write_Index
//
// The following function writes the file 00_index.txt into the directory
//   specified by Path.
//
void Write_Index(const string &Path, const map< string, string, less<string> > &Index)
{
  string File_Name(Path);
  File_Name.append("/");
  File_Name.append("00_index.txt");

  ofstream Index_File(File_Name.c_str());

  // If the file could not be open, blow out of here. (Permission problem? Path
  //    not really a directory?)
  //
  if (!Index_File) return;

  // Visit every element in the map.
  map< string, string, less<string> >::const_iterator p;
  for (p = Index.begin(); p!= Index.end(); ++p) {

    // This hack is because gcc's string class doesn't honor the iostream manipulators.
    const string &Name = (*p).first;
    int   Length = Name.size();
    int   Extra  = (Length < 20) ? 20 - Length : 1;
    Index_File << Name << setw(Extra) << " " << (*p).second << endl;
  }

  // Set permissions on the file.
  chmod(File_Name.c_str(), 0664);
}


//
// Process_Subdirectories
//
// The following function scans the directory specified by Path and for
//   each subdirectory found calls Scan_Directory to handle 00_index.txt
//   in that subdirectory. It is used to recursively descend the directory
//   tree when the user requests that action.
//
void Process_Subdirectories(const string &Path)
{
  DIR    *Directory_Handle = opendir(Path.c_str());
  dirent *Directory_Entry;

  if (Directory_Handle == 0) return;
  while ((Directory_Entry = readdir(Directory_Handle)) != 0) {

    // Ignore files with names starting with '.'
    if (Directory_Entry->d_name[0] == '.') continue;

    // Otherwise compose the full file name.
    string Full_Name(Path);
    Full_Name.append("/");
    Full_Name.append(Directory_Entry->d_name);

    // If it's a directory call Scan_Directory (recursively).
    struct stat File_Info;
    if (stat(Full_Name.c_str(), &File_Info) != 0) continue;
    if (S_ISDIR(File_Info.st_mode)) {
      Scan_Directory(Full_Name);
    }
  }
  closedir(Directory_Handle);
}


//
// Scan_Directory
//
// This function does the main work of the program. It scans the
//   directory named in its parameter and sychronizes 00_index.txt.
//   It then processes subdirectories recursively if that has been
//   requested.
//
void Scan_Directory(const string &Path)
{
  set< string, less<string> >         Real_Names;  // Names from the directory listing.
  map< string, string, less<string> > Index;       // Digested form of 00_index.txt.

  // Read in the current 00_index.txt if there is one.
  Read_Index(Path, Index);

#ifdef NEVER
  // This is for testing/debugging purposes.
  cout << "Contents of 00_index.txt for " << Path << endl;

  map< string, string, less<string> >::iterator p;
  for (p = Index.begin(); p != Index.end(); ++p) {
    cout << "File: " << (*p).first << ", has description: " << (*p).second << endl;
  }
#endif

  // Scan directory for all ordinary files and subdirectories.
  Get_Files(Path, Real_Names);

#ifdef NEVER
  // This is for testing/debugging purposes.
  cout << "List of file/directories in " << Path << endl;

  set< string, less<string> >::iterator p;
  for (p = Real_Names.begin(); p != Real_Names.end(); ++p) {
    cout << "\t" << *p << endl;
  }
#endif

  // Synchronize 00_index.txt with the directory.
  Sync(Real_Names, Index);

  // Write out the new 00_index.txt
  Write_Index(Path, Index);

  // Scan directory for all subdirectories if requested.
  if (Recurse) Process_Subdirectories(Path);
}


//---------------------------------
//           Main Program
//---------------------------------

int main(int Argc, char **Argv)
{
  int    Ch;
  string Group_Name;      // The name of the group for 00_index.txt
  bool   Group_Specified; // =true if Group_Name is to be used.

  // Scan the command line looking for options.
  while ((Ch = getopt(Argc, Argv, "r")) != EOF) {
    switch (Ch) {

      // They want to recurse. Remember that for later.
      case 'r': Recurse = true; break;

      // Unknown option encountered. Message already printed by getopt().
      case '?': exit(1); break;
    } 
  }

  // Step down the command line getting directory names as we go.
  for (int i = optind; i < Argc; i++) {
    string Path(".");
    Path = Argv[i];

    // Process this directory. We should verify that it is really a
    //   directory before we do this.
    Scan_Directory(Path);
  }

  return 0;
}
