/*    
	Scroll.c	2.25
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

#include "Log.h"
#include "kerndef.h"
#include "Scroll.h"

/* external linkage */
extern HPEN 	GetSysColorPen(int);
extern HCLASS32 FindClass(LPCSTR, HINSTANCE);
extern void ScrollbarSendItemDraw(LPTWINSCROLLINFO, WORD, WORD);
extern void ScrollbarDraw(LPTWINSCROLLINFO, LPDRAWITEMSTRUCT);
extern BOOL TWIN_RedrawWindow(HWND, const RECT *, HRGN, UINT);

/* exported routines */
LRESULT DefSCROLLBARProc(HWND, UINT, WPARAM, LPARAM);

/* internal routines */
static LRESULT ScrollbarUpdate(HWND, WPARAM,LPTWINSCROLLINFO); 
static void ScrollbarNotify(HWND, HWND, WORD, int, DWORD);
static LONG ScrollbarCreate(HWND, LPCREATESTRUCT);
static int ScrollbarHittest(LPTWINSCROLLINFO, POINT);
static WORD ScrollbarClickedRect(LPTWINSCROLLINFO, LPRECT);
static int SetTrackingPos(LPTWINSCROLLINFO,LPRECT,BOOL);

static LRESULT
ScrollbarUpdate(HWND hWnd, WPARAM wParam,LPTWINSCROLLINFO lpScrollInfo) 
{
    	RECT rc;
    	int nThumb,nRange;

	GetClientRect(hWnd,&rc);
	if (SBTSTYLE(lpScrollInfo,SBS_VERT)) {
	    rc.top += lpScrollInfo->wArrowHeight;
	    rc.bottom -= 2*lpScrollInfo->wArrowHeight;
	    rc.bottom = max(rc.bottom,rc.top);
	    nThumb = lpScrollInfo->wArrowHeight;
	    nRange = rc.bottom - rc.top;
	} else {
	    rc.left += lpScrollInfo->wArrowWidth;
	    rc.right -= 2*lpScrollInfo->wArrowWidth;
	    rc.right = max(rc.left,rc.right);
	    nThumb = lpScrollInfo->wArrowWidth;
	    nRange = rc.right - rc.left;
	}
	nRange++;

	lpScrollInfo->wThumbOffset =
		(!(lpScrollInfo->nMax-lpScrollInfo->nMin))?0:
		(int)((lpScrollInfo->nPos-lpScrollInfo->nMin)*nRange)/
		(int)(lpScrollInfo->nMax-lpScrollInfo->nMin);

	if (wParam)
		ScrollbarSendItemDraw(lpScrollInfo,
			ODA_LEFTFIELD|ODA_THUMB|ODA_RIGHTFIELD,0);
	return 1;
}

static WNDPROC lpScrollBinToNat = 0;

