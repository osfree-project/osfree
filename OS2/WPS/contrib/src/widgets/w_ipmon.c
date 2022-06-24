
/*
 *@@sourcefile w_ipmon.c:
 *      XCenter IP monitor widget.
 *
 *      This is all new with V0.9.19.
 *
 *      Thanks to Sergey I. Yevtushenko, author of PUMonitor,
 *      for figuring out how to use ioctl() to get the IP
 *      bandwidth.
 *
 *      This file is very similar to the other widgets,
 *      especially the "Pulse", except that the data input
 *      is different, obviously. It is however in a separate
 *      DLL so we don't force people to have networking
 *      installed to run XWorkplace because we link against
 *      SO32DLL.DLL.
 *
 *      I am not sure whether the linking requires a 32-bit
 *      version of TCP/IP to be installed. I hope not.
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 *@@header "shared\center.h"
 */

/*
 *      Copyright (C) 2002-2013 Ulrich M”ller.
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
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WININPUT
#define INCL_WINSYS
#define INCL_WINTIMER
#define INCL_WINDIALOGS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINENTRYFIELDS
#define INCL_WINPOINTERS
#define INCL_WINPROGRAMLIST
#define INCL_WINSWITCHLIST
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
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\nls.h"                // National Language Support helpers
#include "helpers\winh.h"               // PM helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\threads.h"            // thread helpers
#include "helpers\timer.h"              // replacement PM timers
#include "helpers\xstring.h"            // extended string helpers

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\errors.h"              // private XWorkplace error codes
#include "shared\helppanels.h"          // all XWorkplace help panel IDs

#include "shared\center.h"              // public XCenter interfaces
#include "config\drivdlgs.h"            // driver configuration dialogs

#pragma hdrstop                         // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Private definitions
 *
 ********************************************************************/

/*
 *  Some TCP/IP definitions copied from the Warp 4.5 toolkit.
 *  Since we are using the Warp 3 toolkit, we will have trouble
 *  getting this stuff from elsewhere.
 *
 *  Note that we use the 16-bit definitions.
 */

#ifndef IFMIB_ENTRIES
#define IFMIB_ENTRIES 42
#endif

typedef unsigned long   u_long;
VOID EXPENTRY IwgtShowSettingsDlg(PWIDGETSETTINGSDLGDATA pData);

#pragma pack(1)
struct ifmib {
  short ifNumber;
  struct iftable {
    short  ifIndex;
    char   ifDescr[45];
    short  ifType;
    short  ifMtu;
    char   ifPhysAddr[6];
    short  ifOperStatus;
    u_long ifSpeed;
    u_long ifLastChange;
    u_long ifInOctets;
    u_long ifOutOctets;
    u_long ifOutDiscards;
    u_long ifInDiscards;
    u_long ifInErrors;
    u_long ifOutErrors;
    u_long ifInUnknownProtos;
    u_long ifInUcastPkts;
    u_long ifOutUcastPkts;
    u_long ifInNUcastPkts;
    u_long ifOutNUcastPkts;
  } iftable[IFMIB_ENTRIES];
};
#pragma pack()   /* reset to default packing */

typedef char *caddr_t;

#define ioc(x,y)       ((x<<8)|y)
#define SIOSTATIF       ioc('n',48)

#define SOCK_STREAM     1               /* stream socket */
#define SOCK_DGRAM      2               /* datagram socket */
#define SOCK_RAW        3               /* raw-protocol interface */
#define SOCK_RDM        4               /* reliably-delivered message */
#define SOCK_SEQPACKET  5               /* sequenced packet stream */

#define AF_UNSPEC       0               /* unspecified */
#define AF_UNIX         1               /* local to host (pipes, portals) */
#define AF_INET         2               /* internetwork: UDP, TCP, etc. */
#define AF_IMPLINK      3               /* arpanet imp addresses */
#define AF_PUP          4               /* pup protocols: e.g. BSP */
#define AF_CHAOS        5               /* mit CHAOS protocols */
#define AF_NS           6               /* XEROX NS protocols */
#define AF_ISO          7               /* ISO protocols */
#define AF_OSI          AF_ISO          /* OSI is ISO */
#define AF_ECMA         8               /* european computer manufacturers */
#define AF_DATAKIT      9               /* datakit protocols */
#define AF_CCITT        10              /* CCITT protocols, X.25 etc */
#define AF_SNA          11              /* IBM SNA */
#define AF_DECnet       12              /* DECnet */
#define AF_DLI          13              /* Direct data link interface */
#define AF_LAT          14              /* LAT */
#define AF_HYLINK       15              /* NSC Hyperchannel */
#define AF_APPLETALK    16              /* AppleTalk */
#define AF_NETBIOS      17              /* NetBios-style addresses */

#define AF_MAX          18

#define PF_UNSPEC       AF_UNSPEC
#define PF_UNIX         AF_UNIX
#define PF_INET         AF_INET
#define PF_IMPLINK      AF_IMPLINK
#define PF_PUP          AF_PUP
#define PF_CHAOS        AF_CHAOS
#define PF_NS           AF_NS
#define PF_ISO          AF_ISO
#define PF_OSI          AF_OSI
#define PF_ECMA         AF_ECMA
#define PF_DATAKIT      AF_DATAKIT
#define PF_CCITT        AF_CCITT
#define PF_SNA          AF_SNA
#define PF_DECnet       AF_DECnet
#define PF_DLI          AF_DLI
#define PF_LAT          AF_LAT
#define PF_HYLINK       AF_HYLINK
#define PF_APPLETALK    AF_APPLETALK

#define PF_MAX          AF_MAX

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

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

#define WNDCLASS_WIDGET_IPMON    "XWPCenterIPMonWidget"

