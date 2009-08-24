/*    
	DisplayGraphics.c	2.46
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

 */

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "windows.h"

#include "GdiObjects.h"
#include "GdiDC.h"
#include "Log.h"
#include "Driver.h"
#include "DeviceData.h"

extern BOOL CreateDIBImage(LPIMAGEINFO,BITMAPINFOHEADER *,
	RGBQUAD *,LPVOID,UINT);
extern BOOL TWIN_IsColorInPalette(HPALETTE,COLORREF);
extern COLORREF TWIN_PaletteRGB(HDC32,COLORREF);
BOOL GdiSelectBrush(HDC32,LPBRUSHINFO);

void DisplayValidate(HDC32,LPLSDS_PARAMS,DWORD);

static void FixCoordinates(LPRECT);

DWORD
lsd_display_moveto(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    return DRVCALL_GRAPHICS(PGH_MOVETO,hDC32->lpDrvData,dwParam,lpStruct);
}

DWORD
lsd_display_lineto(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    if (hDC32->dwInvalid & (IM_PENMASK|IM_ROP2MASK))
	DisplayValidate(hDC32, lpStruct, IM_PENMASK|IM_ROP2MASK);

    return DRVCALL_GRAPHICS(PGH_LINETO,hDC32->lpDrvData,dwParam,lpStruct);
}

DWORD
lsd_display_polyline(WORD msg,HDC32 hDC32,DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    if (hDC32->dwInvalid & (IM_PENMASK|IM_ROP2MASK))
	DisplayValidate(hDC32, lpStruct, IM_PENMASK|IM_ROP2MASK);

    return DRVCALL_GRAPHICS(PGH_POLYLINE,hDC32->lpDrvData,dwParam,lpStruct);
}

DWORD
lsd_display_rectangle(WORD msg,HDC32 hDC32,DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    FixCoordinates(&lpStruct->lsde.rect);

    if (hDC32->dwInvalid & (IM_PENMASK|IM_BRUSHMASK|IM_ROP2MASK))
	DisplayValidate(hDC32, lpStruct,
		IM_PENMASK|IM_BRUSHMASK|IM_ROP2MASK);

    return DRVCALL_GRAPHICS(PGH_RECTANGLE,hDC32->lpDrvData,0L,lpStruct);
}

DWORD
lsd_display_roundrect(WORD msg,HDC32 hDC32,DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    FixCoordinates(&lpStruct->lsde.rect);

    if (hDC32->dwInvalid & (IM_PENMASK|IM_BRUSHMASK|IM_ROP2MASK))
	DisplayValidate(hDC32, lpStruct, IM_PENMASK|IM_BRUSHMASK|IM_ROP2MASK);

    return DRVCALL_GRAPHICS(PGH_ROUNDRECT,hDC32->lpDrvData,0L,lpStruct);
}

DWORD
lsd_display_ellipse(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    FixCoordinates(&lpStruct->lsde.rect);

    if (hDC32->dwInvalid & (IM_PENMASK|IM_BRUSHMASK|IM_ROP2MASK))
	DisplayValidate(hDC32, lpStruct, IM_PENMASK|IM_BRUSHMASK|IM_ROP2MASK);

    return DRVCALL_GRAPHICS(PGH_ELLIPSE,hDC32->lpDrvData,0L,lpStruct);
}

DWORD
lsd_display_arc(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    FixCoordinates(&lpStruct->lsde.rect);

    if (hDC32->dwInvalid & (IM_PENMASK|IM_BRUSHMASK|IM_ROP2MASK))
	DisplayValidate(hDC32, lpStruct, IM_PENMASK|IM_BRUSHMASK|IM_ROP2MASK);

    return DRVCALL_GRAPHICS(PGH_ARC,hDC32->lpDrvData,(DWORD)msg,lpStruct);
}

DWORD
lsd_display_polygon(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    if (hDC32->dwInvalid & (IM_PENMASK|IM_BRUSHMASK|
				IM_POLYFILLMODEMASK|IM_ROP2MASK))
	DisplayValidate(hDC32, lpStruct,
		IM_PENMASK|IM_BRUSHMASK|IM_POLYFILLMODEMASK|IM_ROP2MASK);

    return DRVCALL_GRAPHICS(PGH_POLYGON,hDC32->lpDrvData,dwParam,lpStruct);
}

DWORD
lsd_display_polypolygon(WORD msg,HDC32 hDC32,DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    if (hDC32->dwInvalid & (IM_PENMASK|IM_BRUSHMASK|
				IM_POLYFILLMODEMASK|IM_ROP2MASK))
	DisplayValidate(hDC32, lpStruct,
		IM_PENMASK|IM_BRUSHMASK|IM_POLYFILLMODEMASK|IM_ROP2MASK);

    return DRVCALL_GRAPHICS(PGH_POLYPOLYGON,hDC32->lpDrvData,dwParam,lpStruct);
}

DWORD
lsd_display_extfloodfill(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    HDC hDC = GETHDC16(hDC32);
    RECT rc;

    if (hDC32->dwInvalid & (IM_BRUSHMASK|IM_ROP2MASK))
	DisplayValidate(hDC32, lpStruct, IM_BRUSHMASK|IM_ROP2MASK);

    GetClipBox(hDC, &rc);
    LPtoDP(hDC,(LPPOINT)&rc,2);
    /* For child windows, LPtoDP does not take into account DOx/DOy; */
    /* we need here actual position in the physical GC -- Lana K. */
    OffsetRect(&rc,hDC32->DOx,hDC32->DOy);

    return DRVCALL_GRAPHICS(PGH_FLOODFILL,hDC32->lpDrvData,&rc,lpStruct);
}

DWORD
lsd_display_stretchblt(WORD msg,HDC32 hDC32,DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    DWORD dwRet;
    POINT pt;
    SIZE sz;
    LSDE_STRETCHDATA *lpsd = &lpStruct->lsde.stretchdata;

    if (hDC32->dwInvalid & (IM_BRUSHMASK|IM_TEXTCOLORMASK|IM_BKCOLORMASK))
	DisplayValidate(hDC32, lpStruct,
		IM_BRUSHMASK|IM_TEXTCOLORMASK|IM_BKCOLORMASK);

    if (msg == LSD_PATBLT) {
	if (lpsd->nWidthDest < 0) {
	    lpsd->nWidthDest = abs(lpsd->nWidthDest);
	    lpsd->xDest -= lpsd->nWidthDest;
	}
	if (lpsd->nHeightDest < 0) {
	    lpsd->nHeightDest = abs(lpsd->nHeightDest);
	    lpsd->yDest -= lpsd->nHeightDest;
	}
	lpsd->nWidthSrc = lpsd->nWidthDest;
	lpsd->nHeightSrc = lpsd->nHeightDest;
    }
    else if (lpsd->hSrcDC32) {
	pt.x = lpsd->xSrc;
	pt.y = lpsd->ySrc;

#if	0
	LPtoDP(GETHDC16(lpsd->hSrcDC32),&pt,1);
	/* LPtoDP uses MM_UAPPLY macros, so DC origins are not taken into */
	/* account; we have to explicitly add them in */
	lpsd->xSrc = pt.x + lpsd->hSrcDC32->DOx;
	lpsd->ySrc = pt.y + lpsd->hSrcDC32->DOy;
#endif	/* 0 */

	/* MM_UAPPLY has been replaced by MM0_LPtoDP_X() and MM0_LPtoDP_Y()
	 * which also work with world transforms.  The 0 in MM0 means that
	 * it assumes that the DC origin is (0,0).  In other words, MM0_*()
	 * does not take the DC origin into account.
	 *
	 * MM_LPtoDP_X() and MM_LPtoDP_Y() have also been added which take
	 * the DC origin into account.  These two functions replace MM_APPLY.
	 *
	 * Hence, it ought to be possible to replace the three lines above
         * with these two...
	 */
	lpsd->xSrc = MM_LPtoDP_X(lpsd->hSrcDC32, pt.x, pt.y);
	lpsd->ySrc = MM_LPtoDP_Y(lpsd->hSrcDC32, pt.x, pt.y);

	sz.cx = lpsd->nWidthSrc;
	sz.cy = lpsd->nHeightSrc;
	LEtoDE(lpsd->hSrcDC32,&sz);
	lpsd->nWidthSrc = sz.cx;
	lpsd->nHeightSrc = sz.cy;

	/*
	 *  Some code to take into account a corresponding fix
	 *  in lsd_mm_stretchblt(), which should have been applied
	 *  here as well.
	 */
	if ((lpsd->hSrcDC32->VEy ^ lpsd->hSrcDC32->WEy) < 0  && lpsd->nHeightSrc < 0)
	    lpsd->nHeightSrc = abs(lpsd->nHeightSrc);
	if ((lpsd->hSrcDC32->VEx ^ lpsd->hSrcDC32->WEx) < 0  && lpsd->nWidthSrc < 0)
	    lpsd->nWidthSrc = abs(lpsd->nWidthSrc);

	if (lpsd->hSrcDC32->dwInvalid & IM_BKCOLORMASK)
	    DisplayValidate(lpsd->hSrcDC32,lpStruct, IM_SRCBKCOLORMASK);

	/* this field is overloaded */
	lpsd->hSrcDC32 = (HDC32)(lpsd->hSrcDC32->lpDrvData);
    }

    dwRet = DRVCALL_GRAPHICS(PGH_STRETCHBLT,hDC32->lpDrvData,0L,lpStruct);
    return dwRet;
}

