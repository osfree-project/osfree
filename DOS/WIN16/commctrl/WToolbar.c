/*  
	WToolbar.c	1.17 
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
#include "WToolbar.h"
#include "WLists.h"
#include "WTooltip.h"
#include "WImgList.h"
#include "WResourc.h"
#include "WTlbCust.h"

#ifndef _WINDOWS
#define  GetTextExtentPoint32	GetTextExtentPoint
#endif

#define  CX_BORDER		1
#define  CY_BORDER		1

#define	 MAX_LABELLENGTH	20
#define  VERT_PADDING		2
#define	 HORZ_PADDING		2

#define  SMALLIMAGE_CX		16
#define  SMALLIMAGE_CY		16
#define  LARGEIMAGE_CX		24
#define  LARGEIMAGE_CY		21

#define  SMALLBTN_CX		22
#define  SMALLBTN_CY		21
#define  LARGEBTN_CX		31
#define  LARGEBTN_CY		30

#define	 STDTOOLBAR_IMAGES	15
#define	 VIEWTOOLBAR_IMAGES	12

extern HINSTANCE hLibInstance;
static char TOOLBAR_PROP[] = "Toolbar_Data";	
static char szCustomizeDlgTemplate[] = "WTLBCUSTOMIZETEMPLATE";	
static HCURSOR	g_hDragCursor = ( HCURSOR )NULL;
static HCURSOR	g_hDefCursor = ( HCURSOR )NULL;
enum { eNoDrag = 0, eEndDrag = eNoDrag, eCancelDrag,
       eLBtnDown, eBeginDrag, eDragging, 
       eShftLBtnDown, eShftBeginDrag, eShftDragging  };
enum { eHitOutside = -1 };

struct TBCDLGBTN;
typedef struct 
{
	int		HitBtn;
	int		HitRow;  /* meaningless unless BtnIndx < 0 */
} TLBHITTEST;

typedef struct 
{
	TBBUTTON	Btn;
	RECT		Rect;  
	int		Row;
/*	LPSTR		pszText;*/
} TLBITEM;

typedef struct 
{
	char		Label[MAX_LABELLENGTH];
} TLBSTR;

typedef struct 
{
	HDC		hbmMonoOld;
	HDC		hbmOldGlyphs;
	HBITMAP		hbmMono;
} TLBGDI;

typedef struct 
{
	UINT		uStyles;
	POINT		Pos;
	SIZE		Size;
	int		Rows;

	int		enDrag;	 /* for dragging */
	TBNOTIFY	tbn;
	char		cBtnText[MAX_LABELLENGTH]; /* holds text of button being dragged */

	HWND		hwTTip;
	HFONT		hFont;
	HDC		hDCGlyphs;
	HDC		hDCMono;
	HBITMAP		hbmImage;

	int		ImgCX;
	int		ImgCY;
	int		ImgCount;
	int		StrCount;

	int		nXPadding;
	int		nYPadding;

	int		ItmHeight;
	int		ItmWidth;
	int		ItmCount;
	int		ItmSelected;
	int		ItmTip; /*the button for which a tooltip is going to be displayed*/

    	BOOL        	bNeedSetRects;
	BOOL		bStandardBmp; /*toolbar with predefined bitmap*/

	WLIST		ItmList; /*array of Toolbar buttons*/
	WLIST		StrList;
} TOOLBARDATA;

#define GETTHISDATA(hwnd) \
   GetProp(hwnd, TOOLBAR_PROP )
#define SETTHISDATA(hwnd, pthisNew) \
   SetProp(hwnd, TOOLBAR_PROP, pthisNew)
#define REMOVETHISDATA(hwnd) \
   RemoveProp(hwnd, TOOLBAR_PROP )


/*prototypes*/
extern void
Toolbr_GetLBxBtnInfo  
(
    	HWND            		hwDlg,
    	HWND            		hwLB,
    	int             		iLBIndx,
    	TBCDLGBTN*      		pBtnInfo
);

extern BOOL 
Tooltip_SetToolbarItemID ( HWND hwTooltip, HWND hwToolbar, UINT newID, LPRECT pnewRect );


static int
Toolbr_AddBitmap
( 
	HWND				hwToolbar,
	TOOLBARDATA*			pthis,
	UINT				nBitmaps,
	LPTBADDBITMAP			lptba
);

static BOOL
Toolbr_AddButtons
( 
	HWND				hwToolbar,
	TOOLBARDATA*			pthis,
	UINT				uButtons,
	LPTBBUTTON			lpButtons
);

static int
Toolbr_CommandToIndex
( 
	HWND				hWnd,
	TOOLBARDATA*			pthis,
	int				idCommand
);

static BOOL
Toolbr_InsertBtn 
( 
	HWND				hwToolbar,
	TOOLBARDATA*			pthis, 
	int				ItmIndx, 
	LPTBBUTTON			lpBtn 
);


static BOOL 
W_IAddImage
( 
	HBITMAP*			phbmImages,
	int*				pImgWidth,
	int*				pImgHeight,
	int*				pImgCount, 
	HBITMAP				hBitmap,
	int				NoOfImages
);

static BOOL
Toolbr_IBeginDrag 
(
	HWND				hWnd,
	TOOLBARDATA*			pthis,
	TLBITEM*			pItem,
	int				idBtn
);

static BOOL 
Toolbr_IBeginPaintBtn
(
	TOOLBARDATA*			pthis, 
	TLBGDI*				ptbg
);

static BOOL
Toolbr_IEndDrag 
(
	HWND				hWnd,
	TOOLBARDATA*			pthis,
	BOOL*				pbInsert
);

static void 
Toolbr_IEndPaintBtn
(
	TOOLBARDATA*			pthis, 
	TLBGDI*				ptbg
);

static void
Toolbr_IResetLayout
(
	HWND				hWnd,
	TOOLBARDATA*			pthis,
	TLBHITTEST			tlbht
);

static void
Toolbr_IResetCheckGroup
(
	HWND				hWnd,
	TLBITEM*			pToolBItmList,
	int				nSelected,
	int				nBtnCount
);


static void
Toolbr_SetTooltip 
( 
	HWND				hWnd, 
	HWND				hwTTip 
);


static void
Toolbr_IPaintBtn 
( 
	HWND				hWnd,
	HDC			    	hDC, 
	TOOLBARDATA*			pthis, 
	TLBITEM*			pItem,
	TLBSTR*				pStr
);

static void
Toolbr_IPaintDragLbxItem 
( 
	HWND				hWnd,
	LPDRAWITEMSTRUCT		lpdi, 
	TOOLBARDATA*			pthis, 
	TLBITEM*			pItem,
	LPSTR				pStr
);



static void
Toolbr_ISetItemRects 
( 
	HWND				hwToolbar,
	TOOLBARDATA*			pthis
);

static int
Toolbr_IBtnFromPt 
( 
	HWND				hwToolbar,
	TOOLBARDATA*			pthis, 
	int			    	x,
	int			    	y,
	TLBHITTEST*			pHitTest
);

static LRESULT  
Toolbr_TBWndProc
(
	HWND				hWnd,
    	UINT				uMessage,
    	WPARAM				wParam,
    	LPARAM				lParam
);

/*Win32 API */
HWND
WCreateToolbarEx
(
	HWND				hWnd,
	DWORD				ws,
	UINT				wID,
	int				nBitmaps,
	HINSTANCE			hBMInst,
	UINT				wBMID,
	LPCTBBUTTON			lpButtons,
	int				iNumButtons,
	int				dxButton,
	int				dyButton,
	int				dxBitmap,
	int				dyBitmap,
	UINT				uStructSize
)
{
	HWND				hwToolbar;
	TBADDBITMAP			tba;
	HGLOBAL				hToolbar	= ( HGLOBAL )NULL;
	TOOLBARDATA*			pthis		= ( TOOLBARDATA* )NULL;

	int				ErrorCode	= 0;


    	ws |= WS_CHILD;
	ws &= ~WS_BORDER;
	hwToolbar = CreateWindowEx ( 0,WTOOLBARCLASSNAME, "Willows Toolbar", ws, 
			0, 0, 0, 0, hWnd, ( HMENU )wID,
			( HINSTANCE )GetWindowLong ( hWnd, GWL_HINSTANCE ), NULL );
	if ( hwToolbar )
	{
		if ( ! ( hToolbar = GETTHISDATA ( hwToolbar ) ) )
			ErrorCode = WTLB_ERR_GETWINPROP;
		else
		if ( ! ( pthis = ( TOOLBARDATA *) GlobalLock ( hToolbar ) ) )
			ErrorCode = WTLB_ERR_GLOBALLOCK;
		else
		{
			pthis->ImgCX = dxBitmap;
			pthis->ImgCY = dyBitmap;
#if 1
			if ( dxButton == 0 )
				dxButton = dxBitmap;
			if ( dyButton == 0 )
				dyButton = dyBitmap;
#endif
			pthis->ItmWidth = dxButton + 2 * HORZ_PADDING;
			pthis->ItmHeight = dyButton + 2 * VERT_PADDING;

			tba.hInst = hBMInst;
			tba.nID = wBMID;
			if ( wBMID == IDB_STD_LARGE_COLOR  || 
		     	     wBMID == IDB_STD_SMALL_COLOR  ||
		     	     wBMID == IDB_VIEW_LARGE_COLOR || 
		     	     wBMID == IDB_VIEW_SMALL_COLOR
		   	   )
				pthis->bStandardBmp = TRUE;
			Toolbr_AddBitmap ( hwToolbar, pthis, nBitmaps, &tba ); 
			Toolbr_AddButtons ( hwToolbar, pthis, iNumButtons, 
				( LPTBBUTTON )lpButtons );
		}

	}

	return hwToolbar;

}
#ifdef 0
/* helper function */
static HBITMAP 
CreateDitherBitmap()
{
	int				i;
	COLORREF			clr;
	long				patGray[8];
	HDC				hDC;
	HBITMAP				hbm;
	struct  /* BITMAPINFO with 16 colors*/
	{
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD      bmiColors[16];
	} bmi;
	memset(&bmi, 0, sizeof(bmi));

	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = 8;
	bmi.bmiHeader.biHeight = 8;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 1;
	bmi.bmiHeader.biCompression = BI_RGB;

	clr = GetSysColor(COLOR_BTNFACE);
	bmi.bmiColors[0].rgbBlue = GetBValue(clr);
	bmi.bmiColors[0].rgbGreen = GetGValue(clr);
	bmi.bmiColors[0].rgbRed = GetRValue(clr);

	/*if (afxData.bWin31)*/
#if 0
		clr = ::GetSysColor(COLOR_BTNHIGHLIGHT);
#else
		clr = RGB(255, 255, 255);
#endif
	bmi.bmiColors[1].rgbBlue = GetBValue(clr);
	bmi.bmiColors[1].rgbGreen = GetGValue(clr);
	bmi.bmiColors[1].rgbRed = GetRValue(clr);

	/* initialize the brushes*/
	for ( i = 0; i < 8; i++ )
	   patGray[i] = (i & 1) ? 0xAAAA5555L : 0x5555AAAAL;

	hDC = GetDC ( ( HWND )NULL );
	hbm = CreateDIBitmap(hDC, &bmi.bmiHeader, CBM_INIT,
		(LPBYTE)patGray, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS);
	ReleaseDC ( ( HWND )NULL, hDC);

	return hbm;
}
#endif

/* Tool bar Implementation*/

static int
Toolbr_AddBitmap
( 
	HWND				hwToolbar,
	TOOLBARDATA*			pthis,
	UINT				nBitmaps,
	LPTBADDBITMAP			lptba
)
{
	int				tmp = ( int )lptba->hInst;
	HINSTANCE			hBMInst = lptba->hInst;
	UINT				wBMID	= lptba->nID;
	HBITMAP				hBitmap = ( HBITMAP )NULL;
	int				Index	= -1;

	tmp = HINST_COMMCTRL;
	if ( hBMInst == HINST_COMMCTRL )
	{
#ifdef _WINDOWS
		hBMInst = ( HINSTANCE )GetWindowLong ( GetParent ( hwToolbar ), GWL_HINSTANCE ); 
#else
		hBMInst = hLibInstance;
#endif
		if ( wBMID == IDB_STD_LARGE_COLOR  || 
		     wBMID == IDB_STD_SMALL_COLOR
		   )
			nBitmaps = 15;
		if ( wBMID == IDB_VIEW_LARGE_COLOR  || 
		     wBMID == IDB_VIEW_SMALL_COLOR
		   )
			nBitmaps = 12;
		if ( wBMID == IDB_STD_LARGE_COLOR  || 
		     wBMID == IDB_VIEW_LARGE_COLOR
		   )
		{
			pthis->ImgCX = LARGEIMAGE_CX;
			pthis->ImgCY = LARGEIMAGE_CY;
			pthis->ItmHeight = LARGEBTN_CY + 2 * VERT_PADDING;
			pthis->ItmWidth = LARGEBTN_CX + 2 * HORZ_PADDING;
		}
		else
		if ( wBMID == IDB_STD_SMALL_COLOR  || 
		     wBMID == IDB_VIEW_SMALL_COLOR
		   )
		{
			pthis->ImgCX = SMALLIMAGE_CX;
			pthis->ImgCY = SMALLIMAGE_CY;
			pthis->ItmHeight = SMALLBTN_CY + 2 * VERT_PADDING;
			pthis->ItmWidth = SMALLBTN_CX + 2 * HORZ_PADDING;
		}

		if ( wBMID == IDB_STD_SMALL_COLOR )
			wBMID = IDB_STDSMALLCOLOR;
	}

	if ( hBMInst ) /* need refinement to eliminate imagelist */
		hBitmap = LoadBitmap ( hBMInst, MAKEINTRESOURCE ( wBMID ) );
	else
		hBitmap = ( HWND )wBMID;

	if ( pthis->hbmImage == ( HBITMAP )NULL )
	{
		pthis->hbmImage = hBitmap;
		pthis->ImgCount = nBitmaps;
		Index = 0;
		if ( pthis->ImgCX <= 0 || pthis->ImgCY <= 0 )
		{
			BITMAP Bitmap;
			GetObject ( hBitmap, sizeof ( BITMAP ), &Bitmap );
			pthis->ImgCX = Bitmap.bmWidth / nBitmaps;
			pthis->ImgCY = Bitmap.bmHeight;
		}
	}
	else
	{
		Index = pthis->ImgCount; 
		W_IAddImage ( &pthis->hbmImage, &pthis->ImgCX, &pthis->ImgCY, 
				&pthis->ImgCount, hBitmap, nBitmaps );
		DeleteObject ( hBitmap );
	}

	return Index;
}

