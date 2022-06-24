
/*
 *@@sourcefile callb_exec.c:
 *      SES kernel hook code.
 *
 *      See strat_init_base.c for an introduction to the driver
 *      structure in general.
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
// #include <secure.h>

#include <string.h>

#include "helpers\tree.h"
#include "helpers\xwpsecty.h"

#include "xwpsec32.sys\types.h"
#include "xwpsec32.sys\StackToFlat.h"
#include "xwpsec32.sys\devhlp32.h"

#include "security\ring0api.h"

#include "xwpsec32.sys\xwpsec_callbacks.h"

/* ******************************************************************
 *
 *   Globals
 *
 ********************************************************************/

extern PXWPSECURITYCONTEXT G_pContextCreateVDM = NULL;

/* ******************************************************************
 *
 *   Callouts
 *
 ********************************************************************/

/*
 *@@ LOADEROPEN:
 *      SES kernel hook for LOADEROPEN.
 *
 *      As with all our hooks, this is stored in G_SecurityHooks
 *      (sec32_callbacks.c) force the OS/2 kernel to call us for
 *      each such event.
 *
 *      This is a "pre" event. Required privileges:
 *
 *      --  XWPACCESS_EXEC on the executable.
 *
 *      LOADEROPEN always receives a fully qualified pathname.
 *
 *      --  For EXE files, we get this sequence for the EXE file,
 *          running in the context of the thread that is running
 *          DosExecPgm:
 *
 *          1)  OPEN_PRE, OPEN_POST
 *          2)  LOADEROPEN
 *          3)  GETMODULE
 *          4)  EXEC_PRE, EXEC_POST (which returns the new PID)
 *
 *          Note that DosStartSession and WinStartApp are special
 *          cases. The callbacks apparently appear on the process
 *          that actually calls these APIs, but the parent process
 *          is somehow magically switched to be that of the first
 *          PMSHELL (or whatever process owns the session manager).
 *
 *      --  For unqualified DLL names that have not yet been loaded,
 *          we get:
 *
 *          1)  GETMODULE short name
 *          2)  OPEN_PRE, OPEN_POST with the long name for every
 *              directory along the LIBPATH
 *          3)  LOADEROPEN with the long name and SFN that was found
 *
 *      --  For unqualified DLL names that are already loaded, we get:
 *
 *          1)  GETMODULE short name
 *
 *          only. @@todo How do we authenticate this? We can
 *          presume that this module is already loaded, and we
 *          must check the full file's permissions!
 *
 *      --  For fully qualified DLL names, we get:
 *
 *          1)  GETMODULE full name
 *          2)  OPEN_PRE, OPEN_POST with the full name
 *          3)  LOADEROPEN
 *
 *      Context: Possibly any ring-3 thread on the system.
 */

ULONG LOADEROPEN(PSZ pszPath,
                 ULONG SFN)
{
    APIRET  rc = NO_ERROR;

    if (    (G_pidShell)
         && (!DevHlp32_GetInfoSegs(&G_pGDT,
                                   &G_pLDT))
       )
    {
        // authorize event if it is not from XWPShell
        PXWPSECURITYCONTEXT pThisContext;
        if (G_pidShell != G_pLDT->LIS_CurProcID)
        {
            if (!(pThisContext = ctxtFind(G_pLDT->LIS_CurProcID)))
                rc = G_rcUnknownContext;
            else
                ;
        }
        else
            pThisContext = NULL;

        if (G_bLog == LOG_ACTIVE)
        {
            PEVENTBUF_LOADEROPEN pBuf;
            ULONG   ulPathLen = strlen(pszPath);

            if (pBuf = ctxtLogEvent(pThisContext,
                                    EVENT_LOADEROPEN,
                                    sizeof(EVENTBUF_LOADEROPEN) + ulPathLen))
            {
                pBuf->SFN = SFN;
                pBuf->rc = rc;
                pBuf->ulPathLen = ulPathLen;
                memcpy(pBuf->szPath,
                       pszPath,
                       ulPathLen + 1);
            }
        }
    }

    return rc;
}

/*
 *@@ GETMODULE:
 *      SES kernel hook for GETMODULE.
 *
 *      As with all our hooks, this is stored in G_SecurityHooks
 *      (sec32_callbacks.c) force the OS/2 kernel to call us for
 *      each such event.
 *
 *      This is a "pre" event. Required privileges:
 *
 *      --  XWPACCESS_EXEC on the executable.
 *
 *      This can come in with "pure" (unqualified) module names.
 *      See LOADEROPEN for details.
 *
 *      However if the module is already in memory, it is not
 *      followed by any other callout. We should then check if
 *      the current task is allowed to see the module. @@todo
 *      How do we figure out the access rights if the module
 *      name is _not_ fully qualified?
 *
 *      Context: Possibly any ring-3 thread on the system.
 */

