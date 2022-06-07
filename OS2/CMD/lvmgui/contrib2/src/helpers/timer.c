
/*
 *@@sourcefile timer.c:
 *      XTimers, which can be used to avoid excessive PM
 *      timer usage.
 *
 *      The functions here allow you to share a single PM
 *      timer for many windows on the same thread. Basically,
 *      these start a "master PM timer", to whose WM_TIMER
 *      message your window procedure must respond by calling
 *      tmrTimerTick, which will "distribute" the timer tick
 *      to those XTimers which have elapsed.
 *
 *      The main advantage of the XTimers is that these
 *      are not a limited resource (as opposed to PM timers).
 *      I don't know how many PM timers exist on the system,
 *      but PMREF says that the no. of remaining timers can
 *      be queried with WinQuerySysValue(SV_CTIMERS).
 *
 *      XTimers are used excessively by the default widgets
 *      of XWorkplace's XCenter.
 *
 *      There are a few limitations with the XTimers though:
 *
 *      --  tmrTimerTick (which you must call when the
 *          "master" WM_TIMER comes in) does not post WM_TIMER
 *          messages to the windows specified in the subtimers,
 *          but calls the window procedure of the target window
 *          directly. This makes sure that timers work even if
 *          some thread is hogging the SIQ.
 *
 *          This however requires that all XTimers must run on
 *          the same thread as the owner window of the master
 *          timer which was specified with tmrCreateSet.
 *
 *      --  Queue timers (with HWND == NULLHANDLE) are not
 *          supported.
 *
 *      --  When a window is destroyed, its timers are not
 *          automatically cleaned up. tmrTimerTick does
 *          detect invalid windows and removes them from the
 *          timers list before posting, but to be on the safe
 *          side, always call tmrStopAllTimers when WM_DESTROY
 *          comes into a window which has used timers.
 *
 *      So to use XTimers, do the following:
 *
 *      1.  Create a timer set with tmrCreateSet. Specify
 *          an owner window and the timer ID of the master
 *          PM timer.
 *
 *      2.  You can then start and stop XTimers for windows
 *          on the same thread by calling tmrStartXTimer and
 *          tmrStopXTimer, respectively.
 *
 *      3.  In the window proc of the owner window, respond
 *          to WM_TIMER for the master PM timer by calling
 *          tmrTimerTick. This will call the window procs
 *          of those windows with WM_TIMER messages directly
 *          whose XTimers have elapsed.
 *
 *      Function prefixes:
 *      --  tmr*   timer functions
 *
 *@@header "helpers\timer.h"
 *@@added V0.9.7 (2000-12-04) [umoeller]
 */

/*
 *      Copyright (C) 2000-2005 Ulrich M”ller.
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

// OS2 includes

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINTIMER
#include <os2.h>

#include <stdio.h>
#include <setjmp.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\datetime.h"
#include "helpers\except.h"
#include "helpers\linklist.h"
#include "helpers\math.h"
#include "helpers\standards.h"
#include "helpers\threads.h"
#include "helpers\timer.h"

// #define DEBUG_XTIMERS

/*
 *@@category: Helpers\PM helpers\Timer replacements
 *      see timer.c.
 */

/* ******************************************************************
 *
 *   Private declarations
 *
 ********************************************************************/

/*
 *@@ XTIMER:
 *      one of these represents an XTimer.
 *      These are stored in a linked list in
 *      an _XTIMERSET.
 */

typedef struct _XTIMER
{
    USHORT     usTimerID;           // timer ID, as passed to tmrStartXTimer
    HWND       hwndTarget;          // target window, as passed to tmrStartXTimer
    ULONG      ulTimeout;           // timer's timeout (in ms)
    ULONG      ulNextFire;          // next time scalar (from dtGetULongTime) to fire at
} XTIMER, *PXTIMER;

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

HMTX                G_hmtxTimers = NULLHANDLE;  // timers lock mutex

/* ******************************************************************
 *
 *   Private functions
 *
 ********************************************************************/

