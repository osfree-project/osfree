/*    
	EditDraw.c	1.6
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
#include "Edit.h"
#include "Log.h"
#include "kerndef.h"

void
PaintEditControl(LPEDIT lp)
{
	HWND hParent ;
	PAINTSTRUCT ps ;
	RECT rect ;
	int i, nFirst, nLast ;

	HideCaret(lp->hWnd) ;
	BeginPaint(lp->hWnd, &ps) ;

	SetBkMode(ps.hdc, OPAQUE);
	SetBkColor(ps.hdc, GetSysColor(COLOR_WINDOW));
	SetTextColor(ps.hdc, GetSysColor(COLOR_WINDOWTEXT));
	hParent = GetParent(lp->hWnd);

	lp->hBr = GetStockObject(BLACK_BRUSH) ;

	if (hParent)
		lp->hBr = (HBRUSH)SendMessage(hParent,
			GET_WM_CTLCOLOR_MSG(CTLCOLOR_EDIT),
			GET_WM_CTLCOLOR_MPS(ps.hdc,lp->hWnd, CTLCOLOR_EDIT));

	if (lp->hFont) 
	        SelectObject(ps.hdc, lp->hFont);

	if (lp->dwStyle & WS_BORDER)
	{
		GetClientRect(lp->hWnd, &rect) ;
		FrameRect(ps.hdc, &rect, lp->hBr) ;
	}

	CopyRect(&rect, &ps.rcPaint) ;
	WindowToClientRect(lp, &rect) ;

	nFirst = rect.top / lp->lineheight ;
	nLast = rect.bottom / lp->lineheight ;
	if (nLast >= lp->nFragments)
		nLast = lp->nFragments - 1;

	for (i = nFirst ; i <= nLast ; i++)
		DrawLineAt(lp, ps.hdc, i) ;

	EndPaint(lp->hWnd, &ps) ;
	ShowCaret(lp->hWnd) ;
}

/*
 * InvalidateClientRect: invalidates the given rectangle
 *      in our client area (only)
 *
 * Since we use CS_PARENTDC, we must make sure that all
 * drawing is only done in our own client area.  Hence the
 * use of a clipping region in DrawLineAt.  For politeness, we
 * shouldn't invalidate our parent's window, either...
 */
void InvalidateClientRect(LPEDIT lp, RECT *rcInvalid)
{
	RECT rc ;
	CopyRect(&rc, rcInvalid) ;
	ClientToWindowRect(lp, &rc) ;
	IntersectRect(&rc, &rc, &lp->rcClient) ;
	InvalidateRect(lp->hWnd, &rc, TRUE) ;
}
 
/*
 * DrawLineAt: Draw a line of text on the given DC
 *
 * DrawLineAt can draw five kinds of lines:
 *  (1) ___________________  No anchor
 *  (2) ___________********  Anchor begins
 *  (3) *******************  All anchor
 *  (4) *******____________  Anchor ends
 *
 *  (5) _______******______  Anchor starts _and_ ends
 *
 * To accomodate this, it knows of three "segments" of text, one starting
 * at zero and two more for the possible anchored/unanchored segments.
 * For each segment, it alternates the anchor flag, sets the text and
 * background colors, and draws text.
 */
