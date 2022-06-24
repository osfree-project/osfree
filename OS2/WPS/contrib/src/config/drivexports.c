
/*
 *@@sourcefile drivers.c:
 *      implementation code for the "Drivers" page
 *      in the "OS/2 Kernel" object (XFldSystem).
 *
 *      This code was created with V0.9.0 in cfgsys.c and
 *      has been extracted to this file with V0.9.3. See cfgsys.c
 *      for introductory remarks.
 *
 *      Function prefix for this file:
 *      --  cfg*
 *
 *@@added V0.9.3 (2000-04-17) [umoeller]
 *@@header "config\cfgsys.h"
 */

/*
 *      Copyright (C) 1999-2003 Ulrich M”ller.
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
 *  7)  headers in implementation dirs (e.g. filesys\, as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <ctype.h>
#include <direct.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers

// SOM headers which don't crash with prec. header files
#include "xfobj.ih"

// XWorkplace implementation headers
#include "shared\common.h"              // the majestic XWorkplace include file

#include "config\drivdlgs.h"            // driver configuration dialogs

// other SOM headers
#pragma hdrstop                 // VAC++ keeps crashing otherwise

/*
 *@@ drvDisplayHelp:
 *      little helper for driver dialogs to display
 *      a help panel.
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

BOOL drvDisplayHelp(PVOID pvKernel,
                    const char *pcszHelpFile,
                    ULONG ulHelpPanel)
{
    if (!pcszHelpFile)
        pcszHelpFile = cmnQueryHelpLibrary();
    return _wpDisplayHelp((WPObject*)pvKernel,
                          ulHelpPanel,
                          (PSZ)pcszHelpFile);
}

/*
 *@@ drv_strtok:
 *      little helper for driver plugins since
 *      the subsystem library won't support
 *      strtok.
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

char* drv_strtok(char *string1, const char *string2)
{
    return strtok(string1, string2);
}

/*
 *@@ drv_memicmp:
 *      little helper for driver plugins since
 *      the subsystem library won't support
 *      memicmp.
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

int drv_memicmp(void *buf1, void *buf2, unsigned int cnt)
{
    return memicmp(buf1, buf2, cnt);
}

/*
 *@@ drv_sprintf:
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

int drv_sprintf(char *pBuf, const char *pcszFormat, ...)
{
    va_list     args;
    int         i;
    va_start(args, pcszFormat);
    i = vsprintf(pBuf, pcszFormat, args);
    va_end(args);

    return i;
}

/*
 *@@ drv_strstr:
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 */

char* drv_strstr(const char *string1, const char *string2)
{
    return strstr(string1, string2);
}
