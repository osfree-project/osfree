
/*
 *@@sourcefile callb_move.c:
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

#include "xwpsec32.sys\types.h"
#include "xwpsec32.sys\StackToFlat.h"
#include "xwpsec32.sys\devhlp32.h"

// #include "security\ring0api.h"

#include "xwpsec32.sys\xwpsec_callbacks.h"

/*
 *@@ MOVE_PRE:
 *      SES kernel hook for MOVE_PRE (move or rename).
 *
 *      As with all our hooks, this is stored in G_SecurityHooks
 *      (sec32_callbacks.c) force the OS/2 kernel to call us for
 *      each such event.
 *
 *      This is a "pre" event. Required privileges:
 *
 *      --  for source file: XWPACCESS_WRITE | XWPACCESS_DELETE.
 *
 *      --  for target file: XWPACCESS_WRITE.
 *
 *      Context: Possibly any ring-3 thread on the system.
 */

ULONG CallType MOVE_PRE(PSZ pszNewPath,
                        PSZ pszOldPath)
{
    APIRET  rc = NO_ERROR;

    return rc;
}

/*
 *@@ MOVE_POST:
 *      SES kernel hook for MOVE_POST.
 *
 *      As with all our hooks, this is stored in G_SecurityHooks
 *      (sec32_callbacks.c) force the OS/2 kernel to call us for
 *      each such event.
 *
 *      Context: Possibly any ring-3 thread on the system.
 */

VOID CallType MOVE_POST(PSZ pszNewPath,
                        PSZ pszOldPath,
                        ULONG RC)
{
}


