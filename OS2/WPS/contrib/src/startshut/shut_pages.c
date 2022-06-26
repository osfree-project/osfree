
/*
 *@@sourcefile shut_pages.c:
 *      notebook pages for XShutdown configuration (in
 *      the Desktop's settings notebook).
 *
 *      This has been separated from shutdown.c with
 *      V1.0.0 (2002-09-13) [umoeller].
 *
 *
 *      All the functions in this file have the xsd* prefix.
 *
 *@@header "startshut\shutdown.h"
 */

/*
 *      Copyright (C) 1997-2006 Ulrich M”ller.
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

#define INCL_DOSPROCESS
#define INCL_DOSSESMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINFRAMEMGR
#define INCL_WINDIALOGS
#define INCL_WINPOINTERS
#define INCL_WINSHELLDATA
#define INCL_WINPROGRAMLIST
#define INCL_WINSWITCHLIST
#define INCL_WINCOUNTRY
#define INCL_WINSYS
#define INCL_WINMENUS
#define INCL_WINSTATICS
#define INCL_WINENTRYFIELDS
#define INCL_WINBUTTONS
#define INCL_WINLISTBOXES
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h
#include <stdarg.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\animate.h"            // icon and other animations
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\exeh.h"               // executable helpers
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\procstat.h"           // DosQProcStat handling
#include "helpers\standards.h"          // some standard macros
#include "helpers\threads.h"            // thread helpers
#include "helpers\winh.h"               // PM helper routines
#include "helpers\wphandle.h"           // file-system object handles
#include "helpers\xprf.h"               // replacement profile (INI) functions

// SOM headers which don't crash with prec. header files
#include "xfldr.ih"                     // needed for shutdown folder

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\notebook.h"            // generic XWorkplace notebook handling
#include "xwpapi.h"                     // public XWorkplace definitions

// headers in /hook
#include "hook\xwphook.h"

#include "filesys\object.h"             // XFldObject implementation
#include "filesys\xthreads.h"           // extra XWorkplace threads

#include "media\media.h"                // XWorkplace multimedia support

#include "helpers\xwpsecty.h"           // XWorkplace Security base

#include "startshut\apm.h"              // APM power-off for XShutdown
#include "startshut\archives.h"         // archiving declarations
#include "startshut\shutdown.h"         // XWorkplace eXtended Shutdown

// other SOM headers
#pragma hdrstop
#include <wpdesk.h>                     // WPDesktop; includes WPFolder also
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)

/* ******************************************************************
 *
 *   Shutdown settings pages
 *
 ********************************************************************/

#ifndef __NOXSHUTDOWN__

#define COLUMN_WIDTH        100

static const CONTROLDEF
    ShutdownGroup = LOADDEF_GROUP(ID_SDDI_SHUTDOWNGROUP, SZL_AUTOSIZE),
#ifndef __EASYSHUTDOWN__
    RebootAfterwardsCB = CONTROLDEF_AUTOCHECKBOX(
                            LOAD_STRING, // "~Reboot afterwards"
                            ID_SDDI_REBOOT,
                            COLUMN_WIDTH,
                            -1),
#endif
    RebootActionsButton = CONTROLDEF_PUSHBUTTON(
                            LOAD_STRING, // "Reboot actio~ns..."
                            ID_SDDI_REBOOTEXT,
                            COLUMN_WIDTH,
                            STD_BUTTON_HEIGHT),
    CanDesktopAltF4 = LOADDEF_AUTOCHECKBOX(ID_SDDI_CANDESKTOPALTF4),
    AnimationTxt = LOADDEF_TEXT(ID_SDDI_ANIMATE_TXT),
    AnimationBeforeShutdownCB = LOADDEF_AUTOCHECKBOX(ID_SDDI_ANIMATE_SHUTDOWN),
    AnimationBeforeRebootCB = LOADDEF_AUTOCHECKBOX(ID_SDDI_ANIMATE_REBOOT),
    PowerOffCB = CONTROLDEF_AUTOCHECKBOX(
                            LOAD_STRING, // "Power-off computer"
                            ID_SDDI_POWEROFF,
                            COLUMN_WIDTH,
                            -1),
    PowerOffDelayCB = LOADDEF_AUTOCHECKBOX(ID_SDDI_DELAY),
    APMPowerOffRadio = LOADDEF_FIRST_AUTORADIO(ID_SDDI_APMPOWEROFF),
    ACPIPowerOffRadio = LOADDEF_NEXT_AUTORADIO(ID_SDDI_ACPIPOWEROFF),
    APMLevelText1 = LOADDEF_TEXT(ID_SDDI_APMVERSION_TXT),
    APMLevelText2 = LOADDEF_TEXT(ID_SDDI_APMVERSION),
    ACPILevelText1 = LOADDEF_TEXT(ID_SDDI_ACPIVERSION_TXT),
    ACPILevelText2 = LOADDEF_TEXT(ID_SDDI_ACPIVERSION),
