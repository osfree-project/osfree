// LISTALL.C - file listing routines for 4xxx / TCMD family
//   Copyright (c) 1993 - 1998  Rex C. Conn  All rights reserved

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "4all.h"


static int ListInit( void );
static int ListOpenFile( char *);
static void ListHome( void );
static int ListMoveLine( int);
static LONG MoveViewPtr(LONG, LONG *);
static LONG ComputeLines(LONG,LONG);
static VOID SetRightMargin( void );
static INT DisplayLine(UINT, LONG);
int FormatLine( char [], LONG, int *, int);
static BOOL SearchFile( long );
VOID ListPrintFile( void );
void ListSetCurrent(LONG);
static void ListFileRead(PCH, unsigned int);
static int GetPrevChar( void );
static int GetNextChar( void );


LISTFILESTRUCT LFile;

CHAR  szListFindWhat[80] = "";
CHAR  szFFindFindWhat[128] = "";
static char szClip[MAXFILENAME];

static BOOL bListSkipLine = 0;
BOOL  fEndSearch = 0;

static int nScreenRows;         // screen size in rows
static int nScreenColumns;      // screen size in columns
static int nRightMargin;        // right margin (column #)
static int nNormal;         // normal video attribute
static int nInverse;            // inverse video attribute
static int nStart;          // start index & current index for
static int nCurrent;            // "previous file" support
long uListFlags;            // LIST options (/H, /S, /W, /X)


#include "listc.c"


extern RANGES aRanges;

// FFIND structure
typedef struct
{
    unsigned long fFlags;
    unsigned int uMode;
    long lFilesFound;
    long lLinesFound;
    char szDrives[32];
    char szSource[MAXFILENAME];
    char szFilename[MAXFILENAME];
    char szText[512];
} FFIND_STRUCT;

static int _ffind( FFIND_STRUCT * );


