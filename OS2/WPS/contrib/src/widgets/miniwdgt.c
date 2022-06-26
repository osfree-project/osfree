
/*
 *@@sourcefile miniwdgt.c:
 *      code template for a minimal XCenter widget.
 *
 *      This is an example of an XCenter widget plugin.
 *      This code is compiled into a separate widget
 *      plugin DLL, which (as with all widget plugins)
 *      must be put into the plugins/xcenter directory
 *      of the XWorkplace installation directory.
 *
 *      This dummy widget only displays a small rectangle
 *      with a question mark. This can be taken as a
 *      template for implementing something more useful.
 *
 *      NOTE: This code is absolutely minimal. It cannot
 *
 *      -- display settings dialog;
 *
 *      -- display tooltips;
 *
 *      -- store any settings in a widget setup string.
 *
 *      If you want to use any of that, download the XWorkplace
 *      source code and look at the samples in the src\widgets
 *      directory, which are more complex, but more powerful
 *      as well.
 *
 *      Of course, you are free not to use this code and
 *      rewrite everything from scratch.
 *
 *      Any XCenter widget plugin DLL must export the
 *      following procedures by ordinal:
 *
 *      -- Ordinal 1 (WgtInitModule): this must
 *         return the widgets which this DLL provides.
 *
 *      -- Ordinal 2 (WgtUnInitModule): this must
 *         clean up global DLL data.
 *
 *      Unless you start your own threads in your widget,
 *      you can safely compile the widget with the VAC
 *      subsystem libraries to reduce the DLL's size.
 *      You can also import functions from XFLDR.DLL
 *      to avoid code duplication.
 *
 *      This is all new with V0.9.9.
 *
 *@@added V0.9.9 (2001-02-06) [umoeller]
 *@@header "shared\center.h"
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
 *
 *      This file is part of the XWorkplace source package.
 *      Even though XWorkplace is free software under the GNU General
 *      Public License version 2 (GPL), permission is granted, free
 *      of charge, to use this file for the purpose of creating software
 *      that integrates into XWorkplace or eComStation, even if that
 *      software is not published under the GPL.
 *
 *      This permission  extends to this single file only, but NOT to
 *      other files of the XWorkplace sources, not even those that
 *      are based on or include this file.
 *
 *      As a special exception to the GPL, using this file for the
 *      purpose of creating software that integrates into XWorkplace
 *      or eComStation is not considered creating a derivative work
 *      in the sense of the GPL. In plain English, you are not bound
 *      to the terms and conditions of the GPL if you use only this
 *      file for that purpose. You are bound by the GPL however if
 *      you incorporate code from GPL'ed XWorkplace source files where
 *      this notice is not present.
 *
 *      This file is distributed in the hope that it will be useful,
 *      but still WITHOUT ANY WARRANTY; without even the implied
 *      warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *      In no event shall the authors and/or copyright holders be
 *      liable for any damages or other claims arising from the use
 *      of this software.
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

#define INCL_DOSMODULEMGR
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINDIALOGS
#define INCL_WININPUT
#define INCL_WINSWITCHLIST
#define INCL_WINRECTANGLES
#define INCL_WINPOINTERS
#define INCL_WINSYS
#define INCL_WINLISTBOXES
#define INCL_WINENTRYFIELDS

#define INCL_GPILOGCOLORTABLE
#include <os2.h>

// C library headers
#include <stdio.h>
#include <string.h>

// generic headers
// If this file were part of the XWorkplace sources, we'd now include
// the generic "setup.h" file, which has common set up code for every
// single XWorkplace code file. But it's not, so we won't include that.
// #include "setup.h"                      // code generation and debugging options

// headers in /helpers
// This would be the place to include headers from the "XWorkplace helpers".
// But since we do a minimal sample here, we can't include those helpful
// routines... again, see the src\widgets in the XWorkplace source code
// for how these functions can be imported from XFLDR.DLL to avoid duplicate
// code.
// #include "helpers\dosh.h"               // Control Program helper routines
// #include "helpers\gpih.h"               // GPI helper routines
// #include "helpers\prfh.h"               // INI file helper routines;
                                        // this include is required for some
                                        // of the structures in shared\center.h
// #include "helpers\winh.h"               // PM helper routines
// #include "helpers\xstring.h"            // extended string helpers

// XWorkplace implementation headers
// If this file were part of the XCenter sources, we'd now include
// "center.h" from the "include\shared" directory. Since we're not
// part of the XCenter sources here, we include that file from the
// "toolkit" directory in the binary release. That file is identical
// to "include\shared\center.h" in the XWorkplace sources.
#include "shared\center.h"              // public XCenter interfaces

#pragma hdrstop                     // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Private definitions
 *
 ********************************************************************/

// None currently.

/* ******************************************************************
 *
 *   XCenter widget class definition
 *
 ********************************************************************/

