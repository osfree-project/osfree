/*  
	WTooltip.c	1.15 
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
#include "WTooltip.h"

#ifndef _WINDOWS
#define	 MoveToEx(hdc,x,y,old)	MoveTo(hdc,x,y)
#define  RegisterClassEx	RegisterClass
#define  GetTextExtentPoint32	GetTextExtentPoint
#define  WNDCLASSEX		WNDCLASS   
#if !defined(WM_CTLCOLORDLG)
#define	 WM_CTLCOLORDLG		WM_CTLCOLOR
#endif
#endif

#define TT_CXPADDING 			10
#define TT_CYPADDING 			8

#define TOOLTIPTEXT_LENGTH		80
#define	TTINITIAL_TIMERID		10
#define	TTAUTOPOP_TIMERID		TTINITIAL_TIMERID+1
#define	TTRESHOW_TIMERID		TTINITIAL_TIMERID+2

static char *g_szToolSubclass = "TooltipSubClass";
static char *g_szToolHandle = "TooltipHandle";

/*iActiveTool and iPointedTool are both index into an list of ToolInfo structure, ToolInfoList.
  They are both initially set to -1 */
static char TOOLTIP_PROP[] = "Tooltip_Data";	
typedef struct {
	BOOL		bActive;
	int		nInitDelay;
	int		nAutoPopDelay;
	int		nReshowDelay;
	HFONT		hFont;
	POINT		ptMouseLoc;
	int		iActiveTool;
	int		iPointedTool;
	BOOL		bTimerSet;
	WLIST		ToolInfoList;
} TOOLTIPDATA;

typedef struct {
	TOOLINFO	ToolInfo;
	char		szText[80];
} WTOOLINFO;




#define GETTHISDATA(hwnd) \
   GetProp(hwnd, TOOLTIP_PROP )
#define SETTHISDATA(hwnd, pthisNew) \
   SetProp(hwnd, TOOLTIP_PROP, pthisNew)


/*prototypes*/
void 
Tooltip_OnTimer ( HWND hWnd, UINT uTimerID );
void
Tooltip_ProcessEvent ( HWND hTooltip, TOOLTIPDATA* pthis, HWND hwReceive, UINT msg );
LRESULT  
Tooltip_TTMWndProc ( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam );
BOOL 
Tooltip_IFindTool ( WLIST* pWList, TOOLINFO* pFindToolInfo, WTOOLINFO** ppActiveTool, int* Index );
void 
Tooltip_IGetToolRect ( TOOLINFO* pTooltip, LPRECT pRect );
BOOL
Tooltip_IRetrieveText ( HWND hwTooltip, WTOOLINFO* pwToolInfo );
void 
ToolInfo_IRemoveSubclass ( TOOLINFO* pTool );
BOOL 
ToolInfo_ISetSubclass ( HWND hwTooltip, TOOLINFO* pTool );
BOOL
Tooltip_IShowActiveTip ( HWND hwTooltip, TOOLTIPDATA* pTooltip, POINT ptMouse );
LRESULT WINAPI 
Tooltip_SubclassProc ( HWND hWnd, UINT	uMessage, WPARAM wParam, LPARAM	lParam );


BOOL 
Tooltip_OnCreate 
(
	HWND				hWnd, 
	CREATESTRUCT*			pcs
) 
{

	HWND				hwndParent	= GetParent(hWnd);
	HDC				hDC		= GetDC	( hwndParent );
	HGLOBAL				hTooltip	= ( HGLOBAL )NULL;
	TOOLTIPDATA*			pthis		= ( TOOLTIPDATA* )NULL;

	int				ErrorCode	= 0;
	/* Allocate memory for a THISDATA structure and store a pointer to it in
	   the window extra bytes.*/
	if ( ! ( hTooltip = GlobalAlloc ( GHND, sizeof ( TOOLTIPDATA ) ) ) )
		ErrorCode = WTT_ERR_GLOBALALLOC;
	else
	if ( ! ( pthis = ( TOOLTIPDATA *) GlobalLock ( hTooltip ) ) )
		ErrorCode = WTT_ERR_GLOBALLOCK;
	else
	{
		pthis->iActiveTool = -1;
		pthis->iPointedTool = -1;
		pthis->nInitDelay = pthis->nReshowDelay = 1000;
		pthis->nAutoPopDelay = 3000;
		pthis->hFont = ( HFONT )GetStockObject ( ANSI_VAR_FONT );
		WLCreate ( &pthis->ToolInfoList, sizeof ( WTOOLINFO ) );
		SETTHISDATA(hWnd, hTooltip);

		
	}

	ReleaseDC ( hwndParent, hDC );
	if ( pthis )
		GlobalUnlock ( hTooltip );
	return ( ErrorCode >= 0 );
}



void Tooltip_OnDestroy ( HWND hWnd )
{
	HGLOBAL			hTooltip	= ( HGLOBAL )NULL;
	TOOLTIPDATA*		pthis		= ( TOOLTIPDATA* )NULL;
	WTOOLINFO*		pToolInfoList	= ( WTOOLINFO* )NULL;
	WTOOLINFO*		pTool		= ( WTOOLINFO* )NULL;
	UINT			Count		= 0;
	int			ErrorCode	= 0;
	int			i;
	if ( ! ( hTooltip = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTT_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLTIPDATA *) GlobalLock ( hTooltip ) ) )
		ErrorCode = WTT_ERR_GLOBALLOCK;
	else
	if ( WLLock ( &pthis->ToolInfoList, ( LPVOID )&pToolInfoList ) )
		ErrorCode = WTT_ERR_LISTLOCK;
	else
	{
		WLCount ( &pthis->ToolInfoList, &Count );
		for ( i = 0; i < Count; i++ )
		{
			pTool = pToolInfoList + i;
			ToolInfo_IRemoveSubclass ( ( TOOLINFO* )pTool );
		}
	}

	if ( pToolInfoList )
		WLUnlock ( &pthis->ToolInfoList, ( LPVOID )&pToolInfoList );
	WLDestroy ( &pthis->ToolInfoList );
	if ( pthis )
		GlobalUnlock ( hTooltip );
	if ( hTooltip )
		GlobalFree ( hTooltip );
}

