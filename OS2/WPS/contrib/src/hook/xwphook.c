
/*
 *@@sourcefile xwphook.c:
 *      this has the all the code for XWPHOOK.DLL, which implements
 *      all the XWorkplace PM system hooks, as well as a few exported
 *      interface functions which may only be called by the XWorkplace
 *      daemon process (XWPDAEMN.EXE).
 *
 *      There are four hooks at present:
 *
 *      --  hookSendMsgHook
 *
 *      --  hookLockupHook
 *
 *      --  hookInputHook
 *
 *      --  hookPreAccelHook
 *
 *      See the remarks in the respective functions.
 *
 *      I have tried to design the hooks for clarity and lucidity with
 *      respect to interfaces and configuration --
 *      as if that was possible with system hooks at all, since they
 *      are so messy by nature.
 *
 *      With V0.9.12, the hook code was spread across several files
 *
 *      1)  to group the actual hook functions together for a better
 *          code path,
 *
 *      2)  to make all this more readable.
 *
 *      A couple words of wisdom about the hook code:
 *
 *      --  System hooks are possibly running in the context of each PM
 *          thread which is processing a message -- that is, possibly each
 *          thread on the system which has created a message queue.
 *
 *          As a result, great care must be taken when accessing global
 *          data, and one must keep in mind for each function which thread
 *          it might run in, or otherwise we'd be asking for system hangs.
 *          Don't expect OS/2 to handle exceptions properly if all PM
 *          programs crash at the same time.
 *
 *          It _is_ possible to access static global variables of
 *          this DLL from every function, because the DLL is
 *          linked with the STATIC SHARED flags (see xwphook.def).
 *          This means that the single data segment of the DLL
 *          becomes part of each process which is using the DLL
 *          (that is, each PM process on the system). We use this
 *          for the HOOKDATA structure, which is a static global
 *          structure in the DLL's shared data segment and is
 *          returned to the daemon when it calls hookInit in the DLL.
 *
 *          HOOKDATA includes a HOOKCONFIG structure. As a result,
 *          to configure the basic hook flags, the daemon can simply
 *          modify the fields in the HOOKDATA structure to configure
 *          the hook's behavior.
 *
 *          It is however _not_ possible to use malloc() to allocate
 *          global memory and use it between several calls of the
 *          hooks, because that memory will belong to one process
 *          only, even if the pointer is stored in a global DLL
 *          variable. The next time the hook gets called and accesses
 *          that memory, some fairly random application will crash
 *          (the one the hook is currently called for), or the system
 *          will hang completely.
 *
 *          For this reason, the hooks use another block of shared
 *          memory internally, which is protected by a named mutex
 *          semaphore, for storing the list of object hotkeys (which
 *          is variable in size). This block is (re)allocated in
 *          hookSetGlobalHotkeys and requested in the hook when
 *          WM_CHAR comes in. The mutex is necessary because when
 *          hotkeys are changed, the daemon changes the structure by
 *          calling hookSetGlobalHotkeys.
 *
 *      --  The exported hook* functions may only be used by one
 *          single process. It is not possible for one process to
 *          call hookInit and for another to call another hook*
 *          function, because the shared memory which is allocated
 *          here must be "owned" by a certain process and would not be
 *          properly freed if several processes called the hook
 *          interfaces.
 *
 *          So, per definition, it is only the XWorkplace daemon's
 *          (XWPDAEMN.EXE) responsibility to interface and configure
 *          the hook. Theoretically, we could have done this from
 *          XFLDR.DLL in PMSHELL.EXE also, but using the daemon has a
 *          number of advantages to that, since it can be terminated
 *          and restarted independently of the WPS (see xwpdaemn.c
 *          for details). Also, if something goes wrong, it's only
 *          the daemon which crashes, and not the entire WPS.
 *
 *          When any configuration data changes which is of importance
 *          to the hook, XFLDR.DLL writes this data to OS2.INI and
 *          then posts the daemon a message. The daemon then reads
 *          the new data and notifies the hook thru defined interface
 *          functions. All configuration structures are declared in
 *          xwphook.h.
 *
 *@@added V0.9.0 [umoeller]
 *@@header "hook\hook_private.h"
 */

/*
 *      Copyright (C) 1999-2003 Ulrich M”ller.
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
 */

#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSMODULEMGR
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINSWITCHLIST
#define INCL_WININPUT
#define INCL_WINPOINTERS
#define INCL_WINMENUS
#define INCL_WINSCROLLBARS
#define INCL_WINSTDCNR
#define INCL_WINSYS
#define INCL_WINTIMER
#define INCL_WINHOOKS
#include <os2.h>

#include <stdio.h>

// PMPRINTF in hooks is a tricky issue;
// avoid this unless this is really needed.
// If enabled, NEVER give the PMPRINTF window
// the focus, or your system will hang solidly...
#define DONTDEBUGATALL
#define DONT_REPLACE_FOR_DBCS
#define DONT_REPLACE_MALLOC         // in case mem debug is enabled
#include "setup.h"

#include "hook\xwphook.h"
#include "hook\hook_private.h"          // private hook and daemon definitions

#pragma hdrstop

/******************************************************************
 *
 *  Global variables
 *
 ******************************************************************/

/*
 *      We must initialize all these variables to
 *      something, because if we don't, the compiler
 *      won't put them in the DLL's shared data segment
 *      (see introductory notes).
 */

HOOKDATA        G_HookData = {0};
    // this includes HOOKCONFIG; its address is
    // returned from hookInit and stored within the
    // daemon to configure the hook

PGLOBALHOTKEY   G_paGlobalHotkeys = NULL;
    // pointer to block of shared memory containing
    // the hotkeys; maintained by hookSetGlobalHotkeys,
    // requested by the pre-accel hook
ULONG           G_cGlobalHotkeys = 0;
    // count of items in that array (_not_ array size!)

PFUNCTIONKEY    G_paFunctionKeys = NULL;
    // pointer to block of shared memory containing
    // the function keys; maintained by hookSetGlobalHotkeys,
    // requested by the pre-accel hook
ULONG           G_cFunctionKeys = 0;
    // count of items in that array (_not_ array size!)

HMTX            G_hmtxGlobalHotkeys = NULLHANDLE;
    // mutex for protecting the keys arrays

/******************************************************************
 *
 *  System-wide global variables for input hook
 *
 ******************************************************************/

HWND    G_hwndUnderMouse = NULLHANDLE;
HWND    G_hwndLastFrameUnderMouse = NULLHANDLE;
HWND    G_hwndLastSubframeUnderMouse = NULLHANDLE;
POINTS  G_ptsMousePosWin = {0};
POINTL  G_ptlMousePosDesktop = {0};
HWND    G_hwndRootMenu = NULLHANDLE; // V0.9.14 (2001-08-01) [lafaix]
HWND    G_hwndStickyMenu = NULLHANDLE; // V1.0.0 (2002-09-12) [lafaix]

/*
 * Prototypes:
 *
 */

VOID EXPENTRY hookSendMsgHook(HAB hab, PSMHSTRUCT psmh, BOOL fInterTask);
VOID EXPENTRY hookLockupHook(HAB hab, HWND hwndLocalLockupFrame);
BOOL EXPENTRY hookInputHook(HAB hab, PQMSG pqmsg, USHORT fs);
BOOL EXPENTRY hookPreAccelHook(HAB hab, PQMSG pqmsg, ULONG option);

