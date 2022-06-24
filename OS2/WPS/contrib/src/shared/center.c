
/*
 *@@sourcefile center.c:
 *      shared code for the XCenter implementation. This code
 *      has been declared "shared" because many different
 *      components rely on it -- for example, the widget plugin
 *      DLLs.
 *
 *      The XCenter is a WPAbstract subclass which creates an
 *      almost standard PM frame/client pair for its default view
 *      (on XCenter::wpOpen, which in turn calls ctrpCreateXCenterView).
 *
 *      The nice thing about the XCenter (compared to the WarpCenter)
 *      is that any XCenter @widget can be dynamically replaced,
 *      even while the XCenter is open.
 *
 *      To write your own widgets, you only need to create a new
 *      standard PM window class which follows a few rules to
 *      interface with the XCenter. The XCenter is even capable
 *      of dynamically loading and unloading plugin widgets from a DLL.
 *      Best of all, it will automatically free the DLL when it's
 *      no longer used (e.g. when all XCenters are closed) so that
 *      Desktop restarts are _not_ needed for testing plugins.
 *
 *      See fnwpXCenterMainClient for a description of the XCenter
 *      window hierarchy.
 *
 *      See @xcenter_instance, @widget, @widget_class, @plugin_dll.
 *
 *      <B>Structures</B>
 *
 *      There are a bunch of structures which are used for communication
 *      between the various windows and classes.
 *
 *      -- On XCenter view creation (ctrpCreateXCenterView), an
 *         XCENTERGLOBALS structure is created in the XCenter's
 *         window words, which is later made public to the widgets
 *         as well.
 *
 *      -- Each known @widget_class (either one of the built-ins in
 *         XFLDR.DLL or those loaded from a @plugin_dll) is described
 *         in an XCENTERWIDGETCLASS structure. An array of those
 *         structures must be returned by a widget @plugin_dll so
 *         that the XCenter can know what plugin classes are in
 *         a DLL.
 *
 *      -- The XCenter instance data contains a linked list of
 *         XCENTERWIDGETSETTING structures, which each describe
 *         a widget which has been configured for display. The
 *         widgets use their own class-specific setup strings
 *         to save and restore their data. This saves the widgets
 *         from having to maintain their own data in OS2.INI.
 *         The XCenter will simply dump all the widget strings
 *         with its other data (for WPS programmers: this happens
 *         in XCenter::wpSaveState).
 *
 *      -- When an XCenter is opened, it creates its widgets.
 *         Each widget then receives an XCENTERWIDGET structure
 *         on WM_CREATE for itself.
 *
 *         This contains all kinds of data specific to the widget.
 *         Other than setting a few of these fields on WM_CREATE,
 *         the XCenter does not care much about what the widget
 *         does -- it has its own PM window and can paint there
 *         like crazy. It is however strongly recommended to
 *         pass all unprocessed messages to ctrDefWidgetProc
 *         instead of WinDefWindowProc to avoid resource leaks.
 *
 *         The widget receives a function pointer to ctrDefWidgetProc
 *         in its XCENTERWIDGET structure (on WM_CREATE). So in
 *         order to be able to pass all unprocessed messages to
 *         ctrDefWidgetProc, the first thing the widget must do
 *         on WM_CREATE is to store the pointer to its XCENTERWIDGET
 *         in its QWL_USER window word (see PMREF).
 *
 *         There is a "pUser" pointer in that structure that the
 *         widget can use for allocating its own data.
 *
 *         For details, see XCENTERWIDGETCLASS and XCENTERWIDGET.
 *
 *      <B>Widget settings</B>
 *
 *      Again, each widget can store its own data in its window
 *      words (by using the pUser field of XCENTERWIDGET).
 *      This is OK while the widget is running...
 *
 *      Saving the settings is a bit more tricky because there
 *      can be many widgets in many XCenters. Even though a
 *      widget programmer may choose to use a fixed location
 *      in OS2.INI, for example, for saving widget settings,
 *      this isn't such a good idea.
 *
 *      The XCenter offers widgets to store their data together
 *      with the XCenter instance settings as a "setup string".
 *      These look similar to regular WPS setup strings (in the
 *      "keyword1=value1" form), even though the widgets are not
 *      Desktop objects themselves.
 *
 *      Since the XCenter is derived from WPAbstract, if a widget
 *      chooses to do so, its instance data ends up somewhere in
 *      OS2.INI (thru the regular wpSaveState/wpRestoreState
 *      mechanism). The XCenter takes care of unpacking the setup
 *      strings for the widgets, so the widget should only be
 *      able to do two things:
 *
 *      --  to parse a setup string and set up its binary data
 *          in XCENTERWIDGET on WM_CREATE (and possibly later,
 *          when a settings dialog changes the setup string);
 *
 *      --  to create a new setup string from its binary data
 *          to have that data saved.
 *
 *      XFLDR.DLL exports a few helper functions that plugins can
 *      import to aid in that (see ctrScanSetupString,
 *      ctrParseColorString, ctrFreeSetupValue, and ctrSetSetupString).
 *
 *      Using setup strings has the following advantages:
 *
 *      --  The data is stored together with the XCenter instance.
 *          When the XCenter gets deleted, your data is cleaned
 *          up automatically.
 *
 *      --  The XCenter can produce a single WPS setup string
 *          for itself so that the same XCenter with all its
 *          widgets can be recreated somewhere else.
 *
 *      XCenter also offers support for widget settings dialogs
 *      which work on both open widget settings and from the
 *      XCenter settings notebook itself (even if the XCenter
 *      is not currently open). Such settings dialogs operate
 *      on setup strings only. See WIDGETSETTINGSDLGDATA for
 *      details.
 *
 *      WARNING: As with all WPS objects, the total instance data
 *      for each XCenter is limited to 64K due to the dull Prf*
 *      limitations. Since all the widget setup strings go into
 *      the XCenter instance data, do _not_ use excessively long
 *      setup strings.
 *
 *      <B>Module handling</B>
 *
 *      Any XCenter @plugin_dll will get dynamically loaded and
 *      unloaded when XCenters are opened and closed. As a general
 *      rule, all plug-in DLLs get loaded when the first XCenter
 *      is opened and unloaded when the last XCenter is closed.
 *      As a result, you can simply close all open XCenters to
 *      unlock your plug-in DLL.
 *
 *      However, this will not work if you register an exit list
 *      handler in your DLL because DosFreeModule then fails.
 *      This appears to be a limitation in the OS/2 loader, or
 *      maybe PM is involved somewhere too, who knows. So do not
 *      register exit list handlers please.
 *
 *      Each plug-in DLL must export at least two, preferrably
 *      three functions with the ordinals 1, 2, and 3. See the
 *      bottom of include\shared\center.h for the prototypes.
 *
 *      --  Ordinal 1 gets called after the DLL has been loaded.
 *          This "init module" export must register the PM classes
 *          for the widget classes in the module and return an
 *          array of XCENTERWIDGETCLASS structures to the XCenter.
 *
 *          See WgtInitModule in src\widgets\____sample.c for
 *          the prototype and additional information.
 *
 *      --  Ordinal 2 gets called right before the DLL gets
 *          unloaded. This can be used to free global data that
 *          you might have allocated.
 *
 *          See WgtUnInitModule in src\widgets\____sample.c for
 *          the prototype and additional information.
 *
 *      --  Ordinal 3 is optional, but strongly recommended,
 *          to tell the XCenter the minimum XCenter version that
 *          is required for the DLL to work. This is to protect
 *          the DLL from being run on systems that are outdated.
 *
 *          See WgtQueryVersion in src\widgets\____sample.c for
 *          the prototype and additional information.
 *
 *      <B>Importing functions</B>
 *
 *      The only function that a widget is really required to
 *      use is ctrDefWidgetProc, whose address is passed to it
 *      on WM_CREATE in the XCENTERWIDGET structure. So there's
 *      no need to import additional functions from XFLDR.DLL.
 *
 *      However, to reduce the DLL's code size, a widget plugin
 *      DLL may import any function that is exported from XFLDR.DLL.
 *      (See the first part of the EXPORTS section in src\shared\xwp.def
 *      for a list of exported functions). In the "init module"
 *      export that is required for plugin DLLs, the plugin DLL
 *      receives the module handle of XFLDR.DLL that it can use
 *      with DosQueryProcAddr to receive a function pointer. See
 *      the samples in src\widgets for how this is done.
 *
 *      Since many of the helpers functions use the VAC _Optlink
 *      calling convention, this will presently only work with
 *      IBM VAC. However, the ctr* exports have APIENTRY, so you
 *      should be able to use those with any compiler.
 *
 *      <B>Where is what?</B>
 *
 *      This file (shared\center.c) "only" contains things which
 *      are of interest for widget DLLs. This has mostly
 *      functions which are exported from XFLDR.DLL so that plugins
 *      can import them for convenience. Besides, this has the
 *      default widget window proc (ctrDefWidgetProc).
 *
 *      The "engine" of the XCenter which does all the hard stuff
 *      (settings management, window creation, window reformatting,
 *      DLL management, etc.) is in xcenter\ctr_engine.c. You
 *      better not touch that if you only want to write a plugin.
 *
 *      Function prefix for this file:
 *      --  ctr*
 *
 *      This is all new with V0.9.7.
 *
 *@@added V0.9.7 (2000-11-27) [umoeller]
 *@@header "shared\center.h"
 */

