/*****************************************************************************
 ** LVMPM - airboot.c                                                       **
 *****************************************************************************
 * Routines for working with Air-Boot.                                       *
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

// ----------------------------------------------------------------------------
// INTERNAL CONSTANTS

#define AIRBOOT_INSTALLER  "AIRBOOT2.EXE"   // AiR-BOOT installer executable
#define AIRBOOT_INSTPARMS  "/CID"           // AiR-BOOT installer parameters

#define BOFF_AIRBOOT_SIG  0x2        // offset of AiR-BOOT signature in MBR
#define ACH_AIRBOOT_SIG   "AiRBOOT"  // contents of a valid AiR-BOOT signature
#define CCH_AIRBOOT_SIG   7          // length of a valid AiR-BOOT signature

#define AB_PROMPT_MAXZ    (( STRING_RES_MAXZ * 4 ) + 16 )


/* ------------------------------------------------------------------------- *
 * AirBoot_IsInstalled()                                                     *
 *                                                                           *
 * Check to see if AiR-BOOT is installed by scanning the MBR on disk 1 for   *
 * the AiR-BOOT signature.                                                   *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PCARDINAL32 pError : Pointer to LVM return code.                        *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if the AiR-BOOT (i.e. its signature) was found in the MBR.         *
 *   FALSE if AiR-BOOT was not found in the MBR, or if an error occurred.    *
 * ------------------------------------------------------------------------- */
BOOL AirBoot_IsInstalled( PCARDINAL32 pError )
{
    PSZ  pchSig;                                // AiR-BOOT signature bytes
    BYTE abSector[ BYTES_PER_SECTOR ] = {0};    // buffer for MBR contents

    LvmReadSectors( 1, 0, 1, &abSector, pError );
    if ( *pError == LVM_ENGINE_NO_ERROR ) {
        pchSig = (PSZ) abSector + BOFF_AIRBOOT_SIG;
        if ( strncmp( pchSig, ACH_AIRBOOT_SIG, CCH_AIRBOOT_SIG ) == 0 )
            return TRUE;
    }
    return FALSE;
}


/* ------------------------------------------------------------------------- *
 * AirBoot_GetInstaller()                                                    *
 *                                                                           *
 * Determine whether or not the AiR-BOOT installer is available, and define  *
 * the pathname as appropriate.  If the installer is not found, disable      *
 * AiR-BOOT in the global flags.                                             *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PDVMGLOBAL pGlobal: Pointer to global program data                      *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if the AiR-BOOT installer was found, FALSE otherwise               *
 * ------------------------------------------------------------------------- */
BOOL AirBoot_GetInstaller( PDVMGLOBAL pGlobal )
{
    APIRET rc;

    rc = DosSearchPath( SEARCH_ENVIRONMENT | SEARCH_CUR_DIRECTORY, "PATH",
                        AIRBOOT_INSTALLER, pGlobal->szABEXE, CCHMAXPATH+1 );
    if ( rc )
        memset( pGlobal->szABEXE, 0, CCHMAXPATH+1 );

    if ( !pGlobal->szABEXE[0] ) {
        pGlobal->fsProgram &= ~FS_APP_ENABLE_AB;
        return FALSE;
    }

    return TRUE;
}


/* ------------------------------------------------------------------------- *
 * AirBoot_Install                                                           *
 *                                                                           *
 * Executes the AiR-BOOT install program (in silent/CID mode).               *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd: application client window handle                             *
 *                                                                           *
 * RETURNS: n/a                                                              *
 * ------------------------------------------------------------------------- */
