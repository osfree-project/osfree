
/*
 *@@sourcefile strat_ioctl.c:
 *      strategy routines for PDD "open", "ioctl", and "close"
 *      commands, plus the implementation for the various
 *      IOCtl commands supported by the driver.
 *
 *      See strat_init_base.c for an introduction.
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
 *      Based on the MWDD32.SYS example sources,
 *      Copyright (C) 1995, 1996, 1997  Matthieu Willm (willm@ibm.net).
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

#include "security\ring0api.h"

#include "xwpsec32.sys\xwpsec_callbacks.h"

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

extern ULONG        G_open_count = 0;

extern PPROCESSLIST G_pplTrusted = NULL;           // global list of trusted processes (fixed mem)

extern XWPSECSTATUS G_R0Status = {0};

extern HVDHSEM      G_hmtx = NULLHANDLE;

/* ******************************************************************
 *
 *   IOCtl implementation
 *
 ********************************************************************/

/*
 *@@ ioctlRegisterDaemon:
 *      implementation for XWPSECIO_REGISTER in sec32_ioctl.
 *
 *      If this returns NO_ERROR, access control is ENABLED
 *      globally.
 *
 *      Context: XWPShell task time.
 */

IOCTLRET ioctlRegisterDaemon(struct reqpkt_ioctl *pRequest)   // flat ptr to request packet
{
    PPROCESSLIST pplShell;
    ULONG ul;

    // get flat pointer to data packet, which is PROCESSLIST
    if (DevHlp32_VirtToLin(pRequest->data,
                           __StackToFlat(&pplShell)))
        return ERROR_I24_INVALID_PARAMETER;

    // clear out the old security contexts, in case xwpshell
    // got restarted (should only happen on development machines!)
    ctxtClearAll();

    // for each process in the shell's list of process, create
    // one priviledged security context
    for (ul = 0;
         ul < pplShell->cTrusted;
         ++ul)
    {
        PXWPSECURITYCONTEXT pContext;
        if (!(pContext = ctxtCreate(pplShell->apidTrusted[ul],
                                    0,        // parent PID (unknown here)
                                    1)))      // 1 subject handle for root
            // oh boy
            return ERROR_I24_GEN_FAILURE;

        pContext->ctxt.aSubjects[0] = 0;        // root subject handle
    }

    // reset logging flag in case this is a reopen
    G_bLog = LOG_INACTIVE;

    // now get the caller's (shell's) PID and store that,
    // because we need that for extra privileges of the shell;
    // also once G_pidShell is != null,
    // ACCESS CONTROL IS ENABLED GLOBALLY
    if (DevHlp32_GetInfoSegs(&G_pGDT,
                             &G_pLDT))
        // error:
        return ERROR_I24_GEN_FAILURE;

    // alright from now on we authenticate!
    G_pidShell = G_pLDT->LIS_CurProcID;

    return NO_ERROR;
}

/*
 *@@ ioctlDeregisterDaemon:
 *      implementation for XWPSECIO_DEREGISTER in sec32_ioctl.
 *
 *      After this, access control is DISABLED.
 *
 *      Context: XWPShell task time.
 */

VOID ioctlDeregisterDaemon(VOID)
{
    if (G_pplTrusted)
    {
        utilFreeFixed(G_pplTrusted,
                      G_pplTrusted->cbStruct);
        G_pplTrusted = NULL;
    }

    ctxtStopLogging();

    ctxtClearAll();

    G_pidShell = 0;
}

/* ******************************************************************
 *
 *   Strategy commands
 *
 ********************************************************************/

/*
 *@@ sec32_open:
 *      this strategy command opens the device as a result of
 *      a valid DosOpen call on the driver.
 *
 *      This is called from sec32_strategy() since it's stored in
 *      driver_routing_table in sec32_strategy.c.
 *
 *      We allow "open" only once at any given time, that is,
 *      once XWPShell has opened the driver, nobody else can.
 *      This way we do not have to perform PID/TID validation
 *      for every ioctl that comes in.
 *
 *      Context: XWPShell task time.
 */

