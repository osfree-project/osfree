/*************************************************************************

	@(#)XEvents.c	2.37
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

**************************************************************************/
 

#include <sys/types.h>
#include <sys/time.h>
#include <math.h>

#include "windows.h"
#include "windowsx.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "Log.h"
#include "kerndef.h"
#include "DrvDP.h"
#include "compat.h"
#include "WinDefs.h"
#include "Driver.h"

static int DriverPrivateEvents(XEvent *);

/* external library hook */
extern TWINLIBCALLBACKPROC lpLibCallback;

/* this is exported to the library for now... */
DWORD DriverWaitEvent(BOOL);
void DriverFlushXEvents(void);

DWORD DrvHandleKeyboardEvents(LPARAM, LPARAM, LPVOID);
DWORD DrvWindowsEvents(LPARAM,LPARAM,LPVOID);
DWORD InternalICCCM(int ,PRIVATEDISPLAY *,XEvent *);

static BOOL bSelectWait = FALSE;

static void DrvPrintTree(PRIVATEDISPLAY *dp, Window start);

BOOL
DriverInSelectWait(void)
{
	/* HACK: Another thread can inquire as to whether the driver is
	 * blocked in a select() call.  Then Post*Message() will send a
	 * fictitious X event (which gets discarded) which will unblock
	 * the select() call.  This allows another thread to post messages
	 * while the driver is in a select() block.
	 */
	return (bSelectWait);
}

void
DriverFlushXEvents(void)
{
    PRIVATEDISPLAY *dp = GETDP();

    if (dp && dp->display)
	XFlush(dp->display);
}

DWORD
DriverWaitEvent(BOOL bNoWait)
{
	struct timeval timeout,*timeptr;
	fd_set rmask,wmask,emask;
	int    xstate,nfound,timer;
	unsigned long    timestart;

	XEvent Report,Peek;
        PRIVATEDISPLAY    *dp;

	timer = (int)lpLibCallback(TWINLIBCALLBACK_TIMEOUT,
			0, 0, (LPVOID)0);

        dp = GETDP();

	/* if we have a timer, when should it go off? */
	/* timer is number of milliseconds to wait... */
	if(timer) {
		/*printf("have timer %d server=%ld\n",
			timer,dp->ServerTime); */
		timestart = dp->ServerTime;
	} else {
		timestart = 0;
	}


	/* state machine, 
 	 * xstate <> 0 -> a msg posted to application 
 	 * xstate == 0 -> idle inside wait event
	 */
	xstate = 0;

	for(;;) {
		/* do we have an event queued? */ 
		if(XEventsQueued(dp->display,QueuedAfterFlush)) {

			/* go get the event that is pending */
			XNextEvent(dp->display, &Report);

			/* is it motion??? */
			if(Report.type == MotionNotify) {

				/* while we have events, check motion */
				while (XEventsQueued(dp->display,
					QueuedAfterFlush)) {

					/* peek at next... */
					XPeekEvent(Report.xmotion.display,
						&Peek);
				
					/* if not motion for current window */
					if(Peek.type != MotionNotify)
						break;
					if(Peek.xmotion.window != 
						Report.xmotion.window)
						break;

					/* lets get this one instead */
					XNextEvent(Report.xmotion.display,
						&Report);
				}
			}

			/* process the event */
			xstate = DriverPrivateEvents(&Report);

			/* no message was actually posted, try again... */
			if (xstate == 0)
				continue;

			/* a message was posted, so do not wait??? */
			timer = -1;
		} 

		/* something we did took time off the clock */
		if(timestart && (timestart != (unsigned long) dp->ServerTime)) {
			
			timer = dp->ServerTime - timestart;

			timer = (int)lpLibCallback(TWINLIBCALLBACK_TIMEOUT,
					(DWORD)timer, 0, (LPVOID)0);

			/* timer < 0 means something triggered, so quit */
			if(timer < 0) {
				/*printf("timer triggered\n"); */
				return 1L;
			}
			/* timer is now reset to new wait amount... */
			/* adjust it to polling if we did post something... */
			if (xstate != 0) {
				/*printf("something was posted, so poll\n");*/
				timer = -1;
			}
			timestart = dp->ServerTime;
		}

		/* if we are not blocking, and we have no timers,  */
		/* then poll, do not wait... */
		if(bNoWait && timer == 0) 
			timer = -1;

		/* timer == 0 means to block indefinitely... 	*/
		/* timeptr == 0 means no timeout 		*/
		/* timeptr != 0 means to wait some time... 	*/

		if(timer == 0) {
			timeptr = 0;
		} else if(timer < 1000) {
			/* less than a second */
			timeout.tv_sec  = 0;

			/* if we are polling set usec == 0 */
			/* else set to the number of usecs */
			if(timer < 0) {
				timeout.tv_usec = 0;
			} else {
				timeout.tv_usec = timer*1000;
			}
			timeptr = &timeout;
		} else {
			/* more than a second... */
			timeout.tv_sec =  timer/1000;
			timeout.tv_usec = 0;
			timeptr = &timeout;
		}

		FD_ZERO(&rmask);
		FD_ZERO(&emask);
		FD_ZERO(&wmask);
		FD_SET(ConnectionNumber(dp->display),&rmask);
		FD_SET(ConnectionNumber(dp->display),&emask);

		bSelectWait = TRUE;
		nfound = select(FD_SETSIZE,
			(fd_set *)&rmask,
			(fd_set *)&wmask,
			(fd_set *)&emask,
			timeptr);
		bSelectWait = FALSE;

		/* did we find anything? */
		if(nfound == 0) {
			/* if we were polling, then quit... */
			if(timer < 0) 
				break;
			
			/* we timed out, so check/decrement timer amount */
			timer = (int)lpLibCallback(TWINLIBCALLBACK_TIMEOUT,
					(DWORD)timer, 0, (LPVOID)0);

			/* timer < 0 means something triggered, so quit */
			if(timer < 0) {
				/*printf("we waited and got timeout\n"); */
				return 0L;
			}

			/* we did wait, but not enough... */
			/* go wait some more... */
			continue;
		}

		/* we were polling, ie. timer < 0, so we are done... */
		if(timer < 0) {
			/*printf("polling, w/ timer < 0?\n"); */
			break;
		}
	}
	return xstate;
}

