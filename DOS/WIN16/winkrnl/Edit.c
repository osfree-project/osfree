/*    
	Edit.c	1.23
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

#include "Edit.h"
#include "Editx.h"
#include "kerndef.h"
#include "Log.h"

HCLASS32 FindClass(LPCSTR, HINSTANCE);

static BOOL OnCreate(HWND,LPCREATESTRUCT);
static void OnLButtonDblClk(HWND,BOOL,int,int,UINT);
static void OnLButtonDown(HWND,BOOL,int,int,UINT);
static void OnLButtonUp(HWND,int,int,UINT);
static void OnMouseMove(HWND,int,int,UINT);
static void OnKey(HWND,UINT,BOOL,int,UINT);
static void OnKillFocus(HWND,HWND);
static void OnSetFocus(HWND,HWND);
#ifdef TWIN32
	static void OnChar(HWND hWnd, TCHAR ch, int cRepeat);
	static void OnSetText(HWND hWnd, LPCTSTR lpszText);
	static INT OnGetText(HWND hWnd, int cchTextMax, LPTSTR lpszText);
#else
	static void OnChar(HWND,UINT,int);
	static void OnSetText(HWND,LPCSTR);
	static UINT OnGetText(HWND,int,LPSTR);
#endif
static BOOL OnEraseBkgnd(HWND,HDC);
static void OnPaint(HWND);
static UINT OnGetTextLength(HWND);
static HFONT OnGetFont(HWND);
static void OnSetFont(HWND,HFONT,BOOL);
static BOOL OnNCCreate(HWND,LPCREATESTRUCT);
static void OnNCDestroy(HWND);
static UINT OnGetDlgCode(HWND,LPMSG);
static void OnSize(HWND,UINT,int,int);
static int OnVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
static int OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
static void OnUndo(HWND);
static LRESULT OnConvert(HWND,WPARAM,LPARAM);
static HANDLE OnEMGetHandle(HWND);
static void OnEMSetHandle(HWND, HANDLE);
static BOOL OnEMGetModify(HWND);
static void OnEMSetModify(HWND, BOOL);
static DWORD OnEMGetSel(HWND, LPDWORD, LPDWORD);
static void OnEMSetSel(HWND, UINT, int, int);
static void OnEMReplaceSel(HWND, LPCSTR);
static int OnEMGetLineCount(HWND);
static int OnEMLineIndex(HWND,int);
static int OnEMLineLength(HWND, int);
static int OnEMLineFromChar(HWND, int);
static int OnEMGetFirstVisibleLine(HWND);
static int OnEMGetLine(HWND, int, LPSTR);

#ifdef LATER
static UINT OnEMGetPasswordChar(HWND);
static UINT OnEMSetPasswordChar(HWND, UINT);
#endif

static void OnEMLimitText(HWND, UINT);
static EDITWORDBREAKPROC OnEMGetWordBreakProc(HWND);
static void OnEMSetWordBreakProc(HWND, EDITWORDBREAKPROC);
static BOOL OnEMLineScroll(HWND, int, int);
static BOOL OnEMSetReadOnly(HWND, BOOL);
static BOOL OnEMCanUndo(HWND);
static void OnEMEmptyUndoBuffer(HWND);
static void OnEMSetRect(HWND, LPRECT);
static void OnEMSetRectNP(HWND, LPRECT);
static void OnEMGetRect(HWND, LPRECT);
static BOOL OnEMSetTabStops(HWND, int, LPINT);
static BOOL OnEMFmtLines(HWND, BOOL);

static void OnCut(HWND);
static void OnCopy(HWND);
static void OnPaste(HWND);

static void ResetEditControl(HWND, LPEDIT);

extern void HandleKey(HWND , UINT );

LRESULT CALLBACK _export DefEDITProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	switch(wMsg)
	{
		HANDLE_MSG(hWnd,WM_LBUTTONUP,OnLButtonUp);
		HANDLE_MSG(hWnd,WM_LBUTTONDOWN,OnLButtonDown);
		HANDLE_MSG(hWnd,WM_LBUTTONDBLCLK,OnLButtonDblClk);
		HANDLE_MSG(hWnd,WM_MOUSEMOVE,OnMouseMove);
		HANDLE_MSG(hWnd,WM_SETFOCUS,OnSetFocus);
		HANDLE_MSG(hWnd,WM_KILLFOCUS,OnKillFocus);
		HANDLE_MSG(hWnd,WM_GETTEXTLENGTH,OnGetTextLength);
		HANDLE_MSG(hWnd,WM_SETTEXT,OnSetText);
		HANDLE_MSG(hWnd,WM_GETTEXT,OnGetText);
		HANDLE_MSG(hWnd,WM_ERASEBKGND,OnEraseBkgnd);
 		HANDLE_MSG(hWnd,WM_PAINT,OnPaint);
		HANDLE_MSG(hWnd,WM_GETFONT,OnGetFont);
		HANDLE_MSG(hWnd,WM_SETFONT,OnSetFont);
		HANDLE_MSG(hWnd,WM_NCCREATE,OnNCCreate);
		HANDLE_MSG(hWnd,WM_NCDESTROY,OnNCDestroy);
		HANDLE_MSG(hWnd,WM_GETDLGCODE,OnGetDlgCode);
		HANDLE_MSG(hWnd,WM_SIZE,OnSize);
		HANDLE_MSG(hWnd,WM_CREATE,OnCreate);
		HANDLE_MSG(hWnd,WM_KEYUP,OnKey);
		HANDLE_MSG(hWnd,WM_KEYDOWN,OnKey);
		HANDLE_MSG(hWnd,WM_CHAR,OnChar);
		HANDLE_MSG(hWnd,WM_VSCROLL,OnVScroll);
		HANDLE_MSG(hWnd,WM_HSCROLL,OnHScroll);
		HANDLE_MSG(hWnd,WM_UNDO,OnUndo);
		HANDLE_MSG(hWnd,WM_CUT,OnCut);
		HANDLE_MSG(hWnd,WM_COPY,OnCopy);
		HANDLE_MSG(hWnd,WM_PASTE,OnPaste);

		HANDLE_MSG(hWnd,WM_CONVERT,OnConvert);

		HANDLE_MSG(hWnd,EM_GETHANDLE,OnEMGetHandle);
		HANDLE_MSG(hWnd,EM_SETHANDLE,OnEMSetHandle);
		HANDLE_MSG(hWnd,EM_GETMODIFY,OnEMGetModify);
		HANDLE_MSG(hWnd,EM_SETMODIFY,OnEMSetModify);
		HANDLE_MSG(hWnd,EM_GETSEL,OnEMGetSel);
		HANDLE_MSG(hWnd,EM_SETSEL,OnEMSetSel);
		HANDLE_MSG(hWnd,EM_REPLACESEL,OnEMReplaceSel);
		HANDLE_MSG(hWnd,EM_GETLINECOUNT,OnEMGetLineCount);
		HANDLE_MSG(hWnd,EM_LINEINDEX,OnEMLineIndex);
		HANDLE_MSG(hWnd,EM_LINELENGTH,OnEMLineLength);
		HANDLE_MSG(hWnd,EM_LINEFROMCHAR,OnEMLineFromChar);
		HANDLE_MSG(hWnd,EM_GETFIRSTVISIBLELINE,OnEMGetFirstVisibleLine);
		HANDLE_MSG(hWnd,EM_GETLINE,OnEMGetLine);
		HANDLE_MSG(hWnd,EM_LIMITTEXT,OnEMLimitText);
		HANDLE_MSG(hWnd,EM_GETWORDBREAKPROC,OnEMGetWordBreakProc);
		HANDLE_MSG(hWnd,EM_SETWORDBREAKPROC,OnEMSetWordBreakProc);
		HANDLE_MSG(hWnd,EM_LINESCROLL,OnEMLineScroll);
		HANDLE_MSG(hWnd,EM_SETREADONLY,OnEMSetReadOnly);
		HANDLE_MSG(hWnd,EM_UNDO,OnUndo);
		HANDLE_MSG(hWnd,EM_CANUNDO,OnEMCanUndo);
		HANDLE_MSG(hWnd,EM_EMPTYUNDOBUFFER,OnEMEmptyUndoBuffer);
		HANDLE_MSG(hWnd,EM_GETRECT,OnEMGetRect);
		HANDLE_MSG(hWnd,EM_SETRECT,OnEMSetRect);
		HANDLE_MSG(hWnd,EM_SETRECTNP,OnEMSetRectNP);
		HANDLE_MSG(hWnd,EM_SETTABSTOPS,OnEMSetTabStops);
		HANDLE_MSG(hWnd,EM_FMTLINES,OnEMFmtLines);
	}
	return DefWindowProc(hWnd, wMsg, wParam, lParam);
}

static BOOL
OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
	LPEDIT lp ;
	if (!(lp = GetLPEdit(hWnd)))
		return FALSE ;

	if (HasVScroll(lp))
		SetScrollRange(hWnd, SB_VERT, 0, 100, TRUE) ;

	if (HasHScroll(lp))
		SetScrollRange(hWnd, SB_HORZ, 0, 100, TRUE) ;

	return TRUE;
}

/*
 * OnLButtonDblClk: Double-click handler
 *
 * -- Get the focus and the capture (since we _did_ get a click).
 * -- Call ClearAnchor() to save the last highlight for updating.
 * -- Manually set the anchor to the word beginning before the click.
 * -- Call MovePosTo() to move the caret to the word starting after the
 *	anchor, select it, and update.
 *
 * This could be really simple if Microsoft had used the WordBreakProc function
 * to try to find the next word.  They don't.  So we duplicate WordBreakProc's
 * logic here...
 */
