
/*
 *@@sourcefile w_monitors.c:
 *      XCenter monitor widgets (clock, memory, swapper).
 *
 *      This is an example of an XCenter widget plugin.
 *      This widget resides in MONITORS.DLL, which (as
 *      with all widget plugins) must be put into the
 *      plugins/xcenter directory of the XWorkplace
 *      installation directory.
 *
 *      Any XCenter widget plugin DLL must export the
 *      following procedures by ordinal:
 *
 *      -- Ordinal 1 (MwgtInitModule): this must
 *         return the widgets which this DLL provides.
 *
 *      -- Ordinal 2 (MwgtUnInitModule): this must
 *         clean up global DLL data.
 *
 *      The makefile in src\widgets compiles widgets
 *      with the VAC subsystem library. As a result,
 *      multiple threads are not supported.
 *
 *      This is all new with V0.9.7.
 *
 *@@added V0.9.7 (2000-12-02) [umoeller]
 *@@header "shared\center.h"
 */

/*
 *      Copyright (C) 2000-2010 Ulrich M”ller.
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
#define INCL_DOSMODULEMGR
#define INCL_DOSEXCEPTIONS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDATETIME
#define INCL_DOSMISC
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WININPUT
#define INCL_WINPOINTERS
#define INCL_WINPROGRAMLIST
#define INCL_WINSWITCHLIST
#define INCL_WINSYS
#define INCL_WINTIMER
#define INCL_WINMENUS
#define INCL_WINWORKPLACE

#define INCL_GPIPRIMITIVES
#define INCL_GPILOGCOLORTABLE
#define INCL_GPILCIDS
#define INCL_GPIREGIONS
#include <os2.h>

// C library headers
#include <stdio.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#define DONT_REPLACE_MALLOC             // in case mem debug is enabled
#define DONT_REPLACE_FOR_DBCS           // do not replace strchr with DBCS version
#include "setup.h"                      // code generation and debugging options

// disable wrappers, because we're not linking statically
#ifdef DOSH_STANDARDWRAPPERS
    #undef DOSH_STANDARDWRAPPERS
#endif
#ifdef WINH_STANDARDWRAPPERS
    #undef WINH_STANDARDWRAPPERS
#endif

// headers in /helpers
#include "helpers\apmh.h"               // Advanced Power Management helpers
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\nls.h"                // National Language Support helpers
#include "helpers\prfh.h"               // INI file helper routines;
                                        // this include is required for some
                                        // of the structures in shared\center.h
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\timer.h"              // replacement PM timers
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\center.h"              // public XCenter interfaces
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\helppanels.h"          // all XWorkplace help panel IDs

#include "config\drivdlgs.h"            // driver configuration dialogs

#include "filesys\disk.h"               // XFldDisk implementation

#include "hook\xwphook.h"               // hook and daemon definitions

// #include "startshut\apm.h"              // APM power-off for XShutdown

#pragma hdrstop                     // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Private definitions
 *
 ********************************************************************/

#define MWGT_DATE               1
// #define MWGT_SWAPPER            2        disabled V1.0.0 (2002-08-21) [umoeller]
#define MWGT_MEMORY             3
#define MWGT_TIME               4
#define MWGT_POWER              5       // V0.9.12 (2001-05-26) [umoeller]
#define MWGT_DISKFREE           6       // V0.9.14 (2001-08-01) [umoeller]

APIRET16 APIENTRY16 Dos16MemAvail(PULONG pulAvailMem);

/* ******************************************************************
 *
 *   XCenter widget class definition
 *
 ********************************************************************/

/*
 *      This contains the name of the PM window class and
 *      the XCENTERWIDGETCLASS definition(s) for the widget
 *      class(es) in this DLL.
 */

#define WNDCLASS_WIDGET_MONITORS    "XWPCenterMonitorWidget"

static const XCENTERWIDGETCLASS G_WidgetClasses[] =
    {
        {
            WNDCLASS_WIDGET_MONITORS,
            MWGT_DATE,
            "Date",
            (PCSZ)(XCENTER_STRING_RESOURCE | ID_CRSI_WIDGET_DATE),
                                        // widget class name displayed to user
                                        // (NLS DLL) V0.9.19 (2002-05-07) [umoeller]
            WGTF_UNIQUEPERXCENTER | WGTF_TOOLTIP | WGTF_TRAYABLE,
            NULL        // no settings dlg
        },
        {
            WNDCLASS_WIDGET_MONITORS,
            MWGT_TIME,
            "Time",
            (PCSZ)(XCENTER_STRING_RESOURCE | ID_CRSI_WIDGET_TIME),
                                        // widget class name displayed to user
                                        // (NLS DLL) V0.9.19 (2002-05-07) [umoeller]
            WGTF_UNIQUEPERXCENTER | WGTF_TOOLTIP | WGTF_TRAYABLE,
            NULL        // no settings dlg
        },
        {
            WNDCLASS_WIDGET_MONITORS,
            MWGT_MEMORY,
            "PhysMemory",
            (PCSZ)(XCENTER_STRING_RESOURCE | ID_CRSI_WIDGET_FREEMEM),
                                        // widget class name displayed to user
                                        // (NLS DLL) V0.9.19 (2002-05-07) [umoeller]
            WGTF_UNIQUEPERXCENTER | WGTF_TOOLTIP | WGTF_TRAYABLE,
            NULL        // no settings dlg
        },
        {
            WNDCLASS_WIDGET_MONITORS,
            MWGT_POWER,
            "Power",
            (PCSZ)(XCENTER_STRING_RESOURCE | ID_CRSI_WIDGET_POWER),
                                        // widget class name displayed to user
                                        // (NLS DLL) V0.9.19 (2002-05-07) [umoeller]
            WGTF_UNIQUEGLOBAL | WGTF_TOOLTIP | WGTF_TRAYABLE,
            NULL        // no settings dlg
        },
        {
            WNDCLASS_WIDGET_MONITORS,
            MWGT_DISKFREE,
            "DiskFreeCondensed",
            (PCSZ)(XCENTER_STRING_RESOURCE | ID_CRSI_WIDGET_DISKFREE_COND),
                                        // widget class name displayed to user
                                        // (NLS DLL) V0.9.19 (2002-05-07) [umoeller]
            WGTF_TOOLTIP | WGTF_TRAYABLE | WGTF_SIZEABLE,
            NULL        // no settings dlg
        }
    };

/* ******************************************************************
 *
 *   Function imports from XFLDR.DLL
 *
 ********************************************************************/

/*
 *      To reduce the size of the widget DLL, it is
 *      compiled with the VAC subsystem libraries.
 *      In addition, instead of linking frequently
 *      used helpers against the DLL again, we import
 *      them from XFLDR.DLL, whose module handle is
 *      given to us in the INITMODULE export.
 *
 *      Note that importing functions from XFLDR.DLL
 *      is _not_ a requirement. We only do this to
 *      avoid duplicate code.
 *
 *      For each funtion that you need, add a global
 *      function pointer variable and an entry to
 *      the G_aImports array. These better match.
 */

// resolved function pointers from XFLDR.DLL
PAPMHOPEN papmhOpen = NULL;
PAPMHREADSTATUS papmhReadStatus = NULL;
PAPMHCLOSE papmhClose = NULL;

PCMNGETSTRING pcmnGetString = NULL;
PCMNQUERYDEFAULTFONT pcmnQueryDefaultFont = NULL;
PCMNQUERYHELPLIBRARY pcmnQueryHelpLibrary = NULL;
PCMNQUERYMAINRESMODULEHANDLE pcmnQueryMainResModuleHandle = NULL;

PCTRDEFWIDGETPROC pctrDefWidgetProc = NULL;
PCTRFREESETUPVALUE pctrFreeSetupValue = NULL;
PCTRPARSECOLORSTRING pctrParseColorString = NULL;
PCTRSCANSETUPSTRING pctrScanSetupString = NULL;

PDRV_SPRINTF pdrv_sprintf = NULL;

PDSKQUERYINFO pdskQueryInfo = NULL;

PGPIHBOX pgpihBox = NULL;
PGPIHCREATEFONT pgpihCreateFont = NULL;
PGPIHDRAW3DFRAME pgpihDraw3DFrame = NULL;
PGPIHFINDPRESFONT pgpihFindPresFont = NULL;
PGPIHMATCHFONT pgpihMatchFont = NULL;
PGPIHSETPOINTSIZE pgpihSetPointSize = NULL;
PGPIHSPLITPRESFONT pgpihSplitPresFont = NULL;
PGPIHSWITCHTORGB pgpihSwitchToRGB = NULL;
PGPIHCREATEXBITMAP pgpihCreateXBitmap = NULL;
PGPIHDESTROYXBITMAP pgpihDestroyXBitmap = NULL;

PKRNQUERYDAEMONOBJECT pkrnQueryDaemonObject = NULL;
PKRNPOSTTHREAD1OBJECTMSG pkrnPostThread1ObjectMsg = NULL;

PNLSDATETIME2 pnlsDateTime2 = NULL;
PNLSTHOUSANDSULONG pnlsThousandsULong = NULL;

PTMRSTARTXTIMER ptmrStartXTimer = NULL;
PTMRSTOPXTIMER ptmrStopXTimer = NULL;

PWINHFREE pwinhFree = NULL;
PWINHINSERTMENUITEM pwinhInsertMenuItem = NULL;
PWINHINSERTSUBMENU pwinhInsertSubmenu = NULL;
PWINHQUERYPRESCOLOR pwinhQueryPresColor = NULL;
PWINHQUERYWINDOWFONT pwinhQueryWindowFont = NULL;
PWINHSETWINDOWFONT pwinhSetWindowFont = NULL;

PXSTRCAT pxstrcat = NULL;
PXSTRCATC pxstrcatc = NULL;
PXSTRCLEAR pxstrClear = NULL;
PXSTRINIT pxstrInit = NULL;

