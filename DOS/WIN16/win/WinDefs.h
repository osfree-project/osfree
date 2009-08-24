/*
	@(#)WinDefs.h	2.26
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
 
#ifndef WinDefs__h
#define WinDefs__h

#include "kerndef.h"
#include "ObjEngine.h"

/* Functions for EnumAllWindows */
#define	EN_CHILD	0x0000
#define	EN_ALL		0x0001
#define	EN_TASK		0x0002

/* Property list element structure */
typedef struct tagPROPITEM {
    struct tagPROPITEM *lpNextProp;	/* ptr to the next */
    ATOM	atmProp;		/* property string */
    HANDLE	hHandle;		/* associaited handle */
    BOOL	bOriginalAtom;		/* =1, if app passed atom */
					/* needed for enumeration */
} PROPITEM;

typedef PROPITEM *LPPROPITEM;


/* Window info structure */
typedef struct tagWININFO {
    OBJHEAD	ObjHead;		/* generic object header */
    LPSTR	lpWindowName;		/* Window Name */
    WNDPROC	lpfnWndProc;		/* Window procedure */
    DWORD	dwStyle;		/* window style */
    DWORD	dwExStyle;		/* window extended style */
    DWORD	dwWinFlags;		/* internal window flags */
    HWND	hWndParent;		/* parent window handle */
    HWND	hWndSibling;		/* Sibling link */
    HWND	hWndChild;		/* Child link */
    HWND	hWndGroup;		/* Group link */
    HWND	hWndOwner;		/* owner for the popup */
    HWND	hWndLastChild;		/* last created child for group links */
    HMENU	hMenu;			/* menu handle */
    HMENU	hSysMenu;		/* system menu handle */
    HWND	hWndFrame;		/* frame window handle */
    HWND	hWndHZScroll;		/* horizontal scrollbar */
    HWND	hWndVTScroll;		/* vertical scrollbar */
    HWND	hWndIconTitle;		/* associated icon title window */
    HINSTANCE	hInstance;		/* creator's instance handle */
    HANDLE	hTask;			/* creator's task */
    WORD	wChildID;		/* ID for child windows */
    WORD	wWidth;			/* width of the client area */
    WORD	wHeight;		/* height of the client area */
    WORD	wXBorder;		/* width of a vertical border */
    WORD	wYBorder;		/* height of a horizontal border */
    RECT	rWnd;			/* window rectangle */
    RECT	rcNC;			/* non-client areas rect */
    POINT	ptMaxSize;		/* maximized size */
    POINT	ptMaxPosition;		/* maximized position (left-top) */
    POINT	ptMinPosition;		/* minimized position (left-top) */
    POINT	ptMinSize;		/* minimized size */
    DWORD	dwMinPosIndex;		/* row/col index for icon position */
    POINT	ptMinTrackSize;		/* minimal tracking size */
    POINT	ptMaxTrackSize;		/* maximal tracking size */
    RECT	rcNormalPosition;	/* restored position */
    DWORD	dwNormalStyle;		/* restored style */
    UINT	showCmd;		/* current show window state */
    DWORD       UpdateRegion;		/* update region... */
    LPVOID	lpDrvData;		/* driver private stuff */
    WORD	wMenuHeight;		/* menu area height in pixels */
    HDC		hDC;			/* store for OWNDC/CLASSDC/INPAINT DC */
    HWND	hChildFocus;		/* focus holder handle */
    HCLASS32	hWindowClass32;		/* ptr to CLASSINFO */
    LPPROPITEM	lpPropertyList;		/* prop list */
    DWORD	dwUserData;		/* user data (WIN32) */
    int		cbWndExtra;		/* window extra bytes */
    LPSTR	lpWndExtra;		/* pointer to extra space */
} WININFO;

typedef	WININFO	*LPWININFO;


/* Multi-window position info structure */
typedef struct tagMULTIWINPOS {
    OBJHEAD	ObjHead;		/* generic object header */
    int		nMaxNumber;		/* Max number of units */
    int		nUsed;			/* number of user units */
    LPWINDOWPOS lpWinPosInfo;		/* pointer to data */
} MULTIWINPOS;

typedef MULTIWINPOS *LPMULTIWINPOS;

void GetPhysicalRect(HWND,LPRECT);

typedef	LPWININFO	HWND32;
#define	GETHWND32	GETWININFO
#define	CHECKHWND32	CHECKWININFO

#define	GETHWND16(h32)    (HWND)((h32)->ObjHead.hObj)
#define	GETHWNDFRAME(h32) ((h32)->hWndFrame)