void
DrawLineAt(LPEDIT lp, HDC hDC, int LineIndex)
{
	POINT	pt;
	int	i; 
	int	TextWidth; 
	int	begspot; 
	int	endspot;
	int	nSegment[3] ;	/* Cumulative length to anchor transitions */
	int	nSegLen[3] ;	/* Length of the pertinent segment */
	int	nMaxSegment = 0 ;
	BOOL	bAnchor ;
	COLORREF	crText, crBack ;
	HRGN	hrgnClip ;
	int LineLength = lp->lpFragments[LineIndex].nLength;
	int LineOffset = lp->lpFragments[LineIndex].nOffset;
	char *pData ;

	nSegment[0] = 0 ;	/* Always start at 1st char in line */
	bAnchor = FALSE ;	/* By default; see next section */
	crText = GetTextColor(hDC) ;
	crBack = GetBkColor(hDC) ;

		/* Set up to draw anchored region, if necesary */
	if (AnchorSet(lp) && (HasFocus(lp) || NoHideSel(lp)))
	{
			/* Find ordered start, end of anchor area */
		GetAnchor(lp, &begspot, &endspot) ;

			/* Does this line start anchored? */
		bAnchor = IndexBeforeLine(lp, begspot, LineIndex) && !IndexBeforeLine(lp, endspot, LineIndex) ;

			/* Does the anchor begin on this line? */
		if (IndexOnLine(lp, begspot, LineIndex))
			nSegment[++nMaxSegment] = begspot - LineOffset ;

			/* Does the anchor end on this line? */
		if (IndexOnLine(lp, endspot, LineIndex))
			nSegment[++nMaxSegment] = endspot - LineOffset ;
	}

		/* Set up segment lengths */
	for (i = 0 ; i < nMaxSegment ; i++)
		nSegLen[i] = nSegment[i+1] - nSegment[i] ;
	nSegLen[nMaxSegment] = LineLength - nSegment[nMaxSegment] ;

		/* Get text width */
	TextWidth = LOWORD(GetTabbedTextExtent(hDC,
		&lp->npdata[LineOffset], LineLength, lp->cTabs, lp->pTabPos));

	pt.x = xyToHPos(lp, LineIndex, 0) ;
	pt.y = LineIndex * lp->lineheight;
	ClientToWindowPoint(lp, &pt);

	if (IsPassword(lp))
	{
		pData = WinMalloc(LineLength) ;
		for (i = 0 ; i < LineLength ; i++)
			pData[i] = lp->PasswordChar ;
	}
	else
		pData = &lp->npdata[LineOffset] ;

	hrgnClip = CreateRectRgnIndirect(&lp->rcClient) ;
	SelectClipRgn(hDC, hrgnClip) ;
	DeleteObject(hrgnClip) ;

	for (i = 0 ; i <= nMaxSegment ; i++)
	{
		if (bAnchor)
		{
			SetBkColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
			SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
		}
		else
		{
			SetBkColor(hDC, crBack) ;
			SetTextColor(hDC, crText) ;
		}
		TabbedTextOut(hDC, pt.x, pt.y, pData + nSegment[i], nSegLen[i], lp->cTabs, lp->pTabPos, lp->rcClient.left) ;
		pt.x += LOWORD(GetTabbedTextExtent(hDC, pData + nSegment[i], nSegLen[i], lp->cTabs, lp->pTabPos)) ;
		bAnchor = !bAnchor ;
	}

	if (IsPassword(lp))
		WinFree(pData) ;

	SetBkColor(hDC, crBack) ;
	SetTextColor(hDC, crText) ;
}

/*
 * InvalidateLine: Marks the specified line for redrawing
 */
void InvalidateLine(LPEDIT lp, int nLine)
{
	RECT rc ;
	rc.left = lp->ptOffset.x ;
	rc.right = rc.left + Width(lp) ;
	rc.top = nLine * lp->lineheight ;
	rc.bottom = rc.top + lp->lineheight ;

	InvalidateClientRect(lp, &rc) ;
}

/*
 * UpdateMe: Do the actual updating, set the caret position, etc.
 */
void UpdateMe(LPEDIT lp)
{
	HideCaret(lp->hWnd) ;
	ScrollToHere(lp) ;
	UpdateWindow(lp->hWnd) ;
	DoCaretPos(lp, TRUE) ;
	ShowCaret(lp->hWnd) ;
}

/*
 * UpdateHighlight: Invalidates the selected text
 */
