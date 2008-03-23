/*  
	WImgList.c
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


#define _WIMAGELIST_SOURCE
#include "windows.h"
#include "WLists.h"
#define MAXOVERLAY	4
typedef struct _WIMAGE
{
	HDC			hMemoryDC;
	HDC			hMaskDC;
	int			NumImages;
	int			NumUsed;
} WIMAGE;

typedef struct TAG_WIMAGELIST
{
	HGLOBAL		hMemImageList;
	int		Height;
	int		Width;
	UINT		CreateFlags;
	int		GrowBy;
	COLORREF	BackgroundColor;
	COLORREF	ForegroundColor;
	COLORREF	MaskColor;
    	WIMAGE      	ImgList;
	HBRUSH		hBlend25Brush;
	HBRUSH		hBlend50Brush;
	int		OverlayIndices[MAXOVERLAY];
} _WIMAGELIST;


#include "WCommCtrl.h"
#include "WImgList.h"
#include "WImageIO.h"
#include "WBmp.h"


typedef struct _WDRAGIMAGE
{
	WHIMAGELIST		pImageList;
	int			ImageIndex;
	HWND			hDragWnd;
	HDC			hBkgroundDC;
	HDC			hImageDC;
	HDC			hMaskDC;
	POINT			ptPos;
	POINT			ptSize;
	POINT			ptHotSpot;
	BOOL			bShow;
} WDRAGIMAGE;


#ifndef _WINDOWS
#define SetBrushOrgEx(hdc,x,y,Old)	SetBrushOrg(hdc,x,y)
#endif
#define WIL_ADDINDEX			-1
#define ISVALIDOVERLAY(i) (i >= 1 && i <= 4)
#define GETOVERLAYINDEX(i) ( ( DrawFlags & ILD_OVERLAYMASK ) >> 8 )

static char*			szProperty = "WDragImage";

BOOL WINAPI
WImage_IRemove
( 
	WIMAGE				*Image,
	int			    	nSubImageRemoved,
	int			    	nHeight,
	int			    	nWeight,
	BOOL				bMask
);

int 
WImageList_IAddImage
( 
	WHIMAGELIST			hImageList, 
	int			    	nAddedImages,
	HBITMAP				hBitmap,
	HBITMAP				hMask
);

static BOOL 
W_IDrawMaskedImage 
( 
	WHIMAGELIST			hImageList,
	int			    	ImageIndex,
	HDC				hDstDC,
	int				X,
	int				Y,
	int				DX,
	int				DY,
	COLORREF			BackgroundColor,
	COLORREF			ForegroundColor,
	UINT				DrawFlags
);

static BOOL 
W_IDrawNonMaskedImage 
( 
	WHIMAGELIST			hImageList,
	int				ImageIndex,
	HDC				hDstDC,
	int				X,
	int				Y,
	int				DX,
	int				DY,
	COLORREF			BackgroundColor,
	COLORREF			ForegroundColor,
	UINT				DrawFlags
);

static BOOL 
W_IRemoveAll 
( 
   WIMAGE*			pImage
);

/*----------------------------------------------------------------------------*/
int WINAPI
WImageList_ICopyImage
( 
	WHIMAGELIST			hImageList, 
	int				ImageIndex,
	HBITMAP				hBitmap,
	HBITMAP				hMask
)

{

	BITMAP				Bitmap;
	BITMAP				Mask;
	HBITMAP				hTempBitmap = ( HBITMAP )NULL;
	HDC				hTempDC     = ( HDC )NULL;
	HDC				hDeskDC     = GetDC ( GetDesktopWindow () );
	int				NumImages   = 1;
	WIMAGE				*pImage     = &hImageList->ImgList;
	
	int				ErrorCode   = 0;


	memset ( &Bitmap, 0, sizeof ( BITMAP ) );
	memset ( &Mask, 0, sizeof ( BITMAP ) );

	if ( ! ( GetObject ( hBitmap, sizeof ( BITMAP ), &Bitmap ) ) )
		ErrorCode = WIL_ERR_GETOBJECT;
	else
	if ( ( hImageList->CreateFlags & ILC_MASK ) && ( hMask ) && 
		( ! ( GetObject ( hMask, sizeof ( BITMAP ), &Mask ) ) ) )
		ErrorCode = WIL_ERR_GETOBJECT;
	else
	if ( ! ( hTempDC = CreateCompatibleDC ( hDeskDC ) ) )
		ErrorCode = WIL_ERR_CREATECOMPATIBLEDC;
	else
	{
		SelectObject ( hTempDC, hBitmap );

		if ( pImage->hMemoryDC )
			BitBlt ( pImage->hMemoryDC, ImageIndex * hImageList->Width, 0, 
				hImageList->Width, hImageList->Height, hTempDC, 0, 0, SRCCOPY );
		else
		if ( ! ( pImage->hMemoryDC = CreateCompatibleDC ( hDeskDC ) ) )
			ErrorCode = WIL_ERR_CREATECOMPATIBLEDC;
		else
		if ( ! ( hTempBitmap = CreateBitmap ( ( int )( Bitmap.bmWidth / hImageList->Width ) * hImageList->Width,
			Bitmap.bmHeight < hImageList->Height ? Bitmap.bmHeight : hImageList->Height, Bitmap.bmPlanes,
			Bitmap.bmBitsPixel, ( void* )NULL ) ) )
			ErrorCode = WIL_ERR_CREATEBITMAP;
		else
		{
			SelectObject( pImage->hMemoryDC, hTempBitmap );
			BitBlt ( pImage->hMemoryDC, 0, 
				Bitmap.bmHeight < hImageList->Height ? hImageList->Height - Bitmap.bmHeight : 0, 
				( int )( Bitmap.bmWidth / hImageList->Width ) * hImageList->Width,
				Bitmap.bmHeight < hImageList->Height ? Bitmap.bmHeight : hImageList->Height,
				hTempDC, 0, 0, SRCCOPY );
			pImage->NumUsed = ( int )( Bitmap.bmWidth / hImageList->Width );
			pImage->NumImages = hImageList->GrowBy;
			if ( pImage->NumUsed > hImageList->GrowBy )
				pImage->NumImages = pImage->NumUsed;
			NumImages = pImage->NumImages;
		}
		if ( hTempBitmap )
		{
			DeleteObject ( hTempBitmap );
			hTempBitmap = ( HBITMAP ) NULL;
		}
		if ( ( hMask ) && ( hImageList->CreateFlags & ILC_MASK ) )
		{
			SelectObject ( hTempDC, hMask );
			if ( pImage->hMaskDC )
				BitBlt ( pImage->hMaskDC, ImageIndex * hImageList->Width, 0, 
					hImageList->Width, hImageList->Height, hTempDC, 0, 0, 
					SRCCOPY );
			else
			if ( ! ( pImage->hMaskDC = CreateCompatibleDC ( hDeskDC ) ) )
				ErrorCode = WIL_ERR_CREATECOMPATIBLEDC;
			else
			if ( ! ( hTempBitmap = CreateCompatibleBitmap ( hDeskDC,
					( int )( Mask.bmWidth / hImageList->Width ) * hImageList->Width,
					Mask.bmHeight < hImageList->Height ? Mask.bmHeight : hImageList->Height ) ) )
				ErrorCode = WIL_ERR_CREATECOMPATIBLEBITMAP;
			else
			{
				SelectObject ( pImage->hMaskDC, hTempBitmap );
				BitBlt ( pImage->hMaskDC, 0,
					Mask.bmHeight < hImageList->Height ? hImageList->Height - Bitmap.bmHeight : 0,
					( int )( Mask.bmWidth / hImageList->Width ) * hImageList->Width,
					Mask.bmHeight < hImageList->Height ? Mask.bmHeight : hImageList->Height,
					hTempDC, 0, 0, SRCCOPY );
			}

		}
	}

	if ( hTempDC )
		DeleteDC ( hTempDC );

	if ( hTempBitmap )
		DeleteObject ( hTempBitmap );

	if ( hDeskDC )
		ReleaseDC ( GetDesktopWindow (), hDeskDC );

	return ( ErrorCode );

}

/*----------------------------------------------------------------------------------------------
 * Parameters:
 * pImage : pointer to WIMAGE structure that would contain the HDC space for the bitmaps to be
 *	    copied into.
 * nAddedImages : number of new images being copied from hBitmap into WIMAGE::hMemoryDC starting
 *		from position indexed by ImageIndex
 *----------------------------------------------------------------------------------------------*/
