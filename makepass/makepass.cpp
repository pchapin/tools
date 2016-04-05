/****************************************************************************
FILE          : MAKEPASS.CPP
LAST REVISION : May 29, 2000
SUBJECT       : Silly password generating program.
PROGRAMMER    : (C) Copyright 2000 by Peter Chapin

Please send comments or bug reports to

     Peter Chapin
     P.O. Box 317
     Randolph Center, VT 05061
     Peter.Chapin@vtc.vsc.edu
****************************************************************************/

#include <cctype>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include "editbuff.h"

using namespace std;

// Maximum number of characters in a word unit.
const int UNIT_SIZE = 4;

// Abstract type to hold a word unit. Constructor insures word is blank.
struct Word_Unit {
  char The_Unit[UNIT_SIZE + 1];

  Word_Unit() { The_Unit[0] = '\0'; }
};


//=================================
//           Global Data
//=================================

// We need a linked list of word units. There will be a lot of them!
vector<Word_Unit> Unit_List;

// Name of the file containing the word units. Since I wrote this
// program, I will use a default that is good for me. This name can be
// specified on the command line, however.
//
char *Unit_File = "PUNITS.TXT";


//==========================================
//           Function Definitions
//==========================================

//
// The following function converts a string to lowercase.
//
static void my_strlwr(char *Buffer)
{
  while (*Buffer) {
    *Buffer = tolower(*Buffer);
     Buffer++;
  }
}


//
// The following function reads the file containing word units. Return
// the number of units read.
//
long Read_Units(char *File_Name)
{
  FILE *Unit_File = fopen(File_Name, "r");
  if (Unit_File == NULL) return 0;

  char Line_Buffer[128+2];

  // Read every line of the file.
  while (fgets(Line_Buffer, 128+2, Unit_File) != NULL) {

    // Skip comment lines or screwed up lines (or blank lines).
    if (Line_Buffer[0] == '#') continue;
    if (!isalpha(Line_Buffer[0])) continue;

    // Blow away the '\n' at the end if there is one.
    char *End_Pntr = strchr(Line_Buffer, '\n');
    if (End_Pntr != NULL) *End_Pntr = '\0';

    Word_Unit New_Unit;

    // Copy the word unit into a Word_Unit object. This should be done
    // with an appropriate constructor. I got lazy. So sue me.
    // 
    strncpy(New_Unit.The_Unit, Line_Buffer, UNIT_SIZE);
    New_Unit.The_Unit[UNIT_SIZE] = '\0';

    // Suggest passwords in lower case. Uppercase is too much.
    my_strlwr(New_Unit.The_Unit);

    // Add this unit to the list.
    Unit_List.push_back(New_Unit);
  }

  fclose(Unit_File);
  return Unit_List.size();
}


//
// The following function does the grunt work of suggesting a password.
// The parameter is the number of units in the unit list.
// 
void Generate_Password(int Unit_Count)
{
  Edit_Buffer Password;     // Holds our suggestions.

  do {
    vector<int> Unit_Breaks;   // Remember the breaks between units.
    Unit_Breaks.push_back(0);  // The beginning is a break.

    Password.Erase();       // Forget our old suggestion.

    // First decide: should there be a prefix?
    int Prefix = rand() % 10 + 1;
    if (Prefix <= 2) {
      static char *Prefix_Strings[] = {
        "pre", "un", "in", "non", "sub"
      };
      const int Prefix_Count = sizeof(Prefix_Strings)/sizeof(char *);

      int Prefix_Index = rand() % Prefix_Count;       // Select one.
      Password.Append(Prefix_Strings[Prefix_Index]);  // Add it on.
      Unit_Breaks.push_back(Password.Length()); // Remember the break point.
    }

    // First decide: two or three word units? (Right now... stick with two)
    int Number_Units = 2;

    // Come up with the basic units.
    for (int Count = 0; Count < Number_Units; Count++) {
      int Index = rand() % Unit_Count;             // Select one.
      Password.Append(Unit_List[Index].The_Unit);  // Add it on.
      Unit_Breaks.push_back(Password.Length());    // Remember the break point.
    }

    // Next decide: should there be a suffix?
    int Suffix = rand() % 2;
    if (Suffix) {
      static char *Suffix_Strings[] = {
        "o", "s", "a", "ing", "ist", "est", "ly", "ed", "less"
      };
      const int Suffix_Count = sizeof(Suffix_Strings)/sizeof(char *);

      int Suffix_Index = rand() % Suffix_Count;       // Select one.
      Password.Append(Suffix_Strings[Suffix_Index]);  // Add it on.
      Unit_Breaks.push_back(Password.Length()); // Remember the break point.
    }

    // Next decide: should there be a special?
    int Special = rand() % 2;
    if (Special) {
      static char Special_Characters[] = ",./<>?;':\"\\|=+-_*&^%$#@!";
      const int Special_Count = (sizeof(Special_Characters)-1)/sizeof(char);

      // Select a special character and a place to put it. Special
      // characters only appear between units. It's easier to remember
      // that way.
      // 
      int Special_Index  = rand() % Special_Count;
      int Special_Break  = rand() % Unit_Breaks.size();
      int Special_Offset = Unit_Breaks[Special_Break];
      Password.Insert(Special_Characters[Special_Index], Special_Offset);
    }

  } while (Password.Length() < 6);

  printf("How about: %-20s ", (char *)Password);
}


//
// Very simple main program.
//
int main(int Argc, char **Argv)
{
  int Done = 0;

  if (Argc != 1) Unit_File = Argv[1];

  printf("Reading word unit file. Please wait...");
  long Unit_Count = Read_Units(Unit_File);
  if (Unit_Count == 0) {
    printf("\n\nUnable to open word unit file. Exiting.\n\n");
    return 1;
  }
  printf("\n\n");

  // Does this require LONG_MAX > INT_MAX ??
  if (Unit_Count > INT_MAX) {
    printf("WARNING: Too many word units, truncating list\n");
    Unit_Count = INT_MAX;
  }

  // Set the random number generator seed.
  time_t Current = time(NULL);
  srand((int)Current);

  do {
    int Response;

    Generate_Password((int)Unit_Count);
    printf("Another? [y]/n ");
    Response = getchar();
    while (getchar() != '\n') ;
    if (Response == 'N' || Response == 'n') Done = 1;
    printf("\n");
  } while (!Done);

  return 0;
}
