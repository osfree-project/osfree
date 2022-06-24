
/*
 *@@sourcefile statbars.c:
 *      this file contains status bar code:
 *
 *      --  status bar window handling (see stbCreate);
 *
 *      --  status bar info translation logic (see stbComposeText)
 *
 *      More status bar code can be found with the following:
 *
 *      --  When selections change in a folder view (i.e.
 *          fnwpSubclWPFolderWindow receives CN_EMPHASIS
 *          notification), fnwpStatusBar is posted an STBM_UPDATESTATUSBAR
 *          message, which in turn calls XFolder::xwpUpdateStatusBar,
 *          which normally calls stbComposeText in turn (the main entry
 *          point to the mess in this file).
 *
 *      Function prefix for this file:
 *      --  stb*
 *
 *      This file is new with XFolder 0.81. V0.80 used
 *      SOM multiple inheritance to introduce new methods
 *      with XFldObject, which proved to cause too many problems.
 *      This is now all done in this file using SOM kernel functions
 *      to determine the class of a selected object.
 *
 *      It is thus now much easier to add support for new classes
 *      also, because no new class replacements have to be introduced.
 *      In order to do so, go through all the funcs below and add new
 *      "if" statements. No changes in other files should be necessary.
 *      You will have to add a #include below for that class though
 *      to be able to access the SOM class object for that new class.
 *
 *      With V0.9.19, the status bar window code was moved into
 *      this file as well.
 *
 *      Status bar notebook pages were moved to statbar_pages.c
 *      with V1.0.1 (2002-12-08) [umoeller].
 *
 *@@header "filesys\statbars.h"
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

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINSYS             // needed for presparams
#define INCL_WININPUT
#define INCL_WINTIMER
#define INCL_WINSTATICS
#define INCL_WINSHELLDATA       // Prf* functions
#define INCL_WINPROGRAMLIST     // needed for PROGDETAILS, wppgm.h

#define INCL_GPIPRIMITIVES
#include <os2.h>

// C library headers
#include <stdio.h>
#include <string.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\threads.h"            // thread helpers
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers

// SOM headers which don't crash with prec. header files
// #include "xfobj.ih"
#include "xfldr.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\classtest.h"           // some cheap funcs for WPS class checks
#include "shared\common.h"              // the majestic XWorkplace include file

#include "helpers\comctl.h"             // common controls (window procs)
#include "filesys\folder.h"             // XFolder implementation
#include "filesys\fdrsubclass.h"        // folder subclassing engine
#include "filesys\fdrviews.h"           // common code for folder views
#include "filesys\fdrsplit.h"           // folder split views
#include "filesys\object.h"             // XFldObject implementation
#include "filesys\program.h"            // program implementation; WARNING: this redefines macros
#include "filesys\statbars.h"           // status bar translation logic
#include "filesys\xthreads.h"           // extra XWorkplace threads

// other SOM headers
#pragma hdrstop                         // VAC++ keeps crashing otherwise
#include <wpdisk.h>                     // WPDisk
#include <wppgm.h>                      // WPProgram

#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

static CHAR    G_szXFldObjectStatusBarMnemonics[CCHMAXMNEMONICS] = "";
static CHAR    G_szWPProgramStatusBarMnemonics[CCHMAXMNEMONICS] = "";
static CHAR    G_szWPDiskStatusBarMnemonics[CCHMAXMNEMONICS] = "";
static CHAR    G_szWPFileSystemStatusBarMnemonics[CCHMAXMNEMONICS] = "";
static STATIC CHAR    G_szWPUrlStatusBarMnemonics[CCHMAXMNEMONICS] = "";

STATIC MRESULT EXPENTRY fnwpStatusBar(HWND hwndBar, ULONG msg, MPARAM mp1, MPARAM mp2);

/* ******************************************************************
 *
 *   Status bar window
 *
 ********************************************************************/

/*
 *@@ stbClassCanHaveStatusBars:
 *      returns TRUE if the given object's class
 *      can have status or tool bars.
 *
 *      This rules out the active desktop and
 *      the Object Desktop tab launchpad and
 *      control center classes.
 *
 *@@added V0.9.19 (2002-04-17) [umoeller]
 */

BOOL stbClassCanHaveStatusBars(WPFolder *somSelf)
{
    PCSZ    pcszClass;
    return (
                // no status bar for active Desktop
                // (somSelf != cmnQueryActiveDesktop())
                        // er, well, views other than the active Desktop window
                        // can have status bars V1.0.1 (2002-12-08) [umoeller]
                // rule out object desktop classes V0.9.19 (2002-04-17) [umoeller]
                (pcszClass = _somGetClassName(somSelf))
             && (strcmp(pcszClass, G_pcszTabLaunchPad))
             && (strcmp(pcszClass, G_pcszControlCenter))
           );
}

/*
 *@@ stbFolderWantsStatusBars:
 *      returns TRUE if status bars are enabled for
 *      this folder in general, either explicitly
 *      or globally.
 *
 *@@added V1.0.0 (2002-08-21) [umoeller]
 */

BOOL stbFolderWantsStatusBars(WPFolder *somSelf)
{
    XFolderData *somThis = XFolderGetData(somSelf);

    return (    (_bStatusBarInstance == STATUSBAR_ON)
             || (    (_bStatusBarInstance == STATUSBAR_DEFAULT)
                  && (cmnQuerySetting(sfDefaultStatusBarVisibility))
                )
           );
}

/*
 *@@ stbViewCanHaveBars:
 *
 *@@added V1.0.1 (2002-12-08) [umoeller]
 */

BOOL stbViewCanHaveBars(WPFolder *somSelf,
                        HWND hwndFrame,
                        ULONG ulView,
                        XWPSETTING s)           // in: sflSBForViews or sflTBForViews
{
    ULONG   flViews;

    return (
             // 1) rule out desktop and Object Desktop classes
             // V0.9.19 (2002-04-17) [umoeller]
                (    (!hwndFrame)
                  || (hwndFrame != cmnQueryActiveDesktopHWND())
                )
             && (stbClassCanHaveStatusBars(somSelf))
             // 2) status bar only if allowed for the current view type
             && (flViews = cmnQuerySetting(sflSBForViews))
             && (
                    (    (ulView == OPEN_CONTENTS)
                      && (flViews & SBV_ICON)
                    )
                 || (    (ulView == OPEN_TREE)
                      && (flViews & SBV_TREE)
                    )
                 || (    (ulView == OPEN_DETAILS)
                      && (flViews & SBV_DETAILS)
                    )
                 || (    (    (ulView == *G_pulVarMenuOfs + ID_XFMI_OFS_SPLITVIEW)
                           || (ulView == *G_pulVarMenuOfs + ID_XFMI_OFS_SPLIT_SUBFILES)
                         )
                      && (flViews & SBV_SPLIT)
                    )
                )
           );
}

/*
 *@@ stbViewHasStatusBar:
 *      returns TRUE if the given view for the given
 *      folder should have a status bar.
 *
 *      This returns FALSE if status bars are disabled
 *      globally, or locally for the folder, or if
 *      the folder's class shouldn't have status bars,
 *      of if the view has been ruled out in "Workplace
 *      Shell".
 *
 *      This func was added with V0.9.19 to have one
 *      central place which verifies whether a folder
 *      view can have a status bar. This is necessary
 *      so we can finally rule out status bars for
 *      the Object Desktop folder subclasses.
 *
 *@@added V0.9.19 (2002-04-17) [umoeller]
 *@@changed V1.0.1 (2002-11-30) [umoeller]: added split view support
 *@@changed V1.0.1 (2002-12-08) [umoeller]: added hwndFrame to allow bars for non-active desktop windows
 */

BOOL stbViewHasStatusBar(WPFolder *somSelf,
                         HWND hwndFrame,       // in: folder frame for desktop checks
                         ULONG ulView)         // in: OPEN_CONTENTS, OPEN_TREE, OPEN_DETAILS, or ID_XFMI_OFS_SPLITVIEW
{
    return (
#ifndef __NOCFGSTATUSBARS__
                (cmnQuerySetting(sfStatusBars))
             &&
#endif
                stbFolderWantsStatusBars(somSelf)
             && stbViewCanHaveBars(somSelf,
                                   hwndFrame,
                                   ulView,
                                   sflSBForViews)
                    // V1.0.1 (2002-12-08) [umoeller]
           );
}

/*
 *@@ stbViewHasToolBar:
 *      returns TRUE if the given view for the given
 *      folder should have a tool bar.
 *
 *      This returns FALSE if tool bars are disabled
 *      globally, or locally for the folder, or if
 *      the folder's class shouldn't have tool bars,
 *      of if the view has been ruled out in "Workplace
 *      Shell".
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 */

BOOL stbViewHasToolBar(WPFolder *somSelf,
                       HWND hwndFrame,       // in: folder frame for desktop checks
                       ULONG ulView)         // OPEN_CONTENTS, OPEN_TREE, OPEN_DETAILS, or ID_XFMI_OFS_SPLITVIEW
{
    ULONG               flViews;

    return (
#ifndef __NOCFGSTATUSBARS__
                (cmnQuerySetting(sfToolBars))
             &&
#endif
                stbFolderWantsStatusBars(somSelf)
             && stbViewCanHaveBars(somSelf,
                                   hwndFrame,
                                   ulView,
                                   sflTBForViews)
           );
}

/*
 *@@ stbCreateBar:
 *      this actually creates the status bar window.
 *
 *@@added V1.0.0 (2002-08-21) [umoeller]
 */

HWND stbCreateBar(WPFolder *somSelf,        // in: (root) folder
                  WPObject *pRealObject,    // in: disk object or folder
                  HWND hwndFrame,           // in: folder view frame
                  HWND hwndCnr)             // in: FID_CLIENT of the frame or some other window
{
    HWND hwndBar;

    if (hwndBar = WinCreateWindow(hwndFrame,
                                  WC_STATIC,              // wnd class
                                  (PSZ)cmnGetString(ID_XSSI_POPULATING),
                                        // "Collecting objects..."
                                  SS_TEXT | DT_LEFT | DT_VCENTER // wnd style flags
                                      | DT_ERASERECT
                                      | WS_VISIBLE | WS_CLIPSIBLINGS,
                                  0L, 0L, -1L, -1L,
                                  hwndFrame,        // owner
                                  HWND_BOTTOM,
                                  ID_STATUSBAR,            // ID
                                  NULL,
                                  NULL))
    {
        PCSZ    pszStatusBarFont = cmnQueryStatusBarSetting(SBS_STATUSBARFONT);

        // set up window data (QWL_USER) for status bar
        PSTATUSBARDATA psbd;

        if (psbd = NEW(STATUSBARDATA))
        {
            psbd->somSelf    = somSelf;
            psbd->pRealObject = pRealObject;        // V1.0.0 (2002-08-21) [umoeller]
            psbd->hwndFrame = hwndFrame;
            psbd->hwndCnr = hwndCnr;
            psbd->habStatusBar = WinQueryAnchorBlock(hwndBar);
            psbd->idTimer    = 0;
            psbd->fDontBroadcast = TRUE;
                        // prevents broadcasting of WM_PRESPARAMSCHANGED
            psbd->fFolderPopulated = FALSE;
                        // suspends updating until folder is populated;

            WinSetWindowPtr(hwndBar, QWL_USER, psbd);

            // subclass static control to make it a status bar
            psbd->pfnwpStatusBarOriginal = WinSubclassWindow(hwndBar,
                                                             fnwpStatusBar);
            WinSetPresParam(hwndBar,
                            PP_FONTNAMESIZE,
                            (ULONG)strlen(pszStatusBarFont) + 1,
                            (PVOID)pszStatusBarFont);
        }
    }

    return hwndBar;
}

/*
 *@@ stbCreate:
 *      this creates the status bar for the given folder view.
 *
 *      This gets called in the following situations:
 *
 *      a)   from fdrManipulateNewView, when a folder view is
 *           created which needs a status bar;
 *
 *      b)   later if status bar visibility settings are changed
 *           either for the folder or globally.
 *
 *      Parameters:
 *
 *      --  psfv:      pointer to SUBCLFOLDERVIEW structure of
 *                     current folder frame; this contains the
 *                     folder frame window handle
 *
 *      This func returns the hwnd of the status bar or NULL if calling
 *      the func was useless, because the status bar was already
 *      (de)activated.
 *
 *      The status bar always has the ID ID_STATUSBAR (0x9001) so you
 *      can get the status bar HWND later by calling
 *
 +          WinQueryWindow(hwndFrame, ID_STATUSBAR)
 *
 *      Note that this does _not_ change the folder's visibility
 *      settings, but only creates the actual status bar window
 *      and reformats the folder's PM frame window.
 *
 *      This function _must_ be called only from the same thread
 *      where the folder frame window is running (normally TID 1),
 *      otherwise the WPS will crash or PM will hang.
 *
 *      Because of all this, call XFolder::xwpSetStatusBarVisibility
 *      instead if you wish to change folder status bars. That method
 *      will do the display also.
 *
 *@@changed V0.9.0 [umoeller]: this used to be an instance method (xfldr.c)
 *@@changed V0.9.19 (2002-04-17) [umoeller]: renamed from fdrCreateStatusBar, extracted stbDestroy
 */

