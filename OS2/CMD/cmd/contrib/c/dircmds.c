// DIRCMDS.C - Directory commands for 4xxx / TCMD family
//   Copyright (c) 1988 - 1997  Rex C. Conn  All rights reserved

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <dos.h>
#include <malloc.h>
#include <share.h>
#include <string.h>

#include "4all.h"
#include "wrappers.h"

static int GetDirectory(char *, char *);
static int DrawTree( char *, char * );
static void _nxtrow(void);
static int files_cmp( DIR_ENTRY *, DIR_ENTRY *);
static void ssort(char *, unsigned int);
static DIR_ENTRY * GetDescriptions( unsigned int, DIR_ENTRY *, char *);
static int _PrintGB(long long);


// disk total & free space
static QDISKINFO DiskInfo;

// files and sizes within directory tree branch
static unsigned long ulTreeFiles, ulTreeDirs;
static long long tree_size;
static long long tree_alloc;
static unsigned int dir_row;            // current row number
static unsigned int dir_mode;           // search attribute
static unsigned int dir_columns;        // number of columns (varies by screen size )
static unsigned int scr_columns;        // screen size in columns (base 1)
static int nScreenColor;                // default screen color ( for colorization)
static int fConsole;                    // if != 0, STDOUT is console
static int fIFS;                        // IFS type
static long long total;                 // total of file sizes
static long long llTotalAllocated;      // total amount of disk space actually used
extern RANGES aRanges;                  // date/time/size ranges
static char *pszCPBuffer;               // pointer to temp buffer
static unsigned int nCPLength;
static unsigned int nMaxFilename;


#define DIR_ENTRY_BLOCK 256
#define DIR_LFN_BYTES 16384


// initialize directory variables
void init_dir(void)
{
    fConsole = QueryIsConsole( STDOUT );

    gszSortSequence[0] = '\0';      // default to sort by filename
    gszFindDesc[0] = '\0';

    gnDirTimeField = 0;
    fIFS = 0;
    gchInclusiveMode = gchExclusiveMode = 0;
    glDirFlags = 0L;
    dir_columns = 1;
    dir_mode = FILE_DIRECTORY;        // default to normal files + directories, was 0x10
    ulTreeFiles = ulTreeDirs = 0L;
    tree_size = tree_alloc = 0;
    scr_columns = GetScrCols();
    nScreenColor = -1;
}


// initialize page size variables
void init_page_size(void)
{
    dir_row = gnPageLength = 0;
}


// retrieve files based on specified attributes
char *GetSearchAttributes( char *arg )
{
    int att, exclusive_flag;

    dir_mode = ( FIND_BYATTS | FILE_DIRECTORY | FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM );  // 0x417

    // kludge for DOS 5 format
    if ( (arg != NULL ) && (*arg == ':' ) )
        arg++;

    gchInclusiveMode = gchExclusiveMode = 0;

    // if no arg (/A), default to everything
    if ( ( arg == NULL ) || ( *arg == '\0' ) )
        return arg;

    // DOS 5-style; the '-' means exclusive search
    for ( ; ( *arg != '\0' ); arg++ ) {

        exclusive_flag = 0;

        if ( *arg == '-' ) {
            arg++;
            exclusive_flag = 1;
        }

        att = _ctoupper( *arg );

        if ( att == 'R' )
            att = _A_RDONLY;
        else if ( att == 'H' )
            att = _A_HIDDEN;
        else if ( att == 'S' )
            att = _A_SYSTEM;
        else if ( att == 'D' )
            att = _A_SUBDIR;
        else if ( att == 'A' )
            att = _A_ARCH;
        else if ( att == 'N' )
            att = 0;
        else if ( att == '_' )
            continue;
        else
            break;

        // set the flags & turn off the opposite flag (for example,
        //   if set in an alias)
        if ( exclusive_flag ) {
            gchExclusiveMode |= att;
            gchInclusiveMode &= ~att;
        } else {
            gchInclusiveMode |= att;
            gchExclusiveMode &= ~att;
        }
    }

    return arg;
}


// get the sort sequence
char * dir_sort_order(char *arg)
{
    // kludge for DOS 5 format
    if ( *arg == ':' )
        arg++;

    sscanf( strlwr( arg ), "%14[-acdeginsurtz]", gszSortSequence );

    return( arg + strlen( gszSortSequence ));
}


