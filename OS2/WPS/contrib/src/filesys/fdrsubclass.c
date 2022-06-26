
/*
 *@@sourcefile fdrsubclass.c:
 *      this file is ALL new with V0.9.3 and now implements
 *      folder frame subclassing, which has been largely
 *      redesigned with V0.9.3.
 *
 *      XWorkplace subclasses all WPFolder frame windows to
 *      intercept a large number of messages. This is needed
 *      for the majority of XWorkplace's features, which are
 *      not directly SOM/WPS-related, but are rather straight
 *      PM programming.
 *
 *      Most of the standard WPS methods are really encapsulations
 *      of PM messages. When the WPS opens a folder window, it
 *      creates a container control as a child of the folder frame
 *      window (with the id FID_CLIENT), which is subclassed to
 *      get all the container WM_CONTROL messages. Then, for
 *      example, if the user opens a context menu on an object
 *      in the container, the frame gets WM_CONTROL with
 *      CN_CONTEXTMENU. The WPS then finds the WPS (SOM) object
 *      which corresponds to the container record core on which
 *      the context menu was opened and invokes the WPS menu
 *      methods on it, that is, wpFilterPopupMenu and
 *      wpModifyPopupMenu. Similar things happen with WM_COMMAND
 *      and wpMenuItemSelected.
 *
 *      The trick is just how to get "past" the WPS, which does
 *      a lot of hidden stuff in its method code. By subclassing
 *      the folder frames ourselves, we get all the messages which
 *      are being sent or posted to the folder _before_ the WPS
 *      gets them and can thus modify the WPS's behavior even if
 *      no methods have been defined for a certain event. An
 *      example of this is that the contents of an XFolder status
 *      bar changes when object selections have changed in the
 *      folder: this reacts to WM_CONTROL with CN_EMPHASIS.
 *
 *      Subclassing is done in XFolder::wpOpen after having called
 *      the parent method (WPFolder's), which returns the handle
 *      of the new folder frame which the WPS has created.
 *      XFolder::wpOpen calls fdrManipulateNewView for doing this.
 *
 *      XFolder's subclassed frame window proc is called
 *      fnwpSubclWPFolderWindow. Take a look at it, it's one of
 *      the most interesting parts of XWorkplace. It handles status
 *      bars (which are frame controls), tree view auto-scrolling,
 *      special menu features, the "folder content" menus and more.
 *
 *      This gives us the following hierarchy of window procedures:
 *
 *      1. our fnwpSubclWPFolderWindow, first, followed by
 *
 *      2. the WPS folder frame window subclass, followed by
 *
 *      3. WC_FRAME's default frame window procedure, followed by
 *
 *      4. WinDefWindowProc last.
 *
 *      If additional WPS enhancers are installed (e.g. Object
 *      Desktop), they will appear at the top of the chain also.
 *      I guess it depends on the hierarchy of replacement classes
 *      in the WPS class list which one sits on top.
 *
 *      While we had an ugly global linked list of subclassed
 *      folder views in all XFolder and XWorkplace versions
 *      before V0.9.3, I have finally (after two years...)
 *      found a more elegant way of storing folder data. The
 *      secret is simply re-registing the folder view window
 *      class ("wpFolder window"), which is initially registered
 *      by the WPS somehow. No more global variables and mutex
 *      semaphores...
 *
 *      This hopefully fixes most folder serialization problems
 *      which have occurred with previous XFolder/XWorkplace versions.
 *
 *      Since I was unable to find out where WinRegisterClass
 *      gets called from in the WPS, I have implemented a local
 *      send-message hook for PMSHELL.EXE only which re-registers
 *      that window class again when the first WM_CREATE for a
 *      folder view comes in. See fdr_SendMsgHook. Basically,
 *      this adds more bytes for the window words so we can store
 *      a SUBCLFOLDERVIEW structure for each folder view in
 *      the window words, instead of having to maintain a global
 *      linked list.
 *
 *      When a folder view is subclassed (during XFolder::wpOpen
 *      processing) by fdrSubclassFolderView, a SUBCLFOLDERVIEW is
 *      created and stored in the new window words. Appears to
 *      work fine so far.
 *
 *      Function prefix for this file:
 *      --  fdr*
 *
 *@@added V0.9.3 [umoeller]
 *@@header "filesys\folder.h"
 */

/*
 *      Copyright (C) 1997-2007 Ulrich M”ller.
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

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINFRAMEMGR
#define INCL_WININPUT
#define INCL_WINRECTANGLES
#define INCL_WINPOINTERS
#define INCL_WINSYS             // needed for presparams
#define INCL_WINMENUS
#define INCL_WINTIMER
#define INCL_WINDIALOGS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINENTRYFIELDS
#define INCL_WINLISTBOXES
#define INCL_WINSTDCNR
#define INCL_WINSCROLLBARS
#define INCL_WINSHELLDATA       // Prf* functions
#define INCL_WINHOOKS

#define INCL_GPIPRIMITIVES
#define INCL_GPILOGCOLORTABLE
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\winh.h"               // PM helper routines

// SOM headers which don't crash with prec. header files
#include "xfdataf.ih"
#include "xfldr.ih"
// #include "xfobj.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\contentmenus.h"        // shared menu logic
#include "shared\errors.h"              // private XWorkplace error codes
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)

#include "filesys\fileops.h"            // file operations implementation
#include "filesys\filetype.h"           // extended file types implementation
#include "filesys\folder.h"             // XFolder implementation
#include "filesys\fdrcommand.h"         // folder menu command reactions
#include "filesys\fdrhotky.h"           // folder hotkey handling
#include "filesys\fdrmenus.h"           // shared folder menu logic
#include "filesys\fdrsubclass.h"        // folder subclassing engine
#include "filesys\icons.h"              // icons handling
#include "filesys\object.h"             // XFldObject implementation
#include "filesys\statbars.h"           // status bar translation logic
#include "filesys\xthreads.h"           // extra XWorkplace threads

// other SOM headers
#pragma hdrstop                         // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

// flag for whether we have manipulated the "wpFolder window"
// PM window class already; this is done in fdr_SendMsgHook
static BOOL         G_WPFolderWinClassExtended = FALSE;

static CLASSINFO    G_WPFolderWinClassInfo;

static ULONG        G_SFVOffset = 0;

STATIC MRESULT EXPENTRY fnwpSubclWPFolderWindow(HWND hwndFrame,
                                                  ULONG msg,
                                                  MPARAM mp1,
                                                  MPARAM mp2);

/* ******************************************************************
 *
 *   Send-message hook
 *
 ********************************************************************/

/*
 *@@ fdr_SendMsgHook:
 *      local send-message hook for PMSHELL.EXE only.
 *      This is installed from M_XFolder::wpclsInitData
 *      and needed to re-register the "wpFolder window"
 *      PM window class which is used by the WPS for
 *      folder views. We add more window words to that
 *      class for storing our window data.
 *
 *@@added V0.9.3 (2000-04-08) [umoeller]
 *@@changed V0.9.16 (2001-12-02) [umoeller]: now releasing hook again
 */

VOID EXPENTRY fdr_SendMsgHook(HAB hab,
                              PSMHSTRUCT psmh,
                              BOOL fInterTask)
{
    /*
     * WM_CREATE:
     *
     */

    CHAR    szClass[300];

    // re-register the WPFolder window class if we haven't
    // done this yet; this is needed because per default,
    // the WPS "wpFolder window" class apparently uses
    // QWL_USER for other purposes...

    if (    (psmh->msg == WM_CREATE)
         && (!G_WPFolderWinClassExtended)
         && (WinQueryClassName(psmh->hwnd,
                               sizeof(szClass),
                               szClass))
         && (!strcmp(szClass, WC_WPFOLDERWINDOW)) // "wpFolder window"))
            // it's a folder:
            // OK, we have the first WM_CREATE for a folder window
            // after Desktop startup now...
         && (WinQueryClassInfo(hab,
                               (PSZ)WC_WPFOLDERWINDOW, // "wpFolder window",
                               &G_WPFolderWinClassInfo))
        )
    {
        // _Pmpf(("    wpFolder cbWindowData: %d", G_WPFolderWinClassInfo.cbWindowData));
        // _Pmpf(("    QWL_USER is: %d", QWL_USER));

        // replace original window class
        if (WinRegisterClass(hab,
                             (PSZ)WC_WPFOLDERWINDOW,
                             G_WPFolderWinClassInfo.pfnWindowProc, // fdr_fnwpSubclassedFolder2,
                             G_WPFolderWinClassInfo.flClassStyle,
                             G_WPFolderWinClassInfo.cbWindowData + 16))
        {
            // _Pmpf(("    WinRegisterClass OK"));

            // OK, window class successfully re-registered:
            // store the offset of our window word for the
            // SUBCLFOLDERVIEW's in a global variable
            G_SFVOffset = G_WPFolderWinClassInfo.cbWindowData + 12;

            // don't do this again
            G_WPFolderWinClassExtended = TRUE;

            // we can now uninstall the hook, we've done
            // what we had to do...
            // V0.9.16 (2001-12-02) [umoeller]
            if (!WinReleaseHook(WinQueryAnchorBlock(HWND_DESKTOP),
                                HMQ_CURRENT,
                                HK_SENDMSG,
                                (PFN)fdr_SendMsgHook,
                                NULLHANDLE))  // module handle, can be 0 for local hook
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "WinReleaseHook failed.");
        }
        // else _Pmpf(("    WinRegisterClass failed"));
    }
}

/* ******************************************************************
 *
 *   Management of folder frame window subclassing
 *
 ********************************************************************/

/*
 *@@ fdrSubclassFolderView:
 *      creates a SUBCLFOLDERVIEW for the given folder
 *      view.
 *
 *      We also create a supplementary folder object window
 *      for the view here and store the HWND in the SFV.
 *
 *      This stores the SFV pointer in the frame's window
 *      words at the ulWindowWordOffset position (by calling
 *      WinSetWindowPtr). If ulWindowWordOffset is -1,
 *      this uses a special offset that was determined
 *      internally. This is safe, but ONLY with folder windows
 *      created from XFolder::wpOpen ("true" folder views
 *      of the "wpFolder window" PM window class).
 *
 *      The ulWindowWordOffset param has been added to allow
 *      subclassing container owners other than "true" folder
 *      frames, for example some container which was subclassed
 *      by the WPS because objects have been inserted using
 *      WPObject::wpCnrInsertObject. For example, if you have
 *      a standard frame that you have full control over,
 *      specify QWL_USER (0) in those cases. This is now used
 *      by the new folder split views and file dialogs.
 *
 *      This no longer actually subclasses the frame because
 *      fnwpSubclWPFolderWindow requires the
 *      SFV to be at a fixed position. After calling this,
 *      subclass the folder frame yourself. "wpFolder window"
 *      views will use fnwpSubclWPFolderWindow, while
 *      split views will have their own frame procs that handle
 *      a few events differently.
 *
 *@@added V0.9.3 (2000-04-08) [umoeller]
 *@@changed V0.9.3 (2000-04-08) [umoeller]: no longer using the linked list
 *@@changed V0.9.9 (2001-03-11) [umoeller]: added ulWindowWordOffset param
 *@@changed V0.9.9 (2001-03-11) [umoeller]: no longer subclassing
 *@@changed V0.9.9 (2001-03-11) [umoeller]: renamed from fdrSubclassFolderView
 *@@changed V1.0.1 (2002-11-30) [umoeller]: fixed creation of suppl. obj window, which is not needed for split views
 */

