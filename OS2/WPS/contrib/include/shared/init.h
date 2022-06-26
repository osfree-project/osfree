
/*
 *@@sourcefile init.h:
 *      header file for init.c. See remarks there.
 *
 *@@include #include <os2.h>
 *@@include #include "shared\init.h"
 */

/*
 *      Copyright (C) 1997-2003 Ulrich M”ller.
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

#ifndef INIT_HEADER_INCLUDED
    #define INIT_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Startup logging
     *
     ********************************************************************/

    void initLog(const char* pcszFormat,
                 ...);

    /* ******************************************************************
     *
     *   Panic flags
     *
     ********************************************************************/

    ULONG initQueryPanicFlags(VOID);

        // flags set by the "panic" dialog if "Shift"
        // was pressed during startup.
        // Per default, this field is set to zero,
        // but if the user disables something in the
        // "Panic" dialog, this may have any of the
        // following flags:
        // -- SUF_SKIPBOOTLOGO: skip boot logo
        // -- SUF_SKIPXFLDSTARTUP: skip XFldStartup processing
        // -- SUF_SKIPQUICKOPEN: skip "quick open" folder processing
        #ifndef __NOBOOTLOGO__
            #define SUF_SKIPBOOTLOGO            0x0001  // skip boot logo
        #endif
        #ifndef __NOXWPSTARTUP__
            #define SUF_SKIPXFLDSTARTUP         0x0002  // skip XFldStartup processing
        #endif
        #ifndef __NOQUICKOPEN__
            #define SUF_SKIPQUICKOPEN           0x0004  // skip "quick open" folder processing
        #endif

    /* ******************************************************************
     *
     *   XWorkplace initialization
     *
     ********************************************************************/

    BOOL initMain(VOID);

    BOOL initRepairDesktopIfBroken(VOID);

    VOID initDesktopPopulated(VOID);

#endif
