
/*
 *@@sourcefile fdrview.c:
 *      shared code for all folder views.
 *
 *@@added V1.0.0 (2002-08-28) [umoeller]
 *@@header "filesys\folder.h"
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

#pragma strings(readonly)

/*
 *  Suggested #include order:
 *  1)  os2.h
 *  2)  C library headers
 *  3)  setup.h (code generation and debugging options)
 *  4)  headers in helpers\
 *  5)  at least one SOM implementation header (*.ih)
 *  6)  dlgids.h, headers in shared\ (as needed)
 *  7)  headers in implementation dirs (e.g. filesys\, as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINFRAMEMGR
#define INCL_WINPOINTERS
#define INCL_WININPUT
#define INCL_WINSTDCNR
#define INCL_WINSTDDRAG
#define INCL_WINSHELLDATA
#define INCL_WINSCROLLBARS
#define INCL_WINSYS

#define INCL_GPIBITMAPS
#define INCL_GPIREGIONS
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\winh.h"               // PM helper routines

// SOM headers which don't crash with prec. header files
#include "xfldr.ih"
#include "xfdisk.ih"
#include "xfdataf.ih"

// XWorkplace implementation headers
#include "shared\classtest.h"           // some cheap funcs for WPS class checks
#include "shared\cnrsort.h"             // container sort comparison functions
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)

#include "filesys\folder.h"             // XFolder implementation
#include "filesys\fdrsplit.h"           // folder split views
#include "filesys\fdrviews.h"           // common code for folder views
#include "filesys\object.h"             // XFldObject implementation

// other SOM headers
#pragma hdrstop                         // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Private declarations
 *
 ********************************************************************/

/*
 *@@ IMAGECACHEENTRY:
 *
 *@@added V1.0.0 (2002-08-24) [umoeller]
 */

typedef struct _IMAGECACHEENTRY
{
    WPFileSystem    *pobjImage;     // a WPImageFile really

    HBITMAP         hbm;            // result from WPImageFile::wpQueryBitmapHandle

} IMAGECACHEENTRY, *PIMAGECACHEENTRY;

/*
 *@@ SUBCLCNR:
 *      QWL_USER data for containers that were
 *      subclassed to paint backgrounds.
 *
 *      This gets created and set by fdrMakeCnrPaint.
 *      The view settings are then manipulated
 *      every time the
 *
 *@@added V1.0.0 (2002-08-24) [umoeller]
 */

typedef struct _SUBCLCNR
{
    HWND    hwndCnr;
    PFNWP   pfnwpOrig;

    SIZEL   szlCnr;         // container dimensions, updated with
                            // every WM_WINDOWPOSCHANGED

    LONG    lcolBackground;

    // only if usColorOrBitmap == 0x128, we set the following:
    HBITMAP hbm;            // folder background bitmap
    SIZEL   szlBitmap;      // size of that bitmap

    PIBMFDRBKGND    pBackground;        // pointer to WPFolder instance data
                                        // with the bitmap settings

    /*
    USHORT  usTiledOrScaled;        // 0x132 == normal
                                    // 0x133 == tiled
                                    // 0x134 == scaled
                        // #define BKGND_NORMAL        0x132
                        // #define BKGND_TILED         0x133
                        // #define BKGND_SCALED        0x134
    USHORT  usScaleFactor;          // normally 1
    */

} SUBCLCNR, *PSUBCLCNR;

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

HMTX        G_hmtxImages = NULLHANDLE;
LINKLIST    G_llImages;     // linked list of IMAGECACHEENTRY structs, auto-free

/* ******************************************************************
 *
 *   Image file cache
 *
 ********************************************************************/

/*
 *@@ LockImages:
 *      locks the background image cache.
 */

STATIC BOOL LockImages(VOID)
{
    if (G_hmtxImages)
        return !DosRequestMutexSem(G_hmtxImages, SEM_INDEFINITE_WAIT);

    if (!DosCreateMutexSem(NULL,
                           &G_hmtxImages,
                           0,
                           TRUE))
    {
        lstInit(&G_llImages,
                TRUE);      // auto-free
        return TRUE;
    }

    return FALSE;
}

/*
 *@@ UnlockImages:
 *      unlocks the background image cache.
 */

STATIC VOID UnlockImages(VOID)
{
    DosReleaseMutexSem(G_hmtxImages);
}

/* ******************************************************************
 *
 *   Painting container backgrounds
 *
 ********************************************************************/

/*
 *@@ DrawBitmapClipped:
 *      draws a subrectangle of the given bitmap at
 *      the given coordinates.
 *
 *      The problem with CM_PAINTBACKGROUND is that
 *      we receive an update rectangle, and we MUST
 *      ONLY PAINT IN THAT RECTANGLE, or we'll overpaint
 *      parts of the container viewport that will not
 *      be refreshed otherwise.
 *
 *      At the same time, we must center or tile bitmaps.
 *      This function is a wrapper around WinDrawBitmap
 *      that will paint the given bitmap at the pptlOrigin
 *      position while making sure that only parts in
 *      prclClip will actually be painted.
 *
 *@@added V1.0.0 (2002-08-24) [umoeller]
 */

