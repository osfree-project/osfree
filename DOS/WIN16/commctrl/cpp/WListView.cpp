/*******************************************************************************

	@(#)WListView.cpp	1.7 	ListView Control
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


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

*******************************************************************************/





/*--- Includes  --------------------------------------------------------------*/
#include <limits.h>
#include "WCommCtrl.h"
#include "CommCtrlRC.h"
#include "WListView.h"
#include "WALVColumns.h"
#include "WALVItems.h"
#include "WString.h"
#include "WImgList.h"




/*--- Defines ----------------------------------------------------------------*/

#define WLVD_VERTICALSTRINGSPACEING     1
#define WLVD_HORIZONTALSTRINGSPACING    2
#define WVLD_HORIZONTALSPACING          12
#define WLVD_MINHORIZONTALWIDTH         100
#define WLVD_HEADERWINDOWHEIGHT         20
#define WLVD_BEGINTRACKING              -65535


/*--- Constants---------------------------------------------------------------*/

const int                               WLV_STANDARDERROR = -1;



/*--- Structures -------------------------------------------------------------*/

typedef struct WLVS_LOCAL
{
    LPSTR                               pListViewClass;
} WLVS_LOCAL;



/*----- Classes -----*/

class CWLListView 
{

    public:

        CWLListView ();
        ~CWLListView ();


};

/*----- Static Locals --------------------------------------------------------*/

static WLVS_LOCAL                       WLVSLocal;
static CWLListView                      *pWListView = NULL;


/*--- WLVCDefWindowProc - Handle Window Messages -----------------------------*/
extern "C" LRESULT EXPORT CALLBACK
WLVCDefWindowProc 
(
	HWND                                hWnd,
	UINT                                Message,
	WPARAM                              wParam,
	LPARAM                              lParam
)

