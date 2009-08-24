/*    
	WindowRedraw.c	2.47
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
#include <string.h>

#include "windows.h"
#include "windowsx.h"

#include "kerndef.h"
#include "Kernel.h"
#include "GdiDC.h"
#include "GdiRegions.h"
#include "WinDefs.h"
#include "WinData.h"
#include "Driver.h"
#include "Log.h"

/* external routines */
extern BOOL DestroyRgn(HRGN);
extern int ExcludeClipRgn(HDC,HRGN);
extern DWORD DriverWaitEvent(BOOL);
extern void TWIN_GenerateClip(HWND32,HRGN,DWORD,BOOL);
extern HWND TWIN_GetTopLevelFrame(HWND);

/* Prototypes for static internal routines */
static void InternalEndPaint(HWND, const PAINTSTRUCT *, BOOL);
static void InternalInvalidateWindows(HWND,const RECT *,HRGN,UINT);
static BOOL PropagateExpose(HWND32,int,LPVOID);
static void CopyWindowArea(HWND32,LPPOINT,LPSIZE); 

/* exported stuff */
HWND InternalUpdateWindows(void);
void InternalSetPaintFlags(HWND, UINT);
void InternalPaintWindows(HWND,UINT);
BOOL TWIN_ForwardExpose(HWND,int,LPVOID);
void TWIN_GenerateExpose(HWND32,int,LPPOINT,LPSIZE);
void TWIN_ExposeChildren(HWND32,HRGN);
BOOL TWIN_RedrawWindow(HWND,const RECT *,HRGN,UINT);

/*****************************************************************************/

HDC WINAPI
BeginPaint(HWND hWnd, LPPAINTSTRUCT lpPaint)
{
    HWND32 hWnd32;

    APISTR((LF_APICALL,"BeginPaint(HWND=%x,LPPAINTSTRUCT=%p)\n", 
		hWnd,lpPaint));

    if (!(hWnd32 = GETHWND32(hWnd))) {
        APISTR((LF_APIFAIL,"BeginPaint: returns HDC 0\n"));
	return (HDC)0;
    }

    if (!(lpPaint->hdc = GetDCEx(hWnd,(HRGN)0,
				 DCX_INTERSECTUPDATE))) {
	RELEASEWININFO(hWnd32);
        APISTR((LF_APIFAIL,"BeginPaint: returns HDC 0\n"));
	return (HDC)0;
    }
    hWnd32->hDC = lpPaint->hdc;
    
    APISTR((LF_API,"BeginPaint(hwnd=%.04x,lpPaint=%x)\n", 
		hWnd,lpPaint));

    hWnd32->dwWinFlags |= WFINPAINT;

    lpPaint->fErase = (hWnd32->dwWinFlags & WFBKERASE)?TRUE:FALSE;

    GetUpdateRect(hWnd, &lpPaint->rcPaint, FALSE);

    if (lpPaint->fErase) {
	InternalPaintWindows(hWnd,RDW_ERASENOW|RDW_NOCHILDREN);
	hWnd32->dwWinFlags &= ~WFBKERASE;
    }

    RELEASEWININFO(hWnd32);

    RedrawWindow(hWnd,(const RECT *)0,(HRGN)0,RDW_VALIDATE|RDW_NOCHILDREN);

    /* save caret... */ 

    APISTR((LF_APIRET,"BeginPaint: returns HDC %x\n",lpPaint->hdc));
    return lpPaint->hdc;
}

void WINAPI
EndPaint(HWND hWnd, const PAINTSTRUCT *lpPaint)
{
    InternalEndPaint(hWnd, lpPaint, TRUE);
}

static void
InternalEndPaint(HWND hWnd, const PAINTSTRUCT *lpPaint, BOOL bFlag)
{
    ClearWF(hWnd,WFINPAINT);
    ReleaseDC(hWnd, lpPaint->hdc);
}

HWND
InternalUpdateWindows(void)
{
    HWND32 hWnd32;
    HWND hWnd, hCurParent, hTmp;
    static HWND hDesktop = 0;
    DWORD dwClassStyle;

    if (hDesktop == (HWND)0)
	hDesktop = GetDesktopWindow();

    /* start with a first top-level window */
    hWnd = GetTopWindow(hDesktop);
    hCurParent = hDesktop;

    for (;;) {
	while(hWnd) {
	    hWnd32 = GETHWND32(hWnd);
	    if(hWnd32 == 0) {
		hWnd = GetWindow(hWnd, GW_HWNDNEXTSIB);
		continue;
	    }
	    dwClassStyle = GetClassStyle(hWnd);
	    /* if there is nothing to paint in this branch of a tree,
		go to the next sibling */
	    if (!(hWnd32->dwWinFlags &
		(WFCHILDDIRTY | WFDIRTY | WFNCDIRTY)) ||
		((dwClassStyle & CS_PARENTDC) &&
		 !IsWindowVisible(hWnd)) ||
		(hWnd32->hTask != GetCurrentTask())) {
	        hWnd = GetWindow(hWnd, GW_HWNDNEXTSIB);

		RELEASEWININFO(hWnd32);
		continue;
	    }
	    /* the current painting criteria:
		dirty and visible and mapped or ncdirty and mapped */
	    /* if the window meets the criteria, return it */
	    if (((hWnd32->dwWinFlags & WFDIRTY)
		 && (hWnd32->dwStyle & WS_VISIBLE)
		 && (hWnd32->dwWinFlags & WFMAPPED)) ||
		((hWnd32->dwWinFlags & WFNCDIRTY) &&
		 TestWF(Get32WindowFrame(hWnd32),WFMAPPED))) {

		RELEASEWININFO(hWnd32);
		return hWnd;
	    }
	    hTmp = hWnd;
	    /* if there are no dirty children, do not go deeper */
	    if (!(hWnd32->dwWinFlags & WFCHILDDIRTY) ||
		(hWnd32->dwStyle & WS_MINIMIZE))
		hWnd = GetWindow(hWnd, GW_HWNDNEXTSIB);
	    else {
		/* go one level down */
		if((hWnd = GetWindow(hWnd, GW_CHILD)))
		    hCurParent = hTmp;
	    }

	    RELEASEWININFO(hWnd32);
	}
	ClearWF(hCurParent, WFCHILDDIRTY);
	/* go to the next sibling, if there is none, go up one level */
	hWnd32 = GETHWND32(hCurParent);
	if (hWnd32 && !(hWnd = hWnd32->hWndSibling)) {
	    if (!(hWnd32->dwStyle & WS_CHILD)) 
	    {
		/* we reached the last top-level window */
		RELEASEWININFO(hWnd32);
		return (HWND)0;
	    }
	    
	    hWnd = hWnd32->hWndParent;
		
	}
	if(hWnd32 == 0)
	   return (HWND) 0;

	RELEASEWININFO(hWnd32);

	hWnd32 = GETHWND32(hWnd);
	hCurParent = (hWnd32->dwStyle & WS_CHILD)?
		hWnd32->hWndParent:hDesktop;
	RELEASEWININFO(hWnd32);
    }
}

