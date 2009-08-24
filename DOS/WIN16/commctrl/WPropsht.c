/*  
	WPropsht.c	
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

#include "WResourc.h"
#include "WTrackbr.h"
#include "WTooltip.h"
#include "WUpdown.h"
#include "WImgList.h"
#include "WTab.h"
#include "WPropsht.h"

#define ALIGN_16BIT(x)          ((int)((int)(x)+1) & ~1)
#define ALIGN_32BIT(x)          ((int)((int)(x)+3) & ~3)

#define TAB_MARGIN	10
#define BTN_MARGIN	5
#define TABICON_CX	16
#define TABICON_CY	16
#define MAX_PAGECAPTION	40
#ifdef _WINDOWS
#define	PAGEDIALOGDATA	DLGTEMPLATE
#else
#define PAGEDIALOGDATA  DLGTEMPLATE /*PAGEDLGDATA*/
#endif
typedef int ( *LPPROPSHCALLBACK )( HWND, UINT, LPARAM );
typedef UINT ( *LPPROPSHPAGECALLBACK )( HWND, UINT, LPPROPSHEETPAGE );

extern HINSTANCE	hLibInstance;
enum { returnNormal = 0, returnReboot = ID_PSREBOOTSYSTEM, returnRestartWindows = ID_PSRESTARTWINDOWS };
typedef enum { btnOK, btnBack = btnOK, btnNext, btnCancel, btnApply,
	       btnHelp, btnFinish = btnHelp } enBtn;
char *btnText[] = { "OK", "Cancel", "Apply Now", "Help" };
char *aliasTxt[] = { "Close", "Next","Back","Finish" };
 

typedef struct {
	DWORD	style;
	BYTE    bNumberOfItems;
	WORD	x,y;
	WORD    cx,cy;
	LPSTR    szMenuName;
	LPSTR    szClassName;
	LPSTR 	szCaption;
	WORD	wPointSize;
	LPSTR    szFaceName;
} PAGEDLGDATA;

/*
 * hMem : handle to a memory block containing this _PSP structure, to be used in
 *	  WDestroyPropertyPage () to free allocated memory.
 * hwPage : handle to a modeless dialogbox implemented as a page.
 */
typedef struct 
{
	PROPSHEETPAGE	psPage;

	int		nTab;
	BOOL		bInit; /*whether the page is initialized */
	HWND		hwPage;
	HGLOBAL		hMem; 
} _PSP, *PPSP;

typedef struct 
{
	PPSP		hPage; 
} HPSPAGE;


typedef struct 
{
	RECT		rcDisplay; /*the display area for the tab control*/
	BOOL		bOkIsClose; /* whether the OK button changed to Close*/
	BOOL		bImageList; /* whether the image list is existent */
	UINT		nChanges; /* ==0 => Apply btn disabled, 
				       incremented when a PSM_CHANGED is received,
				       decremented when a PSM_UNCHANGED received */
	int		nRet;	/* return status passed to EndDialog*/

	HWND		hwBtn[4];
	HWND		hwTab;
	HWND		hwPageSel; /* window handle to a selected page */

	int		PageHeight;
	int		PageWidth;
	int		PageCount;

	HIMAGELIST	hImgList;

	WLIST		PageList; /*array of Property Pages*/
	PROPSHEETHEADER psh;
} PROPSHEETDATA;

static char PROPSHEET_PROP[] = "Propsht_Data";	

#define GETTHISDATA(hwnd) \
   GetProp(hwnd, PROPSHEET_PROP )
#define SETTHISDATA(hwnd, pthisNew) \
   SetProp(hwnd, PROPSHEET_PROP, pthisNew)
#define REMOVETHISDATA(hwnd) \
   RemoveProp(hwnd, PROPSHEET_PROP )


static char szPropSheetID[] = "WPropSheetTemplate";

/*prototype*/
HPROPSHEETPAGE WINAPI
WCreatePropertySheetPage
(
	LPCPROPSHEETPAGE		lppsp
);


#if 0
static void 
PropSh_OnHScroll 
(	
	HWND				hwnd, 
	HWND				hwndCtl, 
	UINT				codeNotify, 
	int				Pos 
);

static void 
PropSh_OnVScroll 
(
	HWND				hwnd, 
	HWND				hwndCtl, 
	UINT				codeNotify, 
	int				Pos 
);
#endif
#if 0
static void 
PropSh_OnOK 
(
	HWND				hwnd, 
	int				id, 
	HWND				hwndCtl, 
	UINT				codeNotify
);
#endif

static void
PropSh_AddPage
(
	HWND				hwPS,
	PROPSHEETDATA*			pthis,
	PPSP				hPage,
	BOOL				bResize
);

static BOOL CALLBACK WPropSh_DlgProc 
(
	HWND				hwnd, 
	UINT				uMsg,
	WPARAM				wParam, 
	LPARAM				lParam
); 

static BOOL  
PropSh_SetCurSel
(
	HWND				hWnd,
	PROPSHEETDATA*			pthis,
        int				PageIndx,
        HPROPSHEETPAGE			hPage
);

/******************************************************************************
 *Property Sheet Helper functions
 ******************************************************************************/

/*
 * NOTE: returns the dialog base units of the first page, which is used to 
 * determine the size of following pages in a property sheet. Window API
 * GetDialogBaseUnit () wouldn't help here because it always assume the font
 * used in every dialog box to be System Fixed Font( my guess ).
 */

static DWORD
PropSh_IGetDlgUnits
(
	PROPSHEETDATA*			pthis
)
{

#ifdef LATER
	if ( WLLock ( &pthis->PageList, ( LPVOID )&p1stPage ) )
		ErrorCode = WPS_ERR_LISTLOCK;
	else
	{
		TEXTMETRIC tm;				
		HDC hDC = GetDC ( p1stPage->hPage->hwPage );
		GetTextMetrics ( hDC, &tm );
		ReleaseDC ( p1stPage->hPage->hwPage, hDC );
		Height = ( int )tm.tmAscent;
		Width = tm.tmAscent / 2;
	}

	if ( p1stPage )
		WLUnlock ( &pthis->PageList, ( LPVOID )&p1stPage );

	return ( MAKELONG ( Width, Height ) );
#endif
	return GetDialogBaseUnits();

}
/*
 * NOTE: pSrcPage may be pointing to a _PSP structure or a PROPERTYSHEET structure.
 * _PSP is not exposed in WIN32, so we are free to define _PSP but we have to make it
 * private.
 * Parameter: 
 *   bReSize : whether the size of this page would stretch the property sheet
 */
