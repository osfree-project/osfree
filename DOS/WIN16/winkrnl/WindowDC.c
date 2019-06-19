/*    
	WindowDC.c	2.44
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
#include "windowsx.h"

#include "WindowDC.h"
#include "kerndef.h"
#include "Driver.h"
#include "GdiDC.h"
#include "GdiDCAttr.h"
#include "GdiRegions.h"
#include "Log.h"
#include "WinDefs.h"

void TWIN_GenerateClip(HWND32, HRGN, DWORD, BOOL);
BOOL TWIN_ReparentDC(HWND);
BOOL TWIN_GdiDestroyDC(HDC);
BOOL TWIN_OffsetDCOrigins(HWND32, int, int);
BOOL TWIN_RecalcVisRgns(HWND32);
HRGN TWIN_CalcVisRgn(HWND, DWORD);

static void ClipChildren(HWND32, HRGN);
static void CalcDCOrigin(HDC32, HWND32, HWND);
static DWORD TWIN_GetDCXFlags(HDC);

HDC WINAPI
GetDC(HWND hWnd)
{
    HDC hDC;
    APISTR((LF_APICALL,"GetDC(HWND=%x)\n",hWnd));
    hDC =  GetDCEx(hWnd,(HRGN)0,DCX_DEFAULTCLIP);
    APISTR((LF_APIRET,"GetDC: returns HDC %x\n",hDC));
    return hDC;
}

HDC WINAPI
GetWindowDC(HWND hWnd)
{
    HDC hDC;
    APISTR((LF_APICALL,"GetWindowDC(HWND=%x)\n",hWnd));
    hDC =  GetDCEx(hWnd,(HRGN)0,DCX_DEFAULTCLIP|DCX_WINDOW|DCX_CACHE);
    APISTR((LF_APIRET,"GetWindowDC: returns HDC %x\n",hDC));
    return hDC;
}

static DWORD
TWIN_GetDCXFlags(HDC hDC)
{
    HDC32 hDC32;
    DWORD rv;

    ASSERT_HDC(hDC32,hDC,0);

    rv = hDC32->dwDCXFlags;

    RELEASEDCINFO(hDC32);
    return rv;
}

HWND WIN32API
WindowFromDC(HDC hDC)
{
    HDC32 hDC32;
    HWND hwnd;

    APISTR((LF_APICALL,"WindowFromDC(HDC=%x)\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    hwnd = hDC32->hWnd;

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"WindowFromDC: returns HWND %x\n",hwnd));
    return hwnd;
}

HDC WINAPI
GetDCEx(HWND hWnd, HRGN hRgnClip, DWORD dwDCXFlags)
{
    HDC hDC = 0;
    HDC32 hDC32=NULL;
    HWND32 hWnd32=NULL;
    DWORD dwClassStyle=0;
    HWND hWndDC = 0;
    HRGN hVisibleRgn = 0;
    DWORD dwVisibleRgnData = 0;
    DWORD dwUpdateRgnData = 0;
    BOOL bInitDisplayDC = TRUE;

    APISTR((LF_APICALL,"GetDCEx(HWND=%x,HRGN=%x,DWORD=%x)\n",
	hWnd,hRgnClip,dwDCXFlags));

    if (hWnd) {
	if (!(hWnd32 = GETHWND32(hWnd))) {
    	    APISTR((LF_APIFAIL,"GetDCEx: returns HDC 0\n"));
	    return (HDC)0;
	}
	dwClassStyle = GetClassStyle(hWnd);

	/* this behaviour can also be forced by */
	/* setting DCX_CLIPCHILDREN directly in dwDCXFlags */
	if (hWnd32->dwStyle & WS_CLIPCHILDREN)
	    dwDCXFlags |= DCX_CLIPCHILDREN;

	/* this behaviour can also be forced by */
	/* setting DCX_CLIPSIBLINGS directly in dwDCXFlags */
	if (hWnd32->dwStyle & WS_CLIPSIBLINGS)
	    dwDCXFlags |= DCX_CLIPSIBLINGS;
    }
	
    /* if GetDC(0) or forced cache DC or plain vanilla GetDC */
    if (
	/* no hWnd */
	(hWnd == 0) ||
	/* or forced cached DC (window DCs are also from the cache) */
	(dwDCXFlags & (DCX_CACHE|DCX_WINDOW)) ||
	/* or no funny class styles */
	!(dwClassStyle & (CS_OWNDC|CS_CLASSDC)) ||
	/* or own DC, and driver structures are not yet created */
	((dwClassStyle & CS_OWNDC) && !(hWnd32->dwStyle & WS_CHILD) && 
		!WIN_GETDRVDATA(hWnd32->hWndFrame))
       ) {
	/* we get a DC from the cache */
	if (!(hDC32 = GdiCreateHDC(TRUE))) {
	    if (hWnd32)
		RELEASEWININFO(hWnd32);
    	    APISTR((LF_APIFAIL,"GetDCEx: returns HDC 0\n"));
	    return (HDC)0;
	}
	hDC = GETHDC16(hDC32);

	/* now calculate DC scope: top-level frame or desktop */
	/* and clipping: no clip/parentlip/clientclip */
	if (!hWnd) {
	    /* for GetDC(0) use zero hWnd and no clipping */
	    hWndDC = 0;
	}
	else {
	    hWndDC = TWIN_GetTopLevelFrame(hWnd);
	    if (dwClassStyle & CS_PARENTDC ||
		(!(hWnd32->dwStyle & WS_CHILD) &&
		 !(dwDCXFlags & DCX_CLIPSIBLINGS)))
		dwDCXFlags |= DCX_PARENTCLIP;
	    else
		if (!(dwDCXFlags & DCX_PARENTCLIP))
		    dwDCXFlags |= DCX_CLIENTCLIP;
	}
	if ((dwDCXFlags & DCX_PARENTCLIP) &&
	    (hWnd == hWndDC || Get32WindowFrame(hWnd32) == hWndDC))
	    hWndDC = 0;

	hDC32->dwDCXFlags = dwDCXFlags | DCX_CACHE_DC | DCX_DISPLAY_DC;

	if (!(hDC32->lpDrvData = (LPVOID)DRVCALL_DC(PDCH_CREATE,
		hDC32->dwDCXFlags, 0L,
		(hWndDC)?WIN_GETDRVDATA(hWndDC):0))) {
	    RELEASEDCINFO(hDC32);
	    FREEGDI(hDC);
	    if (hWnd32)
		RELEASEWININFO(hWnd32);
    	    APISTR((LF_APIFAIL,"GetDCEx: returns HDC 0\n"));
	    return (HDC)0;
	}
    }
    else {
	/* go check class styles for CS_OWNDC/CS_CLASSDC */
	if (dwClassStyle & CS_OWNDC) {
	    /* for CS_OWNDC windows DC is gotten once */
	    /* and is kept in window structure */
	    hWndDC = TWIN_GetTopLevelFrame(hWnd);
	    if ((hDC = hWnd32->hDC) && (hDC32 = GETHDC32(hDC))) {
		bInitDisplayDC = FALSE;
		if ((hDC32->dwDCXFlags & DCX_PARENTCLIP) &&
		    (hWnd == hWndDC || Get32WindowFrame(hWnd32) == hWndDC))
		    hWndDC = 0;
	    }
	    else {
		/* create new own DC and stick it into the window */
		if (!(hDC32 = GdiCreateHDC(FALSE))) {

		    if (hWnd32)
			RELEASEWININFO(hWnd32);
    	    	    APISTR((LF_APIFAIL,"GetDCEx: returns HDC 0\n"));
		    return (HDC)0;
		}

		if ((dwClassStyle & CS_PARENTDC) ||
		    (!(hWnd32->dwStyle & WS_CHILD) &&
		     !(dwDCXFlags & DCX_CLIPSIBLINGS)))
		    dwDCXFlags |= DCX_PARENTCLIP;
		else
		    if (!(dwDCXFlags & DCX_PARENTCLIP))
			dwDCXFlags |= DCX_CLIENTCLIP;

		if ((dwDCXFlags & DCX_PARENTCLIP) &&
		    (hWnd == hWndDC || Get32WindowFrame(hWnd32) == hWndDC))
		    hWndDC = 0;

		hDC = GETHDC16(hDC32);
		hDC32->dwDCXFlags = dwDCXFlags | DCX_OWN_DC | DCX_DISPLAY_DC;

		if (!(hDC32->lpDrvData = (LPVOID)DRVCALL_DC(PDCH_CREATE,
			hDC32->dwDCXFlags, 0L,
			(hWndDC)?WIN_GETDRVDATA(hWndDC):0))) {
			
		    RELEASEDCINFO(hDC32);
		    FREEGDI(hDC);
		    if (hWnd32)
			RELEASEWININFO(hWnd32);
    	    	    APISTR((LF_APIFAIL,"GetDCEx: returns HDC 0\n"));
		    return (HDC)0;
		}

		hWnd32->hDC = hDC;
	    }
	} /* if (dwClassStyle & CS_OWNDC) */
	else if (dwClassStyle & CS_CLASSDC) {
	    /* for CS_CLASSDC windows class DC is gotten upon first request */
	    /* and is a desktop window DC offset and clipped as needed */
	    if ((hDC = GetClassDC(hWnd)) &&
		    (hDC32 = GETHDC32(hDC))) {
		bInitDisplayDC = FALSE;
	    }
	    else {
		/* we have to create a new object and stick it into class */
		if (!(hDC32 = GdiCreateHDC(FALSE))) {
		    if (hWnd32)
			RELEASEWININFO(hWnd32);
    	    	    APISTR((LF_APIFAIL,"GetDCEx: returns HDC 0\n"));
		    return (HDC)0;
		}
		hDC = GETHDC16(hDC32);
		dwDCXFlags |= DCX_PARENTCLIP;
		hDC32->dwDCXFlags = dwDCXFlags | DCX_CLASS_DC | DCX_DISPLAY_DC;

		/* this is a root window DC */
		if (!(hDC32->lpDrvData = (LPVOID)DRVCALL_DC(PDCH_CREATE,
			hDC32->dwDCXFlags, 0L, 0))) {
		    RELEASEDCINFO(hDC32);
		    FREEGDI(hDC);
		    if (hWnd32)
			RELEASEWININFO(hWnd32);
    	    	    APISTR((LF_APIFAIL,"GetDCEx: returns HDC 0\n"));
		    return (HDC)0;
		}

		SetClassDC(hWnd, hDC);
	    }
	    hWnd32->hDC = hDC;
	    hWndDC = 0;
	} /* if (dwClassStyle & CS_CLASSDC) */
    } /* not cache DC */

    hDC32->hWnd = hWnd;

    if (bInitDisplayDC)
	GdiInitDisplayDC(hDC32);

    if (hWnd == 0)
    {
	if (hWnd32)
	    RELEASEWININFO(hWnd32);
	RELEASEDCINFO(hDC32);
    	APISTR((LF_APIRET,"GetDCEx: returns HDC %x\n",hDC));
	return hDC;
    }

    /* now set the DC origin */
    CalcDCOrigin(hDC32,hWnd32,hWndDC);

    /*
     * Determine the visible region and get a pointer to the description of it
     */
    hVisibleRgn = TWIN_CalcVisRgn(hWnd, hDC32->dwDCXFlags);
    if (hVisibleRgn)
	dwVisibleRgnData = TWIN_InternalGetRegionData(hVisibleRgn);

    /*
     * If we have been asked to intersect with the update region, then
     * set the update region.  Otherwise ignore the current update region.
     */
    if (dwDCXFlags & DCX_INTERSECTUPDATE)
    {
	if (hWnd && !(hDC32->dwDCXFlags & DCX_WINDOW))
	    dwUpdateRgnData = GetWindowLong(hWnd, GWL_UPDATE);
	if (hRgnClip)
	{
	    DRVCALL_REGIONS(PRH_INTERSECTREGION, 
			    dwUpdateRgnData,
			    TWIN_InternalGetRegionData(hRgnClip),
			    dwUpdateRgnData);
	}
    }

    DRVCALL_DC(PDCH_SETUPDATECLIP, 
	       dwUpdateRgnData,
	       dwVisibleRgnData, 
	       hDC32->lpDrvData);
	
    if (hVisibleRgn)
	DeleteObject(hVisibleRgn);

    RELEASEDCINFO(hDC32);
    if (hWnd32)
	RELEASEWININFO(hWnd32);

    APISTR((LF_APIRET,"GetDCEx: returns HDC %x\n",hDC));
    return hDC;
}

