/*  
	WLVLBox.c	1.6 
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

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

*/

#ifdef _WINDOWS
#include <windows.h>
#include <commctrl.h>
#else
#include "WCommCtrl.h"
#endif
#include "windowsx.h"
#include "assert.h"
#include <math.h>

#include <string.h>

#include "WLists.h"
#include "WLView.h"
/*#include "WLVLBox.h"*/

#ifndef _WINDOWS
#define  GetTextExtentPoint32   GetTextExtentPoint
#endif
#define X_OFFSET    5
#define Y_OFFSET    4
#define EDITWIDTH_MIN 30

LRESULT WINAPI 
LVEdit_WndProc
(
	HWND				hWnd,
    UINT				uMessage,
    WPARAM				wParam,
    LPARAM				lParam
);

void
LVLBox_OnHScroll
(
    HWND                    hWnd,
    int                     iCode,
    int                     iPos
)
{
    RECT                    rc;
    int                     iminPos;
    int                     imaxPos;
    int                     iPageSize;
    int                     iLineSize;
    UINT                    uStyleMask;
	HGLOBAL		            hRsrc   	= ( HGLOBAL )NULL;
	LISTVIEWDATA*	        pthis		= ( LISTVIEWDATA* )NULL;
    BOOL                    bRedraw     = FALSE;

    hRsrc = LView_IGetResource ( hWnd, &pthis );
    assert ( hRsrc );
    GetScrollRange ( hWnd, SB_HORZ, &iminPos, &imaxPos );
    GetClientRect ( hWnd, &rc );
    if ( ( uStyleMask = pthis->uStyles & LVS_TYPEMASK ) == LVS_LIST )
    {
        iLineSize = 1;
        iPageSize = 2;
    }
    else
    {
        iLineSize = 8;
        iPageSize = ( rc.right - rc.left ) / iLineSize;
    }
    switch ( iCode )
    {
        case    SB_LINELEFT: if ( pthis->psLBox.ptOrg.x > 0 )
                              {
                                iLineSize = 0 - iLineSize;
                                bRedraw = TRUE;
                              }
                              break;
        case    SB_LINERIGHT: if ( pthis->psLBox.ptOrg.x + iLineSize > imaxPos )
                                   iLineSize = imaxPos - pthis->psLBox.ptOrg.x;
                              if ( iLineSize > 0 )
                                bRedraw = TRUE;
                              break;
        case    SB_PAGEUP:    if ( pthis->psLBox.ptOrg.x > 0 )
                              {
                                iLineSize = 0 - iPageSize;
                                bRedraw = TRUE;
                              }
                              break;
        case    SB_PAGEDOWN:  if ( pthis->psLBox.ptOrg.x + iPageSize > imaxPos )
                                  iPageSize = imaxPos - pthis->psLBox.ptOrg.x;
                              if ( iPageSize > 0 )
                              {
                                iLineSize = iPageSize;
                                bRedraw = TRUE;
                              }
                              break;
        
        case    SB_THUMBPOSITION:
        case    SB_THUMBTRACK: pthis->psLBox.ptOrg.x = iPos;
                               iLineSize = 0;
                               bRedraw = TRUE;
                               break;

    }
    if ( bRedraw )
        LView_OnScroll ( hWnd, pthis, iLineSize, 0 ); 

    LView_IReleaseResource ( hRsrc, pthis );

}


void
LVLBox_OnVScroll
(
    HWND                    hWnd,
    int                     iCode,
    int                     iPos
)
{
    RECT                    rc;
    int                     iminPos;
    int                     imaxPos;
    int                     iPageSize;
    int                     iLineSize;
    UINT                    uStyleMask;
	HGLOBAL		            hRsrc   	= ( HGLOBAL )NULL;
	LISTVIEWDATA*	        pthis		= ( LISTVIEWDATA* )NULL;
    BOOL                    bRedraw     = FALSE;

    hRsrc = LView_IGetResource ( hWnd, &pthis );
    assert ( hRsrc );
    uStyleMask = pthis->uStyles & LVS_TYPEMASK;
    GetScrollRange ( hWnd, SB_VERT, &iminPos, &imaxPos );
    GetClientRect ( hWnd, &rc );
    iLineSize = pthis->psLBox.ItmHeight;
    iPageSize = rc.bottom / pthis->psLBox.ItmHeight;
    switch ( iCode )
    {
        case    SB_LINEUP:    if ( pthis->psLBox.ptOrg.y > 0 )
                              {
                                iLineSize = 0 - pthis->psLBox.ItmHeight;
                                if ( uStyleMask == LVS_REPORT ||
                                     uStyleMask == LVS_LIST )
                                     iLineSize = -1;
                                bRedraw = TRUE;
                              }
                              break;
        case    SB_LINEDOWN:  if ( pthis->psLBox.ptOrg.y + iLineSize > imaxPos )
                                iLineSize = imaxPos - pthis->psLBox.ptOrg.y;
                              if ( iLineSize > 0 )
                              {
                                if ( uStyleMask == LVS_REPORT ||
                                     uStyleMask == LVS_LIST )
                                     iLineSize = 1;
                                bRedraw = TRUE;
                              }
                              break;
        case    SB_PAGEUP:    if ( pthis->psLBox.ptOrg.y > 0 )
                              {
                                pthis->psLBox.lTopIndx -= iPageSize;
                                if ( pthis->psLBox.lTopIndx < 0 )
                                  pthis->psLBox.lTopIndx = 0;
                                iLineSize = 0 - iPageSize;
                                bRedraw = TRUE;
                              }
                              break;
        case    SB_PAGEDOWN:  if ( pthis->psLBox.ptOrg.y + iPageSize <= imaxPos )
                                  iPageSize = imaxPos - pthis->psLBox.ptOrg.y;
                              if ( iPageSize > 0 )
                              {
                                pthis->psLBox.lTopIndx += iPageSize;
                                if ( pthis->psLBox.lTopIndx >= pthis->psLBox.lCount - iPageSize )
                                  pthis->psLBox.lTopIndx = pthis->psLBox.lCount - iPageSize;
                                if ( pthis->psLBox.lTopIndx < 0 )
                                  pthis->psLBox.lTopIndx = 0;
                                iLineSize = iPageSize;
                                bRedraw = TRUE;
                              }
                              break;
        case    SB_THUMBPOSITION:
        case    SB_THUMBTRACK: pthis->psLBox.lTopIndx = iPos;
                               iLineSize = 0;
                               bRedraw = TRUE;
                               break;

    }
    if ( bRedraw )
        LView_OnScroll ( hWnd, pthis, 0, iLineSize ); 
    LView_IReleaseResource ( hRsrc, pthis );
}

