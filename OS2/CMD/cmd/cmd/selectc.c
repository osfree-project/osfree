// SELECTC.C - character-mode SELECT
//   Copyright (c) 1993 - 1997 Rex C. Conn

static void ShowSelectHeader(SELECT_ARGS *);
static int getmark(SELECT_ARGS *sel);
static void dirprint(SELECT_ARGS *, int, unsigned int, unsigned int);


#if _DOS && (_WIN == 0)
#pragma alloc_text( _TEXT, select_cmd )
#endif

// select the files for a command
int select_cmd( int argc, char **argv )
{
    char *arg, *selectline;
    int rval = 0, mode = 0x10;
    SELECT_ARGS sel;

    selectline = *argv;

    // check for SELECT options
    if (( arg = argv[1]) == NULL )
        return (usage(SELECT_USAGE));

    if (( rval = InitSelect( &sel, arg, &mode )) != 0 )
        return rval;

    if (( *arg == '\0' ) || (( sel.pFileVar = strpbrk( arg, "([" )) == NULL ) || (( sel.cmd_tail = scan( sel.pFileVar, (( *sel.pFileVar == '(' ) ? ")" : "]"), "`[" )) == BADQUOTES ) || ( sel.cmd_tail[0] == '\0' ))
        return (usage(SELECT_USAGE));

    sel.open_paren = *sel.pFileVar;
    sel.close_paren = sel.cmd_tail[0];

    // rub out the '(' & ')' (or [])
    *sel.pFileVar++ = '\0';
    *sel.cmd_tail++ = '\0';

    // save the command on the stack
    sel.cmd_start = _alloca( strlen( arg ) + 1 );
    strcpy( sel.cmd_start, arg );
    arg = _alloca( strlen( sel.cmd_tail ) + 1 );
    sel.cmd_tail = strcpy( arg, sel.cmd_tail );

    // do any variable expansion on the file spec
    if ( var_expand( strcpy( selectline, sel.pFileVar ), 1 ) != 0 )
        return ERROR_EXIT;

    sel.pFileVar = _alloca( strlen( selectline ) + 1 );
    strcpy( sel.pFileVar, selectline );

    for ( argc = 0; (( sel.arg = ntharg( sel.pFileVar, argc )) != NULL) && (rval != CTRLC); argc++) {

        sel.sfiles = 0L;        // initialize pointer to file array
        sel.entries = 0;

        if ( setjmp( cv.env ) == -1 ) {
        dir_free( sel.sfiles );
        clear_screen();
        rval = CTRLC;
        goto select_bye;
        }

        EnableSignals();
        copy_filename( selectline, sel.arg );

        // force \*.* onto a directory name
        if (is_dir( selectline ))
        mkdirname( selectline, WILD_FILE );

        // save path part of name
        if ((arg = path_part( selectline)) == NULL)
        arg = NULLSTR;
        strcpy( sel.szPath, arg );

        if (mkfname( selectline, 0 ) == NULL) {
        rval = ERROR_EXIT;
        goto select_bye;
        }

        // check to see if it's an HPFS partition
        if ( ifs_type( selectline) != 0 )
        glDirFlags |= DIRFLAGS_HPFS;

        // search the directory for the specified file(s)
        if ( SearchDirectory( (mode | FIND_DATERANGE), selectline, (DIR_ENTRY **)&( sel.sfiles), &(sel.entries), &(sel.aSelRanges), 5 ) != 0 ) {
        rval = ERROR_EXIT;
        goto select_bye;
        }

        if ( sel.entries > 0 ) {

        // clear the screen
        clear_screen();

        // get default normal and inverse attributes
        if ( gpIniptr->SelectColor != 0 )
            Scroll( 0, 0, GetScrRows(), GetScrCols()-1, 0, gpIniptr->SelectColor );

        // get default screen colors
        GetAtt( &(sel.normal), &(sel.inverse));
        if ( gpIniptr->SelectStatusColor != 0 )
            sel.inverse = gpIniptr->SelectStatusColor;

        // display SELECT header
        ShowSelectHeader( &sel );

        // get filename colors
        ColorizeDirectory( sel.sfiles, sel.entries, 0 );

        sel.dirline = sel.dirfirst = 0;     // beginning of page

        // get the selection list
        if ( getmark( &sel ) == -1 )
            sel.marked = 0;
        clear_screen();

        if ( sel.length >= MAXLINESIZ) {
            rval = error( ERROR_4DOS_COMMAND_TOO_LONG, NULL );
            sel.marked = 0;
        }

        // process the marked files in order
        rval = ProcessSelect( &sel, selectline );
        }

        dir_free( sel.sfiles);
    }

select_bye:
    EnableSignals();
    return rval;
}


