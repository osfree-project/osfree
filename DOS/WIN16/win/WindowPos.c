/*    
	WindowPos.c	2.112
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
#include "windowsx.h"

#include "WinData.h"
#include "Driver.h"
#include "Log.h"
#include "WinDefs.h"
#include "compat.h"

#include <stdio.h>
#include <string.h>


extern ATOM atmGlobalLookup[];

extern DWORD DriverWaitEvent(BOOL);
extern void TWIN_GenerateExpose(HWND32,int,LPPOINT,LPSIZE);
extern BOOL TWIN_OffsetDCOrigins(HWND32, int, int);
extern BOOL TWIN_RecalcVisRgns(HWND32);
extern BOOL TWIN_RedrawWindow(HWND, const RECT *, HRGN, UINT);

extern void TWIN_ExposeChildren(HWND32,HRGN);
extern void TWIN_GenerateClip(HWND32, HRGN, DWORD, BOOL);
extern int UnionRectWithRegion(HRGN, const LPRECT);
extern void DrawSizeBox(HWND32);

extern int GetCompatibilityFlags(int);

BOOL TWIN_InsertAsFirstSibling(HWND32);

/* Prototypes for static internal routines */
static void ChangeZOrder(HWND32, HWND, BOOL);
static BOOL InitWinManStructs(HWND);
static BOOL AnyPopupEnumProc(HWND, LPARAM);
static BOOL ShowOwnedPopupsEnumProc(HWND, LPARAM);
static HWND InternalSetActiveWindow(HWND32);
static void ActivateApp(HWND32, HWND32);
static BOOL EnumAllWindows(WORD, HANDLE, WNDENUMPROC, LPARAM);
static BOOL GetAvailableIconPosition(HWND32, LPRECT);
static HWND TWIN_ChildWindowFromPointEx(HWND, POINT, UINT);
static BOOL WindowFromPointEnumProc(HWND, LPARAM);
static void ChildMinMaximize(HWND,UINT,BOOL);
static void TopMinMaximize(HWND,UINT,BOOL);
static BOOL InternalSetWindowPos(LPWINDOWPOS, WORD);
static int ChangeZOrderFindTop(HWND32 hWnd32, void *search_data);
static int ChangeZOrderFindBottom(HWND32 hWnd32, void *search_data);
static int ChangeZOrderFindPreviousSibling(HWND32 hWnd32, void *search_data);
static int ChangeZOrderInvalidate(HWND32 hWnd32, void *additional_data);
static HWND TWIN_EnumerateSiblingWindows(HWND, int (*)(HWND32, void *), void *);

static HWND hWndRoot = 0;
static HWND32 lpRootInfo = (HWND32)NULL;

static HWND hCurrentActiveWindow = 0;
static HWND hCurrentCapture = 0;
static HWND hCurrentFocusWindow = 0;

static BOOL bHaveFocus = FALSE;

static HTASK hActiveApp = (HTASK)0;

/* action flags for DrawIconTitle */

#define DIT_RECALC		0x0001
#define DIT_NORECALC		0x0000
#define DIT_REDRAW		0x0002
#define DIT_NOREDRAW		0x0000

struct TWIN_changezorderinvalidate_s
  {
	HWND32	hwnd32;	
	HRGN  	hrgn;
	POINT   pt;
  };


/* Initialization routine */

BOOL
InitWindows()
{
    HWND hWnd;

    if (!(hWnd = CreateRootWindow()))
	return FALSE;

    return InitWinManStructs(hWnd);
}

static BOOL
InitWinManStructs(HWND hRoot)
{
    HWND32 lpRootWindow;

    if (!(lpRootWindow = GETHWND32(hRoot))) {
	return(FALSE);
    }

    lpRootWindow->hWndParent = 0;
    lpRootWindow->hWndSibling = 0;
    lpRootWindow->hWndChild = 0;

    hWndRoot = hRoot;
    lpRootInfo = lpRootWindow;

    return(TRUE);
}

/* API functions */

/* Window-creation group */

void WINAPI
AdjustWindowRect(LPRECT lpRect, DWORD dwStyle, BOOL bMenu)
{
    AdjustWindowRectEx(lpRect, dwStyle | WS_CHILD, bMenu, 0L);
}

void WINAPI
AdjustWindowRectEx(LPRECT lpRect, DWORD dwStyle, BOOL bMenu,
			DWORD dwExStyle)
{
    RECT rcNC;

    APISTR((LF_APICALL,
	"AdjustWindowRect(LPRECT=%x,DWORD=%x,BOOL=%x,DWORD=%x)\n",
		lpRect,dwStyle,bMenu,dwExStyle));

    CalcNCDimensions(&rcNC, dwStyle, dwExStyle,
		(bMenu)?(WORD)GetSystemMetrics(SM_CYMENU):0);
#ifdef	LATER
	now SM_CYMENU is not correct
#endif
    lpRect->left -= rcNC.left;
    lpRect->top -= rcNC.top;
    lpRect->right += rcNC.right;
    lpRect->bottom += rcNC.bottom;
    APISTR((LF_APIRET,"AdjustWindowRect; returns void.\n"));
}

BOOL WINAPI
GetWindowRect(HWND hWnd, LPRECT lpRect)
{
    HWND32 hWnd32;
    RECT rcExpectedNC;

    APISTR((LF_APICALL,
	"GetWindowRect(HWND=%x,RECT *=%x)\n",
	hWnd, lpRect));

    if (!(hWnd32 = GETHWND32(hWnd))) {
	SetRectEmpty(lpRect);
    	APISTR((LF_APIFAIL, "GetWindowRect: returns BOOL FALSE\n"));
	return FALSE;
    }
    GetPhysicalRect(hWnd,lpRect);

    if (!(hWnd32->dwStyle & WS_CHILD)) {
	CalcExpectedNC(&rcExpectedNC,hWnd32->dwStyle,hWnd32->dwExStyle);
	lpRect->top -= rcExpectedNC.top;
	lpRect->left -= rcExpectedNC.left;
	lpRect->right += rcExpectedNC.right;
	lpRect->bottom += rcExpectedNC.bottom;
    }

    RELEASEWININFO(hWnd32);
    
    APISTR((LF_APIRET, "GetWindowRect: returns BOOL TRUE\n"));
    return (TRUE);

}

void
GetPhysicalRect(HWND hWnd, LPRECT lpRect)
{
    HWND32 hWnd32;
    POINT pt;

    if (!(hWnd32 = GETHWND32(hWnd))) {
	SetRectEmpty(lpRect);
	return;
    }

    if (hWnd32->dwStyle & WS_CHILD) {
	pt.x = hWnd32->rWnd.left;
	pt.y = hWnd32->rWnd.top;
	ClientToScreen(hWnd32->hWndParent,&pt);
	SetRect(lpRect,pt.x,pt.y,
		pt.x+(hWnd32->rWnd.right-hWnd32->rWnd.left),
		pt.y+(hWnd32->rWnd.bottom-hWnd32->rWnd.top));
    }
    else {
	SetRect(lpRect,hWnd32->rWnd.left,hWnd32->rWnd.top,
		hWnd32->rWnd.right,hWnd32->rWnd.bottom);
    }

    RELEASEWININFO(hWnd32);
}

BOOL WINAPI
GetClientRect(HWND hWnd, LPRECT lpRect)
{
    HWND32 hWnd32;

    APISTR((LF_APICALL, "GetClientRect(HWND=%x,RECT *=%x)\n",
	    hWnd, lpRect));

    if (!(hWnd32 = GETHWND32(hWnd))) {
	SetRectEmpty(lpRect);
    	APISTR((LF_APIFAIL, "GetClientRect: returns BOOL FALSE\n"));
	return FALSE;
    }

    SetRect(lpRect,0,0,hWnd32->wWidth,hWnd32->wHeight);

    RELEASEWININFO(hWnd32);

    APISTR((LF_APIRET, "GetClientRect: returns BOOL TRUE\n"));
    return (TRUE);
}

int WINAPI
GetClassName(HWND hWnd, LPSTR lpClassName, int cchClassName)
{
    HWND32 hWndTmp32;
    int rv;

    APISTR((LF_APICALL, "GetClassName(HWND=%x,LPSTR=%s,int=%d)\n",
	    hWnd, lpClassName,cchClassName));

    if (hWnd == 0) {
	APISTR((LF_APIFAIL,"GetClassName: returns int 0\n"));
	return 0;
    }
    if (!(hWndTmp32 = GETHWND32(hWnd))) {
	APISTR((LF_APIFAIL,"GetClassName: returns int 0\n"));
	return 0;
    }
    if (!hWndTmp32->hWindowClass32) {
	RELEASEWININFO(hWndTmp32);
	APISTR((LF_APIFAIL,"GetClassName: returns int 0\n"));
	return 0;
    }
    rv = InternalGetClassName(hWndTmp32->hWindowClass32,lpClassName,
		    		cchClassName);

    RELEASEWININFO(hWndTmp32);

    APISTR((LF_APIRET,"GetClassName: returns int %d\n",rv));
    return rv;
}

LONG WINAPI
GetClassLong(HWND hWnd, int nIndex)
{
    HWND32 hWndTmp32;
    LONG rv;

    APISTR((LF_APICALL, "GetClassLong(HWND=%x,int=%d)\n",
	    hWnd, nIndex));

    if (!(hWndTmp32 = GETHWND32(hWnd))) {
    	APISTR((LF_APIFAIL,"GetClassLong: returns int %d\n",0));
	return 0L;
    }
    if (!hWndTmp32->hWindowClass32) {
    	APISTR((LF_APIFAIL,"GetClassLong: returns int %d\n",0));
	RELEASEWININFO(hWndTmp32);
	return 0L;
    }

    rv = GetClassHandleLong(hWndTmp32->hWindowClass32,nIndex);

    RELEASEWININFO(hWndTmp32);
    
    APISTR((LF_APIRET,"GetClassLong: returns LONG %d\n",rv));
    return rv;
}

WORD WINAPI
GetClassWord(HWND hWnd, int nIndex)
{
    HWND32 hWndTmp32;
    WORD rv;

    APISTR((LF_APICALL, "GetClassWord(HWND=%x,int=%d)\n",
	    hWnd, nIndex));
    if (!(hWndTmp32 = GETHWND32(hWnd))) {
    	APISTR((LF_APIFAIL,"GetClassWord: returns WORD %d\n",0));
	return 0L;
    }
    if (!hWndTmp32->hWindowClass32) {
	RELEASEWININFO(hWndTmp32);
    	APISTR((LF_APIFAIL,"GetClassWord: returns WORD %d\n",0));
	return 0L;
    }

    rv = GetClassHandleWord(hWndTmp32->hWindowClass32,nIndex);

    RELEASEWININFO(hWndTmp32);

    APISTR((LF_APIRET,"GetClassWord: returns WORD %d\n",rv));
    return rv;
}

LONG WINAPI
SetClassLong(HWND hWnd, int nIndex, LONG dwNewLong)
{
    HWND32 hWndTmp32;
    LONG rv;

    APISTR((LF_APICALL, "SetClassLong(HWND=%x,int=%d,LONG=%x)\n",
	    hWnd, nIndex,dwNewLong));
    if (!(hWndTmp32 = GETHWND32(hWnd))) {
    	APISTR((LF_APIFAIL,"SetClassLong: returns LONG %d\n",0));
	return 0L;
    }
    if (!hWndTmp32->hWindowClass32) {
	RELEASEWININFO(hWndTmp32);
    	APISTR((LF_APIFAIL,"SetClassLong: returns LONG %d\n",0));
	return 0L;
    }

    rv = SetClassHandleLong(hWndTmp32->hWindowClass32,
				nIndex, dwNewLong);

    RELEASEWININFO(hWndTmp32);
    
    APISTR((LF_APIRET,"SetClassLong: returns LONG %d\n",rv));
    return rv;
}

WORD WINAPI
SetClassWord(HWND hWnd, int nIndex, WORD wNewWord)
{
    HWND32 hWndTmp32;
    WORD rv;

    APISTR((LF_APICALL, "SetClassWord(HWND=%x,int=%d,WORD=%x)\n",
	    hWnd, nIndex,wNewWord));

    if (!(hWndTmp32 = GETHWND32(hWnd))) {
    	APISTR((LF_APIRET,"SetClassWord: returns WORD %d\n",0));
	return 0L;
    }
    if (!hWndTmp32->hWindowClass32) {
	RELEASEWININFO(hWndTmp32);
    	APISTR((LF_APIRET,"SetClassWord: returns WORD %d\n",0));
	return 0L;
    }

    rv = SetClassHandleWord(hWndTmp32->hWindowClass32,
				nIndex, wNewWord);

    RELEASEWININFO(hWndTmp32);
    
    APISTR((LF_APIRET,"SetClassWord: returns WORD %d\n",rv));
    return rv;
}

/* Display and movement group */

HDWP WINAPI
BeginDeferWindowPos(int nNumWindows)
{
    LPMULTIWINPOS lpMultiWinPos;
    HANDLE hWinPos;

    APISTR((LF_APICALL, "BeginDeferWindowPos(int=%d)\n",
	    nNumWindows));
    if (nNumWindows < 0 || !(lpMultiWinPos = CREATEHPOS(hWinPos))) {
    	APISTR((LF_APIFAIL,"BeginDeferWindowPos: returns HDWP %x\n",0));
	return (HDWP)0;
    }

    if (!(lpMultiWinPos->lpWinPosInfo =
		(LPWINDOWPOS)WinMalloc(nNumWindows*sizeof(WINDOWPOS)))) {
	RELEASEPOSINFO(lpMultiWinPos);
	FREEHPOS(hWinPos);
    	APISTR((LF_APIFAIL,"BeginDeferWindowPos: returns HDWP %x\n",0));
	return (HDWP)0;
    }

    lpMultiWinPos->nMaxNumber = nNumWindows;
    lpMultiWinPos->nUsed = 0;

    RELEASEPOSINFO(lpMultiWinPos);
    APISTR((LF_APIRET,"BeginDeferWindowPos: returns HDWP %x\n", hWinPos));
    return hWinPos;
}

BOOL WINAPI
BringWindowToTop(HWND hWnd)
{
    HWND32 hWnd32;
    HWND hWndParent;
    WINDOWPOS wp;
    BOOL rc;

    APISTR((LF_APICALL,"BringWindowToTop(HWND=%x)\n",hWnd));

    if (!(hWnd32 = GETHWND32(hWnd))) {
    	APISTR((LF_APIFAIL,"BringWindowToTop: returns BOOL FALSE\n"));
	return FALSE;
    }

    hWndParent = (hWnd32->dwStyle & WS_CHILD)?
		hWnd32->hWndParent:GetDesktopWindow();

    RELEASEWININFO(hWnd32);

    if (hWnd != GetWindow(hWndParent,GW_CHILD)) {
	wp.hwnd = hWnd;
	wp.hwndInsertAfter = HWND_TOP;
	wp.x = wp.y = wp.cx = wp.cy = 0;
	wp.flags = SWP_NOMOVE|SWP_NOSIZE;
	rc =  InternalSetWindowPos(&wp,MATCH_CLIENT);
    	APISTR((LF_APIRET,"BringWindowToTop: returns BOOL %x\n",rc));
	return rc;
    }

    APISTR((LF_APIRET,"BringWindowToTop: returns BOOL TRUE\n"));
    return TRUE;
}

HDWP WINAPI
DeferWindowPos(HDWP hWinPosInfo, HWND hWnd, HWND hWndInsertAfter,
		int x, int y, int cx, int cy, UINT uiFlags)
{
    LPMULTIWINPOS lpMultiPosInfo;
    LPWINDOWPOS lpWinPosTemp;

    APISTR((LF_APICALL,
"DeferWindowPos(HDWP=%p,HWND=%x,HWND=%x,int=%d,int=%d,int=%d,int=%d,UINT=%x)\n",
	hWinPosInfo,hWnd,hWndInsertAfter,x,y,cx,cy,uiFlags));

    if (!(lpMultiPosInfo = GETPOSINFO(hWinPosInfo))) {
	APISTR((LF_APIFAIL,"DeferWindowPos: return HDWP 0\n"));
	return (HDWP)0;
    }

    if (lpMultiPosInfo->nUsed == lpMultiPosInfo->nMaxNumber) {
	if (!(lpMultiPosInfo->lpWinPosInfo =
		(LPWINDOWPOS)WinRealloc((LPSTR)(lpMultiPosInfo->lpWinPosInfo),
		sizeof(WINDOWPOS)*(lpMultiPosInfo->nMaxNumber + 1)))) {
	    RELEASEPOSINFO(lpMultiPosInfo);

	    APISTR((LF_APIFAIL,"DeferWindowPos: return HDWP 0\n"));
	    return (HDWP)0;
	}
	lpMultiPosInfo->nMaxNumber++;
    }

    lpWinPosTemp = &lpMultiPosInfo->lpWinPosInfo[lpMultiPosInfo->nUsed++];
    lpWinPosTemp->hwnd = hWnd;
    lpWinPosTemp->hwndInsertAfter = hWndInsertAfter;
    lpWinPosTemp->x = x;
    lpWinPosTemp->y = y;
    lpWinPosTemp->cx = cx;
    lpWinPosTemp->cy = cy;
    lpWinPosTemp->flags = uiFlags;

    RELEASEPOSINFO(lpMultiPosInfo);
    
    APISTR((LF_APIRET,"DeferWindowPos: returns HDWP %x\n",hWinPosInfo));
    return hWinPosInfo;
}

BOOL WINAPI
EndDeferWindowPos(HDWP hWinPosInfo)
{
    LPMULTIWINPOS lpMultiPosInfo;
    LPWINDOWPOS lpWinPosInfo;
    WORD nCount;
    BOOL ret = TRUE;

    APISTR((LF_APICALL, "EndDeferWindowPos(HDWP=%p)\n",hWinPosInfo));

    if (!(lpMultiPosInfo = GETPOSINFO(hWinPosInfo))) {
	APISTR((LF_APIFAIL,"EndDeferWindowPos: return BOOL FALSE\n"));
	return FALSE;
    }

    lpWinPosInfo = lpMultiPosInfo->lpWinPosInfo;
    
    for (nCount=0; nCount < (WORD)(lpMultiPosInfo->nUsed); nCount++) {
	ret &= InternalSetWindowPos(lpWinPosInfo,MATCH_CLIENT);
	lpWinPosInfo++;
    }

    RELEASEPOSINFO(lpMultiPosInfo);
    FREEHPOS(hWinPosInfo);
    APISTR((LF_APIRET,"EndDeferWindowPos: returns BOOL %d\n",ret));
    return ret;
}

