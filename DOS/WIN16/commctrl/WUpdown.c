/*  
	WUpdown.c	1.5
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
#include <stdio.h>
#include <commctrl.h>
#else
#include "WCommCtrl.h"
#endif
#include "string.h"
#include "WLists.h"
#include "WUpdown.h"


#define	 UDBUTTONS_GAP		1
#define  SPINBTN_TIMERID	10
#define  SPINBTN_MAXCX		20
static char SPINBTN_PROP[] = "SpinBtn_Data";	
enum UdState { udsNull, udsIncrement, udsDecrement };
enum UdButton { udUpButton, udDownButton };
enum UdTimer { udNullTimer, udInitTimer, udOpTimer };

typedef struct {
	RECT	Rect;
	UINT	uStyles;
	int     nState;
	int	nBase;

	HWND	hwBuddy;
	BOOL	bEdtBorder;

	POINT   ptRange;
	int	Value;
	int	nTimer;
	UDACCEL udAccel;  /*current accelerator*/
	WLIST	AccelList; /*array of accelerators to be added by user*/
} SPINBTNDATA;

#define GETTHISDATA(hwnd) \
   GetProp(hwnd, SPINBTN_PROP )
#define SETTHISDATA(hwnd, pthisNew) \
   SetProp(hwnd, SPINBTN_PROP, pthisNew)
#define REMOVETHISDATA(hwnd) \
   RemoveProp(hwnd, SPINBTN_PROP )


/*prototypes*/
static void
SpinBtn_SetBuddyInt ( HWND hWnd, SPINBTNDATA* pthis );

static void
SpinBtn_IDrawBtn ( HDC hDC, SPINBTNDATA* pthis, int Btn, RECT rcBtn );

static void
SpinBtn_IGetBtnRect ( HWND hWnd, SPINBTNDATA* pthis, int Btn, RECT* prcBtn );

static BOOL
SpinBtn_IHitTest ( HWND hWnd, SPINBTNDATA* pthis, int x, int y, RECT* prcBtn );

static void
SpinBtn_ISetAutoBuddy ( HWND hWnd, SPINBTNDATA* pthis );

static void
SpinBtn_IDrawBorder ( HDC hDC, SPINBTNDATA* pthis, RECT rcBtn );

static LRESULT  
SpinBtn_UDMWndProc
(
	    HWND			hWnd,
        UINT			uMessage,
        WPARAM			wParam,
        LPARAM			lParam
);


static void
SpinBtn_GetBuddyInt 
(
	HWND				hWnd, 
	SPINBTNDATA*			pUpdown
)
{
	char				szBuddy[40];
	char				szText[40];
	char*				pszBuddy;
	char*				pszText;
	long				lBuddy;

	if ( !( pUpdown->uStyles & UDS_SETBUDDYINT ) )
		return;

	if ( !IsWindow ( pUpdown->hwBuddy ) )
		return;
	if ( GetWindowText (	pUpdown->hwBuddy, szBuddy, sizeof ( szBuddy ) ) )
	{
		pszBuddy = szBuddy;
		pszText = szText;
		do 
		{
			if ( *pszBuddy != ',' )
				*pszText++ = *pszBuddy;
		}
		while ( *pszBuddy++ );
		lBuddy = strtol ( szText, 0, pUpdown->nBase == 16 ? 16 : 10 );
		if ( lBuddy && 
		     ( ( pUpdown->ptRange.x < pUpdown->ptRange.y &&
		         lBuddy >= pUpdown->ptRange.x && 
			 lBuddy <= pUpdown->ptRange.y ) ||
		       ( pUpdown->ptRange.x > pUpdown->ptRange.y &&
		         lBuddy <= pUpdown->ptRange.x && 
			 lBuddy >= pUpdown->ptRange.y )
		      )
		   )
			 pUpdown->Value = ( int )lBuddy;
	}

}

