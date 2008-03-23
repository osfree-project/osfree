/*    
	GdiClipping.c	2.22
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

#include "GdiDC.h"
#include "GdiRegions.h"
#include "Driver.h"
#include "DeviceData.h"
#include "Log.h"

static int GdiCombineClip(HDC32,LPRECT,HRGN,int);
static int GdiOffsetClipRgn(HDC32, LPSIZE);

extern HRGN TWIN_CalcVisRgn(HWND, DWORD);

int WIN32API
GetClipRgn(HDC hDC, HRGN hRgn)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    int nRet;

    APISTR((LF_APICALL,"GetClipRgn(HDC=%x,HRGN=%x)\n",hDC,hRgn));

    ASSERT_HDC(hDC32,hDC,-1);
    if (!hRgn)
	nRet = -1;
    else {

	argptr.lsde_validate.dwInvalidMask = 0;
	argptr.lsde.region = hRgn;

	nRet = (int)DC_OUTPUT(LSD_GETCLIPRGN,hDC32,0,&argptr);
    }

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"GetClipRgn: returns int %d\n",nRet));
    return nRet;
}

/* GetClipRgn16() is the undocumented Win 3.1 API GetClipRgn() */
HRGN WINAPI
GetClipRgn16(HDC hDC)
{
    HRGN hRgn;

    APISTR((LF_APICALL,"GetClipRgn(HDC=%x)\n",hDC));

    hRgn = CreateRectRgn(0,0,0,0);
    if ( hRgn )
	GetClipRgn(hDC, hRgn);

    APISTR((LF_APIRET,"GetClipRgn16: returns HRGN %x\n",hRgn));
    return hRgn;
}

int WINAPI
GetClipBox(HDC hDC, LPRECT lpRect)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    int nRet;

    APISTR((LF_APICALL,"GetClipBox(HDC=%x,LPRECT=%p)\n", hDC,lpRect));

    ASSERT_HDC(hDC32,hDC,ERROR);

    argptr.lsde_validate.dwInvalidMask = 0;

    nRet = (int)DC_OUTPUT(LSD_GETCLIPBOX,hDC32,0,&argptr);

    if (nRet > ERROR) {
	if (nRet != NULLREGION)
	    *lpRect = argptr.lsde.rect;
	else
	    SetRectEmpty(lpRect);
    }

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"GetClipBox: returns int %d\n",nRet));
    return nRet;
}

int WINAPI
ExcludeClipRect(HDC hDC,int left,int top,int right,int bottom)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    int nRet;

    APISTR((LF_APICALL,"ExcludeClipRect(HDC=%x,int=%d,int=%d,int=%d,int=%d)\n",
	hDC,left,top,right,bottom));

    ASSERT_HDC(hDC32,hDC,ERROR);

    argptr.lsde_validate.dwInvalidMask = 0;

    SetRect(&argptr.lsde.rect,left,top,right,bottom);

    nRet = (int)DC_OUTPUT(LSD_EXCLUDECLIPRECT,hDC32,0,&argptr);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"ExcludeClipRect: returns int %d\n",nRet));
    return nRet;
}

int WINAPI
IntersectClipRect(HDC hDC,int left,int top,int right,int bottom)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    int nRet;

    APISTR((LF_APICALL,"IntersectClipRect(HDC=%x,int=%d,int=%d,int=%d,int=%d)\n",
	hDC,left,top,right,bottom));

    ASSERT_HDC(hDC32,hDC,ERROR);

    argptr.lsde_validate.dwInvalidMask = 0;

    SetRect(&argptr.lsde.rect,left,top,right,bottom);

    nRet = (int)DC_OUTPUT(LSD_INTERSECTCLIPRECT,hDC32,0,&argptr);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"IntersectClipRect: returns int %d\n",nRet));
    return nRet;
}

int WINAPI
OffsetClipRgn(HDC hDC,int xoff,int yoff)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    int nRet;

    APISTR((LF_APICALL,"OffsetClipRgn(HDC=%x,int=%d,int=%d)\n",
	hDC,xoff,yoff));

    ASSERT_HDC(hDC32,hDC,ERROR);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.size.cx = xoff;
    argptr.lsde.size.cy = yoff;

    nRet = (int)DC_OUTPUT(LSD_OFFSETCLIPRGN,hDC32,0,&argptr);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"OffsetClipRgn: returns int %d\n",nRet));
    return nRet;
}

