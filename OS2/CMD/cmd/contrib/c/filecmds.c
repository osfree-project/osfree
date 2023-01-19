// FILECMDS.C - File handling commands (COPY, MOVE, DEL, etc.) for 4xxx / TCMD
//   Copyright (c) 1988 - 1998 Rex C. Conn   All rights reserved

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <process.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <dos.h>
#include <malloc.h>
#include <share.h>
#include <string.h>

#include "4all.h"
#include "wrappers.h"

extern void UpdateFuzzyTree( char *, int );

static void wildname(char *, char *, char *);
static int FileCopy(char *, char *, unsigned int, LONGLONG *);

static int disk_full;
RANGES aRanges;                         // date/time/size ranges


// display the number of files copied/moved/renamed/deleted, with special
//   kludge for "1 file" vs. "2 files"
void FilesProcessed( char *szFormat, long lFiles )
{
    printf( szFormat, lFiles, (( lFiles == 1L ) ? ONE_FILE : MANY_FILES ));
}


// COPY_OPTIONS
#define COPY_BY_ATTRIBUTES 1
#define COPY_ASCII 2
#define COPY_BINARY 4
#define COPY_CHANGED 8
#define COPY_NOERRORS 0x10
#define COPY_HIDDEN 0x20
#define COPY_MODIFIED 0x40
#define COPY_NOTHING 0x80
#define COPY_QUERY 0x100
#define COPY_QUIET 0x200
#define COPY_REPLACE 0x400
#define COPY_SUBDIRS 0x800
#define COPY_TOTALS 0x1000
#define COPY_UPDATED 0x2000
#define COPY_VERIFY 0x4000
#define COPY_CLEAR_ARCHIVE 0x8000L
#define COPY_EAS 0x10000L
#define COPY_DOS62_Y 0x10000L
#define COPY_KEEP_ATTRIBUTES 0x20000L
#define COPY_OVERWRITE_RDONLY 0x40000L
#define COPY_CMDDIRS 0x80000L
#define COPY_EXISTS 0x100000L

// COPY_FLAGS
#define APPEND_FLAG 1
#define DEVICE_SOURCE 2
#define PIPE_SOURCE 4
#define DEVICE_TARGET 8
#define PIPE_TARGET 0x10
#define ASCII_SOURCE 0x20
#define BINARY_SOURCE 0x40
#define ASCII_TARGET 0x80
#define BINARY_TARGET 0x100
#define COPY_CREATE 0x200
#define MOVING_FILES 0x400
#define VERIFY_COPY 0x1000

typedef struct {
    char szSource[MAXFILENAME];
    char szSourceName[MAXFILENAME];
    char szTarget[MAXFILENAME];
    char szTargetName[MAXFILENAME];
    char *szFilename;
    char *pFirstCopyArg;            // pointer to first source arg in COPY
    long fOptions;
    unsigned int fFlags;
    long lFilesCopied;
    LONGLONG llAppendOffset;            // 20090828 AB LFS
    FILESEARCH t_dir;
} COPY_STRUCT;

static int _copy( COPY_STRUCT *);
static int fClip;

// copy or append file(s)
int copy_cmd( int argc, char **argv )
{
    char *arg;
    char szClip[MAXFILENAME], *tptr = NULL;
    int nTlen, rval = 0, nLength, fOldVerify;
    long lCopy;
    COPY_STRUCT Copy;

    Copy.lFilesCopied = Copy.llAppendOffset = 0L;
    Copy.fFlags = COPY_CREATE;
    Copy.fOptions = 0;

    disk_full = 0;

    // remove the white space around appends (+)
    collapse_whitespace( *(++argv), "+" );

    // get date/time/size ranges
    if ( GetRange( *argv, &aRanges, 0 ) != 0 )
        return ERROR_EXIT;

    gchExclusiveMode = gchInclusiveMode = 0;

    // scan command line, parsing for & removing switches
    for ( argc = 0; (( arg = ntharg( *argv, argc )) != NULL ); ) {

        // check for MS-DOS 6.2 /-Y option & convert it to a COPY /R
        if ( _stricmp( arg, "/-Y" ) == 0 )
            lCopy = COPY_REPLACE;
        else if ( ( lCopy = switch_arg( arg, "*ABCEHMNPQRSTUVXFKZJ" )) == -1 )
            return( usage( COPY_USAGE ));  // invalid switch

        Copy.fOptions |= lCopy;

        // leading ASCII (/A) or Binary (/B) switch?
        if ( argc == 0 ) {
            if ( lCopy & COPY_ASCII )
                Copy.fFlags |= ASCII_SOURCE;
            else if ( lCopy & COPY_BINARY )
                Copy.fFlags |= BINARY_SOURCE;
        }

        // remove leading /A & /B, & collapse spaces before
        //   trailing /A & /B
        if ( ( lCopy & ( COPY_ASCII | COPY_BINARY )) && ( argc > 0 ) ) {

            // strip leading whitespace
            while ( ( iswhite( *(--gpNthptr) )) && ( gpNthptr >= *argv ) )
                strcpy( gpNthptr, gpNthptr+1 );

        } else if ( *arg == gpIniptr->SwChr ) {

            // remove the switch
            strcpy( gpNthptr, skipspace( gpNthptr + strlen( arg )));
            continue;

        } else if ( argc > 0 ) {
            // save pointer to potential target
            tptr = gpNthptr;
        }

        argc++;
    }
    // check for flag to preserve all attributes (won't work w/Netware!)
    if ( Copy.fOptions & COPY_KEEP_ATTRIBUTES )
        Copy.fFlags |= MOVING_FILES;

    // if query, turn off quiet flag
    if ( Copy.fOptions & COPY_QUERY )
        Copy.fOptions &= ~( COPY_QUIET | COPY_TOTALS );

    // abort if no file arguments
    if ( first_arg( *argv ) == NULL )
        return( usage( COPY_USAGE ));

    // get the target name, or "*.*" for default name
    if ( tptr != NULL ) {

        copy_filename( Copy.szTarget, first_arg( tptr ));

        // check for ASCII (/A) or binary (/B) switches at end of line
        switch ( switch_arg( ntharg( tptr, 1 ), "AB" ) ) {
            case 1:
                Copy.fFlags |= ASCII_TARGET;
                break;
            case 2:
                Copy.fFlags |= BINARY_TARGET;
        }

        *tptr = '\0';           // remove target from command line

        // check target for device name
        if ( QueryIsDevice( Copy.szTarget ) ) {
            // set "target is device" flag
            Copy.fFlags |= DEVICE_TARGET;
        } else if ( QueryIsPipeName( Copy.szTarget ) ) {
            // set "target is device" flag
            Copy.fFlags |= PIPE_TARGET;
        }

        if ( Copy.fFlags & ( DEVICE_TARGET | PIPE_TARGET ) ) {
            // can't "replace" or "update" device or pipe!
            Copy.fOptions &= ~( COPY_REPLACE | COPY_UPDATED);
        }

    } else
        strcpy( Copy.szTarget, (( Copy.fOptions & COPY_SUBDIRS ) ? gcdir( NULL, 0 ) : WILD_FILE ));

    if ( Copy.fOptions & COPY_VERIFY ) {

        // save original VERIFY status
        fOldVerify = QueryVerifyWrite();

        // set VERIFY ON
        SetVerifyWrite( 1 );
    }

    if ( Copy.fOptions & COPY_EAS )
        Copy.fFlags |= COPY_EAS;

    if ( setjmp( cv.env ) == -1 ) {
        rval = CTRLC;
        goto copy_abort;
    }

    // if target not a char device or pipe, build full filename
    if ( ( Copy.fFlags & ( DEVICE_TARGET | PIPE_TARGET )) == 0 ) {

        if ( mkfname( Copy.szTarget, 0 ) == NULL )
            goto copy_abort;

        // don't build target name yet if copying subdirs
        if ( ( Copy.fOptions & COPY_SUBDIRS ) == 0 ) {
            if ( is_dir( Copy.szTarget ) )
                mkdirname( Copy.szTarget, WILD_FILE );
            else if ( Copy.szTarget[ strlen( Copy.szTarget ) - 1] == '\\' ) {
                rval = error( ERROR_PATH_NOT_FOUND, Copy.szTarget );
                goto copy_abort;
            }
        }
    }

    // check for appending
    arg = *argv;
    do {
        arg = scan( arg, "+", QUOTES + 1 );

        if ( *arg == '+' ) {

            // remove the '+' and set append flags
            *arg = ' ';

            // default for appending files is ASCII
            if ( ( Copy.fFlags & BINARY_SOURCE ) == 0 )
                Copy.fFlags |= ASCII_SOURCE;

            // can't append to a device or pipe!
            if ( ( Copy.fFlags & DEVICE_TARGET ) == 0 )
                Copy.fFlags |= APPEND_FLAG;
        }

    } while ( *arg );

    Copy.pFirstCopyArg = *argv;

    nTlen = strlen( Copy.szTarget );

    for ( rval = argc = 0; (( arg = ntharg( *argv, argc )) != NULL ); argc++ ) {

        // clear "source is device/pipe" flags
        Copy.fFlags &= ~( DEVICE_SOURCE | PIPE_SOURCE );

        if ( QueryIsDevice( arg ) )
            Copy.fFlags |= DEVICE_SOURCE;
        else if ( QueryIsPipeName( arg ) )
            Copy.fFlags |= PIPE_SOURCE;
        else {
            // if source is not a device, make a full filename
            if ( mkfname( arg, 0 ) == NULL )
                break;
            if ( is_dir( arg ) ) {
                mkdirname( arg, WILD_FILE );
                if ( (Copy.fOptions & COPY_CMDDIRS || gpIniptr->CMDDirMvCpy) &&
                    (!strstr( arg, ":\\*") || !strstr( arg, ":/*")) ) {
                    char *p;
                    PSZ temp = strdup(arg);
                    
                    p = Copy.szTarget + strlen(Copy.szTarget) - 1;
                    if ( ~Copy.fOptions & COPY_SUBDIRS || strchr(p, '\\') ||  strchr(p, '/')) {
                        p = strrchr(Copy.szTarget, '\\');
                        if ( p )
                            *p = '\0';
                        else {
                            p = strrchr(Copy.szTarget, '/');
                            if ( p )
                                *p = '\0';
                        }
                    }
                    p = strrchr(temp, '\\');
                    if ( p ) {
                        *p = '\0';
                        p = strrchr(temp, '\\');
                        if ( !p ) {
                            strcat(Copy.szTarget, "\\");
                            p = temp;
                        }
                    } else {
                        p = strrchr(temp, '/');
                        if ( p ) {
                            *p = '\0';
                            p = strrchr(temp, '/');
                            if ( !p ) {
                                strcat(Copy.szTarget, "/");
                                p = temp;
                            }
                        }
                    }
                    if ( p ) {
                        strcat(Copy.szTarget, p);
                        MakeDirectory( Copy.szTarget, 1 );
                        if ( ~Copy.fOptions & COPY_SUBDIRS )
                            mkdirname(Copy.szTarget, WILD_FILE );
                    } else {
                        free(temp);
                        return( error( ERROR_PATH_NOT_FOUND, Copy.szTarget ));
                    }
                    free(temp);
                }
            }
        }

        copy_filename( Copy.szSource, arg );

        // check for trailing ASCII (/A) or Binary (/B) switches
        if ( ( lCopy = switch_arg( ntharg( *argv, argc+1 ), "AB" )) > 0 ) {

            // remove the source ASCII or binary flags
            Copy.fFlags &= ~( ASCII_SOURCE | BINARY_SOURCE );
            Copy.fFlags |= (( lCopy == 1 ) ? ASCII_SOURCE : BINARY_SOURCE );
            argc++;
        }

        // check for dumdums doing an infinitely recursive COPY
        //   (i.e., "COPY /S . subdir" )
        nLength = ((( arg = path_part( Copy.szSource )) != NULL ) ? strlen( arg ) : 0 );
        if ( Copy.fOptions & COPY_SUBDIRS ) {
            if ( _strnicmp( arg, Copy.szTarget, nLength ) == 0 )
                return( error( ERROR_4DOS_INFINITE_COPY, Copy.szSource ));
        }

        // save the source filename part ( for recursive calls and
        //   include lists)
        Copy.szFilename = Copy.szSource + nLength;
        arg = _alloca( strlen( Copy.szFilename ) + 1 );
        Copy.szFilename = strcpy( arg, Copy.szFilename );

        szClip[0] = '\0';
        fClip = 0;
        if ( stricmp( Copy.szSource, CLIP ) == 0 ) {
            RedirToClip( szClip, 99 );
            if ( CopyFromClipboard( szClip ) != 0 )
                continue;
            strcpy( Copy.szSource, szClip );
            fClip = 1;
            Copy.fFlags &= ~DEVICE_SOURCE;
        } else if ( stricmp( Copy.szTarget, CLIP ) == 0 ) {
            RedirToClip( szClip, 99 );
            if ( Copy.fFlags & APPEND_FLAG ) {
                if ( CopyFromClipboard( szClip ) != 0 )
                    continue;
            }
            strcpy( Copy.szTarget, szClip );
            fClip = 2;
            Copy.fFlags &= ~DEVICE_TARGET;
        }

        rval = _copy( &Copy );

        if ( fClip & 1 )
            strcpy( Copy.szSource, CLIP );
        else if ( fClip & 2 ) {
            // copying to clipboard - get a handle to the temp
            //   file and use it to copy the file to the clip
            if ( ( fClip = _sopen( Copy.szTarget, O_RDONLY | O_BINARY, SH_DENYNO )) >= 0 ) {
                CopyToClipboard( fClip );
                _close( fClip );
            }
            strcpy( Copy.szTarget, CLIP );
        }

        if ( szClip[0] )
            remove( szClip );

        // restore original "target"
        if ( ( Copy.fFlags & APPEND_FLAG ) == 0 )
            Copy.szTarget[ nTlen ] = '\0';

        if ( ( setjmp( cv.env ) == -1 ) || ( cv.exception_flag ) || ( rval == CTRLC ) ) {
            rval = (( disk_full == 0 ) ? CTRLC : ERROR_EXIT );
            break;
        }
        EnableSignals();
    }

    copy_abort:

    // reset VERIFY flag to original state
    if ( Copy.fOptions & COPY_VERIFY )
        SetVerifyWrite( fOldVerify );

    if ( ( Copy.fOptions & COPY_QUIET ) == 0 )
        FilesProcessed( FILES_COPIED, Copy.lFilesCopied );

    // disable signal handling momentarily
    HoldSignals();

    return(( Copy.lFilesCopied != 0L ) ? rval : ERROR_EXIT );
}


