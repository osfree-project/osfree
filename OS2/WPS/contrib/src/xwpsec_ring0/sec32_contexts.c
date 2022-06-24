
/*
 *@@sourcefile sec32_contexts.c:
 *      security contexts implementation.
 *
 *      See strat_init_base.c for an introduction to the driver
 *      structure in general.
 *
 *      This file has the driver "engine", so-to-say, with
 *      functions that get called from most other parts of the
 *      driver.
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
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

// #ifdef __IBMC__
// #pragma strings(readonly)
// #endif

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>

#include <builtin.h>
#include <string.h>

#include "helpers\tree.h"
#include "helpers\xwpsecty.h"

#include "xwpsec32.sys\types.h"
#include "xwpsec32.sys\StackToFlat.h"
#include "xwpsec32.sys\DevHlp32.h"
#include "xwpsec32.sys\reqpkt32.h"

#include "xwpsec32.sys\xwpsec_callbacks.h"

#include "security\ring0api.h"

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

extern struct InfoSegGDT
                *G_pGDT = 0;            // OS/2 global infoseg
extern struct InfoSegLDT
                *G_pLDT = 0;            // OS/2 local  infoseg

extern XWPSECSTATUS
                G_R0Status;             // in strat_ioctl.c

extern HVDHSEM  G_hmtx;                 // in strat_ioctl.c

extern PLOGBUF  G_pLogFirst = NULL,     // ptr to first log buffer in linklist
                G_pLogLast = NULL;      // ptr to last log buffer in linklist

extern ULONG    G_cLogBufs = 0;         // no. of log buffers currently allocated

extern ULONG    G_idLogBufNext = 0,     // global log buf ID (counter)
                G_idEventNext = 0;      // global event ID (counter)

extern BYTE     G_bLog = LOG_INACTIVE;

extern ULONG    G_idLogBlock = 0;       // if != 0, the blockid that the logging thread
                                        // is currently blocked on;
                                        // if 0, the logging thread is either busy
                                        // in ring-3 or ready to run after a ProcRun()

TREE            *G_ContextsTree;

PRING0BUF       G_pRing0Buf = NULL;     //  system ACL table

CHAR            G_szResource[CCHMAXPATH];   // scratch buffer for authentication

/* ******************************************************************
 *
 *   Initialization
 *
 ********************************************************************/

/*
 *@@ ctxtInit:
 *      initializes the security contexts engine. Must be called
 *      exactly once at driver init.
 */

VOID ctxtInit(VOID)
{
    treeInit(&G_ContextsTree,
             &G_R0Status.cContexts);
}

/* ******************************************************************
 *
 *   Audit logging
 *
 ********************************************************************/

