/*
	@(#)Editx.h	2.7
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
 
#ifndef EDITX_H
#define EDITX_H

#ifdef	HANDLE_WM_HSCROLL
#undef	HANDLE_WM_HSCROLL
#undef	HANDLE_WM_VSCROLL
#endif

/* int Cls_OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos); */
#define HANDLE_WM_HSCROLL(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((hwnd), \
			GET_WM_HSCROLL_HWND(wParam,lParam), \
			GET_WM_HSCROLL_CODE(wParam,lParam), \
			GET_WM_HSCROLL_POS(wParam,lParam))

/* int Cls_OnVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos); */
#define HANDLE_WM_VSCROLL(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((hwnd), \
			GET_WM_VSCROLL_HWND(wParam,lParam), \
			GET_WM_VSCROLL_CODE(wParam,lParam), \
			GET_WM_VSCROLL_POS(wParam,lParam))

/* HANDLE OnEMGetHandle(HWND hWnd) */
#define HANDLE_EM_GETHANDLE(hWnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HANDLE)(fn)((hWnd))

/* void OnEMSetHandle(HWND hWnd, HANDLE hBuf) */
#define HANDLE_EM_SETHANDLE(hWnd, wParam, lParam, fn) \
    ((fn)((hWnd), (HANDLE)(wParam)), 0L)

/* BOOL OnEMGetModify(HWND hWnd) */
#define HANDLE_EM_GETMODIFY(hWnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(BOOL)(fn)((hWnd));

/* void OnEMSetModify(HWND hWnd, BOOL fModified) */
#define HANDLE_EM_SETMODIFY(hWnd, wParam, lParam, fn) \
    ((fn)((hWnd), (BOOL)(wParam)), 0L)

/* DWORD OnEMGetSel(HWND hWnd, LPDWORD lpdwStart, LPDWORD lpdwEnd) */
#define HANDLE_EM_GETSEL(hWnd, wParam, lParam, fn) \
    (DWORD)(fn)((hWnd), (LPDWORD)(LONG)(wParam), (LPDWORD)(lParam))

/* void OnEMSetSel(HWND hWnd, UINT fScroll, int nStart, int nEnd) */
#define HANDLE_EM_SETSEL(hWnd, wParam, lParam, fn) \
    ((fn)((hWnd),(UINT)(wParam), \
    GET_EM_SETSEL_START(wParam,lParam), \
    GET_EM_SETSEL_END(wParam,lParam)), 0L)

/* void OnEMReplaceSel(HWND hWnd, LPCSTR lpszReplace) */
#define HANDLE_EM_REPLACESEL(hWnd, wParam, lParam, fn) \
    ((fn)((hWnd), (LPCSTR)(lParam)), 0L)

/* int OnEMGetLineCount(HWND hWnd) */
#define HANDLE_EM_GETLINECOUNT(hWnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((hWnd))

/* int OnEMLineIndex(HWND hWnd, int nLine) */
#define HANDLE_EM_LINEINDEX(hWnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((hWnd), (int)(wParam))

/* int OnEMLineLength(HWND hWnd, int nCh) */
#define HANDLE_EM_LINELENGTH(hWnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((hWnd), (int)(wParam))

/* int OnEMLineFromChar(HWND hWnd, int nCh) */
#define HANDLE_EM_LINEFROMCHAR(hWnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((hWnd), (int)(wParam))

/* int OnEMGetFirstVisibleLine(HWND hWnd) */
#define HANDLE_EM_GETFIRSTVISIBLELINE(hWnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((hWnd))

/* int OnEMGetLine(HWND hWnd, int nLine, LPSTR lpszLine) */
#define HANDLE_EM_GETLINE(hWnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((hWnd), (int)(wParam), (LPSTR)(lParam))

/* UINT OnEMGetPasswordChar(HWND hWnd) */
#define HANDLE_EM_GETPASSWORDCHAR(hWnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)((hWnd))

/* int OnEMSetPasswordChar(HWND hWnd, UINT ch) */
#define HANDLE_EM_SETPASSWORDCHAR(hWnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((hWnd),(UINT)(wParam))

/* LRESULT OnConvert(HWND hwnd, WPARAM wParam, LPARAM lParam); */
#define HANDLE_WM_CONVERT(hwnd, wParam, lParam, fn) \
    (LRESULT)(fn)((hwnd), (wParam), (lParam))

/* void OnEMLimitText(HWND hWnd, UINT cchMax) */
#define HANDLE_EM_LIMITTEXT(hWnd, wParam, lParam, fn) \
    ((fn)((hWnd), (UINT)(wParam)), 0L)

/* EDITWORDBREAKPROC OnEMGetWordBreakProc(HWND hWnd) */
#define HANDLE_EM_GETWORDBREAKPROC(hWnd, wParam, lParam, fn) \
    (LRESULT)(fn)(hWnd)

/* void OnEMSetWordBreakProc(HWND hWnd, EDITWORDBREAKPROC ewbp) */
#define HANDLE_EM_SETWORDBREAKPROC(hWnd, wParam, lParam, fn) \
    ((fn)((hWnd), (EDITWORDBREAKPROC)(lParam)), 0L)

/* BOOL OnEMLineScroll(HWND hWnd, int dx, int dy) */
#define HANDLE_EM_LINESCROLL(hWnd, wParam, lParam, fn) \
    (LRESULT)(fn)((hWnd), (int)(short)HIWORD((lParam)), (int)(short)LOWORD((lParam)))

/* BOOL OnEMSetReadOnly(HWND hWnd, BOOL bReadOnly) */
#define HANDLE_EM_SETREADONLY(hWnd, wParam, lParam, fn) \
    (LRESULT)(fn)((hWnd), (BOOL)(short)(wParam))

/* void OnEMUndo(HWND hwnd) */
#define HANDLE_EM_UNDO(hWnd, wParam, lParam, fn) \
    ((fn)(hWnd), 0L)

/* BOOL OnEMCanUndo(HWND hWnd) */
#define HANDLE_EM_CANUNDO(hWnd, wParam, lParam, fn) \
    (LRESULT)(fn)(hWnd)

/* void OnEMEmptyUndoBuffer(HWND hWnd) */
#define HANDLE_EM_EMPTYUNDOBUFFER(hWnd, wParam, lParam, fn) \
    ((fn)(hWnd), 0L)

/* void OnEMSetRect(HWND hWnd, LPRECT lpRect) */
#define HANDLE_EM_SETRECT(hWnd, wParam, lParam, fn) \
    ((fn)((hWnd), (LPRECT)(lParam)), 0L)

/* void OnEMSetRectNP(HWND hWnd, LPRECT lpRect) */
#define HANDLE_EM_SETRECTNP(hWnd, wParam, lParam, fn) \
    ((fn)((hWnd), (LPRECT)(lParam)), 0L)

/* void OnEMGetRect(HWND hWnd, LPRECT lpRect) */
#define HANDLE_EM_GETRECT(hWnd, wParam, lParam, fn) \
    ((fn)((hWnd), (LPRECT)(lParam)), 0L)

/* BOOL OnEMSetTabStops(HWND hWnd, int nTabStops, LPINT pTabStops) */
#define HANDLE_EM_SETTABSTOPS(hWnd, wParam, lParam, fn) \
    (LRESULT)(fn)((hWnd), (int)(wParam), (LPINT)(lParam))

/* BOOL OnEMFmtLines(HWND hWnd, BOOL bFormat) */
#define HANDLE_EM_FMTLINES(hWnd, wParam, lParam, fn) \
    (LRESULT)(fn)((hWnd), (BOOL)(wParam))

#endif /* EDITX_H */