static int _copy(COPY_STRUCT *Copy)
{
    int fval;
    int mode = FIND_CREATE, i, rval = 0;
    char *source_arg, *pszTargetArg;
    FILESEARCH dir;
    char *pszLFN;

    EnableSignals();

    // copy date/time/size range info
    memmove( &(dir.aRanges), &aRanges, sizeof(RANGES) );

    // get hidden & system files too
    if ( ( Copy->fOptions & COPY_BY_ATTRIBUTES ) || ( Copy->fOptions & COPY_HIDDEN) )
        mode |= 0x07;

    // if copying subdirectories, create the target (must be a directory!)
    if ( Copy->fOptions & COPY_SUBDIRS ) {

        // remove trailing '/' or '\'
        strip_trailing( Copy->szTarget+3, SLASHES );
        // insert Copy change
        if ( ( Copy->fOptions & COPY_NOTHING ) == 0 ) {

            // if we created the directory, then copy any
            //    existing description
            if ( ( Copy->fFlags & ( DEVICE_TARGET | PIPE_TARGET )) == 0 ) {

                MakeDirectory( Copy->szTarget, 1 );

                if ( is_dir( Copy->szTarget ) == 0 )
                    return( error( ERROR_PATH_NOT_FOUND, Copy->szTarget ));
            }
        }

        if ( ( Copy->fFlags & DEVICE_TARGET ) == 0 )
            mkdirname( Copy->szTarget, WILD_FILE );
    }

    // suppress error message from find_file
    if ( Copy->fOptions & COPY_NOERRORS )
        mode |= FIND_NOERRORS;  // 0x100

    for ( fval = FIND_FIRST; ; fval = FIND_NEXT ) {

        // if it's a device, only look for it once
        if ( Copy->fFlags & ( DEVICE_SOURCE | PIPE_SOURCE ) ) {

            if ( fval == FIND_NEXT )
                break;

            copy_filename( Copy->szSourceName, Copy->szSource );

        } else {

            if ( find_file( fval, Copy->szSource, (mode | FIND_BYATTS | FIND_DATERANGE ), &dir, Copy->szSourceName ) == NULL )
                break;

            if ( ( setjmp( cv.env ) == -1 ) || ( cv.exception_flag ) ) {
                (void)DosFindClose( dir.hdir );
                return CTRLC;
            }
        }

        // clear overwrite query flag
        Copy->fOptions &= ~COPY_EXISTS;

        // don't copy unmodified files
        if ( ( Copy->fOptions & COPY_MODIFIED ) && (( dir.attrib & _A_ARCH) == 0 ) )
            continue;

        // build the target name
        wildname( Copy->szTargetName, Copy->szTarget, Copy->szSourceName );

        // if changed (/C), only copy source if target is older
        // if update (/U), only copy the source if the target is
        //   older or doesn't exist
        // if replace (/R) prompt before overwriting existing file

        if ( ( Copy->fOptions & ( COPY_UPDATED | COPY_CHANGED | COPY_REPLACE )) &&
             ( find_file( FIND_FIRST, Copy->szTargetName, FIND_4OS2_STD, &(Copy->t_dir), NULL ) != NULL ) ) {
            if ( Copy->fOptions & ( COPY_UPDATED | COPY_CHANGED ) ) {
                if ( ( Copy->t_dir.fdLW.fdLWrite > dir.fdLW.fdLWrite ) || (( Copy->t_dir.fdLW.fdLWrite == dir.fdLW.fdLWrite ) && ( Copy->t_dir.ftLW.ftLWrite >= dir.ftLW.ftLWrite )) )
                    continue;
            }

            // if querying for replacement, don't ask if appending!
            if ( ( Copy->fOptions & COPY_REPLACE ) && ( Copy->fFlags & COPY_CREATE ) )
                Copy->fOptions |= COPY_EXISTS;

        } else if ( Copy->fOptions & COPY_CHANGED )
            continue;       // target doesn't exist

        // check for special conditions w/append
        //   (the source is the same as the target)
        if ( _stricmp( Copy->szSourceName, Copy->szTargetName ) == 0 ) {

            // if not the first arg, check to make sure the arg
            //   wasn't the target & hasn't been overwritten
            if ( ( Copy->fFlags & COPY_CREATE ) == 0 ) {
                // check for "copy all.lst + *.lst"
                if ( _stricmp( first_arg( Copy->pFirstCopyArg ), Copy->szSourceName ) != 0 )
                    rval = error( ERROR_4DOS_CONTENTS_LOST, Copy->szSourceName );
            }

            // if target file is first source file, ignore it
            if ( Copy->fFlags & APPEND_FLAG )
                goto next_copy;
        }
        // if copying from FAT->HPFS or HPFS->FAT, adjust target name
        //    (but only if ambiguous target specified!)
        pszLFN = NULLSTR;
        if ( ( pszTargetArg = fname_part( Copy->szTarget )) != NULL ) {
            strip_leading( pszTargetArg, "*?[]." );
            if ( *pszTargetArg == '\0' )
                pszLFN = MakeSFN( Copy->szSourceName, Copy->szTargetName );
        }

        if ( (( Copy->fOptions & ( COPY_QUIET | COPY_TOTALS )) == 0 ) || (( Copy->fOptions & COPY_REPLACE ) && ( Copy->fOptions & COPY_EXISTS )) ) {

            printf( "%s =>%s %s", (( fClip == 1 ) ? CLIP : Copy->szSourceName ), (( Copy->fFlags & COPY_CREATE ) ? NULLSTR : ">" ), (( fClip == 2 ) ? CLIP : Copy->szTargetName ));

            // query if /Q, or /R and target exists
            if ( Copy->fOptions & ( COPY_QUERY | COPY_EXISTS ) ) {

                if ( ( i = QueryInputChar((( Copy->fOptions & COPY_EXISTS ) ? REPLACE : NULLSTR ), YES_NO_REST )) == REST_CHAR )
                    Copy->fOptions &= ~( COPY_QUERY | COPY_REPLACE );
                else if ( i == NO_CHAR )
                    continue;
                else if ( i == ESCAPE )
                    break;
            } else
                crlf();
        }

        if ( ( Copy->fOptions & COPY_NOTHING ) == 0 ) {

            // if /Z, overwrite RDONLY files
            if ( Copy->fOptions & COPY_OVERWRITE_RDONLY )
                SetFileMode( Copy->szTargetName, 0 );

            if ( (( rval = FileCopy( Copy->szTargetName, Copy->szSourceName, Copy->fFlags, &( Copy->llAppendOffset ))) == CTRLC ) || ( cv.exception_flag ) || ( setjmp( cv.env ) == -1 ) ) {
                // reset the directory search handles
                (void)DosFindClose( dir.hdir );
                return CTRLC;
            }

            if ( rval == 0 ) {

                Copy->lFilesCopied++;

                if ( Copy->fOptions & COPY_CLEAR_ARCHIVE ) {
                    // clear the archive bit
                    SetFileMode( Copy->szSourceName, dir.attrib & 0xDF );
                }

                // copy an existing description to the new name
                if ( ( Copy->fFlags & ( APPEND_FLAG | DEVICE_SOURCE | DEVICE_TARGET | PIPE_SOURCE | PIPE_TARGET)) == 0 )
                    process_descriptions( Copy->szSourceName, Copy->szTargetName, DESCRIPTION_COPY );

                // if we truncated an LFN, save the original
                //   name to the new EA
                if ( *pszLFN != '\0' )
                    EAWriteASCII( Copy->szTargetName, LONGNAME_EA, pszLFN );
            }
        }

        next_copy:
        EnableSignals();
        // if target is unambiguous file, append next file(s)
        //   if appending to wildcard, use first target name
        if ( strpbrk( Copy->szTarget, WILD_CHARS ) == NULL ) {
            if ( ( Copy->fFlags & ( DEVICE_TARGET | PIPE_TARGET )) == 0 )
                Copy->fFlags |= APPEND_FLAG;
        } else if ( Copy->fFlags & APPEND_FLAG )
            copy_filename( Copy->szTarget, Copy->szTargetName );

        // if appending, clear the CREATE flag
        if ( Copy->fFlags & APPEND_FLAG )
            Copy->fFlags &= ~COPY_CREATE;
    }

    // copy subdirectories too?
    if ( Copy->fOptions & COPY_SUBDIRS ) {
        sprintf( Copy->szSource, FMT_DOUBLE_STR, path_part( Copy->szSource), WILD_FILE );

        // save the current source filename start
        source_arg = strchr( Copy->szSource, '*' );

        // save the current target filename start
        if ( ( Copy->fFlags & ( DEVICE_TARGET | PIPE_TARGET )) == 0 )
            pszTargetArg = Copy->szTarget + strlen( path_part( Copy->szTarget ));

        // search for all subdirectories in this (sub)directory
        //   tell find_file() not to display errors, and to return
        //   only subdirectory names
#define M ( FIND_NOERRORS | FIND_DIRONLY | FILE_DIRECTORY )      // 0x310
        for ( fval = FIND_FIRST;
            ( find_file( fval, Copy->szSource, (mode | M ), &dir, NULL ) != NULL );
            fval = FIND_NEXT ) {
#undef M
            // make the new "source" and "target"
            sprintf( source_arg, FMT_PATH_STR, dir.name, Copy->szFilename );
            // strip the filename part from the target
            if ( ( Copy->fFlags & ( DEVICE_TARGET | PIPE_TARGET )) == 0 )
                copy_filename( pszTargetArg, dir.name );

            // process directory tree recursively
            rval = _copy( Copy );
            if ( ( setjmp( cv.env ) == -1 ) || ( cv.exception_flag ) ) {
                (void)DosFindClose( dir.hdir );
                return CTRLC;
            }

            // restore the original name
            strcpy( source_arg, WILD_FILE );
        }
    }

    // disable signal handling momentarily
    HoldSignals();

    return rval;
}


