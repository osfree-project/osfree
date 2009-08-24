/* 
    @(#)DriverProc.c	1.2 - [XxXxXxXx] multimedia driver
  
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

#include <windows.h>
#include <mmsystem.h>

/* [XxXxXxXx] BEGIN ******************************************************** */

static LRESULT XXXX_Install(DWORD dwDriverID, HDRVR hDriver,
	LPDRVCONFIGINFO lpDrvConfigInfo)
{
	return (0);
}

static LRESULT XXXX_Remove(DWORD dwDriverID, HDRVR hDriver)
{
	return (0);
}

static LRESULT XXXX_Load(DWORD dwDriverID, HDRVR hDriver)
{
	return (0);
}

static LRESULT XXXX_Free(DWORD dwDriverID, HDRVR hDriver)
{
	return (0);
}

static LRESULT XXXX_Enable(DWORD dwDriverID, HDRVR hDriver)
{
	return (0);
}

static LRESULT XXXX_Disable(DWORD dwDriverID, HDRVR hDriver)
{
	return (0);
}

static LRESULT XXXX_Open(DWORD dwDriverID, HDRVR hDriver,
	LPSTR lpszOpenParam, DWORD dwOpenParam)
{
	return (0);
}

static LRESULT XXXX_Close(DWORD dwDriverID, HDRVR hDriver,
	DWORD dwCloseParam1, DWORD dwCloseParam2)
{
	return (0);
}

static LRESULT XXXX_QueryConfigure(DWORD dwDriverID, HDRVR hDriver)
{
	return (0);
}

static LRESULT XXXX_Configure(DWORD dwDriverID, HDRVR hDriver,
	HWND hConfigWndParent, LPDRVCONFIGINFO lpDrvConfigInfo)
{
	return (0);
}

/* [XxXxXxXx] END ********************************************************** */

LRESULT	CALLBACK
DriverProc(DWORD dwDriverID,
	HDRVR hDriver,
	UINT uMsg,
	LPARAM lParam1,
	LPARAM lParam2)
{
	LPDRVCONFIGINFO lpDrvConfigInfo;
	LPSTR lpszOpenParam;
	DWORD dwOpenParam;
	DWORD dwCloseParam1, dwCloseParam2;
	HWND hConfigWndParent;

	switch (uMsg)
	{
	case DRV_INSTALL:
		lpDrvConfigInfo = (LPDRVCONFIGINFO) lParam2;
		return XXXX_Install(dwDriverID, hDriver,
			lpDrvConfigInfo);
	case DRV_REMOVE:
		return XXXX_Remove(dwDriverID, hDriver);
	case DRV_LOAD:
		return XXXX_Load(dwDriverID, hDriver);
	case DRV_FREE:
		return XXXX_Free(dwDriverID, hDriver);
	case DRV_ENABLE:
		return XXXX_Enable(dwDriverID, hDriver);
	case DRV_DISABLE:
		return XXXX_Disable(dwDriverID, hDriver);
	case DRV_OPEN:
		lpszOpenParam = (LPSTR) lParam1;
		dwOpenParam = (DWORD) lParam2;
		return XXXX_Open(dwDriverID, hDriver,
			lpszOpenParam, dwOpenParam);
	case DRV_CLOSE:
		dwCloseParam1 = (DWORD) lParam1;
		dwCloseParam2 = (DWORD) lParam2;
		return XXXX_Close(dwDriverID, hDriver,
			dwCloseParam1, dwCloseParam2);
	case DRV_QUERYCONFIGURE:
		return XXXX_QueryConfigure(dwDriverID, hDriver);
	case DRV_CONFIGURE:
		hConfigWndParent = (HWND) lParam1;
		lpDrvConfigInfo = (LPDRVCONFIGINFO) lParam2;
		return XXXX_Configure(dwDriverID, hDriver,
			hConfigWndParent, lpDrvConfigInfo);
	}

	return DefDriverProc(dwDriverID, hDriver, uMsg, lParam1, lParam2);

}

