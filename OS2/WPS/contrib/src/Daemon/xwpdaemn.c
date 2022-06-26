
/*
 *@@sourcefile xwpdaemn.c:
 *      this has the code for the XWorkplace Daemon process
 *      (XWPDAEMN.EXE). This is an invisible PM process which
 *      does not appear in the window list, but is only seen
 *      by the XWorkplace main DLL (XFLDR.DLL).
 *
 *      The daemon is started automatically by XFLDR.DLL upon
 *      first Desktop startup (details follow). Currently, the
 *      daemon is responsible for the following:
 *
 *      1.  Installation and configuration of the XWorkplace
 *          PM hook (XWPHOOK.DLL, xwphook.c).
 *
 *          Since XWPHOOK.DLL may only be interfaced by one
 *          process, it is the daemon's responsibility to
 *          configure the hook. When any data changes which
 *          is of importance to the hook, XFLDR.DLL writes
 *          that data to OS2.INI and then posts a message to
 *          the daemon's object window (fnwpDaemonObject).
 *          The daemon then reads the new data and notifies
 *          the hook. See xwphook.c for details.
 *
 *          Note: If you wish to configure the hook or daemon,
 *          NEVER call the hook or the daemon directly.
 *          I have created proper interfaces for everything
 *          you can think of in config/hookintf.c, which
 *          also has the new notebook pages in the "Mouse" and
 *          "Keyboard" objects.
 *
 *      2.  Keeping track of Desktop restarts, since the daemon
 *          keeps running even while the Desktop restarts. Shared
 *          memory is used for communication.
 *
 *          The interaction between XWPDAEMN.EXE and XFLDR.DLL works
 *          as follows:
 *
 *          a) After system startup, PM starts the program specified
 *             in RUNWORKPLACE in CONFIG.SYS, which should be the
 *             WPS (PMSHELL.EXE).
 *
 *          b) When the WPS is started for the first time (after
 *             a reboot), initMain (XFLDR.DLL) checks for whether
 *             an XWPGLOBALSHARED structure (xwphook.h) has already
 *             been allocated as a block of shared memory.
 *
 *             At system startup, this is not the case. As a result,
 *             initMain allocates that block and then starts
 *             XWPDAEMN.EXE using WinStartApp.
 *
 *          c) The daemon then requests access to that block of
 *             shared memory so that the usage count of that
 *             block is then 2. It then creates the daemon object
 *             window (fnwpDaemonObject) and posts T1M_DAEMONREADY
 *             to krn_fnwpThread1Object in XFLDR.DLL.
 *
 *             krn_fnwpThread1Object will then send XDM_HOOKINSTALL
 *             to the daemon object window to install the hook (if
 *             the hook is enabled in XWPSetup). If XPager has
 *             been enabled also, XDM_STARTSTOPPAGER will also
 *             be sent to start XPager (see below).
 *
 *             After the Desktop has been opened, XFLDR.DLL will
 *             post XDM_DESKTOPREADY to the Daemon because both
 *             the hook and XPager will need the window handle
 *             of the currently active Desktop.
 *
 *             This situation persists while the WPS is running
 *             (unless the hook is explicitly disabled by the
 *             user or the daemon gets killed for some reason).
 *             If any hook configuration data changes, e.g. via
 *             the WPS Keyboard, Mouse, or Screen settings
 *             objects, XFLDR.DLL notifies the daemon by posting
 *             messages to the daemon's object window
 *             (fnwpDaemonObject), which in turn notifies the hook.
 *
 *          d) If the WPS is restarted for any reason (because
 *             of a crash or explicitly by the user), the daemon
 *             (and thus the hook) is not terminated, but keeps
 *             running. Since OS/2 maintains a reference count
 *             for shared memory, the XWPGLOBALSHARED structure
 *             is not freed, because the daemon is still using
 *             it (its usage count is now 1).
 *
 *          e) When the WPS is re-initializing, initMain
 *             this time realizes that the XWPGLOBALSHARED structure
 *             exists already and will thus find out that the WPS is
 *             not started for the first time. Instead, access to
 *             the existing XWPGLOBALSHARED structure is requested
 *             (so we have a reference count of 2 again).
 *
 *             XFLDR.DLL then posts the new Desktop window handle with
 *             XDM_DESKTOPREADY so the daemon data can be updated.
 *
 *             After the Desktop is up, if XWPGLOBALSHARED.fProcessStartupFolder
 *             is TRUE (which has been set to this by XShutdown if the
 *             respective checkbox was checked), the XWorkplace
 *             startup folder is processed again.
 *
 *          f) If the NLS DLL is changed, or when it is first loaded,
 *             XFLDR.DLL fills the achNLSStrings array in the XWPGLOBALSHARED
 *             structure with the daemon-specific NLS strings, and then
 *             posts a XDM_NLSCHANGED message to the daemon object window.
 *
 *             [As of this writing, there is only support for at least
 *              12 specific strings (the shared buffer is 3072 bytes long,
 *              i.e., it can only contains up to 12 256-bytes-long strings).
 *              Should the need arises, the XDM_NLSCHANGED message would
 *              have to be updated, to for example proceeds incrementaly.]
 *
 *          In short, at any time after the WPS is starting for
 *          the first time, at least one process is using the
 *          XWPGLOBALSHARED structure: either the WPS process (with
 *          XFLDR.DLL) or XWPDAEMON.EXE or both. The structure
 *          only gets freed if both processes are killed, which
 *          is not probable under normal circumstances.
 *
 *          This new approach has several advantages:
 *
 *          a)  The XWorkplace startup folder finally works all the
 *              time, which was not the case with versions < 0.9.0.
 *
 *          b)  The hook does not get re-installed at every WPS
 *              restart, which saves PM resources. Apparently, system
 *              hooks can only be installed a limited number of times
 *              before the system hangs (I get around 10-15 tries).
 *              I have added a workaround to the daemon to properly
 *              deregister the hook on process termination so I get
 *              around 40 tries now, but this still looks like a PM
 *              bug to me.
 *
 *          c)  While programming on the hook, I can simply restart
 *              the daemon to have the hook reloaded, instead of
 *              having to restart the WPS all the time.
 *
 *      3.  The XPager window for virtual desktops. See pg_control.c
 *          for details.
 *
 *      4.  Mouse helpers.  Sliding focus, pointer hidding and
 *          MB3 (auto) scrolling use fnwpDaemonObject.
 *
 *      5.  Drive monitors, which are now used by the diskfree
 *          widget that was added with V0.9.14 to avoid the
 *          "Drive not ready" popups in PMSHELL.EXE.
 *          This is configured by sending messages to
 *          fnwpDaemonObject; see remarks there.
 *
 *      Both the hook and the daemon are all new with V0.9.0.
 *
 *@@added V0.9.0 [umoeller]
 *@@header "hook\xwphook.h"
 *@@header "hook\xwpdaemn.h"
 */

/*
 *      Copyright (C) 1999-2008 Ulrich M”ller.
 *      Copyright (C) 1993-1999 Roman Stangl.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 */

#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSDEVICES
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINPOINTERS
#define INCL_WINMENUS
#define INCL_WINTIMER
#define INCL_WINMESSAGEMGR
#define INCL_WINSCROLLBARS
#define INCL_WININPUT
#define INCL_WINSHELLDATA
#define INCL_WINPROGRAMLIST
#define INCL_WINSYS
#define INCL_WINWORKPLACE
#define INCL_WINSWITCHLIST
#define INCL_WINSTDCNR
#define INCL_WINERRORS
#define INCL_SHLERRORS

#define INCL_GPIBITMAPS
#define INCL_GPIPRIMITIVES
#define INCL_GPILOGCOLORTABLE
#include <os2.h>

#include <stdio.h>
#include <time.h>
#include <setjmp.h>

#define DONT_REPLACE_MALLOC         // in case mem debug is enabled
#define DONT_REPLACE_FOR_DBCS
#include "setup.h"                      // code generation and debugging options

#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\gpih.h"
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\regexp.h"             // extended regular expressions
#include "helpers\shapewin.h"
#include "helpers\standards.h"
#include "helpers\threads.h"

#include "xwpapi.h"                     // public XWorkplace definitions

#include "hook\xwphook.h"               // hook and daemon definitions
#include "hook\hook_private.h"          // private hook and daemon definitions
#include "hook\xwpdaemn.h"              // XPager and daemon declarations

#include "bldlevel.h"

#include "shared\kernel.h"              // XWorkplace Kernel

#pragma hdrstop

/* ******************************************************************
 *
 *   Private declarations
 *
 ********************************************************************/

#define VERT      1
#define HORZ      2
#define ALL       3  // VERT | HORZ

#define NORTH     0
#define NORTHEAST 1
#define EAST      2
#define SOUTHEAST 3
#define SOUTH     4
#define SOUTHWEST 5
#define WEST      6
#define NORTHWEST 7
#define CENTER    8

/*
 *@@ NOTIFYCLIENT:
 *
 *@@added V0.9.14 (2001-08-21) [umoeller]
 */

typedef struct _NOTIFYCLIENT
{
    HWND            hwndNotify;
    ULONG           ulMessage;
} NOTIFYCLIENT, *PNOTIFYCLIENT;

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

HAB         G_habDaemon = NULLHANDLE;
        // anchor block of thread 1
HMQ         G_hmqDaemon = NULLHANDLE;
        // HMQ of thread 1
USHORT      G_pidDaemon = NULLHANDLE;
        // process ID of XWPDAEMN.EXE V0.9.19 (2002-05-07) [umoeller]


// pointer to hook data in hook dll;
// this includes HOOKCONFIG
PHOOKDATA       G_pHookData = NULL;

// pointer to shared-memory daemon globals
PXWPGLOBALSHARED   G_pXwpGlobalShared = NULL;

// daemon icon from resources
HPOINTER        G_hptrDaemon = NULLHANDLE;

// sliding focus data
static ULONG    G_ulSlidingFocusTimer = 0;    // timer ID for delayed sliding focus
// window to be activated
static HWND     G_hwndSlidingUnderMouse = NULLHANDLE;
static HWND     G_hwndSliding2Activate = NULLHANDLE;

// sliding menu data
ULONG           G_ulSlidingMenuTimer = 0;
MPARAM          G_SlidingMenuMp1Saved = 0;
            // MP1 of WM_MOUSEMOVE posted back to hook; added for checking

// click watches
LINKLIST        G_llClickWatches;           // linked list of NOTIFYCLIENT structs
                                            // for click watches, auto-free
LINKLIST        G_llWinlistWatches;         // linked list of NOTIFYCLIENT structs
                                            // for winlist watches, auto-free

// auto-move ptr data
ULONG           G_ulMovingPtrTimer = 0;
ULONG           G_ulMSMovingPtrStart = 0;
POINTL          G_ptlMovingPtrStart,
                G_ptlMovingPtrNow,
                G_ptlMovingPtrEnd;
LONG            G_lLastMovingPtrRadius = 0;

const char *WNDCLASS_DAEMONOBJECT = "XWPDaemonObject";

// MB3 (auto) scroll data
HPOINTER        G_hptrOld = NULLHANDLE,
                G_hptrNESW = NULLHANDLE,
                G_hptrNS = NULLHANDLE,
                G_hptrEW = NULLHANDLE,
                G_hptrN = NULLHANDLE,
                G_hptrNE = NULLHANDLE,
                G_hptrE = NULLHANDLE,
                G_hptrSE = NULLHANDLE,
                G_hptrS = NULLHANDLE,
                G_hptrSW = NULLHANDLE,
                G_hptrW = NULLHANDLE,
                G_hptrNW = NULLHANDLE;

BOOL            G_fScrollOriginLoaded = FALSE;

SHAPEFRAME      G_sfScrollOrigin = {0};
POINTL          G_ptlScrollOrigin = {0},
                G_ptlScrollCurrent = {0};
LONG            G_lScrollMode = ALL;
HPOINTER        G_ahptrPointers[9] = {0};
ULONG           G_ulAutoScrollTick = 0;

THREADINFO      G_tiWinlistThread = {0};

/* ******************************************************************
 *
 *   Exception hooks (except.c)
 *
 ********************************************************************/

/*
 *@@ dmnExceptOpenLogFile:
 *      hook function for the exception handlers (except.c).
 *
 *@@added V0.9.4 (2000-07-11) [umoeller]
 */

FILE* _System dmnExceptOpenLogFile(VOID)
{
    CHAR        szFileName[CCHMAXPATH];
    FILE        *file;
    DATETIME    DT;

    sprintf(szFileName, "%c:\\%s", doshQueryBootDrive(), XFOLDER_DMNCRASHLOG);
    if (file = fopen(szFileName, "a"))
    {
        DosGetDateTime(&DT);
        fprintf(file, "\nXWorkplace Daemon trap message -- Date: %04d-%02d-%02d, Time: %02d:%02d:%02d\n",
            DT.year, DT.month, DT.day,
            DT.hours, DT.minutes, DT.seconds);
#define LOGFILENAME XFOLDER_DMNCRASHLOG
        fprintf(file, "------------------------------------------------------------------\n"
                      "\nAn internal error occurred in the XWorkplace Daemon (XWPDAEMN.EXE).\n"
                      "Please send a bug report to " CONTACT_ADDRESS_USER "\n"
                      "so that this error may be fixed for future XWorkplace versions.\n"
                      "Please supply this file (?:\\" LOGFILENAME ") with your e-mail\n"
                      "and describe as exactly as possible the conditions under which\n"
                      "the error occurred.\n"
                      "\nRunning XWorkplace version: " BLDLEVEL_VERSION " built " __DATE__ "\n");

    }

    return file;
}

/*
 *@@ dmnExceptExplain:
 *      hook function for the exception handlers (except.c).
 *
 *@@added V0.9.4 (2000-07-11) [umoeller]
 */

VOID _System dmnExceptExplain(FILE *file,      // in: logfile from fopen()
                              PTIB ptib,       // in: thread info block
                              ULONG ulpri)     // in: thread priority
{
    // nothing so far
}

/*
 *@@ dmnExceptError:
 *      hook function for the exception handlers (except.c).
 *
 *@@added V0.9.4 (2000-07-11) [umoeller]
 */

VOID APIENTRY dmnExceptError(const char *pcszFile,
                             ULONG ulLine,
                             const char *pcszFunction,
                             APIRET arc)     // in: DosSetExceptionHandler error code
{
    CHAR        szFileName[CCHMAXPATH];
    FILE        *file;
    DATETIME    DT;

    sprintf(szFileName, "%c:\\%s", doshQueryBootDrive(), XFOLDER_DMNCRASHLOG);
    if (file = fopen(szFileName, "a"))
    {
        DosGetDateTime(&DT);
        fprintf(file, "\nXWorkplace Daemon trap message -- Date: %04d-%02d-%02d, Time: %02d:%02d:%02d\n",
            DT.year, DT.month, DT.day,
            DT.hours, DT.minutes, DT.seconds);

        fprintf(file,
                "%s line %d (%s): TRY_* macro failed to install exception handler (APIRET %d)",
                pcszFile, ulLine, pcszFunction,
                arc);
        fclose(file);
    }
}

/* ******************************************************************
 *
 *   Hook interface
 *
 ********************************************************************/

/*
 *@@ dmnStartXPager:
 *      starts XPager by calling pgrBuildWinlist
 *      and pgrCreatePager and starting
 *      fntMoveThread.
 *
 *      This gets called when XDM_STARTSTOPPAGER is
 *      received by fnwpDaemonObject.
 *
 *      Call dmnKillXPager to stop XPager again.
 *
 *      This function does not affect the operation of
 *      the daemon in general; it is only the XPager
 *      which is affected. However, the hooks will
 *      notice this and process more messages.
 *
 *@@added V0.9.2 (2000-02-22) [umoeller]
 *@@changed V0.9.3 (2000-05-21) [umoeller]: fixed startup problems, added new thread flags
 */

