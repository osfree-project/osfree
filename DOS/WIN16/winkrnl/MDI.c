/*    
	MDI.c	2.37
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
#include "WinDefs.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define	MWD_LPMDICLIENT	16	/* Here we reserve first 16 bytes, because */
				/* QuattroPro just writes into the first 4 */
				/* and maybe more... (ugly stuff!!!)	   */
#define CWM_DELETE 0
#define CWM_APPEND 1
#define CWM_UPDATE 2

#define	MSF_CHILDMAXIMIZED	0x0001	/* MDI child is maximized */
#define	MSF_CHILDMINIMIZED	0x0002	/* MDI child is minimized */

#define MSB_VERT	0x0001		/* vertical scrollbar is active */
#define MSB_HORZ	0x0002		/* horizontal scrollbar is active */
#define MSB_BOTH	0x0003		/* both scrollbars are active */

#define NEXTWINDSTRING "Nex&t\tCtrl+F6"

typedef	struct tagMDIClient
{
    HMENU	hMenu;			/* application's main menu */
    HMENU	hWindowMenu;		/* application's "Window" menu */
    HWND	hActiveChild;		/* currently active child */
    int		nSeparatorPos;		/* separator pos. in "Window" menu */
    UINT	idFirstChild;		/* first child ID */
    int		nNumChildren;		/* number of MDI children */
    UINT	uiStateFlags;		/* state flags */
    int		nChildPosIndex;		/* position to place next child */
    RECT	rcLogicalRect;		/* client rect (for scrolling) */
    BOOL        bScrollInProgress;
    BOOL        bNoScrollBars;          /* if neither of WS_HSCROLL or WS_VSCROLL... */
} MDICLIENT;

typedef	MDICLIENT *LPMDICLIENT;

void TileChildWindows(HWND,WORD);	/* undocumented API */
void CascadeChildWindows(HWND,WORD);	/* undocumented API */

extern void ModifyMenuBar(HWND,HWND,UINT);

static BOOL ChangeWindowMenu(HWND, UINT);
static void CalcMDIScrollPos(HWND);
static void RecalcMDIClientRange(HWND,LPRECT); 
static void ScrollChildren(HWND,UINT,UINT,int);

char buf[0x100];

