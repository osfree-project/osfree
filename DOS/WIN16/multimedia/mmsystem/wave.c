/*     
	wave.c	1.2 multimedia waveform audio functions
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

/* [INTERNAL] (MMSYSTEM) Wave ********************************************** */

typedef DWORD (FAR *WIDPROC)(UINT uDeviceID, UINT uMsg,
	DWORD dwUser, DWORD dwParam1, DWORD dwParam2);

typedef DWORD (FAR *WODPROC)(UINT uDeviceID, UINT uMsg,
	DWORD dwUser, DWORD dwParam1, DWORD dwParam2);

static UINT uNumWaveDrivers = 0;
static HDRVR hWaveDrivers[10];

static UINT uNumWaveInDrivers = 0;
static UINT uNumWaveInDevices[10];
static WIDPROC widMessage[10];

static UINT uNumWaveOutDrivers = 0;
static UINT uNumWaveOutDevices[10];
static WODPROC wodMessage[10];

static UINT waveOpenDrivers()
{
	/* open device drivers specified in SYSTEM.INI file
	 *
	 * single device driver
	 * --------------------
	 * [drivers]
	 * wave=...
	 *
	 * multiple device drivers
	 * -----------------------
	 * [drivers]
	 * wave1=...
	 * wave2=...
	 * ...
	 *
	 */
	static char wave[6] = "wave?";
	UINT u;

	/* open device drivers once */
	if (uNumWaveDrivers)
		return (uNumWaveDrivers);

	/* for each device driver
	 *	- open device driver
	 */
	for (u = uNumWaveDrivers = 0; u <= 9; u++)
	{
		wave[sizeof(wave)-2] = "\000123456789"[u];
		if((hWaveDrivers[uNumWaveDrivers] = OpenDriver(wave, NULL, 0L)))
			uNumWaveDrivers++;
		if (u != uNumWaveDrivers)
			break;
	}
	return (uNumWaveDrivers);

}

static UINT waveInOpenDrivers()
{
	UINT u;
	HINSTANCE hWaveDriverModule;

	/* open device drivers */
	if (!uNumWaveDrivers && !waveOpenDrivers())
		return (0);

	/* for each device driver
	 *	- find widMessage() function
	 *	- call widMessage() function to get number of devices
	 */
	for (u = uNumWaveInDrivers = 0; u < uNumWaveDrivers; u++)
	{
		if ((hWaveDriverModule =
			GetDriverModuleHandle(hWaveDrivers[u])) &&
		    (widMessage[uNumWaveInDrivers] = (WIDPROC)
			GetProcAddress(hWaveDriverModule, "widMessage")) &&
		    (uNumWaveInDevices[uNumWaveInDrivers] =
			(widMessage[uNumWaveInDrivers])(0, WIDM_GETNUMDEVS,
			0, 0, 0))) 

			uNumWaveInDrivers++;
	}
	return (uNumWaveInDrivers);

}

static UINT waveOutOpenDrivers()
{
	UINT u;
	HINSTANCE hWaveDriverModule;

	/* open device drivers */
	if (!uNumWaveDrivers && !waveOpenDrivers())
		return (0);

	/* for each device driver
	 *	- find wodMessage() function
	 *	- call wodMessage() function to get number of devices
	 */
	for (u = uNumWaveOutDrivers = 0; u < uNumWaveDrivers; u++)
	{
		if ((hWaveDriverModule =
			GetDriverModuleHandle(hWaveDrivers[u])) &&
		    (wodMessage[uNumWaveOutDrivers] = (WODPROC)
			GetProcAddress(hWaveDriverModule, "wodMessage")) &&
		    (uNumWaveOutDevices[uNumWaveOutDrivers] =
			(wodMessage[uNumWaveOutDrivers])(0, WODM_GETNUMDEVS,
			0, 0, 0)))

		    uNumWaveOutDrivers++;
	}
	return (uNumWaveOutDrivers);

}

static UINT waveInDeviceMapper(UINT uWaveInDeviceID)
{
	static UINT uWaveInDeviceMapperID = WAVE_MAPPER;
	UINT u, v, w;
	WAVEINCAPS wic;

	/* open device drivers */
	if (!uNumWaveInDrivers && !waveInOpenDrivers())
		return (uWaveInDeviceID);

	/* get device mapper ID */
	if (uWaveInDeviceID == WAVE_MAPPER)
	{
		if (uWaveInDeviceMapperID == WAVE_MAPPER)
		{
			for (u = w = 0; u < uNumWaveInDrivers; u++)
			{
				for (v = 0; v < uNumWaveInDevices[u]; v++, w++)
				{
					if ((widMessage[u])(v, WIDM_GETDEVCAPS,
						0,
						(DWORD) (LPWAVEINCAPS) &wic,
						(DWORD) sizeof(wic)))
						continue;
					if (wic.wPid == MM_WAVE_MAPPER)
						uWaveInDeviceMapperID = w;
				}
			}
		}
		uWaveInDeviceID = uWaveInDeviceMapperID;
	}
	return (uWaveInDeviceID);

}

