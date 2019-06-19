/*    
	ButtonDraw.c	2.12
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
#include "Button.h"

extern HBRUSH GetControlBrush(HWND, HDC, WORD);
extern HPEN GetSysColorPen(int);

void Draw3DButtonRect(HDC, HPEN, HPEN, RECT, BOOL);
void ButtonDrawItem(HWND, LPDRAWITEMSTRUCT, LONG);
void SendButtonItemDraw(HWND, WORD, WORD, BOOL);

static void DrawPushButton(HWND, LPDRAWITEMSTRUCT, LONG);
static void DrawRadioCheckButton(HWND, LPDRAWITEMSTRUCT, LONG);
static void DrawGroupBox(HWND,LPDRAWITEMSTRUCT,LONG);

void
SendButtonItemDraw(HWND hWnd, WORD itemAction, WORD itemState,
			BOOL bOwner)
{
    DRAWITEMSTRUCT dis;
    HDC hDC;
    RECT rcClient;
    DWORD dwStyle;

    dwStyle = GetWindowLong(hWnd,GWL_STYLE);
    if (dwStyle & WS_VISIBLE) {
	hDC = GetDC(hWnd);
	GetClientRect(hWnd, &rcClient);
	dis.CtlType = ODT_BUTTON;
	dis.CtlID = GetWindowID(hWnd);
	dis.itemID = 0;
	dis.itemAction = itemAction;
	dis.hwndItem = hWnd;
	dis.hDC = hDC;
	CopyRect(&dis.rcItem, &rcClient);
	dis.itemData = 0L;
	if (bOwner) {
	    WORD wUserState = 0;
	    if (itemState & BSF_SELECTED)
		wUserState |= ODS_SELECTED;
	    if (itemState & BSF_GRAYED)
		wUserState |= ODS_GRAYED;
	    if (itemState & BSF_DISABLED)
		wUserState |= ODS_DISABLED;
	    if (itemState & BSF_CHECKED)
		wUserState |= ODS_CHECKED;
	    if (itemState & BSF_FOCUS)
		wUserState |= ODS_FOCUS;
	    itemState = wUserState;
	}
	dis.itemState  = itemState;
	if (bOwner)
	    SendMessage(GetParent(hWnd), WM_DRAWITEM,
			(WPARAM)dis.CtlID, (LONG)&dis);
	else 
	    ButtonDrawItem(hWnd, &dis, dwStyle);

	ReleaseDC(hWnd, hDC);
    }
}

void
ButtonDrawItem(HWND hWnd, LPDRAWITEMSTRUCT lpDis, LONG dwStyle)
{
	    switch (dwStyle & 0xf) {
		case BS_CHECKBOX:
		case BS_AUTOCHECKBOX:
		case BS_RADIOBUTTON:
		case BS_AUTORADIOBUTTON:
		case BS_3STATE:
		case BS_AUTO3STATE:
		    DrawRadioCheckButton(hWnd,lpDis,dwStyle);
		    break;
		case BS_PUSHBUTTON:
		case BS_DEFPUSHBUTTON:
		    DrawPushButton(hWnd,lpDis,dwStyle);
		    break;
		case BS_GROUPBOX:
		    DrawGroupBox(hWnd,lpDis,dwStyle);
		    break;
		default:
		    break;
	    }
}

void
ButtonEraseBackground(HWND hWnd, HDC hDC)
{
    HBRUSH hBrush;
    RECT rcClient;

    GetClientRect(hWnd,&rcClient);
#ifdef	LATER
	Set brush origin to the parent window
#endif
    hBrush = GetControlBrush(hWnd, hDC, CTLCOLOR_BTN);
    FillRect(hDC, &rcClient, hBrush);
}

static void
DrawPushButton(HWND hWnd, LPDRAWITEMSTRUCT lpDis, LONG dwWndStyle)
{
    WORD wBtnStyle;
    HFONT hFont;
    RECT rcClient, rcText, rcTemp;
    int nBorderWidth, nBorderHeight;
    int nHorOffset, nVerOffset;
    char szWindowText[80];
    HBRUSH hBrushBtnFace;
    HPEN hPenFrame,hPenWhite,hPenBtnShadow;
#ifdef LATER
    HBRUSH hBrushBtnText, hBrushBtnFace, hBrushWindow;
    HPEN hPenWindow,hPenBtnText,hPenBtnShadow,hPenFrame,hPenWhite;
#endif
    int nCount;
    DWORD dwTextExtent;
    DWORD dwAmpersand;
    WORD wTextAlign;
    COLORREF crGray;

    wBtnStyle = (WORD)dwWndStyle & 0x000f;

    GetClientRect(hWnd, &rcClient);

    SetBkMode(lpDis->hDC, TRANSPARENT);

    hPenFrame = GetSysColorPen(COLOR_WINDOWFRAME);
    hPenWhite = GetStockObject(WHITE_PEN);
    hPenBtnShadow = GetSysColorPen(COLOR_BTNSHADOW);
    hBrushBtnFace = GetSysColorBrush(COLOR_BTNFACE);

    if (lpDis->itemAction & (ODA_TEXT | ODA_DRAWENTIRE | ODA_FOCUS)) {
	if ((hFont = GetWindowWord(hWnd, BWD_HFONT)))
	    SelectObject(lpDis->hDC, hFont);
    }

	    nBorderWidth = GetSystemMetrics(SM_CXBORDER);
	    nBorderHeight = GetSystemMetrics(SM_CYBORDER);
	    if ((wBtnStyle == BS_DEFPUSHBUTTON) ||
				(lpDis->itemState & BSF_FOCUS)) {
		nBorderWidth *= 2;
		nBorderHeight *= 2;
	    }
	    if (lpDis->itemAction & (ODA_DRAWENTIRE | ODA_FOCUS)) {
		SelectObject(lpDis->hDC, hPenFrame);

		for (nCount = 0; nCount < nBorderWidth; nCount++) {
		    MoveTo(lpDis->hDC, rcClient.left+nCount,
			rcClient.top+1);
		    LineTo(lpDis->hDC, rcClient.left+nCount,
			rcClient.bottom-1);
		    MoveTo(lpDis->hDC, rcClient.right-nBorderWidth+nCount,
			rcClient.top+1);
		    LineTo(lpDis->hDC, rcClient.right-nBorderWidth+nCount,
			rcClient.bottom-1);
		}
		for (nCount = 0; nCount < nBorderHeight; nCount++) {
		    MoveTo(lpDis->hDC, rcClient.left+1,
			rcClient.top+nCount);
		    LineTo(lpDis->hDC, rcClient.right-1,
			rcClient.top+nCount);
		    MoveTo(lpDis->hDC, rcClient.left+1,
			rcClient.bottom-nBorderHeight+nCount);
		    LineTo(lpDis->hDC, rcClient.right-1,
			rcClient.bottom-nBorderHeight+nCount);
		}
	    }
	    InflateRect(&rcClient, -nBorderWidth, -nBorderHeight);
	    if (lpDis->itemAction & (ODA_DRAWENTIRE|ODA_SELECT)) {
		FillRect(lpDis->hDC, &rcClient, hBrushBtnFace);
	    }
	    if (lpDis->itemAction & (ODA_SELECT | ODA_DRAWENTIRE | ODA_FOCUS))
		if (!(lpDis->itemState & BSF_SELECTED)) {
		    Draw3DButtonRect(lpDis->hDC,
			hPenWhite,
			hPenBtnShadow,
			rcClient,
			FALSE);
		    if (!(lpDis->itemAction & (ODA_SELECT | ODA_DRAWENTIRE))) {
			SetRect(&rcTemp,rcClient.left+2,rcClient.top+2,
					rcClient.right-2,rcClient.bottom-2);
			FrameRect(lpDis->hDC, &rcTemp, hBrushBtnFace);
		    }
		}
		else {
		    Draw3DButtonRect(lpDis->hDC,
			hPenWhite,
			hPenBtnShadow,
			rcClient,
			TRUE);
		}
	    if (lpDis->itemAction &
		(ODA_TEXT | ODA_DRAWENTIRE | ODA_FOCUS)) {
#ifdef	LATER
	Handle gray text if disabled
#endif
		SetBkColor(lpDis->hDC, GetSysColor(COLOR_BTNFACE));

		GetWindowText(hWnd, szWindowText, 80);
		dwTextExtent = GetTextExtent(lpDis->hDC, szWindowText,
				strlen(szWindowText));
	        if(strchr(szWindowText,'&')) {
	    	    dwAmpersand = GetTextExtent(lpDis->hDC, "&",1);
		    dwTextExtent -= (DWORD) LOWORD(dwAmpersand);
	        }
		if (lpDis->itemState & BSF_SELECTED)
		    OffsetRect(&rcClient, 2, 2);
		if (dwWndStyle & BS_LEFTTEXT)
		    wTextAlign = DT_LEFT | DT_VCENTER | DT_WORDBREAK;
		else
		    wTextAlign = DT_CENTER | DT_VCENTER | DT_WORDBREAK;
		nHorOffset = (int)(rcClient.right-rcClient.left-
		    6 - LOWORD(dwTextExtent))/2;
		nVerOffset = (int)(rcClient.bottom-rcClient.top-
		    HIWORD(dwTextExtent))/2;
		if (lpDis->itemAction & (ODA_TEXT | ODA_DRAWENTIRE)) {
		    CopyRect(&rcText, &rcClient);
		    InflateRect(&rcText, -4, -4);
		    FillRect(lpDis->hDC, &rcText, hBrushBtnFace);
		    if (!(lpDis->itemState & BSF_DISABLED)) {
		        SetTextColor(lpDis->hDC, GetSysColor(COLOR_BTNTEXT));
		        DrawText(lpDis->hDC, szWindowText, strlen(szWindowText),
			    &rcClient, wTextAlign);
		    }
		    else {
			crGray = GetSysColor(COLOR_GRAYTEXT);
			if (crGray) {
			    SetTextColor(lpDis->hDC,crGray);
			    DrawText(lpDis->hDC, szWindowText,
				strlen(szWindowText),
				&rcClient, wTextAlign);
			}
			else {
#ifdef	LATER
	call GrayString
#endif
			}
		    }
		}
		InflateRect(&rcClient, -nHorOffset, -nVerOffset);

		if (lpDis->itemAction & ODA_FOCUS) {
		    FrameRect(lpDis->hDC, &rcClient, hBrushBtnFace);
		    if (lpDis->itemState & BSF_FOCUS)
			DrawFocusRect(lpDis->hDC,&rcClient);
		}
	    }
}

static void
DrawRadioCheckButton(HWND hWnd, LPDRAWITEMSTRUCT lpDis, LONG dwWndStyle)
{
    WORD wBtnStyle;
    HFONT hFont;
    RECT rcClient, rcText, rcCheck;
    char szWindowText[80];
    HPEN hPenBtnText;
    HBRUSH hBrushBtnText, hBrushWindow, hBrushIndeterminate;
    int nCount,nTextLen;
    DWORD dwTextExtent;
    DWORD dwAmpersand;
    WORD wTextAlign;
    COLORREF crGray;

    wBtnStyle = (WORD)dwWndStyle & 0x000f;

    GetClientRect(hWnd, &rcClient);

    SetBkMode(lpDis->hDC, TRANSPARENT);

    hPenBtnText = GetSysColorPen(COLOR_BTNTEXT);
    hBrushWindow = GetControlBrush(hWnd, lpDis->hDC, CTLCOLOR_BTN);
    hBrushBtnText = GetSysColorBrush(COLOR_BTNTEXT);
    hBrushIndeterminate = GetSysColorBrush(COLOR_GRAYTEXT);

    if ((hFont = GetWindowWord(hWnd, BWD_HFONT)))
	SelectObject(lpDis->hDC, hFont);

    /* Phase 1: get object dimensions */
    SetBkColor(lpDis->hDC, GetSysColor(COLOR_WINDOW));
    if (lpDis->itemState & BSF_DISABLED) {
	hPenBtnText = GetSysColorPen(COLOR_GRAYTEXT);
	hBrushBtnText = GetSysColorBrush(COLOR_GRAYTEXT);
    }
    nTextLen = GetWindowText(hWnd, szWindowText, 80);
    if (nTextLen != 0) {
	dwTextExtent = GetTextExtent(lpDis->hDC, szWindowText,
				strlen(szWindowText));
	if (strchr(szWindowText,'&')) {
	    dwAmpersand = GetTextExtent(lpDis->hDC, "&",1);
	    dwTextExtent -= (DWORD) LOWORD(dwAmpersand);
	}
	nCount = (int)(rcClient.bottom - rcClient.top - HIWORD(dwTextExtent))/2;
	rcCheck.top = nCount;
	rcCheck.bottom = rcCheck.top + HIWORD(dwTextExtent);
	rcText.top = rcCheck.top;
	rcText.bottom = rcCheck.bottom;

	if (dwWndStyle & BS_LEFTTEXT) {
#ifdef LATER
	    rcCheck.left = rcClient.right - HIWORD(dwTextExtent) - 3;
	    rcCheck.right = rcCheck.left + HIWORD(dwTextExtent);
#else
	    rcCheck.right = rcClient.right;
	    rcCheck.left = rcCheck.right - HIWORD(dwTextExtent);
#endif
	}
	else {
	    rcCheck.left = rcClient.left;
	    rcCheck.right = rcClient.left + HIWORD(dwTextExtent);
	}
    }
    else {
	dwTextExtent = GetTextExtent(lpDis->hDC, " ", 1);
	if (dwWndStyle & BS_LEFTTEXT) {
	    rcCheck.right = rcClient.right;
	    rcCheck.left = rcCheck.right - HIWORD(dwTextExtent);
	}
	else {
	    rcCheck.left = rcClient.left;
	    rcCheck.right = rcClient.left + HIWORD(dwTextExtent);
	}
	nCount = (int)(rcClient.bottom - rcClient.top - HIWORD(dwTextExtent))/2;
	rcCheck.top = nCount;
	rcCheck.bottom = rcCheck.top + HIWORD(dwTextExtent);
	SetRectEmpty(&rcText);
    }

	    /* Phase 2: draw the item w/ frame of rectangle or ellipse */
	    if (lpDis->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {

		/* draw the basic object */
		if ((wBtnStyle == BS_CHECKBOX) ||
		    (wBtnStyle == BS_AUTOCHECKBOX) ||
		    (wBtnStyle == BS_3STATE) ||
		    (wBtnStyle == BS_AUTO3STATE))
		    FrameRect(lpDis->hDC, &rcCheck, hBrushBtnText);
		else {
		    SelectObject(lpDis->hDC,hBrushWindow);
		    SelectObject(lpDis->hDC,hPenBtnText);
		    Ellipse(lpDis->hDC,rcCheck.left,rcCheck.top,
				rcCheck.right,rcCheck.bottom);
		}

		rcCheck.left++;
		rcCheck.top++;
		rcCheck.right--;
		rcCheck.bottom--;

		/* can it ever be checked? */
		if ((wBtnStyle == BS_CHECKBOX) ||
		    (wBtnStyle == BS_AUTOCHECKBOX) ||
		    (wBtnStyle == BS_3STATE) ||
		    (wBtnStyle == BS_AUTO3STATE))
		    FillRect(lpDis->hDC, &rcCheck, hBrushWindow);
		
		/* is it selected */
		if (lpDis->itemState & BSF_SELECTED) {
			if ((wBtnStyle == BS_CHECKBOX) ||
		    	    (wBtnStyle == BS_AUTOCHECKBOX) ||
			    (wBtnStyle == BS_3STATE) ||
			    (wBtnStyle == BS_AUTO3STATE))
		    	    FrameRect(lpDis->hDC, &rcCheck, hBrushBtnText);
			else {
		    	    SelectObject(lpDis->hDC,hBrushWindow);
		    	    SelectObject(lpDis->hDC,hPenBtnText);
		    	    Ellipse(lpDis->hDC,
				rcCheck.left,rcCheck.top,
				rcCheck.right,rcCheck.bottom);
			}
		}

		/* is it checked */
		if ((lpDis->itemState & BSF_CHECKMASK) == 2 &&
		    ((wBtnStyle == BS_3STATE) ||
		     (wBtnStyle == BS_AUTO3STATE))) {
		    FillRect(lpDis->hDC, &rcCheck, hBrushIndeterminate);
		}
		else if (lpDis->itemState & BSF_CHECKED) {
		    if ((wBtnStyle == BS_CHECKBOX) ||
		        (wBtnStyle == BS_AUTOCHECKBOX) ||
			(wBtnStyle == BS_3STATE) ||
			(wBtnStyle == BS_AUTO3STATE)) {
		    	SelectObject(lpDis->hDC, hPenBtnText);
			MoveTo(lpDis->hDC, rcCheck.left, rcCheck.top);
			LineTo(lpDis->hDC, rcCheck.right, rcCheck.bottom);
			MoveTo(lpDis->hDC, rcCheck.left, rcCheck.bottom-1);
			LineTo(lpDis->hDC, rcCheck.right, rcCheck.top-1);
		    }
		    else {
			SelectObject(lpDis->hDC,hPenBtnText);
			SelectObject(lpDis->hDC,hBrushBtnText);
			Ellipse(lpDis->hDC,
				rcCheck.left+3,
				rcCheck.top+3,
				rcCheck.right-3,
				rcCheck.bottom-3);
		    }
		}
	    }

	    /* Phase 3: Add Text to the item... */
	    if ((lpDis->itemAction & (ODA_DRAWENTIRE|ODA_TEXT|ODA_FOCUS)) &&
		nTextLen != 0) {

		if (!(dwWndStyle & BS_LEFTTEXT)) {
		    rcText.left = HIWORD(dwTextExtent) + 4;
		    if (lpDis->itemAction & (ODA_DRAWENTIRE | ODA_TEXT)) {
			rcText.right = rcClient.right - 1;
			FillRect(lpDis->hDC,&rcText,hBrushWindow);
		    }
		    rcText.right = rcText.left + LOWORD(dwTextExtent);
		}
		else {
		    rcText.right = rcClient.right - HIWORD(dwTextExtent) - 4;
		    if (lpDis->itemAction & (ODA_DRAWENTIRE | ODA_TEXT)) {
			rcText.left = rcClient.left + 1;
			FillRect(lpDis->hDC,&rcText,hBrushWindow);
		    }
		    rcText.left = rcText.right - LOWORD(dwTextExtent);
		}

		if (lpDis->itemAction & (ODA_DRAWENTIRE | ODA_TEXT)) {
		    wTextAlign =  DT_LEFT | DT_VCENTER | DT_WORDBREAK;
		    if (!(lpDis->itemState & BSF_DISABLED)) {
		        SetTextColor(lpDis->hDC, GetSysColor(COLOR_BTNTEXT));
		        DrawText(lpDis->hDC, szWindowText, strlen(szWindowText),
			    &rcText, wTextAlign);
		    }
		    else {
			crGray = GetSysColor(COLOR_GRAYTEXT);
			if (crGray) {
			    SetTextColor(lpDis->hDC,crGray);
			    DrawText(lpDis->hDC, szWindowText,
				strlen(szWindowText),
				&rcText, wTextAlign);
			}
			else {
#ifdef	LATER
	call GrayString
#endif
			}
		    }
		}
	    }
		
	    /* Phase 4: Draw in the focus */
	    if ((lpDis->itemAction & ODA_FOCUS) && nTextLen != 0) {
		rcText.left -= 1;
		rcText.right += 1;
		rcText.top -= 2;
		rcText.bottom += 2;
		FrameRect(lpDis->hDC, &rcText, hBrushWindow);
		if (lpDis->itemState & BSF_FOCUS) 
		    DrawFocusRect(lpDis->hDC, &rcText);
	    }
}