/*
 *@@ LockTimers:
 *
 *@@added V0.9.12 (2001-05-12) [umoeller]
 */

STATIC BOOL LockTimers(VOID)
{
    if (!G_hmtxTimers)
        return !DosCreateMutexSem(NULL,
                                  &G_hmtxTimers,
                                  0,
                                  TRUE);      // request!

    return !DosRequestMutexSem(G_hmtxTimers, SEM_INDEFINITE_WAIT);
}

/*
 *@@ UnlockTimers:
 *
 *@@added V0.9.12 (2001-05-12) [umoeller]
 */

STATIC VOID UnlockTimers(VOID)
{
    DosReleaseMutexSem(G_hmtxTimers);
}

/*
 *@@ FindTimer:
 *      returns the XTIMER structure from the global
 *      linked list which matches the given window
 *      _and_ timer ID.
 *
 *      Internal function. Caller must hold the mutex.
 */

STATIC PXTIMER FindTimer(PXTIMERSET pSet,          // in: timer set (from tmrCreateSet)
                         HWND hwnd,                // in: timer target window
                         USHORT usTimerID)         // in: timer ID
{
    PLINKLIST pllXTimers;
    if (    (pSet)
         && (pllXTimers = (PLINKLIST)pSet->pvllXTimers)
       )
    {
        PLISTNODE pNode = lstQueryFirstNode(pllXTimers);
        while (pNode)
        {
            PXTIMER pTimer = (PXTIMER)pNode->pItemData;
            if (    (pTimer->usTimerID == usTimerID)
                 && (pTimer->hwndTarget == hwnd)
               )
            {
                return pTimer;
            }

            pNode = pNode->pNext;
        }
    }

    return NULL;
}

/*
 *@@ RemoveTimer:
 *      removes the specified XTIMER structure from
 *      the global linked list of running timers.
 *
 *      Internal function. Caller must hold the mutex.
 */

STATIC VOID RemoveTimer(PXTIMERSET pSet,       // in: timer set (from tmrCreateSet)
                        PXTIMER pTimer)        // in: timer to remove
{
    PLINKLIST pllXTimers;
    if (    (pSet)
         && (pllXTimers = (PLINKLIST)pSet->pvllXTimers)
       )
    {
        #ifdef DEBUG_XTIMERS
        _Pmpf((__FUNCTION__ ": removing timer %d", pTimer->usTimerID));
        #endif
        lstRemoveItem(pllXTimers,
                      pTimer);       // auto-free!
    }
}

/*
 *@@ AdjustPMTimer:
 *      goes thru all XTimers in the sets and starts
 *      or stops the PM timer with a decent frequency.
 *
 *      Internal function. Caller must hold the mutex.
 *
 *@@added V0.9.9 (2001-03-07) [umoeller]
 *@@changed V0.9.14 (2001-07-07) [umoeller]: added GCD optimizations
 */

