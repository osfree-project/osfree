/*  
	WTrackbr.c	1.5 
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
#include "WTrackbr.h"
#include "math.h"

#define  MAXPOINTS		    6

#define	 TBTICK_DOWNWARD	1
#define	 TBTICK_UPWARD		2
#define	 TBTICK_RIGHTWARD	3
#define	 TBTICK_LEFTWARD	4

#define	TBOFFSET		    8    /*for a horz. trackbar, the horz. offset of the*/
				                 /*trackbar channel	from the control window rect.*/
#define TBCHANNEL_HEIGHTMIN	3    /*the channel height of a horizontal trackbar*/
#define TBCHANNEL_HEIGHTMAX	10   /*the channel height of a horizontal trackbar with 
                                   selection enabled*/

#define TBTHUMB_OVER		8    /*offset of the thumb top relative to the channel top*/
#define TBTHUMB_HEIGHTMAX	TBCHANNEL_HEIGHTMAX+TBTHUMB_OVER*2   /*the thumb height of a horizontal trackbar*/
#define TBTHUMB_HEIGHTMIN	5    /*the thumb height of a horizontal trackbar*/
#define TBTHUMB_WIDTHMAX	10   /*the thumb width of a horizontal trackbar */
#define TBTHUMB_WIDTHMIN	2    /*the thumb width of a horizontal trackbar */
#define TBTHUMB_PTR_HEIGHT  5    /*the height of the thumb pointer*/
#define TBTICK_HEIGHT		2
#define TBTICK_OFFSET		3    /*offset of a tick with respect to the bounding*/
			            	     /*rectangle of the thumb*/
#define TBCHANNEL_YMAX	    TBTHUMB_OVER    /*offset of a horz trackbar with respect to the top */
static char TRACKBAR_PROP[] = "Trackbar_Data";	
typedef struct {
	POINT	ptPos;
	SIZE	ptSize;
	UINT	uStyles;
	BOOL    bDrag;

	RECT	rectSlide;
	HDC	    hBkDC;
	HBITMAP hbmBack;

	SIZE    sizRange;
	int	    Range;	/* = abs ( sizRange.cy - sizRange.cx )*/
	int     nThumbPos;
	SIZE	sizSelect;
	int	    TicCount; /*length of int array whose handle is hTics*/
	HGLOBAL hTics;
	int	    nTickFreq;
	int     nPageSize;
	int	    nLineSize;
    BOOL    bFocus;
} TRACKBARDATA;

#define GETTHISDATA(hwnd) \
   GetProp(hwnd, TRACKBAR_PROP )
#define SETTHISDATA(hwnd, pthisNew) \
   SetProp(hwnd, TRACKBAR_PROP, pthisNew)
#define REMOVETHISDATA(hwnd) \
   RemoveProp(hwnd, TRACKBAR_PROP )


/*prototypes*/
static void Trackbar_PaintChannel ( HDC hDC, TRACKBARDATA* pTrackbar );
static void Trackbar_PaintThumb ( HDC hDC, TRACKBARDATA* pTrackbar );
static void Trackbar_PaintTicks ( HDC hDC, TRACKBARDATA* pTrackbar );

static int Trackbar_OnEraseBkground ( HWND hWnd, HDC hDC );

static LRESULT  
Trackbar_TBMWndProc ( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam );

static int
Trackbar_IGetCurPoint ( TRACKBARDATA* pTrackbar );

static int
Trackbar_IResetRange ( TRACKBARDATA* pthis );

static int
Trackbar_IGetPointFrPos ( TRACKBARDATA* pTrackbar, int Pos );

static int
Trackbar_IGetPosFrPoint ( TRACKBARDATA* pTrackbar, int Point );

static void
Trackbar_IResetTics ( TRACKBARDATA* pthis, int newTicFreq );

static void
Trackbar_IMoveThumb ( HWND hWnd, TRACKBARDATA* pthis, int newPoint );

static int
Trackbar_ISetThumbPos ( TRACKBARDATA* pthis, int NewPos );

static void
Trackbar_IDrawSelectTic ( HDC hDC, int x, int y, UINT uFlag );

static BOOL 
Trackbar_OnCreate 
(
	HWND				hWnd, 
	CREATESTRUCT*		pcs
) 
{

	RECT				rcClient;
    int                 iChannelHeight;
    int                 iThumbHeight;
    int                 iTickSpace;
	HWND				hwndParent	= GetParent(hWnd);
	HDC				    hDC		    = GetDC	( hwndParent );
	HGLOBAL				hTrackbar	= ( HGLOBAL )NULL;
	TRACKBARDATA*		pthis		= ( TRACKBARDATA* )NULL;

	int				    ErrorCode	= 0;

	if ( ! ( hTrackbar = GlobalAlloc ( GHND, sizeof ( TRACKBARDATA ) ) ) )
		ErrorCode = WTB_ERR_GLOBALALLOC;
	else
	if ( ! ( pthis = ( TRACKBARDATA *) GlobalLock ( hTrackbar ) ) )
		ErrorCode = WTB_ERR_GLOBALLOCK;
	else
	{
		SETTHISDATA(hWnd, hTrackbar);
		pthis->uStyles = pcs->style;
		pthis->bDrag = FALSE;
		pthis->nTickFreq = 1;
		pthis->sizRange.cx = 0;
		pthis->sizRange.cy = 10;
		pthis->Range = 10;
		pthis->sizSelect.cx = 0;
		pthis->sizSelect.cy = 0;
		pthis->nPageSize = 1;
		pthis->nLineSize = 1;
		pthis->nThumbPos = 0;

        iChannelHeight = ( pthis->uStyles & TBS_ENABLESELRANGE )?
                         TBCHANNEL_HEIGHTMAX : TBCHANNEL_HEIGHTMIN;   
        iTickSpace  = ( pthis->uStyles & TBS_NOTICKS ) ? 1 :
                      ( TBTICK_HEIGHT + TBTICK_OFFSET );
		GetClientRect ( hWnd, &rcClient );
		if ( pthis->uStyles & TBS_VERT )
		{
            iThumbHeight = 
                ( pthis->uStyles & TBS_ENABLESELRANGE )? TBTHUMB_HEIGHTMAX : 
                ( rcClient.right - iTickSpace < TBTHUMB_HEIGHTMIN )?
                TBTHUMB_HEIGHTMIN : 
                (rcClient.right - iTickSpace > TBTHUMB_HEIGHTMAX )?
                TBTHUMB_HEIGHTMAX : rcClient.right - iTickSpace;   
            pthis->rectSlide.left = rcClient.left + iTickSpace;
			pthis->rectSlide.right = pthis->rectSlide.left + iThumbHeight;
			pthis->rectSlide.top = rcClient.top;
			pthis->rectSlide.bottom = pthis->rectSlide.top + TBTHUMB_WIDTHMAX ;
			pthis->ptPos.x = pthis->rectSlide.left + TBTHUMB_OVER;
			pthis->ptPos.y = rcClient.top + TBOFFSET;
			pthis->ptSize.cx = iChannelHeight;
			pthis->ptSize.cy = rcClient.bottom - rcClient.top - 2 * TBOFFSET;
		}
		else
		{
            iThumbHeight = 
                ( pthis->uStyles & TBS_ENABLESELRANGE )? TBTHUMB_HEIGHTMAX : 
                ( rcClient.bottom - iTickSpace < TBTHUMB_HEIGHTMIN )?
                TBTHUMB_HEIGHTMIN : 
                (rcClient.bottom - iTickSpace > TBTHUMB_HEIGHTMAX )?
                TBTHUMB_HEIGHTMAX : rcClient.bottom - iTickSpace;   
			pthis->rectSlide.left = rcClient.left;
			pthis->rectSlide.right = pthis->rectSlide.left + TBTHUMB_WIDTHMAX;
			pthis->rectSlide.top = rcClient.top + TBTICK_HEIGHT + TBTICK_OFFSET;
			pthis->rectSlide.bottom = rcClient.top + iThumbHeight;
			pthis->ptPos.x = rcClient.left + TBOFFSET;
			pthis->ptPos.y = pthis->rectSlide.top + TBTHUMB_OVER;
			pthis->ptSize.cx = rcClient.right - rcClient.left - 2 * TBOFFSET;
			pthis->ptSize.cy = iChannelHeight; 
		}

		pthis->hbmBack = CreateCompatibleBitmap ( hDC,
					pthis->rectSlide.right - pthis->rectSlide.left+2, 
					pthis->rectSlide.bottom - pthis->rectSlide.top+2 );
		pthis->hBkDC = CreateCompatibleDC ( hDC );
		pthis->hbmBack = SelectObject ( pthis->hBkDC, pthis->hbmBack );

		if ( pthis->uStyles & TBS_NOTHUMB )
		{
			pthis->TicCount = 0;
			pthis->hTics = ( HGLOBAL )NULL;
		}
		else
		{
			Trackbar_IResetTics ( pthis, pthis->nTickFreq );
		}
	}

	ReleaseDC ( hwndParent, hDC );
	if ( pthis )
		GlobalUnlock ( hTrackbar );
	return ( ErrorCode >= 0 );
}