/*
 *      Logging works as follows:
 *
 *      1)  After the driver has been opened successfully
 *          by the shell, it starts a logging thread,
 *          which calls DosDevIOCtl into the driver with
 *          an XWPSECIO_GETLOGBUF function code. This
 *          ends up in ctxtFillLogBuf() where the thread
 *          is blocked until logging data becomes available.
 *
 *      2)  Whenever ctxtLogEvent is called (e.g. from the
 *          OPEN_PRE callout), it receives the size of the
 *          data that is to go into the log buffers.
 *
 *          If no log buffer has yet been allocated, or if
 *          there's one, but there's not enough room left
 *          for the new data, ctxtLogEvent allocates one.
 *          The driver maintains a linked list of logging
 *          buffers and always appends to the last one
 *          (while the ring-3 thread always takes off the
 *          first buffer from the list).
 *
 *          Otherwise we append to the current logging buffer.
 *
 *          In any case, we then append a new EVENTLOGENTRY
 *          to the LOGBUF (either the old one or the new one).
 *
 *      3)  We then check if the ring-3 thread is currently
 *          blocked. If so, we unblock it; otherwise it's
 *          still busy processing previous data and we do
 *          nothing.
 *
 *      Whenever the ring-3 thread gets unblocked,
 *      ctxtFillLogBuf() takes the first buffer off the list
 *      and copies it into ring-3 memory. The buffer from
 *      the list is then freed again.
 *
 *      Sample event flow:
 *
 +          ring 3 logging thread     ³ ring 0 (task time          ³ ring 0 (other task)
 +                                    ³ of logging thread,         ³
 +                                    ³ GETLOGBUF implement'n)     ³
 +                                    ³                            ³
 +      ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 +                                    ³                            ³
 +          XWPShell starts logging   ³                            ³
 +          thread,                   ³                            ³
 +                                    ³                            ³
 +          calls GETLOGBUF ioctl     ³                            ³
 +                                    ³                            ³
 +                                    ³ ctxtFillLogBuf() sees      ³
 +                                    ³ we have no LOGBUF data,    ³
 +                                    ³ so ProcBlock() logging     ³
 +                                    ³ thread                     ³
 +                                    ³                            ³
 +      (*) (blocked)                 ³                            ³ OPEN_PRE calls ctxtLogEvent
 +          ³                         ³                            ³
 +          ³                         ³                            ³ 1) append LOGBUF data
 +          ³                         ³                            ³
 +          ³                         ³                            ³ 2) see that log thread is blocked:
 +          ³                         ³                            ³    ProcRun() logging thread,
 +          ÀÄ (unblocked)            ³                            ³    return
 +                                    ³                            ³
 +                                    ³ ProcBlock() returns in     ³
 +                                    ³ ctxtFillLogBuf():          ³
 +                                    ³                            ³
 +                                    ³ memcpy to ring-3 LOGBUF    ³
 +                                    ³ return from ioctl          ³
 +                                    ³                            ³
 +          process logbufs           ³                            ³
 +          ³                         ³                            ³
 +          ³                         ³                            ³
 +          ³                         ³                            ³ OPEN_PRE calls ctxtLogEvent
 +          ³                         ³                            ³
 +          ³                         ³                            ³ 1) append LOGBUF data
 +          ³                         ³                            ³
 +          ³                         ³                            ³ 2) see that log thread is busy:
 +          ³                         ³                            ³    do nothing
 +          ³                         ³                            ³
 +          ³                         ³                            ³
 +          ³                         ³                            ³ OPEN_PRE calls ctxtLogEvent
 +          ³                         ³                            ³
 +          ³                         ³                            ³ 1) append LOGBUF data
 +          ³                         ³                            ³
 +          ³                         ³                            ³ 2) see that log thread is busy:
 +          ³                         ³                            ³    do nothing
 +          ³                         ³                            ³
 +          ³                         ³                            ³
 +          À calls GETLOGBUF ioctl   ³                            ³
 +                                    ³                            ³
 +                                    ³ ctxtFillLogBuf()           ³
 +                                    ³ sees that we have data:    ³
 +                                    ³ do not block               ³
 *                                    ³                            ³
 +                                    ³ memcpy to ring-3 LOGBUF    ³
 +                                    ³ return from ioctl          ³
 +                                    ³                            ³
 +          process logbufs           ³                            ³ (no events this time)
 +          ³                         ³                            ³
 +          ³                         ³                            ³
 +          À calls GETLOGBUF ioctl   ³                            ³
 +                                    ³                            ³
 +                                    ³ ctxtFillLogBuf() sees      ³
 +                                    ³ we have no LOGBUF data,    ³
 +                                    ³ so ProcBlock() logging     ³
 +                                    ³ thread                     ³
 +                                    ³                            ³
 +                                    ³ go back to (*) above       ³
 +                                    ³                            ³
 +      ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 *
 */

/*
 *@@ AllocLogBuffer:
 *      allocates and initializes a new LOGBUF
 *      as fixed kernel memory.
 *
 *      Preconditions: Caller must hold the big lock
 *      cos VMAlloc may block!
 *
 *      Context: Possibly any ring-3 thread on the system.
 */

