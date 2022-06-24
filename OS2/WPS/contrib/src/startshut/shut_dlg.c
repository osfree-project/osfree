
/*
 *@@sourcefile shut_pages.c:
 *      dialogs for XShutdown, such as the confirmation
 *      and "reboot actions" etc. dialogs.
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
 *      Copyright (C) 1997-2012 Ulrich M”ller.
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
#include "helpers\stringh.h"            // string helper routines
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
 *   Global variables
 *
 ********************************************************************/

// shutdown animation
extern SHUTDOWNANIM     G_sdAnim = {0};

static BOOL             G_fConfirmWindowExtended = TRUE;
static BOOL             G_fConfirmDialogReady = FALSE;
static ULONG            G_ulConfirmHelpPanel = NULLHANDLE;

static PFNWP            G_pfnwpFrameOrig = NULL;
static PXBITMAP         G_pbmDim = NULLHANDLE;

/* ******************************************************************
 *
 *   XShutdown confirmation
 *
 ********************************************************************/

/*
 *@@ ReformatConfirmWindow:
 *      depending on fExtended, the shutdown confirmation
 *      dialog is extended to show the "reboot to" listbox
 *      or not.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.1 (2000-01-20) [umoeller]: reformat wasn't working right; fixed.
 */

STATIC VOID ReformatConfirmWindow(HWND hwndDlg,        // in: confirmation dlg window
                                  BOOL fExtended)      // in: if TRUE, the list box will be shown
{
    // _Pmpf(("ReformatConfirmWindow: %d, ready: %d", fExtended, G_fConfirmDialogReady));

    if (G_fConfirmDialogReady)
        if (fExtended != G_fConfirmWindowExtended)
        {
            HWND    hwndBootMgrListbox = WinWindowFromID(hwndDlg, ID_SDDI_BOOTMGR);
            SWP     swpBootMgrListbox;
            SWP     swpDlg;

            WinQueryWindowPos(hwndBootMgrListbox, &swpBootMgrListbox);
            WinQueryWindowPos(hwndDlg, &swpDlg);

            if (fExtended)
                swpDlg.cx += swpBootMgrListbox.cx;
            else
                swpDlg.cx -= swpBootMgrListbox.cx;

            WinShowWindow(hwndBootMgrListbox, fExtended);
            WinSetWindowPos(hwndDlg,
                            NULLHANDLE,
                            0, 0,
                            swpDlg.cx, swpDlg.cy,
                            SWP_SIZE);

            G_fConfirmWindowExtended = fExtended;
        }
}

/*
 * fnwpConfirm:
 *      dlg proc for XShutdown confirmation windows.
 *
 *@@changed V0.9.0 [umoeller]: redesigned the whole confirmation window.
 *@@changed V0.9.1 (2000-01-20) [umoeller]: reformat wasn't working right; fixed.
 */

STATIC MRESULT EXPENTRY fnwpConfirm(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = MPNULL;
    switch (msg)
    {
        case WM_CONTROL:
            /* _Pmpf(("WM_CONTROL %d: fConfirmWindowExtended: %d",
                    SHORT1FROMMP(mp1), fConfirmWindowExtended)); */

            if (    (SHORT2FROMMP(mp1) == BN_CLICKED)
                 || (SHORT2FROMMP(mp1) == BN_DBLCLICKED)
               )
                switch (SHORT1FROMMP(mp1)) // usItemID
                {
                    case ID_SDDI_SHUTDOWNONLY:
                    case ID_SDDI_STANDARDREBOOT:
                        ReformatConfirmWindow(hwndDlg, FALSE);
                    break;

                    case ID_SDDI_REBOOTTO:
                        ReformatConfirmWindow(hwndDlg, TRUE);
                    break;
                }

            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        break;

        case WM_HELP:
            cmnDisplayHelp(NULL,
                           G_ulConfirmHelpPanel);
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }
    return mrc;
}

/*
 *@@ fnwpDimScreen:
 *
 *@@added V0.9.16 (2002-01-04) [umoeller]
 */

