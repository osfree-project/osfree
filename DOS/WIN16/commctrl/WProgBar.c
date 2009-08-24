/*  
	WProgBar.c	
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
#include "math.h"
#include "string.h"
#include "WProgBar.h"

#ifndef _WIN32
#define	 MoveToEx(hdc,x,y,old)	MoveTo(hdc,x,y)
#define  RegisterClassEx	RegisterClass
#define  WNDCLASSEX		WNDCLASS
#endif

#define DEF_STEPSIZE	10
#define MIN_STEPSIZE	3

static char PROGBAR_PROP[] = "Progressbar_Data";	
typedef struct {
	UINT	uStyles;
	UINT	maxRange;
	UINT	minRange;
	UINT	curPos;
	UINT	iStep;
	UINT    iVisStep;
	UINT	iVisStepWidth;
	UINT	curVisPos;
	UINT	totalSteps;

} PROGBARDATA;

#define GETTHISDATA(hwnd) \
   GetProp(hwnd, PROGBAR_PROP )
#define SETTHISDATA(hwnd, pthisNew) \
   SetProp(hwnd, PROGBAR_PROP, pthisNew)
#define REMOVETHISDATA(hwnd) \
   RemoveProp(hwnd, PROGBAR_PROP )


/*prototypes*/
int ProgBar_OnEraseBkground ( HWND hWnd, HDC hDC );
LRESULT ProgBar_TBMWndProc(HWND hWnd,UINT uMessage,WPARAM wParam,LPARAM lParam);
static int ProgBar_IStepIt ( HWND hWnd, PROGBARDATA* pthis );
static void ProgBar_IDrawBorder ( HWND hWnd, HDC hDC );
static void ProgBar_IDrawTilPos ( HWND hWnd, PROGBARDATA* pthis, int iPos );


BOOL 
ProgBar_OnCreate 
(
	HWND				hWnd, 
	CREATESTRUCT*		pcs
) 
{

	RECT				rcClient;
	HWND				hwndParent	= GetParent(hWnd);
	HDC					hDC			= GetDC	( hwndParent );
	HGLOBAL				hProgBar	= ( HGLOBAL )NULL;
	PROGBARDATA*		pthis		= ( PROGBARDATA* )NULL;

	int				ErrorCode	= 0;

	if ( ! ( hProgBar = GlobalAlloc ( GHND, sizeof ( PROGBARDATA ) ) ) )
		ErrorCode = WPB_ERR_GLOBALALLOC;
	else
	if ( ! ( pthis = ( PROGBARDATA *) GlobalLock ( hProgBar ) ) )
		ErrorCode = WPB_ERR_GLOBALLOCK;
	else
	{
		SETTHISDATA(hWnd, hProgBar);
		pthis->uStyles = pcs->style;
		pthis->iStep = 10;
		pthis->maxRange = 100;
		pthis->minRange = 0;
		pthis->curPos = 0;
		GetClientRect ( hWnd, &rcClient );

	}

	ReleaseDC ( hwndParent, hDC );
	if ( pthis )
		GlobalUnlock ( hProgBar );
	return ( ErrorCode >= 0 );
}


/*Need revisit this*/
void ProgBar_OnDestroy ( HWND hWnd )
{
	HGLOBAL				hProgBar	= ( HGLOBAL )NULL;
	PROGBARDATA*		pthis		= ( PROGBARDATA* )NULL;

	int				ErrorCode	= 0;

	if ( ! ( hProgBar = GlobalAlloc ( GHND, sizeof ( PROGBARDATA ) ) ) )
		ErrorCode = WPB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( PROGBARDATA *) GlobalLock ( hProgBar ) ) )
		ErrorCode = WPB_ERR_GLOBALLOCK;
	else
	{
		REMOVETHISDATA ( hWnd );
		GlobalFree ( hProgBar );
	}
}

int ProgBar_OnEraseBkground 
(
		HWND				hWnd, 
		HDC				hDC 
)
{
		RECT				rect;
		HPEN				hPen    = GetStockObject ( NULL_PEN );
		HBRUSH				hBrush	= GetStockObject ( LTGRAY_BRUSH );
	if ( hDC )
	{
		hPen = SelectObject ( hDC, hPen );
		hBrush = SelectObject ( hDC, hBrush );
		GetClientRect ( hWnd, &rect );
		Rectangle ( hDC, rect.left, rect.top,
					   rect.right, rect.bottom );
		hPen = SelectObject ( hDC, hPen );
		hBrush = SelectObject ( hDC, hBrush );
		return 1;
	}

	return 0;

}