static const RESOLVEFUNCTION G_aImports[] =
    {
        "apmhOpen", (PFN*)&papmhOpen,
        "apmhReadStatus", (PFN*)&papmhReadStatus,
        "apmhClose", (PFN*)&papmhClose,
        "cmnGetString", (PFN*)&pcmnGetString,
        "cmnQueryDefaultFont", (PFN*)&pcmnQueryDefaultFont,
        "cmnQueryHelpLibrary", (PFN*)&pcmnQueryHelpLibrary,
        "cmnQueryMainResModuleHandle", (PFN*)&pcmnQueryMainResModuleHandle,
        "ctrDefWidgetProc", (PFN*)&pctrDefWidgetProc,
        "ctrFreeSetupValue", (PFN*)&pctrFreeSetupValue,
        "ctrParseColorString", (PFN*)&pctrParseColorString,
        "ctrScanSetupString", (PFN*)&pctrScanSetupString,
        "drv_sprintf", (PFN*)&pdrv_sprintf,
        "dskQueryInfo", (PFN*)&pdskQueryInfo,
        "gpihBox", (PFN*)&pgpihBox,
        "gpihCreateFont", (PFN*)&pgpihCreateFont,
        "gpihDraw3DFrame", (PFN*)&pgpihDraw3DFrame,
        "gpihFindPresFont", (PFN*)&pgpihFindPresFont,
        "gpihMatchFont", (PFN*)&pgpihMatchFont,
        "gpihSetPointSize", (PFN*)&pgpihSetPointSize,
        "gpihSplitPresFont", (PFN*)&pgpihSplitPresFont,
        "gpihSwitchToRGB", (PFN*)&pgpihSwitchToRGB,
        "gpihCreateXBitmap", (PFN*)&pgpihCreateXBitmap,
        "gpihDestroyXBitmap", (PFN*)&pgpihDestroyXBitmap,
        "krnQueryDaemonObject", (PFN*)&pkrnQueryDaemonObject,
        "krnPostThread1ObjectMsg", (PFN*)&pkrnPostThread1ObjectMsg,
        "nlsDateTime2", (PFN*)&pnlsDateTime2,
        "nlsThousandsULong", (PFN*)&pnlsThousandsULong,
        "tmrStartXTimer", (PFN*)&ptmrStartXTimer,
        "tmrStopXTimer", (PFN*)&ptmrStopXTimer,
        "winhFree", (PFN*)&pwinhFree,
        "winhInsertMenuItem", (PFN*)&pwinhInsertMenuItem,
        "winhInsertSubmenu", (PFN*)&pwinhInsertSubmenu,
        "winhQueryPresColor", (PFN*)&pwinhQueryPresColor,
        "winhQueryWindowFont", (PFN*)&pwinhQueryWindowFont,
        "winhSetWindowFont", (PFN*)&pwinhSetWindowFont,
        "xstrcat", (PFN*)&pxstrcat,
        "xstrcatc", (PFN*)&pxstrcatc,
        "xstrClear", (PFN*)&pxstrClear,
        "xstrInit", (PFN*)&pxstrInit
    };

/* ******************************************************************
 *
 *   Private widget instance data
 *
 ********************************************************************/

/*
 *@@ MONITORSETUP:
 *      instance data to which setup strings correspond.
 *      This is also a member of MONITORPRIVATE.
 *
 *      Putting these settings into a separate structure
 *      is no requirement technically. However, once the
 *      widget uses a settings dialog, the dialog must
 *      support changing the widget settings even if the
 *      widget doesn't currently exist as a window, so
 *      separating the setup data from the widget window
 *      data will come in handy for managing the setup
 *      strings.
 */

typedef struct _MONITORSETUP
{
    LONG            lcolBackground,         // background color
                    lcolForeground;         // foreground color (for text)

    PSZ             pszFont;
            // if != NULL, non-default font (in "8.Helv" format);
            // this has been allocated using local malloc()!

    ULONG           cxCurrent,
                    cyCurrent;

    /*
     *  disks to monitor (if MWGT_DISKFREE)
     *
     */

    PSZ             pszDiskList;    // array of plain drive letters to monitor;
                                    // warning, this can be NULL if all drives
                                    // are deselected!
                                    // V0.9.19 (2002-06-15) [umoeller]

} MONITORSETUP, *PMONITORSETUP;

#define DFFL_REPAINT            0x0001
#define DFFL_FLASH              0x0002
#define DFFL_BACKGROUND         0x0004

#define FLASH_MAX               30
            // count of 100 ms intervals that the flash should
            // last; 20 means 2 seconds then

#define WIDGET_BORDER           1

/*
 *@@ DISKDATA:
 *
 *@@added V0.9.14 (2001-08-03) [umoeller]
 */

typedef struct _DISKDATA
{
    LONG        lKB;            // currently free KBS;
                                // if negative, this is an APIRET from xwpdaemon
    ULONG       fl;             // DFFL_* flags

    LONG        lFade;          // fade percentage while (fl & DFFL_FLASH);
                                // initially set to 100 and fading to 0

    LONG        lX,             // cached x position of this drive in RefreshDiskfreeBitmap
                lCX;

} DISKDATA, *PDISKDATA;

/*
 *@@ MONITORPRIVATE:
 *      more window data for the various monitor widgets.
 *
 *      An instance of this is created on WM_CREATE in
 *      fnwpMonitorWidgets and stored in XCENTERWIDGET.pUser.
 */

typedef struct _MONITORPRIVATE
{
    PXCENTERWIDGET pWidget;
            // reverse ptr to general widget data ptr; we need
            // that all the time and don't want to pass it on
            // the stack with each function call

    ULONG           ulType;
                // one of the following:
                // -- MWGT_DATE: date widget
                // -- MWGT_TIME: time widget
                // -- MWGT_SWAPPER: swap monitor widget
                // -- MWGT_MEMORY: memory monitor widget;
                // -- MWGT_POWER: power monitor widget;
                // -- MWGT_DISKFREE: condensed diskfree widget
                // this is copied from the widget class on WM_CREATE

    MONITORSETUP    Setup;
            // widget settings that correspond to a setup string

    ULONG           ulTimerID;      // if != NULLHANDLE, update timer is running

    /*
     *  date/time (for MWGT_DATE, MWGT_TIME)
     *
     */

    CHAR            szDateTime[200];

    /*
     *  APM data (if MWGT_POWER)
     *
     */

    APIRET          arcAPM;         // error code if APM.SYS open failed

    PAPM            pApm;           // APM status data

    HPOINTER        hptrAC,         // "AC" icon
                    hptrBattery;    // "battery" icon

    /*
     *  diskfree data (if MWGT_DISKFREE)
     *
     */

    PDISKDATA       paDiskDatas;    // array of 26 DISKDATA structures
                                    // holding temporary information

    USHORT          idFlashTimer;   // != 0 if flash timer is running

    ULONG           ulPointSize;
    FATTRS          fattrs;
    FONTMETRICS     fm;
    BOOL            fFontMatched;

    PXBITMAP        pBitmap;        // bitmap for diskfree display
                                    // (to avoid flicker)
    LONG            lcidFont;

    ULONG           ulRefreshBitmap;        // 0 = just paint the bitmap
                                            // 1 = update the bitmap partly
                                            // 2 = do a full repaint

    BOOL            fContextMenuHacked;

    // V1.0.8 (2007-01-14) [pr]
    BOOL            fTooltipShowing;    // TRUE only while tooltip is currently
                                        // showing over this widget

    CHAR            szTooltipText[50];  // current tooltip text

} MONITORPRIVATE, *PMONITORPRIVATE;

/* ******************************************************************
 *
 *   Widget settings dialog
 *
 ********************************************************************/

/*
 *      This section contains shared code to manage the
 *      widget's settings. This can translate a widget
 *      setup string into the fields of a binary setup
 *      structure and vice versa. This code is used by
 *      both an open widget window and a settings dialog.
 */

/*
 *@@ MwgtFreeSetup:
 *      cleans up the data in the specified setup
 *      structure, but does not free the structure
 *      itself.
 */

VOID MwgtFreeSetup(PMONITORSETUP pSetup)
{
    if (pSetup)
    {
        if (pSetup->pszFont)
        {
            free(pSetup->pszFont);
            pSetup->pszFont = NULL;
        }

        if (pSetup->pszDiskList)
        {
            free(pSetup->pszDiskList);
            pSetup->pszDiskList = NULL;
        }
    }
}

/*
 *@@ MwgtScanSetup:
 *      scans the given setup string and translates
 *      its data into the specified binary setup
 *      structure.
 *
 *      NOTE: It is assumed that pSetup is zeroed
 *      out. We do not clean up previous data here.
 *
 *@@changed V1.0.2 (2003-02-03) [umoeller]: changed default text color
 */

VOID MwgtScanSetup(PCSZ pcszSetupString,
                   PMONITORSETUP pSetup,
                   BOOL fIsDiskfree)
{
    PSZ p;
    // background color
    if (p = pctrScanSetupString(pcszSetupString,
                                "BGNDCOL"))
    {
        pSetup->lcolBackground = pctrParseColorString(p);
        pctrFreeSetupValue(p);
    }
    else
        // default color:
        pSetup->lcolBackground = WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0);

    // text color:
    if (p = pctrScanSetupString(pcszSetupString,
                                "TEXTCOL"))
    {
        pSetup->lcolForeground = pctrParseColorString(p);
        pctrFreeSetupValue(p);
    }
    else
        pSetup->lcolForeground = WinQuerySysColor(HWND_DESKTOP,
                                                  SYSCLR_WINDOWTEXT,        // changed V1.0.2 (2003-02-03) [umoeller]
                                                  0);

    // font:
    // we set the font presparam, which automatically
    // affects the cached presentation spaces
    if (p = pctrScanSetupString(pcszSetupString,
                                "FONT"))
    {
        pSetup->pszFont = strdup(p);
        pctrFreeSetupValue(p);
    }
    // else: leave this field null

    if (fIsDiskfree)
    {
        // disks to monitor:
        if (p = pctrScanSetupString(pcszSetupString,
                                    "DISKS"))
        {
            pSetup->pszDiskList = strdup(p);
            pctrFreeSetupValue(p);
        }
        else
        {
            ULONG ulBootDrive;
            CHAR ch[3] = "?";
            DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                            &ulBootDrive,
                            sizeof(ulBootDrive));
            ch[0] = ulBootDrive + 'A' - 1;
            pSetup->pszDiskList = strdup(ch);
        }

        // width
        if (p = pctrScanSetupString(pcszSetupString,
                                    "WIDTH"))
        {
            pSetup->cxCurrent = atoi(p);
            pctrFreeSetupValue(p);
        }
        else
            pSetup->cxCurrent = 100;
    }
}

/*
 *@@ MwgtSaveSetup:
 *      composes a new setup string.
 *      The caller must invoke xstrClear on the
 *      string after use.
 *
 *@@added V0.9.7 (2000-12-04) [umoeller]
 */

