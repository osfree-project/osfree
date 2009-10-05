/*    
	EditMisc.c	1.8
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

EditMisc.c: Miscellaneous functions

This file includes all of the inlines from Edit.h, plus EditClip.c,
EditClient.c, and EditRRN.c.
*/


 
#include "windows.h"
#include "windowsx.h"
#include "Edit.h"
#include <string.h>


LPEDIT GetLPEdit(HWND hWnd)
		{ return (LPEDIT)GetWindowLong(hWnd, SWD_LPSTR) ; }

void NotifyParent(LPEDIT lp, UINT msg)
{
	register HWND hwndParent ;
	if ((hwndParent = GetParent(lp->hWnd)))
		SendMessage(hwndParent, WM_COMMAND,
			GET_WM_COMMAND_MPS(lp->lpid,lp->hWnd,msg)) ;
}

int HPosAtIndex(LPEDIT lp, int idx)
{
	register int line = FindLine(lp, idx) ;
	return HPosAt(lp, line, idx - lp->lpFragments[line].nOffset) ;
}

int HPosHere(LPEDIT lp)
	{ return HPosAtIndex(lp, lp->lpidx) ; }

void ScrollToHere(LPEDIT lp)
{
	ScrollToLine(lp, lp->nCurFragment) ;
	ScrollToHPos(lp, HPosHere(lp)) ;
}

BOOL AnchorSet(LPEDIT lp)
	{ return lp->anchor != no_anchor ; }

BOOL HasFocus(LPEDIT lp)
	{ return lp->state & ES_FOCUS ? TRUE : FALSE ; }
BOOL FormatLines(LPEDIT lp)
	{ return lp->state & ES_FORMATLINE ? TRUE : FALSE ; }
BOOL IsModified(LPEDIT lp)
	{ return lp->state & ES_MODIFIED ? TRUE : FALSE ; }

BOOL HasVScroll(LPEDIT lp)
	{ return lp->dwStyle & WS_VSCROLL ? TRUE : FALSE ; }
BOOL AutoVScroll(LPEDIT lp)
	{ return lp->dwStyle & ES_AUTOVSCROLL ? TRUE : FALSE ; }
BOOL HasHScroll(LPEDIT lp)
	{ return lp->dwStyle & WS_HSCROLL ? TRUE : FALSE ; }
BOOL AutoHScroll(LPEDIT lp)
	{ return lp->dwStyle & ES_AUTOHSCROLL ? TRUE : FALSE ; }
BOOL Multiline(LPEDIT lp)
	{ return lp->dwStyle & ES_MULTILINE ? TRUE : FALSE ; }
BOOL NoHideSel(LPEDIT lp)
	{ return lp->dwStyle & ES_NOHIDESEL ? TRUE : FALSE ; }
BOOL IsReadOnly(LPEDIT lp)
	{ return lp->dwStyle & ES_READONLY ? TRUE : FALSE ; }
BOOL IsPassword(LPEDIT lp)
	{ return lp->dwStyle & ES_PASSWORD ? TRUE : FALSE ; }

void SetReadOnly(LPEDIT lp, BOOL b)
{
	if (b)
		lp->dwStyle |= ES_READONLY ;
	else
		lp->dwStyle &= ~(ES_READONLY) ;
}

void SetModified(LPEDIT lp, BOOL b)
{
	if (b)
		lp->state |= ES_MODIFIED ;
	else
		lp->state &= ~ES_MODIFIED ;
}

int Width(LPEDIT lp)
	{ return lp->rcClient.right - lp->rcClient.left ; }
int Height(LPEDIT lp)
	{ return lp->rcClient.bottom - lp->rcClient.top ; }

FRAGMENT *CurFragment(LPEDIT lp)
	{ return &lp->lpFragments[lp->nCurFragment] ; }
char   *StartOf(LPEDIT lp, int nFrag)
	{ return &lp->npdata[lp->lpFragments[nFrag].nOffset] ; }

BOOL CanUndo(LPEDIT lp)
	{ return lp->pUndoBuffer != NULL ; }