PSUBCLFOLDERVIEW fdrCreateSFV(HWND hwndFrame,           // in: folder frame
                              HWND hwndCnr,             // in: frame's FID_CLIENT
                              ULONG ulWindowWordOffset,
                                  // in: offset at which to store
                                  // SUBCLFOLDERVIEW ptr in
                                  // frame's window words, or -1
                                  // for safe default
                              BOOL fCreateSuppl,        // in: create suppl. object window?
                                                        // TRUE only for subclassed WPS folders
                              WPFolder *somSelf,
                                   // in: folder; either XFolder's somSelf
                                   // or XFldDisk's root folder
                              WPObject *pRealObject)
                                   // in: the "real" object; for XFolder, this is == somSelf,
                                   // for XFldDisk, this is the disk object (needed for object handles)
{
    PSUBCLFOLDERVIEW psliNew;

    if (psliNew = NEW(SUBCLFOLDERVIEW))
    {
        ZERO(psliNew);

        if (!hwndCnr)
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                   "hwndCnr is NULLHANDLE for folder %s.",
                   _wpQueryTitle(somSelf));

        // store various other data here
        psliNew->hwndFrame = hwndFrame;
        psliNew->somSelf = somSelf;
        psliNew->pRealObject = pRealObject;
        psliNew->hwndCnr = hwndCnr;

        // psliNew->fRemoveSourceEmphasis = FALSE;
                // taken care of by memset above V1.0.0 (2002-09-13) [umoeller]

        // set status bar hwnd to zero at this point;
        // this will be created elsewhere
        // psliNew->hwndStatusBar = NULLHANDLE;
                // taken care of by memset above V1.0.0 (2002-09-13) [umoeller]

        // create a supplementary object window
        // for this folder frame (see
        // fdr_fnwpSupplFolderObject for details)
        if (    (fCreateSuppl)          // V1.0.1 (2002-11-30) [umoeller]
             && (psliNew->hwndSupplObject = winhCreateObjectWindow(WNDCLASS_SUPPLOBJECT,
                                                                   psliNew))
           )
        {
            psliNew->ulWindowWordOffset
                    = (ulWindowWordOffset == -1)
                         ? G_SFVOffset        // window word offset which we've
                                          // calculated in fdr_SendMsgHook
                         : ulWindowWordOffset; // changed V0.9.9 (2001-03-11) [umoeller]
        }

        // store SFV in frame's window words
        WinSetWindowPtr(hwndFrame,
                        psliNew->ulWindowWordOffset,
                        psliNew);
    }

    return psliNew;
}

/*
 *@@ fdrSubclassFolderView:
 *      calls fdrCreateSFV and subclasses the folder
 *      frame with fnwpSubclWPFolderWindow.
 *
 *      This gets called for standard XFldDisk and
 *      XFolder frames.
 *
 *@@added V0.9.9 (2001-03-11) [umoeller]
 */

PSUBCLFOLDERVIEW fdrSubclassFolderView(HWND hwndFrame,
                                       HWND hwndCnr,
                                       WPFolder *somSelf,
                                            // in: folder; either XFolder's somSelf
                                            // or XFldDisk's root folder
                                       WPObject *pRealObject)
                                            // in: the "real" object; for XFolder, this is == somSelf,
                                            // for XFldDisk, this is the disk object (needed for object handles)
{
    PSUBCLFOLDERVIEW psfv;
    if (psfv = fdrCreateSFV(hwndFrame,
                            hwndCnr,
                            -1,    // default window word V0.9.9 (2001-03-11) [umoeller]
                            TRUE,            // create suppl. object window
                            somSelf,
                            pRealObject))
    {
        psfv->pfnwpOriginal = WinSubclassWindow(hwndFrame,
                                                fnwpSubclWPFolderWindow);
    }

    return psfv;
}

/*
 *@@ fdrQuerySFV:
 *      this retrieves the PSUBCLFOLDERVIEW from the
 *      specified subclassed folder frame. One of these
 *      structs is maintained for each open folder view
 *      to store window data which is needed everywhere.
 *
 *      Works only for "true" folder frames created by
 *      XFolder::wpOpen.
 *
 *      Returns NULL if not found.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist functions
 *@@changed V0.9.0 [umoeller]: pulIndex added to function prototype
 *@@changed V0.9.0 [umoeller]: moved this func here from common.c
 *@@changed V0.9.1 (2000-02-14) [umoeller]: reversed order of functions; now subclassing is last
 *@@changed V0.9.3 (2000-04-08) [umoeller]: completely replaced
 */

PSUBCLFOLDERVIEW fdrQuerySFV(HWND hwndFrame,        // in: folder frame to find
                             PULONG pulIndex)       // out: index in linked list if found
{
    return (PSUBCLFOLDERVIEW)WinQueryWindowPtr(hwndFrame,
                                               G_SFVOffset);
}

/*
 *@@ fdrRemoveSFV:
 *      reverse to fdrSubclassFolderView, this removes
 *      a PSUBCLFOLDERVIEW from the folder frame again.
 *      Called upon WM_DESTROY in folder frames.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist functions
 *@@changed V0.9.0 [umoeller]: moved this func here from common.c
 *@@changed V0.9.3 (2000-04-08) [umoeller]: completely replaced
 */

VOID fdrRemoveSFV(PSUBCLFOLDERVIEW psfv)
{
    WinSetWindowPtr(psfv->hwndFrame,
                    psfv->ulWindowWordOffset, // V0.9.9 (2001-03-11) [umoeller]
                    NULL);
    free(psfv);
}

/*
 *@@ fdrManipulateNewView:
 *      this gets called from XFolder::wpOpen after a new
 *      Icon, Tree, or Details view has been successfully
 *      opened by WPFolder::wpOpen.
 *
 *      This is the one place that hacks all the XWorkplace
 *      enhancements into the folder. It manipulates the
 *      view according to the global or instance settings
 *      (subclassing the frame, sorting, full status bar,
 *      path in title etc.).
 *
 *      This is ONLY used for folders, not for WPDisk's.
 *      This calls fdrSubclassFolderView in turn.
 *
 *@@added V0.9.0 (99-11-27) [umoeller]
 *@@changed V0.9.1 (2000-02-08) [umoeller]: status bars were added even if globally disabled; fixed.
 *@@changed V0.9.3 (2000-04-08) [umoeller]: adjusted for new folder frame subclassing
 *@@changed V0.9.19 (2002-04-17) [umoeller]: now using stbViewHasStatusBar to fix Object Desktop classes
 */

VOID fdrManipulateNewView(WPFolder *somSelf,        // in: folder with new view
                          HWND hwndNewFrame,        // in: new view (frame) of folder
                          ULONG ulView)             // in: OPEN_CONTENTS, OPEN_TREE, or OPEN_DETAILS
{
    PSUBCLFOLDERVIEW    psfv = 0;
    XFolderData         *somThis = XFolderGetData(somSelf);
    HWND                hwndCnr = WinWindowFromID(hwndNewFrame, FID_CLIENT);

    ULONG flViews;

    // subclass the new folder frame window;
    // this creates a SUBCLFOLDERVIEW for the view
    psfv = fdrSubclassFolderView(hwndNewFrame,
                                 hwndCnr,
                                 somSelf,
                                 somSelf);  // "real" object; for folders, this is the folder too

    // change the window title to full path, if allowed
    if (    (_bFullPathInstance == 1)
         || ((_bFullPathInstance == 2) && (cmnQuerySetting(sfFullPath)))
       )
        fdrSetOneFrameWndTitle(somSelf, hwndNewFrame);

    // add status bar, if allowed:
    if (    (stbViewHasStatusBar(somSelf,
                                 hwndNewFrame,  // V1.0.1 (2002-12-08) [umoeller]
                                 ulView))      // V0.9.19 (2002-04-17) [umoeller]
         && (psfv)
       )
    {
        stbCreate(psfv);
    }

    // replace sort stuff
#ifndef __ALWAYSEXTSORT__
    if (cmnQuerySetting(sfExtendedSorting))
#endif
        if (hwndCnr)
        {
            PMPF_SORT(("setting folder sort"));

            fdrSetFldrCnrSort(somSelf,
                              hwndCnr,
                              TRUE);        // force
        }
}

/* ******************************************************************
 *
 *   New subclassed folder frame message processing
 *
 ********************************************************************/

/*
 *@@ PostWMChar:
 *      little helper for posting a WM_CHAR msg
 *      twice, once for key down and once for
 *      key up.
 *
 *@@added V0.9.19 (2002-06-02) [umoeller]
 */

STATIC VOID PostWMChar(HWND hwnd,
                       USHORT fsFlags,
                       MPARAM mp2)
{
    WinPostMsg(hwnd,
               WM_CHAR,
               MPFROM2SHORT(fsFlags, 0),
               mp2);
    WinPostMsg(hwnd,
               WM_CHAR,
               MPFROM2SHORT(fsFlags | KC_KEYUP, 0),
               mp2);
}

/*
 * FormatFrame:
 *      part of the implementation of WM_FORMATFRAME in
 *      fnwpSubclWPFolderWindow. Gets called from FormatFrame2
 *      but has been exported to be useful for split views too.
 *
 *      Part of the needed frame hacks for folder status bars.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 *@@changed V0.9.18 (2002-03-24) [umoeller]: fixed stupid scroll bars when always sort is off
 *@@changed V1.0.2 (2003-11-05) [bvl]: changed usage of folder size to use the SWP array other fram controls work now as well @@fixes 481
 */

VOID fdrFormatFrame(HWND hwndFrame,
                    HWND hwndStatusBar,
                    MPARAM mp1,            // in: mp1 from WM_FORMATFRAME (points to SWP array)
                    ULONG ulCount,         // in: frame control count (returned from default wnd proc)
                    HWND *phwndClient)     // out: client window (ptr can be NULL)
{
    // access the SWP array that is passed to us
    // and search all the controls for the container child window,
    // which for folders always has the ID 0x8008
    ULONG       ul;
    PSWP        swpArr = (PSWP)mp1;

    for (ul = 0;
         ul < ulCount;
         ul++)
    {
        HWND    hwndThis = swpArr[ul].hwnd;

        if (WinQueryWindowUShort(hwndThis, QWS_ID) == 0x8008)  // FID_CLIENT
        {
            // container found: reduce size of container by
            // status bar height
            // POINTL      ptlBorderSizes;
            ULONG       ulStatusBarHeight = cmnQueryStatusBarHeight();

            // redundant now V1.0.2 (2004-01-08) [umoeller]
            // WinSendMsg(hwndFrame,
            //            WM_QUERYBORDERSIZE,
            //            (MPARAM)&ptlBorderSizes,
            //            0);

            // first initialize the _new_ SWP for the status bar.
            // Since the SWP array for the std frame controls is
            // zero-based, and the standard frame controls occupy
            // indices 0 thru ulCount-1 (where ulCount is the total
            // count), we use ulCount for our static text control.
            swpArr[ulCount].fl = SWP_MOVE | SWP_SIZE | SWP_NOADJUST | SWP_ZORDER;
            // swpArr[ulCount].x  = ptlBorderSizes.x;
            // use swpArr[ul].x to allow other frame controls to exist V1.0.2 (2003-11-05) [bvl]
            swpArr[ulCount].x = swpArr[ul].x;
            swpArr[ulCount].y  = swpArr[ul].y; // ptlBorderSizes.y;
            swpArr[ulCount].cx = swpArr[ul].cx;  // same as cnr's width
            swpArr[ulCount].cy = ulStatusBarHeight;
            swpArr[ulCount].hwndInsertBehind = HWND_BOTTOM; // HWND_TOP;
            swpArr[ulCount].hwnd = hwndStatusBar;

            // adjust the origin and height of the container to
            // accomodate our static text control
            swpArr[ul].y  += swpArr[ulCount].cy;
            swpArr[ul].cy -= swpArr[ulCount].cy;

            // now we need to adjust the workspace origin of the cnr
            // accordingly, or otherwise the folder icons will appear
            // outside the visible cnr workspace and scroll bars will
            // show up.
            // We only do this the first time we're arriving here
            // (which should be before the WPS is populating the folder);
            // psfv->fNeedCnrScroll has been initially set to TRUE
            // by stbCreate.
            #if 0 // #ifdef __DEBUG__
            {
                PMPF_STATUSBARS(("psfv->fNeedCnrScroll: %d", psfv->fNeedCnrScroll));

                cnrhQueryCnrInfo(hwndThis, &CnrInfo);

                PMPF_STATUSBARS(("Old CnrInfo.ptlOrigin.y: %lX", CnrInfo.ptlOrigin.y ));
            }
            #endif

            if (phwndClient)
                *phwndClient = hwndThis;

            break;  // we're done
        } // end if WinQueryWindowUShort
    } // end for (ul = 0; ul < ulCount; ul++)
}