HWND stbCreate(PSUBCLFOLDERVIEW psli2)
{
    HWND hrc = NULLHANDLE;

    // XFolderData *somThis = XFolderGetData(somSelf);

    PMPF_STATUSBARS(("[%s]", _wpQueryTitle(psli2->somSelf)));

    if (psli2)
    {
        if (psli2->hwndStatusBar) // already activated: update only
        {
            PMPF_STATUSBARS(("    status bar already exists, posting STBM_UPDATESTATUSBAR"));

            WinPostMsg(psli2->hwndStatusBar, STBM_UPDATESTATUSBAR, 0, 0);
            // and quit
        }
        // else create status bar as a static control
        // (which will be subclassed below)
        else if (psli2->hwndStatusBar = stbCreateBar(psli2->somSelf,
                                                     psli2->pRealObject,
                                                     psli2->hwndFrame,
                                                     psli2->hwndCnr))
        {
            XFolderData *somThis = XFolderGetData(psli2->somSelf);
            CHAR    szFolderPosKey[50];
            ULONG   ulIni;
            ULONG   cbIni;
            BOOL    fInflate = FALSE;
            SWP     swp;
            ULONG   ulView = wpshQueryView(psli2->somSelf,
                                           psli2->hwndFrame);

            #ifdef __DEBUG__
                PCSZ pcszView;
                CHAR szView[100];
                PMPF_STATUSBARS(("    created new status bar hwnd 0x%lX", psli2->hwndStatusBar));
                switch (ulView)
                {
                    case OPEN_TREE: pcszView = "Tree"; break;
                    case OPEN_CONTENTS: pcszView = "Contents"; break;
                    case OPEN_DETAILS: pcszView = "Details"; break;
                    default:
                        sprintf(szView, "unknown %d", ulView);
                        pcszView = szView;
                }
                PMPF_STATUSBARS(("    View: %s", pcszView));
            #endif

            // inflate only for standard folder views,
            // this rules out split view
            // V1.0.0 (2002-08-21) [umoeller]
            switch (ulView)
            {
                case OPEN_TREE:
                case OPEN_CONTENTS:
                case OPEN_DETAILS:
                    // now "inflate" the folder frame window if this is the first
                    // time that this folder view has been opened with a status bar;
                    // if we didn't do this, the folder frame would be too small
                    // for the status bar and scroll bars would appear. We store
                    // a flag in the folder's instance data for each different view
                    // that we've inflated the folder frame, so that this happens
                    // only once per view.

                    // From wpobject.h:
                    // #define VIEW_CONTENTS      0x00000001
                    // #define VIEW_SETTINGS      0x00000002
                    // #define VIEW_HELP          0x00000004
                    // #define VIEW_RUNNING       0x00000008
                    // #define VIEW_DETAILS       0x00000010
                    // #define VIEW_TREE          0x00000020

                    sprintf(szFolderPosKey, "%d@XFSB",
                            _wpQueryHandle(psli2->pRealObject));

                    cbIni = sizeof(ulIni);
                    if (!PrfQueryProfileData(HINI_USER,
                                             (PSZ)WPINIAPP_FOLDERPOS,
                                             szFolderPosKey,
                                             &ulIni,
                                             &cbIni))
                        ulIni = 0;

                    if (ulIni & (   (ulView == OPEN_CONTENTS) ? VIEW_CONTENTS
                                  : (ulView == OPEN_TREE) ? VIEW_TREE
                                  : VIEW_DETAILS
                                ))
                        fInflate = FALSE;
                    else
                        fInflate = TRUE;

                    PMPF_STATUSBARS(("   fInflate = %d, ulView = %d", fInflate, ulView));

                    // set a flag for the subclassed folder frame
                    // window proc that this folder view needs no additional scrolling
                    // (this is evaluated in WM_FORMATFRAME msgs)
                    // changed V1.0.1 (2003-02-02) [umoeller]
                    psli2->bNeedCnrScroll = SCROLL_ADJUSTFORSB;

                    if (fInflate)
                    {
                        // this view has not been inflated yet:
                        // inflate now and set flag for this view

                        ULONG ulStatusBarHeight = cmnQueryStatusBarHeight();
                        WinQueryWindowPos(psli2->hwndFrame, &swp);
                        // inflate folder frame
                        WinSetWindowPos(psli2->hwndFrame, 0,
                                        swp.x, (swp.y - ulStatusBarHeight),
                                        swp.cx, (swp.cy + ulStatusBarHeight),
                                        SWP_MOVE | SWP_SIZE);

                        // mark this folder view as "inflated" in OS2.INI
                        ulIni |= (   (ulView == OPEN_CONTENTS) ? VIEW_CONTENTS
                                   : (ulView == OPEN_TREE) ? VIEW_TREE
                                   : VIEW_DETAILS
                                 );
                        PrfWriteProfileData(HINI_USER,
                                            (PSZ)WPINIAPP_FOLDERPOS,
                                            szFolderPosKey,
                                            &ulIni,
                                            sizeof(ulIni));
                    }
                    else
                        // always do this for icon view if auto-sort is off
                        // V0.9.18 (2002-03-24) [umoeller]
                        // WM_FORMATFRAME _will_ have to scroll the container
                        // hrm, don't think so... scroll only if we did _not_
                        // inflate because the view has already previously
                        // been opened V1.0.1 (2003-02-02) [umoeller]
                        if (    (ulView == OPEN_CONTENTS)
                             && (  (_lAlwaysSort == SET_DEFAULT)
                                        ? !cmnQuerySetting(sfAlwaysSort)
                                        : !_lAlwaysSort)
                           )
                            psli2->bNeedCnrScroll = SCROLL_VERYFIRSTTIME;

                    PMPF_STATUSBARS(("    set psli2->fNeedCnrScroll: %d", psli2->bNeedCnrScroll));
                    PMPF_STATUSBARS(("    sending WM_UPDATEFRAME"));

            } // end switch (ulView)

            // enforce reformatting / repaint of frame window
            WinSendMsg(psli2->hwndFrame, WM_UPDATEFRAME, 0, 0);

            // update status bar contents
            WinPostMsg(psli2->hwndStatusBar, STBM_UPDATESTATUSBAR, 0, 0);

            hrc = psli2->hwndStatusBar;
        }
    }

    return hrc;
}

/*
 *@@ stbDestroy:
 *      destroys the status bar window for the given
 *      folder view.
 *
 *      This code used to be in stbCreate, but we
 *      shouldn't destroy something in a function
 *      that is called "create" really.
 *
 *@@added V0.9.19 (2002-04-17) [umoeller]
 */

VOID stbDestroy(PSUBCLFOLDERVIEW psli2)
{
    // hide status bar:
    if (psli2->hwndStatusBar)
    {
        CHAR    szFolderPosKey[50];
        ULONG   ulIni;
        ULONG   cbIni;
        SWP     swp;
        HWND    hwndStatus = psli2->hwndStatusBar;
        BOOL    fDeflate = FALSE;
        ULONG   ulView = wpshQueryView(psli2->somSelf,
                                       psli2->hwndFrame);

        psli2->hwndStatusBar = 0;
        WinSendMsg(psli2->hwndFrame, WM_UPDATEFRAME, 0, 0);
        winhDestroyWindow(&hwndStatus);

        // decrease the size of the frame window by the status bar height,
        // if we did this before
        sprintf(szFolderPosKey, "%d@XFSB",
                _wpQueryHandle(psli2->pRealObject));

        cbIni = sizeof(ulIni);
        if (PrfQueryProfileData(HINI_USER,
                                (PSZ)WPINIAPP_FOLDERPOS,     // "PM_Workplace:FolderPos"
                                szFolderPosKey,
                                &ulIni,
                                &cbIni) == FALSE)
            ulIni = 0;

        if (ulIni & (   (ulView == OPEN_CONTENTS) ? VIEW_CONTENTS
                      : (ulView == OPEN_TREE) ? VIEW_TREE
                      : VIEW_DETAILS
                    ))
            fDeflate = TRUE;
        else
            fDeflate = TRUE;

        if (fDeflate)
        {
            ULONG ulStatusBarHeight = cmnQueryStatusBarHeight();
            WinQueryWindowPos(psli2->hwndFrame, &swp);
            WinSetWindowPos(psli2->hwndFrame, 0,
                            swp.x, (swp.y + ulStatusBarHeight),
                            swp.cx, (swp.cy - ulStatusBarHeight),
                            SWP_MOVE | SWP_SIZE);

            ulIni &= ~(   (ulView == OPEN_CONTENTS) ? VIEW_CONTENTS
                        : (ulView == OPEN_TREE) ? VIEW_TREE
                        : VIEW_DETAILS
                      );
            PrfWriteProfileData(HINI_USER,
                                (PSZ)WPINIAPP_FOLDERPOS,     // "PM_Workplace:FolderPos"
                                szFolderPosKey,
                                &ulIni,
                                sizeof(ulIni));
        }
    }
}

/*
 * stb_UpdateCallback:
 *      callback func for WOM_UPDATEALLSTATUSBARS in
 *      XFolder Worker thread to refresh status bar
 *      visibilities for the given folder. This gets
 *      called whenever status bar visibility settings
 *      changed, or the mnemonics have changed, in
 *      the "Workplace Shell" object.
 *
 *      This is called by xf(cls)ForEachOpenView, which
 *      also passes the parameters to this func.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 *@@changed V1.0.1 (2002-11-30) [umoeller]: added support for updating split views
 */

BOOL _Optlink stb_UpdateCallback(WPFolder *somSelf,
                                 HWND hwndView,
                                 ULONG ulView,
                                 ULONG ulActivate)
                             // 1: show/hide status bars according to
                             //    folder/Global settings
                             // 2: reformat status bars (e.g. because
                             //    fonts have changed)
{
    PSUBCLFOLDERVIEW    psfv;

    PMPF_STATUSBARS(("ulActivate = %d", ulActivate));

    // fdrQuerySFV doesn't work for split views
    if (ulView == *G_pulVarMenuOfs + ID_XFMI_OFS_SPLITVIEW)
    {
        PMPF_STATUSBARS(("view 0x%lX is split view, posting WM_CONTROL SN_UPDATESTATUSBAR", ulView));

        WinPostMsg(hwndView,
                   WM_CONTROL,
                   MPFROM2SHORT(FID_CLIENT,
                                SN_UPDATESTATUSBAR),
                   (MPARAM)ulActivate);
    }
    else if (psfv = fdrQuerySFV(hwndView, NULL))
    {
        PMPF_STATUSBARS(("found SUBCLFOLDERVIEW for view 0x%lX", ulView));

        if (ulActivate == 2) // "update" flag
            WinPostMsg(psfv->hwndSupplObject,
                       SOM_ACTIVATESTATUSBAR,
                       (MPARAM)2,
                       (MPARAM)hwndView);
        else
        {
            // show/hide flag:

            // check if the folder should have a status bar
            // depending on the new settings
            // V0.9.19 (2002-04-17) [umoeller]
            BOOL fNewVisible = stbViewHasStatusBar(somSelf,
                                                   hwndView,    // V1.0.1 (2002-12-08) [umoeller]
                                                   ulView);


            if ( fNewVisible != (psfv->hwndStatusBar != NULLHANDLE) )
            {
                // visibility changed:
                // we now post a message to the folder frame's
                // supplementary object window;
                // because we cannot mess with the windows on
                // the Worker thread, we need to have the
                // status bar created/destroyed/changed on the
                // folder's thread
                WinPostMsg(psfv->hwndSupplObject,
                           SOM_ACTIVATESTATUSBAR,
                           (MPARAM)fNewVisible,     // show/hide flag
                           (MPARAM)hwndView);

                return TRUE;
            }
        }
    }
    else  // V1.0.9
        if (ulView == *G_pulVarMenuOfs + ID_XFMI_OFS_SPLITVIEW)
        {
            HWND hwndMainControl = WinWindowFromID(hwndView, FID_CLIENT);
            PSPLITCONTROLLER pctl;

            if (   hwndMainControl
                && (pctl = WinQueryWindowPtr(hwndMainControl, QWL_USER))
                && pctl->xfc.hwndStatusBar
               )
            {
                WinPostMsg(pctl->xfc.hwndStatusBar, STBM_UPDATESTATUSBAR,
                           (MPARAM) pctl->hwndFocusCnr, MPNULL);
            }
        }

    return FALSE;
}

/*
 * stb_PostCallback:
 *      this posts a message to each view's status bar
 *      if it exists.
 *
 *      This is a callback to fdrForEachOpenInstanceView,
 *      which gets
 *      called from XFolder::wpAddToContent and
 *      XFolder::wpDeleteFromContent.
 *
 *@@added V0.9.6 (2000-10-26) [pr]
 */

BOOL _Optlink stb_PostCallback(WPFolder *somSelf,
                               HWND hwndView,
                               ULONG ulView,
                               ULONG msg)            // message
{
    PSUBCLFOLDERVIEW psfv;

    if (    (ulView == OPEN_CONTENTS)
         || (ulView == OPEN_DETAILS)
         || (ulView == OPEN_TREE)
         || (ulView == *G_pulVarMenuOfs + ID_XFMI_OFS_SPLITVIEW)  // V1.0.9
       )
    {
        if (    (psfv = fdrQuerySFV(hwndView, NULL))
             && (psfv->hwndStatusBar)
           )
            WinPostMsg(psfv->hwndStatusBar,
                       msg,
                       0,
                       0);
    }

    return TRUE;
}