STATIC VOID AdjustPMTimer(PXTIMERSET pSet)
{
    PLINKLIST pllXTimers = (PLINKLIST)pSet->pvllXTimers;
    ULONG   cTimers = lstCountItems(pllXTimers);

    #ifdef DEBUG_XTIMERS
        _Pmpf((__FUNCTION__ ": entering"));
    #endif

    if (!cTimers)
    {
        // no XTimers running:
        if (pSet->idPMTimerRunning)
        {
            // but PM timer running:
            // stop it
            WinStopTimer(pSet->hab,
                         pSet->hwndOwner,
                         pSet->idPMTimer);
            pSet->idPMTimerRunning = 0;
        }

        pSet->ulPMTimeout = 0;
    }
    else
    {
        // we have timers:

        ULONG       ulOldPMTimeout = pSet->ulPMTimeout;

        PLISTNODE   pNode = lstQueryFirstNode(pllXTimers);
        PXTIMER     pTimer1 = (PXTIMER)pNode->pItemData;

        if (cTimers == 1)
        {
            // only one timer:
            // that's easy
            #ifdef DEBUG_XTIMERS
                _Pmpf(("  got 1 timer"));
            #endif

            pSet->ulPMTimeout = pTimer1->ulTimeout;
        }
        else if (cTimers == 2)
        {
            // exactly two timers:
            // find the greatest common denominator
            PXTIMER pTimer2 = (PXTIMER)pNode->pNext->pItemData;
            #ifdef DEBUG_XTIMERS
                _Pmpf(("  got 2 timers"));
            #endif

            pSet->ulPMTimeout = mathGCD(pTimer1->ulTimeout,
                                        pTimer2->ulTimeout);
        }
        else
        {
            // more than two timers:
            // run through all timers and find the greatest
            // common denominator of all frequencies...
            int     *paInts = (int*)_alloca(sizeof(int) * cTimers),
                    i = 0;

            #ifdef DEBUG_XTIMERS
                _Pmpf(("  got %d timers", cTimers));
            #endif

            // fill an array of integers with the
            // timer frequencies
            while (pNode)
            {
                pTimer1 = (PXTIMER)pNode->pItemData;

                #ifdef DEBUG_XTIMERS
                    _Pmpf(("    timeout %d is %d", i, pTimer1->ulTimeout));
                #endif

                paInts[i++] = pTimer1->ulTimeout;

                pNode = pNode->pNext;
            }

            pSet->ulPMTimeout = mathGCDMulti(paInts,
                                             cTimers);
        }

        #ifdef DEBUG_XTIMERS
            _Pmpf(("--> GCD is %d", pSet->ulPMTimeout));
        #endif

        if (    (!pSet->idPMTimerRunning)       // timer not running?
             || (pSet->ulPMTimeout != ulOldPMTimeout) // timeout changed?
           )
        {
            // start or restart PM timer
            pSet->idPMTimerRunning = WinStartTimer(pSet->hab,
                                                   pSet->hwndOwner,
                                                   pSet->idPMTimer,
                                                   pSet->ulPMTimeout);
        }
    }
}

/* ******************************************************************
 *
 *   Exported functions
 *
 ********************************************************************/

/*
 *@@ tmrCreateSet:
 *      creates a "timer set" for use with the XTimer functions.
 *      This is the first step if you want to use the XTimers.
 *
 *      hwndOwner must specify the "owner window", the target
 *      window for the master PM timer. This window must respond
 *      to a WM_TIMER message with the specified usPMTimerID and
 *      invoke tmrTimerTick then.
 *
 *      Note that the master timer is not started until an XTimer
 *      is started.
 *
 *      Use tmrDestroySet to free all resources again.
 *
 *@@added V0.9.9 (2001-02-28) [umoeller]
 */

PXTIMERSET tmrCreateSet(HWND hwndOwner,         // in: owner window
                        USHORT usPMTimerID)
{
    PXTIMERSET pSet = NEW(XTIMERSET);
    if (pSet)
    {
        pSet->hab = WinQueryAnchorBlock(hwndOwner);
        pSet->hwndOwner = hwndOwner;
        pSet->idPMTimer = usPMTimerID;
        pSet->idPMTimerRunning = 0;
        pSet->ulPMTimeout = 0;

        pSet->pvllXTimers = (PVOID)lstCreate(TRUE);
    }

    return pSet;
}

/*
 *@@ tmrDestroySet:
 *      destroys a timer set previously created using
 *      tmrCreateSet.
 *
 *      Of course, this will stop all XTimers which
 *      might still be running with this set.
 *
 *@@added V0.9.9 (2001-02-28) [umoeller]
 *@@changed V0.9.12 (2001-05-12) [umoeller]: added mutex protection
 */

