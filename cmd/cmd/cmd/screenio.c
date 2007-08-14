// SCREENIO.C - Screen input routine for the 4xxx / TCMD family
//   (c) 1991 - 1998 Rex C. Conn  All rights reserved

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include <string.h>
#include <io.h>

#include "4all.h"


static int TabComplete( char *, char *, int );
static int fname_length( char * );
static int PositionEGets( char * );
static void clearline( char * );
void efputs( char * );
static void PadBlanks( int );

static char *base;                      // start of line
static char *curpos;                    // current position

static int nScreenRows;                 // current screen size
static int nScreenColumns;
static int nHomeRow;                    // starting row & column
static int nHomeColumn;
static int nCursorRow;                  // current row & column position
static int nCursorColumn;
static int nEditFlag;
static int fPassword;
static int nColor;
int fInsert;

// invalid filename characters
static char INVALID_CHARS[] = "  ()|<> \t;,`+=";


// get string from STDIN, with full line editing
//      curpos = input string
//      nMaxLength = max entry length
//      nEditFlag =  EDIT_COMMAND = command line
//                  EDIT_DATA = other input
//                  EDIT_ECHO = echo line & get DATA
//                  EDIT_DIALOG = edit line within current dialog window
int egets( char *pStr, int nMaxLength, int fFlags)
{
        int c;
        char *ptr;
        int i, n, fEcho = 1, fLFN = 1, fExecuteNow = 0;
        char *arg, curstate = 0;
        char source[MAXFILENAME];
        PCH fptr, kptr;

        // don't take any input in a detached process
        if ( gpLIS->typeProcess == PT_DETACHED )
                return 0;

        kptr = (char *)NULLSTR;

        // get the insert mode
        fInsert = (char)gpIniptr->EditMode;

        base = curpos = pStr;

        nEditFlag = fFlags;
        fPassword = (( nEditFlag & EDIT_PASSWORD ) != 0 );

        // if not echoing existing input, clear the input line
        if (( nEditFlag & EDIT_ECHO ) == 0 )
                *curpos = '\0';

        if (( nEditFlag & EDIT_NO_INPUTCOLOR ) || (( nColor = (( nEditFlag & EDIT_DIALOG ) ? gpIniptr->ListColor : gpIniptr->InputColor )) == 0 ))
                nColor = -1;

        // set the default cursor size
        SetCurSize( 0 );

        // get the number of screen lines and columns
        nScreenRows = GetScrRows();
        nScreenColumns = GetScrCols();

        // get the cursor position
        GetCurPos( &nHomeRow, &nHomeColumn );

        nCursorRow = nHomeRow;
        nCursorColumn = nHomeColumn;

        efputs( base );

        // check for line wrap & adjust home position accordingly
        PositionEGets( strend( base ));

        // get & set starting cursor position
        if ( fEcho )
                PositionEGets( curpos );

        for ( ; ; ) {

egets_key:
                // get keystroke from keyboard or alias keylist
                if ( *kptr == '\0' ) {

                        if ( fExecuteNow )
                                c = CR;
                        else {
                                fEcho = 1;
                                if (( c = GetKeystroke( EDIT_NO_ECHO | EDIT_SWAP_SCROLL | nEditFlag )) == EOF )
                                        goto egots_cr_key;
                                c = cvtkey( c, MAP_GEN | MAP_EDIT );
                        }

                } else {

                        c = cvtkey( *kptr++, MAP_GEN | MAP_EDIT );

                        // set escaped character
                        if (( c == (int)gpIniptr->EscChr ) && (( gpIniptr->Expansion & EXPAND_NO_ESCAPES ) == 0 ))
                                c = escape_char( *kptr++ );
                }

                // get & set cursor position
                if ( fEcho )
                        PositionEGets( curpos );

egots_key:

                switch ( c ) {
                case LF:
                        glpHptr = 0L;

                case CR:
egots_cr_key:
                        // reset cursor shape on exit
                        SetCurSize( 0 );

                        // go to EOL before CR/LF (may be multiple lines)
                        if ( fEcho )
                                PositionEGets( strend( curpos ));
                        if (( nEditFlag & EDIT_NO_CRLF ) == 0 )
                                crlf();

                        if ( nEditFlag & EDIT_COMMAND ) {

                                // adjust history pointer if command line entry
                                //  (glpHptr reset to NULL for a new command,
                                //  or if HistoryCopy=YES, or for an @@ alias)
                                // otherwise, set it to just past the current
                                if (( gpIniptr->HistoryCopy ) || (( fExecuteNow == 1 ) && ( fEcho == 0 )))
                                    glpHptr = 0L;

                                else if ( glpHptr != 0L ) {

                                    if ( strcmp( (PCH)base, glpHptr ) != 0 )
                                        glpHptr = 0L;

                                    else {

                                        // move history entry to end
                                        if ( gpIniptr->HistoryMove ) {
                                            // kill current entry & force it
                                            //   to be saved again at the end
                                            fptr = next_env( glpHptr );
                                            kptr = end_of_env( glpHptr );
                                            memmove( glpHptr, fptr, (unsigned int)( kptr - fptr ) + 1 );
                                            glpHptr = 0L;
                                        } else
                                            glpHptr++;
                                    }
                                }
                        }
                        return ( strlen( base ));

                case F1:        // help me!
                        _help( first_arg( base ), NULL );
                        break;

                case 6:         // ^F to expand aliases
                        if (( nEditFlag & EDIT_COMMAND ) == 0 ) {
                                honk();
                                break;
                        }

                        // perform alias expansion
                        clearline( base );
                        if ( alias_expand( base ) != 0 )
                                honk();

                        efputs( base );
                        curpos = strend( base );
                        break;

                case 11:        // ^K save line to history & clear input
                        if ( strlen( base ) > 0 ) {
                                glpHptr = 0L;
                                addhist( base );
                        }

                case ESC:       // cancel line
                        if (( nEditFlag & EDIT_DIALOG ) && ( *base == '\0' ))
                                goto egots_cr_key;

                        curpos = base;
                        clearline( curpos );
                        *curpos = '\0';
                        break;

                case CUR_LEFT:
                        if ( curpos > base )
                                curpos--;
                        break;

                case CUR_RIGHT:
                        if (*curpos)
                                curpos++;
                        break;

                case 12:
                case CTL_LEFT:
                        // delete word left (^L ) or move word left (^left)
                        for (ptr = curpos; ( curpos > base ); ) {
                                if ((!iswhite(*(--curpos))) && ((iswhite( curpos[-1])) || (!ispunct(*curpos) && ispunct( curpos[-1]))))
                                        break;
                        }

                        if ( c == 12) {         // delete word left (^L )
                                c = PositionEGets( strend( curpos ));
                                strcpy( curpos, ptr );
                                c -= PositionEGets( strend( curpos ));
                                PositionEGets( curpos );
                                efputs( curpos );
                                PadBlanks( c );
                        }

                        break;

                case 18:
                case CTL_RIGHT:
                        // delete word right (^R or ^BS) or move word right
                        for (ptr = curpos; (*curpos != '\0' ); ) {
                                if ((!iswhite(*(++curpos))) && ((iswhite( curpos[-1])) || (!ispunct(*curpos) && ispunct( curpos[-1]))))
                                        break;
                        }

                        if ( c == 18 ) {           // delete word right (^R)
                                c = PositionEGets( strend( curpos ));
                                curpos = strcpy( ptr, curpos );
                                c -= PositionEGets( strend( curpos ));
                                PositionEGets( curpos );
                                efputs( curpos );
                                PadBlanks( c );
                        }

                        break;

                case INS:
                        fInsert ^= 1;
                        curstate ^= 1;
                        SetCurSize( curstate );
                        break;

                case HOME:
                        curpos = base;
                        break;

                case END:
                        // goto end of line
                        curpos += strlen( curpos );
                        break;

                case CTL_HOME:
                        // delete from start of line to cursor
                        clearline( base );
                        curpos = strcpy( base, curpos );
                        efputs( curpos );
                        break;

                case CTL_END:
                        // erase to end of line
                        clearline( curpos );
                        *curpos = '\0';
                        break;

                case 4:         // delete history entry
                case 5:         // goto end of history
                case F3:        // recall last command (like COMMAND.COM)
                case CUR_UP:    // get previous command from history
                case CUR_DOWN:  // get next command from history

                        // disable if no history or not getting command line
                        if ((( nEditFlag & EDIT_COMMAND ) == 0 ) || ( *glpHistoryList == '\0' )) {
                                honk();
                                break;
                        }

                        // F3 behaves like COMMAND.COM
                        //   (get remainder of previous line)
                        if ( c == F3) {

                                fptr = prev_hist( NULL );
                                i = (int)( curpos - base );

                                // abort if new line longer than old one
                                if ((int)strlen( fptr ) <= i )
                                        break;
                                fptr += i;
                                goto redraw;
                        }

                        // ^E go to history end
                        if ( c == 5 ) {
                                glpHptr = 0L;
                                curpos = base;
                                clearline( curpos );
                                *curpos = '\0';
                        }

                        // if there are any characters on the current line,
                        //   try to match them (from the END of the history
                        //   list; otherwise, just get the previous or next
                        //   entry in the history list

                        arg = skipspace( base );
                        n = 0;

                        if ((ptr = strend( arg )) > arg ) {
                                n = (int)( ptr - arg );
                                glpHptr = 0L;
                        } else if ( c == 4 ) {
                                // don't delete unknown history entries!
                                honk();
                                break;
                        }

                        for ( ; ; ) {

                                PCH SaveHptr, LastHptr;

                                // ^D delete line from history,
                                //   then return previous entry
                                if (( c == 4 ) && ( glpHptr != 0L )) {
                                        fptr = next_env( glpHptr );
                                        memmove( glpHptr, fptr, gpIniptr->HistorySize - (fptr - glpHistoryList) );
                                }

                                if ( c == CUR_DOWN ) {

                                        SaveHptr = LastHptr = next_hist( glpHptr );
                                        for ( fptr = SaveHptr; ; LastHptr = fptr ) {
                                                i = 1;
                                                if (( fptr == 0L ) || (( i = strnicmp( fptr, (PCH)arg, n )) == 0 ) || (( fptr = next_hist( fptr )) == SaveHptr ) || ( fptr == LastHptr ))
                                                        break;
                                        }

                                } else {
                                        SaveHptr = LastHptr = prev_hist( glpHptr );
                                        for ( fptr = SaveHptr; ; LastHptr = fptr ) {
                                                i = 1;
                                                if (( fptr == 0L ) || (( i = strnicmp( fptr, (PCH)arg, n )) == 0 ) || (( fptr = prev_hist( fptr )) == SaveHptr ) || ( fptr == LastHptr ))
                                                        break;
                                        }
                                }

                                // failed a match?
                                if ( i != 0 ) {

                                        honk();

                                        // if no more matches for ^D, clear
                                        //   line & abort
                                        if ( c == 4) {
                                                curpos = base;
                                                clearline( curpos );
                                                *curpos = '\0';
                                        }

                                        break;
                                }

                                // redraw current line
                                arg = curpos = base;
redraw:
                                glpHptr = fptr;

                                clearline( curpos );
                                if ( glpHptr != 0L ) {
                                        strcpy( curpos, glpHptr );
                                        efputs( curpos );
                                }

                                // put cursor at EOL
                                curpos += strlen( curpos );

                                if ( c == F3)
                                        break;

                                // check for wrap at end of screen
                                PositionEGets( curpos );

                                // if next key is cur up, cur down, or ^D, stay
                                //   in the loop
                                c = cvtkey( GetKeystroke( EDIT_NO_ECHO | EDIT_SWAP_SCROLL ), MAP_GEN | MAP_EDIT );
                                if (( c != CUR_UP ) && ( c != CUR_DOWN ) && ( c != 4 ))
                                        goto egots_key;
                        }

                        break;

                case PgUp:      // display a popup history selection box
                case PgDn:
                        c = PgUp;

                case CTL_PgUp:  // display a popup directory selection box
                case CTL_PgDn:
                        {
                        extern int gnPopExitKey;
                        char * *list = 0L;
                        int saved_char;
                        unsigned long size = 0L;

                        fptr = (( c == PgUp )  ? glpHistoryList : glpDirHistory );
                        saved_char = c;
                        i = c = 0;

                        // disable if not getting command line buffer
                        if ( nEditFlag & EDIT_COMMAND ) {

                            // get the list into an array
                            for ( ; ( *fptr ); fptr = next_env( fptr )) {

                                // check for partial match
                                if (( saved_char == PgUp ) && ( *base ) && ( strnicmp(base,fptr,strlen( base )) != 0 ))
                                        continue;

                                // allocate memory for 256 entries at a time
                                if (( c % 256 ) == 0 ) {
                                        size += 1024;
                                        list = (char * *)ReallocMem( (char *)list, size );
                                }

                                list[ c++ ] = fptr;
                                if ( saved_char == PgUp ) {
                                        // check for current history pointer
                                        if ( fptr == glpHptr - 1 )
                                                i = c + 1;
                                        else if ( fptr == glpHptr )
                                                i = c;
                                }
                            }
                        }

                        // no entries or no matches?
                        if ( c == 0 ) {
                                honk();
                                break;
                        }

                        if (( i == 0 ) || ( i > c ))
                                i = c;

                        // display the popup selection list
                        if (( fptr = wPopSelect( gpIniptr->PWTop, gpIniptr->PWLeft, gpIniptr->PWHeight, gpIniptr->PWWidth, list, c, i, (( saved_char == PgUp ) ? HISTORY_TITLE : DIRECTORY_TITLE ), NULL, NULL, 0 )) != 0L ) {

                                if ( saved_char == PgUp ) {

                                        strcpy( base, fptr );
                                        glpHptr = fptr;

                                        // display line & set cursor to end
                                        PositionEGets( base );
                                        efputs( base );
                                        curpos = strend( base );

                                } else {
                                        strcpy( source, fptr );
                                        if ( gnPopExitKey == LF ) {
                                                strins( curpos, source );
                                                efputs( curpos );
                                        }
                                }
                        }

                        FreeMem( (char *)list );

                        // reenable signal handling after cleanup
                        EnableSignals();

                        // if we had a ^C in wPopSelect, abort after cleanup
                        if ( cv.exception_flag ) {
                                crlf();
                                longjmp( cv.env, -1 );
                        }

                        if ( gnPopExitKey == CR ) {

                                if ( saved_char != PgUp ) {
                                        __cd( source, CD_CHANGEDRIVE | CD_SAVELASTDIR );
                                        if ( *base == '\0' )
                                                goto egots_cr_key;
                                } else
                                        goto egots_cr_key;
                        }

                        break;
                        }

                case BACKSPACE:
                case DEL:
                    if ( c == BACKSPACE ) {

                        if ( curpos == base )
                                break;
                        curpos--;

                    }

                    if ( *curpos ) {
                        c = PositionEGets( strend( curpos ));
                        strcpy( curpos, curpos+1 );
                        c -= PositionEGets( strend( curpos ));
                        PositionEGets( curpos );
                        efputs( curpos );
                        PadBlanks( c );
                    }
                    break;

                case F8:
                        // F8 not allowed except following F9 / F10
                        honk();
                        break;

                case F10:
                        // convert an initial F10 to an F9
                        c = F9;
                case F7:
                        // popup a listbox with matching filenames
                case F9:
                    // F9 - substitute file name
                    // F8 - substitute previous file name
                    // F10 = append next matching filename
                    {

                    char fname[MAXFILENAME], *start;
                    char * *list = 0L, *fpListStart;
                    int fval, length, offset, fIncludeList = 0;
                    unsigned int list_size = 0xFFF0;
                    unsigned long size = 0L;
                    FILESEARCH dir;

                    INVALID_CHARS[1] = ' ';

                    // disable if not getting command line buffer
                    if ( nEditFlag & EDIT_COMMAND ) {

                        INVALID_CHARS[0] = gpIniptr->SwChr;
                        if (( gpIniptr->UnixPaths ) && ( INVALID_CHARS[0] == '/' ))
                                INVALID_CHARS[0] = ' ';

RedoFileName:
                        // find start of filename
                        for ( arg = NULL, start = curpos; ( start > base ); start--) {

                                // HPFS / NTFS / LFN quoted name?
                                if ( start[-1] == '"' ) {
                                        if (( --start > base ) && ( strchr( " \t", start[-1] ) == NULL )) {
                                                while ((--start > base ) && (*start != '"' ))
                                                        ;
                                        }
                                        break;
                                }

                                // check for an include list entry
                                if ( start[-1] == ';' ) {

                                        // check for DR-DOS password
                                        if ( start[-2] == ';' )
                                                start--;
                                        else if ( arg == NULL ) {
                                                fIncludeList = 1;
                                                arg = start;
                                        }

                                } else if (( start[-1] < 33 ) || ( strchr( INVALID_CHARS, start[-1] ) != NULL ))
                                        break;
                        }

                        // determine the filename length
                        length = fname_length( start );

                        // save original source template
                        sprintf( source, FMT_PREC_STR, (( length > MAXFILENAME ) ? MAXFILENAME : length ), start );

                        // is this an include list entry?
                        if ( arg != NULL ) {

                                length = fname_length( arg );
                                sprintf( fname, FMT_PREC_STR, (( length > MAXFILENAME) ? MAXFILENAME : length ), arg );

                                if ( path_part( fname ) == NULL )
                                        insert_path( source, fname, source );
                                else
                                        strcpy( source, fname );
                                start = arg;
                        }

                        // replace "." and ".." w/expanded name
                        if ((( arg = fname_part( source )) != NULL ) && (( stricmp( arg, "." ) == 0 ) || ( stricmp( arg, ".." ) == 0 ))) {
                                mkfname( source, 0 );
                                mkdirname( source, (( ifs_type( source ) != 0 ) ? "*" : WILD_FILE ));
                        } else {
                                // append wildcard(s)
                                arg = ext_part( source );
                                strcat( source, ((( ifs_type( source ) != 0 ) || ( arg != NULL )) ? "*" : WILD_FILE ));
                        }

                        // if we have a "...", we need to expand it
                        if ( strstr( source, "..." ) != NULL )
                                mkfname( source, 0 );
                        else
                                StripQuotes( source );

                        // determine the filename length
                        length = fname_length( start );

                        for ( offset = -1, fval = FIND_FIRST; ; ) {

                                if ( c == F8 ) {

                                        // get previous entry
                                        // already at start of dir?
                                        if ( offset <= 0 )
                                                honk();
                                        else
                                                offset--;

                                        fval = FIND_FIRST;
                                }

                                for ( n = 0; ; ) {

                                        int nMode = 0;

                                        // kludge for embedded ' in first arg
                                        ptr = skipspace( base );
                                        ptr = ntharg( base, (( *ptr == '\'' ) ? 0 : 0x1000 ));
                                        if (( ptr != NULL ) && ( start > base + strlen( ptr )))
                                                nMode = 0x7;

                                        // check for matching file or dir
                                        if (( arg = find_file( fval, source, (( fLFN ) ? 0x8110 : 0x8190 ) | nMode, &dir, fname )) == NULL ) {

                                            if (( fval == FIND_FIRST ) && ( *start == '@' )) {
                                                INVALID_CHARS[1] = '@';
                                                goto RedoFileName;
                                            }
                                            break;
                                        }

                                        fval = FIND_NEXT;

                                        if (( ptr != NULL ) && ( start > base + strlen( ptr ))) {

                                                // check FileCompletion arg
                                                if ( TabComplete( ptr, arg, dir.attrib ) == 0 )
                                                        continue;
                                        }

                                        // adjust for a LFN with
                                        //   embedded whitespace
                                        AddQuotes( arg );

                                        // if first arg on the line, only
                                        //   display dirs and executable exts
                                        if (( c != F10 ) && ( start == skipspace( base ) ) && ( source[ strlen( source ) - 1 ] == '*' )) {

                                                if ( dir.attrib & _A_SUBDIR )
                                                        strcat( arg, "\\" );

                                                else {

                                                        if (( ptr = ext_part( arg )) == NULL )
                                                                continue;

                                                        for ( i = 0; ( executables[i] != NULL ); i++ ) {
                                                                if ( _stricmp( ptr, executables[i] ) == 0 )
                                                                        break;
                                                        }

                                                        if (( executables[i] == NULL ) && ( *(executable_ext( ptr )) == '\0' ))
                                                                continue;
                                                }

                                        } else if (( gpIniptr->AppendDir ) && ( dir.attrib & _A_SUBDIR ) && ( fIncludeList == 0 ))
                                                strcat( arg, "\\" );

                                        i = strlen( arg ) + strlen( base );
                                        if ( c == F10 )
                                                i += 2;
                                        else
                                                i -= length;

                                        if ( i >= nMaxLength ) {
                                                arg = NULL;
                                                break;
                                        }

                                        // remove a path added for include lists
                                        if ( fIncludeList )
                                                strcpy( arg, fname_part( fname ) );

                                        if ( c == F7 ) {

                                                // allocate memory for 64 entries at a time
                                                if (( n % 64 ) == 0 ) {

                                                        size += 256;
                                                        if (( list = (char * *)ReallocMem((char *)list,size )) == 0L ) {
                                                                n = 0;
                                                                break;
                                                        }

                                                        if ( n == 0 ) {
                                                                HoldSignals();
                                                                fptr = AllocMem( &list_size );
                                                                list_size -= 0xFF;
                                                                list[0] = fpListStart = fptr;
                                                        }
                                                }

                                                // add filename to list
                                                i = strlen( arg ) + 1;

                                                if (( list == 0L ) || ( fpListStart == 0L ) || (((unsigned int)( fptr - fpListStart ) + i) >= list_size )) {
                                                    FreeMem( fpListStart );
                                                    FreeMem((char *)list );
                                                    goto egets_key;
                                                }

                                                StripQuotes( arg );
                                                list[ n ] = strcpy( fptr, arg );
                                                fptr += i;
                                        }

                                        // if this isn't a "get previous"
                                        //   then increment offset & break now
                                        if (( n++ >= offset) && ( c == F8 ))
                                                break;

                                        else if (( c != F7 ) && ( c != F8 )) {
                                                offset++;
                                                break;
                                        }
                                }

                                // display the popup selection list
                                if (( c == F7 ) && ( n > 0 )) {

                                        if (( fptr = wPopSelect( gpIniptr->PWTop, gpIniptr->PWLeft, gpIniptr->PWHeight, gpIniptr->PWWidth, list, n, 1, FILENAMES_TITLE, NULL, NULL, 1 )) != 0L ) {

                                                strcpy( fname, fptr );
                                                arg = strend( fname ) - 1;
                                                if ( *arg == '\\' ) {
                                                        *arg = '\0';
                                                        AddQuotes( fname );
                                                        strcat( fname, "\\" );
                                                } else
                                                        AddQuotes( fname );
                                                arg = fname;
                                        }

                                        FreeMem( fpListStart );
                                        FreeMem( (char *)list );

                                        // reenable signal handling after cleanup
                                        EnableSignals();

                                        if ( fptr == 0L )
                                                goto egets_key;

                                        // if we had a ^C in wPopSelect, abort after cleanup
                                        if ( cv.exception_flag ) {
                                                crlf();
                                                longjmp( cv.env, -1 );
                                        }
                                }

                                if ( arg != NULL ) {

                                        // collapse the command line
                                        //   and insert new filename & path
                                        if ( c == F10 ) {

                                                // skip past previous arg
                                                start += length;

                                                // insert space for next match
                                                strins( arg, " " );

                                        } else {
                                                // clear to EOL
                                                clearline( start );
                                                strcpy( start, start + length );
                                        }

                                        curpos = start;
                                        PositionEGets( curpos );

                                        strins( curpos, arg );
                                        efputs( curpos );

                                        // put cursor at end of new argument
                                        length = strlen( arg );
                                        curpos += length;

                                        // check for wrap at end of screen
                                        PositionEGets( strend( curpos ));
                                        PositionEGets( curpos );

                                        // adjust for inserted space in F10
                                        if ( c == F10 ) {
                                                start++;
                                                length--;
                                        }

                                } else
                                        honk();

                                if ( c == F7 )
                                        goto egets_key;

                                c = cvtkey( GetKeystroke( EDIT_NO_ECHO | EDIT_SWAP_SCROLL ), MAP_GEN | MAP_EDIT );

                                if (( c != F8 ) && ( c != F9 ) && ( c != F10 ))
                                        goto egots_key;
                        }
                    }
                    }

                default:

                        // allow users to enter characters normally trapped by
                        //  4xxx by preceding them with an ASCII 255
                        if ( c == 0xFF ) {

                            if ( *kptr == '\0' )
                                c = GetKeystroke( EDIT_NO_ECHO | nEditFlag );
                            else
                                c = *kptr++;

                        } else {

                            // clear high bit which may have been set by cvtkey
                            c &= 0x7FFF;

                            // check for a redefined key (Fn key, Alt key, etc.)
                            if (( *kptr == '\0' ) && (( c < 32 ) || ( c > 0xFF ))) {

                                // check for user defined key aliases
                                for ( kptr = glpAliasList; ( *kptr != '\0' ); kptr = next_env( kptr )) {

                                    if ( *kptr == '@' ) {

                                        // @@n means "execute immediately, with
                                        //   no echo"
                                        if ( kptr[1] == '@' ) {
                                            fExecuteNow = 1;
                                            fEcho = 0;
                                            kptr++;
                                        }

                                        // accept either "@59" or "@F1" syntax
                                        if ( isdigit( kptr[1] ) == 0 )
                                            kptr++;

                                        sscanf( kptr, "%*[^=]=%n", &n );

                                        // matched the current key?
                                        if (( n > 0 ) && (( i = keyparse( kptr, n-1 )) == c )) {
                                            // we found a key alias
                                            kptr += n;
                                            goto egets_key;
                                        }
                                    }

                                    fEcho = 1;
                                    fExecuteNow = 0;
                                }
                            }
                        }

                        // check for invalid or overlength entry
                        if (( c > 0xFF ) || (( strlen( base ) >= (unsigned int)nMaxLength ) && ((*curpos == '\0' ) || (fInsert))))
                                honk();

                        else if (( nEditFlag & EDIT_DIGITS ) && ( isdigit( c ) == 0 ))
                                honk();

                        else {
                                // open a hole for insertions
                                if ( fInsert )
                                        memmove( curpos + 1, curpos, strlen( curpos )+1 );
                                else if ( *curpos == TAB )      // collapse tab
                                        clearline( curpos );

                                // add new terminator if at EOL
                                if ( *curpos == '\0' )
                                        curpos[1] = '\0';
                                *curpos = (char)c;
                                if ( fEcho )
                                        efputs( curpos );
                                curpos++;

                                // adjust home row if necessary
                                if ( fEcho )
                                        PositionEGets( strend( curpos ));
                        }
                }

                // get & set cursor position
                if ( fEcho )
                        PositionEGets( curpos );
        }
}


