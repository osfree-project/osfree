
/*
 *@@sourcefile pmsems.h:
 *      header file declaring the format, the indices, and the names
 *      of the global fast mutex semaphores used by PM and GRE,
 *      according to the OS/2 debugging handbook.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 */

/*      Copyright (C) 2002 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *@@include #include <os2.h>
 *@@include #include "helpers\pmsems.h"
 */

#if __cplusplus
extern "C" {
#endif

#ifndef PMSEMS_HEADER_INCLUDED
    #define PMSEMS_HEADER_INCLUDED

    #pragma pack(1)

    /*
     *@@ PMSEM:
     *      PM semaphore definition, according to
     *      the OS/2 debugging handbook.
     *
     *@@added V1.0.0 (2002-08-12) [umoeller]
     */

    typedef struct _PMSEM
    {
        CHAR        szMagic[7];
        BYTE        bSemaphore;         // 386 semaphore byte
        ULONG       ulOwnerPidTid;
        ULONG       cNesting;           // owner nested use count
        ULONG       cWaiters;           // number of waiters
        ULONG       ulDebug1;           // number of times sem used (debug only)
        HEV         hev;                // OS/2 event semaphore
        ULONG       ulDebug2;           // address of caller (debug only)
    } PMSEM, *PPMSEM;

    #pragma pack()

    #define PMSEM_ATOM          0
    #define PMSEM_USER          1
    #define PMSEM_VISLOCK       2
    #define PMSEM_DEBUG         3
    #define PMSEM_HOOK          4
    #define PMSEM_HEAP          5
    #define PMSEM_DLL           6
    #define PMSEM_THUNK         7
    #define PMSEM_XLCE          8
    #define PMSEM_UPDATE        9
    #define PMSEM_CLIP          10
    #define PMSEM_INPUT         11
    #define PMSEM_DESKTOP       12
    #define PMSEM_HANDLE        13
    #define PMSEM_ALARM         14
    #define PMSEM_STRRES        15
    #define PMSEM_TIMER         16
    #define PMSEM_CONTROLS      17
    #define GRESEM_GreInit      18
    #define GRESEM_AutoHeap     19
    #define GRESEM_PDEV         20
    #define GRESEM_LDEV         21
    #define GRESEM_CodePage     22
    #define GRESEM_HFont        23
    #define GRESEM_FontCntxt    24
    #define GRESEM_FntDrvr      25
    #define GRESEM_ShMalloc     26
    #define GRESEM_GlobalData   27
    #define GRESEM_DbcsEnv      28
    #define GRESEM_SrvLock      29
    #define GRESEM_SelLock      30
    #define GRESEM_ProcLock     31
    #define GRESEM_DriverSem    32
    #define GRESEM_semIfiCache  33
    #define GRESEM_semFontTable 34

    #define LASTSEM             34

    #ifndef __DEFINE_PMSEMS
        static const PCSZ G_papcszSems[] =
    #else
        static const PCSZ G_papcszSems[] =
            {
                "PMSEM_ATOM", // 0
                "PMSEM_USER", // 1
                "PMSEM_VISLOCK", // 2
                "PMSEM_DEBUG", // 3
                "PMSEM_HOOK", // 4
                "PMSEM_HEAP", // 5
                "PMSEM_DLL", // 6
                "PMSEM_THUNK", // 7
                "PMSEM_XLCE", // 8
                "PMSEM_UPDATE", // 9
                "PMSEM_CLIP", // 10
                "PMSEM_INPUT", // 11
                "PMSEM_DESKTOP", // 12
                "PMSEM_HANDLE", // 13
                "PMSEM_ALARM", // 14
                "PMSEM_STRRES", // 15
                "PMSEM_TIMER", // 16
                "PMSEM_CONTROLS", // 17
                "GRESEM_GreInit", // 18
                "GRESEM_AutoHeap", // 19
                "GRESEM_PDEV", // 20
                "GRESEM_LDEV", // 21
                "GRESEM_CodePage", // 22
                "GRESEM_HFont", // 23
                "GRESEM_FontCntxt", // 24
                "GRESEM_FntDrvr", // 25
                "GRESEM_ShMalloc", // 26
                "GRESEM_GlobalData", // 27
                "GRESEM_DbcsEnv", // 28
                "GRESEM_SrvLock", // 29
                "GRESEM_SelLock", // 30
                "GRESEM_ProcLock", // 31
                "GRESEM_DriverSem", // 32
                "GRESEM_semIfiCache", // 33
                "GRESEM_semFontTable", // 34
            };

    #endif      // #ifndef __DEFINE_PMSEMS

#endif

#if __cplusplus
}
#endif