int 
WImageList_IAddImage
( 
	WHIMAGELIST			hImageList, 
	int				nAddedImages,
	HBITMAP				hBitmap,
	HBITMAP				hMask
)
{
	BITMAP				Bitmap;
	BITMAP				Mask;
	WIMAGE				*pImage     = &hImageList->ImgList;
	HBITMAP				hTempBitmap = ( HBITMAP )NULL;
	HDC				hTempDC     = ( HDC )NULL;
	HDC				hMemDC      = ( HDC )NULL;
	HDC				hMaskDC     = ( HDC )NULL;
	HDC				hDeskDC     = GetDC ( GetDesktopWindow () );
	int				ImageIndex  = -1;
	int				NewWidth    = 0;
	int				ErrorCode   = 0;


	memset ( &Bitmap, 0, sizeof ( BITMAP ) );
	memset ( &Mask, 0, sizeof ( BITMAP ) );

	if ( ! ( GetObject ( hBitmap, sizeof ( BITMAP ), &Bitmap ) ) )
		ErrorCode = WIL_ERR_GETOBJECT;
	else
	if ( ( hImageList->CreateFlags & ILC_MASK ) && ( hMask ) && 
		( ! ( GetObject ( hMask, sizeof ( BITMAP ), &Mask ) ) ) )
		ErrorCode = WIL_ERR_GETOBJECT;
	else
	if ( ! ( hTempDC = CreateCompatibleDC ( hDeskDC ) ) )
		ErrorCode = WIL_ERR_CREATECOMPATIBLEDC;
    	else
    	{
        	ImageIndex = pImage->NumUsed;
        	if ( nAddedImages > pImage->NumImages - pImage->NumUsed )
        	{
	     		hMemDC = CreateCompatibleDC ( hDeskDC );
    	 		NewWidth = max ( nAddedImages + pImage->NumImages, 
					 hImageList->GrowBy );
	     		hTempBitmap = CreateBitmap ( NewWidth * hImageList->Width,
			    			hImageList->Height, Bitmap.bmPlanes,
			    			Bitmap.bmBitsPixel, ( void* )NULL );
             		if ( hMemDC && hTempBitmap )
             		{
                		hTempBitmap = SelectObject ( hMemDC, hTempBitmap );
				if ( pImage->hMemoryDC )
				{
		    			BitBlt ( hMemDC, 0, 0, pImage->NumUsed * 
					 	 hImageList->Width, 
						 hImageList->Height, 
					 	 pImage->hMemoryDC, 0, 0, SRCCOPY );
                			DeleteDC ( pImage->hMemoryDC );
				}
                		pImage->hMemoryDC = hMemDC;
                		hMemDC = ( HDC )NULL;
    				pImage->NumImages = NewWidth;

                		if ( hImageList->CreateFlags & ILC_MASK )
                		{
    		        		hMaskDC = CreateCompatibleDC ( hDeskDC );
        				DeleteObject ( hTempBitmap );
		            		hTempBitmap = CreateBitmap ( NewWidth * hImageList->Width,
			            	hImageList->Height, Bitmap.bmPlanes,
			            	Bitmap.bmBitsPixel, ( void* )NULL );
                    			if ( hMaskDC && hTempBitmap )
                    			{
                        			hTempBitmap = SelectObject ( hMaskDC, hTempBitmap );
                        			if ( pImage->hMaskDC )
						{
							BitBlt ( hMaskDC, 0, 0, 
	                        	 	 	 	 pImage->NumUsed * 
								 hImageList->Width, 
					 	 		 hImageList->Height, 
	                        	 	 		 pImage->hMaskDC, 0,
								 0, SRCCOPY );
                        				DeleteDC ( pImage->hMaskDC );
						}
                        			pImage->hMaskDC = hMaskDC;
                        			hMaskDC = ( HDC )NULL;
                    			}
                		}
             		}
        	}

		if ( pImage->hMemoryDC )
		{
            		if ( hImageList->CreateFlags & ILC_MASK )
            		{
    		    		hMask = SelectObject ( hTempDC, hMask );
				StretchBlt ( pImage->hMaskDC, pImage->NumUsed * 
				     	     hImageList->Width, 0, 
				     	     nAddedImages * hImageList->Width, 
                                      	     hImageList->Height, hTempDC, 0, 0, 
					     Mask.bmWidth, Mask.bmHeight, SRCCOPY );
    		    		hMask = SelectObject ( hTempDC, hMask );
            		}
    			hBitmap = SelectObject ( hTempDC, hBitmap );
			StretchBlt ( pImage->hMemoryDC, pImage->NumUsed * 
				     hImageList->Width,
				     0, nAddedImages * hImageList->Width, 
				     hImageList->Height, hTempDC, 0, 0, 
				     Bitmap.bmWidth, Bitmap.bmHeight, SRCCOPY );
    			hBitmap = SelectObject ( hTempDC, hBitmap );
						 pImage->NumUsed += nAddedImages;
		}

    	}

	if ( hTempDC )
		DeleteDC ( hTempDC );

	if ( hTempBitmap )
		DeleteObject ( hTempBitmap );

	if ( hDeskDC )
		ReleaseDC ( GetDesktopWindow (), hDeskDC );

	return ImageIndex;

}

/*----------------------------------------------------------------------------*/
int WINAPI
WImageList_Add
(
	WHIMAGELIST			hImageList,
	HBITMAP				hBitmap,
	HBITMAP				hMask
)

{

	BITMAP				Bitmap;
	UINT				ImageListSections;
	int				ImageIndex		    = -1;
	int				nAddedImages		    = 0;
	int				ErrorCode		    = 0;

	memset ( &Bitmap, 0, sizeof ( BITMAP ) );

    ImageListSections = hImageList->ImgList.NumUsed;
	if ( ! ( GetObject ( hBitmap, sizeof ( BITMAP ), &Bitmap ) ) )
		ErrorCode = WIL_ERR_GETOBJECT;
	else
	if ( hImageList->Height <= 0 || hImageList->Width <= 0 )
		ErrorCode = WIL_ERR_INVALIDVALUE;
	else
	{
		nAddedImages = Bitmap.bmWidth <= hImageList->Width ? 1:( Bitmap.bmWidth / hImageList->Width );  
		ImageIndex = WImageList_IAddImage ( hImageList, nAddedImages, hBitmap, hMask );
	}

    return ImageIndex;
}

/*----------------------------------------------------------------------------*/
int WINAPI
WImageList_AddMasked
(
	WHIMAGELIST		hImageList,
	HBITMAP			hBitmap,
	COLORREF		MaskColor
)

{
	BITMAP			Bitmap;
	HDC			hDeskDC		= GetDC ( GetDesktopWindow () );
	HDC			hbmDC		= ( HDC )NULL;
	HDC			hMaskDC		= ( HDC )NULL;
	HBITMAP			hbmMask		= ( HBITMAP )NULL;
	int			NewIndex	= 0;
	int			ErrorCode	= 0;


	memset ( &Bitmap, 0, sizeof ( BITMAP ) );

	if ( ! ( hMaskDC = CreateCompatibleDC( hDeskDC ) )  ||
	     ! ( hbmDC   = CreateCompatibleDC( hDeskDC ) )
	   )
		ErrorCode = WIL_ERR_CREATECOMPATIBLEDC;
	else
	if ( ! ( GetObject ( hBitmap, sizeof ( BITMAP ), &Bitmap ) ) )
		ErrorCode = WIL_ERR_GETOBJECT;
	else
	if ( ! ( hbmMask = CreateBitmap ( ( int ) Bitmap.bmWidth, ( int ) Bitmap.bmHeight,
	                          	  1, 1, NULL ) ) )
		ErrorCode = WIL_ERR_CREATEBITMAP;
	else
	{
		/*create a mask */
		if ( MaskColor == CLR_NONE )
			MaskColor = RGB ( 255, 255, 255 );
		hBitmap = SelectObject ( hbmDC, hBitmap );
		SetBkColor ( hbmDC, MaskColor );
		hbmMask = SelectObject ( hMaskDC, hbmMask );
		BitBlt ( hMaskDC, 0, 0, ( int ) Bitmap.bmWidth, Bitmap.bmHeight,
			 hbmDC, 0, 0, SRCCOPY );
		hBitmap = SelectObject ( hbmDC, hBitmap );
		hbmMask = SelectObject ( hMaskDC, hbmMask );

		NewIndex = WImageList_Add ( hImageList, hBitmap, hbmMask );
	}
	
	if ( hbmMask )
		DeleteObject ( hbmMask );

	if ( hbmDC )
		DeleteDC ( hbmDC );
	if ( hMaskDC )
		DeleteDC ( hMaskDC );
	
	if ( hDeskDC )
		ReleaseDC ( GetDesktopWindow (), hDeskDC );

	return ( NewIndex );

}

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_BeginDrag
(
	WHIMAGELIST		hImageList,
	int			ImageIndex,
	int			HotSpotX,
	int			HotSpotY
)

