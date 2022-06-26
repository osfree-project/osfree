
/*
 *@@sourcefile fhandles.h:
 *      header file for fhandles.c (file-system object handles).
 *
 *      This file is new with V0.9.5.
 *
 *@@include #include <os2.h>
 *@@include #include "filesys\fhandles.h"
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
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

#ifndef FHANDLES_HEADER_INCLUDED
    #define FHANDLES_HEADER_INCLUDED

    /* ******************************************************************
     *
     *  File-system handles interface
     *
     ********************************************************************/

    BOOL fhdlLoadHandles(VOID);

#endif


