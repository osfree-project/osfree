// OPTION2 - Option.Exe for 4OS2
//   (c) 1997  Rex C. Conn  All rights reserved

#include "product.h"

#define INCL_BASE
#define INCL_PM
#include <osfree.h>

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <share.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <process.h>

#include "build.h"

// Constants from 4OS2.H
#define MAXFILENAME     260

// INI file data areas
#include "resource.h"
#include "inistruc.h"

INIFILE gaInifile;
INIFILE *gpIniptr = (INIFILE *)&gaInifile;
char gaIniStrings[INI_STRMAX];
UINT gaIniKeys[2 * INI_KEYMAX];

#define INIPARSE 1
#include "inifile.h"

//#define ID_RESOURCE 1

char * path_part( char * );
extern MRESULT EXPENTRY ClientWndProc( HWND, ULONG, MPARAM, MPARAM );

HWND hParentFrame;
static HAB hab;
static char *pHelpName;
static char szIniName[260];
static char *pszShareName;
static PIB *ppib;


int main( int argc, char **argv )
{
        extern void ConfigNotebook(void);
        extern char *_LpPgmName;

        static char szClientClass[] = "OPTION4O.S2";
        static ULONG flFrameFlags = FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER |
                FCF_MINMAX | FCF_SHELLPOSITION | FCF_TASKLIST;
        int rval = 0;
        HMQ hmq;
        HWND hWndClient;
        QMSG qmsg;
        PTIB ptib;
        char *pIniData = 0L, *pIniStrings, *pIniKeys;
        char szErrorBuf[256];
        INIFILE *pTempIni;

        hab = WinInitialize( 0 );
        hmq = WinCreateMsgQueue( hab, 0L );
        (void)DosGetInfoBlocks( &ptib, &ppib );

        // set 4OS2.INI name
        sprintf( szIniName, "%s4OS2.INI", path_part( _LpPgmName ));

        WinRegisterClass( hab, szClientClass, ClientWndProc, CS_SIZEREDRAW, 0 );
        hParentFrame = WinCreateStdWindow( HWND_DESKTOP, 0, &flFrameFlags, szClientClass, NULL, 0L, 0, 0, &hWndClient );

        // Make sure we got the shared memory name, and that the memory is available
        if ((( pszShareName = argv[1] ) == (void *)0L/*NULL*/ ) || ( DosGetNamedSharedMem( (PPVOID)&pIniData, pszShareName, PAG_READ | PAG_WRITE ) != 0 )) {

                WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, "This program must be started from the 4OS2 OPTION command.", "OPTION2.EXE", 0, ( MB_OK | MB_ERROR ));
                rval = 1;

        } else {

                // Save help name for _help()
                pHelpName = argv[2];

                // See if we were passed valid data
                pTempIni = (INIFILE *)pIniData;

                if (( pTempIni->INISig != 0x4DD4 ) || ( pTempIni->INIBuild != VER_BUILD )) {

                        sprintf( szErrorBuf, "Your copy of OPTION.EXE is not compatible with this version of 4OS2 (%04X  %d/%d).", pTempIni->INISig, pTempIni->INIBuild, VER_BUILD );
                        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, szErrorBuf, "OPTION", 0, ( MB_OK | MB_ERROR ));
                        rval = 1;

                } else {

                        // Set up pointers to shared memory
                        pIniStrings = pIniData + sizeof(INIFILE);
                        pIniKeys = pIniStrings + INI_STRMAX;

                        // Copy data from shared memory, set local pointers
                        memmove( (char *)gpIniptr, pIniData, sizeof(INIFILE));
                        gaInifile.StrData = (char *)memmove( gaIniStrings, pIniStrings, INI_STRMAX);
                        gaInifile.Keys = (unsigned int *)memmove( (char *)gaIniKeys, pIniKeys, INI_KEYS_SIZE);
                }
        }

        if (rval == 0) {

                WinPostMsg( hParentFrame, WM_USER, 0, 0 );

                while ( WinGetMsg( hab, &qmsg, NULLHANDLE, 0, 0 ))
                        WinDispatchMsg( hab, &qmsg );

                // Copy modified INIFILE data back to shared memory
                memmove( pIniData, (char *)gpIniptr, sizeof(INIFILE) );
                memmove( pIniStrings, gpIniptr->StrData, INI_STRMAX);
                memmove( pIniKeys, (char *)(gpIniptr->Keys), INI_KEYS_SIZE);

                // Clean up
                DosFreeMem( pIniData );
                WinDestroyWindow( hParentFrame );
                WinDestroyMsgQueue( hmq );
                WinTerminate( hab );
        }

        return rval;
}


