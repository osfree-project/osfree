/*  
	WTab.c	1.10 
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

/* 

Note about tab control tooltip:

If a tab control is solely responsible for sending message TTM_ADDTOOL
to the tooltip whether it's created with flag TCS_TOOLTIPS or assigned a 
tooltip by the application.

The app doesn't need to set up a hook or have the tooltip subclass a tab control
in order to display the tooltip for the tab control. The tab control itself would 
relay the mousemove message to the tooltip using TTM_RELAYEVENT. Also, even 
though transparently to the app, the tab control would set it up as target to 
receive TTN_NEEDTXT from the tooltip and and forward the message to its parent 
after putting the index of the tab item the mouse is hovering over as wParam.
The lParam of message TTN_NEEDTXT sent to tab control's parent would be pointing 
to a TOOLTIPTEXT structure whose hdr member structure would have tooltip window 
handle as hwnd, tab control handle or ID, dependent on whether the flag is 
TTF_IDISHWND, as id and TTN_NEEDTXT as code.

*/

#ifdef _WINDOWS
#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#else
#include "WCommCtrl.h"
#endif
#include "string.h"
#include "WLists.h"
#include "WTab.h"
#include "WUpdown.h"
#include "WTooltip.h"
#include "WImgList.h"

#ifndef _WINDOWS
#define  GetTextExtentPoint32   GetTextExtentPoint
#endif

#define	 MAX_LABELLENGTH	40
#define  VERT_PADDING		2
#define	 HORZ_PADDING		5
#define	 HORZ_GAP		3
#define	 HORZ_MARGIN		4
#define	 VERT_MARGIN		5
#define  SCROLLER_WIDTH		20
#define  SCROLLER_HEIGHT	15

static char TABCONTROL_PROP[] = "TabCtrl_Data";	


typedef struct 
{
	RECT	Rect;  
	SIZE	Size;
	int	Row;
	BOOL	bClipped;

	char    szLabel[MAX_LABELLENGTH];
	UINT	mask;
	int	iImage;
	LPSTR	pszText;
	LPARAM	lParam;
} TABITEM;


typedef struct 
{
	RECT		Rect; /*the area that doesn't include the tab items*/
	int		Rows;

	BOOL		bFocus;

	HWND		hwSpin;
	HWND		hwTTip;
	HFONT		hFont;

	LPARAM		hImageL;
	int		ImgCX;
	int		ImgCY;
	BOOL		bTabItmResized;

	int		nXPadding;
	int		nYPadding;
	UINT		uStyles;

	int		ItmHeight;
	int		ItmWidth;
	int		ItmCount;
	int		ItmSelected;
	int		ItmVisible;  /* the leftmost visible tab item*/
	int		ItmTip; /* the tab item for which a tooltip is going to be displayed*/

	int		cbExtra; /* to handle TCM_SETITEMEXTRA*/
	WLIST		TabItmList; /*array of Tab Items*/
} TABCTLDATA;

#define GETTHISDATA(hwnd) \
   GetProp(hwnd, TABCONTROL_PROP )
#define SETTHISDATA(hwnd, pthisNew) \
   SetProp(hwnd, TABCONTROL_PROP, pthisNew)
#define REMOVETHISDATA(hwnd) \
   RemoveProp(hwnd, TABCONTROL_PROP )


/*prototypes*/
static BOOL
TabItem_DrawFocusRect
(
	HDC			hDC,
	TABCTLDATA*		pthis
);


static BOOL
TabItem_IsVisible
(
	TABITEM*			pTabItem
);

static void
TabCtl_IResetItmVisible
(
	TABCTLDATA*			pthis,
	int				iSelected
);


static void
TabCtl_SetTooltip 
( 
	HWND				hWnd, 
	HWND				hwTTip 
);

static void
TabCtl_ICalcItemSize 
( 
	HDC				hDC,
	TABCTLDATA*			pthis, 
	TABITEM*			ptabi,
	BOOL				bAllowForImage
);	       

static void
TabCtl_IGetTabItmContentRect 
( 
	HDC				hDC,
	TABCTLDATA*			pthis,
	TABITEM*			pTabItm,
	RECT*				prcIcon,
	RECT*				prcText
);


static void
TabCtl_IPaintTabCtlBorder
(
	HDC				hDC,
	TABCTLDATA*			pthis,
	TABITEM*			pTabItemList
);

static void
TabCtl_IPaintTab 
( 
	HWND				hWnd,
	HDC				hDC, 
	TABCTLDATA*			pthis, 
	TABITEM*			pTabItem,
	int				nItem 
);

static void
TabCtl_IReCalcSize 
( 
	HDC				hDC,
	TABCTLDATA*			pthis 
);


static void
TabCtl_ISelChange 
( 
	HWND				hwTabCtl,
	TABCTLDATA*			pthis, 
	int				iSelected
);


static void
TabCtl_ISetItemRects 
( 
	HWND				hwTabCtl,
	TABCTLDATA*			pthis
);

static void
TabCtl_ISetMultiLRects 
( 
	HWND				hwTabCtl,
	TABCTLDATA*			pthis
);

static int
TabCtl_ITabFromPt 
( 
	HWND				hwTabCtl,
	TABCTLDATA*			pthis, 
	int				x,
	int				y,
	UINT*				uHitFlag
);

static LRESULT  
TabCtl_TCMWndProc
(
	HWND				hWnd,
        UINT				uMessage,
        WPARAM				wParam,
        LPARAM				lParam
);


/*Implementation*/
static void
TabCtl_AdjustRect
( 
	HWND				hwTabCtl,
	TABCTLDATA*			pthis,
	BOOL				bLarger,
	RECT*				lpRect
)
{
	if ( !lpRect )
		return;

	if ( bLarger )
	{
		lpRect->left -= HORZ_MARGIN;
		lpRect->right += HORZ_MARGIN;
		lpRect->top -= ( pthis->ItmHeight * pthis->Rows + VERT_MARGIN );
		lpRect->bottom += VERT_MARGIN;
	}
	else
	{
		lpRect->left += HORZ_MARGIN;
		lpRect->right -= HORZ_MARGIN;
		lpRect->top += ( pthis->ItmHeight * pthis->Rows + VERT_MARGIN );
		lpRect->bottom -= VERT_MARGIN;
	}
}

static BOOL
TabCtl_DeleteItems
(
	HWND				hwTabCtl,
	TABCTLDATA*			pthis, 
	int				ItmIndx 
)
{
	BOOL				result = FALSE;

	if ( ItmIndx == -1 )
	{
		if ( WLDestroy ( &pthis->TabItmList ) == 0 )
		{
			WLCreate ( &pthis->TabItmList, sizeof ( TABITEM ) );	
			pthis->ItmCount = 0;
			pthis->ItmSelected = pthis->ItmVisible = pthis->ItmTip = -1;
			result = TRUE;
		}
	}
	else
	if ( ItmIndx >= 0 && ItmIndx < pthis->ItmCount )
	{
		if ( WLDelete ( &pthis->TabItmList, ItmIndx ) == 0 )
		{
			pthis->ItmCount--;
			result = TRUE;
		}
	}

	return result;
}

static BOOL
TabCtl_GetItem 
( 
	HWND				hwTabCtl,
	TABCTLDATA*			pthis, 
	int				ItmIndx, 
	TC_ITEM*			ptci 
)
{
	TABITEM*			pTabItemList	= ( TABITEM* )NULL;
	TABITEM*			pTabItem	= ( TABITEM* )NULL;
	BOOL				Result		= FALSE;
	int				ErrorCode	= 0;

	if ( !ptci || ItmIndx < 0 || ItmIndx >= pthis->ItmCount )
		return FALSE;

	if ( WLLock ( &pthis->TabItmList, ( LPVOID )&pTabItemList ) )
		ErrorCode = WTC_ERR_LISTLOCK;
	else
	{
		pTabItem = pTabItemList + ItmIndx;
		ptci->mask = pTabItem->mask;
		ptci->pszText = pTabItem->szLabel;    
		ptci->cchTextMax = MAX_LABELLENGTH;   
		ptci->iImage = pTabItem->iImage;  
		ptci->lParam = pTabItem->lParam;  
		if ( pthis->cbExtra )
		{
			void* psExtra = &ptci->lParam;
			void* pdExtra = &pTabItem->lParam;
			memcpy ( pdExtra, psExtra, pthis->cbExtra );
		}
		Result = TRUE;
	}

	if ( pTabItemList )
		WLUnlock ( &pthis->TabItmList, ( LPVOID )&pTabItemList );
	return Result;

}

static BOOL
TabCtl_GetItemRect 
( 
	HWND				hwTabCtl,
	TABCTLDATA*			pthis, 
	int				ItmIndx, 
	RECT*				pRect 
)
{
	TABITEM*			pTabItemList	= ( TABITEM* )NULL;
	TABITEM*			pTabItem	= ( TABITEM* )NULL;
	BOOL				Result		= FALSE;
	int				ErrorCode	= 0;

	if ( !pRect || ItmIndx < 0 || ItmIndx >= pthis->ItmCount )
		return FALSE;

	if ( WLLock ( &pthis->TabItmList, ( LPVOID )&pTabItemList ) )
		ErrorCode = WTC_ERR_LISTLOCK;
	else
	{
		pTabItem = pTabItemList + ItmIndx;
		SetRect ( pRect, pTabItem->Rect.left, pTabItem->Rect.top,
				 pTabItem->Rect.right, pTabItem->Rect.bottom );
		Result = TRUE;
	}

	if ( pTabItemList )
		WLUnlock ( &pthis->TabItmList, ( LPVOID )&pTabItemList );
	return Result;

}