static BOOL
PropSh_IInitPage
(
	HWND				hwPropSh,
	PROPSHEETDATA*			pthis,
	PPSP				pSrcPage,
	BOOL				bReSize
)
{
	PAGEDIALOGDATA*			pTmpl	= ( PAGEDIALOGDATA* )NULL; 
	HGLOBAL				hglb	= ( HGLOBAL )NULL;
	HRSRC				hrsrc	= ( HRSRC )NULL;
	HICON				hIcon	= ( HICON )NULL;

	/* call property sheet page callback function */
	if (pSrcPage->psPage.dwFlags & PSP_USECALLBACK)
	{
		LPPROPSHPAGECALLBACK pfnCallback;
		if (!(pfnCallback = (LPPROPSHPAGECALLBACK)
			pSrcPage->psPage.pfnCallback))
		{
			return (FALSE);
		}
		if (!(pfnCallback)((HWND) 0, PSPCB_CREATE, &pSrcPage->psPage))
		{
			/* prevent property sheet page creation */
			return (FALSE);
			/* LATER: fix PropSh_AddPage to examine return value */
		}
	}

/*	memcpy ( pDstPage, pSrcPage, sizeof ( PROPSHEETPAGE ) );*/

	if ( pSrcPage->psPage.hInstance == ( HINSTANCE )NULL )
		pSrcPage->psPage.hInstance = pthis->psh.hInstance;

	pTmpl = ( PAGEDIALOGDATA* )pSrcPage->psPage.u.pResource;
	if ( ( pSrcPage->psPage.dwFlags == PSP_DEFAULT ) ||
	     !( pSrcPage->psPage.dwFlags & PSP_DLGINDIRECT )
	   )
	{
		hrsrc = FindResource ( pSrcPage->psPage.hInstance, pSrcPage->psPage.u.pszTemplate, RT_DIALOG );
		hglb = LoadResource ( pSrcPage->psPage.hInstance, hrsrc );
		pTmpl = ( PAGEDIALOGDATA* )LockResource ( hglb );
	}

	if ( bReSize )
	{
		if ( pTmpl->cx > pthis->PageWidth )
			pthis->PageWidth = pTmpl->cx;
		if ( pTmpl->cy > pthis->PageHeight )
			pthis->PageHeight = pTmpl->cy;
	}

	pTmpl->style &= ~( WS_CAPTION | DS_MODALFRAME | WS_BORDER | WS_POPUP |
			   WS_THICKFRAME );
	pTmpl->style |= WS_CHILD;

	pSrcPage->hwPage = CreateDialogIndirectParam ( pSrcPage->psPage.hInstance,
				pTmpl, hwPropSh, pSrcPage->psPage.pfnDlgProc, pSrcPage->psPage.lParam ); 
	
	SetWindowPos ( pSrcPage->hwPage, HWND_BOTTOM, 0, 0, 0, 0, 
		SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW );

	if ( ( pSrcPage->psPage.dwFlags & PSP_USEICONID ) &&
	     ( pSrcPage->psPage.u2.pszIcon ) )	
	{
#if 1
		hIcon = LoadImage ( pSrcPage->psPage.hInstance,
			pSrcPage->psPage.u2.pszIcon, IMAGE_ICON, 
				TABICON_CX, TABICON_CY, LR_DEFAULTCOLOR );
#else
		hIcon = WLoadImage ( pSrcPage->psPage.hInstance,
			pSrcPage->psPage.u2.pszIcon, IMAGE_ICON, 
				TABICON_CX, TABICON_CY, LR_DEFAULTCOLOR );
#endif
	}

	if ( ( pSrcPage->psPage.dwFlags & PSP_USEHICON ) &&
	     ( pSrcPage->psPage.u2.hIcon ) )	
		hIcon = pSrcPage->psPage.u2.hIcon;
	if ( hIcon )
	{
		pthis->bImageList = TRUE;
		if ( ! pthis->hImgList )
			pthis->hImgList = ImageList_Create ( TABICON_CX, TABICON_CY, ILC_COLOR | ILC_MASK, 10, 10 );
		ImageList_ReplaceIcon ( pthis->hImgList, -1, hIcon );
	}

	pSrcPage->bInit = TRUE;

	if ( hglb )
		FreeResource ( hglb );
	return TRUE;
}

static BOOL
PropSh_IInitSheet
(
	HWND				hwPropSh,
	PROPSHEETDATA*			pthis,
	LPPROPSHEETHEADER		ppsh
)
{
	int				i;
	RECT				rcSheet;
	RECT				rcOK;
	DWORD				dwStyle;
/*	_PSP				PageInfo;*/
/*	TC_ITEM				tci;*/
	PROPSHEETPAGE*			pPage		= ( PROPSHEETPAGE* )NULL;
	HPROPSHEETPAGE*			ppPage		= ( HPROPSHEETPAGE* )NULL;
	HPROPSHEETPAGE			hPage		= ( HPROPSHEETPAGE )NULL;
	int				Count		= ppsh->nPages;
	HWND				hwOK		= GetDlgItem ( hwPropSh, IDOK );
	int				ErrorCode	= 0;

	pthis->psh.hInstance = ( HINSTANCE )GetWindowLong ( hwPropSh, GWL_HINSTANCE );
	pthis->psh.dwFlags = ppsh->dwFlags;
	pthis->PageWidth = pthis->PageHeight 
			 = pthis->PageCount    = 0;
	pthis->hwPageSel = ( HWND )NULL;
	pthis->nChanges = 0;
	pthis->nRet = returnNormal;

	GetClientRect ( hwPropSh, &rcSheet );
	GetWindowRect ( hwOK, &rcOK );
	MapWindowPoints ( HWND_DESKTOP, hwPropSh, ( LPPOINT )&rcOK.left, 2 );
	InflateRect ( &rcSheet, -TAB_MARGIN, -TAB_MARGIN );
	dwStyle = WS_CHILD | WS_CLIPSIBLINGS | TCS_FOCUSONBUTTONDOWN | TCS_TOOLTIPS;
	if ( !( pthis->psh.dwFlags & PSH_WIZARD ) )
 		dwStyle |= WS_VISIBLE;
	pthis->hwTab = CreateWindow ( WWC_TABCONTROL,NULL, dwStyle, 
			rcSheet.left, rcSheet.top, 
			rcSheet.right - rcSheet.left, 
			rcOK.top - rcSheet.top - TAB_MARGIN, hwPropSh, ( HMENU )NULL,
			pthis->psh.hInstance, NULL );

	/*memset ( &tci, 0, sizeof ( TC_ITEM ) );*/
	if ( ppsh->dwFlags & PSH_PROPSHEETPAGE )
	{
		pPage = ( PROPSHEETPAGE* )ppsh->u3.ppsp;
		ppPage = &hPage;
		hPage = WCreatePropertySheetPage ( pPage );
	}
	else
		ppPage = ppsh->u3.phpage;

	for ( i = 0; i < Count; i++ )
	{
		PropSh_AddPage ( hwPropSh, pthis, ( PPSP )*ppPage, TRUE );
		if ( ppsh->dwFlags & PSH_PROPSHEETPAGE )
			hPage = WCreatePropertySheetPage ( ++pPage );
		else
			ppPage++;

	}
	
	if ( !( pthis->psh.dwFlags & PSH_WIZARD ) && pthis->bImageList )
		SendMessage ( pthis->hwTab, TCM_SETIMAGELIST, 0, ( LPARAM )pthis->hImgList );


	return ( ErrorCode >= 0 );
}