/*
 *@@gloss: xcenter_instance XCenter instance
 *      An "XCenter instance" is an object of the XCenter WPS
 *      class. There can only be one open view per XCenter so
 *      that an XCenter instance is identical to an XCenter
 *      view when it's open, but there can be several XCenter
 *      instances on the system (i.e. one for each screen border).
 */

/*
 *@@gloss: widget widget
 *      A "widget" is a PM subwindow of an open XCenter which
 *      displays something. An open XCenter can contain zero,
 *      one, or many widgets. Every XCenter keeps a list of
 *      widgets together with their settings in its SOM instance
 *      data. This is totally transparent to the widgets.
 *
 *      Each widget is defined by its @widget_class.
 */

/*
 *@@gloss: widget_class widget class
 *      A "widget class" defines a @widget. Basically, it's a
 *      plain C structure (XCENTERWIDGETCLASS) with a PM window
 *      procedure which is used to create the PM widget windows
 *      (the "widget instances"). Depending on the widget
 *      class's attributes, there can be one or several
 *      instances of a widget class. If you want a different
 *      widget in the XCenter, you need to write a widget class.
 *
 *      A widget class basically boils down to writing a PM
 *      window class, with some additional rules to follow. To
 *      make things easier, several widget classes can share
 *      the same PM class though.
 *
 *      Some widget classes are built into the XCenter itself
 *      (i.e. reside in XFLDR.DLL), but the XCenter can load
 *      them from a @plugin_dll too. Several of the widget classes
 *      that come with XWorkplace have been created as plug-ins to
 *      show you how it's done (see the src\widgets directory).
 */