MRESULT EXPENTRY ClientWndProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
        switch (msg) {
        case WM_USER:
                ConfigNotebook();
                WinPostMsg( hParentFrame, WM_QUIT, 0, 0 );
                return 0;
        }

        return ( WinDefWindowProc( hwnd, msg, mp1, mp2 ));
}


// free OS2 memory block
void FreeMem( char *fptr )
{
        if ( fptr != 0L )
                (void)DosFreeMem( fptr );
}


// allocate OS2 memory
char *AllocMem(unsigned int *size )
{
        PVOID BaseAddress = 0L;

        // allocate (but don't commit!) 128K so we'll have lots of room
        if ( DosAllocMem( &BaseAddress, 0x20000L, (PAG_WRITE | PAG_READ)) == 0 )
                (void)DosSetMem( BaseAddress, (ULONG)*size, (PAG_READ | PAG_WRITE | PAG_COMMIT ));

        return (char *)BaseAddress;
}


// strip the specified leading characters
void StripLeading( char *arg, char *delims )
{
        while (( *arg != '\0' ) && ( strchr( delims, *arg ) != NULL ))
                strcpy( arg, arg+1 );
}


// strip the specified trailing characters
void StripTrailing(char *arg, char *delims)
{
        int i;

        for ( i = strlen( arg ); ((--i >= 0 ) && ( strchr( delims, arg[i] ) != NULL )); )
                arg[i] = '\0';
}


// get command line from console or file
int getline( int fd, char *line, int nMaxSize )
{
        int i;

        // get a line and set the file pointer to the next line
        nMaxSize = read( fd, line, nMaxSize );

        for ( i = 0; ; i++, line++ ) {

                if (( i >= nMaxSize ) || ( *line == 26 ))
                        break;

                // skip a LF following a CR or LF
                if (( *line == '\r' ) || ( *line == '\n' )) {
                        if (( ++i < nMaxSize ) && ( line[1] == '\n' ))
                                i++;
                        break;
                }
        }

        // truncate the line
        *line = '\0';

        if ( i >= 0 )
                (void)_lseek( fd, (long)( i - nMaxSize ), SEEK_CUR );

        return i;
}


// make up for the lack of OS/2 APIs!