// display a directory list
int dir_cmd( int argc, char **argv )
{
    char *arg, *ptr;
    char filename[MAXFILENAME], fname[MAXFILENAME + 2]; // 2016-05-24 SHL + 2 in case quoted
    char szCPBuffer[595];
    int n, rval;

    init_dir();                     // initialize the DIR global variables
    init_page_size();
    pszCPBuffer = szCPBuffer;
    memset(pszCPBuffer, 0, 595);
    // default to *.* on FAT, * on LFN/HPFS/NTFS
    strcpy( fname, WILD_FILE );
    argc = rval = 0;

    // kludge for "dir,"
    if ( stricmp( argv[0]+3, "," ) == 0 ) {
        dir_mode = FIND_BYATTS | FILE_DIRECTORY | FILE_READONLY |
                   FILE_HIDDEN | FILE_SYSTEM;   // 0x417
    }

    // initialize date/time/size ranges
    if ( GetRange( argv[1], &aRanges, 0 ) != 0 )
        return ERROR_EXIT;

    do {

        if ( (arg = ntharg( argv[1], argc++ )) != NULL ) {

            if ( *arg == gpIniptr->SwChr ) {

                // point past switch character
                for ( arg++; ( *arg != '\0' ); ) {

                    switch ( _ctoupper(*arg++) ) {
                        case '1':       // single column display
                        case '2':       // 2 column display
                        case '4':       // 4 column display
                            dir_columns = ( arg[-1] - '0' );
                            break;

                        case 'A':       // retrieve based on specified attribs
                            arg = GetSearchAttributes( arg );
                            break;

                        case 'B':       // no headers, details, or summaries
                            glDirFlags |= ( DIRFLAGS_NO_HEADER | DIRFLAGS_NO_FOOTER | DIRFLAGS_NAMES_ONLY );
                            break;

                        case 'D':       // turn off directory colorization
                            glDirFlags |= DIRFLAGS_NO_COLOR;
                            break;

                        case 'E':       // display in upper case
                            glDirFlags |= DIRFLAGS_UPPER_CASE;
                            break;

                        case 'F':       // display fully expanded filename
                            glDirFlags |= ( DIRFLAGS_FULLNAME | DIRFLAGS_NO_HEADER | DIRFLAGS_NO_FOOTER );
                            break;

                        case 'G':
                            glDirFlags |= DIRFLAGS_ALLOCATED;
                            break;

                        case 'H':       // no "." or ".."
                            glDirFlags |= DIRFLAGS_NO_DOTS;
                            break;

                        case 'I':       // find matching descriptions
                            if ( *arg == '"' )
                                sscanf( ++arg, "%127[^\"]", gszFindDesc );
                            else
                                strcpy( gszFindDesc,arg);
                            arg = NULLSTR;
                            break;

                        case 'J':       // DOS justify filenames
                            glDirFlags |= DIRFLAGS_JUSTIFY;
                            break;

                        case 'K':       // no header
                            glDirFlags |= DIRFLAGS_NO_HEADER;
                            break;

                        case 'L':       // display in lower case
                            glDirFlags |= DIRFLAGS_LOWER_CASE;
                            break;

                        case 'M':       // no footer
                            glDirFlags |= DIRFLAGS_NO_FOOTER;
                            break;

                        case 'N':
                            // default to HPFS format
                            glDirFlags |= DIRFLAGS_HPFS;
                            break;

                        case 'O':       // dir sort order
                            arg = dir_sort_order( arg );
                            break;

                        case 'P':       // pause on pages
                            gnPageLength = GetScrRows();
                            break;

                        case 'Q':
                            glDirFlags |= DIRFLAGS_NO_ERRORS;
                            break;

                        case 'R':       // truncate descriptions
                            glDirFlags |= DIRFLAGS_TRUNCATE_DESCRIPTION;
                            break;

                        case 'S':       // recursive dir scan
                            glDirFlags |= DIRFLAGS_RECURSE;
                            break;

                        case 'T':       // display attributes or time field
                            if ( *arg ) {

                                if ( *arg == ':' )
                                    arg++;

                                switch ( _ctoupper( *arg ) ) {
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
                                        goto DirErrorExit;
                                }
                                arg = NULLSTR;

                            } else {
                                glDirFlags |= DIRFLAGS_SHOW_ATTS;
                                dir_columns = 1;
                            }
                            break;

                        case 'U':       // summaries only
                            glDirFlags |= DIRFLAGS_SUMMARY_ONLY;
                            break;

                        case 'V':       // vertical sort
                            glDirFlags |= DIRFLAGS_VSORT;
                            break;

                        case 'W':       // wide screen display
                            dir_columns = ( scr_columns / 16 );
                            glDirFlags |= DIRFLAGS_WIDE;
                            break;

                        case 'Z':
                            glDirFlags |= DIRFLAGS_HPFS_TO_FAT;
                            break;

                        default:
                            DirErrorExit:
                            error( ERROR_INVALID_PARAMETER, arg-1 );
                            rval = usage( DIR_USAGE );
                            goto dir_bye;
                    }
                }

            } else {

                // it must be a file or directory name
                strncpy( fname, arg, sizeof(fname) ); // 2016-05-24 SHL
                fname[sizeof(fname) - 1] = '\0'; // 2016-05-24 SHL

                // check for trailing switches
                // KLUDGE for COMMAND.COM compatibility ( DIR *.c /w)
                for ( n = argc; (( ptr = ntharg( argv[1], n )) != NULL ); n++ ) {
                    // check for another file spec
                    if ( *ptr != gpIniptr->SwChr )
                        goto show_dir;
                }
            }

        } else {

            show_dir:
            // put a path on the argument
            if ( mkfname( fname, 0 ) != NULL ) {

                // kludge to convert /B/S to /F/S
                if ( ( glDirFlags & DIRFLAGS_RECURSE ) && ( glDirFlags & DIRFLAGS_NAMES_ONLY ) )
                    glDirFlags |= DIRFLAGS_FULLNAME;

                // disable /J if used with /F
                if ( glDirFlags & DIRFLAGS_FULLNAME )
                    glDirFlags &= ~DIRFLAGS_JUSTIFY;

                if ( ( glDirFlags & DIRFLAGS_NO_HEADER ) == 0 )
                    _nxtrow();

                // if not a server sharename, display volume label and
                //   get usage stats for drive (we have to get the volume
                //   label first to kludge around a bug in the the
                //   Corel CD-ROM drivers)

                // DR-DOS returns an error on a JOINed drive!
                DiskInfo.ClusterSize = -1L;

                if ( is_net_drive( fname ) == 0 ) {

                    if ( ( glDirFlags & DIRFLAGS_NO_HEADER ) == 0 ) {
                        if ( getlabel( fname ) != 0 )
                            continue;
                        _nxtrow();
                    }

                    if ( QueryDiskInfo( fname, &DiskInfo, 1 ) != 0 )
                        continue;
                }

                // save filename part ( for recursive calls & include lists)
                copy_filename( filename, ( fname + strlen( path_part( fname ))) );

                // check to see if it's an LFN/HPFS/NTFS partition
                //   & the /N flag isn't set
                if ( (( fIFS = ifs_type( fname )) != 0 ) && (( glDirFlags & DIRFLAGS_HPFS ) == 0 ) ) {
                    glDirFlags |= DIRFLAGS_HPFS;
                    rval = GetDirectory( fname, filename );
                    glDirFlags &= ~DIRFLAGS_HPFS;
                } else
                    rval = GetDirectory( fname, filename );

                if ( ( setjmp( cv.env ) == -1 ) || ( rval == CTRLC ) ) {
                    rval = CTRLC;
                    goto dir_bye;
                }
                EnableSignals();
                // display the free bytes
                if ( ( DiskInfo.ClusterSize > 0L ) && (( glDirFlags & DIRFLAGS_NO_FOOTER ) == 0 ) ) {
                    printf( DIR_BYTES_FREE, DiskInfo.BytesFree );
                    // 20090430 AB added display in GB and MB
                    if ( gaInifile.NewByteDisp ) {
                        _PrintGB(DiskInfo.BytesFree);
                    }
                    printf( DIR_BYTES_FREE2);
                    _nxtrow();
                }
            }
        }

    } while ( ( rval != CTRLC ) && ( arg != NULL ) );

    dir_bye:
    HoldSignals();
    return rval;
}


