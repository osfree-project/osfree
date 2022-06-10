
/*
 *@@sourcefile shapewin.c:
 *      contains helper functions for "shaped" PM windows which
 *      can be non-rectangular in shape and have transparency.
 *
 *      This file is new with V0.85.
 *
 *      Usage: All PM programs.
 *
 *      Function prefixes:
 *      --  shp*    shape window functions
 *
 *      This file consists of two parts:
 *
 *  *** Part 1 ***
 *
 *  The code in Part 1 implements the "Shape" window. This is used
 *  to display a bitmap on the screen with transparency.
 *
 *  Really, it is not the "Shape" window which displays the bitmap.
 *  Since PM can only handle rectangular windows, as many subwindows
 *  as required are created, which are only managed by the (invisible)
 *  "Shape" window.
 *
 *  See shp_fnwpShapeMgr for a window hierarchy.
 *
 *  If you are not using the frame window functions in Part 2, the
 *  shape window class must be registered like this (the declarations
 *  are in shapewin.h):
 *
 +      WinRegisterClass(
 +                     hab,                // anchor block handle
 +                     SHAPE_WINDOW_CLASS, // class name (shapewin.h)
 +                     shp_fnwpShapeMgr,    // window procedure (in this file)
 +                     0L,                 // default window style
 +                     sizeof(PVOID)) ;    // class data size
 *
 *  You can then create shape windows using WinCreateWindow():
 +      hwndShape = WinCreateWindow(
 +                     hwndParent,      // see remark (2)
 +                     SHAPE_WINDOW_CLASS, // class name (shapewin.h)
 +                     pszName,         // doesn't matter
 +                     flStyle,         // can be 0
 +                     x, y, cx, cy,    // can all be 0; shape window
 +                                      // size depends on bitmap
 +                     hwndOwner,       // see remark (2)
 +                     hwndInsertBehind, // should be HWND_TOP
 +                     id,
 +                     pCtlData,        // *SHPCTLDATA, see remark (1)
 +                     pPresParams);    // NULL
 *
 *  Remarks:
 *
 *  1)  Shape windows _require_ a SHPCTLDATA structure for input with
 *      the pCtlData parameter of WinCreateWindow. This structure
 *      must contain the following:
 +          SHORT   cx, cy ;       // bitmap size
 +          HPS     hpsMask;       // HPS with bitmap selected into
 +          HPS     hpsDraw;       // HPS used for drawing the bitmap
 *
 *      Note that the bitmap in hpsMask determines the drawing mask
 *      of the bitmap, while the bitmap in hpsDraw determines the
 *      bitmap to be drawn. You can use the same HPS (and thus
 *      the same bitmap) for both, but you can also use different
 *      ones.
 *
 *      You _must_ select the bitmap(s) to be displayed into the HPS's,
 *      using GpiSetBitmap, or nothing will work. shpLoadBitmap
 *      in Part 2 of this file offers you a one-shot function for
 *      loading a bitmap either from resources or a PM 1.3 *.BMP
 *      file.
 *
 *      Transparency is determined according to the most lower-left
 *      pixel of the bitmap in hpsMask. That is, all hpsDraw bitmap
 *      pixels which have the same color as hpsMask pixel (0, 0)
 *      will be made transparent.
 *
 *      During window creation, the shape window analyzes the bitmap
 *      which was selected into hpsMask and creates lots of rectangular
 *      PM windows as neccessary. This is dependent of the
 *      "transparency" pixels of the bitmap.
 *
 *      There is no limit for the size of the bitmap (in the HPS's).
 *      But you should be very careful with large bitmaps, because
 *      this can block the system forever. Shape windows are real
 *      CPU hogs. The number of PM subwindows created depends on the
 *      size of the bitmap and on the transparency. The calculations
 *      are line-based:
 *      --  each line is examined, and a PM window is created for
 *          each line of the input bitmap;
 *      --  if this line contains "transparency" pixels, more PM
 *          windows are created accordingly for that line.
 *      --  However, if several vertically adjacent subwindows have
 *          the same left and right coordinates, they are combined
 *          into one window.
 *
 *      As a result, the more single transparent pixels you have,
 *      the more windows need to be created. The more rectangular
 *      transparent areas you have, the less windows need to be
 *      created.
 *
 *  2)  The "Shape" window requires both a parent and an owner window.
 *      As always, the parent window determines the visibility, while
 *      the owner window requires a more abstract relationship. With
 *      shape windows, the owner window is important because all the
 *      input messages (for mouse and keyboard input) are forwarded
 *      to the owner.
 *
 *      So it's best to set the parent to HWND_DESKTOP and specify
 *      a frame window for the owner. If you don't already have a
 *      frame window in your application, you can create an invisible
 *      frame window just for message handling. That's what the shp*
 *      functions in Part 2 of this file do (Akira Hatakeyama calls
 *      this the "Voodoo" feature).
 *
 *      Never use the shape window itself as a top-level window,
 *      but one of your own windows instead, which own the shape
 *      window.
 *
 *      Note that the keyboard and mouse input messages which are
 *      forwarded to the owner contain mouse coordinates relative
 *      to the rectangular shape subwindow which actually received
 *      the message, not relative to the frame window. So you better
 *      determine the real mouse position using WinQueryPointerPos().
 *
 *  Note that the size of the shape window is defined once upon
 *  creation and cannot be changed later because this would conflict
 *  with all the subwindows which were created. For the same reason,
 *  you cannot alter the bitmap of the shape window after creation.
 *  You must create a second shape bitmap for that.
 *  Thus all the size parameters in WinSetWindowPos are swallowed and
 *  ignored.
 *
 *  *** Part 2 ***
 *
 *  This implements a proper invisible frame window for displaying
 *  shaped bitmap windows directly on the desktop.
 *  See shpCreateWindows for a working example.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\shapewin.h"
 */

/*
 *  Credits:
 *      1)  Functions for shaped windows.
 *          This is based on the ShapeWin library 1.01 (SHAPEWIN.ZIP at
 *          Hobbes), (C) 1998, 1999 Software Research Associates, Inc.
 *          This has been written by Akira Hatakeyama (akira@sra.co.jp).
 *          Updated by Takayuki Suwa (jjsuwa@ibm.net).
 *          The original was placed under the GPL.
 *
 *          Changes made by Ulrich M”ller (February 1999):
 *          --  some speedup (marked with *UM)
 *          --  renamed and commented out almost all of the functions
 *              and structures to make their use more lucid.
 *
 *      2)  Functions for easily creating shape frame windows from a
 *          given HBITMAP. This has been written by me, after getting
 *          some inspiration from the sample programs in SHAPEWIN.ZIP.
 *
 *      Copyright (C) 1998-2000
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
 *
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINDIALOGS
#define INCL_WININPUT
#define INCL_WINPOINTERS
#define INCL_WINSYS
#define INCL_WINRECTANGLES
#define INCL_WINTRACKRECT

#define INCL_GPILOGCOLORTABLE
#define INCL_GPIPRIMITIVES
#define INCL_GPIBITMAPS
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers/gpih.h"
#include "helpers/shapewin.h"

#pragma hdrstop

/*
 *@@category: Helpers\PM helpers\Window classes\Shaped windows (transparency)
 *      see shapewin.c.
 */

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

