/*=========================================================================*/
/*                      Tab Size Readjustment Program                      */
/*                                                                         */
/*                Copyright (c) 1987 William C. Colley, III                */
/*                                                                         */
/*                                                                         */
/*      This  program  combines  the  functions  of an ENTAB program and a */
/* DETAB program in the same package.  Combining  the two  allows a simple */
/* means of  retabifying a  program.   For example, let's say you edit a C */
/* program with your editor set for a tab increment of  4.   Now, you TYPE */
/* this file  on the console and you find that it laps over the right edge */
/* of the screen because your terminal expands tabs  with a  tab increment */
/* of 8.   What  you really would like to do is delete some of the tabs so */
/* that your program will TYPE just like it looked inside the editor.  The */
/* following command will do just that:                                    */
/*                                                                         */
/*      RETAB -O4 -N8 oldfile newfile                                      */
/*                                                                         */
/*      In the  above command  line, the  two file names must be in order, */
/* but the options can be spread around anywhere.  In fact, the  - options */
/* are optional. Their default values are set by a pair of #defines below. */
/* The -O option specifies the tab increment that the old file was created */
/* with.  The  -N  option  specifies  the desired tab increment of the new */
/* file.                                                                   */
/*                                                                         */
/*      Note that or -N1  causes no  entabification.   Thus, the following */
/* command will detabify a file:                                           */
/*                                                                         */
/*      RETAB -O8 -N1 oldfile newfile                                      */
/*                                                                         */
/*      Also note  that if the old file contains no tabs, there will be no */
/* detabification.  Thus, the following command will entabify a detabified */
/* file:                                                                   */
/*                                                                         */
/*      RETAB -N8 oldfile newfile                                          */
/*                                                                         */
/*      Blanks  and  tabs  in  quoted  strings  are  left  alone.    The C */
/* conventions  regarding  character  constants  like  '"'  and  character */
/* strings like "Here\'s a \"quote\"." are supported.                      */
/*                                                                         */
/*                                                                         */
/*=========================================================================*/

#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

//  User-adjustable default values:
#define OLDTABS     8       //  Default old tab increment.
#define NEWTABS     8       //  Default new tab increment.


// Values for state.
#define NORMAL      0       //  Normal state.
#define QUOTE       1       //  Quoted string skip state.
#define BLANK       2       //  Blank parsing state.

const char *usage[]={
  "     This program  allows you  to insert  or delete  tabs in  a text file so",
  "that the file looks the same when the  tab stops  are changed.  This program",
  "can be  used to  remove all tabs from a file or to replace as many spaces as",
  "possible with tabs.",
  "",
  "RETAB -o[old tab stop] -n[new tab stop] oldfile newfile",
  "",
  "     For example:",
  "",
  "RETAB -o8 -n1 old new",
  "     Replaces all tabs with spaces. Spaces are added assuming the  tab stops",
  "     are every eight columns.",
  "",
  "RETAB -o8 -n8 old new",
  "     Inserts as  many tabs as it can assuming tab stops are every eight. Any",
  "     tabs that already exist in the file are handled correctly.",
  "",
  "RETAB -o4 -n8 old new",
  "     Adjusts tabs so that a file  created with  tab stops  every 4  looks ok",
  "     when printed on a printer that uses tab stops every 8.",
  "",
  "     The default  tab stop  size is  8 for  both the  old and the new files.",
  "Thus \"RETAB old new\" inserts tabs, and \"RETAB -n1 old new\" removes them.",
  NULL
};


char *adjust_date( const char *ANSI_date )
{
    static char  buffer[13];
           char *buffer_pointer;

    // Make a working copy of the date as from the ANSI __DATE__ macro.
    strcpy( buffer, ANSI_date );

    // Open up space for the comma.
    for( buffer_pointer  = strchr( buffer, '\0' );
         buffer_pointer >= &buffer[6];
         buffer_pointer-- ) {
        *( buffer_pointer + 1 ) = *buffer_pointer;
    }

    // Put the comma in.
    buffer[6] = ',';

    // If this is a date from 1 to 9, close up the extra space.
    if( buffer[4] == '0' || buffer[4] == ' ' ) {
        for( buffer_pointer = &buffer[4]; *buffer_pointer; buffer_pointer++ ) {
            *buffer_pointer = *( buffer_pointer + 1 );
        }
    }

    // Return are result.
    return buffer;
}


void print_usage( )
{
    const char **line;
    
    for( line = usage; *line; line++ )
        printf( "%s\n", *line );
}