/*
 *@@ CallResolvedUpdateStatusBar:
 *      resolves XFolder::xwpUpdateStatusBar via name-lookup
 *      resolution on pFolder and, if found, calls that method
 *      implementation.
 *
 *      Always use this function instead of calling the method
 *      directly... because that would probably lead to confusion.
 *
 *@@added V0.9.7 (2001-01-13) [umoeller]
 *@@changed V0.9.19 (2002-04-17) [umoeller]: made this static
 */

STATIC VOID CallResolvedUpdateStatusBar(WPFolder *pFolder,
                                        HWND hwndStatusBar,
                                        HWND hwndCnr)
{
    BOOL fObjectInitialized = objIsObjectInitialized(pFolder);
    XFolderData *somThis = XFolderGetData(pFolder);
    somTD_XFolder_xwpUpdateStatusBar pfnResolvedUpdateStatusBar = NULL;

    // do SOM name-lookup resolution for xwpUpdateStatusBar
    // (which will per default find the XFolder method, V0.9.0)
    // if we have not done that for this instance already;
    // we store the method pointer in the instance data
    // for speed
    if (fObjectInitialized) // V0.9.3 (2000-04-29) [umoeller]
        if (_pfnResolvedUpdateStatusBar)
            // already resolved: get resolved address
            // from instance data
            pfnResolvedUpdateStatusBar
                = (somTD_XFolder_xwpUpdateStatusBar)_pfnResolvedUpdateStatusBar;

    if (!pfnResolvedUpdateStatusBar)
    {
        // not resolved yet:
        pfnResolvedUpdateStatusBar
                = (somTD_XFolder_xwpUpdateStatusBar)somResolveByName(
                                          pFolder,
                                          "xwpUpdateStatusBar");
        if (fObjectInitialized)
            // object initialized:
            _pfnResolvedUpdateStatusBar = (PVOID)pfnResolvedUpdateStatusBar;
    }

    // finally, compose the text by calling
    // the resolved method
    if (pfnResolvedUpdateStatusBar)
        pfnResolvedUpdateStatusBar(pFolder,
                                   hwndStatusBar,
                                   hwndCnr);
    else
        WinSetWindowText(hwndStatusBar,
                         "*** error in name-lookup resolution");
}

/*
 *@@ stbUpdate:
 *      updates the status bars of all open views of the
 *      specified folder.
 *
 *@@added V0.9.7 (2001-01-13) [umoeller]
 */

VOID stbUpdate(WPFolder *pFolder)
{
    // if (_wpFindUseItem(pFolder, USAGE_OPENVIEW, NULL))
    {
        // folder has an open view;
        // now we go search the open views of the folder and get the
        // frame handle of the desired view (ulView)
        /*      replaced V1.0.0 (2002-08-28) [umoeller]
        PVIEWITEM   pViewItem;
        for (pViewItem = _wpFindViewItem(pFolder, VIEW_ANY, NULL);
             pViewItem;
             pViewItem = _wpFindViewItem(pFolder, VIEW_ANY, pViewItem)) */

        PUSEITEM pui;
        for (pui = _wpFindUseItem(pFolder, USAGE_OPENVIEW, NULL);
             pui;
             pui = _wpFindUseItem(pFolder, USAGE_OPENVIEW, pui))
        {
            PVIEWITEM pvi = (PVIEWITEM)(pui + 1);

            switch (pvi->view)
            {
                case OPEN_CONTENTS:
                case OPEN_DETAILS:
                case OPEN_TREE:
                {
                    HWND hwndStatusBar;
                    HWND hwndCnr;
                    if (    (hwndStatusBar = WinWindowFromID(pvi->handle, ID_STATUSBAR))
                         && (hwndCnr = WinWindowFromID(pvi->handle, FID_CLIENT))
                       )
                        CallResolvedUpdateStatusBar(pFolder,
                                                    hwndStatusBar,
                                                    hwndCnr);
                }
            }
        }
    }
}

/* ******************************************************************
 *
 *   XFolder window procedures
 *
 ********************************************************************/

/*
 *@@ StatusTimer:
 *
 *@@added V0.9.16 (2001-10-28) [umoeller]
 *@@changed V0.9.16 (2001-10-28) [umoeller]: fixed bad view check for tree views
 *@@changed V0.9.16 (2001-10-28) [umoeller]: fixed bad excpt handler cleanup
 */

STATIC VOID StatusTimer(HWND hwndBar,
                        PSTATUSBARDATA psbd)
{
    TRY_LOUD(excpt1)
    {
        BOOL    fUpdate = TRUE;

        // stop timer (it's just for one shot)
        WinStopTimer(psbd->habStatusBar, // anchor block,
                     hwndBar,
                     1);
        psbd->idTimer = 0;

        // if we're not fully populated yet, start timer again and quit;
        // otherwise we would display false information.
        // We need an extra flag in the status bar data because the
        // FOI_POPULATEDWITHALL is reset to 0 by the WPS for some reason
        // when an object is deleted from an open folder, and no further
        // status bar updates would occur then
        if (!psbd->fFolderPopulated)
        {
            ULONG   ulFlags = _wpQueryFldrFlags(psbd->somSelf);
            ULONG   ulView = wpshQueryView(// psbd->psfv->somSelf,
                                           // V0.9.16 (2001-10-28) [umoeller]: we
                                           // can't use somSelf because root folders
                                           // never hold the view information... use
                                           // the "real object" instead, which, for
                                           // root folders, holds the disk object
                                           psbd->pRealObject,
                                           psbd->hwndFrame);

            PMPF_STATUSBARS(("view = 0x%lX (%s)",
                             ulView,
                             cmnIdentifyView(ulView)));

            // for tree views, check if folder is populated with folders;
            // otherwise check for populated with all
            if (    (   (ulView == OPEN_TREE)
                     && ((ulFlags & FOI_POPULATEDWITHFOLDERS) !=0)
                    )
                || ((ulFlags & FOI_POPULATEDWITHALL) != 0)
               )
            {
                psbd->fFolderPopulated = TRUE;
            }
            else
            {
                // folder not yet populated:
                // restart timer with a lower frequency
                // to have this checked again
                psbd->idTimer = WinStartTimer(psbd->habStatusBar, // anchor block
                                              hwndBar,
                                              1,
                                              300);   // this time, use 300 ms
                // and stop
                // break;       // whoa, this didn't deregister the excpt handler
                                // V0.9.16 (2001-10-28) [umoeller]
                fUpdate = FALSE;
            }
        }

        if (fUpdate)
            // OK:
            CallResolvedUpdateStatusBar(psbd->somSelf,
                                        hwndBar,
                                        psbd->hwndCnr);
    }
    CATCH(excpt1)
    {
        WinSetWindowText(hwndBar, "*** error composing text");
    } END_CATCH();

}

/*
 *@@ StatusPaint:
 *
 *@@added V0.9.16 (2001-10-28) [umoeller]
 *@@changed V1.0.1 (2003-01-30) [umoeller]: optimized, fixed possible buf overflow
 */

STATIC VOID StatusPaint(HWND hwndBar)
{
    // preparations:
    HPS     hps = WinBeginPaint(hwndBar, NULLHANDLE, NULL);

    TRY_LOUD(excpt1)
    {
        RECTL   rclBar,
                rclPaint,
                rcl2;
        POINTL  ptl1;
        PSZ     pszText;
        USHORT  usLength;
        LONG    lNextX;
        PSZ     p1, p2, p3;
        ULONG   ulStyle = cmnQuerySetting(sulSBStyle);
        LONG    lHiColor = WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONLIGHT, 0),
                lLoColor = WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONDARK, 0);

        WinQueryWindowRect(hwndBar,
                           &rclBar);        // exclusive
        // switch to RGB mode
        gpihSwitchToRGB(hps);

        // 1) draw background
        WinFillRect(hps,
                    &rclBar,                // exclusive
                    cmnQuerySetting(slSBBgndColor));

        rclPaint.xLeft = rclBar.xLeft;
        rclPaint.yBottom = rclBar.yBottom;
        rclPaint.xRight = rclBar.xRight - 1;
        rclPaint.yTop = rclBar.yTop - 1;

        // 2) draw 3D frame in selected style
        switch (ulStyle)
        {
            case SBSTYLE_WARP3RAISED:
                // Warp 3 style, raised
                gpihDraw3DFrame2(hps,
                                 &rclPaint,
                                 1,
                                 lHiColor,
                                 lLoColor);
            break;

            case SBSTYLE_WARP3SUNKEN:
                // Warp 3 style, sunken
                gpihDraw3DFrame2(hps,
                                 &rclPaint,
                                 1,
                                 lLoColor,
                                 lHiColor);
            break;

            case SBSTYLE_WARP4MENU:
                // Warp 4 menu style: draw 3D line at top only
                rclPaint.yBottom = rclPaint.yTop - 1;
                gpihDraw3DFrame2(hps,
                                 &rclPaint,
                                 1,
                                 lLoColor,
                                 lHiColor);
            break;

            default:
                // Warp 4 button style
                // draw "sunken" outer rect
                gpihDraw3DFrame(hps,
                                &rcl2,
                                2,
                                lLoColor,
                                lHiColor);
                // draw "raised" inner rect
                rclPaint.xLeft++;
                rclPaint.yBottom++;
                rclPaint.xRight--;
                rclPaint.yTop--;
                gpihDraw3DFrame2(hps,
                                 &rclPaint,
                                 2,
                                 lHiColor,
                                 lLoColor);
            break;
        }

        // 3) start working on text; we do "simple" GpiCharString
        //    if no tabulators are defined, but calculate some
        //    subrectangles otherwise
        if (pszText = winhQueryWindowText(hwndBar))
        {
                // pszText now has the translated status bar text
                // except for the tabulators ("$x" keys)
            p1 = pszText;
            p2 = NULL;
            ptl1.x = 7;

            do  // while tabulators are present
            {
                // search for tab mnemonic
                if (p2 = strstr(p1, "$x("))
                {
                    // tab found: calculate next x position into lNextX
                    usLength =  p2 - p1;

                    if (p3 = strchr(p2, ')'))
                    {
                        PSZ     p4 = strchr(p2, '%');

                        // optimized V1.0.1 (2003-01-30) [umoeller]
                        *p3 = '\0';
                        lNextX = atoi(p2 + 3);
                        *p3 = ')';

                        if (lNextX < 0)
                        {
                            // if the result is negative, it's probably
                            // meant to be an offset from the right
                            // status bar border
                            lNextX = (rclBar.xRight + lNextX); // lNextX is negative
                        }
                        else if ((p4) && (p4 < p3))
                        {
                            // if we have a '%' char before the closing
                            // bracket, consider lNextX a percentage
                            // parameter and now translate it into an
                            // absolute x position using the status bar's
                            // width
                            lNextX = (rclBar.xRight * lNextX) / 100;
                        }
                    }
                    else
                        p2 = NULL;
                }
                else
                    usLength = strlen(p1);

                ptl1.y = (cmnQuerySetting(sulSBStyle) == SBSTYLE_WARP4MENU) ? 5 : 7;
                // set the text color to the global value;
                // this might have changed via color drag'n'drop
                GpiSetColor(hps, cmnQuerySetting(slSBTextColor));
                    // the font is already preset by the static
                    // text control (phhhh...)

                if (p2)
                {
                    // did we have tabs? if so, print text clipped to rect
                    RECTL rcl;
                    rcl.xLeft   = 0;
                    rcl.yBottom = 0; // ptl1.y;
                    rcl.xRight  = lNextX-10; // 10 pels space
                    rcl.yTop    = rclBar.yTop;
                    GpiCharStringPosAt(hps,
                                       &ptl1,
                                       &rcl,
                                       CHS_CLIP,
                                       usLength,
                                       p1,
                                       NULL);
                }
                else
                {
                    // no (more) tabs: just print
                    GpiMove(hps, &ptl1);
                    GpiCharString(hps, usLength, p1);
                }

                if (p2)
                {   // "tabulator" was found: set next x pos
                    ptl1.x = lNextX;
                    p1 = p3+1;
                }

            } while (p2);       // go for next tabulator, if we had one

            free(pszText);
        } // end if (pszText)
    }
    CATCH(excpt1)
    {
        PSZ pszErr = "*** error painting status bar";
        POINTL ptl = {5, 5};
        GpiMove(hps, &ptl);
        GpiCharString(hps, strlen(pszErr), pszErr);
    } END_CATCH();

    WinEndPaint(hps);
}

/*
 *@@ StatusPresParamChanged:
 *
 *@@added V0.9.16 (2001-10-28) [umoeller]
 */