int sec32_open(PTR16 reqpkt)
{
    // no need for the request packet here, it only has
    // the SFN number for us...

    // we allow only one open at a time, which should be the
    // ring-3 control program, so check if we've been opened
    // already
    if (G_open_count)
        return STDON | STERR | ERROR_I24_DEVICE_IN_USE;

    // create our serialization kernel mutex on the first call
    if (!G_hmtx)
        if (!(VDHCreateSem(&G_hmtx, VDH_MUTEXSEM)))
            return STDON | STERR | ERROR_I24_GEN_FAILURE;

    // success:
    // set open count to one (will be decreased by "close"
    // strategy command again)
    ++G_open_count;

    return STDON;
}

/*
 *@@ sec32_ioctl:
 *      implementation for the "IOCtl" strategy call, which is
 *      the interface for the ring-3 shell.
 *      Gets called from sec32_strategy() since it's stored in
 *      driver_routing_table in sec32_strategy.c.
 *
 *      Since only one process may open the driver, we can
 *      be sure it's XWPShell doing the IOCtl here. This makes
 *      sure that no other process can attempt to implement a
 *      second access control mechanism (which would be a
 *      security hole).
 *
 *      The IOCtl request packet is:
 *
 +      struct reqpkt_ioctl {
 +             struct reqhdr  header;
 +             unsigned char  cat;      // category
 +             unsigned char  func;     // function
 +             PTR16          parm;
 +             PTR16          data;
 +             unsigned short sfn;
 +             unsigned short parmlen;
 +             unsigned short datalen;
 +         };
 *
 *      Context: XWPShell task time.
 */