DWORD
lsd_display_stretchdibits(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LSDE_STRETCHDATA *lpsd = &lpStruct->lsde.stretchdata;
    HBITMAP hDIB,hBMPOld;
    HDC hCompatDC;
    HDC32 hCompatDC32;
    BOOL bRet;
    LPLOGPALETTE lp;
    HPALETTE     hpal,hpalold = 0;

    if (!lpsd || !lpsd->lpbmi)
	return 0;

    if (!(hCompatDC = CreateCompatibleDC(GETHDC16(hDC32))))
	return 0;

    hpal = hDC32->hPalette;
    lp   = WinMalloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY)*356);

    GetPaletteEntries(hpal,0,256,lp->palPalEntry);
    lp->palVersion = 0x300;
    lp->palNumEntries = 256;

    hpal = CreatePalette(lp);
    WinFree(lp);

    if (hpal)
       hpalold = SelectPalette(hCompatDC,hpal,0);

    if (!(hDIB = CreateDIBitmap(hCompatDC,&lpsd->lpbmi->bmiHeader,
	    CBM_INIT,lpsd->lpvBits,lpsd->lpbmi,lpsd->fuColorUse))) {
	DeleteDC(hCompatDC);
	return 0;
    }
    hBMPOld = SelectObject(hCompatDC,hDIB);

    if (hDC32->dwInvalid & IM_BRUSHMASK)
	DisplayValidate(hDC32,lpStruct,IM_BRUSHMASK);

    hCompatDC32 = GETDCINFO(hCompatDC);
    lpsd->hSrcDC32 = (HDC32)(hCompatDC32->lpDrvData);
    RELEASEDCINFO(hCompatDC32);

    bRet = (BOOL)DRVCALL_GRAPHICS(PGH_STRETCHBLT,
		hDC32->lpDrvData,0L,lpStruct);

    if (hpal) {
       SelectPalette(hCompatDC, hpalold, 0);
       DeleteObject(hpal);
    }

    SelectObject(hCompatDC, hBMPOld);
    DeleteObject(hDIB);
    DeleteDC(hCompatDC);
    return (bRet)?lpsd->yDest:0;
}

DWORD
lsd_display_setpixel(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    if (hDC32->dwInvalid & IM_ROP2MASK)
	DisplayValidate(hDC32, lpStruct,IM_ROP2MASK);

    return DRVCALL_GRAPHICS(PGH_SETPIXEL,hDC32->lpDrvData,dwParam,lpStruct);
}

DWORD
lsd_display_getpixel(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    return DRVCALL_GRAPHICS(PGH_GETPIXEL,hDC32->lpDrvData,0L,lpStruct);
}

DWORD
lsd_display_setdibtodevice(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LSDS_PARAMS argptr;
    LSDE_STRETCHDATA *lpsd = &lpStruct->lsde.stretchdata;
    IMAGEINFO image,*lpimage = &image;
    BOOL ret;

    memset((LPSTR)lpimage,'\0',sizeof(IMAGEINFO));

    lpimage->ImagePlanes    = (lpsd->lpbmi->bmiHeader.biPlanes == 1) ? 1 :
		DRVCALL_GRAPHICS(PGH_GETDEVICECAPS,
				hDC32->lpDrvData,PLANES,0);
    lpimage->ImageDepth = (lpsd->lpbmi->bmiHeader.biBitCount == 1) ? 1 :
		DRVCALL_GRAPHICS(PGH_GETDEVICECAPS,
				hDC32->lpDrvData,BITSPIXEL,0);
    lpsd->lpbmi->bmiHeader.biHeight = lpsd->startline + lpsd->numlines;

    lpimage->rcsinfo = NULL;

    if (!CreateDIBImage(lpimage,&(lpsd->lpbmi->bmiHeader),
			&lpsd->lpbmi->bmiColors[0],(LPVOID)lpsd->lpvBits,
			lpsd->fuColorUse))
	return 0;

    argptr.lsde_validate.dwInvalidMask = 0;
    argptr.lsde.imagedata.lpimagedata = lpimage->lpDrvData;
    argptr.lsde.imagedata.xSrc = lpsd->xSrc;
    argptr.lsde.imagedata.ySrc = lpsd->ySrc+lpsd->startline;
    argptr.lsde.imagedata.xDest = lpsd->xDest;
    argptr.lsde.imagedata.yDest = lpsd->yDest;
    argptr.lsde.imagedata.cx = lpsd->nWidthDest;
    argptr.lsde.imagedata.cy = lpsd->nHeightDest;
    ret = DRVCALL_GRAPHICS(PGH_PUTIMAGE,hDC32->lpDrvData,0L,&argptr);

    DRVCALL_IMAGES(PIH_DESTROYIMAGE,0L,0L,lpimage->lpDrvData);

    if (ret)
	return (DWORD)lpsd->numlines;
    else
	return 0L;
}

DWORD
lsd_display_getaspectratio(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    return DRVCALL_GRAPHICS(PGH_GETASPECTRATIO,
		hDC32->lpDrvData,dwParam,lpStruct);
}

DWORD
lsd_display_getdevcaps(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    return DRVCALL_GRAPHICS(PGH_GETDEVICECAPS,hDC32->lpDrvData,dwParam,0);
}

DWORD
lsd_display_scrolldc(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    return DRVCALL_GRAPHICS(PGH_SCROLLDC,hDC32->lpDrvData,dwParam,lpStruct);
}

DWORD
lsd_display_getnearclr(WORD wMsg,
	HDC32 hDC32,
	DWORD dwParam,
	LPLSDS_PARAMS lpParam)
{
    if (hDC32->hPalette != GetStockObject(DEFAULT_PALETTE))
	if (TWIN_IsColorInPalette(hDC32->hPalette,
			lpParam->lsde.colorref))
	    return lpParam->lsde.colorref;
    return DRVCALL_COLORS(PLH_GETNEARCOLOR, lpParam->lsde.colorref, 0L, 0L);
}

