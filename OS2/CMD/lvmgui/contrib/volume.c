/*****************************************************************************
 ** LVMPM - volume.c                                                        **
 *****************************************************************************
 * Logic for managing the various volume-related secondary dialogs.          *
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
 * VolumeCreate                                                              *
 *                                                                           *
 * Present the volume creation dialogs and respond to them accordingly.      *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwnd   : handle of the main program client window            *
 *   PDVMGLOBAL pGlobal: the main program's global data                      *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if a new volume was created, FALSE otherwise.                      *
 * ------------------------------------------------------------------------- */
BOOL VolumeCreate( HWND hwnd, PDVMGLOBAL pGlobal )
{
    Partition_Information_Record pir;
    DVMCREATEPARMS data = {0};
    USHORT         usBtnID;
    BOOL           bRC = FALSE;
    ULONG          i;
    CARDINAL32     iRC;

    if ( !pGlobal || !pGlobal->disks || !pGlobal->ulDisks )
        return FALSE;

    data.hab         = pGlobal->hab;
    data.hmri        = pGlobal->hmri;
    data.fsProgram   = pGlobal->fsProgram;
    data.fsEngine    = pGlobal->fsEngine;
    data.disks       = pGlobal->disks;
    data.ulDisks     = pGlobal->ulDisks;
    data.ctry        = pGlobal->ctry;
    data.fType       = 0;
    data.fBootable   = FALSE;
    data.cLetter     = '\0';
    data.pPartitions = NULL;
    data.ulNumber    = 0;
    strcpy( data.szFontDlgs, pGlobal->szFontDlgs );
    strcpy( data.szFontDisks, pGlobal->szFontDisks );
    VolumeDefaultName( data.szName, pGlobal );

    do {
        usBtnID = WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) VolumeCreate1WndProc,
                             pGlobal->hmri, IDD_VOLUME_CREATE_1, &data );
        if ( usBtnID != DID_OK )
            goto cleanup;
        usBtnID = WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) VolumeCreate2WndProc,
                             pGlobal->hmri, IDD_VOLUME_CREATE_2, &data );
    } while ( usBtnID == DID_PREVIOUS );
    if ( usBtnID != DID_OK )
        goto cleanup;

    // Get the partition(s) information.
    if ( data.ulNumber && data.pPartitions ) {
        for ( i = 0; i < data.ulNumber; i++ ) {
            pir = LvmGetPartitionInfo( data.pPartitions[ i ], &iRC );
            if ( pir.Partition_Type == FREE_SPACE_PARTITION ) {
                // Open the partition creation dialog
                bRC = PartitionCreate( hwnd, pGlobal, data.pPartitions + i, PARTITION_FLAG_VOLUME_FREESPACE );
                if ( !bRC ) goto cleanup;
            }
        }
        // Partitions are ready, now create the volume.
        LvmCreateVolume( data.szName,
                         (data.fType & VOLUME_TYPE_ADVANCED)? TRUE: FALSE,
                         data.fBootable,
                         data.cLetter,
                         0L,
                         data.ulNumber,
                         data.pPartitions,
                         &iRC );
        if ( pGlobal->pLog )
            Log_CreateVolume( pGlobal, data, iRC );
        if ( iRC == LVM_ENGINE_NO_ERROR ) {
            SetModified( hwnd, TRUE );
            bRC = TRUE;
        }
        else
            PopupEngineError( NULL, iRC, hwnd, pGlobal->hab, pGlobal->hmri );
    }
    else
        DebugBox("Internal program error: invalid data returned from dialog!");

cleanup:
    if ( data.pPartitions) free ( data.pPartitions );

    return bRC;
}


