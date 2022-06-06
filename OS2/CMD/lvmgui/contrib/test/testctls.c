#include "lvm_ctls.h"

#define WINDOWCLASS "LvmControls"

#define ID_DISKLIST 1000
#define ID_BTNC     1100
#define ID_BTND     1101
#define ID_BTNE     1102
#define ID_BTNF     1103
#define ID_BTNG     1104

// dummy LVM handles
#define LH_PARTC    100
#define LH_PARTD    101
#define LH_PARTE    102
#define LH_PARTF    200
#define LH_PARTG    104

#define LH_DISK1    10000
#define LH_DISK2    10001
#define LH_DISK3    10002


MRESULT EXPENTRY MainWndProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );


HWND panel1 = 0,
     panel2 = 0,
     panel3 = 0;

HWND disklist = 0;

HWND btnC, btnD, btnE, btnF, btnG;

HWND hwndDiskPopup, hwndPartPopup;

int main( int argc, char *argv[] )
{
    HAB       hab;                    // anchor block handle
    HMQ       hmq;                    // message queue handle
    HWND      hwndFrame = 0,          // window handle
              hwndClient = 0;         // client area handle
    QMSG      qmsg;                   // message queue
    CHAR      szError[ 256 ];         // error message buffer
    ULONG     flStyle = FCF_TITLEBAR | FCF_SYSMENU | FCF_MINMAX | FCF_SIZEBORDER |
                        FCF_ICON | FCF_AUTOICON | FCF_TASKLIST;
    BOOL      fInitFailure = FALSE;
    HPOINTER  hicon;


    hab = WinInitialize( 0 );
    if ( hab == NULLHANDLE ) {
        sprintf( szError, "WinInitialize() failed.");
        fInitFailure = TRUE;
    }
    else {
        hmq = WinCreateMsgQueue( hab, 0 );
        if ( hmq == NULLHANDLE ) {
            sprintf( szError, "Unable to create message queue:\nWinGetLastError() = 0x%X\n", WinGetLastError(hab) );
            fInitFailure = TRUE;
        }
    }

    if ( !fInitFailure && !DLRegisterClass( hab )) {
        sprintf( szError, "Failed to register class %s:\nWinGetLastError() = 0x%X\n", WC_LVMDISKS, WinGetLastError(hab) );
        fInitFailure = TRUE;
    }
    if (( !fInitFailure ) &&
        ( !WinRegisterClass( hab, WINDOWCLASS, MainWndProc, CS_SIZEREDRAW, 0 )))
    {
        sprintf( szError, "Failed to register class %s:\nWinGetLastError() = 0x%X\n", WINDOWCLASS, WinGetLastError(hab) );
        fInitFailure = TRUE;
    }
    if ( !fInitFailure ) {
        hwndFrame = WinCreateStdWindow( HWND_DESKTOP, 0L, &flStyle,
                                        WINDOWCLASS, "Test LVM Controls", 0L,
                                        NULLHANDLE, 1, &hwndClient );
        if (( hwndFrame == NULLHANDLE ) || ( hwndClient == NULLHANDLE )) {
            sprintf( szError, "Failed to create application window:\nWinGetLastError() = 0x%X\n", WinGetLastError(hab) );
            fInitFailure = TRUE;
        }
    }

    if ( fInitFailure ) {
        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, szError, "Program Initialization Error", 0, MB_CANCEL | MB_ERROR );
    } else {
#if 0
/*
        PVCTLDATA ctldata = {0};

        ctldata.cb      = sizeof( PVCTLDATA );
        ctldata.fl      = LPV_FL_EMPHASIS;
        ctldata.bType   = LPV_TYPE_PRIMARY;
        ctldata.bOS     = 1;
        ctldata.ulSize  = 10000;
        ctldata.cLetter = 'C';
        sprintf( ctldata.szName, "Test Partition 1");
        panel1 = WinCreateWindow( hwndClient, WC_PARTITIONVIEW, "", WS_VISIBLE,
                                  0, 0, 100, 100, hwndClient, HWND_TOP, 100, &ctldata, NULL );

        ctldata.fl      = 0;
        ctldata.bType   = LPV_TYPE_LOGICAL;
        ctldata.bOS     = 5;
        ctldata.ulSize  = 24000;
        ctldata.cLetter = 'M';
        sprintf( ctldata.szName, "Test Partition Two");
        panel2 = WinCreateWindow( hwndClient, WC_PARTITIONVIEW, "", WS_VISIBLE,
                                  101, 0, 100, 100, hwndClient, HWND_TOP, 100, &ctldata, NULL );
*/

        DVCTLDATA ctldata = {0};
        PVCTLDATA aParts[ 5 ] = {0};

        ctldata.cb = sizeof( DVCTLDATA );
        ctldata.ulSize = 512000;
        ctldata.number = 1;
        sprintf( ctldata.szName, "MY TEST HARD DISK 1");
        panel1 = WinCreateWindow( hwndClient, WC_DISKVIEW, "", WS_VISIBLE,
                                  10, 10, 0, 0, hwndClient, HWND_TOP, 100, &ctldata, NULL );
        WinSetPresParam( panel1, PP_FONTNAMESIZE, 11, (PVOID)"9.WarpSans");

        aParts[0].cb      = sizeof( PVCTLDATA );
        aParts[0].bType   = LPV_TYPE_PRIMARY;
        aParts[0].bOS     = 1;
        aParts[0].ulSize  = 100000;
        aParts[0].cLetter = 'C';
        aParts[0].number  = 1;
        aParts[0].handle  = 100;
        sprintf( aParts[0].szName, "Partition 1");
        aParts[1].cb      = sizeof( PVCTLDATA );
        aParts[1].bType   = LPV_TYPE_PRIMARY;
        aParts[1].bOS     = 1;
        aParts[1].ulSize  = 150000;
        aParts[1].cLetter = 'D';
        aParts[1].number  = 2;
        aParts[1].handle  = 101;
        sprintf( aParts[1].szName, "Partition 2");
        aParts[2].cb      = sizeof( PVCTLDATA );
        aParts[2].bType   = LPV_TYPE_LOGICAL;
        aParts[2].bOS     = 1;
        aParts[2].ulSize  = 150000;
        aParts[2].cLetter = 'E';
        aParts[2].number  = 3;
        aParts[2].handle  = 102;
        sprintf( aParts[2].szName, "Partition 3");
        aParts[3].cb      = sizeof( PVCTLDATA );
        aParts[3].bType   = LPV_TYPE_FREE;
        aParts[3].bOS     = 1;
        aParts[3].ulSize  = 12000;
        aParts[3].cLetter = 0;
        aParts[3].number  = 4;
        aParts[3].handle  = 103;
        sprintf( aParts[3].szName, "");
        aParts[4].cb      = sizeof( PVCTLDATA );
        aParts[4].bType   = LPV_TYPE_LOGICAL;
        aParts[4].bOS     = 1;
        aParts[4].ulSize  = 100000;
        aParts[4].cLetter = 'G';
        aParts[4].number  = 5;
        aParts[4].handle  = 104;
        sprintf( aParts[4].szName, "Partition 5");

        WinSendMsg( panel1, LDM_SETPARTITIONS, MPFROMLONG(5), MPFROMP(aParts) );

        hicon = WinLoadPointer( HWND_DESKTOP, NULLHANDLE, 10 );
        if ( hicon == NULLHANDLE ) ErrorPopup("No icon");
        WinSendMsg( panel1, LDM_SETDISKICON, MPFROMP(hicon), 0 );
#else

        DVCTLDATA aDisks[ 3 ]  = {0};
        PVCTLDATA aParts1[ 5 ] = {0},
                  aParts2[ 3 ] = {0},
                  aParts3[ 1 ] = {0};


        btnC = WinCreateWindow( hwndClient, WC_BUTTON, "C", BS_PUSHBUTTON | WS_GROUP,
                                0, 0, 0, 0, hwndClient, HWND_TOP, ID_BTNC, NULL, NULL );
        btnD = WinCreateWindow( hwndClient, WC_BUTTON, "D", BS_PUSHBUTTON,
                                0, 0, 0, 0, hwndClient, HWND_TOP, ID_BTND, NULL, NULL );
        btnE = WinCreateWindow( hwndClient, WC_BUTTON, "E", BS_PUSHBUTTON,
                                0, 0, 0, 0, hwndClient, HWND_TOP, ID_BTNE, NULL, NULL );
        btnF = WinCreateWindow( hwndClient, WC_BUTTON, "F", BS_PUSHBUTTON,
                                0, 0, 0, 0, hwndClient, HWND_TOP, ID_BTNF, NULL, NULL );
        btnG = WinCreateWindow( hwndClient, WC_BUTTON, "G", BS_PUSHBUTTON,
                                0, 0, 0, 0, hwndClient, HWND_TOP, ID_BTNG, NULL, NULL );

        disklist = WinCreateWindow( hwndClient, WC_LVMDISKS, "", WS_VISIBLE | WS_CLIPCHILDREN | WS_GROUP,
                                    1, 1, 0, 0, hwndClient, HWND_TOP, ID_DISKLIST, NULL, NULL );
        WinSetPresParam( disklist, PP_FONTNAMESIZE, 11, (PVOID)"9.WarpSans");

        aDisks[0].cb = sizeof( DVCTLDATA );
        aDisks[0].ulSize = 512000;
        aDisks[0].number = 1;
        //aDisks[0].handle = LH_DISK1;
        sprintf( aDisks[0].szName, "MY TEST HARD DISK 1");
        aDisks[1].cb = sizeof( DVCTLDATA );
        aDisks[1].ulSize = 255002;
        aDisks[1].number = 2;
        //aDisks[1].handle = LH_DISK2;
        sprintf( aDisks[1].szName, "Disk 2");
        aDisks[2].cb = sizeof( DVCTLDATA );
        aDisks[2].ulSize = 300000;
        aDisks[2].number = 3;
        //aDisks[2].handle = LH_DISK3;
        sprintf( aDisks[2].szName, "Another Disk");
        WinSendMsg( disklist, LLM_SETDISKS, MPFROMLONG( 3 ), MPFROMP( aDisks ));

        panel1 = (HWND) WinSendMsg( disklist, LLM_QUERYDISKHWND, MPFROMLONG(0), MPFROMSHORT(TRUE) );
        panel2 = (HWND) WinSendMsg( disklist, LLM_QUERYDISKHWND, MPFROMLONG(1), MPFROMSHORT(TRUE) );
        panel3 = (HWND) WinSendMsg( disklist, LLM_QUERYDISKHWND, MPFROMLONG(2), MPFROMSHORT(TRUE) );

        hicon = WinLoadPointer( HWND_DESKTOP, NULLHANDLE, 10 );
        if ( hicon ) {
            WinSendMsg( panel1, LDM_SETDISKICON, MPFROMP(hicon), 0 );
            WinSendMsg( panel2, LDM_SETDISKICON, MPFROMP(hicon), 0 );
            WinSendMsg( panel3, LDM_SETDISKICON, MPFROMP(hicon), 0 );
        }

        aParts1[0].cb      = sizeof( PVCTLDATA );
        aParts1[0].bType   = LPV_TYPE_PRIMARY;
        aParts1[0].bOS     = 1;
        aParts1[0].ulSize  = 100000;
        aParts1[0].cLetter = 'C';
        aParts1[0].number  = 1;
        aParts1[0].handle  = (ADDRESS) LH_PARTC;
        sprintf( aParts1[0].szName, "Partition 1");
        aParts1[1].cb      = sizeof( PVCTLDATA );
        aParts1[1].bType   = LPV_TYPE_PRIMARY;
        aParts1[1].bOS     = 1;
        aParts1[1].ulSize  = 150000;
        aParts1[1].cLetter = 'D';
        aParts1[1].number  = 2;
        aParts1[1].handle  = (ADDRESS) LH_PARTD;
        sprintf( aParts1[1].szName, "Partition 2");
        aParts1[2].cb      = sizeof( PVCTLDATA );
        aParts1[2].bType   = LPV_TYPE_LOGICAL;
        aParts1[2].bOS     = 1;
        aParts1[2].ulSize  = 150000;
        aParts1[2].cLetter = 'E';
        aParts1[2].number  = 3;
        aParts1[2].handle  = (ADDRESS) LH_PARTE;
        sprintf( aParts1[2].szName, "Partition 3");
        aParts1[3].cb      = sizeof( PVCTLDATA );
        aParts1[3].bType   = LPV_TYPE_FREE;
        aParts1[3].bOS     = 1;
        aParts1[3].ulSize  = 12000;
        aParts1[3].cLetter = 0;
        aParts1[3].number  = 4;
        aParts1[3].handle  = (ADDRESS) 103;
        sprintf( aParts1[3].szName, "");
        aParts1[4].cb      = sizeof( PVCTLDATA );
        aParts1[4].bType   = LPV_TYPE_LOGICAL;
        aParts1[4].bOS     = 1;
        aParts1[4].ulSize  = 100000;
        aParts1[4].cLetter = 'G';
        aParts1[4].number  = 5;
        aParts1[4].handle  = (ADDRESS) LH_PARTG;
        sprintf( aParts1[4].szName, "Partition 4");
        WinSendMsg( panel1, LDM_SETPARTITIONS, MPFROMLONG( 5 ), MPFROMP( aParts1 ));

        aParts2[0].cb      = sizeof( PVCTLDATA );
        aParts2[0].bType   = LPV_TYPE_PRIMARY;
        aParts2[0].bOS     = 1;
        aParts2[0].ulSize  = 100000;
        aParts2[0].cLetter = 'F';
        aParts2[0].number  = 1;
        aParts2[0].handle  = (ADDRESS) LH_PARTF;
        sprintf( aParts2[0].szName, "Partition 5");
        aParts2[1].cb      = sizeof( PVCTLDATA );
        aParts2[1].bType   = LPV_TYPE_LOGICAL;
        aParts2[1].bOS     = 1;
        aParts2[1].ulSize  = 100000;
        aParts2[1].cLetter = 0;
        aParts2[1].number  = 2;
        aParts2[1].handle  = (ADDRESS) 201;
        sprintf( aParts2[1].szName, "Partition 6");
        aParts2[2].cb      = sizeof( PVCTLDATA );
        aParts2[2].bType   = LPV_TYPE_FREE;
        aParts2[2].bOS     = 1;
        aParts2[2].ulSize  = 55002;
        aParts2[2].cLetter = 0;
        aParts2[2].number  = 3;
        aParts2[2].handle  = (ADDRESS) 202;
        sprintf( aParts2[2].szName, "");
        WinSendMsg( panel2, LDM_SETPARTITIONS, MPFROMLONG( 3 ), MPFROMP( aParts2 ));

        aParts3[0].cb      = sizeof( PVCTLDATA );
        aParts3[0].bType   = LPV_TYPE_FREE;
        aParts3[0].bOS     = 1;
        aParts3[0].ulSize  = 300000;
        aParts3[0].cLetter = 0;
        aParts3[0].number  = 3;
        aParts3[0].handle  = (ADDRESS) 300;
        sprintf( aParts3[0].szName, "");
//        WinSendMsg( panel3, LDM_SETPARTITIONS, MPFROMLONG( 1 ), MPFROMP( aParts3 ));

        WinSendMsg( disklist, LLM_SETDISKEMPHASIS, MPFROMP( panel1 ), MPFROM2SHORT( TRUE, LDV_FS_SELECTED ));
        WinSendMsg( disklist, LLM_SETSTYLE, MPFROMSHORT( LLD_FS_TITLE ), 0 );
#endif

        WinSetWindowPos( hwndFrame, HWND_TOP, 100, 100, 600, 400, SWP_SHOW | SWP_MOVE | SWP_SIZE | SWP_ACTIVATE );
        WinSetFocus( HWND_DESKTOP, disklist );

        // Now run the main program message loop
        while ( WinGetMsg( hab, &qmsg, 0, 0, 0 )) WinDispatchMsg( hab, &qmsg );
    }

    // Clean up and exit
    WinDestroyWindow( hwndFrame );
    WinDestroyMsgQueue( hmq );
    WinTerminate( hab );

    return ( 0 );
}


