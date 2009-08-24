/*    
	Events.c	1.39
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
#include "Kernel.h"	/* QueueAddMsg */
#include "Frame.h"	/* DoNCXXX */
#include "WinDefs.h"	/* GetFrameClient */
#include "Log.h"
#include "Driver.h"
#include "KrnTask.h"
#include "Hotkey.h"
#include <stdarg.h>

DWORD TWIN_DrvCallback(DWORD, LPARAM, LPARAM, LPVOID);
BOOL TWIN_ForwardButtonEvent(LPMSG);

static BOOL PostButtonMessage(LPMSG);
static BOOL ForwardKeyboardEvent(LPMSG);
static BOOL TWIN_PostMessage(LPMSG);
static HWND	TWIN_CreateTimerWindow( TWINTIMERWIN* timerPB );

extern int TWIN_GetTimeout(int);
extern BOOL TWIN_ForwardExpose(HWND, int, LPVOID);
extern BOOL TWIN_ForwardConfigure(HWND, LPVOID);
extern HWND TWIN_VisibleChildWindowFromPoint(HWND, POINT);
extern HWND TWIN_ConvertToSysScroll(HWND, BOOL, LPPOINT);
extern BOOL TWIN_InternalClipboard(DWORD, DWORD);
extern HWND TWIN_InternalFocus(int, HWND, LONG);
extern DWORD TWIN_InternalMemory(int, LPSTR, UINT);
extern void TWIN_vslogstr(unsigned long, va_list);

#define	BC_BUTTONUP		0
#define	BC_BUTTONDOWN		1
#define	BC_BUTTONDBLCLK		2
#define	BC_MOUSEMOVE		3

DWORD
TWIN_DrvCallback(DWORD dwFunc, LPARAM lParam1, LPARAM lParam2, LPVOID lpStruct)
{
    switch (dwFunc) {
	case TWINLIBCALLBACK_BUTTON:
	    return (DWORD)PostButtonMessage((LPMSG)lpStruct);

	case TWINLIBCALLBACK_KEYBOARD:
	    return (DWORD)ForwardKeyboardEvent((LPMSG)lpStruct);

	case TWINLIBCALLBACK_TIMEOUT:
	    return (DWORD)TWIN_GetTimeout((int)lParam1);

	case TWINLIBCALLBACK_POSTMESSAGE:
	    return (DWORD)TWIN_PostMessage((LPMSG)lpStruct);

	case TWINLIBCALLBACK_EXPOSE:
	    return (DWORD)TWIN_ForwardExpose((HWND)lParam1, (int)lParam2,
			lpStruct);

	case TWINLIBCALLBACK_CONFIGURE:
	    return (DWORD)TWIN_ForwardConfigure((HWND)lParam1, lpStruct);

	case TWINLIBCALLBACK_CLIPBOARD:
	    return (DWORD)TWIN_InternalClipboard((DWORD)lParam1,(DWORD)lParam2);

	case TWINLIBCALLBACK_FOCUS:
	    return (DWORD)TWIN_InternalFocus((int)lParam2,
			GetFrameClient((HWND)lParam1),
			(LONG)lpStruct);

	case TWINLIBCALLBACK_MEMORY:
	    return TWIN_InternalMemory((int)lParam1,
				(LPSTR)lpStruct, (UINT)lParam2);
				
	case TWINLIBCALLBACK_CHECKOBJ:
		return TWIN_CheckObject((int)lParam1);
		
	case TWINLIBCALLBACK_VSLOGSTR:
		{
			TWIN_vslogstr(lParam1, lpStruct);
			return 0L;
		}

	case TWINLIBCALLBACK_TIMERWINDOW:
		return((DWORD )TWIN_CreateTimerWindow((LPTWINTIMERWIN )lpStruct ));
		
	default:
	    return 0L;
    }
}

static BOOL
PostButtonMessage(LPMSG lpMsg)
{
    return QueueAddMsg(0,-1,lpMsg);
}

