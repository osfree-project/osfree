/*    
	WindowBytes.c	2.16
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

#include "kerndef.h"
#include "WinDefs.h"
#include "Log.h"

/* Prototypes for static internal routines */
static LONG WindowLong(int, HWND, int, DWORD);
static WORD WindowWord(int, HWND, int, WORD);
static BYTE WindowByte(int, HWND, int, BYTE);
static DWORD ChangeWindowStyle(HWND32, DWORD);

/*************************************************************/

LONG WINAPI
GetWindowLong(HWND hWnd, int nIndex)
{
    LONG rc;
    APISTR((LF_APICALL,"GetWindowLong(HWND=%x,int=%d)\n",hWnd,nIndex));
    rc = WindowLong(WND_GET, hWnd, nIndex, 0);
    APISTR((LF_APIRET,"GetWindowLong: returns LONG %d\n",rc));
    return rc;
}

WORD WINAPI
GetWindowWord(HWND hWnd, int nIndex)
{
    WORD rc;
    APISTR((LF_APICALL,"GetWindowWord(HWND=%x,int=%d)\n",hWnd,nIndex));
    rc = WindowWord(WND_GET, hWnd, nIndex, 0);
    APISTR((LF_APIRET,"GetWindowWord: returns WORD %d\n",rc));
    return rc;
}

LONG WINAPI
SetWindowLong(HWND hWnd, int nIndex, LONG dwNewLong)
{
#ifdef TWIN32
    STYLESTRUCT style;
#endif
    LONG 	lResult;

    APISTR((LF_APICALL,"SetWindowLong(HWND=%x,int=%d,LONG=%x)\n",
	hWnd,nIndex,dwNewLong));

#ifdef TWIN32
    if ( nIndex == GWL_STYLE )
    {
	style.styleNew = dwNewLong;
	style.styleOld = GetWindowLong ( hWnd, GWL_STYLE );
	SendMessage ( hWnd, WM_STYLECHANGING, GWL_STYLE, ( LPARAM )&style );
    }
#endif

    lResult = WindowLong(WND_SET, hWnd, nIndex, dwNewLong);

#ifdef TWIN32
    if ( nIndex == GWL_STYLE )
	SendMessage ( hWnd, WM_STYLECHANGED, GWL_STYLE, ( LPARAM )&style );
#endif

    APISTR((LF_APIRET,"SetWindowLong: returns LONG %d\n",lResult));
    return lResult;
}

WORD WINAPI
SetWindowWord(HWND hWnd, int nIndex, WORD wNewWord)
{
    WORD rc;
    APISTR((LF_APICALL,"SetWindowWord(HWND=%x,int=%d,WORD=%x)\n",
			hWnd,nIndex,wNewWord));

    rc = WindowWord(WND_SET, hWnd, nIndex, wNewWord);

    APISTR((LF_APIRET,"SetWindowWord: returns WORD %x\n",rc));
    return rc;
}

