/*
   KEYSTACK hook/unhook code for TCMD/OS2 and 4OS2
   Copyright 1997  Rex C. Conn for JP Software Inc.  All rights reserved
*/

#include <string.h>

#define INCL_BASE
#define INCL_PM
#include <os2.h>


typedef VOID (APIENTRY STHOOK)( HMQ );
typedef ULONG (EXPENTRY KSHOOK)( HAB, BOOL, PQMSG );

int main( void )
{
        int rval = 0;
        char szBuf[260];
        HMODULE hModule;
        QMSG msg;
        HAB hHAB;
        HMQ hHMQ;
        STHOOK *pfnStartHook = 0L;
        KSHOOK *pfnKeystackHookProc = 0L;

        hHAB = WinInitialize( 0 );
        hHMQ = WinCreateMsgQueue( hHAB, 0L );

        // Look for our DLL in the LIBPATH
        if (( DosLoadModule( szBuf, sizeof( szBuf ), "JPOS2DLL", &hModule ) != 0 ) ||
            ( DosQueryProcAddr( hModule, 0, "STARTHOOK", (PFN *)&pfnStartHook ) != 0 ) || ( DosQueryProcAddr( hModule, 0, "KEYSTACKHOOKPROC", (PFN *)&pfnKeystackHookProc ) != 0 )) {
                rval = WinGetLastError( hHAB );
                goto KeystackExit;
        }

        // call .DLL
        (*pfnStartHook)( hHMQ );

        // Install the Journal Playback hook.
        if ( WinSetHook( hHAB, NULLHANDLE, HK_JOURNALPLAYBACK, (PFN)pfnKeystackHookProc, hModule ) == 0 )
                rval = WinGetLastError( hHAB );

        else {

                while ( WinGetMsg( hHAB, &msg, 0, 0, 0 )) {
                        if ( msg.msg == WM_USER )
                                break;
                        WinDispatchMsg( hHAB, &msg );
                }

                WinReleaseHook( hHAB, 0, HK_JOURNALPLAYBACK, (PFN)pfnKeystackHookProc, hModule );
        }

KeystackExit:
        WinDestroyMsgQueue( hHMQ );
        WinTerminate( hHAB );

        return rval;
}

