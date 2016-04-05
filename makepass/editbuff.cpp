/*****************************************************************************
FILE          : editbuff.cpp
LAST REVISION : July 1992
SUBJECT       : Implementation of class Edit_Buffer
PROGRAMMER    : Peter Chapin

This file contains the implementation of class Edit_Buffer. Objects from
this class allow the client to perform simple editing operations on strings
which may be arbitrarily long (up to the size of unsigned int minus the
overhead used by the library to manage a chunk of dynamic memory).

Edit_Buffers obey the following constraints:

If the Workspace member is not NULL, it points at a null terminated string.
This string is the literal text being edited. In other words, the string
embeded in an Edit_Buffer does not contain any embeded control codes. This
is so conversion to char pointer can be done easily.

The Workspace member is NULL if the object is in the "error state". Objects
in the error state ignore most actions. It is possible, however, to use
such an object as the source of an initialization or assignment. If such an
object is the destination of an assignment, it may be brought out of the
error state.

Objects enter the error state when a memory allocation operation fails.
Thus insert, append, or replace (for large offsets) operations may cause
the object to immediately enter the error state. The string stored in the
object beforehand cannot be recovered.

It is possible to manipulate the stored string at offsets that are beyond
the true end of the string. The object "behaves normally." In particular,
deleting a large offset does nothing while inserting or replacing at a
large offset will extend the string with spaces.

The performance of Edit_Buffer objects may not be very good. A memory
allocation operation is done during every Edit_Buffer operation which could
be very expensive in some applications. Additionally, heap memory may
become fragmented quickly.

Things could be improved by forcing the memory allocation operations to be
done is larger chunks (say 8 bytes at a time). This would reduce their
frequency by a large factor.

In addition, the usage of these objects would be complicated if the strings
contained application specific control codes or escape sequences. In such a
case, the offsets would have to be adjusted since the offset into the
logical string would not be the same as the offset into the physical
string. This might be the case, for example, in a hypertext editor. Class
Edit_Buffer does not support such things directly.

In 8086 large data models, the performance issues identified above are not
as much of a problem. Generally, heap allocations in large data models are
each in their own segment. Furthermore, segments can't be closer than 16
bytes. Thus memory allocation operations are done, in effect, in 16 byte
chunks already.

     Please send comments and bug reports to

         Peter Chapin
         P.O. Box 317
         Randolph Center, VT 05061

*****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "editbuff.h"

/*=================================*/
/*           Global Data           */
/*=================================*/

// None Required!!

/*==========================================================*/
/*           Private Members of class Edit_Buffer           */
/*==========================================================*/

// None Required!!

/*=========================================================*/
/*           Public Members of class Edit_Buffer           */
/*=========================================================*/

//-------------------------------------------------
//           Constructors and destructor
//-------------------------------------------------

/*----------------------------------------------------------------------------
Edit_Buffer::Edit_Buffer();
Edit_Buffer::Edit_Buffer(const char *String);
Edit_Buffer::Edit_Buffer(const Edit_Buffer &Existing);
Edit_Buffer &Edit_Buffer::operator=(const Edit_Buffer &Existing);
Edit_Buffer::~Edit_Buffer();

The construction and destruction functions are straightforward. Notice that
when an object in the error state is used to initialize another object, the
new object is put into the error state. Notice also that the destructor
depends on the fact that deleting a NULL pointer is safe.
----------------------------------------------------------------------------*/

Edit_Buffer::Edit_Buffer()
  {
    Workspace = (char *)malloc(1);
    if (Workspace != NULL) *Workspace = '\0';
    return;
  }


Edit_Buffer::Edit_Buffer(const char *String)
  {
    if (String == NULL) {
      Workspace = (char *)malloc(1);
      if (Workspace != NULL) *Workspace = '\0';
      return;
    }

    register size_t Length = strlen(String);

    Workspace = (char *)malloc(Length + 1);
    if (Workspace != NULL) memcpy(Workspace, String, Length + 1);
    return;
  }


Edit_Buffer::Edit_Buffer(const Edit_Buffer &Existing)
  {
    // If initializer is in the error state, put this object into the error state.
    if (Existing.Workspace == NULL) {
      Workspace = NULL;
    }

    // Load the string from the initializer.
    else {
      register size_t Length = strlen(Existing.Workspace);

      Workspace = (char *)malloc(Length + 1);
      if (Workspace != NULL) memcpy(Workspace, Existing.Workspace, Length + 1);
    }
    return;
  }