static int
TabCtl_HitTest 
( 
	HWND				hwTabCtl,
	TABCTLDATA*			pthis,
	TC_HITTESTINFO*			ptch 
)
{
	int				TabSelected;

	TabSelected = TabCtl_ITabFromPt ( hwTabCtl, pthis, ptch->pt.x, ptch->pt.y, &ptch->flags );

	return  TabSelected;
	
}


static int
TabCtl_InsertItem 
( 
	HWND				hwTabCtl,
	TABCTLDATA*			pthis, 
	int				ItmIndx, 
	TC_ITEM*			ptci 
)
{
	TABITEM*			ptabi;
	HGLOBAL				hInsert		= ( HGLOBAL )NULL;
	LPVOID				lpInsert	= ( LPVOID )NULL;
	HDC				hDC		= GetDC ( hwTabCtl );
	UINT				Count		= 0;
	int				ErrorCode	= 0;

	if ( ! ( hInsert = GlobalAlloc ( GHND, sizeof ( TABITEM ) + pthis->cbExtra ) ) )
		ErrorCode = WTC_ERR_GLOBALALLOC;
	else
	if ( ! ( lpInsert = GlobalLock ( hInsert ) ) )
		ErrorCode = WTC_ERR_GLOBALLOCK;
	else
	{
		ptabi = ( TABITEM* )lpInsert;
		ptabi->mask = ptci->mask;
		ptabi->iImage = ptci->iImage;
		ptabi->lParam = ptci->lParam;
		if ( ptci->mask & TCIF_TEXT )
		{
			int Len = lstrlen ( ptci->pszText );
			if ( ++Len >= MAX_LABELLENGTH )
				Len = MAX_LABELLENGTH - 1;
#if 1
                        lstrcpyn ( ptabi->szLabel, ptci->pszText, Len );
#else
                        lstrcpy ( ptabi->szLabel, ptci->pszText );
#endif		
		}

		if ( pthis->cbExtra )
		{
			void* psExtra = &ptabi->lParam;
			void* pdExtra = &ptci->lParam;
			memcpy ( pdExtra, psExtra, pthis->cbExtra );
		}

		
		WLCount ( &pthis->TabItmList, &Count );
		if ( ItmIndx > Count )
			ItmIndx = Count;

		TabCtl_ICalcItemSize ( hDC, pthis, ptabi, pthis->hImageL ? TRUE : FALSE );

		if ( WLInsert ( &pthis->TabItmList, ItmIndx, lpInsert ) < 0 )
			ErrorCode = WTC_ERR_LISTINSERT;
		else
		{
			if ( pthis->ItmVisible < 0 )
				pthis->ItmSelected = pthis->ItmVisible = 0;
			pthis->ItmCount++;
			/*TabCtl_ISetItemRects ( hwTabCtl, pthis );*/
		}
	}
	

	if ( hDC )
		ReleaseDC ( hwTabCtl, hDC );
	if ( lpInsert )
	{
		GlobalUnlock ( hInsert );
		GlobalFree ( hInsert );
	}

	return  ( ErrorCode >= 0 ? ItmIndx : -1 );
	
}


static BOOL
TabCtl_OnCreate 
(
	HWND				hWnd, 
	CREATESTRUCT*			pcs
) 
{
	HGLOBAL				hTabCtl		= ( HGLOBAL )NULL;
	TABCTLDATA*			pthis		= ( TABCTLDATA* )NULL;

	int				ErrorCode	= 0;

	if ( ! ( hTabCtl = GlobalAlloc ( GHND, sizeof ( TABCTLDATA ) ) ) )
		ErrorCode = WTC_ERR_GLOBALALLOC;
	else
	if ( ! ( pthis = ( TABCTLDATA *) GlobalLock ( hTabCtl ) ) )
		ErrorCode = WTC_ERR_GLOBALLOCK;
	else
	{
		pthis->uStyles = pcs->style;
		pthis->ItmSelected = -1;
		pthis->ItmVisible = -1;
		pthis->ItmHeight = 20;
		pthis->Rows = 1;
		pthis->nXPadding = HORZ_PADDING;
		pthis->nYPadding = VERT_PADDING;
		pthis->hFont = ( HFONT )GetStockObject ( ANSI_VAR_FONT );
		pthis->bTabItmResized = TRUE;
		WLCreate ( &pthis->TabItmList, sizeof ( TABITEM ) );
		if ( ! ( pthis->uStyles & TCS_MULTILINE ) )
		{
			pthis->hwSpin = CreateWindowEx ( 0, WC_UPDOWN, NULL, 
							WS_CHILD | UDS_HORZ, 0, 0, 0, 0,
							hWnd, ( HMENU )NULL, pcs->hInstance, ( LPVOID )NULL );
			SendMessage ( pthis->hwSpin, UDM_SETBUDDY, ( WPARAM )hWnd, 0L );
		}

		if ( pthis->uStyles & TCS_TOOLTIPS )
		{
			pthis->hwTTip = CreateWindowEx ( 0, WC_TOOLTIP, ( LPSTR ) NULL, 
				WS_POPUP | WS_BORDER | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, 
				CW_USEDEFAULT, CW_USEDEFAULT, pcs->hwndParent, ( HMENU )NULL, 
				pcs->hInstance, NULL); 

			TabCtl_SetTooltip ( hWnd, pthis->hwTTip );
		}

		if ( pthis->uStyles & TCS_OWNERDRAWFIXED )
		{
			MEASUREITEMSTRUCT mi;
			memset ( &mi, 0, sizeof (MEASUREITEMSTRUCT ) );
			mi.CtlID = GetDlgCtrlID ( hWnd );
			SendMessage ( pcs->hwndParent, WM_MEASUREITEM, 
					GetDlgCtrlID ( hWnd ), ( LPARAM )&mi );
			pthis->ItmWidth = mi.itemWidth;
			pthis->ItmHeight = mi.itemHeight;
			pthis->uStyles |= TCS_FIXEDWIDTH;
		}	
		GetClientRect ( hWnd, &pthis->Rect );
		SETTHISDATA(hWnd, hTabCtl);
	}

	if ( pthis ) 
		GlobalUnlock ( hTabCtl );

	return ( ErrorCode >= 0 );
}

static int 
TabCtl_OnHScroll 
(
	HWND				hWnd, 
	int				nScrollCode, 
	int				nPos, 
	HWND				hwScroller
) 
{
	RECT				rect;
	HGLOBAL				hTabCtl		= ( HGLOBAL )NULL;
	TABCTLDATA*			pthis		= ( TABCTLDATA* )NULL;

	int				ErrorCode	= 0;
	int				Result		= -1;

	if ( ! ( hTabCtl = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTC_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TABCTLDATA *) GlobalLock ( hTabCtl ) ) )
		ErrorCode = WTC_ERR_GLOBALLOCK;
	else
	if ( pthis->hwSpin == hwScroller )
	{
		if ( nScrollCode == SB_LINERIGHT && 
		     pthis->ItmVisible + 1 < pthis->ItmCount )
		     pthis->ItmVisible++;
		if ( nScrollCode == SB_LINELEFT && 
		     pthis->ItmVisible - 1 >= 0 )
		     pthis->ItmVisible--;
		TabCtl_ISetItemRects ( hWnd, pthis );
		
		GetClientRect ( hWnd, &rect );
		rect.bottom = pthis->Rect.top + VERT_MARGIN;
		InvalidateRect ( hWnd, &rect, TRUE );
		if ( pthis->hwSpin )
			ValidateRect ( pthis->hwSpin, ( RECT* )NULL );
		Result = 0;

	}

	if ( pthis )
		GlobalUnlock ( hTabCtl );
	
	return Result;

}


static void TabCtl_OnKeyDown 
(
	HWND				hWnd, 
	int				VKey, 
	int				cRepeat, 
	UINT				keyFlags
) 
{
	int				TabSelected	= 0;
	HGLOBAL				hTabCtl		= ( HGLOBAL )NULL;
	TABCTLDATA*			pthis		= ( TABCTLDATA* )NULL;
	BOOL				bSelChange	= FALSE;

	int				ErrorCode	= 0;
	
	if ( ! ( hTabCtl = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTC_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TABCTLDATA *) GlobalLock ( hTabCtl ) ) )
		ErrorCode = WTC_ERR_GLOBALLOCK;
	else
/*	if ( pthis->uStyles & UDS_ARROWKEYS )*/
	{
		switch ( VKey )
		{
			case VK_RIGHT	:
			case VK_DOWN	:
				if ( pthis->ItmSelected + 1 < pthis->ItmCount )
				{
					TabSelected = pthis->ItmSelected + 1;
					bSelChange = TRUE;
				}
				break;
			case VK_LEFT	:
			case VK_UP	:
				if ( pthis->ItmSelected > 0 )
				{
					TabSelected = pthis->ItmSelected - 1;
					bSelChange = TRUE;
				}
				break;
			case VK_HOME	:
				TabSelected = 0;
				bSelChange = TRUE;
				break;
			case VK_END	:
				TabSelected = pthis->ItmCount - 1;
				bSelChange = TRUE;
				break;
		}
	}

	if ( bSelChange  )
	{
		TabCtl_ISelChange ( hWnd, pthis, TabSelected );
	}
	if ( pthis )
		GlobalUnlock ( hTabCtl );
}


static void TabCtl_OnKeyUp (HWND hWnd, int VKey, int cRepeat, UINT keyFlags) 
{
	HGLOBAL				hTabCtl		= ( HGLOBAL )NULL;
	TABCTLDATA*			pthis		= ( TABCTLDATA* )NULL;

	int				ErrorCode	= 0;
	
	if ( ! ( hTabCtl = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTC_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TABCTLDATA *) GlobalLock ( hTabCtl ) ) )
		ErrorCode = WTC_ERR_GLOBALLOCK;
	else
	if ( pthis->uStyles & UDS_ARROWKEYS )
	{
	}

	if ( pthis )
		GlobalUnlock ( hTabCtl );
}