/*
 *@@gloss: plugin_dll plug-in DLL
 *      A "widget plug-in DLL" is a separate DLL which resides
 *      in the "plugins\xcenter" directory of the XWorkplace
 *      installation directory. This must contain one or
 *      several widget classes. There are a couple of
 *      procedures that a widget plug-in DLL is required to
 *      export to make the XCenter see the widget classes.
 */

/*
 *      Copyright (C) 2000-2006 Ulrich M”ller.
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
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WININPUT
#define INCL_WINPOINTERS
#define INCL_WINMENUS
#define INCL_WINSYS

#define INCL_GPICONTROL
#define INCL_GPIPRIMITIVES
#define INCL_GPILOGCOLORTABLE
#define INCL_GPIREGIONS
#include <os2.h>

// C library headers
#include <stdio.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\except.h"             // exception handling
#include "helpers\gpih.h"               // gpi helper
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\stringh.h"            // string helper routines
#include "helpers\timer.h"              // replacement PM timers
#include "helpers\winh.h"               // win helper

// SOM headers which don't crash with prec. header files
#include "xcenter.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\errors.h"              // private XWorkplace error codes

#include "shared\center.h"              // public XCenter interfaces
#include "xcenter\centerp.h"            // private XCenter implementation

// #include "filesys\object.h"             // XFldObject implementation

#pragma hdrstop                     // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Public widget helpers (exported from XFLDR.DLL)
 *
 ********************************************************************/

/*
 *@@ ctrScanSetupString:
 *      helper function exported from XFLDR.DLL.
 *
 *      A widget can use this helper to get the value
 *      which has been set with a keyword in a widget
 *      setup string.
 *
 *      Returns the value for the keyboard in a new
 *      buffer (which should be freed using ctrFreeSetupValue),
 *      or NULL if not found.
 *
 *      This assumes that semicola (';') chars are used to
 *      separate the key/value pairs. You are not required
 *      to use that, but this function won't work otherwise,
 *      and you'll have to write your own parser then.
 *
 *      Example:
 +
 +          PSZ pszSetupString = "key1=value1;key2=value2;"
 +          PSZ pszValue = ctrScanSetupString(pszSetupString,
 +                                            "key1");
 +          ...
 +          ctrFreeSetupValue(pszValue);
 *
 *      would have returned "value1".
 *
 *      This searches for the keyword _without_ respect
 *      to case. It is recommended to use upper-case
 *      keywords only.
 */

PSZ ctrScanSetupString(PCSZ pcszSetupString, // in: entire setup string
                       PCSZ pcszKeyword)     // in: keyword to look for (e.g. "FONT")
{
    PSZ pszValue = 0;
    if ((pcszSetupString) && (pcszKeyword))
    {
        ULONG       ulKeywordLen = strlen(pcszKeyword);
        const char  *pKeywordThis = pcszSetupString,
                    *pEquals = strchr(pcszSetupString, '=');

        while (pEquals)
        {
            if (!strnicmp(pKeywordThis, pcszKeyword, ulKeywordLen))
            {
                // keyword found:
                // get value
                PCSZ pEOValue;
                if (pEOValue = strchr(pEquals, ';'))
                    // value is before another separator:
                    pszValue = strhSubstr(pEquals + 1, pEOValue);
                else
                    // ";" not found: use rest of string (0 byte)
                    pszValue = strdup(pEquals + 1);

                // we got what we need; exit
                break;
            }

            // else not our keyword:
            // go on
            if (!(pKeywordThis = strchr(pEquals, ';')))
                // was last keyword:
                break; // while
            else
            {
                // not last keyword: search on after separator
                pKeywordThis++;
                pEquals = strchr(pKeywordThis, '=');
            }

        } // end while (pEquals)
    }

    return pszValue;
}

/*
 *@@ ctrParseColorString:
 *      helper function exported from XFLDR.DLL.
 *
 *      Returns an RGB LONG value for the color
 *      string pointed to by p. That value can
 *      be directly used with an HPS in RGB mode
 *      (see GpiCreateLogColorTable).
 *
 *      "p" is expected to point to a six-digit
 *      hex string in the "RRGGBB" format.
 */

LONG ctrParseColorString(PCSZ p)
{
    if (p)
        return strtol(p, NULL, 16);

    return 0;
}

/*
 *@@ ctrFreeSetupValue:
 *      helper function exported from XFLDR.DLL.
 *
 *      Frees a value returned by ctrScanSetupString.
 */

VOID ctrFreeSetupValue(PSZ p)
{
    free(p);
}

/*
 *@@ ctrSetSetupString:
 *      helper function exported from XFLDR.DLL.
 *
 *      This helper must be invoked to change the
 *      setup string for a widget in the context
 *      of a widget settings dialog.
 *
 *      See WIDGETSETTINGSDLGDATA for more about
 *      settings dialogs.
 *
 *      This only works while a widget settings
 *      dialog is open. This separate function is
 *      needed because a settings dialog can be
 *      open even if the XCenter is not, and sending
 *      the XCM_SAVESETUP message wouldn't work then.
 *
 *      hSetting is passed to a setup dialog procedure
 *      in its WIDGETSETTINGSDLGDATA structure. This
 *      identifies the widget settings internally and
 *      better be valid.
 *
 *      If the affected widget is currently open, it
 *      will be sent a WM_CONTROL message with the
 *      XN_SETUPCHANGED notification code.
 *
 *@@added V0.9.7 (2000-12-07) [umoeller]
 *@@changed V0.9.11 (2001-04-25) [umoeller]: adjusted for new ctrpShowSettingsDlg
 */

