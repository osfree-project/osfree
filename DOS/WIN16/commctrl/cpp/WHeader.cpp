/*******************************************************************************

 	@(#)WHeader.cpp	1.7 *	Header Control
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
/*******************************************************************************
*
*	HD_LAYOUT has not been implemented.	
*	
*
*******************************************************************************/





/*--- Includes  --------------------------------------------------------------*/
#include "WCommCtrl.h"
#include "CommCtrlRC.h"
#include "WHeader.h"
#include "WAHDItem.h"
#include "WString.h"



/*--- Defines ----------------------------------------------------------------*/
#define WHD_HORIZONTALBORDER            6

#define WHD_BUTTONNORMAL				0
#define WHD_BUTTONDOWN                  1
#define WHD_NOINDEX                     0xFFFFFFFF
#define WHD_NOOFFSET                    0xFFFFFFFF
#define WHD_NOX                         0xFFFFFFFF

#define XYINRECT( X, Y, lpRect )        ( ( (lpRect)->left <= X ) && ( X <= (lpRect)->right ) &&\
                              	            ( (lpRect)->top <= Y ) && ( Y <= (lpRect)->bottom ) )




/*--- Structures -------------------------------------------------------------*/

typedef struct WHS_LOCAL
{
    LPSTR                               pHeaderClass;
    HCURSOR                             hDivCursor;
} WHS_LOCAL;



/*----- Classes -----*/

class CWLHeader 
{

    public:

        CWLHeader ();
        ~CWLHeader ();


};

/*----- Static Locals --------------------------------------------------------*/

static WHS_LOCAL                        WHSLocal;
static CWLHeader                        *pWHeader = NULL;


/*--- WHCDefWindowProc - Handle Window Messages ------------------------------*/
extern "C" LRESULT EXPORT CALLBACK
WHCDefWindowProc 
(
	HWND                                hWnd,
	UINT                                Message,
	WPARAM                              wParam,
	LPARAM                              lParam
)