#define MOVE_BY_ATTRIBUTES 1
#define MOVE_CHANGED 2
#define MOVE_TO_DIR 4
#define MOVE_NOERRORS 8
#define MOVE_FORCEDEL 0x10
#define MOVE_HIDDEN 0x20
#define MOVE_MODIFIED 0x40
#define MOVE_NOTHING 0x80
#define MOVE_QUERY 0x100
#define MOVE_QUIET 0x200
#define MOVE_REPLACE 0x400
#define MOVE_SUBDIRS 0x800
#define MOVE_TOTALS 0x1000
#define MOVE_UPDATED 0x2000
#define MOVE_VERIFY 0x4000
#define MOVE_CMDDIR 0x8000
#define MOVE_SOURCEISDIR 0x10000L
#define MOVE_EXISTS 0x20000L

typedef struct {
    char szSource[MAXFILENAME];
    char szSourceName[MAXFILENAME];
    char szTarget[MAXFILENAME];
    char szTargetName[MAXFILENAME];
    char *szFilename;
    long fFlags;
    long lFilesMoved;
    FILESEARCH t_dir;
    char szDescription[512];
} MOVE_STRUCT;

static int _mv( MOVE_STRUCT *);

// move files by renaming across directories or copying/deleting
int mv_cmd(int argc, char **argv)
{
    char *arg;
    unsigned int fTargetIsDir, fFreed;
    int nTlen, rval = 0, nLength, fOldVerify;
    QDISKINFO DiskInfo;
    MOVE_STRUCT Move;
    long long lBytesFreed = 0;
    Move.lFilesMoved = 0L;
    disk_full = 0;
    fClip = 0;

    // get date/time/size ranges
    if ( GetRange( argv[1], &aRanges, 0 ) != 0 )
        return ERROR_EXIT;

    // check for and remove switches
    // abort if no filename arguments
    if ( ( GetSwitches( argv[1], "*CDEFHMNPQRSTUVJ", &(Move.fFlags), 0 ) != 0 ) || ( first_arg( argv[1]) == NULL ) )
        return(usage( MOVE_USAGE ));

    // if query before moving, disable MOVE_QUIET & MOVE_TOTALS
    if ( Move.fFlags & MOVE_QUERY )
        Move.fFlags &= ~( MOVE_QUIET | MOVE_TOTALS );

    // get the target name, or "*.*" for default name
    if ( ( arg = last_arg( argv[1], &argc )) != NULL ) {
        copy_filename( Move.szTarget, arg );
        *gpNthptr = '\0';               // remove target from command line
    } else {
        // turn off "create directory" flag - if no target is specified,
        //   files are being moved to the current (existing!) directory
        Move.fFlags &= ~MOVE_TO_DIR;

        strcpy( Move.szTarget, ((( Move.fFlags & MOVE_SUBDIRS ) || ( Move.fFlags & MOVE_CMDDIR )) ? gcdir( NULL, 0 ) : WILD_FILE ));
    }

    // can't move a file to a device or pipe!
    if ( ( QueryIsDevice( Move.szTarget )) || ( QueryIsPipeName( Move.szTarget )) )
        return( error( ERROR_ACCESS_DENIED, Move.szTarget ));

    if ( mkfname( Move.szTarget, 0 ) == NULL )
        return ERROR_EXIT;

    // is the target a directory?
    if ( ( fTargetIsDir = is_dir( Move.szTarget )) == 0 ) {

        if ( Move.fFlags & MOVE_TO_DIR ) {

            // if /D, target MUST be directory, so prompt to create it
            printf( MOVE_CREATE_DIR, Move.szTarget );
            if ( QueryInputChar( NULLSTR, YES_NO ) != YES_CHAR )
                return ERROR_EXIT;

            // if > 1 source, target must be dir!
        } else if ( ( argc > 1 ) || ( Move.szTarget[ strlen( Move.szTarget)-1] == '\\' ) )
            return( error( ERROR_PATH_NOT_FOUND, Move.szTarget ));
    }

    nTlen = strlen( Move.szTarget );

    if ( Move.fFlags & MOVE_VERIFY ) {

        // save original VERIFY status
        fOldVerify = QueryVerifyWrite();

        // set VERIFY ON
        SetVerifyWrite( 1 );
    }

    for ( argc = 0; (( arg = ntharg( argv[1], argc )) != NULL ); argc++ ) {

        if ( mkfname( arg, 0 ) == NULL ) {
            rval = ERROR_EXIT;
            break;
        }

        Move.fFlags &= ~MOVE_SOURCEISDIR;
        if ( is_dir( arg ) ) {

            // check to see if it's just a directory rename in the
            //   same path
            copy_filename( Move.szSource, path_part( arg ) );
            if ( ( _stricmp( Move.szSource, path_part( Move.szTarget) ) == 0 ) && (is_file( Move.szTarget) == 0 ) && ( fTargetIsDir == 0 ) ) {
                if ( rename( arg, Move.szTarget ) == 0 ) {
                    if ( stricmp( "EA", DESCRIPTION_FILE ) != 0 )
                        process_descriptions( arg, Move.szTarget, ( DESCRIPTION_COPY | DESCRIPTION_REMOVE ) );
                    Move.lFilesMoved++;
                    continue;
                }
            }

            // get entire contents of subdirectory
            mkdirname( arg, WILD_FILE );
            Move.fFlags |= MOVE_SOURCEISDIR;
        }

        copy_filename( Move.szSource, arg );

        // check for dumdums doing an infinitely recursive MOVE
        //   (i.e., "MOVE /S . subdir" )
        arg = path_part( Move.szSource );
        nLength = strlen( arg );
        if ( (Move.fFlags & MOVE_SUBDIRS) /*|| ( Move.fFlags & MOVE_CMDDIR )*/) {
            if ( _strnicmp( arg, Move.szTarget, nLength ) == 0 ) {
                rval = error( ERROR_4DOS_INFINITE_MOVE, Move.szSource );
                break;
            }
        }

        // save the source filename part ( for recursive calls and
        //   include lists)
        Move.szFilename = Move.szSource + nLength;
        arg = _alloca( strlen( Move.szFilename ) + 1 );
        Move.szFilename = strcpy( arg, Move.szFilename );

        fFreed = 0;
        if ( ( strnicmp( Move.szSource, Move.szTarget, 2 ) != 0 ) && ( is_net_drive( Move.szSource ) == 0 ) && ( QueryDriveRemote( gcdisk( Move.szSource )) == 0 ) ) {
            fFreed = 1;
            (void)QueryDiskInfo( Move.szSource, &DiskInfo, 1 );
            lBytesFreed -= DiskInfo.BytesFree;
        }

        rval = _mv( &Move );

        // restore original target name
        Move.szTarget[ nTlen ] = '\0';

        if ( ( setjmp( cv.env ) == -1 ) || ( rval == CTRLC ) ) {
            rval = (( disk_full == 0 ) ? CTRLC : ERROR_EXIT );
            break;
        }
        EnableSignals();
        if ( fFreed ) {
            (void)QueryDiskInfo( Move.szSource, &DiskInfo, 1 );
            lBytesFreed += DiskInfo.BytesFree;
        }
    }

    // reset VERIFY flag to original state
    if ( Move.fFlags & MOVE_VERIFY )
        SetVerifyWrite( fOldVerify );

    if ( ( Move.fFlags & MOVE_QUIET) == 0 ) {
        FilesProcessed( FILES_MOVED, Move.lFilesMoved );
        if ( ( lBytesFreed > 0L ) && ( Move.lFilesMoved > 0L ) )
            printf( FILES_BYTES_FREED, lBytesFreed );
        crlf();
    }

    // disable signal handling momentarily
    HoldSignals();

    return(( Move.lFilesMoved != 0L ) ? rval : ERROR_EXIT );
}