LRESULT
DefSCROLLBARProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    LPTWINSCROLLINFO lpScrollInfo;
    WORD wScrollCode = (WORD)-1;
    WORD wState = 0;
    WORD wAction = 0;
    WORD wHit;
    int nPos,nNewPos;
    POINT pt;
    RECT rc;
    HCLASS32 hScrollClass32;
    static HWND hWndCapture = (HWND)0;

    if (!(lpScrollInfo = (LPTWINSCROLLINFO)
		GetWindowLong(hWnd,SWD_LPSCROLLINFO)) &&
	wMsg != WM_CREATE && wMsg != WM_NCCREATE && wMsg != WM_CONVERT)
	return FALSE;

    switch(wMsg) {
	case WM_CREATE:
	    return ScrollbarCreate(hWnd,(LPCREATESTRUCT)lParam);

	case WM_DESTROY:
	    SetWindowLong(hWnd,SWD_LPSCROLLINFO,0L);
	    FREESBAR(lpScrollInfo->ObjHead.hObj);
	    return 1L;

	case WM_ERASEBKGND:
	    return 1L;

	case WM_ENABLE:
	    if (wParam)
		SBCSTATE(lpScrollInfo,SSF_GRAYED);
	    else
		SBSSTATE(lpScrollInfo,SSF_GRAYED);
	    InvalidateRect(hWnd,NULL,FALSE);
	    return 1L;

	case WM_SHOWWINDOW:
	    if (wParam)
		SBCSTATE(lpScrollInfo,SSF_HIDDEN);
	    else
		SBSSTATE(lpScrollInfo,SSF_HIDDEN);
	    return 1L;

	case WM_CANCELMODE:
	    SBCSTATE(lpScrollInfo,SSF_DOWNARROW|SSF_UPARROW);
	    UserKillSystemTimer(hWnd,IDT_FIRSTCLICK);
	    UserKillSystemTimer(hWnd,IDT_HOLDCLICK);
	    if (hWnd == GetCapture()) {
		if (!hWndCapture)
		    ReleaseCapture();
		else {
		    SetCapture(hWndCapture);
		    hWndCapture = (HWND)0;
		}
	    }
	    hWndCapture = (HWND)0;
	    return 1L;

	case WM_SYSTIMER:
	    if (wParam == IDT_FIRSTCLICK) {
		UserKillSystemTimer(hWnd,wParam);
		SetSystemTimer(hWnd,IDT_HOLDCLICK,CTICKS_HOLDCLICK,NULL);
	    }
	    if (!SBTSTATE(lpScrollInfo,SSF_MOUSEOUT)) {
	        wAction = ScrollbarClickedRect(lpScrollInfo,&rc);
		switch(wAction) {
		    case ODA_LEFTARROW:
			wScrollCode = SB_LINEUP;
			break;
		    case ODA_RIGHTARROW:
			wScrollCode = SB_LINEDOWN;
			break;
		    case ODA_LEFTFIELD:
			wScrollCode = SB_PAGEUP;
			break;
		    case ODA_RIGHTFIELD:
			wScrollCode = SB_PAGEDOWN;
			break;
		    case ODA_THUMB:
			wAction = 0;
			break;
		}
		if (wAction)
		    ScrollbarNotify(hWnd,GetParent(hWnd),wScrollCode,
		        0, lpScrollInfo->dwStyle);
	    }
	    return 1L;

	case WM_GETDLGCODE:
	    return (LONG)DLGC_WANTARROWS;

	case WM_PAINT:
	    ScrollbarSendItemDraw(lpScrollInfo,ODA_DRAWENTIRE,0);
	    ValidateRect(hWnd,NULL);
	    return 0L;

	case WM_KEYDOWN:
	    switch(wParam) {
		case VK_PRIOR:
		    if (!SBTSTATE(lpScrollInfo,SSF_UPARROWGRAYED))
			wScrollCode = SB_PAGEUP;
		    break;
		case VK_NEXT:
		    if (!SBTSTATE(lpScrollInfo,SSF_DOWNARROWGRAYED))
			    wScrollCode = SB_PAGEDOWN;
		    break;
		case VK_END:
		    if (!SBTSTATE(lpScrollInfo,SSF_DOWNARROWGRAYED))
			    wScrollCode = SB_BOTTOM;
		    break;
		case VK_HOME:
		    if (!SBTSTATE(lpScrollInfo,SSF_UPARROWGRAYED))
			    wScrollCode = SB_TOP;
		    break;
		case VK_LEFT:
		case VK_UP:
		    if (!SBTSTATE(lpScrollInfo,SSF_UPARROWGRAYED))
			    wScrollCode = SB_LINEUP;
		    break;
		case VK_RIGHT:
		case VK_DOWN:
		    if (!SBTSTATE(lpScrollInfo,SSF_DOWNARROWGRAYED))
			    wScrollCode = SB_LINEDOWN;
		    break;
	    }
	    if (wScrollCode != (WORD)-1) {
		ScrollbarNotify(hWnd,GetParent(hWnd),wScrollCode,0,
				lpScrollInfo->dwStyle);
		SBSSTATE(lpScrollInfo,SSF_NOTIFYSENT);
	    }
	    return 0L;

	case WM_KEYUP:
	    if (SBTSTATE(lpScrollInfo,SSF_NOTIFYSENT) &&
		(wParam >= VK_PRIOR) && (wParam <= VK_DOWN)) {
		ScrollbarNotify(hWnd,GetParent(hWnd),SB_ENDSCROLL,0,
				lpScrollInfo->dwStyle);
		SBCSTATE(lpScrollInfo,SSF_NOTIFYSENT);
	    }
	    return 0L;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	    pt.x = (int)((short)LOWORD(lParam));
	    pt.y = (int)((short)HIWORD(lParam));
	    wHit = ScrollbarHittest(lpScrollInfo,pt);
	    if (wHit) {
		switch(wHit) {
		    case SBH_HITLEFTARROW:
			if (SBTSTATE(lpScrollInfo,SSF_LEFTARROWGRAYED))
			    return 0L;
		        wState = SSF_LEFTARROW;
			wAction = ODA_LEFTARROW;
			wScrollCode = SB_LINEUP;
		        break;
		    case SBH_HITRIGHTARROW:
			if (SBTSTATE(lpScrollInfo,SSF_RIGHTARROWGRAYED))
			    return 0L;
		        wState = SSF_RIGHTARROW;
			wAction = ODA_RIGHTARROW;
			wScrollCode = SB_LINEDOWN;
		        break;
		    case SBH_HITLEFTFIELD:
			if (SBTSTATE(lpScrollInfo,SSF_LEFTARROWGRAYED))
			    return 0L;
			wState = SSF_LEFTFIELD;
			wAction = ODA_LEFTFIELD;
			wScrollCode = SB_PAGEUP;
			break;
		    case SBH_HITRIGHTFIELD:
			if (SBTSTATE(lpScrollInfo,SSF_RIGHTARROWGRAYED))
			    return 0L;
			wState = SSF_RIGHTFIELD;
			wAction = ODA_RIGHTFIELD;
			wScrollCode = SB_PAGEDOWN;
			break;
		    case SBH_HITTHUMB:
			if (SBTSTATE(lpScrollInfo,SSF_GRAYED))
			    return 0L;
			wState = SSF_THUMB|SSF_THUMBTRACK;
			wAction = ODA_THUMBTRACK;
			wScrollCode = SB_THUMBTRACK;
			lpScrollInfo->nTrackingPos = lpScrollInfo->nPos;
			lpScrollInfo->wTrackingOffset =
				lpScrollInfo->wThumbOffset;
			break;
		    default:
			return 0L;
	        }
	        SBSSTATE(lpScrollInfo,wState);
	        ScrollbarSendItemDraw(lpScrollInfo,wAction,ODS_SELECTED);
		hWndCapture = SetCapture(hWnd);
		SetSystemTimer(hWnd,IDT_FIRSTCLICK,CTICKS_FIRSTCLICK,NULL);
		ScrollbarNotify(hWnd,GetParent(hWnd),wScrollCode,
			(wHit == SBH_HITTHUMB)?lpScrollInfo->nPos:0,
			lpScrollInfo->dwStyle);
	    }
	    return 0L;

	case WM_SIZE:
	    if (SBTSTYLE(lpScrollInfo,SBS_VERT))
		lpScrollInfo->wArrowWidth = LOWORD(lParam);
	    else
		lpScrollInfo->wArrowHeight = HIWORD(lParam);
	    ScrollbarUpdate(hWnd,0,lpScrollInfo);
	    ScrollbarSendItemDraw(lpScrollInfo,ODA_DRAWENTIRE,0);
	    return 1L;

	case WM_MOUSEMOVE:
	    if (!SBTSTATE(lpScrollInfo,SSF_CLICKED))
		return 1L;
	    wAction = ScrollbarClickedRect(lpScrollInfo,&rc);
	    pt.x = (int)((short)LOWORD(lParam));
	    pt.y = (int)((short)HIWORD(lParam));
	    wState = lpScrollInfo->wState;
	    if (wAction != ODA_THUMB) {
	        if (PtInRect(&rc,pt)) {
		    SBCSTATE(lpScrollInfo,SSF_MOUSEOUT);
		    if (wState != lpScrollInfo->wState)
		    ScrollbarSendItemDraw(lpScrollInfo,wAction,ODS_SELECTED);
	        }
	        else {
		    SBSSTATE(lpScrollInfo,SSF_MOUSEOUT);
		    if (wState != lpScrollInfo->wState)
		        ScrollbarSendItemDraw(lpScrollInfo,wAction,0);
	        }
	    }
	    else {
		WORD nOldTrackingPos = lpScrollInfo->nTrackingPos;
		
		if (SBTSTATE(lpScrollInfo,SSF_THUMBTRACK))
		    ScrollbarSendItemDraw(lpScrollInfo,ODA_THUMBTRACK,0);
	    	GetClientRect(hWnd,&rc);
		if (SBTSTYLE(lpScrollInfo,SBS_VERT)) {
		    rc.top += lpScrollInfo->wArrowHeight;
		    rc.bottom -= lpScrollInfo->wArrowHeight;
		}
		else {
		    rc.left += lpScrollInfo->wArrowWidth;
		    rc.right -= lpScrollInfo->wArrowWidth;
		}
		if (PtInRect(&rc,pt)) {
	    	    lpScrollInfo->wTrackingOffset = (WORD)
			(SBTSTYLE(lpScrollInfo,SBS_VERT)) ?
			pt.y - lpScrollInfo->wArrowHeight * 3 / 2 :
			pt.x - lpScrollInfo->wArrowWidth  * 3 / 2;
		    SetTrackingPos(lpScrollInfo,&rc,FALSE);
		}
		else {
		    lpScrollInfo->nTrackingPos = lpScrollInfo->nPos;
		    lpScrollInfo->wTrackingOffset = lpScrollInfo->wThumbOffset;
		}
		ScrollbarSendItemDraw(lpScrollInfo,ODA_THUMBTRACK,
			ODS_SELECTED);
		SBSSTATE(lpScrollInfo,SSF_THUMBTRACK);

		if (nOldTrackingPos != lpScrollInfo->nTrackingPos)
		{
		    ScrollbarNotify(hWnd,GetParent(hWnd),SB_THUMBTRACK,
				    lpScrollInfo->nTrackingPos,
				    lpScrollInfo->dwStyle);
		}
	    }
	    return 1L;

	case WM_LBUTTONUP:
	    if (!SBTSTATE(lpScrollInfo,SSF_CLICKED))
		return 0L;
	    UserKillSystemTimer(hWnd,IDT_FIRSTCLICK);
	    UserKillSystemTimer(hWnd,IDT_HOLDCLICK);
	    if (hWnd == GetCapture()) {
		if (!hWndCapture)
		    ReleaseCapture();
		else {
		    SetCapture(hWndCapture);
		    hWndCapture = (HWND)0;
		}
	    }
	    if (!SBTSTATE(lpScrollInfo,SSF_MOUSEOUT)) {
		wAction = 0;
		if (SBTSTATE(lpScrollInfo,SSF_UPARROW|SSF_LEFTARROW))
		    wAction |= ODA_LEFTARROW;
		if (SBTSTATE(lpScrollInfo,SSF_DOWNARROW|SSF_RIGHTARROW))
		    wAction |= ODA_RIGHTARROW;
		if (SBTSTATE(lpScrollInfo,SSF_THUMB)) {
		    if (lpScrollInfo->wThumbOffset !=
				lpScrollInfo->wTrackingOffset) {
			GetClientRect(hWnd,&rc);
			if (SBTSTYLE(lpScrollInfo,SBS_VERT)) {
			    rc.top += lpScrollInfo->wArrowHeight;
			    rc.bottom -= lpScrollInfo->wArrowHeight;
			}
			else {
			    rc.left += lpScrollInfo->wArrowWidth;
			    rc.right -= lpScrollInfo->wArrowWidth;
			}
			lpScrollInfo->nPos = SetTrackingPos(lpScrollInfo,
							&rc,TRUE);
			ScrollbarNotify(hWnd,GetParent(hWnd),SB_THUMBPOSITION,
				lpScrollInfo->nPos,
				lpScrollInfo->dwStyle);
			wAction |= ODA_DRAWENTIRE;
		    }
		    else {
			if (SBTSTATE(lpScrollInfo,SSF_THUMBTRACK))
			    wAction |= ODA_THUMBTRACK;
		    }
		}
		if (SBTSTATE(lpScrollInfo,SSF_UPFIELD|SSF_LEFTFIELD))
		    wAction |= ODA_LEFTFIELD;
		if (SBTSTATE(lpScrollInfo,SSF_DOWNFIELD|SSF_RIGHTFIELD))
		    wAction |= ODA_RIGHTFIELD;
	        ScrollbarSendItemDraw(lpScrollInfo,wAction,0);
	    }
	    SBCSTATE(lpScrollInfo,SSF_CLICKED|SSF_MOUSEOUT|SSF_THUMBTRACK);
	    ScrollbarNotify(hWnd,GetParent(hWnd),SB_ENDSCROLL,0,
				lpScrollInfo->dwStyle);
	    return 0L;

	case WM_DRAWITEM:
	    ScrollbarDraw(lpScrollInfo,(LPDRAWITEMSTRUCT)lParam);
	    return 1L;

	case SBM_GETSCROLLPOS:
	    return (LONG)lpScrollInfo->nPos;

	case SBM_SETSCROLLPOS:
	    nPos = lpScrollInfo->nPos;
	    nNewPos = (int)lParam;
	    nNewPos = max(nNewPos,(int)lpScrollInfo->nMin);
	    nNewPos = min(nNewPos,(int)lpScrollInfo->nMax);

	    lpScrollInfo->nPos = nNewPos;

	    ScrollbarUpdate(hWnd,wParam,lpScrollInfo);

	    return (LONG)nPos;

	case SBM_GETSCROLLRANGE:
	    return MAKELONG(lpScrollInfo->nMin,lpScrollInfo->nMax);

	case SBM_SETSCROLLRANGE:
	    lpScrollInfo->nMin = LOWORD(lParam);
	    lpScrollInfo->nMax = HIWORD(lParam);

	    ScrollbarUpdate(hWnd,wParam,lpScrollInfo);
	    return 1L;

	case WM_CONVERT:
	    if (!lpScrollBinToNat) {
		hScrollClass32 = FindClass("SCROLLBAR",0);
		lpScrollBinToNat = (WNDPROC)GetClassHandleLong(
				hScrollClass32,GCL_BINTONAT);
	    }
	    if (lpScrollBinToNat)
		return lpScrollBinToNat(hWnd,wMsg,wParam,lParam);
	    else
		return (LRESULT)0;

	default:
	    return DefWindowProc(hWnd,wMsg,wParam,lParam);
    }
}