void Tooltip_OnMouseMove 
(
	HWND			hWnd, 
	int			x, 
	int			y, 
	UINT			keyFlags
) 
{
	HGLOBAL			hTrackbar	= ( HGLOBAL )NULL;
	TOOLTIPDATA*		pthis		= ( TOOLTIPDATA* )NULL;
	int			ErrorCode	= 0;

	if ( ! ( hTrackbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTT_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLTIPDATA *) GlobalLock ( hTrackbar ) ) )
		ErrorCode = WTT_ERR_GLOBALLOCK;
	else
	{
	}
	
	if ( pthis )
		GlobalUnlock ( hTrackbar );
}


void Tooltip_OnPaint 
(
	HWND				hWnd, 
	HDC				hPaintDC 
)
{
	RECT				rect;
	PAINTSTRUCT			ps;
	WTOOLINFO*			pToolInfoList	= ( WTOOLINFO* )NULL;
	WTOOLINFO*			pActiveTool	= ( WTOOLINFO* )NULL;
	HGLOBAL				hTooltip	= ( HGLOBAL )NULL;
	TOOLTIPDATA*			pthis		= ( TOOLTIPDATA* )NULL;
	HBRUSH				hBrush		= CreateSolidBrush(RGB(255, 255, 128));
	int				ErrorCode	= 0;

	
	if ( ! ( hTooltip = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTT_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLTIPDATA *) GlobalLock ( hTooltip ) ) )
		ErrorCode = WTT_ERR_GLOBALLOCK;
	else
	if ( WLLock ( &pthis->ToolInfoList, ( LPVOID )&pToolInfoList ) )
		ErrorCode = WTT_ERR_LISTLOCK;
	else
	if ( pthis->iActiveTool >= 0 )
	{
		pActiveTool = pToolInfoList + pthis->iActiveTool;
		BeginPaint ( hWnd, &ps );
		GetClientRect ( hWnd, &rect );
                OffsetRect ( &rect, 1, 1 );
		pthis->hFont = SelectObject ( ps.hdc, pthis->hFont );
		FillRect ( ps.hdc, &rect, hBrush );
		SetBkMode ( ps.hdc, TRANSPARENT );
		TextOut ( ps.hdc, 3, 3, pActiveTool->szText, strlen ( pActiveTool->szText ) );
		pthis->hFont = SelectObject ( ps.hdc, pthis->hFont );
		EndPaint ( hWnd, &ps );

		

	}

	if ( pToolInfoList )
		WLUnlock ( &pthis->ToolInfoList, ( LPVOID )&pToolInfoList );

      	if ( hBrush )
		DeleteObject ( hBrush );
	if ( pthis )
		GlobalUnlock ( hTooltip );

}

void Tooltip_OnTimer ( HWND hWnd, UINT uTimerID )
{
	POINT				ptMouse;
	HGLOBAL				hTooltip	= ( HGLOBAL )NULL;
	TOOLTIPDATA*			pthis		= ( TOOLTIPDATA* )NULL;
	int				ErrorCode	= 0;

	if ( uTimerID != TTINITIAL_TIMERID &&
	     uTimerID != TTRESHOW_TIMERID &&
	     uTimerID != TTAUTOPOP_TIMERID 
	   ) 
		return;
	
	if ( ! ( hTooltip = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTT_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLTIPDATA *) GlobalLock ( hTooltip ) ) )
		ErrorCode = WTT_ERR_GLOBALLOCK;
	else
	{
		GetCursorPos ( &ptMouse );
		if ( pthis->iActiveTool >= 0 )
		{
			if ( uTimerID == TTAUTOPOP_TIMERID )
			{
				pthis->iActiveTool = -1;
				ShowWindow ( hWnd, SW_HIDE );
				KillTimer ( hWnd, TTAUTOPOP_TIMERID );
				KillTimer ( hWnd, TTINITIAL_TIMERID );
			}
			else
			if ( ptMouse.x != pthis->ptMouseLoc.x ||
			     ptMouse.y != pthis->ptMouseLoc.y 			
			   ) 
			{
				pthis->iActiveTool = -1;
				pthis->iPointedTool = -1;
				ShowWindow ( hWnd, SW_HIDE );
			}
		}
		else
		if ( pthis->iPointedTool >= 0 )
		{
			if ( ptMouse.x == pthis->ptMouseLoc.x &&
			     ptMouse.y == pthis->ptMouseLoc.y 			
			   ) 
			{
				if ( uTimerID == TTINITIAL_TIMERID )
				{
					pthis->iActiveTool = pthis->iPointedTool;
					Tooltip_IShowActiveTip ( hWnd, pthis, ptMouse );
			/* Tooltips should not be on a timer */
					/*SetTimer ( hWnd, TTAUTOPOP_TIMERID, pthis->nAutoPopDelay, NULL );*/
				}
			}
			else
				pthis->iPointedTool = -1;
		}
	}

	if ( pthis )
		GlobalUnlock ( hTooltip );
}