STATIC MRESULT EXPENTRY fnwpDimScreen(HWND hwndFrame, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_PAINT:
        {
            HPS hps = WinBeginPaint(hwndFrame, NULLHANDLE, NULL);
            POINTL ptl = {0,0};
            if (G_pbmDim)
                WinDrawBitmap(hps,
                              G_pbmDim->hbm,
                              NULL,
                              &ptl,
                              0,
                              0,
                              DBM_NORMAL); // DBM_HALFTONE);
            WinEndPaint(hps);
        }
        break;

        case WM_DESTROY:
            gpihDestroyXBitmap(&G_pbmDim);
            mrc = G_pfnwpFrameOrig(hwndFrame, msg, mp1, mp2);
        break;

        default:
            mrc = G_pfnwpFrameOrig(hwndFrame, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ CreateDimScreenWindow:
 *
 *@@added V0.9.16 (2002-01-04) [umoeller]
 */

STATIC HWND CreateDimScreenWindow(VOID)
{
    HWND hwnd;

    FRAMECDATA  fcdata;

    HPS hpsScreen;
    RECTL rcl;
    HAB hab = winhMyAnchorBlock();

    // before capturing the screen, sleep a millisecond,
    // because otherwise windows are not fully repainted
    // if the desktop menu was open, and we get that
    // into the screenshot then... so give the windows
    // time to repaint
    winhSleep(10);

    // go capture screen
    hpsScreen = WinGetScreenPS(HWND_DESKTOP);
    rcl.xLeft = 0;
    rcl.yBottom = 0;
    rcl.xRight = G_cxScreen;
    rcl.yTop = G_cyScreen;
    if (G_pbmDim = gpihCreateBmpFromPS(hab,
                                       hpsScreen,
                                       &rcl))
    {
        POINTL  ptl = {0, 0};
        GpiMove(G_pbmDim->hpsMem, &ptl);  // still 0, 0
        ptl.x = rcl.xRight - 1;
        ptl.y = rcl.yTop - 1;
        GpiSetColor(G_pbmDim->hpsMem, RGBCOL_BLACK);
        GpiSetPattern(G_pbmDim->hpsMem, PATSYM_HALFTONE);
        GpiBox(G_pbmDim->hpsMem,
               DRO_FILL, // interior only
               &ptl,
               0, 0);    // no corner rounding
    }
    WinReleasePS(hpsScreen);

    fcdata.cb            = sizeof(FRAMECDATA);
    fcdata.flCreateFlags = FCF_NOBYTEALIGN;
    fcdata.hmodResources = NULLHANDLE;
    fcdata.idResources   = 0;

    hwnd = WinCreateWindow(HWND_DESKTOP,
                           WC_FRAME,
                           "",
                           0,       // flags
                           0,
                           0,
                           G_cxScreen,
                           G_cyScreen,
                           NULLHANDLE,
                           HWND_TOP,
                           0,
                           &fcdata,
                           NULL);

    G_pfnwpFrameOrig = WinSubclassWindow(hwnd, fnwpDimScreen);

    WinShowWindow(hwnd, TRUE);

    // WinSetSysModalWindow(HWND_DESKTOP, hwnd);
        // sysmodal isn't good or the help button
        // in the dialog can't bring up the help panel
        // V0.9.19 (2002-04-24) [umoeller]

    return hwnd;
}

/*
DLGTEMPLATE ID_SDD_CONFIRM LOADONCALL MOVEABLE DISCARDABLE
BEGIN
    DIALOG  "XWorkplace: Extended Shutdown", ID_SDD_CONFIRM, 35, 26, 288,
            103, , FCF_SYSMENU | FCF_TITLEBAR
    BEGIN
        ICON            ID_SDICON, ID_SDDI_ICON, 10, 82, 20, 16, WS_GROUP
        LTEXT           "Please select how the system shall be shut down.",
                        -1, 35, 79, 150, 19, DT_WORDBREAK
        GROUPBOX        "Reboot options", -1, 10, 40, 175, 39, NOT WS_GROUP
        AUTORADIOBUTTON "Shut down ~only", ID_SDDI_SHUTDOWNONLY, 15, 61, 100,
                        10, WS_GROUP | WS_TABSTOP
        AUTORADIOBUTTON "St~andard reboot", ID_SDDI_STANDARDREBOOT, 15, 52,
                        100, 10, WS_TABSTOP
        AUTORADIOBUTTON "Reboot ~to...", ID_SDDI_REBOOTTO, 15, 43, 100, 10,
                        WS_TABSTOP
        LISTBOX         ID_SDDI_BOOTMGR, 195, 13, 85, 84, LS_NOADJUSTPOS |
                        LS_HORZSCROLL | WS_GROUP
        DEFPUSHBUTTON   "~Shutdown", DID_OK, 10, 25, 55, 12
        PUSHBUTTON      "~Cancel", DID_CANCEL, 70, 25, 55, 12
        PUSHBUTTON      "~Help", ID_SDD_BOOTMGR, 130, 25, 55, 12, BS_HELP
        AUTOCHECKBOX    "~Empty trash can", ID_SDDI_EMPTYTRASHCAN, 10, 14,
                        175, 8, WS_GROUP
        AUTOCHECKBOX    "Show this ~dialog next time", ID_SDDI_MESSAGEAGAIN,
                        10, 6, 175, 8, WS_GROUP
    END
END
*/

#define DLG_WIDTH   (3 * STD_BUTTON_WIDTH + 2 * COMMON_SPACING + 10)

static CONTROLDEF
    TrafficLightIcon = CONTROLDEF_ICON(0, ID_SDDI_ICON),
    ConfirmShutText = CONTROLDEF_TEXT_WORDBREAK(
                            LOAD_STRING,
                            ID_SDDI_CONFIRM_TEXT,
                            DLG_WIDTH - 20),
    RebootOptGroup = LOADDEF_GROUP(
                            ID_SDDI_REBOOTOPT_GROUP,
                            SZL_AUTOSIZE),
    ShutdownOnlyRadio = LOADDEF_FIRST_AUTORADIO(ID_SDDI_SHUTDOWNONLY),
    StandardRebootRadio = LOADDEF_NEXT_AUTORADIO(ID_SDDI_STANDARDREBOOT),
#ifndef __EASYSHUTDOWN__
    RebootToRadio = LOADDEF_NEXT_AUTORADIO(ID_SDDI_REBOOTTO),
#endif
    BootMgrListbox = CONTROLDEF_LISTBOX(
                            ID_SDDI_BOOTMGR,
                            100,
                            50),      // full height
#ifndef __EASYSHUTDOWN__
    MessageAgainCB = LOADDEF_AUTOCHECKBOX(ID_SDDI_MESSAGEAGAIN),
#endif
    EmptyTrashCB = LOADDEF_AUTOCHECKBOX(ID_SDDI_EMPTYTRASHCAN),
    OKButton = CONTROLDEF_DEFPUSHBUTTON(
                            0,
                            DID_OK,
                            STD_BUTTON_WIDTH,
                            STD_BUTTON_HEIGHT),
    CancelButton = CONTROLDEF_PUSHBUTTON(
                            0,
                            DID_CANCEL,
                            STD_BUTTON_WIDTH,
                            STD_BUTTON_HEIGHT),
    HelpButton = CONTROLDEF_HELPPUSHBUTTON(
                            LOAD_STRING,
                            DID_HELP,
                            STD_BUTTON_WIDTH,
                            STD_BUTTON_HEIGHT);

static const DLGHITEM dlgConfirmShutdown[] =
    {
        START_TABLE,            // root table, required
            START_ROW(ROW_VALIGN_CENTER),
                START_TABLE,
                    START_ROW(0),
                        START_TABLE,            // root table, required
                            START_ROW(ROW_VALIGN_CENTER),       // shared settings group
                                CONTROL_DEF(&TrafficLightIcon),
                                CONTROL_DEF(&ConfirmShutText),
                        END_TABLE,
                    START_ROW(0),
                        START_GROUP_TABLE_EXT(&RebootOptGroup, TABLE_INHERIT_SIZE),
                            START_ROW(0),
                                CONTROL_DEF(&ShutdownOnlyRadio),
                            START_ROW(0),
                                CONTROL_DEF(&StandardRebootRadio),
                       #ifndef __EASYSHUTDOWN__
                            START_ROW(0),
                                CONTROL_DEF(&RebootToRadio),
                       #endif
                        END_TABLE,
                    START_ROW(0),
                        CONTROL_DEF(&EmptyTrashCB),
        #ifndef __EASYSHUTDOWN__
                    START_ROW(0),
                        CONTROL_DEF(&MessageAgainCB),
        #endif
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&OKButton),
                        CONTROL_DEF(&CancelButton),
                        CONTROL_DEF(&HelpButton),
                END_TABLE,
                CONTROL_DEF(&BootMgrListbox),
        END_TABLE,
    };

/*
 *@@ xsdConfirmShutdown:
 *      this displays the eXtended Shutdown (not Restart Desktop)
 *      confirmation box. Returns MBID_YES/NO.
 *
 *@@changed V0.9.0 [umoeller]: redesigned the whole confirmation window.
 *@@changed V0.9.1 (2000-01-20) [umoeller]: reformat wasn't working right; fixed.
 *@@changed V0.9.1 (2000-01-30) [umoeller]: added exception handling.
 *@@changed V0.9.4 (2000-08-03) [umoeller]: added "empty trash can"
 *@@changed V0.9.7 (2000-12-13) [umoeller]: global settings weren't unlocked, fixed
 */

ULONG xsdConfirmShutdown(PSHUTDOWNPARAMS psdParms)
{
    ULONG       ulReturn = MBID_NO;
    BOOL        fStore = FALSE;
    HWND        hwndConfirm = NULLHANDLE,
                hwndDim;

    TRY_LOUD(excpt1)
    {
        HMODULE     hmodResource = cmnQueryNLSModuleHandle(FALSE);
        ULONG       ulKeyLength;
        PSZ         p = NULL,
                    pINI = NULL;
        ULONG       ulCheckRadioButtonID = ID_SDDI_SHUTDOWNONLY;

        HPOINTER    hptrShutdown = WinLoadPointer(HWND_DESKTOP, hmodResource,
                                                  ID_SDICON);

        hwndDim = CreateDimScreenWindow();

        G_fConfirmWindowExtended = TRUE;
        G_fConfirmDialogReady = FALSE;
        G_ulConfirmHelpPanel = ID_XSH_XSHUTDOWN_CONFIRM;

        // now using dialog formatter V1.0.0 (2002-09-17) [umoeller]
        OKButton.pcszText = cmnGetString(DID_OK);
        CancelButton.pcszText = cmnGetString(DID_CANCEL);

        if (!dlghCreateDlg(&hwndConfirm,
                           hwndDim,
                           FCF_FIXED_DLG,
                           fnwpConfirm,
                           cmnGetString(ID_SDDI_CONFIRM_TITLE),
                           dlgConfirmShutdown,
                           ARRAYITEMCOUNT(dlgConfirmShutdown),
                           NULL,
                           cmnQueryDefaultFont()))
        {
            HWND hwndAni;

            WinPostMsg(hwndConfirm,
                       WM_SETICON,
                       (MPARAM)hptrShutdown,
                       NULL);

            // set radio buttons
    #ifndef __EASYSHUTDOWN__
            winhSetDlgItemChecked(hwndConfirm, ID_SDDI_MESSAGEAGAIN, psdParms->optConfirm);
    #endif

            if (cmnTrashCanReady())
            {
                if (psdParms->optEmptyTrashCan)
                    winhSetDlgItemChecked(hwndConfirm, ID_SDDI_EMPTYTRASHCAN, TRUE);
            }
            else
                // trash can not ready: disable item
                WinEnableControl(hwndConfirm, ID_SDDI_EMPTYTRASHCAN, FALSE);

            if (psdParms->optReboot)
                ulCheckRadioButtonID = ID_SDDI_STANDARDREBOOT;

            // insert ext reboot items into combo box;
            // check for reboot items in OS2.INI
            if (PrfQueryProfileSize(HINI_USER,
                                    (PSZ)INIAPP_XWORKPLACE,
                                    (PSZ)INIKEY_BOOTMGR,
                                    &ulKeyLength))
            {
                BOOL fSelectFirst = TRUE;

                // items exist: evaluate
                if (pINI = malloc(ulKeyLength))
                {
                    PrfQueryProfileData(HINI_USER,
                                        (PSZ)INIAPP_XWORKPLACE,
                                        (PSZ)INIKEY_BOOTMGR,
                                        pINI,
                                        &ulKeyLength);
                    p = pINI;
                    while (strlen(p))
                    {
                        WinSendDlgItemMsg(hwndConfirm, ID_SDDI_BOOTMGR,
                                          LM_INSERTITEM,
                                          (MPARAM)LIT_END,
                                          (MPARAM)p);
                         // skip description string
                        p += (strlen(p)+1);
                        // skip reboot command
                        p += (strlen(p)+1);
                    }
                }

                // select reboot item from last time
                if (cmnQuerySetting(susLastRebootExt) != 0xFFFF)
                {
                    if (WinSendDlgItemMsg(hwndConfirm, ID_SDDI_BOOTMGR,
                                          LM_SELECTITEM,
                                          (MPARAM)cmnQuerySetting(susLastRebootExt), // item index
                                          (MPARAM)TRUE)) // select (not deselect)
                        // OK, don't check first item then
                        fSelectFirst = FALSE;  // V1.0.9 (2012-02-20) [pr]: @@fixes 1135

                    if (ulCheckRadioButtonID == ID_SDDI_STANDARDREBOOT)
                        ulCheckRadioButtonID = ID_SDDI_REBOOTTO;
                }

                // finally fix the non-selected reboot item
                // V1.0.0 (2002-09-17) [umoeller]
                if (fSelectFirst)
                    WinSendDlgItemMsg(hwndConfirm, ID_SDDI_BOOTMGR,
                                      LM_SELECTITEM,
                                      (MPARAM)0,
                                      (MPARAM)TRUE); // select (not deselect)
            }
            else
                // no items found: disable
                WinEnableControl(hwndConfirm, ID_SDDI_REBOOTTO, FALSE);

            // check radio button
            winhSetDlgItemChecked(hwndConfirm, ulCheckRadioButtonID, TRUE);
            winhSetDlgItemFocus(hwndConfirm, ulCheckRadioButtonID);

            // make window smaller if we don't have "reboot to"
            G_fConfirmDialogReady = TRUE;       // flag for ReformatConfirmWindow
            if (ulCheckRadioButtonID != ID_SDDI_REBOOTTO)
                ReformatConfirmWindow(hwndConfirm, FALSE);

            cmnSetControlsFont(hwndConfirm, 1, 5000);
            winhCenterWindow(hwndConfirm);      // still hidden

            xsdLoadAnimation(&G_sdAnim);
            hwndAni = WinWindowFromID(hwndConfirm, ID_SDDI_ICON);
            ctlPrepareAnimation(hwndAni,
                                XSD_ANIM_COUNT,
                                &(G_sdAnim.ahptr[0]),
                                150,    // delay
                                TRUE);  // start now

            // go!!
            ulReturn = WinProcessDlg(hwndConfirm);

            ctlStopAnimation(hwndAni);
            xsdFreeAnimation(&G_sdAnim);

            if (ulReturn == DID_OK)
            {
    #ifndef __NOXSHUTDOWN__
                ULONG flShutdown = cmnQuerySetting(sflXShutdown);

    #ifndef __EASYSHUTDOWN__
                // check "show this msg again"
                if (!(winhIsDlgItemChecked(hwndConfirm, ID_SDDI_MESSAGEAGAIN)))
                    flShutdown |= XSD_NOCONFIRM;
    #endif
    #endif

                // check empty trash
                psdParms->optEmptyTrashCan
                    = (winhIsDlgItemChecked(hwndConfirm, ID_SDDI_EMPTYTRASHCAN) != 0);

                // check reboot options
                psdParms->optReboot = FALSE;
                if (winhIsDlgItemChecked(hwndConfirm, ID_SDDI_REBOOTTO))
                {
                    USHORT usSelected = (USHORT)WinSendDlgItemMsg(hwndConfirm, ID_SDDI_BOOTMGR,
                                                                  LM_QUERYSELECTION,
                                                                  (MPARAM)LIT_CURSOR,
                                                                  MPNULL);
                    USHORT us;
                    psdParms->optReboot = TRUE;

                    p = pINI;
                    for (us = 0; us < usSelected; us++)
                    {
                        // skip description string
                        p += (strlen(p) + 1);
                        // skip reboot command
                        p += (strlen(p) + 1);
                    }
                    // skip description string to get to reboot command
                    p += (strlen(p) + 1);
                    strlcpy(psdParms->szRebootCommand,
                            p,
                            sizeof(psdParms->szRebootCommand));

    #ifndef __NOXSHUTDOWN__
                    flShutdown |= XSD_REBOOT;
    #endif
                    cmnSetSetting(susLastRebootExt, usSelected);
                }
                else if (winhIsDlgItemChecked(hwndConfirm, ID_SDDI_STANDARDREBOOT))
                {
                    psdParms->optReboot = TRUE;
                    // szRebootCommand is a zero-byte only, which will lead to
                    // the standard reboot in the Shutdown thread
    #ifndef __NOXSHUTDOWN__
                    flShutdown |= XSD_REBOOT;
    #endif
                    cmnSetSetting(susLastRebootExt, 0xFFFF);
                }
    #ifndef __NOXSHUTDOWN__
                else
                    // standard shutdown:
                    flShutdown &= ~XSD_REBOOT;
    #endif

    #ifndef __NOXSHUTDOWN__
                cmnSetSetting(sflXShutdown, flShutdown);
    #endif
            }

            if (pINI)
                free(pINI);
        }
    }
    CATCH(excpt1)
    {
    } END_CATCH();

    winhDestroyWindow(&hwndConfirm);
    winhDestroyWindow(&hwndDim);

    return ulReturn;
}

static CONTROLDEF
    ConfirmWPSText = CONTROLDEF_TEXT_WORDBREAK(
                            LOAD_STRING,
                            ID_SDDI_CONFIRMWPS_TEXT,
                            SZL_REMAINDER),         // changed V1.0.1 (2003-01-05) [umoeller]
    ConfirmLogoffText = CONTROLDEF_TEXT_WORDBREAK(
                            LOAD_STRING,
                            ID_XSSI_XSD_CONFIRMLOGOFFMSG,
                            SZL_REMAINDER),         // changed V1.0.1 (2003-01-05) [umoeller]
    CloseAllSessionsCB = LOADDEF_AUTOCHECKBOX(ID_SDDI_WPS_CLOSEWINDOWS),
#ifndef __NOXWPSTARTUP__
    RunShutdownFdr = LOADDEF_AUTOCHECKBOX(ID_SDDI_WPS_RUNSHUTDOWNFDR),
    StartupFoldersCB = LOADDEF_AUTOCHECKBOX(ID_SDDI_WPS_STARTUPFOLDER),
#endif
    ArchiveOnceCB = LOADDEF_AUTOCHECKBOX(ID_SDDI_ARCHIVEONCE);

static const DLGHITEM dlgConfirmRestartDesktopHead[] =
    {
        START_TABLE,            // root table, required
            START_ROW(ROW_VALIGN_CENTER),       // shared settings group
                CONTROL_DEF(&TrafficLightIcon),
    };

static const DLGHITEM dlgConfirmRestartDesktopOnly[] =
    {
                CONTROL_DEF(&ConfirmWPSText),
#ifndef __NOXWPSTARTUP__
            START_ROW(0),
                CONTROL_DEF(&RunShutdownFdr),
#endif
            START_ROW(0),
                CONTROL_DEF(&CloseAllSessionsCB),
#ifndef __NOXWPSTARTUP__
            START_ROW(0),
                CONTROL_DEF(&StartupFoldersCB),
#endif
            START_ROW(0),
                CONTROL_DEF(&ArchiveOnceCB),
    };

static const DLGHITEM dlgConfirmLogoffOnly[] =
    {
                CONTROL_DEF(&ConfirmLogoffText),
#ifndef __NOXWPSTARTUP__
            START_ROW(0),
                CONTROL_DEF(&RunShutdownFdr),
#endif
            START_ROW(0),
                CONTROL_DEF(&CloseAllSessionsCB),
    };

static const DLGHITEM dlgConfirmRestartDesktopTail[] =
    {
#ifndef __EASYSHUTDOWN__
            START_ROW(0),
                CONTROL_DEF(&MessageAgainCB),
#endif
            START_ROW(ROW_VALIGN_CENTER),
                CONTROL_DEF(&OKButton),
                CONTROL_DEF(&CancelButton),
                CONTROL_DEF(&HelpButton),
        END_TABLE
    };

/*
 *@@ xsdConfirmRestartWPS:
 *      this displays the "Restart Desktop" or "Logoff"
 *      confirmation box. Returns MBID_OK/CANCEL.
 *
 *@@changed V0.9.5 (2000-08-10) [umoeller]: added XWPSHELL.EXE interface
 *@@changed V0.9.16 (2002-01-13) [umoeller]: rewritten to use dialog formatter
 *@@changed V0.9.19 (2002-04-18) [umoeller]: added "archive once" feature
 *@@changed V0.9.19 (2002-04-18) [umoeller]: added a decent help panel, finally
 *@@changed V1.0.1 (2003-01-05) [umoeller]: some prettypretty for logoff
 *@@changed V1.0.1 (2003-01-29) [umoeller]: added "run shutdown folder" setting
 */

ULONG xsdConfirmRestartWPS(PSHUTDOWNPARAMS psdParms)
{
    ULONG       ulReturn = MBID_CANCEL;
    HWND        hwndConfirm;
    HMODULE     hmodResource = cmnQueryNLSModuleHandle(FALSE);

    HPOINTER    hptrShutdown = WinLoadPointer(HWND_DESKTOP,
                                              hmodResource,
                                              ID_SDICON);

    HWND        hwndDim = CreateDimScreenWindow();
    PDLGARRAY   pArray = NULL;

    G_ulConfirmHelpPanel = ID_XSH_RESTARTWPS_CONFIRM; // ID_XMH_RESTARTWPS;
                                    // changed V0.9.19 (2002-04-18) [umoeller]

    OKButton.pcszText = cmnGetString(DID_OK);
    CancelButton.pcszText = cmnGetString(DID_CANCEL);

    // format logoff differently from restart desktop V1.0.1 (2003-01-05) [umoeller]
    if (!dlghCreateArray(   ARRAYITEMCOUNT(dlgConfirmRestartDesktopHead)
                          + ARRAYITEMCOUNT(dlgConfirmRestartDesktopOnly)
                          + ARRAYITEMCOUNT(dlgConfirmLogoffOnly)
                          + ARRAYITEMCOUNT(dlgConfirmRestartDesktopTail),
                         &pArray))
    {
        dlghAppendToArray(pArray,
                          dlgConfirmRestartDesktopHead,
                          ARRAYITEMCOUNT(dlgConfirmRestartDesktopHead));
        if (psdParms->ulCloseMode == SHUT_LOGOFF)
            dlghAppendToArray(pArray,
                              dlgConfirmLogoffOnly,
                              ARRAYITEMCOUNT(dlgConfirmLogoffOnly));
        else
            dlghAppendToArray(pArray,
                              dlgConfirmRestartDesktopOnly,
                              ARRAYITEMCOUNT(dlgConfirmRestartDesktopOnly));
        dlghAppendToArray(pArray,
                          dlgConfirmRestartDesktopTail,
                          ARRAYITEMCOUNT(dlgConfirmRestartDesktopTail));

        if (!dlghCreateDlg(&hwndConfirm,
                           hwndDim,
                           FCF_FIXED_DLG,
                           fnwpConfirm,
                           cmnGetString((psdParms->ulCloseMode == SHUT_LOGOFF)
                                            ? ID_XSSI_XSD_CONFIRMLOGOFFTITLE
                                            : ID_SDDI_CONFIRMWPS_TITLE),
                           pArray->paDlgItems,
                           pArray->cDlgItemsNow,
                           NULL,
                           cmnQueryDefaultFont()))
        {
            HWND    hwndAni;

            WinSendMsg(hwndConfirm,
                       WM_SETICON,
                       (MPARAM)hptrShutdown,
                       NULL);

            if (psdParms->ulCloseMode == SHUT_LOGOFF)
            {
                // logoff:
                psdParms->optWPSProcessShutdown = TRUE;
                psdParms->optWPSCloseWindows = TRUE;
                psdParms->optWPSReuseStartupFolder = TRUE;
            }
            else
            {
                psdParms->optWPSProcessShutdown = psdParms->optWPSCloseWindows;
            }

            if (!_wpclsQueryFolder(_WPFolder,
                                   (PSZ)XFOLDER_SHUTDOWNID,
                                   FALSE))
            {
                psdParms->optWPSProcessShutdown = FALSE;
#ifndef __NOXWPSTARTUP__
                WinEnableControl(hwndConfirm, ID_SDDI_WPS_RUNSHUTDOWNFDR, FALSE);
#endif
            }

            winhSetDlgItemChecked(hwndConfirm, ID_SDDI_WPS_CLOSEWINDOWS, psdParms->optWPSCloseWindows);
#ifndef __NOXWPSTARTUP__
            winhSetDlgItemChecked(hwndConfirm, ID_SDDI_WPS_RUNSHUTDOWNFDR, psdParms->optWPSProcessShutdown);
            winhSetDlgItemChecked(hwndConfirm, ID_SDDI_WPS_STARTUPFOLDER, psdParms->optWPSCloseWindows);
#endif

#ifndef __EASYSHUTDOWN__
            winhSetDlgItemChecked(hwndConfirm, ID_SDDI_MESSAGEAGAIN, psdParms->optConfirm);
#endif

            xsdLoadAnimation(&G_sdAnim);
            hwndAni = WinWindowFromID(hwndConfirm, ID_SDDI_ICON);
            ctlPrepareAnimation(hwndAni,
                                XSD_ANIM_COUNT,
                                G_sdAnim.ahptr,
                                150,    // delay
                                TRUE);  // start now

            winhCenterWindow(hwndConfirm);      // still hidden
            WinShowWindow(hwndConfirm, TRUE);

            // *** go!
            ulReturn = WinProcessDlg(hwndConfirm);

            ctlStopAnimation(hwndAni);
            xsdFreeAnimation(&G_sdAnim);

            if (ulReturn == DID_OK)
            {
#ifndef __NOXSHUTDOWN__
                ULONG fl = cmnQuerySetting(sflXShutdown);
#endif
#ifndef __NOXWPSTARTUP__
                psdParms->optWPSProcessShutdown = winhIsDlgItemChecked(hwndConfirm,
                                                                       ID_SDDI_WPS_RUNSHUTDOWNFDR);

#endif
                psdParms->optWPSCloseWindows = winhIsDlgItemChecked(hwndConfirm,
                                                                    ID_SDDI_WPS_CLOSEWINDOWS);

                if (psdParms->ulCloseMode != SHUT_LOGOFF)
                {
                    // regular restart Desktop:
                    // save close windows/startup folder settings
#ifndef __NOXSHUTDOWN__
                    if (psdParms->optWPSCloseWindows)
                        fl |= XSD_WPS_CLOSEWINDOWS;
                    else
                        fl &= ~XSD_WPS_CLOSEWINDOWS;
#endif
#ifndef __NOXWPSTARTUP__
                    psdParms->optWPSReuseStartupFolder = winhIsDlgItemChecked(hwndConfirm,
                                                                              ID_SDDI_WPS_STARTUPFOLDER);
#endif

                    // V0.9.19 (2002-04-17) [umoeller]
                    if (winhIsDlgItemChecked(hwndConfirm,
                                             ID_SDDI_ARCHIVEONCE))
                    {
                        PARCHIVINGSETTINGS pArcSettings = arcQuerySettings();
                        pArcSettings->ulArcFlags |= ARCF_NEXT;
                        arcSaveSettings();
                    }
                }

#ifndef __EASYSHUTDOWN__
                if (!(winhIsDlgItemChecked(hwndConfirm,
                                           ID_SDDI_MESSAGEAGAIN)))
                    fl |= XSD_NOCONFIRM;

                cmnSetSetting(sflXShutdown, fl);
#endif
            }

            winhDestroyWindow(&hwndConfirm);
        }

        dlghFreeArray(&pArray);
    }

    winhDestroyWindow(&hwndDim);

    return ulReturn;
}

/* ******************************************************************
 *
 *   Auto-close details
 *
 ********************************************************************/

/*
 *@@ xsdLoadAutoCloseItems:
 *      this gets the list of VIO windows which
 *      are to be closed automatically from OS2.INI
 *      and appends AUTOCLOSELISTITEM's to the given
 *      list accordingly.
 *
 *      If hwndListbox != NULLHANDLE, the program
 *      titles are added to that listbox as well.
 *
 *      Returns the no. of items which were added.
 *
 *@@added V0.9.1 (99-12-10) [umoeller]
 */

USHORT xsdLoadAutoCloseItems(PLINKLIST pllItems,   // in: list of AUTOCLOSELISTITEM's to append to
                             HWND hwndListbox)     // in: listbox to add items to or NULLHANDLE if none
{
    USHORT      usItemCount = 0;
    ULONG       ulKeyLength;
    PSZ         p, pINI;

    // get existing items from INI
    if (PrfQueryProfileSize(HINI_USER,
                            (PSZ)INIAPP_XWORKPLACE,
                            (PSZ)INIKEY_AUTOCLOSE,
                            &ulKeyLength))
    {
        // printf("Size: %d\n", ulKeyLength);
        // items exist: evaluate
        pINI = malloc(ulKeyLength);
        if (pINI)
        {
            PrfQueryProfileData(HINI_USER,
                                (PSZ)INIAPP_XWORKPLACE,
                                (PSZ)INIKEY_AUTOCLOSE,
                                pINI,
                                &ulKeyLength);
            p = pINI;
            //printf("%s\n", p);
            while (strlen(p))
            {
                PAUTOCLOSELISTITEM pliNew = malloc(sizeof(AUTOCLOSELISTITEM));
                strlcpy(pliNew->szItemName,
                        p,
                        sizeof(pliNew->szItemName));
                lstAppendItem(pllItems, // pData->pllAutoClose,
                              pliNew);

                if (hwndListbox)
                    WinSendMsg(hwndListbox,
                               LM_INSERTITEM,
                               (MPARAM)LIT_END,
                               (MPARAM)p);

                p += (strlen(p) + 1);

                if (strlen(p))
                {
                    pliNew->usAction = *((PUSHORT)p);
                    p += sizeof(USHORT);
                }

                usItemCount++;
            }

            free(pINI);
        }
    }

    return usItemCount;
}

/*
 *@@ xsdWriteAutoCloseItems:
 *      reverse to xsdLoadAutoCloseItems, this writes the
 *      auto-close items back to OS2.INI.
 *
 *      This returns 0 only if no error occurred. If something
 *      != 0 is returned, that's the index of the list item
 *      which was found to be invalid.
 *
 *@@added V0.9.1 (99-12-10) [umoeller]
 */

USHORT xsdWriteAutoCloseItems(PLINKLIST pllItems)
{
    USHORT  usInvalid = 0;
    PSZ     pINI, p;
    // BOOL    fValid = TRUE;
    ULONG   ulItemCount = lstCountItems(pllItems);

    // store data in INI
    if (ulItemCount)
    {
        pINI = malloc(
                    sizeof(AUTOCLOSELISTITEM)
                  * ulItemCount);
        memset(pINI, 0,
                    sizeof(AUTOCLOSELISTITEM)
                  * ulItemCount);
        if (pINI)
        {
            PLISTNODE pNode = lstQueryFirstNode(pllItems);
            USHORT          usCurrent = 0;
            p = pINI;
            while (pNode)
            {
                PAUTOCLOSELISTITEM pli = pNode->pItemData;
                if (strlen(pli->szItemName) == 0)
                {
                    usInvalid = usCurrent;
                    break;
                }

                strcpy(p, pli->szItemName);
                p += (strlen(p)+1);
                *((PUSHORT)p) = pli->usAction;
                p += sizeof(USHORT);

                pNode = pNode->pNext;
                usCurrent++;
            }

            PrfWriteProfileData(HINI_USER,
                                (PSZ)INIAPP_XWORKPLACE,
                                (PSZ)INIKEY_AUTOCLOSE,
                                pINI,
                                (p - pINI + 2));

            free (pINI);
        }
    } // end if (pData->pliAutoClose)
    else
        // no items: delete INI key
        PrfWriteProfileData(HINI_USER,
                            (PSZ)INIAPP_XWORKPLACE,
                            (PSZ)INIKEY_AUTOCLOSE,
                            NULL, 0);

    return usInvalid;
}

/*
 *@@ fnwpAutoCloseDetails:
 *      dlg func for "Auto-Close Details".
 *      This gets called from the notebook callbacks
 *      for the "XDesktop" notebook page
 *      (fncbDesktop1ItemChanged, xfdesk.c).
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist.c functions
 *@@changed V0.9.19 (2002-05-23) [umoeller]: removed Ctrl+C option, which never worked
 *@@changed V0.9.19 (2002-05-23) [umoeller]: now using dialog formatter
 */

STATIC MRESULT EXPENTRY fnwpAutoCloseDetails(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = (MPARAM)NULL;

    PAUTOCLOSEWINDATA pData = (PAUTOCLOSEWINDATA)WinQueryWindowPtr(hwndDlg, QWL_USER);

    switch (msg)
    {
        case WM_INITDLG:
        {
            // create window data in QWL_USER
            pData = malloc(sizeof(AUTOCLOSEWINDATA));
            memset(pData, 0, sizeof(AUTOCLOSEWINDATA));
            pData->pllAutoClose = lstCreate(TRUE);  // auto-free items

            // set animation
            xsdLoadAnimation(&G_sdAnim);
            ctlPrepareAnimation(WinWindowFromID(hwndDlg,
                                                ID_SDDI_ICON),
                                XSD_ANIM_COUNT,
                                &(G_sdAnim.ahptr[0]),
                                150,    // delay
                                TRUE);  // start now

            pData->usItemCount = 0;

            pData->usItemCount = xsdLoadAutoCloseItems(pData->pllAutoClose,
                                                      WinWindowFromID(hwndDlg,
                                                                      ID_XSDI_XRB_LISTBOX));

            winhSetEntryFieldLimit(WinWindowFromID(hwndDlg, ID_XSDI_XRB_ITEMNAME),
                                   100-1);

            WinSetWindowULong(hwndDlg, QWL_USER, (ULONG)pData);

            WinPostMsg(hwndDlg, XM_UPDATE, MPNULL, MPNULL);
        }
        break;

        /*
         * WM_CONTROL:
         *
         */

        case WM_CONTROL:
            switch (SHORT1FROMMP(mp1))
            {
                /*
                 * ID_XSDI_XRB_LISTBOX:
                 *      listbox was clicked on:
                 *      update other controls with new data
                 */

                case ID_XSDI_XRB_LISTBOX:
                    if (SHORT2FROMMP(mp1) == LN_SELECT)
                        WinSendMsg(hwndDlg, XM_UPDATE, MPNULL, MPNULL);
                break;

                /*
                 * ID_XSDI_XRB_ITEMNAME:
                 *      user changed item title: update data
                 */

                case ID_XSDI_XRB_ITEMNAME:
                    if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
                    {
                        if (pData)
                        {
                            if (pData->pliSelected)
                            {
                                WinQueryDlgItemText(hwndDlg, ID_XSDI_XRB_ITEMNAME,
                                                    sizeof(pData->pliSelected->szItemName)-1,
                                                    pData->pliSelected->szItemName);
                                WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_LISTBOX,
                                                  LM_SETITEMTEXT,
                                                  (MPARAM)pData->sSelected,
                                                  (MPARAM)(pData->pliSelected->szItemName));
                            }
                        }
                    }
                break;

                // radio buttons
                case ID_XSDI_ACL_WMCLOSE:
                // case ID_XSDI_ACL_CTRL_C:     removed V0.9.19 (2002-05-23) [umoeller]
                case ID_XSDI_ACL_KILLSESSION:
                case ID_XSDI_ACL_SKIP:
                    if (SHORT2FROMMP(mp1) == BN_CLICKED)
                    {
                        if (pData)
                        {
                            if (pData->pliSelected)
                            {
                                pData->pliSelected->usAction =
                                    (SHORT1FROMMP(mp1) == ID_XSDI_ACL_WMCLOSE)
                                        ? ACL_WMCLOSE
                                    // : (SHORT1FROMMP(mp1) == ID_XSDI_ACL_CTRL_C)
                                    //     ? ACL_CTRL_C
                                    : (SHORT1FROMMP(mp1) == ID_XSDI_ACL_KILLSESSION)
                                        ? ACL_KILLSESSION
                                    : ACL_SKIP;
                            }
                        }
                    }
                break;

                default:
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
            }
        break;

        case XM_UPDATE:
        {
            // posted from various locations to wholly update
            // the dlg items
            if (pData)
            {
                pData->pliSelected = NULL;
                pData->sSelected = (USHORT)WinSendDlgItemMsg(hwndDlg,
                        ID_XSDI_XRB_LISTBOX,
                        LM_QUERYSELECTION,
                        (MPARAM)LIT_CURSOR,
                        MPNULL);
                //printf("  Selected: %d\n", pData->sSelected);
                if (pData->sSelected != LIT_NONE)
                {
                    pData->pliSelected = (PAUTOCLOSELISTITEM)lstItemFromIndex(
                                               pData->pllAutoClose,
                                               pData->sSelected);
                }

                if (pData->pliSelected)
                {
                    WinSetDlgItemText(hwndDlg, ID_XSDI_XRB_ITEMNAME,
                            pData->pliSelected->szItemName);

                    switch (pData->pliSelected->usAction)
                    {
                        case ACL_WMCLOSE:
                            winhSetDlgItemChecked(hwndDlg,
                                ID_XSDI_ACL_WMCLOSE, 1); break;
                        // case ACL_CTRL_C:     removed V0.9.19 (2002-05-23) [umoeller]
                        //     winhSetDlgItemChecked(hwndDlg,
                        //         ID_XSDI_ACL_CTRL_C, 1); break;
                        case ACL_KILLSESSION:
                            winhSetDlgItemChecked(hwndDlg,
                                ID_XSDI_ACL_KILLSESSION, 1); break;
                        case ACL_SKIP:
                            winhSetDlgItemChecked(hwndDlg,
                                ID_XSDI_ACL_SKIP, 1); break;
                    }
                }
                else
                    WinSetDlgItemText(hwndDlg,
                                      ID_XSDI_XRB_ITEMNAME,
                                      "");

                WinEnableControl(hwndDlg, ID_XSDI_XRB_ITEMNAME,
                            (pData->pliSelected != NULL));

                WinEnableControl(hwndDlg, ID_XSDI_ACL_WMCLOSE,
                            (pData->pliSelected != NULL));
                // WinEnableControl(hwndDlg, ID_XSDI_ACL_CTRL_C,
                //             (pData->pliSelected != NULL));
                WinEnableControl(hwndDlg, ID_XSDI_ACL_KILLSESSION,
                            (pData->pliSelected != NULL));
                WinEnableControl(hwndDlg, ID_XSDI_ACL_SKIP,
                            (pData->pliSelected != NULL));

                WinEnableControl(hwndDlg,
                                  ID_XSDI_XRB_DELETE,
                                  (   (pData->usItemCount > 0)
                                   && (pData->pliSelected)
                                  ));
            }
        }
        break;

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {

                /*
                 * ID_XSDI_XRB_NEW:
                 *      create new item
                 */

                case ID_XSDI_XRB_NEW:
                {
                    PAUTOCLOSELISTITEM pliNew = malloc(sizeof(AUTOCLOSELISTITEM));
                    strcpy(pliNew->szItemName, "???");
                    pliNew->usAction = ACL_SKIP;
                    lstAppendItem(pData->pllAutoClose,
                                  pliNew);

                    pData->usItemCount++;
                    WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_LISTBOX,
                                      LM_INSERTITEM,
                                      (MPARAM)LIT_END,
                                      (MPARAM)pliNew->szItemName);
                    WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_LISTBOX,
                                      LM_SELECTITEM, // will cause XM_UPDATE
                                      (MPARAM)(lstCountItems(
                                              pData->pllAutoClose)),
                                      (MPARAM)TRUE);
                    winhSetDlgItemFocus(hwndDlg, ID_XSDI_XRB_ITEMNAME);
                    WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_ITEMNAME,
                                      EM_SETSEL,
                                      MPFROM2SHORT(0, 1000), // select all
                                      MPNULL);
                }
                break;

                /*
                 * ID_XSDI_XRB_DELETE:
                 *      delete selected item
                 */

                case ID_XSDI_XRB_DELETE:
                {
                    //printf("WM_COMMAND ID_XSDI_XRB_DELETE BN_CLICKED\n");
                    if (pData)
                    {
                        if (pData->pliSelected)
                        {
                            lstRemoveItem(pData->pllAutoClose,
                                          pData->pliSelected);
                            WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_LISTBOX,
                                              LM_DELETEITEM,
                                              (MPARAM)pData->sSelected,
                                              MPNULL);
                        }
                        WinPostMsg(hwndDlg, XM_UPDATE, MPNULL, MPNULL);
                    }
                    winhSetDlgItemFocus(hwndDlg, ID_XSDI_XRB_LISTBOX);
                }
                break;

                /*
                 * DID_OK:
                 *      store data in INI and dismiss dlg
                 */

                case DID_OK:
                {
                    USHORT usInvalid;
                    if (usInvalid = xsdWriteAutoCloseItems(pData->pllAutoClose))
                    {
                        WinAlarm(HWND_DESKTOP, WA_ERROR);
                        WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_LISTBOX,
                                          LM_SELECTITEM,
                                          (MPARAM)usInvalid,
                                          (MPARAM)TRUE);
                    }
                    else
                        // dismiss dlg
                        mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
                }
                break;

                default:  // includes DID_CANCEL
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
                break;
            }
        break;

        case WM_HELP:
            cmnDisplayHelp(NULL,
                           ID_XFH_AUTOCLOSEDETAILS);
        break;

        /*
         * WM_DESTROY:
         *      clean up allocated memory
         */

        case WM_DESTROY:
        {
            ctlStopAnimation(WinWindowFromID(hwndDlg, ID_SDDI_ICON));
            xsdFreeAnimation(&G_sdAnim);
            lstFree(&pData->pllAutoClose);
            free(pData);
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        }
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        break;
    }
    return mrc;
}

