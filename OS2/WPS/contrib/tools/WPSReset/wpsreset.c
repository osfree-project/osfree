
/*
 * wpsreset.c:
 *      This resets the WPS using WinRestartWorkplace().
 *      This is a much cleaner and less brutal way than using
 *      PrfReset(), which was previously used.
 *
 *      PrfReset() was originally intended to change the user INI
 *      file (OS2.INI) and would then restart the WPS. But it also
 *      worked if you called it with the current user INI file.
 *      I suppose the WPS reacts to the PL_ALTERED msg which is
 *      broadcast to all msg queues on the system by terminating
 *      itself. The first instance of PMSHELL.EXE will then restart
 *      the WPS.
 *
 *      WPSRESET only works if you pass it the "-D" parameter on the
 *      command line in order to prevent accidental Desktop restarts.
 *      I don't remember what "-D" stands for. Maybe "dumb".
 */

/*
 *      Copyright (C) 1997-2005 Ulrich M”ller.
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

#include <os2.h>
#include <string.h>
#include <stdio.h>

#include "bldlevel.h"

BOOL32 APIENTRY WinRestartWorkplace(void);


int main(int argc, char *argv[])
{
    BOOL    fContinue = FALSE;

    if (argc == 2)
        // check for "-D" parameter
        if (strcmp(argv[1], "-D") == 0)
            fContinue = TRUE;

    if (fContinue)
    {
        WinRestartWorkplace();
    }
    else
    {
        printf("wpsreset V"BLDLEVEL_VERSION" ("__DATE__") (C) 1998-2005 Ulrich M”ller\n");
        printf("Restarts the Workplace Shell process.\n");
        printf("Usage: wpsreset -D\n");
    }

    return (0);
}