STATIC VOID StatusPresParamChanged(HWND hwndBar,
                                   PSTATUSBARDATA psbd,
                                   MPARAM mp1)
{
    if (psbd->fDontBroadcast)
    {
        // this flag has been set if it was not this status
        // bar whose presparams have changed, but some other
        // status bar; in this case, update only
        psbd->fDontBroadcast = FALSE;
        // update parent's frame controls (because font size
        // might have changed)
        WinSendMsg(WinQueryWindow(hwndBar, QW_PARENT), WM_UPDATEFRAME, 0, 0);
        // update ourselves
        WinPostMsg(hwndBar, STBM_UPDATESTATUSBAR, 0, 0);
        // and quit
        return;
    }

    // else: it was us that the presparam has been set for
    PMPF_STATUSBARS(("WM_PRESPARAMCHANGED: %lX", mp1 ));

    // now check what has changed
    switch ((ULONG)mp1)
    {
        case PP_FONTNAMESIZE:
        {
            ULONG attrFound;
            // CHAR  szDummy[200];
            CHAR  szNewFont[100];
            WinQueryPresParam(hwndBar,
                              PP_FONTNAMESIZE,
                              0,
                              &attrFound,
                              (ULONG)sizeof(szNewFont),
                              (PVOID)&szNewFont,
                              0);
            cmnSetStatusBarSetting(SBS_STATUSBARFONT,
                                   szNewFont);
            // update parent's frame controls (because font size
            // might have changed)
            WinSendMsg(WinQueryWindow(hwndBar, QW_PARENT),
                       WM_UPDATEFRAME,
                       0,
                       0);
        }
        break;

        case PP_FOREGROUNDCOLOR:
        case PP_BACKGROUNDCOLOR:
        {
            ULONG   ul = 0,
                    attrFound = 0;

            WinQueryPresParam(hwndBar,
                              (ULONG)mp1,
                              0,
                              &attrFound,
                              (ULONG)sizeof(ul),
                              (PVOID)&ul,
                              0);
            if ((ULONG)mp1 == PP_FOREGROUNDCOLOR)
                cmnSetSetting(slSBTextColor, ul);
            else
                cmnSetSetting(slSBBgndColor, ul);

            WinPostMsg(hwndBar, STBM_UPDATESTATUSBAR, 0, 0);
        }
        break;
    }

    // finally, broadcast this message to all other status bars;
    // this is handled by the Worker thread
    PMPF_STATUSBARS(("posting WOM_UPDATEALLSTATUSBARS"));
    xthrPostWorkerMsg(WOM_UPDATEALLSTATUSBARS,
                      (MPARAM)2,      // update display
                      0);
}

/*
 *@@ fnwpStatusBar:
 *      since the status bar is just created as a static frame
 *      control, it is subclassed (by stbCreate),
 *      and this is the wnd proc for this.
 *
 *      This handles the new STBM_UPDATESTATUSBAR message (which
 *      is posted any time the status bar needs to be updated)
 *      and intercepts WM_TIMER and WM_PAINT.
 *
 *@@changed V0.9.0 [umoeller]: fixed context menu problem (now open view)
 *@@changed V0.9.0 [umoeller]: now using new XFolder methods to encapsulate status bar updates
 *@@changed V0.9.0 [umoeller]: adjusted to new gpihDraw3DFrame
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 *@@changed V0.9.1 (99-12-19) [umoeller]: finally fixed the context menu problems with MB2 right-click on status bar
 */

STATIC MRESULT EXPENTRY fnwpStatusBar(HWND hwndBar, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PSTATUSBARDATA psbd = (PSTATUSBARDATA)WinQueryWindowPtr(hwndBar, QWL_USER);
    MRESULT        mrc = 0;

    if (psbd)
    {
        PFNWP      pfnwpStatusBarOriginal = psbd->pfnwpStatusBarOriginal;

        switch(msg)
        {
            /*
             *@@ STBM_UPDATESTATUSBAR:
             *      update the status bar text. We will set a timer
             *      for a short delay to filter out repetitive
             *      messages here.
             *
             *      This timer is "one-shot" in that it will be
             *      started here and stopped as soon as WM_TIMER
             *      is received.
             *
             *      Parameters:
             *
             *      --  HWND mp1: if != NULLHANDLE, this sets a new
             *          container window for the status bar to
             *          retrieve its information from.
             *          V1.0.0 (2002-08-21) [umoeller]
             *
             *      --  mp2: always NULL.
             *
             *@@changed V1.0.0 (2002-08-21) [umoeller]: now allowing mp1 to change the hwndCnr
             */

            case STBM_UPDATESTATUSBAR:
                if (psbd->idTimer == 0)
                    // only if timer is not yet running: start it now
                    psbd->idTimer = WinStartTimer(psbd->habStatusBar, // anchor block
                                                  hwndBar,
                                                  1,
                                                  100); // delay: 100 ms

                if (mp1)
                    psbd->hwndCnr = (HWND)mp1;  // V1.0.0 (2002-08-21) [umoeller]
            break;

            /*
             * WM_TIMER:
             *      a timer is started by STBM_UPDATESTATUSBAR
             *      to avoid flickering;
             *      we now compose the actual text to be displayed
             */

            case WM_TIMER:
                if ((ULONG)mp1 == 1)
                    StatusTimer(hwndBar, psbd);
                else
                    mrc = pfnwpStatusBarOriginal(hwndBar, msg, mp1, mp2);
            break;

            /*
             * WM_PAINT:
             *      this, well, paints the status bar.
             *      Since the status bar is just a static control,
             *      we want to do some extra stuff to make it prettier.
             *      At this point, the window text (of the status bar)
             *      contains the fully translated status bar mnemonics,
             *      except for the tabulators ("$x" flags), which we'll
             *      deal with here.
             */

            case WM_PAINT:
                StatusPaint(hwndBar);
            break;

            /*
             * STBM_PROHIBITBROADCASTING:
             *      this msg is sent to us to tell us we will
             *      be given new presentation parameters soon;
             *      we will set a flag to TRUE in order to
             *      prevent broadcasting the pres params again.
             *      We need this flag because there are two
             *      situations in which we are given
             *      WM_PRESPARAMCHANGED messages:
             *      1)   the user has dropped something on this
             *           status bar window; in this case, we're
             *           getting WM_PRESPARAMCHANGED directly,
             *           without STBM_PROHIBITBROADCASTING, which
             *           means that we should notify the Worker
             *           thread to broadcast WM_PRESPARAMCHANGED
             *           to all status bars;
             *      2)   a status bar other than this one had been
             *           dropped something upon; in this case, we'll
             *           get WM_PRESPARAMCHANGED also (from the
             *           Worker thread), but STBM_PROHIBITBROADCASTING
             *           beforehand so that we know we should not
             *           notify the Worker thread again.
             *      See WM_PRESPARAMCHANGED below.
             */

            case STBM_PROHIBITBROADCASTING:
                psbd->fDontBroadcast = TRUE;
            break;

            /*
             * WM_PRESPARAMCHANGED:
             *      if fonts or colors were dropped on the bar, update
             *      GlobalSettings and have this message broadcast to all
             *      other status bars on the system.
             *      This is also posted to us by the Worker thread
             *      after some OTHER status bar has been dropped
             *      fonts or colors upon; in this case, psbd->fDontBroadcast
             *      is TRUE (see above), and we will only update our
             *      own display and NOT broadcast, because this is
             *      already being done.
             */

            case WM_PRESPARAMCHANGED:
                mrc = pfnwpStatusBarOriginal(hwndBar, msg, mp1, mp2);
                StatusPresParamChanged(hwndBar, psbd, mp1);
            break;

            /*
             * WM_BUTTON1CLICK:
             *      if button1 is clicked on the status
             *      bar, we should reset the focus to the
             *      container.
             */

            case WM_BUTTON1CLICK:
                WinSetFocus(HWND_DESKTOP,
                            psbd->hwndCnr);
            break;

            /*
             * WM_BUTTON1DBLCLK:
             *      on double-click on the status bar,
             *      open the folder's settings notebook.
             */

            case WM_BUTTON1DBLCLK:
                WinPostMsg(psbd->hwndFrame,
                           WM_COMMAND,
                           (MPARAM)WPMENUID_PROPERTIES,
                           MPFROM2SHORT(CMDSRC_MENU,
                                        FALSE) );     // results from keyboard operation
            break;

            /*
             * WM_CONTEXTMENU:
             *      if the user right-clicks on status bar,
             *      display folder's context menu. Parameters:
             *      mp1:
             *          POINTS mp1      pointer position, win coords
             *      mp2:
             *          USHORT usReserved  should be 0
             *          USHORT fPointer    if TRUE: results from keyboard
             */

            case WM_CONTEXTMENU:
            {
                POINTL  ptl;
                ptl.x = SHORT1FROMMP(mp1);
                ptl.y = SHORT2FROMMP(mp1)-20;   // this is in cnr coords!
                _wpDisplayMenu(psbd->somSelf,
                               psbd->hwndFrame, // owner
                               psbd->hwndCnr,   // parent
                               &ptl,
                               MENU_OPENVIEWPOPUP,
                               0);
            }
            break;

            /*
             * WM_DESTROY:
             *      call original wnd proc, then free psbd
             */

            case WM_DESTROY:
                mrc = pfnwpStatusBarOriginal(hwndBar, msg, mp1, mp2);
                WinSetWindowULong(hwndBar, QWL_USER, 0);
                free(psbd);
            break;

            default:
                mrc = pfnwpStatusBarOriginal(hwndBar, msg, mp1, mp2);

        } // end switch
    } // end if (psbd)

    return mrc;
}

/********************************************************************
 *
 *   Status bar text composition
 *
 ********************************************************************/

/*
 *@@ stbVar1000Double:
 *      generates an automatically scaled disk space string.
 *      Uses 1000 as the divisor for kB and mB.
 *
 *@@added V0.9.6 (2000-11-12) [pr]
 *@@changed V1.0.4 (2005-10-08) [pr]: fixed stupid bug in gB display being 1000 times too big
 */

PSZ stbVar1000Double(PSZ pszTarget,
                     double space,
                     CHAR cThousands)
{
    if (space < 1000.0)
        sprintf(pszTarget,
                "%.0f%s",
                space,
                cmnGetString( ((ULONG)space == 1)
                                 ? ID_XSSI_BYTE
                                 : ID_XSSI_BYTES));  // pszBytes
    else
        if (space < 10000.0)
            nlsVariableDouble(pszTarget, space, cmnGetString(ID_XSSI_BYTES),  cThousands); // pszBytes
        else
        {
            space /= 1000;
            if (space < 10000.0)
                nlsVariableDouble(pszTarget, space, " kB", cThousands);
            else
            {
                space /= 1000;
                if (space < 10000.0)
                    nlsVariableDouble(pszTarget, space, " mB", cThousands);
                else
                {
                    space /= 1000;
                    nlsVariableDouble(pszTarget, space, " gB", cThousands);
                }
            }
        }

    return pszTarget;
}

/*
 *@@ stbVar1024Double:
 *      generates an automatically scaled disk space string.
 *      Uses 1024 as the divisor for KB and MB.
 *
 *@@added V0.9.6 (2000-11-12) [pr]
 *@@changed V1.0.4 (2005-10-08) [pr]: fixed stupid bug in GB display being 1024 times too big
 */

PSZ stbVar1024Double(PSZ pszTarget,
                     double space,
                     CHAR cThousands)
{
    if (space < 1000.0)
        sprintf(pszTarget,
                "%.0f%s",
                space,
                cmnGetString(   (ULONG)(space == 1)
                                  ? ID_XSSI_BYTE
                                  : ID_XSSI_BYTES));  // pszBytes
    else
        if (space < 10240.0)
            nlsVariableDouble(pszTarget, space, cmnGetString(ID_XSSI_BYTES),  cThousands); // pszBytes
        else
        {
            space /= 1024;
            if (space < 10240.0)
                nlsVariableDouble(pszTarget, space, " KB", cThousands);
            else
            {
                space /= 1024;
                if (space < 10240.0)
                    nlsVariableDouble(pszTarget, space, " MB", cThousands);
                else
                {
                    space /= 1024;
                    nlsVariableDouble(pszTarget, space, " GB", cThousands);
                }
            }
        }

    return pszTarget;
}

#ifndef __NOCFGSTATUSBARS__

/*
 *@@ stbClassAddsNewMnemonics:
 *      returns TRUE if a class introduces new status bar
 *      mnemonics. Used by the "Status bars" notebook page
 *      in the "Select class" dialog to enable/disable
 *      classes which may be selected to set new status
 *      bar single-object information.
 *
 *@@changed V0.9.0 [umoeller]: now using _WPDisk instead of _XFldDisk
 */

BOOL stbClassAddsNewMnemonics(SOMClass *pClassObject)
{
    return (    (pClassObject == _XFldObject)
             || (pClassObject == _WPProgram)
             || (pClassObject == _WPDisk)
             || (pClassObject == _WPFileSystem)
             || (pClassObject == ctsResolveWPUrl()) // V1.0.1 (2002-12-08) [umoeller]
           );
}

/*
 *@@ stbSetClassMnemonics:
 *      this changes the status bar mnemonics for "single object"
 *      info for objects of this class and subclasses to
 *      pszText. If *pszText points to null-length string, no
 *      status bar info will be displayed; if pszText is NULL,
 *      the menmonics will be reset to the default value.
 *      This is called by the "Status bars" notebook page
 *      only when the user changes status bar mnemonics.
 *
 *      pClassObject may be one of the following:
 *      --  _XFldObject
 *      --  _WPProgram
 *      --  _WPDisk
 *      --  _WPFileSystem
 *      --  _WPUrl
 *
 +      This returns FALSE upon errors.
 *
 *@@changed V0.9.0 [umoeller]: now using _WPDisk instead of _XFldDisk
 *@@changed V0.9.0 [umoeller]: now returning FALSE upon errors
 *@@changed V0.9.5 (2000-09-20) [pr]: WPSharedDir has WPDisk status bar
 */