BOOL
Tooltip_IShowActiveTip ( HWND hwTooltip, TOOLTIPDATA* pTooltip, POINT ptMouse )
{
	TOOLTIPTEXT			ttText;
	HDC				hDC		= GetDC ( hwTooltip );
	WTOOLINFO*			pToolInfoList	= ( WTOOLINFO* )NULL;
	WTOOLINFO*			pActiveTool	= ( WTOOLINFO* )NULL;
	int				ErrorCode	= 0;
	SIZE				size;
	static int			SM_CYMenuSize;
	static int			SM_CYScreen;
	static int			SM_CXScreen;

	memset ( &ttText, 0, sizeof ( TOOLTIPTEXT ) );	
	if ( ( int )pTooltip->ToolInfoList.ElementCount <= pTooltip->iActiveTool )
		ErrorCode =  WTT_ERR_OUTOFBOUNDS;
	else
	if ( WLLock ( &pTooltip->ToolInfoList, ( LPVOID )&pToolInfoList ) )
		ErrorCode = WTT_ERR_LISTLOCK;
	else
	{
		int	ntxtLen = 0;
		pActiveTool = pToolInfoList + pTooltip->iActiveTool;
		Tooltip_IRetrieveText ( hwTooltip, pActiveTool );

		if ( ( ntxtLen = strlen ( pActiveTool->szText ) ) )
		{
			pTooltip->hFont = SelectObject ( hDC, pTooltip->hFont );
			GetTextExtentPoint32 ( hDC, pActiveTool->szText, 
						ntxtLen, &size ); 
			pTooltip->hFont = SelectObject ( hDC, pTooltip->hFont );
			
			size.cx += TT_CXPADDING;
			size.cy += TT_CYPADDING;
			SM_CYMenuSize   = GetSystemMetrics ( SM_CYMENU ) / 2;
			SM_CYScreen	= GetSystemMetrics ( SM_CYSCREEN );
			SM_CXScreen	= GetSystemMetrics ( SM_CXSCREEN );

			if ( ptMouse.y + size.cy + SM_CYMenuSize < SM_CYScreen )
				ptMouse.y += SM_CYMenuSize;
			else
				ptMouse.y -= SM_CYMenuSize;

			if ( ptMouse.x - size.cx/2 < 0 )
				ptMouse.x = 0;
			else
			if ( ptMouse.x + size.cx > SM_CXScreen )
				ptMouse.x = SM_CXScreen - size.cx;
			else
				ptMouse.x -= size.cx/2;

#if 1
			SetWindowPos ( hwTooltip, HWND_TOP, ptMouse.x, ptMouse.y, 
			     size.cx, size.cy, SWP_SHOWWINDOW| SWP_NOACTIVATE );
#else
			MoveWindow ( hwTooltip, ptMouse.x, ptMouse.y, 
			     size.cx, size.cy, TRUE );
			BringWindowToTop ( hwTooltip );
#endif
			UpdateWindow( hwTooltip );
		}
	}

	if ( pToolInfoList )
		WLUnlock ( &pTooltip->ToolInfoList, ( LPVOID )&pToolInfoList );

	ReleaseDC(hwTooltip,hDC);

	return ( ErrorCode >= 0 );

}

HWND
Tooltip_WindowFromPoint ( HWND hTooltip, TOOLTIPDATA* pTooltip, POINT* ppt )
{
	return WindowFromPoint ( *ppt ); 
}


BOOL 
Tooltip_IIsPointInTool ( TOOLTIPDATA* pTooltip, int nIndex, HWND hwMouse, POINT ptMouse )
{
	WTOOLINFO*			pToolInfoList	= ( WTOOLINFO* )NULL;
	WTOOLINFO*			pActiveTool	= ( WTOOLINFO* )NULL;
	TOOLINFO*			pActiveToolInfo	= ( TOOLINFO* )NULL;
	BOOL				bReturn		= FALSE;
	int				ErrorCode	= 0;
	
	if ( ( int )pTooltip->ToolInfoList.ElementCount <= nIndex )
		return	FALSE;
	if ( WLLock ( &pTooltip->ToolInfoList, ( LPVOID )&pToolInfoList ) )
		ErrorCode = WTT_ERR_LISTLOCK;
	else
	{
		pActiveTool = pToolInfoList + nIndex;
		pActiveToolInfo = &pActiveTool->ToolInfo;
        	ScreenToClient ( pActiveToolInfo->hwnd, &ptMouse );
        	if ( pActiveToolInfo->uFlags &  TTF_IDISHWND )        
        	{
            		GetWindowRect ( ( HWND )pActiveToolInfo->uId, &pActiveToolInfo->rect );
            		MapWindowPoints ( ( HWND )NULL, pActiveToolInfo->hwnd, 
                              (LPPOINT)&pActiveToolInfo->rect, 2 );
        	}

        	if ( PtInRect ( &pActiveToolInfo->rect, ptMouse ) )
			bReturn = TRUE;
	}

	if ( pToolInfoList )
		WLUnlock ( &pTooltip->ToolInfoList, ( LPVOID )&pToolInfoList );

	return bReturn;
}

void 
Tooltip_IGetToolRect ( TOOLINFO* pTooltip, LPRECT pRect )
{

	if ( pTooltip->uFlags & TTF_IDISHWND ) 
	{
		if ( IsWindow ( ( HWND )pTooltip->uId ) )
			GetWindowRect ( ( HWND )pTooltip->uId, pRect );			
	}
	else
		GetWindowRect ( GetDlgItem ( pTooltip->hwnd, pTooltip->uId ), pRect );
}

void
Tooltip_RelayEvent ( HWND hTooltip, TOOLTIPDATA* pthis, MSG* msg)
{
  
	if ( pthis->bActive ) 
	{
	
		/* Filter messages and pass on interesting ones...*/

		switch ( msg->message ) 
		{
			case WM_MOUSEMOVE:
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP: 
			  /* Process the event*/
			  Tooltip_ProcessEvent ( hTooltip, pthis, msg->hwnd, msg->message );
			  break;
		}
	}
}

void
Tooltip_SetDelayTime ( HWND hwTooltip, TOOLTIPDATA* pthis, UINT uFlags, int Delay)
{
	if ( uFlags == TTDT_AUTOMATIC )
		pthis->nInitDelay = pthis->nAutoPopDelay = pthis->nReshowDelay = Delay;
	else
	if ( uFlags == TTDT_AUTOPOP )
		pthis->nAutoPopDelay = Delay;
	else
	if ( uFlags == TTDT_RESHOW )
		pthis->nReshowDelay = Delay;
	else
		pthis->nInitDelay = Delay;
	if ( pthis->bTimerSet )
	{
		SetTimer ( hwTooltip, TTINITIAL_TIMERID, pthis->nInitDelay, NULL );
	}
}