// find files or text
int ffind_cmd( int argc, char **argv )
{
    char *arg, *ptr;
    int i, n, nLength, rval = 0;
    FFIND_STRUCT Find;
    LISTFILESTRUCT SaveLFile;

    // initialize variables for SearchDirectory()
    init_dir();

    memset( &Find, '\0', sizeof( FFIND_STRUCT) );
    fEndSearch = 0;
    uListFlags = 0L;

    // turn off the error reporting in SearchDirectory()
    glDirFlags |= DIRFLAGS_RECURSE;

    // no "." or ".."
    glDirFlags |= DIRFLAGS_NO_DOTS;

    sprintf( Find.szDrives, FMT_CHAR, gcdisk( NULL ) + 64 );

//      if ( argv[1] == NULL )
//      return ( usage( FFIND_USAGE ));

        // get date/time/size ranges
        if ( GetRange( argv[1], &aRanges, 1 ) != 0 )
        return ERROR_EXIT;

        init_page_size();

        // check for and remove switches; abort if no filename arguments
        for ( argc = 0; (( arg = ntharg( argv[1], argc | 0x1000 )) != NULL ) && ( *arg == gpIniptr->SwChr ); argc++ ) {

        // point past switch character
        for ( arg++; ( *arg != '\0' ); ) {

            switch ( _ctoupper( *arg++ )) {
            case 'A':
                // retrieve based on specified atts
                arg = GetSearchAttributes( arg );
                Find.uMode = 0x417;
                break;

            case 'B':   // bare display ( filenames only)
                glDirFlags |= (DIRFLAGS_NAMES_ONLY | DIRFLAGS_NO_HEADER | DIRFLAGS_NO_FOOTER);
                break;

            case 'C':   // case-sensitive
                Find.fFlags |= FFIND_CHECK_CASE;
                break;

            case 'D':   // search entire disk(s)
                Find.fFlags |= ( FFIND_SUBDIRS | FFIND_ALL);

                // check for additional drives
                if ( *arg ) {
                    sscanf( arg, "%31s%n", Find.szDrives, &i );
                    arg += i;
                    strupr( Find.szDrives );
                }
                break;

            case 'E':   // display filenames in upper case
                glDirFlags |= DIRFLAGS_UPPER_CASE;
                break;

            case 'I':
                Find.fFlags |= FFIND_NOWILDCARDS;
                break;

            case 'K':   // no header
                glDirFlags |= DIRFLAGS_NO_HEADER;
                break;

            case 'L':   // display line numbers
                Find.fFlags |= FFIND_LINE_NUMBERS;
                break;

            case 'M':   // no footer
                glDirFlags |= DIRFLAGS_NO_FOOTER;
                break;

            case 'O':   // dir sort order
                // kludges for DOS 5 format
                arg = dir_sort_order( arg );
                break;

            case 'P':   // pause after each page
                gnPageLength = GetScrRows();
                break;

            case 'R':   // reverse search (from end)
                Find.fFlags |= FFIND_REVERSE_SEARCH;
                break;

            case 'S':   // search subdirectories
                Find.fFlags |= FFIND_SUBDIRS;
                break;

            case 'V':   // show all matching lines / offsets
                Find.fFlags |= FFIND_SHOWALL;
                break;

            case 'X':   // hex search
                Find.fFlags |= FFIND_HEX_DISPLAY;
                if ( _ctoupper( *arg ) == 'T' )
                    arg++;
                else {
                    if ( *arg == '\0' )
                        break;
                    Find.fFlags |= FFIND_HEX_SEARCH;
                }

            case 'T':   // text search
                if ( *arg != '\0' ) {
                    if ( *arg == '"' )
                    sscanf( ++arg, "%127[^\"]", szFFindFindWhat );
                    else
                    strcpy( szFFindFindWhat, arg );

                    arg = NULLSTR;
                    Find.fFlags |= FFIND_TEXT;
                    break;
                }

            default:    // invalid option
                error( ERROR_INVALID_PARAMETER, arg-1 );
                return ( usage( FFIND_USAGE ));
            }
        }
        }

        // skip the switch statements
        if ( gpNthptr == NULL ) {
        if ( QueryIsPipeHandle( STDIN ))
            argv[1] = CONSOLE;
        else
            return ( usage( FFIND_USAGE ));
        } else
        strcpy( argv[1], gpNthptr );

    Find.fFlags |= FFIND_NOERROR;

    if ( Find.fFlags & FFIND_TEXT ) {

        // save the LIST variables
        memmove( &SaveLFile, &LFile, sizeof(LISTFILESTRUCT) );

        // allocate buffer if searching for text
        ListInit();

        // don't search for directories for text!
        Find.uMode &= 0x407;
    }

    if ( setjmp( cv.env ) == -1 ) {
        rval = CTRLC;
        goto ffind_bye;
    }

    for ( argc = 0; ( fEndSearch == 0 ); argc++ ) {

        for ( n = 0; (( Find.szDrives[n] != '\0' ) && ( fEndSearch == 0 )); n++ ) {

        if (( arg = ntharg( argv[1], argc )) == NULL ) {
            fEndSearch = 1;
            break;
        }

        // no multi-drive specs if drive already named!
        if (( Find.fFlags & FFIND_ALL ) && ( arg[1] != ':' ) && (arg[1] != '\\' )) {

            // check for drive range (c-f)
            if ( Find.szDrives[n] == '-' ) {
                if (( n > 0 ) && ( Find.szDrives[n+1] > (char)( Find.szDrives[n-1] + 1)))
                    Find.szDrives[--n] += 1;
                else
                    n++;
            }
            i = Find.szDrives[n];

            strins( arg, " :\\" );
            *arg = (char)i;

        } else
            n = strlen( Find.szDrives ) - 1;

        // build the source name
        if (( QueryIsDevice( arg )) && ( QueryIsCON( arg )))
           uListFlags |= LIST_STDIN;

        else if ( mkfname( arg, 0 ) == NULL ) {
            rval = ERROR_EXIT;
            goto ffind_bye;

        } else {

            if ( glDirFlags & DIRFLAGS_UPPER_CASE )
            strupr( arg );

            // check to see if it's an HPFS partition
            if ( ifs_type( arg ) != 0 )
            glDirFlags |= DIRFLAGS_HPFS;

            // if a directory, append "\*.*"
            // if not a directory, append a ".*" to files with no extension
            if ( is_dir( arg ))
                mkdirname( arg, (( glDirFlags & DIRFLAGS_HPFS ) ? "*" : WILD_FILE ));
            // don't do this stuff on an LFN / NTFS volume!
            else if (( glDirFlags & DIRFLAGS_HPFS ) == 0 ) {

                // if no include list & no extension specified, add default one
                //   else if no filename, insert a wildcard filename
                if ( strchr( arg, ';' ) == NULL ) {

                    if ( ext_part( arg ) == NULL ) {
            if ( strpbrk( arg, WILD_CHARS ) == NULL )
                            strcat( arg, WILD_FILE );
                    } else {
                        // point to the beginning of the filename
                        ptr = arg + strlen( path_part( arg ));
                        if ( *ptr == '.' )
                            strins( ptr, "*" );
                    }
                }
            }
        }

        copy_filename( Find.szSource, arg );

        // save the source filename part ( for recursive calls &
        //   include lists)
        nLength = 0;
        if (( arg = path_part( Find.szSource )) != NULL )
            nLength = strlen( arg );
        strcpy( Find.szFilename, Find.szSource + nLength );

        i = _ffind( &Find );

        if (( setjmp( cv.env) == -1 ) || ( i == CTRLC )) {
            rval = CTRLC;
            fEndSearch = 1;
            break;
        }
        EnableSignals();
        if ( i != 0 )
            rval = ERROR_EXIT;
        }
    }

    if (( glDirFlags & DIRFLAGS_NO_FOOTER ) == 0 ) {
        // display number of lines & files found
        crlf();
        _page_break();
        if ( Find.fFlags & FFIND_TEXT )
            printf( FFIND_TEXT_FOUND, Find.lLinesFound, (( Find.lLinesFound == 1) ? FFIND_ONE_LINE : FFIND_MANY_LINES) );
        FilesProcessed( FFIND_FOUND, Find.lFilesFound );
        crlf();
        _page_break();
    }

ffind_bye:
    if ( Find.fFlags & FFIND_TEXT ) {

        FreeMem( (char *)LFile.lpBufferStart );
        if ( LFile.hHandle > 0 )
            _close( LFile.hHandle );
        LFile.hHandle = 0;

        // restore the LIST variables
        memmove( &LFile, &SaveLFile, sizeof(LISTFILESTRUCT) );
    }

    // disable signal handling momentarily
    HoldSignals();

    return (( Find.lFilesFound != 0L ) ? rval : ERROR_EXIT );
}