static BOOL
InternalSetWindowPos(LPWINDOWPOS lpWindowPos, WORD wMatch)
{
    HWND32 hWnd32,hWndFrame32,hHScroll32,hVScroll32;
    DWORD dwClassStyle;
    DWORD dwStyle;
    HWND hWndClient,hWndFrame,hWndHZScroll,hWndVTScroll;
    RECT rc;
    int nHZDiff, nVTDiff;
    BOOL bPosChanged = FALSE;
    BOOL bNeedToMove = FALSE;
    RECT rcNewPos;
    POINT ptOldPos;
    SIZE szOldSize;
    RECT rcOldClientArea;

	if (!(hWnd32 = GETHWND32(lpWindowPos->hwnd))) {
	    return FALSE;
	}

	switch(wMatch) {
	    case MATCH_FRAME:
		hWndFrame = lpWindowPos->hwnd;
		hWndClient = hWnd32->hWndChild;
		hWndFrame32 = hWnd32;
		hWnd32 = GETHWND32(hWndClient);
		break;
	    case MATCH_CLIENT:
	 	hWndFrame = hWnd32->hWndFrame;
		hWndClient = lpWindowPos->hwnd;
		if (!(hWndFrame32 = GETHWND32(hWndFrame))) {
		    RELEASEWININFO(hWnd32);
		    return FALSE;
		}
		break;
	    default:
		RELEASEWININFO(hWnd32);
		return FALSE;
	}

	SetRect(&rcOldClientArea, 0, 0, hWnd32->wWidth, hWnd32->wHeight);

	SendMessage(hWndClient,WM_WINDOWPOSCHANGING,0,(LPARAM)lpWindowPos);

	hWndHZScroll = hWnd32->hWndHZScroll;
	hWndVTScroll = hWnd32->hWndVTScroll;
	dwStyle = hWnd32->dwStyle;
	dwClassStyle = GetClassStyle(hWndClient);

	/* Phase 1: if we are hiding, do it now */
	if (lpWindowPos->flags & SWP_HIDEWINDOW) {
	    hWnd32->dwStyle &= ~WS_VISIBLE;
	    hWndFrame32->dwStyle &= ~WS_VISIBLE;
	    hWnd32->dwWinFlags &= ~(WFMAPPED|WFMAPPINGPENDING);
	    hWndFrame32->dwWinFlags &= ~(WFMAPPED|WFMAPPINGPENDING);

	    if (!(lpWindowPos->flags & SWP_NOREDRAW)) {

		hWnd32->dwWinFlags &= ~WFVISIBILITY;

		if (hWnd32->dwStyle & WS_HSCROLL) {
		    if (hWnd32->hWndHZScroll) {
			hHScroll32 = GETHWND32(hWnd32->hWndHZScroll);
			hHScroll32->dwStyle &= ~WS_VISIBLE;
			RELEASEWININFO(hHScroll32);
		    }
		}
		if (hWnd32->dwStyle & WS_VSCROLL) {
		    if (hWnd32->hWndVTScroll) {
			hVScroll32 = GETHWND32(hWnd32->hWndVTScroll);
			hVScroll32->dwStyle &= ~WS_VISIBLE;
			RELEASEWININFO(hVScroll32);
		    }
		}


		if (!(dwStyle & WS_CHILD)) {
		    DRVCALL_WINDOWS(PWSH_SHOWWINDOW,
			DSW_HIDE, 0,
			WIN_GETDRVDATA(hWndFrame));
		    DRVCALL_WINDOWS(PWSH_FLUSHEXPOSES,0,0,0);
		}
		else
		    TWIN_GenerateExpose(hWnd32,EXPOSE_HIDE,NULL,NULL);

		/* DriverPeekTypedEvents on Expose in the driver code */
		/* may have caused WFMAPPED to be set as a result of */
		/* an Expose event handling, so it has to be reset again */
		hWnd32->dwWinFlags &= ~WFMAPPED;
		hWndFrame32->dwWinFlags &= ~WFMAPPED;

	    }

	    if (hWnd32->dwStyle & WS_MINIMIZE)
		ShowIconTitle(GETHWND16(hWnd32),FALSE);

	    /* you cannot hide and show at the same time */
	    lpWindowPos->flags &= ~SWP_SHOWWINDOW;

	    TWIN_RecalcVisRgns(hWnd32);
	}

	/* Phase 2: if we are changing Z-order, do it now */
	if (!(lpWindowPos->flags & SWP_NOZORDER))
	    ChangeZOrder(hWnd32, lpWindowPos->hwndInsertAfter,
			(BOOL)(lpWindowPos->flags & SWP_NOREDRAW));

	ptOldPos.x = hWnd32->rWnd.left;
	ptOldPos.y = hWnd32->rWnd.top;

	szOldSize.cx = hWndFrame32->rWnd.right-hWndFrame32->rWnd.left;
	szOldSize.cy = hWndFrame32->rWnd.bottom-hWndFrame32->rWnd.top;

	/* Phase 3: if we are moving, do it now */
	if (!(lpWindowPos->flags & SWP_NOMOVE)) {
	    /* Adjust for expected Windows values for soft requests */
	    if (!(hWnd32->dwStyle & WS_CHILD) &&
		    !(lpWindowPos->flags & SWP_DRIVER)) {
		CalcExpectedNC(&rc,dwStyle,hWnd32->dwExStyle);
		lpWindowPos->x += rc.left;
		lpWindowPos->y += rc.top;
	    }
	    if ((hWnd32->rWnd.left != lpWindowPos->x) ||
		(hWnd32->rWnd.top != lpWindowPos->y)) {

		if (lpWindowPos->flags & SWP_DRIVER) { 
		/* this is a "hard" move */
		    SetRect(&hWndFrame32->rWnd,
			lpWindowPos->x,
			lpWindowPos->y,
			lpWindowPos->x+hWndFrame32->rWnd.right-
				hWndFrame32->rWnd.left,
			lpWindowPos->y+hWndFrame32->rWnd.bottom-
				hWndFrame32->rWnd.top);
		    CopyRect(&hWnd32->rWnd,&hWndFrame32->rWnd);

		    bPosChanged = TRUE;
		}
		else {
		    /* this is a "soft" move */
		    rcNewPos.left = lpWindowPos->x;
		    rcNewPos.top = lpWindowPos->y;
		    rcNewPos.right = rcNewPos.bottom = 0;

		    bNeedToMove = TRUE;
		    bPosChanged = TRUE;

		     SetRect(&hWndFrame32->rWnd,
		        lpWindowPos->x,
			lpWindowPos->y,
			lpWindowPos->x+hWndFrame32->rWnd.right-
				hWndFrame32->rWnd.left,
			lpWindowPos->y+hWndFrame32->rWnd.bottom-
				hWndFrame32->rWnd.top);
		     CopyRect(&hWnd32->rWnd,&hWndFrame32->rWnd);
		}
	    }
	    else
		lpWindowPos->flags |= SWP_NOMOVE;

	}

	/* Phase 4: if we are resizing, do it now */
	if (!(lpWindowPos->flags & SWP_NOSIZE)) {
	    SetRect(&rc,0,0,1,1);
	    AdjustWindowRectEx(&rc,
			hWnd32->dwStyle,
			(hWnd32->hMenu)?TRUE:FALSE,
			hWnd32->dwExStyle);
	    lpWindowPos->cx = max(lpWindowPos->cx,rc.right-rc.left);
	    lpWindowPos->cy = max(lpWindowPos->cy,rc.bottom-rc.top);

	    /* Adjust for expected Windows values for soft requests */
	    if (!(hWnd32->dwStyle & WS_CHILD) &&
		!(lpWindowPos->flags & SWP_DRIVER)) {
		CalcExpectedNC(&rc,dwStyle,hWnd32->dwExStyle);
		lpWindowPos->cx -= (rc.left + rc.right);
		lpWindowPos->cy -= (rc.top + rc.bottom);
	    }

	    /* calculate the delta for horizontal and vertical */
	    nHZDiff = hWndFrame32->rWnd.right - hWndFrame32->rWnd.left -
			lpWindowPos->cx;
	    nVTDiff = hWndFrame32->rWnd.bottom - hWndFrame32->rWnd.top -
			lpWindowPos->cy;

	    /* if they differ, then we need to resize */
	    if (nHZDiff || nVTDiff) {

		bPosChanged = TRUE;

		/* set the frame size */
		hWndFrame32->rWnd.right = hWndFrame32->rWnd.left+
					  lpWindowPos->cx;
		hWndFrame32->rWnd.bottom = hWndFrame32->rWnd.top+
					   lpWindowPos->cy;

		/* set the client area rectangle */
		hWnd32->rWnd.right = hWndFrame32->rWnd.right;
		hWnd32->rWnd.bottom = hWndFrame32->rWnd.bottom;

		/* calc the NC & client rects of the client */
		CopyRect(&rc,&hWnd32->rWnd);

		/* this is the frame width, ie. the menu width */
		/* so don't set the height...ie. the menu height */
		hWndFrame32->wWidth = lpWindowPos->cx;

		/* let user tell us the correct client size */
		SendMessage(hWndClient, WM_NCCALCSIZE, 0, (LPARAM)(&rc));

		/* new client size... */
		hWnd32->wWidth = rc.right - rc.left;
		hWnd32->wHeight = rc.bottom - rc.top;

		/* invalidate exposed client area */
		if (((nHZDiff && (dwClassStyle & CS_HREDRAW)) ||
			 (nVTDiff && (dwClassStyle & CS_VREDRAW))) &&
	    		 !(lpWindowPos->flags & SWP_NOREDRAW)) 
		    InvalidateRect(hWndClient, NULL, TRUE);

		if (!(lpWindowPos->flags & SWP_DRIVER)) {
		    /* resize the frame to user specification */
		    SetRect(&rc,0,0,lpWindowPos->cx,lpWindowPos->cy);
		    if (bNeedToMove) {
			rc.left = rcNewPos.left;
			rc.top = rcNewPos.top;
			if (!(dwStyle & WS_CHILD)) 
			    DRVCALL_WINDOWS(PWSH_MOVERESIZEFRAME,
				0L,&rc,
				WIN_GETDRVDATA(hWndFrame));
			else if (!(lpWindowPos->flags & SWP_NOREDRAW)) 
			    TWIN_GenerateExpose(hWnd32,EXPOSE_SIZEMOVE,
					&ptOldPos,&szOldSize);
			TWIN_OffsetDCOrigins(hWnd32,
				rcNewPos.left-ptOldPos.x,
				rcNewPos.top-ptOldPos.y);
			bNeedToMove = FALSE;
		    }
		    else
			if (!(dwStyle & WS_CHILD))
			    DRVCALL_WINDOWS(PWSH_RESIZEFRAME,
				0L,&rc,
				WIN_GETDRVDATA(hWndFrame));
			else if (!(lpWindowPos->flags & SWP_NOREDRAW))
			    TWIN_GenerateExpose(hWnd32,EXPOSE_SIZE,
					&ptOldPos,&szOldSize);
		}
		else {
		    if (!(dwStyle & WS_CHILD) &&
				(lpWindowPos->cx != szOldSize.cx))
			/* force invalidation of the  menu bar */
			TWIN_RedrawWindow(hWndClient,NULL,0,
					RDW_FRAME|RDW_INVALIDATE);
		}

		/* notify the frame */
		SendMessage(hWndFrame, WM_SIZE, SIZENORMAL,
			MAKELPARAM(lpWindowPos->cx,lpWindowPos->cy));

		/* resize the HZ scroll, if any */
		if (hWndHZScroll) {
			hHScroll32 = GETHWND32(hWndHZScroll);
			CalcSysScrollLocation(hWnd32,
				lpWindowPos->cx,
				lpWindowPos->cy,
				&rc,
				FALSE);

			if (rc.right && rc.bottom) {
			    hHScroll32->wWidth = (WORD)rc.right;
			    hHScroll32->wHeight = (WORD)rc.bottom;
			    SetRect(&hHScroll32->rWnd,rc.left,rc.top,
				rc.left+rc.right,
				rc.top+rc.bottom);
			    SendMessage(hWndHZScroll, WM_SIZE, SIZENORMAL,
					MAKELPARAM(rc.right,rc.bottom));
			}
			RELEASEWININFO(hHScroll32);
		}

		/* resize the VT scroll, if any */
		if (hWndVTScroll) {
			hVScroll32 = GETHWND32(hWndVTScroll);
			CalcSysScrollLocation(hWnd32,
				lpWindowPos->cx,
				lpWindowPos->cy,
				&rc,
				TRUE);

			if (rc.right && rc.bottom) {
			    hVScroll32->wWidth = (WORD)rc.right;
			    hVScroll32->wHeight = (WORD)rc.bottom;
			    SetRect(&hVScroll32->rWnd,rc.left,rc.top,
				rc.left+rc.right,
				rc.top+rc.bottom);
			    SendMessage(hWndVTScroll, WM_SIZE, SIZENORMAL,
					MAKELPARAM(rc.right,rc.bottom));
			}
			RELEASEWININFO(hVScroll32);
		}

		TWIN_RecalcVisRgns(hWnd32);

		if (lpWindowPos->flags & SWP_DRIVER) {
		    if (rcOldClientArea.right < (int)hWnd32->wWidth)
		    {
		    	rc.top = rcOldClientArea.top;
		    	rc.left = rcOldClientArea.right;
		    	rc.bottom = hWnd32->wHeight;
		    	rc.right = hWnd32->wWidth;
		    	InvalidateRect(hWndClient, &rc, TRUE);
		    }

		    if (rcOldClientArea.bottom < (int)hWnd32->wHeight)
		    {
		    	rc.top = rcOldClientArea.bottom;
		    	rc.left = rcOldClientArea.left;
		    	rc.bottom = hWnd32->wHeight;
		    	rc.right = hWnd32->wWidth;
			InvalidateRect(hWndClient, &rc, TRUE);
		    }
		}

	    }		/* end of width/height changed */
	    else
		lpWindowPos->flags |= SWP_NOSIZE;
	}		/* end of 'resize' */

	if (bNeedToMove) { /* we still need to physically move */
	    if (!(hWnd32->dwStyle & WS_CHILD))
		DRVCALL_WINDOWS(PWSH_MOVEFRAME,
		    0L,&rcNewPos,WIN_GETDRVDATA(hWndFrame));
	    else if (!(lpWindowPos->flags & SWP_NOREDRAW))
		TWIN_GenerateExpose(hWnd32,EXPOSE_MOVE,&ptOldPos,&szOldSize);
	    TWIN_OffsetDCOrigins(hWnd32,
			lpWindowPos->x-ptOldPos.x,
			lpWindowPos->y-ptOldPos.y);
	}

	/* Phase 5: if we are showing, do it now */
	if (lpWindowPos->flags & SWP_SHOWWINDOW) {
	    BOOL bLogicallyVisible = FALSE;

	    hWnd32->dwStyle |= WS_VISIBLE;
	    hWndFrame32->dwStyle |= WS_VISIBLE;

	    /* in case of SWP_NOREDRAW if the window is already logically */
	    /* visible, do not bother showing it again and waiting... */
	    if ((bLogicallyVisible = ((lpWindowPos->flags & SWP_NOREDRAW) &&
		(hWnd32->dwWinFlags & WFVISIBILITY))))
		hWnd32->dwWinFlags |= WFMAPPED;
	    else {
		hWnd32->dwWinFlags |= WFMAPPINGPENDING;
		/* turn logical visibility on to force the call to */
	       	/* DrvWaitEvent in the for loop below */
		hWnd32->dwWinFlags |= WFVISIBILITY;

		if (hWnd32->dwStyle & WS_HSCROLL) {
		    if (hWnd32->hWndHZScroll) {
			hHScroll32 = GETHWND32(hWnd32->hWndHZScroll);
			hHScroll32->dwStyle |= WS_VISIBLE;
			if (hWnd32->dwStyle & WS_CHILD) {
			    hHScroll32->dwWinFlags |= WFMAPPED;
			    hHScroll32->dwWinFlags &= ~WFMAPPINGPENDING;
			}
		    }
		}
		if (hWnd32->dwStyle & WS_VSCROLL) {
		    if (hWnd32->hWndVTScroll) {
			hVScroll32 = GETHWND32(hWnd32->hWndVTScroll);
			hVScroll32->dwStyle |= WS_VISIBLE;
			if (hWnd32->dwStyle & WS_CHILD) {
			    hVScroll32->dwWinFlags |= WFMAPPED;
			    hVScroll32->dwWinFlags &= ~WFMAPPINGPENDING;
			}
		    }
		}

		if (!(dwStyle & WS_CHILD)) {
		    DRVCALL_WINDOWS(PWSH_SHOWWINDOW,
			DSW_SHOW, 0,
			WIN_GETDRVDATA(hWndFrame));
		    DRVCALL_WINDOWS(PWSH_FLUSHEXPOSES,0,0,0);
		}
		else {
		    hWnd32->dwWinFlags |= WFMAPPED;
		    hWndFrame32->dwWinFlags |= WFMAPPED;
		    hWnd32->dwWinFlags &= ~WFMAPPINGPENDING;
		    hWndFrame32->dwWinFlags &= ~WFMAPPINGPENDING;
		    if (!(lpWindowPos->flags & SWP_NOREDRAW)) {
			InvalidateRect(hWndClient,NULL,TRUE);
			hWnd32->dwWinFlags |= WFNCDIRTY;
		    }
		}
		if (hWnd32->dwStyle & WS_MINIMIZE)
		    ShowIconTitle(GETHWND16(hWnd32),TRUE);
	    }

	    if (!bLogicallyVisible &&
		!(hWnd32->dwExStyle & WS_EX_TRANSPARENT) &&
		IsWindowVisible(hWndClient)) {

		BOOL bInvisible = FALSE;

		for(;;) {
		    if (hWnd32->dwWinFlags & WFMAPPED)
			break;

		    if ((bInvisible = (hWnd32->dwWinFlags & WFVISIBILITY)?
				FALSE:TRUE))
			break;

		    (void)DriverWaitEvent(FALSE);
		}
		/* if invisible and parentdc, will possibly draw into parent */
		if (bInvisible && (dwClassStyle & CS_PARENTDC)) {
			hWnd32->dwWinFlags |= WFVISIBILITY;
			hWnd32->dwWinFlags |= WFMAPPED;
			hWnd32->dwWinFlags &= ~WFMAPPINGPENDING;
			InvalidateRgn(hWndClient,(HRGN)0,TRUE);
		}
	    }
	    else
		if ((dwClassStyle & CS_PARENTDC) ||
		    (hWnd32->dwExStyle & WS_EX_TRANSPARENT)) {
		    hWnd32->dwWinFlags |= WFVISIBILITY;
		    hWnd32->dwWinFlags |= WFMAPPED;
		    hWnd32->dwWinFlags &= ~WFMAPPINGPENDING;
		    InvalidateRgn(hWndClient,(HRGN)0,TRUE);
		}

	    TWIN_RecalcVisRgns(hWnd32);
	}

	/* Phase 6: if we are activating or drawing the frame, do it now */
	if (!(lpWindowPos->flags & SWP_NOACTIVATE))
	    InternalSetActiveWindow(hWnd32);
	else
	    if (lpWindowPos->flags & SWP_DRAWFRAME)
		DrawWindowFrame(hWndClient, TRUE);

    if (bPosChanged) {
	SendMessage(hWndClient,WM_WINDOWPOSCHANGED,0,(LPARAM)lpWindowPos);
	if (hWnd32->dwStyle & WS_MINIMIZE &&
				IsWindow(hWnd32->hWndIconTitle))
	    DrawIconTitle(hWnd32->hWndIconTitle,DIT_RECALC);
    }

    RELEASEWININFO(hWnd32);
    RELEASEWININFO(hWndFrame32);
    return TRUE;
}

BOOL WINAPI
MoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight,
		BOOL bRepaint)
{
    UINT uiFlags;
    BOOL rc;


    APISTR((LF_APICALL,"MoveWindow(hWnd=%x,x=%d,y=%d,w=%d,h=%d,bPaint=%d)\n",
	hWnd,X,Y,nWidth,nHeight,bRepaint));

    uiFlags = SWP_NOZORDER|SWP_NOACTIVATE;

    if (!bRepaint)
	    uiFlags |= SWP_NOREDRAW;

    rc = SetWindowPos(hWnd,0,X,Y,nWidth,nHeight,uiFlags);
    APISTR((LF_APIRET,"MoveWindow: returns BOOL %d\n",rc));
    return rc;
}

BOOL WINAPI
SetWindowPos(HWND hWnd, HWND hWndInsertAfter,
		int X, int Y, int cx, int cy, UINT wFlags)
{
    BOOL rc;
    APISTR((LF_APICALL,
	"SetWindowPos(h:%.04x,hwndafter:%.04x,x=%x,y=%x,cx=%x,cy=%xfl:%x\n",
		hWnd, hWndInsertAfter, X,Y,cx,cy,wFlags));

    rc = EndDeferWindowPos(DeferWindowPos(BeginDeferWindowPos(1),
			hWnd, hWndInsertAfter, X, Y, cx, cy, wFlags));
    APISTR((LF_APIRET,"SetWindowPos: returns BOOL %d\n",rc));
    return rc;
}

static void ChangeZOrderMaximize(HWND32 hWnd32OldTop, HWND32 hWnd32NewTop)
{
    /* If the old top and new top variables are set then we need to check   */
    /* if the old top was maximized.  If it was, then maximize the new top. */
    if (hWnd32OldTop && hWnd32NewTop && 
	(hWnd32OldTop->dwStyle & WS_CHILD) &&
	(hWnd32NewTop->dwStyle & WS_CHILD) &&
	hWnd32OldTop->hWndParent == hWnd32NewTop->hWndParent &&
	(hWnd32OldTop->dwStyle & WS_MAXIMIZE))
    {
	ShowWindow(GETHWND16(hWnd32NewTop), SW_MAXIMIZE);
    }
}