/* ------------------------------------------------------------------------- *
 * VolumeCreate1WndProc()                                                    *
 *                                                                           *
 * Dialog procedure for the first volume creation dialog.                    *
 * See OS/2 PM reference for a description of input and output.              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY VolumeCreate1WndProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    static PDVMCREATEPARMS pData;
    CHAR   szRes[ STRING_RES_MAXZ ];
    PSZ    pszItem;
    LONG   cch;
    USHORT usIdx;

    switch( msg ) {

        case WM_INITDLG:
            pData = (PDVMCREATEPARMS) mp2;
            if ( !pData ) {
                WinSendMsg( hwnd, WM_CLOSE, 0, 0 );
                break;
            }

            // Set the control text according to the current circumstances
            WinLoadString( pData->hab, pData->hmri,
                           ( pData->fsProgram & FS_APP_IBMTERMS ) ?
                             IDS_VOLUME_NEW_COMPATIBLE :
                             IDS_VOLUME_NEW_STANDARD,
                           STRING_RES_MAXZ, szRes );
            WinSetDlgItemText( hwnd, IDD_VOLUME_CREATE_STANDARD, szRes );
            WinLoadString( pData->hab, pData->hmri,
                           ( pData->fsProgram & FS_APP_IBMTERMS ) ?
                             IDS_VOLUME_NEW_LVM :
                             IDS_VOLUME_NEW_ADVANCED,
                           STRING_RES_MAXZ, szRes );
            WinSetDlgItemText( hwnd, IDD_VOLUME_CREATE_ADVANCED, szRes );

            if ( pData->ulNumber && pData->pPartitions ) {
                // Partition was preselected, so we won't need the second dialog
                // Change the Next button text
                WinLoadString( pData->hab, pData->hmri, IDS_UITEXT_CREATE,
                               STRING_RES_MAXZ, szRes );
                WinSetDlgItemText( hwnd, DID_OK, szRes );
                // Also change the window title to Convert
                WinLoadString( pData->hab, pData->hmri, IDS_PARTITION_CONVERT_TITLE,
                               STRING_RES_MAXZ, szRes );
                WinSetWindowText( hwnd, szRes );

                // If this is a logical partition, disable bootable option if no IBM Boot Manager
                if (( pData->fType & PARTITION_TYPE_LOGICAL ) &&
                    !( pData->fsEngine & FS_ENGINE_BOOTMGR ))
                {
                    WinShowWindow( WinWindowFromID( hwnd,
                                                    IDD_VOLUME_CREATE_BOOTABLE ),
                                   FALSE );
                }
            }

            if ( pData->fsEngine & FS_ENGINE_AIRBOOT ) {
                WinShowWindow( WinWindowFromID( hwnd,
                                                IDD_VOLUME_CREATE_BOOTABLE ),
                               FALSE );
            }
            else {
                WinLoadString( pData->hab, pData->hmri,
                               ( pData->fsEngine & FS_ENGINE_BOOTMGR ) ?
                                 IDS_VOLUME_NEW_BOOTABLE :
                                 IDS_VOLUME_NEW_STARTABLE,
                               STRING_RES_MAXZ, szRes );
                WinSetDlgItemText( hwnd, IDD_VOLUME_CREATE_BOOTABLE, szRes );
            }

            // Set up the borders
            g_pfnRecProc = WinSubclassWindow(
                             WinWindowFromID( hwnd, IDD_DIALOG_INSET ),
                             (pData->fsProgram & FS_APP_PMSTYLE)? InsetBorderProc: OutlineBorderProc );
            WinSubclassWindow( WinWindowFromID( hwnd, IDD_DIALOG_INSET2 ),
                               (pData->fsProgram & FS_APP_PMSTYLE)? InsetBorderProc: OutlineBorderProc );

            // Set the dialog font
            if ( pData->szFontDlgs[ 0 ] )
                WinSetPresParam( hwnd, PP_FONTNAMESIZE,
                                 strlen( pData->szFontDlgs ) + 1,
                                 (PVOID) pData->szFontDlgs );

            // Populate the drive letter list
            VolumePopulateLetters( WinWindowFromID( hwnd,
                                                    IDD_VOLUME_LETTER_LIST ),
                                   pData->hab, pData->hmri, 0 );

            // If we have pre-set options, populate them (not needed?)
            if ( pData->fType == VOLUME_TYPE_ADVANCED )
                WinSendDlgItemMsg( hwnd, IDD_VOLUME_CREATE_ADVANCED,
                                   BM_CLICK, MPFROMSHORT( TRUE ), 0 );
            else {
                WinSendDlgItemMsg( hwnd, IDD_VOLUME_CREATE_STANDARD,
                                   BM_CLICK, MPFROMSHORT( TRUE ), 0 );
                WinSendDlgItemMsg( hwnd, IDD_VOLUME_CREATE_BOOTABLE,
                                   BM_SETCHECK, MPFROMSHORT( pData->fBootable ),
                                   0 );
            }
            WinSetDlgItemText( hwnd, IDD_VOLUME_NAME_FIELD, pData->szName );

            // Display the dialog
            CentreWindow( hwnd, WinQueryWindow( hwnd, QW_OWNER ), SWP_SHOW | SWP_ACTIVATE );
            return (MRESULT) TRUE;


        case WM_COMMAND:
            switch ( SHORT1FROMMP( mp1 )) {
                case DID_OK:        // Next button

                    // Get the requested volume type
                    if ( WinQueryButtonCheckstate( hwnd,
                                                   IDD_VOLUME_CREATE_ADVANCED ))
                        pData->fType = VOLUME_TYPE_ADVANCED;
                    else {
                        pData->fType = VOLUME_TYPE_STANDARD;

                        // See if the volume is to be bootable/startable
                        if ( WinQueryButtonCheckstate( hwnd,
                                                       IDD_VOLUME_CREATE_BOOTABLE ))
                            pData->fBootable = TRUE;
                        else
                            pData->fBootable = FALSE;
                    }

                    // Get the volume name
                    cch = WinQueryDlgItemTextLength( hwnd, IDD_VOLUME_NAME_FIELD );
                    if ( cch ) {
                        WinQueryDlgItemText( hwnd, IDD_VOLUME_NAME_FIELD,
                                             (LONG) sizeof( pData->szName ), pData->szName );
                    }
                    // TODO else give an error

                    // Get the drive letter
                    usIdx = (USHORT) WinSendDlgItemMsg( hwnd,
                                                        IDD_VOLUME_LETTER_LIST,
                                                        LM_QUERYSELECTION,
                                                        MPFROMSHORT( LIT_FIRST ),
                                                        0 );
                    if ( usIdx != (USHORT)LIT_NONE ) {
                        cch =  WinQueryLboxItemTextLength(
                                 WinWindowFromID( hwnd, IDD_VOLUME_LETTER_LIST ),
                                 usIdx );
                        if (( pszItem = (PSZ) malloc( cch + 1 )) != NULL ) {
                            WinQueryLboxItemText( WinWindowFromID( hwnd, IDD_VOLUME_LETTER_LIST ),
                                                  usIdx, pszItem, cch );
                            WinLoadString( pData->hab, pData->hmri,
                                           IDS_LETTER_NONE,
                                           STRING_RES_MAXZ, szRes );
                            if ( strncmp( pszItem, szRes, cch ) == 0 )
                                pData->cLetter = '\0';
                            else {
                                WinLoadString( pData->hab, pData->hmri,
                                               IDS_LETTER_AUTO,
                                               STRING_RES_MAXZ, szRes );
                                if ( strncmp( pszItem, szRes, cch ) == 0 )
                                    pData->cLetter = '*';
                                else
                                    pData->cLetter = pszItem[ 0 ];
                            }
                            free( pszItem );
                        }
                    }

                    break;


                case DID_CANCEL:
                    break;
            }
            break;


        case WM_CONTROL:
            switch ( SHORT1FROMMP( mp1 )) {

                case IDD_VOLUME_CREATE_STANDARD:
                    if ( SHORT2FROMMP( mp1 ) == BN_CLICKED )
                        WinEnableControl( hwnd, IDD_VOLUME_CREATE_BOOTABLE,
                                          TRUE );
                    break;

                case IDD_VOLUME_CREATE_ADVANCED:
                    if ( SHORT2FROMMP( mp1 ) == BN_CLICKED )
                        WinEnableControl( hwnd, IDD_VOLUME_CREATE_BOOTABLE,
                                          FALSE );
                    break;
            }
            break;


        default: break;

    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * VolumeCreate2WndProc()                                                    *
 *                                                                           *
 * Dialog procedure for the second volume creation dialog.                   *
 * See OS/2 PM reference for a description of input and output.              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY VolumeCreate2WndProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    static PDVMCREATEPARMS pData = NULL;
    PDISKNOTIFY pNotify = NULL;            // disk notification message info
    WNDPARAMS   wndp = {0};
    PVCTLDATA   pvd  = {0};
    CHAR        szRes[ STRING_RES_MAXZ ],
                szSize[ 15 ],
                szBuffer[ STRING_RES_MAXZ ];
    PSWP        pswp, pswpOld;
    SWP         wp;
    POINTL      ptl;
    ULONG       cb, i;
    USHORT      fsMask;


    switch( msg ) {

        case WM_INITDLG:
            pData = (PDVMCREATEPARMS) mp2;
            if ( !pData ) {
                WinSendMsg( hwnd, WM_CLOSE, 0, 0 );
                break;
            }

            // Set the control text according to the current circumstances
            WinLoadString( pData->hab, pData->hmri,
                           ( pData->fType == VOLUME_TYPE_STANDARD ) ?
                             IDS_VOLUME_NEW_SELECT_ONE :
                             IDS_VOLUME_NEW_SELECT_SOME,
                           STRING_RES_MAXZ, szRes );
            WinSetDlgItemText( hwnd, IDD_VOLUME_CREATE_SELECT, szRes );

            // Set up the border
            g_pfnRecProc = WinSubclassWindow(
                             WinWindowFromID( hwnd, IDD_DIALOG_INSET ),
                             InsetBorderProc
                           );

            // Set the dialog fonts
            if ( pData->szFontDlgs[ 0 ] )
                WinSetPresParam( hwnd, PP_FONTNAMESIZE,
                                 strlen( pData->szFontDlgs ) + 1,
                                 (PVOID) pData->szFontDlgs );
            if ( pData->szFontDisks[ 0 ] )
                WinSetPresParam( WinWindowFromID( hwnd,
                                                  IDD_VOLUME_CREATE_LIST ),
                                 PP_FONTNAMESIZE,
                                 strlen( pData->szFontDisks ) + 1,
                                 (PVOID) pData->szFontDisks );

            // Populate the list of disks/partitions
            VolumePopulateDisks( WinWindowFromID( hwnd, IDD_VOLUME_CREATE_LIST ),
                                 pData );

            // Show/hide the advanced controls as necessary
            WinShowWindow( WinWindowFromID( hwnd, IDD_VOLUME_CREATE_CONTENTS ),
                           ( pData->fType == VOLUME_TYPE_ADVANCED ) ? TRUE : FALSE );
            WinShowWindow( WinWindowFromID( hwnd, IDD_VOLUME_CREATE_ADD ),
                           ( pData->fType == VOLUME_TYPE_ADVANCED ) ? TRUE : FALSE );
            WinShowWindow( WinWindowFromID( hwnd, IDD_VOLUME_CREATE_REMOVE ),
                           ( pData->fType == VOLUME_TYPE_ADVANCED ) ? TRUE : FALSE );

            // Set the dialog size
            if ( PrfQueryProfileSize( HINI_USERPROFILE, SZ_INI_APP,
                                      SZ_INI_KEY_SEL_SIZE, &cb ) &&
                 ( cb == sizeof( POINTL )))
            {
                PrfQueryProfileData( HINI_USERPROFILE, SZ_INI_APP,
                                     SZ_INI_KEY_SEL_SIZE, &ptl, &cb );
            }
            else {
                ptl.x = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN ) * 0.7;
                ptl.y = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN ) * 0.4;
            }
            WinSetWindowPos( hwnd, HWND_TOP, 0, 0, ptl.x, ptl.y, SWP_SIZE );

            // (Advanced) Disable the Create button until partitions are added
            if ( pData->fType == VOLUME_TYPE_ADVANCED )
                WinEnableControl( hwnd, DID_OK, FALSE );

            // Display the dialog
            CentreWindow( hwnd, WinQueryWindow( hwnd, QW_OWNER ), SWP_SHOW | SWP_ACTIVATE );
            return (MRESULT) FALSE;


        case WM_COMMAND:
            switch ( SHORT1FROMMP( mp1 )) {

                case DID_OK:        // Create button
                    // Populate the pPartitions array of selected partitions
                    if ( pData->fType == VOLUME_TYPE_ADVANCED ) {
                        // Get the handles from the listbox
                        pData->ulNumber = (ULONG) WinSendDlgItemMsg( hwnd, IDD_VOLUME_CREATE_CONTENTS, LM_QUERYITEMCOUNT, 0, 0 );
                        if ( pData->pPartitions ) free( pData->pPartitions );
                        pData->pPartitions = (PADDRESS) calloc( pData->ulNumber, sizeof( ADDRESS ));
                        if ( !pData->pPartitions ) break;
                        for ( i = 0; i < pData->ulNumber; i++ ) {
                            pData->pPartitions[ i ] = WinSendDlgItemMsg( hwnd, IDD_VOLUME_CREATE_CONTENTS, LM_QUERYITEMHANDLE, MPFROMSHORT( i ), 0L );
                        }
                    }
                    else {
                        // Find the currently-selected disk+partition
                        if ( GetSelectedPartition(
                                WinWindowFromID( hwnd, IDD_VOLUME_CREATE_LIST ), &pvd
                             ))
                        {
                            pData->ulNumber = 1;
                            pData->pPartitions = (PADDRESS) calloc( 1, sizeof( ADDRESS ));
                            if ( pData->pPartitions )
                                pData->pPartitions[ 0 ] = pvd.handle;
                        }
                        // else no partition selected - leave selection empty on return

                    }
                    break;

                // These can just fall through (it's the return code that counts)
                case DID_PREVIOUS:
                case DID_CANCEL:
                    break;

                case IDD_VOLUME_CREATE_ADD:
                    VolumeAddPartition( hwnd );
                    return (MRESULT) 0;

                case IDD_VOLUME_CREATE_REMOVE:
                    VolumeRemovePartition( hwnd );
                    return (MRESULT) 0;
            }
            break;


        case WM_CONTROL:
            switch( SHORT2FROMMP( mp1 )) {

                case LLN_EMPHASIS:
                    pNotify = (PDISKNOTIFY) mp2;
                    if ( pNotify->hwndPartition ) {

                        /* A partition was selected.  Enable or disable the
                         * "Add" button as appropriate, and set the status
                         * text for the current partition.
                         */

                        fsMask = (USHORT) WinSendMsg( pNotify->hwndPartition,
                                                      LPM_GETEMPHASIS, 0, 0 );
                        if ( fsMask & LPV_FS_SELECTED ) {

                            // Get the partition information from its control
                            wndp.fsStatus  = WPM_CTLDATA;
                            wndp.cbCtlData = sizeof( PVCTLDATA );
                            wndp.pCtlData  = &pvd;
                            WinSendMsg( pNotify->hwndPartition,
                                        WM_QUERYWINDOWPARAMS,
                                        MPFROMP( &wndp ), 0 );

                            //  Disable "Add" if partition is unavailable or already added
                            if ( pvd.fDisable || pvd.fInUse || VolumePartitionIsAdded( hwnd, pvd ))
                                WinEnableControl( hwnd, IDD_VOLUME_CREATE_ADD, FALSE );
                            else
                                WinEnableControl( hwnd, IDD_VOLUME_CREATE_ADD, TRUE );

                            nlsThousandsULong( szSize, pvd.ulSize,
                                               pData->ctry.cThousands );

                            // Set the status text
                            if ( pvd.bType == LPV_TYPE_FREE ) {
                                WinLoadString( pData->hab, pData->hmri,
                                               IDS_STATUS_FREESPACE_SHORT,
                                               STRING_RES_MAXZ, szRes );
                                sprintf( szBuffer, szRes, pvd.szName, szSize,
                                         (pData->fsProgram & FS_APP_IECSIZES) ?
                                         " MiB" : " MB");
                            }
                            else {
                                WinLoadString( pData->hab, pData->hmri,
                                               IDS_STATUS_PARTITION_SHORT,
                                               STRING_RES_MAXZ, szRes );
                                sprintf( szBuffer, szRes, pvd.szName, szSize,
                                         (pData->fsProgram & FS_APP_IECSIZES) ?
                                         " MiB" : " MB");
                                if ( pvd.fDisable )
                                    WinLoadString( pData->hab, pData->hmri,
                                                   IDS_STATUS_INUSE,
                                                   STRING_RES_MAXZ, szRes );
                                else if ( pvd.bType == LPV_TYPE_UNUSABLE )
                                    WinLoadString( pData->hab, pData->hmri,
                                                   IDS_STATUS_UNUSABLE,
                                                   STRING_RES_MAXZ, szRes );
                                else
                                    WinLoadString( pData->hab, pData->hmri,
                                                   IDS_STATUS_AVAILABLE,
                                                   STRING_RES_MAXZ, szRes );
                                strncat( szBuffer, szRes, STRING_RES_MAXZ-1 );
                            }
                            WinSetDlgItemText( hwnd, IDD_VOLUME_CREATE_STATUS,
                                               szBuffer );
                        }
                    }
                    return (MRESULT) 0;

                case LN_SELECT:
                    if ( (SHORT) WinSendMsg( (HWND) mp2,
                                             LM_QUERYSELECTION,
                                             MPFROMSHORT( LIT_FIRST ), 0 )
                        == LIT_NONE )
                        WinEnableControl( hwnd, IDD_VOLUME_CREATE_REMOVE, FALSE );
                    else
                        WinEnableControl( hwnd, IDD_VOLUME_CREATE_REMOVE, TRUE );
                    break;
            }
            break;


        case WM_DESTROY:
            // Save the window size
            if ( WinQueryWindowPos( hwnd, &wp )) {
                ptl.x = wp.cx;
                ptl.y = wp.cy;
                PrfWriteProfileData( HINI_USERPROFILE, SZ_INI_APP,
                                     SZ_INI_KEY_SEL_SIZE, &ptl, sizeof( POINTL ));
            }
            // Free the disk control memory
            WinSendMsg( WinWindowFromID( hwnd, IDD_VOLUME_CREATE_LIST ),
                        LLM_SETDISKS, 0, 0 );
            break;


        case WM_SIZE:
            if ( !pData ) break;
            VolumeCreate2Resize( hwnd, SHORT1FROMMP(mp2), SHORT2FROMMP(mp2),
                                 ( pData->fType == VOLUME_TYPE_ADVANCED ) ?
                                 TRUE : FALSE );
            break;


        case WM_WINDOWPOSCHANGED:
            pswp = PVOIDFROMMP( mp1 );
            pswpOld = pswp + 1;
            // WinDefDlgProc doesn't dispatch WM_SIZE, so we do it here.
            if ( pswp->fl & SWP_SIZE ) {
                WinSendMsg( hwnd, WM_SIZE, MPFROM2SHORT(pswpOld->cx,pswpOld->cy),
                            MPFROM2SHORT(pswp->cx,pswp->cy) );
            }
            break;

        default: break;

    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * VolumeCreate2Resize                                                       *
 *                                                                           *
 * Resize/reposition various controls on the second volume-creation dialog   *
 * in response to the dialog being resized.                                  *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND  hwnd  : handle of the dialog window                               *
 *   SHORT usW   : new dialog width                                          *
 *   SHORT usH   : new dialog height                                         *
 *   BOOL  fMulti: TRUE if the controls for multiple partitions are visible  *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void VolumeCreate2Resize( HWND hwnd, SHORT usW, SHORT usH, BOOL fMulti )
{
    SWP    swp;             // dimensions of dialog titlebar
    LONG   lBX, lBY;        // width (x and y) of dialog border
    POINTL aptl[ 2 ];

    // Get the size of the frame interior by subtracting the borders & titlebar
    WinQueryWindowPos( WinWindowFromID(hwnd, FID_TITLEBAR), &swp );
    if ( swp.cy > 0 ) usH -= swp.cy;
    lBX = WinQuerySysValue( HWND_DESKTOP, SV_CXSIZEBORDER );
    if ( lBX > 0 ) usW -= 2 * lBX;
    lBY = WinQuerySysValue( HWND_DESKTOP, SV_CYSIZEBORDER );
    if ( lBY > 0 ) usH -= 2 * lBY;

    // Now convert this size into dialog coordinates (easier to deal with)
    aptl[ 0 ].x = usW;
    aptl[ 0 ].y = usH;
    WinMapDlgPoints( hwnd, aptl, 1, FALSE );
    usW = aptl[ 0 ].x;
    usH = aptl[ 0 ].y;

/* Resize these components as follows (bl_x, bl_y, tl_x, tl_y):
 *   IDD_DIALOG_INSET         = 1, 18, width-1, height-2
 *   IDD_VOLUME_CREATE_SELECT = 5, height-24, width-5, (18)
 *   If creating a compatibility volume:
 *     IDD_VOLUME_CREATE_LIST     = 4, 36, width-3, height-32
 *     IDD_VOLUME_CREATE_CONTENTS = (hide)
 *     IDD_VOLUME_CREATE_ADD      = (hide)
 *     IDD_VOLUME_CREATE_REMOVE   = (hide)
 *     IDD_DIALOG_INSET2          = 4, 21, width-3, (12)
 *     IDD_VOLUME_CREATE_STATUS   = 5, 22, width-5, (10)
 *   If creating a LVM volume:
 *     IDD_VOLUME_CREATE_LIST     = 4, 36, width-110, height-32
 *     IDD_VOLUME_CREATE_CONTENTS = width-106, 21, (102), height-46
 *     IDD_VOLUME_CREATE_ADD      = width-106, height-42, (50), (13)
 *     IDD_VOLUME_CREATE_REMOVE   = width-54,  height-42, (50), (13)
 *     IDD_DIALOG_INSET2          = 4, 21, width-88, (12)
 *     IDD_VOLUME_CREATE_STATUS   = 5, 22, width-110, (10)
 *   DID_HELP                 = width-46, 2, (40), (13)
 */
    // Adjust each control's position and convert back to window coordinates
    aptl[ 0 ].x = 1;                            // left
    aptl[ 0 ].y = 18;                           // bottom
    aptl[ 1 ].x = usW - ( 2 * aptl[0].x );      // width
    aptl[ 1 ].y = usH - 2 - aptl[0].y;          // height
    WinMapDlgPoints( hwnd, aptl, 2, TRUE );
    WinSetWindowPos( WinWindowFromID( hwnd, IDD_DIALOG_INSET ),
                     NULLHANDLE, aptl[0].x + lBX, aptl[0].y + lBY,
                     aptl[1].x, aptl[1].y, SWP_SIZE | SWP_MOVE );
    aptl[ 0 ].x = 5;
    aptl[ 0 ].y = usH - 24;
    aptl[ 1 ].x = usW - ( 2 * aptl[0].x );
    aptl[ 1 ].y = 18;
    WinMapDlgPoints( hwnd, aptl, 2, TRUE );
    WinSetWindowPos( WinWindowFromID( hwnd, IDD_VOLUME_CREATE_SELECT ),
                     NULLHANDLE, aptl[0].x + lBX, aptl[0].y + lBY,
                     aptl[1].x, aptl[1].y, SWP_SIZE | SWP_MOVE );

    if ( fMulti ) {
        aptl[ 0 ].x = 4;
        aptl[ 0 ].y = 32;
        aptl[ 1 ].x = usW - 110 - aptl[0].x;
        aptl[ 1 ].y = usH - 30 - aptl[0].y;
        WinMapDlgPoints( hwnd, aptl, 2, TRUE );
        WinSetWindowPos( WinWindowFromID( hwnd, IDD_VOLUME_CREATE_LIST ),
                         NULLHANDLE, aptl[0].x + lBX, aptl[0].y + lBY,
                         aptl[1].x, aptl[1].y, SWP_SIZE | SWP_MOVE );
        aptl[ 0 ].x = usW - 106;
        aptl[ 0 ].y = 21;
        aptl[ 1 ].x = 102;
        aptl[ 1 ].y = usH - 46 - aptl[0].y;
        WinMapDlgPoints( hwnd, aptl, 2, TRUE );
        WinSetWindowPos( WinWindowFromID( hwnd, IDD_VOLUME_CREATE_CONTENTS ),
                         NULLHANDLE, aptl[0].x + lBX, aptl[0].y + lBY,
                         aptl[1].x, aptl[1].y, SWP_SIZE | SWP_MOVE );
        aptl[ 0 ].x = usW - 106;
        aptl[ 0 ].y = usH - 42;
        aptl[ 1 ].x = 50;
        aptl[ 1 ].y = 13;
        WinMapDlgPoints( hwnd, aptl, 2, TRUE );
        WinSetWindowPos( WinWindowFromID( hwnd, IDD_VOLUME_CREATE_ADD ),
                         NULLHANDLE, aptl[0].x + lBX, aptl[0].y + lBY,
                         aptl[1].x, aptl[1].y, SWP_SIZE | SWP_MOVE );
        aptl[ 0 ].x = usW - 54;
        aptl[ 0 ].y = usH - 42;
        aptl[ 1 ].x = 50;
        aptl[ 1 ].y = 13;
        WinMapDlgPoints( hwnd, aptl, 2, TRUE );
        WinSetWindowPos( WinWindowFromID( hwnd, IDD_VOLUME_CREATE_REMOVE ),
                         NULLHANDLE, aptl[0].x + lBX, aptl[0].y + lBY,
                         aptl[1].x, aptl[1].y, SWP_SIZE | SWP_MOVE );
#if 0
        aptl[ 0 ].x = 3;
        aptl[ 0 ].y = 20;
        aptl[ 1 ].x = usW - 86 - aptl[0].x;
        aptl[ 1 ].y = 12;
        WinMapDlgPoints( hwnd, aptl, 2, TRUE );
        WinSetWindowPos( WinWindowFromID( hwnd, IDD_DIALOG_INSET2 ),
                         NULLHANDLE, aptl[0].x + lBX, aptl[0].y + lBY,
                         aptl[1].x, aptl[1].y, SWP_SIZE | SWP_MOVE );
#endif
        aptl[ 0 ].x = 4;
        aptl[ 0 ].y = 21;
        aptl[ 1 ].x = usW - 110 - aptl[0].x;
        aptl[ 1 ].y = 10;
        WinMapDlgPoints( hwnd, aptl, 2, TRUE );
        WinSetWindowPos( WinWindowFromID( hwnd, IDD_VOLUME_CREATE_STATUS ),
                         NULLHANDLE, aptl[0].x + lBX, aptl[0].y + lBY,
                         aptl[1].x, aptl[1].y, SWP_SIZE | SWP_MOVE );
    }
    else {
        aptl[ 0 ].x = 4;
        aptl[ 0 ].y = 31;
        aptl[ 1 ].x = usW - 3 - aptl[0].x;
        aptl[ 1 ].y = usH - 30 - aptl[0].y;
        WinMapDlgPoints( hwnd, aptl, 2, TRUE );
        WinSetWindowPos( WinWindowFromID( hwnd, IDD_VOLUME_CREATE_LIST ),
                         NULLHANDLE, aptl[0].x + lBX, aptl[0].y + lBY,
                         aptl[1].x, aptl[1].y, SWP_SIZE | SWP_MOVE );
#if 0
        aptl[ 0 ].x = 4;
        aptl[ 0 ].y = 21;
        aptl[ 1 ].x = usW - 3 - aptl[0].x;
        aptl[ 1 ].y = 12;
        WinMapDlgPoints( hwnd, aptl, 2, TRUE );
        WinSetWindowPos( WinWindowFromID( hwnd, IDD_DIALOG_INSET2 ),
                         NULLHANDLE, aptl[0].x + lBX, aptl[0].y + lBY,
                         aptl[1].x, aptl[1].y, SWP_SIZE | SWP_MOVE );
#endif
        aptl[ 0 ].x = 4;
        aptl[ 0 ].y = 20;
        aptl[ 1 ].x = usW - 3 - aptl[0].x;
        aptl[ 1 ].y = 10;
        WinMapDlgPoints( hwnd, aptl, 2, TRUE );
        WinSetWindowPos( WinWindowFromID( hwnd, IDD_VOLUME_CREATE_STATUS ),
                         NULLHANDLE, aptl[0].x + lBX, aptl[0].y + lBY,
                         aptl[1].x, aptl[1].y, SWP_SIZE | SWP_MOVE );
    }

    aptl[ 0 ].x = usW - 42;
    aptl[ 0 ].y = 2;
    aptl[ 1 ].x = 40;
    aptl[ 1 ].y = 13;
    WinMapDlgPoints( hwnd, aptl, 2, TRUE );
    WinSetWindowPos( WinWindowFromID( hwnd, DID_HELP ),
                     NULLHANDLE, aptl[0].x + lBX, aptl[0].y + lBY,
                     aptl[1].x, aptl[1].y, SWP_SIZE | SWP_MOVE );

}


