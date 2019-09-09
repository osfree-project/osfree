/*    
	Static.c	2.18
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
#include "Static.h"

/* external functions */
extern HBRUSH GetControlBrush(HWND, HDC, WORD);
extern LONG InternalSetText(HWND, LPSTR);

/* exported functions */
LRESULT DefSTATICProc(HWND,UINT,WPARAM,LPARAM);

/* internal functions */
static void StaticPaint(HWND);

LRESULT
DefSTATICProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    WORD wStatStyle;
    HICON hIcon;
    LPCREATESTRUCT lpcs;
    LPSTR lpIconName;

    wStatStyle = (WORD)GetWindowLong(hWnd, GWL_STYLE) & 0x000f;

    switch(wMsg) {
	case WM_CREATE:
	    if (wStatStyle != SS_ICON || !(lpcs = (LPCREATESTRUCT)lParam))
		return (LRESULT)0;
	    if (lpcs->lpszName) {
		lpIconName = (*(LPWORD)&lpcs->lpszName[0] == 0xffff)?
		    (LPSTR)MAKEINTRESOURCE(*(LPWORD)&lpcs->lpszName[2]):
		    (LPSTR)lpcs->lpszName;
		if (!(hIcon = LoadIcon(GetWindowInstance(hWnd),
			lpIconName)))
		    hIcon = LoadIcon((HINSTANCE)0,lpIconName);
		SetWindowWord(hWnd,SWD_HICON,hIcon);
	    }
	    MoveWindow(hWnd,lpcs->x,lpcs->y,
		GetSystemMetrics(SM_CXICON),
		GetSystemMetrics(SM_CYICON),
		FALSE);
	    return (LRESULT)0;

	case WM_DESTROY:
	    if (wStatStyle != SS_ICON)
		return 0L;
	    if ((hIcon = GetWindowWord(hWnd,SWD_HICON)) &&
		!GetWindowWord(hWnd,SWD_FLAGS))
		DestroyIcon(hIcon);
	    return (LRESULT)0;

	case WM_ENABLE:
	    break;

	case WM_ERASEBKGND:
	    return 1L;

	case WM_PAINT:
	    StaticPaint(hWnd);
	    return 0L;

	case WM_GETTEXTLENGTH:
	    if (IsTextStatic(wStatStyle))
		break;	/* pass to DefWindowProc */
	    else
		return (LRESULT)0;

	case WM_SETTEXT:
	    if (IsTextStatic(wStatStyle)) {
        	InternalSetText(hWnd, (LPSTR)lParam);
		RedrawWindow(hWnd,(const RECT *)0,(HRGN)0,
		    RDW_INVALIDATE|RDW_UPDATENOW);
	    }
	    return (LRESULT)0;

	case WM_GETFONT:
	    if (IsTextStatic(wStatStyle))
		return (LONG)GetWindowWord(hWnd, SWD_HFONT);
	    else
		return (LRESULT)0;

	case WM_SETFONT:
	    if (IsTextStatic(wStatStyle)) {
		SetWindowWord(hWnd, SWD_HFONT, wParam);
		if (lParam)
		    RedrawWindow(hWnd,(const RECT *)0,(HRGN)0,
			RDW_INVALIDATE|RDW_UPDATENOW);
	    }
	    else
		return (LRESULT)0;

	case WM_NCCREATE:
	    if (IsTextStatic(wStatStyle))
		break; /* pass to DefWindowProc */
	    else
		return (LRESULT)1;

	case WM_NCDESTROY:
	    if (IsTextStatic(wStatStyle))
		break; /* pass to DefWindowProc */
	    else
		return (LRESULT)0;

	case WM_NCHITTEST:
	    return (LRESULT)HTTRANSPARENT;

	case WM_GETDLGCODE:
	    return (LONG)DLGC_STATIC;

	case STM_GETICON:
	    if (wStatStyle == SS_ICON)
		return (LRESULT)GetWindowWord(hWnd,SWD_HICON);
	    else
		return (LRESULT)0;

	case STM_SETICON:
	    if (wStatStyle != SS_ICON)
		return (LRESULT)0;
	    hIcon = (HICON)SetWindowWord(hWnd, SWD_HICON, (WORD)wParam);
	    SetWindowWord(hWnd,SWD_FLAGS,1);
	    SetWindowPos(hWnd,(HWND)0,0,0,
		GetSystemMetrics(SM_CXICON),
		GetSystemMetrics(SM_CYICON),
		SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
	    RedrawWindow(hWnd,(const RECT *)0,(HRGN)0,
		RDW_INVALIDATE|RDW_UPDATENOW);
	    return (LRESULT)hIcon;

    }
    return DefWindowProc(hWnd, wMsg, wParam, lParam);
}

/* Supplemental routines */

