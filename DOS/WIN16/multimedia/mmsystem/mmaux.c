/*    
	mmaux.c	1.1 mmaux.c - multimedia auxiliary audio functions*
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
#include "mmddk.h"

/* [INTERNAL] (MMSYSTEM) Auxiliary Audio *********************************** */

typedef DWORD (FAR *AUXPROC)(UINT uDeviceID, UINT uMsg,
	DWORD dwUser, DWORD dwParam1, DWORD dwParam2);

static UINT uNumAuxDrivers = 0;
static HDRVR hAuxDrivers[10];
static UINT uNumAuxDevices[10];
static AUXPROC auxMessage[10];

static UINT auxOpenDrivers()
{
	/* open device drivers specified in SYSTEM.INI file
	 *
	 * single device driver
	 * --------------------
	 * [drivers]
	 * aux=...
	 *
	 * multiple device drivers
	 * -----------------------
	 * [drivers]
	 * aux1=...
	 * aux2=...
	 * ...
	 *
	 */
	static char aux[5] = "aux?";
	UINT u;
	HINSTANCE hAuxDriverModule;

	/* open device drivers once */
	if (uNumAuxDrivers)
		return (uNumAuxDrivers);

	/* for each device driver
	 *	- open device driver
	 *	- find auxMessage() function
	 *	- call auxMessage() function to get number of devices
	 */
	for (u = uNumAuxDrivers = 0; u <= 9; u++)
	{
		aux[sizeof(aux)-2] = "\000123456789"[u];
		if ((hAuxDrivers[uNumAuxDrivers] =
			OpenDriver(aux, NULL, 0L))) {
		   hAuxDriverModule =
			GetDriverModuleHandle(hAuxDrivers[uNumAuxDrivers]);
		   auxMessage[uNumAuxDrivers] = (AUXPROC)
			GetProcAddress(hAuxDriverModule, "auxMessage");
		   uNumAuxDevices[uNumAuxDrivers] =
			(auxMessage[uNumAuxDrivers])(0, AUXDM_GETNUMDEVS,
			0, 0, 0);
		   uNumAuxDrivers++;
	        }

		if (u != uNumAuxDrivers)
			break;
	}
	return (uNumAuxDrivers);

}

static UINT auxDeviceMapper(UINT uAuxDeviceID)
{
	static UINT uAuxDeviceMapperID = AUX_MAPPER;
	UINT u, v, w;
	AUXCAPS ac;

	/* open device drivers */
	if (!uNumAuxDrivers && !auxOpenDrivers())
		return (uAuxDeviceID);

	/* get device mapper ID */
	if (uAuxDeviceID == AUX_MAPPER)
	{
		if (uAuxDeviceMapperID == AUX_MAPPER)
		{
			for (u = w = 0; u < uNumAuxDrivers; u++)
			{
				for (v = 0; v < uNumAuxDevices[u]; v++, w++)
				{
					if ((auxMessage[u])(v, AUXDM_GETDEVCAPS,
						0,
						(DWORD) (LPAUXCAPS) &ac,
						(DWORD) sizeof(ac)))
						continue;
					if (ac.wPid == /* MM_ */ AUX_MAPPER)
						uAuxDeviceMapperID = w;
				}
			}
		}
		uAuxDeviceID = uAuxDeviceMapperID;
	}
	return (uAuxDeviceID);

}

static UINT auxDeviceMessage(UINT uAuxDeviceID, UINT uMsg,
	DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
	UINT u;

	/* open device drivers */
	if (!uNumAuxDrivers && !auxOpenDrivers())
		return (MMSYSERR_NODRIVER);

	/* get device mapper ID */
	if ((uAuxDeviceID = auxDeviceMapper(uAuxDeviceID)) == AUX_MAPPER)
		return (MMSYSERR_BADDEVICEID);

	/* send message to device driver */
	for (u = 0; u < uNumAuxDrivers; u++)
	{
		if (uAuxDeviceID >= uNumAuxDevices[u])
		{
			uAuxDeviceID -= uNumAuxDevices[u];
			continue;
		}
		return (auxMessage[u])(uAuxDeviceID, uMsg,
			dwUser, dwParam1, dwParam2);
	}
	return (MMSYSERR_BADDEVICEID);

}

/* [API] (MMSYSTEM) Auxiliary Audio **************************************** */

UINT	WINAPI
auxGetNumDevs()
{
	UINT u, uAuxNumDevs;

	/* open device drivers */
	if (!uNumAuxDrivers && !auxOpenDrivers())
		return (0);

	/* count total number of devices supported by device drivers */
	for (u = uAuxNumDevs = 0; u < uNumAuxDrivers; u++)
		uAuxNumDevs += uNumAuxDevices[u];
	return (uAuxNumDevs);

}

UINT	WINAPI
auxGetDevCaps(UINT uAuxDeviceID, LPAUXCAPS lpAuxCaps, UINT uAuxCapsSize)
{
	return auxOutMessage(uAuxDeviceID, AUXDM_GETDEVCAPS,
		(DWORD) lpAuxCaps, (DWORD) uAuxCapsSize);
}

UINT	WINAPI
auxGetVolume(UINT uAuxDeviceID, LPDWORD lpAuxVolume)
{
	return auxOutMessage(uAuxDeviceID, AUXDM_GETVOLUME,
		(DWORD) lpAuxVolume, 0);
}

UINT	WINAPI
auxSetVolume(UINT uAuxDeviceID, DWORD dwAuxVolume)
{
	return auxOutMessage(uAuxDeviceID, AUXDM_SETVOLUME,
		dwAuxVolume, 0);
}

DWORD	WINAPI
auxOutMessage(UINT uAuxDeviceID, UINT uMsg, DWORD dwParam1, DWORD dwParam2)
{
	return auxDeviceMessage(uAuxDeviceID, uMsg, 0, dwParam1, dwParam2);
}