#define ACL_WIDTH       200
#define ACL_LB_WIDTH    (ACL_WIDTH - COMMON_SPACING - STD_BUTTON_WIDTH)

static const CONTROLDEF
    ACLIcon = CONTROLDEF_ICON(NULLHANDLE, ID_SDDI_ICON),
    ACLIntro = LOADDEF_TEXT_WORDBREAK(ID_XSDI_ACL_INTRO, ACL_WIDTH - 30),
    ACLListbox = CONTROLDEF_LISTBOX(ID_XSDI_XRB_LISTBOX, ACL_LB_WIDTH, 60),
    ACLNew = LOADDEF_NOFOCUSBUTTON(ID_XSDI_XRB_NEW),
    ACLDelete = LOADDEF_NOFOCUSBUTTON(ID_XSDI_XRB_DELETE),
    ACLSessionTitle = LOADDEF_TEXT(ID_XSDI_ACL_SESSIONTITLE),
    ACLEntryField = CONTROLDEF_ENTRYFIELD("", ID_XSDI_XRB_ITEMNAME, ACL_WIDTH, SZL_AUTOSIZE),
    ACLDoWhatGroup = LOADDEF_GROUP(ID_XSDI_ACL_DOWHATGROUP, SZL_AUTOSIZE),
    ACLSkipRadio = LOADDEF_FIRST_AUTORADIO(ID_XSDI_ACL_SKIP),
    ACLWMCloseRadio = LOADDEF_NEXT_AUTORADIO(ID_XSDI_ACL_WMCLOSE),
    ACLKillRadio = LOADDEF_NEXT_AUTORADIO(ID_XSDI_ACL_KILLSESSION);