static BOOL
Toolbr_AddButtons
( 
	HWND				hwToolbar,
	TOOLBARDATA*			pthis,
	UINT				uButtons,
	LPTBBUTTON			lpButtons
)
{
	int				i;
	int				Count;
	
	if ( !lpButtons )
		return FALSE;
	
	Count = pthis->ItmCount;
	for ( i = 0; i < ( int )uButtons; i++, Count++ )
	{
		Toolbr_InsertBtn ( hwToolbar, pthis, Count, lpButtons + i );
	}

	pthis->ItmCount = Count;

	return TRUE;
}

static int
Toolbr_AddStrings
( 
	HWND				hwToolbar,
	TOOLBARDATA*			pthis,
	HINSTANCE			hInst,
	LPSTR				lpStrings
)
{
	int			    	i;
	int				Count;
	int				Index;
	int				Len;
	SIZE				Size;
	char				szLabel[MAX_LABELLENGTH];
	char*				pLabel		= ( char* )NULL;
	HDC				hDC		= GetDC ( hwToolbar );
	HFONT				OldFont		= ( HFONT )NULL;

	
	if ( !lpStrings )
		return -1;

	Index = pthis->StrCount;
	
	memset ( &Size, 0, sizeof ( SIZE ) );
	if ( hInst )
	{
		if ( LoadString ( hInst, LOWORD ( lpStrings ), szLabel, MAX_LABELLENGTH ) == 0 )
			return -1;
		WLInsert ( &pthis->StrList, Index, szLabel);
		return Index;
	}

	pLabel = lpStrings;
	OldFont = SelectObject ( hDC, pthis->hFont );
	for ( i = 0, Count = Index; ; i++, Count++ )
	{
		if ( *pLabel == '\0' )
			break;
		if ( WLInsert ( &pthis->StrList, Count, pLabel) )
			break;
		Len = lstrlen ( pLabel );
		Len = min ( sizeof ( TLBSTR ), Len );  
		GetTextExtentPoint32 ( hDC, pLabel, Len, &Size );
		if ( pthis->ItmWidth <= Size.cx )
			pthis->ItmWidth = Size.cx + 2 * HORZ_PADDING;
		while ( *pLabel++ );
	}
	
	pthis->ItmHeight = pthis->ImgCY + Size.cy + 3 * VERT_PADDING;	
	pthis->StrCount = Count;

	SelectObject ( hDC, OldFont );
	ReleaseDC ( hwToolbar, hDC );
	return Index;
}

static void
Toolbr_AutoSize
( 
	HWND				hwToolbar,
	TOOLBARDATA*			pthis
)
{
	RECT				rect;

	GetClientRect ( GetParent ( hwToolbar ), &rect );
	pthis->Size.cx = rect.right - rect.left;
	Toolbr_ISetItemRects ( hwToolbar, pthis );
#if 1
	pthis->Pos.x = rect.left;
	pthis->Pos.y = ((pthis->uStyles & CCS_BOTTOM) == CCS_BOTTOM) ?
			rect.bottom - pthis->Size.cy : rect.top;
#endif
	InvalidateRect ( hwToolbar, NULL, TRUE );
	UpdateWindow ( hwToolbar );
}


static BOOL
Toolbr_ChangeBitmap
( 
	HWND				hwToolbar,
	TOOLBARDATA*			pthis,
	int				idButton,
	int				iBitmap
)
{
	TLBITEM*			pToolBItmList	= ( TLBITEM* )NULL;
	int				ErrorCode	= 0;

	if ( iBitmap >= pthis->ImgCount )
		ErrorCode = WTLB_ERR_OUTOFRANGE;	
	else
	if ( ( idButton = Toolbr_CommandToIndex ( hwToolbar, pthis, idButton ) ) == -1 )
		ErrorCode = WTLB_ERR_OUTOFRANGE;	
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	{
		( pToolBItmList + idButton )->Btn.iBitmap = iBitmap;
	}


	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );
	
	return ( ErrorCode >= 0 );
}

static BOOL
Toolbr_CheckBtn
( 
	HWND				hWnd,
	TOOLBARDATA*			pthis,
	int				idButton,
	BOOL				fCheck
)
{
	TLBITEM*			pToolBItmList	= ( TLBITEM* )NULL;
	TLBITEM*			pItem		= ( TLBITEM* )NULL;
	int				BtnChecked;
	int				ErrorCode	= 0;

	if ( idButton >= pthis->ItmCount )
		ErrorCode = WTLB_ERR_OUTOFRANGE;	
	else
	if ( ( BtnChecked = Toolbr_CommandToIndex ( hWnd, pthis, idButton ) ) == -1 )
		ErrorCode = WTLB_ERR_OUTOFRANGE;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	{
		RECT	rcBtn;
		pthis->ItmSelected = BtnChecked;
		pItem = pToolBItmList + BtnChecked;
		if ( pItem && pItem->Btn.fsStyle & TBSTYLE_CHECK )
		{
			if ( fCheck )
			{
				if ( pItem->Btn.fsStyle & TBSTYLE_GROUP )
					Toolbr_IResetCheckGroup ( hWnd, pToolBItmList, BtnChecked, 
							pthis->ItmCount );
				pItem->Btn.fsState |= TBSTATE_PRESSED;
			}
			else
				pItem->Btn.fsState &= ~TBSTATE_PRESSED;

			CopyRect ( &rcBtn, &pItem->Rect );
			InflateRect ( &rcBtn, 1, 1 );
			InvalidateRect ( hWnd, &rcBtn, TRUE );
			UpdateWindow ( hWnd );
			SendMessage ( GetParent ( hWnd ), WM_COMMAND, 
				MAKELONG ( pItem->Btn.idCommand, 0 ), 0L );
		}
	}


	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );
	
	return ( ErrorCode >= 0 );
}


static int
Toolbr_CommandToTextIndex
( 
	HWND				hWnd,
	TOOLBARDATA*			pthis,
	int				idCommand
)
{
	int				j;
	int				i;
	TLBITEM*			pToolBItmList	= ( TLBITEM* )NULL;
	int				ErrorCode	= 0;

	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	{
		for ( i = 0, j = -1; i < pthis->ItmCount; i++ )
		{
			if ( ( pToolBItmList + i )->Btn.idCommand > 0 )
				j++;
			if ( ( pToolBItmList + i )->Btn.idCommand == idCommand )
				break;
		}
	}


	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );
	
	return ( j < pthis->ItmCount ? j : -1 );
}

static int
Toolbr_CommandToIndex
( 
	HWND				hWnd,
	TOOLBARDATA*			pthis,
	int				idCommand
)
{
	int				i = -1;
	TLBITEM*			pToolBItmList	= ( TLBITEM* )NULL;
	int				ErrorCode    	= 0;

	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	{
		for ( i = 0; i < pthis->ItmCount; i++ )
		{
			if ( ( pToolBItmList + i )->Btn.idCommand == idCommand )
				break;
		}
	}


	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );
	
	return ( i < pthis->ItmCount ? i : -1 );
}

static BOOL
Toolbr_DeleteBtn
(
	HWND				hwToolbar,
	TOOLBARDATA*		pthis, 
	int				    iButton 
)
{
	BOOL				bRedraw		    = FALSE;
	int				    ErrorCode	    = -1;

	if ( iButton < 0 || iButton >= pthis->ItmCount )
        return FALSE;

    if ( WLDelete ( &pthis->ItmList, iButton ) == 0 )
    {
        pthis->ItmCount--;
        ErrorCode = 0;
        bRedraw = TRUE;
    }

	if ( bRedraw )
	{
		Toolbr_ISetItemRects ( hwToolbar, pthis );
		InvalidateRect ( hwToolbar, NULL, TRUE );
		UpdateWindow ( hwToolbar );
	}
	
	return ( ErrorCode >= 0 );
}
/* to be called from within the customize dialogbox WM_DRAWITEM handler */
void
Toolbr_DrawDragLBxItem 
( 
    	HWND                hwDlg,
    	HWND                hwToolbar, 
    	LPDRAWITEMSTRUCT    lpdi,
    	LPSTR               pText
)
{
    	int                 index;
	HGLOBAL		    hToolbar	    = ( HGLOBAL )NULL;
	TOOLBARDATA*	    pthis	    = ( TOOLBARDATA* )NULL;
	TLBITEM*	    pToolBItmList   = ( TLBITEM* )NULL;
	TLBITEM*	    pItem	    = ( TLBITEM* )NULL;
	int				    ErrorCode	    = 0;
	
	if ( ! ( hToolbar = GETTHISDATA ( hwToolbar ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLBARDATA *) GlobalLock ( hToolbar ) ) )
		ErrorCode = WTLB_ERR_GLOBALLOCK;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
    	{
        	TLBGDI  tbg;
        	TBCDLGBTN   BtnInfo;
        	Toolbr_GetLBxBtnInfo ( hwDlg, lpdi->hwndItem, lpdi->itemID, &BtnInfo );
        	index = Toolbr_CommandToIndex ( hwToolbar, pthis, BtnInfo.iCmdID );
        	pItem = index != -1 ? pToolBItmList + index : NULL;
        	Toolbr_IBeginPaintBtn ( pthis, &tbg );
        	Toolbr_IPaintDragLbxItem ( hwToolbar, lpdi, pthis, pItem, BtnInfo.Text );
        	Toolbr_IEndPaintBtn ( pthis, &tbg );
    	}

    	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );
    	if ( pthis )
		GlobalUnlock ( hToolbar );
}

static BOOL
Toolbr_EnableBtn
(
	HWND			hwToolbar,
	TOOLBARDATA*	pthis, 
	int				idButton,
	BOOL			bEnable
)
{
	TLBITEM*		pToolBItmList	= ( TLBITEM* )NULL;
	BOOL			bResult = FALSE;
	int				ErrorCode	= 0;

	if ( ( idButton = Toolbr_CommandToIndex ( hwToolbar, pthis, idButton ) ) == -1 )
		ErrorCode = WTLB_ERR_OUTOFRANGE;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	{
		RECT rcBtn;
		( pToolBItmList + idButton )->Btn.fsState |= bEnable ? TBSTATE_ENABLED : 
								       TBSTATE_INDETERMINATE;

		CopyRect ( &rcBtn, &( ( pToolBItmList + idButton )->Rect) );
		InflateRect ( &rcBtn, 1, 1 );
		InvalidateRect ( hwToolbar, &rcBtn, TRUE );
		UpdateWindow ( hwToolbar );
		bResult = TRUE;
	}

	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );
	return ( ErrorCode >= 0 && bResult );
}

/*
 * Note: Microsoft docs state that if unsuccessful, this function would return 0. But 0 is a valid
 *       value for the index of a button's bitmap. So we assume that if unsuccessful, the return
 *       value would be -1, which is more reasonable.
 */
static int
Toolbr_GetBitmap
(
	HWND				hwToolbar,
	TOOLBARDATA*			pthis, 
	int				idButton
)
{
	TLBITEM*			pToolBItmList	= ( TLBITEM* )NULL;
	int				iBitmap		= -1;
	int				ErrorCode	= 0;

	if ( ( idButton = Toolbr_CommandToIndex ( hwToolbar, pthis, idButton ) ) == -1 )
		ErrorCode = WTLB_ERR_OUTOFRANGE;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	{
		iBitmap = ( pToolBItmList + idButton )->Btn.iBitmap;
	}

	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );
	return iBitmap;
}


static int
Toolbr_GetBitmapFlag
(
	HWND				hwToolbar
)
{
	int				iFlag	= 0;
	HDC				hDC	= GetDC ( ( HWND )NULL );

	if ( GetDeviceCaps ( hDC, LOGPIXELSX ) >= 120 )
		iFlag = TBBF_LARGE;
	ReleaseDC ( ( HWND )NULL, hDC );
	
	return iFlag;
}