static BOOL
PropSh_ISetButtonsLayout
(
	HWND				hwPropSht,
	PROPSHEETDATA*			pthis,
	RECT*				prcTab
)
{
	int				i;
	RECT				rcButton;
	int				btnWidth = 0;

	for ( i = 0; i < 4; i++ )
		pthis->hwBtn[i] = ( HWND )NULL;
	if ( pthis->psh.dwFlags & PSH_WIZARD )
	{
		pthis->hwBtn[btnBack] = GetDlgItem ( hwPropSht, ID_BACK ); 
		pthis->hwBtn[btnNext] = GetDlgItem ( hwPropSht, ID_NEXT ); 
		pthis->hwBtn[btnCancel] = GetDlgItem ( hwPropSht, IDCANCEL ); 
		pthis->hwBtn[btnFinish] = GetDlgItem ( hwPropSht, ID_FINISH ); 
		DestroyWindow ( GetDlgItem ( hwPropSht, IDOK ) );
		DestroyWindow ( GetDlgItem ( hwPropSht, IDAPPLY ) );
		DestroyWindow ( GetDlgItem ( hwPropSht, ID_HELP ) );

	}
	else
	{
		pthis->hwBtn[btnOK] = GetDlgItem ( hwPropSht, IDOK ); 
		pthis->hwBtn[btnCancel] = GetDlgItem ( hwPropSht, IDCANCEL ); 
		if ( pthis->psh.dwFlags & PSH_NOAPPLYNOW )
			DestroyWindow ( GetDlgItem ( hwPropSht, IDAPPLY ) );
		else
			pthis->hwBtn[btnApply] = GetDlgItem ( hwPropSht, IDAPPLY ); 
		if ( pthis->psh.dwFlags & PSH_HASHELP )
			pthis->hwBtn[btnHelp] = GetDlgItem ( hwPropSht, ID_HELP ); 
		else
			DestroyWindow ( GetDlgItem ( hwPropSht, ID_HELP ) );
		DestroyWindow ( GetDlgItem ( hwPropSht, ID_BACK ) );
		DestroyWindow ( GetDlgItem ( hwPropSht, ID_NEXT ) );
		DestroyWindow ( GetDlgItem ( hwPropSht, ID_FINISH ) );
	}

	GetClientRect ( pthis->hwBtn[0], &rcButton );
	btnWidth = rcButton.right - rcButton.left;
	rcButton.top    = prcTab->bottom + TAB_MARGIN;
	rcButton.left   = prcTab->right - btnWidth;
	rcButton.bottom += rcButton.top;

	for ( i = 3; i >= 0; i-- )
	{
		if ( pthis->hwBtn[i] == ( HWND )NULL )
			continue;
		SetWindowPos ( pthis->hwBtn[i], ( HWND )NULL, rcButton.left, rcButton.top, 
				0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
		rcButton.left   -= ( btnWidth + BTN_MARGIN );

	}
	
	prcTab->bottom = rcButton.bottom + TAB_MARGIN;
	if ( pthis->psh.dwFlags & PSH_WIZARD )
	{
		GetWindowRect ( pthis->hwBtn[btnNext], &rcButton );
		MapWindowPoints ( ( HWND )NULL, hwPropSht, (LPPOINT)&rcButton.left, 2 );
		SetWindowPos ( pthis->hwBtn[btnFinish], ( HWND )NULL, rcButton.left, rcButton.top, 
				0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
		ShowWindow ( pthis->hwBtn[btnFinish], SW_HIDE );
		EnableWindow ( pthis->hwBtn[btnBack], FALSE );
	}
	return TRUE;
}

static BOOL
PropSh_ISetLayout
(
	HWND				hwPropSht,
	PROPSHEETDATA*			pthis
)
{
	RECT				rcTab;
	DWORD				dwDlgBase;

	SetRectEmpty ( &rcTab );

	dwDlgBase = PropSh_IGetDlgUnits ( pthis );

	/*change page sizes into pixel units*/
	pthis->PageWidth = MulDiv(pthis->PageWidth,LOWORD ( dwDlgBase ) , 4);
	pthis->PageHeight = MulDiv(pthis->PageHeight,HIWORD ( dwDlgBase ), 8);

	rcTab.left  = 0;
	rcTab.top   = 0;
	rcTab.right = pthis->PageWidth;
	rcTab.bottom = pthis->PageHeight;

	TabCtrl_AdjustRect ( pthis->hwTab, TRUE, &rcTab );
	OffsetRect ( &rcTab, TAB_MARGIN - rcTab.left, TAB_MARGIN - rcTab.top );
	CopyRect ( &pthis->rcDisplay, &rcTab );
	TabCtrl_AdjustRect ( pthis->hwTab, FALSE, &pthis->rcDisplay );

	/*if ( !( pthis->psh.dwFlags & PSH_WIZARD ) )*/
		SetWindowPos ( pthis->hwTab, ( HWND )NULL, rcTab.left, rcTab.top, 
			rcTab.right - rcTab.left, rcTab.bottom - rcTab.top,
			SWP_NOZORDER );

	PropSh_ISetButtonsLayout ( hwPropSht, pthis, &rcTab );

	SetWindowPos ( hwPropSht, ( HWND )NULL, 0, 0,
			rcTab.right + TAB_MARGIN + 2 * GetSystemMetrics ( SM_CXDLGFRAME ), 
		        rcTab.bottom + 2 * GetSystemMetrics ( SM_CXDLGFRAME ) + 
				GetSystemMetrics ( SM_CYMENU ), SWP_NOMOVE | SWP_NOZORDER );
	return TRUE;

}

/******************************************************************************
 *Property Sheet APIs
 ******************************************************************************/

HPROPSHEETPAGE WINAPI
WCreatePropertySheetPage
(
	LPCPROPSHEETPAGE		lppsp
)
{
	PPSP				pInternalPage;
	HGLOBAL				hPage	= ( HGLOBAL )NULL;

	hPage = GlobalAlloc ( GHND, sizeof ( _PSP ) );
	pInternalPage = ( PPSP )GlobalLock ( hPage );

	memcpy ( pInternalPage, lppsp, sizeof ( PROPSHEETPAGE ) );
	pInternalPage->hMem = hPage;

	return ( HPROPSHEETPAGE )pInternalPage;
}

BOOL WINAPI
WDestroyPropertySheetPage
(
	HPROPSHEETPAGE			hPSPage
)
{
	HGLOBAL				hPage;
	PPSP				pPage	= ( PPSP )hPSPage;

	/* call property sheet page callback function */
	if (pPage->psPage.dwFlags & PSP_USECALLBACK)
	{
		LPPROPSHPAGECALLBACK pfnCallback;
		if (!(pfnCallback = (LPPROPSHPAGECALLBACK)
			pPage->psPage.pfnCallback))
		{
			return (FALSE);
		}
		(pfnCallback)((HWND) 0, PSPCB_RELEASE, &pPage->psPage);
	}

	if ( pPage->bInit )
		DestroyWindow ( pPage->hwPage );
	hPage = pPage->hMem;
	GlobalUnlock ( hPage );
	return ( GlobalFree ( hPage ) == ( HGLOBAL )NULL );
	
}

int 
WPropertySheet 
(
	LPCPROPSHEETHEADER		lppsh	
)
{
	HWND				hwDlg;
	if ( lppsh->dwFlags & PSH_MODELESS )
	{
#ifdef _WINDOWS
        hwDlg = CreateDialogParam ( lppsh->hInstance, szPropSheetID, lppsh->hwndParent,
			                WPropSh_DlgProc, ( LPARAM )lppsh );
#else
        hwDlg = CreateDialogParam ( hLibInstance, szPropSheetID, lppsh->hwndParent,
			                WPropSh_DlgProc, ( LPARAM )lppsh );
#endif
		ShowWindow ( hwDlg, TRUE );
	}
	else
	{
#ifdef _WINDOWS
		hwDlg = ( HWND )DialogBoxParam ( lppsh->hInstance, szPropSheetID, lppsh->hwndParent,
			                WPropSh_DlgProc, ( LPARAM )lppsh );
#else
		hwDlg = ( HWND )DialogBoxParam ( hLibInstance, szPropSheetID, lppsh->hwndParent,
			                WPropSh_DlgProc, ( LPARAM )lppsh );
#endif
	}

	return ( int )hwDlg;
}



/*******************************************
 * Property Sheet Event Handlers
 */
static BOOL 
PropSh_OnInitDialog 
(	
	HWND			hwPropSht, 
	HWND			hwndFocus, 
	LPARAM			lParam
) 
{
	LPPROPSHCALLBACK	lpCallback	= ( LPPROPSHCALLBACK )NULL;
	HGLOBAL			hPropShData	= ( HGLOBAL )NULL;
	PROPSHEETDATA*		pthis		= ( PROPSHEETDATA* )NULL;
	int		    	ErrorCode	= 0;

	if ( ! ( hPropShData = GlobalAlloc ( GHND, sizeof ( PROPSHEETDATA ) ) ) )
		ErrorCode = WPS_ERR_GLOBALALLOC;
	else
	if ( ! ( pthis = ( PROPSHEETDATA* ) GlobalLock ( hPropShData ) ) )
		ErrorCode = WPS_ERR_GLOBALLOCK;
	else
	{
		WLCreate ( &pthis->PageList, sizeof ( HPSPAGE ) );
		if ( pthis->psh.dwFlags & PSH_USECALLBACK )
		{
			lpCallback = ( LPPROPSHCALLBACK )pthis->psh.pfnCallback;
			( *lpCallback )( hwPropSht, PSCB_INITIALIZED, 0L );
		}

		PropSh_IInitSheet ( hwPropSht, pthis, ( LPPROPSHEETHEADER )lParam );
		SETTHISDATA ( hwPropSht, hPropShData );

		PropSh_ISetLayout ( hwPropSht, pthis );

		if ( pthis->psh.dwFlags & PSH_WIZARD ) 
		{
			/*ShowWindow ( pthis->hwTab, SW_HIDE );*/
			SetWindowPos ( pthis->hwTab, HWND_BOTTOM,0,0,0,0,
					SWP_NOSIZE | SWP_NOMOVE | SWP_HIDEWINDOW );
		}
		if ( !( pthis->psh.dwFlags & PSH_WIZARD ) &&
		     pthis->hwBtn[btnApply] 
		   )
			EnableWindow ( pthis->hwBtn[btnApply], FALSE );
		if ( pthis->psh.dwFlags & PSH_USEPSTARTPAGE )
			PropSh_SetCurSel ( hwPropSht, pthis, 0, ( HPROPSHEETPAGE )pthis->psh.u2.pStartPage ); 
		else
			PropSh_SetCurSel ( hwPropSht, pthis, pthis->psh.u2.nStartPage, 0L );
	/*	SetWindowText ( hwPropSht, "This is a test" );*/

	}

	if ( pthis )
		GlobalUnlock ( hPropShData );
	return(TRUE);         /* Accept default focus window.*/
}

#ifdef _DEBUG
HWND hTabDlg = ( HWND )NULL;
extern BOOL CALLBACK 
PropPg_DlgProc 
(
	HWND			hwnd, 
	UINT			uMsg,
	WPARAM			wParam, 
	LPARAM			lParam
) ;

#endif

/**************************************************************************************
 * Concerning the tooltip, for now we assume that the page would be ultimately
 * responsible for processing the TTN_NEEDTEXT ( Microsoft doc is not clear about
 * this ). Therefore, the property sheet would relay this message to the page
 * window where the mouse is hovering over. Also note that this message is first sent
 * to the tab control inside the property sheet. It then relays the message to the
 * property sheet. And again this feature is just mere assumption. 
 */
static VOID PropSh_OnWMNotify 
( 
	HWND				hDlg, 
	UINT				uCtlID, 
	LPARAM				lParam 
) 
{ 

	int				nTabSel		= -1;
	PROPSHEETDATA*			pPropShtData	= ( PROPSHEETDATA* )NULL;
	HGLOBAL				hPropSheet	= ( HGLOBAL )NULL;
	HPSPAGE*			pPageList	= ( HPSPAGE* )NULL;
	PPSP				pPage		= ( PPSP )NULL;
		
	int				ErrorCode	= 0;


	if ( ! ( hPropSheet = GETTHISDATA ( hDlg ) ) )
		ErrorCode = WPS_ERR_GETWINPROP;
	else
	if ( ! ( pPropShtData = ( PROPSHEETDATA*) GlobalLock ( hPropSheet ) ) )
		ErrorCode = WPS_ERR_GLOBALLOCK;
	if ( WLLock ( &pPropShtData->PageList, ( LPVOID )&pPageList ) )
		ErrorCode = WPS_ERR_LISTLOCK;
	else
	{
		nTabSel = TabCtrl_GetCurSel ( pPropShtData->hwTab );
		pPage = ( pPageList + nTabSel )->hPage;
		if ( ( ( ( LPNMHDR ) lParam )->code ) == TCN_SELCHANGING ) 
			SetWindowPos ( pPage->hwPage, HWND_BOTTOM, 0, 0, 0, 0, 
					SWP_NOMOVE |SWP_HIDEWINDOW | SWP_NOSIZE );

		if ( ( ( ( LPNMHDR ) lParam )->code ) == TCN_SELCHANGE ) 
		{
			pPropShtData->hwPageSel = pPage->hwPage;
			SetWindowPos ( pPage->hwPage, HWND_TOP, 
				pPropShtData->rcDisplay.left,
				pPropShtData->rcDisplay.top, 
				0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
		}

		if ( ( ( ( LPNMHDR ) lParam )->code ) == TTN_NEEDTEXT ) 
		{ 
			if ( uCtlID >= 0 && uCtlID < ( UINT )pPropShtData->PageCount )
			{
				pPage = ( pPageList + uCtlID )->hPage;
				SendMessage ( pPage->hwPage, WM_NOTIFY, uCtlID, lParam ); 
				
			}
		} 
	}

	if ( pPageList )
		WLUnlock ( &pPropShtData->PageList, ( LPVOID )&pPageList );
	if ( pPropShtData )
		GlobalUnlock ( hPropSheet );
	return; 
} 
 

static void 
PropSh_OnDestroy 
(
	HWND				hwnd
) 
{
	int				i;
	PROPSHEETDATA*			pPropShtData	= ( PROPSHEETDATA* )NULL;
	HGLOBAL				hPropSheet	= ( HGLOBAL )NULL;
	HPSPAGE*			pPageList	= ( HPSPAGE* )NULL;
	PPSP				pPage		= ( PPSP )NULL;
		
	int				ErrorCode	= 0;


	if ( ! ( hPropSheet = GETTHISDATA ( hwnd ) ) )
		ErrorCode = WPS_ERR_GETWINPROP;
	else
	if ( ! ( pPropShtData = ( PROPSHEETDATA*) GlobalLock ( hPropSheet ) ) )
		ErrorCode = WPS_ERR_GLOBALLOCK;
	if ( WLLock ( &pPropShtData->PageList, ( LPVOID )&pPageList ) )
		ErrorCode = WPS_ERR_LISTLOCK;
	else
	{
		for ( i = 0; i < pPropShtData->PageCount; i++ )
		{
			pPage = ( pPageList + i )->hPage;
			WDestroyPropertySheetPage ( ( HPROPSHEETPAGE )pPage );
		}

	}

	if ( pPageList )
		WLUnlock ( &pPropShtData->PageList, ( LPVOID )&pPageList );
	if ( pPropShtData )
		GlobalUnlock ( hPropSheet );
	
	REMOVETHISDATA ( hwnd );
	GlobalFree ( hPropSheet );

	return; 
}

static void 
PropSh_OnCancel
(
	HWND			hwnd, 
	PROPSHEETDATA*		pPropShtData,
	int			id, 
	HWND			hwndCtl, 
	UINT			codeNotify
) 
{
	HPSPAGE*			pPageList	= ( HPSPAGE* )NULL;
	PPSP				pPage		= ( PPSP )NULL;
		
	int				ErrorCode	= 0;

	if ( WLLock ( &pPropShtData->PageList, ( LPVOID )&pPageList ) )
		ErrorCode = WPS_ERR_LISTLOCK;
	else
	{
		int i;
		PSHNOTIFY pshNotify;
		pshNotify.hdr.code = PSN_RESET;
		pshNotify.lParam = ( LPARAM )TRUE;
		for ( i = 0; i < pPropShtData->PageCount; i++ )
		{
			pPage = ( pPageList + i )->hPage;
			pshNotify.hdr.hwndFrom = hwnd;
			SendMessage ( pPage->hwPage,WM_NOTIFY, 
				( WPARAM )0, ( LPARAM )&pshNotify );
		}
		
	}

	if ( pPageList )
		WLUnlock ( &pPropShtData->PageList, ( LPVOID )&pPageList );
	if ( pPropShtData->psh.dwFlags & PSH_MODELESS )
		DestroyWindow ( hwnd );
	else
		EndDialog ( hwnd, FALSE );

}


static void 
PropSh_OnApply
(
	HWND			hwnd, 
	PROPSHEETDATA*		pPropShtData,
	int			id, 
	HWND			hwndCtl, 
	UINT			codeNotify,
	BOOL			bOKClicked
) 
{
	HPSPAGE*			pPageList	= ( HPSPAGE* )NULL;
	PPSP				pPage		= ( PPSP )NULL;
		
	int				ErrorCode	= 0;

	if ( WLLock ( &pPropShtData->PageList, ( LPVOID )&pPageList ) )
		ErrorCode = WPS_ERR_LISTLOCK;
	else
	{
		int i;
		PSHNOTIFY pshNotify;
		pshNotify.hdr.hwndFrom = hwnd;

		pshNotify.hdr.code = PSN_KILLACTIVE;
		SendMessage ( pPropShtData->hwPageSel, WM_NOTIFY, 
			( WPARAM )0, ( LPARAM )&pshNotify );
		if ( GetWindowLong ( pPropShtData->hwPageSel, DWL_MSGRESULT ) == FALSE )
		{
			pshNotify.hdr.code = PSN_APPLY;
			if ( pPropShtData->bOkIsClose )
				pshNotify.lParam = ( LPARAM )FALSE;
			else
				pshNotify.lParam = ( LPARAM )TRUE;
			for ( i = 0; i < pPropShtData->PageCount; i++ )
			{
				pPage = ( pPageList + i )->hPage;
				SendMessage ( pPage->hwPage,WM_NOTIFY, 
					( WPARAM )0, ( LPARAM )&pshNotify );
				if ( GetWindowLong ( pPage->hwPage, DWL_MSGRESULT ) 
					== PSNRET_INVALID_NOCHANGEPAGE )
				{
					PostMessage ( pPropShtData->hwTab, PSM_SETCURSEL,
							( WPARAM )i, 0L );
					ErrorCode = WPS_ERR_GENERAL;
					break;
				}
			}
		}
		else
			ErrorCode = WPS_ERR_GENERAL;
		
	}

	if ( pPageList )
		WLUnlock ( &pPropShtData->PageList, ( LPVOID )&pPageList );

	if ( bOKClicked && ( ErrorCode == 0 ) )
	{
		if ( pPropShtData->psh.dwFlags & PSH_MODELESS )
			DestroyWindow ( hwnd );
		else
			EndDialog ( hwnd, FALSE );
	}
}


static void 
PropSh_OnHelp
(
	HWND			hwnd, 
	PROPSHEETDATA*		pPropShtData,
	int			id, 
	HWND			hwndCtl, 
	UINT			codeNotify
) 
{

	NMHDR				Notify;
	Notify.hwndFrom = hwnd;

	Notify.code = PSN_HELP;
	SendMessage ( pPropShtData->hwPageSel, WM_NOTIFY, 
		( WPARAM )0, ( LPARAM )&Notify );

}


static void 
PropSh_OnGoBack
(
	HWND			hwnd, 
	PROPSHEETDATA*		pPropShtData,
	int			id, 
	HWND			hwndCtl, 
	UINT			codeNotify
) 
{
	int				nextPage	= -1;
	HPSPAGE*			pPageList	= ( HPSPAGE* )NULL;
	PPSP				pPage		= ( PPSP )NULL;
		
	int				ErrorCode	= 0;

	if ( WLLock ( &pPropShtData->PageList, ( LPVOID )&pPageList ) )
		ErrorCode = WPS_ERR_LISTLOCK;
	else
	{
		int i;
		PSHNOTIFY pshNotify;
		pshNotify.hdr.hwndFrom = hwnd;

		pshNotify.hdr.code = PSN_WIZBACK;
		SendMessage ( pPropShtData->hwPageSel, WM_NOTIFY, 
			( WPARAM )0, ( LPARAM )&pshNotify );
		nextPage = GetWindowLong ( pPropShtData->hwPageSel, DWL_MSGRESULT );
		if ( nextPage != -1 )
		{
			if ( nextPage <= 0 || nextPage >= pPropShtData->PageCount )
				for ( i = 0, nextPage = 0; i < pPropShtData->PageCount; i++, nextPage++ )
				{
					pPage = ( pPageList + i )->hPage; 
					if ( pPropShtData->hwPageSel == pPage->hwPage )
						break;
				}
				
			if ( nextPage > 0 )
			{
				SetWindowPos ( pPropShtData->hwPageSel, HWND_BOTTOM, 0, 0, 0, 0, 
					SWP_NOMOVE |SWP_HIDEWINDOW | SWP_NOSIZE );
				pPage = ( pPageList + nextPage - 1 )->hPage;
				pPropShtData->hwPageSel = pPage->hwPage;
				SetWindowPos ( pPage->hwPage, HWND_TOP, pPropShtData->rcDisplay.left,
				pPropShtData->rcDisplay.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
			}
#if 1
			if ( nextPage == 1 )
				EnableWindow ( pPropShtData->hwBtn[btnBack], FALSE );
#endif
			if ( IsWindowVisible( pPropShtData->hwBtn[btnFinish] ) )
			{
				ShowWindow( pPropShtData->hwBtn[btnFinish], SW_HIDE );
				ShowWindow( pPropShtData->hwBtn[btnNext], SW_SHOW );
			}
		}
		
	}

	if ( pPageList )
		WLUnlock ( &pPropShtData->PageList, ( LPVOID )&pPageList );
}


static void 
PropSh_OnGoFinish
(
	HWND			hwnd, 
	PROPSHEETDATA*		pPropShtData,
	int			id, 
	HWND			hwndCtl, 
	UINT			codeNotify
) 
{
	HPSPAGE*			pPageList	= ( HPSPAGE* )NULL;
		
	int				ErrorCode	= 0;

	if ( WLLock ( &pPropShtData->PageList, ( LPVOID )&pPageList ) )
		ErrorCode = WPS_ERR_LISTLOCK;
	else
	{
		PSHNOTIFY pshNotify;
		pshNotify.hdr.hwndFrom = hwnd;

		pshNotify.hdr.code = PSN_WIZFINISH;
		SendMessage ( pPropShtData->hwPageSel, WM_NOTIFY, 
			( WPARAM )0, ( LPARAM )&pshNotify );
	}

	if ( pPageList )
		WLUnlock ( &pPropShtData->PageList, ( LPVOID )&pPageList );

	if ( pPropShtData->psh.dwFlags & PSH_MODELESS )
		DestroyWindow ( hwnd );
	else
		EndDialog ( hwnd, FALSE );
}


static void 
PropSh_OnGoNext
(
	HWND			hwnd, 
	PROPSHEETDATA*		pPropShtData,
	int			id, 
	HWND			hwndCtl, 
	UINT			codeNotify
) 
{
	int				nextPage	= -1;
	HPSPAGE*			pPageList	= ( HPSPAGE* )NULL;
	PPSP				pPage		= ( PPSP )NULL;
		
	int				ErrorCode	= 0;

	if ( WLLock ( &pPropShtData->PageList, ( LPVOID )&pPageList ) )
		ErrorCode = WPS_ERR_LISTLOCK;
	else
	{
		int i;
		PSHNOTIFY pshNotify;
		pshNotify.hdr.hwndFrom = hwnd;

		pshNotify.hdr.code = PSN_WIZNEXT;
		SendMessage ( pPropShtData->hwPageSel, WM_NOTIFY, 
			( WPARAM )0, ( LPARAM )&pshNotify );
		nextPage = GetWindowLong ( pPropShtData->hwPageSel, DWL_MSGRESULT );
		if ( nextPage != -1 )
		{
			if ( nextPage <= 0 || nextPage >= pPropShtData->PageCount )
				for ( i = 0, nextPage = 0; i < pPropShtData->PageCount; i++, nextPage++ )
				{
					pPage = ( pPageList + i )->hPage;
					if ( pPropShtData->hwPageSel == pPage->hwPage )
						break;
				}
			
			if ( nextPage == 0 )
				EnableWindow ( pPropShtData->hwBtn[btnBack], TRUE );
			if ( nextPage + 1 < pPropShtData->PageCount )
			{
				SetWindowPos ( pPropShtData->hwPageSel, HWND_BOTTOM, 0, 0, 0, 0, 
					SWP_NOMOVE |SWP_HIDEWINDOW | SWP_NOSIZE );
				pPage = (pPageList + nextPage + 1 )->hPage;
				pPropShtData->hwPageSel = pPage->hwPage;
				SetWindowPos ( pPage->hwPage, HWND_TOP, 
					pPropShtData->rcDisplay.left,
					pPropShtData->rcDisplay.top, 
					0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
			}
#if 1
			else
			{
				ShowWindow ( pPropShtData->hwBtn[btnNext], SW_HIDE );
				ShowWindow ( pPropShtData->hwBtn[btnFinish], SW_SHOW );
			}
#endif
		}
		
	}

	if ( pPageList )
		WLUnlock ( &pPropShtData->PageList, ( LPVOID )&pPageList );
}


static void 
PropSh_OnCommand 
(
	HWND				hwnd, 
	int				id, 
	HWND				hwndCtl, 
	UINT				codeNotify
) 
{
	PROPSHEETDATA*			pthis		= ( PROPSHEETDATA* )NULL;
	HGLOBAL				hPropSheet	= ( HGLOBAL )NULL;
		
	int				ErrorCode	= 0;
                       
	if ( ! ( hPropSheet = GETTHISDATA ( hwnd ) ) )
		ErrorCode = WPS_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( PROPSHEETDATA*) GlobalLock ( hPropSheet ) ) )
		ErrorCode = WPS_ERR_GLOBALLOCK;
	else
	{
		switch ( id )
		{
			
			case IDAPPLY    : 
				PropSh_OnApply ( hwnd, pthis, id, hwndCtl, codeNotify, FALSE ); 
				break;
			case IDOK    : 
				PropSh_OnApply ( hwnd, pthis, id, hwndCtl, codeNotify, TRUE); 
				break;
			case IDCANCEL: 
				PropSh_OnCancel ( hwnd, pthis, id, hwndCtl, codeNotify); 
				break;
			case ID_HELP: 
				PropSh_OnHelp ( hwnd, pthis, id, hwndCtl, codeNotify); 
				break;
			case ID_NEXT : 
				PropSh_OnGoNext ( hwnd, pthis, id, hwndCtl, codeNotify ); 
				break;
			case ID_BACK : 
				PropSh_OnGoBack ( hwnd, pthis, id, hwndCtl, codeNotify ); 
				break;
			case ID_FINISH : 
				PropSh_OnGoFinish ( hwnd, pthis, id, hwndCtl, codeNotify ); 
				break;
		}
	}

	if ( pthis )
		GlobalUnlock ( hPropSheet );

}

#if 0
void PropSh_OnHScroll (HWND hwnd, HWND hwndCtl, UINT codeNotify, int Pos ) 
{
}

void PropSh_OnVScroll (HWND hwnd, HWND hwndCtl, UINT codeNotify, int Pos ) 
{
}
#endif

static void
PropSh_AddPage
(
	HWND				hwPS,
	PROPSHEETDATA*			pthis,
	PPSP				hPage,
	BOOL				bResize
)
{
	_PSP				PageInfo;
	int				ErrorCode = 0;
	char 				pBuf[MAX_PAGECAPTION];

	memset ( &PageInfo, 0, sizeof ( _PSP ) );
	PropSh_IInitPage ( hwPS, pthis, hPage, bResize );
	if ( WLInsert ( &pthis->PageList, pthis->PageCount, &hPage )  < 0 )
	{
		ErrorCode = WPS_ERR_LISTINSERT;
		return;
	}
#if 1
	if ( !( pthis->psh.dwFlags & PSH_WIZARD ) )
#endif
	{
		TC_ITEM	tci;
		HRSRC hResInfo;
		HGLOBAL hResource;
		PAGEDIALOGDATA *pResource;

		memset ( &tci, 0, sizeof ( TC_ITEM ) );
		tci.mask = TCIF_TEXT;

		pResource = (PAGEDIALOGDATA *)
			hPage->psPage.u.pResource;
		if (!(hPage->psPage.dwFlags & PSP_DLGINDIRECT))
		{
			hResInfo = FindResource(hPage->psPage.hInstance,
				hPage->psPage.u.pszTemplate,
				RT_DIALOG);
			hResource = LoadResource(hPage->psPage.hInstance,
				hResInfo);
			pResource = (PAGEDIALOGDATA *) LockResource(hResource);
		}

		if ((hPage->psPage.dwFlags & PSP_USETITLE)
		 && (hPage->psPage.pszTitle))
		{
			tci.pszText = ( LPSTR )hPage->psPage.pszTitle;
			if ( HIWORD ( tci.pszText ) == 0 )
			{
				LoadString ( hPage->psPage.hInstance, 
					LOWORD(tci.pszText), 
					pBuf, MAX_PAGECAPTION );
				tci.pszText = pBuf;
			}
		}
#if 1
		else
		{
			LPWORD lpcur = (LPWORD)((LPBYTE)pResource +
					sizeof(DLGTEMPLATE));

			/* First skip over menu specification */
			if (*lpcur == 0xFFFF)
				lpcur += 2;
			else if (*lpcur)
                		lpcur += ALIGN_16BIT(strlen((LPSTR)lpcur)+1)/2;
			else
				lpcur++;
			/* Next skip over class name specification */
			if (*lpcur == 0xFFFF)
				lpcur += 2;
			else if (*lpcur)
                		lpcur += ALIGN_16BIT(strlen((LPSTR)lpcur)+1)/2;
			else
				lpcur++;
			/* Now we are positioned at the title (caption) */
			tci.pszText = (LPSTR)lpcur;
		}
#endif
		tci.cchTextMax = lstrlen ( tci.pszText );
		tci.iImage = pthis->PageCount;
		SendMessage ( pthis->hwTab, TCM_INSERTITEM, 
			( WPARAM )pthis->PageCount, ( LPARAM )&tci );

		if (!(hPage->psPage.dwFlags & PSP_DLGINDIRECT))
		{
			FreeResource(hResource);
		}
	}

	pthis->PageCount++;
}

static void  
PropSh_PressBtn
(
	HWND				hWnd,
	PROPSHEETDATA*			pthis,
        int				iBtn
)
{
	enBtn				eBtn;
	int				nID;
	switch ( iBtn )
	{
		case PSBTN_APPLYNOW : eBtn = btnApply; nID = IDAPPLY;
				      break;
		case PSBTN_BACK	    : eBtn = btnBack; nID = ID_BACK;
				      break;
		case PSBTN_CANCEL   : eBtn = btnCancel; nID = IDCANCEL;
				      break;
		case PSBTN_FINISH   : eBtn = btnFinish; nID = ID_FINISH;
				      break;
		case PSBTN_HELP	    : eBtn = btnHelp; nID = ID_HELP;
				      break;
		case PSBTN_NEXT	    : eBtn = btnNext; nID = ID_NEXT;
				      break;
		case PSBTN_OK	    : eBtn = btnOK; nID = IDOK;
				      break;
	}

	SendMessage ( pthis->hwBtn[eBtn], WM_COMMAND, 
			MAKELONG ( nID, BN_CLICKED ), ( LPARAM )GetDlgItem ( hWnd, nID ) );
}

static int  
PropSh_QuerySiblings
(
	HWND				hWnd,
	PROPSHEETDATA*			pthis,
        WPARAM				param1,
	LPARAM				param2
)
{
	HPSPAGE*			pPageList	= ( HPSPAGE* )NULL;
	PPSP				pPage		= ( PPSP )NULL;
		
	int				ErrorCode	= 0;
	int				val		= 0;

	if ( WLLock ( &pthis->PageList, ( LPVOID )&pPageList ) )
		ErrorCode = WPS_ERR_LISTLOCK;
	else
	{
		int i;
		int val = 0;
		for ( i = 0; i < pthis->PageCount; i++ )
		{
			pPage = ( pPageList + i )->hPage;
			if ( ( val = SendMessage ( pPage->hwPage, PSM_QUERYSIBLINGS, 
				param1, param2 ) ) != 0 )
				break;
		}
		
	}

	if ( pPageList )
		WLUnlock ( &pthis->PageList, ( LPVOID )&pPageList );
	
	return val;
}


static void  
PropSh_RemovePage
(
	HWND				hWnd,
	PROPSHEETDATA*			pthis,
        int				PageIndx,
        HPROPSHEETPAGE			hPage
)
{
	int				i;
/*	PSHNOTIFY			pshNotify;*/
	HPSPAGE*			pPageList	= ( HPSPAGE* )NULL;
	PPSP				pPage		= ( PPSP )NULL;
	PPSP				pRemovePage	= ( PPSP )hPage;
	int				ErrorCode	= 0;
	
	if ( !pRemovePage || !( pRemovePage->hwPage ) )
	{
		if ( WLLock ( &pthis->PageList, ( LPVOID )&pPageList ) )
		{
			ErrorCode = WPS_ERR_LISTLOCK;
			return;
		}
		else
		if ( PageIndx >= 0 && PageIndx < pthis->PageCount )
		{
			pRemovePage = ( pPageList + PageIndx )->hPage;
		}
	}

	if ( !pRemovePage || !( pRemovePage->hwPage ) )
		return;

	for ( i = 0; i < pthis->PageCount; i++ )
	{
		pPage = ( pPageList + i )->hPage;
		if ( pRemovePage->hwPage == pPage->hwPage ) 
			break;
	}

	if ( i < pthis->PageCount ) 
	{
		WDestroyPropertySheetPage ( ( HPROPSHEETPAGE )pPage );
		if ( WLDelete ( &pthis->PageList, i ) == 0 )
			pthis->PageCount--;
		if ( !( pthis->psh.dwFlags & PSH_WIZARD ) )
			SendMessage ( pthis->hwTab, TCM_DELETEITEM, 
				( WPARAM )PageIndx, 0L );
	}

	if ( pPageList )
		WLUnlock ( &pthis->PageList, ( LPVOID )&pPageList );

}


/*
 * This function assume that when hPage is not NULL, its hwPage handle is valid and
 * is used to display the selected page despite despite the value of PageIndx and
 * its corresponding hPage stored internally by the property sheet.
 * Need more test on this
 */
static BOOL  
PropSh_SetCurSel
(
	HWND				hWnd,
	PROPSHEETDATA*			pthis,
        int				PageIndx,
        HPROPSHEETPAGE			hPage
)
{
	PSHNOTIFY			pshNotify;
	HPSPAGE*			pPageList	= ( HPSPAGE* )NULL;
	PPSP				pPage		= ( PPSP )hPage;
	int				ErrorCode	= 0;

	if ( pPage && pPage->hwPage )
	{
	}
	else
	if ( WLLock ( &pthis->PageList, ( LPVOID )&pPageList ) )
		ErrorCode = WPS_ERR_LISTLOCK;
	else
	if ( PageIndx >= 0 && PageIndx < pthis->PageCount )
	{
		pPage = ( pPageList + PageIndx )->hPage;
		pshNotify.hdr.hwndFrom = hWnd;
		if ( pthis->hwPageSel &&
		     pthis->hwPageSel != pPage->hwPage )
		{
			pshNotify.hdr.code = PSN_KILLACTIVE;
			SendMessage ( pthis->hwPageSel, WM_NOTIFY, 
				( WPARAM )0, ( LPARAM )&pshNotify );
			SetWindowPos ( pthis->hwPageSel, HWND_BOTTOM, 0, 0, 0, 0
				, SWP_NOMOVE |SWP_HIDEWINDOW | SWP_NOSIZE );

		}
		pthis->hwPageSel = pPage->hwPage;
		pshNotify.hdr.code = PSN_SETACTIVE;
		SendMessage ( pthis->hwPageSel, WM_NOTIFY, 
				( WPARAM )0, ( LPARAM )&pshNotify );
		SetWindowPos ( pthis->hwPageSel, HWND_TOP, 
				pthis->rcDisplay.left,
				pthis->rcDisplay.top, 
				0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
	}


	if ( pPageList )
		WLUnlock ( &pthis->PageList, ( LPVOID )&pPageList );
#if 1
	if ( !( pthis->psh.dwFlags & PSH_WIZARD ) )
#endif
#if 1
		PostMessage ( pthis->hwTab, TCM_SETCURSEL, ( WPARAM )PageIndx, 0L );
#else
		SendMessage ( pthis->hwTab, TCM_SETCURSEL, ( WPARAM )PageIndx, 0L );
#endif
	return ( ErrorCode >= 0 );
}

static void  
PropSh_SetFinishTxt
(
	HWND				hWnd,
	PROPSHEETDATA*			pthis,
        LPSTR				lpText
)
{
	RECT				rc;
	int				btnWidth;

	GetWindowRect ( pthis->hwBtn[btnCancel], &rc );
	MapWindowPoints ( ( HWND )NULL, hWnd, ( LPPOINT )&rc.left, 2 );
	btnWidth = rc.right - rc.left;
	SetWindowPos ( pthis->hwBtn[btnFinish],  ( HWND )NULL, 
		rc.left - btnWidth - TAB_MARGIN, rc.top, 
				0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
	ShowWindow ( pthis->hwBtn[btnNext], SW_HIDE );
	ShowWindow ( pthis->hwBtn[btnBack], SW_HIDE );

}

static void  
PropSh_SetTitle
(
	HWND				hWnd,
	PROPSHEETDATA*			pthis,
	DWORD				dwStyle,
        LPSTR				lpText
)
{
	char				buf[80];
	static char*			szPropPrefix = "Properties for ";
	char*				pBuf	     = buf;
	int				cb	     = 0;

	if ( dwStyle &	PSH_PROPTITLE )
	{
		sprintf ( buf, "%s", szPropPrefix );
		pBuf = buf + sizeof ( szPropPrefix );
	}

	if ( HIWORD ( lpText ) == 0 )
	{
		cb = LoadString ( pthis->psh.hInstance, LOWORD ( lpText ), 
			pBuf, 80 - sizeof ( szPropPrefix ) );
		if ( cb == 0 )
			return;
	}
	else
		strcat ( buf, lpText );

	SetWindowText ( hWnd, buf );

}

static void  
PropSh_SetWizBtn
(
	HWND				hWnd,
	PROPSHEETDATA*			pthis,
	DWORD				dwFlag
)
{
	
	EnableWindow ( pthis->hwBtn[btnBack], dwFlag & PSWIZB_BACK );
	EnableWindow ( pthis->hwBtn[btnNext], dwFlag & PSWIZB_NEXT );
	EnableWindow ( pthis->hwBtn[btnFinish], dwFlag & PSWIZB_FINISH );
}


/*********************************************************************
 * These functions process messages sent to PropertySheet dialog box.
 */
static LRESULT  
PropSh_PSMWndProc
(
	HWND				hWnd,
        UINT				uMessage,
        WPARAM				wParam,
        LPARAM				lParam
)
{
	HGLOBAL				hPropSht	= ( HGLOBAL )NULL;
	PROPSHEETDATA*			pthis		= ( PROPSHEETDATA* )NULL;
	LRESULT				lResult		= 0L;
	
	int				ErrorCode	= 0;

	if ( ! ( hPropSht = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WPS_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( PROPSHEETDATA* ) GlobalLock ( hPropSht ) ) )
		ErrorCode = WPS_ERR_GLOBALLOCK;
	else
	{
		switch ( uMessage )
		{
			case PSM_ADDPAGE:
				PropSh_AddPage ( hWnd, pthis, ( PPSP )lParam, FALSE );
				break;
			case PSM_APPLY:
				PropSh_OnApply ( hWnd, pthis, IDAPPLY, GetDlgItem ( hWnd, IDAPPLY )
					, BN_CLICKED, FALSE ); 
				break;
			case PSM_CANCELTOCLOSE:
				EnableWindow ( pthis->hwBtn[btnCancel], FALSE );
				SetWindowText ( pthis->hwBtn[btnOK], aliasTxt[btnOK] );
				pthis->bOkIsClose = TRUE;
				break;
			case PSM_CHANGED:
				pthis->nChanges++;
				if ( pthis->hwBtn[btnApply] )
					EnableWindow ( pthis->hwBtn[btnApply], TRUE );
				break;
			case PSM_GETCURRENTPAGEHWND:
				lResult = ( LRESULT )pthis->hwPageSel;
				break;
			case PSM_GETTABCONTROL:
				lResult = ( LRESULT )pthis->hwTab;
				break;
			case PSM_PRESSBUTTON:
				PropSh_PressBtn ( hWnd, pthis, ( int )wParam );
				break;
			case PSM_QUERYSIBLINGS:
				lResult = ( LRESULT )PropSh_QuerySiblings ( hWnd, pthis, wParam, lParam );
				break;
			case PSM_REBOOTSYSTEM:
				pthis->nRet = returnReboot;
				break;
			case PSM_REMOVEPAGE:
				PropSh_RemovePage ( hWnd, pthis, 
					     ( int )wParam, ( HPROPSHEETPAGE )lParam );
				break;
			case PSM_RESTARTWINDOWS:
				pthis->nRet = returnRestartWindows;
				break;
			case PSM_SETCURSEL:
				lResult = ( LRESULT )PropSh_SetCurSel ( hWnd, pthis, 
					     ( int )wParam, ( HPROPSHEETPAGE )lParam );
				break;
			case PSM_SETCURSELID:
				break;
			case PSM_SETFINISHTEXT:
				PropSh_SetFinishTxt ( hWnd, pthis, ( LPSTR )lParam );
				break;
			case PSM_SETTITLE:
				PropSh_SetTitle ( hWnd, pthis, 
					( DWORD )wParam, ( LPSTR )lParam );
				break;
			case PSM_SETWIZBUTTONS:
				PropSh_SetWizBtn ( hWnd, pthis, ( DWORD )lParam );
				break;
			case PSM_UNCHANGED:
				if ( --pthis->nChanges  == 0 &&
				     pthis->hwBtn[btnApply] 
				   )
					EnableWindow ( pthis->hwBtn[btnApply], FALSE );
				break;
		}
	}

	if ( pthis )
		GlobalUnlock ( hPropSht );
	return lResult;
}


static BOOL CALLBACK WPropSh_DlgProc (HWND hwnd, UINT uMsg,
   WPARAM wParam, LPARAM lParam) 
{
   switch (uMsg)     
   {
	   case WM_INITDIALOG :
		   return PropSh_OnInitDialog ( ( hwnd ), ( HWND )( wParam ), lParam );
	   case WM_DESTROY :  
		   PropSh_OnDestroy ( hwnd );
		   break;
	   case WM_COMMAND :
		   PropSh_OnCommand ( hwnd, (int)( LOWORD( wParam ) ), ( HWND )( lParam ), ( UINT )HIWORD ( wParam ) );
		   break;
	   case WM_NOTIFY  :
		   PropSh_OnWMNotify ( hwnd, wParam, lParam );
		   break;
	   case WM_SETICON:
		   break;
	   case PSM_ISDIALOGMESSAGE:
		   {
			MSG* pmsg = ( MSG* )lParam;
			if ( hwnd == pmsg->hwnd )
			{
			      WPropSh_DlgProc ( hwnd, pmsg->message, 
						pmsg->wParam, pmsg->lParam );
			      return TRUE;
			}
			return FALSE;
		   }
	   case PSM_ADDPAGE:
	   case PSM_APPLY:
	   case PSM_CANCELTOCLOSE:
	   case PSM_CHANGED:
	   case PSM_GETCURRENTPAGEHWND:
	   case PSM_GETTABCONTROL:
	   case PSM_PRESSBUTTON:
	   case PSM_QUERYSIBLINGS:
	   case PSM_REBOOTSYSTEM:
	   case PSM_REMOVEPAGE:
	   case PSM_RESTARTWINDOWS:
	   case PSM_SETCURSEL:
	   case PSM_SETCURSELID:
	   case PSM_SETFINISHTEXT:
	   case PSM_SETTITLE:
	   case PSM_SETWIZBUTTONS:
	   case PSM_UNCHANGED:
			return PropSh_PSMWndProc ( hwnd, uMsg, wParam, lParam );
   }

   return ( FALSE );         /* We didn't process the message.*/
}

/* End of File */