// write a string to the .INI file
void TCWritePrivateProfileStr( char *pszSection, char *pszItem, char *pszBuffer )
{
        int i;
        int fd, nLength;
        unsigned int uSize;
        long lOffset = 0L;
        char szBuffer[1024], szOutput[1024], *pchTail;

        if ( pszBuffer == NULL )
                pszBuffer = "";

        // open the INI file
        if (( fd = sopen( szIniName, (O_RDWR | O_BINARY | O_CREAT), SH_DENYWR, (S_IREAD | S_IWRITE) )) <= 0 ) {
                return;
        }

        if ( pszItem != NULL )
                nLength = strlen( pszItem );

        while ( getline( fd, szBuffer, 1023 ) > 0 ) {

                StripLeading( szBuffer, " \t[" );
                StripTrailing( szBuffer, " \t]" );
                if ( stricmp( szBuffer, pszSection ) == 0 ) {

                        for ( ; ; ) {

                                if (( pszItem != NULL ) && ( szBuffer[0] ))
                                        lOffset = _lseek( fd, 0L, SEEK_CUR );
                                szBuffer[0] = '\0';
                                i = getline( fd, szBuffer, 1023 );

                                StripLeading( szBuffer, " \t" );
                                if (( i <= 0 ) || ( szBuffer[0] == '[' ) || (( pszItem != NULL ) && ( strnicmp( szBuffer, pszItem, nLength ) == 0 ) && ( szBuffer[ nLength ] == '=' ))) {

                                    // save remainder of file
                                    uSize = 32768;
                                    pchTail = AllocMem( &uSize );
                                    nLength = read( fd, pchTail, uSize );
                                    _lseek( fd, lOffset, SEEK_SET );

                                    if ( pszItem != NULL ) {
                                        // write new line
                                        sprintf( szOutput, "%s=%s\r\n", pszItem, pszBuffer );
                                        write( fd, szOutput, strlen(szOutput));
                                    }

                                    // if entry didn't exist, it was just
                                    //   inserted & we need to move the
                                    //   next section down
                                    if ( szBuffer[0] == '[' ) {
                                        sprintf( szOutput, "\r\n%s\r\n", szBuffer );
                                        write( fd, szOutput, strlen(szOutput));
                                    }

                                    // append tail
                                    if ( nLength > 0 )
                                        write( fd, pchTail, nLength );

                                    // truncate file
                                    lOffset = _lseek( fd, 0L, SEEK_CUR );
                                    (void)chsize( fd, lOffset );
                                    FreeMem( pchTail );
                                    goto TCWBye;
                                }
                        }
                }

                if ( pszItem == NULL )
                        lOffset = _lseek( fd, 0L, SEEK_CUR );
        }

        // write new section
        if ( pszItem != NULL ) {
                sprintf( szOutput, "[%s]\r\n%s=%s\r\n", pszSection, pszItem, pszBuffer );
                write( fd, szOutput, strlen(szOutput));
        }
TCWBye:
        close( fd );
}


// return the path stripped of the filename (or NULL if no path)
char * path_part( char *s )
{
        static char buffer[260];

        strcpy( buffer, s );

        // search path backwards for beginning of filename
        for ( s = buffer + strlen(buffer); ( --s >= buffer ); ) {

                // accept either forward or backslashes as path delimiters
                if (( *s == '\\' ) || ( *s == '/' ) || ( *s == ':' )) {
                        // take care of arguments like "d:.." & "..\.."
                        if ( _stricmp( s+1, ".." ) != 0 )
                                s[1] = '\0';
                        return buffer;
                }
        }

        return NULL;
}


// call VIEW.EXE
int _help(char *arg, char *opts)
{
        char argline[128];
        PROGDETAILS Details;

        // initialize start structure
        memset( &Details, '\0', sizeof(PROGDETAILS) );

        Details.Length = sizeof(PROGDETAILS);
        Details.progt.progc = PROG_PM;
        Details.progt.fbVisible = SHE_VISIBLE;
        Details.swpInitial.fl = SWP_ACTIVATE | SWP_SHOW | SWP_SIZE | SWP_MOVE;
        Details.pszEnvironment = ppib->pib_pchenv;
        Details.pszStartupDir = "";
        Details.swpInitial.hwndInsertBehind = HWND_TOP;

        // Check if we know the helpo name
        if ((Details.pszExecutable = pHelpName) == NULL) {
                DosBeep( 440, 54 );
                return 1;
        }

        // Create command line
        sprintf( argline, "4OS2 %.*s", 120, arg );

        // start VIEW.EXE in the desktop window
        if ( WinStartApp( hParentFrame, &Details, argline, NULL, 0 ) == (HAPP)NULL )
                return ( WinGetLastError( hab ));
        return 0;
}