void WINAPI
UpdateWindow(HWND hWnd)
{
    HWND32 hWnd32, lpFrameInfo;
    HWND hWndChild;

    if (!(hWnd32 = GETHWND32(hWnd))) {
	return;
    }

    /* some people (POWERPNT) are calling UpdateWindow in response to
       BeginPaint's WM_ERASEBKGND, creating an ugly loop. So, if we are
       in the BeginPaint/EndPaint sequence, do not do anything */
    if (!IsWindowVisible(hWnd) || (hWnd32->dwWinFlags & WFINPAINT))
    {
	RELEASEWININFO(hWnd32);
	return;
    }

    lpFrameInfo = GETHWND32(Get32WindowFrame(hWnd32));

    DRVCALL_WINDOWS(PWSH_FLUSHEXPOSES,0,0,0);

    for(;;) {

	if ((hWnd32->dwWinFlags & WFMAPPED) ||
	    !(hWnd32->dwWinFlags & WFVISIBILITY))
	    break;

	DriverWaitEvent(FALSE);

    }

    if (hWnd32->dwWinFlags & WFNCDIRTY) {
	     SendMessage(hWnd, WM_NCPAINT, 0, 0);
	}

    if (hWnd32->dwWinFlags & WFDIRTY) {
	     SendMessage(hWnd, WM_PAINT, 0, 0);
	}

    for (hWndChild = hWnd32->hWndChild;
	 hWndChild;
	 hWndChild = GetNextSibling(hWndChild))
	UpdateWindow(hWndChild);

    RELEASEWININFO(hWnd32);
    RELEASEWININFO(lpFrameInfo);
    
}

void WINAPI
InvalidateRgn(HWND hWnd,HRGN hRgn,BOOL bErase)
{
    if (!hWnd)
	return;

    RedrawWindow(hWnd,(const RECT *)0,hRgn,
	RDW_INVALIDATE|((bErase)?RDW_ERASE:0));
}
 
void WINAPI
InvalidateRect(HWND hWnd, const RECT *lpRect, BOOL bErase)
{
    RedrawWindow((hWnd)?hWnd:GetDesktopWindow(),
	lpRect,(HRGN)0,RDW_FRAME|RDW_INVALIDATE|((bErase)?RDW_ERASE:0));
}

void WINAPI 
ValidateRgn(HWND hWnd,HRGN hRgn)
{
    RedrawWindow(hWnd,(const RECT *)0,hRgn,
	RDW_VALIDATE|RDW_NOERASE|RDW_NOCHILDREN);
}

void WINAPI
ValidateRect(HWND hWnd, const RECT *lpRect)
{
    RedrawWindow(hWnd,lpRect,(HRGN)0,
	RDW_VALIDATE|RDW_NOERASE|RDW_NOCHILDREN);
}

BOOL WINAPI
RedrawWindow(HWND hWnd, const RECT *lprcUpdate, HRGN hrgnUpdate, UINT fuRedraw)
{
    if (fuRedraw & RDW_INVALIDATE)
	fuRedraw |= RDW_PAINT;
    if (fuRedraw & RDW_VALIDATE)
	fuRedraw |= RDW_NOPAINT;

    return TWIN_RedrawWindow(hWnd, lprcUpdate, hrgnUpdate, fuRedraw);

}

BOOL
TWIN_RedrawWindow(HWND hWnd, const RECT *lprcUpdate,
		HRGN hrgnUpdate, UINT fuRedraw)
{
    HWND32 hWnd32;
    BOOL bInvalidate,bValidate;

    if (!(hWnd32 = GETHWND32(hWnd))) {
	return FALSE;
    }
    RELEASEWININFO(hWnd32);

    APISTR((LF_API,"RedrawWindow(hWnd=%x,lprc=%x,hrgn=%x,fu=%x)\n",
	    hWnd,lprcUpdate,hrgnUpdate,fuRedraw));

    bInvalidate = (fuRedraw & RDW_INVALIDATE)?TRUE:FALSE;
    bValidate = (fuRedraw & RDW_VALIDATE)?TRUE:FALSE;

    if (bInvalidate && bValidate)
	return FALSE;

    /* first (in)validate all windows affected */
    if (bInvalidate || bValidate)
	InternalInvalidateWindows(hWnd,lprcUpdate,hrgnUpdate,fuRedraw);

    /* if we have to deliver now, do it */
    if (fuRedraw & (RDW_ERASENOW|RDW_UPDATENOW)) {
	InternalPaintWindows(hWnd,fuRedraw);
    }

    return TRUE;
}