static void TabCtl_OnLButtonDown 
(	
	HWND				hWnd, 
	int				x, 
	int				y, 
	UINT				keyFlags
) 
{
	int				TabSelected	= 0;
	HGLOBAL				hTabCtl		= ( HGLOBAL )NULL;
	TABCTLDATA*			pthis		= ( TABCTLDATA* )NULL;
	int				ErrorCode	= 0;
	
	if ( ! ( hTabCtl = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTC_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TABCTLDATA *) GlobalLock ( hTabCtl ) ) )
		ErrorCode = WTC_ERR_GLOBALLOCK;
	else
	{
		if ( ( TabSelected = TabCtl_ITabFromPt ( hWnd, pthis, x, y, NULL ) ) != -1 )
		{
			if ( TabSelected != pthis->ItmSelected )
				TabCtl_ISelChange ( hWnd, pthis, TabSelected );
		}
		if ( pthis->uStyles & TCS_FOCUSONBUTTONDOWN )
			SetFocus ( hWnd );
	}
	if ( pthis )
		GlobalUnlock ( hTabCtl );
}


static void 
TabCtl_OnKillFocus 
(	
	HWND				hWnd
) 
{
	HGLOBAL				hTabCtl		= ( HGLOBAL )NULL;
	TABCTLDATA*			pthis		= ( TABCTLDATA* )NULL;
	HDC				hDC		= GetDC ( hWnd );
	int				ErrorCode	= 0;
	
	if ( ! ( hTabCtl = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTC_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TABCTLDATA *) GlobalLock ( hTabCtl ) ) )
		ErrorCode = WTC_ERR_GLOBALLOCK;
	else
	if ( pthis->bFocus )
	{
		TabItem_DrawFocusRect ( hDC, pthis );
		pthis->bFocus = FALSE;
	}
	
	if ( pthis )
		GlobalUnlock ( hTabCtl );
	if ( hDC )
		ReleaseDC ( hWnd, hDC );
}


static void 
TabCtl_OnSetFocus 
(	
	HWND				hWnd
) 
{
	HGLOBAL				hTabCtl		= ( HGLOBAL )NULL;
	TABCTLDATA*			pthis		= ( TABCTLDATA* )NULL;
	HDC				hDC		= GetDC ( hWnd );
	int				ErrorCode	= 0;
	
	if ( ! ( hTabCtl = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTC_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TABCTLDATA *) GlobalLock ( hTabCtl ) ) )
		ErrorCode = WTC_ERR_GLOBALLOCK;
	else
	{
		TabItem_DrawFocusRect ( hDC, pthis );
		pthis->bFocus = TRUE;
	}
	
	if ( pthis )
		GlobalUnlock ( hTabCtl );
	if ( hDC )
		ReleaseDC ( hWnd, hDC );
}


static void TabCtl_OnLButtonUp 
(	
	HWND				hWnd, 
	int				x, 
	int				y, 
	UINT				keyFlags
) 
{
	HGLOBAL				hTabCtl		= ( HGLOBAL )NULL;
	TABCTLDATA*			pthis		= ( TABCTLDATA* )NULL;

	int				ErrorCode	= 0;
	
	if ( ! ( hTabCtl = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTC_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TABCTLDATA *) GlobalLock ( hTabCtl ) ) )
		ErrorCode = WTC_ERR_GLOBALLOCK;
	else
	{
	}

	if ( pthis )
		GlobalUnlock ( hTabCtl );
}

static void 
TabCtl_OnDestroy 
(	
	HWND				hWnd 
)
{
	HGLOBAL				hTabCtl		= ( HGLOBAL )NULL;
	TABCTLDATA*			pthis		= ( TABCTLDATA* )NULL;

	int				ErrorCode	= 0;

	if ( ! ( hTabCtl = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTC_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TABCTLDATA *) GlobalLock ( hTabCtl ) ) )
		ErrorCode = WTC_ERR_GLOBALLOCK;
	else
	{
		REMOVETHISDATA ( hWnd );
		WLDestroy ( &pthis->TabItmList );
		if ( pthis->hFont )
			DeleteObject ( pthis->hFont );
		GlobalFree ( hTabCtl );
	}
}

static HFONT 
TabCtl_OnGetFont 
(
	HWND			hWnd
) 
{
	HGLOBAL				hTabCtl		= ( HGLOBAL )NULL;
	TABCTLDATA*			pthis		= ( TABCTLDATA* )NULL;
	HFONT				hFont		= ( HFONT )NULL;
	int				ErrorCode	= 0;

	if ( ! ( hTabCtl = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTC_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TABCTLDATA *) GlobalLock ( hTabCtl ) ) )
		ErrorCode = WTC_ERR_GLOBALLOCK;
	else
	{
		hFont = pthis->hFont;
	}

	if ( pthis )
		GlobalUnlock ( hTabCtl );

	return hFont;
}

static void 
TabCtl_OnMouseMove 
(
	HWND			hWnd, 
	LPARAM			lParam, 
	UINT			wParam
) 
{
	POINT				pt;
	TABITEM*			pTabItemList	= ( TABITEM* )NULL;
	TABITEM*			pTabItem	= ( TABITEM* )NULL;
	HGLOBAL				hTabCtl		= ( HGLOBAL )NULL;
	TABCTLDATA*			pthis		= ( TABCTLDATA* )NULL;
	int				i		= 0;
	int				ErrorCode	= 0;
	int				y		= HIWORD ( lParam );

	if ( ! ( hTabCtl = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTC_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TABCTLDATA *) GlobalLock ( hTabCtl ) ) )
		ErrorCode = WTC_ERR_GLOBALLOCK;
	else
	if ( WLLock ( &pthis->TabItmList, ( LPVOID )&pTabItemList ) )
		ErrorCode = WTC_ERR_LISTLOCK;
	else
	if ( pthis->hwTTip && ( y < pthis->Rect.top ) )
	{
		pt.x = LOWORD ( lParam );
		pt.y = HIWORD ( lParam );

		for ( i = 0; i < pthis->ItmCount; i++ )
		{
			pTabItem = pTabItemList + i;
			if ( PtInRect ( &pTabItem->Rect, pt ) )
			{
				MSG msg;
				msg.hwnd = hWnd;
				msg.message = WM_MOUSEMOVE;
				msg.wParam  = wParam;
				msg.lParam  = lParam;
/*				if ( pthis->hwTTip ) //&& ( y < pthis->Rect.top ) )*/
				{
				SendMessage ( pthis->hwTTip, TTM_RELAYEVENT, 0, 
				    ( LPARAM ) ( LPMSG )&msg); 
				pthis->ItmTip = i;
				}
				break;
			}
		}
	}
	else
		pthis->ItmTip = -1;

	if ( pTabItemList )
		WLUnlock ( &pthis->TabItmList, ( LPVOID )&pTabItemList );
	if ( pthis )
		GlobalUnlock ( hTabCtl );
}


static void TabCtl_OnNotify ( HWND hWnd, UINT uCtlID, LPARAM lParam ) 
{ 

	TABITEM*			pTabItemList	= ( TABITEM* )NULL;
	HGLOBAL				hTabCtl		= ( HGLOBAL )NULL;
	TABCTLDATA*			pthis		= ( TABCTLDATA* )NULL;
	int				ErrorCode	= 0;

	LPTOOLTIPTEXT			lpttt; 

	if ( ! ( hTabCtl = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTC_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TABCTLDATA *) GlobalLock ( hTabCtl ) ) )
		ErrorCode = WTC_ERR_GLOBALLOCK;
	else
	if ( WLLock ( &pthis->TabItmList, ( LPVOID )&pTabItemList ) )
		ErrorCode = WTC_ERR_LISTLOCK;
	else
	if ( ( ( ( LPNMHDR ) lParam )->code ) == TTN_NEEDTEXT ) 
	{ 
		lpttt = ( LPTOOLTIPTEXT )lParam;
		lpttt->hdr.idFrom = pthis->ItmTip;
		lpttt->uFlags &= ~TTF_IDISHWND;
		SendMessage ( GetParent ( hWnd ), WM_NOTIFY, ( WPARAM )pthis->ItmTip,
				( LPARAM )lpttt );

	} 

	if ( pTabItemList )
		WLUnlock ( &pthis->TabItmList, ( LPVOID )&pTabItemList );
	if ( pthis )
		GlobalUnlock ( hTabCtl );
} 