void ClearUndoBuffer(LPEDIT lp)
{
	if (lp->pUndoBuffer) {
	    if (lp->pUndoBuffer->pDelChars)
		WinFree(lp->pUndoBuffer->pDelChars) ;
	    WinFree((LPVOID)lp->pUndoBuffer) ;
	    lp->pUndoBuffer = NULL ;
	}
}

/*
 * ClientToWindow: client area coordinate translation
 *
 * These two functions translate a point or rectangle
 * by subtracting the offset from scrolling
 * (EDIT::ptOffset) and adding the client area offset
 * (EDIT::rcClient.top, EDIT::rcClient.left).
 */

void ClientToWindowPoint(LPEDIT lp, POINT *pt)
{
	pt->x -= lp->ptOffset.x ;
	pt->y -= lp->ptOffset.y ;

	pt->x += lp->rcClient.left ;
	pt->y += lp->rcClient.top ;
}

void ClientToWindowRect(LPEDIT lp, RECT *rc)
{
	rc->top    -= lp->ptOffset.y ;
	rc->left   -= lp->ptOffset.x ;
	rc->bottom -= lp->ptOffset.y ;
	rc->right  -= lp->ptOffset.x ;

	rc->top    += lp->rcClient.top ;
	rc->left   += lp->rcClient.left ;
	rc->bottom += lp->rcClient.top ;
	rc->right  += lp->rcClient.left ;
}

/*
 * WindowToClient: window coordinate translation
 *
 * These two functions translate a window coordinate
 * to client area coordinates by performing the
 * opposite operations as above.
 *
 * WindowToClient(ClientToWindow(pt)) == pt
 *
 */

void WindowToClientPoint(LPEDIT lp, POINT *pt)
{
	pt->x += lp->ptOffset.x ;
	pt->y += lp->ptOffset.y ;

	pt->x -= lp->rcClient.left ;
	pt->y -= lp->rcClient.top ;
}

void WindowToClientRect(LPEDIT lp, RECT *rc)
{
	rc->top    += lp->ptOffset.y ;
	rc->left   += lp->ptOffset.x ;
	rc->bottom += lp->ptOffset.y ;
	rc->right  += lp->ptOffset.x ;

	rc->top    -= lp->rcClient.top ;
	rc->left   -= lp->rcClient.left ;
	rc->bottom -= lp->rcClient.top ;
	rc->right  -= lp->rcClient.left ;
}

/*
 * Clipboard functions: a function to copy text TO the clipboard and a
 *  function to copy text FROM the clipboard
 */

void
SelectionToClipboard(HWND hWnd,LPEDIT lp)
{
	HGLOBAL hText;
	LPSTR 	lpText;
	int	TextSize;
	int	BegSpot;
	int	EndSpot;

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

	if (OpenClipboard(hWnd))
	{
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hText);
		CloseClipboard();
	}

}

void
ClipboardToSelection(HWND hWnd,LPEDIT lp)
{
	HGLOBAL hCbData;
	LPSTR 	lpCbData;

	if (!OpenClipboard(hWnd))
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
						
	InsertChars(lp, lpCbData, lstrlen(lpCbData), TRUE);

	CloseClipboard();
}

/*

Windows supports the EM_FMTLINES message, which causes soft line breaks
to be inserted in the text buffer where word wrap occurs.  This is only
a problem when (1) we have been send EM_FMTLINES(TRUE), and (2) we do not
have the ES_AUTOHSCROLL style, and consequently have word breaks.

Since we must also support this functionality, the following functions are
provided to support soft line breaks.

Comment: Since the soft line break characters are \r\r\n (two carraige
returns and a line feed), I've notated these functions ...RRN to
distinguish them from the "normal" edit control behavior.

*/

/*
 * EDIT::InsertRRN(): Insert soft line breaks (\r\r\n)
 *
 * This function calls ReformatFragments() to ensure that the fragment table
 * contains valid lines, and then inserts soft line-break characters.
 *
 * Algorithm:
 *
 * 1. ReformatFragments(), setting nFragments and rebuilding the fragment
 *       table.
 * 2. Count soft line breaks.  For each fragment, if it does _not_ end in
 *       \n, it is a soft line break.  (Un*x systems end lines in \n, DOS in
 *       \r\n.)
 * 3. Reallocate the memory buffer.  Since we now know how many soft line
 *       breaks there are, and each one has 3 characters, the new memory
 *       size must be strlen+3*nRRN bytes.
 * 4. Insert soft line breaks.  Starting at the end (since we know how long
 *       the buffer is and how many soft line breaks there are), move some
 *       bytes, insert a line break, move some bytes...
 */

