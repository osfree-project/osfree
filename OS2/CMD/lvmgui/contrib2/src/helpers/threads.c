
/*
 *@@sourcefile threads.c:
 *      contains helper functions for creating, destroying, and
 *      synchronizing threads, including PM threads with a
 *      message queue which is created automatically.
 *
 *      See thrCreate() for how to start such a thread.
 *
 *      Usage: All OS/2 programs.
 *
 *      Function prefixes (new with V0.81):
 *      --  thr*        Thread helper functions
 *
 *      This file is new with V0.81 and contains all the thread
 *      functions that used to be in helpers.c.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\threads.h"
 */

/*
 *      Copyright (C) 1997-2002 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_WINMESSAGEMGR
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#include <os2.h>

#include <string.h>
#include <stdlib.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\linklist.h"
#include "helpers\threads.h"

#pragma hdrstop

/*
 *@@category: Helpers\Control program helpers\Thread management
 *      see threads.c.
 */

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

LINKLIST        G_llThreadInfos;
            // linked list of all THREADINFOS ever created...
            // no auto-free
HMTX            G_hmtxThreadInfos = NULLHANDLE;

/* ******************************************************************
 *
 *   Functions
 *
 ********************************************************************/

/*
 *@@ LockThreadInfos:
 *
 *@@added V0.9.9 (2001-03-07) [umoeller]
 */

STATIC BOOL LockThreadInfos(VOID)
{
    APIRET arc = NO_ERROR;

    if (G_hmtxThreadInfos == NULLHANDLE)
    {
        // first call:
        arc = DosCreateMutexSem(NULL,     // unnamed
                                &G_hmtxThreadInfos,
                                0,        // unshared
                                TRUE);    // request!
        lstInit(&G_llThreadInfos, FALSE);
    }
    else
        arc = DosRequestMutexSem(G_hmtxThreadInfos,
                                 SEM_INDEFINITE_WAIT);

    return (arc == NO_ERROR);
}

/*
 *@@ UnlockThreadInfos:
 *
 *@@added V0.9.9 (2001-03-07) [umoeller]
 */

STATIC VOID UnlockThreadInfos(VOID)
{
    DosReleaseMutexSem(G_hmtxThreadInfos);
}

/*
 *@@ thr_fntGeneric:
 *      generic thread function used by thrCreate.
 *      This in turn calls the actual thread function
 *      specified with thrCreate.
 *
 *@@added V0.9.2 (2000-03-05) [umoeller]
 *@@changed V0.9.7 (2000-12-18) [lafaix]: THRF_TRANSIENT support added
 *@@changed V1.0.0 (2002-08-21) [umoeller]: added WinCancelShutdown to avoid shutdown hangs
 */

STATIC VOID _Optlink thr_fntGeneric(PVOID ptiMyself)
{
    PTHREADINFO pti = (PTHREADINFO)ptiMyself;

    if (pti)
    {
        HEV hevExitComplete;

        if (pti->flFlags & THRF_WAIT)
            // "Wait" flag set: thrCreate is then
            // waiting on the wait event sem posted...
            // note: we do not post if THRF_WAIT_EXPLICIT!
            // thread must then post itself
            DosPostEventSem(pti->hevRunning);

        if (pti->flFlags & THRF_PMMSGQUEUE)
        {
            // create msg queue
            if ((pti->hab = WinInitialize(0)))
            {
                if ((pti->hmq = WinCreateMsgQueue(pti->hab, 4000)))
                {
                    // run WinCancelShutdown; this func is used
                    // in XWPDAEMN and causes the default OS/2
                    // shutdown to hang
                    // V1.0.0 (2002-08-21) [umoeller]
                    WinCancelShutdown(pti->hmq, TRUE);

                    // run thread func
                    ((PTHREADFUNC)pti->pThreadFunc)(pti);
                    WinDestroyMsgQueue(pti->hmq);
                }
                WinTerminate(pti->hab);
            }
        }
        else
            // no msgqueue:
            // run thread func without msg queue
            ((PTHREADFUNC)pti->pThreadFunc)(pti);

        if (pti->flFlags & (THRF_WAIT | THRF_WAIT_EXPLICIT))    // V0.9.9 (2001-03-14) [umoeller]
            // "Wait" flag set: delete semaphore
            DosCloseEventSem(pti->hevRunning);

        // V0.9.9 (2001-03-07) [umoeller]
        // remove thread from global list
        if (LockThreadInfos())
        {
            lstRemoveItem(&G_llThreadInfos, pti);
            UnlockThreadInfos();
        }

        // copy event sem before freeing pti
        hevExitComplete = pti->hevExitComplete;

        // set exit flags
        // V0.9.7 (2000-12-20) [umoeller]
        pti->tid = NULLHANDLE;

        if (pti->ptidRunning)
            // clear "running" flag
            *(pti->ptidRunning) = 0;

        // (2000-12-18) [lafaix] clean up pti if thread is transient.
        if (pti->flFlags & THRF_TRANSIENT)
            free(pti);

        if (hevExitComplete)
            // caller wants notification:
            DosPostEventSem(hevExitComplete);
                    // V0.9.16 (2001-12-08) [umoeller]
    }

    // thread func exits
}