BOOL
TWIN_ReparentDC(HWND hWnd)
{
    HWND32 hWnd32;
    HDC32 hDC32;
    DWORD dwClassStyle;
    HWND hWndDC;
    HRGN hRgn;
    DWORD dwDCXFlags;

    ASSERT_HWND(hWnd32,hWnd,FALSE);

    dwClassStyle = GetClassStyle(hWnd);

    if ((dwClassStyle & CS_OWNDC) && hWnd32->hDC) {
	ASSERT_HDC(hDC32,hWnd32->hDC,FALSE);

	dwDCXFlags = hDC32->dwDCXFlags;

	/* first get rid of the driver portion */
	DRVCALL_DC(PDCH_DELETE,0L,0L,hDC32->lpDrvData);

	hWndDC = TWIN_GetTopLevelFrame(hWnd);

	if ((dwClassStyle & CS_PARENTDC) ||
	    (!(hWnd32->dwStyle & WS_CHILD) &&
	     !(dwDCXFlags & DCX_CLIPSIBLINGS)))
	    dwDCXFlags |= DCX_PARENTCLIP;
	else
	    if (!(dwDCXFlags & DCX_PARENTCLIP))
		dwDCXFlags |= DCX_CLIENTCLIP;

	if ((dwDCXFlags & DCX_PARENTCLIP) &&
	    (hWnd == hWndDC || Get32WindowFrame(hWnd32) == hWndDC))
	    hWndDC = 0;

	hDC32->dwDCXFlags = dwDCXFlags | DCX_OWN_DC | DCX_DISPLAY_DC;

	hDC32->lpDrvData = (LPVOID)DRVCALL_DC(PDCH_CREATE,
			hDC32->dwDCXFlags, 0L,
			(hWndDC)?WIN_GETDRVDATA(hWndDC):0);

	hDC32->dwInvalid = (DWORD)-1;

	CalcDCOrigin(hDC32,hWnd32,hWndDC);

	hRgn = TWIN_CalcVisRgn(hWnd, hDC32->dwDCXFlags);

	DRVCALL_DC(PDCH_SETUPDATECLIP,
		(hWnd != 0 &&
		 !(hDC32->dwDCXFlags & DCX_WINDOW))?
			GetWindowLong(hWnd,GWL_UPDATE):0,
		(hRgn)?TWIN_InternalGetRegionData(hRgn):0,
		hDC32->lpDrvData);

	RELEASEDCINFO(hDC32);
	if (hRgn)
	    DeleteObject(hRgn);
    }

    RELEASEWININFO(hWnd32);
    return TRUE;
}