VOID MwgtSaveSetup(PXSTRING pstrSetup,       // out: setup string (is cleared first)
                   PMONITORSETUP pSetup,
                   BOOL fIsDiskfree)
{
    CHAR    szTemp[100];
    // PSZ     psz = 0;
    pxstrInit(pstrSetup, 100);

    pdrv_sprintf(szTemp, "BGNDCOL=%06lX;",
            pSetup->lcolBackground);
    pxstrcat(pstrSetup, szTemp, 0);

    pdrv_sprintf(szTemp, "TEXTCOL=%06lX;",
            pSetup->lcolForeground);
    pxstrcat(pstrSetup, szTemp, 0);

    if (pSetup->pszFont)
    {
        // non-default font:
        pdrv_sprintf(szTemp, "FONT=%s;",
                pSetup->pszFont);
        pxstrcat(pstrSetup, szTemp, 0);
    }

    if (fIsDiskfree)
    {
        pdrv_sprintf(szTemp, "WIDTH=%d;",
                     pSetup->cxCurrent);
        pxstrcat(pstrSetup, szTemp, 0);

        if (pSetup->pszDiskList)        // V0.9.19 (2002-06-15) [umoeller]
        {
            pdrv_sprintf(szTemp, "DISKS=%s",
                         pSetup->pszDiskList);
            pxstrcat(pstrSetup, szTemp, 0);
        }
    }
}

/*
 *@@ MwgtSaveSetupAndSend:
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

VOID MwgtSaveSetupAndSend(HWND hwnd,
                          PMONITORPRIVATE pPrivate)
{
    XSTRING strSetup;
    MwgtSaveSetup(&strSetup,
                  &pPrivate->Setup,
                  (pPrivate->ulType == MWGT_DISKFREE));
    if (strSetup.ulLength)
        // changed V0.9.13 (2001-06-21) [umoeller]:
        // post it to parent instead of fixed XCenter client
        // to make this trayable
        WinSendMsg(WinQueryWindow(hwnd, QW_PARENT), // pPrivate->pWidget->pGlobals->hwndClient,
                   XCM_SAVESETUP,
                   (MPARAM)hwnd,
                   (MPARAM)strSetup.psz);
    pxstrClear(&strSetup);
}

/* ******************************************************************
 *
 *   Widget settings dialog
 *
 ********************************************************************/

// None currently.

/* ******************************************************************
 *
 *   Helpers
 *
 ********************************************************************/

/*
 *@@ UpdateDiskMonitors:
 *      notifies XWPDAEMN.EXE of the new disks to
 *      be monitored.
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

BOOL UpdateDiskMonitors(HWND hwnd,
                        PMONITORPRIVATE pPrivate)
{
    HWND    hwndDaemon;
    PID     pidDaemon;
    TID     tidDaemon;

    ULONG   cDisks;

    if (    (pPrivate->Setup.pszDiskList)
         && (cDisks = strlen(pPrivate->Setup.pszDiskList))
         && (hwndDaemon = pkrnQueryDaemonObject())
         && (WinQueryWindowProcess(hwndDaemon,
                                   &pidDaemon,
                                   &tidDaemon))
       )
    {
        PADDDISKWATCH pAddDiskWatch = NULL;

        // remove all existing disk watches
        WinSendMsg(hwndDaemon,
                   XDM_REMOVEDISKWATCH,
                   (MPARAM)hwnd,
                   (MPARAM)-1);     // all watches

        if (    (!DosAllocSharedMem((PPVOID)&pAddDiskWatch,
                                    NULL,
                                    sizeof(ADDDISKWATCH),
                                    PAG_COMMIT | OBJ_GIVEABLE | PAG_READ | PAG_WRITE))
             && (!DosGiveSharedMem(pAddDiskWatch,
                                   pidDaemon,
                                   PAG_READ))
            )
        {
            ULONG ul;

            // _PmpfF(("gave 0x%lX to pid 0x%lX", pAddDiskWatch, pidDaemon));

            ZERO(pAddDiskWatch);
            pAddDiskWatch->hwndNotify = hwnd;
            pAddDiskWatch->ulMessage = WM_USER;

            for (ul = 0;
                 ul < cDisks;
                 ul++)
            {
                pAddDiskWatch->ulLogicalDrive
                    = pPrivate->Setup.pszDiskList[ul] - 'A' + 1;
                WinSendMsg(hwndDaemon,
                           XDM_ADDDISKWATCH,
                           (MPARAM)pAddDiskWatch,
                           0);
            }
        }

        if (pAddDiskWatch)
            DosFreeMem(pAddDiskWatch);

        pPrivate->ulRefreshBitmap = 2;
    }

    return FALSE;
}

/* ******************************************************************
 *
 *   PM window class implementation
 *
 ********************************************************************/

/*
 *      This code has the actual PM window class.
 *
 */

/*
 *@@ MwgtCreate:
 *      implementation for WM_CREATE.
 *
 *@@changed V0.9.12 (2001-05-26) [umoeller]: added "power" support
 *@@changed V1.0.9 (2010-07-30) [pr]: fix garbage tooltip on Memory widget
 */

MRESULT MwgtCreate(HWND hwnd,
                   PXCENTERWIDGET pWidget)
{
    MRESULT mrc = 0;        // continue window creation
    ULONG ulUpdateFreq = 1000;

    PMONITORPRIVATE pPrivate = malloc(sizeof(MONITORPRIVATE));
    memset(pPrivate, 0, sizeof(MONITORPRIVATE));
    // link the two together
    pWidget->pUser = pPrivate;
    pPrivate->pWidget = pWidget;

    // get widget type (clock, memory, ...) from class setting;
    // this is lost after WM_CREATE
    if (pWidget->pWidgetClass)
        pPrivate->ulType = pWidget->pWidgetClass->ulExtra;

    pPrivate->Setup.cxCurrent = 10;          // we'll resize ourselves later
    pPrivate->Setup.cyCurrent = 10;

    // initialize binary setup structure from setup string
    MwgtScanSetup(pWidget->pcszSetupString,
                  &pPrivate->Setup,
                  (pPrivate->ulType == MWGT_DISKFREE));

    // set window font (this affects all the cached presentation
    // spaces we use)
    pwinhSetWindowFont(hwnd,
                       (pPrivate->Setup.pszFont)
                        ? pPrivate->Setup.pszFont
                        // default font: use the same as in the rest of XWorkplace:
                        : pcmnQueryDefaultFont());

    // enable context menu help
    pWidget->pcszHelpLibrary = pcmnQueryHelpLibrary();
    switch (pPrivate->ulType)
    {
        case MWGT_DATE:
        case MWGT_TIME:
            pWidget->ulHelpPanelID = ID_XSH_WIDGET_CLOCK_MAIN;
        break;

        /*  disabled V1.0.0 (2002-08-21) [umoeller]
        case MWGT_SWAPPER:
            pWidget->ulHelpPanelID = ID_XSH_WIDGET_SWAP_MAIN;
        break;
        */

        case MWGT_MEMORY:
            ulUpdateFreq = 0;  // V1.0.9
            pWidget->ulHelpPanelID = ID_XSH_WIDGET_MEMORY_MAIN;
        break;

        case MWGT_POWER:
        {
            ULONG ulAction = 0;

            pWidget->ulHelpPanelID = ID_XSH_WIDGET_POWER_MAIN;
            ulUpdateFreq = 10 * 1000;       // once per minute

            if (!(pPrivate->arcAPM = papmhOpen(&pPrivate->pApm)))
            {
                if (pPrivate->pApm->usLowestAPMVersion < 0x101)  // version 1.1 or above
                    pPrivate->arcAPM = -1;
                else
                {
                    // no error at all:
                    // read first value, because next update
                    // won't be before 1 minute from now
                    if (!(pPrivate->arcAPM = papmhReadStatus(pPrivate->pApm, NULL)))
                    {
                        // and load the icons
                        HMODULE hmod = pcmnQueryMainResModuleHandle();
                        pPrivate->hptrAC = WinLoadPointer(HWND_DESKTOP,
                                                          hmod,
                                                          ID_POWER_AC);
                        pPrivate->hptrBattery = WinLoadPointer(HWND_DESKTOP,
                                                               hmod,
                                                               ID_POWER_BATTERY);

                        pPrivate->Setup.cyCurrent = pWidget->pGlobals->cxMiniIcon + 2;
                    }
                }
            }

            if (pPrivate->arcAPM)
                ulUpdateFreq = 0;
        }
        break;

        case MWGT_DISKFREE:
            ulUpdateFreq = 0;           // no timer here

            pWidget->ulHelpPanelID = ID_XSH_WIDGET_DISKFREE_COND;

            // allocate new array of LONGs for the KB values
            pPrivate->paDiskDatas = malloc(sizeof(DISKDATA) * 27);
            memset(pPrivate->paDiskDatas, 0, sizeof(DISKDATA) * 27);

            // tell XWPDaemon about ourselves
            UpdateDiskMonitors(hwnd,
                               pPrivate);
        break;
    }

    // start update timer
    if (ulUpdateFreq)
        pPrivate->ulTimerID = ptmrStartXTimer(pWidget->pGlobals->pvXTimerSet,
                                              hwnd,
                                              1,
                                              ulUpdateFreq);    // 1000, unless "power"

    return mrc;
}

/*
 *@@ FreeBitmapData:
 *
 *@@added V0.9.14 (2001-08-03) [umoeller]
 */

VOID FreeBitmapData(PMONITORPRIVATE pPrivate)
{
    if (pPrivate->pBitmap)
    {
        if (pPrivate->lcidFont)
        {
            GpiSetCharSet(pPrivate->pBitmap->hpsMem, LCID_DEFAULT);
            GpiDeleteSetId(pPrivate->pBitmap->hpsMem, pPrivate->lcidFont);
            pPrivate->lcidFont = NULLHANDLE;
        }

        pgpihDestroyXBitmap(&pPrivate->pBitmap);
    }
}

/*
 *@@ MwgtDestroy:
 *
 *@@added V0.9.12 (2001-05-26) [umoeller]
 *@@changed V0.9.14 (2001-08-01) [umoeller]: fixed memory leak
 */

VOID MwgtDestroy(HWND hwnd)
{
    PXCENTERWIDGET pWidget;
    PMONITORPRIVATE pPrivate;

    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PMONITORPRIVATE)pWidget->pUser)
       )
    {
        if (pPrivate->ulTimerID)
            ptmrStopXTimer(pPrivate->pWidget->pGlobals->pvXTimerSet,
                           pWidget->hwndWidget,
                           pPrivate->ulTimerID);

        if (pPrivate->pApm)
            papmhClose(&pPrivate->pApm);

        if (pPrivate->hptrAC)
            WinDestroyPointer(pPrivate->hptrAC);
        if (pPrivate->hptrBattery)
            WinDestroyPointer(pPrivate->hptrBattery);

        MwgtFreeSetup(&pPrivate->Setup);        // V0.9.14 (2001-08-01) [umoeller]

        if (pPrivate->paDiskDatas)
            free(pPrivate->paDiskDatas);

        FreeBitmapData(pPrivate);

        free(pPrivate);
    } // end if (pPrivate)
}