static void ProgBar_OnPaint 
(
		HWND				hWnd, 
		HDC				hPaintDC 
)
{
	PAINTSTRUCT		ps;
	HGLOBAL			hProgBar	= ( HGLOBAL )NULL;
	PROGBARDATA*	pthis		= ( PROGBARDATA* )NULL;
	
	int			ErrorCode	= 0;

	if ( ! ( hProgBar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WPB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( PROGBARDATA *) GlobalLock ( hProgBar ) ) )
		ErrorCode = WPB_ERR_GLOBALLOCK;
	else
	{
		/* Start painting.*/
		BeginPaint(hWnd, &ps);
		ProgBar_IDrawTilPos ( hWnd, pthis, pthis->curPos );
		ProgBar_IDrawBorder ( hWnd, ps.hdc ); 
		EndPaint(hWnd, &ps);
	}
	if ( pthis )
		GlobalUnlock ( hProgBar );
}

LRESULT WINAPI 
ProgBar_WndProc
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
			if ( ! ProgBar_OnCreate ( hWnd, ( LPCREATESTRUCT )lParam ) )
				return ( LRESULT ) -1L;
			break;
		case WM_DESTROY:
			ProgBar_OnDestroy ( hWnd );
			break;
		case WM_ERASEBKGND:
			return ( LRESULT )ProgBar_OnEraseBkground ( hWnd, ( HDC )wParam );
			break;
		
		case WM_PAINT:
			ProgBar_OnPaint( hWnd, ( HDC ) wParam  );
			break;

		default:
		    if ( uMessage >= TBM_GETPOS && uMessage <= TBM_GETTHUMBLENGTH )
			    return ProgBar_TBMWndProc(hWnd, uMessage, wParam, lParam);
		    return DefWindowProc(hWnd, uMessage, wParam, lParam);
	}
    
	    return 0;
}

int ProgBar_ISetRange ( HWND , PROGBARDATA* , int , int );

LRESULT  
ProgBar_TBMWndProc
(
	HWND			hWnd,
    	UINT			uMessage,
    	WPARAM			wParam,
    	LPARAM			lParam
)
{
	HGLOBAL			hProgBar	= ( HGLOBAL )NULL;
	PROGBARDATA*		pthis		= ( PROGBARDATA* )NULL;
	
	int				ErrorCode	= 0;

	if ( ! ( hProgBar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WPB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( PROGBARDATA *) GlobalLock ( hProgBar ) ) )
		ErrorCode = WPB_ERR_GLOBALLOCK;
	else
	{
		switch (uMessage)
		{
			case PBM_DELTAPOS:
				{
					int OldPos = pthis->curPos;
					pthis->curPos += ( int )wParam;
					InvalidateRect ( hWnd, ( HWND )NULL, TRUE );
					return ( LRESULT )OldPos;
				}
				
			case PBM_SETPOS:
				{
					int OldPos = pthis->curPos;
					pthis->curPos = ( int )wParam;
					InvalidateRect ( hWnd, ( HWND )NULL, TRUE );
					return ( LRESULT )OldPos;
				}

			case PBM_SETRANGE:
				return ( LRESULT )ProgBar_ISetRange ( hWnd, pthis, LOWORD (lParam ), HIWORD (lParam ) );

			case PBM_SETSTEP:
				{
					UINT OldStep = pthis->iStep;
					pthis->iStep = wParam;
					return ( LRESULT )OldStep;
				}
			
			case PBM_STEPIT:
				ProgBar_IStepIt ( hWnd, pthis );
				break;

			default:
			    return DefWindowProc(hWnd, uMessage, wParam, lParam);
		}
	}

	if ( pthis )
		GlobalUnlock ( hProgBar );

	if ( pthis )
		GlobalUnlock ( hProgBar );
	return 0;
}


