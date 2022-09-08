/*****************************************************************************
 * lvmpm.c                                                                   *
 *                                                                           *
 * Copyright (C) 2011-2019 Alexander Taylor.                                 *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify it *
 *   under the terms of the GNU General Public License as published by the   *
 *   Free Software Foundation; either version 2 of the License, or (at your  *
 *   option) any later version.                                              *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful, but     *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of              *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 *   General Public License for more details.                                *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License along *
 *   with this program; if not, write to the Free Software Foundation, Inc., *
 *   59 Temple Place, Suite 330, Boston, MA  02111-1307  USA                 *
 *****************************************************************************/
#include "lvmpm.h"

PFNWP g_pfnRecProc;             // Default SS_FGNDFRAME window procedure
PFNWP g_pfnTextProc;            // Default SS_TEXT window procedure

/* ------------------------------------------------------------------------- *
 * main()                                                                    *
 * ------------------------------------------------------------------------- */
int main( int argc, char *argv[] )
{
    DVMGLOBAL global = {0};                        // program global data
    HMQ       hmq;                                 // main message queue
    HWND      hwndFrame,                           // main program frame
              hwndClient,                          // main program client
              hwndHelp;                            // help instance handle
    HACCEL    hAccel;                              // accel-table handle
    HELPINIT  help;                                // help init structure
    QMSG      qmsg;                                // message queue
    CHAR      szFile[ CCHMAXPATH+1 ]       = {0},  // NLV filename
              szRes[ STRING_RES_MAXZ ]     = {0},  // string resource buffer
              szError[ STRING_ERROR_MAXZ ] = {0};  // error buffer
    BOOL      bInitErr   = FALSE,                  // initialization failed
              bLVMOK     = FALSE,                  // LVM engine is open
              bQuit      = FALSE,                  // program exit confirmed
              bAppendLog = FALSE;                  // append to rather than replace log
    LONG      lXRes, lYRes,                        // current screen size
              lX, lY, lW, lH,                      // initial window coordinates
              lS;                                  // initial splitbar position
    ULONG     flStyle,                             // window style flags
              i;                                   // loop index for arguments


    global.ulReturn = RETURN_INITFAIL;

    /* Parse the command-line arguments, if any.
     */
    for ( i = 1; i < argc; i++ ) {
        if ( strnicmp( argv[ i ], "/log", 4 ) == 0 ) {
            global.fsProgram |= FS_APP_LOGGING;
        }
        else if ( strnicmp( argv[ i ], "/keep", 5 ) == 0 ) {
            bAppendLog = TRUE;
        }
    }

    /* Do the necessary Presentation Manager initialization for the application.
     * Note: Any error messages required prior to successful loading of the
     * resource library must be hard-coded (English).  Once the resources have
     * been loaded successfully, all text should be loaded as string resources.
     */
    global.hab = WinInitialize( 0 );
    if ( !global.hab ) {
        sprintf( szError, "WinInitialize() failed.");
        bInitErr = TRUE;
    }
    else if (( hmq = WinCreateMsgQueue( global.hab, 0 )) == NULLHANDLE ) {
        sprintf( szError, "Unable to create message queue: PM error code 0x%X\n",
                 ERRORIDERROR( WinGetLastError( global.hab )));
        bInitErr = TRUE;
    }

    /* Now try to initialize the application data.
     */
    if ( !bInitErr ) {
        // Get any saved program settings from OS2.INI
        Settings_Load( &global, &lX, &lY, &lW, &lH, &lS );

        // Find out if we're running on a DBCS system
        if ( CheckDBCS() ) global.fsProgram |= FS_APP_DBCS;

        // Get the PM_Locale settings
        nlsQueryCountrySettings( &(global.ctry) );

        // Try to load the resource library.  Look in %ULSPATH%\<lang> first...
        if ( ! GetLanguageFile( szFile, RESOURCE_LIB ) ||
            (( DosLoadModule( (PSZ) szError, sizeof( szError ),
                              szFile, &(global.hmri) )) != NO_ERROR ))
        {
            // Failing that, try loading it from the LIBPATH like any other DLL
            if (( DosLoadModule( (PSZ) szError, sizeof( szError ),
                  RESOURCE_LIB, &(global.hmri) )) != NO_ERROR )
            {
                sprintf( szError,
                         "Unable to locate resource library \"%s\":\r\nThe PM error code is 0x%X\n",
                         RESOURCE_LIB, ERRORIDERROR( WinGetLastError( global.hab )));
                bInitErr = TRUE;
            }
        }

        // Register our custom disk list control
        if ( !bInitErr ) {
            if ( ! DLRegisterClass( global.hab )) {
                if ( WinLoadString( global.hab, global.hmri, IDS_ERROR_REGISTER,
                                    STRING_RES_MAXZ, szRes ))
                    sprintf( szError, szRes, WC_LVMDISKS,
                             ERRORIDERROR( WinGetLastError( global.hab )));
                else
                    sprintf( szError,
                             "Failed to register class %.40s:\r\nThe PM error code is 0x%08X\n",
                             WC_LVMDISKS, ERRORIDERROR( WinGetLastError( global.hab )));
                bInitErr = TRUE;
            }
        }

        // Register our custom volume display
        if ( !bInitErr &&
            ( ! WinRegisterClass( global.hab, WC_LOGICALVIEW, VolumesPanelProc,
                                  CS_SIZEREDRAW, sizeof( PDVMLOGVIEWCTLDATA ))))
        {
            if ( WinLoadString( global.hab, global.hmri, IDS_ERROR_REGISTER,
                                STRING_RES_MAXZ, szRes ))
                sprintf( szError, szRes, WC_LOGICALVIEW,
                         ERRORIDERROR( WinGetLastError( global.hab )));
            else
                sprintf( szError,
                         "Failed to register class %.40s:\r\nThe PM error code is 0x%08X\n",
                         WC_LOGICALVIEW, ERRORIDERROR( WinGetLastError( global.hab )));
            bInitErr = TRUE;
        }

        // Register the XWPhelpers tooltip class
        if ( !bInitErr &&
             ( ! ctlRegisterTooltip( global.hab )))
        {
            if ( WinLoadString( global.hab, global.hmri, IDS_ERROR_INITDLG,
                                STRING_RES_MAXZ, szRes ))
                sprintf( szError, szRes,
                         COMCTL_TOOLTIP_CLASS, ERRORIDERROR( WinGetLastError( global.hab )));
            else
                sprintf( szError,
                         "Failed to register class %.40s:\r\nThe PM error code is 0x%08X\n",
                         COMCTL_TOOLTIP_CLASS, ERRORIDERROR( WinGetLastError( global.hab )));
            bInitErr = TRUE;
        }
    }

    /* If all went OK, the preparatory initialization is done.
     * Now we create the main program window.
     */
    if ( !bInitErr &&
        ( ! WinRegisterClass( global.hab, SZ_CLASSNAME, MainWndProc,
                              CS_SIZEREDRAW, sizeof( PDVMGLOBAL ))))
    {
        if ( WinLoadString( global.hab, global.hmri, IDS_ERROR_INITDLG,
                            STRING_RES_MAXZ, szRes ))
            sprintf( szError, szRes,
                     SZ_CLASSNAME, ERRORIDERROR( WinGetLastError( global.hab )));
        else
            sprintf( szError,
                     "Failed to register class %.40s:\r\nThe PM error code is 0x%08X\n",
                     SZ_CLASSNAME, ERRORIDERROR( WinGetLastError( global.hab )));
        bInitErr = TRUE;
    }

    if ( !bInitErr ) {
        WinLoadString( global.hab, global.hmri,
                       IDS_PROGRAM_TITLE, STRING_RES_MAXZ, szRes );
        flStyle = FCF_STANDARD | FCF_AUTOICON | FCF_NOBYTEALIGN;
        flStyle &= ~FCF_SHELLPOSITION;
        hwndFrame = WinCreateStdWindow( HWND_DESKTOP, 0, &flStyle, SZ_CLASSNAME,
                                        szRes, CS_SIZEREDRAW, global.hmri,
                                        ID_MAINPROGRAM, &hwndClient );
        // TODO create a string resource for this
        if ( hwndFrame == NULLHANDLE ) {
            sprintf( szError, "Failed to create application window:\r\nThe PM error code is 0x%08X\n",
                     ERRORIDERROR( WinGetLastError( global.hab )));
            bInitErr = TRUE;
        }
    }

    /* If any initialization error occurred up to this point, display the
     * predetermined error message and then exit.
     */
    if ( bInitErr )
    {
        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, szError,
                       SZ_ERROR_INIT, 0, MB_MOVEABLE | MB_OK | MB_ERROR );
    }
    else {
        // We're in business!
        global.ulReturn = RETURN_NORMAL;
        global.hwndMenu = WinWindowFromID( hwndFrame, FID_MENU );

        // Initialize keyboard shortcuts
        hAccel = WinLoadAccelTable( global.hab, global.hmri, ID_MAINPROGRAM );
        WinSetAccelTable( global.hab, hAccel, hwndFrame );

        // Initialize the online help
        WinLoadString( global.hab, global.hmri, IDS_HELP_TITLE, STRING_RES_MAXZ, szRes );
        help.cb                       = sizeof( HELPINIT );
        help.pszTutorialName          = NULL;
        help.phtHelpTable             = (PHELPTABLE) MAKELONG( ID_MAINPROGRAM, 0xFFFF );
        help.hmodHelpTableModule      = global.hmri;
        help.hmodAccelActionBarModule = NULLHANDLE;
        help.fShowPanelId             = CMIC_SHOW_PANEL_ID;
        help.idAccelTable             = 0;
        help.idActionBar              = 0;
        help.pszHelpWindowTitle       = szRes;
        help.pszHelpLibraryName       = ( GetLanguageFile( szFile, HELP_FILE ) &&
                                          FileExists( szFile )) ?
                                        szFile : HELP_FILE;
        hwndHelp = WinCreateHelpInstance( global.hab, &help );
        if ( ! WinAssociateHelpInstance( hwndHelp, hwndFrame )) {
            WinLoadString( global.hab, global.hmri, IDS_HELP_LOAD_ERROR, STRING_RES_MAXZ, szError );
            WinLoadString( global.hab, global.hmri, IDS_HELP_ERROR_TITLE, STRING_RES_MAXZ, szRes );
            WinMessageBox( HWND_DESKTOP, hwndFrame, szError, szRes,
                           0, MB_MOVEABLE | MB_OK | MB_ERROR );
        }

        // See if the Air-Boot installer is available
        AirBoot_GetInstaller( &global );

        // Store a pointer to our global data structure
        WinSetWindowPtr( hwndClient, 0, &global );

        // Set up the main window contents
        MainWindowInit( hwndClient, lS );

        // Initialize the log file if logging was requested
        if ( global.fsProgram & FS_APP_LOGGING ) {
            global.pLog = LogFileInit( bAppendLog );
        }

        // Open the LVM engine
        if ( LVM_Start( global.pLog, global.hab, global.hmri )) {
            // LVM started OK; now populate our LVM data structures
            bLVMOK = TRUE;
            if ( ! LVM_InitData( hwndClient, &global ))
                bQuit = TRUE;
        }
        else bQuit = TRUE;

        if ( !bQuit ) {
            // Populate our UI using the data obtained from LVM
            DiskListPopulate( hwndClient );
            VolumeContainerPopulate( WinWindowFromID( global.hwndVolumes, IDD_VOLUMES ),
                                     global.volumes, global.ulVolumes,
                                     global.hab, global.hmri,
                                     global.fsProgram, FALSE
                                   );

            // Set up the menu-bar
            SetBootMgrActions( &global );
            SetAvailableActions( hwndClient );

            // Position and display the program window
            if (( lW < 1 ) || ( lH < 1 )) {
                lXRes = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
                lYRes = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );
                lW = lXRes * 0.8;
                lH = lYRes * 0.7;
                WinSetWindowPos( hwndFrame, HWND_TOP, 0, 0, lW, lH, SWP_SIZE );
                CentreWindow( hwndFrame, NULLHANDLE, SWP_SHOW | SWP_ACTIVATE );
            }
            else {
                WinSetWindowPos( hwndFrame, HWND_TOP, lX, lY, lW, lH,
                                 SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ACTIVATE );
            }
            MainWindowFocus( hwndClient, TRUE );
        }

        // main program loop
        while ( !bQuit ) {
            // loop until quit signalled
            while ( WinGetMsg( global.hab, &qmsg, 0, 0, 0 ))
                WinDispatchMsg( global.hab, &qmsg );

            /* Send a message asking the user to confirm exiting if there were
             * changes made, unless ulReturn indicates we've already been told
             * to reboot or abort. (It's a bit obnoxious to do this on WM_QUIT
             * rather than WM_CLOSE, but partitioning is such a critical thing
             * that it's hopefully justified.)
             */
            if (( global.ulReturn == RETURN_NORMAL ) &&
                (ULONG) WinSendMsg( hwndFrame, WM_COMMAND,
                                    MPFROM2SHORT( ID_VERIFYQUIT, 0 ), 0 )
                         != MBID_CANCEL )
                bQuit = TRUE;

            // cancel quit if necessary
            if ( qmsg.hwnd == NULLHANDLE && !bQuit )
                WinCancelShutdown( hmq, FALSE );
        }
        if ( hwndHelp )
            WinDestroyHelpInstance( hwndHelp );
    }

    Settings_Save( hwndFrame, &global );
    WinDestroyWindow( hwndFrame );

    // Shut down the LVM engine
    if ( bLVMOK ) {
        LVM_FreeData( &global );
        LVM_Stop( &global );
    }
    // Close the logfile if there is one
    if ( global.pLog )
        fclose( global.pLog );

    // Reboot the system if necessary
    if ( global.ulReturn == RETURN_REBOOT )
        WinShutdownSystem( global.hab, hmq );

    WinDestroyMsgQueue( hmq );
    WinTerminate( global.hab );

    return ( global.ulReturn );
}