int WINAPI
ReleaseDC(HWND hWnd, HDC hDC)
{
    HDC32 hDC32;
    HWND32 hWnd32 = NULL;
    HRGN hRgnClip = 0;

    APISTR((LF_API,"ReleaseDC: hWnd %x hDC %x\n",hWnd,hDC));

    ASSERT_HDC(hDC32,hDC,0);

    if (hWnd  && !(hWnd32 = GETHWND32(hWnd))) {
	RELEASEDCINFO(hDC32);
	return 0;
    }

    if (hDC32->hWnd != hWnd) {
	if (!(hDC32->dwDCXFlags & (DCX_OWN_DC|DCX_CLASS_DC))) {
	    if (hWnd32)
		RELEASEWININFO(hWnd32);
	    hWnd = hDC32->hWnd;
	    hWnd32 = GETHWND32(hWnd);
	}
	else {
	    if (hWnd32)
		RELEASEWININFO(hWnd32);
	    RELEASEDCINFO(hDC32);
	    return 0;
	}
    }

    if (hWnd32 && (hWnd32->dwWinFlags & WFINPAINT) && (hDC == hWnd32->hDC)) {
	    /* somebody is trying to release a DC gotten by BeginPaint */
	    /* in this case just reset the clipping to the update region */
	if (hDC32->dwDCXFlags & DCX_CLIENTCLIP) {
	    hRgnClip = CreateRectRgn(0,0,hWnd32->wWidth,hWnd32->wHeight);
	    DRVCALL_DC(PDCH_SETUPDATECLIP,
			hWnd32->UpdateRegion,
			TWIN_InternalGetRegionData(hRgnClip),hDC32->lpDrvData);
	    DeleteObject(hRgnClip);
	}
	else
	    DRVCALL_DC(PDCH_SETUPDATECLIP,
			hWnd32->UpdateRegion,0,hDC32->lpDrvData);

	RELEASEWININFO(hWnd32);
	RELEASEDCINFO(hDC32);
	return 1;
    }

    if (hDC32->dwDCXFlags & (DCX_OWN_DC|DCX_CLASS_DC)) {
	/* for class/own DCs blow away the clipping */
	if (hDC32->dwDCXFlags & DCX_CLIENTCLIP) {
	    hRgnClip = CreateRectRgn(0,0,hWnd32->wWidth,hWnd32->wHeight);
	    DRVCALL_DC(PDCH_SETUPDATECLIP,
			hWnd32->UpdateRegion,
			TWIN_InternalGetRegionData(hRgnClip),hDC32->lpDrvData);
	    DeleteObject(hRgnClip);
	}
	else
	    DRVCALL_DC(PDCH_SETUPDATECLIP,0L,0,hDC32->lpDrvData);

	if (hWnd32)
	    RELEASEWININFO(hWnd32);
	RELEASEDCINFO(hDC32);
	return 1;
    }

    /* DeleteDC would fail if we do not do this... */
    hDC32->hWnd = (HWND)0;

    if (hWnd32)
	RELEASEWININFO(hWnd32);
    RELEASEDCINFO(hDC32);
    return DeleteDC(hDC);
}