void
DisplayValidate(HDC32 hDC32,LPLSDS_PARAMS lpStruct,DWORD dwInvalidMask)
{
    LSDE_VALIDATE *lpvs = &lpStruct->lsde_validate;
    DWORD dwMask;
    LPBRUSHINFO lpBrushInfo;
    LPPENINFO lpPenInfo;
    SIZE sz;

    dwMask = hDC32->dwInvalid & dwInvalidMask;

    if (dwMask & IM_POLYFILLMODEMASK) 
	lpvs->PolyFillMode = hDC32->PolyFillMode;

    if (dwMask & IM_TEXTCOLORMASK) 
	if (hDC32->TextColor & PALETTE_MASK)
	    lpvs->TextColor = TWIN_PaletteRGB(hDC32,hDC32->TextColor);
	else
	    lpvs->TextColor = hDC32->TextColor;

    if (dwMask & IM_BKCOLORMASK) 
	if (hDC32->BackColor & PALETTE_MASK)
	    lpvs->BackColor = TWIN_PaletteRGB(hDC32,hDC32->BackColor);
	else
	    lpvs->BackColor = hDC32->BackColor;

    if (dwMask & IM_BKMODEMASK) 
	lpvs->BackMode = hDC32->BackMode;

    if (dwMask & IM_ROP2MASK)
	lpvs->ROP2 = hDC32->ROP2;

    if (dwMask & IM_STRETCHMODEMASK)
	lpvs->StretchMode = hDC32->StretchBltMode;

    if (dwMask & (IM_BRUSHDEPTHMASK|IM_BRUSHCOLORMASK|IM_BRUSHFILLMASK)) {
	lpBrushInfo = GETBRUSHINFO(hDC32->hBrush);
	if (lpBrushInfo) {
	    if (dwMask & IM_BRUSHDEPTHMASK)
		/* need to re-select the brush */
		GdiSelectBrush(hDC32,lpBrushInfo);
	    if (dwMask & (IM_BRUSHCOLORMASK|IM_BRUSHFILLMASK)) {
		lpvs->BrushFlag = LOWORD(lpBrushInfo->dwFlags);
		if (lpBrushInfo->lpBrush.lbColor & PALETTE_MASK)
		    lpvs->BrushColor = TWIN_PaletteRGB(hDC32,
						lpBrushInfo->lpBrush.lbColor);
		else
		    lpvs->BrushColor = lpBrushInfo->lpBrush.lbColor;
		lpvs->BrushPrivate = lpBrushInfo->lpPrivate;
	    }
	}
	RELEASEBRUSHINFO(lpBrushInfo);
    }

    if (dwMask & (IM_PENCOLORMASK|IM_PENSTYLEMASK|IM_PENWIDTHMASK)) {
	lpPenInfo = GETPENINFO(hDC32->hPen);
	if (lpPenInfo) {
	    lpvs->PenStyle = lpPenInfo->lpPen.lopnStyle;
	    if (lpPenInfo->lpPen.lopnColor & PALETTE_MASK)
		lpvs->PenColor = TWIN_PaletteRGB(hDC32,
						lpPenInfo->lpPen.lopnColor);
	    else
		lpvs->PenColor = lpPenInfo->lpPen.lopnColor;

	    /* pen width is re-scaled thru map mode */
	    sz.cx = lpPenInfo->lpPen.lopnWidth.x;
	    sz.cy = 0;
	    LEtoDE(hDC32, &sz);
	    lpvs->PenWidth = max(sz.cx, 1);   /* Pen is at least one pixel */
	    lpvs->PenHeight = max(sz.cx, 1);  /* Win 3.1 does not support pen height */
	}
	RELEASEPENINFO(lpPenInfo);
    }

    if (dwInvalidMask == IM_SRCBKCOLORMASK) { 
	/* This mask is used only by itself in StretchBlt; */
	/* By now lpvs is already filled, so we add on top */
	lpvs->SrcDCBkColor = hDC32->BackColor;
	if (hDC32->BackColor & PALETTE_MASK)
	    lpvs->SrcDCBkColor = TWIN_PaletteRGB(hDC32,hDC32->BackColor);
	else
	    lpvs->SrcDCBkColor = hDC32->BackColor;
	lpvs->dwInvalidMask |= IM_SRCBKCOLORMASK;
    }
    else {
	/* Clear the bits that the driver is going to validate */
	hDC32->dwInvalid &= ~dwMask;
	lpvs->dwInvalidMask = dwMask;
    }
}

DWORD
lsd_display_init(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return 0;
}

DWORD
lsd_display_selectpal(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    HPALETTE32 hPalette32;
    HPALETTE hPal = (HPALETTE)dwParam, hPalOld;
    BOOL fPalBack = (BOOL)lpStruct;

    ASSERT_HPALETTE(hPalette32,hPal,0L);
    RELEASEPALETTEINFO(hPalette32);

    hPalOld = hDC32->hPalette;
    hDC32->hPalette = hPal;
    hDC32->fPalBack = fPalBack;

    LOCKGDI(hPal);

    if (hPalOld)
	UNLOCKGDI(hPalOld);

    return hPalOld;
}

DWORD
lsd_display_realizepal(WORD wMsg,
	LPDC lpDC,
	DWORD dwParam,
	LPLSDS_PARAMS lpParam)
{
	LPPALETTE lpPalette;
	LPLOGPALETTE lpLogPalette;
	LPUINT lpSystemPaletteIndex;
	BOOL bForegroundPalette, bInitSystemPalette;
	UINT u;

	APISTR((LF_API, "lsd_display_realizepal: wMsg %d lpDC %p\n",
		wMsg, lpDC));

	ASSERT_PALETTE(lpPalette, lpDC->hPalette, GDI_ERROR);

	if (!(lpLogPalette = lpPalette->lpLogPalette))
	{
		SetLastErrorEx(1, 0);
		return (DWORD)((UINT) GDI_ERROR);
	}

	if (!(lpSystemPaletteIndex = (LPUINT)
		WinRealloc(lpDC->lpSystemPaletteIndex,
		(lpLogPalette->palNumEntries + 1) * sizeof(UINT))))
	{
		SetLastErrorEx(1, 0);
		return (DWORD)((UINT) GDI_ERROR);
	}

	if (0)
	{
		LOGSTR((LF_LOG, "lsd_display_realizepal:"
			" realizing foreground palette\n"));
/*
		DRVCALL_COLORS(PLG_,
*/


	}
	bForegroundPalette = TRUE;



/* SEND WM_PALETTECHANGED here */
/* OR let SetFocus() send WM_PALETTE... */


	for (u = 0; u < lpLogPalette->palNumEntries; u++)
	{
		bInitSystemPalette = ((u == 0) && (bForegroundPalette == TRUE))
			? TRUE : FALSE;
		lpSystemPaletteIndex[u] = (UINT)
			DRVCALL_COLORS(PLH_SETSYSTEMPALETTE,
				(DWORD) lpDC->lpDrvData,
				(DWORD) bInitSystemPalette,
				(LPPALETTEENTRY)
					&(lpLogPalette->palPalEntry[u]));
		LOGSTR((LF_LOG, "lsd_display_realizepal:\n"
			" palette index [%d] rgb (%d, %d, %d) flags %d"
			" <-> system palette index [%d]\n", u,
			lpLogPalette->palPalEntry[u].peRed,
			lpLogPalette->palPalEntry[u].peGreen,
			lpLogPalette->palPalEntry[u].peBlue,
			lpLogPalette->palPalEntry[u].peFlags,
			lpSystemPaletteIndex[u]));
	}
	lpDC->lpSystemPaletteIndex = lpSystemPaletteIndex;

	lpPalette->fIsRealized = TRUE;

	UNLOCK_PALETTE(hPalette);

	return (u);

}


