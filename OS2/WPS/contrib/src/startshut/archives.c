
/*
 *@@sourcefile archives.c:
 *      the purpose of this code is to examine the WPS's status
 *      by evaluating dates, directory and system INI file contents.
 *      We can then draw the conclusion (depending on user settings)
 *      whether the WPS backup functionality should be enabled.
 *
 *      These checks are performed in arcCheckIfBackupNeeded, which
 *      gets called during Desktop startup.
 *
 *      This file is ALL new with V0.9.0.
 *
 *      This is based on ideas in the WPSArcO code which has kindly
 *      been provided by Stefan Milcke (Stefan.Milcke@t-online.de).
 *
 *      All funtions in this file have the arc* prefix.
 *
 *      Note that the ARCHIVINGSETTINGS are only manipulated by the
 *      new Desktop "Archives" notebook page, whose code is
 *      in xfdesk.c. For the archiving settings, we do not use
 *      the global settings (as the rest of XWorkplace does), but
 *      this separate structure instead. See arcQuerySettings.
 *
 *@@added V0.9.0 [umoeller]
 *@@header "startshut\archives.h"
 */

/*
 *      Copyright (C) 1999-2008 Ulrich M”ller.
 *
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

/*
 *@@todo:
 *  csm:
         Ok, no guarantee, but this is what I found out:
             Offset 0x00: (int32)    "Nummer des Archives nach Restore"
                                         (Ist immer 0, wenn ein Backup gemacht wurde.
                                          Nach einem Restore ist hier die Nummer des Archives zu finden)
             Offset 0xCF: (int32)    "Dateien bei jedem Systemstart aktivieren"
                                         (0 = off, 1 = on/selected)
             Offset 0xD7: (int32)    "Anzahl der zu sichernden Archive"
             Offset 0xD9: (int32)    "Anzeige der Optionen bei jedem Neustart"
                                         (0 = off, 2 = on/selected)
             Offset 0xDD: (int32)    "Zeitsperre fuer Anzeigen der Optionen"
                                         (0 to 999)
         The rest of the file I don't know (besides the path
         beginning at offset 0x06, 200 bytes long)
 */

#pragma strings(readonly)

/*
 *  Suggested #include order:
 *  1)  os2.h
 *  2)  C library headers
 *  3)  setup.h (code generation and debugging options)
 *  4)  headers in helpers\
 *  5)  at least one SOM implementation header (*.ih)
 *  6)  dlgids.h, headers in shared\ (as needed)
 *  7)  headers in implementation dirs (e.g. filesys\, as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSDATETIME
#define INCL_DOSERRORS

#define INCL_WINDIALOGS
#define INCL_WINBUTTONS
#define INCL_WINSTATICS
#define INCL_WINSTDSPIN
#define INCL_WINSHELLDATA       // Prf* functions
#include <os2.h>

#include <string.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <time.h>
#include <process.h>
#include <math.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\datetime.h"           // date/time helpers
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\dosh.h"
#include "helpers\prfh.h"
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"
#include "helpers\winh.h"               // PM helper routines
#include "helpers\wphandle.h"           // file-system object handles
#include "helpers\xstring.h"            // extended string helpers

// SOM headers which don't crash with prec. header files
#include "xfdesk.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\notebook.h"            // generic XWorkplace notebook handling

#include "startshut\archives.h"         // archiving declarations

// other SOM headers
#pragma hdrstop                 // VAC++ keeps crashing otherwise

/********************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

static ARCHIVINGSETTINGS   G_ArcSettings;
static DATETIME            G_dtLastArchived;
static BOOL                G_fSettingsLoaded = FALSE;

static CHAR                G_szArcBaseFilename[CCHMAXPATH] = "";

#define ARCOFS_JUSTRESTORED             0x00000
#define ARCOFS_ARCHIVINGENABLED         0x000CF
#define ARCOFS_MAXARCHIVES              0x000D7
#define ARCOFS_SHOWRESTORE              0x000D9
#define ARCOFS_RESTORETIMEOUT_LOW       0x000DD
#define ARCOFS_RESTORETIMEOUT_HIGH      0x000DE

/********************************************************************
 *
 *   "Archives" page replacement in WPDesktop
 *
 ********************************************************************/

// PSZ's for percentage spinbutton
static PCSZ     G_apcszPercentages[] =
    {
        "0.010",
        "0.025",
        "0.050",
        "0.075",
        "0.100",
        "0.250",
        "0.500",
        "0.750",
        "1.000",
        "2.500",
        "5.000"
    };

#define SPIN_WIDTH      40
#define SPIN_HEIGHT     STD_SPIN_HEIGHT