/* unconditionally zaps the DC */
BOOL
TWIN_GdiDestroyDC(HDC hDC)
{
    HDC32 hDC32;

    ASSERT_HDC(hDC32,hDC,FALSE);

    hDC32->hWnd = (HWND)0;

    RELEASEDCINFO(hDC32);
    return DeleteDC(hDC);
}

/* this calculates visible region for a given DC */
HRGN
TWIN_CalcVisRgn(HWND hWnd, DWORD dwDCXFlags)
{
    HWND32 hWnd32;
    HRGN hRgn = 0;
    RECT rcParent, rc;

    if (0 == (hWnd32 = GETHWND32(hWnd)))
	return (HRGN)0;

    if (!IsWindowVisible(hWnd)) {
	hRgn = CreateRectRgn(0,0,0,0);
	RELEASEWININFO(hWnd32);
	return hRgn;
    }

    if (dwDCXFlags & DCX_CLIENTCLIP) {
	if (dwDCXFlags & DCX_WINDOW) {
	    GetWindowRect(hWnd,&rc);
	    hRgn = CreateRectRgn(0,0,
		rc.right - rc.left, rc.bottom - rc.top);
	}
	else {
	    hRgn = CreateRectRgn(0,0,hWnd32->wWidth,hWnd32->wHeight);
	    /* clip the children out, if needed */
	    if (hWnd32->dwStyle & WS_CLIPCHILDREN)
		ClipChildren(hWnd32,hRgn);
	}
	if (hWnd32->dwStyle & WS_CHILD) {
	    /* clip the siblings out */
	    TWIN_GenerateClip(hWnd32,hRgn,dwDCXFlags,
		(hWnd32->dwStyle & WS_CLIPSIBLINGS)?TRUE:FALSE);
	}
    }
    else {
	if (dwDCXFlags & DCX_PARENTCLIP) {
	    if (hWnd32->dwStyle & WS_CHILD) {
		GetClientRect(hWnd32->hWndParent,&rcParent);
		OffsetRect(&rcParent,-hWnd32->rWnd.left,-hWnd32->rWnd.top);
		if (!(dwDCXFlags & DCX_WINDOW))
		    OffsetRect(&rcParent,-hWnd32->rcNC.left,-hWnd32->rcNC.top);
		hRgn = CreateRectRgnIndirect(&rcParent);
		/* clip the children out, if needed */
		if (!(dwDCXFlags & DCX_WINDOW) &&
		    (hWnd32->dwStyle & WS_CLIPCHILDREN))
		    ClipChildren(hWnd32,hRgn);
		TWIN_GenerateClip(hWnd32,hRgn,dwDCXFlags,
		    (hWnd32->dwStyle & WS_CLIPSIBLINGS)?TRUE:FALSE);
	    }
	}
    }

    RELEASEWININFO(hWnd32);
    return hRgn;
}