// _CRT_init is the C run-time environment initialization function.
// It will return 0 to indicate success and -1 to indicate failure.
int _CRT_init(void);

// _CRT_term is the C run-time environment termination function.
// It only needs to be called when the C run-time functions are statically
// linked, as is the case with XWorkplace.
void _CRT_term(void);

/******************************************************************
 *
 *  Helper functions
 *
 ******************************************************************/

/*
 *@@ hookLog:
 *
 *@@added V0.9.16 (2002-01-13) [umoeller]
 */

VOID hookLog(PCSZ pcszSourceFile, // in: source file name
             ULONG ulLine,               // in: source line
             PCSZ pcszFunction,   // in: function name
             PCSZ pcszFormat,     // in: format string (like with printf)
             ...)                        // in: additional stuff (like with printf)
{
    APIRET      arc;
    va_list     args;
    CHAR        sz[1000];
    HFILE       hf;
    ULONG       ulAction;

    ULONG       ulBootDrive;
    DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                    &ulBootDrive,
                    sizeof(ulBootDrive));
    sprintf(sz,
            "%c:\\%s",
            ulBootDrive + 'A' - 1,
            "xwphook.log");

    if (!(arc = DosOpen(sz,
                        &hf,
                        &ulAction,
                        0,
                        FILE_ARCHIVED,
                        OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                        OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_READWRITE,
                        NULL)))       // EAs
    {
        DATETIME DT;
        PSZ     psz = sz;
        ULONG   cb;
        ULONG   ulOffset;

        if (ulAction == FILE_EXISTED)
            // get its size and set ptr to end for append
            DosSetFilePtr(hf,
                          0,
                          FILE_END,
                          &ulOffset);

        DosGetDateTime(&DT);
        psz += sprintf(psz,
                       "%04d-%02d-%02d %02d:%02d:%02d "
                       "%s (%s, line %d):\r\n    ",
                       DT.year, DT.month, DT.day,
                       DT.hours, DT.minutes, DT.seconds,
                       pcszFunction, pcszSourceFile, ulLine);
        va_start(args, pcszFormat);
        psz += vsprintf(psz, pcszFormat, args);
        *psz++ = '\r';
        *psz++ = '\n';
        va_end(args);
        cb = psz - sz;
        DosWrite(hf,
                 sz,
                 cb,
                 &ulAction);
        DosSetFileSize(hf, ulOffset + ulAction);
        DosClose(hf);
    }
}

/*
 *@@ InitializeGlobalsForHooks:
 *      this gets called from hookInit to initialize
 *      the global variables. We query the PM desktop,
 *      the window list, and other stuff we need all
 *      the time.
 */

VOID InitializeGlobalsForHooks(VOID)
{
    HENUM   henum;
    HWND    hwndThis;
    BOOL    fFound;

    // screen dimensions
    G_HookData.cxScreen = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
    G_HookData.cyScreen = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);

    // V0.9.19 (2002-06-13) [umoeller]
    G_HookData.szlIcon.cx = WinQuerySysValue(HWND_DESKTOP, SV_CXICON);
    G_HookData.szlIcon.cy = WinQuerySysValue(HWND_DESKTOP, SV_CYICON);

    // PM desktop (the WPS desktop is handled by the daemon)
    G_HookData.hwndPMDesktop = WinQueryDesktopWindow(G_HookData.habDaemonObject,
                                                     NULLHANDLE);

    WinQueryWindowProcess(HWND_DESKTOP, &G_HookData.pidPM, NULL);
            // V0.9.7 (2001-01-21) [umoeller]

    // enumerate desktop window to find the window list:
    // according to PMTREE, the window list has the following
    // window hierarchy:
    //      WC_FRAME
    //        +--- WC_TITLEBAR
    //        +--- Menu
    //        +--- WindowList
    //        +---
    fFound = FALSE;
    henum = WinBeginEnumWindows(HWND_DESKTOP);
    while (     (!fFound)
             && (hwndThis = WinGetNextWindow(henum))
          )
    {
        CHAR    szClass[200];
        if (WinQueryClassName(hwndThis, sizeof(szClass), szClass))
        {
            if (!strcmp(szClass, "#1"))
            {
                // frame window: check the children
                HENUM   henumFrame;
                HWND    hwndChild;
                henumFrame = WinBeginEnumWindows(hwndThis);
                while (    (!fFound)
                        && (hwndChild = WinGetNextWindow(henumFrame))
                      )
                {
                    CHAR    szChildClass[200];
                    if (WinQueryClassName(hwndChild, sizeof(szChildClass), szChildClass))
                    {
                        if (    (!strcmp(szChildClass, "WindowList"))
                             && (G_HookData.hwndSwitchListCnr = WinWindowFromID(hwndChild,
                                                                                100))
                           )
                        {
                            // yup, found:
                            G_HookData.hwndSwitchList = hwndThis;
                            fFound = TRUE;
                        }
                    }
                }

                WinEndEnumWindows(henumFrame);
            }
        }
    }

    WinEndEnumWindows(henum);
}

/******************************************************************
 *
 *  Hook interface
 *
 ******************************************************************/

/*
 *@@ _DLL_InitTerm:
 *      this function gets called automatically by the OS/2 DLL
 *      during DosLoadModule processing, on the thread which
 *      invoked DosLoadModule.
 *
 *      We override this function (which is normally provided by
 *      the runtime library) to intercept this DLL's module handle.
 *
 *      Since OS/2 calls this function directly, it must have
 *      _System linkage.
 *
 *      Note: You must then link using the /NOE option, because
 *      the VAC++ runtimes also contain a _DLL_Initterm, and the
 *      linker gets in trouble otherwise.
 *      The XWorkplace makefile takes care of this.
 *
 *      This function must return 0 upon errors or 1 otherwise.
 *
 *@@changed V0.9.0 [umoeller]: reworked locale initialization
 */

unsigned long _System _DLL_InitTerm(unsigned long hModule,
                                    unsigned long ulFlag)
{
    switch (ulFlag)
    {
        case 0:
            // store the DLL handle in the global variable
            G_HookData.hmodDLL = hModule;

            // now initialize the C run-time environment before we
            // call any runtime functions
            if (_CRT_init() == -1)
               return 0;  // error
        break;

        case 1:
            // DLL being freed: cleanup runtime
            _CRT_term();
            break;

        default:
            // other code: beep for error
            DosBeep(100, 100);
            return 0;     // error
    }

    // a non-zero value must be returned to indicate success
    return 1;
}

/*
 *@@ hookInit:
 *      registers (sets) all the hooks and initializes data.
 *
 *      In any case, a pointer to the DLL's static HOOKDATA
 *      structure is returned. In this struct, the caller
 *      can examine the two flags for whether the hooks
 *      were successfully installed.
 *
 *      Note: All the exported hook* interface functions must
 *      only be called by the same process, which is the
 *      XWorkplace daemon (XWPDAEMN.EXE).
 *
 *      This gets called by XWPDAEMN.EXE when
 *
 *@@changed V0.9.1 (2000-02-01) [umoeller]: fixed missing global updates
 *@@changed V0.9.2 (2000-02-21) [umoeller]: added new system hooks
 */

