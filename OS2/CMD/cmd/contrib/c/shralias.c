/*
   SHRALIAS for 4OS2 and Take Command for OS/2

   Copyright 1997  JP Software Inc.  All rights reserved

   This program hooks the Alias, History, and Directory History
   shared memory segments so that they don't disappear with the last
   4OS2 or Take Command for OS/2 shell.

   It should only be run from 4OS2 or Take Command.
*/

#define INCL_BASE
#include <os2.h>

#include "shralias.h"


char *SharedAliases = SHR_4OS2_ALIAS;
char *SharedHistory = SHR_4OS2_HISTORY;
char *SharedDirHist = SHR_4OS2_DIRHIST;
char *SentinelSem = SHR_4OS2_SENTINEL;
char BadRun[] = "SHRALIAS.EXE cannot be run manually -- you must use the 4OS2 or Take Command\r\nSHRALIAS command.  See the SHRALIAS command in the online help for details.\r\n";

int main( int argc, char **argv )
{
    unsigned int nAlias, nHistory, nDirectory;
    unsigned long ulDummy;
    PCH AliasList, HistoryList, DirectoryList;
    HEV hev = 0;

    // check for reasonable-looking parameters
    if ( ( argc <= 1 ) || ( argv[1][0] != '/' ) || (( argv[1][1] != '4') && ( argv[1][1] != 't' )) ) {
        DosWrite(2, BadRun, sizeof(BadRun) - 1, &ulDummy);
        return 1;
    }

#if 0
    // check for invocation by Take Command
    if ( argv[1][1] == 't' ) {
        SharedAliases = SHR_TCOS2_ALIAS;
        SharedHistory = SHR_TCOS2_HISTORY;
        SharedDirHist = SHR_TCOS2_DIRHIST;
        SentinelSem = SHR_TCOS2_SENTINEL;
    }
#endif

    // try to attach to the 4OS2 or Take Command shared memory areas
    nAlias = DosGetNamedSharedMem( (PVOID)&AliasList, SharedAliases, PAG_READ | PAG_WRITE );
    nHistory = DosGetNamedSharedMem( (PVOID)&HistoryList, SharedHistory, PAG_READ | PAG_WRITE );
    nDirectory = DosGetNamedSharedMem( (PVOID)&DirectoryList, SharedDirHist, PAG_READ | PAG_WRITE );

    if ( ( nAlias == 0) || ( nHistory == 0 ) || ( nDirectory == 0 ) ) {

        // create the sentinel semaphore
        if ( DosCreateEventSem( SentinelSem, &hev, 0, 0 ) == 0 ) {

            // wait forever on sentinel semaphore - if it's
            //   posted, then an unload (/U) is in process
            DosWaitEventSem( hev, (unsigned long int) -1 );
            DosCloseEventSem( hev );
        }

        // free up any shared memory we managed to attach to
        if ( nAlias == 0 )
            DosFreeMem( AliasList );
        if ( nHistory == 0 )
            DosFreeMem( HistoryList );
        if ( nDirectory == 0 )
            DosFreeMem( DirectoryList );
    }

    return 0;
}