BOOL dmnStartXPager(VOID)
{
    BOOL brc = FALSE;

#ifndef __NOPAGER__

    if (G_pHookData)
        if (    (G_pHookData->fInputHooked)
             && (G_pHookData->fPreAccelHooked)
           )
        {
            brc = pgrCreatePager();
               // this checks for whether the pager already
               // exists; otherwise it sets the global window
               // handles in HOOKDATA;
               // the hook sees this and will start processing
               // XPager messages
        }
#endif

    return brc;
}

#ifndef __NOPAGER__

/*
 *@@ dmnKillXPager:
 *      destroys the XPager control window and
 *      stops the additional XPager threads.
 *      This is the reverse to dmnStartXPager and
 *      calls pgrRecoverWindows in turn.
 *
 *      This gets called when XDM_STARTSTOPPAGER is
 *      received by fnwpDaemonObject.
 *
 *      This function does not affect the operation of
 *      the daemon in general; it is only the XPager
 *      which is affected. However, the hooks will
 *      notice this and process fewer messages.
 *
 *@@added V0.9.2 (2000-02-22) [umoeller]
 *@@changed V0.9.3 (2000-04-25) [umoeller]: adjusted for new T1M_PAGERCLOSED
 *@@changed V0.9.9 (2001-04-05) [pr]: fixed trap
 */

VOID dmnKillXPager(BOOL fNotifyKernel)    // in: if TRUE, we post T1M_PAGERCLOSED (TRUE) to the kernel
{
    if (    G_pHookData
         && G_pHookData->hwndPagerFrame
         && G_pHookData->hwndPagerMoveThread
       )
    {
        // XPager running:
        // ULONG   ulRequest;
        // save page mage frame
        HWND    hwndPagerFrame = G_pHookData->hwndPagerFrame;

        // stop move thread
        WinPostMsg(G_pHookData->hwndPagerMoveThread, WM_QUIT, 0, 0);

        pgrRecoverWindows(G_habDaemon,
                          FALSE);           // not WPS only

        // set global window handles to NULLHANDLE;
        // the hook sees this and will stop processing
        // XPager messages
        G_pHookData->hwndPagerClient = NULLHANDLE;
        G_pHookData->hwndPagerFrame = NULLHANDLE;

        // then destroy the XPager window
        WinDestroyWindow(hwndPagerFrame);

        // notify kernel that XPager has been closed
        // so that the global settings can be updated
        WinPostMsg(G_pXwpGlobalShared->hwndThread1Object,
                   T1M_PAGERCLOSED,
                   (MPARAM)fNotifyKernel,       // if TRUE, XPager will be disabled
                   0);
    }
}

#endif

/*
 *@@ LoadHotkeysForHook:
 *      this loads the global object hotkeys from
 *      OS2.INI and passes them to the hook by
 *      calling hookSetGlobalHotkeys, which will
 *      then allocate shared memory for the hotkeys.
 *
 *      This gets called from main() when the daemon
 *      is (re)started and from fnwpDaemonObject
 *      when the hotkeys have changed.
 *
 *@@changed V0.9.3 (2000-04-20) [umoeller]: added function keys support
 */

VOID LoadHotkeysForHook(VOID)
{
    PGLOBALHOTKEY pHotkeys = NULL;
    ULONG cHotkeys = 0;
    PFUNCTIONKEY pFunctionKeys = NULL;
    ULONG cFunctionKeys = 0;

    ULONG   ulSizeOfData = 0;

    // _Pmpf(("LoadHotkeysForHook, pHookData: 0x%lX", G_pHookData));

    // 1) hotkeys

    // get size of data for pszApp/pszKey
    if (PrfQueryProfileSize(HINI_USER,
                            INIAPP_XWPHOOK,
                            INIKEY_HOOK_HOTKEYS,
                            &ulSizeOfData))
    {
        pHotkeys = (PGLOBALHOTKEY)malloc(ulSizeOfData);
        if (PrfQueryProfileData(HINI_USER,
                                INIAPP_XWPHOOK,
                                INIKEY_HOOK_HOTKEYS,
                                pHotkeys,
                                &ulSizeOfData))
            // hotkeys successfully loaded:
            // calc no. of items in array
            cHotkeys = ulSizeOfData / sizeof(GLOBALHOTKEY);
    }

    // 2) function keys

    // get size of data for pszApp/pszKey
    ulSizeOfData = 0;
    if (PrfQueryProfileSize(HINI_USER,
                            INIAPP_XWPHOOK,
                            INIKEY_HOOK_FUNCTIONKEYS,
                            &ulSizeOfData))
    {
        pFunctionKeys = (PFUNCTIONKEY)malloc(ulSizeOfData);
        if (PrfQueryProfileData(HINI_USER,
                                INIAPP_XWPHOOK,
                                INIKEY_HOOK_FUNCTIONKEYS,
                                pFunctionKeys,
                                &ulSizeOfData))
        {
            // hotkeys successfully loaded:
            // calc no. of items in array
            cFunctionKeys = ulSizeOfData / sizeof(FUNCTIONKEY);
            // _Pmpf(("  Got %d function keys", cFunctionKeys));
            // _Pmpf(("  function key %d is 0x%lX", 1, pFunctionKeys[1].ucScanCode));
        }
    }

    // 3) notify hook
    hookSetGlobalHotkeys(pHotkeys,
                         cHotkeys,
                         pFunctionKeys,
                         cFunctionKeys);

    // we can safely free the hotkeys, because
    // the hook has copied them to shared memory
    if (pHotkeys)
        free(pHotkeys);
    if (pFunctionKeys)
        free(pFunctionKeys);
}

/*
 *@@ dmnLoadPagerSettings:
 *      loads the XPager settings from OS2.INI,
 *      if present. Depending on flConfig, XPager
 *      will reformat and/or re-adjust itself.
 *
 *@@added V0.9.3 (2000-04-09) [umoeller]
 *@@changed V0.9.4 (2000-07-10) [umoeller]: added PGRCFG_STICKIES
 *@@changed V0.9.6 (2000-11-06) [umoeller]: fixed startup desktop to upper left
 *@@changed V0.9.9 (2001-03-15) [lafaix]: relaxed startup desktop position
 */

BOOL dmnLoadPagerSettings(ULONG flConfig)
{
    ULONG   cb;

    cb = sizeof(PAGERCONFIG);
    if (PrfQueryProfileData(HINI_USER,
                            INIAPP_XWPHOOK,
                            INIKEY_HOOK_PAGERCONFIG,
                            &G_pHookData->PagerConfig,
                            &cb))
    {
        // success:

        // always set start desktop within limits V0.9.9 (2001-03-15) [lafaix]
        if (G_pHookData->PagerConfig.bStartX > G_pHookData->PagerConfig.cDesktopsX)
            G_pHookData->PagerConfig.bStartX = G_pHookData->PagerConfig.cDesktopsX;
        if (G_pHookData->PagerConfig.bStartY > G_pHookData->PagerConfig.cDesktopsY)
            G_pHookData->PagerConfig.bStartY = G_pHookData->PagerConfig.cDesktopsY;

        if (    (G_pHookData->hwndPagerClient)
             && (G_pHookData->hwndPagerFrame)
           )
        {
            if (flConfig & PGRCFG_STICKIES)
            {
                // stickies changed:
                if (pgrLockWinlist())
                {
                    // kill all regexps that were compiled
                    // V0.9.19 (2002-04-17) [umoeller]
                    ULONG ul;
                    for (ul = 0;
                         ul < MAX_STICKIES;
                         ul++)
                    {
                        if (G_pHookData->paEREs[ul])
                        {
                            rxpFree(G_pHookData->paEREs[ul]);
                            G_pHookData->paEREs[ul] = NULL;
                        }
                    }

                    pgrUnlockWinlist();
                }
                pgrBuildWinlist();
            }

            if (flConfig & PGRCFG_REFORMAT)
                WinSendMsg(G_pHookData->hwndPagerClient,
                           PGRM_POSITIONFRAME,
                           0,
                           0);

            if (flConfig & PGRCFG_REPAINT)
                WinPostMsg(G_pHookData->hwndPagerClient,
                           PGRM_REFRESHCLIENT,
                           (MPARAM)TRUE,    // nuke template bitmap to get colors right
                           0);
        }

        return TRUE;
    }

    return FALSE;
}

/*
 *@@ LoadHookConfig:
 *      this (re)loads the configuration data
 *      from OS2.INI directly into the shared
 *      global data segment of XWPHOOK.DLL.
 *
 *      This has all the configuration except
 *      for the object hotkeys.
 *
 *      This gets called from main() when the daemon
 *      is (re)started and from fnwpDaemonObject
 *      upon receiving XDM_HOOKCONFIG when the
 *      config data has changed.
 */

BOOL LoadHookConfig(BOOL fHook,         // in: reload hook settings
                    BOOL fXPager)       // in: reload XPager settings
{
    BOOL brc = FALSE;

    // _Pmpf(("XWPDAEMON: LoadHookConfig, pHookData: 0x%lX", G_pHookData));
    // _Pmpf(("    fHook: %d", fHook));
    // _Pmpf(("    fXPager: %d", fXPager));

    if (G_pHookData)
    {
        ULONG cb =  sizeof(HOOKCONFIG);

        if (fHook)
        {
            // safe defaults
            memset(&G_pHookData->HookConfig, 0, sizeof(HOOKCONFIG));
            // overwrite from INI, if found
            brc = PrfQueryProfileData(HINI_USER,
                                      INIAPP_XWPHOOK,
                                      INIKEY_HOOK_CONFIG,
                                      &G_pHookData->HookConfig,
                                      &cb);
        }

#ifndef __NOPAGER__
        if (fXPager)
        {
            // safe defaults
            // shortcuts to global config
            PPAGERCONFIG pPagerCfg = &G_pHookData->PagerConfig;

            // first, set the fake desktop size... we no longer move windows
            // by the size of the PM screen (e.g. 1024x768), but instead we
            // use eight extra pixels, so we can get rid of the borders of
            // maximized windows.

            G_pHookData->szlEachDesktopFaked.cx = G_pHookData->cxScreen + 8;
            G_pHookData->szlEachDesktopFaked.cy = G_pHookData->cyScreen + 8;

            pPagerCfg->cDesktopsX = 3;
            pPagerCfg->cDesktopsY = 2;
            pPagerCfg->bStartX = 1;
            pPagerCfg->bStartY = 1;
            pPagerCfg->flPager = PGRFL_PAGE1_DEFAULTS | PGRFL_PAGE2_DEFAULTS;
            pPagerCfg->ulFlashDelay = 2000;

            // stickies
            memset(pPagerCfg->aszStickies,
                   0,
                   sizeof(pPagerCfg->aszStickies));
            pPagerCfg->cStickies = 0;
            memset(pPagerCfg->aulStickyFlags,
                   0,
                   sizeof(pPagerCfg->aulStickyFlags));

            pPagerCfg->flPaintMode = PMOD_TOPBOTTOM;
            pPagerCfg->lcolDesktop1 = RGBCOL_DARKBLUE;
            pPagerCfg->lcolDesktop2 = RGBCOL_BLACK;
            pPagerCfg->lcolActiveDesktop = RGBCOL_BLUE;
            pPagerCfg->lcolGrid = RGBCOL_GRAY;
            pPagerCfg->lcolInactiveWindow = RGBCOL_WHITE;
            pPagerCfg->lcolActiveWindow = RGBCOL_GREEN;
            pPagerCfg->lcolWindowFrame = RGBCOL_BLACK;
            pPagerCfg->lcolInactiveText = RGBCOL_BLACK;
            pPagerCfg->lcolActiveText = RGBCOL_BLACK;

            pPagerCfg->flKeyShift = KC_CTRL | KC_ALT;

            // overwrite from INI, if found
            dmnLoadPagerSettings(0);

            // in any case, write them back to OS2.INI because
            // otherwise XWPScreen doesn't work right
            PrfWriteProfileData(HINI_USER,
                                INIAPP_XWPHOOK,
                                INIKEY_HOOK_PAGERCONFIG,
                                &G_pHookData->PagerConfig,
                                sizeof(PAGERCONFIG));
        }
#endif
    }

    return brc;
}

/*
 *@@ LoadNLSResources:
 *      this queries the current NLS resources (menu item titles for
 *      examples) and copies them to the NLSDATA structure in the
 *      shared HOOKDATA area.
 *
 *      G_pXwpGlobalShared->achNLSStrings contains a 0-terminated list
 *      of strings.
 *
 *@@added V1.0.0 (2002-09-15) [lafaix]
 */

BOOL LoadNLSResources(VOID)
{
    _PmpfF(("entering, G_pXwpGlobalShared 0x%lX, G_pHookData 0x%lX"));
    if (    (G_pXwpGlobalShared)
         && (G_pHookData)
       )
    {
        USHORT us, usLen;
        PNLSDATA pNLSData = &G_pHookData->NLSData;
        PSZ pszSrc = G_pXwpGlobalShared->achNLSStrings;
        PSZ pszDest = pNLSData->achBuf;

        _PmpfF(("loading strings"));

        for (us = 0; us < MAX_NLS; us++)
            pNLSData->apszNLSStrings[us] = NULL;

        for (us = 0; us < MAX_NLS; us++)
        {
            usLen = strlen(pszSrc) + 1;
            // stops processing if the storage buffer is too small
            if ((pszDest + usLen) > (pNLSData->achBuf + sizeof(pNLSData->achBuf)))
                break;

            memcpy(pszDest, pszSrc, usLen + 1); // V1.0.0 (2002-09-17) [umoeller]

            pNLSData->apszNLSStrings[us] = pszDest;
            pszSrc += usLen;
            pszDest += usLen;

            // end of the source buffer
            if (*pszSrc == 0)
                break;
        }

        return TRUE;
    }

    return FALSE;
}

/*
 *@@ InstallHook:
 *      this installs the hooks in XWPHOOK.DLL by calling
 *      hookInit in XWPHOOK.DLL and then loads the configuration
 *      by calling LoadHotkeysForHook and LoadHookConfig.
 *
 *      This sets the global G_pHookData variable which
 *      points to the shared HOOKDATA structure in
 *      XWPHOOK.DLL.
 *
 *      This gets called when fnwpDaemonObject receives
 *      XDM_HOOKINSTALL.
 *
 *      At this point, if this is the first call, only the
 *      daemon object window has been created in main().
 *      XPager doesn't exist yet and will only be started
 *      later when a separate XDM_STARTSTOPPAGER message
 *      is received by fnwpDaemonObject.
 */

VOID InstallHook(VOID)
{
    if (!G_pXwpGlobalShared->fAllHooksInstalled)
    {
        // install hook
        if (    (G_pHookData = hookInit(G_pXwpGlobalShared->hwndDaemonObject))
             && (G_pHookData->fInputHooked)
             && (G_pHookData->fPreAccelHooked)
           )
        {
            // success:
            G_pXwpGlobalShared->fAllHooksInstalled = TRUE;
            // load hotkeys list from OS2.INI
            LoadHotkeysForHook();
            // load config from OS2.INI
            LoadHookConfig(TRUE,
                           TRUE);

            // copy NLS resources from shared mem to
            // hook data
            // V1.0.0 (2002-09-17) [umoeller]
            LoadNLSResources();
        }
    }
}

/*
 *@@ DeinstallHook:
 *      this removes the hooks by calling hookKill
 *      in XWPHOOK.DLL and also calls dmnKillXPager.
 *
 *      This gets called when fnwpDaemonObject receives
 *      XDM_HOOKINSTALL. Also, this gets called on the
 *      daemon exit list in case the daemon crashes.
 *
 *@@changed V0.9.4 (2000-07-14) [umoeller]: timers kept running after this; fixed
 *@@changed V0.9.19 (2002-06-15) [lafaix]: minor cleanup
 */

