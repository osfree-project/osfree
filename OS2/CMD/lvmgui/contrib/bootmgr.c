/*****************************************************************************
 ** LVMPM - bootmgr.c                                                       **
 *****************************************************************************
 * Routines for working with the IBM Boot Manager.                           *
 *****************************************************************************
 *                                                                           *
 * Copyright (C) 2011-2019 Alexander Taylor.                                 *
 *                                                                           *
 *  This program is free software; you can redistribute it and/or modify it  *
 *  under the terms of the GNU General Public License as published by the    *
 *  Free Software Foundation; either version 2 of the License, or (at your   *
 *  option) any later version.                                               *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful, but      *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of               *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *  General Public License for more details.                                 *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License along  *
 *  with this program; if not, write to the Free Software Foundation, Inc.,  *
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA                  *
 *                                                                           *
 *****************************************************************************/

#include "lvmpm.h"


/* ------------------------------------------------------------------------- *
 * BootMgrOptions()                                                          *
 *                                                                           *
 * Presents the Boot Manager options dialog, and then handles any settings   *
 * that changed.                                                             *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd : Handle to the main program client window.                   *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void BootMgrOptions( HWND hwnd )
{
    DVMBOOTMGRPARAMS params;                   // dialog initialization data
    PDVMGLOBAL       pGlobal;                  // global application data


    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( !pGlobal ) return;

    params.hab       = pGlobal->hab;
    params.hmri      = pGlobal->hmri;
    strcpy( params.szFontDlgs, pGlobal->szFontDlgs );
    // this flag should not be set in the main program, but just to be safe...
    params.fsProgram = pGlobal->fsProgram & ~FS_CHANGED;

    // display the Boot Manager options dialog
    WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) BootMgrOptionsProc,
               pGlobal->hmri, IDD_BOOTMGR_OPTIONS, &params );

    if ( params.fsProgram & FS_CHANGED ) {
        SetModified( hwnd, TRUE );
    }

}


/* ------------------------------------------------------------------------- *
 * BootMgrOptionsProc()                                                      *
 *                                                                           *
 * Dialog procedure for the Boot Manager options dialog.                     *
 * See OS/2 PM Reference for a description of input and output.              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY BootMgrOptionsProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    static PDVMBOOTMGRPARAMS pData;

    ADDRESS    hDefault;        // handle of the default boot item
    BOOLEAN    fIsVolume,       // TRUE if hDefault represents a volume
               fTimed,          // TRUE if boot timeout is enabled
               fVerbose;        // TRUE when menu display is verbose/advanced
    CARDINAL32 timeout,         // current timeout interval in seconds
               rc;              // LVM return code
    SHORT      sDefault;        // default list entry index
    LONG       lTimerVal;       // read spinbox value
#if 0
    CHAR       szRes1[ STRING_RES_MAXZ ],  // string resource buffers
               szRes2[ STRING_RES_MAXZ ];
#endif

    switch ( msg ) {

        case WM_INITDLG:
            pData = (PDVMBOOTMGRPARAMS) mp2;
            if ( !pData ) {
                WinSendMsg( hwnd, WM_CLOSE, 0, 0 );
                break;
            }

            // Set up the border
            g_pfnRecProc = WinSubclassWindow(
                             WinWindowFromID( hwnd, IDD_DIALOG_INSET ),
                             InsetBorderProc );

            // Set the dialog font
            if ( pData->szFontDlgs[ 0 ] )
                WinSetPresParam( hwnd, PP_FONTNAMESIZE,
                                 strlen( pData->szFontDlgs ) + 1,
                                 (PVOID) pData->szFontDlgs );

            // Get the current Boot Manager settings
            LvmGetBootMgrOptions( &hDefault, &fIsVolume,
                                  &fTimed, &timeout, &fVerbose, &rc );
            if ( rc != LVM_ENGINE_NO_ERROR ) {
                PopupEngineError( NULL, rc, hwnd, pData->hab, pData->hmri );
                return (MRESULT) TRUE;
            }

            // Now populate the dialog fields with the retrieved settings
            //

            // Timeout settings
            WinSendDlgItemMsg( hwnd, IDD_BOOTMGR_TIMEOUT_VALUE, SPBM_SETLIMITS,
                               MPFROMLONG( LVM_BOOT_TIMEOUT_LIMIT ),
                               MPFROMLONG( 1 ));
            if ( fTimed ) {
                WinSendDlgItemMsg( hwnd, IDD_BOOTMGR_TIMEOUT_VALUE,
                                   SPBM_SETCURRENTVALUE,
                                   MPFROMLONG( timeout ), 0 );
                WinSendDlgItemMsg( hwnd, IDD_BOOTMGR_TIMEOUT_ENABLE,
                                   BM_SETCHECK, MPFROMSHORT( 1 ), 0 );
            } else {
                WinSendDlgItemMsg( hwnd, IDD_BOOTMGR_TIMEOUT_VALUE,
                                   SPBM_SETCURRENTVALUE,
                                   MPFROMLONG( US_BMGR_DEF_TIMEOUT ), 0 );
                WinSendDlgItemMsg( hwnd, IDD_BOOTMGR_TIMEOUT_ENABLE,
                                   BM_SETCHECK, MPFROMSHORT( 0 ), 0 );
            }

            // Default boot item
            sDefault = BootMgrPopulateMenu( hwnd, hDefault, pData );
#if 0
            if ( sDefault == BOOTMGR_MENU_EMPTY ) {
                WinLoadString( pData->hab, pData->hmri, IDS_BOOTMGR_EMPTY,
                               STRING_RES_MAXZ, szRes1 );
                WinLoadString( pData->hab, pData->hmri, IDS_BOOTMGR_EMPTY_TITLE,
                               STRING_RES_MAXZ, szRes2 );
                WinMessageBox( HWND_DESKTOP, hwnd, szRes1, szRes2,
                               0, MB_OK | MB_WARNING | MB_MOVEABLE  );
                msg = WM_COMMAND;       // forces dialog to close
            }
            else
#endif
            if ( sDefault == BOOTMGR_MENU_NODEF ) {
                WinSendDlgItemMsg( hwnd, IDD_BOOTMGR_DEFAULT_LIST,
                                   LM_SELECTITEM,
                                   MPFROMSHORT( 0 ), MPFROMSHORT( TRUE ));
            } else {
                WinSendDlgItemMsg( hwnd, IDD_BOOTMGR_DEFAULT_LIST,
                                   LM_SELECTITEM,
                                   MPFROMSHORT( sDefault+1 ),
                                   MPFROMSHORT( TRUE ));
            }

            // View mode
            if ( fVerbose )
                WinSendDlgItemMsg( hwnd, IDD_BOOTMGR_MODE_VERBOSE,
                                   BM_SETCHECK, MPFROMSHORT( 1 ), 0 );
            else
                WinSendDlgItemMsg( hwnd, IDD_BOOTMGR_MODE_BRIEF,
                                   BM_SETCHECK, MPFROMSHORT( 1 ), 0 );

            // Display the dialog
            CentreWindow( hwnd, WinQueryWindow( hwnd, QW_OWNER ),
                          SWP_ACTIVATE | SWP_SHOW );
            return (MRESULT) TRUE;


        case WM_COMMAND:
            switch ( SHORT1FROMMP( mp1 )) {

                case DID_OK:
                    // Get the selected values from the dialog
                    //

                    // Default boot item
                    sDefault = (SHORT) WinSendDlgItemMsg(
                                           hwnd, IDD_BOOTMGR_DEFAULT_LIST,
                                           LM_QUERYSELECTION,
                                           MPFROMSHORT( LIT_FIRST ), 0
                                       );
                    if ( sDefault > 0 )
                        hDefault = (ADDRESS) WinSendDlgItemMsg(
                                                 hwnd, IDD_BOOTMGR_DEFAULT_LIST,
                                                 LM_QUERYITEMHANDLE,
                                                 MPFROMSHORT( sDefault ), 0
                                             );
                    else
                        hDefault = NULL;

                    // Timeout settings
                    if ( WinQueryButtonCheckstate( hwnd,
                                                   IDD_BOOTMGR_TIMEOUT_ENABLE )
                         == 1 )
                    {
                        fTimed = TRUE;
                        WinSendDlgItemMsg( hwnd, IDD_BOOTMGR_TIMEOUT_VALUE,
                                           SPBM_QUERYVALUE,
                                           MPFROMP( &lTimerVal ),
                                           MPFROM2SHORT( 0, SPBQ_ALWAYSUPDATE ));
                        timeout = (CARDINAL32) lTimerVal;
                    }
                    else fTimed = FALSE;

                    // View mode setting
                    if ( WinQueryButtonCheckstate( hwnd,
                                                   IDD_BOOTMGR_MODE_VERBOSE )
                         == 1 )
                    {
                        fVerbose = TRUE;
                    }
                    else fVerbose = FALSE;

                    // Update Boot Manager with the new values
                    LvmSetBootMgrOptions( hDefault, fTimed,
                                          timeout, fVerbose, &rc );
                    if ( rc == LVM_ENGINE_NO_ERROR ) {
                        pData->fsProgram |= FS_CHANGED;
                    }
                    else
                        PopupEngineError( NULL, rc, hwnd, pData->hab, pData->hmri );
                    break;


                default: break;

            }

        default: break;
    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * BootMgrPopulateMenu()                                                     *
 *                                                                           *
 * Populate the 'bootable volumes' combo-box menu control with the list of   *
 * bootable volumes as reported by LVM.                                      *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND              hwnd    : dialog window handle                        *
 *   ADDRESS           hDefault: handle of the current default boot object   *
 *   PDVMBOOTMGRPARAMS pData   : dialog data structure                       *
 *                                                                           *
 * RETURNS: SHORT                                                            *
 *   The index (within the menu control) of the default boot item, or one    *
 *   of the following values:                                                *
 *     BOOTMGR_MENU_EMPTY: There are no items in the menu.                   *
 *     BOOTMGR_MENU_NODEF: There is no current default boot item.            *
 * ------------------------------------------------------------------------- */