static void Trackbar_OnKeyDown (HWND hWnd, int VKey, int cRepeat, UINT keyFlags) 
{
	UINT			uMsg;
	WORD			wNotify;
	HGLOBAL			hTrackbar	= ( HGLOBAL )NULL;
	TRACKBARDATA*	pthis		= ( TRACKBARDATA* )NULL;
	BOOL			bScroll		= TRUE;
	int			    newPos;

	int			    ErrorCode	= 0;

	if ( ! ( hTrackbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TRACKBARDATA *) GlobalLock ( hTrackbar ) ) )
		ErrorCode = WTB_ERR_GLOBALLOCK;
	else
	{
		if ( pthis->uStyles & TBS_VERT )
			uMsg = WM_VSCROLL;
		else
			uMsg = WM_HSCROLL;
		switch ( VKey )
		{
			case VK_RIGHT	:
			case VK_DOWN	:
				wNotify = TB_LINEDOWN;
				newPos  = pthis->nThumbPos + pthis->nLineSize;
				break;
			case VK_LEFT	:
			case VK_UP	:
				wNotify = TB_LINEUP;
				newPos  = pthis->nThumbPos - pthis->nLineSize;
				break;
			case VK_HOME	:
				wNotify = TB_TOP;
				newPos  = pthis->sizRange.cx;
				break;
			case VK_END	:
				wNotify = TB_BOTTOM;
				newPos  = pthis->sizRange.cy;
				break;
			case VK_NEXT	:
				wNotify = TB_PAGEDOWN;
				newPos  = pthis->nThumbPos + pthis->nPageSize;
				break;
			case VK_PRIOR	:
				wNotify = TB_PAGEUP;
				newPos  = pthis->nThumbPos - pthis->nPageSize;
				break;
			default         :
				bScroll = FALSE;
		}

        bScroll = bScroll && 
                  ( ( pthis->uStyles & TBS_ENABLESELRANGE ) ?  
                    ( newPos >= pthis->sizSelect.cx ) &&
                    ( newPos <= pthis->sizSelect.cy ) : TRUE );
		if ( bScroll )
		{
			Trackbar_IMoveThumb ( hWnd, pthis, newPos );
			SendMessage ( GetParent ( hWnd ), uMsg, 
					MAKELONG ( wNotify, pthis->nThumbPos ), 
					( LPARAM ) hWnd );
		}
	}	
	
	if ( pthis )
		GlobalUnlock ( hTrackbar );
}


static void Trackbar_OnKeyUp (HWND hWnd, int VKey, int cRepeat, UINT keyFlags) 
{
	UINT			uMsg;
	HGLOBAL			hTrackbar	= ( HGLOBAL )NULL;
	TRACKBARDATA*	pthis		= ( TRACKBARDATA* )NULL;
	int			    ErrorCode	= 0;

	if ( ! ( hTrackbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TRACKBARDATA *) GlobalLock ( hTrackbar ) ) )
		ErrorCode = WTB_ERR_GLOBALLOCK;
	else
	{
		uMsg = WM_HSCROLL;
		if ( pthis->uStyles & TBS_VERT )
			uMsg = WM_VSCROLL;
		switch ( VKey )
		{
			case VK_RIGHT	:
			case VK_DOWN	:
			case VK_LEFT	:
			case VK_UP	:
			case VK_HOME	:
			case VK_END	:
			case VK_NEXT	:
			case VK_PRIOR	:
				SendMessage ( GetParent ( hWnd ), uMsg, 
					MAKELONG ( TB_ENDTRACK, pthis->nThumbPos ), ( LPARAM ) hWnd );
				break;
		}

	}	
	
	if ( pthis )
		GlobalUnlock ( hTrackbar );
}


