/*    
	DefWnd.c	2.19
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
#include "kerndef.h"
#include "Log.h"
#include "GdiDC.h"
#include <stdio.h>

/* external stuff */
extern BOOL TestWF(HWND, DWORD);
extern void ClearWF(HWND, DWORD);
extern void SetWF(HWND, DWORD);
extern BOOL IsTopLevel(HWND);
extern LONG InternalSetText(HWND, LPSTR);
extern int  InternalGetText(HWND, LPSTR, int);
extern int  InternalGetTextLength(HWND);
extern void InternalFreeText(HWND);
extern void CalcClientRect(HWND, LPRECT);
extern void GetNCRect(HWND, LPRECT);
extern HBRUSH GetClassBackgroundBrush(HWND);
extern void RedrawIconTitle(HWND);
extern void ShowIconTitle(HWND, BOOL);
extern void DrawCaption(HWND, HDC, BOOL, BOOL);
extern void DrawWindowFrame(HWND, BOOL);
extern int  DoNCHitTest(HWND, POINT);
extern LONG DoNCMouse(HWND, UINT, int, POINT);
extern void DoSysCommand(HWND, WPARAM, LPARAM);
extern void LBoxDefaultDrawItem(LPDRAWITEMSTRUCT);
extern void LBoxDefaultMeasureItem(HWND, LPMEASUREITEMSTRUCT);
extern int LBoxDefaultCompareItem(LPCOMPAREITEMSTRUCT);
extern LRESULT TWIN_CancelMode(HWND);

/* API functions */