BOOL
LVEdit_Subclass
(
    HWND                hwLBox
)
{
    WNDPROC             lpEdWndProc = ( WNDPROC )NULL;
    
    lpEdWndProc = ( WNDPROC )SetWindowLong ( hwLBox, GWL_WNDPROC, 
                                        ( LPARAM )( WNDPROC )LVEdit_WndProc ); 
    if ( !lpEdWndProc )
        return TRUE;
    return FALSE;
}


LRESULT WINAPI 
LVEdit_WndProc
(
	HWND				hWnd,
    UINT				uMessage,
    WPARAM				wParam,
    LPARAM				lParam
)
{
    LRESULT             lResult             = 0;
    BOOL                bCallOrigWndProc    = FALSE;
    static int          tmHeight            = 0;
	switch (uMessage)
	{
        case WM_ERASEBKGND:
            {
                RECT    rc;
                HDC     hdc = ( HDC )wParam;
                GetClientRect ( hWnd, &rc );
                Rectangle ( hdc, rc.left, rc.top, rc.right, rc.bottom );
                return -1;
            }
        case WM_GETDLGCODE:
			return ( LRESULT )DLGC_WANTARROWS | DLGC_WANTALLKEYS;
        case WM_SETFONT:
            {
                HDC     hdc;
                TEXTMETRIC tm;
                CallWindowProc( ( FARPROC )GetClassLong ( hWnd, GCL_WNDPROC ),
                                   hWnd, uMessage, wParam, lParam );
                hdc = GetDC ( hWnd );
                GetTextMetrics ( hdc, &tm );
                tmHeight = tm.tmHeight;
                ReleaseDC ( hWnd, hdc );
                break;
            }
	case WM_KEYDOWN:
            CallWindowProc( ( FARPROC )GetClassLong ( hWnd, GCL_WNDPROC ),
                                   hWnd, uMessage, wParam, lParam );
            if ( ( int )wParam == VK_ESCAPE || ( int )wParam == VK_RETURN )
                SendMessage ( GetParent ( hWnd ), uMessage, wParam, lParam );
            else
            {
                RECT    rect;                      
                char    szText[MAXSTRING];
                UINT    edtStyle;
                RECT    rcText;
                HDC     hDC = GetDC ( hWnd );
#if 0
                RECT    rcDad;                      
                SIZE    size;
                HWND    hwParent = GetParent ( hWnd );
                GetWindowRect ( hWnd, &rect );
                MapWindowPoints ( ( HWND )NULL, hwParent, ( LPPOINT )&rect.left, 2 ); 
                GetClientRect ( hwParent, &rcDad );
                GetWindowText ( hWnd, szText, MAXSTRING );
                GetTextExtentPoint32 ( hDC, szText, lstrlen ( szText ), &size ); 
#else
                GetWindowText ( hWnd, szText, MAXSTRING );
                GetWindowRect ( hWnd, &rect );
                SetRect ( &rcText, 0, 0, rect.right - rect.left, rect.bottom - rect.top );
#endif
                edtStyle = GetWindowLong ( hWnd, GWL_STYLE );
                if ( edtStyle & ES_MULTILINE )
                {
                    DrawText ( hDC, szText, lstrlen ( szText ),&rcText, 
                               DT_CALCRECT | DT_WORDBREAK );
#if 1
                    if ( rcText.bottom > rect.bottom - rect.top )
                        SetWindowPos ( hWnd, ( HWND )NULL, 0, 0, 
                                   rect.right-rect.left, rcText.bottom,
                                   SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER );
#endif
                }
                else
                {
                    DrawText ( hDC, szText, lstrlen ( szText ),&rcText, 
                               DT_CALCRECT | DT_LEFT );
                    if ( rcText.right - rcText.left < 50 )
                        rcText.right = rcText.left + 50;
#if 1
                    SetWindowPos ( hWnd, ( HWND )NULL, 0, 0, 
                                   rcText.right - rcText.left + 5, 
                                   rcText.bottom,
                                   SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER );
#endif
                }
                ReleaseDC ( hWnd, hDC );
            }

            break;

        default:
			return CallWindowProc( ( FARPROC )GetClassLong ( hWnd, GCL_WNDPROC ),
                                   hWnd, uMessage, wParam, lParam );
	}
    
	if ( bCallOrigWndProc )
        lResult = CallWindowProc( ( FARPROC )GetClassLong ( hWnd, GCL_WNDPROC ),
                                   hWnd, uMessage, wParam, lParam );
    return lResult;
}