static const XCENTERWIDGETCLASS G_WidgetClasses[] =
    {
        {
            WNDCLASS_WIDGET_IPMON,
            0,
            "IPMonitor",
            (PCSZ)(XCENTER_STRING_RESOURCE | ID_CRSI_WIDGET_IPMONITOR),
                                       // widget class name displayed to user
            WGTF_SIZEABLE | WGTF_TOOLTIP,
            IwgtShowSettingsDlg        // with settings dlg
        },
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
PCMNGETSTRING pcmnGetString = NULL;
PCMNQUERYDEFAULTFONT pcmnQueryDefaultFont = NULL;
PCMNQUERYHELPLIBRARY pcmnQueryHelpLibrary = NULL;

PCTLMAKECOLORRECT pctlMakeColorRect = NULL;

PCTRDEFWIDGETPROC pctrDefWidgetProc = NULL;
PCTRFREESETUPVALUE pctrFreeSetupValue = NULL;
PCTRPARSECOLORSTRING pctrParseColorString = NULL;
PCTRSCANSETUPSTRING pctrScanSetupString = NULL;

PDOSHRESOLVEIMPORTS pdoshResolveImports = NULL; // V1.0.5 (2006-05-28) [pr]

PDRV_SPRINTF pdrv_sprintf = NULL;

PGPIHBOX pgpihBox = NULL;
PGPIHCREATEBITMAP pgpihCreateBitmap = NULL;
PGPIHCREATEMEMPS pgpihCreateMemPS = NULL;
PGPIHDRAW3DFRAME pgpihDraw3DFrame = NULL;
PGPIHSWITCHTORGB pgpihSwitchToRGB = NULL;
PGPIHCREATEXBITMAP pgpihCreateXBitmap = NULL;
PGPIHDESTROYXBITMAP pgpihDestroyXBitmap = NULL;

PNLSTHOUSANDSULONG pnlsThousandsULong = NULL;

PTMRSTARTXTIMER ptmrStartXTimer = NULL;
PTMRSTOPXTIMER ptmrStopXTimer = NULL;

PWINHFREE pwinhFree = NULL;
PWINHINSERTMENUITEM pwinhInsertMenuItem = NULL;
PWINHINSERTSUBMENU pwinhInsertSubmenu = NULL;
PWINHQUERYPRESCOLOR pwinhQueryPresColor = NULL;
PWINHSETPRESCOLOR pwinhSetPresColor = NULL; // V1.0.8 (2007-08-05) [pr]
PWINHQUERYWINDOWFONT pwinhQueryWindowFont = NULL;
PWINHSETWINDOWFONT pwinhSetWindowFont = NULL;

PDLGHCREATEDLG pdlghCreateDlg = NULL;
PWINHCENTERWINDOW pwinhCenterWindow = NULL;

PXSTRCAT pxstrcat = NULL;
PXSTRCLEAR pxstrClear = NULL;
PXSTRINIT pxstrInit = NULL;
PXSTRPRINTF pxstrPrintf = NULL;

static const RESOLVEFUNCTION G_aImports[] =
    {
        "cmnGetString", (PFN*)&pcmnGetString,
        "cmnQueryDefaultFont", (PFN*)&pcmnQueryDefaultFont,
        "cmnQueryHelpLibrary", (PFN*)&pcmnQueryHelpLibrary,
        "ctlMakeColorRect", (PFN*)&pctlMakeColorRect,
        "ctrDefWidgetProc", (PFN*)&pctrDefWidgetProc,
        "ctrFreeSetupValue", (PFN*)&pctrFreeSetupValue,
        "ctrParseColorString", (PFN*)&pctrParseColorString,
        "ctrScanSetupString", (PFN*)&pctrScanSetupString,
        "doshResolveImports", (PFN*)&pdoshResolveImports, // V1.0.5 (2006-05-28) [pr]
        "drv_sprintf", (PFN*)&pdrv_sprintf,
        "gpihBox", (PFN*)&pgpihBox,
        "gpihCreateBitmap", (PFN*)&pgpihCreateBitmap,
        "gpihCreateMemPS", (PFN*)&pgpihCreateMemPS,
        "gpihDraw3DFrame", (PFN*)&pgpihDraw3DFrame,
        "gpihSwitchToRGB", (PFN*)&pgpihSwitchToRGB,
        "gpihCreateXBitmap", (PFN*)&pgpihCreateXBitmap,
        "gpihDestroyXBitmap", (PFN*)&pgpihDestroyXBitmap,
        "nlsThousandsULong", (PFN*)&pnlsThousandsULong,
        "tmrStartXTimer", (PFN*)&ptmrStartXTimer,
        "tmrStopXTimer", (PFN*)&ptmrStopXTimer,
        "winhFree", (PFN*)&pwinhFree,
        "winhInsertMenuItem", (PFN*)&pwinhInsertMenuItem,
        "winhInsertSubmenu", (PFN*)&pwinhInsertSubmenu,
        "winhQueryPresColor", (PFN*)&pwinhQueryPresColor,
        "winhSetPresColor", (PFN*)&pwinhSetPresColor, // V1.0.8 (2007-08-05) [pr]
        "winhQueryWindowFont", (PFN*)&pwinhQueryWindowFont,
        "winhSetWindowFont", (PFN*)&pwinhSetWindowFont,

        "dlghCreateDlg", (PFN*)&pdlghCreateDlg,
        "winhCenterWindow", (PFN*)&pwinhCenterWindow,

        "xstrcat", (PFN*)&pxstrcat,
        "xstrClear", (PFN*)&pxstrClear,
        "xstrInit", (PFN*)&pxstrInit,
        "xstrPrintf", (PFN*)&pxstrPrintf
    };

// V1.0.5 (2006-05-28) [pr]: Add declarations for importing SO32DLL functions manually
typedef int _System SO32_SOCK_INIT(void);
typedef SO32_SOCK_INIT *PSO32_SOCK_INIT;
PSO32_SOCK_INIT pfn_sock_init = NULL;

typedef int _System SO32_SOCKET(int, int, int);
typedef SO32_SOCKET *PSO32_SOCKET;
PSO32_SOCKET pfn_socket = NULL;

typedef int _System SO32_SOCLOSE(int);
typedef SO32_SOCLOSE *PSO32_SOCLOSE;
PSO32_SOCLOSE pfn_soclose = NULL;

typedef int _System SO32_IOCTL(int, int, char *, int);
typedef SO32_IOCTL *PSO32_IOCTL;
PSO32_IOCTL pfn_ioctl = NULL;

static HMODULE G_hmodSO32DLL = NULLHANDLE;

static const RESOLVEFUNCTION G_aImports2[] =
    {
        "SOCK_INIT", (PFN*)&pfn_sock_init,
        "SOCKET", (PFN*)&pfn_socket,
        "SOCLOSE", (PFN*)&pfn_soclose,
        "IOCTL", (PFN*)&pfn_ioctl
    };

/* ******************************************************************
 *
 *   Private widget instance data
 *
 ********************************************************************/

/*
 *@@ MONITORSETUP:
 *      instance data to which setup strings correspond.
 *      This is also a member of WIDGETPRIVATE.
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
    LONG        lcolBackground,         // background color
                lcolForeground;         // foreground color (for text)

    LONG        lcolIn,
                lcolOut;

    PSZ         pszFont;
            // if != NULL, non-default font (in "8.Helv" format);
            // this has been allocated using local malloc()!

    LONG        cx;
            // current width; we're sizeable, and we wanna
            // store this

    ULONG       ulDevIndex;
    ULONG       ulPrevDevIndex;
            // eo:  Added to avoid extreme values upon interface change

} MONITORSETUP, *PMONITORSETUP;

/*
 *@@ SNAPSHOT:
 *
 */

typedef struct _SNAPSHOT
{
    ULONG       ulIn,
                ulOut;
} SNAPSHOT, *PSNAPSHOT;

/*
 *@@ ifIO:
 *
 *@@added V1.0.8 (2008-05-29) [pr]
 */

typedef struct _ifIO
{
    ULONG       ulInLo,
                ulInHi,
                ulOutLo,
                ulOutHi;
} ifIO, *PifIO;

/*
 *@@ WIDGETPRIVATE:
 *      more window data for the various monitor widgets.
 *
 *      An instance of this is created on WM_CREATE in
 *      fnwpMonitorWidgets and stored in XCENTERWIDGET.pUser.
 */

typedef struct _WIDGETPRIVATE
{
    PXCENTERWIDGET  pWidget;
            // reverse ptr to general widget data ptr; we need
            // that all the time and don't want to pass it on
            // the stack with each function call

    MONITORSETUP    Setup;
            // widget settings that correspond to a setup string

    int             sock;           // the one and only socked while
                                    // we exist (opened on create)

    struct ifmib    statif;         // the sick structure filled by
                                    // ioctl(SIOSTATIF)
    ifIO            IOcount[IFMIB_ENTRIES]; // 64 bit I/O counters V1.0.8 (2008-05-29) [pr]

    // Getting the socket throughput works by periodically running
    // ioctl(SIOSTATIF), which gives us the total no. of bytes
    // that went in and out per interface. By comparing that with
    // the value from the previous loop, we can get the throughput
    // rate for input and output, respectively.
    ULONG           ulPrevTotalIn,
                    ulPrevTotalOut,
                    ulPrevMillisecs;

    BOOL            fCreating;      // TRUE while in WM_CREATE (anti-recursion)

    BOOL            fContextMenuHacked;
    HWND            fContextMenuSource;
                                    // eo:  Store window handle of submenu for later updating

    ULONG           ulTimerID;      // if != NULLHANDLE, update timer is running

    PXBITMAP        pBitmap;        // bitmap for pulse graph; this contains only
                                    // the "client" (without the 3D frame)

    BOOL            fUpdateGraph;

    ULONG           cSnapshots;
    PSNAPSHOT       paSnapshots;

    ULONG           ulMax;              // maximimum in or out value in array
                                        // (for scaling)

    BOOL            fTooltipShowing;    // TRUE only while tooltip is currently
                                        // showing over this widget

    PSZ             pszTooltipFormat;   // NLS tooltip format string
    XSTRING         strTooltipText;     // current tooltip text

} WIDGETPRIVATE, *PWIDGETPRIVATE;


/*
 *      This section contains shared code to manage the
 *      widget's settings. This can translate a widget
 *      setup string into the fields of a binary setup
 *      structure and vice versa. This code is used by
 *      both an open widget window and a settings dialog.
 */

/*
 *@@ IwgtFreeSetup:
 *      cleans up the data in the specified setup
 *      structure, but does not free the structure
 *      itself.
 */

VOID IwgtFreeSetup(PMONITORSETUP pSetup)
{
    if (pSetup)
    {
        if (pSetup->pszFont)
        {
            free(pSetup->pszFont);
            pSetup->pszFont = NULL;
        }
    }
}

/*
 *@@ IwgtScanSetup:
 *      scans the given setup string and translates
 *      its data into the specified binary setup
 *      structure.
 *
 *      NOTE: It is assumed that pSetup is zeroed
 *      out. We do not clean up previous data here.
 *
 */

VOID IwgtScanSetup(PCSZ pcszSetupString,
                   PMONITORSETUP pSetup)
{
    PSZ p;

    // width
    if (p = pctrScanSetupString(pcszSetupString,
                                "WIDTH"))
    {
        pSetup->cx = atoi(p);
        pctrFreeSetupValue(p);
    }
    else
        pSetup->cx = 100;

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
        pSetup->lcolForeground = WinQuerySysColor(HWND_DESKTOP, SYSCLR_WINDOWTEXT, 0);
                // defaults to black

    // font:
    // we set the font presparam, which automatically
    // affects the cached presentation spaces
    if (p = pctrScanSetupString(pcszSetupString,
                                "FONT"))
    {
        pSetup->pszFont = strdup(p);
        pctrFreeSetupValue(p);
    }
    else
        pSetup->pszFont = strdup("9.WarpSans");

    if (p = pctrScanSetupString(pcszSetupString,
                                "SOURCE"))
    {
        pSetup->ulDevIndex = atoi(p);
        pctrFreeSetupValue(p);
    }

    // In-graph color
    if (p = pctrScanSetupString(pcszSetupString,
                                "INGCOL"))
    {
        pSetup->lcolIn = pctrParseColorString(p);
        pctrFreeSetupValue(p);
    }
    else
        // default color:
        pSetup->lcolIn = RGBCOL_DARKGREEN;

    // Out-graph color:
    if (p = pctrScanSetupString(pcszSetupString,
                                "OUTGCOL"))
    {
        pSetup->lcolOut = pctrParseColorString(p);
        pctrFreeSetupValue(p);
    }
    else
        pSetup->lcolOut = RGBCOL_RED;
}

/*
 *@@ IwgtSaveSetup:
 *      composes a new setup string.
 *      The caller must invoke xstrClear on the
 *      string after use.
 */

VOID IwgtSaveSetup(PXSTRING pstrSetup,       // out: setup string (is cleared first)
                   PMONITORSETUP pSetup)
{
    CHAR    szTemp[100];
    pxstrInit(pstrSetup, 100);

    pdrv_sprintf(szTemp, "WIDTH=%d;",
            pSetup->cx);
    pxstrcat(pstrSetup, szTemp, 0);

    pdrv_sprintf(szTemp, "BGNDCOL=%06lX;",
            pSetup->lcolBackground);
    pxstrcat(pstrSetup, szTemp, 0);

    pdrv_sprintf(szTemp, "TEXTCOL=%06lX;",
            pSetup->lcolForeground);
    pxstrcat(pstrSetup, szTemp, 0);

    pdrv_sprintf(szTemp, "INGCOL=%06lX;",
            pSetup->lcolIn);
    pxstrcat(pstrSetup, szTemp, 0);

    pdrv_sprintf(szTemp, "OUTGCOL=%06lX;",
            pSetup->lcolOut);
    pxstrcat(pstrSetup, szTemp, 0);

    if (pSetup->pszFont)
    {
        // non-default font:
        pdrv_sprintf(szTemp, "FONT=%s;",
                pSetup->pszFont);
        pxstrcat(pstrSetup, szTemp, 0);
    }

    pdrv_sprintf(szTemp, "SOURCE=%d;",
                 pSetup->ulDevIndex);
    pxstrcat(pstrSetup, szTemp, 0);
}

/*
 *@@ IwgtSaveSetupAndSend:
 *
 */

VOID IwgtSaveSetupAndSend(HWND hwnd,
                          PMONITORSETUP pSetup)
{
    XSTRING strSetup;
    IwgtSaveSetup(&strSetup,
                  pSetup);
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

#define INDEX_BACKGROUND        1000
#define INDEX_FOREGROUND        1001
#define INDEX_GRAPHIN           1002
#define INDEX_GRAPHOUT          1003

/*
 *@@ SubclassAndSetColor:
 *
 */

STATIC VOID SubclassAndSetColor(HWND hwndDlg,
                                ULONG ulID,
                                PCSZ pcszTitle,
                                LONG lColor,
                                LONG lBackColor)
{
    HWND hwnd;
    if (hwnd = WinWindowFromID(hwndDlg, ulID))
    {
        WinSetWindowText(hwnd,
                         (PSZ)pcszTitle);
        WinSetPresParam(hwnd,
                        PP_BACKGROUNDCOLOR,
                        sizeof(LONG),
                        &lColor);
        if (ulID == 1000 + INDEX_FOREGROUND)
            WinSetPresParam(hwnd,
                            PP_FOREGROUNDCOLOR,
                            sizeof(LONG),
                            &lBackColor);

        pctlMakeColorRect(hwnd);
    }
}

/*
 *@@ GetColor:
 *
 */

STATIC LONG GetColor(HWND hwndDlg,
                     ULONG ulID)
{
    return pwinhQueryPresColor(WinWindowFromID(hwndDlg, ulID),
                               PP_BACKGROUNDCOLOR,
                               FALSE,
                               SYSCLR_DIALOGBACKGROUND);
}

#define COLOR_WIDTH     60
#define COLOR_HEIGHT    16

static CONTROLDEF
    OKButton = CONTROLDEF_DEFPUSHBUTTON(NULL, DID_OK, STD_BUTTON_WIDTH, STD_BUTTON_HEIGHT),
    CancelButton = CONTROLDEF_PUSHBUTTON(NULL, DID_CANCEL, STD_BUTTON_WIDTH, STD_BUTTON_HEIGHT),

    GraphsGroup = CONTROLDEF_GROUP(
                            NULL,
                            -1,
                            -1,
                            -1),

    GraphInColor
                = CONTROLDEF_TEXT(NULL,
                                  1000 + INDEX_GRAPHIN,
                                  COLOR_WIDTH,
                                  COLOR_HEIGHT),

    GraphOutColor
                = CONTROLDEF_TEXT(NULL,
                                  1000 + INDEX_GRAPHOUT,
                                  COLOR_WIDTH,
                                  COLOR_HEIGHT),


    OthersGroup = CONTROLDEF_GROUP(
                            NULL,
                            -1,
                            -1,
                            -1),

    BackgroundColor
                = CONTROLDEF_TEXT(NULL,
                                  1000 + INDEX_BACKGROUND,
                                  COLOR_WIDTH,
                                  COLOR_HEIGHT),
    ForegroundColor
                = CONTROLDEF_TEXT(NULL,
                                  1000 + INDEX_FOREGROUND,
                                  COLOR_WIDTH,
                                  COLOR_HEIGHT);

static const DLGHITEM
    dlgIpmon[] =
    {
        START_TABLE,
            START_ROW(0),
                START_GROUP_TABLE(&GraphsGroup),
                    START_ROW(0),
                        CONTROL_DEF(&GraphInColor),
                    START_ROW(0),
                        CONTROL_DEF(&GraphOutColor),
                END_TABLE,
                START_GROUP_TABLE(&OthersGroup),
                    START_ROW(0),
                        CONTROL_DEF(&BackgroundColor),
                    START_ROW(0),
                        CONTROL_DEF(&ForegroundColor),
                END_TABLE,
            START_ROW(0),
                CONTROL_DEF(&OKButton),
                CONTROL_DEF(&CancelButton),
        END_TABLE
    };

/*
 *@@ IwgtShowSettingsDlg:
 *
 *@@added V0.9.19 (2002-06-02) [umoeller]
 *@@changed V0.9.20 (2002-07-06) [umoeller]: fixed wrong title
 *@@changed V1.0.8 (2007-08-05) [pr]: now setting Presparams @@fixes 994
 */

VOID EXPENTRY IwgtShowSettingsDlg(PWIDGETSETTINGSDLGDATA pData)
{
    HWND hwndDlg = NULLHANDLE;
    APIRET arc;

    ULONG       ul;

    GraphsGroup.pcszText = pcmnGetString(ID_CRSI_IPWGT_GRAPHCOLORS);
    OthersGroup.pcszText = pcmnGetString(ID_CRSI_PWGT_OTHERCOLORS);

    OKButton.pcszText = pcmnGetString(DID_OK);
    CancelButton.pcszText = pcmnGetString(DID_CANCEL);

    if (!(arc = pdlghCreateDlg(&hwndDlg,
                               pData->hwndOwner,
                               FCF_FIXED_DLG,
                               WinDefDlgProc,
                               pcmnGetString(ID_CRSI_WIDGET_IPMONITOR),
                                    // fixed wrong title ID_CRSI_PWGT_TITLE
                                    // V0.9.20 (2002-07-06) [umoeller]
                               dlgIpmon,
                               ARRAYITEMCOUNT(dlgIpmon),
                               NULL,
                               pcmnQueryDefaultFont())))
    {
        // go scan the setup string
        MONITORSETUP  Setup;
        IwgtScanSetup(pData->pcszSetupString,
                      &Setup);

        // for each color control, set the background color
        // according to the settings

        SubclassAndSetColor(hwndDlg,
                            1000 + INDEX_GRAPHIN,
                            pcmnGetString(ID_CRSI_IPWGT_GRAPHINCOLOR),
                            Setup.lcolIn,
                            Setup.lcolBackground);

        SubclassAndSetColor(hwndDlg,
                            1000 + INDEX_GRAPHOUT,
                            pcmnGetString(ID_CRSI_IPWGT_GRAPHOUTCOLOR),
                            Setup.lcolOut,
                            Setup.lcolBackground);

        SubclassAndSetColor(hwndDlg,
                            1000 + INDEX_BACKGROUND,
                            pcmnGetString(ID_CRSI_PWGT_BACKGROUNDCOLOR), // "Background",
                            Setup.lcolBackground,
                            Setup.lcolForeground);
        SubclassAndSetColor(hwndDlg,
                            1000 + INDEX_FOREGROUND,
                            pcmnGetString(ID_CRSI_PWGT_TEXTCOLOR), // "Text",
                            Setup.lcolForeground,
                            Setup.lcolBackground);

        // go!
        pwinhCenterWindow(hwndDlg);
        if (DID_OK == WinProcessDlg(hwndDlg))
        {
            XSTRING strSetup;

            // get the colors back from the controls
            Setup.lcolIn = GetColor(hwndDlg,
                                    1000 + INDEX_GRAPHIN);

            Setup.lcolOut = GetColor(hwndDlg,
                                     1000 + INDEX_GRAPHOUT);

            Setup.lcolBackground = GetColor(hwndDlg,
                                            1000 + INDEX_BACKGROUND);
            // V1.0.8 (2007-08-05) [pr]
            pwinhSetPresColor(pData->pView->hwndWidget,
                              PP_BACKGROUNDCOLOR,
                              Setup.lcolBackground);

            Setup.lcolForeground = GetColor(hwndDlg,
                                            1000 + INDEX_FOREGROUND);
            // V1.0.8 (2007-08-05) [pr]
            pwinhSetPresColor(pData->pView->hwndWidget,
                              PP_FOREGROUNDCOLOR,
                              Setup.lcolForeground);

            IwgtSaveSetup(&strSetup,
                          &Setup);

            pData->pctrSetSetupString(pData->hSettings,
                                      strSetup.psz);
            pxstrClear(&strSetup);
        }

        WinDestroyWindow(hwndDlg);
    }
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
 *@@ IwgtCreate:
 *      implementation for WM_CREATE.
 *
 *@@changed V1.0.8 (2007-08-05) [pr]: now setting Presparams @@fixes 994
 *@@changed V1.0.8 (2008-05-29) [pr]: make traffic byte count into KB @@fixes 536
 */

STATIC MRESULT IwgtCreate(HWND hwnd,
                          PXCENTERWIDGET pWidget)
{
    MRESULT mrc = 0;        // continue window creation
    CHAR    szTemp[200];

    PWIDGETPRIVATE pPrivate = malloc(sizeof(WIDGETPRIVATE));
    memset(pPrivate, 0, sizeof(WIDGETPRIVATE));
    // link the two together
    pWidget->pUser = pPrivate;
    pPrivate->pWidget = pWidget;

    pPrivate->fCreating = TRUE;

    pxstrInit(&pPrivate->strTooltipText, 0);

    // compose tooltip format string... something like
    // "%s\nin: %lu%c%lu KB/s (%s KB total)\nout: %lu%c%lu KB/s (%s KB total)"
    // V1.0.8 (2008-05-29)
    sprintf(szTemp, "%%s\n%s: %%lu%%c%%lu KB/s (%%s KB %s)\n"
                         "%s: %%lu%%c%%lu KB/s (%%s KB %s)",
           pcmnGetString(ID_CRSI_IPWGT_GRAPHINCOLOR),       // "Incoming"
           pcmnGetString(ID_CRSI_IPWGT_TOTAL),              // "total"
           pcmnGetString(ID_CRSI_IPWGT_GRAPHOUTCOLOR),      // "Outgoing"
           pcmnGetString(ID_CRSI_IPWGT_TOTAL));             // "total"
    pPrivate->pszTooltipFormat = strdup(szTemp);

    // initialize binary setup structure from setup string
    IwgtScanSetup(pWidget->pcszSetupString,
                  &pPrivate->Setup);

    pfn_sock_init();

    pPrivate->ulMax = 1;        // avoid division by zero

    // create socket for while the widget is running
    pPrivate->sock = pfn_socket(PF_INET, SOCK_RAW, 0);  // V1.0.10 (2013-06-01)

    if (pPrivate->sock > 0)
    {
        // socket created OK: get first shot of data
        // so we can do calculations from now on
        if (!pfn_ioctl(pPrivate->sock,
                       SIOSTATIF,
                       (caddr_t)&pPrivate->statif,
                       sizeof(pPrivate->statif)))
        {
            // now update "latest" with the data of the
            // selected device
            ULONG i;

            // V1.0.8 (2008-05-29)
            for (i = 0; i < IFMIB_ENTRIES; i++)
            {
                pPrivate->IOcount[i].ulInLo = pPrivate->statif.iftable[i].ifInOctets;
                pPrivate->IOcount[i].ulOutLo = pPrivate->statif.iftable[i].ifOutOctets;
            }

            if (pPrivate->Setup.ulDevIndex >= IFMIB_ENTRIES)
                pPrivate->Setup.ulDevIndex = 0;

            i = pPrivate->Setup.ulDevIndex;

            pPrivate->ulPrevTotalIn = pPrivate->statif.iftable[i].ifInOctets;
            pPrivate->ulPrevTotalOut = pPrivate->statif.iftable[i].ifOutOctets;

            DosQuerySysInfo(QSV_MS_COUNT,
                            QSV_MS_COUNT,
                            (PVOID)&pPrivate->ulPrevMillisecs,
                            sizeof(pPrivate->ulPrevMillisecs));
        }
    }

    // set window font (this affects all the cached presentation
    // spaces we use)
    pwinhSetWindowFont(hwnd,
                       pPrivate->Setup.pszFont);
    // V1.0.8 (2007-08-05) [pr]
    pwinhSetPresColor(hwnd,
                      PP_BACKGROUNDCOLOR,
                      pPrivate->Setup.lcolBackground);
    pwinhSetPresColor(hwnd,
                      PP_FOREGROUNDCOLOR,
                      pPrivate->Setup.lcolForeground);

    // enable context menu help
    pWidget->pcszHelpLibrary = pcmnQueryHelpLibrary();
    pWidget->ulHelpPanelID = ID_XSH_WIDGET_IPMON_MAIN;

    // start update timer
    pPrivate->ulTimerID = ptmrStartXTimer(pWidget->pGlobals->pvXTimerSet,
                                         hwnd,
                                         1,
                                         1000);

    pPrivate->fCreating = FALSE;

    return mrc;
}

/*
 *@@ IwgtDestroy:
 *
 *@@added V0.9.19 (2002-06-08) [umoeller]
 */

STATIC VOID IwgtDestroy(HWND hwnd)
{
    PXCENTERWIDGET pWidget;
    PWIDGETPRIVATE pPrivate;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PWIDGETPRIVATE)pWidget->pUser)
       )
    {
        if (pPrivate->ulTimerID)
            ptmrStopXTimer(pPrivate->pWidget->pGlobals->pvXTimerSet,
                           hwnd,
                           pPrivate->ulTimerID);

        if (pPrivate->pBitmap)
            pgpihDestroyXBitmap(&pPrivate->pBitmap);
                    // this was missing V0.9.12 (2001-05-20) [umoeller]

        if (pPrivate->paSnapshots)
            free(pPrivate->paSnapshots);

        if (pPrivate->sock > 0)  // V1.0.10 (2013-06-01)
            pfn_soclose(pPrivate->sock);

        pxstrClear(&pPrivate->strTooltipText);
        free(pPrivate->pszTooltipFormat);

        free(pPrivate);
    } // end if (pPrivate)
}

