
/*
 *@@sourcefile w_winlist.c:
 *      XCenter "Window list" widget.
 *
 *      This is an example of an XCenter widget plugin.
 *      This widget resides in WINLIST.DLL, which (as
 *      with all widget plugins) must be put into the
 *      plugins/xcenter directory of the XWorkplace
 *      installation directory.
 *
 *      Any XCenter widget plugin DLL must export the
 *      following procedures by ordinal:
 *
 *      -- Ordinal 1 (WwgtInitModule): this must
 *         return the widgets which this DLL provides.
 *
 *      -- Ordinal 2 (WwgtUnInitModule): this must
 *         clean up global DLL data.
 *
 *      The makefile in src\widgets compiles widgets
 *      with the VAC subsystem library. As a result,
 *      multiple threads are not supported.
 *
 *      This is all new with V0.9.7.
 *
 *      With V0.9.19, the window list has been reworked
 *      quite a bit to cooperate with the XWP daemon
 *      in order to no longer block the PM SIQ in certain
 *      situations. See pg_winlist.c for details.
 *
 *@@added V0.9.7 (2000-12-06) [umoeller]
 *@@header "shared\center.h"
 */

/*
 *      Copyright (C) 2000-2008 Ulrich M”ller.
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
ew */

#define INCL_DOSPROCESS
#define INCL_DOSMODULEMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINDIALOGS
#define INCL_WININPUT
#define INCL_WINSWITCHLIST
#define INCL_WINRECTANGLES
#define INCL_WINPOINTERS
#define INCL_WINSYS
#define INCL_WINMENUS
#define INCL_WINLISTBOXES
#define INCL_WINENTRYFIELDS

#define INCL_GPIPRIMITIVES
#define INCL_GPILOGCOLORTABLE
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
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\prfh.h"               // INI file helper routines;
                                        // this include is required for some
                                        // of the structures in shared\center.h
#include "helpers\stringh.h"            // string helper routines
#include "helpers\timer.h"              // replacement PM timers
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\center.h"              // public XCenter interfaces
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\kernel.h"              // XWorkplace Kernel

#include "config\drivdlgs.h"            // driver configuration dialogs

#include "hook\xwphook.h"               // hook and daemon definitions

#pragma hdrstop                     // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Private definitions
 *
 ********************************************************************/

// width of outer widget border:
#define WIDGET_BORDER           1
// width of window button borders:
#define THICK_BUTTON_BORDER     2

#define ICON_TEXT_SPACING       4

#define WLM_WINDOWCHANGE            (WM_USER + 200)
#define WLM_ICONCHANGE              (WM_USER + 201)

// string used for separating filters in setup strings;
// this better not appear in window titles
static PCSZ G_pcszFilterSeparator = "#~^ø@";

VOID EXPENTRY WwgtShowSettingsDlg(PWIDGETSETTINGSDLGDATA pData);

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

#define WNDCLASS_WIDGET_WINLIST "XWPCenterWinlistWidget"

static const XCENTERWIDGETCLASS G_WidgetClasses[] =
    {
        WNDCLASS_WIDGET_WINLIST,
        0,
        "WindowList",
        (PCSZ)(XCENTER_STRING_RESOURCE | ID_CRSI_WIDGET_WINDOWLIST),
                                    // widget class name displayed to user
                                    // (NLS DLL) V0.9.19 (2002-05-07) [umoeller]
        WGTF_UNIQUEPERXCENTER | WGTF_TOOLTIP_AT_MOUSE
            | WGTF_CONFIRMREMOVE,       // V0.9.20 (2002-08-08) [umoeller]
        WwgtShowSettingsDlg
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
PCMNLOADDLG pcmnLoadDlg = NULL;
PCMNQUERYDEFAULTFONT pcmnQueryDefaultFont = NULL;
PCMNQUERYHELPLIBRARY pcmnQueryHelpLibrary = NULL;
PCMNQUERYNLSMODULEHANDLE pcmnQueryNLSModuleHandle = NULL;
PCMNSETCONTROLSFONT pcmnSetControlsFont = NULL;
PCMNQUERYACTIVEDESKTOPHWND pcmnQueryActiveDesktopHWND = NULL;

PCTRDEFWIDGETPROC pctrDefWidgetProc = NULL;
PCTRDISPLAYHELP pctrDisplayHelp = NULL;
PCTRFREESETUPVALUE pctrFreeSetupValue = NULL;
PCTRPARSECOLORSTRING pctrParseColorString = NULL;
PCTRSCANSETUPSTRING pctrScanSetupString = NULL;
PCTRISXCENTERVIEW pctrIsXCenterView = NULL;

PDOSHMYPID pdoshMyPID = NULL;

PDRV_SPRINTF pdrv_sprintf = NULL;
PDRV_STRSTR pdrv_strstr = NULL;

PGPIHDRAW3DFRAME pgpihDraw3DFrame = NULL;
PGPIHDRAWPOINTER pgpihDrawPointer = NULL;
PGPIHMANIPULATERGB pgpihManipulateRGB = NULL;
PGPIHMEDIUMRGB pgpihMediumRGB = NULL;
PGPIHSWITCHTORGB pgpihSwitchToRGB = NULL;

PKRNPOSTDAEMONMSG pkrnPostDaemonMsg = NULL;
PKRNSENDDAEMONMSG pkrnSendDaemonMsg = NULL;

PLSTAPPENDITEM plstAppendItem = NULL;
PLSTCLEAR plstClear = NULL;
PLSTCOUNTITEMS plstCountItems = NULL;
PLSTCREATE plstCreate = NULL;
PLSTINIT plstInit = NULL;
PLSTFREE plstFree = NULL;
PLSTMALLOC plstMalloc = NULL;
PLSTNODEFROMINDEX plstNodeFromIndex = NULL;
PLSTQUERYFIRSTNODE plstQueryFirstNode = NULL;
PLSTREMOVENODE plstRemoveNode = NULL;
PLSTSTRDUP plstStrDup = NULL;

PTMRSTARTXTIMER ptmrStartXTimer = NULL;
PTMRSTOPXTIMER ptmrStopXTimer = NULL;

PWINHCENTERWINDOW pwinhCenterWindow = NULL;
PWINHFREE pwinhFree = NULL;
PWINHMERGEMENUS pwinhMergeMenus = NULL;
PWINHMERGEINTOSUBMENU pwinhMergeIntoSubMenu = NULL;
PWINHQUERYPRESCOLOR pwinhQueryPresColor = NULL;
PWINHQUERYSWITCHLIST pwinhQuerySwitchList = NULL;
PWINHQUERYWINDOWFONT pwinhQueryWindowFont = NULL;
PWINHSETWINDOWFONT pwinhSetWindowFont = NULL;

PSTRHBEAUTIFYTITLE pstrhBeautifyTitle = NULL;

PXSTRCAT pxstrcat = NULL;
PXSTRCPY pxstrcpy = NULL;
PXSTRCLEAR pxstrClear = NULL;
PXSTRINIT pxstrInit = NULL;

static const RESOLVEFUNCTION G_aImports[] =
    {
        "cmnGetString", (PFN*)&pcmnGetString,
        "cmnLoadDlg", (PFN*)&pcmnLoadDlg,
        "cmnQueryDefaultFont", (PFN*)&pcmnQueryDefaultFont,
        "cmnQueryHelpLibrary", (PFN*)&pcmnQueryHelpLibrary,
        "cmnQueryNLSModuleHandle", (PFN*)&pcmnQueryNLSModuleHandle,
        "cmnSetControlsFont", (PFN*)&pcmnSetControlsFont,
        "cmnQueryActiveDesktopHWND", (PFN*)&pcmnQueryActiveDesktopHWND,
        "ctrDefWidgetProc", (PFN*)&pctrDefWidgetProc,
        "ctrDisplayHelp", (PFN*)&pctrDisplayHelp,
        "ctrFreeSetupValue", (PFN*)&pctrFreeSetupValue,
        "ctrParseColorString", (PFN*)&pctrParseColorString,
        "ctrScanSetupString", (PFN*)&pctrScanSetupString,
        "ctrIsXCenterView", (PFN*)&pctrIsXCenterView,
        "doshMyPID", (PFN*)&pdoshMyPID,
        "drv_sprintf", (PFN*)&pdrv_sprintf,
        "drv_strstr", (PFN*)&pdrv_strstr,
        "gpihDraw3DFrame", (PFN*)&pgpihDraw3DFrame,
        "gpihDrawPointer", (PFN*)&pgpihDrawPointer,
        "gpihManipulateRGB", (PFN*)&pgpihManipulateRGB,
        "gpihMediumRGB", (PFN*)&pgpihMediumRGB,
        "gpihSwitchToRGB", (PFN*)&pgpihSwitchToRGB,
        "krnPostDaemonMsg", (PFN*)&pkrnPostDaemonMsg,
        "krnSendDaemonMsg", (PFN*)&pkrnSendDaemonMsg,
        "lstAppendItem", (PFN*)&plstAppendItem,
        "lstClear", (PFN*)&plstClear,
        "lstCountItems", (PFN*)&plstCountItems,
        "lstCreate", (PFN*)&plstCreate,
        "lstFree", (PFN*)&plstFree,
        "lstInit", (PFN*)&plstInit,
        "lstMalloc", (PFN*)&plstMalloc,
        "lstNodeFromIndex", (PFN*)&plstNodeFromIndex,
        "lstQueryFirstNode", (PFN*)&plstQueryFirstNode,
        "lstRemoveNode", (PFN*)&plstRemoveNode,
        "lstStrDup", (PFN*)&plstStrDup,
        "tmrStartXTimer", (PFN*)&ptmrStartXTimer,
        "tmrStopXTimer", (PFN*)&ptmrStopXTimer,
        "winhCenterWindow", (PFN*)&pwinhCenterWindow,
        "winhFree", (PFN*)&pwinhFree,
        "winhMergeMenus", (PFN*)&pwinhMergeMenus,
        "winhMergeIntoSubMenu", (PFN*)&pwinhMergeIntoSubMenu,
        "winhQueryPresColor", (PFN*)&pwinhQueryPresColor,
        "winhQuerySwitchList", (PFN*)&pwinhQuerySwitchList,
        "winhQueryWindowFont", (PFN*)&pwinhQueryWindowFont,
        "winhSetWindowFont", (PFN*)&pwinhSetWindowFont,
        "strhBeautifyTitle", (PFN*)&pstrhBeautifyTitle,
        "xstrcat", (PFN*)&pxstrcat,
        "xstrcpy", (PFN*)&pxstrcpy,
        "xstrClear", (PFN*)&pxstrClear,
        "xstrInit", (PFN*)&pxstrInit
    };

/* ******************************************************************
 *
 *   Private widget instance data
 *
 ********************************************************************/

/*
 *@@ WINLISTSETUP:
 *      instance data to which setup strings correspond.
 *      This is also a member of WINLISTPRIVATE.
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

typedef struct _WINLISTSETUP
{
    LONG        lcolBackground,         // background color
                lcolForeground;         // foreground color (for text)

    PSZ         pszFont;
            // if != NULL, non-default font (in "8.Helv" format);
            // this has been allocated using local malloc()!

    LINKLIST    llFilters;
            // linked list of window filters; this contains
            // plain swtitle PSZs allocated with plstStrDup

} WINLISTSETUP, *PWINLISTSETUP;

/*
 *@@ SWLISTENTRY:
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 */

typedef struct _WINLISTENTRY
{
    SWCNTRL     swctl;          // original switch list entry
            /*
              typedef struct _SWCNTRL {
                HWND         hwnd;                   window handle
                HWND         hwndIcon;               window-handle icon
                                                     --> hacked to contain frame icon
                HPROGRAM     hprog;                  program handle (not used)
                PID          idProcess;              process identity
                ULONG        idSession;              session identity
                ULONG        uchVisibility;          visibility
                                                     --> hacked to button index
                ULONG        fbJump;                 jump indicator
                                                     --> hacked to contain SWP.fl
                CHAR         szSwtitle[MAXNAMEL+4];  switch-list control block title (null-terminated)
                ULONG        bProgType;              program type
              } SWCNTRL;
            */

    ULONG       flSWP;

    // items not touched by FillEntry

    HPOINTER    hptr;
    ULONG       ulIndex;

} WINLISTENTRY, *PWINLISTENTRY;

/*
 *@@ WINLISTPRIVATE:
 *      more window data for the "Winlist" widget.
 *
 *      An instance of this is created on WM_CREATE
 *      fnwpWinlistWidget and stored in XCENTERWIDGET.pUser.
 *
 *@@changed V0.9.19 (2002-06-02) [umoeller]: now using WINLISTENTRY structs instead of SWCNTRL's
 */

typedef struct _WINLISTPRIVATE
{
    PXCENTERWIDGET pWidget;
            // reverse ptr to general widget data ptr; we need
            // that all the time and don't want to pass it on
            // the stack with each function call

    SIZEL       szlSysIcon;
            // system icon size V0.9.20 (2002-07-31) [umoeller]

    WINLISTSETUP Setup;
            // widget settings that correspond to a setup string

    HWND        hwndButtonContextMenu,
            // != 0 after context menu has been hacked for the first time
                hwndContextMenuShowing;
            // != 0 while context menu is showing (overriding standard widget)

    LINKLIST    llWinList;
            // linked list of WINLISTENTRY's with items to be displayed.
            // First item is displayed left, last item right.
            // This constructed from the current switch list, however:
            // -- filtered items are not on here;
            // -- items are always added to the tail (so new items
            //    appear to the right).
            // This is now a linked list (V0.9.11) because apparently
            // WinQuerySwitchList reuses the existing items so that
            // sometimes new items appear randomly in the middle.
            // V0.9.19 (2002-06-02) [umoeller]:
            // now using WINLISTENTRY structs instead of SWCNTRL's

    PWINLISTENTRY   pCtrlActive,
            // ptr into pswBlock for last active window or NULL if none
                    pCtrlSourceEmphasis,
            // ptr into pswBlock for button with source emphasis or NULL if none
                    pCtrlMenu;
            // same as pCtrlSourceEmphasis; second field needed because WM_COMMAND
            // might come in after WM_MENUEND (for source emphasis)

    XSTRING     strTooltip;
            // tip for the tooltip control
} WINLISTPRIVATE, *PWINLISTPRIVATE;

/* ******************************************************************
 *
 *   Widget setup management
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
 *@@ WwgtClearSetup:
 *      cleans up the data in the specified setup
 *      structure, but does not free the structure
 *      itself.
 */

STATIC VOID WwgtClearSetup(PWINLISTSETUP pSetup)
{
    if (pSetup)
    {
        plstClear(&pSetup->llFilters);

        if (pSetup->pszFont)
        {
            free(pSetup->pszFont);
            pSetup->pszFont = NULL;
        }
    }
}

/*
 *@@ WwgtScanSetup:
 *      scans the given setup string and translates
 *      its data into the specified binary setup
 *      structure.
 *
 *      NOTE: It is assumed that pSetup is zeroed
 *      out. We do not clean up previous data here.
 *
 *@@changed V0.9.14 (2001-08-01) [umoeller]: fixed memory leak
 *@@changed V1.0.2 (2003-02-03) [umoeller]: changed default text color
 */

STATIC VOID WwgtScanSetup(PCSZ pcszSetupString,
                          PWINLISTSETUP pSetup)
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

    // filters:
    plstInit(&pSetup->llFilters, TRUE);
    if (p = pctrScanSetupString(pcszSetupString,
                                "FILTERS"))
    {
        PSZ pFilter = p;
        PSZ pSep = 0;
        ULONG ulSeparatorLength = strlen(G_pcszFilterSeparator);
        do
        {
            if (pSep = pdrv_strstr(pFilter, G_pcszFilterSeparator))
                *pSep = '\0';
            // append copy filter; use plstStrDup to
            // allow auto-free of the list
            plstAppendItem(&pSetup->llFilters,
                           plstStrDup(pFilter));
            if (pSep)
                pFilter += (strlen(pFilter) + ulSeparatorLength);
        } while (pSep);

        pctrFreeSetupValue(p);      // V0.9.14 (2001-08-01) [umoeller]
    }
}

