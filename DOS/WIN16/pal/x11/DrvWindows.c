/*
	@(#)DrvWindows.c	2.45
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
 
#include <sys/types.h>

#include "windows.h"
#include "windowsx.h"
#include "Driver.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "Log.h"
#include "DrvDP.h"		/* PRIVATEDISPLAY */
#include "WinData.h"
#include "GdiRegions.h"		/* REGION32 */
#include "DrvDC.h"
#include "DrvKeyboard.h"

	/* enable new palette code */
#define TWIN_PALETTE

/* external library hook */
extern TWINLIBCALLBACKPROC lpLibCallback;

static DWORD DrvInitWindowSubsystem(UINT,BOOL);
static DWORD DrvCreateWindows(LPARAM, LPARAM, LPVOID);
static DWORD DrvDestroyWindows(LPARAM, LPARAM, LPVOID);
static DWORD DrvHandleWindowEvents(XEvent *);
static DWORD DrvShowWindow(LPARAM, LPARAM, LPVOID);
static DWORD DrvWindowsSetCapture(LPARAM,LPARAM,LPVOID);
static DWORD DrvWindowsReleaseCapture(LPARAM,LPARAM,LPVOID);
static DWORD DrvCapture(int, Window);
static DWORD DrvStackingOrder(LPARAM,LPARAM,LPVOID);
static DWORD DrvFixupStackingOrder(HWND, Window, Window);
static DWORD DrvFlushWindowExposes(LPARAM, LPARAM, LPVOID);
static DWORD DrvWindowsMoveFrame(LPARAM,LPARAM,LPVOID);
static DWORD DrvWindowsMoveResizeFrame(LPARAM,LPARAM,LPVOID);
static DWORD DrvWindowsResizeFrame(LPARAM,LPARAM,LPVOID);
static DWORD DrvWindowsSetText(LPARAM,LPARAM,LPVOID);
static DWORD DrvWindowsIconify(LPARAM,LPARAM,LPVOID);
static DWORD DrvWindowsRaiseWindow(LPARAM,LPARAM,LPVOID);
DWORD DrvWindowsEvents(LPARAM,LPARAM,LPVOID);
static DWORD DrvWindowsGetCaps(LPARAM,LPARAM,LPVOID);
static DWORD DrvWindowsFixupStackingOrder(LPARAM,LPARAM,LPVOID);
DWORD DrvWindowsInit(LPARAM,LPARAM,LPVOID);
DWORD DrvWindowsTab(void);

#define X_ICON_SIZE	64
#define DRV_GETSET_GET	1
#define DRV_GETSET_SET	2

typedef struct
{
	long	State;
	Window	IconWindow;
	Window	PixmapWindow;
} WM_WINDOW_STATE;

extern DWORD DriverPeekTypedEvent(int,Window);
extern void  DriverFlushXEvents(void);
extern DWORD DrvMakePixel(COLORREF, LPDRIVERDC);

static DWORD 
DrvGetSetWMState ( Window		XWindow,
                   WM_WINDOW_STATE*	pWMWindowState,
                   int			GetSetStatus )
{

	Display*		pXDisplay = GETDP()->display;
	static Atom		AWM_STATE = 0;
	XWindowAttributes	XWindowAttributes;
	unsigned char*		pProperty;
	unsigned long		nItems, bytes_remain;
	Atom			Atom;
	int			aFormat;


	if ( ! AWM_STATE )
		AWM_STATE = XInternAtom ( pXDisplay, "WM_STATE", FALSE );

	if ( ! AWM_STATE )
		return ( 0xffffffff );
	else
	if ( ! XGetWindowAttributes ( pXDisplay, XWindow,
	                              &XWindowAttributes ) )
		return ( 0xffffffff );
	else
	{
		XGetWindowProperty ( pXDisplay, XWindow,
		                     AWM_STATE, 0L, 3L, FALSE, AWM_STATE,
		                     &Atom, &aFormat, &nItems, 
		                     &bytes_remain, &pProperty );
		if ( ! pProperty )
			return ( 0xffffffff );
		else
		{
			switch ( GetSetStatus )
			{
				default:
				case DRV_GETSET_GET:
					memcpy ( ( unsigned char* )pWMWindowState, pProperty, 
					         sizeof ( WM_WINDOW_STATE ) );
					break;

				case DRV_GETSET_SET:
					XChangeProperty ( pXDisplay,
					                  XWindow, AWM_STATE,
					                  AWM_STATE, 32,
					                  PropModeReplace,
					                  ( unsigned char* )pWMWindowState,
					                  3 );
					break;
			}
			XFree ( pProperty );
		}
	}

	return ( 0 );

}