static int GetScrollPage(HWND hWnd, int nScrollBar)
{
	return (0);
}

static int SetScrollPage(HWND hWnd, int nScrollBar, int nPage, BOOL bRedraw)
{
	return (0);
}

int WINAPI
GetScrollPos(HWND hWnd, int fnBar)
{
    HWND hDest;

    if (!IsWindow(hWnd))
	return 0;
    switch(fnBar) {
	case SB_HORZ:
	    hDest = GetWindowHScroll(hWnd);
	    break;
	case SB_VERT:
	    hDest = GetWindowVScroll(hWnd);
	    break;
	case SB_CTL:
	    hDest = hWnd;
	    break;
	default:
	    return 0;
    }
    if (IsWindow(hDest))
	return SendMessage(hDest,SBM_GETSCROLLPOS,0,0L);
    else
	return 0;
}

int WINAPI
SetScrollPos(HWND hWnd, int fnBar, int nPos, BOOL fRepaint)
{
    HWND hDest;
    DWORD dwStyle;

    APISTR((LF_API,"SetScrollPos: hWnd %x, fn %x, pos %d, flag %d\n",
		hWnd,fnBar,nPos,fRepaint));

    if (!IsWindow(hWnd))
	return 0;

    switch(fnBar) {
	case SB_HORZ:
	    dwStyle = GetWindowStyle(hWnd);
	    if (!(dwStyle & WS_HSCROLL))
		fRepaint = FALSE;
	    hDest = GetWindowHScroll(hWnd);
	    break;
	case SB_VERT:
	    dwStyle = GetWindowStyle(hWnd);
	    if (!(dwStyle & WS_VSCROLL))
		fRepaint = FALSE;
	    hDest = GetWindowVScroll(hWnd);
	    break;
	case SB_CTL:
	    hDest = hWnd;
	    break;
	default:
	    return 0;
    }
    if (IsWindow(hDest))
	return SendMessage(hDest,SBM_SETSCROLLPOS,
			(WPARAM)fRepaint,(LPARAM)nPos);
    else
	return 0;
}

