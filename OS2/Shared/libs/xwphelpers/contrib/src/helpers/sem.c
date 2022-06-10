
/*
 *@@sourcefile sem.c:
 *      implements fast mutex semaphores.
 *
 *      This is an OS/2 implementation of what Win32 calls
 *      "critical sections". This is highly optimized for
 *      the case where there is only one thread trying to
 *      access the critical section, i.e. it is available most
 *      of the time. Therefore we can use these critical
 *      sections for all our serialization and not lose any
 *      performance when concurrent access is unlikely.
 *
 *      In case there is multiple access, we use the OS/2 kernel
 *      event semaphores.
 *
 *      Function prefix:
 *
 *      --  sem*: semaphore helpers.
 *
 *@@added V0.9.20 (2002-08-04) [umoeller]
 *@@header "helpers\semaphores.h"
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

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS

#define INCL_WINMESSAGEMGR
#include <os2.h>

#include <stdlib.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers/sem.h"

#pragma hdrstop

/*
 *@@category: Helpers\Control program helpers\Semaphores
 *      see sem.c.
 */

// encode PID and TID into one 32bit value
#define MAKE_THREADID(processid, threadid)  ((processid << 16) | threadid)

/*
 *@@ GetTID:
 *
 */

ULONG GetTID(VOID)
{
    PTIB   ptib;
    PPIB   ppib;
    APIRET rc;

    if (!(rc = DosGetInfoBlocks(&ptib, &ppib)))
        return MAKE_THREADID(ppib->pib_ulpid,
                             ptib->tib_ptib2->tib2_ultid);

    return 0;
}

/*
 *@@ GetPID:
 *
 */

ULONG GetPID(VOID)
{
    PTIB   ptib;
    PPIB   ppib;
    APIRET rc;

    if (!(rc = DosGetInfoBlocks(&ptib, &ppib)))
        return ppib->pib_ulpid;

    return 0;
}

/*
 *@@ semCreate:
 *      the equivalent to DosCreateMutexSem.
 */

APIRET semCreate(PFASTMTX pmtx,
                 PCSZ pszSemName)
{
    APIRET rc;

    if (!pmtx)
        return ERROR_INVALID_PARAMETER;

    // initialize lock count with special value -1, meaning noone posesses it
    pmtx->LockCount      = -1;
    pmtx->RecursionCount = 0;
    pmtx->OwningThread   = 0;

    if (rc = DosCreateEventSem((PSZ)pszSemName,
                               &pmtx->hmtxLock,
                               (pszSemName) ? DC_SEM_SHARED : 0,
                               0))
        pmtx->hmtxLock = 0;

    pmtx->Reserved = GetPID();

    return rc;
}

/*
 *@@ semOpen:
 *      the equivalent to DosOpenMutexSem.
 */

APIRET semOpen(PFASTMTX pmtx,
               PCSZ pszSemName)
{
    HMTX   hmtxLock = 0;
    APIRET rc;

    if (!pszSemName)
        return ERROR_INVALID_PARAMETER;

    return DosOpenEventSem((PSZ)pszSemName, &hmtxLock);
}

/*
 *@@ semClose:
 *      the equivalent to DosCloseMutexSem.
 */

APIRET semClose(PFASTMTX pmtx)
{
    APIRET rc;

    if (!pmtx)
        return ERROR_INVALID_PARAMETER;

    if (!pmtx->hmtxLock)
        return ERROR_INVALID_HANDLE;

    if (pmtx->RecursionCount)  /* Should not happen */
        return ERROR_SEM_BUSY;

    pmtx->LockCount      = -1;
    pmtx->RecursionCount = 0;
    pmtx->OwningThread   = 0;
    rc = DosCloseEventSem(pmtx->hmtxLock);
    pmtx->hmtxLock       = 0;
    pmtx->Reserved       = (ULONG)-1;

    return rc;
}

/*
 *@@ semRequest:
 *      the equivalent to DosRequestMutexSem.
 */

APIRET semRequest(PFASTMTX pmtx)
{
    ULONG   threadid = GetTID();

    if (!pmtx)
        return ERROR_INVALID_PARAMETER;

    // create pmtx sect just in time...
    if (!pmtx->hmtxLock)
        semCreate(pmtx, NULL);

    // do an atomic increase of the lockcounter and see if it is > 0
    // (i.e. it is already posessed)
    if (lockIncrement(&pmtx->LockCount))
    {
        // semaphore was already requested:

testenter:
        // if the same thread is requesting it again, memorize it
        if (pmtx->OwningThread == threadid)
        {
            pmtx->RecursionCount++;
            return NO_ERROR;
        }

        // current owner is different thread:

        // do an atomic operation where we compare the owning thread id with 0
        // and if this is true, exchange it with the id of the current thread
        if (lockCompareExchange((PLONG)&pmtx->OwningThread, threadid, 0))
        {
            // the compare did not return equal, i.e. the pmtx sect is in use

            ULONG   cPosts;
            APIRET  rc;

            /* Now wait for it */
            if (rc = DosWaitEventSem(pmtx->hmtxLock, SEM_INDEFINITE_WAIT))
                return rc;

            DosResetEventSem(pmtx->hmtxLock, &cPosts);

            // multiple waiters could be running now. Repeat the logic so that
            // only one actually can get the critical section
            goto testenter;
        }
    }

    pmtx->OwningThread   = GetTID();
    pmtx->RecursionCount = 1;

    return NO_ERROR;
}

/*
 *@@ semAssert:
 *      returns TRUE if the current thread currently owns
 *      the mutex.
 *
 */

BOOL semAssert(PFASTMTX pmtx)
{
    return (    (pmtx)
             && (pmtx->OwningThread)
             && (pmtx->OwningThread == GetTID())
           );
}

/*
 *@@ semTry:
 *
 */

BOOL semTry(PFASTMTX pmtx)
{
    if (lockIncrement(&pmtx->LockCount))
    {
        if (pmtx->OwningThread == GetTID())
        {
            pmtx->RecursionCount++;
            return TRUE;
        }

        lockDecrement(&pmtx->LockCount);

        return FALSE;
    }

    pmtx->OwningThread   = GetTID();
    pmtx->RecursionCount = 1;

    return TRUE;
}

/*
 *@@ semRelease:
 *      the equivalent of DosReleaseMutexSem.
 */

APIRET semRelease(PFASTMTX pmtx)
{
    if (!pmtx)
        return ERROR_INVALID_PARAMETER;

    if (pmtx->OwningThread != GetTID())
        return ERROR_NOT_OWNER;

    if (--pmtx->RecursionCount)
    {
        lockDecrement(&pmtx->LockCount );
        return NO_ERROR;
    }

    pmtx->OwningThread = 0;

    if (lockDecrement(&pmtx->LockCount) >= 0)
        // someone is waiting
        DosPostEventSem(pmtx->hmtxLock);

    return NO_ERROR;
}