static LONG
WindowLong(int Func, HWND hWnd, int nIndex, DWORD dwNewLong)
{
    HWND32 hWndTmp32;
    LPBYTE     ptr,ptr1;
    DWORD dwTemp,dw;
    int i;
    BYTE b;

    if (!(hWndTmp32 = GETHWND32(hWnd))) {
	return(0L);
    }

    if (nIndex < 0) {
	switch(nIndex) {
	    case GWL_EXSTYLE:
		dwTemp = hWndTmp32->dwExStyle;
		if (Func == WND_SET)
		    hWndTmp32->dwExStyle = dwNewLong;
		break;
	    case GWL_STYLE:
		dwTemp = hWndTmp32->dwStyle;
		if (Func == WND_SET)
		    hWndTmp32->dwStyle = ChangeWindowStyle(hWndTmp32,dwNewLong);
		break;
	    case GWL_WNDPROC:
		dwTemp = (DWORD)hWndTmp32->lpfnWndProc;
		if (Func == WND_SET)
		    hWndTmp32->lpfnWndProc = (WNDPROC)dwNewLong;
		break;
	    case GWL_DRVDATA:
		dwTemp = (DWORD)hWndTmp32->lpDrvData;
		if (Func == WND_SET)
		    hWndTmp32->lpDrvData = (LPVOID)dwNewLong;
		break;
	    case GWL_UPDATE:
		dwTemp = (DWORD) hWndTmp32->UpdateRegion;
		if (Func == WND_SET)
		    hWndTmp32->UpdateRegion = dwNewLong;
		break;
	    case GWL_USERDATA:
		dwTemp = (DWORD) hWndTmp32->dwUserData;
		if (Func == WND_SET)
		    hWndTmp32->dwUserData = dwNewLong;
		break;
#ifdef	TWIN32
	    case GWL_HINSTANCE:
		dwTemp = (DWORD)(hWndTmp32->hInstance);
		if (Func == WND_SET)
		    hWndTmp32->hInstance = (HINSTANCE)dwNewLong;
		break;
	    case GWL_ID:
		dwTemp = hWndTmp32->wChildID;
		if (Func == WND_SET)
		    hWndTmp32->wChildID = dwNewLong;
		break;
	    case GWL_HMENU:
		dwTemp = (DWORD)hWndTmp32->hMenu;
		if (Func == WND_SET)
		    hWndTmp32->hMenu = (HMENU)dwNewLong;
		break;
	    case GWL_HSYSMENU:
		dwTemp = (DWORD)hWndTmp32->hSysMenu;
		if (Func == WND_SET)
		    hWndTmp32->hSysMenu = (HMENU)dwNewLong;
		break;
	    case GWL_HWNDMENU:
		dwTemp = (DWORD)hWndTmp32->hWndFrame;
		if (Func == WND_SET)
		    hWndTmp32->hWndFrame = (HWND)dwNewLong;
		break;
	    case GWL_HWNDFOCUS:
		dwTemp = (DWORD)hWndTmp32->hChildFocus;
		if (Func == WND_SET)
		    hWndTmp32->hChildFocus = (HWND)dwNewLong;
		break;
	    case GWL_HWNDPARENT:
		dwTemp = (DWORD)hWndTmp32->hWndParent;
		if (Func == WND_SET) {
		    if (hWndTmp32->dwStyle & WS_CHILD)
			return SetParent(hWnd,(HWND)dwNewLong);
		    else
			hWndTmp32->hWndOwner = (HWND)dwNewLong;
		}
		break;
	    case GWL_HWNDHZSCROLL:
		dwTemp = (DWORD)hWndTmp32->hWndHZScroll;
		if (Func == WND_SET)
		    hWndTmp32->hWndHZScroll = (HWND)dwNewLong;
		break;
	    case GWL_HWNDVTSCROLL:
		dwTemp = (DWORD)hWndTmp32->hWndVTScroll;
		if (Func == WND_SET)
		    hWndTmp32->hWndVTScroll = (HWND)dwNewLong;
		break;
	    case GWL_HDC:
		dwTemp = (DWORD)hWndTmp32->hDC;
		if (Func == WND_SET)
		    hWndTmp32->hDC = (HDC)dwNewLong;
		break;
#endif
	    default:
		ERRSTR((LF_ERROR,
		    "WindowLong: Unknown Index: %d\n", nIndex));
		dwTemp = 0;
		break;
	}

	RELEASEWININFO(hWndTmp32);
	return dwTemp;
    }

    /* assigning extra bytes... */
    ptr = ptr1 = (LPBYTE)(hWndTmp32->lpWndExtra) + nIndex;

    for (i = 0, dwTemp = 0; i < sizeof(DWORD); i++) {
	dw = ((DWORD)(*ptr++)) << (i*8);
	dwTemp |= dw;
    }

    if (Func == WND_SET)
	for (i = 0; i < sizeof(DWORD); i++) {
	    b = (BYTE)(dwNewLong >> (i*8));
	    *ptr1++ = b;
	}

    RELEASEWININFO(hWndTmp32);
    return dwTemp;
}

static WORD
WindowWord(int Func, HWND hWnd, int nIndex, WORD wNewWord)
{
    HWND32 hWndTmp32;
    LPBYTE     ptr,ptr1;
    WORD      wTemp,w;
    int i;
    BYTE b;

    if (!(hWndTmp32 = GETHWND32(hWnd))) {
	ERRSTR((LF_ERROR, "WindowWord: Bad Window: %x\n", hWnd));
	return 0;
    }

    if (nIndex < 0) {
	switch(nIndex) {
#ifndef	TWIN32
	    case GWW_HINSTANCE:
		wTemp = (WORD)(hWndTmp32->hInstance);
		if (Func == WND_SET)
		    hWndTmp32->hInstance = (HINSTANCE)wNewWord;
		break;
	    case GWW_ID:
		wTemp = hWndTmp32->wChildID;
		if (Func == WND_SET)
		    hWndTmp32->wChildID = wNewWord;
		break;
	    case GWW_HMENU:
		wTemp = (WORD)hWndTmp32->hMenu;
		if (Func == WND_SET)
		    hWndTmp32->hMenu = (HMENU)wNewWord;
		break;
	    case GWW_HSYSMENU:
		wTemp = (WORD)hWndTmp32->hSysMenu;
		if (Func == WND_SET)
		    hWndTmp32->hSysMenu = (HMENU)wNewWord;
		break;
	    case GWW_HWNDMENU:
		wTemp = (WORD)hWndTmp32->hWndFrame;
		if (Func == WND_SET)
		    hWndTmp32->hWndFrame = (HWND)wNewWord;
		break;
	    case GWW_HWNDFOCUS:
		wTemp = (WORD)hWndTmp32->hChildFocus;
		if (Func == WND_SET)
		    hWndTmp32->hChildFocus = (HWND)wNewWord;
		break;
	    case GWW_HWNDPARENT:
		wTemp = (WORD)hWndTmp32->hWndParent;
		if (Func == WND_SET) {
		    if (hWndTmp32->dwStyle & WS_CHILD)
			return SetParent(hWnd,(HWND)wNewWord);
		    else
			hWndTmp32->hWndOwner = (HWND)wNewWord;
		}
		break;
	    case GWW_HWNDHZSCROLL:
		wTemp = (WORD)hWndTmp32->hWndHZScroll;
		if (Func == WND_SET)
		    hWndTmp32->hWndHZScroll = (HWND)wNewWord;
		break;
	    case GWW_HWNDVTSCROLL:
		wTemp = (WORD)hWndTmp32->hWndVTScroll;
		if (Func == WND_SET)
		    hWndTmp32->hWndVTScroll = (HWND)wNewWord;
		break;
	    case GWW_HDC:
		wTemp = (WORD)hWndTmp32->hDC;
		if (Func == WND_SET)
		    hWndTmp32->hDC = (HDC)wNewWord;
		break;
#endif
	    default:
		ERRSTR((LF_ERROR,
		    "WindowWord: Unknown Index: %d\n", nIndex));
		wTemp = 0;
	}
    }
    else {
	ptr = ptr1 = (LPBYTE)(hWndTmp32->lpWndExtra) + nIndex;

	for (i = 0, wTemp = 0; i < sizeof(WORD); i++) {
	    w = ((WORD)(*ptr++)) << (i*8);
	    wTemp |= w;
	}

	if (Func == WND_SET)
	    for (i = 0; i < sizeof(WORD); i++) {
		b = (BYTE)(wNewWord >> (i*8));
		*ptr1++ = b;
	    }
    }

    RELEASEWININFO(hWndTmp32);
    return wTemp;
}

