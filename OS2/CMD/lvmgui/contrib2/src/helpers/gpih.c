
/*
 *@@sourcefile gpih.c:
 *      contains GPI (graphics) helper functions.
 *
 *      Usage: All PM programs.
 *
 *      Function prefixes (new with V0.81):
 *      --  gpih*   GPI helper functions
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\gpih.h"
 */

/*
 *      Copyright (C) 1997-2002 Ulrich Mîller.
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

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSPROFILE
#define INCL_DOSSEMAPHORES
#define INCL_DOSMODULEMGR
#define INCL_DOSRESOURCES
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINPOINTERS
#define INCL_WINSYS

#define INCL_GPIPRIMITIVES
#define INCL_GPIBITMAPS
#define INCL_GPILOGCOLORTABLE
#define INCL_GPILCIDS
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "setup.h"                      // code generation and debugging options

#ifdef WINH_STANDARDWRAPPERS
#undef WINH_STANDARDWRAPPERS
#endif
#include "helpers\dosh.h"
#include "helpers\gpih.h"
#include "helpers\winh.h"
#include "helpers\stringh.h"

#pragma hdrstop

// array for querying device capabilities (gpihQueryDisplayCaps)
LONG            DisplayCaps[CAPS_DEVICE_POLYSET_POINTS] = {0};
BOOL            G_fCapsQueried = FALSE;

/*
 *@@category: Helpers\PM helpers\GPI helpers
 *      See gpih.c.
 */

/*
 *@@category: Helpers\PM helpers\GPI helpers\Devices
 */

/*
 *@@gloss: GPI_rectangles GPI rectangles
 *      OS/2 PM (and GPI) uses two types of rectangles. This is rarely
 *      mentioned in the documentation, so a word is in order here.
 *
 *      In general, graphics operations involving device coordinates
 *      (such as regions, bit maps and bit blts, and window management)
 *      use inclusive-exclusive rectangles. In other words, with
 *      those rectangles, xRight - xLeft is the same as the width
 *      of the rectangle (and yTop - yBottom = height).
 *
 *      All other graphics operations, such as GPI functions that
 *      define paths, use inclusive-inclusive rectangles.
 *
 *      This can be a problem with mixing Win and Gpi functions. For
 *      example, WinQueryWindowRect returns an inclusive-exclusive
 *      rectangle (tested V0.9.7 (2000-12-20) [umoeller]).
 *
 *      WinFillRect expects an inclusive-exclusive rectangle, so it
 *      will work with a rectangle from WinQueryWindowRect directly.
 *
 *      By contrast, the GpiBox expects an inclusive-inclusive rectangle.
 */

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

STATIC HMTX     G_hmtxLCIDs = NULLHANDLE;

/* ******************************************************************
 *
 *   Rectangle helpers
 *
 ********************************************************************/

/*
 *@@ gpihIsPointInRect:
 *      like WinPtInRect, but doesn't need a HAB.
 *
 *      NOTE: as opposed to WinPtInRect, prcl is
 *      considered inclusive, that is, TRUE is
 *      returned even if x or y are exactly
 *      the same as prcl->xRight or prcl->yTop.
 *
 *@@added V0.9.9 (2001-02-28) [umoeller]
 */

BOOL gpihIsPointInRect(PRECTL prcl,
                       LONG x,
                       LONG y)
{
    if (prcl)
        return (    (x >= prcl->xLeft)
                 && (x <= prcl->xRight)
                 && (y >= prcl->yBottom)
                 && (y <= prcl->yTop)
               );

    return FALSE;
}

/*
 *@@ gpihInflateRect:
 *      Positive l will make the rectangle larger.
 *      Negative l will make the rectangle smaller.
 *
 *@@added V0.9.9 (2001-02-28) [umoeller]
 */

VOID gpihInflateRect(PRECTL prcl,
                     LONG l)
{
    if (prcl && l)
    {
        prcl->xLeft -= l;
        prcl->yBottom -= l;
        prcl->xRight += l;
        prcl->yTop += l;
    }
}

/*
 *@@ gpihCopyRectIncl:
 *      copies prclWin to prclGpi, making it
 *      inclusive-inclusive for use with GPI
 *      functions.
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 */

VOID gpihCopyRectIncl(PRECTL prclGpi,       // out: GPI rectangle
                      PRECTL prclWin)       // in: WIN rectangle
{
    prclGpi->xLeft = prclWin->xLeft;
    prclGpi->yBottom = prclWin->yBottom;
    prclGpi->xRight = prclWin->xRight - 1;
    prclGpi->yTop = prclWin->yTop - 1;
}

/* ******************************************************************
 *
 *   Device helpers
 *
 ********************************************************************/

/*
 *@@ gpihQueryDisplayCaps:
 *      this returns certain device capabilities of
 *      the display device. ulIndex must be one of
 *      the indices as described in DevQueryCaps.
 *
 *      This function will load all the device capabilities
 *      only once into a global array and re-use them afterwards.
 *
 *@@changed V0.9.16 (2001-12-18) [umoeller]: fixed multiple loads
 */

ULONG gpihQueryDisplayCaps(ULONG ulIndex)
{
    if (!G_fCapsQueried)
    {
        HPS hps = WinGetScreenPS(HWND_DESKTOP);
        HDC hdc = GpiQueryDevice(hps);
        DevQueryCaps(hdc, 0, CAPS_DEVICE_POLYSET_POINTS, &DisplayCaps[0]);
        G_fCapsQueried = TRUE;      // was missing V0.9.16 (2001-12-18) [umoeller]
    }

    return DisplayCaps[ulIndex];
}

/*
 *@@category: Helpers\PM helpers\GPI helpers\Colors
 */

/* ******************************************************************
 *
 *   Color helpers
 *
 ********************************************************************/

/*
 * HackColor:
 *
 */

STATIC VOID HackColor(PBYTE pb, double dFactor)
{
    ULONG ul = (ULONG)((double)(*pb) * dFactor);
    if (ul > 255)
        *pb = 255;
    else
        *pb = (BYTE)ul;
}

/*
 *@@ gpihManipulateRGB:
 *      this changes an RGB color value
 *      by multiplying each color component
 *      (red, green, blue) with dFactor.
 *
 *      Each color component is treated separately,
 *      so if overflows occur (because dFactor
 *      is > 1), this does not affect the other
 *      components.
 *
 *@@changed V0.9.11 (2001-04-25) [umoeller]: changed prototype to use a double now
 */

VOID gpihManipulateRGB(PLONG plColor,       // in/out: RGB color
                       double dFactor)      // in: factor (> 1 makes brigher, < 1 makes darker)
{
    PBYTE   pb = (PBYTE)plColor;

    // in memory, the bytes are blue, green, red, unused

    // blue
    ULONG   ul = (ULONG)(   (double)(*pb) * dFactor
                        );
    if (ul > 255)
        *pb = 255;
    else
        *pb = (BYTE)ul;

    // green
    ul = (ULONG)(   (double)(*(++pb)) * dFactor
                );
    if (ul > 255)
        *pb = 255;
    else
        *pb = (BYTE)ul;

    // red
    ul = (ULONG)(   (double)(*(++pb)) * dFactor
                );
    if (ul > 255)
        *pb = 255;
    else
        *pb = (BYTE)ul;
}

#define MEDIATE(a, b) (LONG)(a) + (((LONG)(b) - (LONG)(a)) / 2)

/*
 *@@ gpihMediumRGB:
 *      returns the arithmetic medium color between
 *      lcol1 and lcol2.
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 */

LONG gpihMediumRGB(LONG lcol1, LONG lcol2)
{
    return MAKE_RGB(
                     MEDIATE(GET_RED(lcol1),
                             GET_RED(lcol2)),
                     MEDIATE(GET_GREEN(lcol1),
                             GET_GREEN(lcol2)),
                     MEDIATE(GET_BLUE(lcol1),
                             GET_BLUE(lcol2))
                   );
}

/*
 *@@ gpihSwitchToRGB:
 *      this switches the given HPS into RGB mode. You should
 *      always use this if you are operating with RGB colors.
 *
 *      This is just a shortcut to calling
 *
 +          GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULL);
 *
 *@@changed V0.9.7 (2001-01-15) [umoeller]: turned macro into function to reduce fixups
 */

BOOL gpihSwitchToRGB(HPS hps)
{
    return GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULL);
}

/*
 *@@category: Helpers\PM helpers\GPI helpers\Drawing primitives
 */

/* ******************************************************************
 *
 *   Drawing primitives helpers
 *
 ********************************************************************/

/*
 *@@ gpihDrawRect:
 *      this draws a simple rectangle with the current
 *      color (use GpiSetColor before calling this function).
 *
 *      The specified rectangle is inclusive, that is, the top
 *      right corner specifies the top right pixel to be drawn
 *      (see @GPI_rectangles).
 *
 *      This sets the current position to the bottom left corner
 *      of prcl.
 *
 *@added V0.9.0
 */

VOID gpihDrawRect(HPS hps,      // in: presentation space for output
                  PRECTL prcl)  // in: rectangle to draw (inclusive)
{
    POINTL ptl1;

    ptl1.x = prcl->xLeft;
    ptl1.y = prcl->yBottom;
    GpiMove(hps, &ptl1);
    ptl1.y = prcl->yTop - 1;
    GpiLine(hps, &ptl1);
    ptl1.x = prcl->xRight - 1;
    GpiLine(hps, &ptl1);
    ptl1.y = prcl->yBottom;
    GpiLine(hps, &ptl1);
    ptl1.x = prcl->xLeft;
    GpiLine(hps, &ptl1);
}

/*
 *@@ gpihBox:
 *      this is a shortcurt to GpiBox, using the specified
 *      rectangle.
 *
 *      As opposed to WinFillRect, this works with memory
 *      (bitmap) PS's also.
 *
 *      The specified rectangle is inclusive, that is, the top
 *      right corner specifies the top right pixel to be drawn.
 *      This is different from WinFillRect (see @GPI_rectangles).
 *
 *      Changes to the HPS:
 *
 *      --  the current position is moved to the lower left
 *          corner of *prcl.
 *
 *@@changed V0.9.0 [umoeller]: renamed from gpihFillRect
 *@@changed V0.9.0 [umoeller]: modified function prototype to support lControl
 *@@changed V0.9.7 (2001-01-17) [umoeller]: removed lColor
 */

VOID gpihBox(HPS hps,              // in: presentation space for output
             LONG lControl,        // in: one of DRO_OUTLINE, DRO_FILL, DRO_OUTLINEFILL
             PRECTL prcl)          // in: rectangle to draw (inclusive)
{
    POINTL      ptl;

    ptl.x = prcl->xLeft;
    ptl.y = prcl->yBottom;
    GpiMove(hps, &ptl);
    ptl.x = prcl->xRight;
    ptl.y = prcl->yTop;
    GpiBox(hps,
           lControl,    // DRO_*
           &ptl,
           0, 0);       // no corner rounding
}

/*
 *@@ gpihMarker:
 *      this draws a quick marker (a filled
 *      rectangle) at the specified position.
 *      The rectangle will be drawn so that
 *      the specified point is in its center.
 *
 *      No PS data is changed.
 *
 *@@changed V0.9.7 (2001-01-17) [umoeller]: removed lColor
 */

VOID gpihMarker(HPS hps,
                LONG x,             // in: x-center of rectangle
                LONG y,             // in: y-center of rectangle
                ULONG ulWidth)      // in: rectangle width and height
{
    POINTL  ptlSave;
    RECTL   rclTemp;
    ULONG   ulWidth2 = ulWidth / 2;
    rclTemp.xLeft = x - ulWidth2;
    rclTemp.xRight = x + ulWidth2;
    rclTemp.yBottom = y - ulWidth2;
    rclTemp.yTop    = y + ulWidth2;

    GpiQueryCurrentPosition(hps, &ptlSave);
    gpihBox(hps,
            DRO_FILL,
            &rclTemp);
    GpiMove(hps, &ptlSave);
}

/*
 *@@ gpihThickBox:
 *      draws a box from the specified rectangle with the
 *      specified width.
 *
 *      The specified rectangle is inclusive, that is, the top
 *      right corner specifies the top right pixel to be drawn.
 *      This is different from WinFillRect
 *      (see @GPI_rectangles).
 *
 *      If usWidth > 1, the additional pixels will be drawn towards
 *      the _center_ of the rectangle. prcl thus always specifies
 *      the bottom left and top right pixels to be drawn.
 *
 *      This is different from using GpiSetLineWidth, with which
 *      I was unable to find out in which direction lines are
 *      extended.
 *
 *      This is similar to gpihDraw3DFrame, except that everything
 *      is painted in the current color.
 *
 *@@added V0.9.7 (2000-12-06) [umoeller]
 */

VOID gpihDrawThickFrame(HPS hps,            // in: presentation space for output
                        PRECTL prcl,        // in: rectangle to draw (inclusive)
                        ULONG ulWidth)      // in: line width (>= 1)
{
    ULONG ul = 0;
    for (;
         ul < ulWidth;
         ul++)
    {
        GpiMove(hps, (PPOINTL)prcl);
        GpiBox(hps,
               DRO_OUTLINE,
               (PPOINTL)&(prcl->xRight),
               0,
               0);

        // and one more to the outside
        prcl->xLeft++;
        prcl->yBottom++;
        prcl->xRight--;
        prcl->yTop--;
    }
}