/*
 *@@ IwgtControl:
 *      implementation for WM_CONTROL.
 *
 *@@changed V1.0.8 (2007-08-05) [pr]: now setting Presparams on XN_SETUPCHANGED @@fixes 994
 */

STATIC BOOL IwgtControl(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    BOOL brc = FALSE;

    PXCENTERWIDGET pWidget;
    PWIDGETPRIVATE pPrivate;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PWIDGETPRIVATE)pWidget->pUser)
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
                        pszl->cx = pPrivate->Setup.cx;
                        pszl->cy = 10;
                        brc = TRUE;
                    }
                    break;

                    /*
                     * XN_SETUPCHANGED:
                     *      XCenter has a new setup string for
                     *      us in mp2.
                     */

                    case XN_SETUPCHANGED:
                    {
                        const char *pcszNewSetupString = (const char*)mp2;

                        // reinitialize the setup data
                        IwgtFreeSetup(&pPrivate->Setup);
                        IwgtScanSetup(pcszNewSetupString,
                                      &pPrivate->Setup);
                        // V1.0.8 (2007-08-05) [pr]
                        pwinhSetWindowFont(pWidget->hwndWidget,
                                           (pPrivate->Setup.pszFont)
                                            ? pPrivate->Setup.pszFont
                                            // default font: use the same as in the rest of XWorkplace:
                                            : pcmnQueryDefaultFont());
                        pwinhSetPresColor(pWidget->hwndWidget,
                                          PP_BACKGROUNDCOLOR,
                                          pPrivate->Setup.lcolBackground);
                        pwinhSetPresColor(pWidget->hwndWidget,
                                          PP_FOREGROUNDCOLOR,
                                          pPrivate->Setup.lcolForeground);

                        WinInvalidateRect(pWidget->hwndWidget, NULL, FALSE);
                    }
                    break;
                }
            }
            break;

            case ID_XCENTER_TOOLTIP:
            {
                switch (usNotifyCode)
                {
                    case TTN_NEEDTEXT:
                    {
                        PTOOLTIPTEXT pttt = (PTOOLTIPTEXT)mp2;
                        pttt->pszText = pPrivate->strTooltipText.psz;
                        pttt->ulFormat = TTFMT_PSZ;
                    }
                    break;

                    case TTN_SHOW:
                        pPrivate->fTooltipShowing = TRUE;
                    break;

                    case TTN_POP:
                        pPrivate->fTooltipShowing = FALSE;
                    break;
                }
            }
        } // end if (pPrivate)
    } // end if (pWidget)

    return brc;
}

