/*    
	UsrCursor.c	1.17
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
#include "Log.h"
#include "WinDefs.h"
#include "GdiObjects.h"
#include "Module.h"
#include "Resources.h"
#include "Driver.h"

typedef struct tagINTERNALCURINFO {
    HCURSOR hCursor;   /* hCursor of windows that have no cursor */
			/* or temporary cursor to be overridden   */
    HWND   hWnd;	/* top-level frame associated with cursor */
    LPVOID lpDrvData;   /* cursor drv data	*/
    int    nCount;	/* display count;	*/
#ifdef LATER
			/* if (nCount < 0), cursor is not visible */
#endif
} INTERNALCURINFO;

void TWIN_SetCursorWindow(HWND);
HWND TWIN_GetCursorWindow(void);

static INTERNALCURINFO InternalCursor;
static HCURSOR hNullCursor;

static BOOL TWIN_DefineCursorWindow();
    
HCURSOR WINAPI
SetCursor(HCURSOR hCursor)
{
    HCURSOR hLast;
    LPVOID lpWinData;
    LPCURSORINFO lpCursorInfo;
    LRESULT ret;

    APISTR((LF_APICALL,"SetCursor(HCURSOR=%x)\n",hCursor));

    hLast = InternalCursor.hCursor;
    if (!TWIN_DefineCursorWindow() && (hCursor == hLast)) {
        APISTR((LF_APIRET,"SetCursor: returns HCURSOR %x\n",hCursor));
	return hCursor;
    }

    if (hCursor == 0) {
	ret = DRVCALL_CURSORS(PCH_HIDECURSOR,0L,0L,0L);
	if (ret) {
	    InternalCursor.hCursor = 0;
	    InternalCursor.lpDrvData = 0;
	    InternalCursor.nCount = -1;
            APISTR((LF_APIRET,"SetCursor: returns HCURSOR %x\n",hLast));
	    return hLast;
	}
	else  	/* driver couldn't do it */
	    if (!hNullCursor)
		hNullCursor = LoadCursor(0,"NULLCURSOR");
   }

    if (!(lpCursorInfo = (hCursor)?GETCURSORINFO(hCursor):
				GETCURSORINFO(hNullCursor))) {
        APISTR((LF_APIFAIL,"SetCursor: returns HCURSOR %x\n",0));
	return 0;
    }

    InternalCursor.hCursor = hCursor;
    InternalCursor.lpDrvData = lpCursorInfo->lpCurData;
    InternalCursor.nCount = 0;

    lpWinData = (LPVOID)((InternalCursor.hWnd)?
			(LPVOID)WIN_GETDRVDATA(InternalCursor.hWnd):NULL);

    DRVCALL_CURSORS(PCH_SETCURSOR,InternalCursor.lpDrvData,lpWinData,0L);
    if (hLast == 0)
	DRVCALL_CURSORS(PCH_SHOWCURSOR,0L,0L,0L);

    RELEASECURSORINFO(lpCursorInfo);

    APISTR((LF_APIRET,"SetCursor: returns HCURSOR %x\n",hLast));
    return hLast;
}

int WINAPI
ShowCursor(BOOL fShow)
{
    LPVOID lpWinData;
    LPCURSORINFO lpCursorInfo;

    APISTR((LF_API,"ShowCursor: %x\n",fShow));

    if (fShow) {
	if (++(InternalCursor.nCount) == 0)
	    if (!(DRVCALL_CURSORS(PCH_SHOWCURSOR,0L,0L,0L)) &&
				InternalCursor.hCursor) {
		TWIN_DefineCursorWindow();
		lpWinData = (LPVOID)((InternalCursor.hWnd)?
			(LPVOID)WIN_GETDRVDATA(InternalCursor.hWnd):
			NULL);
		DRVCALL_CURSORS(PCH_SETCURSOR,InternalCursor.lpDrvData,
					lpWinData,0L);
	    }
    }
    else {
	if (--(InternalCursor.nCount) == -1) {
	    if (!DRVCALL_CURSORS(PCH_HIDECURSOR,0L,0L,0L)) {
		if (!hNullCursor)
		    hNullCursor = LoadCursor(0,"NULLCURSOR");
		lpCursorInfo = GETCURSORINFO(hNullCursor);
		TWIN_DefineCursorWindow();
		lpWinData = (LPVOID)((InternalCursor.hWnd)?
			(LPVOID)WIN_GETDRVDATA(InternalCursor.hWnd):
			NULL);
		DRVCALL_CURSORS(PCH_SETCURSOR,lpCursorInfo->lpCurData,
					lpWinData,0L);
		RELEASECURSORINFO(lpCursorInfo);
	    }
	}
    }
	    
    return InternalCursor.nCount;
}