static void TabCtl_OnPaint 
(
	HWND				hWnd, 
	HDC				hPaintDC 
)
{
	PAINTSTRUCT			ps;
	TABITEM*			pTabItemList	= ( TABITEM* )NULL;
	TABITEM*			pTabItem	= ( TABITEM* )NULL;
	HGLOBAL				hTabCtl		= ( HGLOBAL )NULL;
	TABCTLDATA*			pthis		= ( TABCTLDATA* )NULL;
	
	int				ErrorCode	= 0;

		BeginPaint ( hWnd, &ps );
	if ( ! ( hTabCtl = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTC_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TABCTLDATA *) GlobalLock ( hTabCtl ) ) )
		ErrorCode = WTC_ERR_GLOBALLOCK;
	else
	if ( WLLock ( &pthis->TabItmList, ( LPVOID )&pTabItemList ) )
		ErrorCode = WTC_ERR_LISTLOCK;
	else
	{	
		pthis = ( TABCTLDATA *) GlobalLock ( hTabCtl );
		if ( pthis->bTabItmResized )
		{
			TabCtl_IReCalcSize ( ps.hdc, pthis );
			TabCtl_ISetItemRects ( hWnd, pthis );
			pthis->bTabItmResized = FALSE;
		}
/*		if ( pthis->uStyles & TCS_MULTILINE )
			TabCtl_ISetMultiLRects ( hWnd, pthis );*/
		if ( pthis->ItmCount > 0 )
		{
			int i;
			RECT rcTabCtl;
			GetClientRect ( hWnd, &rcTabCtl );
			pthis->hFont = SelectObject ( ps.hdc, pthis->hFont );
			SetBkMode ( ps.hdc, TRANSPARENT );
			for ( i = 0; i < pthis->ItmCount ; i++ )
			{
				pTabItem = pTabItemList + i;
				if ( i != pthis->ItmSelected &&
				     TabItem_IsVisible ( pTabItem ) 
				   )
					TabCtl_IPaintTab ( hWnd, ps.hdc, pthis, pTabItem, i );
			}

			if ( pthis->bFocus )
				TabItem_DrawFocusRect ( ps.hdc, pthis ); 
			pTabItem = pTabItemList + pthis->ItmSelected;
			if ( TabItem_IsVisible ( pTabItem ) )
			{
				TabCtl_IPaintTab ( hWnd, ps.hdc, pthis, pTabItem, pthis->ItmSelected );
				if ( pthis->bFocus )
					TabItem_DrawFocusRect ( ps.hdc, pthis ); 
			}

			pthis->hFont = SelectObject ( ps.hdc, pthis->hFont );
		}

		if ( !( pthis->uStyles & TCS_BUTTONS ) )
			TabCtl_IPaintTabCtlBorder ( ps.hdc, pthis, pTabItemList );
	}

		EndPaint ( hWnd, &ps );

	if ( pTabItemList )
		WLUnlock ( &pthis->TabItmList, ( LPVOID )&pTabItemList );
	if ( pthis )
		GlobalUnlock ( hTabCtl );
}

static void 
TabCtl_OnSetFont 
(
	HWND			hWnd,
	HFONT			newFont,
	BOOL			bRedraw
) 
{
	HGLOBAL				hTabCtl		= ( HGLOBAL )NULL;
	TABCTLDATA*			pthis		= ( TABCTLDATA* )NULL;
	int				ErrorCode	= 0;

	if ( ! ( hTabCtl = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTC_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TABCTLDATA *) GlobalLock ( hTabCtl ) ) )
		ErrorCode = WTC_ERR_GLOBALLOCK;
	else
	{
		pthis->hFont = newFont;
	}

	if ( pthis )
		GlobalUnlock ( hTabCtl );
	if ( bRedraw )
		InvalidateRect ( hWnd, NULL, TRUE );
	
}


#ifdef 0
static void TabCtl_OnTimer 
(
	HWND				hWnd, 
	UINT				uTimerID 
)
{
	HGLOBAL				hTabCtl		= ( HGLOBAL )NULL;
	TABCTLDATA*			pthis		= ( TABCTLDATA* )NULL;
	
	int				ErrorCode	= 0;

	if ( ! ( hTabCtl = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTC_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TABCTLDATA *) GlobalLock ( hTabCtl ) ) )
		ErrorCode = WTC_ERR_GLOBALLOCK;
	else
	{
	}
	if ( pthis )
		GlobalUnlock ( hTabCtl );
}
#endif


static void
TabCtl_IReCalcSize 
( 
	HDC				hDC,
	TABCTLDATA*			pthis 
)					
{
	TABITEM*			pTabItemList	= ( TABITEM* )NULL;
	TABITEM*			pTabItem	= ( TABITEM* )NULL;
	int				ErrorCode	= 0;

	if ( WLLock ( &pthis->TabItmList, ( LPVOID )&pTabItemList ) )
		ErrorCode = WTC_ERR_LISTLOCK;
	else
	{	
		int i;
		for ( i = 0; i < pthis->ItmCount; i++ )
		{
			pTabItem = pTabItemList + i;
#if 0
			if ( pthis->uStyles & TCS_FIXEDWIDTH )
			{
				pTabItem->Size.cx = pthis->ItmWidth + 2 * pthis->nXPadding;
				if ( pthis->hImageL )
					pTabItem->Size.cx += ( pthis->ImgCX + pthis->nXPadding );
			}
			else
#endif
				TabCtl_ICalcItemSize ( hDC, pthis, pTabItem, TRUE );
		}
	}

	if ( pTabItemList )
		WLUnlock ( &pthis->TabItmList, ( LPVOID )&pTabItemList );

}


static void
TabCtl_ICalcItemSize 
( 
	HDC					hDC,
	TABCTLDATA*				pthis, 
	TABITEM*				ptabi,
	BOOL					bAllowForImage
)
{
	SIZE					size;


	ptabi->Size.cy = pthis->ItmHeight;

	if ( ptabi->szLabel[0] != '\0' )
	{
		pthis->hFont = SelectObject ( hDC, pthis->hFont );
		GetTextExtentPoint32 ( hDC, ptabi->szLabel, 
			    lstrlen ( ptabi->szLabel ), &size ); 
		ptabi->Size.cx = size.cx + pthis->nXPadding * 2;
		pthis->hFont = SelectObject ( hDC, pthis->hFont );
	}

	if ( bAllowForImage && pthis->hImageL )
		ptabi->Size.cx += ( pthis->ImgCX + pthis->nXPadding );

	if ( pthis->uStyles & TCS_FIXEDWIDTH )
	{
		if ( ptabi->Size.cx > pthis->ItmWidth )
		{
			pthis->ItmWidth = ptabi->Size.cx;
			pthis->bTabItmResized = TRUE;
		}
	}
}

/*Calculate the extra width a tab item of a certain row needs to be extended so that
//all tab items of that row would fill the width of the tab control
//Row : the row inlvolved
//ItmIndx : index of the first tab item of Row
*/
static int
TabCtl_ICalcRJustdWidth 
( 
	TABCTLDATA*				pthis,
	TABITEM*				pTabItemList,
	int					Row,
	int					ItmIndx
)
{
	int					RowWidth;
	int					i		= ItmIndx;
	int					j		= 0;
	int					nTotalWidth	= 0;
	TABITEM*				pTabItem	= ( TABITEM* )NULL;

	pTabItem = pTabItemList + i;
	while ( Row == pTabItem->Row && i < pthis->ItmCount )
	{
		nTotalWidth += ( pTabItem->Size.cx + HORZ_GAP );
		j++;i++;
		pTabItem = pTabItemList + i;
	}

	RowWidth = pthis->Rect.right - pthis->Rect.left;
	if ( Row != 0 )
		RowWidth -= HORZ_GAP;

	return ( RowWidth - nTotalWidth ) / j;

}

static BOOL
TabCtl_IIsClipped 
(
	TABCTLDATA*				pthis,					
	int					iSelected
)
{
	TABITEM*			pTabItemList	= ( TABITEM* )NULL;
	TABITEM*			pTabItem	= ( TABITEM* )NULL;
	BOOL				bClipped	= FALSE;
	int				ErrorCode	= 0;

	if ( WLLock ( &pthis->TabItmList, ( LPVOID )&pTabItemList ) )
		ErrorCode = WTC_ERR_LISTLOCK;
	else
	{
		pTabItem = pTabItemList + iSelected;
		if ( pTabItem->bClipped )
			bClipped = TRUE;
	}

	if ( pTabItemList )
		WLUnlock ( &pthis->TabItmList, ( LPVOID )&pTabItemList );

	return ( ErrorCode >= 0 && bClipped );

}

static BOOL
TabCtl_IIsLastTItmOfRow 
( 
	TABCTLDATA*				pthis,
	TABITEM*				pTabItemList,
	int					Row,
	int					ItmIndx
)
{
	BOOL					bResult		= TRUE;

	ItmIndx++;
	if ( ( ItmIndx < pthis->ItmCount ) &&
	     ( ( pTabItemList + ItmIndx )->Row == Row )
	   )
		bResult = FALSE;
	return bResult;

}

static BOOL
TabItem_DrawFocusRect
(
	HDC			hDC,
	TABCTLDATA*		pthis
)
{
	RECT				rect;
	TABITEM*			pTabItemList	= ( TABITEM * )NULL;
	TABITEM*			pTabItem	= ( TABITEM * )NULL;
	int				ErrorCode	= 0;
	if ( WLLock ( &pthis->TabItmList, ( LPVOID )&pTabItemList ) )
		ErrorCode = WTC_ERR_LISTLOCK;
	else
	{
		pTabItem = pTabItemList + pthis->ItmSelected;
		CopyRect ( &rect, &pTabItem->Rect ); 
		InflateRect ( &rect, -2, -2 );
		DrawFocusRect ( hDC, &rect );
	}

	if ( pTabItemList )
		WLUnlock ( &pthis->TabItmList, ( LPVOID )&pTabItemList );
	return ( ErrorCode >= 0 );

}

static BOOL
TabItem_IsVisible
(
	TABITEM*			pTabItem
)
{
	return ( pTabItem->Rect.right > pTabItem->Rect.left );
}


static void
TabCtl_IPaintTabCtlBorder
(
	HDC				hDC,
	TABCTLDATA*			pthis,
	TABITEM*			pTabItemList
)
{
	HPEN				htmPen;
	HPEN				hWPen		= GetStockObject ( WHITE_PEN );
	HPEN				hBPen		= GetStockObject ( BLACK_PEN );
	HPEN				hShade		= CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNSHADOW ) );

	htmPen = SelectObject ( hDC, hWPen );
	MoveToEx ( hDC , pthis->Rect.left, pthis->Rect.bottom, NULL );
	LineTo ( hDC, pthis->Rect.left, pthis->Rect.top );

	if ( pthis->ItmVisible >= 0 &&
	     TabItem_IsVisible ( pTabItemList + pthis->ItmSelected ) )
	{
		TABITEM* pTabItem = pTabItemList + pthis->ItmSelected;
		LineTo ( hDC, pTabItem->Rect.left, pTabItem->Rect.bottom );
		MoveToEx ( hDC, pTabItem->Rect.right, pTabItem->Rect.bottom, NULL );
	}

	LineTo ( hDC, pthis->Rect.right, pthis->Rect.top );

	hWPen = SelectObject ( hDC, hBPen );
	LineTo ( hDC, pthis->Rect.right, pthis->Rect.bottom );
	LineTo ( hDC, pthis->Rect.left, pthis->Rect.bottom );

	hBPen = SelectObject ( hDC, htmPen );

	hShade = SelectObject ( hDC, hShade );
	MoveToEx ( hDC, pthis->Rect.right - 1, pthis->Rect.top, NULL );
	LineTo ( hDC, pthis->Rect.right-1, pthis->Rect.bottom - 1 );
	LineTo ( hDC, pthis->Rect.left, pthis->Rect.bottom - 1);
	hShade = SelectObject ( hDC, hShade );

	DeleteObject ( hShade );

}