void WINAPI
GetScrollRange(HWND hWnd, int fnBar, LPINT lpnMinPos, LPINT lpnMaxPos)
{
    HWND hDest;
    DWORD dwScrollRange;

    if (!IsWindow(hWnd))
	return;
    switch(fnBar) {
	case SB_HORZ:
	    hDest = GetWindowHScroll(hWnd);
	    break;
	case SB_VERT:
	    hDest = GetWindowVScroll(hWnd);
	    break;
	case SB_CTL:
	    hDest = hWnd;
	    break;
	default:
	    return;
    }
    if (IsWindow(hDest)) {
	dwScrollRange =  SendMessage(hDest,SBM_GETSCROLLRANGE,0,0L);
	*lpnMinPos = LOWORD(dwScrollRange);
	*lpnMaxPos = HIWORD(dwScrollRange);
    }
    else
	return;
}

void WINAPI
SetScrollRange(HWND hWnd, int fnBar, int nMin, int nMax, BOOL fRepaint)
{
    HWND hDest;
    BOOL bSystem = FALSE;
    DWORD dwStyle, dwScroll=0;

    if (!IsWindow(hWnd))
	return;
    switch(fnBar) {
	case SB_HORZ:
	    hDest = GetWindowHScroll(hWnd);
	    bSystem = TRUE;
	    dwScroll = WS_HSCROLL;
	    break;
	case SB_VERT:
	    hDest = GetWindowVScroll(hWnd);
	    bSystem = TRUE;
	    dwScroll = WS_VSCROLL;
	    break;
	case SB_CTL:
	    hDest = hWnd;
	    break;
	default:
	    return;
    }

    if (!hDest || IsWindow(hDest)) {
	if (bSystem) {
	    dwStyle = GetWindowStyle(hWnd);
	    if (nMin == nMax) {
		if (!hDest)
		    return;
		else {
		    if (dwStyle & dwScroll)
			ShowScrollBar(hWnd,fnBar,FALSE);
		}
	    }
	    else {
		if (!(dwStyle & dwScroll)) {
		    ShowScrollBar(hWnd,fnBar,TRUE);
		    if (!hDest)
			hDest = (fnBar == SB_VERT)?
				    GetWindowVScroll(hWnd):
				    GetWindowHScroll(hWnd);
		}
		if (!IsWindow(hDest))
		    return;
	    }
	}
	else
	    if (!hDest)
		return;

	SendMessage(hDest,SBM_SETSCROLLRANGE,
		(WPARAM)fRepaint,MAKELPARAM(nMin,nMax));
    }
}