void AirBoot_Install( HWND hwnd )
{
    PDVMGLOBAL   pGlobal;                  // pointer to global program data
    PPROGDETAILS pDetails;                 // structure used by WinStartApp()
    PSZ          psz;                      // offset within path string
    UCHAR        szPath[ CCHMAXPATH + 1 ], // various string buffers
                 szRes1[ STRING_RES_MAXZ ],
                 szRes2[ STRING_RES_MAXZ ],
                 szError[ STRING_APP_ERROR_MAXZ ],
                 szBuffer[ AB_PROMPT_MAXZ ] = {0};
    ULONG        ulErr,                    // WinGetLastError() error code
                 flStyle;                  // WinMessageBox() style flags
    CARDINAL32   rc;                       // LVM return code


    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( !pGlobal || !pGlobal->szABEXE[0] )
        return;

    // Prompt for confirmation
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_AIRBOOT_INSTALL_TITLE, STRING_RES_MAXZ, szRes1 );
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_AIRBOOT_INSTALL_1, STRING_RES_MAXZ, szBuffer );
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_AIRBOOT_INSTALL_2, STRING_RES_MAXZ, szRes2 );
    strncat( szBuffer, szRes2, AB_PROMPT_MAXZ );
    if ( pGlobal->fsEngine & FS_ENGINE_PENDING ) {
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_SAVE_WARNING, STRING_RES_MAXZ, szRes2 );
        strncat( szBuffer, szRes2, AB_PROMPT_MAXZ );
        strncat( szBuffer, "\r\n\r\n", AB_PROMPT_MAXZ );
        flStyle = MB_YESNO | MB_WARNING | MB_MOVEABLE;
    }
    else
        flStyle = MB_YESNO | MB_QUERY | MB_MOVEABLE;
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_PROCEED_CONFIRM, STRING_RES_MAXZ, szRes2 );
    strncat( szBuffer, szRes2, AB_PROMPT_MAXZ );
    strncat( szBuffer, "\r\n\r\n", AB_PROMPT_MAXZ );

    if ( WinMessageBox( HWND_DESKTOP, hwnd, szBuffer, szRes1,
                        0, flStyle ) != MBID_YES )
        return;

    // Save any changes still pending
    if ( pGlobal->fsEngine & FS_ENGINE_PENDING ) {
        LvmCommit( &rc );
        if ( pGlobal->pLog ) {
            fprintf( pGlobal->pLog, "-------------------------------------------------------------------------------\n");
            fprintf( pGlobal->pLog, "LVM CHANGES COMMITTED: %u\n", rc );
        }
        if ( rc != LVM_ENGINE_NO_ERROR ) {
            PopupEngineError( NULL, rc, hwnd, pGlobal->hab, pGlobal->hmri );
            return;
        }
        SetModified( hwnd, FALSE );
    }

    // Clear our data structures and close the LVM engine
    LVM_FreeData( pGlobal );
    LVM_Stop( pGlobal );

    // Run the Air-Boot install utility (external program)
    strncpy( szPath, pGlobal->szABEXE, CCHMAXPATH );
    if (( psz = strrchr( szPath, '\\')) != NULL ) *psz = 0;
    else szPath[0] = 0;
    pDetails = (PPROGDETAILS) calloc( 1, sizeof(PROGDETAILS) );
    pDetails->Length          = sizeof( PROGDETAILS );
    pDetails->progt.progc     = PROG_DEFAULT;
    pDetails->progt.fbVisible = SHE_INVISIBLE;
    pDetails->pszExecutable   = strdup( pGlobal->szABEXE );
    pDetails->pszParameters   = strdup( AIRBOOT_INSTPARMS );
    pDetails->pszStartupDir   = ( szPath[0] ? szPath : NULL );
    pDetails->swpInitial.fl   = SWP_HIDE;

    pGlobal->happAB = WinStartApp( hwnd, pDetails, NULL, NULL,
                                   SAF_INSTALLEDCMDLINE | SAF_STARTCHILDAPP );
    if ( pGlobal->happAB == NULLHANDLE ) {
        // Show an error if the program failed to start
        ulErr = WinGetLastError( pGlobal->hab );
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_ERROR_STARTAPP, STRING_RES_MAXZ, szRes1 );
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_ERROR_GENERIC, STRING_RES_MAXZ, szRes2 );
        sprintf( szError, szRes1, pDetails->pszExecutable,
                 ERRORIDERROR( ulErr ));
        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, szError, szRes2,
                       0, MB_OK | MB_ERROR );
    }

    free( pDetails->pszExecutable );
    free( pDetails->pszParameters );
    free( pDetails );
}



/* ------------------------------------------------------------------------- *
 * AirBoot_InstallerExit                                                     *
 *                                                                           *
 * Interprets the result of executing the AiR-BOOT install program.  Called  *
 * on WM_APPTERMINATENOTIFY from the main window handler.                    *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwnd   : application client window handle                    *
 *   ULONG      rc     : return code from install program                    *
 *   PDVMGLOBAL pGlobal: Pointer to global program data                      *
 *                                                                           *
 * RETURNS: n/a                                                              *
 * ------------------------------------------------------------------------- */