static int _mv(MOVE_STRUCT *Move)
{
    int fval;
    int rval = 0;
    unsigned int i, mode = FIND_CREATE, uRDONLY;
    unsigned int fCopyFlags = COPY_CREATE | MOVING_FILES;
    char *arg, *source_arg, *pszTargetArg;
    LONGLONG llAppendOffset = 0LL;            // 20090828 AB LFS
    FILESEARCH dir;
    char *pszLFN;

    EnableSignals();
    dir.hdir = INVALID_HANDLE_VALUE;
    if ( ( setjmp( cv.env ) == -1 ) || ( cv.exception_flag ) ) {
        (void)DosFindClose( dir.hdir );
        return CTRLC;
    }

    // copy date/time range info
    memmove( &(dir.aRanges), &aRanges, sizeof(RANGES) );

    // get hidden & system files too
    if ( ( Move->fFlags & MOVE_BY_ATTRIBUTES ) || ( Move->fFlags & MOVE_HIDDEN) )
        mode |= 0x07;
    // if the target is a directory, add a wildcard filename

    if ( is_dir( Move->szTarget ) ) {
        if ( ((~Move->fFlags & MOVE_CMDDIR)  && !gpIniptr->CMDDirMvCpy) ||
            ( (Move->szSource[1] == ':') && ( Move->szSource[3] == '*') ) || (~Move->fFlags & MOVE_SOURCEISDIR)) {
            mkdirname( Move->szTarget, WILD_FILE );
        }
        else { // Copy change
            char *p;
            PSZ temp = strdup(Move->szSource);
    
            p = strrchr(temp, '\\');
            if ( p ) {
                *p = '\0';
                p = strrchr(temp, '\\');
                if ( !p ) {
                    strcat(Move->szTarget, "\\");
                    p = temp;
                }
            } else {
                p = strrchr(temp, '/');
                if ( p ) {
                    *p = '\0';
                    p = strrchr(temp, '/');
                    if ( !p ) {
                        strcat(Move->szTarget, "/");
                        p = temp;
                    }
                }
            }
            if ( p ) {
                strcat(Move->szTarget, p);
            } else {
                free(temp);
                return( error( ERROR_PATH_NOT_FOUND, Move->szTarget ));
            }
            free(temp);
            // if moving subdirectories, create the target
    
            // remove trailing '/' or '\' in target
            strip_trailing( Move->szTarget+3, SLASHES );
    
            MakeDirectory( Move->szTarget, 1 );
            if ( is_dir( Move->szTarget ) == 0 )
                return( error( ERROR_PATH_NOT_FOUND, Move->szTarget ));
    
            // move description to newly-created target
            if ( Move->fFlags & MOVE_SOURCEISDIR ) {
                copy_filename( Move->szSourceName, path_part( Move->szSource) );
                strip_trailing( Move->szSourceName+3, SLASHES );
                process_descriptions( Move->szSourceName, Move->szTarget, DESCRIPTION_COPY );
            }
    
            mkdirname( Move->szTarget, WILD_FILE );
        }
    }
    if (( Move->fFlags & MOVE_SUBDIRS ) || ( Move->fFlags & MOVE_TO_DIR )) {

        // if moving subdirectories, create the target

        // remove trailing '/' or '\' in target
        strip_trailing( Move->szTarget+3, SLASHES );

        MakeDirectory( Move->szTarget, 1 );
        if ( is_dir( Move->szTarget ) == 0 )
            return( error( ERROR_PATH_NOT_FOUND, Move->szTarget ));

        // move description to newly-created target
        if ( Move->fFlags & MOVE_SOURCEISDIR ) {
            copy_filename( Move->szSourceName, path_part( Move->szSource) );
            strip_trailing( Move->szSourceName+3, SLASHES );
            process_descriptions( Move->szSourceName, Move->szTarget, DESCRIPTION_COPY );
        }

        mkdirname( Move->szTarget, WILD_FILE );
    }
    // suppress error message from find_file
    if ( Move->fFlags & MOVE_NOERRORS )
        mode |= FIND_NOERRORS;  // 0x100

    // expand the wildcards
    for ( fval = FIND_FIRST; ; fval = FIND_NEXT ) {

        if ( find_file( fval, Move->szSource, (mode | FIND_BYATTS | FIND_DATERANGE ), &dir, Move->szSourceName ) == NULL )
            break;

        if ( ( setjmp( cv.env ) == -1 ) || ( cv.exception_flag ) ) {
            (void)DosFindClose( dir.hdir );
            return CTRLC;
        }

        // prevent destructive moves like "mv *.* test.fil"
        if ( ( strpbrk( Move->szTarget, WILD_CHARS ) == NULL ) && ( fval == FIND_NEXT) )
            return( error( ERROR_4DOS_CANT_CREATE, Move->szTarget ));

        // don't move unmodified files?
        if ( ( Move->fFlags & MOVE_MODIFIED ) && (( dir.attrib & _A_ARCH ) == 0 ) )
            continue;

        // make the target name
        wildname( Move->szTargetName, Move->szTarget, Move->szSourceName );

        // clear the EXISTS flag
        Move->fFlags &= ~MOVE_EXISTS;

        // if changed (/C), only move source if target is older
        // if update (/U), only move the source if the target is
        //   older or doesn't exist
        // if replace (/R) prompt before overwriting existing file

        if ( ( Move->fFlags & ( MOVE_UPDATED | MOVE_CHANGED | MOVE_REPLACE )) &&
             ( find_file( FIND_FIRST, Move->szTargetName, FIND_4OS2_STD, &( Move->t_dir), NULL ) != NULL ) ) {

            if ( Move->fFlags & ( MOVE_UPDATED | MOVE_CHANGED) ) {
                if ( ( Move->t_dir.fdLW.fdLWrite > dir.fdLW.fdLWrite) || (( Move->t_dir.fdLW.fdLWrite == dir.fdLW.fdLWrite) && ( Move->t_dir.ftLW.ftLWrite >= dir.ftLW.ftLWrite)) )
                    continue;
            }

            if ( Move->fFlags & MOVE_REPLACE )
                Move->fFlags |= MOVE_EXISTS;

        } else if ( Move->fFlags & MOVE_CHANGED )
            continue;       // target doesn't exist

        // check for same name
        if ( _stricmp( Move->szSourceName, Move->szTargetName ) == 0 )
            return( error( ERROR_4DOS_DUP_COPY, Move->szSourceName ));

        // if moving from FAT->HPFS or HPFS->FAT, adjust target name
        //    (but only if ambiguous target specified!)
        pszLFN = NULLSTR;
        if ( ( pszTargetArg = fname_part( Move->szTarget )) != NULL ) {
            strip_leading( pszTargetArg, "*?[]." );
            if ( *pszTargetArg == '\0' )
                pszLFN = MakeSFN( Move->szSourceName, Move->szTargetName );
        }

        if ( (( Move->fFlags & ( MOVE_QUIET | MOVE_TOTALS )) == 0 ) || ( Move->fFlags & MOVE_EXISTS ) ) {

            printf( FMT_STR_TO_STR, Move->szSourceName, Move->szTargetName );
            if ( Move->fFlags & ( MOVE_QUERY | MOVE_EXISTS ) ) {
                if ( ( i = QueryInputChar((( Move->fFlags & MOVE_EXISTS ) ? REPLACE : NULLSTR ), YES_NO_REST )) == REST_CHAR )
                    Move->fFlags &= ~( MOVE_QUERY | MOVE_REPLACE );
                else if ( i == NO_CHAR )
                    continue;
                else if ( i == ESCAPE )
                    break;
            } else
                crlf();
        }

        if ( ( Move->fFlags & MOVE_NOTHING ) == 0 ) {
            // we have to do this in two steps in OS/2 in case
            //   they're using EA's - otherwise, the EA for the
            //   source will be gone when we try to write the
            //   target EA!
            Move->szDescription[0] = '\0';
            process_descriptions( Move->szSourceName, Move->szDescription, DESCRIPTION_READ );
            // if source is RDONLY, remove the bit (Netware refuses
            //   to rename a RDONLY file)
            if ( dir.attrib & _A_RDONLY ) {
                (void)SetFileMode( Move->szSourceName, (dir.attrib & ~_A_RDONLY) );
                uRDONLY = 1;
            } else
                uRDONLY = 0;

            // try a simple rename first (except if the MOVE is
            //  to a different drive)
            rval = -1;
            if ( _ctoupper( *Move->szSourceName ) == _ctoupper( *Move->szTargetName ) ) {

                if ( ( rval = rename( Move->szSourceName, Move->szTargetName )) != 0 ) {
                    // rename didn't work; try deleting target
                    remove( Move->szTargetName );
                    rval = rename( Move->szSourceName, Move->szTargetName );
                }
            }

            if ( rval != 0 ) {

                // rename didn't work; try a destructive copy
                if ( (( rval = FileCopy( Move->szTargetName, Move->szSourceName, fCopyFlags, &llAppendOffset )) == CTRLC ) || ( setjmp( cv.env ) == -1 ) ) {

                    // reset the directory search handle
                    (void)DosFindClose( dir.hdir );
                    SetFileMode( Move->szSourceName, dir.attrib );
                    return CTRLC;
                }

                if ( rval != 0 ) {
                    // reset original attributes on error
                    SetFileMode( Move->szSourceName, dir.attrib );
                    continue;
                }

                // if we truncated a LFN, save the original
                //   name to the new EA
                if ( *pszLFN != '\0' )
                    EAWriteASCII( Move->szTargetName, LONGNAME_EA, pszLFN );

                // rub out read-only, hidden, and system files
                if ( dir.attrib & ( _A_RDONLY | _A_HIDDEN | _A_SYSTEM ) )
                    (void)SetFileMode( Move->szSourceName, 0 );

                // file copied OK, so remove the original
                _doserrno = 0;

                // support for DEL /F(orce) in OS/2
                if ( Move->fFlags & MOVE_FORCEDEL )
                    _doserrno = DosForceDelete( Move->szSourceName );
                else
                    remove( Move->szSourceName );

                if ( ( _doserrno != 0 ) && (( Move->fFlags & MOVE_NOERRORS ) == 0 ) )
                    error( _doserrno, Move->szSourceName );
            }

            // if source is RDONLY, restore the bit in the target
            if ( uRDONLY )
                (void)SetFileMode( Move->szTargetName, dir.attrib );

            // increment # of files moved counter
            Move->lFilesMoved++;

            // move description to target
            process_descriptions( Move->szTargetName, Move->szDescription, DESCRIPTION_WRITE | DESCRIPTION_CREATE );
        }
    }

    // build wildcard source for directory search
    sprintf( Move->szSource, FMT_DOUBLE_STR, path_part( Move->szSource ), WILD_FILE );

    // save the current target filename start
    pszTargetArg = Move->szTarget + strlen( path_part( Move->szTarget ));

    // update the source descript.ion file by deleting descriptions
    //   for any moved or missing files
    if ( (( Move->fFlags & MOVE_NOTHING ) == 0 ) && (( Move->lFilesMoved != 0L ) ||
                                                     ( Move->fFlags & MOVE_SUBDIRS ) ||
                                                     ( Move->fFlags & MOVE_CMDDIR )) )
        process_descriptions( NULL, Move->szSource, DESCRIPTION_REMOVE );

    // move subdirectories too?
    if ( ( Move->fFlags & MOVE_SUBDIRS ) /*|| ( Move->fFlags & MOVE_CMDDIR )*/) {
        // save the current source filename start
        source_arg = strchr( Move->szSource, '*' );

        // search for all subdirectories in this (sub)directory
        //   tell find_file() not to display errors, and to return
        //   only subdirectory names
#define M ( FIND_NOERRORS | FIND_DIRONLY | FILE_DIRECTORY )      // 0x310
        for ( fval = FIND_FIRST; ( find_file( fval, Move->szSource, (mode | M ),&dir,NULL ) != NULL ); fval = FIND_NEXT ) {
#undef M
            // strip the filename part from the target
            //   and make the new "source" and "target"
            sprintf( source_arg, FMT_PATH_STR, dir.name, Move->szFilename );
            copy_filename( pszTargetArg, dir.name );

            // process directory tree recursively
            Move->fFlags |= MOVE_SOURCEISDIR;
            rval = _mv( Move );
            if ( ( setjmp( cv.env ) == -1 ) || ( cv.exception_flag ) || ( rval == CTRLC) ) {
                (void)DosFindClose( dir.hdir );
                return CTRLC;
            }

            // restore the original name
            strcpy( source_arg, WILD_FILE );

            if ( rval != 0 )
                break;
        }
    // try to delete the subdirectory (unless it's the root or
    //   the current dir for that drive)
        if ( ( Move->fFlags & MOVE_NOTHING ) == 0 ) {
    
            // strip name and trailing '\'
            source_arg[-1] = '\0';
    
            if ( (( arg = gcdir( Move->szSource, 1 )) != NULL ) && ( strlen( Move->szSource ) > 3 ) &&
                ( _stricmp( Move->szSource, arg ) != 0 ) ) {
                // remove the directory & its description
                SetFileMode( Move->szSource, _A_SUBDIR );
                DestroyDirectory( Move->szSource );
                process_descriptions( NULL, Move->szSource, DESCRIPTION_REMOVE );
            } else {
                // can't use DestroyDirectory if move was on same drive!
                UpdateFuzzyTree( Move->szSource, 1 );
            }
        }
    }
    if ( ( Move->fFlags & MOVE_CMDDIR ) ) {
                // remove the directory & its description
                SetFileMode( Move->szSource, _A_SUBDIR );
                DestroyDirectory( Move->szSource );
                process_descriptions( NULL, Move->szSource, DESCRIPTION_REMOVE );

    }
    // set "target" back to original value
    *pszTargetArg = '\0';

    // disable signal handling momentarily
    HoldSignals();

    return rval;
}


