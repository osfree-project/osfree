/*  
	WStatusb.c	WStatusBar
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
#include "WStatusb.h"
#include "WLists.h"
/*#include "WImgList.h"*/
#include "WResourc.h"

#define	 MAX_LABELLENGTH	20
#define  VERT_PADDING		2
#define	 HORZ_PADDING		2

static char STATUSBAR_PROP[] = "Statusbar_Data";	
enum { eNull, eResizeReady, eResizing  };

typedef struct
{
     	 HMENU hMenu ;
	 UINT uiString ;
} STBMENUSTR ;


typedef struct 
{
	RECT		Rect;  
    	HANDLE      	hText;
    	UINT        	uType;
    	long        	lText; /* 32 bit info when uType is SBT_OWNERDRAW */
} STBITEM;

typedef struct 
{
	LONG		uStyles;
	POINT		Pos;
	SIZE		Size;

    	int	        nXPadding;
	int	        nYPadding;
    	int         	nXGap;

    	RECT        	rcSizeGrip;
    	RECT        	rcOutline;  /* used when dragging to resize parent window */
    	HCURSOR     	hCursor;
    	HCURSOR     	hDefCursor;
    	POINT       	ptLBDown;

	HFONT		hFont;
	int	        ItmCount;
	WLIST		ItmList; /* array of Status bar panes */
	int	    	enDrag;	 /* for dragging */
    	STBITEM    	Simple;
    	BOOL       	bSimple;


} STATUSBARDATA;

#define GETTHISDATA(hwnd) \
   GetProp(hwnd, STATUSBAR_PROP )
#define SETTHISDATA(hwnd, pthisNew) \
   SetProp(hwnd, STATUSBAR_PROP, pthisNew)
#define REMOVETHISDATA(hwnd) \
   RemoveProp(hwnd, STATUSBAR_PROP )


/*prototypes*/

static void
Statbr_IDrawBoxOutline
(
    HWND                hWnd,
    LPRECT              prcBox
);


static void
Statbr_IDrawPart 
( 
	HWND				hWnd,
	HDC			    	hDC, 
	STATUSBARDATA*			pthis, 
	STBITEM*			pItem
);

static void
Statbr_IDrawPartBorder 
( 
	HDC				hDC, 
	UINT				uType, 
	RECT*				pRect
);
  
static BOOL
Statbr_ISetPartText 
( 
	STBITEM*			pItem, 
	LPCSTR				lpText	
);

static LRESULT  
Statbr_STBWndProc
(
	HWND				hWnd,
    UINT				uMessage,
    WPARAM				wParam,
    LPARAM				lParam
);

/*Win32 API */
HWND 
WCreateStatusWindow
(
	LONG				ws,
    	LPCTSTR               		lpText,
	HWND				hWnd,
	UINT				wID
)
{
	HWND				hwStatusbar;
	HGLOBAL				hStatusbar	= ( HGLOBAL )NULL;
	STATUSBARDATA*			pthis		= ( STATUSBARDATA* )NULL;

	int			    	ErrorCode	= 0;


	hwStatusbar = CreateWindow ( WSTATUSBARCLASSNAME, "WLStatusbar", ws | WS_CHILD, 
			0, 0, 0, 0, hWnd, ( HMENU )wID,
			( HINSTANCE )GetWindowLong ( hWnd, GWL_HINSTANCE ), NULL );
	if ( hwStatusbar )
	{
		if ( ! ( hStatusbar = GETTHISDATA ( hwStatusbar ) ) )
			ErrorCode = WSTB_ERR_GETWINPROP;
		else
		if ( ! ( pthis = ( STATUSBARDATA *) GlobalLock ( hStatusbar ) ) )
			ErrorCode = WSTB_ERR_GLOBALLOCK;
		else
		if ( lpText )
		{
			Statbr_ISetPartText ( &pthis->Simple, lpText );
		}

        if ( pthis )
            GlobalUnlock ( hStatusbar );

	}

	return hwStatusbar;

}

void WINAPI
WDrawStatusText
(
    HDC                  hDC,
    LPRECT               lprc,
    LPCTSTR              pszText,
    UINT                 uFlags
)
{
    if ( ! pszText )
        return;

    FillRect ( hDC, lprc, GetStockObject ( LTGRAY_BRUSH ) );
    SetBkMode ( hDC, TRANSPARENT );
    DrawText ( hDC, pszText, -1, lprc, DT_SINGLELINE | DT_VCENTER );
    Statbr_IDrawPartBorder ( hDC, uFlags, lprc );
}
/* Note: A closer look at this function would reveal that this 
   function would never traverse the array pointed to by lpwIDs
   parameter ( no wonder the array size is not passed ), but
   instead expect lpwIDs to be pointing to an array element who
   contains a pair of menu handle, which is exactly equal to 
   hMainMenu, and its corresponding string ID, then the string ID
   for the popup menu item would be deduced by adding the hMainMenu
   string ID and the menu index found in LOWORD(wParam) */