static void
ClipChildren(HWND32 hWnd32, HRGN hRgn)
{
    HWND hWndChild;
    HWND32 hWndChild32;

    for (hWndChild = hWnd32->hWndChild;
	     hWndChild;
	     hWndChild = GetWindow(hWndChild,GW_HWNDNEXTSIB)) 
    {
	if (!(hWndChild32 = GETHWND32(hWndChild)))
	    continue;
	if ((hWndChild32->dwStyle & WS_VISIBLE) &&
		!(hWndChild32->dwExStyle & WS_EX_TRANSPARENT))
	    SubtractRectFromRegion(hRgn,&hWndChild32->rWnd);
	RELEASEWININFO(hWndChild32);
    }
}

void
TWIN_GenerateClip(HWND32 hWnd32, HRGN hRgn, DWORD dwDCXFlags,
		BOOL bClipSiblings)
{
    HWND hWndParent, hWndTree, hWndChild, hWnd;
    HWND32 hWndParent32, hWndChild32;
    RECT rcChild,rcParent;

    hWnd = GETHWND16(hWnd32);

    if (bClipSiblings) {
	hWndParent = hWnd32->hWndParent;
	hWndTree = hWnd;
    }
    else {
	/* clip by parent's client area */
	hWndParent = hWnd32->hWndParent;
	hWndParent32 = GETHWND32(hWndParent);
	SetRect(&rcParent,0,0,hWndParent32->wWidth,hWndParent32->wHeight);
	RELEASEWININFO(hWndParent32);

	MapWindowPoints(hWndParent,hWnd,(LPPOINT)&rcParent,2);
	if (dwDCXFlags & DCX_WINDOW)
	    OffsetRect(&rcParent,hWnd32->rcNC.left,hWnd32->rcNC.top);
	IntersectRectWithRegion(hRgn,&rcParent);

	hWndTree = hWnd32->hWndParent;
	hWndParent32 = GETHWND32(hWndTree);
	hWndParent = hWndParent32->hWndParent;
	RELEASEWININFO(hWndParent32);
    }

    hWndParent32 = NULL;
    for (; hWndParent;
	 hWndTree = hWndParent, hWndParent = hWndParent32->hWndParent) {
	if (hWndParent32)
	    RELEASEWININFO(hWndParent32);
	hWndParent32 = GETHWND32(hWndParent);
	SetRect(&rcParent,0,0,hWndParent32->wWidth,hWndParent32->wHeight);
	MapWindowPoints(hWndParent,hWnd,(LPPOINT)&rcParent,2);
	if (dwDCXFlags & DCX_WINDOW)
	    OffsetRect(&rcParent,hWnd32->rcNC.left,hWnd32->rcNC.top);
	IntersectRectWithRegion(hRgn,&rcParent);
	for (hWndChild = hWndParent32->hWndChild;
	     hWndChild != hWndTree;
	     hWndChild = GetWindow(hWndChild,GW_HWNDNEXTSIB)) {
	    hWndChild32 = GETHWND32(hWndChild);
	    if ((hWndChild32->dwStyle & WS_VISIBLE) &&
			!(hWndChild32->dwExStyle & WS_EX_TRANSPARENT)) {
		rcChild = hWndChild32->rWnd;
		MapWindowPoints(hWndParent,hWnd,(LPPOINT)&rcChild,2);
		if (dwDCXFlags & DCX_WINDOW)
		    OffsetRect(&rcChild,hWnd32->rcNC.left,hWnd32->rcNC.top);
		SubtractRectFromRegion(hRgn,&rcChild);
	    }
	    RELEASEWININFO(hWndChild32);
	}
    }

    if (hWndParent32)
	RELEASEWININFO(hWndParent32);
}