BOOL    G_ShapeRegisteredRegion = FALSE;

/* ******************************************************************
 *
 *   Part 1: Shape window functions
 *
 ********************************************************************/

// forward declarations for structures, because
// the structures are referenced from each other
typedef struct  _SHPWINCTLDATA  *PSHPWINCTLDATA;
typedef struct  _SHPREGION  *PSHPREGION;

/*
 * SHPWINCTLDATA:
 *      internal shape window control data.
 *      This is the shape-window-internal structure
 *      to allow the shape window to manage itself.
 *      This is created in WM_CREATE with the data
 *      passed in the mp1 and mp2 parameters
 *      (CREATESTRUCT and SHPCTLDATA).
 *      This is stored in a shape window's window words.
 */

typedef struct _SHPWINCTLDATA
{
    // common window parameters
    // (copied from CREATESTRUCT in WM_CREATE)
    HWND        hwndShape ;    // main shape window handle
    HWND        hwndParent;    // shape window's parent (should be HWND_DESKTOP)
    HWND        hwndOwner ;    // shape window's owner, to whom we'll forward all
                               // all the input messages
    USHORT      id        ;    // window ID
    SHORT       x         ;    // x - horz. position
    SHORT       y         ;    // y - vert. position
    SHORT       cx        ;    // horz. size
    SHORT       cy        ;    // vert. size
    PSZ         pszText   ;    // window text

    // additional window parameters
    HPS         hpsDraw;       // the input HPS with the bitmap
    ULONG       nRegion;       // number of "regions"
    PSHPREGION  aRegion;       // array of "regions"

} SHPWINCTLDATA;

/*
 * SHPREGION:
 *      this is the structure for shape window
 *      "regions". Note that has nothing to do
 *      with GPI regions, but is simply a structure
 *      for each rectangular area in the shape
 *      window's bitmap which can be displayed
 *      as a rectangular PM window.
 */

typedef struct _SHPREGION
{
    PSHPWINCTLDATA   pCtrl;   // link to parent
    HWND        hwnd ;         // drawing window
    RECTL       rect ;         // rectangle of sub-bitmap
} SHPREGION;

/*
 * SHPSPAN:
 *      this is a temporary structure used during
 *      the creation of SHPREGION's from the
 *      input bitmap.
 */

typedef struct _SHPSPAN {
    ULONG   nSize   ;
    ULONG   nUsed   ;
    RECTL   aSpan[1];
} SHPSPAN, *PSHPSPAN;

// the following defines how many SHPSPAN structures
// will be initially created and afterwards for span
// expansions by spanExpand
#define SPAN_ALLOC_STEP    1024

/*
 * spanFree:
 *
 */

STATIC void spanFree(PSHPSPAN pSpan)
{
    if (pSpan != NULL)
        free(pSpan);
}

/*
 * spanCreate:
 *      this creates SPAN_ALLOC_STEP SHPSPAN structures.
 */

STATIC PSHPSPAN spanCreate(void)
{
    PSHPSPAN pSpan;
    int     len  ;

    len = sizeof(SHPSPAN) + sizeof(RECTL) * SPAN_ALLOC_STEP;

    if ((pSpan = (PSHPSPAN)malloc(len)) == NULL)
        return NULL;

    memset(pSpan, 0, len);
    pSpan->nSize = SPAN_ALLOC_STEP;
    pSpan->nUsed = 0   ;
    return pSpan;
}

/*
 * spanExpand:
 *      this expands the memory used for the SHPSPAN
 *      structures if more spans need to be appended.
 *      Another SPAN_ALLOC_STEP SHPSPAN structures are created.
 */

STATIC PSHPSPAN spanExpand(PSHPSPAN pOld)
{
    PSHPSPAN pNew;
    int     len ;

    // TRACE("spanExpand\n");

    len = sizeof(SHPSPAN) + sizeof(RECTL) * (pOld->nSize + SPAN_ALLOC_STEP);

    if ((pNew = (PSHPSPAN)malloc(len)) == NULL)
        return NULL;

    memset(pNew, 0, len);
    pNew->nSize = pOld->nSize + SPAN_ALLOC_STEP;
    pNew->nUsed = pOld->nUsed;
    memcpy(pNew->aSpan, pOld->aSpan, sizeof(RECTL) * pOld->nUsed);
    spanFree(pOld);

    return pNew;
}

/*
 * spanAppend:
 *      this adds a new SHPSPAN to the list of spans.
 *
 */

STATIC PSHPSPAN spanAppend(PSHPSPAN pSpan,
                           int y,       // bottom y; top y = y+1
                           int x1,      // left x
                           int x2)      // right x
{
    int     i;
    PRECTL  p;
    PSHPSPAN pNew;

    // TRACE("spanAppend %d (%d %d)\n", y, x1, x2);

    /*
     * check if continuous one
     */

    for (i = 0; i < pSpan->nUsed; i++) {
        p = &pSpan->aSpan[i];
        if (p->yTop == y && p->xLeft == x1 && p->xRight == x2)
        {
            p->yTop += 1;
            return pSpan;      // merged
        }
    }

    /*
     * if not enough space, expand
     */

    if ((pSpan->nUsed + 1) >= pSpan->nSize)
    {
        if ((pNew = spanExpand(pSpan)) == NULL)
            return NULL;

        pSpan = pNew;
    }

    /*
     * append a rectangle
     */

    pSpan->aSpan[pSpan->nUsed].yTop = y + 1;
    pSpan->aSpan[pSpan->nUsed].yBottom = y ;
    pSpan->aSpan[pSpan->nUsed].xLeft   = x1;
    pSpan->aSpan[pSpan->nUsed].xRight  = x2;
    pSpan->nUsed += 1;

    return pSpan;
}

#ifdef  SHPDEBUG

    /*
     * ptisin:
     *
     */

    static BOOL ptisin(PSHPSPAN pSpan, int x, int y)
    {
        int     i;

        for (i = 0; i < pSpan->nUsed; i++)
        {
            if (y >= pSpan->aSpan[i].yTop) {
                continue;
            }
            if (y < pSpan->aSpan[i].yBottom) {
                continue;
            }
            if (x < pSpan->aSpan[i].xLeft) {
                continue;
            }
            if (x >= pSpan->aSpan[i].xRight) {
                continue;
            }
            return TRUE;
        }
        return FALSE;
    }

    /*
     * dumpSpan:
     *
     */

    static void dumpSpan(PSHPSPAN pSpan)
    {
        int     i, maxx, maxy, x, y;

        TRACE("dumpSpan %d\n", pSpan->nUsed);

        maxx = maxy = 0;

        for (i = 0; i < pSpan->nUsed; i++)
        {
            if (pSpan->aSpan[i].yTop > maxy)
            {
                maxy = pSpan->aSpan[i].yTop;
            }
            if (pSpan->aSpan[i].xRight > maxx)
            {
                maxx = pSpan->aSpan[i].xRight;
            }
        }

        for (y = maxy - 1; y >= 0; y--)
        {
            printf("%03d : ", y);
            for (x = 0; x < maxx; x++)
            {
                if (ptisin(pSpan, x, y))
                    printf("#");
                else
                    printf("_");
            }
            printf("\n");
        }
    }