{

    CWListView                          *pListView;
    LV_ITEM                             *pLVItem;
	PAINTSTRUCT                         PaintStruct;

    LRESULT                             ErrorCode      = 0;
    int                                 ExceptionError;
	int *			SaveCatchBuffer;


    if ( ( ! ( pListView = ( CWListView*)GetWindowLong ( hWnd, 0 ) ) ) &&
        ( Message != WM_CREATE ) )
        ErrorCode = DefWindowProc ( hWnd, Message, wParam, lParam );
    else
        switch ( Message )
	    {
		    default:
			    ErrorCode = pListView->WDefWindowProc ( hWnd, Message, wParam, lParam );
			    break;

		    case WM_CREATE:
#ifdef WEXCEPTIONS
                try
                {
                    pListView = new CWListView ( hWnd, ( LPCREATESTRUCT )lParam );
                    SetWindowLong ( hWnd, 0, ( long )pListView );
                }
                catch ( ExceptionError )
                {
                    ErrorCode = ExceptionError;
                }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
	{
		pListView = new CWListView( hWnd, ( LPCREATESTRUCT )lParam );
		SetWindowLong( hWnd, 0, ( long )pListView );
	}
	else
	{
		ErrorCode = ExceptionError;
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif
			    break;

            case WM_DESTROY:
                delete ( pListView );
                SetWindowLong ( hWnd, 0, 0 );
                break;

		    case WM_PAINT:
			    if ( wParam = ( WPARAM )BeginPaint ( hWnd, &PaintStruct ) )
			    {
                    pListView->OnWM_PAINT( &( PaintStruct.rcPaint ) );
				    EndPaint ( hWnd, &PaintStruct );
			    }
			    break;

            case WM_ERASEBKGND:
                ErrorCode = pListView->WDefWindowProc ( hWnd, Message, wParam, lParam );
                pListView->OnWM_ERASEBKGND ( ( HDC )wParam );
                break;

            case WM_SIZE:
                ErrorCode = pListView->WDefWindowProc ( hWnd, Message, wParam, lParam );
                pListView->OnWM_SIZE ();
                break;

            case WM_GETFONT:
                ErrorCode = ( LRESULT )pListView->OnWM_GETFONT ();
                break;

            case WM_SETFONT:
                pListView->OnWM_SETFONT ( wParam, lParam );
                break;

            case WM_WININICHANGE:
                {
                    LPSTR           pString = ( LPSTR )lParam;
                    pString = NULL;
                }
                break;

            case WM_NOTIFY:
                ErrorCode = ( LRESULT )pListView->OnWM_NOTIFY ( wParam, ( LPNMHDR )lParam );
                break;

            case WM_HSCROLL:
                pListView->OnWM_HSCROLL ( ( int )LOWORD ( wParam ), ( short int )HIWORD ( wParam ) );
                break;

            case WM_VSCROLL:
                pListView->OnWM_VSCROLL ( ( int )LOWORD ( wParam ), ( short int )HIWORD ( wParam ) );
                break;

            case LVM_ARRANGE:
                pListView->Arrange ( ( UINT )wParam );
                break;

            case LVM_CREATEDRAGIMAGE:
                break;

            case LVM_DELETEALLITEMS:
                if ( ErrorCode = ( LRESULT )pListView->DeleteAllItems() )
                    InvalidateRect ( hWnd, NULL, TRUE );
                break;

            case LVM_DELETECOLUMN:
                if ( ErrorCode = ( LRESULT )pListView->DeleteColumn ( ( DWORD )wParam ) )
                    InvalidateRect ( hWnd, NULL, TRUE );
                break;

            case LVM_DELETEITEM:
                if ( ErrorCode = ( LRESULT )pListView->DeleteItem ( ( DWORD )wParam ) )
                    InvalidateRect ( hWnd, NULL, TRUE );
                break;

            case LVM_EDITLABEL:
                break;

            case LVM_ENSUREVISIBLE:
                break;

            case LVM_FINDITEM:
                ErrorCode = ( LRESULT )pListView->FindItem ( ( DWORD )wParam, ( LV_FINDINFO* )lParam );
                break;

            case LVM_GETBKCOLOR:
                ErrorCode = ( LRESULT )pListView->LVGetBKColor();
                break;

            case LVM_GETCALLBACKMASK:
                break;

            case LVM_GETCOLUMN:
                ErrorCode = ( LRESULT )pListView->GetColumn ( ( DWORD )wParam, ( LV_COLUMN* )lParam );
                break;

            case LVM_GETCOLUMNWIDTH:
                ErrorCode = ( LRESULT )pListView->GetColumnWidth ( ( DWORD )wParam );
                break;

            case LVM_GETCOUNTPERPAGE:
                break;

            case LVM_GETEDITCONTROL:
                ErrorCode = ( LRESULT )pListView->GetEditControl ();
                break;

            case LVM_GETIMAGELIST:
                ErrorCode = ( LRESULT )pListView->GetImageList ( wParam ? ELV_IMAGELIST_SMALL : ELV_IMAGELIST_LARGE );
                break;

            case LVM_GETITEM:
                pLVItem   = ( LV_ITEM* )lParam;
                ErrorCode = ( LRESULT )pListView->GetItem ( ( DWORD )pLVItem->iSubItem, ( DWORD )pLVItem->iItem, pLVItem );
                break;

            case LVM_GETITEMCOUNT:
                ErrorCode = ( LRESULT )pListView->GetItemCount();
                break;

            case LVM_GETITEMPOSITION:
                break;

            case LVM_GETITEMRECT:
                break;

            case LVM_GETITEMSTATE:
                ErrorCode = ( LRESULT )pListView->GetItemState ( ( DWORD )wParam, ( UINT )lParam );
                break;

            case LVM_GETITEMTEXT:
                ErrorCode = ( LRESULT )pListView->GetItemText ( ( DWORD )wParam, ( LV_ITEM* )lParam );
                break;

            case LVM_GETNEXTITEM:
                break;

            case LVM_GETORIGIN:
                ErrorCode = ( LRESULT )pListView->GetOrigin ( ( LPPOINT )lParam );
                break;

            case LVM_GETSTRINGWIDTH:
                ErrorCode = ( LRESULT )pListView->GetStringWidth ( ( LPSTR )lParam );
                break;

            case LVM_GETTEXTBKCOLOR:
                ErrorCode = ( LRESULT )pListView->LVGetTextBKColor();
                break;

            case LVM_GETTEXTCOLOR:
                ErrorCode = ( LRESULT )pListView->LVGetTextColor();
                break;

            case LVM_GETTOPINDEX:
                break;

            case LVM_GETVIEWRECT:
                ErrorCode = ( LRESULT )pListView->GetViewRect ( ( LPRECT )lParam );
                break;

            case LVM_HITTEST:
                break;

            case LVM_INSERTCOLUMN:
                if ( ( ErrorCode = ( LRESULT )pListView->InsertColumn ( ( DWORD )wParam, ( LV_COLUMN* )lParam ) ) != WLV_STANDARDERROR )
                    if ( ( GetWindowLong ( hWnd, GWL_STYLE ) & LVS_TYPEMASK ) == LVS_REPORT )
                        InvalidateRect ( hWnd, NULL, TRUE );
                break;

            case LVM_INSERTITEM:
                pLVItem   = ( LV_ITEM* )lParam;
                if ( ( ErrorCode = ( LRESULT )pListView->InsertItem ( ( DWORD )pLVItem->iItem, pLVItem ) ) != WLV_STANDARDERROR )
                {
                    pListView->RedrawItems ( ErrorCode, pListView->GetItemCount() - 1 );
                    UpdateWindow ( hWnd );
                }
                break;

            case LVM_REDRAWITEMS:
                ErrorCode = ( LRESULT )pListView->RedrawItems ( ( DWORD )HIWORD ( lParam ), ( DWORD )LOWORD ( lParam ) );
                break;

            case LVM_SCROLL:
                ErrorCode = ( LRESULT )pListView->Scroll ( ( int )HIWORD ( lParam ), ( int )LOWORD ( lParam ) );
                break;

            case LVM_SETBKCOLOR:
                if ( ErrorCode = ( LRESULT )pListView->LVSetBKColor ( ( COLORREF )lParam ) )
                    InvalidateRect ( hWnd, NULL, TRUE );
                break;

            case LVM_SETCALLBACKMASK:
                break;

            case LVM_SETCOLUMN:
                if ( ErrorCode = ( LRESULT )pListView->SetColumn ( ( DWORD )wParam, ( LV_COLUMN* )lParam ) )
                    InvalidateRect ( hWnd, NULL, TRUE );
                break;

            case LVM_SETCOLUMNWIDTH:
                if ( ErrorCode = ( LRESULT )pListView->SetColumnWidth ( ( DWORD )wParam, ( DWORD )lParam ) )
                    InvalidateRect ( hWnd, NULL, TRUE );
                break;

            case LVM_SETIMAGELIST:
                if ( ErrorCode = ( LRESULT )pListView->SetImageList ( wParam ? ELV_IMAGELIST_SMALL : ELV_IMAGELIST_LARGE, ( HIMAGELIST )lParam ) )
                    InvalidateRect ( hWnd, NULL, TRUE );
                break;

            case LVM_SETITEM:
                pLVItem   = ( LV_ITEM* )lParam;
                if ( ErrorCode = ( LRESULT )pListView->SetItem ( ( DWORD )pLVItem->iSubItem, ( DWORD )pLVItem->iItem, pLVItem ) )
                {
                    switch ( GetWindowLong ( hWnd, GWL_STYLE ) & LVS_TYPEMASK )
                    {
                        case LVS_ICON:
                        case LVS_SMALLICON:
                        case LVS_LIST:
                            if ( pLVItem->iSubItem )
                                break;
                        case LVS_REPORT:
                            InvalidateRect ( hWnd, NULL, TRUE );
                            break;
                    }
                }
                break;

            case LVM_SETITEMCOUNT:
                ErrorCode = ( LRESULT )pListView->SetItemCount ( ( DWORD )wParam );
                break;

            case LVM_SETITEMPOSITION:
                break;

            case LVM_SETITEMSTATE:
                ErrorCode = ( LRESULT )pListView->SetItemState ( ( DWORD )wParam, ( LV_ITEM* )lParam );
                break;

            case LVM_SETITEMTEXT:
                ErrorCode = ( LRESULT )pListView->SetItemText ( ( DWORD )wParam, ( LV_ITEM* )lParam );
                break;

            case LVM_SETTEXTBKCOLOR:
                if ( ErrorCode = ( LRESULT )pListView->LVSetTextBKColor ( ( COLORREF )lParam ) )
                    InvalidateRect ( hWnd, NULL, TRUE );
                break;

            case LVM_SETTEXTCOLOR:
                if ( ErrorCode = ( LRESULT )pListView->LVSetTextColor ( ( COLORREF )lParam ) )
                    InvalidateRect ( hWnd, NULL, TRUE );
                break;

            case LVM_SORTITEMS:
                if ( ErrorCode = ( LRESULT )pListView->Sort ( ( PFNLVCOMPARE )lParam, ( LPARAM )wParam ) )
                    InvalidateRect ( hWnd, NULL, TRUE );
                break;

            case LVM_UPDATE:
                ErrorCode = ( LRESULT )pListView->Update ( ( DWORD )wParam );
                break;

	    }

	return ( ErrorCode );

}

/*===== CWLListView ============================================================*/

CWLListView::CWLListView ()

{

    WNDCLASS                            WndClass;


    memset ( &WLVSLocal, 0, sizeof ( WLVS_LOCAL ) );
    WLVSLocal.pListViewClass = WLVD_CLASSNAME;
    /*------------------------------------------------------
		Create a window class which any application can
		use with a grey background as the default.
	------------------------------------------------------*/
	memset ( &WndClass, 0, sizeof ( WndClass ) );
	WndClass.style         = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc   = WLVCDefWindowProc;
	WndClass.cbClsExtra    = 0;
	WndClass.cbWndExtra    = sizeof ( CWListView* );
	WndClass.hInstance     = pWCommCtrl->hInstance();
	WndClass.hIcon         = ( HICON )NULL;
	WndClass.hbrBackground = GetStockObject ( LTGRAY_BRUSH );
	WndClass.lpszMenuName  = ( LPCTSTR )NULL;
	WndClass.lpszClassName = WLVSLocal.pListViewClass;
	if ( ! RegisterClass ( &WndClass ) )
    {
        Throw ( CatchBuffer, WERR_REGISTERCLASS );
    }

}



CWLListView::~CWLListView ()

{

    WNDCLASS                            WndClass;


    if ( GetClassInfo ( pWCommCtrl->hInstance(), WLVSLocal.pListViewClass, &WndClass ) )
        UnregisterClass ( WLVSLocal.pListViewClass, pWCommCtrl->hInstance() );

}


/*--- Create a ListView Window -------------------------------------------------*/
CWListView::CWListView (
    HWND                                hWindow,
    LPCREATESTRUCT                      pCreateStruct )

{

    LV_COLUMN                           LVColumn;
    RECT                                Rect;


    hWnd = hWindow;
    SetWindowLong ( hWnd, 0, ( DWORD )this );

    OnWM_CREATE ( hWnd, pCreateStruct );

    hLargeImageList   = ( HIMAGELIST )NULL;
    hSmallImageList   = ( HIMAGELIST )NULL;
    hFont             = ( HFONT )NULL;
    CurrMaxIconWidth  = 0;
    CurrMaxIconHeight = 0;
    SetFont ();

    if ( ! ( hEditWnd = CreateWindow ( "Edit", NULL, WS_CHILD | WS_BORDER, 
                                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
                                        CW_USEDEFAULT, hWnd, ( HMENU )NULL, 
                                        ( HINSTANCE )GetWindowLong ( hWnd, GWL_HINSTANCE ), 
                                        NULL ) ) )
        Throw ( CatchBuffer, WERR_CREATEWINDOW );

    GetClientRect ( hWnd, &Rect );
    if ( ! ( hHeaderWnd = CreateWindow ( WC_HEADER, NULL, 
                                            WS_CHILD | HDS_BUTTONS | HDS_HORZ | HDS_HIDDEN,
                                            0, 0, Rect.right - Rect.left, GetRowHeight(), 
                                            hWnd, ( HMENU )NULL, 
                                            ( HINSTANCE )GetWindowLong ( hWnd, GWL_HINSTANCE ), 
                                            NULL ) ) )
        Throw ( CatchBuffer, WERR_CREATEWINDOW );
	
    /*--- Save the window style ---*/
    Style = pCreateStruct->style;
	
	/*--- Save the control id which is passed in as the menu handle ---*/
    ID = ( int )pCreateStruct->hMenu;

    /*--- Initialize other members ---*/
    hBackgroundBrush = ( HBRUSH )NULL;
    SendMessage ( hWnd, LVM_SETBKCOLOR, 0, ( LPARAM )GetSysColor ( COLOR_BACKGROUND ) );
    TextBKColor            = GetSysColor ( COLOR_WINDOW );
    TextColor              = GetSysColor ( COLOR_WINDOWTEXT );
    pColumns               = NULL;
    CurrMaxWidth           = WLVD_MINHORIZONTALWIDTH;
    CurrHeight             = GetStringHeight();
    NextLargeIconPoint.x   =
    NextLargeIconPoint.y   = 0;
    /*--- Create Column List ---*/
    if ( ! ( pColumns = new CWALVColumns() ) )
        Throw ( CatchBuffer, WERR_ALLOCATION );
    else
    {
        memset ( &LVColumn, 0, sizeof ( LV_COLUMN ) );
        CreatingDummyColumn    = TRUE;
        CreatingFirstColumn    = FALSE;
        UserFirstColumnCreated = FALSE;
        InsertColumn ( 0, &LVColumn );
        CreatingDummyColumn = FALSE;
    }
    
}



/*--- Destroy a Header Window ------------------------------------------------*/
CWListView::~CWListView ()

{

    DWORD                               ColumnIndex;
    DWORD                               ColumnCount;
    CWLVColumn                          *pColumn;


    if ( hBackgroundBrush )
        DeleteObject ( hBackgroundBrush );
    if ( hEditWnd )
        DestroyWindow ( hEditWnd );
    if ( hHeaderWnd )
        DestroyWindow ( hHeaderWnd );
    if ( hFont )
        DeleteObject ( hFont );
    if ( pColumns )
    {
        for ( ColumnIndex = 0, ColumnCount = pColumns->ColumnCount(); ColumnIndex < ColumnCount; ColumnIndex++ )
        {
            pColumn = (*pColumns)[ ColumnIndex ];
            delete ( pColumn );
        }
        delete ( pColumns );
    }

}

/*--- Arrange - Arrange the icons --------------------------------------------*/
void CWListView::Arrange (
    UINT                                Code )

{

    RECT                                ClientRect;
    int                                 ItemsPerRow;
    int                                 ItemsPerColumn;
    DWORD                               ItemIndex;

    DWORD                               ItemCount = GetItemCount();
    DWORD                               Style     = GetWindowLong ( hWnd, GWL_STYLE );


    if ( Code & LVA_ALIGNLEFT )
    {
        Style &= ~LVS_ALIGNTOP;
        Style |= LVS_ALIGNLEFT;
    }
    else
    if ( Code & LVA_ALIGNTOP )
    {
        Style &= ~LVS_ALIGNLEFT;
        Style |= LVS_ALIGNTOP;
    }

    GetClientRect ( hWnd, &ClientRect );
    CalcIconSizes();
    if ( ! ( ItemsPerRow = ( ClientRect.right - ClientRect.left ) / CurrMaxIconWidth ) )
        ItemsPerRow = 1;
    if ( ! ( ItemsPerColumn = ( ClientRect.bottom - ClientRect.top ) / CurrMaxIconHeight ) )
        ItemsPerColumn = 1;
    switch ( Style & LVS_ALIGNMASK )           
    {
        case LVS_ALIGNTOP:
            SetExtent ( ClientRect.right - ClientRect.left,
                round ( ( float )ItemCount / ( float )ItemsPerRow ) * CurrMaxIconHeight );
            for ( ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++ ) 
            {
                NextLargeIconPoint.x = ( ItemIndex / ItemsPerRow ) * CurrMaxIconWidth;
                NextLargeIconPoint.y = ( ItemIndex % ItemsPerRow ) * CurrMaxIconHeight;
                pColumns->SetItemPosition ( 0, ItemIndex, &NextLargeIconPoint );
            }
            NextLargeIconPoint.x = ( ItemCount / ItemsPerRow ) * CurrMaxIconWidth;
            NextLargeIconPoint.y = ( ItemCount % ItemsPerRow ) * CurrMaxIconHeight;
            break;

        case LVS_ALIGNLEFT:
            SetExtent ( round ( ( float )ItemCount / ( float )ItemsPerColumn ) * CurrMaxIconWidth,
                ClientRect.bottom - ClientRect.top );
            for ( ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++ ) 
            {
                NextLargeIconPoint.x = ( ItemIndex % ItemsPerColumn ) * CurrMaxIconWidth;
                NextLargeIconPoint.y = ( ItemIndex / ItemsPerColumn ) * CurrMaxIconHeight;
                pColumns->SetItemPosition ( 0, ItemIndex, &NextLargeIconPoint );
            }
            NextLargeIconPoint.x = ( ItemCount % ItemsPerColumn ) * CurrMaxIconWidth;
            NextLargeIconPoint.y = ( ItemCount / ItemsPerColumn ) * CurrMaxIconHeight;
            break;
    }
    InvalidateRect ( hWnd, NULL, TRUE );
    UpdateWindow ( hWnd );

}

/*--- LVSetBKColor - Set the Background Color ----------------------------------*/
BOOL CWListView::LVSetBKColor (
    COLORREF                            NewBKColor )

{

    HBRUSH                              hNewBrush;
    LOGBRUSH                            LogBrush;



    memset ( &LogBrush, 0, sizeof ( LogBrush ) );
    LogBrush.lbStyle = BS_SOLID;
    LogBrush.lbColor = NewBKColor;
    if ( ! ( hNewBrush = CreateBrushIndirect ( &LogBrush ) ) )
        return ( FALSE );

    if ( hBackgroundBrush )
        DeleteObject ( hBackgroundBrush );

    hBackgroundBrush = hNewBrush;

    BKColor = NewBKColor;

    return ( TRUE );

}

/*--- LVSetTextBKColor - Set the Text Background Color -------------------------*/
BOOL CWListView::LVSetTextBKColor (
    COLORREF                            NewBKColor )

{

    TextBKColor = NewBKColor;

    return ( TRUE );

}

/*--- LVSetTextColor - Set the Text Color --------------------------------------*/
BOOL CWListView::LVSetTextColor (
    COLORREF                            NewBKColor )

{

    TextColor = NewBKColor;

    return ( TRUE );

}

/*--- GetImageList - Get HIMAGELIST according to the type --------------------*/
HIMAGELIST CWListView::GetImageList (
    ELV_IMAGELIST_TYPES                 ImageListType )

{

    switch ( ImageListType )
    {
        case ELV_IMAGELIST_LARGE:
            return ( hLargeImageList );

        case ELV_IMAGELIST_SMALL:
            return ( hSmallImageList );

    }

    return ( ( HIMAGELIST )NULL );

}

/*--- SetImageList - Set the image list according to the type ----------------*/
BOOL CWListView::SetImageList (
    ELV_IMAGELIST_TYPES                 ImageListType,
    HIMAGELIST                          hImageList )

{

    switch ( ImageListType )
    {
        default:
            return ( FALSE );

        case ELV_IMAGELIST_LARGE:
            hLargeImageList = hImageList;

        case ELV_IMAGELIST_SMALL:
            hSmallImageList = hImageList;

    }

    return ( TRUE );

}

/*--- InsertColumn - Insert a new column -------------------------------------*/
int CWListView::InsertColumn (
    DWORD                               IndexBefore,
    LV_COLUMN                           *pLVColumn )

{

    HD_ITEM                             HDItem;
    int                                 ExceptionError;
	int *			SaveCatchBuffer;


#ifdef WEXCEPTIONS
    try
    {
        if ( CreatingDummyColumn )
            pColumns->InsertColumn ( IndexBefore, pLVColumn );
        else
        if ( ! UserFirstColumnCreated )
        {
            CreatingFirstColumn    = TRUE;
            UserFirstColumnCreated = SetColumn ( IndexBefore, pLVColumn );
            CreatingFirstColumn    = FALSE;
        }
        else
        {
            pColumns->InsertColumn ( IndexBefore, pLVColumn );
            LVColumnToHDItem ( pLVColumn, &HDItem );
            if ( Header_InsertItem ( hHeaderWnd, IndexBefore, &HDItem ) == -1 )
                return ( WLV_STANDARDERROR );
        }
    }
    catch ( ExceptionError )
    {
        pWCommCtrl->Error ( ExceptionError );
        return ( WLV_STANDARDERROR );
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
	{
		if ( CreatingDummyColumn )
			pColumns->InsertColumn( IndexBefore, pLVColumn );
		else if ( ! UserFirstColumnCreated )
		{
			CreatingFirstColumn = TRUE;
			UserFirstColumnCreated = SetColumn( IndexBefore, pLVColumn );
			CreatingFirstColumn = FALSE;
		}
		else
		{
			pColumns->InsertColumn( IndexBefore, pLVColumn );
			LVColumnToHDItem ( pLVColumn, &HDItem );
			if ( Header_InsertItem( hHeaderWnd, IndexBefore, &HDItem ) == -1 )
			{
				WinFree( CatchBuffer );
				CatchBuffer = SaveCatchBuffer;
				return( WLV_STANDARDERROR );
			}
		}
	}
	else
	{
		pWCommCtrl->Error( ExceptionError );
		WinFree( CatchBuffer );
		CatchBuffer = SaveCatchBuffer;
		return( WLV_STANDARDERROR );
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif

    return ( ( int )IndexBefore );

}

/*--- DeleteColumn - Deletes a column ----------------------------------------*/
BOOL CWListView::DeleteColumn (
    DWORD                               ColumnIndex )

{

    LV_COLUMN                           LVColumn;
    DWORD                               ColumnCount = pColumns->ColumnCount();
    int                                 ExceptionError;
	int *			SaveCatchBuffer;


    switch ( ColumnCount )
    {
        case 0:
            return ( FALSE );

        case 1:
            if ( ColumnIndex )
                return ( FALSE );
            UserFirstColumnCreated = FALSE;
            Header_DeleteItem ( hHeaderWnd, 0 );
            memset ( &LVColumn, 0, sizeof ( LV_COLUMN ) );
            LVColumn.mask = 0xFFFFFFFF;
            SetColumn ( 0, &LVColumn );
            return ( TRUE );
    }

#ifdef WEXCEPTIONS
    try
    {
        pColumns->DeleteColumn ( ColumnIndex );
        Header_DeleteItem ( hHeaderWnd, ColumnIndex );
    }
    catch ( ExceptionError )
    {
        pWCommCtrl->Error ( ExceptionError );
        return ( FALSE );
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0)
	{
		pColumns->DeleteColumn( ColumnIndex );
		Header_DeleteItem( hHeaderWnd, ColumnIndex );
	}
	else
	{	
		pWCommCtrl->Error( ExceptionError );
		WinFree( CatchBuffer );
		CatchBuffer = SaveCatchBuffer;
		return ( FALSE );
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif

    return ( TRUE );

}

/*--- InsertItem - Insert a new Item row -------------------------------------*/
int CWListView::InsertItem (
    DWORD                               IndexBefore,
    LV_ITEM                             *pLVItem )

{

    DWORD                               Style = GetWindowLong ( hWnd, GWL_STYLE );
    int                                 ExceptionError;


#ifdef WEXCEPTIONS
    try
    {
        pColumns->InsertItemRow ( IndexBefore );
        pColumns->SetItem ( 0, IndexBefore, pLVItem );
        if ( GetItemCount() == 1 )
            CalcIconSizes();
        pColumns->SetItemPosition ( 0, IndexBefore, &NextLargeIconPoint );
        switch ( Style & LVS_ALIGNMASK )
        {
            case LVS_ALIGNLEFT:
                if ( NextLargeIconPoint.x + 2 * ( int )CurrMaxIconWidth <= GetXExtent() )
                    NextLargeIconPoint.x += CurrMaxIconWidth;
                else
                {
                    NextLargeIconPoint.x = 0;
                    NextLargeIconPoint.y += CurrMaxIconHeight;
                }
                break;

            case LVS_ALIGNTOP:
                if ( NextLargeIconPoint.y + 2 * ( int )CurrMaxIconHeight <= GetYExtent() )
                    NextLargeIconPoint.y += CurrMaxIconHeight;
                else
                {
                    NextLargeIconPoint.x += CurrMaxIconWidth;
                    NextLargeIconPoint.y = 0;
                }
                break;
        }
    }
    catch ( ExceptionError )
    {
        pWCommCtrl->Error ( ExceptionError );
        return ( WLV_STANDARDERROR );
    }
#else
    if ( ExceptionError )
    {
        pWCommCtrl->Error ( ExceptionError );
        return ( WLV_STANDARDERROR );
    }
    else
    {
        pColumns->InsertItemRow ( IndexBefore );
        pColumns->SetItem ( 0, IndexBefore, pLVItem );
        if ( GetItemCount() == 1 )
            CalcIconSizes();
        pColumns->SetItemPosition ( 0, IndexBefore, &NextLargeIconPoint );
        switch ( Style & LVS_ALIGNMASK )
        {
            case LVS_ALIGNLEFT:
                if ( NextLargeIconPoint.x + 2 * ( int )CurrMaxIconWidth <= GetXExtent() )
                    NextLargeIconPoint.x += CurrMaxIconWidth;
                else
                {
                    NextLargeIconPoint.x = 0;
                    NextLargeIconPoint.y += CurrMaxIconHeight;
                }
                break;

            case LVS_ALIGNTOP:
                if ( NextLargeIconPoint.y + 2 * ( int )CurrMaxIconHeight <= GetYExtent() )
                    NextLargeIconPoint.y += CurrMaxIconHeight;
                else
                {
                    NextLargeIconPoint.x += CurrMaxIconWidth;
                    NextLargeIconPoint.y = 0;
                }
                break;
        }
    }
#endif

    return ( ( int ) IndexBefore );

}

/*--- DeleteItem - Deletes an item row ---------------------------------------*/
BOOL CWListView::DeleteItem (
    DWORD                               ItemIndex )

{

    int                                 ExceptionError;
	int *			SaveCatchBuffer;


#ifdef WEXCEPTIONS
    try
    {
        pColumns->DeleteItemRow ( ItemIndex );
    }
    catch ( ExceptionError )
    {
        pWCommCtrl->Error ( ExceptionError );
        return ( FALSE );
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
	{
		pColumns->DeleteItemRow( ItemIndex );
	}
	else
	{
		pWCommCtrl->Error( ExceptionError );
		WinFree( CatchBuffer );
		CatchBuffer = SaveCatchBuffer;
		return( FALSE );
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif

    return ( TRUE );

}

/*--- GetColumn - Gets the data for a column ---------------------------------*/
BOOL CWListView::GetColumn (
    DWORD                               ColumnIndex,
    LV_COLUMN                           *pLVColumn )

{

    int                                 ExceptionError;
	int * 			SaveCatchBuffer;	


    if ( ! UserFirstColumnCreated )
        return ( FALSE );

#ifdef WEXCEPTIONS
    try
    {
        pColumns->GetColumn ( ColumnIndex, pLVColumn );
    }
    catch ( ExceptionError )
    {
        pWCommCtrl->Error ( ExceptionError );
        return ( FALSE );
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
	{
		pColumns->GetColumn ( ColumnIndex, pLVColumn );
	}
	else
	{
		pWCommCtrl->Error( ExceptionError );
		WinFree( CatchBuffer );
		CatchBuffer = SaveCatchBuffer;	
		return( FALSE );
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif

    return ( TRUE );

}

/*--- GetColumn - Sets the data for a column ---------------------------------*/
BOOL CWListView::SetColumn (
    DWORD                               ColumnIndex,
    LV_COLUMN                           *pLVColumn )

{

    HD_ITEM                             HDItem;
    int                                 ExceptionError;

    BOOL                                ReturnValue = TRUE;
	int *			SaveCatchBuffer;


    if ( ( ! CreatingDummyColumn ) && ( ! UserFirstColumnCreated ) && ( ! CreatingFirstColumn ) )
        ReturnValue = FALSE;
    else
    {
#ifdef WEXCEPTIONS
        try
        {
            pColumns->SetColumn ( ColumnIndex, pLVColumn );
            LVColumnToHDItem ( pLVColumn, &HDItem );
            if ( UserFirstColumnCreated )
                ReturnValue = Header_SetItem ( hHeaderWnd, ColumnIndex, &HDItem );
            else
                ReturnValue = ( Header_InsertItem ( hHeaderWnd, ColumnIndex, &HDItem ) != -1 );
        }
        catch ( ExceptionError )
        {
            pWCommCtrl->Error ( ExceptionError );
            ReturnValue = FALSE;
        }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
	{
		pColumns->SetColumn( ColumnIndex, pLVColumn );
		LVColumnToHDItem( pLVColumn, &HDItem );
		if ( UserFirstColumnCreated )
			ReturnValue = Header_SetItem( hHeaderWnd, ColumnIndex, &HDItem );
		else
			ReturnValue = (Header_InsertItem( hHeaderWnd, ColumnIndex, &HDItem ) != -1 );
	}
	else
	{
		pWCommCtrl->Error( ExceptionError );
		ReturnValue = FALSE;
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif
    }

    return ( ReturnValue );

}

/*--- GetItem - Gets the data for an item ------------------------------------*/
BOOL CWListView::GetItem (
    DWORD                               ColumnIndex,
    DWORD                               ItemIndex,
    LV_ITEM                             *pLVItem )

{

    int                                 ExceptionError;
	int *			SaveCatchBuffer;


#ifdef WEXCEPTIONS
    try
    {
        pColumns->GetItem ( ColumnIndex, ItemIndex, pLVItem );
    }
    catch ( ExceptionError )
    {
        pWCommCtrl->Error ( ExceptionError );
        return ( FALSE );
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 ) 
	{
		pColumns->GetItem( ColumnIndex, ItemIndex, pLVItem );
	}
	else
	{
		pWCommCtrl->Error( ExceptionError );
		WinFree( CatchBuffer );
		CatchBuffer = SaveCatchBuffer;
		return( FALSE );
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif

    return ( TRUE );

}

/*--- GetColumn - Sets the data for an item ----------------------------------*/
BOOL CWListView::SetItem (
    DWORD                               ColumnIndex,
    DWORD                               ItemIndex,
    LV_ITEM                             *pLVItem )

{

    int                                 ExceptionError;
	int *			SaveCatchBuffer;


#ifdef WEXCEPTIONS
    try
    {
        pColumns->SetItem ( ColumnIndex, ItemIndex, pLVItem );
    }
    catch ( ExceptionError )
    {
        pWCommCtrl->Error ( ExceptionError );
        return ( FALSE );
    }
#else
	SaveCatchBuffer = CatchBuffer; 
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 ) 
	{
		pColumns->SetItem( ColumnIndex, ItemIndex, pLVItem );
	}
	else
	{
		pWCommCtrl->Error( ExceptionError );
		WinFree( CatchBuffer );
		CatchBuffer = SaveCatchBuffer;
		return( FALSE );
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif

    return ( TRUE );

}

/*--- DeleteAllItems - Deletes all item rows ---------------------------------*/
BOOL CWListView::DeleteAllItems ()

{

    DWORD                               ItemCount;
    int                                 ExceptionError;
	int *			SaveCatchBuffer;


#ifdef WEXCEPTIONS
    try
    {
        for ( ItemCount = pColumns->ItemCount(); ItemCount--; /*--- No itteration ---*/ )
            pColumns->DeleteItemRow ( ItemCount );
    }
    catch ( ExceptionError )
    {
        pWCommCtrl->Error ( ExceptionError );
        return ( FALSE );
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
	{
		for ( ItemCount = pColumns->ItemCount(); ItemCount--; )
			pColumns->DeleteItemRow ( ItemCount );
	}
	else
	{
		pWCommCtrl->Error( ExceptionError );
		WinFree( CatchBuffer );
		CatchBuffer = SaveCatchBuffer;
		return( FALSE );
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif

    return ( TRUE );

}

/*--- GetItemCount - Gets the number of rows/items ---------------------------*/
DWORD CWListView::GetItemCount ()

{

    return ( ( int )pColumns->ItemCount() );

}

/*--- FindItem - Finds and item according to the Find Info -------------------*/
DWORD CWListView::FindItem (
    DWORD                               IndexToStart,
    LV_FINDINFO                         *pFindInfo )

{

    LV_ITEM                             Item;
    BOOL                                ParamMatch;
    BOOL                                TextMatch;
    UINT                                Length;
    int                                 ExceptionError;

    BOOL                                Found          = FALSE;
    DWORD                               ItemCount      = GetItemCount();
    DWORD                               Index          = ( IndexToStart == ( DWORD )-1 ) ? 0 : IndexToStart;
   	int * 			SaveCatchBuffer;
    
#ifdef WEXCEPTIONS
    try
    {
        Item.mask = LVIF_TEXT | LVIF_PARAM;
        if ( pFindInfo->flags & LVFI_STRING )
            Length = ( int )strlen ( pFindInfo->psz );
        else
            TextMatch = TRUE;
        while ( ( Index < ItemCount ) && ( ! Found ) )
        {
            GetItem ( 0, Index, &Item );
            ParamMatch = ( ( pFindInfo->flags & LVFI_PARAM ) ? ( pFindInfo->lParam == Item.lParam ) : TRUE );
            if ( pFindInfo->flags & LVFI_STRING ) 
                if ( pFindInfo->flags & LVFI_PARTIAL )
                    if ( Length <= strlen ( Item.pszText ) )
                        TextMatch = ( ! ( strncmp ( Item.pszText, pFindInfo->psz, Length ) ) );
                    else
                        TextMatch = FALSE;
                else
                    TextMatch = ( ( ! ( strcmp ( Item.pszText, pFindInfo->psz ) ) ) &&
                                    ( Length == strlen ( Item.pszText ) ) );
            Found = ( ParamMatch && TextMatch );
            Index++;
        }
        if ( ( ! Found ) && ( IndexToStart ) && ( IndexToStart != ( DWORD )-1 ) )
        {
            Index = 0;
            while ( ( Index < IndexToStart ) && ( ! Found ) )
            {
                GetItem ( 0, Index, &Item );
                ParamMatch = ( ( pFindInfo->flags & LVFI_PARAM ) ? ( pFindInfo->lParam == Item.lParam ) : TRUE );
                if ( pFindInfo->flags & LVFI_STRING ) 
                    if ( pFindInfo->flags & LVFI_PARTIAL )
                        if ( Length <= strlen ( Item.pszText ) )
                            TextMatch = ( ! ( strncmp ( Item.pszText, pFindInfo->psz, Length ) ) );
                        else
                            TextMatch = FALSE;
                    else
                        TextMatch = ( ( ! ( strcmp ( Item.pszText, pFindInfo->psz ) ) ) &&
                                        ( Length == strlen ( Item.pszText ) ) );
                Found = ( ParamMatch && TextMatch );
                Index++;
            }
        }
    }
    catch ( ExceptionError )
    {
        pWCommCtrl->Error ( ExceptionError );
        return ( ( DWORD )WLV_STANDARDERROR );
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
    {
        Item.mask = LVIF_TEXT | LVIF_PARAM;
        if ( pFindInfo->flags & LVFI_STRING )
            Length = ( int )strlen ( pFindInfo->psz );
        else
            TextMatch = TRUE;
        while ( ( Index < ItemCount ) && ( ! Found ) )
        {
            GetItem ( 0, Index, &Item );
            ParamMatch = ( ( pFindInfo->flags & LVFI_PARAM ) ? ( pFindInfo->lParam == Item.lParam ) : TRUE );
            if ( pFindInfo->flags & LVFI_STRING ) 
                if ( pFindInfo->flags & LVFI_PARTIAL )
                    if ( Length <= strlen ( Item.pszText ) )
                        TextMatch = ( ! ( strncmp ( Item.pszText, pFindInfo->psz, Length ) ) );
                    else
                        TextMatch = FALSE;
                else
                    TextMatch = ( ( ! ( strcmp ( Item.pszText, pFindInfo->psz ) ) ) &&
                                    ( Length == strlen ( Item.pszText ) ) );
            Found = ( ParamMatch && TextMatch );
            Index++;
        }
        if ( ( ! Found ) && ( IndexToStart ) && ( IndexToStart != ( DWORD )-1 ) )
        {
            Index = 0;
            while ( ( Index < IndexToStart ) && ( ! Found ) )
            {
                GetItem ( 0, Index, &Item );
                ParamMatch = ( ( pFindInfo->flags & LVFI_PARAM ) ? ( pFindInfo->lParam == Item.lParam ) : TRUE );
                if ( pFindInfo->flags & LVFI_STRING ) 
                    if ( pFindInfo->flags & LVFI_PARTIAL )
                        if ( Length <= strlen ( Item.pszText ) )
                            TextMatch = ( ! ( strncmp ( Item.pszText, pFindInfo->psz, Length ) ) );
                        else
                            TextMatch = FALSE;
                    else
                        TextMatch = ( ( ! ( strcmp ( Item.pszText, pFindInfo->psz ) ) ) &&
                                        ( Length == strlen ( Item.pszText ) ) );
                Found = ( ParamMatch && TextMatch );
                Index++;
            }
        }
    }
	else
	{
		pWCommCtrl->Error( ExceptionError );
		WinFree( CatchBuffer );
		CatchBuffer = SaveCatchBuffer;
		return( ( DWORD ) WLV_STANDARDERROR );
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;	
#endif

    return ( Found ? ( DWORD )Index : ( DWORD )WLV_STANDARDERROR );

}

/*--- GetItemState - Gets the item's state according to the mask -------------*/
UINT CWListView::GetItemState ( 
    DWORD                               ItemIndex,
    UINT                                Mask )

{

    UINT                                ReturnMask;
    LV_ITEM                             Item;
    int                                 ExceptionError;
	int *			SaveCatchBuffer;


#ifdef WEXCEPTIONS
    try
    {
        Item.mask = LVIF_STATE;
        GetItem ( 0, ItemIndex, &Item );
        ReturnMask = 0;
        ReturnMask |= Mask & LVIS_SELECTED       ? Item.state & LVIS_SELECTED       : 0;
        ReturnMask |= Mask & LVIS_FOCUSED        ? Item.state & LVIS_FOCUSED        : 0;
        ReturnMask |= Mask & LVIS_CUT            ? Item.state & LVIS_CUT            : 0;
        ReturnMask |= Mask & LVIS_DROPHILITED    ? Item.state & LVIS_DROPHILITED    : 0;
        ReturnMask |= Mask & LVIS_OVERLAYMASK    ? Item.state & LVIS_OVERLAYMASK    : 0;
        ReturnMask |= Mask & LVIS_STATEIMAGEMASK ? Item.state & LVIS_STATEIMAGEMASK : 0;
    }
    catch ( ExceptionError )
    {
        pWCommCtrl->Error ( ExceptionError );
        return ( 0 );
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
    {
        Item.mask = LVIF_STATE;
        GetItem ( 0, ItemIndex, &Item );
        ReturnMask = 0;
        ReturnMask |= Mask & LVIS_SELECTED       ? Item.state & LVIS_SELECTED       : 0;
        ReturnMask |= Mask & LVIS_FOCUSED        ? Item.state & LVIS_FOCUSED        : 0;
        ReturnMask |= Mask & LVIS_CUT            ? Item.state & LVIS_CUT            : 0;
        ReturnMask |= Mask & LVIS_DROPHILITED    ? Item.state & LVIS_DROPHILITED    : 0;
        ReturnMask |= Mask & LVIS_OVERLAYMASK    ? Item.state & LVIS_OVERLAYMASK    : 0;
        ReturnMask |= Mask & LVIS_STATEIMAGEMASK ? Item.state & LVIS_STATEIMAGEMASK : 0;
    }
	else
	{
		pWCommCtrl->Error( ExceptionError );
		WinFree( CatchBuffer );
		CatchBuffer = SaveCatchBuffer;
		return( 0 );
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif

    return ( ReturnMask );

}

/*--- GetItemState - Gets the item's state according to the mask -------------*/
BOOL CWListView::SetItemState (
    DWORD                               ItemIndex,
    LV_ITEM                             *pLVItem )

{

    int                                 ExceptionError;
	int *			SaveCatchBuffer;

#ifdef WEXCEPTIONS
    try
    {
        SetItem ( 0, ItemIndex, pLVItem );
    }
    catch ( ExceptionError )
    {
        pWCommCtrl->Error ( ExceptionError );
        return ( FALSE );
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
	{
		SetItem( 0, ItemIndex, pLVItem );
	}
	else
	{
		pWCommCtrl->Error( ExceptionError );
		WinFree( CatchBuffer );
		CatchBuffer = SaveCatchBuffer;
		return( FALSE );
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif

    return ( TRUE );

}

/*--- GetItemText - Gets the item's Text -------------------------------------*/
int CWListView::GetItemText (
    DWORD                               ItemIndex,
    LV_ITEM                             *pLVItem )

{

    int                                 ExceptionError;
	int *			SaveCatchBuffer;


#ifdef WEXCEPTIONS
    try
    {
        pLVItem->mask = LVIF_TEXT;
        GetItem ( pLVItem->iSubItem, ItemIndex, pLVItem );
    }
    catch ( ExceptionError )
    {
        pWCommCtrl->Error ( ExceptionError );
        return ( 0 );
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
	{
		pLVItem->mask = LVIF_TEXT;
		GetItem( pLVItem->iSubItem, ItemIndex, pLVItem );
	}
	else
	{
		pWCommCtrl->Error( ExceptionError );
		WinFree( CatchBuffer );
		CatchBuffer = SaveCatchBuffer;
		return( 0 );
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif

    return ( strlen ( pLVItem->pszText ) );

}

/*--- SetItemText - Sets the item's Text -------------------------------------*/
int CWListView::SetItemText (
    DWORD                               ItemIndex,
    LV_ITEM                             *pLVItem )

{

    int                                 ExceptionError;
	int *			SaveCatchBuffer;


#ifdef WEXCEPTIONS
    try
    {
        SetItem ( pLVItem->iSubItem, ItemIndex, pLVItem );
    }
    catch ( ExceptionError )
    {
        pWCommCtrl->Error ( ExceptionError );
        return ( FALSE );
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
	{
		SetItem( pLVItem->iSubItem, ItemIndex, pLVItem );
	}
	else
	{
		pWCommCtrl->Error( ExceptionError );
		WinFree( CatchBuffer );
		CatchBuffer = SaveCatchBuffer;
		return( FALSE );
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif

    return ( TRUE );

}

/*--- GetColumnWidth - Get the width of a column -----------------------------*/
int CWListView::GetColumnWidth (
    DWORD                               ColumnIndex )

{

    LV_COLUMN                           LVColumn;
    int                                 ExceptionError;
	int * 			SaveCatchBuffer;

    if ( ! UserFirstColumnCreated )
        return ( 0 );

    LVColumn.mask = LVCF_WIDTH;

#ifdef WEXCEPTIONS
    try
    {
        GetColumn ( ColumnIndex, &LVColumn );
    }
    catch ( ExceptionError )
    {
        pWCommCtrl->Error ( ExceptionError );
        return ( 0 );
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
	{
		GetColumn( ColumnIndex, &LVColumn );
	}
	else
	{
		pWCommCtrl->Error ( ExceptionError );
		WinFree( CatchBuffer );
		CatchBuffer = SaveCatchBuffer;
		return( 0 );
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer; 
	
	
#endif

    return ( LVColumn.cx );

}

/*--- SetColumnWidth - Set the width of a column -----------------------------*/
BOOL CWListView::SetColumnWidth (
    DWORD                               ColumnIndex,
    DWORD                               Width )

{

    LV_COLUMN                           LVColumn;
    HD_ITEM                             HDItem;
    int                                 ExceptionError;
	int *			SaveCatchBuffer;


    if ( ! UserFirstColumnCreated )
        return ( FALSE );

    LVColumn.mask = LVCF_WIDTH;
    LVColumn.cx   = Width;

#ifdef WEXCEPTIONS
    try
    {
        SetColumn ( ColumnIndex, &LVColumn );
        LVColumnToHDItem ( &LVColumn, &HDItem );
        Header_SetItem ( hHeaderWnd, ColumnIndex, &HDItem );
    }
    catch ( ExceptionError )
    {
        pWCommCtrl->Error ( ExceptionError );
        return ( FALSE );
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
	{
		SetColumn( ColumnIndex, &LVColumn );
		LVColumnToHDItem( &LVColumn, &HDItem );
		Header_SetItem( hHeaderWnd, ColumnIndex, &HDItem );
	}
	else
	{
		pWCommCtrl->Error( ExceptionError );
		WinFree( CatchBuffer );
		CatchBuffer = SaveCatchBuffer;
		return( FALSE );
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif

    return ( TRUE );

}

/*--- Sort - Sort Items ------------------------------------------------------*/
BOOL CWListView::Sort (
    PFNLVCOMPARE                        pfnCompare,
    LPARAM                              lParam )

{

    return ( (((*pColumns)[0])->GetItems())->Sort ( ( PFNWCOMPARE )pfnCompare, lParam ) );

}



/*--- SetItemCount - Set the number of items ---------------------------------*/
BOOL CWListView::SetItemCount (
    DWORD                               NewItemCount )

{

    DWORD                               ItemIndex;
    LV_ITEM                             LVItem;
    int                                 ExceptionError;
	int *			SaveCatchBuffer;


    memset ( &LVItem, 0, sizeof ( LV_ITEM ) );

#ifdef WEXCEPTIONS
    try
    {
        if ( NewItemCount > GetItemCount() )
            for ( ItemIndex = GetItemCount(); ItemIndex < NewItemCount; ItemIndex++ )
                InsertItem ( ItemIndex, &LVItem );
        return ( TRUE );
    }
    catch ( ExceptionError )
    {
        pWCommCtrl->Error ( ExceptionError );
        return ( FALSE );
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
	{
		if ( NewItemCount > GetItemCount() )
			for ( ItemIndex = GetItemCount(); ItemIndex < NewItemCount; ItemIndex++ )
				InsertItem ( ItemIndex, &LVItem );
		WinFree( CatchBuffer );
		CatchBuffer = SaveCatchBuffer;
		return( TRUE );
	}
	else
	{
		pWCommCtrl->Error( ExceptionError );
		WinFree( CatchBuffer );
		CatchBuffer = SaveCatchBuffer;
		return( FALSE );
	}
#endif

}

/*--- GetViewRect - Get the total View area ----------------------------------*/
BOOL CWListView::GetViewRect (
    LPRECT                              pRect )

{

    pRect->top    = 0;
    pRect->left   = 0;
    pRect->bottom = GetYExtent();
    pRect->right  = GetXExtent();

    return ( TRUE );

}

/*--- GetOrigin - Get the current origin -------------------------------------*/
BOOL CWListView::GetOrigin (
    LPPOINT                             pPoint )

{

    pPoint->x = GetXOrigin();
    pPoint->y = GetYOrigin();

    return ( TRUE );

}

/*--- GetStringWidth - Calculate the width of a string -----------------------*/
int CWListView::GetStringWidth (
    LPSTR                               pString )

{

    HDC                                 hDC;
    RECT                                Rect;


    if ( ! ( hDC = GetDC ( hWnd ) ) )
        Throw ( CatchBuffer, WERR_GETDC );

    memset ( &Rect, 0, sizeof ( RECT ) );
    DrawText ( hDC, pString, strlen ( pString ), &Rect, DT_CALCRECT );

    ReleaseDC ( hWnd, hDC );

    return ( Rect.right - Rect.left );

}

/*--- Scroll - Scroll the client area ----------------------------------------*/
BOOL CWListView::Scroll (
    int                                 dx,
    int                                 dy )

{

    int                                 NewPosition;

    if ( dx )
    {
        NewPosition = GetXOrigin() + dx;
        if ( ( NewPosition < 0 ) || ( GetXExtent() < NewPosition ) )
            return ( FALSE );
        OnWM_HSCROLL ( SB_THUMBPOSITION, NewPosition );
    }

    if ( dy )
    {
        NewPosition = GetYOrigin() + dy;
        if ( ( NewPosition < 0 ) || ( GetYExtent() < NewPosition ) )
            return ( FALSE );
        OnWM_VSCROLL ( SB_THUMBPOSITION, NewPosition );
    }

    return ( TRUE );

}
/*--- RedrawItems - Redraw the items specified -------------------------------*/
BOOL CWListView::RedrawItems (
    DWORD                               StartIndex,
    DWORD                               FinishIndex,
    LPRECT                              pPaintRect )

{

    RECT                                ClientRect;
    RECT                                HeaderRect;
    DWORD                               ItemIndex;
    DWORD                               StringWidth;
    DWORD                               Column;
    DWORD                               ColumnCount;
    DWORD                               TotalWidth;
    HFONT                               hFont;
    HIMAGELIST                          hImageList;
    int                                 X;
    int                                 Y;
    int                                 ExceptionError;

    DWORD                               IconWidth      = 0;
    DWORD                               Style          = GetWindowLong ( hWnd, GWL_STYLE );
    HDC                                 hDC            = ( HDC )NULL;
    DWORD                               RowHeight      = GetRowHeight();
    DWORD                               ItemsPerColumn = GetItemsPerColumn();
    DWORD                               ItemsPerRow    = GetItemsPerRow();
    DWORD                               ItemCount      = GetItemCount();
    BOOL                                ReturnValue    = FALSE;
	int *			SaveCatchBuffer;


#ifdef WEXCEPTIONS
    try
    {
        if ( ! ( hDC = GetDC ( hWnd ) ) )
            Throw ( CatchBuffer, WERR_GETDC );

        if ( hFont = ( HFONT )SendMessage ( hWnd, WM_GETFONT, ( WPARAM )NULL, ( LPARAM )NULL ) ) 
            SelectObject ( hDC, hFont );
                                                             
        GetClientRect ( hWnd, &ClientRect );

        ShowWindow ( hHeaderWnd, ( ( Style & LVS_TYPEMASK ) == LVS_REPORT ) ? SW_SHOW : SW_HIDE );

        switch ( Style & LVS_TYPEMASK )
        {
            case LVS_SMALLICON:
            case LVS_LIST:
                CurrMaxWidth = WLVD_MINHORIZONTALWIDTH;
                if ( hImageList = GetImageList ( ELV_IMAGELIST_SMALL ) )
                {
                    X = 0;
                    ImageList_GetIconSize ( hImageList, &X, &Y );
                    IconWidth = ( DWORD )X + WVLD_HORIZONTALSPACING;
                }
                for ( ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++ )
                    if ( CurrMaxWidth < ( StringWidth = GetItemStringWidth ( ItemIndex ) + IconWidth ) )
                        CurrMaxWidth = StringWidth;
                CurrHeight = GetStringHeight();
                if ( ( Style & LVS_TYPEMASK ) == LVS_LIST )
                    SetExtent ( round( ( float )ItemCount / ( float )ItemsPerColumn ) * ( CurrMaxWidth + WVLD_HORIZONTALSPACING ),
                        ClientRect.bottom - ClientRect.top );
                else
                    SetExtent ( ClientRect.right - ClientRect.left,
                        round ( ( float )ItemCount / ( float )ItemsPerRow ) * RowHeight );
                break;

            case LVS_REPORT:
                GetWindowRect ( hHeaderWnd, &HeaderRect );
                SetWindowPos ( hHeaderWnd, ( HWND )NULL, (-1) * GetXOrigin(), 0, 
                    GetXOrigin() + ( ClientRect.right - ClientRect.left ), 
                    HeaderRect.bottom - HeaderRect.top, SWP_NOZORDER | SWP_SHOWWINDOW );
                InvalidateRect ( hHeaderWnd, NULL, FALSE );
                UpdateWindow ( hHeaderWnd );
                for ( Column = 0, ColumnCount = pColumns->ColumnCount(), TotalWidth = 0; Column < ColumnCount; Column++ )
                    TotalWidth += GetColumnWidth ( Column );
                SetExtent ( TotalWidth,
                    ItemCount * ( CurrHeight + WLVD_VERTICALSTRINGSPACEING ) + RowHeight );
                break;

            case LVS_ICON:
                break;
        }

        if ( hDC )
        {
            ReleaseDC ( hWnd, hDC );
            hDC = ( HDC )NULL;
        }
        while ( ( StartIndex <= FinishIndex ) &&
            ( ReturnValue = Update ( StartIndex, pPaintRect ) ) )
            StartIndex++;
    }
    catch ( ExceptionError )
    {
        if ( hDC )
        {
            ReleaseDC ( hWnd, hDC );
            hDC = ( HDC )NULL;
        }
        pWCommCtrl->Error ( ExceptionError );
        ReturnValue = FALSE;
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
	{
        if ( ! ( hDC = GetDC ( hWnd ) ) )
            Throw ( CatchBuffer, WERR_GETDC );

        if ( hFont = ( HFONT )SendMessage ( hWnd, WM_GETFONT, ( WPARAM )NULL, ( LPARAM )NULL ) )
            SelectObject ( hDC, hFont );

        GetClientRect ( hWnd, &ClientRect );

        ShowWindow ( hHeaderWnd, ( ( Style & LVS_TYPEMASK ) == LVS_REPORT ) ? SW_SHOW : SW_HIDE )
;

        switch ( Style & LVS_TYPEMASK )
        {
            case LVS_SMALLICON:
            case LVS_LIST:
                CurrMaxWidth = WLVD_MINHORIZONTALWIDTH;
                if ( hImageList = GetImageList ( ELV_IMAGELIST_SMALL ) )
                {
                    X = 0;
                    ImageList_GetIconSize ( hImageList, &X, &Y );
                    IconWidth = ( DWORD )X + WVLD_HORIZONTALSPACING;
                }
                for ( ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++ )
                    if ( CurrMaxWidth < ( StringWidth = GetItemStringWidth ( ItemIndex ) + IconWidth ) )
                        CurrMaxWidth = StringWidth;
                CurrHeight = GetStringHeight();
                if ( ( Style & LVS_TYPEMASK ) == LVS_LIST )
                    SetExtent ( round( ( float )ItemCount / ( float )ItemsPerColumn ) * ( CurrMaxWidth + WVLD_HORIZONTALSPACING ), 
                        ClientRect.bottom - ClientRect.top );
                else
                    SetExtent ( ClientRect.right - ClientRect.left,
                        round ( ( float )ItemCount / ( float )ItemsPerRow ) * RowHeight );
                break;

            case LVS_REPORT:
                GetWindowRect ( hHeaderWnd, &HeaderRect );
                SetWindowPos ( hHeaderWnd, ( HWND )NULL, (-1) * GetXOrigin(), 0,
                    GetXOrigin() + ( ClientRect.right - ClientRect.left ),
                    HeaderRect.bottom - HeaderRect.top, SWP_NOZORDER | SWP_SHOWWINDOW );
                InvalidateRect ( hHeaderWnd, NULL, FALSE );
                UpdateWindow ( hHeaderWnd );
                for ( Column = 0, ColumnCount = pColumns->ColumnCount(), TotalWidth = 0; Column <
 ColumnCount; Column++ )
                    TotalWidth += GetColumnWidth ( Column );
                SetExtent ( TotalWidth,
                    ItemCount * ( CurrHeight + WLVD_VERTICALSTRINGSPACEING ) + RowHeight );
                break;

            case LVS_ICON:
                break;
        }

        if ( hDC )
        {
            ReleaseDC ( hWnd, hDC );
            hDC = ( HDC )NULL;
        }
        while ( ( StartIndex <= FinishIndex ) &&
            ( ReturnValue = Update ( StartIndex, pPaintRect ) ) )
            StartIndex++;
	}
	else
	{
        if ( hDC )
        {
            ReleaseDC ( hWnd, hDC );
            hDC = ( HDC )NULL;
        }
        pWCommCtrl->Error ( ExceptionError );
        ReturnValue = FALSE;
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;	
#endif

    return ( ReturnValue );

}

/*--- Update - Update a particular item --------------------------------------*/
BOOL CWListView::Update (
    DWORD                               ItemIndex,
    LPRECT                              pPaintRect )

{

    HDC                                 hDC;
    POINT                               Point;
    RECT                                ClientRect;
    RECT                                DrawRect;
    RECT                                ViewRect;
    RECT                                IntRect;
    DWORD                               Width;
    DWORD                               Height;
    DWORD                               ColumnIndex;
    DWORD                               ColumnCount;
    DWORD                               ColumnOffset;
    HIMAGELIST                          hImageList;
    DWORD                               ImageIndex;
    COLORREF                            BackgroundColor;
    CWLVColumn                          *pColumn;
    int                                 X;
    int                                 Y;

    DWORD                               RowHeight      = GetRowHeight ();
    DWORD                               ItemsPerColumn = GetItemsPerColumn();
    DWORD                               ItemsPerRow    = GetItemsPerRow();
    DWORD                               Style          = GetWindowLong ( hWnd, GWL_STYLE );
    DWORD                               DrawTextStyle  = DT_LEFT | DT_SINGLELINE;


    if ( ! ( hDC = GetDC ( hWnd ) ) )
        return ( FALSE );

    GetClientRect ( hWnd, &ClientRect );
    SetWindowOrgEx ( hDC, GetXOrigin(), GetYOrigin(), NULL );
    BackgroundColor = SetBkColor ( hDC, TextBKColor );
    SetTextColor ( hDC, TextColor );
    SelectObject ( hDC, hFont );
    if ( ( Height = GetStringHeight () ) != CurrHeight )
    {
        CurrHeight = Height;
        InvalidateRect ( hWnd, NULL, TRUE );
        return ( TRUE );
    }


    switch ( Style & LVS_TYPEMASK )
    {
        case LVS_SMALLICON:
        case LVS_LIST:
            Width  = GetItemStringWidth ( ItemIndex );
            if ( Width > CurrMaxWidth )
                InvalidateRect ( hWnd, NULL, TRUE );
            else
            {
                ViewRect.top    = GetYOrigin();
                ViewRect.left   = GetXOrigin();
                ViewRect.bottom = ViewRect.top + ( ClientRect.bottom - ClientRect.top );
                ViewRect.right  = ViewRect.left + ( ClientRect.right - ClientRect.left );
                if ( pPaintRect )
                {
                    ViewRect.top    += pPaintRect->top;
                    ViewRect.left   += pPaintRect->left;
                    ViewRect.right  -= ( ViewRect.right - ViewRect.left - pPaintRect->right );
                    ViewRect.bottom -= ( ViewRect.bottom - ViewRect.top - pPaintRect->bottom );
                }
                if ( ( Style & LVS_TYPEMASK ) == LVS_SMALLICON )
                {
                    DrawRect.top    = ( ItemIndex / ItemsPerRow ) * ( Height + WLVD_VERTICALSTRINGSPACEING );
                    DrawRect.left   = ( ItemIndex % ItemsPerRow ) * ( CurrMaxWidth + WVLD_HORIZONTALSPACING );
                }
                else
                {
                    DrawRect.top    = ( ItemIndex % ItemsPerColumn ) * ( Height + WLVD_VERTICALSTRINGSPACEING );
                    DrawRect.left   = ( ItemIndex / ItemsPerColumn ) * ( CurrMaxWidth + WVLD_HORIZONTALSPACING );
                }
                DrawRect.bottom = DrawRect.top + Height;
                DrawRect.right  = DrawRect.left + CurrMaxWidth;
                if ( IntersectRect ( &IntRect, &ViewRect, &DrawRect ) )
                {
                    if ( ( hImageList = GetImageList ( ELV_IMAGELIST_SMALL ) ) &&
                        ( ( ImageIndex = pColumns->GetItemImageIndex ( 0, ItemIndex ) ) != WLV_STANDARDERROR ) )
                    {
                        ImageList_Draw ( hImageList, ImageIndex, hDC, DrawRect.left, DrawRect.top, ILD_NORMAL );
                        ImageList_GetIconSize ( hImageList, &X, &Y );
                        DrawRect.left += X;
                    }
                    DrawRect.left   += WLVD_HORIZONTALSTRINGSPACING;
                    if ( pColumns->GetItemTextLength ( 0, ItemIndex ) )
                        CWString ( pColumns->GetItemString ( 0, ItemIndex ) ).DrawFittedString ( hDC, DrawTextStyle, WLVD_HORIZONTALSTRINGSPACING, &DrawRect );
                }
            }
            break;

        case LVS_ICON:
            ViewRect.top    = GetYOrigin();
            ViewRect.left   = GetXOrigin();
            ViewRect.bottom = ViewRect.top + ( ClientRect.bottom - ClientRect.top );
            ViewRect.right  = ViewRect.left + ( ClientRect.right - ClientRect.left );
            pColumns->GetItemPosition ( 0, ItemIndex, &Point );
            DrawRect.top    = Point.y;
            DrawRect.left   = Point.x;
            DrawRect.bottom = SHRT_MAX;
            DrawRect.right  = DrawRect.left + CurrMaxIconWidth;
            if ( IntersectRect ( &IntRect, &ViewRect, &DrawRect ) )
            {
                if ( ( hImageList = GetImageList ( ELV_IMAGELIST_LARGE ) ) &&
                    ( ( ImageIndex = pColumns->GetItemImageIndex ( 0, ItemIndex ) ) != WLV_STANDARDERROR ) )
                {
                    ImageList_Draw ( hImageList, ImageIndex, hDC, DrawRect.left, DrawRect.top, ILD_NORMAL );
                    ImageList_GetIconSize ( hImageList, &X, &Y );
                    DrawRect.top += Y;
                }
                DrawRect.top += WLVD_VERTICALSTRINGSPACEING;
                if ( pColumns->GetItemTextLength ( 0, ItemIndex ) )
                    DrawText ( hDC, pColumns->GetItemString ( 0, ItemIndex ), 
                        pColumns->GetItemTextLength ( 0, ItemIndex ), 
                        &DrawRect, DT_CENTER | DT_WORDBREAK );
            }
            break;

        case LVS_REPORT:
            ViewRect.top    = GetYOrigin() + RowHeight;
            ViewRect.left   = GetXOrigin();
            ViewRect.bottom = ViewRect.top + ( ClientRect.bottom - ClientRect.top ) - RowHeight;
            ViewRect.right  = ViewRect.left + ( ClientRect.right - ClientRect.left );
            for ( ColumnIndex = 0, ColumnCount = pColumns->ColumnCount(), ColumnOffset = 0; 
                ColumnIndex < ColumnCount; ColumnIndex++ )
            {
                Width           = GetColumnWidth ( ColumnIndex );
                DrawRect.top    = RowHeight + ItemIndex * ( Height + WLVD_VERTICALSTRINGSPACEING );
                DrawRect.left   = ColumnOffset;
                DrawRect.bottom = DrawRect.top + Height;
                DrawRect.right  = DrawRect.left + Width - WLVD_HORIZONTALSTRINGSPACING; 
                ColumnOffset    += Width;
                DrawTextStyle   = DT_LEFT | DT_SINGLELINE;
                pColumn         = (*pColumns)[ ColumnIndex ];
                if ( pColumn->LVColumn.fmt & LVCFMT_CENTER )
                    DrawTextStyle |= DT_CENTER;
                else
                if ( pColumn->LVColumn.fmt & LVCFMT_RIGHT )
                    DrawTextStyle |= DT_RIGHT;
                if ( IntersectRect ( &IntRect, &ViewRect, &DrawRect ) )
                {
                    if ( ( ! ColumnIndex ) && ( hImageList = GetImageList ( ELV_IMAGELIST_SMALL ) ) &&
                        ( ( ImageIndex = pColumns->GetItemImageIndex ( 0, ItemIndex ) ) != WLV_STANDARDERROR ) )
                    {
                        ImageList_Draw ( hImageList, ImageIndex, hDC, DrawRect.left, DrawRect.top, ILD_NORMAL );
                        ImageList_GetIconSize ( hImageList, &X, &Y );
                        DrawRect.left += X;
                    }
                    DrawRect.left += WLVD_HORIZONTALSTRINGSPACING;
                    if ( pColumns->GetItemTextLength ( 0, ItemIndex ) )
                        CWString ( pColumns->GetItemString ( ColumnIndex, ItemIndex ) ).DrawFittedString ( hDC, DrawTextStyle, WLVD_HORIZONTALSTRINGSPACING, &DrawRect );
                }
            }
            break;
    }

    SetBkColor ( hDC, BackgroundColor );

    ReleaseDC ( hWnd, hDC );

    return ( TRUE );

}
/*--- OnWM_PAINT - Paint the List View Control -------------------------------*/
void CWListView::OnWM_PAINT (
    LPRECT                              pPaintRect )

{

    DWORD                               ItemCount = GetItemCount();

    
    if ( ItemCount )
        RedrawItems ( 0, ItemCount - 1 );

}



void CWListView::OnWM_ERASEBKGND (
    HDC                                 hDC )

{

    RECT                                ClientRect;


    GetClientRect ( hWnd, &ClientRect );
    FillRect ( hDC, &ClientRect, hBackgroundBrush );

}



void CWListView::OnWM_SIZE ()

{

    RECT                                ClientRect;
    RECT                                HeaderWindowRect;


    GetClientRect ( hWnd, &ClientRect );
    GetWindowRect ( hHeaderWnd, &HeaderWindowRect );
    MoveWindow ( hHeaderWnd, 0, 0, ClientRect.right - ClientRect.left,
        HeaderWindowRect.bottom - HeaderWindowRect.top, FALSE );

}



void CWListView::OnWM_SETFONT (
    WPARAM                              wParam,
    LPARAM                              lParam )

{

    if ( hEditWnd )
        SendMessage ( hEditWnd, WM_SETFONT, wParam, lParam );
    if ( hHeaderWnd )
        SendMessage ( hHeaderWnd, WM_SETFONT, wParam, lParam );
    WDefWindowProc ( hWnd, WM_SETFONT, wParam, lParam );

}



HFONT CWListView::OnWM_GETFONT ()

{

    return ( hFont );

}



long CWListView::OnWM_NOTIFY (
    DWORD                               ControlID,
    LPNMHDR                             pNMHdr )

{

    HGDIOBJ                             hOldObject;
    int                                 DrawMode;
    HDC                                 hDC;
    RECT                                Rect;
    DWORD                               Column;

    long                                ReturnValue = 0;
    HD_NOTIFY                           *pHDNotify  = ( HD_NOTIFY* )pNMHdr;


    switch ( pNMHdr->code )
    {
        case HDN_BEGINTRACK:
            Track      = WLVD_BEGINTRACKING;
            TrackIndex = pHDNotify->iItem;
            for ( Column = 0, TrackOffset = 0; Column < ( DWORD )TrackIndex; Column++ )
                TrackOffset += GetColumnWidth ( Column );
            break;

        case HDN_ENDTRACK:
            SetColumnWidth ( TrackIndex, pHDNotify->pitem->cxy );
            InvalidateRect ( hWnd, NULL, TRUE );
            break;

        case HDN_TRACK:
            if ( hDC = GetDC ( hWnd ) )
            {
                hOldObject = SelectObject ( hDC, GetStockObject ( WHITE_PEN ) );
                DrawMode   = SetROP2 ( hDC, R2_XORPEN );
                GetClientRect ( hWnd, &Rect );
                if ( Track != WLVD_BEGINTRACKING )
                {
                    MoveToEx ( hDC, TrackOffset + Track, GetRowHeight (), NULL );
                    LineTo ( hDC, TrackOffset + Track, Rect.bottom );
                }
                Track = pHDNotify->pitem->cxy;
                MoveToEx ( hDC, TrackOffset + Track, GetRowHeight (), NULL );
                LineTo ( hDC, TrackOffset + Track, Rect.bottom );
                SetROP2 ( hDC, DrawMode );
                SelectObject ( hDC, hOldObject );
                ReleaseDC ( hWnd, hDC );
            }
            break;
    }

    return ( ReturnValue );

}



void CWListView::OnWM_HSCROLL (
    int                                 PositionCode,
    short int                           Position )

{

    RECT                                Rect;
    RECT                                ClientRect;
    int                                 XRange;
    int                                 Width;
    int                                 XExtent = GetXExtent();
    int                                 XOrigin = GetXOrigin();
    DWORD                               Style   = GetWindowLong ( hWnd, GWL_STYLE );


    GetClientRect ( hWnd, &ClientRect );
    CopyRect ( &Rect, &ClientRect );
    Width = ClientRect.right - ClientRect.left;
    if ( ( XRange = XExtent - Width ) < 0 )
        XRange = 0;

    switch ( PositionCode )
    {
        case SB_BOTTOM:
            Position = XRange;
            break;

        case SB_ENDSCROLL:
            return;
            break;

        case SB_LINELEFT:
            if ( ( Position = XOrigin - WVLD_HORIZONTALSPACING ) < 0 )
                Position = 0;
            break;

        case SB_LINERIGHT:
            if ( ( Position = XOrigin + WVLD_HORIZONTALSPACING ) > XRange )
                Position = XRange;
            break;

        case SB_PAGELEFT:
            if ( ( Position = XOrigin - Width ) < 0 )
                Position = 0;
            break;

        case SB_PAGERIGHT:
            if ( ( Position = XOrigin + Width ) > XRange )
                Position = XRange;
            break;

        case SB_THUMBPOSITION:
            break;

        case SB_THUMBTRACK:
            break;

        case SB_TOP:
            Position = 0;
            break;
    }

    if ( Position != XOrigin )
    {
        if ( ( XOrigin > Position ) && ( XOrigin - Position < Width ) )
        { /*--- Scrolling Left --- and --- Scrolling within visible area ---*/
            Rect.left  = 0;
            Rect.right = Width - ( XOrigin - Position );
            ScrollWindow ( hWnd, XOrigin - Position, 0, &Rect, &ClientRect );
            Rect.left  = Rect.right;
            Rect.right = ClientRect.right;
        }
        else
        if ( ( Position > XOrigin ) && ( Position - XOrigin < Width ) )
        { /*--- Scrolling Right --- and --- Scrolling within visible area ---*/
            Rect.left  = Position - XOrigin;
            Rect.right = ClientRect.right;
            ScrollWindow ( hWnd, XOrigin - Position, 0, &Rect, &ClientRect );
            Rect.right = Rect.left;
            Rect.left  = 0;
        }
        SetXRange ( XRange, FALSE );
        SetXOrigin ( Position, TRUE );
        InvalidateRect ( hWnd, &Rect, TRUE );
        UpdateWindow ( hWnd );
    }

}



void CWListView::OnWM_VSCROLL (
    int                                 PositionCode,
    short int                           Position )

{

    RECT                                Rect;
    RECT                                ClientRect;
    int                                 YRange;
    int                                 Height;
    int                                 RowsPerPage;
    int                                 YExtent   = GetYExtent();
    int                                 YOrigin   = GetYOrigin();
    int                                 RowHeight = ( int )GetRowHeight();
    DWORD                               Style     = GetWindowLong ( hWnd, GWL_STYLE );


    GetClientRect ( hWnd, &ClientRect );
    if ( ( Style & LVS_TYPEMASK ) == LVS_REPORT )
        ClientRect.top += RowHeight;
    CopyRect ( &Rect, &ClientRect );
    Height = ClientRect.bottom - ClientRect.top;
    RowsPerPage = round ( ( float )Height / ( float )RowHeight );
    if ( ( YRange = YExtent - RowsPerPage * RowHeight ) < 0 )
        YRange = 0;

    switch ( PositionCode )
    {
        case SB_BOTTOM:
            Position = YRange;
            break;

        case SB_ENDSCROLL:
            return;
            break;

        case SB_LINEDOWN:
            if ( ( Position = YOrigin + ( int )RowHeight ) > YRange )
                Position = YRange;
            break;

        case SB_LINEUP:
            if ( ( Position = YOrigin - ( int )RowHeight ) < 0 )
                Position = 0;
            break;

        case SB_PAGEUP:
            if ( ( Position = YOrigin - Height ) < 0 )
                Position = 0;
            break;

        case SB_PAGEDOWN:
            if ( ( Position = YOrigin + Height ) > YRange )
                Position = YRange;
            break;

        case SB_THUMBPOSITION:
            break;

        case SB_THUMBTRACK:
            break;

        case SB_TOP:
            Position = 0;
            break;
    }

    if ( Position != YOrigin )
    {
        if ( ( YOrigin > Position ) && ( YOrigin - Position < Height ) )
        { /*--- Scrolling up --- and --- Scrolling within visible area ---*/
            Rect.top     = 0;
            Rect.bottom  = Height - ( YOrigin - Position );
            ScrollWindow ( hWnd, 0, YOrigin - Position, &Rect, &ClientRect );
            Rect.top    = Rect.bottom;
            Rect.bottom = ClientRect.bottom;
        }
        else
        if ( ( Position > YOrigin ) && ( Position - YOrigin < Height ) )
        { /*--- Scrolling bottom --- and --- Scrolling within visible area ---*/
            Rect.top    = Position - YOrigin;
            Rect.bottom = ClientRect.bottom;
            ScrollWindow ( hWnd, 0, YOrigin - Position, &Rect, &ClientRect );
            Rect.bottom = Rect.top;
            Rect.top    = 0;
        }
        SetYRange ( YRange, FALSE );
        SetYOrigin ( Position, TRUE );
        InvalidateRect ( hWnd, &Rect, TRUE );
        UpdateWindow ( hWnd );
    }

}

DWORD CWListView::GetStringHeight ()

{

    HDC                                 hDC;
    DWORD                               Height;
    RECT                                Rect;


    if ( ! ( hDC = GetDC ( hWnd ) ) )
        Throw ( CatchBuffer, WERR_GETDC );

    memset ( &Rect, 0, sizeof ( RECT ) );
    Height = DrawText ( hDC, "Testing", 7, &Rect, DT_CALCRECT );
    if ( Height != ( DWORD )( Rect.bottom - Rect.top ) )
        Height = Rect.bottom - Rect.top;

    ReleaseDC ( hWnd, hDC );

    return ( Height ? Height : 1 );

}



DWORD CWListView::GetItemStringWidth (
    DWORD                               ItemIndex )

{

    LV_ITEM                             LVItem;
    int                                 Length;
    DWORD                               Width;

    char                                *pString = NULL;


    if ( ! ( Length = pColumns->GetItemTextLength ( 0, ItemIndex ) ) )
        return ( 0 );

    LVItem.iSubItem = 0;
    LVItem.cchTextMax = Length + 1;

    if ( ! ( pString = new char [ Length + 1 ] ) )
        Throw ( CatchBuffer, WERR_ALLOCATION );

    LVItem.pszText  = pString;
    GetItemText ( ItemIndex, &LVItem );

    Width = ( DWORD )GetStringWidth ( LVItem.pszText );

    delete [] pString;

    return ( Width );

}



DWORD CWListView::GetItemsPerRow ()

{

    RECT                                Rect;

    DWORD                               ItemsPerRow = 1;
    DWORD                               Style       = GetWindowLong ( hWnd, GWL_STYLE );


    GetClientRect ( hWnd, &Rect );
    if ( ! ( ItemsPerRow = ( Rect.right - Rect.left - GetSystemMetrics ( SM_CXVSCROLL ) ) / 
        ( CurrMaxWidth ) ) )
        ItemsPerRow = 1;

    return ( ItemsPerRow );

}



DWORD CWListView::GetRowHeight ()

{

    HIMAGELIST                          hImageList;
    int                                 X;
    int                                 Y;

    DWORD                               RowHeight = 0;


    RowHeight = ( DWORD )( GetStringHeight () +
                            WLVD_VERTICALSTRINGSPACEING );

    if ( hImageList = GetImageList ( ELV_IMAGELIST_SMALL ) )
    {
        ImageList_GetIconSize ( hImageList, &X, &Y );
        if ( Y > ( int )RowHeight )
            RowHeight = Y;
    }

    return ( RowHeight );

}



DWORD CWListView::GetItemsPerColumn ()

{

    RECT                                Rect;
    DWORD                               Height;

    DWORD                               ItemsPerColumn = 1;
    DWORD                               Style          = GetWindowLong ( hWnd, GWL_STYLE );


    GetClientRect ( hWnd, &Rect );

    Height = Rect.bottom - Rect.top;

    switch ( Style & LVS_TYPEMASK )
    {
        case LVS_LIST:
            if ( ! ( Style & WS_HSCROLL ) )
                Height -= GetSystemMetrics ( SM_CYHSCROLL );
            break;

        case LVS_REPORT:
            GetWindowRect ( hHeaderWnd, &Rect );
            Height -= ( Rect.bottom - Rect.top );
            break;
    }
    if ( ! ( ItemsPerColumn = ( DWORD )( Height / ( GetStringHeight () + WLVD_VERTICALSTRINGSPACEING ) ) ) )
        ItemsPerColumn = 1;
    else
    if ( ItemsPerColumn * ( GetStringHeight() + WLVD_VERTICALSTRINGSPACEING ) + GetStringHeight () <= Height )
        ItemsPerColumn++;

    return ( ItemsPerColumn );

}



void CWListView::CalcIconSizes ()

{

    RECT                                CalcRect;
    HIMAGELIST                          hImageList;
    int                                 X;
    int                                 Y;
    HDC                                 hDC;
    DWORD                               ItemIndex;

    DWORD                               ItemCount = GetItemCount();


    if ( ! ( hDC = GetDC ( hWnd ) ) )
        Throw ( CatchBuffer, WERR_GETDC );

    CurrMaxIconHeight =
    CurrMaxIconWidth  = 0;

    for ( ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++ )
    {
        memset ( &CalcRect, 0, sizeof ( CalcRect ) );
        DrawText ( hDC, pColumns->GetItemString ( 0, ItemIndex ), 
            pColumns->GetItemTextLength ( 0, ItemIndex ), &CalcRect,
            DT_CENTER | DT_CALCRECT );
        if ( CalcRect.right - CalcRect.left > ( int )CurrMaxIconWidth )
            CurrMaxIconWidth = CalcRect.right - CalcRect.left;
        if ( CalcRect.bottom - CalcRect.top > ( int )CurrMaxIconHeight )
            CurrMaxIconHeight = CalcRect.bottom - CalcRect.top;
    }
    CurrMaxIconWidth  += WVLD_HORIZONTALSPACING;
    CurrMaxIconHeight += WLVD_VERTICALSTRINGSPACEING;
    if ( hImageList = GetImageList ( ELV_IMAGELIST_LARGE ) )
    {
        ImageList_GetIconSize ( hImageList, &X, &Y );
        if ( ( DWORD )X + WVLD_HORIZONTALSPACING > CurrMaxIconWidth )
            CurrMaxIconWidth = ( DWORD )X + WVLD_HORIZONTALSPACING;
        CurrMaxIconHeight += Y + WLVD_VERTICALSTRINGSPACEING;
    }

    ReleaseDC ( hWnd, hDC );

}


void CWListView::LVColumnToHDItem (
    LV_COLUMN                           *pLVColumn,
    HD_ITEM                             *pHDItem )

{

    memset ( pHDItem, 0, sizeof ( HD_ITEM ) );
    if ( pLVColumn->mask & LVCF_FMT )
    {
        pHDItem->mask |= HDI_FORMAT;
        switch ( pLVColumn->fmt )
        {
            case LVCFMT_LEFT:
                pHDItem->fmt |= HDF_LEFT;
                break;

            case LVCFMT_RIGHT:
                pHDItem->fmt |= HDF_RIGHT;
                break;

            case LVCFMT_CENTER:
                pHDItem->fmt |= HDF_CENTER;
                break;
        }
    }
    if ( pLVColumn->mask & LVCF_WIDTH )
    {
        pHDItem->mask |= HDI_WIDTH;
        pHDItem->cxy  = pLVColumn->cx;
    }
    if ( pLVColumn->mask & LVCF_TEXT )
    {
        pHDItem->mask       |= HDI_TEXT;
        pHDItem->fmt        |= HDF_STRING;
        pHDItem->cchTextMax = pLVColumn->cchTextMax;
        pHDItem->pszText    = pLVColumn->pszText;
    }

}



void CWListView::SetFont ()

{
#ifdef LATER

    struct
    {
        short                           lfHeight;
        short                           lfWidth;
        short                           lfEscapement;
        short                           lfOrientation;
        short                           lfWeight;
        BYTE                            lfItalic;
        BYTE                            lfUnderline;
        BYTE                            lfStrikeOut;
        BYTE                            lfCharSet;
        BYTE                            lfOutPrecision;
        BYTE                            lfClipPrecision;
        BYTE                            lfQuality;
        BYTE                            lfPitchAndFamily;
        char                            lfFaceName [ LF_FACESIZE ];
    }
                                        LogFont16;

    DWORD                               LogFont16Size = sizeof ( LogFont16 );
    LOGFONT                             LogFont;
    HFONT                               hNewFont;
    DWORD                               Type = REG_BINARY;
    HKEY                                hSubKey;
    HDC                                 hDC = NULL;

    
    if ( ( ERROR_SUCCESS == RegOpenKey ( HKEY_CURRENT_USER, 
                                        "Control Panel\\Desktop\\WindowMetrics", 
                                        &hSubKey ) ) &&
        ( ERROR_SUCCESS == RegQueryValueEx ( hSubKey, "IconFont", NULL, &Type, 
                                            ( LPBYTE )&LogFont16, 
                                            &LogFont16Size ) ) &&
        ( hDC = GetDC ( hWnd ) ) )
    {
        memset ( &LogFont, 0, sizeof ( LOGFONT ) );
        LogFont.lfHeight         = (-1) * MulDiv ( LogFont16.lfHeight, 
                                                    GetDeviceCaps ( hDC, LOGPIXELSY ), 72 );
        LogFont.lfWidth          = LogFont16.lfWidth;
        LogFont.lfEscapement     = LogFont16.lfEscapement;
        LogFont.lfOrientation    = LogFont16.lfOrientation;
        LogFont.lfWeight         = LogFont16.lfWeight;
        LogFont.lfItalic         = LogFont16.lfItalic;
        LogFont.lfUnderline      = LogFont16.lfUnderline;
        LogFont.lfStrikeOut      = LogFont16.lfStrikeOut;
        LogFont.lfCharSet        = LogFont16.lfCharSet;
        LogFont.lfOutPrecision   = LogFont16.lfOutPrecision;
        LogFont.lfClipPrecision  = LogFont16.lfClipPrecision;
        LogFont.lfQuality        = LogFont16.lfQuality;
        LogFont.lfPitchAndFamily = LogFont16.lfPitchAndFamily;
        strncpy ( LogFont.lfFaceName, LogFont16.lfFaceName, LF_FACESIZE );
        if ( hNewFont = CreateFontIndirect ( &LogFont ) )
        {
            if ( hFont )
            {
                DeleteObject ( hFont );
                hFont = ( HFONT )NULL;
            }
            hFont = hNewFont;
            SendMessage ( hWnd, WM_SETFONT, ( WPARAM )hFont, MAKELPARAM ( TRUE, 0 ) );
        }
    }

    if ( hDC )
        ReleaseDC ( hWnd, hDC );
#endif

}



void WINAPI WInitListView()
{
    if ( ! pWListView )
    {
        pWListView = new CWLListView;
    }
}