PLOGBUF AllocLogBuffer(VOID)
{
    PLOGBUF pBuf;

    if (pBuf = utilAllocFixed(LOGBUFSIZE))    // 64K
    {
        pBuf->cbUsed = sizeof(LOGBUF);
        pBuf->pNext = NULL;
        pBuf->idLogBuf = G_idLogBufNext++;
        pBuf->cLogEntries = 0;

        // update global ring-0 status
        ++(G_R0Status.cLogBufs);

        // statistics: keep track of max. buffers ever allocated
        if (G_R0Status.cLogBufs > G_R0Status.cMaxLogBufs)
            G_R0Status.cMaxLogBufs = G_R0Status.cLogBufs;
    }

    return pBuf;
}

/*
 *@@ ctxtLogEvent:
 *      appends a new EVENTLOGENTRY to the global logging
 *      data and returns a pointer to where event-specific
 *      data can be copied.
 *
 *      Runs at task time of any thread in the system
 *      that calls a system API, including XWPShell
 *      itself. For example, we end up here whenever
 *      someone calls DosOpen through the OPEN_PRE
 *      callout.
 *
 *      Each log entry consists of a fixed-size
 *      EVENTLOGENTRY struct with information like
 *      date, time, pid, and tid of the event and
 *      the event code. This data is filled in by
 *      this function.
 *
 *      This function returns a pointer to the first
 *      byte after that structure where the caller must
 *      copy the event-specific data to. This is to avoid
 *      multiple memcpy's so the logger can memcpy directly
 *      into the newly allocated buffer. However, the cbData
 *      passed in to this function needs to be the size of
 *      the entire _variable_ data so we can reserve enough
 *      memory already in here.
 *
 *      Preconditions:
 *
 *      --  Call this only if (G_bLog == LOG_ACTIVE) (that is,
 *          XWPShell is actually picking up logging data) or
 *          we'll leak memory.
 *
 *      --  DevHlp32_GetInfoSegs _must_ have been called
 *          beforehand so that the global "local infoseg"
 *          ptr is valid.
 *
 *      --  This possibly allocates a new logging buffer
 *          via VMAlloc, so there is a slim chance that this
 *          may block if OS/2 needs to work on the swapper.
 *          As a result, the caller must be reentrant and
 *          not rely on static data before and after this
 *          call.
 *
 *      Context: Possibly any ring-3 thread on the system.
 */

PVOID ctxtLogEvent(PXWPSECURITYCONTEXT pContext,        // in: current security context (may be 0, just for logging)
                   ULONG ulEventCode,      // in: EVENT_* code
                   ULONG cbData)           // in: size of buffer corresponding to that code
{
    PVOID   pvReturn = NULL;
    // determine size we need:
    ULONG   cbLogEntry =   sizeof(EVENTLOGENTRY)    // fixed-size struct first
                         + cbData;                  // event-specific data next

    // request logging mutex
    if (!VDHRequestMutexSem(G_hmtx, -1))
        ctxtStopLogging();
    else
    {
        // do we have a log buffer allocated currently?
        if (!G_pLogLast)
        {
            // no: that's easy, allocate one log buffer and
            // set it as the first and last; we'll use that then
            G_pLogFirst
            = G_pLogLast
            = AllocLogBuffer();
        }
        else
        {
            // we do have a log buffer currently: ensure
            // we have enough room left in this one
            if (G_pLogLast->cbUsed + cbLogEntry >= LOGBUFSIZE)      // 64K
            {
                // no: allocate a new one
                G_pLogLast->pNext = AllocLogBuffer();
                // and set this as the last buffer
                G_pLogLast = G_pLogLast->pNext;

                // G_pLogFirst remains the same
            }
        }

        if (!G_pLogLast)
            // error allocating memory:
            // stop logging globally!
            ctxtStopLogging();
        else
        {
            // determine address of new target EVENTLOGENTRY in the LOGBUF
            PEVENTLOGENTRY pEntry = (PEVENTLOGENTRY)((PBYTE)G_pLogLast + G_pLogLast->cbUsed);

            // 1) fill fixed EVENTLOGENTRY struct

            pEntry->cbStruct = cbLogEntry;
            pEntry->ulEventCode = ulEventCode;
            pEntry->idEvent = G_idEventNext++;      // global counter
            pEntry->idContext = (ULONG)pContext;

            // copy the first 16 bytes from local infoseg
            // into log entry (these match our CONTEXTINFO
            // declaration in ring0api.h)
            memcpy(&pEntry->ctxt,
                   G_pLDT,
                   sizeof(CONTEXTINFO));

            // copy the first 20 bytes from global infoseg
            // into log entry (these match our TIMESTAMP
            // declaration in ring0api.h)
            memcpy(&pEntry->stamp,
                   G_pGDT,
                   sizeof(TIMESTAMP));

            // 2) return ptr to event-specific data
            pvReturn = (PBYTE)pEntry + sizeof(EVENTLOGENTRY);

            // update the current LOGBUF
            G_pLogLast->cbUsed += cbLogEntry;
            ++(G_pLogLast->cLogEntries);

            // update global ring-0 status
            ++(G_R0Status.cLogged);

            // unblock ring-3 thread, if blocked
            if (G_idLogBlock)
                DevHlp32_ProcRun(G_idLogBlock);
        }

        VDHReleaseMutexSem(G_hmtx);
    }

    return pvReturn;
}