{
	POINT			ptNewSize;
	HGLOBAL			hDragImage;
	ICONINFO		IconInfo;
	HDC			hDeskDC		= GetDC ( GetDesktopWindow () );
	HICON			hIcon		= ( HICON )NULL;
	HBITMAP			hBkBitmap	= ( HBITMAP )NULL;
	WDRAGIMAGE*		pDragImage	= ( WDRAGIMAGE* )NULL;
	HGLOBAL			hPrevDragImage	= ( HGLOBAL )NULL;
	BITMAP			Bitmap;
	int			ErrorCode	= 0;

	memset ( &IconInfo, 0, sizeof ( ICONINFO ) );
	memset ( &Bitmap, 0, sizeof ( BITMAP ) );

	if ( ! ( hIcon = WImageList_GetIcon ( hImageList, ImageIndex, ILD_TRANSPARENT ) ) )
	{
		ErrorCode = WIL_ERR_GETICON;
		return FALSE;
	}
	if ( !GetIconInfo ( hIcon, &IconInfo ) )
	{
		ErrorCode =  WIL_ERR_GETIMAGEINFO;
		return FALSE;
	}

	GetObject ( IconInfo.hbmColor, sizeof ( BITMAP ), &Bitmap ); 
	ptNewSize.x = Bitmap.bmWidth;
	ptNewSize.y = Bitmap.bmHeight;

	if ( ! ( hDragImage = GlobalAlloc ( GHND, sizeof ( WDRAGIMAGE ) ) ) )
		ErrorCode = WIL_ERR_GLOBALALLOC;
	else
	if ( ! ( pDragImage = ( WDRAGIMAGE* )GlobalLock ( hDragImage ) ) )
		ErrorCode = WIL_ERR_GLOBALLOCK;
	else
	if ( ! ( pDragImage->hBkgroundDC = CreateCompatibleDC ( hDeskDC ) ) )
		ErrorCode = WIL_ERR_CREATECOMPATIBLEDC;
	if ( ! ( hBkBitmap = CreateCompatibleBitmap ( hDeskDC, ptNewSize.x, ptNewSize.y ) ) )
		ErrorCode = WIL_ERR_CREATECOMPATIBLEBITMAP;
	else
	if ( ! ( pDragImage->hImageDC = CreateCompatibleDC ( hDeskDC ) ) ||
	     ! ( pDragImage->hMaskDC = CreateCompatibleDC ( hDeskDC )  )
	   )
		ErrorCode = WIL_ERR_CREATECOMPATIBLEDC;
	else
	if ( ! ( pDragImage->pImageList =  WImageList_Create ( ptNewSize.x, ptNewSize.y, 
							hImageList->CreateFlags, 
							10, hImageList->GrowBy ) ) )
		ErrorCode =  WIL_ERR_LISTCREATE;
	else
	if ( WImageList_ReplaceIcon ( pDragImage->pImageList, -1, hIcon ) == -1 )
		ErrorCode =  WIL_ERR_REPLACEICON;
	else
	{
		IconInfo.hbmColor = SelectObject ( pDragImage->hImageDC, IconInfo.hbmColor );
		IconInfo.hbmMask = SelectObject ( pDragImage->hMaskDC, IconInfo.hbmMask );
		hBkBitmap = SelectObject ( pDragImage->hBkgroundDC, hBkBitmap );
		pDragImage->ptSize.x = ptNewSize.x;
		pDragImage->ptSize.y = ptNewSize.y;
		pDragImage->ptHotSpot.x = HotSpotX;
		pDragImage->ptHotSpot.y = HotSpotY;
		pDragImage->bShow = TRUE;

		if ( ! SetProp ( GetDesktopWindow (), szProperty, hDragImage ) )
			ErrorCode = WIL_ERR_SETWINPROP;
	}

	if ( hIcon )
		DestroyIcon ( hIcon );
	if ( hBkBitmap )
		DeleteObject ( hBkBitmap );
	if ( IconInfo.hbmColor )
		DeleteObject ( IconInfo.hbmColor );
	if ( IconInfo.hbmMask )
		DeleteObject ( IconInfo.hbmMask );
	
	if ( hPrevDragImage )
		GlobalFree ( hPrevDragImage );
	if ( pDragImage )
		GlobalUnlock ( hDragImage );

	return ( ErrorCode >= 0 );

}

/*----------------------------------------------------------------------------*/
WHIMAGELIST WINAPI
WImageList_Create
(
	int			SubImageWidth,
	int			SubImageHeight,
	UINT			CreateFlags,
	int			NumberOfSubImages,
	int			NumberOfImagesToGrowBy
)

{
	HGLOBAL			hMemImageList   = ( HGLOBAL )NULL;
	WHIMAGELIST		hImageList      = ( WHIMAGELIST )NULL;
	HBITMAP			hBitmap	        = ( HBITMAP )NULL;
	int			ErrorCode       = 0;

	static WORD wBlend25Bits [] =
		{ 0xAA, 0x00, 0x55, 0x00, 0xAA, 0x00, 0x55, 0x00 };
	static WORD wBlend50Bits [] =
		{ 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA };

	if ( ! ( hMemImageList = GlobalAlloc ( GHND, sizeof ( _WIMAGELIST ) ) ) )
		ErrorCode = WIL_ERR_GLOBALALLOC;
	else
	if ( ! ( hImageList = ( WHIMAGELIST ) GlobalLock ( hMemImageList ) ) )
		ErrorCode = WIL_ERR_GLOBALLOCK;
	else
	{
		hImageList->hMemImageList   = hMemImageList;
		hImageList->Height          = SubImageHeight;
		hImageList->Width           = SubImageWidth;
		hImageList->CreateFlags     = CreateFlags;
		hImageList->GrowBy          = NumberOfImagesToGrowBy;
		hImageList->ForegroundColor = CLR_DEFAULT;
		hImageList->MaskColor       = CLR_DEFAULT;
		hImageList->BackgroundColor = CLR_NONE;
		hImageList->hBlend25Brush   = ( HBRUSH )NULL;
		hImageList->hBlend50Brush   = ( HBRUSH )NULL;
		
		if ( ( CreateFlags & ILC_MASK ) )
		{
			hBitmap = CreateBitmap ( 8, 8, 1, 1, ( LPSTR ) wBlend25Bits );
			hImageList->hBlend25Brush = CreatePatternBrush ( hBitmap );
			DeleteObject ( hBitmap );

			hBitmap = CreateBitmap ( 8, 8, 1, 1, ( LPSTR ) wBlend50Bits );
			hImageList->hBlend50Brush = CreatePatternBrush ( hBitmap );
			DeleteObject ( hBitmap );
		}
	}

	if ( ErrorCode < 0 )
	{
		if ( hImageList )
		{
			GlobalUnlock ( hMemImageList );
			hImageList = ( WHIMAGELIST )NULL;
		}
		if ( hMemImageList )
			GlobalFree ( hMemImageList );
	}

	return ( hImageList );

}

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_Destroy
(
	WHIMAGELIST		hImageList
)

{

    if ( hImageList->ImgList.hMemoryDC )
		DeleteDC ( hImageList->ImgList.hMemoryDC );
	if ( hImageList->ImgList.hMaskDC )
		DeleteDC ( hImageList->ImgList.hMaskDC );

	GlobalUnlock ( hImageList->hMemImageList );
	GlobalFree ( hImageList->hMemImageList );

	if ( hImageList->hBlend25Brush )
		DeleteObject ( hImageList->hBlend25Brush );
	if ( hImageList->hBlend50Brush )
		DeleteObject ( hImageList->hBlend50Brush );

	hImageList = ( WHIMAGELIST ) NULL;

	return ( TRUE );

}

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_DragEnter
(
	HWND				hWnd,
	int				WindowX,  /* Note this is not ClientX */
	int				WindowY   /* Note this is not ClientY */
)

{
	HDC				hDragWndDC	= ( HDC )NULL;
	HGLOBAL				hDragImage	= ( HGLOBAL )NULL;
	WDRAGIMAGE*			pDragImage	= ( WDRAGIMAGE* )NULL;

	int				ErrorCode	= 0;

	if ( ! ( hDragImage = GetProp ( GetDesktopWindow (), szProperty ) ) ||
	     ! ( pDragImage = ( WDRAGIMAGE *) GlobalLock ( hDragImage  )    ) 
	   )
		ErrorCode = WIL_ERR_GLOBALLOCK;
	else
	if ( ! ( hDragWndDC = GetWindowDC ( hWnd ) ) )
		ErrorCode = WIL_ERR_INVALIDVALUE;
	else
	{
		pDragImage->hDragWnd = hWnd;
		pDragImage->ptPos.x = WindowX-pDragImage->ptHotSpot.x;
		pDragImage->ptPos.y = WindowY-pDragImage->ptHotSpot.y;
		/*record the background where the drag image would be drawn on*/
		BitBlt ( pDragImage->hBkgroundDC, 0, 0,
						  pDragImage->ptSize.x,
						  pDragImage->ptSize.y,
			 hDragWndDC, pDragImage->ptPos.x, 
				     pDragImage->ptPos.y, SRCCOPY );
		/*draw the drag image using True Mask method*/
		BitBlt ( hDragWndDC, pDragImage->ptPos.x, 
				 pDragImage->ptPos.y,
 				 pDragImage->ptSize.x, pDragImage->ptSize.y,
				 pDragImage->hImageDC, 0, 0, SRCINVERT );
		BitBlt ( hDragWndDC, pDragImage->ptPos.x,
				 pDragImage->ptPos.y,
				 pDragImage->ptSize.x, pDragImage->ptSize.y,
				 pDragImage->hMaskDC, 0, 0, SRCAND );
		BitBlt ( hDragWndDC, pDragImage->ptPos.x,
				 pDragImage->ptPos.y,
				 pDragImage->ptSize.x, pDragImage->ptSize.y,
				 pDragImage->hImageDC, 0, 0, SRCINVERT );
		
/*		LockWindowUpdate ( hWnd );*/
	}

	if ( hDragWndDC )
		ReleaseDC ( hWnd, hDragWndDC );
	if ( pDragImage )
		GlobalUnlock ( hDragImage );
	return ( ErrorCode  >= 0 );
}

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_DragLeave
(
	HWND				hWnd
)