/* ------------------------------------------------------------------------- *
 * MainWndProc()                                                             *
 *                                                                           *
 * This is the event handler for the main application client window.         *
 * See OS/2 PM reference for a description of input and output.              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY MainWndProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    PDVMGLOBAL   pGlobal = NULL;            // pointer to global data
    PDISKNOTIFY  pNotify = NULL;            // disk notification message info
    DVCTLDATA    dvdata  = {0};             // disk control data structure
    PVCTLDATA    pvd     = {0};             // partition control data structure
    WNDPARAMS    wp      = {0};             // used to query window params
    BOOL         bSelected;                 // is a valid partition selected?
    HWND         hDisk,                     // disk control handle
                 hwndHelp;                  // program help instance
    CARDINAL32   ulError;                   // LVM error code
    ULONG        ulID,                      // matched pres-param ID
                 ulChoice,                  // user message-box selection
                 rc;                        // return code
    LONG         lClr;                      // background colour
    USHORT       fsMask;                    // used for various masks
    HPS          hps;                       // presentation space handle
    POINTL       ptl;                       // mouse pointer position
    RECTL        rcl;                       // drawing rectangle
    BOOLEAN      fSaved;                    // LVM changes committed OK
    CHAR         szRes1[ STRING_RES_MAXZ ], // string resource buffers
                 szRes2[ STRING_RES_MAXZ ];


    switch( msg ) {

        case WM_CREATE:
            return (MRESULT) FALSE;
            // WM_CREATE end


        case WM_CLOSE:
            WinPostMsg( hwnd, WM_QUIT, 0, 0 );
            return (MRESULT) 0;


        case WM_COMMAND:
            switch( SHORT1FROMMP( mp1 )) {

                case ID_FOCUS_FORWARD:
                    MainWindowFocus( hwnd, TRUE );
                    break;

                case ID_FOCUS_BACK:
                    MainWindowFocus( hwnd, FALSE );
                    break;


                case ID_AIRBOOT_INSTALL:
                    AirBoot_Install( hwnd );
                    break;


                case ID_AIRBOOT_REMOVE:
                    AirBoot_Delete( hwnd );
                    break;


                case ID_BM_INSTALL:
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    // Get the handle of the currently-selected disk
                    hDisk = (HWND) WinSendMsg( pGlobal->hwndDisks,
                                               LLM_QUERYDISKEMPHASIS, 0,
                                               MPFROMSHORT( LDV_FS_SELECTED ));
                    if ( !hDisk ) return (MRESULT) 0;
                    // Now query the disk information to find out its number
                    wp.fsStatus  = WPM_CTLDATA;
                    wp.cbCtlData = sizeof( DVCTLDATA );
                    wp.pCtlData  = &dvdata;
                    WinSendMsg( hDisk, WM_QUERYWINDOWPARAMS, MPFROMP( &wp ), 0 );
                    // Install Boot Manager to the selected disk
                    BootMgrInstall( hwnd, dvdata.number );
                    break;


                case ID_BM_REMOVE:
                    BootMgrRemove( hwnd );
                    break;


                case ID_BM_OPTIONS:
                    BootMgrOptions( hwnd );
                    break;


                case ID_LVM_NEWMBR:             // Rewrite MBR
                    DiskRewriteMBR( hwnd );
                    break;


                case ID_LVM_DISK:
                    if ( DiskRename( hwnd ))
                        LVM_Refresh( hwnd );
                    break;


                case ID_LVM_REFRESH:            // Refresh
                    LVM_Refresh( hwnd );
                    break;


                case ID_LVM_SAVE:               // Save changes
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    if ( pGlobal->fsEngine & FS_ENGINE_PENDING ) {
                        WinLoadString( pGlobal->hab, pGlobal->hmri,
                                       IDS_SAVE_CONFIRM, STRING_RES_MAXZ, szRes1 );
                        WinLoadString( pGlobal->hab, pGlobal->hmri,
                                       IDS_SAVE_TITLE, STRING_RES_MAXZ, szRes2 );
                        ulChoice = WinMessageBox( HWND_DESKTOP, hwnd, szRes1, szRes2, 0,
                                                  MB_YESNO | MB_QUERY | MB_MOVEABLE );
                        if ( ulChoice == MBID_YES ) {
                            fSaved = LvmCommit( &ulError );
                            if ( fSaved ) {
                                if ( pGlobal->pLog ) {
                                    fprintf( pGlobal->pLog, "-------------------------------------------------------------------------------\n");
                                    fprintf( pGlobal->pLog, "LVM CHANGES COMMITTED: %u\n", ulError );
                                }
                                SetModified( hwnd, FALSE );
                                // Changes saved; now see if a reboot is needed
                                if ( LvmRebootRequired() ) {
                                    WinLoadString( pGlobal->hab, pGlobal->hmri,
                                                   IDS_REBOOT_NOTIFY, STRING_RES_MAXZ, szRes1 );
                                    WinLoadString( pGlobal->hab, pGlobal->hmri,
                                                   IDS_REBOOT_TITLE, STRING_RES_MAXZ, szRes2 );
                                    WinMessageBox( HWND_DESKTOP, hwnd, szRes1, szRes2,
                                                   0, MB_OK | MB_WARNING | MB_MOVEABLE  );
                                    // Set the reboot flag and close the window
                                    pGlobal->ulReturn = RETURN_REBOOT;
                                    WinPostMsg( hwnd, WM_QUIT, 0, 0 );
                                }
                                else LVM_Refresh( hwnd );
                            }
                            else {
                                // Error during save!  Hope this doesn't happen.
                                PopupEngineError( NULL, ulError, hwnd,
                                                  pGlobal->hab, pGlobal->hmri );
                                pGlobal->ulReturn = ulError;
                            }
                        }
                    }
                    break;


                case ID_LVM_EXIT:               // Quit the program
                    WinPostMsg( hwnd, WM_QUIT, 0, 0 );
                    break;


                case ID_VOLUME_CREATE:          // Create a volume
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    if ( VolumeCreate( hwnd, pGlobal ))
                        LVM_Refresh( hwnd );
                    break;


                case ID_VOLUME_DELETE:          // Delete a volume
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    if ( VolumeDelete( hwnd, pGlobal ))
                        LVM_Refresh( hwnd );
                    break;


                case ID_VOLUME_RENAME:
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    if ( VolumeRename( hwnd, pGlobal ))
                        LVM_Refresh( hwnd );
                    break;


                case ID_VOLUME_LETTER:
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    if ( VolumeSetLetter( hwnd, pGlobal ))
                        LVM_Refresh( hwnd );
                    break;


                case ID_VOLUME_BOOTABLE:
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    if ( VolumeMakeBootable( hwnd, pGlobal ))
                        LVM_Refresh( hwnd );
                    break;


                case ID_VOLUME_STARTABLE:
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    if ( VolumeMakeStartable( hwnd, pGlobal ))
                        LVM_Refresh( hwnd );
                    break;


                case ID_PARTITION_CREATE:       // Create a partition
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    // Make sure free space is selected
                    bSelected = FALSE;
                    if ( GetSelectedPartition( pGlobal->hwndDisks, &pvd ) &&
                        ( pvd.bType == LPV_TYPE_FREE ))
                    {
                        bSelected = TRUE;
                    }
                    if ( bSelected ) {
                        if ( PartitionCreate( hwnd, pGlobal, &(pvd.handle), 0 ))
                            LVM_Refresh( hwnd );
                    }
                    else {
                        WinLoadString( pGlobal->hab, pGlobal->hmri,
                                       IDS_PARTITION_NOT_FREESPACE, STRING_RES_MAXZ, szRes1 );
                        WinLoadString( pGlobal->hab, pGlobal->hmri,
                                       IDS_ERROR_SELECTION, STRING_RES_MAXZ, szRes2 );
                        WinMessageBox( HWND_DESKTOP, hwnd, szRes1, szRes2,
                                       0, MB_MOVEABLE | MB_OK | MB_ERROR );
                    }
                    break;


                case ID_PARTITION_RENAME:
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    if ( PartitionRename( hwnd, pGlobal ))
                        LVM_Refresh( hwnd );
                    break;


                case ID_PARTITION_DELETE:
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    if ( PartitionDelete( hwnd, pGlobal ))
                        LVM_Refresh( hwnd );
                    break;


                case ID_PARTITION_CONVERT:
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    if ( PartitionConvertToVolume( hwnd, pGlobal ))
                        LVM_Refresh( hwnd );
                    break;


                case ID_PARTITION_ADD:
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    if ( PartitionAddToVolume( hwnd, pGlobal ))
                        LVM_Refresh( hwnd );
                    break;


                case ID_PARTITION_BOOTABLE:
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    if ( PartitionMakeBootable( hwnd, pGlobal ))
                        LVM_Refresh( hwnd );
                    break;


                case ID_PARTITION_ACTIVE:
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    if ( PartitionMakeActive( hwnd, pGlobal ))
                        LVM_Refresh( hwnd );
                    break;


                case ID_PREFS:                  // Application prefs dialog
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    fsMask = pGlobal->fsProgram;
                    WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) PrefsDlgProc,
                               pGlobal->hmri, IDD_PREFERENCES, pGlobal );
                    fsMask ^= pGlobal->fsProgram;

                    if ( fsMask & FS_APP_PMSTYLE ) {
                        // Visual style preference changed

                        // Change the volume-info panel divider style
                        WinSendMsg( pGlobal->hwndVolumes, LPM_STYLECHANGED,
                                    MPFROMSHORT( fsMask ), 0 );

                        // Change the colour scheme
                        if ( pGlobal->fsProgram & FS_APP_PMSTYLE ) {
                            lClr = SYSCLR_DIALOGBACKGROUND;
                            WinSetPresParam( hwnd, PP_BACKGROUNDCOLORINDEX,
                                             sizeof( lClr ), &lClr );
                            WinSetPresParam( pGlobal->hwndVolumes,
                                             PP_BACKGROUNDCOLORINDEX,
                                             sizeof( lClr ), &lClr );
                            lClr = SYSCLR_WINDOWTEXT;
                            WinSetPresParam( hwnd, PP_FOREGROUNDCOLORINDEX,
                                             sizeof( lClr ), &lClr );
                            WinSetPresParam( pGlobal->hwndVolumes,
                                             PP_FOREGROUNDCOLORINDEX,
                                             sizeof( lClr ), &lClr );
                        }
                        else {
                            lClr = 0xF0F0F0;
                            WinSetPresParam( hwnd, PP_BACKGROUNDCOLOR,
                                             sizeof( lClr ), &lClr );
                            WinSetPresParam( pGlobal->hwndVolumes,
                                             PP_BACKGROUNDCOLOR,
                                             sizeof( lClr ), &lClr );
                            lClr = 0x000000;
                            WinSetPresParam( hwnd, PP_FOREGROUNDCOLOR,
                                             sizeof( lClr ), &lClr );
                            WinSetPresParam( pGlobal->hwndVolumes,
                                             PP_FOREGROUNDCOLOR,
                                             sizeof( lClr ), &lClr );
                        }
                    }
                    if ( fsMask & FS_APP_IBMTERMS ) {
                        // Volume type terminology preference changed
                        ChangeVolumeTypeDisplay( hwnd, pGlobal );
                    }
                    if ( fsMask & FS_APP_IECSIZES ) {
                        // M(i)B terminology preference changed
                        ChangeSizeDisplay( hwnd, pGlobal );
                    }

                    if ( fsMask & FS_APP_UNIFORM ) {
                        USHORT usCount, i;
                        HWND hwndDisk;
                        usCount = (USHORT) WinSendMsg( pGlobal->hwndDisks, LLM_QUERYDISKS, 0, 0 );
                        for ( i = 0; i < usCount; i++ ) {
                            hwndDisk = (HWND) WinSendMsg( pGlobal->hwndDisks, LLM_QUERYDISKHWND,
                                                          MPFROMSHORT( i ), MPFROMSHORT( TRUE ));
                            if ( hwndDisk )
                                WinSendMsg( hwndDisk, LDM_SETSTYLE,
                                            MPFROMSHORT( (pGlobal->fsProgram & FS_APP_UNIFORM)? LDS_FS_UNIFORM: 0 ),
                                            0 );
                        }
                    }

                    if ( fsMask & FS_APP_HIDE_FREEPRM ) {
                        // Disk filter changed; empty and repopulate the list
                        DiskListClear( pGlobal );
                        DiskListPopulate( hwnd );
                    }

                    if ( fsMask & FS_APP_HIDE_NONLVM ) {
                        // Volume filter changed; empty and repopulate the container
                        VolumeContainerClear( pGlobal );
                        VolumeContainerPopulate( WinWindowFromID( pGlobal->hwndVolumes, IDD_VOLUMES ),
                                                 pGlobal->volumes, pGlobal->ulVolumes,
                                                 pGlobal->hab, pGlobal->hmri,
                                                 pGlobal->fsProgram, FALSE
                                               );
                    }

                    if (( fsMask & FS_APP_ENABLE_AB ) ||
                        ( fsMask & FS_APP_ENABLE_BM ))
                    {
                        if ( !pGlobal->szABEXE[0] ) {
                            pGlobal->fsProgram &= ~FS_APP_ENABLE_AB;
                            // should we show a warning here?
                        }
                        SetBootMgrActions( pGlobal );
                        SetAvailableActions( hwnd );
                    }

                    break;


                case ID_FONTS:                  // Font prefs dialog
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    if ( WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) FontsDlgProc,
                               pGlobal->hmri, IDD_FONTS, pGlobal ) == DID_OK )
                    {
                        MainWindowSetFonts( hwnd, pGlobal );
                    }
                    break;


                case ID_HELP_GENERAL:           // General/default help (F1)
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    if (( hwndHelp = WinQueryHelpInstance( hwnd )) != NULLHANDLE )
                        WinSendMsg( hwndHelp, HM_EXT_HELP, 0, 0 );
                    break;


                case ID_HELP_TERMS:             // Terminology help
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    if (( hwndHelp = WinQueryHelpInstance( hwnd )) != NULLHANDLE )
                        WinSendMsg( hwndHelp, HM_DISPLAY_HELP,
                                    MPFROMSHORT( 2 ), MPFROMSHORT( HM_RESOURCEID ));
                    break;


                case ID_HELP_ABOUT:             // Product information
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) ProdInfoDlgProc,
                               pGlobal->hmri, IDD_ABOUT, pGlobal );
                    break;


                case ID_VERIFYQUIT:             // Verify quit (called by WM_QUIT)
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    ulChoice = MBID_NO;

                    if (( pGlobal->fsProgram & FS_APP_BOOTWARNING ) &&
                        ( ! CheckBootable( pGlobal )))
                    {
                        // Warn if there are no bootable/startable OS/2 volumes
                        WinLoadString( pGlobal->hab, pGlobal->hmri,
                                       IDS_NOBOOT_WARNING, STRING_RES_MAXZ, szRes1 );
                        WinLoadString( pGlobal->hab, pGlobal->hmri,
                                       IDS_NOBOOT_TITLE, STRING_RES_MAXZ, szRes2 );
                        ulChoice = WinMessageBox( HWND_DESKTOP, hwnd, szRes1, szRes2,
                                                  0,    // TODO should create a help ID and panel for this
                                                  MB_YESNO | MB_WARNING | MB_MOVEABLE );
                        if ( ulChoice != MBID_YES )
                            return (MRESULT) MBID_CANCEL;
                    }

                    if ( pGlobal->fsEngine & FS_ENGINE_PENDING ) {
                        WinLoadString( pGlobal->hab, pGlobal->hmri,
                                       IDS_SAVE_QUIT, STRING_RES_MAXZ, szRes1 );
                        WinLoadString( pGlobal->hab, pGlobal->hmri,
                                       IDS_SAVE_TITLE, STRING_RES_MAXZ, szRes2 );
                        ulChoice = WinMessageBox( HWND_DESKTOP, hwnd,
                                                  szRes1, szRes2, 0,
                                                  MB_YESNOCANCEL |
                                                   MB_QUERY | MB_MOVEABLE );
                        if ( ulChoice == MBID_YES ) {
                            fSaved = LvmCommit( &ulError );
                            if ( fSaved ) {
                                if ( pGlobal->pLog ) {
                                    fprintf( pGlobal->pLog, "-------------------------------------------------------------------------------\n");
                                    fprintf( pGlobal->pLog, "LVM CHANGES COMMITTED: %u\n", ulError );
                                }
                                // Changes saved; now see if a reboot is needed
                                if ( LvmRebootRequired() ) {
                                    WinLoadString( pGlobal->hab, pGlobal->hmri,
                                                   IDS_REBOOT_NOTIFY, STRING_RES_MAXZ, szRes1 );
                                    WinLoadString( pGlobal->hab, pGlobal->hmri,
                                                   IDS_REBOOT_TITLE, STRING_RES_MAXZ, szRes2 );
                                    WinMessageBox( HWND_DESKTOP, hwnd, szRes1, szRes2,
                                                   0, MB_OK | MB_WARNING | MB_MOVEABLE  );
                                    pGlobal->ulReturn = RETURN_REBOOT;
                                }
                            }
                            else {
                                // Error during save!  Hope this doesn't happen.
                                PopupEngineError( NULL, ulError, hwnd,
                                                  pGlobal->hab, pGlobal->hmri );
                                pGlobal->ulReturn = ulError;
                            }
                        }
                    }
                    return (MRESULT) ulChoice;
                    // ID_VERIFYQUIT end

                default: break;

            }
            return (MRESULT) 0;
            // WM_COMMAND end


        case WM_CONTROL:
            switch( SHORT2FROMMP( mp1 )) {

                case CN_HELP:
                    // Workaround for WM_HELP getting eaten by containers
                    WinSendMsg( hwnd, WM_HELP,
                                MPFROMSHORT( ID_HELP_GENERAL ),
                                MPFROM2SHORT( CMDSRC_OTHER, FALSE ));
                    return (MRESULT) 0;


                /* There's only one disk list in the window, so we don't have to
                 * check the control ID for LLN_* notifications...
                 */

                case LLN_EMPHASIS:
                    pNotify = (PDISKNOTIFY) mp2;
                    if ( pNotify->hwndDisk ) {
                        fsMask = (USHORT) WinSendMsg( pNotify->hwndDisk,
                                                      LDM_GETEMPHASIS, 0, 0 );
                        DiskListSelect( hwnd, pNotify->usDisk,
                                        (BOOL)(fsMask & LDV_FS_SELECTED) );
                        if ( pNotify->hwndPartition ) {
                            fsMask = (USHORT) WinSendMsg( pNotify->hwndPartition,
                                                          LPM_GETEMPHASIS, 0, 0 );
                            if ( fsMask & LPV_FS_SELECTED ) {
                                DiskListPartitionSelect( hwnd, pNotify->hwndPartition );
                            }
                        }
                        else Status_Clear( hwnd );
                    }
                    return (MRESULT) 0;

                case LLN_KILLFOCUS:         // Disk list lost focus
                    Status_Clear( hwnd );   // Clear the status bar
                    return (MRESULT) 0;

                case LLN_SETFOCUS:          // Disk list gained focus
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );

                    // Don't continue if we're about to refresh the LVM data
                    if ( pGlobal->fsEngine & FS_ENGINE_REFRESH )
                        return (MRESULT) 0;

                    // Get the handle of the currently-selected disk
                    hDisk = (HWND) WinSendMsg( pGlobal->hwndDisks,
                                               LLM_QUERYDISKEMPHASIS, 0,
                                               MPFROMSHORT( LDV_FS_SELECTED ));
                    // Display the selected disk/partition in the status bar
                    if ( hDisk )
                        Status_ShowDisk( hwnd, hDisk );
                    return (MRESULT) 0;

                case LLN_CONTEXTMENU:
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    pNotify = (PDISKNOTIFY) mp2;
                    if ( pNotify && pNotify->hwndDisk ) {
                        if ( pNotify->hwndPartition && pGlobal->hwndPopupPartition ) {
                            WinSendMsg( pNotify->hwndDisk, LDM_SETEMPHASIS,
                                        MPFROMP( pNotify->hwndPartition ),
                                        MPFROM2SHORT( TRUE, LPV_FS_SELECTED | LPV_FS_CONTEXT | LDV_FS_SELECTED ));
                            WinQueryPointerPos( HWND_DESKTOP, &ptl );
                            WinPopupMenu( HWND_DESKTOP, hwnd, pGlobal->hwndPopupPartition,
                                          ptl.x, ptl.y, ID_PARTITION_CREATE,
                                          PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1 );
                        }
                        else {
                            WinSendMsg( pGlobal->hwndDisks, LLM_SETDISKEMPHASIS,
                                        MPFROMP( pNotify->hwndDisk ),
                                        MPFROM2SHORT( TRUE, LDV_FS_SELECTED | LDV_FS_FOCUS ));
                            // TODO popup disk menu?
                        }
                    }
                    return (MRESULT) 0;

                default: break;
            }
            break;
            // WM_CONTROL end


        case WM_APPTERMINATENOTIFY:
            pGlobal = WinQueryWindowPtr( hwnd, 0 );
            if ( (HAPP) mp1 == pGlobal->happAB ) {
                // The AiR-BOOT installer has terminated
                AirBoot_InstallerExit( hwnd, (ULONG) mp2, pGlobal );
            }
            return (MRESULT) 0;


        case WM_DESTROY:
            MainWindowCleanup( hwnd );
            break;


        case WM_HELP:
            break;


        case WM_MENUEND:
            // Clear any context menu emphasis on disk
            pGlobal = WinQueryWindowPtr( hwnd, 0 );
            hDisk = (HWND) WinSendMsg( pGlobal->hwndDisks,
                                       LLM_QUERYDISKEMPHASIS, 0,
                                       MPFROMSHORT( LDV_FS_SELECTED ));
            if ( hDisk )
                WinSendMsg( hDisk, LDM_SETEMPHASIS, MPVOID,
                            MPFROM2SHORT( FALSE, LPV_FS_CONTEXT | LDV_FS_CONTEXT ));
            break;


        case WM_PAINT:
            pGlobal = WinQueryWindowPtr( hwnd, 0 );
            if ( !pGlobal ) break;

            hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );

            // Get the current colour pres-params
            GpiCreateLogColorTable( hps, 0, LCOLF_RGB, 0, 0, NULL );
            rc = WinQueryPresParam( hwnd, PP_BACKGROUNDCOLORINDEX,
                                    PP_BACKGROUNDCOLOR, &ulID, sizeof(lClr),
                                    &lClr, QPF_ID1COLORINDEX | QPF_NOINHERIT );
            if ( !rc ) lClr = SYSCLR_WINDOW;
            else if ( ulID == PP_BACKGROUNDCOLORINDEX )
                lClr = GpiQueryRGBColor( hps, 0, lClr );

            // paint the statusbar background
            rcl.xLeft   = 0;
            rcl.yBottom = 0;
            rcl.xRight  = pGlobal->lStatusWidth;
            rcl.yTop    = pGlobal->lStatusHeight + 3;
            WinFillRect( hps, &rcl, lClr );
            // draw the status bar borders
            rcl.xRight  = pGlobal->lStatusDiv - 1;
            rcl.yTop    = pGlobal->lStatusHeight;
            if ( pGlobal->fsProgram & FS_APP_PMSTYLE )
                DrawInsetBorder( hps, rcl );
            else
                DrawOutlineBorder( hps, rcl );
            rcl.xLeft   = pGlobal->lStatusDiv + 1;
            rcl.xRight  = pGlobal->lStatusWidth;
            if ( pGlobal->fsProgram & FS_APP_PMSTYLE )
                DrawInsetBorder( hps, rcl );
            else
                DrawOutlineBorder( hps, rcl );
            WinEndPaint( hps );
            break;


        case WM_PRESPARAMCHANGED:
            switch ( (ULONG) mp1 ) {
                case PP_BACKGROUNDCOLOR:
                case PP_BACKGROUNDCOLORINDEX:
                case PP_FOREGROUNDCOLOR:
                case PP_FOREGROUNDCOLORINDEX:
#if 0
                    // Do we actually want/need to do this...?
                    // Update the status fields with the new colour
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    if ( pGlobal ) {
                        rc = WinQueryPresParam( hwnd, (ULONG) mp1, 0, NULL,
                                                sizeof( lClr ), &lClr, 0 );
                        if ( rc ) {
                            WinSetPresParam( WinWindowFromID( hwnd,
                                                IDD_STATUS_SELECTED ),
                                             (ULONG) mp1,
                                             sizeof( lClr ), &lClr );
                            WinSetPresParam( WinWindowFromID( hwnd,
                                                IDD_STATUS_SIZE ),
                                             (ULONG) mp1,
                                             sizeof( lClr ), &lClr );
                            WinSetPresParam( WinWindowFromID( hwnd,
                                                IDD_STATUS_TYPE ),
                                             (ULONG) mp1,
                                             sizeof( lClr ), &lClr );
                            WinSetPresParam( WinWindowFromID( hwnd,
                                                IDD_STATUS_FLAGS ),
                                             (ULONG) mp1,
                                             sizeof( lClr ), &lClr );
                            WinSetPresParam( WinWindowFromID( hwnd,
                                                IDD_STATUS_MODIFIED ),
                                             (ULONG) mp1,
                                             sizeof( lClr ), &lClr );

                        }
                    }
#endif
                    WinInvalidateRect( hwnd, NULL, TRUE );
                    break;

                default: break;
            }
            break;


        case WM_SIZE:
            pGlobal = WinQueryWindowPtr( hwnd, 0 );
            if ( !pGlobal ) break;
            MainWindowSize( hwnd,
                            SHORT1FROMMP( mp2 ), SHORT2FROMMP( mp2 ), pGlobal );
            break;


        default: break;

    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * MainWindowCleanup()                                                       *
 *                                                                           *
 * Frees up various resources used by the main window on program exit.       *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd: handle of the program client window                          *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void MainWindowCleanup( HWND hwnd )
{
    PDVMGLOBAL pGlobal;
    HWND       hwndPartCnr;

    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( !pGlobal ) return;

    VolumeContainerClear( pGlobal );
    VolumeContainerDestroy( pGlobal );
    hwndPartCnr = (HWND) WinSendDlgItemMsg( pGlobal->hwndVolumes, IDD_VOL_INFO,
                                            VIM_GETCONTAINERHWND, 0, 0 );
    PartitionContainerClear( hwndPartCnr, pGlobal );
    PartitionContainerDestroy( pGlobal );
    DiskListClear( pGlobal );

    if ( pGlobal->hwndPopupDisk )
        WinDestroyWindow( pGlobal->hwndPopupDisk );
    if ( pGlobal->hwndPopupPartition )
        WinDestroyWindow( pGlobal->hwndPopupPartition );
    if ( pGlobal->hwndPopupVolume )
        WinDestroyWindow( pGlobal->hwndPopupVolume );
    if ( pGlobal->hwndSplit )
        WinDestroyWindow( pGlobal->hwndSplit );
    if ( pGlobal->hwndVolumes )
        WinDestroyWindow( pGlobal->hwndVolumes );
    if ( pGlobal->hwndDisks )
        WinDestroyWindow( pGlobal->hwndDisks );

    WinSetWindowPtr( hwnd, 0, NULL );
}


/* ------------------------------------------------------------------------- *
 * MainWindowInit()                                                          *
 *                                                                           *
 * Creates and configures the main window contents.                          *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND   hwnd: handle of the program client window                        *
 *   LONG   lSB : split-bar position                                         *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void MainWindowInit( HWND hwnd, LONG lSB )
{
    PDVMGLOBAL    pGlobal;
    HMODULE       hIconLib;
    SPLITBARCDATA sbdata = {0};
    CHAR          szRes[ STRING_RES_MAXZ ];
    LONG          lClr;

    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( !pGlobal ) return;

    // Set the client's default colours (mainly just used by the status areas)
    if ( pGlobal->fsProgram & FS_APP_PMSTYLE ) {
        lClr = SYSCLR_DIALOGBACKGROUND;
        WinSetPresParam( hwnd, PP_BACKGROUNDCOLORINDEX, sizeof( lClr ), &lClr );
        lClr = SYSCLR_WINDOWTEXT;
        WinSetPresParam( hwnd, PP_FOREGROUNDCOLORINDEX, sizeof( lClr ), &lClr );
    }
    else {
        lClr = 0xF0F0F0;
        WinSetPresParam( hwnd, PP_BACKGROUNDCOLOR, sizeof( lClr ), &lClr );
        lClr = 0x000000;
        WinSetPresParam( hwnd, PP_FOREGROUNDCOLOR, sizeof( lClr ), &lClr );
    }

    // Create the top and bottom panels
    pGlobal->hwndVolumes = WinCreateWindow( hwnd, WC_LOGICALVIEW, "",
                                            WS_VISIBLE | WS_CLIPCHILDREN | CS_SIZEREDRAW,
                                            0, 0, 0, 0, hwnd, HWND_TOP,
                                            IDD_LOGICAL, pGlobal, NULL );

    // Set the default colours of the volume panel
    if ( pGlobal->fsProgram & FS_APP_PMSTYLE ) {
        lClr = SYSCLR_DIALOGBACKGROUND;
        WinSetPresParam( pGlobal->hwndVolumes, PP_BACKGROUNDCOLORINDEX,
                         sizeof( lClr ), &lClr );
        lClr = SYSCLR_WINDOWTEXT;
        WinSetPresParam( pGlobal->hwndVolumes, PP_FOREGROUNDCOLORINDEX,
                         sizeof( lClr ), &lClr );
    }
    else {
        lClr = 0xF0F0F0;
        WinSetPresParam( pGlobal->hwndVolumes, PP_BACKGROUNDCOLOR,
                         sizeof( lClr ), &lClr );
        lClr = 0x000000;
        WinSetPresParam( pGlobal->hwndVolumes, PP_FOREGROUNDCOLOR,
                         sizeof( lClr ), &lClr );
    }

    // We load these icons and store them globally because we may need to change
    // between them quite often.
    pGlobal->hptrStandard = WinLoadPointer( HWND_DESKTOP,
                                            pGlobal->hmri, IDP_VOL_BASIC );
    pGlobal->hptrAdvanced = WinLoadPointer( HWND_DESKTOP,
                                            pGlobal->hmri, IDP_VOL_ADVANCED );
    pGlobal->hptrUnknown  = WinLoadPointer( HWND_DESKTOP,
                                            pGlobal->hmri, IDP_VOL_UNKNOWN  );
    if ( DosQueryModuleHandle("PMWP.DLL", &hIconLib ) == NO_ERROR ) {
        pGlobal->hptrCD  = WinLoadPointer( HWND_DESKTOP, hIconLib, 19 );
        pGlobal->hptrLAN = WinLoadPointer( HWND_DESKTOP, hIconLib, 16 );
    }
    else {
        pGlobal->hptrCD  = WinLoadPointer( HWND_DESKTOP, pGlobal->hmri, IDP_VOL_CDROM );
        pGlobal->hptrLAN = WinLoadPointer( HWND_DESKTOP, pGlobal->hmri, IDP_VOL_NETWORK );
    }

    pGlobal->hwndDisks = WinCreateWindow( hwnd, WC_LVMDISKS, "", WS_VISIBLE |
                                          WS_CLIPCHILDREN | CS_SIZEREDRAW,
                                          0, 0, 0, 0, hwnd, HWND_TOP,
                                          IDD_DISKLIST, NULL, NULL );
    WinSendMsg( pGlobal->hwndDisks, LLM_SETSTYLE, MPFROMSHORT( LLD_FS_TITLE ), 0 );
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_LABELS_DISKS, STRING_RES_MAXZ, szRes );
    WinSendMsg( pGlobal->hwndDisks, LLM_SETTITLE, MPFROMP( szRes ), 0 );

    // Create the splitbar window
    sbdata.ulSplitWindowID     = IDD_SPLIT_WINDOW;
    sbdata.ulCreateFlags       = SBCF_HORIZONTAL | SBCF_PERCENTAGE |
                                 SBCF_3DSUNK | SBCF_MOVEABLE;
    sbdata.lPos                = lSB? lSB: 40;
    sbdata.ulLeftOrBottomLimit = 0;
    sbdata.ulRightOrTopLimit   = 0;
    sbdata.hwndParentAndOwner  = hwnd;
    pGlobal->hwndSplit = ctlCreateSplitWindow( pGlobal->hab, &sbdata );
    WinSendMsg( pGlobal->hwndSplit, SPLM_SETLINKS,
                MPFROMP( pGlobal->hwndDisks ), MPFROMP( pGlobal->hwndVolumes ));

    // Create the context menus
    //pGlobal->hwndPopupDisk = WinLoadMenu( HWND_DESKTOP, pGlobal->hmri, IDM_CONTEXT_DISK );
    pGlobal->hwndPopupPartition = WinLoadMenu( HWND_DESKTOP, pGlobal->hmri, IDM_CONTEXT_PARTITION );
    pGlobal->hwndPopupVolume = WinLoadMenu( HWND_DESKTOP, pGlobal->hmri, IDM_CONTEXT_VOLUME );

    // Create the tooltip control used by the disk list
    pGlobal->hwndTT = WinCreateWindow( HWND_DESKTOP, COMCTL_TOOLTIP_CLASS, NULL,
                                       0, 0, 0, 0, 0, hwnd, HWND_TOP,
                                       IDD_TOOLTIP, NULL, NULL );
    if ( !pGlobal->hwndTT ) {
        sprintf( szRes, "Failed to create tooltip control:\r\nThe PM error code is 0x%08X\n",
                 ERRORIDERROR( WinGetLastError( pGlobal->hab )));
        DebugBox(szRes);
    }

    // Create the status bar text fields
    WinCreateWindow( hwnd, WC_STATIC, "", SS_TEXT | DT_VCENTER | WS_VISIBLE,
                     0, 0, 0, 0, hwnd, HWND_TOP, IDD_STATUS_SELECTED, NULL, NULL );
    WinCreateWindow( hwnd, WC_STATIC, "", SS_TEXT | DT_VCENTER | WS_VISIBLE,
                     0, 0, 0, 0, hwnd, HWND_TOP, IDD_STATUS_SIZE, NULL, NULL );
    WinCreateWindow( hwnd, WC_STATIC, "", SS_TEXT | DT_VCENTER | WS_VISIBLE,
                     0, 0, 0, 0, hwnd, HWND_TOP, IDD_STATUS_TYPE, NULL, NULL );
    WinCreateWindow( hwnd, WC_STATIC, "", SS_TEXT | DT_VCENTER | WS_VISIBLE,
                     0, 0, 0, 0, hwnd, HWND_TOP, IDD_STATUS_FLAGS, NULL, NULL );
    WinCreateWindow( hwnd, WC_STATIC, "", SS_TEXT | DT_CENTER | DT_VCENTER | WS_VISIBLE,
                     0, 0, 0, 0, hwnd, HWND_TOP, IDD_STATUS_MODIFIED, NULL, NULL );

    lClr = CLR_DARKRED;
    WinSetPresParam( WinWindowFromID( hwnd, IDD_STATUS_MODIFIED ),
                     PP_FOREGROUNDCOLORINDEX, sizeof( lClr ), &lClr );

    // Set up the containers
    VolumeContainerSetup( pGlobal );
    PartitionContainerSetup( pGlobal );

    // Set the default fonts
    MainWindowSetFonts( hwnd, pGlobal );
}


/* ------------------------------------------------------------------------- *
 * MainWindowSetFonts()                                                      *
 *                                                                           *
 * Sets the various fonts on the main window according to the global font    *
 * preferences.                                                              *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwnd:    handle of the program client window                 *
 *   PDVMGLOBAL pGlobal: pointer to global data structure                    *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void MainWindowSetFonts( HWND hwnd, PDVMGLOBAL pGlobal )
{
    HWND hwndCtl;

    /* Main window font; this is only specifically used on the status bars,
     * but it will also be inherited as the default for all other fonts if
     * (and only if) they have no values of their own defined.
     */
    if ( pGlobal->szFontMain[ 0 ] ) {
        CHAR szFont[ FACESIZE+4 ] = {0};
        HPS  hps;

        WinSetPresParam( hwnd, PP_FONTNAMESIZE,
                         strlen( pGlobal->szFontMain ) + 1,
                         (PVOID) pGlobal->szFontMain );
        hps = WinGetPS( hwnd );
        strcpy( szFont, pGlobal->szFontMain );
        GetBoldFontPP( hps, szFont );
        WinReleasePS( hps );
        WinSetPresParam( WinWindowFromID( hwnd, IDD_STATUS_MODIFIED ),
                         PP_FONTNAMESIZE, strlen( szFont )+1, (PVOID) szFont );
    }
    else {
        WinSetPresParam( hwnd, PP_FONTNAMESIZE, 11, (PVOID)"9.WarpSans");
        WinSetPresParam( WinWindowFromID( hwnd, IDD_STATUS_MODIFIED ),
                         PP_FONTNAMESIZE, 16, (PVOID)"9.WarpSans Bold");
    }

    // Volume container font
    hwndCtl = WinWindowFromID( pGlobal->hwndVolumes, IDD_VOLUMES );
    if ( hwndCtl && pGlobal->szFontCnr[ 0 ] ) {
        WinSetPresParam( hwndCtl, PP_FONTNAMESIZE,
                         strlen( pGlobal->szFontCnr ) + 1,
                         (PVOID) pGlobal->szFontCnr );
    }

    // Volume details panel font
    hwndCtl = WinWindowFromID( pGlobal->hwndVolumes, IDD_VOL_INFO );
    if ( hwndCtl && pGlobal->szFontInfo[ 0 ] ) {
        WinSetPresParam( hwndCtl, PP_FONTNAMESIZE,
                         strlen( pGlobal->szFontInfo ) + 1,
                         (PVOID) pGlobal->szFontInfo );
    }

    // Disk list font
    if ( pGlobal->hwndDisks && pGlobal->szFontDisks[ 0 ] ) {
        WinSetPresParam( pGlobal->hwndDisks, PP_FONTNAMESIZE,
                         strlen( pGlobal->szFontDisks ) + 1,
                         (PVOID) pGlobal->szFontDisks );
    }
}