static void
OnLButtonDblClk(HWND hWnd, BOOL fDoubleClick, int xPos, int yPos, UINT keyFlags)
{
	LPEDIT 	lp;
	POINT	pt ;

	if (!(lp = GetLPEdit(hWnd)))
	    return;

	pt.x = xPos ;
	pt.y = yPos ;
	WindowToClientPoint(lp, &pt) ;

	lp->npdata = (LPSTR)EditMemoryAPI(hWnd, EMA_LOCK,lp->hData, 0L);
	SetFocus(hWnd);
	SetCapture(hWnd);

	ClearAnchor(lp) ;	/* Reset anchor */
	lp->anchor = lp->WordBreakProc(lp->npdata, FindSpot(lp, pt.x, pt.y),
				lp->strlen, WB_LEFT) ;
	/* This is the DefWordBreakProc replacement. */
	{
		int iWordBegin = lp->anchor + 1 ;
		int nTextLen = lp->strlen ;
		char *lpszText = lp->npdata ;
		if (iWordBegin >= nTextLen)
			iWordBegin = nTextLen - 1 ;

			/* CHANGE HERE: _Don't_ skip whitespace, see if
			 * statement below */
			if (lpszText[iWordBegin] == '\r' ||
			    lpszText[iWordBegin] == '\n')
			{
				while (iWordBegin < nTextLen &&
					lpszText[iWordBegin] == '\r')
					iWordBegin++ ;
				iWordBegin++ ;
			}
			/* CHANGE HERE: If the previous word pointed to white-
			 * space (i. e. start of line or start of file), select
			 * _only_ the whitespace. */
			else if (lp->WordBreakProc(lpszText, iWordBegin, nTextLen,
				WB_ISDELIMITER))
			{
				while (iWordBegin < nTextLen &&
					lp->WordBreakProc(lpszText, iWordBegin,
						nTextLen, WB_ISDELIMITER))
					iWordBegin++ ;
			}
			else
			{
				while (iWordBegin < nTextLen &&
					!lp->WordBreakProc(lpszText, iWordBegin,
						nTextLen, WB_ISDELIMITER) &&
					lpszText[iWordBegin] != '\r' &&
					lpszText[iWordBegin] != '\n')
					iWordBegin++ ;
				while (iWordBegin < nTextLen &&
					lp->WordBreakProc(lpszText, iWordBegin,
						nTextLen, WB_ISDELIMITER))
					iWordBegin++ ;
			}

		MovePosTo(lp, iWordBegin, prev) ;
	}

	EditMemoryAPI(hWnd, EMA_UNLOCK,lp->hData, 0L);
}

static void
OnLButtonDown(HWND hWnd, BOOL fDoubleClick, int xPos, int yPos, UINT keyFlags)
{
	LPEDIT 	lp;
	POINT pt ;

	if (fDoubleClick)
	{
		OnLButtonDblClk(hWnd, fDoubleClick, pt.x, pt.y, keyFlags) ;
		return ;
	}

	lp = GetLPEdit(hWnd) ;
	if (!lp)
		return ;

	pt.x = xPos ;
	pt.y = yPos ;
	WindowToClientPoint(lp, &pt) ;

	lp->npdata = (LPSTR)EditMemoryAPI(hWnd, EMA_LOCK,lp->hData, 0L);
	SetFocus(hWnd);
	SetCapture(hWnd);

	ClearAnchor(lp) ;
	MovePosTo(lp, FindSpot (lp, pt.x, pt.y), any);

	lp->bMouseAnchor = TRUE;
	SetAnchor(lp, lp->lpidx) ;
	DoCaretPos(lp, TRUE);
	EditMemoryAPI(hWnd, EMA_UNLOCK,lp->hData, 0L);
}

static void
OnLButtonUp(HWND hWnd, int xPos, int yPos, UINT keyFlags)
{
	LPEDIT 	lp;

	if(!(lp = GetLPEdit(hWnd))) 
		return;

	ReleaseCapture();
	lp->bMouseAnchor = FALSE;
	if (lp->anchor == lp->lpidx)
	{
		ClearAnchor(lp) ;
		UpdateHighlight(lp) ;
	}
}

static void
OnMouseMove(HWND hWnd, int xPos, int yPos, UINT keyFlags)
{
	POINT pt ;
	LPEDIT 	lp;

	pt.x = xPos ;
	pt.y = yPos ;

	if(!(lp = GetLPEdit(hWnd))) 
	    return;

	WindowToClientPoint(lp, &pt) ;

	if (HasFocus(lp) && lp->bMouseAnchor)
	{
		lp->npdata = (LPSTR)EditMemoryAPI(hWnd, EMA_LOCK, lp->hData, 0);

		SetAnchor(lp, lp->lpidx) ;

		MovePosTo(lp, FindSpot (lp, pt.x, pt.y), here);

		EditMemoryAPI(hWnd, EMA_UNLOCK,lp->hData, 0L);
	}
}

static void
OnKey(HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	LPEDIT 	lp;

	if(!(lp = GetLPEdit(hWnd))) 
	    return;

	if (fDown) {
		lp->npdata = (LPSTR)EditMemoryAPI(hWnd, EMA_LOCK, lp->hData, 0L);
		HandleKey(hWnd, vk);
		EditMemoryAPI(hWnd, EMA_UNLOCK, lp->hData, 0L);
	}
  	else {
		switch (vk) {
	  		case VK_SHIFT:
				lp->state &= ~ES_SHIFTDOWN;
				break;

			case VK_CONTROL:
				lp->state &= ~ES_CTRLDOWN;
				break;
		}
	}
}

static void
OnKillFocus(HWND hWnd, HWND hwndNewFocus)
{
	LPEDIT 	lp;

	if (!(lp = GetLPEdit(hWnd))) 
	    return;

	lp->bMouseAnchor = FALSE;

	lp->npdata = (LPSTR)EditMemoryAPI(hWnd, EMA_LOCK, lp->hData, 0L);
	lp->state &= ~(ES_FOCUS | ES_CARET);

	/* if ctrl if on; make it off */
	if (lp->state & ES_CTRLDOWN)
		lp->state &= ~ES_CTRLDOWN;

	/* if shift is on; make it off */
	if (lp->state & ES_SHIFTDOWN)
		lp->state &= ~ES_SHIFTDOWN;

	HideCaret(hWnd) ;
	DestroyCaret();

	UpdateAnchor(lp) ;
	UpdateWindow(hWnd) ;

	NotifyParent(lp, EN_KILLFOCUS) ;
	EditMemoryAPI(hWnd, EMA_UNLOCK, lp->hData, 0L);

}

static void
OnSetFocus(HWND hWnd, HWND hwndOldFocus)
{
	LPEDIT 	lp;

	if (hWnd == hwndOldFocus)
		return ;		/* We already have focus, exit */

	if(!(lp = GetLPEdit(hWnd))) 
    	return;

	lp->state |= ES_FOCUS;
	UpdateAnchor(lp) ;
	UpdateWindow(hWnd) ;
	DoCaretPos(lp, FALSE) ;
	ShowCaret(hWnd) ;
	NotifyParent(lp, EN_SETFOCUS) ;
}