#ifndef __EASYSHUTDOWN__
    SaveINIFilesText = LOADDEF_TEXT(ID_SDDI_SAVEINIS_TXT),
    SaveINIFilesCombo = CONTROLDEF_DROPDOWNLIST(
                            ID_SDDI_SAVEINIS_LIST,
                            150, 40),
#endif
    SharedGroup = LOADDEF_GROUP(ID_SDDI_SHAREDGROUP, SZL_AUTOSIZE),
    EmptyTrashCB = LOADDEF_AUTOCHECKBOX(ID_SDDI_EMPTYTRASHCAN),
#ifndef __EASYSHUTDOWN__
    ConfirmCB = LOADDEF_AUTOCHECKBOX(ID_SDDI_CONFIRM),
    WarpCenterFirstCB = LOADDEF_AUTOCHECKBOX(ID_SDDI_WARPCENTERFIRST),
#endif
    AutoCloseVIOCB = CONTROLDEF_AUTOCHECKBOX(
                            LOAD_STRING, // "Auto-close non-~PM sessions"
                            ID_SDDI_AUTOCLOSEVIO,
                            COLUMN_WIDTH,
                            -1),
    AutoCloseButton = CONTROLDEF_PUSHBUTTON(
                            LOAD_STRING, // "Auto-close det~ails..."
                            ID_SDDI_AUTOCLOSEDETAILS,
                            COLUMN_WIDTH,
                            STD_BUTTON_HEIGHT),
    LogCB = LOADDEF_AUTOCHECKBOX(ID_SDDI_LOG),
    CreateShutdownFldrButton = CONTROLDEF_PUSHBUTTON(
                            LOAD_STRING, // "Create XShutdown folder"
                            ID_SDDI_CREATESHUTDOWNFLDR,
                            -1,
                            STD_BUTTON_HEIGHT);

static const DLGHITEM dlgShutdown[] =
    {
        START_TABLE,            // root table, required
            START_ROW(0),       // shared settings group
                START_GROUP_TABLE(&SharedGroup),
                    START_ROW(0),
                        CONTROL_DEF(&EmptyTrashCB),
#ifndef __EASYSHUTDOWN__
                    START_ROW(0),
                        CONTROL_DEF(&ConfirmCB),
                    START_ROW(0),
                        CONTROL_DEF(&WarpCenterFirstCB),
#endif
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&AutoCloseVIOCB),
                        CONTROL_DEF(&AutoCloseButton),
                    START_ROW(0),
                        CONTROL_DEF(&LogCB),
                    START_ROW(0),
                        CONTROL_DEF(&CreateShutdownFldrButton),
                END_TABLE,      // end of group
            START_ROW(0),       // shutdown only group
                // create group on top
                START_GROUP_TABLE(&ShutdownGroup),
                    START_ROW(0),
                        CONTROL_DEF(&CanDesktopAltF4),
#ifndef __EASYSHUTDOWN__
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&RebootAfterwardsCB),
                        CONTROL_DEF(&RebootActionsButton),
