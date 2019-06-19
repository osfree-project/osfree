
/*  WWindow.h	1.1
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

#ifndef CWWINDOW_H
#define CWWINDOW_H


#define CWWD_CURRENT                            (-1)


#if defined ( __cplusplus )

class CWWindow
{

    public:

        HWND                                    hWnd;

        CWWindow ();
        ~CWWindow ();


        virtual void OnWM_CREATE (
            HWND                                hCreatedWindow,
            LPCREATESTRUCT                      pCreateStruct );
        virtual void OnWM_SIZE (
            WPARAM                              Flag,
            int                                 Width,
            int                                 Height );

        virtual void SetExtent (
            int                                 X,
            int                                 Y,
            int                                 Width = 0,
            int                                 Height = 0 );
        void SetXExtent (
            int                                 X )
        {
            SetExtent ( X, CWWD_CURRENT );
        }
        void SetYExtent (
            int                                 Y )
        {
            SetExtent ( CWWD_CURRENT, Y );
        }
        long GetExtent ()
        {
            return ( MAKELONG ( ( short )XExtent, ( short )YExtent ) );
        }
        int GetXExtent ()
        {
            return ( XExtent );
        }
        int GetYExtent ()
        {
            return ( YExtent );
        }
        virtual void SetOrigin (
            int                                 X,
            int                                 Y,
            BOOL                                Redraw = TRUE );
        void SetXOrigin (
            int                                 X,
            BOOL                                Redraw = TRUE )
        {
            SetOrigin ( X, CWWD_CURRENT, Redraw );
        }
        void SetYOrigin (
            int                                 Y,
            BOOL                                Redraw )
        {
            SetOrigin ( CWWD_CURRENT, Y, Redraw );
        }
        long GetOrigin ()
        {
            return ( MAKELONG ( ( short )XOrigin, ( short )YOrigin ) );
        }
        int GetXOrigin ()
        {
            return ( XOrigin );
        }
        int GetYOrigin ()
        {
            return ( YOrigin );
        }
        virtual void SetRange (
            int                                 X,
            int                                 Y,
            BOOL                                Redraw = TRUE );
        void SetXRange (
            int                                 X,
            BOOL                                Redraw = TRUE )
        {
            SetRange ( X, CWWD_CURRENT, Redraw );
        }
        void SetYRange (
            int                                 Y,
            BOOL                                Redraw )
        {
            SetRange ( CWWD_CURRENT, Y, Redraw );
        }
        long GetRange ()
        {
            return ( MAKELONG ( ( short )XRange, ( short )YRange ) );
        }
        int GetXRange ()
        {
            return ( XRange );
        }
        int GetYRange ()
        {
            return ( YRange );
        }
        virtual LRESULT WDefWindowProc (
            HWND                                hCurrWnd,
            UINT                                Message,
            WPARAM                              wParam,
            LPARAM                              lParam );


    private:

        int                                     XOrigin;
        int                                     YOrigin;
        int                                     XExtent;
        int                                     YExtent;
        int                                     XRange;
        int                                     YRange;

        void UpdateScrollBar (
            UINT                                ScrollBar,
            int                                 Origin,
            int                                 Range,
            int                                 Extent );
        void ShowScrollBars (
            int                                 Width  = 0,
            int                                 Height = 0 );

};


extern "C"
{
#endif /* #if defined ( __cplusplus ) */



#if defined ( __cplusplus )
}
#endif /* #if defined ( __cplusplus ) */



#endif /* #ifndef CWWINDOW_H */