static int _ffind( FFIND_STRUCT *Find )
{
    unsigned int i;
    char *arg;
    int c, n = 0, rval = 0, nLength;
    unsigned int entries = 0;
    long lLines;
    DIR_ENTRY *files = 0L;    // file array in system memory

    // trap ^C and clean up
    if ( setjmp( cv.env ) == -1 ) {
        dir_free( files );
        return CTRLC;
    }

    EnableSignals();
    if ( glDirFlags & DIRFLAGS_UPPER_CASE )
        strupr( Find->szSource );
    else if (( glDirFlags & DIRFLAGS_HPFS ) == 0 )
        strlwr( Find->szSource );

    // look for matches
    if ((( uListFlags & LIST_STDIN ) == 0 ) && ( SearchDirectory(( Find->uMode | FIND_DATERANGE ), Find->szSource, (DIR_ENTRY **)&files, &entries, &aRanges, 0 ) != 0 )) {
        rval = ERROR_EXIT;

    } else {

        if ( uListFlags & LIST_STDIN )
            entries = 1;
        else
            arg = path_part( Find->szSource );

        for ( i = 0; (( i < entries ) && ( fEndSearch == 0 )); i++ ) {

            if ( uListFlags & LIST_STDIN )
                strcpy( LFile.szName, "CON" );
            else
                sprintf( LFile.szName, "%s%Fs", arg, (( glDirFlags & DIRFLAGS_HPFS ) ? files[i].hpfs_name : files[i].file_name ));

            if ( Find->fFlags & FFIND_TEXT ) {

                if ( uListFlags & LIST_STDIN ) {

                    LFile.hHandle = STDIN;
                    LFile.lSize = (ULONG)-1;
                } else if (( LFile.hHandle = _sopen( LFile.szName, (O_BINARY | O_RDONLY), SH_DENYNO )) == -1 ) {
                    rval = error( _doserrno, LFile.szName );
                    continue;
                }

                if ( LFile.hHandle != STDIN )
                    LFile.lSize = QuerySeekSize( LFile.hHandle );
                LFile.lpEOF = 0L;

                ListHome();
                lLines = 0L;
                if ( Find->fFlags & FFIND_REVERSE_SEARCH ) {
                    // goto the last row
                    while ( ListMoveLine( 1 ) != 0 )
                        ;
// TODO:                }

                    // display filename
                    if ( glDirFlags & DIRFLAGS_NAMES_ONLY ) {
                        // if /B, _only_ display name!
                        more_page( LFile.szName, 0 );
                        break;
                    }

                    if (( n == 0 ) && (( glDirFlags & DIRFLAGS_NO_HEADER) == 0 )) {
                        crlf();
                        _page_break();
                        sprintf( Find->szText, "---- %s", LFile.szName );
                        more_page( Find->szText, 0 );
                        n++;
                    }

                    if ( Find->fFlags & FFIND_HEX_DISPLAY ) {
                        sprintf( Find->szText, FFIND_OFFSET, LFile.lViewPtr, LFile.lViewPtr );
                        more_page( Find->szText, 0 );

                    } else {

                        // display line number
                        nLength = 0;
                        if ( Find->fFlags & FFIND_LINE_NUMBERS )
                        nLength = sprintf( Find->szText, "[%ld] ", LFile.lCurrentLine + gpIniptr->ListRowStart );

                        // display the line
                        while ((( c = GetNextChar()) != EOF ) && ( c != LF ) && ( c != CR ) && ( nLength < 511)) {
                        if ( c == 0 )
                            c = ' ';
                        Find->szText[ nLength++ ] = (char)c;
                        }
                        Find->szText[ nLength ] = '\0';
                        more_page( Find->szText, 0 );
                    }

                    if (( Find->fFlags & FFIND_SHOWALL ) == 0 )
                    break;

                    // skip to next line
                    if (( Find->fFlags & FFIND_REVERSE_SEARCH ) == 0 ) {
                    if ( Find->fFlags & FFIND_HEX_DISPLAY )
                        LFile.lViewPtr += LFile.nSearchLen;
                        else if ( ListMoveLine( 1 ) == 0 )
                        break;
                    }

                    ListSetCurrent( LFile.lViewPtr );
                }

                _close( LFile.hHandle );
                LFile.hHandle = 0;
                if ( lLines == 0L )
                    continue;

                Find->lLinesFound += lLines;

            } else {
                more_page( LFile.szName, 0 );
            }

            Find->lFilesFound++;
        }

        dir_free( files );
        files = 0L;

        // search subdirectories too?
        if ( Find->fFlags & FFIND_SUBDIRS ) {

            insert_path( Find->szSource, WILD_FILE, path_part( Find->szSource ) );

            // save the current source filename start
            arg = strchr( Find->szSource, '*' );
            entries = 0;

            // search for subdirectories
            if ( SearchDirectory((( Find->uMode & 0xF) | 0x210), Find->szSource, (DIR_ENTRY **)&files, &entries, &aRanges, (( gszSortSequence[0] == '\0' ) ? 2 : 0 )) != 0 ) {
                rval = ERROR_EXIT;
                goto find_abort;
            }

            for ( i = 0; (( i < entries ) && ( fEndSearch == 0 )); i++ ) {

                // make directory name
                sprintf( arg, "%Fs\\%s", (( glDirFlags & DIRFLAGS_HPFS) ? files[i].hpfs_name : files[i].file_name ), Find->szFilename );

                rval = _ffind( Find );

                // check for ^C and reset ^C trapping
                if (( setjmp( cv.env ) == -1 ) || ( rval == CTRLC ))
                    rval = CTRLC;

                // quit on error (probably out of memory)
                if ( rval )
                    break;

                EnableSignals();
            }

            dir_free( files );
            files = 0L;
        }
    }
find_abort:
    // disable signal handling momentarily
    HoldSignals();
    return rval;
}