static UINT waveOutDeviceMapper(UINT uWaveOutDeviceID)
{
	static UINT uWaveOutDeviceMapperID = WAVE_MAPPER;
	UINT u, v, w;
	WAVEOUTCAPS woc;

	/* open device drivers */
	if (!uNumWaveOutDrivers && !waveOutOpenDrivers())
		return (uWaveOutDeviceID);

	/* get device mapper ID */
	if (uWaveOutDeviceID == WAVE_MAPPER)
	{
		if (uWaveOutDeviceMapperID == WAVE_MAPPER)
		{
			for (u = w = 0; u < uNumWaveOutDrivers; u++)
			{
				for (v = 0; v < uNumWaveOutDevices[u]; v++, w++)
				{
					if ((wodMessage[u])(v, WODM_GETDEVCAPS,
						0,
						(DWORD) (LPWAVEOUTCAPS) &woc,
						(DWORD) sizeof(woc)))
						continue;
					if (woc.wPid == MM_WAVE_MAPPER)
						uWaveOutDeviceMapperID = w;
				}
			}
		}
		uWaveOutDeviceID = uWaveOutDeviceMapperID;
	}
	return (uWaveOutDeviceID);

}

static UINT waveInDeviceMessage(UINT uWaveInDeviceID, UINT uMsg,
	DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
	UINT u;

	/* open device drivers */
	if (!uNumWaveInDrivers && !waveInOpenDrivers())
		return (MMSYSERR_NODRIVER);

	/* get device mapper ID */
	if ((uWaveInDeviceID = waveInDeviceMapper(uWaveInDeviceID))
		== WAVE_MAPPER)
		return (MMSYSERR_BADDEVICEID);

	/* send message to device driver */
	for (u = 0; u < uNumWaveInDrivers; u++)
	{
		if (uWaveInDeviceID >= uNumWaveInDevices[u])
		{
			uWaveInDeviceID -= uNumWaveInDevices[u];
			continue;
		}
		return (widMessage[u])(uWaveInDeviceID, uMsg,
			dwUser, dwParam1, dwParam2);
	}
	return (MMSYSERR_BADDEVICEID);

}

static UINT waveOutDeviceMessage(UINT uWaveOutDeviceID, UINT uMsg,
	DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
	UINT u;

	/* open device drivers */
	if (!uNumWaveOutDrivers && !waveOutOpenDrivers())
		return (MMSYSERR_NODRIVER);

	/* get device mapper ID */
	if ((uWaveOutDeviceID = waveOutDeviceMapper(uWaveOutDeviceID))
		== WAVE_MAPPER)
		return (MMSYSERR_BADDEVICEID);

	/* send message to device driver */
	for (u = 0; u < uNumWaveOutDrivers; u++)
	{
		if (uWaveOutDeviceID >= uNumWaveOutDevices[u])
		{
			uWaveOutDeviceID -= uNumWaveOutDevices[u];
			continue;
		}
		return (wodMessage[u])(uWaveOutDeviceID, uMsg,
			dwUser, dwParam1, dwParam2);
	}
	return (MMSYSERR_BADDEVICEID);

}

#define WAVEIN_SIGNATURE	0x5749
#define WAVEOUT_SIGNATURE	0x574F

typedef struct {
	UINT	uWaveInSignature;
	UINT	uWaveInDeviceID;
	DWORD	dwWaveInUser;
} WAVEIN;
typedef WAVEIN		*PWAVEIN;
typedef WAVEIN NEAR	*NPWAVEIN;
typedef WAVEIN FAR	*LPWAVEIN;

typedef struct {
	UINT	uWaveOutSignature;
	UINT	uWaveOutDeviceID;
	DWORD	dwWaveOutUser;
} WAVEOUT;
typedef WAVEOUT		*PWAVEOUT;
typedef WAVEOUT NEAR	*NPWAVEOUT;
typedef WAVEOUT FAR	*LPWAVEOUT;