VOID DeinstallHook(VOID)
{
    if (G_pXwpGlobalShared->fAllHooksInstalled)
    {
        G_pXwpGlobalShared->fAllHooksInstalled = FALSE;

#ifndef __NOMOVEMENT2FEATURES__
        // if mouse pointer is currently hidden,
        // show it now (otherwise it'll be lost...)
        if (G_pHookData)
            if (G_pHookData->fMousePointerHidden)
                WinShowPointer(HWND_DESKTOP, TRUE);
#endif

#ifndef __NOPAGER__
        dmnKillXPager(FALSE);
#endif

        hookKill();

        // stop timers which might still be running V0.9.4 (2000-07-14) [umoeller]
#ifndef __NOSLIDINGFOCUS__
        WinStopTimer(G_habDaemon,
                     G_pXwpGlobalShared->hwndDaemonObject,
                     TIMERID_SLIDINGFOCUS);
#endif
        WinStopTimer(G_habDaemon,
                     G_pXwpGlobalShared->hwndDaemonObject,
                     TIMERID_SLIDINGMENU);
#ifndef __NOMOVEMENT2FEATURES__
        WinStopTimer(G_habDaemon,
                     G_pXwpGlobalShared->hwndDaemonObject,
                     TIMERID_AUTOHIDEMOUSE);
#endif

        // _Pmpf(("XWPDAEMON: hookKilled called"));
    }
    G_pHookData = NULL;
}

/* ******************************************************************
 *
 *   Hook message processing
 *
 ********************************************************************/

typedef HSWITCH APIENTRY WINHSWITCHFROMHAPP(HAPP happ);

/*
 *@@ winhHSWITCHfromHAPP:
 *      resolves and calls the undocumented
 *      WinHSWITCHfromHAPP API.
 *
 *@@added V0.9.19 (2002-04-17) [umoeller]
 */

HSWITCH winhHSWITCHfromHAPP(HAPP happ)
{
    static WINHSWITCHFROMHAPP *pWinHSWITCHfromHAPP = NULL;

    if (!pWinHSWITCHfromHAPP)
        // first call: import WinHSWITCHfromHAPP
        // WinHSWITCHfromHAPP PMMERGE.5199
        doshQueryProcAddr("PMMERGE",
                          5199,
                          (PFN*)&pWinHSWITCHfromHAPP);

    if (pWinHSWITCHfromHAPP)
        return pWinHSWITCHfromHAPP(happ);

    return NULLHANDLE;
}

/*
 *@@ winhSleep:
 *      copied over here from winh.c.
 *
 *@@added V1.0.2 (2003-02-07) [umoeller]
 */

VOID winhSleep(ULONG ulSleep)    // in: sleep time in milliseconds
{
    HWND    hwnd;

    if (hwnd = WinCreateWindow(HWND_OBJECT,
                               WC_STATIC,
                               (PSZ)"",
                               0,
                               0,0,0,0,
                               0,
                               HWND_BOTTOM,
                               0,
                               NULL,
                               NULL))
    {
        QMSG    qmsg;
        HAB     hab;

        if (    (hab = WinQueryAnchorBlock(hwnd))
             && (WinStartTimer(hab,
                               hwnd,
                               1,
                               ulSleep))
           )
        {
            while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
            {
                if (    (qmsg.hwnd == hwnd)
                     && (qmsg.msg == WM_TIMER)
                     && (qmsg.mp1 == (MPARAM)1)     // timer ID
                   )
                    break;

                WinDispatchMsg(hab, &qmsg);
            }
            WinStopTimer(hab,
                         hwnd,
                         1);
        }
        else
            // timer creation failed:
            DosSleep(ulSleep);

        WinDestroyWindow(hwnd);
    }
    else
        DosSleep(ulSleep);
}

/*
 *@@ ProcessAutoScroll:
 *      this gets called from fnwpDaemonObject to
 *      implement the "AutoScroll" feature.
 *
 *      This is based on WMMouseMove_MB3ScrollLineWise in xwphook.c.
 *
 *@@added V0.9.9 (2001-03-21) [lafaix]
 */

STATIC VOID ProcessAutoScroll(PSCROLLDATA pScrollData,
                              LONG lDelta,
                              BOOL fHorizontal)
{
    USHORT  usScrollCode;
    ULONG   ulMsg;
    ULONG   ulOffset;
    LONG    lTick;

    if (!fHorizontal)
    {
        if (lDelta > 0)
            usScrollCode = SB_LINEDOWN;
        else
            usScrollCode = SB_LINEUP;

        ulMsg = WM_VSCROLL;
    }
    else
    {
        if (lDelta > 0)
            usScrollCode = SB_LINERIGHT;
        else
            usScrollCode = SB_LINELEFT;

        ulMsg = WM_HSCROLL;
    }

    ulOffset = max(0, abs(lDelta) - G_pHookData->HookConfig.usMB3ScrollMin);
    lTick = 1L + 10L - (10L * ulOffset / 50L);

    if (    (ulOffset)
         && (    (lTick <= 0)
              || ((G_ulAutoScrollTick % lTick) == 0)
            )
       )
    {
        LONG   i;
        // save window ID of scroll bar control
        USHORT usScrollBarID = WinQueryWindowUShort(pScrollData->hwndScrollBar,
                                                    QWS_ID);

        for (i = (lTick < 0) ? -lTick : 1; i; i--)
        {
            // send up or down scroll message if still autoscrolling
            if (    (G_pHookData->bAutoScroll)
                 && (WinSendMsg(pScrollData->hwndScrollLastOwner,
                           ulMsg,
                           MPFROMSHORT(usScrollBarID),
                           MPFROM2SHORT(0, usScrollCode)))
               )
                // send end scroll message
                WinSendMsg(pScrollData->hwndScrollLastOwner,
                           ulMsg,
                           MPFROMSHORT(usScrollBarID),
                           MPFROM2SHORT(0, SB_ENDSCROLL));
            else
                break;
        }
    }
}

#ifndef __NOSLIDINGFOCUS__

/*
 *@@ ProcessSlidingFocus:
 *      this gets called from fnwpDaemonObject to
 *      implement the "sliding focus" feature.
 *
 *      This gets called in two situations:
 *
 *      -- If sliding focus delay is off, directly
 *         when the daemon receives XDM_SLIDINGFOCUS
 *         from the hook, that is, if the mouse has
 *         moved over a new frame window which should
 *         be activated.
 *
 *      -- If we have a delay of sliding focus,
 *         fnwpDaemonObject starts a timer on receiving
 *         XDM_SLIDINGFOCUS and calls this func only
 *         after the timer has elapsed.
 *
 *      This was initially based on code from
 *      ProgramCommander/2, but has been extended
 *      quite some since then.
 *
 *@@changed V0.9.3 (2000-05-23) [umoeller]: fixed MDI-subframes problem (VIEW.EXE)
 *@@changed V0.9.4 (2000-06-12) [umoeller]: fixed Win-OS/2 handling, which broke with 0.9.3
 *@@changed V0.9.7 (2000-12-08) [umoeller]: added "ignore XCenter"
 *@@changed V0.9.16 (2002-01-05) [umoeller]: now disabling sliding focus while "move ptr to button" is in progress
 *@@changed V0.9.19 (2002-05-07) [umoeller]: now always ignoring pager
 *@@changed V1.0.8 (2007-02-10) [yuri]: added Open Office non-frame window exception
 */

STATIC VOID ProcessSlidingFocus(HWND hwndFrameInBetween, // in: != NULLHANDLE if hook has detected another frame
                                                         // under the mouse before the desktop frame window was
                                                         // reached
                                HWND hwnd2Activate)     // in: highest parent of hwndUnderMouse, child of desktop
{
    HWND    hwndCurrentlyActive = NULLHANDLE,
            hwndClient = NULLHANDLE;
    BOOL    fIsSeamless = FALSE;

    // rule out desktops, if "ignore desktop" is on
    if (G_pHookData->HookConfig.__fSlidingIgnoreDesktop)
        if (hwnd2Activate == G_pHookData->hwndPMDesktop)
            // target is PM desktop:
            return;
        else if (hwnd2Activate == G_pHookData->hwndWPSDesktop)
            // target is WPS desktop:
            return;

#ifndef __NOPAGER__
    // rule out XPager, if "ignore XPager" is on
    // if (G_pHookData->HookConfig.__fSlidingIgnoreXPager)
    // always ignoring pager now V0.9.19 (2002-05-07) [umoeller]
    if (hwnd2Activate == G_pHookData->hwndPagerFrame)
        return;
#endif

    // stop if "move ptr to button" is in progress
    // V0.9.16 (2002-01-05) [umoeller]
    if (G_ulMovingPtrTimer)
        return;

    // V0.9.7 (2000-12-08) [umoeller]:
    // rule out XCenter, if "ignore XCenter" is on
    // now always ignoring XCenter V0.9.19 (2002-05-07) [umoeller]
    hwndClient = WinWindowFromID(hwnd2Activate, FID_CLIENT);
            // also needed below for seamless
    // Is-XCenter check: the window must have an FID_CLIENT
    // whose class name is WC_XCENTER_CLIENT
    if (hwndClient)
    {
        CHAR szClass[100];
        WinQueryClassName(hwndClient, sizeof(szClass), szClass);
        if (   (!strcmp(szClass, WC_XCENTER_CLIENT))  // target is XCenter
            || (!strcmp(szClass, "SALSUBFRAME")) // V1.0.8 target is Open Office non-frame window
           )
            return;
    }

    // always ignore window list
    if (hwnd2Activate == G_pHookData->hwndSwitchList)
        return;

    if (WinQueryWindowULong(hwnd2Activate, QWL_STYLE) & WS_DISABLED)
        // window is disabled: ignore
        return;

    hwndCurrentlyActive = WinQueryActiveWindow(HWND_DESKTOP);

    // and stop if currently active window is the window list too
    if (hwndCurrentlyActive == G_pHookData->hwndSwitchList)
        return;

    // handle seamless Win-OS/2 windows: those have a frame
    // and an FID_CLIENT of the SeamlessClass class
    if (hwndClient)     // queried above
    {
        CHAR szClassName[200];
        WinQueryClassName(hwndClient, sizeof(szClassName), szClassName);
        // _Pmpf(("ProcessSlidingFocus: hwndClient 0x%lX", hwndClient));
        // _Pmpf(("  class name: %s", szClassName));
        if (!strcmp(szClassName, "SeamlessClass"))
            fIsSeamless = TRUE;
    }

    // rule out redundant processing:
    // we might come here if the mouse has moved
    // across several desktop windows while the
    // sliding-focus delay has not elapsed yet
    // so that the active window really hasn't changed

    // bring-to-top mode
    // or window to activate is seamless Win-OS/2 window
    // and should be brought to the top?
    if (    (G_pHookData->HookConfig.__fSlidingBring2Top)
         || (   (fIsSeamless)
             && (!G_pHookData->HookConfig.__fSlidingIgnoreSeamless)
            )
       )
    {
        if (hwndCurrentlyActive != hwnd2Activate)
        {
            WinSetActiveWindow(HWND_DESKTOP, hwnd2Activate);
            G_pHookData->hwndActivatedByUs = hwnd2Activate;
        }
    }
    else
    {
        // no bring-to-top == preserve-Z-order mode:
        if (!fIsSeamless)
        {
            HWND hwndNewFocus = NULLHANDLE;

            // not seamless Win-OS/2 window:
            HWND    hwndTitlebar = NULLHANDLE;

            /*
             * step 1: handle subframes
             *
             */

            // let's check if we have an MDI frame, that is,
            // another frame below the subframe:
            HWND    hwndFocusSubwin = NULLHANDLE;

            if (hwndFrameInBetween)
                // hook has already detected another sub-frame under
                // the mouse: use that subframe's client
                hwndFocusSubwin = WinWindowFromID(hwndFrameInBetween, FID_CLIENT);

            if (!hwndFocusSubwin)
                // not found: try if maybe the main frame has saved
                // the focus of a subclient
                hwndFocusSubwin = WinQueryWindowULong(hwnd2Activate,
                                                      QWL_HWNDFOCUSSAVE);
                // typical cases of this are:
                // 1) a dialog window with controls, one of which had the focus;
                // 2) a plain frame window with a client which had the focus;
                // 3) complex case: a main frame window with several frame subwindows.
                //    Best example is VIEW.EXE, which has the following hierachy:
                //      main frame -- main client
                //                      +--- another view frame with a sub-client
                //                      +--- another view frame with a sub-client
                //    hwndFocusSubwin then has one of the sub-clients, so we better
                //    check:

            if ((hwndFocusSubwin) && (WinIsWindowVisible(hwndFocusSubwin)))
            {
                // frame window has control or client with focus stored:

                CHAR    szFocusSaveClass[200] = "";

                // from the hwndFocusSubwin, climb up the parents tree until
                // we reach the main frame. If we find another frame between
                // hwndFocusSubwin and the main frame, activate that one as well.
                HWND    hwndTemp = hwndFocusSubwin,
                        hwndMDIFrame = NULLHANDLE;
                                // this receives the "in-between" frame
                while (     (hwndTemp)
                         && (hwndTemp != hwnd2Activate)
                      )
                {
                    WinQueryClassName(hwndTemp,
                                      sizeof(szFocusSaveClass),
                                      szFocusSaveClass);
                    if (!strcmp(szFocusSaveClass, "#1"))
                    {
                        // found another frame in between:
                        hwndMDIFrame = hwndTemp;
                        break;
                    }

                    hwndTemp = WinQueryWindow(hwndTemp, QW_PARENT);
                }

                if (hwndMDIFrame)
                {
                    // yes, we found another frame in between:
                    // make that active as well
                    if (hwndTitlebar = WinWindowFromID(hwndMDIFrame, FID_TITLEBAR))
                    {
                        WinPostMsg(hwndMDIFrame,
                                   WM_ACTIVATE,
                                   MPFROMSHORT(TRUE),
                                   MPFROMHWND(hwndTitlebar));
                    }
                }

                hwndNewFocus = hwndFocusSubwin;
            }
            else
            {
                // frame window has no saved focus:
                // try if we can activate the client
                if (hwndClient = WinWindowFromID(hwnd2Activate,
                                                 FID_CLIENT))
                {
                    // _Pmpf(("        giving focus 2 client"));
                    hwndNewFocus = hwndClient;
                }
                else
                {
                    // _Pmpf(("        no client, giving focus 2 frame"));
                    // we don't even have a client:
                    // activate the frame directly
                    hwndNewFocus = hwnd2Activate;
                }

            }

            // now give the new window the focus
            if (hwndNewFocus)
            {
                WinFocusChange(HWND_DESKTOP,
                               hwndNewFocus,
                               FC_NOSETACTIVE);
            }

            // now handle main frame

            if (hwndCurrentlyActive != hwnd2Activate)
            {
                // deactivate old window
                WinPostMsg(hwndCurrentlyActive,
                           WM_ACTIVATE,
                           MPFROMSHORT(FALSE),      // deactivate
                           MPFROMHWND(hwndNewFocus));

                // activate new window
                if (hwndTitlebar = WinWindowFromID(hwnd2Activate, FID_TITLEBAR))
                {
                    WinPostMsg(hwnd2Activate,
                               WM_ACTIVATE,
                               MPFROMSHORT(TRUE),
                               MPFROMHWND(hwndTitlebar));
                }
            }

            // store activated window in hook data
            // so that the hook knows that we activated
            // this window
            G_pHookData->hwndActivatedByUs = hwnd2Activate;
        }
    }
}

#endif

/*
 *@@ ProcessHotCorner:
 *      implementation for XDM_HOTCORNER in fnwpDaemonObject.
 *
 *@@added V0.9.14 (2001-08-20) [umoeller]
 *@@changed V0.9.18 (2002-02-12) [pr]: added screen wrap
 *@@changed V1.0.5 (2006-05-16) [pr]: Fixed broken pager wrap at vertical borders @@fixes 406
 */

