/*  
	WHdr.c
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
#include "string.h"
#include "WLists.h"
#include "WResourc.h"
#include "WHdr.h"

#define HHT_DIVOPEN_START       6
#define HHT_DIVOPEN_END         10
#define HHT_DIVIDER_SPAN        5
#define HD_DEFITEMWIDTH         10
#define HDMETRIC_CYBORDER       3
#define WHD_HORIZONTALBORDER    6

#ifndef _WINDOWS
#define  GetTextExtentPoint32   GetTextExtentPoint
#define  SetWindowOrgEx(x,y,z,h)    SetWindowOrg(x,y,z)
#endif

enum { eNone, eOverItem, eOverDivider, eOverDivOpen,
       eItemClk, eDragDivider, eDragDivOpen };

static char Headr_PROP[] = "Headr_Data";	

#define GETTHISDATA(hwnd) \
   GetProp(hwnd, Headr_PROP )
#define SETTHISDATA(hwnd, pthisNew) \
   SetProp(hwnd, Headr_PROP, pthisNew)
#define REMOVETHISDATA(hwnd) \
   RemoveProp(hwnd, Headr_PROP )

extern HINSTANCE hLibInstance;

static int Headr_OnEraseBkground ( HWND hWnd, HDC hDC );

static LRESULT  
Headr_HDMWndProc ( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam );

static void
Headr_IDrawBitmap 
( 
    HDC                     hDC, 
    HBITMAP                 hBitmap, 
    UINT                    fmt,
    LPRECT                  pRect 
)
{
	BITMAP                  Bitmap;
	HGDIOBJ                 hOldObject;
	HDC                     hMemoryDC = ( HDC )NULL;
                            

	if ((hMemoryDC = CreateCompatibleDC ( hDC ) ) )
	{
		GetObject ( hBitmap, sizeof ( BITMAP ), &Bitmap );
		hOldObject = SelectObject ( hMemoryDC, hBitmap );
		if ( fmt & HDF_CENTER )
			BitBlt ( hDC, 
			         pRect->left + ( pRect->right - pRect->left - Bitmap.bmWidth ) / 2, 
			         pRect->top + ( pRect->bottom - pRect->top - Bitmap.bmHeight ) / 2,
			         Bitmap.bmWidth, Bitmap.bmHeight, hMemoryDC, 0, 0, SRCCOPY );
		else
		if ( fmt & HDF_RIGHT )
			BitBlt ( hDC,
			         pRect->right - WHD_HORIZONTALBORDER - Bitmap.bmWidth, 
			         pRect->top + ( pRect->bottom - pRect->top - Bitmap.bmHeight ) / 2,
			         Bitmap.bmWidth, Bitmap.bmHeight, hMemoryDC, 0, 0, SRCCOPY );
		else
			BitBlt ( hDC,
			         pRect->left + WHD_HORIZONTALBORDER,
			         pRect->top + ( pRect->bottom - pRect->top - Bitmap.bmHeight ) / 2,
			         Bitmap.bmWidth, Bitmap.bmHeight, hMemoryDC, 0, 0, SRCCOPY );
		SelectObject ( hMemoryDC, hOldObject );
	}

	if ( hMemoryDC )
		DeleteDC ( hMemoryDC );

}

static void
Headr_IDrawFittedString 
( 
    HDC                     hDC, 
    LPSTR                   lpText, 
    UINT                    fmt,
    LPRECT                  lpRect 
)
{
    SIZE                    size;
    int                     pixLen;
    char                    buf[MAXSTRING];
    char                    dots[] = "...";
    int                     iLen = lstrlen ( lpText );

    GetTextExtentPoint32 ( hDC, lpText, iLen, &size );
    pixLen = lpRect->right - lpRect->left;

    if ( pixLen < size.cx )
    {
        int ifmtLen;
        TEXTMETRIC tm;
        GetTextMetrics ( hDC, &tm );
        iLen -= ( size.cx - pixLen ) / tm.tmAveCharWidth;
        ifmtLen = iLen > 3? iLen - 3 : 0;
        if ( ifmtLen > 0 )
            lstrcpyn ( buf, lpText, ifmtLen + 1 );
        else
            *buf = 0;
        lstrcat ( buf, dots );
        lpText = buf;
    }
   
    DrawText ( hDC, lpText, iLen, lpRect, fmt );
}

static BOOL
Headr_IGetItemRect
(
    HWND                    hWnd, 
	HEADERDATA*	            pthis,
    int                     iItem,
    LPRECT                  lpRect
)
{
    PHEADERITEM             pItems      = ( PHEADERITEM )NULL;
    PHEADERITEM             pItem       = ( PHEADERITEM )NULL;

    if ( WLLock ( &pthis->Items, ( LPVOID )&pItems ) )
        return FALSE;

    pItem = pItems + iItem;
    CopyRect ( lpRect, &pItem->rc );
    WLUnlock ( &pthis->Items, ( LPVOID )NULL );

    return TRUE;   
}

static void
Headr_IOwnerDraw
(
    HWND                    hWnd,
    HDC                     hDC, 
	HEADERDATA*	            pthis,
    int                     iItem,
    PHEADERITEM             pItm
)
{
    DRAWITEMSTRUCT          dis;

    memset ( &dis, 0, sizeof ( DRAWITEMSTRUCT ) );
    dis.CtlType = 0;
    dis.CtlID = GetDlgCtrlID ( hWnd );
    dis.hwndItem = hWnd;
    dis.hDC = hDC;
    dis.itemData = pItm->data.lParam;
    dis.itemID = iItem;
    CopyRect ( &dis.rcItem, &pItm->rc );
    dis.itemState = pthis->eMouse == eItemClk && iItem == pthis->iCurSel ? ODS_SELECTED : 0;
    
    SendMessage ( GetParent ( hWnd ), WM_DRAWITEM, dis.CtlID, ( LPARAM )&dis );
}

static LRESULT
Headr_ISendNotify 
(
    HWND                hWnd,
    UINT                uNCode,
    int                 iItem,
    HD_ITEM*            phdi
)
{
    HD_NOTIFY           hdNotify;

    memset ( &hdNotify, 0, sizeof ( HD_NOTIFY ) );
    hdNotify.hdr.code = uNCode;
    hdNotify.hdr.hwndFrom = hWnd;
    hdNotify.hdr.idFrom = GetDlgCtrlID ( hWnd );
    hdNotify.iItem = iItem;
    hdNotify.pitem = phdi;
    return SendMessage ( GetParent ( hWnd ), 
                         WM_NOTIFY, 
                         hdNotify.hdr.idFrom, 
                         ( LPARAM )&hdNotify );

}
/* iItem = -1: recalc rectangles of all items. Else, recalc iItemth dimension and
 * all items going after it.
 */