SHORT BootMgrPopulateMenu( HWND hwnd, ADDRESS hDefault, PDVMBOOTMGRPARAMS pData )
{
    Boot_Manager_Menu       bootmenu; // LVM Boot Manager menu structure
    Boot_Manager_Menu_Item  entry;    // a single menu entry

    SHORT      sDefault = 0;          // index of the default entry, if any
    BOOLEAN    fDefault = FALSE;      // TRUE if a default entry is defined
    CARDINAL32 rc,                    // LVM return code
               i;                     // loop index
    CHAR       szName[ VOLUME_NAME_SIZE + 1 ],  // name of the current entry
               szRes[ STRING_RES_MAXZ ];        // string resource buffer


    // Request the current boot menu from the LVM engine
    bootmenu = LvmGetBootMgrMenu( &rc );
    if ( rc != LVM_ENGINE_NO_ERROR ) {
        PopupEngineError( NULL, rc, hwnd, pData->hab, pData->hmri );
#if 0
        return BOOTMGR_MENU_EMPTY;
#else
        return BOOTMGR_MENU_NODEF;
#endif
    }
#if 0
    if ( bootmenu.Count == 0 ) return BOOTMGR_MENU_EMPTY;
#endif

    // Add the 'last booted' option to the list
    WinLoadString( pData->hab, pData->hmri,
                   IDS_BOOTMGR_LAST_BOOTED, STRING_RES_MAXZ, szRes );
    WinSendDlgItemMsg( hwnd, IDD_BOOTMGR_DEFAULT_LIST,
                       LM_INSERTITEM, 0, MPFROMP( szRes ));

    // Now add the explicit menu items returned by LVM
    for ( i = 0; i < bootmenu.Count; i++ ) {
        entry = bootmenu.Menu_Items[ i ];

        // Get the name of this entry (volume or partition)
        if ( entry.Volume ) {
            Volume_Information_Record vir;
            vir = LvmGetVolumeInfo( entry.Handle, &rc );
            if ( rc == LVM_ENGINE_NO_ERROR )
                strncpy( szName, vir.Volume_Name, VOLUME_NAME_SIZE );
            else
                sprintf( szName, "%.8X (rc=%3u)", entry.Handle, rc );
        }
        else {
            Partition_Information_Record pir;
            pir = LvmGetPartitionInfo( entry.Handle, &rc );
            if ( rc == LVM_ENGINE_NO_ERROR )
                strncpy( szName, pir.Partition_Name, VOLUME_NAME_SIZE );
            else
                sprintf( szName, "%.8X (rc=%3u)", entry.Handle, rc );
        }

        // Add the name to the list
        WinSendDlgItemMsg( hwnd, IDD_BOOTMGR_DEFAULT_LIST, LM_INSERTITEM,
                           MPFROMSHORT( i+1 ), MPFROMP( szName ));

        // Save the handle as well
        WinSendDlgItemMsg( hwnd, IDD_BOOTMGR_DEFAULT_LIST, LM_SETITEMHANDLE,
                           MPFROMSHORT( i+1 ), MPFROMLONG( entry.Handle ));

        // Get the menu index of the default entry
        if ( entry.Handle == hDefault ) {
            sDefault = i;
            fDefault = TRUE;
        }
    }
    if ( !fDefault ) return BOOTMGR_MENU_NODEF;

    return ( sDefault );
}