STATIC VOID ProcessHotCorner(MPARAM mp1)
{
    // create array index: mp1
    LONG    lIndex = (LONG)mp1;
    HOBJECT hobjIndex;

    if (    (G_pHookData)
         && (lIndex >= SCREENCORNER_MIN)
         && (lIndex <= SCREENCORNER_MAX)
            // hot-corner object defined?
         && (hobjIndex = G_pHookData->HookConfig.ahobjHotCornerObjects[lIndex])
       )
    {
        UCHAR ucScanCode = 0;

        // _PmpfF(("got hot corner %d", lIndex));

        switch (hobjIndex)
        {
            // XPager?
#ifndef __NOPAGER__
            case SPECIALOBJ_PAGER_SHOW: // 0xFFFF0002:
                // yes: bring up XPager window
                WinSetWindowPos(G_pHookData->hwndPagerFrame,
                                HWND_TOP,
                                0, 0, 0, 0,
                                SWP_ZORDER | SWP_SHOW | SWP_RESTORE);
            break;

            // XPager screen change?
            case SPECIALOBJ_PAGER_UP: // 0xFFFF0003:
                ucScanCode = 0x61;
            break;

            case SPECIALOBJ_PAGER_RIGHT: // 0xFFFF0004:
                ucScanCode = 0x64;
            break;

            case SPECIALOBJ_PAGER_DOWN: // 0xFFFF0005:
                ucScanCode = 0x66;
            break;

            case SPECIALOBJ_PAGER_LEFT: // 0xFFFF0006:
                ucScanCode = 0x63;
            break;
#endif

            case SPECIALOBJ_SCREENWRAP: // 0xFFFF0007: // V0.9.18 (2002-02-12) [pr]
            {
                POINTL ptlCurrent;

                WinQueryPointerPos(HWND_DESKTOP, &ptlCurrent);
                switch(lIndex)
                {
                    case SCREENCORNER_BOTTOMLEFT:
                        ptlCurrent.x = G_pHookData->cxScreen - 2;
                        ptlCurrent.y = G_pHookData->cyScreen - 2;
                    break;

                    case SCREENCORNER_TOPLEFT:
                        ptlCurrent.x = G_pHookData->cxScreen - 2;
                        ptlCurrent.y = 1;
                    break;

                    case SCREENCORNER_BOTTOMRIGHT:
                        ptlCurrent.x = 1;
                        ptlCurrent.y = G_pHookData->cyScreen - 2;
                    break;

                    case SCREENCORNER_TOPRIGHT:
                        ptlCurrent.x = 1;
                        ptlCurrent.y = 1;
                    break;

                    case SCREENCORNER_TOP:
                        ptlCurrent.y = 1;
                    break;

                    case SCREENCORNER_LEFT:
                        ptlCurrent.x = G_pHookData->cxScreen - 2;
                    break;

                    case SCREENCORNER_RIGHT:
                        ptlCurrent.x = 1;
                    break;

                    case SCREENCORNER_BOTTOM:
                        ptlCurrent.y = G_pHookData->cyScreen - 2;
                    break;
                }

                WinSetPointerPos(HWND_DESKTOP, ptlCurrent.x, ptlCurrent.y);
            }
            break;

            default:
                // real object or some other special code:
                // let XFLDR.DLL thread-1 object handle this
                WinPostMsg(G_pXwpGlobalShared->hwndThread1Object,
                           T1M_OPENOBJECTFROMHANDLE,
                           // pass HOBJECT or special function (0xFFFF000x)
                           (MPARAM)hobjIndex,
                           // pass screen border in mp1
                           (MPARAM)(lIndex + 1));
        } // end switch

#ifndef __NOPAGER__
        if (ucScanCode)
        {
            POINTL ptlCurrScreen;
            // we had a XPager screen change above:
            // where are we right now ?
            // (0,0) is _upper_ left, not bottom left
            // No, not in terms of virtual desktops it isn't.... hence bug 406
            ptlCurrScreen.x = G_pHookData->ptlCurrentDesktop.x / G_pHookData->szlEachDesktopFaked.cx;
            ptlCurrScreen.y = G_pHookData->ptlCurrentDesktop.y / G_pHookData->szlEachDesktopFaked.cy;

            // if we do move, wrap the pointer too so
            // that we don't move too much
            // V1.0.5 (2006-05-16) [pr]: Swapped comparisons for 0x61 and 0x66 @@fixes 406
            if (    (G_pHookData->PagerConfig.flPager & PGRFL_WRAPAROUND)
                 || (    (ucScanCode == 0x61)
                      && (ptlCurrScreen.y < (G_pHookData->PagerConfig.cDesktopsY - 1))
                    )
                 || (    (ucScanCode == 0x66)
                      && (ptlCurrScreen.y > 0)
                    )
                 || (    (ucScanCode == 0x64)
                      && (ptlCurrScreen.x < (G_pHookData->PagerConfig.cDesktopsX - 1))
                    )
                 || (    (ucScanCode == 0x63)
                      && (ptlCurrScreen.x > 0)
                    )
               )
            {
                // we must send this message, not post it
                WinSendMsg(G_pHookData->hwndPagerClient,
                           PGRM_WRAPAROUND,
                           (MPARAM)ucScanCode,
                           0);
                WinSetPointerPos(HWND_DESKTOP,
                                 G_pHookData->cxScreen / 2,
                                 G_pHookData->cyScreen / 2);
            }
        }
#endif
    }
}

/*
 *@@ ProcessChordWinList:
 *      implementation for XDM_WMCHORDWINLIST in fnwpDaemonObject.
 *
 *@@added V0.9.19 (2002-03-28) [umoeller]
 */

STATIC VOID ProcessChordWinList(VOID)
{
    POINTL  ptlMouse;       // mouse coordinates
    SWP     WinListPos;     // position of window list window
    LONG WinListX, WinListY; // new ordinates of window list window
    // get mouse coordinates (absolute coordinates)
    WinQueryPointerPos(HWND_DESKTOP, &ptlMouse);
    // get position of window list window
    WinQueryWindowPos(G_pHookData->hwndSwitchList,
                      &WinListPos);
    // calculate window list position (mouse pointer is center)
    WinListX = ptlMouse.x - (WinListPos.cx / 2);
    if (WinListX < 0)
        WinListX = 0;
    WinListY = ptlMouse.y - (WinListPos.cy / 2);
    if (WinListY < 0)
        WinListY = 0;
    if (WinListX + WinListPos.cx > G_pHookData->cxScreen)
        WinListX = G_pHookData->cxScreen - WinListPos.cx;
    if (WinListY + WinListPos.cy > G_pHookData->cyScreen)
        WinListY = G_pHookData->cyScreen - WinListPos.cy;
    // set window list window to calculated position
    WinSetWindowPos(G_pHookData->hwndSwitchList,
                    HWND_TOP,
                    WinListX, WinListY, 0, 0,
                    SWP_MOVE | SWP_SHOW | SWP_ZORDER);
    // now make it the active window
    WinSetActiveWindow(HWND_DESKTOP,
                       G_pHookData->hwndSwitchList);
}

/*
 *@@ ProcessBeginScroll:
 *      implementation for XDM_BEGINSCROLL in fnwpDaemonObject.
 *
 *@@added V0.9.19 (2002-03-28) [umoeller]
 */

STATIC VOID ProcessBeginScroll(HWND hwndObject)
{
    G_lScrollMode = (G_pHookData->SDXHorz.hwndScrollBar)
                  ? (G_pHookData->SDYVert.hwndScrollBar)
                        ? ALL
                        : HORZ
                  : VERT;

    switch (G_lScrollMode)
    {
        case ALL:
            G_ahptrPointers[NORTH]     = G_hptrN;
            G_ahptrPointers[NORTHEAST] = G_hptrNE;
            G_ahptrPointers[EAST]      = G_hptrE;
            G_ahptrPointers[SOUTHEAST] = G_hptrSE;
            G_ahptrPointers[SOUTH]     = G_hptrS;
            G_ahptrPointers[SOUTHWEST] = G_hptrSW;
            G_ahptrPointers[WEST]      = G_hptrW;
            G_ahptrPointers[NORTHWEST] = G_hptrNW;
            G_ahptrPointers[CENTER]    = G_hptrNESW;
        break;
        case HORZ:
            G_ahptrPointers[NORTH]     = G_hptrEW;
            G_ahptrPointers[NORTHEAST] = G_hptrE;
            G_ahptrPointers[EAST]      = G_hptrE;
            G_ahptrPointers[SOUTHEAST] = G_hptrE;
            G_ahptrPointers[SOUTH]     = G_hptrEW;
            G_ahptrPointers[SOUTHWEST] = G_hptrW;
            G_ahptrPointers[WEST]      = G_hptrW;
            G_ahptrPointers[NORTHWEST] = G_hptrW;
            G_ahptrPointers[CENTER]    = G_hptrEW;
        break;
        case VERT:
            G_ahptrPointers[NORTH]     = G_hptrN;
            G_ahptrPointers[NORTHEAST] = G_hptrN;
            G_ahptrPointers[EAST]      = G_hptrNS;
            G_ahptrPointers[SOUTHEAST] = G_hptrS;
            G_ahptrPointers[SOUTH]     = G_hptrS;
            G_ahptrPointers[SOUTHWEST] = G_hptrS;
            G_ahptrPointers[WEST]      = G_hptrNS;
            G_ahptrPointers[NORTHWEST] = G_hptrN;
            G_ahptrPointers[CENTER]    = G_hptrNS;
        break;
    }

    G_hptrOld = WinQueryPointer(HWND_DESKTOP);
    WinSetPointer(HWND_DESKTOP, G_ahptrPointers[CENTER]);

    if (shpLoadBitmap(G_habDaemon,
                      NULL,          // load from resources
                      NULLHANDLE,    // load from exe
                      139 + G_lScrollMode,
                                     // resource ID
                      &G_sfScrollOrigin))
    {
        WinQueryPointerPos(HWND_DESKTOP, &G_ptlScrollOrigin);
        G_sfScrollOrigin.ptlLowerLeft.x = G_ptlScrollOrigin.x - 16;
        G_sfScrollOrigin.ptlLowerLeft.y = G_ptlScrollOrigin.y - 16;
        shpCreateWindows(&G_sfScrollOrigin);
        G_fScrollOriginLoaded = TRUE;
    }
    else
        G_fScrollOriginLoaded = FALSE;

    // if AutoScroll, start timer
    if (G_pHookData->bAutoScroll)
    {
        G_ulAutoScrollTick = 0;
        WinStartTimer(G_habDaemon,
                      hwndObject,
                      TIMERID_AUTOSCROLL,
                      100); // 10 times per second
    }
}

/*
 *@@ ProcessEndScroll:
 *      implementation for XDM_ENDSCROLL in fnwpDaemonObject.
 *
 *@@added V0.9.19 (2002-03-28) [umoeller]
 */

STATIC VOID ProcessEndScroll(HWND hwndObject)
{
    if (G_hptrOld)
        WinSetPointer(HWND_DESKTOP, G_hptrOld);
    G_hptrOld = NULLHANDLE;

    if (G_fScrollOriginLoaded)
    {
        shpFreeBitmap(&G_sfScrollOrigin);
        WinDestroyWindow(G_sfScrollOrigin.hwndShapeFrame) ;
        WinDestroyWindow(G_sfScrollOrigin.hwndShape);
        G_fScrollOriginLoaded = FALSE;
    }

    // if AutoScroll, stop timer
    if (G_pHookData->bAutoScroll)
    {
        G_pHookData->bAutoScroll = FALSE;
        WinStopTimer(G_habDaemon,
                     hwndObject,
                     (ULONG)TIMERID_AUTOSCROLL);
    }
}

/*
 *@@ ProcessSetPointer:
 *      implementation for XDM_SETPOINTER in fnwpDaemonObject.
 *
 *@@added V0.9.19 (2002-03-28) [umoeller]
 */

STATIC VOID ProcessSetPointer(MPARAM mp1)
{
    if (    (abs(G_ptlScrollOrigin.x-SHORT1FROMMP(mp1)) < 2*(G_pHookData->HookConfig.usMB3ScrollMin))
         && (abs(G_ptlScrollOrigin.y-SHORT2FROMMP(mp1)) < 2*(G_pHookData->HookConfig.usMB3ScrollMin))
       )
        // too close to call
        WinSetPointer(HWND_DESKTOP, G_ahptrPointers[CENTER]);
    else if (G_ptlScrollOrigin.x > SHORT1FROMMP(mp1) + 16)
    {
        // in the west part
        if (G_ptlScrollOrigin.y > SHORT2FROMMP(mp1) + 16)
            WinSetPointer(HWND_DESKTOP, G_ahptrPointers[SOUTHWEST]);
        else
        if (G_ptlScrollOrigin.y < SHORT2FROMMP(mp1) - 16)
            WinSetPointer(HWND_DESKTOP, G_ahptrPointers[NORTHWEST]);
        else
            WinSetPointer(HWND_DESKTOP, G_ahptrPointers[WEST]);
    }
    else if (G_ptlScrollOrigin.x < SHORT1FROMMP(mp1) - 16)
    {
        // in the east part
        if (G_ptlScrollOrigin.y > SHORT2FROMMP(mp1) + 16)
            WinSetPointer(HWND_DESKTOP, G_ahptrPointers[SOUTHEAST]);
        else
        if (G_ptlScrollOrigin.y < SHORT2FROMMP(mp1) - 16)
            WinSetPointer(HWND_DESKTOP, G_ahptrPointers[NORTHEAST]);
        else
            WinSetPointer(HWND_DESKTOP, G_ahptrPointers[EAST]);
    }
    else if (G_ptlScrollOrigin.y > SHORT2FROMMP(mp1))
        // in the south
        WinSetPointer(HWND_DESKTOP, G_ahptrPointers[SOUTH]);
    else
        // in the north
        WinSetPointer(HWND_DESKTOP, G_ahptrPointers[NORTH]);
}

/*
 *@@ ProcessTimer:
 *      implementation for WM_TIMER in fnwpDaemonObject.
 *
 *@@added V0.9.14 (2001-08-21) [umoeller]
 *@@changed V0.9.14 (2001-08-21) [umoeller]: added delayed move-ptr-to-button
 */