BOOL stbSetClassMnemonics(SOMClass *pClassObject,
                          PSZ pszText)
{
    SOMClass *pWPUrl;
    if (pWPUrl = ctsResolveWPUrl())
    {
        if (_somDescendedFrom(pClassObject, pWPUrl))
        {
            // provoke a reload of the settings
            // in stbQueryClassMnemonics
            G_szWPUrlStatusBarMnemonics[0] = '\0';

            // set the class mnemonics in OS2.INI; if
            // pszText == NULL, the key will be deleted,
            // and stbQueryClassMnemonics will use
            // the default value
            return PrfWriteProfileString(HINI_USERPROFILE,
                                         (PSZ)INIAPP_XWORKPLACE,
                                         (PSZ)INIKEY_SBTEXT_WPURL,
                                         pszText);
        }
    }

    // no WPUrl or WPUrl not installed: continue
    if (   (_somDescendedFrom(pClassObject, _WPDisk))
           // V0.9.5 (2000-09-20) [pr] WPSharedDir has disk status bar
        || (ctsDescendedFromSharedDir(pClassObject))
       )
    {
        // provoke a reload of the settings
        // in stbQueryClassMnemonics
        G_szWPDiskStatusBarMnemonics[0] = '\0';

        // set the class mnemonics in OS2.INI; if
        // pszText == NULL, the key will be deleted,
        // and stbQueryClassMnemonics will use
        // the default value
        return PrfWriteProfileString(HINI_USERPROFILE,
                                     (PSZ)INIAPP_XWORKPLACE,
                                     (PSZ)INIKEY_SBTEXT_WPDISK,
                                     pszText);
    }
    else if (_somDescendedFrom(pClassObject, _WPFileSystem))
    {
        // provoke a reload of the settings
        // in stbQueryClassMnemonics
        G_szWPFileSystemStatusBarMnemonics[0] = '\0';

        // set the class mnemonics in OS2.INI; if
        // pszText == NULL, the key will be deleted,
        // and stbQueryClassMnemonics will use
        // the default value
        return PrfWriteProfileString(HINI_USERPROFILE,
                                     (PSZ)INIAPP_XWORKPLACE,
                                     (PSZ)INIKEY_SBTEXT_WPFILESYSTEM,
                                     pszText);
    }
    else if (_somDescendedFrom(pClassObject, _WPProgram))
    {
        // provoke a reload of the settings
        // in stbQueryClassMnemonics
        G_szWPProgramStatusBarMnemonics[0] = '\0';

        // set the class mnemonics in OS2.INI; if
        // pszText == NULL, the key will be deleted,
        // and stbQueryClassMnemonics will use
        // the default value
        return PrfWriteProfileString(HINI_USERPROFILE,
                                     (PSZ)INIAPP_XWORKPLACE,
                                     (PSZ)INIKEY_SBTEXT_WPPROGRAM,
                                     pszText);
    }
    else if (_somDescendedFrom(pClassObject, _XFldObject))
    {
        // provoke a reload of the settings
        // in stbQueryClassMnemonics
        G_szXFldObjectStatusBarMnemonics[0] = '\0';

        // set the class mnemonics in OS2.INI; if
        // pszText == NULL, the key will be deleted,
        // and stbQueryClassMnemonics will use
        // the default value
        return PrfWriteProfileString(HINI_USERPROFILE,
                                     (PSZ)INIAPP_XWORKPLACE,
                                     (PSZ)INIKEY_SBTEXT_WPOBJECT,
                                     pszText);
    }

    return FALSE;
}

#endif

/*
 *@@ stbQueryClassMnemonics:
 *      this returns the status bar mnemonics for "single object"
 *      info for objects of pClassObject and subclasses. This string
 *      is either what has been specified on the "Status bar"
 *      notebook page or a default string from the XFolder NLS DLL.
 *      This is called every time the status bar needs to be updated
 *      (stbComposeText) and by the "Status bars" notebook page.
 *
 *      pClassObject may be _any_ valid WPS class. This function
 *      will automatically determine the correct class mnemonics
 *      from it.
 *
 *      Note: The return value of this function points to a _static_
 *      buffer, which must not be free()'d.
 *
 *@@changed V0.8.5 [umoeller]: fixed problem when WPProgram was replaced
 *@@changed V0.9.0 [umoeller]: function prototype changed to return PSZ instead of ULONG
 *@@changed V0.9.0 [umoeller]: now using _WPDisk instead of _XFldDisk
 *@@changed V0.9.5 (2000-09-20) [pr]: WPSharedDir has WPDisk status bar
 */

PSZ stbQueryClassMnemonics(SOMClass *pClassObject)    // in: class object of selected object
{
    PSZ pszReturn = NULL;

    SOMClass *pWPUrl;
    if (pWPUrl = ctsResolveWPUrl())
    {
        if (_somDescendedFrom(pClassObject, pWPUrl))
        {
            if (G_szWPUrlStatusBarMnemonics[0] == '\0')
#ifndef __NOCFGSTATUSBARS__
                // load string if this is the first time
                if (PrfQueryProfileString(HINI_USERPROFILE,
                                          (PSZ)INIAPP_XWORKPLACE,
                                          (PSZ)INIKEY_SBTEXT_WPURL,
                                          NULL,
                                          &(G_szWPUrlStatusBarMnemonics),
                                          sizeof(G_szWPUrlStatusBarMnemonics))
                        == 0)
#endif
                    // string not found in profile: set default
                    strcpy(G_szWPUrlStatusBarMnemonics, "\"$U\"$x(70%)$D $T");

            pszReturn = G_szWPUrlStatusBarMnemonics;
            // get out of here
            return pszReturn;
        }
    }

    if (    (_somDescendedFrom(pClassObject, _WPDisk))
            // V0.9.5 (2000-09-20) [pr] WPSharedDir has disk status bar
         || (ctsDescendedFromSharedDir(pClassObject))
       )
    {
        if (G_szWPDiskStatusBarMnemonics[0] == '\0')
#ifndef __NOCFGSTATUSBARS__
            // load string if this is the first time
            if (PrfQueryProfileString(HINI_USERPROFILE,
                                      (PSZ)INIAPP_XWORKPLACE,
                                      (PSZ)INIKEY_SBTEXT_WPDISK,
                                      NULL,
                                      &(G_szWPDiskStatusBarMnemonics),
                                      sizeof(G_szWPDiskStatusBarMnemonics))
                    == 0)
#endif
                // string not found in profile: load default from NLS resources
                WinLoadString(WinQueryAnchorBlock(HWND_DESKTOP),
                              cmnQueryNLSModuleHandle(FALSE),
                              ID_XSSI_SBTEXTWPDISK,
                              sizeof(G_szWPDiskStatusBarMnemonics),
                              G_szWPDiskStatusBarMnemonics);

        pszReturn = G_szWPDiskStatusBarMnemonics;
    }
    else if (_somDescendedFrom(pClassObject, _WPFileSystem))
    {
        if (G_szWPFileSystemStatusBarMnemonics[0] == '\0')
#ifndef __NOCFGSTATUSBARS__
            // load string if this is the first time
            if (PrfQueryProfileString(HINI_USERPROFILE,
                        (PSZ)INIAPP_XWORKPLACE,
                        (PSZ)INIKEY_SBTEXT_WPFILESYSTEM,
                        NULL, &(G_szWPFileSystemStatusBarMnemonics),
                        sizeof(G_szWPFileSystemStatusBarMnemonics))
                    == 0)
                // string not found in profile: load default from NLS resources
#endif
                WinLoadString(WinQueryAnchorBlock(HWND_DESKTOP),
                              cmnQueryNLSModuleHandle(FALSE),
                              ID_XSSI_SBTEXTWPDATAFILE,
                              sizeof(G_szWPFileSystemStatusBarMnemonics),
                              G_szWPFileSystemStatusBarMnemonics);

        pszReturn = G_szWPFileSystemStatusBarMnemonics;
    }
    //
    else if (_somDescendedFrom(pClassObject, _WPProgram))  // fixed V0.85
    {
        if (G_szWPProgramStatusBarMnemonics[0] == '\0')
#ifndef __NOCFGSTATUSBARS__
            // load string if this is the first time
            if (PrfQueryProfileString(HINI_USERPROFILE,
                                      (PSZ)INIAPP_XWORKPLACE,
                                      (PSZ)INIKEY_SBTEXT_WPPROGRAM,
                                      NULL,
                                      &(G_szWPProgramStatusBarMnemonics),
                                      sizeof(G_szWPProgramStatusBarMnemonics))
                    == 0)
#endif
                // string not found in profile: load default from NLS resources
                WinLoadString(WinQueryAnchorBlock(HWND_DESKTOP),
                              cmnQueryNLSModuleHandle(FALSE),
                              ID_XSSI_SBTEXTWPPROGRAM,
                              sizeof(G_szWPProgramStatusBarMnemonics),
                              G_szWPProgramStatusBarMnemonics);

        pszReturn = G_szWPProgramStatusBarMnemonics;
    }
    // subsidiarily: XFldObject
    else if (_somDescendedFrom(pClassObject, _XFldObject))
    {
        // should always be TRUE
        if (G_szXFldObjectStatusBarMnemonics[0] == '\0')
#ifndef __NOCFGSTATUSBARS__
            // load string if this is the first time
            if (PrfQueryProfileString(HINI_USERPROFILE,
                                      (PSZ)INIAPP_XWORKPLACE,
                                      (PSZ)INIKEY_SBTEXT_WPOBJECT,
                                      NULL,
                                      &(G_szXFldObjectStatusBarMnemonics),
                                      sizeof(G_szXFldObjectStatusBarMnemonics))
                        == 0)
#endif
                // string not found in profile: load default from NLS resources
                WinLoadString(WinQueryAnchorBlock(HWND_DESKTOP),
                              cmnQueryNLSModuleHandle(FALSE),
                              ID_XSSI_SBTEXTWPOBJECT,
                              sizeof(G_szXFldObjectStatusBarMnemonics),
                              G_szXFldObjectStatusBarMnemonics);

        pszReturn = G_szXFldObjectStatusBarMnemonics;
    } else
        pszReturn = "???";     // should not occur

    return pszReturn;
}

/*
 *@@ GetDivisor:
 *      returns a divisor value for the given character,
 *      which should be the third character in a typical
 *      \tX mnemonic. This was added with V0.9.11 to get
 *      rid of the terrible spaghetti code from the
 *      XFolder days in the "translate mnemnonic" funcs.
 *
 *      Valid format characters will produce the following
 *      return values:
 *
 *      -- 'b': 1, display bytes
 *
 *      -- 'k': 1000, display kBytes
 *
 *      -- 'K': 1024, display KBytes
 *
 *      -- 'm': 1000*1000, display mBytes
 *
 *      -- 'M': 1024*1024, display MBytes
 *
 *      -- 'a': special, -1, display bytes/kBytes/mBytes/gBytes
 *
 *      -- 'A': special, -2 display bytes/KBytes/MBytes/GBytes
 *
 *      *pcReplace receives the no. of characters to
 *      replace in the mnemonic string, which will
 *      be 3 if "c" was a valid format character.
 *
 *      If this returns 0, the "c" character was invalid,
 *      and *pcReplace is set to 2. Do not divide by zero,
 *      of course.
 *
 *@@added V0.9.11 (2001-04-22) [umoeller]
 */

STATIC ULONG GetDivisor(CHAR c,
                        PULONG pcReplace)      // out: chars to replace (2 or 3)
{
    *pcReplace = 3;

    switch (c)
    {
        case 'b':
            return 1;

        case 'k':
            return 1000;

        case 'K':
            return 1024;

        case 'm':
            return 1000*1000;

        case 'M':
            return 1024*1024;

        case 'a':
            return -1;          // special for stbVar1000Double

        case 'A':
            return -2;          // special for stbVar1024Double
    }

    // invalid code:
    *pcReplace = 2;

    return 0;
}

/*
 *@@ FormatDoubleValue:
 *      writes the formatted given "double" value
 *      to the given string buffer.
 *
 *      With ulDivisor, specify the value of GetDivisor().
 *
 *      pszBuf will ALWAYS contain something after
 *      this call.
 *
 *@@added V0.9.11 (2001-04-22) [umoeller]
 *@@changed V1.0.1 (2003-01-17) [umoeller]: now using PCOUNTRYSETTINGS2
 */

STATIC VOID FormatDoubleValue(PSZ pszBuf,              // out: formatted string
                              ULONG ulDivisor,         // in: divisor from GetDivisor()
                              double dbl,              // in: value to format
                              PCOUNTRYSETTINGS2 pcs)    // in: country settings for formatting
{
    switch (ulDivisor)
    {
        case -1:
            stbVar1000Double(pszBuf,
                             dbl,
                             pcs->cs.cThousands);
        break;

        case -2:
            stbVar1024Double(pszBuf,
                             dbl,
                             pcs->cs.cThousands);
        break;

        case 1:     // no division needed, avoid the calc below
            nlsThousandsDouble(pszBuf,
                               dbl,
                               pcs->cs.cThousands);
        break;

        case 0:     // GetDivisor() has detected a syntax error,
                    // avoid division by zero
            strcpy(pszBuf, "Syntax error.");
        break;

        default:        // "real" divisor specified:
        {
            double dValue = (dbl + (ulDivisor / 2)) / ulDivisor;
            nlsThousandsDouble(pszBuf,
                                dValue,
                                pcs->cs.cThousands);
        }
        break;
    }
}