/* ------------------------------------------------------------------------- *
 * VolumePopulateLetters                                                     *
 *                                                                           *
 * Populates a listbox control with the list of available drive letters.     *
 * Drive letters which are currently in use by non-LVM-managed devices will  *
 * be marked as such in the list.                                            *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND    hwndLB: handle of the listbox control                           *
 *   HAB     hab   : program HAB                                             *
 *   HMODULE hmri  : handle of program resource module                       *
 *                                                                           *
 * RETURNS: USHORT                                                           *
 *   The index of the last item in the list.                                 *
 * ------------------------------------------------------------------------- */
SHORT VolumePopulateLetters( HWND hwndLB, HAB hab, HMODULE hmri, CHAR cActive )
{
    CHAR       cLetter,                             // drive letter value
               szItem[ STRING_MINI_MAXZ + 4 ],      // list item string
               szNone[ STRING_MINI_MAXZ ],          // string to use for "none"
               szAuto[ STRING_MINI_MAXZ ],          // string to use for "automatic"
               szCurrent[ STRING_MINI_MAXZ ],       // string to use for "automatic"
               szReserved[ STRING_MINI_MAXZ ];      // reserved indicator
    CARDINAL32 flAvailable,    // mask of all nominally-available drive letters
               flReserved,     // mask of letters being used by non-LVM assignments
               current,        // mask of current drive letter
               rc;
    SHORT      sIdx;

    WinLoadString( hab, hmri, IDS_LETTER_NONE,    STRING_MINI_MAXZ, szNone );
    WinLoadString( hab, hmri, IDS_LETTER_AUTO,    STRING_MINI_MAXZ, szAuto );
    WinLoadString( hab, hmri, IDS_LETTER_INUSE,   STRING_MINI_MAXZ, szReserved );
    WinLoadString( hab, hmri, IDS_LETTER_CURRENT, STRING_MINI_MAXZ, szCurrent );

    flAvailable = LvmAvailableDriveLetters( &rc );
    if ( rc != LVM_ENGINE_NO_ERROR ) return 0;
    flReserved = LvmReservedDriveLetters( &rc );
    if ( rc != LVM_ENGINE_NO_ERROR ) return 0;

    // Insert 'None' as the first item
    WinSendMsg( hwndLB, LM_INSERTITEM, MPFROMSHORT( 0 ), MPFROMP( szNone ));
    // Insert 'Automatic' as the second item
    sIdx = (SHORT) WinSendMsg( hwndLB, LM_INSERTITEM, MPFROMSHORT( LIT_END ), MPFROMP( szAuto ));

    // Insert all the available letters
    for ( cLetter = 'A'; cLetter <= 'Z'; cLetter++ ) {
        current = 0x1 << (cLetter - 'A');
        if ( flAvailable & current ) {
            if ( flReserved & current )
                sprintf( szItem, "%c  %s", cLetter, szReserved );
            else
                sprintf( szItem, "%c", cLetter );
            sIdx = (SHORT) WinSendMsg( hwndLB, LM_INSERTITEM,
                                       MPFROMSHORT( LIT_END ), MPFROMP( szItem ));
        }
        else if ( cActive && ( cLetter == cActive )) {
            sprintf( szItem, "%c  %s", cLetter, szCurrent );
            sIdx = (SHORT) WinSendMsg( hwndLB, LM_INSERTITEM,
                                       MPFROMSHORT( LIT_END ), MPFROMP( szItem ));
        }
    }

    // Select the first available letter by default (third item in list)
    if ( sIdx > 1 )
        WinSendMsg( hwndLB, LM_SELECTITEM, MPFROMSHORT( 2 ), MPFROMSHORT( TRUE ));

    return ( sIdx );
}