{

	int				ErrorCode = 0;

/*	LockWindowUpdate ( NULL );*/
	InvalidateRect ( hWnd, NULL, TRUE );

	return ( ErrorCode >= 0 );

}

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_DragMove
(
	int				WindowX,  /* Note this is not ClientX */
	int				WindowY   /* Note this is not ClientY */
)

{

	HDC				hDragWndDC = ( HDC )NULL;
	HGLOBAL				hDragImage = ( HGLOBAL )NULL;
	WDRAGIMAGE*			pDragImage = ( WDRAGIMAGE* )NULL;

	int				ErrorCode = 0;

	if ( ! ( hDragImage = GetProp ( GetDesktopWindow (), szProperty ) ) ||
	     ! ( pDragImage = ( WDRAGIMAGE *) GlobalLock ( hDragImage  )    ) 
	   )
		ErrorCode = WIL_ERR_GLOBALLOCK;
	else
	if ( ! ( hDragWndDC = GetWindowDC ( pDragImage->hDragWnd ) ) )
		ErrorCode = WIL_ERR_GETDC;
	else
	{
		/*restore the background where the image was previously drawn on*/
		BitBlt ( hDragWndDC, pDragImage->ptPos.x, pDragImage->ptPos.y,
						 pDragImage->ptSize.x, pDragImage->ptSize.y,
			 pDragImage->hBkgroundDC, 0, 0, SRCCOPY );

		pDragImage->ptPos.x = WindowX - pDragImage->ptHotSpot.x;
		pDragImage->ptPos.y = WindowY - pDragImage->ptHotSpot.y;
		/*record the background where the drag image is to be drawn on*/
		BitBlt ( pDragImage->hBkgroundDC, 0, 0,
						  pDragImage->ptSize.x,
						  pDragImage->ptSize.y,
			 hDragWndDC, pDragImage->ptPos.x, pDragImage->ptPos.y, SRCCOPY );
		if ( pDragImage->bShow )
		{
			/*draw the drag image using the true mask method*/
			BitBlt ( hDragWndDC, pDragImage->ptPos.x, pDragImage->ptPos.y,
 							 pDragImage->ptSize.x,
							 pDragImage->ptSize.y,
					 pDragImage->hImageDC, 0, 0, SRCINVERT );
			BitBlt ( hDragWndDC, pDragImage->ptPos.x, pDragImage->ptPos.y,
							 pDragImage->ptSize.x,
							 pDragImage->ptSize.y,
					 pDragImage->hMaskDC, 0, 0, SRCAND );
			BitBlt ( hDragWndDC, pDragImage->ptPos.x, pDragImage->ptPos.y,
							 pDragImage->ptSize.x,
							 pDragImage->ptSize.y,
					 pDragImage->hImageDC, 0, 0, SRCINVERT );
		}
	}

	if ( pDragImage )
		GlobalUnlock ( hDragImage );
	if ( hDragWndDC )
		ReleaseDC ( pDragImage->hDragWnd, hDragWndDC );

	return ( ErrorCode >= 0 );

}

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_DragShowNolock
(
	BOOL				Show
)

{

	HGLOBAL				hDragImage = ( HGLOBAL )NULL;
	WDRAGIMAGE*			pDragImage = ( WDRAGIMAGE* )NULL;

	int				ErrorCode = 0;

	if ( ! ( hDragImage = GetProp ( GetDesktopWindow (), szProperty ) ) ||
	     ! ( pDragImage = ( WDRAGIMAGE *) GlobalLock ( hDragImage  )    ) 
	   )
		ErrorCode = WIL_ERR_GLOBALLOCK;
	else
	{
		 pDragImage->bShow = Show;
		 WImageList_DragMove ( pDragImage->ptPos.x, pDragImage->ptPos.y );
	}

	if ( pDragImage )
		GlobalUnlock ( hDragImage );
	
	return ( ErrorCode >= 0 );

}

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_Draw
(
	WHIMAGELIST			hImageList,
	int				ImageIndex,
	HDC				hDC,
	int				X,
	int				Y,
	UINT				DrawFlags
)

{

	return WImageList_DrawEx ( hImageList, ImageIndex, hDC,
			X, Y, 0, 0, hImageList->BackgroundColor, 
			( COLORREF )GetSysColor ( COLOR_HIGHLIGHT ), DrawFlags);
}

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_DrawEx
(
	WHIMAGELIST		hImageList,
	int			ImageIndex,
	HDC			hDC,
	int			X,
	int			Y,
	int			DX,
	int			DY,
	COLORREF		BackgroundColor,
	COLORREF		ForegroundColor,
	UINT			DrawFlags
)

{

	int				ErrorCode	        = 0;

	if ( BackgroundColor == CLR_NONE )
		DrawFlags |= ILD_TRANSPARENT;
	else
	if ( BackgroundColor == CLR_DEFAULT )
		BackgroundColor = hImageList->BackgroundColor;

	if ( ForegroundColor == CLR_NONE )
		ForegroundColor = GetBkColor ( hDC );
	else
	if ( ForegroundColor == CLR_DEFAULT )
		ForegroundColor = GetSysColor ( COLOR_HIGHLIGHT );

	if ( DX == 0 )
		DX = hImageList->Width;
	if ( DY == 0 )
		DY = hImageList->Height;

	if ( ( hImageList->CreateFlags & ILC_MASK ) /* ||
	     ( DrawFlags & ILD_TRANSPARENT ) ||
	     ( DrawFlags & ILD_MASK )		   */    
	   )
	{		
		W_IDrawMaskedImage ( hImageList, ImageIndex, hDC, X, Y, 
			DX, DY, BackgroundColor, ForegroundColor, DrawFlags );
	}
	else   /* draw normal*/
	{
		W_IDrawNonMaskedImage ( hImageList, ImageIndex, hDC, X, Y, 
			DX, DY, BackgroundColor, ForegroundColor, DrawFlags );
	}

	return ( ErrorCode );

}

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_EndDrag ( )
{

	HANDLE				hDragImage;
	WDRAGIMAGE*			pDragImage	= ( WDRAGIMAGE* )NULL;

	int				ErrorCode	= 0;

	if ( ! ( hDragImage = RemoveProp ( GetDesktopWindow (), szProperty ) ) )
		ErrorCode = WIL_ERR_INVALIDVALUE;
	else
	if ( ! ( pDragImage = ( WDRAGIMAGE* )GlobalLock ( hDragImage ) ) )
		ErrorCode = WIL_ERR_GLOBALLOCK;
	else
	{
		if ( pDragImage->hBkgroundDC )
			DeleteDC ( pDragImage->hBkgroundDC );
		if ( pDragImage->hImageDC )
			DeleteDC ( pDragImage->hImageDC );
		if ( pDragImage->hMaskDC )
			DeleteDC ( pDragImage->hMaskDC );
		if ( pDragImage->pImageList )
			WImageList_Destroy ( pDragImage->pImageList );

		GlobalUnlock ( hDragImage );
		GlobalFree ( hDragImage );	
	}

	return ( ErrorCode >= 0 );

}


/*----------------------------------------------------------------------------*/
COLORREF WINAPI
WImageList_GetBkColor
(
	WHIMAGELIST			hImageList
)

{

	return ( hImageList->BackgroundColor );

}

/*----------------------------------------------------------------------------*/
WHIMAGELIST WINAPI
WImageList_GetDragImage
(
	POINT				*pDragPos,
	POINT				*pHotSpot
)

{
	HGLOBAL				hDragImage	= ( HGLOBAL )NULL;
	WDRAGIMAGE*			pDragImage	= ( WDRAGIMAGE* )NULL;
	WHIMAGELIST			pList		= ( WHIMAGELIST )NULL;

	int				ErrorCode	= 0;

	if ( ! ( hDragImage = GetProp ( GetDesktopWindow (), szProperty ) ) ||
	     ! ( pDragImage = ( WDRAGIMAGE *) GlobalLock ( hDragImage ) ) 
	   )
		ErrorCode = WIL_ERR_GLOBALLOCK;
	else
	if ( ! ( pList = pDragImage->pImageList ) )
		ErrorCode = WIL_ERR_INVALIDVALUE;
	else
	{
		pHotSpot->x = pDragImage->ptHotSpot.x;
		pHotSpot->y = pDragImage->ptHotSpot.y;
		pDragPos->x = pDragImage->ptPos.x;
		pDragPos->y = pDragImage->ptPos.y;
	}	

	if ( pDragImage )
		GlobalUnlock ( hDragImage );
	return ( pList );

}

/*----------------------------------------------------------------------------*/
HICON WINAPI
WImageList_GetIcon
(
	WHIMAGELIST		hImageList,
	int			ImageIndex,
	UINT			DrawFlags
)

