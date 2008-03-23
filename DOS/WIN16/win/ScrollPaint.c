/*    
	ScrollPaint.c	2.20
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


#ifndef OEMRESOURCE
#define OEMRESOURCE	/* for OBM_*** */
#endif
#include "windows.h"
#include "windowsx.h"
#undef OEMRESOURCE

#include "Log.h"
#include "GdiDC.h"
#include "kerndef.h"
#include "WinDefs.h"
#include "Scroll.h"

/* external routines */
extern HBRUSH GetControlBrush(HWND,HDC,WORD);
extern void Draw3DButtonRect(HDC, HPEN, HPEN, RECT, BOOL);
HPEN GetSysColorPen(int );

static HBITMAP LoadArrow(DWORD);

void
ScrollbarSendItemDraw(LPTWINSCROLLINFO lpScrollInfo,
			WORD wAction, WORD wState)
{
    DRAWITEMSTRUCT dis;
    HDC hDC;
    RECT rcClient,rcNC;
    HWND hWndParent;
    DWORD dwStyle;
    HWND32 hWndParent32,hWnd32;
    POINT pt;

    hWndParent = GetParent(lpScrollInfo->hWnd);
    if (!IsWindowVisible(hWndParent))
	return;
    if (!SBTSTATE(lpScrollInfo,SSF_HIDDEN)) {
	dwStyle = GetWindowLong(lpScrollInfo->hWnd,GWL_STYLE);
	if (dwStyle & SBS_SYSTEM) {
	    hDC = GetDCEx(hWndParent,(HRGN)0,DCX_WINDOW);
	    hWnd32 = GETHWND32(lpScrollInfo->hWnd);
	    hWndParent32 = GETHWND32(hWndParent);
	    GetWindowRect(hWndParent,&rcClient);
	    IntersectClipRect(hDC, 0, 0, rcClient.right-rcClient.left,
					rcClient.bottom-rcClient.top);
	    pt.x = hWnd32->rWnd.left;
	    pt.y = hWnd32->rWnd.top;
	    if (!(hWndParent32->dwStyle & WS_CHILD)) {
		CalcExpectedNC(&rcNC,hWndParent32->dwStyle,
				hWndParent32->dwExStyle);
		pt.x += rcNC.left;
		pt.y += rcNC.top;
	    }
	    SetWindowOrg(hDC,-pt.x,-pt.y);
	    RELEASEWININFO(hWnd32);
	    RELEASEWININFO(hWndParent32);
	}
	else
	    hDC = GetDC(lpScrollInfo->hWnd);
	GetClientRect(lpScrollInfo->hWnd,&rcClient);
	dis.CtlType = ODT_SCROLLBAR;
	dis.CtlID = GetWindowID(lpScrollInfo->hWnd);
	dis.itemID = 0;
	dis.itemAction = wAction;
	dis.itemState = wState;
	dis.hwndItem = lpScrollInfo->hWnd;
	dis.hDC = hDC;
	CopyRect(&dis.rcItem,&rcClient);
	dis.itemData = 0L;
	if (SBTSTYLE(lpScrollInfo,SBS_OWNERDRAW)) 
	    SendMessage(hWndParent,WM_DRAWITEM,0,(LONG)&dis);
	else
	    ScrollbarDraw(lpScrollInfo,&dis);
	ReleaseDC(lpScrollInfo->hWnd,hDC);
    }
    
}

