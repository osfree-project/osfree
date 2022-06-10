
/*
 *@@sourcefile sem.h:
 *      header file for sem.c. See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #define INCL_DOSSEMAPHORES
 *@@include #include <os2.h>
 *@@include #include "helpers\semaphores.h"
 */

/*
 *      Copyright (C) 2002 Sander van Leeuwen.
 *      Copyright (C) 2002 Ulrich M”ller.
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

#ifndef SEM_HEADER_INCLUDED
    #define SEM_HEADER_INCLUDED

    /*
     *@@ FASTMTX:
     *
     */

    typedef struct _FASTMTX
    {
        LONG    LockCount;
        LONG    RecursionCount;
        ULONG   OwningThread;
        ULONG   hmtxLock;
        ULONG   Reserved;
    } FASTMTX, *PFASTMTX;

    APIRET semCreate(PFASTMTX pmtx,
                     PCSZ pszSemName);

    APIRET semOpen(PFASTMTX pmtx,
                   PCSZ pszSemName);

    APIRET semClose(PFASTMTX pmtx);

    APIRET semRequest(PFASTMTX pmtx);

    BOOL semAssert(PFASTMTX pmtx);

    BOOL semTry(PFASTMTX pmtx);

    APIRET semRelease(PFASTMTX pmtx);

    LONG APIENTRY lockCompareExchange(PLONG dest, LONG xchg, LONG compare);
    LONG APIENTRY lockDecrement(PLONG);
    LONG APIENTRY lockExchange(PLONG, LONG);
    LONG APIENTRY lockExchangeAdd(PLONG dest, LONG incr);
    LONG APIENTRY lockIncrement(PLONG);

#endif

#if __cplusplus
}
#endif