/* ------------------------------------------------------------------------- *
 * VolumeDefaultName()                                                       *
 *                                                                           *
 * This is a utility function which generates a 'default' volume name.  The  *
 * name generated is 'Volume #' where '#' is a number such that the volume   *
 * name is unique.                                                           *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PSZ pszName       : String buffer to receive the volume name generated. *
 *                       Should be at least VOLUME_NAME_SIZE+1 bytes.        *
 *   PDVMGLOBAL pGlobal: application global data                             *
 *                                                                           *
 * RETURNS: PSZ                                                              *
 *   A pointer to pszName.                                                   *
 * ------------------------------------------------------------------------- */
PSZ VolumeDefaultName( PSZ pszName, PDVMGLOBAL pGlobal )
{
    CHAR  szRes[ STRING_RES_MAXZ ];
    ULONG ulNumber;

    if ( !pszName ) return NULL;

    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_VOLUME_DEFAULT_NAME,
                   STRING_RES_MAXZ, szRes );
    ulNumber = 1;
    sprintf( pszName, szRes, ulNumber );
    while ( VolumeNameExists( pszName, pGlobal )) {
        ulNumber++;
        sprintf( pszName, szRes, ulNumber );
    }
    return pszName;
}


/* ------------------------------------------------------------------------- *
 * VolumeNameExists()                                                        *
 *                                                                           *
 * Detects if a proposed volume name is the same as the name of any          *
 * already-existing volume.                                                  *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PSZ        pszName: the volume name to be verified                      *
 *   PDVMGLOBAL pGlobal: application global data                             *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if the volume name already exists; FALSE otherwise.                *
 * ------------------------------------------------------------------------- */