static void
StaticPaint(HWND hWnd)
{
    WORD wDtAttrs;
    HBRUSH hBr = 0;
    HFONT hFont;
    LPSTR lpszWindowText;
    char szBuf[80];
    int nTextLength;
    DWORD dwStyle;
    WORD wStyle;
    HICON hIcon;
    PAINTSTRUCT ps;
    HDC hDC;
    RECT rcClient;
    TEXTMETRIC tm;
    int dx, dy;    /* Indenting for certain justifications */

    dwStyle = GetWindowLong(hWnd, GWL_STYLE);

    if (!(dwStyle & WS_VISIBLE)) {
	ValidateRgn(hWnd,(HRGN)0);
	return;
    }

    wStyle = LOWORD(dwStyle);

    hDC = BeginPaint(hWnd,&ps);
    GetClientRect(hWnd,&rcClient);
    IntersectClipRect(hDC,0,0,rcClient.right,rcClient.bottom);

    /* Calculate the offsets so that the characters */
    /*  will not hit the top of the text box when drawn. */
    GetTextMetrics(hDC, &tm);
    dy = 0;		      /* do not offset y, see editcntl test program */
    dx = 0;		      /* we may offset x, it is flush left now      */
#ifdef	LATER
    dx = tm.tmAveCharWidth / 2;
#endif


    switch(wStyle & 0xf) {
	case SS_LEFT:
	case SS_CENTER:
	case SS_RIGHT:
	case SS_LEFTNOWORDWRAP:
	    wDtAttrs = (wStyle & SS_NOPREFIX)?DT_NOPREFIX:0;
	    if (wStyle & SS_VCENTER){
	      wDtAttrs |= DT_VCENTER;
	      dy = 0;
	    }
	    wDtAttrs |= DT_EXPANDTABS;
	    switch(wStyle) {
		case SS_LEFT:
		   wDtAttrs |= DT_LEFT | DT_WORDBREAK;
		   break;
		case SS_CENTER:
		   wDtAttrs |= DT_CENTER | DT_WORDBREAK;
		   dx = 0;     /* Remove indenting */
		   break;
		case SS_RIGHT:
		   wDtAttrs |= DT_RIGHT | DT_WORDBREAK;
		   dx *= -1;  /* Apply indenting to right side */
		   break;
		case SS_LEFTNOWORDWRAP:
		   wDtAttrs |= DT_LEFT | DT_EXPANDTABS;
		   break;
	    }

	    hBr = GetControlBrush(hWnd, hDC, CTLCOLOR_STATIC);
	    FillRect(hDC, &ps.rcPaint, hBr);

	    if ((nTextLength = GetWindowTextLength(hWnd)) > 0) {
		SetBkMode(hDC, TRANSPARENT);
		SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT)); 

		if ((hFont = GetWindowWord(hWnd, SWD_HFONT)))
		    hFont = SelectObject(hDC, hFont);

		lpszWindowText = (nTextLength < sizeof(szBuf))?
			szBuf:(LPSTR)WinMalloc(nTextLength+1);

		GetWindowText(hWnd, lpszWindowText, nTextLength+1);

		/* Adjust the rect to control indenting.    */
		/* OffsetRect(&rcClient, dx, dy);	    */

		DrawText(hDC, lpszWindowText, -1, &rcClient, wDtAttrs);

		if (nTextLength >= sizeof(szBuf))
		    WinFree(lpszWindowText);

		if (hFont)
		    SelectObject(hDC, hFont);
	    }
	    break;

	case SS_SIMPLE:
	    if ((nTextLength = GetWindowTextLength(hWnd)) > 0) {
		lpszWindowText = (nTextLength < sizeof(szBuf))?
			szBuf:(LPSTR)WinMalloc(nTextLength+1);

		GetWindowText(hWnd, lpszWindowText, nTextLength+1);
	    }
	    else
		lpszWindowText = "";

	    SetBkMode(hDC,OPAQUE);
	    SetBkColor(hDC,GetSysColor(COLOR_WINDOW));
	    SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
	    if ((hFont = GetWindowWord(hWnd, SWD_HFONT)))
		hFont = SelectObject(hDC, hFont);
	    if (wStyle & SS_NOPREFIX)
		ExtTextOut(hDC,
			0,0,
			ETO_OPAQUE|ETO_CLIPPED,
			&rcClient,
			lpszWindowText, lstrlen(lpszWindowText),
			NULL);
	    else
		TextOut(hDC,
			0,0,
			lpszWindowText, lstrlen(lpszWindowText));

	    if (nTextLength >= sizeof(szBuf))
		WinFree(lpszWindowText);

	    if (hFont)
		SelectObject(hDC, hFont);
	    break;

	case SS_ICON:
	    hBr = GetControlBrush(hWnd, hDC, CTLCOLOR_STATIC);
	    FillRect(hDC, &ps.rcPaint, hBr);
	    hIcon = GetWindowWord(hWnd, SWD_HICON);
	    DrawIcon(hDC,0,0,hIcon);			
	    break;

	case SS_BLACKRECT:
	case SS_GRAYRECT:
	case SS_WHITERECT:
	case SS_BLACKFRAME:
	case SS_GRAYFRAME:
	case SS_WHITEFRAME:
	    switch(wStyle) {
		case SS_BLACKRECT:
		case SS_BLACKFRAME:
		    hBr = GetSysColorBrush(COLOR_WINDOWFRAME);
		    break;
		case SS_GRAYRECT:
		case SS_GRAYFRAME:
		    hBr = GetSysColorBrush(COLOR_BACKGROUND);
		    break;
		case SS_WHITERECT:
		case SS_WHITEFRAME:
		    hBr = GetSysColorBrush(COLOR_WINDOW);
		    break;
	    }
	    if (wStyle < SS_BLACKFRAME)
		FillRect(hDC, &ps.rcPaint, hBr);
	    else {
		FrameRect(hDC, &rcClient, hBr);
	    }
    }
    EndPaint(hWnd,&ps);
}