/*
 *      This contains the name of the PM window class and
 *      the XCENTERWIDGETCLASS definition(s) for the widget
 *      class(es) in this DLL.
 *
 *      The address of this structure (or an array of these
 *      structures, if there were several widget classes in
 *      this plugin) is returned by the "init" export
 *      (WgtInitModule).
 */

#define WNDCLASS_WIDGET_SAMPLE "XWPCenterMinimalWidget"

static const XCENTERWIDGETCLASS G_WidgetClasses[]
    = {
        WNDCLASS_WIDGET_SAMPLE,     // PM window class name
        0,                          // additional flag, not used here
        "MinimalWidget",            // internal widget class name
        "Minimal",                  // widget class name displayed to user
        WGTF_UNIQUEPERXCENTER,      // widget class flags
        NULL                        // no settings dialog
      };

/* ******************************************************************
 *
 *   Function imports from XFLDR.DLL
 *
 ********************************************************************/

/*
 *      To reduce the size of the widget DLL, it can
 *      be compiled with the VAC subsystem libraries.
 *      In addition, instead of linking frequently
 *      used helpers against the DLL again, you can
 *      import them from XFLDR.DLL, whose module handle
 *      is given to you in the INITMODULE export.
 *
 *      Note that importing functions from XFLDR.DLL
 *      is _not_ a requirement. We can't do this in
 *      this minimal sample anyway without having access
 *      to the full XWorkplace source code.
 *
 *      If you want to know how you can import the useful
 *      functions from XFLDR.DLL to use them in your widget
 *      plugin, again, see src\widgets in the XWorkplace sources.
 *      The actual imports would then be made by WgtInitModule.
 */

/* ******************************************************************
 *
 *   Private widget instance data
 *
 ********************************************************************/

// None presently. The samples in src\widgets in the XWorkplace
// sources cleanly separate setup string data from other widget
// instance data to allow for easier manipulation with settings
// dialogs. We have skipped this for the minimal sample.

/* ******************************************************************
 *
 *   Widget setup management
 *
 ********************************************************************/

// None presently. See above.

/* ******************************************************************
 *
 *   Widget settings dialog
 *
 ********************************************************************/

// None currently. To see how a setup dialog can be done,
// see the "window list" widget in the XWorkplace sources
// (src\widgets\w_winlist.c).

/* ******************************************************************
 *
 *   Callbacks stored in XCENTERWIDGETCLASS
 *
 ********************************************************************/

// If you implement a settings dialog, you must write a
// "show settings dlg" function and store its function pointer
// in XCENTERWIDGETCLASS.

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
 *@@ MwgtControl:
 *      implementation for WM_CONTROL in fnwpSampleWidget.
 *
 *      The XCenter communicates with widgets thru
 *      WM_CONTROL messages. At the very least, the
 *      widget should respond to XN_QUERYSIZE because
 *      otherwise it will be given some dumb default
 *      size.
 *
 *@@added V0.9.7 (2000-12-14) [umoeller]
 */

BOOL WgtControl(PXCENTERWIDGET pWidget,
                MPARAM mp1,
                MPARAM mp2)
{
    BOOL brc = FALSE;

    USHORT  usID = SHORT1FROMMP(mp1),
            usNotifyCode = SHORT2FROMMP(mp1);

    // is this from the XCenter client?
    if (usID == ID_XCENTER_CLIENT)
    {
        // yes:

        switch (usNotifyCode)
        {
            /*
             * XN_QUERYSIZE:
             *      XCenter wants to know our size.
             */

            case XN_QUERYSIZE:
            {
                PSIZEL pszl = (PSIZEL)mp2;
                pszl->cx = 30;      // desired width
                pszl->cy = 20;      // desired minimum height
                brc = TRUE;
            }
            break;

        } // end switch (usNotifyCode)
    } // end if (usID == ID_XCENTER_CLIENT)

    return brc;
}

/*
 *@@ WgtPaint:
 *      implementation for WM_PAINT in fnwpSampleWidget.
 *
 *      This really does nothing, except painting a
 *      3D rectangle and printing a question mark.
 */

VOID WgtPaint(HWND hwnd,
              PXCENTERWIDGET pWidget)
{
    HPS hps;
    if (hps = WinBeginPaint(hwnd, NULLHANDLE, NULL))
    {
        RECTL   rclWin;

        // switch HPS to RGB mode
        GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULL);

        // now paint
        WinQueryWindowRect(hwnd,
                           &rclWin);        // exclusive

        WinFillRect(hps,
                    &rclWin,                // exclusive
                    WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0));

        // print question mark
        WinDrawText(hps,
                    1,
                    "?",
                    &rclWin,                // exclusive
                    WinQuerySysColor(HWND_DESKTOP, SYSCLR_WINDOWSTATICTEXT, 0),
                    WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0),
                    DT_CENTER | DT_VCENTER);

        WinEndPaint(hps);
    }
}

