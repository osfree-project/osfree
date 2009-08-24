// SYSCMDS.C - System commands for 4xxx / TCMD family
//   Copyright (c) 1988 - 1998 Rex C. Conn  All rights reserved

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <direct.h>
#include <dos.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <malloc.h>
#include <share.h>
#include <string.h>

#include "4all.h"


static void GetFuzzyDir( char * );
void UpdateFuzzyTree( char *, int );
static int _global( unsigned long, long, char *, char * );

char gaPushdQueue[ DIRSTACKSIZE ];  // PUSHD/POPD storage area
static char lastdir[ MAXPATH ];     // last dir for "CD -"


// change the current directory
int cd_cmd( int argc, char **argv )
{
    char *ptr;
    int fFlags = CD_SAVELASTDIR;
    long fCD = 0;

    if ( argv[1] != NULL ) {

        ptr = first_arg( argv[1] );
        if ( ptr && ( stricmp( ptr, "/N" ) == 0 ) ) {
            fCD |= 2;
            argv++;
        }
    }

    // disable fuzzy searching
    if ( fCD & 2 )
        fFlags |= CD_NOFUZZY;

    // if no args or arg == disk spec, print current working directory
    if (( argv[1] == NULL ) || ( _stricmp(( argv[1] ) + 1, ":" ) == 0 )) {

        if (( ptr = gcdir( argv[1], 0 )) == NULL )
            return ERROR_EXIT;  // invalid drive

        printf( FMT_STR_CRLF, ptr );
        return 0;
    }

    return ( __cd( argv[1], fFlags ));
}


// change the current disk and directory
int cdd_cmd( int argc, char **argv )
{
    char *ptr, *arg;
    char szBuf[128], szDrives[32], szTreePath[260];
    int rval = 0, fFlags = CD_CHANGEDRIVE | CD_SAVELASTDIR, i;

    // if /A, display current directory on all drives C: - Z:
    if ( argv[1] != NULL ) {

        // disable fuzzy searching
        if ( _stricmp( argv[1], "/N" ) == 0 ) {

        fFlags |= CD_NOFUZZY;
        argv++;
        argc--;

        } else if ( _stricmp( argv[1], "/A" ) == 0 ) {

        for ( argc = 3; ( argc <= 26 ); argc++ ) {

            if ( QueryDriveReady( argc )) {
                sprintf( szBuf, "%c:", argc + 64 );
                if (( ptr = gcdir( szBuf, 1 )) != NULL )
                    printf( FMT_STR_CRLF, ptr );
            }
        }

        return 0;

        } else if ( _strnicmp( argv[1], "/S", 2 ) == 0 ) {

        // get tree index location
        if ( gpIniptr->TreePath != INI_EMPTYSTR )
            ptr = (char *)( gpIniptr->StrData + gpIniptr->TreePath );
        else
            ptr = "C:\\";
        strcpy( szTreePath, ptr );
        mkdirname( szTreePath, "jpstree.idx" );

        // scan drive for wildcard searches
        ptr = skipspace( argv[1] + 2 );
        if ( *ptr == '\0' ) {
            // default to all hard disks
            SetDriveString( szDrives );
            ptr = szDrives;
        } else {
            arg = _alloca( strlen( ptr ) + 1 );
            ptr = strcpy( arg, ptr );
        }

        for ( argc = 0; ( *ptr != '\0' ); ) {

            // use TREE to build the directory list
            if ( *ptr == '\\' ) {

                // it's a UNC name
                arg = first_arg( ptr );
                printf( BUILDING_UNC_INDEX, arg );
                sprintf( szBuf, "*@tree /c %s %s %s", arg, (( argc == 0 ) ? ">!" : ">>!" ), szTreePath );
                ptr += strlen( arg ) - 1;

            } else {
                printf( BUILDING_INDEX, *ptr );
                sprintf( szBuf, "*@tree /c %c:\\ %s %s", *ptr, (( argc == 0 ) ? ">!" : ">>!" ), szTreePath );
            }

            // make sure TREE isn't disabled!
            argc = findcmd( "TREE", 1 );
            i = commands[ argc ].pflag;
            commands[ argc ].pflag &= ~CMD_DISABLED;

            rval = command( szBuf, 0 );

            commands[ argc ].pflag = i;

            // check for ^C or stack overflow
            if (( setjmp( cv.env ) == -1 ) || ( rval == CTRLC ) || ( cv.exception_flag ))
                return CTRLC;

            // get next drive argument
            do {
                ptr = skipspace( ptr + 1 );
            } while ( *ptr == ':' );
            argc = 1;
        }

        return 0;
        }
    }

    return ((( argc == 1 ) || ( argv[1][0] == '\0' )) ? usage( CDD_USAGE ) : __cd( argv[1], fFlags ));
}


// get the location of JPSTREE.IDX
static void GetFuzzyDir( char *pszDir )
{
    char *arg;

    if ( gpIniptr->TreePath != INI_EMPTYSTR )
        arg = (char *)( gpIniptr->StrData + gpIniptr->TreePath );
    else
        arg = "C:\\";

    strcpy( pszDir, arg );
    mkdirname( pszDir, "jpstree.idx" );
}


// update JPSTREE.IDX (on _mkdir() & _rmdir() )
void UpdateFuzzyTree( char *pszDirectory, int fFlag )
{
    int i;
    int fd, fPass = 0, fTruncate = 0;
    unsigned int uBlockSize, n;
    char *arg, szDirName[ MAXPATH ], szPath[ MAXPATH ], szDirectory[ MAXPATH ];
    char *pchWrite, *fptr;
    long lReadOffset, lWriteOffset;

    uBlockSize = 16386;
    if (( pchWrite = AllocMem( &uBlockSize )) == 0L )
        return;

    // disable ^C / ^BREAK handling
    HoldSignals();

    // get tree index location
    GetFuzzyDir( szDirName );
    strcpy( szDirectory, pszDirectory );

    szPath[0] = '\0';
        if (( fd = _sopen( szDirName, O_RDWR | O_BINARY, SH_DENYWR )) > 0 ) {

        // if inserting, save the path portion (can't use path_part() here!)
        if ( fFlag == 0 ) {

        strcpy( szPath, szDirectory );

        // search path backwards for beginning of filename
        for ( arg = strend( szPath ); ( --arg >= szPath ); ) {
            if (( *arg == '\\' ) || ( *arg == '/' )) {
                arg[1] = '\0';
                break;
            }
        }

        strip_trailing( szPath+3, "\\/" );
        }

AddEntry:
        // on first pass with an add, look for a matching entry
        // on second pass, add the new entry
        lReadOffset = lWriteOffset = 0L;

        // read 16k blocks of the JPSTREE.IDX file
        while (( FileRead( fd, pchWrite, 16384, &uBlockSize ) == 0 ) && ( uBlockSize != 0 )) {

        // back up to the end of the last line, and seek
        //   to the end of the last line of the current block
        for ( i = uBlockSize; (( --i > 0 ) && ( pchWrite[i] != '\n' )); )
            ;

        pchWrite[++i] = '\0';
        lReadOffset += i;

        // read a line & try for a match
        for ( fptr = pchWrite; (( fTruncate == 0 ) && ( *fptr != '\0' ) && ( *fptr != EoF )); ) {

            // look for argument match
            sscanf( fptr, "%*[\r\n]%[^\004\r\n]%*[^\r\n]%n", szDirName, &n );

            // find where the insertion goes
            if (( fPass == 1 ) && ( _stricmp( szPath, szDirName ) == 0 )) {

                // get remaining size
                lReadOffset = lWriteOffset + ( fptr - pchWrite ) + n;
                lWriteOffset = ( QuerySeekSize( fd ) - lReadOffset );
                _lseek( fd, lReadOffset, SEEK_SET );
                if ( lWriteOffset > 0L ) {

                uBlockSize = (unsigned int)lWriteOffset;

                    // read remainder of file
                    if (( fptr = AllocMem( &uBlockSize )) == 0L )
                    goto ExitUpdateFuzzy;

                    FileRead( fd, fptr, uBlockSize, &n );
                   _lseek( fd, lReadOffset, SEEK_SET );
                }

                // insert it here!
                qprintf( fd, "\r\n%s", szDirectory );

                // write remainder of file
                if ( lWriteOffset > 0L ) {
                FileWrite( fd, fptr, n, &uBlockSize );
                FreeMem( fptr );
                }
                goto ExitUpdateFuzzy;

            } else if (( fPass == 0 ) && ( _stricmp( szDirName, szDirectory ) == 0 )) {

                // entry already exists, so quit
                if ( fFlag == 0 )
                goto ExitUpdateFuzzy;

                // remove this entry
                fTruncate = 1;

                // collapse directory name
                strcpy( fptr, fptr + n );
                break;

            } else {
                // point to beginning of next line
                fptr += n;
                }
        }

        if ( fTruncate ) {

            if (( i = strlen( pchWrite )) > 0 ) {

            _lseek( fd, lWriteOffset, SEEK_SET );
            (void)FileWrite( fd, pchWrite, i, (unsigned int *)&uBlockSize );

            // save current write position &
            //   restore read position
            lWriteOffset += uBlockSize;
            }

        } else
            lWriteOffset = lReadOffset;

        if ( uBlockSize < 4096 )
            break;

        _lseek( fd, lReadOffset, SEEK_SET );
        }

        // truncate the file
        if ( fTruncate )
        (void)_chsize( fd, lWriteOffset );

        if (( fFlag == 0 ) && ( fPass == 0 )) {
        RewindFile( fd );
        fPass = 1;
        goto AddEntry;
        }

ExitUpdateFuzzy:
        _close( fd );
    }

    FreeMem( pchWrite );

    // enable ^C / ^BREAK handling
    EnableSignals();
}