/*
 *@@ MwgtControl:
 *      implementation for WM_CONTROL.
 *
 *@@added V0.9.7 (2000-12-14) [umoeller]
 *@@changed V0.9.9 (2001-02-01) [umoeller]: added tooltips
 *@@changed V1.0.5 (2005-11-26) [pr]: Fix Date widget tooltip
 *@@changed V1.0.8 (2007-01-14) [pr]: Make Date/Time tooltips dynamic @@fixes 917
 */

BOOL MwgtControl(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    BOOL brc = FALSE;
    PXCENTERWIDGET pWidget;
    PMONITORPRIVATE pPrivate;

    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PMONITORPRIVATE)pWidget->pUser)
       )
    {
        USHORT  usID = SHORT1FROMMP(mp1),
                usNotifyCode = SHORT2FROMMP(mp1);

        switch (usID)
        {
            case ID_XCENTER_CLIENT:
            {
                switch (usNotifyCode)
                {
                    /*
                     * XN_QUERYSIZE:
                     *      XCenter wants to know our size.
                     */

                    case XN_QUERYSIZE:
                    {
                        PSIZEL pszl = (PSIZEL)mp2;
                        pszl->cx = pPrivate->Setup.cxCurrent;
                        pszl->cy = pPrivate->Setup.cyCurrent;
                        brc = TRUE;
                    }
                    break;
                }
            }
            break;

            case ID_XCENTER_TOOLTIP:
                if (usNotifyCode == TTN_NEEDTEXT)
                {
                    PTOOLTIPTEXT pttt = (PTOOLTIPTEXT)mp2;
                    switch (pPrivate->ulType)
                    {
                        case MWGT_DATE:
                        case MWGT_TIME:
                        {
                            PCOUNTRYSETTINGS2 pCountrySettings
                                = (PCOUNTRYSETTINGS2)pWidget->pGlobals->pCountrySettings;
                            DATETIME dt;
                            DosGetDateTime(&dt);
                            // V1.0.5 (2005-11-26) [pr]: Fix Date widget tooltip
                            pnlsDateTime2((pPrivate->ulType == MWGT_TIME) ? pPrivate->szDateTime : NULL,
                                          (pPrivate->ulType == MWGT_DATE) ? pPrivate->szDateTime : NULL,
                                          &dt,
                                          pCountrySettings);

                            pttt->pszText = pPrivate->szDateTime;
                        }
                        break;

                        /*  disabled V1.0.0 (2002-08-21) [umoeller]
                        case MWGT_SWAPPER:
                            pttt->pszText = "Current swapper size";
                        break;
                        */

                        case MWGT_MEMORY:
                            pttt->pszText = (PSZ)pcmnGetString(ID_CRSI_MEMORYTOOLTIP); // "Currently free memory";
                                        // localized V1.0.0 (2002-08-21) [umoeller]
                        break;

                        case MWGT_POWER:
                            pttt->pszText = (PSZ)pcmnGetString(ID_CRSI_BATTERYTOOLTIP); // "Battery power";
                                        // localized V1.0.0 (2002-08-21) [umoeller]
                        break;

                        case MWGT_DISKFREE:
                            pttt->pszText = (PSZ)pcmnGetString(ID_CRSI_DISKFREECONDTOOLTIP); // "Free space on disks";
                                        // localized V1.0.0 (2002-08-21) [umoeller]
                        break;
                    }

                    pttt->ulFormat = TTFMT_PSZ;
                }
                else if (usNotifyCode == TTN_SHOW)  // V1.0.8 (2007-01-14) [pr]
                    pPrivate->fTooltipShowing = TRUE;
                else if (usNotifyCode == TTN_POP)
                    pPrivate->fTooltipShowing = FALSE;
            break;
        } // end switch (usID)
    } // end if (pPrivate)

    return brc;
}

/*
 *@@ CalcHatchCol:
 *
 *@@added V0.9.14 (2001-08-03) [umoeller]
 */

LONG CalcHatchCol(LONG lcolBackground,
                  ULONG ulFade)         // in: 0 <= fade <= FLASH_MAX
{
    // color to be used for hatch on the first flash;
    // we'll calculate a value in between lcolHatchMax
    // and lcolBackground for each flash that comes in...
    // the max color is the inverse of the background
    // color, so we get white for black and vice versa
    // (we use LONGs to avoid overflows below)
    LONG    lBackRed = GET_RED(lcolBackground),         // if white: 255
            lBackGreen = GET_GREEN(lcolBackground),
            lBackBlue = GET_BLUE(lcolBackground);
    LONG    lHatchMaxRed    = 255 - lBackRed,           // if white: 0
            lHatchMaxGreen  = 255 - lBackGreen,
            lHatchMaxBlue   = 255 - lBackBlue;
    LONG    lDiffRed,
            lDiffGreen,
            lDiffBlue;
    // calculate the hatch color
    // based on the current flash value...
    // if we're at FLASH_MAX, we'll use
    // lHatchMax, if we're at 0, we'll
    // use lcolBackground
    lDiffRed   =   (lHatchMaxRed - lBackRed)            // if white: -255
                 * (LONG)ulFade
                 / (LONG)FLASH_MAX;
    lDiffGreen =   (lHatchMaxGreen - lBackGreen)
                 * (LONG)ulFade
                 / (LONG)FLASH_MAX;
    lDiffBlue  =   (lHatchMaxBlue - lBackBlue)
                 * (LONG)ulFade
                 / (LONG)FLASH_MAX;

    return (MAKE_RGB(lBackRed + lDiffRed,
                     lBackGreen + lDiffGreen,
                     lBackBlue + lDiffBlue));
}

