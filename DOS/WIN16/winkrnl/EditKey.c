/* 
	EditKey.C
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


   HandleKey handles virtual key input for key with which we
   care about them when they go up and down.

 */

#include "windows.h"
#include "Edit.h"

/*
 * MoveTo: Set the cursor position without disturbing the anchor
 */
void MovePosTo(LPEDIT lp, int index, enum PREFER pref)
{
	SetEditPosition(lp, index, pref);
	UpdateHighlight(lp);
	UpdateMe(lp);
}

/*
 * SetAnchor: Set the starting selection position
 *
 * The selection is between lp->anchor and lp->lpidx, the
 * current cursor position.  Calling SetAnchor when an anchor
 * is already set will have no effect (you can't change a
 * selection's starting position).
 */
void SetAnchor(LPEDIT lp, int idx)
{
	lp->lastAnchor = lp->anchor ;
	lp->lastidx = lp->lpidx ;

	if (lp->anchor == no_anchor)
		lp->anchor = idx ;
}

/* 
 * ClearAnchor: Cancel the existing anchor
 */
void ClearAnchor(LPEDIT lp)
{
	lp->lastAnchor = lp->anchor ;
	lp->lastidx = lp->lpidx ;

	lp->anchor = no_anchor ;
}

/*
 * GetAnchor: Returns ordered start, end of anchor
 */
void GetAnchor(LPEDIT lp, int *beg, int *end)
{
	if (lp->anchor > lp->lpidx)
	{
		*beg = lp->lpidx ;
		*end = lp->anchor ;
	}
	else
	{
		*beg = lp->anchor ;
		*end = lp->lpidx ;
	}
}

/*
 * xyToHPos: Translates line, pos to horizontal pixel
 */
int xyToHPos(LPEDIT lp, int line, int pos)
{
	int HPos, nLineLen, n ;
	HDC hdc ;
	char *pstr ;

	hdc = GetDC(lp->hWnd) ;
	if (lp->hFont)
		SelectObject(hdc, lp->hFont) ;

	if (IsPassword(lp))
	{
	  	int i;
	  
		if (pos > lp->lpFragments[line].nLength)
			n = pos;
		else
			n = lp->lpFragments[line].nLength;
	
		pstr = WinMalloc(n + 1);
		for (i = 0; i < n; i++)
		  	pstr[i] = lp->PasswordChar;
		pstr[i] = '\0';
	}
	else
		pstr = StartOf(lp, line);
		

	HPos = LOWORD(GetTabbedTextExtent(hdc, pstr, 
					  pos, lp->cTabs, lp->pTabPos)) ;
	
	switch (lp->dwStyle & (ES_LEFT | ES_CENTER | ES_RIGHT))
	{
		case ES_RIGHT:
			nLineLen = LOWORD(GetTabbedTextExtent(hdc,
			      pstr, lp->lpFragments[line].nLength,
			      lp->cTabs, lp->pTabPos)) ;
			HPos += lp->rcClient.right /* lp->linelen */ - nLineLen ;
			break ;

		case ES_CENTER:
			nLineLen = LOWORD(GetTabbedTextExtent(hdc,
			      pstr, lp->lpFragments[line].nLength,
			      lp->cTabs, lp->pTabPos)) ;
			HPos += (lp->rcClient.right /* lp->linelen */ - nLineLen) / 2 ;
			break ;
	}
	ReleaseDC(lp->hWnd, hdc) ;
	if (IsPassword(lp))
	  	WinFree(pstr);
	return HPos ;
}

/*
 * HPosToX: Translates horizontal pixel to x pos
 */