static void
TabItem_IDrawLTBorder 
( 
	HDC				hDC, 
	UINT				uStyle, 
	RECT*				pRect,
	BOOL				bSelected 
)
{
	HPEN				hwPen	= CreatePen ( PS_SOLID, 1, RGB ( 255,255,255 ) );
	HPEN				hbPen	= CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNSHADOW ) );
	HPEN				hsdPen  = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNTEXT ) );
	HPEN				htmpPen = ( HPEN )NULL;

	/*draw tab item border*/
	/*draw tab item left and top borders*/
	if ( uStyle & TCS_BUTTONS )
	{	/*without a round corner*/
		if ( bSelected )
		{
			MoveToEx ( hDC, pRect->left + 1, pRect->bottom, NULL );
			hsdPen = SelectObject ( hDC, hsdPen );
			LineTo ( hDC, pRect->left + 1, pRect->top + 1 );
			LineTo ( hDC, pRect->right - 1, pRect->top + 1 );
			hsdPen = SelectObject ( hDC, hsdPen );
		}
		MoveToEx ( hDC, pRect->left, pRect->bottom, NULL );
		htmpPen = bSelected ? hbPen : hwPen;
		htmpPen = SelectObject ( hDC, htmpPen );
		LineTo ( hDC, pRect->left, pRect->top );
	}
	else
	{	/*with a round corner*/
		htmpPen = hwPen;
		htmpPen = SelectObject ( hDC, htmpPen );
		MoveToEx ( hDC, pRect->left, pRect->bottom, NULL );
		LineTo ( hDC, pRect->left, pRect->top + 2 );
		LineTo ( hDC, pRect->left + 2, pRect->top );
	}
	
	LineTo ( hDC, pRect->right -1, pRect->top );
	htmpPen = SelectObject ( hDC, htmpPen );

	/*cleanup*/
	DeleteObject ( hwPen );
	DeleteObject ( hbPen );
	DeleteObject ( hsdPen );

}

/*need revisit*/
static void
TabItem_IDrawRBBorder 
( 
	HDC				hDC, 
	UINT				uStyles, 
	RECT*				pRect,
	BOOL				bSelected,
	BOOL				bClipped
)
{
	HPEN				hwPen	= CreatePen ( PS_SOLID, 1, RGB ( 255,255,255 ) );
	HPEN				hbPen	= CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNSHADOW ) );
	HPEN				hsdPen  = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNTEXT ) );
	HPEN				htmpPen = ( HPEN )NULL;

	if ( bClipped )
	{
		int i;
		int x;
		int y;
		hbPen = SelectObject ( hDC, hbPen );
		for ( i = 1, y = pRect->top; y < pRect->bottom; y += 3, i++ ) 
		{
			x = ( ( i % 4 ) - 2 * ( ( i % 4 ) / 3 ) ) - 1;
			MoveToEx ( hDC, pRect->right + x, y, NULL );
			LineTo ( hDC, pRect->right + x, min ( y + 3, ( int )pRect->bottom ) );
		}
	
/*		hbPen = SelectObject ( hDC, hbPen );*/
		if ( uStyles & TCS_BUTTONS )
		{
			hsdPen = SelectObject ( hDC, hsdPen );
			MoveToEx ( hDC, pRect->right - 1, pRect->bottom, NULL );
			LineTo ( hDC, pRect->left, pRect->bottom );
			hsdPen = SelectObject ( hDC, hsdPen );
			MoveToEx ( hDC, pRect->right - 1, pRect->bottom - 1, NULL );
			LineTo ( hDC, pRect->left, pRect->bottom - 1 );
		}
		hbPen = SelectObject ( hDC, hbPen );

	}
	else
	if ( uStyles & TCS_BUTTONS && bSelected )
	{
		hwPen = SelectObject ( hDC, hwPen );
		MoveToEx ( hDC, pRect->right - 1, pRect->top, NULL );
		LineTo ( hDC, pRect->right - 1, pRect->bottom );
		LineTo ( hDC, pRect->left, pRect->bottom );
		hwPen = SelectObject ( hDC, hwPen );
	}
	else
	{
		htmpPen = SelectObject ( hDC, hbPen );
		MoveToEx ( hDC, pRect->right - 1, pRect->top, NULL );
		LineTo ( hDC, pRect->right - 1, pRect->bottom - 1 );
		if ( uStyles & TCS_BUTTONS )
			LineTo ( hDC, pRect->left, pRect->bottom - 1 );
		hbPen = SelectObject ( hDC, hsdPen );
		MoveToEx ( hDC, pRect->right, pRect->top + 1, NULL );
		LineTo ( hDC, pRect->right, pRect->bottom);
		if ( uStyles & TCS_BUTTONS )
			LineTo ( hDC, pRect->left, pRect->bottom );
		hsdPen = SelectObject ( hDC, htmpPen );
	}

	/*cleanup*/
	DeleteObject ( hwPen );
	DeleteObject ( hbPen );
	DeleteObject ( hsdPen );

}


static void
TabCtl_IPaintTab 
( 
	HWND				hWnd,
	HDC				hDC, 
	TABCTLDATA*			pthis, 
	TABITEM*			pTabItem,
	int				nItem 
)
{
	RECT				rcLabel;
	RECT				rcIcon;
	HPEN				hbPen	= CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNSHADOW ) );
	HPEN				htxtPen = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNTEXT ) );
	RECT*				pRect   = &pTabItem->Rect;

	SetRect ( &rcLabel, pRect->left, pRect->top, pRect->right, pRect->bottom );
	FillRect ( hDC, &rcLabel, GetStockObject ( LTGRAY_BRUSH ) );

	/*draw content of a tab item*/
	if ( pthis->uStyles & TCS_OWNERDRAWFIXED )
	{
		DRAWITEMSTRUCT di;  
		memset ( &di, 0, sizeof ( DRAWITEMSTRUCT ) );
		di.CtlType = ODT_TAB; 
		di.CtlID = GetDlgCtrlID ( hWnd ); 
		di.itemID = nItem; 
		di.hDC = hDC; 
		CopyRect ( &di.rcItem, pRect ); 
		SendMessage ( GetParent ( hWnd ), WM_DRAWITEM, di.CtlID, ( LPARAM )&di );
	}
	else
	{
		TabCtl_IGetTabItmContentRect ( hDC, pthis, pTabItem, &rcIcon, &rcLabel );
#if 1
		if ( pthis->hImageL && pTabItem->iImage != -1 && !IsRectEmpty ( &rcIcon ) )
			ImageList_DrawEx ( ( HIMAGELIST )pthis->hImageL, pTabItem->iImage, hDC,
				rcIcon.left, rcIcon.top, 
				rcIcon.right - rcIcon.left, 
				rcIcon.bottom - rcIcon.top,
				CLR_NONE, CLR_NONE, ILD_NORMAL | ILD_TRANSPARENT );
#else
		if ( pthis->hImageL && pTabItem->iImage != -1 && !IsRectEmpty ( &rcIcon ) )
			ImageList_DrawEx ( pthis->hImageL, pTabItem->iImage, hDC,
				rcIcon.left, rcIcon.top, 
				10, 10, CLR_NONE, CLR_NONE, ILD_NORMAL );
#endif
		DrawText ( hDC, pTabItem->szLabel, -1, &rcLabel, DT_LEFT | DT_SINGLELINE | DT_VCENTER );
	}
	/*draw tab item left and top borders*/
	TabItem_IDrawLTBorder ( hDC, pthis->uStyles, pRect, nItem == pthis->ItmSelected );
	/*draw right and bottom border*/
	TabItem_IDrawRBBorder ( hDC, pthis->uStyles, pRect, 
				nItem == pthis->ItmSelected, pTabItem->bClipped );

	/*cleanup*/
	DeleteObject ( hbPen );
	DeleteObject ( htxtPen );
}