void WINAPI
WMenuHelp
(
    	UINT                	uMsg,
    	WPARAM              	wParam,
    	LPARAM              	lParam,
    	HMENU               	hMainMenu,
    	HINSTANCE           	hInst,
    	HWND                	hwStatus,
    	UINT FAR*           	lpwIDs
)
{
	HGLOBAL			hStatusbar	    = ( HGLOBAL )NULL;
	STATUSBARDATA*		pthis		    = ( STATUSBARDATA* )NULL;
	int		    	ErrorCode	    = 0;
	
	if ( ! ( hStatusbar = GETTHISDATA ( hwStatus ) ) )
		ErrorCode = WSTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( STATUSBARDATA *) GlobalLock ( hStatusbar ) ) )
		ErrorCode = WSTB_ERR_GLOBALLOCK;
	else
	{
    		STBMENUSTR*         pMenuStr;
    		char                buf[256];
    		RECT                rc;
    		HDC                 hDC         = ( HDC )NULL;
    		UINT                uMenuID     = 0;
		HFONT		    hOldFont	= ( HFONT )NULL;
		UINT		    fuFlags	= ( UINT )HIWORD( wParam );
    		pMenuStr = ( STBMENUSTR* )lpwIDs;

    		if ( pMenuStr->hMenu == hMainMenu )
		{
		    if ( fuFlags & MF_POPUP )
		    {
    		    	uMenuID = pMenuStr->uiString;
			/*printf("Bingo: %i %i\n",uMenuID, LOWORD( wParam ) );*/
		     }
		     else
    		    	uMenuID = LOWORD ( wParam ); 
		}

		if ( ( uMenuID == 0 ) && pthis->bSimple )
		{
			GlobalUnlock ( hStatusbar );
			InvalidateRect ( hwStatus, ( RECT* )NULL, TRUE );
			return;
		}
		else
		if ( LoadString ( hInst, uMenuID, buf, 255 ) == 0 )
    		    buf[0] = 0; 
    		if ( ( hDC = GetDC ( hwStatus ) ) )
    		{
        		GetClientRect ( hwStatus, &rc );
			hOldFont = SelectObject ( hDC, pthis->hFont );
        		WDrawStatusText ( hDC, &rc, ( LPCTSTR )&buf, SBT_NOBORDERS );
			SelectObject ( hDC, hOldFont );
        		ReleaseDC ( hwStatus, hDC );
    		}
	}

	if ( pthis )
		GlobalUnlock ( hStatusbar );

}


/* Status bar Implementation*/
static BOOL
Statbr_SetParts
( 
	HWND				hwStatsbar,
	STATUSBARDATA*		pthis,
	int 				nParts,
	LPINT   			aWidths
)
{
	int				    i;
	UINT				    Count;
    STBITEM             sbi;
	STBITEM*			pItemList	= ( STBITEM* )NULL;
	STBITEM*			pItem		= ( STBITEM* )NULL;
    int                 ErrorCode   = 0;
	
    if ( nParts > 255 )
        return FALSE;

    WLCount ( &pthis->ItmList, &Count );
    memset ( &sbi, 0, sizeof ( STBITEM ) );
    if ( Count < nParts )
    {
        for ( i = Count; i < nParts && ErrorCode == 0; i++ )
    	    if ( WLInsert ( &pthis->ItmList, i, &sbi ) < 0 )
	    	    ErrorCode = WSTB_ERR_LISTINSERT;

        if ( ErrorCode )
            return FALSE;
    }

    pthis->ItmCount = nParts;
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItemList ) )
		ErrorCode = WSTB_ERR_LISTLOCK;
	else
    {
	    int left = pthis->nXPadding;
        int top = pthis->nYPadding;
        int bottom = pthis->Size.cy - pthis->nYPadding;
        int right = 0;
        for ( i = 0; i < pthis->ItmCount; i++ )
	    {
            pItem = pItemList + i;
            right = aWidths[i] != -1 ? aWidths[i] :
                                       pthis->Size.cx - pthis->nXPadding;
            SetRect ( &pItem->Rect, left, top, right, bottom );
            left = ( right + pthis->nXGap );
	    }
    }

	if ( pItemList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pItemList );

	return ErrorCode >= 0;
}

static BOOL
Statbr_SetText
( 
	HWND				hwStatsbar,
	STATUSBARDATA*			pthis,
	int 				iInfo,
	LPCTSTR   			szText
)
{
	UINT			    	Count;
	STBITEM*			pItemList	= ( STBITEM* )NULL;
	STBITEM*			pItem		= ( STBITEM* )NULL;
    	int                 		indx        	= 0x00FF;
    	int                 		ErrorCode   	= 0;
	

	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItemList ) )
		ErrorCode = WSTB_ERR_LISTLOCK;
	else
    	if ( szText )
    	{
    	    Count = strlen ( szText );
    	    indx = iInfo & 0xff;
    	    if ( ( indx == 255 ) || ( indx == 0 && pthis->ItmCount == 0 ) )
    	    {
    	        pItem = &pthis->Simple;
    	        pthis->bSimple = TRUE;
    	    }
    	    else
	    if ( indx < pthis->ItmCount )
    	        pItem = pItemList + indx;
	    if ( pItem )
	    {
	    	Statbr_ISetPartText ( pItem, szText );
    	    	pItem->uType = iInfo & 0xff00;
	
    	    	InvalidateRect ( hwStatsbar, &pItem->Rect, TRUE );
    	    	UpdateWindow ( hwStatsbar ); 
	    }
    	}

	if ( pItemList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pItemList );

	return ErrorCode >= 0;
}

static BOOL
Statbr_Simple
( 
	HWND			hwStatsbar,
	STATUSBARDATA*		pthis,
	BOOL 			fSimple
)
{
	BOOL                bChanged;

	bChanged = pthis->bSimple && fSimple;
	pthis->bSimple = fSimple;
	if ( bChanged )
	{
		InvalidateRect ( hwStatsbar, ( LPRECT )NULL, TRUE );
		UpdateWindow ( hwStatsbar );
	}

	return TRUE;
}

extern LPSTR GetTwinFilename();