Display *
TWIN_GetDisplayData()
{
	return GETDP()->display;
}

static TWINDRVSUBPROC DrvWindowsEntryTab[] = {
	DrvWindowsInit,
	DrvWindowsGetCaps,
	DrvWindowsEvents,
	DrvCreateWindows,
	DrvDestroyWindows,
	DrvWindowsMoveFrame,
	DrvWindowsMoveResizeFrame,
	DrvWindowsResizeFrame,
	DrvShowWindow,
	DrvWindowsSetCapture,
	DrvWindowsReleaseCapture,
	DrvStackingOrder,
	DrvWindowsSetText,
	DrvFlushWindowExposes,
	DrvWindowsIconify,
	DrvWindowsRaiseWindow,
	DrvWindowsFixupStackingOrder
};
	

#define	TOPLEVEL_EVENTS	KeyPressMask | \
			KeyReleaseMask | \
			ButtonPressMask | \
			ButtonReleaseMask | \
			EnterWindowMask | \
			LeaveWindowMask | \
			PointerMotionMask | \
			Button1MotionMask | \
			Button2MotionMask | \
			Button3MotionMask | \
			KeymapStateMask | \
			ExposureMask | \
			VisibilityChangeMask | \
			StructureNotifyMask | \
/*			ResizeRedirectMask | */ \
/*			SubstructureNotifyMask | */ \
/*			SubstructureRedirectMask | */ \
			FocusChangeMask | \
			PropertyChangeMask | \
			ColormapChangeMask | \
			OwnerGrabButtonMask

static DWORD
DrvWindowsMoveFrame(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	PRIVATEDISPLAY *dp = GETDP();
	Window frame_win = (Window)lpStruct;
	XMoveWindow(dp->display, frame_win,((LPRECT)dwParm2)->left,
		((LPRECT)dwParm2)->top);
	return 1L;
}

static DWORD
DrvWindowsMoveResizeFrame(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
            PRIVATEDISPLAY *dp = GETDP();
            Window frame_win = (Window)lpStruct;

            /* X does not tolerate windows with zero or negative size */
            if (((LPRECT)dwParm2)->right <= 0)
                ((LPRECT)dwParm2)->right = 1;
            if (((LPRECT)dwParm2)->bottom <= 0)
                ((LPRECT)dwParm2)->bottom = 1;

            XMoveResizeWindow(dp->display,
                frame_win,
                ((LPRECT)dwParm2)->left,
                ((LPRECT)dwParm2)->top,
                ((LPRECT)dwParm2)->right,
                ((LPRECT)dwParm2)->bottom);
            return 1L;
}

static DWORD
DrvWindowsResizeFrame(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
            PRIVATEDISPLAY *dp = GETDP();
            Window frame_win = (Window)lpStruct;

            /* X does not tolerate windows with zero or negative size */
            if (((LPRECT)dwParm2)->right <= 0)
                ((LPRECT)dwParm2)->right = 1;
            if (((LPRECT)dwParm2)->bottom <= 0)
                ((LPRECT)dwParm2)->bottom = 1;

            XResizeWindow(dp->display,
                frame_win,
                ((LPRECT)dwParm2)->right,
                ((LPRECT)dwParm2)->bottom);
            return 1L;
}

static DWORD
DrvWindowsSetCapture(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return DrvCapture(TRUE, (Window)lpStruct);
}

static DWORD
DrvWindowsReleaseCapture(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return DrvCapture(FALSE, 0);
}

static DWORD
DrvWindowsSetText(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
            PRIVATEDISPLAY *dp = GETDP();
            Window frame_win = (Window)lpStruct;
            if (frame_win) {
                XStoreName(dp->display,
                    frame_win,
                    (LPSTR)dwParm2);
                return 1L;
            }
            else
                return 0L;
}

