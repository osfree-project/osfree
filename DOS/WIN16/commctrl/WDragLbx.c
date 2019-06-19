/*  
	WDragListbox
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
#include "windows.h"
#include "Win_User.h"
#include "WCommCtrl.h"
#endif
#include "string.h"
#include "WDragLbx.h"
#include "WResourc.h"

#define  ID_TIMER           16

extern HINSTANCE	hLibInstance;
static char DRAGLBX_PROP[] = "DragLbx_Data";	
static HCURSOR	g_hDragCursor = ( HCURSOR )NULL;
static HCURSOR	g_hDefCursor = ( HCURSOR )NULL;
enum { eNoDrag = 0, eEndDrag = eNoDrag, eCancelDrag,
       eLBtnDown, eBeginDrag, eDragging };

typedef struct 
{
	WNDPROC     lpLbWndProc;
    	UINT        DragMsg;
    	BOOL        bDragMsg; /*controlled by timer to regulate the DL_DRAGGING message stream */
	int	    enDrag;	 /* for dragging */
	int	    ItmSelected;
    	int         ItmCount;
    	RECT        Rect;
    	RECT        iconRect;
} DRAGLBXDATA;

#define GETTHISDATA(hwnd) \
   GetProp(hwnd, DRAGLBX_PROP )
#define SETTHISDATA(hwnd, pthisNew) \
   SetProp(hwnd, DRAGLBX_PROP, pthisNew)
#define REMOVETHISDATA(hwnd) \
   RemoveProp(hwnd, DRAGLBX_PROP )

/*prototypes*/
static LRESULT WINAPI 
DragLb_WndProc
(
	HWND				hWnd,
    	UINT				uMessage,
    	WPARAM				wParam,
    	LPARAM				lParam
);


/*Win32 API */
BOOL WINAPI
WMakeDragList
(
    	HWND                	hwLB
)
{
	HGLOBAL			hDragLbx	= ( HGLOBAL )NULL;
	DRAGLBXDATA*		pthis		= ( DRAGLBXDATA* )NULL;

	int		    	ErrorCode	= 0;

	if ( ! ( hDragLbx = GlobalAlloc ( GHND, sizeof ( DRAGLBXDATA ) ) ) )
		ErrorCode = WDB_ERR_GLOBALALLOC;
	else
	if ( ! ( pthis = ( DRAGLBXDATA *) GlobalLock ( hDragLbx ) ) )
		ErrorCode = WDB_ERR_GLOBALLOCK;
	else
	{
        GetClientRect ( hwLB, &pthis->Rect );
        if ( pthis->ItmCount == LB_ERR )
            pthis->ItmCount = 0;
        pthis->DragMsg = RegisterWindowMessage ( DRAGLISTMSGSTRING );
        pthis->lpLbWndProc = ( WNDPROC )SetWindowLong ( hwLB, GWL_WNDPROC, 
                                        ( LPARAM )( WNDPROC )DragLb_WndProc ); 
        if ( !pthis->lpLbWndProc )
            ErrorCode = WDB_ERR_SETWINDOWLONG;

        SETTHISDATA ( hwLB, hDragLbx );
	}

	if ( pthis )
		GlobalUnlock ( hDragLbx );
	return ( ErrorCode >= 0 );
}

int WINAPI
WLBItemFromPt
(
    HWND            hwLB,	
    POINT           pt,	
    BOOL            bAutoScroll	
)
{
    RECT            rect;
    long            lStyles;
    int             topIndx;
    int             count;
    int             itemHeight;
    int             iIndx   = -1;
        
    GetWindowRect ( hwLB, &rect );
    if ( !PtInRect ( &rect, pt ) )
        return -1;
    count = SendMessage ( hwLB, LB_GETCOUNT, 0, 0L );
    if ( count == 0 )
        return 0;
    lStyles = GetWindowLong ( hwLB, GWL_STYLE );
    topIndx = SendMessage ( hwLB, LB_GETTOPINDEX, 0, 0L );
    ScreenToClient ( hwLB, &pt );
    if ( lStyles & LBS_OWNERDRAWVARIABLE )
    {
        int height = 0;
        for ( iIndx = topIndx; iIndx < count; iIndx++ )
        {
            itemHeight = SendMessage ( hwLB, LB_GETITEMHEIGHT, iIndx, 0L );
            if ( height < pt.y )
                height += itemHeight;
            if ( height >= pt.y )
                break;
        }
    }
    else
    {
        itemHeight = SendMessage ( hwLB, LB_GETITEMHEIGHT, 0, 0L );
        iIndx = pt.y / itemHeight;
        iIndx += topIndx; 
    }

    if ( iIndx >= count ) 
        iIndx = -1;
    return iIndx;
}

