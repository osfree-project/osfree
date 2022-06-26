
/*
 *@@sourcefile callb_delete.c:
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
 *@@ DELETE_PRE:
 *      SES kernel hook for DELETE_PRE.
 *
 *      As with all our hooks, this is stored in G_SecurityHooks
 *      (sec32_callbacks.c) force the OS/2 kernel to call us for
 *      each such event.
 *
 *      This is a "pre" event. Required privileges:
 *
 *      -- XWPACCESS_DELETE on the file.
 *
 *      Context: Possibly any ring-3 thread on the system.
 */

ULONG DELETE_PRE(PSZ pszPath)
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
                if ((fsGranted & XWPACCESS_DELETE) != XWPACCESS_DELETE)
                    rc = ERROR_ACCESS_DENIED;
            }
        }
        else
            pThisContext = NULL;

        if (G_bLog == LOG_ACTIVE)
        {
            PEVENTBUF_FILENAME pBuf;

            if (pBuf = ctxtLogEvent(pThisContext,
                                    EVENT_DELETE_PRE,
                                    sizeof(EVENTBUF_FILENAME) + ulPathLen))
            {
                pBuf->rc = rc;
                pBuf->fsRequired = XWPACCESS_DELETE;
                pBuf->fsGranted = fsGranted;
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
 *@@ DELETE_POST:
 *      SES kernel hook for DELETE_POST.
 *
 *      As with all our hooks, this is stored in G_SecurityHooks
 *      (sec32_callbacks.c) force the OS/2 kernel to call us for
 *      each such event.
 *
 *      Context: Possibly any ring-3 thread on the system.
 */

VOID DELETE_POST(PSZ pszPath,
                 ULONG RC)
{
#if 0       // not needed presently
    if (    (G_pidShell)
         && (!DevHlp32_GetInfoSegs(&G_pGDT,
                                   &G_pLDT))
       )
    {
        if (G_bLog == LOG_ACTIVE)
        {
            PEVENTBUF_FILENAME pBuf;
            ULONG   ulPathLen = strlen(pszPath);

            if (pBuf = ctxtLogEvent(NULL,
                                    EVENT_DELETE_POST,
                                    sizeof(EVENTBUF_FILENAME) + ulPathLen))
            {
                pBuf->rc = RC;
                pBuf->ulPathLen = ulPathLen;
                memcpy(pBuf->szPath,
                       pszPath,
                       ulPathLen + 1);
            }
        }
    }
#endif
}


