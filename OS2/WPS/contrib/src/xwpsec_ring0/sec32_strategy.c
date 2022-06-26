
/*
 *@@sourcefile sec32_strategy.c:
 *      implements the PDD's strategy routing table.
 *
 *      See strat_init_base.c for an introduction to the driver
 *      structure in general.
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
 *      Based on the MWDD32.SYS example sources,
 *      Copyright (C) 1995, 1996, 1997  Matthieu Willm (willm@ibm.net).
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
#define INCL_DOS
#define INCL_NOPMAPI
#include <os2.h>
// #include <secure.h>

#include "xwpsec32.sys\types.h"
#include "xwpsec32.sys\devhlp32.h"
#include "xwpsec32.sys\reqpkt32.h"

#include "xwpsec32.sys\xwpsec_callbacks.h"

/*
 *@@ sec32_invalid_command:
 *      stub "bad command" routine for strategy request
 *      packets that we don't support.
 *
 *      This is called from sec32_strategy() since it's
 *      stored in driver_routing_table below.
 *
 *      Context: Possibly any ring-3 thread on the system.
 */

int sec32_invalid_command (PTR16 reqpkt)
{
    return STDON + STERR + ERROR_I24_BAD_COMMAND;
}

/*
 *@@ driver_routing_table:
 *      this holds the function table which
 *      sec32_strategy uses to call the various
 *      strategy command handlers.
 */

static int (*driver_routing_table[32])() =
{
    sec32_invalid_command,  // 0 = Init
    sec32_invalid_command,  // 1 = Media Check
    sec32_invalid_command,  // 2 = Build BPB
    sec32_invalid_command,  // 3 = Reserved
    sec32_invalid_command,  // 4 = Read
    sec32_invalid_command,  // 5 = Non-Destruct Read NoWait
    sec32_invalid_command,  // 6 = Input Status
    sec32_invalid_command,  // 7 = Input Flush
    sec32_invalid_command,  // 8 = Write
    sec32_invalid_command,  // 9 = Write w/Verify
    sec32_invalid_command,  // A = Output Status
    sec32_invalid_command,  // B = Output Flush
    sec32_invalid_command,  // C = Reserved
    sec32_open,             // D = Device Open (R/M)
    sec32_close,            // E = Device Close (R/M)
    sec32_invalid_command,  // F = Removable Media (R/M)
    sec32_ioctl,            // 10 = Generic Ioctl
    sec32_invalid_command,  // 11 = Reset Media
    sec32_invalid_command,  // 12 = Get Logical Drive Map
    sec32_invalid_command,  // 13 = Set Logical Drive Map
    sec32_invalid_command,  // 14 = DeInstall
    sec32_invalid_command,  // 15 =
    sec32_invalid_command,  // 16 = Get # Partitions
    sec32_invalid_command,  // 17 = Get Unit map
    sec32_invalid_command,  // 18 = No caching read
    sec32_invalid_command,  // 19 = No caching write
    sec32_invalid_command,  // 1A = No caching write w/Verify
    sec32_init_base,        // 1B = Initialize
    sec32_shutdown,         // 1C = Shutdown
    sec32_invalid_command,  // 1D = Get DCS/VCS
    sec32_invalid_command,  // 1E = ???
    sec32_init_complete     // 1F = Init Complete
};


/*
 *@@ sec32_strategy:
 *      this is the 32-bit strategy entry point which
 *      gets called from the 16-bit sec32_stub_strategy()
 *      in sec32_start.asm.
 *
 *      We end up here for every request packet that comes
 *      in from the kernel and call our respective
 *      implementation, if available.
 *
 *      Context: Possibly any ring-3 thread on the system.
 */

int DRV32ENTRY sec32_strategy(PTR16 reqpkt, int index)
{
    if (index < sizeof(driver_routing_table) / sizeof(*driver_routing_table))
        // valid command received: call handler function:
        return driver_routing_table[index](reqpkt);

    return STDON + STERR + ERROR_I24_INVALID_PARAMETER;
}