static void
CalcDCOrigin(HDC32 hDC32, HWND32 hWnd32, HWND hWndDC)
{
    POINT pt;
    RECT rc;
    HDC hDC = GETHDC16(hDC32);

    if (hWnd32->dwStyle & WS_CHILD) {
	/* this is in parent coordinates */
	pt.x = hWnd32->rWnd.left;
	pt.y = hWnd32->rWnd.top;
	MapWindowPoints(hWnd32->hWndParent,hWndDC,&pt,1);
	if (!(hDC32->dwDCXFlags & DCX_WINDOW)) {
	    /* offset by the NC area of the control... */
	    if (hWnd32->dwStyle & WS_BORDER) {
		pt.x += hWnd32->rcNC.left;
		pt.y += hWnd32->rcNC.top;
	    }
	}
	SetDCOrg(hDC,pt.x,pt.y);
    }
    else {
	if (hWndDC) {
	    if (hDC32->dwDCXFlags & DCX_WINDOW) {
		CalcExpectedNC(&rc,hWnd32->dwStyle,hWnd32->dwExStyle);
		SetDCOrg(hDC,-rc.left,-rc.top);
	    }
	    else
		SetDCOrg(hDC,0,hWnd32->wMenuHeight);
	}
	else {
	    if (hDC32->dwDCXFlags & DCX_WINDOW) {
		CalcExpectedNC(&rc,hWnd32->dwStyle,hWnd32->dwExStyle);
		SetDCOrg(hDC,
		    hWnd32->rWnd.left-rc.left,
		    hWnd32->rWnd.top-rc.top);
	    }
	    else
		SetDCOrg(hDC,hWnd32->rWnd.left,
			hWnd32->rWnd.top+hWnd32->wMenuHeight);
	}
    }
}