/*
 *@@ IwgtUpdateGraph:
 *      updates the graph bitmap. This does not paint
 *      on the screen.
 *
 *      Preconditions:
 *      --  pPrivate->hbmGraph must be selected into
 *          pPrivate->hpsMem.
 *
 *@@changed V1.0.8 (2008-05-29) [pr]: make traffic byte count into KB @@fixes 536
 */

STATIC VOID IwgtUpdateGraph(HWND hwnd,
                            PWIDGETPRIVATE pPrivate)
{
    PXCENTERWIDGET pWidget = pPrivate->pWidget;
    RECTL   rclBmp;

    // size for bitmap: same as widget, except
    // for the border
    WinQueryWindowRect(hwnd, &rclBmp);
    rclBmp.xRight -= 2;
    rclBmp.yTop -= 2;

    if (    (pPrivate->pBitmap)
            // bitmap needs to be created:
         || (pPrivate->pBitmap = pgpihCreateXBitmap(pWidget->habWidget,
                                                    rclBmp.xRight,
                                                    rclBmp.yTop))
       )
    {
        HPS     hpsMem = pPrivate->pBitmap->hpsMem;
        POINTL  ptl;

        GpiSetColor(hpsMem, pPrivate->Setup.lcolBackground);

        pgpihBox(hpsMem,
                 DRO_FILL,
                 &rclBmp);

        if (pPrivate->paSnapshots)
        {
            PSNAPSHOT   pLatest = &pPrivate->paSnapshots[pPrivate->cSnapshots - 1];
            PCOUNTRYSETTINGS
                        pCountrySettings = (PCOUNTRYSETTINGS)pWidget->pGlobals->pCountrySettings;
            ULONG       ulIn = pLatest->ulIn * 10 / 1024,
                        ulOut = pLatest->ulOut * 10 / 1024,
                        ulDevice = pPrivate->Setup.ulDevIndex;
            CHAR        szTemp1[50],
                        szTemp2[50];

            // paint input graph
            GpiSetColor(hpsMem, pPrivate->Setup.lcolIn);

            ptl.x = 0;
            ptl.y =    pPrivate->paSnapshots[0].ulIn
                     * rclBmp.yTop
                     / pPrivate->ulMax;
            GpiMove(hpsMem,
                    &ptl);

            for (ptl.x = 1;
                 ptl.x < pPrivate->cSnapshots;
                 ++ptl.x)
            {
                ptl.y =  pPrivate->paSnapshots[ptl.x].ulIn
                         * rclBmp.yTop
                         / pPrivate->ulMax;
                GpiLine(hpsMem,
                        &ptl);
            }

            // paint output graph
            GpiSetColor(hpsMem, pPrivate->Setup.lcolOut);

            ptl.x = 0;
            ptl.y =    pPrivate->paSnapshots[0].ulOut
                     * rclBmp.yTop
                     / pPrivate->ulMax;
            GpiMove(hpsMem,
                    &ptl);

            for (ptl.x = 1;
                 ptl.x < pPrivate->cSnapshots;
                 ++ptl.x)
            {
                ptl.y =    pPrivate->paSnapshots[ptl.x].ulOut
                         * rclBmp.yTop
                         / pPrivate->ulMax;
                GpiLine(hpsMem,
                        &ptl);
            }

            // recompose tooltip string
            pxstrPrintf(&pPrivate->strTooltipText,
                        pPrivate->pszTooltipFormat,
                                // NLS format string from WM_CREATE:
                                // "%s\nin: %lu%c%lu KB/s (%s KB total)\nout: %lu%c%lu KB/s (%s KB total)"
                        // interface name:
                        pPrivate->statif.iftable[ulDevice].ifDescr,
                        // in kb/s:
                        ulIn / 10,
                        pCountrySettings->cDecimal,
                        ulIn % 10,
                        // in total:
                        pnlsThousandsULong(szTemp1,
                                           // V1.0.8 (2008-05-29)
                                           (pPrivate->IOcount[ulDevice].ulInLo >> 10) +
                                           (pPrivate->IOcount[ulDevice].ulInHi << 22),
                                           pCountrySettings->cThousands),
                        // out kb/s:
                        ulOut / 10,
                        pCountrySettings->cDecimal,
                        ulOut % 10,
                        // out total:
                        pnlsThousandsULong(szTemp2,
                                           // V1.0.8 (2008-05-29)
                                           (pPrivate->IOcount[ulDevice].ulOutLo >> 10) +
                                           (pPrivate->IOcount[ulDevice].ulOutHi << 22),
                                           pCountrySettings->cThousands));
        }
    }

    if (pPrivate->fTooltipShowing)
        // tooltip currently showing:
        // refresh its display
        WinSendMsg(pWidget->pGlobals->hwndTooltip,
                   TTM_UPDATETIPTEXT,
                   (MPARAM)pPrivate->strTooltipText.psz,
                   0);

    pPrivate->fUpdateGraph = FALSE;
}