/*
 *@@ FormatFrame2:
 *      the implementation of WM_FORMATFRAME in
 *      fnwpSubclWPFolderWindow.
 *
 *@@added V1.0.0 (2002-08-21) [umoeller]
 *@@changed V1.0.0 (2002-09-09) [umoeller]: fixed annoying scroll bars in folder frame when always sort was off
 *@@changed V1.0.1 (2003-02-02) [umoeller]: really fixed annoying scroll bars in folder frame when always sort was off
 */

MRESULT FormatFrame2(PSUBCLFOLDERVIEW psfv,     // in: frame information
                     MPARAM mp1,
                     MPARAM mp2,
                     PFNWP pfnwpOriginal)
{
    MRESULT mrc;

    //  query the number of standard frame controls
    ULONG ulCount = (ULONG)pfnwpOriginal(psfv->hwndFrame, WM_FORMATFRAME, mp1, mp2);

    PMPF_STATUSBARS(("WM_FORMATFRAME ulCount = %d", ulCount));

    if (psfv->hwndStatusBar)
    {
        HWND hwndClient;

        // we have a status bar:
        // format the frame
        fdrFormatFrame(psfv->hwndFrame,
                       psfv->hwndStatusBar,
                       mp1,
                       ulCount,
                       &hwndClient);

        PMPF_STATUSBARS(("fNeedCnrScroll = %d, hwndClient = 0x%lX",
                         psfv->bNeedCnrScroll,
                         hwndClient));

        // sigh... reworked this code for the, what, fifth time?
        // We really need to tell apart _three_ cases with the
        // damn container scolling: V1.0.1 (2003-02-02) [umoeller]

        // 1) The folder is being opened for the very first time.
        //    In that case, stbCreate() has inflated the folder frame
        //    and set the SCROLL_VERYFIRSTTIME. We then need to
        //    adjust the container origin just a slight little bit.
        if (psfv->bNeedCnrScroll == SCROLL_VERYFIRSTTIME)
        {
            CNRINFO     CnrInfo;
            ULONG       ulStatusBarHeight = cmnQueryStatusBarHeight();

            cnrhQueryCnrInfo(hwndClient, &CnrInfo);

            PMPF_STATUSBARS(("CnrInfo.ptlOrigin.y %d, ulStatusBarHeight %d",
                             CnrInfo.ptlOrigin.y,
                             ulStatusBarHeight));

            // CnrInfo.ptlOrigin.y is always 0, no matter if we get
            // scroll bars later on or not, so the below check didn't
            // exactly help... the point is we must ALWAYS reduce
            // the ptlOrigin.y by the status bar height, or we'll
            // get scroll bars, period
            // V1.0.0 (2002-09-09) [umoeller]

            // if (CnrInfo.ptlOrigin.y >= (LONG)ulStatusBarHeight)
            {
                // RECTL rclViewport;

                CnrInfo.ptlOrigin.y -= ulStatusBarHeight;

                PMPF_STATUSBARS(("New CnrInfo.ptlOrigin.y: %lX", CnrInfo.ptlOrigin.y ));

                WinSendMsg(hwndClient,
                           CM_SETCNRINFO,
                           (MPARAM)&CnrInfo,
                           (MPARAM)CMA_PTLORIGIN);
            }
        } // end if (psfv->fNeedCnrScroll)
        // 2) Subsequent folder opens, when the (inflated) frame
        //    size has been correctly stored by the wps with the
        //    folder position. In that case, we get really huge
        //    scroll offsets for some very strange reason, and
        //    I can't help this except for forcing cnr scroll
        //    to the very bottom. We get in here for every
        //    WM_FORMATFRAME, so do this only once!
        else if (psfv->bNeedCnrScroll == SCROLL_ADJUSTFORSB)   // V1.0.1 (2003-02-01) [umoeller]
        {
            // now scroll the damn container up the maximum;
            // we still get scroll bars in some situations if
            // always sort is off...
            // to scroll the container up _and_ get rid of
            // the scroll bars, we first post HOME to the
            // container's vertical scroll bar and _then_
            // another PAGEUP to the container itself
            // V0.9.18 (2002-03-24) [umoeller]
            // no longer needed since the above code works now V1.0.0 (2002-09-09) [umoeller]
            PostWMChar(WinWindowFromID(hwndClient, 0x7FF9),
                       KC_VIRTUALKEY | KC_CTRL,
                       MPFROM2SHORT(0,
                                    VK_HOME));
            PostWMChar(hwndClient,
                       KC_VIRTUALKEY,
                       MPFROM2SHORT(0,
                                    VK_PAGEUP));
        }
        // 3) subsequent WM_FORMATFRAME's when the window gets
        //    resized or something: do nothing

        // set flag to FALSE to prevent a second adjustment
        psfv->bNeedCnrScroll = 0;

        // increment the number of frame controls
        // to include our status bar
        mrc = (MRESULT)(ulCount + 1);
    } // end if (psfv->hwndStatusBar)
    else
        // no status bar:
        mrc = (MRESULT)ulCount;

    return mrc;
}

/*
 *@@ fdrManipulatePulldown:
 *
 *@@added V1.0.0 (2002-08-28) [umoeller]
 *@@changed V1.0.2 (2003-02-16) [pr]: "Edit"->"Delete" deleted wrong object @@fixes 324
 */

VOID fdrManipulatePulldown(PSUBCLFOLDERVIEW psfv,     // in: frame information
                           SHORT sMenuID,
                           HWND hwndMenuMsg)          // in: mp2 from WM_INITMENU
{
    switch (sMenuID)
    {
        case 0x2CF: // "Folder" pulldown
        case ID_XFM_BAR_FOLDER:     // in split view V1.0.0 (2002-08-28) [umoeller]
            PMPF_MENUS(("  'Folder' pulldown found"));

            // set the "source" object for menu item
            // selections to the folder
            psfv->pSourceObject = psfv->somSelf;
                    // V1.0.0 (2002-08-24) [umoeller]
        break;

        // V1.0.2 (2003-02-16) [pr]: @@fixes 324
        case 0x2D0: // "Edit" submenu
        case ID_XFM_BAR_EDIT:       // in split view V1.0.0 (2002-08-28) [umoeller]
        {
            PMPF_MENUS(("  'Edit' pulldown found"));
            psfv->pSourceObject = wpshQuerySourceObject(psfv->somSelf,
                                                        psfv->hwndCnr,
                                                        TRUE,      // selected mode
                                                        &psfv->ulSelection);
        }
        break;

        case 0x2D1: // "View" submenu
        case ID_XFM_BAR_VIEW:       // in split view V1.0.0 (2002-08-28) [umoeller]
        {
            PMPF_MENUS(("  'View' pulldown found"));

            // set the "source" object for menu item
            // selections to the folder
            psfv->pSourceObject = psfv->somSelf;
                    // V0.9.12 (2001-05-29) [umoeller]
        }
        break;

        /* case 0x2D2:     // "Selected" submenu:
        case ID_XFM_BAR_SELECTED:   // in split view V1.0.0 (2002-08-28) [umoeller]
        break; */

        case 0x2D3: // "Help" submenu: add XFolder product info
        case ID_XFM_BAR_HELP:
            PMPF_MENUS(("  'Help' pulldown found"));

            // set the "source" object for menu item
            // selections to the folder
            psfv->pSourceObject = psfv->somSelf;
                    // V0.9.12 (2001-05-29) [umoeller]
        break;
    }
}

/*
 *@@ fdrInitMenu:
 *      implementation for WM_INITMENU in fnwpSubclWPFolderWindow.
 *      Note that the parent winproc was called first.
 *
 *      WM_INITMENU is sent to a menu owner right before a
 *      menu is about to be displayed. This applies to both
 *      pulldown and context menus.
 *
 *      This is needed for various menu features:
 *
 *      1)  for getting the object which currently has source
 *          emphasis in the folder container, because at the
 *          point WM_COMMAND comes in (and thus wpMenuItemSelected
 *          gets called in response), source emphasis has already
 *          been removed by the WPS.
 *
 *          This is needed for file operations on all selected
 *          objects in the container. We call wpshQuerySourceObject
 *          to find out more about this and store the result in
 *          our SUBCLFOLDERVIEW.
 *
 *          Note that we must ONLY change the source object if
 *          the menu ID is something we recognize as either
 *          the main context menu or the folder menu bar pulldowns.
 *          We get WM_INIMENU for submenus too, and we must
 *          not change the source object then.
 *
 *      2)  for folder content menus, because these are
 *          inserted as empty stubs only in wpModifyPopupMenu
 *          and only filled after the user clicks on them.
 *          We will query a bunch of data first, which we need
 *          later for drawing our items, and then call
 *          mnuFillContentSubmenu, which populates the folder
 *          and fills the menu with the items therein.
 *
 *      3)  for the menu system sounds.
 *
 *      4)  for manipulating Warp 4 folder menu _bars_. We
 *          cannot use the Warp 4 WPS methods defined for
 *          that because we want XWorkplace to run on Warp 3
 *          also.
 *
 *      WM_INITMENU parameters:
 *          SHORT mp1   menu item id
 *          HWND  mp2   menu window handle
 *      Returns: NULL always.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 *@@changed V0.9.4 (2000-07-15) [umoeller]: fixed source object confusion in WM_INITMENU
 *@@changed V0.9.12 (2001-05-29) [umoeller]: fixed broken source object with folder menu bars, which broke new "View" menu items
 *@@changed V0.9.19 (2002-06-18) [umoeller]: optimized, added "batch rename" to view pulldown
 *@@changed V0.9.20 (2002-08-08) [umoeller]: fixed missing hotkey specs for edit and view
 */