static void
ChangeZOrder(HWND32 hWnd32, HWND hWndInsertAfter, BOOL fNoRedraw)
{
    HWND32 hWnd32Temp,hWndPrev32,hWnd32Parent,hWndPrevPopup32;
    HWND hWndPrev, hWndParent, hwnd;
    HRGN hRgn,hRgnOld = 0,hRgnNew;
    POINT pt;

    hWndPrev = GetWindow(GETHWND16(hWnd32), GW_HWNDPREVTREE);
    hWndPrev32 = NULL;

    if ((hWnd32->dwStyle & WS_CHILD) && !fNoRedraw) {
	hRgnOld = CreateRectRgn(0,0,
		hWnd32->rWnd.right-hWnd32->rWnd.left,
		hWnd32->rWnd.bottom-hWnd32->rWnd.top);
	TWIN_GenerateClip(hWnd32,hRgnOld,DCX_WINDOW,TRUE);
    }

    switch ((int)hWndInsertAfter) {
	case (int)HWND_TOPMOST:
	    hWnd32->dwExStyle |= WS_EX_TOPMOST;
	    /* fall through into HWND_TOP case */
	case (int)HWND_TOP:

	    /* find the first guy */
	    hWndParent = (hWnd32->dwStyle & WS_CHILD)?
			hWnd32->hWndParent:GetDesktopWindow();

	    hWnd32Parent = GETHWND32(hWndParent);
	    /* if we are the first, return */
	    if (hWnd32Parent->hWndChild == GETHWND16(hWnd32)) {
		if (hRgnOld)
		    DeleteObject(hRgnOld);
		RELEASEWININFO(hWnd32Parent);
		return;
	    }

	    hWndPrevPopup32 = hWnd32Temp = (HWND32)0;
	    if (!(hWnd32->dwStyle & WS_CHILD))
	    {
#ifdef	LATER
		/* there has to be a lot more code here, doing it right... */
#endif
		/* walk the Z-order list down, skipping popups owned by me */
	        hwnd = TWIN_EnumerateSiblingWindows(hWnd32Parent->hWndChild,
						    ChangeZOrderFindTop,
						    hWnd32);


		/* nothing to do, if all windows above me are my own */
		if (hwnd == GETHWND16(hWnd32)) {
		    if (hRgnOld)
			DeleteObject(hRgnOld);
		    RELEASEWININFO(hWnd32Parent);
		    return;
		}

		if (hwnd && hwnd != hWnd32Parent->hWndChild)
		{
		    hWndPrevPopup32 = GETHWND32(hwnd);
		    if (!(hWndPrevPopup32->dwStyle & WS_VISIBLE))
		    {
			RELEASEWININFO(hWndPrevPopup32);
			hWndPrevPopup32 = NULL;
		    }
		}
	    }

	    /* We may need to maximize the new top window. */
	    hWnd32Temp = GETHWND32(hWnd32Parent->hWndChild);
	    ChangeZOrderMaximize(hWnd32Temp, hWnd32);
	    RELEASEWININFO(hWnd32Temp);
		
	    /* adjust previous link */
	    hWndPrev32 = GETHWND32(hWndPrev);
	    hWndPrev32->hWndSibling = hWnd32->hWndSibling;
	    RELEASEWININFO(hWndPrev32);

	    if (hWndPrevPopup32) {
		hWnd32->hWndSibling = hWndPrevPopup32->hWndSibling;
		hWndPrevPopup32->hWndSibling = GETHWND16(hWnd32);
		/* this is for the call to PrivateWindowHook... */
		hWndInsertAfter = GETHWND16(hWndPrevPopup32);
	    }
	    else {
		/* point to the former first child*/
		hWnd32->hWndSibling = hWnd32Parent->hWndChild;
		/* tell the parent that I am the first */
		hWnd32Parent->hWndChild = GETHWND16(hWnd32);
	    }

	    RELEASEWININFO(hWnd32Parent);
	    RELEASEWININFO(hWndPrevPopup32);
	    break;

	case (int)HWND_BOTTOM:
	    /* find last sibling */
	    hWndParent = (hWnd32->dwStyle & WS_CHILD)?
			hWnd32->hWndParent:GetDesktopWindow();

	    hWnd32Parent = GETHWND32(hWndParent);

	    hwnd = TWIN_EnumerateSiblingWindows(hWnd32Parent->hWndChild,
						ChangeZOrderFindBottom,
						NULL);

	    /* if this is us, return */
	    if (hwnd == GETHWND16(hWnd32)) {
		if (hRgnOld)
		    DeleteObject(hRgnOld);
		RELEASEWININFO(hWnd32Parent);
		return;
	    }

	    /* We may need to maximize the new top window. */
	    if (hWnd32Parent->hWndChild == GETHWND16(hWnd32))
		ChangeZOrderMaximize(hWnd32, GETHWND32(hWnd32->hWndSibling));

	    RELEASEWININFO(hWnd32Parent);
		
	    /* link it to us */
	    hWnd32Temp = GETHWND32(hwnd);
	    hWnd32Temp->hWndSibling = GETHWND16(hWnd32);
	    RELEASEWININFO(hWnd32Temp);

	    /* adjust link for the previous */
	    hWndPrev32 = GETHWND32(hWndPrev);
	    hWndParent = (hWnd32->dwStyle & WS_CHILD)?
			hWnd32->hWndParent:GetDesktopWindow();
	    if (hWndPrev != hWndParent)
		hWndPrev32->hWndSibling = hWnd32->hWndSibling;
    	    else
		hWndPrev32->hWndChild = hWnd32->hWndSibling;
	    RELEASEWININFO(hWndPrev32);

	    /* indicate that we are last */
	    hWnd32->hWndSibling = 0;
	    hWnd32->dwExStyle &= ~WS_EX_TOPMOST;
	    break;

	case (int)HWND_NOTOPMOST:
	    /* find the first guy */
	    hWndParent = (hWnd32->dwStyle & WS_CHILD)?
			hWnd32->hWndParent:GetDesktopWindow();

	    hWnd32Parent = GETHWND32(hWndParent);
	    hwnd = TWIN_EnumerateSiblingWindows(hWnd32Parent->hWndChild,
						ChangeZOrderFindBottom,
						NULL);
	    hWndPrev32 = GETHWND32(hwnd);
	    if ((hWndPrev32->dwExStyle & WS_EX_TOPMOST) || 
		hWndPrev32 == hWnd32)
	    {
		if (hRgnOld)
		    DeleteObject(hRgnOld);
		RELEASEWININFO(hWnd32Parent);
		RELEASEWININFO(hWndPrev32);
		return;
	    }

	    /* We may need to maximize the new top window. */
	    if (hWnd32Parent->hWndChild == GETHWND16(hWnd32))
	    {
		hWnd32Temp = GETHWND32(hWnd32->hWndSibling);
		ChangeZOrderMaximize(hWnd32, hWnd32Temp);
		RELEASEWININFO(hWnd32Temp);
	    }

	    hWndInsertAfter = GETHWND16(hWndPrev32);

	    RELEASEWININFO(hWndPrev32);
	    RELEASEWININFO(hWnd32Parent);
	    break;

	default:
	    hWnd32Temp = NULL;
	    
	    if ((GETHWND16(hWnd32) == hWndInsertAfter) ||
		!(hWnd32Temp = GETHWND32(hWndInsertAfter))) 
	    {
		ERRSTR((LF_ERROR,
			"***ERROR*** ChangeZOrder: wrong hWndInsertAfter %x\n",
			hWndInsertAfter));
		if (hRgnOld)
		    DeleteObject(hRgnOld);
		if (hWnd32Temp)
		    RELEASEWININFO(hWnd32Temp);
		return;
	    }
	    /* adjust link for the previous */
	    hWndParent = (hWnd32->dwStyle & WS_CHILD)?
			hWnd32->hWndParent:GetDesktopWindow();

	    hWndPrev32 = GETHWND32(hWndPrev);

	    if (hWndPrev != hWndParent)
		hWndPrev32->hWndSibling = hWnd32->hWndSibling;
    	    else
	    {
		HWND32 hWndSibling32 = GETHWND32(hWnd32->hWndSibling);

		/* We may need to maximize the new top window. */
		ChangeZOrderMaximize(hWnd32, hWndSibling32);
		RELEASEWININFO(hWndSibling32);

		hWndPrev32->hWndChild = hWnd32->hWndSibling;
	    }
	    
	    /* my sibling link points to the next(hWndInsertAfter) */
	    hWnd32->hWndSibling = hWnd32Temp->hWndSibling;
	    /* hWndInsertAfter points to me */
	    hWnd32Temp->hWndSibling = GETHWND16(hWnd32);

	    RELEASEWININFO(hWnd32Temp);
	    RELEASEWININFO(hWndPrev32);
	    break;
    }

    if (!(hWnd32->dwStyle & WS_CHILD))
	DRVCALL_WINDOWS(PWSH_STACKINGORDER,
		hWndInsertAfter,
		(IsWindow(hWndInsertAfter))?
		    WIN_GETDRVDATA(GetWindowFrame(hWndInsertAfter)):0L,
		WIN_GETDRVDATA(hWnd32->hWndFrame));
    else if (!fNoRedraw) {
	hRgnNew = CreateRectRgn(0,0,
		hWnd32->rWnd.right-hWnd32->rWnd.left,
		hWnd32->rWnd.bottom-hWnd32->rWnd.top);
	TWIN_GenerateClip(hWnd32,hRgnNew,DCX_WINDOW,TRUE);
	hRgn = CreateRectRgn(0,0,0,0);

	/* invalidate our window's exposed area */
	if (CombineRgn(hRgn,hRgnNew,hRgnOld,RGN_DIFF) > NULLREGION) 
		TWIN_ExposeChildren(hWnd32,hRgn);

	/* invalidate the exposed areas of everybody higher than our guy */
	if (CombineRgn(hRgn,hRgnOld,hRgnNew,RGN_DIFF) > NULLREGION) {
	    struct TWIN_changezorderinvalidate_s inv_data;

	    pt.x = hWnd32->rWnd.left;
	    pt.y = hWnd32->rWnd.top;

	    inv_data.pt = pt;
	    inv_data.hrgn = hRgn;
	    inv_data.hwnd32 = hWnd32;
	    
	    hWnd32Parent = GETHWND32(hWndParent);

	    TWIN_EnumerateSiblingWindows(hWnd32Parent->hWndChild,
					 ChangeZOrderInvalidate,
					 &inv_data);

	    RELEASEWININFO(hWnd32Parent);
	}

	DeleteObject(hRgnOld);
	DeleteObject(hRgnNew);
	DeleteObject(hRgn);
    }
}

void
RemoveFromList(HWND32 hWnd32)
{
    HWND hWndPrev, hWndParent;
    HWND32 hWndPrev32,hWndParent32;
    char buffer[80];

    hWndPrev = GetWindow(GETHWND16(hWnd32), GW_HWNDPREVTREE);
    hWndPrev32 = GETHWND32(hWndPrev);

    hWndParent = (hWnd32->dwStyle & WS_CHILD)?
		hWnd32->hWndParent:GetDesktopWindow();
    hWndParent32 = GETHWND32(hWndParent);

    if (hWndPrev32 != hWndParent32)
	/* There is somebody before us in the parent's list */
	hWndPrev32->hWndSibling = hWnd32->hWndSibling;
    else
	hWndPrev32->hWndChild = hWnd32->hWndSibling;

    if (!(hWndPrev = GetWindow(GETHWND16(hWnd32), GW_HWNDPREVGROUP))) {
	sprintf(buffer,"RemoveFromList: group chain broken for %x\n",
		GETHWND16(hWnd32));
	ERRSTR((LF_ERROR,buffer));
	RELEASEWININFO(hWndParent32);
	RELEASEWININFO(hWndPrev32);
	FatalAppExit(0,buffer);
    }
    if (!(hWndPrev32 = GETHWND32(hWndPrev))) {
	sprintf(buffer,"RemoveFromList: bad prevgroup link %x for %x\n",
		hWndPrev, GETHWND16(hWnd32));
	ERRSTR((LF_ERROR,buffer));
	RELEASEWININFO(hWndParent32);
	RELEASEWININFO(hWndPrev32);
	FatalAppExit(0,buffer);
    }

    if (hWndPrev != GETHWND16(hWnd32)) {
	hWndPrev32->hWndGroup = hWnd32->hWndGroup;
	if (GETHWND16(hWnd32) == hWndParent32->hWndLastChild)
	    /* if we are destroying parent's hWndLastChild */
	    /* set it to the previous group link */
	    hWndParent32->hWndLastChild = hWndPrev;
    }
    else
	if (GETHWND16(hWnd32) == hWndParent32->hWndLastChild)
	    /* or set it to NULL */
	    hWndParent32->hWndLastChild = (HWND)0;

    RELEASEWININFO(hWndParent32);
    RELEASEWININFO(hWndPrev32);
}

BOOL 
TWIN_InsertAsFirstSibling(HWND32 hWnd32)
{
    HWND32 hWndParent32,hWndTemp32;
    HWND hWndFirstChild,hWndParent;

    if (!hWnd32) {
	ERRSTR((LF_ERROR,
	    "***ERROR*** TWIN_InsertAsFirstSibling: bad parameter\n"));
	return FALSE;
    }

    hWndParent = ((hWnd32->dwStyle & WS_CHILD) && hWnd32->hWndParent)?
		hWnd32->hWndParent:GetDesktopWindow();
    if (!(hWndParent32 = GETHWND32(hWndParent))) {
	ERRSTR((LF_ERROR,
	    "***ERROR*** TWIN_InsertAsFirstSibling: bad parent\n"));
	return FALSE;
    }

    hWndFirstChild = hWndParent32->hWndChild;
    hWndParent32->hWndChild = GETHWND16(hWnd32);
    hWnd32->hWndSibling = hWndFirstChild;

    if ((hWnd32->dwStyle & WS_GROUP) || !hWndFirstChild) {
	hWnd32->hWndGroup = GETHWND16(hWnd32);
    }
    else {
	hWnd32->hWndGroup = hWndFirstChild;
	hWndFirstChild = GetWindow(hWndFirstChild,GW_HWNDPREVGROUP);
	if (!(hWndTemp32 = GETHWND32(hWndFirstChild))) {
	    ERRSTR((LF_ERROR,
		    "***ERROR*** TWIN_InsertAsFirstSibling: bad group link\n"));
	    RELEASEWININFO(hWndParent32);
	    return FALSE;
	}
	hWndTemp32->hWndGroup = GETHWND16(hWnd32);
    }


    RELEASEWININFO(hWndParent32);
    RELEASEWININFO(hWndTemp32);
    return TRUE;
}

BOOL
InsertAsLastSibling(HWND32 hWnd32)
{
    HWND32 hWndParent32,hWndTemp32;
    HWND hWndParent, hWndTemp;

    if (!hWnd32) {
	ERRSTR((LF_ERROR,
	    "***ERROR*** InsertAsLastSibling: bad parameter\n"));
	return FALSE;
    }

    hWndParent = ((hWnd32->dwStyle & WS_CHILD) && hWnd32->hWndParent)?
		hWnd32->hWndParent:GetDesktopWindow();
    if (!(hWndParent32 = GETHWND32(hWndParent))) {
	ERRSTR((LF_ERROR,
	    "***ERROR*** InsertAsLastSibling: bad parent\n"));
	return FALSE;
    }

    if (!hWndParent32->hWndChild) {
	hWndParent32->hWndChild = GETHWND16(hWnd32);
	hWnd32->hWndGroup = GETHWND16(hWnd32);
    }
    else {
	hWndTemp = TWIN_EnumerateSiblingWindows(hWndParent32->hWndChild,
						ChangeZOrderFindBottom,
						NULL);

	if (!hWndTemp || !(hWndTemp32 = GETHWND32(hWndTemp))) {
	    ERRSTR((LF_ERROR,
		    "***ERROR*** InsertAsLastSibling: "
		    "bad first child or sibling\n"));
	    RELEASEWININFO(hWndParent32);
	    return FALSE;
	}

	hWndTemp32->hWndSibling = GETHWND16(hWnd32);
	RELEASEWININFO(hWndTemp32);

	if (!(hWnd32->dwStyle & WS_GROUP) &&
	    (hWnd32->dwStyle & WS_CHILD)) {
	    if (hWndParent32->hWndLastChild) {
		hWndTemp32 = GETHWND32(hWndParent32->hWndLastChild);
		hWnd32->hWndGroup = hWndTemp32->hWndGroup;
		hWndTemp32->hWndGroup = GETHWND16(hWnd32);
		RELEASEWININFO(hWndTemp32);
	    }
	    else
		hWnd32->hWndGroup = GETHWND16(hWnd32);
	}
	else
	    hWnd32->hWndGroup = GETHWND16(hWnd32);
    }
    hWnd32->hWndSibling = (HWND)0;

    RELEASEWININFO(hWndParent32);
    return TRUE;
}

int WINAPI
GetWindowText(HWND hWnd, LPSTR lpString, int nMaxCount)
{
    HWND32 hWnd32;

    ASSERT_HWND(hWnd32,hWnd,0);
    RELEASEWININFO(hWnd32);

    APISTR((LF_API,"GetWindowText: hWnd %x, max %d\n",hWnd,nMaxCount));

    return SendMessage(hWnd,WM_GETTEXT,(WPARAM)nMaxCount,(LPARAM)lpString);
}

int WINAPI
GetWindowTextLength(HWND hWnd)
{
    HWND32 hWnd32;

    ASSERT_HWND(hWnd32,hWnd,0);
    RELEASEWININFO(hWnd32);

    APISTR((LF_API,"GetWindowTextLength: hWnd %x",hWnd));

    return SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0L);
}

#ifdef TWIN32
BOOL WINAPI
#else
void WINAPI
#endif
SetWindowText(HWND hWnd, LPCSTR lpString)
{
    HWND32 hWnd32;

    if (!(hWnd32 = GETHWND32(hWnd))) {
	ERRSTR((LF_ERROR,"***ERROR*** bad HWND %x\n",hWnd));
#ifdef TWIN32
	return FALSE;
#else
	return;
#endif
    }
    RELEASEWININFO(hWnd32);

    APISTR((LF_API,"SetWindowText(h:%.04x,lpstr:%s)\n",
	hWnd, (lpString)?lpString:"NULL"));

#ifdef TWIN32
    return (BOOL)SendMessage(hWnd, WM_SETTEXT,0,(LPARAM)lpString);
#else
    (void)SendMessage(hWnd, WM_SETTEXT,0,(LPARAM)lpString);
#endif
}

LONG
InternalSetText(HWND hWnd, LPSTR lpWindowName)
{
    HWND32 hWnd32;
    LONG rv;

    hWnd32 = GETHWND32(hWnd);
    if (!lpWindowName) {
	if (hWnd32->lpWindowName)
	    WinFree(hWnd32->lpWindowName);
	hWnd32->lpWindowName = NULL;
	if (!(hWnd32->dwStyle & WS_CHILD))
	    DRVCALL_WINDOWS(PWSH_SETTEXT,
		0L,"",
		(hWnd32->hWndFrame)?
		    WIN_GETDRVDATA(hWnd32->hWndFrame):
		    WIN_GETDRVDATA(hWnd));
	RELEASEWININFO(hWnd32);
	return 1L;
    }
    if (!hWnd32->lpWindowName) {
	if ((hWnd32->lpWindowName =
	    WinMalloc(strlen(lpWindowName) + 1)) == NULL) {
	    ERRSTR((LF_ERROR,
		"***ERROR*** InternalSetText: WinMalloc failed\n"));
	    RELEASEWININFO(hWnd32);
	    return 0L;
	}
    }
    else {
	if ((int)strlen(lpWindowName) > (int)strlen(hWnd32->lpWindowName))
	    if ((hWnd32->lpWindowName =
		WinRealloc(hWnd32->lpWindowName, strlen(lpWindowName) + 1))
			== NULL) {
		    ERRSTR((LF_ERROR,
		    "***ERROR*** InternalSetText: WinRealloc failed\n"));
		    RELEASEWININFO(hWnd32);
		    return 0L;
	    }
    }

    if (!(hWnd32->dwStyle & WS_CHILD))
	DRVCALL_WINDOWS(PWSH_SETTEXT,
		0L,lpWindowName,
		(hWnd32->hWndFrame)?
		    WIN_GETDRVDATA(hWnd32->hWndFrame):
		    WIN_GETDRVDATA(hWnd));

    rv = (LONG)strcpy(hWnd32->lpWindowName,lpWindowName);

    RELEASEWININFO(hWnd32);
    return rv;
}

int
InternalGetText(HWND hWnd, LPSTR lpText, int nMaxCount)
{
    HWND32 hWnd32;
    int rv;

    hWnd32 = GETHWND32(hWnd);
    if (hWnd32->lpWindowName)
    {
	rv = strlen(strncpy(lpText, hWnd32->lpWindowName,
		min(nMaxCount, (int)strlen(hWnd32->lpWindowName)+1)));
	RELEASEWININFO(hWnd32);
	return rv;
    }
    else
	lpText[0] = 0;

    RELEASEWININFO(hWnd32);
    return(0);
}
	
int
InternalGetTextLength(HWND hWnd)
{
    HWND32 hWnd32;
    int rv;

    hWnd32 = GETHWND32(hWnd);
    if (hWnd32->lpWindowName)
    {
	rv = (strlen(hWnd32->lpWindowName));
	RELEASEWININFO(hWnd32);
	return rv;
    }
    
    RELEASEWININFO(hWnd32);
    return(0);
}

void
InternalFreeText(HWND hWnd)
{
    HWND32 hWnd32;

    hWnd32 = GETHWND32(hWnd);
    if (hWnd32->lpWindowName) {
	WinFree(hWnd32->lpWindowName);
	hWnd32->lpWindowName = NULL;
    }
    RELEASEWININFO(hWnd32);
}

/**********************************************************************/

BOOL WINAPI
IsIconic(HWND hWnd)
{
    HWND32 hWnd32;
    BOOL bIconic;

    APISTR((LF_APICALL,"IsIconic(HWND=%x)\n",hWnd));

    if (!(hWnd32 = GETHWND32(hWnd))) {
    	APISTR((LF_APIRET,"IsIconic: returns BOOL FALSE\n"));
	return FALSE;
    }

    bIconic = (hWnd32->dwStyle & WS_MINIMIZE)?TRUE:FALSE;
    RELEASEWININFO(hWnd32);

    APISTR((LF_APIRET,"IsIconic: returns BOOL %d\n",bIconic));
    return bIconic;
}

BOOL WINAPI
IsZoomed(HWND hWnd)
{
    HWND32 hWnd32;
    BOOL rv;

    if (!(hWnd32 = GETHWND32(hWnd)))
	return FALSE;
    rv = (hWnd32->dwStyle & WS_MAXIMIZE)?TRUE:FALSE;
    RELEASEWININFO(hWnd32);
    return rv;
}

BOOL WINAPI
IsWindowEnabled(HWND hWnd)
{
    HWND32 hWnd32;
    BOOL   rc;

    if (!IsWindow(hWnd))
	return FALSE;

    if (!(hWnd32 = GETHWND32(hWnd)))
	return FALSE;

    rc = (hWnd32->dwStyle & WS_DISABLED)?FALSE:TRUE;
    APISTR((LF_API,"IsWindowEnabled(hWnd:%.04x) returns %d\n",hWnd,rc));
    RELEASEWININFO(hWnd32);
    return rc;
}