/*
 *@@ WwgtSaveSetup:
 *      composes a new setup string.
 *      The caller must invoke xstrClear on the
 *      string after use.
 */

STATIC VOID WwgtSaveSetup(PXSTRING pstrSetup,       // out: setup string (is cleared first)
                          PWINLISTSETUP pSetup)
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

    if (plstCountItems(&pSetup->llFilters))
    {
        // we have items on the filters list:
        PLISTNODE   pNode = plstQueryFirstNode(&pSetup->llFilters);
        BOOL        fFirst = TRUE;
        // add keyword first
        pxstrcat(pstrSetup, "FILTERS=", 0);

        while (pNode)
        {
            PSZ pszFilter = (PSZ)pNode->pItemData;
            if (!fFirst)
                // not first loop: add separator first
                pxstrcat(pstrSetup, G_pcszFilterSeparator, 0);

            // append this filter
            pxstrcat(pstrSetup, pszFilter, 0);

            pNode = pNode->pNext;
            fFirst = FALSE;
        }

        // add terminator
        pxstrcat(pstrSetup, ";", 0);
    }
}

/*
 *@@ WwgtSaveSetupAndSend:
 *
 *@@added V0.9.20 (2002-08-10) [umoeller]
 */

VOID WwgtSaveSetupAndSend(HWND hwndWidget,
                          PWINLISTPRIVATE pPrivate)
{
    XSTRING strSetup;
    WwgtSaveSetup(&strSetup,
                  &pPrivate->Setup);
    if (strSetup.ulLength)
        // changed V0.9.13 (2001-06-21) [umoeller]:
        // post it to parent instead of fixed XCenter client
        // to make this trayable
        WinSendMsg(WinQueryWindow(hwndWidget, QW_PARENT), // pPrivate->pWidget->pGlobals->hwndClient,
                   XCM_SAVESETUP,
                   (MPARAM)hwndWidget,
                   (MPARAM)strSetup.psz);
    pxstrClear(&strSetup);
}

/* ******************************************************************
 *
 *   Helpers
 *
 ********************************************************************/

/*
 *@@ IsCtrlDesktopOrXCenter:
 *      returns TRUE if the given frame is either
 *      the desktop or an open XCenter frame.
 *
 *@@added V0.9.16 (2001-12-02) [umoeller]
 */

STATIC BOOL IsCtrlDesktopOrXCenter(HWND hwndFrame)
{
    // rule out all XCenter frames
    // V0.9.16 (2001-12-02) [umoeller]
    if (pctrIsXCenterView(hwndFrame))
        return TRUE;
    // rule out active Desktop
    else if (hwndFrame == pcmnQueryActiveDesktopHWND())
        return TRUE;

    return FALSE;
}

/* ******************************************************************
 *
 *   Widget settings dialog
 *
 ********************************************************************/

/*
 *@@ DumpSwitchList:
 *      puts the current switch list entries into the listbox.
 *
 *@@changed V1.0.1 (2002-09-26) [kai]: remove CRLFs from switchlist title
 */

STATIC VOID DumpSwitchList(HWND hwnd)
{
    PSWBLOCK pswBlock;
    if (pswBlock = pwinhQuerySwitchList(WinQueryAnchorBlock(hwnd)))
            // calls WinQuerySwitchList
    {
        HWND hwndCombo = WinWindowFromID(hwnd, ID_CRDI_FILTERS_NEWCOMBO);
        ULONG ul = 0;
        for (;
             ul < pswBlock->cswentry;
             ul++)
        {
            PSWCNTRL    pCtrlThis = &pswBlock->aswentry[ul].swctl;

            if (    (pCtrlThis->uchVisibility & SWL_VISIBLE)
                 && (!IsCtrlDesktopOrXCenter(pCtrlThis->hwnd))
                            // V0.9.16 (2001-12-02) [umoeller]
               )
            {
                pstrhBeautifyTitle(pCtrlThis->szSwtitle);

                // visible item:
                WinInsertLboxItem(hwndCombo,
                                  LIT_SORTASCENDING,
                                  pCtrlThis->szSwtitle);
            }
        }

        pwinhFree(pswBlock);
    }
}

/*
 *@@ Settings2Dlg:
 *      sets up the dialog according to the current settings.
 */

STATIC VOID Settings2Dlg(HWND hwnd,
                         PWINLISTSETUP pSetup)
{
    HWND hwndFiltersLB = WinWindowFromID(hwnd, ID_CRDI_FILTERS_CURRENTLB);
    PLISTNODE pNode = plstQueryFirstNode(&pSetup->llFilters);

    winhDeleteAllItems(hwnd); // macro
    while (pNode)
    {
        PSZ pszFilterThis = (PSZ)pNode->pItemData;
        WinInsertLboxItem(hwndFiltersLB,
                          LIT_SORTASCENDING,
                          pszFilterThis);

        pNode = pNode->pNext;
    }
}

/*
 *@@ IsSwlistItemAddable:
 *      returns TRUE if the specified combo's
 *      entry field is not empty and not yet
 *      present in the filters list box.
 */

STATIC BOOL IsSwlistItemAddable(HWND hwndCombo,
                                PWINLISTSETUP pSetup)
{
    BOOL fEnableComboAdd = FALSE;
    if (WinQueryWindowTextLength(hwndCombo) != 0)
    {
        // combo has text:
        // check if this is already in the list box...
        CHAR szFilter[MAXNAMEL + 4];
        fEnableComboAdd = TRUE;
        if (WinQueryWindowText(hwndCombo,
                               sizeof(szFilter),
                               szFilter))
        {
            PLISTNODE pNode = plstQueryFirstNode(&pSetup->llFilters);
            while (pNode)
            {
                PSZ pszFilterThis = (PSZ)pNode->pItemData;
                if (!strcmp(pszFilterThis, szFilter))
                {
                    fEnableComboAdd = FALSE;
                    break;
                }
                pNode = pNode->pNext;
            }
        }
    }

    return fEnableComboAdd;
}

/*
 *@@ EnableItems:
 *      enables the items in the dialog according to the
 *      current selections.
 */

STATIC VOID EnableItems(HWND hwnd,
                        PWINLISTSETUP pSetup)
{
    HWND hwndFiltersLB = WinWindowFromID(hwnd, ID_CRDI_FILTERS_CURRENTLB);
    HWND hwndCombo = WinWindowFromID(hwnd, ID_CRDI_FILTERS_NEWCOMBO);

    // "remove" from list box
    WinEnableControl(hwnd,
                     ID_CRDI_FILTERS_REMOVE,
                     (winhQueryLboxSelectedItem(hwndFiltersLB,
                                                LIT_FIRST) // macro
                            != LIT_NONE));

    // "add" from combo box
    WinEnableControl(hwnd,
                     ID_CRDI_FILTERS_ADD,
                     IsSwlistItemAddable(hwndCombo, pSetup));
}

/*
 *@@ Dlg2Settings:
 *      retrieves the new settings from the dialog.
 */