Edit_Buffer &Edit_Buffer::operator=(const Edit_Buffer &Existing)
  {
    // Erase current object.
    free(Workspace);

    // Assign the error state if that's the situation.
    if (Existing.Workspace == NULL) {
      Workspace = NULL;
    }

    // Load the string from the existing object.
    else {
      register size_t Length = strlen(Existing.Workspace);

      Workspace = (char *)malloc(Length + 1);
      if (Workspace != NULL) memcpy(Workspace, Existing.Workspace, Length + 1);
    }
    return *this;
  }


Edit_Buffer::~Edit_Buffer()
  {
    free(Workspace);
    return;
  }

//----------------------------
//           Access
//----------------------------

/*----------------------------------------------------------------------------
Edit_Buffer::operator char *();
char &Edit_Buffer::operator[](unsigned Offset);
bool Edit_Buffer::operator!() const;
unsigned Edit_Buffer::Length() const;

The following functions provide access to various attributes of the object.
Notice that they are declared const to allow usage with constant objects
(or pointers to constant objects, etc). A nonconst version of operator char
*() and operator []() are provided to simplify certain practical problems.

The first allows Edit_Buffer objects to be used where plain char pointers
are required. The length of the internal string should not be changed by
the user code. Note that this function will return a NULL pointer if the
object is in the error state.

The second allows Edit_Buffer objects to be indexed as if they were arrays
of characters. User code should take care not to index off the end of the
buffer. Note that this function will return a reference to a dummy
character if the object is in the error state.

The values returned by operator char*() and operator[]() are INVALIDATED
BY CALLING ANY MEMBER THAT CHANGES THE SIZE OF AN EDIT_BUFFER OBJECT!

Operator! returns TRUE if the object is in the error state.

Length return the length of the internal string (as with strlen(), only
maybe faster). If the object is in the error state, Length() returns 0.
Note that a Length() of zero does not imply the error state!
----------------------------------------------------------------------------*/


Edit_Buffer::operator char *()
  {
    return Workspace;
  }


Edit_Buffer::operator const char *() const
  {
    return Workspace;
  }


char &Edit_Buffer::operator[] (unsigned Offset)
  {
    static char Dummy = ' ';

    if (Workspace == NULL) return Dummy;
    return Workspace[Offset];
  }


const char &Edit_Buffer::operator[] (unsigned Offset) const
  {
    static char Dummy = ' ';

    if (Workspace == NULL) return Dummy;
    return Workspace[Offset];
  }


bool Edit_Buffer::operator!() const
  {
    return (bool) (Workspace == NULL);
  }


unsigned Edit_Buffer::Length() const
  {
    return (Workspace == NULL ? 0 : strlen(Workspace));
  }

//-----------------------------------
//           Manipulation
//-----------------------------------

/*----------------------------------------------------------------------------
bool Edit_Buffer::Insert(char Letter, unsigned Offset);
char Edit_Buffer::Delete(unsigned Offset);
bool Edit_Buffer::Erase();
bool Edit_Buffer::Replace(char &Letter, unsigned Offset);
bool Edit_Buffer::Append(char Letter);
bool Edit_Buffer::Append(const char *Additional);
bool Edit_Buffer::Append(const Edit_Buffer &Existing);
void Edit_Buffer::Trim(unsigned Offset);

The maniuplation functions are all fairly straightforward. Reallocations
are done as required to extend the string, make room for inserted
characters, or release memory due to deleted characters. Most functions
return false if the object is left in the error state and true
otherwise. Replace() returns the old character at the offset in its Letter
parameter. Delete() returns the null character if the object is in the
error state, otherwise it returns the character deleted. If the offset is
very large, Delete will return a space.
----------------------------------------------------------------------------*/

bool Edit_Buffer::Insert(char Letter, unsigned Offset)
  {
    if (Workspace == NULL) return false;

    unsigned Current_Size = (unsigned)strlen(Workspace);

    // If the offset is off the end, extend string so that null byte is at offset.
    if (Offset > Current_Size) {
      Workspace = (char *)realloc(Workspace, Offset + 1);
      if (Workspace != NULL) {
        memset(&Workspace[Current_Size], ' ', Offset - Current_Size);
        Workspace[Offset] = '\0';
        Current_Size = Offset;
      }
    }

    // Return at once if the above operation didn't work.
    if (Workspace == NULL) return false;

    // Now insert the character.
    Workspace = (char *)realloc(Workspace, Current_Size + 2);
    if (Workspace != NULL) {
      memmove(&Workspace[Offset+1], &Workspace[Offset], Current_Size - Offset + 1);
      Workspace[Offset] = Letter;
    }

    return Workspace != NULL;
  }