#endif
                    START_ROW(0),
                        CONTROL_DEF(&AnimationTxt),
                        CONTROL_DEF(&AnimationBeforeShutdownCB),
                        CONTROL_DEF(&AnimationBeforeRebootCB),
                    START_ROW(0),
                        START_TABLE,
                            START_ROW(0),
                                CONTROL_DEF(&PowerOffCB),
                            START_ROW(0),
                                CONTROL_DEF(&G_Spacing),        // notebook.c
                                CONTROL_DEF(&PowerOffDelayCB),
                        END_TABLE,
                        START_TABLE_ALIGN,
                            START_ROW(ROW_VALIGN_CENTER),
                                CONTROL_DEF(&APMPowerOffRadio),
                                CONTROL_DEF(&APMLevelText1),
                                CONTROL_DEF(&APMLevelText2),
                            START_ROW(ROW_VALIGN_CENTER),
                                CONTROL_DEF(&ACPIPowerOffRadio),
                                CONTROL_DEF(&ACPILevelText1),
                                CONTROL_DEF(&ACPILevelText2),
                        END_TABLE,
#ifndef __EASYSHUTDOWN__
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&SaveINIFilesText),
                        CONTROL_DEF(&SaveINIFilesCombo),
#endif
                END_TABLE,      // end of group
            START_ROW(0),       // notebook buttons (will be moved)
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE
    };

static const XWPSETTING G_ShutdownBackup[] =
    {
        sflXShutdown,
#ifndef __EASYSHUTDOWN__
        sulSaveINIS,
#endif
    };

/*
 * xsdShutdownInitPage:
 *      notebook callback function (notebook.c) for the
 *      "XShutdown" page in the Desktop's settings
 *      notebook.
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.2 (2000-03-04) [umoeller]: added "APM delay" support
 *@@changed V0.9.3 (2000-05-22) [umoeller]: added animate on reboot
 *@@changed V0.9.4 (2000-08-03) [umoeller]: added "empty trash can"
 *@@changed V0.9.9 (2001-04-07) [pr]: added missing Undo and Default processing
 *@@changed V0.9.16 (2001-10-08) [umoeller]: now using dialog formatter
 *@@changed V0.9.16 (2002-01-04) [umoeller]: added "alt+f4 on desktop starts shutdown"
 *@@changed V1.0.5 (2006-06-26) [pr]: added ACPI shutdown support
 *@@changed V1.0.6 (2006-08-20) [pr]: update ACPI and power off settings @@fixes 728
 */

VOID xsdShutdownInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                         ULONG flFlags)        // CBI_* flags (notebook.h)
{
    ULONG fl = cmnQuerySetting(sflXShutdown);

    if (flFlags & CBI_INIT)
    {
        HWND        hwndINICombo = NULLHANDLE;
        ULONG       ul;
        ULONG       aulIniStrings[3] =
            {
                ID_XSSI_XSD_SAVEINIS_NEW,   // pszXSDSaveInisNew
                ID_XSSI_XSD_SAVEINIS_OLD,   // pszXSDSaveInisOld
                ID_XSSI_XSD_SAVEINIS_NONE   // pszXSDSaveInisNone
            };

        // first call: backup Global Settings for "Undo" button;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        pnbp->pUser = cmnBackupSettings(G_ShutdownBackup,
                                         ARRAYITEMCOUNT(G_ShutdownBackup));

        // insert the controls using the dialog formatter
        // V0.9.16 (2001-10-08) [umoeller]
        ntbFormatPage(pnbp->hwndDlgPage,
                      dlgShutdown,
                      ARRAYITEMCOUNT(dlgShutdown));

        // V1.0.5 (2006-06-26) [pr]
        WinSetDlgItemText(pnbp->hwndDlgPage, ID_SDDI_APMVERSION, apmQueryVersion());
        WinSetDlgItemText(pnbp->hwndDlgPage, ID_SDDI_ACPIVERSION, acpiQueryVersion());
#ifndef __EASYSHUTDOWN__
        hwndINICombo = WinWindowFromID(pnbp->hwndDlgPage, ID_SDDI_SAVEINIS_LIST);
        for (ul = 0;
             ul < 3;
             ul++)
        {
            WinInsertLboxItem(hwndINICombo,
                              ul,
                              cmnGetString(aulIniStrings[ul]));
        }
#endif
    }

    if (flFlags & CBI_SET)
    {
        BOOL bAPM = apmPowerOffSupported();
        BOOL bACPI = acpiPowerOffSupported();

#ifndef __EASYSHUTDOWN__
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SDDI_REBOOT,
                              (fl & XSD_REBOOT) != 0);