/*
 *@@ gpihDraw3DFrame2:
 *      this draws a rectangle in 3D style with a given line width
 *      and the given colors.
 *
 *      The specified rectangle is inclusive, that is, the top
 *      right corner specifies the top right pixel to be drawn.
 *      This is different from WinFillRect
 *      (see @GPI_rectangles).
 *
 *      If usWidth > 1, the additional pixels will be drawn towards
 *      the _center_ of the rectangle. prcl thus always specifies
 *      the bottom left and top right pixels to be drawn.
 *
 *      Note: With V1.0.0, this now modifies prcl to be smaller towards
 *      the center of the rectangle by usWidth so you can call this several
 *      times with different colors.
 *
 *@@changed V0.9.0 [umoeller]: changed function prototype to have colors specified
 *@@changed V0.9.7 (2000-12-20) [umoeller]: now really using inclusive rectangle...
 *@@changed V1.0.0 (2002-08-24) [umoeller]: renamed, now modifying prcl on output
 */

VOID gpihDraw3DFrame2(HPS hps,              // in: presentation space for output
                      PRECTL prcl,          // in: rectangle (inclusive)
                      USHORT usWidth,       // in: line width (>= 1)
                      LONG lColorLeft,      // in: color to use for left and top; e.g. SYSCLR_BUTTONLIGHT
                      LONG lColorRight)     // in: color to use for right and bottom; e.g. SYSCLR_BUTTONDARK
{
    USHORT us;
    POINTL ptl1;

    for (us = 0;
         us < usWidth;
         us++)
    {
        GpiSetColor(hps, lColorLeft);
        // draw left line
        ptl1.x = prcl->xLeft;
        ptl1.y = prcl->yBottom;
        GpiMove(hps, &ptl1);
        ptl1.y = prcl->yTop;     // V0.9.7 (2000-12-20) [umoeller]
        GpiLine(hps, &ptl1);
        // go right -> draw top
        ptl1.x = prcl->xRight;   // V0.9.7 (2000-12-20) [umoeller]
        GpiLine(hps, &ptl1);
        // go down -> draw right
        GpiSetColor(hps, lColorRight);
        ptl1.y = prcl->yBottom;
        GpiLine(hps, &ptl1);
        // go left -> draw bottom
        ptl1.x = prcl->xLeft;
        GpiLine(hps, &ptl1);

        prcl->xLeft++;
        prcl->yBottom++;
        prcl->xRight--;
        prcl->yTop--;
    }
}

/*
 *@@ gpihDraw3DFrame:
 *      compatibility function for those who used the
 *      export. As opposed to gpihDraw3DFrame2, this
 *      does not modify prcl.
 *
 *@@added V1.0.0 (2002-08-24) [umoeller]
 */

VOID gpihDraw3DFrame(HPS hps,           // in: presentation space for output
                     PRECTL prcl,       // in: rectangle (inclusive)
                     USHORT usWidth,    // in: line width (>= 1)
                     LONG lColorLeft,   // in: color to use for left and top; e.g. SYSCLR_BUTTONLIGHT
                     LONG lColorRight)  // in: color to use for right and bottom; e.g. SYSCLR_BUTTONDARK
{
    RECTL rcl2 = *prcl;
    gpihDraw3DFrame2(hps, &rcl2, usWidth, lColorLeft, lColorRight);
}

/*
 *@@ gpihCharStringPosAt:
 *      wrapper for GpiCharStringPosAt.
 *      Since that function is limited to 512 characters
 *      (according to GPIREF; on my Warp 4 FP13, I actually
 *      get some 3000 characters... whatever this is),
 *      this splits the string into 512 byte chunks and
 *      calls GpiCharStringPosAt accordingly.
 *
 *@@added V0.9.3 (2000-05-06) [umoeller]
 *@@changed V0.9.20 (2002-08-10) [umoeller]: fixed underline for bitmap fonts, which never worked
 */

LONG gpihCharStringPosAt(HPS hps,               // in: presentation space for output
                         PPOINTL pptlStart,
                         PRECTL prclRect,
                         ULONG flOptions,
                         LONG lCount,
                         PCH pchString)
{
    LONG    lHits = 0,
            lCountLeft = lCount;
    PCH     pchThis = pchString;

    GpiMove(hps, pptlStart);

    if (lCount)
    {
        do
        {
            LONG    lCountThis = lCountLeft;
            if (lCountLeft >= 512)
                lCountThis = 512;

            lHits = GpiCharStringPos(hps,
                                     prclRect,
                                     0, // flOptions,
                                     lCountThis,
                                     pchThis,
                                     0);

            pchThis += 512;
            lCountLeft -= 512;
        } while (lCountLeft > 0);
    }

    // I can't get underscore to work with bitmap fonts,
    // so I'm doing it manually always now
    // V0.9.20 (2002-08-10) [umoeller]
    if (flOptions & CHS_UNDERSCORE)
    {
        POINTL ptl2, ptlSave;
        GpiQueryCurrentPosition(hps, &ptlSave);
        ptl2.x = pptlStart->x;
        ptl2.y = pptlStart->y - 2;
        GpiMove(hps, &ptl2);
        ptl2.x = ptlSave.x;
        GpiLine(hps, &ptl2);
        GpiMove(hps, &ptlSave);
    }

    return lHits;
}

/*
 *@@ gpihCalcTextExtent:
 *
 *@@added V1.0.1 (2003-01-17) [umoeller]
 */

VOID gpihCalcTextExtent(HPS hps,            // in: presentation space for output
                        PCSZ pcsz,          // in: string to test
                        PLONG pcx,          // out: max width occupied by a line in the string
                        PULONG pcLines)     // out: no. of lines
{
    LONG    cxLineThis;
    PCSZ    pThis = pcsz;
    *pcx = 0;

    *pcLines = 0;

    if (!pThis)
        return;

    while (*pThis)
    {
        ULONG lenThis;
        PCSZ pNext = strhFindEOL(pThis, &lenThis);

        ++(*pcLines);

        if (lenThis)
        {
            POINTL aptl[TXTBOX_COUNT];
            GpiQueryTextBox(hps,
                            lenThis,
                            (PCH)pThis,
                            TXTBOX_COUNT,
                            aptl);

            cxLineThis = aptl[TXTBOX_TOPRIGHT].x - aptl[TXTBOX_BOTTOMLEFT].x;

            if (cxLineThis > *pcx)
                *pcx = cxLineThis;
        }

        if (*pNext == '\r')
            pNext++;
        pThis = pNext;
    }
}

/*
 *@@ gpihDrawString:
 *      replacement for WinDrawText that can still align
 *      properly with multi-line strings.
 *
 *      fl works as with WinDrawText, that is:
 *
 *      --  specify one of DT_LEFT, DT_CENTER, DT_RIGHT;
 *
 *      --  specifiy one of DT_TOP, DT_VCENTER, DT_BOTTOM.
 *
 *      The alignment definitions are:
 *
 *      --  DT_LEFT                    0x00000000
 *      --  DT_CENTER                  0x00000100
 *      --  DT_RIGHT                   0x00000200
 *      --  DT_TOP                     0x00000000
 *      --  DT_VCENTER                 0x00000400
 *      --  DT_BOTTOM                  0x00000800
 *
 *      Other flags:
 *
 *      --  DT_QUERYEXTENT             0x00000002   (not supported)
 *      --  DT_UNDERSCORE              0x00000010   (not supported)
 *      --  DT_STRIKEOUT               0x00000020   (not supported)
 *      --  DT_TEXTATTRS               0x00000040   (always enabled)
 *      --  DT_EXTERNALLEADING         0x00000080   (not supported)
 *      --  DT_HALFTONE                0x00001000   (not supported)
 *      --  DT_MNEMONIC                0x00002000   (not supported)
 *      --  DT_WORDBREAK               0x00004000   (always enabled)
 *      --  DT_ERASERECT               0x00008000   (not supported)
 *
 *@@added V1.0.1 (2003-01-17) [umoeller]
 */

VOID gpihDrawString(HPS hps,                // in: presentation space for output
                    PCSZ pcsz,              // in: string to test
                    PRECTL prcl,            // in: clipping rectangle (inclusive!)
                    ULONG fl,               // in: alignment flags
                    PFONTMETRICS pfm)
{
    PCSZ    pThis = pcsz;
    POINTL  ptlRun,
            ptlUse;
    LONG    cxRect,
            cyRect,
            cyString;

    if (!pThis || !prcl || !pfm)
        return;

    ptlRun.x = prcl->xLeft;
    ptlRun.y = prcl->yTop;

    cxRect = prcl->xRight - prcl->xLeft + 1;
    cyRect = prcl->yTop - prcl->yBottom + 1;

    // vertical alignment:
    if (fl & (DT_VCENTER | DT_BOTTOM))
    {
        ULONG cLines = strhCount(pcsz, '\n') + 1;
        cyString = cLines * (pfm->lMaxBaselineExt + pfm->lMaxDescender + pfm->lExternalLeading);

        if (fl & DT_VCENTER)
            ptlRun.y = (cyRect - cyString) / 2;
        else
            ptlRun.y = cyString;
    }

    while (*pThis)
    {
        ULONG lenThis;
        PCSZ pNext = strhFindEOL(pThis, &lenThis);

        ptlRun.y -= pfm->lMaxBaselineExt;

        if (lenThis)
        {
            // horizontal alignment:
            if (!(fl & (DT_CENTER | DT_RIGHT)))
                ptlUse.x = ptlRun.x;
            else
            {
                POINTL  aptl[TXTBOX_COUNT];
                LONG    cxString;
                GpiQueryTextBox(hps,
                                lenThis,
                                (PCH)pThis,
                                TXTBOX_COUNT,
                                aptl);

                cxString = aptl[TXTBOX_TOPRIGHT].x - aptl[TXTBOX_BOTTOMLEFT].x;

                if (fl & DT_CENTER)
                    ptlUse.x = ptlRun.x + (cxRect - cxString) / 2;
                else
                    // right
                    ptlUse.x = ptlRun.x + cxRect - cxString;
            }

            ptlUse.y = ptlRun.y + pfm->lMaxDescender;

            GpiCharStringPosAt(hps,
                               &ptlUse,
                               prcl,
                               CHS_CLIP,
                               lenThis,
                               (PCH)pThis,
                               NULL);
        }

        ptlRun.y -= pfm->lExternalLeading;

        if (!*pNext)
            break;

        if (*pNext == '\r')
            pNext++;
        pThis = ++pNext;
    }
}

/*
 *@@ gpihFillBackground:
 *      fills the specified rectangle in the way
 *      that is specified by the given BKGNDINFO
 *      structure. This way one can either use
 *      a solid color, a color fade, a bitmap,
 *      or a combination of those.
 *
 *      See BKGNDINFO for the various parameters.
 *
 *      Since this can potentially be expensive,
 *      it is strongly recommended to use a buffer
 *      bitmap for painting with the size of the
 *      window and bitblt that bitmap into the
 *      window on repaints. This way the background
 *      only has to be recreated on window resize.
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 */

VOID gpihFillBackground(HPS hps,            // in: presentation space for output
                        PRECTL prcl,        // in: rectangle (inclusive!)
                        PBKGNDINFO pInfo)   // in: background into
{
    LONG    l;
    POINTL  ptl;

    switch (pInfo->flPaintMode & PMOD_COLORMASK)
    {
        case PMOD_SOLID:
            // fill with background color
            GpiSetColor(hps,
                        pInfo->lcol1);
            ptl.x = prcl->xLeft;
            ptl.y = prcl->yBottom;
            GpiMove(hps,
                    &ptl);
            ptl.x = prcl->xRight;
            ptl.y = prcl->yTop;
            GpiBox(hps,
                   DRO_FILL,
                   &ptl,
                   0,
                   0);
        break;

        case PMOD_TOPBOTTOM:
        {
            LONG lDiffRed   = (LONG)GET_RED(pInfo->lcol2) - (LONG)GET_RED(pInfo->lcol1);
            LONG lDiffGreen = (LONG)GET_GREEN(pInfo->lcol2) - (LONG)GET_GREEN(pInfo->lcol1);
            LONG lDiffBlue  = (LONG)GET_BLUE(pInfo->lcol2) - (LONG)GET_BLUE(pInfo->lcol1);

            LONG lMax = prcl->yTop - prcl->yBottom;

            // start at top
            ptl.y = prcl->yTop;

            for (l = 0;
                 l <= lMax;
                 ++l)
            {
                // compose RGB color for this line;
                // lcol1 is top, lcol2 is bottom
                LONG lRed   =   GET_RED(pInfo->lcol1)
                              + (   lDiffRed
                                  * l
                                  / lMax
                                );
                LONG lGreen =   GET_GREEN(pInfo->lcol1)
                              + (   lDiffGreen
                                  * l
                                  / lMax
                                );
                LONG lBlue  =   GET_BLUE(pInfo->lcol1)
                              + (   lDiffBlue
                                  * l
                                  / lMax
                                );

                GpiSetColor(hps, MAKE_RGB(lRed, lGreen, lBlue));
                ptl.x = prcl->xLeft;
                GpiMove(hps, &ptl);
                ptl.x = prcl->xRight;
                GpiLine(hps, &ptl);

                // next line below
                --(ptl.y);
            }
        }
        break;

        case PMOD_LEFTRIGHT:
        {
            LONG lDiffRed   = (LONG)GET_RED(pInfo->lcol2) - (LONG)GET_RED(pInfo->lcol1);
            LONG lDiffGreen = (LONG)GET_GREEN(pInfo->lcol2) - (LONG)GET_GREEN(pInfo->lcol1);
            LONG lDiffBlue  = (LONG)GET_BLUE(pInfo->lcol2) - (LONG)GET_BLUE(pInfo->lcol1);

            LONG lMax = prcl->xRight - prcl->xLeft;

            // start at left
            ptl.x = prcl->xLeft;

            for (l = 0;
                 l <= lMax;
                 ++l)
            {
                // compose RGB color for this line;
                // lcol1 is top, lcol2 is bottom
                LONG lRed   =   GET_RED(pInfo->lcol1)
                              + (   lDiffRed
                                  * l
                                  / lMax
                                );
                LONG lGreen =   GET_GREEN(pInfo->lcol1)
                              + (   lDiffGreen
                                  * l
                                  / lMax
                                );
                LONG lBlue  =   GET_BLUE(pInfo->lcol1)
                              + (   lDiffBlue
                                  * l
                                  / lMax
                                );

                GpiSetColor(hps, MAKE_RGB(lRed, lGreen, lBlue));
                ptl.y = prcl->yBottom;
                GpiMove(hps, &ptl);
                ptl.y = prcl->yTop;
                GpiLine(hps, &ptl);

                // next line to the right
                ++(ptl.x);
            }
        }
        break;
    }
}