/*
 *@@ CheckLogicalDrive:
 *      returns TRUE if the specified drive is ready
 *      to avoid the stupid "drive not ready" popups.
 *
 *      This expects that *pulLogicalDrive is -1 if
 *      the drive has not been checked for pDisk.
 *      If so, *pulLogicalDrive is set to pDisk's
 *      drive number if the drive is ready, or 0
 *      otherwise.
 *
 *      Returns TRUE if *pulLogicalDrive is != 0.
 *
 *@@added V0.9.13 (2001-06-14) [umoeller]
 *@@changed V0.9.16 (2001-10-04) [umoeller]: stopped the disk A: clicking for remote disks
 */

STATIC BOOL CheckLogicalDrive(PULONG pulLogicalDrive,
                              WPDisk *pDisk)
{
    if (*pulLogicalDrive == -1)
    {
        // make sure this disk is local;
        // _wpQueryLogicalDrive returns 1 for remote disks
        // and therefore used to provoke a click in drive A:
        // all the time... (how much stupidity can there be?!?)
        // V0.9.16 (2001-10-04) [umoeller]
        if (ctsIsSharedDir(pDisk))
            *pulLogicalDrive = 0;
        else
        {
            *pulLogicalDrive = _wpQueryLogicalDrive(pDisk);
            if (doshAssertDrive(*pulLogicalDrive, 0) != NO_ERROR)
                *pulLogicalDrive = 0;
        }

        PMPF_DISK(("*pulLogicalDrive = %d", *pulLogicalDrive));
    }

    return (*pulLogicalDrive != 0);
}

/*
 *@@ stbTranslateSingleMnemonics:
 *      this method is called on an object by stbComposeText
 *      after the status bar mnemonics have been queried
 *      for the object's class using stbQueryClassMnemonics,
 *      which is passed to this function in pszText.
 *
 *      Starting with V0.9.0, this function no longer gets
 *      shadows, but the dereferenced object in pObject,
 *      if "Dereference Shadows" is enabled in XFldWPS.
 *
 *      As opposed to the functions above, this does not
 *      take a class object, but an _instance_ as a parameter,
 *      because the info has to be displayed for each object
 *      differently.
 *
 *      Note that stbComposeText has changed all the '$' characters
 *      in pszText to the tabulator char ('\t') to avoid
 *      unwanted results if text inserted during the translations
 *      contains '$' chars. So we search for '\t' keys in this
 *      function.
 *
 *      This returns the number of keys that were translated.
 *
 *@@changed V0.9.0 [umoeller]: now using _WPDisk instead of _XFldDisk
 *@@changed V0.9.5 (2000-09-20) [pr]: WPSharedDir has WPDisk status bar
 *@@changed V0.9.6 (2000-11-01) [umoeller]: now using all new xstrFindReplace
 *@@changed V0.9.6 (2000-11-12) [umoeller]: removed cThousands param, now using cached country settings
 *@@changed V0.9.6 (2000-11-12) [pr]: new status bar mnemonics
 *@@changed V0.9.9 (2001-03-19) [pr]: fixed drive space problems
 *@@changed V0.9.11 (2001-04-22) [umoeller]: fixed most of the spaghetti in here
 *@@changed V0.9.11 (2001-04-22) [umoeller]: replaced excessive string searches with xstrrpl
 *@@changed V0.9.11 (2001-04-22) [umoeller]: added $zX mnemonics for total disk size
 *@@changed V0.9.11 (2001-04-22) [umoeller]: added $L mnemonic for disk label
 *@@changed V0.9.13 (2001-06-14) [umoeller]: fixed missing SOMFree
 *@@changed V1.0.1 (2003-01-17) [umoeller]: now using PCOUNTRYSETTINGS2
 */

ULONG stbTranslateSingleMnemonics(SOMClass *pObject,       // in: object
                                  PXSTRING pstrText,       // in/out: status bar text
                                  PCOUNTRYSETTINGS2 pcs)    // in: country settings
{
    ULONG       ulrc = 0;
    CHAR        szTemp[300];        // must be at least CCHMAXPATH!
    PSZ         p;
    ULONG       ulDivisor,
                cReplace;

    /*
     * WPUrl:
     *
     */

    // first check if the thing is a URL object;
    // in addition to the normal WPFileSystem mnemonics,
    // URL objects also support the $U mnemonic for
    // displaying the URL

    SOMClass *pWPUrl;
    if (    (pWPUrl = ctsResolveWPUrl())
         && (_somIsA(pObject, pWPUrl))
       )
    {
        // yes, we have a URL object:
        if (p = strstr(pstrText->psz, "\tU")) // URL mnemonic
        {
            CHAR szFilename[CCHMAXPATH];
            PSZ pszFilename = _wpQueryFilename(pObject, szFilename, TRUE);

            // read in the contents of the file, which
            // contains the URL
            PSZ pszURL = NULL;
            // offset where we found this:
            ULONG   ulFoundOfs = (p - pstrText->psz);

            if (pszFilename)
            {
                doshLoadTextFile(pszFilename, &pszURL, NULL);
                if (pszURL)
                {
                    if (strlen(pszURL) > 100)
                        strcpy(pszURL+97, "...");
                    xstrrpl(pstrText,
                            ulFoundOfs,
                            2,              // chars to replace
                            pszURL,
                            strlen(pszURL));
                    free(pszURL);
                }
            }

            if (!pszURL)
                xstrrpl(pstrText,
                        ulFoundOfs,
                        2,              // chars to replace
                        "?",
                        1);
            ulrc++;
        }
    }

    /*
     * WPDisk:
     *
     */

    if (    (_somIsA(pObject, _WPDisk))
            // V0.9.5 (2000-09-20) [pr] WPSharedDir has disk status bar
         || (ctsIsSharedDir(pObject))
       )
    {
        ULONG   ulLogicalDrive = -1;

        /* single-object status bar text mnemonics understood by WPDisk:

             $F      file system type (HPFS, FAT, CDFS, ...)

             $L      drive label V0.9.11 (2001-04-22) [umoeller]

             $fb     free space on drive in bytes
             $fk     free space on drive in kBytes
             $fK     free space on drive in KBytes
             $fm     free space on drive in mBytes
             $fM     free space on drive in MBytes
             $fa     free space on drive in bytes/kBytes/mBytes/gBytes V0.9.6
             $fA     free space on drive in bytes/KBytes/MBytes/GBytes V0.9.6

             added the following: V0.9.11 (2001-04-22) [umoeller]
             $zb     total space on drive in bytes
             $zk     total space on drive in kBytes
             $zK     total space on drive in KBytes
             $zm     total space on drive in mBytes
             $zM     total space on drive in MBytes
             $za     total space on drive in bytes/kBytes/mBytes/gBytes
             $zA     total space on drive in bytes/KBytes/MBytes/GBytes

            NOTE: the $f keys are also handled by stbComposeText, but
                  those only work properly for file-system objects, so we need
                  to calculate these values for these (abstract) disk objects

         */

        // the following are for free space on drive

        // removed spaghetti here V0.9.11 (2001-04-22) [umoeller]
        p = pstrText->psz;
        while (p = strstr(p, "\tf"))
        {
            double  dbl;
            // offset where we found this:
            ULONG   ulFoundOfs = (p - pstrText->psz);
            // get divisor from third mnemonic char
            ulDivisor = GetDivisor(*(p + 2),
                                      // one of 'b', 'k', 'K', 'm', 'M', 'a', or 'A'
                                      // ulDivisor is 0 if none of these
                                   &cReplace);  // out: chars to replace

            // get the value and format it
            if (    (CheckLogicalDrive(&ulLogicalDrive, pObject))
                            // sets ulLogicalDrive to 0 if drive not ready
                 && (!doshQueryDiskFree(ulLogicalDrive, &dbl))
               )
            {
                // we got a value: format it
                FormatDoubleValue(szTemp,
                                  ulDivisor,
                                  dbl,                  // value
                                  pcs);                 // country settings
                ulrc++;
            }
            else
            {
                szTemp[0] = '?';
                szTemp[1] = '\0';
            }

            // now replace: since we have found the string
            // already, we can safely use xstrrpl directly
            xstrrpl(pstrText,
                    // ofs of first char to replace:
                    ulFoundOfs,
                    // chars to replace:
                    cReplace,
                    // replacement string:
                    szTemp,
                    strlen(szTemp));

            // now adjust search pointer; pstrText might have been
            // reallocated, so search on in new buffer from here
            p = pstrText->psz + ulFoundOfs;  // szTemp could have been anything

            ulrc++;
        }

        // the following are for TOTAL space on drive V0.9.11 (2001-04-22) [umoeller]
        p = pstrText->psz;
        while (p = strstr(p, "\tz"))
        {
            double  dbl;
            // offset where we found this:
            ULONG   ulFoundOfs = (p - pstrText->psz);
            // get divisor from third mnemonic char
            ulDivisor = GetDivisor(*(p + 2),
                                      // one of 'b', 'k', 'K', 'm', 'M', 'a', or 'A'
                                      // ulDivisor is 0 if none of these
                                   &cReplace);  // out: chars to replace

            // get the value and format it
            if (    (CheckLogicalDrive(&ulLogicalDrive, pObject))
                            // sets ulLogicalDrive to 0 if drive not ready
                 && (!doshQueryDiskSize(ulLogicalDrive, &dbl))
               )
            {
                // we got a value: format it
                FormatDoubleValue(szTemp,
                                  ulDivisor,
                                  dbl,                  // value
                                  pcs);                 // country settings
                ulrc++;
            }
            else
            {
                szTemp[0] = '?';
                szTemp[1] = '\0';
            }

            // now replace: since we have found the string
            // already, we can safely use xstrrpl directly
            xstrrpl(pstrText,
                    // ofs of first char to replace:
                    ulFoundOfs,
                    // chars to replace:
                    cReplace,
                    // replacement string:
                    szTemp,
                    strlen(szTemp));

            // now adjust search pointer; pstrText might have been
            // reallocated, so search on in new buffer from here
            p = pstrText->psz + ulFoundOfs;  // szTemp could have been anything

            ulrc++;
        }
        // end total space V0.9.11 (2001-04-22) [umoeller]

        if (p = strstr(pstrText->psz, "\tF"))  // file-system type (HPFS, ...)
        {
            if (    (!CheckLogicalDrive(&ulLogicalDrive, pObject))
                            // sets ulLogicalDrive to 0 if drive not ready
                 || (doshQueryDiskFSType(ulLogicalDrive,
                                        szTemp,
                                        sizeof(szTemp)))
               )
            {
                szTemp[0] = '?';
                szTemp[1] = '\0';
            }

            xstrrpl(pstrText,
                    // ofs of first char to replace:
                    (p - pstrText->psz),
                    // chars to replace:
                    2,
                    // replacement string:
                    szTemp,
                    strlen(szTemp));

            ulrc++;
        }

        // drive label V0.9.11 (2001-04-22) [umoeller]
        if (p = strstr(pstrText->psz, "\tL"))
        {
            if (    (!CheckLogicalDrive(&ulLogicalDrive, pObject))
                            // sets ulLogicalDrive to 0 if drive not ready
                 || (doshQueryDiskLabel(ulLogicalDrive,
                                        szTemp))
               )
            {
                szTemp[0] = '?';
                szTemp[1] = '\0';
            }

            xstrrpl(pstrText,
                    // ofs of first char to replace:
                    (p - pstrText->psz),
                    // chars to replace:
                    2,
                    // replacement string:
                    szTemp,
                    strlen(szTemp));

            ulrc++;
        }
    }

    /*
     * WPFileSystem:
     *
     */

    else if (_somIsA(pObject, _WPFileSystem))
    {
        FILEFINDBUF4    ffbuf4;
        BOOL            fBufLoaded = FALSE;     // V0.9.6 (2000-11-12) [umoeller]

        /* single-object status bar text mnemonics understood by WPFileSystem
           (in addition to those introduced by XFldObject):

             $r      object's real name

             $y      object type (.TYPE EA)
             $D      object creation date
             $T      object creation time
             $a      object attributes

             $Eb     EA size in bytes
             $Ek     EA size in kBytes
             $EK     EA size in KBytes
             $Ea     EA size in bytes/kBytes (V0.9.6)
             $EA     EA size in bytes/KBytes (V0.9.6)
         */

        if (p = strstr(pstrText->psz, "\ty")) // type
        {
            // offset where we found this:
            PSZ p2;
            if (!(p2 = _wpQueryType(pObject)))
                p2 = "?";
            xstrrpl(pstrText,
                    // ofs of first char to replace:
                    (p - pstrText->psz),
                    // chars to replace:
                    2,
                    // replacement string:
                    p2,
                    strlen(p2));
            ulrc++;
        }

        if (p = strstr(pstrText->psz, "\tD"))  // date
        {
            szTemp[0] = '?';
            szTemp[1] = '\0';

            _wpQueryDateInfo(pObject, &ffbuf4);
            fBufLoaded = TRUE;
            nlsFileDate(szTemp,
                         &ffbuf4.fdateLastWrite,
                         pcs);
            xstrrpl(pstrText,
                    // ofs of first char to replace:
                    (p - pstrText->psz),
                    // chars to replace:
                    2,
                    // replacement string:
                    szTemp,
                    strlen(szTemp));
            ulrc++;
        }

        if (p = strstr(pstrText->psz, "\tT"))  // time
        {
            szTemp[0] = '?';
            szTemp[1] = '\0';

            if (!fBufLoaded)
                _wpQueryDateInfo(pObject, &ffbuf4);
            nlsFileTime(szTemp,
                         &ffbuf4.ftimeLastWrite,
                         pcs);
            xstrrpl(pstrText,
                    // ofs of first char to replace:
                    (p - pstrText->psz),
                    // chars to replace:
                    2,
                    // replacement string:
                    szTemp,
                    strlen(szTemp));
            ulrc++;
        }

        if (p = strstr(pstrText->psz, "\ta")) // attribs
        {
            ULONG fAttr = _wpQueryAttr(pObject);
            szTemp[0] = (fAttr & FILE_ARCHIVED) ? 'A' : 'a';
            szTemp[1] = (fAttr & FILE_HIDDEN  ) ? 'H' : 'h';
            szTemp[2] = (fAttr & FILE_READONLY) ? 'R' : 'r';
            szTemp[3] = (fAttr & FILE_SYSTEM  ) ? 'S' : 's';
            szTemp[4] = '\0';
            xstrrpl(pstrText,
                    // ofs of first char to replace:
                    (p - pstrText->psz),
                    // chars to replace:
                    2,
                    // replacement string:
                    szTemp,
                    strlen(szTemp));
            ulrc++;
        }

        // removed spaghetti here V0.9.11 (2001-04-22) [umoeller]
        // note: this makes the $E codes support megabyte format
        // chars as well, but this would be silly to specify...
        p = pstrText->psz;
        while (p = strstr(p, "\tE")) // easize
        {
            double dEASize = (double)_wpQueryEASize(pObject);
            // offset where we found this:
            ULONG   ulFoundOfs = (p - pstrText->psz);

            // get divisor from third mnemonic char
            ulDivisor = GetDivisor(*(p + 2),
                                      // one of 'b', 'k', 'K', 'm', 'M', 'a', or 'A'
                                      // ulDivisor is 0 if none of these
                                   &cReplace);  // out: chars to replace

            FormatDoubleValue(szTemp,
                              ulDivisor,
                              dEASize,
                              pcs);

            // now replace: since we have found the string
            // already, we can safely use xstrrpl directly
            xstrrpl(pstrText,
                    // ofs of first char to replace:
                    ulFoundOfs,
                    // chars to replace:
                    cReplace,
                    // replacement string:
                    szTemp,
                    strlen(szTemp));

            // now adjust search pointer; pstrText might have been
            // reallocated, so search on in new buffer from here
            p = pstrText->psz + ulFoundOfs;  // szTemp could have been anything

            ulrc++;
        }

        if (p = strstr(pstrText->psz, "\tr")) // real name
        {
            szTemp[0] = '?';
            szTemp[1] = '\0';

            _wpQueryFilename(pObject, szTemp, FALSE);
            xstrrpl(pstrText,
                    // ofs of first char to replace:
                    (p - pstrText->psz),
                    // chars to replace:
                    2,
                    // replacement string:
                    szTemp,
                    strlen(szTemp));
            ulrc++;
        }
    }

    /*
     * WPProgram:
     *
     */

    else if (_somIsA(pObject, _WPProgram))
    {
        PPROGDETAILS pDetails = NULL;
        ULONG       ulSize;

        /* single-object status bar text mnemonics understood by WPProgram
           (in addition to those introduced by XFldObject):

            $p      executable program file (as specified in the Settings)
            $P      parameter list (as specified in the Settings)
            $d      working directory (as specified in the Settings)
         */

        if (p = strstr(pstrText->psz, "\tp"))  // program executable
        {
            szTemp[0] = '?';
            szTemp[1] = '\0';

            if (!pDetails)
                pDetails = progQueryDetails(pObject);

            if (pDetails)
                if (pDetails->pszExecutable)
                    strlcpy(szTemp, pDetails->pszExecutable, sizeof(szTemp));
                else
                    szTemp[0] = '\0';

            xstrrpl(pstrText,
                    // ofs of first char to replace:
                    (p - pstrText->psz),
                    // chars to replace:
                    2,
                    // replacement string:
                    szTemp,
                    strlen(szTemp));
            ulrc++;
        }

        if (p = strstr(pstrText->psz, "\tP"))  // program parameters
        {
            szTemp[0] = '?';
            szTemp[1] = '\0';

            if (!pDetails)
                pDetails = progQueryDetails(pObject);

            if (pDetails)
                if (pDetails->pszParameters)
                    strlcpy(szTemp, pDetails->pszParameters, sizeof(szTemp));
                else
                    szTemp[0] = '\0';

            xstrrpl(pstrText,
                    // ofs of first char to replace:
                    (p - pstrText->psz),
                    // chars to replace:
                    2,
                    // replacement string:
                    szTemp,
                    strlen(szTemp));
            ulrc++;
        }

        if (p = strstr(pstrText->psz, "\td"))  // startup dir
        {
            szTemp[0] = '?';
            szTemp[1] = '\0';

            if (!pDetails)
                pDetails = progQueryDetails(pObject);

            if (pDetails)
                if (pDetails->pszStartupDir)
                    strlcpy(szTemp, pDetails->pszStartupDir, sizeof(szTemp));
                else
                    szTemp[0] = '\0';

            xstrrpl(pstrText,
                    // ofs of first char to replace:
                    (p - pstrText->psz),
                    // chars to replace:
                    2,
                    // replacement string:
                    szTemp,
                    strlen(szTemp));
            ulrc++;
        }

        if (pDetails)
            free(pDetails);
    }

    /*
     * XFldObject:
     *
     */

    if (_somIsA(pObject, _WPObject))      // should always be TRUE
    {
        /* single-object status bar text mnemonics understood by WPObject:
             $t      object title
             $w      WPS class default title (e.g. "Data file")
             $W      WPS class name (e.g. WPDataFile)
         */

        if (p = strstr(pstrText->psz, "\tw"))     // class default title
        {
            SOMClass    *pClassObject = _somGetClass(pObject);
            PSZ         pszValue = NULL;

            if (pClassObject)
                pszValue = _wpclsQueryTitle(pClassObject);
            if (!pszValue)
                pszValue = "?";

            xstrrpl(pstrText,
                    // ofs of first char to replace:
                    (p - pstrText->psz),
                    // chars to replace:
                    2,
                    // replacement string:
                    pszValue,
                    strlen(pszValue));
            ulrc++;
        }

        if (p = strstr(pstrText->psz, "\tW"))     // class name
        {
            PSZ pszValue = _somGetClassName(pObject);
            if (!pszValue)
                pszValue = "?";
            xstrrpl(pstrText,
                    // ofs of first char to replace:
                    (p - pstrText->psz),
                    // chars to replace:
                    2,
                    // replacement string:
                    pszValue,
                    strlen(pszValue));
            ulrc++;
        }

        if (p = strstr(pstrText->psz, "\tt"))          // object title
        {
            PCSZ pcszTitle;
            if (pcszTitle = _wpQueryTitle(pObject))
            {
                strlcpy(szTemp, pcszTitle, sizeof(szTemp));
                strhBeautifyTitle(szTemp);
            }
            else
            {
                szTemp[0] = '?';
                szTemp[1] = '\0';
            }

            xstrrpl(pstrText,
                    // ofs of first char to replace:
                    (p - pstrText->psz),
                    // chars to replace:
                    2,
                    // replacement string:
                    szTemp,
                    strlen(szTemp));
            ulrc++;
        }
    }

    return ulrc;
}

