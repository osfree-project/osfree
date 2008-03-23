/*     
	Listbox.c	2.51
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
#include <ctype.h>

#include "windows.h"
#include "windowsx.h"

#include "kerndef.h"
#include "WinDefs.h"
#include "Log.h"
#include "Listbox.h"
#include "Menus.h"

/* external functions */
extern void GetNCRect(HWND, LPRECT);
extern BOOL TestWF(HWND, DWORD);
extern DWORD GetWF(HWND);
extern void ClearWF(HWND, DWORD);
extern void SetWF(HWND, DWORD);
extern HBRUSH GetClassBackgroundBrush(HWND);
extern HBRUSH GetControlBrush(HWND, HDC, WORD);
extern void ListDir(HWND,HWND,UINT,LPSTR);

/* exported functions */
LRESULT DefLISTBOXProc(HWND,UINT,WPARAM,LPARAM);
HMENU32 GetMenuHandle32(HMENU);
BOOL CheckMenuHandle32(HMENU);
LONG LBoxAPI(HMENU32, UINT, LPARAM);
void LBoxDefaultDrawItem(LPDRAWITEMSTRUCT);
void LBoxDefaultMeasureItem(HWND, LPMEASUREITEMSTRUCT);
int LBoxDefaultCompareItem(LPCOMPAREITEMSTRUCT);

/* internal routines */
static LONG LBoxCreate(HWND, LPCREATESTRUCT);
static void LBoxSendItemDraw(LPLISTBOXINFO, UINT, UINT, UINT);
static void LBoxPaint(LPLISTBOXINFO);
static void LBoxDrawItem(LPLISTBOXINFO, LPDRAWITEMSTRUCT);
static int LBoxAddItem(LPLISTBOXINFO, WORD, LPMENUITEMSTRUCT);
static LONG LBoxModifyItem(LPLISTBOXINFO, LPMENUITEMSTRUCT);
static LONG LBoxDeleteItem(LPLISTBOXINFO, LPMENUITEMSTRUCT);
static LONG LBoxFindItem(LPLISTBOXINFO, WORD, LPSTR, WORD);
static void LBoxResetContent(LPLISTBOXINFO, BOOL);
static BOOL LBoxGetSlot(LPLISTBOXINFO);
static void LBoxNotify(LPLISTBOXINFO, int, BOOL);
static LPLISTBOXITEM LBoxItemFromIndex(LPLISTBOXINFO, WORD);
static LPLISTBOXITEM LBoxItemFromCommand(LPLISTBOXINFO, LPWORD);
static LONG LBoxItemData(LPLISTBOXINFO, WORD, DWORD, int);
static void LBoxCalcFrame(LPLISTBOXINFO);
static void LBoxCalcItemSize(LPLISTBOXINFO, LPLISTBOXITEM, WORD);
static void LBoxRecalcPositions(LPLISTBOXINFO);
static WORD LBoxHitTest(LPLISTBOXINFO, POINT);
static void LBoxItemSelection(LPLISTBOXINFO, LPLISTBOXITEM, WORD, WORD, BOOL);
static void LBoxChangeSelection(LPLISTBOXINFO, WORD, WORD);
static DWORD LBoxMeasureItem(LPLISTBOXINFO, LPLISTBOXITEM,WORD);
static DWORD LBoxDoMeasure(LPLISTBOXINFO, LPLISTBOXITEM, LPMEASUREITEMSTRUCT);
static void LBoxEraseBackground(LPLISTBOXINFO, HDC);
static void LBoxSetExtent(LPLISTBOXINFO, WORD);
static void LBoxScrollHorz(LPLISTBOXINFO, UINT, int);
static LRESULT LBoxSetSelection(LPLISTBOXINFO, BOOL, int);
static LRESULT LBoxGetItemHeight(LPLISTBOXINFO, WORD);

static WNDPROC lpLBoxBinToNat = 0;


