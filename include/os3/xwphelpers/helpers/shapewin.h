
/*
 *@@sourcefile shapewin.h:
 *      header file for shapewin.h. See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #define INCL_GPIBITMAPS
 *@@include #include <os2.h>
 *@@include #include "helpers\shapewin.h"
 */

/*      Copyright (C) 1998-2000
 *                  Ulrich M”ller,
 *                  Akira Hatakeyama,
 *                  Takayuki Suwa.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#if __cplusplus
extern "C" {
#endif

#ifndef SHAPEWIN_HEADER_INCLUDED
    #define SHAPEWIN_HEADER_INCLUDED

    /*
     * Name of Control
     */

    #define WC_SHAPE_WINDOW         "ShapeWin"
    #define WC_SHAPE_REGION         "ShapeWinRegion"

    /*
     *@@ SHPCTLDATA:
     *      shape window control data for use with
     *      pCtlData of WinCreateWindow. This _must_
     *      be initialized.
     */

    typedef struct _SHPCTLDATA
    {
        SHORT   cx, cy ;       // size of bitmap
        HPS     hpsMask;       // HPS with bitmap selected into
        HPS     hpsDraw;       // HPS used for drawing the bitmap;
    } SHPCTLDATA, *PSHPCTLDATA;

    /*
     * Window Messages Specific for Shape Window
     */

    #define SHAPEWIN_MSG_UPDATE (WM_USER + 1)
        /* SHAPEWIN_MSG_UPDATE          Update Drawing Image                    */
        /*      MP1 PRECTL              Update Region, NULL for entire region   */
        /*      MP2 NULL                not used                                */

    /*
     *@@ SHAPEFRAME:
     *      common input/output structure for shape frame
     *      windows below.
     */

    typedef struct _SHAPEFRAME
    {
        HDC             hdc;
        HPS             hps;
        HBITMAP         hbm;

        HAB             hab;
        HWND            hwndShapeFrame;
        HWND            hwndShape;
        PFNWP           pfnFrame;
        BITMAPINFOHEADER2   bmi;
        SHPCTLDATA        shpctrl;

        POINTL          ptlLowerLeft;       // added (V1.0)
    } SHAPEFRAME, *PSHAPEFRAME;

    BOOL shpLoadBitmap(HAB hab,
                       PSZ pszBitmapFile,
                       HMODULE hmodResource,
                       ULONG idResource,
                       PSHAPEFRAME psb);

    VOID shpFreeBitmap(PSHAPEFRAME psb);

    BOOL shp2RegisterClasses(HAB hab);

    BOOL shpCreateWindows(PSHAPEFRAME psb);

#endif

#if __cplusplus
}
#endif