VOID DrawBitmapClipped(HPS hps,             // in: presentation space
                       HBITMAP hbm,         // in: bitmap to paint
                       PPOINTL pptlOrigin,  // in: position to paint bitmap at
                       PSIZEL pszlBitmap,   // in: bitmap size
                       PRECTL prclClip)     // in: clip (update) rectangle
{
    RECTL   rclSubBitmap;
    POINTL  ptl;

    PMPF_CNRBITMAPS(("pptlOrigin->x: %d, pptlOrigin->y: %d",
            pptlOrigin->x, pptlOrigin->y));

    /*
       -- pathological case:

          cxBitmap = 200
          cyBitmap = 300

          ÚÄpresentation space (cnr)ÄÄÄÄÄÄÄÄÄÄÄÄ¿     500
          ³                                     ³
          ³                                     ³
          ³      ÚÄbitmapÄÄÄÄÄÄÄ¿   ÚrclClip¿   ³     400
          ³      ³              ³   ³       ³   ³
          ³      ³              ³   ³       ³   ³
          ³      ³              ³  350     450  ³     300
          ³      ³              ³   ³       ³   ³
          ³      ³              ³   ³       ³   ³
          ³      100            ³   ÀÄÄÄÄÄÄÄÙ   ³     200
          ³      ³              ³               ³
          ³      ³              ³               ³
          ³     pptlOriginÄ100ÄÄÙ               ³     100
          ³                                     ³
          ³                                     ³
          ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ     0

          0      100    200    300     400     500
    */

    // do nothing if the bitmap is not even in the
    // clip rectangle

    if (    (prclClip->xLeft >= pptlOrigin->x + pszlBitmap->cx)
         || (prclClip->xRight <= pptlOrigin->x)
         || (prclClip->yBottom >= pptlOrigin->y + pszlBitmap->cy)
         || (prclClip->yTop <= pptlOrigin->y)
       )
    {
        PMPF_CNRBITMAPS(("  pathological case, returning"));

        return;
    }

    if (prclClip->xLeft > pptlOrigin->x)
    {
        /*
           -- rclClip is to the right of bitmap origin:

              cxBitmap = 300
              cyBitmap = 300

              ÚÄpresentation space (cnr)ÄÄÄÄÄÄÄÄÄÄÄÄ¿     500
              ³                                     ³
              ³                                     ³
              ³      ÚÄbitmapÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿       ³     400
              ³      ³                      ³       ³
              ³      ³                      ³       ³
              ³      ³             ÚÄrclClipÄÄÄÄ¿   ³     300
              ³      100           300      ³   ³   ³
              ³      ³             ³        ³   ³   ³
              ³      ³             ÀÄÄÄÄÄ200ÅÄÄÄÙ   ³     200
              ³      ³                      ³       ³
              ³      ³                      ³       ³
              ³     pptlOriginÄÄÄ100ÄÄÄÄÄÄÄÄÙ       ³     100
              ³                                     ³
              ³                                     ³
              ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ     0

              0      100    200    300     400     500

              this gives us: rclSubBitmap xLeft   = 300 - 100 = 200
                                          yBottom = 200 - 100 = 100
                                          xRight  = 450 - 100 = 350 --> too wide, but shouldn't matter
                                          yTop    = 300 - 100 = 200

        */

        rclSubBitmap.xLeft =   prclClip->xLeft
                             - pptlOrigin->x;
        rclSubBitmap.xRight =  prclClip->xRight
                             - pptlOrigin->x;
        ptl.x = prclClip->xLeft;
    }
    else
    {
        /*
           -- rclClip is to the left of bitmap origin:

              cxBitmap = 300
              cyBitmap = 300

              ÚÄpresentation space (cnr)ÄÄÄÄÄÄÄÄÄÄÄÄ¿     500
              ³                                     ³
              ³                                     ³
              ³      ÚÄbitmapÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿       ³     400
              ³      ³                      ³       ³
              ³      ³                      ³       ³
              ³  ÚÄrclClipÄÄÄÄÄÄÄÄÄ¿        ³       ³     300
              ³  50  ³            300       ³       ³
              ³  ³   ³             ³        ³       ³
              ³  ÀÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        ³       ³     200
              ³     100                     ³       ³
              ³      ³                      ³       ³
              ³     pptlOriginÄÄÄ100ÄÄÄÄÄÄÄÄÙ       ³     100
              ³                                     ³
              ³                                     ³
              ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ     0

              0      100    200    300     400     500

        */

        rclSubBitmap.xLeft  = 0;
        rclSubBitmap.xRight = prclClip->xRight - pptlOrigin->x;

        ptl.x = pptlOrigin->x;

    }

    // same thing for y
    if (prclClip->yBottom > pptlOrigin->y)
    {

        rclSubBitmap.yBottom =   prclClip->yBottom
                               - pptlOrigin->y;
        rclSubBitmap.yTop    =   prclClip->yTop
                               - pptlOrigin->y;
        ptl.y = prclClip->yBottom;
    }
    else
    {
        rclSubBitmap.yBottom  = 0;
        rclSubBitmap.yTop = prclClip->yTop - pptlOrigin->y;

        ptl.y = pptlOrigin->y;

    }

    if (    (rclSubBitmap.xLeft < rclSubBitmap.xRight)
         && (rclSubBitmap.yBottom < rclSubBitmap.yTop)
       )
        WinDrawBitmap(hps,
                      hbm,
                      &rclSubBitmap,
                      &ptl,
                      0,
                      0,
                      0);
}

/*
 *@@ PaintCnrBackground:
 *      implementation of CM_PAINTBACKGROUND in fnwpSubclCnr.
 *
 *      This is a MAJOR, MAJOR MESS. No wonder noone ever
 *      uses CM_PAINTBACKGROUND.
 *
 *@@added V1.0.0 (2002-08-24) [umoeller]
 */

MRESULT PaintCnrBackground(HWND hwndCnr,
                           POWNERBACKGROUND pob)        // in: mp1 of CM_PAINTBACKGROUND
{
    PSUBCLCNR       pSubCnr;
    PIBMFDRBKGND    pBackground;
    if (    (pSubCnr = (PSUBCLCNR)WinQueryWindowPtr(hwndCnr, QWL_USER))
         && (pBackground = pSubCnr->pBackground)
       )
    {
        BOOL    fSwitched = FALSE;
        POINTL  ptl;

        // we need not paint the background if we
        // have a bitmap AND it should fill the
        // entire folder (avoid flicker)
        if (    (!pSubCnr->hbm)
             || (BKGND_NORMAL == pBackground->BkgndStore.usTiledOrScaled)
           )
        {
            fSwitched = gpihSwitchToRGB(pob->hps);

            WinFillRect(pob->hps,
                        &pob->rclBackground,
                        pSubCnr->lcolBackground);
        }

        if (    (!pSubCnr->hbm)
             || (!pSubCnr->szlBitmap.cx)     // avoid division by zero below
             || (!pSubCnr->szlBitmap.cy)
           )
            // we're done:
            return (MRESULT)TRUE;

        // draw bitmap then
        switch (pBackground->BkgndStore.usTiledOrScaled)
        {

            /*
             * BKGND_NORMAL:
             *      center the bitmap in the container, and
             *      do NOT scale. As opposed to the WPS, we
             *      do not even scale the bitmap if it is
             *      smaller than the container. I have always
             *      thought that is terribly ugly.
             */

            case BKGND_NORMAL:

                // center bitmap
                ptl.x = (pSubCnr->szlCnr.cx - pSubCnr->szlBitmap.cx) / 2;
                ptl.y = (pSubCnr->szlCnr.cy - pSubCnr->szlBitmap.cy) / 2;

                // draw only the parts of the bitmap that are
                // affected by the cnr update rectangle
                DrawBitmapClipped(pob->hps,
                                  pSubCnr->hbm,
                                  &ptl,
                                  &pSubCnr->szlBitmap,
                                  // clip rectangle: cnr
                                  // update rectangle
                                  &pob->rclBackground);
            break;

            /*
             * BKGND_TILED:
             *      draw the bitmap tiled (as many times as
             *      it fits into the container, unscaled).
             *      As opposed to the WPS, again, we don't
             *      even scale the bitmap if it's smaller
             *      than the container.
             */

            case BKGND_TILED:
                // we implement tiling by simply drawing
                // the bitmap in two loops of "rows" and
                // "columns"; however, we start with the
                // first multiple of szlBitmap.cy that is
                // affected by the update rectangle's bottom
                // to speed things up
                ptl.y =   pob->rclBackground.yBottom
                        / pSubCnr->szlBitmap.cy
                        * pSubCnr->szlBitmap.cy;
                while (ptl.y < pob->rclBackground.yTop)
                {
                    // inner loop: the "columns"
                    ptl.x =   pob->rclBackground.xLeft
                            / pSubCnr->szlBitmap.cx
                            * pSubCnr->szlBitmap.cx;
                    while (ptl.x < pob->rclBackground.xRight)
                    {
                        DrawBitmapClipped(pob->hps,
                                          pSubCnr->hbm,
                                          &ptl,
                                          &pSubCnr->szlBitmap,
                                          &pob->rclBackground);

                        ptl.x += pSubCnr->szlBitmap.cx;
                    }

                    ptl.y += pSubCnr->szlBitmap.cy;
                }
            break;

            /*
             * BKGND_SCALED:
             *      scale the bitmap to match the container
             *      size. This is a bit more difficult,
             *      since we can't use WinDrawBitmap, which
             *      only allows us to specify a subrectangle
             *      of the _source_ bitmap. So we have to
             *      use a clip region and bitblt explicitly.
             *      Not blazingly fast, but unless we use
             *      double buffering, there's no other way.
             */

            case BKGND_SCALED:
            {
                // @@todo ignore scaling factor for now
                POINTL  aptl[4];
                HRGN    hrgnOldClip = NULLHANDLE;
                RECTL   rclClip;
                memcpy(&rclClip, &pob->rclBackground, sizeof(RECTL));

                PMPF_CNRBITMAPS(("BKGND_SCALED"));
                PMPF_CNRBITMAPS(("    szlCnr.cx %d, cy %d",
                        pSubCnr->szlCnr.cx, pSubCnr->szlCnr.cy));
                PMPF_CNRBITMAPS(("    rclClip.xLeft %d, yBottom %d, xRight %d, yTop %d",
                        rclClip.xLeft, rclClip.yBottom, rclClip.xRight, rclClip.yTop));

                // reset clip region to "all" to quickly
                // get the old clip region; we must save
                // and restore that, or the cnr will stop
                // painting quickly
                GpiSetClipRegion(pob->hps,
                                 NULLHANDLE,
                                 &hrgnOldClip);        // out: old clip region
                GpiIntersectClipRectangle(pob->hps,
                                          &rclClip);

                memset(aptl, 0, sizeof(aptl));

                // aptl[0]: target bottom-left, is all 0

                // aptl[1]: target top-right (inclusive!)
                aptl[1].x = pSubCnr->szlCnr.cx - 1;
                aptl[1].y = pSubCnr->szlCnr.cy - 1;

                // aptl[2]: source bottom-left, is all 0

                // aptl[3]: source top-right (exclusive!)
                aptl[3].x = pSubCnr->szlBitmap.cx;
                aptl[3].y = pSubCnr->szlBitmap.cy;

                GpiWCBitBlt(pob->hps,
                            pSubCnr->hbm,
                            4L,             // must always be 4
                            &aptl[0],       // points array
                            ROP_SRCCOPY,
                            BBO_IGNORE);

                // restore the old clip region
                GpiSetClipRegion(pob->hps,
                                 hrgnOldClip,       // can be NULLHANDLE
                                 &hrgnOldClip);

                // hrgnOldClip now has the clip region that was
                // created by GpiIntersectClipRectangle, so delete that
                if (hrgnOldClip)
                    GpiDestroyRegion(pob->hps,
                                     hrgnOldClip);
            }
            break;
        }

        return (MRESULT)TRUE;
    }

    return (MRESULT)FALSE;
}