STATIC MRESULT ProcessTimer(HWND hwndObject, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PULONG      pulStopTimer = NULL;

    switch ((ULONG)mp1)
    {
#ifndef __NOSLIDINGFOCUS__
        /*
         * TIMERID_SLIDINGFOCUS:
         *
         */

        case TIMERID_SLIDINGFOCUS:
            // stop the timer
            pulStopTimer = &G_ulSlidingFocusTimer;

            ProcessSlidingFocus(G_hwndSlidingUnderMouse,
                                G_hwndSliding2Activate);
        break;
#endif

        /*
         * TIMERID_SLIDINGMENU:
         *      started from XDM_SLIDINGMENU.
         */

        case TIMERID_SLIDINGMENU:
            // stop the timer
            pulStopTimer = &G_ulSlidingMenuTimer;

            // post a special WM_MOUSEMOVE message;
            // see WMMouseMove_SlidingMenus in xwphook.c
            // for how this works
            WinPostMsg(G_pHookData->hwndMenuUnderMouse,
                       WM_MOUSEMOVE,
                       G_SlidingMenuMp1Saved,
                            // MP1 which was saved in XDM_SLIDINGMENU
                            // to identify this msg in the hook
                       MPFROM2SHORT(HT_DELAYEDSLIDINGMENU,
                                    KC_NONE));
        break;

        /*
         * TIMERID_MONITORDRIVE:
         *
         */

        /* case TIMERID_MONITORDRIVE:
            CheckRemoveableDrive();
        break; */

#ifndef __NOMOVEMENT2FEATURES__
        /*
         * TIMERID_AUTOHIDEMOUSE:
         *
         */

        case TIMERID_AUTOHIDEMOUSE:
            pulStopTimer = &G_pHookData->idAutoHideTimer;

            WinShowPointer(HWND_DESKTOP, FALSE);
            G_pHookData->fMousePointerHidden = TRUE;
        break;
#endif

        /*
         * TIMERID_AUTOSCROLL:
         *      started from XDM_BEGINSCROLL and stopped from
         *      XDM_ENDSCROLL.
         *
         */

        case TIMERID_AUTOSCROLL:
            WinQueryPointerPos(HWND_DESKTOP, &G_ptlScrollCurrent);
            if (G_lScrollMode & HORZ)
                ProcessAutoScroll(&G_pHookData->SDXHorz,
                                  -(G_ptlScrollOrigin.x - G_ptlScrollCurrent.x),
                                  TRUE);
            if (G_lScrollMode & VERT)
                ProcessAutoScroll(&G_pHookData->SDYVert,
                                  G_ptlScrollOrigin.y - G_ptlScrollCurrent.y,
                                  FALSE);
            G_ulAutoScrollTick++;
        break;

#ifndef __NOMOVEMENT2FEATURES__
        /*
         * TIMERID_MOVINGPTR:
         *      started by XDM_MOVEPTRTOBUTTON and moves the
         *      pointer successively to the default push button.
         *
         *      XDM_MOVEPTRTOBUTTON has stored the initial
         *      mouse position in G_ptlMovingPtrStart and the
         *      position to move to (the center of the default
         *      button) in G_ptlMovingPtrEnd.
         */

        case TIMERID_MOVINGPTR:
        {
            ULONG   ulMSNow = doshQuerySysUptime();
            LONG    lMSElapsed = (ulMSNow - G_ulMSMovingPtrStart);
            POINTL  ptlCurrent;

            ARCPARAMS ap = {1, 1, 0, 0};
            HPS     hps = NULLHANDLE;

            if (G_pHookData->HookConfig.__ulAutoMoveFlags & AMF_ANIMATE)
            {
                // animation: prepare painting then
                hps = WinGetScreenPS(HWND_DESKTOP);
                // @@todo or maybe hps = WinGetClipPS(HWND_DESKTOP, 0, PSF_LOCKWINDOWUPDATE)?

                GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULL);
                GpiSetColor(hps, 0x00FFFFFF);
                GpiSetMix(hps, FM_XOR);
                GpiSetLineWidth(hps, MAKEFIXED(3, 0));

                // prepare the full arc: center is middle
                // of the mouse button
                GpiSetArcParams(hps, &ap);
                GpiMove(hps, &G_ptlMovingPtrEnd);
            }

            // go check if the user is currently moving the mouse...
            // if so, we should stop doing all this.
            WinQueryPointerPos(HWND_DESKTOP, &ptlCurrent);
            if (    (abs(ptlCurrent.x - G_ptlMovingPtrNow.x) > 5)
                 || (abs(ptlCurrent.y - G_ptlMovingPtrNow.y) > 5)
               )
                pulStopTimer = &G_ulMovingPtrTimer;

            if ((hps) && (G_lLastMovingPtrRadius))
                // un-paint the last item
                GpiFullArc(hps,
                           DRO_OUTLINE,
                           MAKEFIXED(G_lLastMovingPtrRadius, 0));

            if (lMSElapsed >= G_pHookData->HookConfig.__ulAutoMoveDelay)
            {
                // max time elapsed: stop then
                pulStopTimer = &G_ulMovingPtrTimer;

                WinSetPointerPos(HWND_DESKTOP,
                                 G_ptlMovingPtrEnd.x,
                                 G_ptlMovingPtrEnd.y);
                G_lLastMovingPtrRadius = 0;
            }
            else
            {
                // we're somewhere in the middle:
                LONG lOfs;
                LONG lMax = (LONG)G_pHookData->HookConfig.__ulAutoMoveDelay;

                // x
                lOfs =   (G_ptlMovingPtrEnd.x - G_ptlMovingPtrStart.x)  // max distance to go
                       * lMSElapsed
                       / lMax;
                G_ptlMovingPtrNow.x = G_ptlMovingPtrStart.x + lOfs;
                // x
                lOfs =   (G_ptlMovingPtrEnd.y - G_ptlMovingPtrStart.y)  // max distance to go
                       * lMSElapsed
                       / lMax;
                G_ptlMovingPtrNow.y = G_ptlMovingPtrStart.y + lOfs;

                // _Pmpf(("ms elapsed: %d, new x/y: %d/%d", lMSElapsed, G_ptlMovingPtrNow.x, G_ptlMovingPtrNow.y));

                WinSetPointerPos(HWND_DESKTOP,
                                 G_ptlMovingPtrNow.x,
                                 G_ptlMovingPtrNow.y);

                // now go paint on the screen
                if ((hps) && (!pulStopTimer))
                {
                    lOfs =   100L
                           - (  100L
                              * lMSElapsed
                              / lMax);
                    GpiFullArc(hps,
                               DRO_OUTLINE,
                               MAKEFIXED(lOfs, 0));
                    G_lLastMovingPtrRadius = lOfs;
                }
            }

            if (hps)
                WinReleasePS(hps);
        }
        break;
#endif

        default:
            return WinDefWindowProc(hwndObject, msg, mp1, mp2);
    }

    if (pulStopTimer)
    {
        WinStopTimer(G_habDaemon,
                     hwndObject,
                     (ULONG)mp1);   // timer ID
        *pulStopTimer = 0;
    }

    return 0;
}

/*
 *@@ ProcessAddNotify:
 *      implementation for XDM_ADDCLICKWATCH and XDM_ADDWINLISTWATCH
 *      in fnwpDaemonObject.
 *
 *      Returns TRUE if successful, FALSE otherwise.
 *
 *@@added V0.9.19 (2002-03-28) [umoeller]
 *@@changed V0.9.19 (2002-06-14) [lafaix]: moved remove action to ProcessRemoveNotify
 */

STATIC MRESULT ProcessAddNotify(HWND hwndNotify,            // in: window to receive notifications
                                ULONG ulMessage,            // in: msg to post
                                PLINKLIST pllNotifies,      // in/out: list of CLIENTNOTIFY structs
                                PULONG pcNotifies)          // out: no. of notifications on the list
{
    MRESULT mrc = (MRESULT)FALSE;

    // check if the hook is running first;
    // or we'll get crashes if not
    // V0.9.17 (2002-02-05) [umoeller]
    if (G_pHookData)
    {
        // no need to lock here, since the daemon object
        // is the only one using this list
        PNOTIFYCLIENT p = NULL;

        PLISTNODE pNode = lstQueryFirstNode(pllNotifies),
                  pNodeFound = NULL;
        while (pNode)
        {
            PLISTNODE pNext = pNode->pNext;
            p = (PNOTIFYCLIENT)pNode->pItemData;
            if (p->hwndNotify == hwndNotify)
            {
                pNodeFound = pNode;
                break;
            }

            pNode = pNext;
        }

        // add watch:
        if (!pNodeFound)
            // we didn't already have one for this window:
            p = NEW(NOTIFYCLIENT);
        // else: p still points to the item found

        p->hwndNotify = hwndNotify;
        p->ulMessage = ulMessage;
        if (!pNodeFound)
            lstAppendItem(pllNotifies, p);

        mrc = (MPARAM)TRUE;

        // refresh flag for hook
        if (pcNotifies)
            *pcNotifies = lstCountItems(pllNotifies);

    } // end if (G_pHookData)

    return mrc;
}

/*
 *@@ ProcessRemoveNotify:
 *      removes the specified windows from the notify list.
 *
 *      Returns TRUE if successful, FALSE otherwise.
 *
 *@@added V0.9.19 (2002-06-14) [lafaix]
 */

STATIC MRESULT ProcessRemoveNotify(HWND hwndNotify,         // in: window to remove
                                   PLINKLIST pllNotifies,   // in/out: list of CLIENTNOTIFY structs
                                   PULONG pcNotifies)          // out: no. of notifications on the list
{
    MRESULT mrc = (MRESULT)FALSE;

    // check if the hook is running first;
    // or we'll get crashes if not
    if (G_pHookData)
    {
        // no need to lock here, since the daemon object
        // is the only one using this list
        PNOTIFYCLIENT p = NULL;

        PLISTNODE pNode = lstQueryFirstNode(pllNotifies),
                  pNodeFound = NULL;
        while (pNode)
        {
            PLISTNODE pNext = pNode->pNext;
            p = (PNOTIFYCLIENT)pNode->pItemData;
            if (p->hwndNotify == hwndNotify)
            {
                pNodeFound = pNode;
                break;
            }
            pNode = pNext;
        }

        if (pNodeFound)
        {
            lstRemoveNode(pllNotifies, pNodeFound);
            mrc = (MPARAM)TRUE;

            // refresh flag for hook
            if (pcNotifies)
                *pcNotifies = lstCountItems(pllNotifies);
        }
    } // end if (G_pHookData)

    return mrc;
}

/*
 *@@ ProcessMouseClicked:
 *      implementation for XDM_MOUSECLICKED in fnwpDaemonObject.
 *
 *@@added V0.9.19 (2002-03-28) [umoeller]
 */

STATIC VOID ProcessMouseClicked(MPARAM mp1, MPARAM mp2)
{
    PLISTNODE pNode = lstQueryFirstNode(&G_llClickWatches);
    while (pNode)
    {
        PLISTNODE pNext = pNode->pNext;
        PNOTIFYCLIENT p = (PNOTIFYCLIENT)pNode->pItemData;
        if (WinIsWindow(G_habDaemon, p->hwndNotify))
            WinPostMsg(p->hwndNotify,
                       p->ulMessage,
                       mp1,             // WM_BUTTONxxxx msg
                       mp2);            // POINTS mouse pos
        else
            // notify window no longer valid:
            lstRemoveNode(&G_llClickWatches,
                          pNode);
        pNode = pNext;
    }
}

#ifndef __NOMOVEMENT2FEATURES__

/*
 *@@ ProcessMovePtrToButton:
 *      implementation for XDM_MOVEPTRTOBUTTON in fnwpDaemonObject.
 *
 *@@added V0.9.19 (2002-03-28) [umoeller]
 */

STATIC VOID ProcessMovePtrToButton(HWND hwndObject, MPARAM mp1)
{
    HWND    hwndDefButton = (HWND)mp1;
    RECTL   rcl;

    // get window pos of the button to move to
    // and use its middle for the target to move to
    WinQueryWindowRect(hwndDefButton, &rcl);
    G_ptlMovingPtrEnd.x = (rcl.xLeft + rcl.xRight) / 2;
    G_ptlMovingPtrEnd.y = (rcl.yTop + rcl.yBottom) / 2;
    WinMapWindowPoints(hwndDefButton,
                       HWND_DESKTOP,
                       &G_ptlMovingPtrEnd,
                       1);

    if (G_pHookData->HookConfig.__ulAutoMoveDelay)
    {
        // delay configured:

        // get current mouse pointer so we can
        // calculate where we need to move to
        WinQueryPointerPos(HWND_DESKTOP, &G_ptlMovingPtrStart);
        // copy to current
        memcpy(&G_ptlMovingPtrNow, &G_ptlMovingPtrStart, sizeof(POINTL));

        // get current time in milliseconds so
        // we can calculate the offset based
        // on the time that has elapsed
        G_ulMSMovingPtrStart = doshQuerySysUptime();

        // reset last radius, in case animation is on
        // and was stopped before done last time
        // V0.9.15 (2001-08-26) [umoeller]
        G_lLastMovingPtrRadius = 0;

        // go (re)start the timer for moving
        G_ulMovingPtrTimer = WinStartTimer(G_habDaemon,
                                           hwndObject,
                                           TIMERID_MOVINGPTR,
                                           25);
    }
    else
        // no delay: go now
        WinSetPointerPos(HWND_DESKTOP,
                         G_ptlMovingPtrEnd.x,
                         G_ptlMovingPtrEnd.y);
}

#endif

/*
 *@@ ProcessStartApp:
 *      implementation for XDM_STARTAPP in fnwpDaemonObject.
 *
 *@@added V0.9.19 (2002-03-28) [umoeller]
 *@@changed V0.9.19 (2002-04-14) [umoeller]: fixed SWP_MINIMIZE
 */

STATIC MRESULT ProcessStartApp(MPARAM mp1, MPARAM mp2)
{
    APIRET arc;

    if (!(arc = DosGetSharedMem(mp2,
                                PAG_READ | PAG_WRITE)))
    {
        HAPP happ;
        if ((happ = WinStartApp((HWND)mp1,
                                (PPROGDETAILS)mp2,
                                ((PPROGDETAILS)mp2)->pszParameters,
                                NULL,
                                SAF_INSTALLEDCMDLINE)))
        {
            // app started OK:

            HSWITCH hsw;

            // return HAPP in PROGDETAILS
            ((PPROGDETAILS)mp2)->Length = happ;

            // VIO and fullscreen sessions keep ending up in the
            // background here, so bring these to the front
            if (    (((PPROGDETAILS)mp2)->progt.progc != PROG_PM)
                 // don't do this if minimized, or we'll restore the window
                 // or switch the screen group!
                 // V0.9.19 (2002-04-14) [umoeller]
                 && (!(((PPROGDETAILS)mp2)->swpInitial.fl & SWP_MINIMIZE))
               )
            {
                // try to find the switch entry, if there's one
                if (hsw = winhHSWITCHfromHAPP(happ))
                {
                    _Pmpf(("switching to hsw 0x%lX.", hsw));
                    WinSwitchToProgram(hsw);
                }
            }
        }
        else
        {
            // error: try to find out what happened
            PERRINFO pei;

            // unfortunately WinStartApp doesn't
            // return meaningful codes like DosStartSession, so
            // try to see what happened

            if (pei = WinGetErrorInfo(G_habDaemon))
            {

                switch (ERRORIDERROR(pei->idError))
                {
                    case PMERR_DOS_ERROR: //  (0x1200)
                        // this is totally meaningless... as far as I
                        // can see, there is no meaningful error info
                        // for this, I tried looking at the error info,
                        // but it's full of garbage...
                        arc = ERROR_FILE_NOT_FOUND;
                    break;

                    case PMERR_INVALID_APPL: //  (0x1530)
                            // Attempted to start an application whose type is not
                            // recognized by OS/2.
                            // This we get also if the executable doesn't exist...
                            // V0.9.18 (2002-03-27) [umoeller]
                        // arc = ERROR_INVALID_EXE_SIGNATURE;
                        arc = ERROR_FILE_NOT_FOUND;
                    break;

                    case PMERR_INVALID_PARAMETERS: //  (0x1208)
                            // An application parameter value is invalid for
                            // its converted PM type. For  example: a 4-byte
                            // value outside the range -32 768 to +32 767 cannot be
                            // converted to a SHORT, and a negative number cannot
                            // be converted to a ULONG or USHORT.
                        arc = ERROR_INVALID_DATA;
                    break;

                    case PMERR_STARTED_IN_BACKGROUND: //  (0x1532)
                            // The application started a new session in the
                            // background.
                        arc = ERROR_SMG_START_IN_BACKGROUND;
                    break;

                    case PMERR_INVALID_WINDOW: // (0x1206)
                            // The window specified with a Window List call
                            // is not a valid frame window.

                    default:
                        arc = ERROR_BAD_FORMAT;
                    break;
                }

                WinFreeErrorInfo(pei);
            }
            else
                arc = ERROR_INVALID_DATA;
        }

        DosFreeMem(mp2);

    } // if (!(arc = DosGetSharedMem(mp2,

    return (MRESULT)arc;
}

#define PN_WINDOWCHANGE 0
#define PN_ICONCHANGE   1

/*
 *@@ ProcessNotifies:
 *      called from ProcessWindowChange and
 *      ProcessIconChange to notify all clients
 *      on the notification list about a window
 *      list change. This will post pNotify->ulMessage
 *      plus ulMsgOffset to every window on the
 *      notify list, with the given MPARAM's.
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 *@@changed V0.9.19 (2002-06-15) [lafaix]: no longer removing inexistant list entries
 */

