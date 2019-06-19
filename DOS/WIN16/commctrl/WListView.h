
/*  WListView.h	1.3 
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

#ifndef CWLISTVIEW_H
#define CWLISTVIEW_H



#if defined ( _WINDOWS )
#   define WLVD_CLASSNAME                   "WSysListView32"
#else
#   define WLVD_CLASSNAME                   "SysListView32"
#endif



typedef enum
{
    ELV_IMAGELIST_LARGE,
    ELV_IMAGELIST_SMALL
} ELV_IMAGELIST_TYPES;



#if defined ( __cplusplus )

#include "WWindow.h"
class CWALVColumns;

class CWListView :
    public CWWindow
{

    public:

        CWListView (
            HWND                                hWnd,
            LPCREATESTRUCT                      pCreateStruct );
        virtual ~CWListView ();

        virtual void OnWM_PAINT (
            LPRECT                              pPaintRect );
        virtual void OnWM_ERASEBKGND (
            HDC                                 hDC );
        virtual void OnWM_SIZE ();
        virtual void OnWM_SETFONT (
            WPARAM                              wParam,
            LPARAM                              lParam );
        virtual HFONT OnWM_GETFONT ();
        virtual long OnWM_NOTIFY (
            DWORD                               ControlID,
            LPNMHDR                             pNMHdr );
        virtual void OnWM_HSCROLL (
            int                                 PositionCode,
            short int                           Position );
        virtual void OnWM_VSCROLL (
            int                                 PositionCode,
            short int                           Position );

        void Arrange (
            UINT                                Code );
        COLORREF LVGetBKColor()
        {
            return ( BKColor );
        }
        COLORREF LVGetTextBKColor()
        {
            return ( TextBKColor );
        }
        COLORREF LVGetTextColor()
        {
            return ( TextColor );
        }
        BOOL LVSetBKColor (
            COLORREF                            NewBKColor );
        BOOL LVSetTextBKColor (
            COLORREF                            NewTextBKColor );
        BOOL LVSetTextColor (
            COLORREF                            NewTextColor );
        HIMAGELIST GetImageList (
            ELV_IMAGELIST_TYPES                 ImageListType );
        BOOL SetImageList (
            ELV_IMAGELIST_TYPES                 ImageListType,
            HIMAGELIST                          hImageList );
        int InsertColumn (
            DWORD                               IndexBefore,
            LV_COLUMN                           *pLVColumn );
        BOOL DeleteColumn (
            DWORD                               ColumnIndex );
        int InsertItem (
            DWORD                               IndexBefore,
            LV_ITEM                             *pLVItem );
        BOOL DeleteItem (
            DWORD                               ItemIndex );
        BOOL GetColumn (
            DWORD                               ColumnIndex,
            LV_COLUMN                           *pLVColumn );
        BOOL SetColumn (
            DWORD                               ColumnIndex,
            LV_COLUMN                           *pLVColumn );
        BOOL GetItem (
            DWORD                               ColumnIndex,
            DWORD                               ItemIndex,
            LV_ITEM                             *pLVItem );
        BOOL SetItem (
            DWORD                               ColumnIndex,
            DWORD                               ItemIndex,
            LV_ITEM                             *pLVItem );
        BOOL DeleteAllItems ();
        DWORD GetItemCount ();
        DWORD FindItem (
            DWORD                               IndexToStart,
            LV_FINDINFO                         *pFindInfo );
        UINT GetItemState ( 
            DWORD                               ItemIndex,
            UINT                                Mask );
        BOOL SetItemState (
            DWORD                               ItemIndex,
            LV_ITEM                             *pLVItem );
        int GetItemText (
            DWORD                               ItemIndex,
            LV_ITEM                             *pLVItem );
        BOOL SetItemText (
            DWORD                               ItemIndex,
            LV_ITEM                             *pLVItem );
        int GetColumnWidth (
            DWORD                               ColumnIndex );
        BOOL SetColumnWidth (
            DWORD                               ColumnIndex,
            DWORD                               Width );
        BOOL Sort (
            PFNLVCOMPARE                        pfnCompare,
            LPARAM                              lParam );
        HWND GetEditControl ()
        {
            return ( hEditWnd );
        }
        BOOL SetItemCount (
            DWORD                               NewItemCount );
        BOOL GetViewRect (
            LPRECT                              pRect );
        BOOL GetOrigin (
            LPPOINT                             pPoint );
        int GetStringWidth (
            LPSTR                               pString );
        BOOL RedrawItems (
            DWORD                               StartIndex,
            DWORD                               FinishIndex,
            LPRECT                              pPaintRect = NULL );
        BOOL Update (
            DWORD                               ItemIndex,
            LPRECT                              pPaintRect = NULL );
        BOOL Scroll (
            int                                 dx,
            int                                 dy );

        
    private:

        HWND                                    hWnd;
        HWND                                    hHeaderWnd;
        DWORD                                   Style;
        int                                     ID;
        HFONT                                   hFont;
        COLORREF                                BKColor;
        HBRUSH                                  hBackgroundBrush;
        COLORREF                                TextBKColor;
        COLORREF                                TextColor;
        HIMAGELIST                              hLargeImageList;
        HIMAGELIST                              hSmallImageList;
        CWALVColumns                            *pColumns;
        HWND                                    hEditWnd;
        DWORD                                   CurrMaxWidth;
        DWORD                                   CurrHeight;
        BOOL                                    UserFirstColumnCreated;
        BOOL                                    CreatingDummyColumn;
        BOOL                                    CreatingFirstColumn;
        BOOL                                    HScroll;
        BOOL                                    VScroll;
        int                                     Track;
        int                                     TrackIndex;
        int                                     TrackOffset;
        POINT                                   NextLargeIconPoint;
        DWORD                                   CurrMaxIconWidth;
        DWORD                                   CurrMaxIconHeight;

        DWORD GetStringHeight ();
        DWORD GetItemStringWidth (
            DWORD                               ItemIndex );
        DWORD GetRowHeight ();
        DWORD GetItemsPerRow ();
        DWORD GetItemsPerColumn ();
        void LVColumnToHDItem (
            LV_COLUMN                           *pLVColumn,
            HD_ITEM                             *pHDItem );
        void SetFont ();
        void ScrollView (
            int                                 dx,
            int                                 dy );
        void CalcIconSizes ();


};

void WINAPI WInitListView();

#endif /*  #if defined ( __cplusplus ) */

#endif /*  #ifndef CWListView_H  */