PHOOKDATA EXPENTRY hookInit(HWND hwndDaemonObject)  // in: daemon object window (receives notifications)
{
    APIRET arc = NO_ERROR;

    if (G_hmtxGlobalHotkeys == NULLHANDLE)
        arc = DosCreateMutexSem(NULL,        // unnamed
                                &G_hmtxGlobalHotkeys,
                                DC_SEM_SHARED,
                                FALSE);      // initially unowned

    if (arc == NO_ERROR)
    {
        G_HookData.hwndDaemonObject = hwndDaemonObject;
        G_HookData.habDaemonObject = WinQueryAnchorBlock(hwndDaemonObject);

        if (G_HookData.hmodDLL)       // initialized by _DLL_InitTerm
        {
            // initialize globals needed by the hook
            InitializeGlobalsForHooks();

            // install hooks, but only once...
            if (!G_HookData.fSendMsgHooked)
                G_HookData.fSendMsgHooked = WinSetHook(G_HookData.habDaemonObject,
                                                       NULLHANDLE, // system hook
                                                       HK_SENDMSG, // send-message hook
                                                       (PFN)hookSendMsgHook,
                                                       G_HookData.hmodDLL);

            if (!G_HookData.fLockupHooked)
                G_HookData.fLockupHooked = WinSetHook(G_HookData.habDaemonObject,
                                                      NULLHANDLE, // system hook
                                                      HK_LOCKUP,  // lockup hook
                                                      (PFN)hookLockupHook,
                                                      G_HookData.hmodDLL);

            if (!G_HookData.fInputHooked)
                G_HookData.fInputHooked = WinSetHook(G_HookData.habDaemonObject,
                                                     NULLHANDLE, // system hook
                                                     HK_INPUT,    // input hook
                                                     (PFN)hookInputHook,
                                                     G_HookData.hmodDLL);

            if (!G_HookData.fPreAccelHooked)
                G_HookData.fPreAccelHooked = WinSetHook(G_HookData.habDaemonObject,
                                                        NULLHANDLE, // system hook
                                                        HK_PREACCEL,  // pre-accelerator table hook (undocumented)
                                                        (PFN)hookPreAccelHook,
                                                        G_HookData.hmodDLL);


            #ifdef HACKSWITCHLIST
                hookLog(__FILE__, __LINE__, __FUNCTION__,
                        "hacking switch list");

                // doing the subclassing directly works, but
                // doing the post msg won't. It won't even
                // log the first string in HackSwitchList().
                HackSwitchList(TRUE);

                /*
                WinPostMsg(G_HookData.hwndSwitchListCnr,
                           WM_HACKSWITCHLIST,
                           MP1_HACKSWITCHLIST,
                           MP2_SUBCLASS);
                */
            #endif
        }

        _Pmpf(("Leaving hookInit"));
    }

    return &G_HookData;
}

/*
 *@@ hookKill:
 *      deregisters the hook function and frees allocated
 *      resources.
 *
 *      Note: This function must only be called by the same
 *      process which called hookInit (that is, the daemon),
 *      or resources cannot be properly freed.
 *
 *@@changed V0.9.1 (2000-02-01) [umoeller]: fixed missing global updates
 *@@changed V0.9.2 (2000-02-21) [umoeller]: added new system hooks
 *@@changed V0.9.3 (2000-04-20) [umoeller]: added function keys support
 */

BOOL EXPENTRY hookKill(void)
{
    BOOL brc = FALSE;

    _Pmpf(("hookKill"));

    #ifdef HACKSWITCHLIST
        hookLog(__FILE__, __LINE__, __FUNCTION__,
                "un-hacking switch list");
        HackSwitchList(FALSE);
    #endif

    if (G_HookData.fInputHooked)
    {
        WinReleaseHook(G_HookData.habDaemonObject,
                       NULLHANDLE,
                       HK_INPUT,
                       (PFN)hookInputHook,
                       G_HookData.hmodDLL);
        G_HookData.fInputHooked = FALSE;
        brc = TRUE;
    }

    if (G_HookData.fPreAccelHooked)
    {
        WinReleaseHook(G_HookData.habDaemonObject,
                       NULLHANDLE,
                       HK_PREACCEL,     // pre-accelerator table hook (undocumented)
                       (PFN)hookPreAccelHook,
                       G_HookData.hmodDLL);
        brc = TRUE;
        G_HookData.fPreAccelHooked = FALSE;
    }

    if (G_HookData.fLockupHooked)
    {
        WinReleaseHook(G_HookData.habDaemonObject,
                       NULLHANDLE,
                       HK_LOCKUP,       // lockup hook
                       (PFN)hookLockupHook,
                       G_HookData.hmodDLL);
        brc = TRUE;
        G_HookData.fLockupHooked = FALSE;
    }

    if (G_HookData.fSendMsgHooked)
    {
        WinReleaseHook(G_HookData.habDaemonObject,
                       NULLHANDLE,
                       HK_SENDMSG,       // lockup hook
                       (PFN)hookSendMsgHook,
                       G_HookData.hmodDLL);
        brc = TRUE;
        G_HookData.fSendMsgHooked = FALSE;
    }

    // free shared mem
    if (G_paGlobalHotkeys)
        DosFreeMem(G_paGlobalHotkeys);
    if (G_paFunctionKeys) // V0.9.3 (2000-04-20) [umoeller]
        DosFreeMem(G_paFunctionKeys);

    if (G_hmtxGlobalHotkeys)
    {
        DosCloseMutexSem(G_hmtxGlobalHotkeys);
        G_hmtxGlobalHotkeys = NULLHANDLE;
    }

    return brc;
}

/*
 *@@ hookSetGlobalHotkeys:
 *      this exported function gets called to update the
 *      hook's lists of global hotkeys and XWP function keys.
 *
 *      This is the only interface to the hotkeys lists.
 *      If we wish to change any of the keys configurations,
 *      we must always pass two complete arrays of new
 *      hotkeys to this function. This is not terribly
 *      comfortable, but we're dealing with global PM
 *      hooks here, so comfort is not really the main
 *      objective.
 *
 *      XFldObject::xwpSetObjectHotkey can be used for
 *      a more convenient interface. That method will
 *      automatically recompose the complete list when
 *      a single hotkey changes and call this func in turn.
 *
 *      hifSetFunctionKeys can be used to reconfigure the
 *      function keys.
 *
 *      This function copies the given lists into two newly
 *      allocated blocks of shared memory, which are used
 *      by the hook. If previous such blocks exist, they
 *      are freed and reallocated. Access to those blocks
 *      is protected by a mutex semaphore internally in case
 *      WM_CHAR comes in while the lists are being modified,
 *      which will cause the hook functions to be running
 *      on some application thread, while this function
 *      must only be called by the daemon.
 *
 *      Note: This function must only be called by the same
 *      process which called hookInit (that is, the
 *      daemon), because otherwise the shared memory cannot
 *      be properly freed.
 *
 *      This returns the DOS error code of the various
 *      semaphore and shared mem API calls.
 *
 *@@changed V0.9.3 (2000-04-20) [umoeller]: added function keys support; changed prototype
 */