static DWORD
DrvWindowsIconify(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
    XIconifyWindow(GETDP()->display,(Window)lpStruct,0);
    return 1L;
}

static DWORD
DrvWindowsRaiseWindow(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
    XRaiseWindow(GETDP()->display,(Window)lpStruct);
    return 1L;
}

static DWORD
DrvCreateWindows(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
    LPWINCREATESTRUCT lpwcs = (LPWINCREATESTRUCT)lpStruct;
    UINT input_class;
    DWORD dwEventMask;
    DWORD dwCWMask;
    XSetWindowAttributes xattr;
    XSizeHints size_hints;
    XWMHints wm_hints;
    Window win_parent, frame_win;
    Pixmap icon_pixmap = None,icon_mask = None;
    DRVIMAGEDATA *lpimage;
    GC gc;
    PRIVATEDISPLAY *Disp = GETDP();
    WM_WINDOW_STATE WMWindowState;

    input_class = (lpwcs->dwExStyle & WS_EX_TRANSPARENT)?
		InputOnly:InputOutput;

    /* set general default values and mask for xattr */
    /* default white X background */
#ifdef	LATER
    these should call color subsystem
#endif
#ifdef TWIN_PALETTE
    xattr.background_pixel = DrvMakePixel(RGB(255,255,255), NULL);
    xattr.border_pixel = DrvMakePixel(RGB(0,0,0), NULL);
#else
    xattr.background_pixel = DrvMakePixel(RGB(255,255,255));
    xattr.border_pixel = DrvMakePixel(RGB(0,0,0));
#endif
    /* default NorthWestGravity */
    xattr.bit_gravity = NorthWestGravity;

#ifdef TWIN_PALETTE
    xattr.colormap = Disp->SystemPalette;
#else
    xattr.colormap = Disp->color_map;
#endif

    dwCWMask = CWBackPixel|CWBorderPixel|CWBitGravity|CWColormap;

	win_parent = RootWindow(Disp->display, Disp->screen);

	/* for top-level window without caption use override_redirect */
	if ((lpwcs->dwStyle & WS_CAPTION) != WS_CAPTION)
	    xattr.override_redirect = True;
	else
	    xattr.override_redirect = False;

	/* if SAVEBITS and server supports save_under, use it */
	if ((lpwcs->dwExStyle & WS_EX_SAVEBITS) &&
	    XDoesSaveUnders(XScreenOfDisplay(Disp->display,Disp->screen)))
	    xattr.save_under = True;
	else
	    xattr.save_under = False;

	if (lpwcs->Cursor) 
	    xattr.cursor = (Cursor)lpwcs->Cursor;
	else
	    xattr.cursor = None;


	/* X does not tolerate zero-size windows */
	lpwcs->rcFrame.right = max(lpwcs->rcFrame.right,1);
	lpwcs->rcFrame.bottom = max(lpwcs->rcFrame.bottom,1);

	if (input_class != InputOnly)
	    frame_win = XCreateWindow(Disp->display, win_parent,
		lpwcs->rcFrame.left,lpwcs->rcFrame.top,
		lpwcs->rcFrame.right,lpwcs->rcFrame.bottom,
		lpwcs->nBorder,
		CopyFromParent,
		input_class,
		CopyFromParent,
		dwCWMask | CWOverrideRedirect| CWSaveUnder | CWCursor,
		&xattr);
	else
	    frame_win = XCreateWindow(Disp->display, win_parent,
		lpwcs->rcFrame.left,lpwcs->rcFrame.top,
		lpwcs->rcFrame.right,lpwcs->rcFrame.bottom,
		0,
		CopyFromParent,
		InputOnly,
		CopyFromParent,
		CWOverrideRedirect,
		&xattr);

	if (frame_win == 0)
	    return 0L;

	/* save necessary contexts in the frame window */
	XSaveContext(Disp->display, frame_win, Disp->client_hwnd,
		(caddr_t)MAKELONG(lpwcs->hWndClient,lpwcs->hWndFrame));
	XSaveContext(Disp->display, frame_win, Disp->window_style,
		(caddr_t)lpwcs->dwStyle);
	XSaveContext(Disp->display, frame_win, Disp->window_ex_style,
		(caddr_t)lpwcs->dwExStyle);

#ifdef	LATER
	/* Use XSetWMProperties for top-level windows instead */
#endif
	/* Set standard hints for window manager */
#ifdef	LATER
	/* we can set user-specified values, min/maxsize, basesize etc. here */
#endif
	size_hints.flags = PPosition | PSize;
	size_hints.x = lpwcs->rcFrame.left;
	size_hints.y = lpwcs->rcFrame.top;
	size_hints.width = lpwcs->rcFrame.right;
	size_hints.height = lpwcs->rcFrame.bottom;

	if (lpwcs->lpIconANDData) {
	    XGCValues values;

	    lpimage = (DRVIMAGEDATA *)lpwcs->lpIconANDData;
	    if (lpimage->image) {
		icon_mask = XCreatePixmap(Disp->display,
				DefaultRootWindow(Disp->display),
				X_ICON_SIZE,X_ICON_SIZE,1);
		values.foreground = 0;
		values.background = 1;
		gc = XCreateGC(Disp->display,icon_mask,
				GCForeground|GCBackground,&values);
		XPutImage(Disp->display,
			  icon_mask,
			  gc,
			  lpimage->image,
			  0,0,0,0,
			  lpimage->nWidth,lpimage->nHeight);
		XFreeGC(Disp->display,gc);
	    }
	}

	if (lpwcs->lpIconXORData) {

	    lpimage = (DRVIMAGEDATA *)lpwcs->lpIconXORData;
	    if (lpimage->image) {
#if 1
		icon_pixmap = XCreateBitmapFromData(Disp->display, 
			DefaultRootWindow(Disp->display), lpimage->image->data,
			lpimage->nWidth, lpimage->nHeight);
#else
		{
	        int xOffset,yOffset;
		icon_pixmap = XCreatePixmap(Disp->display,
				DefaultRootWindow(Disp->display),
				X_ICON_SIZE,X_ICON_SIZE,1);
		gc = XCreateGC(Disp->display,icon_pixmap,0,0L);
		XSetForeground(Disp->display,gc,
				WhitePixel(Disp->display,Disp->screen));
		XFillRectangle(Disp->display,icon_pixmap,gc,0,0,
				X_ICON_SIZE,X_ICON_SIZE);
		xOffset = (X_ICON_SIZE - lpimage->nWidth)/2;
		yOffset = (X_ICON_SIZE - lpimage->nHeight)/2;
		XSetClipMask(Disp->display,gc,icon_mask);
		XSetClipOrigin(Disp->display,gc,xOffset,yOffset);
		XPutImage(Disp->display,
			  icon_pixmap,
			  gc,
			  lpimage->image,
			  0,0,xOffset,yOffset,
			  lpimage->nWidth,lpimage->nHeight);
		XFreeGC(Disp->display,gc);
		}
#endif
	    }
	}

	/* set properties for window manager */
	XSetStandardProperties(Disp->display, frame_win,
	    (HIWORD(lpwcs->lpszName))?lpwcs->lpszName:"",
	    (HIWORD(lpwcs->lpszName))?lpwcs->lpszName:"",
	    icon_pixmap,
#ifdef	LATER
	/* pass argv and argc here */
#else
	    0,0,
#endif
	    &size_hints);

#ifdef	LATER
	/* can this be False??? */
#endif
	wm_hints.input = True;
	wm_hints.icon_pixmap = icon_pixmap;
	wm_hints.icon_mask = icon_mask;
#ifdef	LATER
	/* we can set icon_window,icon position,initial state,group here */
#endif
	wm_hints.flags = InputHint | StateHint;
	if (icon_pixmap != None)
	    wm_hints.flags |= IconPixmapHint;
#ifdef LATER
    	if (lpwcs->dwStyle & WS_MINIMIZE)
		wm_hints.initial_state = IconicState;
	else
#endif
		wm_hints.initial_state = NormalState;
#ifdef LATER
	if ( XGetIconSizes ( Disp->display, frame_win, &pIconSizes, 
	                     &nIconSizes ) )
	{
		IconHeight = pIconSizes->max_height;
		IconWidth  = pIconSizes->max_width;
	}
	else
	{
		IconHeight = IconWidth = X_ICON_SIZE;
	}
	wm_hints.icon_window = XCreateSimpleWindow ( Disp->display,
	                                             win_parent, 0, 0,
	                                             IconWidth, IconHeight,
	                                             0, 0, 0xffffffff );
	xattr.override_redirect = True;
	xattr.background_pixmap = icon_pixmap;
	xattr.background_pixel  = DrvMakePixel(RGB(255,255,255), NULL);
	XChangeWindowAttributes ( Disp->display, wm_hints.icon_window,
	                          CWOverrideRedirect | CWBackPixel,
	                          &xattr );
#endif
	XSetWMHints(Disp->display, frame_win, &wm_hints);

	WMWindowState.State = wm_hints.initial_state;
	WMWindowState.IconWindow = wm_hints.icon_window;
	WMWindowState.PixmapWindow = icon_pixmap;
	DrvGetSetWMState ( frame_win, &WMWindowState, DRV_GETSET_SET );

#ifdef	LATER
	/* can other protocols (WM_TAKE_FOCUS etc.) be used??? */
#endif
	if (Disp->wm_delete_window != None)
	    XSetWMProtocols(Disp->display,frame_win,&Disp->wm_delete_window,1);

	dwEventMask = TOPLEVEL_EVENTS;

	/* Select event types wanted */
	XSelectInput(Disp->display, frame_win, dwEventMask);

    return (DWORD)frame_win;
}