#if 0
extern HPALETTE32 TWIN_hPalRealized32;

DWORD
lsd_display_realizepal(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    HPALETTE32 hPalette32;
    DWORD dwRet;

    if (hDC32->hPalette == GetStockObject(DEFAULT_PALETTE)) {
	TWIN_hPalRealized32 = NULL;
	return 0;
    }

    ASSERT_HPALETTE(hPalette32,hDC32->hPalette,0);

    hPalette32->fIsRealized = TRUE;
    TWIN_hPalRealized32 = hPalette32;

    dwRet = hPalette32->uiNumEntries;
    RELEASEPALETTEINFO(hPalette32);
    return dwRet;
}
#endif	/* 0 */

DWORD
lsd_display_escape(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    int nInt;

    switch (dwParam) {
	case GETTECHNOLOGY:
	    return DRVCALL_GRAPHICS(PGH_GETDEVICECAPS,
			hDC32->lpDrvData,TECHNOLOGY,0);

	case MFCOMMENT:
	    return 0L;

	case QUERYESCSUPPORT:
	    nInt = *((int *)lpStruct->lsde.escape.lpszInData);
	    LOGSTR((LF_LOG,"ESCAPE: QUERYESCSUPPORT for DISPLAY, func %x\n",
				nInt));
	    return 0L;


	default:
	    return 0L;
    }
}

static void
FixCoordinates(LPRECT lpRect)
{
    int tmp;

    /* Make sure that:	lpRect->left < lpRect->right
			lpRect->top < lpRect->bottom
    */
    if ( lpRect->right < lpRect->left ) {
	tmp = lpRect->left;
	lpRect->left = lpRect->right;
	lpRect->right = tmp;
    }
    if ( lpRect->bottom < lpRect->top ) {
	tmp = lpRect->top;
	lpRect->top = lpRect->bottom;
	lpRect->bottom = tmp;
    }
}

/* (WIN32) world transform blt ********************************************* */

static void fraction(double f, int *numer, int *denom);

typedef struct {
	/* nDstStretchBltMode = dst device stretch blt mode
	 * lpSrcDC            = src device
	 * (nSrcX, nSrcY)     = src device pt corresponding to scan line
	 * nSrcWidth          = src device width
	 * refcnt             = reference count
	 * x0...x1            = scan line position
	 * x[0...nSrcWidth-1] = scan line pixels
	 */
	int nDstStretchBltMode;
	LPDC lpSrcDC;
	int nSrcX, nSrcY;
	int nSrcWidth;
	int refcnt;
	int x0, x1;
	void *x;
} SCANBLT;

/*static SCANBLT *alloc_scanblt(); */
static void free_scanblt(SCANBLT **scanblt);

static int x_scanblt(SCANBLT **scanblt);


#ifdef NEVER
#ifdef TWIN32
static SCANBLT *dup_scanblt(SCANBLT *scanblt1);
static SCANBLT *not_scanblt(SCANBLT *scanblt1);
static SCANBLT *and_scanblt(SCANBLT *scanblt1, SCANBLT *scanblt2);
static SCANBLT *or_scanblt(SCANBLT *scanblt1, SCANBLT *scanblt2);
static SCANBLT *xor_scanblt(SCANBLT *scanblt1, SCANBLT *scanblt2);

#ifdef TWIN32
static int r_scanblt(SCANBLT **scanblt);
static int w_scanblt(SCANBLT **scanblt);
#endif

#endif
#endif

typedef struct {
	/* nDstStretchBltMode = dst device stretch blt mode
	 * (nDstX, nDstY)     = dst device pt corresponding to buffer pt (0, 0)
	 * lpSrcDC            = src device
	 * (nSrcX, nSrcY)     = src device pt corresponding to buffer pt (0, 0)
	 * nSrcWidth          = src device width
	 * nSrcHeight         = src device height
	 * (x0, y0)           = top left buffer pt
	 * (x1, y1)           = bottom right buffer pt
	 * y[y0 ... y1]       = scan lines y0 ... y1
	 */
	int nDstStretchBltMode;
	int nDstX, nDstY;
	LPDC lpSrcDC;
	int nSrcX, nSrcY;
	int nSrcWidth, nSrcHeight;
	int x0, x1;
	int y0, y1;
	SCANBLT **y;
} WORLDBLT;

static WORLDBLT *alloc_worldblt(int nDstStretchBltMode,
	int nDstX, int nDstY,
	LPDC lpSrcDC,
	int nSrcX, int nSrcY,
	int nSrcWidth, int nSrcHeight);
static void free_worldblt(WORLDBLT **worldblt);

#ifdef NEVER
#ifdef TWIN32
static int r_worldblt(WORLDBLT **worldblt, int y);
static int w_worldblt(WORLDBLT **worldblt, int y);
#endif
#endif

static int x_worldblt(WORLDBLT **worldblt, int y);

static WORLDBLT *xmirrorblt(WORLDBLT *worldblt);
static WORLDBLT *ymirrorblt(WORLDBLT *worldblt);
static WORLDBLT *scaleblt(WORLDBLT *worldblt, double m11, double m22);
static WORLDBLT *xscaleblt(WORLDBLT *worldblt, double m11);

static WORLDBLT *yscaleblt(WORLDBLT *worldblt, double m22);
static WORLDBLT *xshearblt(WORLDBLT *worldblt, double m12);
static WORLDBLT *yshearblt(WORLDBLT *worldblt, double m21);

/*static SCANBLT *alloc_scanblt()
{
} */

static void free_scanblt(SCANBLT **scanblt)
{
	LOGSTR((LF_LOG, "free_scanblt: %p\n", scanblt));

	if (!scanblt)
		return;

	LOGSTR((LF_LOG, "free_scanblt: *%p = %p\n", scanblt, *scanblt));

	if (!*scanblt)
		return;

	LOGSTR((LF_LOG, "free_scanblt: %p->refcnt = %d\n",
		*scanblt, (*scanblt)->refcnt));

	if (--((*scanblt)->refcnt) == 0)
	{
		LOGSTR((LF_LOG, "free_scanblt: free %p\n", *scanblt));
		WinFree(*scanblt);
	}

	*scanblt = (SCANBLT *) 0;

	LOGSTR((LF_LOG, "free_scanblt: *%p = %p\n", scanblt, *scanblt));

}

#ifdef NEVER
#ifdef TWIN32
static int r_scanblt(SCANBLT **scanblt)
{
	LOGSTR((LF_LOG, "r_scanblt: x_scanblt(%p)\n", scanblt));

	if (!x_scanblt(scanblt))
		return (0);



/*
	LOGSTR((LF_LOG, "r_scanblt: %p->pixels = %p\n",
		*scanblt, (*scanblt)->pixels));

	if (!(*scanblt)->pixels)
	{
	}
*/


	LOGSTR((LF_LOG, "r_scanblt: *%p = %p\n", scanblt, *scanblt));

	return ((*scanblt)->x1 - (*scanblt)->x0);

}




static int w_scanblt(SCANBLT **scanblt)
{
	LOGSTR((LF_LOG, "w_scanblt: x_scanblt(%p)\n", scanblt));

	if (!x_scanblt(scanblt))
		return (0);







	LOGSTR((LF_LOG, "w_scanblt: %p->refcnt = %d\n",
		*scanblt, (*scanblt)->refcnt));

	if ((*scanblt)->refcnt != 1)
	{
		LOGSTR((LF_LOG, "w_scanblt: dup_scanblt(%p)\n", *scanblt));
		*scanblt = dup_scanblt(*scanblt);
	}

	LOGSTR((LF_LOG, "w_scanblt: *%p = %p\n", scanblt, *scanblt));

	return (*scanblt) ? (*scanblt)->x1 - (*scanblt)->x0 : 0;

}
#endif
#endif