// initialize the LIST buffers & globals
static int ListInit( void )
{
    // allocate 256K for 4OS2 & TCMD/OS2 & 4NT & TCMD/32
    LFile.uTotalSize = 0x40000;

    if (( LFile.lpBufferStart = AllocMem( &LFile.uTotalSize )) == 0L )
        return ( error( ERROR_NOT_ENOUGH_MEMORY, NULL ));

    LFile.uBufferSize = ( LFile.uTotalSize / 2 );
    LFile.lpBufferEnd = LFile.lpBufferStart + LFile.uTotalSize;
    LFile.lpCurrent = LFile.lpBufferStart;
    LFile.lpEOF = 0L;

    // initialize LISTFILESTRUCT parms
    LFile.szName[0] = '\0';
    LFile.hHandle = 0;
    LFile.lSize = LFile.lCurrentLine = 0L;

    LFile.lViewPtr = LFile.lFileOffset = 0L;

    // set screen sizes
    nScreenRows = GetScrRows();
    nScreenColumns = GetScrCols();
    nScreenColumns--;

    return 0;
}


// open the file & initialize buffers
static int ListOpenFile( char *fname )
{
    LFile.lpEOF = 0L;

    if ( uListFlags & LIST_STDIN ) {

        // reading from STDIN
        LFile.hHandle = STDIN;
        strcpy( LFile.szName, LIST_STDIN_MSG );
        LFile.lSize = (ULONG)-1;
    } else {

        if ( LFile.hHandle > 0 )
            _close( LFile.hHandle );

        StripQuotes( fname );
        if (( LFile.hHandle = _sopen( (( szClip[0] ) ? szClip : fname ), (O_BINARY | O_RDONLY), SH_DENYNO )) == -1 ) {
            return ( error( _doserrno, fname ));
        }
        LFile.lSize = QuerySeekSize( LFile.hHandle );
        strcpy( LFile.szName, fname );
    }

    ListHome();
    ListUpdateScreen();

    return 0;
}


// reset to the beginning of the file
static void ListHome( void )
{
    // Reset the memory buffer to purge old file data
    LFile.lpBufferEnd = LFile.lpBufferStart + LFile.uTotalSize;
    LFile.lpCurrent = LFile.lpBufferStart;
    LFile.lViewPtr = LFile.lFileOffset = 0L;
    LFile.lCurrentLine = 0L;
    LFile.nListHorizOffset = 0;

    ListFileRead( LFile.lpBufferStart, LFile.uTotalSize );

    // determine EOL character for this file
    for ( LFile.fEoL = CR; (( LFile.lpCurrent != LFile.lpEOF ) && ( LFile.lpCurrent < LFile.lpBufferEnd )); LFile.lpCurrent++ ) {
        if ( *LFile.lpCurrent == LF ) {
            LFile.fEoL = LF;
            break;
        }
    }

    LFile.lpCurrent = LFile.lpBufferStart;
}


// move up/down "n" lines
static int ListMoveLine( int nRows )
{
    long lTemp, lRows;

    lRows = nRows;
    lTemp = MoveViewPtr( LFile.lViewPtr, &lRows );

    if ( lRows == nRows ) {
        LFile.lCurrentLine += lRows;
        LFile.lViewPtr += lTemp;
        return 1;
    }

    return 0;
}


static VOID SetRightMargin( void )
{
    nRightMargin = (( uListFlags & LIST_WRAP ) ? nScreenColumns + 1 : 511 );
}


// read the current file into the specified buffer
static void ListFileRead( PCH fptr, unsigned int size )
{
    unsigned int uBytesRead = 0;

    // position the read pointer
    if ( LFile.hHandle == STDIN ) {

        if ( LFile.lpEOF != 0L )
            return;

        // read until buffer full or no more data
        do {
            if ( FileRead( LFile.hHandle, fptr, size, &uBytesRead ) != 0 )
                break;
            fptr += uBytesRead;

        } while (( uBytesRead > 0 ) && (( size -= uBytesRead ) > 0 ));
        if ( size == 0 )
            uBytesRead = 0;

        // are we at the end of the file yet?
        if ( uBytesRead == size )
            LFile.lpEOF = 0L;
        else {
            LFile.lpEOF = ( fptr + uBytesRead );
            LFile.lSize = (long)( fptr - (long)LFile.lpBufferStart ) + LFile.lFileOffset;
        }
        return;
    }

    _lseek( LFile.hHandle, LFile.lFileOffset, SEEK_SET );
    (void)FileRead( LFile.hHandle, fptr, size, &uBytesRead );

    // check for file length the same size as the input buffer!
    LFile.lpEOF = ((( LFile.lSize > (LONG)( LFile.lFileOffset + uBytesRead )) && ( uBytesRead == size )) ? 0L : fptr + uBytesRead );
}


// set LFile.lpCurrent to the specified file offset
void ListSetCurrent( long lOffset )
{
    if ( lOffset < LFile.lFileOffset ) {

        // get previous block
        while (( lOffset < LFile.lFileOffset ) && ( LFile.lFileOffset > 0L )) {
            if (( LFile.lFileOffset -= LFile.uTotalSize ) < 0L )
                LFile.lFileOffset = 0L;
        }

        ListFileRead( LFile.lpBufferStart, LFile.uTotalSize );

    } else if ( lOffset > (long)( LFile.lFileOffset + LFile.uTotalSize)) {

        // get next block
        while ( lOffset > (long)( LFile.lFileOffset + LFile.uTotalSize ))
            LFile.lFileOffset += LFile.uTotalSize;
        ListFileRead( LFile.lpBufferStart, LFile.uTotalSize );
    }

    // it's (now) in the current buffer
    LFile.lpCurrent = LFile.lpBufferStart + ( lOffset - LFile.lFileOffset );
}