// copy from the source to the target files
//   szOutputName = target file name
//   szInputName = source file name
//   fFlags = create/append, device & ASCII/binary flags
//   llAppendOffset = current end offset in append file
static int FileCopy( char *szOutputName, char *szInputName, unsigned int fFlags, LONGLONG *llAppendOffset )
{
    int rval;
    unsigned int fEoF, uBufferSize;
    int nInputFile = 0, nOutputFile = 0;
    char *segptr, *fptr;
    unsigned int attribute = 0, nBytesRead, nBytesWritten;

    // check for same name
    if ( _stricmp( szInputName, szOutputName ) == 0 ) {
        rval = error( ERROR_4DOS_DUP_COPY, szInputName );
        goto FileCopyExit;
    }

    // if source has type but target doesn't, set target to source type
    // if target has type but source doesn't, set source to target type
    if ( ( rval = fFlags & ( ASCII_SOURCE | BINARY_SOURCE | ASCII_TARGET | BINARY_TARGET)) == ASCII_SOURCE )
        fFlags |= ASCII_TARGET;
    else if ( rval == BINARY_SOURCE )
        fFlags |= BINARY_TARGET;
    else if ( rval == ASCII_TARGET )
        fFlags |= ASCII_SOURCE;
    else if ( rval == BINARY_TARGET )
        fFlags |= BINARY_SOURCE;

    // devices & pipes default to ASCII copy
    if ( ( fFlags & ( DEVICE_SOURCE | PIPE_SOURCE )) && (( fFlags & BINARY_SOURCE ) == 0 ) )
        fFlags |= ASCII_SOURCE;

    if ( ( fFlags & COPY_CREATE ) && (( fFlags & ( ASCII_SOURCE | ASCII_TARGET | DEVICE_SOURCE | DEVICE_TARGET | PIPE_SOURCE | PIPE_TARGET )) == 0 ) && ( _stricmp( fname_part( szInputName ), "con" ) != 0 ) ) {
        // Use internal DosCopy() API so EAs will be copied properly
        HoldSignals();
        TRACE("filecopy");
        rval = DosCopy( szInputName, szOutputName, (( fFlags & COPY_EAS ) ? 5 : 1 ) );
        EnableSignals();

        if ( rval == 0 ) {
            // strip wacky Netware bits & RDONLY bit (unless moving files)
            (void)QueryFileMode( szInputName, &attribute );
            attribute &= (( fFlags & MOVING_FILES ) ? 0x27 : 0x26 );

            // set the target file attributes
            (void)SetFileMode( szOutputName, ( attribute | _A_ARCH) );
        }
        // don't treat an EA "not supported" error as an error!
        if ( (( fFlags & COPY_EAS ) == 0 ) && ( rval >= ERROR_CREATE_EA_FILE ) && ( rval <= ERROR_EAS_NOT_SUPPORTED ) )
            rval = 0;

        rval = (( rval != 0 ) ? error( rval, szOutputName ) : rval );
        goto FileCopyExit;
    }

    // request memory block
    uBufferSize = 0x40000;          // 256K for OS/2 2.x
    if ( ( segptr = AllocMem( &uBufferSize )) == 0L ) {
        rval = error( ERROR_NOT_ENOUGH_MEMORY, NULL );
        goto FileCopyExit;
    }

    if ( setjmp( cv.env ) == -1 ) {         // ^C trapping
        rval = CTRLC;
        goto filebye;
    }

    // open input file for read
    if ( ( nInputFile = _sopen( szInputName, (O_RDONLY | O_BINARY), SH_DENYNO )) < 0 ) {
        rval = error( _doserrno, szInputName );
        goto filebye;
    }

    rval = 0;

    // if source not a device, copy source attributes to target
    if ( ( fFlags & ( DEVICE_SOURCE | PIPE_SOURCE )) == 0 ) {

        (void)QueryFileMode( szInputName, &attribute );

        // strip wacky Netware bits & RDONLY bit (unless moving files)
        attribute &= (( fFlags & MOVING_FILES ) ? 0x27 : 0x26 );
    }

    // if we are appending, open an existing file
    TRACE("append to existing file");
    if ( ( nOutputFile = _sopen( szOutputName, (( fFlags & COPY_CREATE ) ? O_WRONLY | O_BINARY | O_CREAT | O_TRUNC : O_RDWR | O_BINARY), (( fFlags & ( DEVICE_TARGET | PIPE_TARGET)) ? SH_DENYNO : SH_DENYRW), ( S_IREAD | S_IWRITE ) )) < 0 ) {
        rval = error( _doserrno, szOutputName );
        goto filebye;
    }

    if ( fFlags & ( DEVICE_TARGET | PIPE_TARGET ) ) {

        // can't append to device
        fFlags |= COPY_CREATE;

        // char devices & pipes default to ASCII copy
        if ( ( fFlags & ( BINARY_SOURCE | BINARY_TARGET )) == 0 )
            fFlags |= ( ASCII_TARGET | ASCII_SOURCE );
    }

    if ( ( fFlags & COPY_CREATE ) == 0 ) {

        // append at EOF of the target file (we have to kludge a bit
        //   here, because some files end in ^Z and some don't, & we
        //   may be in binary mode)

        // if continuing an append, move to saved EOF in target
        if ( *llAppendOffset != 0LL )
            xlseek( nOutputFile, *llAppendOffset, SEEK_SET );
        else if ( fFlags & BINARY_SOURCE ) {
            *llAppendOffset = QuerySeekSizeLL( nOutputFile );
            TRACE("current filesize %lld, errno=%d", *llAppendOffset, errno);
        } else {

            // We should only get here if the source is ASCII, and
            //   the target is the first source.  Read the file until
            //   EOF or we find a ^Z.
            do {

                if ( ( rval = FileRead( nOutputFile, segptr, uBufferSize, &nBytesRead )) != 0 ) {
                    rval = error( rval, szOutputName );
                    goto filebye;
                }

                // search for a ^Z
                if ( ( fptr = memchr( segptr, 0x1A, nBytesRead )) != 0L ) {

                    // backup to ^Z location
                    *llAppendOffset += ((unsigned long)fptr - (unsigned long)segptr);
                    xlseek( nOutputFile, *llAppendOffset, SEEK_SET );
                    break;
                }

                //TRACE("adding %d bytes", nBytesRead);
                *llAppendOffset += nBytesRead;

            } while ( nBytesRead == uBufferSize );
        }
    }

    for ( fEoF = 0; (( fEoF == 0 ) && ( cv.exception_flag == 0 )); ) {
        if ( ( rval = FileRead( nInputFile, segptr, uBufferSize, &nBytesRead )) != 0 ) {
            rval = error( rval, szInputName );
            goto filebye;
        }
        if ( fFlags & ASCII_SOURCE ) {

            // terminate the file copy at a ^Z
            if ( ( fptr = memchr( segptr, 0x1A, nBytesRead )) != 0L ) {
                nBytesRead = (unsigned int)((unsigned long)fptr - (unsigned long)segptr);
                fEoF++;
            }
        }

        if ( nBytesRead == 0 )
            fEoF++;

        //TRACE(".adding %d bytes", nBytesRead);
        *llAppendOffset += nBytesRead;

        // if target is ASCII, add a ^Z at EOF and set "fEoF" to
        //   force a break after the write
        if ( ( fFlags & ASCII_TARGET ) && (( fEoF ) || (( nBytesRead < uBufferSize) && (( fFlags & DEVICE_SOURCE ) == 0 ))) ) {
            if ( ( fFlags & DEVICE_TARGET ) == 0 )
                segptr[ nBytesRead++ ] = 0x1A;
            fEoF++;
        }

        if ( ( rval = FileWrite( nOutputFile, segptr, nBytesRead, &nBytesWritten )) != 0 ) {
            rval = error( rval, szOutputName );
            goto filebye;
        }

        if ( nBytesRead == 0 )
            break;

        if ( nBytesRead != nBytesWritten ) {    // error in writing

            // if writing to a device, don't expect full blocks
            if ( ( fFlags & DEVICE_TARGET ) == 0 ) {

                error( ERROR_4DOS_DISK_FULL, szOutputName );

                disk_full = 1;
                rval = CTRLC;
            }

            goto filebye;
        }
    }

    filebye:
    FreeMem( segptr );

    if ( nInputFile > 0 )
        _close( nInputFile );

    if ( nOutputFile > 0 ) {
        _close( nOutputFile );

        // if an error writing to the output file, delete it
        //   (unless we're appending
        if ( ( rval ) && ( fFlags & COPY_CREATE ) ) {
            TRACE("error in writting output file, rval=0x%X", rval);
            SetFileMode( szOutputName, _A_NORMAL );
            remove( szOutputName );
        } else {
            // set the target file attributes
            (void)SetFileMode( szOutputName, ( attribute | _A_ARCH ) );
        }
    }

    FileCopyExit:
    // disable signal handling momentarily
    HoldSignals();
    return rval;
}