static DWORD
DrvDestroyWindows(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
    Window win = (Window)lpStruct;
    PRIVATEDISPLAY *Disp = GETDP();

    XDeleteContext(Disp->display,win,Disp->client_hwnd);
    XDeleteContext(Disp->display,win,Disp->window_style);
    XDeleteContext(Disp->display,win,Disp->window_ex_style);

    XDestroyWindow(Disp->display, win);

    DriverFlushXEvents();

    return 1L;
}

static DWORD
DrvShowWindow(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
    DWORD dwShow = (DWORD)dwParm1;
    Window frame_win = (Window)lpStruct;
    PRIVATEDISPLAY *Disp = GETDP();

    if (frame_win) {
	if (dwShow == DSW_SHOW)
	    XMapRaised(Disp->display, frame_win);
	else
	    XUnmapWindow(Disp->display, frame_win);
    }

    return 1L;
}

static DWORD
DrvCapture(BOOL bSet, Window frame_win)
{
    int nRet;
    PRIVATEDISPLAY *Disp = GETDP();

    if (bSet) {

#ifdef	LATER
	/* it seems that we have to refine our capture model:
	1) in Windows only the foreground window can capture the mouse.
	   When a background window attempts to do so, the window receives
	   messages only for mouse events that occur when the cursor spot
	   is within the visible portion of the window.
	   Also, even if the foreground window has captured the mouse,
	   the user can still click another window, bringing it to the
	   foreground.
	2) we can try to make more use of XGrabPointer parameters:
	   set a cursor by setting cursor parameter to the call.
	*/
#else
	nRet = XGrabPointer(Disp->display,
		frame_win,
		False,
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
	 		Button1MotionMask | Button2MotionMask |
	 		Button3MotionMask | OwnerGrabButtonMask,
		GrabModeAsync,
		GrabModeAsync,
		None,
		None,
		CurrentTime);
#endif
	XSync(Disp->display,False);

	return (nRet == GrabSuccess)?1L:0L;
    }
    else {
	Disp = GETDP();
	XUngrabPointer(Disp->display, CurrentTime);
	XSync(Disp->display,False);

	return 1L;
    }
}