int HPosToX(LPEDIT lp, int line, int HPos)
{
	HDC hdc ;
	int nLineLen, i ;

	hdc = GetDC(lp->hWnd) ;
	if (lp->hFont)
		SelectObject(hdc, lp->hFont) ;

	nLineLen = LOWORD(GetTabbedTextExtent(hdc, StartOf(lp, line), lp->lpFragments[line].nLength, lp->cTabs, lp->pTabPos)) ;
	switch (lp->dwStyle & (ES_LEFT | ES_CENTER | ES_RIGHT))
	{
		case ES_RIGHT:
			HPos -= lp->linelen - nLineLen ;
			break ;

		case ES_CENTER:
			HPos -= (lp->linelen - nLineLen) / 2 ;
			break ;
	}

	i = 0 ;
	while (LOWORD(GetTabbedTextExtent(hdc, StartOf(lp, line), i, lp->cTabs, lp->pTabPos)) < (WORD)HPos)
		if (i == lp->lpFragments[line].nLength)
			break ;
		else
			i++ ;

	ReleaseDC(lp->hWnd, hdc) ;
	return i ;
}

void HandleKey(HWND hWnd, UINT vk)
{
	LPEDIT lp ;

	if (!(lp = GetLPEdit(hWnd)))
		return;

	switch (vk)
	{
		case VK_SHIFT:
			lp->state |= ES_SHIFTDOWN;
			break;

		case VK_CONTROL:
			lp->state |= ES_CTRLDOWN;
			break;

		case VK_DELETE:
			HandleDelete(lp);
			break;

		case VK_INSERT:
			HandleInsert(lp);
			break;

	  	case VK_HOME:
			HandleHome(lp);
			break ;

	  	case VK_END:
			HandleEnd(lp);
			break ;

	  	case VK_LEFT:
			HandleLeft(lp);
			break;

	  	case VK_RIGHT:
			HandleRight(lp);
			break;

	  	case VK_UP:
			HandleUp(lp);
			break ;

	  	case VK_DOWN:
			HandleDown(lp);
			break ;

		case VK_NEXT:
			HandleNext(lp);
			break;

		case VK_PRIOR:
			HandlePrior(lp);
			break;

		default:
			break;
	}
}

void HandleDelete(LPEDIT lp)
{
	if ( lp->state & ES_SHIFTDOWN )
		SelectionToClipboard(lp->hWnd, lp);

        NotifyParent(lp, EN_UPDATE);
	if (!AnchorSet(lp)) {
	    if (lp->lpidx < lp->strlen)
		if (lp->npdata[lp->lpidx] == '\r')
		    DeleteChars(lp, lp->lpidx, lp->lpidx+2, TRUE);
		else
		    DeleteChars(lp, lp->lpidx, lp->lpidx+1, TRUE);
	}
	else
	{
		DeleteChars(lp, lp->anchor, lp->lpidx, TRUE);
		ClearAnchor(lp) ;
	}
	SetModified(lp, TRUE);
	DoCaretPos(lp, TRUE);
        UpdateMe(lp);
        NotifyParent(lp, EN_CHANGE);
}

void HandleInsert(LPEDIT lp)
{

	if (lp->state & ES_CTRLDOWN)		/* copy to clipboard */
		SelectionToClipboard(lp->hWnd, lp);
	else if (lp->state & ES_SHIFTDOWN)	/* insert from clipboard */
	    	SendMessage(lp->hWnd, WM_PASTE, 0, 0);
}

void HandleHome(LPEDIT lp)
{
	if (lp->state & ES_CTRLDOWN)
			/* CTRL + HOME: goto begin of display */
	{
		if (lp->state & ES_SHIFTDOWN)
				/* SHIFT+CTRL+HOME: select from anchor to the top */
			SetAnchor(lp, lp->lpidx) ;
		else
			ClearAnchor(lp) ;
		MovePosTo(lp, 0, any);
	}
	else
			/* HOME: goto begin of line */
	{
		if (lp->state & ES_SHIFTDOWN)
				/* SHIFT+HOME: select to line begin */
			SetAnchor(lp, lp->lpidx) ;
		else
			ClearAnchor(lp) ;
		MovePosTo(lp, CurFragment(lp)->nOffset, here);
				
	}
}

void HandleEnd(LPEDIT lp)
{
	int	strlen;

	if (lp->state & ES_CTRLDOWN)
		/* CTRL+END: goto end of display */
	{
		if (lp->state & ES_SHIFTDOWN)
			/* SHIFT+CTRL+END: select from anchor to the bottom */
			SetAnchor(lp, lp->lpidx);
		else
			ClearAnchor(lp);
		MovePosTo(lp, lp->strlen, any);
	}
	else
		/* END: goto of end of line */
	{
		strlen = CurFragment(lp)->nOffset + CurFragment(lp)->nLength;

		if (lp->state & ES_SHIFTDOWN)
			/* SHIFT+END: select to end of text */
			SetAnchor(lp, lp->lpidx);
		else
			ClearAnchor(lp);
		MovePosTo(lp, strlen, here);
	}
}