static void
Statbr_AutoSize
( 
	HWND			hwStatbar,
	STATUSBARDATA*		pthis
)
{
	RECT			rect;
    	TEXTMETRIC          	tm;
    	HDC                 	hDC     = GetDC ( hwStatbar );

    
    	GetClientRect ( GetParent ( hwStatbar ), &rect );

	pthis->Size.cx = rect.right - rect.left;
	GetTextMetrics ( hDC, &tm );
    	pthis->Size.cy = tm.tmHeight + 2 * pthis->nYPadding;
	pthis->Pos.x = rect.left;

        /* CCS_TOP = 0x00000001 CCS_BOTTOM = 0x00000003, not mutually exclusive */
	pthis->Pos.y = (pthis->uStyles & CCS_BOTTOM) == CCS_TOP ? rect.top:
						  rect.bottom - pthis->Size.cy;
    	if ( pthis->uStyles & SBARS_SIZEGRIP  && 
    		(GetPrivateProfileInt("boot", "SizeGrip", TRUE, GetTwinFilename())))
		    SetRect ( &pthis->rcSizeGrip, rect.right - pthis->Size.cy, 0,
                                   rect.right, pthis->Size.cy );
    	else
        	SetRectEmpty ( &pthis->rcSizeGrip );
    
    	SetRect ( &pthis->Simple.Rect, pthis->nXPadding, pthis->nYPadding,
                                   pthis->Size.cx - pthis->nXPadding,
                                   pthis->Size.cy - pthis->nYPadding );

	SetWindowPos ( hwStatbar, ( HWND )NULL, pthis->Pos.x, pthis->Pos.y,
		   pthis->Size.cx, pthis->Size.cy,
		   SWP_NOACTIVATE | SWP_NOZORDER );

    	ReleaseDC ( hwStatbar, hDC );
}


static BOOL
Statbr_GetBorders
( 
	HWND				hwStatsbar,
	STATUSBARDATA*		pthis,
	int 				nParts,
	LPINT   			aWidths
)
{
	
    if ( aWidths == ( LPINT )NULL )
        return FALSE;
    aWidths[0] = pthis->nXPadding;
    aWidths[1] = pthis->nYPadding;
    aWidths[2] = pthis->nXGap;

	return TRUE;
}

       
static BOOL
Statbr_GetParts
( 
	HWND				hwStatsbar,
	STATUSBARDATA*		pthis,
	int 				nParts,
	LPINT   			aWidths
)
{
	int				    i;
	int				    Count;
	STBITEM*			pItemList	= ( STBITEM* )NULL;
	STBITEM*			pItem		= ( STBITEM* )NULL;
    int                 ErrorCode   = 0;
	
    if ( aWidths == ( LPINT )NULL )
        return pthis->ItmCount;
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItemList ) )
		ErrorCode = WSTB_ERR_LISTLOCK;
	else
    {
        Count = nParts > pthis->ItmCount ? pthis->ItmCount : nParts;
        for ( i = 0; i < Count; i++ )
	    {
            pItem = pItemList + i;
            if ( pItem->Rect.right  >= pthis->Size.cx - pthis->nYPadding )
                aWidths[i] = -1;
            else
                aWidths[i] = pItem->Rect.right;
	    }
    }

	if ( pItemList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pItemList );

	return ErrorCode >= 0;
}


static BOOL
Statbr_GetRect
(
	HWND				hwStatsbar,
	STATUSBARDATA*		pthis, 
	int				    iPart,
    LPRECT              lprc
)
{
    BOOL                Result      = FALSE;
	STBITEM*			pItemList	= ( STBITEM* )NULL;
	STBITEM*			pItem		= ( STBITEM* )NULL;
	int				    ErrorCode	= 0;

	if ( lprc == ( LPRECT )NULL )
        return FALSE;
    if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItemList ) )
		ErrorCode = WSTB_ERR_LISTLOCK;
    else
    if ( iPart >= pthis->ItmCount )
		ErrorCode = WSTB_ERR_OUTOFRANGE;
	else
    {
        pItem = pItemList + iPart;
        CopyRect ( lprc, &pItem->Rect );
        Result = TRUE;
    }

	if ( pItemList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pItemList );

	return Result;
}


static LONG
Statbr_GetText
(
	HWND				hwStatsbar,
	STATUSBARDATA*			pthis, 
	int			    	iPart,
	int				cchMaxText,
	LPSTR				lpPartText
)
{
    	LONG                		Result      	= 0L;
	STBITEM*			pItemList	= ( STBITEM* )NULL;
	STBITEM*			pItem		= ( STBITEM* )NULL;
	int			    	len	    	= 0;
	LPVOID				lpText		= NULL;
	int				ErrorCode	= 0;

	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItemList ) )
		ErrorCode = WSTB_ERR_LISTLOCK;
    	else
    	if ( iPart >= pthis->ItmCount )
		ErrorCode = WSTB_ERR_OUTOFRANGE;
	else
    	{
	    if ( iPart == 255 )
		pItem = &pthis->Simple;
	    else 
    	    	pItem = pItemList + iPart;
    	    if ( pItem->uType == SBT_OWNERDRAW )
    	        Result = pItem->lText;
    	    else
    	    if ( ( pItem->hText ) &&
    	         ( lpText = GlobalLock ( pItem->hText ) ) )
    	    {
		if ( lpPartText )
		{
			if ( cchMaxText == -1 )
    	       			lstrcpy ( lpPartText, lpText );
			else
                                lstrcpyn ( lpPartText, lpText, cchMaxText );
		}
    	        len = lstrlen ( lpText );
    	        Result = MAKELONG ( len, pItem->uType );
    	        GlobalUnlock ( pItem->hText );
    	    }
    	}

	if ( pItemList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pItemList );

	return Result;
}