static NPWAVEIN waveInAlloc(LPHWAVEIN lpWaveIn, LPUINT lpWaveInResult)
{
	NPWAVEIN npWaveIn;

	/* allocate and lock handle */
	if (lpWaveIn)
	{
		if ((*lpWaveIn = (HWAVEIN) LocalAlloc(LHND, sizeof(WAVEIN))))
		{
			if ((npWaveIn =
				(NPWAVEIN) LocalLock((HLOCAL) *lpWaveIn)))
			{
				*lpWaveInResult = MMSYSERR_NOERROR;
				return (npWaveIn);
			}
			LocalFree((HLOCAL) *lpWaveIn);
		}
		return ((*lpWaveInResult = MMSYSERR_NOMEM, (NPWAVEIN) 0));
	}
	return (*lpWaveInResult = MMSYSERR_INVALPARAM, (NPWAVEIN) 0);

}

static NPWAVEOUT waveOutAlloc(LPHWAVEOUT lpWaveOut, LPUINT lpWaveOutResult)
{
	NPWAVEOUT npWaveOut;

	/* allocate and lock handle */
	if (lpWaveOut)
	{
		if ((*lpWaveOut = (HWAVEOUT) LocalAlloc(LHND, sizeof(WAVEOUT))))
		{
			if ((npWaveOut =
				(NPWAVEOUT) LocalLock((HLOCAL) *lpWaveOut)))
			{
				*lpWaveOutResult = MMSYSERR_NOERROR;
				return (npWaveOut);
			}
			LocalFree((HLOCAL) *lpWaveOut);
		}
		return (*lpWaveOutResult = MMSYSERR_NOMEM, (NPWAVEOUT) 0);
	}
	return (*lpWaveOutResult = MMSYSERR_INVALPARAM, (NPWAVEOUT) 0);

}

static void waveInFree(HWAVEIN hWaveIn)
{
	/* free handle */
	if (hWaveIn)
		LocalFree((HLOCAL) hWaveIn);
}

static void waveOutFree(HWAVEOUT hWaveOut)
{
	/* free handle */
	if (hWaveOut)
		LocalFree((HLOCAL) hWaveOut);
}

static NPWAVEIN waveInLock(HWAVEIN hWaveIn, LPUINT lpWaveInResult)
{
	NPWAVEIN npWaveIn;

	/* lock handle (and verify handle signature) */
	if (hWaveIn && (npWaveIn = (NPWAVEIN) LocalLock((HLOCAL) hWaveIn)))
	{
		if (npWaveIn->uWaveInSignature == WAVEIN_SIGNATURE)
		{
			*lpWaveInResult = MMSYSERR_NOERROR;
			return (npWaveIn);
		}
		LocalUnlock((HLOCAL) hWaveIn);
	}
	return (*lpWaveInResult = MMSYSERR_INVALHANDLE, (NPWAVEIN) 0);

}

static NPWAVEOUT waveOutLock(HWAVEOUT hWaveOut, LPUINT lpWaveOutResult)
{
	NPWAVEOUT npWaveOut;

	/* lock handle (and verify handle signature) */
	if (hWaveOut && (npWaveOut = (NPWAVEOUT) LocalLock((HLOCAL) hWaveOut)))
	{
		if (npWaveOut->uWaveOutSignature == WAVEOUT_SIGNATURE)
		{
			*lpWaveOutResult = MMSYSERR_NOERROR;
			return (npWaveOut);
		}
		LocalUnlock((HLOCAL) hWaveOut);
	}
	return (*lpWaveOutResult = MMSYSERR_INVALHANDLE, (NPWAVEOUT) 0);

}

static void waveInUnlock(HWAVEIN hWaveIn)
{
	/* unlock handle */
	if (hWaveIn)
		LocalUnlock((HLOCAL) hWaveIn);
}

static void waveOutUnlock(HWAVEOUT hWaveOut)
{
	/* unlock handle */
	if (hWaveOut)
		LocalUnlock((HLOCAL) hWaveOut);
}

static UINT waveInGetUser(HWAVEIN hWaveIn, LPDWORD lpWaveInUser)
{
	UINT uWaveInResult;
	NPWAVEIN npWaveIn;

	/* get handle user data */
	if (lpWaveInUser)
	{
		if ((npWaveIn = waveInLock(hWaveIn, &uWaveInResult)))
		{
			*lpWaveInUser = npWaveIn->dwWaveInUser;
			waveInUnlock(hWaveIn);
		}
		return (uWaveInResult);
	}
	return (MMSYSERR_INVALPARAM);

}

static UINT waveOutGetUser(HWAVEOUT hWaveOut, LPDWORD lpWaveOutUser)
{
	UINT uWaveOutResult;
	NPWAVEOUT npWaveOut;

	/* get handle user data */
	if (lpWaveOutUser)
	{
		if ((npWaveOut = waveOutLock(hWaveOut, &uWaveOutResult)))
		{
			*lpWaveOutUser = npWaveOut->dwWaveOutUser;
			waveOutUnlock(hWaveOut);
		}
		return (uWaveOutResult);
	}
	return (MMSYSERR_INVALPARAM);

}