static int x_scanblt(SCANBLT **scanblt)
{
	LOGSTR((LF_LOG, "x_scanblt: %p\n", scanblt));

	if (!scanblt)
		return (0);

	LOGSTR((LF_LOG, "x_scanblt: *%p = %p\n", scanblt, *scanblt));

	if (!*scanblt)
	{
		if (!(*scanblt = (SCANBLT *) WinMalloc(sizeof(SCANBLT))))
		{
			return (0);
		}
		LOGSTR((LF_LOG, "x_scanblt: *%p = %p\n", scanblt, *scanblt));
		(*scanblt)->refcnt = 1;
		(*scanblt)->x0 = 0;
		(*scanblt)->x1 = 0;
		(*scanblt)->x = (void *) 0;
	}

	return ((*scanblt)->refcnt);

}





#ifdef NEVER
#ifdef TWIN32

static SCANBLT *dup_scanblt(SCANBLT *scanblt1)
{
#ifdef LATER
	SCANBLT *scanblt = allo--c_scanblt(scanblt1);

	LOGSTR((LF_LOG, "dup_scanblt: %p = %p\n", scanblt, scanblt1));

	if (scanblt)
	{
	}

	return (scanblt);
#else
	return 0;
#endif
}
#endif
#endif

#ifdef NEVER
#ifdef TWIN32
static SCANBLT *not_scanblt(SCANBLT *scanblt1)
{
	SCANBLT *scanblt = dup_scanblt(scanblt1);

	LOGSTR((LF_LOG, "not_scanblt: %p = ~%p\n", scanblt, scanblt1));

	if (scanblt && w_scanblt(&scanblt))
	{
	}

	return (scanblt);

}

static SCANBLT *and_scanblt(SCANBLT *scanblt1, SCANBLT *scanblt2)
{
	SCANBLT *scanblt = dup_scanblt(scanblt1);

	LOGSTR((LF_LOG, "and_scanblt: %p = %p & %p\n",
		scanblt, scanblt1, scanblt2));

	if (scanblt && w_scanblt(&scanblt) && r_scanblt(&scanblt2))
	{
	}

	return (scanblt);

}

static SCANBLT *or_scanblt(SCANBLT *scanblt1, SCANBLT *scanblt2)
{
	SCANBLT *scanblt = dup_scanblt(scanblt1);

	LOGSTR((LF_LOG, "or_scanblt: %p = %p | %p\n",
		scanblt, scanblt1, scanblt2));

	if (scanblt)
	{
	}

	return (scanblt);

}

static SCANBLT *xor_scanblt(SCANBLT *scanblt1, SCANBLT *scanblt2)
{
	SCANBLT *scanblt = dup_scanblt(scanblt1);

	LOGSTR((LF_LOG, "xor_scanblt: %p = %p ^ %p\n",
		scanblt, scanblt1, scanblt2));

	if (scanblt)
	{
	}

	return (scanblt);

}

#endif
#endif


static WORLDBLT *alloc_worldblt(int nDstStretchBltMode,
	int nDstX, int nDstY,
	LPDC lpSrcDC,
	int nSrcX, int nSrcY,
	int nSrcWidth, int nSrcHeight)
{
	WORLDBLT *worldblt = (WORLDBLT *) WinMalloc(sizeof(WORLDBLT));

	LOGSTR((LF_LOG, "alloc_worldblt: %p\n"
		" dst stretch mode = %d\n"
		" dst pt = (%d, %d)\n"
		" src DC %p\n"
		" src pt = (%d, %d)\n"
		" src w x h = %d x %d\n",
		worldblt,
		nDstStretchBltMode,
		nDstX, nDstY,
		lpSrcDC,
		nSrcX, nSrcY,
		nSrcWidth, nSrcHeight));

	if (worldblt)
	{
		worldblt->nDstStretchBltMode = nDstStretchBltMode;
		worldblt->nDstX = nDstX;
		worldblt->nDstY = nDstY;
		worldblt->lpSrcDC = lpSrcDC;
		worldblt->nSrcX = nSrcX;
		worldblt->nSrcY = nSrcY;
		worldblt->nSrcWidth = abs(nSrcWidth);
		worldblt->nSrcHeight = abs(nSrcHeight);
		worldblt->x0 = 0;
		worldblt->x1 = nSrcWidth;
		worldblt->y0 = 0;
		worldblt->y1 = nSrcHeight;
		worldblt->y = (SCANBLT **) 0;

		if (nSrcWidth < 0)
			worldblt = xmirrorblt(worldblt);
		if (nSrcHeight < 0)
			worldblt = ymirrorblt(worldblt);
	}

	return (worldblt);



}







static void free_worldblt(WORLDBLT **worldblt)
{
	LOGSTR((LF_LOG, "free_worldblt: %p\n", worldblt));

	if (!worldblt)
		return;

	LOGSTR((LF_LOG, "free_worldblt: *%p = %p\n", worldblt, *worldblt));

	if (!*worldblt)
		return;

	LOGSTR((LF_LOG, "free_worldblt: %p->y = %p\n",
		*worldblt, (*worldblt)->y));

	if ((*worldblt)->y)
	{
		int y;
		for (y = (*worldblt)->y0; y < (*worldblt)->y1; y++)
		{
			LOGSTR((LF_LOG, "free_worldblt: free %p->y[%d] = %p\n",
				*worldblt, y, (*worldblt)->y[y]));
			free_scanblt(&((*worldblt)->y[y]));
		}
		LOGSTR((LF_LOG, "free_worldblt: free %p + %d = %p\n",
			(*worldblt)->y, (*worldblt)->y0,
			(*worldblt)->y + (*worldblt)->y0));
		WinFree((*worldblt)->y + (*worldblt)->y0);
		(*worldblt)->y = (SCANBLT **) 0;
	}

	LOGSTR((LF_LOG, "free_worldblt: free %p\n", *worldblt));
	WinFree(*worldblt);
	*worldblt = (WORLDBLT *) 0;

	LOGSTR((LF_LOG, "free_worldblt: *%p = %p\n", worldblt, *worldblt));

}

#ifdef NEVER
#ifdef TWIN32
static int r_worldblt(WORLDBLT **worldblt, int y)
{
	LOGSTR((LF_LOG, "r_worldblt: x_worldblt(%p, %d)\n", worldblt, y));

	if (!x_worldblt(worldblt, y))
		return (0);

	LOGSTR((LF_LOG, "w_worldblt: r_scanblt(%p)\n", (*worldblt)->y + y));

	return (r_scanblt((*worldblt)->y + y));

}

static int w_worldblt(WORLDBLT **worldblt, int y)
{
	LOGSTR((LF_LOG, "w_worldblt: x_worldblt(%p, %d)\n", worldblt, y));

	if (!x_worldblt(worldblt, y))
		return (0);

	LOGSTR((LF_LOG, "w_worldblt: w_scanblt(%p)\n", (*worldblt)->y + y));

	return (w_scanblt((*worldblt)->y + y));

}
#endif
#endif