/*
 *@@ fnwpSampleWidget:
 *      window procedure for the winlist widget class.
 *
 *      There are a few rules which widget window procs
 *      must follow. See XCENTERWIDGETCLASS in center.h
 *      for details.
 *
 *      Other than that, this is a regular window procedure
 *      which follows the basic rules for a PM window class.
 */

MRESULT EXPENTRY fnwpSampleWidget(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;
    // get widget data from QWL_USER (stored there by WM_CREATE)
    PXCENTERWIDGET pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER);
                    // this ptr is valid after WM_CREATE

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
         *      We could use XCENTERWIDGET.pUser for allocating
         *      another private memory block for our own stuff,
         *      for example to be able to store fonts and colors.
         *      We ain't doing this in the minimal sample.
         */

        case WM_CREATE:
            WinSetWindowPtr(hwnd, QWL_USER, mp1);
            if (    (!(pWidget = (PXCENTERWIDGET)mp1))
                 || (!pWidget->pfnwpDefWidgetProc)
               )
                // shouldn't happen... stop window creation!!
                mrc = (MPARAM)TRUE;
        break;

        /*
         * WM_CONTROL:
         *      process notifications/queries from the XCenter.
         */

        case WM_CONTROL:
            mrc = (MPARAM)WgtControl(pWidget, mp1, mp2);
        break;

        /*
         * WM_PAINT:
         *      well, paint the widget.
         */

        case WM_PAINT:
            WgtPaint(hwnd, pWidget);
        break;

        /*
         * WM_PRESPARAMCHANGED:
         *      A well-behaved widget would intercept
         *      this and store fonts and colors.
         */

        /* case WM_PRESPARAMCHANGED:
        break; */

        /*
         * WM_DESTROY:
         *      clean up. This _must_ be passed on to
         *      ctrDefWidgetProc.
         */

        case WM_DESTROY:
            // If we had any user data allocated in WM_CREATE
            // or elsewhere, we'd clean this up here.
            // We _MUST_ pass this on, or the default widget proc
            // cannot clean up.
            mrc = pWidget->pfnwpDefWidgetProc(hwnd, msg, mp1, mp2);
        break;

        default:
            mrc = pWidget->pfnwpDefWidgetProc(hwnd, msg, mp1, mp2);

    } // end switch(msg)

    return mrc;
}

/* ******************************************************************
 *
 *   Exported procedures
 *
 ********************************************************************/

/*
 *@@ WgtInitModule:
 *      required export with ordinal 1, which must tell
 *      the XCenter how many widgets this DLL provides,
 *      and give the XCenter an array of XCENTERWIDGETCLASS
 *      structures describing the widgets.
 *
 *      With this call, you are given the module handle of
 *      XFLDR.DLL. For convenience, and if you have the full
 *      XWorkplace source code, you could resolve imports
 *      for some useful functions which are exported thru
 *      src\shared\xwp.def. We don't do this here.
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
 *      Your widget window _will_ be resized by the XCenter,
 *      even if you're not planning it to be.
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

ULONG EXPENTRY WgtInitModule(HAB hab,               // XCenter's anchor block
                             HMODULE hmodPlugin, // module handle of the widget DLL
                             HMODULE hmodXFLDR,     // XFLDR.DLL module handle
                             PCXCENTERWIDGETCLASS *ppaClasses,
                             PSZ pszErrorMsg)       // if 0 is returned, 500 bytes of error msg
{
    ULONG   ulrc = 0;

    // register our PM window class
    if (!WinRegisterClass(hab,
                          WNDCLASS_WIDGET_SAMPLE,
                          fnwpSampleWidget,
                          CS_PARENTCLIP | CS_SIZEREDRAW | CS_SYNCPAINT,
                          sizeof(PVOID))
                                // extra memory to reserve for QWL_USER
                         )
        // error registering class: report error then
        strcpy(pszErrorMsg, "WinRegisterClass failed.");
    else
    {
        // no error:
        // return widget classes array
        *ppaClasses = G_WidgetClasses;

        // return no. of classes in this DLL (one here):
        ulrc = sizeof(G_WidgetClasses) / sizeof(G_WidgetClasses[0]);
    }

    return ulrc;
}

/*
 *@@ WgtUnInitModule:
 *      optional export with ordinal 2, which can clean
 *      up global widget class data.
 *
 *      This gets called by the XCenter right before
 *      a widget DLL gets unloaded. Note that this
 *      gets called even if the "init module" export
 *      returned 0 (meaning an error) and the DLL
 *      gets unloaded right away.
 */

VOID EXPENTRY WgtUnInitModule(VOID)
{
}

/*
 *@@ WgtQueryVersion:
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

VOID EXPENTRY WgtQueryVersion(PULONG pulMajor,
                              PULONG pulMinor,
                              PULONG pulRevision)
{
    // report 0.9.9
    *pulMajor = 0;
    *pulMinor = 9;
    *pulRevision = 9;
}