static void
SpinBtn_Notify 
(
	HWND				hWnd, 
	SPINBTNDATA*			pUpdown
)
{
	
	NM_UPDOWN			nud;
	int				nDelta = 0;
	HWND				hwParent		= GetParent ( hWnd );
	
	if ( pUpdown->nState == udsDecrement )
	{
		if ( pUpdown->ptRange.x < pUpdown->ptRange.y &&
		     pUpdown->Value < pUpdown->ptRange.y
		   )
		   nDelta = 1;
		else
		if ( pUpdown->ptRange.x > pUpdown->ptRange.y &&
		     pUpdown->Value > pUpdown->ptRange.y
		   )
		   nDelta = -1;
		else
		if ( ( pUpdown->Value == pUpdown->ptRange.y ) &&
		     ( pUpdown->uStyles & UDS_WRAP )		     
		   )
		   nDelta = pUpdown->ptRange.y > pUpdown->ptRange.x ?
			    pUpdown->ptRange.x - pUpdown->ptRange.y :
			    pUpdown->ptRange.y - pUpdown->ptRange.x;
	}
	else
	if ( pUpdown->nState == udsIncrement )
	{
		if ( pUpdown->ptRange.x < pUpdown->ptRange.y &&
		     pUpdown->Value > pUpdown->ptRange.x
		   )
		   nDelta = -1;
		else
		if ( pUpdown->ptRange.x > pUpdown->ptRange.y &&
		     pUpdown->Value < pUpdown->ptRange.x
		   )
		   nDelta = 1;
		else
		if ( ( pUpdown->Value == pUpdown->ptRange.x ) &&
		     ( pUpdown->uStyles & UDS_WRAP )		     
		   )
		   nDelta = pUpdown->ptRange.y > pUpdown->ptRange.x ?
			    pUpdown->ptRange.y - pUpdown->ptRange.x :
			    pUpdown->ptRange.x - pUpdown->ptRange.y;
	}

	memset ( &nud, 0, sizeof ( NM_UPDOWN ) );
	nud.hdr.hwndFrom = hWnd;
	nud.hdr.idFrom = GetDlgCtrlID ( hWnd );
	nud.hdr.code = UDN_DELTAPOS;
	nud.iPos = pUpdown->Value;
	nud.iDelta = nDelta;

	if ( hwParent )
		if ( SendMessage ( hwParent, WM_NOTIFY, 0, ( LPARAM )&nud ) )
			return;
	pUpdown->Value += nud.iDelta;
	if ( IsWindow ( pUpdown->hwBuddy ) )
	{
		if ( pUpdown->uStyles & UDS_SETBUDDYINT )
			SpinBtn_SetBuddyInt ( hWnd, pUpdown );
		else
		{
			int nNotify = pUpdown->nState == udsDecrement ? SB_LINEDOWN : SB_LINEUP;
			UINT uMessage = pUpdown->uStyles & UDS_HORZ ? WM_HSCROLL : WM_VSCROLL;
			SendMessage ( pUpdown->hwBuddy, uMessage, MAKELONG ( nNotify, pUpdown->Value ),
						( LPARAM )hWnd );
				
		}
	}

}

