
/*
 * ximgview.h:
 *      header file for ximgview.c.
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

/* ******************************************************************
 *
 *   Constants
 *
 ********************************************************************/

#define     WC_MY_CLIENT_CLASS  "MyClientClass"

#define     INIAPP              "XWorkplace::ximgview"
#define     INIKEY_MAINWINPOS   "MainWinPos"
#define     INIKEY_LASTDIR      "LastDir"
#define     INIKEY_SETTINGS     "Settings"

#define     WM_DONELOADINGBMP   WM_USER

/* ******************************************************************
 *
 *   Menu item ID's
 *
 ********************************************************************/

#define IDM_FILE                    100
#define IDMI_FILE_NEW               101
#define IDMI_FILE_OPEN              102
#define IDMI_FILE_SAVE              103
#define IDMI_FILE_SAVEAS            104
#define IDMI_FILE_EXIT              109

#define IDM_VIEW                    300
#define IDMI_VIEW_SIZEORIG          301

#define IDM_OPTIONS                 400
#define IDMI_OPT_RESIZEAFTERLOAD    401
#define IDMI_OPT_CONSTRAIN2SCREEEN  402
#define IDMI_OPT_SCALE2WINSIZE      403