/* LSD_EXCLUDECLIPRECT and LSD_INTERSECTCLIPRECT come here */
DWORD
lsd_excludecliprect(WORD msg,HDC32 hDC32,DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    RECT rc = lpStruct->lsde.rect;

    /* mapmode transform is done here because mm_ routines take DC origin */
    /* into account, whereas LPtoDP does not */
    LPtoDP(GETHDC16(hDC32),(LPPOINT)&rc,2);

    return (DWORD)GdiCombineClip(hDC32,&rc,(HRGN)0,
		(msg == LSD_EXCLUDECLIPRECT)?RGN_DIFF:RGN_AND);
}

int
ExcludeClipRgn(HDC hDC, HRGN hRgn)
{
    HDC32 hDC32;
    int nRet = ERROR;

    APISTR((LF_APICALL,"ExcludeClipRgn(HDC=%x,HRGN=%x)\n",hDC,hRgn));

    ASSERT_HDC(hDC32,hDC,ERROR);

    if (hRgn)
	nRet = GdiCombineClip(hDC32, (LPRECT)0, hRgn, RGN_DIFF);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"ExcludeClipRgn: returns int %d\n",nRet));
    return nRet;
}

int
IntersectClipRgn(HDC hDC, HRGN hRgn)
{
    HDC32 hDC32;
    int nRet = ERROR;

    APISTR((LF_APICALL,"IntersectClipRgn(HDC=%x,HRGN=%x)\n",hDC,hRgn));

    ASSERT_HDC(hDC32,hDC,ERROR);

    if (hRgn)
	nRet = GdiCombineClip(hDC32, (LPRECT)0, hRgn, RGN_AND);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"IntersectClipRgn: returns int %d\n",nRet));
    return nRet;
}

static int
GdiCombineClip(HDC32 hDC32, LPRECT lpRect, HRGN hRgnInput,int fCombineMode)
{
    HDC hDC;
    HRGN hRgn,hClipRgn;
    int ret;

    hDC = GETHDC16(hDC32);

    if (lpRect) 
	hRgn = CreateRectRgnIndirect(lpRect);
    else 
	hRgn = CreateRectRgn(0,0,0,0);

    if (hRgnInput)
	CombineRgn(hRgn,hRgnInput,(HRGN)0,RGN_COPY);

    hClipRgn = CreateRectRgn(0,0,0,0);
    if (GetClipRgn(hDC,hClipRgn) == 0) {
	if (!hDC32->hWnd) {
	    SetRectRgn(hClipRgn,-32768,-32768,32767,32767);
	}
	else {
	    DeleteObject(hClipRgn);
	    hClipRgn = TWIN_CalcVisRgn(hDC32->hWnd, hDC32->dwDCXFlags);
	}
	ret = CombineRgn(hRgn,hClipRgn,hRgn,fCombineMode);
    }
    else
	ret = CombineRgn(hRgn,hClipRgn,hRgn,fCombineMode);

    SelectClipRgn(hDC,hRgn);

    DeleteObject(hRgn);
    DeleteObject(hClipRgn);

    return ret;
}

/* LSD_OFFSETCLIPRGN comes here */
DWORD
lsd_offsetcliprgn(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return (DWORD)GdiOffsetClipRgn(hDC32,&(lpStruct->lsde.size));
}

static int 
GdiOffsetClipRgn(HDC32 hDC32, LPSIZE lpSize)
{
    HDC hDC;
    HRGN hClipRgn;
    int ret;

    hDC = GETHDC16(hDC32);

    LEtoDE(hDC32,lpSize);

    hClipRgn = CreateRectRgn(0,0,0,0);

    if (GetClipRgn(hDC,hClipRgn) == 0)
	ret = SIMPLEREGION;
    else {
	OffsetRgn(hClipRgn,lpSize->cx,lpSize->cy);
	SelectClipRgn(hDC,hClipRgn);
	ret = COMPLEXREGION;
    }

    DeleteObject(hClipRgn);

    return ret;
}