/* ------------------------------------------------------------------------- *
 * BootMgrInstall()                                                          *
 *                                                                           *
 * Installs Boot Manager on the selected disk drive, as long as said disk    *
 * drive is either the first or second physical drive in the system.  Boot   *
 * Manager will be installed in the first available block of free space, and *
 * occupies one cylinder of disk space.  (It occupies a primary partition.)  *
 * Called from the main application's window procedure.                      *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwnd: main application client window                         *
 *   CARDINAL32 disk: numberof the selected disk drive                       *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if Boot Manager was installed successfully, FALSE otherwise        *
 * ------------------------------------------------------------------------- */
BOOL BootMgrInstall( HWND hwnd, CARDINAL32 disk )
{
    PDVMGLOBAL pGlobal;                    // global application data
    CARDINAL32 rc;                         // return code from LVM
    CHAR       szRes1[ STRING_RES_MAXZ ],  // string resource buffers
               szRes2[ STRING_RES_MAXZ ],
               szRes3[ STRING_RES_MAXZ ];


    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( !pGlobal ) return FALSE;

    if ( pGlobal->fsEngine & FS_ENGINE_BOOTMGR ) return FALSE;

    if ( disk > 2 ) {
        // Boot Manager can't be installed on any disk past the second
        WinLoadString( pGlobal->hab, pGlobal->hmri, IDS_BOOTMGR_WRONG_DISK,
                       STRING_RES_MAXZ, szRes1 );
        WinLoadString( pGlobal->hab, pGlobal->hmri, IDS_BOOTMGR_WRONG_TITLE,
                       STRING_RES_MAXZ, szRes2 );
        WinMessageBox( HWND_DESKTOP, hwnd, szRes1, szRes2, 0,
                       MB_OK | MB_ERROR | MB_MOVEABLE );
        return FALSE;
    }

    // Prompt for confirmation
    WinLoadString( pGlobal->hab, pGlobal->hmri, IDS_BOOTMGR_INSTALL,
                   STRING_RES_MAXZ, szRes1 );
    WinLoadString( pGlobal->hab, pGlobal->hmri, IDS_BOOTMGR_INSTALL_TITLE,
                   STRING_RES_MAXZ, szRes2 );
    sprintf( szRes3, szRes1, disk );
    if ( WinMessageBox( HWND_DESKTOP, hwnd, szRes3, szRes2, 0,
                        MB_OKCANCEL | MB_QUERY | MB_MOVEABLE ) != MBID_OK )
        return FALSE;

    // TODO should show a warning if there are any disks >512 GB or if
    // a Windows Vista/7 partition is detected (how?) ...

    // Call LVM to install Boot Manager
    LvmInstallBootMgr( disk, &rc );
    if ( pGlobal->pLog ) {
        fprintf( pGlobal->pLog, "-------------------------------------------------------------------------------\n");
        fprintf( pGlobal->pLog, "Installing Boot Manager on disk: %u\nResult: %u\n", disk, rc );
    }
    if ( rc != LVM_ENGINE_NO_ERROR ) {
        PopupEngineError( NULL, rc, hwnd, pGlobal->hab, pGlobal->hmri );
        return FALSE;
    }

    // Update the current status
    pGlobal->fsEngine |= FS_ENGINE_BOOTMGR | FS_ENGINE_REFRESH;
    SetModified( hwnd, TRUE );

    // Make sure the menu state reflects that Boot Manager is now active
    WinSendMsg( pGlobal->hwndMenu, MM_SETITEMATTR,
                MPFROM2SHORT( ID_BM_INSTALL, TRUE ),
                MPFROM2SHORT( MIA_DISABLED, MIA_DISABLED ));
    WinSendMsg( pGlobal->hwndMenu, MM_SETITEMATTR,
                MPFROM2SHORT( ID_BM_REMOVE, TRUE ),
                MPFROM2SHORT( MIA_DISABLED, 0 ));
    WinSendMsg( pGlobal->hwndMenu, MM_SETITEMATTR,
                MPFROM2SHORT( ID_BM_OPTIONS, TRUE ),
                MPFROM2SHORT( MIA_DISABLED, 0 ));
    // The "bootable" menu items will be enabled by the item selection logic

    // The drive layout has changed, so refresh the LVM data
    LVM_Refresh( hwnd );

    return TRUE;
}


