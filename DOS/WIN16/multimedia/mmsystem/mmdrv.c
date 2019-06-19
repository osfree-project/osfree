/*     
	mmdrv.c	1.2 multimedia installable driver functions*
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
#include "mmsystem.h"

/* [INTERNAL] (MMSYSTEM) Installable Driver ******************************** */

#define DRVR_SIGNATURE		0x4452

typedef struct {
	UINT		uDriverSignature;
	HINSTANCE	hDriverModule;
	DRIVERPROC	DriverProc;
	DWORD		dwDriverID;
} DRVR;
typedef DRVR		*PDRVR;
typedef DRVR NEAR	*NPDRVR;
typedef DRVR FAR	*LPDRVR;

static DWORD dwDrvID = 0;

static NPDRVR DrvAlloc(HDRVR FAR *lpDriver, LPUINT lpDrvResult)
{
	NPDRVR npDriver;

	/* allocate and lock handle */
	if (lpDriver)
	{
		if ((*lpDriver = (HDRVR) LocalAlloc(LHND, sizeof(DRVR))))
		{
			if ((npDriver = (NPDRVR) LocalLock((HLOCAL) *lpDriver)))
			{
				*lpDrvResult = MMSYSERR_NOERROR;
				return (npDriver);
			}
			LocalFree((HLOCAL) *lpDriver);
		}
		return (*lpDrvResult = MMSYSERR_NOMEM, (NPDRVR) 0);
	}
	return (*lpDrvResult = MMSYSERR_INVALPARAM, (NPDRVR) 0);

}

static void DrvFree(HDRVR hDriver)
{
	/* free handle */
	if (hDriver)
		LocalFree((HLOCAL) hDriver);
}

static NPDRVR DrvLock(HDRVR hDriver, LPUINT lpDrvResult)
{
	NPDRVR npDriver;

	/* lock handle (and verify handle signature) */
	if (hDriver && (npDriver = (NPDRVR) LocalLock((HLOCAL) hDriver)))
	{
		if (npDriver->uDriverSignature == DRVR_SIGNATURE)
		{
			*lpDrvResult = MMSYSERR_NOERROR;
			return (npDriver);
		}
		LocalUnlock((HLOCAL) hDriver);
	}
	return (*lpDrvResult = MMSYSERR_INVALHANDLE, (NPDRVR) 0);

}

static void DrvUnlock(HDRVR hDriver)
{
	/* unlock handle */
	if (hDriver)
		LocalUnlock((HLOCAL) hDriver);
}

/* [API] (MMSYSTEM) Installable Driver ************************************* */

HDRVR	WINAPI
DrvOpen(LPCSTR lpszDriverName, LPCSTR lpszSectionName, LPARAM lParam2)
{
	char filename[MAX_PATH], *f;
	UINT uDrvResult;
	HDRVR hDriver;
	NPDRVR npDriver;

	/* get driver filename */
	if (!GetPrivateProfileString(lpszSectionName
		? lpszSectionName : "drivers", lpszDriverName, "",
		filename, sizeof(filename), "system.ini"))
		return ((HDRVR) 0);
	for (f = filename; *f; f++)
	{
		/* extract trailing driver arguments
		 *
		 * system.ini
		 * ----------
		 * [drivers]
		 * drivername1=filename1 args
		 * drivername2=filename2 args
		 * ...
		 *
		 */
		if (*f == ' ')
		{
			*f++ = '\0';
			break;
		}
	}
	if (!*f)
		f = (char *) 0;

	/* allocate and lock handle */
	if (!(npDriver = DrvAlloc(&hDriver, &uDrvResult)))
		return ((HDRVR) 0);
	npDriver->uDriverSignature = DRVR_SIGNATURE;
	if (!(npDriver->hDriverModule = LoadLibrary(filename)))
	{
		DrvUnlock(hDriver);
		DrvFree(hDriver);
		return ((HDRVR) 0);
	}
	if (!(npDriver->DriverProc = (DRIVERPROC)
		GetProcAddress(npDriver->hDriverModule, "DriverProc")))
	{
		FreeLibrary(npDriver->hDriverModule);
		DrvUnlock(hDriver);
		DrvFree(hDriver);
		return ((HDRVR) 0);
	}
	npDriver->dwDriverID = ++dwDrvID;

	/* load driver */
	if (GetModuleUsage(npDriver->hDriverModule) == 1)
	{
		(npDriver->DriverProc)(0, hDriver, DRV_LOAD, 0, 0);
	}

	/* enable driver */
	if (GetModuleUsage(npDriver->hDriverModule) == 1)
	{
		(npDriver->DriverProc)(0, hDriver, DRV_ENABLE, 0, 0);
	}

	/* open driver */
	(npDriver->DriverProc)(npDriver->dwDriverID, hDriver, DRV_OPEN,
		(LPARAM) (LPSTR) f, lParam2);

	/* unlock handle (and free handle if error) */
	DrvUnlock(hDriver);
	if (uDrvResult)
	{
		DrvFree(hDriver);
		hDriver = (HDRVR) 0;
	}
	return (hDriver);

}