char Edit_Buffer::Delete(unsigned Offset)
  {
    char Return_Value;
    if (Workspace == NULL) return '\0';

    unsigned Current_Size = (unsigned)strlen(Workspace);

    if (Offset >= Current_Size) Return_Value = ' ';
    else {
      Return_Value = Workspace[Offset];
      memmove(&Workspace[Offset], &Workspace[Offset+1], Current_Size - Offset);
      Workspace = (char *)realloc(Workspace, Current_Size);
    }

    return Return_Value;
  }

bool Edit_Buffer::Erase()
  {
    // Remove existing string. If in error state, attempt to rebuild.
    free(Workspace);

    // Construct a null buffer.
    Workspace = (char *)malloc(1);
    if (Workspace != NULL) *Workspace = '\0';

    // Return TRUE if object ok.
    return Workspace != NULL;
  }

bool Edit_Buffer::Replace(char &Letter, unsigned Offset)
  {
    if (Workspace == NULL) return false;

    unsigned Current_Size = (unsigned)strlen(Workspace);

    // If the offset is off the end, extend string so that null byte is
    // just past offset.
    // 
    if (Offset >= Current_Size) {
      Workspace = (char *)realloc(Workspace, Offset + 2);
      if (Workspace != NULL) {
        memset(&Workspace[Current_Size], ' ', Offset - Current_Size + 1);
        Workspace[Offset+1] = '\0';
        Current_Size = Offset+1;
      }
    }

    // Return at once if the above operation didn't work.
    if (Workspace == NULL) return false;

    // Now replace the character.
    char Old_Letter = Workspace[Offset];
    Workspace[Offset] = Letter;
    Letter = Old_Letter;

    return Workspace != NULL;
  }


bool Edit_Buffer::Append(char Letter)
  {
    if (Workspace == NULL) return false;

    register unsigned Current_Size = (unsigned)strlen(Workspace);
    if ((Workspace = (char *)realloc(Workspace, Current_Size + 2)) != NULL) {
      Workspace[Current_Size++] = Letter;
      Workspace[Current_Size  ] = '\0';
    }

    return Workspace != NULL;
  }


bool Edit_Buffer::Append(const char *Additional)
  {
    if (Workspace == NULL) return false;
    if (Additional == NULL) return true;

    unsigned Current_Size    = (unsigned)strlen(Workspace);
    unsigned Additional_Size = (unsigned)strlen(Additional);

    if ((Workspace = (char *)realloc(Workspace, Current_Size + Additional_Size + 1)) != NULL) {
      strcat(Workspace, Additional);
      Workspace[Current_Size + Additional_Size] = '\0';
    }

    return Workspace != NULL;
  }


bool Edit_Buffer::Append(const Edit_Buffer &Existing)
  {
    if (Workspace == NULL) return false;
    if (Existing.Workspace == NULL) return true;

    unsigned Current_Size    = (unsigned)strlen(Workspace);
    unsigned Additional_Size = (unsigned)strlen(Existing.Workspace);

    if ((Workspace = (char *)realloc(Workspace, Current_Size + Additional_Size + 1)) != NULL) {
      strcat(Workspace, Existing.Workspace);
      Workspace[Current_Size + Additional_Size] = '\0';
    }

    return Workspace != NULL;
  }


void Edit_Buffer::Trim(unsigned Offset)
  {
    if (Workspace == NULL) return;

    unsigned Current_Size = (unsigned) strlen(Workspace);

    if (Offset >= Current_Size) return;
    Workspace = (char *)realloc(Workspace, Offset + 1);
    if (Workspace != NULL) Workspace[Offset] = '\0';
  }

void Edit_Buffer::Reverse_Trim(unsigned Offset)
  {
    char *Temp_Workspace;

    if (Workspace == NULL) return;

    unsigned Current_Size = (unsigned) strlen(Workspace);

    // If we're trying to erase the whole thing, just do it!
    if (Offset >= Current_Size) {
      Erase();
      return;
    }

    Temp_Workspace = (char *)malloc(Current_Size - Offset + 1);

    if (Temp_Workspace != NULL) {
      strcpy(Temp_Workspace, &Workspace[Offset]);
      free(Workspace);
      Workspace = Temp_Workspace;
    }

    return;
  }

//------------------------------------
//           Administration
//------------------------------------

/*----------------------------------------------------------------------------
void Edit_Buffer::Clear_Error();

The following function attempts to bring an object in the error state out
of that state. Note that this function may fail; the object may remain in
the error state afterwards.
----------------------------------------------------------------------------*/

void Edit_Buffer::Clear_Error()
  {
    if (Workspace != NULL) return;

    Workspace = (char *)malloc(1);
    if (Workspace != NULL) *Workspace = '\0';
    return;
  }