{
	ICONINFO		IconInfo;
	HDC				hDeskDC			= GetDC ( GetDesktopWindow () );
	HDC				hMemDC			= ( HDC )NULL;
	HDC				hMemMaskDC		= ( HDC )NULL;
	HICON			hIcon			= ( HICON )NULL;
	WIMAGE			*pImage	        = ( WIMAGE* )NULL;

	int				ErrorCode = 0;

	memset ( &IconInfo, 0, sizeof ( ICONINFO ) );

	pImage = &hImageList->ImgList;
    if ( ImageIndex >= hImageList->ImgList.NumUsed )
		ErrorCode = WIL_ERR_INDEXOUTOFRANGE;	
	else
	if ( ! ( hMemDC = CreateCompatibleDC( hDeskDC ) ) ||
	     ! ( hMemMaskDC = CreateCompatibleDC( hDeskDC ) )
	   )
		ErrorCode = WIL_ERR_CREATECOMPATIBLEDC;
	else
	if ( ! ( IconInfo.hbmColor = CreateCompatibleBitmap( hDeskDC, hImageList->Width, 
					hImageList->Height ) ) ||
	     ! ( IconInfo.hbmMask = CreateCompatibleBitmap( hMemMaskDC, hImageList->Width, 
					hImageList->Height ) )
	   )
		ErrorCode = WIL_ERR_CREATECOMPATIBLEBITMAP;
	else
	{
		IconInfo.fIcon = TRUE;
		IconInfo.xHotspot = 0;
		IconInfo.yHotspot = 0;

		
		SelectObject( hMemMaskDC, IconInfo.hbmMask );
		if ( ( hImageList->CreateFlags & ILC_MASK ) &&
			( pImage->hMaskDC ) )
		{

			BitBlt( hMemMaskDC, 0, 0,
				hImageList->Width, hImageList->Height, pImage->hMaskDC,
				ImageIndex * hImageList->Width, 0,SRCCOPY ); 

		}
		else
			PatBlt ( hMemMaskDC,0,0,hImageList->Width, hImageList->Height,BLACKNESS );

		SelectObject( hMemDC, IconInfo.hbmColor );
		BitBlt( hMemDC, 0, 0,
		        hImageList->Width, hImageList->Height, pImage->hMemoryDC,
		        ImageIndex * hImageList->Width , 
			0,SRCCOPY ); 


		hIcon = CreateIconIndirect ( &IconInfo );
	}

	if ( IconInfo.hbmColor )
		DeleteObject ( IconInfo.hbmColor );

	if ( IconInfo.hbmMask )
		DeleteObject ( IconInfo.hbmMask );

	if ( hMemDC )
		DeleteDC ( hMemDC );

	if ( hMemMaskDC )
		DeleteDC ( hMemMaskDC );

	if ( hDeskDC )
		ReleaseDC ( GetDesktopWindow (), hDeskDC );

	return ( hIcon );

}

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_GetIconSize
(
	WHIMAGELIST		hImageList,
	int				*pIconWidth,
	int				*pIconHeight
)

{

	int				ErrorCode = 0;

	if ( hImageList->Width <= 0  ||	hImageList->Height <= 0 )
		ErrorCode = WIL_ERR_INVALIDVALUE;
	else
	{
		*pIconWidth  = hImageList->Width;
		*pIconHeight = hImageList->Height;
	}
	
	return ( ErrorCode >= 0 );

}

/*----------------------------------------------------------------------------*/
int WINAPI
WImageList_GetImageCount 
( 
	WHIMAGELIST		hImageList
)

{
    return hImageList->ImgList.NumUsed;
}

BOOL WINAPI
WImageList_GetImageInfo 
( 
	WHIMAGELIST		hImageList,
	int				ImageIndex,
	IMAGEINFO FAR   *pImageInfo
)

{
	HDC				hDeskDC			= GetDC ( GetDesktopWindow () );
	HDC				hMemDC			= ( HDC )NULL;
	WIMAGE			*pImage	        = ( WIMAGE* )NULL;

	int				ErrorCode		= 0;

	memset ( pImageInfo, 0, sizeof ( IMAGEINFO ) );
	
    pImage = &hImageList->ImgList;
    if ( ImageIndex >= hImageList->ImgList.NumUsed )
		ErrorCode = WIL_ERR_INDEXOUTOFRANGE;	
	else
	if ( ! ( hMemDC = CreateCompatibleDC( hDeskDC ) ) )
		ErrorCode = WIL_ERR_CREATECOMPATIBLEDC;
	else
	if ( ! ( pImageInfo->hbmImage = CreateCompatibleBitmap ( hDeskDC, 
						pImage->NumUsed * hImageList->Width, 
						hImageList->Height ) ) ||
	       ( ( hImageList->CreateFlags & ILC_MASK ) &&
		 ! ( pImageInfo->hbmMask  = CreateCompatibleBitmap ( hDeskDC, 
						pImage->NumUsed * hImageList->Width, 
						hImageList->Height ) ) )
	       
	   )

	{

		SelectObject ( hMemDC, pImageInfo->hbmImage );
		BitBlt( hMemDC, 0, 0,
		        pImage->NumUsed * hImageList->Width, hImageList->Height, 
			pImage->hMemoryDC, 0, 0, SRCCOPY ); 
		 
		SelectObject ( hMemDC, pImageInfo->hbmMask );
		if ( hImageList->CreateFlags & ILC_MASK ) 
			if ( pImage->hMaskDC ) 
				BitBlt( hMemDC, 0, 0,
					pImage->NumUsed * hImageList->Width, 
					hImageList->Height, 
					pImage->hMaskDC, 0,	0, SRCCOPY ); 
			else
				PatBlt ( hMemDC, 0, 0, 
					 pImage->NumUsed * hImageList->Width, 
					 hImageList->Height, BLACKNESS );

		pImageInfo->rcImage.left = ImageIndex * hImageList->Width;
		pImageInfo->rcImage.top  = 0;
		pImageInfo->rcImage.right = pImageInfo->rcImage.left + hImageList->Width;
		pImageInfo->rcImage.bottom = hImageList->Height;		
	}

	if ( hMemDC )
		DeleteDC ( hMemDC );

	if ( hDeskDC )
		ReleaseDC ( GetDesktopWindow (), hDeskDC );
	return ( ErrorCode >= 0 );

}

/*----------------------------------------------------------------------------*/
WHIMAGELIST WINAPI
WImageList_LoadBitmap
(
	HINSTANCE			hInstance,
	LPCSTR				pstrBitmap,
	int				    SubImageWidth,
	int				    NumberOfImagesToGrowBy,
	COLORREF			MaskColor
)

{

	return WImageList_LoadImage(hInstance, pstrBitmap, SubImageWidth,
			NumberOfImagesToGrowBy, MaskColor, IMAGE_BITMAP, 0);

}

/*----------------------------------------------------------------------------
 *Note: need revisit. 
 *----------------------------------------------------------------------------*/
WHIMAGELIST WINAPI
WImageList_LoadImage
(
	HINSTANCE			hInstance,
	LPCSTR				pstrImage,
	int				SubImageWidth,
	int				NumberOfImagesToGrowBy,
	COLORREF			MaskColor,
	UINT				ImageType,
	UINT				LoadFlags
)

{

	BITMAP				bmColor;
	HBITMAP				hBitmap		= ( HBITMAP )NULL;
	HBITMAP				hbmMono		= ( HBITMAP )NULL;
	WHIMAGELIST			hImageList	= ( WHIMAGELIST )NULL;
	int				ErrorCode	= 0;
	UINT				CreateFlags	= 0;
	HDC				hDeskDC		= GetDC ( GetDesktopWindow () );
	HDC				hColorDC	= ( HDC )NULL;
	HDC				hMonoDC		= ( HDC )NULL;

	memset ( &bmColor, 0, sizeof ( BITMAP ) );

	if ( ! ( hBitmap = W_ILoadImage ( hInstance, pstrImage, 
					MaskColor, ImageType, LoadFlags ) ) )
		ErrorCode = WIL_ERR_LOADIMAGE;
	else
	if ( ! ( GetObject ( hBitmap, sizeof ( BITMAP ), &bmColor ) ) )
		ErrorCode = WIL_ERR_GETOBJECT;
	else
	if ( ( LoadFlags & LR_MONOCHROME ) &&
	     ( ! ( hColorDC = CreateCompatibleDC ( hDeskDC ) ) ||
	       ! ( hMonoDC = CreateCompatibleDC ( hDeskDC ) )  ||
	       ! ( hbmMono = CreateBitmap ( bmColor.bmWidth, bmColor.bmHeight, 1, 1, NULL ) )
	     )
	   )
		ErrorCode = WIL_ERR_CREATECOMPATIBLEDC;
	else
	{
		if ( LoadFlags & LR_MONOCHROME ) 
		{
			hBitmap = SelectObject ( hColorDC, hBitmap );
			hbmMono = SelectObject ( hMonoDC, hbmMono );
			BitBlt ( hMonoDC, 0, 0, bmColor.bmWidth, bmColor.bmHeight,
				 hColorDC, 0, 0, SRCCOPY );
			hBitmap = SelectObject ( hColorDC, hBitmap );
			hbmMono = SelectObject ( hMonoDC, hbmMono );
			DeleteObject ( hBitmap );
			hBitmap = hbmMono;
		}
		else
			CreateFlags |= ILC_MASK;	

		if ( SubImageWidth <= 0 )
		{
			SubImageWidth = ( ImageType & IMAGE_ICON ) ? GetSystemMetrics ( SM_CXICON ) :	
					( ImageType & IMAGE_CURSOR ) ? GetSystemMetrics ( SM_CXCURSOR ) :					
					bmColor.bmWidth;
		}
		
		if ( SubImageWidth <= 0 )
			ErrorCode = WIL_ERR_INVALIDVALUE;
		else
		if ( ! ( hImageList = WImageList_Create ( SubImageWidth, 
							  bmColor.bmHeight, CreateFlags,
							  bmColor.bmWidth / SubImageWidth,
							  NumberOfImagesToGrowBy ) ) )
			ErrorCode = WIL_ERR_LISTCREATE;
		else
		if ( WImageList_AddMasked ( hImageList, hBitmap, MaskColor ) == -1 )
		{
			WImageList_Destroy ( hImageList );
			hImageList = NULL;
			ErrorCode = WIL_ERR_ADDMASKED;
		}
	}

	if ( hBitmap )
		DeleteObject ( hBitmap );

	if ( hColorDC )
		DeleteDC ( hColorDC );
	if ( hMonoDC )
		DeleteDC ( hMonoDC );

	if ( hDeskDC )
		ReleaseDC ( GetDesktopWindow (), hDeskDC );

	return ( hImageList );

}

