/*****************************************************************************
 ** LVMPM - disk.c                                                          **
 *****************************************************************************
 * Logic for managing the various disk-related commands.                     *
 *****************************************************************************
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
 * DiskRewriteMBR()                                                          *
 *                                                                           *
 * Forces an immediate rewrite of the Master Boot Record on the selected     *
 * disk (prompting for confirmation first).                                  *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd: application client window handle                             *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void DiskRewriteMBR( HWND hwnd )
{
    PDVMGLOBAL   pGlobal;
    WNDPARAMS    wp    = {0};                     // disk control window parameters
    PDVCTLDATA   pDisk = NULL;                    // disk control data
    HWND         hwndDisk;                        // handle of disk control
    UCHAR        szRes1[ STRING_RES_MAXZ ],       // string resource buffers
                 szRes2[ STRING_RES_MAXZ ],
                 szBuffer[ STRING_RES_MAXZ * 3 ];
    USHORT       cb;
    CARDINAL32   iNumber = 1,                     // LVM disk number (default to 1 for this operation)
                 iRC;


    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( !pGlobal ) return;

    // Get the hwnd of the currently-selected disk control
    hwndDisk = (HWND) WinSendMsg( pGlobal->hwndDisks,
                                  LLM_QUERYDISKEMPHASIS, 0,
                                  MPFROMSHORT( LDV_FS_SELECTED ));
    if ( !hwndDisk ) return;

    // Query the control data for the LVM disk number
    wp.fsStatus  = WPM_CTLDATA;
    wp.cbCtlData = sizeof( DVCTLDATA );
    pDisk = (PDVCTLDATA) calloc( 1, wp.cbCtlData );
    if ( pDisk ) {
        wp.pCtlData = (PVOID) pDisk;
        if ( (BOOL) WinSendMsg( hwndDisk, WM_QUERYWINDOWPARAMS, MPFROMP( &wp ), 0 ))
            iNumber = pDisk->number;
        free( pDisk );
    }

    // Generate the confirmation message
    cb = sizeof( szBuffer );
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_NEWMBR_TITLE, STRING_RES_MAXZ, szRes1 );
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_NEWMBR_CONFIRM, STRING_RES_MAXZ, szRes2 );
    sprintf( szBuffer, szRes2, iNumber );

    // If this is the first disk, include a warning about wiping 3rd-party boot loaders
    if ( iNumber == 1 ) {
        if ( pGlobal->fsEngine & FS_ENGINE_AIRBOOT )
            WinLoadString( pGlobal->hab, pGlobal->hmri,
                           IDS_NEWMBR_WARN_1, STRING_RES_MAXZ, szRes2 );
        else
            WinLoadString( pGlobal->hab, pGlobal->hmri,
                           IDS_NEWMBR_WARN_2, STRING_RES_MAXZ, szRes2 );
        strncat( szBuffer, szRes2, cb );
    }
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_PROCEED_CONFIRM, STRING_RES_MAXZ, szRes2 );
    strncat( szBuffer, szRes2, cb );
    strncat( szBuffer, "\r\n\n", cb );
    if ( WinMessageBox( HWND_DESKTOP, hwnd, szBuffer, szRes1, 0,
                        MB_YESNO | MB_WARNING | MB_MOVEABLE ) == MBID_YES )
    {
        LvmNewMBR( pGlobal->disks[ iNumber-1 ].handle, &iRC );
        if ( iRC != LVM_ENGINE_NO_ERROR )
            PopupEngineError( NULL, iRC, hwnd, pGlobal->hab, pGlobal->hmri );
        else {
            SetModified( hwnd, TRUE );
            pGlobal->fsEngine &= ~FS_ENGINE_AIRBOOT;
            MenuItemEnable( pGlobal->hwndMenu, NULLHANDLE, ID_AIRBOOT_REMOVE, FALSE );
            WinLoadString( pGlobal->hab, pGlobal->hmri,
                           IDS_SUCCESS_TITLE, STRING_RES_MAXZ, szRes1 );
            WinLoadString( pGlobal->hab, pGlobal->hmri,
                           IDS_NEWMBR_OK, STRING_RES_MAXZ, szRes2 );
            WinMessageBox( HWND_DESKTOP, hwnd, szRes2, szRes1, 0,
                           MB_OK | MB_INFORMATION | MB_MOVEABLE );
        }
    }
}


/* ------------------------------------------------------------------------- *
 * DiskRename()                                                              *
 * Open the disk name dialog to rename a disk drive.                         *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 * ------------------------------------------------------------------------- */