void
ScrollbarDraw(LPTWINSCROLLINFO lpScrollInfo, LPDRAWITEMSTRUCT lpDis)
{
    RECT	rc;
    HBRUSH	hBrushScroll,hBrushFace;
    int		cx,cy,x,y;
    BOOL	bVert;
    BOOL	bSelected=FALSE;
    BOOL	bDrawThumb = FALSE;
    static HBITMAP hUpArrow,hUpArrowD;
    static HBITMAP hDnArrow,hDnArrowD;
    static HBITMAP hRgArrow,hRgArrowD;
    static HBITMAP hLfArrow,hLfArrowD;
    static HBITMAP hUpArrowI,hDnArrowI;
    static HBITMAP hLfArrowI,hRgArrowI;
    static BITMAP bm;
    DWORD	dwStyle;
    HBITMAP hBitmap;
    HDC hDCMem;

    hDCMem = CreateCompatibleDC(lpDis->hDC);

    if (lpDis->itemAction & ODA_DRAWENTIRE) 
	lpDis->itemAction |= (ODA_LEFTARROW | ODA_LEFTFIELD | ODA_THUMB |
				ODA_RIGHTARROW | ODA_RIGHTFIELD);

    hBrushFace = GetSysColorBrush(COLOR_BTNFACE);
    dwStyle = GetWindowLong(lpScrollInfo->hWnd,GWL_STYLE);
    if (dwStyle & SBS_SYSTEM)
	hBrushScroll = GetSysColorBrush(COLOR_SCROLLBAR);
    else
	hBrushScroll = GetControlBrush(lpScrollInfo->hWnd,
				lpDis->hDC,CTLCOLOR_SCROLLBAR);

    if (lpScrollInfo->dwStyle & SBS_SIZEBOX) {
	FillRect(lpDis->hDC,&lpDis->rcItem,hBrushScroll);
	return;
    }
    if (bm.bmWidth == 0) {
	hUpArrow = LoadArrow(OBM_UPARROW);
	GetObject(hUpArrow,sizeof(BITMAP),&bm);
    }

    if (lpDis->itemAction & (ODA_LEFTFIELD|ODA_THUMB|ODA_RIGHTFIELD))
	SBCSTATE(lpScrollInfo,SSF_THUMBTRACK);

    bVert = SBTSTYLE(lpScrollInfo,SBS_VERT);

    /* Calc the shifts for the arrows */
    if (!SBTSTATE(lpScrollInfo,SSF_MOUSEOUT)) {
	if (lpDis->itemState & ODS_SELECTED)
	    bSelected = TRUE;
    }

    /* Draw the face color and the outer frame */
    SelectObject(lpDis->hDC, hBrushScroll);

    /* PHASE 1:	Draw the entire area */
    if (lpDis->itemAction & ODA_DRAWENTIRE) {
	Rectangle(lpDis->hDC,
		lpDis->rcItem.left,
		lpDis->rcItem.top,
		lpDis->rcItem.right,
		lpDis->rcItem.bottom);
    }
    else if (lpDis->itemAction & (ODA_LEFTFIELD|ODA_THUMB|ODA_RIGHTFIELD)) {
	if (bVert) {
	    MoveTo(lpDis->hDC,lpDis->rcItem.left,lpDis->rcItem.top);
	    LineTo(lpDis->hDC,lpDis->rcItem.left,lpDis->rcItem.bottom);
	    MoveTo(lpDis->hDC,lpDis->rcItem.right-1,lpDis->rcItem.top);
	    LineTo(lpDis->hDC,lpDis->rcItem.right-1,lpDis->rcItem.bottom);
	}
	else {
	    MoveTo(lpDis->hDC,lpDis->rcItem.left,lpDis->rcItem.top);
	    LineTo(lpDis->hDC,lpDis->rcItem.right,lpDis->rcItem.top);
	    MoveTo(lpDis->hDC,lpDis->rcItem.left,lpDis->rcItem.bottom-1);
	    LineTo(lpDis->hDC,lpDis->rcItem.right,lpDis->rcItem.bottom-1);
	}
    }

    /* PHASE 2:	Draw the Left Arrow */
    if (lpDis->itemAction & ODA_LEFTARROW) {
	/* Draw arrows depending on the orientation */
	if (!bVert) {
	    if (SBTSTATE(lpScrollInfo,SSF_LEFTARROWGRAYED)) {
		if (!hLfArrowI) 
		    hLfArrowI = LoadBitmap(0,(LPSTR)OBM_LFARROWI);
		hBitmap = hLfArrowI;
	    }
	    else {
		if (lpDis->itemState & ODS_SELECTED) {
		    if (hLfArrowD == 0)
			hLfArrowD = LoadArrow(OBM_LFARROWD);
		    hBitmap = hLfArrowD;
		}
		else {
		    if (hLfArrow == 0) 
		        hLfArrow = LoadArrow(OBM_LFARROW);
		    hBitmap = hLfArrow;
		}
	    }
	    x = lpDis->rcItem.left;
	    y = lpDis->rcItem.top;
	    cx = lpScrollInfo->wArrowWidth;
	    cy = lpDis->rcItem.bottom-lpDis->rcItem.top;
	}
	else {
	    if (SBTSTATE(lpScrollInfo,SSF_LEFTARROWGRAYED)) {
		if (!hUpArrowI) 
		    hUpArrowI = LoadBitmap(0,(LPSTR)OBM_UPARROWI);
		hBitmap = hUpArrowI;
	    }
	    else {
		if (lpDis->itemState & ODS_SELECTED) {
		    if (hUpArrowD == 0)
			hUpArrowD = LoadArrow(OBM_UPARROWD);
		    hBitmap = hUpArrowD;
		}
		else {
		    if (hUpArrow == 0) 
		        hUpArrow = LoadArrow(OBM_UPARROW);
		    hBitmap = hUpArrow;
		}
	    }

            x = lpDis->rcItem.left;
	    y = lpDis->rcItem.top;
	    cx = lpDis->rcItem.right-lpDis->rcItem.left;
	    cy = lpScrollInfo->wArrowHeight;
	}

	SelectObject(hDCMem,hBitmap);
        StretchBlt(lpDis->hDC,
		x,y, cx,cy,
                hDCMem,0,0,
                bm.bmWidth,bm.bmHeight,
                SRCCOPY);
    }

    /* PHASE 3:	Draw the Left Field */
    if (lpDis->itemAction & ODA_LEFTFIELD) {
	if (!bVert) {
	    cx = lpDis->rcItem.left+lpScrollInfo->wArrowWidth;
	    SetRect(&rc,
		cx,
		lpDis->rcItem.top+1,
		cx+lpScrollInfo->wThumbOffset-1,
		lpScrollInfo->wArrowHeight-1);
	}
	else {
	    cy = lpDis->rcItem.top+lpScrollInfo->wArrowHeight;
	    SetRect(&rc,
		lpDis->rcItem.left+1,
		cy,
		lpScrollInfo->wArrowWidth-1,
		cy+lpScrollInfo->wThumbOffset-1);
	}


	if (!IsRectEmpty(&rc))
	    FillRect(lpDis->hDC,&rc,hBrushScroll);
	if (bSelected)
	    InvertRect(lpDis->hDC,&rc);
    }

    /* PHASE 4:		Draw the Thumb */
    if (lpDis->itemAction & (ODA_THUMB | ODA_THUMBTRACK)) {
	if (!bVert) {
	    cx = lpDis->rcItem.left+lpScrollInfo->wArrowWidth+
		((lpDis->itemAction & ODA_THUMB) ?
				lpScrollInfo->wThumbOffset :
				lpScrollInfo->wTrackingOffset);
	    SetRect(&rc,
		cx,
		lpDis->rcItem.top+1,
		cx+lpScrollInfo->wArrowWidth-1,
		lpScrollInfo->wArrowHeight-1);

            x = rc.left;
            cy = lpDis->rcItem.right - lpScrollInfo->wArrowWidth;
            y = rc.right;
	}
	else {
	    cy = lpDis->rcItem.top+lpScrollInfo->wArrowHeight+
		((lpDis->itemAction & ODA_THUMB) ?
				lpScrollInfo->wThumbOffset :
				lpScrollInfo->wTrackingOffset);
	    SetRect(&rc,
		lpDis->rcItem.left+1,
		cy,
		lpScrollInfo->wArrowWidth-1,
		cy+lpScrollInfo->wArrowHeight-1);

            cx = cy;
            x = rc.top;
            cy = lpDis->rcItem.bottom - lpScrollInfo->wArrowHeight;
            y = rc.bottom;
	}

	bDrawThumb = (cx <= x && y <= cy ) ? TRUE : FALSE;


	if(bDrawThumb) {
	    if (lpDis->itemAction & ODA_THUMBTRACK) {
		InflateRect(&rc,1,1);
		DrawFocusRect(lpDis->hDC,&rc);
	    }
	    else {
		FillRect(lpDis->hDC,&rc,hBrushFace);
		Draw3DButtonRect(lpDis->hDC,
			GetStockObject(WHITE_PEN),
			GetSysColorPen(COLOR_BTNSHADOW),rc,0);
		MoveTo(lpDis->hDC,rc.left-1,rc.top-1);
		LineTo(lpDis->hDC,rc.right,rc.top-1);
		LineTo(lpDis->hDC,rc.right,rc.bottom);
		LineTo(lpDis->hDC,rc.left-1,rc.bottom);
		LineTo(lpDis->hDC,rc.left-1,rc.top-1);
	    }
	}
    }

    /* PHASE 5:	Draw the Right Field */
    if (lpDis->itemAction & ODA_RIGHTFIELD) {
	if (!bVert) {
	    cx = lpDis->rcItem.left+2*lpScrollInfo->wArrowWidth+
			lpScrollInfo->wThumbOffset;
	    SetRect(&rc,cx,lpDis->rcItem.top+1,
	    	lpDis->rcItem.right - lpScrollInfo->wArrowWidth,
		lpScrollInfo->wArrowHeight-1);
	}
	else {
	    cy = lpDis->rcItem.top+2*lpScrollInfo->wArrowHeight+
		lpScrollInfo->wThumbOffset;
	    SetRect(&rc,lpDis->rcItem.left+1,
		cy,
		lpScrollInfo->wArrowWidth-1,
		lpDis->rcItem.bottom-lpScrollInfo->wArrowHeight);
	}


	if (!IsRectEmpty(&rc))
	    FillRect(lpDis->hDC,&rc,hBrushScroll);
	if (bSelected)
	    InvertRect(lpDis->hDC,&rc);
    }

    /* PHASE 6:	Draw the Right Arrow */
    if (lpDis->itemAction & ODA_RIGHTARROW) {
	if (!bVert) {
	    if (SBTSTATE(lpScrollInfo,SSF_RIGHTARROWGRAYED)) {
		if (!hRgArrowI) 
		    hRgArrowI = LoadBitmap(0,(LPSTR)OBM_RGARROWI);
		hBitmap = hRgArrowI;
	    }
	    else {
		if (lpDis->itemState & ODS_SELECTED) {
		    if (hRgArrowD == 0)
			hRgArrowD = LoadArrow(OBM_RGARROWD);
		    hBitmap = hRgArrowD;
		}
		else {
		    if (hRgArrow == 0) 
		        hRgArrow = LoadArrow(OBM_RGARROW);
		    hBitmap = hRgArrow;
		}
	    }
	    x = lpDis->rcItem.right-lpScrollInfo->wArrowWidth;
	    y = lpDis->rcItem.top;
	    cx = lpScrollInfo->wArrowWidth;
	    cy = lpDis->rcItem.bottom-lpDis->rcItem.top;
	}
	else {
	    if (SBTSTATE(lpScrollInfo,SSF_RIGHTARROWGRAYED)) {
		if (!hDnArrowI) 
		    hDnArrowI = LoadBitmap(0,(LPSTR)OBM_DNARROWI);
		hBitmap = hDnArrowI;
	    }
	    else {
		if (lpDis->itemState & ODS_SELECTED) {
		    if (hDnArrowD == 0)
			hDnArrowD = LoadArrow(OBM_DNARROWD);
		    hBitmap = hDnArrowD;
		}
		else {
		    if (hDnArrow == 0) 
		        hDnArrow = LoadArrow(OBM_DNARROW);
		    hBitmap = hDnArrow;
		}
	    }
	    x = lpDis->rcItem.left;
	    y = lpDis->rcItem.bottom-lpScrollInfo->wArrowHeight;
	    cx = lpDis->rcItem.right-lpDis->rcItem.left;
	    cy = lpScrollInfo->wArrowHeight;
     
	}

	SelectObject(hDCMem,hBitmap);
	StretchBlt(lpDis->hDC,
		x,y,cx,cy,
		hDCMem,0,0,
		bm.bmWidth,bm.bmHeight,
		SRCCOPY);
    }
#ifdef LATER
    /* PHASE 5:		Draw the Thumb Focus Rect */
    if (lpDis->itemAction & ODA_THUMBTRACK) {
	if (!bVert) {
	    cx = lpDis->rcItem.left+lpScrollInfo->wArrowWidth+
		lpScrollInfo->wTrackingOffset;
	    SetRect(&rcThumb,
		cx,
		lpDis->rcItem.top,
		cx+lpScrollInfo->wArrowWidth,
		lpScrollInfo->wArrowHeight);
	}
	else {
	    cy = lpDis->rcItem.top+lpScrollInfo->wArrowHeight+
		lpScrollInfo->wTrackingOffset;
	    SetRect(&rcThumb,
		lpDis->rcItem.left,
		cy,
		lpScrollInfo->wArrowWidth,
		cy+lpScrollInfo->wArrowHeight);
	}
	DrawFocusRect(lpDis->hDC,&rcThumb);
    }
#endif
    SelectObject(hDCMem,GetStockObject(SYSTEM_BITMAP));
    DeleteDC(hDCMem);

}