/*
 *@@ RefreshDiskfreeBitmap:
 *      called from MwgtPaint2 for diskfree type only.
 *
 *      This is another major mess because this also
 *      handles the flashing for each drive rectangle
 *      individually.
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

VOID RefreshDiskfreeBitmap(HWND hwnd,
                           PMONITORPRIVATE pPrivate,
                           PCOUNTRYSETTINGS2 pCountrySettings,
                           PRECTL prclWin,          // exclusive!
                           HPS hpsWin,
                           BOOL fPaintAll)
{
    LONG    lcolBackground = pPrivate->Setup.lcolBackground;

    ULONG   cx = prclWin->xRight - prclWin->xLeft,
            cy = prclWin->yTop - prclWin->yBottom;

    HPS hpsMem;

    if (!pPrivate->pBitmap)
    {
        // bitmap needs to be created:
        pPrivate->pBitmap = pgpihCreateXBitmap(pPrivate->pWidget->habWidget,
                                               cx,
                                               cy);
        hpsMem = pPrivate->pBitmap->hpsMem;

        if (!pPrivate->fFontMatched)
        {
            // this is the very first time that we're called
            // to create the bitmap (or the font has changed
            // in MwgtPresParamChanged): go find the font
            // then
            PSZ pszFace = pPrivate->Setup.pszFont;
            pgpihSplitPresFont(pPrivate->Setup.pszFont,
                               &pPrivate->ulPointSize,
                               &pszFace);

            pgpihMatchFont(hpsMem,
                           pPrivate->ulPointSize,
                           FALSE,       // face mode
                           pszFace,
                           0,
                           &pPrivate->fattrs,       // FATTRS
                           &pPrivate->fm);         // FONTMETRICS

            pPrivate->fFontMatched = TRUE;
        }

        pPrivate->lcidFont = pgpihCreateFont(hpsMem,
                                             &pPrivate->fattrs);
        GpiSetCharSet(hpsMem,
                      pPrivate->lcidFont);
        if (pPrivate->fm.fsDefn & FM_DEFN_OUTLINE)
            pgpihSetPointSize(hpsMem, pPrivate->ulPointSize);

        // enforce repaint below
        pPrivate->ulRefreshBitmap = 2;
        fPaintAll = TRUE;
    }
    else
        hpsMem = pPrivate->pBitmap->hpsMem;

    // now make rclWin inclusive
    (prclWin->xRight)--;
    (prclWin->yTop)--;

    /* GpiIntersectClipRectangle(hpsWin,
                              prclWin); */

    if (pPrivate->pBitmap)
    {
        POINTL      ptlBmpDest;

        if (pPrivate->ulRefreshBitmap)
        {
            POINTL ptlNow1,
                   ptlTemp;

            if (pPrivate->ulRefreshBitmap == 2)
                fPaintAll = TRUE;

            if (fPaintAll)
            {
                RECTL rcl = {0, 0, cx-1, cy-1};
                GpiSetColor(hpsMem,
                            pPrivate->Setup.lcolBackground);
                pgpihBox(hpsMem,
                         DRO_FILL,
                         &rcl);
            }

            if (pPrivate->Setup.pszDiskList)
            {
                ULONG   cDisks = strlen(pPrivate->Setup.pszDiskList),
                        ul;
                CHAR    szTemp2[50];
                LONG    lcolHatchNow = -1;     // calc on first need

                #define X_OFS 10

                ptlNow1.x = 0;
                ptlNow1.y = cy / 2;
                // GpiMove(hpsMem, &ptlNow1);

                GpiSetTextAlignment(hpsMem,
                                    TA_LEFT,
                                    TA_HALF);

                GpiSetColor(hpsMem,
                            pPrivate->Setup.lcolForeground);
                GpiSetBackColor(hpsMem,
                                pPrivate->Setup.lcolBackground);

                // now repaint all the disk entries
                // (for each character in the pszDisks string)
                for (ul = 0;
                     ul < cDisks;
                     ul++)
                {
                    // drive letter
                    CHAR c = pPrivate->Setup.pszDiskList[ul];
                    if ((c > 'A') && (c <= 'Z'))
                    {
                        // get the offset into paDiskDatas array
                        ULONG ulLogicalDrive = c - 'A' + 1;
                        PDISKDATA pDataThis = &pPrivate->paDiskDatas[ulLogicalDrive];

                        if (    (fPaintAll)
                             || (pDataThis->fl & DFFL_REPAINT)
                           )
                        {
                            ULONG ulLength;
                            LONG l = pDataThis->lKB;
                            LONG lBackMix = BM_OVERPAINT;

                            if (l >= 0)
                            {
                                CHAR szTemp3[50];
                                pdrv_sprintf(szTemp2,
                                             "%c:%sM",
                                             // drive letter:
                                             c,
                                             // free KB:
                                             pnlsThousandsULong(szTemp3,
                                                                 (l + 512) / 1024,
                                                                 pCountrySettings->cs.cThousands));
                            }
                            else
                                // error:
                                pdrv_sprintf(szTemp2,
                                             "%c: E%d",
                                             // drive letter:
                                             c,
                                             // error (negative):
                                             -l);

                            ulLength = strlen(szTemp2);

                            // if we're not repainting all,
                            // we must use the xpos we saved from a
                            // previous run where we stored them all
                            if (!fPaintAll)
                                ptlNow1.x = pDataThis->lX;
                            else
                                pDataThis->lX = ptlNow1.x;

                            if (    (fPaintAll)
                                 || (pDataThis->fl & (DFFL_FLASH | DFFL_BACKGROUND))
                               )
                            {
                                POINTL      aptlText[TXTBOX_COUNT];
                                POINTL      ptlRect;
                                            // ptlNow2;
                                // find the size of the rectangle we need
                                GpiQueryTextBox(hpsMem,
                                                ulLength,
                                                szTemp2,
                                                TXTBOX_COUNT,
                                                aptlText);
                                // GpiQueryCurrentPosition(hpsMem, &ptlNow2);

                                ptlRect.x = ptlNow1.x; //  - (X_OFS / 2);
                                ptlRect.y = 0;

                                GpiMove(hpsMem, &ptlRect);

                                if (aptlText[TXTBOX_TOPRIGHT].x != pDataThis->lCX)
                                {
                                    // whoa, width changed:
                                    pDataThis->lCX = aptlText[TXTBOX_TOPRIGHT].x;
                                    // redraw all from now
                                    fPaintAll = TRUE;
                                }

                                if (pDataThis->fl & DFFL_FLASH)
                                {
                                    // flash, but not background:
                                    GpiSetColor(hpsMem,
                                                CalcHatchCol(lcolBackground,
                                                             pDataThis->lFade));

                                    GpiSetPattern(hpsMem, PATSYM_DIAG1);
                                }
                                else
                                {
                                    // background only:
                                    GpiSetColor(hpsMem,
                                                pPrivate->Setup.lcolBackground);

                                    GpiSetPattern(hpsMem, PATSYM_DEFAULT);
                                }

                                // draw the flash or background
                                ptlRect.x += aptlText[TXTBOX_TOPRIGHT].x + X_OFS;
                                ptlRect.y = cy - 1;     // inclusive!
                                GpiSetBackMix(hpsMem, BM_OVERPAINT);
                                GpiBox(hpsMem,
                                       DRO_FILL,
                                       &ptlRect,
                                       0,
                                       0);

                                GpiSetBackMix(hpsMem, BM_LEAVEALONE);

                                /* if (fPaintAll)
                                    pDataThis->lX = ptlRect.x; */

                                GpiSetColor(hpsMem,
                                            pPrivate->Setup.lcolForeground);

                                // unset background flag, if this was set
                                pDataThis->fl &= ~DFFL_BACKGROUND;
                            }

                            // GpiSetBackMix(hpsMem, lBackMix);

                            /* if (!fPaintAll)
                                if (ptlNow.x != pDataThis->lX)
                                    // xpos has changed compared to
                                    // previous run (e.g. string has become
                                    // wider):
                                    fPaintAll = TRUE; */

                            ptlTemp.x = ptlNow1.x + (X_OFS / 2);
                            ptlTemp.y = ptlNow1.y;
                            GpiMove(hpsMem, &ptlTemp);
                            GpiCharString(hpsMem,
                                          ulLength,
                                          szTemp2);

                            // unset repaint flag
                            pDataThis->fl &= ~DFFL_REPAINT;

                            GpiQueryCurrentPosition(hpsMem,
                                                    &ptlNow1);
                            ptlNow1.x += (X_OFS / 2);
                        }
                    }
                }
            }

            // unset refresh flag
            pPrivate->ulRefreshBitmap = 0;

        } // end if (pPrivate->fRefreshBitmap)

        // now go blit the bitmap
        ptlBmpDest.x = prclWin->xLeft;
        ptlBmpDest.y = prclWin->yBottom;
        // now paint graph from bitmap
        WinDrawBitmap(hpsWin,
                      pPrivate->pBitmap->hbm,
                      NULL,     // entire bitmap
                      &ptlBmpDest,
                      0, 0,
                      DBM_NORMAL);
    } // end if pPrivate->pBitmap)
}

/*
 * MwgtPaint2:
 *      implementation for WM_PAINT.
 *
 *      The specified HPS is switched to RGB mode before
 *      painting.
 *
 *@@changed V0.9.12 (2001-05-26) [umoeller]: added "power" support
 *@@changed V0.9.16 (2002-01-13) [umoeller]: no longer allowing the widget to shrink, just expand
 *@@changed V1.0.8 (2007-01-14) [pr]: Make Date/Time tooltips dynamic @@fixes 917
 */

VOID MwgtPaint2(HWND hwnd,
                PMONITORPRIVATE pPrivate,
                HPS hps,
                BOOL fDrawFrame)
{
    RECTL       rclWin;
    CHAR        szPaint[400] = "";
    ULONG       ulPaintLen = 0;
    POINTL      aptlText[TXTBOX_COUNT];
    ULONG       ulExtraWidth = 0;           // for battery icon only

    BOOL        fSharedPaint = TRUE;

    // country settings from XCenter globals
    // (what a pointer)
    PCOUNTRYSETTINGS2 pCountrySettings
        = (PCOUNTRYSETTINGS2)pPrivate->pWidget->pGlobals->pCountrySettings;

    // now paint button frame
    WinQueryWindowRect(hwnd, &rclWin);
    pgpihSwitchToRGB(hps);

    if (pPrivate->pWidget->pGlobals->flDisplayStyle & XCS_SUNKBORDERS)
    {
        if (fDrawFrame)
        {
            RECTL rcl2;
            rcl2.xLeft = rclWin.xLeft;
            rcl2.yBottom = rclWin.yBottom;
            rcl2.xRight = rclWin.xRight - 1;
            rcl2.yTop = rclWin.yTop - 1;
            pgpihDraw3DFrame(hps,
                             &rcl2,
                             WIDGET_BORDER,
                             pPrivate->pWidget->pGlobals->lcol3DDark,
                             pPrivate->pWidget->pGlobals->lcol3DLight);
        }

        // now paint middle
        rclWin.xLeft += WIDGET_BORDER;
        rclWin.yBottom += WIDGET_BORDER;
        rclWin.xRight -= WIDGET_BORDER;
        rclWin.yTop -= WIDGET_BORDER;
    }

    if ((fDrawFrame) && (pPrivate->ulType != MWGT_DISKFREE))
        WinFillRect(hps,
                    &rclWin,
                    pPrivate->Setup.lcolBackground);

    switch (pPrivate->ulType)
    {
        case MWGT_DATE:
        case MWGT_TIME:
        {
            DATETIME    DateTime;

            DosGetDateTime(&DateTime);
            pnlsDateTime2((pPrivate->ulType == MWGT_DATE) ? szPaint : pPrivate->szTooltipText,  // V1.0.8 (2007-01-14) [pr]
                          (pPrivate->ulType == MWGT_TIME) ? szPaint : pPrivate->szTooltipText,
                          &DateTime,
                          pCountrySettings);
        }
        break;

        /* disabled V1.0.0 (2002-08-21) [umoeller]
        case MWGT_SWAPPER:
        break;
        */

        case MWGT_MEMORY:
        {
            ULONG ulMem = 0;
            APIRET arc;
            if (!(arc = Dos16MemAvail(&ulMem)))
            {
                pnlsThousandsULong(szPaint,
                                    ulMem / 1024,
                                    pCountrySettings->cs.cThousands);
                strcat(szPaint, " KB");
            }
            else
                pdrv_sprintf(szPaint, "E %d", arc);
        }
        break;

        case MWGT_POWER:
            if (pPrivate->arcAPM == -1)
                // insufficient APM version:
                strcpy(szPaint, "APM 1.2 required.");
            else if (pPrivate->arcAPM)
                // other error occurred:
                pdrv_sprintf(szPaint, "E %d", pPrivate->arcAPM);
            else
            {
                // APM is OK:
                PCSZ pcsz = NULL;
                switch (pPrivate->pApm->bBatteryStatus)
                {
                    case 0x00: pcsz = "High"; break;
                    case 0x01: pcsz = "Low"; break;
                    case 0x02: pcsz = "Critical"; break;
                    case 0x03: pcsz = "Charging"; break;

                    default: pdrv_sprintf(szPaint, "No battery");
                }
                if (pcsz)
                {
                    ULONG cxMiniIcon = pPrivate->pWidget->pGlobals->cxMiniIcon;

                    pdrv_sprintf(szPaint, "%d%%", pPrivate->pApm->bBatteryLife);

                    WinDrawPointer(hps,
                                   0,
                                   (    (rclWin.yTop - rclWin.yBottom)
                                      - cxMiniIcon
                                   ) / 2,
                                   (pPrivate->pApm->fUsingAC)
                                        ? pPrivate->hptrAC
                                        : pPrivate->hptrBattery,
                                   DP_MINI);

                    // add offset for painting text
                    ulExtraWidth = cxMiniIcon;
                }
            }
        break;

        case MWGT_DISKFREE:
            #ifdef DEBUG_XTIMERS
                _PmpfF(("calling RefreshDiskfreeBitmap"));
            #endif
            RefreshDiskfreeBitmap(hwnd,
                                  pPrivate,
                                  pCountrySettings,
                                  &rclWin,
                                  hps,
                                  fDrawFrame);        // paint all?
            fSharedPaint = FALSE;
        break;
    }

    // fSharedPaint is true for all except diskfree V0.9.14 (2001-08-01) [umoeller]
    if (fSharedPaint)
    {
        ulPaintLen = strlen(szPaint);
        GpiQueryTextBox(hps,
                        ulPaintLen,
                        szPaint,
                        TXTBOX_COUNT,
                        aptlText);
        /* if (    abs((aptlText[TXTBOX_TOPRIGHT].x + ulExtraWidth) + 4 - rclWin.xRight)
                > 4
           ) */
        // expand ourselves only, but never shrink
        // V0.9.16 (2002-01-13) [umoeller]
        if (   (aptlText[TXTBOX_TOPRIGHT].x + ulExtraWidth) + 4
             > rclWin.xRight
           )
        {
            // we need more space: tell XCenter client
            pPrivate->Setup.cxCurrent = (   aptlText[TXTBOX_TOPRIGHT].x
                                          + ulExtraWidth
                                          + 2 * WIDGET_BORDER
                                          + 4);
            WinPostMsg(WinQueryWindow(hwnd, QW_PARENT),
                       XCM_SETWIDGETSIZE,
                       (MPARAM)hwnd,
                       (MPARAM)pPrivate->Setup.cxCurrent);
        }
        else
        {
            // sufficient space:
            // GpiSetBackMix(hps, BM_OVERPAINT);
                    // now using DT_ERASERECT V0.9.16 (2002-01-13) [umoeller]
            rclWin.xLeft += ulExtraWidth;
            WinDrawText(hps,
                        ulPaintLen,
                        szPaint,
                        &rclWin,
                        pPrivate->Setup.lcolForeground,
                        pPrivate->Setup.lcolBackground,
                        DT_ERASERECT | DT_CENTER | DT_VCENTER);
        }
    }

    // V1.0.8 (2007-01-14) [pr]
    if (pPrivate->fTooltipShowing)
        // tooltip currently showing:
        // refresh its display
        WinSendMsg(pPrivate->pWidget->pGlobals->hwndTooltip,
                   TTM_UPDATETIPTEXT,
                   (MPARAM)pPrivate->szTooltipText,
                   0);

}