/* This routine recursively (in)validates windows according to the uChildren
   flag, starting with hWnd32Start. Only the child windows that are visible,
   mapped and intersect the hRgn are affected. uAction determines what type
   of (in)validation is performed: client paint, background erase and/or frame.
*/
static void
InternalInvalidateWindows(HWND hWndStart, const RECT *lprcUpdate,
			HRGN hrgnUpdate, UINT uAction)
{
    HWND32 hWnd32,hWndChild32;
    HWND hWndChild;
    HRGN hRgn = (HRGN)0;
    RECT rcParent,rcChild;
    int rc;

    /* first process self, then recursively process children */

    if ((hWnd32 = GETHWND32(hWndStart)) == 0)
	return;

    /* fix up the regions */
    /* if there is a valid region, take it */
    if (hrgnUpdate && IsRegion(hrgnUpdate))
	hRgn = hrgnUpdate;
    if (uAction & RDW_INVALIDATE) {
	if (uAction & RDW_PAINT) {
	    if (!lprcUpdate || hRgn)
		rc = AddRgnToUpdate(hWndStart,hRgn);
	    else
		rc = AddRectToUpdate(hWndStart,lprcUpdate);
	    /* if update region is empty or error occured - do not invalidate */
	    if (rc == NULLREGION || rc == ERROR) {
		uAction &= ~(RDW_PAINT|RDW_ERASE);
		if (!(uAction & RDW_FRAME))
		{
		    RELEASEWININFO(hWnd32);
		    return;
		}
	    }
	}
    }
    else
	if (uAction & RDW_VALIDATE) {
	    if (uAction & RDW_NOPAINT) {
		if (!lprcUpdate || hRgn)
		    rc = SubtractRgnFromUpdate(hWndStart,hRgn);
		else
		    rc = SubtractRectFromUpdate(hWndStart,lprcUpdate);
		/* if something left in the update region - do not reset */
		if (rc != NULLREGION) {
		    uAction &= ~(RDW_NOPAINT|RDW_NOERASE);
		    if (!(uAction & RDW_NOFRAME))
		    {
			RELEASEWININFO(hWnd32);
			return;
		    }
		}
	    }
	}

    /* set the necessary flags */
    InternalSetPaintFlags(hWndStart,uAction);

    /* stop recursion if NOCHILDREN or we encountered CLIPCHILDREN */
    if ((uAction & RDW_NOCHILDREN) ||
	(!(uAction & RDW_ALLCHILDREN) && (hWnd32->dwStyle & WS_CLIPCHILDREN)))
    {
	RELEASEWININFO(hWnd32);
	return;
    }

    if (hRgn)
	GetRgnBox(hRgn,&rcParent);
    else {
	if (lprcUpdate)
	    CopyRect(&rcParent,lprcUpdate);
	else
	    SetRect(&rcParent,0,0,hWnd32->wWidth,hWnd32->wHeight);
    }

    /* we take only children that intersect the box of hRgn */
    for (hWndChild = hWnd32->hWndChild;
	 hWndChild;
	 hWndChild = GetWindow(hWndChild,GW_HWNDNEXTSIB)) {
	if ((hWndChild32 = GETHWND32(hWndChild))) {
	    /* rcParent is in parent coordinates */
	    /* child's rWnd is in parent coords as well */
	    if (IntersectRect(&rcChild,&rcParent,&hWndChild32->rWnd)) {
		/* map intersection to child's coordinates */
		MapWindowPoints(hWndChild32->hWndParent,hWndChild,
				(POINT *)&rcChild,2);
		InternalInvalidateWindows(hWndChild,&rcChild,(HRGN)0,uAction);
	    }
	}
	RELEASEWININFO(hWndChild32);
    }

    RELEASEWININFO(hWnd32);
}

void
InternalSetPaintFlags(HWND hWnd, UINT uAction)
{
    HWND32 hWnd32,hWndParent32;
    HWND32 hNext32;
    HTASK hTask;

    if (!hWnd || !(hWnd32 = GETHWND32(hWnd)))
	return;

    if (uAction & RDW_VALIDATE) {
	if (uAction & RDW_NOPAINT)
	    hWnd32->dwWinFlags &= ~WFDIRTY;
	if (uAction & RDW_NOERASE)
	    hWnd32->dwWinFlags &= ~WFBKERASE;
	if (uAction & RDW_NOFRAME)
	    hWnd32->dwWinFlags &= ~WFNCDIRTY;
    }
    else if (uAction & RDW_INVALIDATE) {
	if (uAction & RDW_PAINT)
	    hWnd32->dwWinFlags |= WFDIRTY;
	if (uAction & RDW_ERASE)
	    hWnd32->dwWinFlags |= WFBKERASE;
	if (uAction & RDW_FRAME)
	    hWnd32->dwWinFlags |= WFNCDIRTY;

	hTask = hWnd32->hTask;

	QueueSetFlags(hTask, QFPAINT);

	if (hWnd32->dwStyle & WS_CHILD) {
	    for (hWndParent32 = GETHWND32(hWnd32->hWndParent);
		 hWndParent32;
		 hWndParent32 = hNext32) 
	    {
		hWndParent32->dwWinFlags |= WFCHILDDIRTY;
		hNext32 = ((hWndParent32->hWndParent) ?
			   GETHWND32(hWndParent32->hWndParent) : 0);
		RELEASEWININFO(hWndParent32);
	    }
	}
    }

    RELEASEWININFO(hWnd32);
}