static int GetDirectory( char *current, char *filename )
{
    unsigned int i;
    unsigned int j, k, n, entries = 0;
    unsigned int uFilesDisplayed = 0, uDirsDisplayed = 0;
    unsigned int last_entry, v_rows, v_offset, uColumns;
    int rval = 0;
    unsigned long t_files, t_dirs;
    long long t_size, t_alloc;
    DIR_ENTRY *files = 0L;    // file array in system memory
    char *ptr;

    // trap ^C and clean up
    if ( setjmp( cv.env ) == -1 ) {
        dir_abort:
        dir_free( files );
        return CTRLC;
    }

    EnableSignals();

    uColumns = dir_columns;
    nMaxFilename = 0;

    // force case change?
    if ( glDirFlags & DIRFLAGS_UPPER_CASE )
        strupr( current );
    else if ( (( glDirFlags & DIRFLAGS_HPFS ) == 0 ) || ( glDirFlags & DIRFLAGS_LOWER_CASE ) )
        strlwr( current );

    // save the current tree size & entries for recursive display
    t_size = tree_size;
    t_alloc = tree_alloc;
    t_files = ulTreeFiles;
    t_dirs = ulTreeDirs;
    total = llTotalAllocated = 0LL;

    if ( is_dir( current ) ) {
        ptr = WILD_FILE;
        mkdirname( current, ptr );
        filename = ptr;
        // don't do this stuff on an LFN / HPFS / NTFS volume!
    } else if ( ( glDirFlags & DIRFLAGS_HPFS ) == 0 ) {
        // if no include list & no extension specified, add default one
        //   else if no filename, insert a wildcard filename
        if ( strchr( current, ';' ) == NULL ) {

            if ( ext_part( current ) == NULL ) {
                ptr = strlast( current );
                if ( ( *ptr != '*' ) && ( *ptr != '?' ) )
                    strcat( current, WILD_FILE );
            } else {
                // point to the beginning of the filename
                ptr = current + strlen( path_part( current ));
                if ( *ptr == '.' )
                    strins( ptr, "*" );
            }
        }
    }

    // set "get descriptions?" flag
    j = (( gszFindDesc[0] != '\0' ) || ((( glDirFlags & DIRFLAGS_SUMMARY_ONLY ) == 0 ) && ( uColumns == 1 )));

    // check for file colorization (SET COLORDIR=...)
    if ( ( glDirFlags & DIRFLAGS_SUMMARY_ONLY ) == 0 )
        j |= 4;

    // look for matches
    if ( SearchDirectory( dir_mode | FIND_DATERANGE, current, ( DIR_ENTRY **)&files, &entries, &aRanges, j ) != 0 ) {
        rval = ERROR_EXIT;
        goto do_dir_bye;
    }

    if ( entries > 0 ) {

        if ( ( glDirFlags & DIRFLAGS_NO_HEADER ) == 0 ) {

            if ( glDirFlags & DIRFLAGS_RECURSING_NOW )
                _nxtrow();

            // display "Directory of c:\wonky\*.*"
            printf( DIRECTORY_OF, current );
            _nxtrow();
            _nxtrow();
        }

        // mondo ugly kludge to emulate CMD.EXE behavior
        if ( ( uColumns > 1 ) && ( glDirFlags & DIRFLAGS_HPFS ) && (( glDirFlags & DIRFLAGS_HPFS_TO_FAT ) == 0 ) ) {

            // check if max HPFS name is wider than screen!
            if ( ( nMaxFilename + 4 ) > ( scr_columns / uColumns ) ) {
                if ( ( uColumns = ( scr_columns / ( nMaxFilename + 4 ))) <= 1 ) {
                    glDirFlags &= ~DIRFLAGS_VSORT;
                    uColumns = 0;
                }
            }
        }

        // directory display loop
        for ( n = 0, last_entry = 0, v_rows = v_offset = 0; ( n < entries ); n++ ) {

            if ( files[n].attribute & FILE_DIRECTORY )
                uDirsDisplayed++;
            else
                uFilesDisplayed++;

            // get file size & size rounded to cluster size
            total += files[n].file_size;
            if ( DiskInfo.ClusterSize != -1L ) {

                // HPFS volumes use 1 cluster for 0-byte files
                if ( ( fIFS == 1 ) && ( glDirFlags & DIRFLAGS_HPFS ) && ( files[n].file_size == 0L ) ) {
                    // but don't count "." and ".."!
                    if ( ( stricmp( files[n].hpfs_name, "." ) != 0 ) && ( stricmp( files[n].hpfs_name, ".." ) != 0 ) )
                        llTotalAllocated += DiskInfo.ClusterSize;
                } else
                    if ( files[n].file_size > 0L ) {
                    llTotalAllocated += (long long)(( files[n].file_size + ( DiskInfo.ClusterSize - 1 )) / DiskInfo.ClusterSize ) * DiskInfo.ClusterSize;
                }
            }

            // summaries only?
            if ( glDirFlags & DIRFLAGS_SUMMARY_ONLY )
                continue;

            // check for vertical (column) sort
            // NOTE: This is magic code!  If you change it, you
            //   will probably break it!!
            if ( ( glDirFlags & DIRFLAGS_VSORT ) && ( uColumns > 0 ) ) {

                // check for new page
                if ( n >= last_entry ) {

                    // get the last entry for previous page
                    v_offset = last_entry;

                    // get last entry for this page
                    if ( ( gnPageLength == 0 ) || (( last_entry += ( uColumns * ( gnPageLength - dir_row))) > entries) )
                        last_entry = entries;

                    v_rows = ((( last_entry - v_offset ) + ( uColumns - 1 )) / uColumns );
                }

                i = n - v_offset;
                k = ( i % uColumns );
                i = (( i / uColumns ) + ( k * v_rows )) + v_offset;

                // we might have uneven columns on the last page!
                if ( last_entry % uColumns ) {
                    j = ( last_entry % uColumns );
                    if ( k > j )
                        i -= ( k - j );
                }

            } else
                i = n;

            if ( glDirFlags & DIRFLAGS_ALLOCATED )
                files[i].file_size = ((long long)( files[i].file_size + ( DiskInfo.ClusterSize - 1 )) / DiskInfo.ClusterSize ) * DiskInfo.ClusterSize;

            if ( glDirFlags & DIRFLAGS_FULLNAME ) {

                ptr = path_part( current );

                // just print the fully expanded filename
                printf( "%s%Fs", ptr, ((( glDirFlags & DIRFLAGS_HPFS ) && (( glDirFlags & DIRFLAGS_HPFS_TO_FAT ) == 0 )) ? files[i].hpfs_name : files[i].file_name ));
                uColumns = 1;

            } else if ( (( glDirFlags & DIRFLAGS_HPFS ) == 0 ) || ( glDirFlags & DIRFLAGS_HPFS_TO_FAT ) || (( glDirFlags & DIRFLAGS_NT_ALT_NAME ) && (( uColumns > 1 ) || ( glDirFlags & DIRFLAGS_NAMES_ONLY ))) ) {

                if ( (( glDirFlags & DIRFLAGS_NAMES_ONLY ) == 0 ) || ( uColumns > 1 ) ) {

                    // if DIR /W, display directories with [ ]'s
                    if ( ( glDirFlags & DIRFLAGS_WIDE ) && ( files[i].attribute & _A_SUBDIR ) )
                        nCPLength = sprintf( pszCPBuffer, "[%Fs]%*s", files[i].file_name, 12 - strlen( files[i].file_name ), NULLSTR );
                    else
                        nCPLength = sprintf( pszCPBuffer, FMT_FAR_LEFT_STR, (( uColumns > 4 ) ? 14 : 12 ), files[i].file_name );

                    if ( uColumns <= 2 ) {
                        nCPLength += sprintf( pszCPBuffer+nCPLength, ( files[i].attribute & _A_SUBDIR) ? DIR_LABEL : "%9q", files[i].file_size );
                        nCPLength += sprintf( pszCPBuffer+nCPLength, "  %s ",(( files[i].fd.ufDate == 0 ) ? "           " : FormatDate( files[i].fd.file_date.months,files[i].fd.file_date.days, files[i].fd.file_date.years+1980, 0 )));
                        nCPLength += sprintf( pszCPBuffer+nCPLength, (( files[i].fd.ufDate == 0 ) ? "      " : (( gaCountryInfo.fsTimeFmt == 0 ) ? "%2u%c%02u%c" : " %2u%c%02u" )),
                                              files[i].ft.file_time.hours, gaCountryInfo.szTimeSeparator[0], files[i].ft.file_time.minutes, files[i].ampm );

                        if ( uColumns == 1 ) {
                            if ( scr_columns > 40 ) {
                                if ( glDirFlags & DIRFLAGS_SHOW_ATTS )
                                    nCPLength += sprintf( pszCPBuffer+nCPLength," %s",show_atts( files[i].attribute ));
                                nCPLength += sprintf( pszCPBuffer+nCPLength," %Fs",files[i].file_id);

                                // wrap the description?
                                if ( ( glDirFlags & DIRFLAGS_TRUNCATE_DESCRIPTION ) == 0 ) {

                                    while ( (unsigned int)nCPLength >= scr_columns ) {

                                        for ( j = scr_columns-1; (( pszCPBuffer[j] != ' ' ) && ( pszCPBuffer[j] != '\t' ) && ( j > 50 )); j-- )
                                            ;

                                        // if no whitespace, make some!
                                        if ( j <= 50 ) {
                                            j = scr_columns - 1;
                                            strins( pszCPBuffer+j, " " );
                                        }
                                        pszCPBuffer[j] = '\0';

                                        // display the line
                                        color_printf( files[i].color, FMT_STR, pszCPBuffer );
                                        _nxtrow();

                                        // pad start of next line w/blanks
                                        memset( pszCPBuffer, ' ', 39 );
                                        strcpy( pszCPBuffer + 39, pszCPBuffer + j + 1 );
                                        nCPLength = strlen( pszCPBuffer );
                                    }
                                }
                            }
                        }

                    } else if ( ( uColumns == 4 ) && (( glDirFlags & DIRFLAGS_WIDE ) == 0 ) ) {

                        // FIXME - change to INT64??
                        LONGLONG fsize;

                        // display file size in Kb, Mb, or Gb
                        if ( files[i].attribute & _A_SUBDIR )
                            sprintf( pszCPBuffer+nCPLength, "  <D>" );

                        else {

                            fsize = files[i].file_size;

                            // if between 1 & 10 Mb, display
                            //   decimal part too
                            if ( ( files[i].file_size >= 1048576L ) && ( files[i].file_size < 10485760L ) ) {
                                fsize /= 1048576L;
                                sprintf( pszCPBuffer+nCPLength, " %q.%qM", fsize, (( (long long)files[i].file_size % 1048576L ) / 104858L ));
                            } else {
                                for ( ptr = DIR_FILE_SIZE; (( fsize = (( fsize + 1023 ) / 1024L )) > 999 ); ptr++ )
                                    ;
                                sprintf( pszCPBuffer+nCPLength, "%4q%c", fsize, *ptr );
                            }
                        }
                    }

                } else
                    strcpy( pszCPBuffer, files[ i ].file_name );

                // display the line
                color_printf( files[i].color, FMT_STR, pszCPBuffer );

            } else {

                // HPFS or LFN format
                // clear color_printf string length
                nCPLength = 0;

                // format HPFS long filename display
                if ( ( uColumns == 1 ) && ( glDirFlags & DIRFLAGS_WIDE ) == 0 ) {

                    if ( ( glDirFlags & DIRFLAGS_NAMES_ONLY ) == 0 ) {

                        // date
                        nCPLength = sprintf( pszCPBuffer, (( files[i].fd.ufDate == 0 )  ? "           " : FormatDate( files[i].fd.file_date.months,files[i].fd.file_date.days,files[i].fd.file_date.years+1980, 0)));
                        
                        // time
                        nCPLength += sprintf( pszCPBuffer+nCPLength, (( files[i].ft.ufTime == 0 ) ? "        " : "  %2u%c%02u%c" ),files[i].ft.file_time.hours,gaCountryInfo.szTimeSeparator[0],files[i].ft.file_time.minutes,files[i].ampm);

                        // size or <DIR>
                        nCPLength += sprintf( pszCPBuffer+nCPLength, ( files[i].attribute & _A_SUBDIR) ? HPFS_DIR_LABEL : "%15Lq", files[i].file_size );
                        // size of EA list
                        nCPLength += sprintf( pszCPBuffer+nCPLength, "%7Lu  ", files[i].ea_size );    // 20090819 AB, was %12Lu, EA size can not be bigger than 64k
                    }

                    // check if attribute display requested
                    if ( glDirFlags & DIRFLAGS_SHOW_ATTS )
                        nCPLength += sprintf( pszCPBuffer+nCPLength," %s  ", show_atts( files[i].attribute ));
                }

                // set max HPFS name width
                if ( uColumns > 1 ) {
                    j = ( scr_columns / uColumns ) - 4;
                } else if ( uColumns < 1 )
                    j = nMaxFilename;
                else
                    j = 0;

                // filename
                if ( ( uColumns != 1 ) && ( files[i].attribute & _A_SUBDIR ) )
                    nCPLength += sprintf( pszCPBuffer+nCPLength, "[%Fs]%*s", files[i].hpfs_name, j - strlen( files[i].hpfs_name ), NULLSTR );
                else
                    nCPLength += sprintf( pszCPBuffer+nCPLength, FMT_FAR_LEFT_STR, j+2, files[i].hpfs_name );
                if ( fConsole == 0 )
                    qputs( pszCPBuffer );

                else {

                    // wrap long HPFS names
                    for ( j = 0; ; ) {

                        color_printf( files[i].color, FMT_PREC_STR, scr_columns, pszCPBuffer+j );
                        if ( ( j += scr_columns ) >= nCPLength )
                            break;

                        if ( files[i].color != -1 )
                            _nxtrow();
                        else
                            _page_break();
                    }
                }
            }

            // if 2 or 4 column, or wide display, add spaces
            if ( ( uColumns <= 1 ) || ((( n + 1 ) % uColumns ) == 0 ) )
                _nxtrow();
            else if ( ( glDirFlags & DIRFLAGS_HPFS ) && (( glDirFlags & DIRFLAGS_HPFS_TO_FAT ) == 0 ) )
                qputs( "  " );
            else
                qputs(( glDirFlags & DIRFLAGS_WIDE ) ? "  " : "   " );
        }

        if ( ( uColumns > 1 ) && ( n % uColumns ) )
            _nxtrow();

        ulTreeFiles += uFilesDisplayed;
        ulTreeDirs += uDirsDisplayed;
        tree_alloc += llTotalAllocated;
        tree_size += total;
    } else            // OS/2 considers no files to be an error return
        rval = ERROR_EXIT;

    // print the directory totals
    if ( (( glDirFlags & DIRFLAGS_NO_FOOTER ) == 0 ) && ((( uFilesDisplayed + uDirsDisplayed ) > 0 ) || (( glDirFlags & DIRFLAGS_RECURSE ) == 0 )) ) {

        // 20090430 AB added display in GB and MB
        i = printf( DIR_BYTES_IN_FILES_NEW1, total );
        if ( gaInifile.NewByteDisp ) {
            _PrintGB( total );
        }
        i += printf( DIR_BYTES_IN_FILES_NEW2, (long)uFilesDisplayed, (( uFilesDisplayed == 1 ) ? ONE_FILE : MANY_FILES ), (LONG)uDirsDisplayed, ((uDirsDisplayed == 1) ? ONE_DIR : MANY_DIRS ));

        // if not a server sharename, display allocated & free space
        if ( ( current[0] != '\\' ) && ( uFilesDisplayed > 0 ) ) {
            // 20090430 AB added display in GB and MB and in new line
            if ( gaInifile.NewByteDisp > 1 ) {
                // 20090505 AB allocated space is displayed with NewByteDisp level 2
                printf( DIR_BYTES_ALLOCATED_NEW1, llTotalAllocated );
                _PrintGB( llTotalAllocated );
                printf( DIR_BYTES_ALLOCATED_NEW2 );
            } else if ( !gaInifile.NewByteDisp ) {
                printf( DIR_BYTES_ALLOCATED, llTotalAllocated );
            }
        }

        _nxtrow();
    }

    // free memory used to store dir entries
    dir_free( files );
    files = 0L;

    // do a recursive directory search?
    if ( glDirFlags & DIRFLAGS_RECURSE ) {

        // remove filename
        insert_path( current, WILD_FILE, path_part( current ) );
        ptr = strchr( current, '*' );

        entries = 0;

        // search for subdirectories
#define M ( FIND_DIRONLY | FILE_DIRECTORY )      // 0x210
        if ( SearchDirectory((( dir_mode & 0xFF ) | M ), current, (DIR_ENTRY **)&files, &entries, &aRanges, 0 ) != 0 ) {
#undef M
            rval = ERROR_EXIT;
            goto do_dir_bye;
        }

        for ( i = 0; ( i < entries ); i++ ) {

            glDirFlags |= DIRFLAGS_RECURSING_NOW;

            // make directory name
            sprintf( ptr, "%Fs\\%s", (( glDirFlags & DIRFLAGS_HPFS ) ? files[i].hpfs_name : files[i].file_name ), filename );
            rval = GetDirectory( current, filename );

            // check for ^C and reset ^C trapping
            if ( ( setjmp( cv.env ) == -1) || ( rval == CTRLC ) )
                goto dir_abort;
            EnableSignals();
        }

        dir_free( files );
        files = 0L;

        // if we've got some subdirectories & some entries in the
        //   current directory, print a subtotal
        if ( (( glDirFlags & DIRFLAGS_NO_FOOTER) == 0 ) && (( ulTreeFiles + ulTreeDirs) > (t_files + t_dirs)) && (entries > 0 ) ) {

            // display figures for local tree branch
            strcpy( ptr, filename );
            _nxtrow();
            printf( DIR_TOTAL, current );
            _nxtrow();
            t_files = ( ulTreeFiles - t_files );
            t_dirs = ( ulTreeDirs - t_dirs );
            t_size = ( tree_size - t_size );
            t_alloc = ( tree_alloc - t_alloc );

            // 20090430 AB added display in GB and MB
            i = printf( DIR_BYTES_IN_FILES_NEW1, t_size );
            if ( gaInifile.NewByteDisp ) {
                _PrintGB( t_size );
            }
            i += printf( DIR_BYTES_IN_FILES_NEW2, t_files, ((t_files == 1) ? ONE_FILE : MANY_FILES ), t_dirs, ((t_dirs == 1) ? ONE_DIR : MANY_DIRS ));

            if ( DiskInfo.ClusterSize != -1L ) {
                // print allocated size
                // 20090430 AB added display in GB and MB and in new line
                if ( gaInifile.NewByteDisp > 1 ) {
                    printf( DIR_BYTES_ALLOCATED_NEW1, t_alloc );
                    _PrintGB( t_alloc );
                    printf( DIR_BYTES_ALLOCATED_NEW2 );
                } else if ( !gaInifile.NewByteDisp ) {
                    printf( DIR_BYTES_ALLOCATED, t_alloc );
                }
            }

            _nxtrow();
        }
    }

    do_dir_bye:
    // disable signal handling momentarily
    HoldSignals();

    return rval;
}


