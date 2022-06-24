
/*
 * hook_private.h:
 *      private declarations for the hook and the daemon.
 *      These are not seen by XFLDR.DLL.
 *
 *@@include #define INCL_WINMESSAGEMGR
 *@@include #include <os2.h>
 *@@include #include xwphook.h
 */

/*
 *      Copyright (C) 1999-2003 Ulrich M”ller.
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

#ifndef HOOK_PRIVATE_HEADER_INCLUDED
    #define HOOK_PRIVATE_HEADER_INCLUDED

    #ifdef __DEBUG__
        // #define HACKSWITCHLIST
    #endif

    /*
    #ifdef HACKSWITCHLIST
        #define WM_HACKSWITCHLIST           (WM_USER + 0x1324)
        #define MP1_HACKSWITCHLIST          ((MPARAM)0xf4134442)
        #define MP2_SUBCLASS                ((MPARAM)0xf8678c22)
        #define MP2_UNSUBCLASS              ((MPARAM)0xf22781fe)
    #endif
    */

    /* ******************************************************************
     *
     *   Declarations
     *
     ********************************************************************/

    /*
     * HK_PREACCEL:
     *      additional undocumented PM hook type,
     *      for pre-accelerator table hooks.
     *      This definition taken out of the
     *      ProgramCommander/2 source (thanks,
     *      Roman Stangl).
     */

    #define HK_PREACCEL             17

    /* ******************************************************************
     *
     *   Shared mem/semaphore IDs
     *
     ********************************************************************/

    #define SHMEM_HOTKEYS           "\\SHAREMEM\\XWORKPLC\\HOTKEYS.DAT"
    #define SHMEM_FUNCTIONKEYS      "\\SHAREMEM\\XWORKPLC\\FUNCKEYS.DAT"
                // added V0.9.3 (2000-04-20) [umoeller]

    #define IDMUTEX_ONEINSTANCE     "\\SEM32\\XWORKPLC\\ONEINST.MTX"
    #define TIMEOUT_HMTX_HOTKEYS    6000

    // timer IDs for fnwpDaemonObject
#ifndef __NOSLIDINGFOCUS__
    #define TIMERID_SLIDINGFOCUS        1
#endif
    #define TIMERID_SLIDINGMENU         2
    #define TIMERID_MONITORDRIVE        3
#ifndef __NOMOVEMENT2FEATURES__
    #define TIMERID_AUTOHIDEMOUSE       4
#endif
    #define TIMERID_AUTOSCROLL          5
#ifndef __NOMOVEMENT2FEATURES__
    #define TIMERID_MOVINGPTR           6