static const CONTROLDEF
    ArcCriteriaGroup = LOADDEF_GROUP(ID_XSDI_ARC_CRITERIA_GROUP, DEFAULT_TABLE_WIDTH),
    EnableArchiveCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_ARC_ENABLE),
    ArcAlwaysCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_ARC_ALWAYS),
    ArcNextCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_ARC_NEXT),
    ArcINICB = LOADDEF_AUTOCHECKBOX(ID_XSDI_ARC_INI),
    ArcINISpin =
        {
            WC_SPINBUTTON,
            NULL,
            WS_VISIBLE | WS_TABSTOP
                | SPBS_MASTER | SPBS_JUSTLEFT | SPBS_JUSTRIGHT | SPBS_JUSTCENTER
                | SPBS_FASTSPIN,
            ID_XSDI_ARC_INI_SPIN,
            CTL_COMMON_FONT,
            { SPIN_WIDTH, SPIN_HEIGHT },     // size
            COMMON_SPACING,
        },
    ArcINITxt2 = LOADDEF_TEXT(ID_XSDI_ARC_INI_SPINTXT1),
    ArcDaysCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_ARC_DAYS),
    ArcDaysSpin =
        {
            WC_SPINBUTTON,
            NULL,
            WS_VISIBLE | WS_TABSTOP
                | SPBS_MASTER | SPBS_JUSTLEFT | SPBS_JUSTRIGHT | SPBS_JUSTCENTER
                | SPBS_FASTSPIN,
            ID_XSDI_ARC_DAYS_SPIN,
            CTL_COMMON_FONT,
            { SPIN_WIDTH, SPIN_HEIGHT },     // size
            COMMON_SPACING,
        },
    ArcDaysTxt2 = LOADDEF_TEXT(ID_XSDI_ARC_DAYS_SPINTXT1),
    ArcShowStatusCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_ARC_SHOWSTATUS),
    Arc2Group = LOADDEF_GROUP(ID_XSDI_ARC_ARCHIVES_GROUP, DEFAULT_TABLE_WIDTH),
    ArcNoTxt1 = LOADDEF_TEXT(ID_XSDI_ARC_ARCHIVES_NO_TXT1),
    ArcNoSpin =
        {
            WC_SPINBUTTON,
            "0",
            WS_VISIBLE | WS_TABSTOP
                | SPBS_MASTER | SPBS_JUSTLEFT | SPBS_JUSTRIGHT | SPBS_JUSTCENTER,
            ID_XSDI_ARC_ARCHIVES_NO_SPIN,
            CTL_COMMON_FONT,
            { SPIN_WIDTH, SPIN_HEIGHT },     // size
            COMMON_SPACING,
        },
    ArcNoTxt2 = LOADDEF_TEXT(ID_XSDI_ARC_ARCHIVES_NO_TXT2),
    ArcRestOptions = LOADDEF_GROUP(ID_XSDI_ARC_RESTORE_GROUP,DEFAULT_TABLE_WIDTH),
    ArcRestShowAlways = LOADDEF_AUTOCHECKBOX(ID_XSDI_ARC_RESTORE_ALWAYS),
    ArcRestTimeoutTxt = LOADDEF_TEXT(ID_XSDI_ARC_RESTORE_TXT1), // V1.0.5 (2005-11-26) [pr]
    ArcRestTimeoutSpin =
        {
            WC_SPINBUTTON,
            NULL,
            WS_VISIBLE | WS_TABSTOP
                | SPBS_MASTER | SPBS_JUSTLEFT | SPBS_JUSTRIGHT | SPBS_JUSTCENTER,
            ID_XSDI_ARC_RESTORE_SPIN,
            CTL_COMMON_FONT,
            { SPIN_WIDTH, SPIN_HEIGHT },     // size; V1.0.5 (2005-11-26) [pr]
            COMMON_SPACING,
        },
    ArcRestTimeoutTxt2 = LOADDEF_TEXT(ID_XSDI_ARC_RESTORE_TXT2);

static const DLGHITEM dlgArchives[] =
    {
        START_TABLE,            // root table, required
            START_ROW(0),       // row 1 in the root table, required
                // create group on top
                START_GROUP_TABLE(&ArcCriteriaGroup),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&EnableArchiveCB),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&G_Spacing),    // notebook.c
                        CONTROL_DEF(&ArcAlwaysCB),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&G_Spacing),    // notebook.c
                        CONTROL_DEF(&ArcNextCB),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&G_Spacing),    // notebook.c
                        CONTROL_DEF(&ArcINICB),
                        CONTROL_DEF(&ArcINISpin),
                        CONTROL_DEF(&ArcINITxt2),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&G_Spacing),    // notebook.c
                        CONTROL_DEF(&ArcDaysCB),
                        CONTROL_DEF(&ArcDaysSpin),
                        CONTROL_DEF(&ArcDaysTxt2),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&ArcShowStatusCB),
                END_TABLE,
            START_ROW(0),       // row 2 in the root table
                START_GROUP_TABLE(&Arc2Group),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&ArcNoTxt1),
                        CONTROL_DEF(&ArcNoSpin),
                        CONTROL_DEF(&ArcNoTxt2),
                END_TABLE,
            START_ROW(0),       // row 3 in the root table
                START_GROUP_TABLE(&ArcRestOptions),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&ArcRestShowAlways),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&ArcRestTimeoutTxt),
                        CONTROL_DEF(&ArcRestTimeoutSpin),
                        CONTROL_DEF(&ArcRestTimeoutTxt2),
                END_TABLE,
            START_ROW(0),       // notebook buttons (will be moved)
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE
    };

/*
 * arcArchivesInitPage:
 *      notebook callback function (notebook.c) for the
 *      "Archives" page replacement in the Desktop's settings
 *      notebook.
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.9 (2001-04-07) [pr]: fixed Undo/Default
 *@@changed V0.9.16 (2001-11-22) [umoeller]: now using dlg formatter
 *@@changed V1.0.4 (2005-10-17) [bvl]: support restore settings @@fixes 471
 */