/*
 *@@ ctxtStopLogging:
 *      gets called from ioctlDeregisterDaemon to clean up
 *      when the driver is closed. Aside from freeing all
 *      remaining buffers, we must explicitly unblock the
 *      ring-3 thread or we'll end up with a zombie shell.
 *
 *      Context: "close" request packet from XWPShell,
 *      or XWPShell ring-3 logging thread.
 */

VOID ctxtStopLogging(VOID)
{
    // free all memory
    PLOGBUF pThis = G_pLogFirst;
    while (pThis)
    {
        PLOGBUF pNext = pThis->pNext;
        --(G_R0Status.cLogBufs);
        utilFreeFixed(pThis,
                      LOGBUFSIZE);     // 64K
        pThis = pNext;
    }

    G_pLogFirst
    = G_pLogLast
    = NULL;

    // stop logging until next open
    G_bLog = LOG_ERROR;

    // force running the logger thread
    // or we'll have a zombie XWPSHELL;
    // we MUST check G_idLogBlock because
    // we also get called from ctxtFillLogBuf()
    // on errors, when G_idLogBlock is already null
    if (G_idLogBlock)
        DevHlp32_ProcRun(G_idLogBlock);
}

/*
 *@@ ctxtFillLogBuf:
 *      implementation for XWPSECIO_GETLOGBUF in sec32_ioctl().
 *
 *      This gets called at task time of the ring-3 logging
 *      thread only. That thread keeps calling this IOCtl
 *      function and assumes to be blocked until data is
 *      available.
 *
 *      See LOGBUF for the general event flow.
 *
 *      This function must:
 *
 *      1)  Check if logging data is available.
 *
 *          --  If we have data, go to 2).
 *
 *          --  If not, we block. We will get unblocked by
 *              ctxtLogEvent as soon as data comes in.
 *
 *      2)  Copy first logging buffer to ring-3 memory.
 *
 *      3)  In any case, we return with logging data.
 *
 *      Context: XWPSECIO_GETLOGBUF ioctl request packet
 *      from XWPShell only, that is, the ring-3 logging
 *      thread.
 */