static BOOL
Toolbr_GetButton
(
	HWND				hwToolbar,
	TOOLBARDATA*			pthis, 
	int				idButton,
	LPTBBUTTON			lpTBButton
)
{
	TLBITEM*			pToolBItmList	= ( TLBITEM* )NULL;
	BOOL				bResult = FALSE;
	int				ErrorCode	= 0;

	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	if ( lpTBButton && idButton >= 0 && idButton <= pthis->ItmCount )
	{
		*lpTBButton =  ( pToolBItmList + idButton )->Btn;
		bResult = TRUE;
	}

	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );

	return ( ErrorCode >= 0 && bResult );
}

static int
Toolbr_GetButtonText
(
	HWND				hwToolbar,
	TOOLBARDATA*			pthis, 
	int				idButton,
	LPSTR				lpBtnText
)
{
	TLBSTR*				pToolBItmText	= ( TLBSTR* )NULL;
	TLBSTR*				pItmText	= ( TLBSTR* )NULL;
	int				len		= -1;
	LPVOID				lpText		= NULL;
	int				ErrorCode	= 0;

	if ( pthis->StrCount <= 0 )
		return -1;
	if ( ( idButton = Toolbr_CommandToTextIndex ( hwToolbar, pthis, idButton ) ) == -1 )
		ErrorCode = WTLB_ERR_OUTOFRANGE;
	else
	if ( WLLock ( &pthis->StrList, ( LPVOID )&pToolBItmText ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	if ( lpBtnText && idButton >= 0 && idButton <= pthis->StrCount )
	{
		pItmText = pToolBItmText + idButton;
		lpText = lstrcpyn( lpBtnText, ( LPSTR )pItmText, sizeof ( TLBSTR ) );
	}
	if ( pToolBItmText )
		WLUnlock ( &pthis->StrList, ( LPVOID )&pToolBItmText );

	if ( lpText )
		len = lstrlen ( ( LPSTR ) lpText );
	return len;
}


static BOOL
Toolbr_GetItemRect
(
	HWND				hwToolbar,
	TOOLBARDATA*			pthis, 
	int				idButton,
	LPRECT				lpRect
)
{
	TLBITEM*			pToolBItmList	= ( TLBITEM* )NULL;
	BOOL				bResult		= FALSE;
	int				ErrorCode	= 0;

	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	if ( !( ( pToolBItmList + idButton )->Btn.fsState & TBSTATE_HIDDEN ) && 
	     lpRect && idButton >= 0 && idButton <= pthis->ItmCount )
	{
		CopyRect ( lpRect, &( ( pToolBItmList + idButton )->Rect ) );
		bResult = TRUE;
	}

	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );

	return ( ErrorCode >= 0 && bResult );
}

static BYTE
Toolbr_GetState
(
	HWND				hwToolbar,
	TOOLBARDATA*			pthis, 
	int				idButton
)
{
	TLBITEM*			pToolBItmList	= ( TLBITEM* )NULL;
	int				State		= -1;
	int				ErrorCode	= 0;

	if ( ( idButton = Toolbr_CommandToIndex ( hwToolbar, pthis, idButton ) ) == -1 )
		ErrorCode = WTLB_ERR_OUTOFRANGE;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
		State =  ( pToolBItmList + idButton )->Btn.fsState;

	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );

	return State;
}

static BOOL
Toolbr_HideBtn
(
	HWND				hwToolbar,
	TOOLBARDATA*			pthis, 
	int				idButton,
	BOOL				bHide
)
{
	TLBITEM*			pToolBItmList	    = ( TLBITEM* )NULL;
	TLBITEM*			pItem		    = ( TLBITEM* )NULL;
	BOOL				bSkip		    = FALSE;
	int				ErrorCode	    = 0;

	if ( ( idButton = Toolbr_CommandToIndex ( hwToolbar, pthis, idButton ) ) == -1 )
		ErrorCode = WTLB_ERR_OUTOFRANGE;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
    	if ( idButton < pthis->ItmCount )
	{
		pItem = pToolBItmList + idButton;
        	bSkip = ( bHide && pItem->Btn.fsState & TBSTATE_HIDDEN ) ||
		        ( !bHide && !( pItem->Btn.fsState & TBSTATE_HIDDEN ) );
		if ( !bSkip )
		{
			pItem->Btn.fsState ^= TBSTATE_HIDDEN;
#if 0
			CopyRect ( &rcBtn, &( ( pToolBItmList + idButton )->Rect) );
			InflateRect ( &rcBtn, 1, 1 );
#endif
		}
	}

	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );
	if ( !bSkip )
	{
		Toolbr_ISetItemRects ( hwToolbar, pthis );
		InvalidateRect ( hwToolbar, NULL, TRUE );
		UpdateWindow ( hwToolbar );
	}
	return ( ErrorCode >= 0 );
}

static BOOL
Toolbr_InDeterminate
(
	HWND				hwToolbar,
	TOOLBARDATA*			pthis, 
	int				idButton,
	BOOL				bInDeterminate
)
{
	RECT				rcBtn;
	TLBITEM*			pToolBItmList	= ( TLBITEM* )NULL;
	TLBITEM*			pItem		= ( TLBITEM* )NULL;
	BOOL				bSkip		= FALSE;

	int				ErrorCode	= 0;

	if ( ( idButton = Toolbr_CommandToIndex ( hwToolbar, pthis, idButton ) ) == -1 )
		ErrorCode = WTLB_ERR_OUTOFRANGE;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	{
		pItem = pToolBItmList + idButton;
		bSkip = ( bInDeterminate && pItem->Btn.fsState & TBSTATE_INDETERMINATE ) ||
		        ( !bInDeterminate && !( pItem->Btn.fsState & TBSTATE_INDETERMINATE ) );
		if ( !bSkip )
		{
			pItem->Btn.fsState ^= TBSTATE_INDETERMINATE;
			CopyRect ( &rcBtn, &( pItem->Rect) );
			InflateRect ( &rcBtn, 1, 1 );
		}
	}

	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );
	if ( !bSkip )
	{
		InvalidateRect ( hwToolbar, &rcBtn, TRUE );
		UpdateWindow ( hwToolbar );
	}
	return ( ErrorCode >= 0 );
}

static BOOL
Toolbr_PressBtn
(
	HWND				hwToolbar,
	TOOLBARDATA*			pthis, 
	int				idButton,
	BOOL				bPress
)
{
	RECT				rcBtn;
	TLBITEM*			pToolBItmList	= ( TLBITEM* )NULL;
	TLBITEM*			pItem		= ( TLBITEM* )NULL;
	BOOL				bSkip		= FALSE;

	int				ErrorCode	= 0;

	if ( ( idButton = Toolbr_CommandToIndex ( hwToolbar, pthis, idButton ) ) == -1 )
		ErrorCode = WTLB_ERR_OUTOFRANGE;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	{
		pItem = pToolBItmList + idButton;
		bSkip = ( bPress && pItem->Btn.fsState & TBSTATE_PRESSED ) ||
		        ( !bPress && !( pItem->Btn.fsState & TBSTATE_PRESSED ) );
		if ( !bSkip )
		{
			pItem->Btn.fsState ^= TBSTATE_PRESSED;
			CopyRect ( &rcBtn, &( pItem->Rect) );
			InflateRect ( &rcBtn, 1, 1 );
		}
	}

	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );
	if ( !bSkip )
	{
		InvalidateRect ( hwToolbar, &rcBtn, TRUE );
		UpdateWindow ( hwToolbar );
	}
	return ( ErrorCode >= 0 );
}

static BOOL
Toolbr_SetBMPSize
(
	HWND				hwToolbar,
	TOOLBARDATA*			pthis, 
	int				Width,
	int				Height
)
{
#if 0	/* MFC4 sets size after adding items */
	if ( pthis->ImgCount < 0 )
		return FALSE;
#endif	/* MFC4 */
	pthis->ImgCX = Width;
	pthis->ImgCY = Height;
	return TRUE;
}

static BOOL
Toolbr_SetBtnSize
(
	HWND				hwToolbar,
	TOOLBARDATA*			pthis, 
	int				Width,
	int				Height
)
{
    RECT            rect;
#if 0	/* MFC4 sets size after adding items */
	if ( pthis->ItmCount > 0 )
		return FALSE;
#endif	/* MFC4 */
	pthis->ItmWidth = Width;
	pthis->ItmHeight = Height;

    GetClientRect ( GetParent ( hwToolbar ), &rect );
	pthis->Size.cy = pthis->uStyles & CCS_NODIVIDER ?
			 pthis->ItmHeight + 2 * VERT_PADDING :
			 pthis->ItmHeight + 3 * VERT_PADDING ;
	pthis->Pos.y = ( pthis->uStyles & CCS_BOTTOM ) == CCS_BOTTOM ?
        rect.bottom - pthis->Size.cy : rect.top;
#if 1
    pthis->bNeedSetRects = TRUE;
#else
    SetWindowPos ( hwToolbar, ( HWND )NULL, pthis->Pos.x, pthis->Pos.y,
		   pthis->Size.cx, pthis->Size.cy,
		   SWP_NOACTIVATE | SWP_NOZORDER );
#endif
    return TRUE;
}


static BOOL
Toolbr_QueryStat
(
	HWND				hwToolbar,
	TOOLBARDATA*			pthis, 
	UINT				uMessage,
	int				idButton
)
{
	TLBITEM*			pToolBItmList	= ( TLBITEM* )NULL;
	TLBITEM*			pItem		= ( TLBITEM* )NULL;
	int				State		= 0;
	BOOL				bResult		= FALSE;

	int				ErrorCode	= 0;

	if ( ( idButton = Toolbr_CommandToIndex ( hwToolbar, pthis, idButton ) ) == -1 )
		ErrorCode = WTLB_ERR_OUTOFRANGE;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	{
		State = uMessage == TB_ISBUTTONCHECKED ? TBSTATE_CHECKED :	
			uMessage == TB_ISBUTTONENABLED ? TBSTATE_ENABLED :	
			uMessage == TB_ISBUTTONHIDDEN  ? TBSTATE_HIDDEN :	
			uMessage == TB_ISBUTTONINDETERMINATE ? TBSTATE_INDETERMINATE :	
			uMessage == TB_ISBUTTONPRESSED ? TBSTATE_PRESSED : 0;
		bResult = pItem->Btn.fsState & State;
	}

	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );
	return bResult;
}

static BOOL
Toolbr_SetCmdID
(
	HWND				hwToolbar,
	TOOLBARDATA*			pthis, 
	UINT				indx,
	UINT				cmdID
)
{
	BOOL				bResult		= FALSE;
	TLBITEM*			pToolBItmList	= ( TLBITEM* )NULL;

	int				ErrorCode	= 0;

	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	if ( indx >= 0 && indx <= ( UINT )pthis->ItmCount )
	{
		( pToolBItmList + indx )->Btn.idCommand = cmdID;
		bResult = TRUE;
	}

	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );

	return ( bResult );
}

static void
Toolbr_SetRows 
( 
	HWND				hwToolbar,
	TOOLBARDATA*			pthis,
	int				cRows,
	BOOL				bLarger,
	LPRECT				lpRect
)
{
	int				i;
	int				ItmsPerRow;
	int				Count		= 0;
	TLBITEM*			pItemList	= ( TLBITEM* )NULL;
	TLBITEM*			pItem		= ( TLBITEM* )NULL;

	ItmsPerRow = pthis->ItmCount / cRows;
	Count = pthis->ItmCount;
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItemList ) )
		return;

	for ( i = 0; i < Count; i++ )
	{
		pItem = pItemList + i;
		if ( ( pItem->Btn.fsState & TBSTATE_HIDDEN ) ||
		     ( pItem->Btn.fsStyle & TBSTYLE_SEP ) 
		   )
			continue;
		if ( ( i + 1 ) % ItmsPerRow == 0 )
			pItem->Btn.fsState |= TBSTATE_WRAP;
	}

	pthis->Rows = cRows;
	pthis->Size.cy = pthis->Rows * ( pthis->ItmHeight + 2 * pthis->nYPadding );
	if ( lpRect )
	{
		SetRect ( lpRect, pthis->Pos.x, pthis->Pos.y, 
			  pthis->Pos.x + pthis->Size.cx, pthis->Pos.y + pthis->Size.cy );
	}
		/*CopyRect ( lpRect, &pthis->Rect );*/
	if ( pItemList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pItemList );

}


static BOOL 
Toolbr_IBeginPaintBtn
(
	TOOLBARDATA*		pthis, 
	TLBGDI*				ptbg
)
{

	ptbg->hbmOldGlyphs = ( HBITMAP )SelectObject ( pthis->hDCGlyphs, pthis->hbmImage );
	ptbg->hbmMono = CreateBitmap ( pthis->ImgCX, pthis->ImgCY, 1, 1, NULL );
	ptbg->hbmMonoOld = ( HBITMAP )SelectObject ( pthis->hDCMono, ptbg->hbmMono);
	if ( ptbg->hbmOldGlyphs == ( HBITMAP )NULL || 
         ptbg->hbmMono == ( HBITMAP )NULL || 
         ptbg->hbmMonoOld == ( HBITMAP )NULL )
	{
		if ( ptbg->hbmMono)
			DeleteObject ( ptbg->hbmMono );
		return FALSE;
	}
	return TRUE;
}