void
InternalPaintWindows(HWND hWnd, UINT uAction)
{
    HWND32 hWnd32,hWndChild32;
    HWND hWndChild;
    UINT uMsg;
    HDC hDC;

    if (!(uAction & (RDW_ERASENOW|RDW_UPDATENOW)))
	return;

    /* first process self, then recursively process children */

    if ((hWnd32 = GETHWND32(hWnd)) == 0)
	return;

    if (hWnd32->dwWinFlags & (WFBKERASE|WFDIRTY|WFNCDIRTY)) {
	if (hWnd32->dwWinFlags & WFNCDIRTY) {
	    if (NonEmptyNCRect(hWnd))
		SendMessage(hWnd,WM_NCPAINT,0,0L);
	}
	if (hWnd32->dwWinFlags & WFBKERASE) {
	    uMsg = ((hWnd32->dwStyle & WS_MINIMIZE) &&
		    GetClassIcon(hWnd))?
				WM_ICONERASEBKGND:WM_ERASEBKGND;
	    /* need to force cache DC, because hWnd can have */
	    /* OWNDC with funny things in it... */
	    hDC = GetDCEx(hWnd,(HRGN)0,DCX_CACHE | DCX_INTERSECTUPDATE);

	    /*  exclude valid children areas from erasing for
		non-WS_CLIPCHILDREN windows. Maybe we have to do
		Save/RestoreDC here (CLASS/OWNDC???)  */
	    if (!(hWnd32->dwStyle & WS_CLIPCHILDREN)) {
		for (hWndChild = hWnd32->hWndChild;
		     hWndChild;
		     hWndChild = GetWindow(hWndChild,GW_HWNDNEXTSIB)) {
		    if (!(hWndChild32 = GETHWND32(hWndChild)))
			break;
		    if (!(hWndChild32->dwWinFlags & WFMAPPED))
		    {
			RELEASEWININFO(hWndChild32);
			continue;
		    }
		    
		    if (!(hWndChild32->dwWinFlags & (WFNCDIRTY|WFDIRTY)))
			ExcludeClipRect(hDC,hWndChild32->rWnd.left,
				hWndChild32->rWnd.top,
				hWndChild32->rWnd.right,
				hWndChild32->rWnd.bottom);
		    else {
			HRGN hRgnClient,hRgnUpdate;
			int nRet;

			/* this region is the client area */
			hRgnClient = CreateRectRgn(0,0,
				hWndChild32->wWidth,hWndChild32->wHeight);
			/* this is the dirty region */
			hRgnUpdate = CreateRectRgn(0,0,0,0);
			GetUpdateRgn(hWndChild,hRgnUpdate,FALSE);
			/* this is a "clean" region */
			nRet = CombineRgn(hRgnClient,hRgnClient,
				hRgnUpdate,RGN_DIFF);
			if (nRet != ERROR && nRet != NULLREGION) {
			    /* convert it to parent coordinates */
			    OffsetRgn(hRgnClient,
				hWndChild32->rWnd.left+hWndChild32->rcNC.left,
				hWndChild32->rWnd.top+hWndChild32->rcNC.top);
			    /* exclude it */
			    ExcludeClipRgn(hDC,hRgnClient);
			}
			/* what do we do with non-client areas??? */
			DeleteObject(hRgnClient);
			DeleteObject(hRgnUpdate);
		    }
		    RELEASEWININFO(hWndChild32);
		}
	    }

	    SendMessage(hWnd,uMsg,(WPARAM)hDC,0L);
	    ReleaseDC(hWnd,hDC);
	}
	if ((uAction & RDW_UPDATENOW) && (hWnd32->dwWinFlags & WFDIRTY)) {
	    if (IsIconic(hWnd) && GetClassIcon(hWnd))
		SendMessage(hWnd,WM_PAINTICON,1,0L);
	    else
		SendMessage(hWnd,WM_PAINT,0,0L);
	}
    }

    /* stop recursion if NOCHILDREN or we encountered CLIPCHILDREN */
    if ((uAction & RDW_NOCHILDREN) ||
	(!(uAction & RDW_ALLCHILDREN) && (hWnd32->dwStyle & WS_CLIPCHILDREN)))
    {
	RELEASEWININFO(hWnd32);
	return;
    }

    for (hWndChild = hWnd32->hWndChild;
	 hWndChild;
	 hWndChild = GetWindow(hWndChild,GW_HWNDNEXTSIB))
	InternalPaintWindows(hWndChild,uAction);

    RELEASEWININFO(hWnd32);
}