BOOL
TWIN_ForwardButtonEvent(LPMSG lpMsg)
{
    HWND hWnd,hWndFrame,hWndCapture,hWndAncestor,hWndSibling;
    HWND hWndParent;
    HWND32 hWndParent32  = 0;
    RECT rcWnd;
    POINT pt;
    BOOL bEat = FALSE, bNonClient;
    WORD wClassStyle;
    int nBtnMessage;
    int nNCHit,nMouseActivate;
    DWORD dwStyle,dwExStyle;

    switch (lpMsg->message) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	    nBtnMessage = BC_BUTTONDOWN;
	    break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	    nBtnMessage = BC_BUTTONUP;
	    break;

	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	    nBtnMessage = BC_BUTTONDBLCLK;
	    break;

	case WM_MOUSEMOVE:
	    nBtnMessage = BC_MOUSEMOVE;
	    break;

	default:
	    return FALSE;
    }

    hWnd   = 0;
    nNCHit = 0;

    if (nBtnMessage != BC_MOUSEMOVE)
	(void)DRVCALL_KEYBOARD(PKH_BTNSTATE,0,0,lpMsg);

    if ((hWndCapture = GetCapture())) {
	lpMsg->hwnd = hWndCapture;
    }
    else {
	hWndFrame = lpMsg->hwnd;
	hWnd = GetWindow(lpMsg->hwnd,GW_CHILD);
	lpMsg->hwnd = TWIN_VisibleChildWindowFromPoint(hWnd,lpMsg->pt);
	if (0 == (hWnd = lpMsg->hwnd))
	    return FALSE;
	nNCHit = LOINT(SendMessage(lpMsg->hwnd,
				WM_NCHITTEST,(WPARAM)0,
				MAKELPARAM(lpMsg->pt.x,lpMsg->pt.y)));
    }

    if ((nBtnMessage == BC_BUTTONDBLCLK) && (hWndCapture == 0) && 
				(nNCHit == HTCLIENT)) {
	wClassStyle = GetClassStyle(lpMsg->hwnd);
	if (!(wClassStyle & CS_DBLCLKS))
	    /* if no doubleclicks style, turn back into BUTTONDOWN */
	    lpMsg->message -= 2;
    }

    /* if capture is set, bypass the normal hit testing */
    if (hWndCapture != 0) {
	pt = lpMsg->pt;
	if (nBtnMessage != BC_MOUSEMOVE)
		(void)DRVCALL_KEYBOARD(PKH_BTNSTATE,0,0,lpMsg);

	/* 
	 * check if the window with capture is a system scroll bar.  If yes,
	 * then we need to calculate point based on the parent window.
	 */
	hWndParent = GetParent(hWndCapture);
	if(hWndParent)
		hWndParent32 = GETHWND32(hWndParent);
	if (hWndParent32 && hWndParent32->hWndVTScroll == hWndCapture)
	{
	    ScreenToClient(hWndParent,&pt);
	    TWIN_ConvertToSysScroll(hWndParent, TRUE, &pt);
	}
	else if (hWndParent32 && hWndParent32->hWndHZScroll == hWndCapture)
	{
	    ScreenToClient(hWndParent,&pt);
	    TWIN_ConvertToSysScroll(hWndParent, FALSE, &pt);
	}
	else
	    ScreenToClient(hWndCapture,&pt);

	RELEASEWININFO(hWndParent32);

	lpMsg->lParam = MAKELPARAM(pt.x,pt.y);
	return TRUE;
    }

    if (nNCHit == HTERROR || nNCHit == HTNOWHERE)
	return FALSE;

    if (nNCHit == HTTRANSPARENT) {
	for (hWndSibling = GetWindow(hWnd,GW_HWNDNEXT);
	     hWndSibling;
	     hWndSibling = GetWindow(hWndSibling,GW_HWNDNEXT)) {
	    GetWindowRect(hWndSibling,&rcWnd);
	    if (PtInRect(&rcWnd,lpMsg->pt)) {
		nNCHit = (int)((short)LOWORD(SendMessage(
		    hWndSibling,WM_NCHITTEST,(WPARAM)0,
		    MAKELPARAM(lpMsg->pt.x,lpMsg->pt.y))));
		if (nNCHit != HTTRANSPARENT &&
		    nNCHit != HTNOWHERE &&
		    nNCHit != HTERROR)
		    break;
	    }
	}

	if (hWndSibling)
	    hWnd = hWndSibling;
	else {
	    if (0 == (hWnd = GetParent(hWnd)))
		return FALSE;
	    nNCHit = (int)((short)LOWORD(SendMessage(
		hWnd,WM_NCHITTEST,(WPARAM)0,
		MAKELPARAM(lpMsg->pt.x,lpMsg->pt.y))));
	}
	/* what about WM_GETCURSOR here??? */
	lpMsg->hwnd = hWnd;
    }

    if (nNCHit == HTHSCROLL || nNCHit == HTVSCROLL) {
	/* for now in case of scrollbars send directly to scrolls */
	/* what about WM_SETCURSOR here??? */
	pt = lpMsg->pt;
	ScreenToClient(hWnd,&pt);
	if ((HWND)0 == (lpMsg->hwnd = TWIN_ConvertToSysScroll(hWnd,
		(nNCHit == HTVSCROLL)?TRUE:FALSE,&pt)))
	    return FALSE;
	lpMsg->lParam = MAKELPARAM(pt.x,pt.y);
	SendMessage(lpMsg->hwnd,WM_SETCURSOR,
		(WPARAM)hWnd,MAKELPARAM(nNCHit,lpMsg->message));
	if (nBtnMessage != BC_MOUSEMOVE)
	    (void)DRVCALL_KEYBOARD(PKH_BTNSTATE,0,0,lpMsg);
	return TRUE;
    }

    bNonClient = (nNCHit != HTCLIENT);

    if (!bNonClient) {
	dwStyle = GetWindowStyle(hWnd);
	if (nBtnMessage == BC_BUTTONUP) {
	    if (((dwStyle & WS_CAPTION) == WS_CAPTION) &&
		!TestWF(hWnd,WFFRAMEON))
		/* button up in a client area of inactive window */
		PostMessage(hWnd,WM_CHILDACTIVATE,0,0L);
	}
	else if (nBtnMessage != BC_MOUSEMOVE) {
	    /* button down or doubleclick */
	    if (!TestWF(hWnd,WFFRAMEON)) {
		/* button press in a client area of inactive window */
		nMouseActivate = (int)((short)SendMessage(
				hWnd,WM_MOUSEACTIVATE,
				(WPARAM)GetTopLevelAncestor(hWnd),
				MAKELPARAM(HTCLIENT,lpMsg->message)));
		switch (nMouseActivate) {
		    case MA_ACTIVATE:
			    if ((dwStyle & WS_CAPTION) == WS_CAPTION)
				BringWindowToTop(hWnd);
			break;
		    case MA_NOACTIVATE:
			break;
		    case MA_ACTIVATEANDEAT:
			if ((dwStyle & WS_CAPTION) == WS_CAPTION)
			    BringWindowToTop(hWnd);
			bEat = TRUE;
			break;
		    case MA_NOACTIVATEANDEAT:
			bEat = TRUE;
			break;
		}
		if (bEat)
		    /* do we have do send WM_CURSOR here??? */
		    return FALSE;
	    }	/* if (!TestWF(hWnd,WFFRAMEON)) */
	    if (!IsTopLevel(hWnd)) {
		dwExStyle = GetWindowExStyle(hWnd);
		if (!(dwExStyle & WS_EX_NOPARENTNOTIFY)) {
		    hWndAncestor = hWnd;
		    do {
			hWndAncestor = GetParent(hWndAncestor);
			dwStyle = GetWindowStyle(hWndAncestor);
			pt = lpMsg->pt;
			ScreenToClient(hWndAncestor,&pt);
			SendMessage(hWndAncestor,WM_PARENTNOTIFY,
				GET_WM_PARENTNOTIFY2_MPS(lpMsg->message,
					pt.x,pt.y));
		    } while (dwStyle & WS_CHILD);
		}
	    }
	}
    }
    else {
	if (!IsTopLevel(hWnd)) {
	    if (nBtnMessage != BC_BUTTONUP && nBtnMessage != BC_MOUSEMOVE) {
		/* button down or doubleclick */
		dwStyle = GetWindowStyle(hWnd);
		if (!TestWF(hWnd,WFFRAMEON)) {
		    /* press in a non-client area of inactive window */
		    nMouseActivate = (int)((short)SendMessage(
				hWnd,WM_MOUSEACTIVATE,
				(WPARAM)GetTopLevelAncestor(hWnd),
				MAKELPARAM(HTCLIENT,lpMsg->message)));
		    switch (nMouseActivate) {
			case MA_ACTIVATE:
			    if ((dwStyle & WS_CAPTION) == WS_CAPTION)
				BringWindowToTop(hWnd);
			    break;
			case MA_NOACTIVATE:
			    break;
			case MA_ACTIVATEANDEAT:
			    if ((dwStyle & WS_CAPTION) == WS_CAPTION)
				BringWindowToTop(hWnd);
			    bEat = TRUE;
			    break;
			case MA_NOACTIVATEANDEAT:
			    bEat = TRUE;
			    break;
		    }
		    if (bEat)
			/* do we have do send WM_CURSOR here??? */
			return FALSE;
		}	/* if (!TestWF(hWnd,WFFRAMEON)) */
		dwExStyle = GetWindowLong(hWnd,GWL_EXSTYLE);
		if (!(dwExStyle & WS_EX_NOPARENTNOTIFY)) {
		    hWndAncestor = hWnd;
		    do {
			hWndAncestor = GetParent(hWndAncestor);
			dwStyle = GetWindowStyle(hWndAncestor);
			pt = lpMsg->pt;
			ScreenToClient(hWndAncestor,&pt);
			SendMessage(hWndAncestor,WM_PARENTNOTIFY,
				GET_WM_PARENTNOTIFY2_MPS(lpMsg->message,
					pt.x,pt.y));
		    } while (dwStyle & WS_CHILD);
		}
	    }
	}	/* if (!IsTopLevel(hWnd)) */
    }

    SendMessage(hWnd,WM_SETCURSOR,
	(WPARAM)hWnd,MAKELPARAM(nNCHit,lpMsg->message));

    if (bNonClient) {
	lpMsg->message -= (WM_LBUTTONDOWN - WM_NCLBUTTONDOWN);
	lpMsg->wParam  = (WPARAM)nNCHit;
	lpMsg->lParam = MAKELPARAM(lpMsg->pt.x,lpMsg->pt.y);
    }
    else {
	pt = lpMsg->pt;
	ScreenToClient(hWnd,&pt);
	lpMsg->lParam = MAKELPARAM(pt.x,pt.y);
    }

    return TRUE;
}