static void 
Toolbr_IEndPaintBtn
(
	TOOLBARDATA*		pthis, 
	TLBGDI*				ptbg
)
{
	SelectObject ( pthis->hDCMono, ptbg->hbmMonoOld );
	DeleteObject ( ptbg->hbmMono );
	SelectObject ( pthis->hDCGlyphs, ptbg->hbmOldGlyphs );
}


static BOOL
Toolbr_InsertBtn 
( 
	HWND				hwToolbar,
	TOOLBARDATA*			pthis, 
	int				ItmIndx, 
	LPTBBUTTON			lpBtn 
)
{
	TLBITEM				tlbi;
	UINT				Count		= 0;
	int				ErrorCode	= 0;

	memset ( &tlbi, 0, sizeof ( TLBITEM ) );
	tlbi.Btn = *lpBtn;
	if ( lpBtn->fsStyle & TBSTYLE_SEP && lpBtn->iBitmap <= 0 )
	{
		
		tlbi.Btn.iBitmap = 2*pthis->nXPadding;
	}		
	WLCount ( &pthis->ItmList, &Count );
	if ( ItmIndx > Count )
		ItmIndx = Count;

	if ( WLInsert ( &pthis->ItmList, ItmIndx, &tlbi ) < 0 )
		ErrorCode = WTLB_ERR_LISTINSERT;
	else
	{
		pthis->ItmCount++;
        	pthis->bNeedSetRects = TRUE;
		Toolbr_ISetItemRects ( hwToolbar, pthis );
	}
	

	return  ( ErrorCode >= 0 );
}


static BOOL
Toolbr_OnCreate 
(
	HWND				hWnd, 
	CREATESTRUCT*			pcs
) 
{
	RECT				rect;
	HGLOBAL				hToolbar	= ( HGLOBAL )NULL;
	TOOLBARDATA*			pthis		= ( TOOLBARDATA* )NULL;

	int				ErrorCode	= 0;

	if ( ! ( hToolbar = GlobalAlloc ( GHND, sizeof ( TOOLBARDATA ) ) ) )
		ErrorCode = WTLB_ERR_GLOBALALLOC;
	else
	if ( ! ( pthis = ( TOOLBARDATA *) GlobalLock ( hToolbar ) ) )
		ErrorCode = WTLB_ERR_GLOBALLOCK;
	else
	{
		pthis->uStyles = pcs->style;
        	pthis->uStyles &= ~WS_BORDER;
        	pthis->StrCount = 0;
		pthis->nXPadding = HORZ_PADDING;
		pthis->nYPadding = VERT_PADDING;
		pthis->ImgCX = SMALLIMAGE_CX;
		pthis->ImgCY = SMALLIMAGE_CY;
		pthis->ItmWidth = pthis->ImgCX + 2 * HORZ_PADDING;
		pthis->ItmHeight = pthis->ImgCY + 2 * VERT_PADDING;
		pthis->bStandardBmp = FALSE;
		
		pthis->hFont = ( HFONT )GetStockObject ( ANSI_VAR_FONT );
		pthis->hDCGlyphs = CreateCompatibleDC ( ( HDC )NULL );
		pthis->hDCMono = CreateCompatibleDC ( ( HDC )NULL );
#if 0
		if ( ( hbmGray = CreateDitherBitmap () ) )
			pthis->hbrDither = CreatePatternBrush(hbmGray);
#endif
		WLCreate ( &pthis->ItmList, sizeof ( TLBITEM ) );
		WLCreate ( &pthis->StrList, sizeof ( TLBSTR ) );
		if ( pthis->uStyles & TBSTYLE_TOOLTIPS )
		{
			pthis->hwTTip = CreateWindowEx ( 0, WC_TOOLTIP, ( LPSTR ) NULL, 
				WS_POPUP | WS_BORDER | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, 
				CW_USEDEFAULT, CW_USEDEFAULT, hWnd, ( HMENU )NULL, 
				( HINSTANCE )GetWindowLong ( hWnd, GWL_HINSTANCE ), NULL ); 

			Toolbr_SetTooltip ( hWnd, pthis->hwTTip );
		}

		SETTHISDATA ( hWnd, hToolbar );
		GetClientRect ( GetParent ( hWnd ), &rect );

		pthis->Size.cx = rect.right - rect.left;
		pthis->Size.cy = pthis->uStyles & CCS_NODIVIDER ?
				 pthis->ItmHeight + 2 * VERT_PADDING :
				 pthis->ItmHeight + 3 * VERT_PADDING ;
		pthis->Pos.x = rect.left;
		pthis->Pos.y = ( pthis->uStyles & CCS_BOTTOM ) == CCS_BOTTOM ?
            	rect.bottom - pthis->Size.cy : rect.top;
        	pthis->bNeedSetRects = TRUE;
		

		SetWindowPos ( hWnd, ( HWND )NULL, pthis->Pos.x, pthis->Pos.y,
			   pthis->Size.cx, pthis->Size.cy,
			   SWP_NOACTIVATE | SWP_NOZORDER );
	}

	if ( pthis )
		GlobalUnlock ( hToolbar );
	return ( ErrorCode >= 0 );
}


static void Toolbr_OnLButtonDblClk 
(	
	HWND				hWnd, 
	int				x, 
	int				y, 
	UINT				keyFlags
) 
{
	HGLOBAL				hToolbar	    = ( HGLOBAL )NULL;
	TOOLBARDATA*			pthis		    = ( TOOLBARDATA* )NULL;
	int				ErrorCode	    = 0;
	
	if ( ! ( hToolbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLBARDATA *) GlobalLock ( hToolbar ) ) )
		ErrorCode = WTLB_ERR_GLOBALLOCK;
	else
	if ( pthis->uStyles & CCS_ADJUSTABLE )
	{
        NMHDR hdr;
        hdr.hwndFrom = hWnd;
        hdr.idFrom = GetDlgCtrlID ( hWnd );
        hdr.code = TBN_BEGINADJUST;
        SendMessage ( GetParent ( hWnd ), WM_NOTIFY, 0, ( LPARAM )&hdr ); 
        /*somehow hWnd would never become the dialog's parent */
        /*so we have to pass it as lParam */
#ifdef _WINDOWS
        DialogBoxParam ( ( HINSTANCE )GetWindowLong ( hWnd, GWL_HINSTANCE ), szCustomizeDlgTemplate,
                    hWnd, ( DLGPROC )Toolbr_CustomizeWndProc, ( LPARAM )hWnd ); 
#else
        DialogBoxParam ( hLibInstance, szCustomizeDlgTemplate,
                    hWnd, ( DLGPROC )Toolbr_CustomizeWndProc, ( LPARAM )hWnd ); 
#endif
        hdr.code = TBN_ENDADJUST;
        SendMessage ( GetParent ( hWnd ), WM_NOTIFY, 0, ( LPARAM )&hdr ); 
	}

    if ( pthis )
		GlobalUnlock ( hToolbar );
}


static void Toolbr_OnLButtonDown 
(	
	HWND				hWnd, 
	int				x, 
	int				y, 
	UINT				keyFlags
) 
{
	unsigned char				cKeyBuf[256];
	HGLOBAL				hToolbar	    = ( HGLOBAL )NULL;
	TOOLBARDATA*			pthis		    = ( TOOLBARDATA* )NULL;
	TLBITEM*			pToolBItmList	    = ( TLBITEM* )NULL;
	TLBITEM*			pItem		    = ( TLBITEM* )NULL;
	int				ErrorCode	    = 0;
	
	if ( ! ( hToolbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLBARDATA *) GlobalLock ( hToolbar ) ) )
		ErrorCode = WTLB_ERR_GLOBALLOCK;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	if ( ( pthis->ItmSelected = Toolbr_IBtnFromPt ( hWnd, pthis, x, y, NULL ) ) >= 0 )
	{
		RECT	rcBtn;
        /*pthis->ItmSelected = BtnClicked;*/
		pItem = pToolBItmList + pthis->ItmSelected;
		
        if ( !( pItem->Btn.fsStyle & TBSTYLE_SEP ) )
		{
			if ( pthis->uStyles & TBSTYLE_ALTDRAG )
			{
				cKeyBuf[VK_MENU] = 0;
#ifdef _WINDOWS
                if ( GetKeyboardState ( cKeyBuf ) )
#else
                GetKeyboardState ( cKeyBuf ); 
#endif
				    pthis->enDrag = ( cKeyBuf[VK_MENU] & 0x80 )? eShftLBtnDown :
									     eLBtnDown;
			}
			else	
			    pthis->enDrag = keyFlags & MK_SHIFT ? eShftLBtnDown : eLBtnDown;
		}

		if ( pthis->enDrag == eLBtnDown &&
             !( pItem->Btn.fsStyle & TBSTYLE_SEP ) &&
		     !( pItem->Btn.fsState & TBSTATE_INDETERMINATE ) &&
		     ( pItem->Btn.fsState & TBSTATE_ENABLED ) 
		   )
		{
			if ( pItem->Btn.fsStyle & TBSTYLE_GROUP )
			{
				Toolbr_IResetCheckGroup ( hWnd, pToolBItmList, pthis->ItmSelected, 
								pthis->ItmCount );
				pItem->Btn.fsState |= TBSTATE_PRESSED;
			}
			else
			if ( pItem->Btn.fsStyle & TBSTYLE_CHECK )
				pItem->Btn.fsState ^= TBSTATE_PRESSED;
			else
				pItem->Btn.fsState |= TBSTATE_PRESSED;
			CopyRect ( &rcBtn, &pItem->Rect );
			InflateRect ( &rcBtn, 1, 1 );
			InvalidateRect ( hWnd, &rcBtn, TRUE );
			UpdateWindow ( hWnd );
		}

	}

	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );
	if ( pthis )
		GlobalUnlock ( hToolbar );
}


static void Toolbr_OnLButtonUp 
(	
	HWND				hWnd, 
	int				    x, 
	int				    y, 
	UINT				keyFlags
) 
{
	RECT				rcBtn;
	HGLOBAL				hToolbar	    = ( HGLOBAL )NULL;
	TOOLBARDATA*		pthis		    = ( TOOLBARDATA* )NULL;
	TLBITEM*			pToolBItmList	= ( TLBITEM* )NULL;
	TLBITEM*			pItem		    = ( TLBITEM* )NULL;
	BOOL				bSendMessage	= FALSE;
	BOOL				bInsert		    = FALSE;

	int				    ErrorCode	    = 0;
	
	if ( ! ( hToolbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLBARDATA* ) GlobalLock ( hToolbar ) ) )
		ErrorCode = WTLB_ERR_GLOBALLOCK;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
    if ( pthis->ItmSelected >= 0 )
	{
		pItem = pToolBItmList + pthis->ItmSelected;
		if ( !( pItem->Btn.fsState & TBSTATE_INDETERMINATE ) &&
		     ( pItem->Btn.fsState & TBSTATE_ENABLED ) &&
                     !( pItem->Btn.fsStyle & TBSTYLE_SEP )
		   )
		{
			if ( pItem->Btn.fsStyle ==  TBSTYLE_BUTTON )
			{
				pItem->Btn.fsState ^= TBSTATE_PRESSED;
				CopyRect ( &rcBtn, &pItem->Rect );
				InflateRect ( &rcBtn, 1, 1 );
				InvalidateRect ( hWnd, &rcBtn, TRUE );
				UpdateWindow ( hWnd );
			}
			bSendMessage = TRUE;
		}

		if ( pthis->enDrag == eLBtnDown )
		{
			ReleaseCapture ();
			pthis->enDrag = eEndDrag;
		}
		if ( pthis->enDrag != eNoDrag )
			bSendMessage = FALSE;

		Toolbr_IEndDrag ( hWnd, pthis, &bInsert );
		if ( bSendMessage )
			SendMessage ( GetParent ( hWnd ), WM_COMMAND, 
				MAKELONG ( pItem->Btn.idCommand, 0 ), 0L );

	}

	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );
	if ( bInsert )
	{
		TLBHITTEST	tbht;
		Toolbr_IBtnFromPt ( hWnd, pthis, x, y, &tbht ); 
		Toolbr_IResetLayout ( hWnd, pthis, tbht );
	}
	if ( pthis )
		GlobalUnlock ( hToolbar );
}

static void 
Toolbr_OnDestroy 
(	
	HWND				hWnd 
)
{
	HGLOBAL				hToolbar	= ( HGLOBAL )NULL;
	TOOLBARDATA*			pthis		= ( TOOLBARDATA* )NULL;

	int				ErrorCode	= 0;

	if ( ! ( hToolbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLBARDATA *) GlobalLock ( hToolbar ) ) )
		ErrorCode = WTLB_ERR_GLOBALLOCK;
	else
	{
		REMOVETHISDATA ( hWnd );
		WLDestroy ( &pthis->ItmList );
		if ( pthis->hFont )
			DeleteObject ( pthis->hFont );
		DeleteDC ( pthis->hDCGlyphs );
		DeleteDC ( pthis->hDCMono );

		/*DeleteObject ( pthis->hbrDither );*/
		GlobalFree ( hToolbar );
	}
}