void UpdateHighlight(LPEDIT lp)
{
	int idxStart, idxEnd ;

	if (AnchorSet(lp) || lp->lastAnchor != no_anchor)
	{
		idxStart = lp->lpidx ;
		idxEnd = lp->lpidx ;
		if (lp->lastidx < idxStart)		idxStart = lp->lastidx ;
		if (lp->lastidx > idxEnd)		idxEnd   = lp->lastidx ;
		UpdateRange(lp, idxStart, idxEnd) ;

		idxEnd = idxStart = AnchorSet(lp) ? lp->anchor : lp->lastidx ;
		if (lp->lastAnchor == no_anchor)
			lp->lastAnchor = lp->lpidx ;
		if (lp->lastAnchor < idxStart)		idxStart = lp->lastAnchor ;
		if (lp->lastAnchor > idxEnd)		idxEnd   = lp->lastAnchor ;
		UpdateRange(lp, idxStart, idxEnd) ;
	}

	lp->lastAnchor = lp->anchor ;
	lp->lastidx = lp->lpidx ;
}

void UpdateAnchor(LPEDIT lp)
{
	int idxStart, idxEnd ;
	if (!AnchorSet(lp))
		return ;
	GetAnchor(lp, &idxStart, &idxEnd) ;
	UpdateRange(lp, idxStart, idxEnd) ;
}

void UpdateRange(LPEDIT lp, int idxStart, int idxEnd)
{
	int lineStart, lineEnd ;
	int i ;
	RECT rc ;

	if (idxStart == idxEnd)
		return ;		/* Nothing to do */

	lineStart = FindLine(lp, idxStart) ;
	lineEnd = FindLine(lp, idxEnd) ;

	if (lineStart == lineEnd)
	{
		rc.left = xyToHPos(lp, lineStart, idxStart - lp->lpFragments[lineStart].nOffset) ;
		rc.right = xyToHPos(lp, lineEnd, idxEnd - lp->lpFragments[lineEnd].nOffset) + 1;
		rc.top = lineStart * lp->lineheight ;
		rc.bottom = rc.top + lp->lineheight + 1 ;
		InvalidateClientRect(lp, &rc) ;
	}
	else
	{
			/* Update everything from the anchor start to EOL */
		rc.left = xyToHPos(lp, lineStart, idxStart - lp->lpFragments[lineStart].nOffset) ;
		rc.right = lp->linelen ;
		rc.top = lineStart * lp->lineheight ;
		rc.bottom = rc.top + lp->lineheight + 1;
		InvalidateClientRect(lp, &rc) ;

			/* Update everything from start of line to anchor end */
		rc.left = 0 ;
		rc.right = xyToHPos(lp, lineEnd, idxEnd - lp->lpFragments[lineEnd].nOffset) + 1;
		rc.top = lineEnd * lp->lineheight ;
		rc.bottom = rc.top + lp->lineheight + 1 ;
		InvalidateClientRect(lp, &rc) ;

		for (i = lineStart + 1 ; i < lineEnd ; i++)
			InvalidateLine(lp, i) ;
	}
}

/*
 * DoCaretPos: Position the caret on-screen and scroll so it is visible
 */

void DoCaretPos(LPEDIT lp, BOOL bScroll)
{
	POINT pt;
		
	/*
	 * We only do caret stuff when we have focus and
	 * when we know how big the font is.
	 */
	if (!HasFocus(lp) || !lp->lineheight)
		return;

	/* check to see for vertical scrolling calcs */
	if (bScroll)
		ScrollToHere(lp) ;

	pt.y = lp->nCurFragment * lp->lineheight - lp->externalLeading ;
	pt.x = xyToHPos(lp, lp->nCurFragment, lp->lpidx - CurFragment(lp)->nOffset) ;

	/* If we haven't already created the cursor, do it now */
	if (!(lp->state & ES_CARET))
	{
		CreateCaret(lp->hWnd, (int) NULL, (int) NULL, lp->lineheight);
		lp->state |= ES_CARET;
	}

	ClientToWindowPoint(lp, &pt);
	SetCaretPos(pt.x, pt.y);
}