static const DLGHITEM G_dlgAutoCloseDetails[] =
    {
        START_TABLE,
            START_ROW(ROW_VALIGN_CENTER),
                CONTROL_DEF(&ACLIcon),
                CONTROL_DEF(&ACLIntro),
            START_ROW(ROW_VALIGN_CENTER),
                CONTROL_DEF(&ACLListbox),
                START_TABLE,
                    START_ROW(0),
                        CONTROL_DEF(&ACLNew),
                    START_ROW(0),
                        CONTROL_DEF(&ACLDelete),
                END_TABLE,
            START_ROW(0),
                CONTROL_DEF(&ACLSessionTitle),
            START_ROW(0),
                CONTROL_DEF(&ACLEntryField),
            START_ROW(0),
                START_GROUP_TABLE(&ACLDoWhatGroup),
                    START_ROW(0),
                        CONTROL_DEF(&ACLSkipRadio),
                    START_ROW(0),
                        CONTROL_DEF(&ACLWMCloseRadio),
                    START_ROW(0),
                        CONTROL_DEF(&ACLKillRadio),
                END_TABLE,
            START_ROW(0),
                CONTROL_DEF(&G_OKButton),
                CONTROL_DEF(&G_CancelButton),
                CONTROL_DEF(&G_HelpButton),
        END_TABLE
    };