#ifdef TWIN32
static void
OnChar(HWND hWnd, TCHAR ch, int cRepeat)
#else
static void
OnChar(HWND hWnd, UINT ch, int cRepeat)
#endif
{
	LPEDIT 	lp;

	if (!(lp = GetLPEdit(hWnd))) 
		return;
	
	/* expand the text buffer for later... */
	if ((lp->strlen + 8) > lp->memlen)
	{
		lp->memlen += 512;
		lp->hData = (HANDLE)EditMemoryAPI(hWnd, EMA_REALLOC, lp->hData, lp->memlen); 
		lp->npdata = (LPSTR)EditMemoryAPI(hWnd, EMA_LOCK, lp->hData,0L);
	}

	SetModified(lp, TRUE) ;
	TextMsg(lp, ch);
	EditMemoryAPI(hWnd, EMA_UNLOCK, lp->hData, 0L);
}

static void
OnSetText(HWND hWnd, LPCSTR lpText)
{
	LPEDIT 	lp;
	LPSTR lpszText;

	if (!(lp = GetLPEdit(hWnd))) 
    		return;
	
	lpszText = (lpText)?(LPSTR)lpText:"";

	lp->strlen = (unsigned int)lstrlen(lpszText);

	if (lp->strlen > lp->memlen)
	{
		do
		{
			lp->memlen += 512;
	    } while (lp->strlen > lp->memlen);
	    
	    lp->hData = (HANDLE)EditMemoryAPI(hWnd, EMA_REALLOC, lp->hData, (LPARAM)lp->memlen); 
	}

	lp->npdata = (LPSTR)EditMemoryAPI(hWnd, EMA_LOCK, lp->hData, 0L);
	lstrcpy(lp->npdata, lpszText);

	if ( lp->dwStyle & ES_UPPERCASE )
		AnsiUpper(lp->npdata);
	else if ( lp->dwStyle & ES_LOWERCASE )
		AnsiLower(lp->npdata);

	ClearUndoBuffer(lp);
	NotifyParent(lp, EN_UPDATE);
	ResetEditControl(hWnd, lp);
	UpdateWindow(hWnd) ;
	EditMemoryAPI(hWnd, EMA_UNLOCK,lp->hData, 0L);
	NotifyParent(lp, EN_CHANGE);
}

static BOOL
OnEraseBkgnd(HWND hWnd, HDC hDC)
{
	HWND hParent;
	RECT Rect;
	LPEDIT 	lp;
	HBRUSH oldBrush= (HBRUSH)NULL;
	int	gotbrush= 0;
	 
	if(!(lp = GetLPEdit(hWnd))) 
    	return FALSE;
	
	if ( !lp->hBr )
		lp->hBr = GetStockObject ( WHITE_BRUSH );
	if ((hParent = GetParent(hWnd)))
	{
		oldBrush= SelectObject(hDC, lp->hBr);
		lp->hBr = (HBRUSH)SendMessage(hParent,
			GET_WM_CTLCOLOR_MSG(CTLCOLOR_EDIT),
			GET_WM_CTLCOLOR_MPS(hDC, hWnd, CTLCOLOR_EDIT));
		gotbrush= !(lp->hBr == (HBRUSH)NULL);
		if (!gotbrush)
			lp->hBr= CreateSolidBrush(GetSysColor(COLOR_WINDOW));
	}

	GetClientRect(hWnd, &Rect);
	FillRect(hDC, &Rect, lp->hBr);

	if (oldBrush)
		SelectObject(hDC, oldBrush);

	if (gotbrush)
	{
		DeleteObject(lp->hBr);		
		lp->hBr= (HBRUSH) NULL;
	}

	return TRUE;
}

static void
OnPaint(HWND hWnd)
{
	LPEDIT 	lp;

	if(!(lp = GetLPEdit(hWnd))) 
    	return;
	
	lp->npdata = (LPSTR)EditMemoryAPI(hWnd, EMA_LOCK, lp->hData, 0L);
	
	PaintEditControl(lp);
	EditMemoryAPI(hWnd, EMA_UNLOCK, lp->hData, 0L);
}

#ifdef TWIN32
static INT
OnGetText(HWND hWnd, int cchTextMax, LPSTR lpszText)
#else
static UINT
OnGetText(HWND hWnd, int cchTextMax, LPSTR lpszText)
#endif
{
	LPEDIT 	lp;
	int len = 0;

	if(!(lp = GetLPEdit(hWnd))) 
    	return 0;

	lp->npdata = (LPSTR)EditMemoryAPI(hWnd, EMA_LOCK, lp->hData, 0L);

	if(lp->npdata)
	{
	   len = min(cchTextMax, (int)(lp->strlen+1));
	   lstrcpyn(lpszText, lp->npdata, len);
	}

	EditMemoryAPI(hWnd, EMA_UNLOCK, lp->hData, 0L);

	return len;
}

static UINT
OnGetTextLength(HWND hWnd)
{
	LPEDIT 	lp;

	if(!(lp = GetLPEdit(hWnd))) 
    	return 0;
	return lp->strlen;	
}

static HFONT
OnGetFont(HWND hWnd)
{
	LPEDIT 	lp;

	if(!(lp = GetLPEdit(hWnd))) 
    	return 0;

	return lp->hFont;	
}

static void
OnSetFont(HWND hwndCtl, HFONT hfont, BOOL fRedraw)
{
	LPEDIT 	lp;

	if(!(lp = GetLPEdit(hwndCtl))) 
	    return;

	if (hfont)
	{
		lp->npdata = (LPSTR)EditMemoryAPI(hwndCtl, EMA_LOCK,lp->hData, 0L);
		if (HasFocus(lp)) 
		{
			HideCaret(hwndCtl);
			DestroyCaret();
		}

		lp->state &= ~ES_CARET;
		lp->hFont = hfont;

		SetLineHeight(lp);
		SetEditRect(lp);

		DoCaretPos(lp, FALSE);
		ShowCaret(hwndCtl) ;

			/* Invalidate all to force redraw */
		InvalidateRect(hwndCtl, NULL, FALSE);
		EditMemoryAPI(hwndCtl, EMA_UNLOCK, lp->hData, 0L);
	}
}

static BOOL
OnNCCreate(HWND hWnd, LPCREATESTRUCT lpcs)
{
	LPEDIT 	lp;

	lp = (LPEDIT)WinMalloc(sizeof(EDIT)) ;
	if (!lp)
		return FALSE ;

	lp->hWnd = hWnd ;
	SetWindowLong(hWnd, SWD_LPSTR, (long)lp);

	lp->lpid = (unsigned int) GetWindowID(hWnd);

	lp->state = 0 ;
	lp->lpidx = 0 ;
	lp->anchor = no_anchor ;
	lp->hData = 0 ;
	lp->npdata = NULL ;
	lp->uSelDS = 0 ;
	lp->memlen = lp->strlen = 0 ;
	lp->nMaxLen = 65535L ;
	lp->lpFragments = NULL ;
	lp->nCurFragment = 0 ;
	lp->linelen = lp->lineheight = 0 ;
	lp->VisLines = 0 ;
	lp->nFirstFragment = 0 ;
	lp->nFragments = 1 ;
	lp->ptOffset.x = 0 ;
	lp->ptOffset.y = 0 ;
	SetRectEmpty(&lp->rcClient) ;
	lp->hBr = 0 ;
	lp->hFont = 0 ;
	lp->dwStyle = 0 ;
	lp->PasswordChar = '*' ;
	lp->bMouseAnchor = FALSE ;
	lp->WordBreakProc = DefWordBreakProc ;
	lp->pUndoBuffer = NULL ;
	lp->cTabs = 0 ;
	lp->pTabPos = NULL ;

	/* allocate mem for text buffer */
	lp->strlen = (lpcs->lpszName) ? lstrlen(lpcs->lpszName) : 0;
	lp->memlen = (lp->strlen) ? lp->strlen + 1 : INIT_DATA_SIZE;

	if (!(lp->hData = (HANDLE)EditMemoryAPI(hWnd, EMA_ALLOC,
			0, lp->memlen))) {
	    WinFree((LPSTR)lp) ;
	    return FALSE;
	}

	if (!(lp->npdata = (LPSTR)EditMemoryAPI(hWnd,EMA_LOCK,lp->hData,0L)))
	{
	    EditMemoryAPI(hWnd, EMA_FREE, lp->hData,0L);
	    WinFree((LPSTR)lp) ;
	    return FALSE;
	}

	/* allocate mem for FRAGMENTS index array */
	if (!(lp->lpFragments = (LPFRAGMENT)WinMalloc(sizeof(FRAGMENT)))) {
	    EditMemoryAPI(hWnd, EMA_UNLOCK, lp->hData, 0L);
	    EditMemoryAPI(hWnd, EMA_FREE, lp->hData,0L);
	    WinFree((LPSTR)lp) ;
	    return FALSE;
	}

	lp->nFirstFragment = 0;

	if (lp->strlen)
	{
	    lstrcpy(lp->npdata, lpcs->lpszName);
	    lp->lpFragments[0].nOffset = 0;
	    lp->lpFragments[0].nLength = lp->strlen;
	    SetEditPosition(lp, lp->strlen, any);
	}
	else 
	{
	    lstrcpy(lp->npdata, "\0");
	    lp->lpFragments[0].nOffset = 0;
	    lp->lpFragments[0].nLength = 0;
	    SetEditPosition(lp, 0, any);
	}

	lp->dwStyle = lpcs->style;

	EditMemoryAPI(hWnd, EMA_UNLOCK,lp->hData, 0L);
	return TRUE;
}

