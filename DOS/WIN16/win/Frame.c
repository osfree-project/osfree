/*    
	Frame.c	2.45
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
#include <string.h>

#ifndef OEMRESOURCE
#define OEMRESOURCE     /* for OBM_CLOSE */
#endif
#include "windows.h"
#include "windowsx.h"
#undef OEMRESOURCE

#include "Frame.h"
#include "WinData.h"
#include "Driver.h"
#include "WinDefs.h"
#include "Log.h"
#include "WinConfig.h"

#define WM_ISACTIVEICON		0x0035

/* action flags for DrawIconTitle */

#define DIT_RECALC		0x0001
#define DIT_NORECALC		0x0000
#define DIT_REDRAW		0x0002
#define DIT_NOREDRAW		0x0000



/* external stuff */
extern BOOL TestWF(HWND, DWORD);
extern DWORD GetWF(HWND);
extern void ClearWF(HWND, DWORD);
extern void SetWF(HWND, DWORD);
extern HPEN 	GetSysColorPen(int);

void DrawSizeBox(HWND32);

static void DrawMinMaxGlyph(HDC, RECT, int, BOOL);
static void DrawSysMenuGlyph(HDC, RECT, BOOL);
static void SetMinMaxGlyphRect(HWND, LPRECT, int);
static void DoNCDragMove(HWND,int,POINT);
static void ClipCursorByClient(HWND);

#define	GLYPH_MIN	1
#define	GLYPH_MAX	0
#define	GLYPH_RESTORE	3

LRESULT
FrameProc(HWND hWndFrame, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND32 hWndFrame32;
    int nNCHit;
    POINT pt;

    if (!(hWndFrame32 = GETHWND32(hWndFrame)))
	return 0L;

    if (uMsg == WM_CLOSE) {
        HWND hWndClient  = Get32FrameClient(hWndFrame32);
	RELEASEWININFO(hWndFrame32);
	return SendMessage(hWndClient,WM_CLOSE,0,0L);
    }

    if (!(hWndFrame32->dwStyle & WS_CHILD) &&
	GetWindowMenu(Get32FrameClient(hWndFrame32))) {
        RELEASEWININFO(hWndFrame32);
	return MenuBarProc(hWndFrame,uMsg,wParam,lParam);
    }

    switch (uMsg) {
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
	case WM_MOUSEMOVE:
	    pt.x = LOWORD(lParam);
	    pt.y = HIWORD(lParam);
	    ClientToScreen(hWndFrame,&pt);
	    nNCHit = (int)SendMessage(
		Get32FrameClient(hWndFrame32),
		WM_NCHITTEST,
		0,
		MAKELONG(pt.x,pt.y));
	    /* WS_NC*BUTTON* = WM_*BUTTON* - 0x160 */
	    {
	        LRESULT lr =
		  SendMessage(Get32FrameClient(hWndFrame32),uMsg - 0x160,
			      (WPARAM)nNCHit,MAKELONG(pt.x,pt.y));
		RELEASEWININFO(hWndFrame32);
		return lr;
	    }
	default:
	    RELEASEWININFO(hWndFrame32);
	    return DefWindowProc(hWndFrame,uMsg,wParam,lParam);
    }
}

char buffer[80];

void
DrawWindowFrame(HWND hWnd, BOOL fDrawMenuBar)
{
    HWND32 hWnd32;
    int nBorder, nCount, nStart = 0;
    RECT rcWnd,rc;
    HBRUSH hBr;
    HPEN hPen,hOldPen;
    HDC hDC;
    BOOL bActive;
    BOOL fHorzScroll = FALSE,fVertScroll = FALSE;
    int nOffset;

    hWnd32 = GETHWND32(hWnd);
    if (hWnd32->dwStyle & WS_MINIMIZE) {
	ClearWF(hWnd, WFNCDIRTY);
	RELEASEWININFO(hWnd32);
	return;
    }
    if (hWnd32->dwStyle & WS_CHILD) {
	bActive = (hWnd32->dwWinFlags & WFFRAMEON)?TRUE:FALSE;
	hDC = GetWindowDC(hWnd);
	CopyRect(&rc, &hWnd32->rWnd);
	OffsetRect(&rc, -rc.left, -rc.top);
	CopyRect(&rcWnd,&rc);

	nBorder = ((hWnd32->dwStyle & WS_BORDER) ||
		   ((hWnd32->dwStyle & WS_DLGFRAME) &&
		    !(hWnd32->dwStyle & WS_BORDER)))?
		GetSystemMetrics(SM_CXBORDER):0;
	if (nBorder) {
	    hBr = GetSysColorBrush(COLOR_WINDOWFRAME);
	    for (nCount = 0; nCount < nBorder; nCount++) {
	        FrameRect(hDC, &rcWnd, hBr);
	        InflateRect(&rcWnd, -1, -1);
	    }
	    nStart = nBorder;
	}

	nBorder = ((hWnd32->dwStyle & WS_DLGFRAME) &&
		   !(hWnd32->dwStyle & WS_BORDER))?
		GetSystemMetrics(SM_CXDLGFRAME):0;
	if (nBorder) {
	    hBr = (bActive)?
			GetSysColorBrush(COLOR_ACTIVECAPTION):
			GetSysColorBrush(COLOR_INACTIVECAPTION);
	    for (nCount = nStart; nCount < nBorder-1; nCount++) {
	        FrameRect(hDC, &rcWnd, hBr);
	        InflateRect(&rcWnd, -1, -1);
	    }
	    hBr = GetSysColorBrush(COLOR_WINDOWFRAME);
	    FrameRect(hDC, &rcWnd, hBr);
	}

	nBorder = ((hWnd32->dwStyle & WS_THICKFRAME) ||
        	   (hWnd32->dwExStyle & WS_EX_DLGMODALFRAME))?
		GetSystemMetrics(SM_CXFRAME):0;
	if (nBorder) {
	    hBr = (bActive)?
			GetSysColorBrush(COLOR_ACTIVEBORDER):
			GetSysColorBrush(COLOR_INACTIVEBORDER);
	    for (nCount = nStart; nCount < nBorder-1; nCount++) {
	        FrameRect(hDC, &rcWnd, hBr);
	        InflateRect(&rcWnd, -1, -1);
	    }
	    hBr = GetSysColorBrush(COLOR_WINDOWFRAME);
	    FrameRect(hDC, &rcWnd, hBr);
	    hPen = GetSysColorPen(COLOR_WINDOWFRAME);
	    hOldPen = SelectObject(hDC,hPen);
	    nOffset = GetSystemMetrics(SM_CYCAPTION);

	/* upper left-hand corner */
	    MoveTo(hDC,rc.left,rc.top+nBorder+nOffset-1);
	    LineTo(hDC,rc.left+nBorder-1,rc.top+nBorder+nOffset-1);
	    MoveTo(hDC,rc.left+nBorder+nOffset-1,rc.top);
	    LineTo(hDC,rc.left+nBorder+nOffset-1,rc.top+nBorder-1);
	/* upper right-hand corner */
	    MoveTo(hDC,rc.right-nOffset-nBorder,rc.top);
	    LineTo(hDC,rc.right-nOffset-nBorder,rc.top+nBorder-1);
	    MoveTo(hDC,rc.right-nBorder,rc.top+nOffset+nBorder-1);
	    LineTo(hDC,rc.right-1,rc.top+nOffset+nBorder-1);
	/* lower left */
	    MoveTo(hDC,rc.left,rc.bottom-nOffset-nBorder);
	    LineTo(hDC,rc.left+nBorder-1,rc.bottom-nOffset-nBorder);
	    MoveTo(hDC,rc.left+nBorder+nOffset-1,rc.bottom-nBorder);
	    LineTo(hDC,rc.left+nBorder+nOffset-1,rc.bottom-1);
	/* lower right */
	    MoveTo(hDC,rc.right-nOffset-nBorder,rc.bottom-nBorder);
	    LineTo(hDC,rc.right-nOffset-nBorder,rc.bottom-1);
	    MoveTo(hDC,rc.right-nBorder,rc.bottom-nOffset-nBorder);
	    LineTo(hDC,rc.right-1,rc.bottom-nOffset-nBorder);

	    SelectObject(hDC,hOldPen);
	}
	if ((hWnd32->dwStyle & WS_CAPTION) == WS_CAPTION)
	    DrawCaption(hWnd,hDC,TRUE,bActive);
	ReleaseDC(hWnd, hDC);
    }
    else {
	SendMessage(hWnd,WM_GETTEXT,(WPARAM)80,(LPARAM)buffer);
	DRVCALL_WINDOWS(PWSH_SETTEXT,0L,buffer,
			WIN_GETDRVDATA(hWnd32->hWndFrame));
	if (hWnd32->hMenu && fDrawMenuBar)
	    DrawMenuBar(hWnd);
    }
    if (hWnd32->hWndHZScroll && IsWindowVisible(hWnd32->hWndHZScroll)) {
	fHorzScroll = TRUE;
	SendMessage(hWnd32->hWndHZScroll,WM_PAINT,0,0L);
    }
    if (hWnd32->hWndVTScroll && IsWindowVisible(hWnd32->hWndVTScroll)) {
	fVertScroll = TRUE;
	SendMessage(hWnd32->hWndVTScroll,WM_PAINT,0,0L);
    }
    if (fHorzScroll && fVertScroll) 
	DrawSizeBox(hWnd32);

    ClearWF(hWnd, WFNCDIRTY);
    RELEASEWININFO(hWnd32);
}