/* to be called by customize dialog WM_MEASUREITEM event handler only */
void 
Toolbr_OnMeasureItem 
(
	HWND			    hWnd, 
	LPMEASUREITEMSTRUCT	lpmi
) 
{
	HGLOBAL				hToolbar	= ( HGLOBAL )NULL;
	TOOLBARDATA*		pthis		= ( TOOLBARDATA* )NULL;
    int                 ErrorCode   = 0;

	if ( ! ( hToolbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLBARDATA *) GlobalLock ( hToolbar ) ) )
		ErrorCode = WTLB_ERR_GLOBALLOCK;
    else
        lpmi->itemHeight = pthis->ItmHeight + 2 * VERT_PADDING;

    if ( pthis )
		GlobalUnlock ( hToolbar );
}

static void 
Toolbr_OnMouseMove 
(
	HWND			hWnd, 
	UINT			wParam,
	LPARAM			lParam 
) 
{
	POINT			pt;
	TLBITEM*		pItemList	= ( TLBITEM* )NULL;
	TLBITEM*		pItem		= ( TLBITEM* )NULL;
	HGLOBAL			hToolbar	= ( HGLOBAL )NULL;
	TOOLBARDATA*		pthis		= ( TOOLBARDATA* )NULL;
	int		    	i	    	= 0;
	int		    	ErrorCode	= 0;

	if ( ! ( hToolbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLBARDATA *) GlobalLock ( hToolbar ) ) )
		ErrorCode = WTLB_ERR_GLOBALLOCK;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItemList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	{
		pt.x = LOWORD ( lParam );
		pt.y = HIWORD ( lParam );
		for ( i = 0; i < pthis->ItmCount; i++ )
		{
			pItem = pItemList + i;
			if ( PtInRect ( &pItem->Rect, pt ) && 
                             !( pItem->Btn.fsStyle & TBSTYLE_SEP ) )
			{
				if ( pthis->hwTTip ) 
				{
					MSG msg;
					
					if ( pthis->ItmTip != pItem->Btn.idCommand )
					{
						Tooltip_SetToolbarItemID ( pthis->hwTTip, hWnd,
							pItem->Btn.idCommand, &pItem->Rect );
						pthis->ItmTip = pItem->Btn.idCommand;
					}
					msg.hwnd = hWnd;
					msg.message = WM_MOUSEMOVE;
					msg.wParam  = wParam;
					msg.lParam  = lParam;
					SendMessage ( pthis->hwTTip, TTM_RELAYEVENT, 0, 
					    ( LPARAM ) ( LPMSG )&msg); 
				}

				if ( pthis->uStyles & CCS_ADJUSTABLE )
					Toolbr_IBeginDrag ( hWnd, pthis, pItem, i );
				break;
			}
		}
	}

	if ( pItemList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pItemList );

	if ( pthis )
		GlobalUnlock ( hToolbar );
}


#ifdef 0
static void Toolbr_OnNcPaint ( HWND hWnd, UINT uCtlID, LPARAM lParam ) 
{ 

	TLBITEM*			pItemList	= ( TLBITEM* )NULL;
	HGLOBAL				hToolbar	= ( HGLOBAL )NULL;
	TOOLBARDATA*			pthis		= ( TOOLBARDATA* )NULL;
	int				ErrorCode	= 0;

	LPTOOLTIPTEXT			lpttt; 

	return;
	if ( ! ( hToolbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLBARDATA *) GlobalLock ( hToolbar ) ) )
		ErrorCode = WTLB_ERR_GLOBALLOCK;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItemList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	if ( ( ( ( LPNMHDR ) lParam )->code ) == TTN_NEEDTEXT ) 
	{ 
		lpttt = ( LPTOOLTIPTEXT )lParam;

	} 

	if ( pthis )
		GlobalUnlock ( hToolbar );
	if ( pItemList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pItemList );
} 
#endif

static void Toolbr_OnNotify ( HWND hWnd, UINT uCtlID, LPARAM lParam ) 
{ 

	TLBITEM*			pItemList	= ( TLBITEM* )NULL;
	HGLOBAL				hToolbar	= ( HGLOBAL )NULL;
	TOOLBARDATA*			pthis		= ( TOOLBARDATA* )NULL;
	int				ErrorCode	= 0;

	LPTOOLTIPTEXT			lpttt; 

	if ( ! ( hToolbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLBARDATA *) GlobalLock ( hToolbar ) ) )
		ErrorCode = WTLB_ERR_GLOBALLOCK;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItemList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	if ( ( ( ( LPNMHDR ) lParam )->code ) == TTN_NEEDTEXT ) 
	{ 
		lpttt = ( LPTOOLTIPTEXT )lParam;
		lpttt->hdr.idFrom = pthis->ItmTip;
		lpttt->uFlags &= ~TTF_IDISHWND;
		SendMessage ( GetParent ( hWnd ), WM_NOTIFY, ( WPARAM )pthis->ItmTip,
				( LPARAM )lpttt );
	} 

	if ( pItemList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pItemList );
	if ( pthis )
		GlobalUnlock ( hToolbar );
} 

static void Toolbr_DrawBorder
(
	HWND			hWnd,
	HDC			hDC,
	UINT			uStyle
)
{
	RECT				rc;
	HPEN				hbPen	= CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNSHADOW ) );
	HPEN				hOPen;

	GetClientRect ( hWnd, &rc );
	hOPen = SelectObject ( hDC, hbPen );
	MoveToEx ( hDC, rc.left, rc.top, NULL );
	LineTo ( hDC, rc.right, rc.top );
	SelectObject ( hDC, GetStockObject ( WHITE_PEN ) );
	MoveToEx ( hDC, rc.left, rc.top + 1, NULL );
	LineTo ( hDC, rc.right, rc.top + 1 );
	SelectObject ( hDC, hOPen );
	
	DeleteObject ( hbPen );
}

static void Toolbr_OnPaint 
(
	HWND				hWnd, 
	HDC				hPaintDC 
)
{
	PAINTSTRUCT			ps;
	TLBITEM*			pToolBItmList	= ( TLBITEM* )NULL;
	TLBITEM*			pItem		= ( TLBITEM* )NULL;
	TLBSTR*	 			pToolBStrList	= ( TLBSTR* )NULL;
	TLBSTR*				pStr		= ( TLBSTR* )NULL;
	HGLOBAL				hToolbar	= ( HGLOBAL )NULL;
	TOOLBARDATA*			pthis		= ( TOOLBARDATA* )NULL;
	HFONT				OldFont		= ( HFONT )NULL;
	int				ErrorCode	= 0;

	BeginPaint ( hWnd, &ps );
	if ( !IsWindowVisible ( hWnd ) )
		goto EndToolbarPaint;
	if ( ! ( hToolbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLBARDATA *) GlobalLock ( hToolbar ) ) )
		ErrorCode = WTLB_ERR_GLOBALLOCK;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	if ( WLLock ( &pthis->StrList, ( LPVOID )&pToolBStrList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	{
		Toolbr_DrawBorder ( hWnd, ps.hdc, pthis->uStyles );
		if ( pthis->ItmCount > 0 )
		{
			int i;
			TLBGDI	tbg;
            		Toolbr_IBeginPaintBtn ( pthis, &tbg );
			OldFont = SelectObject ( ps.hdc, pthis->hFont );
			SetBkMode ( ps.hdc, TRANSPARENT );
			for ( i = 0; i < pthis->ItmCount ; i++ )
			{
				pItem = pToolBItmList + i;
				if ( ( pItem->Btn.fsStyle & TBSTYLE_SEP ) ||
				     ( pItem->Btn.fsState & TBSTATE_HIDDEN ) )
					continue;
				if ( pItem->Btn.iString < pthis->StrCount )
					pStr = pToolBStrList + pItem->Btn.iString;
				else
					pStr = NULL;
				Toolbr_IPaintBtn ( hWnd, ps.hdc, pthis, pItem, pStr ); 
			}


			SelectObject ( ps.hdc, OldFont );
			Toolbr_IEndPaintBtn ( pthis, &tbg );
		}

	}


	if ( pToolBStrList )
		WLUnlock ( &pthis->StrList, ( LPVOID )&pToolBStrList );
	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );
	if ( pthis )
		GlobalUnlock ( hToolbar );
EndToolbarPaint:
	EndPaint ( hWnd, &ps );
}

static int 
Toolbr_OnSize 
(
	HWND			hWnd,
	DWORD			fwSizeType,
	int			nWidth,
	int			nHeight
) 
{
	HGLOBAL				hToolbar	= ( HGLOBAL )NULL;
	TOOLBARDATA*			pthis		= ( TOOLBARDATA* )NULL;
	int				ErrorCode	= 0;

	if ( ! ( hToolbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLBARDATA *) GlobalLock ( hToolbar ) ) )
		ErrorCode = WTLB_ERR_GLOBALLOCK;
	else
	{
		Toolbr_AutoSize ( hWnd, pthis );
	}

	if ( pthis )
		GlobalUnlock ( hToolbar );

	return 0;	
}

static int 
Toolbr_OnWndPosChanging 
(
	HWND			hWnd,
	LPWINDOWPOS		lpWndPos
) 
{
	HGLOBAL				hToolbar	= ( HGLOBAL )NULL;
	TOOLBARDATA*			pthis		= ( TOOLBARDATA* )NULL;
	int				ErrorCode	= 0;

	if ( ! ( hToolbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLBARDATA *) GlobalLock ( hToolbar ) ) )
		ErrorCode = WTLB_ERR_GLOBALLOCK;
	else
	if ( pthis->bStandardBmp )
	{
		lpWndPos->cy = pthis->Size.cy;
	}

	if ( pthis )
		GlobalUnlock ( hToolbar );

	return 0;	
}

/* create a mono bitmap mask:*/
#if 1
static void 
ToolBr_ICreateMask
(
	TOOLBARDATA*			pthis,
	int				iImage, 
	POINT				ptOffset,
	BOOL				bHilite, 
	BOOL				bHiliteShadow
)
{
	/* initalize whole area with 0's				      */
	PatBlt ( pthis->hDCMono, 0, 0, pthis->ImgCX - 2, pthis->ImgCY - 2, WHITENESS );

	/* create mask based on color bitmap*/
	/* convert this to 1's*/
	SetBkColor ( pthis->hDCGlyphs, GetSysColor ( COLOR_BTNFACE ) );
	BitBlt ( pthis->hDCMono, ptOffset.x, ptOffset.y, pthis->ImgCX, pthis->ImgCY,
		 pthis->hDCGlyphs, iImage * pthis->ImgCX, 0, SRCCOPY);

	if (bHilite)
	{
		/* convert this to 1's*/
		SetBkColor ( pthis->hDCGlyphs, RGB ( 255, 255, 255 ) );
		/* OR in the new 1's*/
		BitBlt ( pthis->hDCMono, ptOffset.x, ptOffset.y, pthis->ImgCX, pthis->ImgCY,
			 pthis->hDCGlyphs, iImage * pthis->ImgCX, 0, SRCPAINT);

		if ( bHiliteShadow )
			BitBlt ( pthis->hDCMono, 1, 1, pthis->ImgCX - 3, pthis->ImgCY - 3,
				 pthis->hDCMono, 0, 0, SRCAND);
	}
}
#endif


static void
Toolbr_IDrawBtnBorder 
( 
	HDC				hDC, 
	UINT				uState, 
	RECT*				pRect
)
{
	HPEN				hwPen	= CreatePen ( PS_SOLID, 1, RGB ( 255,255,255 ) );
	HPEN				hbPen	= CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNSHADOW ) );
	HPEN				hsdPen  = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNTEXT ) );
	HPEN				htmpPen = ( HPEN )NULL;

	if ( uState & TBSTATE_PRESSED )
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

	SelectObject ( hDC, htmpPen );

	/*cleanup*/
	DeleteObject ( hwPen );
	DeleteObject ( hbPen );
	DeleteObject ( hsdPen );

}

static void
Toolbr_IGetBtnRects 
( 
	HDC				hDC,
	TOOLBARDATA*			pthis,
	RECT*   			prcBtn,
    	BYTE                		fsState,
	RECT*				prcIcon,
	RECT*				prcText
)
{                                             
	SIZE				Off;

	CopyRect ( prcIcon, prcBtn );
    	if ( prcText )
	    CopyRect ( prcText, prcIcon );

	Off.cx = ( pthis->ItmWidth - pthis->ImgCX ) >> 1;
	if ( pthis->StrCount > 0 )
		Off.cy = 2 * pthis->nYPadding;
	else
		Off.cy = ( pthis->ItmHeight - pthis->ImgCY ) >> 1;
	if ( fsState & TBSTATE_PRESSED )
	{
		Off.cx += 1;
		Off.cy += 1;
	}
	OffsetRect ( prcIcon, Off.cx, Off.cy );

    	if ( prcText )
    		prcText->top += pthis->ImgCY + 2 * pthis->nYPadding;
}