/*
 * IwgtPaint2:
 *      this does the actual painting of the frame (if
 *      fDrawFrame==TRUE) and the pulse bitmap.
 *
 *      Gets called by IwgtPaint.
 *
 *      The specified HPS is switched to RGB mode before
 *      painting.
 *
 *      If DosPerfSysCall succeeds, this diplays the pulse.
 *      Otherwise an error msg is displayed.
 */

STATIC VOID IwgtPaint2(HWND hwnd,
                       PWIDGETPRIVATE pPrivate,
                       HPS hps,
                       BOOL fDrawFrame)     // in: if TRUE, everything is painted
{
    PXCENTERWIDGET pWidget = pPrivate->pWidget;
    PMONITORSETUP pSetup = &pPrivate->Setup;
    RECTL       rclWin;
    ULONG       ulBorder = 1;
    CHAR        szTemp[200];

    // now paint button frame
    WinQueryWindowRect(hwnd,
                       &rclWin);        // exclusive
    pgpihSwitchToRGB(hps);

    rclWin.xRight--;
    rclWin.yTop--;
        // rclWin is now inclusive

    if (fDrawFrame)
    {
        LONG lDark, lLight;

        if (pPrivate->pWidget->pGlobals->flDisplayStyle & XCS_SUNKBORDERS)
        {
            lDark = pWidget->pGlobals->lcol3DDark;
            lLight = pWidget->pGlobals->lcol3DLight;
        }
        else
        {
            lDark =
            lLight = pPrivate->Setup.lcolBackground;
        }

        pgpihDraw3DFrame(hps,
                         &rclWin,        // inclusive
                         ulBorder,
                         lDark,
                         lLight);
    }

    if (pPrivate->fUpdateGraph)
        // graph bitmap needs to be updated:
        IwgtUpdateGraph(hwnd, pPrivate);

    if (pPrivate->pBitmap)
    {
        POINTL      ptlBmpDest;
        ptlBmpDest.x = rclWin.xLeft + ulBorder;
        ptlBmpDest.y = rclWin.yBottom + ulBorder;
        // now paint graph from bitmap
        WinDrawBitmap(hps,
                      pPrivate->pBitmap->hbm,
                      NULL,     // entire bitmap
                      &ptlBmpDest,
                      0, 0,
                      DBM_NORMAL);

        if (pPrivate->paSnapshots)
        {
            PSNAPSHOT   pLatest = &pPrivate->paSnapshots[pPrivate->cSnapshots - 1];
            PCOUNTRYSETTINGS
                        pCountrySettings = (PCOUNTRYSETTINGS)pWidget->pGlobals->pCountrySettings;
            ULONG       ulIn = pLatest->ulIn * 10 / 1024,
                        ulOut = pLatest->ulOut * 10 / 1024,
                        ul;

            // text to print above bitmap
            ul = pdrv_sprintf(szTemp,
                              "%lu%c%lu | %lu%c%lu",
                              ulIn / 10,
                              pCountrySettings->cDecimal,
                              ulIn % 10,
                              ulOut / 10,
                              pCountrySettings->cDecimal,
                              ulOut % 10);

            GpiSetColor(hps, pPrivate->Setup.lcolForeground);
            WinDrawText(hps,
                        ul,
                        szTemp,
                        &rclWin,
                        0,
                        0,
                        DT_CENTER | DT_VCENTER | DT_TEXTATTRS);
        }
    }
}