// free the array(s) used for directory storage
void dir_free( DIR_ENTRY *files)
{
    // free the segment containing the long HPFS filenames
    if ( ( files != 0L ) && ( glDirFlags & DIRFLAGS_HPFS ) )
        FreeMem( files[0].hpfs_base );
    FreeMem( (char *)files );
}


// go to next row & check for page full
static void _nxtrow( void )
{
    crlf();

    _page_break();
}


// pause if we're at the end of a page & /P was specified
void _page_break( void )
{
    if ( ++dir_row == gnPageLength ) {

        dir_row = 0;

        // make sure STDOUT hasn't been redirected
        if ( QueryIsConsole( STDOUT ) == 0 )
            return;

        qprintf( STDOUT, PAUSE_PAGE_PROMPT );
        if ( GetKeystroke( EDIT_NO_ECHO | EDIT_ECHO_CRLF ) == ESC )
            BreakOut();
    }
}


// get filename colors
void ColorizeDirectory( DIR_ENTRY *files, unsigned int entries, int wc_flag)
{
    unsigned int i, n;
    unsigned int c_entries, size;
    int row, column;
    int j, len, fg, bg;
    char *fptr, *colordir, buf[32];
    CDIR *cdir = 0L;

    // check for no colors requested or STDOUT redirected
    if ( ( fConsole == 0 ) || ( glDirFlags & DIRFLAGS_NO_COLOR) )
        return;

    // if no COLORDIR variable, was ColorDir defined in 4DOS.INI?
    if ( (( colordir = get_variable( COLORDIR )) == 0L ) && ( gpIniptr->DirColor != INI_EMPTYSTR) )
        colordir = (char *)( gpIniptr->StrData + gpIniptr->DirColor );

    if ( colordir == 0L )
        return;

    // if wc_flag = 1, then write a space & reread it to get the default
    //   background color
    if ( wc_flag ) {
        // get current cursor position
        GetCurPos( &row, &column );
        qputc( STDOUT, ' ' );
        SetCurPos( row, column );
    }

    GetAtt( (unsigned int *)&nScreenColor, &size );

    n = ( nScreenColor >> 4 );

    // create the structure with the extensions & colors
    for ( c_entries = 0, size = 0; ( *colordir ); ) {

        if ( ( c_entries % ( 1024 / sizeof(CDIR))) == 0 ) {
            size += 1024;
            if ( (cdir = (CDIR *)ReallocMem((char *)cdir,(ULONG)size )) == 0L )
                return;
        }

        // get the next extension in color list
        sscanf( colordir, " %29F[^ \t:;] %n", cdir[c_entries].cname, &len );

        // default to white foreground
        fg = 7;
        bg = n;

        // get the color for this extension
        if ( ( fptr = strchr( colordir, ':' )) != 0L ) {
            sscanf( ++fptr, "%30[^;]", buf );
            ParseColors( buf, &fg, &bg );
        }

        // add it to the list
        cdir[c_entries].color = ( fg + ( bg << 4 ));
        c_entries++;

        colordir += len;

        if ( ( *colordir == ':' ) || ( *colordir == ';' ) ) {
            // skip the color specs & get next extension
            while ( ( *colordir ) && ( *colordir++ != ';' ) )
                ;
        }
    }

    // compare file extension against extensions saved in colordir struct
    for ( i = 0; ( i < entries ); i++ ) {

        // get filename extension
        // in HPFS name, the *LAST* '.' is the extension!
        fptr = (( glDirFlags & DIRFLAGS_HPFS ) ? files[i].hpfs_name : files[i].file_name );
        if ( ( fptr = strrchr( fptr, '.' )) == 0L )
            fptr = NULLSTR;
        else
            fptr++;

        for ( n = 0; ( n < c_entries ); n++ ) {

            if ( wild_cmp( cdir[n].cname, fptr, TRUE, TRUE ) != 0 ) {

                // check for "colorize by attributes"
                if ( strlen( cdir[n].cname ) > 3 ) {
                    for ( j = 0; ( j < 5 ); j++ ) {
                        if ( ( stricmp( cdir[n].cname, colorize_atts[j].type ) == 0 ) && ( files[i].attribute & colorize_atts[j].attr ) )
                            goto got_dcolor;
                    }
                }

                continue;
            }
            got_dcolor:
            files[i].color = cdir[n].color;
            break;
        }
    }

    FreeMem( (char *)cdir );
}