static LONG
Statbr_GetTextLength
(
	HWND				hwStatsbar,
	STATUSBARDATA*			pthis, 
	int				iPart
)
{
    	LONG                		Result      	= 0L;
	STBITEM*			pItemList	= ( STBITEM* )NULL;
	STBITEM*			pItem		= ( STBITEM* )NULL;
	int			    	len	    	= 0;
	LPVOID				lpText		= NULL;
	int			    	ErrorCode	= 0;

	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItemList ) )
		ErrorCode = WSTB_ERR_LISTLOCK;
    	else
    	if ( iPart >= pthis->ItmCount )
		ErrorCode = WSTB_ERR_OUTOFRANGE;
	else
    	{
    	    pItem = pItemList + iPart;
    	    if ( ( pItem->hText ) &&
    	         ( lpText = GlobalLock ( pItem->hText ) ) )
    	    {
    	        len = lstrlen ( lpText );
    	        Result = MAKELONG ( len, pItem->uType );
    	        GlobalUnlock ( pItem->hText );
    	    }
    	}

	if ( pItemList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pItemList );

	return Result;
}


static BOOL
Statbr_OnCreate 
(
	HWND			hWnd, 
	CREATESTRUCT*		pcs
) 
{
	HGLOBAL			hStatusbar	= ( HGLOBAL )NULL;
	STATUSBARDATA*		pthis		= ( STATUSBARDATA* )NULL;
        HDC                     hDC         	= GetDC ( hWnd );

	int		    	ErrorCode	= 0;

	if ( ! ( hStatusbar = GlobalAlloc ( GHND, sizeof ( STATUSBARDATA ) ) ) )
		ErrorCode = WSTB_ERR_GLOBALALLOC;
	else
	if ( ! ( pthis = ( STATUSBARDATA *) GlobalLock ( hStatusbar ) ) )
		ErrorCode = WSTB_ERR_GLOBALLOCK;
	else
	{
		pthis->uStyles = pcs->style;
		pthis->nXPadding = GetSystemMetrics ( SM_CXBORDER );
		pthis->nYPadding = GetSystemMetrics ( SM_CYBORDER );
        	pthis->nXGap = HORZ_PADDING;
#if 0	/* MFC4: assumes default is multipart status bar */
        	pthis->bSimple = TRUE;
#else	/* MFC4 */
        	pthis->bSimple = FALSE;
#endif	/* MFC4 */
        
        	pthis->hFont = ( HFONT )GetStockObject ( ANSI_VAR_FONT );

        	WLCreate ( &pthis->ItmList, sizeof ( STBITEM ) );
        	pthis->hDefCursor = LoadCursor ( ( HINSTANCE )NULL, IDC_ARROW );
		pthis->hCursor = LoadCursor ( ( HINSTANCE )NULL, IDC_SIZENWSE ); 

/*		WLCreate ( &pthis->StrList, sizeof ( TLBSTR ) );*/

		SETTHISDATA ( hWnd, hStatusbar );
        	Statbr_AutoSize ( hWnd, pthis );
	}

	if ( pthis )
		GlobalUnlock ( hStatusbar );

    if ( hDC )
        	ReleaseDC ( hWnd, hDC );
	return ( ErrorCode >= 0 );
}

static void Statbr_OnLButtonDown 
(	
	HWND				hWnd, 
	int				    x, 
	int				    y, 
	UINT				keyFlags
) 
{
	HGLOBAL				hStatusbar	    = ( HGLOBAL )NULL;
	STATUSBARDATA*		pthis		    = ( STATUSBARDATA* )NULL;
	STBITEM*			pStatBItmList	= ( STBITEM* )NULL;
	int				    ErrorCode	    = 0;
	
	if ( ! ( hStatusbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WSTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( STATUSBARDATA *) GlobalLock ( hStatusbar ) ) )
		ErrorCode = WSTB_ERR_GLOBALLOCK;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pStatBItmList ) )
		ErrorCode = WSTB_ERR_LISTLOCK;
	else
    if ( pthis->enDrag == eResizeReady )
	{
		GetWindowRect ( GetParent ( hWnd ), &pthis->rcOutline );
        pthis->rcOutline.left += 2;
        pthis->rcOutline.top += 2;
        Statbr_IDrawBoxOutline ( hWnd, &pthis->rcOutline );
        pthis->enDrag = eResizing;
        pthis->ptLBDown.x = x;
        pthis->ptLBDown.y = y;
        ClientToScreen ( hWnd, &pthis->ptLBDown );
        SetCapture ( hWnd );
	}

	if ( pStatBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pStatBItmList );
	if ( pthis )
		GlobalUnlock ( hStatusbar );
}


static void Statbr_OnLButtonUp 
(	
	HWND				hWnd, 
	int				    x, 
	int				    y, 
	UINT				keyFlags
) 
{
	HGLOBAL				hStatusbar	    = ( HGLOBAL )NULL;
	STATUSBARDATA*		pthis		    = ( STATUSBARDATA* )NULL;
	STBITEM*			pStatBItmList	= ( STBITEM* )NULL;

	int				ErrorCode	= 0;
	
	if ( ! ( hStatusbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WSTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( STATUSBARDATA* ) GlobalLock ( hStatusbar ) ) )
		ErrorCode = WSTB_ERR_GLOBALLOCK;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pStatBItmList ) )
		ErrorCode = WSTB_ERR_LISTLOCK;
	else
    if ( pthis->enDrag == eResizing )
	{
        HWND   hwParent = GetParent ( hWnd );
        Statbr_IDrawBoxOutline ( hwParent, &pthis->rcOutline );
		pthis->enDrag = eNull;
        ReleaseCapture ( );
		SetWindowPos ( hwParent, ( HWND )NULL, pthis->rcOutline.left, pthis->rcOutline.top,
			   pthis->rcOutline.right - pthis->rcOutline.left, 
               pthis->rcOutline.bottom - pthis->rcOutline.top,
			   SWP_NOACTIVATE | SWP_NOZORDER );

	}

	if ( pStatBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pStatBItmList );
	if ( pthis )
		GlobalUnlock ( hStatusbar );
}