APIRET EXPENTRY hookSetGlobalHotkeys(PGLOBALHOTKEY pNewHotkeys, // in: new hotkeys array
                                     ULONG cNewHotkeys,         // in: count of items in array -- _not_ array size!!
                                     PFUNCTIONKEY pNewFunctionKeys, // in: new hotkeys array
                                     ULONG cNewFunctionKeys)         // in: count of items in array -- _not_ array size!!
{
    BOOL    fSemOpened = FALSE,
            fLocked = FALSE;
    APIRET arc;

    // request access to the hotkeys mutex
    if (!(arc = DosOpenMutexSem(NULL,       // unnamed
                                &G_hmtxGlobalHotkeys)))
    {
        fSemOpened = TRUE;
        arc = DosRequestMutexSem(G_hmtxGlobalHotkeys,
                                 TIMEOUT_HMTX_HOTKEYS);
    }

    if (!arc)
    {
        fLocked = TRUE;

        // 1) hotkeys

        if (G_paGlobalHotkeys)
        {
            // hotkeys defined already: free the old ones
            DosFreeMem(G_paGlobalHotkeys);
            G_paGlobalHotkeys = 0;
        }

        if (!(arc = DosAllocSharedMem((PVOID*)(&G_paGlobalHotkeys),
                                      SHMEM_HOTKEYS,
                                      sizeof(GLOBALHOTKEY) * cNewHotkeys,
                                            // rounded up to 4KB
                                      PAG_COMMIT | PAG_READ | PAG_WRITE)))
        {
            // copy hotkeys to shared memory
            memcpy(G_paGlobalHotkeys,
                   pNewHotkeys,
                   sizeof(GLOBALHOTKEY) * cNewHotkeys);
            G_cGlobalHotkeys = cNewHotkeys;
        }

        // 2) function keys V0.9.3 (2000-04-20) [umoeller]

        if (G_paFunctionKeys)
        {
            // function keys defined already: free the old ones
            DosFreeMem(G_paFunctionKeys);
            G_paFunctionKeys = 0;
        }

        if (!(arc = DosAllocSharedMem((PVOID*)(&G_paFunctionKeys),
                                      SHMEM_FUNCTIONKEYS,
                                      sizeof(FUNCTIONKEY) * cNewFunctionKeys, // rounded up to 4KB
                                      PAG_COMMIT | PAG_READ | PAG_WRITE)))
        {
            // copy function keys to shared memory
            memcpy(G_paFunctionKeys,
                   pNewFunctionKeys,
                   sizeof(FUNCTIONKEY) * cNewFunctionKeys);
            G_cFunctionKeys = cNewFunctionKeys;
            _Pmpf(("hookSetGlobalHotkeys: G_cFunctionKeys = %d", G_cFunctionKeys));
        }
    }

    if (fLocked)
        DosReleaseMutexSem(G_hmtxGlobalHotkeys);
    if (fSemOpened)
        DosCloseMutexSem(G_hmtxGlobalHotkeys);

    return arc;
}

/******************************************************************
 *
 *  Send-Message Hook
 *
 ******************************************************************/

#ifndef __NOPAGER__

/*
 *@@ ProcessMsgsForWinlist:
 *      message processing for the daemon window list
 *      to intercept messages in both hookInputHook
 *      and hookSendMsgHook.
 *
 *      This intercepts all messages that the window
 *      list needs to record changes. We then post
 *      either XDM_WINDOWCHANGE or XDM_ICONCHANGE
 *      to the daemon, which will notify all its
 *      clients -- most importantly the pager and
 *      the XCenter window list widget.
 *
 *      See pg_winlist.c for details.
 *
 *@@added V0.9.2 (2000-02-21) [umoeller]
 *@@changed V0.9.4 (2000-07-10) [umoeller]: fixed float-on-top
 *@@changed V0.9.7 (2001-01-15) [dk]: WM_SETWINDOWPARAMS added
 *@@changed V0.9.7 (2001-01-18) [umoeller]: removed lockup call, pager doesn't need this
 *@@changed V0.9.7 (2001-01-18) [umoeller]: fixed sticky odin windows
 *@@changed V0.9.7 (2001-01-18) [umoeller]: fixed sticky EPM
 *@@changed V0.9.19 (2002-05-28) [umoeller]: reworked for new winlist handling
 */

VOID ProcessMsgsForWinlist(HWND hwnd,
                           ULONG msg,
                           MPARAM mp1,
                           MPARAM mp2)
{
    PSWP pswp;

    // first check, just for speed
    if (    (msg == WM_CREATE)
         || (msg == WM_DESTROY)
         || (msg == WM_ACTIVATE)
         || (msg == WM_WINDOWPOSCHANGED)
         // respect WM_SETWINDOWPARAMS only if text has changed
         || (   (msg == WM_SETWINDOWPARAMS)
             && (((PWNDPARAMS)mp1)->fsStatus & WPM_TEXT) // 0x0001
            )
         // record icon changes
         || (msg == WM_SETICON)     // V0.9.19 (2002-05-28) [umoeller]
       )
    {
        if (    (WinQueryWindow(hwnd, QW_PARENT) == G_HookData.hwndPMDesktop)
             && (hwnd != G_HookData.hwndPagerFrame)
                    // V0.9.7 (2001-01-23) [umoeller]
           )
        {
            CHAR    szClass[30];

            if (WinQueryClassName(hwnd, sizeof(szClass), szClass))
            {
                if (    (!strcmp(szClass, "#1"))
                     || (!strcmp(szClass, "wpFolder window"))
                     || (!strcmp(szClass, "Win32FrameClass"))
                                // that's for Odin V0.9.7 (2001-01-18) [umoeller]
                     || (!strcmp(szClass, "EFrame"))
                                // that's for EPM V0.9.7 (2001-01-19) [dk]
                   )
                {
                    if (msg == WM_SETICON)
                        WinPostMsg(G_HookData.hwndDaemonObject,
                                   XDM_ICONCHANGE,
                                   (MPARAM)hwnd,
                                   (MPARAM)mp1);        // HPOINTER
                    else
                        WinPostMsg(G_HookData.hwndDaemonObject,
                                   XDM_WINDOWCHANGE,
                                   (MPARAM)hwnd,
                                   (MPARAM)msg);
                } // end if (    (strcmp(szClass, ...
            } // end if (WinQueryClassName(hwnd, sizeof(szClass), szClass))
        } // end if (WinQueryWindow(hwnd, QW_PARENT) == HookData.hwndPMDesktop)
    }
}

#endif

/*
 *@@ hookSendMsgHook:
 *      send-message hook.
 *
 *      The send-message hook runs on the thread which called
 *      WinSendMsg, before the message is delivered to the
 *      target window (which might be on a different thread
 *      or even in a different process). In other words, as
 *      opposed to the input hook, this runs on the _sender's_
 *      thread.
 *
 *      We must not do any complex processing in here, especially
 *      calling WinSendMsg(). Instead, we post msgs to other places,
 *      because sending messages will recurse into this hook forever.
 *
 *      Be warned that many PM API functions send messages also.
 *      This applies especially to WinSetWindowPos and such.
 *      They are a no-no in here because they would lead to
 *      infinite recursion also.
 *
 *      SMHSTRUCT is defined as follows:
 *
 *          typedef struct _SMHSTRUCT {
 *            MPARAM     mp2;
 *            MPARAM     mp1;
 *            ULONG      msg;
 *            HWND       hwnd;
 *            ULONG      model;  //  Message identity ?!?
 *          } SMHSTRUCT;
 *
 *@@added V0.9.2 (2000-02-21) [umoeller]
 *@@changed V0.9.9 (2001-03-10) [umoeller]: fixed errant sliding menu behavior
 *@@changed V0.9.14 (2001-08-01) [lafaix]: added menu mode check for auto hide
 *@@changed V0.9.14 (2001-08-02) [lafaix]: added auto move to default button
 *@@changed V0.9.16 (2001-11-22) [umoeller]: hotkeys stopped working after lockup if XPager wasn't running; fixed
 *@@changed V0.9.19 (2002-04-04) [lafaix]: enabled AMF_ALWAYSMOVE for auto move feature
 *@@changed V0.9.20 (2002-07-03) [umoeller]: fixed pager stay on top
 *@@changed V0.9.20 (2002-07-16) [lafaix]: AHF_IGNOREMENUS now recognize Mozilla menus too
 *@@changed V1.0.0 (2002-09-14) [lafaix]: added support for stickyness toggle in system menus
 */