BOOL WINAPI
IsChild(HWND hWndParent, HWND hWnd)
{
    HWND32 hWnd32;
    BOOL   rc = FALSE;


    if (!(hWnd32 = GETHWND32(hWnd)))  {
    	APISTR((LF_API,"IsChild(hPar:%.04x,hWnd:%.04x) returns 0 (bad hWnd)\n",
		hWndParent,hWnd,rc));
	return FALSE;
    }

    while ((hWnd32->dwStyle & WS_CHILD) && hWnd32->hWndParent
		&& (hWnd32->hWndParent != hWndParent))
	hWnd32 = GETHWND32(hWnd32->hWndParent);

    rc =  (hWnd32->hWndParent == hWndParent);

    APISTR((LF_API,
	"IsChild(hPar:%.04x,h:%.04x) returns %d\n",hWndParent,hWnd,rc));
    RELEASEWININFO(hWnd32);
    return rc;
}

BOOL
IsTopLevel(HWND hWnd)
{
    HWND32 hWnd32;
    BOOL   rc;

    if (!(hWnd32 = GETHWND32(hWnd))) {
    	APISTR((LF_API,"IsTopLevel(hWnd:%.04x) returns 0 (bad hWnd)\n",hWnd));
	return FALSE;
    }

    rc = (hWnd32->dwStyle & WS_CHILD)?FALSE:TRUE;

    APISTR((LF_API,"IsTopLevel(h:%.04x) returns %x\n",hWnd,rc));

    RELEASEWININFO(hWnd32);
    return rc;
}

BOOL WINAPI
IsWindow(HWND hWnd)
{
    HWND32 hWnd32;
    BOOL   rc;

    if (hWnd == 0)
	return FALSE;

    hWnd32 = CHECKHWND32(hWnd);

    rc = (hWnd32)?TRUE:FALSE;

    RELEASEWININFO(hWnd32);

    return rc;
}

HWND
GetTopLevelAncestor(HWND hWnd)
{
    HWND32 hWnd32;
    HWND32 hWndNext32;
    HWND hwnd;

    if (!(hWnd32 = GETHWND32(hWnd)))
	return (HWND)0;
    while (hWnd32->dwStyle & WS_CHILD)
    {
	hWndNext32 = GETHWND32(hWnd32->hWndParent);
	RELEASEWININFO(hWnd32);
	hWnd32 = hWndNext32;
    }
    
    hwnd = GETHWND16(hWnd32);
    RELEASEWININFO(hWnd32);
    return hwnd;
}

HWND
TWIN_GetTopLevelFrame(HWND hWnd)
{
    HWND hWndTopLevel,hWndFrame;

    if (0 != (hWndTopLevel = GetTopLevelAncestor(hWnd))) {
	hWndFrame = GetWindowFrame(hWndTopLevel);
	return (hWndFrame)?hWndFrame:hWndTopLevel;
    }
    else
	return (HWND)0;
}
LONG
TWIN_GetWindowData(HWND hWnd)
{
	HWND	hWndFrame;

    	hWndFrame = TWIN_GetTopLevelFrame(hWnd);

	return WIN_GETDRVDATA(hWndFrame);
}

BOOL WINAPI
IsWindowVisible(HWND hWnd)
{
    HWND32 hWnd32;
    HWND32 hWndNext32;

    ASSERT_HWND(hWnd32, hWnd, FALSE);

    if (!(hWnd32->dwStyle & WS_VISIBLE))
    {
	RELEASEWININFO(hWnd32);
	return FALSE;
    }

    while ((hWnd32->dwStyle & WS_CHILD) && hWnd32->hWndParent) {
	hWndNext32 = GETHWND32(hWnd32->hWndParent);
	RELEASEWININFO(hWnd32);
	hWnd32 = hWndNext32;
	
	if (!(hWnd32->dwStyle & WS_VISIBLE))
	{
	    RELEASEWININFO(hWnd32);
	    return FALSE;
	}
    }
    RELEASEWININFO(hWnd32);
    return TRUE;
}

BOOL
HaveDisabledAncestor(HWND hWnd)
{
    HWND32 hWnd32;
    HWND32 hWndNext32;

    if (!(hWnd32 = GETHWND32(hWnd)))
	return TRUE;

    if (hWnd32->dwStyle & WS_DISABLED)
    {
	RELEASEWININFO(hWnd32);
	return(TRUE);
    }

    while ((hWnd32->dwStyle & WS_CHILD) && hWnd32->hWndParent) {
	hWndNext32 = GETHWND32(hWnd32->hWndParent);
	RELEASEWININFO(hWnd32);
	hWnd32 = hWndNext32;

	if (!hWnd32)
	    return TRUE;
	
	if (hWnd32->dwStyle & WS_DISABLED)
	{
	    RELEASEWININFO(hWnd32);
	    return TRUE;
	}
    }

    RELEASEWININFO(hWnd32);
    return(FALSE);
}

HWND WINAPI
GetParent(HWND hWnd)
{
    HWND32 hWnd32;
    HWND hWndParent;

    if (!(hWnd32 = GETHWND32(hWnd)))
	return (HWND)0;

    if (!(hWnd32->dwStyle & (WS_CHILD|WS_POPUP)))
	hWndParent = (HWND)0;
    else
	hWndParent = (hWnd32->dwStyle & WS_CHILD)?
		hWnd32->hWndParent:
		hWnd32->hWndOwner;

    APISTR((LF_API,"GetParent(hWnd:%x) returns %x\n",
	hWnd,hWndParent));

    RELEASEWININFO(hWnd32);
    return hWndParent;
}

/**************************************************************************/

void WINAPI
CloseWindow(HWND hWnd)
{
    HWND32 hWnd32;

    if (!(hWnd32 = GETHWND32(hWnd))) {
	ERRSTR((LF_ERROR,"***ERROR*** CloseWindow: bad hWnd %x\n",hWnd));
	return;
    }

    APISTR((LF_API, "CloseWindow(hWnd:%x)\n",hWnd));

    if (!(hWnd32->dwStyle & WS_POPUP) && !(hWnd32->dwStyle & WS_CHILD))
	ShowWindow(hWnd,SW_MINIMIZE);

    RELEASEWININFO(hWnd32);
}

typedef struct tagSOPENUMSTRUCT
  {
	HWND  hWndOwner;
	BOOL  fShow;
  } SOPENUMSTRUCT;

void WINAPI
ShowOwnedPopups(HWND hWnd, BOOL fShow)
{
    HWND32 hWnd32;
    SOPENUMSTRUCT sopenum;

    if (!(hWnd32 = GETHWND32(hWnd))) {
	ERRSTR((LF_ERROR,"***ERROR*** ShowOwnedPopups: bad hWnd %x\n",hWnd));
	return;
    }
    RELEASEWININFO(hWnd32);

    APISTR((LF_API, "ShowOwnedPopups(hWnd:%x,show:%d)\n",hWnd,fShow));

    sopenum.hWndOwner = hWnd;
    sopenum.fShow = fShow;
    EnumWindows((WNDENUMPROC)ShowOwnedPopupsEnumProc, (LPARAM)&sopenum);
}

static BOOL
ShowOwnedPopupsEnumProc(HWND hWnd, LPARAM lParam)
{
    HWND32 hWnd32;
    SOPENUMSTRUCT *lpsopenum = (SOPENUMSTRUCT *)lParam;

    if (!(hWnd32 = GETHWND32(hWnd)))
	return TRUE;
    if ((hWnd32->dwStyle & WS_POPUP) &&
	(hWnd32->hWndOwner == lpsopenum->hWndOwner)) {
	if ((hWnd32->dwStyle & WS_VISIBLE) && !lpsopenum->fShow)
	    ShowWindow(hWnd,SW_HIDE);
	if (!(hWnd32->dwStyle & WS_VISIBLE) && lpsopenum->fShow)
	    ShowWindow(hWnd,SW_SHOW);
    }
    RELEASEWININFO(hWnd32);
    return FALSE;
}

BOOL WINAPI
OpenIcon(HWND hWnd)
{
    return ShowWindow(hWnd,SW_SHOWNORMAL);
}

BOOL WINAPI
ShowWindow(HWND hWnd, int nCmdShow)
{
    HWND32 hWnd32;
    HWND hWndTop;
    DWORD dwWinFlags, dwStyle;
    WORD wSWPflags;
    BOOL bVisibleState;

    if (!(hWnd32 = GETHWND32(hWnd))) {
	ERRSTR((LF_ERROR,"***ERROR*** ShowWindow: HM_GETOBJ failed\n"));
	return(FALSE);
    }

    APISTR((LF_API,
	"ShowWindow(h:%.04x,nCmdShow:%d)\n",hWnd,nCmdShow));

    dwWinFlags = hWnd32->dwWinFlags;
    dwStyle = hWnd32->dwStyle;

    bVisibleState = (dwWinFlags & (WFMAPPED|WFMAPPINGPENDING))?TRUE:FALSE;

    if (((bVisibleState && nCmdShow != SW_HIDE) ||
	(!bVisibleState && nCmdShow == SW_HIDE)) &&
	(UINT)nCmdShow == hWnd32->showCmd)
    {
	RELEASEWININFO(hWnd32);
	return bVisibleState;
    }

    wSWPflags = SWP_NOSIZE | SWP_NOMOVE;

    /* switch on the command */
    switch(nCmdShow) {
	case SW_HIDE:	/* hide and pass activation to another window */

	    /* if we are already invisible, return */
	    if (!bVisibleState)
	    {
		RELEASEWININFO(hWnd32);
		return FALSE;
	    }

	    wSWPflags |= SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER;

	    /* notify the guy */
	    SendMessage(hWnd,WM_SHOWWINDOW,(WPARAM)0,(LPARAM)0);

	    /* do the job */
	    SetWindowPos(hWnd,(HWND)0,0,0,0,0,(UINT)wSWPflags);
	    break;


	case SW_SHOWMINIMIZED: /* activate and minimize */
	case SW_SHOWMINNOACTIVE: /* minimize and do not activate */
	case SW_MINIMIZE: /* minimize and activate next window */
	    /* if we are visible and minimized, do nothing */
	    if (!(bVisibleState && (dwStyle & WS_MINIMIZE))) 
		MinMaximize(hWnd,nCmdShow,TRUE);
	    if (!bVisibleState) {
		wSWPflags= SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER;
		SetWindowPos(hWnd,(HWND)0,0,0,0,0,(UINT)wSWPflags);
	    }
	    break;

	case SW_MAXIMIZE:
	    /* if we are visible and maximized, do nothing */
	    if (!(bVisibleState && (dwStyle & WS_MAXIMIZE))) 
		MinMaximize(hWnd,SW_MAXIMIZE,TRUE);
	    
	    if (!bVisibleState) {
		wSWPflags = SWP_SHOWWINDOW|SWP_NOSIZE|
					SWP_NOMOVE|SWP_NOZORDER;
		SendMessage(hWnd,WM_SHOWWINDOW,(WPARAM)1,(LPARAM)0);
		SetWindowPos(hWnd,(HWND)0,0,0,0,0,(UINT)wSWPflags);
	    }
	    break;

	case SW_SHOWNORMAL:
	case SW_RESTORE:
	    if ((hWnd32->showCmd == SW_SHOWNORMAL) ||
		(hWnd32->showCmd == SW_RESTORE))
	    {
		RELEASEWININFO(hWnd32);
		return TRUE;
	    }

	    hWnd32->showCmd = (UINT)nCmdShow;
	    wSWPflags = 0;
	    
	    if (dwStyle & WS_MINIMIZE) 
		MinMaximize(hWnd,SW_MINIMIZE,FALSE);
	    else if (dwStyle & WS_MAXIMIZE) {
		MinMaximize(hWnd,SW_MAXIMIZE,FALSE);
		hWndTop = GetTopLevelAncestor(hWnd);
	    }

	    /* if we are invisible, show window */
	    if (!bVisibleState) {
		wSWPflags |= SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE;
	        SendMessage(hWnd,WM_SHOWWINDOW,(WPARAM)1,(LPARAM)0);
		SetWindowPos(hWnd,(HWND)0,0,0,0,0,(UINT)wSWPflags);
		SendMessage(hWnd,WM_SIZE,(WPARAM)SIZE_RESTORED,
			MAKELPARAM(hWnd32->wWidth,hWnd32->wHeight));
#ifdef	LATER
		send WM_MOVE??
#endif
	    }	
	    RELEASEWININFO(hWnd32);
	    return bVisibleState;

	case SW_SHOWNOACTIVATE:
#ifdef	LATER
	what is the deal here??
#endif
	case SW_SHOW:
	case SW_SHOWNA:
	    wSWPflags = SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER;
	    
	    /* if we are already active, no need to activate again */
	    if (nCmdShow != SW_SHOW
#ifdef LATER
		|| GetTopLevelAncestor(hWnd) == GetActiveWindow()
#endif
					)
		wSWPflags |= SWP_NOACTIVATE;

	    /* if we are invisible, show window */
	    if (!bVisibleState) {
	        SendMessage(hWnd,WM_SHOWWINDOW,(WPARAM)1,(LPARAM)0);
		SetWindowPos(hWnd,(HWND)0,0,0,0,0,(UINT)wSWPflags);
		SendMessage(hWnd,WM_SIZE,(WPARAM)SIZE_RESTORED,
			MAKELPARAM(hWnd32->wWidth,hWnd32->wHeight));
#ifdef	LATER
		send WM_MOVE??
#endif
	    }	
	    break;

	default:
	    break;
    }
    hWnd32->showCmd = (UINT)nCmdShow;
    RELEASEWININFO(hWnd32);
    return bVisibleState;
}

void
MinMaximize(HWND hWnd,UINT nShow,BOOL fAction)
{
    if (IsTopLevel(hWnd))
	TopMinMaximize(hWnd,nShow,fAction);
    else
	ChildMinMaximize(hWnd,nShow,fAction);
}

static void
TopMinMaximize(HWND hWnd,UINT nShow,BOOL fAction)
{
    HWND32 hWnd32;
    BOOL bVisible;
    HDWP hDWP;
    RECT rc;
    UINT uiFlags = 0;

    if (!(hWnd32 = GETHWND32(hWnd)))
	return;
    bVisible = (hWnd32->dwWinFlags & (WFMAPPED|WFMAPPINGPENDING))?TRUE:FALSE;

    if (fAction) {	/* do the job */
	switch (nShow) {

	case SW_MAXIMIZE:
	    SetRect(&rc,0,0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN));
	    hWnd32->ptMaxPosition.x = 0;
	    hWnd32->ptMaxPosition.y = 0;
	    hWnd32->ptMaxSize.x = rc.right;
	    hWnd32->ptMaxSize.y = rc.bottom;
	    hWnd32->dwStyle |= WS_MAXIMIZE;
	    if (hWnd32->dwStyle & WS_MINIMIZE) { 
#ifdef	LATER
		/* We better do something, or else... */
#endif
		hWnd32->dwStyle &= ~WS_MINIMIZE;
	    }
	    else 		/* memorize normal position */
		hWnd32->rcNormalPosition = hWnd32->rWnd;
	    if (bVisible) {
		hDWP = BeginDeferWindowPos(3);
		DeferWindowPos(hDWP,hWnd,(HWND)0,
                        0,0,0,0,
                        SWP_HIDEWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER);
		DeferWindowPos(hDWP,hWnd,(HWND)0,
		    	hWnd32->ptMaxPosition.x,hWnd32->ptMaxPosition.y,
			hWnd32->ptMaxSize.x,hWnd32->ptMaxSize.y,
			SWP_NOREDRAW|SWP_NOACTIVATE|SWP_NOZORDER);
		DeferWindowPos(hDWP,hWnd,(HWND)0,
                        0,0,0,0,
			SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER);
		EndDeferWindowPos(hDWP);
	    }
	    else {
		SetWindowPos(hWnd,(HWND)0,
		    	hWnd32->ptMaxPosition.x,hWnd32->ptMaxPosition.y,
			hWnd32->ptMaxSize.x,hWnd32->ptMaxSize.y,
			SWP_NOREDRAW|SWP_NOACTIVATE|SWP_NOZORDER);
		SendMessage(hWnd,WM_SIZE,(WPARAM)SIZE_MAXIMIZED,
			MAKELPARAM( hWnd32->ptMaxSize.x -
				    (hWnd32->rcNC.left+hWnd32->rcNC.right),
				    hWnd32->ptMaxSize.y -
				    (hWnd32->rcNC.top+hWnd32->rcNC.bottom)));
	    }

	    break;

	case SW_MINIMIZE:
	case SW_SHOWMINIMIZED:
	case SW_SHOWMINNOACTIVE:

	{
		HWND hWndFrame;
		hWnd32->dwStyle |= WS_MINIMIZE;
    		hWndFrame = TWIN_GetTopLevelFrame(hWnd);
    		DRVCALL_WINDOWS(PWSH_ICONIFY,0L,0L, WIN_GETDRVDATA(hWndFrame));
	    	break;

	}

	default:
	    break;
	}
    }
    else { 	/* undo the changes */
	uiFlags = 0;
	    
	 /* if we are already active, no need to activate again */
	if (hWnd == GetActiveWindow())
	    uiFlags |= SWP_NOACTIVATE;

	if (nShow == SW_MAXIMIZE)
	    hWnd32->dwStyle &= ~WS_MAXIMIZE;

	CalcNCDimensions(&rc,hWnd32->dwStyle,
			hWnd32->dwExStyle,
		    	hWnd32->wMenuHeight);
	hDWP = BeginDeferWindowPos(2);
	DeferWindowPos(hDWP,hWnd,(HWND)0,
		0,0,0,0,
		SWP_HIDEWINDOW|SWP_NOSIZE|
		SWP_NOMOVE|SWP_NOZORDER);
	DeferWindowPos(hDWP,hWnd,HWND_TOP,
		hWnd32->rcNormalPosition.left,
		hWnd32->rcNormalPosition.top,
		hWnd32->rcNormalPosition.right - hWnd32->rcNormalPosition.left,
		hWnd32->rcNormalPosition.bottom - hWnd32->rcNormalPosition.top,
		(UINT)uiFlags);
	EndDeferWindowPos(hDWP);
	ChangeNCDimensions(hWnd,&rc);
	if (bVisible) 
	    SetWindowPos(hWnd,(HWND)0,
		0,0,0,0,
		SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER);
	SendMessage(hWnd,WM_SIZE,(WPARAM)SIZE_RESTORED,
			MAKELPARAM(hWnd32->wWidth,hWnd32->wHeight));
    }

    RELEASEWININFO(hWnd32);
}

#define WFCHILDHIDDEN 0x02000000