static void
Statbr_DestroyParts
(
    STATUSBARDATA*		pthis
)
{
    int                 i;
    UINT                 Count;
    STBITEM*		pItemList	= ( STBITEM* )NULL;
    STBITEM*		pItem		= ( STBITEM* )NULL;
    int                 ErrorCode   	= 0;

    WLCount ( &pthis->ItmList, &Count );

    if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItemList ) )
		ErrorCode = WSTB_ERR_LISTLOCK;
	else
    {
	if ( pthis->Simple.hText )
		GlobalFree ( pthis->Simple.hText );
        for ( i = 0; i < Count; i++ )
        {
            pItem = pItemList + i;
            if ( pItem->hText )
                GlobalFree ( pItem->hText );
        }
    }

    if ( pItemList )
	WLUnlock ( &pthis->ItmList, ( LPVOID )&pItemList );
    WLDestroy ( &pthis->ItmList );
}

static void 
Statbr_OnDestroy 
(	
	HWND				hWnd 
)
{
	HGLOBAL				hStatusbar	= ( HGLOBAL )NULL;
	STATUSBARDATA*		pthis		= ( STATUSBARDATA* )NULL;

	int				ErrorCode	= 0;

	if ( ! ( hStatusbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WSTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( STATUSBARDATA *) GlobalLock ( hStatusbar ) ) )
		ErrorCode = WSTB_ERR_GLOBALLOCK;
	else
	{
		REMOVETHISDATA ( hWnd );
       		Statbr_DestroyParts ( pthis );
		if ( pthis->hFont )
		    DeleteObject ( pthis->hFont );
        	if ( pthis->hCursor )
        	    DestroyCursor ( pthis->hCursor );
        	if ( pthis->hDefCursor )
        	    DestroyCursor ( pthis->hDefCursor );
		/*DeleteObject ( pthis->hbrDither );*/
		GlobalFree ( hStatusbar );
	}
}

static void
Statbr_IDrawBoxOutline
(
    HWND                hWnd,
    LPRECT              prcBox
)
{
    HDC                 hdc;
    int                 mode;
    HBRUSH              hbr;
    HPEN                hpen    = CreatePen ( PS_SOLID, 3, RGB ( 192, 192, 192 ) );
    HPEN                htmppen = ( HPEN )NULL;

    hdc = GetDC ( ( HWND )NULL );

    htmppen = SelectObject ( hdc, hpen );
    hbr = SelectObject ( hdc, GetStockObject ( NULL_BRUSH ) );
    mode = SetROP2 ( hdc, R2_NOT );
    Rectangle ( hdc, prcBox->left, prcBox->top, prcBox->right, prcBox->bottom );

    SetROP2 ( hdc, mode );
    SelectObject ( hdc, htmppen );
    SelectObject ( hdc, hbr );
    DeleteObject ( hpen );
    ReleaseDC ( ( HWND )NULL, hdc );
}

static HFONT 
Statbr_OnGetFont 
(
	HWND			hWnd
) 
{
	HGLOBAL			hStatusbar	= ( HGLOBAL )NULL;
	STATUSBARDATA*		pthis		= ( STATUSBARDATA* )NULL;
	HFONT			hFont		= ( HFONT )NULL;
	int			ErrorCode	= 0;

	if ( ! ( hStatusbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WSTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( STATUSBARDATA *) GlobalLock ( hStatusbar ) ) )
		ErrorCode = WSTB_ERR_GLOBALLOCK;
	else
	{
		hFont = pthis->hFont;	
	}

	if ( pthis )
		GlobalUnlock ( hStatusbar );

	return hFont;
}

static LONG 
Statbr_OnGetText 
(
	HWND			hWnd,
	int			cchTextMax,
	LPTSTR			lpText	
) 
{
	LONG			lInfo		= 0L;
	HGLOBAL			hStatusbar	= ( HGLOBAL )NULL;
	STATUSBARDATA*		pthis		= ( STATUSBARDATA* )NULL;
	int			ErrorCode	= 0;

	if ( ! ( hStatusbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WSTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( STATUSBARDATA *) GlobalLock ( hStatusbar ) ) )
		ErrorCode = WSTB_ERR_GLOBALLOCK;
	else
	{
		int indx = pthis->bSimple ? 255 : 0;
		lInfo = Statbr_GetText ( hWnd, pthis, indx, cchTextMax, lpText );
	}

	if ( pthis )
		GlobalUnlock ( hStatusbar );

	return lInfo;
}

static void 
Statbr_OnMouseMove 
(
	HWND			    hWnd, 
	UINT			    wParam,
	LPARAM			    lParam 
) 
{
	static POINT		pt;
	STBITEM*			pItemList	= ( STBITEM* )NULL;
	HGLOBAL				hStatusbar	= ( HGLOBAL )NULL;
	STATUSBARDATA*		pthis		= ( STATUSBARDATA* )NULL;
	int				    ErrorCode	= 0;

	if ( ! ( hStatusbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WSTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( STATUSBARDATA *) GlobalLock ( hStatusbar ) ) )
		ErrorCode = WSTB_ERR_GLOBALLOCK;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItemList ) )
		ErrorCode = WSTB_ERR_LISTLOCK;
	else
	{
		pt.x = LOWORD ( lParam );
		pt.y = HIWORD ( lParam );
        
		if ( pthis->enDrag == eResizing )
        {
            Statbr_IDrawBoxOutline ( hWnd, &pthis->rcOutline );
            ClientToScreen ( hWnd, &pt );
            pthis->rcOutline.right += ( pt.x - pthis->ptLBDown.x );
            pthis->rcOutline.bottom += ( pt.y - pthis->ptLBDown.y );
            pthis->ptLBDown = pt;
            Statbr_IDrawBoxOutline ( hWnd, &pthis->rcOutline );
        }
        else
        if ( PtInRect ( &pthis->rcSizeGrip, pt ) )
        {
			SetCursor ( pthis->hCursor );
		    if ( pthis->enDrag == eNull )
                pthis->enDrag = eResizeReady;
        }
        else
        {
            SetCursor ( pthis->hDefCursor );
            pthis->enDrag = eNull;
        }
	}

	if ( pItemList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pItemList );

	if ( pthis )
		GlobalUnlock ( hStatusbar );
}

