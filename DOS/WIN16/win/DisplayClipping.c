/*    
	DisplayClipping.c	1.6
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

 */

#include <string.h>

#include "windows.h"

#include "GdiDC.h"
#include "GdiRegions.h"
#include "Log.h"
#include "Driver.h"
#include "DeviceData.h"

DWORD
lsd_display_getclipbox(WORD msg, HDC32 hDC32,
		DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    REGION32    Rgn32Clip;
    LPRECT lpRect = &lpStruct->lsde.rect;

    if ((Rgn32Clip = (REGION32)DRVCALL_DC(PDCH_GETCLIP,
		0L,0L,hDC32->lpDrvData)))
	return DRVCALL_REGIONS(PRH_REGIONBOX,Rgn32Clip,0,lpRect);
    else {
	/* No clipping region exists */
	if (hDC32->hWnd)
	    GetClientRect(hDC32->hWnd,lpRect);
	else
	    SetRect(lpRect,0,0,
		(int)DRVCALL_GRAPHICS(PGH_GETDEVICECAPS,hDC32->lpDrvData,
			HORZRES,0),
		(int)DRVCALL_GRAPHICS(PGH_GETDEVICECAPS,hDC32->lpDrvData,
			VERTRES,0));
	return SIMPLEREGION;
    }
}

DWORD
lsd_display_getcliprgn(WORD msg, HDC32 hDC32,
		DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    HRGN hRgn = lpStruct->lsde.region;
    REGION32 Rgn32,Rgn32Clip;

    if (!(Rgn32 = TWIN_InternalGetRegionData(hRgn)))
	return -1;

    if ((Rgn32Clip = (REGION32)DRVCALL_DC(PDCH_GETCLIP,
			0L,0L,hDC32->lpDrvData))) {
	(void)DRVCALL_REGIONS(PRH_COPYREGION,Rgn32Clip,Rgn32,0);
	return 1;
    }
    else {
	(void)DRVCALL_REGIONS(PRH_SETREGIONEMPTY,Rgn32,0,0);
	return 0;
    }
}