/* [API] (MMSYSTEM) Wave In ************************************************ */

UINT	WINAPI
waveInGetNumDevs()
{
	UINT u, uWaveInNumDevs;

	/* open device drivers */
	if (!uNumWaveInDrivers && !waveInOpenDrivers())
		return (0);

	/* count total number of devices supported by device drivers */
	for (u = uWaveInNumDevs = 0; u < uNumWaveInDrivers; u++)
		uWaveInNumDevs += uNumWaveInDevices[u];
	return (uWaveInNumDevs);

}

UINT	WINAPI
waveInGetDevCaps(UINT uWaveInDeviceID,
	LPWAVEINCAPS lpWaveInCaps, UINT uWaveInCapsSize)
{
	return waveInDeviceMessage(uWaveInDeviceID, WIDM_GETDEVCAPS,
		0, (DWORD) lpWaveInCaps, (DWORD) uWaveInCapsSize);
}

UINT	WINAPI
waveInOpen(LPHWAVEIN lpWaveIn, UINT uWaveInDeviceID,
	const WAVEFORMAT FAR *lpWaveInFormat,
	DWORD dwWaveInCallback, DWORD dwWaveInInstance,
	DWORD dwWaveInOpenFlag)
{
	UINT uWaveInResult;
	NPWAVEIN npWaveIn;
	WAVEOPENDESC WaveInOpenDesc;

	/* get device mapper ID (else query all devices for format support) */
	if ((uWaveInDeviceID = waveInDeviceMapper(uWaveInDeviceID))
		== WAVE_MAPPER)
	{
		UINT u, v;
		for (u = 0, v = waveInGetNumDevs(); u < v; u++)
		{
			if (!(uWaveInResult = waveInOpen((LPHWAVEIN) 0, u,
				lpWaveInFormat, 0, 0, WAVE_FORMAT_QUERY)))
				break;
		}
		if (u >= v)
			return (MMSYSERR_BADDEVICEID);
		uWaveInDeviceID = u;
	}

	/* query device for format support */
	if (dwWaveInOpenFlag & WAVE_FORMAT_QUERY)
	{
		WaveInOpenDesc.hWave = (HWAVE) 0;
		WaveInOpenDesc.lpFormat = lpWaveInFormat;
		WaveInOpenDesc.dwCallback = (DWORD) 0;
		WaveInOpenDesc.dwInstance = (DWORD) 0;
		return waveInDeviceMessage(uWaveInDeviceID, WIDM_OPEN,
			(DWORD) (LPDWORD) 0,
			(DWORD) (LPWAVEOPENDESC) &WaveInOpenDesc,
			WAVE_FORMAT_QUERY);
	}

	/* allocate and lock handle */
	if (!(npWaveIn = waveInAlloc(lpWaveIn, &uWaveInResult)))
		return (uWaveInResult);
	npWaveIn->uWaveInSignature = WAVEIN_SIGNATURE;
	npWaveIn->uWaveInDeviceID = uWaveInDeviceID;
	npWaveIn->dwWaveInUser = 0;

	/* open handle */
	WaveInOpenDesc.hWave = *lpWaveIn;
	WaveInOpenDesc.lpFormat = lpWaveInFormat;
	WaveInOpenDesc.dwCallback = dwWaveInCallback;
	WaveInOpenDesc.dwInstance = dwWaveInInstance;
	uWaveInResult = waveInDeviceMessage(uWaveInDeviceID, WIDM_OPEN,
		(DWORD) (LPDWORD) &npWaveIn->dwWaveInUser,
		(DWORD) (LPWAVEOPENDESC) &WaveInOpenDesc,
		dwWaveInOpenFlag);

	/* unlock handle (and free handle if error) */
	waveInUnlock(*lpWaveIn);
	if (uWaveInResult)
		waveInFree(*lpWaveIn);
	return (uWaveInResult);

}

UINT	WINAPI
waveInClose(HWAVEIN hWaveIn)
{
	UINT uWaveInResult;

	if (!(uWaveInResult = waveInMessage(hWaveIn, WIDM_CLOSE, 0, 0)))
		waveInFree(hWaveIn);

	return (uWaveInResult);

}