#if __WATCOMC__ < 1280
int _chdrive( unsigned nDrive )
{
    unsigned nTotDrives, nCurDrive;

    _dos_setdrive(nDrive, &nTotDrives);
    _dos_getdrive( &nCurDrive );
    return ( nCurDrive != nDrive );
}
#endif


int __cd( char *dir, int fFlags )
{
    char *ptr = NULL;
    int i;
    int fd, nLength, fWildAll = 0, rval = 0;
    unsigned int uPass = 0, uMatches;
    unsigned long ulListSize, ulSize = 0L;
    char dirname[ MAXPATH*2 ], szDirectory[ MAXPATH*2 ], szDrive[3];
    char *pszPath, *pszSavedPath;
    char * *list = 0L;
    char *fenvptr, *fptr, *fpStartList;

    // save original "dir" (if it was passed as "path_part" it will be
    //   overwritten)
    pszPath = _alloca( strlen( dir ) + 1 );
    dir = strcpy( pszPath, dir );

    pszSavedPath = NULLSTR;

    i = gcdisk( NULL );
    if ( i == 0 )
        ptr = gcdir( NULL, 1 );

    if ((( i > 0 ) && ( QueryDriveReady( i ) == 1 )) || ( ptr != NULL )) {

        // save current directory for "CD -" or "CDD -"
        if (( ptr != NULL ) || (( ptr = gcdir( NULL, 0 )) != NULL )) {
            pszSavedPath = _alloca( strlen( ptr ) + 1 );
            strcpy( pszSavedPath, ptr );
        }
    }

    // save to cd / cdd popup window stack?
    if ( fFlags & CD_SAVELASTDIR ) {

        if ( _stricmp( dir, "-" ) == 0 ) {
            dir = lastdir;
            if ( *dir == '\0' )
                return 0;
        }

        if ( ptr != NULL )
            SaveDirectory( glpDirHistory, ptr );
    }

    if ( strpbrk( dir, WILD_CHARS ) == NULL ) {

        copy_filename( dirname, dir );

        // look for _CDPATH / CDPATH environment variable
        if ((( fenvptr = get_variable( CDPATH )) == 0L ) && (( fenvptr = get_variable( CDPATH + 1 )) == 0L ))
        fenvptr = NULLSTR;

        for ( ; ; ) {

        // expand the directory name to support things like "cd ..."
        if ( mkfname( dirname, 0 ) == NULL )
            return ERROR_EXIT;

        // remove trailing \ or / in syntax like "cd c:\123\wks\"
        strip_trailing( dirname+3, SLASHES );

ChangeDirectory:
        // try to change to that directory
        if ( _chdir( dirname ) == 0 ) {

            if ( fFlags & CD_SAVELASTDIR )
                strcpy( lastdir, pszSavedPath );

            if (( fFlags & CD_CHANGEDRIVE ) == 0 )
                return 0;

            // if "change drive" flag set, change to it
            return (( _chdrive( gcdisk( dirname )) != 0 ) ? error( ERROR_INVALID_DRIVE,dirname ) : 0 );

        } else if ( uPass != 0 )
            return (( fFlags & CD_NOERROR ) ? ERROR_EXIT : error( _doserrno, dirname ));

        // get next CDPATH arg, skipping '=', ';', ','
        sscanf( fenvptr, " %*[,;=]%n", &nLength );
        fenvptr += nLength;

        // if the CD fails, check to see if we can try (more) CDPATH
        if (( dir[1] == ':' ) || ( *dir == '\\' ) || ( *dir == '/' ) || ( *fenvptr == '\0' )) {
            if ( gpIniptr->FuzzyCD != 0 )
                break;
            return (( fFlags & CD_NOERROR ) ? ERROR_EXIT : error( _doserrno, dir ));
        }

        // get new directory to try
        sscanf( fenvptr, " %255[^;=,]%n", dirname, &nLength );
        fenvptr += nLength;

        // append the original argument to the new pathname
        mkdirname( dirname, dir );
        }

        if (( gpIniptr->FuzzyCD == 0 ) || ( fFlags & CD_NOFUZZY ))
        return (( fFlags & CD_NOERROR ) ? ERROR_EXIT : error( ERROR_PATH_NOT_FOUND, dir ));
    }

    // try wildcard matching
    szDrive[0] = '\0';

    StripQuotes( dir );

    // remove trailing \ or /
    strip_trailing( dir + 1, SLASHES );

    // if CD * , display current drive only
    if (( stricmp( dir, "*" ) == 0 ) && (( fFlags & CD_CHANGEDRIVE ) == 0 )) {

        if ((( ptr = gcdir( NULL, 1 )) != NULL ) && ( ptr[1] == ':' ))
            sprintf( szDrive, "%.2s", ptr );

    // if "d:*", only display dirs for that drive
    } else if (( ptr = path_part( dir )) != NULL ) {

        // save & remove drive spec
        if ( ptr[1] == ':' ) {
            sprintf( szDrive, "%.2s", ptr );
            strcpy( ptr, ptr + 2 );
            strcpy( dir, dir + 2 );
        }
    }

    copy_filename( dirname, dir );
    ptr = _alloca( strlen( dirname ) + 1 );
    dir = strcpy( ptr, dirname );

    // get tree index location
    GetFuzzyDir( dirname );

    // build JPSTREE.IDX if it doesn't exist
    if ( is_file( dirname ) == 0 ) {
        if ( command( "cdd /s", 0 ) == CTRLC )
            return CTRLC;
    }

        if (( fd = _sopen( dirname, O_RDONLY | O_BINARY, SH_DENYWR )) < 0 )
        return 0;

    if ( setjmp( cv.env ) == -1 ) {
        rval = CTRLC;
        goto ExitCD;
    }

    uMatches = 0xFFF0L;
    fptr = fpStartList = AllocMem( &uMatches );
    ulListSize = uMatches - 0xFF;

    //  Read list array looking for match.  If we have more than
    //   one match, call wPopSelect

    fWildAll = ( stricmp( dir, "*" ) == 0 );

    pszPath = path_part( dir );
    for ( i = uMatches = 0; ; uPass++ ) {

        // kludge to allow wildcard search even if FuzzyCD == 0
        if ( uPass >= gpIniptr->FuzzyCD ) {
        if ( uPass == 0 ) {
            if ( strpbrk( dir, WILD_CHARS ) == NULL )
            break;
            } else
            break;
        }

        // on first pass, only look for exact matches
        // on second pass, append a "*"; on third pass prefix a "*"
        if ( uPass == 1 ) {

        // if we already got an exact match, quit now
        if (( i > 0 ) && ( strpbrk( dir, WILD_CHARS ) == NULL ))
            break;
        strcat( dir, "*" );

        // if "*dir*" match requested, skip the second pass and go
        //   straight to the third pass
        if ( gpIniptr->FuzzyCD == 3 )
            continue;

        } else if ( uPass == 2 ) {
        if ( *dir == '*' )
            break;
        strins( dir, "*" );
        }

        // rewind the file
        RewindFile( fd );
        while ( getline( fd, szDirectory, MAXPATH, EDIT_DATA ) > 0 ) {

        if ( szDirectory[0] == '\0' )
            continue;

        // check for "d:*" syntax
        if (( szDrive[0] ) && ( _ctoupper( szDrive[0] ) != _ctoupper( szDirectory[0] ) ))
            continue;

        sscanf( szDirectory, "%[^\004]", dirname );
        if ( fWildAll == 0 ) {

            if ( pszPath != NULL ) {
                if ( wild_cmp( dir, ((( *pszPath == '\\' ) || ( *pszPath == '/' )) ? dirname + 2 : dirname + 3 ), 2, TRUE ) != 0 )
                    continue;
            } else if ( wild_cmp( dir, fname_part( dirname ), 2, TRUE ) != 0 )
                continue;
        }

        // allocate memory for 64 entries at a time
        if (( uMatches % 64 ) == 0 ) {
            ulSize += 256;
            list = (char * *)ReallocMem( (char *)list, ulSize );
        }

        uMatches++;

        nLength = strlen( szDirectory ) + 1;

        if (( list == 0L ) || ( fpStartList == 0L ) || (((unsigned int)( fptr - fpStartList ) + nLength ) >= ulListSize )) {

            ulListSize += 0x8000;
            if (( fpStartList = ReallocMem( fpStartList, ulListSize )) == 0L ) {
                _close( fd );
                rval = error ( ERROR_NOT_ENOUGH_MEMORY, NULL );
                goto ExitCD;
            }
        }

        list[ i ] = strcpy( fptr, szDirectory );

        // check for SFN's appended following a ^D
        if (( fenvptr = strchr( fptr, 4 )) != 0L )
            *fenvptr = '\0';
        fptr += nLength;
        i++;
        }

        if ( uMatches != 0 )
        break;
    }

    _close( fd );

    if (( i == 0 ) || ( uMatches == 0 )) {
        rval = (( fFlags & CD_NOERROR ) ? ERROR_EXIT : error( ERROR_PATH_NOT_FOUND, dir ));
        goto ExitCD;
    }

    dirname[0] = '\0';
    *(++fptr) = '\0';

    fenvptr = NULLSTR;
    if ( uMatches == 1 )
        strcpy( dirname, list[0] );

    else {
        // call the popup window
        if (( fptr = wPopSelect( gpIniptr->CDDTop, gpIniptr->CDDLeft, gpIniptr->CDDHeight, gpIniptr->CDDWidth, list, i, 1, " Change Directory ", NULL, NULL, (( fWildAll ) ? 4 : 5 ) )) != 0L )
            strcpy( dirname, fptr );
    }

    // free the pointer array & list memory
ExitCD:
    FreeMem( fpStartList );
    FreeMem( (char *)list );

    // if we aborted wPopSelect with ^C, bomb after cleanup
    if ( cv.exception_flag )
        return CTRLC;

    if (( rval == 0 ) && ( dirname[0] )) {
        uPass = 3;
        goto ChangeDirectory;
    }

    return rval;
}