// check the FileComplete .INI directive for a matching extension / type
static int TabComplete( char *pszCommand, char *pszFilename, int nAttribute )
{
        int i, j;
        int nLength;
        char *arg, *pszExt, szName[128], szExtensions[128];
        char *fpszComplete;
        char *fptr;

        arg = _alloca( strlen( pszCommand ) + 1 );
        pszCommand = strcpy( arg, pszCommand );

        // if no FileCompletion variable, was FileCompletion defined in *.INI?
        if ((( fpszComplete = get_variable( "FileCompletion" )) == 0L ) && ( gpIniptr->FC != INI_EMPTYSTR ))
                fpszComplete = (char *)( gpIniptr->StrData + gpIniptr->FC );

        if ( fpszComplete == 0L )
                return (( nAttribute & ( _A_HIDDEN | _A_SYSTEM )) ? 0 : 1 );

        if ((( pszExt = ext_part( pszFilename )) != NULL ) && ( *pszExt == '.' ))
                pszExt++;

        while ( *fpszComplete ) {

                // get the next filename in completion list
                szName[0] = '\0';
                nLength = 0;
                sscanf( fpszComplete, " %127[^ \t:;] %n", szName, &nLength );

                // does it match the first command?
                arg = fname_part( pszCommand );
                if (( arg != NULL ) && ( stricmp( arg, szName ) == 0 ) && (( fptr = strchr( fpszComplete, ':' )) != 0L )) {

                        // get the matching extensions for this filename
                        sscanf( ++fptr, "%127[^;]", szExtensions );

                        for ( i = 0; (( arg = ntharg( szExtensions, i )) != NULL ); i++ ) {

                            if (( pszExt != NULL ) && ( wild_cmp( arg, pszExt, TRUE, TRUE ) == 0 ) && (( nAttribute & ( _A_HIDDEN | _A_SYSTEM )) == 0 ))
                                return 1;

                            // check for "complete by attributes"
                            if ( strlen( arg ) > 3 ) {
                                for ( j = 0; ( j < 5 ); j++ ) {
                                    if (( stricmp( arg, colorize_atts[j].type ) == 0 ) && ( nAttribute & colorize_atts[j].attr ))
                                        return 1;
                                }
                            }
                        }

                        return 0;
                }

                fpszComplete += nLength;

                if (( *fpszComplete == ':' ) || ( *fpszComplete == ';' )) {
                        // skip the extensions & get next filename
                        while (( *fpszComplete ) && ( *fpszComplete++ != ';' ))
                                ;
                }
        }

        return (( nAttribute & ( _A_HIDDEN | _A_SYSTEM )) ? 0 : 1 );
}