LRESULT
DefLISTBOXProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    LPLISTBOXINFO lpLBoxInfo;
    LPLISTBOXITEM lpLBItem;
    MENUITEMSTRUCT mis;
    RECT rcClient;
    WORD wIndex = 0;
    int nRet,fnScroll;
    POINT pt;
    DWORD dwExStyle;
    static HWND hWndCapture = (HWND)0;
    HCLASS32 hLBoxClass32;
    int nOffset;
    WORD wStart, wEnd, w;
    LRESULT rc;

    if ((lpLBoxInfo = (LPLISTBOXINFO)GetWindowLong(hWnd, LWD_LPMENUDATA))
			== (LPLISTBOXINFO)-1L)
	return (LRESULT)LB_ERR;

    switch(wMsg) {
	case WM_NCCREATE:
	    rc = LBoxCreate(hWnd, (LPCREATESTRUCT)lParam);
	    return rc;

	case WM_CREATE:
	    if (lpLBoxInfo->wLBoxStyle & LBS_DISABLENOSCROLL) {
		DWORD dwStyle = GetWindowStyle(hWnd);
		if (dwStyle & WS_VSCROLL) {
		    EnableScrollBar(hWnd,SB_VERT,ESB_DISABLE_BOTH);
		    LBoxSetState(lpLBoxInfo, LSF_VSCROLLDISABLED);
		}
		if (dwStyle & WS_HSCROLL) {
		    EnableScrollBar(hWnd,SB_HORZ,ESB_DISABLE_BOTH);
		    LBoxSetState(lpLBoxInfo, LSF_HSCROLLDISABLED);
		}
	    }
	    return TRUE;

	case WM_DESTROY:
	    /* set invalid ptr to the window */
	    SetWindowLong(hWnd, LWD_LPMENUDATA, -1L);
	    if (!(lpLBoxInfo->wLBoxStyle & LBS_PRELOADED)) {
		/* free all allocated data */
		LBoxResetContent(lpLBoxInfo,TRUE);
		/* destroy the listbox handle */
		FREELBOX(lpLBoxInfo->ObjHead.hObj);
	    }
	    else {
		/* reset content without destroying the data,
		   reset hWnd field in the structure */
		LBoxResetContent(lpLBoxInfo,FALSE);
		lpLBoxInfo->hWnd = 0;
	    }
	    return 1L;

        case WM_ERASEBKGND:
	    LBoxEraseBackground(lpLBoxInfo,(HDC)wParam);
	    return 1L;

        case WM_PAINT:
	    LBoxPaint(lpLBoxInfo);
            return 1L;

	case WM_GETDLGCODE:
	    return (LONG)(DLGC_WANTCHARS | DLGC_WANTARROWS);

	case WM_GETFONT:
	    return (LRESULT)((lpLBoxInfo)?lpLBoxInfo->hFont:0);

	case WM_SETFONT:
	    lpLBoxInfo->hFont = (HFONT)wParam;
	    LBoxCalcFrame(lpLBoxInfo);
	    lpLBoxInfo->bPosChanged = TRUE;
	    if (lParam)
		InvalidateRgn(hWnd,(HRGN)0,TRUE);
	    return 1L;

	case WM_SETFOCUS:
            {
            LPLISTBOXITEM lpLBitem;
	    WORD          wState;

	    LBoxSetState(lpLBoxInfo, LSF_FOCUS);
	    wIndex = lpLBoxInfo->wSelection;
	    lpLBitem = LBoxItemFromIndex(lpLBoxInfo, wIndex);
	    if (lpLBitem)
               wState = (lpLBitem->wItemFlags & LIF_HILITE) ? ODS_SELECTED : 0;
            else 
		wState = 0;

	    wState |= ODS_FOCUS;
	    LBoxSendItemDraw(lpLBoxInfo, (UINT)wIndex, ODA_FOCUS, wState);
	    LBoxNotify(lpLBoxInfo, LBN_SETFOCUS, FALSE);
	    return 1L;	    
	    }

	case WM_KILLFOCUS:
            {
            LPLISTBOXITEM lpLBitem;
	    WORD          wState;

	    LBoxClearState(lpLBoxInfo, LSF_FOCUS);
	    wIndex = lpLBoxInfo->wSelection;
	    lpLBitem = LBoxItemFromIndex(lpLBoxInfo, wIndex);
	    if (lpLBitem)
                wState = (lpLBitem->wItemFlags & LIF_HILITE) ? ODS_SELECTED : 0;
            else 
		wState = 0;

	    LBoxSendItemDraw(lpLBoxInfo, (UINT)wIndex, ODA_FOCUS, wState);
	    LBoxNotify(lpLBoxInfo, LBN_KILLFOCUS, FALSE);
	    return 1L;	 
	    }

	case WM_CHAR:
	    nRet = -1;
	    if ((lpLBoxInfo->wLBoxStyle & LBS_WANTKEYBOARDINPUT) &&
			!(lpLBoxInfo->wLBoxStyle & LBS_HASSTRINGS)) {
		nRet = (int)(short)SendMessage(lpLBoxInfo->hWndParent,
			WM_CHARTOITEM,
			GET_WM_CHARTOITEM_MPS(wParam,0,lpLBoxInfo->hWnd));
		/* LOWORD of lParam should be caret pos */
		if (nRet == -2)
		    return 1L;
	    }
	    if (nRet >= 0) {
		wIndex = (WORD)nRet;
		LBoxChangeSelection(lpLBoxInfo,wIndex,LIF_BYPOSITION);
	    }
	    else if (isalnum(wParam)) {
		if (!(lpLBoxInfo->wLBoxStyle & LBS_HASSTRINGS))
		    wIndex = (WORD)-1;
		else {
		    char szFindChar[2];
		    wIndex = (lpLBoxInfo->wSelection == (WORD)-1)?
				0:lpLBoxInfo->wSelection;
		    szFindChar[0] = LOBYTE(wParam);
		    szFindChar[1] = '\0';
		    wIndex = LBoxFindItem(lpLBoxInfo,wIndex,szFindChar,
				LB_SELECTSTRING);
		}
	    }
	    if ((wIndex == (WORD)LB_ERR) ||
		((wIndex >= lpLBoxInfo->wTopIndex) && (wIndex <
			(WORD)(lpLBoxInfo->wTopIndex+lpLBoxInfo->wFrameSize))))
		return 0L;
	    if (wIndex < lpLBoxInfo->wTopIndex)
		lpLBoxInfo->wTopIndex = wIndex;
	    else
		lpLBoxInfo->wTopIndex = wIndex-lpLBoxInfo->wFrameSize+1;
	    if (lpLBoxInfo->wState & LSF_SCROLLENABLED) {
		if (lpLBoxInfo->wLBoxStyle & LBS_MULTICOLUMN)
		    fnScroll = SB_HORZ;
		else
		    fnScroll = SB_VERT;
		SetScrollPos(lpLBoxInfo->hWnd,fnScroll,
					lpLBoxInfo->wTopIndex,TRUE);
	    }
	    lpLBoxInfo->bPosChanged = TRUE;
	    InvalidateRgn(hWnd,(HRGN)0,TRUE);
	    return 0L;

	case WM_KEYDOWN:
	    nRet = -1;
	    if (lpLBoxInfo->wLBoxStyle & LBS_WANTKEYBOARDINPUT) {
		nRet = (int)SendMessage(lpLBoxInfo->hWndParent,WM_VKEYTOITEM,
			GET_WM_VKEYTOITEM_MPS(wParam,0,lpLBoxInfo->hWnd));
		/* LOWORD of lParam should be caret pos */
		if (nRet == -2)
		    return 1L;
	    }
	    if (nRet >= 0) 
		wIndex = (WORD)nRet;
	    else {	/* default action */
		if (lpLBoxInfo->wItemCount == 0)
		    return 1L;
		switch (wParam) {
		    case VK_LEFT:
#ifdef LATER
			if (lpLBoxInfo->wLBoxStyle & LBS_MULTICOLUMN) {}
#endif
		    case VK_UP:
			wIndex = lpLBoxInfo->wSelection;
			if ((wIndex > 0) && (wIndex != (WORD)-1))
			    wIndex--;
			else
			    return 1L;
			break;
		    case VK_RIGHT:
#ifdef LATER
			if (lpLBoxInfo->wLBoxStyle & LBS_MULTICOLUMN) {}
#endif
		    case VK_DOWN:
			wIndex = lpLBoxInfo->wSelection;
			if ((wIndex != (WORD)-1) && (wIndex <
				( WORD )(lpLBoxInfo->wTopIndex+lpLBoxInfo->wFrameSize)))
			    wIndex++;
			else
			    return 1L;
			break;
		    case VK_HOME:
			wIndex = 0;
			break;
		    case VK_END:
			wIndex = lpLBoxInfo->wItemCount;
			break;
#ifdef LATER
		    case VK_PRIOR:
		    case VK_NEXT:
#endif
		    default:
			return 1L;
		}
	    }
	    LBoxChangeSelection(lpLBoxInfo,wIndex,LIF_BYPOSITION);
	    if ((wIndex == (WORD)LB_ERR) ||
		    ((wIndex >= lpLBoxInfo->wTopIndex) &&
		     (wIndex < (WORD)
			(lpLBoxInfo->wTopIndex+lpLBoxInfo->wFrameSize))))
		    return 0L;
	    if (wIndex < lpLBoxInfo->wTopIndex)
		lpLBoxInfo->wTopIndex = wIndex;
	    else
		lpLBoxInfo->wTopIndex = wIndex-lpLBoxInfo->wFrameSize+1;
	    if (lpLBoxInfo->wState & LSF_SCROLLENABLED) {
		if (lpLBoxInfo->wLBoxStyle & LBS_MULTICOLUMN)
		    fnScroll = SB_HORZ;
		else
		    fnScroll = SB_VERT;
		SetScrollPos(lpLBoxInfo->hWnd,fnScroll,
			lpLBoxInfo->wTopIndex,TRUE);
	    }
	    lpLBoxInfo->bPosChanged = TRUE;
	    InvalidateRgn(hWnd,(HRGN)0,TRUE);
	    return 1L;

	case WM_MOUSEMOVE:
	    if (!LBoxTestState(lpLBoxInfo,LSF_BUTTONDOWN))
		return (LRESULT)LB_ERR;
	    pt.x = (int)(short)LOWORD(lParam);
	    pt.y = (int)(short)HIWORD(lParam);
	    wIndex = LBoxHitTest(lpLBoxInfo, pt);
	    if (wIndex == (WORD)-1)
		return (LRESULT)LB_ERR;
	    if (wIndex !=  lpLBoxInfo->wSelection) {
	        LBoxChangeSelection(lpLBoxInfo,wIndex,LIF_BYPOSITION);
	        if (!(lpLBoxInfo->wLBoxStyle & LBS_COMBOLBOX))
	            LBoxNotify(lpLBoxInfo, LBN_SELCHANGE, TRUE);
	    }
	    return (LRESULT)wIndex;

	case WM_LBUTTONDBLCLK:
	    SetFocus(hWnd);
	    pt.x = (int)(short)LOWORD(lParam);
	    pt.y = (int)(short)HIWORD(lParam);
	    wIndex = LBoxHitTest(lpLBoxInfo, pt);
	    if (wIndex == (WORD)-1)
		return (LRESULT)LB_ERR;
	    if (wIndex !=  lpLBoxInfo->wSelection) {
	        LBoxChangeSelection(lpLBoxInfo,wIndex,LIF_BYPOSITION);
	        if (!(lpLBoxInfo->wLBoxStyle & LBS_COMBOLBOX))
	            LBoxNotify(lpLBoxInfo, LBN_SELCHANGE, TRUE);
	    }
	    LBoxNotify(lpLBoxInfo,LBN_DBLCLK,TRUE);
	    return (LRESULT)wIndex;

	case WM_LBUTTONDOWN:
	    LBoxSetState(lpLBoxInfo, LSF_BUTTONDOWN);
	    if (!LBoxTestState(lpLBoxInfo, LSF_CAPTUREACTIVE)) {
		dwExStyle = GetWindowExStyle(hWnd);
		if (!(dwExStyle & WS_EX_NOCAPTURE))
		    hWndCapture = SetCapture(hWnd);
		LBoxSetState(lpLBoxInfo, LSF_CAPTUREACTIVE);
	    }
	    SetFocus(hWnd);
	    pt.x = (int)(short)LOWORD(lParam);
	    pt.y = (int)(short)HIWORD(lParam);
	    wIndex = LBoxHitTest(lpLBoxInfo, pt);
	    if (wIndex == (WORD)-1)
		return (LRESULT)LB_ERR;
	    if (wIndex !=  lpLBoxInfo->wSelection) {
	        LBoxChangeSelection(lpLBoxInfo,wIndex,LIF_BYPOSITION);
	        if (!(lpLBoxInfo->wLBoxStyle & LBS_COMBOLBOX))
	            LBoxNotify(lpLBoxInfo, LBN_SELCHANGE, TRUE);
	    }
	    return (LRESULT)wIndex;

	case WM_LBUTTONUP:
	    if (!LBoxTestState(lpLBoxInfo,LSF_BUTTONDOWN))
		return (LRESULT)-1;
	    LBoxClearState(lpLBoxInfo,LSF_BUTTONDOWN);
	    if (LBoxTestState(lpLBoxInfo, LSF_CAPTUREACTIVE)) {
		dwExStyle = GetWindowExStyle(hWnd);
		if (!(dwExStyle & WS_EX_NOCAPTURE)) {
		    if (!hWndCapture)
		        ReleaseCapture();
		    else {
		        SetCapture(hWndCapture);
		        hWndCapture = (HWND)0;
		    }
		}
		LBoxClearState(lpLBoxInfo,LSF_CAPTUREACTIVE);
	    }
	    pt.x = (int)(short)LOWORD(lParam);
            pt.y = (int)(short)HIWORD(lParam);
            wIndex = LBoxHitTest(lpLBoxInfo, pt);
	    if ((lpLBoxInfo->wLBoxStyle & LBS_COMBOLBOX) &&
		(wIndex != (WORD)-1)) {
		LBoxNotify(lpLBoxInfo,LBN_SELCHANGE,TRUE);
	    }
	    return (LRESULT)wIndex;

	case WM_VSCROLL:
	    nOffset = 0;
	    wIndex = lpLBoxInfo->wTopIndex;

	    switch(GET_WM_VSCROLL_CODE(wParam,lParam)) {
		case SB_LINEUP:
		    if (lpLBoxInfo->wTopIndex) 
			lpLBoxInfo->wTopIndex--;
		    break;

		case SB_PAGEUP:
		    if (lpLBoxInfo->wTopIndex) {
			lpLBoxInfo->wTopIndex =
				(lpLBoxInfo->wTopIndex >=
				 lpLBoxInfo->wFrameSize)?
				(lpLBoxInfo->wTopIndex -
				 lpLBoxInfo->wFrameSize):0;
		    }
		    break;
		case SB_LINEDOWN:
		    if ((WORD)(lpLBoxInfo->wTopIndex+lpLBoxInfo->wFrameSize+1)
				<= lpLBoxInfo->wItemCount) 
			lpLBoxInfo->wTopIndex++;
		    break;

		case SB_PAGEDOWN:
		    if ((WORD)(lpLBoxInfo->wTopIndex+lpLBoxInfo->wFrameSize+1)
			<= lpLBoxInfo->wItemCount)
			lpLBoxInfo->wTopIndex =
			    min((WORD)(lpLBoxInfo->wTopIndex +
					lpLBoxInfo->wFrameSize),
				(WORD)(lpLBoxInfo->wItemCount-
					lpLBoxInfo->wFrameSize));
		    break;
		case SB_THUMBTRACK:
		    if (GET_WM_VSCROLL_POS(wParam,lParam) !=
				GetScrollPos(lpLBoxInfo->hWnd,SB_VERT))
		        lpLBoxInfo->wTopIndex =
				GET_WM_VSCROLL_POS(wParam,lParam);
		    break;
	    }
	    if (wIndex != lpLBoxInfo->wTopIndex) {
		lpLBoxInfo->bPosChanged = TRUE;
		SetScrollPos(lpLBoxInfo->hWnd,SB_VERT,
			    		lpLBoxInfo->wTopIndex,TRUE);
		nOffset = 0;
		wStart = min(wIndex,lpLBoxInfo->wTopIndex);
		wEnd = max(wIndex,lpLBoxInfo->wTopIndex);
		if ((wEnd - wStart) == lpLBoxInfo->wFrameSize) 
		    InvalidateRgn(hWnd,(HRGN)0,TRUE);
		else {
		    for (w = wStart; w < wEnd; w++)
			nOffset += (int)LBoxGetItemHeight(lpLBoxInfo, w);
		    if (wIndex < lpLBoxInfo->wTopIndex)
			nOffset = -nOffset;
		    ScrollWindow(hWnd,0,nOffset,NULL,NULL);
		    UpdateWindow(hWnd);
		}
	    }
	    return 1L;

	case WM_HSCROLL:
	    if (!(lpLBoxInfo->wLBoxStyle & LBS_MULTICOLUMN)) {
		LBoxScrollHorz(lpLBoxInfo,
				(UINT)GET_WM_HSCROLL_CODE(wParam,lParam),
				GET_WM_HSCROLL_POS(wParam,lParam));
		return 1L;
	    }
	    switch(GET_WM_HSCROLL_CODE(wParam,lParam)) {
		case SB_LINEUP:
		case SB_PAGEUP:
		    if (lpLBoxInfo->wTopIndex) {
			if (GET_WM_HSCROLL_CODE(wParam,lParam) == SB_LINEUP) {
		/* Scroll to the beginning of previous column */
#ifdef LATER
			    for (wIndex=0, lpLBItem = lpLBInfo->lpListHead;
				    wIndex < lpLBoxInfo->wTopIndex;
				    lpLBItem = lpLBItem->lpNextItem, wIndex++);
			    
#else
			    lpLBoxInfo->wTopIndex--;
#endif
			}
			else
			    lpLBoxInfo->wTopIndex =
				(lpLBoxInfo->wTopIndex >=
				 lpLBoxInfo->wFrameSize)?
				(lpLBoxInfo->wTopIndex -
				 lpLBoxInfo->wFrameSize):0;
			SetScrollPos(lpLBoxInfo->hWnd,SB_HORZ,
					lpLBoxInfo->wTopIndex,TRUE);
			lpLBoxInfo->bPosChanged = TRUE;
			InvalidateRgn(hWnd,(HRGN)0,TRUE);
		    }
		    break;
		case SB_LINEDOWN:
		    if ((WORD)(lpLBoxInfo->wTopIndex + lpLBoxInfo->wFrameSize) >
					lpLBoxInfo->wItemCount) 
			break;
		/* Scroll to the beginning of next column */
		    for (wIndex=0, lpLBItem = lpLBoxInfo->lpListHead;
				wIndex < lpLBoxInfo->wTopIndex;
				lpLBItem = lpLBItem->lpNextItem, wIndex++);
		    for (wIndex=lpLBoxInfo->wTopIndex+1,
				lpLBItem = lpLBItem->lpNextItem;
				wIndex < lpLBoxInfo->wItemCount && 
				!(lpLBItem->pt.y == 0 || lpLBItem->pt.y == -1);
				lpLBItem = lpLBItem->lpNextItem, wIndex++);
		    if (wIndex < lpLBoxInfo->wItemCount)
			lpLBoxInfo->wTopIndex = wIndex;
		    else
			break;
		    SetScrollPos(lpLBoxInfo->hWnd,SB_HORZ,
					lpLBoxInfo->wTopIndex,TRUE);
		    lpLBoxInfo->bPosChanged = TRUE;
		    InvalidateRgn(hWnd,(HRGN)0,TRUE);
		    break;

		case SB_PAGEDOWN:
		    wIndex = lpLBoxInfo->wFrameSize;
		    if ((WORD)(lpLBoxInfo->wTopIndex+lpLBoxInfo->wFrameSize+1)
			<= lpLBoxInfo->wItemCount) {
			lpLBoxInfo->wTopIndex =
			    min((WORD)(lpLBoxInfo->wTopIndex+wIndex),
				(WORD)(lpLBoxInfo->wItemCount-
					lpLBoxInfo->wFrameSize));
			if (lpLBoxInfo->wLBoxStyle & LBS_MULTICOLUMN)
			    SetScrollPos(lpLBoxInfo->hWnd,SB_HORZ,
					lpLBoxInfo->wTopIndex,TRUE);
			else
			    SetScrollPos(lpLBoxInfo->hWnd,SB_VERT,
					lpLBoxInfo->wTopIndex,TRUE);
			lpLBoxInfo->bPosChanged = TRUE;
			InvalidateRgn(hWnd,(HRGN)0,TRUE);
		    }
		    break;
		case SB_THUMBTRACK:
		    if ((wIndex=GET_WM_HSCROLL_POS(wParam,lParam)) !=
			GetScrollPos(lpLBoxInfo->hWnd,SB_HORZ)) {
		        lpLBoxInfo->wTopIndex =
				GET_WM_HSCROLL_POS(wParam,lParam);
			if (lpLBoxInfo->wLBoxStyle & LBS_MULTICOLUMN)
			    SetScrollPos(lpLBoxInfo->hWnd,SB_HORZ,
					lpLBoxInfo->wTopIndex,TRUE);
			lpLBoxInfo->bPosChanged = TRUE;
			InvalidateRgn(hWnd,(HRGN)0,TRUE);
		    }
		    break;
	    }
	    return 1L;

	case WM_SETREDRAW:
	    if (wParam)
		lpLBoxInfo->wState |= LSF_REDRAW;
	    else
		lpLBoxInfo->wState &= ~LSF_REDRAW;
	    return 1L;

	case WM_ENABLE:		/* TODO */
	    return 1L;

	case WM_SIZE:
	    LBoxCalcFrame(lpLBoxInfo);
	    if (lpLBoxInfo->wHorzExtent)
		LBoxSetExtent(lpLBoxInfo,lpLBoxInfo->wHorzExtent);
	    lpLBoxInfo->bPosChanged = TRUE;
	    return 1L;

	case WM_MOUSEACTIVATE:
	    SetFocus(hWnd);
	    break;

	case WM_NCACTIVATE:
	    if (wParam)
		SetFocus(hWnd);
	    break;


	/* listbox specific messages */
	case LB_ADDSTRING:
	case LB_INSERTSTRING:
	    memset((char *)&mis, '\0', sizeof(MENUITEMSTRUCT));
	    mis.wLeftIndent = lpLBoxInfo->wLeftIndent;
	    mis.wRightIndent = lpLBoxInfo->wRightIndent;
	    mis.lpItemData = (LPSTR)lParam;
	    mis.wItemFlags = LIF_STRING | LIF_ENABLED | LIF_UNCHECKED;
	    if (wMsg == LB_INSERTSTRING)
		mis.wPosition = wParam;
	    rc = (LRESULT)LBoxAddItem(lpLBoxInfo,
			(wMsg == LB_ADDSTRING)?LAF_APPEND:LAF_INSERT,
			&mis);
	    return rc;

	/* this is an HSWIN private message */
	case LB_ADDITEM:
	    return (LRESULT)(LBoxAddItem(lpLBoxInfo, wParam,
			(LPMENUITEMSTRUCT)lParam) >= 0);

	case LB_DELETESTRING:
	    memset((char *)&mis, '\0', sizeof(MENUITEMSTRUCT));
	    mis.wItemFlags = LIF_BYPOSITION;
	    mis.wPosition = wParam;
	    rc = (LRESULT)LBoxDeleteItem(lpLBoxInfo, &mis);
	    return rc;

	case LB_DIR:
	    if ( lParam != 0  &&  strlen((LPSTR)lParam) != 0 )
	    	ListDir(hWnd,0,wParam,(LPSTR)lParam);
	    else
		return (LRESULT)LB_ERR;
	    return 1L;		/* TODO */

	case LB_GETCOUNT:
	    return (LRESULT)(lpLBoxInfo->wItemCount);

	case LB_GETCURSEL:
	    if (lpLBoxInfo->wSelection == (WORD)-1)
		return (LRESULT)LB_ERR;
	    return (LRESULT)lpLBoxInfo->wSelection;

	case LB_GETITEMHEIGHT:
	    return LBoxGetItemHeight(lpLBoxInfo, (WORD)wParam);

	case LB_SETITEMHEIGHT:
	    if (!(lpLBoxInfo->wLBoxStyle & LBS_OWNERDRAWVARIABLE)) {
		lpLBoxInfo->wItemHeight = LOWORD(lParam);
	    }
	    else {
		if (!(lpLBItem = LBoxItemFromIndex(lpLBoxInfo,wParam)))
		    return (LRESULT)LB_ERR;
		lpLBItem->wItemHeight = LOWORD(lParam);
	    }
	    GetClientRect(lpLBoxInfo->hWnd,&rcClient);
	    lpLBoxInfo->bPosChanged = TRUE;
	    return (LRESULT)0;

	case LB_GETITEMDATA:
	case LB_GETSEL:
	case LB_GETTEXT:
	case LB_GETTEXTLEN:
	case LB_SETITEMDATA:
	    rc = (LRESULT)LBoxItemData(lpLBoxInfo, wParam, lParam, wMsg);
	    return rc;

	/* this is an HSWIN private message */
	case LB_GETITEMINDENTS:
	/* this is an HSWIN private message */
	case LB_SETITEMINDENTS:
	/* this is an HSWIN private message */
	case LB_GETITEMBITMAPS:
	/* this is an HSWIN private message */
	case LB_SETITEMBITMAPS:
	case LB_GETITEMRECT:
	    memset((char *)&mis, '\0', sizeof(MENUITEMSTRUCT));
	    mis.wItemFlags = LIF_BYPOSITION;
	    mis.wPosition = wParam;
	    switch(wMsg) {
		case LB_GETITEMINDENTS:
		case LB_SETITEMINDENTS:
		    mis.wAction = LCA_INDENTS;
		    if (wMsg == LB_SETITEMINDENTS) {
			mis.wAction |= LCA_SET;
			mis.wLeftIndent = LOWORD(lParam);
			mis.wRightIndent = HIWORD(lParam);
		    }
		    break;
#ifdef LATER
		case LB_GETITEMBITMAPS:
		case LB_SETITEMBITMAPS:
		    mis.wAction = LCA_BITMAPS;
		    if (wMsg == LB_SETITEMBITMAPS) {
			mis.wAction |= LCA_SET;
			mis.hLeftBitmap = LOWORD(lParam);
			mis.hRightBitmap = HIWORD(lParam);
		    }
		    break;
#endif
		case LB_GETITEMRECT:
		    /* if the item is not visible (not in the frame) - fail */
		    if ((wParam < lpLBoxInfo->wTopIndex) ||
			wParam >= (WPARAM)
			  (lpLBoxInfo->wTopIndex+lpLBoxInfo->wFrameSize))
			return (LRESULT)LB_ERR;
		    mis.wAction = LCA_RECT;
		    mis.lpItemData = (LPSTR)lParam;
		    break;
	    }
	    rc = (LRESULT)LBoxModifyItem(lpLBoxInfo, &mis);
	    return rc;

	case LB_GETSELCOUNT:
	    if (BSINGLESEL(lpLBoxInfo))
		return (LRESULT)LB_ERR;
	    else
		return (LRESULT)((lpLBoxInfo->wSelection == (WORD)-1) ? 0 :
				lpLBoxInfo->wSelCount);

	case LB_GETSELITEMS:
	{
	    LPINT lpItems = (LPINT)lParam;
	    int i;

	    if (BSINGLESEL(lpLBoxInfo))
		return LB_ERR;

	    if (lpItems == NULL)
		return LB_ERR;

	    if (lpLBoxInfo->wSelection == (WORD)-1)
		return 0L;

	    if (lpLBoxInfo->wSelCount == 1) {
		*lpItems = (int)lpLBoxInfo->wSelection;
		return 1L;
	    }

	    for (lpLBItem = lpLBoxInfo->lpListHead, i = 0, nRet = 0;
		    lpLBItem && (nRet <= ( int )min(( WPARAM )lpLBoxInfo->wSelCount,wParam));
		    lpLBItem = lpLBItem->lpNextItem,i++) {
		if (lpLBItem->wItemFlags & LIF_HILITE)
		    lpItems[nRet++] = i;
	    }
	    return (LRESULT)nRet;
	}
	    break;

	case LB_GETCARETINDEX:	/* TODO */
	    break;

	case LB_SETCARETINDEX:	/* TODO */
	    break;

	case LB_GETTOPINDEX:
	    return (LRESULT)(lpLBoxInfo->wTopIndex);

	case LB_RESETCONTENT:
	    LBoxResetContent(lpLBoxInfo,TRUE);
	    return (LRESULT)LB_OKAY;

	case LB_SELECTSTRING:
	case LB_FINDSTRING:
	case LB_FINDSTRINGEXACT:
	    rc = (LRESULT)LBoxFindItem(lpLBoxInfo,wParam,(LPSTR)lParam,wMsg);
	    return rc;

	case LB_SELITEMRANGE:	/* TODO */
	    break;

	case LB_SETCOLUMNWIDTH:	
	    lpLBoxInfo->wColumnWidth = wParam;
	    lpLBoxInfo->bFrameChanged = TRUE;
	    lpLBoxInfo->bPosChanged = TRUE;
	    InvalidateRgn(hWnd,(HRGN)0,TRUE);
	    break;

	case LB_SETCURSEL:
	    if (!BSINGLESEL(lpLBoxInfo)
		|| ((wParam != (WPARAM)-1) &&
		    (wParam >= lpLBoxInfo->wItemCount)))
		return (LRESULT)LB_ERR;
	    LBoxChangeSelection(lpLBoxInfo,(WORD)wParam,LIF_BYPOSITION);
	    break;

	case LB_SETHORIZONTALEXTENT:
	    LBoxSetExtent(lpLBoxInfo,(WORD)wParam);
	    return 1L;

	case LB_GETHORIZONTALEXTENT:
	    if ((lpLBoxInfo->wState & LSF_SCROLLENABLED) && 
			lpLBoxInfo->wHorzExtent)
		    return (LRESULT)lpLBoxInfo->wHorzExtent;
	    else
		return 0L;

	case LB_SETSEL:
	    if (BSINGLESEL(lpLBoxInfo))
		return LB_ERR;		/* this might be wrong! */
	    return LBoxSetSelection(lpLBoxInfo, (BOOL)wParam,
					(int)LOSHORT(lParam));

	case LB_SETTABSTOPS:	/* TODO */
	    break;

	case LB_SETTOPINDEX:
	    if (lpLBoxInfo->wTopIndex != (WORD)wParam) {
		wIndex = 0;		/* calc max. scroll allowed */
		if ( lpLBoxInfo->wItemCount > lpLBoxInfo->wFrameSize )
		    wIndex = lpLBoxInfo->wItemCount - lpLBoxInfo->wFrameSize;
		if ( wParam > wIndex )
		    wParam = wIndex;
		lpLBoxInfo->wTopIndex = (WORD)wParam;
		lpLBoxInfo->bPosChanged = TRUE;
		InvalidateRgn(hWnd,(HRGN)0,FALSE);
	    }
	    return (LRESULT)LB_OKAY;

	/* this is an HSWIN private message */
	case LB_GETINDENTS:
	    return MAKELPARAM(lpLBoxInfo->wLeftIndent,
				lpLBoxInfo->wRightIndent);

	/* this is an HSWIN private message */
	case LB_SETINDENTS:
	    lpLBoxInfo->wLeftIndent = LOWORD(lParam);
	    lpLBoxInfo->wRightIndent = HIWORD(lParam);
	    if (LBoxTestState(lpLBoxInfo,LSF_REDRAW))
		InvalidateRgn(hWnd, (HRGN)0, FALSE);
	    return (LRESULT)LB_OKAY;

	case WM_CONVERT:
	    if (!lpLBoxBinToNat) {
		hLBoxClass32 = FindClass("LISTBOX",0);
		lpLBoxBinToNat = (WNDPROC)GetClassHandleLong(
				hLBoxClass32,GCL_BINTONAT);
	    }
	    if (lpLBoxBinToNat)
		return lpLBoxBinToNat(hWnd,wMsg,wParam,lParam);
	    else
	default:
	    break;
    }
    return DefWindowProc(hWnd, wMsg, wParam, lParam);
}