/*
 *@@ fnwpSubclCnr:
 *      window proc that our containers are subclassed with
 *      to implement container background painting. See
 *      PaintCnrBackground.
 *
 *      We have a SUBCLCNR struct in QWL_USER, which was
 *      put there by fdrMakeCnrPaint.
 *
 *@@added V1.0.0 (2002-08-24) [umoeller]
 */

MRESULT EXPENTRY fnwpSubclCnr(HWND hwndCnr, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT     mrc = 0;
    PSUBCLCNR   pSubCnr;

    switch (msg)
    {
        case CM_PAINTBACKGROUND:
            mrc = PaintCnrBackground(hwndCnr, (POWNERBACKGROUND)mp1);
        break;

        case WM_WINDOWPOSCHANGED:
            // when we resize and have a bitmap, repaint the
            // entire container to get everything right
            if (pSubCnr = (PSUBCLCNR)WinQueryWindowPtr(hwndCnr, QWL_USER))
            {
                if (((PSWP)mp1)->fl & SWP_SIZE)
                {
                    pSubCnr->szlCnr.cx = ((PSWP)mp1)->cx;
                    pSubCnr->szlCnr.cy = ((PSWP)mp1)->cy;

                    if (pSubCnr->hbm)
                        WinInvalidateRect(hwndCnr, NULL, TRUE);
                }

                mrc = pSubCnr->pfnwpOrig(hwndCnr, msg, mp1, mp2);
            }
        break;

        case WM_VSCROLL:
        case WM_HSCROLL:
            if (pSubCnr = (PSUBCLCNR)WinQueryWindowPtr(hwndCnr, QWL_USER))
            {
                switch (SHORT2FROMMP(mp2))
                {
                    // case SB_LINEUP:
                    // case SB_LINEDOWN:
                    // case SB_PAGEUP:
                    // case SB_PAGEDOWN:
                    case SB_ENDSCROLL:
                    case SB_SLIDERPOSITION:
                        if (pSubCnr->hbm)
                            WinInvalidateRect(hwndCnr, NULL, TRUE);

                }

                mrc = pSubCnr->pfnwpOrig(hwndCnr, msg, mp1, mp2);
            }
        break;

        case WM_DESTROY:
            if (pSubCnr = (PSUBCLCNR)WinQueryWindowPtr(hwndCnr, QWL_USER))
            {
                mrc = pSubCnr->pfnwpOrig(hwndCnr, msg, mp1, mp2);

                free(pSubCnr);
            }
        break;

        default:
            pSubCnr = (PSUBCLCNR)WinQueryWindowPtr(hwndCnr, QWL_USER);
            mrc = pSubCnr->pfnwpOrig(hwndCnr, msg, mp1, mp2);
        break;
    }

    return mrc;
}

/*
 *@@ fdrvMakeCnrPaint:
 *      subclasses the given cnr with fnwpSubclCnr
 *      to make it process CM_PAINTBACKGROUND.
 *
 *      This creates a SUBCLCNR struct and puts it
 *      into the cnr's QWL_USER.
 *
 *      After this, call fdrvSetCnrLayout to fill
 *      that struct with meaningful bitmap data
 *      for a given folder.
 *
 *@@added V1.0.0 (2002-08-24) [umoeller]
 */

BOOL fdrvMakeCnrPaint(HWND hwndCnr)
{
    PSUBCLCNR   pSubCnr;
    CNRINFO     CnrInfo;

    PMPF_SPLITVIEW(("entering, hwndCnr is 0x%lX", hwndCnr));

    if (    (cnrhQueryCnrInfo(hwndCnr, &CnrInfo))
         && (pSubCnr = NEW(SUBCLCNR))
       )
    {
        PMPF_SPLITVIEW(("   got cnrinfo, subclassing"));

        ZERO(pSubCnr);

        pSubCnr->hwndCnr = hwndCnr;
        WinSetWindowPtr(hwndCnr, QWL_USER, pSubCnr);
        if (pSubCnr->pfnwpOrig = WinSubclassWindow(hwndCnr,
                                                   fnwpSubclCnr))
        {
            CnrInfo.flWindowAttr |= CA_OWNERPAINTBACKGROUND;
            WinSendMsg(hwndCnr,
                       CM_SETCNRINFO,
                       (MPARAM)&CnrInfo,
                       (MPARAM)CMA_FLWINDOWATTR);

            return TRUE;
        }

        free(pSubCnr);
    }

    return FALSE;
}

/*
 *@@ ResolveColor:
 *
 *@@added V1.0.0 (2002-08-24) [umoeller]
 */

