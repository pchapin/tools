/*****************************************************************************
FILE          : editbuff.hpp
LAST REVISION : July 1992
SUBJECT       : Public interface to class Edit_Buffer
PROGRAMMER    : Peter Chapin

Objects from this class are "arbitrarly" long strings which provide basic
editing operations.

Please send comments and bug reports to

         Peter Chapin
         P.O. Box 317
         Randolph Center, VT 05061

*****************************************************************************/

#ifndef EDITBUFF_HPP
#define EDITBUFF_HPP

class Edit_Buffer {

  private:
    char *Workspace;      // Points at null terminated string in buffer.

  public:
    // Constructors and destructor.
    Edit_Buffer();                  // Empty buffer.
    Edit_Buffer(const char *);      // Loads buffer with given string.
    Edit_Buffer(const Edit_Buffer &);             // Copies string.
    Edit_Buffer &operator=(const Edit_Buffer &);  // Copies string.
   ~Edit_Buffer();

    // Access.
    operator char *();              // Returns pointer imbeded string.
    operator const char *() const;  // Returns pointer to imbeded string.
    char &operator[](unsigned Offset);  // Returns reference to imbeded string.
    const char &operator[](unsigned Offset) const; // ditto...
    bool operator !() const;        // Returns YES if object in error state.
    unsigned Length() const;        // Returns length of imbeded string.

    // Manipulation.
    bool Insert(char Letter, unsigned Offset);
    char Delete(unsigned Offset);
    bool Erase();
    bool Replace(char &Letter, unsigned Offset);
    bool Append(char);
    bool Append(const char *);
    bool Append(const Edit_Buffer &);

    void Trim(unsigned Offset);
      // Delete to end of line. The character at Offset is the first to
      //   be deleted. If Offset is past the end of the buffer, there is
      //   no effect.

    void Reverse_Trim(unsigned Offset);
      // Delete to start of line. The character at Offset is preserved. If
      //   offset is past the end of the buffer, the entire line is erased.
      //   This function uses a temporary buffer to hold the tail of the
      //   line. If there's isn't enough memory to do that, this function
      //   has no overall effect.

    // Administration.
    void Clear_Error();      // Reinitializes an object in the error state.
  };

#endif