static HBITMAP
LoadArrow(DWORD dwID)
{
    HBITMAP hBitmap;

#ifdef MOTIF    /* Motif implementation variables */
    BITMAPINFO *lpbmi;
    HDC hDC;
    LPBYTE lpBits;
    int i;
    RGBQUAD *lprgb;
    COLORREF crTmp;
    HRSRC hResource;
    HGLOBAL hGlobal;
    BITMAPINFO *lpBitmapInfo;
    int nNumColors;
#endif


#ifndef MOTIF
    hBitmap = LoadBitmap(0,(LPSTR)dwID);
#else
    hResource = FindResource(0,
			(LPSTR)(dwID-(OBM_UPARROW-OBM_M_UPARROW)),
			RT_BITMAP);
    hGlobal = LoadResource(0,hResource);
    lpBitmapInfo = (BITMAPINFO *)LockResource(hGlobal);
    if (lpBitmapInfo == 0)
	return (HBITMAP)0;
    nNumColors = 1<<lpBitmapInfo->bmiHeader.biBitCount;
    lpbmi = (BITMAPINFO *)WinMalloc(sizeof(BITMAPINFOHEADER) *
			sizeof(RGBQUAD)*nNumColors);
    lpbmi->bmiHeader = lpBitmapInfo->bmiHeader;
    lprgb = &lpbmi->bmiColors[0];
    hDC = GetDC(0);
    for (i=0; i<nNumColors; i++) {
	crTmp = RGB(lpBitmapInfo->bmiColors[i].rgbRed,
		    lpBitmapInfo->bmiColors[i].rgbGreen,
		    lpBitmapInfo->bmiColors[i].rgbBlue);
	if (crTmp == RGB(255,0,0))
	    crTmp = GetSysColor(COLOR_BTNFACE);
	else if (crTmp == RGB(0,0,0))
	    crTmp = GetSysColor(COLOR_BTNSHADOW);
	else if (crTmp == RGB(255,255,255))
	    crTmp = GetSysColor(COLOR_BTNHIGHLIGHT);

	lprgb[i].rgbRed = GetRValue(crTmp);
	lprgb[i].rgbGreen = GetGValue(crTmp);
	lprgb[i].rgbBlue = GetBValue(crTmp);
    }
    lpBits = (LPBYTE)lpBitmapInfo + sizeof(BITMAPINFOHEADER) +
			sizeof(RGBQUAD)*nNumColors;
    hBitmap = CreateDIBitmap(hDC,&lpBitmapInfo->bmiHeader,CBM_INIT,
		lpBits,lpbmi,DIB_RGB_COLORS);
    ReleaseDC(0,hDC);
    WinFree((LPSTR)lpbmi);
    UnlockResource(hGlobal);
    FreeResource(hGlobal);
#endif

    return hBitmap;
}