BOOL
TWIN_OffsetDCOrigins(HWND32 hWnd32, int oX, int oY)
{
    DWORD dwClassStyle;
    HWND hWnd = GETHWND16(hWnd32);
    int i;

    dwClassStyle = GetClassStyle(hWnd);
    if ((dwClassStyle & CS_OWNDC) && hWnd32->hDC)
	OffsetDCOrg(hWnd32->hDC,oX,oY);

    for (i = 0; i < TWIN_DCCacheSize; i++) {
	if (TWIN_DCCache[i].fBusy &&
	    TWIN_DCCache[i].lpDC->hWnd &&
	    (TWIN_DCCache[i].lpDC->hWnd == hWnd ||
	     IsChild(hWnd,TWIN_DCCache[i].lpDC->hWnd)))
	    OffsetDCOrg((HDC)TWIN_DCCache[i].lpDC->ObjHead.hObj,oX,oY);
    }

    return TRUE;
}

BOOL
TWIN_RecalcVisRgns(HWND32 hWnd32)
{
    DWORD dwClassStyle;
    HRGN hVisRgn;
    HWND hWnd = GETHWND16(hWnd32);
    DWORD dwDCXFlags;
    int i;

    dwClassStyle = GetClassStyle(hWnd);
    if ((dwClassStyle & CS_OWNDC) && hWnd32->hDC) {
	dwDCXFlags = TWIN_GetDCXFlags(hWnd32->hDC);
	if ((hVisRgn = TWIN_CalcVisRgn(hWnd,dwDCXFlags))) {
	    SelectVisRgn(hWnd32->hDC,hVisRgn);
	    DeleteObject(hVisRgn);
	}
    }

    for (i = 0; i < TWIN_DCCacheSize; i++) {
	if (TWIN_DCCache[i].fBusy &&
	    TWIN_DCCache[i].lpDC->hWnd &&
	    (TWIN_DCCache[i].lpDC->hWnd == hWnd ||
	     IsChild(hWnd,TWIN_DCCache[i].lpDC->hWnd))) {
	    if ((hVisRgn = TWIN_CalcVisRgn(TWIN_DCCache[i].lpDC->hWnd,
				TWIN_DCCache[i].lpDC->dwDCXFlags))) {
		SelectVisRgn((HDC)TWIN_DCCache[i].lpDC->ObjHead.hObj,hVisRgn);
		DeleteObject(hVisRgn);
	    }
	}
    }

    return TRUE;
}