BOOL ctrSetSetupString(LHANDLE hSetting,
                       PCSZ pcszNewSetupString)
{
    BOOL brc = FALSE;
    // get pointer to structure on stack in ShowSettingsDlg;
    // this identifies the setting...
    PWGTSETTINGSTEMP pSettingsTemp;
    PPRIVATEWIDGETSETTING pSetting;
    if (    (pSettingsTemp = (PWGTSETTINGSTEMP)hSetting)
         && (pSetting = pSettingsTemp->pSetting)
       )
    {
        // change setup string in the settings structure
        if (pSetting->Public.pszSetupString)
            // we already had a setup string:
            free(pSetting->Public.pszSetupString);

        pSetting->Public.pszSetupString = strhdup(pcszNewSetupString, NULL);
                    // can be NULL

        brc = TRUE;

        // do we have an open view?
        if (pSettingsTemp->pWidget)
        {
            // yes:
            // send notification
            WinSendMsg(pSettingsTemp->pWidget->hwndWidget,
                       WM_CONTROL,
                       MPFROM2SHORT(ID_XCENTER_CLIENT,
                                    XN_SETUPCHANGED),
                       (MPARAM)pSetting->Public.pszSetupString);
        }

        _wpSaveDeferred(pSettingsTemp->somSelf);
    }

    return brc;
}

/*
 *@@ ctrDisplayHelp:
 *      helper function exported from XFLDR.DLL.
 *
 *      Displays the given help panel in the given
 *      help file. This saves widgets from having
 *      to maintain their own help instance.
 *
 *@@added V0.9.7 (2000-12-07) [umoeller]
 */

BOOL ctrDisplayHelp(PCXCENTERGLOBALS pGlobals,
                    PCSZ pcszHelpFile,       // in: help file (can be q'fied)
                    ULONG ulHelpPanelID)
{
    BOOL brc = FALSE;
    if (pGlobals)
    {
        PXCENTERWINDATA pXCenterData;
        if (pXCenterData = (PXCENTERWINDATA)WinQueryWindowPtr(pGlobals->hwndClient,
                                                              QWL_USER))
        {
            brc = _wpDisplayHelp(pXCenterData->somSelf,
                                 ulHelpPanelID,
                                 (PSZ)pcszHelpFile);
        }
    }

    return brc;
}

/*
 *@@ ctrPaintStaticWidgetBorder:
 *      helper function exported from XFLDR.DLL.
 *
 *      This is useful if your widget is a static widget.
 *
 *      This function paints (if needed) a static widget border
 *      around the widget.  It uses the appropriate colors.
 *
 *      This function is generaly called when handling the
 *      WP_PAINT message in the widget's window proc.
 *
 *@@added V0.9.14 (2001-07-28) [lafaix]
 */

VOID ctrPaintStaticWidgetBorder(HPS hps,
                                PXCENTERWIDGET pWidget)
{
    if (hps && pWidget)
    {
        RECTL       rclWin;
        LONG lDark, lLight;
        HWND hwnd = pWidget->hwndWidget;

        WinQueryWindowRect(hwnd, &rclWin);        // exclusive
        gpihSwitchToRGB(hps);

        rclWin.xRight--;
        rclWin.yTop--;
            // rclWin now inclusive

        if (pWidget->pGlobals->flDisplayStyle & XCS_SUNKBORDERS)
        {
            lDark = pWidget->pGlobals->lcol3DDark;
            lLight = pWidget->pGlobals->lcol3DLight;
        }
        else
        {
            lDark =
            lLight = winhQueryPresColor(hwnd,
                                        PP_BACKGROUNDCOLOR,
                                        TRUE,
                                        SYSCLR_DIALOGBACKGROUND);
        }

        gpihDraw3DFrame2(hps,
                         &rclWin,        // inclusive
                         1,
                         lDark,
                         lLight);
    }
}

/*
 *@@ ctrPlaceAndPopupMenu:
 *      little helper func which pop up a menu above or below
 *      the specified location.
 *
 *      This is useful if you want to display menus from widgets.
 *
 *      (This does not work with current folder content menus as
 *       they build their content during WM_INITMENU, i.e., during
 *       the WinPopupMenu call.)
 *
 *@@added V0.9.14 (2001-07-24) [lafaix]
 */

