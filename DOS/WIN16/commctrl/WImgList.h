
/*  WImgList.h	1.5 
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


#ifndef __WIMAGELIST_H__
#define __WIMAGELIST_H__


#include "WLists.h"

#define WIL_ERR_GENERAL			-1
#define WIL_ERR_GETOBJECT		-2
#define WIL_ERR_CREATECOMPATIBLEDC	-3
#define WIL_ERR_LISTLOCK		-4
#define WIL_ERR_INDEXOUTOFRANGE		-5
#define WIL_ERR_LISTINSERT		-6
#define WIL_ERR_GLOBALALLOC		-7
#define WIL_ERR_GLOBALLOCK		-8
#define WIL_ERR_LISTCREATE		-9
#define	WIL_ERR_CREATECOMPATIBLEBITMAP	-10
#define WIL_ERR_CREATEBITMAP		-11
#define WIL_ERR_CREATESOLIDBRUSH	-12
#define WIL_ERR_LISTREMOVE	        -13
#define WIL_ERR_GETIMAGEINFO	        -14
#define WIL_ERR_LOADIMAGE	        -15
#define WIL_ERR_INVALIDVALUE	        -16
#define WIL_ERR_GETICON		        -17
#define WIL_ERR_ADDMASKED		        -18
#define WIL_ERR_REPLACEICON			-19
#define WIL_ERR_SETWINPROP			-20
#define WIL_ERR_GETDC			-21
#define WIL_ERR_DRAWICON			-22




#ifdef _WINDOWS

#else

#define WHIMAGELIST			HIMAGELIST

#define WImageList_Add			ImageList_Add
#define WImageList_AddMasked		ImageList_AddMasked
#define WImageList_BeginDrag		ImageList_BeginDrag
#define WImageList_Create		ImageList_Create
#define WImageList_Destroy		ImageList_Destroy
#define WImageList_DragEnter		ImageList_DragEnter
#define WImageList_DragLeave		ImageList_DragLeave
#define WImageList_DragMove		ImageList_DragMove
#define WImageList_DragShowNolock	ImageList_DragShowNolock
#define WImageList_Draw			ImageList_Draw
#define WImageList_DrawEx		ImageList_DrawEx
#define WImageList_EndDrag		ImageList_EndDrag
#define WImageList_GetBkColor		ImageList_GetBkColor
#define WImageList_GetDragImage		ImageList_GetDragImage
#define WImageList_GetIcon		ImageList_GetIcon
#define WImageList_GetIconSize		ImageList_GetIconSize
#define WImageList_GetImageCount	ImageList_GetImageCount
#define WImageList_LoadBitmap		ImageList_LoadBitmap
#define WImageList_LoadImage		ImageList_LoadImage
#define WImageList_Merge			ImageList_Merge
#define WImageList_Remove		ImageList_Remove
#define WImageList_Replace		ImageList_Replace
#define WImageList_ReplaceIcon		ImageList_ReplaceIcon
#define WImageList_SetBkColor		ImageList_SetBkColor
#define WImageList_SetDragCursorImage	ImageList_SetDragCursorImage
#define WImageList_SetIconSize		ImageList_SetIconSize
#define WImageList_SetOverlayImage	ImageList_SetOverlayImage

#define WImageList_Read			"----- ImageList_Read is not supported -----"
#define WImageList_Write			"----- ImageList_Write is not supported -----"

#endif /* #ifdef _WINDOWS #else */

#if 0
#ifdef _WIMAGELIST_SOURCE
typedef _WIMAGELIST*			WHIMAGELIST;
#else
typedef void*				WHIMAGELIST;
#endif
#endif