{

    CWHeader                            *pHeader;
	PAINTSTRUCT                         PaintStruct;
	int				ExceptionError;

    LRESULT                             ErrorCode      = 0;
    int *				SaveCatchBuffer;


    if ( ( ! ( pHeader = ( CWHeader*)GetWindowLong ( hWnd, 0 ) ) ) &&
        ( Message != WM_CREATE ) )
        ErrorCode = DefWindowProc ( hWnd, Message, wParam, lParam );
    else
        switch ( Message )
	    {
		    default:
			    ErrorCode = DefWindowProc ( hWnd, Message, wParam, lParam );
			    break;

		    case WM_CREATE:
#ifdef WEXCEPTIONS
                try
                {
                    pHeader = new CWHeader ( hWnd, ( LPCREATESTRUCT )lParam );
                    SetWindowLong ( hWnd, 0, ( long )pHeader );
                }
                catch ( int ExceptionError )
                {
                    ErrorCode = ExceptionError;
                }
#else
	SaveCatchBuffer = CatchBuffer;
        CatchBuffer = (int *)WinMalloc( sizeof ( CATCHBUF ) );
        ExceptionError = Catch ( CatchBuffer );
        if ( ExceptionError == 0 )
	{
		pHeader = new CWHeader ( hWnd, ( LPCREATESTRUCT )lParam );
		SetWindowLong ( hWnd, 0, ( long )pHeader );
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
                delete ( pHeader );
                SetWindowLong ( hWnd, 0, 0 );
                break;

		    case WM_PAINT:
			    if ( wParam = ( WPARAM )BeginPaint ( hWnd, &PaintStruct ) )
			    {
                    ErrorCode = pHeader->Paint ( ( HDC )wParam );
				    EndPaint ( hWnd, &PaintStruct );
			    }
			    break;

		    case WM_LBUTTONDOWN:
                ErrorCode = pHeader->OnLButtonDown ( ( UINT )wParam, LOWORD ( lParam ), HIWORD ( lParam ) );
			    break;

		    case WM_MOUSEMOVE:
			    ErrorCode = pHeader->OnMouseMove ( ( UINT )wParam, LOWORD ( lParam ), HIWORD ( lParam ) );
			    break;

		    case WM_LBUTTONUP:
			    ErrorCode = pHeader->OnLButtonUp ( ( UINT )wParam, LOWORD ( lParam ), HIWORD ( lParam ) );
			    break;

		    case HDM_DELETEITEM:
			    ErrorCode = pHeader->DeleteItem ( ( DWORD )wParam );
			    break;

		    case HDM_GETITEM:
			    ErrorCode = pHeader->GetItem ( ( DWORD )wParam, ( HD_ITEM* ) lParam );
			    break;

		    case HDM_GETITEMCOUNT:
			    ErrorCode = pHeader->GetItemCount ();
			    break;

		    case HDM_INSERTITEM:
			    ErrorCode = pHeader->InsertItem ( wParam, ( HD_ITEM* ) lParam );
			    break;

		    case HDM_LAYOUT:
			    ErrorCode = pHeader->Layout ( ( HD_LAYOUT* ) lParam );
			    break;

		    case HDM_SETITEM:
			    ErrorCode = pHeader->SetItem ( ( DWORD )wParam, ( HD_ITEM* ) lParam );
			    break;
	    }

	return ( ErrorCode );

}

/*===== CWLHeader ============================================================*/

CWLHeader::CWLHeader ()

{

    WNDCLASS                            WndClass;


    memset ( &WHSLocal, 0, sizeof ( WHS_LOCAL ) );
    WHSLocal.pHeaderClass = WHD_CLASSNAME;
    if ( ! ( WHSLocal.hDivCursor = LoadCursor ( pWCommCtrl->hInstance(), MAKEINTRESOURCE ( IDC_DIVIDOR ) ) ) )
        Throw ( CatchBuffer, WERR_LOADRESOURCE );
    /*------------------------------------------------------
		Create a window class which any application can
		use with a grey background as the default.
	------------------------------------------------------*/
	memset ( &WndClass, 0, sizeof ( WndClass ) );
	WndClass.style         = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc   = WHCDefWindowProc;
	WndClass.cbClsExtra    = 0;
	WndClass.cbWndExtra    = sizeof ( CWHeader* );
	WndClass.hInstance     = pWCommCtrl->hInstance();
	WndClass.hIcon         = ( HICON )NULL;
	WndClass.hbrBackground = GetStockObject ( LTGRAY_BRUSH );
	WndClass.lpszMenuName  = ( LPCTSTR )NULL;
	WndClass.lpszClassName = WHSLocal.pHeaderClass;
	if ( ! RegisterClass ( &WndClass ) )
        Throw ( CatchBuffer, WERR_REGISTERCLASS );

}



CWLHeader::~CWLHeader ()

{

    WNDCLASS                            WndClass;


    if ( GetClassInfo ( pWCommCtrl->hInstance(), WHSLocal.pHeaderClass, &WndClass ) )
        UnregisterClass ( WHSLocal.pHeaderClass, pWCommCtrl->hInstance() );

}


/*--- Create a Header Window -------------------------------------------------*/
CWHeader::CWHeader (
    HWND                                hWindow,
    LPCREATESTRUCT                      pCreateStruct )

{

    hWnd = hWindow;
	/*--- Save the window style ---*/
    Style = pCreateStruct->style;
	
	/*--- Save the control id which is passed in as the menu handle ---*/
    ID = ( int )pCreateStruct->hMenu;

    /*--- Initialize other members ---*/
    Captured = FALSE;
    Index    = WHD_NOINDEX;
    Offset   = WHD_NOOFFSET;
    Down     = FALSE;
    OldX     = WHD_NOX;

    /*--- Create Item List ---*/
    if ( ! ( pHDItems = new CWAHDItem () ) )
        Throw ( CatchBuffer, WERR_ALLOCATION );

}



/*--- Destroy a Header Window ------------------------------------------------*/
CWHeader::~CWHeader ()

{

    DWORD                               ItemIndex;
    DWORD                               ItemCount;
    CWHDItem                            *pItem;


    if ( pHDItems )
    {
        for ( ItemIndex = 0, ItemCount = pHDItems->Count(); ItemIndex < ItemCount; ItemIndex++ )
        {
            pItem = (*pHDItems)[ ItemIndex ];
            delete ( pItem );
        }
        delete ( pHDItems );
    }

}
/*--- Draw the bitmap using the format in the rectangle ----------------------*/
void CWHeader::DrawBitmap (
    HDC                                 hDC,
    HBITMAP                             hBitmap,
    LPRECT                              pRect,
    int                                 Format )

{

	BITMAP                   Bitmap;
	HGDIOBJ                  hOldObject;
	HDC                      hMemoryDC = ( HDC )NULL;

	int                      ErrorCode = 0;


	if ( hMemoryDC = CreateCompatibleDC ( hDC ) )
	{
		GetObject ( hBitmap, sizeof ( BITMAP ), &Bitmap );
		hOldObject = SelectObject ( hMemoryDC, hBitmap );
		if ( Format & HDF_CENTER )
			BitBlt ( hDC, 
			         pRect->left + ( pRect->right - pRect->left - Bitmap.bmWidth ) / 2, 
			         pRect->top + ( pRect->bottom - pRect->top - Bitmap.bmHeight ) / 2,
			         Bitmap.bmWidth, Bitmap.bmHeight, hMemoryDC, 0, 0, SRCCOPY );
		else
		if ( Format & HDF_RIGHT )
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

/*--- Paint an Item within the entire control --------------------------------*/
LRESULT CWHeader::PaintItem (
    DWORD                               ItemIndex,
    HDC                                 hDC,
    DWORD                               *pOffset )

{

    CWHDItem                            *pHDItem;
    RECT                                ClientRect;
	RECT                                DrawRect;
	RECT                                CalcRect;
	HGDIOBJ                             hOldObject;
	UINT                                NewOffset;
	UINT                                DTFormat;
    	DWORD                               ItemOffset;
	TEXTMETRIC                          TextMetric;
	BOOL                                Drawn = FALSE;
	int                                 PadFactor = 2 * WHD_HORIZONTALBORDER;
	LPSTR                               pDisplayString = ( LPSTR )NULL;
	HPEN                                hGrayPen = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNSHADOW ) );
    	HDC                                 hUseDC   = ( HDC )NULL;

	LRESULT                             ErrorCode   = 0;


    hUseDC = ( hDC ? hDC : GetDC ( hWnd ) );
    pHDItem = (*pHDItems) [ ItemIndex ];
    /*--- Figure out the rectangle for the current item ---*/
	GetClientRect ( hWnd, &ClientRect );
    ItemOffset      = ( pOffset ? *pOffset : Offset );
    NewOffset       = ItemOffset + pHDItem->HDItem.cxy;
	DrawRect.top    = ClientRect.top;
	DrawRect.bottom = ClientRect.bottom;
	DrawRect.left   = ItemOffset + 1;
	DrawRect.right  = NewOffset;
	/*--- Clean off the area for the current item ---*/
	FillRect ( hUseDC, &DrawRect, GetStockObject ( LTGRAY_BRUSH ) );

	CopyRect ( &DrawRect, &ClientRect );
	DrawRect.right = NewOffset;
	DrawRect.left  = ItemOffset;
	/*--- If there is text to be drawn ---*/
	if ( ( pHDItem->HDItem.fmt & HDF_STRING ) && ( pHDItem->HDItem.pszText ) )
	{
		GetTextMetrics ( hUseDC, &TextMetric );
		DTFormat = DT_VCENTER;
		DrawRect.top   = ( ClientRect.bottom - ClientRect.top ) / 2 - 
		                 TextMetric.tmHeight / 2;
		DrawRect.bottom = ( ClientRect.bottom - ClientRect.top ) / 2 + 
		                  TextMetric.tmHeight / 2;
		if ( Down )
		{ /*--- If the button is down the text should appear down and right by 1 pixel ---*/
			DrawRect.top++;
			DrawRect.bottom++;
			DrawRect.left++;
			DrawRect.right++;
		}
		if ( pHDItem->HDItem.fmt & HDF_CENTER )
		{
			DTFormat |= DT_CENTER;
		}
		else
		if ( pHDItem->HDItem.fmt & HDF_RIGHT )
		{ /*--- if the text is right justified then there is a minimum space to 
			    the right that is required ---*/
			DTFormat       |= DT_RIGHT;
			DrawRect.right -= WHD_HORIZONTALBORDER;
			PadFactor      -= WHD_HORIZONTALBORDER;
		}
		else
		{ /*--- if the text is left justified then there is a minimum space to the 
			    left that is required ---*/
			DTFormat      |= DT_LEFT;
			DrawRect.left += WHD_HORIZONTALBORDER;
			PadFactor     -= WHD_HORIZONTALBORDER;
		}
		SetBkMode ( hUseDC, TRANSPARENT );
		CopyRect ( &CalcRect, &DrawRect );
        CWString ( pHDItem->HDItem.pszText ).DrawFittedString ( hUseDC, DTFormat, PadFactor, &DrawRect );
	}
	else
	if ( ( pHDItem->HDItem.fmt & HDF_BITMAP ) && ( pHDItem->HDItem.hbm ) )
	{ /*--- Draw the bitmap ---*/
		if ( Down )
		{ /*--- If the button is down the bitmap should appear down and right by 1 
		        pixel ---*/
			DrawRect.top++;
			DrawRect.bottom++;
			DrawRect.left++;
			DrawRect.right++;
		}
		DrawBitmap ( hUseDC, pHDItem->HDItem.hbm, &DrawRect, pHDItem->HDItem.fmt );
	}

	if ( Style & HDS_BUTTONS )
	{ /*--- if the button style is being used then make the item look like a button ---*/
		if ( Down )
		{ /*--- if the button is pressed down then draw it depressed ---*/
			MoveToEx ( hUseDC, ItemOffset, ClientRect.top, ( LPPOINT )NULL );
			hOldObject = SelectObject ( hUseDC, hGrayPen );
			LineTo ( hUseDC, NewOffset - 1, ClientRect.top );
			LineTo ( hUseDC, NewOffset - 1, ClientRect.bottom - 1 );
			LineTo ( hUseDC, ItemOffset, ClientRect.bottom - 1 );
			LineTo ( hUseDC, ItemOffset, ClientRect.top );
		}
		else
		{ /*--- the button is up so draw it raised ---*/
			MoveToEx ( hUseDC, ItemOffset, ClientRect.bottom - 2, ( LPPOINT )NULL );
			hOldObject = SelectObject ( hUseDC, GetStockObject ( WHITE_PEN ) );
			LineTo ( hUseDC, ItemOffset, ClientRect.top );
			LineTo ( hUseDC, NewOffset - 1, ClientRect.top );
			
			MoveToEx ( hUseDC, NewOffset - 2, ClientRect.top + 1, ( LPPOINT )NULL );
			SelectObject ( hUseDC, hGrayPen );
			LineTo ( hUseDC, NewOffset - 2, ClientRect.bottom - 2 );
			LineTo ( hUseDC, ItemOffset, ClientRect.bottom - 2 );

			MoveToEx ( hUseDC, NewOffset - 1, ClientRect.top, ( LPPOINT )NULL );
			SelectObject ( hUseDC, GetStockObject ( BLACK_PEN ) );
			LineTo ( hUseDC, NewOffset - 1, ClientRect.bottom - 1 );
			LineTo ( hUseDC, ItemOffset, ClientRect.bottom - 1 );
		}

		SelectObject ( hUseDC, hOldObject );		
	}
	else
	{ /*--- the normal style is being used so draw it as such ---*/
		MoveToEx ( hUseDC, ItemOffset, ClientRect.bottom - 1, ( LPPOINT )NULL );
		hOldObject = SelectObject ( hUseDC, GetStockObject ( WHITE_PEN ) );
		LineTo ( hUseDC, NewOffset - 1, ClientRect.bottom - 1 );
		LineTo ( hUseDC, NewOffset - 1, ClientRect.top - 1 );
		MoveToEx ( hUseDC, ItemOffset, ClientRect.bottom - 2, ( LPPOINT )NULL );
		SelectObject ( hUseDC, hGrayPen );
		LineTo ( hUseDC, NewOffset - 2, ClientRect.bottom - 2 );
		LineTo ( hUseDC, NewOffset - 2, ClientRect.top - 1 );
		SelectObject ( hUseDC, hOldObject );
	}

    if ( pOffset )
        *pOffset = NewOffset;

    if ( ! hDC )
        ReleaseDC ( hWnd, hUseDC );
	
	if ( hGrayPen )
		DeleteObject ( hGrayPen );

	return ( ErrorCode );

}
/*--- WHCPaint - Paint the control -------------------------------------------*/
LRESULT CWHeader::Paint (
    HDC                                 hDC )

{

	UINT                                ItemCount;
	UINT                                ItemIndex;
	RECT                                ClientRect;
	HDC                                 hWindowDC  = ( HDC )NULL;
	DWORD                               ItemOffset = 0;

	LRESULT                             ErrorCode  = 0;


    GetClientRect ( hWnd, &ClientRect );
    FillRect ( hDC, &ClientRect, GetStockObject ( LTGRAY_BRUSH ) );
    if ( hWindowDC = GetDC ( hWnd ) )
    {
        for ( ItemIndex = 0, ItemCount = pHDItems->Count(); ItemIndex < ItemCount; ItemIndex++ )
            PaintItem ( ItemIndex, hWindowDC, &ItemOffset );
        ReleaseDC ( hWnd, hWindowDC );
    }

	return ( ErrorCode );

}
/*--- Delete an Item ---------------------------------------------------------*/
LRESULT CWHeader::DeleteItem (
	DWORD                               ItemIndex )

{
	
	LRESULT                             ErrorCode = 0;
	int				ExceptionError;
	int *			SaveCatchBuffer;


#ifdef WEXCEPTIONS
    try
    {
        pHDItems->Delete ( ItemIndex );
        InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE );
    }
    catch ( int ExceptionError )
    {
        ErrorCode = ( LRESULT ) ExceptionError;
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof ( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
	{
		pHDItems->Delete ( ItemIndex );
		InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE );
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;	
#endif

    return ( ErrorCode );

}

/*--- Get an Item's Data -----------------------------------------------------*/
LRESULT CWHeader::GetItem (
	DWORD                               ItemIndex,
	HD_ITEM                             *pItem )

{

    CWHDItem                            *pHDItem;
	
	LRESULT                             ErrorCode = 0;
	int				    ExceptionError;
	int *				SaveCatchBuffer;


#ifdef WEXCEPTIONS
    try
    {
        pHDItem = (*pHDItems) [ ItemIndex ];
        pHDItem->Get ( pItem );
    }
    catch ( int ExceptionError )
    {
        ErrorCode = ( LRESULT ) ExceptionError;
    }
#else
	SaveCatchBuffer = CatchBuffer;
 	CatchBuffer = (int *)WinMalloc( sizeof ( CATCHBUF ) ); 
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
	{
		pHDItem = (*pHDItems) [ ItemIndex ];
		pHDItem->Get ( pItem );
	}
	else
	{
		ErrorCode = ( LRESULT ) ExceptionError;
	}
	WinFree ( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif

    return ( ErrorCode );

}

/*--- Get the number of items ------------------------------------------------*/
LRESULT CWHeader::GetItemCount ( )

{
	
	LRESULT                             ErrorCode = 0;
	int				ExceptionError;
	int *			SaveCatchBuffer;


#ifdef WEXCEPTIONS
    try
    {
        ErrorCode = pHDItems->Count();
    }
    catch ( int ExceptionError )
    {
        ErrorCode = ( LRESULT ) ExceptionError;
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof ( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0)
	{
		ErrorCode = pHDItems->Count();
	}
	else
	{
		ErrorCode = ( LRESULT ) ExceptionError;
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif

    return ( ErrorCode );

}

/*--- Insert a new item into the list ----------------------------------------*/
LRESULT CWHeader::InsertItem (
	DWORD                               ItemIndex,
	HD_ITEM                             *pItem )

{

    CWHDItem                            *pHDItem;
	
	LRESULT                             ErrorCode = 0;
	int				ExceptionError;
	int *			SaveCatchBuffer;


#ifdef WEXCEPTIONS
    try
    {
        pHDItem = new CWHDItem ( pItem );
        pHDItems->Insert ( ItemIndex, pHDItem );
        InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE );
    }
    catch ( int ExceptionError )
    {
        ExceptionError = 0; // this is here simply to avoid the warning
        ErrorCode = ( LRESULT )ErrorCode;
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof ( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
	{
		pHDItem = new CWHDItem ( pItem );
		pHDItems->Insert ( ItemIndex, pHDItem );
		InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE );
	}
	else
	{
		ExceptionError = 0;
		ErrorCode = ( LRESULT )ErrorCode;
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif

    return ( ErrorCode );

}

/*----------------------------------------------------------------------------*/
LRESULT CWHeader::Layout (
	HD_LAYOUT                           *lpLayout )

{
	
	LRESULT                             ErrorCode = 0;

    return ( ErrorCode );

}

/*--- Set the data of an item ------------------------------------------------*/
LRESULT CWHeader::SetItem (
	DWORD                               ItemIndex,
	HD_ITEM                             *pItem )

{

    HD_NOTIFY                           HDNotify;
    CWHDItem                            *pHDItem;
	
    LRESULT                             ErrorCode = 0;
	int				ExceptionError;
	int *			SaveCatchBuffer;


#ifdef WEXCEPTIONS
    try
    {
        pHDItem = (*pHDItems) [ ItemIndex ];
		memset ( &HDNotify, 0, sizeof ( HDNotify ) );
		HDNotify.hdr.hwndFrom = hWnd;
		HDNotify.hdr.idFrom   = ID;
		HDNotify.hdr.code     = HDN_ITEMCHANGING;
		HDNotify.iItem        = ItemIndex;
		HDNotify.iButton      = 0; /*--- There is no define for this and 0 means 
			                             left button ---*/
		HDNotify.pitem        = &pHDItem->HDItem;
        if ( ! Notify ( &HDNotify ) )
        {
            pHDItem->Assign ( pItem );
            InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE );
            HDNotify.hdr.code = HDN_ITEMCHANGED;
            Notify ( &HDNotify );
        }
    }
    catch ( int ExceptionError )
    {
        ErrorCode = ( LRESULT ) ExceptionError;
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof ( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
	{
		pHDItem = (*pHDItems) [ ItemIndex ];	
		memset ( &HDNotify, 0, sizeof ( HDNotify ) );
		HDNotify.hdr.hwndFrom = hWnd;
		HDNotify.hdr.idFrom   = ID;
		HDNotify.hdr.code     = HDN_ITEMCHANGING;
		HDNotify.iItem        = ItemIndex;
		HDNotify.iButton      = 0;
		HDNotify.pitem        = &pHDItem->HDItem;
		if ( ! Notify ( &HDNotify ) )
		{
			pHDItem->Assign ( pItem );
			InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE );
			HDNotify.hdr.code = HDN_ITEMCHANGED;
			Notify ( &HDNotify );
		}
	}
	else
	{
		ErrorCode = ( LRESULT ) ExceptionError;
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif

    return ( ErrorCode );

}

/*--- WHCLMouseDown - Handle the Left Mouse Button down ----------------------*/
LRESULT CWHeader::OnLButtonDown (
	UINT                                KeyStatus,
	int                                 X,
	int                                 Y )

{

	DWORD                               ItemCount;
	HD_NOTIFY                           HDNotify;
    BOOL                                Capture;
    CWHDItem                            *pHDItem;

    DWORD                               x = ( DWORD )X;
    DWORD                               y = ( DWORD )Y;

	LRESULT                             ErrorCode = 0;


    Index      = 0;
    ItemCount  = pHDItems->Count();
    Offset     = 0;
    while ( ( Index < ItemCount ) && 
        ( ! ( ( Offset <= x ) && ( x <= Offset + ( (*pHDItems)[ Index ] )->HDItem.cxy ) )  ) )
    {
        Offset += ( (*pHDItems)[ Index ] )->HDItem.cxy;
        Index++;
    }
    Capture = TRUE;
    if ( ( GetCursor () == WHSLocal.hDivCursor ) && ( Index < ItemCount ) )
    {
        if ( Index + 1 < ItemCount ) 
        {
            pHDItem = (*pHDItems)[ Index + 1 ];
            if ( ! pHDItem->HDItem.cxy )
                Index++;
        }
        pHDItem = (*pHDItems)[ Index ];
		memset ( &HDNotify, 0, sizeof ( HDNotify ) );
		HDNotify.hdr.hwndFrom = hWnd;
		HDNotify.hdr.idFrom   = ID;
		HDNotify.hdr.code     = HDN_BEGINTRACK;
		HDNotify.iItem        = Index;
		HDNotify.iButton      = 0; /*--- There is no define for this and 0 means 
			                             left button ---*/
		HDNotify.pitem        = &pHDItem->HDItem;
        Capture               = ! Notify ( &HDNotify );
    }
    if ( Index >= ItemCount )
    {
        Index  = WHD_NOINDEX;
        Offset = WHD_NOOFFSET;
    }
    else
    if ( Capture )
    {
        SetCapture ( hWnd );
        Down = GetCursor () != WHSLocal.hDivCursor;
        PaintItem ( Index );
        Captured = TRUE;
    }

    return ( ErrorCode );

}	

/*--- Handle the Left Mouse Button up ----------------------------------------*/
LRESULT CWHeader::OnLButtonUp (
	UINT                                KeyStatus,
	int                                 X,
	int                                 Y )

{

	HDC                                 hDC;
	int                                 DrawMode;
	HGDIOBJ                             hOldObject;
	RECT                                ClientRect;
	HD_NOTIFY                           HDNotify;
	int                                 Oldcxy;
    CWHDItem                            *pHDItem;

    DWORD                               x = ( DWORD )X;
    DWORD                               y = ( DWORD )Y;

	int                                 ErrorCode = 0;


    Down = WHD_BUTTONNORMAL;
    if ( Captured )
    {
        ReleaseCapture ();
        Captured = FALSE;
        pHDItem  = ( Index == WHD_NOINDEX ? (*pHDItems)[ 0 ] : (*pHDItems)[ Index ] );
        memset ( &HDNotify, 0, sizeof ( HD_NOTIFY ) );
		HDNotify.hdr.hwndFrom = hWnd;
		HDNotify.hdr.idFrom   = ID;
		HDNotify.iItem        = Index;
		HDNotify.iButton      = 0; /*--- There is no define for this and 0 means 
						                left button ---*/
		HDNotify.pitem        = &pHDItem->HDItem;
        if ( ( Style & HDS_BUTTONS ) && ( GetCursor() != WHSLocal.hDivCursor ) )
        {
            Down = WHD_BUTTONNORMAL;
            PaintItem ( Index );
            HDNotify.hdr.code = HDN_ITEMCLICK;
            Notify ( &HDNotify );
        }
        else
        if ( GetCursor() == WHSLocal.hDivCursor )
        {
            HDNotify.hdr.code = HDN_ENDTRACK;
            Notify ( &HDNotify );
            GetClientRect ( hWnd, &ClientRect );
            if ( hDC = GetDC ( hWnd ) )
            {
                hOldObject = SelectObject ( hDC, GetStockObject ( WHITE_PEN ) );
                DrawMode   = SetROP2 ( hDC, R2_XORPEN );
                if ( OldX != WHD_NOX )
                {
                    MoveToEx ( hDC, OldX, ClientRect.top, ( LPPOINT )NULL );
                    LineTo ( hDC, OldX, ClientRect.bottom );
                }
                SetROP2 ( hDC, DrawMode );
                SelectObject ( hDC, hOldObject );
                HDNotify.hdr.code = HDN_ITEMCHANGING;
                Oldcxy            = pHDItem->HDItem.cxy;
                pHDItem->HDItem.cxy = ( x < Offset ? 0 : x - Offset );
                if ( Notify ( &HDNotify ) )
                    pHDItem->HDItem.cxy = Oldcxy;
                else
                {
                    Paint ( hDC );
                    HDNotify.hdr.code = HDN_ITEMCHANGED;
                    Notify ( &HDNotify );
                }
                ReleaseDC ( hWnd, hDC );
            }
        }
    }

    return ( ErrorCode );

}	

/*--- Handle the Left Mouse Button move --------------------------------------*/
LRESULT CWHeader::OnMouseMove (
	UINT                                KeyStatus,
	int                                 X,
	int                                 Y )

{

	UINT                                ItemCount;
	RECT                                ClientRect;
	int                                 DrawMode;
	HDC                                 hDC;
	HGDIOBJ                             hOldObject;
	HD_NOTIFY                           HDNotify;
    CWHDItem                            *pHDItem;
	UINT                                ItemIndex   = 0;
	int                                 ItemOffset  = 0;
	BOOL                                ShowDividor = FALSE;

	LRESULT                             ErrorCode   = 0;


    if ( Captured )
    {
        pHDItem = (*pHDItems)[ Index ];
        GetClientRect ( hWnd, &ClientRect );
        if ( ( hDC = GetDC ( hWnd ) ) )
        {
            if ( GetCursor() != WHSLocal.hDivCursor )
            {
                ClientRect.left  = Offset;
                ClientRect.right = Offset + pHDItem->HDItem.cxy;
                if ( ( XYINRECT ( X, Y, &ClientRect ) ) && ( Down == WHD_BUTTONNORMAL ) )
                {
                    Down = WHD_BUTTONDOWN;
                    ErrorCode = PaintItem ( Index, hDC );
                }
                else
                if ( ( ! XYINRECT ( X, Y, &ClientRect ) ) && ( Down == WHD_BUTTONDOWN ) )
                {
                    Down = WHD_BUTTONNORMAL;
                    ErrorCode = PaintItem ( Index, hDC );
                }
            }
            else
            {
                hOldObject = SelectObject ( hDC, GetStockObject ( WHITE_PEN ) );
                DrawMode   = SetROP2 ( hDC, R2_XORPEN );
                if ( OldX != WHD_NOX )
                {
                    MoveToEx ( hDC, OldX, ClientRect.top, ( LPPOINT )NULL );
                    LineTo ( hDC, OldX, ClientRect.bottom );
                }
                memset ( &HDNotify, 0, sizeof ( HDNotify ) );
			    HDNotify.hdr.hwndFrom = hWnd;
			    HDNotify.hdr.idFrom   = ID;
			    HDNotify.hdr.code     = HDN_TRACK;
			    HDNotify.iItem        = Index;
			    HDNotify.iButton      = 0; /*--- There is no define for this and 0 means 
							                left button ---*/
			    HDNotify.pitem        = &pHDItem->HDItem;
                if ( Notify ( &HDNotify ) )
                {
                    ReleaseCapture();
                    Captured = FALSE;
                    OldX     = WHD_NOX;
                }
                else
                {
                    MoveToEx ( hDC, X, ClientRect.top, ( LPPOINT )NULL );
                    LineTo ( hDC, X, ClientRect.bottom );
                    OldX = X;
                }
                SetROP2 ( hDC, DrawMode );
                SelectObject ( hDC, hOldObject );
            }
            ReleaseDC ( hWnd, hDC );
        }
    }
    else
    {
        ItemOffset  = 0;
        ItemIndex   = 0;
        ItemCount   = pHDItems->Count();
        ShowDividor = FALSE;
        if ( ItemCount )
            do
            {
                pHDItem = (*pHDItems) [ ItemIndex ];
                if ( ! ( ShowDividor = ( ( ( ItemOffset <= X ) &&
                                       ( X < ItemOffset + ( ItemIndex ? WHD_HORIZONTALBORDER : 0 ) ) ) ||
                                       ( ( ItemOffset + pHDItem->HDItem.cxy - WHD_HORIZONTALBORDER < X ) &&
                                       ( X < ItemOffset + pHDItem->HDItem.cxy ) ) ) ) )
                {
                    ItemOffset += pHDItem->HDItem.cxy;
                    ItemIndex++;
                }
            } while ( ( ! ShowDividor ) && ( ItemIndex < ItemCount ) );
        if ( ShowDividor )
        {
            Index = ItemIndex;
            if ( GetCursor () != WHSLocal.hDivCursor )
                SetCursor ( WHSLocal.hDivCursor );
        }
        else
        if ( GetCursor () == WHSLocal.hDivCursor )
            SetCursor ( LoadCursor ( ( HINSTANCE )NULL, IDC_ARROW ) );
    }

    return ( ErrorCode );

}	

void WINAPI WInitHeader()
{

	int				ExceptionError;
	int *			SaveCatchBuffer;


    if ( ! pWHeader )
    {
#ifdef WEXCEPTIONS
	try
	{
        	pWHeader = new CWLHeader;
	}
	catch ( int ExceptionError )
	{
		OutputDebugString ( "Could not initialize CWLHeader\n" );
	}
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
	{
		pWHeader = new CWLHeader;
	}
	else
	{
		OutputDebugString ( "Could not initialize CWLHeader\n" );
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif
    }

}