BOOL
TWIN_ForwardExpose(HWND hWndFrame, int nFunc, LPVOID lpStruct)
{
    HWND hWndPrev;
    HWND32 hWndClient32,hWndFrame32;

    ASSERT_HWND(hWndFrame32, hWndFrame, FALSE);

    ASSERT_HWND(hWndClient32, Get32FrameClient(hWndFrame32), FALSE);

    hWndClient32->dwWinFlags |= WFMAPPED;
    if (hWndClient32->dwWinFlags & WFMAPPINGPENDING) {
	hWndClient32->dwWinFlags &= ~WFMAPPINGPENDING;
	QueueSetFlags(GetWindowTask(hWndFrame), QFPAINT);
    }
    hWndFrame32->dwWinFlags |= WFMAPPED;
    hWndFrame32->dwWinFlags &= ~WFMAPPINGPENDING;

    PropagateExpose(hWndFrame32,nFunc,lpStruct);

    RedrawWindow(Get32FrameClient(hWndFrame32),NULL,(HRGN)0,
	RDW_ERASENOW|RDW_NOCHILDREN);
    InternalSetPaintFlags(Get32FrameClient(hWndFrame32),
			RDW_VALIDATE|RDW_NOERASE);

    /*
     * It is possible that this event occured because the ZORDER changed
     * without our consent.  This can happen if we are using the X11 PAL.
     * Since there is no way of knowing for sure, we will call the PAL and
     * tell it to reposition this window to where we believe it should already
     * be.  If it is already there then there SHOULD be no harm in doing this.
     * This assumption should be verified on all platforms.
     */
    /* Who is in the ZORDER just before us? */
    hWndPrev = GetWindow(GETHWND16(hWndClient32), GW_HWNDPREVTREE);
    if (hWndPrev)
    {
	HWND hWndPrevFrame = GetWindowFrame(hWndPrev);
	HWND32 hWndPrevFrame32, hWndFrame32;
	DWORD dataPrev, dataWnd;

	hWndPrevFrame32 = CHECKHWND32(hWndPrevFrame);
	hWndFrame32 = CHECKHWND32(hWndFrame);
	RELEASEWININFO(hWndPrevFrame32);
	RELEASEWININFO(hWndFrame32);

	if (hWndPrevFrame32 && hWndFrame32)
	{
	    dataPrev = WIN_GETDRVDATA(hWndPrevFrame);
	    dataWnd = WIN_GETDRVDATA(hWndFrame);
	
	    if (dataPrev && dataWnd)
	    {
#ifdef NEW_ZORDER
		DRVCALL_WINDOWS(PWSH_FIXUPSTACKINGORDER, 
				hWndPrev, dataPrev, dataWnd);
#endif
	    }
	}
    }	

    /*
     * Release all locked memory and return.
     */
    RELEASEWININFO(hWndClient32);
    RELEASEWININFO(hWndFrame32);
    return TRUE;
}