STATIC VOID Dlg2Settings(HWND hwnd,
                         PWINLISTSETUP pSetup)
{
    HWND hwndFiltersLB = WinWindowFromID(hwnd, ID_CRDI_FILTERS_CURRENTLB);
    SHORT sItemCount = WinQueryLboxCount(hwndFiltersLB),
          s = 0;
    // clear previous list
    plstClear(&pSetup->llFilters);

    for (s = 0;
         s < sItemCount;
         s++)
    {
        CHAR szFilter[MAXNAMEL+4];  // same as in SWCNTRL.szSwtitle[];
        WinQueryLboxItemText(hwndFiltersLB,
                             s,
                             szFilter,
                             sizeof(szFilter));
        // append copy filter; use plstStrDup to
        // allow auto-free of the list
        plstAppendItem(&pSetup->llFilters,
                       plstStrDup(szFilter));
    }
}

/*
 *@@ fnwpSettingsDlg:
 *      dialog proc for the winlist settings dialog.
 */

STATIC MRESULT EXPENTRY fnwpSettingsDlg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        /*
         * WM_INITDLG:
         *      fill the control. We get the
         *      WIDGETSETTINGSDLGDATA in mp2.
         */

        case WM_INITDLG:
        {
            PWIDGETSETTINGSDLGDATA pData = (PWIDGETSETTINGSDLGDATA)mp2;
            PWINLISTSETUP pSetup;
            WinSetWindowPtr(hwnd, QWL_USER, pData);
            if (pSetup = malloc(sizeof(WINLISTSETUP)))
            {
                memset(pSetup, 0, sizeof(*pSetup));
                // store this in WIDGETSETTINGSDLGDATA
                pData->pUser = pSetup;

                WwgtScanSetup(pData->pcszSetupString, pSetup);

                // limit entry field to max len of switch titles
                winhSetEntryFieldLimit(WinWindowFromID(hwnd,
                                                       ID_CRDI_FILTERS_NEWCOMBO),
                                       MAXNAMEL + 4 - 1); // null terminator!

                DumpSwitchList(hwnd);
                Settings2Dlg(hwnd, pSetup);
                EnableItems(hwnd, pSetup);
            }
            mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);
        }
        break;

        /*
         * WM_COMMAND:
         *
         */

        case WM_COMMAND:
        {
            PWIDGETSETTINGSDLGDATA pData;
            PWINLISTSETUP pSetup;
            if (    (pData = (PWIDGETSETTINGSDLGDATA)WinQueryWindowPtr(hwnd, QWL_USER))
                 && (pSetup = (PWINLISTSETUP)pData->pUser)
               )
            {
                USHORT usCmd = (USHORT)mp1;
                HWND hwndFiltersLB = WinWindowFromID(hwnd, ID_CRDI_FILTERS_CURRENTLB);
                HWND hwndCombo = WinWindowFromID(hwnd, ID_CRDI_FILTERS_NEWCOMBO);

                switch (usCmd)
                {
                    /*
                     * ID_CRDI_FILTERS_REMOVE:
                     *      remove selected filter
                     */

                    case ID_CRDI_FILTERS_REMOVE:
                    {
                        SHORT sSel = winhQueryLboxSelectedItem(hwndFiltersLB,
                                                               LIT_FIRST); // macro
                        if (sSel != LIT_NONE)
                        {
                            WinDeleteLboxItem(hwndFiltersLB, sSel);
                            Dlg2Settings(hwnd, pSetup);
                            EnableItems(hwnd, pSetup);

                            // reset focus to listbox
                            WinSetFocus(HWND_DESKTOP, hwndFiltersLB);
                        }
                    }
                    break;

                    /*
                     * ID_CRDI_FILTERS_ADD:
                     *      add specified filter
                     */

                    case ID_CRDI_FILTERS_ADD:
                        if (IsSwlistItemAddable(hwndCombo, pSetup))
                        {
                            CHAR szFilter[MAXNAMEL + 4];
                            // we have an entry:
                            if (WinQueryWindowText(hwndCombo,
                                                   sizeof(szFilter),
                                                   szFilter))
                            {
                                // SHORT sSel;
                                WinInsertLboxItem(hwndFiltersLB,
                                                  LIT_SORTASCENDING,
                                                  szFilter);
                                winhSetLboxSelectedItem(hwndFiltersLB,
                                                        LIT_NONE,
                                                        0);
                                // clean up combo
                                WinSetWindowText(hwndCombo, NULL);
                                winhSetLboxSelectedItem(hwndCombo,
                                                        LIT_NONE,
                                                        0);

                                // get dlg items
                                Dlg2Settings(hwnd, pSetup);
                                EnableItems(hwnd, pSetup);

                                // reset focus to combo
                                WinSetFocus(HWND_DESKTOP, hwndCombo);
                            }
                        }
                    break;

                    /*
                     * DID_OK:
                     *      OK button -> recompose settings
                     *      and get outta here.
                     */

                    case DID_OK:
                    {
                        XSTRING strSetup;
                        WwgtSaveSetup(&strSetup,
                                      pSetup);
                        pData->pctrSetSetupString(pData->hSettings,
                                                  strSetup.psz);
                        pxstrClear(&strSetup);
                        WinDismissDlg(hwnd, DID_OK);
                    }
                    break;

                    /*
                     * DID_CANCEL:
                     *      cancel button...
                     */

                    case DID_CANCEL:
                        WinDismissDlg(hwnd, DID_CANCEL);
                    break;

                    case DID_HELP:
                        pctrDisplayHelp(pData->pGlobals,
                                        pcmnQueryHelpLibrary(),
                                        ID_XSH_WIDGET_WINLIST_SETTINGS);
                    break;
                } // end switch (usCmd)
            } // end if (pSetup)
        }
        break; // WM_COMMAND

        /*
         * WM_CONTROL:
         *
         */

        case WM_CONTROL:
        {
            PWIDGETSETTINGSDLGDATA pData;
            PWINLISTSETUP pSetup;
            if (    (pData = (PWIDGETSETTINGSDLGDATA)WinQueryWindowPtr(hwnd, QWL_USER))
                 && (pSetup = (PWINLISTSETUP)pData->pUser)
               )
            {
                USHORT usItemID = SHORT1FROMMP(mp1),
                       usNotifyCode = SHORT2FROMMP(mp1);
                switch (usItemID)
                {
                    // current filters listbox:
                    case ID_CRDI_FILTERS_CURRENTLB:
                        if (usNotifyCode == LN_SELECT)
                            // change "remove" button state
                            EnableItems(hwnd, pSetup);
                    break;

                    // switchlist combobox:
                    case ID_CRDI_FILTERS_NEWCOMBO:
                        if (usNotifyCode == CBN_EFCHANGE)
                            // entry field has changed:
                            // change "add" button state
                            EnableItems(hwnd, pSetup);
                        else if (usNotifyCode == CBN_ENTER)
                            // double-click on list item:
                            // simulate "add" button
                            WinPostMsg(hwnd,
                                       WM_COMMAND,
                                       (MPARAM)ID_CRDI_FILTERS_ADD,
                                       MPFROM2SHORT(CMDSRC_OTHER, TRUE));
                    break;
                }
            }
        }
        break;

        /*
         * WM_DESTROY:
         *
         */

        case WM_DESTROY:
        {
            PWIDGETSETTINGSDLGDATA pData;
            PWINLISTSETUP pSetup;
            if (    (pData = (PWIDGETSETTINGSDLGDATA)WinQueryWindowPtr(hwnd, QWL_USER))
                 && (pSetup = (PWINLISTSETUP)pData->pUser)
               )
            {
                WwgtClearSetup(pSetup);
                free(pSetup);
            } // end if (pData)

            mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);
        }
        break;

        default:
            mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);
    }

    return mrc;
}

/* ******************************************************************
 *
 *   Callbacks stored in XCENTERWIDGETCLASS
 *
 ********************************************************************/

/*
 *@@ WwgtShowSettingsDlg:
 *      this displays the winlist widget's settings
 *      dialog.
 *
 *      This procedure's address is stored in
 *      XCENTERWIDGET so that the XCenter knows that
 *      we can do this.
 *
 *      When calling this function, the XCenter expects
 *      it to display a modal dialog and not return
 *      until the dialog is destroyed. While the dialog
 *      is displaying, it would be nice to have the
 *      widget dynamically update itself.
 */