static BYTE
WindowByte(int Func, HWND hWnd, int nIndex, BYTE bNewByte)
{
    HWND32 hWndTmp32;
    LPSTR	ptr;
    BYTE bTemp = 0;

    if (!(hWndTmp32 = GETHWND32(hWnd))) {
	ERRSTR((LF_ERROR, "WindowByte: Bad Window: %x\n", hWnd));
	return 0;
    }

    if (nIndex < 0) {
	ERRSTR((LF_ERROR, "WindowByte: Unknown Index: %d\n", nIndex));
    } else {
	ptr = (LPSTR)(hWndTmp32->lpWndExtra) + nIndex;
	bTemp = ptr[0];
	switch(Func) {
	    case WND_OR:
		bTemp |= bNewByte;
		*ptr = bTemp;
		break;
	    case WND_AND:
		bTemp &= ~bNewByte;
		*ptr = bTemp;
		break;
	    case WND_XOR:
		bTemp = ~(bTemp ^ ~bNewByte);
		*ptr = bTemp;
		break;
	    case WND_TEST:
		bTemp &= bNewByte;
		break;
	    case WND_SET:
		*ptr = bNewByte;
		break;
	}
    }

    RELEASEWININFO(hWndTmp32);
    return(bTemp);
}

BYTE
GetWindowByte(HWND hWnd, int nIndex)
{
    return WindowByte(WND_GET, hWnd, nIndex, 0);
}

BYTE
SetWindowByte(HWND hWnd, int nIndex, BYTE bNewByte)
{
    return(WindowByte(WND_SET, hWnd, nIndex, bNewByte));
}

BOOL
OrWindowByte(HWND hWnd, int nIndex, BYTE bMask)
{
    return((BOOL)WindowByte(WND_OR, hWnd, nIndex, bMask));
}

BOOL
AndWindowByte(HWND hWnd, int nIndex, BYTE bMask)
{
    return((BOOL)WindowByte(WND_AND, hWnd, nIndex, bMask));
}

BOOL
XorWindowByte(HWND hWnd, int nIndex, BYTE bMask)
{
    return((BOOL)WindowByte(WND_XOR, hWnd, nIndex, bMask));
}

BOOL
TestWindowByte(HWND hWnd, int nIndex, BYTE bMask)
{
    return((BOOL)WindowByte(WND_TEST, hWnd, nIndex, bMask));
}

static DWORD
ChangeWindowStyle(HWND32 hWnd32, DWORD dwNewStyle)
{
    RECT rcNewNC;

    if ((hWnd32->dwStyle & (WS_BORDER|WS_DLGFRAME|WS_THICKFRAME)) !=
	(dwNewStyle & (WS_BORDER|WS_DLGFRAME|WS_THICKFRAME))) {
	CalcNCDimensions(&rcNewNC,dwNewStyle,
			hWnd32->dwExStyle, hWnd32->wMenuHeight);
	ChangeNCDimensions(GETHWND16(hWnd32),&rcNewNC);
    }

    if ((hWnd32->dwStyle & WS_HSCROLL) != (dwNewStyle & WS_HSCROLL))
	ShowScrollBar(GETHWND16(hWnd32),SB_HORZ,
		(dwNewStyle & WS_HSCROLL)?TRUE:FALSE);

    if ((hWnd32->dwStyle & WS_VSCROLL) != (dwNewStyle & WS_VSCROLL))
	ShowScrollBar(GETHWND16(hWnd32),SB_VERT,
		(dwNewStyle & WS_VSCROLL)?TRUE:FALSE);

    return dwNewStyle;
}