VOID arcArchivesInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                          ULONG flFlags)        // CBI_* flags (notebook.h)
{
    PARCHIVINGSETTINGS pArcSettings = arcQuerySettings();

    if (flFlags & CBI_INIT)
    {
        // first call: backup archive settings for "Undo" button;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        if (pnbp->pUser = malloc(sizeof(ARCHIVINGSETTINGS)))
            memcpy(pnbp->pUser, pArcSettings, sizeof(ARCHIVINGSETTINGS));

        // insert the controls using the dialog formatter
        // V0.9.16 (2001-11-22) [umoeller]
        ntbFormatPage(pnbp->hwndDlgPage,
                      dlgArchives,
                      ARRAYITEMCOUNT(dlgArchives));

        WinSendDlgItemMsg(pnbp->hwndDlgPage, ID_XSDI_ARC_INI_SPIN,
                          SPBM_SETARRAY,
                          (MPARAM)&G_apcszPercentages,
                          (MPARAM)ARRAYITEMCOUNT(G_apcszPercentages));
    }

    if (flFlags & CBI_SET)
    {
        ULONG       ul = 0;

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_ARC_ENABLE,
                              (pArcSettings->ulArcFlags & ARCF_ENABLED) != 0);
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_ARC_ALWAYS,
                              (pArcSettings->ulArcFlags & ARCF_ALWAYS) != 0);
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_ARC_NEXT,
                              (pArcSettings->ulArcFlags & ARCF_NEXT) != 0);
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_ARC_INI,
                              (pArcSettings->ulArcFlags & ARCF_INI) != 0);

        // INI files percentage:
        // go thru the spin button array and find
        // percentage that matches
        for (ul = 0; ul < ARRAYITEMCOUNT(G_apcszPercentages); ul++)
        {
            float dTemp;
            // CHAR szTempx[100];
            // convert current array item to float
            sscanf(G_apcszPercentages[ul], "%f", &dTemp);

            // same?
            if (fabs(dTemp - pArcSettings->dIniFilesPercent) < 0.00001)
                                // prevent rounding errors
            {
                // yes: set this spin button array item
                WinSendDlgItemMsg(pnbp->hwndDlgPage, ID_XSDI_ARC_INI_SPIN,
                                  SPBM_SETCURRENTVALUE,
                                  (MPARAM)ul,
                                  (MPARAM)NULL);
                break;
            }
        }

        // every xxx days
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_ARC_DAYS,
                              (pArcSettings->ulArcFlags & ARCF_DAYS) != 0);
        winhSetDlgItemSpinData(pnbp->hwndDlgPage, ID_XSDI_ARC_DAYS_SPIN,
                               1, 50,       // spin button limits
                               pArcSettings->ulEveryDays);

        // status
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_ARC_SHOWSTATUS,
                              pArcSettings->fShowStatus);

        // no. of archives
        arcSetNumArchives(&pArcSettings->cArchivesCount,
                          FALSE);       // query
        winhSetDlgItemSpinData(pnbp->hwndDlgPage, ID_XSDI_ARC_ARCHIVES_NO_SPIN,
                               1, 9,        // spin button limits
                               pArcSettings->cArchivesCount);

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_ARC_RESTORE_ALWAYS,
                              pArcSettings->fShowRestore);

        winhSetDlgItemSpinData(pnbp->hwndDlgPage, ID_XSDI_ARC_RESTORE_SPIN,
                               0, 999,        // spin button limits
                               pArcSettings->usRestoreTimeOut);
    }

    if (flFlags & CBI_ENABLE)
    {
        BOOL    fEnabled = ((pArcSettings->ulArcFlags & ARCF_ENABLED) != 0),
                fAlways = ((pArcSettings->ulArcFlags & ARCF_ALWAYS) != 0),
                fINI = winhIsDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_ARC_INI),
                fDays = winhIsDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_ARC_DAYS);
        WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_ARC_ALWAYS,
                          fEnabled);
        WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_ARC_NEXT,
                          fEnabled && !fAlways);
        WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_ARC_INI,
                          fEnabled && !fAlways);
        WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_ARC_INI_SPIN,
                          ( (fEnabled) && (fINI) ) && !fAlways);
        WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_ARC_INI_SPINTXT1,
                          ( (fEnabled) && (fINI) ) && !fAlways);
        WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_ARC_DAYS,
                          fEnabled && !fAlways);
        WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_ARC_DAYS_SPIN,
                          ( (fEnabled) && (fDays) ) && !fAlways);
        WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_ARC_DAYS_SPINTXT1,
                          ( (fEnabled) && (fDays) ) && !fAlways);

        WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_ARC_SHOWSTATUS,
                          fEnabled);

        WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_ARC_RESTORE_SPIN,
        				  pArcSettings->fShowRestore);
        WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_ARC_RESTORE_TXT1,
        				  pArcSettings->fShowRestore);				
        WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_ARC_RESTORE_TXT2,
        				  pArcSettings->fShowRestore);				
    }
}

/*
 * arcArchivesItemChanged:
 *      notebook callback function (notebook.c) for the
 *      "Archives" page replacement in the Desktop's settings
 *      notebook.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.9 (2001-04-07) [pr]: fixed Undo/Default
 *@@changed V1.0.4 (2005-10-17) [bvl]: support restore settings @@fixes 471
 */

