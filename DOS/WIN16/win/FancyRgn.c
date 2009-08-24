/*    
	FancyRgn.c	2.12
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

/* This file contains APIs to create fancy regions.
   Standard Windows APIs:
	CreateEllipticRgn{Indirect}
	CreateRoundRectRgn
   Our extensions:
	CreateFloodRgn - to be added
	CreatePie/Chord...Rgn ???
*/

#include <string.h>

#include "windows.h"

#include "GdiDC.h"
#include "DeviceData.h"
#include "Log.h"
#include "GdiRegions.h"
#include "Driver.h"

struct  tagDIBInfo  {
    BITMAPINFO	*lpbDIBHeader ;
    LPSTR	lpcDIBBits;
    HGLOBAL	hDIB;
    HBITMAP	hDIBopBMP;
    int		nDIBByteWidth;
};
typedef struct tagDIBInfo DIBInfo;

#define	RGN_ERROR ((HRGN) -1)

static HRGN CreateFancyRgn(int, LPPOINT);
static BOOL CreateFancyDIB(WORD, LPPOINT, DIBInfo *);
static BOOL DestroyFancyDIB(DIBInfo *);
static void GenerateRegion(REGION32, LPPOINT, DIBInfo *);
static DWORD DrvCommonCreateFancyRgn(WORD, LPPOINT);

extern HRGN GdiCreateRegion(LPSTR);

HRGN    WINAPI 
CreateEllipticRgn(int left, int top, int right, int bottom)
{
    POINT arc[2];

    arc[0].x = left;
    arc[0].y = top;
    arc[1].x = right;
    arc[1].y = bottom;

    return CreateFancyRgn(LSD_ELLIPSE, arc);
}

HRGN    WINAPI 
CreateEllipticRgnIndirect(const RECT FAR* lpr)
{
    POINT arc[2];

    if (!lpr)
	return 0;

    arc[0].x = lpr->left;
    arc[0].y = lpr->top;
    arc[1].x = lpr->right;
    arc[1].y = lpr->bottom;
    return CreateFancyRgn(LSD_ELLIPSE, arc);
}

HRGN    WINAPI 
CreateRoundRectRgn(int left, int top, int right, int bottom, int w,int h)
{
    POINT arc[3];

    arc[0].x = left;
    arc[0].y = top;
    arc[1].x = right;
    arc[1].y = bottom;
    arc[2].x = w;
    arc[2].y = h;

    return CreateFancyRgn(LSD_ROUNDRECT, arc);
}

static HRGN
CreateFancyRgn(int nFunction, LPPOINT lpArc)
{
    POINT arc[2];
    int  x,y; 
    HRGN hRgnFancy;
    REGION32 Rgn32Fancy;
    WORD wDriverFunc;
    DWORD dwCaps,dwCapMask;

    switch(nFunction) {
	case LSD_ELLIPSE:
	    wDriverFunc = PRH_CREATEELLIPTICREGION;
	    dwCapMask = RSUB_CANDO_ELLIPTIC;
	    break;
	case LSD_ROUNDRECT:
	    wDriverFunc = PRH_CREATEROUNDRECTREGION;
	    dwCapMask = RSUB_CANDO_ROUNDRECT;
	    break;
	default:
	    return (HRGN)0;
    }

    arc[0] = lpArc[0];
    arc[1] = lpArc[1];

    x = arc[0].x;
    y = arc[0].y;

    /* make the rectangle 0 based, to match the DIB */
    OffsetRect((LPRECT)arc,-x,-y);

    dwCaps = DRVCALL_REGIONS(DSUBSYSTEM_GETCAPS, 0, 0, 0);

    if (dwCaps & dwCapMask)
	Rgn32Fancy =
	(REGION32)DRVCALL_REGIONS(wDriverFunc, 0, 0, arc);
    else
	Rgn32Fancy = (REGION32)DrvCommonCreateFancyRgn(nFunction,arc);

    if (!Rgn32Fancy)
	return (HRGN)0;

    hRgnFancy = GdiCreateRegion((LPSTR)Rgn32Fancy);

    OffsetRgn(hRgnFancy,x,y);

    return hRgnFancy;
}