BOOL VolumeNameExists( PSZ pszName, PDVMGLOBAL pGlobal )
{
    ULONG i;

    for ( i = 0; i < pGlobal->ulVolumes; i++ ) {
        if ( ! strncmp( pGlobal->volumes[ i ].szName,
                        pszName, VOLUME_NAME_SIZE ))
            return TRUE;
    }
    return FALSE;
}


/* ------------------------------------------------------------------------- *
 * VolumePopulateDisks()                                                     *
 *                                                                           *
 * This function populates the disks list control on the partition selection *
 * dialog.                                                                   *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND            hwndCtl: handle of the disk list control                *
 *   PDVMCREATEPARMS pData  : common data from the dialog window             *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void VolumePopulateDisks( HWND hwndCtl, PDVMCREATEPARMS pData )
{
    Partition_Information_Array  partitions;
    Volume_Information_Record    volume;    // Current volume information

    PDVCTLDATA      pDiskCtl;       // Array of disk control data structures
    PPVCTLDATA      pPartCtl;       // Array of partition control data structures
    HWND            hwndDisk;       // Handle of disk control
    HMODULE         hIconLib;       // Handle of DLL containing icon resources
    HPOINTER        hptrHDD,        // Icon for normal hard disks
                    hptrPRM;        // Icon for normal PRM disks
    CARDINAL32      rc;             // LVM return code
    ULONG           ulCount,        // Number of disks in the list
                    cb,
                    i, j,
                    ulAvP,          // No. of the first available partition
                    ulAvD;          // No. of the disk containing ulAvP


    // Find out how many disks to add (skip unusable/empty/memory disks)
    for ( i = 0, ulCount = 0; i < pData->ulDisks; i++ ) {
        if ( pData->disks[ i ].fUnusable ||
             /* pData->disks[ i ].fPRM || */
             pData->disks[ i ].fBigFloppy ||
             ( pData->disks[ i ].iSerial == SERIAL_MEMDISK &&
               ! strcmp( pData->disks[ i ].szName, STRING_MEMDISK )))
            continue;
        ulCount++;
    }

    // Add the disks
    cb = sizeof( DVCTLDATA );
    pDiskCtl = (PDVCTLDATA) calloc( ulCount, cb );
    if ( !pDiskCtl ) return;
    for ( i = 0, ulCount = 0; i < pData->ulDisks; i++ ) {

        // Skip past empty or unusable drives, or MemDisk
        if ( pData->disks[ i ].fUnusable ||
             /* pData->disks[ i ].fPRM || */
             pData->disks[ i ].fBigFloppy ||
             ( pData->disks[ i ].iSerial == SERIAL_MEMDISK &&
               ! strcmp( pData->disks[ i ].szName, STRING_MEMDISK )))
            continue;

        pDiskCtl[ ulCount ].cb = cb;
        pDiskCtl[ ulCount ].handle = pData->disks[ i ].handle;
        pDiskCtl[ ulCount ].number = pData->disks[ i ].iNumber;
        pDiskCtl[ ulCount ].ulSize = pData->disks[ i ].iSize;
        strncpy( pDiskCtl[ ulCount ].szName, pData->disks[ i ].szName,
                 DISK_NAME_SIZE );
        nlsThousandsULong( pDiskCtl[ ulCount ].szSize,
                           pData->disks[ i ].iSize, pData->ctry.cThousands );
        strncat( pDiskCtl[ ulCount ].szSize,
                 ( pData->fsProgram & FS_APP_IECSIZES ) ? " MiB" : " MB",
                 SIZE_TEXT_LIMIT );
        ulCount++;
    }
    WinSendMsg( hwndCtl, LLM_SETDISKS,
                MPFROMLONG( ulCount ), MPFROMP( pDiskCtl ));


    // Now set the info for each disk

    rc = (APIRET) DosQueryModuleHandle("PMWP.DLL", &hIconLib );
    if ( rc == NO_ERROR ) {
        hptrHDD     = WinLoadPointer( HWND_DESKTOP, hIconLib, 13 );
        hptrPRM     = WinLoadPointer( HWND_DESKTOP, hIconLib, 95 );
    }
    else {
        hptrHDD = WinLoadPointer( HWND_DESKTOP, pData->hmri, IDP_HDD );
        hptrPRM = WinLoadPointer( HWND_DESKTOP, pData->hmri, IDP_PRM );
    }

    ulAvP = 0;
    ulAvD = 0;
    for ( i = 0, ulCount = 0; i < pData->ulDisks; i++ ) {

        // Skip past empty or unusable drives, or MemDisk
        if ( pData->disks[ i ].fUnusable ||
             pData->disks[ i ].fPRM || pData->disks[ i ].fBigFloppy ||
             ( pData->disks[ i ].iSerial == SERIAL_MEMDISK &&
               ! strcmp( pData->disks[ i ].szName, STRING_MEMDISK )))
            continue;

        hwndDisk = (HWND) WinSendMsg( hwndCtl, LLM_QUERYDISKHWND,
                                      MPFROMSHORT( ulCount ),
                                      MPFROMSHORT( TRUE ));
        if ( !hwndDisk ) continue;
        ulCount++;

        // Set the correct disk icon
        WinSendMsg( hwndDisk, LDM_SETDISKICON,
                    MPFROMP( pData->disks[ i ].fPRM ? hptrPRM : hptrHDD ), 0 );

        if ( pData->disks[ i ].fUnusable ) continue;

        // Set the partitions for the disk

        partitions = LvmGetPartitions( pData->disks[ i ].handle, &rc );
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
            pPartCtl[ j ].disk    = pData->disks[ i ].iNumber;
            pPartCtl[ j ].handle  = partitions.Partition_Array[ j ].Partition_Handle;
            strncpy( pPartCtl[ j ].szName,
                     partitions.Partition_Array[ j ].Partition_Name,
                     PARTITION_NAME_SIZE );

            // Mark in-use partitions as unavailable.
            if ( partitions.Partition_Array[ j ].Partition_Status == PARTITION_IS_IN_USE )
                pPartCtl[ j ].fDisable = TRUE;
            else if ( !ulAvP ) {
                ulAvD = pData->disks[ i ].iNumber;
                ulAvP = j + 1;
            }

            // Get the actual (not configured) drive letter
            if ( partitions.Partition_Array[ j ].Volume_Handle ) {
                volume = LvmGetVolumeInfo( partitions.Partition_Array[ j ].Volume_Handle, &rc );
                pPartCtl[ j ].cLetter = rc ? '\0' : volume.Current_Drive_Letter;
            }
            else
                pPartCtl[ j ].cLetter = '\0';
        }
        WinSendMsg( hwndDisk, LDM_SETPARTITIONS,
                    MPFROMLONG( partitions.Count ), MPFROMP( pPartCtl ));
        free( pPartCtl );
        LvmFreeMem( partitions.Partition_Array );
    }

    free( pDiskCtl );

    // Set selection emphasis on the first available partition, if possible
    if ( ulCount ) {
        hwndDisk = (HWND) WinSendMsg( hwndCtl, LLM_QUERYDISKHWND,
                                      MPFROMLONG( ulAvD-1 ), MPFROMSHORT( TRUE ));
//      WinSendMsg( hwndCtl, LLM_SETDISKEMPHASIS,
//                  MPFROMP( hwndDisk ), MPFROM2SHORT( TRUE, LDV_FS_SELECTED ));
        if ( ulAvP ) {
            HWND hwndPart;
            hwndPart = (HWND) WinSendMsg( hwndDisk, LDM_QUERYPARTITIONHWND,
                                          MPFROMLONG( ulAvP-1 ),
                                          MPFROMSHORT( TRUE ));
            if ( hwndPart )
                WinSendMsg( hwndDisk, LDM_SETEMPHASIS, MPFROMP( hwndPart ),
                            MPFROM2SHORT( TRUE, LPV_FS_SELECTED ));
        }
    }
}


