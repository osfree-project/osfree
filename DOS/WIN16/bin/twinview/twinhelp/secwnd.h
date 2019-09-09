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
#ifndef SECWND_H
#define SECWND_H

/***********************************
**
**  System Includes
**
***********************************/
#include <windows.h>


/*************************************
*
*  App. Includes
*
**************************************/
#include "filebuff.h"


/*************************************
*
*  App. Defines
*
**************************************/
typedef HGLOBAL HSECWNDINFO;


/***********************************
**
**  Shared Function Prototypes
**
***********************************/

void __far __pascal ShowSecWndTopic
(
  HWND hDataWnd, 
  HTOPICDATA hNewTopicsData,
  WORD wSecWndNum 
);
void __far __pascal CloseAllSecWnd( HWND hDataWnd );
void __far __pascal SecondaryWindowClosing( HWND hDataWnd, HWND hSecWnd );

BOOL __far __pascal SecWndExists( HWND hDataWnd );

BOOL __far __pascal DoMainSecWindow
(
  HWND hMainWnd, 
  HTOPICDATA hNewTopicsData,
  COLORREF __far * ScrollColor,
  COLORREF __far * NonScrollColor
);

void __far __pascal SetMainWindowTitle( HWND hWnd, HTOPICDATA hTopicData );

#endif