#define NUMERIC_SORT 0x1000
#define NO_EXTENSION_SORT 0x10
#define NO_WHITESPACE_SORT 0x20
#define ASCII_SORT 1

// FAR string compare
int fstrcmp( char *s1, char *s2, int fASCII )
{
    static char szFmt[] = "%ld%n";
    int i;
    long ln1, ln2;

    while ( *s1 ) {

        // sort numeric strings (i.e., 3 before 13)
        if ( ( fASCII & NUMERIC_SORT ) && is_unsigned_digit( *s1 ) && is_unsigned_digit( *s2 ) ) {

            // kludge for morons who do "if %1# == -# ..."
            if ( (( isdigit( *s1 ) == 0 ) && ( isdigit( s1[1] ) == 0 )) || (( isdigit( *s2 ) == 0 ) && (isdigit(s2[1]) == 0 )) ) {
                fASCII &= ~NUMERIC_SORT;
                goto NotNumeric;
            }

            sscanf( s1, szFmt, &ln1, &i );
            s1 += i;
            sscanf( s2, szFmt, &ln2, &i );
            s2 += i;

            if ( ln1 != ln2 )
                return(( ln1 > ln2 ) ? 1 : -1 );

        } else {
            NotNumeric:
            if ( ( i = ( _ctoupper( *s1 ) - _ctoupper( *s2 ))) != 0 )
                return i;

            if ( ( fASCII & NO_EXTENSION_SORT ) && ( *s1 == '.' ) ) {
                if ( ( strchr( s1+1, '.' ) == NULL ) && ( strchr( s2+1, '.' ) == NULL ) )
                    return 0;
            }

            s1++;
            s2++;
        }
    }

    return( _ctoupper( *s1 ) - _ctoupper( *s2 ));
}