/*
 *@@ xsdShowAutoCloseDetails:
 *      displays the "auto-close details" dialog.
 *
 *@@added V0.9.19 (2002-05-23) [umoeller]
 */

VOID xsdShowAutoCloseDetails(HWND hwndOwner)
{
    HWND hwndDlg;
    if (!dlghCreateDlg(&hwndDlg,
                       hwndOwner,
                       FCF_FIXED_DLG,
                       fnwpAutoCloseDetails,
                       cmnGetString(ID_XSD_AUTOCLOSE), // "Auto-Close Non-PM Sessions"
                       G_dlgAutoCloseDetails,
                       ARRAYITEMCOUNT(G_dlgAutoCloseDetails),
                       NULL,
                       cmnQueryDefaultFont()))
    {
        winhCenterWindow(hwndDlg);      // still hidden
        WinProcessDlg(hwndDlg);
        winhDestroyWindow(&hwndDlg);
    }
}

/* ******************************************************************
 *
 *   User reboot options
 *
 ********************************************************************/

static const CONTROLDEF
    RebootIntro = LOADDEF_TEXT_WORDBREAK(ID_XSDI_ACL_INTRO, ACL_WIDTH - 30),
    RebootUp = LOADDEF_NOFOCUSBUTTON(ID_XSDI_XRB_UP),
    RebootDown = LOADDEF_NOFOCUSBUTTON(ID_XSDI_XRB_DOWN),
    RebootActionTitleTxt = LOADDEF_TEXT(ID_XSDI_XRB_ITEMNAME_TXT),
    RebootActionTitleEF = CONTROLDEF_ENTRYFIELD(
                                "",
                                ID_XSDI_XRB_ITEMNAME,
                                SZL_REMAINDER,       // remaining width next to partitions button
                                SZL_AUTOSIZE),
    RebootPartitionsButton = LOADDEF_NOFOCUSBUTTON(ID_XSDI_XRB_PARTITIONS),
    RebootActionCmdTxt = LOADDEF_TEXT(ID_XSDI_XRB_COMMAND_TXT),
    RebootActionCmdEF = CONTROLDEF_ENTRYFIELD(
                                "",
                                ID_XSDI_XRB_COMMAND,
                                -100,       // full width
                                SZL_AUTOSIZE);