VOID EXPENTRY WwgtShowSettingsDlg(PWIDGETSETTINGSDLGDATA pData)
{
    HWND hwnd;
    if (hwnd = pcmnLoadDlg(pData->hwndOwner,
                           fnwpSettingsDlg,
                           ID_CRD_WINLISTWGT_SETTINGS,
                           // pass original setup string with WM_INITDLG
                           (PVOID)pData))
    {
        pcmnSetControlsFont(hwnd,
                            1,
                            10000);

        pwinhCenterWindow(hwnd);         // invisibly

        // go!!
        WinProcessDlg(hwnd);

        WinDestroyWindow(hwnd);
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
 *@@ GetPaintableRect:
 *      retrieves the widget's client rectangle
 *      and shrinks it by the thin outer widget
 *      border. The rectangle put into prcl can
 *      be used for painting the buttons.
 */

STATIC VOID GetPaintableRect(PWINLISTPRIVATE pPrivate,
                             PRECTL prcl)
{
    WinQueryWindowRect(pPrivate->pWidget->hwndWidget,
                       prcl);

    // if (pPrivate->pWidget->pGlobals->flDisplayStyle & XCS_SUNKBORDERS)
    {
        prcl->xLeft += WIDGET_BORDER;
        prcl->xRight -= WIDGET_BORDER;
        prcl->yBottom += WIDGET_BORDER;
        prcl->yTop -= WIDGET_BORDER;
    }

    // we won't use WinInflateRect... what the hell
    // does this API need an anchor block for to
    // do a couple of additions?!?
    /*
    WinInflateRect(pWidget->hab,
                   prcl,
                   -WIDGET_BORDER,
                   -WIDGET_BORDER); */
}

/*
 *@@ IsCtrlFiltered:
 *      returns TRUE if the control is filtered
 *      and should therefore not be displayed
 *      as a button in the window list.
 *
 *      This is the one place which is used by
 *      all other parts to determine whether the
 *      specified switch control is filtered.
 *
 *      Note: You must pass in an original switch
 *      list entry, not one from the private list,
 *      because we are hacking the entries on
 *      that list.
 *
 *@@changed V0.9.12 (2001-04-28) [umoeller]: now matching first chars of filter only
 *@@changed V0.9.16 (2001-11-25) [umoeller]: now ruling out desktop and all XCenters always
 */

STATIC BOOL IsCtrlFiltered(PLINKLIST pllFilters,   // in: pPrivate->Setup.llFilters
                           HWND hwndXCenterFrame,  // in: pPrivate->pWidget->pGlobals->hwndFrame
                           PSWCNTRL pCtrlOrig)     // in: un-hacked switch list entry
{
    BOOL brc = FALSE;

    // rule out invisible tasklist entries
    if (!(pCtrlOrig->uchVisibility & SWL_VISIBLE))
    {
        #ifdef DEBUG_WINDOWLIST
        _PmpfF(("SWL_VISIBLE is null, filtering"));
        #endif

        brc = TRUE;
    }
    // rule out Desktop and all XCenters
    else if (IsCtrlDesktopOrXCenter(pCtrlOrig->hwnd))
    {
        #ifdef DEBUG_WINDOWLIST
        _PmpfF(("item is desktop or XCenter, filtering"));
        #endif

        brc = TRUE;
    }
    else
    {
        // go thru user-defined filters
        PLISTNODE pNode = plstQueryFirstNode(pllFilters);
        while (pNode)
        {
            PSZ pszFilterThis = (PSZ)pNode->pItemData;
            if (!strncmp(pCtrlOrig->szSwtitle,
                         pszFilterThis,
                         strlen(pszFilterThis)))    // match length of filter only
                                                    // V0.9.12 (2001-04-28) [umoeller]
            {
                // filtered:
                brc = TRUE;
                break;
            }

            pNode = pNode->pNext;
        }
    }
    return brc;
}

/*
 *@@ FindSwitchNodeFromHWND:
 *      searches the given list of SWCNTRL entries
 *      for the specified HWND.
 *
 *@@added V0.9.11 (2001-04-18) [umoeller]
 */

STATIC PLISTNODE FindSwitchNodeFromHWND(PLINKLIST pll,
                                        HWND hwnd)
{
    PLISTNODE pNode;
    for (pNode = plstQueryFirstNode(pll);
         pNode;
         pNode = pNode->pNext)
    {
        PSWCNTRL pCtrl = (PSWCNTRL)pNode->pItemData;
        if (pCtrl->hwnd == hwnd)
            return pNode;
    }

    return NULL;
}

/*
 *@@ FillEntry:
 *      fills the given WINLISTENTRY. Note that
 *      this siply copies the SWCNTRL and fills
 *      the SWP as well, but does not modify hptr
 *      or ulIndex, which must be set by the caller.
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 */

STATIC VOID FillEntry(PWINLISTENTRY pCtrl,
                      PSWCNTRL pOrigEntry)
{
    SWP     swp;

    memcpy(&pCtrl->swctl, pOrigEntry, sizeof(SWCNTRL));
    WinQueryWindowPos(pOrigEntry->hwnd,
                      &swp);
    pCtrl->flSWP = swp.fl;
}

/*
 *@@ AddEntry:
 *      adds a new window list entry for the given
 *      SWCNTRL. Checks the filters list beforehand;
 *      returns NULL if the window is filtered.
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 */

STATIC PWINLISTENTRY AddEntry(PWINLISTPRIVATE pPrivate,
                              PSWCNTRL pOrigEntry)
{
    PWINLISTENTRY pCtrl = NULL;

    #ifdef DEBUG_WINDOWLIST
    _PmpfF(("%lX (%s)", pOrigEntry->hwnd, pOrigEntry->szSwtitle));
    #endif

    if (    (!IsCtrlFiltered(&pPrivate->Setup.llFilters,
                             pPrivate->pWidget->pGlobals->hwndFrame,
                             pOrigEntry))
         && (pCtrl = plstMalloc(sizeof(WINLISTENTRY)))
            // use XFLDR.DLL heap, this is auto-free
       )
    {
        FillEntry(pCtrl, pOrigEntry);
        pCtrl->hptr = pOrigEntry->hwndIcon;     // set by daemon probably
        pCtrl->ulIndex = plstCountItems(&pPrivate->llWinList);

        plstAppendItem(&pPrivate->llWinList,
                       pCtrl);
    }

    return pCtrl;
}

/*
 *@@ AddOrRefreshEntry:
 *      adds or refreshes the window list entry for
 *      hwnd. This calls AddEntry, which checks the
 *      filters list.
 *
 *      Returns the new entry or NULL if the entry
 *      was not added (probably because it's on the
 *      filters list).
 *
 *      Note: Despite its name, this function may actually
 *      _remove_ a node from the list if its title changed
 *      and now matches a filter.
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 *@@changed V1.0.1 (2002-09-26) [kai]: remove CRLFs from switchlist title
 */

STATIC PWINLISTENTRY AddOrRefreshEntry(PWINLISTPRIVATE pPrivate,
                                       HWND hwnd,                // in: window to add entry for
                                       PBOOL pfRefreshAll)       // out: TRUE if item was added or removed
{
    HSWITCH hsw;
    PWINLISTENTRY  pCtrl = NULL;
    if (hsw = WinQuerySwitchHandle(hwnd, 0))
    {
        // check if the item is in the list
        PLISTNODE pNode;
        SWCNTRL ctl;
        if (pNode = FindSwitchNodeFromHWND(&pPrivate->llWinList,
                                           hwnd))
        {
            // yes: refresh title
            pCtrl = (PWINLISTENTRY)pNode->pItemData;
            WinQuerySwitchEntry(hsw, &ctl);

            pstrhBeautifyTitle(ctl.szSwtitle);

            // now, the item might be filtered now because its
            // title changed, so check filters list again
            if (IsCtrlFiltered(&pPrivate->Setup.llFilters,
                               pPrivate->pWidget->pGlobals->hwndFrame,
                               &ctl))
            {
                // filtered: kill it!
                plstRemoveNode(&pPrivate->llWinList, pNode);
                pCtrl = NULL;
                *pfRefreshAll = TRUE;
            }
            else
                // not filtered:
                FillEntry(pCtrl, &ctl);
        }
        else
        {
            // not in list: add it if it's not filtered
            if (!WinQuerySwitchEntry(hsw, &ctl))
            {
                pstrhBeautifyTitle(ctl.szSwtitle);
                if (pCtrl = AddEntry(pPrivate, &ctl))
                    *pfRefreshAll = TRUE;
            }
        }

        // check if this is active
        if (    (pCtrl)
             && (pCtrl->swctl.hwnd == WinQueryActiveWindow(HWND_DESKTOP))
           )
            pPrivate->pCtrlActive = pCtrl;
    }

    return pCtrl;
}

/*
 *@@ ScanSwitchList:
 *      scans or rescans the PM task list (switch list)
 *      and initializes all data in WINLISTPRIVATE
 *      accordingly.
 *
 *      Gets called from WwgtCreate.
 *
 *@@changed V0.9.11 (2001-04-18) [umoeller]: rewritten
 *@@changed V0.9.12 (2001-04-28) [umoeller]: didn't pick up changes in the filters, fixed
 *@@changed V0.9.19 (2002-05-28) [umoeller]: rewritten to use XWPDAEMN
 *@@changed V1.0.1 (2002-09-26) [kai]: remove CRLFs from switchlist title
 */

STATIC VOID ScanSwitchList(HWND hwndWidget,
                           PWINLISTPRIVATE pPrivate)
{
    PSWBLOCK pswBlock;
    ULONG pid, tid;

    plstClear(&pPrivate->llWinList);      // auto-free

    // we used to call WinQuerySwitchList here, but let's ask
    // the daemon for his list because he's got those entries in order
    // V0.9.19 (2002-05-28) [umoeller]
    if (    (WinQueryWindowProcess(hwndWidget,
                                   &pid,
                                   &tid))
         && (pswBlock = (PSWBLOCK)pkrnSendDaemonMsg(XDM_QUERYWINLIST,
                                                    (MPARAM)pid,
                                                    0))
        )
    {
        // daemon has given us a block of shared memory:
        ULONG ul;
        for (ul = 0;
             ul < pswBlock->cswentry;
             ++ul)
        {
            pstrhBeautifyTitle(pswBlock->aswentry[ul].swctl.szSwtitle);
            AddEntry(pPrivate,
                     &pswBlock->aswentry[ul].swctl);
        }

        DosFreeMem(pswBlock);
    }
}

/*
 *@@ CalcButtonCX:
 *      returns the width of the specified button.
 *      *pcxRegular receives the regular width;
 *      this is the same as the returned size of
 *      the current button, except if the button
 *      is the last one.
 *
 *@@added V0.9.9 (2001-01-29) [umoeller]
 */

STATIC LONG CalcButtonCX(PWINLISTPRIVATE pPrivate,
                         PRECTL prclSubclient,
                         PWINLISTENTRY pCtrlThis,  // in: switch list entry; can be NULL
                         PLONG pcxRegular)
{
    LONG    cxPerButton = 0;
    ULONG   cShow;

    // avoid division by zero
    if (cShow = plstCountItems(&pPrivate->llWinList))
    {
        // max paint space:
        ULONG   ulCX = (prclSubclient->xRight - prclSubclient->xLeft);
        // calc width for this button...
        // use standard with, except for last button
        cxPerButton = ulCX / cShow;

        // limit size per button V0.9.9 (2001-01-29) [umoeller]
        if (cxPerButton > 130)
        {
            cxPerButton = 130;
            *pcxRegular = cxPerButton;
        }
        else
        {
            // no limit:
            *pcxRegular = cxPerButton;
        }
    }

    return cxPerButton;
}

/*
 *@@ DrawOneCtrl:
 *      paints one window button.
 *
 *      Gets called from DrawAllCtrls and from
 *      RedrawActiveChanged.
 *
 *      Preconditions: The switch list should have
 *      been scanned (ScanSwitchList).
 *
 *@@changed V0.9.9 (2001-02-08) [umoeller]: now centering icons on button vertically
 *@@changed V0.9.19 (2002-05-28) [umoeller]: added halftoning
 *@@changed V0.9.19 (2002-06-18) [umoeller]: fixed bad indices by always re-numbering here
 */

STATIC VOID DrawOneCtrl(PWINLISTPRIVATE pPrivate,
                        HPS hps,
                        PRECTL prclSubclient,     // in: paint area (exclusive)
                        PWINLISTENTRY pCtrlThis,  // in: switch list entry to paint
                        HWND hwndActive,          // in: currently active window
                        PLONG plNextX)            // out: next X pos (ptr can be NULL)
{
    // colors for borders
    LONG    lLeft,
            lRight;
    ULONG   cShow;

    const XCENTERGLOBALS *pGlobals = pPrivate->pWidget->pGlobals;

    // avoid division by zero
    if (cShow = plstCountItems(&pPrivate->llWinList))
    {
        RECTL   rclButtonArea;

        LONG    lButtonColor = pPrivate->Setup.lcolBackground;

        LONG    cxRegular = 0,
                cxThis = CalcButtonCX(pPrivate,
                                      prclSubclient,
                                      pCtrlThis,
                                      &cxRegular);

        LONG    lButtonBorder
            = ((pGlobals->flDisplayStyle & XCS_FLATBUTTONS) == 0)
                            ? THICK_BUTTON_BORDER   // raised buttons
                            : 1;        // flat buttons

        BOOL    fHalftone;

        if ((hwndActive) && (pCtrlThis->swctl.hwnd == hwndActive))
        {
            // active window: paint rect lowered
            lLeft = pGlobals->lcol3DDark;
            lRight = pGlobals->lcol3DLight;

            // and paint button lighter:
            pgpihManipulateRGB(&lButtonColor, 1.1);
        }
        else
        {
            lLeft = pGlobals->lcol3DLight;
            lRight = pGlobals->lcol3DDark;
        }

        // draw frame
        rclButtonArea.yBottom = prclSubclient->yBottom;
        // calculate X coordinate: ulindex has
        // the button index as calculated by ScanSwitchList,
        // so we can multiply
        rclButtonArea.xLeft = prclSubclient->xLeft
                              + (pCtrlThis->ulIndex * cxRegular);
        rclButtonArea.yTop = prclSubclient->yTop - 1;
        rclButtonArea.xRight = rclButtonArea.xLeft + cxThis - 1;

        // draw button frame
        pgpihDraw3DFrame(hps,
                         &rclButtonArea,        // inclusive
                         lButtonBorder,
                         lLeft,
                         lRight);

        // store next X if caller wants it
        if (plNextX)
            *plNextX = rclButtonArea.xRight + 1;

        // source emphasis for this control?
        if (    (pPrivate->pCtrlSourceEmphasis)
             && (pCtrlThis == pPrivate->pCtrlSourceEmphasis)
           )
        {
            // yes:
            POINTL ptl;
            GpiSetColor(hps, RGBCOL_BLACK);
            GpiSetLineType(hps, LINETYPE_DOT);
            ptl.x = rclButtonArea.xLeft;
            ptl.y = rclButtonArea.yBottom;
            GpiMove(hps, &ptl);
            ptl.x = rclButtonArea.xRight;
            ptl.y = rclButtonArea.yTop;
            GpiBox(hps,
                   DRO_OUTLINE,
                   &ptl,
                   0, 0);
        }

        // draw button middle
        rclButtonArea.xLeft += lButtonBorder;
        rclButtonArea.yBottom += lButtonBorder;
        rclButtonArea.xRight -= (lButtonBorder - 1);
        rclButtonArea.yTop -= (lButtonBorder - 1);
        WinFillRect(hps,
                    &rclButtonArea,         // exclusive
                    lButtonColor);

        // draw halftone if minimized or hidden
        fHalftone = (!!(pCtrlThis->flSWP & (SWP_MINIMIZE | SWP_HIDE)));

        if (pCtrlThis->hptr)
        {
            // V0.9.9 (2001-02-08) [umoeller] now centering icons
            // on buttons vertically, if the widget is really tall
            LONG   cy = rclButtonArea.yTop - rclButtonArea.yBottom;
            LONG    y = rclButtonArea.yBottom + (cy - pGlobals->cxMiniIcon) / 2;
            if (y < rclButtonArea.yBottom + 1)
                y = rclButtonArea.yBottom + 1;

            /* WinDrawPointer(hps,
                           rclButtonArea.xLeft + 1,
                           y,
                           pCtrlThis->hptr,
                           (fHalftone)
                            ? DP_MINI | DP_HALFTONED
                            : DP_MINI);
                    V0.9.20 (2002-07-31) [umoeller]
                    replaced with
            */

            pgpihDrawPointer(hps,
                             rclButtonArea.xLeft + 1,
                             y,
                             pCtrlThis->hptr,
                             &pPrivate->szlSysIcon,
                             &rclButtonArea,            // clip rectangle
                             (fHalftone)
                              ? DP_MINI | DP_HALFTONED
                              : DP_MINI);


            rclButtonArea.xLeft += pGlobals->cxMiniIcon;
        }

        // add another pixel for the text
        rclButtonArea.xLeft += ICON_TEXT_SPACING;
                    // added macro V0.9.16 (2001-12-08) [umoeller]
        // dec right for clipping
        rclButtonArea.xRight--;

        // draw switch list title
        GpiSetColor(hps,
                    (fHalftone)
                        ? pgpihMediumRGB(pPrivate->Setup.lcolForeground,
                                         lButtonColor)
                        : pPrivate->Setup.lcolForeground);
        WinDrawText(hps,
                    strlen(pCtrlThis->swctl.szSwtitle),
                    pCtrlThis->swctl.szSwtitle,
                    &rclButtonArea,
                    0,
                    0,
                    DT_LEFT | DT_VCENTER | DT_TEXTATTRS);
    }
}

/*
 *@@ DrawAllCtrls:
 *      redraws all visible switch list controls by invoking
 *      DrawOneCtrl on them. Gets called from WwgtPaint.
 *
 *@@changed V0.9.11 (2001-04-18) [umoeller]: adjusted for new linklist
 */

STATIC VOID DrawAllCtrls(PWINLISTPRIVATE pPrivate,
                         HPS hps,
                         PRECTL prclSubclient)       // in: max available space (exclusive)
{
    LONG    lcolBackground = pPrivate->Setup.lcolBackground;

    // count of entries in switch list:
    ULONG   cEntries = plstCountItems(&pPrivate->llWinList);

    // make background color darker:
    pgpihManipulateRGB(&lcolBackground, 0.91);

    if (!cEntries)      // avoid division by zero
    {
        // draw no buttons
        WinFillRect(hps,
                    prclSubclient,
                    lcolBackground);
    }
    else
    {
        HWND    hwndActive = WinQueryActiveWindow(HWND_DESKTOP);

        LONG    lNextX = 0;
        ULONG   ulIndexThis = 0;

        PLISTNODE pNode;
        for (pNode = plstQueryFirstNode(&pPrivate->llWinList);
             pNode;
             pNode = pNode->pNext)
        {
            PWINLISTENTRY pCtrlThis = (PWINLISTENTRY)pNode->pItemData;
            // renumber entries because items might have
            // been added or removed
            pCtrlThis->ulIndex = ulIndexThis++;
            DrawOneCtrl(pPrivate,
                        hps,
                        prclSubclient,
                        pCtrlThis,
                        hwndActive,
                        &lNextX);
        }

        if (lNextX < prclSubclient->xRight)
        {
            // we have leftover space at the right:
            RECTL rcl2;
            rcl2.xLeft = lNextX;
            rcl2.yBottom = prclSubclient->yBottom;
            rcl2.xRight = prclSubclient->xRight;
            rcl2.yTop = prclSubclient->yTop;
            WinFillRect(hps,
                        &rcl2,
                        lcolBackground);
        }
    } // end if (cWins)
}

/*
 *@@ DoRedraw:
 *
 *@@added V0.9.20 (2002-08-10) [umoeller]
 */

STATIC VOID DoRedraw(HWND hwnd,
                     PWINLISTPRIVATE pPrivate,
                     PWINLISTENTRY pCtrlThis)   // in: switch list entry to paint or NULL for all
{
    HPS hps;

    // and repaint
    if (hps = WinGetPS(hwnd))
    {
        RECTL       rclSubclient;
        GetPaintableRect(pPrivate, &rclSubclient);
        pgpihSwitchToRGB(hps);

        // check if we need to redraw _everything_
        // (entry added or removed, which changes the scaling)
        if (!pCtrlThis)
            DrawAllCtrls(pPrivate,
                         hps,
                         &rclSubclient);
        else
            // redraw _one_ control only
            // (title or icon or active window changed)
            DrawOneCtrl(pPrivate,
                        hps,
                        &rclSubclient,
                        pCtrlThis,
                        WinQueryActiveWindow(HWND_DESKTOP),
                        NULL);

        WinReleasePS(hps);
    }
}

/*
 *@@ FindCtrlFromPoint:
 *      returns the SWCNTRL from the global list
 *      which matches the specified point, which
 *      must be in the widget's window coordinates.
 *
 *      Returns NULL if not found.
 *
 *@@changed V0.9.11 (2001-04-18) [umoeller]: adjusted for new linklist
 */

STATIC PWINLISTENTRY FindCtrlFromPoint(PWINLISTPRIVATE pPrivate,
                                       PPOINTL pptl,
                                       PRECTL prclSubclient)    // in: from GetPaintableRect
{
    PWINLISTENTRY pCtrl = NULL;

    if (WinPtInRect(pPrivate->pWidget->habWidget,
                    prclSubclient,
                    pptl))
    {
        // calc width for this button...
        // use standard with, except for last button
        LONG    cxRegular = 0,
                cxThis = CalcButtonCX(pPrivate,
                                      prclSubclient,
                                      NULL,
                                      &cxRegular);
        // avoid division by zero
        if (cxRegular)
        {
            // go find button with that index
            ULONG   ulButtonIndex = pptl->x / cxRegular;
            PLISTNODE pNode;
            if (pNode = plstNodeFromIndex(&pPrivate->llWinList,
                                          ulButtonIndex))
                pCtrl = (PWINLISTENTRY)pNode->pItemData;
        }
    }

    return pCtrl;
}

/*
 *@@ IsMenuItemEnabled:
 *
 */

STATIC BOOL IsMenuItemEnabled(HWND hwndMenu, USHORT usItem)
{
    BOOL brc = FALSE;
    if ((SHORT)WinSendMsg(hwndMenu,
                          MM_ITEMPOSITIONFROMID,
                          MPFROM2SHORT(usItem, TRUE),
                          0)
            != MIT_NONE)
    {
        // item exists:
        if ((USHORT)WinSendMsg(hwndMenu,
                               MM_QUERYITEMATTR,
                               MPFROM2SHORT(usItem, TRUE),
                               (MPARAM)MIA_DISABLED)
                    == 0)
            brc = TRUE;
    }

    return brc;
}

/*
 *@@ WwgtCreate:
 *      implementation for WM_CREATE.
 *
 *@@changed V0.9.11 (2001-04-18) [umoeller]: adjusted for new linklist
 *@@changed V0.9.19 (2002-05-28) [umoeller]: adjusted for new daemon interface
 */

STATIC MRESULT WwgtCreate(HWND hwnd,
                          PXCENTERWIDGET pWidget)
{
    MRESULT mrc = 0;
    // PSZ p;
    PWINLISTPRIVATE pPrivate = malloc(sizeof(WINLISTPRIVATE));
    memset(pPrivate, 0, sizeof(WINLISTPRIVATE));
    // link the two together
    pWidget->pUser = pPrivate;
    pPrivate->pWidget = pWidget;

    pxstrInit(&pPrivate->strTooltip, 0);

    plstInit(&pPrivate->llWinList,
             TRUE);         // auto-free

    // system mini icon size V0.9.20 (2002-07-31) [umoeller]
    pPrivate->szlSysIcon.cx = WinQuerySysValue(HWND_DESKTOP, SV_CXICON);
    pPrivate->szlSysIcon.cy = WinQuerySysValue(HWND_DESKTOP, SV_CYICON);

    // initialize binary setup structure from setup string
    WwgtScanSetup(pWidget->pcszSetupString,
                  &pPrivate->Setup);

    // set window font (this affects all the cached presentation
    // spaces we use)
    pwinhSetWindowFont(hwnd,
                       (pPrivate->Setup.pszFont)
                        ? pPrivate->Setup.pszFont
                        // default font: use the same as in the rest of XWorkplace:
                        : pcmnQueryDefaultFont());

    // enable context menu help
    pWidget->pcszHelpLibrary = pcmnQueryHelpLibrary();
    pWidget->ulHelpPanelID = ID_XSH_WIDGET_WINLIST_MAIN;

    // start update timer
    /* pPrivate->ulTimerID = ptmrStartXTimer(pWidget->pGlobals->pvXTimerSet,
                                          hwnd,
                                          1,
                                          250); */
            // V0.9.19 (2002-05-28) [umoeller]

    // use XWPDAEMN instead V0.9.19 (2002-05-28) [umoeller]
    pkrnSendDaemonMsg(XDM_ADDWINLISTWATCH,
                      (MPARAM)hwnd,
                      (MPARAM)WLM_WINDOWCHANGE);

    // initialize switch list data
    // (must be after XDM_ADDWINLISTWATCH so we can receive
    // the frame icons)
    ScanSwitchList(hwnd, pPrivate);

    return mrc;
}

/*
 *@@ WwgtDestroy:
 *      implementation for WM_DESTROY.
 */

STATIC VOID WwgtDestroy(HWND hwnd)
{
    PXCENTERWIDGET pWidget;
    PWINLISTPRIVATE pPrivate;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PWINLISTPRIVATE)pWidget->pUser)
       )
    {
        /* if (pPrivate->ulTimerID)
            ptmrStopXTimer(pWidget->pGlobals->pvXTimerSet,
                           hwnd,
                           pPrivate->ulTimerID); */

        // stop winlist watch V0.9.19 (2002-05-28) [umoeller]
        pkrnSendDaemonMsg(XDM_REMOVEWINLISTWATCH,
                          (MPARAM)hwnd,
                          NULL);

        if (pPrivate->hwndButtonContextMenu)
        {
            WinDestroyWindow(pPrivate->hwndButtonContextMenu);
            pPrivate->hwndButtonContextMenu = NULLHANDLE;
        }

        plstClear(&pPrivate->llWinList);      // auto-free

        WwgtClearSetup(&pPrivate->Setup);

        pxstrClear(&pPrivate->strTooltip);

        free(pPrivate);
                // pWidget is cleaned up by DestroyWidgets
    }
}