static BOOL
SpinBtn_OnCreate 
(
	HWND				hWnd, 
	CREATESTRUCT*			pcs
) 
{
	HGLOBAL				hSpinBtn	= ( HGLOBAL )NULL;
	SPINBTNDATA*			pthis		= ( SPINBTNDATA* )NULL;

	int				ErrorCode	= 0;

	if ( ! ( hSpinBtn = GlobalAlloc ( GHND, sizeof ( SPINBTNDATA ) ) ) )
		ErrorCode = WUD_ERR_GLOBALALLOC;
	else
	if ( ! ( pthis = ( SPINBTNDATA *) GlobalLock ( hSpinBtn ) ) )
		ErrorCode = WUD_ERR_GLOBALLOCK;
	else
	{
		SETTHISDATA(hWnd, hSpinBtn);
		pthis->uStyles = pcs->style;
		pthis->ptRange.x = 100;
		pthis->ptRange.y = 0;
		pthis->Value = 0;
		pthis->nBase = 10;
		pthis->nState = udsNull;
		pthis->nTimer = udNullTimer;
		pthis->udAccel.nSec = 1;
		pthis->udAccel.nInc = 1;
		WLCreate ( &pthis->AccelList, sizeof ( UDACCEL ) );
#if 1
		SpinBtn_ISetAutoBuddy ( hWnd, pthis );
#else
		if ( !pthis->hwBuddy || !IsWindow ( pthis->hwBuddy ) )
		{
			if ( pthis->uStyles & UDS_AUTOBUDDY )
			        pthis->hwBuddy = GetWindow ( hWnd, GW_HWNDPREV );

		}

		if ( pthis->hwBuddy && IsWindow ( pthis->hwBuddy ) )
		{
			GetWindowRect ( pthis->hwBuddy, &rcBuddy );
		        MapWindowPoints ( HWND_DESKTOP, GetParent ( pthis->hwBuddy ), 
						 ( LPPOINT )&rcBuddy.left, 2 );
			if ( pthis->uStyles & UDS_ALIGNRIGHT ) {
				SetWindowPos( hWnd, ( HWND )NULL, rcBuddy.right+1, rcBuddy.top, 0, 0,
					     SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
			}
			else 
			if ( pthis->uStyles & UDS_ALIGNLEFT) 
			{
				GetWindowRect( hWnd, &rcUpdown );
				SetWindowPos( hWnd, ( HWND )NULL, rcBuddy.left -
					( rcUpdown.right - rcUpdown.left ) - 1, 
					rcBuddy.top, 0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
			}
		}		
#endif

	}

	if ( pthis )
		GlobalUnlock ( hSpinBtn );

	return ( ErrorCode >= 0 );
}

static void SpinBtn_OnKeyDown 
(
	HWND				hWnd, 
	int				VKey, 
	int				cRepeat, 
	UINT				keyFlags
) 
{
	HGLOBAL				hSpinBtn	= ( HGLOBAL )NULL;
	SPINBTNDATA*			pthis		= ( SPINBTNDATA* )NULL;

	int				ErrorCode	= 0;
	
	if ( ! ( hSpinBtn = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WUD_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( SPINBTNDATA *) GlobalLock ( hSpinBtn ) ) )
		ErrorCode = WUD_ERR_GLOBALLOCK;
	else
	if ( pthis->uStyles & UDS_ARROWKEYS )
	{
		switch ( VKey )
		{
			case VK_UP:
			case VK_LEFT:
				pthis->nState = udsDecrement;
				break;
			case VK_DOWN:
			case VK_RIGHT:
				pthis->nState = udsIncrement;
				break;
		}

		if ( !( pthis->nState == udsNull ) )
		{
			SpinBtn_Notify ( hWnd, pthis );
			InvalidateRect ( hWnd, NULL, TRUE );
			UpdateWindow ( hWnd );
		}

	}

	if ( pthis )
		GlobalUnlock ( hSpinBtn );
}


static void SpinBtn_OnKeyUp (HWND hWnd, int VKey, int cRepeat, UINT keyFlags) 
{
	HGLOBAL				hSpinBtn	= ( HGLOBAL )NULL;
	SPINBTNDATA*			pthis		= ( SPINBTNDATA* )NULL;

	int				ErrorCode	= 0;
	
	if ( ! ( hSpinBtn = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WUD_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( SPINBTNDATA *) GlobalLock ( hSpinBtn ) ) )
		ErrorCode = WUD_ERR_GLOBALLOCK;
	else
	if ( pthis->uStyles & UDS_ARROWKEYS )
	{
		switch ( VKey )
		{
			case VK_UP:
			case VK_RIGHT:
				pthis->nState = udsIncrement;
				break;
			case VK_DOWN:
			case VK_LEFT:
				pthis->nState = udsDecrement;
				break;
		}

		if ( !( pthis->nState == udsNull ) )
		{
			pthis->nState = udsNull;
			InvalidateRect ( hWnd, NULL, TRUE );
			UpdateWindow ( hWnd );
		}

	}

	if ( pthis )
		GlobalUnlock ( hSpinBtn );
}


static void SpinBtn_OnLButtonDown 
(	
	HWND				hWnd, 
	int				x, 
	int				y, 
	UINT				keyFlags
) 
{
	RECT				rcBtn;
	HGLOBAL				hSpinBtn	= ( HGLOBAL )NULL;
	SPINBTNDATA*			pthis		= ( SPINBTNDATA* )NULL;
	int				ErrorCode	= 0;
	
	if ( ! ( hSpinBtn = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WUD_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( SPINBTNDATA *) GlobalLock ( hSpinBtn ) ) )
		ErrorCode = WUD_ERR_GLOBALLOCK;
	else
	{
		if ( SpinBtn_IHitTest ( hWnd, pthis, x, y, &rcBtn ) )
		{
			SetFocus ( hWnd );
			SetCapture ( hWnd );
			InvalidateRect ( hWnd, NULL, TRUE );
			UpdateWindow ( hWnd );
			SpinBtn_GetBuddyInt ( hWnd, pthis );
			SpinBtn_Notify ( hWnd, pthis );
			if ( pthis->nTimer == udNullTimer )
			{
				pthis->nTimer = udInitTimer;
				SetTimer ( hWnd, SPINBTN_TIMERID, 500, NULL );
			}
		}
	}

	if ( pthis )
		GlobalUnlock ( hSpinBtn );
}

static void SpinBtn_OnLButtonUp 
(	
	HWND				hWnd, 
	int				x, 
	int				y, 
	UINT				keyFlags
) 
{
	HGLOBAL				hSpinBtn	= ( HGLOBAL )NULL;
	SPINBTNDATA*			pthis		= ( SPINBTNDATA* )NULL;

	int				ErrorCode	= 0;
	
	if ( ! ( hSpinBtn = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WUD_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( SPINBTNDATA *) GlobalLock ( hSpinBtn ) ) )
		ErrorCode = WUD_ERR_GLOBALLOCK;
	else
	{
		ReleaseCapture ( );
		pthis->nState = udsNull;
		if ( pthis->nTimer != udNullTimer )
		{
			KillTimer ( hWnd, SPINBTN_TIMERID ); 
			pthis->nTimer = udNullTimer;
		}
		InvalidateRect ( hWnd, NULL, TRUE );
		UpdateWindow ( hWnd );
	}

	if ( pthis )
		GlobalUnlock ( hSpinBtn );
}

static void SpinBtn_OnDestroy ( HWND hWnd )
{
	HGLOBAL		hSpinBtn	= ( HGLOBAL )NULL;

	if ( ( hSpinBtn = GETTHISDATA ( hWnd ) ) )
	{
		REMOVETHISDATA ( hWnd );
		GlobalFree ( hSpinBtn );
	}
}

static void SpinBtn_OnMouseMove 
(
	HWND			hWnd, 
	int			x, 
	int			y, 
	UINT			keyFlags
) 
{
}


static void SpinBtn_OnPaint 
(
	HWND				hWnd, 
	HDC				hPaintDC 
)
{
	PAINTSTRUCT ps;
	RECT				rcBtn;
	HGLOBAL				hSpinBtn	= ( HGLOBAL )NULL;
	SPINBTNDATA*			pthis		= ( SPINBTNDATA* )NULL;
	
	int				ErrorCode	= 0;

	if ( ! ( hSpinBtn = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WUD_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( SPINBTNDATA *) GlobalLock ( hSpinBtn ) ) )
		ErrorCode = WUD_ERR_GLOBALLOCK;
	else
	{	
	
		BeginPaint ( hWnd, &ps );
		SpinBtn_IGetBtnRect ( hWnd, pthis, udUpButton, &rcBtn ); 
		SpinBtn_IDrawBtn ( ps.hdc, pthis, udUpButton, rcBtn );
		SpinBtn_IGetBtnRect ( hWnd, pthis, udDownButton, &rcBtn ); 
		SpinBtn_IDrawBtn ( ps.hdc, pthis, udDownButton, rcBtn );
		if ( pthis->bEdtBorder )
		{
			GetClientRect ( hWnd, &rcBtn );
			SpinBtn_IDrawBorder ( ps.hdc, pthis, rcBtn );
		}
		EndPaint ( hWnd, &ps );
	}

	if ( pthis )
		GlobalUnlock ( hSpinBtn );
}

static void
SpinBtn_SetBuddyInt 
(
	HWND			hWnd,
	SPINBTNDATA*		pthis
)
{
	char			buf[40];
	char			szBuddy[40];
	char*			pbuf		= buf;
	char*			pOut		= buf;
	char*			pszBuddy	= szBuddy;
		
	if ( !( pthis->uStyles & UDS_SETBUDDYINT ) )
		return;
	
	if ( pthis->hwBuddy )
	{
		if ( pthis->nBase == 16 )
			sprintf ( buf, "%X", pthis->Value );
		else
			sprintf ( buf, "%d", pthis->Value );
		if ( !( pthis->uStyles & UDS_NOTHOUSANDS ) ) 
		{
			int len = strlen ( buf );
			if (len > 3) 
			{
				int i = len % 3;
				int j;
				for ( j = 0; j < len; j++ ) 
				{
					if (j % 3 == i)
						*pszBuddy++ = ',';
					*pszBuddy++ = *pbuf++;
				}

				*pszBuddy = *pbuf;

				pOut = szBuddy;

			}
		
			SetWindowText ( pthis->hwBuddy, pOut);
		}
	}
}


static void SpinBtn_OnTimer 
(
	HWND				hWnd, 
	UINT				uTimerID 
)
{
	HGLOBAL				hSpinBtn	= ( HGLOBAL )NULL;
	SPINBTNDATA*			pthis		= ( SPINBTNDATA* )NULL;
	
	int				ErrorCode	= 0;

	if ( uTimerID != SPINBTN_TIMERID )
		return;
	if ( ! ( hSpinBtn = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WUD_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( SPINBTNDATA *) GlobalLock ( hSpinBtn ) ) )
		ErrorCode = WUD_ERR_GLOBALLOCK;
	else
	{
		if ( pthis->nTimer == udOpTimer )
			SpinBtn_Notify ( hWnd, pthis );
		else
		{
			pthis->nTimer = udOpTimer;
			SetTimer ( hWnd, SPINBTN_TIMERID, 100, NULL );
		}
	}
}


static void
SpinBtn_IDrawBorder
(
	HDC			hDC,
	SPINBTNDATA*		pthis,
	RECT			rcBtn
)
{
	HPEN			hBpen  =	CreatePen ( PS_SOLID, 3, RGB (255,255,255) );
	HPEN			hTpen  =	CreatePen ( PS_SOLID, 2, RGB (0,0,0) );
	
	rcBtn.top += 1;
	if ( pthis->uStyles & UDS_ALIGNRIGHT )
	{
	hBpen = SelectObject ( hDC, hBpen );
	MoveToEx ( hDC, rcBtn.left, rcBtn.bottom, NULL );
	LineTo ( hDC, rcBtn.right, rcBtn.bottom );
	LineTo ( hDC, rcBtn.right, rcBtn.top );
	hTpen = SelectObject ( hDC, hTpen );
	LineTo ( hDC, rcBtn.left-1, rcBtn.top );
	hBpen = SelectObject ( hDC, hBpen );
	}
	if ( pthis->uStyles & UDS_ALIGNLEFT )
	{
	hBpen = SelectObject ( hDC, hBpen );
	MoveToEx ( hDC, rcBtn.right, rcBtn.bottom, NULL );
	LineTo ( hDC, rcBtn.left, rcBtn.bottom );
	hTpen = SelectObject ( hDC, hTpen );
	LineTo ( hDC, rcBtn.left, rcBtn.top );
	LineTo ( hDC, rcBtn.right+1, rcBtn.top );
	hBpen = SelectObject ( hDC, hBpen );
	}

	DeleteObject ( hBpen );
	DeleteObject ( hTpen );
	
}


static void
SpinBtn_IDrawBtnBorder
(
	HDC		    	hDC,
	SPINBTNDATA*	pthis,
	int     		Btn,
	RECT			rcBtn
)
{
	HPEN			hTpen  =	GetStockObject ( WHITE_PEN );
	HPEN			hBpen  =	GetStockObject ( BLACK_PEN );
	
	if (  ( ( pthis->nState & udsDecrement ) && ( Btn == udUpButton ) ) ||
	      ( ( pthis->nState & udsIncrement ) && ( Btn == udDownButton ) ) 
	   )
	{
		hTpen = GetStockObject ( BLACK_PEN );
		hBpen = GetStockObject ( WHITE_PEN );

	}
	rcBtn.left += 1;
	rcBtn.top += 1;
	hBpen = SelectObject ( hDC, hBpen );
	MoveToEx ( hDC, rcBtn.left, rcBtn.bottom, NULL );
	LineTo ( hDC, rcBtn.right, rcBtn.bottom );
	LineTo ( hDC, rcBtn.right, rcBtn.top );
	hTpen = SelectObject ( hDC, hTpen );
	LineTo ( hDC, rcBtn.left, rcBtn.top );
	LineTo ( hDC, rcBtn.left, rcBtn.bottom );
	hBpen = SelectObject ( hDC, hBpen );
	
}

static void
SpinBtn_IDrawBtn
(
	HDC		    	hDC,
	SPINBTNDATA*	pthis,
	int     		Btn,
	RECT			rcBtn
)
{
	int			    Offset;
	POINT			ptStart;
	POINT			ptEnd;
	int			    height;
	int			    width;

	
	SpinBtn_IDrawBtnBorder ( hDC, pthis, Btn, rcBtn );
	height = rcBtn.bottom - rcBtn.top;
	width = rcBtn.right - rcBtn.left;

	if ( pthis->uStyles & UDS_HORZ )
	{
		Offset = Btn == udUpButton ? width * 2 / 3 : width / 3;
		ptEnd.x = ptStart.x = rcBtn.left + Offset;
		ptStart.y = rcBtn.top + ( height + 1 ) / 4;
		ptEnd.y = ptStart.y + height / 2;
		while ( ptStart.y < ptEnd.y )
		{
			MoveToEx ( hDC, ptStart.x, ptStart.y, NULL );
			LineTo ( hDC, ptEnd.x, ptEnd.y );
			ptStart.y += 1;
			ptEnd.y -= 1;
			ptStart.x += Btn == udUpButton ? -1 : 1;
			ptEnd.x = ptStart.x;
		}
	}
	else
	{
		Offset = Btn == udUpButton ? height * 2 / 3 : ( height ) / 3 ;
		ptEnd.y = ptStart.y = rcBtn.top + Offset;
		ptStart.x = rcBtn.left + ( width + 1 ) / 4;
		ptEnd.x = ptStart.x + width / 2;
		while ( ptStart.x < ptEnd.x )
		{
			MoveToEx ( hDC, ptStart.x, ptStart.y, NULL );
			LineTo ( hDC, ptEnd.x, ptEnd.y );
			ptStart.y += Btn == udUpButton ? -1 : 1;
			ptEnd.y = ptStart.y;
			ptStart.x++;
			ptEnd.x--;
		}
	}
}

static void
SpinBtn_IGetBtnRect
(
	HWND			hWnd,
	SPINBTNDATA*	pthis,
	int     		Btn,
	RECT*			prcBtn
)
{

#if 1
	GetClientRect ( hWnd, prcBtn );
	prcBtn->right--;
	prcBtn->bottom--;
#else
	prcBtn->left = pthis->Rect.left;
	prcBtn->top = pthis->Rect.top;
	prcBtn->bottom = pthis->Rect.bottom;
	prcBtn->right = pthis->Rect.right;
	
#endif
	if ( pthis->uStyles & UDS_HORZ )
	{
		if ( Btn == udUpButton )
			prcBtn->right =	( prcBtn->right - prcBtn->left ) / 2;
		else
			prcBtn->left =	( prcBtn->right - prcBtn->left ) / 2;
	}
	else
	{
		if ( Btn == udUpButton )
			prcBtn->bottom = ( prcBtn->bottom - prcBtn->top ) / 2 +1;
		else
			prcBtn->top +=	( prcBtn->bottom - prcBtn->top ) / 2;
	}
}

static BOOL
SpinBtn_IHitTest
(
	HWND				hWnd, 
	SPINBTNDATA*			pthis,
	int				x, 
	int				y,
	RECT*				prcBtn
)
{
	

	pthis->nState = udsNull;
	SpinBtn_IGetBtnRect ( hWnd, pthis, udUpButton, prcBtn );
	if ( x > prcBtn->left && x < prcBtn->right &&
	     y > prcBtn->top && y < prcBtn->bottom
	   )
		pthis->nState = udsDecrement;

	SpinBtn_IGetBtnRect ( hWnd, pthis, udDownButton, prcBtn );
	if ( x > prcBtn->left && x < prcBtn->right &&
	     y > prcBtn->top && y < prcBtn->bottom
	   )
		pthis->nState = udsIncrement;

	return ( pthis->nState != udsNull );
}

static void
SpinBtn_ISetAutoBuddy 
(	HWND				hWnd, 
	SPINBTNDATA*			pthis 
)
{
	RECT				rcBuddy;
	RECT				rcUpdown;
	char				szClassName[64];

	if ( !pthis->hwBuddy || !IsWindow ( pthis->hwBuddy ) )
	{
		if ( pthis->uStyles & UDS_AUTOBUDDY )
			pthis->hwBuddy = GetWindow ( hWnd, GW_HWNDPREV );

	}

	if ( pthis->hwBuddy && IsWindow ( pthis->hwBuddy ) )
	{
		GetWindowRect ( pthis->hwBuddy, &rcBuddy );
		MapWindowPoints ( HWND_DESKTOP, GetParent ( pthis->hwBuddy ), 
					 ( LPPOINT )&rcBuddy.left, 2 );
		GetClassName ( pthis->hwBuddy, szClassName, sizeof ( szClassName ) );
		if ( pthis->uStyles & UDS_ALIGNRIGHT ) 
		{
			if ( lstrcmp ( szClassName, "Edit" ) == 0 )
			{
				rcBuddy.right -= SPINBTN_MAXCX;
				SetWindowPos ( pthis->hwBuddy, ( HWND )NULL, rcBuddy.left, rcBuddy.top, 
					       rcBuddy.right - rcBuddy.left, 
					       rcBuddy.bottom - rcBuddy.top,
					       SWP_NOACTIVATE | SWP_NOZORDER );
				pthis->bEdtBorder = TRUE;
			}
			SetWindowPos( hWnd, ( HWND )NULL, 
				rcBuddy.right-1, rcBuddy.top, 
				SPINBTN_MAXCX, rcBuddy.bottom - rcBuddy.top,
				SWP_NOACTIVATE | SWP_NOZORDER );
		}
		else 
		if ( pthis->uStyles & UDS_ALIGNLEFT) 
		{
			GetWindowRect( hWnd, &rcUpdown );
			if ( lstrcmp ( szClassName, "Edit" ) == 0 )
			{
				rcBuddy.left += SPINBTN_MAXCX;
				SetWindowPos ( pthis->hwBuddy, ( HWND )NULL, rcBuddy.left, rcBuddy.top, 
					       rcBuddy.right - rcBuddy.left, 
					       rcBuddy.bottom - rcBuddy.top,
					       SWP_NOACTIVATE | SWP_NOZORDER );
				pthis->bEdtBorder = TRUE;
			}
#if 1
			SetWindowPos( hWnd, ( HWND )NULL, rcBuddy.left - SPINBTN_MAXCX, 
				rcBuddy.top, SPINBTN_MAXCX, ( rcBuddy.bottom - rcBuddy.top ),
				SWP_NOACTIVATE | SWP_NOZORDER);
#else
			SetWindowPos( hWnd, ( HWND )NULL, rcBuddy.left - 1, rcBuddy.top, 
				SPINBTN_MAXCX, ( rcUpdown.bottom - rcUpdown.top ),
				SWP_NOACTIVATE | SWP_NOZORDER);
#endif
		}
	}	

	GetClientRect ( hWnd, &pthis->Rect );
	if ( pthis->bEdtBorder )
	{
		pthis->Rect.top +=2;
		pthis->Rect.bottom -=3;
		if ( pthis->uStyles & UDS_ALIGNRIGHT )
		{
			pthis->Rect.right -=3;
		}
		if ( pthis->uStyles & UDS_ALIGNLEFT )
		{
			pthis->Rect.left += 2;
		}
	}
	else
	{
		pthis->Rect.right -= 1;
		pthis->Rect.bottom -= 1;
	}
}

static LRESULT WINAPI 
SpinBtn_WndProc
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
			if ( ! SpinBtn_OnCreate ( hWnd, ( LPCREATESTRUCT )lParam ) )
				return ( LRESULT ) -1L;
			break;
		case WM_DESTROY:
			SpinBtn_OnDestroy ( hWnd );
			break;
		case WM_ENABLE:
			break;
		case WM_GETDLGCODE:
			return ( LRESULT )DLGC_WANTARROWS;
		
		case WM_KEYDOWN:
			SpinBtn_OnKeyDown ( hWnd, (UINT)( wParam ), ( int )( short )LOWORD ( lParam ), 
				(UINT)HIWORD ( lParam ) );
			break;
		
		case WM_KEYUP:
			SpinBtn_OnKeyUp ( hWnd, (UINT)( wParam ), ( int )( short )LOWORD ( lParam ), 
				(UINT)HIWORD ( lParam ) );
			break;

		case WM_LBUTTONDOWN:
			SpinBtn_OnLButtonDown ( hWnd,(int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )( wParam ) );
			break;

		case WM_LBUTTONUP:
			SpinBtn_OnLButtonUp ( hWnd,(int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )( wParam ) );
			break;

		case WM_MOUSEMOVE:
			SpinBtn_OnMouseMove ( hWnd,(int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )( wParam ) );
			break;

		case WM_PAINT:
			SpinBtn_OnPaint( hWnd, ( HDC ) wParam  );
			break;

		case WM_TIMER:
			SpinBtn_OnTimer ( hWnd, ( UINT )wParam );
		case UDM_GETBUDDY:
		case UDM_SETBUDDY:
		case UDM_GETACCEL:
		case UDM_GETBASE:
		case UDM_GETPOS:
		case UDM_GETRANGE:
		case UDM_SETACCEL:
		case UDM_SETBASE:
		case UDM_SETPOS:	 
		case UDM_SETRANGE:
			return SpinBtn_UDMWndProc(hWnd, uMessage, wParam, lParam);
		default:
			return DefWindowProc(hWnd, uMessage, wParam, lParam);
	}
    
	    return 0;
}