static const DLGHITEM G_dlgRebootActions[] =
    {
        START_TABLE,
            START_ROW(ROW_VALIGN_CENTER),
                CONTROL_DEF(&ACLIcon),
                CONTROL_DEF(&RebootIntro),
            START_ROW(ROW_VALIGN_CENTER),
                CONTROL_DEF(&ACLListbox),
                START_TABLE,
                    START_ROW(0),
                        CONTROL_DEF(&ACLNew),
                    START_ROW(0),
                        CONTROL_DEF(&ACLDelete),
                    START_ROW(0),
                        CONTROL_DEF(&RebootUp),
                    START_ROW(0),
                        CONTROL_DEF(&RebootDown),
                END_TABLE,
            START_ROW(0),
                CONTROL_DEF(&RebootActionTitleTxt),
            START_ROW(0),
                START_TABLE_EXT(TABLE_INHERIT_SIZE),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&RebootActionTitleEF),
                        CONTROL_DEF(&RebootPartitionsButton),
                END_TABLE,
            START_ROW(0),
                CONTROL_DEF(&RebootActionCmdTxt),
            START_ROW(0),
                CONTROL_DEF(&RebootActionCmdEF),
            START_ROW(0),
                CONTROL_DEF(&G_OKButton),
                CONTROL_DEF(&G_CancelButton),
                CONTROL_DEF(&G_HelpButton),
        END_TABLE
    };

/*
 *@@ fnwpUserRebootOptions:
 *      dlg proc for the "Extended Reboot" options.
 *      This gets called from the notebook callbacks
 *      for the "XDesktop" notebook page
 *      (fncbDesktop1ItemChanged, xfdesk.c).
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist.c functions
 *@@changed V0.9.0 [umoeller]: renamed from fnwpRebootExt
 *@@changed V0.9.0 [umoeller]: added "Partitions" button
 *@@changed V1.0.2 (2003-12-03) [umoeller]: now using the dlg formatter
 *@@changed V1.0.9 (2010-09-30) [pr]: fix item count on Delete
 */