static void Trackbar_OnLButtonDown (HWND hWnd, int x, int y, UINT keyFlags) 
{
	HGLOBAL			hTrackbar	= ( HGLOBAL )NULL;
	TRACKBARDATA*	pthis		= ( TRACKBARDATA* )NULL;
	RECT*			pRect		= ( RECT* )NULL;
	
	int			    ErrorCode	= 0;

	if ( ! ( hTrackbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TRACKBARDATA *) GlobalLock ( hTrackbar ) ) )
		ErrorCode = WTB_ERR_GLOBALLOCK;
	else
	{
		pRect =	&pthis->rectSlide;
	
		pthis->bDrag = FALSE;
		if ( ( x > pRect->left ) && ( x < pRect->right ) && 
		     ( y > pRect->top  ) && ( y < pRect->bottom )
		   )
		{
			pthis->bDrag = TRUE;
			SetCapture ( hWnd );
			SetFocus ( hWnd );
		}
		else
		if ( ( x >= pthis->ptPos.x ) && ( x <= pthis->ptPos.x + pthis->ptSize.cx ) && 
		     ( y >= pthis->ptPos.y  ) && ( y <= pthis->ptPos.y + pthis->ptSize.cy )
		   )
		{
			int		clickPoint;
			int		nThumbPoint = Trackbar_IGetCurPoint ( pthis );
			clickPoint = ( pthis->uStyles & TBS_VERT ) ? y : x;
			if ( clickPoint < nThumbPoint )
				SendMessage ( hWnd, WM_KEYDOWN, ( WPARAM )VK_PRIOR, 0L );
			if ( clickPoint > nThumbPoint )
				SendMessage ( hWnd, WM_KEYDOWN, ( WPARAM )VK_NEXT, 0L );
			SetFocus ( hWnd );
		}


	}	
	
	if ( pthis )
		GlobalUnlock ( hTrackbar );
}

static void Trackbar_OnLButtonUp (HWND hWnd, int x, int y, UINT keyFlags) 
{
	UINT			uMsg;
	HGLOBAL			hTrackbar	= ( HGLOBAL )NULL;
	TRACKBARDATA*	pthis		= ( TRACKBARDATA* )NULL;
	
	int				ErrorCode	= 0;

	if ( ! ( hTrackbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TRACKBARDATA *) GlobalLock ( hTrackbar ) ) )
		ErrorCode = WTB_ERR_GLOBALLOCK;
	else
    if ( pthis->bDrag )
    {
		pthis->bDrag = FALSE;
    	ReleaseCapture ();
#if 0
        if ( pthis->uStyles & TBS_VERT )
		    pthis->nThumbPos = Trackbar_IGetPosFrPoint ( pthis, y );
	    else
		    pthis->nThumbPos = Trackbar_IGetPosFrPoint ( pthis, x );
#endif
	    uMsg = ( pthis->uStyles & TBS_VERT ) ? WM_HSCROLL : WM_VSCROLL;
        Trackbar_IMoveThumb ( hWnd, pthis, pthis->nThumbPos );

        SendMessage ( GetParent ( hWnd ), uMsg, 
			    MAKELONG ( TB_THUMBTRACK, pthis->nThumbPos ), ( LPARAM )hWnd );

    }
	

	if ( pthis )
		GlobalUnlock ( hTrackbar );

}

/*Need revisit this*/
static void Trackbar_OnDestroy ( HWND hWnd )
{
	HGLOBAL				hTrackbar	= ( HGLOBAL )NULL;
	TRACKBARDATA*		pthis		= ( TRACKBARDATA* )NULL;

	int				    ErrorCode	= 0;

	if ( ! ( hTrackbar = GlobalAlloc ( GHND, sizeof ( TRACKBARDATA ) ) ) )
		ErrorCode = WTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TRACKBARDATA *) GlobalLock ( hTrackbar ) ) )
		ErrorCode = WTB_ERR_GLOBALLOCK;
	else
	{
		if ( pthis->hTics )
			GlobalFree ( pthis->hTics );
		REMOVETHISDATA ( hWnd );
		GlobalFree ( hTrackbar );
	}
}

static void Trackbar_OnMouseMove 
(
	HWND			hWnd, 
	int			    x, 
	int			    y, 
	UINT			keyFlags
) 
{
	SIZE			Size;
	UINT			uMessage;
    int             newThumbPos;
	HGLOBAL			hTrackbar	= ( HGLOBAL )NULL;
	TRACKBARDATA*	pthis		= ( TRACKBARDATA* )NULL;
	RECT*			pRect		= ( RECT* )NULL;
	POINT*			pptPos		= ( POINT* )NULL;
	SIZE*			pptSize		= ( SIZE* )NULL;
	BOOL			bTrue		= FALSE;
	int			    ErrorCode	= 0;

	if ( ! ( hTrackbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TRACKBARDATA *) GlobalLock ( hTrackbar ) ) )
		ErrorCode = WTB_ERR_GLOBALLOCK;
	else
	{
		if ( pthis->uStyles & TBS_NOTHUMB )
			return;

		pRect   = &pthis->rectSlide;
		pptPos	= &pthis->ptPos;
		pptSize = &pthis->ptSize;
		Size.cx = pRect->right - pRect->left;
		Size.cy = pRect->bottom - pRect->top;
		if ( pthis->uStyles & TBS_VERT )
        {
			newThumbPos = Trackbar_IGetPosFrPoint ( pthis, y );
			bTrue = ( pthis->bDrag ) && ( y >= pptPos->y ) && ( y <= pptPos->y + pptSize->cy ) ;
            
        }
		else
        {
			newThumbPos = Trackbar_IGetPosFrPoint ( pthis, x );
			bTrue = ( pthis->bDrag ) && ( x >= pptPos->x ) && ( x <= pptPos->x + pptSize->cx ) ;
        }

        if ( pthis->uStyles & TBS_ENABLESELRANGE )
            bTrue = bTrue && ( newThumbPos >= pthis->sizSelect.cx ) &&
                             ( newThumbPos <= pthis->sizSelect.cy );          
        if ( bTrue )
		{
			HDC hDC = GetDC(hWnd);
			BitBlt ( hDC, pRect->left, pRect->top, 
				Size.cx+2, Size.cy+2, pthis->hBkDC, 0, 0, SRCCOPY );

			if ( pthis->uStyles & TBS_VERT )
			{
				pthis->rectSlide.top = y - TBTHUMB_WIDTHMAX/2;
				pthis->rectSlide.bottom = y + TBTHUMB_WIDTHMAX/2;
			}
			else
			{
				pthis->rectSlide.left = x - TBTHUMB_WIDTHMAX/2;
				pthis->rectSlide.right = x + TBTHUMB_WIDTHMAX/2;
			}

			pthis->nThumbPos = newThumbPos;
            Trackbar_PaintThumb ( hDC, pthis );
			ReleaseDC(hWnd, hDC);

			uMessage = ( pthis->uStyles & TBS_VERT ) ? WM_VSCROLL : WM_HSCROLL;
			SendMessage ( GetParent ( hWnd ), uMessage, 
					MAKELONG ( TB_THUMBTRACK, pthis->nThumbPos ), ( LPARAM )hWnd );
		}
	}
	
	if ( pthis )
		GlobalUnlock ( hTrackbar );
}