/*
 *@@category: Helpers\PM helpers\GPI helpers\Fonts
 */

/* ******************************************************************
 *
 *   Font helpers
 *
 ********************************************************************/

/*
 *@@ gpihMatchFont:
 *      attempts to find a font matching the specified
 *      data and fills the specified FATTRS structure
 *      accordingly.
 *
 *      This function performs the insane "11-step process" to
 *      match a font, as described in the GPI reference.
 *
 *      This function can operate in two modes:
 *
 *      -- "Family" mode. In that case, specify the font family name
 *         with pszName and set fFamily to TRUE. This is useful for
 *         WYSIWYG text viewing if you need several font faces for
 *         the same family, such as Courier Bold, Bold Italics, etc.
 *         You can specify those attributes with usFormat then.
 *
 *      -- "Face" mode. In that case, specify the full font face name
 *         with pszName and set fFamily to FALSE. This is useful for
 *         font presentation parameters which use the "WarpSans Bold"
 *         format. In that case, set usFormat to 0.
 *
 *      Returns TRUE if a "true" match was found, FALSE
 *      otherwise. In both cases, *pfa receives data
 *      which will allow GpiCreateLogFont to work; however,
 *      if FALSE is returned, GpiCreateLogFont will most
 *      likely find the default font (System Proportional)
 *      only.
 *
 *      If (pFontMetrics != NULL), *pFontMetrics receives the
 *      FONTMETRICS of the font which was found. If an outline
 *      font has been found (instead of a bitmap font),
 *      FONTMETRICS.fsDefn will have the FM_DEFN_OUTLINE bit set.
 *
 *      This function was extracted from gpihFindFont with
 *      0.9.14 to allow for caching the font search results,
 *      which is most helpful for memory device contexts,
 *      where gpihFindFont can be inefficient.
 *
 *@@added V0.9.14 (2001-08-03) [umoeller]
 *@@changed V0.9.14 (2001-08-03) [umoeller]: fixed a few weirdos with outline fonts
 */

BOOL gpihMatchFont(HPS hps,                 // in: presentation space for output
                   LONG lSize,              // in: font point size
                   BOOL fFamily,            // in: if TRUE, pszName specifies font family;
                                            //     if FALSE, pszName specifies font face
                   const char *pcszName,    // in: font family or face name (without point size)
                   USHORT usFormat,         // in: none, one or several of:
                                            // -- FATTR_SEL_ITALIC
                                            // -- FATTR_SEL_UNDERSCORE (underline)
                                            // -- FATTR_SEL_BOLD
                                            // -- FATTR_SEL_STRIKEOUT
                                            // -- FATTR_SEL_OUTLINE (hollow)
                   FATTRS *pfa,             // out: font attributes if found
                   PFONTMETRICS pFontMetrics) // out: font metrics of created font (optional)
{
    // first find out how much memory we need to allocate
    // for the FONTMETRICS structures
    ULONG   ul = 0;
    LONG    lTemp = 0;
    LONG    cFonts = GpiQueryFonts(hps,
                                   QF_PUBLIC | QF_PRIVATE,
                                   NULL, // pszFaceName,
                                   &lTemp,
                                   sizeof(FONTMETRICS),
                                   NULL);
    PFONTMETRICS    pfm = (PFONTMETRICS)malloc(cFonts * sizeof(FONTMETRICS)),
                    pfm2 = pfm,
                    pfmFound = NULL;

    BOOL            fQueriedDevice = FALSE;     // V0.9.14 (2001-08-01) [umoeller]
    LONG            alDevRes[2];            // device resolution

    // _Pmpf(("gpihFindFont: enumerating for %s, %d points", pcszName, lSize));

    GpiQueryFonts(hps,
                  QF_PUBLIC | QF_PRIVATE,
                  NULL, // pszFaceName,
                  &cFonts,
                  sizeof(FONTMETRICS),      // length of each metrics structure
                                            // -- _not_ total buffer size!
                  pfm);

    // now we have an array of FONTMETRICS
    // for EVERY font that is installed on the system...
    // these things are completely unsorted, so there's
    // nothing we can rely on, we have to check them all.

    // fill in some default values for FATTRS,
    // in case we don't find something better
    // in the loop below; these values will be
    // applied if
    // a)   an outline font has been found;
    // b)   bitmap fonts have been found, but
    //      none for the current device resolution
    //      exists;
    // c)   no font has been found at all.
    // In all cases, GpiCreateLogFont will do
    // a "close match" resolution (at the bottom).
    pfa->usRecordLength = sizeof(FATTRS);
    pfa->fsSelection = usFormat; // changed later if better font is found
    pfa->lMatch = 0L;             // closest match
    strcpy(pfa->szFacename, pcszName);
    pfa->idRegistry = 0;          // default registry
    pfa->usCodePage = 0;          // default codepage
    // the following two must be zero, or outline fonts
    // will not be found; if a bitmap font has been passed
    // to us, we'll modify these two fields later
    pfa->lMaxBaselineExt = 0;     // font size (height)
    pfa->lAveCharWidth = 0;       // font size (width)
    pfa->fsType = 0;              // default type
    pfa->fsFontUse = FATTR_FONTUSE_NOMIX;

    // now go thru the array of FONTMETRICS
    // to check if we have a bitmap font
    // pszFaceName; the default WPS behavior
    // is that bitmap fonts appear to take
    // priority over outline fonts of the
    // same name, so we check these first
    pfm2 = pfm;
    for (ul = 0;
         ul < cFonts;
         ul++)
    {
        const char *pcszCompare = (fFamily)
                                     ? pfm2->szFamilyname
                                     : pfm2->szFacename;

        /* _Pmpf(("  Checking font: %s (Fam: %s), %d, %d, %d",
               pcszCompare,
               pfm2->szFamilyname,
               pfm2->sNominalPointSize,
               pfm2->lMaxBaselineExt,
               pfm2->lAveCharWidth)); */

        if (!strcmp(pcszCompare, pcszName))
        {
            /* _Pmpf(("  Found font %s; slope %d, usWeightClass %d",
                    pfm2->szFacename,
                    pfm2->sCharSlope,
                    pfm2->usWeightClass)); */

            if ((pfm2->fsDefn & FM_DEFN_OUTLINE) == 0)
            {
                // image (bitmap) font:
                // check point size
                if (pfm2->sNominalPointSize == lSize * 10)
                {
                    // OK: check device resolutions, because
                    // normally, there are always several image
                    // fonts for different resolutions
                    // for bitmap fonts, there are normally two versions:
                    // one for low resolutions, one for high resolutions
                    if (!fQueriedDevice)
                    {
                        DevQueryCaps(GpiQueryDevice(hps),
                                     CAPS_HORIZONTAL_FONT_RES,
                                     2L,
                                     alDevRes);
                        fQueriedDevice = TRUE;
                    }

                    if (    (pfm2->sXDeviceRes == alDevRes[0])
                         && (pfm2->sYDeviceRes == alDevRes[1])
                       )
                    {
                        // OK: use this for GpiCreateLogFont
                        pfa->lMaxBaselineExt = pfm2->lMaxBaselineExt;
                        pfa->lAveCharWidth = pfm2->lAveCharWidth;
                        // pfa->lMatch = pfm2->lMatch;

                        pfmFound = pfm2;
                        break;
                    }
                }
            }
            else
                // outline font:
                if (pfmFound == NULL)
                {
                    // no bitmap font found yet:

                    /*
                        #define FATTR_SEL_ITALIC               0x0001
                        #define FATTR_SEL_UNDERSCORE           0x0002
                        #define FATTR_SEL_OUTLINE              0x0008
                        #define FATTR_SEL_STRIKEOUT            0x0010
                        #define FATTR_SEL_BOLD                 0x0020
                     */

                    if (    (!fFamily)          // face mode is OK always
                                                // V0.9.14 (2001-08-03) [umoeller]
                         || (    (    (    (usFormat & FATTR_SEL_BOLD)
                                        && (pfm2->usWeightClass == 7) // bold
                                      )
                                   || (    (!(usFormat & FATTR_SEL_BOLD))
                                        && (pfm2->usWeightClass == 5) // regular
                                      )
                                 )
                              && (    (    (usFormat & FATTR_SEL_ITALIC)
                                        && (pfm2->sCharSlope != 0) // italics
                                      )
                                   || (    (!(usFormat & FATTR_SEL_ITALIC))
                                        && (pfm2->sCharSlope == 0) // regular
                                      )
                                 )
                            )
                       )
                    {
                        // yes, we found a true font for that face:
                        pfmFound = pfm2;

                        // use this exact font for GpiCreateLogFont
                        pfa->lMatch = pfm2->lMatch;

                        // the following two might have been set
                        // for a bitmap font above
                        // V0.9.14 (2001-08-03) [umoeller]
                        pfa->lMaxBaselineExt = pfm2->lMaxBaselineExt;
                        pfa->lAveCharWidth = pfm2->lAveCharWidth;

                        pfa->idRegistry = pfm2->idRegistry;

                        // override NOMIX // V0.9.14 (2001-08-03) [umoeller]
                        pfa->fsFontUse = FATTR_FONTUSE_OUTLINE;

                        // according to GPIREF, we must also specify
                        // the full face name... geese!
                        strcpy(pfa->szFacename, pfm2->szFacename);
                        // unset flag in FATTRS, because this would
                        // duplicate bold or italic
                        pfa->fsSelection = 0;

                        // _Pmpf(("    --> using it"));
                        // but loop on, because we might have a bitmap
                        // font which should take priority
                    }
                }
        }

        pfm2++;
    }

    if (pfmFound)
        // FONTMETRICS found:
        // copy font metrics?
        if (pFontMetrics)
            memcpy(pFontMetrics, pfmFound, sizeof(FONTMETRICS));

    // free the FONTMETRICS array
    free(pfm);

    return (pfmFound != NULL);
}

/*
 *@@ gpihSplitPresFont:
 *      splits a presentation parameter font
 *      string into the point size and face
 *      name so that it can be passed to
 *      gpihFindFont more easily.
 *
 *@@added V0.9.1 (2000-02-15) [umoeller]
 *@@changed V0.9.20 (2002-07-19) [umoeller]: optimized
 */

BOOL gpihSplitPresFont(PSZ pszFontNameSize,  // in: e.g. "12.Courier"
                       PULONG pulSize,       // out: integer point size (e.g. 12);
                                             // ptr must be specified
                       PSZ *ppszFaceName)    // out: ptr into pszFontNameSize
                                             // (e.g. "Courier")
{
    PCHAR   pcDot;
    if (    (pszFontNameSize)
         && (pcDot = strchr(pszFontNameSize, '.'))
       )
    {
        // _Pmpf(("Found font PP: %s", pszFontFound));
        // sscanf(pszFontNameSize, "%lu", pulSize);
        *pulSize = atoi(pszFontNameSize);       // V0.9.20 (2002-07-19) [umoeller]
        *ppszFaceName = pcDot + 1;
        return TRUE;
    }

    return FALSE;
}

/*
 *@@ gpihLockLCIDs:
 *      requests the mutex for serializing the
 *      lcids.
 *
 *      With GPI, lcids are a process-wide resource and not
 *      guaranteed to be unique. In the worst case, while your
 *      font routines are running, another thread modifies the
 *      lcids and you get garbage. If your fonts suddenly
 *      turn to "System Proportional", you know this has
 *      happened.
 *
 *      As a result, whenever you work on lcids, request this
 *      mutex during your processing. If you do this consistently
 *      across all your code, you should be safe.
 *
 *      gpihFindFont uses this mutex. If you call GpiCreateLogFont
 *      yourself somewhere, you should do this under the protection
 *      of this function.
 *
 *      Call gpihUnlockLCIDs to unlock.
 *
 *@@added V0.9.9 (2001-04-01) [umoeller]
 */

BOOL gpihLockLCIDs(VOID)
{
    if (!G_hmtxLCIDs)
        // first call: create
        return !DosCreateMutexSem(NULL,
                                  &G_hmtxLCIDs,
                                  0,
                                  TRUE);     // request!

    // subsequent calls: request
    return !DosRequestMutexSem(G_hmtxLCIDs, SEM_INDEFINITE_WAIT);
}