/*-----------------------------P R O T O T Y P E S-----------------------------*/
#ifdef __cplusplus
extern "C" 		/* begin C++ defs */
{
#endif
int WINAPI
WImageList_Add
(
    WHIMAGELIST			hImageList,
	HBITMAP				hBitmap,
	HBITMAP				hMask
);
	
/*----------------------------------------------------------------------------*/
int WINAPI
WImageList_AddMasked
(
	WHIMAGELIST			hImageList,
	HBITMAP				hMask,
	COLORREF			MaskColor
);

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_BeginDrag
(
	WHIMAGELIST			hImageList,
	int				ImageIndex,
	int				HotSpotX,
	int				HotSpotY
);

/*----------------------------------------------------------------------------*/
WHIMAGELIST WINAPI
WImageList_Create
(
	int				SubImageWidth,
	int				SubImageHeight,
	UINT				CreateFlags,
	int				NumberOfSubImages,
	int				NumberOfImagesToGrowBy
);

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_Destroy
(
	WHIMAGELIST			hImageList
);

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_DragEnter
(
	HWND				hWnd,
	int				WindowX,  /* Note this is not ClientX */
	int				WindowY   /* Note this is not ClientY */
);

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_DragLeave
(
	HWND				hWnd
);

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_DragMove
(
	int				WindowX,  /* Note this is not ClientX */
	int				WindowY   /* Note this is not ClientY */
);

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_DragShowNolock
(
	BOOL				Show
);

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
);

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_DrawEx
(
	WHIMAGELIST			hImageList,
	int				ImageIndex,
	HDC				hDC,
	int				X,
	int				Y,
	int				DX,
	int				DY,
	COLORREF			BackgroundColor,
	COLORREF			ForegroundColor,
	UINT				DrawFlags
);

/*----------------------------------------------------------------------------*/
COLORREF WINAPI
WImageList_GetBkColor
(
	WHIMAGELIST			hImageList
);

/*----------------------------------------------------------------------------*/
WHIMAGELIST WINAPI
WImageList_GetDragImage
(
	POINT				*pDragPosition,
	POINT				*pHotSpot
);

/*----------------------------------------------------------------------------*/
HICON WINAPI
WImageList_GetIcon
(
	WHIMAGELIST			hImageList,
	int				ImageIndex,
	UINT				DrawFlags
);

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_GetIconSize
(
	WHIMAGELIST			hImageList,
	int				*pIconWidth,
	int				*pIconHeight
);

/*----------------------------------------------------------------------------*/
int WINAPI
WImageList_GetImageCount 
( 
	WHIMAGELIST			hImageList
);

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_GetImageInfo 
( 
	WHIMAGELIST			hImageList,
	int				ImageIndex,
	IMAGEINFO FAR                   *pImageInfo
);

/*----------------------------------------------------------------------------*/
WHIMAGELIST WINAPI
WImageList_LoadBitmap
(
	HINSTANCE			hInstance,
	LPCSTR				pstrBitmap,
	int				SubImageWidth,
	int				NumberOfImagesToGrowBy,
	COLORREF			MaskColor
);

/*----------------------------------------------------------------------------*/
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
);

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
);

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_Remove 
( 
	WHIMAGELIST			hImageList, 
	int				ImageIndex
);

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_Replace
(
	WHIMAGELIST			hImageList,
	int				ImageIndex,
	HBITMAP				hBitmap,
	HBITMAP				hMask
);

/*----------------------------------------------------------------------------*/
int WINAPI
WImageList_ReplaceIcon
(
	WHIMAGELIST			hImageList,
	int				ImageIndex,
	HICON				hIcon
);

/*----------------------------------------------------------------------------*/
COLORREF WINAPI
WImageList_SetBkColor
(
	WHIMAGELIST			hImageList,
	COLORREF			BackgroundColor
);

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_SetDragCursorImage
(
	WHIMAGELIST			hImageList,
	int				ImageIndex,
	int				HotSpotX,
	int				HotSpotY
);

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_SetIconSize
(
	WHIMAGELIST			hImageList,
	int				SubImageHeight,
	int				SubImageWidth
);

/*----------------------------------------------------------------------------*/
BOOL WINAPI
WImageList_SetOverlayImage
(
	WHIMAGELIST			hImageList,
	int				ImageIndex,
	int				OverlayIndex
);

BOOL WINAPI
WImageList_EndDrag
(
);

#ifdef __cplusplus
}			/* end  C++ defs */
#endif /* __cplusplus block */


#endif /* #ifdef __IMAGELIST_H__ */