/* ------------------------------------------------------------------------- *
 * Window procedure for the main client window.                              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY MainWndProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    POINTL      ptl;
    RECTL       rcl;
    HPS         hps;
    PDISKNOTIFY pNotify;
    USHORT      fsEmp;
    BOOL        fSet;
    HWND        hwndPart;
    static HWND hwndSource = NULLHANDLE;

    switch( msg ) {

        case WM_CREATE:
            hwndDiskPopup = WinLoadMenu( HWND_DESKTOP, 0, 100 );
            hwndPartPopup = WinLoadMenu( HWND_DESKTOP, 0, 200 );
            WinShowWindow( hwnd, TRUE );
            return (MRESULT) FALSE;

        case WM_CONTEXTMENU:
            if ( SHORT2FROMMP( mp2 )) {
                HWND hwndFocus, hwndDisk;
                hwndFocus = WinQueryFocus( HWND_DESKTOP );
                if ( hwndFocus == disklist ) {
                    hwndDisk = (HWND) WinSendMsg( disklist,
                                                  LLM_QUERYDISKEMPHASIS, 0,
                                                  MPFROMSHORT( LDV_FS_SELECTED ));
                    if ( hwndDisk && hwndPartPopup ) {
                        hwndPart = (HWND) WinSendMsg( hwndDisk,
                                                      LDM_QUERYPARTITIONEMPHASIS, 0,
                                                      MPFROMSHORT( LPV_FS_SELECTED ));
                        if ( hwndPart ) {
                            RECTL rcl;
                            LONG offx, offy;
                            WinSendMsg( hwndDisk, LDM_SETEMPHASIS,
                                        MPFROMP( hwndPart ),
                                        MPFROM2SHORT( TRUE, LPV_FS_CONTEXT ));
                            WinQueryWindowRect( hwndPart, &rcl );
                            offx = rcl.xRight / 2;
                            offy = rcl.yTop / 2;
                            WinMapWindowPoints( hwndPart, HWND_DESKTOP, (PPOINTL)&rcl, 2 );
                            WinPopupMenu( HWND_DESKTOP, hwnd, hwndPartPopup,
                                          rcl.xLeft + offx, rcl.yBottom + offy, 0,
                                          PU_HCONSTRAIN | PU_VCONSTRAIN |
                                          PU_KEYBOARD | PU_MOUSEBUTTON1 );
                            hwndSource = hwndDisk;
                        }
                    }
                }
            }
            break;

        case WM_COMMAND:
            switch ( (USHORT) mp1 ) {
                case ID_BTNC:
                    hwndPart = (HWND) WinSendMsg( disklist, LLM_GETPARTITION,
                                                  MPFROMLONG( LH_PARTC ),
                                                  MPFROMLONG( LH_DISK1 ));
                    fsEmp = (USHORT) WinSendMsg( hwndPart, LPM_GETEMPHASIS, 0, 0 );
                    fSet = ( fsEmp & LPV_FS_ACTIVE ) ? FALSE : TRUE;
                    WinSendMsg( hwndPart, LPM_SETEMPHASIS, MPFROMSHORT( fSet ),
                                MPFROMSHORT( LPV_FS_ACTIVE ));
                    break;

                case ID_BTND:
                    hwndPart = (HWND) WinSendMsg( disklist, LLM_GETPARTITION,
                                                  MPFROMLONG( LH_PARTD ),
                                                  MPFROMLONG( LH_DISK1 ));
                    fsEmp = (USHORT) WinSendMsg( hwndPart, LPM_GETEMPHASIS, 0, 0 );
                    fSet = ( fsEmp & LPV_FS_ACTIVE ) ? FALSE : TRUE;
                    WinSendMsg( hwndPart, LPM_SETEMPHASIS, MPFROMSHORT( fSet ),
                                MPFROMSHORT( LPV_FS_ACTIVE ));
                    break;

                case ID_BTNE:
                    hwndPart = (HWND) WinSendMsg( disklist, LLM_GETPARTITION,
                                                  MPFROMLONG( LH_PARTE ),
                                                  MPFROMLONG( LH_DISK1 ));
                    fsEmp = (USHORT) WinSendMsg( hwndPart, LPM_GETEMPHASIS, 0, 0 );
                    fSet = ( fsEmp & LPV_FS_ACTIVE ) ? FALSE : TRUE;
                    WinSendMsg( hwndPart, LPM_SETEMPHASIS, MPFROMSHORT( fSet ),
                                MPFROMSHORT( LPV_FS_ACTIVE ));
                    break;

                case ID_BTNF:
                    hwndPart = (HWND) WinSendMsg( disklist, LLM_GETPARTITION,
                                                  MPFROMLONG( LH_PARTF ),
                                                  MPFROMLONG( LH_DISK2 ));
                    fsEmp = (USHORT) WinSendMsg( hwndPart, LPM_GETEMPHASIS, 0, 0 );
                    fSet = ( fsEmp & LPV_FS_ACTIVE ) ? FALSE : TRUE;
                    WinSendMsg( hwndPart, LPM_SETEMPHASIS, MPFROMSHORT( fSet ),
                                MPFROMSHORT( LPV_FS_ACTIVE ));
                    break;

                case ID_BTNG:
                    hwndPart = (HWND) WinSendMsg( disklist, LLM_GETPARTITION,
                                                  MPFROMLONG( LH_PARTG ), 0 );
                    fsEmp = (USHORT) WinSendMsg( hwndPart, LPM_GETEMPHASIS, 0, 0 );
                    fSet = ( fsEmp & LPV_FS_ACTIVE ) ? FALSE : TRUE;
                    WinSendMsg( hwndPart, LPM_SETEMPHASIS, MPFROMSHORT( fSet ),
                                MPFROMSHORT( LPV_FS_ACTIVE ));
                    break;

            }
            return (MRESULT) 0;

        case WM_CONTROL:
            switch ( SHORT2FROMMP( mp1 )) {
                case LLN_CONTEXTMENU:
                    if ( SHORT1FROMMP( mp1 ) == ID_DISKLIST ) {
                        pNotify = (PDISKNOTIFY) mp2;
                        if ( pNotify->hwndPartition && hwndPartPopup )
                        {
                            WinSendMsg( pNotify->hwndDisk, LDM_SETEMPHASIS,
                                        MPFROMP( pNotify->hwndPartition ),
                                        MPFROM2SHORT( TRUE, LPV_FS_CONTEXT ));
                            WinQueryPointerPos( HWND_DESKTOP, &ptl );
                            WinPopupMenu( HWND_DESKTOP, hwnd, hwndPartPopup,
                                          ptl.x, ptl.y, 0,
                                          PU_HCONSTRAIN | PU_VCONSTRAIN |
                                          PU_KEYBOARD | PU_MOUSEBUTTON1 );
                            hwndSource = pNotify->hwndDisk;
                        }
                        else if ( pNotify->hwndDisk && hwndDiskPopup )
                        {
                            WinSendMsg( pNotify->hwndDisk, LDM_SETEMPHASIS, 0,
                                        MPFROM2SHORT( TRUE, LDV_FS_CONTEXT ));
                            WinQueryPointerPos( HWND_DESKTOP, &ptl );
                            WinPopupMenu( HWND_DESKTOP, hwnd, hwndDiskPopup,
                                          ptl.x, ptl.y, 0,
                                          PU_HCONSTRAIN | PU_VCONSTRAIN |
                                          PU_KEYBOARD | PU_MOUSEBUTTON1 );
                            hwndSource = pNotify->hwndDisk;
                        }
                    }
                    break;
            }
            break;

        case WM_MENUEND:
            if ( hwndSource ) {
                WinSendMsg( hwndSource, LDM_SETEMPHASIS, 0,
                            MPFROM2SHORT( FALSE, LDV_FS_CONTEXT | LPV_FS_CONTEXT ));
                hwndSource = NULLHANDLE;
            }
            break;

        case WM_PAINT:
            WinQueryWindowRect( hwnd, &rcl );
            if (( hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE )) != NULLHANDLE ) {

                GpiBeginPath( hps, 1L );
                ptl.x = rcl.xLeft;
                ptl.y = rcl.yBottom;
                GpiMove( hps, &ptl );
                ptl.x = rcl.xRight;
                ptl.y = rcl.yTop;
                GpiBox( hps, DRO_OUTLINE, &ptl, 0, 0 );
                ptl.x = rcl.xLeft + 1;
                ptl.y = rcl.yBottom + 1;
                GpiMove( hps, &ptl );
                ptl.x = rcl.xRight - 2;
                ptl.y = rcl.yTop - 50;
                GpiBox( hps, DRO_OUTLINE, &ptl, 0, 0 );
                GpiEndPath( hps );
                GpiSetColor( hps, SYSCLR_DIALOGBACKGROUND );
                GpiFillPath( hps, 1L, FPATH_ALTERNATE );

                //WinFillRect( hps, &rcl, SYSCLR_DIALOGBACKGROUND );
                WinEndPaint( hps );
            }
            break;

        case WM_SIZE:
#if 0
            if ( !panel1 ) break;
            WinQueryWindowRect( hwnd, &rcl );
            WinSetWindowPos( panel1, HWND_TOP, 10, 10, rcl.xRight - 20, rcl.yTop - 30, SWP_SIZE );
#else
            if ( !disklist ) break;
            WinQueryWindowRect( hwnd, &rcl );
            WinSetWindowPos( disklist, HWND_TOP, 1, 1, rcl.xRight - 2, rcl.yTop - 50, SWP_SIZE );

            WinSetWindowPos( btnC, HWND_TOP, 1,   rcl.yTop - 40, 60, 40, SWP_MOVE | SWP_SIZE | SWP_SHOW );
            WinSetWindowPos( btnD, HWND_TOP, 62,  rcl.yTop - 40, 60, 40, SWP_MOVE | SWP_SIZE | SWP_SHOW );
            WinSetWindowPos( btnE, HWND_TOP, 123, rcl.yTop - 40, 60, 40, SWP_MOVE | SWP_SIZE | SWP_SHOW );
            WinSetWindowPos( btnF, HWND_TOP, 184, rcl.yTop - 40, 60, 40, SWP_MOVE | SWP_SIZE | SWP_SHOW );
            WinSetWindowPos( btnG, HWND_TOP, 245, rcl.yTop - 40, 60, 40, SWP_MOVE | SWP_SIZE | SWP_SHOW );

#endif
            break;

        case WM_CLOSE:
            WinPostMsg( hwnd, WM_QUIT, 0, 0 );
            return (MRESULT) 0;

    } // end event handlers

    return ( WinDefWindowProc( hwnd, msg, mp1, mp2 ));

}