static Window TopLevelParent(Window win)
{
    PRIVATEDISPLAY *dp = GETDP();
    Window last_parent = win;
    Window root;
    Window parent;
    Window *children;
    unsigned int nchildren;
    
    while (XQueryTree(dp->display, last_parent, 
		      &root, &parent, &children, &nchildren))
    {
	XFree(children);
	if (!parent || parent == root)
	    break;

	last_parent = parent;
    }

    return last_parent;
}

static DWORD
DrvFixupStackingOrder(HWND hWndInsertAfter, Window WinAfter, Window win)
{
    XWindowChanges changes;
    PRIVATEDISPLAY *Disp = GETDP();
    Window WinAfterParent, winParent;
	
    WinAfterParent = TopLevelParent(WinAfter);
    winParent = TopLevelParent(win);
	
    changes.sibling = winParent;
    changes.stack_mode = Above;
    XConfigureWindow(Disp->display, WinAfterParent, 
		     CWSibling|CWStackMode, &changes);
    return 0;
}

static DWORD
DrvStackingOrder(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
    HWND hWndInsertAfter = (HWND)dwParm1;
    HWND WinAfter = (HWND)dwParm2;
    Window win = (Window)lpStruct;
    Window win_restack[2];
    PRIVATEDISPLAY *Disp = GETDP();
    int nFunc = (int)hWndInsertAfter;

    switch (nFunc) {
	case (int)HWND_TOP:
#ifdef	LATER
	/* this should be resolved at the upper level */
#else
	case (int)HWND_TOPMOST:
#endif
	    XRaiseWindow(Disp->display, win);
	    break;

	case (int)HWND_BOTTOM:
#ifdef	LATER
	/* this should be resolved at the upper level */
#else
	case (int)HWND_NOTOPMOST:
#endif
	    XLowerWindow(Disp->display, win);
	    break;

	default:
	    /*  this means a real hWndInsertAfter */
	    win_restack[0] = WinAfter;
	    win_restack[1] = win;
	    XRestackWindows(Disp->display, win_restack, 2);
	    break;
    }

    return 1L;
}