// Display Line
static INT DisplayLine( UINT nRow, LONG lLinePtr )
{
    int i, n;
    INT nLength, nHOffset = 0, nHexOffset;
    int nBytesPrinted = 0;
    CHAR *arg, szBuffer[512], cSave;

    ListSetCurrent( lLinePtr );

    nLength = FormatLine( szBuffer, lLinePtr, &nBytesPrinted, TRUE );

    if (( uListFlags & LIST_HEX ) == 0 ) {
        nHOffset = LFile.nListHorizOffset;
        nLength -= LFile.nListHorizOffset;
        nHexOffset = 0;
    } else
        nHexOffset = 9;

    // adjust to max screen width
    if ( nLength > ( nScreenColumns + 1 ))
        nLength = nScreenColumns + 1;

    if (( nBytesPrinted > 0 ) && ( nLength > 0 )) {

        (void)VioWrtCharStrAtt( szBuffer+nHOffset, nLength, nRow, 0, (PBYTE)&nNormal, 0 );
    }

    // if we're displaying a search result, highlight the string
    if (( LFile.fDisplaySearch ) && ( strlen( szBuffer ) > (unsigned int)nHOffset )) {

        for ( i = 0; ; ) {

            if ( LFile.fDisplaySearch & 2 )
                arg = strstr( szBuffer + nHOffset + i + nHexOffset, szListFindWhat );
            else
                arg = stristr( szBuffer + nHOffset + i + nHexOffset, szListFindWhat );
            if ( arg == NULL )
                break;

            n = strlen( szListFindWhat );
            cSave = arg[ n ];
            arg[ n ] = '\0';
            i = (int)( arg - ( szBuffer + nHOffset ));
            WriteStrAtt( nRow, i, nInverse, arg );
            arg[ n ] = cSave;
            i += n;
        }
    }

    return nBytesPrinted;
}


// Compute number of lines between the specified points in the file
static LONG ComputeLines( LONG lOldPtr, LONG lNewPtr )
{
    int ch, n, fDirection = 0;
    LONG lRowCount;

    // always count upwards (it makes wrap computations a lot easier!)
    if ( lOldPtr > lNewPtr ) {
        lRowCount = lOldPtr;
        lOldPtr = lNewPtr;
        lNewPtr = lRowCount;
        fDirection++;
    }

    lRowCount = 0L;
    ListSetCurrent( lOldPtr );

    if ((uListFlags & LIST_HEX) == 0 ) {

        SetRightMargin();

        for ( n = 0; ( lOldPtr < lNewPtr ); lOldPtr++ ) {

        // don't call GetNextChar unless absolutely necessary!
        if ( LFile.lpCurrent == LFile.lpEOF )
            break;

        if ( LFile.lpCurrent < LFile.lpBufferEnd )
            ch = *LFile.lpCurrent++;
        else if (( ch = GetNextChar()) == EOF )
            break;

        // do tab expansion
        if ( ch == TAB )
            n = ((( n + TABSIZE ) / TABSIZE ) * TABSIZE );

        else if ((++n >= nRightMargin) || ( ch == LFile.fEoL )) {

            // if EOL increment the row count
            lRowCount++;
            n = 0;
        }
        }

    } else
        lRowCount = (( lNewPtr - lOldPtr) / 16 );

    return (( fDirection) ? -lRowCount : lRowCount );
}


// format the current line in either ASCII or hex mode
int FormatLine( char szBuffer[], long lLinePtr, int *nBytesPrinted, int fTabs )
{
    int i, n;
    char szScratch[4];

    *nBytesPrinted = 0;

    if (( uListFlags & LIST_HEX ) == 0 ) {

        // Print as ASCII until CR/LF or for "nRightMargin" characters.

        SetRightMargin();
        for ( i = 0; ( i < 511 ); ) {

            // don't call GetNextChar unless absolutely necessary
            if ( LFile.lpCurrent == LFile.lpEOF )
                break;

            if ( LFile.lpCurrent < LFile.lpBufferEnd )
                szBuffer[i] = *LFile.lpCurrent++;
            else
                szBuffer[i] = (char)GetNextChar();

            (*nBytesPrinted)++;

            if ( szBuffer[i] == (char)LFile.fEoL )
                break;

            // ignore CR's
            if ( szBuffer[i] == CR )
                continue;

            if ( i >= nRightMargin ) {
                GetPrevChar();
                (*nBytesPrinted)--;
                break;
            }

            // Expand Tab Characters
            if (( szBuffer[i] == TAB) && ( fTabs )) {

                n = ((( i + TABSIZE ) / TABSIZE ) * TABSIZE );
                for ( ; (( i < n ) && ( i < nRightMargin )); i++ ) {
                    // insert spaces until next tab stop
                    szBuffer[i] = ' ';
                }

            } else {
                if ( uListFlags & LIST_HIBIT )
                    szBuffer[i] &= 0x7F;
                i++;
            }
        }

        szBuffer[i] = '\0';

    } else {

        // Convert from Hex and print.

        // clear the whole line to blanks
        sprintf( szBuffer, "%04lx %04lx%70s", (long)( lLinePtr/0x10000L), (long)( lLinePtr&0xFFFF), NULLSTR );

        for ( i = 0; ( i < 16 ); i++ ) {

            if (( n = GetNextChar()) == EOF )
                break;

            if ( uListFlags & LIST_HIBIT )
                n &= 0x7F;

            sprintf( szScratch, "%02x", n );
            memmove( szBuffer+10+( i*3)+( i/8), szScratch, 2 );
            szBuffer[ i+61 ] = (char)(((( n >= TAB) && ( n <= CR )) || ( n == 0 ) || ( n == 27 )) ? '.' : n );
        }

        *nBytesPrinted = i;
        i = 78;
    }

    return i;
}


