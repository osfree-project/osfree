// LISTC.C - file listing routines for 4xxx family
//   Copyright (c) 1993 - 1997  Rex C. Conn  All rights reserved

int ListEntry(char *);
static int _list(char *);
static void ListUpdateScreen( void );
static void list_wait( char * );
static void clear_header( void );
static void wWriteListStr( int, int, POPWINDOWPTR, char * );

static char dirty_header;               // if != 0, redisplay header
static long fSearchFlags = 0;



// dummy routine for far entry to list_cmd
int ListEntry( char *pszFilename )
{
        char *argv[3];

        argv[1] = pszFilename;
        argv[2] = NULL;
        return ( list_cmd( 2, argv ));
}


// edit or display a file with vertical & horizontal scrolling & text searching
int list_cmd( int argc, char **argv )
{
        int fval, rval = 0;
        char szSource[MAXFILENAME], szFileName[MAXFILENAME], *arg;
        FILESEARCH dir;

        memset( &dir, '\0', sizeof(FILESEARCH) );

        // check for and remove switches
        if ( GetRange( argv[1], &(dir.aRanges), 0 ) != 0 )
                return ERROR_EXIT;

        // check for /T"search string"
        GetMultiCharSwitch( argv[1], "T", szSource, 255 );
        if ( szSource[0] == '"' )
                sscanf( szSource+1, "%79[^\"]", szListFindWhat );
        else if ( szSource[0] )
                sprintf( szListFindWhat, FMT_PREC_STR, 79, szSource );

        if ( GetSwitches( argv[1], "*HIRSWX", &uListFlags, 0 ) != 0 )
                return ( usage( LIST_USAGE ));

        if ( szSource[0] )
                uListFlags |= LIST_SEARCH;

        // check for pipe to LIST w/o explicit /S switch
        if ( first_arg( argv[1] ) == NULL ) {
                if ( QueryIsPipeHandle( STDIN ))
                        uListFlags |= LIST_STDIN;
                else if (( uListFlags & LIST_STDIN ) == 0 )
                        return ( usage( LIST_USAGE ));
        }

        // initialize buffers & globals
        if ( ListInit() )
                return ERROR_EXIT;
        nCurrent = nStart = 0;

        dir.hdir = INVALID_HANDLE_VALUE;

        // ^C handling
        if ( setjmp( cv.env ) == -1 ) {
list_abort:
                (void)DosFindClose( dir.hdir );
                clear_screen();
                rval = CTRLC;
                goto list_bye;
        }

RestartFileSearch:
        for ( argc = 0; ; argc++ ) {

                // break if at end of arg list, & not listing STDIN
                if (( arg = ntharg( argv[1], argc )) == NULL ) {
                        if (( uListFlags & LIST_STDIN ) == 0 )
                                break;
                } else
                        strcpy( szSource, arg );

                for ( fval = FIND_FIRST; ; ) {

                        szClip[0] = '\0';

                        // if not reading from STDIN, get the next matching file
                        if (( uListFlags & LIST_STDIN ) == 0 ) {

                                // qualify filename
                                mkfname( szSource, 0 );
                                if ( is_dir( szSource ))
                                        mkdirname( szSource, WILD_FILE );

                                if ( stricmp( szSource, CLIP ) == 0 ) {

                                    RedirToClip( szClip, 99 );
                                    if ( CopyFromClipboard( szClip ) != 0 )
                                        break;
                                    strcpy( szFileName, szClip );

                                } else if ( QueryIsPipeName( szSource )) {

                                    // only look for pipe once
                                    if ( fval == FIND_NEXT )
                                        break;
                                    copy_filename( szFileName, szSource );

                                } else if ( find_file( fval, szSource, (FIND_BYATTS | FIND_CREATE | FIND_DATERANGE | 0x07), &dir, szFileName ) == NULL ) {
                                    rval = (( fval == FIND_FIRST ) ? ERROR_EXIT : 0 );
                                    break;

                                } else if ( nStart < nCurrent ) {
                                    nStart++;
                                    fval = FIND_NEXT;
                                    continue;

                                } else if ( dir.size > 0L )
                                    LFile.lSize = dir.size;
                        }

                        // clear the screen
                        clear_screen();

                        if (( rval = _list( szFileName )) == CTRLC )
                                goto list_abort;

                        if ( szClip[0] )
                                remove( szClip );

                        if ( rval != 0 )
                                break;

                        SetCurPos( nScreenRows, 0 );

                        if (( szClip[0] ) || ( uListFlags & LIST_STDIN ))
                                break;

                        LFile.hHandle = _close( LFile.hHandle );

                        // increment index to current file
                        if ( nCurrent < nStart ) {
                                (void)DosFindClose( dir.hdir );
                                nStart = 0;
                                goto RestartFileSearch;
                        } else {
                                fval = FIND_NEXT;
                                nCurrent++;
                                nStart++;
                        }
                }

                // we can only read STDIN once!
                uListFlags &= ~LIST_STDIN;
        }

        crlf();
list_bye:
        FreeMem( (char *)LFile.lpBufferStart );
        if ( LFile.hHandle > 0 )
                _close( LFile.hHandle );

        // disable signal handling momentarily
        HoldSignals();

        return rval;
}