/*
 *@@ thrCreate:
 *      this function fills a THREADINFO structure and starts
 *      a new thread using _beginthread.
 *
 *      You must pass the thread function in pfn, which will
 *      then be executed. The thread will be passed a pointer
 *      to the THREADINFO structure as its thread parameter.
 *      The ulData field in that structure is set to ulData
 *      here. Use whatever you like.
 *
 *      The thread function must be declared like this:
 *
 +          void _Optlink fntWhatever(PTHREADINFO ptiMyself)
 *
 *      You should manually specify _Optlink because if the
 *      function is prototyped somewhere, VAC will automatically
 *      modify the function's linkage, and you'll run into
 *      crashes.
 *
 *      The thread's ptiMyself is then a pointer to the
 *      THREADINFO structure passed to this function.
 *      In your thread function, ulData may be obtained like this:
 *
 +          ULONG ulData = ptiMyself->ulData;
 *
 *      thrCreate does not start your thread func directly,
 *      but only through the thr_fntGeneric wrapper to
 *      provide additional functionality. As a consequence,
 *      in your own thread function, NEVER call _endthread
 *      explicitly, because this would skip the exit processing
 *      (cleanup) in thr_fntGeneric. Instead, just fall out of
 *      your thread function, or return().
 *
 *      The THREADINFO structure passed to this function must
 *      be accessible all the time while the thread is running
 *      because the thr* functions will use it for maintenance.
 *      This function does NOT check whether a thread is
 *      already running in *pti. Do not use the same THREADINFO
 *      for several threads.
 *
 *      If you do not want to manage the structure yourself,
 *      you can pass the THRF_TRANSIENT flag (see below).
 *
 *      ptidRunning is an optional parameter and can be NULL.
 *      If non-null, it must point to a ULONG which will contain
 *      the thread's TID while the thread is still running.
 *      Once the thread exits, the ULONG will automatically
 *      be set to zero; this is useful for a quick check whether
 *      the thread is currently busy.
 *
 *      Note: ptidRunning is only reliable if you set the
 *      THRF_WAIT or THRF_WAIT_EXPLICIT flags.
 *
 *      flFlags can be any combination of the following:
 *
 *      -- THRF_PMMSGQUEUE: creates a PM message queue on the
 *         thread. Your thread function will find the HAB and
 *         the HMQ in its THREADINFO. These are automatically
 *         destroyed when the thread terminates.
 *
 *         WARNING: Be careful with this setting for short-lived
 *         threads which are started synchronously while some
 *         other thread is locking PM, e.g. in a WinSendMsg
 *         call. While creation of the thread will still
 *         succeed, the thread will _not_ terminate properly
 *         while PM is locked, so do not wait for such a thread
 *         to terminate!! (V0.9.16)
 *
 *      -- THRF_WAIT: if this is set, thrCreate does not
 *         return to the caller until your thread function
 *         has successfully started running. This is done by
 *         waiting on an event semaphore which is automatically
 *         posted by thr_fntGeneric. This is useful for the
 *         typical PM "Worker" thread where you need to disable
 *         menu items on thread 1 while the thread is running.
 *
 *      -- THRF_WAIT_EXPLICIT: like THRF_WAIT, but in this case,
 *         your thread function _must_ post THREADINFO.hevRunning
 *         yourself (thr_fntGeneric will not automatically
 *         post it). Useful for waiting until your own thread
 *         function is fully initialized, e.g. if it creates
 *         an object window.
 *
 *         WARNING: if your thread forgets to post this, we'll
 *         hang.
 *
 *         Added V0.9.9 (2001-03-14) [umoeller].
 *
 *      -- THRF_TRANSIENT: creates a "transient" thread where
 *         pti may be NULL. A THREADINFO structure is then
 *         allocated from the heap internally, but not visible
 *         to the caller.
 *
 *      This now (V0.9.9) returns the TID of the new thread or
 *      null on errors.
 *
 *@@changed V0.9.0 [umoeller]: default stack size raised for Watcom (thanks, Rdiger Ihle)
 *@@changed V0.9.0 [umoeller]: _beginthread is now only called after all variables have been set (thanks, Rdiger Ihle)
 *@@changed V0.9.2 (2000-03-04) [umoeller]: added stack size parameter
 *@@changed V0.9.2 (2000-03-06) [umoeller]: now using thr_fntGeneric; thrGoodbye is no longer needed
 *@@changed V0.9.3 (2000-04-29) [umoeller]: removed stack size param; added fCreateMsgQueue
 *@@changed V0.9.3 (2000-05-01) [umoeller]: added pbRunning and flFlags
 *@@changed V0.9.5 (2000-08-26) [umoeller]: now using PTHREADINFO
 *@@changed V0.9.7 (2000-12-18) [lafaix]: THRF_TRANSIENT support added
 *@@changed V0.9.9 (2001-02-06) [umoeller]: now returning TID
 *@@changed V0.9.9 (2001-03-07) [umoeller]: added pcszThreadName
 *@@changed V0.9.9 (2001-03-14) [umoeller]: added THRF_WAIT_EXPLICIT
 *@@changed V0.9.12 (2001-05-20) [umoeller]: changed pfRunning to ptidRunning
 *@@changed V0.9.16 (2001-10-28) [umoeller]: made ptidRunning volatile to prohibit compiler optimizations
 */