#define	ASSERT_HWND(hWnd32,hWnd,err) { \
		if (!(hWnd)) \
		    return (err); \
		if (!((hWnd32) = GETHWND32((hWnd)))) { \
		    ERRSTR((LF_ERROR,"***ERROR*** bad HWND %x %s:%d\n",(hWnd),__FILE__,__LINE__)); \
		    return (err); \
		}}

#define	WIN_GETDRVDATA(hWnd) (DWORD)GetWindowLong(hWnd,GWL_DRVDATA)

#define	Get32WindowFrame(hWnd32) (hWnd32->hWndFrame)
#define	GetFrameClient(hWnd) (HWND)GetWindow(hWnd,GW_CHILD)
#define	Get32FrameClient(hFrame32) (hFrame32->hWndChild)

/* Prototypes for external routines */
extern DWORD CalcBorders(DWORD,DWORD);
extern void CalcNCDimensions(LPRECT,DWORD,DWORD,WORD);
extern void CalcExpectedNC(LPRECT, DWORD, DWORD);
extern void CalcWMDimensions(LPRECT,DWORD,DWORD);
extern void DrawWindowFrame(HWND, BOOL);
extern LPSTR GetClassMenuName(HCLASS32);
extern UINT InternalGetClassName(HCLASS32,LPSTR,int);
extern HCLASS32 FindClass(LPCSTR, HINSTANCE);
extern void LockClass(HCLASS32,BOOL);
extern void InternalGetClassInfo(HCLASS32, LPWNDCLASS);
extern WORD MeasureWindowMenu(HWND, HWND);
extern HTASK GetTaskFromInstance(HINSTANCE);
extern void QueueSetFlags(HTASK, WORD);
extern int GetUpdateRgnEx(HWND,LPRECT,HRGN,BOOL);
extern void ModifyMenuBar(HWND,HWND,UINT);
extern HMENU ModifySystemMenu(HWND,UINT);
extern int SubtractRectFromRegion(HRGN, const LPRECT);
extern void DriverFlushXEvents(void);
extern void DrawIconTitle(HWND,UINT);
extern BOOL IsRegion(HRGN);
extern int AddRgnToUpdate(HWND,HRGN);
extern int AddRectToUpdate(HWND,const RECT *);
extern int SubtractRgnFromUpdate(HWND,HRGN);
extern int SubtractRectFromUpdate(HWND,const RECT *);
extern BOOL CreateSystemScrollbars(HWND,int);

/* Prototypes for the exported routines */
BOOL InitWindows();
BYTE GetWindowByte(HWND, int);
BYTE SetWindowByte(HWND, int, BYTE);
BOOL OrWindowByte(HWND, int, BYTE);
BOOL AndWindowByte(HWND, int, BYTE);
BOOL XorWindowByte(HWND, int, BYTE);
BOOL TestWindowByte(HWND, int, BYTE);
BOOL TestWF(HWND, DWORD);
DWORD GetWF(HWND);
void ClearWF(HWND, DWORD);
void SetWF(HWND, DWORD);
BOOL IsTopLevel(HWND);
void SetControlStyle(HWND, WORD);
LONG InternalSetText(HWND, LPSTR);
int  InternalGetText(HWND, LPSTR, int);
int  InternalGetTextLength(HWND);
void InternalFreeText(HWND);
void CalcClientRect(HWND, LPRECT);
void SetWindowMenuHeight(HWND, WORD);
void ChangeNCDimensions(HWND, LPRECT);
void GetNCRect(HWND, LPRECT);
BOOL NonEmptyNCRect(HWND);
HBRUSH GetClassBackgroundBrush(HWND);
HBRUSH GetControlBrush(HWND, HDC, WORD);
HWND GetTopLevelAncestor(HWND);
BOOL HaveInvisibleAncestor(HWND);
BOOL HaveDisabledAncestor(HWND);
void RedrawIconTitle(HWND);
void ShowIconTitle(HWND, BOOL);
BOOL InternalEraseBackground(HWND, HDC);
HWND InternalUpdateWindows(void);
HWND TWIN_InternalFocus(int, HWND, LONG);
void InvalidateWindow(HWND,BOOL,BOOL);
HWND CreateRootWindow();
BOOL InsertAsLastSibling(HWND32);
void RemoveFromList(HWND32);
void CalcSysScrollLocation(HWND32,WORD,WORD,LPRECT,BOOL);
void MinMaximize(HWND,UINT,BOOL);
HWND TWIN_GetTopLevelFrame(HWND);
HWND TWIN_FindAnotherOverlapped(HWND);

#define GetIconTitle(h) (GETHWND32(h))->hWndIconTitle

#endif /* WinDefs__h */
