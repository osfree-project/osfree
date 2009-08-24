/*    
	GdiBrute.c	1.8
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

#include <stdio.h>

#include "windows.h"
#include "print.h"

#include "Log.h"
#include "GdiObjects.h"
#include "GdiDDK.h"
#include "PrinterDC.h"

static COLORREF MapColorToBW(COLORREF);
static HDC dmPrepareDC(LPBITMAP, LPDRAWMODE);

extern LONG TWIN_SetImageBits(HBITMAP, DWORD, const void *, int);
extern LONG TWIN_GetImageBits(HBITMAP, LONG, LPVOID, int);

int WINAPI
dmRealizeObject(LPVOID lpdv, int iStyle, LPSTR lpbIn, LPSTR lpbOut,
                LPTEXTXFORM lptf)
{
    LPPSBITMAP lpPSBitmap = (LPPSBITMAP)lpdv;
    LPBITMAP lpBitmap;

    LOGSTR((LF_LOG,"dmRealizeObject: called with object type %d\n",iStyle));

    switch (iStyle) {
	case DRV_OBJ_PBITMAP:
	    lpBitmap = (LPBITMAP)lpbIn;
	    LOGSTR((LF_LOG,"dmRealizeObject: bitmap %dX%d\n",
		lpBitmap->bmWidth,lpBitmap->bmHeight));
	    if (lpPSBitmap->bm.bmBits)
		WinFree((LPSTR)lpPSBitmap->bm.bmBits);
	    lpPSBitmap->bm = *lpBitmap;
	    break;
    }

    return 1;
}

BOOL WINAPI
dmBitBlt(LPVOID lpdv, int xDst, int yDst, LPBITMAP lpbmSrc, int xSrc, int ySrc,
        int cx, int cy, long rop, LPVOID lpbr, LPDRAWMODE lpdm)
{
    return dmStretchBlt(lpdv,xDst,yDst,cx,cy,
			lpbmSrc,xSrc,ySrc,cx,cy,
			rop,lpbr,lpdm);
}

BOOL WINAPI
dmStretchBlt(LPVOID lpdv, int xDst, int yDst, int cxDst, int cyDst,
		LPBITMAP lpbmSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
		long rop, LPVOID lpbr, LPDRAWMODE lpdm)
{
    LPPSBITMAP lpDest = (LPPSBITMAP)lpdv;
    LPPSBITMAP lpSrc = (LPPSBITMAP)lpbmSrc;
    LPBRUSHINFO lpBrushInfo = (LPBRUSHINFO)lpbr;
    HDC hDCDest,hDCSrc;
    BOOL rc;
    HBITMAP hBitmap;

    if (!lpDest || !lpDest->bm.bmBits || (lpSrc && !lpDest->bm.bmBits))
	return FALSE;

    if (lpDest->bm.bmBitsPixel != 1 || (lpSrc && lpSrc->bm.bmBitsPixel != 1))
	return FALSE;

    hDCDest = dmPrepareDC(&lpDest->bm, lpdm);

    SelectObject(hDCDest,(HBRUSH)lpBrushInfo->ObjHead.hObj);

    hDCSrc = (lpSrc)?dmPrepareDC(&lpSrc->bm,0):0;

    rc = StretchBlt(hDCDest,xDst,yDst,cxDst,cyDst,
		hDCSrc,xSrc,ySrc,cxSrc,cySrc,rop);

    SelectObject(hDCDest,GetStockObject(WHITE_BRUSH));

    hBitmap = SelectObject(hDCDest,GetStockObject(SYSTEM_BITMAP));
    DeleteDC(hDCDest);

    TWIN_GetImageBits(hBitmap,lpDest->bm.bmWidthBytes * lpDest->bm.bmHeight,
			lpDest->bm.bmBits,32);

    DeleteObject(hBitmap);

    if (hDCSrc) {
	hBitmap = SelectObject(hDCSrc,GetStockObject(SYSTEM_BITMAP));
	DeleteDC(hDCSrc);
	DeleteObject(hBitmap);
    }

    return rc;
}

int
dmScanLR(LPVOID lpdv, int ix, int iy, DWORD co, int iStyle)
{
	return(0);
}

int
dmOutput(LPVOID lpdv, int iStyle, int cpt, LPPOINT lppt, LPVOID lppen,
	LPVOID lpbr, LPDRAWMODE lpdm, LPRECT lprcClip)
{
    LPPSBITMAP lpPSBitmap = (LPPSBITMAP)lpdv;
    LPBITMAP lpbm;
    LPBRUSHINFO lpBrushInfo = (LPBRUSHINFO)lpbr;
    LPPENINFO lpPenInfo = (LPPENINFO)lppen;
    HDC hDC;
    HBITMAP hBitmap;
    BOOL retcode = 0;
    LOGPEN LogPen;
    HPEN hPen,hOldPen;

    if (!lpPSBitmap || !lpPSBitmap->bm.bmBits)
	return FALSE;

    lpbm = (LPBITMAP)lpPSBitmap;
    hDC = dmPrepareDC(lpbm,lpdm);

    switch (iStyle) {
	case OS_POLYLINE:
	    LogPen = lpPenInfo->lpPen;
	    LogPen.lopnColor = MapColorToBW(LogPen.lopnColor);
	    hPen = CreatePenIndirect(&LogPen);
	    hOldPen = SelectObject(hDC, hPen);
	    SetROP2(hDC, lpdm->Rop2);
	    retcode = Polyline(hDC, lppt, cpt);
	    SelectObject(hDC,hOldPen);
	    DeleteObject(hPen);
	    break;

	case OS_RECTANGLE:
	case OS_ELLIPSE:
	case OS_ROUNDRECT:
	case OS_ARC:
	case OS_PIE:
	case OS_CHORD:
	case OS_ALTPOLYGON:
	case OS_WINDPOLYGON:
	    LogPen = lpPenInfo->lpPen;
	    LogPen.lopnColor = MapColorToBW(LogPen.lopnColor);
	    hPen = CreatePenIndirect(&LogPen);
	    hOldPen = SelectObject(hDC, hPen);
	    SetROP2(hDC, lpdm->Rop2);
	    SelectObject(hDC,(HBRUSH)lpBrushInfo->ObjHead.hObj);

	    switch (iStyle) {
		case OS_RECTANGLE:
		    retcode = Rectangle(hDC,lppt[0].x,lppt[0].y,
					lppt[1].x,lppt[1].y);
		    break;
		case OS_ELLIPSE:
		    retcode = Ellipse(hDC,lppt[0].x,lppt[0].y,
					lppt[1].x,lppt[1].y);
		    break;
		case OS_ROUNDRECT:
		    retcode = RoundRect(hDC,lppt[0].x,lppt[0].y,
					lppt[1].x,lppt[1].y,
					lppt[2].x,lppt[2].y);
		    break;
		case OS_ARC:
		    retcode = Arc(hDC,lppt[0].x,lppt[0].y,
					lppt[1].x,lppt[1].y,
					lppt[2].x,lppt[2].y,
					lppt[3].x,lppt[3].y);
		    break;
		case OS_PIE:
		    retcode = Pie(hDC,lppt[0].x,lppt[0].y,
					lppt[1].x,lppt[1].y,
					lppt[2].x,lppt[2].y,
					lppt[3].x,lppt[3].y);
		    break;
		case OS_CHORD:
		    retcode = Chord(hDC,lppt[0].x,lppt[0].y,
					lppt[1].x,lppt[1].y,
					lppt[2].x,lppt[2].y,
					lppt[3].x,lppt[3].y);
		    break;
		case OS_ALTPOLYGON:
		    SetPolyFillMode(hDC,ALTERNATE);
		    retcode = Polygon(hDC,lppt,cpt);
		    break;
		case OS_WINDPOLYGON:
		    SetPolyFillMode(hDC,WINDING);
		    retcode = Polygon(hDC,lppt,cpt);
		    break;
	    }

	    SelectObject(hDC,hOldPen);
	    DeleteObject(hPen);
	    SelectObject(hDC,GetStockObject(WHITE_BRUSH));
	    break;

	default:
	    retcode = FALSE;
    }

    hBitmap = SelectObject(hDC,GetStockObject(SYSTEM_BITMAP));
    DeleteDC(hDC);

    TWIN_GetImageBits(hBitmap,lpbm->bmWidthBytes * lpbm->bmHeight,
			lpbm->bmBits,32);

    DeleteObject(hBitmap);

    return retcode;
}

DWORD
dmPixel(LPVOID lpdv, int ix, int iy, DWORD co, LPDRAWMODE lpdm)
{
    LPPSBITMAP lpPSBitmap = (LPPSBITMAP)lpdv;
    LPBITMAP lpbm;
    HDC hDC;
    HBITMAP hBitmap;
    COLORREF retcode;

    if (!lpPSBitmap || !lpPSBitmap->bm.bmBits)
	return FALSE;

    lpbm = (LPBITMAP)lpPSBitmap;
    hDC = dmPrepareDC(lpbm,lpdm);

    if (lpdm) {
	SetROP2(hDC, lpdm->Rop2);
	retcode = SetPixel(hDC,ix,iy,(COLORREF)co);
    }
    else
	retcode = GetPixel(hDC,ix,iy);

    hBitmap = SelectObject(hDC,GetStockObject(SYSTEM_BITMAP));
    DeleteDC(hDC);

    if (lpdm)
	TWIN_GetImageBits(hBitmap,lpbm->bmWidthBytes * lpbm->bmHeight,
			lpbm->bmBits,32);

    DeleteObject(hBitmap);

    return (DWORD)retcode;
}

long
dmStrBlt(LPVOID lpdv, int ix, int iy, LPRECT lprcClip, LPSTR lpbSrc,
        int cbSrc, LPVOID lpdf, LPDRAWMODE lpdm, LPTEXTXFORM lpft)
{
	return(0);
}

COLORREF
dmColorInfo(LPVOID lpdv, DWORD rgb, LPDWORD lpco)
{
    LPPSBITMAP lpPSBitmap = (LPPSBITMAP)lpdv;
    COLORREF cr;

    if (!lpPSBitmap)
	return 0;

    if (lpPSBitmap->bm.bmBitsPixel == 1)
	cr = MapColorToBW(rgb);
    else
	cr = rgb;

    if (lpco)
	*lpco = cr;

    return cr;
}

static HDC
dmPrepareDC(LPBITMAP lpbm, LPDRAWMODE lpdm)
{
    HBITMAP hBitmap;
    HDC hDC;
    DWORD dwSize;

    hDC = CreateCompatibleDC(0);
    hBitmap = CreateBitmap(lpbm->bmWidth,lpbm->bmHeight,1,
		lpbm->bmBitsPixel,NULL);
    dwSize = lpbm->bmWidthBytes * lpbm->bmHeight;
    SelectObject(hDC,hBitmap);
    TWIN_SetImageBits(hBitmap,dwSize,lpbm->bmBits,32);
    if (lpdm) {
	SetTextColor(hDC,MapColorToBW(lpdm->TextColor));
	SetBkColor(hDC,MapColorToBW(lpdm->bkColor));
    }

    return hDC;
}

static COLORREF
MapColorToBW(COLORREF cr)
{
    UINT r,g,b;

    r = GetRValue(cr);
    g = GetGValue(cr);
    b = GetBValue(cr);

    return (COLOR_INTENSITY(r,g,b) > 127)?
	RGB(255,255,255):RGB(0,0,0);
}