BOOL WINAPI
PtVisible(HDC hDC, int x, int y)
{
    HDC32 hDC32;
    RECT rcRect;
    POINT pt;
    HRGN hRgn;
    BITMAP Bitmap;
    BOOL bRet = TRUE;

    APISTR((LF_APICALL,"PtVisible(HDC=%x,int=%d,int=%d)\n",hDC,x,y));

    ASSERT_HDC(hDC32,hDC,FALSE);

    pt.x = x;
    pt.y = y;
    LPtoDP(hDC,&pt,1);

    hRgn = CreateRectRgn(0,0,0,0);

    if (GetClipRgn(hDC,hRgn) > 0)
	bRet = PtInRegion(hRgn,pt.x,pt.y);
    else {
	if (hDC32->hWnd) 
	    GetClientRect(hDC32->hWnd,&rcRect);
	else if (hDC32->dwDCXFlags & DCX_COMPATIBLE_DC) {
	    GetObject(hDC32->hBitmap,sizeof(BITMAP),(LPVOID)&Bitmap);
	    SetRect(&rcRect,0,0,Bitmap.bmWidth,Bitmap.bmHeight);
	}
	else {	/* desktop DC */
	    SetRect(&rcRect,0,0,GetSystemMetrics(SM_CXSCREEN),
				GetSystemMetrics(SM_CYSCREEN));
	    pt.x += hDC32->DOx;
	    pt.y += hDC32->DOy;
	}
	bRet = PtInRect(&rcRect,pt);
    }

    DeleteObject(hRgn);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"PtVisible: returns BOOL %d\n",bRet));
    return bRet;
}

BOOL WINAPI
RectVisible(HDC hDC, const RECT *lpRect)
{
    HDC32 hDC32;
    HRGN hRgn;
    BOOL bRet = TRUE;
    RECT rcRect,rcDevice;
    BITMAP Bitmap;

    APISTR((LF_APICALL,"RectVisible(HDC=%x,RECT *%p)\n", hDC,lpRect));

    if (!lpRect) {
    	APISTR((LF_APIFAIL,"PtVisible: returns BOOL %d\n",FALSE));
	return FALSE;
    }

    ASSERT_HDC(hDC32,hDC,FALSE);

    CopyRect(&rcDevice,lpRect);
    LPtoDP(hDC,(POINT *)&rcDevice,2);

    hRgn = CreateRectRgn(0,0,0,0);

    if (GetClipRgn(hDC,hRgn) > 0)
	bRet = RectInRegion(hRgn,&rcDevice);
    else {
	if (hDC32->hWnd) 
	    GetClientRect(hDC32->hWnd,&rcRect);
	else if (hDC32->dwDCXFlags & DCX_COMPATIBLE_DC) {
	    GetObject(hDC32->hBitmap,sizeof(BITMAP),(LPVOID)&Bitmap);
	    SetRect(&rcRect,0,0,Bitmap.bmWidth,Bitmap.bmHeight);
	}
	else {
	    SetRect(&rcRect,0,0,GetSystemMetrics(SM_CXSCREEN),
				GetSystemMetrics(SM_CYSCREEN));
	    OffsetRect(&rcDevice,hDC32->DOx,hDC32->DOy);
	}
	bRet = IntersectRect(&rcRect,&rcDevice,&rcRect);
    }

    DeleteObject(hRgn);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"RectVisible: returns BOOL %d\n",bRet));
    return bRet;
}

int WINAPI
SelectClipRgn(HDC hDC,HRGN hRegion)
{
    HDC32 hDC32;
    int nRet;

    APISTR((LF_APICALL,"SelectClipRgn(HDC=%x,HRGN=%x)\n",hDC,hRegion));

    ASSERT_HDC(hDC32,hDC,0);

    nRet = (int)DC_OUTPUT(LSD_SELECTCLIPRGN,hDC32,hRegion,0);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"SelectClipRgn: returns int %d\n",nRet));
    return nRet;
}

int	WINAPI
ExtSelectClipRgn(HDC hDC, HRGN hRgn, int nClipMode)
{
	HDC32 hDC32;
	int nClipResult = 0;

	APISTR((LF_APICALL, "ExtSelectClipRgn(HDC=%x,HRGN=%x,int=%d)\n",
		hDC, hRgn, nClipMode));

	ASSERT_HDC(hDC32, hDC, 0);

	RELEASEDCINFO(hDC32);

        APISTR((LF_APIRET,"ExtSelectClipRgn: returns int %d\n",nClipResult));
	return (nClipResult);

}

/* these undocumented GDI functions are only used for display DCs */
int
SelectVisRgn(HDC hDC, HRGN hVisRgn)
{
    HDC32 hDC32;
    REGION32 Rgn32;
    int nRet = ERROR;

    APISTR((LF_APICALL,"SelectVisRgn(HDC=%x,HRGN=%x)\n",hDC,hVisRgn));
    ASSERT_HDC(hDC32,hDC,ERROR);

/* HOW DOES Rgn32 GET RELEASED ? */
    if ((Rgn32 = TWIN_InternalGetRegionData(hVisRgn))) {
	(void)DRVCALL_DC(PDCH_SETVISRGN,0, Rgn32, hDC32->lpDrvData);
	nRet = COMPLEXREGION;
    }

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"SelectVisRgn: returns int %d\n",nRet));
    return nRet;
}