// expand a wildcard name
static void wildname( char *target, char *source, char *pszTemplate )
{
    char *ptr;
    // build a legal filename (right filename & extension size)
    insert_path( source, fname_part( source ), source );

    // if source has path, copy it to target
    if ( ( ptr = path_part( source )) != NULL ) {
        strcpy( target, ptr );
        source += strlen( ptr );
        target += strlen( ptr );
    } else
        ptr = NULLSTR;

    pszTemplate = fname_part( pszTemplate );

    // OS/2 has a handy built-in function!
    (void)DosEditName( 1, pszTemplate, source, target, ( MAXFILENAME - strlen(ptr)) );
    target = strend( target );

    // strip trailing '.'
    if ( ( target[-1] == '.' ) && ( target[-2] != '.' ) )
        target--;
    *target = '\0';
}


#define REN_BY_ATTRIBUTES 1
#define REN_NOERRORS 2
#define REN_NOTHING 4
#define REN_QUERY 8
#define REN_QUIET 0x10
#define REN_SUBDIRS 0x20
#define REN_TOTALS 0x40

// rename files (including to different directories) or directories
int ren_cmd( int argc, char **argv )
{
    char *arg;
    char source[MAXFILENAME], target_name[MAXFILENAME], target[MAXFILENAME];
    int i, fval, rval = 0;
    unsigned int mode = 0;
    long fRen, lFilesRenamed = 0;
    FILESEARCH dir;

    // get date/time/size ranges
    if ( GetRange( argv[1], &(dir.aRanges), 0 ) != 0 )
        return ERROR_EXIT;

    // check for and remove switches
    // get the target name, or abort if no target
    if ( ( GetSwitches( argv[1], "*ENPQST", &fRen, 0 ) != 0 ) || (( arg = last_arg( argv[1], &argc )) == NULL ) )
        return( usage( RENAME_USAGE ));

    if ( fRen & REN_BY_ATTRIBUTES )
        mode |= 0x07;

    // if query, turn off quiet mode
    if ( fRen & REN_QUERY )
        fRen &= ~( REN_QUIET | REN_TOTALS );

    copy_filename( target, arg );
    StripQuotes( target );
    *gpNthptr = '\0';               // remove target from command line

    // rename matching directories?
    if ( ( fRen & REN_SUBDIRS ) || ( strpbrk( target, WILD_CHARS ) == NULL ) )
        mode |= FILE_DIRECTORY; // 0x10

    // suppress error message from find_file
    if ( fRen & REN_NOERRORS )
        mode |= FIND_NOERRORS;  // 0x100

    for ( argc = 0; (( arg = ntharg( argv[1], argc)) != NULL ); argc++ ) {

        if ( mkfname( arg, (( fRen & REN_NOERRORS ) ? MKFNAME_NOERROR : 0 )) == NULL )
            continue;

        for ( fval = FIND_FIRST; ; fval = FIND_NEXT ) {

            if ( find_file( fval, arg, ( mode | FIND_BYATTS | FIND_CREATE | FIND_DATERANGE | FIND_NO_DOTNAMES ), &dir, source ) == NULL )
                break;

            if ( setjmp( cv.env ) == -1 ) {
                (void)DosFindClose( dir.hdir );
                rval = CTRLC;
                goto rename_abort;
            }

            wildname( target_name, target, source );

            if ( mkfname( source, 0 ) == NULL )
                return ERROR_EXIT;

            // nasty kludge to emulate COMMAND.COM & CMD.EXE stupid
            //   renaming convention (if source has path & target
            //   doesn't, rename into source directory)
            if ( path_part( target_name ) == NULL )
                strins( target_name, path_part( source ));

            if ( mkfname( target_name, 0 ) == NULL )
                return ERROR_EXIT;

            if ( is_dir( target_name ) ) {
                // destination is directory; create target name
                //   (unless we're renaming directories!)
                if ( !is_dir( source ) )
                    mkdirname( target_name, fname_part( source ));
            }

            if ( ( fRen & ( REN_QUIET | REN_TOTALS )) == 0 ) {

                printf( FMT_STR_TO_STR, source, target_name );
                if ( fRen & REN_QUERY ) {
                    if ( ( i = QueryInputChar( NULLSTR, YES_NO_REST )) == REST_CHAR )
                        fRen &= ~REN_QUERY;
                    else if ( i == NO_CHAR )
                        continue;
                    else if ( i == ESCAPE )
                        break;
                } else
                    crlf();
            }

            if ( ( fRen & REN_NOTHING ) == 0 ) {

                if ( rename( source, target_name ) == -1 ) {
                    // check if error is with source or target
                    rval = error( _doserrno, ((( _doserrno == ERROR_ACCESS_DENIED) || ( _doserrno == ERROR_NOT_SAME_DEVICE ) || ( _doserrno == ERROR_PATH_NOT_FOUND)) ? target_name : source));
                } else {
                    lFilesRenamed++;

                    // rename description to target dir
                    if ( stricmp( "EA", DESCRIPTION_FILE ) != 0 )
                        process_descriptions( source, target_name, DESCRIPTION_COPY );

                    // update JPSTREE.IDX
                    if ( is_dir( target_name ) ) {
                        UpdateFuzzyTree( source, 1 );
                        UpdateFuzzyTree( target_name, 0 );
                    }
                }
            }
        }

        // delete the descriptions for renamed or missing files
        if ( (( fRen & REN_NOTHING ) == 0 ) && ( lFilesRenamed != 0L ) ) {
            strcat( arg, WILD_FILE );
            process_descriptions( NULL, arg, DESCRIPTION_REMOVE );
        }
    }

    rename_abort:
    if ( ( fRen & REN_QUIET ) == 0 )
        FilesProcessed( FILES_RENAMED, lFilesRenamed );

    return(( lFilesRenamed != 0L ) ? rval : ERROR_EXIT );
}


#define DEL_BY_ATTRIBUTES 1
#define DEL_NOERRORS 2
#define DEL_FORCE 4
#define DEL_NOTHING 8
#define DEL_QUERY 0x10
#define DEL_QUIET 0x20
#define DEL_SUBDIRS 0x40
#define DEL_TOTALS 0x80
#define DEL_RMDIR 0x100
#define DEL_ALL 0x200
#define DEL_ZAP_ALL 0x400
#define DEL_DIRS_ONLY 0x800
#define DEL_WIPE 0x1000
#define DEL_NO_FCBS 0x2000


typedef struct {
    char szSource[MAXFILENAME];
    char szTarget[MAXFILENAME];
    char *szFilename;
    long fFlags;
    unsigned int nLocalDeleted;
    long lFilesDeleted;
} DEL_STRUCT;

static int _del( DEL_STRUCT * );

// Delete file(s)
int del_cmd( int argc, char **argv )
{
    char *arg;
    int rval = 0, i, nLength;
    QDISKINFO DiskInfo;
    DEL_STRUCT Del;
    long long lBytesFreed = 0;

    // get date/time/size ranges
    if ( GetRange( argv[1], &aRanges, 0 ) != 0 )
        return ERROR_EXIT;

    // check for and remove switches
    if ( ( GetSwitches( argv[1], "*EFNPQSTXYZDW", &(Del.fFlags), 0 ) != 0 ) || ( first_arg( argv[1]) == NULL ) )
        return( usage( DELETE_USAGE ));

        // be quiet by default
        Del.fFlags &= DEL_QUIET;

    // if query, turn off quiet flag
    if ( Del.fFlags & DEL_QUERY )
        Del.fFlags &= ~( DEL_QUIET | DEL_TOTALS );

    Del.lFilesDeleted = 0L;
    for ( argc = 0; (( arg = ntharg( argv[1], argc )) != NULL ); argc++ ) {

        // build the source name
        if ( mkfname( arg, 0 ) == NULL )
            return ERROR_EXIT;

        // if it's a directory, append the "*.*" wildcard
        if ( is_dir( arg ) )
            mkdirname( arg, WILD_FILE );
        else if ( Del.fFlags & DEL_DIRS_ONLY ) {
            if ( ( Del.fFlags & DEL_NOERRORS ) == 0 )
                rval = error( ERROR_4DOS_NOT_A_DIRECTORY, arg );
            continue;
        }

        copy_filename( Del.szSource, arg );

        nLength = strlen( path_part( Del.szSource ));

        // save the source filename part ( for recursive calls &
        //   include lists)
        Del.szFilename = Del.szSource + nLength;

        if ( (( Del.fFlags & DEL_QUERY) == 0 ) && (( Del.fFlags & DEL_ALL ) == 0 ) ) {

            // confirm before deleting an entire directory
            //   ( filename & extension == wildcards only)
            sscanf( Del.szFilename, "%*[?*.;]%n", &i );
            if ( Del.szFilename[i] == '\0' ) {
                printf( "%s : ", Del.szSource );
                if ( QueryInputChar( ARE_YOU_SURE, YES_NO ) != YES_CHAR )
                    continue;
            }
        }

        arg = _alloca( strlen( Del.szFilename ) + 1 );
        Del.szFilename = strcpy( arg, Del.szFilename );

        if ( ( is_net_drive( Del.szSource ) == 0 ) && ( QueryDriveRemote( gcdisk( Del.szSource )) == 0 ) ) {
            (void)QueryDiskInfo( Del.szSource, &DiskInfo, 1 );
            lBytesFreed -= DiskInfo.BytesFree;
        }

        i = _del( &Del );

        if ( ( setjmp( cv.env ) == -1 ) || ( i == CTRLC ) )
            rval = CTRLC;
        EnableSignals();
        if ( ( is_net_drive( Del.szSource ) == 0 ) && ( QueryDriveRemote( gcdisk( Del.szSource)) == 0 ) ) {
            (void)QueryDiskInfo( Del.szSource, &DiskInfo, 1 );
            lBytesFreed += DiskInfo.BytesFree;
        }

        if ( rval == CTRLC )
            break;

        if ( i != 0 )
            rval = ERROR_EXIT;
    }

    if ( ( Del.fFlags & DEL_QUIET ) == 0 ) {
        // display number of files deleted, & bytes freed
        FilesProcessed( FILES_DELETED, Del.lFilesDeleted );
        if ( lBytesFreed > 0L )
            printf( FILES_BYTES_FREED, lBytesFreed );
        crlf();
    }

    // disable signal handling momentarily
    HoldSignals();

    return(( Del.lFilesDeleted != 0L ) ? rval : ERROR_EXIT );
}


