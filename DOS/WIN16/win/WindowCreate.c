/*    
	WindowCreate.c	2.60
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
#include "GdiObjects.h"
#include "WinDefs.h"
#include "WinData.h"
#include "Driver.h"
#include "Log.h"
#include "Hook.h"
#include "WindowDC.h"
#include "Kernel.h"
#include "Classes.h"
#include "Messages.h"

/* Imported prototypes */
extern ATOM atmGlobalLookup[];
extern HWND TWIN_GetCursorWindow(void);
extern void TWIN_SetCursorWindow(HWND);
extern BOOL TWIN_InsertAsFirstSibling(HWND32);
extern BOOL TWIN_ReparentDC(HWND);
extern BOOL TWIN_GdiDestroyDC(HDC);

/* Prototypes for static internal routines */
static void SendDestroyMessages(HWND);
static void FreeWindow(HWND);
static HWND AddScrollbar(HWND32,BOOL);
static void FillIconData(LPWINCREATESTRUCT,HICON);
static void FillCursorData(LPWINCREATESTRUCT,HCURSOR);

HWND WINAPI
CreateWindow(LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle,
	     int X, int Y, int nWidth, int nHeight,
	     HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, 
	     LPVOID lpParam)
{
    HWND hWndTopLevelAncestor;
    DWORD dwExStyle = 0;
    ATOM atmClass;

    if ((dwStyle & WS_CHILD) && IsWindow(hWndParent)) {
	hWndTopLevelAncestor = GetTopLevelAncestor(hWndParent);
	atmClass = GetClassWord(hWndTopLevelAncestor,GCW_ATOM);
	if (atmClass == atmGlobalLookup[LOOKUP_DIALOGCLASS])
	    dwExStyle = WS_EX_NOPARENTNOTIFY;
    }
	
    return CreateWindowEx(dwExStyle,lpClassName,lpWindowName,dwStyle,X,Y,
		nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

static WNDCLASSEX FrameInfo,ScrollInfo;
static HCLASS32 hClassFrame32 = 0;
static HCLASS32 hClassScroll32 = 0;

HWND WINAPI
CreateWindowEx(DWORD dwExStyle, LPCSTR lpClass, 
		LPCSTR lpWindowName, DWORD dwStyle,
		int X, int Y, int nWidth, int nHeight, 
		HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, 
		LPVOID lpParam)
{
    HWND32 hWnd32, hFrame32, hWndParent32; 
    HCLASS32 hClass32;
    WNDCLASSEX ClassInfo;
    LPSTR lpExtraTemp;
    LPSTR lpClassName;
    HWND hWnd,hWndFrame;
    HWND hWndAncestor;
    int nDefMaxWidth, nDefMaxHeight;
    int nDefFrameWidth, nDefFrameHeight;
    int nWidthReq, nHeightReq;
    DWORD dwXYBorders,dwWinStyle;
    CREATESTRUCT csClient,csFrame;
    CBT_CREATEWND cbtcs;
    MINMAXINFO mmi;
    WINCREATESTRUCT wcs;
    DWORD dwDrvData;
    LPSTR lpClassMenu;
    BOOL bHScroll,bVScroll,bScrollResult;
    RECT rc;
    RECT rcExpectedNC;
    LRESULT lResult;
    ATOM atmClass;

    hInstance = GetInstanceFromModule((HMODULE)hInstance);

    /* Check if class has been registered */
    lpClassName = (LPSTR)lpClass;
    if (!(HIWORD((DWORD)lpClassName)))
	if ((DWORD)lpClassName == (DWORD)WC_DIALOG)
	    lpClassName = TWIN_DIALOGCLASS;

    if (!(hClass32=FindClass(lpClassName, hInstance))) {
	return 0;
    }

    /* Get class information */
    InternalGetClassInfoEx(hClass32, &ClassInfo);
    atmClass = (ATOM)GetClassHandleWord(hClass32,GCW_ATOM);

    /* Check if the frame class has been registered */
    if (!hClassFrame32) {
	if (!(hClassFrame32 = FindClass(TWIN_FRAMECLASS, (HINSTANCE)0))) {
	    RELEASECLASSINFO(hClass32);
	    return 0;
	}
        /* Get frame class information */
	InternalGetClassInfoEx(hClassFrame32, &FrameInfo);
    }

    /* Create client window handle */
    if (!(hWnd32 = CREATEHWIN(hWnd))) {
	RELEASECLASSINFO(hClass32);
	return 0;
    }

    /* Create frame window handle */
    if (!(hFrame32 = CREATEHWIN(hWndFrame))) {
	RELEASEWININFO(hWnd32);
	RELEASECLASSINFO(hClass32);
	FREEHWND(hWnd);
	return 0;
    }
    hWnd32->hWndFrame = hWndFrame;
    hFrame32->hWndChild = hWnd;

    /* Set window procedures */
    hWnd32->lpfnWndProc = ClassInfo.lpfnWndProc;
    hFrame32->lpfnWndProc = FrameInfo.lpfnWndProc;

    /* set style */
    /* WS_POPUP takes precedence over WS_CHILD. Only one of them can be set */
    if (dwStyle & WS_POPUP)
	dwStyle &= ~WS_CHILD;

    if (!(dwStyle & WS_CHILD)) {
	dwStyle &= ~WS_MINIMIZE;

	/* top-level window must have WS_CLIPSIBLINGS */
	dwStyle |= WS_CLIPSIBLINGS;

	/* non-popup top-level window must have caption */
	if (!(dwStyle & WS_POPUP))
	    dwStyle |= WS_CAPTION;
    }

    hWnd32->dwStyle = dwStyle & ~WS_VISIBLE;
    hWnd32->dwExStyle = dwExStyle;
    hFrame32->dwStyle = hWnd32->dwStyle;
    hFrame32->dwExStyle = dwExStyle;
    if (ClassInfo.style & CS_SAVEBITS)
	hFrame32->dwExStyle |= WS_EX_SAVEBITS;

    /* set client parent, owner, group link, wChildID, hMenu fields */
    hWnd32->hWndParent = (hWnd32->dwStyle & WS_CHILD)?hWndParent:(HWND)0;
    hWnd32->hWndOwner = (hWnd32->dwStyle & WS_CHILD)?(HWND)0:hWndParent;

    if (hWnd32->dwStyle & WS_CHILD) {
	hWnd32->wChildID = (WORD)hMenu;
	hWnd32->hMenu = 0;
    }
    else {
	hWnd32->wChildID = 0;
	if (!hMenu) {
		lpClassMenu = GetClassMenuName(hClass32);
	    if (lpClassMenu)
			hMenu = LoadMenu(hInstance,lpClassMenu);
	}
	hWnd32->hMenu = hMenu;
    }

    if (hWnd32->dwStyle & WS_SYSMENU)
	if ( hWnd32->dwStyle & WS_CHILD )
		hWnd32->hSysMenu = LoadMenu ( ( HINSTANCE )0, "CHILDSYSMENU" );
	else
		hWnd32->hSysMenu = LoadMenu((HINSTANCE)0,"SYSMENU");

    hFrame32->wChildID = (WORD)(hWnd32->hMenu);
    hFrame32->hWndParent = hWnd32->hWndParent;
    hFrame32->hWndOwner = hWnd;

    /* set instance handle */
    hWnd32->hInstance = hInstance;
    hFrame32->hInstance = hInstance;
    hWnd32->hTask = GetCurrentTask();
    hFrame32->hTask = GetCurrentTask();

    /* set class pointer */
    hWnd32->hWindowClass32 = hClass32;
    hFrame32->hWindowClass32 = hClassFrame32;

    hFrame32->dwWinFlags = hWnd32->dwWinFlags;

    /* allocate client extra bytes */
    if (ClassInfo.cbWndExtra) {
	if (!hWnd32->lpWndExtra) { /* this should always be zero */
	    if (!(lpExtraTemp = WinMalloc(ClassInfo.cbWndExtra))) {
		RELEASEWININFO(hWnd32);
		RELEASEWININFO(hFrame32);
		RELEASECLASSINFO(hClass32);
	        FREEHWND(hWnd);
	        FREEHWND(hWndFrame);
	        return (HWND)0;
	    }
	    memset(lpExtraTemp, '\0', ClassInfo.cbWndExtra);
	    hWnd32->cbWndExtra = ClassInfo.cbWndExtra;
	    hWnd32->lpWndExtra = lpExtraTemp;
	}
    }
    else {
	hWnd32->cbWndExtra = 0;
	hWnd32->lpWndExtra = NULL;
    }

    /* Allocate frame extra bytes */
    if (FrameInfo.cbWndExtra) {
	if (!(lpExtraTemp = WinMalloc(FrameInfo.cbWndExtra))) {
	    RELEASEWININFO(hWnd32);
	    RELEASEWININFO(hFrame32);
	    RELEASECLASSINFO(hClass32);
	    FREEHWND(hWnd);
	    FREEHWND(hWndFrame);
	    return (HWND)0;
	}
	memset(lpExtraTemp, '\0', FrameInfo.cbWndExtra);
	hFrame32->cbWndExtra = FrameInfo.cbWndExtra;
	hFrame32->lpWndExtra = lpExtraTemp;
    }
    else {
	hFrame32->cbWndExtra = 0;
	hFrame32->lpWndExtra = NULL;
    }

    /* Fill CREATESTRUCT for the hook */
    csClient.lpCreateParams = lpParam;
    csClient.hInstance = hInstance;
    csClient.hMenu = (hWnd32->dwStyle & WS_CHILD)?hWnd32->wChildID:hWnd32->hMenu;
    csClient.hwndParent = hWndParent;
    csClient.cy = nHeight;
    csClient.cx = nWidth;
    csClient.y = Y;
    csClient.x = X;
    /* dwStyle has original WS_VISIBLE still in it */
    csClient.style = dwStyle;
	/* can this be an atom? */
    csClient.lpszName = HIWORD(lpWindowName)?(LPSTR)lpWindowName:NULL;
    csClient.lpszClass = (atmClass == atmGlobalLookup[LOOKUP_DIALOGCLASS])?
			WC_DIALOG:
			(LPSTR) lpClass;
    csClient.dwExStyle = dwExStyle;

    cbtcs.lpcs = &csClient;
    cbtcs.hwndInsertAfter = (HWND)1;

    if (lpHookList[WH_CBT+1]) {
	if (lpHookList[WH_CBT+1]->lpfnHookProc(HCBT_CREATEWND,
		(WPARAM)hWnd, (LPARAM)&cbtcs)) { 
	    RELEASEWININFO(hWnd32);
	    RELEASEWININFO(hFrame32);
	    RELEASECLASSINFO(hClass32);
	    FREEHWND(hWnd);
	    FREEHWND(hWndFrame);
	    return (HWND)0;
	}
    }

    nDefMaxWidth = GetSystemMetrics(SM_CXSCREEN);
    nDefMaxHeight = GetSystemMetrics(SM_CYSCREEN);
    nDefFrameWidth = GetSystemMetrics(SM_CXFRAME);
    nDefFrameHeight = GetSystemMetrics(SM_CYFRAME);

    /* Send WM_GETMINMAXINFO, if necessary */
    if (hWnd32->dwStyle & (WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)) {
	mmi.ptReserved.x = mmi.ptReserved.y = 0;
	mmi.ptMaxSize.x = nDefMaxWidth + 2 * nDefFrameWidth;
	mmi.ptMaxSize.y = nDefMaxHeight + 2 * nDefFrameHeight;
	mmi.ptMaxPosition.x = -nDefFrameWidth;
	mmi.ptMaxPosition.y = -nDefFrameHeight;
	mmi.ptMinTrackSize.x = GetSystemMetrics(SM_CXMINTRACK);
	mmi.ptMinTrackSize.y = GetSystemMetrics(SM_CYMINTRACK);
	mmi.ptMaxTrackSize.x = nDefMaxWidth + 2 * nDefFrameWidth;
	mmi.ptMaxTrackSize.y = nDefMaxHeight + 2 * nDefFrameHeight;

	SendMessage(hWnd, WM_GETMINMAXINFO, 0, (LPARAM)&mmi);

	hWnd32->ptMaxSize = mmi.ptMaxSize;
	hWnd32->ptMaxPosition = mmi.ptMaxPosition;
	hWnd32->ptMinTrackSize = mmi.ptMinTrackSize;
	hWnd32->ptMaxTrackSize = mmi.ptMaxTrackSize;
    }

    /* set our maximums to 3/4 of screen for CW_USEDEFAULT cases */
    /* this should be put into twinrc later */
    /*  NOTE:  FIX ME!!  This thinking is broken; we should use some */
    /*   configurable scaling parameters in the twinrc */
    if (nDefMaxWidth > 780) {
	if ( nWidth == (int)CW_USEDEFAULT )
	    nDefMaxWidth = MulDiv(nDefMaxWidth, 3, 4);
	if ( nHeight == (int)CW_USEDEFAULT )
	    nDefMaxHeight = MulDiv(nDefMaxHeight, 3, 4);
    }

    /* calc window borders */
    dwXYBorders = CalcBorders(hWnd32->dwStyle,hWnd32->dwExStyle);
    hWnd32->wXBorder = hFrame32->wXBorder = LOWORD(dwXYBorders);
    hWnd32->wYBorder = hFrame32->wYBorder = HIWORD(dwXYBorders);

    /* Determine the final origin */
    if (X == (int)CW_USEDEFAULT) {
	if (hWnd32->dwStyle & (WS_CHILD|WS_POPUP)) {
	    hWnd32->rWnd.left = 0;
	    hWnd32->rWnd.top = 0;
	}
	else {
	    hWnd32->rWnd.left = hFrame32->rWnd.left = 10;
	    hWnd32->rWnd.top = hFrame32->rWnd.top = 10;
	}
    }
    else {
	    hWnd32->rWnd.left = hFrame32->rWnd.left = X;
	    hWnd32->rWnd.top = hFrame32->rWnd.top = Y;
    }

    if (!(hWnd32->dwStyle & WS_CHILD)) { 
	CalcExpectedNC(&rcExpectedNC,hWnd32->dwStyle,dwExStyle);
	hWnd32->rWnd.left += rcExpectedNC.left;
	hWnd32->rWnd.top += rcExpectedNC.top;
	hFrame32->rWnd.left = hWnd32->rWnd.left;
	hFrame32->rWnd.top = hWnd32->rWnd.top;
    }

    /* Fill CREATESTRUCT for the frame */
    csFrame.lpCreateParams = NULL;
    csFrame.hInstance = hFrame32->hInstance;
    csFrame.hMenu = hWnd32->hMenu;
    csFrame.hwndParent = hFrame32->hWndParent;
    csFrame.cy = 0;
    csFrame.cx = 0;
    csFrame.y = hFrame32->rWnd.top;
    csFrame.x = hFrame32->rWnd.left;
    csFrame.style = hFrame32->dwStyle;
    csFrame.lpszName = NULL;
    csFrame.lpszClass = TWIN_FRAMECLASS;
    csFrame.dwExStyle = hFrame32->dwExStyle;

    /* Send WM_NCCREATE to the frame */
    if (SendMessage(hWndFrame, WM_NCCREATE, 0, (LPARAM)&csFrame) == 0) {
	RELEASEWININFO(hWnd32);
	RELEASEWININFO(hFrame32);
	RELEASECLASSINFO(hClass32);
	FREEHWND(hWnd);
	FREEHWND(hWndFrame);
	return 0;
    }

    /* Insert client window to the winman list */
    InsertAsLastSibling(hWnd32);
    if (hWnd32->dwStyle & WS_CHILD)
    {
	if (!(hWndParent32 = GETHWND32(hWndParent))) {
	    RELEASEWININFO(hWnd32);
	    RELEASEWININFO(hFrame32);
	    RELEASECLASSINFO(hClass32);
	    FREEHWND(hWnd);
	    FREEHWND(hWndFrame);
	    return 0;
	}

	hWndParent32->hWndLastChild = hWnd;

	RELEASEWININFO(hWndParent32);
    }

    /* Calc NC rect for the client with default menu height */
    CalcNCDimensions(&hWnd32->rcNC,
		hWnd32->dwStyle,
		dwExStyle,(hWnd32->hMenu)?GetSystemMetrics(SM_CYMENU):0);

    /* Determine the final size */
    /* Child windows do not need any winman adjustments */
    if (hWnd32->dwStyle & WS_CHILD) {
	if (nWidth == (int)CW_USEDEFAULT) {
	    hWnd32->rWnd.right = 0;
	    hWnd32->rWnd.bottom = 0;
	}
	else {
	    if (nWidth != 0) {
		nWidth = max(nWidth,hWnd32->rcNC.left+hWnd32->rcNC.right+1);

/**********************************************************************/
/*	MS Windows does not impose the following limit.  So, I am     */
/*	the following code out.  We discovered this as a result of    */
/*      a dialog box with the WS_CHILD style being created wider than */
/*	it was supposed to be.					      */
#if THIS_IS_NOT_DEFINED
		if ((hWnd32->dwStyle & WS_CAPTION) == WS_CAPTION)
		    nWidth = max(nWidth,100);
#endif
/**********************************************************************/
	    }
	    if (nHeight != 0)
		nHeight = max(nHeight,hWnd32->rcNC.top+hWnd32->rcNC.bottom+1);
	    if (atmClass == atmGlobalLookup[LOOKUP_MDICLIENT]) {
		GetClientRect(hWndParent,&rc);
		nWidth = rc.right;
		nHeight = rc.bottom;
	    }
	    hWnd32->rWnd.right = hWnd32->rWnd.left + nWidth;
	    hWnd32->rWnd.bottom = hWnd32->rWnd.top + nHeight;
	}
    }
    else {
	if (nWidth == (int)CW_USEDEFAULT) {
	    if (hWnd32->dwStyle & WS_POPUP)
		nWidthReq = nHeightReq = 5;
	    else {
		nWidthReq = nDefMaxWidth -
				(rcExpectedNC.left + rcExpectedNC.right);
		nHeightReq = nDefMaxHeight -
				(rcExpectedNC.top + rcExpectedNC.bottom);
	    }
	}
	else {
	    nWidthReq = nWidth - (rcExpectedNC.left + rcExpectedNC.right);
	    nWidthReq = max(nWidthReq,1);
	    nHeightReq = nHeight - (rcExpectedNC.top + rcExpectedNC.bottom);
	    nHeightReq = max(nHeightReq,1);
	}
	if ((hWnd32->dwStyle & WS_CAPTION) == WS_CAPTION)
	    nWidthReq = max(nWidthReq,100);

	hWnd32->rWnd.right = hWnd32->rWnd.left +
			min(nWidthReq, nDefMaxWidth);
	hWnd32->rWnd.bottom = hWnd32->rWnd.top +
			min(nHeightReq, nDefMaxHeight);
    }

    /* Really measure window menu bar */
    hWnd32->wMenuHeight = 0;
    if (hWnd32->hMenu && !(hWnd32->dwStyle & WS_CHILD))
	hWnd32->wMenuHeight = MeasureWindowMenu(hWnd, hWndFrame);

    /* Fill width and height for the frame */
    hFrame32->rWnd.right = hWnd32->rWnd.right;
    hFrame32->rWnd.bottom = hWnd32->rWnd.bottom;
    csFrame.cy = hFrame32->rWnd.bottom-hFrame32->rWnd.top;
    csFrame.cx = hFrame32->rWnd.right-hFrame32->rWnd.left;

    csClient.lpszClass = (LPSTR) lpClassName;
    /* wChildID may have been changed by WM_MINMAXINFO */
    csClient.hMenu = (hWnd32->dwStyle & WS_CHILD)?
		hWnd32->wChildID:hWnd32->hMenu;

    if (!(hWnd32->dwStyle & WS_CHILD)) {
	/* Fill the WINCREATESTRUCT */
	wcs.lpszName = csClient.lpszName;
	wcs.dwStyle = hWnd32->dwStyle;
	/* this field may have WS_EX_SAVEBITS set */
	wcs.dwExStyle = csFrame.dwExStyle;
	wcs.nBorder = hWnd32->wXBorder;
	FillIconData(&wcs,ClassInfo.hIcon);
	FillCursorData(&wcs,ClassInfo.hCursor);

	wcs.hWndFrame = hWndFrame;
	wcs.hWndClient = hWnd;

	SetRect(&wcs.rcFrame,csFrame.x,csFrame.y,csFrame.cx,csFrame.cy);

	/* Create low-level windows */
	if (0 == (dwDrvData = DRVCALL_WINDOWS(PWSH_CREATEWINDOW,
		0,0,&wcs))) {
	    RELEASEWININFO(hWnd32);
	    RELEASEWININFO(hFrame32);
	    RELEASECLASSINFO(hClass32);
	    FREEHWND(hWnd);
	    FREEHWND(hWndFrame);
	    return (HWND)0;
	}
	SetWindowLong(hWndFrame,GWL_DRVDATA,dwDrvData);
    }

    SetWindowLong(hWnd,GWL_UPDATE,DRVCALL_REGIONS(PRH_CREATEREGION,0,0,0));

    /* Readjust client CREATESTRUCT dimensions for WM_NCCREATE */
    csClient.x = hWnd32->rWnd.left;
    csClient.y = hWnd32->rWnd.top;
    csClient.cy = csFrame.cy;
    csClient.cx = csFrame.cx;

    /* Create non-client area structs */
    if (!SendMessage(hWnd, WM_NCCREATE, 0, (LPARAM)&csClient)) {
	RELEASEWININFO(hWnd32);
	RELEASEWININFO(hFrame32);
	RELEASECLASSINFO(hClass32);
	FREEHWND(hWnd);
	FREEHWND(hWndFrame);
	return (HWND)0;
    }

    /* Calc NC rect for the client */
    CalcNCDimensions(&hWnd32->rcNC,
		hWnd32->dwStyle,dwExStyle,hWnd32->wMenuHeight);

    /* WS_?SCROLL styles may have been changed during WM_NCCREATE processing */
    bHScroll = (hWnd32->dwStyle & WS_HSCROLL)?TRUE:FALSE;
    bVScroll = (hWnd32->dwStyle & WS_VSCROLL)?TRUE:FALSE;

    if (bHScroll || bVScroll) {
	if (bHScroll && bVScroll)
	    bScrollResult = CreateSystemScrollbars(hWnd,SB_BOTH);
	else
	    bScrollResult = CreateSystemScrollbars(hWnd,
			(bHScroll)?SB_HORZ:SB_VERT);
	if (!bScrollResult) {
	    FREEHWND(hWnd);
	    FREEHWND(hWndFrame);
	}
    }

    /* Calculate client rect */
    CopyRect(&rc, &hWnd32->rWnd);
    SendMessage(hWnd, WM_NCCALCSIZE, 0, (LPARAM)&rc);
    hWnd32->wWidth = rc.right - rc.left;
    hWnd32->wHeight = rc.bottom - rc.top;

    /* Set frame's NC and client area */
    SetRectEmpty(&hFrame32->rcNC);
    hFrame32->wWidth = csFrame.cx;
    hFrame32->wHeight = hWnd32->wMenuHeight;

    RELEASEWININFO(hFrame32);

    /* Lock window classes */
    LockClass(hClass32,TRUE);
    LockClass(hClassFrame32,TRUE);

    RELEASECLASSINFO(hClass32);

    /* wChildID may have been changed by previous messages */
    csClient.hMenu = (hWnd32->dwStyle & WS_CHILD)?
		hWnd32->wChildID:hWnd32->hMenu;

    /* Send WM_CREATE */
    lResult = SendMessage(hWnd, WM_CREATE, 0, (LPARAM)&csClient);
    if (lResult == (LRESULT)-1) {
	RELEASEWININFO(hWnd32);
	return (HWND)0;
    }

    /* do minimize/maximize for invisible first */
    if (!(csClient.style & WS_VISIBLE)) {
	if (hWnd32->dwStyle & WS_CHILD) {	
					/* MinMaximize to handle top-level */
	    if (hWnd32->dwStyle & WS_MINIMIZE)	
		MinMaximize(hWnd,SW_MINIMIZE,TRUE);
	    else if (hWnd32->dwStyle & WS_MAXIMIZE)
		MinMaximize(hWnd,SW_MAXIMIZE,TRUE);
	}
    }

    /* Send PARENTNOTIFY, if appropriate */
    if ((hWnd32->dwStyle & WS_CHILD) &&
	!(hWnd32->dwExStyle & WS_EX_NOPARENTNOTIFY)) {
	hWndAncestor = hWnd;
	do {
	    hWndAncestor = GetParent(hWndAncestor);
	    dwWinStyle = GetWindowStyle(hWndAncestor);
	    SendMessage(hWndAncestor,WM_PARENTNOTIFY,
		GET_WM_PARENTNOTIFY_MPS(WM_CREATE,hWnd32->wChildID,hWnd));
	} while (dwWinStyle & WS_CHILD);
    }

    /* if the window have been destroyed during WM_PARENTNOTIFY, return */
    if (!IsWindow(hWnd))
    {
	RELEASEWININFO(hWnd32);
	return (HWND)0;
    }

    /* Show window, if necessary */
    if (csClient.style & WS_VISIBLE) {
	if (hWnd32->dwStyle & WS_MINIMIZE) {
	    hWnd32->dwStyle &= ~WS_MINIMIZE;
	    ShowWindow(hWnd,SW_MINIMIZE);
	}
	else if (hWnd32->dwStyle & WS_MAXIMIZE) {
	    hWnd32->dwStyle &= ~WS_MAXIMIZE;
	    ShowWindow(hWnd,SW_MAXIMIZE);
	}
	else
	    ShowWindow(hWnd, SW_SHOW);
    }

    RELEASEWININFO(hWnd32);
    return hWnd;
}

HWND
CreateRootWindow()
{
    static HWND hRoot;

    HWND32 lpRootWindow;
    HCLASS32 hClassRoot32;
    WNDCLASSEX RootClass;
    int nScreenWidth,nScreenHeight;

    if(hRoot)
	return hRoot;

    /* Check if class has been registered */
    if (!(hClassRoot32 = FindClass((LPSTR)"ROOTWClass",(HANDLE)0))) {
	return (HWND)0;
    }

    /* Get class information */
    InternalGetClassInfoEx(hClassRoot32,&RootClass);

    /* Create handle for the new window */
    if (!(lpRootWindow = CREATEHWIN(hRoot))) {
	RELEASECLASSINFO(hClassRoot32);
	return 0;
    }

    /* Set root window proc */
    lpRootWindow->lpfnWndProc = RootClass.lpfnWndProc;

    /* set style & flags*/
    lpRootWindow->dwStyle = WS_POPUP|WS_VISIBLE;
    lpRootWindow->dwWinFlags = 0L;

    /* set class pointer */
    lpRootWindow->hWindowClass32 = hClassRoot32;

    /* for now root window doesn't have any extra bytes */

    /* Set root window size */
    nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

    SetRect(&lpRootWindow->rWnd,0,0,nScreenWidth,nScreenHeight);
    lpRootWindow->wWidth = nScreenWidth;
    lpRootWindow->wHeight = nScreenHeight;

    /* Every window must have a task.  It determines the queue to use */
    lpRootWindow->hTask = GetCurrentTask();

    RELEASEWININFO(lpRootWindow);
    RELEASECLASSINFO(hClassRoot32);
    return hRoot;
}

static void
SendDestroyMessages(HWND hWndDestroy)
{
    HWND hWnd;

    SendMessage(hWndDestroy,WM_DESTROY,0,0);

    if (!IsWindow(hWndDestroy))
	return;

    hWnd = GetWindow(hWndDestroy,GW_CHILD);
    while (hWnd) {
	SendDestroyMessages(hWnd);
	hWnd = GetWindow(hWnd,GW_HWNDNEXTSIB);
    }
}

static void
FreeWindow(HWND hWnd)
{
    HWND hWndFree,hWndFreeNext;
    HWND32 hWnd32;
    DWORD dwClassStyle;

    if (!(hWnd32 = GETHWND32(hWnd)))
	return;

    hWndFree = hWnd32->hWndChild;
    while (hWndFree) {
	hWndFreeNext = GetWindow(hWndFree,GW_HWNDNEXTSIB);
	FreeWindow(hWndFree);
	hWndFree = hWndFreeNext;
    }

    if (hWnd == GetActiveWindow()) {
	if (hWnd32->hWndOwner)
	    SetActiveWindow(hWnd32->hWndOwner);
	else
	    SetActiveWindow(TWIN_FindAnotherOverlapped(hWnd));
    }

    if (hWnd == GetFocus())
	SetFocus(0);
    if (hWnd == GetCapture())
	ReleaseCapture();

    if (!(hWnd32->dwStyle & WS_CHILD))
	if (hWnd32->hMenu) {
	    if (IsMenu(hWnd32->hMenu))
		DestroyMenu(hWnd32->hMenu);
	}
    if (hWnd32->hSysMenu) {
	if (IsMenu(hWnd32->hSysMenu))
	    DestroyMenu(hWnd32->hSysMenu);
    }

    /* NOTE: this used to be after releasing OWN/CLASS DC, but */
    /* at least one app does a GetDC(hWnd) during the NCDESTROY*/
    /* so, don't release it until after. The GetDC code needs  */
    /* the DC to maintain the hardware DC, (read gc for X11)   */

    /* send the last message to this window */
    SendMessage(hWnd,WM_NCDESTROY,0,0);

    /* get rid of DCs that may still be gotten */
    dwClassStyle = GetClassStyle(hWnd);
    if ((dwClassStyle & CS_OWNDC) && hWnd32->hDC)
	TWIN_GdiDestroyDC(hWnd32->hDC);

    if (!(hWnd32->dwStyle & WS_CHILD))
	/* physically destroy the window */
	DRVCALL_WINDOWS(PWSH_DESTROYWINDOW,0L,0L,
			WIN_GETDRVDATA(Get32WindowFrame(hWnd32)));

    if (hWnd32->UpdateRegion)
	DRVCALL_REGIONS(PRH_DESTROYREGION,hWnd32->UpdateRegion,0,0);

    LockClass(hWnd32->hWindowClass32,FALSE);

    SendMessage(Get32WindowFrame(hWnd32),WM_NCDESTROY,0,0L);
    FREEHWND(Get32WindowFrame(hWnd32));

    if (hWnd32->hWndHZScroll) {
	SendMessage(hWnd32->hWndHZScroll,WM_DESTROY,0,0L);
	FREEHWND(hWnd32->hWndHZScroll);
    }
    if (hWnd32->hWndVTScroll) {
	SendMessage(hWnd32->hWndVTScroll,WM_DESTROY,0,0L);
	FREEHWND(hWnd32->hWndVTScroll);
    }

    RELEASEWININFO(hWnd32);
    TWIN_FlushWindowMessages(hWnd);
    FREEHWND(hWnd);
}

BOOL
TWIN_InternalDestroyWindow(HWND hWnd)
{
    HWND32 hWnd32;

    if (!(hWnd32 = CHECKHWND32(hWnd)))
	return FALSE;

    hWnd32->dwWinFlags |= WFDEAD;
    RELEASEWININFO(hWnd32);
    
    TWIN_FlushWindowMessages(hWnd);

    return DestroyWindow(hWnd);
}

BOOL WINAPI
DestroyWindow(HWND hWnd)
{
    HWND32 hWnd32, hWndSib32;
    HWND hWndSibling, hWndAncestor, hWndFocus, hWndTmp, hWndOwner;
    DWORD dwStyle;
    int parent_with_focus;

    if (!(hWnd32 = CHECKHWND32(hWnd))) {
	return FALSE;
    }

#ifdef	LATER
	Send WM_CANCELMODE, if caption is active
	Call RealizeDefaultPalette
	Send WM_PALETTECHANGED to all windows
#endif

    if ((hWnd32->dwStyle & WS_CHILD) &&
	!(hWnd32->dwExStyle & WS_EX_NOPARENTNOTIFY)) {
	hWndAncestor = hWnd;
	do {
	    hWndAncestor = GetParent(hWndAncestor);
	    dwStyle = GetWindowStyle(hWndAncestor);
	    SendMessage(hWndAncestor,WM_PARENTNOTIFY,
		GET_WM_PARENTNOTIFY_MPS(WM_DESTROY,hWnd32->wChildID,hWnd));
	} while (dwStyle & WS_CHILD);
    }
    if (!IsWindow(hWnd))
	return TRUE;

    if (hWnd32->dwStyle & WS_VISIBLE) {
	if (hWnd32->dwStyle & WS_CHILD)
	    ShowWindow(hWnd, SW_HIDE);
	else	/* top-level */
	    SetWindowPos(hWnd,0,0,0,0,0,
		SWP_HIDEWINDOW|SWP_NOACTIVATE|
		SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);
    }
    if (!IsWindow(hWnd))
	return TRUE;

    if (hWnd32->hWndIconTitle)
	DestroyWindow(hWnd32->hWndIconTitle);

    if (!(hWnd32->dwStyle & WS_CHILD)) {
	hWndSibling = GetWindow(hWnd, GW_HWNDFIRST);
	while (hWndSibling) {
	    /* save the next sibling before this one is destroyed */
	    hWndSib32 = GETHWND32(hWndSibling);
	    hWndTmp = hWndSib32->hWndSibling;
	    hWndOwner = hWndSib32->hWndOwner;
	    RELEASEWININFO(hWndSib32);
	    
	    if (hWndOwner == hWnd)
		DestroyWindow(hWndSibling);
	    if (hWndTmp && !IsWindow(hWndTmp))	/* start all over */
		hWndSibling = GetWindow(hWnd, GW_HWNDFIRST);
	    else
		hWndSibling = hWndTmp;
	}
    }

    /* if focus is held by this window or one of it's descendants,
	revert to the parent */

    parent_with_focus= 0;

    if ((hWndFocus = GetFocus()))
	if (hWndFocus == hWnd || IsChild(hWnd,hWndFocus))
	    if(IsWindow(hWnd)) {
		parent_with_focus= GetParent(hWnd);
		SetFocus(parent_with_focus);
	    }

    /* if we are destroying the frame that has the cursor, reset
	the cursor ownership */
    if (!(hWnd32->dwStyle & WS_CHILD)) {
	if (Get32WindowFrame(hWnd32) == TWIN_GetCursorWindow()) {
	    TWIN_SetCursorWindow((HWND)0);
	    if (parent_with_focus) {
		int n;
		POINT FAR lppt;

		GetCursorPos(&lppt);
		n=SendMessage(parent_with_focus, WM_NCHITTEST,(WPARAM)0, MAKELPARAM(lppt.x,lppt.y));
		SendMessage(parent_with_focus, WM_SETCURSOR, (WPARAM)parent_with_focus, n);
	    }
	}
    }

    SendDestroyMessages(hWnd);
    RemoveFromList(hWnd32);
    RELEASEWININFO(hWnd32);
    FreeWindow(hWnd);
    return TRUE;
}


static void
ScreenToClientRect(HWND hWnd, RECT *rcScreen, RECT *rcClient)
{
    POINT pt;

    /*
     *  Convert a Rectangle from screen coordinates to client
     *  coordinates, by converting the two defining points.
     */
    pt.x = rcScreen->left;
    pt.y = rcScreen->top;
    ScreenToClient(hWnd, &pt);
    rcClient->left = pt.x;
    rcClient->top = pt.y;

    pt.x = rcScreen->right;
    pt.y = rcScreen->bottom;
    ScreenToClient(hWnd, &pt);
    rcClient->right = pt.x;
    rcClient->bottom = pt.y;
}


HWND WINAPI
SetParent(HWND hWndChild, HWND hWndNewParent)
{
    HWND32 hWndChild32, hWnd32Parent, hWndFrame32;
    HWND hWndOldParent,hWndFrame;
    WINCREATESTRUCT wcs;
    RECT rcOldFrame;
    RECT rcOldChild;
    RECT rcFrame;
    RECT rcChild;
    RECT rcParent;
    DWORD dwOldStyle;

    APISTR((LF_API,
	"SetParent(hWndChild:%.04x,hWndNewParent:%.04x)\n",
		hWndChild, hWndNewParent));

    if (!(hWndChild32 = GETHWND32(hWndChild))) {
		return(0);
    }
    if ((hWndOldParent = hWndChild32->hWndParent) == hWndNewParent)
    {
	RELEASEWININFO(hWndChild32);
	return hWndOldParent;
    }

    if (hWndNewParent) {
	if (!(hWnd32Parent = GETHWND32(hWndNewParent))) {
	    RELEASEWININFO(hWndChild32);
	    return hWndOldParent;
	}
    }
    GetWindowRect(hWndChild,&rcOldChild);
    RemoveFromList(hWndChild32);

    dwOldStyle = hWndChild32->dwStyle;

    hWndFrame = GetWindowFrame(hWndChild);
    GetWindowRect(hWndFrame,&rcOldFrame);
    hWndFrame32 = GETHWND32(hWndFrame);
    hWndChild32->hWndParent = hWndNewParent;
    hWndFrame32->hWndParent = hWndNewParent;

    if (hWndNewParent) {
	hWndChild32->dwStyle &= ~WS_POPUP;
	hWndChild32->dwStyle |= WS_CHILD;
	hWndFrame32->dwStyle &= ~WS_POPUP;
	hWndFrame32->dwStyle |= WS_CHILD;
    }
    else {
	hWndChild32->dwStyle &= ~WS_CHILD;
	hWndChild32->dwStyle |= WS_POPUP;
	hWndFrame32->dwStyle &= ~WS_CHILD;
	hWndFrame32->dwStyle |= WS_POPUP;
    }

    TWIN_InsertAsFirstSibling(hWndChild32);

    if ((dwOldStyle & WS_CHILD) && (hWndNewParent == 0)) {
	POINT pt;
	HICON hIcon;
	HCURSOR hCursor;

	memset((LPSTR)&wcs,0,sizeof(WINCREATESTRUCT));
	wcs.lpszName = hWndChild32->lpWindowName;
	wcs.dwStyle = hWndChild32->dwStyle;
	wcs.dwExStyle = hWndFrame32->dwExStyle;
	wcs.nBorder = hWndChild32->wXBorder;
	if ((hIcon = GetClassIcon(hWndChild)))
	    FillIconData(&wcs,hIcon);
	if ((hCursor = GetClassCursor(hWndChild)))
	    FillCursorData(&wcs,hCursor);
	wcs.hWndFrame = Get32WindowFrame(hWndChild32);
	wcs.hWndClient = hWndChild;
	pt.x = hWndFrame32->rWnd.left;
	pt.y = hWndFrame32->rWnd.top;
	ClientToScreen(hWndOldParent,&pt);
	SetRect(&wcs.rcFrame,pt.x,pt.y,
			     pt.x + hWndFrame32->wWidth,
			     pt.y + hWndFrame32->wHeight);
	hWndFrame32->lpDrvData = (LPVOID)DRVCALL_WINDOWS(PWSH_CREATEWINDOW,
					0,0,&wcs);
    }
    else
	if (!(dwOldStyle & WS_CHILD) && hWndNewParent) {
	    DRVCALL_WINDOWS(PWSH_DESTROYWINDOW,0L,0L,
		WIN_GETDRVDATA(Get32WindowFrame(hWndChild32)));
	}

    TWIN_ReparentDC(hWndChild);

    RELEASEWININFO(hWndChild32);
    RELEASEWININFO(hWndFrame32);
    RELEASEWININFO(hWnd32Parent);

    if (hWndOldParent && IsWindowVisible(hWndOldParent))
    {
	ScreenToClientRect(hWndOldParent, &rcOldFrame, &rcParent);
	InvalidateRect(hWndOldParent, &rcParent, TRUE);
	ScreenToClientRect(hWndOldParent, &rcOldChild, &rcParent);
	InvalidateRect(hWndOldParent, &rcParent, TRUE);
    }

    if (IsWindowVisible(hWndChild))
    {
        GetClientRect(hWndFrame,&rcFrame);
        InvalidateRect(hWndFrame, &rcFrame, TRUE);
        GetClientRect(hWndChild,&rcChild);
        InvalidateRect(hWndChild, &rcChild, TRUE);
    }

    return hWndOldParent;
}

BOOL
CreateSystemScrollbars(HWND hWnd, int nFlag)
{
    HWND32 hWnd32;
    BOOL bHScroll = FALSE,bVScroll = FALSE;
    HWND hWndHZScroll,hWndVTScroll;

    switch (nFlag) {
	case SB_HORZ:
	    bHScroll = TRUE;
	    break;
	case SB_VERT:
	    bVScroll = TRUE;
	    break;
	case SB_BOTH:
	    bHScroll = bVScroll = TRUE;
	    break;
	default:
	    return FALSE;
    }

    if (bHScroll || bVScroll) {
	/* Check if the scrollbar class has been registered */
	if (!hClassScroll32) {
	    if (!(hClassScroll32 = FindClass(TWIN_SYSSCROLLCLASS,0))) {
	        return FALSE;
	    }
	    /* Get scrollbar class info */
	    InternalGetClassInfoEx(hClassScroll32,&ScrollInfo);
	    LockClass(hClassScroll32,TRUE);
	}
    }

    if (!(hWnd32 = GETHWND32(hWnd)))
	return FALSE;

    if (bHScroll) {
	if (!(hWndHZScroll = AddScrollbar(hWnd32,FALSE)))
	{
	    RELEASEWININFO(hWnd32);
	    return FALSE;
	}
	
    }

    if (bVScroll) {
	if (!(hWndVTScroll = AddScrollbar(hWnd32,TRUE)))
	{
	    RELEASEWININFO(hWnd32);
	    return FALSE;
	}
	
    }

    RELEASEWININFO(hWnd32);
    return TRUE;
}

static HWND
AddScrollbar(HWND32 hWnd32, BOOL bVert)
{
    HWND hWndScroll;
    HWND32 hScroll32;
    CREATESTRUCT csScroll;
    RECT rcScroll;

    /* Create scrollbar window handle */
    if (!(hScroll32 = CREATEHWIN(hWndScroll))) {
	return (HWND)0;
    }

    hScroll32->lpfnWndProc = ScrollInfo.lpfnWndProc;
    hScroll32->dwStyle = WS_CHILD |
			((bVert)?SBS_VERT:SBS_HORZ) | SBS_SYSTEM;
    hScroll32->dwExStyle = 0;
    hScroll32->hWndParent = hScroll32->hWndOwner = GETHWND16(hWnd32);
    hScroll32->hInstance = hWnd32->hInstance;
    hScroll32->hTask = GetCurrentTask();
    hScroll32->hWindowClass32 = hClassScroll32;
    hScroll32->cbWndExtra = ScrollInfo.cbWndExtra;
    hScroll32->lpWndExtra = WinMalloc(ScrollInfo.cbWndExtra);
    memset(hScroll32->lpWndExtra, '\0', ScrollInfo.cbWndExtra);

    /* fill CREATESTRUCT */
    csScroll.lpCreateParams = NULL;
    csScroll.hInstance = hScroll32->hInstance;
    csScroll.hwndParent = hScroll32->hWndParent;
    csScroll.cx = 0;
    csScroll.cy = 0;
    csScroll.x = 0;
    csScroll.y = 0;
    csScroll.style = hScroll32->dwStyle;
    csScroll.lpszName = NULL;
    csScroll.lpszClass = TWIN_SYSSCROLLCLASS;
    csScroll.dwExStyle = hScroll32->dwExStyle;

    CalcSysScrollLocation(hWnd32,
		(WORD)(hWnd32->rWnd.right-hWnd32->rWnd.left),
		(WORD)(hWnd32->rWnd.bottom-hWnd32->rWnd.top),
		&rcScroll,
		bVert);
    csScroll.x = rcScroll.left;
    csScroll.y = rcScroll.top;
    csScroll.cx = rcScroll.right;
    csScroll.cy = rcScroll.bottom;

    if (bVert)
	hWnd32->hWndVTScroll = hWndScroll;
    else
	hWnd32->hWndHZScroll = hWndScroll;

    SendMessage(hWndScroll,WM_CREATE,0,(LPARAM)&csScroll);

    SetRectEmpty(&hScroll32->rcNC);
    hScroll32->wWidth = csScroll.cx;
    hScroll32->wHeight = csScroll.cy;
    SetRect(&hScroll32->rWnd,
		csScroll.x,
		csScroll.y,
		csScroll.x+csScroll.cx,
		csScroll.y+csScroll.cy);

    RELEASEWININFO(hScroll32);
    return hWndScroll;
}

void
CalcSysScrollLocation(HWND32 hWnd32,WORD wWidth, WORD wHeight,
			LPRECT lprcScroll,BOOL bVert)
{
    int nScrollWidth,nScrollHeight;
    RECT rcNC;

    nScrollWidth = GetSystemMetrics(SM_CXVSCROLL);
    nScrollHeight = GetSystemMetrics(SM_CYHSCROLL);
    CalcExpectedNC(&rcNC,hWnd32->dwStyle,hWnd32->dwExStyle);

    if (!bVert) {
	SetRect(lprcScroll,
		hWnd32->rcNC.left,
		wHeight-hWnd32->rcNC.bottom,
		wWidth-hWnd32->rcNC.left-hWnd32->rcNC.right,
		nScrollHeight);

	if (rcNC.left)
	    lprcScroll->left--;
	if (rcNC.right)
	    lprcScroll->right++;
	if (hWnd32->dwStyle & WS_VSCROLL)
	    lprcScroll->right++;
	return;
    }
    else {
	SetRect(lprcScroll,
		wWidth-hWnd32->rcNC.right,
		hWnd32->rcNC.top,
		nScrollWidth,
		wHeight-hWnd32->rcNC.top-hWnd32->rcNC.bottom);

	if (rcNC.top)
	    lprcScroll->top--;
	if (rcNC.bottom)
	    lprcScroll->bottom++;
	if (hWnd32->dwStyle & WS_HSCROLL)
	    lprcScroll->bottom++;
	return;
    }
}

static void
FillIconData(LPWINCREATESTRUCT lpwcs,HICON hIcon)
{
    LPTWIN_ICONINFO lpIconInfo;
    LPIMAGEINFO lpImageInfo;

    if (!hIcon || !(lpIconInfo = GETICONINFO(hIcon))) {
	lpwcs->lpIconXORData = lpwcs->lpIconANDData = 0;
	return;
    }

    if (!(lpImageInfo = GETBITMAPINFO(lpIconInfo->hXORImage)))
    {
        if (!(lpImageInfo = GETBITMAPINFO(lpIconInfo->hMonoBmp)))
        {
	   lpwcs->lpIconXORData = lpwcs->lpIconANDData = 0;
	   RELEASEICONINFO(lpIconInfo);
	   return;
        }
    }
    lpwcs->lpIconXORData = lpImageInfo->lpDrvData;

    RELEASEBITMAPINFO(lpImageInfo);

    if (!(lpImageInfo = GETBITMAPINFO(lpIconInfo->hANDMask))) {
	lpwcs->lpIconANDData = 0;
	RELEASEICONINFO(lpIconInfo);
	return;
    }
    lpwcs->lpIconANDData = lpImageInfo->lpDrvData;
    RELEASEBITMAPINFO(lpImageInfo);
}


static void
FillCursorData(LPWINCREATESTRUCT lpwcs,HCURSOR hCursor)
{
    LPCURSORINFO lpCursorInfo;

    if (!hCursor) 
	hCursor = LoadCursor(0,IDC_ARROW);

    if ((lpCursorInfo = GETCURSORINFO(hCursor)))
    {
	lpwcs->Cursor = lpCursorInfo->lpCurData;
	RELEASECURSORINFO(lpCursorInfo);
    }
    else
	lpwcs->Cursor = NULL;
}