/*
 *@@ MwgtControl:
 *      implementation for WM_CONTROL.
 *
 *@@added V0.9.7 (2000-12-14) [umoeller]
 */

STATIC BOOL WwgtControl(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    BOOL brc = FALSE;

    PXCENTERWIDGET pWidget;
    PWINLISTPRIVATE pPrivate;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PWINLISTPRIVATE)pWidget->pUser)
       )
    {
        USHORT  usID = SHORT1FROMMP(mp1),
                usNotifyCode = SHORT2FROMMP(mp1);

        if (usID == ID_XCENTER_CLIENT)
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
                    pszl->cx = -1;
                    pszl->cy = 2 * WIDGET_BORDER        // thin border
                               + 2                      // space around icon
                               + pWidget->pGlobals->cxMiniIcon;
                    if ((pWidget->pGlobals->flDisplayStyle & XCS_FLATBUTTONS) == 0)
                        // raised buttons
                        pszl->cy += 2 * THICK_BUTTON_BORDER;
                    else
                        // flat buttons
                        pszl->cy += 2;      // 2*1 pixel for thin border
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
                    PCSZ pcszNewSetupString = (const char*)mp2;

                    // reinitialize the setup data
                    WwgtClearSetup(&pPrivate->Setup);
                    WwgtScanSetup(pcszNewSetupString, &pPrivate->Setup);

                    // rescan switch list, because filters might have changed
                    ScanSwitchList(hwnd, pPrivate);
                    WinInvalidateRect(pWidget->hwndWidget, NULL, FALSE);
                }
                break;
            }
        } // if (usID == ID_XCENTER_CLIENT)
        else
        {
            if (usID == ID_XCENTER_TOOLTIP)
            {
                switch (usNotifyCode)
                {
                    case TTN_NEEDTEXT:
                    {
                        PTOOLTIPTEXT    pttt = (PTOOLTIPTEXT)mp2;
                        RECTL           rclSubclient;
                        POINTL          ptlPointer;
                        PWINLISTENTRY   pCtrlClicked = NULL;

                        // find the winlist item under the mouse
                        WinQueryPointerPos(HWND_DESKTOP,
                                           &ptlPointer);
                        WinMapWindowPoints(HWND_DESKTOP,    // from
                                           hwnd,
                                           &ptlPointer,
                                           1);
                        GetPaintableRect(pPrivate, &rclSubclient);
                        pxstrClear(&pPrivate->strTooltip);

                        if (pCtrlClicked = FindCtrlFromPoint(pPrivate,
                                                             &ptlPointer,
                                                             &rclSubclient))
                            pxstrcpy(&pPrivate->strTooltip,
                                     pCtrlClicked->swctl.szSwtitle,
                                     0);
                        else
                            pxstrcpy(&pPrivate->strTooltip,
                                     pcmnGetString(ID_CRSI_WIDGET_WINDOWLIST),
                                            // "Window list",
                                     0);

                        pttt->pszText = pPrivate->strTooltip.psz;
                        pttt->ulFormat = TTFMT_PSZ;
                    }
                    break;
                }
            }
        }
    } // end if (pPrivate)

    return brc;
}