/* ------------------------------------------------------------------------- *
 * BootMgrRemove()                                                           *
 *                                                                           *
 * De-installs Boot Manager and deletes its partition.  All current Boot     *
 * Manager settings are lost.  Called from the main application's window     *
 * procedure.                                                                *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd: main application client window                               *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void BootMgrRemove( HWND hwnd )
{
    PDVMGLOBAL pGlobal;                    // global application data
    CARDINAL32 rc;                         // return code from LVM
    CHAR       szRes1[ STRING_RES_MAXZ ],  // string resource buffers
               szRes2[ STRING_RES_MAXZ ];


    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( !pGlobal ) return;

    // Prompt for confirmation
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_BOOTMGR_DELETE, STRING_RES_MAXZ, szRes1 );
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_BOOTMGR_DELETE_TITLE, STRING_RES_MAXZ, szRes2 );
    if ( WinMessageBox( HWND_DESKTOP, hwnd, szRes1, szRes2, 0,
                        MB_YESNO | MB_DEFBUTTON2 | MB_QUERY | MB_MOVEABLE )
         != MBID_YES )
    {
        return;
    }

    // Call LVM to delete Boot Manager
    LvmDeleteBootMgr( &rc );
    if ( pGlobal->pLog ) {
        fprintf( pGlobal->pLog, "-------------------------------------------------------------------------------\n");
        fprintf( pGlobal->pLog, "Deleting Boot Manager\nResult: %u\n", rc );
    }
    if ( rc != LVM_ENGINE_NO_ERROR ) {
        PopupEngineError( NULL, rc, hwnd, pGlobal->hab, pGlobal->hmri );
        return;
    }

    // Update the current status
    pGlobal->fsEngine &= ~FS_ENGINE_BOOTMGR;
    pGlobal->fsEngine |= FS_ENGINE_REFRESH;
    SetModified( hwnd, TRUE );

    // Make sure the menu state reflects that Boot Manager is not installed
    WinSendMsg( pGlobal->hwndMenu, MM_SETITEMATTR,
                MPFROM2SHORT( ID_BM_INSTALL, TRUE ),
                MPFROM2SHORT( MIA_DISABLED, 0 ));
    WinSendMsg( pGlobal->hwndMenu, MM_SETITEMATTR,
                MPFROM2SHORT( ID_BM_REMOVE, TRUE ),
                MPFROM2SHORT( MIA_DISABLED, MIA_DISABLED ));
    WinSendMsg( pGlobal->hwndMenu, MM_SETITEMATTR,
                MPFROM2SHORT( ID_BM_OPTIONS, TRUE ),
                MPFROM2SHORT( MIA_DISABLED, MIA_DISABLED ));
    // The "bootable" menu items will be disabled in the item selection logic

    // The drive layout has changed, so refresh the LVM data
    LVM_Refresh( hwnd );
}