static void
OnNCDestroy(HWND hWnd)
{
	LPEDIT 	lp;
	
	if(!(lp = GetLPEdit(hWnd))) 
    	return;

	if (lp->state & ES_CARET)
	{
		HideCaret(hWnd) ;
		DestroyCaret() ;
	}

	WinFree((LPSTR)lp->lpFragments);

	if (lp->hData)
	{
		EditMemoryAPI(hWnd, EMA_UNLOCK, lp->hData, 0L);
		EditMemoryAPI(hWnd, EMA_FREE, lp->hData, 0L);
	}
	
	if (lp->pUndoBuffer)
	{
		if (lp->pUndoBuffer->pDelChars)
			WinFree(lp->pUndoBuffer->pDelChars) ;
		WinFree((LPSTR)(lp->pUndoBuffer)) ;
	}
	WinFree((LPSTR)lp) ;
}

static UINT
OnGetDlgCode(HWND hWnd, LPMSG lpmsg)
{
	return (DLGC_WANTCHARS | DLGC_HASSETSEL | DLGC_WANTARROWS);
}

static void
OnSize(HWND hWnd, UINT state, int cx, int cy)
{
	int nHeight = 0;
	LPEDIT 	lp;

	if(!(lp = GetLPEdit(hWnd))) 
	    return;
	SetEditRect(lp);

	nHeight = cy ;
	nHeight += lp->lineheight-1;

	lp->VisLines = Multiline(lp) ? 
			nHeight / lp->lineheight : 1; 
}

static int
OnVScroll(HWND hWnd, HWND hwndCtl, UINT code, int pos)
{
	int sbMin,sbMax,sbPos ;
	LPEDIT lp ;

	if (!(lp = GetLPEdit(hWnd)))
		return 0;

	GetScrollRange(hWnd, SB_VERT, &sbMin, &sbMax) ;
	sbPos = GetScrollPos(hWnd, SB_VERT) ;

	switch (code) {
		case SB_TOP:
			sbPos = sbMin ;
			break ;

		case SB_BOTTOM:
			sbPos = sbMax ;
			break ;

		case SB_LINEUP:
			VScroll(lp, -1) ;
			return 0;

		case SB_LINEDOWN:
			VScroll(lp, 1) ;
			return 0;

		case SB_PAGEUP:
			VScroll(lp, -lp->VisLines) ;
			return 0;

		case SB_PAGEDOWN:
			VScroll(lp, lp->VisLines) ;
			return 0;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			sbPos = pos ;
			break ;

                case 0x040E: /* Undocumented message: MiD 19-DEC-1995 */
                             /* Notepad, etc. doesn't update vertical thumb position without that! */
                        return (lp->nFragments == 0) ? 0 : lp->nFirstFragment * 100 / lp->nFragments;

		default:
			return 0;
		}

	VScroll(lp, (sbPos * lp->nFragments) / 100 - lp->nFirstFragment) ;

	return 0;
}

static int
OnHScroll(HWND hWnd, HWND hwndCtl, UINT code, int pos)
{
	int sbMin, sbMax, sbPos ;
	LPEDIT lp ;

	if (!(lp = GetLPEdit(hWnd)))
		return 0;

	GetScrollRange(hWnd, SB_HORZ, &sbMin, &sbMax) ;

	switch (code) {

		case SB_LEFT:
			sbPos = sbMin ;
			break ;

		case SB_RIGHT:
			sbPos = sbMax ;
			break ;

		case SB_LINELEFT:
			HScroll(lp, -lp->aveWidth) ;
			return 0;

		case SB_LINERIGHT:
			HScroll(lp, lp->aveWidth) ;
			return 0;

		case SB_PAGELEFT:
			HScroll(lp, -Width(lp)) ;
			return 0;

		case SB_PAGERIGHT:
			HScroll(lp, Width(lp)) ;
			return 0;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			sbPos = pos ;
			break;

                case 0x040E: /* Undocumented message: MiD 19-DEC-1995 */
                        {    /* Notepad, etc. doesn't update horizontal thumb position without that! */
                        int  i, nLongest = 0;
                        char szBuf[256];
 
                        for (i = 0;   i < lp->nFragments;  i++)
                            {
                            *(WORD *)szBuf = sizeof(szBuf) - 1;
                            SendMessage(hWnd, EM_GETLINE, i, (LPARAM)(LPSTR)szBuf);
                            if (nLongest < (int)strlen(szBuf))
                               nLongest = strlen(szBuf);
                            }
                        return (nLongest == 0) ? 0 : (lp->ptOffset.x/lp->aveWidth) * 100 / nLongest;
                        }

		default:
			return 0;
		}

	HScroll(lp, (sbPos * lp->linelen) / 100 - lp->ptOffset.x) ;

	return 0;
}

static WNDPROC lpEditBinToNat = 0;

static LRESULT
OnConvert(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HCLASS32 hEditClass32;

    if (!lpEditBinToNat) {
	hEditClass32 = FindClass("EDIT",0);
	lpEditBinToNat = (WNDPROC)GetClassHandleLong(
			hEditClass32,GCL_BINTONAT);
    }
    if (lpEditBinToNat)
	return lpEditBinToNat(hWnd, WM_CONVERT, wParam, lParam);
    else
	return (LRESULT)0;
}

static HANDLE
OnEMGetHandle(HWND hWnd)
{
	LPEDIT lp;
 
	return (lp = GetLPEdit(hWnd)) ? lp->hData : 0 ;
}

static void
OnEMSetHandle(HWND hWnd, HANDLE hBuf)
{
	LPEDIT lp;

	if(!(lp = GetLPEdit(hWnd))) 
		return;

	lp->hData = hBuf;
	lp->memlen = (unsigned int)EditMemoryAPI(hWnd, EMA_SIZE, lp->hData, 0L);
	ClearUndoBuffer(lp) ;
	ResetEditControl(hWnd,lp);
}


static BOOL
OnEMGetModify(HWND hWnd)
{
	LPEDIT lp;
	
	if ((lp = GetLPEdit(hWnd)))
	    	return IsModified(lp);
    	return FALSE;
}

static void
OnEMSetModify(HWND hWnd, BOOL fModified)
{
	LPEDIT lp;
	
	if ((lp = GetLPEdit(hWnd)))
	    	SetModified(lp, fModified);
}

static DWORD
OnEMGetSel(HWND hWnd, LPDWORD lpdwStart, LPDWORD lpdwEnd)
{
	LPEDIT lp;
	int beg, end ;

	if(!(lp = GetLPEdit(hWnd))) 
	{
		if (lpdwStart)
			*lpdwStart = 0;
		if (lpdwEnd)
			*lpdwEnd = 0;
   		return ((DWORD)0);
	}

	if (AnchorSet(lp))
	{
		GetAnchor(lp, &beg, &end) ;
		if (lpdwStart)
			*lpdwStart = beg;
		if (lpdwEnd)
			*lpdwEnd = end;
		return MAKELONG(beg, end) ;
	}
	if (lpdwStart)
		*lpdwStart = lp->lpidx;
	if (lpdwEnd)
		*lpdwEnd = lp->lpidx;
	return MAKELONG(lp->lpidx, lp->lpidx) ;
}
	