/* This is the driver helper */
static DWORD
DrvCommonCreateFancyRgn(WORD wFunc, LPPOINT lpArc)
{
    REGION32 Rgn32;
    DIBInfo  monoDIB;

    if (!(Rgn32 = (REGION32)DRVCALL_REGIONS(PRH_CREATEREGION,0,0,0)))
	return 0;

    /* Create the mono bitmap of the object */
    if (!CreateFancyDIB(wFunc, lpArc, &monoDIB)) {
	(void)DRVCALL_REGIONS(PRH_DESTROYREGION,Rgn32,0,0);
	return 0;
    }

    /* Generate clipping region */
    GenerateRegion(Rgn32, lpArc, &monoDIB);

    /* Cleanup */
    DestroyFancyDIB(&monoDIB);

    return (DWORD)Rgn32;
}

/**************************************************************
 *
 * Walk a scanline to find a set bit
 * Optimize in four ways,
 *	keep bits in server normal format, ie. 
 *	 	don't do a getbitmap bits...
 *		manipulate server normal bits directly
 *	offset the rect added to the region on the fly...
 *		don't offset the whole region at the end
 *	walk the edge of the ellipse, do not re-scan each line...
 *	move the region code down, keep in server normal format
 *		don't convert from hRgn on each pixel
 *		call X region code directly...
 *		
 */
static void
GenerateRegion(REGION32 Rgn32, LPPOINT lpArc, DIBInfo *lpDIBInfo)
{
	int	x,y;
	POINT   pt;
	int	width,height;
	int	centerx,centery;
	int	delta;
	RECT	valid,*lprc;

	lprc = (LPRECT)lpArc;

	width = lprc->right/2;
	height = lprc->bottom/2;

	centerx = lprc->left + width;
	if(lprc->right & 1)
		centerx++;
	centery = lprc->top + height;
	if(lprc->bottom & 1)
		centery++;

	for(x=lprc->left,y=lprc->top;y <= height ;x++) {
		if( x==width) {
			y++;
			x = lprc->left;
			continue;
		}

		pt.x = x;
		pt.y = y;

		if (lpDIBInfo->lpcDIBBits[lpDIBInfo->nDIBByteWidth * pt.y + pt.x/8] &
					(0x80>>(pt.x % 8))) {
			delta = centerx - pt.x;
			valid.left = pt.x;
			valid.top  = pt.y;
			valid.right = centerx+delta;
			delta = centery - pt.y;
			valid.bottom = centery + delta;
			(void)DRVCALL_REGIONS(PRH_UNIONRECTWITHREGION,
			    Rgn32, 0, &valid);
			y++;
			x = lprc->left;
		}
	}
}