IOCTLRET ctxtFillLogBuf(PLOGBUF pLogBufR3,              // in: flat pointer to ring-3 mem from ioctl
                        ULONG blockid)                  // in: 16:16 pointer of reqpkt for ProcBlock id
{
    IOCTLRET    rc = NO_ERROR;

    switch (G_bLog)
    {
        case LOG_INACTIVE:
            // very first call:
            // activate logging from now on
            G_bLog = LOG_ACTIVE;
        break;

        case LOG_ERROR:
            // we had an error previously:
            // never do anything then
            return ERROR_I24_GEN_FAILURE;
    }

    _disable();
    while (    (!G_pLogFirst)
            && (G_bLog == LOG_ACTIVE)
                    // keep rechecking G_bLog too because we get unblocked
                    // also when the driver is closed, and we'll end up
                    // with a zombie XWPShell in that case otherwise
          )
    {
        // no logging data yet:
        // block on the 16-bit request packet address
        G_idLogBlock = blockid;
        if (DevHlp32_ProcBlock(G_idLogBlock,
                               -1,     // wait forever
                               0))     // interruptible
        {
            // probably thread died or something
            rc = ERROR_I24_CHAR_CALL_INTERRUPTED;
            break;
        }

        _disable();
    }
    _enable();

    // in any case, thread is no longer blocked,
    // so reset global blockid
    G_idLogBlock = 0;

    // the logging memory _might_ have been freed
    // if the driver was closed from XWPShell via
    // ctxtStopLogging(), so check again if we
    // really have memory
    if (    (!rc)
         && (G_pLogFirst)
         && (G_bLog == LOG_ACTIVE)
       )
    {
        PLOGBUF     pFirst;

        // buffer status:
        //                                   º case 1: only one buf   º case 2: two bufs       º case 3: four bufs
        //                                   º         G_pLogFirst    º         G_pLogFirst    º         G_pLogFirst
        //                                   º         ³   G_pLogLast º         ³   G_pLogLast º         ³   G_pLogLast
        //                                   º         ³   ³   pFirst º         ³   ³   pFirst º         ³   ³   pFirst
        //                                   º         ³   ³   ³      º         ³   ³   ³      º         ³   ³   ³
        //                                   º         B1  B1  ?      º         B1  B2  ?      º         B1  B4  ?
        // we have logging data:             º         ³   ³   ³      º         ³   ³   ³      º         ³   ³   ³
        memcpy(pLogBufR3,              //    º         ³   ³   ³      º         ³   ³   ³      º         ³   ³   ³
               G_pLogFirst,            //    º         ³   ³   ³      º         ³   ³   ³      º         ³   ³   ³
               G_pLogFirst->cbUsed);   //    º         ³   ³   ³      º         ³   ³   ³      º         ³   ³   ³
                                       //    º         ³   ³   ³      º         ³   ³   ³      º         ³   ³   ³
        // unlink this record                º         ³   ³   ³      º         ³   ³   ³      º         ³   ³   ³
        pFirst = G_pLogFirst;          //    º         ³   ³   B1     º         ³   ³   B1     º         ³   ³   B1
                                       //    º         ³   ³          º         ³   ³          º         ³   ³
        if (pFirst == G_pLogLast)      //    º yes:    ³   ³          º no      ³   ³          º no      ³   ³
            // we only had one buffer:       º         ³   ³          º         ³   ³          º         ³   ³
            // unset last                    º         ³   ³          º         ³   ³          º         ³   ³
            G_pLogLast = NULL;         //    º         ³   NUL        º         ³   ³          º         ³   ³
                                       //    º         ³   ³          º         ³   ³          º         ³   ³
        G_pLogFirst = G_pLogFirst->pNext; // º         NUL ³          º         ³   ³          º         ³   ³
            // will be NULL if this was last º         ³   ³          º         B2  ³          º         B2  ³
        --(G_R0Status.cLogBufs);       //    º         ³   ³          º         ³   ³          º         ³   ³
                                       //    º         ³   ³          º         ³   ³          º         ³   ³
        utilFreeFixed(pFirst,          //    º         ³   ³          º         ³   ³          º         ³   ³
                      LOGBUFSIZE);  // 64K   º         ³   ³          º         ³   ³          º         ³   ³
    }                                  // final:       NUL NUL        º         B2  B2         º         B2  B4
    else
    {
        // to be safe, free all buffers
        // if we had an error (this won't call
        // ProcRun since the blockid is null)
        ctxtStopLogging();

        rc = ERROR_I24_CHAR_CALL_INTERRUPTED;
    }

    return rc;
}

/* ******************************************************************
 *
 *   Security contexts implementation
 *
 ********************************************************************/

/*
 *@@ ctxtCreate:
 *      allocates a new security context for the given process
 *      and inserts it into our private list of contexts.
 *
 *      This initializes all fields _except_ the aSubjects array,
 *      which is left to the caller.
 *
 *      Returns NULL on errors.
 *
 *      WARNING: This allocates a small buffer, so this may block
 *      in rare cases.
 *
 *@@added V1.0.2 (2003-11-09) [umoeller]
 */