static BOOL
ForwardKeyboardEvent(LPMSG lpMsg)
{
    if ((HWND)0 == (lpMsg->hwnd = GetFocus())) {
	lpMsg->hwnd = GetActiveWindow();
	if (lpMsg->message == WM_KEYUP)
	    lpMsg->message = WM_SYSKEYUP;
	if (lpMsg->message == WM_KEYDOWN)
	    lpMsg->message = WM_SYSKEYDOWN;
    }

#ifdef TWIN32
    /* See if we have a hotkey registered for this key */
    if (!CheckHotKey(lpMsg))
#endif /* TWIN32 */
    QueueAddMsg(0,-1, lpMsg);

    return TRUE;
}

static BOOL
TWIN_PostMessage(LPMSG lpMsg)
{
    PostMessage(lpMsg->hwnd,lpMsg->message,lpMsg->wParam,lpMsg->lParam);
    return TRUE;
}

static HWND
TWIN_CreateTimerWindow( LPTWINTIMERWIN timerPB )
{
	WNDCLASS    wndClass;
	HINSTANCE	hInst = TWIN_GetInstanceFromTask( GetCurrentTask());
	HWND		result;

	memset((LPSTR )&wndClass, 0, sizeof( WNDCLASS )); 
	wndClass.style			= 0;
	wndClass.lpfnWndProc	= timerPB->winProc;
	wndClass.cbClsExtra		= 0;
	wndClass.cbWndExtra		= 0;
	wndClass.hInstance		= hInst;
	wndClass.hIcon			= (HICON) NULL;
	wndClass.hbrBackground	= 0;
	wndClass.hCursor		= 0;
	wndClass.lpszMenuName	= NULL;
	wndClass.lpszClassName	= timerPB->winName;

	RegisterClass( &wndClass );

	result = CreateWindow ( timerPB->winName, /* class name */
				NULL,             /* window name */
				WS_POPUP,         /* style */
				0,0,              /* initial x,y coordinates */
				0,0,              /* initial width & height */
				(HWND )NULL,      /* parent */
				(HMENU )NULL,     /* menu */
				(HINSTANCE )hInst, /* application instance */
				NULL );            /* extended params */
	if ( result != (HWND )NULL ) {
		UINT	timerResult;
		timerResult = SetTimer( result, timerPB->timerID, timerPB->timeout,
								(TIMERPROC )MakeProcInstance((FARPROC )timerPB->winProc, hInst ));
		if ( timerResult == 0 ) {
			/*
			 *  TODO:  Kill the window
			 */
			return((HWND )NULL );		
		}
	}
	return( result );
}