/*
 *@@ WwgtPaint:
 *      implementation for WM_PAINT.
 */

STATIC VOID WwgtPaint(HWND hwnd)
{
    PXCENTERWIDGET pWidget;
    PWINLISTPRIVATE pPrivate;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PWINLISTPRIVATE)pWidget->pUser)
       )
    {
        HPS hps;
        if (hps = WinBeginPaint(hwnd, NULLHANDLE, NULL))
        {
            RECTL       rclWin;

            // now paint frame
            WinQueryWindowRect(hwnd,
                               &rclWin);        // exclusive
            pgpihSwitchToRGB(hps);

            // if (pPrivate->pWidget->pGlobals->flDisplayStyle & XCS_SUNKBORDERS)
            {
                rclWin.xRight--;
                rclWin.yTop--;
                pgpihDraw3DFrame(hps,
                                 &rclWin,           // inclusive
                                 WIDGET_BORDER,
                                 pWidget->pGlobals->lcol3DDark,
                                 pWidget->pGlobals->lcol3DLight);

                rclWin.xLeft++;
                rclWin.yBottom++;
            }

            // now paint buttons in the middle
            DrawAllCtrls(pPrivate,
                         hps,
                         &rclWin);

            WinEndPaint(hps);
        }
    }
}

/*
 *@@ WwgtWindowChange:
 *      implementation for WLM_WINDOWCHANGE and WLM_ICONCHANGE.
 *
 *      This is our implementation msg for XDM_ADDWINLISTWATCH
 *      and gets posted from XWPDAEMN.EXE when the switch list
 *      changes. As a result, we no longer need a timer for
 *      scanning the switch list, since the daemon does that
 *      for us now.
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 *@@changed V0.9.19 (2002-06-18) [umoeller]: fixed bad button indices
 */

STATIC VOID WwgtWindowChange(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PXCENTERWIDGET pWidget;
    PWINLISTPRIVATE pPrivate;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PWINLISTPRIVATE)pWidget->pUser)
       )
    {
        HWND            hwndRefresh = NULLHANDLE;
        BOOL            fRedrawAll = FALSE;
        PWINLISTENTRY   pCtrlRedraw = NULL;

        if (msg == WLM_WINDOWCHANGE)
        {
            switch ((ULONG)mp2)
            {
                case WM_CREATE: // (HWND)mp1 was created.
                    #ifdef DEBUG_WINDOWLIST
                        _PmpfF(("WLM_WINDOWCHANGE %lX, WM_CREATE", mp1));
                    #endif
                    hwndRefresh = (HWND)mp1;
                break;

                case WM_SETWINDOWPARAMS:
                    #ifdef DEBUG_WINDOWLIST
                        _PmpfF(("WLM_WINDOWCHANGE %lX, WM_SETWINDOWPARAMS", mp1));
                    #endif
                    hwndRefresh = (HWND)mp1;
                break;

                case WM_WINDOWPOSCHANGED:
                    #ifdef DEBUG_WINDOWLIST
                        _PmpfF(("WLM_WINDOWCHANGE %lX, WM_WINDOWPOSCHANGED", mp1));
                    #endif
                    hwndRefresh = (HWND)mp1;
                break;

                case WM_DESTROY: // (HWND)mp1 was destroyed.
                {
                    PLISTNODE pNode = plstQueryFirstNode(&pPrivate->llWinList);
                    ULONG ulIndexThis = 0;
                    #ifdef DEBUG_WINDOWLIST
                        _PmpfF(("WLM_WINDOWCHANGE %lX, WM_DESTROY", mp1));
                    #endif
                    while (pNode)
                    {
                        // run thru the list, remove the item
                        // corresponding to mp1
                        if (((PWINLISTENTRY)pNode->pItemData)->swctl.hwnd == (HWND)mp1)
                        {
                            plstRemoveNode(&pPrivate->llWinList,
                                           pNode);           // auto-free
                            fRedrawAll = TRUE;
                            break;
                        }

                        pNode = pNode->pNext;
                    }

                    pCtrlRedraw = NULL;     // we've set fRedrawAll
                }
                break;

                case WM_ACTIVATE: // (HWND)mp1 was activated or deactivated.
                {
                    if (pPrivate->pCtrlActive)
                    {
                        pCtrlRedraw = pPrivate->pCtrlActive;
                        pPrivate->pCtrlActive = NULL;
                    }

                    if (!pCtrlRedraw)
                        hwndRefresh = WinQueryActiveWindow(HWND_DESKTOP);
                }
                break;
            }
        } // if (msg == WLM_WINDOWCHANGE)
        else
        {
            // WLM_ICONCHANGE:
            PLISTNODE pNode;
            if (pNode = FindSwitchNodeFromHWND(&pPrivate->llWinList,
                                               (HWND)mp1))
            {
                // redraw icon then
                pCtrlRedraw = (PWINLISTENTRY)pNode->pItemData;
                pCtrlRedraw->hptr = (HPOINTER)mp2;
                #ifdef DEBUG_WINDOWLIST
                _PmpfF(("WLM_ICONCHANGE %lX (%s), %lX",
                        mp1,
                        pCtrlRedraw->swctl.szSwtitle,
                        mp2));
                #endif
            }
        }

        if (hwndRefresh)
            // one list item to refresh: find it in the list
            pCtrlRedraw = AddOrRefreshEntry(pPrivate,
                                            hwndRefresh,
                                            &fRedrawAll);

        // do we need to repaint?
        if (pCtrlRedraw || fRedrawAll)
        {
            DoRedraw(hwnd,
                     pPrivate,
                     (fRedrawAll)
                        ? NULL
                        : pCtrlRedraw);
        }
    }
}

/*
 *@@ WwgtButton1Down:
 *      implementation for WM_BUTTON1DOWN.
 *
 *@@changed V0.9.11 (2001-04-18) [umoeller]: fixed minimized to maximized, which never worked
 *@@changed V0.9.12 (2001-05-12) [umoeller]: fixed showing hidden windows
 *@@changed V0.9.19 (2002-06-18) [umoeller]: added hide if window does not support minimize
 */