/*
 *@@ UnlockLCIDs:
 *      releases the mutex for serializing the
 *      lcids.
 *
 *@@added V0.9.9 (2001-04-01) [umoeller]
 */

VOID gpihUnlockLCIDs(VOID)
{
    DosReleaseMutexSem(G_hmtxLCIDs);
}

/*
 *@@ gpihQueryNextLCID:
 *      returns the next available lcid for the given HPS.
 *      Actually, it's the next available lcid for the
 *      entire process, since there can be only 255 altogether.
 *      Gets called by gpihFindFont automatically.
 *
 *      WARNING: This function by itself is not thread-safe.
 *      See gpihLockLCIDs for how to serialize this.
 *
 *      Code was extensively re-tested, works (V0.9.12 (2001-05-31) [umoeller]).
 *
 *@@added V0.9.3 (2000-05-06) [umoeller]
 *@@changed V0.9.9 (2001-04-01) [umoeller]: removed all those sick sub-allocs
 */

LONG gpihQueryNextFontID(HPS hps)       // in: presentation space for output
{
    LONG    lcidNext = -1;

    LONG lCount =  GpiQueryNumberSetIds(hps);
                             //  the number of local identifiers
                             //  (lcids) currently in use, and
                             //  therefore the maximum number
                             //  of objects for which information
                             //  can be returned

    // _Pmpf((__FUNCTION__ ": Entering"));

    if (lCount == 0)
    {
        // none in use yet:
        lcidNext = 15;

        // _Pmpf(("  no lcids in use"));
    }
    else
    {
        // #define GQNCL_BLOCK_SIZE 400*sizeof(LONG)

        PLONG  alTypes = NULL;  // object types
        PSTR8  aNames = NULL;   // font names
        PLONG  allcids = NULL;  // local identifiers

        if (    (alTypes = (PLONG)malloc(lCount * sizeof(LONG)))
             && (aNames = (PSTR8)malloc(lCount * sizeof(STR8)))
             && (allcids = (PLONG)malloc(lCount * sizeof(LONG)))
           )
        {
            if (GpiQuerySetIds(hps,
                               lCount,
                               alTypes,
                               aNames,
                               allcids))
            {
                // FINALLY we have all the lcids in use.
                BOOL    fContinue = TRUE;
                lcidNext = 15;

                // _Pmpf(("  %d fonts in use, browsing...", lCount));

                // now, check if this lcid is in use already:
                while (fContinue)
                {
                    BOOL fFound = FALSE;
                    ULONG ul;
                    fContinue = FALSE;
                    for (ul = 0;
                         ul < lCount;
                         ul++)
                    {
                        if (allcids[ul] == lcidNext)
                        {
                            fFound = TRUE;
                            break;
                        }
                    }

                    if (fFound)
                    {
                        // lcid found:
                        // try next higher one

                        // _Pmpf(("       %d is busy...", lcidNext));

                        lcidNext++;
                        fContinue = TRUE;
                    }
                    // else
                        // else: return that one
                        // _Pmpf(("  %d is free", lcidNext));
                }
            }
        }

        if (alTypes)
            free(alTypes);
        if (aNames)
            free(aNames);
        if (allcids)
            free(allcids);
    }

    // _Pmpf((__FUNCTION__ ": Returning lcid %d", lcidNext));

    return lcidNext;
}

/*
 *@@ gpihCreateFont:
 *
 *@@added V0.9.14 (2001-08-03) [umoeller]
 */

LONG gpihCreateFont(HPS hps,            // in: presentation space for output
                    FATTRS *pfa)
{
    LONG lLCIDReturn = 0;

    if (gpihLockLCIDs())        // V0.9.9 (2001-04-01) [umoeller]
    {
        // new logical font ID: last used plus one
        lLCIDReturn = gpihQueryNextFontID(hps);

        GpiCreateLogFont(hps,
                         NULL,  // don't create "logical font name" (STR8)
                         lLCIDReturn,
                         pfa);

        gpihUnlockLCIDs();
    }

    return lLCIDReturn;
}

/*
 *@@ gpihFindFont:
 *      this returns a new logical font ID (LCID) for the specified
 *      font by calling gpihMatchFont first and then
 *      GpiCreateLogFont to create a logical font from the
 *      data returned.
 *
 *      See gpihMatchFont for additional explanations.
 *
 *      To then use the font whose LCID has been returned by this
 *      function for text output, call:
 +          GpiSetCharSet(hps, lLCIDReturned);
 *
 *      <B>Font Point Sizes:</B>
 *
 *      1)  For image (bitmap) fonts, the size is fixed, and
 *          you can directly draw after the font has been
 *          selected. GpiSetCharBox has no effect on image
 *          fonts unless you switch to character mode 2
 *          (if you care for that: GpiSetCharMode).
 *
 *      2)  For outline fonts however, you need to define a
 *          character box if you want to output text in a
 *          size other than the default size. This is almost
 *          as bad a mess as this function, so gpihSetPointSize
 *          has been provided for this. See remarks there.
 *
 *      <B>Example:</B>
 *
 *      This example prints text in "24.Courier".
 *
 +          PSZ     pszOutput = "Test output";
 +          FONTMETRICS FontMetrics;
 +          LONG    lLCID = gpihFindFont(hps,
 +                                       24,        // point size
 +                                       FALSE,     // face, not family
 +                                       "Courier",
 +                                       0,
 +                                       &FontMetrics);
 +          if (lLCID)
 +          {
 +              // no error:
 +              GpiSetCharSet(hps, lLCID);
 +              if (FontMetrics.fsDefn & FM_DEFN_OUTLINE)
 +                  // outline font found (should be the case):
 +                  gpihSetPointSize(hps, 24);
 +          }
 +          GpiCharString(hps, strlen(pszOutput), pszOutput);
 *
 *      <B>Details:</B>
 *
 *      First, GpiQueryFonts is called to enumerate all the fonts on
 *      the system. We then evaluate the awful FONTMETRICS data
 *      of those fonts to perform a "real" match.
 *
 *      If that fails, we allow GPI to do a "close" match based
 *      on the input values. This might result in the system
 *      default font (System Proportional) to be found, in the
 *      worst case. But even then, a new LCID is returned.
 *
 *      The "close match" comes in especially when using the
 *      font attributes (bold, italics) and we are unable to
 *      find the correct outline font for that. Unfortunately,
 *      the information in FONTMETRICS.fsSelection is wrong,
 *      wrong, wrong for the large majority of fonts. (For
 *      example, I get the "bold" flag set for regular fonts,
 *      and vice versa.) So we attempt to use the other fields,
 *      but this doesn't always help. Not even Netscape gets
 *      this right.
 *
 *      <B>Font faces:</B>
 *
 *      This is terribly complicated as well. You need to
 *      differentiate between "true" emphasis faces (that
 *      is, bold and italics are implemented thru separate
 *      font files) and the ugly GPI simulation, which simply
 *      makes the characters wider or shears them.
 *
 *      This function even finds true "bold" and "italic" faces
 *      for outline fonts. To do this, always specify the "family"
 *      name as pszFaceName (e.g. "Courier" instead of "Courier
 *      Bold") and set the flags for usFormat (e.g. FATTR_SEL_BOLD).
 *      Note that this implies that you need call this function
 *      twice to create two logical fonts for regular and bold faces.
 *
 *      If a "true" emphasis font is not found, the GPI simulation
 *      is enabled.
 *
 *      <B>Remarks:</B>
 *
 *      1)  This function _always_ creates a new logical font,
 *          whose ID is returned, even if the specified font
 *          was not found or a "close match" was performed by
 *          GPI. As a result, do not call this function twice
 *          for the same font specification, because there are
 *          only 255 logical font IDs for each process.
 *
 *      2)  Since this function always creates an LCID,
 *          you should _always_ free the LCID later.
 *          This is only valid if the font is no longer selected
 *          into any presentation space. So use these calls:
 +              GpiSetCharSet(hps, LCID_DEFAULT);
 +              GpiDeleteSetId(hps, lLCIDReturnedByThis);
 *
 *      3)  Using this function, bitmap fonts will have priority
 *          over outline fonts of the same face name. This is
 *          how the WPS does it too. This is most obvious with
 *          the "Helv" font, which exists as a bitmap font for
 *          certain point sizes only.
 *
 *      4)  Since logical font IDs are shared across the
 *          process, a mutex is requested while the lcids are
 *          being queried and/or manipulated. In other words,
 *          this func is now thread-safe (V0.9.9).
 *
 *          This calls gpihQueryNextFontID in turn to find the
 *          next free lcid. See remarks there.
 *
 *      <B>Font metrics:</B>
 *
 *      The important values in the returned FONTMETRICS are
 *      like this (according to PMREF):
 *
 +   …Õ ________________________________________________
 +   ∫
 +   ∫ lExternalLeading, according to font designer.
 +   ∫  ________________________________________________  Õª
 +   »Õ                                                    ∫
 +                                  #       #              ∫
 +                                  ##     ##              ∫ lMaxAscender (of entire;
 +   …Õ _______________             # #   # #              ∫ font); this can be > capital
 +   ∫                 ####         #  # #  #              ∫ letters because miniscules
 +   ∫                #    #        #   #   #              ∫ can exceed that.
 +   ∫  lXHeight      #    #        #       #              ∫
 +   ∫                #    #        #       #              ∫
 +   ∫                #    #        #       #              ∫
 +   ∫  _______________#####________#_______#___ baseline Õª
 +   »Õ                    #                               ∫
 +                         #                               ∫ lMaxDescender
 +      ______________ ####______________________________ Õº
 +
 *
 *      In turn, lMaxBaselineExt is lMaxAscender + lMaxDescender.
 *
 *      Soooo... to find out about the optimal line spacing, GPIREF
 *      recommends to use lMaxBaselineExt + lExternalLeading.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.3 (2000-05-06) [umoeller]: didn't work for more than one font; now using gpihQueryNextFontID
 *@@changed V0.9.3 (2000-05-06) [umoeller]: usFormat didn't work; fixed
 *@@changed V0.9.4 (2000-08-08) [umoeller]: added fFamily
 *@@changed V0.9.9 (2001-04-01) [umoeller]: made this thread-safe, finally
 *@@changed V0.9.14 (2001-08-01) [umoeller]: some optimizations
 */

LONG gpihFindFont(HPS hps,               // in: presentation space for output
                  LONG lSize,            // in: font point size
                  BOOL fFamily,          // in: if TRUE, pszName specifies font family;
                                         //     if FALSE, pszName specifies font face
                  const char *pcszName,  // in: font family or face name (without point size)
                  USHORT usFormat,       // in: none, one or several of:
                                         // -- FATTR_SEL_ITALIC
                                         // -- FATTR_SEL_UNDERSCORE (underline)
                                         // -- FATTR_SEL_BOLD
                                         // -- FATTR_SEL_STRIKEOUT
                                         // -- FATTR_SEL_OUTLINE (hollow)
                  PFONTMETRICS pFontMetrics) // out: font metrics of created font (optional)
{
    FATTRS  FontAttrs;

    gpihMatchFont(hps,
                  lSize,
                  fFamily,
                  pcszName,
                  usFormat,
                  &FontAttrs,
                  pFontMetrics);

    return gpihCreateFont(hps,
                          &FontAttrs);

    // _Pmpf((__FUNCTION__ ": returning lcid %d", lLCIDReturn));
}

/*
 *@@ gpihFindPresFont:
 *      similar to gpihFindFont, but this one evaluates
 *      the PP_FONTNAMESIZE presentation parameter of the
 *      specified window instead. If that one is not set,
 *      the specified default font is used instead.
 *
 *      Note that as opposed to gpihFindFont, this one
 *      takes a "8.Helv"-type string as input.
 *
 *      See gpihFindFont for additional remarks, which
 *      gets called by this function.
 *
 *      Again, if an outline font has been returned, you
 *      must also set the "character box" for the HPS, or
 *      your text will always have the same point size.
 *      Use gpihSetPointSize for that, using the presparam's
 *      point size, which is returned by this function
 *      into *plSize, if (plSize != NULL):
 +
 +          FONTMETRICS FontMetrics;
 +          LONG        lPointSize;
 +          LONG        lLCID = gpihFindPresFont(hwnd, hps, "8.Helv",
 +                                               &FontMetrics,
 +                                               &lPointSize);
 +          GpiSetCharSet(hps, lLCID);
 +          if (FontMetrics.fsDefn & FM_DEFN_OUTLINE)
 +              gpihSetPointSize(hps, lPointSize);
 *
 *      If errors occur, e.g. if the font string does not
 *      conform to the "size.face" format, null is returned.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.9 (2001-04-01) [umoeller]: now supporting NULLHANDLE hwnd
 */