/*----------------------------------------------------------------------------*/
WHIMAGELIST WINAPI
WImageList_Merge
(
	WHIMAGELIST			hImageList1,
	int				ImageIndex1,
	WHIMAGELIST			hImageList2,
	int				ImageIndex2,
	int				DX,
	int				DY
)

{

	POINT				ptNewSize;
	IMAGEINFO			ImageInfo1;
	IMAGEINFO			ImageInfo2;
	HDC				hDeskDC		= GetDC ( GetDesktopWindow () );
	WHIMAGELIST			pNewImageList	= ( WHIMAGELIST )NULL;
	HICON				hIcon		= ( HICON )NULL;
	HICON				hIcon1		= ( HICON )NULL;
	HICON				hIcon2		= ( HICON )NULL;
	int				ErrorCode	= 0;

	memset ( &ImageInfo1, 0, sizeof ( IMAGEINFO ) );
	memset ( &ImageInfo2, 0, sizeof ( IMAGEINFO ) );

	if ( ! ( hIcon1 = WImageList_GetIcon ( hImageList1, ImageIndex1, ILD_TRANSPARENT ) ) ||

	     ! ( hIcon2 = WImageList_GetIcon ( hImageList2, ImageIndex2, ILD_TRANSPARENT ) )
	   )
		ErrorCode = WIL_ERR_GETICON;
	else
	if ( ! ( hIcon = W_IOverDrawIcon ( hIcon1, hIcon2, DX, DY, &ptNewSize) ) ) 
		ErrorCode =  WIL_ERR_DRAWICON;
	else
	if ( ! ( pNewImageList = WImageList_Create ( ptNewSize.x, ptNewSize.y,
				    hImageList1->CreateFlags, 10, 10 ) )
	   )
		ErrorCode =  WIL_ERR_LISTCREATE ;
	else
	{
		WImageList_ReplaceIcon ( pNewImageList, -1, hIcon );
	}

	if ( ImageInfo1.hbmImage )
		DeleteObject ( ImageInfo1.hbmImage );
	if ( ImageInfo1.hbmMask )
		DeleteObject ( ImageInfo1.hbmMask );
	if ( ImageInfo2.hbmImage )
		DeleteObject ( ImageInfo2.hbmImage );
	if ( ImageInfo2.hbmMask )
		DeleteObject ( ImageInfo2.hbmMask );

	if ( hDeskDC )
		ReleaseDC ( GetDesktopWindow (), hDeskDC );

	return pNewImageList;

}

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_Remove 
( 
	WHIMAGELIST		hImageList, 
	int				ImageIndex
)

{

	WIMAGE			*pImage	            = &hImageList->ImgList;
	int				ErrorCode		    = 0;

    if ( ImageIndex == -1)
		W_IRemoveAll ( pImage );
	else
	if ( ! WImage_IRemove( pImage, ImageIndex ,
			       hImageList->Height, hImageList->Width,
			       hImageList->CreateFlags & ILC_MASK ) )
		ErrorCode = WIL_ERR_LISTREMOVE;
	
	return ( ErrorCode >= 0 );

}

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_Replace
(
	WHIMAGELIST		hImageList,
	int			ImageIndex,
	HBITMAP			hBitmap,
	HBITMAP			hMask
)
{
	int			ErrorCode           = 0;

	if ( ( hImageList->ImgList.NumUsed == 0 ) && 
         ( ImageIndex != WIL_ADDINDEX ) )
		ErrorCode = WIL_ERR_INDEXOUTOFRANGE;
	else
	{
		if ( ImageIndex != WIL_ADDINDEX )
		{ /*--- Replacing rather than adding ---*/
			ErrorCode = WImageList_ICopyImage ( hImageList, ImageIndex, hBitmap, hMask );
		}
		else
		{ /*--- Adding an image as opposed to replacing ---*/
			ErrorCode = ( WImageList_IAddImage ( hImageList, 1, hBitmap, hMask ) == -1 );
		}
	}

	return ( ErrorCode >= 0 );

}

/*----------------------------------------------------------------------------*/
int WINAPI
WImageList_ReplaceIcon
(
	WHIMAGELIST		hImageList,
	int		    	ImageIndex,
	HICON			hIcon
)

{

	int		    	ErrorCode = 0;
	ICONINFO		IconInfo;

	memset ( &IconInfo,0, sizeof ( ICONINFO ) );
	
	if ( ImageIndex < -1 )
		 ErrorCode = WIL_ERR_INVALIDVALUE;
	else
	if ( ! GetIconInfo ( hIcon, &IconInfo ) )
		 ErrorCode =  WIL_ERR_GETICON;
	else
	if ( ImageIndex == -1 )
		 ImageIndex = WImageList_IAddImage ( hImageList, 1, 
                            IconInfo.hbmColor, IconInfo.hbmMask );
	else
	if ( ! WImageList_Replace ( hImageList, ImageIndex, 
			IconInfo.hbmColor, IconInfo.hbmMask ) )
		 ErrorCode =  WIL_ERR_REPLACEICON;

	if ( ErrorCode >= 0 )
		return ImageIndex;
	else
		return -1;

}

/*----------------------------------------------------------------------------*/
COLORREF WINAPI
WImageList_SetBkColor
(
	WHIMAGELIST			hImageList,
	COLORREF			BackgroundColor
)

{

	COLORREF			ReturnColor = hImageList->BackgroundColor;

	hImageList->BackgroundColor = BackgroundColor;
	
	return ( ReturnColor );

}

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_SetDragCursorImage
(
	WHIMAGELIST		hImageList,
	int		    	ImageIndex,
	int		    	HotSpotX,
	int		        HotSpotY
)

{
	POINT			ptNewSize;
	ICONINFO		IconInfo;
	HDC		    	hDeskDC		= GetDC ( GetDesktopWindow () );
	HICON			hIcon		= ( HICON )NULL;
	HICON			hIcon1		= ( HICON )NULL;
	HICON			hIcon2		= ( HICON )NULL;
	HBITMAP			hBkBitmap	= ( HBITMAP )NULL;
	HGLOBAL			hDragImage	= ( HGLOBAL )NULL;
	WDRAGIMAGE*		pDragImage	= ( WDRAGIMAGE* )NULL;

	int				    ErrorCode	= 0;

	memset ( &IconInfo, 0, sizeof ( ICONINFO ) );

	if ( ! ( hDragImage = GetProp ( GetDesktopWindow (), szProperty ) ) ||
	     ! ( pDragImage = ( WDRAGIMAGE *) GlobalLock ( hDragImage  )    ) 
	   )
		ErrorCode = WIL_ERR_GLOBALLOCK;
	else
	if ( ! ( hIcon2 = WImageList_GetIcon ( hImageList, ImageIndex, ILD_TRANSPARENT ) ) ||
	     ! ( hIcon1 = WImageList_GetIcon ( pDragImage->pImageList, 0, ILD_TRANSPARENT ) )	   
	   )
		ErrorCode = WIL_ERR_GETICON;
	else
	if ( ! ( hIcon = W_IOverDrawIcon ( hIcon1, hIcon2, HotSpotX, HotSpotY, &ptNewSize) ) ) 
		ErrorCode =  WIL_ERR_DRAWICON;
	else
	if ( ! GetIconInfo ( hIcon, &IconInfo ) )
		ErrorCode = WIL_ERR_GETIMAGEINFO;
	else
	if ( ! ( hBkBitmap = CreateCompatibleBitmap ( hDeskDC, ptNewSize.x, ptNewSize.y ) ) )
		ErrorCode = WIL_ERR_CREATECOMPATIBLEBITMAP;
	else
	{
		IconInfo.hbmColor = SelectObject ( pDragImage->hImageDC, IconInfo.hbmColor );
		IconInfo.hbmMask = SelectObject ( pDragImage->hMaskDC, IconInfo.hbmMask );
		hBkBitmap = SelectObject ( pDragImage->hBkgroundDC, hBkBitmap );
		pDragImage->ptSize.x = ptNewSize.x;
		pDragImage->ptSize.y = ptNewSize.y;
		pDragImage->ptHotSpot.x = HotSpotX;
		pDragImage->ptHotSpot.y = HotSpotY;
	}

	
	if ( hBkBitmap )
		DeleteObject ( hBkBitmap );

	if ( hIcon )
		DestroyIcon ( hIcon );
	if ( hIcon1 )
		DestroyIcon ( hIcon1 );
	if ( hIcon2 )
		DestroyIcon ( hIcon2 );

	if ( IconInfo.hbmColor )
		DeleteObject ( IconInfo.hbmColor );
	if ( IconInfo.hbmMask )
		DeleteObject ( IconInfo.hbmMask );

	if ( hDeskDC )
		ReleaseDC ( GetDesktopWindow (), hDeskDC );

	if ( pDragImage )
		GlobalUnlock ( hDragImage );

	return ( ErrorCode >= 0 );

}