// MoveViewPtr - Update view pointer by scrolling the number of
//   lines specified in ptrRow.  If limited by the top or bottom
//   of the file, modify ptrRow to indicate the actual number of rows.
static LONG MoveViewPtr(LONG lFilePtr, LONG *ptrRow)
{
    int i, n;
    LONG lSaveOffset, lOffset = 0L, lRow, lRowCount = 0L;
    PCH lpTemp, lpSave, lpEnd;

    ListSetCurrent( lFilePtr );

    lRow = *ptrRow;

    if ((uListFlags & LIST_HEX) == 0 ) {

        SetRightMargin();
        if ( lRow > 0 ) {

        for ( ; ( lRowCount < lRow ); ) {

            lpSave = LFile.lpCurrent;
            lSaveOffset = LFile.lFileOffset;

            for ( n = 0; ; ) {

                if ((( i = GetNextChar()) == EOF ) || ( i == LFile.fEoL ))
                    break;

                if ( n >= nRightMargin ) {
                    GetPrevChar();
                    break;
                }

                // get tab expansion
                if ( i == TAB)
                    n = ((( n + TABSIZE ) / TABSIZE ) * TABSIZE );
                else
                    n++;
            }

            // check for buffer move
            if ( LFile.lFileOffset != lSaveOffset )
                lpSave -= LFile.uBufferSize;

            lOffset += (long)((long)LFile.lpCurrent - (long)lpSave);

            if ( i == EOF )
                break;

            lRowCount++;
        }

        } else {

        // move backwards
        for ( ; ( lRowCount > lRow); lRowCount--) {

            lSaveOffset = LFile.lFileOffset;

            // get previous line
            lpEnd = LFile.lpCurrent;
            n = GetPrevChar();
            while ((( i = GetPrevChar()) != EOF ) && ( i != LFile.fEoL ))
                ;

            // if not at start of file, move to beginning
            //   of the line
            if ( i == EOF ) {
                if ( lpEnd == LFile.lpCurrent)
                    break;
            } else
                GetNextChar();

            lpTemp = lpSave = LFile.lpCurrent;

            // check for buffer move
            if ( LFile.lFileOffset != lSaveOffset)
                lpEnd += LFile.uBufferSize;
            if ( lpEnd > LFile.lpBufferEnd)
                lpEnd = LFile.lpBufferEnd;

            // adjust for long or wrapped lines
            for ( n = 0; ( lpTemp < lpEnd ); lpTemp++ ) {

                if ( *lpTemp != (char)LFile.fEoL ) {

                // check if past right margin but less
                //   than current position
                if (( n >= nRightMargin ) && ( lpTemp + 1 < lpEnd)) {
                    n = 0;
                    lpSave = lpTemp;
                }

                // kludge for TAB offsets
                if ( *lpTemp == TAB )
                    n = ((( n + TABSIZE ) / TABSIZE ) * TABSIZE );
                else
                    n++;
                }
            }

            lOffset += (long)((long)lpSave - (long)lpEnd );
        }
        }

        *ptrRow = lRowCount;

    } else {

        lOffset = ( lRow * 16);
        *ptrRow = lRow;

        // Limit Checking
        if ((( lFilePtr == 0 ) && ( lRow < 0 )) || (( lFilePtr == LFile.lSize) && ( lRow > 0 ))){
        lOffset = 0L;
        *ptrRow = 0;
        } else if (( lFilePtr + lOffset) < 0 ) {
        lOffset = -lFilePtr;
        *ptrRow = (INT)( lOffset / 16);
        } else if (( lFilePtr + lOffset) > LFile.lSize) {
        lOffset = (LONG)(LFile.lSize - lFilePtr);
        *ptrRow = (INT)( lOffset / 16);
        }
    }

    return lOffset;
}