/*
 *@@ IwgtPaint:
 *      implementation for WM_PAINT.
 */

STATIC VOID IwgtPaint(HWND hwnd)
{
    HPS hps;
    if (hps = WinBeginPaint(hwnd, NULLHANDLE, NULL))
    {
        // get widget data and its button data from QWL_USER
        PXCENTERWIDGET pWidget;
        PWIDGETPRIVATE pPrivate;
        if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
             && (pPrivate = (PWIDGETPRIVATE)pWidget->pUser)
           )
        {
            IwgtPaint2(hwnd,
                       pPrivate,
                       hps,
                       TRUE);        // draw frame
        } // end if (pPrivate)

        WinEndPaint(hps);

    } // end if (hps)
}

/*
 *@@ GetSnapshot:
 *      updates the newest entry in the snapshots
 *      array with the current IP values.
 *
 *@@changed V1.0.8 (2008-05-29) [pr]: make traffic byte count into KB @@fixes 536
 *@@changed V1.0.8 (2008-05-29) [pr]: auto scale maximum reading @@fixes 921
 *@@changed V1.0.10 (2013-06-01) [pr]: add resilience to socket/ioctl failure
 */

STATIC VOID GetSnapshot(PWIDGETPRIVATE pPrivate)
{
    if (pPrivate->sock <= 0)
        pPrivate->sock = pfn_socket(PF_INET, SOCK_RAW, 0);

    if (pPrivate->sock > 0)
    {
        int rc = pfn_ioctl(pPrivate->sock,
                           SIOSTATIF,
                           (caddr_t)&pPrivate->statif,
                           sizeof(pPrivate->statif));

        if (rc)
        {
            pfn_soclose(pPrivate->sock);
            pPrivate->sock = pfn_socket(PF_INET, SOCK_RAW, 0);
            rc = pfn_ioctl(pPrivate->sock,
                           SIOSTATIF,
                           (caddr_t)&pPrivate->statif,
                           sizeof(pPrivate->statif));
        }

        if (!rc)
        {
            ULONG       ulMillisecs,
                        ulDivisor,
                        ulIndex,
                        ulIn,
                        ulOut,
                        i;
            PSNAPSHOT   pLatest = &pPrivate->paSnapshots[pPrivate->cSnapshots - 1];

            // V1.0.8 (2008-05-29)
            for (i = 0; i < IFMIB_ENTRIES; i++)
            {
                ulIn = pPrivate->statif.iftable[i].ifInOctets;
                ulOut = pPrivate->statif.iftable[i].ifOutOctets;
                if (ulIn < pPrivate->IOcount[i].ulInLo)
                    pPrivate->IOcount[i].ulInHi++;

                pPrivate->IOcount[i].ulInLo = ulIn;
                if (ulOut < pPrivate->IOcount[i].ulOutLo)
                    pPrivate->IOcount[i].ulOutHi++;

                pPrivate->IOcount[i].ulOutLo = ulOut;
            }

            DosQuerySysInfo(QSV_MS_COUNT,
                            QSV_MS_COUNT,
                            (PVOID)&ulMillisecs,
                            sizeof(ULONG));

            if (!(ulDivisor = ulMillisecs - pPrivate->ulPrevMillisecs))
                // avoid div by zero
                ulDivisor = 1;

            pPrivate->ulPrevMillisecs = ulMillisecs;

            // do not crash the array
            if (pPrivate->Setup.ulDevIndex >= IFMIB_ENTRIES)
                pPrivate->Setup.ulDevIndex = 0;

            ulIndex = pPrivate->Setup.ulDevIndex;
            ulIn = pPrivate->statif.iftable[ulIndex].ifInOctets;
            ulOut = pPrivate->statif.iftable[ulIndex].ifOutOctets;

            // eo: Check if interface has changed, in that case update Prev* values.
            if (ulIndex != pPrivate->Setup.ulPrevDevIndex)
            {
                pPrivate->ulPrevTotalIn  = ulIn;
                pPrivate->ulPrevTotalOut = ulOut;
                pPrivate->Setup.ulPrevDevIndex = ulIndex;
            }

            // now update "latest" with the data of the
            // selected device; the point is, we get the
            // current bandwidth by checking how much time
            // has elapsed and then subtracting the old
            // total bytes value from the new one

            // 1) input bytes
            if (ulIn < pPrivate->ulPrevTotalIn)
                // eo:  If octet count has wrapped around (ULONG), compensate.
                pLatest->ulIn =   (   (double)ulIn
                                    + (0xffffffff - pPrivate->ulPrevTotalIn)
                                  ) * 1000
                                  / ulDivisor;
            else
                pLatest->ulIn =   (   (double)ulIn
                                    - pPrivate->ulPrevTotalIn
                                  ) * 1000
                                  / ulDivisor;

            pPrivate->ulPrevTotalIn = ulIn;

            // 2) output bytes
            if (ulOut < pPrivate->ulPrevTotalOut)
                // eo:  If octet count has wrapped around (ULONG), compensate.
                pLatest->ulOut =   (   (double)ulOut
                                     + (0xffffffff - pPrivate->ulPrevTotalOut)
                                   ) * 1000
                                   / ulDivisor;
            else
                pLatest->ulOut =   (   (double)ulOut
                                     - pPrivate->ulPrevTotalOut
                                   ) * 1000
                                   / ulDivisor;

            pPrivate->ulPrevTotalOut = ulOut;
            // V1.0.8 (2008-05-29)
            pPrivate->ulMax = 1;
            for (i = 0; i < pPrivate->cSnapshots; i++)
            {
                if (pPrivate->paSnapshots[i].ulIn > pPrivate->ulMax)
                    pPrivate->ulMax = pPrivate->paSnapshots[i].ulIn;

                if (pPrivate->paSnapshots[i].ulOut > pPrivate->ulMax)
                    pPrivate->ulMax = pPrivate->paSnapshots[i].ulOut;
            }
        }
    }
}

