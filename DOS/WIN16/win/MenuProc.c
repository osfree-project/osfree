/*    
	MenuProc.c	2.39
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
#include "Menus.h"
#include "Listbox.h"
#include "Driver.h"
#include "WinDefs.h"
#include <string.h>
#include <ctype.h>

/* internal stuff */
static WORD MenuHitTest(HMENU32,POINT *);
static BOOL CreatePopup (HWND,LPTRACKPOPUPSTRUCT,HMENU,WORD,LPRECT,BOOL);
static void DrawPullrightGlyph(HDC,LPRECT,WORD,COLORREF);
static void DrawCheckMark(LPDRAWITEMSTRUCT,DWORD,WORD,HBITMAP,
			COLORREF,COLORREF);
static int PopupHitTest(HWND *, int, POINT);
static void DrawPopupMenuItem(HMENU32,HWND,LPDRAWITEMSTRUCT);
static void ChangeMBSelection(HWND,HMENU32,LPTRACKPOPUPSTRUCT,WORD,WORD);
static void SetPopupMenuSel(HWND,HWND,WORD,WORD);
static LRESULT InternalMenuProc(HWND,UINT,WPARAM,LPARAM);

/* external procedures */
extern void MenuDrawItem(HDC,HMENU32,HWND,WORD,WORD);
extern DWORD CalcPopupMenuDimensions(HMENU,HWND);
extern HMENU32 GetMenuHandle32(HMENU);
extern LONG LBoxAPI(HMENU32, UINT, LPARAM);
extern HPEN 	GetSysColorPen(int);
extern WORD MeasureWindowMenu(HWND,HWND);
extern void SetWindowMenuHeight(HWND, WORD);
extern BOOL IsMouseOrKeyboardMsg(UINT);
HMENU32 TWIN_FindMenuItem(HMENU32,UINT);

static void
DrawPullrightGlyph(HDC hDC, LPRECT lprc, WORD wRightIndent,
		COLORREF crColor)
{
    HPEN hPen;
    HBRUSH hBrush;
    POINT points[3];

    hPen = CreatePen(PS_SOLID,1,crColor);
    hBrush = CreateSolidBrush(crColor);
    SelectObject(hDC,hPen);
    SelectObject(hDC,hBrush);

    points[0].x = points[2].x = lprc->right + wRightIndent/3;
    points[0].y = lprc->top + (lprc->bottom - lprc->top)/3;
    points[2].y = lprc->bottom - (lprc->bottom - lprc->top)/3;
    points[1].x = points[0].x + wRightIndent/3;
    points[1].y = lprc->top + (lprc->bottom - lprc->top)/2;
    Polygon(hDC,points,3);

    DeleteObject(hPen);
    DeleteObject(hBrush);
}

static int
PopupHitTest(HWND *hPopups, int nPopups, POINT pt)
{
    int n;
    RECT rcWnd;

    for (n = nPopups-1; n>=0; n--) {
	if (hPopups[n] == 0)
		continue;
	GetWindowRect(hPopups[n],&rcWnd);
	if (PtInRect(&rcWnd,pt))
	    return n;
    }
    return -1;
}