static void
OnEMSetSel(HWND hWnd, UINT fScroll, int nStart, int nEnd)
{
	LPEDIT lp;
	
	if(!(lp = GetLPEdit(hWnd))) 
   		return;

	if ( nStart == nEnd )
		return;

	lp->npdata = (LPSTR)EditMemoryAPI(hWnd, EMA_LOCK, lp->hData, 0L);

	ClearAnchor(lp);
	UpdateHighlight(lp);

	if (nStart == 0 && nEnd == -1)	/* select all text */
	{
		SetAnchor(lp, 0) ;
		SetEditPosition(lp, lp->strlen, any) ;
		UpdateHighlight(lp) ;
	} 
	else				/* de-select all text */
		if (nStart != -1) 
		{
			if (nStart < nEnd) 
			{
				SetAnchor(lp, nStart);
				SetEditPosition(lp, nEnd, any);
			} 
			else 
			{
				SetAnchor(lp, nEnd);
				SetEditPosition(lp, nStart, any);
			}
			UpdateHighlight(lp) ;
		}
	
	DoCaretPos(lp, fScroll);				

	EditMemoryAPI(hWnd, EMA_UNLOCK,lp->hData, 0L);
}

static void
OnEMReplaceSel(HWND hWnd, LPCSTR lpszReplace)
{
	LPEDIT lp;

	if(!(lp = GetLPEdit(hWnd))) 
 		return;

	lp->npdata = (LPSTR)EditMemoryAPI(hWnd, EMA_LOCK, lp->hData, 0L);
	InsertChars(lp, (LPSTR)lpszReplace, strlen(lpszReplace), FALSE);
	EditMemoryAPI(hWnd, EMA_UNLOCK, lp->hData, 0L);
}

static int
OnEMGetLineCount(HWND hWnd)
{
	LPEDIT lp;
	
	if ((lp = GetLPEdit(hWnd)))
		return lp->nFragments;
	return 1;
}

static int
OnEMLineIndex(HWND hWnd, int nLine)
{
	LPEDIT lp;
	
	if(!(lp = GetLPEdit(hWnd)))
		return 0;
		
	if(nLine >= lp->nFragments)
		return -1;

	return (lp->lpFragments[nLine < 0 ? lp->nCurFragment : nLine].nOffset);				
}

static int
OnEMLineLength(HWND hWnd, int nCh)

{
	LPEDIT lp;
		
	if(!(lp = GetLPEdit(hWnd)))
		return 0;

	if (nCh < 0)
		if (AnchorSet(lp))
		{
			int beg, end ;
			int begline, endline ;
			
			GetAnchor(lp, &beg, &end) ;
			begline = lp->lpFragments[FindLine(lp, beg)].nOffset ;
			endline = lp->lpFragments[FindLine(lp, end)].nOffset + lp->lpFragments[FindLine(lp, end)].nLength ;
			return (endline - begline) - (end-beg) ;
		}
		else
			nCh = lp->lpidx ;

	return lp->lpFragments[FindLine(lp, nCh)].nLength;

}

static int
OnEMLineFromChar(HWND hWnd, int nCh)
{
	LPEDIT lp;
	
	if(!(lp = GetLPEdit(hWnd)))
		return 0;

	if (nCh < 0)
	{
		if (AnchorSet(lp))
		{
			int beg, end ;
			GetAnchor(lp, &beg, &end) ;
			return FindLine(lp, beg) ;
		}
		return FindLine(lp, lp->lpidx) ;
	}
	return FindLine(lp, nCh) ;
}

static int
OnEMGetFirstVisibleLine(HWND hWnd)
{
	LPEDIT lp;
	
	if(!(lp = GetLPEdit(hWnd)))
		return 0;

	return lp->nFirstFragment;
}

static int
OnEMGetLine(HWND hWnd, int nLine, LPSTR lpszLine)
{
	LPEDIT lp;
	int len;
	
	if(!(lp = GetLPEdit(hWnd)))
		return 0;

	/* what selected values... */
	if(nLine < 0)
		nLine = lp->nCurFragment ;

	if(nLine >= lp->nFragments)
		return 0;

	strncpy(lpszLine, &lp->npdata[lp->lpFragments[nLine].nOffset], len = min((WORD)(*((LPWORD)lpszLine)),(WORD)lp->lpFragments[nLine].nLength));
		/* Above changed from lstrcpyn, since that sticks a null in last position.  TPK 3-25-97 */
	lpszLine[len] = '\0'; /* MiD 19-DEC-1995: a must, since lstrcpyn doesn't do it! */
	return len;
}

#ifdef LATER
static UINT
OnEMGetPasswordChar(HWND hWnd)
{
	LPEDIT lp;
	
	if(!(lp = GetLPEdit(hWnd)))
		return 0;
	return (lp->PasswordChar);
}


static UINT
OnEMSetPasswordChar(HWND hWnd, UINT ch)
{
	LPEDIT lp;

	if(!(lp = GetLPEdit(hWnd)))
		return 0;

	lp->PasswordChar = ch;

	return (lp->PasswordChar);
}
#endif

static void
OnEMLimitText(HWND hWnd, UINT cchMax)
{
	LPEDIT lp ;

	if ((lp = GetLPEdit(hWnd)))
		lp->nMaxLen = cchMax ? cchMax : 0xFFFF ;
}

static EDITWORDBREAKPROC
OnEMGetWordBreakProc(HWND hWnd)
{
	LPEDIT lp ;
	if ((lp = GetLPEdit(hWnd)))
		return lp->WordBreakProc ;
	return 0 ;
}

static void
OnEMSetWordBreakProc(HWND hWnd, EDITWORDBREAKPROC ewbp)
{
	LPEDIT lp ;
	if ((lp = GetLPEdit(hWnd)))
		lp->WordBreakProc = ewbp ;
}

static BOOL
OnEMLineScroll(HWND hWnd, int dx, int dy)
{
	LPEDIT lp;
	if (!(lp = GetLPEdit(hWnd)))
		return FALSE ;

	if (!Multiline(lp))
		return FALSE ;

	HScroll(lp, dx * lp->aveWidth) ;
	VScroll(lp, dy) ;
	return TRUE ;
}

static BOOL
OnEMSetReadOnly(HWND hWnd, BOOL bReadOnly)
{
	LPEDIT lp;
	if (!(lp = GetLPEdit(hWnd)))
		return FALSE ;

	SetReadOnly(lp, bReadOnly) ;
	return TRUE ;
}

static void
OnUndo(HWND hWnd)
{
	LPEDIT lp;
	if (!(lp = GetLPEdit(hWnd)))
		return ;

	DoUndo(lp) ;
}

static BOOL
OnEMCanUndo(HWND hWnd)
{
	LPEDIT lp;
	if (!(lp = GetLPEdit(hWnd)))
		return FALSE ;

	return CanUndo(lp) ;
}

static void
OnEMEmptyUndoBuffer(HWND hWnd)
{
	LPEDIT lp;
	if (!(lp = GetLPEdit(hWnd)))
		return ;

	ClearUndoBuffer(lp) ;
}

static void
OnEMSetRect(HWND hWnd, LPRECT lpRect)
{
	OnEMSetRectNP(hWnd, lpRect) ;
	InvalidateRect(hWnd, NULL, TRUE) ;
	UpdateWindow(hWnd) ;
}

static void
OnEMSetRectNP(HWND hWnd, LPRECT lpRect)
{
	LPEDIT lp ;
	if (!(lp = GetLPEdit(hWnd)))
		return ;

	lp->state |= ES_FORMATRECT ;
	CopyRect(&lp->rcClient, lpRect) ;
	InflateRect(&lp->rcClient, -INNER_OFFSET, -INNER_OFFSET) ;
}

static void
OnEMGetRect(HWND hWnd, LPRECT lpRect)
{
	LPEDIT lp ;
	if (!(lp = GetLPEdit(hWnd)))
		return ;

	CopyRect(lpRect, &lp->rcClient) ;
}