LONG gpihFindPresFont(HWND hwnd,          // in: window to search for presparam or NULLHANDLE
                      BOOL fInherit,      // in: search parent windows too?
                      HPS hps,            // in: HPS for font selection
                      const char *pcszDefaultFont, // in: default font if not found (i.e. "8.Helv")
                      PFONTMETRICS pFontMetrics, // out: font metrics of created font (optional)
                      PLONG plSize)       // out: presparam's point size (optional)
{
    CHAR    szPPFont[200] = "";
    const char *pcszFontFound = 0;
    CHAR    szFaceName[300] = "";
    ULONG   ulFontSize = 0;

    if (    (hwnd)        // V0.9.9 (2001-04-01) [umoeller]
         && (WinQueryPresParam(hwnd,
                               PP_FONTNAMESIZE,          // first PP to query
                               0,                        // second PP to query
                               NULL,                     // out: which one is returned
                               (ULONG)sizeof(szPPFont),  // in: buffer size
                               (PVOID)&szPPFont,         // out: PP value returned
                               (fInherit)
                                    ? 0
                                    : QPF_NOINHERIT))
       )
        // PP found:
        pcszFontFound = szPPFont;
    else
        pcszFontFound = pcszDefaultFont;

    if (pcszFontFound)
    {
        const char *pcDot = strchr(pcszFontFound, '.');
        if (pcDot)
        {
            // _Pmpf(("Found font PP: %s", pszFontFound));
            sscanf(pcszFontFound, "%lu", &ulFontSize);
            if (plSize)
                *plSize = ulFontSize;
            strcpy(szFaceName, pcDot + 1);
            return gpihFindFont(hps,
                                ulFontSize,
                                FALSE,         // face, not family name
                                szFaceName,
                                0,
                                pFontMetrics);
        }
    }

    return 0;
}

/*
 *@@ gpihSetPointSize:
 *      this invokes GpiSetCharBox on the given HPS to
 *      set the correct "character box" with the proper
 *      parameters.
 *
 *      This is necessary for text output with outline
 *      fonts. Bitmap fonts have a fixed size, and
 *      calling this function is not necessary for them.
 *
 *      Unfortunately, IBM has almost not documented
 *      how to convert nominal point sizes to the
 *      correct character cell values. This involves
 *      querying the output device (DevQueryCaps).
 *
 *      I have found one hint for this procedure in GPIREF
 *      (chapter "Character string primitives", "Using...",
 *      "Drawing text"), but the example code given
 *      there is buggy, because it must be "72" instead
 *      of "720". #%(%!"ı!!.
 *
 *      So here we go. If you want to output text in,
 *      say, "Courier" and 24 points (nominal point size),
 *      select the font into your HPS and call
 +          gpihSetPointSize(hps, 24)
 *      and you're done. See gpihFindFont for a complete
 *      example.
 *
 *      Call this function as many times as needed. This
 *      consumes no resources at all.
 *
 *      This returns the return value of GpiSetCharBox.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.14 (2001-08-03) [umoeller]: fixed bad rounding errors
 */

BOOL gpihSetPointSize(HPS hps,          // in: presentation space for output
                      LONG lPointSize)  // in: desired nominal point size
{
    SIZEF   box;
    HDC     hdc = GpiQueryDevice(hps);       // get the HDC from the HPS
    LONG    alDevRes[2];
    DevQueryCaps(GpiQueryDevice(hps),       // get the HDC from the HPS
                 CAPS_HORIZONTAL_FONT_RES,
                 2L,
                 alDevRes);

    // V0.9.14: this code didn't work... it produced rounding
    // errors which set the font size different from what
    // it should be according to the WPS font palette
    /* box.cx = MAKEFIXED((lPointSize * alDevRes[0]) / 72, 0);
    box.cy = MAKEFIXED((lPointSize * alDevRes[1]) / 72, 0); */

    // V0.9.14 (2001-08-03) [umoeller]: now using this one
    // instead
    lPointSize *= 65536;
    box.cx = (FIXED)(lPointSize / 72 * alDevRes[0]);
    box.cy = (FIXED)(lPointSize / 72 * alDevRes[1]);

    return GpiSetCharBox(hps, &box);
}

/*
 *@@ gpihQueryLineSpacing:
 *      this returns the optimal line spacing for text
 *      output with the current HPS; this is computed
 *      by evaluating those incredible FONTMETRICS.
 *
 *      This might be helpful if you write text to the
 *      screen yourself and need the height of a text
 *      line to advance to the next.
 *
 *@@changed V0.9.7 (2000-12-20) [umoeller]: removed psz param
 */

LONG gpihQueryLineSpacing(HPS hps)      // in: presentation space for output
{
    FONTMETRICS fm;

    if (GpiQueryFontMetrics(hps, sizeof(FONTMETRICS), &fm))
        return ( (  fm.lMaxBaselineExt     // max vertical font space
                   +fm.lExternalLeading)    // space advised by font designer
               );

    return 15;
}

/*
 *@@category: Helpers\PM helpers\GPI helpers\Bitmaps/Icons
 */

/* ******************************************************************
 *
 *   Bitmap helpers
 *
 ********************************************************************/

/*
 *@@ gpihCreateMemPS:
 *      creates a memory device context and presentation space so
 *      that they are compatible with the screen device context and
 *      presentation space. These are stored in *hdcMem and *hpsMem.
 *
 *      This is a one-shot function for the standard code that is
 *      always needed when working with bitmaps in a memory device
 *      context.
 *
 *      psizlPage must point to a SIZEL structure containing the
 *      width and height for the memory PS. Specify the size of
 *      the future bitmap here. Specify {0, 0} to get a PS with
 *      the size of the full screen, which consumes quite a bit
 *      of memory though.
 *
 *      Returns FALSE upon errors. In that case, both hdcMem and
 *      hpsMem are set to NULLHANDLE.
 *
 *      To cleanup after this function has returned TRUE, use the
 *      following:
 +          GpiDestroyPS(hpsMem);
 +          DevCloseDC(hdcMem);
 *
 *@@changed V0.9.3 (2000-05-18) [umoeller]: added psiszlPage
 */

BOOL gpihCreateMemPS(HAB hab,       // in: anchor block
                     PSIZEL psizlPage, // in: width and height for mem PS
                     HDC *hdcMem,   // out: memory DC or NULLHANDLE upon errors
                     HPS *hpsMem)   // out: memory PS or NULLHANDLE upon errors
{
    BOOL brc = FALSE;
    PSZ pszData[4] = { "Display", NULL, NULL, NULL };

    // create new memory DC
    if ((*hdcMem = DevOpenDC(hab,
                             OD_MEMORY,      // create memory DC
                             "*",            // token: do not take INI info
                             4,              // item count in pszData
                             (PDEVOPENDATA)pszData,
                             NULLHANDLE)))    // compatible with screen
    {
        // memory DC created successfully:
        // create compatible PS
        if ((*hpsMem = GpiCreatePS(hab,
                                   *hdcMem,      // HDC to associate HPS with (GPIA_ASSOC);
                                                 // mandatory for GPIT_MICRO
                                   psizlPage,    // is (0, 0) == screen size
                                   PU_PELS       // presentation page units: pixels
                                    | GPIA_ASSOC    // associate with hdcMem (req. for GPIT_MICRO)
                                    | GPIT_MICRO)))  // micro presentation space
            brc = TRUE;
        else
        {
            // error (hpsMem == NULLHANDLE):
            // close memory DC again
            DevCloseDC(*hdcMem);
            *hdcMem = NULLHANDLE;
        }
    }

    return brc;
}

/*
 *@@ gpihCreateBitmap:
 *      calls gpihCreateBitmap2 with cPlanes and cBitCount == 0
 *      for compatibility with exports. Widgets might
 *      have used this func.
 *
 *@@changed V0.9.0 [umoeller]: function prototype changed to cx and cy
 *@@changed V0.9.16 (2001-12-18) [umoeller]: now using optimized gpihCreateBitmap2
 */

HBITMAP gpihCreateBitmap(HPS hpsMem,        // in: memory DC
                         ULONG cx,          // in: width of new bitmap
                         ULONG cy)          // in: height of new bitmap
{
    return gpihCreateBitmap2(hpsMem,
                             cx,
                             cy,
                             0,
                             0);            // current screen bit count
}

static const BITMAPINFOHEADER2 G_bih2Template[] =
    {
        sizeof(BITMAPINFOHEADER2), // ULONG      cbFix;
        0,                  // ULONG      cx;               // tbr
        0,                  // ULONG      cy;               // tbr
        0,                  // USHORT     cPlanes;          // tbr
        0,                  // USHORT     cBitCount;        // tbr
        BCA_UNCOMP,         // ULONG      ulCompression;
        0,                  // ULONG      cbImage;
        70,                 // ULONG      cxResolution;
        70,                 // ULONG      cyResolution;
        2,                  // ULONG      cclrUsed;
        0,                  // ULONG      cclrImportant;
        BRU_METRIC,         // USHORT     usUnits;
                                    // measure units for cxResolution/cyResolution: pels per meter
        0,                  // USHORT     usReserved;
        BRA_BOTTOMUP,       // USHORT     usRecording;
                                    // scan lines are bottom to top (default)

        BRH_NOTHALFTONED,   // USHORT     usRendering;
                                    // other algorithms aren't documented anyway
        0,                  // ULONG      cSize1;
                                    // parameter for halftoning (undocumented anyway)
        0,                  // ULONG      cSize2;
                                    // parameter for halftoning (undocumented anyway)
        BCE_RGB,            // ULONG      ulColorEncoding;
                                    // only possible value
        0                   // ULONG      ulIdentifier;
                                    // application-specific data
    };

/*
 *@@ gpihCreateBitmap2:
 *      creates a new bitmap for a given memory PS.
 *      This bitmap will have the cPlanes and bitcount
 *      which are found in the memory PS.
 *      For all the mysterious other values, we use
 *      fixed default values, this doesn't seem to hurt.
 *
 *      Note that the bitmap is _not_ yet selected into
 *      the specified memory PS. You must call
 +          GpiSetBitmap(hpsMem, hbm)
 *      to do this.
 *
 *      Returns the bitmap handle or NULLHANDLE upon errors.
 *
 *@@added V0.9.16 (2001-12-18) [umoeller]
 *@@changed V1.0.1 (2002-11-30) [umoeller]: optimized
 */

HBITMAP gpihCreateBitmap2(HPS hpsMem,        // in: memory DC
                          ULONG cx,          // in: width of new bitmap
                          ULONG cy,          // in: height of new bitmap
                          ULONG cPlanes,     // in: color planes (usually 1); if 0, current screen is used
                          ULONG cBitCount)   // in: either 1, 4, or 24; if 0, current screen value
{
    LONG alData[2];
    BITMAPINFOHEADER2 bih2;

    // determine the device's plane/bit-count format;
    // alData[0] then has cPlanes,
    // alData[1] has cBitCount
    if (GpiQueryDeviceBitmapFormats(hpsMem, 2, alData))
    {
        // copy values from global template V1.0.1 (2002-11-30) [umoeller]
        memcpy(&bih2, &G_bih2Template, sizeof(bih2));

        // fix variable fields V1.0.1 (2002-11-30) [umoeller]
        bih2.cx = cx;
        bih2.cy = cy;
        bih2.cPlanes = (cPlanes) ? cPlanes : alData[0];
        bih2.cBitCount = (cBitCount) ? cBitCount : alData[1];
        bih2.cbImage = (    (   (bih2.cx
                                    * (1 << bih2.cPlanes)
                                    * (1 << bih2.cBitCount)
                                ) + 31
                            ) / 32
                       ) * bih2.cy;

        // create a bit map that is compatible with the display
        return GpiCreateBitmap(hpsMem,
                               &bih2,
                               0,            // do not initialize
                               NULL,         // init data
                               NULL);
    }

    return NULLHANDLE;
}

/*
 *@@ gpihQueryBitmapSize:
 *      returns the width and height of the given bitmap.
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 */

BOOL gpihQueryBitmapSize(HBITMAP hbm,       // in: bitmap handle for query
                         PSIZEL pszl)       // out: size (cx, cy) of bitmap
{
    BITMAPINFOHEADER2 bmi2;
    // query bitmap info
    bmi2.cbFix = sizeof(bmi2);
    if (    (hbm)
         && (GpiQueryBitmapInfoHeader(hbm, &bmi2))
       )
    {
        pszl->cx = bmi2.cx;
        pszl->cy = bmi2.cy;
        return TRUE;
    }

    return FALSE;
}

/*
 *@@ gpihCreateHalftonedBitmap:
 *      this creates a half-toned copy of the
 *      input bitmap by doing the following:
 *
 *      1)  create a new bitmap with the size of hbmSource;
 *      2)  copy hbmSource to the new bitmap (using GpiWCBitBlt);
 *      3)  overpaint every other pixel with lColorGray.
 *
 *      Note that the memory DC is switched to RGB mode, so
 *      lColorGray better be an RGB color.
 *
 *      Note: hbmSource must _not_ be selected into any device
 *      context, or otherwise GpiWCBitBlt will fail.
 *
 *      This returns the new bitmap or NULLHANDLE upon errors.
 *
 *@added V0.9.0
 *@@changed V1.0.1 (2002-11-30) [umoeller]: optimized
 */