#endif  // SHPDEBUG

/*
 * shprgnDraw:
 *      this gets called upon receiving WM_PAINT in
 *      shp_fnwpShapeRegion. We simply draw the
 *      subrectangle from the shape window's bitmap
 *      which corresponds to our region rectangle.
 */

STATIC void shprgnDraw(HPS hps, PSHPREGION pRgn)
{
    POINTL      apt[3];

    apt[0].x = 0;
    apt[0].y = 0;
    apt[1].x = (pRgn->rect.xRight - pRgn->rect.xLeft);
    apt[1].y = (pRgn->rect.yTop - pRgn->rect.yBottom);
    apt[2].x = pRgn->rect.xLeft  ;
    apt[2].y = pRgn->rect.yBottom;
    GpiBitBlt(hps, pRgn->pCtrl->hpsDraw, 3, apt, ROP_SRCCOPY, 0);
}

/*
 *@@ shp_fnwpShapeRegion:
 *      this is the window procedure for each of the
 *      shape window's rectangular subwindows.
 *
 *      See shp_fnwpShapeMgr for a window hierarchy.
 */

STATIC MRESULT EXPENTRY shp_fnwpShapeRegion(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PSHPREGION   pRgn;
    HPS         hps ;

    pRgn = (PSHPREGION)WinQueryWindowPtr(hwnd, 0);

    switch (msg)
    {
        /*
         * WM_CREATE:
         *      store the SHPREGION pointer in our
         *      window words.
         */

        case WM_CREATE:
            // TRACE("WM_CREATE\n");
            pRgn = (PSHPREGION)PVOIDFROMMP(mp1);
            WinSetWindowPtr(hwnd, 0, (PVOID) pRgn);
        return (MRESULT)FALSE;

        /*
         * WM_PAINT:
         *      draw our subrectangle of the main bitmap.
         */

        case WM_PAINT:
            // TRACE("WM_PAINT (%d %d) (%d %d)\n", pRgn->rect.yBottom, pRgn->rect.xLeft, pRgn->rect.yTop, pRgn->rect.xRight);
            hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);
            shprgnDraw(hps, pRgn);
            WinEndPaint(hps)  ;
        return (MRESULT)0;

        case WM_MOUSEMOVE    :
        case WM_BUTTON1DOWN  :
        case WM_BUTTON1UP    :
        case WM_BUTTON1CLICK :
        case WM_BUTTON1DBLCLK:
        case WM_BUTTON2DOWN  :
        case WM_BUTTON2UP    :
        case WM_BUTTON2CLICK :
        case WM_BUTTON2DBLCLK:
        case WM_BUTTON3DOWN  :
        case WM_BUTTON3UP    :
        case WM_BUTTON3CLICK :
        case WM_BUTTON3DBLCLK:
        case WM_CHAR         :
        case WM_VIOCHAR      :
        case WM_BEGINDRAG    :
        case WM_ENDDRAG      :
        case WM_SINGLESELECT :
        case WM_OPEN         :
        case WM_CONTEXTMENU  :
        case WM_CONTEXTHELP  :
        case WM_TEXTEDIT     :
        case WM_BEGINSELECT  :
        case WM_ENDSELECT    :
            // forward all these to the shape window
            return WinSendMsg(pRgn->pCtrl->hwndShape, msg, mp1, mp2);
    }

    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/*
 * shpmgrWMAdjustWindowPos:
 *      adjust the region windows' positions, visibility etc.
 *      by going thru the whole region windows list and
 *      repositioning all the windows.
 *      This gets called upon receiving WM_ADJUSTWINDOWPOS
 *      in shp_fnwpShapeMgr.
 */

STATIC int shpmgrWMAdjustWindowPos(PSHPWINCTLDATA pCtrl, PSWP pSwp)
{
    int         i   ;
    PSHPREGION   pRgn;
    ULONG       fl  ;
    // HPS         hps ;

    PSWP pswpArray;

    // TRACE("shpmgrWMAdjustWindowPos - %d, %08x\n", pCtrl->nRegion, pSwp->fl);

    if (pCtrl->nRegion == 0 || pCtrl->aRegion == NULL)
    {
        // TRACE("ShpDrawRegion - no region to open\n");
        return -1;
    }

    if ((fl = pSwp->fl) & SWP_MOVE)
    {
        pCtrl->x = pSwp->x;
        pCtrl->y = pSwp->y;
    }

    pswpArray = (PSWP) malloc(sizeof(SWP) * pCtrl->nRegion);

    // go thru all the "regions" and adjust their sizes
    for (i = 0, pRgn = pCtrl->aRegion;
         i < pCtrl->nRegion;
         i++, pRgn++)
    {
        pswpArray[i].fl = fl;
        pswpArray[i].cy = (pRgn->rect.yTop - pRgn->rect.yBottom);
        pswpArray[i].cx = (pRgn->rect.xRight - pRgn->rect.xLeft);
        pswpArray[i].y = (pCtrl->y + pRgn->rect.yBottom);
        pswpArray[i].x = (pCtrl->x + pRgn->rect.xLeft);
        pswpArray[i].hwndInsertBehind = pSwp->hwndInsertBehind;
        pswpArray[i].hwnd = pRgn->hwnd;
    }

    // set all window positions at once
    WinSetMultWindowPos(WinQueryAnchorBlock(pCtrl->hwndShape),
                        &pswpArray[0],
                        pCtrl->nRegion);
    free((void*) pswpArray);

    // calling WinInvalidateRect is not neccessary,
    // because the windows' update regions are already
    // properly set by WinSetMultWindowPos (*UM)

    /* for (i = 0, pRgn = pCtrl->aRegion;
         i < pCtrl->nRegion;
         i++, pRgn++)
    {
        if (fl & (SWP_MOVE | SWP_ZORDER)) {
            WinInvalidateRect(pRgn->hwnd, NULL, FALSE);
        }
    } */

    return 0;
}

/*
 * shpmgrUpdateRegions:
 *      this gets called by shp_fnwpShapeMgr upon receiving
 *      SHAPEWIN_MSG_UPDATE.
 */