// recursive file deletion routine
static int _del( DEL_STRUCT *Del )
{
    int i;
    unsigned int mode = FIND_CREATE;
    int fval, rval = 0, rc;
    char *arg, *source_name;
    FILESEARCH dir;

    EnableSignals();

    // copy date/time range info
    memmove( &(dir.aRanges), &aRanges, sizeof(RANGES) );

    // delete hidden files too?
    if ( ( Del->fFlags & DEL_BY_ATTRIBUTES ) || ( Del->fFlags & DEL_ZAP_ALL ) )
        mode |= 0x07;

    dir.hdir = HDIR_CREATE;
    // trap ^C in DEL loop, and close directory search handles
    if ( setjmp( cv.env ) == -1 ) {
        _del_abort:
        (void)DosFindClose( dir.hdir );
        return CTRLC;
    }

    // suppress error message from find_file
    if ( Del->fFlags & DEL_NOERRORS )
        mode |= FIND_NOERRORS;  // 0x100

    // do a new-style delete
    for ( fval = FIND_FIRST; ( find_file( fval, Del->szSource, (mode | FIND_BYATTS | FIND_DATERANGE ), &dir, Del->szTarget) != NULL ); fval = FIND_NEXT ) {

        if ( Del->fFlags & DEL_QUERY ) {

            qputs( DELETE_QUERY );
            if ( ( i = QueryInputChar( Del->szTarget, YES_NO_REST )) == REST_CHAR )
                Del->fFlags &= ~DEL_QUERY;
            else if ( i == NO_CHAR )
                continue;
            else if ( i == ESCAPE )
                break;

        } else if ( ( Del->fFlags & ( DEL_QUIET | DEL_TOTALS )) == 0 )
            printf( DELETING_FILE, Del->szTarget );

        if ( ( Del->fFlags & DEL_NOTHING ) == 0 ) {

            // overwrite a file with 0's before erasing it
            if ( Del->fFlags & DEL_WIPE ) {
                long lSize;
                if ( ( lSize = QueryFileSize( Del->szTarget, 0 )) > 0L ) {

                    if ( ( i = _sopen( Del->szTarget, (O_WRONLY | O_BINARY), SH_DENYRW )) < 0 )
                        goto Del_Next;

                    arg = malloc( 512 );
                    memset( arg, '\0', 512 );

                    while ( lSize != 0L ) {
                        _write( i, arg, (( lSize > 512L ) ? 512 : (int)lSize ));
                        if ( lSize > 512 )
                            lSize -= 512;
                        else
                            lSize = 0L;
                    }

                    free( arg );
                    _close( i );
                }
            }
            Del->lFilesDeleted++;
            // support for DEL /F(orce) in OS/2 2
            if ( Del->fFlags & ( DEL_FORCE | DEL_WIPE ) ) {
                rc = _doserrno = DosForceDelete( Del->szTarget);
            } else
                rc = remove( Del->szTarget );

            if ( rc != 0 ) {

                // if /Z(ap) flag, clear hidden/rdonly
                //   attributes & try to delete again
                if ( ( Del->fFlags & DEL_ZAP_ALL ) && ( SetFileMode( Del->szTarget,_A_NORMAL ) == 0 ) ) {
                    if ( Del->fFlags & DEL_FORCE ) {
                        rc = _doserrno = DosForceDelete( Del->szTarget);
                    } else
                        rc = remove( Del->szTarget );
                }
            }
            Del_Next:
            if ( rc ) {
                Del->lFilesDeleted--;
                if ( ( Del->fFlags & DEL_NOERRORS ) == 0 )
                    rval = error( _doserrno, Del->szTarget );
                else
                    rval = ERROR_EXIT;
            }
        }
    }

    // delete the description(s) for deleted or missing files
    sprintf( Del->szSource, FMT_DOUBLE_STR, path_part( Del->szSource ), WILD_FILE );

    // update description file
    if ( (( Del->fFlags & DEL_NOTHING) == 0 ) && (( Del->lFilesDeleted != 0L ) || ( Del->fFlags & DEL_RMDIR )) )
        process_descriptions( NULL, Del->szSource, DESCRIPTION_REMOVE );

    // delete matching subdirectory files too?
    if ( Del->fFlags & DEL_SUBDIRS ) {

        // save the current subdirectory start
        source_name = strchr( Del->szSource, '*' );

        // search for all subdirectories in this (sub)directory
        //   tell find_file() not to display errors and to only
        //   return subdir names
#define M ( FIND_NOERRORS | FIND_DIRONLY | FILE_DIRECTORY )      // 0x310
        for ( fval = FIND_FIRST; ( find_file( fval, Del->szSource, (mode | M ), &dir, NULL ) != NULL ); fval = FIND_NEXT ) {
#undef M

            // make the new "source"
            sprintf( source_name, FMT_PATH_STR, dir.name, Del->szFilename );

            // process directory tree recursively
            rval = _del( Del );

            if ( ( setjmp( cv.env ) == -1 ) || ( rval == CTRLC) )
                goto _del_abort;

            // restore the original name
            strcpy( source_name, WILD_FILE );
        }
    }

    // delete the subdirectory?
    if ( ( Del->fFlags & DEL_RMDIR ) && (( Del->fFlags & DEL_NOTHING ) == 0 ) ) {

        // remove the last filename from the directory name
        source_name = path_part( Del->szSource );
        strip_trailing( source_name, SLASHES );

        // don't try to delete the root or current directories
        if ( (( arg = gcdir( source_name, 1 )) == NULL ) || (( strlen( source_name) > 3 ) && ( _stricmp( source_name, arg ) != 0 )) ) {
            if ( ( Del->fFlags & DEL_ZAP_ALL ) || ( gchInclusiveMode & _A_RDONLY ) )
                SetFileMode( source_name, _A_SUBDIR );
            if ( DestroyDirectory( source_name ) == -1 ) {
                if ( ( Del->fFlags & DEL_NOERRORS ) == 0 )
                    error( _doserrno, source_name );
            } else  // remove the directory & its description
                process_descriptions( NULL, source_name, DESCRIPTION_REMOVE );
        }
    }

    // disable signal handling momentarily
    HoldSignals();

    return rval;
}


// change a file's date & time
int touch_cmd( int argc, char **argv )
{
    char *arg;
    int rval = 0, rc, fval = 0, fField = 0;
    // 2022-05-03 SHL Allow touch to touch directories
    int mode = FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_DIRECTORY;   // was 0x07
    int fQuiet = 0, fForce = 0, fCreate = 0;
    unsigned int hours, minutes, seconds, uAttribute;
    unsigned int month, day, year;
    char szTarget[MAXFILENAME];
    FILESEARCH dir;
    DATETIME sysDateTime;

    if ( argc == 1 )
        return( usage( TOUCH_USAGE ));

    QueryDateTime( &sysDateTime );
    sysDateTime.seconds /= 2;

    // get date/time/size ranges
    if ( GetRange( argv[1], &(dir.aRanges), 0 ) != 0 )
        return ERROR_EXIT;

    for ( argc = 0;
        ( arg = ntharg( argv[1], argc | NTHARG_SWITCH_CHAR_IS_DATA )) != NULL;
        argc++ ) {

        if ( *arg == gpIniptr->SwChr ) {

            for ( arg++; ( *arg != '\0' ); ) {

                rc = _ctoupper( *arg++ );
                switch ( rc ) {
                    case 'E':
                        mode |= FIND_NOERRORS;      // 0x100
                        break;

                    case 'C':
                        // create the file if it doesn't exist
                        fCreate = 1;
                        break;

                    case 'F':
                        fForce = 1;
                        break;

                    case 'Q':
                        fQuiet = 1;
                        break;

                    case 'D':
                    case 'T':

                        if ( *arg == ':' )
                            arg++;
                        if ( ( *arg == 'W' ) || ( *arg == 'w' ) ) {
                            fField = 0;
                            arg++;
                        } else if ( ( *arg == 'A' ) || ( *arg == 'a' ) ) {
                            fField = 1;
                            arg++;
                        } else if ( ( *arg == 'C' ) || ( *arg == 'c' ) ) {
                            fField = 2;
                            arg++;
                        }

                        if ( rc == 'D' ) {

                            // valid date entry?
                            if ( *arg == '\0' )     // use today's date
                                break;

                            if ( GetStrDate( arg, &month, &day, &year ) == 3 ) {

                                if ( ( sysDateTime.year = year ) < 80 )
                                    sysDateTime.year += 2000;
                                else if ( sysDateTime.year < 100 )
                                    sysDateTime.year += 1900;

                                sysDateTime.month = (unsigned char)month;
                                sysDateTime.day = (unsigned char)day;
                                arg = NULLSTR;
                                break;
                            }

                            return( error( ERROR_4DOS_INVALID_DATE, arg ));

                        } else {

                            seconds = 0;

                            if ( *arg == '\0' )     // use today's time
                                break;

                            if ( ( sscanf( arg, DATE_FMT, &hours, &minutes, &seconds ) >= 2 ) && ( hours < 24 ) && ( minutes < 60 ) && ( seconds < 60 ) ) {

                                // check for AM/PM syntax
                                if ( ( arg = strpbrk( arg, "AP" )) != NULL ) {
                                    if ( ( hours == 12 ) && ( *arg == 'A' ) )
                                        hours -= 12;
                                    else if ( ( *arg == 'P' ) && ( hours < 12 ) )
                                        hours += 12;
                                }

                                sysDateTime.hours = (unsigned char)hours;
                                sysDateTime.minutes = (unsigned char)minutes;
                                sysDateTime.seconds = (unsigned char)(seconds / 2);
                                arg = NULLSTR;
                                break;
                            }

                            return( error( ERROR_4DOS_INVALID_TIME, arg ));
                        }

                    default:
                        return( usage( TOUCH_USAGE ));
                }
            }

            continue;
        }

        mkfname( arg, 0 );

        // display specified files (including hidden & read-only)
        for ( fval = FIND_FIRST; ( fval != 0 ); fval = FIND_NEXT ) {

            if ( find_file( fval, arg, (mode | FIND_NOERRORS | FIND_BYATTS | FIND_DATERANGE ), &dir, szTarget ) == NULL ) {

                if ( fval == FIND_FIRST ) {

                    // if file doesn't exist, create it
                    if ( ( fCreate ) && (( fval = _sopen( arg, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, SH_DENYWR, S_IWRITE | S_IREAD )) >= 0 ) ) {
                        _close( fval );
                        strcpy( szTarget, arg );
                        fval = 0;
                    } else {
                        if ( ( mode & FIND_NOERRORS ) == 0 )
                            error( ERROR_FILE_NOT_FOUND, arg );
                        rval = ERROR_EXIT;
                        break;
                    }

                } else
                    break;
            } // if find_file

            if ( fForce ) {
                // set the target file attributes, less FILE_READONLY
                QueryFileMode( szTarget, &uAttribute );
                SetFileMode( szTarget, uAttribute & (FILE_ARCHIVED | FILE_HIDDEN | FILE_SYSTEM) );        // was 0x26
            }

            // display the time being set
            if ( fQuiet == 0 )
                printf( "%s %02d:%02d:%02d  %s\n", FormatDate( sysDateTime.month, sysDateTime.day, sysDateTime.year, 0 ), sysDateTime.hours, sysDateTime.minutes, sysDateTime.seconds * 2, szTarget );

            if ( (( rc =  SetFileDateTime( szTarget, 0, &sysDateTime, fField )) != 0 ) &&
                 (( mode & FIND_NOERRORS ) == 0 ) )
                rval = error( rc, szTarget );

            // restore original attributes
            if ( fForce )
                (void)SetFileMode( szTarget, uAttribute );
        }
    }

    return(( fval == 0 ) ? usage( TOUCH_USAGE ) : rval );
}