/* ------------------------------------------------------------------------- *
 * MainWindowSize()                                                          *
 *                                                                           *
 * (Re)sizes the main window contents.                                       *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwnd:    handle of the program client window                 *
 *   LONG       lWidth:  new client width                                    *
 *   LONG       lHeight: new client height                                   *
 *   PDVMGLOBAL pGlobal: pointer to global data structure                    *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void MainWindowSize( HWND hwnd, LONG lWidth, LONG lHeight, PDVMGLOBAL pGlobal )
{
    LONG        lY,         // bottom of split-bar window
                lW,         // width of current status area
                lSX, lSW;   // position and width of current status field
    RECTL       rcl;        // boundaries of status area
    FONTMETRICS fm;         // current font metrics
    HPS         hps;        // client presentation space

    hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
    SetFontFromPP( hps, hwnd, GetCurrentDPI( hwnd ));
    GpiQueryFontMetrics( hps, sizeof( FONTMETRICS ), &fm );
    WinEndPaint( hps );

    // Resize the status bars
    pGlobal->lStatusWidth  = lWidth;
    pGlobal->lStatusHeight = 2 + ( fm.lEmHeight * 1.2 ) +
                             fm.lExternalLeading + fm.lInternalLeading;
    pGlobal->lStatusDiv = lWidth - ( lWidth / 6 );

    // text fields in the left status area
    lW  = pGlobal->lStatusDiv - 7;
    lSX = 5;
    lSW = lW / 2;
    WinSetWindowPos( WinWindowFromID( hwnd, IDD_STATUS_SELECTED ), HWND_TOP,
                     lSX, 2, lSW, pGlobal->lStatusHeight - 2, SWP_SIZE | SWP_MOVE );
    lSX += lSW + 2;
    lSW  = ( lW - lSX ) / 3;
    WinSetWindowPos( WinWindowFromID( hwnd, IDD_STATUS_SIZE ), HWND_TOP,
                     lSX, 2, lSW, pGlobal->lStatusHeight - 2, SWP_SIZE | SWP_MOVE );
    lSX += lSW + 2;
    lSW  = (( lW - lSX ) / 3 ) * 2;
    WinSetWindowPos( WinWindowFromID( hwnd, IDD_STATUS_TYPE ), HWND_TOP,
                     lSX, 2, lSW, pGlobal->lStatusHeight - 2, SWP_SIZE | SWP_MOVE );
    lSX += lSW + 2;
    lSW  = lW - lSX;
    WinSetWindowPos( WinWindowFromID( hwnd, IDD_STATUS_FLAGS ), HWND_TOP,
                     lSX, 2, lSW, pGlobal->lStatusHeight - 2, SWP_SIZE | SWP_MOVE );

    // (lone) text field in the right status area
    lW = lWidth - pGlobal->lStatusDiv - 3;
    lSX = pGlobal->lStatusDiv + 3;
    lSW = lW - 2;
    WinSetWindowPos( WinWindowFromID( hwnd, IDD_STATUS_MODIFIED ), HWND_TOP,
                     lSX, 2, lSW, pGlobal->lStatusHeight - 2, SWP_SIZE | SWP_MOVE );

    // Resize the main splitbar window
    lY = pGlobal->lStatusHeight + 3;
    WinSetWindowPos( pGlobal->hwndSplit, HWND_TOP,
                     0, lY, lWidth, lHeight - lY, SWP_SIZE | SWP_MOVE );

    // Force a redraw of the status bars
    rcl.xLeft   = 0;
    rcl.yBottom = 0;
    rcl.xRight  = pGlobal->lStatusWidth;
    rcl.yTop    = lY;
    WinInvalidateRect( hwnd, &rcl, TRUE );

}


/* ------------------------------------------------------------------------- *
 * MainWindowFocus()                                                         *
 *                                                                           *
 * Switches keyboard focus to the next/previous control.  There are only     *
 * three controls on the main window that accept keyboard focus: the volume  *
 * container, the partition container, and the disk list.                    *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd : handle of the program client window                         *
 *   BOOL fNext: TRUE if switching to the next control, FALSE if to previous *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void MainWindowFocus( HWND hwnd, BOOL fNext )
{
    PDVMGLOBAL pGlobal;             // global data (includes disk list handle)
    HWND hwndFocus,                 // handle of current focus window
         hwndTarget,                // handle of target focus window
         hwndVC,                    // handle of volume container
         hwndPC;                    // handle of partition container
    BOOL fPCShown;                  // is partition container visible?


    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( !pGlobal ) return;

    hwndVC = WinWindowFromID( pGlobal->hwndVolumes, IDD_VOLUMES );
    hwndPC = (HWND) WinSendMsg( WinWindowFromID( pGlobal->hwndVolumes,
                                                 IDD_VOL_INFO ),
                                VIM_GETCONTAINERHWND, 0, 0 );
    fPCShown = WinIsWindowVisible( hwndPC );

    hwndFocus = WinQueryFocus( HWND_DESKTOP );
    if ( hwndFocus == hwndVC ) {
        hwndTarget = ( fNext && fPCShown ) ? hwndPC : pGlobal->hwndDisks;
    }
    else if ( hwndFocus == hwndPC ) {
        hwndTarget = fNext ? pGlobal->hwndDisks : hwndVC;
    }
    else if ( hwndFocus == pGlobal->hwndDisks ) {
        hwndTarget = ( fNext || !fPCShown ) ? hwndVC : hwndPC;
    }
    else
        hwndTarget = hwndVC;

    WinSetFocus( HWND_DESKTOP, hwndTarget );
}


/* ------------------------------------------------------------------------- *
 * PopupEngineError()                                                        *
 *                                                                           *
 * Displays message-box for LVM API error messages.                          *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PSZ pszMessage : The error message to display, or NULL to have this     *
 *                    function look it up from the error code                *
 *   CARDINAL32 code: The LVM engine error code                              *
 *   HWND hwnd      : Handle of the window producing the message-box         *
 *   HAB hab        : Anchor-block for the application                       *
 *   HMODULE hmri   : Handle of the resource containing NLV stringtables     *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void PopupEngineError( PSZ pszMessage, CARDINAL32 code, HWND hwnd, HAB hab, HMODULE hmri )
{
    CHAR   szRes[ STRING_RES_MAXZ ],
           szRes2[ STRING_RES_MAXZ + 12 ];
    PSZ    pszTitle;
    ULONG  cch,
           ulStyle;
    USHORT usID;

    cch = WinLoadString( hab, hmri, IDS_ERROR_ENGINE, STRING_RES_MAXZ, szRes );
    pszTitle = (PSZ) malloc( cch + 4 );
    if ( pszTitle )
        sprintf( pszTitle, szRes, code );

    if (( code & 0xFF00 ) == LVM_ERROR_INCOMPATIBLE_PARTITIONING )
        usID = IDD_ENGINE_ERROR_INCOMPATIBLE;
    else
        usID = IDD_ENGINE_ERROR + code;

    if ( !pszMessage ) {
        if (( code & 0xFF00 ) == LVM_ERROR_INCOMPATIBLE_PARTITIONING ) {
            WinLoadString( hab, hmri, IDS_ERROR_LVM_INCOMPATIBLE, STRING_RES_MAXZ, szRes );
            sprintf( szRes2, szRes, (code & 0xFF)+1 );
            pszMessage = szRes2;
        }
        else {
            WinLoadString( hab, hmri, IDS_ERROR_LVM + code, STRING_RES_MAXZ, szRes );
            pszMessage = szRes;
        }
    }

    ulStyle = MB_MOVEABLE | MB_APPLMODAL | MB_OK | MB_HELP | MB_ERROR;
    WinMessageBox( HWND_DESKTOP, hwnd, pszMessage,
                   pszTitle? pszTitle: SZ_ERROR_GENERIC, usID, ulStyle );

    if ( pszTitle ) free ( pszTitle );
}


/* ------------------------------------------------------------------------- *
 * CheckBootable()                                                           *
 *                                                                           *
 * See if there are any bootable volumes defined.  A bootable volume is one  *
 * that meets all of the following criteria:                                 *
 *   - Must have a fixed drive letter                                        *
 *   - If Air-Boot is installed: must simply be a compatibility volume.      *
 *     Otherwise: must have either the LVM bootable/startable flag set.      *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *     PDVMGLOBAL pGlobal: Pointer to global program data                    *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   FALSE if there are no bootable volumes found.                           *
 * ------------------------------------------------------------------------- */
BOOL CheckBootable( PDVMGLOBAL pGlobal )
{
    PLVMVOLUMEINFO pVol;
    ULONG          i;
    BOOL           fBootable;

    if ( !pGlobal || !pGlobal->ulVolumes || ! pGlobal->volumes )
        return FALSE;

    // Look through all volumes until we find one which meets the criteria.
    fBootable = FALSE;
    for ( i = 0; !fBootable && ( i < pGlobal->ulVolumes ); i++ ) {
        pVol = (PLVMVOLUMEINFO)(pGlobal->volumes + i);
        if ( !pVol->cPreference ||
             !( pVol->cPreference >= 'C') && (pVol->cPreference <= 'Z'))
            continue;       // no permanent drive letter, so skip

        // OK, it has a valid drive letter.  Now check the bootable status.
        if (( pGlobal->fsEngine & FS_ENGINE_AIRBOOT ) && pVol->fCompatibility )
            // Air-Boot will accept any compatibility volume
            fBootable = TRUE;
        else if (( pVol->iStatus > 0 ) && ( pVol->iStatus < 3 ))
            // Otherwise check the volume's status field
            fBootable = TRUE;
    }
    return fBootable;
}