BOOL
Tooltip_AddTool 
( 
	HWND			hwTooltip,
	TOOLTIPDATA*		pthis, 
	TOOLINFO*		pToolInfo
)		  
{
	WTOOLINFO		WToolInfo;
	UINT			Count		= 0;
	int			ErrorCode	= 0;
	BOOL			fOk		= TRUE;

	memset ( &WToolInfo, 0, sizeof ( WTOOLINFO ) );
	memcpy ( &WToolInfo, pToolInfo, sizeof ( TOOLINFO ) );
	if ( ( !pToolInfo->hinst ) &&
	     ( pToolInfo->lpszText ) &&	
	     ( pToolInfo->lpszText != LPSTR_TEXTCALLBACK ) 
	   )
	{
		memcpy ( WToolInfo.szText, pToolInfo->lpszText, TOOLTIPTEXT_LENGTH );
		WToolInfo.ToolInfo.lpszText = WToolInfo.szText;
	}
	WLCount ( &pthis->ToolInfoList, &Count );
	if ( WLInsert ( &pthis->ToolInfoList, Count, &WToolInfo ) < 0 )
		ErrorCode = WTT_ERR_LISTINSERT;
	else
	if ( pToolInfo->uFlags & TTF_SUBCLASS )
	{
#if 1
		fOk = ToolInfo_ISetSubclass ( hwTooltip, pToolInfo );
#else
		hwAdded = pToolInfo->uFlags & TTF_IDISHWND ? ( HWND )pToolInfo->uId : 
				GetDlgItem ( pToolInfo->hwnd, pToolInfo->uId );
		fOk = SetProp ( hwAdded, g_szToolSubclass, ( HANDLE )(DWORD ) 
			SetWindowLong ( hwAdded, GWL_WNDPROC, 
					( LONG )Tooltip_SubclassProc ) );
		fOk = SetProp ( hwAdded, g_szToolHandle, hwTooltip ); 
#endif
	}
	
	return ( ErrorCode >= 0 && fOk);
}


void
Tooltip_DeleteTool 
( 
	TOOLTIPDATA*			pthis, 
	TOOLINFO*			pDelToolInfo
)		  
{
	int				i;
	WTOOLINFO*			pDelTool	= ( WTOOLINFO* )NULL;

	if ( Tooltip_IFindTool ( &pthis->ToolInfoList, pDelToolInfo, &pDelTool, &i ) )
		WLDelete ( &pthis->ToolInfoList, i );
	
}

BOOL
Tooltip_GetTool
( 
	TOOLTIPDATA*			pthis, 
	UINT				uToolIndex,
	TOOLINFO*			pToolInfo			
)		  
{
	UINT				Count		= 0;
	int				ErrorCode	= 0;
	WTOOLINFO*			pToolInfoList	= ( WTOOLINFO* )NULL;
	WTOOLINFO*			pActiveTool	= ( WTOOLINFO* )NULL;
	BOOL				Result		= FALSE;

	if ( WLCount ( &pthis->ToolInfoList, &Count ) || Count == 0 )
		return FALSE;
	if ( uToolIndex >= ( UINT )Count )
		ErrorCode = WTT_ERR_OUTOFBOUNDS;
	else
	if ( WLLock ( &pthis->ToolInfoList, ( LPVOID )&pToolInfoList ) )
		ErrorCode = WTT_ERR_LISTLOCK;
	else
	{
		if ( uToolIndex == -1 )
			uToolIndex = pthis->iActiveTool >= 0 ? pthis->iActiveTool :
				     pthis->iPointedTool >= 0 ? pthis->iPointedTool : -1;
		
		if ( uToolIndex != -1 )
		{
			pActiveTool = pToolInfoList + uToolIndex;
			memcpy ( pToolInfo, &pActiveTool->ToolInfo,
					sizeof ( TOOLINFO ) );
			Result = TRUE;
		}
	}

	if ( pToolInfoList )
		WLUnlock ( &pthis->ToolInfoList, ( LPVOID )&pToolInfoList );

	return Result;
	
}

int
Tooltip_GetToolCount
( 
	TOOLTIPDATA*			pthis
)		  
{
	UINT				Count		= 0;

	WLCount ( &pthis->ToolInfoList, &Count ); 
	return Count;	
}



void
Tooltip_GetText
( 
	HWND				hwTooltip,
	TOOLTIPDATA*			pthis, 
	TOOLINFO*			pToolInfo
)		  
{
	int				i;
	WTOOLINFO*			pMatchTool	= ( WTOOLINFO* )NULL;

	if ( Tooltip_IFindTool ( &pthis->ToolInfoList, pToolInfo, &pMatchTool, &i ) )
	{
		Tooltip_IRetrieveText ( hwTooltip, pMatchTool );
		pToolInfo->lpszText = pMatchTool->szText;
	}
}

BOOL
Tooltip_IRetrieveText 
( 
	HWND				hwTooltip,
	WTOOLINFO*			pwToolInfo 
)
{
	TOOLTIPTEXT			ttText;
	UINT				strResId	= 0;
	char*				tipText		= 0;
	HINSTANCE			hInst		= ( HINSTANCE )NULL;
	TOOLINFO*			pToolInfo	= &pwToolInfo->ToolInfo;


	memset ( &ttText, 0, sizeof ( TOOLTIPTEXT ) );	
	if ( pToolInfo->lpszText == LPSTR_TEXTCALLBACK ) 
	{
		ttText.hdr.hwndFrom = hwTooltip;
		ttText.hdr.idFrom = pToolInfo->uId;
		ttText.hdr.code = TTN_NEEDTEXT;
		ttText.uFlags = pToolInfo->uFlags;
		/* lpszText should always point to a valid buffer. The application
		   is then free to either assign a new buffer to or copy text into 
		   buffer pointed to by it*/
		ttText.lpszText = ttText.szText;
		SendMessage ( pToolInfo->hwnd, WM_NOTIFY, ( WPARAM )pToolInfo->uId,
				( LPARAM )&ttText );
		
		if ( ttText.hinst && ttText.lpszText ) 
		{
			hInst = ttText.hinst;
			strResId = LOWORD ( ttText.lpszText );
		}
		else 
		if ( ttText.szText[0] )
			tipText = ttText.szText;
		else 
		if ( ttText.lpszText ) 
			tipText = ttText.lpszText;

	}
	else
	if ( pToolInfo->hinst && pToolInfo->lpszText )
	{
		hInst = pToolInfo->hinst;
		strResId = LOWORD ( pToolInfo->lpszText );

	}

	if ( hInst && strResId ) 
	{
		if ( LoadString ( hInst, strResId, ttText.szText, sizeof ( ttText.szText ) ) )
			tipText = ttText.szText;
	}
	if ( !tipText && pwToolInfo->szText[0] == '\0' )
		return FALSE;
	if ( tipText )
		memcpy ( pwToolInfo->szText, tipText, TOOLTIPTEXT_LENGTH );
	
	return TRUE;
}