UINT	WINAPI
waveInPrepareHeader(HWAVEIN hWaveIn,
	LPWAVEHDR lpWaveInHdr, UINT uWaveInHdrSize)
{
	UINT uWaveInResult;
	HGLOBAL hWaveInHdr, hWaveInData;

	if (((uWaveInResult = waveInMessage(hWaveIn, WIDM_PREPARE,
		(DWORD) lpWaveInHdr, (DWORD) uWaveInHdrSize))
		== MMSYSERR_NOTSUPPORTED)
	 && (lpWaveInHdr != NULL)
	 && (uWaveInHdrSize >= sizeof(WAVEHDR)))
	{
		/* test WHDR_PREPARED bit */
		if (lpWaveInHdr->dwFlags & WHDR_PREPARED)
			return (MMSYSERR_NOERROR);
		/* prepare header */
		if (!lpWaveInHdr->lpData
		 || !(hWaveInHdr = GlobalHandle(lpWaveInHdr))
		 || !(hWaveInData = GlobalHandle(lpWaveInHdr->lpData))
		 || !GlobalPageLock(hWaveInHdr))
			return (MMSYSERR_NOMEM);
		if (!GlobalPageLock(hWaveInData))
		{
			GlobalPageUnlock(hWaveInHdr);
			return (MMSYSERR_NOMEM);
		}
		/* set WHDR_PREPARED bit */
		lpWaveInHdr->dwFlags |= WHDR_PREPARED;
		return (MMSYSERR_NOERROR);
	}

	return (uWaveInResult);

}

UINT	WINAPI
waveInUnprepareHeader(HWAVEIN hWaveIn,
	LPWAVEHDR lpWaveInHdr, UINT uWaveInHdrSize)
{
	UINT uWaveInResult;
	HGLOBAL hWaveInHdr, hWaveInData;

	if (((uWaveInResult = waveInMessage(hWaveIn, WIDM_UNPREPARE,
		(DWORD) lpWaveInHdr, (DWORD) uWaveInHdrSize))
		== MMSYSERR_NOTSUPPORTED)
	 && (lpWaveInHdr != NULL)
	 && (uWaveInHdrSize >= sizeof(WAVEHDR)))
	{
		/* test WHDR_PREPARED bit */
		if (!(lpWaveInHdr->dwFlags & WHDR_PREPARED))
			return (MMSYSERR_NOERROR);
		/* unprepare header */
		if (!lpWaveInHdr->lpData
		 || !(hWaveInHdr = GlobalHandle(lpWaveInHdr))
		 || !(hWaveInData = GlobalHandle(lpWaveInHdr->lpData))
		 || !GlobalPageUnlock(hWaveInHdr))
			return (MMSYSERR_NOMEM);
		if (!GlobalPageUnlock(hWaveInData))
		{
			GlobalPageLock(hWaveInHdr);
			return (MMSYSERR_NOMEM);
		}
		/* reset WHDR_PREPARED bit */
		lpWaveInHdr->dwFlags &= ~WHDR_PREPARED;
		return (MMSYSERR_NOERROR);
	}

	return (uWaveInResult);

}

UINT	WINAPI
waveInAddBuffer(HWAVEIN hWaveIn,
	LPWAVEHDR lpWaveInHdr, UINT uWaveInHdrSize)
{
	return waveInMessage(hWaveIn, WIDM_ADDBUFFER,
		(DWORD) lpWaveInHdr, (DWORD) uWaveInHdrSize);
}

UINT	WINAPI
waveInReset(HWAVEIN hWaveIn)
{
	return waveInMessage(hWaveIn, WIDM_RESET, 0, 0);
}

UINT	WINAPI
waveInStart(HWAVEIN hWaveIn)
{
	return waveInMessage(hWaveIn, WIDM_START, 0, 0);
}

UINT	WINAPI
waveInStop(HWAVEIN hWaveIn)
{
	return waveInMessage(hWaveIn, WIDM_STOP, 0, 0);
}

UINT	WINAPI
waveInGetPosition(HWAVEIN hWaveIn, LPMMTIME lpMmTime, UINT uMmTimeSize)
{
	return waveInMessage(hWaveIn, WIDM_STOP,
		(DWORD) lpMmTime, (DWORD) uMmTimeSize);
}

UINT	WINAPI
waveInGetID(HWAVEIN hWaveIn, LPUINT lpWaveInDeviceID)
{
	UINT uWaveInResult;
	NPWAVEIN npWaveIn;

	/* get handle device ID */
	if (lpWaveInDeviceID)
	{
		if ((npWaveIn = waveInLock(hWaveIn, &uWaveInResult)))
		{
			*lpWaveInDeviceID = npWaveIn->uWaveInDeviceID;
			waveInUnlock(hWaveIn);
		}
		return (uWaveInResult);
	}
	return (MMSYSERR_INVALPARAM);

}

DWORD	WINAPI
waveInMessage(HWAVEIN hWaveIn, UINT uMsg, DWORD dwParam1, DWORD dwParam2)
{
	UINT uWaveInResult = 0;
	UINT uWaveInDeviceID;
	DWORD dwWaveInUser;

	if ((waveInGetID(hWaveIn, &uWaveInDeviceID)) &&
	    (waveInGetUser(hWaveIn, &dwWaveInUser))) {
	
		waveInDeviceMessage(uWaveInDeviceID, uMsg,
			dwWaveInUser, dwParam1, dwParam2);
	}

	return (uWaveInResult);

}