MRESULT arcArchivesItemChanged(PNOTEBOOKPAGE pnbp,
                               ULONG ulItemID,
                               USHORT usNotifyCode,
                               ULONG ulExtra)      // for checkboxes: contains new state
{
    PARCHIVINGSETTINGS pArcSettings = arcQuerySettings();
    ULONG           ulSetFlags = 0;
    BOOL            fSave = TRUE;

    switch (ulItemID)
    {
        case ID_XSDI_ARC_ENABLE:
            ulSetFlags |= ARCF_ENABLED;
        break;

        case ID_XSDI_ARC_ALWAYS:
            ulSetFlags |= ARCF_ALWAYS;
        break;

        case ID_XSDI_ARC_NEXT:
            ulSetFlags |= ARCF_NEXT;
        break;

        case ID_XSDI_ARC_INI:
            ulSetFlags |= ARCF_INI;
        break;

        case ID_XSDI_ARC_INI_SPIN:
            if (    (pnbp->flPage & NBFL_PAGE_INITED)       // skip this during initialization
                 && (usNotifyCode == SPBN_CHANGE)
               )
            {
                CHAR    szTemp[100];
                float   flTemp = 0;
                // query current spin button array
                // item as a string
                WinSendDlgItemMsg(pnbp->hwndDlgPage, ulItemID,
                                  SPBM_QUERYVALUE,
                                  (MPARAM)szTemp,
                                  MPFROM2SHORT(sizeof(szTemp),
                                  SPBQ_ALWAYSUPDATE));
                sscanf(szTemp, "%f", &flTemp);
                pArcSettings->dIniFilesPercent = (double)flTemp;
            }
        break;

        case ID_XSDI_ARC_DAYS:
            ulSetFlags |= ARCF_DAYS;
        break;

        case ID_XSDI_ARC_DAYS_SPIN:
            pArcSettings->ulEveryDays = winhAdjustDlgItemSpinData(pnbp->hwndDlgPage,
                                                                  ulItemID,
                                                                  0,              // no grid
                                                                  usNotifyCode);
        break;

        case ID_XSDI_ARC_SHOWSTATUS:
            pArcSettings->fShowStatus = ulExtra;
        break;

        case ID_XSDI_ARC_ARCHIVES_NO_SPIN:
            pArcSettings->cArchivesCount = (CHAR)winhAdjustDlgItemSpinData(pnbp->hwndDlgPage,
                                                                           ulItemID,
                                                                           0,              // no grid
                                                                           usNotifyCode);
            arcSetNumArchives(&pArcSettings->cArchivesCount,
                              TRUE);        // set
            fSave = FALSE;
        break;

        case ID_XSDI_ARC_RESTORE_ALWAYS:
            pArcSettings->fShowRestore = ulExtra;
            arcSetArchiveByte(pArcSettings->fShowRestore,
                              ARCOFS_SHOWRESTORE);
            pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
            fSave = FALSE;
        break;

        case ID_XSDI_ARC_RESTORE_SPIN:
            pArcSettings->usRestoreTimeOut = (USHORT)winhAdjustDlgItemSpinData(pnbp->hwndDlgPage,
                                                                           ulItemID,
                                                                           0,              // no grid
                                                                           usNotifyCode);

            arcSetArchiveByte((CHAR)pArcSettings->usRestoreTimeOut,
                              ARCOFS_RESTORETIMEOUT_LOW);
            arcSetArchiveByte((CHAR)(pArcSettings->usRestoreTimeOut >> 8),
                              ARCOFS_RESTORETIMEOUT_HIGH);
            fSave = FALSE;
        break;

        case DID_UNDO:
        {
            // "Undo" button: get pointer to backed-up archive settings
            PARCHIVINGSETTINGS pWASBackup = (PARCHIVINGSETTINGS)(pnbp->pUser);

            // and restore the settings for this page
            pArcSettings->ulArcFlags = pWASBackup->ulArcFlags;
            pArcSettings->dIniFilesPercent = pWASBackup->dIniFilesPercent;
            pArcSettings->ulEveryDays = pWASBackup->ulEveryDays;
            pArcSettings->fShowStatus = pWASBackup->fShowStatus;
            pArcSettings->cArchivesCount = pWASBackup->cArchivesCount;
            arcSetNumArchives(&pArcSettings->cArchivesCount,
                              TRUE);        // set
            pArcSettings->fShowRestore = pWASBackup->fShowRestore;
            arcSetArchiveByte(pArcSettings->fShowRestore,
                              ARCOFS_SHOWRESTORE);
            pArcSettings->usRestoreTimeOut = pWASBackup->usRestoreTimeOut;
            arcSetArchiveByte((CHAR)pArcSettings->usRestoreTimeOut,
                              ARCOFS_RESTORETIMEOUT_LOW);
            arcSetArchiveByte((CHAR)(pArcSettings->usRestoreTimeOut >> 8),
                              ARCOFS_RESTORETIMEOUT_HIGH);
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        }
        break;

        case DID_DEFAULT:
            // set the default settings for this settings page
            arcSetDefaultSettings();
            arcSetNumArchives(&pArcSettings->cArchivesCount,
                              TRUE);        // set
            arcSetArchiveByte(pArcSettings->fShowRestore,
                              ARCOFS_SHOWRESTORE);
            arcSetArchiveByte((CHAR)pArcSettings->usRestoreTimeOut,
                              ARCOFS_RESTORETIMEOUT_LOW);
            arcSetArchiveByte((CHAR)(pArcSettings->usRestoreTimeOut >> 8),
                              ARCOFS_RESTORETIMEOUT_HIGH);
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        break;

        default:
            fSave = FALSE;
    }

    if (ulSetFlags)
    {
        if (ulExtra)    // checkbox checked
            pArcSettings->ulArcFlags |= ulSetFlags;
        else            // checkbox unchecked
            pArcSettings->ulArcFlags &= ~ulSetFlags;
        // re-enable dlg items by calling the INIT callback
        pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
    }

    if (fSave)
        // enable/disable items
        arcSaveSettings();

    return (MPARAM)0;
}

/********************************************************************
 *
 *   Archiving settings
 *
 ********************************************************************/

/*
 *@@ arcSetDefaultSettings:
 *      this initializes the global ARCHIVINGSETTINGS
 *      structure with default values.
 *
 *@@changed V0.9.9 (2001-04-07) [pr]: fixed Undo/Default
 *@@changed V1.0.4 (2005-10-17) [bvl]: added extra settings from ARCHBASE.$$$ @@fixes 471
 */

VOID arcSetDefaultSettings(VOID)
{
    PMPF_STARTUP(("**** Settings defaults"));

    G_ArcSettings.ulArcFlags = 0;
    G_ArcSettings.dIniFilesPercent = .1;
    G_ArcSettings.ulEveryDays = 1;
    G_ArcSettings.fShowStatus = TRUE;
    G_ArcSettings.cArchivesCount = 3;
    G_ArcSettings.fShowRestore = FALSE;
    G_ArcSettings.usRestoreTimeOut = 0;
}

/*
 *@@ arcQuerySettings:
 *      this returns the global ARCHIVINGSETTINGS
 *      structure, which is filled with the data
 *      from OS2.INI if this is queried for the
 *      first time.
 *
 *@@changed V0.9.9 (2001-04-07) [pr]: fixed Undo/Default
 *@@changed V1.0.4 (2005-10-17) [bvl]: Read extra settings from ARCHBASE.$$$ @@fixes 471
 */

PARCHIVINGSETTINGS arcQuerySettings(VOID)
{
    if (!G_fSettingsLoaded)
    {
        ULONG   cbData = sizeof(G_ArcSettings);
        BOOL    brc = FALSE;
        // first call:
        G_fSettingsLoaded = TRUE;
        // load settings
        brc = PrfQueryProfileData(HINI_USER,
                                  (PSZ)INIAPP_XWORKPLACE,
                                  (PSZ)INIKEY_ARCHIVE_SETTINGS,
                                  &G_ArcSettings,
                                  &cbData);
        if ((!brc) || cbData != sizeof(G_ArcSettings))
            // data not found:
        {
            G_ArcSettings.dAppsSizeLast = 0;
            G_ArcSettings.dKeysSizeLast = 0;
            G_ArcSettings.dDataSumLast = 0;
            arcSetDefaultSettings();
        }

        cbData = sizeof(G_dtLastArchived);
        brc = PrfQueryProfileData(HINI_USER,
                                  (PSZ)INIAPP_XWORKPLACE,
                                  (PSZ)INIKEY_ARCHIVE_LASTBACKUP,
                                  &G_dtLastArchived,
                                  &cbData);
        if ((!brc) || cbData != sizeof(G_dtLastArchived))
        {
            // data not found:
            DosGetDateTime(&G_dtLastArchived);
            G_dtLastArchived.year = 1990;       // enfore backup then
        }

        // initialize szArcBaseFilename
        sprintf(G_szArcBaseFilename,
                "%c:\\OS2\\BOOT\\ARCHBASE.$$$",
                doshQueryBootDrive());

        // Get data from ARCHBASE.$$$
        arcQueryArchiveByte((CHAR*)&G_ArcSettings.fShowRestore,
                            ARCOFS_SHOWRESTORE);
        arcQueryArchiveByte((CHAR*)&G_ArcSettings.usRestoreTimeOut,
                            ARCOFS_RESTORETIMEOUT_LOW);
        arcQueryArchiveByte(((CHAR*)&G_ArcSettings.usRestoreTimeOut)+1,
                            ARCOFS_RESTORETIMEOUT_HIGH);
    }

    return &G_ArcSettings;
}

