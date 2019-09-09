
/*  WHeader.h
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



#ifndef CWHEADER_H
#define CWHEADER_H



#if defined ( _WINDOWS )
#define WHD_CLASSNAME                   "WSysHeader32"
#else
#define WHD_CLASSNAME                   "SysHeader32"
#endif


#if defined ( __cplusplus )

class CWAHDItem;



class CWHeader
{

    public:

        CWHeader (
            HWND                                hWnd,
            LPCREATESTRUCT                      pCreateStruct );
        virtual ~CWHeader ();

        LRESULT Paint (
            HDC                                 hDC );
        LRESULT OnLButtonDown (
            UINT                                KeyStatus,
            int                                 X,
            int                                 Y );
        LRESULT OnMouseMove (
            UINT                                KeyStatus,
            int                                 X,
            int                                 Y );
        LRESULT OnLButtonUp (
            UINT                                KeyStatus,
            int                                 X,
            int                                 Y );
        LRESULT DeleteItem (
            DWORD                               ItemIndex );
        LRESULT GetItem (
            DWORD                               ItemIndex,
            HD_ITEM                             *pItem );
        LRESULT GetItemCount ();
        LRESULT InsertItem (
            DWORD                               ItemIndex,
            HD_ITEM                             *pItem );
        LRESULT Layout (
            HD_LAYOUT                           *pLayout );
        LRESULT SetItem (
            DWORD                               ItemIndex,
            HD_ITEM                             *pItem );


    private:

        HWND                                    hWnd;
        DWORD                                   Style;
        int                                     ID;
        CWAHDItem                               *pHDItems;
        BOOL                                    Captured;
        DWORD                                   Index;
        DWORD                                   Offset;
        BOOL                                    Down;
        int                                     OldX;

        LRESULT Notify (
            HD_NOTIFY                           *pNotify )
        {
            return ( SendMessage ( GetParent ( hWnd ), WM_NOTIFY, ( WPARAM )ID, ( LPARAM )pNotify ) );
        }

        void DrawBitmap (
            HDC                                 hDC,
            HBITMAP                             hBitmap,
            LPRECT                              pRect,
            int                                 Format );
        LRESULT PaintItem (
            DWORD                               ItemIndex,
            HDC                                 hDC = ( HDC )NULL,
            DWORD                               *pOffset = NULL );


};

void WINAPI WInitHeader();

#endif /* #if defined ( __cplusplus ) */

#endif /* #ifndef CWHeader_H */