void
DrawSizeBox(HWND32 hWnd32)
{
    RECT rc, rcNC;
    HDC hDC;
    DWORD dwBorders;

    SetRect(&rc, hWnd32->rWnd.right-hWnd32->rWnd.left-hWnd32->rcNC.right+1,
		hWnd32->rWnd.bottom-hWnd32->rWnd.top-hWnd32->rcNC.bottom+1,
		hWnd32->rWnd.right-hWnd32->rWnd.left,
		hWnd32->rWnd.bottom-hWnd32->rWnd.top);

    hDC = GetWindowDC(GETHWND16(hWnd32));

    if (hWnd32->dwStyle & WS_CHILD) {
	dwBorders = CalcBorders(hWnd32->dwStyle, hWnd32->dwExStyle);
	rc.right -= LOWORD(dwBorders);
	rc.bottom -= HIWORD(dwBorders);
    }
    else {
	CalcExpectedNC(&rcNC, hWnd32->dwStyle, hWnd32->dwExStyle);
	OffsetRect(&rc, rcNC.left, rcNC.top);
    }
    FillRect(hDC,&rc,GetSysColorBrush(COLOR_SCROLLBAR));
    ReleaseDC(GETHWND16(hWnd32),hDC);
}

char szWindowText[80];

void
DrawCaption(HWND hWnd, HDC hDC, BOOL bFlag, BOOL bFunc)
{
    HWND32 hWnd32;
    HBRUSH hCaptionBrush;
    COLORREF crTextColor;
    RECT rc,rcTemp;
    HPEN hBlackPen;
    int nVTBorder,nHZBorder;
    int nCaptionHeight;

    hWnd32 = GETHWND32(hWnd);
    if (!(hWnd32->dwStyle & WS_CHILD) ||
	(hWnd32->dwStyle & WS_MINIMIZE)) {
        RELEASEWININFO(hWnd32);
	return;
    }
    if ((hWnd32->dwStyle & WS_CAPTION) == WS_CAPTION) {
	SendMessage(hWnd, WM_GETTEXT, 80, (LONG)szWindowText);
	if (bFunc) {
	    hCaptionBrush = GetSysColorBrush(COLOR_ACTIVECAPTION);
	    crTextColor = GetSysColor(COLOR_CAPTIONTEXT);
	}
	else {
	    hCaptionBrush = GetSysColorBrush(COLOR_INACTIVECAPTION);
	    crTextColor = GetSysColor(COLOR_INACTIVECAPTIONTEXT);
	}
	hBlackPen = GetStockObject(BLACK_PEN);
	nVTBorder = hWnd32->wXBorder;
	nHZBorder = hWnd32->wYBorder;
	nCaptionHeight = GetSystemMetrics(SM_CYCAPTION);
	SetRect(&rc,
		nVTBorder,
		nHZBorder,
		hWnd32->rWnd.right-hWnd32->rWnd.left-nVTBorder,
		nHZBorder+nCaptionHeight);
	FillRect(hDC,&rc,hCaptionBrush);
	SelectObject(hDC,hBlackPen);
	MoveTo(hDC,rc.left,rc.bottom-1);
	LineTo(hDC,rc.right-1,rc.bottom-1);

	if (hWnd32->dwStyle & WS_SYSMENU) {
	    SetRect(&rcTemp,
		rc.left-1,
		rc.top-1,
		rc.left+nCaptionHeight,
		rc.bottom);
	    DrawSysMenuGlyph(hDC,rcTemp,FALSE);
	    rc.left += nCaptionHeight;
	}

	SetRect(&rcTemp,rc.right - nCaptionHeight,rc.top - 1,
			rc.right, rc.bottom);
			
	if (hWnd32->dwStyle & WS_MAXIMIZEBOX) {
	    DrawMinMaxGlyph(hDC,rcTemp,GLYPH_MAX,FALSE);
	    rcTemp.left -= nCaptionHeight;
	    rcTemp.right -= nCaptionHeight;
	    rc.right -= nCaptionHeight;
	}
	if (hWnd32->dwStyle & WS_MINIMIZEBOX) {
	    DrawMinMaxGlyph(hDC,rcTemp,GLYPH_MIN,FALSE);
	    rc.right -= nCaptionHeight;
	}

	SetTextColor(hDC,crTextColor);
	SetBkMode(hDC,TRANSPARENT);
	DrawText(hDC,szWindowText,strlen(szWindowText),
		&rc,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    }
    RELEASEWININFO(hWnd32);
}

static void
SetMinMaxGlyphRect(HWND hWnd, LPRECT lprc, int nType)
{
    HWND32 hWnd32;
    int nCaptionHeight,nVTBorder,nHZBorder;
    int nRightEdge;

    SetRectEmpty(lprc);
    if (!(hWnd32 = GETHWND32(hWnd)))
	return;
    nCaptionHeight = GetSystemMetrics(SM_CYCAPTION);
    nVTBorder = hWnd32->wXBorder;
    nHZBorder = hWnd32->wYBorder;
    nRightEdge = hWnd32->rWnd.right-hWnd32->rWnd.left-nVTBorder;
    switch (nType) {
	case GLYPH_MAX:
	    if (!(hWnd32->dwStyle & WS_MAXIMIZEBOX)) {
	        RELEASEWININFO(hWnd32);
		return;
	    }
	    SetRect(lprc,
		nRightEdge-nCaptionHeight,
		nHZBorder-1,
		nRightEdge,
		nHZBorder+nCaptionHeight-1);
	    break;
	case GLYPH_MIN:
	    if (!(hWnd32->dwStyle & WS_MINIMIZEBOX)) {
	        RELEASEWININFO(hWnd32);
		return;
	    }
	    SetRect(lprc,
                nRightEdge-nCaptionHeight,
                nHZBorder-1,
                nRightEdge,
                nHZBorder+nCaptionHeight-1);
	    if (hWnd32->dwStyle & WS_MAXIMIZEBOX)
		OffsetRect(lprc,-nCaptionHeight,0);
	    break;
    }
    RELEASEWININFO(hWnd32);
}

static void
DrawMinMaxGlyph(HDC hDC, RECT rc, int nType, BOOL fPressed)
{
    static HBITMAP hZoom,hZoomD,hReduce,hReduceD;
    HBITMAP hBitmap = 0,hBitmapOld;
    static BITMAP bm;
    HDC hDCMem;

    if (bm.bmWidth == 0) {
	hZoom = LoadBitmap(0,(LPSTR)OBM_ZOOM);
	GetObject(hZoom,sizeof(BITMAP),(LPVOID)&bm);
    }
    switch (nType) {
	case GLYPH_MAX:
	    if (fPressed == FALSE)  
		hBitmap = hZoom;
	    else {
		if (hZoomD == 0)
		    hZoomD = LoadBitmap(0,(LPSTR)OBM_ZOOMD);
		hBitmap = hZoomD;
	    }
	    break;

	case GLYPH_MIN:
	    if (fPressed == FALSE) { 
		if (hReduce == 0)
		    hReduce = LoadBitmap(0,(LPSTR)OBM_REDUCE);
		hBitmap = hReduce;
	    }
	    else {
		if (hReduceD == 0)
		    hReduceD = LoadBitmap(0,(LPSTR)OBM_REDUCED);
		hBitmap = hReduceD;
	    }
	    break;
    }
    hDCMem = CreateCompatibleDC(hDC);
    hBitmapOld = SelectObject(hDCMem,hBitmap);
    BitBlt(hDC,rc.left,rc.top,bm.bmWidth,bm.bmHeight,hDCMem,0,0,SRCCOPY);
    SelectObject(hDCMem,hBitmapOld);
    DeleteDC(hDCMem);
}

static void
DrawSysMenuGlyph(HDC hDC, RECT rc, BOOL fPressed)
{
    static HBITMAP hSysMenuBitmap = 0;
    static BITMAP bm;
    HBITMAP hBitmap,hBitmapOld;
    HDC hDCMem,hDCSrc;

    hDCMem = CreateCompatibleDC(hDC);
    if (hSysMenuBitmap) 
	hBitmapOld = SelectObject(hDCMem,hSysMenuBitmap);
    else {
	hBitmap = LoadBitmap(0,(LPSTR)OBM_CLOSE);
	GetObject(hBitmap,sizeof(BITMAP),(LPVOID)&bm);
	hDCSrc = CreateCompatibleDC(hDC);
	SelectObject(hDCSrc,hBitmap);
	hSysMenuBitmap = CreateCompatibleBitmap(hDC,
			rc.right-rc.left,rc.bottom-rc.top);
	hBitmapOld = SelectObject(hDCMem,hSysMenuBitmap);
	BitBlt(hDCMem,0,0,bm.bmWidth/2,bm.bmHeight,
        		hDCSrc,bm.bmWidth/2,0,
        		SRCCOPY);
        SelectObject(hDCMem,GetStockObject(BLACK_PEN));
	SelectObject(hDCMem,GetStockObject(NULL_BRUSH));
        Rectangle(hDCMem,0,0,rc.right-rc.left,rc.bottom-rc.top);
	SelectObject(hDCSrc,hBitmapOld);
	DeleteObject(hBitmap);
	DeleteDC(hDCSrc);
	GetObject(hSysMenuBitmap,sizeof(BITMAP),(LPVOID)&bm);
    }
    BitBlt(hDC,rc.left,rc.top,bm.bmWidth,bm.bmHeight,
		hDCMem,0,0,SRCCOPY);
    SelectObject(hDCMem,hBitmapOld);
    DeleteDC(hDCMem);

    if (fPressed) 
	InvertRect(hDC,&rc);
}

DWORD
CalcBorders(DWORD dwStyle, DWORD dwExStyle)
{
    int nXBorder,nYBorder;	

    nXBorder = nYBorder = 0;

    if ((dwStyle & WS_BORDER) ||
        (!(dwStyle & WS_POPUP) && !(dwStyle & WS_CHILD))) {
	nXBorder = GetSystemMetrics(SM_CXBORDER);
	nYBorder = GetSystemMetrics(SM_CYBORDER);
    }

    if ((dwStyle & WS_THICKFRAME) || (dwExStyle & WS_EX_DLGMODALFRAME)) {
	nXBorder = GetSystemMetrics(SM_CXFRAME);
	nYBorder = GetSystemMetrics(SM_CYFRAME);
    }

    if ((dwStyle & WS_DLGFRAME) && !(dwStyle & WS_BORDER)) {
	nXBorder = GetSystemMetrics(SM_CXDLGFRAME);
	nYBorder = GetSystemMetrics(SM_CYDLGFRAME);
    }

    return MAKELONG(nXBorder,nYBorder);
}

void
CalcNCDimensions(LPRECT lpRect, DWORD dwStyle, DWORD dwExStyle,
		 WORD wMenuHeight)
{
    DWORD dwXYBorders;

    SetRectEmpty(lpRect);

    if (dwStyle & WS_MINIMIZE)
	return;

    if (dwStyle & WS_CHILD) {
	dwXYBorders = CalcBorders(dwStyle,dwExStyle);
        lpRect->top = lpRect->bottom = HIWORD(dwXYBorders);
        lpRect->left = lpRect->right = LOWORD(dwXYBorders);
        if ((dwStyle & WS_CAPTION) == WS_CAPTION) {
            lpRect->top += GetSystemMetrics(SM_CYCAPTION);
        }
    }
    else {
        lpRect->top += wMenuHeight;
	dwXYBorders = 0;
    }
    if (dwStyle & WS_HSCROLL) {
	lpRect->bottom += GetSystemMetrics(SM_CYHSCROLL);
	if (HIWORD(dwXYBorders)) /* scrollbar's border overlaps window's border */
	    lpRect->bottom--;
    }
    if (dwStyle & WS_VSCROLL) {
	lpRect->right += GetSystemMetrics(SM_CXVSCROLL);
	if (LOWORD(dwXYBorders))
	    lpRect->right--;
    }
}

void
CalcExpectedNC(LPRECT lpRect, DWORD dwStyle, DWORD dwExStyle)
{
    DWORD dwXYBorders;

    SetRectEmpty(lpRect);

    dwXYBorders = CalcBorders(dwStyle,dwExStyle);
    lpRect->top = lpRect->bottom = HIWORD(dwXYBorders);
    lpRect->left = lpRect->right = LOWORD(dwXYBorders);
    if ((dwStyle & WS_CAPTION) == WS_CAPTION)
	lpRect->top += GetSystemMetrics(SM_CYCAPTION);
}

HWND
TWIN_ConvertToSysScroll(HWND hWnd, BOOL bVert, LPPOINT lppt)
{
    HWND32 hWnd32,hWndScroll32;
    HWND hWndScroll;

    ASSERT_HWND(hWnd32,hWnd,0);

    if ((HWND)0 == (hWndScroll = (bVert)?
		hWnd32->hWndVTScroll:hWnd32->hWndHZScroll)) {
        RELEASEWININFO(hWnd32)
	return 0;
    }

    ASSERT_HWND(hWndScroll32,hWndScroll,0);

    lppt->x -= hWndScroll32->rWnd.left-hWnd32->rcNC.left;
    lppt->y -= hWndScroll32->rWnd.top-hWnd32->rcNC.top;

    RELEASEWININFO(hWnd32);
    RELEASEWININFO(hWndScroll32);
    return hWndScroll;
}

void
DoSysCommand(HWND hWnd,WPARAM wParam,LPARAM lParam)
{
    switch (wParam & 0xfff0) {
	case SC_CLOSE:
	    SendMessage(hWnd,WM_CLOSE,(WPARAM)0,(LPARAM)0L);
	    break;
	case SC_MINIMIZE:
	    ShowWindow(hWnd,SW_MINIMIZE);
	    break;
	case SC_MAXIMIZE:
	    ShowWindow(hWnd,SW_MAXIMIZE);
	    break;
	case SC_RESTORE:
	    ShowWindow(hWnd,SW_RESTORE);
	    break;
	case SC_MOVE:
	case SC_SIZE:
	default:
	    break;
    }
}

int
DoNCHitTest(HWND hWnd, POINT ptScreen)
{
    RECT rcMenu,rcCaption,rcInside,rcWnd,rcScroll;
    HWND32 hWnd32;
    int nHZScroll, nVTScroll, nYCaption;
    int nWidth, nHeight, nXBorder, nYBorder;
    static hRgnBorder = (HRGN)0;
    static hRgnWnd = (HRGN)0;
    static hRgnInside = (HRGN)0;
    POINT pt;

    if (!(hWnd32 = GETHWND32(hWnd)))
	return HTERROR;

    if (hWnd32->dwStyle & WS_MINIMIZE) {
        RELEASEWININFO(hWnd32);
	return HTSYSMENU;
    }

    pt = ptScreen;
    ScreenToClient(hWnd32->hWndFrame,&pt);

    nWidth = hWnd32->rWnd.right-hWnd32->rWnd.left;
    nHeight = hWnd32->rWnd.bottom-hWnd32->rWnd.top;

    SetRect(&rcWnd,0,0,nWidth,nHeight);
    if (!PtInRect(&rcWnd,pt)) {
        RELEASEWININFO(hWnd32);        
	return HTNOWHERE;
    }

    if (hWnd32->dwStyle & WS_CHILD) {
	nXBorder = hWnd32->wXBorder;
	nYBorder = hWnd32->wYBorder;
    }
    else 
	nXBorder = nYBorder = 0;
    
    if (!(hWnd32->dwStyle & WS_CHILD)) {
	if (hWnd32->hMenu) {
	    SetRect(&rcMenu,0,0,nWidth,hWnd32->wMenuHeight);
	    if (PtInRect(&rcMenu,pt)) {
	        RELEASEWININFO(hWnd32);
		return HTMENU;
	    }
	}
    }
    else if ((hWnd32->dwStyle & WS_CAPTION) == WS_CAPTION) {
	nYCaption = GetSystemMetrics(SM_CYCAPTION);
	SetRect(&rcCaption,
		nXBorder,nYBorder,
		nWidth-nXBorder,nYBorder+nYCaption);

	if (PtInRect(&rcCaption,pt)) {
	    if ((hWnd32->dwStyle & WS_SYSMENU) &&
		(pt.x <= rcCaption.left + nYCaption)) {
	        RELEASEWININFO(hWnd32);
		return HTSYSMENU;
	    }
		
	    if (hWnd32->dwStyle & WS_MAXIMIZEBOX) {
		if (pt.x >= rcCaption.right - nYCaption)
		    return HTMAXBUTTON;
		if ((hWnd32->dwStyle & WS_MINIMIZEBOX) &&
			(pt.x >= rcCaption.right - 2*nYCaption)) {
		    RELEASEWININFO(hWnd32);
		    return HTMINBUTTON;
		}
	    }
	    if ((hWnd32->dwStyle & WS_MINIMIZEBOX) &&
		(pt.x >= rcCaption.right - nYCaption)) {
	        RELEASEWININFO(hWnd32);
		return HTMINBUTTON;
	    }
	    RELEASEWININFO(hWnd32);
	    return HTCAPTION;
	}
    }

    nHZScroll = GetSystemMetrics(SM_CYHSCROLL);
    nVTScroll = GetSystemMetrics(SM_CXVSCROLL);

    if (hWnd32->dwStyle & WS_VSCROLL) {
	SetRect(&rcScroll,
		nWidth-nXBorder-nVTScroll,
		hWnd32->rcNC.top,
		nWidth-nXBorder,
		nHeight-hWnd32->rcNC.bottom);
	if (PtInRect(&rcScroll,pt)) {
	    RELEASEWININFO(hWnd32);
	    return HTVSCROLL;
	}
    }

    if (hWnd32->dwStyle & WS_HSCROLL) {
	SetRect(&rcScroll,
		hWnd32->rcNC.left,
		nHeight-nYBorder-nHZScroll,
		nWidth-hWnd32->rcNC.right,
		nHeight-nYBorder);
	if (PtInRect(&rcScroll,pt)) {
	    RELEASEWININFO(hWnd32);
	    return HTHSCROLL;
	}
    }

    SetRect(&rcInside,
		nXBorder,nYBorder,
		nWidth-nXBorder,nHeight-nYBorder);
    if (PtInRect(&rcInside,pt)) {
        RELEASEWININFO(hWnd32);
	return HTCLIENT;
    }

    if (!hRgnWnd)
	hRgnWnd = CreateRectRgn(0,0,0,0);
    SetRectRgn(hRgnWnd,
		rcWnd.left,
		rcWnd.top,
		rcWnd.right,
		rcWnd.bottom);

    if (!hRgnInside)
	hRgnInside = CreateRectRgn(0,0,0,0);
    SetRectRgn(hRgnInside,
		rcInside.left,
		rcInside.top,
		rcInside.right,
		rcInside.bottom);

    if (!hRgnBorder)
	hRgnBorder = CreateRectRgn(0,0,0,0);
    CombineRgn(hRgnBorder,hRgnWnd,hRgnInside,RGN_DIFF);

    if (PtInRegion(hRgnBorder,pt.x,pt.y)) {
	if (!(hWnd32->dwStyle & WS_THICKFRAME)) {
	        RELEASEWININFO(hWnd32);	  
		return HTBORDER;
	}
	else {
	    int wXb = (int)hWnd32->wXBorder;
	    int wYb = (int)hWnd32->wYBorder;
	    RELEASEWININFO(hWnd32);  /* all paths return so release now */
	    if (pt.x <= wXb) {
		if (pt.y <= nHZScroll)
		    return HTTOPLEFT;
		if (pt.y >= nHeight-nHZScroll)
		    return HTBOTTOMLEFT;
		return HTLEFT;
	    }
	    if (pt.x >= (int)nWidth-wXb) {
		if (pt.y <= nHZScroll)
		    return HTTOPRIGHT;
		if (pt.y >= nHeight-nHZScroll)
		    return HTBOTTOMRIGHT;
		return HTRIGHT;
	    }
	    if (pt.x <= nVTScroll)
		return (pt.y <= wYb)?
				HTTOPLEFT:HTBOTTOMLEFT;
	    if (pt.x >= nWidth-nVTScroll)
		return (pt.y <= wYb)?
				HTTOPRIGHT:HTBOTTOMRIGHT;
	    return (pt.y <= wYb)?
				HTTOP:HTBOTTOM;
	}
    }
    else {
        RELEASEWININFO(hWnd32);
	return HTNOWHERE;
    }
}

LONG
DoNCMouse(HWND hWnd, UINT uMsg, int nNCHit, POINT pt)
{
    HCURSOR hCursor;
    static HCURSOR hARROW = (HCURSOR)0;
    static HCURSOR hSIZEWE = (HCURSOR)0;
    static HCURSOR hSIZENS = (HCURSOR)0;
    static HCURSOR hSIZENESW = (HCURSOR)0;
    static HCURSOR hSIZENWSE = (HCURSOR)0;
    DWORD dwStyle;

    switch (uMsg) {
	case WM_NCMOUSEMOVE:
	    dwStyle = GetWindowLong(hWnd,GWL_STYLE);
	    if (!(dwStyle & WS_CHILD))
		break;
	    switch(nNCHit) {
		case HTCAPTION:
		    if (!hARROW)
		        hARROW = LoadCursor(0,IDC_ARROW);
		    hCursor = hARROW;
		    break;
		case HTRIGHT:
		case HTLEFT:
	            if (!hSIZEWE)
		        hSIZEWE = LoadCursor(0,IDC_SIZEWE);
		    hCursor = hSIZEWE;
		    break;
		case HTTOP:
		case HTBOTTOM:
		    if (!hSIZENS)
			hSIZENS = LoadCursor(0,IDC_SIZENS);
		    hCursor = hSIZENS;
		    break;
		case HTBOTTOMLEFT:
		case HTTOPRIGHT:
		    if (!hSIZENESW)
			hSIZENESW = LoadCursor(0,IDC_SIZENESW);
		    hCursor = hSIZENESW;
		    break;
		case HTBOTTOMRIGHT:
		case HTTOPLEFT:
		    if (!hSIZENWSE)
			hSIZENWSE = LoadCursor(0,IDC_SIZENWSE);
		    hCursor = hSIZENWSE;
		    break;
		default:
		    if (!hARROW)
		        hARROW = LoadCursor(0,IDC_ARROW);
		    hCursor = hARROW;
		    break;
	    }
	    SetCursor(hCursor);
	    break;

	case WM_NCLBUTTONDOWN:
	    dwStyle = GetWindowLong(hWnd,GWL_STYLE);
	    if (dwStyle & (WS_DLGFRAME|WS_THICKFRAME)) {
	    	DoNCDragMove(hWnd, nNCHit, pt);
            }	
	    break;

	case WM_NCLBUTTONDBLCLK:
	    if (nNCHit == HTCAPTION)
		PostMessage(hWnd, WM_SYSCOMMAND, 
		            (WPARAM)SC_MAXIMIZE,
		            MAKELPARAM(pt.x,pt.y));
	    else
	    if ( IsIconic ( hWnd ) )
		PostMessage ( hWnd, WM_SYSCOMMAND, ( WPARAM )SC_RESTORE,
		              MAKELPARAM ( pt.x, pt.y ) );
	    break;
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONDOWN:
	case WM_NCRBUTTONUP:
	case WM_NCRBUTTONDBLCLK:
	case WM_NCMBUTTONDOWN:
	case WM_NCMBUTTONUP:
	case WM_NCMBUTTONDBLCLK:
	default:
	    break;
    }
    return 0L;
}

LRESULT
IconTitleWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hWndOwner;

    switch(uMsg) {
	case WM_ACTIVATE:
	    break;

	case WM_CLOSE:
	    return (LRESULT)0;

	case WM_ERASEBKGND:
	    DrawIconTitle(hWnd,DIT_NORECALC|DIT_REDRAW);
	    break;

	case WM_SHOWWINDOW:
	    if (wParam) 
		DrawIconTitle(hWnd,DIT_RECALC|DIT_NOREDRAW);
	    return (LRESULT)1L;

	case WM_NCHITTEST:
	    return (LRESULT)HTCLIENT;

	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
    	    hWndOwner = GetWindow(hWnd,GW_OWNER);
	    if ((uMsg != WM_MOUSEMOVE) && !TestWF(hWnd,WFFRAMEON)) 
		SendMessage(hWndOwner,WM_CHILDACTIVATE,0,0L);
	    return SendMessage(hWndOwner,uMsg-WM_MOUSEMOVE+WM_NCMOUSEMOVE,
				wParam,lParam);

	default:
	    break;
    }
    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

char szText[40];

void
DrawIconTitle(HWND hWnd, UINT uiAction)
{
    HWND hWndOwner;
    HBRUSH hCaptionBackBrush;
    COLORREF crCaption;
    RECT rc,rcText,rcOwner;
    POINT pt;
    HDC hDC;
    int dx, dy;
    int nTextHeight;
    BOOL fActive;
    static HFONT hFont = 0;
    HFONT hOldFont;
    LOGFONT lf;
    int nXSpacing,nXIcon;
    int nMaxLen;

    if (!(hWndOwner = GetWindow(hWnd,GW_OWNER)))
	return;
    hDC = GetDC(hWnd);
    if (!hFont) {
	memset((LPSTR)&lf,'\0',sizeof(LOGFONT));

	lf.lfHeight = GetTwinInt(WCP_ICONFONTSIZE);

	GetTwinString(WCP_ICONFONTFACE,lf.lfFaceName,LF_FACESIZE);

	hFont = CreateFontIndirect(&lf);
    }

    hOldFont = SelectObject(hDC,hFont);
    SendMessage(hWndOwner,WM_GETTEXT,40,(LONG)szText);
    szText[39] = '\0';
    GetWindowRect(hWnd,&rc);
    SetRectEmpty(&rcText);
    nTextHeight = DrawText(hDC,szText,-1,&rcText,
		DT_CALCRECT|DT_SINGLELINE|DT_CENTER|DT_NOPREFIX|DT_NOCLIP);
    nXIcon = GetSystemMetrics(SM_CXICON);
    nXSpacing = GetSystemMetrics(SM_CXICONSPACING);
    nMaxLen = nXIcon + nXSpacing;
    if (rcText.right > nMaxLen) {
#ifdef LATER
	lpLine = &szText[0];
	nBreak = 0;
	for (i=0; szText[i]; i++) 
	    if (szText[i] == ' ') {
		lpNewLine = &szText[i+1];
		nLen1 = LOWORD(GetTextExtent(hDC,lpLine,i-nBreak));
		nLen2 = LOWORD(GetTextExtent(hDC,lpNewLine,strlen(lpNewLine)));
		if (nLen1 + nLen2 <= nMaxLen)
		    continue;
		szText[i] = '\n';
		if (nLen2 <= nMaxLen) 
		    break;
		lpLine = lpNewLine;
		nBreak = i;
	    }
	    if (szText[i] && (uiAction & DIT_RECALC)) {
		SetRectEmpty(&rcText);
		nTextHeight = DrawText(hDC,szText,-1,&rcText,
				DT_CALCRECT|DT_CENTER|DT_NOPREFIX|DT_NOCLIP);
	    }
#else	/* Doesn't work -- DrawText doesn't do the right thing */
	SetRect(&rcText,0,0,nMaxLen,0);
	nTextHeight = DrawText(hDC,szText,-1,&rcText,
				DT_WORDBREAK|DT_CALCRECT|
				DT_CENTER|DT_NOPREFIX|DT_NOCLIP);
#endif
    }
    if (uiAction & DIT_RECALC) {
	dx = rcText.right - (rc.right - rc.left) + 2;
	dy = nTextHeight - (rc.bottom - rc.top) + 2;
	GetWindowRect(hWndOwner,&rcOwner);
	if (dx || dy || (rc.top != (rcOwner.bottom + 2)) || 
			(rc.left != (rcOwner.left - (rc.right - rc.left)/2))) {
	    pt.x = rcOwner.left + (rcOwner.right-rcOwner.left)/2;
	    pt.y = rcOwner.bottom + 2;
	    pt.x -= rcText.right/2;
	    ScreenToClient(GetParent(hWnd),&pt);
	    SetWindowPos(hWnd,0,pt.x,pt.y,
			rcText.right+2,nTextHeight+2,
			SWP_NOZORDER|SWP_NOACTIVATE);
	    if (!(uiAction & DIT_NOREDRAW)) {
		SelectObject(hDC,hOldFont);
		ReleaseDC(hWnd,hDC);
		hDC = GetDC(hWnd);
		hOldFont = SelectObject(hDC,hFont);
	    }
	}
    }
    if (!(uiAction & DIT_NOREDRAW)) {
	fActive = (BOOL)SendMessage(hWndOwner,WM_ISACTIVEICON,0,0L);
	GetClientRect(hWnd,&rc);
	if (fActive) {
	    hCaptionBackBrush = GetSysColorBrush(COLOR_ACTIVECAPTION);
	    crCaption = GetSysColor(COLOR_ACTIVECAPTION);
	}
	else {
	    hCaptionBackBrush = GetSysColorBrush(COLOR_APPWORKSPACE);
	    crCaption = GetSysColor(COLOR_APPWORKSPACE);
	}

	FillRect(hDC,&rc,hCaptionBackBrush);

	SetBkColor(hDC,crCaption);

	SetTextColor(hDC,
		(fActive)?GetSysColor(COLOR_CAPTIONTEXT):
			GetSysColor(COLOR_INACTIVECAPTIONTEXT));
	DrawText(hDC,szText,-1,&rc,
			DT_WORDBREAK|DT_CENTER|DT_NOPREFIX|DT_NOCLIP);
    }
    SelectObject(hDC,hOldFont);
    ReleaseDC(hWnd,hDC);
}

#define	NCM_DRAGGING		0x0002
#define	NCM_MINBUTTONDOWN	0x0004
#define	NCM_MAXBUTTONDOWN	0x0008

static void
DoNCDragMove(HWND hWnd,int nNCHit, POINT ptStart)
{
    HWND32 hWnd32;
    HDC hWindowDC;
    UINT uiStatus = 0;
    HCURSOR hSaveCursor = 0,hCursor = 0;
    int nInitHit;
    int nXDiff, nYDiff;
    HDC hDCParent = 0;
    BOOL bResizeRequested = FALSE;
    RECT rcCurr, rcPrev, rc;
    POINT pt, ptScreen;
    WORD wSWPFlags = 0;
    HDWP hDWP;
    int nCaptionHeight,nVTBorder,nHZBorder;
    HPEN hPen = 0;
    HBRUSH hBrush;
    MSG msg;

    if (!(hWnd32 = GETHWND32(hWnd)))
	return;

    if (nNCHit == HTMENU) {
	ScreenToClient(hWnd32->hWndFrame,&ptStart);
	FrameProc(hWnd32->hWndFrame,WM_LBUTTONDOWN, 0,
		MAKELPARAM(ptStart.x, ptStart.y));
	RELEASEWININFO(hWnd32);	  
	return;
    }

    SendMessage(hWnd,WM_ENTERSIZEMOVE,0,0L);

    SetCapture(hWnd32->hWndFrame);
    nInitHit = nNCHit;

    if (hWnd32->dwStyle & WS_MINIMIZE) 
	SendMessage(hWnd,WM_SYSCOMMAND,SC_MOVE,0L);
    else {
	switch (nNCHit) {
	    case HTMAXBUTTON:
		hWindowDC = GetWindowDC(hWnd);
		SetMinMaxGlyphRect(hWnd,&rc,GLYPH_MAX);
		DrawMinMaxGlyph(hWindowDC,rc,GLYPH_MAX,TRUE);
		uiStatus |= NCM_MAXBUTTONDOWN;
		ReleaseDC(hWnd,hWindowDC);
		break;
	    case HTMINBUTTON:
		hWindowDC = GetWindowDC(hWnd);
		SetMinMaxGlyphRect(hWnd,&rc,GLYPH_MIN);
		DrawMinMaxGlyph(hWindowDC,rc,GLYPH_MIN,TRUE);
		uiStatus |= NCM_MINBUTTONDOWN;
		ReleaseDC(hWnd,hWindowDC);
		break;
	    case HTCAPTION:
	    case HTLEFT:
	    case HTTOP:
	    case HTRIGHT:
	    case HTBOTTOM:
	    case HTTOPLEFT:
	    case HTTOPRIGHT:
	    case HTBOTTOMLEFT:
	    case HTBOTTOMRIGHT:
		hDCParent = GetDCEx(hWnd,(HRGN)0,DCX_WINDOW|DCX_PARENTCLIP);
		SetViewportOrg(hDCParent,
		    -hWnd32->rWnd.left,-hWnd32->rWnd.top);
		uiStatus |= NCM_DRAGGING;
		SetROP2(hDCParent,R2_NOT);
		hPen = CreatePen(PS_SOLID,2,RGB(0,0,0));
		hPen = SelectObject(hDCParent,hPen);
		hBrush = SelectObject(hDCParent,GetStockObject(NULL_BRUSH));
		SetRectEmpty(&rcPrev);
		SendMessage(hWnd,WM_SYSCOMMAND,
			(nNCHit == HTCAPTION)?SC_MOVE:SC_SIZE,
			MAKELPARAM(ptStart.x,ptStart.y));
		ClipCursorByClient(hWnd32->hWndParent);
		if ((hWnd32->dwStyle & WS_CAPTION) == WS_CAPTION)
		    BringWindowToTop(hWnd);
		break;
	    case HTSYSMENU:
		nCaptionHeight = GetSystemMetrics(SM_CYCAPTION);
		nVTBorder = hWnd32->wXBorder;
		nHZBorder = hWnd32->wYBorder;
		SetRect(&rc,
			nVTBorder,
			nHZBorder,
			nVTBorder+nCaptionHeight-1,
			nHZBorder+nCaptionHeight-1);
		hWindowDC = GetWindowDC(hWnd);
		InvertRect(hWindowDC,&rc);
		ptScreen.x = rc.left;
		ptScreen.y = rc.bottom;
		ClientToScreen(hWnd32->hWndFrame,&ptScreen);
		TrackPopupMenu(hWnd32->hSysMenu,TPM_LEFTALIGN,
			ptScreen.x,ptScreen.y,1,hWnd,(LPRECT)0);
		InvertRect(hWindowDC,&rc);
		ReleaseDC(hWnd,hWindowDC);
	        RELEASEWININFO(hWnd32);
		return;
	}
    }
    WaitMessage();
    while (TRUE) {
	if (PeekMessage(&msg,hWnd,
			WM_NCMOUSEMOVE,WM_NCMBUTTONDBLCLK,PM_REMOVE))
	    msg.message += (WM_MOUSEMOVE - WM_NCMOUSEMOVE);
	else {
	    if (!PeekMessage(&msg,hWnd32->hWndFrame,
			WM_MOUSEFIRST,WM_MOUSELAST,PM_REMOVE)) {
		/*
		 *  Wait for there to be a message in the queue, so
		 *  we don't just call PeekMessage() repeatedly, causing
		 *  a CPU-bound loop.  This still doesn't help us if
		 *  we ever get a non-mouse message, since those will
		 *  sit in the queue, causing WaitMessage() to immediately
		 *  return.  However, this fixes the problem 99% of the
		 *  time, since it is very rare to get any other type of
		 *  message while dragging a window.  (A keypress could
		 *  be generated, but most users aren't going to do that.)
		 *  The extra work to remove the CPU-bound loop in that case
		 *  (basically, a GetMessage() equivalent that will allow
		 *  filtering on two distinct sets of messages) is a bit
		 *  more than is desired to be done at this time.
	 	 */
		WaitMessage();
		continue;
	    }
	}

	pt = msg.pt;
	nNCHit = DoNCHitTest(hWnd,pt);

	if (msg.message == WM_MOUSEMOVE) {
	    if (hWnd32->dwStyle & WS_MINIMIZE) {
		if (!(uiStatus & NCM_DRAGGING)) { /* start drag */
		    uiStatus |= NCM_DRAGGING;
		    hSaveCursor = GetCursor();
		    if ((hCursor = GetDragIconCursor(hWnd))) {
			SetCursor(hCursor);
			SetWindowPos(hWnd,(HWND)NULL,0,0,0,0,
				SWP_HIDEWINDOW|SWP_NOZORDER|SWP_NOSIZE|
				SWP_NOMOVE|SWP_NOACTIVATE);
		    }
		    ShowIconTitle(hWnd,FALSE);
		    UpdateWindow(hWnd32->hWndParent);
		    ClipCursorByClient(hWnd32->hWndParent);
		}
		else
		    continue;
	    }
	    else { /* non-iconic window */
		if (nInitHit == HTMINBUTTON) {
		    if (((nNCHit == HTMINBUTTON) &&
				(!(uiStatus & NCM_MINBUTTONDOWN))) ||
			((nNCHit != HTMINBUTTON) &&
				((uiStatus & NCM_MINBUTTONDOWN)))) {
			SetMinMaxGlyphRect(hWnd,&rc,GLYPH_MIN);
			hWindowDC = GetWindowDC(hWnd);
			if (nNCHit == HTMINBUTTON) {
			    DrawMinMaxGlyph(hWindowDC,rc,GLYPH_MIN,TRUE);
			    uiStatus |= NCM_MINBUTTONDOWN;
			}
			else {
			    DrawMinMaxGlyph(hWindowDC,rc,GLYPH_MIN,FALSE);
			    uiStatus &= ~NCM_MINBUTTONDOWN;
			}
			ReleaseDC(hWnd,hWindowDC);
		    }
		    continue;
		}
		if (nInitHit == HTMAXBUTTON) {
		    if (((nNCHit == HTMAXBUTTON) &&
				(!(uiStatus & NCM_MAXBUTTONDOWN))) ||
			((nNCHit != HTMAXBUTTON) &&
				((uiStatus & NCM_MAXBUTTONDOWN)))) {
			SetMinMaxGlyphRect(hWnd,&rc,GLYPH_MAX);
			hWindowDC = GetWindowDC(hWnd);
			if (nNCHit == HTMAXBUTTON) {
			    DrawMinMaxGlyph(hWindowDC,rc,GLYPH_MAX,TRUE);
			    uiStatus |= NCM_MAXBUTTONDOWN;
			}
			else {
			    DrawMinMaxGlyph(hWindowDC,rc,GLYPH_MAX,FALSE);
			    uiStatus &= ~NCM_MAXBUTTONDOWN;
			}
			ReleaseDC(hWnd,hWindowDC);
		    }
		    continue;
		}
		/* here we are in dragging mode */
		nXDiff = pt.x - ptStart.x;
		nYDiff = pt.y - ptStart.y;
	        switch(nInitHit) {
		    case HTCAPTION:
			SetRect(&rcCurr,hWnd32->rWnd.left + nXDiff,
				hWnd32->rWnd.top + nYDiff,
				hWnd32->rWnd.right + nXDiff,
				hWnd32->rWnd.bottom + nYDiff);
		        break;
		    case HTRIGHT:
			SetRect(&rcCurr,hWnd32->rWnd.left,
				hWnd32->rWnd.top,
				hWnd32->rWnd.right + nXDiff,
				hWnd32->rWnd.bottom);
		        break;
		    case HTLEFT:
			SetRect(&rcCurr,hWnd32->rWnd.left + nXDiff,
				hWnd32->rWnd.top,
				hWnd32->rWnd.right,
				hWnd32->rWnd.bottom);
		        break;
		    case HTTOP:
			SetRect(&rcCurr,hWnd32->rWnd.left,
				hWnd32->rWnd.top + nYDiff,
				hWnd32->rWnd.right,
				hWnd32->rWnd.bottom);
			break;
		    case HTBOTTOM:
			SetRect(&rcCurr,hWnd32->rWnd.left,
				hWnd32->rWnd.top,
				hWnd32->rWnd.right,
				hWnd32->rWnd.bottom + nYDiff);
		        break;
		    case HTBOTTOMLEFT:
			SetRect(&rcCurr,hWnd32->rWnd.left + nXDiff,
				hWnd32->rWnd.top,
				hWnd32->rWnd.right,
				hWnd32->rWnd.bottom + nYDiff);
			break;
		    case HTTOPRIGHT:
			SetRect(&rcCurr,hWnd32->rWnd.left,
				hWnd32->rWnd.top + nYDiff,
				hWnd32->rWnd.right + nXDiff,
				hWnd32->rWnd.bottom);
		        break;
		    case HTBOTTOMRIGHT:
			SetRect(&rcCurr,hWnd32->rWnd.left,
				hWnd32->rWnd.top,
				hWnd32->rWnd.right + nXDiff,
				hWnd32->rWnd.bottom + nYDiff);
			break;
		    case HTTOPLEFT:
			SetRect(&rcCurr,hWnd32->rWnd.left + nXDiff,
				hWnd32->rWnd.top + nYDiff,
				hWnd32->rWnd.right,
				hWnd32->rWnd.bottom);
		        break;
		    default:
		        break;
	        }
		Rectangle(hDCParent,rcCurr.left,rcCurr.top,
					rcCurr.right,rcCurr.bottom);
		if (!IsRectEmpty(&rcPrev)) 
		    Rectangle(hDCParent,rcPrev.left,rcPrev.top,
					rcPrev.right,rcPrev.bottom);
		CopyRect(&rcPrev,&rcCurr);
	    }
	    continue;
	}
	else if (msg.message == WM_LBUTTONUP) {
	    ReleaseCapture();
	    if (hWnd32->dwStyle & WS_MINIMIZE) {
		if (!(uiStatus & NCM_DRAGGING)) {
		    SendMessage(hWnd,WM_SYSCOMMAND,SC_MOUSEMENU,
				MAKELONG(pt.x,pt.y));
		    TrackPopupMenu(hWnd32->hSysMenu,TPM_LEFTALIGN,
				pt.x,pt.y,1,hWnd,(LPRECT)0);
		    RELEASEWININFO(hWnd32);
		    return;
		}
		else {
		    ScreenToClient(GetParent(hWnd),&pt);
		    pt.x -= GetSystemMetrics(SM_CXICON)/2;
		    pt.y -= GetSystemMetrics(SM_CYICON)/2;
		    if (hCursor) {
			SetCursor(hSaveCursor);
			DestroyCursor(hCursor);
		    }
		    hDWP = BeginDeferWindowPos(2);
		    DeferWindowPos(hDWP,hWnd,0,pt.x,pt.y,
			0,0,SWP_NOSIZE);
		    DeferWindowPos(hDWP,hWnd,0,0,0,0,0,
			SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE|
			SWP_NOACTIVATE|SWP_NOZORDER);
		    EndDeferWindowPos(hDWP);
		    ShowIconTitle(hWnd,TRUE);
		    ClipCursor((LPRECT)NULL);
		}
		break;
	    }
	    else {
		if (nInitHit == HTMINBUTTON) {
		    if (!(uiStatus & NCM_MINBUTTONDOWN)) {
		        RELEASEWININFO(hWnd32);
			return;
		    }
		    SetMinMaxGlyphRect(hWnd,&rc,GLYPH_MIN);
		    hWindowDC = GetWindowDC(hWnd);
		    DrawMinMaxGlyph(hWindowDC,rc,GLYPH_MIN,FALSE);
		    ReleaseDC(hWnd,hWindowDC);
		    PostMessage(hWnd,
			WM_SYSCOMMAND,
			(WPARAM)SC_MINIMIZE,
			MAKELPARAM(pt.x,pt.y));
		    RELEASEWININFO(hWnd32);
		    return;
		}
		if (nInitHit == HTMAXBUTTON) {
		    if (!(uiStatus & NCM_MAXBUTTONDOWN)) {
		        RELEASEWININFO(hWnd32);
			return;
		    }
		    SetMinMaxGlyphRect(hWnd,&rc,GLYPH_MAX);
		    hWindowDC = GetWindowDC(hWnd);
		    DrawMinMaxGlyph(hWindowDC,rc,GLYPH_MAX,FALSE);
		    ReleaseDC(hWnd,hWindowDC);
		    PostMessage(hWnd,
			WM_SYSCOMMAND,
			(WPARAM)SC_MAXIMIZE,
			MAKELPARAM(pt.x,pt.y));
		    RELEASEWININFO(hWnd32);
		    return;
		}
		switch (nInitHit) {
		    case HTCAPTION:
			SetRect(&rcCurr,
				hWnd32->rWnd.left+pt.x-ptStart.x,
				hWnd32->rWnd.top+pt.y-ptStart.y,
				0,0);
			wSWPFlags = SWP_NOSIZE;
			bResizeRequested = TRUE;
			break;
		    case HTTOP:
			SetRect(&rcCurr,
				hWnd32->rWnd.left,
				hWnd32->rWnd.top+pt.y-ptStart.y,
				hWnd32->rWnd.right-hWnd32->rWnd.left,
				hWnd32->rWnd.bottom-hWnd32->rWnd.top+
					ptStart.y-pt.y);
			wSWPFlags = 0;
			bResizeRequested = TRUE;
			break;
		    case HTLEFT:
			SetRect(&rcCurr,
				hWnd32->rWnd.left+pt.x-ptStart.x,
				hWnd32->rWnd.top,
				hWnd32->rWnd.right-hWnd32->rWnd.left+
					ptStart.x-pt.x,
				hWnd32->rWnd.bottom-hWnd32->rWnd.top);
			wSWPFlags = 0;
			bResizeRequested = TRUE;
			break;
		    case HTRIGHT:
			SetRect(&rcCurr,0,0,
				hWnd32->rWnd.right-hWnd32->rWnd.left+
						pt.x-ptStart.x,
				hWnd32->rWnd.bottom-hWnd32->rWnd.top);
			wSWPFlags = SWP_NOMOVE;
			bResizeRequested = TRUE;
			break;
		    case HTBOTTOM:
			SetRect(&rcCurr,0,0,
				hWnd32->rWnd.right-hWnd32->rWnd.left,
				hWnd32->rWnd.bottom-hWnd32->rWnd.top+
						pt.y-ptStart.y);
			wSWPFlags = SWP_NOMOVE;
			bResizeRequested = TRUE;
			break;
		    case HTBOTTOMLEFT:
			SetRect(&rcCurr,
				hWnd32->rWnd.left+pt.x-ptStart.x,
				hWnd32->rWnd.top,
				hWnd32->rWnd.right-hWnd32->rWnd.left+
						ptStart.x-pt.x,
				hWnd32->rWnd.bottom-hWnd32->rWnd.top+
						pt.y-ptStart.y);
			wSWPFlags = 0;
			bResizeRequested = TRUE;
			break;
		    case HTTOPRIGHT:
			SetRect(&rcCurr,
				hWnd32->rWnd.left,
				hWnd32->rWnd.top+pt.y-ptStart.y,
				hWnd32->rWnd.right-hWnd32->rWnd.left+
						pt.x-ptStart.x,
				hWnd32->rWnd.bottom-hWnd32->rWnd.top+
						ptStart.y-pt.y);
			wSWPFlags = 0;
			bResizeRequested = TRUE;
			break;
		    case HTBOTTOMRIGHT:
			SetRect(&rcCurr,0,0,
				hWnd32->rWnd.right-hWnd32->rWnd.left+
						pt.x-ptStart.x,
				hWnd32->rWnd.bottom-hWnd32->rWnd.top+
						pt.y-ptStart.y);
			wSWPFlags = SWP_NOMOVE;
			bResizeRequested = TRUE;
			break;
		    case HTTOPLEFT:
			SetRect(&rcCurr,
				hWnd32->rWnd.left+pt.x-ptStart.x,
				hWnd32->rWnd.top+pt.y-ptStart.y,
				hWnd32->rWnd.right-hWnd32->rWnd.left+
						ptStart.x-pt.x,
				hWnd32->rWnd.bottom-hWnd32->rWnd.top+
						ptStart.y-pt.y);
			wSWPFlags = 0;
			bResizeRequested = TRUE;
			break;
		    default:
			break;
		}

		if (!IsRectEmpty(&rcPrev))
		    Rectangle(hDCParent,rcPrev.left,rcPrev.top,
					rcPrev.right,rcPrev.bottom);
		if (bResizeRequested)
		    SetWindowPos(hWnd,HWND_TOP,
				rcCurr.left,rcCurr.top,
				rcCurr.right,rcCurr.bottom,
				(UINT)wSWPFlags);

		hPen = SelectObject(hDCParent,hPen);
		DeleteObject(hPen);
		ReleaseDC(hWnd,hDCParent);
		ClipCursor((LPRECT)NULL);
		break;
	    }
	}
	break;
    }	/* while (TRUE) */
    SendMessage(hWnd,WM_EXITSIZEMOVE,0,0L);
    RELEASEWININFO(hWnd32);
}

/* ClipCursorByClient sets cursor clipping to the window's client area  */

static void
ClipCursorByClient(HWND hWnd)
{
    RECT rc;

    GetClientRect(hWnd,&rc);
    ClientToScreen(hWnd,(LPPOINT)&rc);
    rc.right += rc.left;
    rc.bottom += rc.top;
    ClipCursor(&rc);
}