static void Statbr_DrawSizeGrip
(
	HWND			    hWnd,
	HDC			        hDC,
	STATUSBARDATA*	    pthis
)
{
	int                 i;
    RECT				rc;
	HPEN				hbPen	= CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNTEXT ) );
	HPEN				hOPen;

	CopyRect ( &rc, &pthis->rcSizeGrip );
    rc.top = rc.bottom - 3;
    rc.left = rc.right -3;
	hOPen = SelectObject ( hDC, hbPen );
    for ( i = 1; i <= 3; i++ )
    {
        MoveToEx ( hDC, rc.right , rc.top, NULL );
    	LineTo ( hDC, rc.left, rc.bottom );
    	SelectObject ( hDC, GetStockObject ( WHITE_PEN ) );
        MoveToEx ( hDC, rc.right , rc.top - 2, NULL );
    	LineTo ( hDC, rc.left - 2, rc.bottom );
	    SelectObject ( hDC, hbPen );
        rc.top -= 5;
        rc.left -= 5;

    }
	SelectObject ( hDC, hOPen );
	
	DeleteObject ( hbPen );
}

static void Statbr_OnPaint 
(
	HWND				hWnd, 
	HDC			    	hPaintDC 
)
{
	PAINTSTRUCT			ps;
	STBITEM*			pStatBItmList	    = ( STBITEM* )NULL;
	STBITEM*			pItem		    = ( STBITEM* )NULL;
	HGLOBAL				hStatusbar	    = ( HGLOBAL )NULL;
	STATUSBARDATA*			pthis		    = ( STATUSBARDATA* )NULL;
	HFONT				OldFont		    = ( HFONT )NULL;
	int				    ErrorCode	    = 0;

	if ( ! ( hStatusbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WSTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( STATUSBARDATA *) GlobalLock ( hStatusbar ) ) )
		ErrorCode = WSTB_ERR_GLOBALLOCK;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pStatBItmList ) )
		ErrorCode = WSTB_ERR_LISTLOCK;
	else
	{
		SetWindowPos ( hWnd, ( HWND )NULL, pthis->Pos.x, pthis->Pos.y,
			   pthis->Size.cx, pthis->Size.cy,
			   SWP_NOACTIVATE | SWP_NOZORDER );
		BeginPaint ( hWnd, &ps );
		/*if ( pthis->ItmCount > 0 )*/
		{
			int i;
			OldFont = SelectObject ( ps.hdc, pthis->hFont );
			SetBkMode ( ps.hdc, TRANSPARENT );
            	if ( pthis->bSimple )
            	{
            	    pItem = &pthis->Simple;
		    Statbr_IDrawPart ( hWnd, ps.hdc, pthis, pItem ); 
            	}
            	else
		    for ( i = 0; i < pthis->ItmCount ; i++ )
		    {
		        pItem = pStatBItmList + i;
			if ( pItem->uType == SBT_OWNERDRAW )
			{
			    DRAWITEMSTRUCT di;
			    di.CtlID = GetDlgCtrlID ( hWnd );
		            di.itemID = i;
			    di.hwndItem = hWnd;
			    di.hDC = ps.hdc;
			    CopyRect ( &di.rcItem, &pItem->Rect );
			    di.itemData = pItem->lText;
			    SendMessage ( GetParent ( hWnd ), WM_DRAWITEM,
					  GetDlgCtrlID ( hWnd ), ( LPARAM )&di );
			}
			else
			    Statbr_IDrawPart ( hWnd, ps.hdc, pthis, pItem ); 
		    }


		SelectObject ( ps.hdc, OldFont );
		}

        if ( pthis->uStyles & SBARS_SIZEGRIP && 
    		(GetPrivateProfileInt("boot", "SizeGrip", TRUE, (LPSTR) GetTwinFilename())))
            Statbr_DrawSizeGrip ( hWnd, ps.hdc, pthis );

		EndPaint ( hWnd, &ps );
	}


	if ( pStatBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pStatBItmList );
	if ( pthis )
		GlobalUnlock ( hStatusbar );
}

static int 
Statbr_OnSize 
(
	HWND			hWnd,
	DWORD			fwSizeType,
	int			    nWidth,
	int			    nHeight
) 
{
	HGLOBAL			hStatusbar	= ( HGLOBAL )NULL;
	STATUSBARDATA*	pthis		= ( STATUSBARDATA* )NULL;
	int				ErrorCode	= 0;

	if ( ! ( hStatusbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WSTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( STATUSBARDATA *) GlobalLock ( hStatusbar ) ) )
		ErrorCode = WSTB_ERR_GLOBALLOCK;
	else
	{
		Statbr_AutoSize ( hWnd, pthis );
	}

	if ( pthis )
		GlobalUnlock ( hStatusbar );

	return 0;	
}


static void 
Statbr_OnSetFont 
(
	HWND			hWnd,
	HFONT			hFont,
	BOOL			bReDraw
) 
{
	HGLOBAL			hStatusbar	= ( HGLOBAL )NULL;
	STATUSBARDATA*		pthis		= ( STATUSBARDATA* )NULL;
	int			ErrorCode	= 0;

	if ( ! ( hStatusbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WSTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( STATUSBARDATA *) GlobalLock ( hStatusbar ) ) )
		ErrorCode = WSTB_ERR_GLOBALLOCK;
	else
	{
		if ( hFont )
			pthis->hFont = hFont;	
		if ( bReDraw )
			InvalidateRect ( hWnd, ( RECT*)NULL, TRUE );
	}

	if ( pthis )
		GlobalUnlock ( hStatusbar );

}