/* ------------------------------------------------------------------------- *
 * PrefsDlgProc()                                                            *
 *                                                                           *
 * Dialog procedure for the preferences dialog.                              *
 * See OS/2 PM reference for a description of input and output.              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY PrefsDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    PDVMGLOBAL pGlobal;
//    CHAR       szRes[ STRING_RES_MAXZ ],
//               szBuffer[ STRING_RES_MAXZ + 10 ];
//    ULONG      cch;
    USHORT     usCtl;


    switch( msg ) {

        case WM_INITDLG:
            pGlobal = (PDVMGLOBAL) mp2;
            if ( !pGlobal ) break;

            // Set up the borders
            g_pfnRecProc = WinSubclassWindow(
                             WinWindowFromID( hwnd, IDD_DIALOG_INSET ),
                             InsetBorderProc );
            WinSubclassWindow( WinWindowFromID( hwnd, IDD_DIALOG_INSET2 ),
                               InsetBorderProc );

            WinSetWindowPtr( hwnd, 0, pGlobal );

            if ( pGlobal->szFontDlgs[ 0 ] )
                WinSetPresParam( hwnd, PP_FONTNAMESIZE,
                                 strlen( pGlobal->szFontDlgs ) + 1,
                                 (PVOID) pGlobal->szFontDlgs );

            if ( pGlobal->fsProgram & FS_APP_IBMTERMS )
                WinCheckButton( hwnd, IDD_PREFS_TERMS_IBM, 1 );
            if ( pGlobal->fsProgram & FS_APP_IECSIZES )
                WinCheckButton( hwnd, IDD_PREFS_TERMS_IEC, 1 );
            if ( pGlobal->fsProgram & FS_APP_BOOTWARNING )
                WinCheckButton( hwnd, IDD_PREFS_BOOT_WARNING, 1 );
            if ( pGlobal->fsProgram & FS_APP_HIDE_FREEPRM )
                WinCheckButton( hwnd, IDD_PREFS_NO_EMPTY_DISKS, 1 );
            if ( pGlobal->fsProgram & FS_APP_HIDE_NONLVM )
                WinCheckButton( hwnd, IDD_PREFS_NO_ALIEN_VOLUMES, 1 );
            if ( pGlobal->fsProgram & FS_APP_UNIFORM )
                WinCheckButton( hwnd, IDD_PREFS_UNIFORM, 1 );
            if ( pGlobal->fsProgram & FS_APP_AUTOSELECT )
                WinCheckButton( hwnd, IDD_PREFS_AUTOSELECT, 1 );

            if ( pGlobal->fsProgram & FS_APP_PMSTYLE )
                usCtl = IDD_PREFS_STYLE_PM;
            else
                usCtl = IDD_PREFS_STYLE_WEB;
            WinSendDlgItemMsg( hwnd, usCtl, BM_CLICK, MPFROMSHORT( TRUE ), 0 );

            if ( pGlobal->fsProgram & FS_APP_ENABLE_BM )
                WinCheckButton( hwnd, IDD_PREFS_ENABLE_BM, 1 );
            if ( pGlobal->fsProgram & FS_APP_ENABLE_AB )
                WinCheckButton( hwnd, IDD_PREFS_ENABLE_AB, 1 );

            CentreWindow( hwnd, WinQueryWindow( hwnd, QW_OWNER ), SWP_SHOW );

            return (MRESULT) FALSE;


        case WM_COMMAND:
            switch ( SHORT1FROMMP( mp1 )) {

                case DID_OK:
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );

                    if ( WinQueryButtonCheckstate( hwnd, IDD_PREFS_TERMS_IBM ))
                        pGlobal->fsProgram |= FS_APP_IBMTERMS;
                    else
                        pGlobal->fsProgram &= ~FS_APP_IBMTERMS;

                    if ( WinQueryButtonCheckstate( hwnd, IDD_PREFS_TERMS_IEC ))
                        pGlobal->fsProgram |= FS_APP_IECSIZES;
                    else
                        pGlobal->fsProgram &= ~FS_APP_IECSIZES;

                    if ( WinQueryButtonCheckstate( hwnd, IDD_PREFS_BOOT_WARNING ))
                        pGlobal->fsProgram |= FS_APP_BOOTWARNING;
                    else
                        pGlobal->fsProgram &= ~FS_APP_BOOTWARNING;

                    if ( WinQueryButtonCheckstate( hwnd, IDD_PREFS_UNIFORM ))
                        pGlobal->fsProgram |= FS_APP_UNIFORM;
                    else
                        pGlobal->fsProgram &= ~FS_APP_UNIFORM;

                    if ( WinQueryButtonCheckstate( hwnd, IDD_PREFS_AUTOSELECT ))
                        pGlobal->fsProgram |= FS_APP_AUTOSELECT;
                    else
                        pGlobal->fsProgram &= ~FS_APP_AUTOSELECT;

                    if ( WinQueryButtonCheckstate( hwnd, IDD_PREFS_NO_EMPTY_DISKS ))
                        pGlobal->fsProgram |= FS_APP_HIDE_FREEPRM;
                    else
                        pGlobal->fsProgram &= ~FS_APP_HIDE_FREEPRM;

                    if ( WinQueryButtonCheckstate( hwnd, IDD_PREFS_NO_ALIEN_VOLUMES ))
                        pGlobal->fsProgram |= FS_APP_HIDE_NONLVM;
                    else
                        pGlobal->fsProgram &= ~FS_APP_HIDE_NONLVM;

                    if ( WinQueryButtonCheckstate( hwnd, IDD_PREFS_STYLE_PM ))
                        pGlobal->fsProgram |= FS_APP_PMSTYLE;
                    else
                        pGlobal->fsProgram &= ~FS_APP_PMSTYLE;

                    if ( WinQueryButtonCheckstate( hwnd, IDD_PREFS_ENABLE_BM ))
                        pGlobal->fsProgram |= FS_APP_ENABLE_BM;
                    else
                        pGlobal->fsProgram &= ~FS_APP_ENABLE_BM;

                    if ( WinQueryButtonCheckstate( hwnd, IDD_PREFS_ENABLE_AB ))
                        pGlobal->fsProgram |= FS_APP_ENABLE_AB;
                    else
                        pGlobal->fsProgram &= ~FS_APP_ENABLE_AB;

                    break;
            }
            break;


        default: break;
    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * FontsDlgProc()                                                            *
 *                                                                           *
 * Dialog procedure for the font preferences dialog.                         *
 * See OS/2 PM reference for a description of input and output.              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY FontsDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    PDVMGLOBAL pGlobal;
    CHAR       szRes[ STRING_RES_MAXZ ];
    ULONG      cch;


    switch( msg ) {

        case WM_INITDLG:
            pGlobal = (PDVMGLOBAL) mp2;
            if ( !pGlobal ) break;
            WinSetWindowPtr( hwnd, 0, pGlobal );

            // Set up the borders
            g_pfnRecProc = WinSubclassWindow(
                             WinWindowFromID( hwnd, IDD_DIALOG_INSET ),
                             InsetBorderProc
                           );
            WinSubclassWindow( WinWindowFromID( hwnd, IDD_DIALOG_DIVIDER ),
                               HorizontalRuleProc );

            // Set up the preview controls
            g_pfnTextProc = WinSubclassWindow(
                              WinWindowFromID( hwnd, IDD_FONTS_STATUS_PREVIEW ),
                              FontPreviewProc
                            );
            WinSubclassWindow( WinWindowFromID( hwnd, IDD_FONTS_CONTAINER_PREVIEW ),
                               FontPreviewProc );
            WinSubclassWindow( WinWindowFromID( hwnd, IDD_FONTS_DETAILS_PREVIEW ),
                               FontPreviewProc );
            WinSubclassWindow( WinWindowFromID( hwnd, IDD_FONTS_DISKS_PREVIEW ),
                               FontPreviewProc );
            WinSubclassWindow( WinWindowFromID( hwnd, IDD_FONTS_DIALOG_PREVIEW ),
                               FontPreviewProc );

            if ( pGlobal->szFontMain[ 0 ] )
                WinSetPresParam(
                    WinWindowFromID( hwnd, IDD_FONTS_STATUS_PREVIEW ),
                    PP_FONTNAMESIZE, strlen( pGlobal->szFontMain ) + 1,
                    (PVOID) pGlobal->szFontMain
                );
            else
                WinSetPresParam(
                    WinWindowFromID( hwnd, IDD_FONTS_STATUS_PREVIEW ),
                    PP_FONTNAMESIZE, 11, (PVOID)"9.WarpSans"
                );

            if ( pGlobal->szFontCnr[ 0 ] )
                WinSetPresParam(
                    WinWindowFromID( hwnd, IDD_FONTS_CONTAINER_PREVIEW ),
                    PP_FONTNAMESIZE, strlen( pGlobal->szFontCnr ) + 1,
                    (PVOID) pGlobal->szFontCnr
                );
            else
                WinSetPresParam(
                    WinWindowFromID( hwnd, IDD_FONTS_CONTAINER_PREVIEW ),
                    PP_FONTNAMESIZE, 11, (PVOID)"9.WarpSans"
                );

            if ( pGlobal->szFontInfo[ 0 ] )
                WinSetPresParam(
                    WinWindowFromID( hwnd, IDD_FONTS_DETAILS_PREVIEW ),
                    PP_FONTNAMESIZE, strlen( pGlobal->szFontInfo ) + 1,
                    (PVOID) pGlobal->szFontInfo
                );
            else
                WinSetPresParam(
                    WinWindowFromID( hwnd, IDD_FONTS_DETAILS_PREVIEW ),
                    PP_FONTNAMESIZE, 11, (PVOID)"9.WarpSans"
                );

            if ( pGlobal->szFontDisks[ 0 ] )
                WinSetPresParam(
                    WinWindowFromID( hwnd, IDD_FONTS_DISKS_PREVIEW ),
                    PP_FONTNAMESIZE, strlen( pGlobal->szFontDisks ) + 1,
                    (PVOID) pGlobal->szFontDisks
                );
            else
                WinSetPresParam(
                    WinWindowFromID( hwnd, IDD_FONTS_DISKS_PREVIEW ),
                    PP_FONTNAMESIZE, 11, (PVOID)"9.WarpSans"
                );

            if ( pGlobal->szFontDlgs[ 0 ] ) {
                // Use the dialog font for the actual dialog as well...
                WinSetPresParam( hwnd, PP_FONTNAMESIZE,
                    strlen( pGlobal->szFontDlgs ) + 1,
                    (PVOID) pGlobal->szFontDlgs
                );
                WinSetPresParam(
                    WinWindowFromID( hwnd, IDD_FONTS_DIALOG_PREVIEW ),
                    PP_FONTNAMESIZE, strlen( pGlobal->szFontDlgs ) + 1,
                    (PVOID) pGlobal->szFontDlgs
                );
            }
            else {
                WinLoadString( pGlobal->hab, pGlobal->hmri,
                               IDS_LABELS_DEFAULT_FONT, STRING_RES_MAXZ, szRes );
                WinSetDlgItemText( hwnd, IDD_FONTS_DIALOG_PREVIEW, szRes );
            }

            CentreWindow( hwnd, WinQueryWindow( hwnd, QW_OWNER ), SWP_SHOW );


            return (MRESULT) FALSE;


        case WM_COMMAND:
            switch ( SHORT1FROMMP( mp1 )) {

                case IDD_FONTS_CONTAINER_BTN:
                    SelectFont( hwnd, IDD_FONTS_CONTAINER_PREVIEW );
                    return (MRESULT) 0;

                case IDD_FONTS_DETAILS_BTN:
                    SelectFont( hwnd, IDD_FONTS_DETAILS_PREVIEW );
                    return (MRESULT) 0;

                case IDD_FONTS_DISKS_BTN:
                    SelectFont( hwnd, IDD_FONTS_DISKS_PREVIEW );
                    return (MRESULT) 0;

                case IDD_FONTS_STATUS_BTN:
                    SelectFont( hwnd, IDD_FONTS_STATUS_PREVIEW );
                    return (MRESULT) 0;

                case IDD_FONTS_DIALOG_BTN:
                    SelectFont( hwnd, IDD_FONTS_DIALOG_PREVIEW );
                    return (MRESULT) 0;

                case IDD_FONTS_DIALOG_CLEAR:
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    WinRemovePresParam(
                        WinWindowFromID( hwnd, IDD_FONTS_DIALOG_PREVIEW ),
                        PP_FONTNAMESIZE );
                    WinLoadString( pGlobal->hab, pGlobal->hmri,
                                   IDS_LABELS_DEFAULT_FONT, STRING_RES_MAXZ, szRes );
                    WinSetDlgItemText( hwnd, IDD_FONTS_DIALOG_PREVIEW, szRes );
                    return (MRESULT) 0;

                case DID_OK:
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    WinQueryPresParam(
                        WinWindowFromID( hwnd, IDD_FONTS_CONTAINER_PREVIEW ),
                        PP_FONTNAMESIZE, 0, NULL, sizeof( pGlobal->szFontCnr ),
                        (PVOID) pGlobal->szFontCnr, QPF_NOINHERIT );
                    WinQueryPresParam(
                        WinWindowFromID( hwnd, IDD_FONTS_DETAILS_PREVIEW ),
                        PP_FONTNAMESIZE, 0, NULL, sizeof( pGlobal->szFontInfo ),
                        (PVOID) pGlobal->szFontInfo, QPF_NOINHERIT );
                    WinQueryPresParam(
                        WinWindowFromID( hwnd, IDD_FONTS_DISKS_PREVIEW ),
                        PP_FONTNAMESIZE, 0, NULL, sizeof( pGlobal->szFontDisks ),
                        (PVOID) pGlobal->szFontDisks, QPF_NOINHERIT );
                    WinQueryPresParam(
                        WinWindowFromID( hwnd, IDD_FONTS_STATUS_PREVIEW ),
                        PP_FONTNAMESIZE, 0, NULL, sizeof( pGlobal->szFontMain ),
                        (PVOID) pGlobal->szFontMain, QPF_NOINHERIT );
                    cch = WinQueryPresParam(
                            WinWindowFromID( hwnd, IDD_FONTS_DIALOG_PREVIEW ),
                            PP_FONTNAMESIZE, 0, NULL, sizeof( pGlobal->szFontDlgs ),
                            (PVOID) pGlobal->szFontDlgs, QPF_NOINHERIT );
                    if ( !cch ) *(pGlobal->szFontDlgs) = 0;
                    break;

            }
            break;

        default: break;
    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * SelectFont()                                                              *
 *                                                                           *
 * Presents a font selection dialog, with the current font of the specified  *
 * control preselected.                                                      *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd  : handle of the current dialog window                        *
 *   USHORT usID: ID of the control whose font is to be used                 *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void SelectFont( HWND hwnd, USHORT usID )
{
    FONTMETRICS fm;
    HWND    hwndCtl;
    HPS     hpsCtl;
    FONTDLG fontdlg = {0};
    CHAR    szFont[ FACESIZE+4 ] = {0};
    PSZ     pszName;
    ULONG   ulSize;

    hwndCtl = WinWindowFromID( hwnd, usID );
    hpsCtl  = WinGetPS( hwndCtl );
    GpiQueryFontMetrics( hpsCtl, sizeof( fm ), &fm );

    // This part is just to get the requested point size
    if ( ! WinQueryPresParam( hwndCtl, PP_FONTNAMESIZE, 0, NULL,
                              sizeof( szFont ), szFont, QPF_NOINHERIT ))
    {
        strcpy( szFont, "9.WarpSans");
    }
    gpihSplitPresFont( szFont, &ulSize, &pszName );

    strncpy( szFont, fm.szFamilyname, FACESIZE-1 );
    fontdlg.cbSize = sizeof( FONTDLG );
    fontdlg.hpsScreen      = hpsCtl;
    fontdlg.pszTitle       = NULL;
    fontdlg.pszPreview     = NULL;
    fontdlg.pfnDlgProc     = NULL;
    fontdlg.pszFamilyname  = szFont;
    fontdlg.usFamilyBufLen = sizeof( szFont );
    fontdlg.fxPointSize    = MAKEFIXED( ulSize, 0 );
    fontdlg.flType         = (fm.fsSelection & FM_SEL_ITALIC)? FTYPE_ITALIC: 0;
    fontdlg.usWeight       = (USHORT) fm.usWeightClass;
    fontdlg.clrFore        = SYSCLR_WINDOWTEXT;
    fontdlg.clrBack        = SYSCLR_WINDOW;
    fontdlg.fl             = FNTS_CENTER;
    fontdlg.hMod           = NULLHANDLE;
    WinFontDlg( HWND_DESKTOP, hwnd, &fontdlg );

    if ( fontdlg.lReturn == DID_OK ) {
        ulSize = fontdlg.fxPointSize / 65536;
        sprintf( szFont, "%u.", ulSize );
        strncat( szFont, fontdlg.fAttrs.szFacename, FACESIZE+3 );
        WinSetPresParam( hwndCtl, PP_FONTNAMESIZE,
                         strlen( szFont ) + 1, (PVOID) szFont );
    }

    WinReleasePS( hpsCtl );
}


/* ------------------------------------------------------------------------- *
 * ProdInfoDlgProc()                                                         *
 *                                                                           *
 * Dialog procedure for the 'product information' dialog.                    *
 * See OS/2 PM reference for a description of input and output.              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY ProdInfoDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    PDVMGLOBAL pGlobal;
    CHAR       szRes[ STRING_RES_MAXZ ],
               szBuffer[ STRING_RES_MAXZ + 10 ];
    ULONG      cch;
    IPT        ipt;


    switch( msg ) {

        case WM_INITDLG:
            pGlobal = (PDVMGLOBAL) mp2;
            if ( !pGlobal ) break;

            // Set the version/revision and copyright text
            WinLoadString( pGlobal->hab, pGlobal->hmri,
                           IDS_PRODUCT_VERSION, STRING_RES_MAXZ, szRes );
            sprintf( szBuffer, szRes, SZ_VERSION );
            WinSetDlgItemText( hwnd, IDD_ABOUT_VERSION, szBuffer );
            WinLoadString( pGlobal->hab, pGlobal->hmri,
                           IDS_PRODUCT_REVISION, STRING_RES_MAXZ, szRes );
            sprintf( szBuffer, szRes, SZ_BUILD );
            WinSetDlgItemText( hwnd, IDD_ABOUT_BUILD, szBuffer );
            WinLoadString( pGlobal->hab, pGlobal->hmri,
                           IDS_PRODUCT_COPYRIGHT, STRING_RES_MAXZ, szRes );
            sprintf( szBuffer, szRes, SZ_COPYRIGHT );
            WinSetDlgItemText( hwnd, IDD_ABOUT_AUTHOR, szBuffer );

            // Set the license text
            WinSendDlgItemMsg( hwnd, IDD_ABOUT_LICENSE, MLM_FORMAT,
                               MPFROMSHORT( MLFIE_NOTRANS ), 0 );
            WinSendDlgItemMsg( hwnd, IDD_ABOUT_LICENSE, MLM_SETIMPORTEXPORT,
                               MPFROMP( szRes ), MPFROMLONG( STRING_RES_MAXZ ));
            cch = WinLoadString( pGlobal->hab, pGlobal->hmri,
                                 IDS_GPL_OVERVIEW_1, STRING_RES_MAXZ, szRes );
            ipt = 0;
            WinSendDlgItemMsg( hwnd, IDD_ABOUT_LICENSE, MLM_IMPORT,
                               MPFROMP( &ipt ), MPFROMLONG( cch ));
            cch = WinLoadString( pGlobal->hab, pGlobal->hmri,
                                  IDS_GPL_OVERVIEW_2, STRING_RES_MAXZ, szRes );
            WinSendDlgItemMsg( hwnd, IDD_ABOUT_LICENSE, MLM_IMPORT,
                               MPFROMP( &ipt ), MPFROMLONG( cch ));
            cch = WinLoadString( pGlobal->hab, pGlobal->hmri,
                                  IDS_GPL_OVERVIEW_3, STRING_RES_MAXZ, szRes );
            WinSendDlgItemMsg( hwnd, IDD_ABOUT_LICENSE, MLM_IMPORT,
                               MPFROMP( &ipt ), MPFROMLONG( cch ));
            cch = WinLoadString( pGlobal->hab, pGlobal->hmri,
                                  IDS_GPL_OVERVIEW_4, STRING_RES_MAXZ, szRes );
            WinSendDlgItemMsg( hwnd, IDD_ABOUT_LICENSE, MLM_IMPORT,
                               MPFROMP( &ipt ), MPFROMLONG( cch ));
            cch = WinLoadString( pGlobal->hab, pGlobal->hmri,
                                  IDS_GPL_OVERVIEW_5, STRING_RES_MAXZ, szRes );
            WinSendDlgItemMsg( hwnd, IDD_ABOUT_LICENSE, MLM_IMPORT,
                               MPFROMP( &ipt ), MPFROMLONG( cch ));
            cch = WinLoadString( pGlobal->hab, pGlobal->hmri,
                                  IDS_GPL_OVERVIEW_6, STRING_RES_MAXZ, szRes );
            WinSendDlgItemMsg( hwnd, IDD_ABOUT_LICENSE, MLM_IMPORT,
                               MPFROMP( &ipt ), MPFROMLONG( cch ));

            g_pfnRecProc = WinSubclassWindow(
                             WinWindowFromID( hwnd, IDD_DIALOG_DIVIDER ),
                             HorizontalRuleProc
                           );

            if ( pGlobal->szFontDlgs[ 0 ] )
                WinSetPresParam( hwnd, PP_FONTNAMESIZE,
                                 strlen( pGlobal->szFontDlgs ) + 1,
                                 (PVOID) pGlobal->szFontDlgs );

            CentreWindow( hwnd, WinQueryWindow( hwnd, QW_OWNER ), SWP_SHOW );
            return (MRESULT) FALSE;


        default: break;
    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * VolumesPanelProc()                                                        *
 *                                                                           *
 * Window procedure for the top (or "logical view") panel, which shows the   *
 * list of volumes as well as the current-volume info control.               *
 *                                                                           *
 * See OS/2 PM reference for a description of input and output.              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY VolumesPanelProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    static PDVMGLOBAL     pGlobal;         // global application data
    PDVMLOGVIEWCTLDATA    pCtl;            // internal window control data
    VICTLDATA             infodata = {0};  // control data for info panel

    PNOTIFYRECORDEMPHASIS pNotify;
    PMINIRECORDCORE       pRec;

    HWND   hwndOwner;
    HPS    hps;
    RECTL  rcl;
    POINTL ptl;
    LONG   clrBG,
           clrFG;
    ULONG  ulID,
           rc;
    SHORT  cx, cy,
           cxInfo;
    CHAR   szRes1[ STRING_RES_MAXZ ] = {0},
           szRes2[ STRING_RES_MAXZ ] = {0};


    switch( msg ) {

        case WM_CREATE:
            pGlobal = (PDVMGLOBAL) mp1;
            if ( !pGlobal ) return (MRESULT) TRUE;

            pCtl = (PDVMLOGVIEWCTLDATA) calloc( 1, sizeof( DVMLOGVIEWCTLDATA ));
            if ( !pCtl ) return (MRESULT) TRUE;

            pCtl->hwndVolumes = WinCreateWindow( hwnd, WC_CONTAINER, "Volumes",
                                                 WS_VISIBLE | CS_SIZEREDRAW,
                                                 0, 0, 0, 0, hwnd, HWND_TOP,
                                                 IDD_VOLUMES, NULL, NULL );
            if ( !VIRegisterClass( pGlobal->hab )) {
                WinLoadString( pGlobal->hab, pGlobal->hmri, IDS_ERROR_REGISTER,
                               STRING_RES_MAXZ, szRes1 );
                sprintf( szRes2, szRes1, WC_VOLUMEINFO,
                         ERRORIDERROR( WinGetLastError( pGlobal->hab )));
                DebugBox( szRes2 );
                return (MRESULT) TRUE;
            }

            infodata.cb = sizeof( VICTLDATA );
            infodata.ulCnrID = IDD_VOL_PARTITIONS;
            infodata.fsStyle = ( pGlobal->fsProgram & FS_APP_PMSTYLE ) ?
                                VIV_FS_3DSEP : 0;

#ifdef VICTLDATA_STRINGS
            WinLoadString( pGlobal->hab, pGlobal->hmri, IDS_LABELS_FILESYSTEM,
                           STRING_RES_MAXZ, szRes1 );
            WinLoadString( pGlobal->hab, pGlobal->hmri, IDS_LABELS_SIZE,
                           STRING_RES_MAXZ, szRes2 );
            infodata.pszFSL   = strdup( szRes1 );
            infodata.pszSizeL = strdup( szRes2 );
            WinLoadString( pGlobal->hab, pGlobal->hmri, IDS_LABELS_DEVICE,
                           STRING_RES_MAXZ, szRes1 );
            infodata.pszDeviceL = strdup( szRes1 );
#endif

            pCtl->hwndInfo = WinCreateWindow( hwnd, WC_VOLUMEINFO, "",
                                              WS_VISIBLE | CS_SIZEREDRAW,
                                              0, 0, 0, 0, hwnd, HWND_TOP,
                                              IDD_VOL_INFO, &infodata, NULL );

//            pGlobal->hwndPopupVolume;

            WinSetWindowPtr( hwnd, 0, pCtl );
            return (MRESULT) FALSE;
            // End WM_CREATE


        case WM_DESTROY:
            if (( pCtl = WinQueryWindowPtr( hwnd, 0 )) != NULL ) {
                // TODO destroy the child windows
                free( pCtl );
            }
            break;


        case WM_COMMAND:
            // Pass these up to the main window
            hwndOwner = WinQueryWindow( hwnd, QW_OWNER );
            WinPostMsg( hwndOwner, msg, mp1, mp2 );
            return (MRESULT) 0;


        case WM_CONTROL:
            switch( SHORT1FROMMP( mp1 )) {

                case IDD_VOLUMES:
                    hwndOwner = WinQueryWindow( hwnd, QW_OWNER );
                    pCtl = WinQueryWindowPtr( hwnd, 0 );

                    switch( SHORT2FROMMP( mp1 )) {

                        case CN_EMPHASIS:
                            // A volume has been selected
                            pNotify = (PNOTIFYRECORDEMPHASIS) mp2;
                            if (( pNotify->fEmphasisMask & CRA_SELECTED ) &&
                                ( pNotify->pRecord->flRecordAttr & CRA_SELECTED ))
                            {
                                VolumeContainerSelect( hwndOwner, pGlobal->hwndPopupVolume,
                                                       (PDVMVOLUMERECORD) (pNotify->pRecord) );
                                //Status_Volume( hwndOwner, (PDVMVOLUMERECORD) (pNotify->pRecord) );
                            }
                            break;

                        case CN_KILLFOCUS:
                            // Volume container lost focus
                            //Status_Clear( hwndOwner );
                            break;

                        case CN_SETFOCUS:
                            // Volume container gained focus
                            // set the status bar for the selected record
                            break;

                        case CN_CONTEXTMENU:
                             // If a record was clicked on, select it
                            pRec = (PMINIRECORDCORE) mp2;
                            if ( pRec == NULL )
                                // Otherwise, keep the previous selection
                                pRec = WinSendDlgItemMsg( hwnd, IDD_VOLUMES, CM_QUERYRECORDEMPHASIS,
                                                          MPFROMP( CMA_FIRST ), MPFROMSHORT( CRA_SELECTED ));
                            if ( pRec )
                                WinSendDlgItemMsg( hwnd, IDD_VOLUMES, CM_SETRECORDEMPHASIS, MPFROMP( pRec ),
                                                   MPFROM2SHORT( MPFROMSHORT( TRUE ),
                                                                 MPFROMSHORT( CRA_CURSORED | CRA_SELECTED )));

                            // Open the popup menu under the mouse
                            if ( pGlobal->hwndPopupVolume ) {
                                WinQueryPointerPos( HWND_DESKTOP, &ptl );
                                WinPopupMenu( HWND_DESKTOP, hwnd, pGlobal->hwndPopupVolume, ptl.x, ptl.y, ID_VOLUME_CREATE,
                                              PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1 );
                            }
                            break;

                    }
                    break;

            }
            return (MRESULT) 0;
            // End WM_CONTROL


        case WM_PAINT:
            pCtl = WinQueryWindowPtr( hwnd, 0 );
            hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );

            // Get the current colour pres-params
            GpiCreateLogColorTable( hps, 0, LCOLF_RGB, 0, 0, NULL );
            rc = WinQueryPresParam( hwnd, PP_BACKGROUNDCOLORINDEX,
                                    PP_BACKGROUNDCOLOR, &ulID, sizeof(clrBG),
                                    &clrBG, QPF_ID1COLORINDEX );
            if ( !rc ) clrBG = SYSCLR_WINDOW;
            else if ( ulID == PP_BACKGROUNDCOLORINDEX )
                clrBG = GpiQueryRGBColor( hps, 0, clrBG );

            rc = WinQueryPresParam( hwnd, PP_FOREGROUNDCOLORINDEX,
                                    PP_FOREGROUNDCOLOR, &ulID, sizeof(clrFG),
                                    &clrFG, QPF_ID1COLORINDEX | QPF_NOINHERIT );
            if ( !rc ) clrFG = SYSCLR_WINDOWSTATICTEXT;
            else if ( ulID == PP_FOREGROUNDCOLORINDEX )
                clrFG = GpiQueryRGBColor( hps, 0, clrFG );

            WinQueryWindowRect( hwnd, &rcl );
            WinFillRect( hps, &rcl, clrBG );

            // Draw a border around the volume container
            if ( pCtl && pCtl->hwndVolumes )
            {
                WinQueryWindowRect( pCtl->hwndVolumes, &rcl );
                WinMapWindowPoints( pCtl->hwndVolumes, hwnd, (PPOINTL) &rcl, 2 );
                if ( pGlobal->fsProgram & FS_APP_PMSTYLE ) {
                    rcl.xLeft -= 2;
                    rcl.yBottom -= 2;
                    rcl.xRight +=2;
                    rcl.yTop += 2;
                    DrawNice3DBorder( hps, rcl );
                } else {
                    ptl.x = rcl.xLeft - 1;
                    ptl.y = rcl.yBottom - 1;
                    GpiMove( hps, &ptl );
                    ptl.x = rcl.xRight;
                    ptl.y = rcl.yTop;
                    GpiSetLineType( hps, LINETYPE_DEFAULT );
                    GpiSetColor( hps, 0x808080 );
                    GpiBox( hps, DRO_OUTLINE, &ptl, 0, 0 );
                }
            }
            WinEndPaint( hps );
            break;
            // End WM_PAINT


        case WM_PRESPARAMCHANGED:
            switch ( (ULONG) mp1 ) {
                case PP_BACKGROUNDCOLOR:
                case PP_BACKGROUNDCOLORINDEX:
                case PP_FOREGROUNDCOLOR:
                case PP_FOREGROUNDCOLORINDEX:
                    // Update the info panel with the new colour
                    pCtl = WinQueryWindowPtr( hwnd, 0 );
                    if ( pCtl && pCtl->hwndInfo ) {
                        rc = WinQueryPresParam( hwnd, (ULONG) mp1, 0, NULL,
                                                sizeof( clrBG ), &clrBG, 0 );
                        if ( rc )
                            WinSetPresParam( pCtl->hwndInfo, (ULONG) mp1,
                                             sizeof( clrBG ), &clrBG );
                    }
                    WinInvalidateRect( hwnd, NULL, TRUE );
                    break;

                default: break;
            }
            break;


        case WM_SIZE:
            cx = SHORT1FROMMP( mp2 );
            cy = SHORT2FROMMP( mp2 );
            cxInfo = cx / 3;
            WinSetWindowPos( WinWindowFromID( hwnd, IDD_VOLUMES ),
                             HWND_TOP, 4, 4, cx - cxInfo - 8, cy - 8, SWP_SIZE | SWP_MOVE );
            WinSetWindowPos( WinWindowFromID( hwnd, IDD_VOL_INFO ),
                             HWND_TOP, (cx - cxInfo) + 2, 1, cxInfo, cy,
                             SWP_SIZE | SWP_MOVE );
            break;


        // --------------------------------------------------------------------
        // Custom messages defined for this control
        //

        /* .................................................................. *
         * LPM_STYLECHANGED                                                   *
         *  - mp1 (USHORT) : Mask indicating which flags were changed         *
         *  - mp2          : Unused, should be 0                              *
         *  Returns 0.                                                        *
         *                                                                    *
         * Informs the control that the global style flags have changed.      *
         * mp1 simply indicates which flags have been changed;                *
         * pGlobal->fsProgram has the actual new flag values.                 *
         * .................................................................. */
        case LPM_STYLECHANGED:
            pCtl = WinQueryWindowPtr( hwnd, 0 );
            if ( !pCtl || !pCtl->hwndInfo ) return (MRESULT) 0;

            if ( pGlobal->fsProgram & FS_APP_PMSTYLE ) {
                WinSendMsg( pCtl->hwndInfo, VIM_SETSTYLE,
                            MPFROMSHORT( VIV_FS_3DSEP ), 0 );
            }
            else {
                WinSendMsg( pCtl->hwndInfo, VIM_SETSTYLE, 0, 0 );
            }

            // TODO update the size fields on both containers (DONE?)
            // TODO update the volume type terms (DONE?)

            WinInvalidateRect( hwnd, NULL, TRUE );
            return (MRESULT) 0;


        default: break;
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * PartitionContainerSetup()                                                 *
 *                                                                           *
 * This function initializes the partition container used in the volume      *
 * info panel, and creates the fields for detail view.                       *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PDVMGLOBAL pGlobal: Application global data                             *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void PartitionContainerSetup( PDVMGLOBAL pGlobal )
{
    CNRINFO         cnrinfo = {0};              // container setup structure
    FIELDINFOINSERT finsert = {0};              // field-insertion structure
    PFIELDINFO      fi,                         // pointer to current field info
                    fiAll;                      // pointer to first field info
    HWND            hwndInfoPanel,              // handle of volume-info panel
                    hwndCnr;                    // handle of container
    CHAR            szRes[ STRING_RES_MAXZ ];   // string resource buffer
    LONG            cch;


    if ( !pGlobal ) return;
    hwndInfoPanel = WinWindowFromID( pGlobal->hwndVolumes, IDD_VOL_INFO );
    hwndCnr = (HWND) WinSendMsg( hwndInfoPanel, VIM_GETCONTAINERHWND, 0, 0 );
    if ( !hwndCnr ) return;

    cch = WinLoadString( pGlobal->hab, pGlobal->hmri,
                         IDS_LABELS_PARTITIONS, STRING_RES_MAXZ, szRes );
    if ( cch && (( cnrinfo.pszCnrTitle = (PSZ) calloc( 1, cch + 2 )) != NULL ))
        strncpy( cnrinfo.pszCnrTitle, szRes, cch );
    cnrinfo.flWindowAttr = CV_DETAIL | CA_DETAILSVIEWTITLES;
                           // don't show the title
                           // | CA_CONTAINERTITLE | CA_TITLEREADONLY | CA_TITLESEPARATOR;
    cnrinfo.cyLineSpacing = 3;
    WinSendMsg( hwndCnr, CM_SETCNRINFO, MPFROMP( &cnrinfo ),
                MPFROMLONG( CMA_CNRTITLE | CMA_FLWINDOWATTR | CMA_LINESPACING ));

    // Create the detail view fields
    fiAll = (PFIELDINFO) WinSendMsg( hwndCnr, CM_ALLOCDETAILFIELDINFO,
                                     MPFROMLONG( PARTITION_CNR_FIELDS ), 0 );
    if ( !fiAll ) return;
    fi = fiAll;

    // Partition name column
    cch = WinLoadString( pGlobal->hab, pGlobal->hmri,
                         IDS_FIELD_PARTITION_NAME, STRING_RES_MAXZ, szRes );
    if ( cch && (( fi->pTitleData = (PSZ) calloc( 1, cch + 1 )) != NULL ))
        strncpy( fi->pTitleData, szRes, cch );
    fi->cb        = sizeof( FIELDINFO );
    fi->flData    = CFA_STRING | CFA_HORZSEPARATOR;
    fi->flTitle   = CFA_LEFT;
    fi->offStruct = FIELDOFFSET( DVMPARTITIONRECORD, pszName );
    fi = fi->pNextFieldInfo;

    // Size column
    cch = WinLoadString( pGlobal->hab, pGlobal->hmri,
                         IDS_FIELD_SIZE, STRING_RES_MAXZ, szRes );
    if ( cch && (( fi->pTitleData = (PSZ) calloc( 1, cch + 2 )) != NULL )) {
        if ( pGlobal->fsProgram & FS_APP_IECSIZES )
            sprintf( fi->pTitleData, szRes, "MiB");
        else
            sprintf( fi->pTitleData, szRes, "MB");
    }
    fi->cb        = sizeof( FIELDINFO );
    fi->flData    = CFA_ULONG | CFA_HORZSEPARATOR | CFA_RIGHT;
    fi->flTitle   = CFA_RIGHT;
    fi->offStruct = FIELDOFFSET( DVMPARTITIONRECORD, ulSize );
    fi = fi->pNextFieldInfo;

    // Disk column
    cch = WinLoadString( pGlobal->hab, pGlobal->hmri,
                         IDS_FIELD_DISK, STRING_RES_MAXZ, szRes );
    if ( cch && (( fi->pTitleData = (PSZ) calloc( 1, cch + 1 )) != NULL ))
        strncpy( fi->pTitleData, szRes, cch );
    fi->cb        = sizeof( FIELDINFO );
    fi->flData    = CFA_ULONG | CFA_HORZSEPARATOR;
    fi->flTitle   = CFA_LEFT;
    fi->offStruct = FIELDOFFSET( DVMPARTITIONRECORD, ulDisk );
    fi = fi->pNextFieldInfo;

    finsert.cb = (ULONG) sizeof( FIELDINFOINSERT );
    finsert.pFieldInfoOrder = (PFIELDINFO) CMA_END;
    finsert.fInvalidateFieldInfo = TRUE;
    finsert.cFieldInfoInsert = PARTITION_CNR_FIELDS;

    WinSendMsg( hwndCnr, CM_INSERTDETAILFIELDINFO,
                MPFROMP( fiAll ), MPFROMP( &finsert ));
}