BOOL
Tooltip_GetToolInfo
( 
	TOOLTIPDATA*			pthis, 
	TOOLINFO*			pToolInfo
)		  
{
	int				i;
	WTOOLINFO*			pDesiredTool	= ( WTOOLINFO* )NULL;
	BOOL				Result		= FALSE;
#if 1
	if ( Tooltip_IFindTool ( &pthis->ToolInfoList, pToolInfo, &pDesiredTool, &i ) )
	{
		memcpy ( pToolInfo, &pDesiredTool->ToolInfo, sizeof ( TOOLINFO ) );
		Result = TRUE;
	}

	return Result;

#else
	if ( WLCount ( &pthis->ToolInfoList, &Count ) || Count == 0 )
		return;
	if ( WLLock ( &pthis->ToolInfoList, ( LPVOID )&pToolInfoList ) )
		ErrorCode = WTT_ERR_LISTLOCK;
	else
	{
		for ( i = 0; i < Count; i++ )
		{
			pActiveTool = pToolInfoList + i;
			if ( pToolInfo->hwnd == pActiveTool->ToolInfo.hwnd &&
			     pToolInfo->uId == pActiveTool->ToolInfo.uId	
			   )
			{
				memcpy ( pToolInfo, &pActiveTool->ToolInfo, sizeof ( TOOLINFO ) );
				Result = TRUE;
				break;
			}
		}

	}

	if ( pToolInfoList )
		WLUnlock ( &pthis->ToolInfoList, ( LPVOID )&pToolInfoList );
	return Result;
#endif
	
}


BOOL
Tooltip_HitTest
( 
	TOOLTIPDATA*			pthis, 
	TTHITTESTINFO*			pToolHitTest
)		  
{
	int				i;
	UINT				Count		= 0;
	int				ErrorCode	= 0;
	WTOOLINFO*			pToolInfoList	= ( WTOOLINFO* )NULL;
	WTOOLINFO*			pActiveTool	= ( WTOOLINFO* )NULL;
	BOOL				Result		= FALSE;

	if ( WLCount ( &pthis->ToolInfoList, &Count ) || Count == 0 )
		return FALSE;
	if ( WLLock ( &pthis->ToolInfoList, ( LPVOID )&pToolInfoList ) )
		ErrorCode = WTT_ERR_LISTLOCK;
	else
	{
		for ( i = 0; i < Count; i++ )
		{
			pActiveTool = pToolInfoList + i;
			if ( pToolHitTest->hwnd == pActiveTool->ToolInfo.hwnd &&
			     pToolHitTest->pt.x > pActiveTool->ToolInfo.rect.left &&
			     pToolHitTest->pt.x < pActiveTool->ToolInfo.rect.right &&
			     pToolHitTest->pt.y > pActiveTool->ToolInfo.rect.top &&
			     pToolHitTest->pt.y < pActiveTool->ToolInfo.rect.bottom 
			   )
			{
				memcpy ( &pToolHitTest->ti, &pActiveTool->ToolInfo,
						sizeof ( TTHITTESTINFO ) );
				Result = TRUE;
				break;
			}
		}

	}

	if ( pToolInfoList )
		WLUnlock ( &pthis->ToolInfoList, ( LPVOID )&pToolInfoList );

	return Result;
	
}



void
Tooltip_NewToolRect
( 
	TOOLTIPDATA*			pthis, 
	TOOLINFO*			pToolInfo
)		  
{
	int				i;
	WTOOLINFO*			pDesiredTool	= ( WTOOLINFO* )NULL;

#if 1
	if ( Tooltip_IFindTool ( &pthis->ToolInfoList, pToolInfo, &pDesiredTool, &i ) )
		SetRect ( &pDesiredTool->ToolInfo.rect, 
			  pToolInfo->rect.left,	pToolInfo->rect.right,
			  pToolInfo->rect.top, pToolInfo->rect.bottom );

#else

	if ( WLCount ( &pthis->ToolInfoList, &Count ) || Count == 0 )
		return;
	if ( WLLock ( &pthis->ToolInfoList, ( LPVOID )&pToolInfoList ) )
		ErrorCode = WTT_ERR_LISTLOCK;
	else
	{
		for ( i = 0; i < Count; i++ )
		{
			pActiveTool = pToolInfoList + i;
			if ( pToolInfo->hwnd == pActiveTool->ToolInfo.hwnd &&
			     pToolInfo->uId == pActiveTool->ToolInfo.uId	
			   )
			{
				SetRect ( &pActiveTool->ToolInfo.rect, 
					  pToolInfo->rect.left,	pToolInfo->rect.right,
					  pToolInfo->rect.top, pToolInfo->rect.bottom );
				break;
			}
		}

	}

	if ( pToolInfoList )
		WLUnlock ( &pthis->ToolInfoList, ( LPVOID )&pToolInfoList );
#endif
	
}

BOOL 
ToolInfo_ISetSubclass
(
	HWND				hwTooltip,
	TOOLINFO*			pTool
)
{
	HWND				hwCtrl		= ( HWND )NULL;
	UINT				Flags		= pTool->uFlags;
	BOOL				fOk		= TRUE;

	if ( Flags & TTF_SUBCLASS )
	{
		hwCtrl = Flags & TTF_IDISHWND ? ( HWND )pTool->uId:
				GetDlgItem ( pTool->hwnd, pTool->uId );
		if ( ( fOk = SetProp ( hwCtrl, g_szToolSubclass, ( HANDLE )(DWORD ) 
				SetWindowLong ( hwCtrl, GWL_WNDPROC, 
						( LONG )Tooltip_SubclassProc ) ) )
		   )
			fOk = SetProp ( hwCtrl, g_szToolHandle, hwTooltip ); 
	}

	return ( fOk );

}


