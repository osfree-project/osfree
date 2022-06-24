
/*
 *@@sourcefile filedlg.h:
 *      header file for filedlg.c (file operations).
 *
 *      This file is ALL new with V0.9.9.
 *
 *@@include #define INCL_WINSTDFILE
 *@@include #include <os2.h>
 *@@include #include "filesys\filedlg.h"
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

#ifndef FILEDLG_HEADER_INCLUDED
    #define FILEDLG_HEADER_INCLUDED

    HWND APIENTRY fdlgFileDlg(HWND hwndOwner,
                              PCSZ pcszStartupDir,
                              PFILEDLG pfd);

#endif