LRESULT WINAPI
DefWindowProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    HDC		hDC;
    HBRUSH	hBr;
    HICON	hIcon;
    HWND	hWndParent,hWndFocus;
    HCURSOR	hCursor=(HCURSOR)NULL;
    LRESULT	rc;
    PAINTSTRUCT ps;
    WORD	wWord;
    DWORD	dwClassStyle;
    RECT	rcClient,rcNC;
    WNDPROC lpfnBinToNat;
    POINT pt;
    DWORD dwStyle;
    LPWINDOWPOS lpwp;

    APISTR((LF_APICALL,"DefWindowProc(HWND=%x,UINT=%x,WPARAM=%x,LPARAM=%x)\n",
	hWnd,wMsg, wParam,lParam));

    if(!IsWindow(hWnd)) {
         APISTR((LF_APIFAIL,"DefWindowProc: returns LRESULT 0\n"));
	return((LONG)NULL);
    }

    switch(wMsg) {
	case WM_NCACTIVATE:
	    if (wParam)
		SetWF(hWnd, WFFRAMEON);
	    else
		ClearWF(hWnd, WFFRAMEON);

	    dwStyle = GetWindowLong(hWnd,GWL_STYLE);
	    if ((dwStyle & WS_VISIBLE) && !TestWF(hWnd, WFNONCPAINT)) {
		DrawWindowFrame(hWnd, FALSE);
		if (IsIconic(hWnd))
		    RedrawIconTitle(hWnd);
	    }
            APISTR((LF_APIRET,"DefWindowProc: returns LRESULT 1\n"));
	    return (LRESULT)TRUE;

	case WM_ISACTIVEICON:
	    if (IsIconic(hWnd) && TestWF(hWnd,WFFRAMEON)) {
                APISTR((LF_APIRET,"DefWindowProc: returns LRESULT 1\n"));
		return (LONG)1;
	    } else {
                APISTR((LF_APIRET,"DefWindowProc: returns LRESULT 0\n"));
		return (LONG)NULL;
	    }

	case WM_NCCALCSIZE:
	    CalcClientRect(hWnd, (LPRECT)lParam);
	    break;

	case WM_NCCREATE:
	    rc = (LRESULT)InternalSetText(hWnd,
			(LPSTR)(((LPCREATESTRUCT)lParam)->lpszName));
            APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",rc));
	    return rc;

	case WM_NCDESTROY:
	    InternalFreeText(hWnd);
	    break;

	case WM_NCPAINT:
	    DrawWindowFrame(hWnd, TRUE);
	    break;

	case WM_NCHITTEST:
	    pt.x = (int)(short)LOWORD(lParam);
	    pt.y = (int)(short)HIWORD(lParam);
	    rc = (LRESULT)DoNCHitTest(hWnd,pt);
            APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",rc));
	    return rc;

	case WM_NCMOUSEMOVE:
	case WM_NCLBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCLBUTTONDBLCLK:
	case WM_NCRBUTTONDOWN:
	case WM_NCRBUTTONUP:
	case WM_NCRBUTTONDBLCLK:
	case WM_NCMBUTTONDOWN:
	case WM_NCMBUTTONUP:
	case WM_NCMBUTTONDBLCLK:
#ifdef SEVERE
	    printf("NCxxx message: about to call DoNCMouse\n");
#endif
	    pt.x = (int)(short)LOWORD(lParam);
	    pt.y = (int)(short)HIWORD(lParam);
	    rc =  DoNCMouse(hWnd,wMsg,(int)wParam,pt);
            APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",rc));
	    return rc;

	case WM_CANCELMODE:
	    rc = TWIN_CancelMode(hWnd);
            APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",rc));
	    return rc;
#ifdef	LATER
	cancel scrollbar tracking, menu, button capture etc.
#endif

	case WM_SETTEXT:
	    InternalSetText(hWnd, (LPSTR)lParam);
	    dwStyle = GetWindowLong(hWnd,GWL_STYLE);
	    if (dwStyle & WS_VISIBLE) {
		if (IsIconic(hWnd)) {
		    ShowIconTitle(hWnd, FALSE);
		    ShowIconTitle(hWnd, TRUE);
		}
		else
		    if ((dwStyle & WS_CAPTION) == WS_CAPTION) {
			hDC = GetWindowDC(hWnd);
			DrawCaption(hWnd,hDC,FALSE,TestWF(hWnd, WFFRAMEON));
			ReleaseDC(hWnd, hDC);
		    }
	    }
	    break;

	case WM_GETTEXT:
	    if (wParam) {
		rc =  (LRESULT)InternalGetText(hWnd,(LPSTR)lParam,wParam);
                APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",rc));
	        return rc;
	    }
	    break;
 
	case WM_GETTEXTLENGTH:
	    rc =  (LRESULT)InternalGetTextLength(hWnd);
            APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",rc));
	    return rc;

	case WM_CLOSE:
	    DestroyWindow(hWnd);
	    break;

	case WM_MOVE:
	    if (IsIconic(hWnd)) 
		RedrawIconTitle(hWnd);
	    break;

	case WM_SYSCOMMAND:
	    DoSysCommand(hWnd,wParam,lParam);
	    break;

	case WM_PAINT:
	    BeginPaint(hWnd, (LPPAINTSTRUCT)&ps);
	    EndPaint(hWnd, (LPPAINTSTRUCT)&ps);
	    break;

	case WM_PAINTICON:
	    BeginPaint(hWnd, (LPPAINTSTRUCT)&ps);

	    /*  Draw the icon through the window DC if app used own DC.
		If own DC is used the mapping mode may not be MM_TEXT.
	    */
	    dwClassStyle = GetClassStyle(hWnd);
	    if (dwClassStyle & (CS_OWNDC | CS_CLASSDC)) {
		/* If owndc, do the end paint now so that the
		 * erasebackgrounds/validate regions go through properly. Then
		 * we get a clean window dc to draw the icon into.
		 */
		EndPaint(hWnd, (LPPAINTSTRUCT)&ps);
		hDC = GetWindowDC(hWnd);
	    }
	    else {
		hDC = ps.hdc;
	    }
 
	    /* wParam is TRUE to draw icon, FALSE to ignore paint. */
	    if (wParam) {
		hIcon = GetClassIcon(hWnd);
		GetClientRect(hWnd,&rcClient);
 
		rcClient.left = (rcClient.right -
				 GetSystemMetrics(SM_CXICON)) >> 1;
		rcClient.top = (rcClient.bottom - 
				 GetSystemMetrics(SM_CYICON)) >> 1;
        
		DrawIcon(hDC, rcClient.left, rcClient.top, hIcon);
	    }

	    /* Delete the update region. */
	    if (dwClassStyle & (CS_OWNDC | CS_CLASSDC)) {
		ReleaseDC(hWnd, hDC);
		ValidateRect(hWnd, NULL);
	    }
	    else
		EndPaint(hWnd, (LPPAINTSTRUCT)&ps);
	    break;
        
	case WM_ERASEBKGND:
	    hBr = GetClassBackgroundBrush(hWnd);
	    if (hBr) {
		GetClientRect(hWnd, &rcClient);
		FillRect((HDC)wParam, &rcClient, hBr);
                APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",1));
		return (LRESULT)1;
	    }
	    break;

	case WM_ICONERASEBKGND:
	    /* iconized windows do not have non-client area (today) */
	    /* if they did, we would have called GetDCEx with DCX_WINDOW */
	    hDC = (HDC) GetDCEx(hWnd,(HRGN)0,DCX_CACHE | DCX_INTERSECTUPDATE);
	    if (!IsTopLevel(hWnd)) {
		if (!(hBr = GetClassBackgroundBrush(GetParent(hWnd)))) {
		    ReleaseDC(hWnd,hDC);
                    APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",1));
		    return (LRESULT)1;
		}
		pt.x = pt.y = 0;
		ClientToScreen(GetParent(hWnd),&pt);
		SetBrushOrg(hDC,pt.x,pt.y);
		UnrealizeObject(hBr);
		GetClientRect(hWnd,&rcClient);
		FillRect(hDC,&rcClient,hBr);
	    }

	    ReleaseDC(hWnd,hDC);
            APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",1));
	    return (LRESULT)1;

	case WM_SETREDRAW:
	    dwStyle = GetWindowLong(hWnd,GWL_STYLE);
	    if (wParam)
		dwStyle |= WS_VISIBLE;
	    else
		dwStyle &= ~WS_VISIBLE;
	    SetWindowLong(hWnd,GWL_STYLE,dwStyle);
	    break;

	case WM_ACTIVATE:
	    if (GET_WM_ACTIVATE_STATE(wParam,lParam)) {
		if (!(hWndFocus = GetDialogFocus(hWnd)))
		    hWndFocus = hWnd;
		SetFocus(hWndFocus);
	    }
	    break;

	case WM_MOUSEACTIVATE:
	    /* First give the parent a chance to process the message */
	    if ((hWndParent = GetParent(hWnd))) {
		rc = SendMessage(hWndParent,WM_MOUSEACTIVATE,
				wParam,lParam);
		if (rc) {
                    APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",rc));
		    return rc;
	        }
	    }
	    /*  If the user clicked in the title bar, don't activate now:
		the activation will take place later when the move or size
		occurs. */
	    if ((int)(short)LOWORD(lParam) == HTCAPTION) {
		rc = (LRESULT)MA_NOACTIVATE;
                APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",rc));
		return rc;
	    }

	    rc =  (LRESULT)MA_ACTIVATE;
            APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",rc));
	    return rc;

	case WM_SHOWWINDOW:
          /* Non null descriptor implies popup hide or show. */
          /* We should check whether a popup window or Owned window */
	    dwStyle = GetWindowLong(hWnd,GWL_STYLE);
	    if ((LOWORD(lParam) != 0) &&
		((dwStyle & WS_POPUP) ||
		(!(dwStyle & WS_CHILD) && GetWindow(hWnd, GW_OWNER)))) {
              /* IF NOT(showing, invisible, and not set as hidden) AND
               *   NOT(hiding and not visible)
               */
		if (!(wParam != 0 && !(dwStyle & WS_VISIBLE) &&
			!TestWF(hWnd, WFHIDDENPOPUP)) &&
			!(wParam == 0 && !(dwStyle & WS_VISIBLE))) {
			/* Are we showing? */
		    if (wParam)
			/* Yes, clear the hidden popup flag. */
			ClearWF(hWnd, WFHIDDENPOPUP);
		    else
			/* No, Set WFHIDDENPOPUP */
			SetWF(hWnd, WFHIDDENPOPUP);

		    ShowWindow(hWnd,
                        (wParam ? SW_SHOWNOACTIVATE : SW_HIDE));
                }
            }
	break;

#ifndef	TWIN32
	case WM_CTLCOLOR:
	    if (GET_WM_CTLCOLOR_TYPE(wParam,lParam,wMsg)
			!= CTLCOLOR_SCROLLBAR) {
		SetBkColor(GET_WM_CTLCOLOR_HDC(wParam,lParam,wMsg),
			GetSysColor(COLOR_WINDOW));
		SetTextColor(GET_WM_CTLCOLOR_HDC(wParam,lParam,wMsg),
			GetSysColor(COLOR_WINDOWTEXT));
		hBr = GetSysColorBrush(COLOR_WINDOW);
	    }
	    else {
		SetBkColor(GET_WM_CTLCOLOR_HDC(wParam,lParam,wMsg),
			RGB(255, 255, 255));
		SetTextColor(GET_WM_CTLCOLOR_HDC(wParam,lParam,wMsg),
			RGB(0, 0, 0));
		hBr = GetSysColorBrush(COLOR_SCROLLBAR);
	    }
            APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",hBr));
	    return (LRESULT)hBr;
#else
	case WM_CTLCOLORSCROLLBAR:
	    SetBkColor(GET_WM_CTLCOLOR_HDC(wParam,lParam,wMsg),
			RGB(255, 255, 255));
	    SetTextColor(GET_WM_CTLCOLOR_HDC(wParam,lParam,wMsg),
			RGB(0, 0, 0));
	    hBr = GetSysColorBrush(COLOR_SCROLLBAR);
            APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",hBr));
	    return (LRESULT)hBr;

	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
	    SetBkColor(GET_WM_CTLCOLOR_HDC(wParam,lParam,wMsg),
			GetSysColor(COLOR_WINDOW));
	    SetTextColor(GET_WM_CTLCOLOR_HDC(wParam,lParam,wMsg),
			GetSysColor(COLOR_WINDOWTEXT));
	    hBr = GetSysColorBrush(COLOR_WINDOW);
            APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",hBr));
	    return (LRESULT)hBr;
#endif

	case WM_SETCURSOR: {
	    dwStyle = GetWindowLong(hWnd,GWL_STYLE);
	    if (dwStyle & WS_CHILD) {
		rc = SendMessage(GetParent(hWnd),WM_SETCURSOR,wParam,lParam);
		if (rc) {
                    APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",1));
		    return TRUE;
	        }
	    }
	    switch ( (int)(short)LOWORD(lParam) ) {
		case HTCLIENT:
			hCursor = GetClassCursor((HWND)wParam);
			break;
		case HTMENU:
		case HTHSCROLL:
		case HTVSCROLL:
			hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
			break;
		}
		if ( hCursor ) 
			SetCursor(hCursor);
                APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",1));
		return TRUE;
	}
	break;

	case WM_WINDOWPOSCHANGING:
	    dwStyle = GetWindowLong(hWnd,GWL_STYLE);
	    if ((dwStyle & WS_THICKFRAME) ||
		!(dwStyle & (WS_POPUP|WS_CHILD))) {
#ifdef	LATER
	Send WM_GETMINMAXINFO
#endif
	    }
	    break;

	case WM_WINDOWPOSCHANGED:
	    lpwp = (LPWINDOWPOS)lParam;
	    GetNCRect(hWnd,&rcNC);

	    if (!(lpwp->flags & SWP_NOMOVE))
#ifdef	LATER
	/* figure out how to calculate the correct origin */
#endif
		SendMessage(hWnd, WM_MOVE, 0,
		    MAKELPARAM(lpwp->x+rcNC.left,lpwp->y+rcNC.top));

	    if (!(lpwp->flags & SWP_NOSIZE)) {
		if (IsZoomed(hWnd))
		    wWord = SIZE_MAXIMIZED;
		else
		    wWord = (IsIconic(hWnd))?
			SIZE_MINIMIZED:SIZE_RESTORED;
		GetClientRect(hWnd,&rcClient);
		SendMessage(hWnd, WM_SIZE, (WPARAM)wWord,
			MAKELPARAM(rcClient.right,rcClient.bottom));
	    }
	    break;

	case WM_QUERYOPEN:
	case WM_QUERYENDSESSION:
            APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",1));
	    return (LRESULT)1;

	case WM_CHARTOITEM:
	case WM_VKEYTOITEM:
            APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",-1));
	    return (LRESULT)-1L;

	case WM_DRAWITEM:
	    if (((LPDRAWITEMSTRUCT)lParam)->CtlType == ODT_LISTBOX)
		LBoxDefaultDrawItem((LPDRAWITEMSTRUCT)lParam);
	    break;

	case WM_MEASUREITEM:
	    if (((LPMEASUREITEMSTRUCT)lParam)->CtlType == ODT_LISTBOX)
		/* hWnd is a owner's handle in this case */
		LBoxDefaultMeasureItem(
			GetDlgItem(hWnd,((LPMEASUREITEMSTRUCT)lParam)->CtlID),
			(LPMEASUREITEMSTRUCT)lParam);
	    break;

	case WM_COMPAREITEM:
	    if (((LPCOMPAREITEMSTRUCT)lParam)->CtlType == ODT_LISTBOX) {
		rc = (LRESULT)LBoxDefaultCompareItem(
				(LPCOMPAREITEMSTRUCT)lParam);
                APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",rc));
		return rc;
	    }
	    break;

	case WM_CONVERT:
	    lpfnBinToNat = (WNDPROC)GetClassLong(hWnd,GCL_BINTONAT);
	    if (lpfnBinToNat) {
	        rc = lpfnBinToNat(hWnd,wMsg,wParam,lParam);
                APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",rc));
		return rc;
	    } else
	        FatalAppExit(0,"no BINTONAT routine. Can't continue...");

    }
    APISTR((LF_APIRET,"DefWindowProc: returns LRESULT %d\n",0));
    return (LRESULT)0;
}

/* Supplemental routines */

LRESULT
DefROOTProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    LOGSTR((LF_API,"DefROOTProc(hWnd=%.04x,wMsg=%.04x,wParam=%x,lParam=%x)\n",
	hWnd,wMsg,wParam,lParam));

    return(1L);
}