void entab( int ch, int blanks, int new_tabs, int column, FILE *new_file )
{
    int j;

    if( blanks > 1 && new_tabs > 1 )
        while( ( j = new_tabs - column % new_tabs ) <= blanks ) {
            column += j;
            blanks -= j;
            putc( '\t', new_file );
        }
    for( ; blanks; --blanks ) {
        ++column;
        putc( ' ', new_file );
    }
    if( ch == '\n' ) column = 0;
    else
        ++column;
    putc( ch, new_file );
}


int main(int argc, char **argv)
{
           int   delimiter;
           int   i;
           FILE *old_file;
           FILE *new_file;
    static char *old_file_name = NULL;
    static char *new_file_name = NULL;
    static int   error         = 0;
    static int   old_tabs      = OLDTABS;
    static int   new_tabs      = NEWTABS;
    static int   blanks        = 0;
    static int   column        = 0;
    static int   literal       = 0;
    static int   state         = NORMAL;
    
    printf( "RETAB  (Version 0.1)  Compiled: %s\n", adjust_date( __DATE__ ) );

    while( --argc ) {
        if( **++argv == '-' ) {
            switch( i = *( *argv + 1 ) ) {
        	
            case 'n':
            case 'N':
                if( ( i = atoi( *argv + 2 ) ) < 1 ) {
                    printf( "ERROR:  New tab increment < 1\n\n" );
                    error = !0;
                }
                else new_tabs = i;
                break;

            case 'o':
            case 'O':
                if( ( i = atoi( *argv + 2 ) ) < 1 ) {
                    printf( "ERROR:  Old tab increment < 1\n\n" );
                    error = !0;
                }
                else old_tabs = i;
                break;

            default:
                printf( "ERROR:  Illegal option %s\n\n", *argv );
                error = !0;
                break;
            }
        }
        else if( !old_file_name ) old_file_name = *argv;
        else if( !new_file_name ) new_file_name = *argv;
        else {
            printf( "ERROR:  Extra filename %s\n\n", *argv );
            error = !0;
        }
    }
    if( !old_file_name || !new_file_name ) {
        print_usage( );
        error = !0;
    }
    else if( ( old_file = fopen( old_file_name, "r" ) ) == NULL ) {
        printf( "ERROR:  Cannot find file %s\n\n", old_file_name );
        error = !0;
    }
    else {
        if( ( new_file = fopen( new_file_name, "w" ) ) == NULL ) {
            printf( "ERROR:  Cannot create file %s\n\n", new_file_name );
            error = !0;
        }
        else {
            setvbuf( old_file, NULL, _IOFBF, 8*1024 );
            setvbuf( new_file, NULL, _IOFBF, 8*1024 );
            while( ( i = getc( old_file ) ) != EOF ) {
                i &= 0177;
                switch( state ) {
          	
                case NORMAL:
                    switch( i ) {
                    case '\n':
                        column = 0;
                        putc( '\n', new_file );
                        break;

                    case '\t':
                        blanks = old_tabs -  column % old_tabs;
                        state = BLANK;
                        break;

                    case ' ':
                        blanks = 1;
                        state = BLANK;
                        break;

                    case '\'':
                    case '"':
                        delimiter = i;
                        ++column;
                        putc( i, new_file );
                        break;

                    default:
                        ++column;
                        putc( i, new_file );
                        break;
                    }
                    break;

                case QUOTE:
                    switch( i ) {
                    case '\n':
                        if( !literal ) state = NORMAL;
                        column = 0;
                        break;

                    case '\t':
                        column += new_tabs - column % new_tabs;
                        break;

                    case '\'':
                    case '"' :
                        if( !literal && i == delimiter ) state = NORMAL;
                        ++column;
                        break;

                    default:
                        ++column;
                        break;
                    }
                    putc( i, new_file );
                    break;

                case BLANK:
                    switch( i ) {
                    case '\t':
                        blanks += old_tabs - ( column + blanks ) % old_tabs;
                        break;

                    case ' ':
                        ++blanks;
                        break;

                    case '"':
                    case '\'':
                        delimiter = i;
                        state = QUOTE;
                        entab( i, blanks, new_tabs, column, new_file );
                        break;

                    default:
                        state = NORMAL;
                        entab( i, blanks, new_tabs, column, new_file );
                        break;
                    }
                    break;
                }
                literal = !literal && i == '\\';
            }
            if( ferror( new_file) || fclose( new_file ) ) {
                printf( "ERROR:  Disk full\n\n" );
                error = !0;
            }
        }
        fclose( old_file );
    }
    return error;
}