#endif
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SDDI_CANDESKTOPALTF4,
                              (fl & XSD_CANDESKTOPALTF4) != 0);
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SDDI_ANIMATE_SHUTDOWN,
                              (fl & XSD_ANIMATE_SHUTDOWN) != 0);
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SDDI_ANIMATE_REBOOT,
                              (fl & XSD_ANIMATE_REBOOT) != 0);
        // V1.0.6 (2006-08-20) [pr]: update settings
        if ((fl & XSD_POWEROFF) && !bAPM && !bACPI)
        {
            fl &= ~XSD_POWEROFF;
            cmnSetSetting(sflXShutdown, fl);
        }

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SDDI_POWEROFF,
                              (fl & XSD_POWEROFF) != 0);
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SDDI_DELAY,
                              (fl & XSD_DELAY) != 0);
        // V1.0.6 (2006-08-20) [pr]: update settings @@fixes 728
        if (fl & XSD_ACPIPOWEROFF)
        {
            if (!bACPI)
            {
                fl &= ~XSD_ACPIPOWEROFF;
                cmnSetSetting(sflXShutdown, fl);
            }
        }
        else
        {
            if (bACPI && !bAPM)
            {
                fl |= XSD_ACPIPOWEROFF;
                cmnSetSetting(sflXShutdown, fl);
            }
        }

        winhSetDlgItemChecked(pnbp->hwndDlgPage,
                              ((fl & XSD_ACPIPOWEROFF) != 0)
                                  ? ID_SDDI_ACPIPOWEROFF
                                  : ID_SDDI_APMPOWEROFF,
                              TRUE);
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SDDI_EMPTYTRASHCAN,
                              (fl & XSD_EMPTY_TRASH) != 0);
#ifndef __EASYSHUTDOWN__
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SDDI_CONFIRM,
                              ((fl & XSD_NOCONFIRM) == 0));
#endif
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SDDI_AUTOCLOSEVIO,
                              (fl & XSD_AUTOCLOSEVIO) != 0);
#ifndef __EASYSHUTDOWN__
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SDDI_WARPCENTERFIRST,
                              (fl & XSD_WARPCENTERFIRST) != 0);
#endif
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_SDDI_LOG,
                              (fl & XSD_LOG) != 0);

#ifndef __EASYSHUTDOWN__
        WinSendDlgItemMsg(pnbp->hwndDlgPage, ID_SDDI_SAVEINIS_LIST,
                          LM_SELECTITEM,
                          (MPARAM)(cmnQuerySetting(sulSaveINIS)),
                          (MPARAM)TRUE);        // select
#endif
    }

    if (flFlags & CBI_ENABLE)
    {
        PCKERNELGLOBALS pKernelGlobals = krnQueryGlobals();
        BOOL fXShutdownValid = TRUE; // (cmnQuerySetting(sNoWorkerThread) == 0);
        BOOL fXShutdownEnabled =
                (   (fXShutdownValid)
                 && (cmnQuerySetting(sfXShutdown))
                );
        BOOL fXShutdownOrWPSValid =
                (   (   (cmnQuerySetting(sfXShutdown))
                     || (cmnQuerySetting(sfRestartDesktop))
                    )
                 // && (cmnQuerySetting(sNoWorkerThread) == 0)
                );
        BOOL bAPM = apmPowerOffSupported();
        BOOL bACPI = acpiPowerOffSupported();
        BOOL bEnable = fXShutdownEnabled && (bAPM || bACPI);

        // WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_ENABLED, fXShutdownValid);
        WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_CANDESKTOPALTF4, fXShutdownEnabled);

#ifndef __EASYSHUTDOWN__
        WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_REBOOT, fXShutdownEnabled);
#endif
        WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_REBOOTEXT, fXShutdownEnabled);

        WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_ANIMATE_SHUTDOWN, fXShutdownEnabled);
        WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_ANIMATE_REBOOT, fXShutdownEnabled);

        // V1.0.5 (2006-06-26) [pr]
        WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_POWEROFF, bEnable);
        bEnable = bEnable && ((fl & XSD_POWEROFF) != 0);
        WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_DELAY, bEnable);
        WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_APMPOWEROFF, bEnable && bAPM);
        WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_ACPIPOWEROFF, bEnable && bACPI);

        WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_EMPTYTRASHCAN,
                          ( fXShutdownEnabled && (cmnTrashCanReady()) ) );