static int Trackbar_OnEraseBkground 
(
		HWND			hWnd, 
		HDC				hDC 
)
{
		RECT			rect;
		HPEN			hPen    = GetStockObject ( NULL_PEN );
		HBRUSH			hBrush	= GetStockObject ( LTGRAY_BRUSH );
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



static void Trackbar_OnPaint 
(
	HWND				hWnd, 
	HDC			    	hPaintDC 
)
{
	PAINTSTRUCT         ps;
	HGLOBAL			    hTrackbar	= ( HGLOBAL )NULL;
	TRACKBARDATA*		pthis		= ( TRACKBARDATA* )NULL;
	
	int			        ErrorCode	= 0;

	if ( ! ( hTrackbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TRACKBARDATA *) GlobalLock ( hTrackbar ) ) )
		ErrorCode = WTB_ERR_GLOBALLOCK;
	else
	{
		/* Start painting.*/
		BeginPaint(hWnd, &ps);

		Trackbar_PaintChannel ( ps.hdc, pthis ); 

		if ( ! ( pthis->uStyles & TBS_NOTICKS ) )
			Trackbar_PaintTicks ( ps.hdc, pthis );
		
		if ( ! ( pthis->uStyles & TBS_NOTHUMB ) )
			Trackbar_PaintThumb ( ps.hdc, pthis );
   
		EndPaint(hWnd, &ps);
	}
	if ( pthis )
		GlobalUnlock ( hTrackbar );
}

static void Trackbar_OnSetFocus ( HWND hWnd, BOOL bSetFocus ) 
{
	RECT			rect;
	HGLOBAL			hTrackbar	= ( HGLOBAL )NULL;
	TRACKBARDATA*	pthis		= ( TRACKBARDATA* )NULL;
	
	int			    ErrorCode	= 0;

	if ( ! ( hTrackbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TRACKBARDATA *) GlobalLock ( hTrackbar ) ) )
		ErrorCode = WTB_ERR_GLOBALLOCK;
	else
	{
        HDC     hDC = GetDC ( hWnd );
        pthis->bFocus = bSetFocus;
        GetClientRect ( hWnd, &rect );
        InflateRect ( &rect, -2, -2 );
        DrawFocusRect ( hDC, &rect );
        ReleaseDC ( hWnd, hDC );

	}	
	
	if ( pthis )
		GlobalUnlock ( hTrackbar );
}


static void Trackbar_PaintTicks ( HDC hDC, TRACKBARDATA* pTrackbar )
{

	POINT				ptStart;
	int				    nTickPoint;
	int				    nIter;
	int				    nNoOfTicks;
	RECT*				rcSlide		= &pTrackbar->rectSlide;
	POINT*				ptPos		= &pTrackbar->ptPos;
	HPEN				hpen		= ( HPEN )NULL;
	int*				pTics		= ( int* )NULL;
	BOOL				bDrawSelect	= FALSE;
	
	
	hpen = CreatePen ( PS_SOLID, 1, RGB ( 0, 0, 0 ) );
	hpen  = SelectObject ( hDC, hpen );

	if ( pTrackbar->nTickFreq <= 0 )
		pTrackbar->nTickFreq = 1;
	bDrawSelect = ( pTrackbar->uStyles & TBS_ENABLESELRANGE ) && 
		      ( pTrackbar->sizSelect.cx != pTrackbar->sizSelect.cy );

	pTics = ( int * )GlobalLock ( pTrackbar->hTics );
	nNoOfTicks = pTrackbar->TicCount;
	if ( pTrackbar->uStyles & TBS_VERT )
	{
		if ( ( pTrackbar->uStyles & TBS_BOTH ) ||
		     ( pTrackbar->uStyles & TBS_LEFT )
		   )
		{
			ptStart.x = rcSlide->left - TBTICK_OFFSET;
			ptStart.y = ptPos->y;

			for ( nIter = 0; nIter < pTrackbar->TicCount; nIter++ )
			{
				nTickPoint = Trackbar_IGetPointFrPos ( pTrackbar, pTics[nIter] );
				MoveToEx ( hDC, ptStart.x , 
						nTickPoint, NULL );
				LineTo ( hDC, ptStart.x - TBTICK_HEIGHT, 
						nTickPoint );
			}
			if ( bDrawSelect )
			{
				Trackbar_IDrawSelectTic ( hDC, ptStart.x,  
					Trackbar_IGetPointFrPos ( pTrackbar, pTrackbar->sizSelect.cx ),
					TBTICK_LEFTWARD );
				Trackbar_IDrawSelectTic ( hDC, ptStart.x, 
					Trackbar_IGetPointFrPos ( pTrackbar, pTrackbar->sizSelect.cy ),
					TBTICK_LEFTWARD );
			}
		}
		
		if (  ( pTrackbar->uStyles & TBS_BOTH ) ||
		      ( pTrackbar->uStyles & TBS_AUTOTICKS ) ||
		      ( ( ( WORD )pTrackbar->uStyles )  == TBS_RIGHT  )
		   )
		{
			ptStart.x = rcSlide->right + TBTICK_OFFSET;
			ptStart.y = ptPos->y;

			for ( nIter = 0; nIter < pTrackbar->TicCount; nIter++ )
			{
				nTickPoint = Trackbar_IGetPointFrPos ( pTrackbar, pTics[nIter] );
				MoveToEx ( hDC, ptStart.x , 
						nTickPoint, NULL );
				LineTo ( hDC, ptStart.x + TBTICK_HEIGHT, 
						nTickPoint );
			}
			if ( bDrawSelect )
			{
				Trackbar_IDrawSelectTic ( hDC, ptStart.x,  
					Trackbar_IGetPointFrPos ( pTrackbar, pTrackbar->sizSelect.cx ),
					TBTICK_RIGHTWARD );
				Trackbar_IDrawSelectTic ( hDC, ptStart.x, 
					Trackbar_IGetPointFrPos ( pTrackbar, pTrackbar->sizSelect.cy ),
					TBTICK_RIGHTWARD );
			}

		}
	}
	else
	{
		if ( ( pTrackbar->uStyles & TBS_BOTH ) ||
		     ( pTrackbar->uStyles & TBS_TOP )
		   )
		{
			ptStart.x =  ptPos->x;
			ptStart.y = rcSlide->top - TBTICK_OFFSET;

			for ( nIter = 0; nIter < pTrackbar->TicCount; nIter++ )
			{
				nTickPoint = Trackbar_IGetPointFrPos ( pTrackbar, pTics[nIter] );
				MoveToEx ( hDC, nTickPoint, 
						ptStart.y, NULL );
				LineTo ( hDC, nTickPoint, 
						ptStart.y - TBTICK_HEIGHT );
			}
			if ( bDrawSelect )
			{
				Trackbar_IDrawSelectTic ( hDC, 
					Trackbar_IGetPointFrPos ( pTrackbar, pTrackbar->sizSelect.cx ),
					ptStart.y, TBTICK_UPWARD );
				Trackbar_IDrawSelectTic ( hDC, 
					Trackbar_IGetPointFrPos ( pTrackbar, pTrackbar->sizSelect.cy ),
					ptStart.y, TBTICK_UPWARD );
			}
		}
		
		if (  ( pTrackbar->uStyles & TBS_BOTH ) ||
		      ( pTrackbar->uStyles & TBS_AUTOTICKS ) ||
		      ( ( ( WORD )pTrackbar->uStyles )  == TBS_BOTTOM  )
		   )
		{
			ptStart.x = ptPos->x;
			ptStart.y = rcSlide->bottom + TBTICK_OFFSET;

			for ( nIter = 0; nIter < nNoOfTicks; nIter++ )
			{
				nTickPoint = Trackbar_IGetPointFrPos ( pTrackbar, pTics[nIter] );
				MoveToEx ( hDC, nTickPoint, 
						ptStart.y, NULL );
				LineTo ( hDC, nTickPoint, 
						ptStart.y + TBTICK_HEIGHT );
			}
			if ( bDrawSelect )
			{
				Trackbar_IDrawSelectTic ( hDC, 
					Trackbar_IGetPointFrPos ( pTrackbar, pTrackbar->sizSelect.cx ),
					ptStart.y, TBTICK_DOWNWARD );
				Trackbar_IDrawSelectTic ( hDC, 
					Trackbar_IGetPointFrPos ( pTrackbar, pTrackbar->sizSelect.cy ),
					ptStart.y, TBTICK_DOWNWARD );
			}

		}
	}

	DeleteObject ( SelectObject ( hDC, hpen ) );
}