LRESULT	WINAPI
DrvClose(HDRVR hDriver, LPARAM lParam1, LPARAM lParam2)
{
	UINT uDrvResult;
	NPDRVR npDriver;

	/* lock handle */
	if (!(npDriver = DrvLock(hDriver, &uDrvResult)))
		return (0);

	/* close driver */
	(npDriver->DriverProc)(npDriver->dwDriverID, hDriver, DRV_CLOSE,
		lParam1, lParam2);

	/* disable driver */
	if (GetModuleUsage(npDriver->hDriverModule) == 1)
	{
		(npDriver->DriverProc)(0, hDriver, DRV_DISABLE, 0, 0);
	}

	/* free driver */
	if (GetModuleUsage(npDriver->hDriverModule) == 1)
	{
		(npDriver->DriverProc)(0, hDriver, DRV_FREE, 0, 0);
	}

	/* unlock handle and free handle */
	FreeLibrary(npDriver->hDriverModule);
	DrvUnlock(hDriver);
	DrvFree(hDriver);
	return (1);

}

HINSTANCE	WINAPI
DrvGetModuleHandle(HDRVR hDriver)
{
	UINT uDrvResult;
	NPDRVR npDriver;
	HINSTANCE hModule;

	/* get driver module handle */
	if ((npDriver = DrvLock(hDriver, &uDrvResult)))
	{
		hModule = npDriver->hDriverModule;
		DrvUnlock(hDriver);
		return (hModule);
	}
	return ((HINSTANCE) 0);

}

LRESULT	WINAPI
DrvSendMessage(HDRVR hDriver, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
	UINT uDrvResult;
	NPDRVR npDriver;
	LRESULT lDrvResult;

	/* send message */
	if ((npDriver = DrvLock(hDriver, &uDrvResult)))
	{
		lDrvResult = (npDriver->DriverProc)(npDriver->dwDriverID,
			hDriver, uMsg, lParam1, lParam2);
		DrvUnlock(hDriver);
		return (lDrvResult);
	}
	return (0);

}

LRESULT	WINAPI
DrvDefDriverProc(DWORD dwDriverID, HDRVR hDriver, UINT uMsg,
	LPARAM lParam1, LPARAM lParam2)
{
	/* default failure code */
	return (0);
}

HDRVR	WINAPI
OpenDriver(LPCSTR lpszDriverName, LPCSTR lpszSectionName, LPARAM lParam2)
{
	/* same function as DrvOpen() */
	return DrvOpen(lpszDriverName, lpszSectionName, lParam2);
}

LRESULT	WINAPI
CloseDriver(HDRVR hDriver, LPARAM lParam1, LPARAM lParam2)
{
	/* same function as DrvClose() */
	return DrvClose(hDriver, lParam1, lParam2);
}

HMODULE	WINAPI
GetDriverModuleHandle(HDRVR hDriver)
{
	/* same function as DrvGetModuleHandle() */
	return DrvGetModuleHandle(hDriver);
}

LRESULT	WINAPI
SendDriverMessage(HDRVR hDriver, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
	/* same function as DrvSendMessage() */
	return DrvSendMessage(hDriver, uMsg, lParam1, lParam2);
}

LRESULT	WINAPI
DefDriverProc(DWORD dwDriverID, HDRVR hDriver, UINT uMsg,
	LPARAM lParam1, LPARAM lParam2)
{
	/* same function as DrvDefDriverProc() */
	return DrvDefDriverProc(dwDriverID, hDriver, uMsg, lParam1, lParam2);
}