/*
 *@@ IwgtTimer:
 *      updates the snapshots array, updates the
 *      graph bitmap, and invalidates the window.
 */

STATIC VOID IwgtTimer(HWND hwnd)
{
    PXCENTERWIDGET pWidget;
    PWIDGETPRIVATE pPrivate;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PWIDGETPRIVATE)pWidget->pUser)
       )
    {
        HPS hps;
        RECTL rclClient;
        WinQueryWindowRect(hwnd, &rclClient);
        if (rclClient.xRight)
        {
            ULONG ulGraphCX = rclClient.xRight - 2;    // minus border

            if (!pPrivate->paSnapshots)
            {
                // create array of loads
                ULONG cb = sizeof(SNAPSHOT) * ulGraphCX;
                pPrivate->cSnapshots = ulGraphCX;
                pPrivate->paSnapshots = (PSNAPSHOT)malloc(cb);
                memset(pPrivate->paSnapshots, 0, cb);
            }

            if (pPrivate->paSnapshots)
            {
                // in the array of loads, move each entry one to the front;
                // drop the oldest entry
                memmove(&pPrivate->paSnapshots[0],
                        &pPrivate->paSnapshots[1],
                        sizeof(SNAPSHOT) * (pPrivate->cSnapshots - 1));

                // and update the last entry with the current value
                GetSnapshot(pPrivate);

                // update display
                pPrivate->fUpdateGraph = TRUE;

                hps = WinGetPS(hwnd);
                IwgtPaint2(hwnd,
                           pPrivate,
                           hps,
                           FALSE);       // do not draw frame
                WinReleasePS(hps);
            }
        } // end if (rclClient.xRight)
    } // end if (pPrivate)
}

/*
 *@@ IwgtWindowPosChanged:
 *      implementation for WM_WINDOWPOSCHANGED.
 *
 */

STATIC VOID IwgtWindowPosChanged(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    PXCENTERWIDGET pWidget;
    PWIDGETPRIVATE pPrivate;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PWIDGETPRIVATE)pWidget->pUser)
       )
    {
        PSWP pswpNew = (PSWP)mp1,
             pswpOld = pswpNew + 1;
        if (pswpNew->fl & SWP_SIZE)
        {
            // window was resized:

            // destroy the buffer bitmap because we
            // need a new one with a different size
            if (pPrivate->pBitmap)
                pgpihDestroyXBitmap(&pPrivate->pBitmap);

            if (pswpNew->cx != pswpOld->cx)
            {
                // width changed:
                if (pPrivate->paSnapshots)
                {
                    // we also need a new array of past loads
                    // since the array is cx items wide...
                    // so reallocate the array, but keep past
                    // values
                    ULONG ulNewClientCX = pswpNew->cx - 2;
                    PSNAPSHOT paNewSnapshots =
                        (PSNAPSHOT)malloc(sizeof(SNAPSHOT) * ulNewClientCX);

                    if (ulNewClientCX > pPrivate->cSnapshots)
                    {
                        // window has become wider:
                        // fill the front with zeroes
                        memset(paNewSnapshots,
                               0,
                               (ulNewClientCX - pPrivate->cSnapshots) * sizeof(SNAPSHOT));
                        // and copy old values after that
                        memmove(&paNewSnapshots[(ulNewClientCX - pPrivate->cSnapshots)],
                                pPrivate->paSnapshots,
                                pPrivate->cSnapshots * sizeof(SNAPSHOT));
                    }
                    else
                    {
                        // window has become smaller:
                        // e.g. ulnewClientCX = 100
                        //      pPrivate->cLoads = 200
                        // drop the first items
                        // ULONG ul = 0;
                        memmove(paNewSnapshots,
                                &pPrivate->paSnapshots[pPrivate->cSnapshots - ulNewClientCX],
                                ulNewClientCX * sizeof(SNAPSHOT));
                    }

                    pPrivate->cSnapshots = ulNewClientCX;
                    free(pPrivate->paSnapshots);
                    pPrivate->paSnapshots = paNewSnapshots;
                } // end if (pPrivate->palLoads)

                pPrivate->Setup.cx = pswpNew->cx;
                IwgtSaveSetupAndSend(hwnd, &pPrivate->Setup);
            } // end if (pswpNew->cx != pswpOld->cx)

            // force recreation of bitmap
            pPrivate->fUpdateGraph = TRUE;
            WinInvalidateRect(hwnd, NULL, FALSE);
        } // end if (pswpNew->fl & SWP_SIZE)
    } // end if (pPrivate)
}

/*
 *@@ IwgtPresParamChanged:
 *      implementation for WM_PRESPARAMCHANGED.
 *
 *@@changed V1.0.8 (2007-08-05) [pr]: rewrote this mess @@fixes 994
 */

STATIC VOID IwgtPresParamChanged(HWND hwnd,
                                 ULONG ulAttrChanged)
{
    PXCENTERWIDGET pWidget;
    PWIDGETPRIVATE pPrivate;
    // WM_PRESPARAMCHANGED gets sent before pWidget is set!
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PWIDGETPRIVATE)pWidget->pUser)
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

                // do not do this during WM_CREATE
                if (!pPrivate->fCreating)
                {
                    WinPostMsg(pWidget->pGlobals->hwndClient,
                               XCM_REFORMAT,
                               (MPARAM)XFMF_GETWIDGETSIZES,
                               0);
                }
            }
            break;

            default:
                fInvalidate = FALSE;
        }

        if (fInvalidate)
        {
            // force recreation of bitmap
            pPrivate->fUpdateGraph = TRUE;
            WinInvalidateRect(hwnd, NULL, FALSE);

            // V1.0.8 (2007-08-05) [pr]
            if (!pPrivate->fCreating)
                IwgtSaveSetupAndSend(hwnd, &pPrivate->Setup);
        }
    } // end if (pPrivate)
}