VOID tmrDestroySet(PXTIMERSET pSet)     // in: timer set (from tmrCreateSet)
{
    if (LockTimers())
    {
        if (pSet)
        {
            PLINKLIST pllXTimers;
            if (pllXTimers = (PLINKLIST)pSet->pvllXTimers)
            {
                PLISTNODE pTimerNode = lstQueryFirstNode(pllXTimers);
                while (pTimerNode)
                {
                    PLISTNODE pNext = pTimerNode->pNext;
                    PXTIMER pTimer = (PXTIMER)pTimerNode->pItemData;
                    RemoveTimer(pSet, pTimer);
                    pTimerNode = pNext;
                }

                lstFree(&pllXTimers);
                pSet->pvllXTimers = NULL;
            }

            if (pSet->idPMTimerRunning)
            {
                WinStopTimer(pSet->hab,
                             pSet->hwndOwner,
                             pSet->idPMTimer);
                pSet->idPMTimerRunning = 0;
            }

            free(pSet);
        }

        UnlockTimers();
    }
}

/*
 *@@ tmrTimerTick:
 *      implements a PM timer tick.
 *
 *      When your window procs receives WM_TIMER for the
 *      one PM timer which is supposed to trigger all the
 *      XTimers, it must call this function. This will
 *      evaluate all XTimers on the list and "fire" them
 *      by calling the window procs directly with the
 *      WM_TIMER message.
 *
 *@@added V0.9.9 (2001-02-28) [umoeller]
 *@@changed V0.9.12 (2001-05-12) [umoeller]: added mutex protection
 *@@changed V0.9.12 (2001-05-24) [umoeller]: fixed crash if this got called during tmrTimerTick
 *@@changed V0.9.14 (2001-08-01) [umoeller]: fixed mem overwrite which might have caused crashes if this got called during tmrTimerTick
 *@@changed V0.9.14 (2001-08-03) [umoeller]: fixed "half frequency" regression caused by frequency optimizations
 *@@changed V0.9.16 (2001-12-18) [umoeller]: now using WinDispatchMsg to avoid crashes during win destruction
 *@@changed V0.9.19 (2002-05-04) [umoeller]: added excpt handling to avoid hanging all timers on the mutex
 */