void HandleLeft(LPEDIT lp)
{
	if (lp->lpidx == 0)
		return ;

	if (lp->state & ES_SHIFTDOWN)
		SetAnchor(lp, lp->lpidx) ;
	else
		ClearAnchor(lp) ;

	if (lp->state & ES_CTRLDOWN)
		MovePosTo(lp, lp->WordBreakProc(lp->npdata, lp->lpidx, lp->strlen, WB_LEFT), prev) ;
	else
		MovePosTo(lp, --lp->lpidx, prev) ;
}

void HandleRight(LPEDIT lp)
{
	if (lp->lpidx == lp->strlen)
		return ;

	if (lp->state & ES_SHIFTDOWN)
		SetAnchor(lp, lp->lpidx) ;
	else
		ClearAnchor(lp) ;

	if (lp->state & ES_CTRLDOWN)
		MovePosTo(lp, lp->WordBreakProc(lp->npdata, lp->lpidx, lp->strlen, WB_RIGHT), next) ;
	else
		MovePosTo(lp, ++lp->lpidx, next);
}

void HandleUp(LPEDIT lp)
{
	int x, y, HPos ;
	
	if (lp->nCurFragment == 0 || lp->lpidx == 0)
		return;

	if (lp->state & ES_SHIFTDOWN)
		SetAnchor(lp, lp->lpidx) ;
	else
		ClearAnchor(lp) ;

	y = FindLine(lp, lp->lpidx) ;
	x = lp->lpidx - lp->lpFragments[y].nOffset ;
	HPos = xyToHPos(lp, y, x) ;
	x = HPosToX(lp, --y, HPos) ;
	MovePosTo(lp, lp->lpFragments[y].nOffset + x, other) ;
}

void HandleDown(LPEDIT lp)
{
	int x, y, HPos ;

	if (lp->nCurFragment == lp->nFragments - 1 || lp->lpidx == lp->strlen - 1)
		return;		

	if (lp->state & ES_SHIFTDOWN)
		SetAnchor(lp, lp->lpidx) ;
	else
		ClearAnchor(lp) ;

	y = FindLine(lp, lp->lpidx) ;
	x = lp->lpidx - lp->lpFragments[y].nOffset ;
	HPos = xyToHPos(lp, y, x) ;
	x = HPosToX(lp, ++y, HPos) ;
	MovePosTo(lp, lp->lpFragments[y].nOffset + x, other) ;
}

void HandleNext(LPEDIT lp)
{
	int x, y, HPos ;

	if (lp->state & ES_SHIFTDOWN)
		SetAnchor(lp, lp->lpidx) ;
	else
		ClearAnchor(lp) ;

	y = FindLine(lp, lp->lpidx) ;
	x = lp->lpidx - lp->lpFragments[y].nOffset ;
	HPos = xyToHPos(lp, y, x) ;
	y += lp->VisLines ;
	if (y >= lp->nFragments)
		y = lp->nFragments - 1 ;
	x = HPosToX(lp, y, HPos) ;
	MovePosTo(lp, lp->lpFragments[y].nOffset + x, other) ;
}

void HandlePrior(LPEDIT lp)
{
	int x, y, HPos ;

	if (lp->state & ES_SHIFTDOWN)
		SetAnchor(lp, lp->lpidx) ;
	else
		ClearAnchor(lp) ;

	y = FindLine(lp, lp->lpidx) ;
	x = lp->lpidx - lp->lpFragments[y].nOffset ;
	HPos = xyToHPos(lp, y, x) ;
	y -= lp->VisLines ;
	if (y < 0)
		y = 0 ;
	x = HPosToX(lp, y, HPos) ;
	MovePosTo(lp, lp->lpFragments[y].nOffset + x, other) ;
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