static void
DrawGroupBox(HWND hWnd, LPDRAWITEMSTRUCT lpDis, LONG dwWndStyle)
{
    RECT rc,rcClient,rcText;
    HPEN hPen;
    HFONT hFont;
    int len;
    char lpszWindowText[80];

    GetClientRect(hWnd,&rcClient);
    hPen = SelectObject(lpDis->hDC,GetSysColorPen(COLOR_WINDOWFRAME));
    hFont = (HFONT)GetWindowWord(hWnd,BWD_HFONT);
    if (hFont)
	hFont = SelectObject(lpDis->hDC,hFont);
    len = GetWindowText(hWnd,lpszWindowText,80);
    if (len == 0)
	strcpy(lpszWindowText,"123");
    SetRectEmpty(&rc);
    DrawText(lpDis->hDC,lpszWindowText,-1,&rc,DT_CALCRECT);
    
    if (len) {
	SetTextColor(lpDis->hDC,GetSysColor(COLOR_WINDOWTEXT));
	SetBkMode(lpDis->hDC,TRANSPARENT);
	SetRect(&rcText,8,2,rc.right+8,rc.bottom+2);
	DrawText(lpDis->hDC,lpszWindowText,-1,&rcText,DT_CENTER);
    }
    MoveTo(lpDis->hDC,0,rc.bottom/2);
    if (len) {
	LineTo(lpDis->hDC,5,rc.bottom/2);
	MoveTo(lpDis->hDC,rc.right+11,rc.bottom/2);
	LineTo(lpDis->hDC,rcClient.right-1,rc.bottom/2);
    }
    else
	LineTo(lpDis->hDC,rcClient.right-1,rc.bottom/2);
    LineTo(lpDis->hDC,rcClient.right-1,rcClient.bottom-1);
    LineTo(lpDis->hDC,rcClient.left,rcClient.bottom-1);
    LineTo(lpDis->hDC,rcClient.left,rc.bottom/2);

    hPen = SelectObject(lpDis->hDC,hPen);
    DeleteObject(hPen);
    if (hFont)
	SelectObject(lpDis->hDC,hFont);
}