static BOOL
PropagateExpose(HWND32 hWndFrame32, int nFunc, LPVOID lpStruct)
{
    HWND32 hWndClient32, hWndChildFrame32;
    HWND hWndChild;
    LPRECT lprcExpose;
    DWORD RgnExpose32,RgnClient32,RgnFrame32,RgnTemp32;
    RECT rcClient,rcUpdate,rcFrame;
    HRGN hRgnClient,hRgnFrame;
    POINT pt;

    ASSERT_HWND(hWndClient32, Get32FrameClient(hWndFrame32), FALSE);

    switch (nFunc) {
	case SIMPLEREGION:
	    lprcExpose = (LPRECT)lpStruct;

	    /* invalidate client's areas */
	    SetRect(&rcClient,hWndClient32->rcNC.left,hWndClient32->rcNC.top,
		hWndClient32->wWidth+hWndClient32->rcNC.left,
		hWndClient32->wHeight+hWndClient32->rcNC.top);
	    if (IntersectRect(&rcUpdate,&rcClient,lprcExpose)) {
		OffsetRect(&rcUpdate,-hWndClient32->rcNC.left,
			-hWndClient32->rcNC.top);
		DRVCALL_REGIONS(PRH_UNIONRECTWITHREGION,
			hWndClient32->UpdateRegion,0,&rcUpdate);
		InternalSetPaintFlags(GETHWND16(hWndClient32),
			RDW_INVALIDATE|RDW_PAINT|RDW_ERASE);
	    }

	    /* invalidate frame's areas */
	    hRgnFrame = CreateRectRgn(0,0,
		hWndClient32->wWidth+
			hWndClient32->rcNC.left+hWndClient32->rcNC.right,
		hWndClient32->wHeight+
			hWndClient32->rcNC.top+hWndClient32->rcNC.bottom);
	    SubtractRectFromRegion(hRgnFrame,&rcClient);
	    if (IntersectRectWithRegion(hRgnFrame,lprcExpose) > NULLREGION) {
		if (IsWindow(hWndClient32->hWndHZScroll)) {
		    if (IsWindowVisible(hWndClient32->hWndHZScroll)) {
			SetWF(hWndClient32->hWndHZScroll,WFMAPPED);
			ClearWF(hWndClient32->hWndHZScroll,WFMAPPINGPENDING);
		    }
		}
		if (IsWindow(hWndClient32->hWndVTScroll)) {
		    if (IsWindowVisible(hWndClient32->hWndVTScroll)) {
			SetWF(hWndClient32->hWndVTScroll,WFMAPPED);
			ClearWF(hWndClient32->hWndVTScroll,WFMAPPINGPENDING);
		    }
		}
		InternalSetPaintFlags(GETHWND16(hWndClient32),
			RDW_INVALIDATE|RDW_FRAME);
	    }
	    DeleteObject(hRgnFrame);

	    /* Recursively invalidate children */
	    OffsetRect(lprcExpose,-hWndClient32->rcNC.left,
				-hWndClient32->rcNC.top);
	    for (hWndChild = hWndClient32->hWndChild; hWndChild;
			hWndChild = GetWindow(hWndChild,GW_HWNDNEXTSIB)) {
		hWndChildFrame32 = GETHWND32(GetWindowFrame(hWndChild));
		SetRect(&rcFrame,
			lprcExpose->left-hWndChildFrame32->rWnd.left,
			lprcExpose->top-hWndChildFrame32->rWnd.top,
			lprcExpose->right-hWndChildFrame32->rWnd.left,
			lprcExpose->bottom-hWndChildFrame32->rWnd.top);
		PropagateExpose(hWndChildFrame32,nFunc,(LPVOID)&rcFrame);
		RELEASEWININFO(hWndChildFrame32);
	    }
	    break;

	case COMPLEXREGION:
	    RgnExpose32 = (DWORD)lpStruct;

	    /* invalidate client's areas */
	    hRgnClient = CreateRectRgn(hWndClient32->rcNC.left,
		hWndClient32->rcNC.top,
		hWndClient32->wWidth+hWndClient32->rcNC.left,
		hWndClient32->wHeight+hWndClient32->rcNC.top);
	    RgnClient32 = TWIN_InternalGetRegionData(hRgnClient);
	    if (DRVCALL_REGIONS(PRH_INTERSECTREGION,
		    RgnClient32,RgnExpose32,RgnClient32) > NULLREGION) {
		OffsetRgn(hRgnClient,-hWndClient32->rcNC.left,
				-hWndClient32->rcNC.top);
		DRVCALL_REGIONS(PRH_UNIONREGION,
			hWndClient32->UpdateRegion,RgnClient32,
			hWndClient32->UpdateRegion);
		InternalSetPaintFlags(GETHWND16(hWndClient32),
			RDW_INVALIDATE|RDW_PAINT|RDW_ERASE);
	    }

	    /* invalidate frame's areas */
	    SetRect(&rcClient,hWndClient32->rcNC.left,
		hWndClient32->rcNC.top,
		hWndClient32->wWidth+hWndClient32->rcNC.left,
		hWndClient32->wHeight+hWndClient32->rcNC.top);
	    hRgnFrame = CreateRectRgn(0,0,
		hWndClient32->wWidth+
			hWndClient32->rcNC.left+hWndClient32->rcNC.right,
		hWndClient32->wHeight+
			hWndClient32->rcNC.top+hWndClient32->rcNC.bottom);
	    SubtractRectFromRegion(hRgnFrame,&rcClient);
	    RgnFrame32 = TWIN_InternalGetRegionData(hRgnFrame);
	    if (DRVCALL_REGIONS(PRH_INTERSECTREGION,
		    RgnFrame32,RgnExpose32,RgnFrame32) > NULLREGION) {
		if (IsWindow(hWndClient32->hWndHZScroll)) {
		    if (IsWindowVisible(hWndClient32->hWndHZScroll)) {
			SetWF(hWndClient32->hWndHZScroll,WFMAPPED);
			ClearWF(hWndClient32->hWndHZScroll,WFMAPPINGPENDING);
		    }
		}
		if (IsWindow(hWndClient32->hWndVTScroll)) {
		    if (IsWindowVisible(hWndClient32->hWndVTScroll)) {
			SetWF(hWndClient32->hWndVTScroll,WFMAPPED);
			ClearWF(hWndClient32->hWndVTScroll,WFMAPPINGPENDING);
		    }
		}
		InternalSetPaintFlags(GETHWND16(hWndClient32),
			RDW_INVALIDATE|RDW_FRAME);
	    }

	    DeleteObject(hRgnClient);
	    DeleteObject(hRgnFrame);

	    pt.x = -hWndClient32->rcNC.left;
	    pt.y = -hWndClient32->rcNC.top;
	    DRVCALL_REGIONS(PRH_OFFSETREGION,RgnExpose32,
			0L,(LPVOID)&pt);
	    for (hWndChild = hWndClient32->hWndChild; hWndChild;
			hWndChild = GetWindow(hWndChild,GW_HWNDNEXTSIB)) {
		hWndChildFrame32 = GETHWND32(GetWindowFrame(hWndChild));
		RgnTemp32 = DRVCALL_REGIONS(PRH_CREATEREGION,0,0,0);
		DRVCALL_REGIONS(PRH_COPYREGION,RgnExpose32,RgnTemp32,0);
		pt.x = -hWndChildFrame32->rWnd.left;
		pt.y = -hWndChildFrame32->rWnd.top;
		DRVCALL_REGIONS(PRH_OFFSETREGION,RgnTemp32,0L,&pt);
		PropagateExpose(hWndChildFrame32,nFunc,(LPVOID)RgnTemp32);
		DRVCALL_REGIONS(PRH_DESTROYREGION,RgnTemp32,0,0);
		RELEASEWININFO(hWndChildFrame32);
	    }
	    break;
	default:
	    break;
    }

    RELEASEWININFO(hWndClient32);
    return TRUE;
}

#if 0
static void
GenerateExpose(HWND32 hWnd32, HRGN hRgn)
{
    HWND hWndTopFrame;
    POINT pt;

    TWIN_GenerateClip(hWnd32,hRgn,DCX_WINDOW,TRUE);

    hWndTopFrame = TWIN_GetTopLevelFrame(Get32WindowFrame(hWnd32));
    pt.x = pt.y = 0;
    MapWindowPoints(Get32WindowFrame(hWnd32),hWndTopFrame,&pt,1);
    OffsetRgn(hRgn,pt.x,pt.y);

    PropagateExpose(GETHWND32(hWndTopFrame), COMPLEXREGION,
	(LPVOID)TWIN_InternalGetRegionData(hRgn));
}
#endif