/* ------------------------------------------------------------------------- *
 * VolumeAddPartition()                                                      *
 *                                                                           *
 * This function adds the selected partition to the graphical list of        *
 * partitions to be added to the volume.                                     *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd: handle of the current dialog                                 *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void VolumeAddPartition( HWND hwnd )
{
    HWND      hwndDisks,    // graphical disk list control
              hwndParts,    // listbox showing currently-added partitions
              hwndD,        // currently-selected disk control
              hwndP;        // currently-selected partition control
    WNDPARAMS wp  = {0};    // used to query partition control data
    PVCTLDATA pvd = {0};    // control data of selected partition control
    SHORT     sIdx;

    hwndDisks = WinWindowFromID( hwnd, IDD_VOLUME_CREATE_LIST );
    hwndParts = WinWindowFromID( hwnd, IDD_VOLUME_CREATE_CONTENTS );

    // Get the currently-selected partition control
    hwndD = (HWND) WinSendMsg( hwndDisks, LLM_QUERYDISKEMPHASIS, 0,
                               MPFROMSHORT( LDV_FS_SELECTED ));
    if ( !hwndD ) return;
    hwndP = (HWND) WinSendMsg( hwndD, LDM_QUERYLASTSELECTED,
                               MPFROMLONG( TRUE ), 0 );
    if ( !hwndP ) return;

    // Get the details of the indicated partition
    wp.fsStatus  = WPM_CTLDATA;
    wp.cbCtlData = sizeof( PVCTLDATA );
    wp.pCtlData  = &pvd;
    if ( ! (BOOL) WinSendMsg( hwndP, WM_QUERYWINDOWPARAMS, MPFROMP( &wp ), 0 ))
        return;

    sIdx = (SHORT) WinSendMsg( hwndParts, LM_INSERTITEM,
                               MPFROMSHORT( LIT_END ), MPFROMP( pvd.szName ));
    if (( sIdx == LIT_MEMERROR ) || ( sIdx == LIT_ERROR ))
        return;
    WinSendMsg( hwndParts, LM_SETITEMHANDLE,
                MPFROMSHORT( sIdx ), MPFROMP( pvd.handle ));

    WinSendMsg( hwndP, LPM_SETEMPHASIS,
                MPFROMSHORT( TRUE ), MPFROMSHORT( LPV_FS_ACTIVE ));

    WinEnableControl( hwnd, IDD_VOLUME_CREATE_ADD, FALSE );

    WinEnableControl( hwnd, DID_OK, TRUE );
}


/* ------------------------------------------------------------------------- *
 * VolumePartitionIsAdded()                                                  *
 *                                                                           *
 * This function checks to see if the selected partition is currently in the *
 * graphical list of partitions to be added to the volume.                   *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND      hwndCtl : handle of the disk list control                     *
 *   PVCTLDATA partinfo: information about the selected partition control    *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if the partition was found in the list, FALSE otherwise.           *
 * ------------------------------------------------------------------------- */
BOOL VolumePartitionIsAdded( HWND hwnd, PVCTLDATA partinfo )
{
    HWND    hwndParts;          // listbox of currently-added partitions
    ADDRESS handle;             // LVM handle of the current partition
    USHORT  sCount, sIdx;       // listbox count and index
    BOOL    fFound = FALSE;     // was partition found in list?

    hwndParts = WinWindowFromID( hwnd, IDD_VOLUME_CREATE_CONTENTS );
    sCount = (SHORT) WinSendMsg( hwndParts, LM_QUERYITEMCOUNT, 0, 0 );
    for ( sIdx = 0; sIdx < sCount; sIdx++ ) {
        handle = (ADDRESS) WinSendMsg( hwndParts, LM_QUERYITEMHANDLE,
                                       MPFROMSHORT( sIdx ), 0 );
        if ( handle == partinfo.handle )
            fFound = TRUE;
    }

    return fFound;
}


/* ------------------------------------------------------------------------- *
 * VolumeRemovePartition()                                                   *
 *                                                                           *
 * This function removes the indicated partition from the graphical list of  *
 * partitions to be added to the volume.                                     *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd: handle of the current dialog                                 *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void VolumeRemovePartition( HWND hwnd )
{
    HWND      hwndParts,    // listbox of currently-added partitions
              hwndD,        // currently-selected disk control
              hwndP;        // currently-selected partition control
    ADDRESS   handle;       // LVM handle of the current partition
    WNDPARAMS wp  = {0};    // used to query partition control data
    PVCTLDATA pvd = {0};    // control data of selected partition control
    SHORT     sIdx;

    hwndParts = WinWindowFromID( hwnd, IDD_VOLUME_CREATE_CONTENTS );
    sIdx = (SHORT) WinSendMsg( hwndParts, LM_QUERYSELECTION,
                               MPFROMSHORT( LIT_FIRST ), 0 );
    if ( sIdx == LIT_NONE )
        return;
    // Save the handle for the check below
    handle = (ADDRESS) WinSendMsg( hwndParts, LM_QUERYITEMHANDLE,
                                   MPFROMSHORT( sIdx ), 0 );
    // Remove the partition and disable the "Remove" button
    WinSendMsg( hwndParts, LM_DELETEITEM, MPFROMSHORT( sIdx ), 0 );
    WinEnableControl( hwnd, IDD_VOLUME_CREATE_REMOVE, FALSE );

    /* We now have force a recheck of the proper "Add" button state,
     * in case the removed partition is currently selected (and thus
     * the proper button state has changed without a selection
     * event to trigger the usual check).
     */

    // Get the currently-selected partition control
    hwndD = (HWND) WinSendDlgItemMsg( hwnd, IDD_VOLUME_CREATE_LIST,
                                     LLM_QUERYDISKEMPHASIS, 0,
                                     MPFROMSHORT( LDV_FS_SELECTED ));
    if ( !hwndD ) return;
    hwndP = (HWND) WinSendMsg( hwndD, LDM_QUERYLASTSELECTED,
                               MPFROMLONG( TRUE ), 0 );
    if ( !hwndP ) return;

    // Get the details of the indicated partition
    wp.fsStatus  = WPM_CTLDATA;
    wp.cbCtlData = sizeof( PVCTLDATA );
    wp.pCtlData  = &pvd;
    if ( ! (BOOL) WinSendMsg( hwndP, WM_QUERYWINDOWPARAMS, MPFROMP( &wp ), 0 ))
        return;

    WinSendMsg( hwndP, LPM_SETEMPHASIS,
                MPFROMSHORT( FALSE ), MPFROMSHORT( LPV_FS_ACTIVE ));

    // Re-enable "Add" if selected partition is the one we just removed
    if ( pvd.handle == handle )
        WinEnableControl( hwnd, IDD_VOLUME_CREATE_ADD, TRUE );

    if ( WinSendMsg( hwndParts, LM_QUERYITEMCOUNT, 0, 0 ) == 0 )
        WinEnableControl( hwnd, DID_OK, FALSE );
}


/* ------------------------------------------------------------------------- *
 * VolumeDelete                                                              *
 *                                                                           *
 * Delete the currently-selected volume.                                     *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwnd   : handle of the main program client window            *
 *   PDVMGLOBAL pGlobal: the main program's global data                      *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if the volume was deleted, FALSE otherwise.                        *
 * ------------------------------------------------------------------------- */