UINT	WINAPI
waveInGetErrorText(UINT uError, LPSTR lpErrorText, UINT uErrorTextSize)
{
	UINT uWaveInResult = MMSYSERR_BADERRNUM;

	/* check error text buffer */
	if (!uErrorTextSize)
		return (MMSYSERR_NOERROR);
	if (!lpErrorText)
		return (MMSYSERR_INVALPARAM);

	/* get error text */
	if ((uError >= MMSYSERR_BASE) &&
	    (uError <= MMSYSERR_LASTERROR) ) {
	
		LoadString(GetModuleHandle("mmsystem.dll"),
			uError, lpErrorText, uErrorTextSize);
	}

	/* get error text */
	if ((uError >= WAVERR_BASE) &&
	    (uError <= WAVERR_LASTERROR) ) {
	
		LoadString(GetModuleHandle("mmsystem.dll"),
			uError, lpErrorText, uErrorTextSize);
	}

	/* return get error text result */
	return (uWaveInResult);

}

/* [API] (MMSYSTEM) Wave Out *********************************************** */

UINT	WINAPI
waveOutGetNumDevs()
{
	UINT u, uWaveOutNumDevs;

	/* open device drivers */
	if (!uNumWaveOutDrivers && !waveOutOpenDrivers())
		return (0);

	/* count total number of devices supported by device drivers */
	for (u = uWaveOutNumDevs = 0; u < uNumWaveOutDrivers; u++)
		uWaveOutNumDevs += uNumWaveOutDevices[u];
	return (uWaveOutNumDevs);

}

UINT	WINAPI
waveOutGetDevCaps(UINT uWaveOutDeviceID,
	LPWAVEOUTCAPS lpWaveOutCaps, UINT uWaveOutCapsSize)
{
	return waveOutDeviceMessage(uWaveOutDeviceID, WODM_GETDEVCAPS,
		0, (DWORD) lpWaveOutCaps, (DWORD) uWaveOutCapsSize);
}

UINT	WINAPI
waveOutGetVolume(UINT uWaveOutDeviceID, LPDWORD lpWaveOutVolume)
{
	return waveOutDeviceMessage(uWaveOutDeviceID, WODM_GETVOLUME,
		0, (DWORD) lpWaveOutVolume, 0);
}

UINT	WINAPI
waveOutSetVolume(UINT uWaveOutDeviceID, DWORD dwWaveOutVolume)
{
	return waveOutDeviceMessage(uWaveOutDeviceID, WODM_SETVOLUME,
		0, dwWaveOutVolume, 0);
}

UINT	WINAPI
waveOutOpen(LPHWAVEOUT lpWaveOut, UINT uWaveOutDeviceID,
	const WAVEFORMAT FAR *lpWaveOutFormat,
	DWORD dwWaveOutCallback, DWORD dwWaveOutInstance,
	DWORD dwWaveOutOpenFlag)
{
	UINT uWaveOutResult;
	NPWAVEOUT npWaveOut;
	WAVEOPENDESC WaveOutOpenDesc;

	/* get device mapper ID (else query all devices for format support) */
	if ((uWaveOutDeviceID = waveOutDeviceMapper(uWaveOutDeviceID))
		== WAVE_MAPPER)
	{
		UINT u, v;
		for (u = 0, v = waveOutGetNumDevs(); u < v; u++)
		{
			if (!(uWaveOutResult = waveOutOpen((LPHWAVEOUT) 0, u,
				lpWaveOutFormat, 0, 0, WAVE_FORMAT_QUERY)))
				break;
		}
		if (u >= v)
			return (MMSYSERR_BADDEVICEID);
		uWaveOutDeviceID = u;
	}

	/* query device for format support */
	if (dwWaveOutOpenFlag & WAVE_FORMAT_QUERY)
	{
		WaveOutOpenDesc.hWave = (HWAVE) 0;
		WaveOutOpenDesc.lpFormat = lpWaveOutFormat;
		WaveOutOpenDesc.dwCallback = (DWORD) 0;
		WaveOutOpenDesc.dwInstance = (DWORD) 0;
		return waveOutDeviceMessage(uWaveOutDeviceID, WODM_OPEN,
			(DWORD) (LPDWORD) 0,
			(DWORD) (LPWAVEOPENDESC) &WaveOutOpenDesc,
			WAVE_FORMAT_QUERY);
	}

	/* allocate and lock handle */
	if (!(npWaveOut = waveOutAlloc(lpWaveOut, &uWaveOutResult)))
		return (uWaveOutResult);
	npWaveOut->uWaveOutSignature = WAVEOUT_SIGNATURE;
	npWaveOut->uWaveOutDeviceID = uWaveOutDeviceID;
	npWaveOut->dwWaveOutUser = 0;

	/* open handle */
	WaveOutOpenDesc.hWave = *lpWaveOut;
	WaveOutOpenDesc.lpFormat = lpWaveOutFormat;
	WaveOutOpenDesc.dwCallback = dwWaveOutCallback;
	WaveOutOpenDesc.dwInstance = dwWaveOutInstance;
	uWaveOutResult = waveOutDeviceMessage(uWaveOutDeviceID, WODM_OPEN,
		(DWORD) (LPDWORD) &npWaveOut->dwWaveOutUser,
		(DWORD) (LPWAVEOPENDESC) &WaveOutOpenDesc,
		dwWaveOutOpenFlag);

	/* unlock handle (and free handle if error) */
	waveOutUnlock(*lpWaveOut);
	if (uWaveOutResult)
		waveOutFree(*lpWaveOut);
	return (uWaveOutResult);

}