VOID tmrTimerTick(PXTIMERSET pSet)      // in: timer set (from tmrCreateSet)
{
    volatile BOOL fLocked = FALSE; // XWP V1.0.4 (2005-10-09) [pr]

    TRY_LOUD(excpt1)
    {
        if (fLocked = LockTimers())
        {
            PLINKLIST pllXTimers;
            if (    (pSet)
                 && (pllXTimers = (PLINKLIST)pSet->pvllXTimers)
               )
            {
                // go thru all XTimers and see which one
                // has elapsed; for all of these, post WM_TIMER
                // to the target window proc
                PLISTNODE pTimerNode;

                if (!(pTimerNode = lstQueryFirstNode(pllXTimers)))
                {
                    // no timers left:
                    if (pSet->idPMTimerRunning)
                    {
                        // but PM timer running:
                        // stop it
                        WinStopTimer(pSet->hab,
                                     pSet->hwndOwner,
                                     pSet->idPMTimer);
                        pSet->idPMTimerRunning = 0;
                    }

                    pSet->ulPMTimeout = 0;
                }
                else
                {
                    // we have timers:
                    BOOL    fFoundInvalid = FALSE;

                    // get current time
                    ULONG   ulTimeNow = 0;
                    DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT,
                                    &ulTimeNow, sizeof(ulTimeNow));

                    #ifdef DEBUG_XTIMERS
                        _Pmpf((__FUNCTION__ ": ulTimeNow = %d", ulTimeNow));
                    #endif

                    while (pTimerNode)
                    {
                        // get next node first because the
                        // list can get modified while processing
                        // V0.9.12 (2001-05-24) [umoeller]
                        PLISTNODE pNext = pTimerNode->pNext;

                        PXTIMER pTimer = (PXTIMER)pTimerNode->pItemData;

                        #ifdef DEBUG_XTIMERS
                            _Pmpf(("   timer %d: ulNextFire = %d",
                                    lstIndexFromItem(pllXTimers, pTimer),
                                    pTimer->ulNextFire));
                        #endif

                        if (    (pTimer)
                             // && (pTimer->ulNextFire < ulTimeNow)
                                // V0.9.14 (2001-08-01) [umoeller]
                                // use <= because otherwise we'll get
                                // only half the frequency...
                                // we get here frequently where the
                                // two values are EXACTLY equal due
                                // to the above optimization (DosQuerySysInfo
                                // called once only for the entire loop)
                             && (pTimer->ulNextFire <= ulTimeNow)
                           )
                        {
                            // this timer has elapsed:
                            // fire!

                            #ifdef DEBUG_XTIMERS
                                _Pmpf(("   --> fire!"));
                            #endif

                            if (WinIsWindow(pSet->hab,
                                            pTimer->hwndTarget))
                            {
                                // window still valid:
                                // get the window's window proc
                                QMSG qmsg;
                                PFNWP pfnwp = (PFNWP)WinQueryWindowPtr(pTimer->hwndTarget,
                                                                       QWP_PFNWP);

                                // moved this up V0.9.14 (2001-08-01) [umoeller]
                                pTimer->ulNextFire = ulTimeNow + pTimer->ulTimeout;

                                // call the window proc DIRECTLY
                                // V0.9.16 (2001-12-18) [umoeller]:
                                // now using WinDispatchMsg to avoid crashes
                                // while hwndTarget is being destroyed
                                /* qmsg.hwnd = pTimer->hwndTarget;
                                qmsg.msg = WM_TIMER;
                                qmsg.mp1 = (MPARAM)pTimer->usTimerID;
                                qmsg.mp2 = (MPARAM)0;
                                qmsg.time = 0;
                                qmsg.ptl.x = 0;
                                qmsg.ptl.y = 0;
                                qmsg.reserved = 0;
                                WinDispatchMsg(pSet->hab,
                                               &qmsg); */

                                pfnwp(pTimer->hwndTarget,
                                      WM_TIMER,
                                      (MPARAM)pTimer->usTimerID,
                                      0);
                                    // V0.9.12 (2001-05-24) [umoeller]
                                    // if the winproc chooses to start or
                                    // stop a timer, pNext still points
                                    // to a valid node...
                                    // -- if a timer is removed, that's OK
                                    // -- if a timer is added, it is added to
                                    //    the list, so we'll see it in this loop

                                // V0.9.14 (2001-08-01) [umoeller]

                                // DO NOT REFERENCE pTimer AFTER THIS CODE;
                                // the winproc might have removed the timer,
                                // and since the list is auto-free, pTimer
                                // might have been freed!!
                            }
                            else
                            {
                                // window has been destroyed:
                                lstRemoveNode(pllXTimers,
                                              pTimerNode);
                                    // pNext is still valid

                                fFoundInvalid = TRUE;
                            }

                        } // end if (pTimer->ulNextFire < ulTimeNow)

                        // next timer
                        pTimerNode = pNext; // V0.9.12 (2001-05-24) [umoeller]
                    } // end while (pTimerNode)

                    // destroy invalid timers, if any
                    if (fFoundInvalid)
                        AdjustPMTimer(pSet);

                } // end else if (!pTimerNode)
            } // end if (pllXTimers)
        } // end if (LockTimers())
    }
    CATCH(excpt1) {} END_CATCH();

    if (fLocked)
        UnlockTimers();
}

/*
 *@@ tmrStartXTimer:
 *      starts an XTimer.
 *
 *      Any window can request an XTimer using
 *      this function. This operates similar to
 *      WinStartTimer, except that the number of
 *      XTimers is not limited.
 *
 *      Returns the ID of a new timer or resets an
 *      existing timer (if usTimerID is already used
 *      with hwnd). Use tmrStopXTimer to stop the timer.
 *
 *      Returns 0 if an error occurred. It is thus
 *      invalid to specify a timer ID of 0.
 *
 *      The timer is _not_ stopped automatically
 *      when the window is destroyed.
 *
 *      Note: Unless you own the timer set that
 *      your timer runs on, it is strongly recommended
 *      that your timer frequency is set to a multiple
 *      of 125. The PM master timer behind the timer
 *      set will be set to the greatest common divisor
 *      of all frequencies, and if you set one timer
 *      to 2000 and the other one to 2001, you will
 *      cause quite a lot of overhead. This applies
 *      especially to timers started by XCenter widgets.
 *
 *      For security, all timer frequencies will be
 *      rounded to multiples of 25 anyway. Still,
 *      running two timers at 1000 and 1025 will cause
 *      the master timer to be set to 25, which is
 *      overkill.
 *
 *@@changed V0.9.7 (2000-12-08) [umoeller]: got rid of dtGetULongTime
 *@@changed V0.9.12 (2001-05-12) [umoeller]: added mutex protection
 *@@changed V0.9.14 (2001-07-12) [umoeller]: now rounding freq's to multiples of 25
 */

