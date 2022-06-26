
/*
 *@@sourcefile watchdog.c:
 *
 *@@added V1.0.0 (2002-08-12) [umoeller]
 *@@header "hook\xwpdaemn.h"
 */

/*
 *      Copyright (C) 2002-2003 Ulrich M”ller.
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

#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINSYS
#include <os2.h>

#include <stdio.h>
#include <time.h>
#include <setjmp.h>

#define DONT_REPLACE_FOR_DBCS
#define DONT_REPLACE_MALLOC         // in case mem debug is enabled
#include "setup.h"                      // code generation and debugging options

#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#define __DEFINE_PMSEMS
#include "helpers\pmsems.h"
#include "helpers\procstat.h"
#include "helpers\standards.h"
#include "helpers\threads.h"

#include "xwpapi.h"                     // public XWorkplace definitions

#include "filesys\disk.h"               // XFldDisk implementation

#include "hook\xwphook.h"               // hook and daemon definitions
#include "hook\xwpdaemn.h"              // XPager and daemon declarations

#include "bldlevel.h"

#pragma hdrstop

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

// threadinfos for the two extra threads
THREADINFO  G_tiWatchdog,
            G_tiTeaser;

#define OP_IDLE                     0
#define OP_BEGINENUMWINDOWS         1
#define OP_GETNEXTWINDOW            2
#define OP_ISWINDOWVISIBLE          3
#define OP_QUERYWINDOWPROCESS       4
#define OP_SENDMSG                  5
#define OP_ENDENUMWINDOWS           6

HMTX        G_hmtxWatchdog = NULLHANDLE;

// the following are protected by G_hmtxWatchdog:

ULONG       G_ulOperation = OP_IDLE;    // current operation being performed

ULONG       G_ulOperationTime = 0;      // system uptime when operation was performed

ULONG       G_pidBeingSent = 0,         // PID of window that msg is being sent to
            G_tidBeingSent = 0;         // TID of window that msg is being sent to

// end G_hmtxWatchdog

HMTX        G_hmtxHungs = NULLHANDLE;

// the following are protected by G_hmtxHungs:

/* ******************************************************************
 *
 *   Teaser
 *
 ********************************************************************/

#ifndef WM_QUERYCTLTYPE
#define WM_QUERYCTLTYPE            0x0130
#endif

/*
 *@@ LockWatchdog:
 *
 */

BOOL LockWatchdog(VOID)
{
    if (G_hmtxWatchdog)
        return !DosRequestMutexSem(G_hmtxWatchdog, SEM_INDEFINITE_WAIT);

    // first call:
    return !DosCreateMutexSem(NULL,
                              &G_hmtxWatchdog,
                              0,
                              TRUE);      // request!
}

/*
 *@@ UnlockWatchdog:
 *
 */

VOID UnlockWatchdog(VOID)
{
    DosReleaseMutexSem(G_hmtxWatchdog);
}

/*
 *@@ SetOperation:
 *      atomically sets G_ulOperation to op
 *      and G_ulOperationTime to the current
 *      system uptime.
 *
 */

void SetOperation(ULONG op,
                  PBOOL pfLocked)
{
    if (*pfLocked = LockWatchdog())
    {
        DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT,
                        &G_ulOperationTime,
                        sizeof(G_ulOperationTime));
        G_ulOperation = op;

        UnlockWatchdog();
        *pfLocked = FALSE;
    }
}

/*
 *@@ fntTeaser:
 *      teaser thread started by fntWatchdog.
 *
 *      This does nothing except enumerating all desktop
 *      windows and sending a message to each of them
 *      to be able to detect whether the system is
 *      responsive.
 *
 *      Before each PM call, we set a global flag and the
 *      current system uptime to allow fntWatchdog to
 *      detect how much time we have taken for a call.
 *      This way the watchdog can detect whether we're
 *      stuck in a PM call.
 *
 *      This loops forever.
 *
 */