// save the directory in the specified dir stack
void SaveDirectory( char *dstack, char *dir )
{
    int length;
    int uSize;
    char *dptr, *arg;

    if ( dstack == glpDirHistory ) {
        // check if dir history size has been changed in .INI dialog
        if ( gpIniptr->LocalDirHistory ) {
            if ( gpIniptr->DirHistoryNew > gpIniptr->DirHistorySize )
                glpDirHistory = ReallocMem( glpDirHistory, gpIniptr->DirHistoryNew );
            gpIniptr->DirHistorySize = gpIniptr->DirHistoryNew;
        }
        uSize = gpIniptr->DirHistorySize;
    } else
        uSize = DIRSTACKSIZE;

    length = strlen( dir ) + 1;

    // if CD / CDD history, remove duplicates
    if (( arg = dstack ) == glpDirHistory ) {

        while ( *arg != '\0' ) {

        if ( stricmp( arg, dir ) == 0 ) {
            // remove duplicate
            memmove( arg, next_env( arg ), (int)( end_of_env( arg ) - next_env( arg )) + 1 );
        } else
            arg = next_env( arg );
        }
    }

    // if directory queue overflow, remove the oldest entry
    while (((( dptr = end_of_env( dstack )) - dstack ) + length ) >= ( uSize - 1 ))
        memmove( dstack, next_env( dstack ), (int)( dptr - next_env( dstack )) + 1 );

    strcpy( dptr, dir );
    dptr[ length ] = '\0';
}


int MakeDirectory( char *pszDirectory, int fUpdateFuzzy )
{
    int rval;

    if ((( rval = _mkdir( pszDirectory )) != -1 ) && ( fUpdateFuzzy ))
        UpdateFuzzyTree( pszDirectory, 0 );
    return rval;
}


int DestroyDirectory( char *pszDirectory )
{
    int rval;

    if (( rval = _rmdir( pszDirectory )) != -1 )
        UpdateFuzzyTree( pszDirectory, 1 );
    return rval;
}

// save the current directory & change to new one
int pushd_cmd( int argc, char **argv )
{
    char *dptr;
    char dir[MAXPATH];

    if (( dptr = gcdir( NULL, 0 )) == NULL )    // get current directory
        return ERROR_EXIT;

    strcpy( dir, dptr );

    // change disk and/or directory
    //   (if no arguments, just save the current directory)
    if (( argc > 1 ) && ( __cd( argv[1], CD_CHANGEDRIVE | CD_SAVELASTDIR ) != 0 ))
        return ERROR_EXIT;

    // save the previous disk & directory
    SaveDirectory( gaPushdQueue, dir );

    return 0;
}