VOID EXPENTRY hookSendMsgHook(HAB hab,
                              PSMHSTRUCT psmh,
                              BOOL fInterTask)
{

#ifndef __NOPAGER__

    if (!G_HookData.cSuppressWinlistNotify)
                // this flag is set frequently when XPager
                // is doing tricky stuff; we must not process
                // messages then, or we'll recurse forever
    {
        // OK, go ahead:
        PSWP pswp;

        ProcessMsgsForWinlist(psmh->hwnd,
                             psmh->msg,
                             psmh->mp1,
                             psmh->mp2);

        // V0.9.7 (2001-01-23) [umoeller]
        if (    (G_HookData.PagerConfig.flPager & PGRFL_STAYONTOP)
             // (psmh->msg == WM_ADJUSTWINDOWPOS)        doesn't work
             && (psmh->msg == WM_WINDOWPOSCHANGED)
             && (pswp = (PSWP)psmh->mp1)
             && (pswp->fl & SWP_ZORDER)
             && (psmh->hwnd != G_HookData.hwndPagerFrame)
             // for some reason, this criterion never works
             // V0.9.20 (2002-07-03) [umoeller]
             // && (pswp->hwndInsertBehind == HWND_TOP)
             && (WinIsWindowVisible(G_HookData.hwndPagerFrame))
                // only do this if a desktop window is moved to the top
             && (WinQueryWindow(psmh->hwnd, QW_PARENT) == G_HookData.hwndPMDesktop)
           )
        {
            // but disable switching V0.9.12 (2001-05-31) [umoeller]
            ++G_HookData.cSuppressWinlistNotify;
            WinSetWindowPos(G_HookData.hwndPagerFrame,
                            HWND_TOP,
                            0, 0, 0, 0,
                            SWP_ZORDER);
            --G_HookData.cSuppressWinlistNotify;
        }
    }
#endif

    // special case check... if a menu control has been hidden
    // and it is the menu for which we have a sliding menu
    // timer running in the daemon, stop that timer NOW
    // V0.9.9 (2001-03-10) [umoeller]
    // this happens if the user quickly selects a menu item
    // by clicking on it before the delayed-select timer has
    // elapsed
    if (    (psmh->msg == WM_WINDOWPOSCHANGED)          // comes in for menu hide
            // sliding menus enabled?
         && (G_HookData.HookConfig.fSlidingMenus)
            // delay enabled?
         && (G_HookData.HookConfig.ulSubmenuDelay)
            // changing window is the menu control that we
            // previously started a timer for?
         && (psmh->hwnd == G_HookData.hwndMenuUnderMouse)
            // menu control is hiding?
         && ((PSWP)(psmh->mp1))
         && (((PSWP)(psmh->mp1))->fl & SWP_HIDE)
       )
    {
        // yes: stop the delayed-select timer NOW
        WinPostMsg(G_HookData.hwndDaemonObject,
                   XDM_SLIDINGMENU,
                   (MPARAM)-1,          // stop timer
                   0);
    }
#ifndef __NOMOVEMENT2FEATURES__
    else
        // special extra check, to find out if menu mode is active
        // or not.  If G_hwndRootMenu is not NULLHANDLE, then we are
        // in menu mode (i.e., a menu is active).
        // V0.9.14 (2001-08-01) [lafaix]
        if (    (G_HookData.HookConfig.__fAutoHideMouse)
             && (G_HookData.HookConfig.__ulAutoHideFlags & AHF_IGNOREMENUS)
           )
        {
            CHAR szClass[30];

            do      // just a do for breaking, no loop
            {
                // messages we have no interest in are handled in just
                // three tests, which is the best we can hope for, as we
                // must take care of three different messages

                if (psmh->msg == WM_INITMENU)
                {
                    if (G_hwndRootMenu == NULLHANDLE)
                    {
                        G_hwndRootMenu = (HWND)psmh->mp2;
                        WMMouseMove_AutoHideMouse();
                    }
                    break;
                }

                if (psmh->msg == WM_MENUEND)
                {
                    if (G_hwndRootMenu == (HWND)psmh->mp2)
                    {
                        G_hwndRootMenu = NULLHANDLE;
                        WMMouseMove_AutoHideMouse();
                    }
                    break;
                }

                // Mozilla's menus don't do the WM_INITMENU / WM_MENUEND
                // stuff, so we must handle them specially
                // V0.9.20 (2002-07-16) [lafaix]

                if (psmh->msg == WM_WINDOWPOSCHANGED)
                {
                    if (    (G_hwndRootMenu == NULLHANDLE)
                         && ((PSWP)PVOIDFROMMP(psmh->mp1))
                         && (((PSWP)PVOIDFROMMP(psmh->mp1))->fl & SWP_SHOW)
                         && (WinQueryWindow(psmh->hwnd, QW_PARENT) == G_HookData.hwndPMDesktop)
                         &&  WinQueryClassName(psmh->hwnd, sizeof(szClass), szClass)
                         && (!strcmp(szClass, "MozillaWindowClass"))
                                // we only check if this is a top level window of
                                // the MozillaWindowClass class here, to minimize
                                // overhead, as the check is not required in the
                                // other case
                       )
                    {
                        G_hwndRootMenu = (HWND)psmh->hwnd;
                        WMMouseMove_AutoHideMouse();
                    }
                    else if (    (G_hwndRootMenu == psmh->hwnd)
                              && ((PSWP)PVOIDFROMMP(psmh->mp1))
                              && (((PSWP)PVOIDFROMMP(psmh->mp1))->fl & SWP_HIDE)
                            )
                    {
                        G_hwndRootMenu = NULLHANDLE;
                        WMMouseMove_AutoHideMouse();
                    }
                    break;
                }

            } while (FALSE); // end do
        }

    // yet another extra check, to find out if the
    // about-to-be-shown window contains a default push button
    // V0.9.14 (2001-08-02) [lafaix]
    if (    (G_HookData.HookConfig.__fAutoMoveMouse)
         && (    (    (psmh->msg == WM_SHOW)
                   && (SHORT1FROMMP(psmh->mp1))
                 )
              // for some reasons, opening a dialog window
              // does not produce a WM_SHOW message, so
              // we must check WM_WINDOWPOSCHANGED too
              || (    (psmh->msg == WM_WINDOWPOSCHANGED)
                   && (((PSWP)psmh->mp1)->fl & SWP_SHOW)
                 )
            )
         // we should ignore frames in notebooks; the parent
         // ID is hardcored as it's much faster than querying the
         // parent class and looks safe enough (at worst we don't
         // move the pointer)
         // V0.9.19 (2002-04-04) [lafaix]
         && (WinQueryWindowUShort(WinQueryWindow(psmh->hwnd, QW_PARENT), QWS_ID) != (USHORT)8006)
       )
    {
        HWND hwndDefButton;

        if (hwndDefButton = (HWND)WinQueryWindowULong(psmh->hwnd,
                                                      QWL_DEFBUTTON))
        {
            WinPostMsg(G_HookData.hwndDaemonObject,
                       XDM_MOVEPTRTOBUTTON,
                       (MPARAM)hwndDefButton,
                       0);
        }
        // if the dialog contains no default button, center the
        // pointer over the window
        // V0.9.19 (2002-04-04) [lafaix]
        else if (    (G_HookData.HookConfig.__ulAutoMoveFlags & AMF_ALWAYSMOVE)
                  && (WinQueryWindowUShort(psmh->hwnd, QWS_FLAGS) & FF_DIALOGBOX)
                  // ignore dialogs not direct children of desktop (fixes
                  // netscape odd hierarchy)
                  // V0.9.19 (2002-06-13) [lafaix]
                  // was comparing to HWND_DESKTOP, fixed
                  // V0.9.20 (2002-07-16) [lafaix]
                  && (WinQueryWindow(psmh->hwnd, QW_PARENT) == G_HookData.hwndPMDesktop)
                )
        {
            WinPostMsg(G_HookData.hwndDaemonObject,
                       XDM_MOVEPTRTOBUTTON,
                       (MPARAM)psmh->hwnd,
                       0);
        }
    }
#endif

#ifndef __NOPAGER__
    // adding/removing sticky menu item as needed.  This part is a bit
    // tricky as we have to _send_ messages to add items to the menu.
    // Even weirder, we have to send an inter process message to query
    // the "sticky" state of the current window.
    // V1.0.0 (2002-09-12) [lafaix]
    if (    (G_HookData.PagerConfig.flPager & PGRFL_ADDSTICKYTOGGLE)
             // don't do that if XPager is disabled
         && (G_HookData.hwndPagerFrame)
             // do the fast checks here to reduce overhead
         && (SHORT1FROMMP(psmh->mp1) == SC_SYSMENU)
         && (psmh->mp2)
       )
    {
        if (    (psmh->msg == WM_INITMENU)
                 // item not already added
             && (G_hwndStickyMenu == NULLHANDLE)
                 // top level frames ony
             && (WinQueryWindow(psmh->hwnd, QW_PARENT) == G_HookData.hwndPMDesktop)
           )
        {
            MENUITEM mi = {0};
            HWND hwndFrame = WinQueryWindow(psmh->hwnd, QW_FRAMEOWNER);

            G_hwndStickyMenu = (HWND)psmh->mp2;

            mi.iPosition = MIT_END;
            mi.afStyle = MIS_SEPARATOR;
            mi.afAttribute = 0;
            mi.id = PGRIDM_TOGGLESEPARATOR;
            mi.hwndSubMenu = 0;
            mi.hItem = 0;
            WinSendMsg(G_hwndStickyMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       NULL);

            mi.afStyle = MIS_TEXT|MIS_SYSCOMMAND;
            mi.id = PGRIDM_TOGGLEITEM;

            if (WinSendMsg(G_HookData.hwndDaemonObject,
                           XDM_ISTRANSIENTSTICKY,
                           // the HWND to be tested is either the target
                           // window (in case of a direct menu) or the
                           // frame owner (in case of a submenu).  This
                           // fixes the problem with folders not displaying
                           // the checkmark.
                           // V1.0.0 (2002-09-15) [lafaix]
                           MPFROMHWND(hwndFrame ? hwndFrame : psmh->hwnd),
                           0)
               )
                mi.afAttribute = MIA_CHECKED;
            else
                mi.afAttribute = 0;
            WinSendMsg(G_hwndStickyMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       G_HookData.NLSData.apszNLSStrings[NLS_STICKYTOGGLE]);
        }
        else if (    (psmh->msg == WM_MENUEND)
                  && ((HWND)psmh->mp2 == G_hwndStickyMenu)
                )
        {
            WinSendMsg(G_hwndStickyMenu,
                       MM_DELETEITEM,
                       MPFROM2SHORT(PGRIDM_TOGGLESEPARATOR, FALSE),
                       NULL);
            WinSendMsg(G_hwndStickyMenu,
                       MM_DELETEITEM,
                       MPFROM2SHORT(PGRIDM_TOGGLEITEM, FALSE),
                       NULL);
            G_hwndStickyMenu = NULLHANDLE;
        }
    }
#endif

    // moved this here from ProcessMsgsForWinlist;
    // otherwise this is not picked up if XPager
    // isn't running V0.9.16 (2001-11-22) [umoeller]
    if (    (psmh->msg == WM_DESTROY)
         && (psmh->hwnd == G_HookData.hwndLockupFrame)
       )
    {
        // tested, works V0.9.16 (2001-11-22) [umoeller]

        // current lockup frame being destroyed
        // (system is being unlocked):
        G_HookData.hwndLockupFrame = NULLHANDLE;
    }
}