static void
Toolbr_IPaintDragLbxItem 
( 
	HWND				hWnd,
	LPDRAWITEMSTRUCT		lpdi, 
	TOOLBARDATA*			pthis, 
	TLBITEM*			pItem,
	LPSTR				pStr
)
{
    	RECT                		rcIcon;
	RECT				rcLabel;
	RECT				rcBtn;
    	HDC                 		hDC;
	HBRUSH              		hBkBr;
    	BYTE                		fsState = 0;
    	COLORREF            		txtColor;
	HPEN				hbPen	= CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNSHADOW ) );
	HPEN				htxtPen = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNTEXT ) );
	HPEN				hSelPen = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_HIGHLIGHT ) );
	HPEN				htmpPen = ( HPEN )NULL;
    	HBRUSH              		hHLBr   = CreateSolidBrush ( GetSysColor ( COLOR_HIGHLIGHT ) );

    	hDC = lpdi->hDC;
    	SetBkMode ( hDC, TRANSPARENT ); 
    	hBkBr = CreateSolidBrush ( GetBkColor ( hDC ) );

    	FillRect ( hDC, &lpdi->rcItem, hBkBr );
	txtColor = RGB ( 0, 0, 0 );
    	if ( ( lpdi->itemState & ODS_SELECTED ) &&
    	     !( lpdi->itemState & ODS_FOCUS )  )
    	{
    		htmpPen = SelectObject ( hDC, hSelPen );
    	        Rectangle ( hDC, lpdi->rcItem.left, lpdi->rcItem.top,
    	                         lpdi->rcItem.right, lpdi->rcItem.bottom );
    	        SelectObject ( hDC, htmpPen );
    	}
    	else
    	if ( ( lpdi->itemState & ODS_SELECTED ) &&
    	     ( lpdi->itemState & ODS_FOCUS )  )
    	{
    	        FillRect ( hDC, &lpdi->rcItem, hHLBr );
    	        DrawFocusRect ( hDC, &lpdi->rcItem );
		txtColor = RGB ( 255,255,255 );
    	}
    	SetRect ( &rcLabel, lpdi->rcItem.left + pthis->ItmWidth + 5, lpdi->rcItem.top,
                        lpdi->rcItem.right, lpdi->rcItem.bottom );
    	SetRect ( &rcBtn, lpdi->rcItem.left, lpdi->rcItem.top,
    	                  lpdi->rcItem.left + pthis->ItmWidth,
    	                  lpdi->rcItem.bottom
    	        );
    	InflateRect ( &rcBtn, 0, -1 * VERT_PADDING );
    	OffsetRect ( &rcBtn, 2, 0 );
    	FillRect ( hDC, &rcBtn, GetStockObject ( LTGRAY_BRUSH ) );

    	if ( pItem ) /* if not a list box separator item */
    	{
    	    Toolbr_IGetBtnRects ( hDC, pthis, &rcBtn,
    	                      pItem->Btn.fsState, &rcIcon, NULL );
    	    if ( pItem->Btn.fsState & TBSTATE_INDETERMINATE )
	    {
		    HGDIOBJ hbrOld;
		    POINT ptOffset;
       	     	    COLORREF txtColor, bkColor;
		    ptOffset.x = ptOffset.y = 0;
		    ToolBr_ICreateMask ( pthis, pItem->Btn.iBitmap, ptOffset, TRUE, FALSE);

		    txtColor = SetTextColor(hDC, 0L);                  /* 0's in mono -> 0 (for ROP)*/
		    bkColor = SetBkColor(hDC, (COLORREF)0x00FFFFFFL); /* 1's in mono -> 1*/

		    hbrOld = SelectObject(hDC, GetStockObject ( WHITE_BRUSH ) );
		    if (hbrOld != ( HGDIOBJ )NULL)
		    {
			    /* draw hilight color where we have 0's in the mask*/
			    BitBlt ( hDC, rcIcon.left, rcIcon.top,
				     pthis->ImgCX, pthis->ImgCY,
				     pthis->hDCMono, 0, 0, 0x00B8074AL);
			    SelectObject(hDC, hbrOld);
		    }

		    /*BLOCK: always draw the shadow*/
		    {
			    HGDIOBJ hbrOld = CreateSolidBrush ( GetSysColor ( COLOR_BTNSHADOW ) );
			    hbrOld = SelectObject(hDC, hbrOld );
			    if ( hbrOld != ( HGDIOBJ )NULL )
			    {
				    /* draw the shadow color where we have 0's in the mask*/
				    BitBlt ( hDC, rcIcon.left, rcIcon.top, pthis->ImgCX-2, pthis->ImgCY-2,
				             pthis->hDCMono, 0, 0, 0x00B8074AL );
				    hbrOld = SelectObject ( hDC, hbrOld );
				    DeleteObject ( hbrOld );
			    }
		    }

            	SetTextColor ( hDC, txtColor );
            	SetBkColor ( hDC, bkColor );
	    }
            else
	    {
		    if ( pthis->hbmImage && !( pItem->Btn.fsStyle & TBSTYLE_SEP ) && pItem->Btn.iBitmap != -1 ) 
			    BitBlt ( hDC, rcIcon.left, rcIcon.top, pthis->ImgCX, pthis->ImgCY, 
				     pthis->hDCGlyphs, pItem->Btn.iBitmap * pthis->ImgCX, 0, SRCCOPY );
	    }
    	}

	if ( pStr && *pStr != 0 )
	{
		HFONT	hOldFont = ( HFONT )NULL;
#if 1
        if ( pItem &&
             ( !( pItem->Btn.fsState & TBSTATE_ENABLED ) ||
	       ( pItem->Btn.fsState & TBSTATE_INDETERMINATE ) 
             )
           )
			SetTextColor ( hDC, GetSysColor ( COLOR_BTNSHADOW ) );
#endif
		hOldFont = SelectObject ( hDC, pthis->hFont );
		SetTextColor ( hDC, txtColor );
		DrawText ( hDC, pStr, -1, &rcLabel, DT_SINGLELINE | DT_LEFT | DT_VCENTER );
		SelectObject ( hDC, hOldFont );
	/*	SetTextColor ( hDC, GetSysColor ( COLOR_BTNTEXT ) );*/
	}

    	fsState = pItem ? pItem->Btn.fsState : TBSTATE_INDETERMINATE;
	Toolbr_IDrawBtnBorder ( hDC, fsState, &rcBtn );

	/*cleanup*/
	DeleteObject ( hbPen );
	DeleteObject ( htxtPen );
	DeleteObject ( hSelPen );
    	DeleteObject ( hHLBr );
    	DeleteObject ( hBkBr );
}


/*new*/
static void
Toolbr_IPaintBtn 
( 
	HWND				hWnd,
	HDC				    hDC, 
	TOOLBARDATA*		pthis, 
	TLBITEM*			pItem,
	TLBSTR*				pStr
)
{
	RECT				rcLabel;
	RECT				rcIcon;
	HPEN				hbPen	= CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNSHADOW ) );
	HPEN				htxtPen = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNTEXT ) );
	RECT*				pRect   = &pItem->Rect;

	FillRect ( hDC, pRect, GetStockObject ( LTGRAY_BRUSH ) );

	/*draw content of a toolbar button*/
	Toolbr_IGetBtnRects ( hDC, pthis, &pItem->Rect,
                          pItem->Btn.fsState, &rcIcon, &rcLabel );
	if ( pItem->Btn.fsState & TBSTATE_ENABLED )
	{
		if ( pthis->hbmImage && pItem->Btn.iBitmap != -1 )
			BitBlt ( hDC, rcIcon.left, rcIcon.top, pthis->ImgCX, pthis->ImgCY, 
				 pthis->hDCGlyphs, pItem->Btn.iBitmap * pthis->ImgCX, 0, SRCCOPY );
	}
	else
	{
		HGDIOBJ hbrOld;
		POINT ptOffset;
		ptOffset.x = ptOffset.y = 0;
		ToolBr_ICreateMask ( pthis, pItem->Btn.iBitmap, ptOffset, TRUE, FALSE);

		SetTextColor(hDC, 0L);                  /* 0's in mono -> 0 (for ROP)*/
		SetBkColor(hDC, (COLORREF)0x00FFFFFFL); /* 1's in mono -> 1*/

		hbrOld = SelectObject(hDC, GetStockObject ( WHITE_BRUSH ) );
		if (hbrOld != ( HGDIOBJ )NULL)
		{
			/* draw hilight color where we have 0's in the mask*/
			BitBlt ( hDC, rcIcon.left, rcIcon.top,
				 pthis->ImgCX, pthis->ImgCY,
				 pthis->hDCMono, 0, 0, 0x00B8074AL);
			SelectObject(hDC, hbrOld);
		}

		/*BLOCK: always draw the shadow*/
		{
			HGDIOBJ hbrOld = CreateSolidBrush ( GetSysColor ( COLOR_BTNSHADOW ) );
			hbrOld = SelectObject(hDC, hbrOld );
			if (hbrOld != ( HGDIOBJ )NULL)
			{
				/* draw the shadow color where we have 0's in the mask*/
				BitBlt ( hDC, rcIcon.left, rcIcon.top, pthis->ImgCX-2, pthis->ImgCY-2,
				         pthis->hDCMono, 0, 0, 0x00B8074AL );
				hbrOld = SelectObject ( hDC, hbrOld );
				DeleteObject ( hbrOld );
			}
		}
	}


	if ( pStr )
	{
		if ( !( pItem->Btn.fsState & TBSTATE_ENABLED ) ||
		     ( pItem->Btn.fsState & TBSTATE_INDETERMINATE ) )
			SetTextColor ( hDC, GetSysColor ( COLOR_BTNSHADOW ) );
		DrawText ( hDC, pStr->Label, -1, &rcLabel, DT_SINGLELINE | DT_CENTER );
		SetTextColor ( hDC, GetSysColor ( COLOR_BTNTEXT ) );
	}

	Toolbr_IDrawBtnBorder ( hDC, pItem->Btn.fsState, pRect );

	/*cleanup*/
	DeleteObject ( hbPen );
	DeleteObject ( htxtPen );
}


static BOOL
Toolbr_SetState
(
	HWND				hwToolbar,
	TOOLBARDATA*			pthis, 
	int				idButton,
	int				State
)
{
	TLBITEM*			pToolBItmList	= ( TLBITEM* )NULL;
	int				ErrorCode	= 0;

	if ( ( idButton = Toolbr_CommandToIndex ( hwToolbar, pthis, idButton ) ) == -1 )
		ErrorCode = WTLB_ERR_OUTOFRANGE;
	else
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pToolBItmList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
		( pToolBItmList + idButton )->Btn.fsState = State;

	if ( pToolBItmList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pToolBItmList );

	return ( ErrorCode >= 0 );
}

/*
 * the toolbar would register itself with the tooltip. There's no need ( and there's no way )
 * to register individual toolbar buttons. 
 */
static void
Toolbr_SetTooltip 
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
/*TTF_IDISHWND needed because ti.uId is supposed to be the id of a child*/
/*control of ti.hwnd and in this case ti.hwnd and ti.uId are both the same window*/
		ti.uFlags = 0; /*TTF_IDISHWND; */
		ti.hwnd = hWnd; 
		ti.uId = ( UINT )hWnd; 
        GetClientRect ( hWnd, &ti.rect );
		ti.lpszText = LPSTR_TEXTCALLBACK; 
		SendMessage ( hwTTip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti ); 
		SendMessage ( hwTTip, TTM_ACTIVATE, ( WPARAM )TRUE, 0L );
	}

}


/*----------------------------------------------------------------------------------------------*
  Parameters:
  phbmImages : pointer to HBITMAP which is a collection of images
  pImgWidth  : width of each individual image.
  pImgHeight : height of each individual image.
  hBitmap    : a new collection of images to be added
  NoOfImages : number of new images being added 
 *----------------------------------------------------------------------------------------------*/
static BOOL 
W_IAddImage
( 
	HBITMAP*			phbmImages,
	int*				pImgWidth,
	int*				pImgHeight,
	int*				pImgCount, 
	HBITMAP				hBitmap,
	int				NoOfImages
)

