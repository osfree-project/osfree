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
#ifndef HELPLIST_H
#define HELPLIST_H

/***************************************************************************************
**
**
**                         Help Window List Functions' Include File
**
**
***************************************************************************************/


/*************************************
*
*  System Includes
*
**************************************/
#include <windows.h>



/*************************************
*
*  Window List Structure
*
**************************************/
typedef GLOBALHANDLE HWNDINFO;
typedef struct tagWNDINFO
{
  HWND hAppWnd;
  HWND hHelpWnd;
  HWNDINFO Next;
}
WNDINFO;
typedef WNDINFO __far * FPWNDINFO;



/*************************************
*
*  A help window has been opened.
*  Insert the new window's info into 
*  a help window info. list.
*                  
**************************************/
BOOL __far __pascal SaveAppWndInfo
( 
  HWND hErrorWnd, 
  HWNDINFO __far * hFirstWndInfo, 
  HWND hAppWnd, 
  HWND hHelpWnd 
);



/*************************************
*
*  Get an app. window's help window.
*                  
**************************************/
HWND __far __pascal GetAppsHelpWnd
( 
  HWNDINFO hFirstWndInfo, 
  HWND hAppWnd 
);



/*************************************
*
*  Remove and free an app window's
*  info. from a window list.
*                  
**************************************/
BOOL __far __pascal DeleteAppWndInfo
( 
  HWNDINFO __far * hFirstWndInfo, 
  HWND hAppWnd 
);



/*************************************
*
*  Removes and frees ALL window info. 
*  structures from a window info. list.
*                  
**************************************/
void __far __pascal DeleteAllWndInfo( HWNDINFO __far * hFirstWndInfo );



#endif