/* Supplemental routines */

static LONG
LBoxCreate(HWND hWnd, LPCREATESTRUCT lpCrS)
{
    HMENU hListBox;
    LPLISTBOXINFO lpLBInfo;
    DWORD dwStyle;

    if (lpCrS->style & LBS_PRELOADED) {
	lpLBInfo = (LPLISTBOXINFO)(lpCrS->lpCreateParams);
	hListBox = hWnd;
    }
    else {
	if (!(lpLBInfo = CREATELBOX(hListBox))) {
	    return 0L;
	}
    }
    
    SetWindowWord(hWnd, LWD_HMENU, (WORD)(lpLBInfo->ObjHead.hObj));
    SetWindowLong(hWnd, LWD_LPMENUDATA, (LONG)lpLBInfo);

    lpLBInfo->hWnd = hWnd;
    lpLBInfo->hWndParent = lpCrS->hwndParent;
    lpLBInfo->wLBoxStyle = (WORD)lpCrS->style;
    if (!BOWNERDRAW(lpLBInfo))
	lpLBInfo->wLBoxStyle |= LBS_HASSTRINGS;
    lpLBInfo->wState = (lpCrS->style & LBS_NOREDRAW)?0:LSF_REDRAW;
    if (lpCrS->style & (WS_VSCROLL|WS_HSCROLL)) {
	if (!(lpCrS->style & LBS_DISABLENOSCROLL)) {
	    dwStyle = GetWindowStyle(hWnd);
	    SetWindowStyle(hWnd,dwStyle & ~(WS_VSCROLL|WS_HSCROLL));
	}
	lpLBInfo->wState |= LSF_SCROLLENABLED;
    }
    if (lpCrS->style & LBS_OWNERDRAWVARIABLE)
	lpLBInfo->wLBoxStyle |= LBS_NOINTEGRALHEIGHT;
    lpLBInfo->wTopIndex = 0;
    lpLBInfo->wSelection = (WORD)-1;
    lpLBInfo->wColumnWidth = (lpLBInfo->wLBoxStyle & LBS_MULTICOLUMN)?
		lpCrS->cx/2:lpCrS->cx;
    lpLBInfo->wFrameSize = 0;
    lpLBInfo->wItemHeight = 0;
    lpLBInfo->bPosChanged = TRUE;
    return ((LONG)hListBox);
}

static WORD
LBoxHitTest(LPLISTBOXINFO lpLBInfo, POINT pt)
{
    WORD wIndex;
    RECT rcClient,rcItem;
    LPLISTBOXITEM lpLBItem;

    GetClientRect(lpLBInfo->hWnd, &rcClient);

    /* skip the items up to the current top */
    for (lpLBItem = lpLBInfo->lpListHead, wIndex = 0;
	 lpLBItem && wIndex < lpLBInfo->wTopIndex;
	 wIndex++,lpLBItem = lpLBItem->lpNextItem);

    for(; lpLBItem;
	 lpLBItem = lpLBItem->lpNextItem, wIndex++) {
	SetRect(&rcItem,
		lpLBItem->pt.x,lpLBItem->pt.y,
		lpLBItem->pt.x + lpLBItem->wItemWidth,
		lpLBItem->pt.y + lpLBItem->wItemHeight);
	if (PtInRect(&rcItem,pt))
	    break;
    }
    if (!lpLBItem)
	wIndex = (WORD)-1;
    return wIndex;
}

/* LBoxCalcFrame recalculates the number of visible items   */
/* re-adjusting window size if needed; having done any changes,   */
/* it sets fPosChanged flag on the listbox, meaning that item coords */
/* should be recalculated */