// POP a PUSHD directory
int popd_cmd( int argc, char **argv )
{
    char *dptr;

    if ( argc > 1 ) {

        if ( argv[1][0] == '*' ) {
            // clear directory stack
            gaPushdQueue[0] = '\0';
            gaPushdQueue[1] = '\0';
            return 0;
        }

        return ( usage( POPD_USAGE ));
    }

    if ( gaPushdQueue[0] == '\0' )
        return ( error( ERROR_4DOS_DIR_STACK_EMPTY, NULL ));

    // get last (newest) entry in queue
    for ( dptr = ((char *)end_of_env( gaPushdQueue ) - 1 ); (( dptr > gaPushdQueue ) && ( dptr[-1] != '\0' )); dptr-- )
        ;

    // change drive and directory
    argc = __cd( dptr, CD_CHANGEDRIVE | CD_SAVELASTDIR );

    // remove most recent entry from DIRSTACK
    dptr[0] = '\0';
    dptr[1] = '\0';

    return argc;
}


// Display the directory stack created by PUSHD
int dirs_cmd( int argc, char **argv )
{
    char *dptr;

    if ( gaPushdQueue[0] == '\0' )
        return (error( ERROR_4DOS_DIR_STACK_EMPTY, NULL ));

    for ( dptr = gaPushdQueue; ( *dptr != '\0' ); dptr = next_env( dptr ))
        printf( FMT_FAR_STR_CRLF, dptr );

    return 0;
}


// remove directory
int rd_cmd( int argc, char **argv )
{
    int fval;
    unsigned int rval;
    char source[MAXFILENAME], *dirname;
    FILESEARCH dir;

    if ( GetRange( argv[1], &(dir.aRanges), 0 ) != 0 )
        return ERROR_EXIT;

    if ( first_arg( argv[1] ) == NULL )
        return ( usage( RD_USAGE ));

    for ( rval = argc = 0; (( dirname = ntharg( argv[1], argc )) != NULL ); argc++) {

        // expand filename & remove any trailing backslash
        mkfname( dirname, 0 );
        strip_trailing( dirname+3, SLASHES );

        for ( fval = FIND_FIRST; ( find_file( fval, dirname, (0x210 | FIND_DATERANGE), &dir, source ) != NULL ); fval = FIND_NEXT ) {

            if ( DestroyDirectory( source ) == -1 )
                rval = error( _doserrno, source );
        }

        // check for directory not found!
        if ( fval == FIND_FIRST )
            rval = ERROR_EXIT;

        // remove descriptions from parent directory
        process_descriptions( NULL, dirname, DESCRIPTION_REMOVE );
    }

    return rval;
}


// attach descriptive labels to filenames
int describe_cmd( int argc, char **argv )
{
    int fval;
    char *arg;
    char source[MAXFILENAME], fname[MAXFILENAME], szNewDescription[514];
    int fHaveDescription, rc, rval = 0;
    long fDescribe;
    FILESEARCH dir;

    // get file date/time ranges
    if ( GetRange( *(++argv ), &(dir.aRanges), 0 ) != 0 )
        return ERROR_EXIT;

    // check for and remove /A:-rhsda switch
    if (( GetSwitches( *argv, "*", &fDescribe, 1 ) != 0 ) || ( *argv == NULL ) || ( **argv == '\0' ))
        return ( usage( DESCRIBE_USAGE ));

    if (( setjmp( cv.env ) == -1 ) || ( cv.exception_flag )) {
        (void)DosFindClose( dir.hdir );
        return CTRLC;
    }

    for ( argc = 0; (( arg = ntharg( *argv, argc )) != NULL ); argc++ ) {

        copy_filename( source, arg );

        // get file description
        szNewDescription[0] = '\0';
        fHaveDescription = 0;
        for ( rc = argc; (( arg = ntharg( *argv, rc )) != NULL ); rc++ ) {

            if ( rc > 0 ) {

                // is this argument predefined as a description?
                if (( strnicmp( arg, "/D", 2 ) == 0 ) && ( arg[2] == DOUBLE_QUOTE )) {
                    arg += 2;
                    goto HaveDescription;
                }

                // make sure it's not an HPFS/NTFS/LFN filename
                if (( *arg == DOUBLE_QUOTE ) && (( ifs_type( arg ) == 0 ) || (( is_file( arg ) == 0 ) && ( is_dir( arg ) == 0 )))) {
HaveDescription:
                    fHaveDescription = sscanf( arg + 1, "%511[^\"]", szNewDescription );
                    break;
                }
            }
        }

        // if the current argument is a description, skip it
        if ( rc == argc )
            continue;

        for ( fval = FIND_FIRST; ( find_file( fval, source, (0x1017 | FIND_BYATTS | FIND_DATERANGE | FIND_NO_DOTNAMES), &dir, fname ) != NULL ); fval = FIND_NEXT ) {

            mkfname( fname, 0 );

            // display current description & get new one
            if ( fHaveDescription == 0 ) {

                szNewDescription[0] = '\0';
                process_descriptions( fname, szNewDescription, DESCRIPTION_READ | DESCRIPTION_PROCESS );
                if ( _isatty(STDIN))
                    printf( DESCRIBE_PROMPT, fname );

                // if input is redirected, exit if EoF
                if ( getline( STDIN, szNewDescription, (( strlen( szNewDescription) <= gpIniptr->DescriptMax) ? gpIniptr->DescriptMax : strlen( szNewDescription)),EDIT_ECHO) == 0 ) {
                    if ( _isatty( STDIN ) == 0 )
                        return rval;
                }
            }

            // write the new description
            if (( rc = process_descriptions( fname, szNewDescription, ( DESCRIPTION_WRITE | DESCRIPTION_CREATE | DESCRIPTION_REMOVE | DESCRIPTION_PROCESS ))) != 0 )
                rval = error( rc, DESCRIPTION_FILE );
        }
    }

    return rval;
}

// return or set the path in the environment
int path_cmd( int argc, char **argv )
{
    PCH feptr;

    // if no args, display the current PATH
    if ( argc == 1 ) {
        printf( FMT_FAR_STR_CRLF, (( feptr = get_variable( *argv )) == 0L ) ? (char *)NO_PATH : feptr - 5 );
        return 0;
    }

    // remove whitespace between args & null path spec (";")
    sprintf( *argv+4, FMT_EQUAL_STR, (( argv[1] != NULL ) ? argv[1] + strspn( argv[1], " \t=" ) : NULLSTR));

    // add argument to environment (in upper case for Netware bug)
    return ( add_variable( strupr( *argv )));
}


// set the command-line prompt
int prompt_cmd( int argc, char **argv )
{
    // remove whitespace between args
    sprintf( *argv+6, FMT_EQUAL_STR, (( argv[1] != NULL ) ? argv[1] + strspn( argv[1]," \t=") : NULLSTR) );

    return ( add_variable( *argv ));
}


// Set the screen colors (using ANSI or the BIOS)
int color_cmd( int argc, char **argv )
{
    // scan the input line for the colors requested
    if (( argc < 3 ) || (( argc = GetColors( argv[1], 1 )) < 0 ))
        return ( usage( COLOR_USAGE ));

    set_colors( argc );

    return 0;
}


