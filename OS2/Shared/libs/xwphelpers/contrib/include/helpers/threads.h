
/*
 *@@sourcefile threads.h:
 *      header file for treads.c. See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #define INCL_DOSPROCESS
 *@@include #include <os2.h>
 *@@include #include "helpers\threads.h"
 */

/*
 *      Copyright (C) 1997-2001 Ulrich M”ller.
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

#if __cplusplus
extern "C" {
#endif

#ifndef THREADS_HEADER_INCLUDED
    #define THREADS_HEADER_INCLUDED

    #define THRF_PMMSGQUEUE     0x0001
    #define THRF_WAIT           0x0002
    #define THRF_TRANSIENT      0x0004
    #define THRF_WAIT_EXPLICIT  0x0008          // V0.9.9 (2001-03-14) [umoeller]

    /*
     *@@ THREADINFO:
     *      thread info structure passed to secondary threads.
     *      See thrCreate.
     */

    typedef struct _THREADINFO
    {
        // data maintained by thr* functions
        ULONG   cbStruct;
        void*   pThreadFunc;    // as passed to thrCreate, really a PTHREADFUNC
        volatile unsigned long *ptidRunning;
                                // as passed to thrCreate V0.9.12 (2001-05-20) [umoeller]
        const char *pcszThreadName; // as passed to thrCreate
        ULONG   flFlags;        // as passed to thrCreate
        ULONG   ulData;         // as passed to thrCreate

        TID     tid;
        ULONG   hevRunning;     // this is a HEV really

        // data maintained by thr_fntGeneric
        HAB     hab;            // for PM threads
        HMQ     hmq;            // for PM threads
        HEV     hevExitComplete;    // posted when thread exits V0.9.16 (2001-12-08) [umoeller]

        // data to be maintained by application
        BOOL    fExit;
        ULONG   ulResult;
        ULONG   ulFuncInfo;
        HWND    hwndNotify;
    } THREADINFO, *PTHREADINFO;

    typedef void _Optlink THREADFUNC (PTHREADINFO);
    typedef THREADFUNC *PTHREADFUNC;

    ULONG XWPENTRY thrCreate(PTHREADINFO pti,
                             PTHREADFUNC pfn,
                             volatile unsigned long *ptidRunning,
                             const char *pcszThreadName,
                             ULONG flFlags,
                             ULONG ulData);
    // this function is exported, so add definition here V0.9.13 (2001-06-13) [lafaix]
    typedef BOOL XWPENTRY THRCREATE(PTHREADINFO, PTHREADFUNC, PULONG, const char *, ULONG, ULONG);
    typedef THRCREATE *PTHRCREATE;

    ULONG XWPENTRY thrRunSync(HAB hab,
                              PTHREADFUNC pfn,
                              const char *pcszThreadName,
                              ULONG ulData);

    PTHREADINFO XWPENTRY thrListThreads(PULONG pcThreads);

    BOOL XWPENTRY thrFindThread(PTHREADINFO pti,
                                ULONG tid);

    BOOL XWPENTRY thrClose(PTHREADINFO pti);

    BOOL XWPENTRY thrWait(PTHREADINFO pti);

    BOOL XWPENTRY thrFree(PTHREADINFO pti);

    BOOL XWPENTRY thrKill(PTHREADINFO pti);

    TID XWPENTRY thrQueryID(const THREADINFO* pti);

    ULONG XWPENTRY thrQueryPriority(VOID);

#endif

#if __cplusplus
}
#endif

