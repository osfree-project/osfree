
/*
 *@@sourcefile network.c:
 *
 *
 *      Function prefix for this file:
 *      --  net*
 *
 *      This file is ALL new with V0.9.16.
 *
 *@@header "filesys\trash.h"
 *@@added V0.9.16 (2001-10-19) [umoeller]
 */

/*
 *      Copyright (C) 2001-2003 Ulrich M”ller.
 *
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#pragma strings(readonly)

/*
 *  Suggested #include order:
 *  1)  os2.h
 *  2)  C library headers
 *  3)  setup.h (code generation and debugging options)
 *  4)  headers in helpers\
 *  5)  at least one SOM implementation header (*.ih)
 *  6)  dlgids.h, headers in shared\ (as needed)
 *  7)  headers in filesys\ (as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSPROCESS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h
#include <io.h>
#include <ctype.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers

// SOM headers which don't crash with prec. header files
#include "xwpnetwork.ih"
#include "xwpnetsrv.ih"

// XWorkplace implementation headers

// other SOM headers
#pragma hdrstop

/*
 *@@ netPopulateFirstTime:
 *      this gets called when XWPNetwork::wpPopulate gets
 *      called for the very first time only.
 */

BOOL netPopulateFirstTime(XWPTrashCan *somSelf,
                           ULONG ulFldrFlags)
{
    BOOL            brc = TRUE;     // fixed V0.9.7 (2001-01-17) [umoeller]

    return brc;
}