static void 
Trackbar_PaintThumb ( HDC hDC, TRACKBARDATA* pTrackbar )
{

	RECT*				rcSlide		= &pTrackbar->rectSlide;
	HPEN				nullPen		= ( HPEN )NULL;
	HPEN				whitePen	= ( HPEN )NULL;
	HPEN				blackPen	= ( HPEN )NULL;
	HPEN				origPen		= ( HPEN )NULL;
	HBRUSH				grayBrush	= ( HBRUSH )NULL;
	POINT				points[MAXPOINTS];

	if ( ! ( nullPen  = GetStockObject ( NULL_PEN ) ) ||
	     ! ( whitePen = GetStockObject ( WHITE_PEN ) ) ||
	     ! ( blackPen = GetStockObject ( BLACK_PEN ) ) ||
	     ! ( grayBrush = GetStockObject ( LTGRAY_BRUSH ) ) 
	   )
		return;

	BitBlt ( pTrackbar->hBkDC, 0, 0, rcSlide->right - rcSlide->left+2, 
					 rcSlide->bottom - rcSlide->top+2, 
			    hDC, rcSlide->left, rcSlide->top, SRCCOPY );

	origPen = SelectObject ( hDC, nullPen );
	grayBrush = SelectObject ( hDC, grayBrush );
	Rectangle ( hDC, rcSlide->left, rcSlide->top, 
		rcSlide->right+2, rcSlide->bottom+2 );

	nullPen = SelectObject ( hDC, whitePen );


	if ( pTrackbar->uStyles & TBS_VERT )
	{

		points[0].x = rcSlide->left;
		points[0].y = rcSlide->bottom;
		points[1].x = rcSlide->left;
		points[1].y = rcSlide->top;
		points[2].x = rcSlide->right;
		points[2].y = rcSlide->top;
		points[3].x = rcSlide->right;
		points[3].y = rcSlide->bottom;
		if ( ( pTrackbar->uStyles & TBS_BOTH ) ||
		     ( pTrackbar->uStyles & TBS_LEFT )
		   )
		{
			points[0].x = rcSlide->left;
			points[0].y = rcSlide->top + TBTHUMB_WIDTHMAX/2;
			points[1].x = rcSlide->left + TBTHUMB_PTR_HEIGHT;
			points[1].y = rcSlide->top;

		}	

		if (  ( pTrackbar->uStyles & TBS_BOTH ) ||
		      ( pTrackbar->uStyles & TBS_AUTOTICKS ) ||
		      ( ( ( WORD )pTrackbar->uStyles )  == TBS_RIGHT  )
		   )
		{
			points[2].x = rcSlide->right - TBTHUMB_PTR_HEIGHT;
			points[2].y = rcSlide->top;
			points[3].x = rcSlide->right;
			points[3].y = rcSlide->top + TBTHUMB_WIDTHMAX/2;

		}
		Polyline ( hDC, points, 4 );
		whitePen = SelectObject ( hDC, blackPen );
		points[1].y = rcSlide->bottom;
		points[2].y = rcSlide->bottom;
		Polyline ( hDC, points, 4 );
	}
	else
	{
		points[0].x = rcSlide->right;
		points[0].y = rcSlide->top;
		points[1].x = rcSlide->left;
		points[1].y = rcSlide->top;
		points[2].x = rcSlide->left;
		points[2].y = rcSlide->bottom;
		points[3].x = rcSlide->right;
		points[3].y = rcSlide->bottom;
		if ( ( pTrackbar->uStyles & TBS_BOTH ) ||
		     ( pTrackbar->uStyles & TBS_TOP )
		   )
		{
			points[0].x = rcSlide->left + TBTHUMB_WIDTHMAX/2;
			points[0].y = rcSlide->top;
			points[1].x = rcSlide->left;
			points[1].y = rcSlide->top + TBTHUMB_PTR_HEIGHT;

		}	

		if (  ( pTrackbar->uStyles & TBS_BOTH ) ||
		      ( pTrackbar->uStyles & TBS_AUTOTICKS ) ||
		      ( ( ( WORD )pTrackbar->uStyles )  == TBS_BOTTOM  )
		   )
		{
			points[2].x = rcSlide->left;
			points[2].y = rcSlide->bottom - TBTHUMB_PTR_HEIGHT;
			points[3].x = rcSlide->left + TBTHUMB_WIDTHMAX/2;
			points[3].y = rcSlide->bottom;

		}
		Polyline ( hDC, points, 4 );
		whitePen = SelectObject ( hDC, blackPen );
		points[1].x = rcSlide->right;
		points[2].x = rcSlide->right;
		Polyline ( hDC, points, 4 );
	}

	
	blackPen = SelectObject ( hDC, origPen );
	grayBrush = SelectObject ( hDC, grayBrush );
	DeleteObject ( nullPen );
	DeleteObject ( whitePen );
	DeleteObject ( blackPen );
	DeleteObject ( grayBrush );
}