LONG ResolveColor(LONG lcol)         // in: explicit color or SYSCLR_* index
{
    // 0x40000000 is a special color value that I have
    // seen in use only by the Desktop itself; I assume
    // this is for getting the desktop icon text color
    if (lcol == 0x40000000)
    {
        CHAR    szTemp[20];
        LONG    lRed, lGreen, lBlue;
        PrfQueryProfileString(HINI_USER,
                              "PM_Colors",
                              "DesktopIconText",
                              "0 0 0",
                              szTemp,
                              sizeof(szTemp));
        sscanf(szTemp, "%d %d %d", &lRed, &lGreen, &lBlue);
        lcol = MAKE_RGB(lRed, lGreen, lBlue);
    }
    // otherwise, if the highest byte is set in the
    // color value, it is really a negative SYSCLR_*
    // index; for backgrounds, this is normally
    // SYSCLR_WINDOW, so check this
    else if (lcol & 0xFF000000)
    {
#ifdef __DEBUG__
        #define DUMPCOL(i) case i: pcsz = # i; break
        PCSZ pcsz = "unknown index";
        switch (lcol)
        {
            DUMPCOL(SYSCLR_SHADOWHILITEBGND);
            DUMPCOL(SYSCLR_SHADOWHILITEFGND);
            DUMPCOL(SYSCLR_SHADOWTEXT);
            DUMPCOL(SYSCLR_ENTRYFIELD);
            DUMPCOL(SYSCLR_MENUDISABLEDTEXT);
            DUMPCOL(SYSCLR_MENUHILITE);
            DUMPCOL(SYSCLR_MENUHILITEBGND);
            DUMPCOL(SYSCLR_PAGEBACKGROUND);
            DUMPCOL(SYSCLR_FIELDBACKGROUND);
            DUMPCOL(SYSCLR_BUTTONLIGHT);
            DUMPCOL(SYSCLR_BUTTONMIDDLE);
            DUMPCOL(SYSCLR_BUTTONDARK);
            DUMPCOL(SYSCLR_BUTTONDEFAULT);
            DUMPCOL(SYSCLR_TITLEBOTTOM);
            DUMPCOL(SYSCLR_SHADOW);
            DUMPCOL(SYSCLR_ICONTEXT);
            DUMPCOL(SYSCLR_DIALOGBACKGROUND);
            DUMPCOL(SYSCLR_HILITEFOREGROUND);
            DUMPCOL(SYSCLR_HILITEBACKGROUND);
            DUMPCOL(SYSCLR_INACTIVETITLETEXTBGND);
            DUMPCOL(SYSCLR_ACTIVETITLETEXTBGND);
            DUMPCOL(SYSCLR_INACTIVETITLETEXT);
            DUMPCOL(SYSCLR_ACTIVETITLETEXT);
            DUMPCOL(SYSCLR_OUTPUTTEXT);
            DUMPCOL(SYSCLR_WINDOWSTATICTEXT);
            DUMPCOL(SYSCLR_SCROLLBAR);
            DUMPCOL(SYSCLR_BACKGROUND);
            DUMPCOL(SYSCLR_ACTIVETITLE);
            DUMPCOL(SYSCLR_INACTIVETITLE);
            DUMPCOL(SYSCLR_MENU);
            DUMPCOL(SYSCLR_WINDOW);
            DUMPCOL(SYSCLR_WINDOWFRAME);
            DUMPCOL(SYSCLR_MENUTEXT);
            DUMPCOL(SYSCLR_WINDOWTEXT);
            DUMPCOL(SYSCLR_TITLETEXT);
            DUMPCOL(SYSCLR_ACTIVEBORDER);
            DUMPCOL(SYSCLR_INACTIVEBORDER);
            DUMPCOL(SYSCLR_APPWORKSPACE);
            DUMPCOL(SYSCLR_HELPBACKGROUND);
            DUMPCOL(SYSCLR_HELPTEXT);
            DUMPCOL(SYSCLR_HELPHILITE);
        }

        PMPF_CNRBITMAPS(("  --> %d (%s)", lcol, pcsz));

#endif

        lcol = WinQuerySysColor(HWND_DESKTOP,
                                lcol,
                                0);
    }

    return lcol;
}

/*
 *@@ fdrvRemoveFromImageCache:
 *      removes the given object from the image cache.
 *
 *@@added V1.0.0 (2002-08-24) [umoeller]
 *@@changed V1.0.1 (2003-01-31) [umoeller]: fixed dangling linklist ptr
 */

BOOL fdrvRemoveFromImageCache(WPObject *pobjImage)
{
    BOOL    brc = FALSE,
            fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = LockImages())
        {
            PLISTNODE pNode = lstQueryFirstNode(&G_llImages);
            while (pNode)
            {
                PLISTNODE pNext = pNode->pNext;     // fixed V1.0.1 (2003-01-29) [umoeller]
                PIMAGECACHEENTRY pice = (PIMAGECACHEENTRY)pNode->pItemData;

                if (pice->pobjImage == pobjImage)
                {
                    // found:

                    // delete the bitmap
                    GpiDeleteBitmap(pice->hbm);

                    lstRemoveNode(&G_llImages,
                                  pNode);       // auto-free
                    brc = TRUE;
                    break;
                }

                pNode = pNext;      // fixed V1.0.1 (2003-01-29) [umoeller]
            }
        }
    }
    CATCH(excpt1)
    {
    }
    END_CATCH();

    if (fLocked)
        UnlockImages();

    return brc;
}

/*
 *@@ GetBitmap:
 *      returns the bitmap handle for the given folder
 *      background structure. This will either be
 *      a bitmap from our image cache or a newly
 *      created one, if the image was not in the
 *      cache.
 *
 *      Returns NULLHANDLE if
 *
 *      --  the folder has no background bitmap,
 *
 *      --  the specified bitmap file does not exist or
 *          is not understood or
 *
 *      --  we're running on Warp 3. We require
 *          the help of the WPImageFile class here.
 *
 *@@added V1.0.0 (2002-08-24) [umoeller]
 */

HBITMAP GetBitmap(PIBMFDRBKGND pBkgnd)
{
    HBITMAP     hbm = NULLHANDLE;
    BOOL        fLocked = FALSE;

    WPDataFile  *pobjLocked = NULL;

    TRY_LOUD(excpt1)
    {
        if (    (pBkgnd->BkgndStore.usColorOrBitmap == BKGND_BITMAP)
             && (pBkgnd->BkgndStore.pszBitmapFile)
           )
        {
            WPObject    *pobjImage;

            // check if WPFolder has found the WPImageFile
            // for us already (that is, if the folder had
            // a legacy open view already)
            if (pobjImage = pBkgnd->pobjImage)
            {
                // yes: check that it's valid
                CHAR szFilename[CCHMAXPATH] = "unknown";
                _wpQueryFilename(pBkgnd->pobjImage, szFilename, TRUE);
                if (stricmp(pBkgnd->BkgndStore.pszBitmapFile, szFilename))
                    cmnLog(__FILE__, __LINE__, __FUNCTION__,
                           "Background bitmap mismatch:\n"
                           "Folder has \"%s\", but image file is from \"%s\"",
                           pBkgnd->BkgndStore.pszBitmapFile,
                           szFilename);
            }
            else
            {
                // find the WPImageFile from the path, but DO NOT
                // set the WPImageFile* pointer in the IBMFDRBKGND
                // struct, or the WPS will go boom
                CHAR    szTemp[CCHMAXPATH];
                if (strchr(pBkgnd->BkgndStore.pszBitmapFile, '\\'))
                    strlcpy(szTemp,
                            pBkgnd->BkgndStore.pszBitmapFile,
                            sizeof(szTemp));
                else
                {
                    // not fully qualified: assume ?:\os2\bitmap then
                    sprintf(szTemp,
                            "?:\\os2\\bitmap\\%s",
                            pBkgnd->BkgndStore.pszBitmapFile);
                }

                if (szTemp[0] == '?')
                    szTemp[0] = doshQueryBootDrive();

                pobjImage = _wpclsQueryObjectFromPath(_WPFileSystem,
                                                      szTemp);
            }

            if (pobjImage)
            {
                PMPF_SPLITVIEW(("   got bitmap [%s]", _wpQueryTitle(pobjImage)));

                // now we have the WPImageFile...
                // in any case, LOCK it so it won't go dormant
                // under our butt
                _wpLockObject(pobjImage);

                // now we need to return a HBITMAP... unfortunately
                // WPImageFile does NO reference counting whatsover
                // when doing _wpQueryBitmapHandle, but will create
                // a new HBITMAP for every call, which is just plain
                // stupid. So that's what we need the image cache
                // for: create only one HBITMAP for every image file,
                // no matter how many times it is used...

                // so check if we can find it in the cache
                if (fLocked = LockImages())
                {
                    PLISTNODE pNode = lstQueryFirstNode(&G_llImages);
                    while (pNode)
                    {
                        PIMAGECACHEENTRY pice = (PIMAGECACHEENTRY)pNode->pItemData;
                        if (pice->pobjImage == pobjImage)
                        {
                            // found:
                            hbm = pice->hbm;

                            // then we've locked it before and can unlock
                            // it once
                            // _wpUnlockObject(pobjImage);

                            break;
                        }

                        pNode = pNode->pNext;
                    }

                    if (!hbm)
                    {
                        // image file was not in cache:
                        // then create a HBITMAP and add a cache entry

                        // added new mutex request V1.0.1 (2003-01-29) [umoeller]
                        if (!_xwpRequestContentMutexSem(pobjImage, SEM_INDEFINITE_WAIT))
                        {
                            pobjLocked = pobjImage;

                            if (hbm = _wpQueryHandleFromContents(pobjImage))
                            {
                                PIMAGECACHEENTRY pice;
                                if (pice = NEW(IMAGECACHEENTRY))
                                {
                                    ZERO(pice);
                                    pice->pobjImage = pobjImage;
                                    pice->hbm = hbm;

                                    lstAppendItem(&G_llImages,
                                                  pice);

                                    // remove from cache when it goes dormant
                                    _xwpModifyFlags(pobjImage,
                                                    OBJLIST_IMAGECACHE,
                                                    OBJLIST_IMAGECACHE);
                                }
                            }
                        }
                    }
                } // end if (fLocked = LockImages())
            } // if (pobjImage)
        } // if (    (pBkgnd->BkgndStore.usColorOrBitmap == BKGND_BITMAP)
    }
    CATCH(excpt1)
    {
    }
    END_CATCH();

    // V1.0.1 (2003-01-29) [umoeller]
    if (pobjLocked)
        _xwpReleaseContentMutexSem(pobjLocked);

    if (fLocked)
        UnlockImages();

    return hbm;
}