static BOOL
OnEMSetTabStops(HWND hWnd, int nTabStops, LPINT pTabStops)
{
	LPEDIT lp ;
	LPINT pTabArray ;
	int xDialogBase ;
	int i ;
	if (!(lp = GetLPEdit(hWnd)))
		return FALSE ;

	if (nTabStops == 0)
	{
		pTabArray = NULL ;
	}
	else
	{
		if (!(pTabArray = (LPINT)WinMalloc(nTabStops * sizeof(int))))
			return FALSE ;

		xDialogBase = LOWORD(GetDialogBaseUnits()) ;
		for (i = 0 ; i < nTabStops ; i++)
			pTabArray[i] = pTabStops[i] * xDialogBase / 4 ;
	}
	WinFree((char *)(lp->pTabPos)) ;
	lp->pTabPos = pTabArray ;
	lp->cTabs = nTabStops ;

	return TRUE ;
}

static BOOL OnEMFmtLines(HWND hWnd, BOOL bFormat)
{
	LPEDIT lp ;
	if (!(lp = GetLPEdit(hWnd)))
		return bFormat ;

	if ((bFormat && FormatLines(lp)) ||
	   (!bFormat && !FormatLines(lp)))
	   	return bFormat ;

	if (bFormat)
		InsertRRN(lp) ;
	else
		DeleteRRN(lp) ;

	return bFormat ;
}

static void
ResetEditControl(HWND hWnd, LPEDIT lp)
{
	lp->npdata = (LPSTR)EditMemoryAPI(hWnd, EMA_LOCK, lp->hData, 0L);
	lp->strlen = strlen(lp->npdata);
	lp->nFirstFragment = 0;
	lp->ptOffset.x = 0 ;
	lp->ptOffset.y = 0 ;
	lp->anchor = -1;
	SetModified(lp, FALSE);
	ReformatFragments(lp);
	SetEditPosition(lp, 0, any);
	DoCaretPos(lp, TRUE);

	InvalidateRect(hWnd, 0, TRUE);
	EditMemoryAPI(hWnd, EMA_UNLOCK,lp->hData, 0L);
}

void TextMsg(LPEDIT lp, UINT ch)
{
	char chInsert ;
	UINT shift ;
						
	if(lp->npdata == 0L)
		return;
	
	if (lp->state & ES_CTRLDOWN)
		switch(ch)
		{
			case 'c':
			case 'C':
				/* SelectionToClipboard(lp->hWnd, lp);              */
                                /* Do nothing since we have separate OnCopy handler */
				return;

			case 'v':
			case 'V':
				/* ClipboardToSelection(lp->hWnd, lp);               */
                                /* Do nothing since we have separate OnPaste handler */
				return;

			case 'x':
			case 'X':
				shift = lp->state & ES_SHIFTDOWN;
				lp->state |= ES_SHIFTDOWN;
				HandleDelete(lp);
				lp->state &= ~ES_SHIFTDOWN;
				lp->state |= shift;
				return;

			case 'z':
			case 'Z':
				DoUndo(lp);
				return;
		}
	else
		switch(ch)
		{
			case '\r':
			case '\n':
				InsertChars(lp, "\r\n", 2, TRUE);
				break;
	
			case '\b':
				if (AnchorSet(lp)) 
				{
					DeleteChars(lp, lp->anchor, lp->lpidx, TRUE);
					lp->anchor = no_anchor ;
					break;
				}
				if (lp->lpidx == 0)
					break;
	
				if (lp->npdata[lp->lpidx - 1] == '\n')
					DeleteChars(lp, lp->lpidx - 2, lp->lpidx, TRUE);
				else	
					DeleteChars(lp, lp->lpidx - 1, lp->lpidx, TRUE);
				break;
	
			default:
				chInsert = ch ;
				InsertChars(lp, &chInsert, 1, TRUE);
				break;
		}
	

	NotifyParent(lp, EN_UPDATE) ;
	UpdateMe(lp) ;
	NotifyParent(lp, EN_CHANGE) ;

}

/*
 * InsertChars inserts a character block into the buffer at the
 * current location.  It assumes that there is enough memory and
 * updates the index to follow the insertion.
 * We need to add code to handle new lines in the insertion buffer
 * and to maintain the undo buffer.
 */
void 
InsertChars(LPEDIT lp, LPSTR str, int len, BOOL bTestReadOnlyStyle)
{
	BOOL bFormat ;
	int i;
	int BegSpot;
	RECT rcInvalid ;
	
	if (bTestReadOnlyStyle && IsReadOnly(lp))
	{
		return ;
	}

	if ((bFormat = FormatLines(lp)))
		DeleteRRN(lp) ;

	/* If anything is selected; remove it */
	if (AnchorSet(lp))
	{
		DeleteChars(lp, lp->lpidx, lp->anchor, bTestReadOnlyStyle);
		lp->anchor = no_anchor ;
	}

	BegSpot = lp->lpidx;

	SetUndoInsert(lp, BegSpot, str, len) ;

	if (lp->strlen + len > lp->nMaxLen)
		len = lp->nMaxLen - lp->strlen ;

	if (lp->strlen + len >= lp->memlen)
	{	
		/* expand buffer */
	    while (lp->strlen + len >= lp->memlen)
			lp->memlen += 0x20;
	    EditMemoryAPI(lp->hWnd, EMA_UNLOCK, lp->hData, 0L);
	    lp->hData = (HANDLE)EditMemoryAPI(lp->hWnd, EMA_REALLOC,
							lp->hData, lp->memlen);
	    lp->npdata = (LPSTR)EditMemoryAPI(lp->hWnd, EMA_LOCK, lp->hData, 0L);
	}

	/* Check boundary of control to see if selection fits, clip if needed */
	if ( !Multiline(lp) && !AutoHScroll(lp) )
	{
		WORD NewWidth;
		WORD CurWidth;
		HDC hDC = GetDC(lp->hWnd);

		if (lp->hFont)
			SelectObject(hDC, lp->hFont) ;

		NewWidth = LOWORD(GetTabbedTextExtent(hDC, str, len, lp->cTabs, lp->pTabPos));
		CurWidth = LOWORD(GetTabbedTextExtent(hDC, lp->npdata, lp->strlen, lp->cTabs, lp->pTabPos));

		while ( ( WORD)(NewWidth + CurWidth) > ( WORD )Width(lp) )
			NewWidth = LOWORD(GetTabbedTextExtent(hDC, str, --len, lp->cTabs, lp->pTabPos));

		ReleaseDC(lp->hWnd, hDC);
	}

	/* Make room for the new characters by sliding the old ones down */
	for(i = lp->strlen + 1; i >= BegSpot; i--) 
		lp->npdata[i+ len] = lp->npdata[i];

	/* now add the new characters to the buffer */
	for (i = 0; i < len; i++)
		lp->npdata[BegSpot + i] = str[i];

	if ( lp->dwStyle & ES_UPPERCASE )
		AnsiUpperBuff(&lp->npdata[BegSpot], len);
	else if ( lp->dwStyle & ES_LOWERCASE )
		AnsiLowerBuff(&lp->npdata[BegSpot], len);

	/* Update the state of the control */
	lp->strlen += len;
	lp->npdata[lp->strlen] = '\0';

	/* Invalidate what's left of this line */
	rcInvalid.left = HPosHere(lp) ;
	rcInvalid.right = lp->ptOffset.x + Width(lp) ;
	rcInvalid.top = lp->nCurFragment * lp->lineheight ;
	rcInvalid.bottom = rcInvalid.top + lp->lineheight ;
	InvalidateClientRect(lp, &rcInvalid) ;

	CurFragment(lp)->nLength += len;
	for (i = lp->nCurFragment + 1 ; i < lp->nFragments ; i++)
		lp->lpFragments[i].nOffset += len ;

	if (bFormat)
		InsertRRN(lp) ;

	ReformatFragments(lp);
	SetEditPosition(lp, lp->lpidx + len, any);
}

/*
 * DeleteChars removes a group of characters from the buffer.
 * The characters at indices beg_idx to end_idx-1 are removed.
 * It assumes that everything in OK with the world when it does it.  That
 * means that both end points are >= 0, the memory is actually there, etc.
 */