void 
ToolInfo_IRemoveSubclass
(
	TOOLINFO*			pTool
)
{
	HWND				hwCtrl		= ( HWND )NULL;
	UINT				Flags		= pTool->uFlags;

	if ( Flags & TTF_SUBCLASS )
	{
		hwCtrl = Flags & TTF_IDISHWND ? ( HWND )pTool->uId:
			GetDlgItem ( pTool->hwnd, pTool->uId );
		SetWindowLong ( hwCtrl, GWL_WNDPROC, 
				( LPARAM )( WNDPROC )GetProp ( hwCtrl, g_szToolSubclass ) );
		RemoveProp ( hwCtrl, g_szToolSubclass );
		RemoveProp ( hwCtrl, g_szToolHandle );
	}

}

void
Tooltip_SetToolInfo
( 
	HWND				hwTooltip,
	TOOLTIPDATA*			pthis, 
	TOOLINFO*			pToolInfo
)		  
{
	int				i;
	TOOLINFO			SearchTool;
	WTOOLINFO*			pDesiredTool	= ( WTOOLINFO* )NULL;

	memset ( &SearchTool, 0, sizeof ( TOOLINFO ) );
	SearchTool.hwnd = pToolInfo->hwnd;
	SearchTool.uId = pToolInfo->uFlags & TTF_IDISHWND ?
		GetDlgCtrlID ( ( HWND )pToolInfo->uId ) : pToolInfo->uId;
	if ( Tooltip_IFindTool ( &pthis->ToolInfoList, &SearchTool, &pDesiredTool, &i ) )
	{
		if ( pDesiredTool->ToolInfo.uFlags & TTF_SUBCLASS )
			ToolInfo_IRemoveSubclass ( &pDesiredTool->ToolInfo );
		memcpy ( &pDesiredTool->ToolInfo, pToolInfo, sizeof ( TOOLINFO ) );
		if ( !pToolInfo->hinst && pToolInfo->lpszText != LPSTR_TEXTCALLBACK )
		{
			memcpy ( pDesiredTool->szText, pToolInfo->lpszText, TOOLTIPTEXT_LENGTH ); 
			pDesiredTool->ToolInfo.lpszText = pDesiredTool->szText;
		}
		if ( pDesiredTool->ToolInfo.uFlags & TTF_SUBCLASS )
			ToolInfo_ISetSubclass ( hwTooltip, &pDesiredTool->ToolInfo );

	}
	else
		Tooltip_AddTool ( hwTooltip, pthis, pToolInfo );

}


void
Tooltip_UpdateTipText
( 
	TOOLTIPDATA*			pthis, 
	TOOLINFO*			pToolInfo
)		  
{
	int				i;
	WTOOLINFO*			pDesiredTool	= ( WTOOLINFO* )NULL;

#if 1
	if ( Tooltip_IFindTool ( &pthis->ToolInfoList, pToolInfo, &pDesiredTool, &i ) )
	{
		pDesiredTool->ToolInfo.lpszText = pToolInfo->lpszText;
		pDesiredTool->ToolInfo.hinst = pToolInfo->hinst;
		if ( !pToolInfo->hinst && pToolInfo->lpszText != LPSTR_TEXTCALLBACK )
		{
			memcpy ( pDesiredTool->szText, pToolInfo->lpszText, TOOLTIPTEXT_LENGTH ); 
			pDesiredTool->ToolInfo.lpszText = pDesiredTool->szText;
		}
	}

#else
	if ( WLCount ( &pthis->ToolInfoList, &Count ) || Count == 0 )
		return;
	if ( WLLock ( &pthis->ToolInfoList, ( LPVOID )&pToolInfoList ) )
		ErrorCode = WTT_ERR_LISTLOCK;
	else
	{
		for ( i = 0; i < Count; i++ )
		{
			pActiveTool = pToolInfoList + i;
			if ( pToolInfo->hwnd == pActiveTool->ToolInfo.hwnd &&
			     pToolInfo->uId == pActiveTool->ToolInfo.uId	
			   )
			{
				pActiveTool->ToolInfo.lpszText = pToolInfo->lpszText;
				pActiveTool->ToolInfo.hinst = pToolInfo->hinst;
				if ( !pToolInfo->hinst && pToolInfo->lpszText != LPSTR_TEXTCALLBACK )
				{
					memcpy ( pActiveTool->szText, pToolInfo->lpszText, TOOLTIPTEXT_LENGTH ); 
					pActiveTool->ToolInfo.lpszText = pActiveTool->szText;
				}
				break;
			}
		}

	}

	if ( pToolInfoList )
		WLUnlock ( &pthis->ToolInfoList, ( LPVOID )&pToolInfoList );
#endif	
}