// display the disk status (total size, total used, & total free)
int df_cmd( int argc, char **argv )
{
    int rval = 0;
    QDISKINFO DiskInfo;

    // if no argument, return the default disk
    if ( argc == 1 ) {
        argv[1] = gcdir( NULL, 0 );
        argv[2] = NULL;
    }

    while ( *(++argv ) != NULL ) {

        if ( strnicmp( *argv, "\\\\", 2 ) != 0 ) {
            crlf();
            if ( getlabel( *argv )) {
                rval = ERROR_EXIT;
                continue;
            }
        }
        crlf();

        // get the total & free space
        if ( QueryDiskInfo( *argv, &DiskInfo, 0 ) == 0 ) {
            printf( TOTAL_DISK_USED, DiskInfo.BytesTotal, ( DiskInfo.BytesTotal - DiskInfo.BytesFree ));
            printf( BYTES_FREE, DiskInfo.BytesFree );
        }
    }

    return rval;
}


// Display/Change the 4xxx / TCMD variables
int setdos_cmd( int argc, char **argv )
{
    extern int fNoComma;

    unsigned char *arg;
    int fDisable = 0, i;

    if ( argc == 1 ) {  // display current default parameters

        char szBuf[10];

        // convert /X into readable form
        if ( gpIniptr->Expansion == 0 )
            strcpy( szBuf, "0" );
        else {

            szBuf[0] = '\0';
            for ( argc = 0; ( argc < 8 ); argc++ ) {
            if ( gpIniptr->Expansion & (char)( 1 << argc ))
                IntToAscii( argc + 1, strend( szBuf ));
            }
        }

        printf( SETDOS_IS,

          gpIniptr->BrightBG, gpIniptr->CmdSep,
          gpIniptr->Descriptions, DESCRIPTION_FILE, gpIniptr->EscChr,
          gpIniptr->EvalMin, gaCountryInfo.szDecimal[0],
          gpIniptr->EvalMax, szBuf, gpIniptr->LineIn,
          gpIniptr->EditMode, gpIniptr->NoClobber, gpIniptr->ParamChr,
          (( gpIniptr->Rows == 0 ) ? GetScrRows() + 1 : gpIniptr->Rows),
          gpIniptr->CursO, gpIniptr->CursI,
          gpIniptr->Upper, gpIniptr->BatEcho, gpIniptr->SingleStep);

    } else {

        fNoComma = 1;
        for ( argc = 0; (( arg = ntharg( argv[1], argc )) != NULL ); argc++ ) {

            if (( *arg != gpIniptr->SwChr ) || ( arg[1] == '\0' ) || ( arg[2] == '\0' )) {
setdos_error:
                fNoComma = 0;
                return ( error( ERROR_INVALID_PARAMETER, arg ));
            }

            // point past switch character
            arg++;

            switch ( _ctoupper( *arg++ )) {
            case 'B':   // B - Bright/Blink background
                gpIniptr->BrightBG = (char)atoi( arg );
                SetBrightBG();
                break;

            case 'C':   // C - compound command separator
                if ( isalnum( *arg ))
                    goto setdos_error;
                gpIniptr->CmdSep = *arg;
                break;

            case 'D':   // D - description enable/disable
                if (( *arg == '0' ) || ( *arg == '1' ))
                    gpIniptr->Descriptions = (char)atoi( arg );

                else if (*arg == '"' ) {
                    // set the description file name
                    sscanf( arg + 1, "%63[^\"]", DESCRIPTION_FILE );
                }
                break;

            case 'E':   // E - escape character
                if ( isalnum( *arg ))
                    goto setdos_error;
                gpIniptr->EscChr = *arg;
                break;

            case 'F':   // set EVAL precision
                SetEvalPrecision( arg, &(gpIniptr->EvalMin), &(gpIniptr->EvalMax) );
                break;

            case 'G':   // set decimal & thousands characters
                if ( isalnum( *arg ))
                    gpIniptr->DecimalChar = gpIniptr->ThousandsChar = 0;
                else {
                    unsigned char cNewDecimal, cNewThousands;
                    sscanf( arg, "%c%c", &cNewDecimal, &cNewThousands);
                    gpIniptr->DecimalChar = ((cNewDecimal == '.') ? 1 : 2);
                    gpIniptr->ThousandsChar = ((cNewThousands == '.') ? 1 : 2);
                }
                QueryCountryInfo();
                break;

            case 'I':   // I - disable or enable internal cmds
                if (( argc = findcmd( arg + 1, 1 )) < 0 )
                    return ( error( ERROR_4DOS_UNKNOWN_COMMAND, arg + 1 ));
                if ( *arg == '-' )
                    commands[argc].pflag |= CMD_DISABLED;
                else
                    commands[argc].pflag &= ~CMD_DISABLED;
                break;

            case 'L':
                // L - if 1, use INT 21 0Ah for input
                // (in OS/2, use KbdStringIn)
                gpIniptr->LineIn = (char)atoi( arg );
                break;

            case 'M':
                // M - edit mode (overstrike or insert)
                gpIniptr->EditMode = (char)atoi( arg );
                break;

            case 'N':   // N - noclobber (output redirection)
                gpIniptr->NoClobber = (char)atoi( arg );
                break;

            case 'P':   // P - parameter character
                if ( isalnum( *arg ))
                    goto setdos_error;
                gpIniptr->ParamChr = *arg;
                break;

            case 'R':   // R - number of screen rows
                gpIniptr->Rows = atoi( arg );
                break;

            case 'S':   // S - cursor shape
                // if CursO or CursI == -1, don't attempt
                //   to modify cursor at all
                sscanf( arg, "%d%*1s%d", &(gpIniptr->CursO), &(gpIniptr->CursI) );

                // force cursor shape change (for BAT files)
                SetCurSize( 0 );
                break;

            case 'U':   // U - upper case filename display
                gpIniptr->Upper = (char)atoi( arg );
                break;

            case 'V':   // V - batch command echoing
                gpIniptr->BatEcho = (char)atoi( arg );
                break;

            case 'X':   // X - enable/disable expansion
                //  bit on = disable feature
                //  bit 0=aliases, 1=nested aliases
                //    2=vars, 3=nested vars
                //    4=compounds, 5=redirection
                //    6=quotes, 7=escapes

                for ( ; ( *arg != '\0' ); arg++ ) {

                    if ( is_signed_digit( *arg ) == 0 )
                    goto setdos_error;

                    if ( *arg == '-' )
                    fDisable++;
                    else if ( *arg == '+' )
                    fDisable = 0;
                    else {

                    if (( i = ( *arg - '0' )) == 0 )
                        gpIniptr->Expansion = 0;
                    else if ( fDisable )
                        gpIniptr->Expansion |= (char)(1 << (--i));
                    else
                        gpIniptr->Expansion &= ~((char)(1 << (--i)));
                    }
                }
                break;

            case 'Y':   // batch single-stepping
                gpIniptr->SingleStep = (char)atoi( arg );
                break;

            default:
                goto setdos_error;
            }
        }
        fNoComma = 0;
    }

    return 0;
}


// enable/disable disk verify
int verify_cmd( int argc, char **argv )
{
    if ( argc == 1 )        // inquiring about verify status
        printf( VERIFY_IS, gpInternalName, ( QueryVerifyWrite() ? ON : OFF ));

    else {      // setting new verify status

        if (( argc = OffOn( argv[1] )) == -1 )
            return ( usage( VERIFY_USAGE ));

        SetVerifyWrite( argc );
    }

    return 0;
}