/*
 *@@ fdrvSetCnrLayout:
 *      one-shot function for setting the view settings
 *      of the given container according to the instance
 *      settings of the given folder.
 *
 *      This sets the container's background color and
 *      bitmap, if applicable, foreground color and font.
 *      It does not change the container view itself
 *      (that is, details, tree, name, etc.). For that,
 *      call fdrvSetupView, which calls this in turn,
 *      if necessary.
 *
 *      If you want the container to paint folder
 *      background bitmaps properly, subclass the container
 *      using fdrvMakeCnrPaint first.
 *
 *@@added V1.0.0 (2002-08-24) [umoeller]
 */

VOID fdrvSetCnrLayout(HWND hwndCnr,         // in: cnr whose colors and fonts are to be set
                      XFolder *pFolder,     // in: folder that cnr gets filled with
                      ULONG ulView)         // in: one of OPEN_CONTENTS, OPEN_DETAILS, OPEN_TREE
{
    TRY_LOUD(excpt1)
    {
        // try to figure out the background color
        XFolderData     *somThis = XFolderGetData(pFolder);
        PIBMFOLDERDATA  pFdrData;
        PIBMFDRBKGND    pBkgnd;

        PMPF_SPLITVIEW(("entering, hwndCnr is 0x%lX", hwndCnr));

        if (    (pFdrData = (PIBMFOLDERDATA)_pvWPFolderData)
             && (pBkgnd = pFdrData->pCurrentBackground)
           )
        {
            LONG        lcolBack,
                        lcolFore;
            PSUBCLCNR   pSubCnr;

            // 1) background color and bitmap

            PMPF_SPLITVIEW(("   got fdr bkgnd data for [%s]", _wpQueryTitle(pFolder)));

            lcolBack = ResolveColor(pBkgnd->BkgndStore.lcolBackground);

            if (pSubCnr = (PSUBCLCNR)WinQueryWindowPtr(hwndCnr, QWL_USER))
            {
                // container is subclassed:

                pSubCnr->lcolBackground = lcolBack;

                // set the bitmap handle to what the
                // folder wants; this creates a bitmap
                // if necessary and puts it in the
                // image cache
                if (pSubCnr->hbm = GetBitmap(pBkgnd))
                {
                    // folder has a bitmap:
                    BITMAPINFOHEADER2 bih;
                    bih.cbFix = sizeof(bih);
                    GpiQueryBitmapInfoHeader(pSubCnr->hbm,
                                             &bih);
                    pSubCnr->szlBitmap.cx = bih.cx;
                    pSubCnr->szlBitmap.cy = bih.cy;
                }

                pSubCnr->pBackground = pBkgnd;

                /*
                pSubCnr->usTiledOrScaled = pBkgnd->BkgndStore.usTiledOrScaled;
                pSubCnr->usScaleFactor = pBkgnd->BkgndStore.usScaleFactor;
                */
            }
            else
            {
                // container is not subclassed:
                PMPF_SPLITVIEW(("   cnr 0x%lX is not subclassed", hwndCnr));
                winhSetPresColor(hwndCnr,
                                 PP_BACKGROUNDCOLOR,
                                 lcolBack);
            }

            // 2) foreground color

            switch (ulView)
            {
                case OPEN_CONTENTS:
                    lcolFore = pFdrData->LongArray.rgbIconViewTextColColumns;
                break;

                case OPEN_DETAILS:
                    lcolFore = pFdrData->LongArray.rgbDetlViewTextCol;
                break;

                case OPEN_TREE:
                    lcolFore = pFdrData->LongArray.rgbTreeViewTextColIcons;
                break;
            }

            lcolFore = ResolveColor(lcolFore);

            winhSetPresColor(hwndCnr,
                             PP_FOREGROUNDCOLOR,
                             lcolFore);

            // 3) set the font according to the view flag;
            // _wpQueryFldrFont returns a default font
            // properly if there's no instance setting
            // for this view
            winhSetWindowFont(hwndCnr,
                              _wpQueryFldrFont(pFolder, ulView));
        }
    }
    CATCH(excpt1)
    {
    }
    END_CATCH();
}

/*
 *@@ BuildFieldInfos:
 *      sets the FIELDINFO structs on the given
 *      container based on the folder's details
 *      information.
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 */