static void
TabCtl_IGetTabItmContentRect 
( 
	HDC				hDC,
	TABCTLDATA*			pthis,
	TABITEM*			pTabItm,
	RECT*				prcIcon,
	RECT*				prcText
)
{
	int				iconLeft = 0;

	CopyRect ( prcIcon, &pTabItm->Rect );
	InflateRect ( prcIcon, 0, - pthis->nYPadding ); 
	CopyRect ( prcText, prcIcon );

	if ( pthis->uStyles & TCS_FIXEDWIDTH ) 
	{
		iconLeft = pTabItm->Rect.left + pthis->nXPadding + 
			( pthis->ItmWidth - pTabItm->Size.cx ) / 2;
		
		if ( pthis->uStyles & TCS_FORCEICONLEFT ||
		     pthis->uStyles & TCS_FORCELABELLEFT
		   )
			SetRect ( prcIcon, pTabItm->Rect.left + pthis->nXPadding, 
				   pTabItm->Rect.top + pthis->nYPadding, 
				   pTabItm->Rect.left + pthis->nXPadding + pthis->ImgCX,
				   pTabItm->Rect.top + pthis->nYPadding + pthis->ImgCY );
		else
			SetRect ( prcIcon, iconLeft, pTabItm->Rect.top + pthis->nYPadding, 
				iconLeft + pthis->ImgCX,
				pTabItm->Rect.top + pthis->nYPadding + pthis->ImgCY );
		if ( prcIcon->left > pTabItm->Rect.right )
			SetRectEmpty ( prcIcon );
		if ( prcIcon->right > pTabItm->Rect.right )
			prcIcon->right = pTabItm->Rect.right;

		prcText->left = ( pthis->uStyles & TCS_FORCELABELLEFT ) ?
				prcIcon->left :	iconLeft;
		if ( pthis->ImgCX > 0 )
			prcText->left += ( pthis->ImgCX + pthis->nXPadding );

	}
	else
	{

		iconLeft = ( pTabItm->Rect.right - pTabItm->Rect.left - pTabItm->Size.cx ) / 2;
		if ( iconLeft <= 0 )
			iconLeft = pTabItm->Rect.left + pthis->nXPadding;
		else
			iconLeft += pTabItm->Rect.left + pthis->nXPadding;
		SetRect ( prcIcon, iconLeft, pTabItm->Rect.top + pthis->nYPadding, 
				iconLeft + pthis->ImgCX,
				pTabItm->Rect.top + pthis->nYPadding + pthis->ImgCY );
		if ( prcIcon->left > pTabItm->Rect.right )
			SetRectEmpty ( prcIcon );
		if ( prcIcon->right > pTabItm->Rect.right )
			prcIcon->right = pTabItm->Rect.right;
		prcText->left = iconLeft;
		if ( pthis->ImgCX > 0 )
			prcText->left += ( pthis->ImgCX + pthis->nXPadding );
	}
}

static int
TabCtl_SetCurSel 
( 
	HWND				hWnd, 
	TABCTLDATA*			pthis,
	int				nSel
)
{
	RECT				rect;

	if ( nSel >= pthis->ItmCount || nSel < 0 )
		return -1;
	pthis->ItmSelected = nSel;
	TabCtl_IResetItmVisible ( pthis, nSel );
	TabCtl_ISetItemRects ( hWnd, pthis );

	GetClientRect ( hWnd, &rect );
	rect.bottom = pthis->Rect.top + VERT_MARGIN;
	InvalidateRect ( hWnd, &rect, TRUE );  /*Revisit this later*/

	return 0;
}

static BOOL
TabCtl_SetItem 
( 
	TABCTLDATA*			pthis,
	int				ItmIndx,
	TC_ITEM*			ptci
)
{
	TABITEM*			pTabItemList	= ( TABITEM* )NULL;
	TABITEM*			pTabItem	= ( TABITEM* )NULL;
	BOOL				Result		= FALSE;
	int				ErrorCode	= 0;

	if ( !ptci || ItmIndx < 0 || ItmIndx >= pthis->ItmCount )
		return FALSE;

	if ( WLLock ( &pthis->TabItmList, ( LPVOID )&pTabItemList ) )
		ErrorCode = WTC_ERR_LISTLOCK;
	else
	{
		pTabItem = pTabItemList + ItmIndx;
		if ( ptci->mask & TCIF_TEXT )
		{
			ptci->mask |= TCIF_TEXT;
			pTabItem->pszText = ptci->pszText;
		}
		if ( ptci->mask & TCIF_IMAGE )
		{
			ptci->mask |= TCIF_IMAGE;
			pTabItem->iImage = ptci->iImage;
		}
		if ( ptci->mask & TCIF_PARAM )
		{
			ptci->mask |= TCIF_PARAM;
			pTabItem->lParam = ptci->lParam;
		}
		if ( pthis->cbExtra )
		{
			void* psExtra = &ptci->lParam;
			void* pdExtra = &pTabItem->lParam;
			memcpy ( pdExtra, psExtra, pthis->cbExtra );
		}

		Result = TRUE;

	}

    if ( pTabItemList )
        WLUnlock ( &pthis->TabItmList, ( LPVOID )&pTabItemList );

	return Result;
}

static BOOL
TabCtl_SetItemExtra
( 
	TABCTLDATA*			pthis,
	int				cbExtra
)
{
	BOOL				Result		= FALSE;

	if ( cbExtra > 0 && pthis->ItmCount == 0 )
	{
		if ( WLDestroy ( &pthis->TabItmList ) == 0 )
		{
			WLCreate ( &pthis->TabItmList, sizeof ( TABITEM ) + cbExtra );	
			pthis->cbExtra = cbExtra;
			Result = TRUE;
		}

	}

	return Result;
}


static LONG
TabCtl_SetItemSize
( 
	TABCTLDATA*			pthis,
	int				nWidth,
	int				nHeight
)
{
	int				oWidth		= 0;
	int				oHeight		= 0;

	oWidth = pthis->ItmWidth;
	oHeight = pthis->ItmHeight;
	pthis->ItmWidth = nWidth;
	pthis->ItmHeight = nHeight;
	return MAKELONG ( oWidth, oHeight );
}


static void
TabCtl_SetTooltip 
( 
	HWND				hWnd, 
	HWND				hwTTip 
)
{
	
	TOOLINFO			ti;

	if ( hwTTip )
	{
		memset ( &ti, 0, sizeof ( TOOLINFO ) );
		ti.cbSize = sizeof(TOOLINFO); 
/*TTF_IDISHWND needed because ti.uId is supposed to be the id of a child
  control of ti.hwnd and in this case ti.hwnd and ti.uId are both the same window*/
		ti.uFlags = TTF_IDISHWND; 
		ti.hwnd = hWnd; 
		ti.uId = ( UINT )hWnd; 
		ti.lpszText = LPSTR_TEXTCALLBACK; 
		SendMessage ( hwTTip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti ); 
		SendMessage ( hwTTip, TTM_ACTIVATE, ( WPARAM )TRUE, 0L );
	}

}

/*reset the leftmost visible tab of the tab control such that the selected tab
  would be fully visible ( that means it's not clipped or invisible*/
static void
TabCtl_IResetItmVisible
(
	TABCTLDATA*			pthis,
	int				iSelected
)
{
	int				i;
	int				left		= 0;
	TABITEM*			pTabItemList	= ( TABITEM* )NULL;
	TABITEM*			pTabItm		= ( TABITEM* )NULL;
	BOOL				bDone		= TRUE;
	
	if ( WLLock ( &pthis->TabItmList, ( LPVOID )&pTabItemList ) )
		return;
	if ( pthis->ItmVisible > iSelected )
		pthis->ItmVisible = iSelected;
	else
	do
	{
		bDone = TRUE;
		left = pthis->Rect.left;
		for ( i = pthis->ItmVisible; i <= iSelected; i++ )
		{
			pTabItm = pTabItemList + i;
			left += ( ( ( pthis->uStyles & TCS_FIXEDWIDTH ) ?
				  pthis->ItmWidth : pTabItm->Size.cx ) + HORZ_GAP );
			if ( left > pthis->Rect.right ) 
			{
				if ( pthis->ItmVisible < iSelected )
					pthis->ItmVisible++;
				bDone = FALSE;
				break;
			}
		}

	}
	while ( !bDone );

	if ( pTabItemList )
        WLUnlock ( &pthis->TabItmList, ( LPVOID )&pTabItemList );

}


static void
TabCtl_ISelChange 
( 
	HWND				hwTabCtl,
	TABCTLDATA*			pthis, 
	int				iSelected
)
{
	
	NMHDR				Header;
	RECT				rect;

	Header.hwndFrom = hwTabCtl;
	Header.idFrom = GetDlgCtrlID ( hwTabCtl );
	Header.code = TCN_SELCHANGING;

	if ( SendMessage ( GetParent ( hwTabCtl ), WM_NOTIFY, 
		( WPARAM )GetDlgCtrlID ( hwTabCtl ), ( LPARAM )&Header ) )
		return;

	pthis->ItmSelected = iSelected;
	if ( TabCtl_IIsClipped ( pthis, iSelected ) ||
	     pthis->ItmVisible > pthis->ItmSelected 
	   )
	{
		TabCtl_IResetItmVisible ( pthis, iSelected );
	}
	TabCtl_ISetItemRects ( hwTabCtl, pthis );
	GetClientRect ( hwTabCtl, &rect );
	rect.bottom = pthis->Rect.top + VERT_MARGIN;
	InvalidateRect ( hwTabCtl, &rect, TRUE );  /*Revisit this later*/
	if ( pthis->hwSpin )
		ValidateRect ( pthis->hwSpin, ( RECT* )NULL ); 
	

	Header.code = TCN_SELCHANGE;
	SendMessage ( GetParent ( hwTabCtl ), WM_NOTIFY, 
		( WPARAM )GetDlgCtrlID ( hwTabCtl ), ( LPARAM )&Header );

}