void InsertRRN(LPEDIT lp)
{
	int nRRN, nFrag, nOffset, nPos ;

	if (AutoHScroll(lp))
		return ;	/* Don't word wrap if we auto-h-scroll */

	ReformatFragments(lp) ;

	for (nRRN = nFrag = 0 ; nFrag < lp->nFragments ; nFrag++)
	{
		if (!lp->lpFragments[nFrag].nLength)
			continue ;
		if (lp->npdata[lp->lpFragments[nFrag].nOffset +
			   lp->lpFragments[nFrag].nLength] != '\n' &&
		   (lp->npdata[lp->lpFragments[nFrag].nOffset +
			   lp->lpFragments[nFrag].nLength] != '\r' ||
		    lp->npdata[lp->lpFragments[nFrag].nOffset +
		           lp->lpFragments[nFrag].nLength + 1] != '\n'))
			nRRN++ ;
	}

	nPos = lp->strlen - 1 ;
	lp->strlen += 3 * nRRN ;

	if (lp->strlen > lp->memlen)
	{
		while (lp->strlen > lp->memlen)
			lp->memlen += 0x20 ;
		EditMemoryAPI(lp->hWnd, EMA_UNLOCK, lp->hData, 0L) ;
		lp->hData = (HANDLE)EditMemoryAPI(lp->hWnd, EMA_REALLOC,
						lp->hData, lp->memlen) ;
		lp->npdata = (LPSTR)EditMemoryAPI(lp->hWnd, EMA_LOCK, lp->hData, 0L) ;
	}

	/* nOffset will hold the character after this line in the buffer */
	nOffset = lp->strlen - 1 ;
	for (nFrag = lp->nFragments - 1 ; nFrag >= 0 ; nFrag--)
	{
		if (nOffset <= 0)
			continue ; 
		if (lp->npdata[nPos] != '\n')
		{
			lp->npdata[nOffset--] = '\n' ;
			lp->npdata[nOffset--] = '\r' ;
			lp->npdata[nOffset--] = '\r' ;
			if (lp->nCurFragment > nFrag)
				lp->lpidx += 3 ;
			if (FindLine(lp, lp->anchor) > nFrag)
				lp->anchor += 3 ;
		}
		while(nPos >= lp->lpFragments[nFrag].nOffset)
			lp->npdata[nOffset--] = lp->npdata[nPos--] ; 
		lp->lpFragments[nFrag].nOffset = nOffset + 1 ;
	}
	lp->state |= ES_FORMATLINE ;
}

/*
 * EDIT::DeleteRRN(): Delete the soft line breaks
 *
 * This function calls ReformatFragments(), and rebuilds the buffer without
 * the soft line breaks.
 *
 * Algorithm: Same as InsertRRN().  This time, we don't need to realloc, and
 * we work forwards instead of backwards.
 */

void DeleteRRN(LPEDIT lp)
{
	int nFrag, nOffset, nPos ;

	ReformatFragments(lp) ;

	/* nOffset points to the beginning of the current line to be read */
	nPos = nOffset = 0 ;
	while (nOffset < lp->strlen)
	{
		if (lp->npdata[nOffset] == '\r' &&
		    lp->npdata[nOffset + 1] == '\r' &&
		    lp->npdata[nOffset + 2] == '\n')
		{
			/* Skip the soft line break; update frag table */
			nOffset += 3 ;
			lp->strlen -= 3 ;
			nFrag = FindLine(lp, nOffset) ;
			if (lp->nCurFragment >= nFrag)
				lp->lpidx -= 3 ;
			if (FindLine(lp, lp->anchor) >= nFrag)
				lp->anchor -= 3 ;
			while (nFrag < lp->nFragments)
				lp->lpFragments[nFrag++].nOffset -= 3 ;
		}
		else
			lp->npdata[nPos++] = lp->npdata[nOffset++] ;
	}
	lp->state &= ~ES_FORMATLINE ;
}