static BOOL
Headr_ISetItemRects
(
    HWND                hWnd,
    PHEADERDATA         pthis,
    int                 iItem
)
{
    RECT                rc;
    int                 i;
    PHEADERITEM         pItems      = ( PHEADERITEM )NULL;
    PHEADERITEM         pItem       = ( PHEADERITEM )NULL;

    if ( WLLock ( &pthis->Items, ( LPVOID )&pItems ) )
        return FALSE;
    if ( iItem < 0 )
        iItem = 0;
    GetClientRect ( hWnd, &rc );
    rc.right = rc.left;
    for ( i = 0; i < pthis->iCount; i++ )
    {
        pItem = pItems + i;
        rc.right += ( pItem->data.mask & HDI_WIDTH )? pItem->data.cxy : HD_DEFITEMWIDTH;
        if ( i >= iItem )
            SetRect ( &pItem->rc, rc.left, rc.top, rc.right - 1, rc.bottom );
        rc.left = rc.right;
    }

    WLUnlock ( &pthis->Items, ( LPVOID )NULL);
    pthis->bRecalc = FALSE;
    return TRUE;
}

#ifdef NEVER
/* new */
static BOOL
Headr_IGetItmText
(
    HWND                hWnd,
    LV_ITEM*            pLVItem,
    LPSTR*              lppText  
)
{
    LV_DISPINFO             lvdi;
    lvdi.hdr.code = LVN_GETDISPINFO;
    lvdi.hdr.hwndFrom = hWnd;
    lvdi.hdr.idFrom = GetDlgCtrlID ( hWnd );
    lvdi.item = *pLVItem;
    lvdi.item.mask = LVIF_TEXT;
/*    lvdi.item.iSubItem = 0;*/
    SendMessage ( GetParent ( hWnd ), WM_NOTIFY, lvdi.hdr.idFrom, ( LPARAM )&lvdi );
    *lppText = lvdi.item.pszText;
    return TRUE;
}
#endif

static BOOL 
Headr_OnCreate 
(
	HWND				hWnd, 
	CREATESTRUCT*		pcs
) 
{
    HGLOBAL				hHeader	    = ( HGLOBAL )NULL;
	HEADERDATA*		    pthis		= ( HEADERDATA* )NULL;

	int				    ErrorCode	= 0;

	if ( ! ( hHeader = GlobalAlloc ( GHND, sizeof ( HEADERDATA ) ) ) )
		ErrorCode = WHDR_ERR_GLOBALALLOC;
	else
	if ( ! ( pthis = ( HEADERDATA *) GlobalLock ( hHeader ) ) )
		ErrorCode = WHDR_ERR_GLOBALLOCK;
	else
	{
		SETTHISDATA(hWnd, hHeader);
		pthis->uStyles = pcs->style;
		pthis->hFont = ( HFONT )GetStockObject ( ANSI_VAR_FONT );
        pthis->hDivider = LoadCursor ( hLibInstance, 
                                       MAKEINTRESOURCE ( IDC_DIVIDER ) );
        pthis->hDivOpen = LoadCursor ( hLibInstance, 
                                       MAKEINTRESOURCE ( IDC_DIVOPEN ) );
        pthis->hArrow = LoadCursor( ( HINSTANCE )NULL, 
                                       MAKEINTRESOURCE ( IDC_ARROW ) );
        pthis->iCurSel = -1;

        WLCreate ( &pthis->Items, sizeof ( HEADERITEM ) );
	}

	if ( pthis )
		GlobalUnlock ( hHeader );
	return ( ErrorCode >= 0 );
}