STATIC BOOL BuildFieldInfos(HWND hwndCnr,
                            WPFolder *pFolder,
                            SOMClass *pDetailsClass)
{
    BOOL        brc = TRUE;
    PCLASSFIELDINFO pcfi = NULL;
    ULONG       cColumns;

    TRY_LOUD(excpt1)
    {
        if (cColumns = _wpclsQueryDetailsInfo(pDetailsClass, &pcfi, NULL))
        {
            PFIELDINFO  pfiFirst,
                        pfiThis;

            // clear out all old fieldinfos
            cnrhClearFieldInfos(hwndCnr,
                                FALSE);     // no invalidate just yet

            // allocate one fieldinfo for each column
            if (pfiFirst = WinSendMsg(hwndCnr,
                                      CM_ALLOCDETAILFIELDINFO,
                                      (MPARAM)cColumns,
                                      0))        // reserved
            {
                ULONG   cbOffset = sizeof(MINIRECORDCORE),
                        ul,
                        cInvisible = 0;
                FIELDINFOINSERT fii;
                CNRINFO ci;

                /*
                column 0 [Icon], CFA_SEP 0, CFA_INVIS 0
                column 1 [Title], CFA_SEP 0, CFA_INVIS 0
                column 2 [Object Title], CFA_SEP 0, CFA_INVIS 0
                column 3 [Object Style], CFA_SEP 0, CFA_INVIS 0
                column 4 [Object Class], CFA_SEP 0, CFA_INVIS 0
                column 5 [Real name], CFA_SEP 0, CFA_INVIS 0
                column 6 [Size], CFA_SEP 0, CFA_INVIS 0
                column 7 [Last write date], CFA_SEP 0, CFA_INVIS 0
                column 8 [Last write time], CFA_SEP 0, CFA_INVIS 0
                column 9 [Last access date], CFA_SEP 0, CFA_INVIS 0
                column 10 [Last access time], CFA_SEP 0, CFA_INVIS 0
                column 11 [Creation date], CFA_SEP 0, CFA_INVIS 0
                column 12 [Creation time], CFA_SEP 0, CFA_INVIS 0
                column 13 [Flags], CFA_SEP 0, CFA_INVIS 0
                column 14 [Read Only Flag], CFA_SEP 0, CFA_INVIS 0
                column 15 [Hidden Flag], CFA_SEP 0, CFA_INVIS 0
                column 16 [System Flag], CFA_SEP 0, CFA_INVIS 0
                column 17 [Directory Flag], CFA_SEP 0, CFA_INVIS 0
                column 18 [Archived Flag], CFA_SEP 0, CFA_INVIS 0
                column 19 [Subject], CFA_SEP 0, CFA_INVIS 0
                column 20 [Comment], CFA_SEP 0, CFA_INVIS 0
                column 21 [Key phrases], CFA_SEP 0, CFA_INVIS 0
                column 22 [History], CFA_SEP 0, CFA_INVIS 0
                column 23 [Extended Attribute Size], CFA_SEP 0, CFA_INVIS 0
                */

                pfiThis = pfiFirst;
                for (ul = 0;
                     ul < cColumns;
                     ul++)
                {
                    PMPF_SPLITVIEW(("column %d [%s], CFA_SEP %d, CFA_INVIS %d",
                                    ul,
                                    (!(pfiThis->flTitle & CFA_BITMAPORICON))
                                        ? pcfi->pTitleData
                                        : "non-string",
                                    (pfiThis->flData & CFA_SEPARATOR),
                                    (pfiThis->flData & CFA_INVISIBLE)
                                  ));

                    // set up each cnr FIELDINFO according to the
                    // CLASSFIELDINFO that the details class gave us
                    pfiThis->cb = sizeof(FIELDINFO);

                    pfiThis->flData = pcfi->flData
                                         | CFA_HORZSEPARATOR        // separator beneath column headings
                                         | CFA_OWNER;               // always owner-draw column
                    // the WPS does not add a separator after "object class",
                    // which is the last column before the separator bar
                    if (ul == 4)
                    {
                        pfiThis->flData &= ~CFA_SEPARATOR;
                        // put separator after this
                        ci.pFieldInfoLast = pfiThis;
                    }
                    else if (ul > 1)
                        pfiThis->flData |= CFA_IGNORE;
                    if (!_wpIsDetailsColumnVisible(pFolder, cInvisible))
                        pfiThis->flData |= CFA_INVISIBLE;

                    pfiThis->flTitle    = pcfi->flTitle;
                    pfiThis->pTitleData = pcfi->pTitleData;

                    if (ul > 1)
                    {
                        pfiThis->offStruct = cbOffset;
                        cbOffset += pcfi->ulLenFieldData;
                    }
                    else
                        pfiThis->offStruct = pcfi->offFieldData;

                    // the WPS uses the userdata field for the owner draw proc,
                    // if this column uses one
                    pfiThis->pUserData  = (PVOID)pcfi->pfnOwnerDraw;
                    pfiThis->cxWidth    = pcfi->cxWidth;

                    pcfi = pcfi->pNextFieldInfo;
                    pfiThis = pfiThis->pNextFieldInfo;

                } // end for

                fii.cb = sizeof(fii);
                fii.pFieldInfoOrder = (PFIELDINFO)CMA_FIRST;
                fii.fInvalidateFieldInfo = TRUE;
                fii.cFieldInfoInsert = cColumns;

                if (WinSendMsg(hwndCnr,
                               CM_INSERTDETAILFIELDINFO,
                               (MPARAM)pfiFirst,
                               (MPARAM)&fii))
                {
                    ci.xVertSplitbar  = 240;
                    brc = (BOOL)WinSendMsg(hwndCnr,
                                           CM_SETCNRINFO,
                                           (MPARAM)&ci,
                                           (MPARAM)(CMA_PFIELDINFOLAST | CMA_XVERTSPLITBAR));
                }
            }
        }
    }
    CATCH(excpt1)
    {
        brc = FALSE;
    } END_CATCH();

    return brc;
}

/*
 *@@ fdrvSetupView:
 *      one-shot function to set up the given container for the
 *      given view.
 *
 *      This
 *
 *      --  switches the container view to Name, Details, or Tree;
 *
 *      --  builds the FIELDINFOs for Details view, if necessary
 *          (that is, if the folder details class changed or Details
 *          view is switched to for the first time);
 *
 *      --  sets the correct sort function on the container, if applicable;
 *
 *      --  calls fdrvSetCnrLayout if necessary.
 *
 *      In order to avoid unnecessary switches, this caches the
 *      current settings in the given CNRVIEW struct. On the
 *      first call, all members must be zeroed except for the
 *      hwndCnr field.
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 */

BOOL fdrvSetupView(PCNRVIEW pCnrView,
                   WPFolder *pFolder,   // in: folder that the cnr displays (for querying view settings)
                                        // or NULL for no change
                   ULONG ulView,        // in: one of OPEN_CONTENTS, OPEN_DETAILS, OPEN_TREE or 0 for no change
                   BOOL fMini)          // in: mini icons? (ignored for OPEN_DETAILS)
{
    ULONG       flNewStyle = 0;
    BOOL        fForceNameSort = FALSE,
                fSetFdrSort = FALSE,
                fSetLayout = FALSE,
                fRefreshDetails = FALSE;
    SOMClass    *pDetailsClass;

    PMPF_SPLITVIEW(("entering, hwndCnr is 0x%lX", pCnrView->hwndCnr));

    // view change?
    if (    (ulView)
         && (ulView != pCnrView->ulView)
       )
    {
        PMPF_SPLITVIEW(("   view changed 0x%lX (%s)",
                    ulView,
                    cmnIdentifyView(ulView)));

        switch (ulView)
        {
            case OPEN_TREE:
                flNewStyle = (fMini)
                                ? CV_TREE | CA_TREELINE | CV_ICON | CV_MINI
                                : CV_TREE | CA_TREELINE | CV_ICON;
                fForceNameSort = TRUE;
            break;

            case OPEN_DETAILS:
                flNewStyle = CV_DETAIL | CV_MINI | CA_DETAILSVIEWTITLES | CA_DRAWICON | CA_OWNERDRAW;
                fSetFdrSort = TRUE;
                fRefreshDetails = TRUE;
            break;

            default:
                flNewStyle = (fMini)
                              ? CV_NAME | CV_FLOW | CV_MINI
                              : CV_NAME | CV_FLOW;
                fSetFdrSort = TRUE;

            break;
        }

        // store the new current view
        pCnrView->ulView = ulView;

        fSetLayout = TRUE;
    }

    if (pFolder)
    {
        if (pFolder != pCnrView->pFolder)
        {
            PMPF_SPLITVIEW(("   folder changed [%s]", _wpQueryTitle(pFolder)));

            // folder changed:
            if (pCnrView->ulView == OPEN_DETAILS)
                fRefreshDetails = TRUE;

            fSetFdrSort = TRUE;
            fSetLayout = TRUE;
        }

        pCnrView->pFolder = pFolder;
    }

    if (fRefreshDetails)
    {
        if (    (pDetailsClass = _wpQueryFldrDetailsClass(pCnrView->pFolder))
             && (pDetailsClass != pCnrView->pDetailsClass)
           )
        {
            // fieldinfos changed for this container:
            pCnrView->pDetailsClass = pDetailsClass;
        }
        else
            fRefreshDetails = FALSE;
    }

    if (    flNewStyle
         || fForceNameSort
         || fSetFdrSort
         || fSetLayout
         || fRefreshDetails
       )
    {
        // avoid flicker; the cnr repaints on
        // every presparam change
        WinEnableWindowUpdate(pCnrView->hwndCnr, FALSE);

        BEGIN_CNRINFO()
        {
            if (flNewStyle)
            {
                CNRINFO ci2;
                cnrhQueryCnrInfo(pCnrView->hwndCnr, &ci2);

                // set only the affected flags, or we'll kill
                // CA_OWNERPAINTBACKGROUND and the like
                cnrhSetView(ci2.flWindowAttr
                            & ~(   CV_TREE | CA_TREELINE | CV_ICON | CV_MINI
                                 | CV_DETAIL | CA_DETAILSVIEWTITLES | CA_DRAWICON | CA_OWNERDRAW
                                 | CV_NAME | CV_FLOW
                               )
                            | flNewStyle);

                cnrhSetTreeIndent(20);
            }

            if (fForceNameSort)     // tree view
            {
                cnrhSetSortFunc(fnCompareName);
            }
            else if (fSetFdrSort)
                fdrSetFldrCnrSort(pCnrView->pFolder,
                                  pCnrView->hwndCnr,
                                  TRUE);        // force

        } END_CNRINFO(pCnrView->hwndCnr);

        if (fSetLayout)
        {
            PMPF_SPLITVIEW(("   calling fdrvSetCnrLayout with folder [%s]",
                                _wpQueryTitle(pCnrView->pFolder)));
            fdrvSetCnrLayout(pCnrView->hwndCnr,
                             pCnrView->pFolder,
                             pCnrView->ulView);
        }

        if (fRefreshDetails)
            BuildFieldInfos(pCnrView->hwndCnr,
                            pCnrView->pFolder,
                            pCnrView->pDetailsClass);

        WinShowWindow(pCnrView->hwndCnr, TRUE);
    }

    return TRUE;
}