static int _list( char *pszFileName )
{
        int c, i;
        char szDescription[512], szHeader[132], szLine[10];
        long lTemp, lRow;
        POPWINDOWPTR wn = NULL;
        FILESEARCH dir;

        // get default normal and inverse attributes
        if ( gpIniptr->ListColor != 0 ) {
                Scroll( 0, 0, nScreenRows, nScreenColumns, 0, gpIniptr->ListColor );
        }

        // set colors
        GetAtt( (unsigned int *)&nNormal, (unsigned int *)&nInverse );
        if ( gpIniptr->ListStatusColor != 0 )
                nInverse = gpIniptr->ListStatusColor;

        // flip the first line to inverse video
        clear_header();

        // open the file & initialize buffers
        if ( ListOpenFile( pszFileName ))
                return ERROR_EXIT;

        // kludge for empty files or pipes
        if ( LFile.lSize == 0L )
                LFile.lSize = 1L;

        for ( ; ; ) {

                // display header
                if ( dirty_header ) {
                        clear_header();
                        sprintf( szHeader, LIST_HEADER, fname_part(LFile.szName), gchVerticalBar, gchVerticalBar, gchVerticalBar );
                        WriteStrAtt( 0, 0, nInverse, szHeader );
                        dirty_header = 0;
                }

                // display location within file
                //  (don't use color_printf() so we won't have
                //  problems with windowed sessions)
                i = sprintf( szHeader, LIST_LINE, LFile.nListHorizOffset, LFile.lCurrentLine + gpIniptr->ListRowStart, (int)((( LFile.lViewPtr + 1 ) * 100 ) / LFile.lSize ));
                WriteStrAtt( 0, ( nScreenColumns - i ), nInverse, szHeader );
                SetCurPos( 0, 0 );

                if ( uListFlags & LIST_SEARCH ) {

                        uListFlags &= ~LIST_SEARCH;
                        fSearchFlags = 0;
                        if ( uListFlags & LIST_REVERSE ) {

                                c = LIST_FIND_CHAR_REVERSE;
                                fSearchFlags |= FFIND_REVERSE_SEARCH;

                                // goto the last row
                                while ( ListMoveLine( 1 ) != 0 )
                                        ;
                        }

                        if ( uListFlags & LIST_NOWILDCARDS )
                                fSearchFlags |= FFIND_NOWILDCARDS;
                        bListSkipLine = 0;
                        goto FindNext;
                }

                // get the key from the BIOS, because
                //   STDIN might be redirected
                if ((( c = cvtkey( GetKeystroke( EDIT_NO_ECHO | EDIT_BIOS_KEY | EDIT_UC_SHIFT), MAP_GEN | MAP_LIST)) == ESC ))
                        break;

                switch ( c ) {
                case CTRLC:
                        return CTRLC;

                case CUR_LEFT:
                case CTL_LEFT:

                        if ((uListFlags & LIST_WRAP) || (LFile.nListHorizOffset == 0 ))
                                goto bad_key;

                        if ((LFile.nListHorizOffset -= ((c == CUR_LEFT) ? 8 : 40 )) < 0 )
                                LFile.nListHorizOffset = 0;
                        break;

                case CUR_RIGHT:
                case CTL_RIGHT:

                        if ((uListFlags & LIST_WRAP) || (LFile.nListHorizOffset == 512))
                                goto bad_key;

                        if ((LFile.nListHorizOffset += ((c == CUR_RIGHT) ? 8 : 40 )) > 512)
                                LFile.nListHorizOffset = 512;
                        break;

                case CUR_UP:

                        if ( ListMoveLine( -1 ) == 0 )
                                goto bad_key;

                        Scroll(1, 0, nScreenRows, nScreenColumns, -1, nNormal);
                        DisplayLine( 1, LFile.lViewPtr );
                        continue;

                case CUR_DOWN:

                        if (ListMoveLine(1) == 0 )
                                goto bad_key;

                        Scroll( 1, 0, nScreenRows, nScreenColumns, 1, nNormal );

                        // display last line
                        lTemp = LFile.lViewPtr;
                        lRow = (nScreenRows - 1);
                        lTemp += MoveViewPtr( lTemp,&lRow);
                        if ( lRow == (nScreenRows - 1))
                                DisplayLine( nScreenRows, lTemp );
                        continue;

                case HOME:

                        ListHome();
                        break;

                case END:

                        // goto the last row
                        list_wait( LIST_WAIT );
                        while (ListMoveLine( 1 ) != 0 )
                                ;
                case PgUp:

                        // already at TOF?
                        if ( LFile.lViewPtr == 0L )
                                goto bad_key;

                        for (i = 1; ((i < nScreenRows) && (ListMoveLine(-1) != 0 )); i++)
                                ;

                        break;

                case PgDn:
                case SPACE:

                        if ( ListMoveLine( nScreenRows - 1 ) == 0 )
                                goto bad_key;

                        break;

                case F1:        // help
                        _help( "LIST", NULL );
                        continue;

                case LIST_INFO_CHAR:
                    {
                        unsigned int size = 1024;
                        PCH fptr;
                        int fFSType;
                        char szBuf[10];

                        // disable ^C / ^BREAK handling
                        HoldSignals();

                        if (( uListFlags & LIST_STDIN) == 0 ) {
                            if ( find_file( FIND_FIRST, LFile.szName, 0x2007, &dir, NULL ) == NULL ) {
                                honk();
                                continue;
                            }
                        }

                        // display info on the current file
                        i = ((uListFlags & LIST_STDIN) ? 5 : 10 );
                        if ((fFSType = ifs_type(LFile.szName)) != FAT)
                                i = 11;
                        wn = wOpen( 2, 3, i, 75, nInverse, gpInternalName, NULL );

                        wn->attrib = nNormal;
                        wClear();
                        i = 0;

                        if ( uListFlags & LIST_STDIN )
                            wWriteStrAtt( 0, 1, nNormal, LIST_INFO_PIPE );

                        else {

                            szDescription[0] = '\0';
                            process_descriptions( LFile.szName, szDescription, DESCRIPTION_READ );

                            fptr = AllocMem(&size);

                            strcpy(szLine, FormatDate(dir.fdLW.fdateLastWrite.month,dir.fdLW.fdateLastWrite.day,dir.fdLW.fdateLastWrite.year+80 ));
                            strcpy(szBuf, FormatDate(dir.fdLA.fdateLastAccess.month,dir.fdLA.fdateLastAccess.day,dir.fdLA.fdateLastAccess.year+80 ));
                            sprintf(fptr, ((fFSType != FAT) ? LIST_INFO_HPFS : LIST_INFO_FAT),
                                LFile.szName, szDescription, dir.size,
                                szLine, dir.ftLW.ftimeLastWrite.hours,gaCountryInfo.szTimeSeparator[0],dir.ftLW.ftimeLastWrite.minutes,
                                szBuf, dir.ftLA.ftimeLastAccess.hours,gaCountryInfo.szTimeSeparator[0],dir.ftLA.ftimeLastAccess.minutes,
                                FormatDate(dir.fdC.fdateCreation.month,dir.fdC.fdateCreation.day,dir.fdC.fdateCreation.year+80 ),
                                dir.ftC.ftimeCreation.hours,gaCountryInfo.szTimeSeparator[0],dir.ftC.ftimeCreation.minutes);

                            // print the text
                            for ( ; ( *fptr != '\0' ); i++ ) {
                                sscanf( fptr, "%[^\n]%*c%n", szHeader, &size );
                                wWriteStrAtt( i, 1, nNormal, szHeader );
                                fptr += size;
                            }

                            FreeMem( fptr );
                        }

                        wWriteListStr(i+1,1,wn,PAUSE_PROMPT);
                        GetKeystroke(EDIT_NO_ECHO | EDIT_BIOS_KEY);

                        wRemove(wn);

                        // enable ^C / ^BREAK handling
                        EnableSignals();

                        continue;
                    }

                case LIST_GOTO_CHAR:

                        // goto the specified line / hex offset

                        // disable ^C / ^BREAK handling
                        HoldSignals();

                        wn = wOpen( 2, 5, 4, strlen( LIST_GOTO_OFFSET ) + 20, nInverse, LIST_GOTO_TITLE, NULL );
                        wn->attrib = nNormal;
                        wClear();

                        wWriteListStr( 0, 1, wn, (( uListFlags & LIST_HEX) ? LIST_GOTO_OFFSET : LIST_GOTO));
                        i = egets( szLine, 10, (EDIT_DIALOG | EDIT_BIOS_KEY | EDIT_NO_CRLF));
                        wRemove( wn );

                        // enable ^C / ^BREAK handling
                        EnableSignals();

                        if ( i == 0 )
                                break;
                        list_wait( LIST_WAIT );

                        // if in hex mode, jump to offset
                        if ( uListFlags & LIST_HEX ) {
                                strupr( szLine );
                                sscanf( szLine, "%lx", &lRow );
                                lRow = lRow / 0x10;
                        } else if ( sscanf( szLine, FMT_LONG, &lRow ) == 0 )
                                continue;

                        lRow -= gpIniptr->ListRowStart;
                        if ( lRow >= 0 ) {
                                LFile.lViewPtr = MoveViewPtr( 0L, &lRow );
                                LFile.lCurrentLine = lRow;
                        } else {
                                LFile.lViewPtr += MoveViewPtr( LFile.lViewPtr, &lRow );
                                LFile.lCurrentLine += lRow;
                        }
                        break;

                case LIST_HIBIT_CHAR:

                        // toggle high bit filter
                        uListFlags ^= LIST_HIBIT;
                        break;

                case LIST_WRAP_CHAR:

                        // toggle line wrap
                        uListFlags ^= LIST_WRAP;
                        nRightMargin = (( uListFlags & LIST_WRAP ) ? GetScrCols() : 512);

                        // recalculate current line
                        list_wait( LIST_WAIT );

                        // get start of line
                        LFile.nListHorizOffset = 0;

                        // line number probably changed, so recompute everything
                        LFile.lCurrentLine = ComputeLines( 0L, LFile.lViewPtr );
                        LFile.lViewPtr = MoveViewPtr( 0L, &(LFile.lCurrentLine ));
                        break;

                case LIST_HEX_CHAR:

                        // toggle hex display
                        uListFlags ^= LIST_HEX;

                        // if hex, reset to previous 16-byte
                        //   boundary
                        if ( uListFlags & LIST_HEX )
                                LFile.lViewPtr -= ( LFile.lViewPtr % 16 );
                        else {
                                // if not hex, reset to start of line
                                ListMoveLine( 1 );
                                ListMoveLine( -1 );
                        }

                        // recalculate current line
                        list_wait( LIST_WAIT );
                        LFile.lCurrentLine = ComputeLines( 0L, LFile.lViewPtr );
                        break;

                case LIST_FIND_CHAR:
                case LIST_FIND_CHAR_REVERSE:
                        // find first matching string
                        bListSkipLine = 0;

                case LIST_FIND_NEXT_CHAR:
                case LIST_FIND_NEXT_CHAR_REVERSE:
                        // find next matching string

                        if (( c == LIST_FIND_CHAR ) || ( c == LIST_FIND_CHAR_REVERSE )) {

                                // disable ^C / ^BREAK handling
                                HoldSignals();

                                fSearchFlags = 0;
                                wn = wOpen( 2, 1, 4, 75, nInverse, (( c == LIST_FIND_NEXT_CHAR_REVERSE ) ? LIST_FIND_TITLE_REVERSE : LIST_FIND_TITLE ), NULL );
                                wn->attrib = nNormal;
                                wClear();

                                if ( uListFlags & LIST_HEX ) {
                                        wWriteListStr( 0, 1, wn, LIST_FIND_HEX );
                                        if ( GetKeystroke( EDIT_ECHO | EDIT_BIOS_KEY | EDIT_UC_SHIFT ) == YES_CHAR )
                                                fSearchFlags |= FFIND_HEX_SEARCH;
                                        wClear();
                                }

                                wWriteListStr( 0, 1, wn, LIST_FIND );
                                egets( szListFindWhat, 64, (EDIT_DIALOG | EDIT_BIOS_KEY | EDIT_NO_CRLF));
                                wRemove( wn );

                                // enable ^C / ^BREAK handling
                                EnableSignals();

                        } else {
FindNext:
                                // a "Next" has to be from current position
                                fSearchFlags &= ~FFIND_TOPSEARCH;
                        }

                        if ( szListFindWhat[0] == '\0' )
                                continue;

                        sprintf( szDescription, LIST_FIND_WAIT, szListFindWhat );
                        list_wait( szDescription );

                        // save start position
                        lTemp = LFile.lViewPtr;
                        lRow = LFile.lCurrentLine;

                        if (( c == LIST_FIND_CHAR_REVERSE ) || ( c == LIST_FIND_NEXT_CHAR_REVERSE )) {
                                // start on the previous line
                                fSearchFlags |= FFIND_REVERSE_SEARCH;
                        } else {
                                fSearchFlags &= ~FFIND_REVERSE_SEARCH;
                                // skip the first line (except on /T"xxx")
                                if ( bListSkipLine )
                                        ListMoveLine( 1 );
                        }

                        bListSkipLine = 1;
                        if ( SearchFile( fSearchFlags ) != 1 ) {
                                ListSetCurrent( lTemp );
                                LFile.lViewPtr = lTemp;
                                LFile.lCurrentLine = lRow;
                        } else
                                LFile.fDisplaySearch = (( fSearchFlags & FFIND_CHECK_CASE ) ? 2 : 1 );

                        break;

                case LIST_PRINT_CHAR:

                        // print the file
                        ListPrintFile();
                        continue;

                case LIST_CONTINUE_CHAR:
                case CTL_PgDn:
                        return 0;

                case LIST_PREVIOUS_CHAR:
                case CTL_PgUp:
                        // previous file
                        if ( nCurrent > 0 ) {
                                nCurrent--;
                                return 0;
                        }

                default:
bad_key:
                        honk();
                        continue;
                }

                // rewrite the display
                ListUpdateScreen();
        }

        return 0;
}