PXWPSECURITYCONTEXT ctxtCreate(USHORT pidNew,       // in: pid of newly created process
                               USHORT pidParent,    // in: pid of that process's parent
                               ULONG cSubjects)     // in: no. of subject handles required (>= 1)
{
    ULONG cbStruct =    sizeof(XWPSECURITYCONTEXT)
                      + sizeof(HXSUBJECT) * (cSubjects - 1);
    PXWPSECURITYCONTEXT pContext;
    if (!(pContext = utilAllocFixed(cbStruct)))
        return NULL;

    pContext->tree.ulKey = pidNew;
    pContext->cbStruct = cbStruct;
    pContext->cOpenFiles = 0;
    pContext->ctxt.pidParent = pidParent;
    pContext->ctxt.cSubjects = cSubjects;

    // if this fails, we have a duplicate PID and a problem
    if (treeInsert(&G_ContextsTree,
                   &G_R0Status.cContexts,
                   (TREE*)pContext,
                   treeCompareKeys))
    {
        utilFreeFixed(pContext, cbStruct);
        return NULL;
    }

    return pContext;
}

/*
 *@@ ctxtFind:
 *      returns the security context that corresponds to the
 *      given process ID, or NULL if there is none.
 */

PXWPSECURITYCONTEXT ctxtFind(USHORT pid)
{
    PXWPSECURITYCONTEXT pContext;
    if (!(pContext = (PXWPSECURITYCONTEXT)treeFind(G_ContextsTree,
                                                   pid,
                                                   treeCompareKeys)))
        // if we can't find a context, but just got a CREATEVDM
        // on the parent process beforehand, then create a new
        // context right here
        if (G_pContextCreateVDM)
        {
            if (pContext = ctxtCreate(pid,      // new pid
                                      G_pContextCreateVDM->tree.ulKey,  // parent pid: that of last CREATEVDM
                                      G_pContextCreateVDM->ctxt.cSubjects))
            {
                memcpy(&pContext->ctxt.aSubjects,
                       &G_pContextCreateVDM->ctxt.aSubjects,
                       sizeof(HXSUBJECT) * G_pContextCreateVDM->ctxt.cSubjects);
            }

            G_pContextCreateVDM = NULL;
        }

    return pContext;
}

/*
 *@@ ctxtFree:
 *      deletes the given security context.
 */

VOID ctxtFree(PXWPSECURITYCONTEXT pCtxt)
{
    if (!treeDelete(&G_ContextsTree,
                    &G_R0Status.cContexts,
                    (TREE*)pCtxt))
        utilFreeFixed((TREE*)pCtxt,
                      pCtxt->cbStruct);
}

/*
 *@@ ctxtClearAll:
 *      deletes all security contexts presently held in
 *      ring-0 memory.
 *
 *      This is a janitor function that should get called
 *      only when XWPShell exits, that is, only in
 *      debug builds.
 */

VOID ctxtClearAll(VOID)
{
    TREE* pNode;
    while (pNode = treeFirst(G_ContextsTree))
    {
        ctxtFree((XWPSECURITYCONTEXT*)pNode);
    }

    if (G_pRing0Buf)
    {
        utilFreeFixed(G_pRing0Buf, G_pRing0Buf->cbTotal);
        G_pRing0Buf = NULL;
    }
}

/* ******************************************************************
 *
 *   Authorization
 *
 ********************************************************************/

/*
 *@@ ctxtSendACLs:
 *      implementation for XWPSECIO_SENDACLS (sec32_ioctl()).
 *
 *      This must return an ioctl error code, that is, at least
 *      STDON must be set (see sec32_ioctl()).
 *
 *@@added V1.0.2 (2003-11-13) [umoeller]
 */

int ctxtSendACLs(PRING0BUF pBufR3)
{
    int     rc = STDON;
    ULONG   cbNew = 0;

    PRING0BUF pNewBuf;

    if (!(pNewBuf = utilAllocFixed(pBufR3->cbTotal)))
        rc = STDON | STERR | XWPERR_I24_INVALID_ACL_FORMAT;
    else
        memcpy(pNewBuf,
               pBufR3,
               pBufR3->cbTotal);

    if (G_pRing0Buf)
        utilFreeFixed(G_pRing0Buf, G_pRing0Buf->cbTotal);

    if (G_pRing0Buf = pNewBuf)
        cbNew = pNewBuf->cbTotal;

    G_R0Status.cbACLs = cbNew;

    return rc;
}