LRESULT
MenuBarProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    LPTRACKPOPUPSTRUCT lptps;
    LPCREATESTRUCT lpCrS;
    POINT pt,ptScreen;
    int nXOrigin,nYOrigin;
    WORD wSel = 0,wPrevSel=0;
    WORD wItemID;
    WORD wHeight;
    RECT rcRect,rcClient;
    HWND hWndParent;
    HMENU hMenu = 0,hPopupMenu;
    HMENU32 hMenu32 = 0, hPopupMenu32 = 0, hPopupMenu32a = 0;
    MENUITEMSTRUCT mnis;
    LPDRAWITEMSTRUCT lpdis;
    LPMEASUREITEMSTRUCT lpmis;
    LONG lFlags = 0;
    LPSTR lpItemData,lpTemp;
    char c;
    DWORD dwMenuDim;
    int nPopupHit;
    HWND hPopup = 0, hWndTmp;
    DWORD dwIndents;
    int n=0,vKey,nCount;

    switch(uiMsg) {
	case WM_NCCREATE:
	    lptps = (LPTRACKPOPUPSTRUCT)WinMalloc(sizeof(TRACKPOPUPSTRUCT));
	    if (!lptps)
		return FALSE;
	    memset((LPSTR)lptps,'\0',sizeof(TRACKPOPUPSTRUCT));
	    lpCrS = (LPCREATESTRUCT)lParam;
	    if (lpCrS->lpCreateParams) {
		*lptps = *((LPTRACKPOPUPSTRUCT)lpCrS->lpCreateParams);
		lptps->uiFlags |= TP_TRACKPOPUP;
		dwMenuDim =
			CalcPopupMenuDimensions(lptps->hMenu,lptps->hWndOwner);
		if (GetWindowStyle(lptps->hWndOwner) & WS_MINIMIZE) {
		    GetWindowRect(lptps->hWndOwner, &rcRect);
		    hWndParent = GetParent(lptps->hWndOwner);
		    GetClientRect(hWndParent,&rcClient);
		    pt.x = pt.y = 0;
		    ClientToScreen(hWndParent,&pt);
		    nXOrigin = rcRect.left + 2;
		    if ((nXOrigin + (int)LOWORD(dwMenuDim)) > 
						(pt.x + rcClient.right)) 
			nXOrigin = pt.x + rcClient.right - LOWORD(dwMenuDim);
		    nYOrigin = rcRect.bottom + 2; 
					/* LATER -- adjust for icon title */
		    if ((nYOrigin + (int)HIWORD(dwMenuDim)) > 
						(pt.y + rcClient.bottom))
			nYOrigin = rcRect.top - 2 - HIWORD(dwMenuDim);
		}
		else {
		    nYOrigin = lptps->y;
		    if (lptps->uiFlags & TPM_CENTERALIGN)
			nXOrigin = lptps->x - (int)LOWORD(dwMenuDim)/2;
		    else if (lptps->uiFlags & TPM_RIGHTALIGN)
			nXOrigin = lptps->x - LOWORD(dwMenuDim);
		    else
			nXOrigin = lptps->x;
		}
		SetRect(&rcRect,nXOrigin,nYOrigin,
			LOWORD(dwMenuDim),HIWORD(dwMenuDim));
		if (!CreatePopup(hWnd,lptps,lptps->hMenu,0,&rcRect,FALSE)) {
		    WinFree((LPSTR)lptps);
		    return FALSE;
		}
		SetPopupMenuSel(lptps->hWndOwner,lptps->hPopups[0],0,0);
		SetWindowWord(hWnd, LWD_HMENU, 0);
	    }
	    else {
		lptps->uiFlags |= TP_MENUBAR;
		if (!lpCrS->hMenu)
		    return FALSE;
		lptps->hMenu = lpCrS->hMenu;
		lptps->hWndOwner = GetWindow(hWnd,GW_CHILD);
		lptps->bSystemMenu = FALSE;
		lptps->x = lpCrS->x;
		lptps->y = lpCrS->y;
		SetWindowWord(hWnd, LWD_HMENU, (WORD)lptps->hMenu);
	    }
	    SetWindowLong(hWnd, LWD_LPMENUDATA, (LONG)lptps);
	    return 1L;

	case WM_CREATE:
	    if (!(lptps = (LPTRACKPOPUPSTRUCT)
			GetWindowLong(hWnd,LWD_LPMENUDATA)))
		return FALSE;
	    if (lptps->uiFlags & TP_MENUBAR) /* for trackpopup only */
		return 0L;
	    SetWindowPos(hWnd,HWND_BOTTOM,0,0,0,0,
		SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
	    SetCapture(hWnd);
	    ShowWindow(lptps->hPopups[0],SW_SHOWNA);
	    UpdateWindow(lptps->hPopups[0]);
	    lptps->uiFlags |= LSF_CAPTUREACTIVE;
	    return 1L;

	case WM_PAINT:
	    if ((lptps = (LPTRACKPOPUPSTRUCT)GetWindowLong(hWnd,LWD_LPMENUDATA))
			&& (lptps->uiFlags & TP_MENUBAR))
		DrawMenuBar(lptps->hWndOwner);
	    ValidateRect(hWnd, NULL);
	    return 1L;

	case WM_SIZE:
	    if (!(lptps = (LPTRACKPOPUPSTRUCT)GetWindowLong
				(hWnd,LWD_LPMENUDATA)))
		return FALSE;
	    if (lptps->uiFlags & TP_MENUBAR) {
		wHeight = MeasureWindowMenu(lptps->hWndOwner,hWnd);
		if (!wHeight)
		    return FALSE;
		SetWindowMenuHeight(lptps->hWndOwner,wHeight);
		return FALSE;
	    }
	    break;

	case WM_MOUSEMOVE:
	    if (!(lptps = (LPTRACKPOPUPSTRUCT)GetWindowLong
				(hWnd,LWD_LPMENUDATA)))
		return FALSE;
	    if (!(lptps->uiFlags & LSF_BUTTONDOWN))
		return FALSE;
	    if (!(lptps->uiFlags & LSF_CAPTUREACTIVE))
		return FALSE;
	    vKey = (lptps->uiFlags & TPM_RIGHTBUTTON)?
                                VK_RBUTTON:VK_LBUTTON;
	    if (!(GetKeyState(vKey) & 0x8000))
		break;
	    if (!(hMenu = lptps->hMenu))
		return FALSE;
	    if (!(hMenu32 = GetMenuHandle32(hMenu)))
		return FALSE;
	    pt.x = (int)((signed short)LOWORD(lParam));
	    pt.y = (int)((signed short)HIWORD(lParam));
	    if (lptps->uiFlags & TP_MENUBAR) 
		GetClientRect(hWnd, &rcRect);
	    else
		SetRectEmpty(&rcRect);
	    if (PtInRect(&rcRect,pt)) { /* pt on MenuBar */
		wSel = MenuHitTest(hMenu32,&pt);
		mnis.wPosition = (WORD)-1;
		mnis.wAction = LCA_SELECTION;
		wPrevSel = (WORD)LBoxAPI(hMenu32,LBA_GETDATA,(LPARAM)&mnis);
		if ((wSel == wPrevSel) || (wSel == (WORD)-1))
		    return TRUE;
		ChangeMBSelection(hWnd,hMenu32,lptps,wPrevSel,wSel);
		mnis.wItemFlags = MF_BYPOSITION;
		mnis.wAction = LCA_GET | LCA_FLAGS;
		mnis.wPosition = wSel;
		lFlags = LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
		if (lptps->bSystemMenu)
		    lFlags |= MF_SYSMENU;
		mnis.wAction = LCA_GET|LCA_ITEMID;
		wItemID = (WORD)LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
		SendMessage(lptps->hWndOwner,WM_MENUSELECT,
			GET_WM_MENUSELECT_MPS(wItemID,LOWORD(lFlags),hMenu));
		if (!((lFlags & MF_POPUP) &&
			!(lFlags & (MF_GRAYED|MF_DISABLED|MF_SEPARATOR)))) {
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return TRUE;
		}
		mnis.lpItemData = (LPSTR)&rcRect;
		mnis.wAction = LCA_GET|LCA_RECT;
		LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
		pt.x = rcRect.left;
		pt.y = rcRect.bottom - 1;
		ClientToScreen(hWnd,&pt);
		SetRect(&rcRect,pt.x,pt.y,0,0);
		RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		return CreatePopup(hWnd,lptps,(HMENU)wItemID,wSel,&rcRect,TRUE);
	    }
	    else {
		ClientToScreen(hWnd,&pt);
		nPopupHit = PopupHitTest(lptps->hPopups,lptps->nPopups,pt);
		if (nPopupHit != -1) {
		    hPopup = lptps->hPopups[nPopupHit];
		    ScreenToClient(hPopup,&pt);
		    if (!(lptps->wPopupFlags[nPopupHit] & PSF_BUTTONDOWN)) {
			SendMessage(hPopup,WM_LBUTTONDOWN,
				wParam,MAKELONG(pt.x,pt.y));
			lptps->wPopupFlags[nPopupHit] |= PSF_BUTTONDOWN;
		    }
		    wPrevSel = (WORD)SendMessage(hPopup,LB_GETCURSEL,0,0L);
		    SendMessage(hPopup,uiMsg,wParam,MAKELONG(pt.x,pt.y));
		    wSel = (WORD)SendMessage(hPopup,LB_GETCURSEL,0,0L);
		    if (wPrevSel == wSel) {
		        RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			return TRUE;
		    }
		    hPopupMenu = (HMENU)GetWindowID(hPopup);
		    hPopupMenu32 = GetMenuHandle32(hPopupMenu);
		    mnis.wAction = LCA_GET|LCA_FLAGS;
		    mnis.wItemFlags = MF_BYPOSITION;
		    mnis.wPosition = wSel;
		    lFlags = LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
		    if (lptps->bSystemMenu)
			lFlags |= MF_SYSMENU;
		    mnis.wAction = LCA_GET|LCA_ITEMID;
		    wItemID = (WORD)LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
					(LPARAM)&mnis);
		    SendMessage(lptps->hWndOwner,WM_MENUSELECT,
			GET_WM_MENUSELECT_MPS(wItemID,LOWORD(lFlags),wParam));
		    if ((hWndTmp = lptps->hPopups[nPopupHit+1]) &&
			(wItemID == GetWindowID(hWndTmp))) {
			SendMessage(hWndTmp,LB_SETCURSEL,
					(WPARAM)-1,0L);
			RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			RELEASELBOXINFO((LPLISTBOXINFO)hPopupMenu32);
			return TRUE;
		    }
		    for (n = lptps->nPopups-1; n>nPopupHit; n--) 
			if (lptps->wPopupFlags[n-1] & PSF_POPUPACTIVE) {
			    DestroyWindow(lptps->hPopups[n]);
			    lptps->hPopups[n] = 0;
			    lptps->wPopupFlags[n-1] &= ~PSF_POPUPACTIVE;
			    lptps->nPopups--;
			}
		    if (!((lFlags & MF_POPUP) &&
			!(lFlags & (MF_GRAYED|MF_DISABLED|MF_SEPARATOR)))) {
		        RELEASELBOXINFO((LPLISTBOXINFO)hPopupMenu32);
			RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			return TRUE;
		    }
		    mnis.wAction = LCA_GET|LCA_RECT;
		    mnis.lpItemData = (LPSTR)&rcRect;
		    LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
		    mnis.wAction = LCA_GET|LCA_INDENTS;
		    dwIndents = LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
					(LPARAM)&mnis);
		    pt.x = rcRect.right - HIWORD(dwIndents);
		    pt.y = rcRect.top;
		    ClientToScreen(hPopup,&pt);
		    SetRect(&rcRect,pt.x,pt.y,0,0);
		    return CreatePopup(hWnd,lptps,(HMENU)wItemID,
					wSel,&rcRect,TRUE);
		}
		else {
		    hPopup = lptps->hPopups[lptps->nPopups-1];
		    if (lptps->wPopupFlags[lptps->nPopups-1] & PSF_BUTTONDOWN) 
			SendMessage(hPopup,LB_SETCURSEL,(WPARAM)-1,0L);
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return TRUE;
		}
	    }
	    break;

	case WM_LBUTTONDOWN:
	    if (!(lptps = (LPTRACKPOPUPSTRUCT)GetWindowLong
				(hWnd,LWD_LPMENUDATA)))
		return FALSE;
	    if (!(hMenu = lptps->hMenu))
		return FALSE;
	    if (!(hMenu32 = GetMenuHandle32(hMenu)))
		return FALSE;

	    pt.x = (int)((signed short)LOWORD(lParam));
	    pt.y = (int)((signed short)HIWORD(lParam));
	    ptScreen = pt;
	    ClientToScreen(hWnd,&ptScreen);

	    if (lptps->uiFlags & TP_MENUBAR)
		GetClientRect(hWnd, &rcRect);
	    else
		SetRectEmpty(&rcRect);

	    /* if there are active popups, popup hit test takes precedence */
	    if (lptps->nPopups) {
		nPopupHit = PopupHitTest(lptps->hPopups,
					lptps->nPopups,ptScreen);
	    } else {
		nPopupHit = -1;
	    }

	    if ((nPopupHit == -1) && PtInRect(&rcRect,pt)) {

		wSel = MenuHitTest(hMenu32,&pt);
		mnis.wPosition = (WORD)-1;
		mnis.wAction = LCA_SELECTION;
		wPrevSel = (WORD)LBoxAPI(hMenu32,LBA_GETDATA,(LPARAM)&mnis);

		if (wPrevSel == (WORD)-1) {
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);		  
		    if (wSel == (WORD)-1)
			return TRUE;
		    else if (!(lptps->uiFlags & LSF_CAPTUREACTIVE)) 
			return InternalMenuProc(hWnd,uiMsg,wParam,lParam);
		}
		mnis.wPosition = wSel;
		ChangeMBSelection(hWnd,hMenu32,lptps,wPrevSel,wSel);
		if (wSel == (WORD)-1) {
		    SetWindowWord(hWnd,TP_STATUS,1);
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return TRUE;
		}
		if (wSel != wPrevSel) {
		    mnis.wItemFlags = MF_BYPOSITION;
		    mnis.wAction = LCA_GET|LCA_FLAGS;
		    lFlags = LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
		    if (lptps->bSystemMenu)
			lFlags |= MF_SYSMENU;
		    mnis.wAction = LCA_GET|LCA_ITEMID;
		    wItemID = (UINT)LBoxAPI(hMenu32,LBA_MODIFYITEM,
					(LPARAM)&mnis);
		    SendMessage(lptps->hWndOwner,WM_MENUSELECT,
				GET_WM_MENUSELECT_MPS(wItemID,
					LOWORD(lFlags)|MF_MOUSESELECT,
					lptps->hMenu));
		    lptps->uiFlags |= LSF_BUTTONDOWN;


		    if ((lFlags & MF_POPUP) &&
			    !(lFlags & (MF_GRAYED|MF_DISABLED|MF_SEPARATOR))) {
			mnis.lpItemData = (LPSTR)&rcRect;
			mnis.wAction = LCA_GET|LCA_RECT;
			mnis.wItemFlags = MF_BYPOSITION;
			LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
			pt.x = rcRect.left;
			pt.y = rcRect.bottom - 1;
			ClientToScreen(hWnd,&pt);
			SetRect(&rcRect,pt.x,pt.y,0,0);
			return CreatePopup(hWnd,lptps,(HMENU)wItemID,
					wSel,&rcRect,TRUE);
		    }
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return TRUE;
		}
		else {	/*  if (wSel == wPrevSel) => toggle selection */
		    if (lptps->nPopups) {
			for (n=lptps->nPopups-1;n>=0;n--) {
			    DestroyWindow(lptps->hPopups[n]);
			    lptps->hPopups[n] = 0;
			    if (n > 1) {
				lptps->wPopupFlags[n-1] &= ~PSF_POPUPACTIVE;
				lptps->wPopupFlags[n-1] |= PSF_POPSELKILLED;
				lptps->nPopups--;
			    }
			    else 
				lptps->uiFlags &= ~PSF_POPUPACTIVE;
			}
		    }
		    ChangeMBSelection(hWnd,hMenu32,lptps,wPrevSel,-1);
		    SetWindowWord(hWnd,TP_STATUS,1);
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    lptps->uiFlags |= PSF_POPSELKILLED;
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return TRUE;
		}
	    }
	    else {	/* pt not on the menubar */

		lptps->uiFlags |= LSF_BUTTONDOWN;

		if (nPopupHit == -1) {

		    SendMessage(lptps->hWndOwner,WM_MENUSELECT,
				GET_WM_MENUSELECT_MPS(lptps->hMenu,-1,0));

		    if (lptps->uiFlags & TP_MENUBAR) {
			mnis.wPosition = (WORD)-1;
			mnis.wAction = LCA_SELECTION;
			wPrevSel = (WORD)LBoxAPI(hMenu32,LBA_GETDATA,
						(LPARAM)&mnis);
			ChangeMBSelection(hWnd,hMenu32,lptps,wPrevSel,
						(WORD)-1);
		    }
		    SetWindowWord(hWnd,TP_STATUS,1);
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return TRUE;
		}

		hPopup = lptps->hPopups[nPopupHit];
		ScreenToClient(hPopup,&ptScreen);
		if (lptps->nPopups > (nPopupHit + 1)) {
		    for (n=lptps->nPopups-1;n>nPopupHit;n--) {
			DestroyWindow(lptps->hPopups[n]);
			lptps->hPopups[n] = 0;
		    }
		    lptps->nPopups = nPopupHit + 1;
		    lptps->wPopupFlags[nPopupHit] &= ~PSF_POPUPACTIVE;
		}
		wPrevSel = (WORD)SendMessage(hPopup,LB_GETCURSEL,0,0L);

		SendMessage(hPopup,uiMsg,wParam,
				MAKELONG(ptScreen.x,ptScreen.y));

		if ((wSel = (WORD)SendMessage(hPopup,
			LB_GETCURSEL,0,0L)) != (WORD)-1) {

		    lptps->wPopupFlags[nPopupHit] |= PSF_BUTTONDOWN;
		    hPopupMenu = (HMENU)GetWindowID(hPopup);
		    hPopupMenu32 = GetMenuHandle32(hPopupMenu);
		    mnis.wAction = LCA_GET|LCA_FLAGS;
		    mnis.wItemFlags = MF_BYPOSITION;
		    mnis.wPosition = wSel;
		    lFlags = LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
					(LPARAM)&mnis);
		    if (lptps->bSystemMenu)
			lFlags |= MF_SYSMENU;
		    mnis.wAction = LCA_GET | LCA_ITEMID;
		    wItemID = (WORD)LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
					(LPARAM)&mnis);
		    if (wSel != wPrevSel)
			SendMessage(lptps->hWndOwner,WM_MENUSELECT,
				GET_WM_MENUSELECT_MPS(wItemID,
					LOWORD(lFlags)|MF_MOUSESELECT,
					lptps->hMenu));
		    if (!(lFlags & MF_POPUP) ||
			    (lFlags & (MF_GRAYED|MF_DISABLED|MF_SEPARATOR))) {
		        RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			return TRUE;
		    }

		    if ((lptps->hPopups[nPopupHit+1] != 0) && 
			(wItemID == (HMENU)GetWindowID
				(lptps->hPopups[nPopupHit+1]))) {

			SendMessage(lptps->hPopups[nPopupHit+1],
				LB_SETCURSEL,
				(WPARAM)-1,0L);

			lptps->wPopupFlags[nPopupHit] |= PSF_POPSELKILLED;
			RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			return TRUE;
		    }

		    mnis.wAction = LCA_GET|LCA_RECT;
		    mnis.lpItemData = (LPSTR)&rcRect;
		    LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
		    mnis.wAction = LCA_GET|LCA_INDENTS;
		    dwIndents = LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
				(LPARAM)&mnis);
		    pt.x = rcRect.right - HIWORD(dwIndents);
		    pt.y = rcRect.top;
		    ClientToScreen(hPopup,&pt);
		    SetRect(&rcRect,pt.x,pt.y,0,0);
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    RELEASELBOXINFO((LPLISTBOXINFO)hPopupMenu32);
		    return CreatePopup(hWnd,lptps,wItemID,wSel,&rcRect,TRUE);
		}
	    }
	    return TRUE;
	case WM_LBUTTONUP:
	    if (!(lptps = (LPTRACKPOPUPSTRUCT)GetWindowLong
				(hWnd,LWD_LPMENUDATA)))
		return FALSE;
	    if (!(lptps->uiFlags & LSF_BUTTONDOWN))
		return FALSE;
	    if (lptps->uiFlags & TP_MENUBAR) {
		if (!(hMenu = lptps->hMenu))
		    return FALSE;
		if (!(hMenu32 = GetMenuHandle32(hMenu)))
		    return FALSE;
		GetClientRect(hWnd, &rcRect);
	    }
	    else
		SetRectEmpty(&rcRect);
	    lptps->uiFlags &= ~LSF_BUTTONDOWN;
	    pt.x = (int)((signed short)LOWORD(lParam));
	    pt.y = (int)((signed short)HIWORD(lParam));
	    ptScreen = pt;
	    ClientToScreen(hWnd,&ptScreen);
	    if (lptps->nPopups)
		nPopupHit = PopupHitTest(lptps->hPopups,
					lptps->nPopups,ptScreen);
	    else
		nPopupHit = -1;
	    if ((nPopupHit == -1) && PtInRect(&rcRect,pt)) {
		wSel = MenuHitTest(hMenu32,&pt);
		mnis.wPosition = (WORD)-1;
		mnis.wAction = LCA_SELECTION;
		wPrevSel = (WORD)LBoxAPI(hMenu32,LBA_GETDATA,(LPARAM)&mnis);
		ChangeMBSelection(hWnd,hMenu32,lptps,wPrevSel,wSel);
		if (wSel == (WORD)-1) {
		    SetWindowWord(hWnd,TP_STATUS,1);
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return 0;
		}
		else {
		    if (lptps->uiFlags & PSF_POPSELKILLED) {
			ChangeMBSelection(hWnd,hMenu32,lptps,wPrevSel,(WORD)-1);
			SetWindowWord(hWnd,TP_STATUS,1);
			lptps->uiFlags &= ~PSF_POPSELKILLED;
			RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			return 0;
		    }
		    mnis.wPosition = wSel;
		    mnis.wItemFlags = MF_BYPOSITION;
		    mnis.wAction = LCA_GET | LCA_FLAGS;
		    lFlags = LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
		    if (lptps->bSystemMenu)
			lFlags |= MF_SYSMENU;
		    mnis.wAction = LCA_GET | LCA_ITEMID;
		    wItemID = (WORD)LBoxAPI(hMenu32,LBA_MODIFYITEM,
						(LPARAM)&mnis);
		    if (!(lFlags & MF_POPUP)) {
			ChangeMBSelection(hWnd,hMenu32,lptps,wPrevSel,(WORD)-1);
			if (lFlags & MF_SYSMENU) {
			    GetCursorPos(&pt);
			    PostMessage(lptps->hWndOwner,WM_SYSCOMMAND,
				(WPARAM)wItemID,MAKELONG(pt.x,pt.y));
			}
			else
			    PostMessage(lptps->hWndOwner,WM_COMMAND,
				GET_WM_COMMAND_MPS(wItemID,0,0));
			SetWindowWord(hWnd,TP_STATUS,1);
			RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			return 0;
		    }
		    if (!(lFlags & (MF_DISABLED|MF_GRAYED)))
			SetPopupMenuSel(lptps->hWndOwner,lptps->hPopups[0],0,
				(lFlags & MF_SYSMENU)|MF_MOUSESELECT);
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return 0;
		}
	    }
	    else {	/* pt not in MenuBar rectangle */
		hPopup = (nPopupHit == -1) ? 0 : lptps->hPopups[nPopupHit];
		if (lptps->uiFlags & TP_MENUBAR) {
		    mnis.wPosition = (WORD)-1;
		    mnis.wAction = LCA_SELECTION;
		    wPrevSel = (WORD)LBoxAPI(hMenu32,LBA_GETDATA,(LPARAM)&mnis);
		    if (!hPopup) {
			SetWindowWord(hWnd,TP_STATUS,1);
			ChangeMBSelection(hWnd,hMenu32,lptps,wPrevSel,(WORD)-1);
			RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			return 0;
		    }
		}
		else if (!hPopup) {
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return 0;
		}
		ScreenToClient(hPopup,&ptScreen);
		lptps->wPopupFlags[nPopupHit] &= ~PSF_BUTTONDOWN;
		if (lptps->wPopupFlags[nPopupHit] & PSF_POPUPACTIVE) {
		    SendMessage(hPopup,uiMsg,wParam,
			MAKELONG((WORD)-1,(WORD)-1));
		    if (lptps->wPopupFlags[nPopupHit] & PSF_POPSELKILLED) {
			DestroyWindow(lptps->hPopups[nPopupHit+1]);
			lptps->hPopups[n] = 0;
			lptps->wPopupFlags[nPopupHit] &=
				~(PSF_POPSELKILLED|PSF_POPUPACTIVE);
			lptps->nPopups--;
		    }
		    else
		        SetPopupMenuSel(lptps->hWndOwner,
					lptps->hPopups[nPopupHit+1],0,
					(lFlags & MF_SYSMENU)|MF_MOUSESELECT);
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return TRUE;
		}
		SendMessage(hPopup,uiMsg,wParam,
				MAKELONG(ptScreen.x,ptScreen.y));
		if ((wSel = (WORD)SendMessage(hPopup,
			LB_GETCURSEL,0,0L)) != (WORD)-1) {
		    hMenu = (HMENU)GetWindowID(hPopup);
		    hPopupMenu32 = GetMenuHandle32(hMenu);
		    mnis.wAction = LCA_GET|LCA_FLAGS;
		    mnis.wItemFlags = MF_BYPOSITION;
		    mnis.wPosition = wSel;
		    lFlags = LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
					(LPARAM)&mnis);
		    if (lptps->bSystemMenu)
			lFlags |= MF_SYSMENU;
		    if ((lFlags >= 0) &&
			    (lFlags & (MF_GRAYED|MF_DISABLED|MF_SEPARATOR))) {
		        SetPopupMenuSel(lptps->hWndOwner,hPopup,0,
					(lFlags & MF_SYSMENU)|MF_MOUSESELECT);
		    }
		    else {
			mnis.wAction = LCA_GET|LCA_ITEMID;
			mnis.wPosition = wSel;
			wItemID = (WORD)LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
					(LPARAM)&mnis);
			if (lptps->uiFlags & TP_MENUBAR)
			    ChangeMBSelection(hWnd,hMenu32,lptps,wPrevSel,
							(WORD)-1);
			if (lFlags & MF_SYSMENU) {
			    GetCursorPos(&pt);
			    PostMessage(lptps->hWndOwner,WM_SYSCOMMAND,
					(WPARAM)wItemID,MAKELONG(pt.x,pt.y));
			}
			else
			    PostMessage(lptps->hWndOwner,WM_COMMAND,
					GET_WM_COMMAND_MPS(wItemID,0,0));
			SetWindowWord(hWnd,TP_STATUS,1);
		    }
		    RELEASELBOXINFO((LPLISTBOXINFO)hPopupMenu32);		    
		}
	    }
	    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
	    return 0;

	case WM_LBUTTONDBLCLK:
	    if (!(lptps = (LPTRACKPOPUPSTRUCT)GetWindowLong
			(hWnd,LWD_LPMENUDATA)))
		return FALSE;
	    if (lptps->uiFlags & TP_MENUBAR) {
		if (!TestWF(lptps->hWndOwner,WFCHILDMAXIMIZED))
		    return 0;
		if (!(hMenu = lptps->hMenu))
		    return 0;
		if (!(hMenu32 = GetMenuHandle32(hMenu)))
		    return FALSE;
		pt.x = (int)((signed short)LOWORD(lParam));
		pt.y = (int)((signed short)HIWORD(lParam));
		GetClientRect(hWnd, &rcRect);
		if (!PtInRect(&rcRect,pt)) {
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return 0;
		}
		wSel = MenuHitTest(hMenu32,&pt);
		mnis.wPosition = wSel;
		mnis.wItemFlags = MF_BYPOSITION;
		mnis.wAction = LCA_GET | LCA_FLAGS;
		lFlags = LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
		if (!(lFlags & (MF_POPUP|MF_BITMAP))) {
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return FALSE;
		}
		mnis.wAction = LCA_GET|LCA_ITEMID;
		mnis.wPosition = wSel;
		wItemID = LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
		if (!(hPopupMenu32 = GetMenuHandle32(wItemID))) {
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return FALSE;
		}
		hPopupMenu32a = TWIN_FindMenuItem(hPopupMenu32,(WORD)SC_CLOSE);
		if (hPopupMenu32a) {
		    if (hPopupMenu32a != hPopupMenu32)
		      RELEASELBOXINFO((LISTBOXINFO) hPopupMenu32a);
		    lptps->uiFlags &= ~LSF_BUTTONDOWN;
		    ChangeMBSelection(hWnd,hMenu32,lptps,wSel,(WORD)-1);
		    SetWindowWord(hWnd,TP_STATUS,1);
		    if (lptps->bSystemMenu)
			PostMessage(lptps->hWndOwner,WM_SYSCOMMAND,
				(WPARAM)SC_CLOSE,lParam);
		    else
			PostMessage(lptps->hWndOwner,WM_COMMAND,
				GET_WM_COMMAND_MPS(SC_CLOSE,0,0));
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    RELEASELBOXINFO((LPLISTBOXINFO)hPopupMenu32);
		    return TRUE;
		}
		else {
		  RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		  return FALSE;
		}
	    }
	    else if (lptps->bSystemMenu) {
		if (IsIconic(lptps->hWndOwner))
		    PostMessage(lptps->hWndOwner,WM_SYSCOMMAND,
				(WPARAM)SC_RESTORE,lParam);
		else
		    PostMessage(lptps->hWndOwner,WM_SYSCOMMAND,
				(WPARAM)SC_CLOSE,lParam);
		SetWindowWord(hWnd,TP_STATUS,1);
		RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		return TRUE;
	    }
	    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
	    return 0;

	case MM_MENUINIT:
	    if (!(lptps = (LPTRACKPOPUPSTRUCT)GetWindowLong
				(hWnd,LWD_LPMENUDATA)))
		return 1;
	    if (!(lptps->uiFlags & LSF_CAPTUREACTIVE))
		return 1;
	    else if (!(lptps->uiFlags & TP_MENUBAR))
		return 1;
	    if (!(hMenu32 = GetMenuHandle32(lptps->hMenu)))
		return 1;
	    wSel = wParam;
	    mnis.wAction = LCA_GET|LCA_FLAGS;
	    mnis.wItemFlags = MF_BYPOSITION;
	    mnis.wPosition = wSel;
	    lFlags = LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
	    if (lptps->bSystemMenu)
		lFlags |= MF_SYSMENU;
	    if (lFlags < 0) {
	        RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		return 1;
	    }
	    ChangeMBSelection(hWnd,hMenu32,lptps,(WORD)-1,wSel);
	    mnis.wAction = LCA_GET|LCA_ITEMID;
	    wItemID = (WORD)LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
	    SendMessage(lptps->hWndOwner,WM_MENUSELECT,
		GET_WM_MENUSELECT_MPS(wItemID,LOWORD(lFlags),lptps->hMenu));
	    if (!(lFlags & MF_POPUP)) {
		ChangeMBSelection(hWnd,hMenu32,lptps,wSel,(WORD)-1);
		if (lFlags & MF_SYSMENU) {
		    GetCursorPos(&pt);
		    PostMessage(lptps->hWndOwner,WM_SYSCOMMAND,
				(WPARAM)wItemID,MAKELONG(pt.x,pt.y));
		}
		else
		    PostMessage(lptps->hWndOwner,WM_COMMAND,
				GET_WM_COMMAND_MPS(wItemID,0,0));
		SetWindowWord(hWnd,TP_STATUS,1);
		RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		return 0;
	    }
	    mnis.lpItemData = (LPSTR)&rcRect;
	    mnis.wAction = LCA_GET|LCA_RECT;
	    LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
	    pt.x = rcRect.left;
	    pt.y = rcRect.bottom - 1;
	    ClientToScreen(hWnd,&pt);
	    SetRect(&rcRect,pt.x,pt.y,0,0);
	    if (CreatePopup(hWnd,lptps,(HMENU)wItemID,wSel,&rcRect,TRUE))
		SetPopupMenuSel(lptps->hWndOwner,
			lptps->hPopups[0],0,lFlags & MF_SYSMENU);
	    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
	    return 0;

	case WM_CHAR:
	    if (!(lptps = (LPTRACKPOPUPSTRUCT)GetWindowLong
				(hWnd,LWD_LPMENUDATA)))
		return 1;
	    if (!(lptps->uiFlags & LSF_CAPTUREACTIVE))
		return 1;
	    if (!(hMenu32 = GetMenuHandle32(lptps->hMenu)))
		return 1;
	    mnis.wPosition = (WORD)-1;
	    mnis.wAction = LCA_SELECTION;
	    wPrevSel = (WORD)LBoxAPI(hMenu32,LBA_GETDATA,(LPARAM)&mnis);
	    if (lptps->nPopups) {
		hPopup = lptps->hPopups[lptps->nPopups-1];
		hPopupMenu = (HMENU)GetWindowID(hPopup);
		hPopupMenu32 = GetMenuHandle32(hPopupMenu);
		if (!hPopupMenu32) {
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return 1;
		}
		RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		hMenu32 = hPopupMenu32;
	    }
	    else if (!(lptps->uiFlags & TP_MENUBAR)) {
	            RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return 1;
	    }
	    mnis.wAction = LCA_ITEMCOUNT;
	    nCount = (int)LBoxAPI(hMenu32,LBA_GETDATA,(LPARAM)&mnis);
	    mnis.wItemFlags = MF_BYPOSITION;
	    c = toupper((char)wParam);
	    for (n=0; n<nCount; n++) {
		mnis.wPosition = (WORD)n;
		mnis.wAction = LCA_GET|LCA_FLAGS;
		lFlags = LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
		if ((lFlags < 0) ||
			    (lFlags & (MF_BITMAP|MF_SEPARATOR|MF_OWNERDRAW))) 
		    continue;
		mnis.wAction = LCA_GET|LCA_CONTENTS;
		lpItemData = (LPSTR)LBoxAPI(hMenu32,LBA_MODIFYITEM,
					(LPARAM)&mnis);
		if (!(HIWORD(lpItemData)) || !(lpTemp = strchr(lpItemData,'&')))
		    continue;
		if (toupper(*(LPSTR)(++lpTemp)) == c)
		    break;
	    }
	    if (n == nCount)
		return 1;
	    wSel = n;
	    mnis.wPosition = wSel;
	    mnis.wAction = LCA_GET|LCA_FLAGS;
	    mnis.wItemFlags = MF_BYPOSITION;
	    lFlags = LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
	    if (lptps->bSystemMenu)
		lFlags |= MF_SYSMENU;
	    mnis.wAction = LCA_GET|LCA_ITEMID;
	    wItemID = (WORD)LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
	    if (!lptps->nPopups) {
		if ((lFlags & MF_POPUP) &&
			!(lFlags & (MF_GRAYED|MF_DISABLED|MF_SEPARATOR))) {
		    ChangeMBSelection(hWnd,hMenu32,lptps,wPrevSel,wSel);
		    if (lFlags & (MF_GRAYED|MF_DISABLED))
			return 0;
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    mnis.lpItemData = (LPSTR)&rcRect;
		    mnis.wAction = LCA_GET|LCA_RECT;
		    LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
		    pt.x = rcRect.left;
		    pt.y = rcRect.bottom - 1;
		    ClientToScreen(hWnd,&pt);
		    SetRect(&rcRect,pt.x,pt.y,0,0);
		    if (CreatePopup(hWnd,lptps,(HMENU)wItemID,wSel,
				&rcRect,TRUE))
			SetPopupMenuSel(lptps->hWndOwner,
				lptps->hPopups[0],0,lFlags & MF_SYSMENU);
		}
		else {
		    ChangeMBSelection(hWnd,hMenu32,lptps,wPrevSel,(WORD)-1);
		    SetWindowWord(hWnd,TP_STATUS,1);
		    if (!(lFlags & (MF_GRAYED|MF_DISABLED))) {
			if (lFlags & MF_SYSMENU)
			    PostMessage(lptps->hWndOwner,
					WM_SYSCOMMAND,(WPARAM)wItemID,0L);
			else
			    PostMessage(lptps->hWndOwner,
					WM_COMMAND,
					GET_WM_COMMAND_MPS(wItemID,0,0));
		    }
		}
	    }
	    else {
		if (lFlags & (MF_GRAYED|MF_DISABLED)) {
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return 0;
		}
		if (lFlags & MF_POPUP) {
		    SetPopupMenuSel(lptps->hWndOwner,hPopup,wSel,
				lFlags & MF_SYSMENU);
		    mnis.lpItemData = (LPSTR)&rcRect;
		    mnis.wAction = LCA_GET|LCA_RECT;
		    LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
		    mnis.wAction = LCA_GET|LCA_INDENTS;
		    dwIndents = LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
					(LPARAM)&mnis);
		    pt.x = rcRect.right - HIWORD(dwIndents);
		    pt.y = rcRect.top;
		    ClientToScreen(hWnd,&pt);
		    SetRect(&rcRect,pt.x,pt.y,0,0);
		    if (CreatePopup(hWnd,lptps,(HMENU)wItemID,wSel,
				&rcRect,TRUE))
			SetPopupMenuSel(lptps->hWndOwner,
				lptps->hPopups[lptps->nPopups-1],
				0,lFlags & MF_SYSMENU);
		}
		else {
		    if (lptps->uiFlags & TP_MENUBAR)
			ChangeMBSelection(hWnd,GetMenuHandle32(lptps->hMenu),
				lptps,wPrevSel,(WORD)-1);
		    SetWindowWord(hWnd,TP_STATUS,1);
		    if (lFlags & MF_SYSMENU)
			PostMessage(lptps->hWndOwner,
				WM_SYSCOMMAND,(WPARAM)wItemID,0L);
		    else
			PostMessage(lptps->hWndOwner,
				WM_COMMAND,
				GET_WM_COMMAND_MPS(wItemID,0,0));
		}
	    }
	    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
	    return 0;

	case WM_KEYDOWN:
	    if (!(lptps = (LPTRACKPOPUPSTRUCT)GetWindowLong
				(hWnd,LWD_LPMENUDATA)))
		return 1;
	    if (!(lptps->uiFlags & LSF_CAPTUREACTIVE))
		return 1;
	    if (!(hMenu32 = GetMenuHandle32(lptps->hMenu)))
		return 1;

	    switch (wParam) {
		case VK_LEFT:
		case VK_RIGHT:
		    if (!(lptps->uiFlags & TP_MENUBAR)) {
		        RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			return 0;
		    }
		    if ((lptps->nPopups > 1) && (wParam == VK_LEFT)) {
			lptps->nPopups--;
			DestroyWindow(lptps->hPopups[lptps->nPopups]);
			lptps->hPopups[n] = 0;
			lptps->wPopupFlags[lptps->nPopups] &= ~PSF_POPUPACTIVE;
			hPopup = lptps->hPopups[lptps->nPopups-1];
			hPopupMenu = (HMENU)GetWindowID(hPopup);
			hPopupMenu32 = GetMenuHandle32(hPopupMenu);
			mnis.wAction = LCA_SELECTION;
			mnis.wPosition = (WORD)-1;
			wSel = (WORD)LBoxAPI(hPopupMenu32,LBA_GETDATA,
						(LPARAM)&mnis);
			mnis.wItemFlags = MF_BYPOSITION;
			mnis.wPosition = wSel;
			mnis.wAction = LCA_GET | LCA_FLAGS;
			lFlags = LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
						(LPARAM)&mnis);
			if (lptps->bSystemMenu)
			    lFlags |= MF_SYSMENU;
			mnis.wAction = LCA_GET | LCA_ITEMID;
			wItemID = (WORD)LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
						(LPARAM)&mnis);
			SendMessage(lptps->hWndOwner,WM_MENUSELECT,
				GET_WM_MENUSELECT_MPS(wItemID,LOWORD(lFlags),
							hMenu));
			RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			RELEASELBOXINFO((LPLISTBOXINFO)hPopupMenu32);
			return 0;
		    }
		    if (lptps->nPopups && (wParam == VK_RIGHT)) {
			hPopup = lptps->hPopups[lptps->nPopups-1];
			hPopupMenu = (HMENU)GetWindowID(hPopup);
			hPopupMenu32 = GetMenuHandle32(hPopupMenu);
			mnis.wAction = LCA_SELECTION;
			mnis.wPosition = (WORD)-1;
			wSel = (WORD)LBoxAPI(hPopupMenu32,LBA_GETDATA,
						(LPARAM)&mnis);
			mnis.wItemFlags = MF_BYPOSITION;
			mnis.wPosition = wSel;
			mnis.wAction = LCA_GET | LCA_FLAGS;
			lFlags = LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
						(LPARAM)&mnis);
			if (lptps->bSystemMenu)
			    lFlags |= MF_SYSMENU;
			if ((lFlags & MF_POPUP) &&
			!(lFlags & (MF_GRAYED|MF_DISABLED|MF_SEPARATOR))) {
			    mnis.wAction = LCA_GET | LCA_ITEMID;
			    wItemID = (WORD)LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
						(LPARAM)&mnis);
			    mnis.wAction = LCA_GET|LCA_RECT;
			    mnis.lpItemData = (LPSTR)&rcRect;
			    LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
			    mnis.wAction = LCA_GET|LCA_INDENTS;
			    dwIndents = LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
						(LPARAM)&mnis);
			    pt.x = rcRect.right - HIWORD(dwIndents);
			    pt.y = rcRect.top;
			    ClientToScreen(hPopup,&pt);
			    SetRect(&rcRect,pt.x,pt.y,0,0);
			    if (CreatePopup(hWnd,lptps,(HMENU)wItemID,
					wSel,&rcRect,TRUE))
				SetPopupMenuSel(lptps->hWndOwner,
					lptps->hPopups[lptps->nPopups-1],
					0,lFlags & MF_SYSMENU);
			    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			    RELEASELBOXINFO((LPLISTBOXINFO)hPopupMenu32);
			    return 0;
			}
			/* else -- fall through */
		    }
		    mnis.wPosition = (WORD)-1;
		    mnis.wAction = LCA_SELECTION;
		    wPrevSel = (WORD)LBoxAPI(hMenu32,LBA_GETDATA,(LPARAM)&mnis);
		    if (wPrevSel == (WORD)-1) {
		        RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			return 0;
		    }
		    mnis.wAction = LCA_ITEMCOUNT;
		    nCount = (int)LBoxAPI(hMenu32,LBA_GETDATA,(LPARAM)&mnis);
		    wSel = wPrevSel;
		    (wParam == VK_LEFT) ? wSel-- : wSel++;
		    if (wSel == (WORD)-1)
			wSel = nCount - 1;
		    else if (wSel >= (WORD)nCount)
			wSel = wSel - nCount;
		    ChangeMBSelection(hWnd,hMenu32,lptps,wPrevSel,wSel);
		    mnis.wItemFlags = MF_BYPOSITION;
		    mnis.wAction = LCA_GET | LCA_FLAGS;
		    mnis.wPosition = wSel;
		    lFlags = LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
		    if (lptps->bSystemMenu)
			lFlags |= MF_SYSMENU;
		    mnis.wAction = LCA_GET|LCA_ITEMID;
		    wItemID = (WORD)LBoxAPI(hMenu32,LBA_MODIFYITEM,
					(LPARAM)&mnis);
		    SendMessage(lptps->hWndOwner,WM_MENUSELECT,
			GET_WM_MENUSELECT_MPS(wItemID,LOWORD(lFlags),hMenu));
		    if (!((lFlags & MF_POPUP) &&
			!(lFlags & (MF_GRAYED|MF_DISABLED|MF_SEPARATOR)))) {
		        RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			return 0;
		    }
		    mnis.lpItemData = (LPSTR)&rcRect;
		    mnis.wAction = LCA_GET|LCA_RECT;
		    LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
		    pt.x = rcRect.left;
		    pt.y = rcRect.bottom - 1;
		    ClientToScreen(hWnd,&pt);
		    SetRect(&rcRect,pt.x,pt.y,0,0);
		    if (CreatePopup(hWnd,lptps,(HMENU)wItemID,wSel,
					&rcRect,TRUE))
			SetPopupMenuSel(lptps->hWndOwner,lptps->hPopups[0],
					0,lFlags & MF_SYSMENU);
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return 0;
		case VK_RETURN:
		    mnis.wPosition = (WORD)-1;
		    mnis.wAction = LCA_GET|LCA_SELECTION;
		    wPrevSel = (WORD)LBoxAPI(hMenu32,LBA_GETDATA,(LPARAM)&mnis);
		    if (wPrevSel == (WORD)-1) {
		        RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			return 0;
		    }
		    if (lptps->nPopups == 0) {	/* item on menubar */
			if (!(lptps->uiFlags & TP_MENUBAR)) {
			    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			    return 0;
			}
			mnis.wPosition = wPrevSel;
			mnis.wAction = LCA_GET|LCA_FLAGS;
			mnis.wItemFlags = MF_BYPOSITION;
			lFlags = LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
			if (lptps->bSystemMenu)
			    lFlags |= MF_SYSMENU;
			if (lFlags & (MF_GRAYED|MF_DISABLED)) {
			    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			    return 0;
			}
			mnis.wAction = LCA_GET|LCA_ITEMID;
			wItemID = (WORD)LBoxAPI(hMenu32,LBA_MODIFYITEM,
				(LPARAM)&mnis);
			if (lFlags & MF_POPUP) {
			    mnis.lpItemData = (LPSTR)&rcRect;
			    mnis.wAction = LCA_GET|LCA_RECT;
			    LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
			    pt.x = rcRect.left;
			    pt.y = rcRect.bottom - 1;
			    ClientToScreen(hWnd,&pt);
			    SetRect(&rcRect,pt.x,pt.y,0,0);
			    if (CreatePopup(hWnd,lptps,(HMENU)wItemID,wPrevSel,
					&rcRect,TRUE))
				SetPopupMenuSel(lptps->hWndOwner,
					lptps->hPopups[0],
					0,lFlags & MF_SYSMENU);
			}
			else {
			    ChangeMBSelection(hWnd,hMenu32,lptps,wPrevSel,
					(WORD)-1);
			    SetWindowWord(hWnd,TP_STATUS,1);
			    if (!(lFlags & (MF_GRAYED|MF_DISABLED))) {
				if (lFlags & MF_SYSMENU) 
				    PostMessage(lptps->hWndOwner,
					WM_SYSCOMMAND,(WPARAM)wItemID,0L);
				else
				    PostMessage(lptps->hWndOwner,
					WM_SYSCOMMAND,(WPARAM)wItemID,0L);
			    }
			}
		    }
		    else {
			hPopup = lptps->hPopups[lptps->nPopups-1];
			hPopupMenu = (HMENU)GetWindowID(hPopup);
			hPopupMenu32 = GetMenuHandle32(hPopupMenu);
			wSel = (WORD)SendMessage(hPopup,LB_GETCURSEL,0,0L);
			if (wSel == (WORD)LB_ERR) {
			    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			    RELEASELBOXINFO((LPLISTBOXINFO)hPopupMenu32);
			    return 0;
			}
			mnis.wPosition = wSel;
			mnis.wAction = LCA_GET|LCA_FLAGS;
			mnis.wItemFlags = MF_BYPOSITION;
			lFlags = LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
					(LPARAM)&mnis);
			if (lptps->bSystemMenu)
			    lFlags |= MF_SYSMENU;
			mnis.wAction = LCA_GET|LCA_ITEMID;
			wItemID = (WORD)LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
				(LPARAM)&mnis);
			if ((lFlags & MF_POPUP) && 
			      !(lFlags & (MF_GRAYED|MF_DISABLED))) {
			    mnis.wAction = LCA_GET|LCA_RECT;
			    mnis.lpItemData = (LPSTR)&rcRect;
			    LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
			    mnis.wAction = LCA_GET|LCA_INDENTS;
			    dwIndents = LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
					(LPARAM)&mnis);
			    pt.x = rcRect.right - HIWORD(dwIndents);
			    pt.y = rcRect.top;
			    ClientToScreen(hPopup,&pt);
			    SetRect(&rcRect,pt.x,pt.y,0,0);
			    if (CreatePopup(hWnd,lptps,(HMENU)wItemID,
					wSel,&rcRect,TRUE))
				SetPopupMenuSel(lptps->hWndOwner,
					lptps->hPopups[lptps->nPopups-1],
					0,lFlags & MF_SYSMENU);
			    RELEASELBOXINFO((LPLISTBOXINFO)hPopupMenu32);
			}
			else {
			    if (lptps->uiFlags & TP_MENUBAR)
				ChangeMBSelection(hWnd,hMenu32,lptps,wPrevSel,
					(WORD)-1);
			    SetWindowWord(hWnd,TP_STATUS,1);
			    if (!(lFlags &
					(MF_GRAYED|MF_DISABLED|MF_SEPARATOR))) {
				if (lFlags & MF_SYSMENU)
				    PostMessage(lptps->hWndOwner,WM_SYSCOMMAND,
					(WPARAM)wItemID,0L);
				else
				    PostMessage(lptps->hWndOwner,WM_COMMAND,
					GET_WM_COMMAND_MPS(wItemID,0,0));
			    }
			}
		    }
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return 0;
		case VK_UP:
		case VK_DOWN:
		    if (lptps->nPopups == 0) {
			if (!(lptps->uiFlags & TP_MENUBAR)) {
			    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			    return 0;
			}
		        mnis.wPosition = (WORD)-1;
		        mnis.wAction = LCA_SELECTION;
		        wPrevSel = (WORD)LBoxAPI(hMenu32,LBA_GETDATA,
					(LPARAM)&mnis);
		        if (wPrevSel == (WORD)-1) {
			    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			    return 0;
			}
			mnis.wPosition = wSel;
			mnis.wAction = LCA_GET|LCA_FLAGS;
			mnis.wItemFlags = MF_BYPOSITION;
			/*  FIX_ME   Where is hPopupMenu32 getting  SET?! */
			lFlags = LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
					(LPARAM)&mnis);
			if (lptps->bSystemMenu)
			    lFlags |= MF_SYSMENU;
			mnis.wAction = LCA_GET|LCA_ITEMID;
			wItemID = (WORD)LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
				(LPARAM)&mnis);
			if (!(lFlags & MF_POPUP)) {
			    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			    /*  FIX_ME  hPopupMenu32 as well?? */
			    return 0;
			}
			mnis.lpItemData = (LPSTR)&rcRect;
			mnis.wAction = LCA_GET|LCA_RECT;
			LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
			pt.x = rcRect.left;
			pt.y = rcRect.bottom - 1;
			ClientToScreen(hWnd,&pt);
			SetRect(&rcRect,pt.x,pt.y,0,0);
			if (CreatePopup(hWnd,lptps,(HMENU)wItemID,wSel,
					&rcRect,TRUE))
			    SetPopupMenuSel(lptps->hWndOwner,lptps->hPopups[0],
					0,lFlags & MF_SYSMENU);
		    }
		    else {
			hPopup = lptps->hPopups[lptps->nPopups-1];
			hPopupMenu = (HMENU)GetWindowID(hPopup);
			hPopupMenu32 = GetMenuHandle32(hPopupMenu);
			wPrevSel = (WORD)SendMessage(hPopup,LB_GETCURSEL,0,0L);
			if (wPrevSel == (WORD)LB_ERR) {
			    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
			    RELEASELBOXINFO((LPLISTBOXINFO)hPopupMenu32);
			    return 0;
			}
			mnis.wAction = LCA_ITEMCOUNT;
			nCount = (int)LBoxAPI(hPopupMenu32,LBA_GETDATA,
					(LPARAM)&mnis);
			mnis.wAction = LCA_GET|LCA_FLAGS;
			mnis.wItemFlags = MF_BYPOSITION;
			wSel = wPrevSel;
			if (wParam == VK_UP) {
			    do {
				wSel--;
				if (wSel == (WORD)-1)
				    wSel = nCount - 1;
				mnis.wPosition = wSel;
				lFlags = LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
					(LPARAM)&mnis);
			    } while (lFlags & MF_SEPARATOR);
			}
			else {	/* VK_DOWN */
			    do {
				wSel++;
				if (wSel >= (WORD)nCount)
				    wSel -= nCount;
				mnis.wPosition = wSel;
				lFlags = LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,
					(LPARAM)&mnis);
			    } while (lFlags & MF_SEPARATOR);
			}
		 	if (lptps->bSystemMenu)
			    lFlags |= MF_SYSMENU;
			SetPopupMenuSel(lptps->hWndOwner,hPopup,wSel,
					lFlags & MF_SYSMENU);
			RELEASELBOXINFO((LPLISTBOXINFO)hPopupMenu32);
		    }
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return 0;
		case VK_ESCAPE:
		    if (lptps->nPopups) {
			DestroyWindow(lptps->hPopups[lptps->nPopups - 1]);
			lptps->hPopups[lptps->nPopups - 1] = 0;
                        lptps->nPopups--;
			if (lptps->nPopups)
			    lptps->wPopupFlags[lptps->nPopups - 1] &=
                                ~(PSF_POPSELKILLED|PSF_POPUPACTIVE);
			if (!(lptps->uiFlags & TP_MENUBAR))
			    SetWindowWord(hWnd,TP_STATUS,1);
		    }
		    else {
			if (lptps->uiFlags & TP_MENUBAR) {
			    mnis.wPosition = (WORD)-1;
			    mnis.wAction = LCA_SELECTION;
			    wPrevSel = (WORD)LBoxAPI(hMenu32,LBA_GETDATA,
					(LPARAM)&mnis);
			    if (wPrevSel != (WORD)-1)
				ChangeMBSelection(hWnd,hMenu32,lptps,
					wPrevSel,(WORD)-1);
			}
			SetWindowWord(hWnd,TP_STATUS,1);
		    }
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    return 0;
		default:
		    break;
	    }
	    break;
	case WM_SYSCHAR:
	case WM_SYSKEYUP:
	case WM_SYSKEYDOWN:
	    break;

	case WM_DRAWITEM:
	    if (!(lptps = (LPTRACKPOPUPSTRUCT)GetWindowLong
				(hWnd,LWD_LPMENUDATA)))
		return FALSE;
	    lpdis = (LPDRAWITEMSTRUCT)lParam;
	    if (!(lpdis->itemAction & (ODA_DRAWENTIRE|ODA_SELECT)))
		return 0;
	    if (lpdis->itemID == (UINT)-1) {
		/* this is to draw a focus in empty menu */
		return 0;
	    }
	    hMenu32 = GetMenuHandle32((HMENU)wParam);
	    DrawPopupMenuItem(hMenu32,lptps->hWndOwner,lpdis);
	    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
	    return 0L;

	case WM_MEASUREITEM:
	    if (!(lptps = (LPTRACKPOPUPSTRUCT)GetWindowLong
				(hWnd,LWD_LPMENUDATA)))
		return FALSE;
	    hMenu32 = GetMenuHandle32((HMENU)wParam);
	    lpmis = (LPMEASUREITEMSTRUCT)lParam;
	    mnis.wAction = LCA_GET|LCA_RECT;
	    mnis.wItemFlags = MF_BYPOSITION;
	    mnis.wPosition = (WORD)lpmis->itemID;
	    mnis.lpItemData = (LPSTR)&rcRect;
	    lFlags = LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
	    if (lFlags < 0) {
	        RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		return 0;
	    }
	    lpmis->itemHeight = rcRect.bottom - rcRect.top;
	    lpmis->itemWidth = rcRect.right - rcRect.left;
	    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
	    return 0L;

	case WM_NCDESTROY:
	    if ((lptps = (LPTRACKPOPUPSTRUCT)GetWindowLong
					(hWnd,LWD_LPMENUDATA))) {
		WinFree((LPSTR)lptps);
		SetWindowLong(hWnd,LWD_LPMENUDATA,0L);
	    }
	    return FALSE;

	case WM_COMMAND:
	    if (!(lptps = (LPTRACKPOPUPSTRUCT)
                        GetWindowLong(hWnd,LWD_LPMENUDATA)))
                return FALSE;

	    if (GET_WM_COMMAND_CMD(wParam,lParam) == LBN_SELCHANGE) {
		wSel = (WORD)SendMessage(GET_WM_COMMAND_HWND(wParam,lParam),
				LB_GETCURSEL,
				0,0L);
		if (wSel == (WORD)-1)
			break;
#ifdef	LATER
	/* how does this work in WIN32??? */
#endif
		hMenu32 = GetMenuHandle32((HMENU)wParam);
		mnis.wAction = LCA_GET|LCA_FLAGS;
		mnis.wItemFlags = MF_BYPOSITION;
		mnis.wPosition = wSel;
		lFlags = LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
		if (lFlags < 0) {
		    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
		    break;
		}
		mnis.wAction = LCA_GET|LCA_ITEMID;
		mnis.wPosition = wSel;
		wItemID = (WORD)LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
		if (lptps->bSystemMenu)
		    lFlags |= MF_SYSMENU;
		return SendMessage(lptps->hWndOwner,WM_MENUSELECT,
			GET_WM_MENUSELECT_MPS(wItemID,LOWORD(lFlags),wParam));
	    }
	    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
	    return 0L;

	case WM_CANCELMODE:
	    if (!(lptps = (LPTRACKPOPUPSTRUCT)GetWindowLong
				(hWnd,LWD_LPMENUDATA)))
		return FALSE;
	    if (lptps->uiFlags & LSF_CAPTUREACTIVE)
		ReleaseCapture();
		
	    if (lptps->nPopups) {
		for (n=lptps->nPopups-1; n>=0; n--) {
			DestroyWindow(lptps->hPopups[n]);
			lptps->hPopups[n] = 0;
			lptps->wPopupFlags[n] &= ~PSF_POPUPACTIVE;
		}
		lptps->nPopups = 0;
	    }
	    lptps->uiFlags &= ~(LSF_BUTTONDOWN|LSF_CAPTUREACTIVE);
	    if (lptps->uiFlags & TP_MENUBAR){
		if ((hMenu32 = GetMenuHandle32(lptps->hMenu))) {
		    mnis.wPosition = (WORD)-1;
		    mnis.wAction = LCA_SELECTION;
		    wSel = (WORD)LBoxAPI(hMenu32,LBA_GETDATA,(LPARAM)&mnis);
		    if (wSel != (WORD)-1)
			ChangeMBSelection(hWnd,hMenu32,lptps,wSel,(WORD)-1);
		}
	    }
	    SetWindowWord(hWnd,TP_STATUS,1);
	    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
	    return 0L;

	default:
	    break;
    }
    return DefWindowProc(hWnd,uiMsg,wParam,lParam);
}