ULONG GETMODULE(PSZ pszPath)
{
    APIRET  rc = NO_ERROR;

    if (    (G_pidShell)
         && (!DevHlp32_GetInfoSegs(&G_pGDT,
                                   &G_pLDT))
       )
    {
        // authorize event if it is not from XWPShell
        PXWPSECURITYCONTEXT pThisContext;
        if (G_pidShell != G_pLDT->LIS_CurProcID)
        {
            if (!(pThisContext = ctxtFind(G_pLDT->LIS_CurProcID)))
                rc = G_rcUnknownContext;
            else
                ;
        }
        else
            pThisContext = NULL;

        if (G_bLog == LOG_ACTIVE)
        {
            PEVENTBUF_FILENAME pBuf;
            ULONG   ulPathLen = strlen(pszPath);

            if (pBuf = ctxtLogEvent(pThisContext,
                                    EVENT_GETMODULE,
                                    sizeof(EVENTBUF_FILENAME) + ulPathLen))
            {
                pBuf->rc = rc;
                pBuf->ulPathLen = ulPathLen;
                memcpy(pBuf->szPath,
                       pszPath,
                       ulPathLen + 1);
            }
        }
    }

    return rc;
}

/*
 *@@ EXECPGM:
 *      SES kernel hook for EXECPGM.
 *
 *      As with all our hooks, this is stored in G_SecurityHooks
 *      (sec32_callbacks.c) force the OS/2 kernel to call us for
 *      each such event.
 *
 *      Comes in for EXE files only.
 *
 *      Required privileges:
 *
 *      --  XWPACCESS_EXEC on the executable.
 *
 *      Context: Possibly any ring-3 thread on the system.
 */

ULONG EXECPGM(PSZ pszPath,
              PCHAR pchArgs)
{
    APIRET  rc = NO_ERROR;

    // reset global ptr that might be dangling from
    // last createvdm
    G_pContextCreateVDM = NULL;

    if (    (G_pidShell)
         && (!DevHlp32_GetInfoSegs(&G_pGDT,
                                   &G_pLDT))
       )
    {
        // authorize event if it is not from XWPShell
        PXWPSECURITYCONTEXT pThisContext;
        USHORT  fsGranted = 0;
        ULONG   ulPathLen = strlen(pszPath);

        if (G_pidShell != G_pLDT->LIS_CurProcID)
        {
            if (!(pThisContext = ctxtFind(G_pLDT->LIS_CurProcID)))
                rc = G_rcUnknownContext;
            else
            {
                fsGranted = ctxtQueryPermissions(pszPath,
                                                 ulPathLen,
                                                 pThisContext->ctxt.cSubjects,
                                                 pThisContext->ctxt.aSubjects);

                // all bits of fsRequired must be set in fsGranted
                if ((fsGranted & XWPACCESS_EXEC) != XWPACCESS_EXEC)
                    rc = ERROR_ACCESS_DENIED;
            }
        }
        else
            pThisContext = NULL;

        if (G_bLog == LOG_ACTIVE)
        {
            PEVENTBUF_FILENAME pBuf;

            if (pBuf = ctxtLogEvent(pThisContext,
                                    EVENT_EXECPGM_PRE,
                                    sizeof(EVENTBUF_FILENAME) + ulPathLen))
            {
                pBuf->rc = rc;
                pBuf->fsRequired = XWPACCESS_EXEC;
                pBuf->fsGranted = fsGranted;
                pBuf->ulPathLen = ulPathLen;
                memcpy(pBuf->szPath,
                       pszPath,
                       ulPathLen + 1);

                // log arguments in a second buffer because
                // this can be up to 64K in itself @@todo
                // EVENT_EXECPGM_ARGS
            }
        }
    }

    return rc;
}

/*
 *@@ EXECPGM_POST:
 *      SES kernel hook for EXECPGM_POST.
 *
 *      As with all our hooks, this is stored in G_SecurityHooks
 *      (sec32_callbacks.c) force the OS/2 kernel to call us for
 *      each such event.
 *
 *      Comes in for EXE files only, after a new process has been
 *      created.
 *
 *      This call is somewhat special because whenever a new process
 *      has been created on the system, we must create a new security
 *      context and copy the current credentials (security handles)
 *      into it. The parent process to copy the credentials from is
 *      the current one on which this callback gets called.
 *
 *      Context: Possibly any ring-3 thread on the system.
 */