ULONG thrCreate(PTHREADINFO pti,     // out: THREADINFO data
                PTHREADFUNC pfn,     // in: _Optlink thread function
                volatile unsigned long *ptidRunning,
                                     // out: variable set to TID while thread is running;
                                     // ptr can be NULL
                const char *pcszThreadName, // in: thread name (for identification)
                ULONG flFlags,       // in: THRF_* flags
                ULONG ulData)        // in: user data to be stored in THREADINFO
{
    ULONG ulrc = 0;     // V0.9.9 (2001-02-06) [umoeller]

    // (2000-12-18) [lafaix] TRANSIENT
    if (flFlags & THRF_TRANSIENT)
    {
        if (pti == NULL)
            pti = (PTHREADINFO)malloc(sizeof(THREADINFO));
                    // cleaned up by thr_fntGeneric on exit
    }

    if (pti)
    {
        memset(pti, 0, sizeof(THREADINFO));
        pti->cbStruct = sizeof(THREADINFO);
        pti->pThreadFunc = (PVOID)pfn;
        pti->ptidRunning = ptidRunning;
        pti->pcszThreadName = pcszThreadName; // V0.9.9 (2001-03-07) [umoeller]
        pti->flFlags = flFlags;
        pti->ulData = ulData;

        if (flFlags & (THRF_WAIT | THRF_WAIT_EXPLICIT)) // V0.9.9 (2001-03-14) [umoeller]
            // "Wait" flag set: create an event semaphore which
            // will be posted by thr_fntGeneric (THRF_WAIT only)
            if (DosCreateEventSem(NULL,     // unnamed
                                  &pti->hevRunning,
                                  0,        // unshared
                                  FALSE)    // not posted (reset)
                    != NO_ERROR)
            {
                if (flFlags & THRF_TRANSIENT)
                    free(pti);

                // stop right here
                pti = NULL;
            }

        if (pti)
        {
            pti->tid = _beginthread(        // moved, V0.9.0 (hint: Rdiger Ihle)
                                    thr_fntGeneric, // changed V0.9.2 (2000-03-06) [umoeller]
                                    0,      // unused compatibility param
                                    3*96000, // plenty of stack
                                    pti);   // parameter passed to thread
            ulrc = pti->tid;

            if (pti->ptidRunning)
                // set "running" flag // V0.9.12 (2001-05-20) [umoeller]
                *(pti->ptidRunning) = pti->tid;

            if (ulrc)
            {
                if (LockThreadInfos())
                {
                    lstAppendItem(&G_llThreadInfos, pti);
                    UnlockThreadInfos();
                }

                if (flFlags & (THRF_WAIT | THRF_WAIT_EXPLICIT))
                {
                    // "Wait" flag set: wait on event semaphore
                    DosWaitEventSem(pti->hevRunning,
                                    SEM_INDEFINITE_WAIT);
                }
            }
        }
    }

    return ulrc;
}

