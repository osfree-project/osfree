/*    
	DisplayObjects.c	2.26
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

#include "windows.h"

#include "kerndef.h"
#include "GdiObjects.h"
#include "Log.h"
#include "GdiDC.h"
#include "GdiRegions.h"
#include "DeviceData.h"
#include "Driver.h"

#include "compat.h"

extern BOOL 	GdiSelectFont(HDC32, HFONT);
extern HBITMAP  GdiDitherColor(HDC,COLORREF,int);
extern UINT 	GetCompatibilityFlags(int);

extern COLORREF TWIN_PaletteRGB(HDC32,COLORREF);
extern RGBQUAD VGAColors[];

BOOL GdiSelectBrush(HDC32,LPBRUSHINFO);

DWORD
lsd_display_selectcliprgn(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    HRGN hRgn;
    LPREGIONINFO lpRgnInfo;

    if ((hRgn = (HRGN)dwParam)) {
	if (!(lpRgnInfo = GETREGIONINFO(hRgn)))
	    return (DWORD)ERROR;
	DRVCALL_DC(PDCH_SETCLIP,lpRgnInfo->Rgn32,0L,hDC32->lpDrvData);
	RELEASEREGIONINFO(lpRgnInfo);
	return SIMPLEREGION;
    }
    else {
	DRVCALL_DC(PDCH_SETCLIP,0L,0L,hDC32->lpDrvData);
	return NULLREGION;
    }
}

DWORD
lsd_display_selectobject(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    HANDLE hObject,hLast = 0;
    WORD   wObjType = 0;
    LPOBJHEAD   lpObjInfo = 0;

    if (!(hObject = (HANDLE)dwParam)) {
	return 0L;
    }

    if (!(lpObjInfo = GETGDIINFO(hObject))) {
	return 0L;
    }
    wObjType= GET_OBJECT_TYPE(lpObjInfo);

    switch(wObjType) {

	case OT_BITMAP:
	    hLast = hDC32->hBitmap;
	    if (hLast != hObject) {
		LPIMAGEINFO lpImageInfo;
		LSDS_PARAMS argptr;

		hDC32->hBitmap = hObject;
		lpImageInfo = (LPIMAGEINFO)lpObjInfo;
		argptr.lsde_validate.dwInvalidMask = 0;
		argptr.lsde.imagedata.lpimagedata = lpImageInfo->lpDrvData;
		DRVCALL_GRAPHICS(PGH_SELECTIMAGE,
            		(DWORD)hDC32->lpDrvData, 0L, &argptr);
		hDC32->dwInvalid |= IM_BRUSHDEPTHMASK;
	    }
	    break;

	case OT_BRUSH: 
	    hLast = hDC32->hBrush;
	    if (hLast != hObject) {
		hDC32->hBrush = hObject;
		hDC32->dwInvalid |= IM_BRUSHMASK;
	    }
	    break;

	case OT_PEN:
	    hLast = hDC32->hPen;
	    if (hLast != hObject) {
		LPPENINFO lpPenInfo,lpOldPenInfo;
		if (!(lpPenInfo = GETPENINFO(hObject))) {
		    RELEASEGDIINFO(lpObjInfo);
		    return 0;
		}
		lpOldPenInfo = (hLast)?GETPENINFO(hLast):0;
		hDC32->hPen = hObject;
		if (!lpOldPenInfo)
		    hDC32->dwInvalid |= 
			IM_PENSTYLEMASK|IM_PENWIDTHMASK|IM_PENCOLORMASK;
		else {
		    if (lpPenInfo->lpPen.lopnStyle !=
				lpOldPenInfo->lpPen.lopnStyle)
			hDC32->dwInvalid |= IM_PENSTYLEMASK;
		    if (lpPenInfo->lpPen.lopnWidth.x !=
				lpOldPenInfo->lpPen.lopnWidth.x ||
			lpPenInfo->lpPen.lopnWidth.y != 
				lpOldPenInfo->lpPen.lopnWidth.y)
			hDC32->dwInvalid |= IM_PENWIDTHMASK;
		    if (lpPenInfo->lpPen.lopnColor !=
				lpOldPenInfo->lpPen.lopnColor)
			hDC32->dwInvalid |= IM_PENCOLORMASK;
		RELEASEPENINFO(lpOldPenInfo);
		}
	    RELEASEPENINFO(lpPenInfo);
	    }
	    break;

	case OT_FONT:
	    if (!GdiSelectFont(hDC32,(HFONT)hObject)) {
		RELEASEGDIINFO(lpObjInfo);
		return 0;
	    }
	    hLast = hDC32->hFont;
	    hDC32->hFont = hObject;
	    break;

	case OT_REGION:
	    RELEASEGDIINFO(lpObjInfo);
	    return lsd_display_selectcliprgn(LSD_SELECTCLIPRGN,hDC32,
			(DWORD)hObject,NULL);

	default:
	    RELEASEGDIINFO(lpObjInfo);
	    return 0;
    }

    LOCKGDI(hObject);

    if (hLast)
	UNLOCKGDI(hLast);

    RELEASEGDIINFO(lpObjInfo);
    return (DWORD)hLast;
}

DWORD
lsd_display_enumobjects(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LSDE_ENUMOBJ *lpeo = &lpStruct->lsde.enumobj;
    int fnObjectType = lpeo->fnObjectType;
    int nStockObject,nRetCode = 0;
    HPEN hPen;
    LPPENINFO lpPenInfo;
    HBRUSH hBrush;
    LPBRUSHINFO lpBrushInfo;

    switch (fnObjectType) {
	case OBJ_PEN:
	    for (nStockObject = WHITE_PEN;
		 nStockObject <= NULL_PEN;
		 nStockObject++) {
		hPen = GetStockObject(nStockObject);
		if ((lpPenInfo = GETPENINFO(hPen))) {
		    nRetCode = lpeo->goenmprc((LPVOID)&lpPenInfo->lpPen,
			lpeo->lParam);
		    RELEASEPENINFO(lpPenInfo);
		    if (!nRetCode)
			break;
		} else
		    break;
	    }
	    return nRetCode;

	case OBJ_BRUSH:
	    for (nStockObject = WHITE_BRUSH;
		 nStockObject <= NULL_BRUSH;
		 nStockObject++) {
		hBrush = GetStockObject(nStockObject);
		if ((lpBrushInfo = GETBRUSHINFO(hBrush))) {
		    nRetCode = lpeo->goenmprc(
			(LPVOID)&lpBrushInfo->lpBrush,lpeo->lParam);
		    RELEASEBRUSHINFO(lpBrushInfo);
		    if (!nRetCode)
			return 0;
		} else
		    return 0;
	    }
	    for (nStockObject = HS_HORIZONTAL;
		 nStockObject <= HS_DIAGCROSS;
		 nStockObject++) {
		hBrush = CreateHatchBrush(nStockObject,RGB(0,0,0));
		if ((lpBrushInfo = GETBRUSHINFO(hBrush))) {
		    nRetCode = lpeo->goenmprc(
			(LPVOID)&lpBrushInfo->lpBrush,lpeo->lParam);
		    RELEASEBRUSHINFO(lpBrushInfo);
		    DeleteObject(hBrush);
		    if (!nRetCode)
			break;
		} else
		    return 0;
	    }
	    return nRetCode;

	default:
	    return 0L;
    }
}

BOOL
GdiSelectBrush(HDC32 hDC32, LPBRUSHINFO lpBrushInfo)
{
    UINT uiCompatibility;
    BOOL bDither;
    COLORREF crColor,crTemp;
    LPIMAGEINFO lpImageInfo;
    HBITMAP hBitmap;
    int nDepth = 0;
    WORD wBrushType;
    int i;

    if (lpBrushInfo->lpBrush.lbColor & PALETTE_MASK)
	crColor = TWIN_PaletteRGB(hDC32, lpBrushInfo->lpBrush.lbColor);
    else
	crColor = lpBrushInfo->lpBrush.lbColor;

    if (lpBrushInfo->lpBrush.lbStyle == BS_SOLID) {
	wBrushType = LOWORD(lpBrushInfo->dwFlags);
	if (!(lpBrushInfo->dwFlags & DCX_DISPLAY_DC))
	    /* we have to re-realize solid brush, dither if needed */
	    wBrushType = BFP_UNASSIGNED;
	if (hDC32->hBitmap) {
	    /* if the brush is realized, but its depth is different */
	    /* from the currently selected bitmap, it has to be re- */
	    /* realized */
	    lpImageInfo = GETBITMAPINFO(hDC32->hBitmap);
	    nDepth = lpImageInfo->BitsPixel;
	    if (nDepth != (int)LOBYTE(HIWORD(lpBrushInfo->dwFlags)))
		wBrushType = BFP_UNASSIGNED;
	    RELEASEBITMAPINFO(lpImageInfo);
	}

	if (wBrushType == BFP_UNASSIGNED) {
	    /* solid brush needs to be realized;		*/
	    /* gets resolved to either BFP_PIXEL (solid color)	*/
	    /* or BFP_PIXMAP (dithered pattern)		  	*/
	    if (nDepth == 1) {
		if ((crColor == RGB(0,0,0)) || crColor == RGB(255,255,255))
		    bDither = FALSE;
		else
		    bDither = TRUE;
	    }
	    else {
		uiCompatibility = GetCompatibilityFlags(0);
		if (!(uiCompatibility & WD_NODITHERPURE)) {
		    for (i = 0, bDither = TRUE; bDither && i < 16; i ++) 
			if (RGB(VGAColors[i].rgbRed,
				VGAColors[i].rgbGreen,
				VGAColors[i].rgbBlue) == crColor)
			    bDither = FALSE;
		}
		else {
		    crTemp = GetNearestColor(GETHDC16(hDC32),crColor);
		    if (lpBrushInfo->lpBrush.lbColor & PALETTE_RGB) {
			bDither = FALSE;
			crColor = crTemp;
		    }
		    else 
			bDither = (crColor != crTemp);
		}
	    }
	    if (!bDither) {
		wBrushType = BFP_PIXEL;
		lpBrushInfo->lpPrivate = (LPVOID)DRVCALL_COLORS
				(PLH_MAKEPIXEL,
				 crColor,
				 0L,0L);
	    }
	    else {
		hBitmap = GdiDitherColor
			(GETHDC16(hDC32),crColor,nDepth);
		lpBrushInfo->lpBrush.lbHatch = (int)hBitmap;
		wBrushType = BFP_PIXMAP;
		lpImageInfo = GETBITMAPINFO(hBitmap);
		if (!lpImageInfo) {
		    return FALSE;
		}
		lpBrushInfo->lpPrivate = (LPVOID)DRVCALL_IMAGES
				(PIH_CREATEBRUSH,
				BFP_PIXMAP,0L,
				lpImageInfo->lpDrvData);
		RELEASEBITMAPINFO(lpImageInfo);
	    }
	    lpBrushInfo->dwFlags = DCX_DISPLAY_DC | wBrushType;
	    lpBrushInfo->dwFlags |= (BYTE)nDepth << 16;
	}
    }
    if (!lpBrushInfo->fIsRealized) {
	LSDS_PARAMS argptr;
	argptr.lsde_validate.dwInvalidMask = 0;
	argptr.lsde.point = hDC32->ptBrushOrg;
	DRVCALL_GRAPHICS(PGH_SETBRUSHORG,
				(DWORD)hDC32->lpDrvData,0L, &argptr);
	lpBrushInfo->fIsRealized = TRUE;
    }
    return TRUE;
}