MRESULT EXPENTRY fnwpUserRebootOptions(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = (MPARAM)NULL;

    switch (msg)
    {
        /*
         * WM_INITDLG:
         *
         */

        case WM_INITDLG:
        {
            ULONG       ulKeyLength;
            PSZ         p, pINI;

            // create window data in QWL_USER
            PREBOOTWINDATA pData = malloc(sizeof(REBOOTWINDATA));
            memset(pData, 0, sizeof(REBOOTWINDATA));
            pData->pllReboot = lstCreate(TRUE);

            // set animation
            xsdLoadAnimation(&G_sdAnim);
            ctlPrepareAnimation(WinWindowFromID(hwndDlg, ID_SDDI_ICON),
                                XSD_ANIM_COUNT,
                                &(G_sdAnim.ahptr[0]),
                                150,    // delay
                                TRUE);  // start now

            pData->usItemCount = 0;

            // get existing items from INI
            if (PrfQueryProfileSize(HINI_USER,
                                    (PSZ)INIAPP_XWORKPLACE,
                                    (PSZ)INIKEY_BOOTMGR,
                                    &ulKeyLength))
            {
                // _Pmpf(( "Size: %d", ulKeyLength ));
                // items exist: evaluate
                if (pINI = malloc(ulKeyLength))
                {
                    PrfQueryProfileData(HINI_USER,
                                        (PSZ)INIAPP_XWORKPLACE,
                                        (PSZ)INIKEY_BOOTMGR,
                                        pINI,
                                        &ulKeyLength);
                    p = pINI;
                    // _Pmpf(( "%s", p ));
                    while (strlen(p))
                    {
                        PREBOOTLISTITEM pliNew = malloc(sizeof(REBOOTLISTITEM));
                        strcpy(pliNew->szItemName, p);
                        lstAppendItem(pData->pllReboot,
                                    pliNew);

                        WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_LISTBOX,
                                        LM_INSERTITEM,
                                        (MPARAM)LIT_END,
                                        (MPARAM)p);
                        p += (strlen(p)+1);

                        if (strlen(p))
                        {
                            strcpy(pliNew->szCommand, p);
                            p += (strlen(p)+1);
                        }

                        pData->usItemCount++;
                    }

                    free(pINI);
                }
            }

            WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_ITEMNAME,
                              EM_SETTEXTLIMIT,
                              (MPARAM)(100-1), MPNULL);
            WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_COMMAND,
                              EM_SETTEXTLIMIT,
                              (MPARAM)(CCHMAXPATH-1), MPNULL);

            WinSetWindowULong(hwndDlg, QWL_USER, (ULONG)pData);

            // create "menu button" for "Partitions..."
            ctlMakeMenuButton(WinWindowFromID(hwndDlg, ID_XSDI_XRB_PARTITIONS),
                              // set menu resource module and ID to
                              // 0; this will cause WM_COMMAND for
                              // querying the menu handle to be
                              // displayed
                              0, 0);

            WinPostMsg(hwndDlg, XM_UPDATE, MPNULL, MPNULL);
        }
        break;

        /*
         * WM_CONTROL:
         *
         */

        case WM_CONTROL:
            switch (SHORT1FROMMP(mp1))
            {
                /*
                 * ID_XSDI_XRB_LISTBOX:
                 *      new reboot item selected.
                 */

                case ID_XSDI_XRB_LISTBOX:
                    if (SHORT2FROMMP(mp1) == LN_SELECT)
                        WinSendMsg(hwndDlg, XM_UPDATE, MPNULL, MPNULL);
                break;

                /*
                 * ID_XSDI_XRB_ITEMNAME:
                 *      reboot item name changed.
                 */

                case ID_XSDI_XRB_ITEMNAME:
                    if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
                    {
                        PREBOOTWINDATA pData =
                                (PREBOOTWINDATA)WinQueryWindowPtr(hwndDlg, QWL_USER);
                        // _Pmpf(( "WM_CONTROL ID_XSDI_XRB_ITEMNAME EN_KILLFOCUS" ));
                        if (pData)
                        {
                            if (pData->pliSelected)
                            {
                                WinQueryDlgItemText(hwndDlg, ID_XSDI_XRB_ITEMNAME,
                                                    sizeof(pData->pliSelected->szItemName)-1,
                                                    pData->pliSelected->szItemName);
                                WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_LISTBOX,
                                                  LM_SETITEMTEXT,
                                                  (MPARAM)pData->sSelected,
                                                  (MPARAM)(pData->pliSelected->szItemName));
                            }
                        }
                    }
                break;

                /*
                 * ID_XSDI_XRB_COMMAND:
                 *      reboot item command changed.
                 */

                case ID_XSDI_XRB_COMMAND:
                    if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
                    {
                        PREBOOTWINDATA pData =
                                (PREBOOTWINDATA)WinQueryWindowPtr(hwndDlg, QWL_USER);
                        // _Pmpf(( "WM_CONTROL ID_XSDI_XRB_COMMAND EN_KILLFOCUS" ));
                        if (pData)
                            if (pData->pliSelected)
                                WinQueryDlgItemText(hwndDlg, ID_XSDI_XRB_COMMAND,
                                                    sizeof(pData->pliSelected->szCommand)-1,
                                                    pData->pliSelected->szCommand);
                    }
                break;

                default:
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
            }
        break;

        /*
         * XM_UPDATE:
         *      updates the controls according to the
         *      currently selected list box item.
         */

        case XM_UPDATE:
        {
            PREBOOTWINDATA pData;
            if (pData = (PREBOOTWINDATA)WinQueryWindowPtr(hwndDlg, QWL_USER))
            {
                pData->pliSelected = NULL;
                pData->sSelected = (USHORT)WinSendDlgItemMsg(hwndDlg,
                                                             ID_XSDI_XRB_LISTBOX,
                                                             LM_QUERYSELECTION,
                                                             (MPARAM)LIT_CURSOR,
                                                             MPNULL);
                if (pData->sSelected != LIT_NONE)
                    pData->pliSelected = (PREBOOTLISTITEM)lstItemFromIndex(
                            pData->pllReboot,
                            pData->sSelected);

                if (pData->pliSelected)
                {
                    WinSetDlgItemText(hwndDlg, ID_XSDI_XRB_ITEMNAME,
                            pData->pliSelected->szItemName);
                    WinSetDlgItemText(hwndDlg, ID_XSDI_XRB_COMMAND,
                            pData->pliSelected->szCommand);
                }
                else
                {
                    WinSetDlgItemText(hwndDlg, ID_XSDI_XRB_ITEMNAME,
                            "");
                    WinSetDlgItemText(hwndDlg, ID_XSDI_XRB_COMMAND,
                            "");
                }
                WinEnableControl(hwndDlg, ID_XSDI_XRB_ITEMNAME,
                            (pData->pliSelected != NULL));
                WinEnableControl(hwndDlg, ID_XSDI_XRB_COMMAND,
                            (pData->pliSelected != NULL));
                WinEnableControl(hwndDlg, ID_XSDI_XRB_PARTITIONS,
                            (pData->pliSelected != NULL));
                WinEnableControl(hwndDlg, ID_XSDI_XRB_UP,
                            (   (pData->pliSelected != NULL)
                             && (pData->usItemCount > 1)
                             && (pData->sSelected > 0)
                            ));
                WinEnableControl(hwndDlg, ID_XSDI_XRB_DOWN,
                            (   (pData->pliSelected != NULL)
                             && (pData->usItemCount > 1)
                             && (pData->sSelected < (pData->usItemCount-1))
                            ));

                WinEnableControl(hwndDlg, ID_XSDI_XRB_DELETE,
                            (   (pData->usItemCount > 0)
                             && (pData->pliSelected)
                            )
                        );
            }
        }
        break;

        /*
         * WM_COMMAND:
         *
         */

        case WM_COMMAND:
        {
            PREBOOTWINDATA pData =
                    (PREBOOTWINDATA)WinQueryWindowPtr(hwndDlg, QWL_USER);
            USHORT usItemID = SHORT1FROMMP(mp1);
            switch (usItemID)
            {
                /*
                 * ID_XSDI_XRB_NEW:
                 *      create new item
                 */

                case ID_XSDI_XRB_NEW:
                {
                    PREBOOTLISTITEM pliNew = malloc(sizeof(REBOOTLISTITEM));
                    // _Pmpf(( "WM_COMMAND ID_XSDI_XRB_NEW BN_CLICKED" ));
                    strcpy(pliNew->szItemName, "???");
                    strcpy(pliNew->szCommand, "???");
                    lstAppendItem(pData->pllReboot,
                                  pliNew);

                    pData->usItemCount++;
                    WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_LISTBOX,
                                    LM_INSERTITEM,
                                    (MPARAM)LIT_END,
                                    (MPARAM)pliNew->szItemName);
                    WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_LISTBOX,
                                    LM_SELECTITEM,
                                    (MPARAM)(lstCountItems(
                                            pData->pllReboot)
                                         - 1),
                                    (MPARAM)TRUE);
                    winhSetDlgItemFocus(hwndDlg, ID_XSDI_XRB_ITEMNAME);
                    WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_ITEMNAME,
                            EM_SETSEL,
                            MPFROM2SHORT(0, 1000), // select all
                            MPNULL);
                }
                break;

                /*
                 * ID_XSDI_XRB_DELETE:
                 *      delete delected item
                 */

                case ID_XSDI_XRB_DELETE:
                    if (pData)
                    {
                        if (pData->pliSelected)
                        {
                            lstRemoveItem(pData->pllReboot,
                                    pData->pliSelected);
                            WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_LISTBOX,
                                    LM_DELETEITEM,
                                    (MPARAM)pData->sSelected,
                                    MPNULL);
                            pData->usItemCount--;  // V1.0.9
                        }
                        WinPostMsg(hwndDlg, XM_UPDATE, MPNULL, MPNULL);
                    }
                    winhSetDlgItemFocus(hwndDlg, ID_XSDI_XRB_LISTBOX);
                break;

                /*
                 * ID_XSDI_XRB_UP:
                 *      move selected item up
                 */

                case ID_XSDI_XRB_UP:
                    if (pData)
                    {
                        // _Pmpf(( "WM_COMMAND ID_XSDI_XRB_UP BN_CLICKED" ));
                        if (pData->pliSelected)
                        {
                            PREBOOTLISTITEM pliNew = malloc(sizeof(REBOOTLISTITEM));
                            *pliNew = *(pData->pliSelected);
                            // remove selected
                            lstRemoveItem(pData->pllReboot,
                                    pData->pliSelected);
                            WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_LISTBOX,
                                    LM_DELETEITEM,
                                    (MPARAM)pData->sSelected,
                                    MPNULL);
                            // insert item again
                            lstInsertItemBefore(pData->pllReboot,
                                                pliNew,
                                                (pData->sSelected-1));
                            WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_LISTBOX,
                                            LM_INSERTITEM,
                                            (MPARAM)(pData->sSelected-1),
                                            (MPARAM)pliNew->szItemName);
                            WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_LISTBOX,
                                            LM_SELECTITEM,
                                            (MPARAM)(pData->sSelected-1),
                                            (MPARAM)TRUE); // select flag
                        }
                        WinPostMsg(hwndDlg, XM_UPDATE, MPNULL, MPNULL);
                    }
                    winhSetDlgItemFocus(hwndDlg, ID_XSDI_XRB_LISTBOX);
                break;

                /*
                 * ID_XSDI_XRB_DOWN:
                 *      move selected item down
                 */

                case ID_XSDI_XRB_DOWN:
                    if (pData)
                    {
                        // _Pmpf(( "WM_COMMAND ID_XSDI_XRB_DOWN BN_CLICKED" ));
                        if (pData->pliSelected)
                        {
                            PREBOOTLISTITEM pliNew = malloc(sizeof(REBOOTLISTITEM));
                            *pliNew = *(pData->pliSelected);
                            // remove selected
                            // _Pmpf(( "  Removing index %d", pData->sSelected ));
                            lstRemoveItem(pData->pllReboot,
                                          pData->pliSelected);
                            WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_LISTBOX,
                                    LM_DELETEITEM,
                                    (MPARAM)pData->sSelected,
                                    MPNULL);
                            // insert item again
                            lstInsertItemBefore(pData->pllReboot,
                                                pliNew,
                                                (pData->sSelected+1));
                            WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_LISTBOX,
                                            LM_INSERTITEM,
                                            (MPARAM)(pData->sSelected+1),
                                            (MPARAM)pliNew->szItemName);
                            WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_LISTBOX,
                                            LM_SELECTITEM,
                                            (MPARAM)(pData->sSelected+1),
                                            (MPARAM)TRUE); // select flag
                        }
                        WinPostMsg(hwndDlg, XM_UPDATE, MPNULL, MPNULL);
                    }
                    winhSetDlgItemFocus(hwndDlg, ID_XSDI_XRB_LISTBOX);
                break;

                /*
                 * ID_XSDI_XRB_PARTITIONS:
                 *      "Partitions" button.
                 *      Even though this is part of the WM_COMMAND
                 *      block, this is not really a command msg
                 *      like the other messages; instead, this
                 *      is sent (!) to us and expects a HWND for
                 *      the menu to be displayed on the button.
                 *
                 *      We create a menu containing bootable
                 *      partitions.
                 */

                case ID_XSDI_XRB_PARTITIONS:
                {
                    HPOINTER       hptrOld = winhSetWaitPointer();
                    HWND           hMenu = NULLHANDLE;

                    if (!pData->fPartitionsLoaded)  // V0.9.9 (2001-04-07) [umoeller]
                    {
                        // first time:
                        USHORT         usContext = 0;
                        APIRET arc = doshGetPartitionsList(&pData->pPartitionsList,
                                                           &usContext);

                        pData->fPartitionsLoaded = TRUE;
                    }

                    PMPF_SHUTDOWN(("pData->pPartitionsList is 0x%lX",
                                pData->pPartitionsList));

                    if (pData->pPartitionsList)
                    {
                        PPARTITIONINFO ppi = pData->pPartitionsList->pPartitionInfo;
                        SHORT          sItemId = ID_XSDI_PARTITIONSFIRST;
                        hMenu = WinCreateMenu(HWND_DESKTOP,
                                              NULL); // no menu template
                        while (ppi)
                        {
                            if (ppi->fBootable)
                            {
                                CHAR szMenuItem[100];
                                sprintf(szMenuItem,
                                        "%s (Drive %d, %c:)",
                                        ppi->szBootName,
                                        ppi->bDisk,
                                        ppi->cLetter);
                                winhInsertMenuItem(hMenu,
                                                   MIT_END,
                                                   sItemId++,
                                                   szMenuItem,
                                                   MIS_TEXT,
                                                   0);
                            }
                            ppi = ppi->pNext;
                        }
                    }

                    WinSetPointer(HWND_DESKTOP, hptrOld);

                    mrc = (MRESULT)hMenu;
                }
                break;

                /*
                 * DID_OK:
                 *      store data in INI and dismiss dlg
                 */

                case DID_OK:
                {
                    PSZ     pINI, p;
                    BOOL    fValid = TRUE;
                    ULONG   ulItemCount = lstCountItems(pData->pllReboot);

                    // _Pmpf(( "WM_COMMAND DID_OK BN_CLICKED" ));
                    // store data in INI
                    if (ulItemCount)
                    {
                        pINI = malloc(
                                    sizeof(REBOOTLISTITEM)
                                  * ulItemCount);
                        memset(pINI, 0,
                                    sizeof(REBOOTLISTITEM)
                                  * ulItemCount);

                        if (pINI)
                        {
                            PLISTNODE       pNode = lstQueryFirstNode(pData->pllReboot);
                            USHORT          usCurrent = 0;
                            p = pINI;

                            while (pNode)
                            {
                                PREBOOTLISTITEM pli = pNode->pItemData;

                                if (    (strlen(pli->szItemName) == 0)
                                     || (strlen(pli->szCommand) == 0)
                                   )
                                {
                                    WinAlarm(HWND_DESKTOP, WA_ERROR);
                                    WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_LISTBOX,
                                                    LM_SELECTITEM,
                                                    (MPARAM)usCurrent,
                                                    (MPARAM)TRUE);
                                    fValid = FALSE;
                                    break;
                                }
                                strcpy(p, pli->szItemName);
                                p += (strlen(p)+1);
                                strcpy(p, pli->szCommand);
                                p += (strlen(p)+1);

                                pNode = pNode->pNext;
                                usCurrent++;
                            }

                            PrfWriteProfileData(HINI_USER,
                                        (PSZ)INIAPP_XWORKPLACE,
                                        (PSZ)INIKEY_BOOTMGR,
                                        pINI,
                                        (p - pINI + 2));

                            free (pINI);
                        }
                    } // end if (pData->pliReboot)
                    else
                        PrfWriteProfileData(HINI_USER,
                                    (PSZ)INIAPP_XWORKPLACE,
                                    (PSZ)INIKEY_BOOTMGR,
                                    NULL, 0);

                    // dismiss dlg
                    if (fValid)
                        mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
                }
                break;

                default: // includes DID_CANCEL
                    if (    (pData->pPartitionsList)
                         && (pData->pPartitionsList->cPartitions)
                       )
                    {
                        // partitions valid:
                        ULONG cPartitions = pData->pPartitionsList->cPartitions;
                        if (    (usItemID >= ID_XSDI_PARTITIONSFIRST)
                             && (usItemID < ID_XSDI_PARTITIONSFIRST + cPartitions)
                             && (pData->pliSelected)
                           )
                        {
                            // partition item from "Partitions" menu button:
                            // search partitions list then
                            PPARTITIONINFO ppi = pData->pPartitionsList->pPartitionInfo;
                            SHORT sItemIDCompare = ID_XSDI_PARTITIONSFIRST;
                            while (ppi)
                            {
                                if (ppi->fBootable)
                                {
                                    // bootable item:
                                    // then we have inserted the thing into
                                    // the menu
                                    if (sItemIDCompare == usItemID)
                                    {
                                        // found our one:
                                        // insert into entry field
                                        CHAR szItem[20];
                                        // CHAR szCommand[100];
                                        ULONG ul = 0;

                                        // strip trailing spaces
                                        strcpy(szItem, ppi->szBootName);
                                        for (ul = strlen(szItem) - 1;
                                             ul > 0;
                                             ul--)
                                            if (szItem[ul] == ' ')
                                                szItem[ul] = 0;
                                            else
                                                break;

                                        // now set reboot item's data
                                        // according to the partition item
                                        strcpy(pData->pliSelected->szItemName,
                                               szItem);
                                        // compose new command
                                        sprintf(pData->pliSelected->szCommand,
                                                "setboot /iba:\"%s\"",
                                                szItem);

                                        // update list box item
                                        WinSendDlgItemMsg(hwndDlg, ID_XSDI_XRB_LISTBOX,
                                                          LM_SETITEMTEXT,
                                                          (MPARAM)pData->sSelected,
                                                          (MPARAM)(pData->pliSelected->szItemName));
                                        // update rest of dialog
                                        WinSendMsg(hwndDlg, XM_UPDATE, MPNULL, MPNULL);

                                        break; // while (ppi)
                                    }
                                    else
                                        // next item
                                        sItemIDCompare++;
                                }
                                ppi = ppi->pNext;
                            }

                            break;
                        }
                    }
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
                break;
            }
        }
        break;

        case WM_HELP:
            cmnDisplayHelp(NULL,
                           ID_XFH_REBOOTEXT);
        break;

        /*
         * WM_DESTROY:
         *      clean up allocated memory
         */

        case WM_DESTROY:
        {
            PREBOOTWINDATA pData = (PREBOOTWINDATA)WinQueryWindowPtr(hwndDlg, QWL_USER);
            ctlStopAnimation(WinWindowFromID(hwndDlg, ID_SDDI_ICON));
            xsdFreeAnimation(&G_sdAnim);
            lstFree(&pData->pllReboot);
            if (pData->pPartitionsList)
                doshFreePartitionsList(pData->pPartitionsList);
            free(pData);
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        }
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        break;
    }
    return mrc;
}

/*
 *@@ xsdShowRebootActions:
 *      displays the "Reboot actions" dialog, which is now
 *      produced with the dialog formatter.
 *
 *@@added V1.0.2 (2003-12-03) [umoeller]
 */

VOID xsdShowRebootActions(HWND hwndOwner)
{
    HWND hwndDlg;
    if (!dlghCreateDlg(&hwndDlg,
                       hwndOwner,
                       FCF_FIXED_DLG,
                       fnwpUserRebootOptions,
                       cmnGetString(ID_XSD_REBOOTEXT_TITLE),
                       G_dlgRebootActions,
                       ARRAYITEMCOUNT(G_dlgRebootActions),
                       NULL,
                       cmnQueryDefaultFont()))
    {
        winhCenterWindow(hwndDlg);      // still hidden
        WinProcessDlg(hwndDlg);
        winhDestroyWindow(&hwndDlg);
    }
}