/*
 *@@ fdrvCreateFrameWithCnr:
 *      creates a new WC_FRAME with a WC_CONTAINER as its
 *      client window, with hwndParentOwner being the parent
 *      and owner of the frame.
 *
 *      With flCnrStyle, specify the cnr style to use. The
 *      following may optionally be set:
 *
 *      --  CCS_MINIICONS (optionally)
 *
 *      --  CCS_EXTENDSEL: allow zero, one, many icons to be
 *          selected (default WPS style).
 *
 *      --  CCS_SINGLESEL: allow only exactly one icon to be
 *          selected at a time.
 *
 *      --  CCS_MULTIPLESEL: allow zero, one, or more icons
 *          to be selected, and toggle selections (totally
 *          unusable).
 *
 *      WS_VISIBLE, WS_SYNCPAINT, CCS_AUTOPOSITION, and
 *      CCS_MINIRECORDCORE will always be set.
 *
 *      Returns the frame.
 *
 *@@changed V1.0.1 (2002-11-30) [umoeller]: moved this here from fdrsplit.c
 */

HWND fdrvCreateFrameWithCnr(ULONG ulFrameID,
                            HWND hwndParentOwner,     // in: main client window
                            ULONG flCnrStyle,         // in: cnr style
                            HWND *phwndClient)        // out: client window (cnr)
{
    HWND    hwndFrame;
    ULONG   ws =   WS_VISIBLE
                 | WS_SYNCPAINT
                 | CCS_AUTOPOSITION
                 | CCS_MINIRECORDCORE
                 | flCnrStyle;

    if (hwndFrame = winhCreateStdWindow(hwndParentOwner, // parent
                                        NULL,          // pswpFrame
                                        FCF_NOBYTEALIGN,
                                        WS_VISIBLE,
                                        "",
                                        0,             // resources ID
                                        WC_CONTAINER,  // client
                                        ws,            // client style
                                        ulFrameID,
                                        NULL,
                                        phwndClient))
    {
        // set client as owner
        WinSetOwner(hwndFrame, hwndParentOwner);

        winhSetWindowFont(*phwndClient,
                          cmnQueryDefaultFont());
    }

    return hwndFrame;
}

/* ******************************************************************
 *
 *   Populate management
 *
 ********************************************************************/

/*
 *@@ fdrGetFSFromRecord:
 *      returns the WPFileSystem* which is represented
 *      by the specified record.
 *      This resolves shadows and returns root folders
 *      for WPDisk objects cleanly.
 *
 *      Returns NULL
 *
 *      -- if (fFoldersOnly) and precc does not represent
 *         a folder;
 *
 *      -- if (!fFoldersOnly) and precc does not represent
 *         a file-system object;
 *
 *      -- if the WPDisk or WPShadow cannot be resolved.
 *
 *      If fFoldersOnly and the return value is not NULL,
 *      it is guaranteed to be some kind of folder.
 *
 *@@added V0.9.9 (2001-03-11) [umoeller]
 */

WPFileSystem* fdrvGetFSFromRecord(PMINIRECORDCORE precc,
                                  BOOL fFoldersOnly)
{
    WPObject *pobj = NULL;
    if (    (precc)
         && (pobj = OBJECT_FROM_PREC(precc))
         && (pobj = _xwpResolveIfLink(pobj))
       )
    {
        if (_somIsA(pobj, _WPDisk))
            pobj = _XFldDisk    // V1.0.5 (2006-06-10) [pr]: fix crash
                   ? _xwpSafeQueryRootFolder(pobj, FALSE, NULL)
                   : _wpQueryRootFolder(pobj);

        if (pobj)
        {
            if (fFoldersOnly)
            {
                if (!_somIsA(pobj, _WPFolder))
                    pobj = NULL;
            }
            else
                if (!_somIsA(pobj, _WPFileSystem))
                    pobj = NULL;
        }
    }

    return pobj;
}

/*
 *@@ fdrIsInsertable:
 *      checks if pObject can be inserted in a container.
 *
 *      The return value depends on ulInsert:
 *
 *      --  INSERT_ALL (0): we allow all objects to be inserted,
 *          even broken shadows. This is used by the split
 *          view for the files container.
 *
 *          pcszFileMask is ignored in this case.
 *
 *      --  INSERT_FILESYSTEMS (1): this inserts all WPFileSystem and
 *          WPDisk objects plus shadows pointing to them. This
 *          is for the files container in the file dialog,
 *          obviously, because opening abstracts with a file
 *          dialog is impossible (unless the abstract is a
 *          shadow pointing to a file-system object).
 *
 *          For file-system objects, if (pcszFileMask != NULL), the
 *          object's real name is checked against that file mask also.
 *          For example, if (pcszFileMask == *.TXT), this will
 *          return TRUE only if pObject's real name matches
 *          *.TXT.
 *
 *          This is for the right (files) view.
 *
 *      --  INSERT_FOLDERSONLY (2): only real folders are inserted.
 *          We will not even insert disk objects or shadows,
 *          even if they point to shadows. We will also
 *          not insert folder templates.
 *
 *          pcszFileMask is ignored in this case.
 *
 *          This is for the left (drives) view when items
 *          are expanded.
 *
 *          This will NOT resolve shadows because if we insert
 *          shadows of folders into a container, their contents
 *          cannot be inserted a second time. The WPS shares
 *          records so each object can only be inserted once.
 *
 *      --  INSERT_FOLDERSANDDISKS (3): in addition to folders
 *          (as with INSERT_FOLDERSONLY), we allow insertion
 *          of drive objects too.
 *
 *      In any case, FALSE is returned if the object matches
 *      the above, but the object has the "hidden" attribute on.
 */