UINT	WINAPI
waveOutClose(HWAVEOUT hWaveOut)
{
	UINT uWaveOutResult;

	if (!(uWaveOutResult = waveOutMessage(hWaveOut, WODM_CLOSE, 0, 0)))
		waveOutFree(hWaveOut);

	return (uWaveOutResult);

}

UINT	WINAPI
waveOutPrepareHeader(HWAVEOUT hWaveOut,
	LPWAVEHDR lpWaveOutHdr, UINT uWaveOutHdrSize)
{
	UINT uWaveOutResult;
	HGLOBAL hWaveOutHdr, hWaveOutData;

	if (((uWaveOutResult = waveOutMessage(hWaveOut, WODM_PREPARE,
		(DWORD) lpWaveOutHdr, (DWORD) uWaveOutHdrSize))
		== MMSYSERR_NOTSUPPORTED)
	 && (lpWaveOutHdr != NULL)
	 && (uWaveOutHdrSize >= sizeof(WAVEHDR)))
	{
		/* test WHDR_PREPARED bit */
		if (lpWaveOutHdr->dwFlags & WHDR_PREPARED)
			return (MMSYSERR_NOERROR);
		/* prepare header */
		if (!lpWaveOutHdr->lpData
		 || !(hWaveOutHdr = GlobalHandle(lpWaveOutHdr))
		 || !(hWaveOutData = GlobalHandle(lpWaveOutHdr->lpData))
		 || !GlobalPageLock(hWaveOutHdr))
			return (MMSYSERR_NOMEM);
		if (!GlobalPageLock(hWaveOutData))
		{
			GlobalPageUnlock(hWaveOutHdr);
			return (MMSYSERR_NOMEM);
		}
		/* set WHDR_PREPARED bit */
		lpWaveOutHdr->dwFlags |= WHDR_PREPARED;
		return (MMSYSERR_NOERROR);
	}

	return (uWaveOutResult);

}

UINT	WINAPI
waveOutUnprepareHeader(HWAVEOUT hWaveOut,
	LPWAVEHDR lpWaveOutHdr, UINT uWaveOutHdrSize)
{
	UINT uWaveOutResult;
	HGLOBAL hWaveOutHdr, hWaveOutData;

	if (((uWaveOutResult = waveOutMessage(hWaveOut, WODM_UNPREPARE,
		(DWORD) lpWaveOutHdr, (DWORD) uWaveOutHdrSize))
		== MMSYSERR_NOTSUPPORTED)
	 && (lpWaveOutHdr != NULL)
	 && (uWaveOutHdrSize >= sizeof(WAVEHDR)))
	{
		/* test WHDR_PREPARED bit */
		if (!(lpWaveOutHdr->dwFlags & WHDR_PREPARED))
			return (MMSYSERR_NOERROR);
		/* unprepare header */
		if (!lpWaveOutHdr->lpData
		 || !(hWaveOutHdr = GlobalHandle(lpWaveOutHdr))
		 || !(hWaveOutData = GlobalHandle(lpWaveOutHdr->lpData))
		 || !GlobalPageUnlock(hWaveOutHdr))
			return (MMSYSERR_NOMEM);
		if (!GlobalPageUnlock(hWaveOutData))
		{
			GlobalPageLock(hWaveOutHdr);
			return (MMSYSERR_NOMEM);
		}
		/* reset WHDR_PREPARED bit */
		lpWaveOutHdr->dwFlags &= ~WHDR_PREPARED;
		return (MMSYSERR_NOERROR);
	}

	return (uWaveOutResult);

}