VOID ctrPlaceAndPopupMenu(HWND  hwndOwner,
                          HWND  hwndMenu,
                          BOOL  fAbove)
{
    SWP   swp;
    ULONG ulStyle;
    RECTL rclButton;

    // get the owner size, so that we can position the menu accordingly
    WinQueryWindowRect(hwndOwner, &rclButton);

    if (!fAbove)
    {
        // the menu is to be positionned below the owner; we must
        // compute the menu height

        // ensure that the menu is a popup menu, not an action bar
        ulStyle = WinQueryWindowULong(hwndMenu, QWL_STYLE) & ~MS_ACTIONBAR;

        // we _must_ force MS_POPUP style, or else separators will
        // be considered to have the same height as normal items
        // (@#!&@)

        #ifndef MS_POPUP
            #define MS_POPUP 0x00000010L
        #endif

        ulStyle |= MS_POPUP;
        WinSetWindowULong(hwndMenu, QWL_STYLE, ulStyle);

        // setting fl to SWP_MOVE | SWP_SIZE tells PM that it needs
        // to calculate a new size and position for the menu
        WinQueryWindowPos(hwndMenu, &swp);
        swp.fl = SWP_MOVE | SWP_SIZE;
        swp.hwndInsertBehind = HWND_TOP;

        // we need to set the owner here, so that WM_MEASUREITEM messages
        // are properly handled
        WinSetOwner(hwndMenu, hwndOwner);
        WinSendMsg(hwndMenu, WM_ADJUSTWINDOWPOS, (MPARAM)&swp, NULL);
    }

    WinPopupMenu(hwndOwner,
                 hwndOwner,
                 hwndMenu,
                 0,
                 (fAbove) ? rclButton.yTop : rclButton.yBottom - swp.cy,
                 0,
                 PU_NONE
                     | PU_MOUSEBUTTON1
                     | PU_KEYBOARD
                     | PU_HCONSTRAIN
                     | PU_VCONSTRAIN);
}

/*
 *@@ ctrShowContextMenu:
 *      little helper func which allows you to display a
 *      popup menu on a widget at the mouse position.
 *
 *      This is useful if you intercept WM_CONTEXTMENU
 *      in your widget window proc to show a context
 *      menu other than the standard one.
 *
 *      This function adds emphasis around the widget
 *      (as with the standard context menu) and then
 *      displays the specified menu using WinPopupMenu.
 *      The widget window is specified as the owner
 *      and will thus receive WM_COMMAND's from menu
 *      selections.
 *
 *      NOTE: It is the caller's responsibility to
 *      remove the emphasis again when the context menu
 *      is dismissed. This is most easily done by
 *      also intercepting WM_MENUEND and checking for
 *      hwndContextMenu. To remove the emphasis, invalidate
 *      the XCenter client:
 *
 +          WinInvalidateRect(pWidget->pGlobals->hwndClient, NULL, FALSE);
 *
 *@@added V0.9.9 (2001-03-07) [umoeller]
 */

VOID ctrShowContextMenu(PXCENTERWIDGET pWidget,
                        HWND hwndContextMenu)
{
    HWND hwndClient = pWidget->pGlobals->hwndClient;
    PXCENTERWINDATA pXCenterData;
    if (pXCenterData = (PXCENTERWINDATA)WinQueryWindowPtr(hwndClient, QWL_USER))
    {
        POINTL  ptl;
        WinQueryPointerPos(HWND_DESKTOP, &ptl);

        // draw source emphasis around widget
        ctrpDrawEmphasis(pXCenterData,
                         FALSE,     // draw, not remove emphasis
                         NULL,
                         pWidget->hwndWidget,
                         NULLHANDLE);   // standard PS

        // show menu!!
        WinPopupMenu(HWND_DESKTOP,
                     pWidget->hwndWidget,
                     hwndContextMenu,
                     ptl.x,
                     ptl.y,
                     0,
                     PU_HCONSTRAIN | PU_VCONSTRAIN | PU_MOUSEBUTTON1
                        | PU_MOUSEBUTTON2 | PU_KEYBOARD);
    }
}

/*
 *@@ ctrDrawWidgetEmphasis:
 *      helper function to draw emphasis around a widget.
 *
 *@@added V0.9.13 (2001-06-19) [umoeller]
 */

VOID ctrDrawWidgetEmphasis(PXCENTERWIDGET pWidget,
                           BOOL fRemove)            // in: if TRUE, emphasis is removed
{
    HWND hwndClient = pWidget->pGlobals->hwndClient;
    PXCENTERWINDATA pXCenterData = (PXCENTERWINDATA)WinQueryWindowPtr(hwndClient,
                                                                      QWL_USER);
    HPS hps;
    if (hps = DrgGetPS(hwndClient))
    {
        gpihSwitchToRGB(hps);
        // draw source emphasis around widget
        ctrpDrawEmphasis(pXCenterData,
                         fRemove,
                         NULL,
                         pWidget->hwndWidget,
                         hps);
        DrgReleasePS(hps);
    }
}

/* ******************************************************************
 *
 *   Default widget window proc (exported from XFLDR.DLL)
 *
 ********************************************************************/

/*
 *@@ DwgtContextMenu:
 *      implementation for WM_CONTEXTMENU in ctrDefWidgetProc.
 *
 *@@changed V0.9.13 (2001-06-23) [umoeller]: added WGTF_TRANSPARENT support
 *@@changed V0.9.14 (2001-08-05) [lafaix]: optimized WGTF_TRANSPARENT support
 */

