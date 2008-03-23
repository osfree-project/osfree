/*    
	DisplayDC.c	2.9
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

#include "windows.h"

#include "GdiDC.h"
#include "Driver.h"

DWORD
lsd_display_createdc(WORD msg, HDC32 hDC32,
		DWORD dwParam, LPVOID lpStruct)
{
    /* calling the driver with NULL in the last parameter indicates */
    /* that the DC created is not associated with any specific window */

    if ((hDC32->lpDrvData = (LPVOID)DRVCALL_DC(PDCH_CREATE,
			hDC32->dwDCXFlags,0L,NULL)))
	return 1L;
    else
	return 0L;
}

DWORD
lsd_display_deletedc(WORD msg, HDC32 hDC32,
		DWORD dwParam, LPVOID lpStruct)
{
    return DRVCALL_DC(PDCH_DELETE,0L,0L,hDC32->lpDrvData);
}

DWORD
lsd_display_savedc(WORD msg, HDC32 hDC32,
		DWORD dwParam, LPVOID lpStruct)
{
    if (!hDC32->lpSavedDC)
	return 0L;

    /* Driver call will create and return a copy of internal */
    /* driver-specific data in the saved DC   */

    if (!(hDC32->lpDrvData = (LPVOID)DRVCALL_DC(PDCH_SAVE,0L,0L,
		hDC32->lpSavedDC->lpDrvData)))
	return 0L;

    return 1L;
}

DWORD
lsd_display_restoredc(WORD msg, HDC32 hDC32,
		DWORD dwParam, LPVOID lpStruct)
{
    /* Driver call will deallocate the driver-specific data in the DC;  */
    /* if dwParam is set, it is a second DCINFO struct to take driverdata */
    /* from and pass to the driver */

    return DRVCALL_DC(PDCH_RESTORE,
		((dwParam)?((HDC32)dwParam)->lpDrvData:0L),
		0L,     /* not used */
		hDC32->lpDrvData);
}