static DWORD
DrvFlushWindowExposes(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
    Window frame_win = (Window)lpStruct;
    PRIVATEDISPLAY *Disp = GETDP();

    XSync(Disp->display,False);
    if(frame_win)
    	return (DWORD)DriverPeekTypedEvent(Expose,frame_win);
    return 0;
}

static int ButtonMsg[3][6] = {
	{ 0,WM_LBUTTONUP,  WM_MBUTTONUP,  WM_RBUTTONUP,0,0} ,
	{ 0,WM_LBUTTONDOWN,WM_MBUTTONDOWN,WM_RBUTTONDOWN,0,0} ,
	{ 0,WM_LBUTTONDBLCLK,WM_MBUTTONDBLCLK,WM_RBUTTONDBLCLK,0,0}
};

#define	BEC_BUTTONUP		0
#define	BEC_BUTTONDOWN		1
#define	BEC_BUTTONDBLCLK	2

static DWORD
DrvHandleWindowEvents(XEvent *event)
{
    PRIVATEDISPLAY *dp = GETDP();
    int nBtnMessage = BEC_BUTTONUP;
    DWORD dwClientWin;
    HWND hWnd;
    UINT uiXState,uiWinState;
    MSG msg;

    switch (event->type) {
        case ButtonPress:
	    if ((event->xbutton.time - dp->LastClickTime) < 
			dp->DoubleClickTime)
		nBtnMessage = BEC_BUTTONDBLCLK;
	    else
		nBtnMessage = BEC_BUTTONDOWN;
	    dp->LastClickTime = event->xbutton.time;
	    /* fall thru into ButtonRelease case */

	case ButtonRelease:

	    /* update the known server time */
	    dp->ServerTime    = event->xbutton.time;

	    if (XFindContext(dp->display,event->xbutton.window,
			dp->client_hwnd,(caddr_t *)&dwClientWin))
		return 0L;

	    hWnd = (HWND)HIWORD(dwClientWin);

	    uiXState = event->xbutton.state;
	    if (nBtnMessage != BEC_BUTTONUP) {
		uiWinState = ((event->xbutton.button == Button1)?MK_LBUTTON:
			((event->xbutton.button == Button2)?MK_MBUTTON:
			((event->xbutton.button == Button3)?MK_RBUTTON:0)));
	    }
	    else
		uiWinState = 0;
	    uiWinState |= (uiXState & Button1Mask)? MK_LBUTTON:0;
	    uiWinState |= (uiXState & Button2Mask)? MK_MBUTTON:0;
	    uiWinState |= (uiXState & Button3Mask)? MK_RBUTTON:0;

#ifdef	DEBUG
	    memset((LPSTR)&msg,'\0',sizeof(MSG));
#endif

	    msg.hwnd    = hWnd;
	    msg.message = ButtonMsg[nBtnMessage][event->xbutton.button];
	    msg.wParam  = uiWinState;
	    msg.lParam  = MAKELPARAM(event->xbutton.x,event->xbutton.y);
	    msg.time    = event->xbutton.time;
	    msg.pt.x    = event->xbutton.x_root;
	    msg.pt.y    = event->xbutton.y_root;

	    return lpLibCallback(TWINLIBCALLBACK_BUTTON,
			0,0,(LPVOID)&msg);

	    break;

	case MotionNotify:
	    /* 
	     * The modifiers may have been changed when we weren't looking. 
	     */
	    DrvSynchronizeAllModifiers(event->xmotion.state);

	    dp->ServerTime = event->xmotion.time;

	    if (XFindContext(dp->display,event->xmotion.window,
			dp->client_hwnd,(caddr_t *)&dwClientWin))
		return 0L;

	    uiWinState = (event->xmotion.state & ControlMask)?  MK_CONTROL:0;
	    uiWinState |= (event->xmotion.state & ShiftMask)?
				MK_SHIFT:0;
	    uiWinState |= (event->xmotion.state & Button1Mask)?
				MK_LBUTTON:0;
	    uiWinState |= (event->xmotion.state & Button2Mask)?
				MK_MBUTTON:0;
	    uiWinState |= (event->xmotion.state & Button3Mask)?
				MK_RBUTTON:0;

	    hWnd = HIWORD(dwClientWin);

#ifdef	DEBUG
	    memset((LPSTR)&msg,'\0',sizeof(MSG));
#endif

	    msg.hwnd    = hWnd;
	    msg.message = WM_MOUSEMOVE;
	    msg.wParam  = uiWinState;
	    msg.time    = event->xmotion.time;
	    msg.lParam  = MAKELPARAM(event->xmotion.x,event->xmotion.y);
	    msg.pt.x    = event->xmotion.x_root;
	    msg.pt.y    = event->xmotion.y_root;

	    if (dp->fCursorClipped) {
		BOOL bWarp = FALSE;

		if (msg.pt.x < dp->rcCursorClip.left) {
		    msg.pt.x = dp->rcCursorClip.left;
		    bWarp = TRUE;
		}
		if (msg.pt.x >= dp->rcCursorClip.right) {
		    msg.pt.x = dp->rcCursorClip.right - 1;
		    bWarp = TRUE;
		}
		if (msg.pt.y < dp->rcCursorClip.top) {
		    msg.pt.y = dp->rcCursorClip.top;
		    bWarp = TRUE;
		}
		if (msg.pt.y >= dp->rcCursorClip.bottom) {
		    msg.pt.y = dp->rcCursorClip.bottom - 1;
		    bWarp = TRUE;
		}
		if (bWarp)
		    XWarpPointer(dp->display,None,
				DefaultRootWindow(dp->display),
				0,0,0,0,
				msg.pt.x,msg.pt.y);
	    }

	    return lpLibCallback(TWINLIBCALLBACK_BUTTON,
			0,0,(LPVOID)&msg);

	    break;

	default:
	    return 0L;
    }
}