BOOL DiskRename( HWND hwnd )
{
    PDVMGLOBAL     pGlobal;         // global application data
    DVMDISKPARAMS  data    = {0};   // dialog parameters structure
    WNDPARAMS      wp      = {0};   // disk control window parameters
    PDVCTLDATA     pDisk   = NULL;  // disk control data
    HWND           hwndDisk;        // handle of disk control
    CARDINAL32     iNumber = 0,     // LVM disk number
                   iRC;             // LVM error indicator
    USHORT         usBtnID;
    BOOL           bRC     = FALSE;


    pGlobal = WinQueryWindowPtr( hwnd, 0 );

    // Get the hwnd of the currently-selected disk control
    hwndDisk = (HWND) WinSendMsg( pGlobal->hwndDisks,
                                  LLM_QUERYDISKEMPHASIS, 0,
                                  MPFROMSHORT( LDV_FS_SELECTED ));
    if ( !hwndDisk ) return FALSE;

    wp.fsStatus  = WPM_CTLDATA;
    wp.cbCtlData = sizeof( DVCTLDATA );
    pDisk = (PDVCTLDATA) calloc( 1, wp.cbCtlData );
    if ( pDisk ) {
        wp.pCtlData = (PVOID) pDisk;
        if ( (BOOL) WinSendMsg( hwndDisk, WM_QUERYWINDOWPARAMS, MPFROMP( &wp ), 0 ))
            iNumber = pDisk->number;
        free( pDisk );
    }
    if ( iNumber < 1 ) return FALSE;

    data.hab    = pGlobal->hab;
    data.hmri   = pGlobal->hmri;
    data.handle = pGlobal->disks[ iNumber-1 ].handle;
    strcpy( data.szName, pGlobal->disks[ iNumber-1 ].szName );
    strcpy( data.szFontDlgs, pGlobal->szFontDlgs );
    sprintf( data.achSerial, "%08X", pGlobal->disks[ iNumber-1 ].iSerial );
    data.fsProgram = pGlobal->fsProgram;
    data.fAccessible = !pGlobal->disks[ iNumber-1 ].fUnusable;

    usBtnID = WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) DiskNameDlgProc,
                         pGlobal->hmri, IDD_DISK_NAME, &data );
    if ( usBtnID != DID_OK )
        return FALSE;

    // Now set the disk name
    LvmSetName( data.handle, data.szName, &iRC );
    if ( iRC == LVM_ENGINE_NO_ERROR ) {
        SetModified( hwnd, TRUE );
        bRC = TRUE;
    }
    else
        PopupEngineError( NULL, iRC, hwnd, pGlobal->hab, pGlobal->hmri );

    return ( bRC );
}


/* ------------------------------------------------------------------------- *
 * DiskNameDlgProc()                                                         *
 *                                                                           *
 * Dialog procedure for the disk name dialog.                                *
 * See OS/2 PM reference for a description of input and output.              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY DiskNameDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    static PDVMDISKPARAMS pData;
    CHAR   szName[ DISK_NAME_SIZE+1 ];

    switch( msg ) {

        case WM_INITDLG:
            pData = (PDVMDISKPARAMS) mp2;
            if ( !pData ) {
                WinSendMsg( hwnd, WM_CLOSE, 0, 0 );
                break;
            }

            // Set up the dialog style
            g_pfnRecProc = WinSubclassWindow(
                             WinWindowFromID( hwnd, IDD_DIALOG_INSET ),
                             (pData->fsProgram & FS_APP_PMSTYLE)?
                                InsetBorderProc: OutlineBorderProc );

            // Set the dialog font
            if ( pData->szFontDlgs[ 0 ] )
                WinSetPresParam( hwnd, PP_FONTNAMESIZE,
                                 strlen( pData->szFontDlgs ) + 1,
                                 (PVOID) pData->szFontDlgs );

            // Set the current serial number and name
            WinSetDlgItemText( hwnd, IDD_DISK_SERIAL_FIELD, pData->achSerial );
            WinSetDlgItemText( hwnd, IDD_DISK_NAME_FIELD, pData->szName );

            // Display the dialog
            CentreWindow( hwnd, WinQueryWindow( hwnd, QW_OWNER ), SWP_SHOW | SWP_ACTIVATE );
            return (MRESULT) TRUE;


        case WM_COMMAND:
            switch ( SHORT1FROMMP( mp1 )) {
                case DID_OK:
                    WinQueryDlgItemText( hwnd, IDD_DISK_NAME_FIELD,
                                         (LONG) sizeof( szName ), szName );
                    strncpy( pData->szName, szName, DISK_NAME_SIZE );
                    break;

                case DID_CANCEL:
                    break;
            }
            break;


        default: break;
    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}