void
TWIN_GenerateExpose(HWND32 hWnd32, int nFunc, LPPOINT lpptOld,
			LPSIZE lpOldSize)
{
    HRGN hRgn,hRgnOld,hRgnTmp;
    POINT pt;
    RECT rc;
    SIZE sz;
    HWND hWndTopFrame;

    sz.cx = hWnd32->rWnd.right-hWnd32->rWnd.left;
    sz.cy = hWnd32->rWnd.bottom-hWnd32->rWnd.top;
    hRgn = CreateRectRgn(0,0,sz.cx,sz.cy);
    hRgnOld = 0;

    switch (nFunc) {
	case EXPOSE_MOVE:
	    hRgnOld = CreateRectRgn(0,0,
			lpOldSize->cx,lpOldSize->cy);
	    OffsetRgn(hRgnOld,lpptOld->x-hWnd32->rWnd.left,
			   lpptOld->y-hWnd32->rWnd.top);
	    TWIN_GenerateClip(hWnd32,hRgnOld,DCX_WINDOW,
		(hWnd32->dwStyle & WS_CLIPSIBLINGS)?TRUE:FALSE);
	    CombineRgn(hRgn,hRgnOld,hRgn,RGN_DIFF);
	    hRgnTmp = CreateRectRgn(0,0,
			lpOldSize->cx,lpOldSize->cy);
	    OffsetRgn(hRgnOld,hWnd32->rWnd.left-lpptOld->x,
			hWnd32->rWnd.top-lpptOld->y);
	    CombineRgn(hRgnOld,hRgnTmp,hRgnOld,RGN_DIFF);
	    CombineRgn(hRgn,hRgn,hRgnOld,RGN_OR);
	    DeleteObject(hRgnTmp);
	    CopyWindowArea(hWnd32,lpptOld,&sz);
	    break;
	case EXPOSE_SIZE:
	    hRgnOld = CreateRectRgn(0,0,
			lpOldSize->cx,lpOldSize->cy);
	    CombineRgn(hRgn,hRgnOld,hRgn,RGN_DIFF);
	    break;
	case EXPOSE_SIZEMOVE:
	    hRgnOld = CreateRectRgn(0,0,
			lpOldSize->cx,lpOldSize->cy);
	    OffsetRgn(hRgnOld,lpptOld->x-hWnd32->rWnd.left,
			   lpptOld->y-hWnd32->rWnd.top);
	    TWIN_GenerateClip(hWnd32,hRgnOld,DCX_WINDOW,
		(hWnd32->dwStyle & WS_CLIPSIBLINGS)?TRUE:FALSE);
	    CombineRgn(hRgn,hRgnOld,hRgn,RGN_DIFF);
	    hRgnTmp = CreateRectRgn(0,0,
			lpOldSize->cx,lpOldSize->cy);
	    OffsetRgn(hRgnOld,hWnd32->rWnd.left-lpptOld->x,
			hWnd32->rWnd.top-lpptOld->y);
	    CombineRgn(hRgnOld,hRgnTmp,hRgnOld,RGN_DIFF);
	    CombineRgn(hRgn,hRgn,hRgnOld,RGN_OR);
	    DeleteObject(hRgnTmp);
	    sz.cx = min(sz.cx,lpOldSize->cx);
	    sz.cy = min(sz.cy,lpOldSize->cy);
	    CopyWindowArea(hWnd32,lpptOld,&sz);
	    break;
	case EXPOSE_HIDE:
	    break;
    }

    TWIN_GenerateClip(hWnd32,hRgn,DCX_WINDOW,
		(hWnd32->dwStyle & WS_CLIPSIBLINGS)?TRUE:FALSE);

    hWndTopFrame = TWIN_GetTopLevelFrame(Get32WindowFrame(hWnd32));
    pt.x = pt.y = 0;
    MapWindowPoints(Get32WindowFrame(hWnd32),hWndTopFrame,&pt,1);
    OffsetRgn(hRgn,pt.x,pt.y);

    PropagateExpose(GETHWND32(hWndTopFrame), COMPLEXREGION,
	(LPVOID)TWIN_InternalGetRegionData(hRgn));

    DeleteObject(hRgn);

    if (nFunc != EXPOSE_HIDE) {
	hRgn = CreateRectRgn(0,0,hWnd32->rWnd.right-hWnd32->rWnd.left,
			hWnd32->rWnd.bottom-hWnd32->rWnd.top);
	if (nFunc == EXPOSE_SIZE || nFunc == EXPOSE_SIZEMOVE) {
	    SetRect(&rc,hWnd32->rcNC.left,hWnd32->rcNC.top,
		    lpOldSize->cx-hWnd32->rcNC.right,
		    lpOldSize->cy-hWnd32->rcNC.bottom);
	    hWnd32->dwWinFlags |= WFNCDIRTY;
	}
	else {	/* EXPOSE_MOVE */
	    GetClientRect(hWnd32->hWndParent,&rc);
	    OffsetRect(&rc,-lpptOld->x,-lpptOld->y);
	}
	if (SubtractRectFromRegion(hRgn,&rc) > NULLREGION) {
	    TWIN_ExposeChildren(hWnd32,hRgn);
	    hWnd32->dwWinFlags |= WFNCDIRTY;
	}
	DeleteObject(hRgn);
	DeleteObject(hRgnOld);

#ifdef	LATER
/****************************/
	RedrawWindow(GetFrameClient(hWndTopFrame),NULL,(HRGN)0,RDW_ERASENOW|RDW_ALLCHILDREN);
	InternalSetPaintFlags(GetFrameClient(hWndTopFrame),RDW_VALIDATE|RDW_NOERASE);
/****************************/
#endif
    }
}