/******************************************************************
 *
 *  Lockup Hook
 *
 ******************************************************************/

/*
 *@@ hookLockupHook:
 *
 *@@added V0.9.2 (2000-02-21) [umoeller]
 *@@changed V0.9.6 (2000-11-05) [pr]: fix for hotkeys not working after Lockup
 *@@changed V0.9.7 (2001-01-18) [umoeller]: removed lockup call, pager doesn't need this
 *@@changed V0.9.14 (2001-08-21) [umoeller]: now always storing lockup window, we need this in various places
 */

VOID EXPENTRY hookLockupHook(HAB hab,
                             HWND hwndLocalLockupFrame)
{
    G_HookData.hwndLockupFrame = hwndLocalLockupFrame;
}

/******************************************************************
 *
 *  Input hook (main function)
 *
 ******************************************************************/

/*
 *@@ GetFrameWindow:
 *      this finds the desktop window (child of
 *      HWND_DESKTOP) to which the specified window
 *      belongs.
 */

HWND GetFrameWindow(HWND hwndTemp)
{
    HWND    hwndPrevious = NULLHANDLE;
    // climb up the parents tree until we get a frame
    while (    (hwndTemp)
            && (hwndTemp != G_HookData.hwndPMDesktop)
          )
    {
        hwndPrevious = hwndTemp;
        hwndTemp = WinQueryWindow(hwndTemp, QW_PARENT);
    }

    return hwndPrevious;
}