/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_SetIconSize
(
	WHIMAGELIST		hImageList,
	int		    	SubImageHeight,
	int		    	SubImageWidth
)

{

	int		    	ErrorCode = 0;

	if ( ! hImageList )
		ErrorCode = WIL_ERR_INVALIDVALUE;
	else
	{
		hImageList->Height = SubImageHeight;
		hImageList->Width = SubImageWidth;
		return WImageList_Remove( hImageList, -1 );
	}

	return ( ErrorCode >= 0 );

}

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_SetOverlayImage
(
	WHIMAGELIST		hImageList,
	int			ImageIndex,
	int			OverlayIndex
)

{

	int			ErrorCode = 0;

	if ( OverlayIndex <= 0  || OverlayIndex > MAXOVERLAY )
		ErrorCode = WIL_ERR_INDEXOUTOFRANGE;
	else
	{
		hImageList->OverlayIndices[OverlayIndex-1] = ImageIndex;
	}

	return ( ErrorCode );

}

/*----------------------------------------------------------------------------*
 *----------------------------------------------------------------------------*/

BOOL WINAPI
WImage_IRemove
( 
	WIMAGE			*pImage,
	int			iRemoved,
	int			Height,
	int			Width,
	BOOL			bMask
)
{
	
	HDC		    	hNewDC		= ( HDC )NULL;
	HDC			hMaskDC		= ( HDC )NULL;
	HBITMAP			hNewBitmap	= ( HBITMAP )NULL;
	HBITMAP			hNewMask	= ( HBITMAP )NULL;
	int			ErrorCode	= 0;

    if ( iRemoved >= pImage->NumUsed ) 
        return FALSE;

	if ( ! ( hNewDC = CreateCompatibleDC( pImage->hMemoryDC ) ) ||
	       ( bMask && ! ( hMaskDC = CreateCompatibleDC( pImage->hMaskDC ) ) )	)
	{
	       ErrorCode = WIL_ERR_CREATECOMPATIBLEDC;
	       if ( hNewDC )
			DeleteDC( hNewDC );
	       if ( hMaskDC )
			DeleteDC( hMaskDC );
	}
	else
	if ( ! ( hNewBitmap = CreateCompatibleBitmap ( pImage->hMemoryDC, 
			                      Width * pImage->NumImages, Height ) ) ||
	       ( bMask && ! ( hNewMask = CreateCompatibleBitmap( pImage->hMaskDC,
					      Width * pImage->NumImages, Height ) ) )
	   )
		ErrorCode = WIL_ERR_CREATECOMPATIBLEBITMAP;
	else
	{
		
		hNewBitmap = SelectObject ( hNewDC, hNewBitmap );
        if ( iRemoved > 0 )
			BitBlt ( hNewDC, 0, 0, 
				 Width * iRemoved, Height,
				 pImage->hMemoryDC, 0, 0, SRCCOPY );
		BitBlt ( hNewDC, Width * iRemoved, 0, 
			     pImage->NumUsed - 1, Height,
			     pImage->hMemoryDC, Width * ( iRemoved + 1 ), 0, SRCCOPY );
		
		DeleteDC( pImage->hMemoryDC );
		pImage->hMemoryDC = hNewDC;
		DeleteObject ( hNewBitmap );

		if ( bMask )
		{
			hNewMask = SelectObject ( hMaskDC, hNewMask );
            if ( iRemoved > 0 )
			    BitBlt ( hMaskDC, 0, 0, 
				     Width * iRemoved, Height,
				     pImage->hMaskDC, 0, 0, SRCCOPY );
		    BitBlt ( hMaskDC, Width * iRemoved, 0, 
			         pImage->NumUsed - 1, Height,
			         pImage->hMaskDC, Width * ( iRemoved + 1 ), 0, SRCCOPY );
			
			DeleteDC( pImage->hMaskDC );
			pImage->hMaskDC = hMaskDC;
			DeleteObject ( hNewMask );
		}

		pImage->NumUsed--;
	}

	return (ErrorCode >= 0);
}

static BOOL 
W_IDrawNonMaskedImage 
( 
	WHIMAGELIST		hImageList,
	int			ImageIndex,
	HDC			hDstDC,
	int			X,
	int			Y,
	int			DX,
	int			DY,
	COLORREF		BackgroundColor,
	COLORREF		ForegroundColor,
	UINT			DrawFlags
)
{

	WIMAGE			*pImageBlock	= ( WIMAGE* )NULL;

    if ( ImageIndex >= hImageList->ImgList.NumUsed ) 
        return FALSE;
	pImageBlock = &hImageList->ImgList;

	BitBlt ( hDstDC, X, Y, DX, DY, pImageBlock->hMemoryDC,
			( ImageIndex ) * hImageList->Width, 0, SRCCOPY );

	return TRUE;
}