static void Trackbar_PaintChannel ( HDC hDC, TRACKBARDATA* pTrackbar )
{

	POINT*					ptPos		= &pTrackbar->ptPos;
	SIZE*					ptSize		= &pTrackbar->ptSize;
	SIZE					Select		= pTrackbar->sizSelect;
	SIZE*					pRange		= &pTrackbar->sizRange;
	HPEN					hWhitepen	= ( HPEN )NULL;
	HPEN					hBlackpen	= ( HPEN )NULL;
	HPEN					hpenOriginal	= ( HPEN )NULL;
	HBRUSH					hbrSelect	= ( HBRUSH )NULL;
	hWhitepen = CreatePen(PS_SOLID, 2, RGB(255,255,255));
	hBlackpen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
	hpenOriginal = SelectObject(hDC, hBlackpen);
	hbrSelect = CreateSolidBrush ( RGB ( 0, 0, 255 ) ); 
	if ( Select.cx < pRange->cx )
		Select.cx = pRange->cx;
	if ( Select.cy > pRange->cy )
		Select.cy = pRange->cy;
	if ( pTrackbar->uStyles & TBS_ENABLESELRANGE && Select.cx != Select.cy )
	{
		hbrSelect = SelectObject ( hDC, hbrSelect );
		if ( pTrackbar->uStyles & TBS_VERT )
		{
			Rectangle ( hDC, ptPos->x, 
			                 Trackbar_IGetPointFrPos ( pTrackbar, Select.cx ),
					 ptPos->x + ptSize->cx , 
					 Trackbar_IGetPointFrPos ( pTrackbar, Select.cy ) ); 
		}
		else
		{
			Rectangle ( hDC, Trackbar_IGetPointFrPos ( pTrackbar, Select.cx ), 
					 ptPos->y,
					 Trackbar_IGetPointFrPos ( pTrackbar, Select.cy ), 
					 ptPos->y+ptSize->cy );
		}
		hbrSelect = SelectObject ( hDC, hbrSelect );
	}
	
	/* Draw trackbar channel.*/
	MoveToEx( hDC, ptPos->x+ptSize->cx, ptPos->y, NULL);
	LineTo ( hDC, ptPos->x, ptPos->y );
	LineTo ( hDC, ptPos->x, ptPos->y + ptSize->cy );
	SelectObject ( hDC, hWhitepen );
	LineTo ( hDC, ptPos->x + ptSize->cx, ptPos->y + ptSize->cy );
	LineTo ( hDC, ptPos->x + ptSize->cx, ptPos->y );

	SelectObject ( hDC, hpenOriginal );
	DeleteObject ( hWhitepen );
	DeleteObject ( hBlackpen );
	DeleteObject ( hbrSelect );	
}


static LRESULT WINAPI 
Trackbar_WndProc
(
	HWND			hWnd,
        UINT			uMessage,
        WPARAM			wParam,
        LPARAM			lParam
)
{
	switch (uMessage)
	{
		case WM_COMMAND:
				break;        
		case WM_CREATE:
			if ( ! Trackbar_OnCreate ( hWnd, ( LPCREATESTRUCT )lParam ) )
				return ( LRESULT ) -1L;
			break;
		case WM_DESTROY:
			Trackbar_OnDestroy ( hWnd );
			break;
		case WM_ENABLE:
			break;
		case WM_ERASEBKGND:
			return ( LRESULT )Trackbar_OnEraseBkground ( hWnd, ( HDC )wParam );
			break;
		case WM_GETDLGCODE:
			return ( LRESULT )DLGC_WANTARROWS;
		
		case WM_KEYDOWN:
			Trackbar_OnKeyDown ( hWnd, (UINT)( wParam ), ( int )( short )LOWORD ( lParam ), 
				(UINT)HIWORD ( lParam ) );
			break;
		
		case WM_KEYUP:
			Trackbar_OnKeyUp ( hWnd, (UINT)( wParam ), ( int )( short )LOWORD ( lParam ), 
				(UINT)HIWORD ( lParam ) );
			break;

		case WM_KILLFOCUS:
            Trackbar_OnSetFocus ( hWnd, FALSE );
			break;

		case WM_LBUTTONDOWN:
			Trackbar_OnLButtonDown ( hWnd,(int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )( wParam ) );
			break;

		case WM_LBUTTONUP:
			Trackbar_OnLButtonUp ( hWnd,(int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )( wParam ) );
			break;

		case WM_MOUSEMOVE:
			Trackbar_OnMouseMove ( hWnd,(int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )( wParam ) );
			break;

		case WM_PAINT:
			Trackbar_OnPaint( hWnd, ( HDC ) wParam  );
			break;

		case WM_SETFOCUS:
            Trackbar_OnSetFocus ( hWnd, TRUE );
			break;
		
		default:
		    if ( uMessage >= TBM_GETPOS && uMessage <= TBM_GETTHUMBLENGTH )
			    return Trackbar_TBMWndProc(hWnd, uMessage, wParam, lParam);
		    return DefWindowProc(hWnd, uMessage, wParam, lParam);
	}
    
	    return 0;
}