#ifndef __EASYSHUTDOWN__
        WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_CONFIRM, fXShutdownOrWPSValid);
#endif
        WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_AUTOCLOSEVIO, fXShutdownOrWPSValid);
        WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_AUTOCLOSEDETAILS, fXShutdownOrWPSValid);

        // enable "warpcenter first" if shutdown or WPS have been enabled
        // AND if the WarpCenter was found
#ifndef __EASYSHUTDOWN__
        WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_WARPCENTERFIRST,
                         (    (fXShutdownOrWPSValid)
                           && (G_pAwakeWarpCenter != NULL)
                                    // global variable (xfobj.c, kernel.h) V0.9.20 (2002-07-25) [umoeller]
                         ));
#endif

        WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_LOG, fXShutdownOrWPSValid);

#ifndef __EASYSHUTDOWN__
        WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_SAVEINIS_TXT, fXShutdownEnabled);
        WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_SAVEINIS_LIST, fXShutdownEnabled);
#endif

        if (WinQueryObject((PSZ)XFOLDER_SHUTDOWNID))
            // shutdown folder exists already: disable button
            WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_CREATESHUTDOWNFLDR, FALSE);
    }
}

/*
 * xsdShutdownItemChanged:
 *      notebook callback function (notebook.c) for the
 *      "XShutdown" page in the Desktop's settings
 *      notebook.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.2 (2000-03-04) [umoeller]: added "APM delay" support
 *@@changed V0.9.3 (2000-05-22) [umoeller]: added animate on reboot
 *@@changed V0.9.4 (2000-08-03) [umoeller]: added "empty trash can"
 *@@changed V0.9.9 (2001-04-07) [pr]: added missing Undo and Default processing
 *@@changed V1.0.5 (2006-06-26) [pr]: added ACPI shutdown support
 */