void _Optlink fntTeaser(PTHREADINFO ptiMyself)
{
    BOOL    fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        // run forever unless exit
        while (!ptiMyself->fExit)
        {
            HENUM   henum;
            HWND    hwnd;

            // call WinBeginEnumWindows
            SetOperation(OP_BEGINENUMWINDOWS, &fLocked);
            if (henum = WinBeginEnumWindows(HWND_DESKTOP))
            {
                while (TRUE)
                {
                    ULONG   pid, tid;

                    // call WinGetNextWindow
                    SetOperation(OP_GETNEXTWINDOW, &fLocked);
                    if (!(hwnd = WinGetNextWindow(henum)))
                        // last window of this enumeration:
                        // stop
                        break;

                    // check for message blocks only if the window
                    // is visible; many apps block their input
                    // queue before showing up (mozilla, pmmail),
                    // and we shouldn't report them

                    // WinIsWindowVisible returns TRUE
                    // only if the window has WS_VISIBLE set and
                    // all of its parents do also...

                    SetOperation(OP_ISWINDOWVISIBLE, &fLocked);
                    if (WinIsWindowVisible(hwnd))
                    {
                        // get the window process
                        SetOperation(OP_QUERYWINDOWPROCESS, &fLocked);
                        if (!WinQueryWindowProcess(hwnd, &pid, &tid))
                            // window has died?
                            // start over
                            break;

                        if (fLocked = LockWatchdog())
                        {
                            G_pidBeingSent = pid;
                            G_tidBeingSent = tid;

                            DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT,
                                            &G_ulOperationTime,
                                            sizeof(G_ulOperationTime));
                            G_ulOperation = OP_SENDMSG;

                            UnlockWatchdog();
                            fLocked = FALSE;

                            // call WinSendMsg to see if the target
                            // desktop window responds; we use
                            // WM_QUERYCTLTYPE just because it's
                            // inexpensive processing in any window proc
                            WinSendMsg(hwnd,
                                       WM_QUERYCTLTYPE,
                                       0,
                                       0);
                        }
                    }
                } // while (TRUE)
            }

            // call WinEndEnumWindows
            SetOperation(OP_ENDENUMWINDOWS, &fLocked);
            WinEndEnumWindows(henum);

            SetOperation(OP_IDLE, &fLocked);

            DosSleep(300);
        }
    }
    CATCH(excpt1)
    {
    } END_CATCH();

    if (fLocked)
        UnlockWatchdog();
}

/* ******************************************************************
 *
 *   Watchdog
 *
 ********************************************************************/

/*
 *@@ LockHungs:
 *
 */

BOOL LockHungs(VOID)
{
    if (G_hmtxHungs)
        return !DosRequestMutexSem(G_hmtxHungs, SEM_INDEFINITE_WAIT);

    // first call:
    return !DosCreateMutexSem(NULL,
                              &G_hmtxHungs,
                              0,
                              TRUE);      // request!
}

/*
 *@@ UnlockHungs:
 *
 */

VOID UnlockHungs(VOID)
{
    DosReleaseMutexSem(G_hmtxHungs);
}

typedef struct _HUNG
{
    USHORT  pid;
    USHORT  tid;
    ULONG   op;
    ULONG   sem;
} HUNG, *PHUNG;

#define MAXHUNG         40

HUNG        G_aHungs[MAXHUNG];
ULONG       G_cHungs = 0;

/*
 *@@ AppendHung:
 *
 *      Caller must hold the hungs sem.
 *
 */

VOID AppendHung(USHORT pid,
                USHORT tid,
                ULONG op,
                ULONG sem)
{
    G_aHungs[G_cHungs].pid = pid;
    G_aHungs[G_cHungs].tid = tid;
    G_aHungs[G_cHungs].op = op;
    G_aHungs[G_cHungs++].sem = sem;
}

/*
 *@@ GetOpName:
 *
 */

PCSZ GetOpName(ULONG opHung)
{
    switch (opHung)
    {
        #define SETCASE(o) case o: return # o

        SETCASE(OP_IDLE);
        SETCASE(OP_BEGINENUMWINDOWS);
        SETCASE(OP_GETNEXTWINDOW);
        SETCASE(OP_ISWINDOWVISIBLE);
        SETCASE(OP_QUERYWINDOWPROCESS);
        SETCASE(OP_SENDMSG);
        SETCASE(OP_ENDENUMWINDOWS);
    }

    return "unknown";
}

#define TIMEOUT     (1 * 1000)

/*
 *@@ fntWatchdog:
 *
 */