BOOL fdrvIsInsertable(WPObject *pObject,
                      ULONG ulFoldersOnly,
                      PCSZ pcszFileMask)     // in: upper-case file mask or NULL
{
    if (!pObject)
        return FALSE;       // in any case

    if (ulFoldersOnly > 1)      // INSERT_FOLDERSONLY or INSERT_FOLDERSANDDISKS
    {
        // folders only:
        WPObject *pobj2;

        if (_wpQueryStyle(pObject) & OBJSTYLE_TEMPLATE)
            return FALSE;

        // allow disks with INSERT_FOLDERSANDDISKS only
        if (    (ulFoldersOnly == INSERT_FOLDERSANDDISKS)
             && (    (_somIsA(pObject, _WPDisk))
                  || (    (pobj2 = _xwpResolveIfLink(pObject))
                       && (ctsIsSharedDir(pobj2))
                     )
                )
           )
        {
            // always insert, even if drive not ready
            return TRUE;
        }

        if (_somIsA(pObject, _WPFolder))
        {
            // filter out folder templates and hidden folders
            if (    (!(_wpQueryStyle(pObject) & OBJSTYLE_TEMPLATE))
                 && (!(_wpQueryAttr(pObject) & FILE_HIDDEN))
               )
                return TRUE;
        }

        return FALSE;
    }

    // INSERT_ALL or INSERT_FILESYSTEMS:

    if (ulFoldersOnly == INSERT_ALL)
        return TRUE;

    // INSERT_FILESYSTEMS:

    // disallow broken shadows with INSERT_FILESYSTEMS
    if ((pObject = _xwpResolveIfLink(pObject)))
    {
        if (_somIsA(pObject, _WPDisk))
            return TRUE;

        if (    // filter out non-file systems (shadows pointing to them have been resolved):
                (_somIsA(pObject, _WPFileSystem))
                // filter out hidden objects:
             && (!(_wpQueryAttr(pObject) & FILE_HIDDEN))
           )
        {
            // OK, non-hidden file-system object:
            // regardless of filters, always insert folders
            if (_somIsA(pObject, _WPFolder))
                return TRUE;          // templates too

            if ((pcszFileMask) && (*pcszFileMask))
            {
                // file mask specified:
                CHAR szRealName[CCHMAXPATH];
                if (_wpQueryFilename(pObject,
                                     szRealName,
                                     FALSE))       // not q'fied
                {
                    return doshMatch(pcszFileMask, szRealName);
                }
            }
            else
                // no file mask:
                return TRUE;
        }
    }

    return FALSE;
}

/*
 *@@ IsObjectInCnr:
 *      returns TRUE if pObject has already been
 *      inserted into hwndCnr.
 *
 */

BOOL fdrvIsObjectInCnr(WPObject *pObject,
                       HWND hwndCnr)
{
    BOOL    brc = FALSE;
    BOOL    fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = !_wpRequestObjectMutexSem(pObject, SEM_INDEFINITE_WAIT))
        {
            PUSEITEM    pUseItem = NULL;
            for (pUseItem = _wpFindUseItem(pObject, USAGE_RECORD, NULL);
                 pUseItem;
                 pUseItem = _wpFindUseItem(pObject, USAGE_RECORD, pUseItem))
            {
                // USAGE_RECORD specifies where this object is
                // currently inserted
                PRECORDITEM pRecordItem = (PRECORDITEM)(pUseItem + 1);

                if (hwndCnr == pRecordItem->hwndCnr)
                {
                    brc = TRUE;
                    break;
                }
            }
        }
    }
    CATCH(excpt1)
    {
    } END_CATCH();

    if (fLocked)
        _wpReleaseObjectMutexSem(pObject);

    return brc;
}

/*
 *@@ CLEARCNRDATA:
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 */

typedef struct _CLEARCNRDATA
{
    BOOL        fUnlock;
    LINKLIST    llRecords;                 // linked list of PMINIRECORDCORE structs
} CLEARCNRDATA, *PCLEARCNRDATA;

/*
 *@@ fncbClearCnr:
 *      callback for cnrhForAllRecords, specified
 *      by fdrvClearContainer.
 *
 *@@added V1.0.0 (2002-09-09) [umoeller]
 *@@changed V1.0.1 (2002-11-30) [umoeller]: changed callback param for linklist (speedup)
 */

STATIC ULONG XWPENTRY fncbClearCnr(HWND hwndCnr,
                                   PRECORDCORE precc,
                                   ULONG ulData)      // callback parameter
{
    PCLEARCNRDATA pData = (PCLEARCNRDATA)ulData;

    // WPObject *pobj = OBJECT_FROM_PREC(precc);

    /* _wpCnrRemoveObject(pobj,
                       hwndCnr); */
            // this is _very_ inefficient. Removing 1,000 records
            // can take up to a second this way. Use wpclsRemoveObjects
            // instead.

    // add to list for wpclsRemoveObjects
    lstAppendItem(&pData->llRecords, precc);    // V1.0.1 (2002-11-30) [umoeller]

    if (pData->fUnlock)
        // this is the "unlock" that corresponds to the "lock"
        // that was issued on every object by wpPopulate
        _wpUnlockObject(OBJECT_FROM_PREC(precc));

    return 0;
}

/*
 *@@ fdrvClearContainer:
 *      removes all objects that were inserted into the
 *      specified container and updates the USEITEM's
 *      correctly.
 *
 *      Returns the no. of records that were removed.
 *
 *      flClear can be set to any combination of the
 *      following:
 *
 *      --  Only if CLEARFL_TREEVIEW is set, we will
 *          recurse into subrecords and remove these
 *          also. This makes cleanup a lot faster for
 *          non-tree views.
 *
 *      --  If CLEARFL_UNLOCKOBJECTS is set, we will
 *          unlock every object that we remove once.
 *
 *@@changed V1.0.0 (2002-09-13) [umoeller]: added flClear
 *@@changed V1.0.0 (2002-11-23) [umoeller]: fixed broken lazy drag in progress @@fixes 225
 *@@changed V1.0.1 (2002-11-30) [umoeller]: major speedup @@fixes 270
 */

ULONG fdrvClearContainer(HWND hwndCnr,      // in: cnr to clear
                         ULONG flClear)     // in: CLEARFL_* flags
{
    ULONG       ulrc;
    PDRAGINFO   pdrgInfo;
    CLEARCNRDATA data;
    ULONG       cRecords;

    HPOINTER    hptrOld = winhSetWaitPointer();     // V1.0.1 (2002-11-30) [umoeller]

    // disable window updates
    // for the cnr or this takes forever
    WinEnableWindowUpdate(hwndCnr, FALSE);

    // if this container is the current source of a
    // lazy drag operation, cancel the lazy drag
    // before clearing the container
    if (    (DrgQueryDragStatus() == DGS_LAZYDRAGINPROGRESS)
         && (pdrgInfo = DrgQueryDraginfoPtr(NULL))
         && (pdrgInfo->hwndSource == hwndCnr)
       )
        DrgCancelLazyDrag();

    data.fUnlock = (flClear & CLEARFL_UNLOCKOBJECTS);
    lstInit(&data.llRecords, FALSE);

    // recurse through all records to build a list of
    // records to remove and shoot at them all at once
    // V1.0.1 (2002-11-30) [umoeller]
    ulrc = cnrhForAllRecords(hwndCnr,
                             // recurse only for tree view
                             (flClear & CLEARFL_TREEVIEW)
                                ? NULL
                                : (PRECORDCORE)-1,
                             fncbClearCnr,
                             // callback parameter:
                             (ULONG)&data);        // V1.0.1 (2002-11-30) [umoeller]

    if (cRecords = lstCountItems(&data.llRecords))
    {
        PMINIRECORDCORE *paRecords;
        if (paRecords = (PMINIRECORDCORE*)malloc(cRecords * sizeof(PMINIRECORDCORE)))
        {
            PMINIRECORDCORE *pThis = paRecords;
            PLISTNODE pNode;

            FOR_ALL_NODES(&data.llRecords, pNode)
            {
                *pThis++ = (PMINIRECORDCORE)pNode->pItemData;
            }

            _wpclsRemoveObjects(_WPObject,
                                hwndCnr,
                                (PVOID*)paRecords,
                                cRecords,
                                FALSE);

            free(paRecords);
        }
    }


    lstClear(&data.llRecords);

    WinSetPointer(HWND_DESKTOP, hptrOld);

    WinShowWindow(hwndCnr, TRUE);

    return ulrc;
}