static void
CopyWindowArea(HWND32 hWnd32, LPPOINT lppt, LPSIZE lpsz)
{
    HDC hDCSrc,hDCDest;
    RECT rc,rcTop,rcCopy;
    HWND hWndTop;
    int XDest = 0,YDest = 0,cx,cy;

    hDCSrc = GetDCEx(hWnd32->hWndParent,(HRGN)0,DCX_CACHE|DCX_DEFAULTCLIP);
    hDCDest = GetWindowDC(GETHWND16(hWnd32));
    SetRect(&rc,lppt->x,lppt->y,lppt->x+lpsz->cx,lppt->y+lpsz->cy);
    if (RectVisible(hDCSrc,&rc)) {
	hWndTop = GetTopLevelAncestor(hWnd32->hWndParent);
	GetPhysicalRect(hWndTop,&rcTop);
	MapWindowPoints(HWND_DESKTOP,hWnd32->hWndParent,(LPPOINT)&rcTop,2);
	IntersectRect(&rcCopy,&rc,&rcTop);
	cx = rcCopy.right-rcCopy.left;
	cy = rcCopy.bottom-rcCopy.top;
	if (rcCopy.left > rc.left) 
	    XDest = rcCopy.left - rc.left;
	if (rcCopy.top > rc.top) 
	    YDest = rcCopy.top - rc.top;
	BitBlt(hDCDest,XDest,YDest,cx,cy,hDCSrc,rcCopy.left,rcCopy.top,SRCCOPY);
    }
    ReleaseDC(hWnd32->hWndParent,hDCSrc);
    ReleaseDC(GETHWND16(hWnd32),hDCDest);
}

void
TWIN_ExposeChildren(HWND32 hWnd32, HRGN hRgn)
{
    HWND32 hWndFrame32;

    hWndFrame32 = GETHWND32(Get32WindowFrame(hWnd32));
    PropagateExpose(hWndFrame32,COMPLEXREGION,
		(LPVOID)TWIN_InternalGetRegionData(hRgn));
    RELEASEWININFO(hWndFrame32);
}

void    WINAPI 
ScrollWindow(HWND hWnd, int cx, int cy,
		const RECT FAR *lprcScroll, const RECT FAR *lprcClip)
{
    UINT fuFlags = SW_INVALIDATE|SW_ERASE;

    if (lprcScroll == NULL)
	fuFlags |= SW_SCROLLCHILDREN;

    ScrollWindowEx(hWnd,cx,cy,lprcScroll,lprcClip,(HRGN)0,NULL,fuFlags);
}

int     WINAPI 
ScrollWindowEx(HWND hWnd, int dx, int dy,
                const RECT FAR* lprcScroll, const RECT FAR* lprcClip,
                HRGN hrgnUpdate, RECT FAR* lprcUpdate, UINT flags)
{
    HWND32 hWnd32;
    DWORD dwStyle;
    HWND hChild;
    RECT rcChild,rcClient;
    POINT pt;
    int	xpos,ypos;
    HDC	hDC;
    int	rc;
    BOOL fNeedCleanup = FALSE;

    ASSERT_HWND(hWnd32, hWnd, ERROR);

    /* the DC should not be clipped by child windows... */
    dwStyle = hWnd32->dwStyle;
    if (dwStyle & WS_CLIPCHILDREN)
	hWnd32->dwStyle &= ~WS_CLIPCHILDREN;

    /*
     * Beware of the Win 3.1 documentation with regards to ScrollWindow
     * and ScrollWindowEx.  The documentation indicates that all values
     * are in device points.  However, in reality Win 3.1 behaves as documented
     * in the Win32 documentation.  That is that values are in logical
     * points if the window class has the style CS_OWNDC or CS_CLASSDC.
     * The following GetDCEx call correctly emulates the behavior of both
     * Win 3.1 and Win32.
     */
    hDC = GetDCEx(hWnd,(HRGN)0,DCX_PARENTCLIP);
    hWnd32->dwStyle = dwStyle;

    if (hrgnUpdate == 0) {
	hrgnUpdate = CreateRectRgn(0,0,0,0);
	fNeedCleanup = TRUE;
    }

    /* pass the scroll information down... */
    rc = ScrollDC(hDC,dx,dy,lprcScroll,lprcClip,hrgnUpdate,lprcUpdate);

    ReleaseDC(hWnd,hDC);	

    /* are we invalidating/erasing */

    if(flags & SW_SCROLLCHILDREN) {
	if (lprcScroll) 
	    CopyRect(&rcClient,lprcScroll);
	else 
	    SetRect(&rcClient,0,0,hWnd32->wWidth,hWnd32->wHeight);

	/* for each intersecting child */
	for(hChild = hWnd32->hWndChild;
		    hChild;
		    hChild = GetWindow(hChild,GW_HWNDNEXT)) 
	    if (GetWindow(hChild,GW_OWNER) == 0) {	/* NOT icon title */

		GetWindowRect(hChild,&rcChild);
		pt.x = rcChild.left;
		pt.y = rcChild.top;
		ScreenToClient(hWnd,&pt);
		SetRect(&rcChild,pt.x,pt.y,
				pt.x + (rcChild.right-rcChild.left),
				pt.y + (rcChild.bottom-rcChild.top));
		xpos = rcChild.left + dx;
		ypos = rcChild.top  + dy;
		SetWindowPos(hChild,0,xpos,ypos,0,0,
			SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOREDRAW);
	    }
#ifdef LATER
    /* Reposition the caret if it is in the child window and the cursor
	rectangle intersects the scroll rect */
#endif
    }

    if(flags & (SW_INVALIDATE|SW_ERASE)) 
	if(flags & SW_SCROLLCHILDREN) {
	    OffsetRgn(hrgnUpdate,hWnd32->rcNC.left,hWnd32->rcNC.top);
	    TWIN_ExposeChildren(hWnd32, hrgnUpdate);
	}
	else
	    InvalidateRgn(hWnd,hrgnUpdate, flags&SW_ERASE?TRUE:FALSE);

    if (fNeedCleanup)
	DeleteObject(hrgnUpdate);

    RELEASEWININFO(hWnd32);
    return rc?SIMPLEREGION:ERROR;
}