UINT	WINAPI
waveOutWrite(HWAVEOUT hWaveOut,
	LPWAVEHDR lpWaveOutHdr, UINT uWaveOutHdrSize)
{
	return waveOutMessage(hWaveOut, WODM_WRITE,
		(DWORD) lpWaveOutHdr, (DWORD) uWaveOutHdrSize);
}

UINT	WINAPI
waveOutReset(HWAVEOUT hWaveOut)
{
	return waveOutMessage(hWaveOut, WODM_RESET, 0, 0);
}

UINT	WINAPI
waveOutPause(HWAVEOUT hWaveOut)
{
	return waveOutMessage(hWaveOut, WODM_PAUSE, 0, 0);
}

UINT	WINAPI
waveOutRestart(HWAVEOUT hWaveOut)
{
	return waveOutMessage(hWaveOut, WODM_RESTART, 0, 0);
}

UINT	WINAPI
waveOutBreakLoop(HWAVEOUT hWaveOut)
{
	return waveOutMessage(hWaveOut, WODM_BREAKLOOP, 0, 0);
}

UINT	WINAPI
waveOutGetPosition(HWAVEOUT hWaveOut, LPMMTIME lpMmTime, UINT uMmTimeSize)
{
	return waveOutMessage(hWaveOut, WODM_GETPOS,
		(DWORD) lpMmTime, (DWORD) uMmTimeSize);
}

UINT	WINAPI
waveOutGetPitch(HWAVEOUT hWaveOut, LPDWORD lpWaveOutPitch)
{
	return waveOutMessage(hWaveOut, WODM_GETPITCH,
		(DWORD) lpWaveOutPitch, 0);
}

UINT	WINAPI
waveOutSetPitch(HWAVEOUT hWaveOut, DWORD dwWaveOutPitch)
{
	return waveOutMessage(hWaveOut, WODM_SETPITCH,
		dwWaveOutPitch, 0);
}

UINT	WINAPI
waveOutGetPlaybackRate(HWAVEOUT hWaveOut, LPDWORD lpWaveOutPlaybackRate)
{
	return waveOutMessage(hWaveOut, WODM_GETPLAYBACKRATE,
		(DWORD) lpWaveOutPlaybackRate, 0);
}

UINT	WINAPI
waveOutSetPlaybackRate(HWAVEOUT hWaveOut, DWORD dwWaveOutPlaybackRate)
{
	return waveOutMessage(hWaveOut, WODM_SETPLAYBACKRATE,
		dwWaveOutPlaybackRate, 0);
}

UINT	WINAPI
waveOutGetID(HWAVEOUT hWaveOut, LPUINT lpWaveOutDeviceID)
{
	UINT uWaveOutResult;
	NPWAVEOUT npWaveOut;

	/* get handle device ID */
	if (lpWaveOutDeviceID)
	{
		if ((npWaveOut = waveOutLock(hWaveOut, &uWaveOutResult)))
		{
			*lpWaveOutDeviceID = npWaveOut->uWaveOutDeviceID;
			waveOutUnlock(hWaveOut);
		}
		return (uWaveOutResult);
	}
	return (MMSYSERR_INVALPARAM);

}

DWORD	WINAPI
waveOutMessage(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwParam1, DWORD dwParam2)
{
	UINT uWaveOutResult, uWaveOutDeviceID;
	DWORD dwWaveOutUser;

	if ((uWaveOutResult = waveOutGetID(hWaveOut, &uWaveOutDeviceID)) &&
	    (uWaveOutResult = waveOutGetUser(hWaveOut, &dwWaveOutUser))) {
	
		uWaveOutResult = waveOutDeviceMessage(uWaveOutDeviceID, uMsg,
			dwWaveOutUser, dwParam1, dwParam2);
	}

	return (uWaveOutResult);

}

UINT	WINAPI
waveOutGetErrorText(UINT uError, LPSTR lpErrorText, UINT uErrorTextSize)
{
	UINT uWaveOutResult = MMSYSERR_BADERRNUM;

	/* check error text buffer */
	if (!uErrorTextSize)
		return (MMSYSERR_NOERROR);
	if (!lpErrorText)
		return (MMSYSERR_INVALPARAM);

	/* get error text */
	if ((uError >= MMSYSERR_BASE) &&
	    (uError <= MMSYSERR_LASTERROR)) {
	
	    LoadString(GetModuleHandle("mmsystem.dll"),
		uError, lpErrorText, uErrorTextSize);
	}

	/* get error text */
	if ((uError >= WAVERR_BASE) &&
	    (uError <= WAVERR_LASTERROR)) {
	
            LoadString(GetModuleHandle("mmsystem.dll"),
		uError, lpErrorText, uErrorTextSize);
	}

	/* return get error text result */
	return (uWaveOutResult);

}