/*
 *@@ hookInputHook:
 *      input queue hook.
 *
 *      This hook gets called just before the system returns
 *      from a WinGetMsg or WinPeekMsg call. As a result, this
 *      does _not_ get called on the poster's thread (who called
 *      WinPostMsg), but on the receiver's thread.
 *
 *      However, only _posted_ messages go thru this hook. _Sent_
 *      messages never get here; there is a separate hook type
 *      for that (see hookSendMsgHook).
 *
 *      This implements the XWorkplace mouse features such as
 *      hot corners, sliding focus, mouse-button-3 scrolling etc.
 *
 *      Note: WM_CHAR messages (hotkeys) are not processed here,
 *      but rather in hookPreAccelHook, which gets called even
 *      before this hook. See remarks there.
 *
 *      We return TRUE if the message is to be swallowed, or FALSE
 *      if the current application (or the next hook in the hook
 *      chain) should still receive the message.
 *
 *      I have tried to keep the actual hook function as short as
 *      possible. Several separate subfunctions have been extracted
 *      which only get called if the respective functionality is
 *      actually needed (depending on the message and whether the
 *      respective feature has been enabled), both for clarity and
 *      speed. This is faster because the hook function gets called
 *      thousands of times (for every posted message...), and this
 *      way it is more likely that the hook code can completely remain
 *      in the processor caches all the time when lots of messages are
 *      processed.
 *
 *@@changed V0.9.1 (99-12-03) [umoeller]: added MB3 mouse scroll
 *@@changed V0.9.1 (2000-01-31) [umoeller]: fixed end-scroll bug
 *@@changed V0.9.2 (2000-02-21) [umoeller]: added XPager processing
 *@@changed V0.9.2 (2000-02-25) [umoeller]: HScroll not working when VScroll disabled; fixed
 *@@changed V0.9.2 (2000-02-25) [umoeller]: added checks for mouse capture
 *@@changed V0.9.4 (2000-06-11) [umoeller]: changed MB3 scroll to WM_BUTTON3MOTIONSTART/END
 *@@changed V0.9.4 (2000-06-11) [umoeller]: added MB3 single-click
 *@@changed V0.9.4 (2000-06-14) [umoeller]: fixed PMMail win-list-as-pointer bug
 *@@changed V0.9.9 (2001-01-29) [umoeller]: auto-hide now respects button clicks too
 *@@changed V0.9.9 (2001-03-20) [lafaix]: fixed MB3 scroll for EPM clients
 *@@changed V0.9.9 (2001-03-20) [lafaix]: added many KC_NONE checks
 *@@changed V0.9.9 (2001-03-20) [lafaix]: added MB3 Autoscroll support
 *@@changed V0.9.9 (2001-03-21) [lafaix]: added MB3 Push2Bottom support
 *@@changed V0.9.14 (2001-08-21) [umoeller]: added click watches support
 *@@changed V0.9.20 (2002-07-16) [lafaix]: don't always raise window on MB1 click
 *@@changed V1.0.0 (2002-09-05) [lafaix]: added transient sticky menu support
 */

BOOL EXPENTRY hookInputHook(HAB hab,        // in: anchor block of receiver wnd
                            PQMSG pqmsg,    // in/out: msg data
                            USHORT fs)      // in: either PM_REMOVE or PM_NOREMOVE
{
    // set return value:
    // per default, pass message on to next hook or application
    BOOL        brc = FALSE;

    BOOL        fRestartAutoHide = FALSE;
                            // set to TRUE if auto-hide mouse should be handled

    if (pqmsg == NULL)
        return FALSE;

#ifndef __NOPAGER__
    if (!G_HookData.cSuppressWinlistNotify)
                // this flag is set frequently when XPager
                // is doing tricky stuff; we must not process
                // messages then, or we'll recurse forever
    {
        // OK, go ahead:
        ProcessMsgsForWinlist(pqmsg->hwnd,
                              pqmsg->msg,
                              pqmsg->mp1,
                              pqmsg->mp2);
    }

    // V1.0.0 (2002-09-05) [lafaix]
    if (    (pqmsg->msg == WM_SYSCOMMAND)
         && (SHORT1FROMMP(pqmsg->mp1) == PGRIDM_TOGGLEITEM)
         && (G_HookData.PagerConfig.flPager & PGRFL_ADDSTICKYTOGGLE)
         && (G_HookData.hwndPagerFrame)
       )
    {
        WinPostMsg(G_HookData.hwndDaemonObject,
                   XDM_TOGGLETRANSIENTSTICKY,
                   MPFROMHWND(pqmsg->hwnd),
                   0);

        // Netscape 4.61 enters in an endless loop if we swallow the
        // message.  V1.0.0 (2002-09-14) [lafaix]
        // brc = TRUE;
    }
#endif

    switch(pqmsg->msg)
    {
        /*****************************************************************
         *                                                               *
         *  mouse button 1                                               *
         *                                                               *
         *****************************************************************/

        /*
         * WM_BUTTON1DOWN:
         *      if "sliding focus" is on and "bring to top" is off,
         *      we need to explicitly raise the window under the mouse,
         *      because this fails otherwise. Apparently, PM checks
         *      internally when the mouse is clicked whether the window
         *      is active; if so, it thinks it must be on top already.
         *      Since the active and topmost window can now be different,
         *      we need to implement raising the window ourselves.
         *
         *      Parameters:
         *      -- POINTS mp1: ptsPointerPos
         *      -- USHORT SHORT1FROMMP(mp2): fsHitTestResult
         *      -- USHORT SHORT2FROMMP(mp2): fsFlags (KC_* as with WM_CHAR)
         */

        case WM_BUTTON1DOWN:
            if (    (G_HookData.bAutoScroll)
                 && (G_HookData.hwndCurrentlyScrolling)
               )
            {
                StopMB3Scrolling(TRUE);

                // swallow msg
                brc = TRUE;
            }
#ifndef __NOSLIDINGFOCUS__
            else
                if (    (G_HookData.HookConfig.__fSlidingFocus)
                     && (!G_HookData.HookConfig.__fSlidingBring2Top)
                   )
                {
                    // make sure that the mouse is not currently captured
                    if (WinQueryCapture(HWND_DESKTOP) == NULLHANDLE)
                    {
                        CHAR szWindowClass[4];
                                // use 4 instead of 3, or "#9whatever" is
                                // matched too V0.9.20 (2002-07-16) [umoeller]
                        WinQueryClassName(pqmsg->hwnd,
                                          sizeof(szWindowClass),
                                          szWindowClass);

                        // we only raise the window if MB1 is not
                        // pressed over the titlebar with Ctrl down
                        // (this mimics what PM does, to allow moving
                        // a window in the background)
                        // V0.9.20 (2002-07-16) [lafaix]
                        if (    ((SHORT2FROMMP(pqmsg->mp2) & KC_CTRL) == 0)
                             || strcmp(szWindowClass, "#9")
                           )
                            WinSetWindowPos(GetFrameWindow(pqmsg->hwnd),
                                            HWND_TOP,
                                            0,
                                            0,
                                            0,
                                            0,
                                            SWP_NOADJUST | SWP_ZORDER);
                    }
                }
#endif
        break;

        /*****************************************************************
         *                                                               *
         *  mouse button 2                                               *
         *                                                               *
         *****************************************************************/

        /*
         * WM_BUTTON2DOWN:
         *      if mb2 is clicked on titlebar,
         *      show system menu as context menu.
         *
         *      Based on ideas from WarpEnhancer by Achim Hasenmller.
         */

        case WM_BUTTON2DOWN:
            if (    (G_HookData.bAutoScroll)
                 && (G_HookData.hwndCurrentlyScrolling)
               )
            {
                StopMB3Scrolling(TRUE);

                // swallow msg
                brc = TRUE;
            }
            else
                if (    (G_HookData.HookConfig.fSysMenuMB2TitleBar)
                     && (SHORT2FROMMP(pqmsg->mp2) == KC_NONE)
                   )
                {
                    // make sure that the mouse is not currently captured
                    if (WinQueryCapture(HWND_DESKTOP) == NULLHANDLE)
                    {
                        CHAR szWindowClass[4];
                                // use 4 instead of 3, or "#9whatever" is
                                // matched too (not sure why actually, just
                                // mimicing what we do for button 1 above)
                                // V1.0.0 (2002-09-05) [lafaix]
                        // get class name of window being created
                        WinQueryClassName(pqmsg->hwnd,
                                          sizeof(szWindowClass),
                                          szWindowClass);
                        // mouse button 2 was pressed over a title bar control
                        if (!strcmp(szWindowClass, "#9"))
                        {
                            // copy system menu and display it as context menu
                            WMButton_SystemMenuContext(pqmsg->hwnd);
                            brc = TRUE; // swallow message
                        }
                    }
                }
        break;

        /*****************************************************************
         *                                                               *
         *  mouse button 3                                               *
         *                                                               *
         *****************************************************************/

        case WM_BUTTON3MOTIONSTART:
        case WM_BUTTON3DOWN:
        case WM_BUTTON3UP:
        case WM_BUTTON3CLICK:
            // moved all this handling to hk_scroll.c,
            // this code was getting too long
            // V0.9.16 (2001-11-22) [umoeller]
            brc = HandleMB3Msgs(pqmsg,
                                &fRestartAutoHide);
        break;


        /*
         * WM_CHORD:
         *      MB 1 and 2 pressed simultaneously:
         *      if enabled, we show the window list at the
         *      current mouse position.
         *
         *      Based on ideas from WarpEnhancer by Achim Hasenmller.
         */

        case WM_CHORD:
            brc = FALSE;
            // feature enabled?
            if (G_HookData.HookConfig.fChordWinList)
                // make sure that the mouse is not currently captured
                if (WinQueryCapture(HWND_DESKTOP) == NULLHANDLE)
                {
                    WinPostMsg(G_HookData.hwndDaemonObject,
                               XDM_WMCHORDWINLIST,
                               0, 0);
                    // WMChord_WinList();
                    // brc = TRUE;         // swallow message
                    // we must not swallow the message, or PMMail
                    // will cause the "win list as cursor" bug
                }
        break;

        /*
         * WM_MOUSEMOVE:
         *      "hot corners", "sliding focus", "MB3 scrolling" support.
         *      This is the most complex part of the hook and calls
         *      several subfunctions in turn.
         *
         *      WM_MOUSEMOVE parameters:
         *      -- SHORT SHORT1FROMMP(mp1): sxMouse (in win coords).
         *      -- SHORT SHORT2FROMMP(mp1): syMouse (in win coords).
         *      -- USHORT SHORT1FROMMP(mp2): uswHitTest: NULL if mouse
         *                  is currently captured; otherwise result
         *                  of WM_HITTEST message.
         *      -- USHORT SHORT2FROMMP(mp2): KC_* flags as with WM_CHAR
         *                  or KC_NONE (no key pressed).
         */

        case WM_MOUSEMOVE:
            brc = WMMouseMove(pqmsg,
                              &fRestartAutoHide);
        break; // WM_MOUSEMOVE
    }

#ifndef __NOMOVEMENT2FEATURES__
    if (    (fRestartAutoHide)
         || (    (pqmsg->msg >= WM_BUTTONCLICKFIRST)
              && (pqmsg->msg <= WM_BUTTONCLICKLAST)
                  // always restart autohide for mouse buttons events
                  // V1.0.0 (2002-09-05) [lafaix]
            )
       )
        // handle auto-hide V0.9.9 (2001-01-29) [umoeller]
        WMMouseMove_AutoHideMouse();
#endif

    // V0.9.14 (2001-08-21) [umoeller]
    if (G_HookData.cClickWatches)
        switch (pqmsg->msg)
        {
            case WM_BUTTON1DOWN:
            case WM_BUTTON1UP:
            case WM_BUTTON1DBLCLK:
            case WM_BUTTON2DOWN:
            case WM_BUTTON2UP:
            case WM_BUTTON2DBLCLK:
            case WM_BUTTON3DOWN:
            case WM_BUTTON3UP:
            case WM_BUTTON3DBLCLK:
                WinPostMsg(G_HookData.hwndDaemonObject,
                           XDM_MOUSECLICKED,
                           (MPARAM)pqmsg->msg,
                           pqmsg->mp1);             // POINTS pointer pos
        }

    #if 0
        if (    (pqmsg->hwnd == G_HookData.hwndSwitchListCnr)
             && (pqmsg->msg == WM_HACKSWITCHLIST)
             && (pqmsg->mp1 == MP1_HACKSWITCHLIST)
             && (pqmsg->mp2 == MP2_SUBCLASS)
           )
        {
            HackSwitchList(TRUE);

            // swallow
            brc = TRUE;
        }
    #endif

    return brc;                           // msg not processed if FALSE
}