static void ShowSelectHeader(SELECT_ARGS *sel)
{
    // display SELECT header
    Scroll( 0, 0, 0, GetScrCols()-1, 0, sel->inverse );
    SetCurPos( 0, 0 );
    color_printf( sel->inverse, SELECT_HEADER, gchVerticalBar, gchVerticalBar, gchVerticalBar, gchVerticalBar );

    // display the command & the file spec(s)
    Scroll( 1, 0, 1, GetScrCols()-1, 0, sel->normal );
    SetCurPos( 1, 0 );
    color_printf( sel->normal, "%s%c%s%c%s", sel->cmd_start, sel->open_paren, sel->pFileVar, sel->close_paren, sel->cmd_tail );
}


// SELECT the files to mark
static int getmark(SELECT_ARGS *sel)
{
    extern int ListEntry(char *);
    unsigned int c, i, j, n;
    int nSearch = 0;
    char szFilename[MAXFILENAME+1];
    char szSearch[32];
    jmp_buf saved_env;

    // display first page
    dirprint( sel, 1, sel->dirline, sel->inverse );

    szSearch[0] = '\0';
    for ( sel->marked = 0; ; ) {

        c = cvtkey( GetKeystroke( EDIT_NO_ECHO | EDIT_UC_SHIFT ), MAP_GEN );
        switch ( c ) {
        case ESC:
            return -1;

        case CR:
        case LF:
            // allow a single selection using the RETURN key
            if ( sel->marked == 0 )
                sel->sfiles[sel->dirline].file_mark = ++(sel->marked);
            return sel->marked;

        case SELECT_LIST:
            // if HPFS names with whitespace, quote them
            if ( glDirFlags & DIRFLAGS_HPFS ) {
                sprintf( szFilename, "%s%Fs", sel->szPath, sel->sfiles[sel->dirline].hpfs_name );
                AddQuotes( szFilename );
            } else
                sprintf( szFilename, "%s%Fs", sel->szPath, sel->sfiles[sel->dirline].file_name );

            // save the old "env" (^C destination)
            memmove( saved_env, cv.env, sizeof(saved_env) );
            ListEntry( szFilename );

            // restore the old "env"
            memmove( cv.env, saved_env, sizeof(saved_env) );

            // redraw SELECT screen
            ShowSelectHeader( sel );
            goto redraw_page;

        case '+':       // mark a file
        case SPACE:

            if ( sel->sfiles[ sel->dirline ].file_mark == 0 )
                sel->sfiles[sel->dirline].file_mark = ++(sel->marked);
            else

        case '-':       // unmark a file
            sel->sfiles[sel->dirline].file_mark = 0;

        case CUR_UP:
        case CUR_DOWN:

            // switch current line to normal
            dirprint( sel, -1, sel->dirline, sel->normal );
            if ( c != CUR_UP ) {

                // move scroll bar down, beep if at end
                if ( sel->dirline >= ((int)sel->entries - 1)) {
                    if (c == CUR_DOWN)
                        honk();
                } else if (++(sel->dirline) > sel->dirlast) {
                    (sel->dirfirst)++;
                    Scroll( 2, 0, GetScrRows(), GetScrCols()-1, 1, sel->normal );
                }

            } else {

                // move scroll bar up
                if ( sel->dirline == 0 )
                    honk();     // already at top
                else if (--(sel->dirline) < sel->dirfirst) {
                    (sel->dirfirst)--;
                    Scroll( 2, 0, GetScrRows(), GetScrCols()-1, -1, sel->normal );
                }
            }

            // new line in inverse video
            dirprint( sel, -1, sel->dirline, sel->inverse );
            break;

        case HOME:
            sel->nLeftMargin = sel->dirfirst = sel->dirline = 0;

        case END:
        case PgUp:
        case PgDn:

            i = GetScrRows() - 1;

            // change the directory page
            if (c == END) {

                sel->dirline = sel->entries - 1;
                if (( sel->dirfirst = (( sel->dirline + 1) - i)) < 0 )
                    sel->dirfirst = 0;

            } else if (c == PgDn) {

                if ((int)( sel->dirline += i) >= (int)( sel->entries))
                    sel->dirline = sel->entries - 1;
                if ((int)( sel->dirfirst + i) < (int)(sel->entries))
                    sel->dirfirst += i;

            } else if (c == PgUp) {

                if (( sel->dirfirst -= i) < 0 )
                    sel->dirfirst = 0;
                if (( sel->dirline -= i) < 0 )
                    sel->dirline = 0;
            }

        case '*':       // reverse all current marks (no dirs)
        case '/':       // unmark everything

            for (i = 0; ((c < 0x80 ) && (i < sel->entries)); i++) {

                if (c == '*') {
                    if ( sel->sfiles[i].file_mark == 0 ) {
                        if (( sel->sfiles[i].attribute & _A_SUBDIR) == 0 )
                            sel->sfiles[i].file_mark = ++sel->marked;
                    } else
                        sel->sfiles[i].file_mark = 0;
                } else if (c == '/')
                    sel->sfiles[i].file_mark = sel->marked = 0;
            }

        case CUR_LEFT:
        case CUR_RIGHT:

            if ( c == CUR_LEFT ) {
                if (( sel->nLeftMargin -= 8 ) < 0 )
                    sel->nLeftMargin = 0;
            } else if ( c == CUR_RIGHT ) {
                if ( sel->nLeftMargin < 512 )
                    sel->nLeftMargin += 8;
            }

            // draw a new page
redraw_page:
            dirprint( sel,1,sel->dirline,sel->normal);
            dirprint( sel,-1,sel->dirline,sel->inverse);
            break;

        case F1:    // help
            _help( gpInternalName, NULL );
            break;

        default:
            // try to match a string

            if ( c <= 0xFF ) {

            // search for entry beginning with szSearch
ResetSearch:
            if ( nSearch >= 32 )
                nSearch = 0;
            szSearch[ nSearch++ ] = c;
            szSearch[ nSearch ] = '\0';
            for ( j = 0, i = sel->dirline; ( i < sel->entries ); i++ ) {
StartLoop:

                if ((( glDirFlags & DIRFLAGS_HPFS ) == 0 ) || ( glDirFlags & DIRFLAGS_HPFS_TO_FAT ))
                n = strnicmp( szSearch, sel->sfiles[i].file_name, strlen( szSearch ));
                else
                n = strnicmp( szSearch, sel->sfiles[i].hpfs_name, strlen( szSearch ));

                if ( n == 0 ) {
                // switch current line to normal
                dirprint( sel, -1, sel->dirline, sel->normal );
                sel->dirline = i;

                // draw page
                if (( sel->dirline < sel->dirfirst ) || ( sel->dirline > sel->dirlast )) {
                    sel->dirfirst = sel->dirline;
                    dirprint( sel, 1, sel->dirline, sel->normal );
                    dirprint( sel, -1, sel->dirline, sel->inverse );
                } else
                    dirprint( sel, -1, sel->dirline, sel->inverse );
                goto NoHonk;
                }

                if ( i == ( sel->entries - 1 )) {
                if ( j == 0 ) {
                    // try again from the beginning
                    j = 1;
                    i = 0;
                    goto StartLoop;
                } else if ( j == 1 ) {
                    if ( nSearch == 1 ) {
                        nSearch = 0;
                    break;
                    }
                    nSearch = 0;
                    goto ResetSearch;
                }
                }
            }
            }

            // invalid input
            honk();
NoHonk:
            break;
        }
    }
}