static int x_worldblt(WORLDBLT **worldblt, int y)
{
	LOGSTR((LF_LOG, "x_worldblt: %p, y = %d\n", worldblt, y));

	if (!worldblt)
		return (0);

	LOGSTR((LF_LOG, "x_worldblt: *%p = %p\n", worldblt, *worldblt));

	if (!*worldblt)
		return (0);

	LOGSTR((LF_LOG, "x_worldblt: %p->y0 = %d, %p->y1 = %d\n",
		*worldblt, (*worldblt)->y0,
		*worldblt, (*worldblt)->y1));

	if ((y < (*worldblt)->y0) || (y >= (*worldblt)->y1))
		return (0);

	LOGSTR((LF_LOG, "x_worldblt: %p->y = %p\n",
		*worldblt, (*worldblt)->y));

	if (!(*worldblt)->y)
	{
		int y;
		if (!((*worldblt)->y =
			(SCANBLT **)
			WinMalloc((*worldblt)->nSrcHeight * sizeof(SCANBLT *))))
		{
			return (0);
		}
		(*worldblt)->y -= (*worldblt)->y0;
		LOGSTR((LF_LOG, "x_worldblt: %p->y = %p\n",
			*worldblt, (*worldblt)->y));
		for (y = (*worldblt)->y0; y < (*worldblt)->y1; y++)
		{
			(*worldblt)->y[y] = (SCANBLT *) 0;
			LOGSTR((LF_LOG, "x_worldblt: %p->y[%d] = %p\n",
				*worldblt, y, (*worldblt)->y[y]));
		}
	}

	LOGSTR((LF_LOG, "x_worldblt: x_scanblt(%p)\n", (*worldblt)->y + y));

	return (x_scanblt((*worldblt)->y + y));

}

static WORLDBLT *xmirrorblt(WORLDBLT *worldblt)
{
	/* perform mirror operation
	 *	|  -1   0   0 |
	 *	|   0   1   0 |
	 *	|   0   0   1 |
	 */

	LOGSTR((LF_LOG, "xmirrorblt: %p\n", worldblt));

	if (!worldblt)
		return ((WORLDBLT *) 0);

	if (worldblt->x0 >= worldblt->x1 - 1)
		return (worldblt);



	return (worldblt);


}




static WORLDBLT *ymirrorblt(WORLDBLT *worldblt)
{
	/* perform mirror operation
	 *	|   1   0   0 |
	 *	|   0  -1   0 |
	 *	|   0   0   1 |
	 */

	int y0, y1;
	SCANBLT *y;

	LOGSTR((LF_LOG, "ymirrorblt: %p\n", worldblt));

	if (!worldblt)
		return ((WORLDBLT *) 0);

	if (worldblt->y0 >= worldblt->y1 - 1)
		return (worldblt);

	/* */
	for (y0 = worldblt->y0, y1 = worldblt->y1 - 1; y0 <= y1; y0++, y1--)
	{
		if (!x_worldblt(&worldblt, y0) || !x_worldblt(&worldblt, y1))
		{
			free_worldblt(&worldblt);
			return ((WORLDBLT *) 0);
		}
		y = worldblt->y[y0];
		worldblt->y[y0] = worldblt->y[y1];
		worldblt->y[y1] = y;
	}

	/* */
	y0 = worldblt->y0;
	y1 = worldblt->y1;
	worldblt->y0 = -y1 + 1;
	worldblt->y1 = -y0 + 1;

	/* */
	worldblt->y += y0;
	worldblt->y -= worldblt->y0;

	/* */
	return (worldblt);




}






static WORLDBLT *scaleblt(WORLDBLT *worldblt, double m11, double m22)
{
	/* perform scale operation
	 *	| m11   0   0 |
	 *	|   0 m22   0 |
	 *	|   0   0   1 |
	 */

	LOGSTR((LF_LOG, "scaleblt: %p, m11 = %f, m22 = %f\n",
		worldblt, m11, m22));

	return (yscaleblt(xscaleblt(worldblt, m11), m22));

}

static WORLDBLT *xscaleblt(WORLDBLT *worldblt, double m11)
{
	/* perform scale operation
	 *	| m11   0   0 |
	 *	|   0   1   0 |
	 *	|   0   0   1 |
	 */

	int m11_numer, m11_denom;

	fraction(m11, &m11_numer, &m11_denom);

	LOGSTR((LF_LOG, "xscaleblt: %p, m11 = %f = (%d / %d)\n",
		worldblt, m11, m11_numer, m11_denom));

	if (!worldblt)
		return ((WORLDBLT *) 0);

	if (m11_numer == m11_denom)
		return (worldblt);

	if (m11_numer == -m11_denom)
		return (xmirrorblt(worldblt));

	if (m11_numer < 0)
	{
		worldblt = xmirrorblt(worldblt);
		m11 = -m11;
		m11_numer = -m11_numer;
		LOGSTR((LF_LOG, "xscaleblt: %p, m11 = %f = (%d / %d)\n",
			worldblt, m11, m11_numer, m11_denom));
	}

	if (m11_denom < 0)
	{
		worldblt = xmirrorblt(worldblt);
		m11 = -m11;
		m11_denom = -m11_denom;
		LOGSTR((LF_LOG, "xscaleblt: %p, m11 = %f = (%d / %d)\n",
			worldblt, m11, m11_numer, m11_denom));
	}




	return (worldblt);

}














static WORLDBLT *yscaleblt(WORLDBLT *worldblt, double m22)
{
	/* perform scale operation
	 *	|   1   0   0 |
	 *	|   0 m22   0 |
	 *	|   0   0   1 |
	 */

	int m22_numer, m22_denom;

	fraction(m22, &m22_numer, &m22_denom);

	LOGSTR((LF_LOG, "yscaleblt: %p, m22 = %f = (%d / %d)\n",
		worldblt, m22, m22_numer, m22_denom));

	if (!worldblt)
		return ((WORLDBLT *) 0);

	if (m22_numer == m22_denom)
		return (worldblt);

	if (m22_numer == -m22_denom)
		return (ymirrorblt(worldblt));

	if (m22_numer < 0)
	{
		worldblt = ymirrorblt(worldblt);
		m22 = -m22;
		m22_numer = -m22_numer;
		LOGSTR((LF_LOG, "yscaleblt: %p, m22 = %f = (%d / %d)\n",
			worldblt, m22, m22_numer, m22_denom));
	}

	if (m22_denom < 0)
	{
		worldblt = ymirrorblt(worldblt);
		m22 = -m22;
		m22_denom = -m22_denom;
		LOGSTR((LF_LOG, "yscaleblt: %p, m22 = %f = (%d / %d)\n",
			worldblt, m22, m22_numer, m22_denom));
	}



	return (worldblt);

}








static WORLDBLT *xshearblt(WORLDBLT *worldblt, double m12)
{
	/* perform shear operation
	 *	|   1 m12   0 |
	 *	|   0   1   0 |
	 *	|   0   0   1 |
	 */

	WORLDBLT *dst_worldblt;
	int nDstHeight;

	LOGSTR((LF_LOG, "xshearblt: %p, m12 = %f\n", worldblt, m12));

	if (!worldblt)
		return ((WORLDBLT *) 0);

	if (fabs((worldblt->x1 - worldblt->x0) * m12) < 0.5)
		return (worldblt);

	/* alloc dst (sheared) worldblt buffer */
	nDstHeight = worldblt->nSrcWidth * m12;
	if (!(dst_worldblt = alloc_worldblt(worldblt->nDstStretchBltMode,
		worldblt->nDstX, worldblt->nDstY,
		(HDC32) 0, 0, 0,
		worldblt->nSrcWidth, nDstHeight)))
	{
		free_worldblt(&worldblt);
		return ((WORLDBLT *) 0);
	}

	/* init dst (sheared) worldblt buffer */






	/* free src (unsheared) worldblt buffer */
	free_worldblt(&worldblt);

	/* return dst (sheared) worldblt buffer */
	return (dst_worldblt);

}