void _Optlink fntWatchdog(PTHREADINFO ptiMyself)
{
    BOOL    semWatchdog = FALSE,
            semHungs = FALSE;
    CHAR    szTemp[500];
    HPS     hpsScreen = NULLHANDLE;

    TRY_LOUD(excpt1)
    {
        APIRET  arc;
        PPMSEM  paPMSems;

        LONG    cxScreen = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);

        thrCreate(&G_tiTeaser,
                  fntTeaser,
                  NULL,
                  "Teaser",
                  THRF_PMMSGQUEUE | THRF_WAIT,
                  0);

        // the array of PM semaphores is at PMMERGE.6203
        if (arc = doshQueryProcAddr("PMMERGE",
                                    6203,
                                    (PFN*)&paPMSems))
        {
            // and exit
            ptiMyself->fExit = TRUE;
        }
        else
        {
            CHAR szTest[9];
            szTest[8] = '\0';
            memcpy(szTest, paPMSems, 7);
        }

        // run forever unless exit
        while (!ptiMyself->fExit)
        {
            ULONG   opHung = OP_IDLE,
                    pidBeingSent = 0,
                    tidBeingSent = 0;
            ULONG   ulTimeNow;
            PCSZ    pcszOp = NULL;

            PSZ     apsz[MAXHUNG];
            ULONG   cPaint = 0;

            memset(apsz, 0, sizeof(apsz));

            if (semWatchdog = LockWatchdog())
            {
                DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT,
                                &ulTimeNow,
                                sizeof(ulTimeNow));

                if (    (G_ulOperation != OP_IDLE)
                     && (G_ulOperationTime < ulTimeNow - TIMEOUT)
                   )
                {
                    opHung = G_ulOperation;
                    pidBeingSent = G_pidBeingSent;
                    tidBeingSent = G_tidBeingSent;
                }

                UnlockWatchdog();
                semWatchdog = FALSE;
            }

            // lock the global list of hung processes
            // while we're checking the results
            if (semHungs = LockHungs())
            {
                // reset global count
                G_cHungs = 0;

                if (opHung != OP_IDLE)
                {
                    ULONG   ulSemThis;

                    CHAR    szLogFile[CCHMAXPATH];
                    FILE    *LogFile;
                    sprintf(szLogFile,
                            "%c:\\watchdog.log",
                            doshQueryBootDrive());

                    LogFile = fopen(szLogFile, "a");

                    DosBeep(100, 100);

                    // now differentiate:

                    // 1) If the teaser thread was stuck in WinSendMsg,
                    //    that's the simple case where the target window
                    //    is not currently processing its message queue.
                    //    Typical candidates are VIEW.EXE searching the
                    //    LIBPATH and Netscape formatting complex tables.
                    //
                    //    It is most probable that the hung process is
                    //    thus the process of the target window of
                    //    the WinSendMsg call, which was stored in
                    //    G_pidBeingSent.
                    //
                    //    From my testing, in the simple case, all the
                    //    PMSEMS are unowned, unless other things have
                    //    gone wrong too.
                    //
                    // 2) If the teaser thread got stuck in another PM
                    //    function however, it is quite probable that
                    //    some other PM thread is owning a global PMSEM.

                    // So to find candidates for "hung processes", we
                    // use the current target of WinSendMsg, plus all
                    // current owners of the global PMSEMs we find.

                    if (    (opHung == OP_SENDMSG)
                         && (pidBeingSent)
                       )
                    {
                        AppendHung(pidBeingSent,
                                   tidBeingSent,
                                   opHung,      // OP_SENDMSG
                                   -1);         // no sem yet
                    }

                    // run through the PMSEMs to see if
                    // they're owned
                    for (ulSemThis = 0;
                         ulSemThis <= LASTSEM;
                         ++ulSemThis)
                    {
                        // make sure this is really a PMSEM
                        if (    (!memcmp(paPMSems->szMagic, "PMSEM", 5))
                             || (!memcmp(paPMSems->szMagic, "GRESEM", 6))
                           )
                        {
                            ULONG pidtid;

                            if (pidtid = paPMSems[ulSemThis].ulOwnerPidTid)
                            {
                                // this sem is OWNED:
                                // consider the owner a hung app!
                                AppendHung(LOUSHORT(pidtid),
                                           HIUSHORT(pidtid),
                                           opHung,      // OP_SENDMSG
                                           ulSemThis);         // semid
                            }

                            if (LogFile)
                                fprintf(LogFile,
                                        "pmsem %d (%s) has owner %lX, %d waiters\n",
                                        ulSemThis,
                                        G_papcszSems[ulSemThis],
                                        pidtid,
                                        paPMSems[ulSemThis].cWaiters);

                        }
                        else
                        {
                            // magic is invalid:
                            if (LogFile)
                                fprintf(LogFile,
                                        "magic for sem %d (%s) is invalid, stopping watchdog\n",
                                        ulSemThis,
                                        G_papcszSems[ulSemThis]);
                            ptiMyself->fExit = TRUE;
                            break;
                        }
                    }

                    if (G_cHungs)
                    {
                        PQTOPLEVEL32    t;
                        PQPROCESS32     p;
                        PQMODULE32      m;

                        if (LogFile)
                                fprintf(LogFile,
                                        "!!! got %d hung processes\n",
                                        G_cHungs);

                        // now describe the hung processes
                        if (t = prc32GetInfo(NULL))
                        {
                            ULONG   ulHung;
                            PCSZ    pcszSem;
                            ULONG   sem;

                            for (ulHung = 0;
                                 ulHung < G_cHungs;
                                 ++ulHung)
                            {
                                ULONG   pid = G_aHungs[ulHung].pid,
                                        tid = G_aHungs[ulHung].tid,
                                        ulLength = 0;

                                if (p = prc32FindProcessFromPID(t, pid))
                                {
                                    if (m = prc32FindModule(t, p->usHModule))
                                    {
                                        ulLength = sprintf(szTemp,
                                                "PID 0x%lX (%d) (%s)",
                                                pid,
                                                pid,
                                                m->pcName      // module name
                                              );
                                    }
                                    else
                                        ulLength = sprintf(szTemp,
                                                "PID 0x%lX (%d) (unknown hmod 0x%lX)",
                                                pid,
                                                pid,
                                                p->usHModule);
                                }
                                else
                                    ulLength = sprintf(szTemp,
                                            "unknown PID 0x%lX (%d)",
                                            pid,
                                            pid);

                                sem = G_aHungs[ulHung].sem;

                                if (sem < ARRAYITEMCOUNT(G_papcszSems))
                                    pcszSem = G_papcszSems[sem];
                                else
                                    pcszSem = "no sem";

                                sprintf(szTemp + ulLength,
                                        " , TID %d, op %s, %s",
                                        tid,
                                        GetOpName(G_aHungs[ulHung].op),
                                        pcszSem);


                                if (LogFile)
                                    fprintf(LogFile,
                                            "hung process [%d]: %s\n",
                                            ulHung,
                                            szTemp);

                                apsz[ulHung] = strdup(szTemp);
                            }

                            prc32FreeInfo(t);
                        }
                    } // end if (cHungProcesses)

                    if (LogFile)
                        fclose(LogFile);

                } // end else if (opHung == OP_IDLE)

                // copy var locally for painting
                cPaint = G_cHungs;

                UnlockHungs();
                semHungs = FALSE;
            } // if (semHungs = LockHungs())

            if (cPaint)
            {
                if (hpsScreen = WinGetScreenPS(HWND_DESKTOP))
                {
                    // found any hung processes:
                    #define CX_BORDER   10
                    #define CY_BOTTOM   30
                    #define SPACING     5
                    #define LINEHEIGHT  20

                    RECTL   rcl,
                            rcl2;
                    POINTL  ptl;
                    ULONG   ulPaint;

                    rcl.xLeft = CX_BORDER;
                    rcl.yBottom = CY_BOTTOM;
                    rcl.xRight = cxScreen - 2 * CX_BORDER;
                    rcl.yTop =   rcl.yBottom
                               + 2 * SPACING
                               + cPaint * LINEHEIGHT;

                    WinFillRect(hpsScreen,
                                &rcl,
                                CLR_WHITE);

                    rcl2.xLeft = CX_BORDER + SPACING;
                    rcl2.yBottom = rcl.yBottom;
                    rcl2.xRight = rcl.xRight;
                    rcl2.yTop = rcl.yTop - SPACING; //  - LINEHEIGHT;

                    for (ulPaint = 0;
                         ulPaint < cPaint;
                         ++ulPaint)
                    {
                        PSZ psz;
                        if (psz = apsz[ulPaint])
                        {
                            WinDrawText(hpsScreen,
                                        -1,
                                        psz,
                                        &rcl2,
                                        CLR_BLACK,
                                        CLR_WHITE,
                                        DT_TOP | DT_LEFT);

                            /*
                            GpiCharStringAt(hpsScreen,
                                            &ptl,
                                            strlen(psz),
                                            psz);
                            */

                            free(psz);

                            rcl2.yTop -= LINEHEIGHT;
                        }
                    }

                    WinReleasePS(hpsScreen);
                    hpsScreen = NULLHANDLE;

                }
            } // if (cPaint)

            DosSleep(1000);
        } // while (!ptiMyself->fExit)
    }
    CATCH(excpt1)
    {
    } END_CATCH();

    if (semWatchdog)
        UnlockWatchdog();

    if (semHungs)
        UnlockHungs();

    if (hpsScreen)
        WinReleasePS(hpsScreen);
}

/*
 *@@ dmnStartWatchdog:
 *
 */

VOID dmnStartWatchdog(VOID)
{
    thrCreate(&G_tiWatchdog,
              fntWatchdog,
              NULL,
              "Watchdog",
              THRF_WAIT,
              0);
}