DWORD
DriverPeekTypedEvent(int event_type, Window w)
{
    XEvent Report;
    Bool bIsEvent;
    int nCount = 0;
    PRIVATEDISPLAY    *dp;

    dp = GETDP();

    if (XEventsQueued(dp->display,QueuedAfterFlush)) {
	while ((bIsEvent = (w == (Window)0))?
	    XCheckTypedEvent(dp->display,event_type,&Report):
	    XCheckTypedWindowEvent(dp->display,w,event_type,&Report)) {
	    nCount++;
	    DriverPrivateEvents(&Report);
	}
    }
    return (DWORD)nCount;
}

static int	
DriverPrivateEvents(XEvent *report)
{
	UINT   wstate;
	HWND   hWnd,hWndFrame;
	WINDOWPOS wp;
	int count;
	int msgposted = 0;
	DWORD dwClientWin;
	RECT       rcWnd;
        PRIVATEDISPLAY    *dp;
	static Window win;
	Window win_focus;
	DWORD dwStyle;
	int revert_return;
	char *msgstr = 0;

        dp = GETDP();

	switch  (report->type) {
	case SelectionNotify:
		msgstr = "SelectionNotify";
		dp->ServerTime = report->xselection.time;
		break;
	case SelectionClear:
		msgstr = "SelectionClear";
		dp->ServerTime = report->xselectionclear.time;
		break;
	case SelectionRequest:
		msgstr = "SelectionRequest";
		dp->ServerTime = report->xselectionrequest.time;
		break;

	case PropertyNotify:
		msgstr = "PropertyNotify";
		dp->ServerTime = report->xproperty.time;

	/*
  	 *   fall through to common code, handle X ICCCM
 	 *   PropertyNotify will be related to clipboard and DDE
	 *   ClientMessage will be either
 	 *	wm_protocol	for user/window manager interactions
	 *	ipc_protocol	for DDE and low-level interactions
  	 */
	case ClientMessage:

		msgstr = "ClientMessage";
		if(InternalICCCM(report->type,dp,report))
			msgposted--;

		break;

	case Expose:
	{
	    XRectangle rcexpose;
	    Region RgnExpose = 0;
	    RECT rcExpose;
	    HWND hWndFrame   = 0;

	    msgstr = "Expose";
	    count = 0;

	    do {

		if (count == 0) {
		    if (XFindContext(dp->display,report->xexpose.window,
				dp->client_hwnd,(caddr_t *)&dwClientWin))
			continue;

		    hWndFrame = (HWND)HIWORD(dwClientWin);
		    count = report->xexpose.count;
		    if (count == 0) {
			rcExpose.left = report->xexpose.x;
			rcExpose.top = report->xexpose.y;
			rcExpose.right = report->xexpose.x+
					report->xexpose.width;
			rcExpose.bottom = report->xexpose.y+
					report->xexpose.height;
			lpLibCallback(TWINLIBCALLBACK_EXPOSE,(LPARAM)hWndFrame,
				SIMPLEREGION, (LPVOID)&rcExpose);
			msgposted--;
			continue;
		    }
		    else
			RgnExpose = XCreateRegion();
		}

		rcexpose.x = report->xexpose.x;
		rcexpose.y = report->xexpose.y;
		rcexpose.width = report->xexpose.width;
		rcexpose.height = report->xexpose.height;
		XUnionRectWithRegion(&rcexpose,RgnExpose,RgnExpose);

		if (report->xexpose.count == 0) {
		    lpLibCallback(TWINLIBCALLBACK_EXPOSE,(LPARAM)hWndFrame,
				COMPLEXREGION,(LPVOID)RgnExpose);
		    msgposted--;
		    XDestroyRegion(RgnExpose);
		    count = 0;
		}
	    } while (XCheckTypedEvent(dp->display,Expose,report));
	    break;
	}
	case ReparentNotify:
		msgstr = "ReparentNotify";
 		if (XFindContext(dp->display,report->xreparent.window,
 				dp->client_hwnd,(caddr_t *)&dwClientWin))
 			break;

		/* We have to repeat XResizeWindow for the sake of */
		/* OpenWindows, which loses resizing requests	 */
 		hWnd = LOWORD(dwClientWin);
 		GetPhysicalRect(hWnd,&rcWnd);

 		XResizeWindow(dp->display, report->xreparent.window,
			rcWnd.right-rcWnd.left,
			rcWnd.bottom-rcWnd.top);
		break;

	case ConfigureNotify:
		msgstr = "ConfigureNotify";
		if (XFindContext(dp->display,report->xconfigure.window,
				dp->client_hwnd,(caddr_t *)&dwClientWin))
			break;

		/* Window managers send synthetic ConfigureNotify events */
		/* if the window was moved but not resized (July 27, 1988 */
		/* ICCCM draft). Some window managers (OpenWin) offset the */
		/* origin by some fixed value, so take size portion and go */
		/* get origin from Xlib					 */
		wstate = SWP_NOACTIVATE|SWP_DRIVER|SWP_NOZORDER;
		{
			int xRoot,yRoot;
			Window child_return;

			XTranslateCoordinates(dp->display,
				report->xconfigure.window,
				RootWindow(dp->display,dp->screen),
				0,0,
				&xRoot,&yRoot,
				&child_return);

			report->xconfigure.x = xRoot;
			report->xconfigure.y = yRoot;
		}

		if (!report->xconfigure.width ||
		    !report->xconfigure.height)
		    wstate |= SWP_NOSIZE;

		wp.hwnd = (HWND)HIWORD(dwClientWin);
		wp.hwndInsertAfter = (HWND)0;
		wp.x = report->xconfigure.x;
		wp.y = report->xconfigure.y;
		wp.cx = report->xconfigure.width;
		wp.cy = report->xconfigure.height;
		wp.flags = wstate;
		lpLibCallback(TWINLIBCALLBACK_CONFIGURE,(LPARAM)wp.hwnd,
				0, (LPVOID)&wp);
		break;

	case ButtonPress:
		msgstr = "ButtonPress";
	    	msgposted =  DrvWindowsEvents(0L,0L,(LPVOID)report);
	    	break;
	case ButtonRelease:
		msgstr = "ButtonRelease";
	    	msgposted =  DrvWindowsEvents(0L,0L,(LPVOID)report);
	    	break;
	case MotionNotify:
		msgstr = "MotionNotify";
	    	msgposted =  DrvWindowsEvents(0L,0L,(LPVOID)report);
	    	break;

	case KeyPress:
		msgstr = "KeyPress";
	    	msgposted =  DrvHandleKeyboardEvents(0L,0L,(LPVOID)report);
	    	break;
	case KeyRelease:
		msgstr = "KeyRelease";
	    	msgposted =  DrvHandleKeyboardEvents(0L,0L,(LPVOID)report);
	    	break;

	case EnterNotify:
		msgstr = "EnterNotify";
	    	if (XFindContext(dp->display,report->xcrossing.window,
			dp->client_hwnd,(caddr_t *)&dwClientWin))
			break;
	    	XFindContext(dp->display,report->xcrossing.window,
			dp->window_style,(caddr_t *)&dwStyle);
	    	if ((dwStyle & WS_CAPTION) != WS_CAPTION) {
			XGetInputFocus(dp->display,&win,&revert_return);
			if (win != report->xcrossing.window) {
		    	XSetInputFocus(dp->display,report->xcrossing.window,
				revert_return,CurrentTime);
			}
	    	}
	    	break;

	case LeaveNotify:
		msgstr = "LeaveNotify";
	    	if (XFindContext(dp->display,report->xcrossing.window,
			dp->client_hwnd,(caddr_t *)&dwClientWin))
			break;
	    	XFindContext(dp->display,report->xcrossing.window,
			dp->window_style,(caddr_t *)&dwStyle);
	    	if ((dwStyle & WS_CAPTION) != WS_CAPTION) {
			XGetInputFocus(dp->display,&win_focus,&revert_return);
			if (win && (win != report->xcrossing.window)) {
		    		if (win != RootWindow(dp->display,dp->screen)) {
					XSetInputFocus(dp->display,win,
						revert_return,CurrentTime);
		    		}
		    		else {
					XSetInputFocus(dp->display,PointerRoot,
					0,CurrentTime);
		    		}
			}
			win = 0;
	    	}
	    	break;

	case FocusIn:
		msgstr = "FocusIn";
	case FocusOut:
		if(msgstr == 0)
			msgstr = "FocusOut";

		if (XFindContext(dp->display,report->xfocus.window,
			dp->client_hwnd,(caddr_t *)&dwClientWin))
		    break;
		hWndFrame = HIWORD(dwClientWin);
		lpLibCallback(TWINLIBCALLBACK_FOCUS,(LPARAM)hWndFrame,
				IFC_DRIVERFOCUS,
				(LPVOID)(report->type == FocusIn));
		msgposted--;
		break;

	case KeymapNotify:
		msgstr = "KeymapNotify";
		/* this could/should be used to update us on the current 
  		 * state of the keyboard.  Can it be different from what
		 * we know about it?  Yes, we could have keydown somewhere
		 * and keyup in our space...
		 */
		break;

	case VisibilityNotify:
		msgstr = "VisibilityNotify";
		/*
		 * The following function call prints the entire tree of
		 * windows known to X11.  It is commented out because it
		 * is far too wordy in most debugging situations.
		 */
		DrvPrintTree(dp, XRootWindow(dp->display, dp->screen));

		if (XFindContext(dp->display,
			report->xvisibility.window,
			dp->client_hwnd,
			(caddr_t *)&dwClientWin))
		    break;

		hWnd = (HWND)LOWORD(dwClientWin);
		switch (report->xvisibility.state) {
			case VisibilityUnobscured:
			case VisibilityPartiallyObscured:
				SetWF(hWnd,WFVISIBILITY);
				break;
			case VisibilityFullyObscured:
				ClearWF(hWnd,WFVISIBILITY);
				break;
		}
		msgposted--;
		break;

	case CreateNotify:
		msgstr = "CreateNotify";
		break;
	case DestroyNotify:
		msgstr = "DestroyNotify";
		break;

	case MappingNotify:
		msgstr = "MappingNotify";
		XRefreshKeyboardMapping((XMappingEvent *)report);
		break;

	case MapNotify:
		msgstr = "MapNotify";
#ifdef	DEBUG
		if (XFindContext(dp->display,report->xmap.window,
			dp->client_hwnd,(caddr_t *)&dwClientWin))
		    break;
		hWnd = HIWORD(dwClientWin);
#endif
		break;

	case UnmapNotify:
		msgstr = "UnmapNotify";
		if (XFindContext(dp->display,report->xunmap.window,
			dp->client_hwnd,(caddr_t *)&dwClientWin))
			break;
		hWnd = HIWORD(dwClientWin);
		ClearWF(hWnd, WFMAPPED);
		break;

	case CirculateNotify:
		msgstr = "CirculateNotify";
		break;

	case CirculateRequest:
		msgstr = "CirculateRequest";
		break;

	case NoExpose:
		msgstr = "NoExpose";
		break;

	default:
		msgstr = "UNKNOWN";
		break;

	} /* end switch */

	return msgposted;
}

static void DrvPrintTree(PRIVATEDISPLAY *dp, Window start)
{
    static indent = 0;
    Window root;
    Window parent;
    Window *children;
    unsigned int nchildren;
    Status rv;
    int i;
    char indent_str[80];
    DWORD dwClientWin;
    
    rv = XQueryTree(dp->display, 
		    start,
		    &root, &parent, &children, &nchildren);
    if (rv)
    {
	for (i = 0; i < indent; i++)
	    indent_str[i] = ' ';
	indent_str[i] = '\0';
	
	for (i = 0; i < nchildren; i++)
	{
	    XFindContext(dp->display, children[i], dp->client_hwnd,
			     (caddr_t *) &dwClientWin);
	    indent += 2;
	    DrvPrintTree(dp, children[i]);
	    indent -= 2;
	}

	XFree(children);
    }
}