// SearchFile - Search the file for the value in szSearchStr.
//   If found, the file pointer is modified to the location in the file.
static BOOL SearchFile( long fFlags )
{
    extern int wild_brackets( char *, int, int );
    int c, i;
    INT   nOffset, fIgnoreCase = 0, fWildCards = 0, fWildStar;
    UINT  uLimit;
    LONG  lTemp, lLocalPtr;
    CHAR  szSearchStr[128], *pszSearchStr;

    lLocalPtr = (( fFlags & FFIND_TOPSEARCH ) ? 0L : LFile.lViewPtr );
    ListSetCurrent( lLocalPtr );

    pszSearchStr = (( fFlags & FFIND_NOERROR ) ? szFFindFindWhat : szListFindWhat);

    if ( fFlags & FFIND_HEX_SEARCH ) {

        // convert ASCII hex string to binary equivalent
        strupr( pszSearchStr );
        for ( i = 0; (( sscanf( pszSearchStr, "%x%n", &uLimit, &LFile.nSearchLen ) != 0 ) && ( LFile.nSearchLen > 0 )); i++ ) {
            sprintf( szSearchStr+i, FMT_CHAR, uLimit );
            pszSearchStr += LFile.nSearchLen;
        }

        if ( i == 0 )
            return 0;
        szSearchStr[i] = '\0';
        LFile.nSearchLen = i;

    } else {

        // check for a wildcard search
        // a leading ` means "ignore wildcard characters"
        if ( *pszSearchStr == '`' ) {
            pszSearchStr++;
            // strip (optional) trailing `
            if ((( i = strlen( pszSearchStr )) > 0 ) && ( pszSearchStr[--i] == '`' ))
                pszSearchStr[i] = '\0';
        } else if ((( fFlags & FFIND_NOWILDCARDS ) == 0 ) && ( strpbrk( pszSearchStr, WILD_CHARS ) != NULL ))
            fWildCards = 1;

        strcpy( szSearchStr, pszSearchStr );
        if (( LFile.nSearchLen = strlen( szSearchStr )) == 0 )
            return 0;

        if (( fFlags & FFIND_CHECK_CASE ) == 0 ) {
            strupr( szSearchStr );
            fIgnoreCase = 1;
        }
    }

    if ( fFlags & FFIND_REVERSE_SEARCH )
        strrev( szSearchStr );

    if ( fWildCards == 0 ) {

        // not a real elegant search algorithm, but real small!
        for ( i = 0; ; ) {

                if (( fFlags & FFIND_REVERSE_SEARCH ) == 0 ) {

            // don't call GetNextChar unless absolutely necessary!
            if ( LFile.lpCurrent == LFile.lpEOF )
                break;

            if ( LFile.lpCurrent < LFile.lpBufferEnd )
                c = *LFile.lpCurrent++;

            else {
                // abort search if ESC key is hit
                if (( _kbhit() != 0 ) && ( GetKeystroke( EDIT_NO_ECHO | EDIT_BIOS_KEY ) == ESC ))
                break;
                if (( c = GetNextChar()) == EOF )
                break;
                }

            } else if (( c = GetPrevChar()) == EOF )
            break;

            if (( c >= 'a' ) && ( fIgnoreCase )) {
            if ( c <= 'z' )
                c -= 32;
            else if ( c >= 0x80 )
                c = _ctoupper( c );
            }

            if ( c != (int)szSearchStr[i] ) {

            // rewind file pointer
            if ( i > 0 ) {
                if (( fFlags & FFIND_REVERSE_SEARCH ) == 0 )
                LFile.lpCurrent -= i;
                else
                LFile.lpCurrent += i;
                i = 0;
            }

            } else if ( ++i >= LFile.nSearchLen )
            break;
        }

    } else {

        // search with wildcards
RestartWildcardSearch:
        for ( i = 0, fWildStar = 0, nOffset = 0; ( szSearchStr[i] != '\0' ); ) {

        if ( szSearchStr[i] == '*' ) {

            // skip past * and advance file pointer until a match
            //   of the characters following the * is found.
            for ( fWildStar = 1; (( szSearchStr[++i] == '*' ) || ( szSearchStr[i] == '?' )); )
            ;

        } else {

            if (( fFlags & FFIND_REVERSE_SEARCH ) == 0 ) {

            // don't call GetNextChar unless absolutely necessary!
            if ( LFile.lpCurrent == LFile.lpEOF ) {
                i = 0;
                break;
            }

            if ( LFile.lpCurrent < LFile.lpBufferEnd )
                c = *LFile.lpCurrent++;

            else {

                // abort search if ESC key is hit
                if (( _kbhit() != 0 ) && ( GetKeystroke( EDIT_NO_ECHO | EDIT_BIOS_KEY) == ESC ))
                break;
                c = GetNextChar();
            }

            } else if (( c = GetPrevChar()) == EOF ) {
            i = 0;
            break;
            }

            nOffset++;

            if (( c >= 'a' ) && ( fIgnoreCase )) {
            if ( c <= 'z' )
                c -= 32;
            else if ( c >= 0x80 )
                c = _ctoupper( c );
            }

            // ? matches any single character
            if ( szSearchStr[i] == '?' )
            i++;

            else if (( szSearchStr[i] == '[' ) && ( fWildStar == 0 )) {

            // [ ] checks for a single character
            //   (including ranges)
            if ( wild_brackets( szSearchStr + i, c, fIgnoreCase ) != 0 )
                goto wild_rewind;

            while (( szSearchStr[i]) && ( szSearchStr[i++] != ']' ))
                ;

            } else {

            if ( fWildStar ) {

                int nSaveI, nSaveOffset;

                // following a '*'; so we need to do a complex
                //   match since there could be any number of
                //   preceding characters
                for ( nSaveI = i, nSaveOffset = nOffset; ; ) {

                    // don't search past the current line
                    if ((( c == CR ) || ( c == LF )) && (( fFlags & FFIND_HEX_SEARCH ) == 0 ))
                        goto RestartWildcardSearch;

                    if ( szSearchStr[i] == '[' ) {

                        // get the first matching char
                        if ( wild_brackets( szSearchStr+i, c, fIgnoreCase ) == 0 ) {
                            while (( szSearchStr[i] ) && ( szSearchStr[i++] != ']' ))
                                ;
                        }

                    } else if ( c == (int)szSearchStr[i] )
                        i++;

                    else {
                        // no match yet - keep scanning
                        i = nSaveI;
                        if ( --nOffset > nSaveOffset ) {
                            if (( fFlags & FFIND_REVERSE_SEARCH ) == 0 )
                            LFile.lpCurrent -= ( nOffset - nSaveOffset);
                            else
                            LFile.lpCurrent += ( nOffset - nSaveOffset);
                        }

                        nOffset = nSaveOffset;

                        if (( fFlags & FFIND_REVERSE_SEARCH ) == 0 )
                            nSaveOffset++;
                        else if ( nSaveOffset > 0 )
                            nSaveOffset--;
                        goto NextWildChar;
                    }

                    if (( szSearchStr[i] == '\0' ) || ( szSearchStr[i] == '*' ) || ( szSearchStr[i] == '?' ))
                        break;
NextWildChar:
                    if (( fFlags & FFIND_REVERSE_SEARCH ) == 0 ) {
                            // don't call GetNextChar unless
                        //   absolutely necessary!
                            if ( LFile.lpCurrent == LFile.lpEOF ) {
                        i = 0;
                        break;
                        }

                        if ( LFile.lpCurrent < LFile.lpBufferEnd )
                        c = *LFile.lpCurrent++;
                        else
                        c = GetNextChar();
                        if ( c == LFile.fEoL ) {
                        i = 0;
                        break;
                        }

                    } else if (( c = GetPrevChar()) == EOF ) {
                        i = 0;
                        break;
                    }

                    nOffset++;

                    if ( fIgnoreCase )
                        c = _ctoupper( c );
                }

                // if SearchStr is still an expression, we
                //   failed to find a match
                if ( szSearchStr[i] == '[' ) {
                    i = 0;
                    nOffset = 0;
                }

                fWildStar = 0;

            } else if ( c == (int)szSearchStr[i] )
                i++;

            else {
wild_rewind:
                if ( --nOffset > 0 ) {
                    if (( fFlags & FFIND_REVERSE_SEARCH ) == 0 )
                    LFile.lpCurrent -= nOffset;
                    else
                    LFile.lpCurrent += nOffset;
                }
                nOffset = 0;
                i = 0;
            }
            }
        }
        }

        // if at the end of the search string, we got a good match
        if (( szSearchStr[i] == '\0' ) && ( nOffset > 0 ))
        i = LFile.nSearchLen = nOffset;
        else
        i = 0;
    }

    if ( i == LFile.nSearchLen ) {

        // get current position
        LFile.lViewPtr = ( LFile.lFileOffset + (long)((long)LFile.lpCurrent - (long)LFile.lpBufferStart ));
        if (( fFlags & FFIND_REVERSE_SEARCH ) == 0 )
            LFile.lViewPtr -= LFile.nSearchLen;

        // hex search from FFIND?
        if (( fFlags & FFIND_NOERROR ) && ( fFlags & FFIND_HEX_DISPLAY ))
            return 1;

        // adjust line start & line count for hex display
        if ( uListFlags & LIST_HEX ) {

            LFile.lViewPtr -= ( LFile.lViewPtr % 16 );
            LFile.lCurrentLine = ( LFile.lViewPtr / 16 );

        } else {

            // get beginning of current line
            for ( i = 0; ((( c = GetPrevChar()) != EOF ) && ( c != LFile.fEoL )); i++ )
                ;
            if ( c != EOF )
                GetNextChar();

            // get line number (tricky because the screen may
            //   be wrapped)
            lTemp = ( LFile.lFileOffset + (long)((long)LFile.lpCurrent - (long)LFile.lpBufferStart));
            c = (int)ComputeLines( lTemp, LFile.lViewPtr );

            if ( fFlags & FFIND_TOPSEARCH )
                LFile.lCurrentLine = 0;
            LFile.lCurrentLine += ComputeLines( lLocalPtr, lTemp );

            // get beginning of line with search text
            for ( LFile.lViewPtr = lTemp; ( c > 0 ); c-- )
                ListMoveLine( 1 );
            ListSetCurrent( LFile.lViewPtr );
        }

        return 1;
    }

    // no matching string found
    if (( fFlags & FFIND_NOERROR ) == 0 ) {
        clear_header();
        WriteStrAtt( 0, 1, nInverse, LIST_NOT_FOUND );
        SetCurPos( 0, strlen( LIST_NOT_FOUND ) + 1 );
        honk();
        (void)GetKeystroke( EDIT_NO_ECHO | EDIT_BIOS_KEY );
    }

    return 0;
}