/*
 *@@ HackContextMenu:
 *
 */

STATIC VOID HackContextMenu(PWIDGETPRIVATE pPrivate)
{
    HWND hwndSubmenu;
    SHORT s = (SHORT)WinSendMsg(pPrivate->pWidget->hwndContextMenu,
                                MM_ITEMPOSITIONFROMID,
                                MPFROM2SHORT(ID_CRMI_PROPERTIES,
                                             FALSE),
                                0);
    if (hwndSubmenu = pwinhInsertSubmenu(pPrivate->pWidget->hwndContextMenu,
                                         s + 2,
                                         1999,
                                         pcmnGetString(ID_CRMI_IPWIDGET_SOURCE), // "Source",
                                                // localized V1.0.0 (2002-08-21) [umoeller]
                                         MIS_TEXT,
                                         0,
                                         NULL,
                                         0,
                                         0))
    {
        ULONG i;
        for (i = 0; i < IFMIB_ENTRIES; i++)
        {
            if (pPrivate->statif.iftable[i].ifDescr[0])
            {
                pwinhInsertMenuItem(hwndSubmenu,
                                    MIT_END,
                                    2000 + i,
                                    pPrivate->statif.iftable[i].ifDescr,
                                    MIS_TEXT,
                                    (i == pPrivate->Setup.ulDevIndex)
                                        ? MIA_CHECKED
                                        : 0);
            }
        }

        pwinhInsertMenuItem(pPrivate->pWidget->hwndContextMenu,
                            s + 3,
                            0,
                            "",
                            MIS_SEPARATOR,
                            0);

        pPrivate->fContextMenuHacked = TRUE;
        pPrivate->fContextMenuSource = hwndSubmenu;
            // eo: Store submenu handle for later updating upon interface change
    }
}

/*
 *@@ IwgtContextMenu:
 *      implementation for WM_CONTEXTMENU.
 *
 *@@added V0.9.19 (2002-06-08) [umoeller]
 */

STATIC MRESULT IwgtContextMenu(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    PXCENTERWIDGET pWidget;
    PWIDGETPRIVATE pPrivate;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PWIDGETPRIVATE)pWidget->pUser)
       )
    {
        if (    (pWidget->hwndContextMenu)
             && (!pPrivate->fContextMenuHacked)
           )
        {
            // first call for diskfree:
            // hack the context menu given to us
            HackContextMenu(pPrivate);
        }
        else
        {
            // eo: Update submenu to indicate selected interface without redrawing entire menu
            ULONG i;
            for (i = 0; i < IFMIB_ENTRIES; i++)
            {
                if (pPrivate->statif.iftable[i].ifDescr[0])
                {
                    WinCheckMenuItem(pPrivate->fContextMenuSource, 2000 + i, (i == pPrivate->Setup.ulDevIndex) ? MIA_CHECKED : 0 );
                }
            }
        }

        return pctrDefWidgetProc(hwnd, WM_CONTEXTMENU, mp1, mp2);
    }

    return (MRESULT)0;
}

/*
 *@@ fnwpMonitorWidgets:
 *      window procedure for the IP monitor.
 *
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
         *      WIDGETPRIVATE for our own stuff.
         */

        case WM_CREATE:
            WinSetWindowPtr(hwnd, QWL_USER, mp1);
            if (mp1)
                mrc = IwgtCreate(hwnd, (PXCENTERWIDGET)mp1);
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
            IwgtDestroy(hwnd);
            mrc = pctrDefWidgetProc(hwnd, msg, mp1, mp2);
        break;

        /*
         * WM_CONTROL:
         *      process notifications/queries from the XCenter.
         */

        case WM_CONTROL:
            mrc = (MPARAM)IwgtControl(hwnd, mp1, mp2);
        break;

        /*
         * WM_PAINT:
         *
         */

        case WM_PAINT:
            IwgtPaint(hwnd);
        break;

        /*
         * WM_TIMER:
         *      clock timer --> repaint.
         */

        case WM_TIMER:
            IwgtTimer(hwnd);
        break;

        /*
         * WM_WINDOWPOSCHANGED:
         *      on window resize, allocate new bitmap.
         */

        case WM_WINDOWPOSCHANGED:
            IwgtWindowPosChanged(hwnd, mp1, mp2);
        break;

        /*
         * WM_PRESPARAMCHANGED:
         *
         */

        case WM_PRESPARAMCHANGED:
            IwgtPresParamChanged(hwnd, (ULONG)mp1);
        break;

        case WM_CONTEXTMENU:
            mrc = IwgtContextMenu(hwnd, mp1, mp2);
        break;

        case WM_COMMAND:
        {
            PXCENTERWIDGET pWidget;
            PWIDGETPRIVATE pPrivate;
            if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
                 && (pPrivate = (PWIDGETPRIVATE)pWidget->pUser)
                 && ((SHORT)mp1) >= 2000
                 && ((SHORT)mp1) < 2000 + IFMIB_ENTRIES
               )
            {
                pPrivate->Setup.ulDevIndex = (SHORT)mp1 - 2000;
                IwgtSaveSetupAndSend(hwnd, &pPrivate->Setup);
            }
            else
                mrc = pctrDefWidgetProc(hwnd, msg, mp1, mp2);
        }
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
 *@@ IwgtInitModule:
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
 *@@changed V1.0.5 (2006-05-28) [pr]: Import SO32DLL functions manually
 */

ULONG EXPENTRY IwgtInitModule(HAB hab,         // XCenter's anchor block
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

    // V1.0.5 (2006-05-28) [pr]: Add declarations for importing SO32DLL functions dynamically
    // If this is done statically, the DLL can't be unloaded, which prevents IPMON.DLL
    // from being unloaded.
    if (!fImportsFailed)
    {
        if (pdoshResolveImports("SO32DLL.DLL",
                                &G_hmodSO32DLL,
                                G_aImports2,
                                sizeof(G_aImports2) / sizeof(G_aImports2[0])))
        {
            strcpy(pszErrorMsg, "SO32DLL.DLL imports failed.");
            fImportsFailed = TRUE;
        }
    }

    if (!fImportsFailed)
    {
        if (!WinRegisterClass(hab,
                              WNDCLASS_WIDGET_IPMON,
                              fnwpMonitorWidgets,
                              CS_PARENTCLIP | CS_SIZEREDRAW | CS_SYNCPAINT,
                              sizeof(PWIDGETPRIVATE))
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
 *@@ IwgtUnInitModule:
 *      optional export with ordinal 2, which can clean
 *      up global widget class data.
 *
 *      This gets called by the XCenter right before
 *      a widget DLL gets unloaded. Note that this
 *      gets called even if the "init module" export
 *      returned 0 (meaning an error) and the DLL
 *      gets unloaded right away.
 *
 *@@changed V1.0.5 (2006-05-28) [pr]: Free SO32DLL.DLL
 */

VOID EXPENTRY IwgtUnInitModule(VOID)
{
    if (G_hmodSO32DLL)
    {
        DosFreeModule(G_hmodSO32DLL);
        G_hmodSO32DLL = NULLHANDLE;
    }
}

/*
 *@@ IwgtQueryVersion:
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
 */

VOID EXPENTRY IwgtQueryVersion(PULONG pulMajor,
                               PULONG pulMinor,
                               PULONG pulRevision)
{
    *pulMajor = XFOLDER_MAJOR;              // dlgids.h
    *pulMinor = XFOLDER_MINOR;
    *pulRevision = XFOLDER_REVISION;
}