void WINAPI
WDrawInsert
(
    HWND            hwParent,	
    HWND            hwLB,	
    int             nItem	
)
{
    RECT            rect;
    int             count;
    HICON           hIcon;
    HDC             hDC             = GetDC ( hwParent );

	HGLOBAL			hDragLbx	    = ( HGLOBAL )NULL;
	DRAGLBXDATA*	pthis		    = ( DRAGLBXDATA* )NULL;
	int				ErrorCode	    = 0;
	
	if ( ! ( hDragLbx = GETTHISDATA ( hwLB ) ) )
		ErrorCode = WDB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( DRAGLBXDATA *) GlobalLock ( hDragLbx ) ) )
		ErrorCode = WDB_ERR_GLOBALLOCK;
	else
    {
        count = SendMessage ( hwLB, LB_GETCOUNT, 0, 0L );
        if ( nItem < 0 || nItem >= count )
        {
            InvalidateRect ( hwParent, &pthis->iconRect, TRUE );
            UpdateWindow ( hwParent );
            SetRectEmpty ( &pthis->iconRect );
            return;
        }
        SendMessage ( hwLB, LB_GETITEMRECT, nItem, ( LPARAM )&rect );
        MapWindowPoints ( hwLB, hwParent, ( LPPOINT )&rect.left, 2 );
        OffsetRect ( &rect, -32, -16 );
        if ( pthis->iconRect.left != rect.left || 
             pthis->iconRect.top  != rect.top )
        {
            InvalidateRect ( hwParent, &pthis->iconRect, TRUE );
            UpdateWindow ( hwParent );
#ifdef _WINDOWS
            hIcon = LoadIcon ( GetWindowLong ( hwLB, GWL_HINSTANCE ),
                                        MAKEINTRESOURCE ( IDI_DRAGLBX_ARROW ) );
#else
            hIcon = LoadIcon ( hLibInstance,
                                        MAKEINTRESOURCE ( IDI_DRAGLBX_ARROW ) );
#endif
            DrawIcon ( hDC, rect.left, rect.top, hIcon );
            DestroyIcon ( hIcon );
            SetRect ( &pthis->iconRect, rect.left, rect.top,
                                        rect.left + 32, rect.top + 32 );
        }
    }

	if ( pthis )
		GlobalUnlock ( hDragLbx );
    ReleaseDC ( hwParent, hDC );
}

static void 
DragLb_OnLButtonDown 
(	
	HWND			hWnd, 
	int			x, 
	int			y, 
	UINT			keyFlags
) 
{
	int				ItmClicked;
	HGLOBAL			hDragLbx	    = ( HGLOBAL )NULL;
	DRAGLBXDATA*	pthis		    = ( DRAGLBXDATA* )NULL;
	int				ErrorCode	    = 0;
	
	if ( ! ( hDragLbx = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WDB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( DRAGLBXDATA *) GlobalLock ( hDragLbx ) ) )
		ErrorCode = WDB_ERR_GLOBALLOCK;
	else
	if ( ( ItmClicked = SendMessage ( hWnd, LB_GETCURSEL, 0, 0L ) ) != LB_ERR )
	{
		pthis->ItmSelected = ItmClicked;
		pthis->enDrag = eLBtnDown;
	}

	if ( pthis )
		GlobalUnlock ( hDragLbx );
}


