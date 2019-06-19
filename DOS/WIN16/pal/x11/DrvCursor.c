/*
	@(#)DrvCursor.c	1.11
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
#include "Driver.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "Log.h"
#include "DrvDP.h"

static DWORD DrvCursorSetCursor(LPARAM,LPARAM,LPVOID);
static DWORD DrvCursorGetCursorPos(LPARAM,LPARAM,LPVOID);
static DWORD DrvCursorSetCursorPos(LPARAM,LPARAM,LPVOID);
DWORD DrvCursorSetClipCursor(LPARAM,LPARAM,LPVOID);
static DWORD DrvCursorGetClipCursor(LPARAM,LPARAM,LPVOID);
static DWORD DrvCursorDoNothing(LPARAM,LPARAM,LPVOID);

DWORD DrvCursorTab(void);

static TWINDRVSUBPROC DrvCursorEntryTab[] = {
	DrvCursorDoNothing,
	DrvCursorDoNothing,
	DrvCursorDoNothing,
	DrvCursorSetCursor,
	DrvCursorGetCursorPos,
	DrvCursorSetCursorPos,
	DrvCursorSetClipCursor,
	DrvCursorGetClipCursor,
	DrvCursorDoNothing,
	DrvCursorDoNothing
};

DWORD
DrvCursorTab(void)
{
#if defined(TWIN_RUNTIME_DRVTAB)
	DrvCursorEntryTab[0] = DrvCursorDoNothing;
	DrvCursorEntryTab[1] = DrvCursorDoNothing;
	DrvCursorEntryTab[2] = DrvCursorDoNothing;
	DrvCursorEntryTab[3] = DrvCursorSetCursor;
	DrvCursorEntryTab[4] = DrvCursorGetCursorPos;
	DrvCursorEntryTab[5] = DrvCursorSetCursorPos;
	DrvCursorEntryTab[6] = DrvCursorSetClipCursor;
	DrvCursorEntryTab[7] = DrvCursorGetClipCursor;
	DrvCursorEntryTab[8] = DrvCursorDoNothing;
	DrvCursorEntryTab[9] = DrvCursorDoNothing;
#endif
	return (DWORD)DrvCursorEntryTab;
}

static DWORD
DrvCursorSetCursor(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	PRIVATEDISPLAY *dp = GETDP();
	static BOOL fDefinedRootCursor = FALSE;
	Window frame_win;

	if (fDefinedRootCursor) {
		XUndefineCursor(dp->display,DefaultRootWindow(dp->display));
		fDefinedRootCursor = FALSE;
	}
	if ((frame_win = (Window)dwParam2))
		XDefineCursor(dp->display,frame_win,(Cursor)dwParam1);
	else {
		XDefineCursor(dp->display,DefaultRootWindow(dp->display),
			(Cursor)dwParam1);
		fDefinedRootCursor = TRUE;
	}
	return 1L;
}

static DWORD
DrvCursorGetCursorPos(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
        PRIVATEDISPLAY *dp = GETDP();
        Window root,child;
        int root_x,root_y,child_x,child_y;
        unsigned int key_mask;

	XQueryPointer(dp->display,
		DefaultRootWindow(dp->display),
		&root,&child,&root_x,&root_y,
		&child_x,&child_y,&key_mask);
	((LPPOINT)lpStruct)->x = root_x;
	((LPPOINT)lpStruct)->y = root_y;
	return 1L;
}

static DWORD
DrvCursorSetCursorPos(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
        PRIVATEDISPLAY *dp = GETDP();

	XWarpPointer(dp->display,None,
		DefaultRootWindow(dp->display),
		0,0,0,0,
		((LPPOINT)lpStruct)->x,((LPPOINT)lpStruct)->y);
	return 1L;
}

DWORD
DrvCursorSetClipCursor(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
        PRIVATEDISPLAY *dp = GETDP();
	LPRECT lprc = (LPRECT)lpStruct;

	if (lprc == NULL) {         /* freeing the cursor */
		dp->rcCursorClip.left = 0;
		dp->rcCursorClip.top = 0;
		dp->rcCursorClip.right = 
			XDisplayWidth(dp->display,dp->screen);
		dp->rcCursorClip.bottom = 
			XDisplayHeight(dp->display,dp->screen);
		dp->fCursorClipped = FALSE;
	}
        else {                      /* confine the cursor to given rect */
                dp->rcCursorClip.left = lprc->left;
                dp->rcCursorClip.top = lprc->top;
                dp->rcCursorClip.right = lprc->right;
                dp->rcCursorClip.bottom = lprc->bottom;
                dp->fCursorClipped = TRUE;
	}
        return 1L;
}

static DWORD
DrvCursorGetClipCursor(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
        PRIVATEDISPLAY *dp = GETDP();
        LPRECT lprc = (LPRECT)lpStruct;

	if (lprc) {
		lprc->left = dp->rcCursorClip.left;
		lprc->top = dp->rcCursorClip.top;
		lprc->right = dp->rcCursorClip.right;
		lprc->bottom = dp->rcCursorClip.bottom;
	}
	return 1L;
}

static DWORD
DrvCursorDoNothing(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return 1L;
}