/* ------------------------------------------------------------------------- *
 * PartitionContainerDestroy()                                               *
 *                                                                           *
 * This function frees all the memory previously allocated for the partition *
 * container details view.  It does not empty the container of records; that *
 * is assumed to have been done already.                                     *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PDVMGLOBAL pGlobal: Application global data                             *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void PartitionContainerDestroy( PDVMGLOBAL pGlobal )
{
    PFIELDINFO fi;
    CNRINFO    cnrinfo;
    HWND       hwndInfoPanel,           // handle of volume info panel
               hwndCnr;                 // handle of container

    if ( !pGlobal ) return;
    hwndInfoPanel = WinWindowFromID( pGlobal->hwndVolumes, IDD_VOL_INFO );
    hwndCnr = (HWND) WinSendMsg( hwndInfoPanel, VIM_GETCONTAINERHWND, 0, 0 );
    if ( !hwndCnr ) return;

    fi = WinSendMsg( hwndCnr, CM_QUERYDETAILFIELDINFO,
                     0, MPFROMSHORT( CMA_FIRST ));
    do {
        if ( fi->pTitleData )
            free ( fi->pTitleData );
        fi = fi->pNextFieldInfo;
    } while ( fi != NULL );

    WinSendMsg( hwndCnr, CM_REMOVEDETAILFIELDINFO, 0,
                MPFROM2SHORT( 0, CMA_FREE ));

    if ( WinSendMsg( hwndCnr, CM_QUERYCNRINFO, MPFROMP( &cnrinfo ),
                     MPFROMSHORT( sizeof( CNRINFO ))) != 0 )
    {
        if ( cnrinfo.pszCnrTitle ) free ( cnrinfo.pszCnrTitle );
    }
}


/* ------------------------------------------------------------------------- *
 * PartitionContainerPopulate()                                              *
 *                                                                           *
 * This function populates the partitions container on the current-volume    *
 * information panel.                                                        *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwndCnr                   : Handle of partition container control  *
 *   Partition_Information_Array pia: LVM array of partition records         *
 *   PDVMGLOBAL pGlobal             : Application global data                *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void PartitionContainerPopulate( HWND hwndCnr,
                                 Partition_Information_Array pia,
                                 PDVMGLOBAL pGlobal )
{
    Partition_Information_Record *pPartition;

    PDVMPARTITIONRECORD pRecAll,
                        pRec;
    RECORDINSERT        rins;
    ULONG               cb,
                        i, j;


    if ( !pGlobal || !pia.Count ) return;

    // Allocate the memory for records
    cb = sizeof( DVMPARTITIONRECORD ) - sizeof( MINIRECORDCORE );
    pRecAll = (PDVMPARTITIONRECORD) \
              WinSendMsg( hwndCnr, CM_ALLOCRECORD,
                          MPFROMLONG( cb ), MPFROMLONG( pia.Count ));

    // Populate the record data structures
    pRec = pRecAll;
    for ( i = 0; i < pia.Count; i++ ) {
        pPartition = pia.Partition_Array + i;

        pRec->record.pszIcon  = strdup( pPartition->Partition_Name );
        pRec->record.hptrIcon = NULLHANDLE;
        pRec->record.cb       = sizeof( MINIRECORDCORE );

        pRec->handle  = pPartition->Partition_Handle;
        pRec->hDisk   = pPartition->Drive_Handle;
        pRec->ulSize  = SECS_TO_MiB( pPartition->Usable_Partition_Size );
        pRec->pszName = pRec->record.pszIcon;
        for ( j = 0; j < pGlobal->ulDisks; j++ ) {
            if ( pGlobal->disks[ j ].handle == pPartition->Drive_Handle )
                pRec->ulDisk = pGlobal->disks[ j ].iNumber;
        }

        pRec = (PDVMPARTITIONRECORD) pRec->record.preccNextRecord;
    }

    // Insert the records
    rins.cb                = sizeof( RECORDINSERT );
    rins.pRecordOrder      = (PRECORDCORE) CMA_END;
    rins.pRecordParent     = NULL;
    rins.zOrder            = (USHORT) CMA_TOP;
    rins.fInvalidateRecord = TRUE;
    rins.cRecordsInsert    = pia.Count;

    WinSendMsg( hwndCnr, CM_INSERTRECORD,
                MPFROMP( pRecAll ), MPFROMP( &rins ));

}


/* ------------------------------------------------------------------------- *
 * PartitionContainerClear()                                                 *
 *                                                                           *
 * This function removes all records from the partitions container (and      *
 * frees any memory allocated in each record).                               *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwndCnr: Handle of partition container control               *
 *   PDVMGLOBAL pGlobal: Application global data                             *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void PartitionContainerClear( HWND hwndCnr, PDVMGLOBAL pGlobal )
{
    PDVMPARTITIONRECORD pRec;

    if ( !pGlobal ) return;

    pRec = (PDVMPARTITIONRECORD) \
               WinSendMsg( hwndCnr, CM_QUERYRECORD, NULL,
                           MPFROM2SHORT( CMA_FIRST, CMA_ITEMORDER ));

    // Free the individual record fields
    while ( pRec ) {
        free( pRec->record.pszIcon );
        pRec = (PDVMPARTITIONRECORD) pRec->record.preccNextRecord;
    }

    // Free the record structures
    WinSendMsg( hwndCnr, CM_REMOVERECORD, NULL,
                MPFROM2SHORT( 0, CMA_INVALIDATE | CMA_FREE ));

}


/* ------------------------------------------------------------------------- *
 * VolumeContainerSetup()                                                    *
 *                                                                           *
 * This function initializes the volumes list container, defining its        *
 * appearance and setting the fields for detail view.                        *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PDVMGLOBAL pGlobal: Application global data                             *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void VolumeContainerSetup( PDVMGLOBAL pGlobal )
{
    CNRINFO         cnrinfo = {0};              // container setup structure
    HWND            hwndCnr;                    // handle of container
    CHAR            szRes[ STRING_RES_MAXZ ];   // string resource buffer


    if ( !pGlobal ) return;
    hwndCnr = WinWindowFromID( pGlobal->hwndVolumes, IDD_VOLUMES );

    cnrinfo.flWindowAttr = CV_DETAIL | CA_DETAILSVIEWTITLES;
    cnrinfo.pszCnrTitle = NULL;
#if 1
    if ( WinLoadString( pGlobal->hab, pGlobal->hmri,
                        IDS_LABELS_VOLUMES, STRING_RES_MAXZ, szRes ) > 0 ) {
        cnrinfo.flWindowAttr |= CA_CONTAINERTITLE | CA_TITLEREADONLY |
                                CA_TITLESEPARATOR;
        cnrinfo.pszCnrTitle = strdup( szRes );
    }
#endif
    cnrinfo.cyLineSpacing = 4;
    WinSendMsg( hwndCnr, CM_SETCNRINFO, MPFROMP( &cnrinfo ),
                MPFROMLONG( CMA_FLWINDOWATTR | CMA_LINESPACING | CMA_CNRTITLE ));

    VolumeContainerSetupDetails( hwndCnr, pGlobal->hab, pGlobal->hmri, pGlobal->fsProgram );

}


/* ------------------------------------------------------------------------- *
 * VolumeContainerSetup()                                                    *
 *                                                                           *
 * This function initializes the volumes list container, defining its        *
 * appearance and setting the fields for detail view.                        *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND    hwndCnr  : Handle of container control                          *
 *   HAB     hab      : Anchor block handle                                  *
 *   HMODULE hmri     : Handle of language resource module                   *
 *   USHORT  fsProgram: Program preference flags                             *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void VolumeContainerSetupDetails( HWND hwndCnr, HAB hab, HMODULE hmri, USHORT fsProgram )
{
    FIELDINFOINSERT finsert = {0};              // field-insertion structure
    PFIELDINFO      fi,                         // pointer to current field info
                    fiAll;                      // pointer to first field info
    CHAR            szRes[ STRING_RES_MAXZ ];   // string resource buffer
    LONG            cch;

    // Create the detail view fields
    fiAll = (PFIELDINFO) WinSendMsg( hwndCnr, CM_ALLOCDETAILFIELDINFO,
                                     MPFROMLONG( VOLUME_CNR_FIELDS ), 0 );
    if ( !fiAll ) return;
    fi = fiAll;

    // Drive letter column
    // The title is just an empty string, but alloc'd to make cleanup simpler
    fi->pTitleData = (PSZ) calloc( 1, 1 );
    fi->cb         = sizeof( FIELDINFO );
    fi->flData     = CFA_STRING | CFA_RIGHT;
    fi->flTitle    = CFA_RIGHT;
    fi->offStruct  = FIELDOFFSET( DVMVOLUMERECORD, pszLetter );
    fi = fi->pNextFieldInfo;

    // Volume name column
    cch = WinLoadString( hab, hmri,
                         IDS_FIELD_VOLUME_NAME, STRING_RES_MAXZ, szRes );
    if ( cch && (( fi->pTitleData = (PSZ) calloc( 1, cch + 1 )) != NULL ))
        strncpy( fi->pTitleData, szRes, cch );
    fi->cb        = sizeof( FIELDINFO );
    fi->flData    = CFA_STRING | CFA_HORZSEPARATOR;
    fi->flTitle   = CFA_LEFT;
    fi->offStruct = FIELDOFFSET( DVMVOLUMERECORD, pszName );
    fi = fi->pNextFieldInfo;

    // Size column
    cch = WinLoadString( hab, hmri,
                         IDS_FIELD_SIZE, STRING_RES_MAXZ, szRes );
    if ( cch && (( fi->pTitleData = (PSZ) calloc( 1, cch + 2 )) != NULL )) {
        if ( fsProgram & FS_APP_IECSIZES )
            sprintf( fi->pTitleData, szRes, "MiB");
        else
            sprintf( fi->pTitleData, szRes, "MB");
    }
    fi->cb        = sizeof( FIELDINFO );
    fi->flData    = CFA_ULONG | CFA_HORZSEPARATOR | CFA_RIGHT;
    fi->flTitle   = CFA_RIGHT;
    fi->offStruct = FIELDOFFSET( DVMVOLUMERECORD, ulSize );
    fi = fi->pNextFieldInfo;

    // Filesystem column
    cch = WinLoadString( hab, hmri,
                         IDS_FIELD_FILESYSTEM, STRING_RES_MAXZ, szRes );
    if ( cch && (( fi->pTitleData = (PSZ) calloc( 1, cch + 1 )) != NULL ))
        strncpy( fi->pTitleData, szRes, cch );
    fi->cb        = sizeof( FIELDINFO );
    fi->flData    = CFA_STRING | CFA_HORZSEPARATOR;
    fi->flTitle   = CFA_LEFT;
    fi->offStruct = FIELDOFFSET( DVMVOLUMERECORD, pszFS );
    fi = fi->pNextFieldInfo;

    // Volume type column
    cch = WinLoadString( hab, hmri,
                         IDS_FIELD_VOLUME_TYPE, STRING_RES_MAXZ, szRes );
    if ( cch && (( fi->pTitleData = (PSZ) calloc( 1, cch + 1 )) != NULL ))
        strncpy( fi->pTitleData, szRes, cch );
    fi->cb        = sizeof( FIELDINFO );
    fi->flData    = CFA_STRING | CFA_HORZSEPARATOR;
    fi->flTitle   = CFA_LEFT;
    fi->offStruct = FIELDOFFSET( DVMVOLUMERECORD, pszType );
    fi = fi->pNextFieldInfo;

    // Flags column
    cch = WinLoadString( hab, hmri,
                         IDS_FIELD_FLAGS, STRING_RES_MAXZ, szRes );
    if ( cch && (( fi->pTitleData = (PSZ) calloc( 1, cch + 1 )) != NULL ))
        strncpy( fi->pTitleData, szRes, cch );
    fi->cb        = sizeof( FIELDINFO );
    fi->flData    = CFA_STRING | CFA_HORZSEPARATOR;
    fi->flTitle   = CFA_LEFT;
    fi->offStruct = FIELDOFFSET( DVMVOLUMERECORD, pszFlags );
    fi = fi->pNextFieldInfo;

    finsert.cb = (ULONG) sizeof( FIELDINFOINSERT );
    finsert.pFieldInfoOrder = (PFIELDINFO) CMA_END;
    finsert.fInvalidateFieldInfo = TRUE;
    finsert.cFieldInfoInsert = VOLUME_CNR_FIELDS;

    WinSendMsg( hwndCnr, CM_INSERTDETAILFIELDINFO,
                MPFROMP( fiAll ), MPFROMP( &finsert ));

}


/* ------------------------------------------------------------------------- *
 * VolumeContainerDestroy()                                                  *
 *                                                                           *
 * This function frees all the memory previously allocated for the volume    *
 * list container details view.  It does not empty the container of records; *
 * that is assumed to have been done already.                                *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PDVMGLOBAL pGlobal: Application global data                             *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void VolumeContainerDestroy( PDVMGLOBAL pGlobal )
{
    PFIELDINFO fi;
    HWND       hwndCnr;                    // handle of container

    if ( !pGlobal ) return;
    hwndCnr = WinWindowFromID( pGlobal->hwndVolumes, IDD_VOLUMES );

    fi = WinSendMsg( hwndCnr, CM_QUERYDETAILFIELDINFO,
                     0, MPFROMSHORT( CMA_FIRST ));
    do {
        if ( fi->pTitleData )
            free ( fi->pTitleData );
        fi = fi->pNextFieldInfo;
    } while ( fi != NULL );

    WinSendMsg( hwndCnr, CM_REMOVEDETAILFIELDINFO, 0,
                MPFROM2SHORT( 0, CMA_FREE ));
}


/* ------------------------------------------------------------------------- *
 * DiskListPopulate()                                                        *
 *                                                                           *
 * This function populates the disks list with our global LVM data.          *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd : Client window handle.                                       *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void DiskListPopulate( HWND hwnd )
{
    Partition_Information_Array  partitions;
    Volume_Information_Record    volume;    // Current volume information

    TOOLINFO        ttinfo = {0};   // tooltip tool info structure
    PDVCTLDATA      pDiskCtl;       // Array of disk control data structures
    PPVCTLDATA      pPartCtl;       // Array of partition control data structures
    HWND            hwndDisk,       // Handle of disk control
                    hwndPart;       // Handle of partition control
    HMODULE         hIconLib;       // Handle of DLL containing icon resources
    HPOINTER        hicon,          // Pointer to current icon
                    hptrHDD,        // Icon for normal hard disks
                    hptrPRM,        // Icon for normal PRM disks
                    hptrBad,        // Icon for inaccessible hard disks
                    hptrEmpty,      // Icon for inaccessible PRM disks
                    hptrMemdisk;    // Icon for memory disks
    PDVMGLOBAL      pGlobal;        // Global application data
    CARDINAL32      rc;             // LVM return code
    ULONG           ulCount,        // Number of disks in the list
                    cb,
                    i, j;
    USHORT          usRes;          // resource ID for string
    CHAR            szRes[ STRING_RES_MAXZ ],   // string resource buffer
                    szType[ 40 ],               // partition type string
                    szSize[ 15 ];               // partition size string


    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( !pGlobal || !pGlobal->hwndDisks ) return;

    // Find out how many disks to add
    if ( pGlobal->fsProgram & FS_APP_HIDE_FREEPRM ) {
        for ( i = 0, ulCount = 0; i < pGlobal->ulDisks; i++ ) {
            if ( pGlobal->disks[ i ].fUnusable &&
                ( pGlobal->disks[ i ].fPRM || pGlobal->disks[ i ].fBigFloppy ))
                continue;
            ulCount++;
        }
    }
    else ulCount = pGlobal->ulDisks;

    // Add the disks
    cb = sizeof( DVCTLDATA );
    pDiskCtl = (PDVCTLDATA) calloc( ulCount, cb );
    if ( !pDiskCtl ) return;
    for ( i = 0, ulCount = 0; i < pGlobal->ulDisks; i++ ) {

        // Skip past empty removable drives if the user doesn't want to see them
        if (( pGlobal->fsProgram & FS_APP_HIDE_FREEPRM ) &&
            pGlobal->disks[ i ].fUnusable &&
            ( pGlobal->disks[ i ].fPRM || pGlobal->disks[ i ].fBigFloppy ))
            continue;

        pDiskCtl[ ulCount ].cb = cb;
        pDiskCtl[ ulCount ].handle = pGlobal->disks[ i ].handle;
        pDiskCtl[ ulCount ].number = pGlobal->disks[ i ].iNumber;
        pDiskCtl[ ulCount ].ulSize = pGlobal->disks[ i ].iSize;
        strncpy( pDiskCtl[ ulCount ].szName, pGlobal->disks[ i ].szName,
                 DISK_NAME_SIZE );
        nlsThousandsULong( pDiskCtl[ ulCount ].szSize,
                           pGlobal->disks[ i ].iSize, pGlobal->ctry.cThousands );
        strncat( pDiskCtl[ ulCount ].szSize,
                 ( pGlobal->fsProgram & FS_APP_IECSIZES ) ? " MiB" : " MB",
                 SIZE_TEXT_LIMIT );
        ulCount++;
    }
    WinSendMsg( pGlobal->hwndDisks, LLM_SETDISKS,
                MPFROMLONG( ulCount ), MPFROMP( pDiskCtl ));

    // Now set the info for each disk
    rc = (APIRET) DosQueryModuleHandle("PMWP.DLL", &hIconLib );
    if ( rc == NO_ERROR ) {
        hptrHDD     = WinLoadPointer( HWND_DESKTOP, hIconLib, 13 );
        hptrPRM     = WinLoadPointer( HWND_DESKTOP, hIconLib, 95 );
        hptrEmpty   = WinLoadPointer( HWND_DESKTOP, hIconLib, 76 );
    }
    else {
        hptrHDD     = WinLoadPointer( HWND_DESKTOP, pGlobal->hmri, IDP_HDD );
        hptrPRM     = WinLoadPointer( HWND_DESKTOP, pGlobal->hmri, IDP_PRM );
        hptrEmpty   = WinLoadPointer( HWND_DESKTOP, pGlobal->hmri, IDP_PRM_MISSING );
    }
    hptrMemdisk = WinLoadPointer( HWND_DESKTOP, pGlobal->hmri, IDP_MEMDISK );
    hptrBad     = WinLoadPointer( HWND_DESKTOP, pGlobal->hmri, IDP_DISK_BAD );

    for ( i = 0, ulCount = 0; i < pGlobal->ulDisks; i++ ) {

        // Skip past empty removable drives if the user doesn't want to see them
        if (( pGlobal->fsProgram & FS_APP_HIDE_FREEPRM ) &&
            pGlobal->disks[ i ].fUnusable &&
            ( pGlobal->disks[ i ].fPRM || pGlobal->disks[ i ].fBigFloppy ))
            continue;

        hwndDisk = (HWND) WinSendMsg( pGlobal->hwndDisks, LLM_QUERYDISKHWND,
                                      MPFROMSHORT( ulCount ), MPFROMSHORT( TRUE ));
        if ( !hwndDisk ) continue;
        ulCount++;

        // Set the correct disk icon
        if (( pGlobal->disks[ i ].iSerial == SERIAL_MEMDISK ) &&
            ( !strcmp( pGlobal->disks[ i ].szName, STRING_MEMDISK )))
            hicon = hptrMemdisk;
        else if ( pGlobal->disks[ i ].fUnusable )
            hicon = ( pGlobal->disks[ i ].fPRM ) ? hptrEmpty : hptrBad;
        else
            hicon = ( pGlobal->disks[ i ].fPRM ) ? hptrPRM : hptrHDD;
        WinSendMsg( hwndDisk, LDM_SETDISKICON, MPFROMP(hicon), 0 );

        // Set the disk style
        WinSendMsg( hwndDisk, LDM_SETSTYLE,
                    MPFROMSHORT( (pGlobal->fsProgram & FS_APP_UNIFORM)? LDS_FS_UNIFORM: 0 ),
                    0 );

        if ( pGlobal->disks[ i ].fUnusable ) continue;

        // Set the partitions for the disk
        partitions = LvmGetPartitions( pGlobal->disks[ i ].handle, &rc );
        if ( rc != LVM_ENGINE_NO_ERROR ) continue;

        cb = sizeof( PVCTLDATA );
        pPartCtl = (PPVCTLDATA) calloc( partitions.Count, cb );
        if ( !pPartCtl ) continue;

        for ( j = 0; j < partitions.Count; j++ ) {
            pPartCtl[ j ].cb = cb;
            if ( partitions.Partition_Array[ j ].Partition_Type == FREE_SPACE_PARTITION )
                pPartCtl[ j ].bType = LPV_TYPE_FREE;
            else if (( partitions.Partition_Array[ j ].OS_Flag == 0x0A ) &&
                     ( partitions.Partition_Array[ j ].Partition_Status == PARTITION_IS_IN_USE ))
                pPartCtl[ j ].bType = LPV_TYPE_BOOTMGR;
            else
                pPartCtl[ j ].bType = partitions.Partition_Array[ j ].Primary_Partition ?
                                      LPV_TYPE_PRIMARY : LPV_TYPE_LOGICAL;
            pPartCtl[ j ].fInUse  = (BOOL)( partitions.Partition_Array[ j ].Partition_Status == PARTITION_IS_IN_USE );
            pPartCtl[ j ].bOS     = partitions.Partition_Array[ j ].OS_Flag;
            pPartCtl[ j ].ulSize  = SECS_TO_MiB( partitions.Partition_Array[ j ].Usable_Partition_Size );
            pPartCtl[ j ].number  = j + 1;
            pPartCtl[ j ].disk    = pGlobal->disks[ i ].iNumber;
            pPartCtl[ j ].handle  = partitions.Partition_Array[ j ].Partition_Handle;
            strncpy( pPartCtl[ j ].szName,
                     partitions.Partition_Array[ j ].Partition_Name,
                     PARTITION_NAME_SIZE );
            strncpy( pPartCtl[ j ].szFS,
                     partitions.Partition_Array[ j ].File_System_Name,
                     FILESYSTEM_NAME_SIZE );

            // Get the actual (not configured) drive letter
            pPartCtl[ j ].cLetter = '\0';
            if ( partitions.Partition_Array[ j ].Volume_Handle ) {
                volume = LvmGetVolumeInfo( partitions.Partition_Array[ j ].Volume_Handle, &rc );
                if ( rc == LVM_ENGINE_NO_ERROR ) {
                    if ( !volume.Current_Drive_Letter ) {
                        // No current drive letter, see if there's a newly-assigned one
                        if ( volume.Drive_Letter_Preference )
                            pPartCtl[ j ].cLetter = volume.Drive_Letter_Preference;
                    }
                    else
                        pPartCtl[ j ].cLetter = volume.Current_Drive_Letter;
                }
            }

        }
        WinSendMsg( hwndDisk, LDM_SETPARTITIONS,
                    MPFROMLONG( partitions.Count ), MPFROMP( pPartCtl ));

        // Now get each partition's HWND and register the tooltip for it
        WinLoadString( pGlobal->hab, pGlobal->hmri, IDS_TOOLTIP_PARTITION,
                       STRING_RES_MAXZ, szRes );
        for ( j = 0; j < partitions.Count && pGlobal->hwndTT; j++ ) {
            hwndPart = (HWND) WinSendMsg( hwndDisk, LDM_QUERYPARTITIONHWND,
                                          MPFROMLONG( j ), MPFROMSHORT( TRUE ));
            if ( !hwndPart ) continue;
            ttinfo.ulFlags = TTF_SUBCLASS | TTF_SHYMOUSE | TTF_POS_Y_ABOVE_TOOL;
            ttinfo.hwndToolOwner = hwndDisk;
            ttinfo.hwndTool = hwndPart;

            nlsThousandsULong( szSize, pPartCtl[ j ].ulSize,
                               pGlobal->ctry.cThousands );
            switch ( pPartCtl[ j ].bType ) {
                case LPV_TYPE_FREE    :
                case LPV_TYPE_UNUSABLE: usRes = IDS_TERMS_FREE;
                                        break;
                case LPV_TYPE_LOGICAL : usRes = IDS_TERMS_LOGICAL;
                                        break;
                default               : usRes = IDS_TERMS_PRIMARY;
                                        break;
            }
            WinLoadString( pGlobal->hab, pGlobal->hmri, usRes,
                           STRING_RES_MAXZ, szType );

            ttinfo.pszText = (PSZ) malloc( STRING_RES_MAXZ+1 );
            sprintf( ttinfo.pszText, szRes, pPartCtl[ j ].szName, szType, szSize,
                     ( pGlobal->fsProgram & FS_APP_IECSIZES ) ? " MiB" : " MB" );
            WinSendMsg( pGlobal->hwndTT, TTM_ADDTOOL, 0, MPFROMP( &ttinfo ));
        }

        free( pPartCtl );
        LvmFreeMem( partitions.Partition_Array );

    }

    free( pDiskCtl );

    // Set selection emphasis on the first disk control
    if ( ulCount ) {
        hwndDisk = (HWND) WinSendMsg( pGlobal->hwndDisks, LLM_QUERYDISKHWND,
                                      MPFROMSHORT( 0 ), MPFROMSHORT( TRUE ));
        WinSendMsg( pGlobal->hwndDisks, LLM_SETDISKEMPHASIS,
                    MPFROMP( hwndDisk ), MPFROM2SHORT( TRUE, LDV_FS_SELECTED ));
#if 0
        // remove the partition emphasis if the disk list doesn't have focus
        if ( WinQueryFocus( HWND_DESKTOP ) != pGlobal->hwndDisks )
            WinSendMsg( hwndDisk, LDM_SETEMPHASIS, 0,
                        MPFROM2SHORT( FALSE, LPV_FS_SELECTED ));
#endif
    }

    if ( pGlobal->hwndTT ) {
        WinSendMsg( pGlobal->hwndTT, TTM_SETDELAYTIME,
                    MPFROMLONG( TTDT_INITIAL ), MPFROMLONG( 500 ));
        WinSendMsg( pGlobal->hwndTT, TTM_SETDELAYTIME,
                    MPFROMLONG( TTDT_AUTOPOP ), MPFROMLONG( 3600000 ));
        WinSendMsg( pGlobal->hwndTT, TTM_ACTIVATE, MPFROMLONG( TRUE ), 0 );
    }
}


/* ------------------------------------------------------------------------- *
 * VolumeContainerPopulate()                                                 *
 *                                                                           *
 * This function populates the main volumes container with our global LVM    *
 * data.                                                                     *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND           hwndCnr  : Handle of container control                   *
 *   PLVMVOLUMEINFO volumes  : Array of volume information structures        *
 *   CARDINAL32     ulVolumes: Number of items in 'volumes'                  *
 *   HAB            hab      : Global HAB                                    *
 *   HMODULE        hmri     : Handle to MRI resource module                 *
 *   USHORT         fsProgram: Global program flags                          *
 *   BOOL           fLVM     : Show only LVM (advanced) volumes?             *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void VolumeContainerPopulate( HWND hwndCnr, PLVMVOLUMEINFO volumes, CARDINAL32 ulVolumes, HAB hab, HMODULE hmri, USHORT fsProgram, BOOL fLVM )
{
    PDVMVOLUMERECORD pRecAll,       // List of container records
                     pRec;          // Current container record
    RECORDINSERT     rins;          // Record insertion structure
    ULONG            ulCount,       // Current record number
                     cb,
                     i;
    CHAR             szRes[ STRING_RES_MAXZ ] = {0};  // String resource buffer


    if ( !volumes || !ulVolumes ) return;

    if ( fLVM ) {
        for ( i = 0, ulCount = 0; i < ulVolumes; i++ ) {
            if ( !volumes[ i ].fCompatibility )
                ulCount++;
        }
    }
    else

    if ( fsProgram & FS_APP_HIDE_NONLVM ) {
        for ( i = 0, ulCount = 0; i < ulVolumes; i++ ) {
            if ( volumes[ i ].bDevice <= LVM_DEVICE_PRM )
                ulCount++;
        }
    }
    else ulCount = ulVolumes;

    if ( !ulCount ) return;

    // Allocate the memory for records
    cb = sizeof( DVMVOLUMERECORD ) - sizeof( MINIRECORDCORE );
    pRecAll = (PDVMVOLUMERECORD) WinSendMsg( hwndCnr, CM_ALLOCRECORD,
                                             MPFROMLONG( cb ), MPFROMLONG( ulCount ));

    // Populate the record data structures

    pRec = pRecAll;
    for ( i = 0; i < ulVolumes; i++ ) {
        // Skip over non-advanced volumes if requested by the caller
        if ( fLVM && volumes[ i ].fCompatibility ) continue;

        // Skip over non-managed volumes if the user doesn't want to see them
        if (( fsProgram & FS_APP_HIDE_NONLVM ) &&
            ( volumes[ i ].bDevice > LVM_DEVICE_PRM ))
            continue;

        pRec->record.pszIcon  = strdup( volumes[ i ].szName );
        pRec->record.hptrIcon = NULLHANDLE;
        pRec->record.cb       = sizeof( MINIRECORDCORE );

        pRec->handle    = volumes[ i ].handle;
        pRec->ulVolume  = i;
        pRec->ulSize    = volumes[ i ].iSize;
        pRec->pszName   = pRec->record.pszIcon;
        pRec->pszFS     = strdup( volumes[ i ].szFS );

        // Volume type field
        if ( volumes[ i ].fCompatibility ) {
            WinLoadString( hab, hmri,
                           ( fsProgram & FS_APP_IBMTERMS ) ?
                           IDS_TERMS_COMPATIBILITY : IDS_TERMS_STANDARD,
                           STRING_RES_MAXZ, szRes );
        }
        else {
            WinLoadString( hab, hmri,
                           ( fsProgram & FS_APP_IBMTERMS ) ?
                           IDS_TERMS_LVM : IDS_TERMS_ADVANCED,
                           STRING_RES_MAXZ, szRes );
        }
        pRec->pszType = strdup( szRes );

        // Drive letter field
        if ( !volumes[ i ].cLetter ) {
            // Current drive letter is null, could be for a couple of reasons...

            if ( !volumes[ i ].cPreference ) {
                // No drive letter assigned
                pRec->pszLetter = strdup(" ");
            }
            else {
                // Drive letter changed during this session
                WinLoadString( hab, hmri,
                               IDS_LETTER_CHANGED, STRING_RES_MAXZ, szRes );
                pRec->pszLetter = (PSZ) malloc( strlen( szRes ) + 1 );
                sprintf( pRec->pszLetter, szRes,
                         volumes[ i ].cInitial ?
                             volumes[ i ].cInitial : '*',
                         volumes[ i ].cPreference );
            }

        }
        else {
            // See if the current drive letter is the same as the configured one

            if ( volumes[ i ].cPreference &&
                 ( volumes[ i ].cPreference != volumes[ i ].cLetter ))
            {
                // Volume did not get its preferred drive letter at boot
                WinLoadString( hab, hmri,
                               IDS_LETTER_CHANGED, STRING_RES_MAXZ, szRes );
                pRec->pszLetter = (PSZ) malloc( strlen( szRes ) + 1 );
                sprintf( pRec->pszLetter, szRes,
                         volumes[ i ].cPreference,
                         volumes[ i ].cLetter );
            } else {
                // Drive has had its preferred letter since bootup
                pRec->pszLetter = (PSZ) malloc( 3 );
                sprintf( pRec->pszLetter, "%c:", volumes[ i ].cLetter );
            }

        }

        // Flags field
        if ( volumes[ i ].fCompatibility ) {
            switch ( volumes[ i ].iStatus ) {

                default:
                case LVM_VOLUME_STATUS_NONE:
                    pRec->pszFlags = strdup(" ");
                    break;

                case LVM_VOLUME_STATUS_BOOTABLE:
                    WinLoadString( hab, hmri,
                                   IDS_STATUS_BOOTABLE, STRING_RES_MAXZ, szRes );
                    pRec->pszFlags = strdup( szRes );
                    break;

                case LVM_VOLUME_STATUS_STARTABLE:
                    WinLoadString( hab, hmri,
                                   IDS_STATUS_STARTABLE, STRING_RES_MAXZ, szRes );
                    pRec->pszFlags = strdup( szRes );
                    break;

                case LVM_VOLUME_STATUS_INSTALLABLE:
                    WinLoadString( hab, hmri,
                                   IDS_STATUS_INSTALLABLE, STRING_RES_MAXZ, szRes );
                    pRec->pszFlags = strdup( szRes );
                    break;
            }
        }
        else if ( volumes[ i ].iPartitions > 1 ) {
            WinLoadString( hab, hmri,
                           IDS_STATUS_SPANNED, STRING_RES_MAXZ, szRes );
            pRec->pszFlags = strdup( szRes );
        }
        pRec = (PDVMVOLUMERECORD) pRec->record.preccNextRecord;
    }

    // Insert the records
    rins.cb                = sizeof( RECORDINSERT );
    rins.pRecordOrder      = (PRECORDCORE) CMA_END;
    rins.pRecordParent     = NULL;
    rins.zOrder            = (USHORT) CMA_TOP;
    rins.fInvalidateRecord = TRUE;
    rins.cRecordsInsert    = ulCount;

    WinSendMsg( hwndCnr, CM_INSERTRECORD, MPFROMP( pRecAll ), MPFROMP( &rins ));

}


/* ------------------------------------------------------------------------- *
 * DiskListClear()                                                           *
 *                                                                           *
 * This function de-allocates and removes all disks from the disk list, and  *
 * deletes all associated tooltip data.                                      *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PDVMGLOBAL pGlobal: Application global data                             *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void DiskListClear( PDVMGLOBAL pGlobal )
{
    USHORT   usTools, i;
    TOOLINFO ttinfo = {0};

    // Free the tooltip data
    usTools = (USHORT) WinSendMsg( pGlobal->hwndTT, TTM_GETTOOLCOUNT, 0, 0 );
    for ( i = 0; i < usTools; i++ ) {
        WinSendMsg( pGlobal->hwndTT, TTM_ENUMTOOLS,
                    MPFROMSHORT( i ), MPFROMP( &ttinfo ));
        if ( ttinfo.pszText ) free( ttinfo.pszText );
        ttinfo.pszText = NULL;
        WinSendMsg( pGlobal->hwndTT, TTM_DELTOOL, 0, MPFROMP( &ttinfo ));
    }

    // Remove all disks (this automatically frees the associated memory)
    WinSendMsg( pGlobal->hwndDisks, LLM_SETDISKS, 0, 0 );
}


/* ------------------------------------------------------------------------- *
 * VolumeContainerClear()                                                    *
 *                                                                           *
 * This function removes all records from the main volumes container (and    *
 * frees any memory allocated in each record).                               *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PDVMGLOBAL pGlobal: Application global data                             *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void VolumeContainerClear( PDVMGLOBAL pGlobal )
{
    PDVMVOLUMERECORD pRec;

    if ( !pGlobal ) return;

    pRec = (PDVMVOLUMERECORD) \
        WinSendDlgItemMsg( pGlobal->hwndVolumes, IDD_VOLUMES, CM_QUERYRECORD,
                           NULL, MPFROM2SHORT( CMA_FIRST, CMA_ITEMORDER ));

    // Free the individual record fields
    while ( pRec ) {
        free( pRec->record.pszIcon );
        free( pRec->pszFS );
        free( pRec->pszType );
        free( pRec->pszLetter );
        free( pRec->pszFlags );
        pRec = (PDVMVOLUMERECORD) pRec->record.preccNextRecord;
    }

    // Free the record structures
    WinSendDlgItemMsg( pGlobal->hwndVolumes, IDD_VOLUMES, CM_REMOVERECORD,
                       NULL, MPFROM2SHORT( 0, CMA_INVALIDATE | CMA_FREE ));

}


/* ------------------------------------------------------------------------- *
 * DiskListSelect()                                                          *
 *                                                                           *
 * Handles selection events of disks in the disks list.  (Selection of       *
 * individual partitions, which could be triggered at the same time, are     *
 * handled in DiskListPartitionSelect.)                                      *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd     : Client window handle                                    *
 *   USHORT usDisk : Array index number of the disk                          *
 *   BOOL bSelected: TRUE if a disk is selected, FALSE otherwise             *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void DiskListSelect( HWND hwnd, USHORT usDisk, BOOL bSelected )
{
    PDVMGLOBAL  pGlobal;

    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( bSelected && ( pGlobal->disks[ usDisk ].fUnusable ))
        bSelected = FALSE;

    MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, ID_LVM_DISK, bSelected? TRUE: FALSE );
}


/* ------------------------------------------------------------------------- *
 * DiskListPartitionSelect()                                                 *
 *                                                                           *
 * Handles selection events of partitions in the disks list.                 *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND        hwnd   : Client window handle                               *
 *   PDISKNOTIFY pNotify: Disk list notification structure                   *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void DiskListPartitionSelect( HWND hwnd, HWND hwndPartition )
{
    Partition_Information_Record pir;
    PDVMGLOBAL       pGlobal;
    PDVMVOLUMERECORD pVolRec;
    PVCTLDATA  pvd  = {0};
    WNDPARAMS  wndp = {0};
    CARDINAL32 iErr;
    BOOL       fFreeSpace,
               fActive,
               fBootable,
               fCanBeBootable;

    if ( !hwndPartition ) return;
    pGlobal = WinQueryWindowPtr( hwnd, 0 );

    wndp.fsStatus  = WPM_CTLDATA;
    wndp.cbCtlData = sizeof( PVCTLDATA );
    wndp.pCtlData  = &pvd;
    if ( ! WinSendMsg( hwndPartition, WM_QUERYWINDOWPARAMS, MPFROMP( &wndp ), MPVOID ))
        return;

    Status_Partition( hwnd, &pvd );

    fFreeSpace = (BOOL)( pvd.bType == LPV_TYPE_FREE );
    fCanBeBootable = FALSE;

    pir = LvmGetPartitionInfo( pvd.handle, &iErr );
    fActive = (( iErr == LVM_ENGINE_NO_ERROR ) && pir.Primary_Partition ) ?
                  (BOOL)( pir.Active_Flag == ACTIVE_PARTITION ) :
                  FALSE;
    if (( iErr == LVM_ENGINE_NO_ERROR ) && ( pGlobal->fsEngine & FS_ENGINE_BOOTMGR )) {
        fBootable = (BOOL)(pir.On_Boot_Manager_Menu);
        if ( pir.Partition_Status == PARTITION_IS_AVAILABLE )
            fCanBeBootable = TRUE;
    }
    else
        fBootable = FALSE;

    MenuItemEnable( pGlobal->hwndMenu, pGlobal->hwndPopupPartition,
                    ID_PARTITION_CREATE, fFreeSpace? TRUE: FALSE );
    MenuItemEnable( pGlobal->hwndMenu, pGlobal->hwndPopupPartition,
                    ID_PARTITION_DELETE, fFreeSpace? FALSE: TRUE );
    MenuItemEnable( pGlobal->hwndMenu, pGlobal->hwndPopupPartition,
                    ID_PARTITION_RENAME, fFreeSpace? FALSE: TRUE );
    MenuItemEnable( pGlobal->hwndMenu, pGlobal->hwndPopupPartition,
                    ID_PARTITION_CONVERT, (pvd.fInUse || fFreeSpace)? FALSE: TRUE );
    MenuItemEnable( pGlobal->hwndMenu, pGlobal->hwndPopupPartition,
                    ID_PARTITION_ADD, pvd.fInUse? FALSE: TRUE );

    MenuItemEnable( pGlobal->hwndMenu, pGlobal->hwndPopupPartition,
                    ID_PARTITION_BOOTABLE, fCanBeBootable );
    WinCheckMenuItem( pGlobal->hwndMenu, ID_PARTITION_BOOTABLE, fBootable );
    WinCheckMenuItem( pGlobal->hwndPopupPartition, ID_PARTITION_BOOTABLE, fBootable );

    MenuItemEnable( pGlobal->hwndMenu, pGlobal->hwndPopupPartition,
                    ID_PARTITION_ACTIVE, (pvd.bType == LPV_TYPE_PRIMARY)? TRUE: FALSE );
    WinCheckMenuItem( pGlobal->hwndMenu, ID_PARTITION_ACTIVE, fActive );
    WinCheckMenuItem( pGlobal->hwndPopupPartition, ID_PARTITION_ACTIVE, fActive );

    if (( iErr == LVM_ENGINE_NO_ERROR ) &&
        ( pGlobal->fsProgram & FS_APP_AUTOSELECT ) && pir.Volume_Handle )
    {
        pVolRec = (PDVMVOLUMERECORD) WinSendDlgItemMsg( pGlobal->hwndVolumes,
                                                        IDD_VOLUMES,
                                                        CM_QUERYRECORD, NULL,
                                                        MPFROM2SHORT( CMA_FIRST,
                                                                      CMA_ITEMORDER ));
        while ( pVolRec ) {
            if ( pVolRec->handle == pir.Volume_Handle ) {
                if ( !( pVolRec->record.flRecordAttr & CRA_SELECTED )) {
                    WinSendDlgItemMsg( pGlobal->hwndVolumes, IDD_VOLUMES,
                                       CM_SETRECORDEMPHASIS, MPFROMP( pVolRec ),
                                       MPFROM2SHORT( TRUE, CRA_SELECTED ));
                    VolumeContainerSelect( hwnd, pGlobal->hwndPopupVolume, pVolRec );
                }
                break;
            }
            pVolRec = (PDVMVOLUMERECORD) pVolRec->record.preccNextRecord;
        }
    }

}


/* ------------------------------------------------------------------------- *
 * VolumeContainerSelect()                                                   *
 *                                                                           *
 * Handles selection (highlighting) of a record in the volumes container.    *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd            : Main program client window handle                *
 *   HWND hwndContext     : Handle of container context menu (may be NULL)   *
 *   PDVMVOLUMERECORD pRec: Selected record handle                           *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void VolumeContainerSelect( HWND hwnd, HWND hwndContext, PDVMVOLUMERECORD pRec )
{
    Partition_Information_Array pia;    // array of the volume's partitions

    PDVMGLOBAL     pGlobal;             // global application data
    PLVMVOLUMEINFO pVolume;             // pointer to the current volume info
    VIVOLUMEINFO   vi = {0};            // control data for the info panel
    HWND           hwndInfoPanel,       // handle of the volume info panel
                   hwndPartCnr,         // handle of the partition container
                   hwndDisk,            // handle of the current disk control
                   hwndPart;            // handle of the current partition control
    CARDINAL32     rc;                  // LVM API return code
    BOOL           fLVM,
                   fCheck;
    ULONG          ulID,                // string resource ID
                   i;                   // loop index
    USHORT         fsEmphasis;          // partition emphasis mask
    CHAR           szRes1[ STRING_RES_MAXZ ] = {0},  // string resource buffers
                   szRes2[ STRING_RES_MAXZ ] = {0},
                   szRes3[ STRING_RES_MAXZ ] = {0};
#ifndef VICTLDATA_STRINGS
    CHAR           szRes4[ STRING_RES_MAXZ ] = {0},
                   szRes5[ STRING_RES_MAXZ ] = {0},
                   szRes6[ STRING_RES_MAXZ ] = {0};
#endif


    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( !pGlobal ) return;

    /* Update the fields in the volume information panel
     */
    pVolume = pGlobal->volumes + pRec->ulVolume;

    // Name & drive letter
    vi.pszName   = pVolume->szName;
    vi.pszLetter = pRec->pszLetter;

    // Icon and volume type
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_VOLUME_TYPE_LONG, STRING_RES_MAXZ, szRes1 );
    if ( pVolume->fCompatibility ) {
        switch ( pVolume->bDevice ) {
            default:                  vi.hIcon = pGlobal->hptrStandard; break;
            case LVM_DEVICE_CDROM:    vi.hIcon = pGlobal->hptrCD;       break;
            case LVM_DEVICE_NETWORK:  vi.hIcon = pGlobal->hptrLAN;      break;
            case LVM_DEVICE_UNKNOWN:
                if ( strcmp( pRec->pszFS, "NDFS32") == 0 )
                    vi.hIcon = pGlobal->hptrLAN;
                else
                    vi.hIcon = pGlobal->hptrUnknown;
                break;
        }
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       ( pGlobal->fsProgram & FS_APP_IBMTERMS ) ?
                         IDS_TERMS_COMPATIBILITY : IDS_TERMS_STANDARD,
                       STRING_RES_MAXZ, szRes2 );
    }
    else {
        vi.hIcon = pGlobal->hptrAdvanced;
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       ( pGlobal->fsProgram & FS_APP_IBMTERMS ) ?
                          IDS_TERMS_LVM : IDS_TERMS_ADVANCED,
                       STRING_RES_MAXZ, szRes2 );
    }
    sprintf( szRes3, szRes1, szRes2 );
    vi.pszType = szRes3;

    // Device type
    switch ( pVolume->bDevice ) {
        case LVM_DEVICE_HDD:     ulID = IDS_DEVICE_HDD;     break;
        case LVM_DEVICE_PRM:     ulID = IDS_DEVICE_PRM;     break;
        case LVM_DEVICE_CDROM:   ulID = IDS_DEVICE_CD;      break;
        case LVM_DEVICE_NETWORK: ulID = IDS_DEVICE_LAN;     break;
        default:
        case LVM_DEVICE_UNKNOWN: ulID = IDS_DEVICE_UNKNOWN; break;
    }
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   ulID, STRING_RES_MAXZ, szRes2 );
    vi.pszDevice = szRes2;

    // Filesystem and size
    vi.pszFS = pRec->pszFS;
    nlsThousandsULong( szRes1, pVolume->iSize, pGlobal->ctry.cThousands );
    strncat( szRes1,
             ( pGlobal->fsProgram & FS_APP_IECSIZES ) ? " MiB" : " MB",
             STRING_RES_MAXZ - 1);
    vi.pszSize = szRes1;

