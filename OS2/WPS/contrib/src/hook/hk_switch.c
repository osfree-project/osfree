
/*
 *@@sourcefile hk_switch.c:
 *      code for subclassing the switch list.
 *
 *@@added V1.0.0 (2002-09-13) [umoeller]
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

PFNWP G_pfnwpSwitchListOrig = NULL;

/******************************************************************
 *
 *  Subclassed switch list
 *
 ******************************************************************/

#ifdef HACKSWITCHLIST

/*
 *@@ fnwpSubclassedSwitchlist:
 *      window procedure for the subclassed PM window list.
 *
 *      See HackSwitchList() for how this is done.
 *
 *      This is for the window list's frame. This in turn has
 *      a special client which has the container as its child.
 *      See InitializeGlobalsForHooks().
 *
 *@@added V0.9.16 (2002-01-13) [umoeller]
 */

MRESULT EXPENTRY fnwpSubclassedSwitchlist(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    DosBeep(1000, 10);

    switch (msg)
    {
        #if 0
        case WM_HACKSWITCHLIST:
            if (    (mp1 == MP1_HACKSWITCHLIST)
                 && (mp2 == MP2_UNSUBCLASS)
               )
                HackSwitchList(FALSE);
        break;
        #endif

        default:
            mrc = G_pfnwpSwitchListOrig(hwnd, msg, mp1, mp2);
    }

    return mrc;
}

#pragma pack(1)

/*
 *@@ MQ:
 *      first fields of the PM message queue structure,
 *      as far as I was able to decode that.
 *
 *      The "HMQ" returned by WinQueryWindowPtr(QWL_HMQ)
 *      is really a pointer into PM's shared memory.
 *      Yeah, this is really safe. If someone writes
 *      into that area, PM is doomed, but that's what
 *      we need to do.
 *
 *      There are many more fields following, but we
 *      need only the PID, so I didn't bother.
 *
 *@@added V0.9.16 (2002-01-13) [umoeller]
 */

typedef struct _MQ
{
    struct _MQ      *pNext;         // next MQ
    ULONG           cbStruct;       // size of structure
    ULONG           cMessages;      // current message count
    ULONG           cMaxMessages;   // MQ size
    ULONG           ulUnknown1;
    ULONG           ulUnknown2;
    PID             pid;            // process ID of queue
    TID             tid;            // thread ID of queue
} MQ, *PMQ;

#pragma pack()

/*
 *@@ SubclassSwitchList:
 *      very evil hack for doing a WinSubclassWindow on the
 *      PM switch list window that actually works.
 *
 *      Parameters are as with WinSubclassWindow, i.e.
 *      running this with the old window proc will undo
 *      subclassing. Returns the old window proc that
 *      was replaced.
 *
 *      May run on the Shell process only.
 *
 *@@added V0.9.16 (2002-01-13) [umoeller]
 */

PFNWP SubclassWindow(HWND hwnd,
                     PFNWP pfnwpNew)
{
    PFNWP   pfnwpOld = NULL;
    PMQ     pmq;

    // to make WinSetWindowPtr(QWP_PFNWP) work, the processes
    // of the caller and the window must match; actually they
    // do since we're running on the Shell process here, but
    // the message queue of the Shell process has a PID of 0
    // so we must manually hack that!

    // 1) HMQ is really pointer into PM's shared memory
    if (pmq = (PMQ)WinQueryWindowPtr(hwnd,
                                     QWL_HMQ))
    {
        // 2) remember old PID from MQ (should be 0, but to be safe)
        ULONG   pidOld = pmq->pid;

        // 3) get real Shell PID
        PTIB    ptib;
        PPIB    ppib;
        DosGetInfoBlocks(&ptib, &ppib);

        // 4) hack MQ with real Shell PID
        pmq->pid = ppib->pib_ulpid;

        // 5) subclass (WinSubclassWindow doesn't work here)
        pfnwpOld = (PFNWP)WinQueryWindowPtr(hwnd, QWP_PFNWP);
        WinSetWindowPtr(hwnd, QWP_PFNWP, (PVOID)pfnwpNew);

        // 6) restore old PID
        pmq->pid = pidOld;
    }

    return pfnwpOld;
}

/*
 *@@ HackSwitchList:
 *      subclasses the PM window list, or undoes subclassing.
 *
 *      This is a truly evil hack. Simply running WinSubclassWindow
 *      on the switch list doesn't work for two reasons:
 *
 *      1)  The caller must be on the same process as the window.
 *          This is hard to do with the switch list because it
 *          is created by the Shell process (first PMSHELL).
 *
 *          This is why initHook now sends a very strange user
 *          message to the switch list so that hookSendMsgHook
 *          can detect that (hopefully it is really unique) and
 *          run this code in the Shell process.
 *
 *      2)  Instead of WinSubclassWindow, we simply run
 *          WinSetWindowPtr(QWP_PFNWP) on the switch list. From
 *          my testing, that function doesn't perform as many
 *          checks which is why it works on the switch list too.
 *
 *          Still, for QWP_PFNWP, WinSetWindowPtr appears to
 *          check if the processes of the caller and the window's
 *          queue match. Since, for some reason, the shell's
 *          queue has a PID of 0, we need to actually modify
 *          the queue's memory for a second. Yes, very evil.
 *
 *      May run on the Shell process only.
 *
 *@@added V0.9.16 (2002-01-13) [umoeller]
 */

VOID HackSwitchList(BOOL fInstall)
{
    hookLog(__FILE__, __LINE__, __FUNCTION__,
            "fInstall == %d", fInstall);

    if (fInstall && !G_pfnwpSwitchListOrig)
    {
        // "install" mode:

        hookLog(__FILE__, __LINE__, __FUNCTION__,
                "fInstall = TRUE: subclassing switch list");

        // subclass the switch list
        if (G_pfnwpSwitchListOrig = SubclassWindow(G_HookData.hwndSwitchListCnr,
                                                   fnwpSubclassedSwitchlist))
        {
            #if 0
            CNRINFO CnrInfo;

            CnrInfo.cb = sizeof(CnrInfo);
            CnrInfo.flWindowAttr = CV_NAME | CA_DRAWICON | CA_OWNERDRAW;
            WinSendMsg(G_HookData.hwndSwitchListCnr,
                       CM_SETCNRINFO,
                       (MPARAM)&CnrInfo,
                       (MPARAM)CMA_FLWINDOWATTR);
            #endif
        }

        hookLog(__FILE__, __LINE__, __FUNCTION__,
                "WinSubclassWindow returned 0x%lX", G_pfnwpSwitchListOrig);
    }
    else if (!fInstall && G_pfnwpSwitchListOrig)
    {
        PFNWP pfnwp;
        // "deinstall" mode:
        // un-subclass the switch list
        hookLog(__FILE__, __LINE__, __FUNCTION__,
                "fInstall = FALSE: un-subclassing switch list");

        pfnwp = SubclassWindow(G_HookData.hwndSwitchListCnr,
                               G_pfnwpSwitchListOrig);

        hookLog(__FILE__, __LINE__, __FUNCTION__,
                "WinSubclassWindow returned 0x%lX", pfnwp);
    }
}

#endif