static void
TabCtl_ISetRows 
( 
	HWND				hwTabCtl,
	TABCTLDATA*			pthis
)
{
	int				i;
	int				left;
	RECT				rcTab;
	int				ItemsOfRow	= 0;
	int				Row		= 0;
	int				Count		= 0;
	TABITEM*			pTabItemList	= ( TABITEM* )NULL;
	TABITEM*			pTabItm		= ( TABITEM* )NULL;

	Count = pthis->ItmCount;
	if ( WLLock ( &pthis->TabItmList, ( LPVOID )&pTabItemList ) )
		return;
	
	GetClientRect ( hwTabCtl, &pthis->Rect );
	pthis->Rect.right--;
	pthis->Rect.bottom--;
	CopyRect ( &rcTab, &pthis->Rect );
	
	/*assign row*/
	left = rcTab.left;
	for ( i = 0 ; i < Count; i++ )
	{
		ItemsOfRow++;
		pTabItm = pTabItemList + i;
		pTabItm->Row = Row;
		left += ( pTabItm->Size.cx + HORZ_GAP );

		if ( left > rcTab.right || i + 1 >= Count )
		{
			left = rcTab.left;
			if ( ItemsOfRow > 1 )
			{
				pTabItm->Row = Row + 1;
				ItemsOfRow = 1;
				left += ( pTabItm->Size.cx + HORZ_GAP );
			}
			else /*the tab item is wider than the tab control width*/
				ItemsOfRow = 0;

			Row++;
			/*left = rcTab.left;
			  ItemsOfRow = ItemsOfRow > 1 ? 1 : 0;*/
		}

	}

	pthis->Rows = Row + 1;
	/*pthis->Rect.top += ( Row + 1 ) * pthis->ItmHeight;*/

	if ( pTabItemList )
		WLUnlock ( &pthis->TabItmList, ( LPVOID )&pTabItemList );
}


static void
TabCtl_ISetMultiLRects 
( 
	HWND				hwTabCtl,
	TABCTLDATA*			pthis
)
{
	int				i;
	int				left;
	int				curRow;
	int				nRowSelected;
	RECT				rcTab;
	int				Row		= 0;
	int				Count		= 0;
	int				nExtdWidth	= 0;
	TABITEM*			pTabItemList	= ( TABITEM* )NULL;
	TABITEM*			pTabItm		= ( TABITEM* )NULL;

	Count = pthis->ItmCount;
	if ( WLLock ( &pthis->TabItmList, ( LPVOID )&pTabItemList ) )
		return;
	
	GetClientRect ( hwTabCtl, &pthis->Rect );
	pthis->Rect.right--;
	pthis->Rect.bottom--;
	CopyRect ( &rcTab, &pthis->Rect );

	pthis->Rect.top += pthis->Rows * pthis->ItmHeight;
	
	/*advance the row with selected tab item to be the first row*/
	if ( !( pthis->uStyles & TCS_BUTTONS ) )
	{
		nRowSelected = ( pTabItemList + pthis->ItmSelected )->Row;
		curRow = -1;
		Row = 0;
		for ( i = 0 ; i < Count && nRowSelected != 0 ; i++ )
		{
			pTabItm = pTabItemList + i;
			if ( pTabItm->Row == nRowSelected )
				pTabItm->Row = 0;
			else
			{
				if ( curRow != pTabItm->Row )
				{
					curRow = pTabItm->Row;
					Row++;
				}
				pTabItm->Row = Row;
			}

		}
	}
	
	/*calculate bounding rectangle of each tab item*/
	Row = -1;
	for ( i = 0 ; i < Count; i++ )
	{
		pTabItm = pTabItemList + i;
		if ( Row != pTabItm->Row )
		{
			Row = pTabItm->Row;
			left = rcTab.left;
			if ( i != pthis->ItmSelected )
				left += HORZ_GAP;
			if ( !( pthis->uStyles & TCS_RAGGEDRIGHT ) ) /*== TCS_RIGHTJUSTIFY*/
				nExtdWidth = TabCtl_ICalcRJustdWidth ( pthis, pTabItemList, Row, i );
		}
		pTabItm->bClipped = FALSE;
		/*set bouding rectangle for each visible tab item*/
		pTabItm->Rect.left = left;
		pTabItm->Rect.top = pthis->Rect.top - ( pTabItm->Row + 1 ) * pthis->ItmHeight;
		pTabItm->Rect.right = left + pTabItm->Size.cx + nExtdWidth;
		pTabItm->Rect.bottom = pTabItm->Rect.top + pTabItm->Size.cy;
		if ( i == pthis->ItmSelected && !( pthis->uStyles & TCS_BUTTONS ) )
		{
			pTabItm->Rect.top -= HORZ_GAP;
			pTabItm->Rect.right += HORZ_GAP;
		}
		if ( !( pthis->uStyles & TCS_RAGGEDRIGHT ) && 
		     TabCtl_IIsLastTItmOfRow ( pthis, pTabItemList, Row, i ) &&
		     i == pthis->ItmSelected 
		   )
			pTabItm->Rect.right = pthis->Rect.right;
		
#if 0		
		/*set bouding rectangle for icon in each visible tab item*/
		if ( ( pthis->uStyles & TCS_RAGGEDRIGHT ) || 
		     ( ( pthis->uStyles & TCS_FIXEDWIDTH ) &&
		       ( pthis->uStyles & TCS_FORCELABELLEFT ) 
		     )
		   )
			SetRect ( &pTabItm->rcIcon, left + pthis->nXPadding, 
				pTabItm->Rect.top + pthis->nYPadding, 
				left + pthis->nXPadding + pthis->ImgCX,
				pTabItm->Rect.top + pthis->nYPadding + pthis->ImgCY );
		else
		{
			int offset = ( pTabItm->Rect.right - pTabItm->Rect.left -
				       pTabItm->Size.cx ) / 2 - pthis->nXPadding;				     
			SetRect ( &pTabItm->rcIcon, left + pthis->nXPadding + offset, 
				pTabItm->Rect.top + pthis->nYPadding, 
				left + pthis->nXPadding + pthis->ImgCX + offset,
				pTabItm->Rect.top + pthis->nYPadding + pthis->ImgCY );
		}
#endif
		left += ( pTabItm->Size.cx + nExtdWidth + HORZ_GAP );

	}

	if ( pTabItemList )
		WLUnlock ( &pthis->TabItmList, ( LPVOID )&pTabItemList );
}

static void
TabCtl_ISetSingleLRects 
( 
	HWND				hwTabCtl,
	TABCTLDATA*			pthis
)
{
	int				i;
	int				left;
	RECT				rcTab;
	int				Count		= 0;
	int				nitWidth	= 0;
	BOOL				bNeedScroller	= FALSE;
	TABITEM*			pTabItemList	= ( TABITEM* )NULL;
	TABITEM*			pTabItm		= ( TABITEM* )NULL;

	pthis->Rows = 1;
	Count = pthis->ItmCount;
	if ( WLLock ( &pthis->TabItmList, ( LPVOID )&pTabItemList ) )
		return;
	
	for ( i = 0; i < pthis->ItmVisible; i++ )
	{
		pTabItm = pTabItemList + i;
		SetRectEmpty ( &pTabItm->Rect );
	}

	GetClientRect ( hwTabCtl, &pthis->Rect );
	InflateRect ( &pthis->Rect, -1, -1 );
	CopyRect ( &rcTab, &pthis->Rect );
	if ( Count > 0 )
	{
		pthis->Rect.top += pthis->ItmHeight + HORZ_GAP;
		rcTab.top += HORZ_GAP;
	}
	bNeedScroller = pthis->ItmVisible ? TRUE : FALSE;
	
	left = rcTab.left + HORZ_GAP;
	for ( i = pthis->ItmVisible; i < Count; i++ )
	{
		pTabItm = pTabItemList + i;
		nitWidth = ( pthis->uStyles & TCS_FIXEDWIDTH ) ? pthis->ItmWidth : 
							         pTabItm->Size.cx;
		SetRect ( &pTabItm->Rect, left, rcTab.top, left + nitWidth,
				                           rcTab.top + pTabItm->Size.cy );
		if ( i == pthis->ItmSelected && !( pthis->uStyles & TCS_BUTTONS ) )
		{
			pTabItm->Rect.left -= HORZ_GAP;
			pTabItm->Rect.top -= HORZ_GAP;
			pTabItm->Rect.right += HORZ_GAP;
		}					
		pTabItm->bClipped = FALSE;
		if ( pTabItm->Rect.right > ( rcTab.right - ( bNeedScroller? SCROLLER_WIDTH + 3 : 0 ) ) )
		{
			pTabItm->Rect.right = rcTab.right - SCROLLER_WIDTH - 3 ; 
			pTabItm->bClipped = TRUE;
			bNeedScroller = TRUE;
			break;
		}
		left += ( nitWidth + HORZ_GAP );
	}

	while ( ++i < Count )
	{
		pTabItm = pTabItemList + i;
		SetRectEmpty ( &pTabItm->Rect );
	}

	if ( bNeedScroller )
		SetWindowPos ( pthis->hwSpin, HWND_TOP, pthis->Rect.right - SCROLLER_WIDTH,
		            pthis->Rect.top - SCROLLER_HEIGHT-2, SCROLLER_WIDTH, SCROLLER_HEIGHT,
			    SWP_NOACTIVATE | SWP_SHOWWINDOW );


	if ( pTabItemList )
		WLUnlock ( &pthis->TabItmList, ( LPVOID )&pTabItemList );
}

static void
TabCtl_ISetItemRects 
( 
	HWND				hwTabCtl,
	TABCTLDATA*			pthis
)
{

	if ( pthis->uStyles & TCS_MULTILINE )
	{
		TabCtl_ISetRows ( hwTabCtl, pthis );
		TabCtl_ISetMultiLRects ( hwTabCtl, pthis );
	}
	else
		TabCtl_ISetSingleLRects ( hwTabCtl, pthis );
}