char * GetLogName( int fHistFlag )
{
    char szLogFile[MAXFILENAME];

    // get log filename, or create one if not defined
    if ( fHistFlag ) {
        if ( gpIniptr->HistLogName == INI_EMPTYSTR) {
        sprintf( szLogFile, FMT_PATH, gpIniptr->BootDrive, HLOG_FILENAME );
        (void)ini_string( gpIniptr, ( int *)&gpIniptr->HistLogName, szLogFile, strlen( szLogFile ) );
        }
        return (char *)( gpIniptr->StrData + gpIniptr->HistLogName );
    }

    if ( gpIniptr->LogName == INI_EMPTYSTR) {
        sprintf( szLogFile, FMT_PATH, gpIniptr->BootDrive, LOG_FILENAME );
        (void)ini_string( gpIniptr,( int *)&( gpIniptr->LogName ), szLogFile, strlen( szLogFile ) );
    }

    return (char *)( gpIniptr->StrData + gpIniptr->LogName );
}


// enable/disable command logging, or rename the log file
int log_cmd( int argc, char **argv )
{
    char *pszFileName;
    char *arg, szLogName[128];
    long fLogFlags;

    // check for LOG switches /H(istory), /W(rite file name)
    if ( GetSwitches( argv[1], "HW", &fLogFlags, 1 ) != 0 )
        return ( usage(LOG_USAGE));

    pszFileName = GetLogName( (int)( fLogFlags & 1) );

    if (( arg = first_arg( argv[1] )) == NULL ) {

        // inquiring about log status
        argc = (( fLogFlags & 1 ) ? gpIniptr->HistLogOn : gpIniptr->LogOn);
        printf( LOG_IS, pszFileName, ( argc ? ON : OFF));

    } else if ( fLogFlags & 2 ) {       // change log file name?

        strcpy( szLogName, arg );

        if (( QueryIsDevice( szLogName ) == 0 ) && (mkfname( szLogName, 0 ) == NULL ))
            return ERROR_EXIT;

        if ( fLogFlags & 1 ) {
            gpIniptr->HistLogOn = 1;
            (void)ini_string( gpIniptr,( int *)&( gpIniptr->HistLogName ),szLogName,strlen( szLogName ));
        } else {
            gpIniptr->LogOn = 1;
            (void)ini_string( gpIniptr, (int *)&( gpIniptr->LogName ), szLogName, strlen( szLogName ));
        }

    } else if (( argc = OffOn( argv[1] )) == -1 ) {

        // must be entering a LOG header
        return ( _log_entry( argv[1], (int)( fLogFlags & 1 ) ));

    } else {
        // set (Hist)LogOn = 1 if "ON", 0 if "OFF"
        if ( fLogFlags & 1 )
            gpIniptr->HistLogOn = (char)argc;
        else
            gpIniptr->LogOn = (char)argc;
    }

    return 0;
}


// append the command to the log file
int _log_entry( char *cmd, int fHistoryFlag )
{
    char *pszFileName;

    // get log filename
    pszFileName = GetLogName( fHistoryFlag );

    // We can't open it in DENYRW under DOS, because SHARE has bugs!
    if (( gnGFH = _sopen( pszFileName, (O_WRONLY | O_TEXT | O_CREAT | O_APPEND), SH_DENYWR, (S_IREAD | S_IWRITE))) < 0 ) {

        if ( fHistoryFlag )
            gpIniptr->HistLogOn = 0;
        else
            gpIniptr->LogOn = 0;

        return ( error( _doserrno, pszFileName ));
    }

    if ( fHistoryFlag == 0 )
        qprintf( gnGFH, "[%s %s] ", gdate( 1 ), gtime( gaCountryInfo.fsTimeFmt ));
    qprintf( gnGFH, FMT_STR_CRLF, skipspace( cmd ));

    gnGFH = _close( gnGFH );

    return 0;
}


// flags for global execution of a command
#define GLOBAL_HIDDEN 1
#define GLOBAL_IGNORE 2
#define GLOBAL_PROMPT 4
#define GLOBAL_QUIET 8


// perform a command on the current directory & its subdirectories
int global_cmd( int argc, char **argv )
{
    char *line;
    char savedir[MAXPATH], curdir[MAXPATH];
    long fGlobal, lOffset = 0L;
    int rval;

    // save the original directory
    if (( line = gcdir( NULL, 0 )) == NULL )
        return ERROR_EXIT;

    strcpy( savedir, line );
    strcpy( curdir, line );

    // check for GLOBAL switches /H(idden), /I( gnore), /P(rompt) & /Q(uiet)
    if (( GetSwitches( argv[1], "HIPQ", &fGlobal, 1 ) != 0 ) || ( first_arg( argv[1]) == NULL ))
        return ( usage( GLOBAL_USAGE ));

    // save command line & call global recursive routine
    line = _strdup( gpNthptr );

    // test for GOTO / CANCEL / QUIT
    if ( cv.bn >= 0 )
        lOffset = bframe[cv.bn].offset;

    rval = _global( fGlobal, lOffset, curdir, line );

    if ( setjmp( cv.env ) == -1 )
        rval = CTRLC;

    free( line );

    // restore original directory
    _chdir( savedir );

    return rval;
}


// execute the command in the current directory & all its subdirectories
static int _global( unsigned long lFlags, long lOffset, char *curdir, char *line )
{
    char *dirpath;
    int i, fval, rval;
    FILESEARCH dir;
    unsigned int mode = (0x310 | FIND_CREATE);

    dir.hdir = INVALID_HANDLE_VALUE;

    // retrieve hidden directories if /H
    if ( lFlags & GLOBAL_HIDDEN)
        mode |= 0x07;

    // change to the specified directory
    if ( _chdir( curdir ) == -1 )
        return ( error( _doserrno, curdir ));

    // display subdir name unless quiet switch set
    if ((( lFlags & GLOBAL_QUIET ) == 0 ) || ( lFlags & GLOBAL_PROMPT)) {

        printf( GLOBAL_DIR, curdir );

        // prompt whether to process this directory
        if ( lFlags & GLOBAL_PROMPT) {

            if (( i = QueryInputChar( NULLSTR, YES_NO_REST )) == REST_CHAR)
                lFlags &= ~GLOBAL_PROMPT;
            else if ( i == NO_CHAR)
                goto next_global_dir;
            else if ( i == ESCAPE )
                return ERROR_EXIT;
        } else
            crlf();
    }

    // execute the command; if a non-zero result, test for /I( gnore) option
    rval = command( line, 0 );

    if (( cv.exception_flag ) || (( rval != 0 ) && (( lFlags & GLOBAL_IGNORE) == 0 )))
        return rval;

next_global_dir:

    // check for a CANCEL/QUIT during the GLOBAL
    if (( cv.bn >= 0 ) && ( bframe[cv.bn].offset != lOffset ))
        return 0;

    // make new directory search name
    mkdirname( curdir, WILD_FILE );

    // save the current subdirectory start
    dirpath = strchr( curdir, '*' );

    // search for all subdirectories in this (sub)directory
    for ( rval = 0, fval = FIND_FIRST; ; fval = FIND_NEXT ) {

        if (( setjmp( cv.env ) == -1 ) || ( cv.exception_flag )) {
            if ( dir.hdir != INVALID_HANDLE_VALUE )
                (void)DosFindClose( dir.hdir );
            rval = CTRLC;
            break;
        }

        if ( find_file( fval, curdir, mode, &dir, NULL ) == NULL )
            break;

        // make the new "curdir"
        strcpy( dirpath, dir.name );

        // process directory tree recursively
        if (( rval = _global( lFlags, lOffset, curdir, line )) != 0 )
            break;
    }

    return rval;
}