// get the previous buffer character
static int GetPrevChar( void )
{
    if ( LFile.lpCurrent <= LFile.lpBufferStart ) {     // at top of buffer?
        // can't "unwind" a pipe!
        if (( LFile.hHandle == STDIN ) || ( LFile.lFileOffset == 0L ))
            return EOF;

        // move the 1st half of the buffer to the 2nd half
        memmove( LFile.lpBufferStart+LFile.uBufferSize, LFile.lpBufferStart, LFile.uBufferSize );

        // read the previous block into the 1st half
        LFile.lFileOffset -= LFile.uBufferSize;

        ListFileRead( LFile.lpBufferStart, LFile.uBufferSize );
        LFile.lpCurrent += LFile.uBufferSize;
    }

    return *(--LFile.lpCurrent);
}


// get the next buffer character
static int GetNextChar( void )
{
    if ( LFile.lpCurrent == LFile.lpEOF )       // already at file end?
        return EOF;

    if ( LFile.lpCurrent >= LFile.lpBufferEnd ) {   // at end of buffer?

        // move the 2nd half of the buffer to the 1st half
        memmove( LFile.lpBufferStart, LFile.lpBufferStart+LFile.uBufferSize, LFile.uBufferSize );

        // read a new block into the 2nd half & adjust all the
        //   pointers back to the 1st half
        LFile.lFileOffset += (unsigned int)LFile.uTotalSize;
        ListFileRead( LFile.lpBufferStart+LFile.uBufferSize, LFile.uBufferSize );

        LFile.lFileOffset -= LFile.uBufferSize;
        LFile.lpCurrent -= LFile.uBufferSize;
    }

    return *LFile.lpCurrent++;
}