/*
 *@@ arcSaveSettings:
 *      this writes the ARCHIVINGSETTINGS structure
 *      back to OS2.INI.
 */

BOOL arcSaveSettings(VOID)
{
    return PrfWriteProfileData(HINI_USER,
                               (PSZ)INIAPP_XWORKPLACE,
                               (PSZ)INIKEY_ARCHIVE_SETTINGS,
                               &G_ArcSettings,
                               sizeof(G_ArcSettings));
}

/********************************************************************
 *
 *   Archiving Enabling
 *
 ********************************************************************/

/*
 *@@ arcSetArchiveByte:
 *      writes byte "byte" to offset "offset" in ARCHBASE.$$$.
 *
 *@@added V0.9.13 (2001-06-14) [umoeller]
 */

APIRET arcSetArchiveByte(UCHAR byte,        // in: byte to write
                         LONG offset)
{
    APIRET arc = NO_ERROR;
    // ULONG ulAttr;
    // file is always write protected, so disable this
    // before writing
    if (!(arc = doshSetPathAttr(G_szArcBaseFilename,
                                FILE_ARCHIVED)))
    {
        // open the file for write access then
        PXFILE pFile = NULL;
        ULONG cb = 0;
        if (!(arc = doshOpen(G_szArcBaseFilename,
                             XOPEN_READWRITE_EXISTING,
                             &cb,
                             &pFile)))
        {
            arc = doshWriteAt(pFile,
                              offset,
                              1,
                              &byte);

            doshClose(&pFile);
        }

        doshSetPathAttr(G_szArcBaseFilename,
                        FILE_ARCHIVED | FILE_READONLY);
    }

    if (arc)
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
               "doshSetPathAttr(%s) returned error %d",
               G_szArcBaseFilename,
               arc);

    return arc;
}

/*
 *@@ arcQueryArchiveByte:
 *      gets byte pointed to by "pByte" from offset "offset"
 *      in ARCHBASE.$$$.
 *
 *@@added V0.9.13 (2001-06-14) [umoeller]
 */

APIRET arcQueryArchiveByte(UCHAR *pByte,        // out: read byte
                           LONG offset)
{
    APIRET arc = NO_ERROR;

    // open the file then
    PXFILE pFile;
    ULONG cbFile = 0;
    if (!(arc = doshOpen(G_szArcBaseFilename,
                         XOPEN_READ_EXISTING,
                         &cbFile,
                         &pFile)))
    {
        ULONG cb = 1;
        arc = doshReadAt(pFile,
                         offset,
                         &cb,
                         pByte,
                         0);

        doshClose(&pFile);
    }

    if (arc)
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
               "Got error %d", arc);

    return arc;
}

/*
 *@@ arcForceNoArchiving:
 *      enforces that all archiving is disabled, both in
 *      XWP and the WPS.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

VOID arcForceNoArchiving(VOID)
{
    // force loading of settings
    arcQuerySettings();
    arcSwitchArchivingOn(FALSE);
    G_ArcSettings.ulArcFlags &= ~ARCF_ENABLED;
    arcSaveSettings();
}

#define ID_XFDI_GENERICDLGTEXT          221

static CONTROLDEF
    ArcStatusIcon = CONTROLDEF_ICON(
                            NULLHANDLE,     // replaced below
                            ID_ICONDLG),
    ArcStatusText = CONTROLDEF_TEXT_WORDBREAK(
                            "1\n2\n3",   // room for three lines, replaced below
                            ID_XFDI_GENERICDLGTEXT,
                            200);

static const DLGHITEM dlgArcStatus[] =
    {
        START_TABLE,            // root table, required
            START_ROW(ROW_VALIGN_CENTER),
                CONTROL_DEF(&ArcStatusIcon),
                CONTROL_DEF(&ArcStatusText),
        END_TABLE
    };

/*
 *@@ ShowStatus:
 *
 *@@added V0.9.16 (2001-11-25) [umoeller]
 */

STATIC VOID ShowStatus(HWND hwndStatus,
                       PCSZ pcszText)
{
    WinSetDlgItemText(hwndStatus, ID_XFDI_GENERICDLGTEXT,
                      pcszText);
    WinSetWindowPos(hwndStatus,
                    HWND_TOP,
                    50,
                    50,
                    0,
                    0,
                    SWP_MOVE | SWP_ZORDER | SWP_SHOW);
                            // but no activate
}

/*
 *@@ arcCheckIfBackupNeeded:
 *      this checks the system according to the settings
 *      in ARCHIVINGSETTINGS (i.e. always, next bootup,
 *      date, INI changes -- see arcCheckINIFiles),
 *      and calls arcSwitchArchivingOn according to the
 *      result of these checks.
 *
 *      This gets called from initMain
 *      while the WPS is booting up (see remarks there).
 *      If we enable Desktop archiving here, the WPS will
 *      archive the Desktop soon afterwards.
 *
 *      If (ARCHIVINGSETTINGS.fShowStatus == TRUE), hwndNotify
 *      will receive a msg with the value ulMsg and the
 *      HWND of the notification window in mp1 to be able
 *      to destroy it later.
 *
 *      initMain sets this to the XWorkplace
 *      Thread-1 object window (krn_fnwpThread1Object), which in
 *      turn starts a timer to destroy the window later.
 *
 *@@changed V0.9.4 (2000-07-22) [umoeller]: archiving wasn't always disabled if turned off completely; fixed
 *@@changed V0.9.13 (2001-06-14) [umoeller]: no longer using archive marker file, thanks Stefan Milcke
 *@@changed V0.9.16 (2001-11-19) [umoeller]: now using dialog formatter for status window
 */