void
Headr_OnDrawItem
(
    HWND                    hWnd,
    HDC                     hDC, 
	HEADERDATA*	            pthis,
    int                     iItem,
    PHEADERITEM             pItm
)
{
    RECT                    rcDraw;
    HFONT                   hOFont;
    HPEN                    hOPen;
    UINT                    fmt;
	HPEN                    hGPen = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNSHADOW ) );
	HPEN                    hWPen = GetStockObject ( WHITE_PEN );

    CopyRect ( &rcDraw, &pItm->rc );
	FillRect ( hDC, &rcDraw, GetStockObject ( LTGRAY_BRUSH ) );
    InflateRect ( &rcDraw, -HDMETRIC_CYBORDER, -2 );
    if ( pthis->eMouse == eItemClk )
        OffsetRect ( &rcDraw, 1, 1 );
    fmt = DT_VCENTER;        
    fmt |= ( pItm->data.fmt & HDF_CENTER )? DT_CENTER :
           ( pItm->data.fmt & HDF_RIGHT )? DT_RIGHT : DT_LEFT;
    if ( ( pItm->data.fmt & HDF_STRING ) && pItm->text )
    {
        hOFont = SelectObject ( hDC, pthis->hFont );
        SetBkMode ( hDC, TRANSPARENT );
        Headr_IDrawFittedString ( hDC, pItm->text, fmt, &rcDraw );
        SelectObject ( hDC, hOFont );
    }
    else
    if ( ( pItm->data.fmt & HDF_BITMAP ) && pItm->data.hbm )
        Headr_IDrawBitmap ( hDC, pItm->data.hbm, fmt, &rcDraw );
    else
    if ( pItm->data.fmt & HDF_OWNERDRAW )
    {
        Headr_IOwnerDraw ( hWnd, hDC, pthis, iItem, pItm ); 
        goto EndDrawItem;
    }

    if ( pthis->eMouse == eItemClk )
    {
		hOPen = SelectObject ( hDC, hGPen );
		MoveToEx ( hDC, pItm->rc.left, pItm->rc.bottom, ( LPPOINT )NULL );
		LineTo ( hDC, pItm->rc.left, pItm->rc.top );
		LineTo ( hDC, pItm->rc.right, pItm->rc.top );
		SelectObject ( hDC, hWPen );
		LineTo ( hDC, pItm->rc.right, pItm->rc.bottom );
		LineTo ( hDC, pItm->rc.left, pItm->rc.bottom );
		SelectObject ( hDC, hOPen );
    }
    else
    {
		hOPen = SelectObject ( hDC, hGPen );
		MoveToEx ( hDC, pItm->rc.right, pItm->rc.top, ( LPPOINT )NULL );
		LineTo ( hDC, pItm->rc.right, pItm->rc.bottom );
		LineTo ( hDC, pItm->rc.left, pItm->rc.bottom );
        if ( pthis->uStyles & HDS_BUTTONS )
        {
		    MoveToEx ( hDC, pItm->rc.right-1, pItm->rc.top+1, ( LPPOINT )NULL );
		    LineTo ( hDC, pItm->rc.right-1, pItm->rc.bottom-1 );
		    LineTo ( hDC, pItm->rc.left, pItm->rc.bottom-1 );
            SelectObject ( hDC, hWPen );
        }

        LineTo ( hDC, pItm->rc.left, pItm->rc.top );
		LineTo ( hDC, pItm->rc.right, pItm->rc.top );
		SelectObject ( hDC, hOPen );
    }

EndDrawItem:    
    DeleteObject ( hGPen );

}

static void 
Headr_OnLButtonDown 
(
    HWND            hWnd, 
    int             x, 
    int             y, 
    UINT            keyFlags
) 
{
    HD_HITTESTINFO  ht;
	HGLOBAL			hHeader	    = ( HGLOBAL )NULL;
	HEADERDATA*	    pthis		= ( HEADERDATA* )NULL;
    PHEADERITEM     pItems      = ( PHEADERITEM )NULL;
	int			    ErrorCode	= 0;

	if ( ! ( hHeader = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WHDR_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( HEADERDATA *) GlobalLock ( hHeader ) ) )
		ErrorCode = WHDR_ERR_GLOBALLOCK;
	else
    if ( pthis->eMouse == eOverDivider )
    {
        if ( WLLock ( &pthis->Items, ( LPVOID )&pItems ) == 0 )
        {
            PHEADERITEM pItem = pItems + pthis->iCurSel;
            if ( !Headr_ISendNotify ( hWnd, HDN_BEGINTRACK, pthis->iCurSel, 
                                      &pItem->data ) )
                pthis->eMouse = eDragDivider;
            WLUnlock ( &pthis->Items, ( LPVOID )NULL );
        }
    }
    else
    if ( pthis->eMouse == eOverDivOpen )
    {
/*        SetCapture ( hWnd );*/
        pthis->eMouse = eDragDivOpen;
    }
    else
    {
        int iClick;
        ht.pt.x = x;
        ht.pt.y = y;
        if ( ( iClick = Headr_OnHitTest ( hWnd, pthis, &ht ) ) != -1 &&
             ht.flags & HHT_ONHEADER )
	    {
            HDC                 hDC         = GetDC ( hWnd );
            pthis->eMouse = eItemClk;
            pthis->iCurSel = iClick;
            if ( WLLock ( &pthis->Items, ( LPVOID )&pItems ) == 0 )
            {
                Headr_OnDrawItem ( hWnd, hDC, pthis, pthis->iCurSel, pItems + pthis->iCurSel );
                WLUnlock ( &pthis->Items, ( LPVOID )NULL );
            }
            ReleaseDC ( hWnd, hDC );
        }
    }

    if ( pthis )
		GlobalUnlock ( hHeader );
}