static LRESULT  
SpinBtn_UDMWndProc
(
	    HWND			hWnd,
        UINT			uMessage,
        WPARAM			wParam,
        LPARAM			lParam
)
{
	HGLOBAL				hSpinBtn	= ( HGLOBAL )NULL;
	SPINBTNDATA*		pthis		= ( SPINBTNDATA* )NULL;
	int				    nTemp		= 0;
	int				    nPrev		= 0;
	LRESULT				lResult		= 0L;
	int			    	ErrorCode	= 0;

	if ( ! ( hSpinBtn = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WUD_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( SPINBTNDATA *) GlobalLock ( hSpinBtn ) ) )
		ErrorCode = WUD_ERR_GLOBALLOCK;
	else
	{
		switch ( uMessage )
		{
			case UDM_GETBUDDY:
				lResult = ( LRESULT )pthis->hwBuddy;
				break;
			case UDM_SETBUDDY:
				lResult = ( LRESULT )pthis->hwBuddy;
				pthis->hwBuddy = ( HWND )wParam;
				break;
			case UDM_GETACCEL:
				break;
			case UDM_GETBASE:
				lResult = ( LRESULT )pthis->nBase;
				break;
			case UDM_GETPOS:
				lResult = ( LRESULT )pthis->Value;
				break;
			case UDM_GETRANGE:
				lResult = ( LRESULT )MAKELONG ( pthis->ptRange.y, pthis->ptRange.x );
				break;
			case UDM_SETACCEL:
				/*return Updown_SetAccel (*/
			case UDM_SETBASE:
				if ( wParam == 10 || wParam == 16 )
				{
					lResult = ( LRESULT )pthis->nBase;
					pthis->nBase = ( int )wParam;
					SpinBtn_Notify ( hWnd, pthis );
				}
				break;
			case UDM_SETPOS:
				nTemp = LOWORD ( lParam );
				if ( ( nTemp <= pthis->ptRange.y && nTemp >= pthis->ptRange.x ) ||
				     ( nTemp >= pthis->ptRange.y && nTemp <= pthis->ptRange.x )
				   )
				{
					lResult = ( LRESULT )pthis->Value;
					pthis->Value = nTemp;
					SpinBtn_Notify ( hWnd, pthis );
				}
				break;
			case UDM_SETRANGE:
				nTemp = LOWORD ( lParam );
				nPrev = HIWORD ( lParam );
				if ( ( nTemp <= UD_MAXVAL && nTemp >= UD_MINVAL ) &&
				     ( nPrev <=	UD_MAXVAL && nPrev >= UD_MINVAL ) &&
				     ( abs ( nTemp - nPrev )   <= UD_MAXVAL )
				   )
				{
					pthis->ptRange.x = nPrev;
					pthis->ptRange.y = nTemp;
				}
		}
	}

	if ( pthis )
		GlobalUnlock ( hSpinBtn );
	return lResult;
}


int WINAPI 
SpinBtn_Initialize 
(
	HINSTANCE			hinst
) 
{

	WNDCLASS			wc;
	int				ErrorCode = 0;

	memset ( &wc, 0, sizeof ( WNDCLASS ) );
	#ifdef _WIN32
/*	wc.cbSize 	 = sizeof ( WNDCLASSEX );	*/
	#endif
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
	wc.lpfnWndProc   = SpinBtn_WndProc;
	wc.cbWndExtra    = 0; /*sizeof( Updown_WNDEXTRABYTES );*/
	wc.hInstance     = hinst;
	wc.hCursor       = LoadCursor( ( HINSTANCE )NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject ( LTGRAY_BRUSH );
	wc.lpszClassName = WC_UPDOWN;

	if ( ! RegisterClass ( &wc ) )
		ErrorCode = WUD_ERR_REGISTERCLASS;

	return ( ErrorCode );
}


void WINAPI 
SpinBtn_Terminate 
(
	HINSTANCE			hInstance
) 
{
	WNDCLASS			WndClass;


	if ( GetClassInfo ( hInstance, WC_UPDOWN, &WndClass ) )
		UnregisterClass ( WC_UPDOWN, hInstance );
}


