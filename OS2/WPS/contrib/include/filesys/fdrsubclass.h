
/*
 *@@sourcefile fdrsubclass.h:
 *      header file for folder.c (XFolder implementation).
 *
 *      This file is new with V1.0.0 (2002-08-28) [umoeller].
 *
 *@@include #include <os2.h>
 *@@include #include <wpfolder.h>
 *@@include #include "filesys\fdrsubclass.h"
 */

/*
 *      Copyright (C) 1997-2003 Ulrich M”ller.
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

#ifndef FDRSUBCLASS_HEADER_INCLUDED
    #define FDRSUBCLASS_HEADER_INCLUDED

    /********************************************************************
     *
     *   Folder frame window subclassing
     *
     ********************************************************************/

    /*
     *@@ SUBCLFOLDERVIEW:
     *      structure used with folder frame window subclassing.
     *
     *      One of these structures is created for each folder
     *      view frame which is subclassed by fdrSubclassFolderView
     *      and stored in a window word of the folder frame.
     *      See fdrCreateSFV() for details.
     *
     *      This is one of the most important kludges which
     *      XFolder uses to hook itself into the WPS.
     *      Most importantly, this structure stores the
     *      original frame window procedure before the
     *      folder frame window was subclassed, but we also
     *      use this to store various other data for status
     *      bars etc.
     *
     *      We need this additional structure because all
     *      the data in here is _view_-specific, not
     *      folder-specific, so we cannot store this in
     *      the instance data.
     *
     *@@changed V0.9.1 (2000-01-29) [umoeller]: added pSourceObject and ulSelection fields
     *@@changed V0.9.2 (2000-03-06) [umoeller]: removed ulView, because this might change
     *@@changed V0.9.3 (2000-04-07) [umoeller]: renamed from SUBCLASSEDLISTITEM
     *@@changed V0.9.9 (2001-03-10) [umoeller]: added ulWindowWordOffset
     *@@changed V0.9.19 (2002-04-17) [umoeller]: renamed from SUBCLASSEDFOLDERVIEW
     */

    typedef struct _SUBCLFOLDERVIEW
    {
        HWND        hwndFrame;          // folder view frame window
        WPFolder    *somSelf;           // folder object
        WPObject    *pRealObject;       // "real" object; this is == somSelf
                                        // for folders, but the corresponding
                                        // disk object for WPRootFolders
        PFNWP       pfnwpOriginal;      // orig. frame wnd proc before subclassing
                                        // (WPS folder proc)
        ULONG       ulWindowWordOffset; // as passed to fdrSubclassFolderView

        HWND        hwndStatusBar,      // status bar window; NULL if there's no
                                        // status bar for this view
                    hwndCnr,            // cnr window (child of hwndFrame)
                    hwndSupplObject;    // supplementary object wnd
                                        // (fdr_fnwpSupplFolderObject)

        BYTE        bNeedCnrScroll;     // scroll container after adding status bar?
                        // changed V1.0.1 (2003-02-02) [umoeller]
                        #define SCROLL_VERYFIRSTTIME        1
                        #define SCROLL_ADJUSTFORSB          2
        BOOL        fRemoveSourceEmphasis; // flag for whether XFolder has added
                                        // container source emphasis

        ULONG       ulLastSelMenuItem;  // last selected menu item ID

        WPObject    *pSourceObject;     // object whose record core has source
                                        // emphasis;
                                        // this field is valid only between
                                        // WM_INITMENU and WM_COMMAND; if this
                                        // is NULL, the entire folder whitespace
                                        // has source emphasis
        ULONG       ulSelection;        // one of SEL_WHITESPACE, SEL_SINGLESEL,
                                        // SEL_MULTISEL, SEL_SINGLEOTHER, SEL_NONEATALL
                                        // (see wpshQuerySourceObject);
                                        // this field is valid only between
                                        // WM_INITMENU and WM_COMMAND
    } SUBCLFOLDERVIEW, *PSUBCLFOLDERVIEW;

    PSUBCLFOLDERVIEW fdrCreateSFV(HWND hwndFrame,
                                  HWND hwndCnr,
                                  ULONG ulWindowWordOffset,
                                  BOOL fCreateSuppl,
                                  WPFolder *somSelf,
                                  WPObject *pRealObject);

    PSUBCLFOLDERVIEW fdrSubclassFolderView(HWND hwndFrame,
                                           HWND hwndCnr,
                                           WPFolder *somSelf,
                                           WPObject *pRealObject);

    PSUBCLFOLDERVIEW fdrQuerySFV(HWND hwndFrame,
                                 PULONG pulIndex);

    VOID fdrManipulateNewView(WPFolder *somSelf,
                              HWND hwndNewFrame,
                              ULONG ulView);

    VOID fdrRemoveSFV(PSUBCLFOLDERVIEW psfv);

    VOID fdrCalcFrameRect(MPARAM mp1, MPARAM mp2);

    VOID fdrFormatFrame(HWND hwndFrame,
                        HWND hwndStatusBar,
                        MPARAM mp1,
                        ULONG ulCount,
                        HWND *phwndClient);

    VOID fdrManipulatePulldown(PSUBCLFOLDERVIEW psfv,
                               SHORT sMenuID,
                               HWND hwndMenuMsg);

    VOID fdrInitMenu(PSUBCLFOLDERVIEW psfv,
                     ULONG sMenuIDMsg,
                     HWND hwndMenuMsg);

    BOOL fdrMenuSelect(PSUBCLFOLDERVIEW psfv,
                       MPARAM mp1,
                       MPARAM mp2,
                       BOOL *pfDismiss);

    BOOL fdrWMCommand(PSUBCLFOLDERVIEW psfv,
                      SHORT sCommand);

    MRESULT fdrProcessFolderMsgs(HWND hwndFrame,
                                 ULONG msg,
                                 MPARAM mp1,
                                 MPARAM mp2,
                                 PSUBCLFOLDERVIEW psfv,
                                 PFNWP pfnwpOriginal);

    // supplementary object window msgs (for each
    // subclassed folder frame, xfldr.c)
    #define SOM_ACTIVATESTATUSBAR       (WM_USER+100)

    MRESULT EXPENTRY fdr_fnwpSupplFolderObject(HWND hwndObject,
                                               ULONG msg,
                                               MPARAM mp1,
                                               MPARAM mp2);

#endif