static void 
Headr_OnLButtonUp 
(
    HWND            hWnd, 
    int             x, 
    int             y, 
    UINT            keyFlags,
    BOOL            bWindowsMsg
) 
{
	HGLOBAL			hHeader	    = ( HGLOBAL )NULL;
	HEADERDATA*	    pthis		= ( HEADERDATA* )NULL;
	int			    ErrorCode	= 0;
    BOOL            bRedraw     = FALSE;

	if ( ! ( hHeader = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WHDR_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( HEADERDATA *) GlobalLock ( hHeader ) ) )
		ErrorCode = WHDR_ERR_GLOBALLOCK;
	else
    if ( bWindowsMsg )
    {
        if ( pthis->eMouse == eDragDivider ||
             pthis->eMouse == eDragDivOpen )
        {
            int             mode;
            RECT            rc;
            HD_ITEM         hdi;
            HDC             hDC         = GetDC ( hWnd );

            GetClientRect ( hWnd, &rc );
            mode = SetROP2 ( hDC, R2_NOT );
            Rectangle ( hDC, pthis->ptDrag.x, rc.top, pthis->ptDrag.x+2, rc.bottom );
            SetROP2 ( hDC, mode );
            pthis->ptDrag.x = -2;
            ReleaseDC ( hWnd, hDC );
            pthis->eMouse = eNone;
            ReleaseCapture();

            Headr_IGetItemRect ( hWnd, pthis, pthis->iCurSel, &rc );
            hdi.mask = HDI_WIDTH;
            hdi.cxy = x - rc.left;
            Headr_OnSetItem ( hWnd, pthis, pthis->iCurSel, &hdi );
            hdi.mask = HDI_WIDTH | HDI_BITMAP | HDI_TEXT | HDI_FORMAT;
            Headr_OnGetItem ( hWnd, pthis, pthis->iCurSel, &hdi );
            Headr_ISendNotify ( hWnd, HDN_ENDTRACK, pthis->iCurSel, &hdi );
        }
        else
        if ( pthis->eMouse == eItemClk )
            Headr_ISendNotify ( hWnd, HDN_ITEMCLICK, pthis->iCurSel, NULL );
        else
            bRedraw = TRUE;
        pthis->eMouse = eNone;
    }
    else
    if ( pthis->eMouse == eItemClk )
    {
        pthis->eMouse = eOverItem;
        bRedraw = TRUE;
    }
    else
        pthis->eMouse = eNone;

    if ( bRedraw )
    {
        PHEADERITEM         pItems      = ( PHEADERITEM )NULL;
        HDC                 hDC         = GetDC ( hWnd );
        if ( WLLock ( &pthis->Items, ( LPVOID )&pItems ) == 0 )
        {
            Headr_OnDrawItem ( hWnd, hDC, pthis, pthis->iCurSel, pItems + pthis->iCurSel );
            WLUnlock ( &pthis->Items, ( LPVOID )NULL );
        }
        ReleaseDC ( hWnd, hDC );
    }


    if ( pthis )
		GlobalUnlock ( hHeader );
}

/*Need revisit this*/
static void 
Headr_OnDestroy ( HWND hWnd )
{
	HGLOBAL				hHeader	= ( HGLOBAL )NULL;
	HEADERDATA*		    pthis		= ( HEADERDATA* )NULL;

	int				    ErrorCode	= 0;

	if ( ! ( hHeader = GlobalAlloc ( GHND, sizeof ( HEADERDATA ) ) ) )
		ErrorCode = WHDR_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( HEADERDATA *) GlobalLock ( hHeader ) ) )
		ErrorCode = WHDR_ERR_GLOBALLOCK;
	else
	{
        WLDestroy ( &pthis->Items );
        if ( pthis )
            GlobalUnlock ( hHeader );
        REMOVETHISDATA ( hWnd );
		GlobalFree ( hHeader );
	}
}


