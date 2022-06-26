
/*
 *@@sourcefile sec32_data.c:
 *      global variables for the data segment.
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

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>

// #include "security\ring0api.h"

#include "xwpsec32.sys\types.h"

#include "xwpsec32.sys\xwpsec_callbacks.h"

int                     _dllentry = 0;

extern ULONG            G_pidShell = 0;
                    // while this is null, the daemon is not running
                    // -> local security disabled

extern CHAR             G_szScratchBuf[1000] = "";
        // generic temporary buffer for composing strings etc.

// extern EVENTBUFUNION    G_EventBuf = {0};
        // union with the various event buffers

extern USHORT G_rcUnknownContext = NO_ERROR;

