// SELECT.C - SELECT commands for 4xxx / TCMD family
//   Copyright (c) 1993 - 1997 Rex C. Conn  All rights reserved

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <io.h>

#include "4all.h"


// SELECT arguments
typedef struct {
        unsigned int marked;
        int dirline;
        int dirfirst;
        int dirlast;
        unsigned int normal;
        unsigned int inverse;
        unsigned int entries;
        unsigned int length;
        unsigned int uMarkedFiles;
        int nLeftMargin;
        long lTotalSize;
        char *arg;
        char *cmd_start;
        char *cmd_tail;
        char *pFileVar;
        char open_paren;
        char close_paren;
        char szPath[MAXPATH+1];
        DIR_ENTRY *sfiles;
        RANGES aSelRanges;
} SELECT_ARGS;

static int InitSelect(SELECT_ARGS *, char *, int *);
static int ProcessSelect(SELECT_ARGS *, char *);


#include "selectc.c"            // SELECT for character mode


// check for SELECT options
static int InitSelect( SELECT_ARGS *sel, char *start_line, int *mode )
{
        int i;
        char *arg;

        // clear everything to 0
        memset( (char *)sel, '\0', sizeof(SELECT_ARGS) );

        // reset the DIR flags
        init_dir();

        // initialize date/time/size ranges
        if ( GetRange( start_line, &(sel->aSelRanges), 1 ) != 0 )
                return ERROR_EXIT;

        for ( i = 0; (( arg = ntharg( start_line, i )) != NULL ) && ( *arg == gpIniptr->SwChr ); i++ ) {

                // point past switch character
                for (arg++; (*arg != '\0'); ) {

                        switch ( _ctoupper( *arg++ )) {
                        case 'A':
                                // retrieve based on specified atts
                                arg = GetSearchAttributes( arg );

                                // default to everything
                                *mode = 0x417;
                                break;

                        case 'D':       // don't colorize
                                glDirFlags |= DIRFLAGS_NO_COLOR;
                                break;

                        case 'E':       // display filenames in upper case
                                glDirFlags |= DIRFLAGS_UPPER_CASE;
                                break;

                        case 'H':       // hide "." and ".."
                                glDirFlags |= DIRFLAGS_NO_DOTS;
                                break;

                        case 'I':       // find matching descriptions
                                if ( *arg == '"' )
                                        sscanf( ++arg, "%511[^\"]", gszFindDesc );
                                else
                                        strcpy( gszFindDesc, arg );
                                arg = NULLSTR;
                                break;

                        case 'J':       // DOS justify filenames
                                glDirFlags |= DIRFLAGS_JUSTIFY;
                                break;

                        case 'L':       // display filenames in lower case
                                glDirFlags |= DIRFLAGS_LOWER_CASE;
                                break;

                        case 'O':       // dir sort order
                                // kludges for DOS 5 format
                                arg = dir_sort_order( arg );
                                break;

                        case 'T':       // display attributes or time field

                                if ( *arg ) {

                                        if ( *arg == ':' )
                                                arg++;

                                        switch ( _ctoupper( *arg )) {
                                        case 'A':       // last access
                                                gnDirTimeField = 1;
                                                break;
                                        case 'C':       // creation
                                                gnDirTimeField = 2;
                                                break;
                                        case 'W':       // last write (default)
                                                gnDirTimeField = 0;
                                                break;
                                        default:
                                                goto SelectError;
                                        }
                                        arg = NULLSTR;

                                } else
                                    glDirFlags |= DIRFLAGS_SHOW_ATTS;
                                break;

                        case 'Z':
                                glDirFlags |= DIRFLAGS_HPFS_TO_FAT;
                                break;

                        default:        // invalid option
SelectError:
                                error( ERROR_INVALID_PARAMETER, arg-1 );
                                return ( usage( SELECT_USAGE ));
                        }
                }
        }

        // skip the switch statements
        strcpy( start_line, ((gpNthptr == NULL) ? NULLSTR : gpNthptr));

        return 0;
}


// process the SELECT'd filenames
static int ProcessSelect( SELECT_ARGS *sel, char *szCmdline )
{
        unsigned int i, n;
        int rval = 0;
        long lSaveDirFlags;
        char *szFormat;

        // process the marked files in order
        for ( *szCmdline = '\0', n = 0, i = 1; ((rval != CTRLC) && (i <= sel->marked)); ) {

                if ( setjmp( cv.env ) == -1) {
                        clear_screen();
                        rval = CTRLC;
                        break;
                }

                if (sel->sfiles[n].file_mark == i) {

                        if (sel->open_paren == '(') {

                                // substitute the filename & run the
                                //   specified command

                                // kludge to save flags for "Select DIR ..."
                                lSaveDirFlags = glDirFlags;

                                // if LFN/HPFS names with whitespace, quote them
                                if ( glDirFlags & DIRFLAGS_HPFS ) {
                                        szFormat = ((( strpbrk( sel->sfiles[n].hpfs_name, " \t,+=:" ) != 0L ) || ( strpbrk( sel->szPath, " \t,+=|<>" ) != 0L )) ? "%s\"%s%Fs\"%s" : "%s%s%Fs%s");
                                        sprintf( szCmdline, szFormat, sel->cmd_start, sel->szPath, (( glDirFlags & DIRFLAGS_NT_ALT_NAME ) ? sel->sfiles[n].file_name : sel->sfiles[n].hpfs_name), sel->cmd_tail );
                                } else {

                                        // reassemble pre-/J name
                                        if (( glDirFlags & DIRFLAGS_JUSTIFY ) && ( sel->sfiles[n].file_name[8] == ' ' )) {

                                                int j;

                                                sel->sfiles[n].file_name[8] = '.';
                                                for ( j = 8; (( j > 0 ) && ( sel->sfiles[n].file_name[j-1] == ' ' )); j-- )
                                                        ;
                                                if ( j < 8 )
                                                        strcpy( sel->sfiles[n].file_name + j, sel->sfiles[n].file_name + 8 );
                                        }

                                        sprintf( szCmdline, "%s%s%Fs%s", sel->cmd_start, sel->szPath, sel->sfiles[n].file_name, sel->cmd_tail );
                                }

                                rval = command( szCmdline, 0 );

                                // restore flags (primarily for DIRFLAGS_HPFS)
                                glDirFlags = lSaveDirFlags;
                                *szCmdline = '\0';

                        } else {
                                // if LFN/HPFS names with whitespace, quote them
                                if (glDirFlags & DIRFLAGS_HPFS) {
                                        szFormat = ((( strpbrk( sel->sfiles[n].hpfs_name, " \t,+=:" ) != 0L ) || ( strpbrk( sel->szPath, " \t,+=:" ) != 0L )) ? "%s\"%Fs\" " : "%s%Fs " );
                                        sprintf( strend(szCmdline), szFormat, sel->szPath, sel->sfiles[n].hpfs_name );
                                } else
                                        sprintf( strend(szCmdline), "%s%Fs ", sel->szPath, sel->sfiles[n].file_name );
                        }

                } else if (++n < sel->entries)
                        continue;

                // reset index & marker
                n = 0;
                i++;
        }

        // check for [ ] (put everything on a single line) use
        if (szCmdline[0]) {
                strins( szCmdline, sel->cmd_start );
                strcat( szCmdline, sel->cmd_tail );
                rval = command( szCmdline, 0 );
        }

        HoldSignals();

        return rval;
}