static void 
Headr_OnMouseMove 
(
	HWND			hWnd,
    UINT            uMessage,
	int			    x, 
	int			    y, 
	UINT			keyFlags
) 
{
    int             iOver;
    HD_HITTESTINFO  ht;
	HGLOBAL			hHeader	    = ( HGLOBAL )NULL;
	HEADERDATA*	    pthis		= ( HEADERDATA* )NULL;
	int			    ErrorCode	= 0;

	if ( ! ( hHeader = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WHDR_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( HEADERDATA *) GlobalLock ( hHeader ) ) )
		ErrorCode = WHDR_ERR_GLOBALLOCK;
	else
    {
#if 1
        if ( pthis->eMouse != eDragDivider &&
             pthis->eMouse != eOverDivider &&
             pthis->eMouse != eDragDivOpen &&
             pthis->eMouse != eDragDivOpen )
            SetCursor ( pthis->hArrow );
#endif
        ht.pt.x = x;
        ht.pt.y = y;
        if ( pthis->eMouse == eDragDivider ||
             pthis->eMouse == eDragDivOpen )
        {
            int             mode;
            RECT            rc;
            HD_ITEM         hdi;
            HDC             hDC         = GetDC ( hWnd );

            GetClientRect ( hWnd, &rc );
            mode = SetROP2 ( hDC, R2_NOT );
            Rectangle ( hDC, pthis->ptDrag.x, rc.top, pthis->ptDrag.x+2, rc.bottom );
            Rectangle ( hDC, x, rc.top, x+2, rc.bottom );
            SetROP2 ( hDC, mode );
            pthis->ptDrag.x = x;
            ReleaseDC ( hWnd, hDC );

            hdi.mask = HDI_WIDTH | HDI_BITMAP | HDI_TEXT | HDI_FORMAT;
            Headr_OnGetItem ( hWnd, pthis, pthis->iCurSel, &hdi );
            Headr_ISendNotify ( hWnd, HDN_TRACK, pthis->iCurSel, &hdi );
        }
        else
        if ( ( iOver = Headr_OnHitTest ( hWnd, pthis, &ht ) ) != -1 )
	    {
            if ( pthis->eMouse == eItemClk )
            {
                if ( pthis->iCurSel != iOver )
                    Headr_OnLButtonUp ( hWnd, x, y, keyFlags, FALSE );
            }
            else
            if ( pthis->eMouse == eOverItem )
            {
                if ( pthis->iCurSel == iOver )
                    Headr_OnLButtonDown ( hWnd, x, y, keyFlags );
            }
            else
            if ( pthis->eMouse == eNone && ht.flags & HHT_ONDIVIDER )
            {
                pthis->eMouse = eOverDivider;
                pthis->iCurSel = iOver;
                SetCursor ( pthis->hDivider );
            }
            else
            if ( pthis->eMouse == eNone && ht.flags & HHT_ONDIVOPEN )
            {
                pthis->eMouse = eOverDivOpen;
                pthis->iCurSel = iOver;
                SetCursor ( pthis->hDivOpen );    
            }
            else
            {
                pthis->eMouse = eNone;
            }
            SetCapture ( hWnd );
	    }
        else
        {
            ReleaseCapture ();
        }
    }

    if ( pthis )
		GlobalUnlock ( hHeader );
}


#if 1
static int 
Headr_OnEraseBkground 
(
	HWND			hWnd, 
	HDC				hDC 
)
{
    HGLOBAL			hHeader	= ( HGLOBAL )NULL;
	HEADERDATA*	pthis		= ( HEADERDATA* )NULL;
	RECT			rect;
	HPEN			hPen        = GetStockObject ( BLACK_PEN );
	HBRUSH			hBrush	    = ( HBRUSH )NULL;
	int			    ErrorCode	= 0;
    int             Result      = 0;

	if ( ! ( hHeader = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WHDR_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( HEADERDATA *) GlobalLock ( hHeader ) ) )
		ErrorCode = WHDR_ERR_GLOBALLOCK;
	else
	if ( hDC )
	{
        if ( pthis->BkColor == CLR_NONE )
            hBrush = CreateSolidBrush ( GetSysColor ( COLOR_WINDOW ) );
        else
            hBrush = CreateSolidBrush ( pthis->BkColor );
		hPen = SelectObject ( hDC, hPen );
		hBrush = SelectObject ( hDC, hBrush );
		GetClientRect ( hWnd, &rect );
		Rectangle ( hDC, rect.left, rect.top,
					   rect.right, rect.bottom );
		hPen = SelectObject ( hDC, hPen );
		hBrush = SelectObject ( hDC, hBrush );
        DeleteObject ( hBrush );
		Result = 1;
	}

	if ( pthis )
		GlobalUnlock ( hHeader );
	return Result;

}
#endif

/* in progress */
static void 
Headr_OnPaint 
(
	HWND				hWnd, 
	HDC			    	hPaintDC 
)
{
	PAINTSTRUCT         ps;
	HGLOBAL			    hHeader	    = ( HGLOBAL )NULL;
	HEADERDATA*		    pthis		= ( HEADERDATA* )NULL;
	
	int			        ErrorCode	= 0;

	if ( ! ( hHeader = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WHDR_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( HEADERDATA *) GlobalLock ( hHeader ) ) )
		ErrorCode = WHDR_ERR_GLOBALLOCK;
	else
	{
        int   i;
        RECT    rc;
        PHEADERITEM         pItems      = ( PHEADERITEM )NULL;
        PHEADERITEM         pItem       = ( PHEADERITEM )NULL;

        GetClientRect ( hWnd, &rc );


        /* Start painting.*/
		BeginPaint(hWnd, &ps);
        if ( pthis->bRecalc )
            Headr_ISetItemRects ( hWnd, pthis, -1 );
        pthis->eMouse = eNone;
        if ( WLLock ( &pthis->Items, ( LPVOID )&pItems ) == 0 )
        {
            for ( i = 0; i < pthis->iCount; i++ )
            {
                pItem = pItems + i;
                Headr_OnDrawItem ( hWnd, ps.hdc, pthis, i, pItem );
            }
            if ( pItem->rc.right < rc.right )
            {
                HEADERITEM  hdi;
                memset ( &hdi, 0, sizeof ( HEADERITEM ) );
                hdi.data.mask = HDI_WIDTH;
                hdi.data.cxy = rc.right - pItem->rc.right;
                SetRect ( &hdi.rc, pItem->rc.right+1, pItem->rc.top,
                                   rc.right+1, rc.bottom );
                Headr_OnDrawItem ( hWnd, ps.hdc, pthis, i, &hdi );
            }

            WLUnlock ( &pthis->Items, ( LPVOID )NULL );
        }
		EndPaint(hWnd, &ps);
	}
	if ( pthis )
		GlobalUnlock ( hHeader );
}