LRESULT
DefFrameProc(HWND hWnd, HWND hWndMDIClient,
	     UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT dwResult = 0;
    BOOL bCallDefProc = FALSE;
    LPMDICLIENT lp;
    HWND hWndChild;
    LPSTR lpString;
    int nLength;
    DWORD dwStyle;

    APISTR((LF_APICALL,
	"DefFrameProc(HWND=%x,HWND=%x,UINT=%x,WPARAM=%x,LPARAM=%lx)\n",
         hWnd,hWndMDIClient,uMsg,wParam,lParam));
 
    if (!hWndMDIClient) {
	dwResult =  DefWindowProc(hWnd,uMsg,wParam,lParam);
	APISTR((LF_APIRET,"DefFrameProc: returns LRESULT %lx\n",dwResult));
	return dwResult;
    }

    switch (uMsg) {
	case WM_COMMAND:
	    if (!(lp = (LPMDICLIENT)GetWindowLong(hWndMDIClient,
			MWD_LPMDICLIENT))) {
		APISTR((LF_APIFAIL,"DefFrameProc: returns LRESULT 0\n"));
		return 0;
	    }
	    if (((UINT)GET_WM_COMMAND_ID(wParam,lParam) >= lp->idFirstChild) &&
		((UINT)GET_WM_COMMAND_ID(wParam,lParam) <
				(lp->idFirstChild+lp->nNumChildren))) {
		for (hWndChild = GetWindow(hWndMDIClient,GW_CHILD);
		     hWndChild && (GET_WM_COMMAND_ID(wParam,lParam) !=
					GetWindowID(hWndChild));
		     hWndChild = GetWindow(hWndChild,GW_HWNDNEXTSIB));
		if (!hWndChild) {
			APISTR((LF_APIFAIL,"DefFrameProc: returns LRESULT 0\n"));
		    return 0L;
		}
		SendMessage(hWndMDIClient,WM_MDIACTIVATE,
					GET_WM_MDIACTIVATE_MPS(0,0,hWndChild));
		if (IsIconic(hWndChild))
		    SendMessage(hWndChild,WM_SYSCOMMAND,SC_RESTORE,0L);
		break;
	    }
	    switch (LOWORD(wParam) & 0xfff0) {
		case SC_RESTORE:
		case SC_CLOSE:
		case SC_NEXTWINDOW:
		case SC_MINIMIZE:
		    if (TestWF(hWnd,WFCHILDMAXIMIZED)) {
			for (hWndChild = GetWindow(hWndMDIClient,GW_CHILD);
		 	    hWndChild &&
			    !(GetWindowLong(hWndChild,GWL_STYLE) & WS_MAXIMIZE);
			    hWndChild = GetWindow(hWndChild,GW_HWNDNEXTSIB));
			if (!hWndChild)
	 		    return 0L;
			dwResult = SendMessage(hWndChild,
			    WM_SYSCOMMAND,
			    LOWORD(wParam),
			    lParam);
		    }
		    break;
	    }
	    break;

	case WM_GETTEXT:
	    dwResult = SendMessage(hWndMDIClient,WM_MDIGETACTIVE,0,0L);
#ifndef TWIN32
	    if (HIWORD(dwResult)) {	/* child window is maximized */
		hWndChild = (HWND)LOWORD(dwResult);
		dwStyle = GetWindowLong(hWndChild,GWL_STYLE);
#else	/* TWIN32 */
	    hWndChild = (HWND)(dwResult);
	    if (hWndChild)
	        dwStyle = GetWindowLong(hWndChild,GWL_STYLE);
	    else
		dwStyle = 0;
	    if (dwStyle & WS_MAXIMIZE) {	/* child window is maximized */
#endif	/* TWIN32 */
		if (((dwStyle & WS_CAPTION) ==
				WS_CAPTION) && (dwStyle & WS_MAXIMIZE)) {
		    InternalGetText(hWnd,buf,0x100);
		    strcat(buf," - [");
		    nLength = strlen(buf);
		    lpString = buf + nLength;
		    GetWindowText(hWndChild,lpString,0x100-nLength);
		    strcat(buf,"]");
		    dwResult = (LRESULT)strncpy((LPSTR)lParam,buf,(int)wParam);
		    break;
		}
		else {
		    bCallDefProc = TRUE;
		    break;
		}
	    }
	    else 
		bCallDefProc = TRUE;
#ifdef	LATER
	/*
	    if (!(An MDI child is maximized) ||
		(MDI child has no caption)) {
		bCallDefProc = TRUE;
	    }
	    else {
		Put the frame window's caption in a buffer;
		Append the MDI child's caption to the buffer;
		dwResult = Address of the buffer;
	    }
	*/
#endif
	    break;

	case WM_MENUCHAR:
#ifdef	LATER
	    if (User pressed dash) {
		if (An MDI child is maximized)
		    dwResult = MAKELONG(0,2);
		else
		    if (an MDI child is active) {
			PostMessage(hWndActiveChild,WM_SYSCOMMAND,
			    SC_KEYMENU, MAKELONG('-',0));
			dwResult = MAKELONG(0,1);
		    }
	    }
#endif
	    bCallDefProc = TRUE;
	    break;

	case WM_NCACTIVATE:
	    SendMessage(hWndMDIClient,WM_NCACTIVATE,wParam,lParam);
	    bCallDefProc = TRUE;
	    break;

#ifdef	LATER
	/*
	case WM_NEXTMENU:
	    if ((Frame window is maximized) &&
		(An MDI child is active) &&
		!(An MDI child is maximized)) {
		if (((left arrow pressed) && (frame menu is active))
		    || ((right arrow pressed) &&
			(frame's system menu is active))) {
		    dwResult = MAKELONG(hMenuOfActiveChildsSysMenu,
					hWndOfActiveChild);
		    break;
		}
	    }
	    dwResult = 0;
	    break;
	*/
#endif

	case WM_SETFOCUS:
	    SetFocus(hWndMDIClient);
	    break;

	case WM_SIZE:
	    MoveWindow(hWndMDIClient,0,0,LOWORD(lParam),
			HIWORD(lParam),TRUE);
	    bCallDefProc = TRUE;
	    break;

	default:
	    bCallDefProc = TRUE;
	    break;
    }
    if (bCallDefProc)
	dwResult = DefWindowProc(hWnd,uMsg,wParam,lParam);

    APISTR((LF_APIRET,"DefFrameProc: returns LRESULT %x\n",dwResult));
    return dwResult;
}

LRESULT
DefMDIChildProc(HWND hWndMDIChild, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT dwResult = 0;
    BOOL bCallDefProc = TRUE;
    LPMDICLIENT lp;
    HWND hWndMDIClient;
    HWND hWndActiveChild;

    APISTR((LF_APICALL,
	"DefMDIChildProc(HWND=%x,UINT=%x,WPARAM=%x,LPARAM=%lx)\n",
	hWndMDIChild,uMsg,wParam,lParam));

    switch (uMsg) {

	case WM_CHILDACTIVATE:
	    if (!(hWndMDIClient = GetParent(hWndMDIChild))) {
		APISTR((LF_APIFAIL,"DefMDIChildProc: returns LRESULT 0\n"));
		return 0;
	    }
	    if (!(lp = (LPMDICLIENT)GetWindowLong(hWndMDIClient,
			MWD_LPMDICLIENT))) {
		APISTR((LF_APIFAIL,"DefMDIChildProc: returns LRESULT 0\n"));
		return 0;
	    }
	    hWndActiveChild = lp->hActiveChild;
	    if (hWndActiveChild == hWndMDIChild) {
		APISTR((LF_APIFAIL,"DefMDIChildProc: returns LRESULT 0\n"));
		return 0;
	    }
	    lp->hActiveChild = hWndMDIChild;
	    if (hWndActiveChild) {
		SendMessage(hWndActiveChild,WM_NCACTIVATE,0,0);
		FORWARD_WM_MDIACTIVATE(hWndActiveChild,0,hWndActiveChild,
			hWndMDIChild,SendMessage);
	    }
	    SendMessage(hWndMDIChild,WM_NCACTIVATE,(WPARAM)1,0L);
	    ChangeWindowMenu(hWndMDIClient,CWM_UPDATE);
	    FORWARD_WM_MDIACTIVATE(hWndMDIChild,1,hWndMDIChild,
			hWndActiveChild,SendMessage);
	    SetFocus(hWndMDIChild);
	    APISTR((LF_APIRET,"DefMDIChildProc: returns LRESULT 0\n"));
	    return 0;

	case WM_CLOSE:
	    SendMessage(GetParent(hWndMDIChild),WM_MDIDESTROY,
		(WPARAM)hWndMDIChild,0L);
	    bCallDefProc = FALSE;
	    break;

	case WM_NCCREATE:
	    SetWindowPos(hWndMDIChild,HWND_TOP,0,0,0,0,
			SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
	    break;

	case WM_GETMINMAXINFO:
#ifdef	LATER
	    DoNormalProcessingForChildren;
#endif
	    break;

	case WM_ICONERASEBKGND:
#ifdef	LATER
	    if (An MDI child is maximized) {
		dwResult = 0;
		bCallDefProc = FALSE;
	    }
	    else
		bCallDefProc = TRUE;
#endif
	    break;

	case WM_MENUCHAR:
	    SendMessage(GetParent(GetParent(hWndMDIChild)),
			WM_SYSCOMMAND,
			SC_KEYMENU,
			GET_WM_MENUCHAR_CHAR(wParam,lParam));
	    dwResult = MAKELONG(0,1);
	    bCallDefProc = FALSE;
	    break;

	case WM_MOVE:
#ifdef	LATER
	    if (!IsZoomed(hWndMDIChild))
		calc new scrollbar ranges for hWndMDIClient;
#endif
	    bCallDefProc = TRUE;
	    break;

#ifdef	LATER
	case WM_NEXTMENU:
	    if (left arrow pressed)
		dwResult = MAKELONG(hMenuFrameSysMenu,hWndFrame);
	    else
		dwResult = MAKELONG(hMenuFramesTopLevelMenu,hWndFrame);
	    bCallDefProc = FALSE;
	    break;
#endif

	case WM_SETTEXT:
	    DefWindowProc(hWndMDIChild,uMsg,wParam,lParam);
	    ChangeWindowMenu(GetParent(hWndMDIChild),CWM_UPDATE);

#ifdef	LATER
	/*
	    if (hWndMDIChild is maximized)
		redraw frame's non-client area;
	    break;
	*/
#endif
	    APISTR((LF_APIRET,"DefMDIChildProc: returns LRESULT 0\n"));
	    return 0;

	case WM_SIZE:
#ifdef	LATER
	/*
	    Do normal processing for the MDI child
		this includes determining if scrollbars are necessary
		in MDI client's window

	    That's what J.Richter says; we do it in WM_WINDOWPOSCHANGED.
	*/
#endif
	    break;

	case WM_WINDOWPOSCHANGED:
	    CalcMDIScrollPos(GetParent(hWndMDIChild));
	    break;

	case WM_SYSCOMMAND:
	    switch (wParam & 0xfff0) {

		case SC_NEXTWINDOW:
		    SendMessage(GetParent(hWndMDIChild),WM_MDINEXT,
			(WPARAM)hWndMDIChild,0);
		    break;

		case SC_CLOSE:
		    SendMessage(hWndMDIChild,WM_CLOSE,0,0);
		    return 0;

		default:
		    bCallDefProc = TRUE;
	    }
	    break;

    }

    if (bCallDefProc)
	dwResult = DefWindowProc(hWndMDIChild,uMsg,wParam,lParam);
 
    APISTR((LF_APIRET,"DefMDIChildProc: returns LRESULT %x\n",dwResult));
    return dwResult;
}

static WNDPROC lpMDIClientBinToNat = 0;

LRESULT
DefMDICLIENTProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPMDICLIENT lp;
    LPMDICREATESTRUCT lpmcs;
    HWND hWndMDIChild;
    LPCLIENTCREATESTRUCT lpccs;
    UINT uiChildID;
    HWND hWndNext;
    int nUnit;
    int i;
    POINT pt;
    HDWP hDWP;
    RECT rcRectClient;
    int cxClient,cyClient;
    DWORD dwStyle;
    HCLASS32 hMDIClientClass32;
    HMENU hMenuWnd, hMenuFrame, hSysMenu;
    LRESULT rc;

    APISTR((LF_APICALL,
	"DefMDICLIENTProc(HWND=%x,UINT=%x,WPARAM=%x,LPARAM%lx)\n",
	hWnd,uMsg,wParam,lParam));
 
    switch (uMsg) {
	case WM_NCCREATE:
	    if (!(lp = (LPMDICLIENT)WinMalloc(sizeof(MDICLIENT)))) {
	        APISTR((LF_APIFAIL,"DefMDIClientProc: returns LRESULT 0\n"));
	        return 0;
	    }
	    memset((LPVOID)lp,'\0',sizeof(MDICLIENT));
	    SetWindowLong(hWnd,MWD_LPMDICLIENT,(DWORD)lp);
	    SetWF(hWnd,WFMDICLIENT);
	    lpccs = (LPCLIENTCREATESTRUCT)
			((LPCREATESTRUCT)lParam)->lpCreateParams;
	    if ((lp->hMenu = GetMenu(hWnd)))
		LOCKGDI(lp->hMenu);
	    if (IsMenu(lpccs->hWindowMenu)) {
		lp->hWindowMenu = lpccs->hWindowMenu;
		LOCKGDI(lp->hWindowMenu);
		lp->nSeparatorPos = GetMenuItemCount(lp->hWindowMenu);
            }
	    dwStyle = GetWindowLong(hWnd, GWL_STYLE);
	    lp->idFirstChild = lpccs->idFirstChild;
            lp->bNoScrollBars = (! ((dwStyle & WS_VSCROLL) || (dwStyle & WS_HSCROLL)));
	    APISTR((LF_APIRET,"DefMDIClientProc: returns LRESULT 0\n"));
	    return 1L;

	case WM_CREATE:
	    ShowScrollBar(hWnd,SB_BOTH,FALSE);
	    break;

	case WM_DESTROY:
	    if (!(lp = (LPMDICLIENT)GetWindowLong(hWnd,MWD_LPMDICLIENT))) {
	        APISTR((LF_APIFAIL,"DefMDIClientProc: returns LRESULT 0\n"));
		return 0L;
	    }
	    if (lp->hMenu)
		UNLOCKGDI(lp->hMenu);
	    if (lp->hWindowMenu)
		UNLOCKGDI(lp->hWindowMenu);
	    WinFree((LPSTR)lp);
	    SetWindowLong(hWnd,MWD_LPMDICLIENT,0L);
	    APISTR((LF_APIRET,"DefMDIClientProc: returns LRESULT 1\n"));
	    return 1L;

	case WM_SIZE:
	    if (TestWF(GetTopLevelAncestor(hWnd),WFCHILDMAXIMIZED)) {
		lp = (LPMDICLIENT)GetWindowLong(hWnd,MWD_LPMDICLIENT);
		hWndMDIChild = lp->hActiveChild;
		if (IsZoomed(hWndMDIChild)) {	/* sanity check */
		    cxClient = LOWORD(lParam);
		    cxClient += 2 * GetSystemMetrics(SM_CXFRAME);
		    cyClient = HIWORD(lParam);
		    cyClient += 2 * GetSystemMetrics(SM_CYFRAME) +
				GetSystemMetrics(SM_CYCAPTION);
		    SetWindowPos(hWndMDIChild,(HWND)0,0,0,
			cxClient,cyClient,
			SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOZORDER);
		}
	    }
	    else
		CalcMDIScrollPos(hWnd);
	    break;

	case WM_PARENTNOTIFY:
	    if (GET_WM_PARENTNOTIFY_MSG(wParam,lParam) == WM_LBUTTONDOWN) {
		pt.x = GET_WM_PARENTNOTIFY_X(wParam,lParam);
		pt.y = GET_WM_PARENTNOTIFY_Y(wParam,lParam);
		if ((hWndMDIChild = ChildWindowFromPoint(hWnd,pt)) != hWnd) {
		    BringWindowToTop(hWndMDIChild);
		}
	    }
	    APISTR((LF_APIRET,"DefMDIClientProc: returns LRESULT 0\n"));
	    return 0L;

	case WM_MDICREATE:
	    if (!(lp = (LPMDICLIENT)GetWindowLong(hWnd,MWD_LPMDICLIENT))
		|| !(lpmcs = (LPMDICREATESTRUCT)lParam)) {
	        APISTR((LF_APIFAIL,"DefMDIClientProc: returns LRESULT 0\n"));
		return 0;
	    }
	    if (TestWF(GetTopLevelAncestor(hWnd),WFCHILDMAXIMIZED))
		SendMessage(lp->hActiveChild,WM_SYSCOMMAND,SC_RESTORE,0L);
	    nUnit = GetSystemMetrics(SM_CYCAPTION) + 
			GetSystemMetrics(SM_CYFRAME) - 1;
	    GetClientRect(hWnd,&rcRectClient);
	    cxClient = rcRectClient.right - rcRectClient.left;
	    cyClient = rcRectClient.bottom - rcRectClient.top;
	    if (lpmcs->x == (int)CW_USEDEFAULT) {
		lpmcs->x = lp->nChildPosIndex * nUnit;
		lpmcs->y = lp->nChildPosIndex * nUnit;
		lp->nChildPosIndex = (++lp->nChildPosIndex)%4;
	    }
	    if (lpmcs->cx == (int)CW_USEDEFAULT) {
		lpmcs->cx = cxClient - 3*nUnit;
		lpmcs->cy = cyClient - 3*nUnit;
	    }
	    lpmcs->style &= ~WS_POPUP;
	    if (!(hWndMDIChild =
		CreateWindow(lpmcs->szClass,
			     lpmcs->szTitle,
			     lpmcs->style | WS_VISIBLE |
				WS_CHILD|WS_CLIPSIBLINGS|
				WS_SYSMENU|
				WS_CAPTION|WS_THICKFRAME|
				WS_MINIMIZEBOX|WS_MAXIMIZEBOX,
			     lpmcs->x,
			     lpmcs->y,
			     lpmcs->cx,
			     lpmcs->cy,
			     hWnd,
			     (HMENU)(lp->idFirstChild+lp->nNumChildren),
			     lpmcs->hOwner,
			     (LPVOID)lpmcs))) {
	        APISTR((LF_APIFAIL,"DefMDIClientProc: returns LRESULT 0\n"));
		return 0;
	    }
	    BringWindowToTop(hWndMDIChild);
	    hSysMenu = GetWindowSysMenu(hWndMDIChild);
	    ModifyMenu(hSysMenu,SC_TASKLIST,MF_BYCOMMAND,SC_NEXTWINDOW,
			NEXTWINDSTRING);
	    SetWF(hWndMDIChild,WFMDICHILD);
	    SendMessage(hWnd,WM_MDIACTIVATE,
			GET_WM_MDIACTIVATE_MPS(0,0,hWndMDIChild));
	    lp->nNumChildren++;
	    ChangeWindowMenu(hWnd,CWM_UPDATE);
	    APISTR((LF_APIRET,"DefMDIClientProc: returns LRESULT %x\n",
		hWndMDIChild));
	    return (LPARAM)hWndMDIChild;
			     
	case WM_MDIDESTROY:
	    if (!(lp = (LPMDICLIENT)GetWindowLong(hWnd,MWD_LPMDICLIENT))) {
	        APISTR((LF_APIFAIL,"DefMDIClientProc: returns LRESULT 0\n"));
		return 0;
	    }
	    hWndMDIChild = (HWND)wParam;
	    uiChildID = (UINT)GetWindowID(hWndMDIChild);
	    if ((uiChildID < lp->idFirstChild) ||
		(uiChildID >= lp->idFirstChild+lp->nNumChildren)) {
	        APISTR((LF_APIFAIL,"DefMDIClientProc: returns LRESULT 0\n"));
		return 0L;
	    }
	    if (hWndMDIChild == lp->hActiveChild) {
	        if (lp->nNumChildren == 1)
		    hWndNext = (HWND)0;
	        else
		    for (hWndNext = GetWindow(hWndMDIChild,GW_HWNDNEXT);;
			hWndNext = (hWndNext)?GetWindow(hWndNext,GW_HWNDNEXT):
				GetWindow(GetParent(hWndMDIChild),
					GW_CHILD)) {
			if (hWndNext && (GetWindow(hWndNext,GW_OWNER) == 0))
			    break;
		    }
		if (GetWindowStyle(hWndMDIChild) & WS_MAXIMIZE) {
		    ModifyMenuBar(GetParent(hWnd),hWndMDIChild,SW_RESTORE);
		    DrawMenuBar(GetParent(hWnd));
		    ClearWF(GetParent(hWnd),WFCHILDMAXIMIZED);
		}
		if (hWndNext)
		    BringWindowToTop(hWndNext);
		else {
		    FORWARD_WM_MDIACTIVATE(hWndMDIChild,0,0,
			hWndMDIChild,SendMessage);
		    lp->hActiveChild = 0;
		}
	    }
	    DestroyWindow(hWndMDIChild);
	    for (i=uiChildID-lp->idFirstChild; i<lp->nNumChildren-1; i++) {
		hWndMDIChild = GetDlgItem(hWnd,lp->idFirstChild+i+1);
		SetWindowID(hWndMDIChild,lp->idFirstChild+i);
	    }
	    lp->nNumChildren--;
	    CalcMDIScrollPos(hWnd);
	    ChangeWindowMenu(hWnd,CWM_UPDATE);
	    break;

/* these two should in fact be in kerndef. For now they are identical */
#ifndef	TWIN32
#define	GET_WM_MDIACTIVATE_CLIENT_HWNDACTIVATE(wp, lp)	(HWND)(wp)
#else
#define	GET_WM_MDIACTIVATE_CLIENT_HWNDACTIVATE(wp, lp)	(HWND)(wp)
#endif

	case WM_MDIACTIVATE:
	    if (!(lp = (LPMDICLIENT)GetWindowLong(hWnd,MWD_LPMDICLIENT))) {
	        APISTR((LF_APIFAIL,"DefMDIClientProc: returns LRESULT 0\n"));
		return 0;
	    }
	    if (lp->hActiveChild && (lp->hActiveChild != 
		GET_WM_MDIACTIVATE_CLIENT_HWNDACTIVATE(wParam,lParam))) { 
#ifndef LATER
		if (TestWF(GetTopLevelAncestor(hWnd),WFCHILDMAXIMIZED)) 
		    MinMaximize(
			GET_WM_MDIACTIVATE_CLIENT_HWNDACTIVATE(wParam,lParam),
			SW_MAXIMIZE,TRUE); 
#endif
		rc = BringWindowToTop(
			GET_WM_MDIACTIVATE_CLIENT_HWNDACTIVATE(wParam,lParam));
	        APISTR((LF_APIRET,"DefMDIClientProc: returns LRESULT %x\n",rc));
		return rc;
	    }
	    APISTR((LF_APIRET,"DefMDIClientProc: returns LRESULT 1\n"));
	    return 1L;

	case WM_MDIRESTORE:
	    if (!(lp = (LPMDICLIENT)GetWindowLong(hWnd,MWD_LPMDICLIENT))) {
	        APISTR((LF_APIRET,"DefMDIClientProc: returns LRESULT 0\n"));
		return 0;
	    }
	    hWndMDIChild = (HWND)wParam;
	    if (hWndMDIChild != lp->hActiveChild)
		BringWindowToTop(hWndMDIChild);
	    dwStyle = GetWindowLong(hWndMDIChild,GWL_STYLE);
	    if (dwStyle & (WS_MAXIMIZE | WS_MINIMIZE))
		SendMessage(lp->hActiveChild,WM_SYSCOMMAND,SC_RESTORE,0L);

	    APISTR((LF_APIRET,"DefMDIClientProc: returns LRESULT 0\n"));
	    return 0L;

	case WM_MDINEXT:
	    if (!(lp = (LPMDICLIENT)GetWindowLong(hWnd,MWD_LPMDICLIENT))) {
	        APISTR((LF_APIFAIL,"DefMDIClientProc: returns LRESULT 0\n"));
		return 0;
	    }
	    hWndMDIChild = (HWND)wParam;
	    if ((hWndNext = GetWindow(hWndMDIChild,GW_HWNDNEXT))) {
		hDWP = BeginDeferWindowPos(2);
		if (lp->nNumChildren > 2)
		    DeferWindowPos(hDWP,hWndMDIChild,HWND_BOTTOM,0,0,0,0,
				SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
		DeferWindowPos(hDWP,hWndNext,HWND_TOP,0,0,0,0,
			SWP_NOSIZE|SWP_NOMOVE);
		EndDeferWindowPos(hDWP);
	    }
	    break;

	case WM_MDIMAXIMIZE:
	    break;

	case WM_MDIICONARRANGE:
	    ArrangeIconicWindows(hWnd);
	    APISTR((LF_APIRET,"DefMDIClientProc: returns LRESULT 0\n"));
	    return 0;

	case WM_MDITILE:
	    TileChildWindows(hWnd,(WORD)wParam);
	    APISTR((LF_APIRET,"DefMDIClientProc: returns LRESULT 0\n"));
	    return 0;

	case WM_MDICASCADE:
	    CascadeChildWindows(hWnd,(WORD)wParam);
	    APISTR((LF_APIRET,"DefMDIClientProc: returns LRESULT 0\n"));
	    return 0;

	case WM_MDIGETACTIVE: {

	    if (!(lp = (LPMDICLIENT)GetWindowLong(hWnd,MWD_LPMDICLIENT))) {
	        APISTR((LF_APIRET,"DefMDIClientProc: returns LRESULT 0\n"));
		return 0;
	    }
	    if (lp->hActiveChild == 0) {
	        APISTR((LF_APIRET,"DefMDIClientProc: returns LRESULT 0\n"));
		return 0;
	    }
#ifndef TWIN32
	    rc = MAKELONG(lp->hActiveChild,
			    (TestWF(GetParent(hWnd),WFCHILDMAXIMIZED))?1:0);
#else	/* TWIN32 */
	    rc = (lp->hActiveChild);
#endif	/* TWIN32 */
	    APISTR((LF_APIRET,"DefMDIClientProc: returns LRESULT %x\n",rc));
	    return rc;
	}

	case WM_MDISETMENU:
	    if (!(lp = (LPMDICLIENT)GetWindowLong(hWnd,MWD_LPMDICLIENT))) {
	        APISTR((LF_APIFAIL,"DefMDIClientProc: returns LRESULT 0\n"));
		return 0;
	    }
	    if ((hMenuWnd = GET_WM_MDISETMENU_HMENUW(wParam,lParam)))
		/* "Window" menu is changing */ 
		ChangeWindowMenu(hWnd,CWM_DELETE); /* do it before resetting
							main menu */
	    if ((hMenuFrame = GET_WM_MDISETMENU_HMENUF(wParam,lParam))) {
		LOCKGDI(hMenuFrame);
		if (!SetMenu(GetParent(hWnd),hMenuFrame)) {
		    UNLOCKGDI(hMenuFrame);
	            APISTR((LF_APIFAIL,"DefMDIClientProc: returns LRESULT 0\n"));
		    return 0;
		}
                if (!lp->hMenu) /* Init hMenu, since it's not necessarily done in WM_NCCREATE */
                    {
                    lp->hMenu = GetMenu(GetParent(hWnd));
                    if (lp->hMenu)
                       LOCKGDI(lp->hMenu);
                    }
		if (lp->hMenu)
		   UNLOCKGDI(lp->hMenu);
		if ((TestWF(GetParent(hWnd),WFCHILDMAXIMIZED))) {
		    HMENU hSysMenu = GetSystemMenu(lp->hActiveChild,FALSE);

		    RemoveMenu(lp->hMenu,hSysMenu,MF_BYCOMMAND);
		    RemoveMenu(lp->hMenu,SC_RESTORE,MF_BYCOMMAND);
		    ModifyMenuBar(GetParent(hWnd),lp->hActiveChild,SW_MAXIMIZE);
		}
		lp->hMenu = hMenuFrame;
	    }
	    if (hMenuWnd) {
		/* "Window" menu is changing */ 
		if (lp->hWindowMenu)
		    UNLOCKGDI(lp->hWindowMenu);
		LOCKGDI(hMenuWnd);
		lp->hWindowMenu = hMenuWnd;
		lp->nSeparatorPos = GetMenuItemCount(hMenuWnd);
		ChangeWindowMenu(hWnd,CWM_UPDATE);
	    }
	    APISTR((LF_APIRET,"DefMDIClientProc: returns LRESULT %x\n",
		lp->hMenu));
	    return lp->hMenu;

	case WM_VSCROLL:
	case WM_HSCROLL:
/* this assumes that GET_WM_HSCROLL_ and GET_WM_VSCROLL_ are the same */
	    ScrollChildren(hWnd,uMsg,GET_WM_HSCROLL_CODE(wParam,lParam),
				GET_WM_HSCROLL_POS(wParam,lParam));
	    break;

	case WM_CONVERT: {

	    if (!lpMDIClientBinToNat) {
		hMDIClientClass32 = FindClass("MDICLIENT",0);
		lpMDIClientBinToNat = (WNDPROC)GetClassHandleLong(
				hMDIClientClass32,GCL_BINTONAT);
	    }
            rc = 0;
	    if (lpMDIClientBinToNat)
		rc = lpMDIClientBinToNat(hWnd, uMsg, wParam, lParam);
	    APISTR((LF_APIRET,"DefMDIClientProc: returns LRESULT %x\n",rc));
	    return rc;	
	}

	case WM_SETFOCUS:
	    lp = (LPMDICLIENT)GetWindowLong(hWnd,MWD_LPMDICLIENT);
	    if (lp == 0) {
	        APISTR((LF_APIFAIL,"DefMDIClientProc: returns LRESULT 0\n"));
		return (LRESULT)0;
	    }
	    if (lp->hActiveChild == 0) {
	        APISTR((LF_APIFAIL,"DefMDIClientProc: returns LRESULT 0\n"));
		return (LRESULT)0;
	    }
	    SetFocus(lp->hActiveChild);
	    break;

	default:
	    break;
    }

    rc = DefWindowProc(hWnd,uMsg,wParam,lParam);
    APISTR((LF_APIRET,"DefMDIClientProc: returns LRESULT %x\n",rc));
    return rc;
}

char lpszTemp[0x100];

static BOOL
ChangeWindowMenu(HWND hWnd, UINT mode)
{
    int i, nCount;
    char lpszMenuString[0x100];
    LPMDICLIENT lp;
    HWND hWndChild;

    if (!(lp = (LPMDICLIENT)GetWindowLong(hWnd,MWD_LPMDICLIENT)))
	return FALSE;
    nCount = GetMenuItemCount(lp->hWindowMenu);

    switch (mode) {

    case CWM_DELETE:
	for (i = nCount; i > lp->nSeparatorPos; i--)
	    DeleteMenu(lp->hWindowMenu,i-1,MF_BYPOSITION);
	return TRUE;

    case CWM_UPDATE:
	for (i = nCount; i > lp->nSeparatorPos; i--)
	    DeleteMenu(lp->hWindowMenu,i-1,MF_BYPOSITION);
	if (lp->nNumChildren) 
	    AppendMenu(lp->hWindowMenu,MF_SEPARATOR,0,(LPCSTR)NULL);
	for (i=0; i<lp->nNumChildren;i++) {
	    hWndChild = GetDlgItem(hWnd,lp->idFirstChild+i);
	    GetWindowText(hWndChild, lpszTemp, 0x100);
	    sprintf(lpszMenuString,"&%d %s\n",i+1,lpszTemp);
		AppendMenu(lp->hWindowMenu,
	    	    ((hWndChild == lp->hActiveChild)?MF_CHECKED:MF_UNCHECKED)|
		    MF_STRING|MF_ENABLED,
		    lp->idFirstChild+i, lpszMenuString);
	}
	return TRUE;

    case CWM_APPEND:
	if (nCount == lp->nSeparatorPos)
	    AppendMenu(lp->hWindowMenu,MF_SEPARATOR,0,(LPCSTR)NULL);
	if (nCount == lp->nSeparatorPos + lp->nNumChildren + 1)
	    return TRUE;
	hWndChild = GetDlgItem(hWnd,lp->idFirstChild+lp->nNumChildren-1);
	GetWindowText(hWndChild, lpszTemp, 80);
	sprintf(lpszMenuString,"&%d %s\n",lp->nNumChildren,lpszTemp);
	AppendMenu(lp->hWindowMenu,
	    ((hWndChild == lp->hActiveChild)?MF_CHECKED:MF_UNCHECKED)|
	    MF_STRING|MF_ENABLED,
	    lp->idFirstChild+lp->nNumChildren-1, lpszMenuString);
	return TRUE;

    default:
	return FALSE;
    }
}

void
TileChildWindows(HWND hWnd, WORD wAction)
{
    LPMDICLIENT lp;
    HWND hWndMDIChild;
    RECT rcRectClient;
    HDWP hDWP;
    int cxClient,cyClient,cxChild,cyChild;
    int nXIndex,nYIndex,nRemainder=1;
    int nNumChildren=0,i,j;
    int nCount=0,x,y;
    

    if (!(lp = (LPMDICLIENT)GetWindowLong(hWnd,MWD_LPMDICLIENT)))
	return;
    if (!lp->nNumChildren)
	return;
    ShowScrollBar(hWnd,SB_BOTH,FALSE);
    GetClientRect(hWnd,&rcRectClient);
    cxClient = rcRectClient.right - rcRectClient.left;
    cyClient = rcRectClient.bottom - rcRectClient.top;
    cyClient -= ArrangeIconicWindows(hWnd);

    hWndMDIChild = GetWindow(hWnd, GW_CHILD);
    do {
	if (!IsIconic(hWndMDIChild) && 
			GetWindow(hWndMDIChild, GW_OWNER) == 0) 
	    nNumChildren++;
#ifdef LATER
	if (wAction & MDITILE_SKIPDISABLED) {
	}
#endif
    } while ((hWndMDIChild = GetWindow(hWndMDIChild, GW_HWNDNEXT)) != 0);


    if (nNumChildren == 0)
	return;

    nYIndex = nXIndex = (int)sqrt((double)nNumChildren);
    if (nYIndex == 1)
	if (wAction & MDITILE_HORIZONTAL)
	    nYIndex = nNumChildren;
	else
	    nXIndex = nNumChildren;
    else
	for (;;) {
	    nRemainder = nNumChildren - nYIndex*(nXIndex-1);
	    if (wAction & MDITILE_HORIZONTAL) 
		if ((nRemainder-nXIndex) >= nYIndex) 
	            nYIndex++;
	        else
	            break;
	    else
		if ((nRemainder-nYIndex) >= nXIndex) 
	            nXIndex++;
	        else
	            break;
	}

    cxChild = cxClient/nXIndex;
    cyChild = cyClient/nYIndex;
    hDWP = BeginDeferWindowPos(nNumChildren);

    for (i=0,j=0; i<lp->nNumChildren; i++) {
	hWndMDIChild = GetDlgItem(hWnd,lp->idFirstChild+i);
	if (IsIconic(hWndMDIChild))
	    continue;
	if (hWndMDIChild != lp->hActiveChild)  {
	    if (wAction & MDITILE_HORIZONTAL) {
		x = (nCount-j*nXIndex)*cxChild;
		y = j*cyChild;
		if ((nCount%nXIndex == nXIndex -1) && (j < nYIndex - 1))
		    j++;
	    }
	    else {
		x = j*cxChild;
		y = (nCount-j*nYIndex)*cyChild;
		if ((nCount%nYIndex == nYIndex - 1) && (j < nXIndex -1))
		    j++;
	    }
	    DeferWindowPos(hDWP,hWndMDIChild,(HWND)0,
		    x,y,cxChild,cyChild,
		    SWP_NOACTIVATE|SWP_NOZORDER);
	    if (wAction & MDITILE_HORIZONTAL) {
		if (j == nYIndex - 1)
		    cxChild = cxClient/nRemainder;
	    }
	    else {
		if (j == nXIndex -1)  
		    cyChild = cyClient/nRemainder;
	    }
	    nCount++;
	}
    }
    if (!IsIconic(lp->hActiveChild)) {
	if (wAction & MDITILE_HORIZONTAL) {
	    x = (nRemainder-1)*cxChild;
	    y = (nYIndex-1)*cyChild; 
	}
	else {
	    x = (nXIndex-1)*cxChild;
	    y = (nRemainder-1)*cyChild;
	}
	DeferWindowPos(hDWP,lp->hActiveChild,(HWND)0,
		x,y,cxChild,cyChild,
		SWP_NOACTIVATE|SWP_NOZORDER);
    }
    EndDeferWindowPos(hDWP);
}

void
CascadeChildWindows(HWND hWnd, WORD wAction)
{
    LPMDICLIENT lp;
    HWND hWndMDIChild;
    RECT rcRectClient;
    HDWP hDWP;
    int cxClient,cyClient,cxChild,cyChild;
    int nIndex=0,nUnit;
    int nNumChildren = 0,i;

    if (!(lp = (LPMDICLIENT)GetWindowLong(hWnd,MWD_LPMDICLIENT)))
	return;
    if (!lp->nNumChildren)
	return;
    ShowScrollBar(hWnd,SB_BOTH,FALSE);
    nUnit = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME) - 1;
    GetClientRect(hWnd,&rcRectClient);
    cxClient = rcRectClient.right - rcRectClient.left;
    cyClient = rcRectClient.bottom - rcRectClient.top;
    cyClient -= ArrangeIconicWindows(hWnd);
    hWndMDIChild = GetWindow(hWnd, GW_CHILD);
    do {
	if (!IsIconic(hWndMDIChild) && 
			GetWindow(hWndMDIChild, GW_OWNER) == 0) 
	    nNumChildren++;
#ifdef LATER
	if (wAction & MDITILE_SKIPDISABLED) {
	}
#endif
    } while ((hWndMDIChild = GetWindow(hWndMDIChild, GW_HWNDNEXT)) != 0);

    if (nNumChildren == 0) 
	return;

    cxChild = cxClient - nUnit*3;
    cyChild = cyClient - nUnit*3;
    hDWP = BeginDeferWindowPos(nNumChildren);
    for (i=0; i<lp->nNumChildren; i++) {
	hWndMDIChild = GetDlgItem(hWnd,lp->idFirstChild+i);
	if (IsIconic(hWndMDIChild))
	    continue;
	if (hWndMDIChild != lp->hActiveChild) {
	    DeferWindowPos(hDWP,hWndMDIChild,HWND_TOP,
			nIndex*nUnit, nIndex*nUnit,cxChild,cyChild,
			SWP_NOACTIVATE);
	    nIndex = (nIndex+1)%4;
	}
    }
    if (!IsIconic(lp->hActiveChild))
	DeferWindowPos(hDWP,lp->hActiveChild,HWND_TOP,
		nIndex*nUnit, nIndex*nUnit,cxChild,cyChild,
		SWP_NOACTIVATE);
    EndDeferWindowPos(hDWP);
}

static void
CalcMDIScrollPos(HWND hWnd)
{
    LPMDICLIENT lp;
    RECT rcRectClient;
    RECT rc;
    int cxClient, cyClient;
    BOOL bShowHScroll,bShowVScroll;
    static int nLineHeight, nLineWidth;
    int nLeftDiff, nRightDiff, nDiff;
    int nScrollRange, nScrollPos;
    static BOOL ynInUse = 0;
    DWORD dwStyle;
    HWND  hWndChild;

    if (ynInUse) return; /* prevent recursive calls (WM_WINPOSCHANGED/WM_SIZE) */

    if (!(lp = (LPMDICLIENT)GetWindowLong(hWnd,MWD_LPMDICLIENT)))
	return;
    if (lp->bScrollInProgress)
        return;
    if (lp->bNoScrollBars)
        return;

    ynInUse = TRUE; /* set our "in progress" indicator, so that recursive calls get bounced */
    if (TestWF(GetTopLevelAncestor(hWnd),WFCHILDMAXIMIZED)) {
	ShowScrollBar(hWnd,SB_BOTH,FALSE);
	SetRectEmpty(&lp->rcLogicalRect);
        ynInUse = FALSE;
	return;
    }
    GetClientRect(hWnd,&rcRectClient);
    cxClient = rcRectClient.right - rcRectClient.left;
    dwStyle = GetWindowStyle(hWnd);
    if (dwStyle & WS_VSCROLL)
	cxClient += GetSystemMetrics(SM_CXVSCROLL);
    cyClient = rcRectClient.bottom - rcRectClient.top;
    if (dwStyle & WS_HSCROLL)
	cyClient += GetSystemMetrics(SM_CYHSCROLL);
    bShowHScroll = bShowVScroll = FALSE;
    SetRect(&rc,0,0,0,0);
    for (hWndChild = GetWindow(hWnd,GW_CHILD);
	    hWndChild;
		hWndChild = GetWindow(hWndChild,GW_HWNDNEXT)) 
	RecalcMDIClientRange(hWndChild,&rc);
    if (rc.right > cxClient)
	rc.right += 5;
    if (rc.bottom > cyClient)
	rc.bottom += 5;
    if (rc.left < 0)
	rc.left -= 5;
    if (rc.top < 0)
	rc.top -= 5;

    CopyRect(&lp->rcLogicalRect,&rc);

#ifdef SEVERE
printf("rc.left '%d' rc.right '%d' rc.top '%d' rc.bottom '%d'\ncxClient '%d' cyClient '%d' CXVSCROLL '%d' CYHSCROLL '%d'\n\n",
rc.left,rc.right,rc.top,rc.bottom,cxClient,cyClient,GetSystemMetrics(SM_CXVSCROLL),GetSystemMetrics(SM_CYHSCROLL));
#endif

    /*
    **   Check whether bShowHScroll causes bShowVScroll and vise versa.
    */
    if ((rc.left < 0) || (rc.right > cxClient)) 
        {
	bShowHScroll = TRUE;
        if (rc.top < 0 || rc.bottom > cyClient - GetSystemMetrics(SM_CYHSCROLL))
           bShowVScroll = TRUE;
        }
    if ((rc.top < 0 || rc.bottom > cyClient) && !bShowVScroll) 
        {
	bShowVScroll = TRUE;
        if ((rc.left < 0 || rc.right > cxClient - GetSystemMetrics(SM_CXVSCROLL)) && !bShowHScroll)
           bShowHScroll = TRUE;
        }

    if (bShowHScroll) 
       {
	if (nLineWidth == 0)
	   nLineWidth = (GetSystemMetrics(SM_CXICON) +
				GetSystemMetrics(SM_CXICONSPACING))/2;
	if (bShowVScroll)
	  cxClient -= GetSystemMetrics(SM_CXVSCROLL);
	nLeftDiff = (rc.left < 0) ? -rc.left : 0;
	nRightDiff = (rc.right > cxClient) ? (rc.right - cxClient) : 0;
	nDiff = nLeftDiff + nRightDiff;
	nScrollRange = nDiff / nLineWidth; 
	if (nDiff % nLineWidth)
	   nScrollRange++;
	SetScrollRange(hWnd,SB_HORZ,0,nScrollRange,FALSE);
	nScrollPos = nLeftDiff / nLineWidth;
	if (nLeftDiff % nLineWidth)
	   nScrollPos++;
	SetScrollPos(hWnd,SB_HORZ,nScrollPos,TRUE);
       }

    if (bShowVScroll) 
       {
	if (nLineHeight == 0)
	    nLineHeight = (GetSystemMetrics(SM_CYICON) +
				GetSystemMetrics(SM_CYICONSPACING))/2;
	if (bShowHScroll)
	    cyClient -= GetSystemMetrics(SM_CYHSCROLL);
	nLeftDiff = (rc.top < 0) ? -rc.top : 0;
	nRightDiff = (rc.bottom > cyClient) ? (rc.bottom - cyClient) : 0;
	nDiff = nLeftDiff + nRightDiff;
	nScrollRange = nDiff / nLineHeight; 
	if (nDiff % nLineHeight)
	    nScrollRange++;
	SetScrollRange(hWnd,SB_VERT,0,nScrollRange,FALSE);
	nScrollPos = nLeftDiff / nLineHeight;
	if (nLeftDiff % nLineHeight)
	    nScrollPos++;
	SetScrollPos(hWnd,SB_VERT,nScrollPos,TRUE);
       }
    
    if (bShowHScroll && bShowVScroll) 
       { 
	ShowScrollBar(hWnd,SB_BOTH,TRUE);
       }
    else if (!bShowHScroll && !bShowVScroll) 
            {
	    ShowScrollBar(hWnd,SB_BOTH,FALSE);
            }
         else {	/* showing or hiding only one SB */
	        /* First we hide SB if visible and not needed */
	      if (!bShowHScroll && (GetWindowStyle(hWnd) & WS_HSCROLL))
                 {
	         ShowScrollBar(hWnd,SB_HORZ,FALSE); 
	         }
	      if (!bShowVScroll && (GetWindowStyle(hWnd) & WS_VSCROLL))
                 {
	         ShowScrollBar(hWnd,SB_VERT,FALSE); 
	         }
	      /* Now show SB if not visible and we need 'em */
	      if (bShowHScroll && !(GetWindowStyle(hWnd) & WS_HSCROLL))
                 {
	         ShowScrollBar(hWnd,SB_HORZ,TRUE);
	         }
	      if (bShowVScroll && !(GetWindowStyle(hWnd) & WS_VSCROLL))
                 {
	         ShowScrollBar(hWnd,SB_VERT,TRUE);
	         }
              }
   ynInUse = FALSE;
}

static void
RecalcMDIClientRange(HWND hWndMDIChild, LPRECT lprcResult) 
{
    RECT rcRectChild;
    POINT pt;
    int nRight, nBottom;

    GetWindowRect(hWndMDIChild,&rcRectChild);
    pt.x = rcRectChild.left;
    pt.y = rcRectChild.top;
    ScreenToClient(GetParent(hWndMDIChild),&pt);
    if (pt.x < lprcResult->left)
	lprcResult->left = pt.x;
    if (pt.y < lprcResult->top)
	lprcResult->top = pt.y;
    nRight = pt.x + (rcRectChild.right-rcRectChild.left);
    if (nRight > lprcResult->right) 
	lprcResult->right = nRight;
    nBottom = pt.y + (rcRectChild.bottom-rcRectChild.top);
    if ( nBottom > lprcResult->bottom) 
	lprcResult->bottom = nBottom;
}

static void
ScrollChildren(HWND hWnd, UINT uMsg, UINT uiScrollCode, int nPos)
{
    LPMDICLIENT lp;
    static int nLineHeight, nLineWidth;
    RECT rc;
    POINT pt;
    int nOffset,nDiff,nLeftDiff,nRightDiff;
    int nLines;

    if (!(lp = (LPMDICLIENT)GetWindowLong(hWnd,MWD_LPMDICLIENT)))
	return;

    if (nLineWidth == 0) {
	nLineWidth = (GetSystemMetrics(SM_CXICON) +
				GetSystemMetrics(SM_CXICONSPACING))/2;
	nLineHeight = (GetSystemMetrics(SM_CYICON) +
				GetSystemMetrics(SM_CYICONSPACING))/2;
    }
    GetClientRect(hWnd,&rc);

    switch (uiScrollCode) {
	case SB_LINEDOWN:
	    if (uMsg == WM_HSCROLL) 
		nOffset = ((nDiff = lp->rcLogicalRect.right-rc.right)<0)?0:
			(nDiff < nLineWidth) ? -nDiff : -nLineWidth;
	    else   	/* WM_VSCROLL */
		nOffset = ((nDiff = lp->rcLogicalRect.bottom-rc.bottom)<0)?0:
			(nDiff < nLineHeight) ? -nDiff : -nLineHeight;
	    break;
	case SB_LINEUP:
	    if (uMsg == WM_HSCROLL)  
		nOffset = ((nDiff = rc.left-lp->rcLogicalRect.left)<0)?0:
			(nDiff < nLineWidth) ? nDiff : nLineWidth;
	    else  	/* WM_VSCROLL */
		nOffset = ((nDiff = rc.top - lp->rcLogicalRect.top)<0)?0:
			(nDiff < nLineHeight) ? nDiff : nLineHeight;
	    break;
	case SB_PAGEDOWN:
	    if (uMsg == WM_HSCROLL) 
                {
	        nLines = (rc.right - rc.left)/(2*nLineWidth);
		nOffset = ((nDiff = lp->rcLogicalRect.right-rc.right)<0)?0:
			(nDiff < nLines*nLineWidth) ? -nDiff :
						      -(nLines*nLineWidth);
                }
	    else {  	/* WM_VSCROLL */
	         nLines = (rc.bottom - rc.top)/(2*nLineHeight);
		 nOffset = ((nDiff = lp->rcLogicalRect.bottom-rc.bottom)<0)?0:
			(nDiff < nLines*nLineHeight) ? -nDiff :
						       -(nLines*nLineHeight);
                 }
	    break;
	case SB_PAGEUP:
	    if (uMsg == WM_HSCROLL)  
                {
	        nLines = (rc.right - rc.left)/(2*nLineWidth);
		nOffset = ((nDiff = rc.left-lp->rcLogicalRect.left)<0)?0:
			(nDiff < nLines*nLineWidth) ? nDiff :
						      nLines*nLineWidth;
                }
	    else {  	/* WM_VSCROLL */
	         nLines = (rc.bottom - rc.top)/(2*nLineHeight);
		 nOffset = ((nDiff = rc.top - lp->rcLogicalRect.top)<0)?0:
			(nDiff < nLines*nLineHeight) ? nDiff :
						       nLines*nLineHeight;
                 }
	    break;
	case SB_THUMBPOSITION:
	    if (uMsg == WM_HSCROLL) {
		nLeftDiff = rc.left - lp->rcLogicalRect.left; 
		if (nLeftDiff < 0)
		    nLeftDiff = 0;
		nRightDiff = lp->rcLogicalRect.right - rc.right;
		if (nRightDiff < 0)
		    nRightDiff = 0;
		nDiff = min(nPos*nLineWidth,nLeftDiff+nRightDiff);
	    }
	    else { 	/* WM_VSCROLL */
		nLeftDiff = rc.top - lp->rcLogicalRect.top;
		if (nLeftDiff < 0)
		    nLeftDiff = 0;
		nRightDiff = lp->rcLogicalRect.bottom - rc.bottom;
		if (nRightDiff < 0)
		    nRightDiff = 0;
		nDiff = min(nPos*nLineHeight,nLeftDiff+nRightDiff);
	    }
	    if (nLeftDiff > nDiff)
		nOffset = nLeftDiff - nDiff;
	    else
		nOffset = -(nDiff - nLeftDiff); 
	    break;
	case SB_BOTTOM:
	case SB_TOP:
	default:
	    return;
    }
    if (nOffset == 0)
	return;
    pt.x = pt.y = 0;

    if (uMsg == WM_HSCROLL)
	pt.x += nOffset;
    else
	pt.y += nOffset;

    lp->bScrollInProgress = TRUE;
    ScrollWindow(hWnd,pt.x,pt.y,(LPRECT)NULL,(LPRECT)NULL);
    lp->bScrollInProgress = FALSE;

    CalcMDIScrollPos(hWnd);
}