void
Tooltip_ProcessEvent 
( 
	HWND			hwTooltip, 
	TOOLTIPDATA*		pthis, 
	HWND			hwReceive, 
	UINT			msg
)
{

	POINT			pt; 
	int			i;
	UINT			Count;
	int			nTimer = 0;
	
	GetCursorPos ( &pt );
	if ( pthis->iActiveTool >= 0 ) /*&& msg == WM_MOUSEMOVE  ) */
	{
		if ( pt.x != pthis->ptMouseLoc.x ||
		     pt.y != pthis->ptMouseLoc.y 			
		   ) 
		{
			pthis->iActiveTool = -1;
			ShowWindow ( hwTooltip, SW_HIDE );
			KillTimer ( hwTooltip, TTINITIAL_TIMERID );
		}
	}
	else
	if ( pthis->iActiveTool < 0 ) /*&& msg == WM_MOUSEMOVE ) */
	{
		if ( pthis->iPointedTool < 0 || 
		     pt.x != pthis->ptMouseLoc.x ||
		     pt.y != pthis->ptMouseLoc.y 
		   ) 
		{

			pthis->iPointedTool = -1;
			KillTimer( hwTooltip, TTINITIAL_TIMERID );

			/* Scan for a new tool, if found, restart the timer.*/
			
			WLCount ( &pthis->ToolInfoList, &Count );
			for ( i = 0; i < Count; i++ ) 
			{
				if ( Tooltip_IIsPointInTool ( pthis, i, hwReceive, pt ) )
				{
					pthis->iPointedTool = i;
					nTimer = SetTimer ( hwTooltip, TTINITIAL_TIMERID, pthis->nInitDelay, NULL );
					break;
				}
			}
		}
	}

	/*if ( pthis->iActiveTool < 0 && pthis->iPointedTool >= 0 )*/
	GetCursorPos ( &pthis->ptMouseLoc );

	if ( pthis->iActiveTool < 0 && pthis->iPointedTool < 0 )
		KillTimer ( hwTooltip, TTINITIAL_TIMERID );
}

BOOL Tooltip_SetToolbarItemID ( HWND hwTooltip, HWND hwToolbar, UINT newID, LPRECT pnewRect ) 
{
	UINT			Count		= 0;
	TOOLTIPDATA*		pthis		= ( TOOLTIPDATA* )NULL;
	WTOOLINFO*		pList		= ( WTOOLINFO * )NULL;
	HGLOBAL			hTooltip	= ( HGLOBAL )NULL;
	int			ErrorCode	= 0;

	if ( ! ( hTooltip = GETTHISDATA ( hwTooltip ) ) )
		ErrorCode = WTT_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLTIPDATA *) GlobalLock ( hTooltip ) ) )
		ErrorCode = WTT_ERR_GLOBALLOCK;
	else
	if ( WLCount ( &pthis->ToolInfoList, &Count ) || Count <= 0 )
		ErrorCode =  WTT_ERR_GENERAL;
	else
	if ( WLLock ( &pthis->ToolInfoList, ( LPVOID )&pList ) )
		ErrorCode = WTT_ERR_LISTLOCK;
	else
	{
		int i;
		BOOL bFound	= FALSE;
		WTOOLINFO*	pNextTool;
		for ( i = 0; i < Count; i++ )
		{
			pNextTool = pList + i;
			if ( pNextTool->ToolInfo.hwnd == hwToolbar )
			{
				bFound = TRUE;
				break;
			}
		}

		if ( bFound )
        {
			pNextTool->ToolInfo.uId = newID;
            pNextTool->ToolInfo.rect = *pnewRect;
        }
	}

	if ( pList )
		WLUnlock ( &pthis->ToolInfoList, ( LPVOID )NULL );
	if ( pthis )
		GlobalUnlock ( hTooltip );
	return ErrorCode >= 0;
}
BOOL Tooltip_IFindTool ( WLIST* pWList, TOOLINFO* pFindToolInfo, WTOOLINFO** ppActiveTool, int* Index )
{
	int				i;
	UINT				uToolId;
	WTOOLINFO*			pToolInfoList	= ( WTOOLINFO* )NULL;
	WTOOLINFO*			pNextTool	= ( WTOOLINFO* )NULL;
	int				Found		= FALSE;
	UINT				Count		= 0;
	int				ErrorCode	= 0;
	
	if ( WLCount ( pWList, &Count ) || Count == 0 )
		ErrorCode = WTT_ERR_GENERAL;
	else
	if ( WLLock ( pWList, ( LPVOID )&pToolInfoList ) )
		ErrorCode = WTT_ERR_LISTLOCK;
	else
	{

		*Index = -1;
		for ( i = 0; i < Count; i++ )
		{
			pNextTool = pToolInfoList + i;
			uToolId = pNextTool->ToolInfo.uId;
			if ( pNextTool->ToolInfo.uFlags & TTF_IDISHWND )
				uToolId = GetDlgCtrlID ( ( HWND )pNextTool->ToolInfo.uId ); 
			if ( pFindToolInfo->hwnd == pNextTool->ToolInfo.hwnd &&
			     pFindToolInfo->uId == uToolId	
			   )
			{
				Found = TRUE;
				*ppActiveTool = pNextTool;
				*Index = i;
				break;		  
			}
			
		}
	}

	if ( pToolInfoList )
		WLUnlock ( pWList, ( LPVOID )&pToolInfoList );

	return Found;
}


LRESULT WINAPI 
Tooltip_SubclassProc
(
	HWND			hWnd,
        UINT			uMessage,
        WPARAM			wParam,
        LPARAM			lParam
)
{

	TOOLTIPDATA*		pTooltipData	= ( TOOLTIPDATA* )NULL;
	HGLOBAL			hTooltip	= ( HGLOBAL )NULL;
	HWND			hwTooltip	= GetProp ( hWnd, g_szToolHandle );
	int			ErrorCode	= 0;

	if ( !hwTooltip )
		ErrorCode = WTT_ERR_NULLPOINTER;
	else
	if ( ! ( hTooltip = GETTHISDATA ( hwTooltip ) ) )
		ErrorCode = WTT_ERR_GETWINPROP;
	else
	if ( ! ( pTooltipData = ( TOOLTIPDATA *) GlobalLock ( hTooltip ) ) )
		ErrorCode = WTT_ERR_GLOBALLOCK;
	else
	switch (uMessage)
	{
		case WM_MOUSEMOVE:
			Tooltip_ProcessEvent ( hTooltip, pTooltipData, hWnd, uMessage );
			break;
	}

	if ( pTooltipData )
		GlobalUnlock ( hTooltip );
	return CallWindowProc((FARPROC)(DWORD)GetProp(hWnd,g_szToolSubclass),
			      hWnd, uMessage, wParam, lParam );
}