BOOL arcCheckIfBackupNeeded(HWND hwndNotify,        // in: window to notify
                            ULONG ulMsg)            // in: msg to post to hwndNotify
{
    BOOL    fBackup = FALSE,
            fDisableArchiving = FALSE;
    CHAR    szTemp[300];

    // force loading of settings
    arcQuerySettings();

    if (G_ArcSettings.ulArcFlags & ARCF_ENABLED)
    {
        HWND    hwndStatus = NULLHANDLE;
        XSTRING strMsg;

        CHAR    lRestoredArchiveNumber = 0;

        xstrInit(&strMsg, 300);

        if (G_ArcSettings.fShowStatus)
        {
            ArcStatusIcon.pcszText = (PCSZ)cmnQueryDlgIcon();

            // format the status window
            // V0.9.16 (2001-11-19) [umoeller]
            dlghCreateDlg(&hwndStatus,
                          NULLHANDLE,
                          FCF_TITLEBAR | /* FCF_SYSMENU | */ FCF_DLGBORDER | FCF_NOBYTEALIGN,
                          WinDefDlgProc,
                          cmnGetString(ID_XFD_ARCHIVINGSTATUS),
                          dlgArcStatus,
                          ARRAYITEMCOUNT(dlgArcStatus),
                          NULL,
                          cmnQueryDefaultFont());
        }

        // changed V0.9.13 (2001-06-14) [umoeller]:
        // get the "restored archive" byte
        arcQueryArchiveByte(&lRestoredArchiveNumber,
                            ARCOFS_JUSTRESTORED);

        if (lRestoredArchiveNumber)
        {
            // archive was just restored:
            // disable archiving then
            sprintf(szTemp,
                    cmnGetString(ID_XSSI_ARCRESTORED),      // archive %d was restored
                    lRestoredArchiveNumber);
            xstrcpy(&strMsg, szTemp, 0);
            fBackup = FALSE;
            fDisableArchiving = TRUE;
            arcSetArchiveByte(0, ARCOFS_JUSTRESTORED);
        } // end V0.9.13 (2001-06-14) [umoeller]
        else
        {
            // no archive was just restored:
            if (G_ArcSettings.ulArcFlags & ARCF_ALWAYS)
            {
                fBackup = TRUE;

                if (G_ArcSettings.fShowStatus)
                    ShowStatus(hwndStatus,
                               cmnGetString(ID_XSSI_ARCENABLED));
            }
            else
            {
                BOOL    fCheckINIs = (G_ArcSettings.ulArcFlags & ARCF_INI);

                if (G_ArcSettings.ulArcFlags & ARCF_NEXT)
                {
                    if (G_ArcSettings.fShowStatus)
                        ShowStatus(hwndStatus,
                                   cmnGetString(ID_XSSI_ARCENABLEDONCE));

                    fBackup = TRUE;
                    fCheckINIs = FALSE;     // not necessary

                    // unset this settings flag for next time
                    G_ArcSettings.ulArcFlags &= ~ARCF_NEXT;
                    arcSaveSettings();
                }
                else if (G_ArcSettings.ulArcFlags & ARCF_DAYS)
                {
                    // days-based:
                    DATETIME    dtNow;
                    LONG        lDaysNow,
                                lDaysLast,
                                lDaysPassed;

                    DosGetDateTime(&dtNow);
                    lDaysNow = dtDate2Scalar(dtNow.year,
                                             dtNow.month,
                                             dtNow.day);
                    lDaysLast= dtDate2Scalar(G_dtLastArchived.year,
                                             G_dtLastArchived.month,
                                             G_dtLastArchived.day);
                    lDaysPassed = lDaysNow - lDaysLast;

                    if (lDaysPassed >= G_ArcSettings.ulEveryDays)
                    {
                        fBackup = TRUE;
                        fCheckINIs = FALSE;     // not necessary
                    }

                    if (G_ArcSettings.fShowStatus)
                    {
                        sprintf(szTemp,
                                cmnGetString(ID_XSSI_ARCDAYSPASSED),  // "%d days passed since last backup." // pszArcDaysPassed
                                lDaysPassed);
                        xstrcpy(&strMsg, szTemp, 0);
                        xstrcatc(&strMsg, '\n');
                        sprintf(szTemp,
                                cmnGetString(ID_XSSI_ARCDAYSLIMIT),  // "Limit: %d days." // pszArcDaysLimit
                                G_ArcSettings.ulEveryDays);
                        xstrcat(&strMsg, szTemp, 0);
                        xstrcatc(&strMsg, '\n');
                    }
                }

                if (fCheckINIs)
                {
                    // INI-based:
                    double  dMaxDifferencePercent = 0;

                    if (G_ArcSettings.fShowStatus)
                    {
                        sprintf(szTemp,
                                "%s\n",
                                cmnGetString(ID_XSSI_ARCINICHECKING)) ; // pszArcINIChecking
                        ShowStatus(hwndStatus,
                                   szTemp);
                    }

                    fBackup = arcCheckINIFiles(&G_ArcSettings.dIniFilesPercent,
                                               (PSZ)INIAPP_XWORKPLACE,  // ignore this
                                               &G_ArcSettings.dAppsSizeLast,
                                               &G_ArcSettings.dKeysSizeLast,
                                               &G_ArcSettings.dDataSumLast,
                                               &dMaxDifferencePercent);

                    if (G_ArcSettings.fShowStatus)
                    {
                        sprintf(szTemp,
                                cmnGetString(ID_XSSI_ARCINICHANGED),  // "INI files changed %f %%", // pszArcINIChanged
                                dMaxDifferencePercent);
                        xstrcpy(&strMsg, szTemp, 0);
                        xstrcatc(&strMsg, '\n');
                        sprintf(szTemp,
                                cmnGetString(ID_XSSI_ARCINILIMIT),  // "Limit: %f %%." // pszArcINILimit
                                G_ArcSettings.dIniFilesPercent);
                        xstrcat(&strMsg, szTemp, 0);
                        xstrcatc(&strMsg, '\n');
                    }
                } // end if (fCheckINIs)
            } // end else if (G_ArcSettings.ulArcFlags & ARCF_ALWAYS)
        } // else if (fWasJustRestored)

        if (strMsg.ulLength)
        {
            if (G_ArcSettings.fShowStatus)
            {
                if (fBackup)
                {
                    // archiving to be turned on:
                    // save "last app" etc. data so we won't get this twice
                    arcSaveSettings();
                    xstrcat(&strMsg,
                            cmnGetString(ID_XSSI_ARCENABLED),  // "Desktop archiving enabled"
                            0);
                }
                else
                    xstrcat(&strMsg,
                            cmnGetString(ID_XSSI_ARCNOTNECC),  // "Desktop archiving not necessary"
                            0);

                ShowStatus(hwndStatus,
                           strMsg.psz);
            }
        }

        xstrClear(&strMsg);

        if (G_ArcSettings.fShowStatus)
            WinPostMsg(hwndNotify,
                       ulMsg,
                       (MPARAM)hwndStatus,
                       (MPARAM)NULL);
    }

    arcSwitchArchivingOn(fBackup); // moved V0.9.4 (2000-07-22) [umoeller]

    if (fDisableArchiving)
    {
        G_ArcSettings.ulArcFlags &= ~ARCF_ENABLED;
        arcSaveSettings();
        cmnMessageBoxExt(NULLHANDLE,
                         116,
                         NULL, 0,
                         188,
                         MB_OK);
    }

    return fBackup;
}