STATIC VOID ProcessNotifies(ULONG ulMsgOffset,
                            MPARAM mp1,
                            MPARAM mp2)
{
    PLISTNODE pNode = lstQueryFirstNode(&G_llWinlistWatches);
    while (pNode)
    {
        PLISTNODE pNext = pNode->pNext;
        PNOTIFYCLIENT pNotify = (PNOTIFYCLIENT)pNode->pItemData;
        if (WinIsWindow(G_habDaemon, pNotify->hwndNotify))
            WinPostMsg(pNotify->hwndNotify,
                       pNotify->ulMessage + ulMsgOffset,
                       mp1,
                       mp2);
        else
            // window is no longer valid:
            /* ProcessRemoveNotify(pNotify->hwndNotify,
                                &G_llWinlistWatches,
                                &G_pHookData->cWinlistWatches); */
            // hwndDaemonObject does list maintenance and
            // stops the winlist thread, so post instead
            // V0.9.19 (2002-06-15) [umoeller]
            WinPostMsg(G_pHookData->hwndDaemonObject,
                       XDM_REMOVEWINLISTWATCH,
                       (MPARAM)pNotify->hwndNotify,
                       NULL);

        pNode = pNext;
    }
}

/*
 *@@ ProcessWindowChange:
 *      implementation for XDM_WINDOWCHANGE in fnwpDaemonObject.
 *
 *      See pg_winlist.c for details.
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 *@@changed V0.9.19 (2002-06-18) [umoeller]: fixed missing WM_WINDOWPOSCHANGED
 *@@changed V1.0.8 (2008-05-26) [pr]: fixed re-parenting problem @@fixes 1065
 */

STATIC VOID ProcessWindowChange(MPARAM mp1, MPARAM mp2)
{
    BOOL fPost = TRUE;

    // refresh the list
    switch ((ULONG)mp2)
    {
        case WM_CREATE:
            fPost = pgrCreateWinInfo((HWND)mp1);
        break;

        case WM_DESTROY:
            pgrFreeWinInfo((HWND)mp1);
        break;

        case WM_SETWINDOWPARAMS:
        case WM_WINDOWPOSCHANGED:       // V0.9.19 (2002-06-18) [umoeller]
            if (!(fPost = pgrRefresh((HWND)mp1)))
                fPost = pgrCreateWinInfo((HWND)mp1);  // V1.0.8 (2008-05-26)
        break;
    }

    if (fPost)
    {
        // process notifies
        ProcessNotifies(PN_WINDOWCHANGE,
                        mp1,
                        mp2);
    }
}

/*
 *@@ ProcessIconChange:
 *      implementation for XDM_ICONCHANGE in fnwpDaemonObject.
 *
 *      See pg_winlist.c for details.
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 */

STATIC VOID ProcessIconChange(MPARAM mp1, MPARAM mp2)
{
    if (pgrIconChange((HWND)mp1, (HPOINTER)mp2))
    {
        // process notifies
        ProcessNotifies(PN_ICONCHANGE,
                        mp1,
                        mp2);
    }
}

/* ******************************************************************
 *
 *   Daemon object window
 *
 ********************************************************************/

/*
 *@@ fnwpDaemonObject:
 *      object window created in main(). This runs on thread 1.
 *
 *      This reacts to XDM_* messages (see xwphook.h)
 *      which are either posted from XWPHOOK.DLL for
 *      hotkeys, sliding focus, screen corner objects,
 *      or from XFLDR.DLL for daemon/hook configuration.
 *
 *      This object window is created by main() on startup
 *      and its handle is stored in the XWPGLOBALSHARED
 *      struct in shared memory so that XFLDR.DLL can see
 *      it.
 *
 *      Most of these messages are internal to the daemon
 *      and the hook and should not be posted externally.
 *      However, there are a couple of interface messages
 *      for setup and configuration:
 *
 *      --  XDM_HOOKINSTALL (install or deinstall hook);
 *
 *      --  XDM_DESKTOPREADY (desktop has been opened);
 *
 *      --  XDM_HOOKCONFIG (update hook configuration);
 *
 *      --  XDM_STARTSTOPPAGER;
 *
 *      --  XDM_RECOVERWINDOWS;
 *
 *      --  XDM_PAGERCONFIG;
 *
 *      --  XDM_HOTKEYSCHANGED;
 *
 *      --  XDM_ADDDISKWATCH, XDM_REMOVEDISKWATCH (add
 *          or remove a disk watch);
 *
 *      --  XDM_QUERYDISKS (get info about disks on the
 *          system).
 *
 *      --  XDM_ADDCLICKWATCH, XDM_REMOVECLICKWATCH
 *
 *      --  XDM_DISABLEHOTKEYSTEMP
 *
 *      --  XDM_STARTAPP
 *
 *      --  XDM_ADDWINLISTWATCH, XDM_REMOVEWINLISTWATCH
 *
 *@@changed V0.9.3 (2000-04-12) [umoeller]: added exception handling
 *@@changed V0.9.5 (2000-09-20) [pr]: fixed sliding focus bug
 *@@changed V0.9.9 (2001-04-05) [pr]: fixed trap if hook data was NULL
 *@@changed V0.9.14 (2001-08-01) [umoeller]: added drive monitor
 *@@changed V0.9.14 (2001-08-21) [umoeller]: added click watches support
 *@@changed V0.9.15 (2001-08-26) [umoeller]: move-ptr-to-button animation left circles on screen, fixed
 *@@changed V0.9.19 (2002-03-28) [umoeller]: moved more code out of main proc for better cache locality
 *@@changed V0.9.19 (2002-06-15) [lafaix]: added XDM_REMOVECLICKWATCH and XDM_REMOVEWINLISTWATCH
 *@@changed V0.9.20 (2002-07-19) [lafaix]: disabled auto-move if a mouse button is down
 */