/*
 *@@ thrRunSync:
 *      runs the specified thread function synchronously.
 *
 *      This is a wrapper around thrCreate. However, this
 *      function does not return until the thread function
 *      finishes. This creates a modal message loop on the
 *      calling thread so that the PM message queue is not
 *      blocked while the thread is running. Naturally this
 *      implies that the calling thread has a message queue.
 *
 *      As a special requirement, your thread function (pfn)
 *      must post WM_USER to THREADINFO.hwndNotify just before
 *      exiting. The mp1 value of WM_USER will then be returned
 *      by this function.
 *
 *@@added V0.9.5 (2000-08-26) [umoeller]
 *@@changed V0.9.9 (2001-03-07) [umoeller]: added pcszThreadName
 *@@changed V0.9.16 (2001-12-08) [umoeller]: fixed hang with thrWait
 */

ULONG thrRunSync(HAB hab,               // in: anchor block of calling thread
                 PTHREADFUNC pfn,       // in: passed to thrCreate
                 const char *pcszThreadName, // in: passed to thrCreate
                 ULONG ulData)          // in: passed to thrCreate
{
    ULONG ulrc = 0;
    QMSG qmsg;
    BOOL fQuit = FALSE;
    HWND hwndNotify;
    if (hwndNotify = WinCreateWindow(HWND_OBJECT,
                                     WC_BUTTON,
                                     (PSZ)"",
                                     0,
                                     0,0,0,0,
                                     0,
                                     HWND_BOTTOM,
                                     0,
                                     0,
                                     NULL))
    {
        THREADINFO  ti = {0};
        volatile unsigned long tidRunning = 0;
        thrCreate(&ti,
                  pfn,
                  &tidRunning,
                  pcszThreadName,
                  THRF_PMMSGQUEUE,
                  ulData);
        ti.hwndNotify = hwndNotify;
        // create event sem to wait on V0.9.16 (2001-12-08) [umoeller]
        DosCreateEventSem(NULL,
                          &ti.hevExitComplete,
                          0,
                          FALSE);       // not posted

        while (WinGetMsg(hab,
                         &qmsg, 0, 0, 0))
        {
            // current message for our object window?
            if (    (qmsg.hwnd == hwndNotify)
                 && (qmsg.msg == WM_USER)
               )
            {
                fQuit = TRUE;
                ulrc = (ULONG)qmsg.mp1;
            }

            WinDispatchMsg(hab, &qmsg);
            if (fQuit)
                break;
        }

        // we must wait for the thread to finish, or
        // otherwise THREADINFO is deleted from the stack
        // before the thread exits... will crash!
        // thrWait(&ti);
        // now using event sem V0.9.16 (2001-12-08) [umoeller]
        WinWaitEventSem(ti.hevExitComplete, 5000);
        DosCloseEventSem(ti.hevExitComplete);

        WinDestroyWindow(hwndNotify);
    }

    return ulrc;
}

/*
 *@@ thrListThreads:
 *      returns an array of THREADINFO structures
 *      for all threads that have been started using
 *      thrCreate (or thrRunSync).
 *
 *      If no threads are running yet, this returns
 *      NULL.
 *
 *      Otherwise, this returns the pointer to the
 *      first array item, and *pcThreads receives
 *      the array item count (NOT the total array
 *      size). The array is a copied snapshot of all
 *      current THREADINFO's and must be free()'d
 *      by the caller.
 *
 *@@added V0.9.9 (2001-03-07) [umoeller]
 */

PTHREADINFO thrListThreads(PULONG pcThreads)
{
    PTHREADINFO pArray = 0;

    if (LockThreadInfos())
    {
        PTHREADINFO pThis;
        PLISTNODE pNode;
        *pcThreads = lstCountItems(&G_llThreadInfos);
        if (pArray = (PTHREADINFO)malloc(*pcThreads * sizeof(THREADINFO)))
        {
            pThis = pArray;

            pNode = lstQueryFirstNode(&G_llThreadInfos);
            while (pNode)
            {
                memcpy(pThis,
                       (PTHREADINFO)pNode->pItemData,
                       sizeof(THREADINFO));
                pThis++;
                pNode = pNode->pNext;
            }
        }

        UnlockThreadInfos();
    }

    return pArray;
}