/*
 *@@ ForceRepaint:
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

VOID ForceRepaint(PMONITORPRIVATE pPrivate)
{
    HWND hwnd = pPrivate->pWidget->hwndWidget;
    HPS hps;
    if (hps = WinGetPS(hwnd))
    {
        // force refresh of bitmap (diskfree only)
        pPrivate->ulRefreshBitmap = 1;

        MwgtPaint2(hwnd,
                   pPrivate,
                   hps,
                   FALSE);   // text only

        WinReleasePS(hps);
    } // end if (pPrivate)
}

/*
 *@@ MwgtPaint:
 *      implementation for WM_PAINT.
 *
 *@@added V0.9.19 (2002-06-08) [umoeller]
 */

STATIC VOID MwgtPaint(HWND hwnd)
{
    HPS hps;
    if (hps = WinBeginPaint(hwnd, NULLHANDLE, NULL))
    {
        // get widget data and its button data from QWL_USER
        PXCENTERWIDGET pWidget;
        PMONITORPRIVATE pPrivate;

        if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
             && (pPrivate = (PMONITORPRIVATE)pWidget->pUser)
           )
        {
            MwgtPaint2(hwnd,
                       pPrivate,
                       hps,
                       TRUE);        // draw everything
        }

        WinEndPaint(hps);
    }
}

/*
 *@@ MwgtTimer:
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 *@@changed V0.9.16 (2001-09-20) [umoeller]: battery widget didn't repaint background on status change, fixed
 */

VOID MwgtTimer(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    USHORT usTimerID = (USHORT)mp1;

    PXCENTERWIDGET pWidget;
    PMONITORPRIVATE pPrivate;

    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PMONITORPRIVATE)pWidget->pUser)
       )
    {
        switch (usTimerID)
        {
            case 1:     // timer 1: general update timer for all widgets
                        // except diskfree
            {
                #ifdef DEBUG_XTIMERS
                _Pmpf(("  " __FUNCTION__ ": timer 1"));
                #endif

                if (    (pPrivate->ulType == MWGT_POWER)
                     && (pPrivate->pApm)
                     && (!pPrivate->arcAPM)
                   )
                {
                    BOOL fChanged = FALSE;
                    if (    (pPrivate->arcAPM = papmhReadStatus(pPrivate->pApm,
                                                                &fChanged))
                         || (fChanged)
                       )
                        // ForceRepaint isn't enough here because we must
                        // also invalidate the background
                        // V0.9.16 (2001-09-20) [umoeller]
                        WinInvalidateRect(pWidget->hwndWidget, NULL, FALSE);
                }
                else
                    ForceRepaint(pPrivate);
            }
            break;

            case 2000:     // timer 2: temporary timer for diskfree only
                           // while flashing
                if (pPrivate->paDiskDatas)
                {
                    ULONG ulLogicalDrive,
                          cFlashing = 0;

                    // _PmpfF(("timer 2000"));

                    // run over the array of DISKDATA's and update
                    // all the drive flags that are currently flashing;
                    // after this, force repaint
                    for (ulLogicalDrive = 1;
                         ulLogicalDrive < 27;
                         ulLogicalDrive++)
                    {
                        PDISKDATA pThis = &pPrivate->paDiskDatas[ulLogicalDrive];

                        if (pThis->lFade > 0)
                        {
                            // still fading:
                            if (pThis->lKB < 0)
                            {
                                // if we have an error,
                                // flash with double speed
                                pThis->lFade -= 4;
                            }
                            else
                                (pThis->lFade)--;

                            pThis->fl |= (DFFL_REPAINT | DFFL_BACKGROUND);

                            if (pThis->lFade <= 0)
                            {
                                // now zero:
                                // if we had an error here, restart
                                if (pThis->lKB < 0)
                                {
                                    pThis->lFade = FLASH_MAX;
                                    cFlashing++;
                                }
                                else
                                    // just a change:
                                    // stop flashing here
                                    pThis->fl &= ~DFFL_FLASH;
                            }
                            else
                                // count disks which are still flashing
                                cFlashing++;
                        }
                    }

                    if (!cFlashing)
                    {
                        // no more flashing disks left:
                        // stop the timer then
                        ptmrStopXTimer(pWidget->pGlobals->pvXTimerSet,
                                       pWidget->hwndWidget,
                                       usTimerID);
                        pPrivate->idFlashTimer = 0;
                    }

                    ForceRepaint(pPrivate);
                }
            break;

            default:
                pctrDefWidgetProc(pWidget->hwndWidget, WM_TIMER, mp1, mp2);
        }
    }
}

/*
 *@@ UpdateLogicalDrive:
 *      implementation for WM_USER in fnwpMonitorWidgets.
 *
 *      WM_USER gets posted from XWPDAEMN.EXE whenever the
 *      disk monitor has detected that the free space on
 *      a drive has changed, even it's only a single byte.
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

VOID UpdateLogicalDrive(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    PXCENTERWIDGET pWidget;
    PMONITORPRIVATE pPrivate;

    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PMONITORPRIVATE)pWidget->pUser)
         && (pPrivate->paDiskDatas)
         && ((ULONG)mp1 > 0)
         && ((ULONG)mp1 < 27)
       )
    {
        PDISKDATA       pThis = &pPrivate->paDiskDatas[(ULONG)mp1];

        // _PmpfF(("drive %d", (ULONG)mp1));

        // flash the rectangle
        pThis->fl |= (DFFL_FLASH | DFFL_REPAINT);
        pThis->lFade = FLASH_MAX;

        pThis->lKB = (LONG)mp2;

        if (!pPrivate->idFlashTimer)
        {
            // no timer is running, start it
            pPrivate->idFlashTimer = ptmrStartXTimer(pWidget->pGlobals->pvXTimerSet,
                                                     pWidget->hwndWidget,
                                                     2000,          // timer ID
                                                     100);
        }

        ForceRepaint(pPrivate);
    }
}

/*
 *@@ MwgtWindowPosChanged:
 *      implementation for WM_WINDOWPOSCHANGED.
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

VOID MwgtWindowPosChanged(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    PXCENTERWIDGET pWidget;
    PMONITORPRIVATE pPrivate;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PMONITORPRIVATE)pWidget->pUser)
         && (pPrivate->ulType == MWGT_DISKFREE)
       )
    {
        PSWP pswpNew = (PSWP)mp1,
             pswpOld = pswpNew + 1;
        if (pswpNew->fl & SWP_SIZE)
        {
            // window was resized:
            pPrivate->Setup.cxCurrent = pswpNew->cx;
            MwgtSaveSetupAndSend(hwnd,
                                 pPrivate);

            // destroy the buffer bitmap because we
            // need a new one with a different size
            FreeBitmapData(pPrivate);

            WinInvalidateRect(hwnd, NULL, FALSE);
        } // end if (pswpNew->fl & SWP_SIZE)
    } // end if (pPrivate)
}

/*
 *@@ MwgtPresParamChanged:
 *      implementation for WM_PRESPARAMCHANGED.
 *
 *@@changed V0.9.13 (2001-06-21) [umoeller]: changed XCM_SAVESETUP call for tray support
 *@@changed V1.0.8 (2007-08-05) [pr]: rewrote this mess @@fixes 994
 */

VOID MwgtPresParamChanged(HWND hwnd,
                          ULONG ulAttrChanged)
{
    PXCENTERWIDGET pWidget;
    PMONITORPRIVATE pPrivate;
    // WM_PRESPARAMCHANGED gets sent before XCENTERWIDGET is set!
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PMONITORPRIVATE)pWidget->pUser)
       )
    {
        BOOL fInvalidate = TRUE;

        switch (ulAttrChanged)  // V1.0.8 (2007-08-05) [pr]
        {
            case 0:     // layout palette thing dropped
                pPrivate->Setup.lcolBackground
                    = pwinhQueryPresColor(hwnd,
                                          PP_BACKGROUNDCOLOR,
                                          FALSE,
                                          SYSCLR_DIALOGBACKGROUND);
                pPrivate->Setup.lcolForeground
                    = pwinhQueryPresColor(hwnd,
                                          PP_FOREGROUNDCOLOR,
                                          FALSE,
                                          SYSCLR_WINDOWTEXT);
            break;

            case PP_BACKGROUNDCOLOR:
                pPrivate->Setup.lcolBackground
                    = pwinhQueryPresColor(hwnd,
                                          PP_BACKGROUNDCOLOR,
                                          FALSE,
                                          SYSCLR_DIALOGBACKGROUND);
            break;

            case PP_FOREGROUNDCOLOR:
                pPrivate->Setup.lcolForeground
                    = pwinhQueryPresColor(hwnd,
                                          PP_FOREGROUNDCOLOR,
                                          FALSE,
                                          SYSCLR_WINDOWTEXT);
            break;

            case PP_FONTNAMESIZE:
            {
                PSZ pszFont;
                if (pPrivate->Setup.pszFont)
                {
                    free(pPrivate->Setup.pszFont);
                    pPrivate->Setup.pszFont = NULL;
                }

                if (pszFont = pwinhQueryWindowFont(hwnd))
                {
                    // we must use local malloc() for the font
                    pPrivate->Setup.pszFont = strdup(pszFont);
                    pwinhFree(pszFont);
                }

                // resize ourselves based on the font in WM_PAINT
                // V0.9.16 (2002-01-13) [umoeller]
                if (pPrivate->ulType != MWGT_DISKFREE)
                    WinPostMsg(WinQueryWindow(hwnd, QW_PARENT),
                               XCM_SETWIDGETSIZE,
                               (MPARAM)hwnd,
                               (MPARAM)10);

                // re-match the font now
                pPrivate->fFontMatched = FALSE;
            }
            break;

            default:
                fInvalidate = FALSE;
        }

        if (fInvalidate)
        {
            FreeBitmapData(pPrivate);

            WinInvalidateRect(hwnd, NULL, FALSE);

            MwgtSaveSetupAndSend(hwnd, pPrivate);
        }
    } // end if (pPrivate)
}