static BOOL 
Statbr_OnSetText 
(
	HWND			hWnd,
	LPCTSTR			lpText	
) 
{
	HGLOBAL			hStatusbar	= ( HGLOBAL )NULL;
	STATUSBARDATA*		pthis		= ( STATUSBARDATA* )NULL;
	int			ErrorCode	= 0;

	if ( ! ( hStatusbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WSTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( STATUSBARDATA *) GlobalLock ( hStatusbar ) ) )
		ErrorCode = WSTB_ERR_GLOBALLOCK;
	else
	if ( pthis->bSimple )
	{
		Statbr_ISetPartText ( &pthis->Simple, lpText );
		InvalidateRect ( hWnd, ( RECT *)NULL, TRUE );
		UpdateWindow ( hWnd );
 	}
	else
	{
		LONG lInfo = Statbr_GetText ( hWnd, pthis, 0, -1, ( LPSTR )NULL );
		Statbr_SetText ( hWnd, pthis, 0 | HIWORD ( lInfo ), lpText );
	}

	if ( pthis )
		GlobalUnlock ( hStatusbar );

	return ( ErrorCode >= 0 );
}

static void
Statbr_IDrawPartBorder 
( 
	HDC			    	hDC, 
	UINT				uType, 
	RECT*				pRect
)
{
	HPEN				hwPen	= CreatePen ( PS_SOLID, 1, RGB ( 255,255,255 ) );
	HPEN				hbPen	= CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNSHADOW ) );
	HPEN				hsdPen  = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNTEXT ) );
	HPEN				htmpPen = ( HPEN )NULL;

	
    if ( uType == 0 )
	{	
		/* draw left-top borders */
		htmpPen = SelectObject ( hDC, hbPen );
		MoveToEx ( hDC, pRect->left, pRect->bottom, NULL );
		LineTo ( hDC, pRect->left, pRect->top );
		LineTo ( hDC, pRect->right, pRect->top );
		SelectObject ( hDC, hsdPen );
		MoveToEx ( hDC, pRect->left + 1, pRect->bottom, NULL );
		LineTo ( hDC, pRect->left + 1, pRect->top + 1 );
		LineTo ( hDC, pRect->right, pRect->top + 1 );

		/* draw right-bottom borders */
		SelectObject ( hDC, hwPen );
		MoveToEx ( hDC, pRect->right - 1, pRect->top, NULL );
		LineTo ( hDC, pRect->right - 1, pRect->bottom );
		LineTo ( hDC, pRect->left, pRect->bottom );
	}
	else
    if ( uType == SBT_POPOUT )
	{
		/* draw left-top borders */
		htmpPen = SelectObject ( hDC, hwPen );
		MoveToEx ( hDC, pRect->left, pRect->bottom, NULL );
		LineTo ( hDC, pRect->left, pRect->top );
		LineTo ( hDC, pRect->right -1, pRect->top );

		/* draw right-bottom borders */
		SelectObject ( hDC, hbPen );
		MoveToEx ( hDC, pRect->right - 1, pRect->top, NULL );
		LineTo ( hDC, pRect->right - 1, pRect->bottom - 1 );
		LineTo ( hDC, pRect->left + 1, pRect->bottom - 1 );
		SelectObject ( hDC, hsdPen );
		MoveToEx ( hDC, pRect->right, pRect->top + 1, NULL );
		LineTo ( hDC, pRect->right, pRect->bottom);
		LineTo ( hDC, pRect->left, pRect->bottom );
	}

	if ( htmpPen )
        SelectObject ( hDC, htmpPen );

	/*cleanup*/
	DeleteObject ( hwPen );
	DeleteObject ( hbPen );
	DeleteObject ( hsdPen );

}

/*new*/
static void
Statbr_IDrawPart 
( 
	HWND				hWnd,
	HDC				    hDC, 
	STATUSBARDATA*		pthis, 
	STBITEM*			pItem
)
{

	FillRect ( hDC, &pItem->Rect, GetStockObject ( LTGRAY_BRUSH ) );

	/*draw content of a status bar part*/
	/*Statbr_IGetBtnRects ( hDC, pthis, pItem, &rcIcon, &rcLabel );*/
	if ( pItem->hText )
	{
        	LPSTR lpText = GlobalLock ( pItem->hText );
		DrawText ( hDC, lpText, -1, &pItem->Rect, DT_SINGLELINE | DT_VCENTER );
        	GlobalUnlock ( pItem->hText );
	}

	Statbr_IDrawPartBorder ( hDC, pItem->uType, &pItem->Rect );

}


static BOOL
Statbr_ISetPartText
( 
	STBITEM*			pItem,
	LPCSTR   			szText
)
{
	int			    	Count;
    	LPSTR               		lpText      	= ( LPSTR )NULL;
    	int                 		ErrorCode   	= 0;
	

    	if ( szText )
    	{
    	    Count = strlen ( szText ) + 1;

    	    if ( pItem->hText == ( HANDLE )NULL )
    	        pItem->hText = GlobalAlloc ( GHND, Count * sizeof ( char ) );
    	    else
    	        pItem->hText = GlobalReAlloc ( pItem->hText, Count, GMEM_ZEROINIT );
    	    if ( ( lpText = GlobalLock ( pItem->hText ) ) )
    	    {
    	        lstrcpy ( lpText, szText );
    	        GlobalUnlock ( pItem->hText );
    	    }

    	}

	return ErrorCode >= 0;
}