BOOL WINAPI
EnableScrollBar(HWND hWnd, int fnSBFlags, UINT suArrowFlags)
{
    HWND hDest;
    LPTWINSCROLLINFO lpScrollInfo;
    UINT uiDisableFlags;

    if (!IsWindow(hWnd))
	return FALSE;
    switch(fnSBFlags) {
	case SB_HORZ:
	    hDest = GetWindowHScroll(hWnd);
	    break;
	case SB_VERT:
	    hDest = GetWindowVScroll(hWnd);
	    break;
	case SB_CTL:
	    hDest = hWnd;
	    break;
	default:
	    return FALSE;
    }
    switch (suArrowFlags) {
	case ESB_ENABLE_BOTH:
	    uiDisableFlags = 0;
	    break;
	case ESB_DISABLE_LTUP:
	    uiDisableFlags = SSF_LEFTARROWGRAYED;
	    break;
	case ESB_DISABLE_RTDN:
	    uiDisableFlags = SSF_RIGHTARROWGRAYED;
	    break;
	case ESB_DISABLE_BOTH:
	    uiDisableFlags = SSF_GRAYED;
	    break;
	default:
	    return FALSE;
    }

    if (!(lpScrollInfo = (LPTWINSCROLLINFO)
	    GetWindowLong(hDest,SWD_LPSCROLLINFO))) {
	return FALSE;
    }


    if ((lpScrollInfo->wState & SSF_GRAYED) == (WORD)uiDisableFlags)
	return FALSE;

    lpScrollInfo->wState &= ~SSF_GRAYED;

    if (uiDisableFlags & SSF_LEFTARROWGRAYED)
	lpScrollInfo->wState |= SSF_LEFTARROWGRAYED;

    if (uiDisableFlags & SSF_RIGHTARROWGRAYED)
	lpScrollInfo->wState |= SSF_RIGHTARROWGRAYED;

    if (fnSBFlags == SB_CTL)
	InvalidateRect(hDest,NULL,FALSE);
    else 
	TWIN_RedrawWindow(hWnd,NULL,0,RDW_FRAME|RDW_INVALIDATE);

    return TRUE;
}

