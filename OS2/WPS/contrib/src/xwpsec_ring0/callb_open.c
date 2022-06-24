
/*
 *@@sourcefile callb_open.c:
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
 *   Callouts
 *
 ********************************************************************/

/*
 *@@ OPEN_PRE:
 *      SES kernel hook for OPEN_PRE.
 *
 *      As with all our hooks, this is stored in G_SecurityHooks
 *      (sec32_callbacks.c) force the OS/2 kernel to call us for
 *      each such event.
 *
 *      This is a "pre" event. Required privileges:
 *
 *      --  If (fsOpenFlags & OPEN_ACTION_CREATE_IF_NEW):
 *          XWPACCESS_CREATE
 *
 *      --  If (fsOpenFlags & OPEN_ACTION_OPEN_IF_EXISTS):
 *          XWPACCESS_READ
 *
 *      --  If (fsOpenFlags & OPEN_ACTION_REPLACE_IF_EXISTS):
 *          XWPACCESS_CREATE
 *
 *      --  If (fsOpenMode & OPEN_FLAGS_DASD) (open drive):
 *          XWPACCESS_WRITE and XWPACCESS_DELETE and XWPACCESS_CREATE
 *          for the entire drive (root directory).
 *
 *      --  If (fsOpenMode & OPEN_ACCESS_READONLY):
 *          XWPACCESS_READ
 *
 *      --  If (fsOpenMode & OPEN_ACCESS_WRITEONLY):
 *          XWPACCESS_WRITE
 *
 *      --  If (fsOpenMode & OPEN_ACCESS_READWRITE):
 *          XWPACCESS_READ and XWPACCESS_WRITE
 *
 *      Context: Possibly any ring-3 thread on the system.
 */

ULONG CallType OPEN_PRE(PSZ pszPath,        // in: full path of file
                        ULONG fsOpenFlags,  // in: open flags
                        ULONG fsOpenMode,   // in: open mode
                        ULONG SFN)          // in: system file number
{
    APIRET  rc = NO_ERROR;

    if (    (G_pidShell)
         && (!DevHlp32_GetInfoSegs(&G_pGDT,
                                   &G_pLDT))
       )
    {
        // authorize event if it is not from XWPShell
        PXWPSECURITYCONTEXT pThisContext;
        USHORT  fsRequired = 0,
                fsGranted = 0;
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

                // DosOpen flags
                /* if (fsOpenMode & OPEN_FLAGS_DASD) // open drive:               0x8000
                    flRequired = XWPACCESS_WRITE | XWPACCESS_DELETE | XWPACCESS_CREATE;
                    // @@todo: check drive?
                else */
                {
                    if (fsOpenFlags & (  OPEN_ACTION_CREATE_IF_NEW          // 0x0010
                                       | OPEN_ACTION_REPLACE_IF_EXISTS))    // 0x0002
                                            // @@todo: should "replace" really require "C" perm?
                        fsRequired |= XWPACCESS_CREATE;
                    if (fsOpenFlags & OPEN_ACTION_OPEN_IF_EXISTS)           // 0x0001
                        fsRequired |= XWPACCESS_READ;

                    // OPEN_ACCESS_READONLY           0x0000  /* ---- ---- ---- -000 */
                    // OPEN_ACCESS_WRITEONLY          0x0001  /* ---- ---- ---- -001 */
                    // OPEN_ACCESS_READWRITE          0x0002  /* ---- ---- ---- -010 */
                    switch (fsOpenMode & 3)
                    {
                        case OPEN_ACCESS_READONLY:
                            fsRequired = XWPACCESS_READ;
                        break;

                        case OPEN_ACCESS_WRITEONLY:
                            fsRequired = XWPACCESS_WRITE;
                        break;

                        default:
                            fsRequired = XWPACCESS_READ | XWPACCESS_WRITE;
                        break;
                    }
                }

                // all bits of fsRequired must be set in fsGranted
                if ((fsGranted & fsRequired) != fsRequired)
                    rc = ERROR_ACCESS_DENIED;
            }
        }
        else
            pThisContext = NULL;

        if (G_bLog == LOG_ACTIVE)
        {
            PEVENTBUF_OPEN pBuf;
            if (pBuf = ctxtLogEvent(pThisContext,
                                    EVENT_OPEN_PRE,
                                    sizeof(EVENTBUF_OPEN) + ulPathLen))
            {
                pBuf->fsOpenFlags = fsOpenFlags;
                pBuf->fsOpenMode = fsOpenMode;
                pBuf->SFN = SFN;
                pBuf->Action = 0;           // not used with OPEN_PRE
                pBuf->rc = rc;
                pBuf->PRE.fsRequired = fsRequired;
                pBuf->PRE.fsGranted = fsGranted;
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
 *@@ OPEN_POST:
 *      security callback for OPEN_POST.
 *
 *      As with all our hooks, this is stored in G_SecurityHooks
 *      (sec32_callbacks.c) force the OS/2 kernel to call us for
 *      each such event.
 *
 *      Context: Possibly any ring-3 thread on the system.
 */

ULONG CallType OPEN_POST(PSZ pszPath,
                         ULONG fsOpenFlags,
                         ULONG fsOpenMode,
                         ULONG SFN,
                         ULONG Action,
                         ULONG RC)
{
    if (    (G_pidShell)
         && (!DevHlp32_GetInfoSegs(&G_pGDT,
                                   &G_pLDT))
       )
    {
        PXWPSECURITYCONTEXT pThisContext;

        if (!RC)
        {
            // successful call:
            if ((pThisContext = ctxtFind(G_pLDT->LIS_CurProcID)))
                ++(pThisContext->cOpenFiles);
        }
        else
            pThisContext = NULL;

        if (G_bLog == LOG_ACTIVE)
        {
            PEVENTBUF_OPEN pBuf;
            ULONG   ulPathLen = strlen(pszPath);

            if (pBuf = ctxtLogEvent(pThisContext,
                                    EVENT_OPEN_POST,
                                    sizeof(EVENTBUF_OPEN) + ulPathLen))
            {
                pBuf->fsOpenFlags = fsOpenFlags;
                pBuf->fsOpenMode = fsOpenMode;
                pBuf->SFN = SFN;
                pBuf->Action = Action;
                pBuf->rc = RC;
                if (pThisContext)
                    pBuf->POST.cOpenFiles = pThisContext->cOpenFiles;
                pBuf->ulPathLen = ulPathLen;
                memcpy(pBuf->szPath,
                       pszPath,
                       ulPathLen + 1);
            }
        }
    }

    return NO_ERROR;
}