static LRESULT WINAPI 
Headr_WndProc
(
	    HWND			hWnd,
        UINT			uMessage,
        WPARAM			wParam,
        LPARAM			lParam
)
{
	switch (uMessage)
	{
		case WM_CREATE:
			if ( ! Headr_OnCreate ( hWnd, ( LPCREATESTRUCT )lParam ) )
				return ( LRESULT ) -1L;
			break;
		case WM_DESTROY:
			Headr_OnDestroy ( hWnd );
			break;
#if 1
		case WM_ERASEBKGND:
			return ( LRESULT )Headr_OnEraseBkground ( hWnd, ( HDC )wParam );
			break;
#endif
		case WM_GETDLGCODE:
			return ( LRESULT )DLGC_WANTTAB | DLGC_WANTARROWS;

		case WM_LBUTTONDOWN:
			Headr_OnLButtonDown ( hWnd, (int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )wParam );
			break;

        case WM_LBUTTONUP:
			Headr_OnLButtonUp ( hWnd, (int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )wParam, TRUE );
			break;

        case WM_MOUSEMOVE:
			Headr_OnMouseMove ( hWnd, uMessage, (int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )wParam );
			break;

        case WM_PAINT:
			Headr_OnPaint( hWnd, ( HDC ) wParam  );
			break;

        case WM_SETFOCUS:
            SetFocus ( GetParent ( hWnd ) );
			break;

		default:  
		    if ( uMessage >= HDM_FIRST )
			    return Headr_HDMWndProc(hWnd, uMessage, wParam, lParam);
		    return DefWindowProc(hWnd, uMessage, wParam, lParam);
	}
    
	    return 0;
}

/* new */
static BOOL
Headr_OnDeleteItem
( 
    HWND                hWnd, 
    HEADERDATA*         pthis,
    int                 iItem
)
{
    if ( iItem >= pthis->iCount )
        return FALSE;
    if ( WLDelete ( &pthis->Items, iItem ) )
        return FALSE;
    pthis->iCount--;
    InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE );
    return TRUE;
}

static BOOL
Headr_OnGetItem
( 
    HWND                hWnd, 
    HEADERDATA*         pthis,
    int                 iItem,
    HD_ITEM*            lphdi
)
{
    BOOL                bGood;
    PHEADERITEM         pItems      = ( PHEADERITEM )NULL;
    PHEADERITEM         pItem       = ( PHEADERITEM )NULL;

    if ( iItem > pthis->iCount );
        return FALSE;
    if ( lphdi->mask == 0 )
        return TRUE;
    if ( WLLock ( &pthis->Items, ( LPVOID )&pItems ) )
        return FALSE;
    pItem = pItems + iItem;

    if ( lphdi->mask & HDI_TEXT )
    {
        if ( lphdi->pszText == 0 || !( pItem->data.mask & HDI_TEXT ) )
            goto HdrGetItmExit;
        lstrcpyn ( lphdi->pszText, pItem->text, 
                   min ( pItem->data.cchTextMax, lphdi->cchTextMax ) );
    }
    if ( lphdi->mask & HDI_BITMAP )
    {
        if ( !( pItem->data.mask & HDI_BITMAP ) )
            goto HdrGetItmExit;
        lphdi->hbm = pItem->data.hbm;
    }
    if ( lphdi->mask & HDI_FORMAT )
    {
        if ( !( pItem->data.mask & HDI_FORMAT ) )
            goto HdrGetItmExit;
        lphdi->fmt = pItem->data.fmt;
    }
    if ( lphdi->mask & HDI_HEIGHT )
    {
        if ( !( pItem->data.mask & HDI_HEIGHT ) )
            goto HdrGetItmExit;
        lphdi->cxy = pItem->data.cxy;
    }
    if ( lphdi->mask & HDI_LPARAM )
    {
        if ( !( pItem->data.mask & HDI_LPARAM ) )
            goto HdrGetItmExit;
        lphdi->lParam = pItem->data.lParam;
    }
    
    bGood = TRUE;

HdrGetItmExit:
    WLUnlock ( &pthis->Items, ( LPVOID )NULL );
    
    return bGood;
}