VOID fdrInitMenu(PSUBCLFOLDERVIEW psfv,     // in: frame information
                 ULONG sMenuIDMsg,          // in: mp1 from WM_INITMENU
                 HWND hwndMenuMsg)          // in: mp2 from WM_INITMENU
{
    // get XFolder instance data
    XFolderData     *somThis = XFolderGetData(psfv->somSelf);

    PMPF_MENUS(("WM_INITMENU: sMenuIDMsg = %lX, hwndMenuMsg = %lX",
                (ULONG)sMenuIDMsg,
                hwndMenuMsg ));
    PMPF_MENUS(("  psfv->hwndCnr: 0x%lX", psfv->hwndCnr));

    // store object with source emphasis for later use
    // (this gets lost before WM_COMMAND otherwise),
    // but only if the MAIN menu is being opened
    if (sMenuIDMsg == 0x8020) // main menu ID V0.9.4 (2000-07-15) [umoeller]
    {
        // the WPS has a bug in that source emphasis is removed
        // when going thru several context menus, so we must make
        // sure that we do this only when the main menu is opened
        psfv->pSourceObject = wpshQuerySourceObject(psfv->somSelf,
                                                    psfv->hwndCnr,
                                                    FALSE,      // menu mode
                                                    &psfv->ulSelection);

        PMPF_MENUS(("  main context menu"));
    }

    // play system sound
#ifndef __NOXSYSTEMSOUNDS__
    if (    (sMenuIDMsg < 0x8000) // avoid system menu
         || (sMenuIDMsg == 0x8020) // but include context menu
       )
        cmnPlaySystemSound(MMSOUND_XFLD_CTXTOPEN);
#endif

    // find out whether the menu of which we are notified
    // is a folder content menu; if so (and it is not filled
    // yet), the first menu item is ID_XFMI_OFS_DUMMY
    if ((ULONG)WinSendMsg(hwndMenuMsg,
                          MM_ITEMIDFROMPOSITION,
                          (MPARAM)0,        // menu item index
                          MPNULL)
               == (*G_pulVarMenuOfs + ID_XFMI_OFS_DUMMY))
    {
        // okay, let's go
#ifndef __NOFOLDERCONTENTS__
        if (cmnQuerySetting(sfFolderContentShowIcons))
#endif
        {
            // show folder content icons ON:

            PMPF_MENUS(("  content menu, preparing owner draw"));

            cmnuPrepareOwnerDraw(hwndMenuMsg);
        }

        // add menu items according to folder contents
        cmnuFillContentSubmenu(sMenuIDMsg, hwndMenuMsg);
    }
    else
    {
        // no folder content menu:

        // on Warp 4, check if the folder has a menu bar
        if (    (G_fIsWarp4)
             && (sMenuIDMsg == 0x8005)
           )
        {
            PMPF_MENUS(("  seems to be menu bar, ulLastSelMenuItem is %lX",
                    psfv->ulLastSelMenuItem));

            // seems to be some WPS menu item;
            // since the WPS seems to be using this
            // same ID for all the menu bar submenus,
            // we need to check the last selected
            // menu item, which was stored in the psfv
            // structure by WM_MENUSELECT (below)
            fdrManipulatePulldown(psfv,
                                  psfv->ulLastSelMenuItem,
                                  hwndMenuMsg);
        } // end if (SHORT1FROMMP(mp1) == 0x8005)
    }

    PMPF_MENUS(("    pSourceObject is 0x%lX [%s]",
            psfv->pSourceObject,
            (psfv->pSourceObject)
                ? _wpQueryTitle(psfv->pSourceObject)
                : "NULL"));
    PMPF_MENUS(("  ulSelection is %d (%s)",
            psfv->ulSelection,
            (psfv->ulSelection == SEL_WHITESPACE) ? "SEL_WHITESPACE"
            : (psfv->ulSelection == SEL_SINGLESEL) ? "SEL_SINGLESEL"
            : (psfv->ulSelection == SEL_MULTISEL) ? "SEL_MULTISEL"
            : (psfv->ulSelection == SEL_SINGLEOTHER) ? "SEL_SINGLEOTHER"
            : (psfv->ulSelection == SEL_NONEATALL) ? "SEL_NONEATALL"
            : "unknown"));
}

/*
 * MenuSelect:
 *      this gets called from fnwpSubclWPFolderWindow
 *      when WM_MENUSELECT is received.
 *      We need this for three reasons:
 *
 *      1) we will play a system sound, if desired;
 *
 *      2) we need to swallow this for very large folder
 *         content menus, because for some reason, PM will
 *         select a random menu item after we have repositioned
 *         a menu window on the screen (duh);
 *
 *      3) we can intercept certain menu items so that
 *         these don't get passed to wpMenuItemSelected,
 *         which appears to get called when the WPS folder window
 *         procedure responds to WM_COMMAND (which comes after
 *         WM_MENUSELECT only).
 *         This is needed for menu items such as those in
 *         the "Sort" menu so that the menu is not dismissed
 *         after selection.
 *
 *      WM_MENUSELECT parameters:
 *      --  mp1 -- USHORT usItem - selected menu item
 *              -- USHORT usPostCommand - TRUE: if we return TRUE,
 *                  a message will be posted to the owner.
 *      --  mp2 HWND - menu control wnd handle
 *
 *      If we set pfDismiss to TRUE, wpMenuItemSelected will be
 *      called, and the menu will be dismissed.
 *      Otherwise the message will be swallowed.
 *      We return TRUE if the menu item has been handled here.
 *      Otherwise the default wnd proc will be used.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 *@@changed V1.0.0 (2002-08-21) [umoeller]:
 *@@changed V1.0.1 (2002-12-11) [umoeller]: exported this for split view support
 */

BOOL fdrMenuSelect(PSUBCLFOLDERVIEW psfv,   // in: frame information
                   MPARAM mp1,              // in: mp1 from WM_MENUSELECT
                   MPARAM mp2,              // in: mp2 from WM_MENUSELECT
                   BOOL *pfDismiss)         // out: dismissal flag
{
    BOOL        fHandled = FALSE;
            // return value for WM_MENUSELECT;
            // TRUE means dismiss menu

    USHORT      usItem = SHORT1FROMMP(mp1),
                usPostCommand = SHORT2FROMMP(mp1);

    psfv->ulLastSelMenuItem = usItem;

    // _PmpfF(("usPostCommand = 0x%lX", usPostCommand));

    if (    (usPostCommand)
        && (    (usItem <  0x8000) // avoid system menu
             || (usItem == 0x8020) // include context menu
           )
       )
    {
        // HWND hwndCnr = WinWindowFromID(psfv->hwndFrame, FID_CLIENT);
                // we have this in psfv V1.0.1 (2002-12-11) [umoeller]

        // first find out what kind of objects we have here
        WPObject *pObject = psfv->pSourceObject;
                            // set with WM_INITMENU

#ifndef __NOXSYSTEMSOUNDS__
        // play system sound
        cmnPlaySystemSound(MMSOUND_XFLD_CTXTSELECT);
#endif

        PMPF_MENUS(("  Object selections: %d", psfv->ulSelection));

        if (pObject = _xwpResolveIfLink(pObject))
        {
            if (WinGetKeyState(HWND_DESKTOP, VK_SHIFT) & 0x8000)
            {
                // shift is down: then check whether this is an "open view"
                // item and allow changing the object's default view this
                // way V1.0.0 (2002-08-21) [umoeller]
                ULONG   ulMenuId2 = usItem - *G_pulVarMenuOfs;

                if (    (usItem == OPEN_CONTENTS)
                     || (usItem == OPEN_TREE)
                     || (usItem == OPEN_DETAILS)
                     || (ulMenuId2 == ID_XFMI_OFS_SPLITVIEW)
                     || (    (usItem >= 0x1000)
                          && (usItem <= 0x1000 + MAX_ASSOCS_PER_OBJECT)
                        )
                   )
                {
                    USHORT usOldDefaultView = _wpQueryDefaultView(pObject);

                    if (    (usItem != usOldDefaultView)
                         && (_wpSetDefaultView(pObject,
                                               usItem))
                                // we make sure this fails for the desktop
                                // and the split view
                                // V1.0.0 (2002-09-13) [umoeller]
                       )
                    {
                        PMPF_MENUS(("  un-checking 0x%lX in hMenu 0x%lX",
                                    usOldDefaultView,
                                    mp2));

                        WinSendMsg((HWND)mp2,
                                   MM_SETITEMATTR,
                                   MPFROM2SHORT(usOldDefaultView,
                                                FALSE),
                                   MPFROM2SHORT(MIA_CHECKED,
                                                0));

                        WinSendMsg((HWND)mp2,
                                   MM_SETITEMATTR,
                                   MPFROM2SHORT(usItem,
                                                FALSE),
                                   MPFROM2SHORT(MIA_CHECKED,
                                                MIA_CHECKED));
                        _wpSaveDeferred(pObject);
                    }

                    fHandled = TRUE;
                    *pfDismiss = FALSE;
                }
            }

            // now call the functions in fdrmenus.c for this,
            // depending on the class of the object for which
            // the menu was opened
            if (    (!fHandled)
                 && (_somIsA(pObject, _WPFileSystem))
               )
            {
                fHandled = fcmdSelectingFsysMenuItem(
                               psfv->pSourceObject,
                                    // set in WM_INITMENU;
                                    // note that we're passing
                                    // psfv->pSourceObject instead of pObject;
                                    // psfv->pSourceObject might be a shadow!
                               usItem,
                               (BOOL)usPostCommand,
                               (HWND)mp2,               // hwndMenu
                               psfv->hwndCnr,
                               psfv->ulSelection,       // SEL_* flags
                               pfDismiss);              // dismiss-menu flag

                if (    (!fHandled)
                     && (_somIsA(pObject, _WPFolder))
                   )
                {
                    fHandled = fcmdSelectingFdrMenuItem(pObject,
                                   usItem,
                                   (BOOL)usPostCommand, // fPostCommand
                                   (HWND)mp2,               // hwndMenu
                                   psfv->hwndCnr,
                                   psfv->ulSelection,       // SEL_* flags
                                   pfDismiss);              // dismiss-menu flag
                }
            }

            if (    (fHandled)
                 && (!(*pfDismiss))
               )
            {
                // menu not to be dismissed: set the flag
                // which will remove cnr source
                // emphasis when the main menu is dismissed
                // later (WM_ENDMENU msg here)
                psfv->fRemoveSourceEmphasis = TRUE;
            }
        }
    }

    return fHandled;
}

/*
 *@@ fdrWMCommand:
 *      implementation for WM_COMMAND in fdrProcessFolderMsgs.
 *
 *      Returns TRUE if the item was processed, FALSE otherwise
 *      and the parent func should be called.
 *
 *      Also gets called externally from the split view frame.
 *
 *@@added V1.0.0 (2002-08-28) [umoeller]
 *@@changed V1.0.8 (2007-08-26) [pr]: prevent trap on null SOM pointer
 */

BOOL fdrWMCommand(PSUBCLFOLDERVIEW psfv,
                  SHORT sCommand)
{
    BOOL    brc = FALSE;

    // resolve method by name
    somTD_XFolder_xwpProcessViewCommand pxwpProcessViewCommand = NULL;

    if (psfv->somSelf)  // V1.0.8 (2007-08-26) [pr]
        pxwpProcessViewCommand = (somTD_XFolder_xwpProcessViewCommand)somResolveByName(
                              psfv->somSelf,
                              "xwpProcessViewCommand");

    if (    (pxwpProcessViewCommand)
         && (pxwpProcessViewCommand(psfv->somSelf,
                                    sCommand,
                                    psfv->hwndCnr,
                                    psfv->pSourceObject,
                                    psfv->ulSelection))
       )
        brc = TRUE;

    psfv->pSourceObject = NULL;

    return brc;
}


/*
 *@@ WMChar_Delete:
 *      this gets called if "delete into trash can"
 *      is enabled and WM_CHAR has been detected with
 *      the "Delete" key. We start a file task job
 *      to delete all selected objects in the container
 *      into the trash can, using the oh-so-much advanced
 *      functions in fileops.c.
 *
 *@@added V0.9.1 (2000-01-31) [umoeller]
 *@@changed V0.9.9 (2001-02-16) [umoeller]: added "shift-delete" support; thanks [pr]
 *@@changed V0.9.19 (2002-04-02) [umoeller]: fixed broken true delete if trashcan is disabled
 */

STATIC VOID WMChar_Delete(PSUBCLFOLDERVIEW psfv,
                          BOOL fTrueDelete)             // in: do true delete instead of trash?
{
    ULONG       ulSelection = 0;
    WPObject    *pSelected = 0;

    pSelected = wpshQuerySourceObject(psfv->somSelf,
                                      psfv->hwndCnr,
                                      TRUE,       // keyboard mode
                                      &ulSelection);
    PMPF_TRASHCAN(("WM_CHAR delete: first obj is %s",
                (pSelected) ? _wpQueryTitle(pSelected) : "NULL"));

    if (    (pSelected)
         && (ulSelection != SEL_NONEATALL)
       )
    {
        // collect objects from cnr and start
        // moving them to trash can
        APIRET frc = fopsStartDeleteFromCnr(NULLHANDLE,   // no anchor block, ansynchronously
                                            pSelected,    // first selected object
                                            ulSelection,  // can only be SEL_SINGLESEL
                                                          // or SEL_MULTISEL
                                            psfv->hwndCnr,
                                            fTrueDelete);  // V0.9.19 (2002-04-02) [umoeller]
        PMPF_TRASHCAN(("    got APIRET %d", frc));
    }
}