/*
 * VScroll: Scrolls the text vertically, nPixels down (or up, if negative)
 */
void VScroll(LPEDIT lp, int nLines)
{
	if (!Multiline(lp))
		return ;		/* Can't scroll a single-line edit */

	if (!lp->nFragments)
		return ;		/* Can't scroll no lines */

	if (lp->nFirstFragment + nLines > lp->nFragments)
		nLines = lp->nFragments - lp->nFirstFragment ;
	if (lp->nFirstFragment < -nLines)
		nLines = -lp->nFirstFragment ;
	lp->nFirstFragment += nLines ;
	UpdateWindow(lp->hWnd) ;
	lp->ptOffset.y = lp->nFirstFragment * lp->lineheight ;
	if (HasVScroll(lp))
		SetScrollPos(lp->hWnd, SB_VERT, (lp->nFirstFragment * 100L) / lp->nFragments, TRUE) ;
	NotifyParent(lp, EN_VSCROLL) ;
	HideCaret(lp->hWnd) ;
	ScrollWindow(lp->hWnd, 0, -nLines * lp->lineheight, &lp->rcClient, &lp->rcClient) ;
	DoCaretPos(lp, FALSE) ;
	ShowCaret(lp->hWnd) ;
}

/*
 * ScrollToLine: scrolls the window so the specified line is visible
 */
void ScrollToLine(LPEDIT lp, int lineno)
{
	int nMaxVisible;

		/* Exit if there's no scroll or we're single-line */
	if (!AutoVScroll(lp) || !Multiline(lp))
		return ;

		/* Scroll up if we need to */
	if (lineno < lp->nFirstFragment)
		VScroll(lp, lineno - lp->nFirstFragment) ;

		/* Scroll down if we need to */
	nMaxVisible = lp->nFirstFragment + lp->VisLines - 1 ;
	if (lineno + 1 > nMaxVisible)
		VScroll(lp, lineno + 1 - nMaxVisible) ;
}

/*
 * HScroll: Scrolls horizontally, nPixels right (of left, if negative)
 */
void HScroll(LPEDIT lp, int nPixels)
{
	if (!lp->linelen)
		return;			/* Can't h-scroll w/o line len */

	if (lp->ptOffset.x + nPixels > lp->linelen)
		nPixels = lp->linelen - lp->ptOffset.x ;
	if (lp->ptOffset.x < -nPixels)
		nPixels = -lp->ptOffset.x ;
	UpdateWindow(lp->hWnd) ;
	lp->ptOffset.x += nPixels ;
	if (HasHScroll(lp))
		SetScrollPos(lp->hWnd, SB_HORZ, (lp->ptOffset.x * 100L) / lp->linelen, TRUE) ;
	NotifyParent(lp, EN_HSCROLL) ;
	HideCaret(lp->hWnd) ;
	ScrollWindow(lp->hWnd, -nPixels, 0, &lp->rcClient, &lp->rcClient) ;
	DoCaretPos(lp, FALSE) ;
	ShowCaret(lp->hWnd) ;
}

/*
 * ScrollToHPos: scrolls the window so the specified pixel position is visible
 */
void ScrollToHPos(LPEDIT lp, int hPos)
{
	int nMaxVisible ;

		/* Exit if we can't horizontal scroll */
	if (!AutoHScroll(lp))
		return ;

	nMaxVisible = lp->ptOffset.x + Width(lp) ;

		/* Scroll left if we need to */
	if (hPos < lp->ptOffset.x)
	{
		if (10 * lp->aveWidth < Width(lp))
			hPos -= 10 * lp->aveWidth ;		/* SDK docs say 10 char auto-scroll */
		else
			hPos -= Width(lp) ;		/* But keep cursor on screen no matter what */
		if (hPos < 0)
			hPos = 0 ;
		HScroll(lp, hPos - lp->ptOffset.x) ;
	}

		/* Scroll right if we need to */
	if (hPos > nMaxVisible)
	{
		if (10 * lp->aveWidth < Width(lp))
			hPos += 10 * lp->aveWidth ;
		else
			hPos += Width(lp) ;
		HScroll(lp, hPos - nMaxVisible) ;
	}
}