/******************************************************************
 *
 *  Pre-accelerator hook
 *
 ******************************************************************/

/*
 *@@ hookPreAccelHook:
 *      this is the pre-accelerator-table hook. Like
 *      hookInputHook, this gets called when messages
 *      are coming in from the system input queue, but
 *      as opposed to a "regular" input hook, this hook
 *      gets called even before translation from accelerator
 *      tables occurs.
 *
 *      Pre-accel hooks are not documented in the PM Reference.
 *      I have found this idea (and part of the implementation)
 *      in the source code of ProgramCommander/2, so thanks
 *      go out (once again) to Roman Stangl.
 *
 *      In this hook, we check for the global object hotkeys
 *      so that we can use certain key combinations regardless
 *      of whether they might be currently used in application
 *      accelerator tables. This is especially useful for
 *      "Alt" key combinations, which are usually intercepted
 *      when menus have corresponding shortcuts.
 *
 *      As a result, as opposed to other hotkey software you
 *      might know, XWorkplace does properly react to "Ctrl+Alt"
 *      keyboard combinations, even if a menu would get called
 *      with the "Alt" key. ;-)
 *
 *      As with hookInputHook, we return TRUE if the message is
 *      to be swallowed, or FALSE if the current application (or
 *      the next hook in the hook chain) should still receive the
 *      message.
 *
 *@@changed V0.9.3 (2000-04-09) [umoeller]: added check for system lockup
 *@@changed V0.9.3 (2000-04-09) [umoeller]: added XPager hotkeys
 *@@changed V0.9.3 (2000-04-09) [umoeller]: added KC_SCANCODE check
 *@@changed V0.9.3 (2000-04-10) [umoeller]: moved debug code to hook
 */

BOOL EXPENTRY hookPreAccelHook(HAB hab, PQMSG pqmsg, ULONG option)
{
    // set return value:
    // per default, pass message on to next hook or application
    BOOL        brc = FALSE;

    if (pqmsg == NULL)
        return FALSE;

    switch(pqmsg->msg)
    {
        /*
         * WM_CHAR:
         *      keyboard activity. We check for
         *      object hotkeys; if one is found,
         *      we post XWPDAEMN's object window
         *      a notification.
         */

        case WM_CHAR:
            // process this if
            if (
                        // a) object hotkeys are enabled or
#ifndef __ALWAYSOBJHOTKEYS__
                   (    (G_HookData.HookConfig.__fGlobalHotkeys)
#else
                   (    (TRUE)
#endif
#ifndef __NOPAGER__
                        // b) pager switch-screen hotkeys are enabled
                    ||  (G_HookData.PagerConfig.flPager & PGRFL_HOTKEYS)
#else
                    ||  (FALSE)
#endif
                   )
                  // and system is not locked up
               && (!G_HookData.hwndLockupFrame)
               )
            {
                brc = WMChar_Main(pqmsg);
            }

        break; // WM_CHAR
    } // end switch(msg)

    return brc;
}