static LRESULT  
Trackbar_TBMWndProc
(
	HWND			hWnd,
        UINT			uMessage,
        WPARAM			wParam,
        LPARAM			lParam
)
{
	HGLOBAL			hTrackbar	= ( HGLOBAL )NULL;
	TRACKBARDATA*		pthis		= ( TRACKBARDATA* )NULL;
	LPVOID			pTics		= ( LPVOID )NULL;
	
	int			ErrorCode	= 0;

	if ( ! ( hTrackbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TRACKBARDATA *) GlobalLock ( hTrackbar ) ) )
		ErrorCode = WTB_ERR_GLOBALLOCK;
	else
	{
		switch (uMessage)
		{
			case TBM_GETPOS:
				return ( LRESULT )pthis->nThumbPos;
				
			case TBM_GETRANGEMIN:
				return ( LRESULT )pthis->sizRange.cx;

			case TBM_GETRANGEMAX:
				return ( LRESULT )pthis->sizRange.cy;

			case TBM_GETTIC:
				return ( LRESULT )( pthis->sizRange.cx + pthis->nTickFreq * ( int )wParam );
			
			/*document is vague about the index of the tick mark set
			/assume wParam*/
			case TBM_SETTIC:
				if ( lParam < pthis->sizRange.cx || lParam > pthis->sizRange.cy ||
				     ( int )wParam < 0 || ( int )wParam >= pthis->TicCount )
					return ( LRESULT )FALSE;
				if ( ! ( pTics = GlobalLock ( pthis->hTics ) ) )
					return ( LRESULT )FALSE;
				( ( int *)pTics )[wParam] = lParam;
				GlobalUnlock ( pthis->hTics );
				return ( LRESULT )TRUE;
				break;

			case TBM_SETPOS:
				if ( lParam > pthis->sizRange.cy )
					lParam = pthis->sizRange.cy;
				if ( lParam < pthis->sizRange.cx )
					lParam = pthis->sizRange.cx;
				Trackbar_ISetThumbPos ( pthis, lParam );
				if ( ( BOOL )wParam )
					InvalidateRect ( hWnd, NULL, TRUE );
				break;        

			case TBM_SETRANGE:
				pthis->sizRange.cx = ( int )LOWORD ( lParam );
				pthis->sizRange.cy = ( int )HIWORD ( lParam );
				Trackbar_IResetRange ( pthis );
				if ( ( BOOL )wParam )
					InvalidateRect ( hWnd, NULL, TRUE );
				break;

			case TBM_SETRANGEMIN:
				pthis->sizRange.cx = ( int )lParam;
				Trackbar_IResetRange ( pthis );
				if ( ( BOOL )wParam )
					InvalidateRect ( hWnd, NULL, TRUE );
				break;
			case TBM_SETRANGEMAX:
				pthis->sizRange.cy = ( int )lParam;
				Trackbar_IResetRange ( pthis );
				if ( ( BOOL )wParam )
					InvalidateRect ( hWnd, NULL, TRUE );
				break;
			case TBM_CLEARTICS:
				/*pthis->uStyles ^= TBS_NOTICKS;*/
				GlobalFree ( pthis->hTics );
				pthis->hTics = ( HGLOBAL )NULL;
				pthis->TicCount = 0;
				if ( ( BOOL )wParam )
					InvalidateRect ( hWnd, NULL, TRUE );
				break;
			case TBM_SETSEL:
				pthis->sizSelect.cx = ( int )LOWORD ( lParam );
				pthis->sizSelect.cy = ( int )HIWORD ( lParam );
				if ( ( BOOL )wParam )
					InvalidateRect ( hWnd, NULL, TRUE );
				break;
			case TBM_SETSELSTART:
				pthis->sizSelect.cx = ( int )lParam;
				if ( ( BOOL )wParam )
					InvalidateRect ( hWnd, NULL, TRUE );
				break;
			case TBM_SETSELEND:
				pthis->sizSelect.cy = ( int )lParam;
				if ( ( BOOL )wParam )
					InvalidateRect ( hWnd, NULL, TRUE );
				break;
			case TBM_GETPTICS:
				return ( LRESULT )GlobalLock ( pthis->hTics );
				break;
			case TBM_GETTICPOS:
				if ( ( int )wParam < 0 || ( int )wParam > pthis->TicCount )
					return -1;
				if ( ! ( pTics = GlobalLock ( pthis->hTics ) ) )
					return -1;
				return ( LRESULT )( ( int *)pTics)[wParam];
				break;
			case TBM_GETNUMTICS:
				return ( LRESULT )pthis->TicCount;
				break;
			case TBM_GETSELSTART:
				return ( LRESULT )pthis->sizSelect.cx;
				break;
			case TBM_GETSELEND:
				return ( LRESULT )pthis->sizSelect.cy;
				break;
			case TBM_CLEARSEL:
				pthis->sizSelect.cx = pthis->sizRange.cy = 0;
				if ( ( BOOL )wParam )
					InvalidateRect ( hWnd, NULL, TRUE );
				break;
			/*ignore lParam for now*/
			case TBM_SETTICFREQ:
				Trackbar_IResetTics ( pthis, ( int )wParam );
				break;
			case TBM_SETPAGESIZE:
				pthis->nPageSize = lParam;
				break;
			case TBM_SETLINESIZE:
				pthis->nLineSize = lParam;
				break;

			case TBM_GETLINESIZE:
				return ( LRESULT )pthis->nLineSize;
				break;
			case TBM_GETTHUMBRECT:
				return ( LRESULT )&pthis->rectSlide;
				break;
			case TBM_SETTHUMBLENGTH:
				break;
			case TBM_GETTHUMBLENGTH:
				return ( LRESULT )( pthis->rectSlide.right - pthis->rectSlide.left );
				break;
			case TBM_GETCHANNELRECT:

				break;
			default:
			    return DefWindowProc(hWnd, uMessage, wParam, lParam);
		}
	}

	if ( pthis )
		GlobalUnlock ( hTrackbar );

	if ( pthis )
		GlobalUnlock ( hTrackbar );
	return 0;
}


