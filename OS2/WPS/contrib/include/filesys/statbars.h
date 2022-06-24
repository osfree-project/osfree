
/*
 *@@sourcefile statbars.h:
 *      header file for the status bar translation logic (statbars.c)
 *
 *      Most declarations for statbars.c are still in common.h however.
 *
 *@@include #define INCL_WINWINDOWMGR
 *@@include #define INCL_DOSMODULEMGR
 *@@include #include <os2.h>
 *@@include #include "classes\xfldr.h"  // for common.h
 *@@include #include "shared\common.h"
 *@@include #include "shared\notebook.h" // for status bar notebook callback prototypes
 *@@include #include "filesys\folder.h"
 *@@include #include "filesys\statbars.h"
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

#ifndef STATBARS_HEADER_INCLUDED
    #define STATBARS_HEADER_INCLUDED

    /********************************************************************
     *
     *   Declarations
     *
     ********************************************************************/

    // status bar styles
    #define SBSTYLE_WARP3RAISED     1
    #define SBSTYLE_WARP3SUNKEN     2
    #define SBSTYLE_WARP4RECT       3
    #define SBSTYLE_WARP4MENU       4

    #define SBS_STATUSBARFONT       0x01
    #define SBS_TEXTNONESEL         0x02
    #define SBS_TEXTMULTISEL        0x04

    #define SBV_ICON                0x01
    #define SBV_TREE                0x02
    #define SBV_DETAILS             0x04
    #define SBV_SPLIT               0x08

    // max length of status bar mnemonics
    #define CCHMAXMNEMONICS         256

    // window ID of status bar
    #define ID_STATUSBAR            0x9001

    #ifdef FOLDER_HEADER_INCLUDED
        /*
         *@@ STATUSBARDATA:
         *      stored in QWL_USER wnd data to further describe
         *      a status bar and store some data
         *
         *@@changed V0.9.2 (2000-02-22) [umoeller]: added anchor block
         */

        typedef struct _STATUSBARDATA
        {
            WPFolder    *somSelf;           // the (root) folder of the status bar
            WPObject    *pRealObject;       // "real" object, only != somSelf if disk
            HWND        hwndFrame;          // our parent window
                                            // V1.0.0 (2002-08-21) [umoeller]
            HWND        hwndCnr;            // container window; normally the FID_CLIENT
                                            // of the frame, but for the split view this
                                            // will be different
                                            // V1.0.0 (2002-08-21) [umoeller]
            HAB         habStatusBar;       // status bar PM anchor block V0.9.2 (2000-02-22) [umoeller]
            ULONG       idTimer;            // update delay timer
            BOOL        fDontBroadcast;     // anti-recursion flag for presparams
            BOOL        fFolderPopulated;   // anti-recursion flag for wpPopulate
            PFNWP       pfnwpStatusBarOriginal; // original static control wnd proc
        } STATUSBARDATA, *PSTATUSBARDATA;
    #endif

    // msgs to status bar window (STBM_xxx)
    #define STBM_UPDATESTATUSBAR            (WM_USER + 110)
    #define STBM_PROHIBITBROADCASTING       (WM_USER + 111)

    /* ******************************************************************
     *
     *   Status bar window
     *
     ********************************************************************/

    BOOL stbClassCanHaveStatusBars(WPFolder *somSelf);

    BOOL stbFolderWantsStatusBars(WPFolder *somSelf);

    #ifdef COMMON_HEADER_INCLUDED
    BOOL stbViewCanHaveBars(WPFolder *somSelf,
                            HWND hwndFrame,
                            ULONG ulView,
                            XWPSETTING s);
    #endif

    BOOL stbViewHasStatusBar(WPFolder *somSelf,
                             HWND hwndFrame,
                             ULONG ulView);

    BOOL stbViewHasToolBar(WPFolder *somSelf,
                           HWND hwndFrame,
                           ULONG ulView);

    #ifdef FDRSUBCLASS_HEADER_INCLUDED
        HWND stbCreate(PSUBCLFOLDERVIEW psli2);

        HWND stbCreateBar(WPFolder *somSelf,
                          WPObject *pRealObject,
                          HWND hwndFrame,
                          HWND hwndCnr);

        VOID stbDestroy(PSUBCLFOLDERVIEW psli2);
    #endif

    BOOL _Optlink stb_UpdateCallback(WPFolder *somSelf,
                                     HWND hwndView,
                                     ULONG ulView,
                                     ULONG ulActivate);

    BOOL _Optlink stb_PostCallback(WPFolder *somSelf,
                                   HWND hwndView,
                                   ULONG ulView,
                                   ULONG msg);

    VOID stbUpdate(WPFolder *pFolder);

    /********************************************************************
     *
     *   Status bar text composition
     *
     ********************************************************************/

    #ifdef SOM_WPObject_h

        #ifndef __NOCFGSTATUSBARS__
            BOOL stbClassAddsNewMnemonics(SOMClass *pClassObject);

            BOOL stbSetClassMnemonics(SOMClass *pClassObject,
                                      PSZ pszText);
        #endif

        PSZ stbQueryClassMnemonics(SOMClass *pClassObject);


        #ifdef SOM_WPFolder_h
            PSZ stbComposeText(WPFolder* somSelf,
                               HWND hwndCnr);
        #endif
    #endif

    /* ******************************************************************
     *
     *   Tool bar creation
     *
     ********************************************************************/

    HWND stbCreateToolBar(WPFolder *pFolder,
                          HWND hwndFrame,
                          ULONG ulView,
                          PLONG plToolBarHeight);

    /* ******************************************************************
     *
     *   Notebook callbacks (notebook.c) for "Status bars" pages
     *
     ********************************************************************/

    #ifdef NOTEBOOK_HEADER_INCLUDED
        VOID XWPENTRY stbStatusBar1InitPage(PNOTEBOOKPAGE pnbp,
                                            ULONG flFlags);

        MRESULT XWPENTRY stbStatusBar1ItemChanged(PNOTEBOOKPAGE pnbp,
                                         ULONG ulItemID,
                                         USHORT usNotifyCode,
                                         ULONG ulExtra);

#ifndef __NOCFGSTATUSBARS__
        VOID XWPENTRY stbStatusBar2InitPage(PNOTEBOOKPAGE pnbp,
                                            ULONG flFlags);

        MRESULT XWPENTRY stbStatusBar2ItemChanged(PNOTEBOOKPAGE pnbp,
                                         ULONG ulItemID,
                                         USHORT usNotifyCode,
                                         ULONG ulExtra);
#endif

        VOID stbToolBar1InitPage(PNOTEBOOKPAGE pnbp,
                                 ULONG flFlags);

        MRESULT stbToolBar1ItemChanged(PNOTEBOOKPAGE pnbp,
                                       ULONG ulItemID,
                                       USHORT usNotifyCode,
                                       ULONG ulExtra);
    #endif
#endif