static void
DragLb_OnLButtonUp 
(	
	HWND			hWnd, 
	int				x, 
	int				y, 
	UINT			keyFlags
) 
{
    DRAGLISTINFO    dli;      
	HGLOBAL			hDragLbx	    = ( HGLOBAL )NULL;
	DRAGLBXDATA*	pthis		    = ( DRAGLBXDATA* )NULL;
	BOOL			bSendMessage	= FALSE;

	int				ErrorCode	= 0;
	
	if ( ! ( hDragLbx = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WDB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( DRAGLBXDATA* ) GlobalLock ( hDragLbx ) ) )
		ErrorCode = WDB_ERR_GLOBALLOCK;
	else
	{
		bSendMessage = FALSE;
		if ( pthis->enDrag == eDragging )
            bSendMessage = TRUE;
		ReleaseCapture ();
		SetCursor ( g_hDefCursor );
        KillTimer ( hWnd, ID_TIMER );
	    pthis->enDrag = eEndDrag;

        if ( bSendMessage )
        {
            dli.hWnd = hWnd;
            dli.uNotification = DL_DROPPED;
            dli.ptCursor.x = x;
		    dli.ptCursor.y = y;
            MapWindowPoints ( hWnd, HWND_DESKTOP, ( LPPOINT )&dli.ptCursor, 1 );

            SendMessage ( GetParent ( hWnd ), pthis->DragMsg, GetDlgCtrlID ( hWnd ),
                                            ( LPARAM )&dli );
        }
	}

    if ( pthis )
		GlobalUnlock ( hDragLbx );
}

static void 
DragLb_OnDestroy 
(	
	HWND				hWnd 
)
{
	HGLOBAL				hDragLbx	= ( HGLOBAL )NULL;
	DRAGLBXDATA*		pthis		= ( DRAGLBXDATA* )NULL;

	int				ErrorCode	= 0;

	if ( ! ( hDragLbx = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WDB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( DRAGLBXDATA *) GlobalLock ( hDragLbx ) ) )
		ErrorCode = WDB_ERR_GLOBALLOCK;
	else
	{
		REMOVETHISDATA ( hWnd );
		GlobalFree ( hDragLbx );
	}
}

static void 
DragLb_OnMouseMove 
(
	HWND			hWnd, 
	UINT			wParam,
	LPARAM			lParam 
) 
{
    	DRAGLISTINFO    	dli;      
	HGLOBAL			hDragLbx	= ( HGLOBAL )NULL;
	DRAGLBXDATA*		pthis		= ( DRAGLBXDATA* )NULL;
    	BOOL            	bSendMsg    	= FALSE;
	int			ErrorCode	= 0;

	if ( ! ( hDragLbx = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WDB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( DRAGLBXDATA *) GlobalLock ( hDragLbx ) ) )
		ErrorCode = WDB_ERR_GLOBALLOCK;
	else
	if ( pthis->enDrag == eLBtnDown )
		pthis->enDrag = eBeginDrag;
	else
	if ( pthis->enDrag == eBeginDrag )
	{
        	dli.uNotification = DL_BEGINDRAG;
        	bSendMsg = TRUE;
		pthis->enDrag = eDragging;
		if ( !g_hDragCursor )
			g_hDragCursor = LoadCursor ( ( HINSTANCE )GetWindowLong ( GetParent ( hWnd ),GWL_HINSTANCE ), 
					MAKEINTRESOURCE ( IDC_CURSOR_DRAG ) );
		if ( g_hDragCursor )
			g_hDefCursor = SetCursor ( g_hDragCursor );
        	SetCapture ( hWnd );
        	SetTimer ( hWnd, ID_TIMER, 50, NULL );
	}
    else
	if ( pthis->bDragMsg && pthis->enDrag == eDragging )
    {   /* this portion is invoked by the timer */
        pthis->bDragMsg = FALSE;
        dli.uNotification = DL_DRAGGING;
        bSendMsg = TRUE;
    }

    if ( bSendMsg )
    {
	LRESULT result;
        dli.hWnd = hWnd;
		dli.ptCursor.x = LOWORD ( lParam );
		dli.ptCursor.y = HIWORD ( lParam );
        MapWindowPoints ( hWnd, HWND_DESKTOP, ( LPPOINT )&dli.ptCursor, 1 );
	/* bug : result is always zero, which is wrong. To test, look into
	   function Toolbr_DoDragDrop in WTlbCust.c */
        result = SendMessage ( GetParent ( hWnd ), pthis->DragMsg, GetDlgCtrlID ( hWnd ),
                                        ( LPARAM )&dli );
    }

	if ( pthis )
		GlobalUnlock ( hDragLbx );
}