/* new */
static int
Headr_OnInsertItem 
( 
    HWND                hWnd, 
    HEADERDATA*         pthis,
    int                 iItem,
    HD_ITEM*            lphdi 
)
{
    HEADERITEM          hdri;
    PHEADERITEM         pItems      = ( PHEADERITEM )NULL;
    PHEADERITEM         pItem       = ( PHEADERITEM )NULL;

    memset ( &hdri, 0, sizeof ( HEADERITEM ) );

    memcpy ( &hdri.data, lphdi, sizeof ( HD_ITEM ) );
    if ( iItem > pthis->iCount );
        iItem = pthis->iCount;
    if ( WLInsert ( &pthis->Items, iItem, &hdri ) )
        return -1;
    
    if ( WLLock ( &pthis->Items, ( LPVOID )&pItems ) )
        return -1;
    pItem = pItems + iItem;
    /*memcpy ( &pItem->data, lphdi, sizeof ( HD_ITEM ) );*/
    if ( lphdi->mask & HDI_TEXT )
    {
        pItem->data.pszText = pItem->text;
        if ( pItem->data.cchTextMax >= MAXSTRING )
            pItem->data.cchTextMax = MAXSTRING - 1;
        lstrcpyn ( pItem->text, lphdi->pszText, pItem->data.cchTextMax + 1 );
    }

    if ( pItem->data.fmt == 0 )
    {
        pItem->data.fmt = HDF_LEFT;
        pItem->data.fmt |= pItem->data.mask & HDI_TEXT ? HDF_STRING : HDF_BITMAP;
    }
    WLUnlock ( &pthis->Items, ( LPVOID )NULL );
    pthis->iCount++;
    pthis->bRecalc = TRUE;

    return iItem;
}

/* in progress */
static int
Headr_OnHitTest
( 
    HWND                hWnd, 
    HEADERDATA*         pthis,
    HD_HITTESTINFO*     lpht
)
{
    int                 i;
    RECT                rc;
    POINT               pt;
    PHEADERITEM         pItems      = ( PHEADERITEM )NULL;
    PHEADERITEM         pItem       = ( PHEADERITEM )NULL;
    /*int                 iHit        = -1;*/


    GetClientRect ( hWnd, &rc );
    pt = lpht->pt;
    lpht->flags = 0;
    lpht->iItem = -1;
    if ( pt.x < rc.left )
        lpht->flags = HHT_TOLEFT;
    else
    if ( pt.x > rc.right )
        lpht->flags = HHT_TORIGHT;

    if ( pt.y < rc.top )
        lpht->flags |= HHT_ABOVE;
    else
    if ( pt.y > rc.bottom )
        lpht->flags |= HHT_BELOW;

    if ( lpht->flags )
        return -1;

    lpht->flags = HHT_ONHEADER;
    if ( WLLock ( &pthis->Items, ( LPVOID )&pItems ) )
        return FALSE;

    for ( i = 0; i < pthis->iCount; i++ )
    {
        if ( i < pthis->iCount - 1 )
        {
            pItem = pItems + i + 1;
            if ( pItem->rc.left >= pItem->rc.right &&
                 pt.x <= pItem->rc.left + HHT_DIVOPEN_START &&
                 pt.x >= pItem->rc.left
               )
            {
                lpht->flags |= HHT_ONDIVOPEN;
                lpht->iItem = i + 1;
                goto HitTestExit;
            }
        }

        pItem = pItems + i;
        if ( pt.x <= pItem->rc.right + HHT_DIVIDER_SPAN &&
             pt.x >= pItem->rc.right - HHT_DIVIDER_SPAN
           )
        {
            lpht->flags |= HHT_ONDIVIDER;
            lpht->iItem = i;
            goto HitTestExit;
        }
        else
        if ( pt.x <= pItem->rc.right &&
             pt.x >= pItem->rc.left
           )
        {
            lpht->iItem = i;
            goto HitTestExit;
        }

    }

    lpht->flags = HHT_NOWHERE;

HitTestExit:
    WLUnlock ( &pthis->Items, ( LPVOID )NULL );

    return lpht->iItem;
}

static BOOL
Headr_OnLayout 
( 
    HWND                hWnd, 
    HEADERDATA*         pthis,
    HD_LAYOUT*          lphl 
)
{
    TEXTMETRIC          tm;
    HFONT               hOldFont;
    HDC                 hDC     = GetDC ( hWnd );

    lphl->pwpos->x = lphl->prc->left;
    lphl->pwpos->y = lphl->prc->top;
    lphl->pwpos->cx = lphl->prc->right - lphl->prc->left;
    hOldFont = SelectObject ( hDC, pthis->hFont );
    GetTextMetrics ( hDC, &tm );
    SelectObject ( hDC, hOldFont );
    lphl->pwpos->cy = tm.tmHeight + 2 * HDMETRIC_CYBORDER;

    ReleaseDC ( hWnd, hDC );
    return TRUE;
}