HBITMAP gpihCreateHalftonedBitmap(HAB hab,              // in: anchor block
                                  HBITMAP hbmSource,    // in: source bitmap
                                  LONG lColorGray)      // in: color used for gray
{
    HBITMAP hbmReturn = NULLHANDLE;

    HDC     hdcMem;
    HPS     hpsMem;
    SIZEL   szlPage;

    if (gpihQueryBitmapSize(hbmSource, &szlPage))       // V1.0.1 (2002-11-30) [umoeller]
    {
        if (gpihCreateMemPS(hab, &szlPage, &hdcMem, &hpsMem))
        {
            if ((hbmReturn = gpihCreateBitmap(hpsMem,
                                              szlPage.cx,
                                              szlPage.cy)))
            {
                if (GpiSetBitmap(hpsMem, hbmReturn) != HBM_ERROR)
                {
                    POINTL  aptl[4];

                    // step 1: copy bitmap
                    memset(aptl, 0, sizeof(POINTL) * 4);
                    // aptl[0]: target bottom-left, is all 0
                    // aptl[1]: target top-right (inclusive!)
                    aptl[1].x = szlPage.cx - 1;
                    aptl[1].y = szlPage.cy - 1;
                    // aptl[2]: source bottom-left, is all 0

                    // aptl[3]: source top-right (exclusive!)
                    aptl[3].x = szlPage.cx;
                    aptl[3].y = szlPage.cy;
                    GpiWCBitBlt(hpsMem,     // target HPS (bmp selected)
                                hbmSource,
                                4L,             // must always be 4
                                &aptl[0],       // points array
                                ROP_SRCCOPY,
                                BBO_IGNORE);
                                        // doesn't matter here, because we're not stretching

                    // step 2: overpaint bitmap
                    // with half-toned pattern

                    gpihSwitchToRGB(hpsMem);

                    GpiMove(hpsMem, &aptl[0]);  // still 0, 0
                    aptl[0].x = szlPage.cx - 1;
                    aptl[0].y = szlPage.cy - 1;
                    GpiSetColor(hpsMem, lColorGray);
                    GpiSetPattern(hpsMem, PATSYM_HALFTONE);
                    GpiBox(hpsMem,
                           DRO_FILL, // interior only
                           &aptl[0],
                           0, 0);    // no corner rounding

                    // unselect bitmap
                    GpiSetBitmap(hpsMem, NULLHANDLE);
                } // end if (GpiSetBitmap(hpsMem, hbmReturn) != HBM_ERROR)
                else
                {
                    // error selecting bitmap:
                    GpiDeleteBitmap(hbmReturn);
                    hbmReturn = NULLHANDLE;
                }
            } // end if (hbmReturn = gpihCreateBitmap...

            GpiDestroyPS(hpsMem);
            DevCloseDC(hdcMem);
        } // end if (gpihCreateMemPS(hab, &hdcMem, &hpsMem))
    } // end if (hbmSource)

    return hbmReturn;
}

/*
typedef struct _BITMAPARRAYFILEHEADER {
    USHORT               usType;     //  Type of structure.
    ULONG                cbSize;     //  Size of the BITMAPARRAYFILEHEADER structure in bytes.
    ULONG                offNext;    //  Offset of the next BITMAPARRAYFILEHEADER structure from the start of the file.
    USHORT               cxDisplay;  //  Device width, in pels.
    USHORT               cyDisplay;  //  Device height, in pels.
    BITMAPFILEHEADER     bfh;        //  Bitmap file header structure.
} BITMAPARRAYFILEHEADER;

typedef struct _BITMAPARRAYFILEHEADER2 {
    USHORT                usType;     //  Type of structure.
    ULONG                 cbSize;     //  Size of the BITMAPARRAYFILEHEADER2 structure in bytes.
    ULONG                 offNext;    //  Offset of the next BITMAPARRAYFILEHEADER2 structure from the start of the file.
    USHORT                cxDisplay;  //  Device width, in pels.
    USHORT                cyDisplay;  //  Device height, in pels.
    BITMAPFILEHEADER2     bfh2;       //  Bitmap file header structure.
} BITMAPARRAYFILEHEADER2;

These two are binarily the same, except for the file header that is contained.
*/

/*      OLD FORMAT

typedef struct _BITMAPFILEHEADER {
    USHORT               usType;    //  Type of resource the file contains.
    ULONG                cbSize;    //  Size of the BITMAPFILEHEADER structure in bytes.
    SHORT                xHotspot;  //  Width of hotspot for icons and pointers.
    SHORT                yHotspot;  //  Height of hotspot for icons and pointers.
    USHORT               offBits;   //  Offset in bytes.
    BITMAPINFOHEADER     bmp;       //  Bitmap information header structure.

  typedef struct _BITMAPINFOHEADER {
    ULONG      cbFix;      //  Length of structure.
    USHORT     cx;         //  Bitmap width in pels.
    USHORT     cy;         //  Bitmap height in pels.
    USHORT     cPlanes;    //  Number of bit planes.
    USHORT     cBitCount;  //  Number of bits per pel within a plane.
  } BITMAPINFOHEADER;

} BITMAPFILEHEADER;
*/

/*      NEW FORMAT

typedef struct _BITMAPFILEHEADER2 {
    USHORT                usType;    //  Type of resource the file contains.
    ULONG                 cbSize;    //  Size of the BITMAPFILEHEADER2 structure in bytes.
    SHORT                 xHotspot;  //  Width of hotspot for icons and pointers.
    SHORT                 yHotspot;  //  Height of hotspot for icons and pointers.
    USHORT                offBits;   //  Offset in bytes.
    BITMAPINFOHEADER2     bmp2;      //  Bitmap information header structure.

    typedef struct _BITMAPINFOHEADER2 {
      ULONG      cbFix;            //  Length of structure.
      ULONG      cx;               //  Bitmap width in pels.
      ULONG      cy;               //  Bitmap height in pels.
      USHORT     cPlanes;          //  Number of bit planes.
      USHORT     cBitCount;        //  Number of bits per pel within a plane.
      ULONG      ulCompression;    //  Compression scheme used to store the bit map.
      ULONG      cbImage;          //  Length of bitmap storage data, in bytes.
      ULONG      cxResolution;     //  Horizontal component of the resolution of target device.
      ULONG      cyResolution;     //  Vertical component of the resolution of target device.
      ULONG      cclrUsed;         //  Number of color indexes used.
      ULONG      cclrImportant;    //  Minimum number of color indexes for satisfactory appearance of the bit map.
      USHORT     usUnits;          //  Units of measure.
      USHORT     usReserved;       //  Reserved.
      USHORT     usRecording;      //  Recording algorithm.
      USHORT     usRendering;      //  Halftoning algorithm.
      ULONG      cSize1;           //  Size value 1.
      ULONG      cSize2;           //  Size value 2.
      ULONG      ulColorEncoding;  //  Color encoding.
      ULONG      ulIdentifier;     //  Reserved for application use.
    } BITMAPINFOHEADER2;

    Because of the unfortunate replacement of USHORTs with ULONGs for
    cx and cy in the info header, the cx, cy, and cBitCount data is
    NOT the same between old and new formats. Well, IBM, good job.
    And ICONEDIT _still_ writes the old format, too.

} BITMAPFILEHEADER2;

*/

/*
 *@@ gpihCreateBitmapFromFile:
 *      creates a new HBITMAP from the given bitmap data,
 *      which is assumed to be in a standard OS/2 1.3 or
 *      2.0 bitmap file format. That is, it should start
 *      with either a BITMAPFILEHEADER or BITMAPFILEHEADER2
 *      (if single bitmap) or a BITMAPARRAYFILEHEADER or
 *      BITMAPARRAYFILEHEADER2 (if bitmap array) and all
 *      the bitmap bits following. This format is also
 *      used by bitmap resources.
 *
 *      If the file contains only a single bitmap,
 *      this bitmap is used.
 *
 *      If it contains a bitmap array, we use the
 *      "best bitmap" in the array, which is determined
 *      from the following criteria (in this order):
 *
 *      --  a device-dependent bitmap, if its device
 *          resolution is not too large and the given
 *          HPS can display all its colors;
 *
 *      --  a device-dependent bitmap, if its device
 *          resolution is not too large, even if the
 *          given HPS cannot display all its colors;
 *
 *      --  a device-independent bitmap, if the given
 *          HPS can display all its colors;
 *
 *      --  the first device-independent bitmap in
 *          the file;
 *
 *      --  the first bitmap in the file.
 *
 *      Support for bitmap arrays was added with V0.9.19.
 *      I'm not quite sure if the above is the same way
 *      of selecting the "best bitmap" that GpiLoadBitmap
 *      would do, but without any documentation, who is
 *      supposed to know. It appears to me however that
 *      GpiLoadBitmap always does an _exact_ match of
 *      device resolutions (in contrast to this function).
 *      In other words, if you're running at 1152x864
 *      and the device-dependent bitmap is for 1024x768,
 *      _this_ function would use it, while GpiLoadBitmap
 *      might load a device-independent VGA bitmap
 *      instead. (Looks broken, if you ask me.)
 *
 *      Returns:
 *
 *      --  NO_ERROR
 *
 *      --  ERROR_INVALID_DATA: we can't understand this format.
 *
 *@@added V0.9.20 (2002-07-19) [umoeller]
 */

APIRET gpihCreateBitmapFromFile(HBITMAP *phbm,        // out: bitmap if NO_ERROR
                                HPS hps,              // in: HPS for bmp
                                PBYTE pData)          // in: bitmap data
{
    APIRET arc = NO_ERROR;

    PBITMAPFILEHEADER2 pbfh = (PBITMAPFILEHEADER2)pData;

    // check bitmap magic codes
    switch (pbfh->usType)
    {
        case BFT_BMAP:     // "BM"
            // single bitmap in file (no array):
            if (!(*phbm = GpiCreateBitmap(hps,
                                          &pbfh->bmp2,
                                          CBM_INIT,
                                          (PBYTE)pbfh + pbfh->offBits,
                                          (PBITMAPINFO2)&pbfh->bmp2)))
                arc = ERROR_INVALID_DATA;
        break;

        case BFT_BITMAPARRAY:   // "BA"
        {
            // define a handy union for all the above crap
            typedef union
            {
                BITMAPFILEHEADER    Old;
                BITMAPFILEHEADER2   New;
            } BMPUNION, *PBMPUNION;

            PBMPUNION
                    puFirstDI = NULL,   // first device-independent bitmap
                    puBestDI = NULL,    // best device-independent bitmap
                    puFirstDD = NULL,   // first device-dependent bitmap
                    puBestDD = NULL,    // best device-dependent bitmap
                    puFirstAny = NULL,  // first bitmap of any type
                    puUse;

            // get device resolution for this HPS
            // so we can select the "best bitmap"
            #define GET_CAPS_FIRST  CAPS_WIDTH
            #define GET_CAPS_LAST   CAPS_COLOR_BITCOUNT
            #define GET_NO_CAPS     GET_CAPS_LAST - GET_CAPS_FIRST + 1

            LONG alCaps[GET_NO_CAPS];
            PBITMAPARRAYFILEHEADER2 pba = (PBITMAPARRAYFILEHEADER2)pData;

            DevQueryCaps(GpiQueryDevice(hps),
                         GET_CAPS_FIRST,
                         GET_NO_CAPS,
                         alCaps);

            #define BITCOUNT    alCaps[CAPS_COLOR_BITCOUNT - GET_CAPS_FIRST]
            #define WIDTH       alCaps[CAPS_WIDTH - GET_CAPS_FIRST]
            #define HEIGHT      alCaps[CAPS_HEIGHT - GET_CAPS_FIRST]

            // for-all-bitmaps-in-array loop
            while (pba)
            {
                PBMPUNION puThis = (PBMPUNION)&pba->bfh2;

                LONG cx = 0,
                     cy,
                     cBitCount;

                // ignore this if the type isn't "BM"
                if (puThis->Old.usType == BFT_BMAP)
                {
                    // fill the three, but watch out, the offsets are
                    // different between old and new formats
                    if (puThis->Old.bmp.cbFix == sizeof(BITMAPINFOHEADER))
                    {
                        // old format:
                        cx = puThis->Old.bmp.cx;
                        cy = puThis->Old.bmp.cy;
                        cBitCount = puThis->Old.bmp.cBitCount;
                    }
                    else if (puThis->Old.bmp.cbFix == sizeof(BITMAPINFOHEADER2))
                    {
                        // new format:
                        cx = puThis->New.bmp2.cx;
                        cy = puThis->New.bmp2.cy;
                        cBitCount = puThis->New.bmp2.cBitCount;
                    }
                }

                if (cx)
                {
                    // remember the first bitmap from all that we see
                    if (!puFirstAny)
                        puFirstAny = puThis;

                    // check device resolution... device-independent
                    // one has cxDisplay and cyDisplay set to 0
                    if (    (!pba->cxDisplay)
                         && (!pba->cyDisplay)
                       )
                    {
                        // device-independent:

                        // remember the first device-independent bitmap
                        if (!puFirstDI)
                            puFirstDI = puThis;

                        if (cBitCount <= BITCOUNT)
                            // we can display all the colors:
                            puBestDI = puThis;
                    }
                    else
                    {
                        // device-dependent:
                        // ignore if device resolution is too large
                        if (    (pba->cxDisplay <= WIDTH)
                             && (pba->cyDisplay <= HEIGHT)
                           )
                        {
                            // remember first matching device-dependent bitmap
                            if (!puFirstDD)
                                puFirstDD = puThis;

                            if (cBitCount <= BITCOUNT)
                                puBestDD = puThis;
                        }
                    }
                } // end if cx

                // go for next bmp in array
                if (pba->offNext)
                    // another one coming up:
                    // this ofs is from the beginning of the file
                    pba = (PBITMAPARRAYFILEHEADER2)(pData + pba->offNext);
                else
                    // no more bitmaps:
                    break;
            } // end while (pba)

            if (    (puUse = puBestDD)
                 || (puUse = puFirstDD)
                 || (puUse = puBestDI)
                 || (puUse = puFirstDI)
                 || (puUse = puFirstAny)
               )
            {
                PBITMAPINFOHEADER2 pbih2;
                PBYTE pbInitData;

                if (puUse->Old.bmp.cbFix == sizeof(BITMAPINFOHEADER))
                {
                    // old format:
                    pbih2 = (PBITMAPINFOHEADER2)&puUse->Old.bmp;
                    pbInitData = (PBYTE)pData + puUse->Old.offBits;
                }
                else
                {
                    // new format:
                    pbih2 = &puUse->New.bmp2;
                    pbInitData = (PBYTE)pData + puUse->New.offBits;
                }

                if (!(*phbm = GpiCreateBitmap(hps,
                                              pbih2,
                                              CBM_INIT,
                                              pbInitData,
                                              (PBITMAPINFO2)pbih2)))
                    arc = ERROR_INVALID_DATA;
            }
            else
                arc = ERROR_INVALID_DATA;
        }
        break;
    }

    return arc;
}