/*
 *@@ WMChar:
 *      handler for WM_CHAR in fdrProcessFolderMsgs.
 *
 *      Returns TRUE if msg was processed and should
 *      be swallowed.
 *
 *@@added V0.9.18 (2002-03-23) [umoeller]
 *@@changed V0.9.19 (2002-04-02) [umoeller]: fixed broken true delete if trashcan is disabled
 *@@changed V1.0.0 (2002-08-24) [umoeller]: fixed key up/down processing
 */

STATIC BOOL WMChar(HWND hwndFrame,
                   PSUBCLFOLDERVIEW psfv,
                   MPARAM mp1,
                   MPARAM mp2)
{
    USHORT usFlags    = SHORT1FROMMP(mp1);

    XFolderData         *somThis = XFolderGetData(psfv->somSelf);

    USHORT usch       = SHORT1FROMMP(mp2);
    USHORT usvk       = SHORT2FROMMP(mp2);

    // if (!(usFlags & KC_KEYUP))       removed, process both up and down V1.0.0 (2002-08-24) [umoeller]

    // intercept DEL key
    if (    (usFlags & KC_VIRTUALKEY)
         && (usvk == VK_DELETE)
       )
    {
        // check whether "delete to trash can" is on
#ifndef __ALWAYSTRASHANDTRUEDELETE__
        if (cmnQuerySetting(sfReplaceDelete))       // V0.9.19 (2001-04-13) [umoeller]
#endif
        {
            BOOL fTrueDelete;

            // use true delete if the user doesn't want the
            // trash can or if the shift key is pressed
            if (!(fTrueDelete = cmnQuerySetting(sfAlwaysTrueDelete)))
                fTrueDelete = doshQueryShiftState();

            if (!(usFlags & KC_KEYUP))
                WMChar_Delete(psfv,
                              fTrueDelete);

            // swallow this key,
            // do not process default winproc
            return TRUE;
        }
    }

    // check whether folder hotkeys are allowed at all
    if (
#ifndef __ALWAYSFDRHOTKEYS__
            (cmnQuerySetting(sfFolderHotkeys))
         &&
#endif
            // yes: check folder and global settings
            (    (_bFolderHotkeysInstance == 1)
              || (    (_bFolderHotkeysInstance == 2)   // use global settings:
                   && (cmnQuerySetting(sfFolderHotkeysDefault))
                 )
            )
       )
    {
        // fdrProcessFldrHotkey returns TRUE if this key
        // is a folder hotkey (for both key up and down now)
        // but posts the command only for key down
        // V1.0.0 (2002-08-24) [umoeller]
        if (fdrProcessFldrHotkey(psfv->somSelf,
                                 hwndFrame,
                                 usFlags,
                                 usch,
                                 usvk))
        {
            // was a hotkey:
            // swallow this key,
            // do not process default winproc
            return TRUE;
        }
    }

    return FALSE;
}

    /*
        typedef struct _OWNERITEM {
          HWND      hwnd;            //  Window handle.
          HPS       hps;             //  Presentation-space handle.
          ULONG     fsState;         //  State.
          ULONG     fsAttribute;     //  Attribute.
          ULONG     fsStateOld;      //  Old state.
          ULONG     fsAttributeOld;  //  Old attribute.
          RECTL     rclItem;         //  Item rectangle.
          LONG      idItem;          //  Item identity.
          ULONG     hItem;           //  Item.
        } OWNERITEM;

        The following list defines the OWNERITEM data structure fields as they apply to the
        container control. See OWNERITEM for the default field values.

        hwnd (HWND)
                 Handle of the window in which ownerdraw will occur. The following is a list
                 of the window handles that can be specified for ownerdraw:

                     The container window handle of the icon, name, text, and tree views
                     The container title window handle
                     The left or right window handles of the details view
                     The left or right column heading windows of the details view.

        hps (HPS)
                 Handle of the presentation space of the container window. For the details
                 view that uses a split bar, the presentation space handle is either for the
                 left or right window, depending upon the position of the column. If the
                 details view does not have a split bar, the presentation space handle is for
                 the left window.

        fsState (ULONG)
                 Specifies emphasis flags. This state is not used by the container control
                 because the application is responsible for drawing the emphasis states
                 during ownerdraw.

        fsAttribute (ULONG)
                 Attributes of the record as given in the flRecordAttr field in the
                 RECORDCORE data structure.

        fsStateOld (ULONG)
                 Previous emphasis. This state is not used by the container control because
                 the application is responsible for drawing the emphasis states during
                 ownerdraw.

        fsAttributeOld (ULONG)
                 Previous attribute. This state is not used by the container control because
                 the application is responsible for drawing the emphasis states during
                 ownerdraw.

        rclItem (RECTL)
                 This is the bounding rectangle into which the container item is drawn.

                 If the container item is an icon/text or bit-map/text pair, two
                 WM_DRAWITEM messages are sent to the application. The first
                 WM_DRAWITEM message contains the rectangle bounding the icon or bit map
                 and the second contains the rectangle bounding the text.

                 If the container item contains only text, or only an icon or bit map, only
                 one WM_DRAWITEM message is sent. However, if the current view is the
                 tree icon or tree text view and if the item is a parent item, the application
                 will receive an additional WM_DRAWITEM (in Container Controls) message.
                 The additional message is for the icon or bit map that indicates whether the
                 parent item is expanded or collapsed.

                 If the current view is the details view and the CFA_OWNER attribute is set,
                 the rectangle's size is equal to the width of the column and the height of
                 the tallest field in the container item. CFA_OWNER is an attribute of the
                 FIELDINFO data structure's flData field.

        idItem (ULONG)
                 Identifies the item being drawn. It can be one of the following:

                     CMA_CNRTITLE
                     CMA_ICON
                     CMA_TEXT
                     CMA_TREEICON.

                 This field is not used for the details view and is set to 0.

        hItem (CNRDRAWITEMINFO)
                 Pointer to a CNRDRAWITEMINFO structure.

                 typedef struct _CNRDRAWITEMINFO {
                   PRECORDCORE     pRecord;     //  RECORDCORE structure for the record being drawn.
                   PFIELDINFO      pFieldInfo;  //  FIELDINFO structure for the container column
                                                // being drawn in the details view. This is only
                                                // != NULL if we're in details view.
                 } CNRDRAWITEMINFO;
    */

/*
 *@@ CnrDrawIcon:
 *      helper called from to owner-draw an icon.
 *
 *      flOwnerDraw has the flags that determine what
 *      owner draw we replace. If the top bit
 *      (0x80000000) is also set, we always draw the
 *      mini-icon, no matter what the size of the
 *      paint rectangle is (for Details view).
 *
 *      If we've drawn, we return TRUE.
 *
 *      Yes, this func is slightly complex because we
 *      have to imitate the container's behavior for
 *      in-use, selected, and cursored emphasis,
 *      including all the bugs with wrong coordinates
 *      passed in and other special cases.
 *
 *@@added V0.9.20 (2002-08-04) [umoeller]
 *@@changed V1.0.0 (2002-08-21) [umoeller]: fixed painting problems for folder shadows
 *@@changed V1.0.0 (2002-09-17) [umoeller]: fixed another deadlock for folder shadows
 *@@changed V1.0.1 (2002-12-08) [umoeller]: exported chunks to new _xwpOwnerDrawIcon method
 */