/*
 *@@ ReplaceKeyWithDouble:
 *      replaces the given key with the given
 *      "double" value, using the current
 *      country settings.
 *
 *      pcszKey is assumed to contain the first
 *      two characters of a three-character code,
 *      where the third character specifies the
 *      format to use (see GetDivisor).
 *
 *@@added V0.9.19 (2002-06-02) [umoeller]
 *@@changed V1.0.1 (2003-01-17) [umoeller]: now using PCOUNTRYSETTINGS2
 */

STATIC VOID ReplaceKeyWithDouble(XSTRING *pstrText, // in/out: status bar text
                                 PCSZ pcszKey,      // in: two-character key to search for
                                 double dValue,     // in: value to replace three chars with
                                 PCOUNTRYSETTINGS2 pcs)  // in: country settings for formatting
{
    CHAR        szTemp[300];
    PSZ         p = pstrText->psz;

    while (p = strstr(p, pcszKey))
    {
        // offset where we found this:
        ULONG   ulFoundOfs = (p - pstrText->psz),
                cReplace,
                ulDivisor;

        // get divisor from third mnemonic char
        ulDivisor = GetDivisor(*(p + 2),
                                  // one of 'b', 'k', 'K', 'm', 'M', 'a', or 'A'
                                  // ulDivisor is 0 if none of these
                               &cReplace);  // out: chars to replace

        // get the value and format it
        FormatDoubleValue(szTemp,
                          ulDivisor,
                          dValue,
                          pcs);                 // country settings

        // now replace: since we have found the string
        // already, we can safely use xstrrpl directly
        xstrrpl(pstrText,
                // ofs of first char to replace:
                ulFoundOfs,
                // chars to replace:
                cReplace,
                // replacement string:
                szTemp,
                strlen(szTemp));

        // now adjust search pointer; pstrText might have been
        // reallocated, so search on in new buffer from here
        p = pstrText->psz + ulFoundOfs;  // szTemp could have been anything
    }
}

/*
 *@@ stbComposeText:
 *      this is the main entry point to the status bar
 *      logic which gets called by the status bar wnd
 *      proc (fnwpStatusBar) when the status
 *      bar text needs updating (CN_EMPHASIS).
 *
 *      This function does the following:
 *
 *      1)  First, it finds out which mode the status bar
 *          should operate in (depending on how many objects
 *          are currently selected in hwndCnr).
 *
 *      2)  Then, we find out which mnemonic string to use,
 *          depending on the mode we found out above.
 *          If we're in one-object mode, we call
 *          stbQueryClassMnemonics for the mnemonics to
 *          use depending on the class of the one object
 *          which is selected.
 *
 *      3)  If we're in one-object mode, we call
 *          stbTranslateSingleMnemonics to have class-dependend
 *          mnemonics translated.
 *
 *      4)  Finally, for all modes, we translate the
 *          mnemonics which are valid for all modes in this
 *          function directly.
 *
 *      This returns the new status bar text or NULL upon errors.
 *      The return PSZ must be free()'d by the caller.
 *
 *@@changed V0.9.0 [umoeller]: prototype changed to return new buffer instead of BOOL
 *@@changed V0.9.0 [umoeller]: added shadow dereferencing
 *@@changed V0.9.3 (2000-04-08) [umoeller]: added cnr error return code check
 *@@changed V0.9.5 (2000-10-07) [umoeller]: added "Dereference shadows" for multiple mode
 *@@changed V0.9.6 (2000-11-01) [umoeller]: now using all new xstrFindReplace
 *@@changed V0.9.6 (2000-10-30) [pr]: fixed container item counts
 *@@changed V0.9.6 (2000-11-12) [pr]: new status bar mnemonics
 *@@changed V0.9.11 (2001-04-22) [umoeller]: fixed most of the spaghetti in here
 *@@changed V0.9.11 (2001-04-22) [umoeller]: replaced excessive string searches with xstrrpl
 *@@changed V0.9.11 (2001-04-22) [umoeller]: merged three cnr record loops into one (speed)
 *@@changed V0.9.11 (2001-04-22) [umoeller]: added $zX mnemonics for total disk size
 *@@changed V0.9.11 (2001-04-25) [umoeller]: fixed tree view bug introduced on 2001-04-22
 *@@changed V0.9.19 (2002-06-02) [umoeller]: fixed overflow with > 4 GB selected files, more cleanup
 *@@changed V1.0.9 (2010-07-17) [pr]: added large file support @@fixes 586
 */