void
DeleteChars(LPEDIT lp, int beg_idx, int end_idx, BOOL bTestReadOnlyStyle)
{
	BOOL bFormat ;
	int dn, BegSpot, EndSpot;
	int i;
	RECT rcInvalid ;

	if ((bFormat = FormatLines(lp)))
		DeleteRRN(lp) ;

	if (bTestReadOnlyStyle && IsReadOnly(lp))
	{
		return ;
	}

	if (beg_idx <= end_idx) 
	{
		BegSpot = beg_idx;
		EndSpot = end_idx;
	} 
	else 
	{
		BegSpot = end_idx;
		EndSpot = beg_idx;
	}

/* THIS IS NOT SUPPORTED ON SUN/OS, USE OLD METHOD
 *	memmove(&npdata[BegSpot],
 *		&npdata[EndSpot],
 *		strlen - EndSpot);
 */
	dn = EndSpot - BegSpot;

	SetUndoDelete(lp, BegSpot, &lp->npdata[BegSpot], dn) ;

	for(i = EndSpot; i < lp->strlen; i++)
		lp->npdata[i - dn] = lp->npdata[i];
/* END OF OLD METHOD */

	lp->strlen -= dn ;
	lp->npdata[lp->strlen] = '\0';

	if (bFormat)
		InsertRRN(lp) ;

	/* Invalidate what's left of this line */
	rcInvalid.left = HPosHere(lp) ;
	rcInvalid.right = lp->ptOffset.x + Width(lp) ;
	rcInvalid.top = lp->nCurFragment * lp->lineheight ;
	rcInvalid.bottom = rcInvalid.top + lp->lineheight ;
	InvalidateClientRect(lp, &rcInvalid) ;

	ReformatFragments(lp);
	SetEditPosition(lp, BegSpot, any);
}

/*
 * The SetLineHeight function looks at the textmetrics associated with the
 * current font and sets the line height field in the edit struct
 * appropriately.  It needs to be called when the window is created and
 * whenever we change fonts.
 */

void SetLineHeight(LPEDIT lp)
{
	HDC hDC = GetDC(lp->hWnd);
	TEXTMETRIC textmetric;

	if (lp->hFont)
		SelectObject(hDC, lp->hFont);

	GetTextMetrics(hDC, &textmetric);
	lp->lineheight = textmetric.tmExternalLeading + textmetric.tmHeight;
	lp->externalLeading = textmetric.tmExternalLeading ;
	lp->aveWidth = textmetric.tmAveCharWidth ;

	if (lp->state & ES_CARET)
	{
		DestroyCaret() ;
		lp->state &= ~ES_CARET ;
		DoCaretPos(lp, TRUE) ;
	}

	ReleaseDC(lp->hWnd, hDC);
}

void SetEditRect(LPEDIT lp)
{
	RECT Rect;

	GetClientRect(lp->hWnd, &Rect);

	if ( !lp->lineheight )
		SetLineHeight(lp);

	if ( lp->state & ES_FORMATRECT )
		return ;		/* Format rect not modified by resize */

	if ( lp->dwStyle & WS_BORDER )
	{
		Rect.left += INNER_OFFSET;
		Rect.top += INNER_OFFSET;
		Rect.right -= INNER_OFFSET;
		Rect.bottom -= INNER_OFFSET;
	}

	CopyRect(&lp->rcClient, &Rect);

	/* If we have soft line breaks, redo them */
	if (FormatLines(lp))
	{
		DeleteRRN(lp) ;
		InsertRRN(lp) ;
	}
}

/*
 * FindSpot: Sets edit position based on x, y position
 */

int FindSpot (LPEDIT lp, int xPos, int yPos)
{
	int	nFrag;

	if (xPos < 0) xPos = 0;
	if (yPos < 0) yPos = 0;

	nFrag = yPos / lp->lineheight;

	if (nFrag >= lp->nFragments)
		nFrag = lp->nFragments - 1;

	return lp->lpFragments[nFrag].nOffset + HPosToX(lp, nFrag, xPos) ;
}

/*
 * BuildAFragment: Return the length of the fragment beginning at
 *   the given position, and update the position
 *
 * BuildAFragment seeks along the line, searching for the following:
 *  -- A newline (either \r\n or \n)
 *  -- If ES_AUTOHSCROLL is not set, a line overflow (auto word wrap)
 */
int BuildAFragment(LPEDIT lp, int *pnOffset)
{
	HDC hDC ;
	int nFragmentLength, nFragmentWidth ;
	int nOffsetStart = *pnOffset ;

	hDC = GetDC(lp->hWnd) ;
	if (lp->hFont)
		SelectObject(hDC, lp->hFont) ;
	
	for (nFragmentLength = 1 ; (*pnOffset < lp->strlen) && nFragmentLength <= 1024 ; nFragmentLength++, (*pnOffset)++)
	{
		if (lp->npdata[*pnOffset] == '\r')
		{
			if (!(lp->npdata[++(*pnOffset)] == '\n'))
				(*pnOffset)-- ;
			break;
		}
		if (lp->npdata[*pnOffset] == '\n')
			break ;
		if (!AutoHScroll(lp) && Multiline(lp) && (LOWORD(GetTabbedTextExtent(hDC, &lp->npdata[nOffsetStart], nFragmentLength, lp->cTabs, lp->pTabPos)) > ( WORD)Width(lp)))
		{
			nFragmentLength = lp->WordBreakProc(lp->npdata, *pnOffset, lp->strlen, WB_LEFT) - nOffsetStart ;
			*pnOffset = nOffsetStart + nFragmentLength - 1 ;
			break ;
		}
	}
	if ((nFragmentWidth = LOWORD(GetTabbedTextExtent(hDC, &lp->npdata[nOffsetStart], --nFragmentLength, lp->cTabs, lp->pTabPos))) > lp->linelen)
		lp->linelen = nFragmentWidth ;
	(*pnOffset)++ ;
	ReleaseDC(lp->hWnd, hDC) ;
	return nFragmentLength ;
}

/*
 * ReformatFragments: Rebuilds fragment table and invalidates
 *   changed fragments
 */
void ReformatFragments(LPEDIT lp)
{
	FRAGMENT fragTemp ;
	int nOffset ;
	int nFrag ;

	lp->linelen = 0 ;

	for (nFrag = 0, nOffset = 0 ; (nOffset <= lp->strlen) && (nFrag < lp->nFragments) ; nFrag++)
	{
		fragTemp.nOffset = nOffset ;
		fragTemp.nLength = BuildAFragment(lp, &nOffset) ;
		if ((lp->lpFragments[nFrag].nOffset != fragTemp.nOffset) ||
		    (lp->lpFragments[nFrag].nLength != fragTemp.nLength))
		{
			lp->lpFragments[nFrag].nOffset = fragTemp.nOffset ;
			lp->lpFragments[nFrag].nLength = fragTemp.nLength ;
			InvalidateLine(lp, nFrag) ;
		}
	}

	/* Did the number of fragments shrink? */
	if (nFrag < lp->nFragments)
	{
		/* Invalidate everything extra */
		while (--(lp->nFragments) >= nFrag)
			InvalidateLine(lp, lp->nFragments) ;
		lp->nFragments++ ;

		/* Now shrink the fragment area */
		lp->lpFragments = (LPFRAGMENT)WinRealloc((LPSTR)lp->lpFragments, lp->nFragments * sizeof(FRAGMENT)) ;
	}
	/* Do we need more fragments? */
	if (nOffset <= lp->strlen)
	{
		/* Okay, how many? */
		int nOffsetOld = nOffset ;
		while (nOffset <= lp->strlen)
			lp->nFragments++, BuildAFragment(lp, &nOffset) ;

		/* Expand the fragment area */
		lp->lpFragments = (LPFRAGMENT)WinRealloc((LPSTR)lp->lpFragments, lp->nFragments * sizeof(FRAGMENT)) ;

		/* Now continue the fragment table */
		for (nOffset = nOffsetOld ; (nOffset <= lp->strlen) && (nFrag < lp->nFragments) ; nFrag++)
		{
			lp->lpFragments[nFrag].nOffset = nOffset ;
			lp->lpFragments[nFrag].nLength = BuildAFragment(lp, &nOffset) ;
			InvalidateLine(lp, nFrag) ;
		}
	}
}

/*
 * DefWordBreakProc: The default word break procedure
 *
 * This procedure is responsible for things like finding the next/previous
 * word, determining whether "this" character is a space-type, etc.
 *
 * If the action is WB_ISDELIMITER, it returns TRUE or FALSE; if it is
 * WB_LEFT or WB_RIGHT, it returns the character position of the next word.
 */