{
	BITMAP				Bitmap;
	HDC				hMemoryDC	= ( HDC )NULL;
	HBITMAP				hTempBitmap	= ( HBITMAP )NULL;
	HBITMAP				hbmpOld		= ( HBITMAP )NULL;
	HBITMAP				hOld		= ( HBITMAP )NULL;
	HDC				hTempDC		= ( HDC )NULL;
	HDC				hDeskDC		= GetDC ( GetDesktopWindow () );
	int				NewWidth	= 0;
	int				ErrorCode	= 0;


	memset ( &Bitmap, 0, sizeof ( BITMAP ) );

	if ( ! ( GetObject ( hBitmap, sizeof ( BITMAP ), &Bitmap ) ) )
		ErrorCode = WIL_ERR_GETOBJECT;
	else
	if ( ! ( hMemoryDC = CreateCompatibleDC ( hDeskDC ) ) )
		ErrorCode = WIL_ERR_CREATECOMPATIBLEDC;
	else
	if ( ! ( hTempDC = CreateCompatibleDC ( hDeskDC ) ) )
		ErrorCode = WIL_ERR_CREATECOMPATIBLEDC;
	else
	{
		NewWidth = *pImgWidth;

		if ( ! ( hTempBitmap = CreateBitmap ( NewWidth * ( NoOfImages + *pImgCount ),
			 *pImgHeight, Bitmap.bmPlanes, Bitmap.bmBitsPixel, ( void* )NULL ) ) )
			ErrorCode = WIL_ERR_CREATEBITMAP;
		else
		{
			hbmpOld = SelectObject ( hTempDC, *phbmImages );
			hOld = SelectObject( hMemoryDC, hTempBitmap );
			BitBlt ( hMemoryDC, 0, 0, *pImgCount * NewWidth, *pImgHeight,
				 hTempDC, 0, 0, SRCCOPY );
			
			SelectObject ( hTempDC, hBitmap );
			BitBlt ( hMemoryDC, *pImgCount * NewWidth, 0, 
				 NewWidth * NoOfImages, *pImgHeight,
				 hTempDC, 0, 0, SRCCOPY );
			SelectObject ( hTempDC, hbmpOld );
			SelectObject( hMemoryDC, hOld );
			DeleteObject ( *phbmImages );
			*phbmImages = hTempBitmap;
		}
	}

	if ( hTempDC )
		DeleteDC ( hTempDC );

	if ( hMemoryDC )
		DeleteDC ( hMemoryDC );

	if ( hDeskDC )
		ReleaseDC ( GetDesktopWindow (), hDeskDC );

	return ( ErrorCode == 0 );

}

static BOOL
Toolbr_IBeginDrag 
(
	HWND				hWnd,
	TOOLBARDATA*			pthis,
	TLBITEM*			pItem,
	int				idBtn
)
{
	if ( pthis->enDrag == eBeginDrag || pthis->enDrag == eShftBeginDrag)
	{
		pthis->tbn.hdr.hwndFrom = hWnd;
		pthis->tbn.hdr.idFrom = idBtn;
		pthis->tbn.iItem = idBtn;
		pthis->tbn.tbButton = pItem->Btn;
		pthis->tbn.cchText = Toolbr_GetButtonText ( hWnd, pthis, idBtn, 
						pthis->cBtnText );
		pthis->tbn.pszText = pthis->cBtnText;
		SetCapture ( hWnd );
	}
	
	if ( pthis->enDrag == eLBtnDown )
		pthis->enDrag = eBeginDrag;
	else
	if ( pthis->enDrag == eShftLBtnDown )
		pthis->enDrag = eShftBeginDrag;
	else
	if ( pthis->enDrag == eBeginDrag )
	{
		pthis->tbn.hdr.code = TBN_BEGINDRAG;
		SendMessage ( GetParent ( hWnd ), WM_NOTIFY, 0, ( LPARAM )&pthis->tbn );
		pthis->enDrag = eDragging;
	}
	else
	if ( pthis->enDrag == eShftBeginDrag )
	{
		pthis->tbn.hdr.code = TBN_QUERYDELETE;
		if ( SendMessage ( GetParent ( hWnd ), WM_NOTIFY, 0, ( LPARAM )&pthis->tbn ) )
		{
			pthis->enDrag = eShftDragging;
			if ( !g_hDragCursor )
#ifdef _WINDOWS
				g_hDragCursor = LoadCursor ( ( HINSTANCE )GetWindowLong ( GetParent ( hWnd ),GWL_HINSTANCE ), 
						MAKEINTRESOURCE ( IDC_CURSOR_DRAG ) );
#else
				g_hDragCursor = LoadCursor ( hLibInstance, 
						MAKEINTRESOURCE ( IDC_CURSOR_DRAG ) );
#endif
			if ( g_hDragCursor )
				g_hDefCursor = SetCursor ( g_hDragCursor );
		}
		else
			pthis->enDrag = eCancelDrag;
	}

	return TRUE;
}

static BOOL
Toolbr_IEndDrag 
(
	HWND				hWnd,
	TOOLBARDATA*			pthis,
	BOOL*				pbInsert
)
{
	*pbInsert		= FALSE;
	if ( pthis->enDrag == eDragging )
	{
		ReleaseCapture ();
		pthis->enDrag = eEndDrag;
		pthis->tbn.hdr.code = TBN_ENDDRAG;
		SendMessage ( GetParent ( hWnd ), WM_NOTIFY, 0, ( LPARAM )&pthis->tbn );
	}
	else
	if ( pthis->enDrag == eShftDragging )
	{
		ReleaseCapture ();
		SetCursor ( g_hDefCursor );
		pthis->enDrag = eEndDrag;
		pthis->tbn.hdr.code = TBN_QUERYINSERT;
		if ( SendMessage ( GetParent ( hWnd ), WM_NOTIFY, 0, ( LPARAM )&pthis->tbn ) )
			*pbInsert = TRUE;
	}
	else
	if ( pthis->enDrag == eCancelDrag )
		ReleaseCapture ();

	return TRUE;
}

static void
Toolbr_IResetLayout
(
	HWND				hWnd,
	TOOLBARDATA*			pthis,
	TLBHITTEST			tbHitTest
)
{
    	NMHDR               		Notify;
	TBBUTTON			MovedBtn;
	TBBUTTON			SepBtn;
	int			    	iBtnAfter	= tbHitTest.HitBtn;
	int			    	iHitRow		= tbHitTest.HitRow;

	if ( iBtnAfter == pthis->ItmSelected )
		return;
	memset ( &SepBtn, 0, sizeof ( TBBUTTON ) );
	SepBtn.fsStyle = TBSTYLE_SEP;
	SepBtn.idCommand = -1;
	SepBtn.iBitmap = -1;
	SepBtn.iString = -1;

	Toolbr_GetButton ( hWnd, pthis, pthis->ItmSelected, &MovedBtn );
	if ( iHitRow == eHitOutside )
	{
		Toolbr_HideBtn ( hWnd, pthis, MovedBtn.idCommand, TRUE );
		return;
	}
	if ( WLDelete ( &pthis->ItmList, pthis->ItmSelected ) == 0 )
	{
		pthis->ItmCount--;
		if ( iBtnAfter < 0 )
		{
			if ( pthis->ItmSelected == pthis->ItmCount ) 
				Toolbr_InsertBtn ( hWnd, pthis, pthis->ItmCount, &SepBtn );
			iBtnAfter = pthis->ItmCount;
		}
		else
		if ( iBtnAfter > pthis->ItmSelected )
			iBtnAfter--;
		Toolbr_InsertBtn ( hWnd, pthis, iBtnAfter, &MovedBtn );
		InvalidateRect ( hWnd, NULL, TRUE );
		UpdateWindow ( hWnd );

        Notify.hwndFrom = hWnd;
        Notify.idFrom = GetDlgCtrlID ( hWnd );
        Notify.code = TBN_TOOLBARCHANGE;
        SendMessage ( GetParent ( hWnd ), WM_NOTIFY, Notify.idFrom, ( LPARAM )&Notify );
	}
}

void
Toolbr_MoveBtn
(
    HWND                hWnd,
    int                 iBtnMove,
    int                 iBtnAfter
)
{
    TLBHITTEST          tbht;
    HGLOBAL				hToolbar	    = ( HGLOBAL )NULL;
	TOOLBARDATA*		pthis		    = ( TOOLBARDATA* )NULL;
    int                 ErrorCode       = 0;

	if ( ! ( hToolbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLBARDATA *) GlobalLock ( hToolbar ) ) )
		ErrorCode = WTLB_ERR_GLOBALLOCK;
    else
    {
        pthis->ItmSelected = iBtnMove;
        tbht.HitBtn = iBtnAfter;
        Toolbr_IResetLayout ( hWnd, pthis, tbht );
    }

	if ( pthis )
		GlobalUnlock ( hToolbar );
}

/*reset a group of toolbar buttons so that they look undepressed*/
static void
Toolbr_IResetCheckGroup
(
	HWND				hWnd,
	TLBITEM*			pToolBItmList,
	int				nSelected,
	int				nBtnCount
)
{
	int				i;
	RECT				rcBtn;
	int				OldSelect	= -1;
	TLBITEM*			pItem		= ( TLBITEM* )NULL;
	BOOL				bDone		= FALSE;
	

	for ( i = nSelected - 1; i >= 0;i-- )
	{
		pItem = pToolBItmList + i;
		if ( !( pItem->Btn.fsStyle & TBSTYLE_GROUP ) )
			break;
		if ( pItem->Btn.fsState & TBSTATE_PRESSED )
		{
			bDone = TRUE;
			OldSelect = i;
			break;
		}
	}

	for ( i = nSelected + 1; !bDone && i < nBtnCount; i++ )
	{
		pItem = pToolBItmList + i;
		if ( !( pItem->Btn.fsStyle & TBSTYLE_GROUP ) )
			break;
		if ( pItem->Btn.fsState & TBSTATE_PRESSED )
		{
			bDone = TRUE;
			OldSelect = i;
			break;
		}
	}

	if ( OldSelect > -1 )
	{
		pItem = pToolBItmList + OldSelect;
		pItem->Btn.fsState ^= TBSTATE_PRESSED;
		CopyRect ( &rcBtn, &pItem->Rect );
		InflateRect ( &rcBtn, 1, 1 );
		InvalidateRect ( hWnd, &rcBtn, TRUE );
		UpdateWindow ( hWnd );
	}
}


/* need refinement */
static void
Toolbr_ISetItemRects 
( 
	HWND				hwToolbar,
	TOOLBARDATA*			pthis
)
{
	int				i;
	int				j;
	int				left;
	int				grpleft;
	int				top;
	int				Rows;
	int				Height		= 0;
	int				Count		= 0;
	int				grCount		= 0;
	int				nitWidth	= 0;
	BOOL				bLineBreak	= FALSE;
	TLBITEM*			pItemList	= ( TLBITEM* )NULL;
	TLBITEM*			pItem		= ( TLBITEM* )NULL;
	TLBITEM*			pgrItem		= ( TLBITEM* )NULL;

	Rows = 0;
	Count = pthis->ItmCount;
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItemList ) )
		return;

	
	left = pthis->nXPadding;
	top = pthis->uStyles & CCS_NODIVIDER ? pthis->nYPadding : 2 * pthis->nYPadding;
	for ( i = 0; i < Count; i++ )
	{
		pItem = pItemList + i;
		if ( pItem->Btn.fsState & TBSTATE_HIDDEN )
		{
			SetRectEmpty ( &pItem->Rect );
			continue;
		}
		if ( i >= grCount && pItem->Btn.fsStyle & TBSTYLE_GROUP )
		{
			for ( j = i, grpleft = left,grCount = 0; 
		      	      j < Count ; j++ )
			{
				pgrItem = pItemList + j;
				if ( pgrItem->Btn.fsState & TBSTATE_HIDDEN )
					continue;
				grpleft += pthis->ItmWidth + 1;
				if ( ( pgrItem->Btn.fsStyle & TBSTYLE_SEP ) ||
			     	     !( pgrItem->Btn.fsStyle & TBSTYLE_GROUP ) ||
			     	     ( grpleft > pthis->Size.cx )			     
			   	   )
				{
					grCount = j;
					break;
				}
				else
				if ( grpleft + left > pthis->Size.cx )
				{
					bLineBreak = TRUE;
					grCount = j;
					break;
				}
			}
		}
		bLineBreak = ( bLineBreak || ( left + pthis->ItmWidth > pthis->Size.cx ) ) &&
			     ( pthis->uStyles & TBSTYLE_WRAPABLE );
		if ( bLineBreak )
		{
			Rows++;
			top += ( pthis->ItmHeight + pthis->nYPadding );
			left = pthis->nXPadding;
			bLineBreak = FALSE;
		}
		nitWidth = ( pItem->Btn.fsStyle & TBSTYLE_SEP ) ? pItem->Btn.iBitmap : pthis->ItmWidth;
    		SetRect ( &pItem->Rect, left, top, left + nitWidth, top + pthis->ItmHeight );
		pItem->Row = Rows ;
		left += ( nitWidth + 1 );
		if ( pItem->Btn.fsState & TBSTATE_WRAP )
		{
			Rows++;
			top += ( pthis->ItmHeight + pthis->nYPadding );
			left = pthis->nXPadding;
		}
	}

	pthis->Rows = Rows + 1;
	Height = pthis->Rows * ( pthis->ItmHeight + 2 * pthis->nYPadding );
	Height += pthis->uStyles & CCS_NODIVIDER ? 0 : pthis->nYPadding;
	if ( Height != pthis->Size.cy )
	{
		RECT	rect;
		GetClientRect ( GetParent ( hwToolbar ), &rect );
		pthis->Size.cy = Height;
		pthis->Pos.x = rect.left;
		pthis->Pos.y = ((pthis->uStyles & CCS_BOTTOM) == CCS_BOTTOM) ?
				rect.bottom - pthis->Size.cy : rect.top;
#if 0
            	SetWindowPos ( hwToolbar, ( HWND )NULL, pthis->Pos.x, pthis->Pos.y,
			       pthis->Size.cx, pthis->Size.cy,
			       SWP_NOACTIVATE | SWP_NOZORDER );
#else
		MoveWindow ( hwToolbar, pthis->Pos.x, pthis->Pos.y,
				pthis->Size.cx,  30, TRUE );
#endif

	} 
	pthis->bNeedSetRects = FALSE;

	if ( pItemList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pItemList );
}