/*
         *@@ arcSwitchArchivingOn:
 *      depending on fSwitchOn, this switches Desktop archiving on or off
 *      by manipulating the \OS2\BOOT\ARCHBASE.$$$ file.
 *
 *      In addition, this stores the date of the last archive in OS2.INI
 *      and creates a file on the desktop to mark this as an archive.
 *
 *      This should only be called by arcCheckIfBackupNeeded.
 *
 *@@changed V0.9.13 (2001-06-14) [umoeller]: optimized
 *@@changed V0.9.13 (2001-06-14) [smilcke]: when archiving is switched on we have to clear the restore flag
 */

APIRET arcSwitchArchivingOn(BOOL fSwitchOn)
{
    APIRET arc = NO_ERROR;

    if (fSwitchOn)
    {
        if (!(arc = arcSetArchiveByte(1, ARCOFS_ARCHIVINGENABLED)))
        {
            // reset the restore flag V0.9.13 (2001-06-14) [smilcke]
            arc = arcSetArchiveByte(0, ARCOFS_JUSTRESTORED);

            PMPF_STARTUP(("WPS Archiving activated"));

            // store date of backup in OS2.INI
            DosGetDateTime(&G_dtLastArchived);
            PrfWriteProfileData(HINI_USER,
                                (PSZ)INIAPP_XWORKPLACE,
                                (PSZ)INIKEY_ARCHIVE_LASTBACKUP,
                                &G_dtLastArchived,
                                sizeof(G_dtLastArchived));
        }
    }
    else
    {
        arc = arcSetArchiveByte(0, ARCOFS_ARCHIVINGENABLED);

        PMPF_STARTUP(("WPS Archiving deactivated"));
    }

    if (arc)
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
               "Got error %d", arc);

    return arc;
}

/*
 *@@ arcSetNumArchives:
 *      queries or sets the maximum no. of archives
 *      which are maintained by the WPS.
 *
 *      If (fSet == TRUE), the number of archives is set
 *      to *pcArchives. This must be > 0 and < 10, otherwise
 *      FALSE is returned.
 *
 *      If (fSet == FALSE), the current number of archives
 *      is queried and written into *pcArchives, but not
 *      changed.
 *
 *@@changed V0.9.12 (2001-05-24) [smilcke]: code optimized to use arcSetByte/arcGetByte
 */

BOOL arcSetNumArchives(PCHAR pcArchives,        // in/out: number of archives
                       BOOL fSet)               // if TRUE, archive number is set
{
    BOOL            brc = FALSE;

    if (pcArchives)
        if (    (fSet)
             && (    (*pcArchives < 0)
                  || (*pcArchives > 9)
                )
           )
        {
            brc = FALSE;
        }
        else
        {
            CHAR cArchives = 0;
            arcQueryArchiveByte(&cArchives,
                                ARCOFS_MAXARCHIVES);
            if (fSet)
                arcSetArchiveByte(*pcArchives,
                                  ARCOFS_MAXARCHIVES);
            *pcArchives = cArchives;
        }

    return brc;
}

/*
 *@@ arcCheckINIFiles:
 *      this function goes thru both OS2.INI and OS2SYS.INI and
 *      checks for changes. This gets called from arcCheckIfBackupNeeded.
 *
 *      To find this out, we check the following data:
 *      --  the size of the applications list;
 *      --  the size of the keys list;
 *      --  the actual profile data, for which all values are summed up.
 *
 *      If any of these values differs more than cPercent from the old
 *      data passed to this function, we update that data and return TRUE,
 *      which means that Desktop archiving should be turned on.
 *
 *      Otherwise FALSE is returned, and the data is not changed.
 *
 *      In any case, *pcMaxDifferencePercent is set to the maximum
 *      percentage of difference which was computed.
 *
 *      If (cPercent == 0), all data is checked for, and TRUE is always
 *      returned. This might be useful for retrieving the "last app" etc.
 *      data once.
 *
 *      NOTE: It is the responsibility of the caller to save the
 *      old data somewhere. This function does _not_ check the
 *      INI files.
 *
 *      Set pszIgnoreApp to any INI application which should be ignored
 *      when checking for changes. This is useful if the "last app" etc.
 *      data is stored in the profiles too. If (pszIgnoreApp == NULL),
 *      all applications are checked for.
 */