void
Draw3DButtonRect(HDC hDC, HPEN hPenHigh, HPEN hPenShadow,
		RECT rc, BOOL fClicked)
{
    HPEN     hPenOld;
    POINT    lpt[6];

    hPenOld = SelectObject(hDC, hPenShadow);
    if (fClicked) {
	lpt[0].x = lpt[1].x = rc.left;
	lpt[1].y = lpt[2].y = rc.top;
	lpt[2].x = rc.right-1;
	lpt[0].y = rc.bottom-1;
        Polyline(hDC,lpt,3);
    }
    else {
	lpt[0].x = lpt[1].x = rc.right-1;
	lpt[0].y = rc.top;
	lpt[1].y = lpt[2].y = rc.bottom-1;
	lpt[2].x = rc.left;
	lpt[3].x = rc.left+1;	
	lpt[3].y = lpt[4].y = rc.bottom-2;	
	lpt[4].x = lpt[5].x = rc.right-2;
	lpt[5].y = rc.top+1;
	Polyline(hDC,lpt,6);

	SelectObject(hDC, hPenHigh);
	lpt[0].x = rc.right-1;
	lpt[0].y = lpt[1].y = rc.top;
	lpt[1].x = lpt[2].x = rc.left;
	lpt[2].y = rc.bottom-1;
	lpt[3].x = lpt[4].x = rc.left+1;
	lpt[3].y = rc.bottom-2;
	lpt[4].y = lpt[5].y = rc.top+1;
	lpt[5].x = rc.right-2;
	Polyline(hDC,lpt,6);
    }
    SelectObject(hDC,hPenOld);
}