MRESULT EXPENTRY fnwpDaemonObject(HWND hwndObject, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    TRY_LOUD(excpt1)
    {
        switch (msg)
        {
            /*
             * WM_TIMER:
             *
             */

            case WM_TIMER:
                mrc = ProcessTimer(hwndObject, msg, mp1, mp2);
            break;

            /*
             *@@ XDM_HOOKINSTALL:
             *      this must be sent while the daemon
             *      is running to install or deinstall
             *      the hook. This does not affect operation
             *      of the daemon, which will keep running.
             *
             *      This is used by the XWPSetup "Features"
             *      page and by XFLDR.DLL after main() has
             *      posted T1M_DAEMONREADY to the thread-1
             *      object window.
             *
             *      Parameters:
             *      -- BOOL mp1: if TRUE, the hook will be installed;
             *                   if FALSE, the hook will be deinstalled.
             *
             *      Return value:
             *      -- BOOL TRUE if the hook is now installed,
             *              FALSE if it is not.
             */

            case XDM_HOOKINSTALL:
                // _PmpfF(("got XDM_HOOKINSTALL (%d)", mp1));
                if (mp1)
                    // install the hook:
                    InstallHook();
                        // this sets the global pHookData pointer
                        // to the HOOKDATA in the DLL
                else
                    DeinstallHook();

                mrc = (MRESULT)G_pXwpGlobalShared->fAllHooksInstalled;
            break;

            /*
             *@@ XDM_DESKTOPREADY:
             *      this gets posted from fnwpFileObject after
             *      the WPS desktop frame has been opened
             *      so that the daemon/hook knows about the
             *      HWND of the WPS desktop. This is necessary
             *      for the sliding focus feature and for
             *      XPager.
             *
             *      Parameters:
             *      -- HWND mp1: desktop frame HWND.
             *      -- mp2: unused, always 0.
             *
             *@@changed V0.9.4 (2000-08-08) [umoeller]: fixed problems after Desktop restart
             */

            case XDM_DESKTOPREADY:
                // _Pmpf(("fnwpDaemonObject: got XDM_DESKTOPREADY (hwnd 0x%lX)", mp1));
                // _Pmpf(("    G_pHookData is: 0x%lX", G_pHookData));
                if (G_pHookData)
                {
                    G_pHookData->hwndWPSDesktop = (HWND)mp1;

                    // store the pid too
                    // V0.9.20 (2002-08-10) [umoeller]
                    WinQueryWindowProcess((HWND)mp1,
                                          &G_pHookData->pidWPS,
                                          NULL);

#ifndef __NOPAGER__
                    // give XPager a chance to recognize the Desktop
                    // V0.9.4 (2000-08-08) [umoeller]
                    pgrCreateWinInfo(G_pHookData->hwndWPSDesktop);
#endif
                }
            break;

            /*
             *@@ XDM_HOOKCONFIG:
             *      this gets _sent_ from XFLDR.DLL when
             *      daemon/hook settings have changed.
             *      This causes HOOKDATA.HOOKCONFIG to be
             *      updated from OS2.INI.
             *
             *      Parameters: none.
             *
             *      Return value:
             *      -- BOOL TRUE if successful.
             */

            case XDM_HOOKCONFIG:
                // _Pmpf(("fnwpDaemonObject: got XDM_HOOKCONFIG"));
                // load config from OS2.INI
                mrc = (MRESULT)LoadHookConfig(TRUE,     // hook
                                             FALSE);    // XPager

            break;

            /*
             *@@ XDM_NLSCHANGED:
             *      this message is sent (!) from XFLDR.DLL when
             *      the NLS DLL has changed.
             *      This gives the daemon a chance to update its
             *      NLS-dependant resources.
             *
             *@@added V1.0.0 (2002-09-15) [lafaix]
             */

            case XDM_NLSCHANGED:
                mrc = (MRESULT)LoadNLSResources();
            break;

#ifndef __NOPAGER__
            /*
             *@@ XDM_STARTSTOPPAGER:
             *      starts or stops XPager.
             *
             *      If (mp1 == TRUE), dmnStartXPager is called.
             *      If (mp1 == FALSE), dmnKillXPager is called.
             *
             *      Returns the new state of the pager as a
             *      BOOL.
             *
             *@@added V0.9.2 (2000-02-21) [umoeller]
             */

            case XDM_STARTSTOPPAGER:
                // _Pmpf(("fnwpDaemonObject: got XDM_STARTSTOPPAGER (%d)", mp1));
                if (mp1)
                    dmnStartXPager();
                else
                    dmnKillXPager(FALSE); // no notify

                if (G_pHookData)
                    mrc = (MRESULT)(G_pHookData->hwndPagerFrame != NULLHANDLE);
            break;

            /*
             *@@ XDM_RECOVERWINDOWS:
             *      recovers all windows to the screen.
             *
             *      Posted during XShutdown to make sure that
             *      window positions are not saved off screen.
             *
             *      Parameters:
             *
             *      BOOL mp1: if TRUE, only windows in the WPS process
             *           are recovered. If FALSE, all windows are
             *           recovered. This has been added with V0.9.20
             *           for the "Restart WPS" feature when processes
             *           should keep running on their own desktops.
             *
             *@@added V0.9.12 (2001-05-15) [umoeller]
             *@@changed V0.9.13 (2001-06-14) [umoeller]: fixed crash if hook wasn't active
             *@@changed V0.9.20 (2002-08-10) [umoeller]: added BOOL mp1
             */

            case XDM_RECOVERWINDOWS:
                if (G_pHookData)        // V0.9.13 (2001-06-14) [umoeller]
                    pgrRecoverWindows(G_habDaemon,
                                      (BOOL)mp1);       // fWPSOnly
            break;

            /*
             *@@ XDM_PAGERCONFIG:
             *      this gets _sent_ from XFLDR.DLL when
             *      XPager settings have changed.
             *      This causes the global PAGERCONFIG
             *      data to be updated from OS2.INI.
             *
             *      Parameters:
             *      -- ULONG mp1: any of the following flags:
             *          -- PGRCFG_REPAINT: repaint XPager client
             *          -- PGRCFG_REFORMAT: reformat whole window (e.g.
             *                  because Desktops have changed)
             *          -- PGRCFG_STICKIES: sticky windows have changed,
             *                  rescan window list.
             *
             *      Return value:
             *      -- BOOL TRUE if successful.
             */

            case XDM_PAGERCONFIG:
                // _Pmpf(("fnwpDaemonObject: got XDM_PAGERCONFIG"));
                // load config from OS2.INI
                mrc = (MRESULT)dmnLoadPagerSettings((ULONG)mp1);
            break;

            /*
             *@@ XDM_ADDTRANSIENTSTICKY:
             *      makes a windows temporarily (un)sticky.  The
             *      "sticky" state is not preserved upon WPS restart.
             *
             *      Parameters:
             *      -- HWND mp1: the top-level window to be made
             *          (un)sticky.
             *
             *@@added V0.9.20 (2002-07-25) [lafaix]
             */

            case XDM_TOGGLETRANSIENTSTICKY:
                mrc = (MRESULT)pgrToggleTransientSticky((HWND)mp1);
            break;

            case XDM_ISTRANSIENTSTICKY:
                // _Pmpf(("IsTransientSticky called for HWND %08x", (HWND)mp1));
                mrc = (MRESULT)pgrIsWindowTransientSticky((HWND)mp1);
            break;
#endif

            /*
             *@@ XDM_HOTKEYPRESSED:
             *      message posted by the hook when
             *      a global hotkey is pressed. We forward
             *      this message on to the XWorkplace thread-1
             *      object window so that the object can be
             *      started on thread 1 of the WPS. If we used
             *      WinOpenObject here, the object would be
             *      running on a thread other than thread 1,
             *      which is not really desirable.
             *
             *      Parameters:
             *
             *      -- ULONG mp1: hotkey identifier, probably WPS
             *         object handle.
             *
             *      -- mp2: not used, must be NULL.
             */

            case XDM_HOTKEYPRESSED:
                if (mp1)
                {
                    // _Pmpf(("Forwarding 0x%lX to 0x%lX",
                    //         mp1,
                    //         G_pXwpGlobalShared->hwndThread1Object));
                    // forward msg (cross-process post)
                    WinPostMsg(G_pXwpGlobalShared->hwndThread1Object,
                               T1M_OPENOBJECTFROMHANDLE,
                               mp1,
                               0);      // object hotkey, not screen border
                }
            break;

            /*
             *@@ XDM_HOTKEYSCHANGED:
             *      message posted by hifSetObjectHotkeys
             *      or hifSetFunctionKeys (XFLDR.DLL) when the
             *      list of global object hotkeys or the list
             *      of function keys has changed.
             *
             *      This is posted after the new lists have been
             *      written to OS2.INI so we can safely re-read
             *      this now and notify the hook of the change.
             *
             *      This is NOT posted at startup because we then
             *      read the lists automatically (XDM_HOOKINSTALL,
             *      InstallHook).
             *
             *      Parameters: none.
             *
             *@@changed V0.9.3 (2000-04-20) [umoeller]: this is now also posted for function keys.
             */

            case XDM_HOTKEYSCHANGED:
                LoadHotkeysForHook();
            break;

#ifndef __NOSLIDINGFOCUS__

            /*
             *@@ XDM_SLIDINGFOCUS:
             *      message posted by the hook when the mouse
             *      pointer has moved to a new frame window.
             *
             *      Parameters:
             *      -- HWND mp1: window which was under the mouse
             *              (qmsg.hwnd in hook)
             *      -- HWND mp2: highest parent of that window,
             *              child of PM desktop; this is probably
             *              a WC_FRAME, but doesn't have to be.
             *              If this is NULLHANDLE, the daemon is
             *              telling us to stop any timers which
             *              might be running.
             */

            case XDM_SLIDINGFOCUS:
                G_hwndSlidingUnderMouse = (HWND)mp1;
                G_hwndSliding2Activate = (HWND)mp2;
                if (G_pHookData->HookConfig.__ulSlidingFocusDelay)
                {
                    // delayed sliding focus:
                    if (G_ulSlidingFocusTimer)
                    {
                        WinStopTimer(G_habDaemon,
                                     hwndObject,
                                     TIMERID_SLIDINGFOCUS);
                        G_ulSlidingFocusTimer = NULLHANDLE;
                    }

                    if (G_hwndSliding2Activate)
                        G_ulSlidingFocusTimer = WinStartTimer(G_habDaemon,
                                                              hwndObject,
                                                              TIMERID_SLIDINGFOCUS,
                                                              G_pHookData->HookConfig.__ulSlidingFocusDelay);
                    // else: the daemon is telling us
                    // to stop timers which are running...
                }
                else
                    // immediately
                    // V0.9.5 (2000-09-20)[pr] sliding focus bug
                    if (G_hwndSliding2Activate)
                        ProcessSlidingFocus(G_hwndSlidingUnderMouse,
                                            G_hwndSliding2Activate);
            break;
#endif

            /*
             *@@ XDM_SLIDINGMENU:
             *      message posted by the hook when the mouse
             *      pointer has moved to a new menu item.
             *
             *      Parameters:
             *      -- mp1: mp1 of WM_MOUSEMOVE which was processed.
             *         If this is -1, the hook is telling us to
             *         stop any timer that might be running.
             *      -- mp2: unused.
             *
             *@@added V0.9.2 (2000-02-26) [umoeller]
             *@@changed V0.9.9 (2001-03-10) [umoeller]: added -1 checking
             */

            case XDM_SLIDINGMENU:
                G_SlidingMenuMp1Saved = mp1;

                // delayed sliding menu:
                if (G_ulSlidingMenuTimer)
                    WinStopTimer(G_habDaemon,
                                 hwndObject,
                                 TIMERID_SLIDINGMENU);
                if ((LONG)G_SlidingMenuMp1Saved != -1) // V0.9.9 (2001-03-10) [umoeller]
                    G_ulSlidingMenuTimer = WinStartTimer(G_habDaemon,
                                                         hwndObject,
                                                         TIMERID_SLIDINGMENU,
                                                         G_pHookData->HookConfig.ulSubmenuDelay);
            break;

            /*
             *@@ XDM_HOTCORNER:
             *      message posted by the hook when the mouse
             *      has reached one of the four corners of the
             *      screen ("hot corners"). The daemon then
             *      determines whether to open any object.
             *
             *      Parameters:
             *
             *      -- BYTE mp1: corner reached; see hook\xwphook.h
             *         for definitions
             *
             *      -- mp2: unused, always 0.
             *
             *@@changed V0.9.9 (2001-01-25) [lafaix]: XPager movements actions added
             *@@changed V0.9.14 (2001-08-20) [umoeller]: optimizations
             */

            case XDM_HOTCORNER:
                ProcessHotCorner(mp1);
            break;

            /*
             *@@ XDM_WMCHORDWINLIST:
             *      posted from the hook when WM_CHORD has been
             *      received and the window list should be displayed.
             *      We offload this to the daemon to process this
             *      asynchronously.
             *
             *      Parameters: none.
             *
             *@@added V0.9.4 (2000-06-14) [umoeller]
             */

            case XDM_WMCHORDWINLIST:
                ProcessChordWinList();
            break;

            /*
             *@@ XDM_BEGINSCROLL:
             *      posted from the hook when MB3 scroll is initiated.
             *
             *      Parameters:
             *      -- mp1: unused, always 0.
             *      -- mp2: unused, always 0.
             *
             *@@added V0.9.9 (2001-03-18) [lafaix]
             */

            case XDM_BEGINSCROLL:
                ProcessBeginScroll(hwndObject);
            break;

            /*
             *@@ XDM_ENDSCROLL:
             *      posted from the hook when MB3 scroll is ended.
             *
             *      Parameters:
             *      -- mp1: unused, always 0.
             *      -- mp2: unused, always 0.
             *
             *@@added V0.9.9 (2001-03-18) [lafaix]
             */

            case XDM_ENDSCROLL:
                ProcessEndScroll(hwndObject);
            break;

            /*
             *@@ XDM_SETPOINTER:
             *      posted from the hook when MB3 scroll is active and
             *      the mouse has moved.
             *
             *      Parameters:
             *      -- SHORT1(mp1): current X mouse position on screen
             *      -- SHORT2(mp1): current Y mouse position on screen
             *      -- mp2: unused, always 0.
             *
             *@@added V0.9.9 (2001-03-18) [lafaix]
             */

            case XDM_SETPOINTER:
                ProcessSetPointer(mp1);
            break;

            /*
             *@@ XDM_ADDDISKWATCH:
             *      adds a logical disk to the list of
             *      disks to be watched by the daemon.
             *
             *      This must be sent, not posted, to
             *      the daemon.
             *
             *      Parameters:
             *
             *      -- PADDDISKWATCH mp1: pointer to a
             *         ADDDISKWATCH structure containing
             *         information about the caller and
             *         the disk to be watched.
             *
             *         Warning: If this message is sent
             *         from another process, that struct
             *         must be in shared memory.
             *
             *      -- mp2: not used, must be 0.
             *
             *      Returns TRUE if the disk was added
             *      successfully.
             *
             *      After this has returned TRUE, ADDDISKWATCH.hwndNotify
             *      will be posted ADDDISKWATCH.ulMessage every time
             *      the free space on the disk changes. That message
             *      will receive the following parameters:
             *
             *      --  ULONG mp1: ulLogicalDrive which changed.
             *
             *      --  ULONG mp2: new free space on the disk in KB.
             *
             *@@added V0.9.14 (2001-08-01) [umoeller]
             */

            case XDM_ADDDISKWATCH:
            {
                PADDDISKWATCH p;
                if (p = (PADDDISKWATCH)mp1)
                {
                    if (dmnAddDiskfreeMonitor(p->ulLogicalDrive,
                                              p->hwndNotify,
                                              p->ulMessage))
                        mrc = (MPARAM)TRUE;
                }
            }
            break;

            /*
             *@@ XDM_REMOVEDISKWATCH:
             *      removes a disk watch again. The reverse
             *      to XDM_ADDDISKWATCH.
             *
             *      Parameters:
             *
             *      -- HWND mp1: hwndNotify that was specified
             *         with XDM_ADDDISKWATCH.
             *
             *      -- ULONG mp2: the logical drive for which
             *         the disk watch is to be removed. If -1,
             *         all watches for hwndNotify are removed.
             *
             *@@added V0.9.14 (2001-08-01) [umoeller]
             */

            case XDM_REMOVEDISKWATCH:
                if (dmnAddDiskfreeMonitor((ULONG)mp2,   // log. drive, can be -1
                                          (HWND)mp1,
                                          -1))          // remove
                    mrc = (MPARAM)TRUE;
            break;

            /*
             *@@ XDM_QUERYDISKS:
             *      checks one or all available disks on the system
             *      and returns information about each of these.
             *
             *      Parameters:
             *
             *      -- ULONG mp1: logical drive to check, or -1
             *         for all drives.
             *
             *      -- PXDISKINFO mp2: ptr to an XDISKINFO structure
             *         (filesys\disk.h) receiving the information.
             *         See XDISKINFO for details.
             *
             *         If mp1 is -1, this must point to an array of
             *         26 XDISKINFO structures, each of which
             *         receives information about one disk on the
             *         system.
             *
             *         Warning: If this message is sent
             *         from another process, that struct
             *         or array must be in shared memory.
             *
             *      Returns TRUE on success.
             *
             *@@added V0.9.14 (2001-08-01) [umoeller]
             */

            case XDM_QUERYDISKS:
                mrc = (MPARAM)dmnQueryDisks((ULONG)mp1,
                                            mp2);
            break;

            /*
             *@@ XDM_ADDCLICKWATCH:
             *      adds a "mouse click watch" to the daemon.
             *      This way any window can request to be
             *      notified whenever a mouse click occurs
             *      anywhere on the system.
             *
             *      This must be sent, not posted, to
             *      the daemon.
             *
             *      Parameters:
             *
             *      -- HWND mp1: window to be notified on
             *         mouse clicks.
             *
             *      -- ULONG mp2: message to be posted on
             *         mouse clicks to that window.
             *
             *      That message will be posted on each mouse
             *      click on the system with the following
             *      parameters:
             *
             *      -- ULONG mp1: mouse message for the click,
             *         being one of WM_BUTTONxyyyy, with X
             *         being 1, 2, or 3, and yyy being either
             *         DOWN, UP, or DBLCLK.
             *
             *      -- POINTS mp2: mouse pointer position,
             *         as in mp1 of the WM_BUTTONxxxx messages.
             *
             *      Returns TRUE on success.
             *
             *@@added V0.9.14 (2001-08-21) [umoeller]
             *@@changed V0.9.17 (2002-02-05) [umoeller]: fixed crash if hook not running
             *@@changed V0.9.19 (2002-06-14) [lafaix]: removed the -1 mp2 stuff
             */

            case XDM_ADDCLICKWATCH:
                if (G_pHookData)
                    mrc = ProcessAddNotify((HWND)mp1,
                                           (ULONG)mp2,
                                           &G_llClickWatches,
                                           &G_pHookData->cClickWatches);
            break;

            /*
             *@@ XDM_REMOVECLICKWATCH:
             *      removes a "mouse click watch" from the daemon.
             *      This way any window can remove itself from the
             *      list of windows notified whenever a mouse click
             *      occurs anywhere on the system.
             *
             *      This must be sent, not posted, to
             *      the daemon.
             *
             *      Parameters:
             *
             *      -- HWND mp1: window to be notified on
             *         mouse clicks.
             *
             *      Returns TRUE on success.
             *
             *@@added V0.9.19 (2002-06-14) [lafaix]
             */

            case XDM_REMOVECLICKWATCH:
                if (G_pHookData)
                    mrc = ProcessRemoveNotify((HWND)mp1,
                                              &G_llClickWatches,
                                              &G_pHookData->cClickWatches);
            break;

            /*
             *@@ XDM_MOUSECLICKED:
             *      posted by the hook for every single mouse
             *      click, if we have any click watches.
             *
             *@@added V0.9.14 (2001-08-21) [umoeller]
             */

            case XDM_MOUSECLICKED:
                ProcessMouseClicked(mp1, mp2);
            break;

#ifndef __NOMOVEMENT2FEATURES__

            /*
             *@@ XDM_MOVEPTRTOBUTTON:
             *      posted by the hook if "move ptr to button"
             *      is enabled and a dlg popped up which has
             *      a default button, whose HWND is passed in
             *      mp1.
             *
             *      Parameters:
             *
             *      --  HWND mp1: default button to move to.
             *
             *@@added V0.9.14 (2001-08-21) [umoeller]
             *@@changed V0.9.20 (2002-07-19) [lafaix]: don't move if a button is down
             */

            case XDM_MOVEPTRTOBUTTON:
                if (    ((WinGetKeyState(HWND_DESKTOP, VK_BUTTON1) & 0x8000) == 0)
                     && ((WinGetKeyState(HWND_DESKTOP, VK_BUTTON2) & 0x8000) == 0)
                     && ((WinGetKeyState(HWND_DESKTOP, VK_BUTTON3) & 0x8000) == 0)
                   )
                    ProcessMovePtrToButton(hwndObject, mp1);
            break;
#endif

            /*
             *@@ XDM_DISABLEHOTKEYSTEMP:
             *      this message allows an application to
             *      temporarily disable and re-enable object
             *      hotkeys. This is used by the "Icon" page
             *      to disable the hotkeys while the "Hotkeys"
             *      entry field has the focus.
             *
             *      Parameters:
             *
             *      --  BOOL mp1: if TRUE, hotkeys are disabled;
             *          if FALSE, hotkeys are enabled again.
             *
             *@@added V0.9.16 (2001-12-08) [umoeller]
             */

            case XDM_DISABLEHOTKEYSTEMP:
                if (G_pHookData)
                    G_pHookData->fHotkeysDisabledTemp = (BOOL)mp1;
            break;

            /*
             *@@ XDM_STARTAPP:
             *      sent from XFLDR.DLL to have WinStartApp
             *      called from the Daemon process to avoid
             *      the hangs that occur when WinStartApp
             *      is used within the Workplace process.
             *
             *      Parameters:
             *
             *      --  HWND mp1: hwndNotify for WinStartApp.
             *
             *      --  PPROGDETAILS mp2: block of gettable
             *          shared memory containing all the
             *          program details for WinStartApp
             *          (as allocated by appBuildProgDetails).
             *
             *      Returns APIRET.
             *
             *      If NO_ERROR is returned, the PROGDETAILS.ulLength
             *      field is set to the HAPP that was started.
             *
             *@@added V0.9.19 (2002-03-28) [umoeller]
             */

            case XDM_STARTAPP:
                mrc = ProcessStartApp(mp1, mp2);
            break;

            /*
             *@@ XDM_ADDWINLISTWATCH:
             *      similar to XDM_ADDCLICKWATCH, adds a
             *      "window list watch" to the daemon.
             *      This way any window can request to be
             *      notified whenever the window list changes.
             *
             *      This must be sent, not posted, to
             *      the daemon.
             *
             *      Parameters:
             *
             *      -- HWND mp1: window to be notified on
             *         window list changes.
             *
             *      -- ULONG mp2: messages to be posted on
             *         window list changes. Note that the
             *         window specified in mp1 will receive
             *         two kind of messages: _this_ message
             *         as well as this message + 1 (see below).
             *
             *      The message specified in mp2 will be posted
             *      every time the window list changes with the
             *      following parameters:
             *
             *      --  HWND mp1: the window that changed.
             *
             *      --  ULONG mp2: a message value indicating
             *          the type of change. This will be one
             *          of the following:
             *
             *          --  WM_CREATE: (HWND)mp1 was created.
             *          --  WM_DESTROY: (HWND)mp1 was destroyed.
             *          --  WM_SETWINDOWPARAMS: (HWND)mp1 had its
             *              title or switch list entry changed.
             *          --  WM_WINDOWPOSCHANGED: (HWND)mp1 had its
             *              position changed.
             *          --  WM_ACTIVATE: (HWND)mp1 was activated
             *              or deactivated.
             *
             *      In addition, if a frame icon changes for an
             *      entry in the private window list, the message
             *      (ULONG)mp2 + 1 will be posted with the following
             *      parameters:
             *
             *      --  HWND mp1: frame window that changed.
             *
             *      --  HPOINTER mp2: new icon for the frame.
             *
             *      Returns TRUE on success.
             *
             *@@added V0.9.19 (2002-05-28) [umoeller]
             */

            case XDM_ADDWINLISTWATCH:
                if (    (G_pHookData)
                     && (mrc = ProcessAddNotify((HWND)mp1,
                                                (ULONG)mp2,
                                                &G_llWinlistWatches,
                                                &G_pHookData->cWinlistWatches))
                   )
                {
                    if (G_pHookData->cWinlistWatches == 1)
                    {
                        _Pmpf((__FUNCTION__ ": initializing winlist"));
                        // we just added the first winlist watch:
                        // then scan the full switch list so we
                        // don't produce a flurry of messages on
                        // the winlist thread
                        pgrBuildWinlist();

                        // and start the winlist thread if it's not running
                        thrCreate(&G_tiWinlistThread,
                                  fntWinlistThread,
                                  NULL,
                                  "WinList",
                                  THRF_PMMSGQUEUE,
                                  0);
                    }
                }
            break;

            /*
             *@@ XDM_REMOVEWINLISTWATCH:
             *      similar to XDM_REMOVECLICKWATCH, removes a
             *      "window list watch".
             *      This way any window can request not to be
             *      notified anymore whenever the window list changes.
             *
             *      This must be sent, not posted, to
             *      the daemon.
             *
             *      Parameters:
             *
             *      -- HWND mp1: window to be notified on
             *         mouse clicks.
             *
             *      Returns TRUE on success.
             *
             *@@added V0.9.19 (2002-06-14) [lafaix]
             */

            case XDM_REMOVEWINLISTWATCH:
                if (    (G_pHookData)
                     && (mrc = ProcessRemoveNotify((HWND)mp1,
                                                   &G_llWinlistWatches,
                                                   &G_pHookData->cWinlistWatches))
                   )
                {
                    if (!G_pHookData->cWinlistWatches)
                    {
                        // we just removed the last winlist watch:
                        // stop winlist thread
                        G_tiWinlistThread.fExit = TRUE;
                    }
                }
            break;

            /*
             *@@ XDM_WINDOWCHANGE:
             *      posted by ProcessMsgsForWinlist in the hook
             *      or the window list thread (fntWinlistThread)
             *      whenever important window changes are detected.
             *      This applies only to the msgs listed below.
             *
             *      Parameters:
             *
             *      --  HWND mp1: window for which the msg came in.
             *
             *      --  ULONG mp2: message that came in.
             *
             *      mp2 will be one of the following messages:
             *
             *      --  WM_CREATE
             *
             *      --  WM_DESTROY
             *
             *      --  WM_SETWINDOWPARAMS
             *
             *      --  WM_WINDOWPOSCHANGED
             *
             *      --  WM_ACTIVATE
             *
             *@@added V0.9.19 (2002-05-07) [umoeller]
             *@@changed V0.9.20 (2002-07-19) [pr]: prevent trap when unhooking
             */

            case XDM_WINDOWCHANGE:
                if (    G_pHookData
                     && G_pHookData->cWinlistWatches
                   )
                    ProcessWindowChange(mp1, mp2);
            break;

            /*
             *@@ XDM_ICONCHANGE:
             *      posted by ProcessMsgsForWinlist in the hook
             *      or the window list thread (fntWinlistThread)
             *      whenever a frame icon appears to have changed.
             *
             *      Parameters:
             *
             *      --  HWND mp1: window whose icon changed.
             *
             *      --  HPOINTER mp2: new icon.
             *
             *@@added V0.9.19 (2002-05-28) [umoeller]
             *@@changed V0.9.20 (2002-07-19) [pr]: prevent trap when unhooking
             */

            case XDM_ICONCHANGE:
                if (    G_pHookData
                     && G_pHookData->cWinlistWatches
                   )
                    ProcessIconChange(mp1, mp2);
            break;

            /*
             *@@ XDM_QUERYWINLIST:
             *      causes the daemon to build a SWBLOCK
             *      buffer with the current window list
             *      in shared memory and give it to the
             *      caller, who must specify his pid in
             *      mp1.
             *
             *      This must be sent, not posted. Also
             *      this must not be sent from within
             *      the daemon because then giving the
             *      shared memory probably won't work.
             *
             *      Parameters:
             *
             *      --  PID mp1: pid of sender. This is
             *          required because we use
             *          DosGiveSharedMem here.
             *
             *      --  mp2: unused.
             *
             *      Returns:
             *
             *      PSWBLOCK with the full window list,
             *      or NULL on errors. The sender must
             *      issue DosFreeMem on the pointer that
             *      is returned.
             *
             *      Specialties:
             *
             *      --  PSWBLOCK->cswentries contains
             *          the no. of items in the array.
             *          This better be respected or
             *          you'll bump behind the shared
             *          memory that was allocated.
             *
             *      --  Each SWENTRY.SWCNTRL.hwndIcon is
             *          set to the frame icon that has
             *          been queried by the background
             *          threads in the daemon so that the
             *          caller is saved from having to
             *          send WM_QUERYICON to each frame,
             *          which can block with misbehaving
             *          apps like PMMail.
             *
             *@@added V0.9.19 (2002-05-28) [umoeller]
             */

            case XDM_QUERYWINLIST:
                mrc = (MRESULT)pgrQueryWinList((ULONG)mp1);
            break;

            default:
                mrc = WinDefWindowProc(hwndObject, msg, mp1, mp2);
        }
    }
    CATCH(excpt1)
    {
        mrc = 0;
    } END_CATCH();

    return mrc;
}