/*
 *@@ FindAccess:
 *
 *      Preconditions:
 *
 *      --  Caller must have checked that G_pRing0Buf is not NULL.
 *
 *      --  pcszResource must be upper-cased.
 *
 *      Restrictions:
 *
 *      --  This function must not block since we use
 *          a global scratch buffer.
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

PACCESS FindAccess(PCSZ pcszResource,
                   PULONG pcAccesses)       // out: array item count
{
    PBYTE   pbCurrent = (PBYTE)G_pRing0Buf + sizeof(RING0BUF);
    ULONG   ul;

    for (ul = 0;
         ul < G_pRing0Buf->cACLs;
         ++ul)
    {
        PRESOURCEACL pEntry = (PRESOURCEACL)pbCurrent;
        if (!strcmp(pcszResource,
                    pEntry->szName))
        {
            *pcAccesses = pEntry->cAccesses;
            return (PACCESS)((PBYTE)pEntry->szName + pEntry->cbName);
        }

        pbCurrent += pEntry->cbStruct;
    }

    return NULL;
}

/*
 *@@ QueryPermissions:
 *      returns the ORed XWPACCESS_* flags for the
 *      given resource based on the security context
 *      represented by the given subject handles.
 *
 *      Restrictions:
 *
 *      --  This function must not block since we use
 *          a global scratch buffer.
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

ULONG ctxtQueryPermissions(PCSZ pcszResource,
                           ULONG ulResourceLen,     // in: strlen(pcszResource)
                           ULONG cSubjects,
                           const HXSUBJECT *paSubjects)
{
    PACCESS paAccesses;
    ULONG   cAccesses;
    PSZ     pszResource;

    ULONG   ulRemainingLen = ulResourceLen;

    if (!*paSubjects || !G_pRing0Buf)
        // null subject handle (root) or
        // ACLs not active: allow everything
        return XWPACCESS_ALL;

    memcpy(G_szResource,
           pcszResource,
           ulResourceLen + 1);
    pszResource = __strupr(G_szResource);        // @@todo use fshelper which is nls-aware

    while (TRUE)
    {
        PSZ p;

        PEVENTBUF_FILENAME pBuf;
        if (pBuf = ctxtLogEvent(NULL,
                                EVENT_FINDPERMISSIONS,
                                sizeof(EVENTBUF_FILENAME) + ulRemainingLen))
        {
            pBuf->rc = -1;
            pBuf->ulPathLen = ulRemainingLen;
            memcpy(pBuf->szPath,
                   pszResource,
                   ulRemainingLen + 1);
        }

        if (paAccesses = FindAccess(pszResource,
                                    __StackToFlat(&cAccesses)))
        {
            ULONG   flAccess = 0;
            ULONG   ulA,
                    ulS;

            for (ulA = 0;
                 ulA < cAccesses;
                 ++ulA)
            {
                for (ulS = 0;
                     ulS < cSubjects;
                     ++ulS)
                {
                    if (paSubjects[ulS] == paAccesses[ulA].hSubject)
                        flAccess |= paAccesses[ulA].flAccess;
                }
            }

            if (pBuf = ctxtLogEvent(NULL,
                                    EVENT_FINDPERMISSIONS,
                                    sizeof(EVENTBUF_FILENAME) + ulRemainingLen))
            {
                pBuf->rc = flAccess;
                pBuf->ulPathLen = ulRemainingLen;
                memcpy(pBuf->szPath,
                       pszResource,
                       ulRemainingLen + 1);
            }

            return flAccess;
        }

        // not found: climb up to parent
        if (p = strrchr(pszResource + 1,        // we can have entries like "\DEV"
                        '\\'))
        {
            *p = 0;
            ulRemainingLen = p - pszResource;
        }
        else
            break;
    }

    return 0;
}