STATIC int shpmgrUpdateRegions(PSHPWINCTLDATA pCtrl, PRECTL pRect)
{
    RECTL       rect, intern;
    int         i   ;
    PSHPREGION   pRgn;
    HAB         hab ;
    // HPS         hps ;

    // TRACE("shpmgrUpdateRegions\n");

    hab = WinQueryAnchorBlock(pCtrl->hwndShape);

    if (pRect == NULL) {
        rect.xLeft   = 0       ;
        rect.yBottom = 0       ;
        rect.xRight = pCtrl->cx;
        rect.yTop   = pCtrl->cy;
        pRect = &rect;
    }

    for (i = 0, pRgn = pCtrl->aRegion; i < pCtrl->nRegion; i++, pRgn++)
    {
        if (WinIntersectRect(hab, &intern, pRect, &pRgn->rect) == FALSE) {
            continue;
        } else {
            WinInvalidateRect(pRgn->hwnd, NULL, FALSE);
        }
    }

    return 0;
}

/*
 * shpmgrParseBitmap:
 *      this gets called from shpmgrWMCreate_Bitmap2Regions when the
 *      shape region windows need to be created.
 *      This function does the actual bitmap analysis and
 *      creates a large number of SHPSPAN structures according
 *      to the bitmaps, which can later be turned into
 *      PM windows.
 *      This happens during the processing of WM_CREATE in
 *      shp_fnwpShapeMgr.
 *      CPU usage: relatively low. Maybe 5% of the whole creation
 *      procedure.
 *
 *@@changed V0.9.1 (99-12-03): fixed memory leak
 */

STATIC PSHPSPAN shpmgrParseBitmap(HPS hps, PBITMAPINFOHEADER2 bmih2)
{
    int             blen, hlen;
    PUCHAR          buf;
    PBITMAPINFO2    pbmi;
    PSHPSPAN        pSpan, pNew;
    int             x, y, k;
    LONG            first, color;
    BOOL            inspan;
    int             left;

    // TRACE("shpmgrParseBitmap\n");

    hlen = sizeof(BITMAPINFO2) + sizeof(RGB) * 256;
    blen = ((bmih2->cBitCount * bmih2->cx + 31) / 32) * bmih2->cPlanes * 4;

    pbmi = (PBITMAPINFO2)malloc(hlen);
    buf = (PUCHAR)malloc(blen);

    if (pbmi == NULL || buf == NULL)
    {
        // TRACE("shpmgrParseBitmap - failed to alloc %d %d\n", hlen, blen);
        if (pbmi)
            free(pbmi);
        if (buf)
            free(buf);
        return NULL;
    }
    memcpy(pbmi, bmih2, sizeof(BITMAPINFOHEADER2));

    if ((pSpan = spanCreate()) == NULL)
    {
        // TRACE("shpmgrParseBitmap - failed to make\n");
        free(pbmi);
        free(buf);
        return NULL;
    }

    first = -1;

    for (y = 0; y < bmih2->cy; y++)
    {

        // TRACE("shpmgrParseBitmap - scan line %d\n", y); fflush(stdout);

        GpiQueryBitmapBits(hps, y, 1, (PBYTE)buf, pbmi);

        for (x = 0, inspan = FALSE; x < bmih2->cx; x++)
        {
            k = x * 3;
            color = ((buf[k] << 16) | (buf[k+1] << 8) | buf[k+2]);

            if (first < 0)
            {
                // first iteration: get the very first color (that
                // is the pixel at (0, 0)), with which we will
                // compare the others to determine the mask
                first = color;
            }

            if (inspan == FALSE && color != first)
            {
                inspan = TRUE;
                left = x;
            }
            else if (inspan == TRUE && color == first)
            {
                // transparent color found:
                // create new span
                inspan = FALSE;
                if ((pNew = spanAppend(pSpan, y, left, x)) != NULL)
                {
                    pSpan = pNew;
                }
                else
                {
                    // TRACE("shpmgrParseBitmap - failed to extend\n");
                    break;
                }
            }
        }

        if (inspan == TRUE)
        {
            if ((pNew = spanAppend(pSpan, y, left, x)) != NULL)
                pSpan = pNew;
            else
                // TRACE("shpmgrParseBitmap - failed to extend\n");
                break;
        }
    }

#ifdef  SHPDEBUG
    dumpSpan(pSpan);
#endif

    // fixed these memory leaks V0.9.1 (99-12-03)
    if (pbmi)
        free(pbmi);
    if (buf)
        free(buf);

    return pSpan;
}

/*
 * shpmgrWMCreate_Bitmap2Regions:
 *      this gets called from shpmgrWMCreate (WM_CREATE) to have a
 *      drawing region created from the bitmap which has been
 *      selected into the given HPS.
 *
 *      First step in WM_CREATE.
 */

STATIC int shpmgrWMCreate_Bitmap2Regions(PSHPWINCTLDATA pCtrl,  // in: shape control data
                                         HPS hpsMask)      // in: HPS with selected bitmap
{
    HAB             hab;
    HDC             hdc;
    HPS             hps;
    SIZEL           siz;
    HBITMAP         hbm,
                    hbmPrevious;
    BITMAPINFOHEADER2   bmi;
    POINTL          apt[3];
    PSHPSPAN        pSpan;
    PSHPREGION      pRegn;
    int             i;

    // TRACE("shpmgrWMCreate_Bitmap2Regions\n");

    hab = WinQueryAnchorBlock(pCtrl->hwndShape);

    /*
     * Create Memory DC & HPS
     */

    hdc = DevOpenDC(hab, OD_MEMORY, "*", 0, NULL, NULLHANDLE);
    siz.cx = siz.cy = 0;
    hps = GpiCreatePS(hab, hdc, &siz,
            PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);

    /*
     * Create Bitmap and relate to memory PS
     */

    memset(&bmi, 0, sizeof(bmi));

    bmi.cbFix = sizeof(BITMAPINFOHEADER2);
    bmi.cx = pCtrl->cx;
    bmi.cy = pCtrl->cy;
    bmi.cPlanes       = 1 ;
    bmi.cBitCount     = 24;
    bmi.ulCompression = 0 ;
    bmi.cclrUsed      = 0 ;
    bmi.cclrImportant = 0 ;

    hbm = GpiCreateBitmap(hps, &bmi, 0, NULL, NULL);

    hbmPrevious = GpiSetBitmap(hps, hbm);

    /*
     * Copy in Mask Pattern
     */

    // TRACE("shpmgrWMCreate_Bitmap2Regions - copyin %d x %d mask pattern\n", bmi.cx, bmi.cy);

    apt[0].x = 0;          // Target
    apt[0].y = 0;
    apt[1].x = bmi.cx;
    apt[1].y = bmi.cy;
    apt[2].x = 0;          // Source
    apt[2].y = 0;

    if (GpiBitBlt(hps, pCtrl->hpsDraw, 3, apt, ROP_SRCCOPY, 0) == GPI_ERROR) {
        // TRACE("MakeRect - BitBlt Failed %08x, hdc %08x, hps %08x, hbm %08x\n",
                // WinGetLastError(hab), hdc, hps, hbm);
    }

    /*
     * Parse Mask Pattern
     */

    // call shpmgrParseBitmap; this does the actual creation of the
    // "spans" by comparing all bitmap pixels to the (0, 0)
    // pixel
    if ((pSpan = shpmgrParseBitmap(hps, &bmi)) != NULL)
    {
        if ((pRegn = (PSHPREGION) malloc(sizeof(SHPREGION) * pSpan->nUsed)) == NULL)
        {
        // TRACE("shpmgrWMCreate_Bitmap2Regions - failed to alloc\n");
        }
        else
        {
            // mask created successfully:
            pCtrl->nRegion = pSpan->nUsed;
            pCtrl->aRegion = pRegn;

            for (i = 0; i < pSpan->nUsed; i++)
            {
                pRegn[i].pCtrl    = pCtrl;
                pRegn[i].hwnd = NULLHANDLE;
                pRegn[i].rect.xLeft   = pSpan->aSpan[i].xLeft  ;
                pRegn[i].rect.xRight  = pSpan->aSpan[i].xRight ;
                pRegn[i].rect.yTop    = pSpan->aSpan[i].yTop   ;
                pRegn[i].rect.yBottom = pSpan->aSpan[i].yBottom;
            }
        }
        spanFree(pSpan);
    }

    /*
     * dispose local resources
     */

    GpiSetBitmap(hps, hbmPrevious); // (*UM)
    GpiDeleteBitmap(hbm);
    GpiDestroyPS(hps);
    DevCloseDC(hdc);

    return 0;
}