STATIC VOID DwgtContextMenu(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    PXCENTERWIDGET pWidget;
    if (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
    {
        // if transparent, use the parent context menu instead
        if (    (pWidget->ulClassFlags & WGTF_TRANSPARENT)
             && (WinSendMsg(pWidget->hwndWidget,
                            WM_CONTROL,
                            MPFROM2SHORT(ID_XCENTER_CLIENT,
                                         XN_HITTEST),
                            mp1) == FALSE)
           )
        {
            // we must adjust the mouse coordinate so that the
            // popup menu is well positioned for the parent window
            SWP swp;

            WinQueryWindowPos(hwnd, (PSWP)&swp);

            WinSendMsg(WinQueryWindow(hwnd, QW_PARENT),
                       WM_CONTEXTMENU,
                       MPFROM2SHORT(SHORT1FROMMP(mp1) + swp.x,
                                    SHORT2FROMMP(mp1) + swp.y),
                       mp2);
        }
        else
            ctrShowContextMenu(pWidget,
                               pWidget->hwndContextMenu);
    }
}

/*
 *@@ DwgtMenuEnd:
 *      implementation for WM_MENUEND in ctrDefWidgetProc.
 */

STATIC VOID DwgtMenuEnd(HWND hwnd,
                        HWND hwndMenu)
{
    PXCENTERWIDGET pWidget;
    if (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
    {
        if (hwndMenu == pWidget->hwndContextMenu)
            WinInvalidateRect(WinQueryWindow(hwnd, QW_PARENT),
                              NULL,
                              FALSE);
    }
}

/*
 *@@ DwgtCommand:
 *      implementation for WM_COMMAND in ctrDefWidgetProc.
 *
 *@@changed V0.9.19 (2002-04-14) [umoeller]: added "Properties" and "Close" items to def. widget context menu
 *@@changed V0.9.20 (2002-08-08) [umoeller]: added confirmations for delete
 *@@changed V1.0.0 (2002-08-12) [umoeller]: confirmations for delete never worked, fixed
 */

STATIC VOID DwgtCommand(HWND hwnd,
                        USHORT usCmd)
{
    PXCENTERWIDGET  pWidget;
    HWND            hwndClient;
    PXCENTERWINDATA pXCenterData;

    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (hwndClient = pWidget->pGlobals->hwndClient)
         && (pXCenterData = (PXCENTERWINDATA)WinQueryWindowPtr(hwndClient, QWL_USER))
       )
    {
        switch (usCmd)
        {
            case ID_CRMI_PROPERTIES:
            {
                // widget has a settings dialog:
                // PPRIVATEWIDGETVIEW pOwningTray = ((PPRIVATEWIDGETVIEW)pWidget)->pOwningTray;
                WIDGETPOSITION Pos;
                if (!ctrpQueryWidgetIndexFromHWND(pXCenterData->somSelf,
                                                  pWidget->hwndWidget,
                                                  &Pos))
                {
                    // have the widget show it with the XCenter frame
                    // as its owner
                    ctrpShowSettingsDlg(pXCenterData->somSelf,
                                        pXCenterData->Globals.hwndFrame,
                                        &Pos);
                            // adjusted V0.9.11 (2001-04-25) [umoeller]
                            // V0.9.14 (2001-08-01) [umoeller]
                }
            }
            break;

            case ID_CRMI_HELP:
                if (    (pWidget->pcszHelpLibrary)
                     && (pWidget->ulHelpPanelID)
                   )
                {
                    // widget has specified help itself:
                    _wpDisplayHelp(pXCenterData->somSelf,
                                   pWidget->ulHelpPanelID,
                                   (PSZ)pWidget->pcszHelpLibrary);
                }
                else
                    // fall through, avoid compiler warning
                    goto xcenterhelp;
            break;

            case ID_CRMI_HELP_XCENTER:
xcenterhelp:
            {
                // use XCenter help
                ULONG ulPanel;
                CHAR szHelp[CCHMAXPATH];
                if (_wpQueryDefaultHelp(pXCenterData->somSelf,
                                        &ulPanel,
                                        szHelp))
                    _wpDisplayHelp(pXCenterData->somSelf,
                                   ulPanel,
                                   szHelp);
            }
            break;

            /*
             * ID_CRMI_REMOVEWGT:
             *      "remove widget" menu item.
             *
             *      Works for both subwidgets and root widgets
             *      now. V0.9.19 (2002-05-04) [umoeller]
             */

            case ID_CRMI_REMOVEWGT:
            {
                // V0.9.19 (2002-05-04) [umoeller]
                WIDGETPOSITION Pos;
                if (!ctrpQueryWidgetIndexFromHWND(pXCenterData->somSelf,
                                                  hwnd,
                                                  &Pos))
                {
                    PCXCENTERWIDGETCLASS pClass;
                    PCSZ pcszClassTitle;
                    // added confirmation V0.9.20 (2002-08-08) [umoeller]
                    if (    (!(pWidget->ulClassFlags & WGTF_CONFIRMREMOVE))
                            // confirm remove enabled:
                            // get the clear widget class name
                         || (    (!ctrpFindClass(pWidget->pcszWidgetClass,
                                                 FALSE,       // fMustBeTrayable
                                                 &pClass))
                              && (pcszClassTitle = pClass->pcszClassTitle)
                              // && (DID_YES == cmnMessageBoxExt(hwnd,
                                    // wrong V1.0.0 (2002-08-12) [umoeller]
                              && (MBID_YES == cmnMessageBoxExt(hwnd,
                                                              243, // Remove Widget
                                                              &pcszClassTitle,
                                                              1,
                                                              244, // Are you sure you want to remove this widget of the class %1?
                                                              MB_YESNO))
                            )
                       )
                        _xwpDeleteWidget(pXCenterData->somSelf,
                                         &Pos);
                }
            }
            break;

            case ID_CRMI_XCSUB_PROPERTIES:  // V0.9.19 (2002-04-17) [umoeller]
                _wpViewObject(pXCenterData->somSelf, NULLHANDLE, OPEN_SETTINGS, 0);
            break;

            case ID_CRMI_XCSUB_CLOSE:       // V0.9.19 (2002-04-17) [umoeller]
                WinPostMsg(pXCenterData->Globals.hwndFrame,
                           WM_SYSCOMMAND,
                           (MPARAM)SC_CLOSE, 0);
            break;
        }
    }
}

/*
 *@@ DwgtBeginDrag:
 *      implementation for WM_BEGINDRAG in ctrDefWidgetProc.
 *
 *@@added V0.9.9 (2001-03-09) [umoeller]
 */

STATIC MRESULT DwgtBeginDrag(HWND hwnd, MPARAM mp1)
{
    PXCENTERWIDGET pWidget;
    if (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
    {
        HWND hwndDrop = ctrpDragWidget(hwnd,
                                       pWidget);
    }

    return (MPARAM)TRUE;
}

/*
 *@@ DwgtDestroy:
 *      implementation for WM_DESTROY in ctrDefWidgetProc.
 *
 *      This also frees the XCENTERWIDGET/PRIVATEWIDGETVIEW
 *      that was allocated by ctrpCreateWidgetWindow.
 *
 *      If the widget is a "root" widget, the XCENTERWIDGET
 *      is also removed from pXCenterData->llWidgets.
 *      However, if the widget is in a tray, it is NOT
 *      removed from the widget views list in the tray;
 *      the tray must take care of that before calling
 *      WinDestroyWindow on the widget.
 */

STATIC VOID DwgtDestroy(HWND hwnd)
{
    PXCENTERWIDGET pWidget;
    HWND hwndClient;
    PXCENTERWINDATA pXCenterData;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (hwndClient = pWidget->pGlobals->hwndClient)
         && (pXCenterData = (PXCENTERWINDATA)WinQueryWindowPtr(hwndClient,
                                                               QWL_USER))
       )
    {
        WPObject *pobjLock = NULL;
        TRY_LOUD(excpt1)
        {
            if (pobjLock = cmnLockObject(pXCenterData->somSelf))
            {
                // stop all running timers
                // tmrStopAllTimers(hwnd);

                // remove the widget from the list of open
                // views in the XCenter, but only if this is
                // not a subwidget in a tray;
                // XCENTERWIDGET is first member in PRIVATEWIDGETVIEW,
                // so this typecast works
                if (!((PPRIVATEWIDGETVIEW)pWidget)->pOwningTrayWidget)
                    if (!lstRemoveItem(&pXCenterData->llWidgets,
                                       pWidget))
                        cmnLog(__FILE__, __LINE__, __FUNCTION__,
                               "lstRemoveItem failed.");

                if (pWidget->pcszWidgetClass)
                {
                    free((PSZ)pWidget->pcszWidgetClass);
                    pWidget->pcszWidgetClass = NULL;
                }

                winhDestroyWindow(&pWidget->hwndContextMenu);

                // if the widget was registered with the
                // tooltip control, remove it
                // V0.9.13 (2001-06-21) [umoeller]
                if (pWidget->ulClassFlags & WGTF_TOOLTIP)
                {
                    TOOLINFO ti = {0};
                    ti.hwndToolOwner = pWidget->pGlobals->hwndClient;
                    ti.hwndTool = hwnd;
                    WinSendMsg(pWidget->pGlobals->hwndTooltip,
                               TTM_DELTOOL,
                               (MPARAM)0,
                               &ti);
                }

                free(pWidget);
                WinSetWindowPtr(hwnd, QWL_USER, 0);

                WinPostMsg(hwndClient,
                           XCM_REFORMAT,
                           (MPARAM)XFMF_REPOSITIONWIDGETS,
                           0);
            }
        }
        CATCH(excpt1) {} END_CATCH();

        if (pobjLock)
            _wpReleaseObjectMutexSem(pobjLock);
    }
}

/*
 *@@ DwgtRender:
 *      implementation for DM_RENDER in ctrDefWidgetProc.
 *
 *      This performs source rendering for the single case
 *      that a widget is dropped on a folder and a widget
 *      file thus needs to be created.
 *
 *      See ctrpDragWidget for an introduction to this
 *      mess.
 *
 *@@added V0.9.14 (2001-07-31) [lafaix]
 *@@changed V0.9.14 (2001-08-01) [umoeller]: didn't work for tray subwidgets, fixed
 *@@changed V0.9.19 (2002-06-08) [umoeller]: fixed major leaks
 *@@changed V1.0.6 (2006-08-10) [erdmann]: dragging from XCenter to WPS caused crash @@fixes 806
 */

STATIC BOOL DwgtRender(HWND hwnd,
                       PDRAGTRANSFER pdt)
{
    PXCENTERWIDGET  pWidget;
    BOOL            brc = FALSE;
    HWND            hwndClient;
    PXCENTERWINDATA pXCenterData;

    _PmpfF(("entering"));

    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (DrgVerifyRMF(pdt->pditem, "DRM_OS2FILE", NULL))
         && (hwndClient = pWidget->pGlobals->hwndClient)
         && (pXCenterData = (PXCENTERWINDATA)WinQueryWindowPtr(hwndClient, QWL_USER))
       )
    {
        XCenter        *somSelf = pXCenterData->somSelf;
        WPObject *pobjLock = NULL;

        TRY_LOUD(excpt1)
        {
            if (pobjLock = cmnLockObject(somSelf))
            {
                ULONG ulTrayWidgetIndex = 0,
                      ulTrayIndex = 0,
                      ulWidgetIndex = 0;
                PPRIVATEWIDGETSETTING pSetting;
                WIDGETPOSITION Pos;
                if (    (!ctrpQueryWidgetIndexFromHWND(pXCenterData->somSelf,
                                                       pWidget->hwndWidget,
                                                       &Pos))
                     && (!ctrpFindWidgetSetting(pXCenterData->somSelf,
                                                &Pos,
                                                &pSetting,
                                                NULL))
                   )
                {
                    CHAR ach[CCHMAXPATH];

                    DrgQueryStrName(pdt->hstrRenderToName,
                                    CCHMAXPATH,
                                    ach);

                    brc = ctrpSaveToFile(ach,
                                         pWidget->pcszWidgetClass,
                                         pSetting->Public.pszSetupString);

                } // end if (pSetting)
            }
        }
        CATCH(excpt1)
        {
            brc = FALSE;
        } END_CATCH();

        if (pobjLock)
            _wpReleaseObjectMutexSem(pobjLock);

    } // end if (pXCenterData)

    // post this even if we crash, or we leak;
    // and use DrgSendTransferMsg instead of WinPostMsg
    // V0.9.19 (2002-06-08) [umoeller]
    /*
    DrgSendTransferMsg(pdt->hwndClient,
                       DM_RENDERCOMPLETE,
                       (MPARAM)pdt,
                       (MPARAM)((brc) ? DMFL_RENDEROK
                                      : DMFL_RENDERFAIL));
    */
    // V1.0.6 (2006-08-10) [erdmann]: dragging from XCenter to WPS caused crash @@fixes 806
    DrgPostTransferMsg(pdt->hwndClient,
                       DM_RENDERCOMPLETE,
                       pdt,
                       ((brc) ? DMFL_RENDEROK
                              : DMFL_RENDERFAIL),
                       0,
                       TRUE);

    // and this was missing, which leaked many KBs per drag
    // V0.9.19 (2002-06-08) [umoeller]
    DrgFreeDragtransfer(pdt);

    _PmpfF(("leaving, returning %d", brc));

    return brc;
}