STATIC VOID WwgtButton1Down(HWND hwnd, MPARAM mp1)
{
    PXCENTERWIDGET pWidget;
    PWINLISTPRIVATE pPrivate;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PWINLISTPRIVATE)pWidget->pUser)
       )
    {
        POINTL          ptlClick;
        PWINLISTENTRY   pCtrlClicked;
        RECTL           rclSubclient;

        ptlClick.x = SHORT1FROMMP(mp1);
        ptlClick.y = SHORT2FROMMP(mp1);
        GetPaintableRect(pPrivate, &rclSubclient);

        // find the button the user clicked on
        if (pCtrlClicked = FindCtrlFromPoint(pPrivate,
                                             &ptlClick,
                                             &rclSubclient))
        {
            // check the window's state...
            SWP swp;
            BOOL fRedrawActive = FALSE;
            if (WinQueryWindowPos(pCtrlClicked->swctl.hwnd, &swp))
            {
                if (swp.fl & (SWP_HIDE | SWP_MINIMIZE))
                {
                    // window is hidden or minimized:
                    // restore and activate
                    /* if (WinSetWindowPos(pCtrlClicked->hwnd,
                                        HWND_TOP,
                                        0, 0, 0, 0,
                                        SWP_SHOW | SWP_RESTORE | SWP_ACTIVATE | SWP_ZORDER)) */

                    // the above never worked for maximized windows which
                    // were minimized... the trick is to switch to the
                    // program instead! V0.9.11 (2001-04-18) [umoeller]
                    HSWITCH hsw;
                    if (hsw = WinQuerySwitchHandle(pCtrlClicked->swctl.hwnd,
                                                   pCtrlClicked->swctl.idProcess))
                    {
                        // first check if the thing is hidden V0.9.12 (2001-05-12) [umoeller]
                        if (swp.fl & SWP_HIDE)
                            // yes: show V0.9.12 (2001-05-12) [umoeller]
                            WinSetWindowPos(pCtrlClicked->swctl.hwnd,
                                            0, 0, 0, 0, 0,
                                            SWP_SHOW);

                        if (!WinSwitchToProgram(hsw))   // this returns 0 on success... sigh
                        {
                            // OK, now we have the program active, but it's
                            // probably in the background...
                            WinSetWindowPos(pCtrlClicked->swctl.hwnd,
                                            HWND_TOP,
                                            0, 0, 0, 0,
                                            SWP_SHOW | SWP_ACTIVATE | SWP_ZORDER);
                            fRedrawActive = TRUE;
                        }
                    }
                }
                else
                {
                    // not minimized:
                    // see if it's active
                    HWND hwndActive = WinQueryActiveWindow(HWND_DESKTOP);

                    if (hwndActive == pCtrlClicked->swctl.hwnd)
                    {
                        // window is active:
                        // try minimize...
                        // we better not use WinSetWindowPos directly,
                        // because this solidly messes up some windows;
                        // SC_MINIMIZE will only minimize the window
                        // if the window actually supports that (usually
                        // the PM frame class ignores this if the window
                        // doesn't have the minimize button).

                        // V0.9.19 (2002-06-18) [umoeller]
                        // minimize or hide; the following mess will
                        // always try minimize, _unless_ we _know_
                        // that minimize is disabled and hide is
                        // enabled
                        HWND    hwndSysMenu;
                        USHORT  usSysCommand = SC_MINIMIZE;
                        if (    (hwndSysMenu = WinWindowFromID(pCtrlClicked->swctl.hwnd,
                                                               FID_SYSMENU))
                             && (!(IsMenuItemEnabled(hwndSysMenu, SC_MINIMIZE)))
                             && (IsMenuItemEnabled(hwndSysMenu, SC_HIDE))
                             && (!(swp.fl & SWP_HIDE))
                           )
                           usSysCommand = SC_HIDE;

                        if (WinPostMsg(pCtrlClicked->swctl.hwnd,
                                       WM_SYSCOMMAND,
                                       (MPARAM)usSysCommand,
                                       MPFROM2SHORT(CMDSRC_OTHER, TRUE)))
                            fRedrawActive = TRUE;
                    }
                    else
                        // not minimized, not active:
                        if (WinSetActiveWindow(HWND_DESKTOP, pCtrlClicked->swctl.hwnd))
                            fRedrawActive = TRUE;
                }
            }

            if (fRedrawActive)
            {
                HPS hps;
                if (hps = WinGetPS(hwnd))
                {
                    PWINLISTENTRY pOldActive = pPrivate->pCtrlActive;
                    pgpihSwitchToRGB(hps);

                    // store new active ctrl
                    pPrivate->pCtrlActive = pCtrlClicked;

                    if (pOldActive)
                        // un-activate old button
                        DrawOneCtrl(pPrivate,
                                    hps,
                                    &rclSubclient,
                                    pOldActive,         // old active
                                    pCtrlClicked->swctl.hwnd,     // active wnd
                                    NULL);

                    DrawOneCtrl(pPrivate,
                                hps,
                                &rclSubclient,
                                pCtrlClicked,       // new active
                                pCtrlClicked->swctl.hwnd,     // active wnd
                                NULL);

                    WinReleasePS(hps);
                }
            }
        }
    }
}

/*
 *@@ HackContextMenu:
 *
 *@@added V0.9.7 (2001-01-10) [umoeller]
 *@@changed V0.9.20 (2002-08-10) [umoeller]: cleaned up those stupid menu ids
 */

STATIC VOID HackContextMenu(PWINLISTPRIVATE pPrivate)
{
    static const ULONG aulMenuItems[] =
            {
                ID_CRSI_WINLIST_RESTORE, // "~Restore",
                ID_CRSI_WINLIST_MOVE, // "~Move",
                ID_CRSI_WINLIST_SIZE, // "~Size",
                ID_CRSI_WINLIST_MINIMIZE, // "Mi~nimize",
                ID_CRSI_WINLIST_MAXIMIZE, // "Ma~ximize",
                ID_CRSI_WINLIST_HIDE, // "~Hide",
                0, // "",             // separator
                ID_CRSI_WINLIST_CLOSE, //  "~Close",
                ID_CRSI_WINLIST_KILL1, // "~Kill process ",
                ID_CRSI_WINLIST_FILTER,               // V0.9.20 (2002-08-10) [umoeller]
                0, // ""              // separator
            };

    if (pPrivate->hwndButtonContextMenu = WinCreateMenu(HWND_DESKTOP, NULL))
    {
        // insert window-related menu items from above array
        MENUITEM mi = {0};
        SHORT src = 0;
        SHORT s = 0;
        mi.iPosition = MIT_END;

        for (s = 0;
             s < sizeof(aulMenuItems) / sizeof(aulMenuItems[0]);
             s++)
        {
            PCSZ pcsz;
            if (!(mi.id = aulMenuItems[s]))
            {
                // separator:
                mi.afStyle = MIS_SEPARATOR;
                pcsz = "";
            }
            else
            {
                mi.afStyle = MIS_TEXT;
                pcsz = pcmnGetString(mi.id);
            }

            src = SHORT1FROMMR(WinSendMsg(pPrivate->hwndButtonContextMenu,
                                          MM_INSERTITEM,
                                          (MPARAM)&mi,
                                          (MPARAM)pcsz));
        }

        // now copy the old context menu as submenu;
        // we can't just use the old item and insert it
        // because we still display the original menu if
        // the window list is empty, and PMMERGE crashes then

        // replaced V0.9.20 (2002-08-10) [umoeller]
        pwinhMergeMenus(pPrivate->hwndButtonContextMenu,
                        MIT_END,
                        pPrivate->pWidget->hwndContextMenu,
                        0);

        /*
        pwinhMergeIntoSubMenu(pPrivate->hwndButtonContextMenu,
                              MIT_END,
                              pcmnGetString(ID_CRSI_WINLIST_WIDGET), // "Window list widget",
                              2000,
                              pPrivate->pWidget->hwndContextMenu);
        */
    }
}

/*
 *@@ WwgtContextMenu:
 *      implementation for WM_CONTEXTMENU.
 *
 *      We override the default behavior of the standard
 *      widget window proc.
 *
 *@@added V0.9.8 (2001-01-10) [umoeller]
 */

STATIC MRESULT WwgtContextMenu(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    PXCENTERWIDGET pWidget;
    PWINLISTPRIVATE pPrivate;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PWINLISTPRIVATE)pWidget->pUser)
       )
    {
        if (pWidget->hwndContextMenu)
        {
            POINTL          ptlScreen,
                            ptlWidget;
            RECTL           rclSubclient;
            PWINLISTENTRY   pCtlUnderMouse = NULL;

            WinQueryPointerPos(HWND_DESKTOP, &ptlScreen);
            // convert to widget coords
            memcpy(&ptlWidget, &ptlScreen, sizeof(ptlWidget));
            WinMapWindowPoints(HWND_DESKTOP,        // from
                               hwnd,                // to (widget)
                               &ptlWidget,
                               1);

            if (!pPrivate->hwndButtonContextMenu)
            {
                // first call:
                // hack the context menu given to us
                HackContextMenu(pPrivate);
            } // if (!pPrivate->fContextMenuHacked)

            GetPaintableRect(pPrivate, &rclSubclient);
            pPrivate->hwndContextMenuShowing = NULLHANDLE;

            // did user click on window list button?
            if (pCtlUnderMouse = FindCtrlFromPoint(pPrivate,
                                                   &ptlWidget,
                                                   &rclSubclient))
            {
                // yes: draw source emphasis
                HWND    hwndSysMenu;
                BOOL    fEnableRestore = FALSE,
                        fEnableMove = FALSE,
                        fEnableSize = FALSE,
                        fEnableMinimize = FALSE,
                        fEnableMaximize = FALSE,
                        fEnableHide = FALSE,
                        fEnableKill = FALSE;
                CHAR    szMenuItem[200] = "?";

                HPS hps;
                if (hps = WinGetPS(hwnd))
                {
                    pgpihSwitchToRGB(hps);
                    pPrivate->pCtrlSourceEmphasis = pCtlUnderMouse;
                    pPrivate->pCtrlMenu = pCtlUnderMouse;
                    DrawOneCtrl(pPrivate,
                                hps,
                                &rclSubclient,
                                pCtlUnderMouse,
                                WinQueryActiveWindow(HWND_DESKTOP),
                                NULL);
                    WinReleasePS(hps);
                }

                // enable items...
                // we do a mixture of copying the state from the item's
                // system menu and from querying the window's style. This
                // is kinda sick, but we have the following limitations:
                // -- There is no easy cross-process way of getting the
                //    frame control flags to find out whether the frame
                //    actually has minimize and maximize buttons in the
                //    first place.
                // -- We can't enumerate the frame controls either because
                //    the minimize and maximize button(s) are a single control
                //    which always has the same ID (FID_MINMAX), and we don't
                //    know what it looks like.
                // -- In general, the system menu has the correct items
                //    enabled (SC_MINIMIZE and SC_MAXIMIZE). However, these
                //    items are only refreshed when the sysmenu is made
                //    visible... so we have to query QWL_STYLE as well.
                if (hwndSysMenu = WinWindowFromID(pCtlUnderMouse->swctl.hwnd,
                                                  FID_SYSMENU))
                {
                    ULONG ulStyle = WinQueryWindowULong(pCtlUnderMouse->swctl.hwnd,
                                                        QWL_STYLE);
                    if (    (ulStyle & (WS_MINIMIZED | WS_MAXIMIZED))
                         || ((ulStyle & WS_VISIBLE) == 0)
                       )
                        fEnableRestore = TRUE;

                    fEnableMove     = IsMenuItemEnabled(hwndSysMenu, SC_MOVE);
                    fEnableSize     = IsMenuItemEnabled(hwndSysMenu, SC_SIZE);

                    if ((ulStyle & WS_MINIMIZED) == 0)
                        fEnableMinimize = IsMenuItemEnabled(hwndSysMenu, SC_MINIMIZE);
                    if ((ulStyle & WS_MAXIMIZED) == 0)
                        fEnableMaximize = IsMenuItemEnabled(hwndSysMenu, SC_MAXIMIZE);
                    if ((ulStyle & WS_VISIBLE))
                        fEnableHide     = IsMenuItemEnabled(hwndSysMenu, SC_HIDE);
                }

                WinEnableMenuItem(pPrivate->hwndButtonContextMenu,
                                  ID_CRSI_WINLIST_RESTORE,     // restore
                                  fEnableRestore);
                WinEnableMenuItem(pPrivate->hwndButtonContextMenu,
                                  ID_CRSI_WINLIST_MOVE,     // move
                                  fEnableMove);
                WinEnableMenuItem(pPrivate->hwndButtonContextMenu,
                                  ID_CRSI_WINLIST_SIZE,     // size
                                  fEnableSize);
                WinEnableMenuItem(pPrivate->hwndButtonContextMenu,
                                  ID_CRSI_WINLIST_MINIMIZE,     // minimize
                                  fEnableMinimize);
                WinEnableMenuItem(pPrivate->hwndButtonContextMenu,
                                  ID_CRSI_WINLIST_MAXIMIZE,     // maximize
                                  fEnableMaximize);
                WinEnableMenuItem(pPrivate->hwndButtonContextMenu,
                                  ID_CRSI_WINLIST_HIDE,     // hide
                                  fEnableHide);

                // enable "kill process" only if this is not the WPS
                if (pCtlUnderMouse->swctl.idProcess != pdoshMyPID())
                {
                    // not WPS:
                    fEnableKill = TRUE;
                    pdrv_sprintf(szMenuItem,
                                 pcmnGetString(ID_CRSI_WINLIST_KILL2),
                                    // "~Kill process (PID 0x%lX)",
                                 pCtlUnderMouse->swctl.idProcess);
                }
                else
                    strcpy(szMenuItem,
                           pcmnGetString(ID_CRSI_WINLIST_KILL1));

                WinSetMenuItemText(pPrivate->hwndButtonContextMenu,
                                   ID_CRSI_WINLIST_KILL1,
                                   szMenuItem);
                WinEnableMenuItem(pPrivate->hwndButtonContextMenu,
                                  ID_CRSI_WINLIST_KILL1,
                                  fEnableKill);

                // set close menu item text V0.9.20 (2002-08-10) [umoeller]
                sprintf(szMenuItem,
                        pcmnGetString(ID_CRSI_WINLIST_CLOSE),   // "~Close ""%s"""
                        pCtlUnderMouse->swctl.szSwtitle);
                WinSetMenuItemText(pPrivate->hwndButtonContextMenu,
                                   ID_CRSI_WINLIST_CLOSE,
                                   szMenuItem);

                // set filter menu item text V0.9.20 (2002-08-10) [umoeller]
                sprintf(szMenuItem,
                        pcmnGetString(ID_CRSI_WINLIST_FILTER),   // ~Filter ""%s"""
                        pCtlUnderMouse->swctl.szSwtitle);
                WinSetMenuItemText(pPrivate->hwndButtonContextMenu,
                                   ID_CRSI_WINLIST_FILTER,
                                   szMenuItem);

                if (WinPopupMenu(HWND_DESKTOP,
                                 hwnd,
                                 pPrivate->hwndButtonContextMenu,
                                 ptlScreen.x,
                                 ptlScreen.y,
                                 0,
                                 PU_HCONSTRAIN | PU_VCONSTRAIN | PU_MOUSEBUTTON1
                                    | PU_MOUSEBUTTON2 | PU_KEYBOARD))
                    pPrivate->hwndContextMenuShowing = pPrivate->hwndButtonContextMenu;


                mrc = (MPARAM)TRUE;
            }
            else
                // no control under mouse:
                // call default
                mrc = pctrDefWidgetProc(hwnd, WM_CONTEXTMENU, mp1, mp2);

        } // if (pWidget->hwndContextMenu)
    }

    return mrc;
}