static BOOL
CreatePopup (HWND hWndMenu,LPTRACKPOPUPSTRUCT lptps,HMENU hPopup,
			WORD wIndex, LPRECT lprc, BOOL fShow)
{
    DWORD dwMenuDim;
    HWND hWndFrame;
    int n,nScreenMax;
    HMENU32 hMenu32;

    if ( !lptps->nPopups ) {
	hWndFrame = TWIN_GetTopLevelFrame(lptps->hWndOwner);
	DRVCALL_WINDOWS(PWSH_RAISEWINDOW,0L,0L, WIN_GETDRVDATA(hWndFrame));
    }

    SendMessage(lptps->hWndOwner,WM_INITMENUPOPUP,
		(WPARAM)hPopup,MAKELONG(wIndex,lptps->bSystemMenu));

    if (!(lprc->right && lprc->bottom)) {
	dwMenuDim = CalcPopupMenuDimensions(hPopup,lptps->hWndOwner);
	lprc->right = LOWORD(dwMenuDim);
	lprc->bottom = HIWORD(dwMenuDim);
    }
    lprc->left--;	/* ???? -- sometime we should figure it out */

#ifdef LATER
    /* We should have some way to override SYSTEM.INI setting on screen */
    /* size and get physical screen width, otherwise it looks weird when */
    /* confined to 640 x 480 */
	/* tdd for now do the best we can */
#endif
    /* move the left of the menu, just enough to make it visible */
    nScreenMax = GetSystemMetrics(SM_CXSCREEN);
    if (lprc->left + lprc->right >= nScreenMax)
	lprc->left = nScreenMax - lprc->right;

    /* move the top of the menu, just enough to make it visible */
    nScreenMax = GetSystemMetrics ( SM_CYSCREEN );
    if ( lprc->top + lprc->bottom >= nScreenMax )
	{
		n = lprc->top + lprc->bottom - nScreenMax;
		lprc->top -= n;
#ifdef LATER
		if ( lptps->uiFlags & TP_MENUBAR || wIndex == 0 )
			lprc->top -= GetSystemMetrics ( SM_CYMENU );
#endif
	}

    /* LATER
     * This will kill the creation of an empty menu, ie. if its right or
     * bottom are 0.  Should we 1) create a non-empty list, or just fail
     * as this does?
     */
    if( lprc->right == 0 || lprc->bottom == 0)
	return FALSE;

    n = lptps->nPopups;
    hMenu32 = GetMenuHandle32(hPopup);
    if (!(lptps->hPopups[n] = CreateWindowEx(
		WS_EX_NOCAPTURE|WS_EX_SAVEBITS|WS_EX_POPUPMENU,
		"MENULBOX",
		NULL,
		WS_POPUP|WS_BORDER|WS_CLIPSIBLINGS|
		LBS_OWNERDRAWVARIABLE|LBS_PRELOADED|LBS_HASSTRINGS|LBS_NOTIFY,
		lprc->left,lprc->top,
		lprc->right,lprc->bottom,
		hWndMenu, 0,
		GetWindowInstance(hWndMenu),
		(LPVOID)hMenu32)))  {
        if (hMenu32)
	  RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
	return FALSE;
    }
    if (hMenu32)
      RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
    lptps->nPopups++;
    SetWindowID(lptps->hPopups[n],hPopup);
    if (n > 0)
	lptps->wPopupFlags[n-1] |= PSF_POPUPACTIVE;
    else
	lptps->uiFlags |= PSF_POPUPACTIVE;
    if (fShow) {
	SetWindowPos(lptps->hPopups[n],HWND_TOP,0,0,0,0,
		SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
	UpdateWindow(lptps->hPopups[n]);
    }
    return TRUE;
}

static WORD
MenuHitTest(HMENU32 hMenu32,POINT *lppt)
{
    MENUITEMSTRUCT mnis;
    int n,i;
    RECT rcItemRect;
    WORD wSel = (WORD)-1;

    mnis.wAction = LCA_ITEMCOUNT;
    mnis.wPosition = (WORD)-1;
    n = (int)LBoxAPI(hMenu32,LBA_GETDATA,(LPARAM)&mnis);
    mnis.wItemFlags = MF_BYPOSITION;
    mnis.lpItemData = (LPSTR)&rcItemRect;
    mnis.wAction = LCA_GET|LCA_RECT;
    for (i=0; i<n; i++) {
	mnis.wPosition = (WORD)i;
	LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
	if (PtInRect(&rcItemRect,*lppt)) {
	    wSel = (WORD)i;
	    break;
	}
    }
    return wSel;
}

static void
DrawPopupMenuItem(HMENU32 hMenu32, HWND hWndOwner, LPDRAWITEMSTRUCT lpdis )
{
    MENUITEMSTRUCT mnis;
    DRAWITEMSTRUCT dis;
    LONG lFlags;
    HBRUSH hBr;
    HPEN hPen;
    BITMAP bm;
    HDC hdcMemory = 0;
    HBITMAP hBitmap, hBmpOld, hCheck, hUncheck;
    LPSTR lpTab;
    char String[0x100];
    COLORREF crTextColor,crBkColor;
    DWORD dwIndents, dwROP;

    mnis.wAction = LCA_GET|LCA_FLAGS;
    mnis.wItemFlags = MF_BYPOSITION;
    mnis.wPosition = (WORD)lpdis->itemID;
    lFlags = LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
    if (lFlags < 0)
	return;
    if (lFlags & MF_SEPARATOR) {
	hBr = GetSysColorBrush(COLOR_MENU);
	FillRect(lpdis->hDC, &lpdis->rcItem, hBr);
	hPen = GetSysColorPen(COLOR_MENUTEXT);
	SelectObject(lpdis->hDC,hPen);
	MoveTo(lpdis->hDC,lpdis->rcItem.left,
		(lpdis->rcItem.top+lpdis->rcItem.bottom)/2);
	LineTo(lpdis->hDC,lpdis->rcItem.right,
		(lpdis->rcItem.top+lpdis->rcItem.bottom)/2);
	return;
    }
    mnis.wAction = LCA_GET|LCA_INDENTS;
    dwIndents = LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
    SetBkMode(lpdis->hDC,TRANSPARENT);
    if (lpdis->itemState & ODS_SELECTED) {
	hBr = GetSysColorBrush(COLOR_HIGHLIGHT);
	crBkColor = GetSysColor(COLOR_HIGHLIGHT);
	crTextColor = (LOWORD(lFlags) & (MF_DISABLED|MF_GRAYED))?
			GetSysColor(COLOR_GRAYTEXT):
			GetSysColor(COLOR_HIGHLIGHTTEXT);
    }
    else {
	hBr = GetSysColorBrush(COLOR_MENU);
	crBkColor = GetSysColor(COLOR_MENU);
	crTextColor = (LOWORD(lFlags) & (MF_DISABLED|MF_GRAYED))?
			GetSysColor(COLOR_GRAYTEXT):
			GetSysColor(COLOR_MENUTEXT);
    }
    if (lFlags & MF_OWNERDRAW) {
	dis.CtlType = ODT_MENU;
	dis.CtlID = 0;
	mnis.wAction = LCA_GET | LCA_ITEMID;
	dis.itemID = (UINT)((int)((short)LOWORD(LBoxAPI
			(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis))));
	dis.itemAction = lpdis->itemAction;
	dis.itemState = (lFlags & MF_CHECKED)?ODS_CHECKED:0;
	dis.itemState |= (lFlags & MF_DISABLED)?ODS_DISABLED:0;
	dis.itemState |= (lFlags & MF_GRAYED)?ODS_GRAYED:0;
	dis.itemState |= (lFlags & MF_HILITE)?ODS_SELECTED:0;
	dis.hwndItem = ((LPLISTBOXINFO)hMenu32)->ObjHead.hObj;
	dis.hDC = lpdis->hDC;
	memcpy((LPSTR)&dis.rcItem,(LPSTR)&lpdis->rcItem,sizeof(RECT));
	mnis.wAction = LCA_GET | LCA_CONTENTS;
	dis.itemData = LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
	SendMessage(hWndOwner,WM_DRAWITEM,0,(LPARAM)&dis);
    }
    else {
	FillRect(lpdis->hDC,&lpdis->rcItem,hBr);
	SetTextColor(lpdis->hDC,crTextColor);
	SetBkColor(lpdis->hDC,crBkColor);
	if (lFlags & MF_BITMAP) {
	    hBitmap = (HBITMAP)lpdis->itemData;
	    hdcMemory = CreateCompatibleDC(lpdis->hDC);
	    GetObject(hBitmap,sizeof(BITMAP),&bm);
	    hBmpOld = SelectObject(hdcMemory,hBitmap);
	    dwROP = ((lpdis->itemState & ODS_SELECTED) &&
			(bm.bmBitsPixel != 1)) ? NOTSRCCOPY : SRCCOPY;
	    BitBlt(lpdis->hDC,
			lpdis->rcItem.left+LOWORD(dwIndents),
			lpdis->rcItem.top,
			bm.bmWidth, bm.bmHeight,
			hdcMemory, 0, 0, dwROP);
	    SelectObject(hdcMemory,hBmpOld);
	    DeleteDC(hdcMemory);
	    lpdis->rcItem.left += LOWORD(dwIndents);
	    lpdis->rcItem.right -= HIWORD(dwIndents);
	}
	else {
	    lpdis->rcItem.left += LOWORD(dwIndents);
	    lpdis->rcItem.right -= HIWORD(dwIndents);
	    strcpy(String,(LPSTR)lpdis->itemData);
	    if ((lpTab = strchr(String,'\t'))) {
		*lpTab = 0;
		lpTab++;
		DrawText(lpdis->hDC,(LPCSTR)lpTab,
			strlen((LPSTR)lpTab),
			&lpdis->rcItem,
			DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
	    }
	    else {
		for (lpTab = String; *lpTab && ((int)(*lpTab) != 0x8); lpTab++);
		if (*lpTab) {
		    *lpTab = 0;
		    lpTab++;
		    DrawText(lpdis->hDC,(LPCSTR)lpTab,
				strlen(lpTab),
				&lpdis->rcItem,
				DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
		}
	    }
	    DrawText(lpdis->hDC,(LPCSTR)String,
			strlen((LPSTR)lpdis->itemData),
			&lpdis->rcItem,
			DT_LEFT | DT_SINGLELINE | DT_VCENTER);
	}
	if (lFlags & MF_CHECKED) {
	    mnis.wAction = LCA_GET|LCA_CHECKBMP;
	    hCheck = LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
	    DrawCheckMark(lpdis,lFlags,LOWORD(dwIndents),hCheck,
				crTextColor,crBkColor);
	}
	else {
	    mnis.wAction = LCA_GET|LCA_UNCHECKBMP;
	    hUncheck = LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
	    if (hUncheck) 
		DrawCheckMark(lpdis,lFlags,LOWORD(dwIndents),hUncheck,
				crTextColor,crBkColor);
	}
    }
    if (lFlags & MF_POPUP) 
	DrawPullrightGlyph(lpdis->hDC,&lpdis->rcItem,
				HIWORD(dwIndents),crTextColor);
}

static void
DrawCheckMark(LPDRAWITEMSTRUCT lpdis, DWORD lFlags, WORD wLeftIndent,
		HBITMAP hCheckBmp, COLORREF crTextColor, COLORREF crBkColor)
{
    LPRECT lprc;
    HDC hMemDC,hMemDC1;
    HBITMAP hBitmap,hBitmap1;
    HBITMAP hMonoBmp = 0;
    BITMAP bm;
    HPEN hPen;
    HBRUSH hBrush;
    POINT points[6];
    int X,Y;

    lprc = &lpdis->rcItem;

    if (hCheckBmp) {
	hMemDC = CreateCompatibleDC(lpdis->hDC);
	GetObject(hCheckBmp,sizeof(BITMAP),(LPVOID)&bm);
	SetTextColor(lpdis->hDC,crTextColor);
	SetBkColor(lpdis->hDC,crBkColor);

	/* It appears that color bitmaps get converted to mono here */
	/* in order to match menu colors */

	if (bm.bmBitsPixel != 1) {
	    hMemDC1 = CreateCompatibleDC(lpdis->hDC);
	    hMonoBmp = CreateBitmap(bm.bmWidth,bm.bmHeight,1,1,NULL);
	    hBitmap1 = SelectObject(hMemDC1,hCheckBmp);
	    hBitmap = SelectObject(hMemDC,hMonoBmp);
	    BitBlt(hMemDC,0,0,bm.bmWidth,bm.bmHeight,
		   hMemDC1,0,0,
	    	   (lpdis->itemState & ODS_SELECTED) ? NOTSRCCOPY : SRCCOPY);
	    SelectObject(hMemDC1,hBitmap1);
	    DeleteDC(hMemDC1);
	}
	else
	    hBitmap = SelectObject(hMemDC,hCheckBmp);

	BitBlt(lpdis->hDC,lprc->left - wLeftIndent,
			lprc->top + (lprc->bottom - lprc->top - bm.bmHeight)/2,
			bm.bmWidth,bm.bmHeight,hMemDC,0,0,SRCCOPY);
	SelectObject(hMemDC,hBitmap);

	if (hMonoBmp)
	    DeleteObject(hMonoBmp);

	DeleteDC(hMemDC);
    }
    else if (lFlags & MF_CHECKED) {
	hPen = CreatePen(PS_SOLID,1,crTextColor);
	hBrush = CreateSolidBrush(crTextColor);
	hPen = SelectObject(lpdis->hDC,hPen);
	hBrush = SelectObject(lpdis->hDC,hBrush);
	
	X = lprc->left - wLeftIndent/2;
	Y = lprc->top + (lprc->bottom - lprc->top)/2;
	
	points[0].x = X - 5;
	points[0].y = Y;
	points[1].x = X - 3;
	points[1].y = Y;
	points[2].x = X;
	points[2].y = Y + 3;
	points[3].x = X + 3;
	points[3].y = Y - 7;
	points[4].x = X + 5;
	points[4].y = Y - 7;
	points[5].x = X;
	points[5].y = Y + 5;
	Polygon(lpdis->hDC,points,6);

	hPen = SelectObject(lpdis->hDC,hPen);
	hBrush = SelectObject(lpdis->hDC,hBrush);
	DeleteObject(hPen);
	DeleteObject(hBrush);
    }
}

static void
ChangeMBSelection(HWND hWnd, HMENU32 hMenu32, LPTRACKPOPUPSTRUCT lptps,
			WORD wPrevSel, WORD wSel)
{
    HDC hDC;
    MENUITEMSTRUCT mnis;
    int n;

    if (wPrevSel == wSel)
	return;
    hDC = GetDC(hWnd);
    if (lptps->nPopups) {
	for (n=lptps->nPopups-1;n>=0;n--) {
	    DestroyWindow(lptps->hPopups[n]);
	    lptps->hPopups[n] = 0;
	    lptps->wPopupFlags[n] = 0;
	}
	lptps->nPopups = 0;
    }
    if (wPrevSel != (WORD)-1) {
	HiliteMenuItem(lptps->hWndOwner,lptps->hMenu,(UINT)wPrevSel,
					MF_BYPOSITION);
	MenuDrawItem(hDC,hMenu32,lptps->hWndOwner,wPrevSel,ODA_SELECT);
    }
    mnis.wAction = LCA_SELECTION | LCA_SET;
    mnis.wPosition = wSel;
    LBoxAPI(hMenu32,LBA_SETDATA,(LPARAM)&mnis);
    if (wSel != (WORD)-1) {
	HiliteMenuItem(lptps->hWndOwner,lptps->hMenu,(UINT)wSel,
					MF_HILITE|MF_BYPOSITION);
	MenuDrawItem(hDC,hMenu32,lptps->hWndOwner,wSel,ODA_SELECT);
    }
    lptps->uiFlags &= ~(PSF_POPSELKILLED|PSF_POPUPACTIVE);
    ReleaseDC(hWnd,hDC);
}

static void
SetPopupMenuSel(HWND hWndOwner,HWND hWndPopup,WORD wSel,WORD wMenuFlags)
{
    HMENU hPopupMenu;
    HMENU32 hPopupMenu32;
    MENUITEMSTRUCT mnis;
    WORD wItemID;
    LONG lFlags;

    SendMessage(hWndPopup,LB_SETCURSEL,(WPARAM)wSel,(LPARAM)0L);
    hPopupMenu = (HMENU)GetWindowID(hWndPopup);
    hPopupMenu32 = GetMenuHandle32(hPopupMenu);
    mnis.wPosition = wSel;
    mnis.wAction = LCA_GET|LCA_FLAGS;
    mnis.wItemFlags = MF_BYPOSITION;
    lFlags = LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
    if (lFlags >= 0) {

        mnis.wAction = LCA_GET|LCA_ITEMID;
        wItemID = (WORD)LBoxAPI(hPopupMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
        lFlags |= wMenuFlags; /* can be MF_SYSMENU and/or MF_MOUSESELECT */
        SendMessage(hWndOwner, WM_MENUSELECT,
		    GET_WM_MENUSELECT_MPS(wItemID,LOWORD(lFlags),hPopupMenu));
    }
    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
    RELEASELBOXINFO((LPLISTBOXINFO)hMenuPopup32);
}

void
TranslateMenuChar(HWND hWnd, WPARAM wKey)
{
    HMENU hMenu;
    HMENU32 hMenu32;
    HWND hWndMenu;
    MENUITEMSTRUCT mnis;
    int nCount,i;
    char c;
    LPSTR lpItemData,lpTemp;
    LONG lFlags;

    if (!(hMenu = (HMENU)GetWindowMenu(hWnd)))
	return;
    if (!(hMenu32 = GetMenuHandle32(hMenu)))
	return;

    mnis.wAction = LCA_ITEMCOUNT;
    mnis.wPosition = (WORD)-1;
    nCount = LBoxAPI(hMenu32,LBA_GETDATA,(LPARAM)&mnis);
    c = toupper((char)wKey);
    mnis.wItemFlags = MF_BYPOSITION;
    for (i=0; i<nCount; i++) {
	mnis.wPosition = (WORD)i;
	mnis.wAction = LCA_GET|LCA_FLAGS;
	lFlags = LBoxAPI(hMenu32,LBA_MODIFYITEM,(LPARAM)&mnis);
	if ((lFlags < 0) || (lFlags & (MF_BITMAP|MF_OWNERDRAW)))
	    continue;
	mnis.wAction = LCA_GET|LCA_CONTENTS;
	lpItemData = (LPSTR)LBoxAPI(hMenu32,LBA_MODIFYITEM,
				(LPARAM)&mnis);
	if (!(HIWORD(lpItemData)) || !(lpTemp = strchr(lpItemData,'&')))
	    continue;
	if (toupper(*(LPSTR)(++lpTemp)) == c)
	    break;
    }
    if (i != nCount) {
        hWndMenu = GetWindowFrame(hWnd);
        InternalMenuProc(hWndMenu,MM_MENUINIT,(WPARAM)i,0L);
    }
    RELEASELBOXINFO((LPLISTBOXINFO)hMenu32);
}

static LRESULT
InternalMenuProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    LPTRACKPOPUPSTRUCT lptps;
    HWND hWndOldFocus,hWndCurrentFocus;
    MSG msg,msgHook;
    POINT pt;
    int ret;

    if (!(lptps = (LPTRACKPOPUPSTRUCT)GetWindowLong(hWnd,LWD_LPMENUDATA)))
	return 0;

    SetWindowWord(hWnd,TP_STATUS,0);
    SetCapture(hWnd);
    lptps->uiFlags |= LSF_CAPTUREACTIVE;
    hWndOldFocus = SetFocus(hWnd);
    SendMessage(lptps->hWndOwner,WM_INITMENU,(WPARAM)lptps->hMenu,(LPARAM)0);
    SendMessage(hWnd,uiMsg,wParam,lParam);

    while (GetWindowWord(hWnd,TP_STATUS) == 0) {
	if (GetMessage(&msg,0,0,0)) {
	    TranslateMessage (&msg);

	    if ((ret = IsMouseOrKeyboardMsg(msg.message))) {
		msgHook = msg;
		if (ret == WM_MOUSEFIRST) {
		    msgHook.hwnd = lptps->hWndOwner;
		    pt.x = LOWORD(msg.lParam);
		    pt.y = HIWORD(msg.lParam);
		    MapWindowPoints(msg.hwnd,HWND_DESKTOP,&pt,1);
		    msgHook.lParam = MAKELPARAM(pt.x,pt.y);
		}
		else {
		    msgHook.hwnd = hWndOldFocus;
		}
		if (CallMsgFilter(&msgHook,MSGF_MENU))
		    continue;
	    }

	    DispatchMessage (&msg);
	}
	if (!PeekMessage(&msg,hWnd,0,0,PM_NOYIELD|PM_NOREMOVE))
	    SendMessage(lptps->hWndOwner,WM_ENTERIDLE,MSGF_MENU,
			(LPARAM)lptps->hWndOwner);
    }
    ReleaseCapture();
    lptps->uiFlags &= ~LSF_CAPTUREACTIVE;
    if (hWndOldFocus) {
	hWndCurrentFocus = GetFocus();
#ifdef NOT_NOW___
	if (GetTopLevelAncestor(hWndCurrentFocus) ==
			GetTopLevelAncestor(hWndOldFocus))
#endif /* def NOT_NOW___ */
	    SetFocus(hWndOldFocus);
	hWndOldFocus = 0;
    }
    SendMessage(lptps->hWndOwner,WM_MENUSELECT,GET_WM_MENUSELECT_MPS(0,-1,0));
    return 0;
}