// "except" the specified files from a command by changing their attributes
//    to hidden, running the command, and then unhiding them
int except_cmd( int argc, char **argv )
{
    char *arg, *exceptlist;
    char szFileName[MAXFILENAME], *cmdlist;
    int rval = 0, fval;
    unsigned int attrib;
    FILESEARCH dir;

    // initialize date/time/size ranges
    GetRange( argv[1], &(dir.aRanges), 1 );

    // preserve file exception list
    if ( dir.aRanges.pszExclude != NULL ) {
        arg = _alloca( strlen( dir.aRanges.pszExclude ) + 1 );
        dir.aRanges.pszExclude = strcpy( arg, dir.aRanges.pszExclude );
    }

    if (( argc == 1 ) || ( argv[1][0] != '(') || (( arg = strchr( argv[1], ')' )) == NULL ) || ( *(cmdlist = skipspace( arg + 1 )) == '\0' ))
        return ( usage( EXCEPT_USAGE ));

    // terminate at the close paren ' )'
    *arg = '\0';

    // save the command onto the stack
    arg = _alloca( strlen( cmdlist ) + 1 );
    cmdlist = strcpy( arg, cmdlist );

    // do variable expansion on the exception list
    if ( var_expand( strcpy( gszCmdline, ( argv[1] ) + 1 ), 1 ) != 0 )
        return ERROR_EXIT;

    // save the exception list onto the stack
    arg = _alloca( strlen( gszCmdline ) + 1 );
    exceptlist = strcpy( arg, gszCmdline );

    if ( setjmp( cv.env ) == -1 )
        rval = CTRLC;

    else {

        // hide away the "excepted" files
        for ( argc = 0; (( rval == 0 ) && (( arg = ntharg( exceptlist, argc )) != NULL )); argc++) {

            for ( fval = FIND_FIRST; ( find_file( fval, arg, 0x8810, &dir, szFileName ) != NULL ); fval = FIND_NEXT) {

                if (( rval = QueryFileMode( szFileName, &attrib )) == 0 ) {
                    // can't set directory attribute!
                    attrib &= ( _A_SUBDIR ^ 0xFFFF);
                    rval = SetFileMode( szFileName, ( attrib | _A_HIDDEN) );
                }

                if ( rval != 0 ) {
                    error( rval, szFileName );
                    break;
                }
            }
        }

        if ( rval == 0 )
            rval = command( cmdlist, 0 );

        if ( setjmp( cv.env ) == -1 )
            rval = CTRLC;
    }

    for ( argc = 0; (( arg = ntharg( exceptlist, argc )) != NULL ); argc++ ) {

        // unhide all of the "excepted" files
        for ( fval = FIND_FIRST; ( find_file( fval, arg, 0x8117, &dir, szFileName ) != NULL ); fval = FIND_NEXT ) {

            if ( QueryFileMode( szFileName, &attrib ) == 0 ) {
                // can't set directory attribute!
                attrib &= ( _A_SUBDIR ^ 0xFFFF );
                (void)SetFileMode( szFileName, ( attrib & ( _A_HIDDEN ^ 0xFF)));
            }
        }
    }

    return rval;
}


#define TIMER_1 1
#define TIMER_2 2
#define TIMER_3 4
#define TIMER_SPLIT_TIME  8

// Stopwatch - display # of seconds between calls
int timer_cmd( int argc, char **argv )
{
    int tnum = 0;       // timer # (1, 2, or 3 - base 0 )
    long fTimer;
    char szBuffer[16];

    // get the timer number; default to /1 if none specified
    if ( GetSwitches( argv[1], "123S", &fTimer, 0 ) != 0 )
        return ( usage( TIMER_USAGE ));

    if ( fTimer & TIMER_2 )
        tnum = 1;
    else if ( fTimer & TIMER_3 )
        tnum = 2;

    printf( TIMER_NUMBER, tnum + 1 );

    if (( argv[1] = first_arg( argv[1] )) == NULL )
        argv[1] = NULLSTR;

    if (( gaTimers[tnum].timer_flag == 0 ) || ( _stricmp( argv[1], ON ) == 0 )) {

        DATETIME sysDateTime;

        QueryDateTime( &sysDateTime );

        printf( TIMER_ON, gtime( gaCountryInfo.fsTimeFmt ));

        // start timer - save current time
        gaTimers[tnum].timer_flag = 1;
        gaTimers[tnum].thours = sysDateTime.hours;
        gaTimers[tnum].tminutes = sysDateTime.minutes;
        gaTimers[tnum].tseconds = sysDateTime.seconds;
        gaTimers[tnum].thundreds = sysDateTime.hundredths;

    } else {        // timer toggled off

        // check for split time; turn off timer if not
        if (( fTimer & TIMER_SPLIT_TIME ) == 0 )
            printf( TIMER_OFF, gtime( gaCountryInfo.fsTimeFmt ));

        _timer( tnum, szBuffer );
        printf( TIMER_ELAPSED, szBuffer );

        if (( fTimer & TIMER_SPLIT_TIME ) == 0 )
            gaTimers[tnum].timer_flag = 0;
    }

    return 0;
}


void _timer( int nTnum, char *pszBuffer)
{
    int shours = 0, sminutes = 0, sseconds = 0, shundreds = 0;
    DATETIME sysDateTime;

    // save the split time
    if ( gaTimers[nTnum].timer_flag != 0 ) {

        QueryDateTime( &sysDateTime );

        shours = sysDateTime.hours - gaTimers[nTnum].thours;
        sminutes = sysDateTime.minutes - gaTimers[nTnum].tminutes;
        sseconds = sysDateTime.seconds - gaTimers[nTnum].tseconds;
        shundreds = sysDateTime.hundredths - gaTimers[nTnum].thundreds;
    }

    // adjust negative fractional times
    if ( shundreds < 0 ) {
        shundreds += 100;
        sseconds--;
    }

    if ( sseconds < 0 ) {
        sseconds += 60;
        sminutes--;
    }

    if ( sminutes < 0 ) {
        sminutes += 60;
        shours--;
    }

    if ( shours < 0 )
        shours += 24;

    sprintf( pszBuffer, TIMER_SPLIT, shours, gaCountryInfo.szTimeSeparator[0], sminutes, gaCountryInfo.szTimeSeparator[0], sseconds, gaCountryInfo.szDecimal[0], shundreds);
}


// return the current date in the formats:
//  format_type == 0 : "Mon  Jan 1, 1996"
//  format_type == 1 : " 1/01/96"
//      format_type == 2 : "Mon 1/01/96"
char * gdate( int format_type )
{
    static char szDate[20];
    DATETIME sysDateTime;

    QueryDateTime( &sysDateTime );

    if ( format_type == 1 )
        return ( FormatDate( sysDateTime.month, sysDateTime.day, sysDateTime.year ));

    else if ( gaCountryInfo.fsDateFmt != 1 ) {
        // USA or Japan
        sprintf( szDate, "%s  %s %u, %4u", daytbl[(int)sysDateTime.weekday], montbl[(int)sysDateTime.month-1], (int)sysDateTime.day, sysDateTime.year );
    } else {
        // Europe
        sprintf( szDate, "%s  %u %s %4u", daytbl[(int)sysDateTime.weekday],(int)sysDateTime.day, montbl[(int)sysDateTime.month-1], sysDateTime.year );
    }

    return szDate;
}