HCURSOR WINAPI
GetCursor(void)
{
    APISTR((LF_API,"GetCursor: \n"));

    return InternalCursor.hCursor;
}

#ifdef	TWIN32
BOOL	WINAPI
#else
void	WINAPI
#endif
GetCursorPos(POINT FAR *lppt)
{
    DRVCALL_CURSORS(PCH_GETCURSORPOS,0,0,(LPVOID)lppt);
    APISTR((LF_API,"GetCursorPos: returning %x,%x\n",lppt->x,lppt->y));
#ifdef	TWIN32
	return (TRUE);
#endif
}

#ifdef	TWIN32
BOOL	WINAPI
#else
void	WINAPI
#endif
SetCursorPos(int x, int y)
{
    POINT pt;

    APISTR((LF_API,"SetCursorPos: setting to %x,%x\n",x,y));
    pt.x = x;
    pt.y = y;
    DRVCALL_CURSORS(PCH_SETCURSORPOS,0,0,(LPVOID)&pt);

#ifdef	TWIN32
	return (TRUE);
#endif
}

void  WINAPI
ClipCursor(const RECT FAR* lprc)
{
    if (lprc)
	APISTR((LF_API,"ClipCursor: setting to %x,%x,%x,%x\n",
		lprc->left,lprc->top,lprc->right,lprc->bottom));
    else
	APISTR((LF_API,"ClipCursor: setting to NULL\n"));

    DRVCALL_CURSORS(PCH_SETCLIPCURSOR,0,0,(LPVOID)lprc);
}

void  WINAPI
GetClipCursor(LPRECT lprc)
{
    APISTR((LF_APICALL,"GetClipCursor(LPRECT=%p)\n",lprc));

    if (!lprc) {
         APISTR((LF_APIFAIL,"GetClipCursor: returns void\n"));
	return;
    }

    DRVCALL_CURSORS(PCH_GETCLIPCURSOR,0,0,(LPVOID)lprc);

    APISTR((LF_APIRET,"GetClipCursor: returning void\n"));
}

#ifdef TWIN32
HCURSOR WINAPI
CopyCursor(HCURSOR hCur)
{
    APISTR((LF_APISTUB,"CopyCursor(HCURSOR=%x)\n", hCur));

    return (HCURSOR)NULL;
}

#else
HCURSOR WINAPI
CopyCursor(HINSTANCE hInst, HCURSOR hCur)
{
    APISTR((LF_APISTUB,"CopyCursor(HINSTANCE=%x,HCURSOR=%x)\n",
	hInst,hCur));

    return (HCURSOR)NULL;
}
#endif

static BOOL
TWIN_DefineCursorWindow()
{
    HWND hWnd,hWndFrame;

    hWnd = GetActiveWindow();
    /* Because someone didn't fixup Active window correctly */
    /* when a window is destroyed, e.g., mdidemo ljw 5/9/96 */
    if (!IsWindow(hWnd)) {
	hWnd = 0;
    }
    else {
	hWnd = GetTopLevelAncestor(hWnd);
	if ((hWndFrame = GetWindowFrame(hWnd)))
	    hWnd = hWndFrame;
    }
    if (InternalCursor.hWnd == hWnd) {
	return FALSE;
    }
    else {
	InternalCursor.hWnd = hWnd;
	return TRUE;
    }
}

void
TWIN_SetCursorWindow(HWND hWnd)
{
    if (IsWindow(hWnd))
	InternalCursor.hWnd = hWnd;
    else
	InternalCursor.hWnd = (HWND)0;
}

HWND
TWIN_GetCursorWindow(void)
{
    return InternalCursor.hWnd;
}