static int
Toolbr_IBtnFromPt 
( 
	HWND				hwToolbar,
	TOOLBARDATA*			pthis, 
	int				x,
	int				y,
	TLBHITTEST*			pHitTest
)
{
	POINT				pt;
	TLBHITTEST			tbHitTest;
	TLBITEM*			pItemList	= ( TLBITEM* )NULL;
	TLBITEM*			pItem		= ( TLBITEM* )NULL;
	int				i		= 1;
	int				Count		= 0;
	int				ErrorCode	= 0;
	
	tbHitTest.HitBtn = -1;
	tbHitTest.HitRow = eHitOutside;
	if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItemList ) )
		ErrorCode = WTLB_ERR_LISTLOCK;
	else
	{
		Count = pthis->ItmCount;
		pt.x = x;
		pt.y = y;
		for ( i = 0; i < Count; i++ )
		{
			pItem = pItemList + i;
			if ( y >= pItem->Rect.top && y <= pItem->Rect.bottom )
				tbHitTest.HitRow = pItem->Row;
			if ( PtInRect (	&pItem->Rect, pt ) )
			{
				tbHitTest.HitBtn = i;
				break;
			}
		}
	}

	if ( pHitTest )
		*pHitTest = tbHitTest;
	if ( pItemList )
		WLUnlock ( &pthis->ItmList, ( LPVOID )&pItemList );

	return ( ( ErrorCode >= 0 && i < Count ) ? i : -1 );

}


static LRESULT WINAPI 
Toolbr_WndProc
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
			if ( ! Toolbr_OnCreate ( hWnd, ( LPCREATESTRUCT )lParam ) )
				return ( LRESULT ) -1L;
			break;
		case WM_DESTROY:
			Toolbr_OnDestroy ( hWnd );
			break;
		
		case WM_GETDLGCODE:
			return ( LRESULT )DLGC_WANTARROWS | DLGC_WANTCHARS;
		
		case WM_LBUTTONDBLCLK:
			Toolbr_OnLButtonDblClk ( hWnd,(int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )( wParam ) );
			break;

		case WM_LBUTTONDOWN:
			Toolbr_OnLButtonDown ( hWnd,(int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )( wParam ) );
			break;

		case WM_LBUTTONUP:
			Toolbr_OnLButtonUp ( hWnd,(int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )( wParam ) );
			break;

		case WM_MOUSEMOVE:
			Toolbr_OnMouseMove ( hWnd, ( UINT )wParam, lParam );
			break;
#if 0
		case WM_NCCALCSIZE:
		case WM_NCACTIVATE:
		case WM_NCPAINT:
			Toolbr_OnNcPaint ( hWnd, ( UINT )wParam, lParam );
			break;
#endif
		case WM_NOTIFY:
			Toolbr_OnNotify ( hWnd, ( UINT )wParam, lParam );
			break;

		case WM_PAINT:
			Toolbr_OnPaint( hWnd, ( HDC ) wParam  );
			break;

		case WM_SIZE:
			Toolbr_OnSize( hWnd, ( DWORD )wParam, 
				( int )LOWORD ( lParam ), ( int )HIWORD ( lParam ) );
			break;
		case WM_WINDOWPOSCHANGING:
			return Toolbr_OnWndPosChanging ( hWnd, 
							( LPWINDOWPOS )lParam );
	
		case TB_ADDBITMAP:
		case TB_ADDBUTTONS:
		case TB_ADDSTRING:
		case TB_AUTOSIZE:
		case TB_BUTTONCOUNT:
		case TB_BUTTONSTRUCTSIZE:
		case TB_CHANGEBITMAP:
		case TB_CHECKBUTTON:
		case TB_COMMANDTOINDEX:
		case TB_CUSTOMIZE:
		case TB_DELETEBUTTON:
		case TB_ENABLEBUTTON:
		case TB_GETBITMAP:
		case TB_GETBITMAPFLAGS:
		case TB_GETBUTTON:
		case TB_GETBUTTONTEXT:
		case TB_GETITEMRECT:
		case TB_GETROWS:
		case TB_GETSTATE:
		case TB_GETTOOLTIPS:
		case TB_HIDEBUTTON:
		case TB_INDETERMINATE:
		case TB_INSERTBUTTON:	
		case TB_ISBUTTONCHECKED:	
		case TB_ISBUTTONENABLED:	
		case TB_ISBUTTONHIDDEN:	
		case TB_ISBUTTONINDETERMINATE:	
		case TB_ISBUTTONPRESSED:	
		case TB_PRESSBUTTON:	
		case TB_SAVERESTORE:	
		case TB_SETBITMAPSIZE:	
		case TB_SETBUTTONSIZE:	
		case TB_SETCMDID:	
		case TB_SETPARENT:	
		case TB_SETROWS:	
		case TB_SETSTATE:	
		case TB_SETTOOLTIPS:
			return Toolbr_TBWndProc(hWnd, uMessage, wParam, lParam);
        case DL_BEGINDRAG:
            return TRUE;

        default:
			return DefWindowProc(hWnd, uMessage, wParam, lParam);
	}
    
	    return 0;
}

static LRESULT  
Toolbr_TBWndProc
(
	HWND				hWnd,
    UINT				uMessage,
    WPARAM				wParam,
    LPARAM				lParam
)
{
	HGLOBAL				hToolbar	= ( HGLOBAL )NULL;
	TOOLBARDATA*			pthis		= ( TOOLBARDATA* )NULL;
	LRESULT				lResult		= 0L;
	
	int				ErrorCode	= 0;

	if ( ! ( hToolbar = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TOOLBARDATA *) GlobalLock ( hToolbar ) ) )
		ErrorCode = WTLB_ERR_GLOBALLOCK;
	else
	{
		switch ( uMessage )
		{
			case TB_ADDBITMAP:
				lResult = ( LRESULT )Toolbr_AddBitmap ( hWnd, pthis, ( UINT )wParam,
					( LPTBADDBITMAP ) lParam );
				break;
			case TB_ADDBUTTONS:
				lResult = ( LRESULT )Toolbr_AddButtons ( hWnd, pthis, 
					( UINT )wParam, ( LPTBBUTTON )lParam );
				break;
			case TB_ADDSTRING:
				lResult = ( LRESULT )Toolbr_AddStrings ( hWnd, pthis, 
							( HINSTANCE )wParam, ( LPSTR )lParam );
				break;
			case TB_AUTOSIZE:
				Toolbr_AutoSize ( hWnd, pthis );
				break;
			case TB_BUTTONCOUNT:
				lResult = ( LRESULT )pthis->ItmCount;
				break;
			case TB_BUTTONSTRUCTSIZE:
				break;
			case TB_CHANGEBITMAP:
				lResult = Toolbr_ChangeBitmap ( hWnd, pthis, ( int )wParam, 
						( int )LOWORD ( lParam ) );
				break;
			case TB_CHECKBUTTON:
				lResult = Toolbr_CheckBtn ( hWnd, pthis, ( int )wParam, 
						( BOOL )LOWORD ( lParam ) );
				break;
			case TB_COMMANDTOINDEX:
				lResult = ( LRESULT )Toolbr_CommandToIndex ( hWnd, pthis, ( int )wParam );
				break;
			case TB_CUSTOMIZE:
				break;
			case TB_DELETEBUTTON:
				lResult = ( LRESULT )Toolbr_DeleteBtn ( hWnd, pthis, ( int )wParam );
				break;
			case TB_ENABLEBUTTON:
				lResult = ( LRESULT )Toolbr_EnableBtn ( hWnd, pthis, ( int )wParam, 
							( BOOL )LOWORD ( lParam ) );
				break;
			case TB_GETBITMAP:
				lResult = ( LRESULT )Toolbr_GetBitmap ( hWnd, pthis, ( int )wParam );
				break;
			case TB_GETBITMAPFLAGS:
				lResult = ( LRESULT )Toolbr_GetBitmapFlag ( hWnd );
				break;
			case TB_GETBUTTON:
				lResult = ( LRESULT )Toolbr_GetButton ( hWnd, pthis, ( int )wParam, 
							( LPTBBUTTON )lParam );
				break;
			case TB_GETBUTTONTEXT:
				lResult = ( LRESULT )Toolbr_GetButtonText ( hWnd, pthis, ( int )wParam, 
							( LPSTR )lParam );
				break;
			case TB_GETITEMRECT:
				lResult = ( LRESULT )Toolbr_GetItemRect ( hWnd, pthis, ( int )wParam, 
							( LPRECT )lParam );
				break;
			case TB_GETROWS:
				lResult = ( LRESULT )pthis->Rows;
				break;
			case TB_GETSTATE:
				lResult = ( LRESULT )Toolbr_GetState ( hWnd, pthis, ( int )wParam );
				break;
			case TB_GETTOOLTIPS:
				lResult = ( LRESULT )pthis->hwTTip;
				break;
			case TB_HIDEBUTTON:
				lResult = ( LRESULT )Toolbr_HideBtn ( hWnd, pthis, ( int )wParam, 
							( BOOL )LOWORD ( lParam ) );
				break;
			case TB_INDETERMINATE:
				lResult = ( LRESULT )Toolbr_InDeterminate ( hWnd, pthis, ( int )wParam, 
							( BOOL )LOWORD ( lParam ) );
				break;
			case TB_INSERTBUTTON:	
				lResult = ( LRESULT )Toolbr_InsertBtn ( hWnd, pthis, ( int )wParam, 
							( LPTBBUTTON )lParam );
				break;
			case TB_ISBUTTONCHECKED:	
			case TB_ISBUTTONENABLED:	
			case TB_ISBUTTONHIDDEN:	
			case TB_ISBUTTONINDETERMINATE:	
			case TB_ISBUTTONPRESSED:	
				lResult = ( LRESULT )Toolbr_QueryStat ( hWnd, pthis, uMessage,
							( int )wParam );
				break;
			case TB_PRESSBUTTON:	
				lResult = ( LRESULT )Toolbr_PressBtn ( hWnd, pthis, ( int )wParam,
							( BOOL )LOWORD ( lParam ) );
				break;
			case TB_SAVERESTORE:	
				break;
			case TB_SETBITMAPSIZE:	
				lResult = ( LRESULT )Toolbr_SetBMPSize ( hWnd, pthis, 
						( int )LOWORD ( lParam ), ( int )HIWORD ( lParam ) );
				break;
			case TB_SETBUTTONSIZE:	
				lResult = ( LRESULT )Toolbr_SetBtnSize ( hWnd, pthis, 
						( int )LOWORD ( lParam ), ( int )HIWORD ( lParam ) );
				break;
			case TB_SETCMDID:	
				lResult = ( LRESULT )Toolbr_SetCmdID ( hWnd, pthis, 
						( UINT )wParam, ( UINT )lParam );
				break;
			case TB_SETPARENT:	
				SetParent ( hWnd, ( HWND )wParam );
				break;
			case TB_SETROWS:	
				Toolbr_SetRows ( hWnd, pthis, ( int )LOWORD ( wParam ), 
					( BOOL )HIWORD ( wParam ), ( LPRECT )lParam );
				break;
			case TB_SETSTATE:	
				lResult = ( LRESULT )Toolbr_SetState ( hWnd, pthis, ( int )wParam,
					( int )LOWORD ( lParam ) );
				break;
			case TB_SETTOOLTIPS:
				pthis->hwTTip = ( HWND )wParam;
				Toolbr_SetTooltip ( hWnd, pthis->hwTTip );
				break;
		}
	}

	if ( pthis )
		GlobalUnlock ( hToolbar );
	return lResult;
}


int WINAPI 
Toolbr_Initialize 
(	
	HINSTANCE		hinst
)
{

	WNDCLASS		wc;
	int			ErrorCode	= 0;

	memset ( &wc, 0, sizeof ( WNDCLASS ) );
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS | CS_DBLCLKS;
	wc.lpfnWndProc   = Toolbr_WndProc;
	wc.cbWndExtra    = 0; 
	wc.hInstance     = hinst;
	wc.hCursor       = LoadCursor ( ( HINSTANCE )NULL, IDC_ARROW );
	wc.hbrBackground = GetStockObject ( LTGRAY_BRUSH );
	wc.lpszClassName = WTOOLBARCLASSNAME;

	if ( ! RegisterClass ( &wc ) )
		ErrorCode = WTLB_ERR_REGISTERCLASS;

	return ( ErrorCode );
}


void WINAPI 
Toolbr_Terminate 
(
	HINSTANCE			hInstance
) 
{
	WNDCLASS			WndClass;


	if ( GetClassInfo ( hInstance, WTOOLBARCLASSNAME, &WndClass ) )
		UnregisterClass ( WTOOLBARCLASSNAME, hInstance );

}