// compare routine for filename sort
static int files_cmp( DIR_ENTRY *a, DIR_ENTRY *b )
{
    char *ptr;
    int rval = 0;
    int sortorder, reverse, fASCII = NUMERIC_SORT;
    char *fptr_a, *fptr_b;

    // 'r' sort everything in reverse order
    for ( sortorder = 0, ptr = gszSortSequence; ( *ptr != '\0' ); ptr++ ) {
        if ( *ptr == 'r' ) {
            sortorder = 1;
            break;
        }
    }

    for ( ptr = gszSortSequence; (( *ptr ) && ( rval == 0 )); ptr++ ) {

        // check for backwards sort for this single field
        reverse = 0;
        if ( *ptr == '-' ) {
            reverse++;
            if ( ptr[1] )
                ptr++;
        }

        if ( *ptr == 'u' )
            return rval;

        if ( *ptr == 'a' ) {

            // don't do a numeric sort
            fASCII = ASCII_SORT;

        } else if ( ( *ptr == 'd' ) || ( *ptr == 't' ) ) {

            // sort by date / time
            // (we can't just subtract the dates because of a
            //   16-bit compiler bug)
            if ( a->fd.ufDate > b->fd.ufDate )
                rval = 1;
            else if ( a->fd.ufDate < b->fd.ufDate )
                rval = -1;
            else {

                unsigned int i, n;

                // kludge for AM/PM support
                i = a->ft.ufTime;
                n = b->ft.ufTime;
                if ( a->ampm == 'p' ) {
                    if ( a->ft.file_time.hours < 12 )
                        a->ft.file_time.hours += 12;
                } else if ( ( a->ampm == 'a' ) && ( a->ft.file_time.hours == 12) )
                    a->ft.file_time.hours = 0;

                if ( b->ampm == 'p' ) {
                    if ( b->ft.file_time.hours < 12 )
                        b->ft.file_time.hours += 12;
                } else if ( ( b->ampm == 'a' ) && ( b->ft.file_time.hours == 12) )
                    b->ft.file_time.hours = 0;

                if ( a->ft.ufTime > b->ft.ufTime )
                    rval = 1;
                else if ( a->ft.ufTime < b->ft.ufTime )
                    rval = -1;
                else
                    rval = 0;

                a->ft.ufTime = i;
                b->ft.ufTime = n;
            }

        } else if ( *ptr == 'e' ) {     // extension sort

            if ( glDirFlags & DIRFLAGS_HPFS ) {
                fptr_a = a->hpfs_name;
                fptr_b = b->hpfs_name;
            } else {
                fptr_a = a->file_name;
                fptr_b = b->file_name;
            }

            // sort current dir (.) & root dir (..) first
            for ( ; ( *fptr_a == '.' ); fptr_a++ )
                ;
            for ( ; ( *fptr_b == '.' ); fptr_b++ )
                ;

            // on HPFS volumes, the extension is the LAST '.'
            if ( glDirFlags & DIRFLAGS_HPFS ) {

                if ( ( fptr_a = strrchr( fptr_a, '.' )) == 0L )
                    fptr_a = NULLSTR;
                if ( ( fptr_b = strrchr( fptr_b, '.' )) == 0L )
                    fptr_b = NULLSTR;

            } else {

                // skip to extension
                for ( ; (( *fptr_a != '\0' ) && (*fptr_a != '.' )); fptr_a++ )
                    ;
                for ( ; (( *fptr_b != '\0' ) && (*fptr_b != '.' )); fptr_b++ )
                    ;
            }

            rval = fstrcmp( fptr_a, fptr_b, fASCII );

        } else if ( ( *ptr == 's' ) || ( *ptr == 'z' ) ) {

            // sort by filesize
            rval = (( a->file_size < b->file_size ) ? -1 : ( a->file_size > b->file_size ));

        } else if ( *ptr == 'i' )                // sort by description (ID)
            rval = fstrcmp( a->file_id, b->file_id, fASCII );

        else if ( *ptr == 'n' ) {

            // sort by filename (not including extensions)
            if ( glDirFlags & DIRFLAGS_HPFS )
                rval = fstrcmp( a->hpfs_name, b->hpfs_name, fASCII | NO_EXTENSION_SORT );
            else
                rval = fstrcmp( a->file_name, b->file_name, fASCII | NO_EXTENSION_SORT );

        } else if ( *ptr == 'g' ) {
            // sort directories together
            rval = ((b->attribute & _A_SUBDIR) - (a->attribute & _A_SUBDIR));
        }

        // reverse order for this one field?
        if ( reverse )
            rval = -rval;
    }

    if ( rval == 0 ) {

        // sort directories first
        if ( ( rval = (( b->attribute & _A_SUBDIR) - (a->attribute & _A_SUBDIR))) == 0 ) {
            if ( glDirFlags & DIRFLAGS_HPFS )
                rval = fstrcmp( a->hpfs_name, b->hpfs_name, fASCII );
            else
                rval = fstrcmp( a->file_name, b->file_name, fASCII );
        }
    }

    return(( sortorder ) ? -rval : rval );
}


// do a Shell sort on the directory (much smaller & less stack than Quicksort)
static void ssort(char *base, unsigned int entries)
{
    unsigned int i, gap;
    char *p1, *p2;
    char szTmp[ sizeof(DIR_ENTRY) ];
    long j;

    for ( gap = (entries >> 1); ( gap > 0 ); gap >>= 1 ) {

        for ( i = gap; (i < entries); i++ ) {

            for ( j = (i - gap); (j >= 0L ); j -= gap ) {

                p1 = p2 = base;
                p1 += (j * sizeof( DIR_ENTRY));
                p2 += ((j + gap) * sizeof( DIR_ENTRY));

                if ( files_cmp(( DIR_ENTRY *)p1,( DIR_ENTRY *)p2) <= 0 )
                    break;

                // swap the two records
                memmove( szTmp, p1, sizeof( DIR_ENTRY) );
                memmove( p1, p2, sizeof( DIR_ENTRY) );
                memmove( p2, szTmp, sizeof( DIR_ENTRY) );
            }
        }
    }
}