static void
ChildMinMaximize(HWND hWnd,UINT nShow,BOOL fAction)
{
    HWND32 hWnd32;
    BOOL bVisible;
    HDWP hDWP;
    RECT rc;
    UINT uiFlags = 0;
    HWND hWndTop=0, hWndNext;
    int nNumChildren;
    HWND hWndChild;

    if (!(hWnd32 = GETHWND32(hWnd)))
	return;
    bVisible = (hWnd32->dwWinFlags & (WFMAPPED|WFMAPPINGPENDING))?TRUE:FALSE;

    if (fAction) {	/* do the job */
	switch (nShow) {

	case SW_MAXIMIZE:
	    /* If we have a maximized sibling, then we need to restore */
	    /* it before we maximize.                                  */
	    if (hWnd32->hWndParent)
	    {
		for (hWndNext = GetWindow(hWnd32->hWndParent,GW_CHILD);
		     hWndNext;
		     hWndNext = GetWindow(hWndNext,GW_HWNDNEXTSIB)) 
		{
		    if (IsZoomed(hWndNext)) 
		    {
			ShowWindow(hWndNext, SW_RESTORE);
			break;
		    }
		}
	    }
		
	    if (hWnd32->dwStyle & WS_MINIMIZE) { 
		CalcNCDimensions(&rc,hWnd32->dwStyle,
			hWnd32->dwExStyle,
		    	hWnd32->wMenuHeight);
		ChangeNCDimensions(hWnd,&rc);
	    }
	    GetClientRect(hWnd32->hWndParent,&rc);
	    hWnd32->ptMaxPosition.x = rc.left - hWnd32->rcNC.left;
	    hWnd32->ptMaxPosition.y = rc.top - hWnd32->rcNC.top;
	    hWnd32->ptMaxSize.x = rc.right + hWnd32->rcNC.left +
				hWnd32->rcNC.right;
	    hWnd32->ptMaxSize.y = rc.bottom + hWnd32->rcNC.top +
				hWnd32->rcNC.bottom;
	    hWnd32->dwStyle |= WS_MAXIMIZE;
	    if ((hWnd32->dwStyle & WS_CAPTION) == WS_CAPTION) {
		hWndTop = GetTopLevelAncestor(hWnd);
		SetWF(hWndTop,WFCHILDMAXIMIZED);
		ClearWF(hWnd,WFRESTOREMAXIMIZED);
	    }
	    if (hWnd32->dwStyle & WS_MINIMIZE) { 
		hWnd32->dwStyle &= ~WS_MINIMIZE;
		ShowIconTitle(hWnd,FALSE);

		for (hWndChild = hWnd32->hWndChild,nNumChildren = 0;
			hWndChild;
			hWndChild = GetWindow(hWndChild,GW_HWNDNEXT)) {
		    if (GetWF(hWndChild) & WFCHILDHIDDEN)
			nNumChildren++;
		}
		hDWP = BeginDeferWindowPos(nNumChildren + 2);
		for (hWndChild = hWnd32->hWndChild; hWndChild;
			hWndChild = GetWindow(hWndChild,GW_HWNDNEXT)) {
		    if (GetWF(hWndChild) & WFCHILDHIDDEN) {
			DeferWindowPos(hDWP,hWndChild,0,0,0,0,0,
				SWP_SHOWWINDOW|SWP_NOACTIVATE|SWP_NOZORDER);
			ClearWF(hWndChild,WFCHILDHIDDEN);
		    }
		}
	    }
	    else {		/* memorize normal position */
		hWnd32->rcNormalPosition = hWnd32->rWnd;
		hDWP = BeginDeferWindowPos(2);
	    }

	    DeferWindowPos(hDWP,hWnd,(HWND)0,
                        0,0,0,0,
                        SWP_HIDEWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER);
	    DeferWindowPos(hDWP,hWnd,(HWND)0,
		    	hWnd32->ptMaxPosition.x,hWnd32->ptMaxPosition.y,
			hWnd32->ptMaxSize.x,hWnd32->ptMaxSize.y,
			SWP_NOREDRAW|SWP_NOACTIVATE|SWP_NOZORDER);
	    EndDeferWindowPos(hDWP);

	    if (bVisible) 
		SetWindowPos(hWnd,(HWND)0,0,0,0,0,
			SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER);
	    else 
		SendMessage(hWnd,WM_SIZE,(WPARAM)SIZE_MAXIMIZED,
			MAKELPARAM( hWnd32->ptMaxSize.x -
				    (hWnd32->rcNC.left+hWnd32->rcNC.right),
				    hWnd32->ptMaxSize.y -
				    (hWnd32->rcNC.top+hWnd32->rcNC.bottom)));

	    if ((hWnd32->dwStyle & WS_CAPTION) == WS_CAPTION) {
		ModifyMenuBar(hWndTop,hWnd,SW_MAXIMIZE);
		DrawWindowFrame(hWndTop, TRUE);
	    }

	    break;

	case SW_MINIMIZE:
	case SW_SHOWMINIMIZED:
	case SW_SHOWMINNOACTIVE:

	    uiFlags = SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER;
	    /* if we are already active, no need to activate again */
	    if (nShow != SW_SHOWMINIMIZED ||
		GetTopLevelAncestor(hWnd) == GetActiveWindow())
		uiFlags |= SWP_NOACTIVATE;

	    /* if minimized already, just show */
	    if (hWnd32->dwStyle & WS_MINIMIZE) {
		SendMessage(hWnd,WM_SHOWWINDOW,(WPARAM)1,(LPARAM)0);
		SetWindowPos(hWnd,(HWND)0,
			0,0,0,0,
			(UINT)uiFlags|SWP_SHOWWINDOW);
		SendMessage(hWnd,WM_SIZE,(WPARAM)SIZE_MINIMIZED,
			MAKELPARAM( hWnd32->ptMinSize.x,
				    hWnd32->ptMinSize.y));
		break;
	    }

	    if (hWnd32->dwStyle & WS_CHILD) {
		/* calculate a new iconized position */
		if (!GetAvailableIconPosition(hWnd32,&rc))
		{
		    RELEASEWININFO(hWnd32);
		    return;
		}
		
		/* remember the iconized position in parent coords */
		hWnd32->ptMinPosition.x = rc.left;
		hWnd32->ptMinPosition.y = rc.top;
		hWnd32->ptMinSize.x = rc.right;
		hWnd32->ptMinSize.y = rc.bottom;

		/* set the iconized state */
		hWnd32->dwNormalStyle = hWnd32->dwStyle;
		hWnd32->dwStyle |= WS_MINIMIZE;
		if (hWnd32->dwStyle & WS_MAXIMIZE) {
		    hWndTop = GetTopLevelAncestor(hWnd);
		    hWnd32->dwStyle &= ~WS_MAXIMIZE;
		    if (TestWF(hWndTop,WFCHILDMAXIMIZED)) {
			ClearWF(hWndTop,WFCHILDMAXIMIZED);
			for (hWndChild = GetWindow(GetParent(hWnd),GW_CHILD);
				hWndChild;
				hWndChild = GetWindow(hWndChild,GW_HWNDNEXT)) {
			    if (IsZoomed(hWndChild))
				ChildMinMaximize(hWndChild,SW_MAXIMIZE,FALSE);
			}
			ModifyMenuBar(hWndTop,hWnd,SW_RESTORE);
			DrawWindowFrame(hWndTop, TRUE);
		    }
		    hWnd32->dwWinFlags |= WFRESTOREMAXIMIZED;
		}
		else {	/* only when not maximized */
		/* remember the normal position */
		    hWnd32->rcNormalPosition = hWnd32->rWnd;
		}

		if (hWnd32->dwNormalStyle & (WS_HSCROLL|WS_VSCROLL))
		    ShowScrollBar(hWnd,SB_BOTH,FALSE);

		/* make NC rect empty */
		SetRectEmpty(&rc);
		ChangeNCDimensions(hWnd,&rc);

		if (bVisible) {
		    /* logically hide all its visible children */
		    /* hide the guy, move/resize him, then show him back */
		    for (hWndChild = hWnd32->hWndChild,nNumChildren = 0;
			hWndChild;
			hWndChild = GetWindow(hWndChild,GW_HWNDNEXT)) {
			if (IsWindowVisible(hWndChild))
			    nNumChildren++;
		    }
		    hDWP = (nShow == SW_MINIMIZE)?
				BeginDeferWindowPos(nNumChildren+4):
				BeginDeferWindowPos(nNumChildren+3);
		    for (hWndChild = hWnd32->hWndChild; hWndChild;
				hWndChild = GetWindow(hWndChild,GW_HWNDNEXT)) {
			if (IsWindowVisible(hWndChild)) {
			    DeferWindowPos(hDWP,hWndChild,0,0,0,0,0,
					SWP_HIDEWINDOW|SWP_NOACTIVATE|
					SWP_NOZORDER);
			    SetWF(hWndChild,WFCHILDHIDDEN);
			}
		    }
		    DeferWindowPos(hDWP,hWnd,(HWND)0,
			0,0,0,0,
			SWP_HIDEWINDOW|SWP_NOSIZE|SWP_NOACTIVATE|
			SWP_NOMOVE|SWP_NOZORDER);
		    DeferWindowPos(hDWP,hWnd,(HWND)0,
			hWnd32->ptMinPosition.x,
			hWnd32->ptMinPosition.y,
			hWnd32->ptMinSize.x,
			hWnd32->ptMinSize.y,
			SWP_NOREDRAW|SWP_NOACTIVATE|SWP_NOZORDER);
		    if (nShow == SW_MINIMIZE) {
			DeferWindowPos(hDWP,hWnd,HWND_BOTTOM,
				       0,0,0,0,
				       SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
			/* activate somebody at the same level */
			for (hWndNext = GetWindow(hWnd32->hWndParent,GW_CHILD);
			     hWndNext;
			     hWndNext = GetWindow(hWndNext,GW_HWNDNEXTSIB)) {
			    if (GetWindow(hWndNext,GW_OWNER))
				continue;
			    if (hWndNext != hWnd) {
				DeferWindowPos(hDWP,hWndNext,HWND_TOP,
					0,0,0,0,
					SWP_NOSIZE|SWP_NOMOVE);
				break;
			    }
			}
		    }
		    else {
			DeferWindowPos(hDWP,hWnd,(HWND) 0,
				       0,0,0,0,
				       uiFlags|SWP_SHOWWINDOW);
		    }
		    
		    EndDeferWindowPos(hDWP);
		}
		else {
		    SendMessage(hWnd,WM_SHOWWINDOW,(WPARAM)1,(LPARAM)0);
		    SetWindowPos(hWnd,(HWND)0,
			hWnd32->ptMinPosition.x,
			hWnd32->ptMinPosition.y,
			hWnd32->ptMinSize.x,
			hWnd32->ptMinSize.y,
			SWP_NOREDRAW|SWP_NOACTIVATE|SWP_NOZORDER);
#ifdef	LATER
		    SendMessage(hWnd,WM_SIZE,(WPARAM)SIZE_MINIMIZED,
			    MAKELPARAM( hWnd32->ptMinSize.x,
					hWnd32->ptMinSize.y));
		send WM_MOVE??
#endif
		}
		ModifySystemMenu(hWnd,SW_MINIMIZE);
	    }
	    break;

	default:
	    break;
	}
	hWnd32->showCmd = nShow;
    }
    else { 	/* undo the changes */
	uiFlags = 0;
	    
	 /* if we are already active, no need to activate again */
	if (GetTopLevelAncestor(hWnd) == GetActiveWindow())
	    uiFlags |= SWP_NOACTIVATE;

	if (nShow == SW_MINIMIZE) {
	    /* if we are minimized, restore normal position */
	    /* remember current minimized position in parent coords */
	    hWnd32->ptMinPosition.x = hWnd32->rWnd.left;
	    hWnd32->ptMinPosition.y = hWnd32->rWnd.top;
#ifdef	LATER
	in case of top-level, parent is NULL!
#endif
	    if (hWnd32->dwWinFlags & WFRESTOREMAXIMIZED) {
		MinMaximize(hWnd,SW_MAXIMIZE,TRUE);
		RELEASEWININFO(hWnd32);
		return;
	    }

	    ShowIconTitle(hWnd,FALSE);

	    /* reset the iconic flag and do it */
	    hWnd32->dwStyle &= ~WS_MINIMIZE;

	    if (hWnd32->dwNormalStyle & (WS_VSCROLL|WS_HSCROLL)) {
		int fnBar = 0;

		if (hWnd32->dwNormalStyle & WS_VSCROLL)
		    fnBar = SB_VERT;
		if (hWnd32->dwNormalStyle & WS_HSCROLL)
		    fnBar = (fnBar == SB_VERT) ? SB_BOTH : SB_HORZ;
		ShowScrollBar(hWnd,fnBar,TRUE);
	    }
	}
	else if (nShow == SW_MAXIMIZE) {
	    hWnd32->dwStyle &= ~WS_MAXIMIZE;
	    hWndTop = GetTopLevelAncestor(hWnd);
	    ClearWF(hWndTop,WFCHILDMAXIMIZED);
	    hWnd32->dwWinFlags &= ~WFRESTOREMAXIMIZED;
	}

	CalcNCDimensions(&rc,hWnd32->dwStyle,
			hWnd32->dwExStyle,
		    	hWnd32->wMenuHeight);

	for (hWndChild = hWnd32->hWndChild,nNumChildren = 0;
		hWndChild;
		hWndChild = GetWindow(hWndChild,GW_HWNDNEXT)) {
	    if (GetWF(hWndChild) & WFCHILDHIDDEN)
		nNumChildren++;
	}
	hDWP = BeginDeferWindowPos(nNumChildren + 2);

	for (hWndChild = hWnd32->hWndChild; hWndChild;
		hWndChild = GetWindow(hWndChild,GW_HWNDNEXT)) {
	    if (GetWF(hWndChild) & WFCHILDHIDDEN) {
		DeferWindowPos(hDWP,hWndChild,0,0,0,0,0,
				SWP_SHOWWINDOW|SWP_NOACTIVATE|SWP_NOZORDER);
		ClearWF(hWndChild,WFCHILDHIDDEN);
	    }
	}
	DeferWindowPos(hDWP,hWnd,(HWND)0,
		0,0,0,0,
		SWP_HIDEWINDOW|SWP_NOSIZE|
		SWP_NOMOVE|SWP_NOZORDER);
	DeferWindowPos(hDWP,hWnd,HWND_TOP,
		hWnd32->rcNormalPosition.left,
		hWnd32->rcNormalPosition.top,
		hWnd32->rcNormalPosition.right - hWnd32->rcNormalPosition.left,
		hWnd32->rcNormalPosition.bottom - hWnd32->rcNormalPosition.top,
		(UINT)uiFlags|SWP_NOREDRAW);
	EndDeferWindowPos(hDWP);
	ChangeNCDimensions(hWnd,&rc);
	if (bVisible) 
	    SetWindowPos(hWnd,(HWND)0,
		0,0,0,0,
		SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER);
	SendMessage(hWnd,WM_SIZE,(WPARAM)SIZE_RESTORED,
			MAKELPARAM(hWnd32->wWidth,hWnd32->wHeight));
	if (nShow == SW_MAXIMIZE) {
		ModifyMenuBar(hWndTop,hWnd,SW_RESTORE);
		DrawWindowFrame(hWndTop, TRUE);
	}
    }

    RELEASEWININFO(hWnd32);
}

UINT WINAPI
ArrangeIconicWindows(HWND hWnd)
{
    HWND32 hWnd32;
    HWND hWndChild;
    int nCount;
    HDWP hDWP;
    int nXSpacing,nYSpacing,nXIcon,nYIcon;
    int nIconsInRow,nRow,nColumn;

    if (!(hWnd32 = GETHWND32(hWnd)))
	return 0;	
    for (hWndChild = hWnd32->hWndChild,nCount = 0;
		hWndChild;
		hWndChild = GetWindow(hWndChild, GW_HWNDNEXT)) {
	if (IsIconic(hWndChild) && GetWindow(hWndChild, GW_OWNER) == 0) 
	    nCount++;
    }
    if (nCount == 0) 
    {
	RELEASEWININFO(hWnd32);
	return 0;
    }
    
    nXIcon = GetSystemMetrics(SM_CXICON);
    nYIcon = GetSystemMetrics(SM_CYICON);
    nXSpacing = GetSystemMetrics(SM_CXICONSPACING);
    nYSpacing = GetSystemMetrics(SM_CYICONSPACING);
    nIconsInRow = (int)hWnd32->wWidth/(nXIcon+nXSpacing);
    hDWP = BeginDeferWindowPos(nCount);
    for (hWndChild = hWnd32->hWndChild,nCount = 0;
		hWndChild;
		hWndChild = GetWindow(hWndChild, GW_HWNDNEXT)) {
	if (IsIconic(hWndChild) && GetWindow(hWndChild, GW_OWNER) == 0) {
    	    nRow = nCount/nIconsInRow;
	    nColumn = nCount%nIconsInRow;
	    hWnd32->dwMinPosIndex = MAKELONG((WORD)nColumn,(WORD)nRow);
	    DeferWindowPos(hDWP,hWndChild,(HWND)0,
		nColumn*(nXSpacing+nXIcon) + nXSpacing/2,
		hWnd32->wHeight - (nRow+1)*(nYSpacing+nYIcon),
		nXIcon,nYIcon, SWP_NOZORDER|SWP_NOACTIVATE);
	    nCount++;
	}
    };
    EndDeferWindowPos(hDWP);
    RELEASEWININFO(hWnd32);
    return (nYIcon + nYSpacing);
}

static BOOL
GetAvailableIconPosition(HWND32 hWnd32, LPRECT lprcIcon)
{
    RECT rcParent,rc,rcIcon;
    HWND hWndChild;
    int nCount = 0;
    HWND32 hWndChild32;
    POINT pt;
    int nXSpacing,nYSpacing,nXIcon,nYIcon;
    int nIconsInRow,nRow,nColumn;

#ifdef	LATER
	if top-level, hWndParent == 0
#endif
    if (!hWnd32->hWndParent) {
	SetRectEmpty(lprcIcon);
	return FALSE;
    }
    GetClientRect(hWnd32->hWndParent,&rcParent);
    nXIcon = GetSystemMetrics(SM_CXICON);
    nYIcon = GetSystemMetrics(SM_CYICON);
    nXSpacing = GetSystemMetrics(SM_CXICONSPACING);
    nYSpacing = GetSystemMetrics(SM_CYICONSPACING);
    nIconsInRow = (rcParent.right - rcParent.left)/
		(nXIcon+nXSpacing);
    
    for (;;) {
	nRow = nCount/nIconsInRow;
	nColumn = nCount%nIconsInRow;
	pt.x = nColumn*(nXSpacing+nXIcon) + nXSpacing/2;
	pt.y = rcParent.bottom - (nRow+1)*(nYSpacing+nYIcon);
	SetRect(&rcIcon,pt.x,pt.y,pt.x+nXIcon,pt.y+nYIcon);
	hWndChild = GetWindow(hWnd32->hWndParent, GW_CHILD);
	do {
	    hWndChild32 = GETHWND32(hWndChild);
	    if ((hWndChild32->dwStyle & WS_MINIMIZE) && 
			IntersectRect(&rc,&hWndChild32->rWnd,&rcIcon))
	    {
		RELEASEWININFO(hWndChild32);
		break;
	    }

	    RELEASEWININFO(hWndChild32);
	} while ((hWndChild = GetWindow(hWndChild, GW_HWNDNEXT)) != 0);
	if (hWndChild == 0)
	    break;
	nCount++;
    }
	
    lprcIcon->left = pt.x;
    lprcIcon->top = pt.y;
    lprcIcon->right = nXIcon;
    lprcIcon->bottom = nYIcon;
    return TRUE;
}

BOOL WINAPI
GetWindowPlacement(HWND hWnd, LPWINDOWPLACEMENT lpWndPl)
{
    HWND32 hWnd32;

    APISTR((LF_API,"GetWindowPlacement(hWnd:%x,lpwndpl=%x)\n",hWnd,lpWndPl));

    ASSERT_HWND(hWnd32, hWnd, FALSE);

    lpWndPl->length = sizeof(WINDOWPLACEMENT);
    lpWndPl->flags = 0;	/* TODO WPF_RESTORETOMAXIMIZED */
    lpWndPl->showCmd = hWnd32->showCmd;
    lpWndPl->ptMinPosition = hWnd32->ptMinPosition;
    lpWndPl->ptMaxPosition = hWnd32->ptMaxPosition;

    /* Get the normal position based on the window's current state. */
    if ((hWnd32->dwStyle & WS_MINIMIZE)||(hWnd32->dwStyle & WS_MAXIMIZE))
      lpWndPl->rcNormalPosition = hWnd32->rcNormalPosition;
    else
      lpWndPl->rcNormalPosition = hWnd32->rWnd;


    RELEASEWININFO(hWnd32);
    return TRUE;
}

BOOL WINAPI
SetWindowPlacement(HWND hWnd, const WINDOWPLACEMENT *lpWndPl)
{
    HWND32 hWnd32;
    UINT flags;

    APISTR((LF_API,
    "SetWindowPlacement(h:%x,lpwndpl=[cmd:%d,min:%d,%d,max:%d,%d,norm=%d,%d-%d,%d])\n",
	hWnd,lpWndPl->showCmd,
	lpWndPl->ptMinPosition.x,lpWndPl->ptMinPosition.y,
	lpWndPl->ptMaxPosition.x,lpWndPl->ptMaxPosition.y,
	lpWndPl->rcNormalPosition.left,lpWndPl->rcNormalPosition.top,
	lpWndPl->rcNormalPosition.right,lpWndPl->rcNormalPosition.bottom));

    ASSERT_HWND(hWnd32, hWnd, FALSE);

    if (lpWndPl->flags & WPF_SETMINPOSITION)
	hWnd32->ptMinPosition = lpWndPl->ptMinPosition;

    hWnd32->ptMaxPosition = lpWndPl->ptMaxPosition;
    flags = SWP_NOZORDER|SWP_NOACTIVATE;
    if (((lpWndPl->rcNormalPosition.right - lpWndPl->rcNormalPosition.left) ==
	 (hWnd32->rcNormalPosition.right - hWnd32->rcNormalPosition.left)) &&
	((lpWndPl->rcNormalPosition.bottom - lpWndPl->rcNormalPosition.top) ==
	 (hWnd32->rcNormalPosition.bottom - hWnd32->rcNormalPosition.top)))
    {
	/*
	 *  Size hasn't changed, so don't resize on the SetWindowPos() call.
	 */
	flags |= SWP_NOSIZE;
    }

    if (!((hWnd32->dwStyle & WS_MINIMIZE)||(hWnd32->dwStyle & WS_MAXIMIZE)))
    {
        /*
	 *  Only call this if we are in normal display.
	 */
        SetWindowPos(hWnd, 0, lpWndPl->rcNormalPosition.left,
	    lpWndPl->rcNormalPosition.top,
	    lpWndPl->rcNormalPosition.right - lpWndPl->rcNormalPosition.left,
	    lpWndPl->rcNormalPosition.bottom - lpWndPl->rcNormalPosition.top,
	    flags);
    }
    hWnd32->rcNormalPosition = lpWndPl->rcNormalPosition;

    ShowWindow(hWnd,(int)lpWndPl->showCmd);

    RELEASEWININFO(hWnd32);
    return TRUE;
    /* TODO WPF_RESTORETOMAXIMIZED */
}

/**************************************************************/

/* Input group */

BOOL WINAPI
EnableWindow(HWND hWnd, BOOL bEnable)
{
    HWND hWndMenu;
    HWND32 hWnd32;
    BOOL bOldState;

    if (!(hWnd32 = GETHWND32(hWnd)))
	return FALSE;

    bOldState = (hWnd32->dwStyle & WS_DISABLED)?FALSE:TRUE;

    if (bEnable == bOldState)
    {
	RELEASEWININFO(hWnd32);
	return (bOldState)?FALSE:TRUE;
    }

    if (bEnable)
	hWnd32->dwStyle &= ~WS_DISABLED;
    else
	hWnd32->dwStyle |= WS_DISABLED;

    RELEASEWININFO(hWnd32);

    SendMessage(hWnd,WM_ENABLE,(WPARAM)bEnable,0L);

    if ((hWndMenu = GetWindowFrame(hWnd))) {
	if (!(hWnd32 = GETHWND32(hWndMenu)))
	    return TRUE;
	if (bEnable)
	    hWnd32->dwStyle &= ~WS_DISABLED;
	else
	    hWnd32->dwStyle |= WS_DISABLED;

	RELEASEWININFO(hWnd32);
    }
    return TRUE;
}

/*********************************************************************/

HWND WINAPI
GetLastActivePopup(HWND hWnd)
{
	/* return popup owned by hwnd most recently active */
	/* each popup tells parent when its activated      */
	/* if parent not top level or owned then ignore it */
	/* when parent is activated, then ignore last      */

	APISTR((LF_API,"GetLastActivePopup(hWnd:%x)\n",hWnd));
	return hWnd;
}


HWND WINAPI
GetActiveWindow(void)
{
    return hCurrentActiveWindow;
}

HWND WINAPI
SetActiveWindow(HWND hWnd)
{
    HWND32 hWnd32;
    HWND hWndChild;
    ATOM atmClassName;

    if (hWnd == 0) {
	hCurrentActiveWindow = (HWND)0;
	hActiveApp = (HTASK)0;
        return hWnd;
    }

    if (!(hWnd32 = GETHWND32(hWnd))) {
	ERRSTR((LF_ERROR,"***ERROR*** SetActiveWindow: bad hWnd %x\n",hWnd));
	return (HWND)0;
    }
    if (hWnd32->dwStyle & WS_CHILD)
    {
	RELEASEWININFO(hWnd32);
	return (HWND)0;
    }

    hWndChild = hWnd32->hWndChild;
    RELEASEWININFO(hWnd32);

    if (hWnd != GetActiveWindow()) {
	/* check if hWnd is a frame window */
	if ((atmClassName = GetClassWord(hWnd,GCW_ATOM)) ==
		atmGlobalLookup[LOOKUP_FRAME]) {
	    hWnd = hWndChild;
	    if (!(hWnd32 = GETHWND32(hWnd))) {
		ERRSTR((LF_ERROR,"SetActiveWindow: bad client for frame\n"));
		return (HWND)0;
	    }
	    RELEASEWININFO(hWnd32);

	    if (hWnd == GetActiveWindow())
		return hWnd;
	}

	return InternalSetActiveWindow(hWnd32);
    }
    else
	return hWnd;
}

HWND WINAPI
GetCapture(void)
{
    return hCurrentCapture;
}

#ifdef	TWIN32
BOOL WINAPI
#else
void WINAPI
#endif
ReleaseCapture(void)
{
    APISTR((LF_API,"ReleaseCapture\n"));

    if (hCurrentCapture) {
	DRVCALL_WINDOWS(PWSH_RELEASECAPTURE,0L,0L,0L);
	hCurrentCapture = 0;
    }
#ifdef	TWIN32
    return (TRUE);
#endif
}

HWND WINAPI
SetCapture(HWND hWnd)
{
    HWND32 hWnd32;
    HWND hWndTemp,hWndFrame;

    APISTR((LF_API,"SetCapture: hWnd %x\n",hWnd));

    if (!hWnd) {
	hWndTemp = hCurrentCapture;
	ReleaseCapture();
	return hWndTemp;
    }

    ASSERT_HWND(hWnd32,hWnd,0);
    RELEASEWININFO(hWnd32);

    if (hCurrentCapture)
	DRVCALL_WINDOWS(PWSH_RELEASECAPTURE,0L,0L,0L);

    hWndFrame = TWIN_GetTopLevelFrame(hWnd);

    DRVCALL_WINDOWS(PWSH_SETCAPTURE,0L,0L,
		WIN_GETDRVDATA(hWndFrame));

    hWndTemp = hCurrentCapture;
    hCurrentCapture = hWnd;
    return hWndTemp;
}

/*************************************************************/

/* Information group */

BOOL WINAPI
AnyPopup(void)
{
    return !EnumWindows((WNDENUMPROC)AnyPopupEnumProc, 0);
}

static BOOL
AnyPopupEnumProc(HWND hWnd, LPARAM lParam)
{
    HWND32 hWnd32;

    if (!(hWnd32 = GETHWND32(hWnd)))
	return TRUE;
    if ((hWnd32->dwStyle & WS_VISIBLE) &&
	!hWnd32->hWndOwner)
    {
	RELEASEWININFO(hWnd32);
	return FALSE;
    }
    else
    {
	RELEASEWININFO(hWnd32);
	return TRUE;
    }
}

typedef struct tagWPENUMSTRUCT
  {
	POINT ptTest;
	UINT  uiFlags;
	HWND  hWndMatch;
  } WPENUMSTRUCT;

#define	WPE_VISIBLEONLY		0x0001
#define	WPE_ENABLEDONLY		0x0002

HWND WINAPI
ChildWindowFromPoint(HWND hWndParent, POINT pt)
{
    POINT ptTest = pt;

    ClientToScreen(hWndParent,&ptTest);

    return TWIN_ChildWindowFromPointEx(hWndParent,ptTest,0);
}

/* this also takes screen coords */
HWND
TWIN_VisibleChildWindowFromPoint(HWND hWndParent, POINT pt)
{
    HWND hWndChild = hWndParent, hWnd = hWndParent;
    RECT rcClient;

    do {
	hWnd = hWndChild;
	if (0 ==(hWndChild =
		TWIN_ChildWindowFromPointEx(hWnd,pt,WPE_VISIBLEONLY)))
	    return (HWND)0;
	GetClientRect(hWndChild,&rcClient);
	MapWindowPoints(hWndChild,(HWND)0,(LPPOINT)&rcClient,2);
	if (!PtInRect(&rcClient,pt))
	    return hWndChild;
    } while (hWnd != hWndChild);
    return hWnd;
}

/* this routine, unlike ChildWindowFromPoint, takes screen coordinates */
static HWND
TWIN_ChildWindowFromPointEx(HWND hWndParent, POINT pt, UINT uiFlags)
{
    WPENUMSTRUCT cwpEnumStruct;
    RECT rcParent;

    APISTR((LF_API,"ChildWindowFromPoint(parent:%x,pt=%d,%d)\n",
		hWndParent, pt.x, pt.y));

    cwpEnumStruct.ptTest = pt;
    cwpEnumStruct.hWndMatch = 0;
    cwpEnumStruct.uiFlags = uiFlags;

    if (EnumAllWindows(EN_CHILD, hWndParent,
			(WNDENUMPROC)WindowFromPointEnumProc,
			(LPARAM)&cwpEnumStruct)) {
	GetWindowRect(hWndParent,&rcParent);
	if (PtInRect(&rcParent,pt)) {
	    return hWndParent;
	}
	else {
	    return 0;
	}
    }

    return cwpEnumStruct.hWndMatch;
}

HWND WINAPI
WindowFromPoint(POINT pt)
{
    WPENUMSTRUCT wpEnumStruct;
    HWND hWndReturn, hWndParent;

    wpEnumStruct.hWndMatch = 0;
    wpEnumStruct.ptTest = pt;
    wpEnumStruct.uiFlags = WPE_VISIBLEONLY | WPE_ENABLEDONLY;

    if (!EnumWindows((WNDENUMPROC)WindowFromPointEnumProc,
			(LPARAM)&wpEnumStruct)) {

	hWndParent = wpEnumStruct.hWndMatch;
	while ((hWndReturn = TWIN_ChildWindowFromPointEx(hWndParent,pt,
				WPE_VISIBLEONLY | WPE_ENABLEDONLY))
			!= hWndParent)
	    hWndParent = hWndReturn;

        APISTR((LF_API,"WindowFromPoint(pt:%d,%d) returns %x\n",
		pt.x,pt.y,hWndReturn));
	return hWndReturn;
    }

    return 0;
}

static BOOL
WindowFromPointEnumProc(HWND hWnd, LPARAM lParam)
{
    WPENUMSTRUCT *lpwpEnumStruct;
    HWND32 hWnd32;
    RECT rc;

    lpwpEnumStruct = (WPENUMSTRUCT *)lParam;
    GetWindowRect(hWnd,&rc);
    hWnd32 = GETHWND32(hWnd);
    /* if a window does not meet the criteria, or does not contain
	the point in question, continue */
    if (((lpwpEnumStruct->uiFlags & WPE_VISIBLEONLY) &&
	 !(hWnd32->dwStyle & WS_VISIBLE)) ||
	((lpwpEnumStruct->uiFlags & WPE_ENABLEDONLY) &&
	 (hWnd32->dwStyle & WS_DISABLED)) ||
	!PtInRect(&rc,lpwpEnumStruct->ptTest))
    {
	RELEASEWININFO(hWnd32);
	return TRUE;
    }
    else {
	lpwpEnumStruct->hWndMatch = hWnd;
	RELEASEWININFO(hWnd32);
	return FALSE;
    }
}

/* (WIN32) ChildWindowFromPointEx ****************************************** */

HWND WINAPI
ChildWindowFromPointEx(HWND hwndParent,
	POINT pt,
	UINT uFlags)
{
	UINT flags = 0;

	/* convert WIN32 flags to TWIN flags */
	if (uFlags == CWP_ALL)
		flags = 0;
	if (uFlags & CWP_SKIPINVISIBLE)
		flags |= WPE_VISIBLEONLY;
	if (uFlags & CWP_SKIPDISABLED)
		flags |= WPE_ENABLEDONLY;
#ifdef LATER
	/* **** must extend TWIN_ChildWindowFromPointEx to support this *** */
	if (uFlags & CWP_SKIPTRANSPARENT)
		flags |= CWP_SKIPTRANSPARENT;
#endif

	return TWIN_ChildWindowFromPointEx(hwndParent, pt, flags);

}

/* (WIN32) END ************************************************************* */

BOOL WINAPI
EnumChildWindows(HWND hWndParent, WNDENUMPROC lpProc, LONG lParam)
{
    return EnumAllWindows(EN_ALL, (HANDLE)hWndParent, lpProc,
		(LPARAM)lParam);
}

BOOL WINAPI
EnumTaskWindows(HTASK hTask, WNDENUMPROC lpEnumFunc, LPARAM lParam)
{
    return EnumAllWindows(EN_TASK, (HANDLE)hTask, lpEnumFunc,
		(LPARAM)lParam);
}

BOOL WINAPI
EnumWindows(WNDENUMPROC lpProc,LONG lParam)
{
    return EnumAllWindows(EN_CHILD, (HANDLE)hWndRoot, lpProc,
		(LPARAM)lParam);
}

static BOOL
EnumAllWindows(WORD wFunc, HANDLE hHndl,
		WNDENUMPROC lpEnumFunc, LPARAM lParam)
{
    WNDENUMPROC lpCallBack;
    HWND hWndStart;
    WORD wFlag;

    if (!GetDesktopWindow())
	return FALSE;

    lpCallBack = (WNDENUMPROC)lpEnumFunc;

    switch(wFunc) {
	case EN_CHILD:
	    hWndStart = GetTopWindow((HWND)hHndl);
	    wFlag = GW_HWNDNEXTSIB;
	    break;
	case EN_TASK:
	    hWndStart = GetTopWindow(hWndRoot);
	    wFlag = GW_HWNDNEXTSIB;
	    break;
	case EN_ALL:
	    hWndStart = GetTopWindow((HWND)hHndl);
	    wFlag = GW_HWNDNEXTTREE;
	    break;
	default:
	    return FALSE;
    }

    while (hWndStart) {
	if (wFunc == EN_TASK) {
	    if (hHndl != GetWindowTask(hWndStart)) {
		hWndStart = GetWindow(hWndStart, wFlag);
		continue;
	    }
	}
	if (!lpCallBack(hWndStart, lParam)) {
	    FreeProcInstance((FARPROC)lpEnumFunc);
	    return FALSE;
	}
	/* now if window is still around, continue, else
	   start all over again */
	if (IsWindow(hWndStart)) {
	    hWndStart = GetWindow(hWndStart, wFlag);
	}
	else {
	    switch (wFunc) {
		case EN_CHILD:
		case EN_ALL:
		    hWndStart = GetTopWindow((HWND)hHndl);
		    break;
		case EN_TASK:
		    hWndStart = GetTopWindow(hWndRoot);
		    break;
	    }
	}
    }
    return TRUE;
}

HWND
TWIN_FindAnotherOverlapped(HWND hWnd)
{
    HTASK hTask;
    HWND hWndDesktop,hWndTmp,hWndTop;
    DWORD dwStyle;

    if (!IsWindow(hWnd))
	return (HWND)0;

    hTask = GetWindowTask(hWnd);

    hWndDesktop = GetDesktopWindow();
    for (hWndTmp = GetWindow(hWndDesktop,GW_CHILD),hWndTop = 0;
	 hWndTmp;
	 hWndTmp = GetWindow(hWndTmp,GW_HWNDNEXTSIB)) {
	dwStyle = GetWindowStyle(hWndTmp);
	if (dwStyle & WS_POPUP)
	    continue;
	if ((hTask == GetWindowTask(hWndTmp)) && (hWndTmp != hWnd))
	    return hWndTmp;
	if (hWndTop == 0)
	    hWndTop = hWndTmp;
    }
    return hWndTop;
}

HWND WINAPI
GetTopWindow(HWND hWnd)
{
    return GetWindow((hWnd)?hWnd:GetDesktopWindow(),GW_CHILD);
}

HWND WINAPI
GetNextWindow(HWND hWnd, UINT uFlag)
{
    return GetWindow(hWnd,uFlag);
}

HWND WINAPI
GetDesktopWindow(void)
{
    return hWndRoot;
}

HWND WINAPI
FindWindow(LPCSTR lpszClass, LPCSTR lpszWindow)
{
	return FindWindowEx((HWND)NULL, (HWND)NULL, lpszClass, lpszWindow);
}

/* (WIN32) FindWindowEx **************************************************** */

HWND WINAPI
FindWindowEx(HWND hwndParent, HWND hwndChildAfter,
	LPCTSTR lpszClass, LPCTSTR lpszWindow)
{
    BOOL bFoundChildAfter;
    HWND32 hWnd32;
    HWND hTemp;
    char lpClassName[40];

    if (hwndParent == (HWND)NULL)
	hwndParent = GetDesktopWindow();

    bFoundChildAfter = (hwndChildAfter == (HWND)NULL) ? TRUE : FALSE;

    for (hTemp = GetWindow(hwndParent, GW_CHILD);
	 hTemp;
	 hTemp = GetWindow(hTemp,GW_HWNDNEXTSIB)) {
	if (!bFoundChildAfter) {
		if (hTemp == hwndChildAfter)
			bFoundChildAfter = TRUE;
		continue;
	}
	if (!hTemp)
	    return (HWND)0;
	GetClassName(hTemp,lpClassName,40);
#ifdef	LATER
	this can be an atom
#endif
	if (!lpszClass || !lstrcmpi(lpszClass,lpClassName)) {
	    hWnd32 = GETHWND32(hTemp);
	    if (!lpszWindow || !lstrcmpi(lpszWindow,
					hWnd32->lpWindowName))
	    {
		RELEASEWININFO(hWnd32);
		return hTemp;
	    }
	    RELEASEWININFO(hWnd32);
	}
    }
    return (HWND)0;
}

/* (WIN32) END ************************************************************* */

HWND WINAPI
GetWindow(HWND hWnd, UINT wCmd)
{
    HWND32 hWnd32;
    HWND hWndTemp, hWndParent;
    HWND rv = 0;

    APISTR((LF_APICALL, "GetWindow(HWND=%x,UINT=%x)\n",hWnd,wCmd));

    if (!(hWnd32 = GETHWND32(hWnd))) {
	APISTR((LF_APIFAIL,"GetWindow: returns HWND 0\n"));
	return(0L);
    }

    switch(wCmd) {
	case GW_CHILD:
	    rv = hWnd32->hWndChild;
	    RELEASEWININFO(hWnd32);
	    APISTR((LF_APIRET,"GetWindow: returns HWND %x\n",rv));
	    return rv;

	case GW_HWNDFIRST:
	    hWndParent = (hWnd32->dwStyle & WS_CHILD)?
			hWnd32->hWndParent:GetDesktopWindow();
	    RELEASEWININFO(hWnd32);
	    if (!(hWnd32 = GETHWND32(hWndParent))) {
		APISTR((LF_APIFAIL,"GetWindow: returns HWND 0\n"));
		return(0);
	    }
	    rv = hWnd32->hWndChild;
	    RELEASEWININFO(hWnd32);
	    APISTR((LF_APIRET,"GetWindow: returns HWND %x\n",rv));
	    return rv;
	case GW_HWNDLAST:
	    rv = TWIN_EnumerateSiblingWindows(GETHWND16(hWnd32),
					      ChangeZOrderFindBottom,
					      NULL);
	    RELEASEWININFO(hWnd32);
	    APISTR((LF_APIRET,"GetWindow: returns HWND %x\n",rv));
	    return rv;
	case GW_HWNDNEXTSIB:
	    rv = hWnd32->hWndSibling;
	    RELEASEWININFO(hWnd32);
	    APISTR((LF_APIRET,"GetWindow: returns HWND %x\n",rv));
	    return rv;
	case GW_HWNDNEXTGROUP:
	    rv = hWnd32->hWndGroup;
	    RELEASEWININFO(hWnd32);
	    APISTR((LF_APIRET,"GetWindow: returns HWND %x\n",rv));
	    return rv;
	case GW_HWNDPREVGROUP:
	    if (hWnd32->hWndGroup == hWnd)
	    {
		RELEASEWININFO(hWnd32);
	        APISTR((LF_APIRET,"GetWindow: returns HWND %x\n",rv));
		return hWnd;
	    }

	    hWndTemp = hWnd32->hWndGroup;
	    RELEASEWININFO(hWnd32);
	    hWnd32 = GETHWND32(hWndTemp);
	    while(hWnd32->hWndGroup != hWnd)
	    {
		HWND32 hWndNext32 = GETHWND32(hWnd32->hWndGroup);
		RELEASEWININFO(hWnd32);
		hWnd32 = hWndNext32;
	    }
	    rv = GETHWND16(hWnd32);
	    RELEASEWININFO(hWnd32);
	    APISTR((LF_APIRET,"GetWindow: returns HWND %x\n",rv));
	    return rv;
	case GW_HWNDPREVTREE:
	case GW_HWNDPREVSIB:
	    hWndParent = (hWnd32->dwStyle & WS_CHILD)?
			hWnd32->hWndParent:GetDesktopWindow();
	    RELEASEWININFO(hWnd32);
	    if (!(hWnd32 = GETHWND32(hWndParent))) {
		APISTR((LF_APIFAIL,"GetWindow: returns HWND 0\n"));
		return(0);
	    }
	    if (hWnd32->hWndChild == hWnd)
	    {
		RELEASEWININFO(hWnd32);
		rv = (wCmd == GW_HWNDPREVTREE)?hWndParent:(HWND)0;
	        APISTR((LF_APIRET,"GetWindow: returns HWND %x\n",rv));
		return rv;
	    }
	    else {
		hWndTemp = TWIN_EnumerateSiblingWindows(hWnd32->hWndChild,
					      ChangeZOrderFindPreviousSibling,
					      (void *)(LONG) hWnd);
		RELEASEWININFO(hWnd32);
		hWnd32 = GETHWND32(hWndTemp);
		rv = 0;
		if (hWnd32->hWndSibling != hWnd)
		{
		    /* this means we cannot find a guy */
		    RELEASEWININFO(hWnd32);
		    rv = (wCmd == GW_HWNDPREVTREE)?hWndParent:(HWND)0;
		}
		else
		{
		    rv = GETHWND16(hWnd32);
		    RELEASEWININFO(hWnd32);
		}

	        APISTR((LF_APIRET,"GetWindow: returns HWND %x\n",rv));
		return rv;
	    }
	case GW_OWNER:
	    rv = hWnd32->hWndOwner;
	    RELEASEWININFO(hWnd32);
	    APISTR((LF_APIRET,"GetWindow: returns HWND %x\n",rv));
	    return rv;

	case GW_HWNDNEXTTREE:
	    if (hWnd32->hWndChild)
	    {
		rv = hWnd32->hWndChild;
		RELEASEWININFO(hWnd32);
	        APISTR((LF_APIRET,"GetWindow: returns HWND %x\n",rv));
		return rv;
	    }
	    if (hWnd32->hWndSibling)
	    {
		rv = hWnd32->hWndSibling;
		RELEASEWININFO(hWnd32);
	        APISTR((LF_APIRET,"GetWindow: returns HWND %x\n",rv));
		return rv;
	    }

	    hWndParent = (hWnd32->dwStyle & WS_CHILD)?
			hWnd32->hWndParent:GetDesktopWindow();
	    for (hWndTemp = hWndParent; hWndTemp;
			hWndTemp = GetParent(hWndTemp)) {
		RELEASEWININFO(hWnd32);
		if (!(hWnd32 = GETHWND32(hWndTemp))) {
		    APISTR((LF_APIFAIL,"GetWindow: returns HWND 0\n"));
		    return (HWND)0;
		}
		if (hWnd32->hWndSibling)
		{
		    rv = hWnd32->hWndSibling;
		    RELEASEWININFO(hWnd32);
	    	    APISTR((LF_APIRET,"GetWindow: returns HWND %x\n",rv));
		    return rv;
		}
	    }
	    RELEASEWININFO(hWnd32);
	    APISTR((LF_APIRET,"GetWindow: returns HWND %x\n",0));
	    return (HWND)0;

	default:
	    RELEASEWININFO(hWnd32);
	    APISTR((LF_APIFAIL,"GetWindow: returns HWND 0\n"));
	    return(0);
    }
}

HTASK WINAPI
GetWindowTask(HWND hWnd)
{
    HWND32 hWnd32;
    HTASK rv;

    APISTR((LF_APICALL,"GetWindowTask(hWnd=%x)\n",hWnd));

    if(!IsWindow(hWnd)) {
    	APISTR((LF_APIFAIL,"GetWindowTask returns HTASK %x\n",0));
	return 0;
    }

    ASSERT_HWND(hWnd32,hWnd,0);

    if (hWnd32->hTask)
	rv = hWnd32->hTask;
    else
	rv = GetTaskFromInstance(hWnd32->hInstance);

    RELEASEWININFO(hWnd32);
    APISTR((LF_APIRET,"GetWindowTask returns HTASK %x\n",rv));
    return rv;
}

/*************************************************************/

/* Scrolling group */

void WINAPI
ShowScrollBar(HWND hWnd, int fnBar, BOOL fShow)
{
    HWND32 hWnd32,hHScroll32,hVScroll32;
    RECT rc;
    BOOL bWidthChanged,bHeightChanged;
    BOOL bPaintHorz,bPaintVert;
    int nWindowWidth, nWindowHeight;
    HRGN hRgn = 0;

    if (!(hWnd32 = GETHWND32(hWnd)))
        return;

    if (fnBar == SB_CTL) {
	ShowWindow(hWnd,(fShow)?SW_SHOW:SW_HIDE);
	RELEASEWININFO(hWnd32);
	return;
    }

    bWidthChanged = bHeightChanged = FALSE;
    bPaintHorz = bPaintVert = FALSE;
    if (fnBar != SB_VERT) {
	if (!hWnd32->hWndHZScroll) {
	    if (!fShow || !CreateSystemScrollbars(hWnd,SB_HORZ)) {
		if (fnBar == SB_HORZ)
		{		    
		    RELEASEWININFO(hWnd32);
		    return;
		}
		else
		    fnBar = SB_VERT;
	    }
	}
	if (hWnd32->hWndHZScroll && IsWindow(hWnd32->hWndHZScroll)) {
	    if (fShow && !(hWnd32->dwStyle & WS_HSCROLL)) {
		hWnd32->dwStyle |= WS_HSCROLL;
		bPaintHorz = TRUE;
		bHeightChanged = TRUE;
	    }
	    if (!fShow && (hWnd32->dwStyle & WS_HSCROLL)) {
		SendMessage(hWnd32->hWndHZScroll,WM_CANCELMODE,0,0L);
		hHScroll32 = GETHWND32(hWnd32->hWndHZScroll);
		hHScroll32->dwStyle &= ~WS_VISIBLE;
		RELEASEWININFO(hHScroll32);
		hWnd32->dwStyle &= ~WS_HSCROLL;
		bHeightChanged = TRUE;
	    }
	}
    }
    if (fnBar != SB_HORZ) {
	if (!hWnd32->hWndVTScroll) {
	    if (!fShow || !CreateSystemScrollbars(hWnd,SB_VERT))
	    {
		RELEASEWININFO(hWnd32);
		return;
	    }	    
	}
        if (IsWindow(hWnd32->hWndVTScroll)) {
	    if (fShow && !(hWnd32->dwStyle & WS_VSCROLL)) {
		hWnd32->dwStyle |= WS_VSCROLL;
		bPaintVert = TRUE;
		bWidthChanged = TRUE;
	    }
	    if (!fShow && (hWnd32->dwStyle & WS_VSCROLL)) {
		SendMessage(hWnd32->hWndVTScroll,WM_CANCELMODE,0,0L);
		hVScroll32 = GETHWND32(hWnd32->hWndVTScroll);
		hVScroll32->dwStyle &= ~WS_VISIBLE;
		RELEASEWININFO(hVScroll32);
		hWnd32->dwStyle &= ~WS_VSCROLL;
		bWidthChanged = TRUE;
	    }
	}
    }
    if (bWidthChanged || bHeightChanged) {
	RECT rcOldNC = hWnd32->rcNC;

        CalcNCDimensions(&hWnd32->rcNC,hWnd32->dwStyle,
		    hWnd32->dwExStyle,
		    hWnd32->wMenuHeight);

	nWindowWidth = hWnd32->rWnd.right-hWnd32->rWnd.left;
	nWindowHeight = hWnd32->rWnd.bottom-hWnd32->rWnd.top;
	SetRect(&rc,0,0,
		    nWindowWidth - hWnd32->rcNC.right - hWnd32->rcNC.left,
		    nWindowHeight - hWnd32->rcNC.top - hWnd32->rcNC.bottom);

	hWnd32->wWidth = (WORD)rc.right;
	hWnd32->wHeight = (WORD)rc.bottom;
	SendMessage(hWnd,WM_SIZE,(WPARAM)SIZENORMAL,
		    MAKELPARAM(rc.right,rc.bottom));

	/* window size might have changed while processing WM_SIZE */
	nWindowWidth = hWnd32->rWnd.right-hWnd32->rWnd.left;
	nWindowHeight = hWnd32->rWnd.bottom-hWnd32->rWnd.top;

	/* invalidate the old NC area */
	if ((bWidthChanged && (hWnd32->rcNC.right < rcOldNC.right))) {
	    SetRect(&rc,nWindowWidth-rcOldNC.right,
			    hWnd32->rcNC.top,
			    nWindowWidth-hWnd32->rcNC.right,
			    nWindowHeight-hWnd32->rcNC.bottom);
	    hRgn = CreateRectRgnIndirect(&rc);
	}
	if (bHeightChanged && (hWnd32->rcNC.bottom < rcOldNC.bottom)) {
	    SetRect(&rc,hWnd32->rcNC.left,
			    nWindowHeight-rcOldNC.bottom,
			    nWindowWidth-hWnd32->rcNC.right,
			    nWindowHeight-hWnd32->rcNC.bottom);
	    if (hRgn == 0)
		hRgn = CreateRectRgnIndirect(&rc);
	    else
		UnionRectWithRegion(hRgn,&rc);
	}
	if (hRgn) {
	    TWIN_ExposeChildren(hWnd32,hRgn);
	    DeleteObject(hRgn);
	}

	if ((bWidthChanged && (hWnd32->dwStyle & WS_HSCROLL)) || bPaintHorz) {
	    CalcSysScrollLocation(hWnd32,
			nWindowWidth,
			nWindowHeight,
			&rc,
			FALSE);

	    if (rc.right && rc.bottom) {
		hHScroll32 = GETHWND32(hWnd32->hWndHZScroll);
		hHScroll32->wWidth = (WORD)rc.right;
		hHScroll32->wHeight = (WORD)rc.bottom;
		SetRect(&hHScroll32->rWnd,rc.left,rc.top,
				rc.left+rc.right,rc.top+rc.bottom);
		SendMessage(hWnd32->hWndHZScroll, WM_SIZE, SIZENORMAL,
					MAKELPARAM(rc.right,rc.bottom));
		if (hWnd32->dwStyle & WS_HSCROLL)
		    bPaintHorz = TRUE;
		RELEASEWININFO(hHScroll32);
	    }
	}

	if ((bHeightChanged && (hWnd32->dwStyle & WS_VSCROLL)) || bPaintVert) {
	    CalcSysScrollLocation(hWnd32,
			nWindowWidth,
			nWindowHeight,
			&rc,
			TRUE);

	    if (rc.right && rc.bottom) {
		hVScroll32 = GETHWND32(hWnd32->hWndVTScroll);
		hVScroll32->wWidth = (WORD)rc.right;
		hVScroll32->wHeight = (WORD)rc.bottom;
		SetRect(&hVScroll32->rWnd,rc.left,rc.top,
				rc.left+rc.right,rc.top+rc.bottom);
		SendMessage(hWnd32->hWndVTScroll, WM_SIZE, SIZENORMAL,
					MAKELPARAM(rc.right,rc.bottom));
		if (hWnd32->dwStyle & WS_VSCROLL)
		    bPaintVert = TRUE;
		RELEASEWININFO(hVScroll32);
	    }
	}
	if (bPaintHorz) {
	    hHScroll32 = GETHWND32(hWnd32->hWndHZScroll);
	    hHScroll32->dwStyle |= WS_VISIBLE;
	    RELEASEWININFO(hHScroll32);
	    SendMessage(hWnd32->hWndHZScroll,WM_PAINT,0,0);
	}
	if (bPaintVert) {
	    hVScroll32 = GETHWND32(hWnd32->hWndVTScroll);
	    hVScroll32->dwStyle |= WS_VISIBLE;
	    RELEASEWININFO(hVScroll32);
	    SendMessage(hWnd32->hWndVTScroll,WM_PAINT,0,0);
	}
	if (bPaintHorz && bPaintVert) 
	    DrawSizeBox(hWnd32);
    }

    RELEASEWININFO(hWnd32);
}

BOOL
TestWF(HWND hWnd, DWORD dwFlags)
{
    HWND32 hWnd32;
    BOOL rv;

    if ((hWnd32 = GETHWND32(hWnd)))
    {
	rv = (hWnd32->dwWinFlags & dwFlags);
	RELEASEWININFO(hWnd32);
	return rv;
    }

    return FALSE;
}

DWORD
GetWF(HWND hWnd)
{
    HWND32 hWnd32;
    DWORD rv;

    if ((hWnd32 = GETHWND32(hWnd))) {
	rv = (hWnd32->dwWinFlags);
	RELEASEWININFO(hWnd32);
	return rv;
    }

    return 0L;
}

void
ClearWF(HWND hWnd, DWORD dwClearMask)
{
    HWND32 hWnd32;

    if (hWnd && (hWnd32 = GETHWND32(hWnd)))
	hWnd32->dwWinFlags &= ~dwClearMask;

    RELEASEWININFO(hWnd32);
}

void
SetWF(HWND hWnd, DWORD dwSetMask)
{
    HWND32 hWnd32;

    if (hWnd && (hWnd32 = GETHWND32(hWnd)))
	hWnd32->dwWinFlags |= dwSetMask;

    RELEASEWININFO(hWnd32);
}

void
SetControlStyle(HWND hWnd, WORD wStyle)
{
    HWND32	hWnd32;

    if (hWnd && (hWnd32 = GETHWND32(hWnd))) {
	hWnd32->dwStyle &= 0xffff0000L;
	hWnd32->dwStyle |= (DWORD)wStyle;
    }

    RELEASEWININFO(hWnd32);
}

/***************************************************************************/

void
CalcClientRect(HWND hWnd, LPRECT lprcClient)
{
    HWND32 hWnd32;

    hWnd32 = GETHWND32(hWnd);

    lprcClient->bottom = (lprcClient->bottom-lprcClient->top);
    lprcClient->right = (lprcClient->right-lprcClient->left);
    lprcClient->top = lprcClient->left = 0;
    if (!(hWnd32->dwStyle & WS_MINIMIZE)) {
	lprcClient->bottom -=
		(hWnd32->rcNC.top+hWnd32->rcNC.bottom);
	lprcClient->right -=
		(hWnd32->rcNC.right+hWnd32->rcNC.left);
	if (lprcClient->bottom < 0)
	    lprcClient->bottom = 0;
	if (lprcClient->right < 0)
	    lprcClient->right = 0;
    }

    RELEASEWININFO(hWnd32);
}

void
ChangeNCDimensions(HWND hWnd, LPRECT lprcNCnew)
{
    HWND32 hWnd32;
    RECT rc;
    WPARAM fwSizeType;

    if (!(hWnd32 = GETHWND32(hWnd)))
	return;

#ifdef	LATER
	to be pure, we should forward WM_NCCALCSIZE here ...
#endif
    SetRect(&rc,
	lprcNCnew->left,
	lprcNCnew->top,
	hWnd32->rWnd.right - hWnd32->rWnd.left -
		(lprcNCnew->left + lprcNCnew->right),
	hWnd32->rWnd.bottom - hWnd32->rWnd.top -
		(lprcNCnew->top + lprcNCnew->bottom));

    hWnd32->wWidth = (WORD)rc.right;
    hWnd32->wHeight = (WORD)rc.bottom;

    CopyRect(&hWnd32->rcNC,lprcNCnew);

    fwSizeType = (hWnd32->dwStyle & WS_MINIMIZE)?SIZE_MINIMIZED:
		(hWnd32->dwStyle & WS_MAXIMIZE)?SIZE_MAXIMIZED:
			SIZE_RESTORED;
    SendMessage(hWnd, WM_SIZE, fwSizeType,
		(LPARAM)MAKELONG(hWnd32->wWidth,
			hWnd32->wHeight));
#ifdef	LATER
	Do I have to send WM_MOVE?
#endif

    RELEASEWININFO(hWnd32);
}

void
SetWindowMenuHeight(HWND hWnd, WORD wHeight)
{
    HWND32 hWnd32, hWndFrame32;
    RECT rcNCnew, rcClient;
    HDC hDC;
    HRGN hRgn = 0;
    int yDiff;

    if (!(hWnd32 = GETHWND32(hWnd)))
	return;
    if (hWnd32->wMenuHeight == wHeight)
    {
	RELEASEWININFO(hWnd32);
	return;
    }
    
    CalcNCDimensions(&rcNCnew,hWnd32->dwStyle,hWnd32->dwExStyle,
		wHeight);

    if ((yDiff = rcNCnew.top - hWnd32->rcNC.top) != 0) {
	GetClientRect(hWnd,&rcClient);
	hDC = GetDCEx(hWnd,(HRGN)0,DCX_PARENTCLIP|DCX_CACHE);
	if (yDiff < 0) {
	    hRgn = CreateRectRgn(0,0,0,0);
	    rcClient.top -= yDiff;
	}
	else {
	    hRgn = 0;
	    rcClient.bottom -= yDiff;
	}
	ScrollDC(hDC,0,yDiff,&rcClient,(LPRECT)NULL,hRgn,(LPRECT)NULL);
	ReleaseDC(hWnd,hDC);
    }
    hWndFrame32 = GETHWND32(hWnd32->hWndFrame);

    hWnd32->wMenuHeight = wHeight;
    hWndFrame32->wHeight = wHeight;
    ChangeNCDimensions(hWnd, &rcNCnew);
    if (hRgn) {
	OffsetRgn(hRgn,0,-yDiff);
	InvalidateRgn(hWnd,hRgn,TRUE);
	DeleteObject(hRgn);
    }
    RedrawWindow(hWnd,NULL,0,RDW_INVALIDATE|RDW_ERASE|RDW_FRAME);
    RELEASEWININFO(hWnd32);
}

void
GetNCRect(HWND hWnd, LPRECT lpRect)
{
    HWND32 hWnd32;

    hWnd32 = GETHWND32(hWnd);
    CopyRect(lpRect, &hWnd32->rcNC);
    RELEASEWININFO(hWnd32);
}

BOOL
NonEmptyNCRect(HWND hWnd)
{
    RECT rc;

    GetNCRect(hWnd, &rc);
    return (rc.left | rc.top | rc.right | rc.bottom);
}

HBRUSH
GetClassBackgroundBrush(HWND hWnd)
{
    HBRUSH hBrTemp;

    hBrTemp = GetClassBackBrush(hWnd);
    if ((DWORD)hBrTemp & OM_MASK)
	return hBrTemp;
    if (!hBrTemp || (hBrTemp > (HBRUSH)(COLOR_ENDCOLORS + 1)))
	return (HBRUSH)0;
    return GetSysColorBrush((int)hBrTemp - 1);
}

HBRUSH
GetControlBrush(HWND hWnd, HDC hDC, WORD wType)
{
    HWND hWndParent;
    HBRUSH hBrush;

    hWndParent = GetParent(hWnd);
    if (!hWndParent)
	hWndParent = hWnd;
    hBrush =  (HBRUSH)SendMessage(hWndParent,GET_WM_CTLCOLOR_MSG(wType),
		GET_WM_CTLCOLOR_MPS(hDC,hWnd,wType));
    if (hBrush == 0)
	hBrush = GetStockObject(LTGRAY_BRUSH);
    return hBrush;
}

void WINAPI
MapWindowPoints(HWND hWndFrom, HWND hWndTo,LPPOINT lpptClient, UINT cPoints)
{
    HWND32 hWnd32;
    HWND32 hWndParent32;
    POINT ptFrom,ptTo;
    UINT  count;
    int   skip_next_nc = 0;

    ptFrom.x = ptFrom.y = 0;
    if (hWndFrom && (hWnd32 = GETHWND32(hWndFrom))) {

	    while (hWnd32->dwStyle & WS_CHILD) {
		ptFrom.x += hWnd32->rWnd.left;
		ptFrom.y += hWnd32->rWnd.top;
		if (hWnd32->hWndFrame && !skip_next_nc) {
		    ptFrom.x += hWnd32->rcNC.left;
		    ptFrom.y += hWnd32->rcNC.top;
		}

		hWndParent32 = GETHWND32(hWnd32->hWndParent);

		/* 96/04/16 RJA - This hack is here because system scrollbars
		 *                use a funny rWnd that already includes the
		 *                parent's NC area.
		 */
		if (hWndParent32->hWndHZScroll == GETHWND16(hWnd32) ||
		    hWndParent32->hWndVTScroll == GETHWND16(hWnd32))
		{
		    skip_next_nc = 1;
		}
		
		RELEASEWININFO(hWnd32);
		hWnd32 = hWndParent32;
	    }
	    ptFrom.x += hWnd32->rWnd.left;
	    ptFrom.y += hWnd32->rWnd.top;
	    if (hWnd32->hWndFrame && !skip_next_nc) {
		ptFrom.x += hWnd32->rcNC.left;
		ptFrom.y += hWnd32->rcNC.top;
	    }

	    RELEASEWININFO(hWnd32);
    }

    skip_next_nc = 0;
    ptTo.x = ptTo.y = 0;
    if (hWndTo && (hWnd32 = GETHWND32(hWndTo))) {

	    while (hWnd32->dwStyle & WS_CHILD) {
		ptTo.x += hWnd32->rWnd.left;
		ptTo.y += hWnd32->rWnd.top;
		if (hWnd32->hWndFrame && !skip_next_nc) {
		    ptTo.x += hWnd32->rcNC.left;
		    ptTo.y += hWnd32->rcNC.top;
		}

		hWndParent32 = GETHWND32(hWnd32->hWndParent);

		/* 96/04/16 RJA - This hack is here because system scrollbars
		 *                use a funny rWnd that already includes the
		 *                parent's NC area.
		 */
		if (hWndParent32->hWndHZScroll == GETHWND16(hWnd32) ||
		    hWndParent32->hWndVTScroll == GETHWND16(hWnd32))
		{
		    skip_next_nc = 1;
		}
		
		RELEASEWININFO(hWnd32);
		hWnd32 = hWndParent32;
	    }
	    ptTo.x += hWnd32->rWnd.left;
	    ptTo.y += hWnd32->rWnd.top;
	    if (hWnd32->hWndFrame && !skip_next_nc) {
		ptTo.x += hWnd32->rcNC.left;
		ptTo.y += hWnd32->rcNC.top;
	    }

	    RELEASEWININFO(hWnd32);
    }

    /* ptFrom is the screen coordinate of the origin of hWndFrom */
    /* ptTo   is the screen coordinate of the origin of hWndTo   */
    /* if(hWndFrom) then we convert from client to screen        */
    /* if(hWndTo)   then we convert from screen to client        */
    /* if both are set, we convert client to screen to client    */

    for(count = 0; count < cPoints;count++) {
		if(hWndFrom) {
		    lpptClient->x += ptFrom.x;
		    lpptClient->y += ptFrom.y;
		}

		if(hWndTo) {
		    lpptClient->x -= ptTo.x;
		    lpptClient->y -= ptTo.y;
	        }
		lpptClient++;
    }
}

BOOL WINAPI
ClientToScreen(HWND hWnd, LPPOINT lppt)
{
    MapWindowPoints(hWnd, 0, lppt, 1);
    return (TRUE);
}

BOOL WINAPI
ScreenToClient(HWND hWnd, LPPOINT lppt)
{
    MapWindowPoints(0, hWnd, lppt, 1);
    return (TRUE);
}

BOOL WINAPI
FlashWindow(HWND hWnd, BOOL bInvert)
{
    static BOOL flash = FALSE;

    if (bInvert)
	flash = flash?FALSE:TRUE;
    else
	return FALSE;
    return flash;
}

void
RedrawIconTitle(HWND hWnd)
{
    HWND32 hWnd32;

    if (!(hWnd32 = GETHWND32(hWnd)))
	return;

    if (hWnd32->hWndIconTitle)
	DrawIconTitle(hWnd32->hWndIconTitle,DIT_RECALC|DIT_REDRAW);

    RELEASEWININFO(hWnd32);
}

void
ShowIconTitle(HWND hWnd, BOOL bShow)
{
    HWND32 hWnd32;
    HWND hWndIconTitle;
    RECT rc;

    if (!(hWnd32 = GETHWND32(hWnd)))
	return;
    if (!(hWndIconTitle = hWnd32->hWndIconTitle)) {
	if (!bShow)
	{
	    RELEASEWININFO(hWnd32);
	    return;
	}
	
	GetWindowRect(hWnd,&rc);
	if (!(hWndIconTitle = CreateWindowEx(
			WS_EX_NOPARENTNOTIFY,
			"ICONTITLE",
			NULL,
			WS_CHILD|WS_CLIPSIBLINGS,
			rc.left + rc.right/2,rc.bottom+1,5,5,
			GetParent(hWnd),0,
			GetWindowInstance(hWnd),
			NULL)))
	{
	    RELEASEWININFO(hWnd32);
	    return;
	}
	
	hWnd32->hWndIconTitle = hWndIconTitle;
	RELEASEWININFO(hWnd32);
	hWnd32 = GETHWND32(hWndIconTitle);
	hWnd32->hWndOwner = hWnd;
	SetWindowPos(hWndIconTitle,HWND_BOTTOM,0,0,0,0,
		SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
	ShowWindow(hWndIconTitle,SW_SHOWNA);
	UpdateWindow(hWndIconTitle);
    }
    else
	ShowWindow(hWndIconTitle,(bShow)?SW_SHOWNA:SW_HIDE);

    RELEASEWININFO(hWnd32);
}

static HWND
InternalSetActiveWindow(HWND32 hWnd32)
{
    HWND	hWinAct;
    WINDOWPOS	wp;
    ATOM atmClassName;

    hWinAct = GETHWND16(hWnd32);
    if (!(hWnd32->dwStyle & WS_CHILD)) {
	/* top-level activation */

	/* if we are not on the top, raise window without activation */
	if (hWinAct != GetWindow(GetDesktopWindow(),GW_CHILD)) {
	    wp.hwnd = hWinAct;
	    wp.hwndInsertAfter = HWND_TOP;
	    wp.x = wp.y = wp.cx = wp.cy = 0;
	    wp.flags = SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE;
	    InternalSetWindowPos(&wp,MATCH_CLIENT);
	}

	atmClassName = GetClassWord(hWinAct,GCW_ATOM);
	if ((atmClassName == atmGlobalLookup[LOOKUP_MENULBOX]) ||
	    (atmClassName == atmGlobalLookup[LOOKUP_COMBOLBOX]))
	    return GetActiveWindow();

	/* if we are not currently active, activate application */
	hWinAct = GetActiveWindow();
	if (hActiveApp != GetWindowTask(GETHWND16(hWnd32)))
	{
	    HWND32 hWndTemp32 = NULL;

	    if (hWinAct)
		hWndTemp32 = GETHWND32(hWinAct);
	    
	    ActivateApp(hWnd32, hWndTemp32);

	    RELEASEWININFO(hWndTemp32);
	}
	
	hCurrentActiveWindow = GETHWND16(hWnd32);
	if (IsWindow(hWinAct))
	    SendMessage(hWinAct, WM_ACTIVATE,
		GET_WM_ACTIVATE_MPS(FALSE,
				IsIconic(hWinAct),
				hCurrentActiveWindow));
	SendMessage(hCurrentActiveWindow, WM_ACTIVATE,
		GET_WM_ACTIVATE_MPS(WA_ACTIVE,
				IsIconic(hCurrentActiveWindow),
				hWinAct));
	hWinAct = GETHWND16(hWnd32);
	SendMessage(hWinAct, WM_NCACTIVATE, (WPARAM)TRUE, (LPARAM)0);
    }
    else {
	SendMessage(hWinAct, WM_CHILDACTIVATE, (WPARAM)0, (LPARAM)0);
	if (hWnd32->dwStyle & WS_MINIMIZE)
	    RedrawIconTitle(hWinAct);
    }
	
    return hWinAct;
}

static void
ActivateApp(HWND32 lpNewAct, HWND32 lpOldAct)
{
    HWND hWnd;
    HTASK hTask;

    if (lpOldAct) {
#ifdef	LATER
	DeactivateWindow(lpOldAct);
	DeactivateApp(lpOldAct);
	KillFocus();
#endif
    }
    hTask = lpNewAct->hTask;
    hActiveApp = hTask;
    for (hWnd = GetWindow(GetDesktopWindow(),GW_CHILD);
	 hWnd;
	 hWnd = GetWindow(hWnd,GW_HWNDNEXTSIB))
	SendMessage(hWnd, WM_ACTIVATEAPP, TRUE, (LPARAM)hTask);
}

HWND WINAPI
GetFocus(void)
{
    return TWIN_InternalFocus(IFC_GETFOCUS, 0, 0L);
}

HWND WINAPI
SetFocus(HWND hWnd)
{
    HWND hWndLastFocus,hWndAncestor;

    hWndAncestor = 0;
    /* verify that we have a valid window */
    if (IsWindow(hWnd) &&
	 (hWndAncestor = GetTopLevelAncestor(hWnd)) != GetActiveWindow())
	SetActiveWindow(hWndAncestor);

    if (hWnd == 0 || IsWindow(hWnd)) {
        hWndLastFocus = TWIN_InternalFocus(IFC_SETFOCUS, hWnd, 0L);
    }
    else
      hWndLastFocus = 0;

    /* send color palette messages */
    if (SendMessage(hWndAncestor, WM_QUERYNEWPALETTE, 0, 0) == TRUE)
    {
	SendMessage(HWND_TOPMOST, WM_PALETTECHANGED, hWndAncestor, 0);
    }

    return hWndLastFocus;
}

HWND
TWIN_InternalFocus(int Func, HWND hWnd, LONG bFlag)
{
    int		nstate;
    HWND	hLast;
    HWND 	hFocus = 0;
    HWND32 	hWnd32 = (HWND32) NULL;

    if(hCurrentFocusWindow && IsWindow(hCurrentFocusWindow))
	hLast = hCurrentFocusWindow;
    else {
	hLast = 0;
       	hCurrentFocusWindow = 0; 
    }

    if(Func == IFC_GETFOCUS)
	    return hCurrentFocusWindow;

    /* check for valid window handle */
    if (hWnd != 0) {        /* zero is valid as a special case... */
        if (IsWindow(hWnd)) {
	    if ((hWnd32 = GETHWND32(hWnd)) == 0) {
	        ERRSTR((LF_ERROR,
		    "***ERROR*** TWIN_InternalFocus: HM_GETOBJ failed\n"));
	        return (HWND) hCurrentFocusWindow;
	    }  
        } else
          return (HWND) hCurrentFocusWindow;
    }

    hFocus = hWnd;
    switch(Func) {
	case IFC_SETFOCUS:
	    hCurrentFocusWindow = hWnd;
	    if(hWnd)
		bFlag = bHaveFocus;
	    break;
        case IFC_DRIVERFOCUS:
	    hFocus = GetDialogFocus(GetTopLevelAncestor(hWnd));
	    break;
    }

    nstate = (bHaveFocus)?1:0;
    nstate |= (bFlag)?2:0;

    switch(nstate) {
	case 0:	/* didn't and don't have focus */
	    if (hWnd32 && Func == IFC_SETFOCUS)
		SetDialogFocus(GetTopLevelAncestor(hWnd),hFocus);
	    break;
	case 2: /* didn't and now do have focus */
	    if (hWnd32 && !(hWnd32->dwStyle & WS_DISABLED)) {
		if(hFocus) {
			hLast = GetDialogFocus(GetTopLevelAncestor(hFocus));
			SendMessage(hLast, WM_SETFOCUS, 0, 0L);
			hCurrentFocusWindow = hLast;
			bHaveFocus = TRUE;
#ifdef LATER
/* jco: Mac Menu Bar */
			TWIN_GetMenuBar(hLast, &nativeMenuBar, sizeof(nativeMenuBar));
			TWIN_SetNativeMenuBar(&nativeMenuBar);
/* end Mac Menu Bar */
#endif
		} else 
			bHaveFocus = FALSE;
	    }
	    break;
	case 1: /* did have and now don't have */
	    if (hWnd32 && !(hWnd32->dwStyle & WS_DISABLED)) 
		hLast = GetDialogFocus(GetTopLevelAncestor(hFocus));
	    SendMessage(hLast, WM_KILLFOCUS, 0, 0L);
	    hCurrentFocusWindow = 0;
	    /*  But a SetFocus of 0 call doesn't rob us totally of focus. */
	    if (Func == IFC_DRIVERFOCUS)
	      bHaveFocus = FALSE;
	    break;
	case 3: /* did have and still have */
	    if (hLast && (hLast != hFocus))
		SendMessage(hLast, WM_KILLFOCUS, (WPARAM)hWnd, 0L);
	    if (hWnd && (hLast != hWnd)) {
		SetDialogFocus(GetTopLevelAncestor(hWnd),hWnd);
		SendMessage(hWnd, WM_SETFOCUS, (WPARAM)hLast, 0L);
	    }
	    break;
    }

    if (hWnd32)
	RELEASEWININFO(hWnd32);
    return hLast;
}

#ifdef LATER
/* jco: Mac Menu Bar */
int
TWIN_SetNativeMenuBar(LPSTR lpsz)
{

	LPSTR stringPointer = lpsz;
	
	/* for now, call the driver directly */
	/* Next step is to add the appropriate stuff to Driver.h, etc */
	DrvSetNativeMenuBar(lpsz);
	/* for now, convert to a string and print in log */
	/* Next step is to call driver to install as menu bar */
	while(strlen(stringPointer)) {
		if(stringPointer[strlen(stringPointer) + 2] != 0)
			stringPointer[strlen(stringPointer) + 1] = ' ';
		stringPointer += strlen(stringPointer) + 1;
	}
}
#endif

HWND hWndSysModalWindow = (HWND)0;

HWND WINAPI
GetSysModalWindow()
{
    if (IsWindow(hWndSysModalWindow))
	return hWndSysModalWindow;
    return (HWND)0;
}

HWND WINAPI
SetSysModalWindow(HWND hWnd)
{
    HWND hWndOld = 0;

    if (hWndSysModalWindow && IsWindow(hWndSysModalWindow))
	hWndOld = hWndSysModalWindow;
    hWndSysModalWindow = hWnd;
    return hWndOld;
}

BOOL WINAPI
LockWindowUpdate(HWND hWndLock)
{

    return TRUE;
}

BOOL
TWIN_ForwardConfigure(HWND hWndFrame, LPVOID lpStruct)
{
    LPWINDOWPOS lpwp = (LPWINDOWPOS)lpStruct;

    return InternalSetWindowPos(lpwp, MATCH_FRAME);
}

LRESULT
TWIN_CancelMode(HWND hWnd)
{
    HWND32 hWnd32;

    ASSERT_HWND(hWnd32,hWnd,0);

    if (hWnd32->hMenu)
	SendMessage(hWnd32->hWndFrame,WM_CANCELMODE,0,0L);

    if (hWnd32->hWndHZScroll)
	SendMessage(hWnd32->hWndHZScroll,WM_CANCELMODE,0,0L);
    if (hWnd32->hWndVTScroll)
	SendMessage(hWnd32->hWndVTScroll,WM_CANCELMODE,0,0L);

    RELEASEWININFO(hWnd32);
    return 0L;
}

static HWND
TWIN_EnumerateSiblingWindows(HWND first_wnd, 
				  int (*func)(HWND32, void *), 
				  void *user_data)
{
    HWND last_wnd = 0;
    HWND32 hWnd32;
    HWND32 hNext32;
    int continue_flag = 1;
    
    for (hWnd32 = GETHWND32(first_wnd);
	 hWnd32 && continue_flag;
	 hWnd32 = hNext32)
    {
	last_wnd = GETHWND16(hWnd32);
	continue_flag = (*func)(hWnd32, user_data);

	if (hWnd32->hWndSibling)
	    hNext32 = GETHWND32(hWnd32->hWndSibling);
	else
	    hNext32 = NULL;
	
	RELEASEWININFO(hWnd32);
    }

    if (hNext32)
	RELEASEWININFO(hNext32);

    return last_wnd;
}

static int ChangeZOrderFindTop(HWND32 hWnd32, void *search_data)
{
    HWND32 hWndMoving32 = (HWND32) search_data;
    
    if ((hWnd32 != hWndMoving32) &&
	(hWnd32->dwStyle & WS_POPUP) &&
	(hWnd32->hWndOwner &&
	 ((GetTopLevelAncestor(hWnd32->hWndOwner) == GETHWND16(hWndMoving32))||
	  hWnd32->hWndOwner == hWndMoving32->hWndFrame)))
    {
	return 1;
    }
    else
	return 0;
}

static int ChangeZOrderFindBottom(HWND32 hWnd32, void *search_data)
{
    return 1;
}

static int ChangeZOrderFindPreviousSibling(HWND32 hWnd32, void *search_data)
{
    HWND hWndTarget = (HWND)LOWORD(search_data);
    
    if (hWnd32->hWndSibling == hWndTarget)
	return 0;
    else
	return 1;
}

static int ChangeZOrderInvalidate(HWND32 hWnd32, void *additional_data)
{
    struct TWIN_changezorderinvalidate_s *inv_data =
	(struct TWIN_changezorderinvalidate_s *) additional_data;
    
    OffsetRgn(inv_data->hrgn, 
	      inv_data->pt.x - hWnd32->rWnd.left,
	      inv_data->pt.y - hWnd32->rWnd.top);
    TWIN_ExposeChildren(hWnd32, inv_data->hrgn);
    inv_data->pt.x = hWnd32->rWnd.left;
    inv_data->pt.y = hWnd32->rWnd.top;

    return (hWnd32 != inv_data->hwnd32) ? 1 : 0;
}

/* (WIN32) foreground window *********************************************** */

HWND	WINAPI
GetForegroundWindow(VOID)
{
	APISTR((LF_API, "GetForegroundWindow: (API)\n"));
	return GetActiveWindow();
}

BOOL	WINAPI
SetForegroundWindow(HWND hWnd)
{
	APISTR((LF_API, "SetForegroundWindow: (API) hWnd %x\n", hWnd));
	SetActiveWindow(hWnd);
	return (TRUE);
}

/* (WIN32) IsWindowUnicode ************************************************* */

BOOL WINAPI
IsWindowUnicode(HWND hWnd)
{
	return (FALSE);
}

/* (WIN32) Window Position ************************************************* */

WORD
TWIN_DeferWindowPos(HWND hWnd,
	UINT uMode,
	CONST RECT *lpScreenRect,
	UINT uChildren,
	CONST HWND FAR *lpChildren,
	BOOL (*func)(UINT uMode, HWND hWndChild,
		WORD wChildID, WORD wTotalChildren,
		CONST RECT *lpClientRect, LPRECT lpChildRect))
{
	HDWP hdwp;
	HWND hWndChild;
	WORD wChildID, wTotalChildren;
	RECT rcClientRect, rcChildRect;
	int i, j;

	if ((hdwp = BeginDeferWindowPos(1)) == (HDWP)NULL)
		return (0);

	if (lpScreenRect == NULL)
	{
		/* Use the client area (in client coordinates) of the parent
		 * window.
		 */
		if (GetClientRect(hWnd ? hWnd : GetDesktopWindow(),
			&rcClientRect) == FALSE)
			return (0);
	}
	else
	{
		/* Convert the requested area (in screen coordinates) to
		 * client coordinates.
		 */
		rcClientRect = *lpScreenRect;
		if ((ScreenToClient(hWnd, (LPPOINT)&(rcClientRect.left))
			== FALSE)
		 || (ScreenToClient(hWnd, (LPPOINT)&(rcClientRect.right))
			== FALSE))
			return (0);
	}

	/* For i = 0, increment wTotalChildren for each child window.
	 * For i = 1, defer window position for each child window.
	 */
	wTotalChildren = 0;
	for (i = 0; i < 2; i++)
	{
		wChildID = 0;
		if (lpChildren != NULL)
		{
			/* If lpChildren is not NULL,
			 *	uChildren = number of child windows
			 *	lpChildren = array of child windows
			 */
			for (j = 0; j < uChildren; j++)
			{
				if ((hWndChild = lpChildren[j]) == (HWND)NULL)
					continue;
				if (GetParent(hWndChild) != hWnd)
					continue;
				if (func(uMode, hWndChild,
					wChildID, wTotalChildren,
					&rcClientRect, &rcChildRect) == FALSE)
					continue;
				switch (i)
				{
				case 0:
					wTotalChildren++;
					break;
				case 1:
					hdwp = DeferWindowPos(hdwp, hWndChild,
						(HWND)NULL,
						rcChildRect.left,
						rcChildRect.top,
						rcChildRect.right
							- rcChildRect.left,
						rcChildRect.bottom
							- rcChildRect.top,
						SWP_NOACTIVATE|SWP_NOZORDER);
					break;
				}
				wChildID++;
			}
		}
		else
		{
			/* If lpChildren is NULL, we must iterate through all
			 * child windows.
			 */
			for (hWndChild = GetTopWindow(hWnd);
			     hWndChild != (HWND)NULL;
			     hWndChild = GetWindow(hWndChild, GW_HWNDNEXT))
			{
				if (func(uMode, hWndChild,
					wChildID, wTotalChildren,
					&rcClientRect, &rcChildRect) == FALSE)
					continue;
				switch (i)
				{
				case 0:
					wTotalChildren++;
					break;
				case 1:
					hdwp = DeferWindowPos(hdwp, hWndChild,
						(HWND)NULL,
						rcChildRect.left,
						rcChildRect.top,
						rcChildRect.right
							- rcChildRect.left,
						rcChildRect.bottom
							- rcChildRect.top,
						SWP_NOACTIVATE|SWP_NOZORDER);
					break;
				}
				wChildID++;
			}
		}
	}

	if (EndDeferWindowPos(hdwp) == FALSE)
		return (0);

	return (wTotalChildren);

}

static BOOL cascade(UINT uMode, HWND hWndChild,
	WORD wChildID, WORD wTotalChildren,
	CONST RECT *lpClientRect, LPRECT lpChildRect)
{
	if ((uMode & MDITILE_SKIPDISABLED) && !IsWindowEnabled(hWndChild))
		return (FALSE);

	if (wTotalChildren > 0)
	{
		int nWidth = lpClientRect->right - lpClientRect->left,
		    nHeight = lpClientRect->bottom - lpClientRect->top,
		    nMinWidth = GetSystemMetrics(SM_CXMIN),
		    nMinHeight = GetSystemMetrics(SM_CYMIN);
		*lpChildRect = *lpClientRect;
		if ((3 * nWidth / 4 >= nMinWidth)
		 && (3 * nHeight / 4 >= nMinHeight))
		{
			wTotalChildren = min(wTotalChildren,
				1 + (nWidth/4) / GetSystemMetrics(SM_CYMENU));
			wTotalChildren = min(wTotalChildren,
				1 + (nHeight/4) / GetSystemMetrics(SM_CYMENU));
			wChildID %= wTotalChildren;
			lpChildRect->left += wChildID
				* GetSystemMetrics(SM_CYMENU);
			lpChildRect->top += wChildID
				* GetSystemMetrics(SM_CYMENU);
			lpChildRect->right -= (7 - wChildID)
				* GetSystemMetrics(SM_CYMENU);
			lpChildRect->bottom -= (7 - wChildID)
				* GetSystemMetrics(SM_CYMENU);
		}
	}

	return (TRUE);

}

static BOOL tile(UINT uMode, HWND hWndChild,
	WORD wChildID, WORD wTotalChildren,
	CONST RECT *lpClientRect, LPRECT lpChildRect)
{
	if ((uMode & MDITILE_SKIPDISABLED) && !IsWindowEnabled(hWndChild))
		return (FALSE);

	if (wTotalChildren > 0)
	{
		int nWidth = lpClientRect->right - lpClientRect->left,
		    nHeight = lpClientRect->bottom - lpClientRect->top,
		    nMinWidth = GetSystemMetrics(SM_CXMIN),
		    nMinHeight = GetSystemMetrics(SM_CYMIN);
		*lpChildRect = *lpClientRect;
		if ((nWidth >= nMinWidth) && (nHeight >= nMinHeight))
		{
			switch (uMode & ~MDITILE_SKIPDISABLED)
			{
			case MDITILE_HORIZONTAL:
				wChildID %= nWidth / nMinWidth;
				wTotalChildren = min(nWidth / nMinWidth,
					wTotalChildren);
				lpChildRect->left += wChildID
					* (nWidth - nMinWidth)
					/ wTotalChildren;
				lpChildRect->right = lpChildRect->left
					+ (nWidth - nMinWidth)
					/ wTotalChildren;
				break;
			case MDITILE_VERTICAL:
				wChildID %= nHeight / nMinHeight;
				wTotalChildren = min(nHeight / nMinHeight,
					wTotalChildren);
				lpChildRect->top += wChildID
					* (nHeight - nMinHeight)
					/ wTotalChildren;
				lpChildRect->bottom = lpChildRect->top
					+ (nHeight - nMinHeight)
					/ wTotalChildren;
				break;
			}
		}
	}

	return (TRUE);

}

WORD	WINAPI
CascadeWindows(HWND hWnd,
	UINT uCascadeMode,
	CONST RECT *lpScreenRect,
	UINT uChildren,
	CONST HWND FAR *lpChildren)
{
	return TWIN_DeferWindowPos(hWnd, uCascadeMode, lpScreenRect,
		uChildren, lpChildren, cascade);
}

WORD	WINAPI
TileWindows(HWND hWnd,
	UINT uTileMode,
	CONST RECT *lpScreenRect,
	UINT uChildren,
	CONST HWND FAR *lpChildren)
{
	return TWIN_DeferWindowPos(hWnd, uTileMode, lpScreenRect,
		uChildren, lpChildren, tile);
}