BOOL VolumeDelete( HWND hwnd, PDVMGLOBAL pGlobal )
{
    PDVMVOLUMERECORD pVolRec;               // Selected volume container record
    UCHAR            szRes1[ STRING_RES_MAXZ ],
                     szRes2[ STRING_RES_MAXZ ],
                     szBuffer[ STRING_RES_MAXZ + VOLUME_NAME_SIZE + 1 ];
    BOOL             bRC = FALSE;
    CARDINAL32       iRC;

    if ( !pGlobal || !pGlobal->volumes || !pGlobal->ulVolumes )
        return FALSE;

    // Get the selected volume
    pVolRec = WinSendDlgItemMsg( pGlobal->hwndVolumes, IDD_VOLUMES,
                                 CM_QUERYRECORDEMPHASIS,
                                 MPFROMP( CMA_FIRST ),
                                 MPFROMSHORT( CRA_SELECTED ));
    if ( !pVolRec )
        return FALSE;

    // Generate the confirmation message
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_VOLUME_DELETE_TITLE, STRING_RES_MAXZ, szRes1 );
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_VOLUME_DELETE_CONFIRM, STRING_RES_MAXZ, szRes2 );
    sprintf( szBuffer, szRes2, pVolRec->pszLetter, pVolRec->pszName );

    if ( WinMessageBox( HWND_DESKTOP, hwnd, szBuffer, szRes1, 0,
                        MB_YESNO | MB_WARNING | MB_MOVEABLE ) == MBID_YES )
    {
        LvmDeleteVolume( pVolRec->handle, &iRC );
        if ( pGlobal->pLog ) {
            fprintf( pGlobal->pLog, "-------------------------------------------------------------------------------\n");
            fprintf( pGlobal->pLog, "Deleting volume: 0x%08X\n", pVolRec->handle );
            fprintf( pGlobal->pLog, "Result: %u\n", iRC );
        }
        if ( iRC != LVM_ENGINE_NO_ERROR )
            PopupEngineError( NULL, iRC, hwnd, pGlobal->hab, pGlobal->hmri );
        else {
            SetModified( hwnd, TRUE );
            bRC = TRUE;
        }
    }

    return bRC;
}


/* ------------------------------------------------------------------------- *
 * VolumeRename                                                              *
 *                                                                           *
 * Present the volume name dialog and respond accordingly.                   *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwnd   : handle of the main program client window            *
 *   PDVMGLOBAL pGlobal: the main program's global data                      *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if the volume was deleted, FALSE otherwise.                        *
 * ------------------------------------------------------------------------- */
BOOL VolumeRename( HWND hwnd, PDVMGLOBAL pGlobal )
{
    PDVMVOLUMERECORD pVolRec;               // Selected volume container record
    DVMNAMEPARAMS    data;                  // Window data for rename dialog
    PLVMVOLUMEINFO   pVolume;               // Pointer to the current volume info
    USHORT           usBtnID;
    BOOL             bRC = FALSE;
    CARDINAL32       iRC;

    if ( !pGlobal || !pGlobal->volumes || !pGlobal->ulVolumes )
        return FALSE;

    // Get the selected volume
    pVolRec = WinSendDlgItemMsg( pGlobal->hwndVolumes, IDD_VOLUMES,
                                 CM_QUERYRECORDEMPHASIS,
                                 MPFROMP( CMA_FIRST ),
                                 MPFROMSHORT( CRA_SELECTED ));
    if ( !pVolRec )
        return FALSE;
    if ( pVolRec->ulVolume > pGlobal->ulVolumes )
        return FALSE;
    pVolume = pGlobal->volumes + pVolRec->ulVolume;

    data.hab       = pGlobal->hab;
    data.hmri      = pGlobal->hmri;
    data.handle    = pVolRec->handle;
    data.fVolume   = TRUE;
    data.cLetter   = pVolume->cLetter;
    data.fsProgram = pGlobal->fsProgram;
    strcpy( data.szFontDlgs, pGlobal->szFontDlgs );
    strncpy( data.szName, pVolume->szName, VOLUME_NAME_SIZE );
    strncpy( data.szFS, pVolume->szFS, FILESYSTEM_NAME_SIZE );

    usBtnID = WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) VolumePartitionNameDlgProc,
                         pGlobal->hmri, IDD_VOLUME_NAME, &data );
    if ( usBtnID != DID_OK )
        return FALSE;

    // Now set the disk name
    LvmSetName( data.handle, data.szName, &iRC );
    if ( pGlobal->pLog ) {
        fprintf( pGlobal->pLog, "-------------------------------------------------------------------------------\n");
        fprintf( pGlobal->pLog, "Renaming volume: 0x%08X\n", data.handle );
        fprintf( pGlobal->pLog, "New name: %s\n", data.szName );
        fprintf( pGlobal->pLog, "Result:   %u\n", iRC );
    }
    if ( iRC == LVM_ENGINE_NO_ERROR ) {
        SetModified( hwnd, TRUE );
        bRC = TRUE;
    }
    else
        PopupEngineError( NULL, iRC, hwnd, pGlobal->hab, pGlobal->hmri );

    return ( bRC );
}


/* ------------------------------------------------------------------------- *
 * VolumePartitionNameDlgProc()                                              *
 *                                                                           *
 * Dialog procedure for the volume/partition name dialog.                    *
 * See OS/2 PM reference for a description of input and output.              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY VolumePartitionNameDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    static PDVMNAMEPARAMS pData;
    CHAR   szRes[ STRING_RES_MAXZ ];

    switch( msg ) {

        case WM_INITDLG:
            pData = (PDVMNAMEPARAMS) mp2;
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

            // Set the dialog text and contents
            WinLoadString( pData->hab, pData->hmri,
                           pData->fVolume? IDS_VOLUME_NAME_TITLE :
                                           IDS_PARTITION_NAME_TITLE,
                           STRING_RES_MAXZ, szRes );
            WinSetWindowText( hwnd, szRes );
            WinLoadString( pData->hab, pData->hmri,
                           pData->fVolume? IDS_VOLUME_NAME_PROMPT :
                                           IDS_PARTITION_NAME_PROMPT,
                           STRING_RES_MAXZ, szRes );
            WinSetDlgItemText( hwnd, IDD_VOLUME_NAME_INTRO, szRes );
            WinSetDlgItemText( hwnd, IDD_VOLUME_NAME_FIELD, pData->szName );

            // Display the dialog
            CentreWindow( hwnd, WinQueryWindow( hwnd, QW_OWNER ), SWP_SHOW | SWP_ACTIVATE );
            return (MRESULT) TRUE;


        case WM_COMMAND:
            switch ( SHORT1FROMMP( mp1 )) {
                case DID_OK:
                    WinQueryDlgItemText( hwnd, IDD_VOLUME_NAME_FIELD,
                                         (LONG) sizeof( pData->szName ), pData->szName );
                    break;

                case DID_CANCEL:
                    break;
            }
            break;


        default: break;
    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * VolumeSetLetter                                                           *
 *                                                                           *
 * Present the volume drive letter dialog and respond accordingly.           *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwnd   : handle of the main program client window            *
 *   PDVMGLOBAL pGlobal: the main program's global data                      *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if the volume was deleted, FALSE otherwise.                        *
 * ------------------------------------------------------------------------- */
BOOL VolumeSetLetter( HWND hwnd, PDVMGLOBAL pGlobal )
{
    PDVMVOLUMERECORD pVolRec;               // Selected volume container record
    DVMNAMEPARAMS    data;                  // Window data for rename dialog
    PLVMVOLUMEINFO   pVolume;               // Pointer to the current volume info
    USHORT           usBtnID;
    BOOL             bRC = FALSE;
    CARDINAL32       iRC;

    if ( !pGlobal || !pGlobal->volumes || !pGlobal->ulVolumes )
        return FALSE;

    // Get the selected volume
    pVolRec = WinSendDlgItemMsg( pGlobal->hwndVolumes, IDD_VOLUMES,
                                 CM_QUERYRECORDEMPHASIS,
                                 MPFROMP( CMA_FIRST ),
                                 MPFROMSHORT( CRA_SELECTED ));
    if ( !pVolRec )
        return FALSE;
    if ( pVolRec->ulVolume > pGlobal->ulVolumes )
        return FALSE;
    pVolume = pGlobal->volumes + pVolRec->ulVolume;

    data.hab       = pGlobal->hab;
    data.hmri      = pGlobal->hmri;
    data.handle    = pVolRec->handle;
    data.fVolume   = TRUE;
    data.cLetter   = pVolume->cPreference;
    data.fsProgram = pGlobal->fsProgram;
    strcpy( data.szFontDlgs, pGlobal->szFontDlgs );
    strncpy( data.szName, pVolume->szName, VOLUME_NAME_SIZE );
    strncpy( data.szFS, pVolume->szFS, FILESYSTEM_NAME_SIZE );

    usBtnID = WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) VolumeLetterDlgProc,
                         pGlobal->hmri, IDD_VOLUME_LETTER, &data );
    if ( usBtnID != DID_OK )
        return FALSE;

    // Now set the disk name
    LvmSetDriveLetter( data.handle, data.cLetter, &iRC );
    if ( pGlobal->pLog ) {
        fprintf( pGlobal->pLog, "-------------------------------------------------------------------------------\n");
        fprintf( pGlobal->pLog, "Changing volume letter: 0x%08X\n", data.handle );
        fprintf( pGlobal->pLog, "New letter: %c\n", data.cLetter );
        fprintf( pGlobal->pLog, "Result:     %u\n", iRC );
    }
    if ( iRC == LVM_ENGINE_NO_ERROR ) {
        SetModified( hwnd, TRUE );
        bRC = TRUE;
    }
    else
        PopupEngineError( NULL, iRC, hwnd, pGlobal->hab, pGlobal->hmri );

    return ( bRC );
}