#ifndef VICTLDATA_STRINGS
    WinLoadString( pGlobal->hab, pGlobal->hmri, IDS_LABELS_FILESYSTEM,
                   STRING_RES_MAXZ, szRes4 );
    WinLoadString( pGlobal->hab, pGlobal->hmri, IDS_LABELS_SIZE,
                   STRING_RES_MAXZ, szRes5 );
    WinLoadString( pGlobal->hab, pGlobal->hmri, IDS_LABELS_DEVICE,
                   STRING_RES_MAXZ, szRes6 );
    vi.pszFSL     = szRes4;
    vi.pszSizeL   = szRes5;
    vi.pszDeviceL = szRes6;
#endif

    // Update the panel
    hwndInfoPanel = WinWindowFromID( pGlobal->hwndVolumes, IDD_VOL_INFO );
    WinSendMsg( hwndInfoPanel, VIM_SETINFO, MPFROMP( &vi ), 0 );

    /* Clear active emphasis for all partitions in the disk list
     */
    for ( i = 0; i < pGlobal->ulDisks; i++ ) {
        hwndDisk = (HWND)  WinSendMsg( pGlobal->hwndDisks, LLM_QUERYDISKHWND,
                                       MPFROMLONG( i ), MPFROMSHORT( TRUE ));

        WinSendMsg( hwndDisk, LDM_SETEMPHASIS, 0,
                    MPFROM2SHORT( FALSE, LPV_FS_ACTIVE ));
    }

    /* Now show the current volume's partitions
     */
    hwndPartCnr = (HWND) WinSendMsg( hwndInfoPanel, VIM_GETCONTAINERHWND, 0, 0 );
    PartitionContainerClear( hwndPartCnr, pGlobal );

    pia = LvmGetPartitions( pVolume->handle, &rc );
    if ( rc != LVM_ENGINE_NO_ERROR ) return;

    WinShowWindow( hwndPartCnr, ( pia.Count ? TRUE: FALSE ));

    // Populate the partition container
    if ( pia.Count )
        PartitionContainerPopulate( hwndPartCnr, pia, pGlobal );

    // Highlight the corresponding partitions in the physical view
    for ( i = 0; i < pia.Count; i++ ) {
        hwndPart = (HWND) \
            WinSendMsg( pGlobal->hwndDisks, LLM_GETPARTITION,
                        MPFROMLONG( pia.Partition_Array[i].Partition_Handle ),
                        MPFROMLONG( pia.Partition_Array[i].Drive_Handle ));
        if ( hwndPart ) {
            if (( pGlobal->fsProgram & FS_APP_AUTOSELECT ) && ( i == 0 )) {
                fsEmphasis = (USHORT) WinSendMsg( hwndPart, LPM_GETEMPHASIS, 0, 0 );
                if ( !( fsEmphasis & LPV_FS_SELECTED ))
                    WinSendMsg( hwndPart, LPM_SETEMPHASIS,
                                MPFROMSHORT( TRUE ),
                                MPFROMSHORT( LPV_FS_SELECTED ));
            }
            WinSendMsg( hwndPart, LPM_SETEMPHASIS,
                        MPFROMSHORT( TRUE ), MPFROMSHORT( LPV_FS_ACTIVE ));
        }
    }

    // Now update the allowable menu actions

    // Enable the volume menu items for LVM-managed volumes
    fLVM = (BOOL)( pVolume->bDevice <= LVM_DEVICE_PRM );
    MenuItemEnable( pGlobal->hwndMenu, hwndContext, ID_VOLUME_DELETE, fLVM );
    MenuItemEnable( pGlobal->hwndMenu, hwndContext, ID_VOLUME_RENAME, fLVM );
    MenuItemEnable( pGlobal->hwndMenu, hwndContext, ID_VOLUME_LETTER, fLVM );

    // The 'Bootable' option is only meaningful if Boot Manager is installed
    if ( pVolume->fCompatibility && ( pGlobal->fsEngine & FS_ENGINE_BOOTMGR )) {
        MenuItemEnable( pGlobal->hwndMenu, hwndContext, ID_VOLUME_BOOTABLE, TRUE );
        fCheck = (BOOL)( pVolume->iStatus == LVM_VOLUME_STATUS_BOOTABLE );
    }
    else {
        MenuItemEnable( pGlobal->hwndMenu, hwndContext, ID_VOLUME_BOOTABLE, FALSE );
        fCheck = FALSE;
    }
    WinCheckMenuItem( pGlobal->hwndMenu, ID_VOLUME_BOOTABLE, fCheck );
    if ( hwndContext )
        WinCheckMenuItem( hwndContext, ID_VOLUME_BOOTABLE, fCheck );

    // Only compatibility volumes consisting of a primary partition can be Startable
    if ( pVolume->fCompatibility && pia.Count && ( pia.Partition_Array[ 0 ].Primary_Partition )) {
        MenuItemEnable( pGlobal->hwndMenu, hwndContext, ID_VOLUME_STARTABLE, TRUE );
        fCheck = (BOOL)( pVolume->iStatus == LVM_VOLUME_STATUS_STARTABLE );
    }
    else {
        MenuItemEnable( pGlobal->hwndMenu, hwndContext, ID_VOLUME_STARTABLE, FALSE );
        fCheck = FALSE;
    }
    WinCheckMenuItem( pGlobal->hwndMenu, ID_VOLUME_STARTABLE, fCheck );
    if ( hwndContext )
        WinCheckMenuItem( hwndContext, ID_VOLUME_STARTABLE, fCheck );