STATIC BOOL CnrDrawIcon(HWND hwndCnr,               // in: container HWND (we can't use poi->hwnd)
                        ULONG flOwnerDraw,          // in: OWDRFL_* flags
                        POWNERITEM poi)             // in: mp2 of WM_DRAWITEM
{
    PMINIRECORDCORE     pmrc;
    WPObject            *somSelf;
    BOOL                brc = FALSE;

    // get the object from the record that is to be drawn
    if (    (pmrc = (PMINIRECORDCORE)((PCNRDRAWITEMINFO)poi->hItem)->pRecord)
         && (somSelf = OBJECT_FROM_PREC(pmrc))
       )
    {
        LONG        cx,
                    cy;
        POINTL      ptl2;
        RECTL       rclBack,
                    rclPaint;
        LONG        lCursorOffset = 2;
                        // offset to move outwards from "selected" rectangle
                        // to dotted "cursored" rectangle; negative moves inwards
                        // (for Details view)
        HPOINTER    hptr2;
        ULONG       flMethodRC = 0;

        /*
         * 1) calculate coordinates:
         *
         */

        // determine whether to draw mini icon and set
        // up x and y for the icon to be centered in
        // the rectangle
        cx = poi->rclItem.xRight - poi->rclItem.xLeft;
        cy = poi->rclItem.yTop - poi->rclItem.yBottom;

        PMPF_ICONREPLACEMENTS(("    cx = %d, cy = %d", cx, cy));

        // set the OWDRFL_INUSE flag if applicable
        if (poi->fsAttribute & CRA_INUSE)
            flOwnerDraw |= OWDRFL_INUSE;

        if (    (flOwnerDraw & OWDRFL_MINI)      // force mini-icon (Details view)?
             || (cx < G_cxIcon)
           )
        {
            LONG    cxIcon = G_cxIcon / 2,
                    cyIcon = G_cyIcon / 2;

            rclPaint.xLeft   =   poi->rclItem.xLeft
                               + (cx - cxIcon) / 2;
            rclPaint.xRight  = rclPaint.xLeft + cxIcon;
            rclPaint.yBottom =   poi->rclItem.yBottom
                               + (cy - cyIcon) / 2;
            rclPaint.yTop    = rclPaint.yBottom + cyIcon;

            // in Details view, the "selected" rectangle is the
            // same size as the poi rectangle, but the cursor
            // is painted INSIDE
            if (flOwnerDraw & OWDRFL_MINI)
            {
                memcpy(&rclBack, &poi->rclItem, sizeof(RECTL));

                lCursorOffset = -2;
            }
            else
            {
                rclBack.xLeft = poi->rclItem.xLeft + 2;
                rclBack.yBottom = poi->rclItem.yBottom + 2;
                rclBack.xRight = poi->rclItem.xRight - 2;
                rclBack.yTop = poi->rclItem.yTop - 2;
            }

            flOwnerDraw |= OWDRFL_MINI;
        }
        else
        {
            rclPaint.xLeft   =   poi->rclItem.xLeft
                               + (cx - G_cxIcon) / 2
                               + 1;
            rclPaint.xRight  = rclPaint.xLeft + G_cxIcon;
            rclPaint.yBottom =   poi->rclItem.yBottom
                               + (cy - G_cyIcon) / 2
                               + 1;
            rclPaint.yTop    = rclPaint.yBottom + G_cyIcon;

            rclBack.xLeft = poi->rclItem.xLeft + 2;
            rclBack.yBottom = poi->rclItem.yBottom + 2;
            rclBack.xRight = poi->rclItem.xRight - 1;
            rclBack.yTop = poi->rclItem.yTop - 1;
        }

        /*
         * 2) draw emphasis
         *
         */

        // now, with owner draw, the container doesn't do emphasis
        // for us EXCEPT source and target emphasis... so we need
        // to draw a background rectangle if the object is selected
        if (poi->fsAttribute & (CRA_INUSE | CRA_SELECTED | CRA_CURSORED))
        {
            LONG    lcolHiliteBgnd;

            PMPF_ICONREPLACEMENTS(("[%s] CRA_SELECTED", pmrc->pszIcon));

            // switch the HPS to RGB mode, or the below won't work
            if (GpiCreateLogColorTable(poi->hps,
                                       0,
                                       LCOLF_RGB,
                                       0,
                                       0,
                                       0))
                // tell instance method that RGB is already in RGB mode
                flOwnerDraw |= OWDRFL_RGBMODE;

            lcolHiliteBgnd = winhQueryPresColor2(hwndCnr,
                                                 PP_SHADOWHILITEBGNDCOLOR,
                                                 PP_SHADOWHILITEBGNDCOLORINDEX,
                                                 TRUE,      // inherit
                                                 SYSCLR_SHADOWHILITEBGND);

            if (poi->fsAttribute & CRA_SELECTED)
                WinFillRect(poi->hps,
                            &rclBack,
                            lcolHiliteBgnd);

            if (poi->fsAttribute & CRA_INUSE)
            {
                LONG    lcolHiliteFgnd,
                        lOldPattern;

                if (poi->fsAttribute & CRA_SELECTED)
                    // if we're in-use AND selected,
                    // use the highlite foreground color
                    lcolHiliteFgnd = winhQueryPresColor2(hwndCnr,
                                                         PP_HILITEFOREGROUNDCOLOR,
                                                         PP_HILITEFOREGROUNDCOLORINDEX,
                                                         TRUE,     // inherit
                                                         SYSCLR_HILITEFOREGROUND);
                else
                    // if we're in-use and NOT selected,
                    // use plain-text (NOT SHADOW) foreground color
                    lcolHiliteFgnd = winhQueryPresColor2(hwndCnr,
                                                         PP_FOREGROUNDCOLOR,
                                                         PP_FOREGROUNDCOLORINDEX,
                                                         TRUE,     // inherit
                                                         SYSCLR_WINDOWTEXT);

                GpiSetColor(poi->hps,
                            lcolHiliteFgnd);
                lOldPattern = GpiQueryPattern(poi->hps);
                GpiSetPattern(poi->hps, PATSYM_DIAG1);

                ptl2.x = rclBack.xLeft;
                ptl2.y = rclBack.yBottom;
                GpiMove(poi->hps, &ptl2);
                // yes, the following two SHOULD be inclusive, but the cnr
                // is buggy too and will draw one pixel too much to the top
                // and right for in-use as well. This can easily be checked
                // by selecting an icon with in-use emphasis... I think we
                // should imitate this.
                ptl2.x  = rclBack.xRight;
                ptl2.y  = rclBack.yTop;
                GpiBox(poi->hps, DRO_FILL, &ptl2, 0L, 0L);

                GpiSetPattern(poi->hps, lOldPattern); // PATSYM_DEFAULT);
            }

            if (poi->fsAttribute & CRA_CURSORED)
            {
                LONG    lOldLineType = GpiQueryLineType(poi->hps);

                GpiSetColor(poi->hps, lcolHiliteBgnd);
                GpiSetLineType(poi->hps, LINETYPE_ALTERNATE);

                ptl2.x = rclBack.xLeft - lCursorOffset;
                ptl2.y = rclBack.yBottom - lCursorOffset;
                GpiMove(poi->hps, &ptl2);
                ptl2.x = rclBack.xRight + lCursorOffset - 1;     // inclusive!
                ptl2.y = rclBack.yTop + lCursorOffset - 1;       // inclusive!
                GpiBox(poi->hps, DRO_OUTLINE, &ptl2, 0, 0);

                GpiSetLineType(poi->hps, lOldLineType);
                            // was missing V1.0.0 (2002-08-21) [umoeller]
            }
        }

        /*
         * 3) draw icon
         *
         */

        // the template icons are always drawn via ownerdraw,
        // so we have to do that too
        if (!(_wpQueryStyle(somSelf) & OBJSTYLE_TEMPLATE))
            flMethodRC = _xwpOwnerDrawIcon(somSelf,
                                           pmrc,
                                           poi->hps,
                                           flOwnerDraw,
                                           &rclPaint);
                // V1.0.1 (2002-12-08) [umoeller]
        else
        {
            // template:
            if (!cmnGetStandardIcon(STDICON_TEMPLATE,
                                    &hptr2,
                                    NULL,
                                    NULL))
            {
                WinDrawPointer(poi->hps,
                               rclPaint.xLeft,
                               rclPaint.yBottom,
                               hptr2,
                               (flOwnerDraw & OWDRFL_MINI)
                                   ? DP_MINI
                                   : DP_NORMAL);

                // we can't do a mini-mini icon over a
                // mini template icon, so paint the real
                // icon only if we weren't mini yet
                if (!(flOwnerDraw & OWDRFL_MINI))
                {
                    RECTL rcl2;
                    rcl2.xLeft = rclPaint.xLeft + G_cxIcon / 8;
                    rcl2.xRight = rcl2.xLeft + G_cxIcon / 2;
                    rcl2.yBottom = rclPaint.yBottom + G_cyIcon * 5 / 16;
                    rcl2.yTop = rcl2.yBottom + G_cyIcon / 2;

                    flMethodRC = _xwpOwnerDrawIcon(somSelf,
                                                   pmrc,
                                                   poi->hps,
                                                   flOwnerDraw | OWDRFL_MINI,
                                                   &rcl2);
                        // V1.0.1 (2002-12-08) [umoeller]
                }
            }
        }

        // overpaint with shadow overlay icon, if allowed
        if (    (flOwnerDraw & OWDRFL_SHADOWOVERLAY)
             && (objQueryFlags(somSelf) & OBJFL_WPSHADOW)
             && (!cmnGetStandardIcon(STDICON_SHADOWOVERLAY,
                                     &hptr2,
                                     NULL,
                                     NULL))
           )
        {
            WinDrawPointer(poi->hps,
                           rclPaint.xLeft,
                           rclPaint.yBottom,
                           hptr2,
                           (flOwnerDraw & OWDRFL_MINI)
                                   ? DP_MINI
                                   : DP_NORMAL);
        }

        if (flMethodRC & OWDRFL_RGBMODE)
            GpiCreateLogColorTable(poi->hps,
                                   LCOL_RESET,
                                   LCOLF_CONSECRGB,
                                   0,
                                   0,
                                   0);

        if (flMethodRC & flOwnerDraw & OWDRFL_LAZYLOADICON)
            icomQueueLazyIcon(somSelf);
        else if (flMethodRC & flOwnerDraw & OWDRFL_LAZYLOADTHUMBNAIL)
            icomQueueLazyIcon(somSelf);     // @@todo

        brc = TRUE;
    }

    return brc;
}

/*
 *@@ CnrDrawItem:
 *      processor for WM_DRAWITEM in fdrProcessFolderMsgs.
 *
 *      This allows us to support lazy icons in subclassed
 *      folder views without affecting the behavior of
 *      _wpQueryIcon in general, because that might be
 *      called from places that will need an icon
 *      _immediately_.
 *
 *      If this returns TRUE, we do NOT call the parent
 *      WPS window proc and return TRUE from WM_DRAWITEM,
 *      meaning that we've drawn the item ourselves.
 *
 *@@added V0.9.20 (2002-07-31) [umoeller]
 *@@changed V1.0.5 (2006-06-12) [pr]: @@fixes 371
 */

STATIC BOOL CnrDrawItem(PSUBCLFOLDERVIEW psfv,      // in: folder view data
                        POWNERITEM poi)             // in: mp2 of WM_DRAWITEM
{
    ULONG               flOwnerDraw;
    BOOL                fWeveDrawn = FALSE;
    PCNRDRAWITEMINFO    pcdi;
    CNRINFO             CnrInfo;

    // if none of the owner-draw settings are enabled,
    // get outta here and call the WPS
    if (!(flOwnerDraw = cmnQuerySetting(sflOwnerDrawIcons)))
        return FALSE;

    // If container in Bitmap mode e.g. Light Table folders, let the WPS handle the owner draw
    // V1.0.5 (2006-06-12) [pr]: @@fixes 371
    cnrhQueryCnrInfo(psfv->hwndCnr, &CnrInfo);
    if (CnrInfo.flWindowAttr & CA_DRAWBITMAP)
        return FALSE;

    if (pcdi = (PCNRDRAWITEMINFO)poi->hItem)
    {
        PFIELDINFO pfi;

        // for Details view, pFieldInfo is != NULL always
        // and represents the column to be drawn.... we'll
        // let the WPS handle everything EXCEPT the icon column
        if (pfi = pcdi->pFieldInfo)
        {
            if (pfi->offStruct == FIELDOFFSET(MINIRECORDCORE, hptrIcon))
                fWeveDrawn = CnrDrawIcon(psfv->hwndCnr,
                                         flOwnerDraw | OWDRFL_MINI,
                                         poi);
            // else other column: let WPS do the work
        }
        else
        {
            // not Details view: check what needs to be drawn

            // poi->idItem is one of
            // -- CMA_CNRTITLE
            // -- CMA_ICON
            // -- CMA_TEXT
            // -- CMA_TREEICON

            switch (poi->idItem)
            {
                case CMA_ICON:
                    fWeveDrawn = CnrDrawIcon(psfv->hwndCnr,
                                             flOwnerDraw,
                                             poi);
                break;  // CMA_ICON
            } // end switch (poi->idItem)
        }
    }

    return fWeveDrawn;
}

/*
 *@@ ProcessFolderMsgs:
 *      actual folder view message processing. Called
 *      from fnwpSubclWPFolderWindow. See remarks
 *      there.
 *
 *      This has been separated from fnwpSubclWPFolderWindow
 *      for split view (and file dialog) support. Those
 *      windows do not use fnwpSubclWPFolderWindow, but
 *      rather have their own window procs for the main
 *      frame and the left and right frames and would
 *      like to call this func explicitly.
 *
 *      This is maybe the most central (and most complex) part of
 *      XWorkplace. Since most WPS methods are really just reacting
 *      to messages in the default WPS frame window proc, but for
 *      some features methods are just not sufficient, basically we
 *      simulate what the WPS does here by intercepting _lots_
 *      of messages before the WPS gets them.
 *
 *      Unfortunately, this leads to quite a mess, but we have
 *      no choice.
 *
 *      Things we do in this proc:
 *
 *      --  frame control manipulation for status bars
 *
 *      --  Warp 4 folder menu bar manipulation (WM_INITMENU)
 *
 *      --  handling of certain menu items w/out dismissing
 *          the menu; this calls functions in fdrmenus.c
 *
 *      --  menu owner draw (folder content menus w/ icons);
 *          this calls functions in fdrmenus.c also
 *
 *      --  complete interception of file-operation menu items
 *          such as "delete" for deleting all objects into the
 *          XWorkplace trash can; this is now done thru a
 *          new method, which can be overridden by WPFolder
 *          subclasses (such as the trash can). See
 *          XFolder::xwpProcessViewCommand.
 *
 *      --  container control messages: tree view auto-scroll,
 *          updating status bars etc.
 *
 *      --  playing the new system sounds for menus and such.
 *
 *      Note that this function calls lots of "external" functions
 *      spread across all over the XWorkplace code. I have tried to
 *      reduce the size of this window procedure to an absolute
 *      minimum because this function gets called very often (for
 *      every single folder message) and large message procedures
 *      may thrash the processor caches.
 *
 *@@added V0.9.3 (2000-04-08) [umoeller]
 *@@changed V0.9.7 (2001-01-13) [umoeller]: introduced xwpProcessViewCommand for WM_COMMAND
 *@@changed V0.9.9 (2001-03-11) [umoeller]: renamed from ProcessFolderMsgs, exported now
 *@@changed V0.9.20 (2002-07-31) [umoeller]: added support for lazy icons in WM_DRAWITEM
 */