static int 
DragLb_DoDrag 
(
	HWND			hWnd
) 
{
	HGLOBAL			hDragLbx	= ( HGLOBAL )NULL;
	DRAGLBXDATA*		pthis		= ( DRAGLBXDATA* )NULL;
	int			ErrorCode	= 0;

	if ( ! ( hDragLbx = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WDB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( DRAGLBXDATA *) GlobalLock ( hDragLbx ) ) )
		ErrorCode = WDB_ERR_GLOBALLOCK;
	else
	{
        POINT pt;
        int indx;
        int Count;
        GetCursorPos ( &pt );
        ScreenToClient ( hWnd, &pt );
        if ( PtInRect ( &pthis->Rect, pt ) )
            pthis->bDragMsg = TRUE;
        else
        if ( pt.x > pthis->Rect.left && pt.x < pthis->Rect.right )
        {
            indx = SendMessage ( hWnd, LB_GETTOPINDEX, 0, 0L );
            Count = SendMessage ( hWnd, LB_GETCOUNT, 0, 0L );
            indx = pt.y < pthis->Rect.top ? indx - 1 :
                pt.y > pthis->Rect.bottom ? indx + 1 : -1;
            if ( indx >= 0 && indx < Count )
                SendMessage ( hWnd, LB_SETTOPINDEX, indx, 0L ); 
            pthis->bDragMsg = FALSE;
        }
	}

	if ( pthis )
		GlobalUnlock ( hDragLbx );

	return 0;	
}

/*new*/

static LRESULT WINAPI 
DragLb_WndProc
(
	HWND			hWnd,
    	UINT			uMessage,
    	WPARAM			wParam,
    	LPARAM			lParam
)
{
    	LRESULT             	lResult             = 0;
    	BOOL                	bCallOrigWndProc    = TRUE;
	switch (uMessage)
	{
		case WM_DESTROY:
			DragLb_OnDestroy ( hWnd );
			break;
		
		case WM_GETDLGCODE:
			return ( LRESULT )DLGC_WANTARROWS | DLGC_WANTCHARS;
		
		case WM_LBUTTONDOWN:
            		CallWindowProc( ( FARPROC )GetClassLong ( hWnd, GCL_WNDPROC ),
                                   hWnd, uMessage, wParam, lParam );
			DragLb_OnLButtonDown ( hWnd,(int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )( wParam ) );
            		bCallOrigWndProc = FALSE;
			SetFocus ( hWnd );
			break;

		case WM_LBUTTONUP:
			DragLb_OnLButtonUp ( hWnd,(int)(short)LOWORD(lParam), 
				    ( int )( short )HIWORD ( lParam ), ( UINT )( wParam ) );
			break;

		case WM_MOUSEMOVE:
			DragLb_OnMouseMove ( hWnd, ( UINT )wParam, lParam );
            		bCallOrigWndProc = FALSE;
			break;

       		case WM_TIMER:
            		DragLb_DoDrag ( hWnd );

        	default:
			return CallWindowProc( ( FARPROC )GetClassLong ( hWnd, GCL_WNDPROC ),
                                   hWnd, uMessage, wParam, lParam );
	}
    
	if ( bCallOrigWndProc )
        	lResult = CallWindowProc( ( FARPROC )GetClassLong ( hWnd, GCL_WNDPROC ),
                                   hWnd, uMessage, wParam, lParam );
    	return lResult;
}