//cleanup:
    LvmFreeMem( pia.Partition_Array );
}


/* ------------------------------------------------------------------------- *
 * Status_Clear()                                                            *
 *                                                                           *
 * Clears the main window status-bar text.  (This does not include the       *
 * modified indicator in the right status area.)                             *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd            : Client window handle                             *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void Status_Clear( HWND hwnd )
{
    WinSetDlgItemText( hwnd, IDD_STATUS_SELECTED, "");
    WinSetDlgItemText( hwnd, IDD_STATUS_SIZE,     "");
    WinSetDlgItemText( hwnd, IDD_STATUS_TYPE,     "");
    WinSetDlgItemText( hwnd, IDD_STATUS_FLAGS,    "");
}


/* ------------------------------------------------------------------------- *
 * Status_ShowDisk()                                                         *
 *                                                                           *
 * Handles updating the main-window status bar when the disk list receives   *
 * focus.                                                                    *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd    : Client window handle                                     *
 *   HWND hwndDisk: Handle of selected disk control                          *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void Status_ShowDisk( HWND hwnd, HWND hwndDisk )
{
    PVCTLDATA pvd  = {0};
    WNDPARAMS wndp = {0};
    HWND      hwndPart;

    // If a partition is selected, show its status
    hwndPart = (HWND) WinSendMsg( hwndDisk, LDM_QUERYPARTITIONEMPHASIS,
                                  0, MPFROMSHORT( LPV_FS_SELECTED ));
    if ( hwndPart ) {
        wndp.fsStatus  = WPM_CTLDATA;
        wndp.cbCtlData = sizeof( PVCTLDATA );
        wndp.pCtlData  = &pvd;
        if ( WinSendMsg( hwndPart, WM_QUERYWINDOWPARAMS, MPFROMP( &wndp ), MPVOID ))
            Status_Partition( hwnd, &pvd );
    }

    // Otherwise do nothing
}


/* ------------------------------------------------------------------------- *
 * Status_Volume()                                                           *
 *                                                                           *
 * Sets the main window status-bar text for the currently-selected volume.   *
 * NOTE: We don't currently use this; at the moment, the status bar is only  *
 * used to show the selected partition, if any.  The volume detail panel is  *
 * sufficient for selected-volume information.                               *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd            : Client window handle                             *
 *   PDVMVOLUMERECORD pRec: Currently-selected volume record                 *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void Status_Volume( HWND hwnd, PDVMVOLUMERECORD pRec )
{
    PDVMGLOBAL     pGlobal;                         // global application data
    PLVMVOLUMEINFO pVolume;                         // current volume info
    ULONG          ulID;                            // string resource ID
    CHAR           szRes1[ STRING_RES_MAXZ ] = {0}, // string resource buffers
                   szRes2[ STRING_RES_MAXZ ] = {0},
                   szRes3[ STRING_RES_MAXZ ] = {0};

    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( !pGlobal ) return;

    pVolume = pGlobal->volumes + pRec->ulVolume;

    /* Set the status bar text (at the bottom of the main window)
     */

    // Status 1: selected object (volume)
    if ( pVolume->cLetter || ( pVolume->cInitial && pVolume->cPreference )) {
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_STATUS_VOLUME_LETTER, STRING_RES_MAXZ, szRes1 );
        sprintf( szRes2, szRes1, pVolume->szName,
                 pVolume->cLetter ? pVolume->cLetter : pVolume->cPreference );
    }
    else {
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_STATUS_VOLUME_HIDDEN, STRING_RES_MAXZ, szRes1 );
        sprintf( szRes2, szRes1, pVolume->szName );
    }
    WinSetDlgItemText( hwnd, IDD_STATUS_SELECTED, szRes2 );

    // Status 2: size
    nlsThousandsULong( szRes1, pVolume->iSize, pGlobal->ctry.cThousands );
    strncat( szRes1, ( pGlobal->fsProgram & FS_APP_IECSIZES ) ? " MiB" : " MB",
             STRING_RES_MAXZ-1 );
    WinSetDlgItemText( hwnd, IDD_STATUS_SIZE, szRes1 );

    // Status 3: type
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_STATUS_TYPE, STRING_RES_MAXZ, szRes1 );
    if ( pVolume->fCompatibility )
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       ( pGlobal->fsProgram & FS_APP_IBMTERMS ) ?
                         IDS_TERMS_COMPATIBILITY : IDS_TERMS_STANDARD,
                       STRING_RES_MAXZ, szRes2 );
    else
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       ( pGlobal->fsProgram & FS_APP_IBMTERMS ) ?
                         IDS_TERMS_LVM : IDS_TERMS_ADVANCED,
                       STRING_RES_MAXZ, szRes2 );
    sprintf( szRes3, szRes1, szRes2 );
    WinSetDlgItemText( hwnd, IDD_STATUS_TYPE, szRes3 );

    // Status 4: flags
    switch ( pVolume->iStatus ) {
        default:
        case LVM_VOLUME_STATUS_NONE:
            if ( pVolume->iPartitions > 1 )
                ulID = IDS_STATUS_SPANNED;
            else
                ulID = 0;
            break;
        case LVM_VOLUME_STATUS_BOOTABLE:
            ulID = IDS_STATUS_BOOTABLE;
            break;
        case LVM_VOLUME_STATUS_STARTABLE:
            ulID = IDS_STATUS_STARTABLE;
            break;
        case LVM_VOLUME_STATUS_INSTALLABLE:
            ulID = IDS_STATUS_INSTALLABLE;
            break;
    }

    if ( ulID ) {
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       ulID, STRING_RES_MAXZ, szRes1 );
        WinSetDlgItemText( hwnd, IDD_STATUS_FLAGS, szRes1 );
    }
    else
        WinSetDlgItemText( hwnd, IDD_STATUS_FLAGS, "");

}


/* ------------------------------------------------------------------------- *
 * Status_Partition()                                                        *
 *                                                                           *
 * Sets the main window status-bar text for the currently-selected partition.*
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwnd : Client window handle                                  *
 *   PPVCTLDATA pPart: Selected partition's ctldata                          *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void Status_Partition( HWND hwnd, PPVCTLDATA pPart )
{
    PDVMGLOBAL pGlobal;                             // global application data
    ULONG      ulID;                                // string resource ID
    CHAR       szRes1[ STRING_RES_MAXZ ] = {0},     // string resource buffers
               szRes2[ STRING_RES_MAXZ ] = {0},
               szRes3[ STRING_RES_MAXZ ] = {0};


    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( !pGlobal ) return;

    // Status 1: selected object (partition)

    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_STATUS_PARTITION, STRING_RES_MAXZ, szRes1 );

    switch ( pPart->bType ) {
        case LPV_TYPE_UNUSABLE:
        case LPV_TYPE_FREE:    ulID = IDS_STATUS_FREESPACE; break;
        case LPV_TYPE_LOGICAL: ulID = IDS_TERMS_LOGICAL;    break;
        default:
        case LPV_TYPE_PRIMARY: ulID = IDS_TERMS_PRIMARY;    break;
    }
    WinLoadString( pGlobal->hab, pGlobal->hmri, ulID, STRING_RES_MAXZ, szRes2 );
    sprintf( szRes3, szRes1,
             pPart->disk, pPart->number, szRes2, pPart->szName );
    WinSetDlgItemText( hwnd, IDD_STATUS_SELECTED, szRes3 );


    // Status 2: size

    nlsThousandsULong( szRes1, pPart->ulSize, pGlobal->ctry.cThousands );
    strncat( szRes1, ( pGlobal->fsProgram & FS_APP_IECSIZES ) ? " MiB" : " MB",
             STRING_RES_MAXZ-1 );
    WinSetDlgItemText( hwnd, IDD_STATUS_SIZE, szRes1 );


    // Status 3: type (OS flag)
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_STATUS_FORMAT, STRING_RES_MAXZ, szRes1 );
    sprintf( szRes2, "%02X", pPart->bOS );
    sprintf( szRes3, szRes1, szRes2 );
    if ( strlen( pPart->szFS )) {
        sprintf( szRes1, "  -  %s", pPart->szFS );
        strncat( szRes3, szRes1, STRING_RES_MAXZ-1 );
    }
    WinSetDlgItemText( hwnd, IDD_STATUS_TYPE, szRes3 );


    // Status 4: flags (available/in use)
    if ( pPart->bType == LPV_TYPE_FREE )
        ulID = 0;
    else if ( pPart->fInUse )
        ulID = IDS_STATUS_INUSE;
    else
        ulID = IDS_STATUS_AVAILABLE;
    if ( ulID ) {
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       ulID, STRING_RES_MAXZ, szRes1 );
        WinSetDlgItemText( hwnd, IDD_STATUS_FLAGS, szRes1 );
    }
    else
        WinSetDlgItemText( hwnd, IDD_STATUS_FLAGS, "");

}


/* ------------------------------------------------------------------------- *
 * ChangeSizeDisplay()                                                       *
 *                                                                           *
 * Change the abbreviation for binary megabytes (MB vs MiB) throughout the   *
 * UI, according to the configured preference.                               *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd         : Client window handle                                *
 *   PDVMGLOBAL pGlobal: Application global data (includes setting flag)     *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void ChangeSizeDisplay( HWND hwnd, PDVMGLOBAL pGlobal )
{
    PDVMVOLUMERECORD pRec;                      // selected volume record
    WNDPARAMS wp,                               // disk window parameters
              wpp;                              // partition window parameters
    DVCTLDATA dvdata;                           // disk-view control data
    PVCTLDATA pvdata;                           // disk-view control data
    TOOLINFO  ttinfo;                           // tooltip tool information
    HWND      hwndCnr,                          // handle of a container control
              hwndDisk;                         // handle of a disk-view control
    CHAR      szRes1[ STRING_RES_MAXZ ] = {0},  // string resource buffers
              szRes2[ STRING_RES_MAXZ ] = {0},
              szType[ 40 ],                     // partition type string
              szSize[ 15 ];                     // partition size string
    USHORT    usCount,                          // number of disks in the list
              usParts,                          // number of partitions on disk
              usRes,                            // string resource number
              i;


    if ( WinLoadString( pGlobal->hab, pGlobal->hmri,
                        IDS_FIELD_SIZE, STRING_RES_MAXZ, szRes1 ) > 0 )
    {
        // Change the size column title in both containers
        sprintf( szRes2, szRes1,
                 ( pGlobal->fsProgram & FS_APP_IECSIZES ) ? "MiB": "MB");
        hwndCnr = WinWindowFromID( pGlobal->hwndVolumes, IDD_VOLUMES ),
        SetContainerFieldTitle( hwndCnr, FIELDOFFSET( DVMVOLUMERECORD, ulSize ),
                                szRes2 );

        hwndCnr = (HWND) WinSendMsg( WinWindowFromID( pGlobal->hwndVolumes,
                                                      IDD_VOL_INFO ),
                                     VIM_GETCONTAINERHWND, 0, 0 );
        if ( hwndCnr ) {
            SetContainerFieldTitle( hwndCnr,
                                    FIELDOFFSET( DVMPARTITIONRECORD, ulSize ),
                                    szRes2 );
        }
    }

    // Change the size text of all disks in the disk list
    usCount = (USHORT) WinSendMsg( pGlobal->hwndDisks, LLM_QUERYDISKS, 0, 0 );
    for ( i = 0; i < usCount; i++ ) {
        hwndDisk = (HWND) WinSendMsg( pGlobal->hwndDisks, LLM_QUERYDISKHWND,
                                      MPFROMP( i ), MPFROMSHORT( TRUE ));
        wp.fsStatus     = WPM_CTLDATA;
        wp.cchText      = 0;
        wp.pszText      = NULL;
        wp.cbPresParams = 0;
        wp.pPresParams  = NULL;
        wp.cbCtlData    = sizeof( DVCTLDATA );
        wp.pCtlData     = &dvdata;
        WinSendMsg( hwndDisk, WM_QUERYWINDOWPARAMS, MPFROMP( &wp ), 0 );

        nlsThousandsULong( dvdata.szSize, dvdata.ulSize,
                           pGlobal->ctry.cThousands );
        strncat( dvdata.szSize,
                 ( pGlobal->fsProgram & FS_APP_IECSIZES ) ? " MiB" : " MB",
                 SIZE_TEXT_LIMIT );
        WinSendMsg( hwndDisk, WM_SETWINDOWPARAMS, MPFROMP( &wp ), 0 );

    }

    if ( pGlobal->hwndTT ) {
        // Get the tool info for each partition and update the tooltip text
        WinLoadString( pGlobal->hab, pGlobal->hmri, IDS_TOOLTIP_PARTITION,
                       STRING_RES_MAXZ, szRes1 );
        usParts = (USHORT) WinSendMsg( pGlobal->hwndTT, TTM_GETTOOLCOUNT, 0, 0 );
        for ( i = 0; i < usParts; i++ ) {
            ttinfo.ulFlags       = 0;
            ttinfo.hwndTool      = NULLHANDLE;
            ttinfo.hwndToolOwner = NULLHANDLE;
            ttinfo.pszText       = NULL;
            if ( ! WinSendMsg( pGlobal->hwndTT, TTM_ENUMTOOLS,
                               MPFROMSHORT( i ), MPFROMP( &ttinfo )))
                continue;
            if ( !ttinfo.pszText )
                continue;

            wpp.fsStatus     = WPM_CTLDATA;
            wpp.cchText      = 0;
            wpp.pszText      = NULL;
            wpp.cbPresParams = 0;
            wpp.pPresParams  = NULL;
            wpp.cbCtlData    = sizeof( PVCTLDATA );
            wpp.pCtlData     = &pvdata;
            WinSendMsg( ttinfo.hwndTool, WM_QUERYWINDOWPARAMS,
                        MPFROMP( &wpp ), 0 );

            nlsThousandsULong( szSize, pvdata.ulSize,
                               pGlobal->ctry.cThousands );
            switch ( pvdata.bType ) {
                case LPV_TYPE_FREE    :
                case LPV_TYPE_UNUSABLE: usRes = IDS_TERMS_FREE;
                                        break;
                case LPV_TYPE_LOGICAL : usRes = IDS_TERMS_LOGICAL;
                                        break;
                default               : usRes = IDS_TERMS_PRIMARY;
                                        break;
            }
            WinLoadString( pGlobal->hab, pGlobal->hmri, usRes,
                           STRING_RES_MAXZ, szType );

            sprintf( ttinfo.pszText, szRes1, pvdata.szName, szType, szSize,
                     ( pGlobal->fsProgram & FS_APP_IECSIZES ) ? " MiB" : " MB" );
            WinSendMsg( pGlobal->hwndTT, TTM_SETTOOLINFO, 0, MPFROMP( &ttinfo ));
        }
    }


    // Refresh the volume info-panel contents
    hwndCnr = WinWindowFromID( pGlobal->hwndVolumes, IDD_VOLUMES ),
    pRec = (PDVMVOLUMERECORD) WinSendMsg( hwndCnr, CM_QUERYRECORDEMPHASIS,
                                          MPFROMP( CMA_FIRST ),
                                          MPFROMSHORT( CRA_SELECTED ));
    if ( pRec && ( (ULONG) pRec != -1 ))
        VolumeContainerSelect( hwnd, pGlobal->hwndPopupVolume, pRec );

}


/* ------------------------------------------------------------------------- *
 * ChangeVolumeTypeDisplay()                                                 *
 *                                                                           *
 * Change the terminology for volume type (e.g. 'Advanced' vs 'LVM')         *
 * throughout the UI, according to the configured preference.                *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd         : Client window handle                                *
 *   PDVMGLOBAL pGlobal: Application global data (includes setting flag)     *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void ChangeVolumeTypeDisplay( HWND hwnd, PDVMGLOBAL pGlobal )
{
    PDVMVOLUMERECORD pRec;
    HWND      hwndCnr;                         // handle of container control
    CHAR      szRes[ STRING_RES_MAXZ ] = {0};  // string resource buffers

    if ( !pGlobal ) return;

    hwndCnr = WinWindowFromID( pGlobal->hwndVolumes, IDD_VOLUMES ),

    // Change the volume type column for all volume records
    pRec = (PDVMVOLUMERECORD) WinSendMsg( hwndCnr, CM_QUERYRECORD, NULL,
                                          MPFROM2SHORT( CMA_FIRST,
                                                        CMA_ITEMORDER ));

    while ( pRec ) {
        if ( pRec->pszType ) free( pRec->pszType );

        if ( pGlobal->volumes[ pRec->ulVolume ].fCompatibility ) {
            WinLoadString( pGlobal->hab, pGlobal->hmri,
                           ( pGlobal->fsProgram & FS_APP_IBMTERMS ) ?
                           IDS_TERMS_COMPATIBILITY : IDS_TERMS_STANDARD,
                           STRING_RES_MAXZ, szRes );
        }
        else {
            WinLoadString( pGlobal->hab, pGlobal->hmri,
                           ( pGlobal->fsProgram & FS_APP_IBMTERMS ) ?
                           IDS_TERMS_LVM : IDS_TERMS_ADVANCED,
                           STRING_RES_MAXZ, szRes );
        }
        pRec->pszType = strdup( szRes );

        pRec = (PDVMVOLUMERECORD) pRec->record.preccNextRecord;
    }

    WinSendDlgItemMsg( pGlobal->hwndVolumes, IDD_VOLUMES, CM_INVALIDATERECORD,
                       NULL, MPFROM2SHORT( 0, CMA_TEXTCHANGED ));

    // Refresh the volume info-panel contents
    pRec = (PDVMVOLUMERECORD) WinSendMsg( hwndCnr, CM_QUERYRECORDEMPHASIS,
                                          MPFROMP( CMA_FIRST ),
                                          MPFROMSHORT( CRA_SELECTED ));
    if ( pRec && ( (ULONG) pRec != -1 ))
        VolumeContainerSelect( hwnd, pGlobal->hwndPopupVolume, pRec );

}


/* ------------------------------------------------------------------------- *
 * SetBootMgrActions()                                                       *
 *                                                                           *
 * Sets up the contents of the "Boot Manager" menu according to whether      *
 * Boot Manager and/or Air-Boot are enabled in preferences.  (Note that if   *
 * Boot Manager is installed and active, the "Boot Manager options" and      *
 * partition/volume "Bootable" items will always be added.)                  *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PDVMGLOBAL pGlobal: Application global data (includes setting flag)     *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void SetBootMgrActions( PDVMGLOBAL pGlobal )
{
    HWND     hBMgrMenu    = NULLHANDLE,
             hVolMenu     = NULLHANDLE,
             hPartMenu    = NULLHANDLE;
    MENUITEM mi;
    CHAR     szRes[ STRING_RES_MAXZ ];
    SHORT    asIDs[ 6 ];


    if ( !pGlobal ) return;

    // Get the handles of the various submenus that we need...
    if ( winhQueryMenuItem( pGlobal->hwndMenu, IDM_LVM_BOOTMGR, TRUE, &mi ))
        hBMgrMenu = mi.hwndSubMenu;
    if ( winhQueryMenuItem( pGlobal->hwndMenu, IDM_VOLUME, TRUE, &mi ))
        hVolMenu = mi.hwndSubMenu;
    if ( winhQueryMenuItem( pGlobal->hwndMenu, IDM_PARTITION, TRUE, &mi ))
        hPartMenu = mi.hwndSubMenu;

    /* First, we remove all BootManager/Air-Boot specific menu items.  This
     * will allow us to selectively add the appropriate ones back in without
     * having to worry about messing up the order.
     */
    if ( hVolMenu )
        WinSendMsg( hVolMenu, MM_DELETEITEM,
                    MPFROM2SHORT( ID_VOLUME_BOOTABLE, TRUE ), 0 );
    if ( pGlobal->hwndPopupVolume )
        WinSendMsg( pGlobal->hwndPopupVolume, MM_DELETEITEM,
                    MPFROM2SHORT( ID_VOLUME_BOOTABLE, TRUE ), 0 );
    if ( hPartMenu )
        WinSendMsg( hPartMenu, MM_DELETEITEM,
                    MPFROM2SHORT( ID_PARTITION_BOOTABLE, TRUE ), 0 );
    if ( pGlobal->hwndPopupPartition )
        WinSendMsg( pGlobal->hwndPopupPartition, MM_DELETEITEM,
                    MPFROM2SHORT( ID_PARTITION_BOOTABLE, TRUE ), 0 );

    asIDs[ 0 ] = ID_AIRBOOT_INSTALL;
    asIDs[ 1 ] = ID_AIRBOOT_REMOVE;
    asIDs[ 2 ] = IDM_BM_SEPARATOR;
    asIDs[ 3 ] = ID_BM_OPTIONS;
    asIDs[ 4 ] = ID_BM_INSTALL;
    asIDs[ 5 ] = ID_BM_REMOVE;
    if ( hBMgrMenu )
        winhRemoveMenuItems( hBMgrMenu, asIDs, 6 );

    // If the menu is to remain otherwise empty, just add an information item
    if ( ! ( pGlobal->fsProgram & FS_APP_ENABLE_AB ) &&
         ! ( pGlobal->fsProgram & FS_APP_ENABLE_BM ) &&
         ! ( pGlobal->fsEngine  & FS_ENGINE_BOOTMGR ))
    {
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_MENU_NO_BOOTMGR, STRING_RES_MAXZ, szRes );
        MenuItemAddCnd( hBMgrMenu, MIT_END,
                        ID_AIRBOOT_REMOVE, szRes, MIS_STATIC );
        return;
    }

    // If Air-Boot is enabled, add these menu items...
    if ( pGlobal->fsProgram & FS_APP_ENABLE_AB ) {
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_MENU_AIRBOOT_INSTALL, STRING_RES_MAXZ, szRes );
        MenuItemAddCnd( hBMgrMenu, MIT_END,
                        ID_AIRBOOT_INSTALL, szRes, MIS_TEXT );
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_MENU_AIRBOOT_REMOVE, STRING_RES_MAXZ, szRes );
        MenuItemAddCnd( hBMgrMenu, MIT_END,
                        ID_AIRBOOT_REMOVE, szRes, MIS_TEXT );
    }

    // Add these menu items if Boot Manager is enabled...
    if ( pGlobal->fsProgram & FS_APP_ENABLE_BM ) {

        if ( pGlobal->fsProgram & FS_APP_ENABLE_AB )
            MenuItemAddCnd( hBMgrMenu, MIT_END,
                            IDM_BM_SEPARATOR, "", MIS_SEPARATOR );

        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_MENU_BOOTMGR_OPTIONS, STRING_RES_MAXZ, szRes );
        MenuItemAddCnd( hBMgrMenu, MIT_END, ID_BM_OPTIONS, szRes, MIS_TEXT );

        if ( !( pGlobal->fsProgram & FS_APP_ENABLE_AB ))
            MenuItemAddCnd( hBMgrMenu, MIT_END,
                            IDM_BM_SEPARATOR, "", MIS_SEPARATOR );

        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_MENU_BOOTMGR_INSTALL, STRING_RES_MAXZ, szRes );
        MenuItemAddCnd( hBMgrMenu, MIT_END,
                        ID_BM_INSTALL, szRes, MIS_TEXT );

        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_MENU_BOOTMGR_REMOVE, STRING_RES_MAXZ, szRes );
        MenuItemAddCnd( hBMgrMenu, MIT_END,
                        ID_BM_REMOVE, szRes, MIS_TEXT );

        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_MENU_BOOTABLE, STRING_RES_MAXZ, szRes );
        MenuItemAddCnd( hVolMenu, 6,
                        ID_VOLUME_BOOTABLE, szRes, MIS_TEXT );
        MenuItemAddCnd( pGlobal->hwndPopupVolume, 6,
                        ID_VOLUME_BOOTABLE, szRes, MIS_TEXT );
        MenuItemAddCnd( hPartMenu, 8,
                        ID_PARTITION_BOOTABLE, szRes, MIS_TEXT );
        MenuItemAddCnd( pGlobal->hwndPopupPartition, 8,
                        ID_PARTITION_BOOTABLE, szRes, MIS_TEXT );

    }

    // If BM is actually installed & active, add these anyway...
    else if ( pGlobal->fsEngine & FS_ENGINE_BOOTMGR ) {
        if ( pGlobal->fsProgram & FS_APP_ENABLE_AB )
            MenuItemAddCnd( hBMgrMenu, MIT_END,
                            IDM_BM_SEPARATOR, "", MIS_SEPARATOR );
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_MENU_BOOTMGR_OPTIONS, STRING_RES_MAXZ, szRes );
        MenuItemAddCnd( hBMgrMenu, MIT_END,
                        ID_BM_OPTIONS, szRes, MIS_TEXT );
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_MENU_BOOTABLE, STRING_RES_MAXZ, szRes );
        MenuItemAddCnd( hVolMenu, MIT_END,
                        ID_VOLUME_BOOTABLE, szRes, MIS_TEXT );
        MenuItemAddCnd( pGlobal->hwndPopupPartition, MIT_END,
                        ID_VOLUME_BOOTABLE, szRes, MIS_TEXT );
        MenuItemAddCnd( hPartMenu, MIT_END,
                        ID_PARTITION_BOOTABLE, szRes, MIS_TEXT );
        MenuItemAddCnd( pGlobal->hwndPopupPartition, MIT_END,
                        ID_PARTITION_BOOTABLE, szRes, MIS_TEXT );
    }

    // Disable the Bootable menu options by default
    MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, ID_VOLUME_BOOTABLE,    FALSE );
    MenuItemEnable( pGlobal->hwndMenu, pGlobal->hwndPopupPartition, ID_PARTITION_BOOTABLE, FALSE );
}