BOOL	WINAPI
GetScrollInfo(HWND hWnd, int nScrollBar, LPSCROLLINFO lpScrollInfo)
{
	BOOL bGotScrollInfo;

	APISTR((LF_API, "GetScrollInfo:"
		" (API) hWnd %x nScrollBar %s lpScrollInfo %p\n",
		hWnd,
		(nScrollBar == SB_CTL) ? "SB_CTL" :
		(nScrollBar == SB_HORZ) ? "SB_HORZ" :
		(nScrollBar == SB_VERT) ? "SB_VERT" :
		"SB_???",
		lpScrollInfo));

	if (!lpScrollInfo)
		return (FALSE);

	bGotScrollInfo = FALSE;
	if (lpScrollInfo->fMask & SIF_PAGE)
	{
		lpScrollInfo->nPage = GetScrollPage(hWnd, nScrollBar);
		bGotScrollInfo = TRUE;
	}
	if (lpScrollInfo->fMask & SIF_POS)
	{
		lpScrollInfo->nPos = GetScrollPos(hWnd, nScrollBar);
		bGotScrollInfo = TRUE;
	}
	if (lpScrollInfo->fMask & SIF_RANGE)
	{
		GetScrollRange(hWnd, nScrollBar,
			&lpScrollInfo->nMin, &lpScrollInfo->nMax);
		bGotScrollInfo = TRUE;
	}
	return (bGotScrollInfo);

}

int	WINAPI
SetScrollInfo(HWND hWnd, int nScrollBar, LPSCROLLINFO lpScrollInfo,
	BOOL bRedraw)
{
	int nScrollPos;

	APISTR((LF_API, "SetScrollInfo:"
		" (API) hWnd %x nScrollBar %s lpScrollInfo %p bRedraw %d\n",
		hWnd,
		(nScrollBar == SB_CTL) ? "SB_CTL" :
		(nScrollBar == SB_HORZ) ? "SB_HORZ" :
		(nScrollBar == SB_VERT) ? "SB_VERT" :
		"SB_???",
		lpScrollInfo,
		bRedraw));

	if (!lpScrollInfo)
		return (0);

	nScrollPos = GetScrollPos(hWnd, nScrollBar);
	if (lpScrollInfo->fMask & SIF_DISABLENOSCROLL)
		EnableScrollBar(hWnd, nScrollBar, ESB_DISABLE_BOTH);
	if (lpScrollInfo->fMask & SIF_PAGE)
		SetScrollPage(hWnd, nScrollBar, lpScrollInfo->nPage, bRedraw);
	if (lpScrollInfo->fMask & SIF_POS)
		nScrollPos = SetScrollPos(hWnd, nScrollBar, lpScrollInfo->nPos,
			bRedraw);
	if (lpScrollInfo->fMask & SIF_RANGE)
		SetScrollRange(hWnd, nScrollBar,
			lpScrollInfo->nMin, lpScrollInfo->nMax, bRedraw);
	return (nScrollPos);

}