BOOL arcCheckINIFiles(double* pdPercent,
                      PSZ pszIgnoreApp,        // in: this application will not be checked
                      double* pdAppsSizeLast,
                      double* pdKeysSizeLast,
                      double* pdDataSumLast,
                      double* pdMaxDifferencePercent) // out: maximum difference found
{
    BOOL            brc = FALSE;        // return value
    double          dDataSum = 0,
                    dTotalAppsSize = 0,
                    dTotalKeysSize = 0;
    double          dMaxDifferencePercent = 0;

    // 1) Applications loop
    APIRET          arc;
    PSZ pszAppsList = NULL;

    PMPF_STARTUP(("Checking INI files"));

    if (!(arc = prfhQueryKeysForApp(HINI_PROFILE, // both OS2.INI and OS2SYS.INI
                                    NULL,        // return applications
                                    &pszAppsList)))
    {
        PSZ pApp2 = pszAppsList;

        while (*pApp2 != 0)
        {
            BOOL    fIgnore = FALSE;
            // pApp2 has the current app now

            // ignore this app?
            if (pszIgnoreApp)
                if (!strcmp(pszIgnoreApp, pApp2))
                    fIgnore = TRUE;

            if (!fIgnore)
            {
                // 2) keys loop for this app
                PSZ pszKeysList = NULL;
                if (!(arc = prfhQueryKeysForApp(HINI_PROFILE, // both OS2.INI and OS2SYS.INI
                                                pApp2,        // return keys
                                                &pszKeysList)))
                {
                    PSZ pKey2 = pszKeysList;

                    while (*pKey2 != 0)
                    {
                        // pKey2 has the current key now

                        // 3) get key data
                        ULONG   cbData = 0;
                        PSZ pszData = prfhQueryProfileData(HINI_PROFILE,
                                                           pApp2,
                                                           pKey2,
                                                           &cbData);
                        if (pszData)
                        {
                            // sum up all values
                            PSZ     p = pszData;
                            ULONG   ul = 0;
                            for (ul = 0;
                                 ul < cbData;
                                 ul++, p++)
                                dDataSum += (double)*p;

                            free(pszData);
                        }

                        pKey2 += strlen(pKey2)+1; // next key
                    } // end while (*pKey2 != 0)

                    // add size of keys list to total size
                    dTotalKeysSize += (pKey2 - pszKeysList);

                    free(pszKeysList);
                } // end if (pszKeysList)
            } // end if (!fIgnore)

            pApp2 += strlen(pApp2)+1; // next app
        } // end while (*pApp2 != 0)

        // add size of apps list to total size
        dTotalAppsSize += (pApp2 - pszAppsList);

        free(pszAppsList);
    } // end if (pszAppsList)

    if (*pdPercent != 0)
    {
        #ifdef __DEBUG__
            CHAR szTemp[1000];

            PMPF_STARTUP(("Last, now"));
            sprintf(szTemp, "Apps size: %f, %f", *pdAppsSizeLast, dTotalAppsSize);
            PMPF_STARTUP(("%s", szTemp));
            sprintf(szTemp, "Keys size: %f, %f", *pdKeysSizeLast, dTotalKeysSize);
            PMPF_STARTUP(("%s", szTemp));
            sprintf(szTemp, "Data sum: %f, %f", *pdDataSumLast, dDataSum);
            PMPF_STARTUP(("%s", szTemp));
        #endif

        if ((*pdAppsSizeLast) && (dTotalAppsSize))
        {
            // if so, check if more than the given percentage of application strings
            // where modified
            double dPercentThis =
                        fabs(dTotalAppsSize - *pdAppsSizeLast)     // difference (we need fabs, abs returns 0 always)
                        * 100                               // in percent
                        / *pdAppsSizeLast;

            #ifdef __DEBUG__
                sprintf(szTemp, "%f", dPercentThis);
                PMPF_STARTUP(("dPercent Apps: %s", szTemp));
            #endif

            if (dPercentThis > *pdPercent)
            {
                // yes: store new value (for later writing to log-file)
                // and switch archiving on
                *pdAppsSizeLast = dTotalAppsSize;
                brc = TRUE;
            }

            if (dPercentThis > dMaxDifferencePercent)
                dMaxDifferencePercent = dPercentThis;
        }

        // same logic for keys size
        if ((*pdKeysSizeLast) && (dTotalKeysSize))
        {
            double dPercentThis =
                        fabs(dTotalKeysSize - *pdKeysSizeLast)     // difference (we need fabs, abs returns 0 always)
                        * 100                               // in percent
                        / *pdKeysSizeLast;

            #ifdef __DEBUG__
                sprintf(szTemp, "%f", dPercentThis);
                PMPF_STARTUP(("dPercent Keys: %s", szTemp));
            #endif

            if (dPercentThis > *pdPercent)
            {
                // yes: store new value (for later writing to log-file)
                // and switch archiving on
                *pdKeysSizeLast = dTotalKeysSize;
                brc = TRUE;
            }

            if (dPercentThis > dMaxDifferencePercent)
                dMaxDifferencePercent = dPercentThis;
        }

        // same logic for profile data
        if ((*pdDataSumLast) && (dDataSum))
        {
            double dPercentThis =
                        fabs(dDataSum - *pdDataSumLast)     // difference (we need fabs, abs returns 0 always)
                        * 100                               // in percent
                        / *pdDataSumLast;

            #ifdef __DEBUG__
                sprintf(szTemp, "%f", dPercentThis);
                PMPF_STARTUP(("dPercent Data: %s", szTemp));
            #endif

            if (dPercentThis > *pdPercent)
            {
                // yes: store new value (for later writing to log-file)
                // and switch archiving on
                *pdDataSumLast = dDataSum;
                brc = TRUE;
            }

            if (dPercentThis > dMaxDifferencePercent)
                dMaxDifferencePercent = dPercentThis;
        }

        if (pdMaxDifferencePercent)
            *pdMaxDifferencePercent = dMaxDifferencePercent;

    } // end if (cPercent)

    // check if any size is 0 so the values MUST be written and archiving MUST
    // be switched on
    if (!*pdAppsSizeLast || !*pdKeysSizeLast || !*pdDataSumLast)
    {
        *pdAppsSizeLast = dTotalAppsSize;
        *pdKeysSizeLast = dTotalKeysSize;
        *pdDataSumLast = dDataSum;
        brc = TRUE;
    }

    PMPF_STARTUP(("Done checking INI files, returning %d", brc));

    return brc;
}