/*
 *@@ FindDriveFromWidgetX:
 *      returns the drive letter of the drive
 *      corresponding to the given X position (in
 *      widget coordinates), or 0 if there's none.
 *
 *@@added V0.9.18 (2002-03-23) [umoeller]
 */

CHAR FindDriveFromWidgetX(PMONITORPRIVATE pPrivate,
                          SHORT x)
{
    ULONG   cDisks;
    if (    (pPrivate->Setup.pszDiskList)
         && (cDisks = strlen(pPrivate->Setup.pszDiskList))
       )
    {
        ULONG   ul;
        for (ul = 0;
             ul < cDisks;
             ul++)
        {
            // drive letter
            CHAR c = pPrivate->Setup.pszDiskList[ul];
            if ((c > 'A') && (c <= 'Z'))
            {
                ULONG ulLogicalDrive = c - 'A' + 1;
                // get the offset into paDiskDatas array
                PDISKDATA pDataThis = &pPrivate->paDiskDatas[ulLogicalDrive];

                if (    (x > pDataThis->lX)
                     && (x < pDataThis->lX + pDataThis->lCX)
                   )
                {
                    // got it:
                    return c;
                }
            }
        }
    }

    return '\0';
}

/*
 *@@ MwgtButton1DblClick:
 *      implementation for WM_BUTTON1DBLCLK.
 *
 *@@changed V0.9.12 (2001-05-26) [umoeller]: added "power" support
 *@@changed V0.9.18 (2002-03-23) [umoeller]: now supporting eCSClock on eCS
 *@@changed V0.9.18 (2002-03-23) [umoeller]: now supporting double-click on diskfree to open drive
 *@@changed V0.9.19 (2002-04-02) [umoeller]: eCSClock opened settings, fixed
 *@@changed V1.0.2 (2003-03-07) [umoeller]: now posting msg to t1 obj for opening objects @@fixes 398
 *@@changed V1.0.5 (2005-11-26) [pr]: Open eCSClock first, then System Clock if it fails
 */

VOID MwgtButton1DblClick(HWND hwnd,
                         MPARAM mp1)
{
    PXCENTERWIDGET pWidget;
    PMONITORPRIVATE pPrivate;

    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PMONITORPRIVATE)pWidget->pUser)
       )
    {
        PCSZ        pcszID = NULL,
                    pcszBackupID = NULL;
        HOBJECT     hobj = NULLHANDLE;
        ULONG       ulView = 2, // OPEN_SETTINGS,
                    ulBackupView = 0;       // OPEN_DEFAULT
        CHAR        szTemp[50];

        switch (pPrivate->ulType)
        {
            case MWGT_DATE:
            case MWGT_TIME:
                pcszID = "<ECLOCK_CLKSET>";
                pcszBackupID = "<WP_CLOCK>";
                ulView = 0; // OPEN_DEFAULT
                ulBackupView = 2; // OPEN_SETTINGS
                        // changed V1.0.5 (2005-11-26) [pr]
            break;

            // case MWGT_SWAPPER:       disabled V1.0.0 (2002-08-21) [umoeller]
            case MWGT_MEMORY:
                pcszID = "<XWP_KERNEL>"; // XFOLDER_KERNELID; // "<XWP_KERNEL>";
            break;

            case MWGT_POWER:
                pcszID = "<WP_POWER>";
                ulView = 0; // OPEN_DEFAULT;
            break;

            case MWGT_DISKFREE:     // V0.9.18 (2002-03-23) [umoeller]
            {
                CHAR c;
                if (c = FindDriveFromWidgetX(pPrivate,
                                             SHORT1FROMMP(mp1)))
                {
                    // open the drive
                    sprintf(szTemp,
                            "<WP_DRIVE_%c>",
                            c);
                    pcszID = szTemp;
                    ulView = 0; // OPEN_DEFAULT
                }
            }
            break;
        }

        if (pcszID)
            hobj = WinQueryObject((PSZ)pcszID);

        // if we have a backup ID, try that
        // V0.9.18 (2002-03-23) [umoeller]
        if (!hobj && pcszBackupID)
        {
            hobj = WinQueryObject((PSZ)pcszBackupID);
            ulView = ulBackupView;
            _Pmpf(("%s got hobj 0x%lX", pcszBackupID, hobj));
        }

        if (hobj)
        {
            // changed V1.0.2 (2003-03-07) [umoeller] @@fixes 398
            pkrnPostThread1ObjectMsg(T1M_OPENOBJECTFROMHANDLE2,
                                    (MPARAM)hobj,
                                    (MPARAM)ulView);

            /*
            if (WinOpenObject(hobj,
                              ulView,
                              TRUE))
                WinOpenObject(hobj,
                              ulView,
                              TRUE);
            */
        }
    } // end if (pPrivate)
}

/*
 *@@ HackContextMenu:
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 *@@changed V0.9.16 (2001-12-06) [umoeller]: it was impossible to remove a drive that no longer existed; fixed
 */

VOID HackContextMenu(PMONITORPRIVATE pPrivate)
{
    XDISKINFO aDiskInfos[26];

    HPOINTER hptrOld = WinQueryPointer(HWND_DESKTOP);
    WinSetPointer(HWND_DESKTOP,
                  WinQuerySysPointer(HWND_DESKTOP,
                                     SPTR_WAIT,
                                     FALSE));   // no copy

    // ask xwpdaemn.exe about available drives
    // (this call can take a while)

    if (pdskQueryInfo(aDiskInfos,
                      -1))           // all disks
    {
        // insert the "Drives" submenu after "Properties"

        HWND hwndSubmenu;
        SHORT s = (SHORT)WinSendMsg(pPrivate->pWidget->hwndContextMenu,
                                    MM_ITEMPOSITIONFROMID,
                                    MPFROM2SHORT(ID_CRMI_HELP,      // adjusted V0.9.20 (2002-08-08) [umoeller]
                                                 FALSE),
                                    0);
        if (hwndSubmenu = pwinhInsertSubmenu(pPrivate->pWidget->hwndContextMenu,
                                             0,
                                             1999,
                                             "Drives",
                                             MIS_TEXT,
                                             0, NULL, 0, 0))
        {
            ULONG ul;
            for (ul = 0;
                 ul < 26;
                 ul++)
            {
                PXDISKINFO pThis = &aDiskInfos[ul];
                BOOL fIsDisplaying = (    (pPrivate->Setup.pszDiskList)
                                       && (strchr(pPrivate->Setup.pszDiskList,
                                                  pThis->cDriveLetter))
                                     );

                if (   // list all fixed drives
                       (pThis->flDevice & DFL_FIXED)
                       // even if the drive is not listed as "fixed", make
                       // sure we list the drives that are currently showing;
                       // otherwise the user can't get rid of broken drives
                       // V0.9.16 (2001-12-06) [umoeller]
                     || (fIsDisplaying)
                   )
                {
                    CHAR szText[10];
                    ULONG afAttr = 0;
                    pdrv_sprintf(szText, "%c:", pThis->cDriveLetter);
                    if (fIsDisplaying)
                        afAttr |= MIA_CHECKED;

                    pwinhInsertMenuItem(hwndSubmenu,
                                        MIT_END,
                                        2000 + pThis->cLogicalDrive,
                                        szText,
                                        MIS_TEXT,
                                        afAttr);
                }
            }

            pwinhInsertMenuItem(pPrivate->pWidget->hwndContextMenu,
                                1,
                                0,
                                "",
                                MIS_SEPARATOR,
                                0);

            pPrivate->fContextMenuHacked = TRUE;
        }
    }

    WinSetPointer(HWND_DESKTOP, hptrOld);
}

/*
 *@@ MwgtContextMenu:
 *      implementation for WM_CONTEXTMENU.
 *
 *      We override the default behavior of the standard
 *      widget window proc for the diskfree widget.
 *
 *@@added V0.9.8 (2001-01-10) [umoeller]
 */

MRESULT MwgtContextMenu(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    PXCENTERWIDGET pWidget;
    PMONITORPRIVATE pPrivate;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PMONITORPRIVATE)pWidget->pUser)
       )
    {
        if (    (pPrivate->ulType == MWGT_DISKFREE)
             && (pWidget->hwndContextMenu)
             && (!pPrivate->fContextMenuHacked)
           )
        {
            // first call for diskfree:
            // hack the context menu given to us
            HackContextMenu(pPrivate);
        }

        return pctrDefWidgetProc(hwnd, WM_CONTEXTMENU, mp1, mp2);
    }

    return 0;
}