MRESULT xsdShutdownItemChanged(PNOTEBOOKPAGE pnbp,
                               ULONG ulItemID,
                               USHORT usNotifyCode,
                               ULONG ulExtra)      // for checkboxes: contains new state
{
    ULONG ulChange = 1;
    ULONG ulFlag = -1;
#ifndef __EASYSHUTDOWN__
    ULONG ulSaveINIS = -1;
#endif

    switch (ulItemID)
    {
#ifndef __EASYSHUTDOWN__
        case ID_SDDI_REBOOT:
            ulFlag = XSD_REBOOT;
        break;
#endif

        case ID_SDDI_CANDESKTOPALTF4:
            ulFlag = XSD_CANDESKTOPALTF4;
        break;

        case ID_SDDI_ANIMATE_SHUTDOWN:
            ulFlag = XSD_ANIMATE_SHUTDOWN;
        break;

        case ID_SDDI_ANIMATE_REBOOT:
            ulFlag = XSD_ANIMATE_REBOOT;
        break;

        case ID_SDDI_POWEROFF:
            ulFlag = XSD_POWEROFF;
        break;

        case ID_SDDI_DELAY:
            ulFlag = XSD_DELAY;
        break;

        // V1.0.5 (2006-06-26) [pr]
        case ID_SDDI_APMPOWEROFF:
            ulFlag = XSD_ACPIPOWEROFF;
            ulExtra = FALSE;
        break;

        case ID_SDDI_ACPIPOWEROFF:
            ulFlag = XSD_ACPIPOWEROFF;
            ulExtra = TRUE;
        break;

        case ID_SDDI_EMPTYTRASHCAN:
            ulFlag = XSD_EMPTY_TRASH;
        break;

#ifndef __EASYSHUTDOWN__
        case ID_SDDI_CONFIRM:
            ulFlag = XSD_NOCONFIRM;
            ulExtra = 1 - ulExtra;          // this one is reverse now
                                            // V0.9.16 (2002-01-13) [umoeller]
        break;
#endif

        case ID_SDDI_AUTOCLOSEVIO:
            ulFlag = XSD_AUTOCLOSEVIO;
        break;

#ifndef __EASYSHUTDOWN__
        case ID_SDDI_WARPCENTERFIRST:
            ulFlag = XSD_WARPCENTERFIRST;
        break;
#endif

        case ID_SDDI_LOG:
            ulFlag = XSD_LOG;
        break;

#ifndef __EASYSHUTDOWN__
        case ID_SDDI_SAVEINIS_LIST:
        {
            ULONG ul = (ULONG)WinSendDlgItemMsg(pnbp->hwndDlgPage, ID_SDDI_SAVEINIS_LIST,
                                                LM_QUERYSELECTION,
                                                MPFROMSHORT(LIT_FIRST),
                                                0);
            if (ul >= 0 && ul <= 2)
                ulSaveINIS = ul;
        }
        break;
#endif

        // Reboot Actions (Desktop page 1)
        case ID_SDDI_REBOOTEXT:
            /*
            WinDlgBox(HWND_DESKTOP,         // parent is desktop
                      pnbp->hwndFrame,                  // owner
                      (PFNWP)fnwpUserRebootOptions,     // dialog procedure
                      cmnQueryNLSModuleHandle(FALSE),
                      ID_XSD_REBOOTEXT,        // dialog resource id
                      (PVOID)NULL);            // no dialog parameters
                      */
            xsdShowRebootActions(pnbp->hwndFrame);
            ulChange = 0;
        break;

        // Auto-close details (Desktop page 1)
        case ID_SDDI_AUTOCLOSEDETAILS:
            xsdShowAutoCloseDetails(pnbp->hwndFrame);
            ulChange = 0;
        break;

        // "Create shutdown folder"
        case ID_SDDI_CREATESHUTDOWNFLDR:
        {
            CHAR    szSetup[500];
            HOBJECT hObj = 0;
            sprintf(szSetup,
                "DEFAULTVIEW=ICON;ICONVIEW=NONFLOWED,MINI;"
                "OBJECTID=%s;",
                XFOLDER_SHUTDOWNID);
            if (hObj = WinCreateObject((PSZ)G_pcszXFldShutdown,
                                       // "XFolder Shutdown",
                                       (PSZ)cmnGetString(ID_XFSI_XWPSHUTDOWNFDR),
                                            // finally localized V1.0.0 (2002-08-31) [umoeller]
                                       szSetup,
                                       (PSZ)WPOBJID_DESKTOP, // "<WP_DESKTOP>",
                                       CO_UPDATEIFEXISTS))
                WinEnableControl(pnbp->hwndDlgPage, ID_SDDI_CREATESHUTDOWNFLDR, FALSE);
            else
                cmnMessageBoxExt(pnbp->hwndFrame,
                                 104,
                                 NULL, 0,
                                 106,
                                 MB_OK);
            ulChange = 0;
        }
        break;

        case DID_UNDO:
            // "Undo" button: get pointer to backed-up Global Settings
            cmnRestoreSettings(pnbp->pUser,
                               ARRAYITEMCOUNT(G_ShutdownBackup));
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        break;

        case DID_DEFAULT:
            // set the default settings for this settings page
            // (this is in common.c because it's also used at
            // Desktop startup)
            cmnSetDefaultSettings(pnbp->inbp.ulPageID);
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        break;

        default:
            ulChange = 0;
    }

    if (   (ulFlag != -1)
#ifndef __EASYSHUTDOWN__
        || (ulSaveINIS != -1)
#endif
       )
    {
        if (ulFlag != -1)
        {
            ULONG flShutdown = cmnQuerySetting(sflXShutdown);
            if (ulExtra)
                flShutdown |= ulFlag;
            else
                flShutdown &= ~ulFlag;
            cmnSetSetting(sflXShutdown, flShutdown);
        }
#ifndef __EASYSHUTDOWN__
        if (ulSaveINIS != -1)
            cmnSetSetting(sulSaveINIS, ulSaveINIS);;
#endif
    }

    if (ulChange)
    {
        // enable/disable items
        xsdShutdownInitPage(pnbp, CBI_ENABLE);
    }

    return (MPARAM)0;
}

#endif