/*
 * shpmgrWMCreate_Regions2Windows:
 *      this gets called from shpmgrWMCreate (WM_CREATE) to convert
 *      all the "regions" that have been created by shpmgrWMCreate_Bitmap2Regions
 *      in pCtrl to individual rectangular PM windows.
 *
 *      CPU usage: extremely high, because all the dull PM functions
 *      take so damn long. Calculating the "regions" isn't really
 *      difficult, but it's the PM windows which cause the CPU hog.
 *
 *      Second step in WM_CREATE.
 */

STATIC int shpmgrWMCreate_Regions2Windows(PSHPWINCTLDATA pCtrl)
{
    int         i   ;
    PSHPREGION   pRgn;
    ULONG       flStyle;

    // TRACE("shpmgrWMCreate_Regions2Windows %d regions\n", pCtrl->nRegion);

    // check if any "regions" exist at all
    if (pCtrl->nRegion == 0 || pCtrl->aRegion == NULL)
    {
        // TRACE("shpmgrWMCreate_Regions2Windows - no region to open\n");
        return -1;
    }

    if (G_ShapeRegisteredRegion == FALSE)
    {
        // first call: register "shape region" class
        WinRegisterClass(WinQueryAnchorBlock(pCtrl->hwndShape),
                         WC_SHAPE_REGION,
                         shp_fnwpShapeRegion,
                         CS_PARENTCLIP
                             // use parent's clipping region instead
                             // of the one of the region window
                            | CS_CLIPSIBLINGS
                             // don't allow the subwindows to paint
                             // over siblings (ie. windows with the
                             // same parent); this is neccessary if
                             // HWND_DESKTOP is the parent
                            | CS_SYNCPAINT,
                             // paint immediately
                         sizeof(PVOID));
        G_ShapeRegisteredRegion = TRUE;
    }

    flStyle = 0;

    // now go thru the "regions" list
    for (i = 0, pRgn = pCtrl->aRegion;
         i < pCtrl->nRegion;
         i++, pRgn++)
    {
        // and create a window for each "region"
        pRgn->hwnd = WinCreateWindow(
                pCtrl->hwndParent,      // copy parent window from shape window
                            // changed (*UM)
                        // HWND_DESKTOP,           // Parent Window
                WC_SHAPE_REGION,        // window class
                NULL,                   // window text
                flStyle,                // window style
                (pCtrl->x + pRgn->rect.xLeft),              // x
                (pCtrl->y + pRgn->rect.yBottom),            // y
                (pRgn->rect.xRight - pRgn->rect.xLeft),     // cx
                (pRgn->rect.yTop - pRgn->rect.yBottom),     // cy
                pCtrl->hwndOwner,       // Owner Window
                HWND_TOP,               // Z-Order
                i,                      // Window ID
                pRgn,                   // Control Data
                NULL);                  // Pres. Param.
    }

    return 0;
}

/*
 * shpmgrFreeRegion:
 *      cleanup during WM_DESTROY.
 */

STATIC int shpmgrFreeRegion(PSHPWINCTLDATA pCtrl)
{
    int         i   ;
    PSHPREGION   pRgn;

    for (i = 0, pRgn = pCtrl->aRegion;
         i < pCtrl->nRegion;
         i++, pRgn++)
        WinDestroyWindow(pRgn->hwnd);

    free(pCtrl->aRegion);

    return 0;
}

/*
 * shpmgrWMCreate:
 *      this initializes the "shape window".
 *      This gets called upon receiving WM_CREATE in shp_fnwpShapeMgr.
 *      The procedure is as follows:
 *      1)  the bitmap in pData->hpsDraw is analyzed pixel by
 *          pixel to create a number of "regions" from it
 *          (which are not GPI regions, but an array of SHPREGION
 *          structures. At least one of these structures is
 *          created for each line. This is done by calling
 *          shpmgrWMCreate_Bitmap2Regions.
 *      2)  We then call shpmgrWMCreate_Regions2Windows to actually create
 *          PM windows from all these structures.
 */

STATIC PSHPWINCTLDATA shpmgrWMCreate(HWND hwnd, // in: shape window
                                     PCREATESTRUCT pWin,    // in: create struct of WM_CREATE
                                     PSHPCTLDATA pData)       // in: SHPCTLDATA struct (WM_CREATE mp1)
{
    PSHPWINCTLDATA   pCtrl;

    if (pData->hpsDraw == NULLHANDLE || pData->hpsMask == NULLHANDLE) {
        return NULL;
    }

    // create new PSHPWINCTLDATA structure
    if ((pCtrl = (PSHPWINCTLDATA) malloc(sizeof(SHPWINCTLDATA))) == NULL) {
        return NULL;
    }

    /*
     * Setup Common Window Parameters
     */

    pCtrl->hwndShape  = hwnd;
    pCtrl->hwndParent = pWin->hwndParent;
    pCtrl->hwndOwner  = pWin->hwndOwner ;
    pCtrl->id         = pWin->id;
    pCtrl->x          = pWin->x ;
    pCtrl->y          = pWin->y ;
    pCtrl->cx         = pWin->cx;
    pCtrl->cy         = pWin->cy;

    /*
     * Setup Image Window's Control Data
     */

    pCtrl->cx = pData->cx;
    pCtrl->cy = pData->cy;
    pCtrl->hpsDraw = pData->hpsDraw;

    // now create "regions" from bitmap;
    // this will store the "regions" in pCtrl
    shpmgrWMCreate_Bitmap2Regions(pCtrl, pData->hpsMask);

    // now create as many rectangular PM
    // windows as we have "regions" in pCtrl
    shpmgrWMCreate_Regions2Windows(pCtrl);

    return pCtrl;
}

