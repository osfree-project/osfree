/* 
 
    @(#)mciDriverProc.c	1.2 - multimedia media control interface driver
  
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
#include <mmddk.h>

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

static LRESULT XXXX_mciOpen(DWORD dwDriverID, HDRVR hDriver,
	LPSTR lpszOpenParam, LPMCI_OPEN_DRIVER_PARMS lpMciOpenDriverParms)
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

static LRESULT XXXX_mciOpenDriver(DWORD dwDriverID, HDRVR hDriver,
	DWORD dwMciOpenFlags, LPMCI_OPEN_PARMS lpMciOpenParms)
{
	return (MCIERR_UNSUPPORTED_FUNCTION);
}

static LRESULT XXXX_mciCloseDriver(DWORD dwDriverID, HDRVR hDriver,
	DWORD dwMciGenericFlags, LPMCI_GENERIC_PARMS lpMciGenericParms)
{
	return (MCIERR_UNSUPPORTED_FUNCTION);
}

static LRESULT XXXX_mciGetDevCaps(DWORD dwDriverID, HDRVR hDriver,
	DWORD dwMciGetDevCapsFlags, LPMCI_GETDEVCAPS_PARMS lpMciGetDevCapsParms)
{
	return (MCIERR_UNSUPPORTED_FUNCTION);
}

static LRESULT XXXX_mciInfo(DWORD dwDriverID, HDRVR hDriver,
	DWORD dwMciInfoFlags, LPMCI_INFO_PARMS lpMciInfoParms)
{
	return (MCIERR_UNSUPPORTED_FUNCTION);
}

static LRESULT XXXX_mciStatus(DWORD dwDriverID, HDRVR hDriver,
	DWORD dwMciStatusFlags, LPMCI_STATUS_PARMS lpMciStatusParms)
{
	return (MCIERR_UNSUPPORTED_FUNCTION);
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

	LPMCI_OPEN_DRIVER_PARMS lpMciOpenDriverParms;
	DWORD dwMciOpenFlags;
	LPMCI_OPEN_PARMS lpMciOpenParms;
	DWORD dwMciGenericFlags;
	LPMCI_GENERIC_PARMS lpMciGenericParms;
	DWORD dwMciGetDevCapsFlags;
	LPMCI_GETDEVCAPS_PARMS lpMciGetDevCapsParms;
	DWORD dwMciInfoFlags;
	LPMCI_INFO_PARMS lpMciInfoParms;
	DWORD dwMciStatusFlags;
	LPMCI_STATUS_PARMS lpMciStatusParms;

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
		lpMciOpenDriverParms = (LPMCI_OPEN_DRIVER_PARMS) dwOpenParam;
		return XXXX_mciOpen(dwDriverID, hDriver,
			lpszOpenParam, lpMciOpenDriverParms);
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
	case MCI_OPEN_DRIVER:
		dwMciOpenFlags = (DWORD) lParam1;
		lpMciOpenParms = (LPMCI_OPEN_PARMS) lParam2;
		return XXXX_mciOpenDriver(dwDriverID, hDriver,
			dwMciOpenFlags, lpMciOpenParms);
	case MCI_CLOSE_DRIVER:
		dwMciGenericFlags = (DWORD) lParam1;
		lpMciGenericParms = (LPMCI_GENERIC_PARMS) lParam2;
		return XXXX_mciCloseDriver(dwDriverID, hDriver,
			dwMciGenericFlags, lpMciGenericParms);
	case MCI_GETDEVCAPS:
		dwMciGetDevCapsFlags = (DWORD) lParam1;
		lpMciGetDevCapsParms = (LPMCI_GETDEVCAPS_PARMS) lParam2;
		return XXXX_mciGetDevCaps(dwDriverID, hDriver,
			dwMciGetDevCapsFlags, lpMciGetDevCapsParms);
	case MCI_INFO:
		dwMciInfoFlags = (DWORD) lParam1;
		lpMciInfoParms = (LPMCI_INFO_PARMS) lParam2;
		return XXXX_mciInfo(dwDriverID, hDriver,
			dwMciInfoFlags, lpMciInfoParms);
	case MCI_STATUS:
		dwMciStatusFlags = (DWORD) lParam1;
		lpMciStatusParms = (LPMCI_STATUS_PARMS) lParam2;
		return XXXX_mciStatus(dwDriverID, hDriver,
			dwMciStatusFlags, lpMciStatusParms);
	}

	return DefDriverProc(dwDriverID, hDriver, uMsg, lParam1, lParam2);

}