/* ******************************************************************
 *
 *   main()
 *
 ********************************************************************/

/*
 *@@ TerminateExcHandler:
 *      new extra exception handler which intercepts
 *      all "termination" notifications so that the
 *      daemon can properly clean up if it gets killed.
 *
 *      There seems to be a bug in the PM cleanup routines
 *      for processes which have registered a global PM
 *      hook. It looks like there is some internal table
 *      overflowing if such a process gets killed... at
 *      least such a process cannot be started and killed
 *      more than about 20-30 times before the entire PM
 *      hangs itself up with some strange exception.
 *
 *      This behavior is not only displayed with the XWP
 *      daemon... I have tested this with other global
 *      PM hooks, and they all hang up the system if they
 *      are started and killed many times.
 *
 *      As a result, we register this exception handler
 *      in main() which will do a longjmp back to main()
 *      for any process termination exception. main()
 *      will then deregister the hook and do other
 *      cleanup in the proper order. It looks like this
 *      fixes the PM cleanup problem, or at least gives
 *      us a few more restarts.
 *
 *      This replaces the exit list that was present
 *      previously.
 *
 *@@added V0.9.11 (2001-04-25) [umoeller]
 */

ULONG _System TerminateExcHandler(PEXCEPTIONREPORTRECORD pReportRec,
                                  PEXCEPTIONREGISTRATIONRECORD2 pRegRec2,
                                  PCONTEXTRECORD pContextRec,
                                  PVOID pv)
{
    // V1.0.4 (2005-10-09) [pr]: Optimized
    if (pReportRec->fHandlerFlags & (EH_EXIT_UNWIND | EH_UNWINDING | EH_NESTED_CALL))
       return XCPT_CONTINUE_SEARCH;

    switch (pReportRec->ExceptionNum)
    {
        case XCPT_PROCESS_TERMINATE:
        case XCPT_ASYNC_PROCESS_TERMINATE:
            // jump back to the point in main() which will
            // then clean up and properly exit
            longjmp(pRegRec2->jmpThread, pReportRec->ExceptionNum);
        break;

        case XCPT_ACCESS_VIOLATION:
        case XCPT_INTEGER_DIVIDE_BY_ZERO:
        case XCPT_ILLEGAL_INSTRUCTION:
        case XCPT_PRIVILEGED_INSTRUCTION:
        case XCPT_INVALID_LOCK_SEQUENCE:
        case XCPT_INTEGER_OVERFLOW:
        {
            // "real" exceptions:
            FILE *file = dmnExceptOpenLogFile();
            // write error log
            excExplainException(file,
                                "excHandlerLoud",
                                pReportRec,
                                pContextRec);
            fclose(file);

            longjmp(pRegRec2->jmpThread, pReportRec->ExceptionNum);
        }
        break;
    }

    return XCPT_CONTINUE_SEARCH;
}

/*
 *@@ main:
 *      program entry point.
 *
 *      This first checks for the "-D" parameter, which
 *      is a security measure so that this program does
 *      not get started by curious XWorkplace users.
 *
 *      Then, we access the XWPGLOBALSHARED structure which
 *      has been allocated by initMain (shared\init.c).
 *
 *      We then create our object window (fnwpDaemonObject),
 *      to which the hook will post messages.
 *
 *      This does not install the hooks yet, but posts
 *      T1M_DAEMONREADY to pXwpGlobalShared->hwndThread1Object only.
 *      To actually install the hook, XFLDR.DLL then posts
 *      XDM_HOOKINSTALL to the daemon object window.
 *
 *@@changed V0.9.7 (2001-01-20) [umoeller]: now using higher priority
 *@@changed V0.9.9 (2001-03-18) [lafaix]: loads pointers
 *@@changed V0.9.11 (2001-04-25) [umoeller]: added termination exception handler for proper hook cleanup
 *@@changed V0.9.11 (2001-04-25) [umoeller]: reordered all this code for readability
 *@@changed V0.9.19 (2002-05-23) [umoeller]: fixed startup directory
 *@@changed V1.0.0 (2002-08-21) [umoeller]: fixed hangs on default OS/2 shutdown
 */

int main(int argc, char *argv[])
{
    APIRET arc = NO_ERROR;

    if (    (G_habDaemon = WinInitialize(0))
         && (G_hmqDaemon = WinCreateMsgQueue(G_habDaemon, 0))
         // get our process ID
         && (G_pidDaemon = doshMyPID())
       )
    {
        HMTX    hmtx;

        // set up exception handler callbacks
        excRegisterHooks(dmnExceptOpenLogFile,
                         dmnExceptExplain,
                         dmnExceptError,
                         TRUE);     // beeps

        // disable hard errors V0.9.14 (2001-08-01) [umoeller]
        DosError(FERR_DISABLEHARDERR | FERR_ENABLEEXCEPTION);

        // cancel shutdown requests to avoid hangs with
        // default OS/2 shutdown
        // V1.0.0 (2002-08-21) [umoeller]
        WinCancelShutdown(G_hmqDaemon, TRUE);

        // initialize click-watch list V0.9.14 (2001-08-21) [umoeller]
        lstInit(&G_llClickWatches, TRUE);
        lstInit(&G_llWinlistWatches, TRUE);

        // check safety net parameter "-D"
        if (    (argc != 2)
             || (strcmp(argv[1], "-D"))
           )
        {
            WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                          "Hi there. Thanks for your interest in the XWorkplace Daemon, "
                          "but this program is not intended to be started manually, "
                          "but only automatically by XWorkplace when the Desktop starts up.",
                          "XWorkplace Daemon",
                          0,
                          MB_MOVEABLE | MB_CANCEL | MB_ICONHAND);
        }
        // check the daemon one-instance semaphore, which
        // we create just for testing that the daemon is
        // started only once
        else if ((arc = DosCreateMutexSem(IDMUTEX_ONEINSTANCE,
                                          &hmtx,
                                          DC_SEM_SHARED,
                                          TRUE)))     // owned!!
        {
            // mutex creation failed: another instance of
            // XWPDAEMN.EXE is running, so complain
            WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                          "Another instance of XWPDAEMN.EXE is already running. "
                          "This instance will terminate now.",
                          "XWorkplace Daemon",
                          0,
                          MB_MOVEABLE | MB_CANCEL | MB_ICONHAND);
        }
        else
        {
            // semaphore successfully created: this means
            // that no other instance of XWPDAEMN.EXE is running

            // access the shared memory allocated by
            // XFLDR.DLL; this should always work:
            // a) if the daemon has been started during first
            //    Desktop startup, initMain has
            //    allocated the memory before starting the
            //    daemon;
            // b) at any time later (if the daemon is restarted
            //    manually), we have no problem either
            if ((arc = DosGetNamedSharedMem((PVOID*)&G_pXwpGlobalShared,
                                            SHMEM_XWPGLOBAL,
                                            PAG_READ | PAG_WRITE)))
            {
                WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                              "The XWorkplace Daemon failed to access the data block shared with "
                              "the Workplace process. The daemon will terminate.",
                              "XWorkplace Daemon",
                              0,
                              MB_MOVEABLE | MB_CANCEL | MB_ICONHAND);
            }
            else
            {
                ULONG ulBootDrive;

                // OK:
                G_pXwpGlobalShared->fAllHooksInstalled = FALSE;
                        // V0.9.11 (2001-04-25) [umoeller]

                // change current directory to root directory on
                // drive where XWorkplace is installed, and then
                // set current disk to boot disk; otherwise the
                // user will end up in XWP's "bin" directory when
                // starting a command prompt and changing disks
                // since we are now using the Daemon for program
                // startup V0.9.19 (2002-05-23) [umoeller]
                DosSetCurrentDir("\\");
                DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                                &ulBootDrive,
                                sizeof(ulBootDrive));
                DosSetDefaultDisk(ulBootDrive);

#ifndef __NOPAGER__
                pgrInit();
#endif

                G_hptrDaemon = WinLoadPointer(HWND_DESKTOP,
                                              NULLHANDLE,
                                              1);

                // preload MB3 scroll pointers V0.9.9 (2001-03-18) [lafaix]
                G_hptrNESW = WinLoadPointer(HWND_DESKTOP, NULLHANDLE, 124);
                G_hptrNS   = WinLoadPointer(HWND_DESKTOP, NULLHANDLE, 125);
                G_hptrEW   = WinLoadPointer(HWND_DESKTOP, NULLHANDLE, 121);
                G_hptrN    = WinLoadPointer(HWND_DESKTOP, NULLHANDLE, 122);
                G_hptrNE   = WinLoadPointer(HWND_DESKTOP, NULLHANDLE, 123);
                G_hptrE    = WinLoadPointer(HWND_DESKTOP, NULLHANDLE, 120);
                G_hptrSE   = WinLoadPointer(HWND_DESKTOP, NULLHANDLE, 128);
                G_hptrS    = WinLoadPointer(HWND_DESKTOP, NULLHANDLE, 127);
                G_hptrSW   = WinLoadPointer(HWND_DESKTOP, NULLHANDLE, 129);
                G_hptrW    = WinLoadPointer(HWND_DESKTOP, NULLHANDLE, 130);
                G_hptrNW   = WinLoadPointer(HWND_DESKTOP, NULLHANDLE, 126);

                // create the object window
                WinRegisterClass(G_habDaemon,
                                 (PSZ)WNDCLASS_DAEMONOBJECT,
                                 (PFNWP)fnwpDaemonObject,
                                 0,                  // class style
                                 0);                 // extra window words

                if (G_pXwpGlobalShared->hwndDaemonObject
                    = WinCreateWindow(HWND_OBJECT,
                                      (PSZ)WNDCLASS_DAEMONOBJECT,
                                      (PSZ)"",
                                      0,
                                      0,0,0,0,
                                      0,
                                      HWND_BOTTOM,
                                      0,
                                      NULL,
                                      NULL))
                {
                    EXCEPTSTRUCT    TermExcptStruct = {0};
                    THREADINFO      tiDiskWatch;
                    QMSG    qmsg;

                    // create drive monitor thread
                    // V0.9.14 (2001-08-01) [umoeller]
                    thrCreate(&tiDiskWatch,
                              fntDiskWatch,
                              NULL,
                              "DiskWatch",
                              THRF_WAIT_EXPLICIT,
                              0);

                    // create watchdog threads
                    // V1.0.0 (2002-08-12) [umoeller]
                    // dmnStartWatchdog();

                    // post msg to XFLDR.DLL thread-1 object window
                    // that we're ready, which will in turn send
                    // XDM_HOOKINSTALL
                    WinPostMsg(G_pXwpGlobalShared->hwndThread1Object,
                               T1M_DAEMONREADY,
                               0,
                               0);

                    // register special exception handler just for
                    // thread termination (see TerminateExcHandler)
                    // V0.9.11 (2001-04-25) [umoeller]
                    TermExcptStruct.RegRec2.pfnHandler = (PFN)TerminateExcHandler;
                    DosSetExceptionHandler(
                        (PEXCEPTIONREGISTRATIONRECORD)&TermExcptStruct.RegRec2);

                    if (!(TermExcptStruct.ulExcpt = setjmp(TermExcptStruct.RegRec2.jmpThread)))
                    {
                        // no termination exception:

                        /*
                         *  standard PM message loop
                         *  (we stay in here all the time)
                         */

                        while (WinGetMsg(G_habDaemon, &qmsg, 0, 0, 0))
                            WinDispatchMsg(G_habDaemon, &qmsg);
                    }
                    // else: exception occurred...

                    DosUnsetExceptionHandler((PEXCEPTIONREGISTRATIONRECORD)&TermExcptStruct.RegRec2);

                    // we get here if
                    // a) we received WM_QUIT (can't see why this would happen);
                    // b) the process got killed, which is intercepted in
                    //    TerminateExcHandler (setjmp put us to the above sequence)
                    // c) xwpdaemn trapped somewhere on thread 1, which is
                    //    handled by TerminateExcHandler too

                    // so kill the hook again
                    // (cleanup must be in proper order to avoid PM hangs)
                    DeinstallHook();

#ifdef __DEBUG__
                    DosBeep(2000, 100);
#endif

                    winhSleep(500);      // sleep a bit to allow PM processes to exit the hook DLL
                                         // V1.0.2 (2003-02-07) [umoeller]

#ifdef __DEBUG__
                    DosBeep(1000, 100);
#endif

                    WinDestroyWindow(G_pXwpGlobalShared->hwndDaemonObject);
                    G_pXwpGlobalShared->hwndDaemonObject = NULLHANDLE;
                } // end if (G_pXwpGlobalShared->hwndDaemonObject)
            } // end if DosGetNamedSharedMem
        } // end DosCreateMutexSem...

        WinDestroyMsgQueue(G_hmqDaemon);
        WinTerminate(G_habDaemon);
    }

    return 0;
}