/* ------------------------------------------------------------------------- *
 * VolumeLetterDlgProc()                                                     *
 *                                                                           *
 * Dialog procedure for the volume drive letter dialog.                      *
 * See OS/2 PM reference for a description of input and output.              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY VolumeLetterDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    static PDVMNAMEPARAMS pData;
    CHAR   szRes[ STRING_RES_MAXZ ],
           szPrefix[ 2 ];
    PSZ    pszItem;
    LONG   cch;
    SHORT  sIdx;

    switch( msg ) {

        case WM_INITDLG:
            pData = (PDVMNAMEPARAMS) mp2;
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

            // Populate the drive letter list
            VolumePopulateLetters( WinWindowFromID( hwnd,
                                                    IDD_VOLUME_LETTER_LIST ),
                                                    pData->hab, pData->hmri,
                                                    pData->cLetter );

            // Select the current drive letter
            if ( pData->cLetter ) {
                if ( pData->cLetter == '*')
                    sIdx = 1;
                else {
                    szPrefix[ 0 ] = pData->cLetter;
                    szPrefix[ 1 ] = 0;
                    sIdx = (SHORT) WinSendDlgItemMsg( hwnd, IDD_VOLUME_LETTER_LIST,
                                                      LM_SEARCHSTRING,
                                                      MPFROM2SHORT( LSS_CASESENSITIVE | LSS_PREFIX, 1 ),
                                                      MPFROMP( szPrefix ));
                }
            }
            else sIdx = 0;
            WinSendDlgItemMsg( hwnd, IDD_VOLUME_LETTER_LIST,
                               LM_SELECTITEM, MPFROMSHORT( sIdx ),
                               MPFROMSHORT( TRUE ));

            // Display the dialog
            CentreWindow( hwnd, WinQueryWindow( hwnd, QW_OWNER ), SWP_SHOW | SWP_ACTIVATE );
            return (MRESULT) TRUE;


        case WM_COMMAND:
            switch ( SHORT1FROMMP( mp1 )) {
                case DID_OK:
                    // Get the drive letter
                    sIdx = (SHORT) WinSendDlgItemMsg( hwnd, IDD_VOLUME_LETTER_LIST,
                                                       LM_QUERYSELECTION,
                                                       MPFROMSHORT( LIT_FIRST ), 0 );
                    if ( sIdx != LIT_NONE ) {
                        cch = WinQueryLboxItemTextLength(
                                  WinWindowFromID( hwnd, IDD_VOLUME_LETTER_LIST ),
                                  sIdx );
                        if (( pszItem = (PSZ) malloc( cch + 1 )) != NULL ) {
                            WinQueryLboxItemText( WinWindowFromID( hwnd, IDD_VOLUME_LETTER_LIST ),
                                                  sIdx, pszItem, cch );
                            WinLoadString( pData->hab, pData->hmri,
                                           IDS_LETTER_NONE,
                                           STRING_RES_MAXZ, szRes );
                            if ( strncmp( pszItem, szRes, cch ) == 0 )
                                pData->cLetter = '\0';
                            else {
                                WinLoadString( pData->hab, pData->hmri,
                                               IDS_LETTER_AUTO,
                                               STRING_RES_MAXZ, szRes );
                                if ( strncmp( pszItem, szRes, cch ) == 0 )
                                    pData->cLetter = '*';
                                else
                                    pData->cLetter = pszItem[ 0 ];
                            }
                            free( pszItem );
                        }
                    }
                    break;

                case DID_CANCEL:
                    break;
            }
            break;


        default: break;
    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * VolumeMakeBootable                                                        *
 *                                                                           *
 * Make the currently-selected volume Bootable.  (In LVM terminology, this   *
 * means add it to the IBM Boot Manager menu.)                               *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwnd   : handle of the main program client window            *
 *   PDVMGLOBAL pGlobal: the main program's global data                      *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if the volume was deleted, FALSE otherwise.                        *
 * ------------------------------------------------------------------------- */
BOOL VolumeMakeBootable( HWND hwnd, PDVMGLOBAL pGlobal )
{
    PDVMVOLUMERECORD pVolRec;               // Selected volume container record
    UCHAR            szRes1[ STRING_RES_MAXZ ],
                     szRes2[ STRING_RES_MAXZ ];
    BOOL             bRC = FALSE;
    CARDINAL32       iRC;

    if ( !pGlobal || !pGlobal->volumes || !pGlobal->ulVolumes )
        return FALSE;

    // Get the selected volume
    pVolRec = WinSendDlgItemMsg( pGlobal->hwndVolumes, IDD_VOLUMES,
                                 CM_QUERYRECORDEMPHASIS,
                                 MPFROMP( CMA_FIRST ),
                                 MPFROMSHORT( CRA_SELECTED ));
    if ( !pVolRec )
        return FALSE;

    // Generate the confirmation message
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_VOLUME_BOOTABLE_TITLE, STRING_RES_MAXZ, szRes1 );
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_VOLUME_BOOTABLE_CONFIRM, STRING_RES_MAXZ, szRes2 );

    if ( WinMessageBox( HWND_DESKTOP, hwnd, szRes2, szRes1, 0,
                        MB_YESNO | MB_WARNING | MB_MOVEABLE ) == MBID_YES )
    {
        LvmAddToBootMgr( pVolRec->handle, &iRC );
        if ( pGlobal->pLog ) {
            fprintf( pGlobal->pLog, "-------------------------------------------------------------------------------\n");
            fprintf( pGlobal->pLog, "Making volume bootable: 0x%08X\n", pVolRec->handle );
            fprintf( pGlobal->pLog, "Result: %u\n", iRC );
        }
        if ( iRC != LVM_ENGINE_NO_ERROR )
            PopupEngineError( NULL, iRC, hwnd, pGlobal->hab, pGlobal->hmri );
        else {
            SetModified( hwnd, TRUE );
            bRC = TRUE;
        }
    }

    return bRC;
}


/* ------------------------------------------------------------------------- *
 * VolumeMakeStartable                                                       *
 *                                                                           *
 * Make the currently-selected volume Bootable.  (In LVM terminology, this   *
 * means add it to the IBM Boot Manager menu.)                               *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwnd   : handle of the main program client window            *
 *   PDVMGLOBAL pGlobal: the main program's global data                      *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if the volume was deleted, FALSE otherwise.                        *
 * ------------------------------------------------------------------------- */
BOOL VolumeMakeStartable( HWND hwnd, PDVMGLOBAL pGlobal )
{
    PDVMVOLUMERECORD pVolRec;               // Selected volume container record
    UCHAR            szRes1[ STRING_RES_MAXZ ],
                     szRes2[ STRING_RES_MAXZ ],
                     szBuffer[ STRING_RES_MAXZ + STRING_RES_MAXZ + 4 ];
    BOOL             bRC = FALSE;
    CARDINAL32       iRC;

    if ( !pGlobal || !pGlobal->volumes || !pGlobal->ulVolumes )
        return FALSE;

    // Get the selected volume
    pVolRec = WinSendDlgItemMsg( pGlobal->hwndVolumes, IDD_VOLUMES,
                                 CM_QUERYRECORDEMPHASIS,
                                 MPFROMP( CMA_FIRST ),
                                 MPFROMSHORT( CRA_SELECTED ));
    if ( !pVolRec )
        return FALSE;

    // Generate the confirmation message
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_VOLUME_STARTABLE_CONFIRM, STRING_RES_MAXZ, szRes2 );

    if ( pGlobal->fsEngine & FS_ENGINE_BOOTMGR ) {
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_VOLUME_STARTABLE_BOOTMGR, STRING_RES_MAXZ, szRes1 );
        sprintf( szBuffer, "%s\r\n\r\n%s", szRes2, szRes1 );
    }
    else if ( pGlobal->fsEngine & FS_ENGINE_AIRBOOT ) {
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_VOLUME_STARTABLE_AIRBOOT, STRING_RES_MAXZ, szRes1 );
        sprintf( szBuffer, "%s\r\n\r\n%s", szRes2, szRes1 );
    }
    else strncpy( szBuffer, szRes2, STRING_RES_MAXZ );
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_VOLUME_STARTABLE_TITLE, STRING_RES_MAXZ, szRes1 );

    if ( WinMessageBox( HWND_DESKTOP, hwnd, szBuffer, szRes1, 0,
                        MB_YESNO | MB_WARNING | MB_MOVEABLE ) == MBID_YES )
    {
        LvmSetStartable( pVolRec->handle, &iRC );
        if ( pGlobal->pLog ) {
            fprintf( pGlobal->pLog, "-------------------------------------------------------------------------------\n");
            fprintf( pGlobal->pLog, "Making volume startable: 0x%08X\n", pVolRec->handle );
            fprintf( pGlobal->pLog, "Result: %u\n", iRC );
        }
        if ( iRC != LVM_ENGINE_NO_ERROR )
            PopupEngineError( NULL, iRC, hwnd, pGlobal->hab, pGlobal->hmri );
        else {
            SetModified( hwnd, TRUE );
            bRC = TRUE;
        }
    }

    return bRC;
}