/* Supplemental routines */
static void
ScrollbarNotify(HWND hWnd, HWND hWndParent, WORD wScrollCode,
		int nPos, DWORD dwStyle)
{
    WORD wMsg;

    wMsg = (dwStyle & SBS_VERT)?WM_VSCROLL:WM_HSCROLL;
/* this assumes that GET_WM_HSCROLL_MPS and GET_WM_VSCROLL_MPS are the same */
    SendMessage(hWndParent,wMsg,GET_WM_HSCROLL_MPS(wScrollCode,nPos,
		(dwStyle & SBS_SYSTEM)?0:hWnd));
}

static LONG
ScrollbarCreate(HWND hWnd, LPCREATESTRUCT lpCrS)
{
    HANDLE hScroll;
    LPTWINSCROLLINFO lpScrollInfo;

    if (!(lpScrollInfo = CREATESBAR(hScroll))) {
	return 0L;
    }
    SetWindowLong(hWnd,SWD_LPSCROLLINFO,(LONG)lpScrollInfo);
    lpScrollInfo->dwStyle = lpCrS->style;
    lpScrollInfo->hWnd = hWnd;
    lpScrollInfo->wArrowWidth = (!SBTSTYLE(lpScrollInfo,SBS_VERT))?
	GetSystemMetrics(SM_CXHSCROLL):
	lpCrS->cx;
    lpScrollInfo->wArrowHeight = (SBTSTYLE(lpScrollInfo,SBS_VERT))?
	GetSystemMetrics(SM_CYVSCROLL):
	lpCrS->cy;
    lpScrollInfo->nMin = 0;  /* MiD 19-DEC-1995: assign default scroll range values */
    lpScrollInfo->nMax = 100;
    return (LONG)hScroll;
}

static int
ScrollbarHittest(LPTWINSCROLLINFO lpScrollInfo, POINT pt)
{
    RECT rc;
    int nXbound, nYbound;

    GetClientRect(lpScrollInfo->hWnd,&rc);

    if (!PtInRect(&rc,pt))
	return SBH_NULL;

    if (!SBTSTYLE(lpScrollInfo,SBS_VERT)) {
	nXbound = lpScrollInfo->wArrowWidth;
	if (pt.x < nXbound)
	    return SBH_HITLEFTARROW;

	if((int)(lpScrollInfo->wThumbOffset+2*lpScrollInfo->wArrowWidth-2) <
		rc.right - (int)lpScrollInfo->wArrowWidth) {

		nXbound += lpScrollInfo->wThumbOffset;
		if (pt.x < nXbound)
		    return SBH_HITLEFTFIELD;

		nXbound += lpScrollInfo->wArrowWidth;
		if (pt.x < nXbound)
		    return SBH_HITTHUMB;

		nXbound = rc.right-lpScrollInfo->wArrowWidth;
		if (pt.x < nXbound)
		    return SBH_HITRIGHTFIELD;
	}

	if (pt.x > rc.right - (int)lpScrollInfo->wArrowWidth)
	    return SBH_HITRIGHTARROW;
    } else {
	nYbound = lpScrollInfo->wArrowHeight;
	if (pt.y < nYbound)
	    return SBH_HITUPARROW;

	/* see if its in the field or thumb */
	if((int)(lpScrollInfo->wThumbOffset+2*lpScrollInfo->wArrowHeight-2) <
		rc.bottom - (int)lpScrollInfo->wArrowHeight) {

		nYbound += lpScrollInfo->wThumbOffset;
		if (pt.y < nYbound)
		    return SBH_HITUPFIELD;

		nYbound += lpScrollInfo->wArrowHeight;
		if (pt.y < nYbound)
		    return SBH_HITTHUMB;

		nYbound = rc.bottom-lpScrollInfo->wArrowHeight;
		if (pt.y < nYbound)
		    return SBH_HITDOWNFIELD;
	}

	/* check for the down arrow */
	if (pt.y > rc.bottom - (int)lpScrollInfo->wArrowHeight)
	    return SBH_HITDOWNARROW;

    }

    return SBH_NULL; 
}

