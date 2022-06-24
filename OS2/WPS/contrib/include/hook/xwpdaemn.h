
/*
 * xwpdaemn.h:
 *      XPager and daemon declarations.
 *      These are not visible to the hook nor
 *      to XFLDR.DLL.
 *
 *      Requires xwphook.h to be included first.
 *
 *@@include #define INCL_DOSSEMAPHORES
 *@@include #include <os2.h>
 *@@include #include "hook\xwphook.h"
 *@@include #include "hook\xwpdaemn.h"
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
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

#ifndef PAGER_HEADER_INCLUDED
    #define PAGER_HEADER_INCLUDED

#ifndef __NOPAGER__

    /* ******************************************************************
     *
     *   Pager interface
     *
     ********************************************************************/

    VOID dmnKillXPager(BOOL fNotifyKernel);

    BOOL dmnLoadPagerSettings(ULONG flConfig);

    /* ******************************************************************
     *
     *   Pager definitions
     *
     ********************************************************************/

    #define TIMEOUT_HMTX_WINLIST    20*1000
                // raised V0.9.12 (2001-05-31) [umoeller]

    // move hotkey flags
    #define MOVE_LEFT           1
    #define MOVE_RIGHT          2
    #define MOVE_UP             4
    #define MOVE_DOWN           8

    // font ID to use for the XPager window titles;
    // we always use the same one, because there's only one
    // in the daemon process
    #define LCID_PAGER_FONT  ((ULONG)1)

    #ifdef DEBUG_WINDOWLIST
        typedef struct WINLISTRECORD
        {
            RECORDCORE      recc;

            PSZ             pszHWND;
            CHAR            szHWND[20];

            PSZ             pszHWNDParent;
            CHAR            szHWNDParent[20];

            PSZ             pszFlags;
            CHAR            szFlags[100];

            PSZ             pszPos;
            CHAR            szPos[30];

            PSZ             pszSwtitle;

            PSZ             pszWindowType;

            PSZ             pszClass;
            CHAR            szClass[50];

            PSZ             pszPID;
            CHAR            szPID[20];

            PSZ             pszStyle;
            CHAR            szStyle[300];

            PVOID           pWinInfo;       // reverse linkage

        } WINLISTRECORD, *PWINLISTRECORD;
    #endif

    /*
     *@@ XWINDATA:
     *      variable data collected by pgrGetWinData.
     *      This struct is also part of XWININFO.
     *
     *@@added V0.9.19 (2002-06-18) [umoeller]
     */

    typedef struct _XWINDATA
    {
        HSWITCH     hsw;                // switch entry or NULLHANDLE

        SWCNTRL     swctl;              // switch list entry
                        // HWND hwnd
                        // HWND hwndIcon
                        // HPROGRAM hprog
                        // PID idProcess
                        // ULONG idSession
                        // ULONG uchVisibility
                        // ULONG fbJump
                        // CHAR szSwTitle[MAXNAMEL+4]
                        // ULONG bProgType


        CHAR        szClassName[30];
        ULONG       tid;

        SWP         swp;                // last known window pos

        BYTE        bWindowType;
            #define WINDOW_NIL          1   // "not in list" == not in switch list;
                                            // may still be minimized, maximized,
                                            // or hidden, we don't care then

            // the following styles are treated as special
            // and are left alone by the pager (always sticky)
            #define WINDOW_XWPDAEMON    2   // probably XPager or scroll window,
                                            // ignore (sticky)
            #define WINDOW_WPSDESKTOP   3   // WPS desktop, always sticky

            #define WINDOW_STICKY       4   // window is on sticky list
            #define WINDOW_MINIMIZE     5   // window is minimized, treat as sticky
            #define WINDOW_HIDDEN       6

            // the following types are treated as "normal"
            // windows and moved around by the pager
            #define WINDOW_MAXIMIZE     7   // window is maximized
            #define WINDOW_NORMAL       8

    } XWINDATA, *PXWINDATA;

    /*
     *@@ XWININFO:
     *      one of these exists for every window
     *      that is currently on the daemon window
     *      list. See pg_winlist.c.
     *
     *@@added V0.9.7 (2001-01-21) [umoeller]
     *@@changed V0.9.19 (2002-06-18) [umoeller]: largely reworked for new winlist
     */

    typedef struct _XWININFO
    {
        XWINDATA    data;

        HPOINTER    hptrFrame;          // frame icon (WM_QUERYICON)
        ULONG       flFlags;            // persistent flags for this entry
            #define WLF_ICONCRASHED     0x0001
                        // sometimes we get inexplicable crashes in the
                        // daemon in bitblt... if the exception handler
                        // there catches one of those, it sets this
                        // bit to make sure we won't try again
        #ifdef DEBUG_WINDOWLIST
            PWINLISTRECORD prec;
        #endif

    } XWININFO, *PXWININFO;

    /*
     *@@ XWINTRANSIENT:
     *      one of these exists for every transient sticky window.
     *
     *@@added V1.0.0 (2002-08-13) [lafaix]
     */

    typedef struct _XWINTRANSIENT
    {
        HWND hwnd;                      // transient window handle
        BOOL bSticky;                   // forced stickyness
    } XWINTRANSIENT, *PXWINTRANSIENT;

    /* ******************************************************************
     *
     *   Pager window list
     *
     ********************************************************************/

    APIRET pgrInit(VOID);

    BOOL pgrLockWinlist(VOID);

    VOID pgrUnlockWinlist(VOID);

    PXWININFO pgrFindWinInfo(HWND hwndThis,
                             PVOID *ppListNode);

    BOOL pgrGetWinData(PXWINDATA pWinData, BOOL fQuickCheck);

    BOOL pgrCreateWinInfo(HWND hwnd);

    VOID pgrBuildWinlist(VOID);

    VOID pgrFreeWinInfo(HWND hwnd);

    BOOL pgrRefresh(HWND hwnd);

    BOOL pgrIsSticky(HWND hwnd,
                     PCSZ pcszSwtitle);

    BOOL pgrIconChange(HWND hwnd,
                       HPOINTER hptr);

    #ifdef INCL_WINSWITCHLIST
    PSWBLOCK pgrQueryWinList(ULONG pid);
    #endif

    #ifdef THREADS_HEADER_INCLUDED
        VOID _Optlink fntWinlistThread(PTHREADINFO pti);
    #endif

    /* ******************************************************************
     *
     *   Pager control window
     *
     ********************************************************************/

    BOOL pgrLockHook(PCSZ pcszFile, ULONG ulLine, PCSZ pcszFunction);

    VOID pgrUnlockHook(VOID);

    LONG pgrCalcClientCY(LONG cx);

    BOOL pgrIsShowing(PSWP pswp);

    VOID pgrRecoverWindows(HAB hab,
                           BOOL fWPSOnly);

    BOOL pgrCreatePager(VOID);

    /* ******************************************************************
     *
     *   Pager window movement
     *
     ********************************************************************/

    #ifdef THREADS_HEADER_INCLUDED
        VOID _Optlink fntMoveThread(PTHREADINFO pti);
    #endif

    BOOL pgrMakeWindowVisible(HWND hwnd);

    BOOL pgrSwitchToDesktop(HWND hwnd, BOOL fMove, BOOL fFlashToTop);

    BOOL pgrToggleTransientSticky(HWND hwnd);

    BOOL pgrIsWindowTransientSticky(HWND hwnd);
#endif

    /* ******************************************************************
     *
     *   Drive monitors
     *
     ********************************************************************/

    BOOL dmnAddDiskfreeMonitor(ULONG ulLogicalDrive,
                               HWND hwndNotify,
                               ULONG ulMessage);

    #ifdef THREADS_HEADER_INCLUDED
        void _Optlink fntDiskWatch(PTHREADINFO ptiMyself);
    #endif

    BOOL dmnQueryDisks(ULONG ulLogicalDrive,
                       MPARAM mpDiskInfos);

    /* ******************************************************************
     *
     *   Watchdog
     *
     ********************************************************************/

    VOID dmnStartWatchdog(VOID);

    /* ******************************************************************
     *
     *   Global variables in xwpdaemn.c
     *
     ********************************************************************/

    #ifdef HOOK_PRIVATE_HEADER_INCLUDED
        extern PHOOKDATA    G_pHookData;
    #endif

    extern PXWPGLOBALSHARED G_pXwpGlobalShared;

    extern HPOINTER     G_hptrDaemon;

    #ifdef LINKLIST_HEADER_INCLUDED
        extern LINKLIST     G_llWinInfos;
    #endif

#endif