int CALLBACK DefWordBreakProc(LPSTR lpszText, int iWordBegin, int nTextLen,
		int action)
{
	char c ;
	switch (action)
	{
		case WB_ISDELIMITER:
			c = lpszText[iWordBegin] ;
			return c == ' ' || c == '\t' ;

		case WB_LEFT:
			/* Walk back to the end of the previous word */
			while (--iWordBegin >= 0 && DefWordBreakProc(lpszText,
					iWordBegin, nTextLen, WB_ISDELIMITER)) ;
			/* Walk back to the start of this word */
			if (lpszText[iWordBegin] == '\r' ||
			    lpszText[iWordBegin] == '\n')
				while (--iWordBegin >= 0 && lpszText[iWordBegin] == '\r');
			else
				while (iWordBegin >= 0 &&
					lpszText[iWordBegin] != ' ' &&
					lpszText[iWordBegin] != '\t' &&
					lpszText[iWordBegin] != '\r' &&
					lpszText[iWordBegin] != '\n')
					iWordBegin-- ;

			return ++iWordBegin ;

		case WB_RIGHT:
			while (iWordBegin < nTextLen && DefWordBreakProc(lpszText,
				iWordBegin, nTextLen, WB_ISDELIMITER))
				iWordBegin++ ;
			if (lpszText[iWordBegin] == '\r' ||
			    lpszText[iWordBegin] == '\n')
			{
				while (iWordBegin < nTextLen &&
					lpszText[iWordBegin] == '\r')
					iWordBegin++ ;
				iWordBegin++ ;
			}
			else
			{
				while (iWordBegin < nTextLen &&
					lpszText[iWordBegin] != ' ' &&
					lpszText[iWordBegin] != '\t' &&
					lpszText[iWordBegin] != '\r' &&
					lpszText[iWordBegin] != '\n')
					iWordBegin++ ;
				while (iWordBegin < nTextLen &&
					DefWordBreakProc(lpszText, iWordBegin,
						nTextLen, WB_ISDELIMITER))
					iWordBegin++ ;
			}
			return iWordBegin ;

		default:
			return 0 ;
	}
}

/*
 * EDIT::DoUndo: Main undo routine
 *
 * This routine _must_ save the old edit buffer before calling
 * DeleteChars() and InsertChars() to actually do the undo,
 * since these call SetUndoDelete() and SetUndoInsert() which
 * would mess up our undoing.
 */
void DoUndo(LPEDIT lp)
{
	EDITUNDO *pUndo ;

	if (!CanUndo(lp))
		return ;

	pUndo = lp->pUndoBuffer ;
	lp->pUndoBuffer = NULL ;	/* Pretend we can't undo */

	/* Delete everything we inserted */
	DeleteChars(lp, pUndo->nStartPos, pUndo->nEndPos, TRUE) ;

	/* If we deleted anything, insert it back in */
	if (pUndo->pDelChars)
	{
		lp->anchor = no_anchor ;
		InsertChars(lp, pUndo->pDelChars, pUndo->nDelLen, TRUE) ;
		SetAnchor(lp, pUndo->nStartPos) ;	/* Select undone stuff */
	}
	else
		ClearAnchor(lp) ;

	if (pUndo->pDelChars)
		WinFree((LPSTR)(pUndo->pDelChars)) ;
	WinFree((LPSTR)pUndo) ;

	MovePosTo(lp, lp->lpidx, here) ;
}

/*
 * SetUndoDelete(): Resets the undo buffer for deleted chars
 */
void SetUndoDelete(LPEDIT lp, int nPos, char *pDel, int nLen)
{
	if (lp->pUndoBuffer)
		if (lp->pUndoBuffer->nStartPos != lp->pUndoBuffer->nEndPos)
			ClearUndoBuffer(lp) ;
		else if (lp->pUndoBuffer->nStartPos == nPos + nLen)
		{
			char *pOldChars = lp->pUndoBuffer->pDelChars ;
			lp->pUndoBuffer->pDelChars = WinMalloc(lp->pUndoBuffer->nDelLen + nLen) ;
			strncpy(lp->pUndoBuffer->pDelChars, pDel, nLen) ;
			strncpy(lp->pUndoBuffer->pDelChars + nLen, pOldChars, lp->pUndoBuffer->nDelLen) ;
			lp->pUndoBuffer->nDelLen += nLen ;
			lp->pUndoBuffer->nStartPos -= nLen ;
			lp->pUndoBuffer->nEndPos -= nLen ;
			WinFree(pOldChars) ;
		}
		else if (lp->pUndoBuffer->nStartPos == nPos)
		{
			char *pOldChars = lp->pUndoBuffer->pDelChars ;
			lp->pUndoBuffer->pDelChars = WinMalloc(lp->pUndoBuffer->nDelLen + nLen) ;
			strncpy(lp->pUndoBuffer->pDelChars, pOldChars, lp->pUndoBuffer->nDelLen) ;
			strncpy(lp->pUndoBuffer->pDelChars + lp->pUndoBuffer->nDelLen, pDel, nLen) ;
			lp->pUndoBuffer->nDelLen += nLen ;
			WinFree(pOldChars) ;
		}
		else
			ClearUndoBuffer(lp) ;

	if (!lp->pUndoBuffer)
	{
		lp->pUndoBuffer = (EDITUNDO *)WinMalloc(sizeof(EDITUNDO)) ;
		lp->pUndoBuffer->nStartPos = nPos ;
		lp->pUndoBuffer->nEndPos = nPos ;
		lp->pUndoBuffer->pDelChars = WinMalloc(nLen) ;
		strncpy(lp->pUndoBuffer->pDelChars, pDel, nLen) ;
		lp->pUndoBuffer->nDelLen = nLen ;
	}
}

/*
 * SetUndoInsert(): Tells the undo buffer we inserted chars
 *  May reset entire buffer if position is different
 */
void SetUndoInsert(LPEDIT lp, int nPos, char *pIns, int nLen)
{
	if (lp->pUndoBuffer)
		if (lp->pUndoBuffer->nEndPos != nPos)
			ClearUndoBuffer(lp) ;

	if (!lp->pUndoBuffer)
	{
		lp->pUndoBuffer = (EDITUNDO *)WinMalloc(sizeof(EDITUNDO)) ;
		lp->pUndoBuffer->nStartPos = nPos ;
		lp->pUndoBuffer->nEndPos = nPos ;
		lp->pUndoBuffer->pDelChars = NULL ;
		lp->pUndoBuffer->nDelLen = 0 ;
	}

	lp->pUndoBuffer->nEndPos += nLen ;
}

/****************************************************************/
static void OnCut(HWND hwnd)
{
        LPEDIT  lp = GetLPEdit(hwnd);
 
        if (!lp) return;

	SendMessage(hwnd, WM_COPY, 0, 0L);
	HandleDelete(lp);
}



/****************************************************************/
static void OnCopy(HWND hwnd)
{
        HGLOBAL hText;
        LPSTR   lpText;
        int     TextSize;
        int     BegSpot;
        int     EndSpot;
	LPEDIT  lp = GetLPEdit(hwnd);

	if (!lp) return;
 
        GetAnchor(lp, &BegSpot, &EndSpot) ;
        TextSize = EndSpot - BegSpot ;
 
        if ((TextSize == 0) || !AnchorSet(lp))
                return;
 
        if ((hText = GlobalAlloc(GMEM_MOVEABLE, TextSize*sizeof(char))) == (HGLOBAL)0)
                return;
 
        if ((lpText = (LPSTR)GlobalLock(hText)) == NULL)
        {
                GlobalFree(hText);
                return;
        }
 
        lstrcpyn (lpText, lp->npdata+BegSpot, TextSize);
        *(lpText + TextSize) = '\0';
 
        if (OpenClipboard(hwnd))
        {
                EmptyClipboard();
                SetClipboardData(CF_TEXT, hText);
                CloseClipboard();
        }
}


/****************************************************************/
static void OnPaste(HWND hwnd)
{
        HGLOBAL hCbData;
        LPSTR   lpCbData;
	LPEDIT  lp = GetLPEdit(hwnd);

        if (!lp || !OpenClipboard(hwnd))
                return;
 
        if ((hCbData = GetClipboardData(CF_TEXT)) == (HGLOBAL)0)
        {
                CloseClipboard();
                return;
        }
 
        if ((lpCbData = (LPSTR)GlobalLock(hCbData)) == NULL)
        {
                CloseClipboard();
                return;
        }
 
	SetModified(lp, TRUE);
        InsertChars(lp, lpCbData, lstrlen(lpCbData), TRUE);
	UpdateMe(lp);
	/*DoCaretPos(lp, 1);*/
 
        CloseClipboard();
}