static WORD
ScrollbarClickedRect(LPTWINSCROLLINFO lpScrollInfo, LPRECT lpRect)
{
    WORD wOffset;
    WORD wArrow;
    RECT rcClient;

    if (!SBTSTATE(lpScrollInfo,SSF_CLICKED)) {
	SetRectEmpty(lpRect);
	return 0;
    }

    GetClientRect(lpScrollInfo->hWnd,&rcClient);
    CopyRect(lpRect,&rcClient);

    wArrow = (SBTSTYLE(lpScrollInfo,SBS_VERT))?
	lpScrollInfo->wArrowHeight:lpScrollInfo->wArrowWidth;

    wOffset = 0;
    if (!SBTSTYLE(lpScrollInfo,SBS_VERT)) {
	if (SBTSTATE(lpScrollInfo,SSF_LEFTARROW)) {
	    lpRect->left = wOffset;
	    lpRect->right = wOffset+wArrow;
	    return ODA_LEFTARROW;
        }
	wOffset += wArrow;
	if (SBTSTATE(lpScrollInfo,SSF_LEFTFIELD)) {
	    lpRect->left = wOffset;
	    lpRect->right = wOffset+lpScrollInfo->wThumbOffset;
	    return ODA_LEFTFIELD;
	}
	wOffset += lpScrollInfo->wThumbOffset;
	if (SBTSTATE(lpScrollInfo,SSF_THUMB)) {
	    lpRect->left = wOffset;
	    lpRect->right = wOffset+wArrow;
	    return ODA_THUMB;
	}
	wOffset += wArrow;
	if (SBTSTATE(lpScrollInfo,SSF_RIGHTFIELD)) {
	    lpRect->left = wOffset;
	    lpRect->right = rcClient.right-wArrow;
	    return ODA_RIGHTFIELD;
	}
	wOffset = rcClient.right-wArrow;
	if (SBTSTATE(lpScrollInfo,SSF_RIGHTARROW)) {
	    lpRect->left = wOffset;
	    lpRect->right = rcClient.right;
	    return ODA_RIGHTARROW;
	}
    }
    else {
	if (SBTSTATE(lpScrollInfo,SSF_UPARROW)) {
	    lpRect->top = wOffset;
	    lpRect->bottom = wOffset+wArrow;
	    return ODA_UPARROW;
        }
	wOffset += wArrow;
	if (SBTSTATE(lpScrollInfo,SSF_UPFIELD)) {
	    lpRect->top = wOffset;
	    lpRect->bottom = wOffset+lpScrollInfo->wThumbOffset;
	    return ODA_UPFIELD;
	}
	wOffset += lpScrollInfo->wThumbOffset;
	if (SBTSTATE(lpScrollInfo,SSF_THUMB)) {
	    lpRect->top = wOffset;
	    lpRect->bottom = wOffset+wArrow;
	    return ODA_THUMB;
	}
	wOffset += wArrow;
	if (SBTSTATE(lpScrollInfo,SSF_DOWNFIELD)) {
	    lpRect->top = wOffset;
	    lpRect->bottom = rcClient.bottom-wArrow;
	    return ODA_DOWNFIELD;
	}
	wOffset = rcClient.right-wArrow;
	if (SBTSTATE(lpScrollInfo,SSF_DOWNARROW)) {
	    lpRect->top = wOffset;
	    lpRect->bottom = rcClient.bottom;
	    return ODA_DOWNARROW;
	}
    }

    return 0;
}

static int
SetTrackingPos(LPTWINSCROLLINFO lpScrollInfo, LPRECT lprc, BOOL fSetOffset)
{
    int nDiff, nRange;

    nRange = lpScrollInfo->nMax - lpScrollInfo->nMin;
    nDiff = (SBTSTYLE(lpScrollInfo,SBS_VERT))?
		lprc->bottom - lprc->top - lpScrollInfo->wArrowHeight:
		lprc->right - lprc->left - lpScrollInfo->wArrowWidth;

    if ( (short)lpScrollInfo->wTrackingOffset < 0 )
        lpScrollInfo->wTrackingOffset = 0;
    else if ( lpScrollInfo->wTrackingOffset > nDiff )
        lpScrollInfo->wTrackingOffset = nDiff;

    if (nRange <= 0 || nDiff <= 0)
	lpScrollInfo->nTrackingPos = 0;
    else 
	lpScrollInfo->nTrackingPos =
		MulDiv(lpScrollInfo->wTrackingOffset, nRange, nDiff);

    if (fSetOffset)
	lpScrollInfo->wThumbOffset = 
	    MulDiv(lpScrollInfo->nTrackingPos, nDiff, nRange);

    return lpScrollInfo->nTrackingPos;
}