/* ------------------------------------------------------------------------- *
 * SetAvailableActions()                                                     *
 *                                                                           *
 * Sets the global actions which are enabled/disabled (in the menus, etc.)   *
 * at startup.  Only called after LVM engine has been opened successfully.   *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd: handle of the program client window                          *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void SetAvailableActions( HWND hwnd )
{
    PDVMGLOBAL  pGlobal;


    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( !pGlobal ) return;

    // Always enable these items (as long as LVM is open )
    MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, IDM_LVM_BOOTMGR, TRUE );
    MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, IDM_VOLUME, TRUE );
    MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, IDM_PARTITION, TRUE );
    MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, ID_LVM_NEWMBR, TRUE );
    MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, ID_LVM_REFRESH,  TRUE );

    // Enable Air-Boot removal if (and only if) Air-Boot is installed
    MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, ID_AIRBOOT_REMOVE,
                    (BOOL)( pGlobal->fsEngine & FS_ENGINE_AIRBOOT ));

    // Disable Air-Boot install if the install program is missing
    MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, ID_AIRBOOT_INSTALL,
                    (BOOL)( pGlobal->fsProgram & FS_APP_ENABLE_AB ));

    if ( pGlobal->fsEngine & FS_ENGINE_BOOTMGR ) {
        // If IBM Boot Manager is installed...
        MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, ID_BM_INSTALL, FALSE );
        MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, ID_BM_REMOVE,  TRUE );
        MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, ID_BM_OPTIONS, TRUE );
    }
    else {
        // Otherwise...
        MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, ID_BM_INSTALL, TRUE );
        MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, ID_BM_REMOVE,  FALSE );
        MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, ID_BM_OPTIONS, FALSE );
    }

    // Leave ID_LVM_DISK disabled (will enable when a disk is selected)
    // Leave ID_LVM_SAVE disabled (will enable when FS_ENGINE_PENDING gets set)

    /* Remaining Volume and Partition menu items will only be enabled when
     * an applicable item is selected.
     */

}


/* ------------------------------------------------------------------------- *
 * SetModified()                                                             *
 *                                                                           *
 * Sets the internal flag and the UI text control which indicate that there  *
 * are LVM changes outstanding.                                              *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd     : handle of the main client window                        *
 *   BOOL fModified: TRUE to set the modified flags, FALSE to clear them.    *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void SetModified( HWND hwnd, BOOL fModified )
{
    PDVMGLOBAL  pGlobal;
    CHAR        szRes[ STRING_RES_MAXZ ];

    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( !pGlobal ) return;

    if ( fModified ) {
        // Set the internal flags
        pGlobal->fsEngine |= FS_ENGINE_PENDING;

        // Enable the Save menuitem
        MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, ID_LVM_SAVE, TRUE );

        // Update the status display
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_STATUS_MODIFIED, STRING_RES_MAXZ, szRes );
        WinSetDlgItemText( hwnd, IDD_STATUS_MODIFIED, szRes );
    }
    else {
        pGlobal->fsEngine &= ~FS_ENGINE_PENDING;
        MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, ID_LVM_SAVE, FALSE );
        WinSetDlgItemText( hwnd, IDD_STATUS_MODIFIED, "");
    }
}


/* ------------------------------------------------------------------------- *
 * LVM_Start()                                                               *
 *                                                                           *
 * This function opens the LVM engine (and logs the result if appropriate).  *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   FILE   *pLog: Handle of program logfile                                 *
 *   HAB     hab : Anchor-block for the application                          *
 *   HMODULE hmri: Handle of the resource containing NLV stringtables        *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if LVM was started successfully.                                   *
 * ------------------------------------------------------------------------- */
BOOL LVM_Start( FILE *pLog, HAB hab, HMODULE hmri )
{
    CARDINAL32 ulError;

    LvmOpen( TRUE, &ulError );
    if ( pLog ) {
        fprintf( pLog, "===============================================================================\n");
        if ( ulError != LVM_ENGINE_NO_ERROR )
            fprintf( pLog, "Failed to open LVM Engine (error code %u)\n", ulError );
        else
            fprintf( pLog, "LVM Engine opened successfully\n");
    }

    if ( ulError != LVM_ENGINE_NO_ERROR ) {
        PopupEngineError( NULL, ulError, HWND_DESKTOP, hab, hmri );
        return FALSE;
    }

    return TRUE;
}


/* ------------------------------------------------------------------------- *
 * LVM_Stop()                                                                *
 *                                                                           *
 * This function closes the LVM engine (and logs it if appropriate).         *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PDVMGLOBAL pGlobal: global application data                             *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void LVM_Stop( PDVMGLOBAL pGlobal )
{
    LvmClose();

    // Disable these menu items
    MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, IDM_LVM_BOOTMGR, FALSE );
    MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, ID_LVM_NEWMBR,   FALSE );
    MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, ID_LVM_REFRESH,  FALSE );
    MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, IDM_VOLUME,      FALSE );
    MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, IDM_PARTITION,   FALSE );

    if ( pGlobal->pLog ) {
        fprintf( pGlobal->pLog, "-------------------------------------------------------------------------------\n");
        fprintf( pGlobal->pLog, "LVM Engine closed\n");
        fprintf( pGlobal->pLog, "===============================================================================\n\n");
    }
}


/* ------------------------------------------------------------------------- *
 * LVM_InitData()                                                            *
 *                                                                           *
 * This function populates our global disk/volume/partition information      *
 * structures by gathering information from the LVM engine.                  *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PDVMGLOBAL pGlobal: global application data                             *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if the data structures were populated successfully.                *
 * ------------------------------------------------------------------------- */
BOOL LVM_InitData( HWND hwnd, PDVMGLOBAL pGlobal )
{
    Drive_Control_Array       dca;
    Drive_Information_Record  dir;
    Volume_Control_Array      vca;
    Volume_Information_Record vir;

    ADDRESS    bootmgr;                         // Boot Manager partition handle
    CARDINAL32 rc;                              // LVM return code
    CHAR       szRes[ STRING_RES_MAXZ ],        // string resource buffer
               szError[ STRING_RES_MAXZ + 20 ]; // error-message buffer
    ULONG      cbCR,                            // size of a control record
               cbIR,                            // size of an information record
               i, current;                      // array indices


    /* See if Boot Manager is installed
     */
    bootmgr = LvmGetBootMgr( &rc );
    if (( bootmgr != NULL ) && ( rc == LVM_ENGINE_NO_ERROR ))
        pGlobal->fsEngine |= FS_ENGINE_BOOTMGR;


    /* See if Air-Boot is installed
     */
    if ( AirBoot_IsInstalled( &rc ) && ( rc == LVM_ENGINE_NO_ERROR ))
        pGlobal->fsEngine |= FS_ENGINE_AIRBOOT;


    /* Get the physical disk drive information.
     */
    cbCR = sizeof( Drive_Control_Record );
    cbIR = sizeof( Drive_Information_Record );

    // Request the invariant disk drive data from LVM
    dca = LvmGetDisks( &rc );
    if ( rc != LVM_ENGINE_NO_ERROR ) {
        pGlobal->ulDisks = 0;
        pGlobal->disks   = NULL;
        PopupEngineError( NULL, rc, hwnd, pGlobal->hab, pGlobal->hmri );
        return ( FALSE );
    }
    pGlobal->disks = (PLVMDISKINFO) calloc( dca.Count, sizeof( LVMDISKINFO ));
    if ( !pGlobal->disks ) {
        pGlobal->ulDisks = 0;
        pGlobal->disks   = NULL;
        LvmFreeMem( dca.Drive_Control_Data );
        return ( FALSE );
    }

    // Now ask LVM for each disk's variant data
    for ( i = 0, current = 0; i < dca.Count; i++ ) {
        dir = LvmGetDiskInfo( dca.Drive_Control_Data[ i ].Drive_Handle, &rc );
        if ( rc ) continue;

        // Populate our amalgamated data structure
        memcpy( &(pGlobal->disks[ current ]), &(dca.Drive_Control_Data[ i ]), cbCR );
        memcpy( (PBYTE) &(pGlobal->disks[ current ]) + cbCR, &dir, cbIR );

        // Adjust certain values for our requirements
        if ( dir.Unusable )
            pGlobal->disks[ current ].fCorrupt = FALSE;
        pGlobal->disks[ current ].iSize = SECS_TO_MiB( pGlobal->disks[ current ].iSize );

        // Display a warning for any disk that reports a corrupt partition table
        if ( pGlobal->disks[ current ].fCorrupt ) {
            WinLoadString( pGlobal->hab, pGlobal->hmri,
                           IDS_ERROR_CORRUPT, STRING_RES_MAXZ, szRes );
            sprintf( szError, szRes, pGlobal->disks[ current ].iNumber );
            WinLoadString( pGlobal->hab, pGlobal->hmri, IDS_ERROR_GENERIC,
                           STRING_RES_MAXZ, szRes );
            WinMessageBox( HWND_DESKTOP, hwnd, szError, szRes, 0,
                           MB_MOVEABLE | MB_OK | MB_ERROR );
        }
        current++;
    }
    pGlobal->ulDisks = current;
    LvmFreeMem( dca.Drive_Control_Data );

    if ( pGlobal->pLog )
        Log_DiskInfo( pGlobal );


    /* Get the logical volume information.
     */
    cbCR = sizeof( Volume_Control_Record );
    cbIR = sizeof( Volume_Information_Record );

    // Request the invariant volume data from LVM
    vca = LvmGetVolumes( &rc );
    if ( rc != LVM_ENGINE_NO_ERROR ) {
        pGlobal->ulVolumes = 0;
        pGlobal->volumes   = NULL;
        PopupEngineError( NULL, rc, hwnd, pGlobal->hab, pGlobal->hmri );
        return ( FALSE );
    }
    pGlobal->ulVolumes = vca.Count;
    pGlobal->volumes   = (PLVMVOLUMEINFO) calloc( vca.Count,
                                                  sizeof( LVMVOLUMEINFO ));
    if ( !pGlobal->volumes ) {
        pGlobal->ulVolumes = 0;
        pGlobal->volumes   = NULL;
        LvmFreeMem( vca.Volume_Control_Data );
        return ( FALSE );
    }

    // Now ask LVM for each volume's variant data
    for ( i = 0; i < vca.Count; i++ ) {
        vir = LvmGetVolumeInfo( vca.Volume_Control_Data[ i ].Volume_Handle, &rc );
        if ( rc ) continue;

        // Populate our amalgamated data structure
        memcpy( &(pGlobal->volumes[ i ]), &(vca.Volume_Control_Data[ i ]), cbCR );
        memcpy( (PBYTE) &(pGlobal->volumes[ i ]) + cbCR, &vir, cbIR );

        // Convert the size into megs
        pGlobal->volumes[ i ].iSize = SECS_TO_MiB( pGlobal->volumes[ i ].iSize );
    }
    LvmFreeMem( vca.Volume_Control_Data );

    if ( pGlobal->pLog )
        Log_VolumeInfo( pGlobal );

    return TRUE;
}


/* ------------------------------------------------------------------------- *
 * LVM_FreeData()                                                            *
 *                                                                           *
 * This function empties and (and, where applicable, un-allocates) our       *
 * global disk/volume/partition information structures                       *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PDVMGLOBAL pGlobal: global application data                             *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void LVM_FreeData( PDVMGLOBAL pGlobal )
{
    if ( pGlobal->disks )
        free( pGlobal->disks );
    if ( pGlobal->volumes )
        free( pGlobal->volumes );
}


/* ------------------------------------------------------------------------- *
 * LVM_Refresh()                                                             *
 *                                                                           *
 * Refresh all data about disks and partitions.  This function clears the    *
 * UI, frees the current LVM data structures, then queries the LVM engine    *
 * for up-to-date information.  If successful, the UI is then repopulated    *
 * with the new data.                                                        *
 *                                                                           *
 * This function should be called whenever a change is made to the disk or   *
 * volume layout (a partition or volume is created or deleted, etc.).        *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd : application client window handle                            *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void LVM_Refresh( HWND hwnd )
{
    PDVMGLOBAL       pGlobal;       // global application data
    PDVMVOLUMERECORD pVol;          // selected volume record
    WNDPARAMS        wp     = {0};  // used to query disk control data
    DVCTLDATA        dvdata = {0};  // selected disk's control data
    HWND             hwndFocus,     // current focus window
                     hwndDisk,      // window handle of the selected disk
                     hwndPart,      // window handle of the selected partition
                     hwndPartCnr;   // window handle of the partition container
    ADDRESS          hVolume;       // handle of the selected volume
    USHORT           part;          // number of the selected partition
    CARDINAL32       disk,          // number of the selected disk (from 1)
                     rc;            // LVM return code


    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( !pGlobal ) return;

    // Remember the currently-selected disk+partition by saving their numbers
    hwndDisk = (HWND) WinSendMsg( pGlobal->hwndDisks, LLM_QUERYDISKEMPHASIS,
                                  0, MPFROMSHORT( LDV_FS_SELECTED ));
    wp.fsStatus  = WPM_CTLDATA;
    wp.cbCtlData = sizeof( DVCTLDATA );
    wp.pCtlData  = &dvdata;
    WinSendMsg( hwndDisk, WM_QUERYWINDOWPARAMS, MPFROMP( &wp ), 0 );
    disk = dvdata.number;

    part = (USHORT) WinSendMsg( hwndDisk, LDM_QUERYLASTSELECTED,
                                MPFROMSHORT( FALSE ), 0 );

    // Remember the selected volume
    pVol = (PDVMVOLUMERECORD) WinSendDlgItemMsg( pGlobal->hwndVolumes, IDD_VOLUMES,
                                                 CM_QUERYRECORDEMPHASIS,
                                                 MPFROMP( CMA_FIRST ),
                                                 MPFROMSHORT( CRA_SELECTED ));
    if ( pVol && pGlobal->volumes )
        hVolume = pGlobal->volumes[ pVol->ulVolume ].handle;

    // Empty the GUI
    VolumeContainerClear( pGlobal );
    hwndPartCnr = (HWND) WinSendDlgItemMsg( pGlobal->hwndVolumes, IDD_VOL_INFO,
                                            VIM_GETCONTAINERHWND, 0, 0 );
    PartitionContainerClear( hwndPartCnr, pGlobal );
    DiskListClear( pGlobal );

    // Clear the data structures
    LVM_FreeData( pGlobal );

    // Now get the current data from LVM
    LvmRefresh( &rc );
    if ( pGlobal->pLog ) {
        fprintf( pGlobal->pLog, "===============================================================================\n");
        if ( rc != LVM_ENGINE_NO_ERROR )
            fprintf( pGlobal->pLog, "Failed to refresh LVM Engine (error code %u)\n", rc );
        else
            fprintf( pGlobal->pLog, "Refreshed LVM Engine successfully\n");
    }

    LVM_InitData( hwnd, pGlobal );
    pGlobal->fsEngine &= ~FS_ENGINE_REFRESH;

    // Repopulate the GUI
    VolumeContainerPopulate( WinWindowFromID( pGlobal->hwndVolumes, IDD_VOLUMES ),
                             pGlobal->volumes, pGlobal->ulVolumes,
                             pGlobal->hab, pGlobal->hmri,
                             pGlobal->fsProgram, FALSE
                           );
    DiskListPopulate( hwnd );

    // Re-select the previously selected disk+partition (if they still exist)
    hwndDisk = (HWND) WinSendMsg( pGlobal->hwndDisks, LLM_QUERYDISKHWND,
                                  MPFROMLONG( disk-1 ), MPFROMSHORT( TRUE ));
    hwndPart = (HWND) WinSendMsg( hwndDisk, LDM_QUERYPARTITIONHWND,
                                  MPFROMLONG( part ), MPFROMSHORT( TRUE ));
    WinSendMsg( hwndDisk, LDM_SETEMPHASIS, MPFROMP( hwndPart ),
                MPFROM2SHORT( TRUE, LDV_FS_SELECTED | LPV_FS_SELECTED ));

    // Re-select the previously selected volume (if it still exists)
    if ( pGlobal->volumes ) {
        pVol = (PDVMVOLUMERECORD) \
            WinSendDlgItemMsg( pGlobal->hwndVolumes, IDD_VOLUMES, CM_QUERYRECORD,
                               NULL, MPFROM2SHORT( CMA_FIRST, CMA_ITEMORDER ));
        while ( pVol && ( pGlobal->volumes[ pVol->ulVolume ].handle != hVolume ))
            pVol = (PDVMVOLUMERECORD) pVol->record.preccNextRecord;
        if ( pVol && pGlobal->volumes[ pVol->ulVolume ].handle == hVolume ) {
            WinSendDlgItemMsg( pGlobal->hwndVolumes, IDD_VOLUMES,
                               CM_SETRECORDEMPHASIS, MPFROMP( pVol ),
                               MPFROM2SHORT( TRUE, CRA_SELECTED ));
            VolumeContainerSelect( hwnd, pGlobal->hwndPopupVolume, pVol );
        }
    }

    // Update the menu options as needed
    SetAvailableActions( hwnd );

    // Force a statusbar update
    hwndFocus = WinQueryFocus( HWND_DESKTOP );
    if ( hwndFocus == pGlobal->hwndDisks )
        DiskListPartitionSelect( hwnd, hwndPart );
    else
        WinSendMsg( hwndDisk, LDM_SETEMPHASIS, 0,
                    MPFROM2SHORT( FALSE, LPV_FS_SELECTED ));
}


/* ------------------------------------------------------------------------- *
 * Settings_Load()                                                           *
 *                                                                           *
 * Loads any saved preference and font settings from the INI file.           *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PDVMGLOBAL pGlobal   : Application global data                          *
 *   PLONG pX, pY, pW, pH : Returned window position/size coordinates        *
 *   PLONG pS             : Returned splitbar position                       *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void Settings_Load( PDVMGLOBAL pGlobal, PLONG pX, PLONG pY, PLONG pW, PLONG pH, PLONG pS )
{
    ULONG  cb;               // size of profile item
    RECTL  rcl;              // loaded window position
    LONG   lSplit;
    USHORT fsPrefs;          // initial preferences mask

    // Load the window position & size
    if ( PrfQueryProfileSize( HINI_USERPROFILE, SZ_INI_APP,
                              SZ_INI_KEY_POSITION, &cb ) &&
         ( cb == sizeof( RECTL )))
    {
        PrfQueryProfileData( HINI_USERPROFILE, SZ_INI_APP,
                             SZ_INI_KEY_POSITION, &rcl, &cb );
        *pX = rcl.xLeft;
        *pY = rcl.yBottom;
        *pW = rcl.xRight;
        *pH = rcl.yTop;
    }
    else {
        *pX = 0;
        *pY = 0;
        *pW = 0;
        *pH = 0;
    }

    // Load the splitbar position
    lSplit = 0;
    if ( PrfQueryProfileSize( HINI_USERPROFILE, SZ_INI_APP,
                              SZ_INI_KEY_SPLITBAR, &cb ) &&
         ( cb == sizeof( LONG )))
    {
        PrfQueryProfileData( HINI_USERPROFILE, SZ_INI_APP,
                             SZ_INI_KEY_SPLITBAR, &lSplit, &cb );
    }
    *pS = lSplit;

    // Load the preference flags
    if ( PrfQueryProfileSize( HINI_USERPROFILE, SZ_INI_APP,
                              SZ_INI_KEY_FLAGS, &cb ) &&
         ( cb == sizeof( USHORT )))
    {
        PrfQueryProfileData( HINI_USERPROFILE, SZ_INI_APP,
                             SZ_INI_KEY_FLAGS, &fsPrefs, &cb );
        // Clear non-preference bits just in case they got saved somehow
        fsPrefs &= FS_APP_PREFERENCES;
    }
    else cb = 0;
    if ( !cb )
    // (Default preferences if none were found)
        fsPrefs = FS_APP_BOOTWARNING | FS_APP_PMSTYLE |
                  FS_APP_ENABLE_BM | FS_APP_ENABLE_AB;

    pGlobal->fsProgram |= fsPrefs;

    // Load the font selections
    PrfQueryProfileString( HINI_USERPROFILE, SZ_INI_APP, SZ_INI_KEY_FONT_STB,
                           "9.WarpSans", pGlobal->szFontMain, FACESIZE+4 );
    PrfQueryProfileString( HINI_USERPROFILE, SZ_INI_APP, SZ_INI_KEY_FONT_CNR,
                           "9.WarpSans", pGlobal->szFontCnr, FACESIZE+4 );
    PrfQueryProfileString( HINI_USERPROFILE, SZ_INI_APP, SZ_INI_KEY_FONT_VOL,
                           "9.WarpSans", pGlobal->szFontInfo, FACESIZE+4 );
    PrfQueryProfileString( HINI_USERPROFILE, SZ_INI_APP, SZ_INI_KEY_FONT_DSK,
                           "9.WarpSans", pGlobal->szFontDisks, FACESIZE+4 );
    PrfQueryProfileString( HINI_USERPROFILE, SZ_INI_APP, SZ_INI_KEY_FONT_DLG,
                           NULL, pGlobal->szFontDlgs, FACESIZE+4 );
}


/* ------------------------------------------------------------------------- *
 * Settings_Save()                                                           *
 *                                                                           *
 * Saves the current preference and font settings to the INI file.           *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwndFrame : Application frame handle                         *
 *   PDVMGLOBAL pGlobal   : Application global data                          *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void Settings_Save( HWND hwndFrame, PDVMGLOBAL pGlobal )
{
    SWP    wp;              // result from WinQueryWindowPos
    RECTL  rcl;             // saved window size/position
    USHORT fsPref;          // configured preference flags to save
    LONG   lSplit;          // splitbar position

    // Save the window position
    if ( WinQueryWindowPos( hwndFrame, &wp )) {
        rcl.xLeft   = wp.x;
        rcl.yBottom = wp.y;
        rcl.xRight  = wp.cx;
        rcl.yTop    = wp.cy;
        PrfWriteProfileData( HINI_USERPROFILE, SZ_INI_APP,
                             SZ_INI_KEY_POSITION, &rcl, sizeof( RECTL ));
    }
    // Save the splitbar position
    lSplit = ctlQuerySplitPos( pGlobal->hwndSplit );
    if ( lSplit )
        PrfWriteProfileData( HINI_USERPROFILE, SZ_INI_APP,
                             SZ_INI_KEY_SPLITBAR, &lSplit, sizeof( LONG ));

    // Save the preference flags
    fsPref = pGlobal->fsProgram & FS_APP_PREFERENCES;
    PrfWriteProfileData( HINI_USERPROFILE, SZ_INI_APP, SZ_INI_KEY_FLAGS,
                         &fsPref, sizeof( USHORT ));

    // Save the font selections
    PrfWriteProfileString( HINI_USERPROFILE, SZ_INI_APP, SZ_INI_KEY_FONT_STB,
                           (PSZ) pGlobal->szFontMain );
    PrfWriteProfileString( HINI_USERPROFILE, SZ_INI_APP, SZ_INI_KEY_FONT_CNR,
                           (PSZ) pGlobal->szFontCnr );
    PrfWriteProfileString( HINI_USERPROFILE, SZ_INI_APP, SZ_INI_KEY_FONT_VOL,
                           (PSZ) pGlobal->szFontInfo );
    PrfWriteProfileString( HINI_USERPROFILE, SZ_INI_APP, SZ_INI_KEY_FONT_DSK,
                           (PSZ) pGlobal->szFontDisks );
    PrfWriteProfileString( HINI_USERPROFILE, SZ_INI_APP, SZ_INI_KEY_FONT_DLG,
                           (PSZ) pGlobal->szFontDlgs );
}