// print directory for SELECT
//  page = directory page
//  line = line to print
//  attr = attribute to use for printing
static void dirprint(SELECT_ARGS *sel, int page, unsigned int line, unsigned int attr)
{
    int i;
    int n, columns, rows;
    unsigned int current_row, save_row, last, color, nOffset;
    char szBuffer[256];
    PCH fptr;

    sel->uMarkedFiles = 0;

    current_row = save_row = 2;     // set start row
    rows = GetScrRows();
    last = rows - 2;

    columns = GetScrCols();

    if ( page > 0 ) {           // print entire page
        // clear screen
        Scroll(2,0,rows,columns-1,0,sel->normal);
        line = sel->dirfirst;
    } else              // just print 1 line
        current_row += (line - sel->dirfirst);

    if (( sel->dirlast = ( sel->dirfirst + last)) >= (int)sel->entries)
        sel->dirlast = sel->entries - 1;

    // print page number
    i = (( sel->dirline + 1) / (last + 1) );
    if (( sel->dirline + 1) % (last + 1) )
        i++;

    n = ( sel->entries / rows);
    if ( sel->entries % rows)
        n++;

    sprintf( szBuffer, SELECT_PAGE_COUNT, i, n );
    WriteStrAtt( 0, columns - (strlen(SELECT_PAGE_COUNT) - 1), sel->inverse, szBuffer );

    // initialize the SELECT line length
    sel->length = strlen( sel->cmd_start) + strlen( sel->cmd_tail);

    if ( sel->open_paren == '(')
        sel->length += strlen( sel->szPath) + 13;

    for ( last = 0, sel->lTotalSize = 0L; ( last < sel->entries ); last++ ) {

        if ( sel->sfiles[last].file_mark > 0 ) {

            sel->uMarkedFiles++;
            if ( sel->open_paren == '[') {

                // HPFS partitions have filename in "hpfs_name"
                fptr = ((glDirFlags & DIRFLAGS_HPFS) ? sel->sfiles[last].hpfs_name : sel->sfiles[last].file_name);
                i = strlen(fptr);
                sel->length += strlen( sel->szPath) + i + 1;
            }

            sel->lTotalSize += (LONG)(( sel->sfiles[last].file_size + 1023L ) / 1024L );
        }
    }

    // display the line length
    sprintf( szBuffer, "%4u", sel->length );
    WriteStrAtt( 0, 0, sel->inverse, szBuffer );

    // display the number of files marked & the size (in Kb)
    sprintf( szBuffer, MARKED_FILES, sel->uMarkedFiles, sel->lTotalSize );
    WriteStrAtt( 1, columns-(strlen(MARKED_FILES)+3), sel->normal, szBuffer );

    // last row to print
    last = ((page > 0 ) ? sel->dirlast : line);

    for (nOffset = 0; (line <= last); line++, current_row++) {

        // check for colorization
        // get background color if none specified
        if (( sel->sfiles[line].color & 0x70 ) == 0 )
            sel->sfiles[line].color |= ( sel->normal & 0x70 );

        color = (((attr == sel->normal) && ( sel->sfiles[line].color != -1)) ? sel->sfiles[line].color : attr );

        if (((glDirFlags & DIRFLAGS_HPFS) == 0 ) || ( glDirFlags & DIRFLAGS_HPFS_TO_FAT )) {

            // display the FAT filename
            nOffset = sprintf( szBuffer, "%c%-12Fs", (( sel->sfiles[line].file_mark > 0 ) ? gchRightArrow : ' '), sel->sfiles[line].file_name );

        } else {
            // display the HPFS filename (max of first 45 chars
            //   displayable on an 80-columns display)
            nOffset = sprintf( szBuffer, "%c%-*.*Fs", (( sel->sfiles[line].file_mark > 0 ) ? gchRightArrow : ' '), (columns-35), (columns-35), sel->sfiles[line].hpfs_name );
        }

        if (((glDirFlags & DIRFLAGS_HPFS) == 0 ) || (glDirFlags & DIRFLAGS_HPFS_TO_FAT)) {

            nOffset += sprintf( szBuffer+nOffset, ( sel->sfiles[line].attribute & _A_SUBDIR) ? DIR_LABEL : "%9lu",sel->sfiles[line].file_size );
            nOffset += sprintf( szBuffer+nOffset, ((gaCountryInfo.fsTimeFmt == 0 ) ? "  %s %2u%c%02u%c" : "  %s  %2u%c%02u"),
                FormatDate( sel->sfiles[line].fd.file_date.months, sel->sfiles[line].fd.file_date.days,sel->sfiles[line].fd.file_date.years+80 ),
                sel->sfiles[line].ft.file_time.hours, gaCountryInfo.szTimeSeparator[0],sel->sfiles[line].ft.file_time.minutes, sel->sfiles[line].ampm);
            {
                n = strlen( sel->sfiles[line].file_id);
                i = ((n > sel->nLeftMargin) ? sel->nLeftMargin : n);
                if ((n - i) > (columns - 40 ))
                sprintf( szBuffer+nOffset," %.*Fs%c",(columns-41), sel->sfiles[line].file_id+i, gchRightArrow );
                else
                sprintf( szBuffer+nOffset," %Fs", sel->sfiles[line].file_id+i );
            }

            WriteStrAtt( current_row, 0, color, szBuffer );
            SetLineColor( current_row, 39, (columns - 39), color );

        } else {
            nOffset += sprintf( szBuffer+nOffset, ( sel->sfiles[line].attribute & _A_SUBDIR) ? HPFS_DIR_LABEL : "%15Lu", sel->sfiles[line].file_size );

            sprintf( szBuffer+nOffset, "  %s  %2u%c%02u%c", FormatDate( sel->sfiles[line].fd.file_date.months, sel->sfiles[line].fd.file_date.days, sel->sfiles[line].fd.file_date.years+80 ),
                sel->sfiles[line].ft.file_time.hours, gaCountryInfo.szTimeSeparator[0], sel->sfiles[line].ft.file_time.minutes, sel->sfiles[line].ampm );
            WriteStrAtt( current_row, 0, color, szBuffer );
        }

        if (attr == sel->inverse)
            save_row = current_row;

        attr = sel->normal;
    }

    // move the cursor to the currently selected row (this is
    //   for blind users who need the cursor to move)
    SetCurPos( save_row, 0 );
}