VOID EXECPGM_POST(PSZ pszPath,
                  PCHAR pchArgs,
                  ULONG NewPID)
{
    if (    (G_pidShell)
         && (!DevHlp32_GetInfoSegs(&G_pGDT,
                                   &G_pLDT))
       )
    {
        // find the current security context
        // (i.e. the parent process of new process)
        PXWPSECURITYCONTEXT pThisContext,
                            pNewContext = NULL;

        // alloc a new security context for this process
        // and copy the parent's (current) credentials into it
        if (    (pThisContext = ctxtFind(G_pLDT->LIS_CurProcID))
             && ((pNewContext = ctxtCreate(NewPID,
                                           G_pLDT->LIS_CurProcID,        // parent PID
                                           pThisContext->ctxt.cSubjects)))
           )
        {
            memcpy(&pNewContext->ctxt.aSubjects,
                   &pThisContext->ctxt.aSubjects,
                   sizeof(HXSUBJECT) * pThisContext->ctxt.cSubjects);
        }

        if (G_bLog == LOG_ACTIVE)
        {
            PEVENTBUF_FILENAME pBuf;
            ULONG   ulPathLen = strlen(pszPath);

            if (pBuf = ctxtLogEvent(pNewContext,
                                    EVENT_EXECPGM_POST,
                                    sizeof(EVENTBUF_FILENAME) + ulPathLen))
            {
                pBuf->rc = NewPID;
                pBuf->ulPathLen = ulPathLen;
                memcpy(pBuf->szPath,
                       pszPath,
                       ulPathLen + 1);
            }
        }
    }
}

/*
 *@@ CREATEVDM:
 *
 *@@added V1.0.2 (2003-11-13) [umoeller]
 */

ULONG CREATEVDM(PSZ pszProgram,
                PSZ pszArgs)
{
    APIRET  rc = NO_ERROR;

    if (    (G_pidShell)
         && (!DevHlp32_GetInfoSegs(&G_pGDT,
                                   &G_pLDT))
       )
    {
        // authorize event if it is not from XWPShell
        PXWPSECURITYCONTEXT pThisContext;
        USHORT  fsGranted = 0;
        ULONG   ulPathLen = 0;      // can come in as null!

        if (G_pidShell != G_pLDT->LIS_CurProcID)
        {
            if (!(pThisContext = ctxtFind(G_pLDT->LIS_CurProcID)))
                rc = G_rcUnknownContext;
            else
            {
                if (    (pszProgram)        // can be NULL!
                     && (ulPathLen = strlen(pszProgram))
                   )
                {
                    fsGranted = ctxtQueryPermissions(pszProgram,
                                                     ulPathLen,
                                                     pThisContext->ctxt.cSubjects,
                                                     pThisContext->ctxt.aSubjects);

                    // all bits of fsRequired must be set in fsGranted
                    if ((fsGranted & XWPACCESS_EXEC) != XWPACCESS_EXEC)
                        rc = ERROR_ACCESS_DENIED;
                }
                // else @@todo?!?
            }
        }
        else
            pThisContext = NULL;

        if (G_bLog == LOG_ACTIVE)
        {
            PEVENTBUF_FILENAME pBuf;

            if (pBuf = ctxtLogEvent(pThisContext,
                                    EVENT_CREATEVDM_PRE,
                                    sizeof(EVENTBUF_FILENAME) + ulPathLen))
            {
                pBuf->rc = rc;
                pBuf->fsRequired = XWPACCESS_EXEC;
                pBuf->fsGranted = fsGranted;
                if (pBuf->ulPathLen = ulPathLen)
                    memcpy(pBuf->szPath,
                           pszProgram,
                           ulPathLen + 1);
                else
                    pBuf->szPath[0] = 0;

                // log arguments in a second buffer because
                // this can be up to 64K in itself @@todo
                // EVENT_EXECPGM_ARGS
            }
        }
    }

    return rc;
}

/*
 *@@ CREATEVDM_POST:
 *
 *@@added V1.0.2 (2003-11-13) [umoeller]
 */

VOID CREATEVDM_POST(int rc)
{
    if (    (G_pidShell)
         && (!DevHlp32_GetInfoSegs(&G_pGDT,
                                   &G_pLDT))
       )
    {
        // find the current security context
        // (i.e. the parent process of new process)
        PXWPSECURITYCONTEXT pThisContext,
                            pNewContext = NULL;

        if (G_bLog == LOG_ACTIVE)
        {
            PEVENTBUF_CLOSE pBuf;

            if (pBuf = ctxtLogEvent(pNewContext,
                                    EVENT_CREATEVDM_POST,
                                    sizeof(EVENTBUF_CLOSE)))
            {
                pBuf->SFN = rc;
            }
        }
    }
}