/*
 * shpmgrWMDestroy:
 *      this cleans up the shape's resources.
 *      Gets called upon receiving WM_DESTROY in
 *      shp_fnwpShapeMgr.
 */

STATIC void shpmgrWMDestroy(PSHPWINCTLDATA pCtrl)
{
    if (pCtrl == NULL)
        return;

    shpmgrFreeRegion(pCtrl);
    free(pCtrl);
}

/*
 *@@ shp_fnwpShapeMgr:
 *      this is the window procedure for the "shape window manager".
 *      Register this procedure with WinRegisterWindowClass.
 *
 *      This does the transformation of the bitmap into many
 *      rectangular subwindows upon WM_CREATE, each of which
 *      will use shp_fnwpShapeRegion as its window proc.
 *
 *      There should be the following window hierarchy (the
 *      lines signify ownership):
 *
 +      Your owner window
 +         |
 +         +-- owns "shape manager" window (this window proc)
 +               |
 +               +-- lots of "region" windows (shp_fnwpShapeRegion)
 +               +-- ...
 *
 *      The "region" windows are owned by the "shape manager" window,
 *      but have the same parent as the "shape manager" window.
 *      Normally, this should be set to HWND_DESKTOP when creating the
 *      "shape" window.
 *
 *      This window procedure forwards the following messages
 *      to its owner:
 *      --  WM_MOUSEMOVE
 *      --  all WM_BUTTONxxx messages
 *      --  WM_CHAR
 *      --  WM_VIOCHAR
 *      --  WM_BEGINDRAG
 *      --  WM_ENDDRAG
 *      --  WM_SINGLESELECT
 *      --  WM_OPEN
 *      --  WM_CONTEXTMENU
 *      --  WM_CONTEXTHELP
 *      --  WM_TEXTEDIT
 *      --  WM_BEGINSELECT
 *      --  WM_ENDSELECT
 */