/*
 *@@ MwgtMenuSelect:
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

MRESULT MwgtMenuSelect(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    PXCENTERWIDGET pWidget;
    PMONITORPRIVATE pPrivate;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PMONITORPRIVATE)pWidget->pUser)
       )
    {
        USHORT usItem = SHORT1FROMMP(mp1);
        if (    (pPrivate->ulType == MWGT_DISKFREE)
                // one of the drive letter items?
             && (usItem >= 2000)
             && (usItem <= 2026)
           )
        {
            if (SHORT2FROMMP(mp1) == TRUE)      // WM_COMMAND to be posted:
            {
                ULONG ulLogicalDrive = usItem - 2000;

                // recompose the string of drive letters to
                // be monitored...
                CHAR szNew[30],
                     szNew2[30];
                PSZ pTarget = szNew2;
                ULONG ul;
                BOOL fInList = FALSE;

                memset(szNew, 0, sizeof(szNew));

                if (pPrivate->Setup.pszDiskList)
                {
                    for (ul = 1;
                         ul < 27;
                         ul++)
                    {
                        if (strchr(pPrivate->Setup.pszDiskList,
                                   ul + 'A' - 1))
                        {
                            if (ul == ulLogicalDrive)
                                fInList = TRUE;
                                // drop it
                            else
                                szNew[ul] = 1;
                        }
                    }

                    free(pPrivate->Setup.pszDiskList);
                    pPrivate->Setup.pszDiskList = NULL;
                }

                if (!fInList)
                    szNew[ulLogicalDrive] = 1;

                for (ul = 1;
                     ul < 27;
                     ul++)
                {
                    if (szNew[ul])
                        *pTarget++ = ul + 'A' - 1;
                }
                *pTarget = '\0';

                if (szNew2[0])
                    pPrivate->Setup.pszDiskList = strdup(szNew2);

                WinCheckMenuItem((HWND)mp2,
                                 usItem,
                                 !fInList);

                UpdateDiskMonitors(hwnd,
                                   pPrivate);

                MwgtSaveSetupAndSend(hwnd, pPrivate);

                // full repaint
                WinInvalidateRect(hwnd, NULL, FALSE);

                return (MRESULT)FALSE;
            }
        }

        return pctrDefWidgetProc(hwnd, WM_MENUSELECT, mp1, mp2);
    }

    return 0;
}

/*
 *@@ fnwpMonitorWidgets:
 *      window procedure for the various "Monitor" widget classes.
 *
 *      This window proc is shared among the "Monitor" widgets,
 *      which all have in common that they are text-only display
 *      widgets with a little bit of extra functionality.
 *
 *      Presently, the following widgets are implemented this way:
 *
 *      -- clock;
 *
 *      -- swapper monitor;
 *
 *      -- available-memory monitor.
 *
 *@@changed V0.9.14 (2001-08-01) [umoeller]: added diskfree widget
 */

MRESULT EXPENTRY fnwpMonitorWidgets(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        /*
         * WM_CREATE:
         *      as with all widgets, we receive a pointer to the
         *      XCENTERWIDGET in mp1, which was created for us.
         *
         *      The first thing the widget MUST do on WM_CREATE
         *      is to store the XCENTERWIDGET pointer (from mp1)
         *      in the QWL_USER window word by calling:
         *
         *          WinSetWindowPtr(hwnd, QWL_USER, mp1);
         *
         *      We use XCENTERWIDGET.pUser for allocating
         *      MONITORPRIVATE for our own stuff.
         */

        case WM_CREATE:
            WinSetWindowPtr(hwnd, QWL_USER, mp1);
            if (mp1)
                mrc = MwgtCreate(hwnd, (PXCENTERWIDGET)mp1);
            else
                // stop window creation!!
                mrc = (MPARAM)TRUE;
        break;

        /*
         * WM_DESTROY:
         *      clean up. This _must_ be passed on to
         *      ctrDefWidgetProc.
         */

        case WM_DESTROY:
            MwgtDestroy(hwnd);
            mrc = pctrDefWidgetProc(hwnd, msg, mp1, mp2);
        break;

        /*
         * WM_CONTROL:
         *      process notifications/queries from the XCenter.
         */

        case WM_CONTROL:
            mrc = (MPARAM)MwgtControl(hwnd, mp1, mp2);
        break;

        /*
         * WM_PAINT:
         *
         */

        case WM_PAINT:
            MwgtPaint(hwnd);
        break;

        /*
         * WM_TIMER:
         *      clock timer --> repaint.
         */

        case WM_TIMER:
            MwgtTimer(hwnd, mp1, mp2);
        break;

        /*
         * WM_WINDOWPOSCHANGED:
         *      on window resize, allocate new bitmap.
         */

        case WM_WINDOWPOSCHANGED:
            MwgtWindowPosChanged(hwnd, mp1, mp2);
        break;

        /*
         * WM_PRESPARAMCHANGED:
         *
         */

        case WM_PRESPARAMCHANGED:
            MwgtPresParamChanged(hwnd, (ULONG)mp1);
        break;

        /*
         *@@ WM_BUTTON1DBLCLK:
         *      on double-click on clock, open
         *      system clock settings.
         */

        case WM_BUTTON1DBLCLK:
            MwgtButton1DblClick(hwnd, mp1);
            mrc = (MPARAM)TRUE;     // message processed
        break;

        /*
         * WM_CONTEXTMENU:
         *      modify standard context menu behavior.
         */

        case WM_CONTEXTMENU:
            mrc = MwgtContextMenu(hwnd, mp1, mp2);
        break;

        /*
         * WM_MENUSELECT:
         *      intercept items for diskfree widget
         *      and do not dismiss.
         */

        case WM_MENUSELECT:
            mrc = MwgtMenuSelect(hwnd, mp1, mp2);
        break;

        /*
         * WM_USER:
         *      this message value is given to XWPDaemon
         *      if we're operating in diskfree mode and
         *      thus gets posted from xwpdaemon when a
         *      diskfree value changes.
         *
         *      Parameters:
         *
         *      --  ULONG mp1: ulLogicalDrive which changed.
         *
         *      --  ULONG mp2: new free space on the disk in KB.
         */

        case WM_USER:
            UpdateLogicalDrive(hwnd, mp1, mp2);
        break;

        default:
            mrc = pctrDefWidgetProc(hwnd, msg, mp1, mp2);

    } // end switch(msg)

    return mrc;
}

/* ******************************************************************
 *
 *   Exported procedures
 *
 ********************************************************************/

/*
 *@@ MwgtInitModule:
 *      required export with ordinal 1, which must tell
 *      the XCenter how many widgets this DLL provides,
 *      and give the XCenter an array of XCENTERWIDGETCLASS
 *      structures describing the widgets.
 *
 *      With this call, you are given the module handle of
 *      XFLDR.DLL. For convenience, you may resolve imports
 *      for some useful functions which are exported thru
 *      src\shared\xwp.def. See the code below.
 *
 *      This function must also register the PM window classes
 *      which are specified in the XCENTERWIDGETCLASS array
 *      entries. For this, you are given a HAB which you
 *      should pass to WinRegisterClass. For the window
 *      class style (4th param to WinRegisterClass),
 *      you should specify
 *
 +          CS_PARENTCLIP | CS_SIZEREDRAW | CS_SYNCPAINT
 *
 *      Your widget window _will_ be resized, even if you're
 *      not planning it to be.
 *
 *      This function only gets called _once_ when the widget
 *      DLL has been successfully loaded by the XCenter. If
 *      there are several instances of a widget running (in
 *      the same or in several XCenters), this function does
 *      not get called again. However, since the XCenter unloads
 *      the widget DLLs again if they are no longer referenced
 *      by any XCenter, this might get called again when the
 *      DLL is re-loaded.
 *
 *      There will ever be only one load occurence of the DLL.
 *      The XCenter manages sharing the DLL between several
 *      XCenters. As a result, it doesn't matter if the DLL
 *      has INITINSTANCE etc. set or not.
 *
 *      If this returns 0, this is considered an error, and the
 *      DLL will be unloaded again immediately.
 *
 *      If this returns any value > 0, *ppaClasses must be
 *      set to a static array (best placed in the DLL's
 *      global data) of XCENTERWIDGETCLASS structures,
 *      which must have as many entries as the return value.
 *
 */

ULONG EXPENTRY MwgtInitModule(HAB hab,         // XCenter's anchor block
                              HMODULE hmodPlugin, // module handle of the widget DLL
                              HMODULE hmodXFLDR,    // XFLDR.DLL module handle
                              PCXCENTERWIDGETCLASS *ppaClasses,
                              PSZ pszErrorMsg)  // if 0 is returned, 500 bytes of error msg
{
    ULONG   ulrc = 0,
            ul = 0;
    BOOL    fImportsFailed = FALSE;

    // resolve imports from XFLDR.DLL (this is basically
    // a copy of the doshResolveImports code, but we can't
    // use that before resolving...)
    for (ul = 0;
         ul < sizeof(G_aImports) / sizeof(G_aImports[0]); // array item count
         ul++)
    {
        if (DosQueryProcAddr(hmodXFLDR,
                             0,               // ordinal, ignored
                             (PSZ)G_aImports[ul].pcszFunctionName,
                             G_aImports[ul].ppFuncAddress)
                    != NO_ERROR)
        {
            strcpy(pszErrorMsg, "Import ");
            strcat(pszErrorMsg, G_aImports[ul].pcszFunctionName);
            strcat(pszErrorMsg, " failed.");
            fImportsFailed = TRUE;
            break;
        }
    }

    if (!fImportsFailed)
    {
        if (!WinRegisterClass(hab,
                              WNDCLASS_WIDGET_MONITORS,
                              fnwpMonitorWidgets,
                              CS_PARENTCLIP | CS_SIZEREDRAW | CS_SYNCPAINT,
                              sizeof(PMONITORPRIVATE))
                                    // extra memory to reserve for QWL_USER
                            )
            strcpy(pszErrorMsg, "WinRegisterClass failed.");
        else
        {
            // no error:
            // return classes
            *ppaClasses = G_WidgetClasses;
            // no. of classes in this DLL:
            ulrc = sizeof(G_WidgetClasses) / sizeof(G_WidgetClasses[0]);
        }
    }

    return ulrc;
}

/*
 *@@ MwgtUnInitModule:
 *      optional export with ordinal 2, which can clean
 *      up global widget class data.
 *
 *      This gets called by the XCenter right before
 *      a widget DLL gets unloaded. Note that this
 *      gets called even if the "init module" export
 *      returned 0 (meaning an error) and the DLL
 *      gets unloaded right away.
 *
 *@@added V0.9.7 (2000-12-07) [umoeller]
 */

VOID EXPENTRY MwgtUnInitModule(VOID)
{
}

/*
 *@@ MwgtQueryVersion:
 *      this new export with ordinal 3 can return the
 *      XWorkplace version number which is required
 *      for this widget to run. For example, if this
 *      returns 0.9.10, this widget will not run on
 *      earlier XWorkplace versions.
 *
 *      NOTE: This export was mainly added because the
 *      prototype for the "Init" export was changed
 *      with V0.9.9. If this returns 0.9.9, it is
 *      assumed that the INIT export understands
 *      the new FNWGTINITMODULE_099 format (see center.h).
 *
 *@@added V0.9.9 (2001-02-06) [umoeller]
 */

VOID EXPENTRY MwgtQueryVersion(PULONG pulMajor,
                               PULONG pulMinor,
                               PULONG pulRevision)
{
    *pulMajor = XFOLDER_MAJOR;              // dlgids.h
    *pulMinor = XFOLDER_MINOR;
    *pulRevision = XFOLDER_REVISION;
}

