
/*
 *@@sourcefile gpih.h:
 *      header file for gpih.c (GPI helper functions). See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #define INCL_GPILOGCOLORTABLE       // for some funcs
 *@@include #include <os2.h>
 *@@include #include "helpers\gpih.h"
 */

/*
 *      Copyright (C) 1997-2000 Ulrich M”ller.
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

#ifndef GPIH_HEADER_INCLUDED
    #define GPIH_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Rectangle helpers
     *
     ********************************************************************/

    BOOL gpihIsPointInRect(PRECTL prcl,
                           LONG x,
                           LONG y);

    VOID gpihInflateRect(PRECTL prcl,
                         LONG l);

    /* ******************************************************************
     *
     *   Device helpers
     *
     ********************************************************************/

    ULONG gpihQueryDisplayCaps(ULONG ulIndex);

    /* ******************************************************************
     *
     *   Color helpers
     *
     ********************************************************************/

    // common RGB colors
    #define RGBCOL_BLACK            0x00000000
    #define RGBCOL_WHITE            0x00FFFFFF

    #define RGBCOL_RED              0x00FF0000
    #define RGBCOL_PINK             0x00FF00FF
    #define RGBCOL_BLUE             0x000000FF
    #define RGBCOL_CYAN             0x0000FFFF
    #define RGBCOL_GREEN            0x0000FF00
    #define RGBCOL_YELLOW           0x00FFFF00
    #define RGBCOL_GRAY             0x00CCCCCC

    #define RGBCOL_DARKRED          0x00800000
    #define RGBCOL_DARKPINK         0x00800080
    #define RGBCOL_DARKBLUE         0x00000080
    #define RGBCOL_DARKCYAN         0x00008080
    #define RGBCOL_DARKGREEN        0x00008000
    #define RGBCOL_DARKYELLOW       0x00808000
    #define RGBCOL_DARKGRAY         0x00808080

    /*
     *@@ GET_BLUE:
     *      gets the BLUE (first) byte from a
     *      LONG RGB value.
     *
     *@@added V0.9.14 (2001-08-03) [umoeller]
     */

    #define GET_BLUE(lcol)  *( ((PBYTE)(&(lcol))) )

    /*
     *@@ GET_GREEN:
     *      gets the GREEN (second) byte from a
     *      LONG RGB value.
     *
     *@@added V0.9.14 (2001-08-03) [umoeller]
     */

    #define GET_GREEN(lcol) *( ((PBYTE)(&(lcol))) + 1 )

    /*
     *@@ GET_RED:
     *      gets the RED (third) byte from a
     *      LONG RGB value.
     *
     *@@added V0.9.14 (2001-08-03) [umoeller]
     */

    #define GET_RED(lcol)   *( ((PBYTE)(&(lcol))) + 2 )

    /*
     *@@ MAKE_RGB:
     *      composes a LONG color value from
     *      three BYTE values for red, green,
     *      and blue.
     *
     *@@added V0.9.14 (2001-08-03) [umoeller]
     */

    #define MAKE_RGB(r, g, b) (LONG)((BYTE)(b)) + (((LONG)((BYTE)(g))) << 8) + (((LONG)((BYTE)(r))) << 16)

    VOID XWPENTRY gpihManipulateRGB(PLONG plColor, double dFactor);
    typedef VOID XWPENTRY GPIHMANIPULATERGB(PLONG plColor, double dFactor);
    typedef GPIHMANIPULATERGB *PGPIHMANIPULATERGB;

    LONG XWPENTRY gpihMediumRGB(LONG lcol1, LONG lcol2);
    typedef LONG XWPENTRY GPIHMEDIUMRGB(LONG lcol1, LONG lcol2);
    typedef GPIHMEDIUMRGB *PGPIHMEDIUMRGB;

    BOOL XWPENTRY gpihSwitchToRGB(HPS hps);
    typedef BOOL XWPENTRY GPIHSWITCHTORGB(HPS hps);
    typedef GPIHSWITCHTORGB *PGPIHSWITCHTORGB;

    /* ******************************************************************
     *
     *   Drawing primitives helpers
     *
     ********************************************************************/

    VOID XWPENTRY gpihDrawRect(HPS hps, PRECTL prcl);
    typedef VOID XWPENTRY GPIHDRAWRECT(HPS hps, PRECTL prcl);
    typedef GPIHDRAWRECT *PGPIHDRAWRECT;

    VOID XWPENTRY gpihBox(HPS hps, LONG lControl, PRECTL prcl);
    typedef VOID XWPENTRY GPIHBOX(HPS hps, LONG lControl, PRECTL prcl);
    typedef GPIHBOX *PGPIHBOX;

    VOID XWPENTRY gpihMarker(HPS hps, LONG x, LONG y, ULONG ulWidth);
    typedef VOID XWPENTRY GPIHMARKER(HPS hps, LONG x, LONG y, ULONG ulWidth);
    typedef GPIHMARKER *PGPIHMARKER;

    VOID XWPENTRY gpihDrawThickFrame(HPS hps, PRECTL prcl, ULONG ulWidth);
    typedef VOID XWPENTRY GPIHDRAWTHICKFRAME(HPS hps, PRECTL prcl, ULONG ulWidth);
    typedef GPIHDRAWTHICKFRAME *PGPIHDRAWTHICKFRAME;

    VOID XWPENTRY gpihDraw3DFrame2(HPS hps,
                                   PRECTL prcl,
                                   USHORT usWidth,
                                   LONG lColorLeft,
                                   LONG lColorRight);
    typedef VOID XWPENTRY GPIHDRAW3DFRAME2(HPS hps,
                                           PRECTL prcl,
                                           USHORT usWidth,
                                           LONG lColorLeft,
                                           LONG lColorRight);
    typedef GPIHDRAW3DFRAME2 *PGPIHDRAW3DFRAME2;

    VOID XWPENTRY gpihDraw3DFrame(HPS hps,
                                  PRECTL prcl,
                                  USHORT usWidth,
                                  LONG lColorLeft,
                                  LONG lColorRight);
    typedef VOID XWPENTRY GPIHDRAW3DFRAME(HPS hps,
                                          PRECTL prcl,
                                          USHORT usWidth,
                                          LONG lColorLeft,
                                          LONG lColorRight);
    typedef GPIHDRAW3DFRAME *PGPIHDRAW3DFRAME;

    LONG XWPENTRY gpihCharStringPosAt(HPS hps,
                                      PPOINTL pptlStart,
                                      PRECTL prclRect,
                                      ULONG flOptions,
                                      LONG lCount,
                                      PCH pchString);
    typedef LONG XWPENTRY GPIHCHARSTRINGPOSAT(HPS hps,
                                              PPOINTL pptlStart,
                                              PRECTL prclRect,
                                              ULONG flOptions,
                                              LONG lCount,
                                              PCH pchString);
    typedef GPIHCHARSTRINGPOSAT *PGPIHCHARSTRINGPOSAT;

    /*
     *@@ BKGNDINFO:
     *      background information passed into
     *      gpihFillBackground.
     *
     *      The color flags apply always. The
     *      bitmap flags apply only if hbm is
     *      not NULLHANDLE.
     *
     *@@added V0.9.19 (2002-05-07) [umoeller]
     */

    typedef struct _BKGNDINFO
    {
        ULONG       flPaintMode;
                // a combination of one of the color
                // flags and one of the bitmap flags:
                // a) color flags
                #define PMOD_COLORMASK      0x000F
                #define PMOD_SOLID          0x0000  // solid color, use lcol1
                #define PMOD_TOPBOTTOM      0x0001  // spectrum, lcol1 = top, lcol2 = bottom
                #define PMOD_LEFTRIGHT      0x0002  // spectrum, lcol1 = left, lcol2 = right
                // b) bitmap flags
                #define PMOD_BMPMASK        0x00F0
                #define PMOD_BMP_CENTER     0x0000  // center the bitmap
                #define PMOD_BMP_SCALE_X    0x0002  // scale the bitmap to size in prcl,
                                                    // respect lScale
                #define PMOD_BMP_TILE       0x0004  // tile the bitmap

        LONG        lcol1,              // color 1 (RGB)
                    lcol2;              // color 2 (RGB)

        HBITMAP     hbm;                // bitmap or NULLHANDLE if none
        ULONG       lScale;             // only with PMOD_BMP_SCALE_X: how many times
                                        // the bmp should be scaled (as with WPS
                                        // folder "Background" page); this cannot
                                        // be zero then

    } BKGNDINFO, *PBKGNDINFO;

    VOID gpihFillBackground(HPS hps,
                            PRECTL prcl,
                            PBKGNDINFO pInfo);

    /* ******************************************************************
     *
     *   Font helpers
     *
     ********************************************************************/

    BOOL XWPENTRY gpihMatchFont(HPS hps,
                                LONG lSize,
                                BOOL fFamily,
                                const char *pcszName,
                                USHORT usFormat,
                                FATTRS *pfa,
                                PFONTMETRICS pFontMetrics);
    typedef BOOL XWPENTRY GPIHMATCHFONT(HPS hps,
                                        LONG lSize,
                                        BOOL fFamily,
                                        const char *pcszName,
                                        USHORT usFormat,
                                        FATTRS *pfa,
                                        PFONTMETRICS pFontMetrics);
    typedef GPIHMATCHFONT *PGPIHMATCHFONT;

    BOOL XWPENTRY gpihSplitPresFont(PSZ pszFontNameSize,
                                    PULONG pulSize,
                                    PSZ *ppszFaceName);
    typedef BOOL XWPENTRY GPIHSPLITPRESFONT(PSZ pszFontNameSize,
                                            PULONG pulSize,
                                            PSZ *ppszFaceName);
    typedef GPIHSPLITPRESFONT *PGPIHSPLITPRESFONT;

    BOOL XWPENTRY gpihLockLCIDs(VOID);
    typedef BOOL XWPENTRY GPIHLOCKLCIDS(VOID);
    typedef GPIHLOCKLCIDS *PGPIHLOCKLCIDS;

    VOID XWPENTRY gpihUnlockLCIDs(VOID);
    typedef VOID XWPENTRY GPIHUNLOCKLCIDS(VOID);
    typedef GPIHUNLOCKLCIDS *PGPIHUNLOCKLCIDS;

    LONG XWPENTRY gpihCreateFont(HPS hps, FATTRS *pfa);
    typedef LONG XWPENTRY GPIHCREATEFONT(HPS hps, FATTRS *pfa);
    typedef GPIHCREATEFONT *PGPIHCREATEFONT;

    LONG XWPENTRY gpihFindFont(HPS hps,
                               LONG lSize,
                               BOOL fFamily,
                               const char *pcszName,
                               USHORT usFormat,
                               PFONTMETRICS pFontMetrics);
    typedef LONG XWPENTRY GPIHFINDFONT(HPS hps,
                                       LONG lSize,
                                       BOOL fFamily,
                                       const char *pcszName,
                                       USHORT usFormat,
                                       PFONTMETRICS pFontMetrics);
    typedef GPIHFINDFONT *PGPIHFINDFONT;

    LONG XWPENTRY gpihFindPresFont(HWND hwnd,
                                   BOOL fInherit,
                                   HPS hps,
                                   const char *pcszDefaultFont,
                                   PFONTMETRICS pFontMetrics,
                                   PLONG plSize);
    typedef LONG XWPENTRY GPIHFINDPRESFONT(HWND hwnd,
                                           BOOL fInherit,
                                           HPS hps,
                                           const char *pcszDefaultFont,
                                           PFONTMETRICS pFontMetrics,
                                           PLONG plSize);
    typedef GPIHFINDPRESFONT *PGPIHFINDPRESFONT;

    BOOL XWPENTRY gpihSetPointSize(HPS hps, LONG lPointSize);
    typedef BOOL XWPENTRY GPIHSETPOINTSIZE(HPS hps, LONG lPointSize);
    typedef GPIHSETPOINTSIZE *PGPIHSETPOINTSIZE;

    LONG XWPENTRY gpihQueryLineSpacing(HPS hps);
    typedef LONG XWPENTRY GPIHQUERYLINESPACING(HPS hps);
    typedef GPIHQUERYLINESPACING *PGPIHQUERYLINESPACING;

    /* ******************************************************************
     *
     *   Bitmap helpers
     *
     ********************************************************************/

    BOOL XWPENTRY gpihCreateMemPS(HAB hab, PSIZEL psizlPage, HDC *hdcMem, HPS *hpsMem);
    typedef BOOL XWPENTRY GPIHCREATEMEMPS(HAB hab, PSIZEL psizlPage, HDC *hdcMem, HPS *hpsMem);
    typedef GPIHCREATEMEMPS *PGPIHCREATEMEMPS;

    HBITMAP XWPENTRY gpihCreateBitmap(HPS hpsMem, ULONG  cx, ULONG cy);
    typedef HBITMAP XWPENTRY GPIHCREATEBITMAP(HPS hpsMem, ULONG  cx, ULONG cy);
    typedef GPIHCREATEBITMAP *PGPIHCREATEBITMAP;

    HBITMAP XWPENTRY gpihCreateBitmap2(HPS hpsMem, ULONG  cx, ULONG cy, ULONG cPlanes, ULONG cBitCount);

    HBITMAP XWPENTRY gpihCreateHalftonedBitmap(HAB hab, HBITMAP hbmSource, LONG lColorGray);
    typedef HBITMAP XWPENTRY GPIHCREATEHALFTONEDBITMAP(HAB hab, HBITMAP hbmSource, LONG lColorGray);
    typedef GPIHCREATEHALFTONEDBITMAP *PGPIHCREATEHALFTONEDBITMAP;

    APIRET gpihLoadBitmap(HBITMAP *phbm,
                          HPS hps,
                          HMODULE hmodResource,
                          ULONG idBitmap);

    APIRET XWPENTRY gpihLoadBitmapFile(HBITMAP *phbm, HPS hps, PCSZ pcszBmpFile);
    typedef APIRET XWPENTRY GPIHLOADBITMAPFILE(HBITMAP *phbm, HPS hps, PCSZ pcszBmpFile);
    typedef GPIHLOADBITMAPFILE *PGPIHLOADBITMAPFILE;

    LONG XWPENTRY gpihStretchBitmap(HPS hpsTarget,
                                    HBITMAP hbmSource,
                                    PRECTL prclSource,
                                    PRECTL prclTarget,
                                    BOOL fProportional);
    typedef LONG XWPENTRY GPIHSTRETCHBITMAP(HPS hpsTarget,
                                            HBITMAP hbmSource,
                                            PRECTL prclSource,
                                            PRECTL prclTarget,
                                            BOOL fProportional);
    typedef GPIHSTRETCHBITMAP *PGPIHSTRETCHBITMAP;

    /* BOOL XWPENTRY gpihIcon2Bitmap(HPS hpsMem,
                                  HPOINTER hptr,
                                  LONG lBkgndColor,
                                  PPOINTL pptlLowerLeft,
                                  ULONG ulIconSize); */

    BOOL XWPENTRY gpihDrawPointer(HPS hps,
                                  LONG x,
                                  LONG y,
                                  HPOINTER hptr,
                                  PSIZEL pszlIcon,
                                  PRECTL prclClip,
                                  ULONG fl);
    typedef BOOL XWPENTRY GPIHDRAWPOINTER(HPS hps,
                                          LONG x,
                                          LONG y,
                                          HPOINTER hptr,
                                          PSIZEL pszlIcon,
                                          PRECTL prclClip,
                                          ULONG fl);
    typedef GPIHDRAWPOINTER *PGPIHDRAWPOINTER;

    /* ******************************************************************
     *
     *   XBitmap functions
     *
     ********************************************************************/

    /*
     *@@ XBITMAP:
     *      representation of an XBitmap, which is created
     *      with gpihCreateXBitmap and destroyed with
     *      gpihDestroyXBitmap.
     *
     *      An XBitmap is essentially a set of a memory
     *      device context, a memory presentation space,
     *      and a bitmap which is selected into that
     *      HPS.
     *
     *@@added V0.9.12 (2001-05-20) [umoeller]
     */

    typedef struct _XBITMAP
    {
        HDC     hdcMem;     // memory DC
        HPS     hpsMem;     // memory PS
        HBITMAP hbm;        // bitmap handle
        SIZEL   szl;        // size of bitmap
    } XBITMAP, *PXBITMAP;

    PXBITMAP XWPENTRY gpihCreateXBitmap(HAB hab, LONG cx, LONG cy);
    typedef PXBITMAP XWPENTRY GPIHCREATEXBITMAP(HAB hab, LONG cx, LONG cy);
    typedef GPIHCREATEXBITMAP *PGPIHCREATEXBITMAP;

    PXBITMAP gpihCreateXBitmap2(HAB hab,
                                LONG cx,
                                LONG cy,
                                ULONG cPlanes,
                                ULONG cBitCount);

    HBITMAP XWPENTRY gpihDetachBitmap(PXBITMAP pbmp);

    VOID XWPENTRY gpihDestroyXBitmap(PXBITMAP *ppbmp);
    typedef VOID XWPENTRY GPIHDESTROYXBITMAP(PXBITMAP *ppbmp);
    typedef GPIHDESTROYXBITMAP *PGPIHDESTROYXBITMAP;

    PXBITMAP gpihCreateBmpFromPS(HAB hab,
                                 HPS hpsScreen,
                                 PRECTL prcl);

#endif

#if __cplusplus
}
#endif