static LRESULT WINAPI 
Statbr_WndProc
(
	HWND				hWnd,
    	UINT				uMessage,
    	WPARAM				wParam,
    	LPARAM				lParam
)
{
	LRESULT				lResult = 0L;
	switch (uMessage)
	{
		case WM_CREATE:
			if ( ! Statbr_OnCreate ( hWnd, ( LPCREATESTRUCT )lParam ) )
				return ( LRESULT ) -1L;
			break;
		case WM_DESTROY:
			Statbr_OnDestroy ( hWnd );
			break;
		
		case WM_GETFONT:
			lResult = Statbr_OnGetFont( hWnd );
			break;

		case WM_GETTEXT:
			lResult = Statbr_OnGetText( hWnd, ( int )wParam, ( LPTSTR )lParam );
			break;
		
		case WM_GETDLGCODE:
			return ( LRESULT )DLGC_WANTARROWS | DLGC_WANTCHARS;
		
		case WM_LBUTTONDOWN:
			Statbr_OnLButtonDown ( hWnd,(int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )( wParam ) );
			break;

		case WM_LBUTTONUP:
			Statbr_OnLButtonUp ( hWnd,(int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )( wParam ) );
			break;

		case WM_MOUSEMOVE:
			Statbr_OnMouseMove ( hWnd, ( UINT )wParam, lParam );
			break;

        	case WM_PAINT:
			Statbr_OnPaint( hWnd, ( HDC ) wParam  );
			break;

		case WM_SETFONT:
			Statbr_OnSetFont( hWnd, ( HFONT )wParam, ( BOOL )lParam );
			break;

		case WM_SIZE:
			Statbr_OnSize( hWnd, ( DWORD )wParam, 
				( int )LOWORD ( lParam ), ( int )HIWORD ( lParam ) );
			break;
		
		case WM_SETTEXT:
			lResult = Statbr_OnSetText( hWnd, ( LPCTSTR )lParam );
			break;
		
		case SB_GETBORDERS:
		case SB_GETPARTS:
		case SB_GETRECT:
		case SB_GETTEXT:
		case SB_GETTEXTLENGTH:
		case SB_SETMINHEIGHT:
		case SB_SETPARTS:	
		case SB_SETTEXT:	
		case SB_SIMPLE:	
			return Statbr_STBWndProc(hWnd, uMessage, wParam, lParam);

	        default:
			return DefWindowProc(hWnd, uMessage, wParam, lParam);
	}
    
	    return lResult;
}

static LRESULT  
Statbr_STBWndProc
(
	HWND				hWnd,
    UINT				uMessage,
    WPARAM				wParam,
    LPARAM				lParam
)
{
	HGLOBAL				hStatusbar	= ( HGLOBAL )NULL;
	STATUSBARDATA*		pthis		= ( STATUSBARDATA* )NULL;
	LRESULT				lResult		= 0L;
	
	int				    ErrorCode	= 0;

	if ( ! ( hStatusbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WSTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( STATUSBARDATA *) GlobalLock ( hStatusbar ) ) )
		ErrorCode = WSTB_ERR_GLOBALLOCK;
	else
	{
		switch ( uMessage )
		{
			case SB_GETBORDERS:
				lResult = ( LRESULT )Statbr_GetBorders ( hWnd, pthis, ( int )wParam, 
							( LPINT )lParam );
				break;
			case SB_GETPARTS:
				lResult = ( LRESULT )Statbr_GetParts ( hWnd, pthis, ( int )wParam, 
							( LPINT )lParam );
				break;
			case SB_GETRECT:
				lResult = ( LRESULT )Statbr_GetRect ( hWnd, pthis, ( int )wParam, 
							( LPRECT )lParam );
				break;
			case SB_GETTEXT:
				lResult = ( LRESULT )Statbr_GetText ( hWnd, pthis, ( int )wParam, 
							-1, ( LPSTR )lParam );
				break;
			case SB_GETTEXTLENGTH:
				lResult = ( LRESULT )Statbr_GetTextLength ( hWnd, pthis, ( int )wParam ); 
				break;
		        case SB_SETMINHEIGHT:
		                pthis->Size.cy = ( int )wParam;
				break;
		        case SB_SETPARTS:
				lResult = ( LRESULT )Statbr_SetParts ( hWnd, pthis, 
					            ( int )wParam, ( LPINT )lParam );
				break;
			case SB_SETTEXT:
				lResult = ( LRESULT )Statbr_SetText ( hWnd, pthis, 
					            ( int )wParam, ( LPSTR )lParam );
				break;
			case SB_SIMPLE:
				lResult = ( LRESULT )Statbr_Simple ( hWnd, pthis, ( BOOL )wParam );
				break;
		}
	}

	if ( pthis )
		GlobalUnlock ( hStatusbar );
	return lResult;
}


int WINAPI 
Statbr_Initialize 
(	
	HINSTANCE		hinst
)
{

	WNDCLASS		wc;
	int			ErrorCode	= 0;

	memset ( &wc, 0, sizeof ( WNDCLASS ) );
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS | CS_DBLCLKS;
	wc.lpfnWndProc   = Statbr_WndProc;
	wc.cbWndExtra    = 0; 
	wc.hInstance     = hinst;
	wc.hCursor       = ( HCURSOR )NULL; /*LoadCursor( ( HINSTANCE )NULL, IDC_ARROW);*/
	wc.hbrBackground = GetStockObject ( LTGRAY_BRUSH );
	wc.lpszClassName = WSTATUSBARCLASSNAME;

	if ( ! RegisterClass ( &wc ) )
		ErrorCode = WSTB_ERR_REGISTERCLASS;

	return ( ErrorCode );
}


void WINAPI 
Statbr_Terminate 
(
	HINSTANCE			hInstance
) 
{
	WNDCLASS			WndClass;


	if ( GetClassInfo ( hInstance, WSTATUSBARCLASSNAME, &WndClass ) )
		UnregisterClass ( WSTATUSBARCLASSNAME, hInstance );

}


