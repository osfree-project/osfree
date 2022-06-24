
/*
 *@@sourcefile util_misc.c:
 *      various utility functions.
 *
 *      See strat_init_base.c for an introduction.
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
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

static PVOID        G_pTemp = NULL;

extern XWPSECSTATUS G_R0Status;     // in strat_ioctl.c

/* ******************************************************************
 *
 *   Utilities
 *
 ********************************************************************/

/*
 *@@ utilAllocFixed:
 *      allocates a chunk of fixed kernel memory.
 *
 *      This calls the VMAlloc devhlp with the given cb parameter
 *      and the VMDHA_FIXED flag only. Apparently this devhlp goes
 *      into the kernel's own heap manager under certain conditions,
 *      most importantly, if the requested size is small. It thus
 *      appears to be safe to use this in malloc()-like fashion with
 *      many small objects.
 *
 *@@added V1.0.1 (2003-01-10) [umoeller]
 */

PVOID utilAllocFixed(ULONG cb)
{
    if (DevHlp32_VMAlloc(cb,                // Length
                         VMDHA_NOPHYSADDR,  // PhysAddr == -1
                         VMDHA_FIXED,       // Flags
                         (PVOID*)&G_pTemp))
        return NULL;

    G_R0Status.cbAllocated += cb;
    ++G_R0Status.cAllocations;

    return G_pTemp;
}

/*
 *@@ utilFreeFixed:
 *      frees memory allocated by utilAllocFixed.
 *
 *@@added V1.0.1 (2003-01-10) [umoeller]
 */

VOID utilFreeFixed(PVOID pv,
                   ULONG cb)
{
    DevHlp32_VMFree(pv);

    G_R0Status.cbAllocated -= cb;
    ++G_R0Status.cFrees;
}