/*
 *@@ WwgtMenuEnd:
 *      implementation for WM_MENUEND.
 *
 *@@added V0.9.7 (2001-01-10) [umoeller]
 */

STATIC VOID WwgtMenuEnd(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    BOOL fCallDefault = TRUE;

    PXCENTERWIDGET pWidget;
    PWINLISTPRIVATE pPrivate;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PWINLISTPRIVATE)pWidget->pUser)
         && (pPrivate->hwndContextMenuShowing)
         && (pPrivate->hwndContextMenuShowing == (HWND)mp2)
       )
    {
        // this menu is ending:
        fCallDefault = FALSE;
        pPrivate->hwndContextMenuShowing = NULLHANDLE;
        pPrivate->pCtrlSourceEmphasis = NULL;
        // remove source emphasis... the win list might have
        // changed in between, so just repaint all
        WinInvalidateRect(hwnd, NULL, FALSE);
    }

    if (fCallDefault)
        pctrDefWidgetProc(hwnd, WM_MENUEND, mp1, mp2);
}

/*
 *@@ WwgtCommand:
 *
 *@@added V0.9.7 (2001-01-10) [umoeller]
 *@@changed V0.9.20 (2002-08-10) [umoeller]: cleaned up those stupid menu ids
 *@@changed V0.9.20 (2002-08-10) [umoeller]: added filtering via menu
 */

STATIC VOID WwgtCommand(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    BOOL fCallDefault = TRUE;

    PXCENTERWIDGET pWidget;
    PWINLISTPRIVATE pPrivate;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PWINLISTPRIVATE)pWidget->pUser)
         // we handle only the button menu items here, the
         // rest is handled by the def widget proc
         && (pPrivate->pCtrlMenu)
       )
    {
        USHORT usSysCommand = 0;

        fCallDefault = FALSE;

        switch ((USHORT)mp1)
        {
            case ID_CRSI_WINLIST_RESTORE:
                usSysCommand = SC_RESTORE;
            break;

            case ID_CRSI_WINLIST_MOVE:
                usSysCommand = SC_MOVE;
            break;

            case ID_CRSI_WINLIST_SIZE:
                usSysCommand = SC_SIZE;
            break;

            case ID_CRSI_WINLIST_MINIMIZE:
                usSysCommand = SC_MINIMIZE;
            break;

            case ID_CRSI_WINLIST_MAXIMIZE:
                usSysCommand = SC_MAXIMIZE;
            break;

            case ID_CRSI_WINLIST_HIDE:
                usSysCommand = SC_HIDE;
            break;

            case ID_CRSI_WINLIST_CLOSE:
                usSysCommand = SC_CLOSE;
            break;

            case ID_CRSI_WINLIST_KILL1:
                DosKillProcess(DKP_PROCESS,
                               pPrivate->pCtrlMenu->swctl.idProcess);
            break;

            case ID_CRSI_WINLIST_FILTER:      // filter V0.9.20 (2002-08-10) [umoeller]
                plstAppendItem(&pPrivate->Setup.llFilters,
                               plstStrDup(pPrivate->pCtrlMenu->swctl.szSwtitle));
                WwgtSaveSetupAndSend(hwnd, pPrivate);
                WinPostMsg(hwnd,
                           WLM_WINDOWCHANGE,
                           (MPARAM)pPrivate->pCtrlMenu->swctl.hwnd,
                           (MPARAM)WM_SETWINDOWPARAMS);
            break;

            default:
                fCallDefault = TRUE;
        }

        if (usSysCommand)
            WinPostMsg(pPrivate->pCtrlMenu->swctl.hwnd,
                       WM_SYSCOMMAND,
                       (MPARAM)usSysCommand,
                       MPFROM2SHORT(CMDSRC_OTHER,
                                    TRUE));      // mouse
    }

    if (fCallDefault)
        pctrDefWidgetProc(hwnd, WM_COMMAND, mp1, mp2);
}

/*
 *@@ WwgtPresParamChanged:
 *      implementation for WM_PRESPARAMCHANGED.
 *
 *@@changed V0.9.13 (2001-06-21) [umoeller]: changed XCM_SAVESETUP call for tray support
 *@@changed V1.0.8 (2007-08-05) [pr]: rewrote this mess @@fixes 994
 */

STATIC VOID WwgtPresParamChanged(HWND hwnd, ULONG ulAttrChanged)
{
    PXCENTERWIDGET pWidget;
    PWINLISTPRIVATE pPrivate;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PWINLISTPRIVATE)pWidget->pUser)
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

                pszFont = pwinhQueryWindowFont(hwnd);
                if (pszFont)
                {
                    // we must use local malloc() for the font
                    pPrivate->Setup.pszFont = strdup(pszFont);
                    pwinhFree(pszFont);
                }
            }
            break;

            default:
                fInvalidate = FALSE;
        }

        if (fInvalidate)
        {
            WinInvalidateRect(hwnd, NULL, FALSE);

            WwgtSaveSetupAndSend(hwnd, pPrivate);
        }
    } // end if (pPrivate)
}

/*
 *@@ fnwpWinlistWidget:
 *      window procedure for the winlist widget class.
 *
 *      There are a few rules which widget window procs
 *      must follow. See XCENTERWIDGETCLASS in center.h
 *      for details.
 *
 *      Other than that, this is a regular window procedure
 *      which follows the basic rules for a PM window class.
 */

MRESULT EXPENTRY fnwpWinlistWidget(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
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
         *      WINLISTPRIVATE for our own stuff.
         */

        case WM_CREATE:
            WinSetWindowPtr(hwnd, QWL_USER, mp1);
            if (mp1)
                mrc = WwgtCreate(hwnd, (PXCENTERWIDGET)mp1);
            else
                // stop window creation!!
                mrc = (MPARAM)TRUE;
        break;

        case WM_DESTROY:
            WwgtDestroy(hwnd);
            mrc = pctrDefWidgetProc(hwnd, msg, mp1, mp2);
        break;

        /*
         * WM_CONTROL:
         *      process notifications/queries from the XCenter.
         */

        case WM_CONTROL:
            mrc = (MPARAM)WwgtControl(hwnd, mp1, mp2);
        break;

        /*
         * WM_PAINT:
         *
         */

        case WM_PAINT:
            WwgtPaint(hwnd);
        break;

        /*
         * WM_TIMER:
         *      update timer --> check for updates.
         */

        /* case WM_TIMER:
            WwgtTimer(hwnd, mp1, mp2);
        break; */

        /*
         *@@ WLM_WINDOWCHANGE:
         *
         *@@added V0.9.19 (2002-05-28) [umoeller]
         */

        case WLM_WINDOWCHANGE:
        case WLM_ICONCHANGE:
            WwgtWindowChange(hwnd, msg, mp1, mp2);
        break;

        /*
         * WM_BUTTON1DOWN:
         *      prevent the XCenter from getting the focus...
         */

        case WM_BUTTON1DOWN:
            WwgtButton1Down(hwnd, mp1);
            mrc = (MPARAM)FALSE;
        break;

        /*
         * WM_CONTEXTMENU:
         *      modify standard context menu behavior.
         */

        case WM_CONTEXTMENU:
            mrc = WwgtContextMenu(hwnd, mp1, mp2);
        break;

        /*
         * WM_MENUEND:
         *
         */

        case WM_MENUEND:
            WwgtMenuEnd(hwnd, mp1, mp2);
        break;

        /*
         * WM_COMMAND:
         *      process menu commands.
         */

        case WM_COMMAND:
            WwgtCommand(hwnd, mp1, mp2);
        break;

        /*
         * WM_PRESPARAMCHANGED:
         *
         */

        case WM_PRESPARAMCHANGED:
            // this gets sent before this is set!
            WwgtPresParamChanged(hwnd, (ULONG)mp1);
        break;

        /*
         * WM_DESTROY:
         *      clean up. This _must_ be passed on to
         *      ctrDefWidgetProc.
         */

        default:
            mrc = pctrDefWidgetProc(hwnd, msg, mp1, mp2);
        break;
    } // end switch(msg)

    return mrc;
}

/* ******************************************************************
 *
 *   Exported procedures
 *
 ********************************************************************/

/*
 *@@ WwgtInitModule:
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
 */

ULONG EXPENTRY WwgtInitModule(HAB hab,         // XCenter's anchor block
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
                              WNDCLASS_WIDGET_WINLIST,
                              fnwpWinlistWidget,
                              CS_HITTEST | CS_PARENTCLIP | CS_SIZEREDRAW | CS_SYNCPAINT,
                              sizeof(PWINLISTPRIVATE))
                                    // extra memory to reserve for QWL_USER
                             )
            strcpy(pszErrorMsg, "WinRegisterClass failed.");
        else
        {
            // no error:
            // return classes
            *ppaClasses = G_WidgetClasses;

            // one class in this DLL:
            // no. of classes in this DLL:
            ulrc = sizeof(G_WidgetClasses) / sizeof(G_WidgetClasses[0]);
        }
    }

    return ulrc;
}

/*
 *@@ WwgtUnInitModule:
 *      optional export with ordinal 2, which can clean
 *      up global widget class data.
 *
 *      This gets called by the XCenter right before
 *      a widget DLL gets unloaded. Note that this
 *      gets called even if the "init module" export
 *      returned 0 (meaning an error) and the DLL
 *      gets unloaded right away.
 */

VOID EXPENTRY WwgtUnInitModule(VOID)
{
}

/*
 *@@ WwgtQueryVersion:
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

VOID EXPENTRY WwgtQueryVersion(PULONG pulMajor,
                               PULONG pulMinor,
                               PULONG pulRevision)
{
    *pulMajor = XFOLDER_MAJOR;              // dlgids.h
    *pulMinor = XFOLDER_MINOR;
    *pulRevision = XFOLDER_REVISION;
}

