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
#ifndef BTNBAR_H
#define BTNBAR_H

/***********************************
**
**  System Includes
**
***********************************/

#include <windows.h>


/***********************************
**
**  Shared Defines
**
***********************************/

#define MAX_BUTTON_ID     51  /* Max. length of button ID string. */
#define MAX_BUTTON_TEXT   30  /* Max. length of button text string. */


/***********************************
**
**  Shared Function Prototypes
**
***********************************/

LONG FAR PASCAL ButtonWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

BOOL __far __pascal RegisterButtonBarClass( HINSTANCE hInstance );

BOOL __far __pascal AddBarButton
( 
  HINSTANCE hInstance,
  HWND hButtonBarWnd, 
  LPSTR lpButtonText, 
  LPSTR lpButtonID, 
  LPSTR lpMacro 
);

BOOL __far __pascal DeleteBarButton
( 
  HWND hButtonBarWnd, 
  LPSTR lpButtonID
);

BOOL __far __pascal ChangeButtonsMacro
( 
  HWND hButtonBarWnd, 
  LPSTR lpButtonID,
  LPSTR lpMacro 
);

BOOL __far __pascal EnableButton
( 
  HWND  hButtonBarWnd, 
  LPSTR lpButtonID,
  BOOL  bEnable 
);

HWND __far __pascal CreateButtonBarWnd( HWND hDataWnd, HWND hParentWnd, HINSTANCE hInstance );

HWND __far __pascal GetButtonWindow( HWND hButtonBarWnd, LPSTR lpButtonID );

void __far __pascal RemoveAllButtons( HWND hButtonBarWnd );

#endif