/*
 *@@ gpihLoadBitmap:
 *      creates a new HBITMAP from the given resource.
 *
 *      This is a replacement for GpiLoadBitmap which handles
 *      device-dependent bitmaps in bitmap arrays in a
 *      non-brain-dead way.
 *
 *      Calls gpihCreateBitmapFromFile for handling the resource
 *      data. See remarks there for how we select a bitmap from
 *      bitmap arrays, which is _different_ from GpiLoadBitmap.
 *
 *      Returns:
 *
 *      --  NO_ERROR: *phbm has received new HBITMAP,
 *          to be freed with GpiDeleteBitmap.
 *
 *      --  ERROR_INVALID_DATA: resource exists, but we
 *          can't understand its format.
 *
 *      plus the error codes from DosGetResource.
 *
 *@@added V0.9.20 (2002-07-19) [umoeller]
 */

APIRET gpihLoadBitmap(HBITMAP *phbm,        // out: bitmap if NO_ERROR
                      HPS hps,              // in: HPS for bmp
                      HMODULE hmodResource, // in: module to load bitmap from
                      ULONG idBitmap)       // in: resource ID for bitmap
{
    APIRET arc;

    PBYTE pbData;

    if (!(arc = DosGetResource(hmodResource,
                               RT_BITMAP,
                               idBitmap,
                               (PVOID*)&pbData)))
    {
        arc = gpihCreateBitmapFromFile(phbm,
                                       hps,
                                       pbData);

        DosFreeResource(pbData);
    }

    return arc;
}

/*
 *@@ gpihLoadBitmapFile:
 *      this loads the specified bitmap file into
 *      the given HPS. Note that the bitmap is _not_
 *      yet selected into the HPS.
 *
 *      Calls gpihCreateBitmapFromFile for handling the resource
 *      data. See remarks there for how we select a bitmap from
 *      bitmap arrays, which is _different_ from GpiLoadBitmap.
 *
 *      Returns:
 *
 *      --  NO_ERROR: *phbm has received new HBITMAP,
 *          to be freed with GpiDeleteBitmap.
 *
 *      --  ERROR_INVALID_PARAMETER
 *
 *      --  ERROR_INVALID_DATA: file exists, but we
 *          can't understand its format.
 *
 *      plus the error codes from doshOpen and DosRead.
 *
 *@@changed V0.9.4 (2000-08-03) [umoeller]: this didn't return NULLHANDLE on errors
 *@@changed V0.9.19 (2002-04-14) [umoeller]: rewritten to support bitmap arrays, prototype changed
 *@@changed V0.9.20 (2002-07-19) [umoeller]: extracted bitmap selection into gpihCreateBitmapFromFile
 */

APIRET gpihLoadBitmapFile(HBITMAP *phbm,        // out: bitmap if NO_ERROR
                          HPS hps,              // in: HPS for bmp
                          PCSZ pcszBmpFile)     // in: bitmap filename
{
    APIRET arc;
    PXFILE pFile;
    ULONG cbFile = 0;

    if (!hps || !pcszBmpFile || !phbm)
        return ERROR_INVALID_PARAMETER;

    if (!(arc = doshOpen(pcszBmpFile,
                         XOPEN_READ_EXISTING | XOPEN_BINARY,
                         &cbFile,
                         &pFile)))
    {
        PBYTE   pData;
        if (!(pData = (PBYTE)malloc(cbFile)))
            arc = ERROR_NOT_ENOUGH_MEMORY;
        else
        {
            // read in the ENTIRE file
            if (!(arc = DosRead(pFile->hf,
                                pData,
                                cbFile,
                                &cbFile)))
                // extracted all the code into this extra func
                // V0.9.20 (2002-07-19) [umoeller]
                arc = gpihCreateBitmapFromFile(phbm,
                                               hps,
                                               (PBYTE)pData);

            free(pData);
        }

        doshClose(&pFile);
    }

    return arc;
}

/*
 *@@ gpihStretchBitmap:
 *      this copies hbmSource to the bitmap selected
 *      into hpsTarget, which must be a memory PS.
 *
 *      The source size is the whole size of hbmSource,
 *      the target size is specified in prclTarget
 *      (which is exclusive, meaning that the top right
 *      corner of that rectangle lies _outside_ the target).
 *
 *      This uses GpiWCBitBlt to stretch the bitmap.
 *      hbmSource therefore must _not_ be selected
 *      into any presentation space, or GpiWCBitBlt will
 *      fail.
 *
 *      If (fPropotional == TRUE), the target size is
 *      modified so that the proportions of the bitmap
 *      are preserved. The bitmap data will then be
 *      copied to a subrectangle of the target bitmap:
 *      there will be extra space either to the left
 *      and right of the bitmap data or to the bottom
 *      and top.
 *      The outside areas of the target bitmap are
 *      not changed then, so you might want to fill
 *      the bitmap with some color first.
 *
 *      This returns the return value of GpiWCBitBlt,
 *      which can be:
 *      --  GPI_OK
 *      --  GPI_HITS: correlate hits
 *      --  GPI_ERROR: error occurred (probably either hbmSource not free
 *                     or no bitmap selected into hpsTarget)
 *
 *@added V0.9.0
 *@@changed V1.0.1 (2002-11-30) [umoeller]: optimized
 */

LONG gpihStretchBitmap(HPS hpsTarget,       // in: memory PS to copy bitmap to
                       HBITMAP hbmSource,   // in: bitmap to be copied into hpsTarget (must be free)
                       PRECTL prclSource,   // in: source rectangle -- if NULL, use size of source bmp
                       PRECTL prclTarget,   // in: target rectangle (required)
                       BOOL fProportional)  // in: preserve proportions when stretching?
{
    SIZEL               szlBmp;
    POINTL              aptl[4];
    BOOL                fCalculated = FALSE;

    if (!gpihQueryBitmapSize(hbmSource, &szlBmp))       // V1.0.1 (2002-11-30) [umoeller]
        return GPI_ERROR;

    memset(aptl, 0, sizeof(POINTL) * 4);

    // aptl[2]: source bottom-left, is all 0
    // aptl[3]: source top-right (exclusive!)
    aptl[3].x = szlBmp.cx;
    aptl[3].y = szlBmp.cy;

    if (fProportional)
    {
        // proportional mode:

        // 1) find out whether cx or cy is too
        // large

        ULONG ulPropSource = (szlBmp.cx * 1000)
                                    / szlBmp.cy;
                // e.g. if the bmp is 200 x 100, we now have 2000
        ULONG ulPropTarget = ((prclTarget->xRight - prclTarget->xLeft) * 1000)
                                    / (prclTarget->yTop - prclTarget->yBottom);
                // case 1: if prclTarget is 300 x 100, we now have 3000 (> ulPropSource)
                // case 2: if prclTarget is 150 x 100, we now have 1500 (< ulPropSource)

        // case 1:
        if (ulPropTarget > ulPropSource)
        {
            // prclTarget is too wide (horizontally):
            // decrease width, keep height

            ULONG cx = (prclTarget->xRight - prclTarget->xLeft);
            ULONG cxNew = (cx * ulPropSource) / ulPropTarget;

            // aptl[0]: target bottom-left
            // move left right (towards center)
            aptl[0].x = prclTarget->xLeft + ((cx - cxNew) / 2);
            aptl[0].y = prclTarget->yBottom;

            // aptl[1]: target top-right (inclusive!)
            aptl[1].x = aptl[0].x + cxNew;
            aptl[1].y = prclTarget->yTop;

            fCalculated = TRUE;
        }
        else
        {
            // prclTarget is too high (vertically):
            // keep width, decrease height

            ULONG cy = (prclTarget->yTop - prclTarget->yBottom);
            ULONG cyNew = (cy * ulPropTarget) / ulPropSource;

            // aptl[0]: target bottom-left
            aptl[0].x = prclTarget->xLeft;
            // move bottom up (towards center)
            aptl[0].y = prclTarget->yBottom + ((cy - cyNew) / 2);

            // aptl[1]: target top-right (inclusive!)
            aptl[1].x = prclTarget->xRight;
            aptl[1].y = aptl[0].y + cyNew;
                    // (prclTarget->yTop * ulPropSource) / ulPropTarget;

            fCalculated = TRUE;
        }
    } // end if (pa->ulFlags & ANF_PROPORTIONAL)

    if (!fCalculated)
    {
        // non-proportional mode or equal proportions:
        // stretch to whole size of prclTarget

        // aptl[0]: target bottom-left
        aptl[0].x = prclTarget->xLeft;
        aptl[0].y = prclTarget->yBottom;
        // aptl[1]: target top-right (inclusive!)
        aptl[1].x = prclTarget->xRight;
        aptl[1].y = prclTarget->yTop;
    }

    return GpiWCBitBlt(hpsTarget,       // target HPS (bmp selected)
                       hbmSource,
                       4L,              // must always be 4
                       &aptl[0],        // points array
                       ROP_SRCCOPY,
                       BBO_IGNORE);
                                // ignore eliminated rows or
                                // columns; useful for color
}

/*
 *@@ gpihDrawPointer:
 *      replacement for WinDrawPointer that can do clipping.
 *
 *      Normally, to do clipping with WinDrawPointer, one
 *      would have to alter the clip rectangle for the current
 *      HPS, which requires creating regions and is thus quite
 *      expensive.
 *
 *      Instead, this function allows for specifying a clip
 *      rectangle directly. It blits the icon bitmaps directly
 *      without calling WinDrawPointer.
 *      Besides, since it uses GpiWCBitBlt, it should probably
 *      work with all types of device contexts.
 *
 *      This also replaces gpihIcon2Bitmap, which wasn't quite
 *      working in the first place and couldn't to clipping
 *      either.
 *
 *      If you don't need clipping and are drawing to the
 *      screen only, this function has no advantage over
 *      WinDrawPointer because it's presumably a bit slower.
 *
 *      Flags presently supported in fl:
 *
 *      --  DP_MINI (not DP_MINIICON, as stated in PMREF):
 *          use mini-icon.
 *
 *      --  DP_HALFTONED (V0.9.20)
 *
 *      Preconditions:
 *
 *      --  The hps is assumed to be in RGB mode.
 *
 *      Post conditions:
 *
 *      --  This uses GpiSet(Back)Color, so the foreground
 *          and background colors are undefined after the
 *          call.
 *
 *@@added V0.9.19 (2002-06-18) [umoeller]
 *@@changed V0.9.20 (2002-07-31) [umoeller]: optimized, saved one GpiQueryBitmapInfoHeader
 *@@changed V0.9.20 (2002-08-04) [umoeller]: added DP_HALFTONED
 *@@changed V1.0.1 (2002-11-30) [umoeller]: optimized
 */