STATIC MRESULT EXPENTRY shp_fnwpShapeMgr(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PSHPWINCTLDATA   pCtrl;
    PSWP        pswp ;
    // SHORT       sx; // , sy;

    pCtrl = (PSHPWINCTLDATA) WinQueryWindowPtr(hwnd, 0);

    switch (msg)
    {

        /*
         * WM_CREATE:
         *      shape window is being created.
         *      Parameters:
         *          PSHPCTLDATA mp1
         *          PCREATESTRUCT mp2
         *      Shaped windows must have a SHPCTLDATA
         *      structure in mp1.
         */

        case WM_CREATE:
            // TRACE("WM_CREATE\n");
            pCtrl = shpmgrWMCreate(hwnd,
                                  (PCREATESTRUCT)PVOIDFROMMP(mp2),
                                  (PSHPCTLDATA)PVOIDFROMMP(mp1));

            if (pCtrl == NULL)
                return (MRESULT)TRUE;

            // store control data in window words
            WinSetWindowPtr(hwnd, 0, (PVOID) pCtrl);
            return (MRESULT)FALSE;

        /*
         * WM_DESTROY:
         *      clean up.
         */

        case WM_DESTROY:
            // TRACE("WM_DESTORY\n");
            shpmgrWMDestroy(pCtrl);
            return (MRESULT)0;

        /*
         * WM_ADJUSTWINDOWPOS:
         *      this needs to be manipulated to
         *      adjust the positions of all the
         *      subwindows instead.
         */

        case WM_ADJUSTWINDOWPOS:
            // TRACE("WM_ADJUSTWINDOWPOS\n");
            pswp = (PSWP) PVOIDFROMMP(mp1);
            // DUMPSWP(pswp);

            // enforce the size which we were given
            // in the beginning, because we cannot
            // change the size later
            pswp->cx = pCtrl->cx;
            pswp->cy = pCtrl->cy;

            // adjust the sub-windows
            shpmgrWMAdjustWindowPos(pCtrl, pswp);

            // never show ourselves
            pswp->fl &= ~SWP_SHOW;
            return (MRESULT)0;

        /*
         * SHAPEWIN_MSG_UPDATE:
         *
         */

        case SHAPEWIN_MSG_UPDATE:
            shpmgrUpdateRegions(pCtrl, (PRECTL) PVOIDFROMMP(mp1));
            return (MRESULT)0;

        /*
         * WM_QUERYDLGCODE:
         *
         */

        case WM_QUERYDLGCODE:
            // TRACE("WM_QUERYDLGCODE\n");
            return (MRESULT)DLGC_STATIC;

        /*
         * WM_PAINT:
         *      we just swallow this message because
         *      there's no "shape window" to be painted;
         *      instead, all the region windows get
         *      their own WM_PAINT message
         */

        case WM_PAINT:
        {
            // TRACE("WM_PAINT\n");
            /* HPS hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);

            POINTL apt[3];
            RECTL rclWin;
            WinQueryWindowRect(hwnd, &rclWin);

            // lower left of target
            apt[0].x = 0;
            apt[0].y = 0;
            // upper right of target
            apt[1].x = rclWin.xRight;
            apt[1].y = rclWin.yTop;
            // lower left of source
            apt[2].x = 0;
            apt[2].y = 0;
            GpiBitBlt(hps,
                      pCtrl->hpsDraw,
                      3,
                      apt,
                      ROP_SRCCOPY,
                      0); */

            // delete update region to stop further
            // WM_PAINTs for this window;
            // this is faster than WiNBeginPaint
            WinValidateRect(hwnd, NULL, FALSE);
            // WinEndPaint(hps);
            return (MRESULT)0;
        }

        /*
         * all input-related messages:
         *      forward these to the owner
         */

        case WM_MOUSEMOVE    :
        case WM_BUTTON1DOWN  :
        case WM_BUTTON1UP    :
        case WM_BUTTON1CLICK :
        case WM_BUTTON1DBLCLK:
        case WM_BUTTON2DOWN  :
        case WM_BUTTON2UP    :
        case WM_BUTTON2CLICK :
        case WM_BUTTON2DBLCLK:
        case WM_BUTTON3DOWN  :
        case WM_BUTTON3UP    :
        case WM_BUTTON3CLICK :
        case WM_BUTTON3DBLCLK:
        case WM_CHAR         :
        case WM_VIOCHAR      :
        case WM_BEGINDRAG    :
        case WM_ENDDRAG      :
        case WM_SINGLESELECT :
        case WM_OPEN         :
        case WM_CONTEXTMENU  :
        case WM_CONTEXTHELP  :
        case WM_TEXTEDIT     :
        case WM_BEGINSELECT  :
        case WM_ENDSELECT    :
            return WinSendMsg(pCtrl->hwndOwner, msg, mp1, mp2);

    } // end switch (msg)
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/* ******************************************************************
 *
 *   Part 2: Shape frame functions
 *
 ********************************************************************/

MRESULT EXPENTRY fnwpShapeFrame(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/*
 *@@ shpLoadBitmap:
 *      this creates a memory device context/presentation space
 *      and loads and selects a bitmap into it.
 *
 *      If pszBitmap == NULL, the bitmap is loaded from the
 *      specified resources, otherwise we attempt to load the
 *      bitmap file specified in pszBitmap (using gpihLoadBitmapFile).
 *
 *      Output in the given SHAPEFRAME structure:
 *      --  hab:                as passed to this func
 *      --  hdc:                device context handle
 *      --  hps:                presentation space handle
 *      --  hbm:                bitmap handle
 *      --  bmi:                BITMAPINFOHEADER of hbmp
 *      --  ptlLowerLeft:       lower left corner is set so that
 *                              bitmap is centered on screen
 *
 *      All other fields are neither read nor written to.
 *      Returns TRUE if everything went OK.
 *
 *      You can call this function directly before calling
 *      shpCreateWindows.
 *
 *      Pass the SHAPEFRAME structure to shpFreeBitmap to clean up.
 *
 *@@changed V0.9.0 [umoeller]: added default window positioning
 *@@changed V0.9.0 [umoeller]: removed GpiSetBitmap here
 *@@changed V0.9.9 (2001-03-18) [lafaix]: brc was not set
 */

BOOL shpLoadBitmap(HAB hab, // in: anchor block
                   PSZ pszBitmapFile, // in: OS/2 1.3 bmp file or NULL
                   HMODULE hmodResource, // in: module handle from where the
                            // resources should be loaded if
                            // pszBitmapFile == NULL. This can be 0 for
                            // the current EXE
                   ULONG idResource, // in: bitmap resource ID in that module
                   PSHAPEFRAME psb) // out: bitmap info
{
    SIZEL       siz;
    BOOL        brc = FALSE;

    psb->hab = hab;

    if (psb->hdc = DevOpenDC(hab, OD_MEMORY, "*", 0, NULL, NULLHANDLE))
    {
        siz.cx = siz.cy = 0;
        if (psb->hps = GpiCreatePS(hab,
                                   psb->hdc,
                                   &siz,
                                   PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC))
        {
            if (pszBitmapFile)
            {
                // load bitmap file
                if (gpihLoadBitmapFile(&psb->hbm,
                                       psb->hps,
                                       pszBitmapFile))
                    psb->hbm = NULLHANDLE;
            }
            else
                // load resource bitmap
                psb->hbm = GpiLoadBitmap(psb->hps,
                                         hmodResource,
                                         idResource,
                                         0, 0);
            if (psb->hbm)
            {
                SWP     swpScreen;
                // store bitmap info in structure
                psb->bmi.cbFix = sizeof(psb->bmi);
                GpiQueryBitmapInfoHeader(psb->hbm, &psb->bmi);

                // set ptlLowerLeft so that the bitmap
                // is centered on the screen
                WinQueryWindowPos(HWND_DESKTOP, &swpScreen);
                psb->ptlLowerLeft.x = (swpScreen.cx - psb->bmi.cx) / 2;
                psb->ptlLowerLeft.y = (swpScreen.cy - psb->bmi.cy) / 2;

                brc = TRUE;
            }
        }
    }
    return brc;
}

/*
 *@@ shpFreeBitmap:
 *      this cleans up resources allocated by shpLoadBitmap:
 *      delete the bitmap, destroy the HPS and HDC.
 *
 *@@changed V0.9.3 (2000-04-11) [umoeller]: fixed major resource leak; the bitmap was never freed
 */

VOID shpFreeBitmap(PSHAPEFRAME psb)
{
    if (psb->hbm != NULLHANDLE)
    {
        // unset bitmap in HPS; it cannot be deleted otherwise...
        GpiSetBitmap(psb->hps, NULLHANDLE);     // V0.9.3 (2000-04-11) [umoeller]
        GpiDeleteBitmap(psb->hbm);
        psb->hbm = NULLHANDLE;
    }
    if (psb->hps != NULLHANDLE)
    {
        GpiDestroyPS(psb->hps);
        psb->hps = NULLHANDLE;
    }
    if (psb->hdc != NULLHANDLE)
    {
        DevCloseDC(psb->hdc);
        psb->hdc = NULLHANDLE;
    }
}

/*
 *@@ shpCreateWindows:
 *      this is a one-shot function for creating a
 *      shaped window from a bitmap.
 *
 *      Required fields in SHAPEFRAME for input (those
 *      are all properly set by shpLoadBitmap):
 *      --  hab:                anchor block
 *      --  hps:                presentation space handle
 *      --  hbm:                bitmap handle
 *      --  bmi:                BITMAPINFOHEADER of hbmp
 *      --  ptlLowerLeft:       lower left corner of where
 *                              to position the shape window (new with V0.9.0).
 *
 *      All other fields are ignored for input and only
 *      set for output (and for the shape window later).
 *
 *      You can use shpLoadBitmap to have all the fields
 *      initialized, which will also set ptlLowerLeft
 *      so that the shape window gets centered on the
 *      Desktop.
 *
 *      Otherwise you must initialize the above fields for
 *      yourself.
 *
 *      Obviously, this function uses the same HPS (and
 *      thus bitmap) for both transparency and drawing.
 *      If you wish to use two different bitmaps (one for
 *      transparency, one for drawing), you cannot use
 *      this function.
 *
 *      Note that the windows are now (V0.9.0) also positioned
 *      on the screen.
 *
 *      Output in that structure:
 *      --  hwndShapeFrame:     invisible rectangular frame
 *                              window for the shape window
 *                              (fnwpShapeFrame, created here)
 *      --  hwndShape:          "shape" window
 *                              (shp_fnwpShapeMgr, created here)
 *      --  pfnFrame:           original window procedure of
 *                              WC_FRAME (hwndShapeFrame),
 *                              which is subclassed here
 *                              using fnwpShapeFrame
 *      --  shpctrl:            control data for hwndShape
 *
 *      Returns TRUE if everything went OK.
 *
 *      <B>Example</B> for creating a shaped window and
 *      centering it on the screen:
 +          SHAPEFRAME sb;
 +          if (shpLoadBitmap(hab...,
 +                              szBitmapFile...,
 +                              0, 0,
 +                              &sb))
 +          {
 +              // create shape (transparent) windows
 +              if (shpCreateWindows(habQuickThread, &sb))
 +              {
 *                  ...
 +              }
 +          }
 +          ...
 +          // cleanup
 +              shpFreeBitmap(&sb);
 +              WinDestroyWindow(sb.hwndShapeFrame) ;
 +              WinDestroyWindow(sb.hwndShape);
 *
 *@@changed V0.9.0 [umoeller]: removed hab from function prototype
 *@@changed V0.9.0 [umoeller]: added window positioning
 *@@changed V0.9.0 [umoeller]: added GpiSetBitmap here (always forget that)
 */

BOOL shpCreateWindows(PSHAPEFRAME psb)
{
    BOOL        brc = FALSE;

    if (psb->hbm)
    {
        // bitmap seems to be valid:
        FRAMECDATA  fcd;

        GpiSetBitmap(psb->hps, psb->hbm);

        memset(&fcd, 0, sizeof(fcd));
        fcd.cb = sizeof(fcd);

        // create invisible frame
        psb->hwndShapeFrame = WinCreateWindow(
                HWND_DESKTOP,           // Parent window handle
                WC_FRAME,               // Frame Window Class
                "XWorkplaceLogoShape",  // title
                0,                      // Window Style
                0, 0, 0, 0,             // Position & size
                NULLHANDLE,             // Owner Window
                HWND_TOP,               // Z-Order
                0,                      // Window ID
                &fcd,                   // Control Data
                NULL);                  // Presentation Parameter

        if (psb->hwndShapeFrame)
        {
            // store the SHAPEFRAME structure in the frame's window words
            // so that the subclassed frame can access the data
            WinSetWindowULong(psb->hwndShapeFrame, QWL_USER, (ULONG)psb);

            // subclass the frame window; store the original wnd proc in
            // the SHAPEFRAME structure
            psb->pfnFrame = WinSubclassWindow(psb->hwndShapeFrame, fnwpShapeFrame);

            if ((psb->hwndShapeFrame) && (psb->hps))
            {
                // OK, no errors so far

                // register shape window class; if this is already
                // registered, this won't hurt
                WinRegisterClass(psb->hab,
                                 WC_SHAPE_WINDOW,
                                 shp_fnwpShapeMgr,
                                 0L,        // class style flags
                                 sizeof(PVOID));

                // create shape manager window
                psb->shpctrl.cx = psb->bmi.cx;
                psb->shpctrl.cy = psb->bmi.cy;
                psb->shpctrl.hpsDraw = psb->hps;
                psb->shpctrl.hpsMask = psb->hps;

                // create the "shape" window
                psb->hwndShape = WinCreateWindow(
                                  HWND_DESKTOP,           // Parent Window
                                  WC_SHAPE_WINDOW,        // Window Class
                                  NULL,                   // Window Text
                                  0,                      // Window Style;
                                                          // no clip siblings!
                                  0, 0, 0, 0,             // Pos & Size
                                  psb->hwndShapeFrame,    // Owner Window
                                  HWND_TOP,               // Z-Order
                                  0,                      // Window ID
                                  &psb->shpctrl,          // Control Data
                                  NULL);                  // Pres. Param.

                if (psb->hwndShape)
                {
                    // no error: only then return TRUE
                    brc = TRUE;

                    // and position the windows
                    WinSetWindowPos(psb->hwndShapeFrame, NULLHANDLE,
                            psb->ptlLowerLeft.x, psb->ptlLowerLeft.y,
                            psb->bmi.cx, psb->bmi.cy,
                            (SWP_MOVE | SWP_SIZE | SWP_HIDE));
                    WinSetWindowPos(psb->hwndShape, NULLHANDLE,
                            psb->ptlLowerLeft.x, psb->ptlLowerLeft.y,
                            psb->bmi.cx, psb->bmi.cy,
                            (SWP_MOVE | SWP_SIZE | SWP_SHOW));
                }
            }
        }
    }

    return brc;
}

/*
 *@@ fnwpShapeFrame:
 *      this is the window proc for subclassing the shape frame window
 *      (shpCreateWindows above).
 *      The shaped window proc (shp_fnwpShapeMgr) keeps forwarding messages
 *      to its owner (which is the frame here), so we better handle
 *      those messages.
 */

MRESULT EXPENTRY fnwpShapeFrame(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc;

    // get the SHAPEFRAME structure from the window words,
    // which has been stored there by shpCreateWindows
    PSHAPEFRAME psb = (PSHAPEFRAME)WinQueryWindowULong(hwnd, QWL_USER);

    // _Pmpf(("frame %08x %08x %08x", msg, mp1, mp2));

    switch (msg)
    {

        /*
         * WM_ADJUSTWINDOWPOS:
         *      forward the data we get here to the shape
         *      window (which will in turn reposition all
         *      the region windows);
         *      afterwards, always delete SHOW and set HIDE
         *      for ourselves
         */

        case WM_ADJUSTWINDOWPOS: {
            PSWP pswp = (PSWP) PVOIDFROMMP(mp1);
            POINTL ptl;
            WinQueryPointerPos(HWND_DESKTOP, &ptl);
            WinSetWindowPos(psb->hwndShape,
                            pswp->hwndInsertBehind,
                            // use current mouse position instead
                            // of the original ones
                            ptl.x,
                            ptl.y,
                            // pswp->x,
                            // pswp->y,
                            pswp->cx,
                            pswp->cy,
                            pswp->fl);
            pswp->fl &= ~SWP_SHOW;
            pswp->fl |=  SWP_HIDE;
            mrc = (*psb->pfnFrame)(hwnd, msg, mp1, mp2);
        break; }

        /* the shape window forwards these messages to us:
            WM_MOUSEMOVE
            WM_BUTTON1DOWN
            WM_BUTTON1UP
            WM_BUTTON1CLICK
            WM_BUTTON1DBLCLK
            WM_BUTTON2DOWN
            WM_BUTTON2UP
            WM_BUTTON2CLICK
            WM_BUTTON2DBLCLK
            WM_BUTTON3DOWN
            WM_BUTTON3UP
            WM_BUTTON3CLICK
            WM_BUTTON3DBLCLK
            WM_CHAR
            WM_VIOCHAR
            WM_BEGINDRAG
            WM_ENDDRAG
            WM_SINGLESELECT
            WM_OPEN
            WM_CONTEXTMENU
            WM_CONTEXTHELP
            WM_TEXTEDIT
            WM_BEGINSELECT
            WM_ENDSELECT */

        /*
         * WM_SINGLESELECT:
         *      if we are being clicked upon, bring the
         *      shape window to top instead
         */

        case WM_SINGLESELECT:
            WinSetWindowPos(psb->hwndShape, HWND_TOP, 0, 0, 0, 0, SWP_ZORDER);
            mrc = 0;
        break;

        /*
         * WM_BEGINDRAG:
         *      if we are being dragged with MB2, forward
         *      this to the shape window
         */

        case WM_BEGINDRAG:
            WinSendMsg(psb->hwndShapeFrame, WM_TRACKFRAME,
                MPFROMSHORT(TF_MOVE /* | TF_SETPOINTERPOS*/ ), NULL);
            mrc = 0;
        break;

        default:
            mrc = (*psb->pfnFrame)(hwnd, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ shp2RegisterClasses:
 *
 *@@added V0.9.3 (2000-05-03) [umoeller]
 */

BOOL shp2RegisterClasses(HAB hab)
{
    // register shape window class; if this is already
    // registered, this won't hurt
    return(WinRegisterClass(hab,
                            WC_SHAPE_WINDOW,
                            shp_fnwpShapeMgr,
                            0L,        // class style flags
                            sizeof(PVOID)));
}