USHORT XWPENTRY tmrStartXTimer(PXTIMERSET pSet, // in: timer set (from tmrCreateSet)
                               HWND hwnd,       // in: target window for XTimer
                               USHORT usTimerID, // in: timer ID for XTimer's WM_TIMER (must be > 0)
                               ULONG ulTimeout) // in: XTimer's timeout
{
    USHORT  usrc = 0;

    // _Pmpf((__FUNCTION__ ": entering"));

    if (LockTimers())
    {
        PLINKLIST pllXTimers;
        if (    (pSet)
             && (pllXTimers = (PLINKLIST)pSet->pvllXTimers)
             && (hwnd)
             && (ulTimeout)
             && (usTimerID)     // V0.9.16 (2001-12-18) [umoeller]
           )
        {
            PXTIMER pTimer;
            ULONG ulTimeNow;

            // fix the timeout... we allow only multiples of
            // 25, and it must be at least 25 (otherwise our
            // internal master timer calculations will fail)
            // V0.9.14 (2001-07-07) [umoeller]
            if (ulTimeout < 25)
                ulTimeout = 25;
            else
                ulTimeout = (ulTimeout + 10) / 25 * 25;

            DosQuerySysInfo(QSV_MS_COUNT,
                            QSV_MS_COUNT,
                            &ulTimeNow,
                            sizeof(ulTimeNow));

            // check if this timer exists already
            if (pTimer = FindTimer(pSet,
                                   hwnd,
                                   usTimerID))
            {
                // exists already: reset only
                pTimer->ulNextFire = ulTimeNow + ulTimeout;
                usrc = usTimerID;
            }
            else
            {
                // new timer needed:
                if (pTimer = NEW(XTIMER))
                {
                    pTimer->usTimerID = usTimerID;
                    pTimer->hwndTarget = hwnd;
                    pTimer->ulTimeout = ulTimeout;
                    pTimer->ulNextFire = ulTimeNow + ulTimeout;

                    lstAppendItem(pllXTimers,
                                  pTimer);
                    usrc = usTimerID;
                }
            }

            if (usrc)
                // timer created or reset:
                AdjustPMTimer(pSet);

        } // if ((hwnd) && (ulTimeout))

        UnlockTimers();
    }

    return usrc;
}

/*
 *@@ tmrStopXTimer:
 *      similar to WinStopTimer, this stops the
 *      specified timer (which must have been
 *      started with the same hwnd and usTimerID
 *      using tmrStartXTimer).
 *
 *      Returns TRUE if the timer was stopped.
 *
 *@@changed V0.9.12 (2001-05-12) [umoeller]: added mutex protection
 */

BOOL XWPENTRY tmrStopXTimer(PXTIMERSET pSet,    // in: timer set (from tmrCreateSet)
                            HWND hwnd,
                            USHORT usTimerID)
{
    BOOL brc = FALSE;
    if (LockTimers())
    {
        PXTIMER pTimer;
        #ifdef DEBUG_XTIMERS
        _Pmpf((__FUNCTION__ ": finding timer %d", usTimerID));
        #endif
        if (pTimer = FindTimer(pSet,
                               hwnd,
                               usTimerID))
                // FindTimer checks the params
        {
            RemoveTimer(pSet, pTimer);
            // recalculate
            AdjustPMTimer(pSet);
            brc = TRUE;
        }

        UnlockTimers();
    }

    return brc;
}