static BOOL
CreateFancyDIB(WORD wFunc, LPPOINT lpArc, DIBInfo *lpDIBInfo)
{
    HDC		hScrDC,hDIBopDC;
    HBITMAP	hDIBoldBMP;
    BITMAPINFO	bmiDIBop;
    BITMAP	bm;
    DWORD	dwDIBLen;
    int		width,height;
    RECT	*rp;

    rp = (LPRECT)lpArc;

    width = rp->right - rp->left;
    height = rp->bottom - rp->top;

    lpDIBInfo->lpbDIBHeader  = NULL;
    lpDIBInfo->lpcDIBBits    = NULL;
    lpDIBInfo->hDIB          = 0;
    lpDIBInfo->hDIBopBMP     = 0;
    lpDIBInfo->nDIBByteWidth = 0;

    if (!(hScrDC = GetDC((HWND)NULL)))
   	return FALSE;

    if (!(hDIBopDC = CreateCompatibleDC(hScrDC))) {
    	ReleaseDC((HWND)NULL, hScrDC);
    	return FALSE;
    }
    lpDIBInfo->hDIBopBMP = CreateCompatibleBitmap(hScrDC, width, height);

    hDIBoldBMP = SelectObject( hDIBopDC, lpDIBInfo->hDIBopBMP );
    if ((! lpDIBInfo->hDIBopBMP) || (! hDIBoldBMP)) {
   	DeleteObject(lpDIBInfo->hDIBopBMP);
    	ReleaseDC((HWND)NULL, hScrDC);
    	return FALSE;
    }

    /* fill with black first */
    PatBlt( hDIBopDC,   0,0,   width, height, BLACKNESS );

    /* draw the shape */
    /* we will draw the shape inside of a 0 based rectangle or ellipse */
    if(wFunc == LSD_ELLIPSE) {
	Ellipse(hDIBopDC,0,0,rp->right-rp->left,rp->bottom-rp->top);
    } else {
	RoundRect(hDIBopDC,0,0,rp->right-rp->left,rp->bottom-rp->top,
			lpArc[2].x,lpArc[2].y);
    }

    SelectObject( hDIBopDC, hDIBoldBMP );        /* unselect */

    GetObject(lpDIBInfo->hDIBopBMP, sizeof(BITMAP), (LPSTR)&bm);
    memset((LPVOID)&bmiDIBop, '\0', sizeof(BITMAPINFO));
    bmiDIBop.bmiHeader.biSize   = sizeof(BITMAPINFOHEADER);
    bmiDIBop.bmiHeader.biWidth  = bm.bmWidth;  	/* rect.right; */
    bmiDIBop.bmiHeader.biHeight	= bm.bmHeight; 	/* rect.bottom; */
    bmiDIBop.bmiHeader.biPlanes	= 1;
    bmiDIBop.bmiHeader.biBitCount = 1; 		/* mono */
    bmiDIBop.bmiHeader.biCompression = BI_RGB;	/* no RLE */
    /* rest is 0-ed */

    /* default palette is already selected */
    GetDIBits( hScrDC, lpDIBInfo->hDIBopBMP,	/* bmp is not selected! */
               0, bm.bmHeight,			/* scan lines */
               (LPVOID)NULL,			/* just get info */
               (BITMAPINFO *)&bmiDIBop,
               DIB_RGB_COLORS );

    lpDIBInfo->nDIBByteWidth = bmiDIBop.bmiHeader.biSizeImage / bm.bmHeight;

    dwDIBLen = bmiDIBop.bmiHeader.biSizeImage +
    		sizeof(BITMAPINFO)+
    		2 /*numColors*/ * sizeof(RGBQUAD); /* palette size */

    lpDIBInfo->hDIB = GlobalAlloc(GHND,dwDIBLen);   /* 0-ed */
    if (lpDIBInfo->hDIB) {
	BOOL ok = FALSE;

	lpDIBInfo->lpbDIBHeader = (BITMAPINFO *)GlobalLock(lpDIBInfo->hDIB);
	if (lpDIBInfo->lpbDIBHeader) {
	    lpDIBInfo->lpcDIBBits = ((LPSTR) lpDIBInfo->lpbDIBHeader +
				   sizeof(BITMAPINFO)+2*sizeof(RGBQUAD));
	    *(lpDIBInfo->lpbDIBHeader) = bmiDIBop;  /* cpy header */

       	    if (bm.bmHeight == GetDIBits( hScrDC, lpDIBInfo->hDIBopBMP,
              			0, bm.bmHeight,
              			(LPVOID)(lpDIBInfo->lpcDIBBits),
              			(BITMAPINFO *)lpDIBInfo->lpbDIBHeader,
              			DIB_RGB_COLORS ) ) {

              	ok = TRUE;
	    }
	}
	if (!ok) {
	    DeleteObject(lpDIBInfo->hDIBopBMP);	/* bmp is not selected */
	    lpDIBInfo->lpbDIBHeader = NULL;
            lpDIBInfo->lpcDIBBits   = NULL;
            GlobalUnlock(lpDIBInfo->hDIB);
            GlobalFree(lpDIBInfo->hDIB);
            lpDIBInfo->hDIB = 0;
	}
    }

    DeleteDC(hDIBopDC);
    ReleaseDC((HWND)NULL,hScrDC);
    if (lpDIBInfo->hDIB)
   	return TRUE;
    else
   	return FALSE;
}

static BOOL
DestroyFancyDIB(DIBInfo *lpDIBInfo)
{
    DeleteObject(lpDIBInfo->hDIBopBMP);
    lpDIBInfo->lpbDIBHeader = NULL;
    lpDIBInfo->lpcDIBBits   = NULL;
    if (lpDIBInfo->hDIB) {
   	GlobalUnlock(lpDIBInfo->hDIB);
   	GlobalFree(lpDIBInfo->hDIB);
   	lpDIBInfo->hDIB = 0;
   	return TRUE;
    }
    return FALSE;
}