MRESULT fdrProcessFolderMsgs(HWND hwndFrame,
                             ULONG msg,
                             MPARAM mp1,
                             MPARAM mp2,
                             PSUBCLFOLDERVIEW psfv,     // in: folder view data
                             PFNWP pfnwpOriginal)       // in: original frame window proc
{
    MRESULT         mrc = 0;
    BOOL            fCallDefault = FALSE;

    TRY_LOUD(excpt1)
    {
        switch(msg)
        {
            /* *************************
             *
             * Status bar
             *
             **************************/

            /*
             *  The following code adds status bars to folder frames.
             *  The XFolder status bars are implemented as frame controls
             *  (similar to the title-bar buttons and menus). In order
             *  to do this, we need to intercept the following messages
             *  which are sent to the folder's frame window when the
             *  frame is updated as a reaction to WM_UPDATEFRAME or WM_SIZE.
             *
             *  Note that wpOpen has created the status bar window (which
             *  is a static control subclassed with fdr_fnwpStatusBar) already
             *  and stored the HWND in the SUBCLFOLDERVIEW.hwndStatusBar
             *  structure member (which psfv points to now).
             *
             *  Here we only relate the status bar to the frame. The actual
             *  painting etc. is done in fdr_fnwpStatusBar.
             */

            /*
             * WM_QUERYFRAMECTLCOUNT:
             *      this gets sent to the frame when PM wants to find out
             *      how many frame controls the frame has. According to what
             *      we return here, SWP structures are allocated for WM_FORMATFRAME.
             *      We call the "parent" window proc and add one for the status bar.
             */

            case WM_QUERYFRAMECTLCOUNT:
            {
                // query the standard frame controls count
                ULONG ulrc = (ULONG)pfnwpOriginal(hwndFrame, msg, mp1, mp2);

                // if we have a status bar, increment the count
                if (psfv->hwndStatusBar)
                    ulrc++;

                mrc = (MPARAM)ulrc;
            }
            break;

            /*
             * WM_FORMATFRAME:
             *    this message is sent to a frame window to calculate the sizes
             *    and positions of all of the frame controls and the client window.
             *
             *    Parameters:
             *          mp1     PSWP    pswp        structure array; for each frame
             *                                      control which was announced with
             *                                      WM_QUERYFRAMECTLCOUNT, PM has
             *                                      allocated one SWP structure.
             *          mp2     PRECTL  pprectl     pointer to client window
             *                                      rectangle of the frame
             *          returns USHORT  ccount      count of the number of SWP
             *                                      arrays returned
             *
             *    It is the responsibility of this message code to set up the
             *    SWP structures in the given array to position the frame controls.
             *    We call the "parent" window proc and then set up our status bar.
             */

            case WM_FORMATFRAME:
                mrc = FormatFrame2(psfv, mp1, mp2, pfnwpOriginal);
            break;

            /*
             * WM_CALCFRAMERECT:
             *     this message occurs when an application uses the
             *     WinCalcFrameRect function.
             *
             *     Parameters:
             *          mp1     PRECTL  pRect      rectangle structure
             *          mp2     USHORT  usFrame    frame indicator
             *          returns BOOL    rc         rectangle-calculated indicator
             */

            case WM_CALCFRAMERECT:
            {
                XFRAMECONTROLS  xfc =
                {
                    pfnwpOriginal,
                    NULLHANDLE,
                    psfv->hwndStatusBar,
                    0,
                    cmnQueryStatusBarHeight()
                };
                ctlCalcExtFrameRect(hwndFrame,
                                    &xfc,
                                    mp1,
                                    mp2);
            }
            break;

            /* *************************
             *
             * Menu items:
             *
             **************************/

            /*
             * WM_INITMENU:
             *      this message is sent to a frame whenever a menu
             *      is about to be displayed. This is needed for
             *      various menu features; see InitMenu() above.
             */

            case WM_INITMENU:
                // call the default, in case someone else
                // is subclassing folders (ObjectDesktop?!?);
                // from what I've checked, the WPS does NOTHING
                // with this message, not even for menu bars...
                mrc = pfnwpOriginal(hwndFrame, msg, mp1, mp2);

                // added V0.9.3 (2000-03-28) [umoeller]
                fdrInitMenu(psfv,
                            (ULONG)mp1,
                            (HWND)mp2);
            break;

            /*
             * WM_MENUSELECT:
             *      this is SENT to a menu owner by the menu
             *      control to determine what to do right after
             *      a menu item has been selected. If we return
             *      TRUE, the menu will be dismissed.
             *
             *      See MenuSelect() above.
             */

            case WM_MENUSELECT:
            {
                BOOL fDismiss = TRUE;

                PMPF_MENUS(("WM_MENUSELECT: mp1 = %lX/%lX, mp2 = %lX",
                            SHORT1FROMMP(mp1),
                            SHORT2FROMMP(mp1),
                            mp2 ));

                // always call the default, in case someone else
                // is subclassing folders (ObjectDesktop?!?)
                mrc = pfnwpOriginal(hwndFrame, msg, mp1, mp2);

                // added V0.9.3 (2000-03-28) [umoeller]
                // now handle our stuff; this might modify mrc to
                // have the menu stay on the screen
                if (fdrMenuSelect(psfv,
                                  mp1,
                                  mp2,
                                  &fDismiss))
                    // processed: return the modified flag instead
                    mrc = (MRESULT)fDismiss;
            }
            break;

            /*
             * WM_MENUEND:
             *      this message occurs when a menu control is about to
             *      terminate. We need to remove cnr source emphasis
             *      if the user has requested a context menu from a
             *      status bar.
             *
             *      Note: WM_MENUEND comes in BEFORE WM_COMMAND.
             */

            case WM_MENUEND:
                PMPF_MENUS(("WM_MENUEND: mp1 = %lX, mp2 = %lX",
                            mp1, mp2 ));

                // added V0.9.3 (2000-03-28) [umoeller]

                // menu opened from status bar?
                if (psfv->fRemoveSourceEmphasis)
                {
                    // if so, remove cnr source emphasis
                    /* WinSendMsg(psfv->hwndCnr,
                               CM_SETRECORDEMPHASIS,
                               (MPARAM)NULL,   // undocumented: if precc == NULL,
                                               // the whole cnr is given emphasis
                               MPFROM2SHORT(FALSE,  // remove emphasis
                                            CRA_SOURCE)); */
                    // and make sure the container has the
                    // focus
                    // WinSetFocus(HWND_DESKTOP, psfv->hwndCnr);
                    // reset flag for next context menu
                    psfv->fRemoveSourceEmphasis = FALSE;
                }

                // mrc = pfnwpOriginal(hwndFrame, msg, mp1, mp2);
                fCallDefault = TRUE;        // V1.0.1 (2002-12-15) [umoeller]
            break;

            /*
             * WM_MEASUREITEM:
             *      this msg is sent only once per owner-draw item when
             *      PM needs to know its size. This gets sent to us for
             *      items in folder content menus (if icons are on); the
             *      height of our items will be the same as with
             *      non-owner-draw ones, but we need to calculate the width
             *      according to the item text.
             *
             *      Return value: check mnuMeasureItem.
             */

            case WM_MEASUREITEM:
                if (   (SHORT)mp1
                     > *G_pulVarMenuOfs + ID_XFMI_OFS_VARIABLE
                   )
                {
                    // call the measure-item func in fdrmenus.c
                    mrc = cmnuMeasureItem((POWNERITEM)mp2);
                }
                else
                    // none of our items: pass to original wnd proc
                    mrc = pfnwpOriginal(hwndFrame, msg, mp1, mp2);
            break;

            /* *************************
             *
             * Owner draw
             *
             **************************/

            /*
             * WM_DRAWITEM:
             *      comes in for every owner-draw request.
             *      The WPS does a _lot_ of owner draw painting in
             *      containers. This is what we hack up here to support
             *      lazy icons and a bunch of other nice things.
             *
             *      We hack this up for two purposes:
             *
             *      --  owner-drawing folder content menu items;
             *
             *      --  owner-drawing WPS object records to support
             *          lazy icon loading (new with V0.9.20).
             *
             *      Parameters:
             *
             *      --  USHORT mp1: ID of the item to be drawn. If it's
             *          FID_CLIENT, it's the container.
             *
             *      --  POWNERITEM mp2: owner draw information.
             *
             *      If this returns FALSE, the request has not been
             *      handled, and the container should draw the item
             *      instead.
             */

            case WM_DRAWITEM:
                if ((USHORT)mp1 == FID_CLIENT)
                {
                    // WPS container draw:
                    // intercept this for lazy icons
                    // V0.9.20 (2002-07-31) [umoeller]
                    if (!(mrc = (MRESULT)CnrDrawItem(psfv,
                                                     (POWNERITEM)mp2)))
                        // item not drawn:
                        fCallDefault = TRUE;
                }
                else if (   (SHORT)mp1
                          > *G_pulVarMenuOfs + ID_XFMI_OFS_VARIABLE
                        )
                {
                    // variable menu item: this must be a folder-content
                    // menu item, because for others no WM_DRAWITEM is sent
                    if (cmnuDrawItem(mp1, mp2))
                        mrc = (MRESULT)TRUE;
                    else // error occurred:
                        fCallDefault = TRUE;    // V0.9.3 (2000-04-26) [umoeller]
                }
                else
                    fCallDefault = TRUE;    // V0.9.3 (2000-04-26) [umoeller]
            break;

            /* *************************
             *
             * Miscellaneae:
             *
             **************************/

            /*
             * WM_COMMAND:
             *      this is intercepted to provide "delete" menu
             *      item support if "delete into trashcan"
             *      is on. We cannot use wpMenuItemSelected for
             *      that because that method gets called for
             *      every object, and we'd never know when it's
             *      called for the last object. So we do this
             *      here instead, and wpMenuItemSelected never
             *      gets called.
             *
             *      This now calls xwpProcessViewCommand,
             *      resolved by name.
             *
             *      Note: WM_MENUEND comes in BEFORE WM_COMMAND.
             */

            case WM_COMMAND:
                fCallDefault = !fdrWMCommand(psfv,
                                             SHORT1FROMMP(mp1));
            break;

#ifndef __NOXSHUTDOWN__
            /*
             * WM_SYSCOMMAND:
             *      intercept "close" for the desktop so we can
             *      invoke shutdown on Alt+F4.
             *
             * V0.9.16 (2002-01-04) [umoeller]
             */

            case WM_SYSCOMMAND:
                fCallDefault = TRUE;
                if (    (SHORT1FROMMP(mp1) == SC_CLOSE)
                     && (hwndFrame == cmnQueryActiveDesktopHWND())
                   )
                {
                    if (cmnQuerySetting(sflXShutdown) & XSD_CANDESKTOPALTF4)
                    {
                        WinPostMsg(hwndFrame,
                                   WM_COMMAND,
                                   MPFROMSHORT(WPMENUID_SHUTDOWN),
                                   MPFROM2SHORT(CMDSRC_MENU,
                                                FALSE));
                        fCallDefault = FALSE;
                    }
                }
            break;
#endif

            /*
             * WM_CHAR:
             *      this is intercepted to provide folder hotkeys
             *      and "Del" key support if "delete into trashcan"
             *      is on.
             */

            case WM_CHAR:
                if (WMChar(hwndFrame, psfv, mp1, mp2))
                    // processed:
                    mrc = (MRESULT)TRUE;
                else
                    fCallDefault = TRUE;
            break;

            /*
             * WM_CONTROL:
             *      this is intercepted to check for container
             *      notifications we might be interested in.
             */

            case WM_CONTROL:
            {
                if (SHORT1FROMMP(mp1) == 0x8008) // container!!
                {

                    switch (SHORT2FROMMP(mp1))      // usNotifyCode
                    {
                        /*
                         * CN_BEGINEDIT:
                         *      this is sent by the container control
                         *      when direct text editing is about to
                         *      begin, that is, when the user alt-clicks
                         *      on an object title.
                         *      We'll select the file stem of the object.
                         */

                        /* case CN_BEGINEDIT: {
                            PCNREDITDATA pced = (PCNREDITDATA)mp2;
                            mrc = (MRESULT)pfnwpOriginal(hwndFrame, msg, mp1, mp2);
                            if (pced) {
                                PMINIRECORDCORE pmrc = (PMINIRECORDCORE)pced->pRecord;
                                if (pmrc) {
                                    // editing WPS record core, not title etc.:
                                    // get the window ID of the MLE control
                                    // in the cnr window
                                    HWND hwndMLE = WinWindowFromID(pced->hwndCnr,
                                                        CID_MLE);
                                    if (hwndMLE) {
                                        ULONG cbText = WinQueryWindowTextLength(
                                                                hwndMLE)+1;
                                        PSZ pszText = malloc(cbText);
                                        _Pmpf(("textlen: %d", cbText));
                                        if (WinQueryWindowText(hwndMLE,
                                                               cbText,
                                                               pszText))
                                        {
                                            PSZ pszLastDot = strrchr(pszText, '.');
                                            _Pmpf(("text: %s", pszText));
                                            WinSendMsg(hwndMLE,
                                                    EM_SETSEL,
                                                    MPFROM2SHORT(
                                                        // first char: 0
                                                        0,
                                                        // last char:
                                                        (pszLastDot)
                                                            ? (pszLastDot-pszText)
                                                            : 10000
                                                    ), MPNULL);
                                        }
                                        free(pszText);
                                    }
                                }
                            }
                        }
                        break;  */

#ifndef __NOXSYSTEMSOUNDS__
                        /*
                         * CN_ENTER:
                         *      double-click or enter key:
                         *      play sound
                         */

                        case CN_ENTER:
                            cmnPlaySystemSound(MMSOUND_XFLD_CNRDBLCLK);
                            mrc = pfnwpOriginal(hwndFrame, msg, mp1, mp2);
                        break;
#endif

                        /*
                         * CN_EMPHASIS:
                         *      selection changed:
                         *      update status bar
                         */

                        case CN_EMPHASIS:
                            mrc = pfnwpOriginal(hwndFrame, msg, mp1, mp2);

                            if (psfv->hwndStatusBar)
                            {
                                PMPF_STATUSBARS(("CN_EMPHASIS: posting STBM_UPDATESTATUSBAR to hwnd %lX",
                                            psfv->hwndStatusBar ));

                                WinPostMsg(psfv->hwndStatusBar,
                                           STBM_UPDATESTATUSBAR,
                                           MPNULL,
                                           MPNULL);
                            }
                        break;

                        /*
                         * CN_EXPANDTREE:
                         *      tree view has been expanded:
                         *      do cnr auto-scroll in File thread
                         */

                        case CN_EXPANDTREE:
                            mrc = pfnwpOriginal(hwndFrame, msg, mp1, mp2);

                            if (cmnQuerySetting(sfTreeViewAutoScroll))
                                xthrPostBushMsg(QM_TREEVIEWAUTOSCROLL,
                                                (MPARAM)hwndFrame,
                                                mp2); // PMINIRECORDCORE
                        break;

                        default:
                            fCallDefault = TRUE;
                        break;
                    } // end switch (SHORT2FROMMP(mp1))      // usNotifyCode
                }
            }
            break;

            /*
             * WM_DESTROY:
             *      clean up resources we allocated for
             *      this folder view.
             */

            case WM_DESTROY:
                // destroy the supplementary object window for this folder
                // frame window; do this first because this references
                // the SFV
                winhDestroyWindow(&psfv->hwndSupplObject);

                // upon closing the window, undo the subclassing, in case
                // some other message still comes in
                // (there are usually still two more, even after WM_DESTROY!!)
                WinSubclassWindow(hwndFrame, pfnwpOriginal);

                // and remove this window from our subclassing linked list
                fdrRemoveSFV(psfv);

                // do the default stuff
                fCallDefault = TRUE;
            break;

            default:
                fCallDefault = TRUE;
            break;

        } // end switch
    } // end TRY_LOUD
    CATCH(excpt1)
    {
        // exception occurred:
        // return 0;        // not good V0.9.19 (2002-05-23) [umoeller]
        fCallDefault = FALSE;
        mrc = NULL;
    } END_CATCH();

    if (fCallDefault)
    {
        // this has only been set to TRUE for "default" in
        // the switch statement above; we then call the
        // default window procedure.
        // This is either the original folder frame window proc
        // of the WPS itself or maybe the one of other WPS enhancers
        // which have subclassed folder windows (ObjectDesktop
        // and the like).
        // We do this outside the TRY/CATCH stuff above so that
        // we don't get blamed for exceptions which we are not
        // responsible for, which was the case with XFolder < 0.85
        // (i.e. exceptions in WPFolder or Object Desktop or whatever).
        if (pfnwpOriginal)
            mrc = CMN_CALLWINPROC(pfnwpOriginal, hwndFrame, msg, mp1, mp2);
                        // V1.0.1 (2002-12-19) [umoeller]
        else
        {
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                   "Folder's pfnwpOrig not found.");
            mrc = WinDefWindowProc(hwndFrame, msg, mp1, mp2);
        }
    }

    return mrc;
}