#endif

    // timer IDs for pager
    #define TIMERID_PGR_ACTIVECHANGED   7
    #define TIMERID_PGR_FLASH           8
    #define TIMERID_PGR_REFRESHDIRTIES  9

    // max # of NLS strings
    #define MAX_NLS                     12

    /* ******************************************************************
     *
     *   Structures
     *
     ********************************************************************/

    /*
     *@@ SCROLLDATA:
     *      substructure for storing scroll data
     *      for MB3-scroll processing. Two instances
     *      of this exist in HOOKDATA, one for the
     *      vertical, one for the horizontal scroll bar
     *      of the window which was under the mouse when
     *      MB3 was pressed down.
     *
     *@@added V0.9.2 (2000-02-25) [umoeller]
     */

    typedef struct _SCROLLDATA
    {
        HWND        hwndScrollBar;
                // actual scroll bar window or NULLHANDLE if none
        HWND        hwndScrollLastOwner;
                // WinQueryWindow(hwndScrollBar, QW_OWNER);
                // this gets recalculated with every mouse move
                // V0.9.3 (2000-04-30) [umoeller]

        // cached data used while MB3 is down; V0.9.1 (99-12-03)
        // LONG        lScrollBarSize;
                // size (height or width) of the scroll bar (in window coordinates);
                // calculated when MB3 is depressed; the size of the buttons
                // has already been subtracted
                // V0.9.2 (2000-03-23) [umoeller]: removed this from SCROLLDATA
                // because this might change while the user holds down MB3, so this
                // needs to be recalculated with every WM_MOUSEMOVE
        SHORT       sMB3InitialScreenMousePos,
                // mouse position when MB3 was depressed (in screen coordinates)
                // V0.9.3 (2000-04-30) [umoeller]: changed to screen
                    sMB3InitialThumbPosUnits,
                // scroll bar thumb position when MB3 was depressed (in scroll bar units)
                    sCurrentThumbPosUnits;
                // current scroll bar thumb position (in scroll bar units); this gets updated
                // when the mouse is moved while MB3 is depressed

        BOOL        fPostSBEndScroll;
                // this gets set to TRUE only if MB3 has been depressed and
                // the mouse has been moved and scroller messages have been
                // posted already;
                // this enforces a SB_ENDSCROLL when WM_BUTTON3UP comes
                // in later
    } SCROLLDATA, *PSCROLLDATA;

    /*
     *@@ NLSDATA:
     *      substructure for storing NLS strings used by the hook.  One
     *      instance of this exists in HOOKDATA.
     *
     *      This substructure is filled by the daemon when it receives
     *      a NLS notification from XFLDR.DLL.
     *
     *      The substructure includes its own buffer, so that the
     *      string resources are available for all processes.
     *
     *@@added V1.0.0 (2002-09-16) [lafaix]
     */

   typedef struct _NLSDATA
   {
       PSZ      apszNLSStrings[MAX_NLS];
                // direct access to daemon-specific strings; the entries
                // in apszNLSStrings points to the actual content in achBuf

                #define NLS_STICKYTOGGLE            0

       CHAR     achBuf[3072];
                // the buffer containing the actual strings, not directly
                // used except by LoadNLSResources in xwpdaemn.c
   } NLSDATA, *PNLSDATA;

    /*
     *@@ HOOKDATA:
     *      global hook data structure. Only one instance
     *      of this is in the shared data segment of
     *      XWPHOOK.DLL, and a pointer to that structure
     *      is returned to the daemon which initially loads
     *      that DLL by hookInit and then stored by the
     *      daemon. As a result, this structure is shared
     *      between the hook and the daemon, and both can
     *      access it at any time.
     *
     *      This is statically initialized to 0 when the hook
     *      DLL is loaded. hookInit will then set up most
     *      fields in here.
     *
     *      This contains setup data (the state of the
     *      hook), some data which needs to be cached,
     *      as well as the HOOKCONFIG structure which
     *      is used to configure the hook and the daemon.
     *      That sub-structure gets (re)loaded from OS2.INI
     *      upon daemon startup and when XDM_HOOKCONFIG is
     *      received by fnwpDaemonObject.
     */

    typedef struct _HOOKDATA
    {
        BOOL        fSendMsgHooked,     // send-message hook installed?
                    fLockupHooked,      // lockup hook installed?
                    fInputHooked,       // input hook installed?
                    fPreAccelHooked;    // pre-accelerator table hook installed?

        HWND        hwndDaemonObject;
                // wnd used for notification of events;
                // this is the daemon object window
                // (fnwpDaemonObject) as passed to hookInit
                // and is the same as XWPGLOBALSHARED.hwndDaemonObject.
                // This receives lots of messages from the hook for
                // further processing.

        HAB         habDaemonObject;
                // anchor block of hwndDaemonObject; cached for speed

        HMODULE     hmodDLL;
                // XWPHOOK.DLL module handle

        // damon/hook configuration data shared by daemon and the hook;
        // this gets loaded from OS2.INI
        HOOKCONFIG  HookConfig;

#ifndef __NOPAGER__
        // XPager configuration data shared by daemon and the hook;
        // this gets loaded from OS2.INI also
        PAGERCONFIG PagerConfig;

        PVOID       paEREs[MAX_STICKIES];
                // compiled regular expressions for every SF_MATCHES
                // that exists in PAGERCONFIG.aulStickyFlags;
                // WARNING: malloc'd by daemon, so this cannot be
                // used in the hook
                // V0.9.19 (2002-04-17) [umoeller]

        HWND        hwndPagerClient;
                // XPager client window, created by pgrCreatePager
        HWND        hwndPagerFrame;
                // XPager frame window, created by pgrCreatePager
        HWND        hwndPagerMoveThread;
                // XPager move thread (fnwpMoveThread)

        ULONG       cSuppressWinlistNotify;
                // if > 0, notification msgs from hook to pager
                // are disabled; this is set by pgrLockHook if
                // the pager itself is doing window positioning
                // to avoid infinite recursion
                // V0.9.19 (2002-05-07) [umoeller]
        BOOL        fProcessingWraparound;
                // TRUE while processing a pager wraparound due
                // to hitting a screen border; we do not process
                // sliding focus then
                // added V0.9.9 (2001-03-14) [lafaix]
                // renamed V0.9.19 (2002-05-07) [umoeller]

        POINTL      ptlCurrentDesktop;
                // offset of current desktop in pixels
        SIZEL       szlEachDesktopFaked;
                // size used for each virtual desktop; this is
                // the PM screen size plus a few extra pixels
                // to hide maximized window borders
#endif

        HWND        hwndPMDesktop,
                // desktop window handle (WinQueryDesktopWindow)
                    hwndWPSDesktop,
                // WPS desktop frame window (this only gets set by the daemon later!!)
                    hwndSwitchList,
                // window list handle (frame)
                    hwndSwitchListCnr;
                // window list's container
        ULONG       pidPM,
                // process ID of first PMSHELL.EXE V0.9.7 (2001-01-21) [umoeller]
                    pidWPS;
                // process ID of second PMSHELL.EXE V0.9.20 (2002-08-10) [umoeller]

        HWND        hwndLockupFrame;
                // current lockup window, if any

        // screen dimensions
        // (we use LONG's because otherwise we'd have to typecast
        // when calculating coordinates which might be off-screen;
        // V0.9.2 (2000-02-23) [umoeller])
        LONG        cxScreen,
                    cyScreen;

        // and system icon size V0.9.19 (2002-06-13) [umoeller]
        SIZEL       szlIcon;

        HWND        hwndActivatedByUs;
                // this gets set to a window which was activated by the
                // daemon with sliding focus

        // MB3 scrolling data; added V0.9.1 (99-12-03)
        HWND        hwndCurrentlyScrolling;
                // this is != NULLHANDLE if MB3 has been depressed
                // over a window with scroll bars and reset to
                // NULLHANDLE once MB3 is released. This allows us
                // to simulate capturing the mouse without actually
                // capturing it, which isn't such a good idea in
                // a hook, apparently (changed V0.9.3 (2000-04-30) [umoeller])

        SCROLLDATA  SDXHorz,
                    SDYVert;

        BOOL        bAutoScroll;
                // this is TRUE if auto scrolling has been requested.
                // Set by the hook, and used by XDM_BEGINSCROLL
                // V0.9.9 (2001-03-20) [lafaix]

#ifndef __NOMOVEMENT2FEATURES__
        // auto-hide mouse pointer; added V0.9.1 (99-12-03)
        ULONG       idAutoHideTimer;
                // if != NULL, auto-hide timer is running
        BOOL        fMousePointerHidden;
                // TRUE if mouse pointer has been hidden
        // auto-hide mouse pointer state backup; added V0.9.9 (2001-03-21) [lafaix]
        BOOL        fOldAutoHideMouse;
#endif

        // sliding menus
        HWND        hwndMenuUnderMouse;
        SHORT       sMenuItemIndexUnderMouse;
                // V0.9.12 (2001-05-24) [lafaix]: changed meaning (from identity to index)
        MPARAM      mpDelayedSlidingMenuMp1;

        // click watches V0.9.14 (2001-08-21) [umoeller]
        ULONG       cClickWatches,
        // winlist watches V0.9.19 (2002-05-28) [umoeller]
                    cWinlistWatches;

        // object hotkeys: if this flag is set, hotkeys are
        // temporarily disabled; this gets set when the
        // Hotkeys entry field on the "Icon" page gets the
        // focus, via XDM_DISABLEHOTKEYSTEMP
        // V0.9.16 (2001-12-08) [umoeller]
        BOOL        fHotkeysDisabledTemp;

        // NLS strings V1.0.0 (2002-09-16) [lafaix]
        NLSDATA     NLSData;
    } HOOKDATA, *PHOOKDATA;

    // special key for WM_MOUSEMOVE with delayed sliding menus
    #define         HT_DELAYEDSLIDINGMENU   (HT_NORMAL + 2)

    /* ******************************************************************
     *
     *   XPager definitions needed by the hook
     *
     ********************************************************************/

    #ifndef __NOPAGER__
        #define PGRM_POSITIONFRAME      (WM_USER + 301)
        #define PGRM_REFRESHCLIENT      (WM_USER + 302)
        #define PGRM_WINDOWCHANGE       (WM_USER + 303)
        #define PGRM_ICONCHANGE         (WM_USER + 304)
        #define PGRM_WRAPAROUND         (WM_USER + 305)
        #define PGRM_PAGERHOTKEY        (WM_USER + 306)
        #define PGRM_MOVEBYDELTA        (WM_USER + 307)

        #define PGRIDM_TOGGLESEPARATOR  0x7ffe
        #define PGRIDM_TOGGLEITEM       0x7fff
    #endif

    /* ******************************************************************
     *
     *   Hook DLL prototypes
     *
     ********************************************************************/

    PHOOKDATA EXPENTRY hookInit(HWND hwndDaemonObject);

    BOOL EXPENTRY hookKill(VOID);

    APIRET EXPENTRY hookSetGlobalHotkeys(PGLOBALHOTKEY pNewHotkeys,
                                         ULONG cNewHotkeys,
                                         PFUNCTIONKEY pNewFunctionKeys,
                                         ULONG cNewFunctionKeys);

    /* ******************************************************************
     *
     *   Internal prototypes
     *
     ********************************************************************/

    VOID _Optlink hookLog(PCSZ pcszSourceFile,
                          ULONG ulLine,
                          PCSZ pcszFunction,
                          PCSZ pcszFormat,
                          ...);

    VOID _Optlink HackSwitchList(BOOL fInstall);

    VOID _Optlink StopMB3Scrolling(BOOL fSuccessPostMsgs);

    VOID _Optlink WMButton_SystemMenuContext(HWND hwnd);

    BOOL _Optlink WMMouseMove_MB3Scroll(HWND hwnd);

    BOOL _Optlink HandleMB3Msgs(PQMSG pqmsg,
                                PBOOL pfRestartAutoHide);

    HWND _Optlink GetFrameWindow(HWND hwndTemp);

    BOOL _Optlink WMMouseMove(PQMSG pqmsg,
                              PBOOL pfRestartAutoHide);

    VOID _Optlink WMMouseMove_AutoHideMouse(VOID);

    BOOL _Optlink WMChar_Main(PQMSG pqmsg);

    extern HOOKDATA         G_HookData;
    extern PGLOBALHOTKEY    G_paGlobalHotkeys;
    extern ULONG            G_cGlobalHotkeys;
    extern PFUNCTIONKEY     G_paFunctionKeys;
    extern ULONG            G_cFunctionKeys;
    extern HMTX             G_hmtxGlobalHotkeys;

    extern HWND             G_hwndUnderMouse;
    extern HWND             G_hwndLastFrameUnderMouse;
    extern HWND             G_hwndLastSubframeUnderMouse;
    extern POINTS           G_ptsMousePosWin;
    extern POINTL           G_ptlMousePosDesktop;
    extern HWND             G_hwndRootMenu;
#endif