// return the length of the filename (bounded by INVALID_CHARS)
static int fname_length( char *fname )
{
        char *ptr;

        // determine the filename length
        // allow quoted filenames
        ptr = scan( fname, INVALID_CHARS, QUOTES + 1 );

        return ((int)( ptr - fname ));
}


// get & set column position in the current line
static int PositionEGets( char *pCur )
{
        char *ptr;

        nCursorColumn = nHomeColumn;

        // increment column position
        for ( ptr = base; ( ptr != pCur ); ptr++ )
                incr_column( *ptr, &nCursorColumn );

        // check for line wrap
        nCursorRow = nHomeRow + ( nCursorColumn / nScreenColumns );
        nCursorColumn = ( nCursorColumn % nScreenColumns );

        // adjust cursor row for wrap at end of screen
        if ( nCursorRow > nScreenRows ) {
                nHomeRow -= ( nCursorRow - nScreenRows );
                nCursorRow = nScreenRows;
        }

        // set cursor to "curpos"
        SetCurPos( nCursorRow, nCursorColumn );

        // return length in columns from base to curpos
        return ((( nCursorRow - nHomeRow ) * nScreenColumns ) + ( nCursorColumn - nHomeColumn ));
}


// scrub the line from "ptr" onwards
static void clearline( char *ptr )
{
        int i;

        i = PositionEGets( strend( ptr ));

        // position cursor at "ptr" and fill line with blanks
        i -= PositionEGets( ptr );
        PadBlanks( i );
        PositionEGets( ptr );
}


// print the string to STDOUT
void efputs( char *s )
{
        int row = 0;
        char *szPadStr;
        int column;

        // if password field, display *'s instead of the characters
        szPadStr = (( fPassword ) ? "********" : NULLSTR );
        for ( ; *s; s++ ) {

                column = nCursorColumn;
                incr_column( *s, &nCursorColumn );

                if ( *s == TAB ) {
                        // do tab expansion
                        color_printf( nColor, FMT_LEFT_STR, ( nCursorColumn - column ), szPadStr );
                        column = nCursorColumn;
                } else
                        color_printf( nColor, FMT_CHAR, (( fPassword ) ? '*' : *s ));

                if (( nColor != -1 ) && ( row < ( nCursorColumn / nScreenColumns ))) {
                        crlf();
                        row++;
                }
        }
}


// print the specified number of spaces
static void PadBlanks( int width )
{
        color_printf( (( nColor == (int)( gpIniptr->InputColor )) ? -1 : nColor ), FMT_LEFT_STR, width, NULLSTR );
}