static DWORD
DrvInitWindowSubsystem(UINT uiCompatibility, BOOL bInit)
{
    return 0L;
}



DWORD
DrvIsIconic ( Window	frame_win )
{

	WM_WINDOW_STATE		WMWindowState;

	if ( DrvGetSetWMState ( frame_win, &WMWindowState, 
	                        DRV_GETSET_GET ) )
		return ( 0xffffffff );
	else
		return ( WMWindowState.State == IconicState );


}

DWORD
DrvMinimize( Window frame_win)
{
	Display*		pXDisplay = GETDP()->display;
	WM_WINDOW_STATE		WMWindowState;
	XWMHints*		pWMHints;

	DWORD	ReturnCode = 0;

	if ( ! DrvIsIconic ( frame_win ) )
	{
		if ( DrvGetSetWMState ( frame_win, &WMWindowState, 
		                        DRV_GETSET_GET ) )
			ReturnCode = 0xffffffff;
		else
		{
			pWMHints = XGetWMHints ( pXDisplay, frame_win );
			XUnmapWindow ( pXDisplay, frame_win );
			WMWindowState.State      = IconicState;
			WMWindowState.IconWindow = pWMHints->icon_window;
			DrvGetSetWMState ( frame_win, &WMWindowState,
			                   DRV_GETSET_SET );
			pWMHints->flags         = StateHint | IconPixmapHint | 
			                          IconWindowHint | InputHint;
			pWMHints->input         = True;
			pWMHints->initial_state = IconicState;
			pWMHints->icon_pixmap   = 0;
			pWMHints->icon_window   = WMWindowState.IconWindow;
			XSetWMHints ( pXDisplay, frame_win, pWMHints );
			DrvGetSetWMState ( frame_win, &WMWindowState,
			                   DRV_GETSET_SET );
			/* maybe we should lower it as well? */
			XMapWindow ( pXDisplay, frame_win );
			XFree(pWMHints);
		}
	}
	
	return ( ReturnCode );

}