#if 1
static BOOL 
W_IDrawMaskedImage 
( 
	WHIMAGELIST		hImageList,
	int			ImageIndex,
	HDC			hDstDC,
	int			X,
	int			Y,
	int			DX,
	int			DY,
	COLORREF		BackgroundColor,
	COLORREF		ForegroundColor,
	UINT			DrawFlags
)
{

	HDC		    	hDeskDC		= GetDC ( GetDesktopWindow () );
	HDC			hMaskDC		= CreateCompatibleDC ( hDeskDC );
	HBITMAP			hbmNewMask	= ( HBITMAP )NULL;

	HDC			hTempDC		= CreateCompatibleDC ( hDeskDC );
	HBITMAP			hbm		= CreateCompatibleBitmap ( hDeskDC, DX, DY );
	HBRUSH			hbr		= ( HBRUSH )NULL;

	WIMAGE			*pImageBlock	= ( WIMAGE* )NULL;
	WIMAGE              	*pImageOverlay	= ( WIMAGE* )NULL;
	HBRUSH			hBlendBrush	    = ( HBRUSH )NULL;
	int			OverlayIndex	= 0;
	int			SrcX		= 0;
	int		    	SrcY	        = 0;

    if ( ImageIndex >= hImageList->ImgList.NumUsed )
        return FALSE;
	pImageBlock = &hImageList->ImgList;

	SrcX = ( ImageIndex ) * hImageList->Width;

	hBlendBrush = ( DrawFlags & ILD_BLEND50 )? hImageList->hBlend50Brush:
		     ( DrawFlags & ILD_BLEND25 )? hImageList->hBlend25Brush:
		     ( HBRUSH )NULL;
	if ( hBlendBrush )
	{
		hBlendBrush = SelectObject ( hMaskDC, hBlendBrush );
		SetBrushOrgEx ( hMaskDC, 0, 0, &pt );
	}

	hbmNewMask = CreateCompatibleBitmap ( hMaskDC, DX, DY );
	hbmNewMask = SelectObject ( hMaskDC, hbmNewMask );
	PatBlt ( hMaskDC, 0, 0, DX, DY, PATCOPY );

	if ( ( DrawFlags & ILD_TRANSPARENT ) ||
	     ( ( DrawFlags & ILD_IMAGE ) && ( DrawFlags & ILD_MASK ) ) ||
	     ( ( DrawFlags == ILD_NORMAL ) && ( BackgroundColor == CLR_NONE ) )
	   )
	{
		hbr = SelectObject ( hTempDC, CreateSolidBrush ( ForegroundColor ) );
		hbm = SelectObject ( hTempDC, hbm );
		BitBlt ( hTempDC, 0, 0, DX, DY, pImageBlock->hMemoryDC, SrcX, SrcY, SRCCOPY);

		/* operation 0x00DD0228 would preserve the bits of the destination except when
		   both the destination bit and the corresponding bit of source are black, where
		   the dest bit is set to white. */
		BitBlt ( hMaskDC, 0, 0, DX, DY, pImageBlock->hMaskDC,
			SrcX, SrcY, 0x00DD0228 ); 

		/* preserve the dest. bits which are white, otherwise wherever the mask bits 
		   are black, paint the corresponding dest bits with the foreground color
         */
		if ( hBlendBrush )
			BitBlt ( hTempDC, 0, 0, DX, DY, hMaskDC, 0, 0, 0x00B8074A );

		/*draw bitmap transparently using true mask method*/
		BitBlt(hDstDC, X, Y, DX, DY, hTempDC, 0, 0, SRCINVERT);
		BitBlt(hDstDC, X, Y, DX, DY, pImageBlock->hMaskDC, SrcX, SrcY, SRCAND);
		BitBlt(hDstDC, X, Y, DX, DY, hTempDC, 0, 0, SRCINVERT);

		hbm = SelectObject ( hTempDC, hbm );
		hbr = SelectObject ( hTempDC, hbr );
		OverlayIndex = GETOVERLAYINDEX( DrawFlags );
		ImageIndex = ISVALIDOVERLAY( OverlayIndex )?
			hImageList->OverlayIndices[OverlayIndex-1] : -1;
		if ( ImageIndex >= 0 ) 
		{
            pImageOverlay = &hImageList->ImgList;
			BitBlt(hDstDC, X, Y, DX, DY, pImageOverlay->hMemoryDC, 
				ImageIndex * hImageList->Width, 0, SRCINVERT);
			BitBlt(hDstDC, X, Y, DX, DY, pImageOverlay->hMaskDC, 
				ImageIndex * hImageList->Width, SrcY, SRCAND);
			BitBlt(hDstDC, X, Y, DX, DY, pImageOverlay->hMemoryDC, 
				ImageIndex * hImageList->Width, 0, SRCINVERT);
			/*WImageList_IReleaseWImage ( hImageList );*/
		}

	}
	else
	if ( ( DrawFlags & ILD_MASK ) )
	{

		BitBlt ( hDstDC, X, Y, DX, DY, pImageBlock->hMaskDC,
			SrcX, SrcY, SRCCOPY ); 
		if ( hBlendBrush )
		{
			hbr = SelectObject ( hDstDC, CreateSolidBrush ( ForegroundColor ) );
			BitBlt ( hDstDC, X, Y, DX, DY, hMaskDC, 0, 0, 0x00B8074A );
		}
	}
	else
	{

		if ( hBlendBrush )
		{
			BitBlt ( hDstDC, X, Y, DX, DY, pImageBlock->hMemoryDC, SrcX, SrcY, SRCCOPY );
			hbr = SelectObject ( hDstDC, CreateSolidBrush ( ForegroundColor ) );
			BitBlt ( hDstDC, X, Y, DX, DY, hMaskDC, 0, 0, 0x00B8074A );
			hbr = SelectObject ( hDstDC, hbr );
		}
		else
		{
			hbr = SelectObject ( hDstDC, CreateSolidBrush ( BackgroundColor ) );
			PatBlt ( hDstDC, X, Y, DX, DY, PATCOPY);
			hbr = SelectObject ( hDstDC, hbr );
			BitBlt(hDstDC, X, Y, DX, DY, pImageBlock->hMemoryDC, SrcX, SrcY, SRCINVERT);
			BitBlt(hDstDC, X, Y, DX, DY, pImageBlock->hMaskDC, SrcX, SrcY, SRCAND);
			BitBlt(hDstDC, X, Y, DX, DY, pImageBlock->hMemoryDC, SrcX, SrcY, SRCINVERT);
		}
	}


	/*cleanup*/

	hbmNewMask = SelectObject ( hMaskDC, hbmNewMask );
	if ( hbmNewMask )
		DeleteObject ( hbmNewMask );
	if ( hbm )
		DeleteObject ( hbm );
	if ( hbr )
		DeleteObject ( hbr );
	/*hBlendBrush is managed by image list. No need to destroy it here*/
	if (hBlendBrush)	
		hBlendBrush = SelectObject ( hMaskDC, hBlendBrush );
	if ( hTempDC )
		DeleteDC ( hTempDC );
	if ( hMaskDC )
		DeleteDC ( hMaskDC );
	if ( hDeskDC )
		ReleaseDC ( GetDesktopWindow (), hDeskDC );

	/*WImageList_IReleaseWImage ( hImageList );*/

	return TRUE;
}
#else
static BOOL 
W_IDrawMaskedImage 
( 
   HDC				hDstDC, 
   WIMAGE*			pImageBlock, 
   int				X, 
   int				Y,
   int				DX,
   int				DY,
   int				SrcX,
   int				SrcY,
   COLORREF			ForegroundColor,
   COLORREF			BackgroundColor,
   HBRUSH			hBlendBrush,
   UINT				DrawFlags
)
{

	HDC			hDeskDC		= GetDC ( GetDesktopWindow () );
	HDC			hMaskDC		= CreateCompatibleDC ( hDeskDC );
	HBITMAP			hbmNewMask	= ( HBITMAP )NULL;
	POINT			pt;

	HDC			hTempDC		= CreateCompatibleDC ( hDeskDC );
	HBITMAP			hbm	    	= CreateCompatibleBitmap ( hDeskDC, DX, DY );
	HBRUSH			hbr		= ( HBRUSH )NULL;


	if ( hBlendBrush )
	{
		hBlendBrush = SelectObject ( hMaskDC, hBlendBrush );
		SetBrushOrgEx ( hMaskDC, 0, 0, &pt );
	}

	hbmNewMask = CreateCompatibleBitmap ( hMaskDC, DX, DY );
	hbmNewMask = SelectObject ( hMaskDC, hbmNewMask );
	PatBlt ( hMaskDC, 0, 0, DX, DY, PATCOPY );

	if ( ( DrawFlags & ILD_TRANSPARENT ) ||
	     ( ( DrawFlags & ILD_IMAGE ) && ( DrawFlags & ILD_MASK ) ) ||
	     ( ( DrawFlags == ILD_NORMAL ) && ( BackgroundColor == CLR_NONE ) )
	   )
	{
		hbr = SelectObject ( hTempDC, CreateSolidBrush ( ForegroundColor ) );
		hbm = SelectObject ( hTempDC, hbm );
		BitBlt ( hTempDC, 0, 0, DX, DY, pImageBlock->hMemoryDC, SrcX, SrcY, SRCCOPY);

		/* operation 0x00DD0228 would preserve the bits of the destination except when
		 * both the destination bit and the corresponding bit of source are black, where
		  the dest bit is set to white. */
		BitBlt ( hMaskDC, 0, 0, DX, DY, pImageBlock->hMaskDC,
			SrcX, SrcY, 0x00DD0228 ); 

		/* preserve the dest. bits which are white, otherwise wherever the mask bits 
		   are black, paint the corresponding dest bits with the foreground color*/
		if ( hBlendBrush )
			BitBlt ( hTempDC, 0, 0, DX, DY, hMaskDC, 0, 0, 0x00B8074A );

		/*draw bitmap transparently using true mask method*/
		BitBlt(hDstDC, X, Y, DX, DY, hTempDC, 0, 0, SRCINVERT);
		BitBlt(hDstDC, X, Y, DX, DY, pImageBlock->hMaskDC, SrcX, SrcY, SRCAND);
		BitBlt(hDstDC, X, Y, DX, DY, hTempDC, 0, 0, SRCINVERT);

		hbm = SelectObject ( hTempDC, hbm );
		hbr = SelectObject ( hTempDC, hbr );

	}
	else
	if ( ( DrawFlags & ILD_MASK ) )
	{

		BitBlt ( hDstDC, X, Y, DX, DY, pImageBlock->hMaskDC,
			SrcX, SrcY, SRCCOPY ); 
		if ( hBlendBrush )
		{
			hbr = SelectObject ( hDstDC, CreateSolidBrush ( ForegroundColor ) );
			BitBlt ( hDstDC, X, Y, DX, DY, hMaskDC, 0, 0, 0x00B8074A );
		}
	}
	else
	{
		BitBlt ( hDstDC, X, Y, DX, DY, pImageBlock->hMemoryDC, SrcX, SrcY, SRCCOPY );

		if ( hBlendBrush )
		{
			hbr = SelectObject ( hDstDC, CreateSolidBrush ( ForegroundColor ) );
			BitBlt ( hDstDC, X, Y, DX, DY, hMaskDC, 0, 0, 0x00B8074A );
			hbr = SelectObject ( hDstDC, hbr );
		}
	}


	/*cleanup*/

	hbmNewMask = SelectObject ( hMaskDC, hbmNewMask );
	if ( hbmNewMask )
		DeleteObject ( hbmNewMask );
	if ( hbm )
		DeleteObject ( hbm );
	if ( hbr )
		DeleteObject ( hbr );
	/*hBlendBrush is managed by image list. No need to destroy it here*/
	hBlendBrush = SelectObject ( hMaskDC, hBlendBrush );
	if ( hTempDC )
		DeleteDC ( hTempDC );
	if ( hMaskDC )
		DeleteDC ( hMaskDC );
	if ( hDeskDC )
		ReleaseDC ( GetDesktopWindow (), hDeskDC );

	return TRUE;
}

#endif

static BOOL 
W_IRemoveAll 
( 
   WIMAGE*			pImage
)
{
	int			    ErrorCode = 0;
	
	if ( !pImage )
		ErrorCode = WIL_ERR_INVALIDVALUE;
	else
	{
		if ( pImage->hMemoryDC )
			DeleteDC ( pImage->hMemoryDC );
		if ( pImage->hMaskDC )
			DeleteDC ( pImage->hMaskDC );
        pImage->NumUsed = pImage->NumImages = 0;
	}

	return ( ErrorCode >= 0 );
}