int sec32_ioctl(PTR16 reqpkt)
{
    APIRET  rc;
    struct reqpkt_ioctl *pRequest;

    if (DevHlp32_VirtToLin(reqpkt,
                           __StackToFlat(&pRequest)))
        // could not thunk reqpkt:
        return STDON | STERR | ERROR_I24_INVALID_PARAMETER;

    // check category
    if (pRequest->cat != IOCTL_XWPSEC)      // our category
        return STDON | STERR | ERROR_I24_BAD_COMMAND;

    // check function
    switch (pRequest->func)
    {
        /*
         * XWPSECIO_REGISTER:
         *      called from XWPShell thread 1 for initialization.
         */

        case XWPSECIO_REGISTER:
            if (rc = ioctlRegisterDaemon(pRequest))
                return STDON | STERR | rc;
        break;

        /*
         * XWPSECIO_DEREGISTER:
         *      called from XWPShell thread 1 for cleanup.
         */

        case XWPSECIO_DEREGISTER:
            ioctlDeregisterDaemon();
        break;

        /*
         * XWPSECIO_GETLOGBUF:
         *      called from XWPShell logging thread for
         *      logging support.
         */

        case XWPSECIO_GETLOGBUF:
        {
            PLOGBUF     pLogBufR3;
            // get flat pointer to data packet, which is LOGBUF
            if (DevHlp32_VirtToLin(pRequest->data,
                                   __StackToFlat(&pLogBufR3)))
                return STDON | STERR | ERROR_I24_INVALID_PARAMETER;

            if (rc = ctxtFillLogBuf(pLogBufR3,
                                    // produce ULONG from 16:16 reqpkt ptr (ProcBlock id)
                                    (reqpkt.seg << 16) | reqpkt.ofs))
                return STDON | STERR | rc;
        }
        break;

        /*
         * XWPSECIO_QUERYSTATUS:
         *
         */

        case XWPSECIO_QUERYSTATUS:
        {
            XWPSECSTATUS    *pStatusR3;
            // get flat pointer to data packet, which is LOGBUF
            if (DevHlp32_VirtToLin(pRequest->data,
                                   __StackToFlat(&pStatusR3)))
                return STDON | STERR | ERROR_I24_INVALID_PARAMETER;

            memcpy(pStatusR3,
                   &G_R0Status,
                   sizeof(G_R0Status));
        }
        break;

        /*
         *@@ XWPSECIO_QUERYCONTEXT:
         *
         */

        case XWPSECIO_QUERYCONTEXT:
        {
            SECIOCONTEXT        *pCtxtR3;
            XWPSECURITYCONTEXT  *pThisContext;

            // get flat pointer to data packet, which is SECIOCONTEXT
            if (DevHlp32_VirtToLin(pRequest->data,
                                   __StackToFlat(&pCtxtR3)))
                return STDON | STERR | ERROR_I24_INVALID_PARAMETER;

            // look up the PID
            if (!(pThisContext = ctxtFind(pCtxtR3->pid)))
                return STDON | STERR | ERROR_I24_BAD_UNIT;

            if (pThisContext->ctxt.cSubjects > pCtxtR3->cSubjects)
                // we need more space
                return STDON | STERR | ERROR_I24_BAD_LENGTH;

            pCtxtR3->cSubjects = pThisContext->ctxt.cSubjects;
            memcpy(pCtxtR3->aSubjects,
                   pThisContext->ctxt.aSubjects,
                   sizeof(HXSUBJECT) * pThisContext->ctxt.cSubjects);
        }
        break;

        /*
         *@@ XWPSECIO_SETCONTEXT:
         *
         */

        case XWPSECIO_SETCONTEXT:
        {
            SECIOCONTEXT        *pCtxtR3;
            XWPSECURITYCONTEXT  *pThisContext;

            // get flat pointer to data packet, which is SECIOCONTEXT
            if (DevHlp32_VirtToLin(pRequest->data,
                                   __StackToFlat(&pCtxtR3)))
                return STDON | STERR | ERROR_I24_INVALID_PARAMETER;

            // look up the PID
            if (!(pThisContext = ctxtFind(pCtxtR3->pid)))
                return STDON | STERR | ERROR_I24_BAD_UNIT;

            if (pThisContext->ctxt.cSubjects > pCtxtR3->cSubjects)
            {
                // um, we need more space: reallocate this context
                USHORT pidParent = pThisContext->ctxt.pidParent;
                ctxtFree(pThisContext);
                if (!(pThisContext = ctxtCreate(pCtxtR3->pid,
                                                pidParent,
                                                pCtxtR3->cSubjects)))
                    return STDON | STERR | ERROR_I24_GEN_FAILURE;
            }

            pThisContext->ctxt.cSubjects = pCtxtR3->cSubjects;
            memcpy(pThisContext->ctxt.aSubjects,
                   pCtxtR3->aSubjects,
                   sizeof(HXSUBJECT) * pThisContext->ctxt.cSubjects);
        }
        break;

        /*
         *@@ XWPSECIO_SENDACLS:
         *
         */

        case XWPSECIO_SENDACLS:
        {
            PRING0BUF    pBufR3;
            // get flat pointer to data packet, which is SECIOCONTEXT
            if (DevHlp32_VirtToLin(pRequest->data,
                                   __StackToFlat(&pBufR3)))
                return STDON | STERR | ERROR_I24_INVALID_PARAMETER;

            return ctxtSendACLs(pBufR3);
        }
        break;

        default:
            return STDON | STERR | ERROR_I24_BAD_COMMAND;
    }

    return STDON;
}

/*
 *@@ sec32_close:
 *      implementation for the "close" strategy call, which is
 *      the interface for the ring-3 shell.
 *      Gets called from sec32_strategy() since it's stored in
 *      driver_routing_table in sec32_strategy.c.
 *
 *      Context: XWPShell task time.
 */

int sec32_close(PTR16 reqpkt)
{
    // no need for the request packet here, it only has
    // the SFN number for us...

    // to be safe, call deregister daemon first,
    // just in case the shell crashed and didn't
    // call the ioctl properly
    ioctlDeregisterDaemon();

    --G_open_count;

    return STDON;
}