// return the current time as a string in the format "12:45:07"
char * gtime( int TimeFmt )
{
    static char szTime[10];
    DATETIME sysDateTime;

    QueryDateTime( &sysDateTime );

    // check for 12 hour format
    if ( TimeFmt == 0 ) {
        TimeFmt = 'p';
        if ( sysDateTime.hours < 12 ) {
            if ( sysDateTime.hours == 0 )
                sysDateTime.hours = 12;
            TimeFmt = 'a';
        } else if ( sysDateTime.hours > 12 )
            sysDateTime.hours -= 12;
    } else
        TimeFmt = 0;

    // get the time format for the default country
    sprintf( szTime, TIME_FMT, sysDateTime.hours, gaCountryInfo.szTimeSeparator[0],sysDateTime.minutes, gaCountryInfo.szTimeSeparator[0], sysDateTime.seconds, TimeFmt );

    return szTime;
}


// set new system date
int setdate_cmd( int argc, char **argv )
{
    unsigned int day, month, year;
    char szBuf[12];
    DATETIME sysDateTime;

    if ( argc > 1 ) {
        // date already in command line, so don't ask for it
        sprintf( szBuf, FMT_PREC_STR, 10, argv[1] );
        goto got_date;
    }

    // display current date & time
    printf( "%s  %s", gdate( 0 ), gtime( gaCountryInfo.fsTimeFmt ) );

    for ( ; ; ) {

        printf( NEW_DATE, dateformat[ gaCountryInfo.fsDateFmt ] );
        if ( egets( szBuf, 10, EDIT_DATA ) == 0 )
            break;
got_date:
        // valid date entry?
        if ( GetStrDate( szBuf, &month, &day, &year ) == 3 ) {

            // SetDateTime() requires date & time together
            QueryDateTime( &sysDateTime );

            if (( sysDateTime.year = year ) < 80 )
                sysDateTime.year += 2000;
            else if ( sysDateTime.year < 100 )
                sysDateTime.year += 1900;

            sysDateTime.month = (unsigned char)month;
            sysDateTime.day = (unsigned char)day;

            if ( SetDateTime( (DATETIME *)&sysDateTime ) == 0 )
                break;
        }

        error( ERROR_4DOS_INVALID_DATE, szBuf );
    }

    return 0;
}


// set new system time
int settime_cmd( int argc, char **argv )
{
    char *am_pm;
    unsigned int hours, minutes, seconds;
    char szBuf[12];
    DATETIME sysDateTime;

    if ( argc > 1 ) {
        // time already in command line, so don't ask again
        sprintf( szBuf, FMT_PREC_STR, 10, argv[1] );
        goto got_time;
    }

    // display current date & time
    printf( "%s  %s", gdate( 0 ), gtime( gaCountryInfo.fsTimeFmt ) );

    for ( ; ; ) {

        printf( NEW_TIME );
        if ( egets( szBuf, 10, EDIT_DATA ) == 0 )
            return 0;   // quit or no change
got_time:
        seconds = 0;
        if ( sscanf( szBuf, DATE_FMT, &hours, &minutes, &seconds ) >= 2) {

            // check for AM/PM syntax
            if (( am_pm = strpbrk( strupr( szBuf ), "AP" )) != NULL ) {
                if (( hours == 12 ) && ( *am_pm == 'A' ))
                    hours -= 12;
                else if (( *am_pm == 'P' ) && ( hours < 12 ))
                    hours += 12;
            }

            // SetDateTime requires date & time together
            QueryDateTime( &sysDateTime );

            sysDateTime.hours = (unsigned char)hours;
            sysDateTime.minutes = (unsigned char)minutes;
            sysDateTime.seconds = (unsigned char)seconds;
            sysDateTime.hundredths = 0;

            if ( SetDateTime( (DATETIME *)&sysDateTime ) == 0 )
                break;
        }

        error( ERROR_4DOS_INVALID_TIME, szBuf );
    }

    return 0;
}


// display or set the code page (only for DOS 3.3+, OS/2 & NT)
int chcp_cmd( int argc, char **argv )
{
    int rval = 0;

    if ( argc == 1 ) {

        unsigned long numCP, CodePages[32];

        if (( rval = DosQueryCp( sizeof(CodePages), CodePages, &numCP )) != 0 ) {
            rval = error( rval, NULL );

        } else {

            // CodePage support installed?
            if ( CodePages[0] == 0 )
                rval = error( ERROR_KBD_NO_CODEPAGE_SUPPORT, NULL );
            else {
                numCP >>= 2;
                printf( CODE_PAGE, QueryCodePage() );
                printf( PREPARED_CODE_PAGES );

                for ( argc = 1; ( argc < (int)numCP ); argc++ )
                    printf( " %u", CodePages[argc] );
                crlf();
            }
        }
    }

    if (( argv[1] != NULL ) && (( rval = SetCodePage( atoi( argv[1] ))) != 0 ))
        rval = error( rval, argv[1] );

    return rval;
}


// call the external Help (4HELP.EXE or VIEW.EXE)
int help_cmd( int argc, char **argv )
{
    static char HELP_CMD[] = "HELP.CMD";
    char *arg;

    if (( arg = argv[1] ) != NULL ) {

        // support HELP ON or HELP OFF in HELP.CMD file
        if (( _stricmp( arg, ON ) == 0 ) || ( _stricmp( arg, OFF ) == 0 )) {
            if (( argv[0] = searchpaths( HELP_CMD, NULL, TRUE )) != NULL ) {
                gpBatchName = HELP_CMD;
                return ( batch( 2, argv ));
            }
        }

        // kludge to support "HELP 5" or "HELP SYS0005" syntax
        if (( isdigit( arg[0] )) || ( isalpha( arg[0] ) && isalpha( arg[1] ) && isalpha( arg[2] ) && isdigit( arg[3] ))) {
            if (( argv[0] = searchpaths( "HELPMSG", NULL, TRUE )) != NULL )
                return (external( 2, argv ));
        }
    }

    return ( _help( argv[1], NULL ));
}


//FIXME -- For clarity, eventually we may need to break code below into xxxxCMDS.C
// Set an INI file option
int option_cmd( int argc, char **argv )
{
    int rval;
    int arglen;
    char *arg;
    char *nextarg, *errmsg, szIniArg[513];

    // In 4xxx we use the external OPTION.EXE program
    // In 4OS2 and 4NT, only use OPTION.EXE if no arguments were given
    if ( argc == 1 ) {
        if (( rval = _option()) != 0 )
            return rval;
    }

    // If we are not in 4DOS, and there are arguments, parse them
    else {

        // Holler if first argument does not start with //
        arg = argv[1];
        if (( *arg != '/' ) || ( arg[1] != '/' ))
            return ( error( ERROR_4DOS_BAD_SYNTAX, arg ));

        // Find each argument that starts with "//" and parse as an
        // INI file line
        while ( *arg != '\0' ) {
            arg += 2;
            if (( nextarg = strstr( arg, "//" )) == NULL )
                nextarg = strend( arg );
            arglen = nextarg - arg;
            strncpy( szIniArg, arg, arglen );
            szIniArg[arglen] = '\0';
            if ( IniLine( szIniArg, &gaInifile, 0, 0, 1, &errmsg )) {
                arg[arglen] = '\0';
                return ( error( ERROR_4DOS_BAD_DIRECTIVE, arg ));
            }

            arg = nextarg;
        }
    }

//FIXME -- Need code here to reset option-dependent states:  colors, cursor shape,
//etc.  May need to be different if we are at the prompt vs. in a batch file.

// For 4OS2 and 4NT we should at least reset history / dir history

    QueryCountryInfo();
    SetOSVersion();

    return 0;
}