#define TYPE_BY_ATTRIBUTES 1
#define TYPE_LINENO 2
#define TYPE_PAUSE 4

// display file(s) to stdout
int type_cmd( int argc, char **argv )
{
    char *source, *argline;
    int fval, size, rval=0, col = 0, mode = (O_TEXT | O_RDONLY);
    int iLen;
    long fType, lRow;
    char szClip[MAXFILENAME];
    FILESEARCH dir;

    // get date/time/size ranges
    if ( GetRange( argv[1], &(dir.aRanges), 0 ) != 0 )
        return ERROR_EXIT;

    // check for and remove switches
    if ( ( GetSwitches( argv[1], "*LP", &fType, 0 ) != 0 ) || ( first_arg( argv[1] ) == NULL ) )
        return( usage( TYPE_USAGE ));

    init_page_size();               // clear row & page length vars

    // print the line number
    if ( fType & TYPE_LINENO ) {
        mode = (O_BINARY | O_RDONLY);
        col = 7;
    }

    // pause after each page
    if ( fType & TYPE_PAUSE ) {
        mode = ( O_BINARY | O_RDONLY );
        gnPageLength = GetScrRows();
    }

    // save arguments on stack so we can overwrite with input buffer
    argline = _alloca( strlen( argv[1] ) + 1 );
    strcpy( argline, argv[1] );

    for ( argc = 0; (( source = ntharg( argline, argc )) != NULL ); argc++ ) {

        mkfname( source, 0 );

        // display specified files (including hidden & read-only)
        for ( fval = FIND_FIRST; ; fval = FIND_NEXT ) {

            szClip[0] = '\0';
            if ( QueryIsDevice( source ) ) {

                if ( stricmp( source, CLIP ) == 0 ) {
                    RedirToClip( szClip, 99 );
                    if ( CopyFromClipboard( szClip ) != 0 )
                        break;
                    strcpy( gszCmdline, szClip );
                } else {
                    // kludge for knuckleheads who try to type
                    //   character devices!
                    char *pName;

                    pName = fname_part( source );
                    strcpy( gszCmdline, pName );
                    if ( ( pName == NULL ) /*|| ( stricmp( pName, "NUL" ) != 0 )*/ ) { // 20090905 AB removed stricmp which prevents type on all devices
                        rval = error( ERROR_ACCESS_DENIED, source );
                        break;
                    }

                    // complete device name if not
                    //QueryIsDeviceName( source );  // ToDo: ?  type \dev\ibms506$: works with cmd.exe but not with 4os2
                    //          type \dev\ibms506$ works with 4os2 but not with cmd.exe

                    // only look for devices once
                    if ( fval == FIND_NEXT )
                        break;
                }

            } else if ( QueryIsPipeName( source ) ) {

                // only look for named pipes once
                if ( fval == FIND_NEXT )
                    break;
                strcpy( gszCmdline, source );

#define M ( FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM )
            } else if ( find_file( fval, source, ( M | FIND_BYATTS | FIND_DATERANGE ), &dir, gszCmdline ) == NULL ) {
#undef M
                if ( fval == FIND_FIRST )
                    rval = ERROR_EXIT;
                break;
            }

            if ( ( gnGFH = _sopen( gszCmdline, mode, SH_DENYNO )) < 0 ) {
                rval = error( _doserrno, gszCmdline );
                if ( szClip[0] )
                    break;
                continue;
            }

            // output the file to stdout
            if ( ( fType & TYPE_LINENO ) || ( gnPageLength ) ) {
                int i;
                // 20100130 AB changed to more_page_bin to allow printing of '\0' as needed f.e. with alsahlp$
                // 20100318 AB /L or /P do not work with wrnd32$ cause this would require reading 4k chunks at once
                // one possibility would be when opening a device, read big_buffer (0xFE00 bytes is was cmd.exe does)
                // and then in getline return desired data bytes from big_buffer instead from device
                // as this would lead to buffer copying in getline (imagine big_buffer gets empty, read new chunk,
                // append to big_buffer...) this is not implemented yet
                for ( lRow = 1; ( ( iLen = getline( gnGFH, gszCmdline, CMDBUFSIZ-1, EDIT_DATA ) ) > 0 ); lRow++ ) {
                    TRACE("getline lenght %d", iLen);
                    if ( fType & TYPE_LINENO )
                        printf( "%4lu : ",lRow);
                    for ( i = 0; i < iLen ; i++ ) {
                        if ( gszCmdline[i] == 0x0D || gszCmdline[i] == 0x0A ) gszCmdline[i] = ' ';
                    }
                    more_page_bin( (char *)gszCmdline, col, iLen );
                }

            } else {
                // 20100318 AB changed from 512 to CMDBUFSIZ-1 cause 'type wrnd32$' would need at least 4k buffer
                while ( ( size = _read( gnGFH, gszCmdline, CMDBUFSIZ-1 )) > 0 ) {
                    size = _write( STDOUT, gszCmdline, size );
                    if ( size == -1 ) {
                        if ( QueryIsPipeHandle( STDOUT ) == 0 )
                            error( _doserrno, source );
                        break;
                    }
                }
            }

            gnGFH = _close( gnGFH );

            // delete temporary CLIP: file
            if ( szClip[0] ) {
                remove( source );
                break;
            }
        }
    }

    return rval;
}


// y reads the standard input and writes to standard output, then invokes TYPE
//   to put one or more files to standard output
int y_cmd( int argc, char **argv )
{
    int nBytesRead, fEof;
    char *ptr, szYBuf[514];

    for ( fEof = 0; (( fEof == 0 ) && ( getline( STDIN, szYBuf, 512, EDIT_DATA ) > 0 )); ) {

        // look for ^Z
        if ( ( ptr = strchr( szYBuf, EoF )) != NULL ) {
            if ( ptr == szYBuf )
                break;
            *ptr = '\0';
            fEof = 1;
        } else
            strcat( szYBuf, "\n" );

        nBytesRead = strlen( szYBuf );

        _write( STDOUT, szYBuf, nBytesRead );
    }

    return(( argv[1] != NULL ) ? type_cmd( argc, argv ) : 0 );
}


// tee copies standard input to output & puts a copy in the specified file(s)
int tee_cmd( int argc, char **argv )
{
    char *arg;
    int i;
    int nBytesRead, fEof, mode = 0, rval = 0, nFiles = 1, fd[20], fTeeClip = -1;
    char *ptr, szClip[MAXFILENAME], szTeeBuffer[CMDBUFSIZ+2];
    long fTee;

    // check for "append" switch
    if ( ( GetSwitches( argv[1], "A", &fTee, 0 ) != 0 ) || ( first_arg( argv[1]) == NULL ) )
        return( usage( TEE_USAGE ));

    mode = (( fTee == 0 ) ? (O_RDWR | O_CREAT | O_TEXT | O_TRUNC) : (O_RDWR | O_CREAT | O_TEXT | O_APPEND));

    fd[0] = STDOUT;
    szClip[0] = '\0';

    if ( setjmp( cv.env ) == -1 )
        rval = CTRLC;

    else {

        for ( i = 0; (( arg = ntharg( argv[1], i )) != NULL ) && ( nFiles < 20 ); i++, nFiles++ ) {

            mkfname( arg, 0 );

            // TEE'ing to CLIP: ?
            if ( stricmp( arg, CLIP ) == 0 ) {
                RedirToClip( szClip, 99 );
                arg = szClip;
            }

            if ( ( fd[nFiles] = _sopen( arg, mode, SH_DENYWR, ( S_IREAD | S_IWRITE ))) < 0 ) {
                rval = error( _doserrno, arg );
                goto tee_bye;
            }

            // save file handle for CLIP:
            if ( arg == szClip )
                fTeeClip = fd[nFiles];
        }

        for ( fEof = 0; (( fEof == 0 ) && ( getline( STDIN, szTeeBuffer, CMDBUFSIZ, EDIT_DATA ) > 0 )); ) {

            // look for ^Z
            if ( ( ptr = strchr( szTeeBuffer, EoF )) != NULL ) {
                if ( ptr == szTeeBuffer )
                    break;
                *ptr = '\0';
                fEof = 1;
            } else
                strcat( szTeeBuffer, "\n" );
            nBytesRead = strlen( szTeeBuffer );

            for ( i = 0; ( i < nFiles ); i++ ) {
                _write( fd[i], szTeeBuffer, nBytesRead );
            }
        }
    }

    tee_bye:
    // close the output files (this looks a bit peculiar because we have
    //   to allow for a ^C during the close(), & still close everything
    for ( ; ( nFiles > 1 ); nFiles-- ) {

        // is one of the TEE arguments CLIP: ?
        if ( fTeeClip == fd[nFiles-1] )
            CopyToClipboard( fTeeClip );

        _close( fd[nFiles-1] );
    }

    if ( szClip[0] )
        remove( szClip );

    // disable signal handling momentarily
    HoldSignals();

    return rval;
}