int WINAPI 
ProgBar_Initialize 
(
	HINSTANCE		hinst
) 
{

	WNDCLASS		wc;
	int			ErrorCode	= 0;

	memset ( &wc, 0, sizeof ( WNDCLASS ) );
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
	wc.lpfnWndProc   = ProgBar_WndProc;
	wc.cbWndExtra    = 0; /*sizeof( TRACKBAR_WNDEXTRABYTES );*/
	wc.hInstance     = hinst;
	wc.hCursor       = LoadCursor ( ( HINSTANCE )NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject ( LTGRAY_BRUSH );
	wc.lpszClassName = WC_PROGRESSBAR;

	if ( ! RegisterClass ( &wc ) )
		ErrorCode = WPB_ERR_REGISTERCLASS;

	return ( ErrorCode );
}

void WINAPI ProgBar_Terminate 
(
	HINSTANCE			hInstance
) 
{
	WNDCLASS			WndClass;


	if ( GetClassInfo ( hInstance, WC_PROGRESSBAR, &WndClass ) )
		UnregisterClass ( WC_PROGRESSBAR, hInstance );

}


static void ProgBar_IDrawTilPos ( HWND hWnd, PROGBARDATA* pthis, int iPos )
{
	int		i;
	int		k;

	pthis->curPos = 0;
	pthis->curVisPos = 0;
	for ( i = pthis->minRange + pthis->iStep, k=i; 
	      i <= iPos; 
		  i += pthis->iStep, k += pthis->iStep )
		ProgBar_IStepIt ( hWnd, pthis );
}

void ProgBar_IRecalc ( HWND hWnd, PROGBARDATA* pthis )
{
	RECT	rc;

	GetClientRect ( hWnd, &rc );
	InflateRect ( &rc, -3, -3 );
	pthis->totalSteps = ( pthis->maxRange - pthis->minRange ) / pthis->iStep;
	pthis->iVisStepWidth = ( int ) ceil ( ( double )( rc.right - rc.left ) / pthis->totalSteps );
	pthis->iVisStep = pthis->iStep;
	if ( pthis->iVisStepWidth < MIN_STEPSIZE )
	{
		pthis->iVisStepWidth = MIN_STEPSIZE;
		pthis->iVisStep = ( pthis->maxRange - pthis->minRange ) *
						  pthis->iVisStepWidth /
						  ( rc.right - rc.left );
	}

	if ( pthis->iVisStep <= 0 )
		pthis->iVisStep = 1;
}

int ProgBar_ISetRange ( HWND hWnd, PROGBARDATA* pthis, int minRange, int maxRange )
{
	RECT	rc;
	LPARAM	OldRange = MAKELONG ( pthis->minRange, pthis->maxRange );

	GetClientRect ( hWnd, &rc );
	InflateRect ( &rc, -3, -3 );
	pthis->minRange = minRange;
	pthis->maxRange = maxRange;
	pthis->iVisStepWidth = 0; /*( pthis->maxRange - pthis->minRange ) / pthis->iStep;*/
	return ( OldRange );
}


static int ProgBar_IStepIt ( HWND hWnd, PROGBARDATA* pthis )
{
	int		k;
	int		inc;
	int		iRight;
	RECT	rc;
	HDC		hDC;
	int		curVisPos = pthis->curVisPos;
	HBRUSH	hBr = CreateSolidBrush ( RGB ( 0, 0, 192 ) );

	if ( pthis->iVisStepWidth <= 0 )
		ProgBar_IRecalc ( hWnd, pthis );
	

	inc = pthis->iVisStepWidth;
	if ( inc > DEF_STEPSIZE )
		inc = DEF_STEPSIZE;

	GetClientRect ( hWnd, &rc );
	InflateRect ( &rc, -3, -3 );
	iRight = rc.right;

	hDC = GetDC ( hWnd );
	if ( pthis->iVisStep > pthis->iStep )
	{
		int curPos = pthis->curPos;
		curPos %= pthis->iVisStep;
		if ( curPos + pthis->iStep >= pthis->iVisStep )
		{
			rc.left += curVisPos; /* / pthis->iVisStep * pthis->iVisStepWidth;*/
			rc.right = rc.left + inc-1;
			if ( rc.right > iRight )
				rc.right = iRight;
			FillRect ( hDC, &rc, hBr ); 
			pthis->curVisPos += pthis->iVisStepWidth;
		}
	}
	else
/*	if ( pthis->iVisStep < pthis->iStep )*/
	{
		rc.left += curVisPos - curVisPos % inc;
		rc.right = rc.left + inc-1;
		if ( rc.right > iRight )
			rc.right = iRight;
		k = curVisPos % inc + pthis->iVisStepWidth;
		pthis->curVisPos += pthis->iVisStepWidth;
		while ( TRUE )
		{
			if ( k >= inc )
			{
				FillRect ( hDC, &rc, hBr ); 
				k -= inc;
				rc.left += inc;
				rc.right += inc;
				if ( rc.right > iRight )
					rc.right = iRight;
			}
			else
				break;
		}
	}
	ReleaseDC ( hWnd, hDC );

	pthis->curPos += pthis->iStep;
	return 0;
}

static void ProgBar_IDrawBorder ( HWND hWnd, HDC hDC )
{
	RECT				rc;
	HPEN				hOldpen;
	HPEN				wpen	= ( HPEN )NULL;
	HPEN				bpen	= ( HPEN )NULL;

	GetClientRect ( hWnd, &rc );
	InflateRect ( &rc, -1, -1 );
	wpen = GetStockObject( WHITE_PEN );
	bpen = GetStockObject( BLACK_PEN  );
	
	hOldpen = SelectObject ( hDC, bpen );
	MoveToEx ( hDC, rc.left, rc.bottom, NULL );
	LineTo ( hDC, rc.left, rc.top );
	LineTo ( hDC, rc.right, rc.top );
	SelectObject ( hDC, wpen );
	LineTo ( hDC, rc.right, rc.bottom );
	LineTo ( hDC, rc.left, rc.bottom );
	SelectObject ( hDC, hOldpen );
}