static int
TabCtl_ITabFromPt 
( 
	HWND				hwTabCtl,
	TABCTLDATA*			pthis, 
	int				x,
	int				y,
	UINT*				pHitFlag
)
{
	POINT				pt;
	int				i;
	UINT				uHitFlag;
	TABITEM*			pTabItemList	= ( TABITEM* )NULL;
	TABITEM*			pTabItem	= ( TABITEM* )NULL;
	UINT				Count		= 0;
	int				ErrorCode	= 0;
	
	/* PAY ATTENTION TO THIS FOLLOWING CONDITION */
	if ( pthis->ItmCount && y >= pthis->Rect.top )
		return -1;

	if ( WLLock ( &pthis->TabItmList, ( LPVOID )&pTabItemList ) )
		ErrorCode = WTC_ERR_LISTLOCK;
	else
	if ( WLCount ( &pthis->TabItmList, &Count ) )
		ErrorCode = WTC_ERR_LISTCOUNT;
	else
	{
		pt.x = x;
		pt.y = y;
		uHitFlag = TCHT_NOWHERE;
		for ( i = pthis->ItmVisible; i < Count; i++ )
		{
			pTabItem = pTabItemList + i;
			if ( PtInRect (	&pTabItem->Rect, pt ) )
			{
				if ( pthis->uStyles & TCS_OWNERDRAWFIXED )
					uHitFlag = TCHT_ONITEM;
				else
				if ( pthis->hImageL &&
				     x <= ( pTabItem->Rect.left + pthis->ImgCX + pthis->nXPadding )
				   )
					uHitFlag = TCHT_ONITEMICON;
				else
					uHitFlag = TCHT_ONITEMLABEL;
				break;
			}
		}
	}

	if ( pHitFlag )
		*pHitFlag = uHitFlag;
	if ( pTabItemList )
		WLUnlock ( &pthis->TabItmList, ( LPVOID )&pTabItemList );

	return ( ( ErrorCode >= 0 && i < Count ) ? i : -1 );

}


static LRESULT WINAPI 
TabCtl_WndProc
(
	HWND				hWnd,
        UINT				uMessage,
        WPARAM				wParam,
        LPARAM				lParam
)
{

	switch (uMessage)
	{
		case WM_CREATE:
			if ( ! TabCtl_OnCreate ( hWnd, ( LPCREATESTRUCT )lParam ) )
				return ( LRESULT ) -1L;
			break;
		case WM_DESTROY:
			TabCtl_OnDestroy ( hWnd );
			break;
		case WM_GETDLGCODE:
			return ( LRESULT )DLGC_WANTARROWS | DLGC_WANTCHARS;
		case WM_GETFONT:
			return ( LRESULT )TabCtl_OnGetFont ( hWnd );
		case WM_HSCROLL:
			return ( LRESULT )TabCtl_OnHScroll ( hWnd, LOWORD ( wParam ),
				HIWORD ( lParam ), ( HWND )lParam );
		case WM_KEYDOWN:
			TabCtl_OnKeyDown ( hWnd, (UINT)( wParam ), ( int )( short )LOWORD ( lParam ), 
				(UINT)HIWORD ( lParam ) );
			break;
		
		case WM_KEYUP:
			TabCtl_OnKeyUp ( hWnd, (UINT)( wParam ), ( int )( short )LOWORD ( lParam ), 
				(UINT)HIWORD ( lParam ) );
			break;

		case WM_KILLFOCUS:
			TabCtl_OnKillFocus( hWnd );
			break;
		case WM_LBUTTONDOWN:
			TabCtl_OnLButtonDown ( hWnd,(int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )( wParam ) );
			break;

		case WM_LBUTTONUP:
			TabCtl_OnLButtonUp ( hWnd,(int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )( wParam ) );
			break;

		case WM_MOUSEMOVE:
			TabCtl_OnMouseMove ( hWnd, lParam, ( UINT )wParam );
			break;

		case WM_NOTIFY:
			TabCtl_OnNotify ( hWnd, ( UINT )wParam, lParam );
			break;

		case WM_PAINT:
			if ( IsWindowVisible ( hWnd ) )
				TabCtl_OnPaint( hWnd, ( HDC ) wParam  );
			break;

		case WM_SETFOCUS:
			TabCtl_OnSetFocus( hWnd );
			break;

		case WM_SETFONT:
			TabCtl_OnSetFont( hWnd, ( HFONT )wParam, ( BOOL ) LOWORD ( lParam ) );
			break;

		case TCM_ADJUSTRECT:
		case TCM_DELETEALLITEMS:
		case TCM_DELETEITEM:
		case TCM_GETCURFOCUS:
		case TCM_GETCURSEL:
		case TCM_GETIMAGELIST:
		case TCM_GETITEM:
		case TCM_GETITEMCOUNT:
		case TCM_GETITEMRECT:
		case TCM_GETROWCOUNT:
		case TCM_GETTOOLTIPS:
		case TCM_HITTEST:
		case TCM_INSERTITEM:
		case TCM_REMOVEIMAGE:
		case TCM_SETCURSEL:
		case TCM_SETIMAGELIST:
		case TCM_SETITEM:
		case TCM_SETITEMEXTRA:
		case TCM_SETITEMSIZE:
		case TCM_SETPADDING:
		case TCM_SETTOOLTIPS:	
			return TabCtl_TCMWndProc(hWnd, uMessage, wParam, lParam);
		default:
			return DefWindowProc(hWnd, uMessage, wParam, lParam);
	}
    
	    return 0;
}

static LRESULT  
TabCtl_TCMWndProc
(
	HWND				hWnd,
        UINT				uMessage,
        WPARAM				wParam,
        LPARAM				lParam
)
{
	HGLOBAL				hTabCtl		= ( HGLOBAL )NULL;
	TABCTLDATA*			pthis		= ( TABCTLDATA* )NULL;
	LRESULT				lResult		= 0L;
	
	int				ErrorCode	= 0;

	if ( ! ( hTabCtl = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTC_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TABCTLDATA *) GlobalLock ( hTabCtl ) ) )
		ErrorCode = WTC_ERR_GLOBALLOCK;
	else
	{
		switch ( uMessage )
		{
			case TCM_ADJUSTRECT:
				TabCtl_AdjustRect ( hWnd, pthis, 
					( BOOL )wParam, ( RECT* )lParam );
				break;
			case TCM_DELETEALLITEMS:
				lResult = TabCtl_DeleteItems ( hWnd, pthis, -1 );
				break;
			case TCM_DELETEITEM:
				lResult = TabCtl_DeleteItems ( hWnd, pthis, ( int )wParam );
				break;
			case TCM_GETCURFOCUS:
			case TCM_GETCURSEL:
				lResult = ( LRESULT )pthis->ItmSelected;
				break;
			case TCM_GETIMAGELIST:
				lResult = ( LRESULT )pthis->hImageL;
				break;
			case TCM_GETITEM:
				lResult = TabCtl_GetItem ( hWnd, pthis, ( int )wParam, ( TC_ITEM* )lParam );
				break;
			case TCM_GETITEMCOUNT:
				lResult = ( LRESULT )pthis->ItmCount;
				break;
			case TCM_GETITEMRECT:
				lResult = TabCtl_GetItemRect ( hWnd, pthis, ( int )wParam, ( RECT* )lParam );
				break;
			case TCM_GETROWCOUNT:
				lResult = ( LRESULT )pthis->Rows;
				break;
			case TCM_GETTOOLTIPS:
				lResult = ( LRESULT )pthis->hwTTip;
				break;
			case TCM_HITTEST:
				lResult = ( LRESULT )TabCtl_HitTest ( hWnd, pthis, ( TC_HITTESTINFO* )lParam );
				break;
			case TCM_INSERTITEM:
				lResult = ( LRESULT )TabCtl_InsertItem ( hWnd, pthis, ( int )wParam, ( TC_ITEM* )lParam );
				break;
			case TCM_REMOVEIMAGE:
				ImageList_Remove ( ( HIMAGELIST )pthis->hImageL, ( int )wParam );
				break;
			case TCM_SETCURSEL:
				lResult = ( LRESULT )TabCtl_SetCurSel ( hWnd, pthis, ( int )wParam );
				break;
			case TCM_SETIMAGELIST:
				pthis->hImageL = lParam;
				pthis->bTabItmResized = TRUE;
				ImageList_GetIconSize ( ( HIMAGELIST )pthis->hImageL, &pthis->ImgCX, &pthis->ImgCY );
				break;
			case TCM_SETITEM:
				lResult = ( LRESULT )TabCtl_SetItem ( pthis, ( int )wParam, ( TC_ITEM* )lParam );
				break;
			case TCM_SETITEMEXTRA:
				lResult = ( LRESULT )TabCtl_SetItemExtra ( pthis, ( int )wParam );
				break;
			case TCM_SETITEMSIZE:
				lResult = ( LRESULT )TabCtl_SetItemSize ( pthis, 
							LOWORD ( lParam ), HIWORD ( lParam ) );
				break;
			case TCM_SETPADDING:
				pthis->nXPadding = LOWORD ( lParam );
				pthis->nYPadding = HIWORD ( lParam );
				break;
			case TCM_SETTOOLTIPS:	
				pthis->hwTTip = ( HWND )wParam;
				TabCtl_SetTooltip ( hWnd, pthis->hwTTip );
				break;
		}
	}

	if ( pthis )
		GlobalUnlock ( hTabCtl );
	return lResult;
}


int WINAPI 
TabCtl_Initialize 
(	
	HINSTANCE		hinst
)
{

	WNDCLASS		wc;
	int			ErrorCode	= 0;

	memset ( &wc, 0, sizeof ( WNDCLASS ) );
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
	wc.lpfnWndProc   = TabCtl_WndProc;
	wc.cbWndExtra    = 0; 
	wc.hInstance     = hinst;
	wc.hCursor       = LoadCursor( ( HINSTANCE )NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject ( LTGRAY_BRUSH );
	wc.lpszClassName = WWC_TABCONTROL;

	if ( ! RegisterClass ( &wc ) )
		ErrorCode = WTC_ERR_REGISTERCLASS;

	return ( ErrorCode );
}


void WINAPI 
TabCtl_Terminate 
(
	HINSTANCE			hInstance
) 
{
	WNDCLASS			WndClass;


	if ( GetClassInfo ( hInstance, WWC_TABCONTROL, &WndClass ) )
		UnregisterClass ( WWC_TABCONTROL, hInstance );

}