DWORD
DrvRestore( Window frame_win)
{
	Display*		pXDisplay = GETDP()->display;
	WM_WINDOW_STATE		WMWindowState;
	XWMHints*		pWMHints;

	DWORD	ReturnCode = 0;

	switch ( DrvIsIconic ( frame_win ) )
	{
		case 0xffffffff:
			ReturnCode = 0xffffffff;
			break;

		case 0:
			break;

		default:
			if ( DrvGetSetWMState ( frame_win, &WMWindowState, 
			                        DRV_GETSET_GET ) )
				ReturnCode = 0xffffffff;
			else
			{
				pWMHints = XGetWMHints ( pXDisplay, frame_win );
				XUnmapWindow ( pXDisplay, frame_win );
				WMWindowState.State = NormalState;
				DrvGetSetWMState ( frame_win, &WMWindowState,
				                   DRV_GETSET_SET );
				pWMHints->flags = StateHint;
				pWMHints->initial_state = NormalState;
				XSetWMHints ( pXDisplay, frame_win, pWMHints );
				XMapRaised ( pXDisplay, frame_win );
				XFree(pWMHints);
			}
			break;
	}
	
	return ( ReturnCode );

}

DWORD
DrvWindowsInit(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return	DrvInitWindowSubsystem((UINT)dwParam1, (BOOL)dwParam2);
}

static DWORD
DrvWindowsGetCaps(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return (DWORD)WSUB_CANDO_RESTACK;
}

DWORD
DrvWindowsEvents(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return DrvHandleWindowEvents((XEvent *)lpStruct);
}

DWORD
DrvWindowsTab(void)
{
#if defined(TWIN_RUNTIME_DRVTAB)
        DrvWindowsEntryTab[0] = DrvWindowsInit;
        DrvWindowsEntryTab[1] = DrvWindowsGetCaps;
        DrvWindowsEntryTab[2] = DrvWindowsEvents;
        DrvWindowsEntryTab[3] = DrvCreateWindows;
        DrvWindowsEntryTab[4] = DrvDestroyWindows;
        DrvWindowsEntryTab[5] = DrvWindowsMoveFrame;
        DrvWindowsEntryTab[6] = DrvWindowsMoveResizeFrame;
        DrvWindowsEntryTab[7] = DrvWindowsResizeFrame;
        DrvWindowsEntryTab[8] = DrvShowWindow;
        DrvWindowsEntryTab[9] = DrvWindowsSetCapture;
        DrvWindowsEntryTab[10] = DrvWindowsReleaseCapture;
        DrvWindowsEntryTab[11] = DrvStackingOrder;
        DrvWindowsEntryTab[12] = DrvWindowsSetText;
        DrvWindowsEntryTab[13] = DrvFlushWindowExposes;
        DrvWindowsEntryTab[14] = DrvWindowsIconify;
        DrvWindowsEntryTab[15] = DrvWindowsRaiseWindow;
        DrvWindowsEntryTab[16] = DrvWindowsFixupStackingOrder;
#endif
	return (DWORD)DrvWindowsEntryTab;
}

static DWORD
DrvWindowsFixupStackingOrder(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return DrvFixupStackingOrder((HWND)dwParm1, (Window)dwParm2, (Window)lpStruct);
}