// Read the directory and set the pointer to the saved directory array
//      attrib - search attribute
//      arg - filename/directory to search
//      hptr - pointer to address of FILES array
//      entries - pointer to number of entries in FILES
//      nFlags: 0 = no descriptions
//              1 = get descriptions
//              2 = don't save anything except name
int SearchDirectory( int attrib, char *arg, DIR_ENTRY **hptr, unsigned int *entries, RANGES *paRanges, int nFlags )
{
    unsigned int fval;
    unsigned long size = 0L;
    DIR_ENTRY *files;
    char *saved_arg;
    FILESEARCH dir;
    int h_size = 0, cchLength;
    ULONG hpfs_names_size = 0L;
    char *hpHPFSNames = 0L;   // pointer inside HPFS names segment
    char *hpLFNStart = 0L;

    files = *hptr;

    // create new var so we don't overwrite original argument
    saved_arg = arg;

    // if arg has a ';', it must be an include list
    if ( strchr( arg, ';' ) != NULL ) {
        arg = _alloca( strlen( saved_arg ) + 1 );
        strcpy( arg, saved_arg );
    }
    // copy date/time range info
    memmove( &(dir.aRanges), paRanges, sizeof(RANGES ) );

    for ( fval = FIND_FIRST; ; fval = FIND_NEXT ) {

        // if error, no entries & no recursion, display error & bomb
#define M (FIND_NO_CASE_CONV | FIND_NOERRORS )   // 0x4100
        if ( find_file( fval, arg, (unsigned int)(attrib | M ), &dir, NULL ) == NULL ) {
#undef M

            if ( ( *entries == 0 ) && (( glDirFlags & DIRFLAGS_RECURSE) == 0 ) &&
                (( glDirFlags & DIRFLAGS_NO_ERRORS) == 0 ) )
                error( ERROR_FILE_NOT_FOUND, saved_arg );
            break;
        }

        // if a /B or /H, ignore "." and ".."
        if ( (( glDirFlags & DIRFLAGS_NAMES_ONLY ) || ( glDirFlags & DIRFLAGS_NO_DOTS )) && ( QueryIsDotName( dir.name )) )
            continue;

        // allocate the temporary memory blocks
        if ( ( *entries % DIR_ENTRY_BLOCK ) == 0 ) {
            size += (sizeof( DIR_ENTRY ) * DIR_ENTRY_BLOCK);
            files = (DIR_ENTRY *)ReallocMem( (char *)files, size );
        }

        // set original pointer to allow cleanup on ^C
        if ( ( *hptr = files ) == 0L ) {
            FreeMem( hpLFNStart );
            return( error( ERROR_NOT_ENOUGH_MEMORY, NULL ));
        }

        // initialize HPFS names
        files[ *entries ].hpfs_base = 0L;
        // include the trailing '\0' in the count
        cchLength = dir.cchName + 1;

        // since HPFS partitions can have long filenames, we need
        //   to store them in a different segment
        if ( glDirFlags & DIRFLAGS_HPFS ) {

            // Do we need to allocate more memory for filenames?
            if ( ( h_size -= cchLength ) <= 0 ) {
                //                                char *old_hptr = (char *)files[0].hpfs_base;
                h_size += (DIR_LFN_BYTES - 1);
                hpfs_names_size += DIR_LFN_BYTES;

                if ( ( files[0].hpfs_base = ReallocMem( files[0].hpfs_base, hpfs_names_size )) == 0L ) {
                    FreeMem( (char *)files );
                    return( error( ERROR_NOT_ENOUGH_MEMORY, NULL ));
                }

                if ( *entries == 0 )
                    hpLFNStart = hpHPFSNames = files[0].hpfs_base;
            }

            dir.name[ cchLength ] = '\0';

            // kludge for /E and /L support on LFN/HPFS/NTFS drives
            if ( glDirFlags & DIRFLAGS_LOWER_CASE )
                strlwr( dir.name );
            else if ( glDirFlags & DIRFLAGS_UPPER_CASE )
                strupr( dir.name );

            // We don't do case conversions on LFN/HPFS filenames
            files[ *entries ].hpfs_name = memmove( hpHPFSNames, (char *)dir.name, cchLength );
            hpHPFSNames += cchLength;

            files[ *entries ].file_name[0] = '\0';
            if ( glDirFlags & DIRFLAGS_HPFS_TO_FAT ) {

                // if filename more than 12 chars (FAT maximum),
                //   append a ^P
                if ( cchLength > 13 )
                    dir.name[11] = gchRightArrow;

                cchLength = 13;
                dir.name[ 12 ] = '\0';
                strcpy( files[*entries].file_name, dir.name );
            }

            // get max column width
            if ( ( cchLength - 1 ) > (int)nMaxFilename )
                nMaxFilename = cchLength - 1;

        } else {
            // no default case conversion for NT!
            if ( ( glDirFlags & DIRFLAGS_LOWER_CASE ) || ((( glDirFlags & DIRFLAGS_UPPER_CASE) == 0 ) && (( dir.attrib & _A_SUBDIR ) == 0 ) && ( gpIniptr->Upper == 0 )) )
                strlwr( dir.name );
            // save the name into the FAT area
            memmove( files[ *entries ].file_name, dir.name, cchLength );
        }

        // kludge around (another!) Netware bug with subdirectory sizes
        files[ *entries ].file_size = (( dir.attrib & _A_SUBDIR ) ? 0L : dir.size );
        files[ *entries ].attribute = (char)dir.attrib;

        if ( nFlags & 2 )
            goto NextDirEntry;

        // get the date/time field to display
        if ( ( glDirFlags & DIRFLAGS_HPFS ) && ( gnDirTimeField == 1 ) ) {
            files[ *entries ].fd.ufDate = dir.fdLA.fdLAccess;
            files[ *entries ].ft.ufTime = dir.ftLA.ftLAccess;
        } else if ( ( glDirFlags & DIRFLAGS_HPFS ) && ( gnDirTimeField == 2) ) {
            files[ *entries ].fd.ufDate = dir.fdC.fdCreation;
            files[ *entries ].ft.ufTime = dir.ftC.ftCreation;
        } else {
            // save the FAT date & time
            files[*entries].fd.ufDate = dir.fdLW.fdLWrite;
            files[*entries].ft.ufTime = dir.ftLW.ftLWrite;
        }

        // size of the extended-attribute list
        //   (including the 4 bytes for cbList)
        // KLUDGE for bug in OS/2 2.0 32-bit APIs!
        files[ *entries ].ea_size = (( dir.cbList == 4L ) ? 0L : dir.cbList / 2 );

        files[*entries].file_mark = 0;
        files[*entries].color = -1;
        files[ *entries ].file_id = NULLSTR;

        // time - check for 12-hour format
        if ( gaCountryInfo.fsTimeFmt == 0 ) {

            if ( files[ *entries].ft.file_time.hours >= 12 ) {
                files[ *entries ].ft.file_time.hours -= 12;
                files[ *entries ].ampm = 'p';
            } else
                files[ *entries ].ampm = 'a';

            if ( files[ *entries ].ft.file_time.hours == 0 )
                files[ *entries ].ft.file_time.hours = 12;

        } else
            files[ *entries ].ampm = ' ';

        NextDirEntry:
        (*entries)++;
    }

    // shrink the LFN block
    if ( ( glDirFlags & DIRFLAGS_HPFS ) && ( hpHPFSNames != 0L ) ) {
        files[0].hpfs_base = ReallocMem( files[0].hpfs_base, (unsigned long)( hpHPFSNames - hpLFNStart ) + 2 );
    }

    if ( *entries > 0 ) {

        // get the file descriptions?
        if ( nFlags & 1 ) {

            // don't get descriptions on HPFS partitions - they
            //   already have long filenames
            if ( (( glDirFlags & DIRFLAGS_HPFS ) == 0 ) || ( glDirFlags & DIRFLAGS_HPFS_TO_FAT) || ( gszFindDesc[0] != '\0' ) ) {
                // did we run out of memory getting descriptions?
                if ( ( *hptr = GetDescriptions( *entries, files, arg )) == 0L )
                    return ERROR_EXIT;

                files = *hptr;

                // only match specified descriptions?
                if ( gszFindDesc[0] != '\0' ) {

                    for ( fval = 0; ( fval < *entries ); ) {

                        // remove non-matching entries
                        if ( wild_cmp( gszFindDesc, files[fval].file_id, FALSE, TRUE ) != 0 ) {

                            (*entries)--;
                            if ( fval >= *entries )
                                break;

                            memmove( &( files[fval].dummy_pad), &( files[fval+1].dummy_pad), sizeof(DIR_ENTRY) * ( *entries - fval) );

                        } else
                            fval++;
                    }
                }
            }
        }

        // unless /Ou (unsorted) specified, sort the dir array
        if ( gszSortSequence[0] != 'u' )
            ssort( (char *)*hptr, *entries );

        if ( nFlags & 4 ) {
            // check for file colorization (SET COLORDIR=...)
            ColorizeDirectory( files, *entries, 1 );
        }

        // restore base pointer to HPFS names
        files[0].hpfs_base = hpLFNStart;

        // COMMAND.COM formatting requested?
        if ( glDirFlags & DIRFLAGS_JUSTIFY ) {

            unsigned int i, k;

            for ( i = 0; ( i < *entries ); i++ ) {

                // patch filename for DOS-type justification
                if ( files[i].file_name[0] != '.' ) {

                    for ( k = 0; ( files[i].file_name[k] != '\0' ); k++ ) {

                        if ( files[i].file_name[k] == '.' ) {

                            // move extension & pad w/blanks
                            memmove( files[i].file_name+8, files[i].file_name+k, 4 );

                            for ( ; ( k < 9 ); k++ )
                                files[i].file_name[k] = ' ';
                            break;
                        }
                    }
                }

                // add termination for HPFS names
                files[i].file_name[12] = '\0';
            }
        }
    }

    return 0;
}