static WORLDBLT *yshearblt(WORLDBLT *worldblt, double m21)
{
	/* perform shear operation
	 *	|   1   0   0 |
	 *	| m21   1   0 |
	 *	|   0   0   1 |
	 */

	int y;
	double dx;

	LOGSTR((LF_LOG, "yshearblt: %p, m21 = %f\n", worldblt, m21));

	if (!worldblt)
		return ((WORLDBLT *) 0);

	if (fabs((worldblt->y1 - worldblt->y0) * m21) < 0.5)
		return (worldblt);

	LOGSTR((LF_LOG, "yshearblt: x0 = %d, x1 = %d, y0 = %d, y1 = %d\n",
		worldblt->x0, worldblt->x1, worldblt->y0, worldblt->y1));

	worldblt->x0 = INT_MAX;
	worldblt->x1 = INT_MIN;
	for (y = worldblt->y0, dx = y * m21; y < worldblt->y1; y++, dx += m21)
	{
		if (!x_worldblt(&worldblt, y))
		{
			continue;
		}
		LOGSTR((LF_LOG, "yshearblt: [%d] x0 = %d, x1 = %d\n",
			y, worldblt->y[y]->x0, worldblt->y[y]->x1));
		worldblt->y[y]->x0 += dx;
		worldblt->y[y]->x1 += dx;
		worldblt->x0 = min(worldblt->x0, worldblt->y[y]->x0);
		worldblt->x1 = max(worldblt->x1, worldblt->y[y]->x1);
		LOGSTR((LF_LOG, "yshearblt: [%d] x0 = %d, x1 = %d\n",
			y, worldblt->y[y]->x0, worldblt->y[y]->x1));
	}

	LOGSTR((LF_LOG, "yshearblt: x0 = %d, x1 = %d, y0 = %d, y1 = %d\n",
		worldblt->x0, worldblt->x1, worldblt->y0, worldblt->y1));

	return (worldblt);

}

/* ------------------------------ */

static void fraction(double f, int *numer, int *denom)
{
	double emax, e;
	int n, d;

	LOGSTR((LF_LOG, "fraction: %f\n", f));

	emax = 1.0e6;
	for (d = 16; d > 0; --d)
	{
		n = (int)(f * d);
		e = fabs(f - ((double) n / (double) d));
		if (e <= emax)
		{
			emax = e;
			*numer = n;
			*denom = d;
			LOGSTR((LF_LOG, " = (%d / %d) +/- %f\n", n, d, emax));
		}
	}

}



#ifdef OLD_CODE_NOT_VALID

	int x, y, y0, y1, dy0, dy1;
	double dy;

	y0 = worldblt->y0;
	y1 = worldblt->y1;
	dy0 = (int)(worldblt->x0 * m12);
	dy1 = (int)(worldblt->x1 * m12);
	if ((worldblt = resizeblt(worldblt, dy0, dy1)) == NULL)
		return (NULL);

	for (x = worldblt->x0, dy = x * m12; x < worldblt->x1; x++, dy += m12)
	{
		if (dy < 0.0)
		{
			for (y = y0; y < y1; y++)
			{
				if ((x < worldblt->y[y].x0)
				 || (x >= worldblt->y[y].x1))
					continue;
				worldblt->y[y+dy].x0
					= min(x, worldblt->y[y+dy].x0);
				worldblt->y[y+dy].x1
					= max(x, worldblt->y[y+dy].x1);
			}
		}
		else
		{
			for (y = y1; --y >= y0; )
			{
				worldblt->y[y+dy].pels[]
					&= [];
				worldblt->y[y+dy].pels[]
					|= [y].pixels[]
					& [];
			}
		}
	}
				worldblt->y[dsty].pixels[x & xmask]
					&= pixelmask[x & xmask];
				worldblt->y[dsty].pixels[x & xmask]
					|= worldblt->y[srcy].pixels[x & xmask]
					& pixelmask[x & xmask];

#endif /* OLD_CODE_NOT_VALID */