static BOOL
Headr_OnSetItem
( 
    HWND                hWnd, 
    HEADERDATA*         pthis,
    int                 iItem,
    HD_ITEM*            lphdi
)
{
    PHEADERITEM         pItems      = ( PHEADERITEM )NULL;
    PHEADERITEM         pItem       = ( PHEADERITEM )NULL;

    if ( iItem > pthis->iCount )
        return FALSE;
    if ( WLLock ( &pthis->Items, ( LPVOID )&pItems ) )
        return FALSE;
    pItem = pItems + iItem;
    if ( Headr_ISendNotify ( hWnd, HDN_ITEMCHANGING, iItem, &pItem->data ) == TRUE )
    {
        WLUnlock ( &pthis->Items, ( LPVOID )NULL );
        return FALSE;                 
    }

    if ( lphdi->mask & HDI_TEXT )
    {
        pItem->data.pszText = pItem->text;
        if ( pItem->data.cchTextMax >= MAXSTRING )
            pItem->data.cchTextMax = MAXSTRING - 1;
        lstrcpyn ( pItem->text, lphdi->pszText, pItem->data.cchTextMax );
        pItem->data.mask |= HDI_TEXT;
    }

    if ( lphdi->mask & HDI_BITMAP )
    {
        pItem->data.hbm = lphdi->hbm;
        pItem->data.mask |= HDI_BITMAP;
    }
    if ( lphdi->mask & HDI_FORMAT )
    {
        pItem->data.fmt = lphdi->fmt;
        pItem->data.mask |= HDI_FORMAT;
    }
    if ( lphdi->mask & HDI_WIDTH )
    {
        pItem->data.cxy = lphdi->cxy;
        if ( pItem->data.cxy < 0 )
            pItem->data.cxy = 0;
        pItem->data.mask |= HDI_WIDTH;
    }
    if ( lphdi->mask & HDI_LPARAM )
    {
        pItem->data.lParam = lphdi->lParam;
        pItem->data.mask |= HDI_LPARAM;
    }
    
    Headr_ISendNotify ( hWnd, HDN_ITEMCHANGED, iItem, &pItem->data );
    WLUnlock ( &pthis->Items, ( LPVOID )NULL );
    if ( lphdi->mask & HDI_WIDTH )
        Headr_ISetItemRects ( hWnd, pthis, iItem );
    
    return TRUE;
}

static LRESULT  
Headr_HDMWndProc
(
	HWND			hWnd,
    UINT			uMessage,
    WPARAM			wParam,
    LPARAM			lParam
)
{
	HGLOBAL			hHeader	    = ( HGLOBAL )NULL;
	HEADERDATA*	    pthis		= ( HEADERDATA* )NULL;
	
	int			    ErrorCode	= 0;

	if ( ! ( hHeader = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WHDR_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( HEADERDATA *) GlobalLock ( hHeader ) ) )
		ErrorCode = WHDR_ERR_GLOBALLOCK;
	else
	{
		switch (uMessage)
		{
            case HDM_DELETEITEM:
                return ( LRESULT )Headr_OnDeleteItem ( hWnd, pthis, ( int )wParam );
            case HDM_GETITEM:
                return ( LRESULT )Headr_OnGetItem ( hWnd, pthis, ( int )wParam, ( HD_ITEM* )lParam );
            case HDM_GETITEMCOUNT:
                return ( LRESULT )pthis->iCount;
            case HDM_HITTEST:
                return ( LRESULT )Headr_OnHitTest ( hWnd, pthis, ( HD_HITTESTINFO* )lParam );
            case HDM_INSERTITEM:
                return ( LRESULT )Headr_OnInsertItem ( hWnd, pthis, ( int )wParam, ( HD_ITEM* )lParam );  
            case HDM_LAYOUT:
                return ( LRESULT )Headr_OnLayout ( hWnd, pthis, ( HD_LAYOUT* )lParam );  
            case HDM_SETITEM:
                return ( LRESULT )Headr_OnSetItem ( hWnd, pthis, ( int )wParam, ( HD_ITEM* )lParam );

            default:
			    return DefWindowProc(hWnd, uMessage, wParam, lParam);
		}
	}

	if ( pthis )
		GlobalUnlock ( hHeader );

	return 0;
}


int WINAPI 
Headr_Initialize 
(
	HINSTANCE		hinst
) 
{

	WNDCLASS		wc;
	int			ErrorCode	= 0;

	memset ( &wc, 0, sizeof ( WNDCLASS ) );
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_GLOBALCLASS;
	wc.lpfnWndProc   = Headr_WndProc;
	wc.cbWndExtra    = 0; /*sizeof( TView_WNDEXTRABYTES );*/
	wc.hInstance     = hinst;
	wc.hCursor       = ( HCURSOR )NULL; /*LoadCursor( ( HINSTANCE )NULL, IDC_ARROW);*/
	wc.hbrBackground = GetStockObject ( WHITE_BRUSH );
	wc.lpszClassName = WHD_CLASSNAME;

	if ( ! RegisterClass ( &wc ) )
		ErrorCode = WHDR_ERR_REGISTERCLASS;

	return ( ErrorCode );
}

void WINAPI 
Headr_Terminate 
(
	HINSTANCE			hInstance
) 
{
	WNDCLASS			WndClass;


	if ( GetClassInfo ( hInstance, WHD_CLASSNAME, &WndClass ) )
		UnregisterClass ( WHD_CLASSNAME, hInstance );

}