LRESULT WINAPI 
Tooltip_WndProc
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
			if ( ! Tooltip_OnCreate ( hWnd, ( LPCREATESTRUCT )lParam ) )
				return ( LRESULT ) -1L;
			break;
		case WM_DESTROY:
			Tooltip_OnDestroy ( hWnd );
			break;

		case WM_GETFONT:
			break;

		case WM_MOUSEMOVE:
			Tooltip_OnMouseMove ( hWnd,(int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )( wParam ) );
			break;

		case WM_PAINT:
			Tooltip_OnPaint ( hWnd, ( HDC )wParam );
			break;
		case WM_SETFONT:
			break;
		case WM_TIMER:
			Tooltip_OnTimer ( hWnd, ( UINT )wParam );
			break;
		case TTM_ADDTOOL:
		case TTM_DELTOOL:
		case TTM_NEWTOOLRECT:
		case TTM_RELAYEVENT:
		case TTM_SETDELAYTIME:
		case TTM_GETTEXT:
		case TTM_UPDATETIPTEXT:
		case TTM_HITTEST:
		case TTM_WINDOWFROMPOINT:
		case TTM_ENUMTOOLS:
		case TTM_GETCURRENTTOOL:
		case TTM_GETTOOLCOUNT:
		case TTM_GETTOOLINFO:
		case TTM_SETTOOLINFO:
		case TTM_ACTIVATE:
			return Tooltip_TTMWndProc(hWnd, uMessage, wParam, lParam);
		
		default:
		    return DefWindowProc(hWnd, uMessage, wParam, lParam);
	}
    
	    return 0;
}

LRESULT  
Tooltip_TTMWndProc
(
	HWND			hWnd,
        UINT			uMessage,
        WPARAM			wParam,
        LPARAM			lParam
)
{
	HGLOBAL			hTooltip	= ( HGLOBAL )NULL;
	TOOLTIPDATA*		pthis		= ( TOOLTIPDATA* )NULL;
	LRESULT			lResult		= 0L;
	int			ErrorCode	= 0;

	if ( ! ( hTooltip = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTT_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLTIPDATA *) GlobalLock ( hTooltip ) ) )
		ErrorCode = WTT_ERR_GLOBALLOCK;
	else
	{
		switch (uMessage)
		{
			case TTM_ACTIVATE:
				pthis->bActive =  ( BOOL )wParam;
				break;
			case TTM_ADDTOOL:
				lResult = Tooltip_AddTool ( hWnd, pthis, ( LPTOOLINFO )lParam );
				break;
			case TTM_DELTOOL:
				Tooltip_DeleteTool ( pthis, ( LPTOOLINFO )lParam );
				break;
			case TTM_NEWTOOLRECT:
				Tooltip_NewToolRect ( pthis, ( LPTOOLINFO )lParam );
				break;
			case TTM_RELAYEVENT:
				Tooltip_RelayEvent ( hWnd, pthis, ( LPMSG )lParam );
				break;
			case TTM_SETDELAYTIME:
				Tooltip_SetDelayTime ( hWnd, pthis, 
							( UINT )wParam, ( int )lParam );
				break;

			case TTM_GETTEXT:
				Tooltip_GetText ( hWnd, pthis, ( LPTOOLINFO )lParam );
				break;        

			case TTM_UPDATETIPTEXT:
				Tooltip_UpdateTipText ( pthis, ( LPTOOLINFO )lParam );
				break;

			case TTM_HITTEST:
				lResult = Tooltip_HitTest ( pthis, ( LPHITTESTINFO )lParam );
				break;
			case TTM_WINDOWFROMPOINT:
				lResult = ( LRESULT )Tooltip_WindowFromPoint ( hWnd, pthis, ( POINT* )lParam ); 
				break;
			case TTM_GETCURRENTTOOL:
				wParam = ( WPARAM )-1;
			case TTM_ENUMTOOLS:
				lResult = Tooltip_GetTool ( pthis, 
							( UINT )wParam, ( LPTOOLINFO )lParam );
				break;
			case TTM_GETTOOLCOUNT:
				lResult = Tooltip_GetToolCount ( pthis );
				break;
			case TTM_GETTOOLINFO:
				lResult = Tooltip_GetToolInfo ( pthis, ( LPTOOLINFO )lParam );
				break;
			case TTM_SETTOOLINFO:
				Tooltip_SetToolInfo ( hWnd, pthis, ( LPTOOLINFO )lParam );
				break;
/*			default:
			    return DefWindowProc(hWnd, uMessage, wParam, lParam);*/
		}
	}
	
	if ( pthis )
		GlobalUnlock ( hTooltip );
	return lResult;

}

/*Note: on Windows it's working fine whether the style is set with CS_CLASSDC.
  But on UNIX, a non-CS_CLASSDC tooltip would exhaust the DC cache very soon
  inside BeginPaint(hWnd,&ps), which would call GetDCEx(), which in turn would
  call GdiCreateHDC() if tooltip is not CS_CLASSDC styled. */
int WINAPI Tooltip_Initialize 
(
	HINSTANCE		hinst
) 
{

	WNDCLASS		wc;
	int			ErrorCode	= 0;

	memset ( &wc, 0, sizeof ( WNDCLASS ) );
	wc.style         = CS_SAVEBITS | CS_HREDRAW | CS_CLASSDC | CS_GLOBALCLASS;
	wc.lpfnWndProc   = Tooltip_WndProc;
	wc.cbWndExtra    = 0; /*sizeof( TRACKBAR_WNDEXTRABYTES );*/
	wc.hInstance     = hinst;
	wc.hCursor       = LoadCursor( ( HINSTANCE )NULL, IDC_ARROW);
	/*   wc.hbrBackground = GetStockObject ( LTGRAY_BRUSH );*/
	wc.lpszClassName = WC_TOOLTIP;

	if ( ! RegisterClass ( &wc ) )
		ErrorCode = WTT_ERR_REGISTERCLASS;

	return ( ErrorCode );
}

void WINAPI 
Tooltip_Terminate 
( 
	HINSTANCE			hInstance 
) 
{
	WNDCLASS			WndClass;


	if ( GetClassInfo ( hInstance, WC_TOOLTIP, &WndClass ) )
		UnregisterClass ( WC_TOOLTIP, hInstance );
}

