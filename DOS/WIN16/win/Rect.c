/*    
	Rect.c	2.5
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
#include "Log.h"

BOOL WINAPI
EqualRect(const RECT *r1, const RECT *r2)
{
	return ((r1->left   == r2->left) &&
		(r1->top    == r2->top)  &&
		(r1->right  == r2->right) &&
		(r1->bottom == r2->bottom));

}

void WINAPI
CopyRect(LPRECT d, const RECT *s)
{
	d->left 	= s->left;
	d->top 		= s->top;
	d->right 	= s->right;
	d->bottom 	= s->bottom;
	return;
}

void WINAPI
InflateRect(LPRECT r, int x, int y)
{
	r->left   -= x;
	r->right  += x;
	r->top    -= y;
	r->bottom += y;
}

void WINAPI
OffsetRect(LPRECT r, int x, int y)
{
	r->left   += x;
	r->right  += x;
	r->top    += y;
	r->bottom += y;
}

BOOL WINAPI
PtInRect(const RECT *lpr,POINT pt)
{
    BOOL rc;

    rc = ((lpr->left <= pt.x) && (pt.x < lpr->right) &&
	  (lpr->top  <= pt.y) && (pt.y < lpr->bottom));

    return rc;
}

BOOL WINAPI
IntersectRect(LPRECT lpDestRect, const RECT *lpSrc1Rect,
	      const RECT *lpSrc2Rect)
{
    lpDestRect->left = max(lpSrc1Rect->left, lpSrc2Rect->left);
    lpDestRect->top = max(lpSrc1Rect->top, lpSrc2Rect->top);
    lpDestRect->right = min(lpSrc1Rect->right, lpSrc2Rect->right);
    lpDestRect->bottom = min(lpSrc1Rect->bottom, lpSrc2Rect->bottom);

    if ((lpDestRect->right <= lpDestRect->left) ||
	(lpDestRect->bottom <= lpDestRect->top)) {
	SetRectEmpty(lpDestRect);
	return FALSE;
    }
    else
	return TRUE;
}
 
BOOL    WINAPI 
SubtractRect(RECT FAR* lpDestRect, const RECT FAR*lprc1, const RECT FAR*lprc2)
{
	RECT	rc;

	if(IntersectRect(&rc,lprc1,lprc2)) {
		/* where is it anchored */
		if(rc.left == lprc1->left && rc.top == lprc1->top) {
			if(rc.right == lprc1->right) {
				lpDestRect->left = lprc1->left;
				lpDestRect->top  = rc.bottom;
				lpDestRect->right = lprc1->right;
				lpDestRect->bottom = lprc1->bottom;
				return TRUE;
			}
			if(rc.bottom == lprc1->bottom) {
				lpDestRect->left = rc.right;
				lpDestRect->top  = lprc1->top;
				lpDestRect->right = lprc1->right;
				lpDestRect->bottom = lprc1->bottom;
				return TRUE;
			}
		}
		if(rc.right == lprc1->right && rc.bottom == lprc1->bottom) {
			if(rc.top == lprc1->top) {
				lpDestRect->left = lprc1->left;
				lpDestRect->top  = lprc1->top;
				lpDestRect->right = rc.right;
				lpDestRect->bottom = lprc1->bottom;
				return TRUE;
			}
			if(rc.left == lprc1->left) {
				lpDestRect->left = lprc1->left;
				lpDestRect->top  = lprc1->top;
				lpDestRect->right = lprc1->right;
				lpDestRect->bottom = rc.top;
				return TRUE;
			}
		}
		
	}

	CopyRect(lpDestRect,lprc1);
	return FALSE;
}

BOOL WINAPI
IsRectEmpty(const RECT *lprcRect)
{
    if (!lprcRect) return(TRUE);
    return ((lprcRect->right <= lprcRect->left) ||
        (lprcRect->bottom <= lprcRect->top));
}
 
void WINAPI
SetRect(LPRECT r, int left, int top, int right, int bottom)
{
    r->left     = left;
    r->top      = top;
    r->right    = right;
    r->bottom   = bottom;
}
 
void WINAPI
SetRectEmpty(LPRECT lpRect)
{
    lpRect->left = lpRect->top = lpRect->right = lpRect->bottom = 0;
}
 
int WINAPI
UnionRect(LPRECT lpDestRect, const RECT *lpSrc1Rect,
	  const RECT *lpSrc2Rect)
{
    BOOL bSrc1Empty, bSrc2Empty;
 
    bSrc1Empty = IsRectEmpty(lpSrc1Rect);
    bSrc2Empty = IsRectEmpty(lpSrc2Rect);
 
    if (bSrc1Empty && bSrc2Empty)
        return(0);
    if (bSrc1Empty) {
        memcpy((LPSTR)lpDestRect, (LPSTR)lpSrc2Rect, sizeof(RECT));
        return(1);
    }
    if (bSrc2Empty) {
        memcpy((LPSTR)lpDestRect, (LPSTR)lpSrc1Rect, sizeof(RECT));
        return(1);
    }
    lpDestRect->top = min(lpSrc1Rect->top, lpSrc2Rect->top);
    lpDestRect->left = min(lpSrc1Rect->left, lpSrc2Rect->left);
    lpDestRect->right = max(lpSrc1Rect->right, lpSrc2Rect->right);
    lpDestRect->bottom = max(lpSrc1Rect->bottom, lpSrc2Rect->bottom);
    return(1);
}
