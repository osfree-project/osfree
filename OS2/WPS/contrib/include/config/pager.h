
/*
 *@@sourcefile pager.h:
 *      header file for pager.c (XPager XWPScreen interface).
 *
 *      This file is ALL new with V0.9.3.
 *
 *@@added V0.9.3 (2000-04-08) [umoeller]
 *@@include #include <os2.h>
 *@@include #include "shared\notebook.h"
 *@@include #include "config\pager.h"
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

#ifndef CONFIG_HEADER_INCLUDED
    #define CONFIG_HEADER_INCLUDED

#ifndef __NOPAGER__

    /* ******************************************************************
     *
     *   XPager (XWPScreen) notebook functions (notebook.c)
     *
     ********************************************************************/

    #ifdef NOTEBOOK_HEADER_INCLUDED
        ULONG pgmiInsertPagerPages(WPObject *somSelf,
                                   HWND hwndDlg,
                                   HMODULE savehmod);
    #else
        #error "shared\notebook.h needs to be included before including pager.h".
    #endif

#endif // __NOPAGER__

#endif