int WINAPI 
Trackbar_Initialize 
(
	HINSTANCE		hinst
) 
{

	WNDCLASS		wc;
	int			ErrorCode	= 0;

	memset ( &wc, 0, sizeof ( WNDCLASS ) );
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
	wc.lpfnWndProc   = Trackbar_WndProc;
	wc.cbWndExtra    = 0; /*sizeof( TRACKBAR_WNDEXTRABYTES );*/
	wc.hInstance     = hinst;
	wc.hCursor       = LoadCursor( ( HINSTANCE )NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject ( LTGRAY_BRUSH );
	wc.lpszClassName = WC_TRACKBAR;

	if ( ! RegisterClass ( &wc ) )
		ErrorCode = WTB_ERR_REGISTERCLASS;

	return ( ErrorCode );
}

void WINAPI Trackbar_Terminate 
(
	HINSTANCE			hInstance
) 
{
	WNDCLASS			WndClass;


	if ( GetClassInfo ( hInstance, WC_TRACKBAR, &WndClass ) )
		UnregisterClass ( WC_TRACKBAR, hInstance );

}


static int Trackbar_IGetPosFrPoint ( TRACKBARDATA* pTrackbar, int Point )
{
    double      dPos;
	int			Pos = 0;

    if ( pTrackbar->uStyles & TBS_VERT )
    {
		dPos = pTrackbar->sizRange.cx +
		      (double)( Point - pTrackbar->ptPos.y ) * 
		      ( pTrackbar->sizRange.cy - pTrackbar->sizRange.cx ) / 
		      pTrackbar->ptSize.cy;
    }
	else
    {
        dPos = pTrackbar->sizRange.cx +
		      (double)( Point - pTrackbar->ptPos.x ) * 
		      ( pTrackbar->sizRange.cy - pTrackbar->sizRange.cx ) / 
		      pTrackbar->ptSize.cx;
    }

    Pos = ( ceil ( dPos ) < dPos + 0.5 )? ceil ( dPos ) : (int)dPos;

    return Pos;
}


static int Trackbar_IGetPointFrPos ( TRACKBARDATA* pTrackbar, int Pos )
{
	int			pixPoint	= 0;

	if ( pTrackbar->uStyles & TBS_VERT )
	{
		pixPoint = pTrackbar->ptPos.y +
	                   ( Pos - pTrackbar->sizRange.cx ) * 
			    pTrackbar->ptSize.cy / ( pTrackbar->sizRange.cy - pTrackbar->sizRange.cx );
		if ( pixPoint > pTrackbar->ptPos.y + pTrackbar->ptSize.cy )
			pixPoint = pTrackbar->ptPos.y + pTrackbar->ptSize.cy;
		if ( pixPoint < pTrackbar->ptPos.y )
			pixPoint = pTrackbar->ptPos.y;
	}
	else
	{
		pixPoint = pTrackbar->ptPos.x +
		           ( Pos - pTrackbar->sizRange.cx ) * 
			   pTrackbar->ptSize.cx / ( pTrackbar->sizRange.cy - pTrackbar->sizRange.cx );
		if ( pixPoint > pTrackbar->ptPos.x + pTrackbar->ptSize.cx )
			pixPoint = pTrackbar->ptPos.x + pTrackbar->ptSize.cx;
		if ( pixPoint < pTrackbar->ptPos.x )
			pixPoint = pTrackbar->ptPos.x;
	}

	 
	
	return ( pixPoint );
}


static int Trackbar_ISetThumbPos ( TRACKBARDATA* pthis, int NewPos )
{
	int			newPoint;
	pthis->nThumbPos = NewPos;
	newPoint = Trackbar_IGetPointFrPos ( pthis, NewPos );
	if ( pthis->uStyles & TBS_VERT )
	{
		pthis->rectSlide.top = newPoint - TBTHUMB_WIDTHMAX/2;
		pthis->rectSlide.bottom = newPoint + TBTHUMB_WIDTHMAX/2;
	}
	else
	{
		pthis->rectSlide.left = newPoint - TBTHUMB_WIDTHMAX/2;
		pthis->rectSlide.right = newPoint + TBTHUMB_WIDTHMAX/2;
	}

	return 0;

}

static int Trackbar_IResetRange ( TRACKBARDATA* pthis )
{
	if ( pthis->sizRange.cy > pthis->sizRange.cx )
		pthis->Range = pthis->sizRange.cy - pthis->sizRange.cx;
	else
	if ( pthis->sizRange.cy < pthis->sizRange.cx )
		pthis->Range = pthis->sizRange.cx - pthis->sizRange.cy;
	else
		pthis->Range = 1;
	Trackbar_IResetTics ( pthis, pthis->nTickFreq );

	return pthis->Range;
}

/*each member of the array of tick marks contains the position ( relative to the minimum position )
/of a tick*/
static void Trackbar_IResetTics ( TRACKBARDATA* pthis, int newTicFreq )
{
	int				newTicCount;
	int*				pTics = ( int* )NULL;
	newTicCount = pthis->Range / newTicFreq + 1;
	if ( pthis->TicCount == 0 )
		pthis->hTics = GlobalAlloc ( GHND, newTicCount * sizeof ( int ) );
	else
	if ( newTicCount > pthis->TicCount )
		pthis->hTics = GlobalReAlloc ( pthis->hTics, newTicCount * sizeof ( int ),
						GMEM_MOVEABLE | GMEM_ZEROINIT );	

	if ( pthis->hTics )
	{
		pthis->TicCount = newTicCount;
		pthis->nTickFreq = newTicFreq;
		pTics = ( int* )GlobalLock ( pthis->hTics );
		if ( pTics )
		{
			int	i;
			int	dir = pthis->sizRange.cx <= pthis->sizRange.cy ? 1 : -1;

			for ( i = 0; i < newTicCount; i++ )
				pTics [ i ] = pthis->sizRange.cx + dir * newTicFreq * i;
		}
	}
	else
		pthis->TicCount = 0;
}

static int Trackbar_IGetCurPoint ( TRACKBARDATA* pTrackbar )
{
	int		nResult = 0;
	
	if ( pTrackbar )
		if ( pTrackbar->uStyles & TBS_VERT )
			nResult = pTrackbar->rectSlide.top + TBTHUMB_WIDTHMAX/2;
	        else
			nResult = pTrackbar->rectSlide.left + TBTHUMB_WIDTHMAX/2;
	return nResult;
}

static void Trackbar_IMoveThumb ( HWND hWnd, TRACKBARDATA* pthis, int newPos )
{
	RECT*			pRect		= ( RECT* )NULL;
	HDC hDC = GetDC(hWnd);
	
	if ( pthis->uStyles & TBS_NOTHUMB )
		return;

	pRect   = &pthis->rectSlide;

	if ( newPos < pthis->sizRange.cx )
		newPos = pthis->sizRange.cx;
	if ( newPos > pthis->sizRange.cy )
		newPos = pthis->sizRange.cy;
	
	BitBlt ( hDC, pRect->left, pRect->top, 
		pRect->right - pRect->left+2, 
		pRect->bottom - pRect->top+2, pthis->hBkDC, 0, 0, SRCCOPY );
	Trackbar_ISetThumbPos ( pthis, newPos );

	Trackbar_PaintThumb ( hDC, pthis );
	ReleaseDC(hWnd, hDC);

}

static void Trackbar_IDrawSelectTic ( HDC hDC, int x, int y, UINT uFlag )
{
	POINT			Triangle[3];

	Triangle[0].x = x;
	Triangle[0].y = y;
	switch ( uFlag )
	{
		case TBTICK_LEFTWARD:
			Triangle[1].x = x - 3;
			Triangle[1].y = y - 3;
			Triangle[2].x = x + 3;
			Triangle[2].y = y - 3;
			break;
		case TBTICK_RIGHTWARD:
			Triangle[1].x = x + 3;
			Triangle[1].y = y - 3;
			Triangle[2].x = x + 3;
			Triangle[2].y = y + 3;
			break;
		case TBTICK_UPWARD:
			Triangle[1].x = x + 3;
			Triangle[1].y = y - 3;
			Triangle[2].x = x - 3;
			Triangle[2].y = y - 3;
			break;
		case TBTICK_DOWNWARD:
			Triangle[1].x = x + 3;
			Triangle[1].y = y + 3;
			Triangle[2].x = x - 3;
			Triangle[2].y = y + 3;
			break;

	}

	Polygon ( hDC, Triangle, 3 );
}
