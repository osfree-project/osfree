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
#ifndef HOTSPOTS_H
#define HOTSPOTS_H


/***********************************
**
**  System Includes
**
***********************************/
#include <windows.h>

/***********************************
**
**  Private Includes
**
***********************************/
#include "twinhelp.h"
#include "elements.h"


/***********************************
**
**  Shared Defines
**
***********************************/

typedef HGLOBAL HHOTSPOTBUFFER;


/***********************************
**
**  Shared Function Prototypes
**
***********************************/

HHOTSPOTBUFFER __far __pascal AllocHotSpotBuffer( HWND hWnd );
void  __far __pascal FreeHotSpotBuffer( HHOTSPOTBUFFER hHotSpotBuffer );
BOOL __far __pascal AddHotSpot
( 
  HWND hWnd, 
  HHOTSPOTBUFFER hHotSpotBuffer,
  FPHOTSPOTREC fpHotSpotRec,
  FPPOSRECT fpPosRect 
);
BOOL  __far __pascal IsPointInHotSpot
( 
  HHOTSPOTBUFFER hHotSpotBuffer, 
  POSPOINT PosPoint
);
BOOL  __far __pascal GetHotSpot
( 
  HHOTSPOTBUFFER hHotSpotBuffer,  
  POSPOINT PosPoint, 
  FPHOTSPOTREC fpHotSpotRec,
  FPPOSRECT HotSpotPosRect 
);
void  __far __pascal ClearPagesHotSpots( HHOTSPOTBUFFER hHotSpotBuffer );


#endif