PSZ stbComposeText(WPFolder* somSelf,      // in:  open folder with status bar
                   HWND hwndCnr)           // in:  cnr hwnd of that folder's open view
{
   /* Generic status bar mnemonics handled here (for other than
      "single object" mode too):

        $c      no. of selected objects
        $C      total object count

        $fb     free space on drive in bytes
        $fk     free space on drive in kBytes
        $fK     free space on drive in KBytes
        $fm     free space on drive in mBytes
        $fM     free space on drive in MBytes
        $fa     free space on drive in bytes/kBytes/mBytes/gBytes (V0.9.6)
        $fA     free space on drive in bytes/KBytes/MBytes/GBytes (V0.9.6)

        $zb     total size of drive in bytes V0.9.11 (2001-04-22) [umoeller]
        $zk     total size of drive in kBytes
        $zK     total size of drive in KBytes
        $zm     total size of drive in mBytes
        $zM     total size of drive in MBytes
        $za     total size of drive in bytes/kBytes/mBytes/gBytes
        $zA     total size of drive in bytes/KBytes/MBytes/GBytes

        $sb     size of selected objects in bytes
        $sk     size of selected objects in kBytes
        $sK     size of selected objects in KBytes
        $sm     size of selected objects in mBytes
        $sM     size of selected objects in MBytes
        $sa     size of selected objects in bytes/kBytes/mBytes/gBytes (V0.9.6)
        $sA     size of selected objects in bytes/KBytes/MBytes/GBytes (V0.9.6)

        $Sb     size of folder content in bytes
        $Sk     size of folder content in kBytes
        $SK     size of folder content in KBytes
        $Sm     size of folder content in mBytes
        $SM     size of folder content in MBytes
        $Sa     size of folder content in bytes/kBytes/mBytes/gBytes (V0.9.6)
        $SA     size of folder content in bytes/KBytes/MBytes/GBytes (V0.9.6)

       The "single object" mode mnemonics are translated using
       the above functions afterwards.
    */

    XSTRING     strText;

    ULONG       cVisibleRecords = 0,
                cSelectedRecords = 0;
    // made these doubles to avoid overflows
    // V0.9.19 (2002-06-02) [umoeller]
    double      cbSelected = 0,
                cbTotal = 0;
    ULONG       ulDivisor,
                cReplace;
    USHORT      cmd;
    WPObject    *pobj = NULL,
                *pobjSelected = NULL;
    PSZ         p;
    CHAR        *p2;
    PMINIRECORDCORE prec;

    // get country settings from "Country" object
    PCOUNTRYSETTINGS2 pcs = cmnQueryCountrySettings(FALSE);
    CHAR        szTemp[300];

    xstrInit(&strText, 300);

    // V0.9.11 (2001-04-22) [umoeller]: now, we used to have
    // THREE loops here which ran through the objects in the
    // container, producing a WinSendMsg for each record...
    // since that code ALWAYS ran through all objects at least
    // once (Paul's fix for the invisible object count), I
    // merged the three loops into one now. This can make a
    // noticeable difference when 1000 objects have been
    // selected in the container and the user selects a large
    // number of objects.

    // run-through-all-objects loop...
    for (prec = NULL, cmd = CMA_FIRST;
         ;
         cmd = CMA_NEXT)
    {
        prec = (PMINIRECORDCORE)WinSendMsg(hwndCnr,
                                           CM_QUERYRECORD,
                                           (MPARAM)prec,
                                           MPFROM2SHORT(cmd,     // CMA_FIRST or CMA_NEXT
                                                        CMA_ITEMORDER));
        if (    (prec == NULL)
             || ((ULONG)prec == -1)
           )
            // error, or last:
            break;

        // count all visible objects
        if (!(prec->flRecordAttr & CRA_FILTERED))
        {
            BOOL    fThisSelected = ((prec->flRecordAttr & CRA_SELECTED) != 0);

            // count all visible records
            cVisibleRecords++;

            // now get the object from the MINIRECORDCORE
            if (    (pobj = OBJECT_FROM_PREC(prec))
                 && (wpshCheckObject(pobj))
               )
            {
                WPObject    *pDeref = pobj;

                if (fThisSelected)
                {
                    // count all selected records
                    cSelectedRecords++;
                    pobjSelected = pobj;        // NOT dereferenced if shadow!
                }

#ifndef __NOCFGSTATUSBARS__
                if (cmnQuerySetting(sflDereferenceShadows) & STBF_DEREFSHADOWS_MULTIPLE)
                {
                    // deref multiple shadows
                    pDeref = _xwpResolveIfLink(pDeref);
                }
#endif

                if (pDeref && _somIsA(pDeref, _WPFileSystem))
                {
                    // OK, we got a WPFileSystem:
                    LONGLONG llThis;  // V1.0.9
                    double cbThis;

                    _wpQueryFileSizeL(pDeref, &llThis);
                    cbThis = 65536.0 * 65536.0 * llThis.ulHi + llThis.ulLo;

                    if (fThisSelected)
                        // count bytes of selected objects
                        cbSelected += cbThis;

                    cbTotal += cbThis;
                }
            }
            else
                pobj = NULL;

        } // end if (!(prec->flRecordAttr & CRA_FILTERED))
    } // end for (prec = NULL, cmd = CMA_FIRST; ...

    // pobjSelected is now NULL for single-object mode
    // or contains the last selected object we had in the loop;
    // in one-object mode, it'll be the only object
    // and might still point to a shadow.

    // However, in tree view, there might be a selected record
    // which was not found in the above loop because the loop
    // didn't go thru child records... so do an additional check:
    // V0.9.11 (2001-04-25) [umoeller]
    if (!pobjSelected)
    {
        // send an extra "find selected record" msg; this is
        // sufficient for all cases, since
        // a)  if we're in a tree view, only one record can
        //     be selected in the first place;
        // b)  if we're not in a tree view and pobjSelected
        //     is NULL, we won't find any records with this
        //     message either, because there can't be any
        //     selected records which weren't already found
        //     in the above loop.
        // As a result, we can save ourselves from querying
        // whether the cnr is in tree view in the first place.

        prec = WinSendMsg(hwndCnr,
                          CM_QUERYRECORDEMPHASIS,
                          (MPARAM)CMA_FIRST,
                          (MPARAM)CRA_SELECTED);
        if (    (prec)
             && ((ULONG)prec != -1)
           )
        {
            // if we got a selected record HERE, we MUST
            // be in tree view, so there can be only one
            pobjSelected = OBJECT_FROM_PREC(prec);
            cSelectedRecords = 1;
        }
    }

    // now get the mnemonics which have been set by the
    // user on the "Status bar" page, depending on how many
    // objects are selected
    if ( (cSelectedRecords == 0) || (pobjSelected == NULL) )
        // "no object" mode:
        xstrcpy(&strText, cmnQueryStatusBarSetting(SBS_TEXTNONESEL), 0);
    else if (cSelectedRecords == 1)
    {
        // "single-object" mode: query the text to translate
        // from the object, because we can implement
        // different mnemonics for different WPS classes

        // dereference shadows (V0.9.0)
#ifndef __NOCFGSTATUSBARS__
        if (cmnQuerySetting(sflDereferenceShadows) & STBF_DEREFSHADOWS_SINGLE)
#endif
            pobjSelected = _xwpResolveIfLink(pobjSelected);

        if (pobjSelected == NULL)
            return strdup("");

        xstrcpy(&strText, stbQueryClassMnemonics(_somGetClass(pobjSelected)), 0);
                                                  // object's class object
    }
    else
        // "multiple objects" mode
        xstrcpy(&strText, cmnQueryStatusBarSetting(SBS_TEXTMULTISEL), 0);

    // before actually translating any "$" keys, all the
    // '$' characters in the mnemonic string are changed to
    // the tabulator character ('\t') to avoid having '$'
    // characters translated which might only have been
    // inserted during the translation, e.g. because a
    // filename contains a '$' character.
    // All the translation logic will then only search for
    // those tab characters.
    // V0.9.11 (2001-04-22) [umoeller]: sped it up a bit
    /* while (p2 = strchr(strText.psz, '$'))
        *p2 = '\t'; */
    p2 = strText.psz;
    while (p2 = strchr(p2, '$'))
        *p2++ = '\t';

    if (cSelectedRecords == 1)
    {
        // "single-object" mode: translate
        // object-specific mnemonics first
        stbTranslateSingleMnemonics(pobjSelected,
                                    &strText,
                                    pcs);
        if (_somIsA(pobjSelected, _WPFileSystem))
        {
            LONGLONG llSel;  // V1.0.9
            // if we have a file-system object, we
            // need to re-query its size, because
            // we might have dereferenced a shadow
            // above (whose size was 0 -- V0.9.0)
            _wpQueryFileSizeL(pobjSelected, &llSel);
            cbSelected = 65536.0 * 65536.0 * llSel.ulHi + llSel.ulLo;
        }
    }

    // NOW GO TRANSLATE COMMON CODES

    if (p = strstr(strText.psz, "\tc")) // selected objs count
    {
        sprintf(szTemp, "%u", cSelectedRecords);
        xstrrpl(&strText,
                // ofs of first char to replace:
                (p - strText.psz),
                // chars to replace:
                2,
                // replacement string:
                szTemp,
                strlen(szTemp));
    }

    if (p = strstr(strText.psz, "\tC")) // total obj count
    {
        sprintf(szTemp, "%u", cVisibleRecords);
        xstrrpl(&strText,
                // ofs of first char to replace:
                (p - strText.psz),
                // chars to replace:
                2,
                // replacement string:
                szTemp,
                strlen(szTemp));
    }

    // free space on drive
    ReplaceKeyWithDouble(&strText,
                         "\tf",
                         wpshQueryDiskFreeFromFolder(somSelf),
                         pcs);

    // total disk size
    ReplaceKeyWithDouble(&strText,
                         "\tz",
                         wpshQueryDiskSizeFromFolder(somSelf),
                         pcs);

    // SELECTED size
    ReplaceKeyWithDouble(&strText,
                         "\ts",
                         cbSelected,
                         pcs);

    // TOTAL folder size
    ReplaceKeyWithDouble(&strText,
                         "\tS",
                         cbTotal,
                         pcs);

    // prepend icon position in debug mode
    #ifdef __DEBUG__
    if (    (cSelectedRecords == 1)
         && (pobjSelected)
         && (prec = _wpQueryCoreRecord(pobjSelected))
       )
    {
        XSTRING strNew;
        CHAR sz[100];
        sprintf(sz, "(%d, %d) ", prec->ptlIcon.x, prec->ptlIcon.y);
        xstrInitCopy(&strNew,
                     sz,
                     0);
        xstrcats(&strNew, &strText);
        xstrcpys(&strText, &strNew);
        xstrClear(&strNew);
    }

    // prepend class name in debug mode
    // V0.9.19 (2002-06-15) [umoeller]
    if (cSelectedRecords == 1)
    {
        XSTRING strNew;
        CHAR sz[300];
        sprintf(sz,
                "0x%lX %d {%s} ",
                pobjSelected,           // obj pointer V1.0.1 (2002-12-08) [umoeller]
                _xwpQueryLocks(pobjSelected),
                _somGetClassName(pobjSelected));
        xstrInitCopy(&strNew,
                     sz,
                     0);
        xstrcats(&strNew, &strText);
        xstrcpys(&strText, &strNew);
        xstrClear(&strNew);
    }
    #endif

    if (strText.ulLength)
    {
        // alright, we have something:

        // now translate remaining '\t' characters back into
        // '$' characters; this might happen if the user actually
        // wanted to see a '$' character displayed for his bank account
        while (p2 = strchr(strText.psz, '\t'))
            *p2 = '$';

        return strText.psz;       // do not free the string!
    }

    // else:
    xstrClear(&strText);

    return NULL;
}

/* ******************************************************************
 *
 *   Tool bar creation
 *
 ********************************************************************/

/*
 *@@ stbCreateToolBar:
 *
 *@@added V1.0.1 (2002-12-08) [umoeller]
 */

HWND stbCreateToolBar(WPFolder *pFolder,
                      HWND hwndFrame,
                      ULONG ulView,             // in: OPEN_CONTENTS etc.
                      PLONG plToolBarHeight)    // out: computed height of toolbar control
{
    // create tool bar as child of the frame
    HWND hwndToolBar = NULLHANDLE;
    PTOOLBARCONTROL patbc;
    ULONG       cControls;
    LINKLIST    ll;
    PLISTNODE   pNode;

    PMPF_SPLITVIEW(("Creating toolbar"));

    // create a LINKLIST and pass it as hToolBar to
    // _xwpBuildToolBar; xwpAddToolbarButton malloc's
    // TOOLBARCONTROL so we can create this as auto-free
    lstInit(&ll, TRUE);
    if (    (_xwpBuildToolBar(pFolder,
                              (ULONG)&ll,
                              ulView))
         && (cControls = lstCountItems(&ll))
         && (patbc = (PTOOLBARCONTROL)malloc(cControls * sizeof(TOOLBARCONTROL)))
       )
    {
        PTOOLBARCONTROL pThis = patbc;
        FOR_ALL_NODES(&ll, pNode)
        {
            memcpy(pThis, pNode->pItemData, sizeof(TOOLBARCONTROL));
            pThis++;
        }

        PMPF_SPLITVIEW(("  _xwpBuildToolBar returned %d controls", cControls));

        if (hwndToolBar = ctlCreateToolBar(hwndFrame,
                                           hwndFrame,
                                           WS_VISIBLE
                                               | (cmnQuerySetting(sfTBToolTips)
                                                      ? TBS_TOOLTIPS
                                                      : 0)
                                               | TBS_AUTORESIZE,
                                           // owner for controls:
                                           hwndFrame,
                                           cControls,
                                           patbc))
        {
            SWP swpBar;
            WinQueryWindowPos(hwndToolBar,
                              &swpBar);
            *plToolBarHeight = swpBar.cy;
        }

        PMPF_SPLITVIEW(("  hwndToolBar is 0x%lX, height %d",
                        hwndToolBar,
                        *plToolBarHeight));

        free(patbc);
    }

    lstClear(&ll);

    return hwndToolBar;

}