// display a "WAIT" message
static void list_wait( char *pszPrompt )
{
        // clear the header line
        clear_header();

        // display "WAIT" in blinking chars
        WriteStrAtt( 0, (( nScreenColumns / 2 ) - ( strlen( pszPrompt ) / 2 )), ( nInverse | 0x80 ), pszPrompt );
}


// clear the LIST header line to inverse blanks
static void clear_header(void)
{
        dirty_header = 1;
        Scroll( 0, 0, 0, nScreenColumns, 0, nInverse );
}


// redraw the screen (except for the status line)
static void ListUpdateScreen( void )
{
        int nRow;
        long lTemp = LFile.lViewPtr;

        Scroll( 1, 0, nScreenRows, nScreenColumns, 0, nNormal );

        // if no name yet, we're still initializing!
        if ( LFile.szName[0] == '\0' )
                return;

        for ( nRow = 1; ( nRow <= nScreenRows ); nRow++ )
                lTemp += DisplayLine( nRow, lTemp );
        LFile.fDisplaySearch = 0;
}


// print the file on the default printer
void ListPrintFile( void )
{
        int i, n;
        int c, nRows, nBytesPrinted;
        long lTemp;
        char szBuffer[512];
        POPWINDOWPTR wn;

        // disable ^C / ^BREAK handling
        HoldSignals();

        wn = wOpen( 2, 1, 4, strlen( LIST_PRINTING ) + 8, nInverse, LIST_PRINT_TITLE, NULL );
        wn->attrib = nNormal;
        wClear();
        sprintf( szBuffer, LIST_QUERY_PRINT, LIST_PRINT_FILE_CHAR, LIST_PRINT_PAGE_CHAR );
        wWriteListStr( 0, 1, wn, szBuffer );

        if ((( c = GetKeystroke( EDIT_ECHO | EDIT_UC_SHIFT | EDIT_BIOS_KEY )) == LIST_PRINT_FILE_CHAR ) || ( c == LIST_PRINT_PAGE_CHAR )) {

            // save start position
            lTemp = LFile.lViewPtr;

            // display "Printing ..."
            wWriteListStr( 0, 1, wn, LIST_PRINTING );

            if (( gnGFH = _sopen((( gpIniptr->Printer != INI_EMPTYSTR ) ? gpIniptr->StrData + gpIniptr->Printer : "LPT1" ), (O_TEXT | O_WRONLY | O_CREAT), SH_DENYNO, S_IWRITE | S_IREAD )) >= 0 ) {

                // reset to beginning of file
                if ( c == LIST_PRINT_FILE_CHAR )
                        ListSetCurrent( 0L );
                else {
                        nRows = GetScrRows();
                        ListSetCurrent( LFile.lViewPtr );
                }

                // print the header (filename, date & time)
                qprintf( gnGFH, "%s   %s  %s\n\n", LFile.szName, gdate( 0 ), gtime( gaCountryInfo.fsTimeFmt ) );

                do {
                        // abort printing if a key is hit
                        if (( _kbhit() != 0 ) && ( GetKeystroke( EDIT_NO_ECHO | EDIT_BIOS_KEY ) == ESC ))
                                break;
                        i = FormatLine( szBuffer, LFile.lViewPtr, &nBytesPrinted, TRUE );
                        LFile.lViewPtr += 16;

                        // replace 0-31 with "."
                        if ( uListFlags & LIST_HEX ) {
                            for ( n = 0; ( n < i ); n++ ) {
                                if ( szBuffer[n] < 32 )
                                    szBuffer[n] = '.';
                            }
                        }

                        if (( c == LIST_PRINT_PAGE_CHAR ) && ( nRows-- <= 0 ))
                                break;

                } while (( nBytesPrinted > 0 ) && ( qprintf( gnGFH, "%.*s\n", i, szBuffer ) > 0 ));

                // print a formfeed
                qputc( gnGFH, '\014' );
                gnGFH = _close( gnGFH );

                // restore start position
                LFile.lViewPtr = lTemp;
                ListSetCurrent( LFile.lViewPtr );

            } else
                honk();
        }

        wRemove( wn );

        // enable ^C / ^BREAK handling
        EnableSignals();
}


// write a string to a popup window & position cursor at end
static void wWriteListStr( int nRow, int col, POPWINDOWPTR wn, char *str )
{
        wWriteStrAtt( nRow, col, wn->attrib, str );
        wSetCurPos( nRow, col + strlen( str ));
}

