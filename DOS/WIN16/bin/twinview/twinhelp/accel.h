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
#ifndef ACCEL_H
#define ACCEL_H

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
                
typedef HGLOBAL HACCELSYS;
                
/***********************************
**
**  Shared Function Prototypes
**
***********************************/

BOOL __far __pascal StartAccelSupport( HWND hDataWnd );
void __far __pascal StopAccelSupport( HWND hDataWnd );
BOOL __far __pascal KeyIsPressed( HWND hDataWnd, WORD wKey );
BOOL __far __pascal AddAccelerator
( 
  HWND hDataWnd, 
  WORD wKey, 
  WORD wShiftState, 
  LPSTR lpMacro 
);
BOOL __far __pascal RemoveAccelerator
( 
  HWND hDataWnd, 
  WORD wKey, 
  WORD wShiftState
);
void __far __pascal RemoveAllAccelerators( HWND hDataWnd );


#endif