/*
 *@@ thrFindThread:
 *      attempts to find the thread with the specified
 *      TID; if found, returns TRUE and copies its
 *      THREADINFO into *pti.
 *
 *@@added V0.9.9 (2001-03-07) [umoeller]
 */

BOOL thrFindThread(PTHREADINFO pti,
                   ULONG tid)
{
    BOOL brc = FALSE;
    if (LockThreadInfos())
    {
        PLISTNODE pNode = lstQueryFirstNode(&G_llThreadInfos);
        while (pNode)
        {
            PTHREADINFO ptiThis = (PTHREADINFO)pNode->pItemData;
            if (ptiThis->tid == tid)
            {
                memcpy(pti, ptiThis, sizeof(THREADINFO));
                brc = TRUE;
                break;
            }
            pNode = pNode->pNext;
        }

        UnlockThreadInfos();
    }

    return brc;
}

/*
 *@@ thrClose:
 *      this functions sets the "fExit" flag in
 *      THREADINFO to TRUE.
 *
 *      The thread should monitor this flag
 *      periodically and then terminate itself.
 */

BOOL thrClose(PTHREADINFO pti)
{
    if (pti)
    {
        pti->fExit = TRUE;
        return TRUE;
    }
    return FALSE;
}

/*
 *@@ thrWait:
 *      this function waits for a thread to end by calling
 *      DosWaitThread. Note that this blocks the calling
 *      thread, so only use this function when you're sure
 *      the thread will actually terminate.
 *
 *      Update V0.9.16: Do not use this with PM theads at
 *      all. DosWaitThread can hang the system then.
 *
 *      Returns FALSE if the thread wasn't running or TRUE
 *      if it was and has terminated.
 *
 *@@changed V0.9.0 [umoeller]: now checking for whether pti->tid is still != 0
 */

BOOL thrWait(PTHREADINFO pti)
{
    if (pti)
        if (pti->tid)
        {
            DosWaitThread(&pti->tid, DCWW_WAIT);
            pti->tid = NULLHANDLE;
            return TRUE;
        }
    return FALSE;
}

/*
 *@@ thrFree:
 *      this is a combination of thrClose and
 *      thrWait, i.e. this func does not return
 *      until the specified thread has ended.
 */

BOOL thrFree(PTHREADINFO pti)
{
    if (pti->tid)
    {
        thrClose(pti);
        thrWait(pti);
    }
    return TRUE;
}

/*
 *@@ thrKill:
 *      just like thrFree, but the thread is
 *      brutally killed, using DosKillThread.
 *
 *      Note: DO NOT USE THIS. DosKillThread
 *      cannot clean up the C runtime. In the
 *      worst case, this hangs the system
 *      because the runtime hasn't released
 *      a semaphore or something like that.
 */

BOOL thrKill(PTHREADINFO pti)
{
    if (pti->tid)
    {
        DosResumeThread(pti->tid);
            // this returns an error if the thread
            // is not suspended, but otherwise the
            // system might hang
        DosKillThread(pti->tid);
    }
    return TRUE;
}

/*
 *@@ thrQueryID:
 *      returns thread ID or NULLHANDLE if
 *      the specified thread is not or no
 *      longer running.
 */

TID thrQueryID(const THREADINFO* pti)
{
    if (pti)
        return pti->tid;

    return NULLHANDLE;
}

/*
 *@@ thrQueryPriority:
 *      returns the priority of the calling thread.
 *      The low byte of the low word is a hexadecimal value
 *      representing a rank (value 0 to 31) within a priority class.
 *      Class values, found in the high byte of the low word, are
 *      as follows:
 *      --  0x01  idle
 *      --  0x02  regular
 *      --  0x03  time-critical
 *      --  0x04  server
 *
 *      Note: This cannot retrieve the priority of a
 *      thread other than the one on which this function
 *      is running. Use prc16QueryThreadInfo for that.
 */

ULONG thrQueryPriority(VOID)
{
    PTIB    ptib;
    PPIB    ppib;
    if (DosGetInfoBlocks(&ptib, &ppib) == NO_ERROR)
        if (ptib)
            if (ptib->tib_ptib2)
                return ptib->tib_ptib2->tib2_ulpri;
    return 0;
}


