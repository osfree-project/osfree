/*==============================================================================
//
//  @(#)WWindow.cpp	1.2
//
//  Copyright (c) 1996 Willows Software Inc. All Rights Reserved
//
//============================================================================*/




#include "WCommCtrl.h"
#include "WWindow.h"



CWWindow::CWWindow ()

{

    hWnd = ( HWND )NULL;

    XOrigin     =
    YOrigin     =
    XRange      = 
    YRange      =
    XExtent     =
    YExtent     = 0;

}



CWWindow::~CWWindow ()

{


}

LRESULT CWWindow::WDefWindowProc (
    HWND                                hCurrWnd,
    UINT                                Message,
    WPARAM                              wParam,
    LPARAM                              lParam )

{

    switch ( Message )
    {
        case WM_CREATE:
            OnWM_CREATE ( hCurrWnd, ( LPCREATESTRUCT )lParam );
            break;

        case WM_SIZE:
            OnWM_SIZE ( wParam, ( int )LOWORD ( lParam ), ( int )HIWORD ( lParam ) );
            break;
    }
    return ( DefWindowProc ( hCurrWnd, Message, wParam, lParam ) );

}


void CWWindow::OnWM_CREATE (
    HWND                                hCreatedWindow,
    LPCREATESTRUCT                      pCreateStruct )

{

    hWnd = hCreatedWindow;      

}



void CWWindow::OnWM_SIZE (
    WPARAM                              Flag,
    int                                 Width,
    int                                 Height )

{

    switch ( Flag )
    {
        default:
            SetExtent ( GetXExtent(), GetYExtent(), Width, Height );
        case SIZE_MINIMIZED:
            break;
    }

}


void CWWindow::UpdateScrollBar (
    UINT                                ScrollBar,
    int                                 Origin,
    int                                 Range,
    int                                 Extent )

{

    SetScrollRange ( hWnd, ScrollBar, 0, Extent, FALSE );
    SetScrollPos ( hWnd, ScrollBar, MulDiv ( Origin, Extent, Range ? Range : Extent ), TRUE );
    ShowScrollBar ( hWnd, ScrollBar, TRUE );

}

void CWWindow::SetExtent (
    int                                 X,
    int                                 Y,
    int                                 Width,
    int                                 Height )

{

    if ( X != CWWD_CURRENT )
        XExtent = X;

    if ( Y != CWWD_CURRENT )
        YExtent = Y;

    ShowScrollBars ( Width, Height );

}



void CWWindow::SetOrigin (
    int                                 X,
    int                                 Y,
    BOOL                                Redraw )

{

    if ( X != CWWD_CURRENT )
        XOrigin = X;

    if ( Y != CWWD_CURRENT )
        YOrigin = Y;

    if ( Redraw )
        ShowScrollBars ();

}



void CWWindow::SetRange (
    int                                 X,
    int                                 Y,
    BOOL                                Redraw )

{

    if ( X != CWWD_CURRENT )
        XRange = X;

    if ( Y != CWWD_CURRENT )
        YRange = Y;

    if ( Redraw )
        ShowScrollBars ();

}



void CWWindow::ShowScrollBars (
    int                                 Width,
    int                                 Height )

{

    RECT                                ClientRect;


    if ( ( ! Width ) || ( ! Height ) )
    {
        GetClientRect ( hWnd, &ClientRect );
        Width  = ClientRect.right - ClientRect.left;
        Height = ClientRect.bottom - ClientRect.top;
    }

    if ( ( XExtent > Width ) && ( YExtent > Height ) )
    {
        ShowScrollBar ( hWnd, SB_BOTH, TRUE );
        UpdateScrollBar ( SB_VERT, YOrigin, YRange, YExtent );
        UpdateScrollBar ( SB_HORZ, XOrigin, XRange, XExtent );
    }
    else
    if ( XExtent > Width )
    {
        ShowScrollBar ( hWnd, SB_VERT, FALSE );
        UpdateScrollBar ( SB_HORZ, XOrigin, XRange, XExtent );
    }
    else
    if ( YExtent > Height )
    {
        ShowScrollBar ( hWnd, SB_HORZ, FALSE );
        UpdateScrollBar ( SB_VERT, YOrigin, YRange, YExtent );
    }
    else
    {
        SetScrollRange ( hWnd, SB_BOTH, 0, 0, TRUE );
        ShowScrollBar ( hWnd, SB_VERT, FALSE );
        ShowScrollBar ( hWnd, SB_HORZ, FALSE );
    }

}