DWORD
lsd_display_worldblt(WORD wMsg,
	LPDC lpDC,
	DWORD dwParam,
	LPLSDS_PARAMS lpParams)
{
	LPLSDE_WORLDBLT lpWorldBltParam = &(lpParams->lsde.worldblt);
	int x, y, w, h;
	XFORM xform, xform1, xform2;
	double a11, a22, b12, c21;
	WORLDBLT *worldblt;
	BOOL bSuccess;

	LOGSTR((LF_LOG, "lsd_display_worldblt: wMsg %x lpDC %p\n",
		wMsg, lpDC));

	LOGSTR((LF_LOG,
		" dest pt A = (%d, %d)\n"
		" dest pt B = (%d, %d)\n"
		" dest pt C = (%d, %d)\n",
		lpWorldBltParam->ptDestXY[0].x, lpWorldBltParam->ptDestXY[0].y,
		lpWorldBltParam->ptDestXY[1].x, lpWorldBltParam->ptDestXY[1].y,
		lpWorldBltParam->ptDestXY[2].x, lpWorldBltParam->ptDestXY[2].y
		));

	LOGSTR((LF_LOG,
		" src DC %p\n"
		" (before mapping) src pt = (%d, %d)\n"
		" (before mapping) src w x h = %d x %d\n",
		lpWorldBltParam->lpSrcDC,
		lpWorldBltParam->nSrcX, lpWorldBltParam->nSrcY,
		lpWorldBltParam->nSrcWidth, lpWorldBltParam->nSrcHeight));

	/* map source rectangle coordinates */
	if (lpWorldBltParam->lpSrcDC)
	{
		/* disallow source rotation/shear transformations */
		if ((lpWorldBltParam->lpSrcDC->eM12 != 0.0)
		 || (lpWorldBltParam->lpSrcDC->eM21 != 0.0))
		{
			SetLastErrorEx(1, 0);
			return (DWORD)((BOOL)FALSE);
		}
		/* map source coordinates */
		x = MM_LPtoDP_X(lpWorldBltParam->lpSrcDC,
			lpWorldBltParam->nSrcX,
			lpWorldBltParam->nSrcY);
		y = MM_LPtoDP_Y(lpWorldBltParam->lpSrcDC,
			lpWorldBltParam->nSrcX,
			lpWorldBltParam->nSrcY);
		lpWorldBltParam->nSrcX = x;
		lpWorldBltParam->nSrcY = y;
		/* map source dimensions */
		w = MM_LEtoDE_X(lpWorldBltParam->lpSrcDC,
			lpWorldBltParam->nSrcWidth);
		h = MM_LEtoDE_Y(lpWorldBltParam->lpSrcDC,
			lpWorldBltParam->nSrcHeight);
		lpWorldBltParam->nSrcWidth = w;
		lpWorldBltParam->nSrcHeight = h;
	}

	LOGSTR((LF_LOG,
		" (after mapping) src pt = (%d, %d)\n"
		" (after mapping) src w x h = %d x %d\n",
		lpWorldBltParam->nSrcX, lpWorldBltParam->nSrcY,
		lpWorldBltParam->nSrcWidth, lpWorldBltParam->nSrcHeight));

	/* compute transform matrix from src rectangle to unit square */
	if ((lpWorldBltParam->nSrcWidth == 0)
	 || (lpWorldBltParam->nSrcHeight == 0))
	{
		SetLastErrorEx(1, 0);
		return (DWORD)((BOOL)FALSE);
	}
	xform1.eM11 = 1.0 / lpWorldBltParam->nSrcWidth;
	xform1.eM22 = 1.0 / lpWorldBltParam->nSrcHeight;
	xform1.eM12 = xform1.eM21 = xform1.eDx = xform1.eDy = 0.0;

	/* compute transform matrix from unit square to dst parallelogram */
	xform2.eDx  = lpWorldBltParam->ptDestXY[0].x;
	xform2.eDy  = lpWorldBltParam->ptDestXY[0].y;
	xform2.eM11 = lpWorldBltParam->ptDestXY[1].x - xform2.eDx;
	xform2.eM21 = lpWorldBltParam->ptDestXY[2].x - xform2.eDx;
	xform2.eM12 = lpWorldBltParam->ptDestXY[1].y - xform2.eDy;
	xform2.eM22 = lpWorldBltParam->ptDestXY[2].y - xform2.eDy;

	/* compute transform matrix from src rectangle to dst parallelogram */
	if (!CombineTransform(&xform, &xform1, &xform2))
	{
		SetLastErrorEx(1, 0);
		return (DWORD)((BOOL)FALSE);
	}

	LOGSTR((LF_LOG,
		" xform from src rectangle to dst parallelogram:\n"
		" | eM11 eM12 0 |   | %6.2f %6.2f 0 |\n"
		" | eM21 eM22 0 | = | %6.2f %6.2f 0 |\n"
		" | eDx  eDy  1 |   | %6.2f %6.2f 1 |\n",
		xform.eM11, xform.eM12,
		xform.eM21, xform.eM22,
		xform.eDx,  xform.eDy));

	/* express transform matrix as scale/shear/shear operations:
	 *	| eM11 eM12 0 |   | a11   0 0 | | 1 b12 0 | |   1 0 0 |
	 *	| eM21 eM22 0 | = |   0 a22 0 | | 0   1 0 | | c21 1 0 |
	 *	|    0    0 1 |   |   0   0 1 | | 0   0 1 | |   0 0 1 |
	 * where
	 *	a11 = eM11 - (eM12 * eM21) / eM22
	 *	a22 = eM22
	 *	b12 = eM12 / a11
	 *	c21 = eM21 / a22
	 */
	if (xform.eM22 == 0.0)
	{
		SetLastErrorEx(1, 0);
		return (DWORD)((BOOL)FALSE);
	}
	a11 = xform.eM11 - (xform.eM12 * xform.eM21) / xform.eM22;
	a22 = xform.eM22;
	if (a11 == 0.0)
	{
		SetLastErrorEx(1, 0);
		return (DWORD)((BOOL)FALSE);
	}
	b12 = xform.eM12 / a11;
	c21 = xform.eM21 / a22;

	LOGSTR((LF_LOG,
		" xform expressed as scale/shear/shear operations:\n"
		" | eM11 eM12 0 |   | %6.2f %6.2f 0 | | %6.2f %6.2f 0 |"
						    " | %6.2f %6.2f 0 |\n"
		" | eM21 eM22 0 | = | %6.2f %6.2f 0 | | %6.2f %6.2f 0 |"
						    " | %6.2f %6.2f 0 |\n"
		" | 0    0    1 |   | %6.2f %6.2f 1 | | %6.2f %6.2f 1 |"
						    " | %6.2f %6.2f 1 |\n",
		a11, 0.0, 1.0, b12, 1.0, 0.0,
		0.0, a22, 0.0, 1.0, c21, 1.0,
		0.0, 0.0, 0.0, 0.0, 0.0, 0.0));

	/* allocate worldblt source buffer */
	if (!(worldblt = alloc_worldblt(lpDC->StretchBltMode,
		(int) xform.eDx, (int) xform.eDy,
		lpWorldBltParam->lpSrcDC,
		lpWorldBltParam->nSrcX, lpWorldBltParam->nSrcY,
		lpWorldBltParam->nSrcWidth, lpWorldBltParam->nSrcHeight)))
	{
		SetLastErrorEx(1, 0);
		return (DWORD)((BOOL)FALSE);
	}

	/* perform scale operation
	 *	| a11   0   0 |
	 *	|   0 a22   0 |
	 *	|   0   0   1 |
	 */
	if (!(worldblt = scaleblt(worldblt, a11, a22)))
	{
		SetLastErrorEx(1, 0);
		return (DWORD)((BOOL)FALSE);
	}

	/* perform shear operation
	 *	|   1 b12   0 |
	 *	|   0   1   0 |
	 *	|   0   0   1 |
	 */
	if (!(worldblt = xshearblt(worldblt, b12)))
	{
		SetLastErrorEx(1, 0);
		return (DWORD)((BOOL)FALSE);
	}

	/* perform shear operation
	 *	|   1   0   0 |
	 *	| c21   1   0 |
	 *	|   0   0   1 |
	 */
	if (!(worldblt = yshearblt(worldblt, c21)))
	{
		SetLastErrorEx(1, 0);
		return (DWORD)((BOOL)FALSE);
	}

#ifdef OLD_CODE_NOT_VALID
	bSuccess = DRVCALL_GRAPHICS(PGH_BITBLT, lpDC->lpDrvData, );
	return DRVCALL_GRAPHICS(PGH_BITBLT, lpDC->lpDrvData, dwParam, lpStruct);
#endif

	bSuccess = TRUE;

	/* free worldblt source buffer */
	free_worldblt(&worldblt);

	/* */
	return (bSuccess);

}

DWORD
lsd_display_systempaletteuse(WORD wMsg,
	LPDC lpDC,
	DWORD dwParam,
	LPLSDS_PARAMS lpParams)
{
	UINT uSystemPaletteUse = (UINT) dwParam;
	BOOL bSetSystemPaletteUse = (BOOL) lpParams;

	LOGSTR((LF_LOG, "lsd_display_systempaletteuse: wMsg %x lpDC %p\n"
		" uSystemPaletteUse %d bSetSystemPaletteUse %d\n",
		wMsg, lpDC, uSystemPaletteUse, bSetSystemPaletteUse));

	/* PLH_SYSTEMPALETTEUSE - (UINT) get/set system palette use
	 * dwParam1 - (LPDRIVERDC) lpDriverDC
	 * dwParam2 - (UINT) uSystemPaletteUse
	 *            SYSPAL_STATIC - 20 static colors (16 VGA colors + 4)
	 *            SYSPAL_NOSTATIC - 2 static colors (black / white)
	 * lpParams - (BOOL) bSetSystemPaletteUse
	 *            TRUE - set new system palette use
	 *                   get old system palette use
	 *            FALSE - get system palette use
	 */
	return (UINT) DRVCALL_COLORS(PLH_SYSTEMPALETTEUSE,
		(DWORD) lpDC->lpDrvData,
		(DWORD) uSystemPaletteUse,
		(LPVOID) bSetSystemPaletteUse);

}

DWORD
lsd_display_getsystempalette(WORD wMsg,
	LPDC lpDC,
	DWORD dwParam,
	LPLSDS_PARAMS lpParams)
{
	LOGSTR((LF_LOG, "lsd_display_getsystempalette: wMsg %x lpDC %p\n",
		wMsg, lpDC));

	/* PLH_GETSYSTEMPALETTE - (HPALETTE) get system palette
	 * dwParam1 - (LPDRIVERDC) lpDriverDC
	 * dwParam2 - (unused)
	 * lpParams - (unused)
	 */
	return (HPALETTE) DRVCALL_COLORS(PLH_GETSYSTEMPALETTE,
		(DWORD) lpDC->lpDrvData,
		(DWORD) dwParam,
		(LPVOID) lpParams);

}

DWORD
lsd_display_updatecolors(WORD wMsg,
	LPDC lpDC,
	DWORD dwParam,
	LPLSDS_PARAMS lpParams)
{
	LOGSTR((LF_LOG, "lsd_display_updatecolors: wMsg %x lpDC %p\n",
		wMsg, lpDC));

	/* PGH_UPDATECOLORS - (BOOL) update colors after palette change
	 * dwParam1 - (LPDRIVERDC) lpDriverDC
	 * dwParam2 - (unused)
	 * lpParams - (unused)
	 */
	return (BOOL) DRVCALL_COLORS(PLH_UPDATECOLORS,
		(DWORD) lpDC->lpDrvData,
		(DWORD) dwParam,
		(LPVOID) lpParams);

}