/*
 *@@ ctrDefWidgetProc:
 *      default window procedure for widgets. This is
 *      exported from XFLDR.DLL.
 *
 *      See XCENTERWIDGETCLASS for rules that widget
 *      window procs must conform to.
 *
 *@@added V0.9.7 (2000-12-02) [umoeller]
 *@@changed V0.9.11 (2001-04-25) [umoeller]: changed default widget menu item IDs
 *@@changed V0.9.14 (2001-08-05) [lafaix]: added dnd support for transparent widgets
 */

MRESULT EXPENTRY ctrDefWidgetProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        /*
         * WM_CONTEXTMENU:
         *
         */

        case WM_CONTEXTMENU:
            DwgtContextMenu(hwnd, mp1, mp2);
        break;

        /*
         * WM_MENUEND:
         *
         */

        case WM_MENUEND:
            DwgtMenuEnd(hwnd, (HWND)mp2);
        break;

        /*
         * WM_COMMAND:
         *
         */

        case WM_COMMAND:
            DwgtCommand(hwnd, (USHORT)mp1);
        break;

        /*
         * WM_BEGINDRAG:
         *
         */

        case WM_BEGINDRAG:
            DwgtBeginDrag(hwnd, mp1);
        break;

        /*
         * DM_RENDER:
         *
         */

        case DM_RENDER:
            mrc = (MRESULT)DwgtRender(hwnd, (PDRAGTRANSFER)mp1);
        break;

        /*
         * DM_DISCARDOBJECT:
         *
         */

        case DM_DISCARDOBJECT:
            DwgtCommand(hwnd, ID_CRMI_REMOVEWGT);
        break;

        /*
         * DM_DRAGOVER:
         * DM_DRAGLEAVE:
         * DM_DROP:
         *      forward this to the parent if the widget is
         *      transparent
         */

        case DM_DRAGOVER:
        case DM_DRAGLEAVE:
        case DM_DROP:
        {
            PXCENTERWIDGET pWidget;
            if (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
            {
                // if transparent, use the parent context menu instead
                if (    (pWidget->ulClassFlags & WGTF_TRANSPARENT)
                     && (WinSendMsg(pWidget->hwndWidget,
                                    WM_CONTROL,
                                    MPFROM2SHORT(ID_XCENTER_CLIENT,
                                                 XN_HITTEST),
                                    mp2) == FALSE) // @todo convert mp2 to client pos
                   )
                    mrc = WinSendMsg(WinQueryWindow(hwnd, QW_PARENT),
                                     msg,
                                     mp1,
                                     mp2);
                else
                    mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
            }
        }
        break;

        /*
         * WM_BUTTON1*:
         *      swallow if not transparent
         *
         */

        case WM_BUTTON1CLICK:
        case WM_BUTTON1DBLCLK:
        case WM_BUTTON1DOWN:
        case WM_BUTTON1UP:
        {
            PXCENTERWIDGET pWidget;
            if (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
            {
                // if transparent, do not swallow
                if (    (pWidget->ulClassFlags & WGTF_TRANSPARENT)
                     && (WinSendMsg(pWidget->hwndWidget,
                                    WM_CONTROL,
                                    MPFROM2SHORT(ID_XCENTER_CLIENT,
                                                 XN_HITTEST),
                                    mp1) == FALSE)
                   )
                    mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
            }
        }
        break;

        /*
         * WM_DESTROY:
         *
         */

        case WM_DESTROY:
            DwgtDestroy(hwnd);
        break;

        default:
            mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
    }

    return mrc;
}