static void
LBoxCalcFrame(LPLISTBOXINFO lpLBInfo)
{
    MEASUREITEMSTRUCT mis;
    LPLISTBOXITEM lpLBItem;
    RECT rcWnd,rcClient;
    WORD wFrameSize;
    WORD wItemHeight;
    int nHeight, nNCHeight, nNumColumns;
    int nClientWidth,nClientHeight;
    TEXTMETRIC tm;
    HDC hDC;
    int fnScroll,nDiff;
    DWORD dwStyle;
    BOOL fSetRange = FALSE;
    static BOOL fResizePending = FALSE;

    wItemHeight = 0;

    GetClientRect(lpLBInfo->hWnd,&rcClient);
    GetWindowRect(lpLBInfo->hWnd, &rcWnd);
    if (IsRectEmpty(&rcClient) || IsRectEmpty(&rcWnd))
	return;
    lpLBInfo->bFrameChanged = FALSE;
    nClientWidth = rcClient.right;
    nClientHeight = rcClient.bottom;
    nNCHeight = (rcWnd.bottom - rcWnd.top) - nClientHeight;
    dwStyle = GetWindowStyle(lpLBInfo->hWnd);

    if (fResizePending)
	return;
    fResizePending = TRUE;

    if (!(lpLBInfo->wLBoxStyle & LBS_OWNERDRAWVARIABLE)) {
	if (BOWNERDRAW(lpLBInfo)) {
	    mis.CtlType = ODT_LISTBOX;
	    mis.CtlID = (lpLBInfo->hWnd)?
		GetWindowID(lpLBInfo->hWnd):lpLBInfo->hListBox;
	    mis.itemID = (UINT)-1;
            mis.itemData = 0;
	    mis.itemHeight = 0;
	    SendMessage(lpLBInfo->hWndParent,WM_MEASUREITEM,
			(WPARAM)mis.CtlID,(LONG)&mis);
	    wItemHeight = mis.itemHeight;
	}

	if(wItemHeight == 0) {
	    hDC = GetDC(lpLBInfo->hWnd);
	    if(hDC) { 
		if (lpLBInfo->hFont)
			SelectObject(hDC, lpLBInfo->hFont);
	    	GetTextMetrics(hDC, &tm);
	    	ReleaseDC(lpLBInfo->hWnd, hDC);
	    }
	    wItemHeight = tm.tmHeight+2;
	}

	if(wItemHeight == 0)
		return;
	if (lpLBInfo->wItemHeight != wItemHeight) {
	    lpLBInfo->bPosChanged = TRUE;
	    lpLBInfo->wItemHeight = wItemHeight;
	}

	wFrameSize = (WORD)nClientHeight/wItemHeight;

	if (!(lpLBInfo->wLBoxStyle & LBS_NOINTEGRALHEIGHT)) {
	    if (dwStyle & WS_HSCROLL) 
		nClientHeight += GetSystemMetrics(SM_CYHSCROLL);
	    nDiff = nClientHeight% ( int )wItemHeight;
	    if (nDiff) 
		SetWindowPos(lpLBInfo->hWnd,0,0,0,
		    rcWnd.right-rcWnd.left,rcWnd.bottom-rcWnd.top-nDiff,
		    SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
	}
    }
    else {
	/* owner should have already set heights for all the items by now */
	wFrameSize = 0;
	nHeight = 0;
	for (lpLBItem = LBoxItemFromIndex(lpLBInfo,lpLBInfo->wTopIndex);
	     lpLBItem;
	     lpLBItem = lpLBItem->lpNextItem) {
	    wFrameSize++;
	    nHeight += lpLBItem->wItemHeight;
	    if (nHeight > nClientHeight)
		break;
	}
    }

    if (lpLBInfo->wLBoxStyle & LBS_MULTICOLUMN) {
	if (lpLBInfo->wColumnWidth == 0)
	    lpLBInfo->wColumnWidth = nClientWidth/2;
		
	nNumColumns = nClientWidth / (int)lpLBInfo->wColumnWidth;
	if ((nClientWidth % (int)lpLBInfo->wColumnWidth) > 3)
	    nNumColumns++;
	wFrameSize *= nNumColumns;
    }
    else {
	lpLBInfo->wColumnWidth = nClientWidth;
	if (!(lpLBInfo->wLBoxStyle & LBS_OWNERDRAWVARIABLE) &&
		(lpLBInfo->wLBoxStyle & LBS_NOINTEGRALHEIGHT)) {
	    if ((nClientHeight % (int)wItemHeight) > 5)
		wFrameSize++;
	}
    }

    fnScroll = 0;

    if (lpLBInfo->wState & LSF_SCROLLENABLED) {
	if (lpLBInfo->wLBoxStyle & LBS_DISABLENOSCROLL) {
	/* do not add scrollbars if they are not already there	*/
	/* enable/disable them as needed			*/
	    if (lpLBInfo->wLBoxStyle & LBS_MULTICOLUMN) {
		if (dwStyle & WS_HSCROLL) {
		    if (lpLBInfo->wItemCount > wFrameSize) {
			if (lpLBInfo->wState & LSF_HSCROLLDISABLED) {
			    EnableScrollBar(lpLBInfo->hWnd,SB_HORZ,
					ESB_ENABLE_BOTH);
			    LBoxClearState(lpLBInfo,LSF_HSCROLLDISABLED);
			}
			fSetRange = TRUE;
			fnScroll = SB_HORZ;
		    }
		    else if (!(lpLBInfo->wState & LSF_HSCROLLDISABLED)) {
			EnableScrollBar(lpLBInfo->hWnd,SB_HORZ,
						ESB_DISABLE_BOTH);
			LBoxSetState(lpLBInfo,LSF_HSCROLLDISABLED);
		    }
		}
	    }
	    else {	/* not LBS_MULTICOLUMN */
		if (dwStyle & WS_VSCROLL) {
		    if (lpLBInfo->wItemCount > wFrameSize
			|| lpLBInfo->wTopIndex > 0) {
			if (lpLBInfo->wState & LSF_VSCROLLDISABLED) {
			    EnableScrollBar(lpLBInfo->hWnd,SB_VERT,
					ESB_ENABLE_BOTH);
			    LBoxClearState(lpLBInfo,LSF_VSCROLLDISABLED);
			}
			fSetRange = TRUE;
			fnScroll = SB_VERT;
		    }
		    else if (!(lpLBInfo->wState & LSF_VSCROLLDISABLED)) {
			EnableScrollBar(lpLBInfo->hWnd,SB_VERT,
						ESB_DISABLE_BOTH);
			LBoxSetState(lpLBInfo,LSF_VSCROLLDISABLED);
		    }
		}
	    }
	}
	else {	/* not LBS_DISABLENOSCROLL */
	/* show the scrollbar that is needed */
	    if (lpLBInfo->wLBoxStyle & LBS_MULTICOLUMN)
		fnScroll = SB_HORZ;
	    else
		fnScroll = SB_VERT;
	    if (lpLBInfo->wItemCount > wFrameSize) 
		fSetRange = TRUE;
	}
	if (fSetRange)
	    SetScrollRange(lpLBInfo->hWnd,fnScroll,0,lpLBInfo->wItemCount - 
			wFrameSize,lpLBInfo->wState & LSF_REDRAW);
    }

    lpLBInfo->wFrameSize = wFrameSize;
    fResizePending = FALSE;
}

static void
LBoxRecalcPositions(LPLISTBOXINFO lpLBInfo)
{
    LPLISTBOXITEM lpLBItem,lpPrevLBItem = 0;
    WORD wIndex,X,Y;
    RECT rcClient;

    if (!lpLBInfo->hWnd)
	return;
    if (lpLBInfo->bFrameChanged)
	LBoxCalcFrame(lpLBInfo);
    if (lpLBInfo->wLBoxStyle & LBS_OWNERDRAWVARIABLE)
	lpLBInfo->wFrameSize = lpLBInfo->wItemCount;
    GetClientRect(lpLBInfo->hWnd,&rcClient);
    for (wIndex=0,lpLBItem = lpLBInfo->lpListHead;
		wIndex < lpLBInfo->wTopIndex && lpLBItem;
		wIndex++, lpLBItem = lpLBItem->lpNextItem) 
	lpLBItem->pt.x = lpLBItem->pt.y = (int)-1;
    if (!lpLBItem)
	return;
    X = Y = 0;
    for (wIndex = 0; wIndex < lpLBInfo->wFrameSize && lpLBItem;
		wIndex++, lpLBItem = lpLBItem->lpNextItem) {
	if (!lpLBItem->wItemHeight || !lpLBItem->wItemWidth ||
		(lpLBItem->wItemWidth != lpLBInfo->wColumnWidth))
	    LBoxCalcItemSize(lpLBInfo,lpLBItem,wIndex+lpLBInfo->wTopIndex);
	if (lpLBItem->wItemFlags & (LIF_MENUBREAK | LIF_MENUBARBREAK)) {
	    X += lpPrevLBItem->wItemWidth;
	    if (lpLBItem->wItemFlags & LIF_MENUBARBREAK)
		X++;
	    Y = 0;
	}
	else if ((int)(Y + lpLBItem->wItemHeight) > rcClient.bottom) {
	    if (lpLBInfo->wLBoxStyle & LBS_MULTICOLUMN) {
		if ((int)(X+lpLBInfo->wColumnWidth) >= rcClient.right) {
		    lpLBInfo->wFrameSize = wIndex;
		    break;
		}
		else {
		    Y = 0;
		    X += lpLBInfo->wColumnWidth;
		}
	    }
	    else if (!(lpLBInfo->wLBoxStyle & LBS_NOINTEGRALHEIGHT)) {
		lpLBInfo->wFrameSize = wIndex;
		break;
	    }
	}
	lpLBItem->pt.x = X;
	lpLBItem->pt.y = Y;
	Y += lpLBItem->wItemHeight;
	lpPrevLBItem = lpLBItem;
    }
    if (lpLBItem)
	while ((lpLBItem = lpLBItem->lpNextItem))
	    lpLBItem->pt.x = lpLBItem->pt.y = (int)-1;
    lpLBInfo->bPosChanged = FALSE;
}

static void
LBoxCalcItemSize(LPLISTBOXINFO lpLBInfo, LPLISTBOXITEM lpLBItem, WORD wIndex)
{
    DWORD dwItemDim;

    if (lpLBInfo->wLBoxStyle & LBS_OWNERDRAWVARIABLE) {
	if (lpLBInfo->hWnd == 0)
	    return;
	dwItemDim = LBoxMeasureItem(lpLBInfo, lpLBItem, wIndex);
	lpLBItem->wItemHeight = HIWORD(dwItemDim);
	lpLBItem->wItemWidth = LOWORD(dwItemDim);
    }
    else {
	lpLBItem->wItemHeight = lpLBInfo->wItemHeight;
	lpLBItem->wItemWidth = lpLBInfo->wColumnWidth;
    }
}

void
LBoxDefaultMeasureItem(HWND hWnd, LPMEASUREITEMSTRUCT lpMis)
{
    LPLISTBOXINFO lpLBInfo;
    LPLISTBOXITEM lpLBItem;
    HDC hDC;
    TEXTMETRIC tm;

    lpLBInfo = (LPLISTBOXINFO)GetWindowLong(hWnd,LWD_LPMENUDATA);

    if (!lpLBInfo || (lpLBInfo == (LPLISTBOXINFO)-1))
	return;

    if (lpMis->itemID == (UINT)-1) {
	hDC = GetDC(lpLBInfo->hWnd);
	if(hDC) {
		if (lpLBInfo->hFont)
		    SelectObject(hDC, lpLBInfo->hFont);
		GetTextMetrics(hDC, &tm);
		lpMis->itemHeight = (tm.tmHeight+2);
		lpMis->itemWidth = 0;
		ReleaseDC(lpLBInfo->hWnd, hDC);
	}
	return;
    }

    if (!(lpLBItem = LBoxItemFromIndex(lpLBInfo,lpMis->itemID)))
	return;

    if ((lpLBInfo->wLBoxStyle & LBS_OWNERDRAWMASK) &&
	(!(lpLBInfo->wLBoxStyle & LBS_HASSTRINGS) || (lpLBItem->dwItemData)))
	return;

    LBoxDoMeasure(lpLBInfo, lpLBItem, lpMis);
}

static DWORD
LBoxDoMeasure(LPLISTBOXINFO lpLBInfo, LPLISTBOXITEM lpLBItem,
		LPMEASUREITEMSTRUCT lpMis)
{
    HDC hDC;
    TEXTMETRIC tm;
    DWORD dwSize;

    hDC = GetDC(lpLBInfo->hWnd);
    if (lpLBInfo->hFont)
        SelectObject(hDC, lpLBInfo->hFont);
    GetTextMetrics(hDC, &tm);
    lpMis->itemHeight = (tm.tmHeight+2);
    dwSize = 0;
    if(lpMis->itemData)
	dwSize   = GetTextExtent(hDC,(LPSTR)lpMis->itemData,
			strlen((LPSTR)lpMis->itemData));
    ReleaseDC(lpLBInfo->hWnd, hDC);
    lpMis->itemWidth = LOWORD(dwSize) + lpLBItem->wItemLeftIndent+
		lpLBItem->wItemRightIndent+10;
    return MAKELONG(lpMis->itemWidth,lpMis->itemHeight);
}

static DWORD
LBoxMeasureItem(LPLISTBOXINFO lpLBInfo, LPLISTBOXITEM lpLBItem, WORD wIndex)
{
    MEASUREITEMSTRUCT mis;

    memset((LPSTR)&mis,'\0',sizeof(MEASUREITEMSTRUCT));
    mis.itemData = 
	((lpLBInfo->wLBoxStyle & LBS_OWNERDRAWMASK) && 
        (!(lpLBInfo->wLBoxStyle & LBS_HASSTRINGS) || (lpLBItem->dwItemData)))?
	lpLBItem->dwItemData:(DWORD)(lpLBItem->lpItemString);
    if (BOWNERDRAW(lpLBInfo)) {
	mis.CtlType = ODT_LISTBOX;
	mis.CtlID = (lpLBInfo->hWnd)?
		GetWindowID(lpLBInfo->hWnd):lpLBInfo->hListBox;
	mis.itemID = wIndex;
	SendMessage(lpLBInfo->hWndParent,WM_MEASUREITEM,
		(WPARAM)mis.CtlID,(DWORD)&mis);
	if (mis.itemWidth == 0)
	    mis.itemWidth = lpLBInfo->wColumnWidth;
	if (mis.itemHeight == 0)
	    mis.itemHeight = lpLBItem->wItemHeight;
	return MAKELONG(mis.itemWidth,mis.itemHeight);
    }
    else
	return LBoxDoMeasure(lpLBInfo,lpLBItem,&mis);
}

static void
LBoxSendItemDraw(LPLISTBOXINFO lpLBInfo, UINT uiIndex,
			UINT itemAction, UINT itemState)
{
    DRAWITEMSTRUCT dis;
    HDC hDC;
    RECT rcClient,rcClip;
    LPLISTBOXITEM lpLBItem ;
    BOOL bOwnerDraw,bOwnData;
    HRGN hRgn;
    DWORD dwStyle;

    dwStyle = lpLBInfo->hWnd?
	GetWindowStyle(lpLBInfo->hWnd):0L;
    if (!(dwStyle & WS_VISIBLE))
	return;
    if (!(lpLBItem = LBoxItemFromIndex(lpLBInfo, uiIndex)))
	return;
    if ((lpLBItem->pt.x == (int)-1) && (lpLBItem->pt.y == (int)-1))
	return;
    hDC = GetDC(lpLBInfo->hWnd);
    if (lpLBInfo->wHorzOffset)
	SetWindowOrg(hDC,lpLBInfo->wHorzOffset,0);
    GetClientRect(lpLBInfo->hWnd, &rcClient);
    dis.CtlType = ODT_LISTBOX;
    dis.CtlID = (lpLBInfo->hWnd)?
		GetWindowID(lpLBInfo->hWnd):lpLBInfo->hListBox;
    dis.itemID = (UINT)((int)((short)LOWORD(uiIndex)));
    dis.itemAction = itemAction;
    dis.itemState  = itemState;
    dis.hwndItem = lpLBInfo->hWnd;
    dis.hDC = hDC;
    if (uiIndex != (WORD)-1) {
	SetRect(&dis.rcItem,
		lpLBItem->pt.x,lpLBItem->pt.y,
		lpLBItem->pt.x + lpLBItem->wItemWidth + lpLBInfo->wHorzOffset,
		lpLBItem->pt.y + lpLBItem->wItemHeight);
	bOwnData = BOWNDATA(lpLBInfo) ||
	    ((lpLBItem->wItemFlags & LIF_OWNERDRAW)?TRUE:FALSE);
	dis.itemData = (bOwnData)?
		(DWORD)(lpLBItem->dwItemData):
		(DWORD)(lpLBItem->lpItemString);
    }
    else {
	CopyRect(&dis.rcItem, &rcClient);
	dis.itemData = 0L;
    }
    CopyRect(&rcClip,&dis.rcItem);
    if (lpLBInfo->wHorzOffset)
	OffsetRect(&rcClip,lpLBInfo->wHorzOffset,0);
    IntersectRect(&rcClip,&rcClip,&rcClient);

    hRgn = CreateRectRgnIndirect(&rcClip);
    SelectClipRgn(dis.hDC,hRgn);
    if (lpLBInfo->hFont)
	SelectObject(dis.hDC,lpLBInfo->hFont);
    bOwnerDraw = (BOWNERDRAW(lpLBInfo))?TRUE:FALSE;
    if (bOwnerDraw)
	SendMessage(lpLBInfo->hWndParent,WM_DRAWITEM,
			(WPARAM)dis.CtlID,(LONG)&dis);
    else
	LBoxDrawItem(lpLBInfo, &dis);
    DeleteObject(hRgn);
    ReleaseDC(lpLBInfo->hWnd, hDC);
}

static void
LBoxPaint(LPLISTBOXINFO lpLBInfo)
{
    WORD wIndex;
    BOOL bOwnData;
    LPLISTBOXITEM lpLBItem;
    PAINTSTRUCT ps;
    DRAWITEMSTRUCT dis;
    HRGN hRgn;
    RECT rcClip,rcTemp;
    HPEN hPenOld;

    if ((lpLBInfo->wState & LSF_SCROLLENABLED) &&
			!(lpLBInfo->wLBoxStyle & LBS_DISABLENOSCROLL))
	 ShowScrollBar(lpLBInfo->hWnd,
		(lpLBInfo->wLBoxStyle & LBS_MULTICOLUMN)?SB_HORZ:SB_VERT,
		((lpLBInfo->wTopIndex <= 0  &&
		  lpLBInfo->wItemCount <= lpLBInfo->wFrameSize) ?
	       FALSE:TRUE));

    if (lpLBInfo->bPosChanged)
	LBoxRecalcPositions(lpLBInfo);
     
    /* skip items up to the top item */
    for (lpLBItem = lpLBInfo->lpListHead, wIndex = 0;
	 lpLBItem && wIndex < lpLBInfo->wTopIndex;
	 wIndex++,lpLBItem = lpLBItem->lpNextItem);

    BeginPaint(lpLBInfo->hWnd,&ps);
    if (lpLBInfo->wHorzOffset) {
	SetWindowOrg(ps.hdc,lpLBInfo->wHorzOffset,0);
	OffsetRect(&ps.rcPaint,lpLBInfo->wHorzOffset,0);
    }
    CopyRect(&rcClip,&ps.rcPaint);
    hRgn = CreateRectRgn(0,0,0,0);

    dis.CtlType = ODT_LISTBOX;
    dis.CtlID = (lpLBInfo->hWnd)?
		GetWindowID(lpLBInfo->hWnd):lpLBInfo->hListBox;
    dis.hwndItem = lpLBInfo->hWnd;
    dis.hDC = ps.hdc;

    if (lpLBInfo->hFont)
	SelectObject(dis.hDC,lpLBInfo->hFont);

    for (wIndex = lpLBInfo->wTopIndex;
	 lpLBItem &&
	  (wIndex < (WORD)(lpLBInfo->wTopIndex+lpLBInfo->wFrameSize));
	 wIndex++,lpLBItem = lpLBItem->lpNextItem) {
	if (lpLBItem->wItemFlags & LIF_MENUBARBREAK) {
	    SelectClipRgn(ps.hdc,(HRGN)0);
	    GetClientRect(lpLBInfo->hWnd,&rcTemp);
	    hPenOld = SelectObject(dis.hDC,GetStockObject(BLACK_PEN));
	    MoveTo(dis.hDC,lpLBItem->pt.x-1,0);
	    LineTo(dis.hDC,lpLBItem->pt.x-1,rcTemp.bottom);
	    SelectObject(dis.hDC,hPenOld);
	}
	SetRect(&dis.rcItem,
		lpLBItem->pt.x,lpLBItem->pt.y,
		lpLBItem->pt.x + lpLBItem->wItemWidth + lpLBInfo->wHorzOffset,
		lpLBItem->pt.y + lpLBItem->wItemHeight);
	if (IntersectRect(&rcTemp,&rcClip,&dis.rcItem)) {
	    SetRectRgn(hRgn,rcTemp.left-lpLBInfo->wHorzOffset,rcTemp.top,
			    rcTemp.right-lpLBInfo->wHorzOffset,rcTemp.bottom);
	    SelectClipRgn(dis.hDC,hRgn);
	    dis.itemID = (UINT)((int)((short)wIndex));
	    bOwnData = BOWNDATA(lpLBInfo) ||
		((lpLBItem->wItemFlags & LIF_OWNERDRAW)?TRUE:FALSE);
	    dis.itemData = (bOwnData)?
	    			(DWORD)(lpLBItem->dwItemData):
	    			(DWORD)(lpLBItem->lpItemString);
	    dis.itemState = (lpLBItem->wItemFlags & LIF_HILITE)?ODS_SELECTED:0;
	    dis.itemAction = ODA_DRAWENTIRE;
	    if (BOWNERDRAW(lpLBInfo))
	        SendMessage(lpLBInfo->hWndParent,WM_DRAWITEM,
			(WPARAM)dis.CtlID,(DWORD)&dis);
	    else
	        LBoxDrawItem(lpLBInfo,&dis);
	}
    }
    SelectClipRgn(ps.hdc,(HRGN)0);
    DeleteObject(hRgn);
    EndPaint(lpLBInfo->hWnd,&ps);
}

static void
LBoxEraseBackground(LPLISTBOXINFO lpLBInfo, HDC hDC)
{
    RECT rcClient;
    HBRUSH hBr;
    HWND hWndParent;
    RECT rcParent;
    POINT ptParentOrg;

    GetClientRect(lpLBInfo->hWnd, &rcClient);
    hBr = GetControlBrush(lpLBInfo->hWnd,hDC,CTLCOLOR_LISTBOX);
    if ((hWndParent = GetParent(lpLBInfo->hWnd))) {
	GetWindowRect(hWndParent,&rcParent);
	ptParentOrg.x = rcParent.left;
	ptParentOrg.y = rcParent.top;
	ScreenToClient(lpLBInfo->hWnd,&ptParentOrg);
	SetBrushOrg(hDC,ptParentOrg.x,ptParentOrg.y);
	UnrealizeObject(hBr);
    }
    FillRect(hDC,&rcClient,hBr);
}

void
LBoxDefaultDrawItem(LPDRAWITEMSTRUCT lpDis)
{
    LPLISTBOXINFO lpLBInfo;

    lpLBInfo = (LPLISTBOXINFO)GetWindowLong(
				lpDis->hwndItem, LWD_LPMENUDATA);

    if (!lpLBInfo || (lpLBInfo == (LPLISTBOXINFO)-1))
	return;

    if ((lpLBInfo->wLBoxStyle & LBS_OWNERDRAWMASK) && 
        !(lpLBInfo->wLBoxStyle & LBS_HASSTRINGS))
	return;

    LBoxDrawItem(lpLBInfo,lpDis);
}

static void
LBoxDrawItem(LPLISTBOXINFO lpLBInfo, LPDRAWITEMSTRUCT lpDis)
{
    HBRUSH hBr;
    HPEN hPen;
    RECT rcText, rcInverse;

    if (lpDis->itemID == (UINT)-1)
	return;
    SetBkMode(lpDis->hDC, TRANSPARENT);

    if (!(lpDis->itemAction & (ODA_DRAWENTIRE | ODA_SELECT | ODA_FOCUS)))
	return;

    SetRect(&rcText,lpDis->rcItem.left + 3,
		    lpDis->rcItem.top + 1,
		    lpDis->rcItem.right - 3,
		    lpDis->rcItem.bottom - 1);
    rcInverse = lpDis->rcItem;

    if (lpLBInfo->wHorzExtent) {
	int nMin,nMax,nPos;

	GetScrollRange(lpLBInfo->hWnd,SB_HORZ,&nMin,&nMax);
	if (nMin || nMax) {
	    nPos = GetScrollPos(lpLBInfo->hWnd,SB_HORZ);
	    if (nPos > nMin) {}
	    if (nPos < nMax) {
		rcText.right = lpDis->rcItem.right;
		rcInverse.right = lpDis->rcItem.right;
	    }
	}
    }

    if (lpDis->itemAction & (ODA_DRAWENTIRE|ODA_SELECT)) {
	if (lpDis->itemState & ODS_SELECTED) {
	    hBr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
	    FillRect(lpDis->hDC, &rcInverse, hBr);
	    DeleteObject(hBr);
	    SetTextColor(lpDis->hDC,GetSysColor(COLOR_HIGHLIGHTTEXT));
	    hPen = GetStockObject(WHITE_PEN);
	}
	else {
	    hBr = GetControlBrush(lpLBInfo->hWnd,lpDis->hDC,CTLCOLOR_LISTBOX);
	    FillRect(lpDis->hDC, &rcInverse, hBr);
	    SetTextColor(lpDis->hDC,GetSysColor(COLOR_WINDOWTEXT));
	}
	DrawText(lpDis->hDC,
		    (LPSTR)(lpDis->itemData),
		    strlen((LPSTR)(lpDis->itemData)),
		    &rcText,
		    DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS);
    }
    if ((lpDis->itemState & ODS_FOCUS) || (lpDis->itemAction & ODA_FOCUS))
	DrawFocusRect(lpDis->hDC,&rcInverse);
}

static BOOL
LBoxGetSlot(LPLISTBOXINFO lpLBInfo)
{

    if (!lpLBInfo->lpItemList) {
	if (!(lpLBInfo->lpItemList = (LPLISTBOXITEM *)
		WinMalloc(sizeof(LPLISTBOXITEM)*LPD_SLOTG))) {
	    return FALSE;
	}
	memset((LPVOID)lpLBInfo->lpItemList, '\0',
		sizeof(LPLISTBOXITEM)*LPD_SLOTG);
	lpLBInfo->wSlotCount = LPD_SLOTG;
    }

    if (lpLBInfo->wSlotCount == lpLBInfo->wItemCount) {
	if (!(lpLBInfo->lpItemList = (LPLISTBOXITEM *)
		WinRealloc((LPSTR)lpLBInfo->lpItemList,
		sizeof(LPLISTBOXITEM)*(lpLBInfo->wSlotCount+LPD_SLOTG)))) {
	    return FALSE;
	}
	memset((LPVOID)&lpLBInfo->lpItemList[lpLBInfo->wSlotCount], '\0',
		sizeof(LPLISTBOXITEM)*LPD_SLOTG);
	lpLBInfo->wSlotCount += LPD_SLOTG;
    }
    return TRUE;
}

static void
LBoxNotify(LPLISTBOXINFO lpLBInfo, int nMessage, BOOL bCheck)
{
    if (bCheck &&
	(!(lpLBInfo->wLBoxStyle & LBS_NOTIFY) ||
	 !(lpLBInfo->hWndParent)))
	return;
    SendMessage(lpLBInfo->hWndParent, WM_COMMAND,
	GET_WM_COMMAND_MPS(
		(lpLBInfo->hWnd)?
		    GetWindowID(lpLBInfo->hWnd):lpLBInfo->hListBox,
		lpLBInfo->hWnd,nMessage));
}

static LPLISTBOXITEM
LBoxItemFromIndex(LPLISTBOXINFO lpLBInfo, WORD wIndex)
{
    LPLISTBOXITEM lpCurrent;
    int nCount;

    if (wIndex >= lpLBInfo->wItemCount)
	return (LPLISTBOXITEM)0;
    if (!wIndex)
	return lpLBInfo->lpListHead;
    if (wIndex == (lpLBInfo->wItemCount-1))
	return lpLBInfo->lpListTail;
    for (nCount = 0, lpCurrent = lpLBInfo->lpListHead;
	 nCount < (int)wIndex;
	 nCount++, lpCurrent = lpCurrent->lpNextItem);
    return lpCurrent;
}

static LPLISTBOXITEM
LBoxItemFromCommand(LPLISTBOXINFO lpLBInfo, LPWORD lpwCommand)
{
    LPLISTBOXITEM lpCurrent;
    WORD wIndex;

    for (lpCurrent = lpLBInfo->lpListHead, wIndex = 0;
	 lpCurrent;
	 lpCurrent = lpCurrent->lpNextItem, wIndex++) {
	if (lpCurrent->hPopupMenu == (HMENU)(*lpwCommand)) {
	    *lpwCommand = wIndex;
	    return lpCurrent;
	}
    }
    return (LPLISTBOXITEM)0;
}

static void
LBoxChangeSelection(LPLISTBOXINFO lpLBInfo, WORD wIndex, WORD wFlags)
{
    LPLISTBOXITEM lpLBItem = (LPLISTBOXITEM)0;
    LPLISTBOXITEM lpOldSelection;
    WORD wOldSelection;
    int fnScroll;
    BOOL bKillSel = TRUE;
    int i;


    if (wIndex >= lpLBInfo->wItemCount)
	return;

    if (wIndex != lpLBInfo->wSelection) {
	wOldSelection = lpLBInfo->wSelection;
	lpLBInfo->wSelection = wIndex;
	if (wOldSelection != (WORD)-1) {
	    lpOldSelection = LBoxItemFromIndex(lpLBInfo, wOldSelection);

	    if (!BSINGLESEL(lpLBInfo)) {
		if ((lpLBInfo->wLBoxStyle & LBS_EXTENDEDSEL) &&
			GetKeyState(VK_SHIFT) && (wIndex != (WORD)-1)) {
		    bKillSel = FALSE;
		    lpLBInfo->wSelCount++;
		}
		else {
#ifdef LATER
	/* LBS_MULTIPLESEL style -- treat differently */
#endif
		}
	    }
	    if (bKillSel) {
		/* draw the old ones unselected */
		if ((lpLBInfo->wLBoxStyle & LBS_EXTENDEDSEL) &&
				(lpLBInfo->wSelCount > 1)) {
		    for (lpLBItem = lpLBInfo->lpListHead,i = 0;
				lpLBItem && lpLBInfo->wSelCount;
				lpLBItem = lpLBItem->lpNextItem,i++) {
			if (lpLBItem->wItemFlags & LIF_HILITE) {
			    LBoxItemSelection(lpLBInfo, lpLBItem, (WORD)i,
					LIF_BYPOSITION, FALSE);
			    lpLBInfo->wSelCount--;
			}
		    }
		}
		else 
		    LBoxItemSelection(lpLBInfo, lpOldSelection,
					wOldSelection, LIF_BYPOSITION, FALSE);
		lpLBInfo->wSelCount = 0;
	    }
	    else if (LBoxTestState(lpLBInfo,LSF_FOCUS))
		/* kill focus rect on the old selection */
		LBoxSendItemDraw(lpLBInfo,(UINT)wOldSelection,
					ODA_FOCUS,0);
	}

	if (wIndex != (WORD)-1) {
	    if (wFlags & LIF_BYPOSITION) {
		if (!(lpLBItem = LBoxItemFromIndex(lpLBInfo, wIndex)))
	            return;
            }
            else {
		if (!(lpLBItem = LBoxItemFromCommand(lpLBInfo, &wIndex)))
	            return;
            }
	    LBoxItemSelection(lpLBInfo, lpLBItem, lpLBInfo->wSelection,
			LIF_BYPOSITION|LIF_HILITE, TRUE);
	    lpLBItem->wItemFlags |= LIF_HILITE;
	    lpLBInfo->wSelCount++;

	    /* if new selection is off screen, scroll to show it */
	    if ((lpLBInfo->wItemCount > lpLBInfo->wFrameSize) && 
		    ((wIndex < lpLBInfo->wTopIndex) ||
		    ((int)wIndex >=
			((int)(lpLBInfo->wTopIndex + lpLBInfo->wFrameSize))))) {
		if ((int)(lpLBInfo->wItemCount - wIndex) <
			(int)lpLBInfo->wFrameSize)
		    lpLBInfo->wTopIndex = lpLBInfo->wItemCount -
						lpLBInfo->wFrameSize + 1;
#if 0 /* No need to set top index if selection is in viewable range */
		else
		    lpLBInfo->wTopIndex = wIndex;
#endif  /*  ECW  4/18/96 */
		if (lpLBInfo->wState & LSF_SCROLLENABLED) {
		    if (lpLBInfo->wLBoxStyle & LBS_MULTICOLUMN)
			fnScroll = SB_HORZ;
		    else
			fnScroll = SB_VERT;
		    SetScrollPos(lpLBInfo->hWnd,fnScroll,
			lpLBInfo->wTopIndex, TRUE);
		}
		lpLBInfo->bPosChanged = TRUE;
		InvalidateRgn(lpLBInfo->hWnd,(HRGN)0,TRUE);
		return;
	    }
	}
    }
}

static void
LBoxItemSelection(LPLISTBOXINFO lpLBInfo, LPLISTBOXITEM lpLBItem,
		  WORD wIndex, WORD wFlags, BOOL bFocus)
{
    WORD wAction;
    WORD wItemFlags;

    if (lpLBItem) {
	    if (wFlags & LIF_HILITE)
		lpLBItem->wItemFlags |= LIF_HILITE;
	    else
		lpLBItem->wItemFlags &= ~LIF_HILITE;

	    wAction = ODA_SELECT;
	    wItemFlags = (wFlags & LIF_HILITE)?ODS_SELECTED:0;
	    if (bFocus  && LBoxTestState(lpLBInfo,LSF_FOCUS)) {
		wAction |= ODA_FOCUS;
		wItemFlags |= ODS_FOCUS;
	    }
    } else {
	wAction = ODA_SELECT;
	wItemFlags = 0;
    }

    LBoxSendItemDraw(lpLBInfo, (UINT)wIndex, (UINT)wAction, (UINT)wItemFlags);
}

static LRESULT
LBoxSetSelection(LPLISTBOXINFO lpLBInfo, BOOL fSelect, int nIndex)
{
    LPLISTBOXITEM lpLBItem;
    WORD wFlags = LIF_BYPOSITION;
    int i;
    
    if (fSelect)
	wFlags = LIF_HILITE;

    if (nIndex != -1) { /* select/deselect a particular item */
	lpLBItem = LBoxItemFromIndex(lpLBInfo, (WORD)nIndex);
	if (!lpLBItem)
	    return LB_ERR;
	LBoxItemSelection(lpLBInfo, lpLBItem, (WORD)nIndex, wFlags, FALSE);
    }
    else {	/* select/deselect all item */
	for (lpLBItem = lpLBInfo->lpListHead,i = 0;
		lpLBItem;
		lpLBItem = lpLBItem->lpNextItem,i++) 
	    LBoxItemSelection(lpLBInfo, lpLBItem, (WORD)i, wFlags, FALSE);
    }
    return 0L;
}

static void
LBoxResetContent(LPLISTBOXINFO lpLBInfo, BOOL bDeleteData)
{
    LPLISTBOXITEM lpLBItem, lpNextItem;
    DELETEITEMSTRUCT dis;
    WORD wIndex;
    BOOL bOwnData;
    int fnScroll;
    DWORD dwStyle;

    if (lpLBInfo->wItemCount == 0)
	return;
    for (lpLBItem = lpLBInfo->lpListHead,wIndex = 0;
	 lpLBItem;
	 lpLBItem = lpNextItem, wIndex++) {
      lpNextItem = lpLBItem->lpNextItem;
      if (bDeleteData) {
	bOwnData = BOWNDATA(lpLBInfo) ||
	    ((lpLBItem->wItemFlags & LIF_OWNERDRAW)?TRUE:FALSE);
	if (bOwnData) {
	    dis.CtlType = ODT_LISTBOX;
	    dis.hwndItem = lpLBInfo->hWnd;
	    dis.CtlID = (lpLBInfo->hWnd)?
		GetWindowID(lpLBInfo->hWnd):lpLBInfo->hListBox;
	    dis.itemID = (UINT)((int)((short)wIndex));
	    dis.hwndItem = lpLBInfo->hWnd;
	    dis.itemData = 
		((lpLBInfo->wLBoxStyle & LBS_OWNERDRAWMASK) && 
        	 (!(lpLBInfo->wLBoxStyle & LBS_HASSTRINGS) ||
		  (lpLBItem->dwItemData)))?
		lpLBItem->dwItemData:(DWORD)(lpLBItem->lpItemString);
		if (lpLBInfo->hWndParent)
	        SendMessage(lpLBInfo->hWndParent,WM_DELETEITEM,
			(WPARAM)dis.CtlID,(LPARAM)&dis);
	}
	else
	    if (lpLBItem->lpItemString && !(lpLBItem->wItemFlags & LIF_BITMAP))
		WinFree(lpLBItem->lpItemString);
	WinFree((LPSTR)lpLBItem);
      }
      else {
	lpLBItem->wItemWidth = lpLBItem->wItemHeight = 0;
	lpLBItem->wItemFlags &= ~LIF_HILITE;
      }
    }

    if (bDeleteData) {
	lpLBInfo->lpListHead = lpLBInfo->lpListTail = (LPLISTBOXITEM)0;
	WinFree((LPSTR)lpLBInfo->lpItemList);
	lpLBInfo->lpItemList = 0;
    }

    lpLBInfo->wSelCount = lpLBInfo->wTopIndex = 0;
    if (bDeleteData)
	lpLBInfo->wItemCount = 0;
    lpLBInfo->wSelection = (WORD)-1;
    if (lpLBInfo->hWnd) {
	if (lpLBInfo->wState & LSF_SCROLLENABLED) {
	    if (lpLBInfo->wLBoxStyle & LBS_DISABLENOSCROLL) {
		dwStyle = GetWindowStyle(lpLBInfo->hWnd);
		if ((dwStyle & WS_HSCROLL) &&
			!(lpLBInfo->wState & LSF_HSCROLLDISABLED)) {
		    LBoxSetState(lpLBInfo,LSF_HSCROLLDISABLED);
		    EnableScrollBar(lpLBInfo->hWnd,SB_HORZ,ESB_DISABLE_BOTH);
		}
		if ((dwStyle & WS_VSCROLL) &&
			!(lpLBInfo->wState & LSF_VSCROLLDISABLED)) {
		    LBoxSetState(lpLBInfo,LSF_VSCROLLDISABLED);
		    EnableScrollBar(lpLBInfo->hWnd,SB_VERT,ESB_DISABLE_BOTH);
		}
	    }
	    else {
		if (lpLBInfo->wLBoxStyle & LBS_MULTICOLUMN) 
		    fnScroll = SB_HORZ;
		else
		    fnScroll = SB_VERT;
		SetScrollRange(lpLBInfo->hWnd,fnScroll,0,0,FALSE);
		SetScrollPos(lpLBInfo->hWnd,fnScroll,0,FALSE);
		ShowScrollBar(lpLBInfo->hWnd,fnScroll,FALSE);
	    }
	}
	InvalidateRgn(lpLBInfo->hWnd,(HRGN)0,TRUE);
    }
}


static LONG
LBoxDeleteItem(LPLISTBOXINFO lpLBInfo, LPMENUITEMSTRUCT lpMis)
{
    LPLISTBOXITEM lpLBItem, lpPrevItem;
    DELETEITEMSTRUCT dis;
    int fnScroll;
    WORD wIndex;

    wIndex = lpMis->wPosition;
    if (lpMis->wItemFlags & LIF_BYPOSITION) {
	if (!(lpLBItem = LBoxItemFromIndex(lpLBInfo,wIndex)))
		return (LONG)LB_ERR;
    }
    else {
	if (!(lpLBItem = LBoxItemFromCommand(lpLBInfo,&wIndex)))
	   	return (LONG)LB_ERR;
    }
	if ( lpLBInfo->wSelection >= wIndex )
	{
		if ( lpLBInfo->wSelection == wIndex )
			wIndex = LB_ERR;
		else
			wIndex = lpLBInfo->wSelection - 1;
		SendMessage ( lpLBInfo->hWnd, LB_SETCURSEL, wIndex, 0 );
	}
    if (BOWNERDRAW(lpLBInfo)) {
	dis.CtlType = ODT_LISTBOX;
	dis.CtlID = (lpLBInfo->hWnd)?
		GetWindowID(lpLBInfo->hWnd):lpLBInfo->hListBox;
	dis.itemID = (UINT)((int)((short)wIndex));
	dis.hwndItem = lpLBInfo->hWnd;
	dis.itemData =
	    ((lpLBInfo->wLBoxStyle & LBS_OWNERDRAWMASK) && 
	     (!(lpLBInfo->wLBoxStyle & LBS_HASSTRINGS) ||
	      (lpLBItem->dwItemData)))?
	    lpLBItem->dwItemData:(DWORD)(lpLBItem->lpItemString);
	if (lpLBInfo->hWndParent && lpLBInfo->hWnd)
	    SendMessage(lpLBInfo->hWndParent,WM_DELETEITEM,0,(LONG)&dis);
    }
    else
	if (lpLBItem->lpItemString)
	    WinFree(lpLBItem->lpItemString);

    lpLBInfo->lpItemList[lpLBItem->wSlotIndex] = (LPLISTBOXITEM)0;
    if (lpLBItem == lpLBInfo->lpListHead)
	lpLBInfo->lpListHead = lpLBItem->lpNextItem;
    else {
	for (lpPrevItem = lpLBInfo->lpListHead;
	     lpPrevItem && lpPrevItem->lpNextItem != lpLBItem;
	     lpPrevItem = lpPrevItem->lpNextItem);
	if (lpPrevItem)
	    lpPrevItem->lpNextItem = lpLBItem->lpNextItem;
	if (lpLBItem == lpLBInfo->lpListTail)
	    lpLBInfo->lpListTail = lpPrevItem;
    }
    WinFree((LPSTR)lpLBItem);
    if ((lpLBInfo->wState & LSF_REDRAW) && lpLBInfo->hWnd)
	InvalidateRgn(lpLBInfo->hWnd, (HRGN)0, TRUE);
    lpLBInfo->wItemCount--;

    if ((lpLBInfo->wState & LSF_SCROLLENABLED) &&
		(lpLBInfo->wItemCount == lpLBInfo->wFrameSize)) {
	if (lpLBInfo->wLBoxStyle & LBS_MULTICOLUMN) 
	    fnScroll = SB_HORZ;
	else
	    fnScroll = SB_VERT;
	if (lpLBInfo->wTopIndex != 0) {
	    lpLBInfo->wTopIndex = 0;
	    SetScrollPos(lpLBInfo->hWnd,fnScroll,0,TRUE);
	    lpLBInfo->bPosChanged = TRUE;
	    InvalidateRgn(lpLBInfo->hWnd,(HRGN)0,TRUE);
	}
	if (lpLBInfo->wLBoxStyle & LBS_DISABLENOSCROLL) {
	    DWORD dwStyle = GetWindowStyle(lpLBInfo->hWnd);
	    if (lpLBInfo->wLBoxStyle & LBS_MULTICOLUMN) {
		if (dwStyle & WS_HSCROLL) {
		    EnableScrollBar(lpLBInfo->hWnd,SB_HORZ,ESB_DISABLE_BOTH);
		    LBoxSetState(lpLBInfo,LSF_HSCROLLDISABLED);
		}
	    }
	    else if (dwStyle & WS_VSCROLL) {
		EnableScrollBar(lpLBInfo->hWnd,SB_VERT,ESB_DISABLE_BOTH);
		LBoxSetState(lpLBInfo,LSF_VSCROLLDISABLED);
	    }
	}
	else 
	    ShowScrollBar(lpLBInfo->hWnd,fnScroll,FALSE);
    }
    lpLBInfo->bPosChanged = TRUE;
    InvalidateRgn(lpLBInfo->hWnd,(HRGN)0,TRUE);

#if 0
The following code was to handle the posible changing of the selection due to
the delete.  However this does not seem to be correct.
    if (lpLBInfo->wSelection == wIndex) {
	lpLBInfo->wSelection = (WORD)-1;
	if (lpLBInfo->wItemCount <= wIndex) /* We just deleted the last item */
	    wIndex = lpLBInfo->wItemCount - 1;
	LBoxChangeSelection(lpLBInfo,wIndex,LIF_BYPOSITION);
    }
#endif
    lpLBInfo->wSelection = (WORD)-1;
    return (LONG)lpLBInfo->wItemCount;
}

#define	CF_SENDCOMPARE		0
#define	CF_CASECOMPAREPREFIX	1
#define	CF_CASECOMPAREEXACT	2
#define	CF_BINCOMPARE		3

typedef int (*LBOXCOMPAREPROC)(HWND,LPCOMPAREITEMSTRUCT);

static int LBoxSendCompare(HWND,LPCOMPAREITEMSTRUCT); 
static int LBoxCaseComparePrefix(HWND,LPCOMPAREITEMSTRUCT); 
static int LBoxCaseCompareExact(HWND,LPCOMPAREITEMSTRUCT); 
static int LBoxBinCompare(HWND,LPCOMPAREITEMSTRUCT); 

static LBOXCOMPAREPROC LBoxCompareItem[] = {
	LBoxSendCompare,
	LBoxCaseComparePrefix,
	LBoxCaseCompareExact,
	LBoxBinCompare
};

int
LBoxDefaultCompareItem(LPCOMPAREITEMSTRUCT lpCis)
{
    return LBoxCaseComparePrefix((HWND)0,lpCis);
}

static int
LBoxSendCompare(HWND hDest, LPCOMPAREITEMSTRUCT lpcis)
{
    return (int)((short)SendMessage(hDest,
			WM_COMPAREITEM,0,(LPARAM)lpcis));
}

static int
LBoxCaseComparePrefix(HWND hDest, LPCOMPAREITEMSTRUCT lpcis)
{
    int nLen1, nLen2;

    nLen1 = strlen((LPSTR)(lpcis->itemData1));
    nLen2 = strlen((LPSTR)(lpcis->itemData2));

    /* These two special checks force directories to head of list */
    if (*(LPSTR)lpcis->itemData1 != '[' && *(LPSTR)lpcis->itemData2 == '[')
	return -1;

    if (*(LPSTR)lpcis->itemData1 == '[' && *(LPSTR)lpcis->itemData2 != '[')
	return 1;

    return strncasecmp((LPSTR)(lpcis->itemData1),
			(LPSTR)(lpcis->itemData2),
                        min(nLen1,nLen2));
}

static int
LBoxCaseCompareExact(HWND hDest, LPCOMPAREITEMSTRUCT lpcis)
{
    return strcasecmp((LPSTR)(lpcis->itemData1),
			(LPSTR)(lpcis->itemData2));
}

static int
LBoxBinCompare(HWND hDest, LPCOMPAREITEMSTRUCT lpcis)
{
    return (lpcis->itemData1 == lpcis->itemData2)?0:1;
}

static LONG
LBoxFindItem(LPLISTBOXINFO lpLBInfo, WORD wIndexStart, LPSTR lpszFind,
		WORD wFunc)
{
    LPLISTBOXITEM lpLBItem;
    COMPAREITEMSTRUCT cis;
    WORD wIndex;
    BOOL bOwnData;
    BOOL bSelect;
    LBOXCOMPAREPROC lpfnCompare;

    if (wIndexStart == (WORD)-1) {
	lpLBItem = lpLBInfo->lpListHead;
	wIndex = 0;
    }
    else {
	if ((lpLBItem = LBoxItemFromIndex(lpLBInfo,wIndexStart))) {
	    lpLBItem = lpLBItem->lpNextItem;
	    wIndex = wIndexStart+1;
	}
	else
	    wIndex = 0;
    }

    cis.CtlType = ODT_LISTBOX;
    cis.CtlID = (lpLBInfo->hWnd)?
		GetWindowID(lpLBInfo->hWnd):lpLBInfo->hListBox;
    cis.hwndItem = lpLBInfo->hWnd;
    cis.itemID1 = 0;
    cis.itemData1 = (DWORD)lpszFind;
    bOwnData = BOWNDATA(lpLBInfo); /* TODO: we should examine LIF_OWNERDRAW */
    bSelect = (wFunc == LB_SELECTSTRING);
    if (bOwnData)
	lpfnCompare = (lpLBInfo->wLBoxStyle & LBS_SORT)?
		LBoxCompareItem[CF_SENDCOMPARE]:
		LBoxCompareItem[CF_BINCOMPARE];
    else
	lpfnCompare = (wFunc == LB_FINDSTRINGEXACT)?
		LBoxCompareItem[CF_CASECOMPAREEXACT]:
		LBoxCompareItem[CF_CASECOMPAREPREFIX];

    while(lpLBItem) {
	cis.itemID2 = (UINT)((int)((int)wIndex));
	cis.itemData2 = (bOwnData)?lpLBItem->dwItemData:
	        (DWORD)lpLBItem->lpItemString;
	if (!lpfnCompare(lpLBInfo->hWndParent,&cis)) {
	    if (bSelect && wIndex != lpLBInfo->wSelection) {
	        LBoxChangeSelection(lpLBInfo,wIndex,LIF_BYPOSITION);
		LBoxNotify(lpLBInfo,LBN_SELCHANGE,TRUE);
	    }
	    return (LONG)wIndex; 
	}
	wIndex++;
	lpLBItem = lpLBItem->lpNextItem;
    }

    if (wIndexStart != (WORD)-1) {
	wIndex = 0;
	lpLBItem = lpLBInfo->lpListHead;
		
	while(wIndex <= wIndexStart) {
	    if(lpLBItem == 0)
		return (LONG)wIndex;

	    cis.itemID2 = (UINT)((int)((int)wIndex));
	    cis.itemData2 = (bOwnData)?lpLBItem->dwItemData:
	        (DWORD)lpLBItem->lpItemString;
	    if (!lpfnCompare(lpLBInfo->hWndParent,&cis)) {
	        if (bSelect) {
		    LBoxChangeSelection(lpLBInfo,wIndex,LIF_BYPOSITION);
		    LBoxNotify(lpLBInfo,LBN_SELCHANGE,TRUE);
		}
	        return (LONG)wIndex; 
	    }
	    wIndex++;
	    lpLBItem = lpLBItem->lpNextItem;
	}
    }
    return (LONG)LB_ERR;
}

static LONG
LBoxData(LPLISTBOXINFO lpLBInfo, LPMENUITEMSTRUCT lpMis)
{
    WORD wOldSelection;

    if (lpMis->wAction & LCA_ITEMCOUNT)
	    return (LONG)lpLBInfo->wItemCount;
    if (lpMis->wAction & LCA_FONT) {
	if (lpMis->wAction & LCA_SET)
	    lpLBInfo->hFont = (HFONT)(DWORD)lpMis->lpItemData;
	else
	    return (LONG)lpLBInfo->hFont;
    }
    if (lpMis->wAction & LCA_INDENTS) {
	if (lpMis->wAction & LCA_SET) {
	    lpLBInfo->wLeftIndent = LOWORD((DWORD)lpMis->lpItemData);
	    lpLBInfo->wRightIndent = HIWORD((DWORD)lpMis->lpItemData);
	}
	else
	    return MAKELONG(lpLBInfo->wLeftIndent,lpLBInfo->wRightIndent);
    }
    if (lpMis->wAction & LCA_SELECTION) {
	wOldSelection = lpLBInfo->wSelection;
	if (lpMis->wAction & LCA_SET)
	    lpLBInfo->wSelection = lpMis->wPosition;
	return (LONG)wOldSelection;
    }
    return 0;
}

static LONG
LBoxModifyItem(LPLISTBOXINFO lpLBInfo, LPMENUITEMSTRUCT lpMis)
{
    LPLISTBOXITEM lpLBItem;
    LPRECT lpItemRect;
    WORD wState;
    WORD wIndex;
    WORD wOldState;
    DWORD dwStyle;
    BOOL bOwnData;

    wIndex = lpMis->wPosition;
    if (lpMis->wItemFlags & LIF_BYPOSITION) {
	if (!(lpLBItem = LBoxItemFromIndex(lpLBInfo,wIndex)))
	    return (LONG)LB_ERR;
    }
    else {
	if (!(lpLBItem = LBoxItemFromCommand(lpLBInfo,&wIndex)))
	    return (LONG)LB_ERR;
	else
	    lpMis->wPosition = wIndex;
    }
    wOldState = lpLBItem->wItemFlags;
    if (lpMis->wAction & LCA_CHECKBMP) {
	if (lpMis->wAction & LCA_SET)
	    lpLBItem->hCheckedBmp = lpMis->hCheckedBmp;
	else
	    return (LONG)lpLBItem->hCheckedBmp;
    }
    if (lpMis->wAction & LCA_UNCHECKBMP) {
	if (lpMis->wAction & LCA_SET)
	    lpLBItem->hUncheckedBmp = lpMis->hUncheckedBmp;
	else
	    return (LONG)lpLBItem->hUncheckedBmp;
    }
    if (lpMis->wAction & LCA_LEFTINDENT) {
	if (lpMis->wAction & LCA_SET)
	    lpLBItem->wItemLeftIndent = lpMis->wLeftIndent;
	else
	    return MAKELONG(lpLBItem->wItemLeftIndent,
			    lpLBItem->wItemRightIndent);
    }
    if (lpMis->wAction & LCA_RIGHTINDENT) {
	if (lpMis->wAction & LCA_SET)
	    lpLBItem->wItemRightIndent = lpMis->wRightIndent;
	else
	    return MAKELONG(lpLBItem->wItemLeftIndent,
			    lpLBItem->wItemRightIndent);
    }
    if (lpMis->wAction & LCA_RECT) {
	lpItemRect = (LPRECT)lpMis->lpItemData;
	if (lpMis->wAction & LCA_SET) {
	    lpLBItem->pt.x = lpItemRect->left;
	    lpLBItem->pt.y = lpItemRect->top;
	    lpLBItem->wItemWidth = lpItemRect->right - lpItemRect->left;
	    lpLBItem->wItemHeight = lpItemRect->bottom - lpItemRect->top;
	}
	else 
	    SetRect(lpItemRect,
			lpLBItem->pt.x,lpLBItem->pt.y,
			lpLBItem->pt.x + lpLBItem->wItemWidth,
			lpLBItem->pt.y + lpLBItem->wItemHeight);
	return (LONG)LB_OKAY;
    }

    if (lpMis->wAction & LCA_FLAGS) {
	if (lpMis->wAction & LCA_SET)
	    lpLBItem->wItemFlags = lpMis->wItemFlags;
	else
	    return MAKELONG(lpLBItem->wItemFlags, 0);
    }

    if (lpMis->wAction & LCA_ITEMID) {
	if (lpMis->wAction & LCA_SET) {
	    lpLBItem->hPopupMenu = (HMENU)(lpMis->wIDNewItem);
	}
	else
	    return MAKELONG(lpLBItem->hPopupMenu,0);
    }

    if (lpMis->wAction & LCA_CONTENTS) {
	bOwnData = BOWNDATA(lpLBInfo) ||
	    ((lpLBItem->wItemFlags & LIF_OWNERDRAW)?TRUE:FALSE);
	if (lpMis->wAction & LCA_SET) {
	    if (bOwnData)
	        lpLBItem->dwItemData = (DWORD)(lpMis->lpItemData);
	    else if (HIWORD(lpMis->lpItemData) && 
			HIWORD(lpLBItem->lpItemString) &&
			!(lpLBItem->wItemFlags & LIF_BITMAP)) { 
		if ((int)strlen(lpMis->lpItemData) > 
				(int)strlen(lpLBItem->lpItemString))
		    lpLBItem->lpItemString = WinRealloc(
			lpLBItem->lpItemString,
			strlen(lpMis->lpItemData)+1);
		strcpy(lpLBItem->lpItemString,lpMis->lpItemData);
	    }
	    else
		lpLBItem->lpItemString = lpMis->lpItemData;
	}
	else {
	    if (bOwnData)
		return lpLBItem->dwItemData;
	    else
		return (LONG)lpLBItem->lpItemString;
	}
    }

    dwStyle = lpLBInfo->hWnd?
	GetWindowStyle(lpLBInfo->hWnd):0L;
    if ((lpLBInfo->wState & LSF_REDRAW) && (dwStyle & WS_VISIBLE)) {
	wState = lpLBInfo->wState & (ODS_FOCUS | ODS_DISABLED);
	if (lpLBItem->wItemFlags & LIF_HILITE)
	    wState |= ODS_SELECTED;
	    LBoxSendItemDraw(lpLBInfo,(UINT)wIndex,ODA_SELECT,(UINT)wState);
    }
    return MAKELONG(wOldState,0);
}

static LONG
LBoxItemData(LPLISTBOXINFO lpLBInfo, WORD wIndex,
		DWORD lParam, int nFunc)
{
    LPLISTBOXITEM lpLBItem;
    BOOL bOwnData;

    if (!(lpLBItem = LBoxItemFromIndex(lpLBInfo, wIndex)))
	return (LONG)LB_ERR;
    switch(nFunc) {
	case LB_GETITEMDATA:
	    return (LONG)(lpLBItem->dwItemData);

	case LB_SETITEMDATA:
	    lpLBItem->dwItemData = lParam;
	    if (BOWNERDRAW(lpLBInfo))
		lpLBItem->wItemFlags |= LIF_OWNERDRAW;
	    return (LONG)LB_OKAY;

	case LB_GETSEL:
	    return (LONG)(lpLBItem->wItemFlags & LIF_HILITE);

	case LB_GETTEXT:
	case LB_GETTEXTLEN:
	    bOwnData = BOWNDATA(lpLBInfo);
	    if (nFunc == LB_GETTEXT) {
		if (!bOwnData)
		    strcpy((LPSTR)lParam, lpLBItem->lpItemString);
		else {
		    memcpy((LPSTR)lParam,
			(LPSTR)&lpLBItem->dwItemData,
			sizeof(DWORD));
		}
	    }
	    if (bOwnData)
		return (LONG)LB_OKAY;
	    else
		return (LONG)(strlen(lpLBItem->lpItemString));
    }
    return 0;
}

static int
LBoxAddItem(LPLISTBOXINFO lpLBInfo, WORD wFunc,
		LPMENUITEMSTRUCT lpMis)
{
    UINT uSlot;
    LPLISTBOXITEM lpNewItem, lpAfter, lpCurrent;
    COMPAREITEMSTRUCT cis;
    BOOL bOwnData;
    int fnScroll;
    int nCompPrev, nCompAfter;
    WORD wItemIndex,wNextTop;
    LBOXCOMPAREPROC lpfnCompare;
    RECT rc;

    if (!LBoxGetSlot(lpLBInfo)) {
	LBoxNotify(lpLBInfo, LBN_ERRSPACE, TRUE);
	return (int)LB_ERRSPACE;
    }

    for (uSlot = 0; lpLBInfo->lpItemList[uSlot]; uSlot++);

    lpNewItem = (LPLISTBOXITEM)WinMalloc(sizeof(LISTBOXITEM));
    memset((LPVOID)lpNewItem, '\0', sizeof(LISTBOXITEM));

    bOwnData = BOWNDATA(lpLBInfo) ||
	((lpMis->wItemFlags & LIF_OWNERDRAW)?TRUE:FALSE);
    if (!bOwnData) {
	if (!HIWORD(lpMis->lpItemData) || (lpMis->wItemFlags & MF_BITMAP))
	    lpNewItem->lpItemString = lpMis->lpItemData;
	else {
	    lpNewItem->lpItemString = (LPSTR)
		WinMalloc(strlen(lpMis->lpItemData) + 1);
	    strcpy(lpNewItem->lpItemString, lpMis->lpItemData);
	}
    }
    else
	lpNewItem->dwItemData = (DWORD)(lpMis->lpItemData);
    lpNewItem->wSlotIndex = uSlot;

    lpNewItem->wItemLeftIndent = lpMis->wLeftIndent;
    lpNewItem->wItemRightIndent = lpMis->wRightIndent;

    lpNewItem->wItemFlags = lpMis->wItemFlags;
    lpNewItem->hPopupMenu = (HMENU)(lpMis->wIDNewItem);

    lpLBInfo->lpItemList[uSlot] = lpNewItem;
    lpLBInfo->wItemCount++;

    if (!lpLBInfo->lpListHead) {
	lpLBInfo->lpListHead = lpLBInfo->lpListTail = lpNewItem;
	wItemIndex = 0;
    }
    else {
      switch(wFunc) {
	case LAF_INSERT:
	if ((lpMis->wPosition != (WORD)-1) &&
	    (lpMis->wPosition > (WORD)(lpLBInfo->wItemCount-1)))
	    return (int)LB_ERR;
	if (!lpMis->wPosition) {
	    lpNewItem->lpNextItem = lpLBInfo->lpListHead;
	    lpLBInfo->lpListHead = lpNewItem;
	    lpLBInfo->wSelection++;
	    wItemIndex = 0;
	    break;
	}
	if ((lpMis->wPosition == (WORD)-1)
		|| (lpMis->wPosition == (lpLBInfo->wItemCount-1))) {
	    (lpLBInfo->lpListTail)->lpNextItem = lpNewItem;
	    lpLBInfo->lpListTail = lpNewItem;
	    wItemIndex = lpLBInfo->wItemCount - 1;
	}
	else {
	    for (wItemIndex = 0, lpAfter = (LPLISTBOXITEM)0,
			lpCurrent = lpLBInfo->lpListHead;
		 wItemIndex < lpMis->wPosition;
		 wItemIndex++, lpAfter = lpCurrent,
			lpCurrent = lpCurrent->lpNextItem);
	    lpNewItem->lpNextItem = lpAfter->lpNextItem;
	    lpAfter->lpNextItem = lpNewItem;
	    if (lpMis->wPosition <= lpLBInfo->wSelection)
			lpLBInfo->wSelection++;
	}
	break;

	case LAF_APPEND:
	if (!(lpLBInfo->wLBoxStyle & LBS_SORT)) {
	    (lpLBInfo->lpListTail)->lpNextItem = lpNewItem;
	    lpLBInfo->lpListTail = lpNewItem;
	    wItemIndex = lpLBInfo->wItemCount - 1;
	}
	else {

	    cis.CtlType = ODT_LISTBOX;
	    cis.CtlID = (lpLBInfo->hWnd)?
		GetWindowID(lpLBInfo->hWnd):lpLBInfo->hListBox;
	    cis.hwndItem = lpLBInfo->hWnd;

#ifdef	LATER
	Figure out what this means
#endif
	    cis.itemID1 = 0;

	    cis.itemData1 = (bOwnData)?
                lpNewItem->dwItemData:(DWORD)lpNewItem->lpItemString;
	    lpfnCompare = (bOwnData)?
		LBoxCompareItem[CF_SENDCOMPARE]:
		LBoxCompareItem[CF_CASECOMPAREPREFIX];

	    lpAfter = (LPLISTBOXITEM)0;
	    nCompPrev = 1;
	    lpCurrent = lpLBInfo->lpListHead;
	    wItemIndex = 0;
	    while(lpCurrent) {
		cis.itemID2 = uSlot;
		cis.itemData2 = (bOwnData)?          
                    lpCurrent->dwItemData:(DWORD)lpCurrent->lpItemString;
		nCompAfter = lpfnCompare(lpLBInfo->hWndParent,&cis);
		if ((nCompPrev > 0) && (nCompAfter <= 0))
		    break;
		nCompPrev = nCompAfter;
		lpAfter = lpCurrent;
		lpCurrent = lpCurrent->lpNextItem;
		wItemIndex++;
	    }
	    if (lpAfter) {
		lpNewItem->lpNextItem = lpAfter->lpNextItem;
		if (!lpAfter->lpNextItem)
		    lpLBInfo->lpListTail = lpNewItem;
		lpAfter->lpNextItem = lpNewItem;
	    }
	    else {
		lpNewItem->lpNextItem = lpLBInfo->lpListHead;
		lpLBInfo->lpListHead = lpNewItem;
	    }
	    if (wItemIndex <= lpLBInfo->wSelection)
			lpLBInfo->wSelection++;
	}
	break;
	default:
	    return (int)LB_ERR;
      }
    }
    LBoxCalcItemSize(lpLBInfo,lpNewItem,wItemIndex);
    LBoxRecalcPositions(lpLBInfo);

    if ((lpLBInfo->wState & LSF_REDRAW) && lpLBInfo->hWnd &&
	IsWindowVisible(lpLBInfo->hWnd)) {
	wNextTop = lpLBInfo->wTopIndex+lpLBInfo->wFrameSize;
	if ((wItemIndex >= lpLBInfo->wTopIndex) && (wItemIndex < wNextTop)) {
	    GetClientRect(lpLBInfo->hWnd,&rc);
	    rc.left += lpNewItem->pt.x;
	    rc.top += lpNewItem->pt.y;
	    InvalidateRect(lpLBInfo->hWnd,&rc,TRUE);
	}
    }
    if ((lpLBInfo->wState & LSF_SCROLLENABLED) &&
	(lpLBInfo->wItemCount > lpLBInfo->wFrameSize)) {
	if (lpLBInfo->wLBoxStyle & LBS_MULTICOLUMN) {
	    fnScroll = SB_HORZ;
	    if (lpLBInfo->wState & LSF_HSCROLLDISABLED) {
		EnableScrollBar(lpLBInfo->hWnd,fnScroll,ESB_ENABLE_BOTH);
		LBoxClearState(lpLBInfo,LSF_HSCROLLDISABLED);
	    }
	}
	else {
	    fnScroll = SB_VERT;
	    if (lpLBInfo->wState & LSF_VSCROLLDISABLED) {
		EnableScrollBar(lpLBInfo->hWnd,fnScroll,ESB_ENABLE_BOTH);
		LBoxClearState(lpLBInfo,LSF_VSCROLLDISABLED);
	    }
	}
	SetScrollRange(lpLBInfo->hWnd,fnScroll,0,lpLBInfo->wItemCount - 
		lpLBInfo->wFrameSize,FALSE);
	SetScrollPos(lpLBInfo->hWnd,fnScroll,lpLBInfo->wTopIndex,
		lpLBInfo->wState & LSF_REDRAW);

	if (wItemIndex < lpLBInfo->wTopIndex)		
		InvalidateRgn(lpLBInfo->hWnd,(HRGN)0,TRUE);
    }

    return (int)wItemIndex;
}

static void
LBoxSetExtent(LPLISTBOXINFO lpLBInfo, WORD wExtent)
{
    RECT rc;
    DWORD dwStyle;
    BOOL bSetRange = FALSE;
    HDC hDC;
    int nDiff,nRange;
    TEXTMETRIC tm;

    if (!(lpLBInfo->wState & LSF_SCROLLENABLED))
	return;

    if (lpLBInfo->wLBoxStyle & LBS_MULTICOLUMN)
	return;

    GetClientRect(lpLBInfo->hWnd,&rc);
    dwStyle = GetWindowStyle(lpLBInfo->hWnd);

    if (lpLBInfo->wLBoxStyle & LBS_DISABLENOSCROLL) {
	if (!(dwStyle & WS_HSCROLL))
	    return;
	if (wExtent > ( WORD )rc.right) {
	    if (lpLBInfo->wState & LSF_HSCROLLDISABLED) {
		EnableScrollBar(lpLBInfo->hWnd,SB_HORZ,ESB_ENABLE_BOTH);
		LBoxClearState(lpLBInfo,LSF_HSCROLLDISABLED);
	    }
	    bSetRange = TRUE;
	}
	else if (!(lpLBInfo->wState & LSF_HSCROLLDISABLED)) {
	    EnableScrollBar(lpLBInfo->hWnd,SB_HORZ,ESB_DISABLE_BOTH);
	    LBoxSetState(lpLBInfo,LSF_HSCROLLDISABLED);
	}
    }
    else {
	if (wExtent > ( WORD )rc.right) 
	    bSetRange = TRUE;
	else if (dwStyle & WS_HSCROLL) {
	    SetScrollRange(lpLBInfo->hWnd,SB_HORZ,0,0,FALSE);
	    ShowScrollBar(lpLBInfo->hWnd,SB_HORZ,FALSE);
	}
    }
    if (bSetRange) {
    	HFONT hFont = 0;

	nDiff = wExtent - rc.right;
	hDC = GetDC(0);
	if (lpLBInfo->hFont)
	    hFont = SelectObject(hDC, lpLBInfo->hFont);
	GetTextMetrics(hDC, &tm);
	if (lpLBInfo->hFont)
	    SelectObject(hDC, hFont);
	ReleaseDC(0,hDC);
	nRange = nDiff / tm.tmAveCharWidth;
	if (nDiff % tm.tmAveCharWidth > 2)
	    nRange++;
	SetScrollRange(lpLBInfo->hWnd,SB_HORZ,0,nRange,TRUE);
	SetScrollPos(lpLBInfo->hWnd,SB_HORZ,0,FALSE);
    }
    lpLBInfo->wHorzExtent = wExtent;
}

static void
LBoxScrollHorz(LPLISTBOXINFO lpLBoxInfo, UINT uiScrollCode, int nPos)
{
    int nStep, nScrollPos, nNewPos;
    int nMinPos, nMaxPos, nPage, nDiff;
    RECT rc;

    GetScrollRange(lpLBoxInfo->hWnd,SB_HORZ,&nMinPos,&nMaxPos);
    if (!nMinPos && !nMaxPos)
	return;
    nScrollPos = GetScrollPos(lpLBoxInfo->hWnd,SB_HORZ);
    GetClientRect(lpLBoxInfo->hWnd,&rc);
    nStep = (int)( (( int )lpLBoxInfo->wHorzExtent - ( int )rc.right) / 
		                (nMaxPos - nMinPos) );

    switch (uiScrollCode) {
	case SB_LINEUP:
	    nNewPos = max(nScrollPos - 1,0);
	    break;
	case SB_LINEDOWN:
	    nNewPos = min(nScrollPos + 1,nMaxPos);
	    break;
	case SB_PAGEUP:
	    nPage = rc.right/nStep;
	    nNewPos = max(nMinPos,nScrollPos - nPage);
	    break;
	case SB_PAGEDOWN:
	    nPage = rc.right/nStep;
	    nNewPos = min(nScrollPos + nPage,nMaxPos);
	    break;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
	    nNewPos = nPos;
	    break;
	default:
	    return;
    }
    if (nNewPos != nScrollPos) {
	SetScrollPos(lpLBoxInfo->hWnd,SB_HORZ,nNewPos,TRUE);
	lpLBoxInfo->wHorzOffset = nStep * nNewPos;
	nDiff = nScrollPos - nNewPos;
	ScrollWindow(lpLBoxInfo->hWnd,nDiff*nStep,0,NULL,NULL);
    }
}

static LRESULT
LBoxGetItemHeight(LPLISTBOXINFO lpLBoxInfo, WORD wIndex)
{
    LPLISTBOXITEM lpLBItem;

    if (!(lpLBoxInfo->wLBoxStyle & LBS_OWNERDRAWVARIABLE))
	return (LRESULT)lpLBoxInfo->wItemHeight;

    if (!(lpLBItem = LBoxItemFromIndex(lpLBoxInfo,wIndex)))
	return (LRESULT)LB_ERR;
    if (lpLBItem->wItemHeight == 0) {
	LBoxCalcItemSize(lpLBoxInfo,lpLBItem,wIndex);
	if (lpLBItem->wItemHeight == 0)
	    return (LRESULT)LB_ERR;
    }
    return (LRESULT)lpLBItem->wItemHeight;
}

/* The following is the listbox API interface */
/*  FIX_ME! WARNING:  Somebody has to call RELEASELBOXINFO eventually! */
HMENU32
GetMenuHandle32(HMENU hMenu)
{
    return (HMENU32)GETLBOXINFO(hMenu);
}

BOOL
CheckMenuHandle32(HMENU hMenu)
{
    LPLISTBOXINFO lpInfo = CHECKLBOXINFO(hMenu);
    BOOL bVal = lpInfo ? TRUE : FALSE;
    return bVal;
}

LONG
LBoxAPI(HMENU32 hMenu32, UINT uiAction, LPARAM lParam)
{
    LPLISTBOXINFO lpLBInfo = (LPLISTBOXINFO)0;
    LPMENUCREATESTRUCT lpmcs;
    LPMENUITEMSTRUCT lpmis;
    HMENU hListBox;
    LONG rc;

    if ((uiAction != LBA_CREATE) &&
	!(lpLBInfo = (LPLISTBOXINFO)hMenu32))
	return 0L;
    switch (uiAction) {
	case LBA_CREATE:
	    if (!(lpmcs = (LPMENUCREATESTRUCT)lParam) ||
		!(lpLBInfo = CREATELBOX(hListBox)))
		break;
	    lpLBInfo->wLBoxStyle = LOWORD(lpmcs->dwStyle);
	    lpLBInfo->wState = (lpmcs->dwStyle & LBS_NOREDRAW)?0:LSF_REDRAW;
	    lpLBInfo->wTopIndex = 0;
	    lpLBInfo->wSelection = (WORD)-1;
	    lpLBInfo->hFont = lpmcs->hFont;
	    lpLBInfo->hListBox = hListBox;
	    lpLBInfo->wLeftIndent = LOWORD(lpmcs->dwIndents);
	    lpLBInfo->wRightIndent = HIWORD(lpmcs->dwIndents);
	    return MAKELONG(hListBox,0);

	case LBA_DESTROY:
	    if (lpLBInfo->ObjHead.wRefCount)
		return 0L;
	    LBoxResetContent(lpLBInfo,TRUE);
	    return (LONG)(FREELBOX(lpLBInfo->ObjHead.hObj))?1:0;

	case LBA_MODIFYITEM:
	    rc = (LONG)LBoxModifyItem(lpLBInfo,(LPMENUITEMSTRUCT)lParam);
	    return rc;

	case LBA_INSERTITEM:
	case LBA_APPENDITEM:
	    lpmis = (LPMENUITEMSTRUCT)lParam;
	    lpmis->wLeftIndent = lpLBInfo->wLeftIndent;
	    lpmis->wRightIndent = lpLBInfo->wRightIndent;
	    rc = (LONG)LBoxAddItem(lpLBInfo,
			(uiAction == LBA_APPENDITEM)?LAF_APPEND:LAF_INSERT,
			lpmis);
	    return (rc >= 0)?1:0;

	case LBA_DELETEITEM:
	case LBA_REMOVEITEM:
	    rc = (LONG)LBoxDeleteItem(lpLBInfo,(LPMENUITEMSTRUCT)lParam);
	    return (rc >= 0)?1:0;

	case LBA_GETDATA:
	case LBA_SETDATA:
	    rc = (LONG)LBoxData(lpLBInfo,(LPMENUITEMSTRUCT)lParam);
	    return rc;

	default:
	    break;
    }
    return 0L;
}