// read the description file (if any)
//   entries - number of files to match with description
//   files   - huge pointer to directory array
//   current - directory/file name
static DIR_ENTRY * GetDescriptions( unsigned int entries, DIR_ENTRY *files, char *current )
{
    unsigned int i, n;
    unsigned int cols, uLength, bytes_read;
    int hFH;
    long lFilesSize, lOffset = 0L;
    char dname[MAXFILENAME];
    char *read_buf, *fptr, *hptr, *fdesc;

    // get size before descriptions are added & add 4K for first block
    lFilesSize = (long)(entries + 1);
    lFilesSize = ((lFilesSize * sizeof( DIR_ENTRY)) + 4112L );

    hptr = (char *)&( files[ entries ] );

    // get screen width, unless STDOUT has been redirected
    cols = (( fConsole == 0 ) ? 0x7FFF : scr_columns - 40 );

    // adjust for /T (display attributes)
    if ( glDirFlags & DIRFLAGS_SHOW_ATTS )
        cols -= 6;

    // check DescriptionName for EA storage
    if ( stricmp( "ea", DESCRIPTION_FILE ) == 0 ) {

        for ( i = 0; ( i < entries ); i++ ) {

            if ( ( files = ( DIR_ENTRY *)ReallocMem( (char *)files, lFilesSize )) == 0L ) {
                error( ERROR_NOT_ENOUGH_MEMORY, NULL );
                return files;
            }

            uLength = 511;

            // read the descriptions ( from path if necessary)
            insert_path( dname, (( glDirFlags & DIRFLAGS_HPFS ) ? files[i].hpfs_name : files[i].file_name ), current );

            EAReadASCII( dname, SUBJECT_EA, hptr, (PINT)&uLength );
            if ( uLength == 0 )
                continue;

            // if uLength > screen width, truncate with a right arrow
            if ( ( glDirFlags & DIRFLAGS_TRUNCATE_DESCRIPTION ) && ( uLength > cols ) ) {
                hptr[cols-1] = gchRightArrow;
                hptr[cols] = '\0';
            }

            files[i].file_id = hptr;
            hptr += ++uLength;
        }

        return files;
    }

    // read the descriptions ( from path if necessary)
    insert_path( dname, DESCRIPTION_FILE, current );

    if ( ( hFH = _sopen( dname, (O_RDONLY | O_BINARY), SH_DENYWR )) < 0 )
        return files;
    HoldSignals();

    for ( ; ; ) {

        if ( ( files = ( DIR_ENTRY *)ReallocMem( (char *)files, lFilesSize )) == 0L ) {
            error( ERROR_NOT_ENOUGH_MEMORY, NULL );
            break;
        }

        read_buf = hptr;

        // read 4K blocks of the DESCRIPT.ION file
        if ( ( FileRead( hFH, read_buf, 4096, &bytes_read ) != 0 ) || ( bytes_read == 0 ) )
            break;

        // back up to the end of the last line & terminate there
        if ( ( i = bytes_read) == 4096 ) {
            for ( ; (( --i > 0 ) && ( read_buf[i] != '\n' ) && ( read_buf[i] != '\r' )); )
                ;
            i++;
        }
        read_buf[i] = '\0';

        // read a line & try for a match
        for ( fptr = read_buf; (( *fptr != '\0' ) && ( *fptr != EoF )); ) {

            // check for HPFS names (4DOS can see SOME of them!)
            if ( *fptr == '"' ) {

                if ( ( fdesc = strchr( ++fptr, '"' )) != 0L )
                    *fdesc++ = '\0';

            } else {

                // skip to the description part (kinda kludgy
                //   to avoid problems if no description)
                for ( fdesc = fptr; ; fdesc++ ) {

                    if ( ( *fdesc == ' ' ) || ( *fdesc == ',' ) || ( *fdesc == '\t' ) )
                        break;
                    if ( ( *fdesc == '\r' ) || ( *fdesc == '\n' ) || ( *fdesc == '\0' ) ) {
                        fdesc = 0L;
                        break;
                    }
                }
            }

            if ( fdesc != 0L ) {

                // wipe out space between filename & description
                *fdesc++ = '\0';

                for ( i = 0; ( i < entries ); i++ ) {

                    // try for a match
                    if ( glDirFlags & DIRFLAGS_HPFS ) {
                        n = stricmp( files[i].hpfs_name, fptr );

                    } else {
                        n = stricmp( files[i].file_name, fptr );
                    }

                    if ( n == 0 ) {

                        // save description into same
                        //   block by collapsing filename
                        sscanf( fdesc, "%*[ ,\t]%511F[^\r\n\004\032]%n", hptr, &uLength );

                        // if uLength > screen width,
                        //   truncate with a right arrow
                        if ( ( glDirFlags & DIRFLAGS_TRUNCATE_DESCRIPTION) && ( uLength > cols) ) {
                            hptr[ cols-1 ] = gchRightArrow;
                            hptr[ cols ] = '\0';
                        }

                        files[ i ].file_id = hptr;
                        fdesc += uLength++;
                        hptr += uLength;
                        break;
                    }
                }

                fptr = fdesc;
            }

            // skip the description & get next filename
            while ( ( *fptr != '\0' ) && ( *fptr++ != '\n' ) )
                ;
        }

        if ( bytes_read < 4096 )
            break;

        // seek to the end of the last line of the current block
        lOffset += (unsigned long)( fptr - read_buf );
        _lseek( hFH, lOffset, SEEK_SET );

        // next 4K block (allocating a bit less because we collapsed
        //   the previous block)
        lFilesSize += ((unsigned long)( hptr - read_buf )) + 1;
    }

    _close( hFH );

    EnableSignals();

    return files;
}

int _PrintGB(long long number)
{
    int rc = 0;

    if ( number > 10LL * 1024 * 1024 * 1024 ) {
        // more than 10GB
        printf(DIR_BYTES_GB, number / 1024 / 1024 / 1024);
        rc = 1;
    } else {
        if ( number > 10LL * 1024 * 1024 ) {
            // more than 10MB
            printf(DIR_BYTES_MB, number / 1024 / 1024 );
        }
        rc = 2;
    }
    return( rc );
}