BOOL gpihDrawPointer(HPS hps,           // in: target presentation space
                     LONG x,            // in: lower left target position of icon
                     LONG y,            // in: lower left target position of icon
                     HPOINTER hptr,     // in: icon to be drawn
                     PSIZEL pszlIcon,   // in: icon size (req., should be sysvalues SV_CXICON, SV_CYICON always)
                     PRECTL prclClip,   // in: clip rectangle (inclusive!) or NULL
                     ULONG fl)          // in: DP_* flags
{
    POINTERINFO pi;

    if (    (pszlIcon)
         && (hptr)
         && (WinQueryPointerInfo(hptr, &pi))
       )
    {
        POINTL  aptl[4];
        HBITMAP hbmThis;
        SIZEL   szlAndXor,      // V1.0.1 (2002-11-30) [umoeller]
                szlColor;

        // A HPOINTER really consists of two bitmaps,
        // one monochrome bitmap that has twice the icon
        // height and contains an AND mask in the upper
        // half and an XOR mask in the lower half, and
        // a (probably color) bitmap with the regular
        // icon height.

        // Drawing the icon means (1) blitting the AND
        // mask with ROP_SRCAND, (2) blitting the color
        // bitmap with ROP_SRCPAINT, (3) blitting the
        // XOR mask with ROP_SRCINVERT. Hence the slightly
        // complicated code that follows.

        /*
         * 0)   preparations
         */

        // set up a bunch of variables that are used
        // by the below calculations. We use cx|yIcon
        // to quickly get the system icon dimensions
        // and set up the clip offsets here too,
        // if a clip rectangle is specified.

        LONG    cxIcon = pszlIcon->cx,
                cyIcon = pszlIcon->cy,
                cySrc,
                xRight,
                yTop,
        // clip "rectangle"... this is not really a
        // recangle because it specifies _offsets_
        // towards the center of the icon to be
        // clipped
                lClipLeft = 0,
                lClipBottom = 0,
                lClipRight = 0,
                lClipTop = 0;

        BOOL    fMini;

        if (fMini = !!(fl & DP_MINI))
        {
            cxIcon /= 2;
            cyIcon /= 2;
        }

        // target top right (inclusive)
        xRight = x + cxIcon - 1;
        yTop = y + cyIcon - 1;

        if (prclClip)
        {
            // we have a clip rectangle:
            // set up the clip offsets that are used
            // in both the target and source coordinates
            // for blitting
            if (x < prclClip->xLeft)
                lClipLeft = prclClip->xLeft - x;
            if (xRight > prclClip->xRight)
                lClipRight = xRight - prclClip->xRight;
            if (y < prclClip->yBottom)
                lClipBottom = prclClip->yBottom - y;
            if (yTop > prclClip->yTop)
                lClipTop = yTop - prclClip->yTop;
        }

        // set up target coordinates that are constant
        // for all the three blits

        // aptl[0]: target bottom-left
        aptl[0].x = x + lClipLeft;
        aptl[0].y = y + lClipBottom;

        // aptl[1]: target top-right (inclusive!)
        aptl[1].x = xRight - lClipRight;
        aptl[1].y = yTop - lClipTop;

        if (    (aptl[0].x < aptl[1].x)
             && (aptl[0].y < aptl[1].y)
           )
        {
            LONG    lPatternOld = -1;               // mark as "not changed" for now
            LONG    lAndROP = ROP_SRCAND,           // 0x0088 = 10001000
                    lPaintROP = ROP_SRCPAINT,       // 0x00EE = 11101110
                    lInvertROP = ROP_SRCINVERT;     // 0x0066 = 01100110

            // colors are constant too
            GpiSetColor(hps, RGBCOL_WHITE);
            GpiSetBackColor(hps, RGBCOL_BLACK);

            if (fl & DP_HALFTONED) // V0.9.20 (2002-08-04) [umoeller]
            {
                lPatternOld = GpiQueryPattern(hps);
                GpiSetPattern(hps, PATSYM_HALFTONE);

                lAndROP     = 0x00A8;               //          10101000
                lInvertROP  = 0x00A6;               //          10100110
            }

            /*
             * 1)   work on the AND image
             *      (upper part of the monochrome image)
             */

            if (    (    (fMini)
                      && (hbmThis = pi.hbmMiniPointer)
                    )
                 || (hbmThis = pi.hbmPointer)
               )
            {
                gpihQueryBitmapSize(hbmThis, &szlAndXor);   // V1.0.1 (2002-11-30) [umoeller]

                // use only half the bitmap height
                cySrc = szlAndXor.cy / 2;

                // aptl[2]: source bottom-left
                aptl[2].x =   0
                            + lClipLeft   * szlAndXor.cx / cxIcon;
                aptl[2].y =   cySrc
                            + lClipBottom * cySrc / cyIcon;

                // aptl[3]: source top-right (exclusive!)
                aptl[3].x =   szlAndXor.cx
                            - lClipRight  * szlAndXor.cx / cxIcon;
                aptl[3].y =   szlAndXor.cy
                            - lClipTop    * cySrc / cyIcon;

                GpiWCBitBlt(hps,        // target
                            hbmThis,    // src bmp
                            4L,         // must always be 4
                            aptl,       // point array
                            lAndROP, // ROP_SRCAND,   // source AND target
                            BBO_IGNORE);
            }

            /*
             * 2)   paint the color image; the parts that
             *      are to be transparent are black
             */

            if (    (    (fMini)
                      && (hbmThis = pi.hbmMiniColor)
                    )
                 || (hbmThis = pi.hbmColor)
               )
            {
                gpihQueryBitmapSize(hbmThis, &szlColor);        // V1.0.1 (2002-11-30) [umoeller]

                // aptl[2]: source bottom-left
                aptl[2].x =   0
                            + lClipLeft   * szlColor.cx / cxIcon;
                aptl[2].y =   0
                            + lClipBottom * szlColor.cy / cyIcon;

                // aptl[3]: source top-right (exclusive!)
                aptl[3].x =   szlColor.cx
                            - lClipRight  * szlColor.cx / cxIcon;
                aptl[3].y =   szlColor.cy
                            - lClipTop    * szlColor.cy / cyIcon;

                GpiWCBitBlt(hps,        // target
                            hbmThis,    // src bmp
                            4L,         // must always be 4
                            aptl,       // point array
                            lPaintROP, // ROP_SRCPAINT,
                            BBO_IGNORE);
            }

            /*
             *  3)  work on the XOR image:
             *      (lower part of monochrome bitmap)
             */

            if (    (    (fMini)
                      && (hbmThis = pi.hbmMiniPointer)
                    )
                 || (hbmThis = pi.hbmPointer)
               )
            {
                // we queried the size of this one above V0.9.20 (2002-07-31) [umoeller]

                // use only half the bitmap height
                cySrc = szlAndXor.cy / 2;

                // aptl[2]: source bottom-left
                aptl[2].x =   0
                            + lClipLeft   * szlAndXor.cx / cxIcon;
                aptl[2].y =   0
                            + lClipBottom * cySrc / cyIcon;

                // aptl[3]: source top-right (exclusive!)
                aptl[3].x =   szlAndXor.cx
                            - lClipRight  * szlAndXor.cx / cxIcon;
                aptl[3].y =   cySrc
                            - lClipTop    * cySrc / cyIcon;

                GpiWCBitBlt(hps,        // target
                            hbmThis,    // src bmp
                            4L,         // must always be 4
                            aptl,       // point array
                            lInvertROP, // ROP_SRCINVERT,   // source XOR target
                            BBO_IGNORE);
            }

            // reset old pattern, if changed
            if (lPatternOld != -1)
                GpiSetPattern(hps, lPatternOld);

            return TRUE;
        }
    }

    return FALSE;
}

/*
 *@@category: Helpers\PM helpers\GPI helpers\XBitmaps
 *      Extended bitmaps. See gpihCreateXBitmap for an introduction.
 */

/* ******************************************************************
 *
 *   XBitmap functions
 *
 ********************************************************************/

/*
 *@@ gpihCreateXBitmap:
 *      calls gpihCreateXBitmap2 with cPlanes and cBitCount == 0
 *      for compatibility with exports. Widgets might
 *      have used this func.
 *
 *@@added V0.9.12 (2001-05-20) [umoeller]
 *@@changed V0.9.16 (2001-12-18) [umoeller]: now using optimized gpihCreateXBitmap2
 */

PXBITMAP gpihCreateXBitmap(HAB hab,         // in: anchor block
                           LONG cx,         // in: bitmap width
                           LONG cy)         // in: bitmap height
{
    return gpihCreateXBitmap2(hab,
                              cx,
                              cy,
                              0,
                              0);
}

/*
 *@@ gpihCreateXBitmap:
 *      creates an XBitmap, which is returned in an
 *      XBITMAP structure.
 *
 *      The problem with all the GPI bitmap functions
 *      is that they are quite complex and it is easy
 *      to forget one of the "disassociate" and "deselect"
 *      functions, which then simply leads to enormous
 *      resource leaks in the application.
 *
 *      This function may relieve this a bit. This
 *      creates a memory DC, a memory PS, and a bitmap,
 *      and selects the bitmap into the memory PS.
 *      You can then use any GPI function on the memory
 *      PS to draw into the bitmap. Use the fields from
 *      XBITMAP for that.
 *
 *      The bitmap is created in RGB mode.
 *
 *      Use gpihDestroyXBitmap to destroy the XBitmap
 *      again.
 *
 *      Example:
 *
 +          PXBITMAP pbmp;
 +          if (pbmp = gpihCreateXBitmap(hab, 100, 100))
 +          {
 +              GpiMove(pbmp->hpsMem, ...);
 +              GpiBox(pbmp->hpsMem, ...);
 +
 +              WinDrawBitmap(hpsScreen,
 +                            pbmp->hbm,       // bitmap handle
 +                            ...);
 +              gpihDestroyXBitmap(&pbmp);
 +          }
 *
 *      Without the gpih* functions, the above would expand
 *      to more than 100 lines.
 *
 *@@added V0.9.16 (2001-12-18) [umoeller]
 */

PXBITMAP gpihCreateXBitmap2(HAB hab,         // in: anchor block
                            LONG cx,         // in: bitmap width
                            LONG cy,         // in: bitmap height
                            ULONG cPlanes,     // in: color planes (usually 1); if 0, current screen is used
                            ULONG cBitCount)   // in: either 1, 4, or 24; if 0, current screen value
{
    BOOL fOK = FALSE;
    PXBITMAP pbmp;
    if (pbmp = (PXBITMAP)malloc(sizeof(XBITMAP)))
    {
        memset(pbmp, 0, sizeof(XBITMAP));

        // create memory PS for bitmap
        pbmp->szl.cx = cx;
        pbmp->szl.cy = cy;
        if (gpihCreateMemPS(hab,
                            &pbmp->szl,
                            &pbmp->hdcMem,
                            &pbmp->hpsMem))
        {
            if (cBitCount != 1)
                // not monochrome bitmap:
                gpihSwitchToRGB(pbmp->hpsMem);

            if (pbmp->hbm = gpihCreateBitmap2(pbmp->hpsMem,
                                              cx,
                                              cy,
                                              cPlanes,
                                              cBitCount))
            {
                if (GpiSetBitmap(pbmp->hpsMem,
                                 pbmp->hbm)
                        != HBM_ERROR)
                    fOK = TRUE;
            }
        }

        if (!fOK)
            gpihDestroyXBitmap(&pbmp);
    }

    return pbmp;
}

/*
 *@@ gpihDetachBitmap:
 *      "detaches" the bitmap from the given XBITMAP.
 *      This will deselect the bitmap from the internal
 *      memory PS so it can be used with many OS/2 APIs
 *      that require that the bitmap not be selected
 *      into any memory PS.
 *
 *      Note that it then becomes the responsibility
 *      of the caller to explicitly call GpiDeleteBitmap
 *      because it will not be deleted by gpihDestroyXBitmap.
 *
 *@@added V0.9.16 (2001-12-18) [umoeller]
 */

HBITMAP gpihDetachBitmap(PXBITMAP pbmp)
{
    HBITMAP hbm = pbmp->hbm;
    pbmp->hbm = NULLHANDLE;
    GpiSetBitmap(pbmp->hpsMem, NULLHANDLE);

    return hbm;
}

/*
 *@@ gpihDestroyXBitmap:
 *      destroys an XBitmap created with gpihCreateXBitmap.
 *
 *      To be on the safe side, this sets the
 *      given XBITMAP pointer to NULL as well.
 *
 *@@added V0.9.12 (2001-05-20) [umoeller]
 */

VOID gpihDestroyXBitmap(PXBITMAP *ppbmp)
{
    if (ppbmp)
    {
        PXBITMAP pbmp;
        if (pbmp = *ppbmp)
        {
            if (pbmp->hbm)
            {
                if (pbmp->hpsMem)
                    GpiSetBitmap(pbmp->hpsMem, NULLHANDLE);
                GpiDeleteBitmap(pbmp->hbm);
            }
            if (pbmp->hpsMem)
            {
                GpiAssociate(pbmp->hpsMem, NULLHANDLE);
                GpiDestroyPS(pbmp->hpsMem);
            }
            if (pbmp->hdcMem)
                DevCloseDC(pbmp->hdcMem);

            free(pbmp);

            *ppbmp = NULL;
        }
    }
}

/*
 *@@ gpihCreateBmpFromPS:
 *      this creates a new bitmap and copies a screen rectangle
 *      into it. Consider this a "screen capture" function.
 *
 *      The new bitmap (which is returned) is compatible with the
 *      device associated with hpsScreen. This function calls
 *      gpihCreateMemPS and gpihCreateBitmap to have it created.
 *      The memory PS is only temporary and freed again.
 *
 *      This returns the handle of the new bitmap,
 *      which can then be used for WinDrawBitmap and such, or
 *      NULLHANDLE upon errors.
 *
 *@@changed V0.9.12 (2001-05-20) [umoeller]: fixed excessive mem PS size
 *@@changed V0.9.16 (2001-01-04) [umoeller]: now creating XBITMAP
 */

PXBITMAP gpihCreateBmpFromPS(HAB hab,        // in: anchor block
                             HPS hpsScreen,  // in: screen PS to copy from
                             PRECTL prcl)    // in: rectangle to copy
{

    /* To copy an image from a display screen to a bit map:
      1. Associate the memory device context with a presentation space.
      2. Create a bit map.
      3. Select the bit map into the memory device context by calling GpiSetBitmap.
      4. Determine the location (in device coordinates) of the image.
      5. Call GpiBitBlt and copy the image to the bit map. */

    PXBITMAP pBmp;

    if (pBmp = gpihCreateXBitmap(hab,
                                 prcl->xRight - prcl->xLeft,
                                 prcl->yTop - prcl->yBottom))
    {
        POINTL aptl[3];
        // Copy the screen to the bit map.
        aptl[0].x = 0;              // lower-left corner of destination rectangle
        aptl[0].y = 0;
        aptl[1].x = prcl->xRight;   // upper-right corner for both
        aptl[1].y = prcl->yTop;
        aptl[2].x = prcl->xLeft;    // lower-left corner of source rectangle
        aptl[2].y = prcl->yBottom;

        if (GPI_ERROR == GpiBitBlt(pBmp->hpsMem,
                                   hpsScreen,
                                   sizeof(aptl) / sizeof(POINTL), // Number of points in aptl
                                   aptl,
                                   ROP_SRCCOPY,
                                   BBO_IGNORE))
        {
            // error during bitblt:
            gpihDestroyXBitmap(&pBmp);
        }
    }

    return pBmp;
}