/*
 *@@ fnwpSubclWPFolderWindow:
 *      new window proc for subclassed folder frame windows.
 *      Folder frame windows are subclassed in fdrSubclassFolderView
 *      (which gets called from XFolder::wpOpen or XFldDisk::wpOpen
 *      for Disk views) with the address of this window procedure.
 *
 *      This window proc is ONLY used for subclassing standard
 *      WPS folder icon, details, and tree views (in other words,
 *      those with the "wpFolder window" PM window class).
 *
 *      The actual message processing is now in fdrProcessFolderMsgs.
 *      This allows us to use the same message processing from
 *      other (future) parts of XWorkplace which no longer rely
 *      on subclassing the default WPS folder frames.
 *      Our custom views such as the file dialog and split views
 *      have their own subclassing mechanisms which call
 *      fdrProcessFolderMsgs directly. See the description there
 *      also for what this does for certain messages.
 *
 *@@changed V0.9.0 [umoeller]: moved cleanup code from WM_CLOSE to WM_DESTROY; un-subclassing removed
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 *@@changed V0.9.1 (2000-01-29) [umoeller]: added WPMENUID_DELETE support
 *@@changed V0.9.1 (2000-01-31) [umoeller]: added "Del" key support
 *@@changed V0.9.2 (2000-02-22) [umoeller]: moved default winproc out of exception handler
 *@@changed V0.9.3 (2000-03-28) [umoeller]: added freaky menus setting
 *@@changed V0.9.3 (2000-04-08) [umoeller]: extracted ProcessFolderMsgs
 *@@changed V0.9.20 (2002-07-31) [umoeller]: made this static
 *@@changed V1.0.0 (2002-08-26) [umoeller]: renamed from fnwpSubclassedFolderFrame
 */

STATIC MRESULT EXPENTRY fnwpSubclWPFolderWindow(HWND hwndFrame,
                                                ULONG msg,
                                                MPARAM mp1,
                                                MPARAM mp2)
{
    PSUBCLFOLDERVIEW psfv;

    if (psfv = fdrQuerySFV(hwndFrame,
                           NULL))
        return fdrProcessFolderMsgs(hwndFrame,
                                    msg,
                                    mp1,
                                    mp2,
                                    psfv,
                                    psfv->pfnwpOriginal);

    // SFV not found: use the default
    // folder window procedure, but issue
    // a warning to the log
    cmnLog(__FILE__, __LINE__, __FUNCTION__,
           "Folder SUBCLFOLDERVIEW not found.");

    return G_WPFolderWinClassInfo.pfnWindowProc(hwndFrame, msg, mp1, mp2);
}

/*
 *@@ fdr_fnwpSupplFolderObject:
 *      this is the wnd proc for the "Supplementary Object wnd"
 *      which is created for each folder frame window when it's
 *      subclassed. We need this window to handle additional
 *      messages which are not part of the normal message set,
 *      which is handled by fnwpSubclWPFolderWindow.
 *
 *      This window gets created in fdrSubclassFolderView, when
 *      the folder frame is also subclassed.
 *
 *      If we processed additional messages in fnwpSubclWPFolderWindow,
 *      we'd probably ruin other WPS enhancers which might use the same
 *      message in a different context (ObjectDesktop?), so we use a
 *      different window, which we own all alone.
 *
 *      We cannot use the global XFolder object window either
 *      (krn_fnwpThread1Object, kernel.c) because sometimes
 *      folder windows do not run in the main PM thread
 *      (TID 1), esp. when they're opened using WinOpenObject or
 *      REXX functions. I have found that manipulating windows
 *      from threads other than the one which created the window
 *      can produce really funny results or bad PM hangs.
 *
 *      This wnd proc always runs in the same thread as the folder
 *      frame wnd does.
 *
 *      This func is new with XFolder V0.82.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 */

MRESULT EXPENTRY fdr_fnwpSupplFolderObject(HWND hwndObject, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MPARAM mrc = NULL;
    PSUBCLFOLDERVIEW psfv = (PSUBCLFOLDERVIEW)WinQueryWindowPtr(hwndObject, QWL_USER);

    switch (msg)
    {
        case WM_CREATE:
            // set the USER window word to the SUBCLFOLDERVIEW
            // structure which is passed to us upon window
            // creation (see cmnSubclassFrameWnd, which creates us)
            mrc = WinDefWindowProc(hwndObject, msg, mp1, mp2);
            psfv = (PSUBCLFOLDERVIEW)mp1;
            WinSetWindowULong(hwndObject, QWL_USER, (ULONG)psfv);
        break;

        /*
         *@@ SOM_ACTIVATESTATUSBAR:
         *      add / remove / repaint the folder status bar;
         *      this is posted every time XFolder needs to change
         *      anything about status bars. We must not play with
         *      frame controls from threads other than the thread
         *      in which the status bar was created, i.e. the thread
         *      in which the folder frame is running (which, in most
         *      cases, is thread 1, the main PM thread of the WPS),
         *      because reformatting frame controls from other
         *      threads will cause PM hangs or WPS crashes.
         *
         *      Parameters:
         *
         *      -- ULONG mp1   -- 0: disable (destroy) status bar
         *                     -- 1: enable (create) status bar
         *                     -- 2: update (reformat) status bar
         *
         *      -- HWND  mp2:  hwndView (frame) to update
         */

        case SOM_ACTIVATESTATUSBAR:
        {
            HWND hwndFrame = (HWND)mp2;

            PMPF_STATUSBARS(("SOM_ACTIVATESTATUSBAR, mp1: %lX, psfv: %lX", mp1, psfv));

            if (psfv)
                switch ((ULONG)mp1)
                {
                    case 0:
                        stbDestroy(psfv);
                    break;

                    case 1:
                        stbCreate(psfv);
                    break;

                    default:
                    {
                        // == 2 => update status bars; this is
                        // necessary if the font etc. has changed
                        const char* pszStatusBarFont =
                                cmnQueryStatusBarSetting(SBS_STATUSBARFONT);
                        // avoid recursing
                        WinSendMsg(psfv->hwndStatusBar, STBM_PROHIBITBROADCASTING,
                                   (MPARAM)TRUE, MPNULL);
                        // set font
                        WinSetPresParam(psfv->hwndStatusBar,
                                        PP_FONTNAMESIZE,
                                        (ULONG)(strlen(pszStatusBarFont) + 1),
                                        (PVOID)pszStatusBarFont);
                        // update frame controls
                        WinSendMsg(hwndFrame, WM_UPDATEFRAME, MPNULL, MPNULL);
                        // update status bar text synchronously
                        WinSendMsg(psfv->hwndStatusBar, STBM_UPDATESTATUSBAR, MPNULL, MPNULL);
                    }
                    break;
                }
        }
        break;

        default:
            mrc = WinDefWindowProc(hwndObject, msg, mp1, mp2);
    }

    return mrc;
}