/*
 * HPosAt: returns the pixel position of a character
 */
int HPosAt(LPEDIT lp, int line, int idx)
{
	HDC hdc ;
	int nReturn ;

	hdc = GetDC(lp->hWnd) ;
	if (lp->hFont)
		SelectObject(hdc, lp->hFont) ;
	if (IsPassword(lp))
	{
	    	char *pstr;
		int i;
		
		pstr = WinMalloc(idx + 1);
		for (i = 0; i < idx; i++)
		  	pstr[i] = lp->PasswordChar;
		
		pstr[i] = '\0';
		nReturn = LOWORD(GetTabbedTextExtent(hdc, pstr,
						     idx, lp->cTabs, 
						     lp->pTabPos)) ;
	  	WinFree(pstr);
	}
	else
	{
		nReturn = LOWORD(GetTabbedTextExtent(hdc, StartOf(lp, line), 
						     idx, lp->cTabs, 
						     lp->pTabPos)) ;
	}
	
	ReleaseDC(lp->hWnd, hdc) ;
	return nReturn ;
}

/*
 * FindLine: returns line number of index
 */
int FindLine(LPEDIT lp, int idx)
{
	register int i ;

	for (i = 0 ; i < lp->nFragments && lp->lpFragments[i].nOffset <= idx ; i++) ;
	return --i ;
}

/*
 * IndexBeforeLine: Is the specified index on a line
 *  before the specified one?
 */
BOOL IndexBeforeLine(LPEDIT lp, int idx, int line)
{
	return idx < lp->lpFragments[line].nOffset ;
}

/*
 * IndexOnLine: Is the specified index on the specified line?
 */
BOOL IndexOnLine(LPEDIT lp, int idx, int line)
{
	return ((idx >= lp->lpFragments[line].nOffset) &&
		(idx <= lp->lpFragments[line].nOffset + lp->lpFragments[line].nLength)) ;
}

/*
 * IndexAfterLine: Is the specified index on a later line than
 *  that specified?
 */
BOOL IndexAfterLine(LPEDIT lp, int idx, int line)
{
	return idx > lp->lpFragments[line].nOffset + lp->lpFragments[line].nLength ;
}

/*
    Given a position into the text buffer, update the
	lpidx (position) and nCurFragment (line)
*/
void SetEditPosition(LPEDIT lp, int position, enum PREFER pref)
{
	int nOldFragment = lp->nCurFragment ;

	if ( position < 0 )		/* Start of buffer is 0 */
	{
		position = 0;
	}

	if ( position > lp->strlen )	/* End of buffer is 'strlen' */
	{
		position = lp->strlen;
	}

	lp->lpidx = position ;
	lp->nCurFragment = FindLine(lp, lp->lpidx) ;

	if (pref == here)
		if (lp->nCurFragment < nOldFragment)
			pref = next ;
		else
			pref = prev ;

	if (pref == other)
		if (lp->nCurFragment < nOldFragment)
			pref = prev ;
		else
			pref = next ;

	if (CurFragment(lp)->nLength &&
	    position - CurFragment(lp)->nOffset > CurFragment(lp)->nLength)
		switch (pref)
		{
			case next:
				lp->lpidx = lp->lpFragments[++lp->nCurFragment].nOffset ;
				break ;

			case prev:
			case any:
			default:
				lp->lpidx = CurFragment(lp)->nOffset +
					CurFragment(lp)->nLength ;
				break ;
		}
}

/*
 * Local Variables:
 * c-indent-level:8
 * c-continued-statement-offset:8
 * c-continued-brace-offset:0
 * c-brace-imaginary-offset:0
 * c-brace-offset:-8
 * c-label-offset:-8
 * End:
 */
