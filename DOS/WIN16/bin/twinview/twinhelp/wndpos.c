/***********************************************************************

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

***********************************************************************/
/***************************************************************************************
**
**
**                             Window Position Functions
**
**
***************************************************************************************/
/*************************************
*
*  System Includes
*
**************************************/

/*************************************
*
*  Custom Includes
*
**************************************/
#include "wndpos.h"

/*************************************
*
*  Private Defines
*
**************************************/


/*************************************
*
*  Function Prototypes
*
**************************************/

static void __far __pascal GetWindowSize( HWND hWnd, RECT __far * RectPtr );


/*************************************
*
*  Get a window's position and
*  convert it into the virtual
*  coordinate space we use.
*
**************************************/
void __far __pascal MakeWndVirtualPos( HWND hWnd, WINDOWPLACEMENT __far * WndPlacePtr )
{
  HDC hDC;
  short int OldMappingMode;
  DWORD     OldWindowExt;
  DWORD     OldViewportExt;
  
  
  /* Get the position of the window. */  
  WndPlacePtr->length = sizeof(WINDOWPLACEMENT);
  GetWindowPlacement( hWnd, WndPlacePtr );
  
  if( WndPlacePtr->showCmd != SW_SHOWMAXIMIZED )
  {  
    /* Get DC for desktop. */
    hDC = GetDC( hWnd );
      
    /* Set the window's mapping mode. */
    OldMappingMode = SetMapMode( hDC, MM_ANISOTROPIC );
    OldWindowExt = SetWindowExt( hDC, 1024, 1024 );
    OldViewportExt = SetViewportExt( hDC, GetDeviceCaps( hDC, HORZRES ),
                                     GetDeviceCaps( hDC, VERTRES ) );
      
    /* Convert to virtual coordinates. */
    DPtoLP( hDC, (POINT __far *) &(WndPlacePtr)->rcNormalPosition, 2 );
    
    /* Restore the window's mapping mode. */
    SetMapMode( hDC, OldMappingMode );
    SetWindowExt( hDC, LOWORD(OldWindowExt), HIWORD(OldWindowExt) );
    SetViewportExt( hDC, LOWORD(OldViewportExt), HIWORD(OldViewportExt) );
    
    /* Release the DC. */
    ReleaseDC( hWnd, hDC );
  
    /* Change rect's right and bottom to width and height. */
    WndPlacePtr->rcNormalPosition.right = WndPlacePtr->rcNormalPosition.right -
                                          WndPlacePtr->rcNormalPosition.left;
    WndPlacePtr->rcNormalPosition.bottom = WndPlacePtr->rcNormalPosition.bottom -
                                           WndPlacePtr->rcNormalPosition.top;
  }
}


/*************************************
*
*  Get a window's virtual
*  coordinate position into physcial
*  screen coordinates.
*
**************************************/
void __far __pascal VirtualPosToScrnPos( HWND hWnd, RECT __far * RectPtr )
{
  HDC hDC;
  short int OldMappingMode;
  DWORD     OldWindowExt;
  DWORD     OldViewportExt;
  
  /* Get DC for desktop. */
  hDC = GetDC( hWnd );
  
  /* Set the window's mapping mode. */
  OldMappingMode = SetMapMode( hDC, MM_ANISOTROPIC );
  OldWindowExt = SetWindowExt( hDC, 1024, 1024 );
  OldViewportExt = SetViewportExt( hDC, GetDeviceCaps( hDC, HORZRES ),
                                        GetDeviceCaps( hDC, VERTRES ) );
  
  /* Convert to virtual coordinates. */
  LPtoDP( hDC, (POINT __far *) RectPtr, 2 );

  /* Restore the window's mapping mode. */
  SetMapMode( hDC, OldMappingMode );
  SetWindowExt( hDC, LOWORD(OldWindowExt), HIWORD(OldWindowExt) );
  SetViewportExt( hDC, LOWORD(OldViewportExt), HIWORD(OldViewportExt) );

  /* Release the DC. */
  ReleaseDC( hWnd, hDC );
}