void AirBoot_InstallerExit( HWND hwnd, ULONG rc, PDVMGLOBAL pGlobal )
{
    UCHAR szRes1[ STRING_RES_MAXZ ],
          szRes2[ STRING_RES_MAXZ ],
          szRes3[ STRING_RES_MAXZ ],
          szError[ STRING_APP_ERROR_MAXZ ] = {0};
    ULONG ulID;


    // Restart the LVM engine
    LVM_Start( pGlobal->pLog, pGlobal->hab, pGlobal->hmri );
    LVM_InitData( hwnd, pGlobal );

    // Check to see whether AiR-BOOT was actually installed successfully
    if ( pGlobal->fsEngine & ~FS_ENGINE_AIRBOOT ) {
        switch ( rc ) {
            case 1:  ulID = IDS_ERROR_IO;           break;
            case 2:  ulID = IDS_ERROR_GEOMETRY;     break;
            case 3:  ulID = IDS_ERROR_INSUFFICIENT; break;
            default: ulID = IDS_ERROR_OTHER;        break;
        }
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_AIRBOOT_FAILED_TITLE, STRING_RES_MAXZ, szRes1 );
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_AIRBOOT_FAILED, STRING_RES_MAXZ, szRes2 );
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       ulID, STRING_RES_MAXZ, szRes3 );
        sprintf( szError, szRes2, szRes3, rc );
        WinMessageBox( HWND_DESKTOP, hwnd, szError, szRes1,
                       0, MB_OK | MB_ERROR | MB_MOVEABLE );
    }
    else {
        WinSendMsg( pGlobal->hwndMenu, MM_SETITEMATTR,
                    MPFROM2SHORT( ID_AIRBOOT_REMOVE, TRUE ),
                    MPFROM2SHORT( MIA_DISABLED, 0 ));
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_SUCCESS_TITLE, STRING_RES_MAXZ, szRes1 );
        WinLoadString( pGlobal->hab, pGlobal->hmri,
                       IDS_AIRBOOT_INSTALLED, STRING_RES_MAXZ, szRes2 );
        WinMessageBox( HWND_DESKTOP, hwnd, szRes2, szRes1, 0,
                       MB_OK | MB_INFORMATION | MB_MOVEABLE );
    }

    pGlobal->happAB = NULLHANDLE;
}


/* ------------------------------------------------------------------------- *
 * AirBoot_Delete()                                                          *
 *                                                                           *
 * Forces an immediate rewrite of the Master Boot Record on the first disk   *
 * drive, in order to remove AiR-BOOT.                                       *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd: application client window handle                             *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void AirBoot_Delete( HWND hwnd )
{
    PDVMGLOBAL   pGlobal;
    UCHAR        szRes1[ STRING_RES_MAXZ ],
                 szRes2[ STRING_RES_MAXZ ],
                 szBuffer[ STRING_RES_MAXZ * 2 ];
    CARDINAL32   rc;


    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( !pGlobal ) return;

    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_AIRBOOT_DELETE, STRING_RES_MAXZ, szRes1 );
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_NEWMBR_TITLE, STRING_RES_MAXZ, szBuffer );
    WinLoadString( pGlobal->hab, pGlobal->hmri,
                   IDS_PROCEED_CONFIRM, STRING_RES_MAXZ, szRes2 );
    strncat( szBuffer, szRes2, STRING_RES_MAXZ * 2 );
    strncat( szBuffer, "\r\n", STRING_RES_MAXZ * 2 );
    if ( WinMessageBox( HWND_DESKTOP, hwnd, szBuffer, szRes1, 0,
                        MB_YESNO | MB_WARNING | MB_MOVEABLE ) == MBID_YES )
    {
        LvmNewMBR( pGlobal->disks[ 0 ].handle, &rc );
        if ( pGlobal->pLog ) {
            fprintf( pGlobal->pLog, "-------------------------------------------------------------------------------\n");
            fprintf( pGlobal->pLog, "Writing new MBR to disk: 0\nResult: %u\n", rc );
        }
        if ( rc != LVM_ENGINE_NO_ERROR )
            PopupEngineError( NULL, rc, hwnd, pGlobal->hab, pGlobal->hmri );
        else {
            SetModified( hwnd, TRUE );
            pGlobal->fsEngine &= ~FS_ENGINE_AIRBOOT;
            WinSendMsg( pGlobal->hwndMenu, MM_SETITEMATTR,
                        MPFROM2SHORT( ID_AIRBOOT_REMOVE, TRUE ),
                        MPFROM2SHORT( MIA_DISABLED, MIA_DISABLED ));
            WinLoadString( pGlobal->hab, pGlobal->hmri,
                           IDS_SUCCESS_TITLE, STRING_RES_MAXZ, szRes1 );
            WinLoadString( pGlobal->hab, pGlobal->hmri,
                           IDS_NEWMBR_OK, STRING_RES_MAXZ, szRes2 );
            WinMessageBox( HWND_DESKTOP, hwnd, szRes2, szRes1, 0,
                           MB_OK | MB_INFORMATION | MB_MOVEABLE );
        }
    }
}


