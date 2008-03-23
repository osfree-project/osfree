/*    
	midi.c	1.2 midi.c - multimedia midi audio functions
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

/* [INTERNAL] (MMSYSTEM) Midi ********************************************** */

typedef DWORD (FAR *MIDPROC)(UINT uDeviceID, UINT uMsg,
	DWORD dwUser, DWORD dwParam1, DWORD dwParam2);

typedef DWORD (FAR *MODPROC)(UINT uDeviceID, UINT uMsg,
	DWORD dwUser, DWORD dwParam1, DWORD dwParam2);

static UINT uNumMidiDrivers = 0;
static HDRVR hMidiDrivers[10];

static UINT uNumMidiInDrivers = 0;
static UINT uNumMidiInDevices[10];
static MIDPROC midMessage[10];

static UINT uNumMidiOutDrivers = 0;
static UINT uNumMidiOutDevices[10];
static MODPROC modMessage[10];

static UINT midiOpenDrivers()
{
	/* open device drivers specified in SYSTEM.INI file
	 *
	 * single device driver
	 * --------------------
	 * [drivers]
	 * midi=...
	 *
	 * multiple device drivers
	 * -----------------------
	 * [drivers]
	 * midi1=...
	 * midi2=...
	 * ...
	 *
	 */
	static char midi[6] = "midi?";
	UINT u;

	/* open device drivers once */
	if (uNumMidiDrivers)
		return (uNumMidiDrivers);

	/* for each device driver
	 *	- open device driver
	 */
	for (u = uNumMidiDrivers = 0; u <= 9; u++)
	{
		midi[sizeof(midi)-2] = "\000123456789"[u];
		if ((hMidiDrivers[uNumMidiDrivers] = 
			OpenDriver(midi, NULL, 0L)))
			break;
	}
	return (uNumMidiDrivers);

}

static UINT midiInOpenDrivers()
{
	UINT u;
	HINSTANCE hMidiDriverModule;

	/* open device drivers */
	if (!uNumMidiDrivers && !midiOpenDrivers())
		return (0);

	/* for each device driver
	 *	- find midMessage() function
	 *	- call midMessage() function to get number of devices
	 */
	for (u = uNumMidiInDrivers = 0; u < uNumMidiDrivers; u++)
	{
		if ((hMidiDriverModule =
			GetDriverModuleHandle(hMidiDrivers[u]))) {

		   midMessage[uNumMidiInDrivers] = (MIDPROC)
			GetProcAddress(hMidiDriverModule, "midMessage");

		   uNumMidiInDevices[uNumMidiInDrivers] =
			(midMessage[uNumMidiInDrivers])(0, MIDM_GETNUMDEVS,
			0, 0, 0);
		   uNumMidiInDrivers++;
		}
	}
	return (uNumMidiInDrivers);

}

static UINT midiOutOpenDrivers()
{
	UINT u;
	HINSTANCE hMidiDriverModule;

	/* open device drivers */
	if (!uNumMidiDrivers && !midiOpenDrivers())
		return (0);

	/* for each device driver
	 *	- find modMessage() function
	 *	- call modMessage() function to get number of devices
	 */
	for (u = uNumMidiOutDrivers = 0; u < uNumMidiDrivers; u++)
	{
		if ((hMidiDriverModule =
			GetDriverModuleHandle(hMidiDrivers[u]))) {

		   modMessage[uNumMidiOutDrivers] = (MODPROC)
			GetProcAddress(hMidiDriverModule, "modMessage");

		   uNumMidiOutDevices[uNumMidiOutDrivers] =
			(modMessage[uNumMidiOutDrivers])(0, MODM_GETNUMDEVS,
			0, 0, 0);

		   uNumMidiOutDrivers++;
		}
	}
	return (uNumMidiOutDrivers);

}

static UINT midiInDeviceMapper(UINT uMidiInDeviceID)
{
	static UINT uMidiInDeviceMapperID = MIDI_MAPPER;
	UINT u, v, w;
	MIDIINCAPS mic;

	/* open device drivers */
	if (!uNumMidiInDrivers && !midiInOpenDrivers())
		return (uMidiInDeviceID);

	/* get device mapper ID */
	if (uMidiInDeviceID == MIDI_MAPPER)
	{
		if (uMidiInDeviceMapperID == MIDI_MAPPER)
		{
			for (u = w = 0; u < uNumMidiInDrivers; u++)
			{
				for (v = 0; v < uNumMidiInDevices[u]; v++, w++)
				{
					if ((midMessage[u])(v, MIDM_GETDEVCAPS,
						0,
						(DWORD) (LPMIDIINCAPS) &mic,
						(DWORD) sizeof(mic)))
						continue;
					if (mic.wPid == MM_MIDI_MAPPER)
						uMidiInDeviceMapperID = w;
				}
			}
		}
		uMidiInDeviceID = uMidiInDeviceMapperID;
	}
	return (uMidiInDeviceID);

}

static UINT midiOutDeviceMapper(UINT uMidiOutDeviceID)
{
	static UINT uMidiOutDeviceMapperID = MIDI_MAPPER;
	UINT u, v, w;
	MIDIOUTCAPS moc;

	/* open device drivers */
	if (!uNumMidiOutDrivers && !midiOutOpenDrivers())
		return (uMidiOutDeviceID);

	/* get device mapper ID */
	if (uMidiOutDeviceID == MIDI_MAPPER)
	{
		if (uMidiOutDeviceMapperID == MIDI_MAPPER)
		{
			for (u = w = 0; u < uNumMidiOutDrivers; u++)
			{
				for (v = 0; v < uNumMidiOutDevices[u]; v++, w++)
				{
					if ((modMessage[u])(v, MODM_GETDEVCAPS,
						0,
						(DWORD) (LPMIDIOUTCAPS) &moc,
						(DWORD) sizeof(moc)))
						continue;
					if (moc.wPid == MM_MIDI_MAPPER)
						uMidiOutDeviceMapperID = w;
				}
			}
		}
		uMidiOutDeviceID = uMidiOutDeviceMapperID;
	}
	return (uMidiOutDeviceID);

}

static UINT midiInDeviceMessage(UINT uMidiInDeviceID, UINT uMsg,
	DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
	UINT u;

	/* open device drivers */
	if (!uNumMidiInDrivers && !midiInOpenDrivers())
		return (MMSYSERR_NODRIVER);

	/* get device mapper ID */
	if ((uMidiInDeviceID = midiInDeviceMapper(uMidiInDeviceID))
		== MIDI_MAPPER)
		return (MMSYSERR_BADDEVICEID);

	/* send message to device driver */
	for (u = 0; u < uNumMidiInDrivers; u++)
	{
		if (uMidiInDeviceID >= uNumMidiInDevices[u])
		{
			uMidiInDeviceID -= uNumMidiInDevices[u];
			continue;
		}
		return (midMessage[u])(uMidiInDeviceID, uMsg,
			dwUser, dwParam1, dwParam2);
	}
	return (MMSYSERR_BADDEVICEID);

}

static UINT midiOutDeviceMessage(UINT uMidiOutDeviceID, UINT uMsg,
	DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
	UINT u;

	/* open device drivers */
	if (!uNumMidiOutDrivers && !midiOutOpenDrivers())
		return (MMSYSERR_NODRIVER);

	/* get device mapper ID */
	if ((uMidiOutDeviceID = midiOutDeviceMapper(uMidiOutDeviceID))
		== MIDI_MAPPER)
		return (MMSYSERR_BADDEVICEID);

	/* send message to device driver */
	for (u = 0; u < uNumMidiOutDrivers; u++)
	{
		if (uMidiOutDeviceID >= uNumMidiOutDevices[u])
		{
			uMidiOutDeviceID -= uNumMidiOutDevices[u];
			continue;
		}
		return (modMessage[u])(uMidiOutDeviceID, uMsg,
			dwUser, dwParam1, dwParam2);
	}
	return (MMSYSERR_BADDEVICEID);

}

#define MIDIIN_SIGNATURE	0x4D49
#define MIDIOUT_SIGNATURE	0x4D4F

typedef struct {
	UINT	uMidiInSignature;
	UINT	uMidiInDeviceID;
	DWORD	dwMidiInUser;
} MIDIIN;
typedef MIDIIN		*PMIDIIN;
typedef MIDIIN NEAR	*NPMIDIIN;
typedef MIDIIN FAR	*LPMIDIIN;

typedef struct {
	UINT	uMidiOutSignature;
	UINT	uMidiOutDeviceID;
	DWORD	dwMidiOutUser;
} MIDIOUT;
typedef MIDIOUT		*PMIDIOUT;
typedef MIDIOUT NEAR	*NPMIDIOUT;
typedef MIDIOUT FAR	*LPMIDIOUT;

static NPMIDIIN midiInAlloc(LPHMIDIIN lpMidiIn, LPUINT lpMidiInResult)
{
	NPMIDIIN npMidiIn;

	/* allocate and lock handle */
	if (lpMidiIn)
	{
		if ((*lpMidiIn = (HMIDIIN) LocalAlloc(LHND, sizeof(MIDIIN))))
		{
			if ((npMidiIn =
				(NPMIDIIN) LocalLock((HLOCAL) *lpMidiIn)))
			{
				*lpMidiInResult = MMSYSERR_NOERROR;
				return (npMidiIn);
			}
			LocalFree((HLOCAL) *lpMidiIn);
		}
		return (*lpMidiInResult = MMSYSERR_NOMEM, (NPMIDIIN) 0);
	}
	return (*lpMidiInResult = MMSYSERR_INVALPARAM, (NPMIDIIN) 0);

}

static NPMIDIOUT midiOutAlloc(LPHMIDIOUT lpMidiOut, LPUINT lpMidiOutResult)
{
	NPMIDIOUT npMidiOut;

	/* allocate and lock handle */
	if (lpMidiOut)
	{
		if ((*lpMidiOut = (HMIDIOUT) LocalAlloc(LHND, sizeof(MIDIOUT))))
		{
			if ((npMidiOut =
				(NPMIDIOUT) LocalLock((HLOCAL) *lpMidiOut)))
			{
				*lpMidiOutResult = MMSYSERR_NOERROR;
				return (npMidiOut);
			}
			LocalFree((HLOCAL) *lpMidiOut);
		}
		return (*lpMidiOutResult = MMSYSERR_NOMEM, (NPMIDIOUT) 0);
	}
	return (*lpMidiOutResult = MMSYSERR_INVALPARAM, (NPMIDIOUT) 0);

}

static void midiInFree(HMIDIIN hMidiIn)
{
	/* free handle */
	if (hMidiIn)
		LocalFree((HLOCAL) hMidiIn);
}

static void midiOutFree(HMIDIOUT hMidiOut)
{
	/* free handle */
	if (hMidiOut)
		LocalFree((HLOCAL) hMidiOut);
}

static NPMIDIIN midiInLock(HMIDIIN hMidiIn, LPUINT lpMidiInResult)
{
	NPMIDIIN npMidiIn;

	/* lock handle (and verify handle signature) */
	if (hMidiIn && (npMidiIn = (NPMIDIIN) LocalLock((HLOCAL) hMidiIn)))
	{
		if (npMidiIn->uMidiInSignature == MIDIIN_SIGNATURE)
		{
			*lpMidiInResult = MMSYSERR_NOERROR;
			return (npMidiIn);
		}
		LocalUnlock((HLOCAL) hMidiIn);
	}
	return (*lpMidiInResult = MMSYSERR_INVALHANDLE, (NPMIDIIN) 0);

}

static NPMIDIOUT midiOutLock(HMIDIOUT hMidiOut, LPUINT lpMidiOutResult)
{
	NPMIDIOUT npMidiOut;

	/* lock handle (and verify handle signature) */
	if (hMidiOut && (npMidiOut = (NPMIDIOUT) LocalLock((HLOCAL) hMidiOut)))
	{
		if (npMidiOut->uMidiOutSignature == MIDIOUT_SIGNATURE)
		{
			*lpMidiOutResult = MMSYSERR_NOERROR;
			return (npMidiOut);
		}
		LocalUnlock((HLOCAL) hMidiOut);
	}
	return (*lpMidiOutResult = MMSYSERR_INVALHANDLE, (NPMIDIOUT) 0);

}

static void midiInUnlock(HMIDIIN hMidiIn)
{
	/* unlock handle */
	if (hMidiIn)
		LocalUnlock((HLOCAL) hMidiIn);
}

static void midiOutUnlock(HMIDIOUT hMidiOut)
{
	/* unlock handle */
	if (hMidiOut)
		LocalUnlock((HLOCAL) hMidiOut);
}

static UINT midiInGetUser(HMIDIIN hMidiIn, LPDWORD lpMidiInUser)
{
	UINT uMidiInResult;
	NPMIDIIN npMidiIn;

	/* get handle user data */
	if (lpMidiInUser)
	{
		if ((npMidiIn = midiInLock(hMidiIn, &uMidiInResult)))
		{
			*lpMidiInUser = npMidiIn->dwMidiInUser;
			midiInUnlock(hMidiIn);
		}
		return (uMidiInResult);
	}
	return (MMSYSERR_INVALPARAM);

}

static UINT midiOutGetUser(HMIDIOUT hMidiOut, LPDWORD lpMidiOutUser)
{
	UINT uMidiOutResult;
	NPMIDIOUT npMidiOut;

	/* get handle user data */
	if (lpMidiOutUser)
	{
		if ((npMidiOut = midiOutLock(hMidiOut, &uMidiOutResult)))
		{
			*lpMidiOutUser = npMidiOut->dwMidiOutUser;
			midiOutUnlock(hMidiOut);
		}
		return (uMidiOutResult);
	}
	return (MMSYSERR_INVALPARAM);

}

/* [API] (MMSYSTEM) Midi In ************************************************ */

UINT	WINAPI
midiInGetNumDevs()
{
	UINT u, uMidiInNumDevs;

	/* open device drivers */
	if (!uNumMidiInDrivers && !midiInOpenDrivers())
		return (0);

	/* count total number of devices supported by device drivers */
	for (u = uMidiInNumDevs = 0; u < uNumMidiInDrivers; u++)
		uMidiInNumDevs += uNumMidiInDevices[u];
	return (uMidiInNumDevs);

}

UINT	WINAPI
midiInGetDevCaps(UINT uMidiInDeviceID,
	LPMIDIINCAPS lpMidiInCaps, UINT uMidiInCapsSize)
{
	return midiInDeviceMessage(uMidiInDeviceID, MIDM_GETDEVCAPS,
		0, (DWORD) lpMidiInCaps, (DWORD) uMidiInCapsSize);
}

UINT	WINAPI
midiInOpen(LPHMIDIIN lpMidiIn, UINT uMidiInDeviceID,
	DWORD dwMidiInCallback, DWORD dwMidiInInstance,
	DWORD dwMidiInOpenFlag)
{
	UINT uMidiInResult;
	NPMIDIIN npMidiIn;
	MIDIOPENDESC MidiInOpenDesc;

	/* get device mapper ID */
	if ((uMidiInDeviceID = midiInDeviceMapper(uMidiInDeviceID))
		== MIDI_MAPPER)
		return (MMSYSERR_BADDEVICEID);

	/* allocate and lock handle */
	if (!(npMidiIn = midiInAlloc(lpMidiIn, &uMidiInResult)))
		return (uMidiInResult);
	npMidiIn->uMidiInSignature = MIDIIN_SIGNATURE;
	npMidiIn->uMidiInDeviceID = uMidiInDeviceID;
	npMidiIn->dwMidiInUser = 0;

	/* open handle */
	MidiInOpenDesc.hMidi = *lpMidiIn;
	MidiInOpenDesc.dwCallback = dwMidiInCallback;
	MidiInOpenDesc.dwInstance = dwMidiInInstance;
	uMidiInResult = midiInDeviceMessage(uMidiInDeviceID, MIDM_OPEN,
		(DWORD) (LPDWORD) &npMidiIn->dwMidiInUser,
		(DWORD) (LPMIDIOPENDESC) &MidiInOpenDesc,
		dwMidiInOpenFlag);

	/* unlock handle (and free handle if error) */
	midiInUnlock(*lpMidiIn);
	if (uMidiInResult)
		midiInFree(*lpMidiIn);
	return (uMidiInResult);

}

UINT	WINAPI
midiInClose(HMIDIIN hMidiIn)
{
	UINT uMidiInResult;

	if (!(uMidiInResult = midiInMessage(hMidiIn, MIDM_CLOSE, 0, 0)))
		midiInFree(hMidiIn);

	return (uMidiInResult);

}

UINT	WINAPI
midiInPrepareHeader(HMIDIIN hMidiIn,
	LPMIDIHDR lpMidiInHdr, UINT uMidiInHdrSize)
{
	UINT uMidiInResult;
	HGLOBAL hMidiInHdr, hMidiInData;

	if (((uMidiInResult = midiInMessage(hMidiIn, MIDM_PREPARE,
		(DWORD) lpMidiInHdr, (DWORD) uMidiInHdrSize))
		== MMSYSERR_NOTSUPPORTED)
	 && (lpMidiInHdr != NULL)
	 && (uMidiInHdrSize >= sizeof(MIDIHDR)))
	{
		/* test MHDR_PREPARED bit */
		if (lpMidiInHdr->dwFlags & MHDR_PREPARED)
			return (MMSYSERR_NOERROR);
		/* prepare header */
		if (!lpMidiInHdr->lpData
		 || !(hMidiInHdr = GlobalHandle(lpMidiInHdr))
		 || !(hMidiInData = GlobalHandle(lpMidiInHdr->lpData))
		 || !GlobalPageLock(hMidiInHdr))
			return (MMSYSERR_NOMEM);
		if (!GlobalPageLock(hMidiInData))
		{
			GlobalPageUnlock(hMidiInHdr);
			return (MMSYSERR_NOMEM);
		}
		/* set MHDR_PREPARED bit */
		lpMidiInHdr->dwFlags |= MHDR_PREPARED;
		return (MMSYSERR_NOERROR);
	}

	return (uMidiInResult);

}

UINT	WINAPI
midiInUnprepareHeader(HMIDIIN hMidiIn,
	LPMIDIHDR lpMidiInHdr, UINT uMidiInHdrSize)
{
	UINT uMidiInResult;
	HGLOBAL hMidiInHdr, hMidiInData;

	if (((uMidiInResult = midiInMessage(hMidiIn, MIDM_UNPREPARE,
		(DWORD) lpMidiInHdr, (DWORD) uMidiInHdrSize))
		== MMSYSERR_NOTSUPPORTED)
	 && (lpMidiInHdr != NULL)
	 && (uMidiInHdrSize >= sizeof(MIDIHDR)))
	{
		/* test MHDR_PREPARED bit */
		if (!(lpMidiInHdr->dwFlags & MHDR_PREPARED))
			return (MMSYSERR_NOERROR);
		/* unprepare header */
		if (!lpMidiInHdr->lpData
		 || !(hMidiInHdr = GlobalHandle(lpMidiInHdr))
		 || !(hMidiInData = GlobalHandle(lpMidiInHdr->lpData))
		 || !GlobalPageUnlock(hMidiInHdr))
			return (MMSYSERR_NOMEM);
		if (!GlobalPageUnlock(hMidiInData))
		{
			GlobalPageLock(hMidiInHdr);
			return (MMSYSERR_NOMEM);
		}
		/* reset MHDR_PREPARED bit */
		lpMidiInHdr->dwFlags &= ~MHDR_PREPARED;
		return (MMSYSERR_NOERROR);
	}

	return (uMidiInResult);

}

UINT	WINAPI
midiInAddBuffer(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT uMidiInHdrSize)
{
	return midiInMessage(hMidiIn, MIDM_ADDBUFFER,
		(DWORD) lpMidiInHdr, (DWORD) uMidiInHdrSize);
}

UINT	WINAPI
midiInReset(HMIDIIN hMidiIn)
{
	return midiInMessage(hMidiIn, MIDM_RESET, 0, 0);
}

UINT	WINAPI
midiInStart(HMIDIIN hMidiIn)
{
	return midiInMessage(hMidiIn, MIDM_START, 0, 0);
}

UINT	WINAPI
midiInStop(HMIDIIN hMidiIn)
{
	return midiInMessage(hMidiIn, MIDM_STOP, 0, 0);
}

UINT	WINAPI
midiInGetID(HMIDIIN hMidiIn, LPUINT lpMidiInDeviceID)
{
	UINT uMidiInResult;
	NPMIDIIN npMidiIn;

	/* get handle device ID */
	if (lpMidiInDeviceID)
	{
		if ((npMidiIn = midiInLock(hMidiIn, &uMidiInResult)))
		{
			*lpMidiInDeviceID = npMidiIn->uMidiInDeviceID;
			midiInUnlock(hMidiIn);
		}
		return (uMidiInResult);
	}
	return (MMSYSERR_INVALPARAM);

}

DWORD	WINAPI
midiInMessage(HMIDIIN hMidiIn, UINT uMsg, DWORD dwParam1, DWORD dwParam2)
{
	UINT uMidiInDeviceID;
	DWORD dwMidiInUser;

	if( (midiInGetID(hMidiIn, &uMidiInDeviceID)) &&
	    (midiInGetUser(hMidiIn, &dwMidiInUser))) {

		return midiInDeviceMessage(uMidiInDeviceID, uMsg,
			dwMidiInUser, dwParam1, dwParam2);
	}

	return 0;
}

UINT	WINAPI
midiInGetErrorText(UINT uError, LPSTR lpErrorText, UINT uErrorTextSize)
{
	UINT uMidiInResult = MMSYSERR_BADERRNUM;

	/* check error text buffer */
	if (!uErrorTextSize)
		return (MMSYSERR_NOERROR);
	if (!lpErrorText)
		return (MMSYSERR_INVALPARAM);

	/* get error text */
	if ( (uError >= MMSYSERR_BASE) &&
	     (uError <= MMSYSERR_LASTERROR) ) {

		LoadString(GetModuleHandle("mmsystem.dll"),
			uError, lpErrorText, uErrorTextSize);
	}

	/* get error text */
	if ( (uError >= MIDIERR_BASE) &&
	     (uError <= MIDIERR_LASTERROR) ) {
	
		LoadString(GetModuleHandle("mmsystem.dll"),
			uError, lpErrorText, uErrorTextSize);

	}

	/* return get error text result */
	return (uMidiInResult);

}

/* [API] (MMSYSTEM) Midi Out *********************************************** */

UINT	WINAPI
midiOutGetNumDevs()
{
	UINT u, uMidiOutNumDevs;

	/* open device drivers */
	if (!uNumMidiOutDrivers && !midiOutOpenDrivers())
		return (0);

	/* count total number of devices supported by device drivers */
	for (u = uMidiOutNumDevs = 0; u < uNumMidiOutDrivers; u++)
		uMidiOutNumDevs += uNumMidiOutDevices[u];
	return (uMidiOutNumDevs);

}

UINT	WINAPI
midiOutGetDevCaps(UINT uMidiOutDeviceID,
	LPMIDIOUTCAPS lpMidiOutCaps, UINT uMidiOutCapsSize)
{
	return midiOutDeviceMessage(uMidiOutDeviceID, MODM_GETDEVCAPS,
		0, (DWORD) lpMidiOutCaps, (DWORD) uMidiOutCapsSize);
}

UINT	WINAPI
midiOutGetVolume(UINT uMidiOutDeviceID, LPDWORD lpMidiOutVolume)
{
	return midiOutDeviceMessage(uMidiOutDeviceID, MODM_GETVOLUME,
		0, (DWORD) lpMidiOutVolume, 0);
}

UINT	WINAPI
midiOutSetVolume(UINT uMidiOutDeviceID, DWORD dwMidiOutVolume)
{
	return midiOutDeviceMessage(uMidiOutDeviceID, MODM_SETVOLUME,
		0, dwMidiOutVolume, 0);
}

UINT	WINAPI
midiOutOpen(LPHMIDIOUT lpMidiOut, UINT uMidiOutDeviceID,
	DWORD dwMidiOutCallback, DWORD dwMidiOutInstance,
	DWORD dwMidiOutOpenFlag)
{
	UINT uMidiOutResult;
	NPMIDIOUT npMidiOut;
	MIDIOPENDESC MidiOutOpenDesc;

	/* get device mapper ID */
	if ((uMidiOutDeviceID = midiOutDeviceMapper(uMidiOutDeviceID))
		== MIDI_MAPPER)
		return (MMSYSERR_BADDEVICEID);

	/* allocate and lock handle */
	if (!(npMidiOut = midiOutAlloc(lpMidiOut, &uMidiOutResult)))
 		return (uMidiOutResult);
	npMidiOut->uMidiOutSignature = MIDIOUT_SIGNATURE;
	npMidiOut->uMidiOutDeviceID = uMidiOutDeviceID;
	npMidiOut->dwMidiOutUser = 0;

	/* open handle */
	MidiOutOpenDesc.hMidi = *lpMidiOut;
	MidiOutOpenDesc.dwCallback = dwMidiOutCallback;
	MidiOutOpenDesc.dwInstance = dwMidiOutInstance;
	uMidiOutResult = midiOutDeviceMessage(uMidiOutDeviceID, MODM_OPEN,
		(DWORD) (LPDWORD) &npMidiOut->dwMidiOutUser,
		(DWORD) (LPMIDIOPENDESC) &MidiOutOpenDesc,
		dwMidiOutOpenFlag);

	/* unlock handle (and free handle if error) */
	midiOutUnlock(*lpMidiOut);
	if (uMidiOutResult)
		midiOutFree(*lpMidiOut);
	return (uMidiOutResult);

}

UINT	WINAPI
midiOutClose(HMIDIOUT hMidiOut)
{
	UINT uMidiOutResult;

	if (!(uMidiOutResult = midiOutMessage(hMidiOut, MODM_CLOSE, 0, 0)))
		midiOutFree(hMidiOut);

	return (uMidiOutResult);

}

UINT	WINAPI
midiOutPrepareHeader(HMIDIOUT hMidiOut,
	LPMIDIHDR lpMidiOutHdr, UINT uMidiOutHdrSize)
{
	UINT uMidiOutResult;
	HGLOBAL hMidiOutHdr, hMidiOutData;

	if (((uMidiOutResult = midiOutMessage(hMidiOut, MODM_PREPARE,
		(DWORD) lpMidiOutHdr, (DWORD) uMidiOutHdrSize))
		== MMSYSERR_NOTSUPPORTED)
	 && (lpMidiOutHdr != NULL)
	 && (uMidiOutHdrSize >= sizeof(MIDIHDR)))
	{
		/* test MHDR_PREPARED bit */
		if (lpMidiOutHdr->dwFlags & MHDR_PREPARED)
			return (MMSYSERR_NOERROR);
		/* prepare header */
		if (!lpMidiOutHdr->lpData
		 || !(hMidiOutHdr = GlobalHandle(lpMidiOutHdr))
		 || !(hMidiOutData = GlobalHandle(lpMidiOutHdr->lpData))
		 || !GlobalPageLock(hMidiOutHdr))
			return (MMSYSERR_NOMEM);
		if (!GlobalPageLock(hMidiOutData))
		{
			GlobalPageUnlock(hMidiOutHdr);
			return (MMSYSERR_NOMEM);
		}
		/* set MHDR_PREPARED bit */
		lpMidiOutHdr->dwFlags |= MHDR_PREPARED;
		return (MMSYSERR_NOERROR);
	}

	return (uMidiOutResult);

}

UINT	WINAPI
midiOutUnprepareHeader(HMIDIOUT hMidiOut,
	LPMIDIHDR lpMidiOutHdr, UINT uMidiOutHdrSize)
{
	UINT uMidiOutResult;
	HGLOBAL hMidiOutHdr, hMidiOutData;

	if (((uMidiOutResult = midiOutMessage(hMidiOut, MODM_UNPREPARE,
		(DWORD) lpMidiOutHdr, (DWORD) uMidiOutHdrSize))
		== MMSYSERR_NOTSUPPORTED)
	 && (lpMidiOutHdr != NULL)
	 && (uMidiOutHdrSize >= sizeof(MIDIHDR)))
	{
		/* test MHDR_PREPARED bit */
		if (!(lpMidiOutHdr->dwFlags & MHDR_PREPARED))
			return (MMSYSERR_NOERROR);
		/* unprepare header */
		if (!lpMidiOutHdr->lpData
		 || !(hMidiOutHdr = GlobalHandle(lpMidiOutHdr))
		 || !(hMidiOutData = GlobalHandle(lpMidiOutHdr->lpData))
		 || !GlobalPageUnlock(hMidiOutHdr))
			return (MMSYSERR_NOMEM);
		if (!GlobalPageUnlock(hMidiOutData))
		{
			GlobalPageLock(hMidiOutHdr);
			return (MMSYSERR_NOMEM);
		}
		/* reset MHDR_PREPARED bit */
		lpMidiOutHdr->dwFlags &= ~MHDR_PREPARED;
		return (MMSYSERR_NOERROR);
	}

	return (uMidiOutResult);

}

UINT	WINAPI
midiOutShortMsg(HMIDIOUT hMidiOut, DWORD dwMsg)
{
	return midiOutMessage(hMidiOut, MODM_DATA, dwMsg, 0);
}

UINT	WINAPI
midiOutLongMsg(HMIDIOUT hMidiOut,
	LPMIDIHDR lpMidiOutHdr, UINT uMidiOutHdrSize)
{
	return midiOutMessage(hMidiOut, MODM_LONGDATA,
		(DWORD) lpMidiOutHdr, (DWORD) uMidiOutHdrSize);
}

UINT	WINAPI
midiOutReset(HMIDIOUT hMidiOut)
{
	return midiOutMessage(hMidiOut, MODM_RESET, 0, 0);
}

UINT	WINAPI
midiOutCacheDrumPatches(HMIDIOUT hMidiOut, UINT uPatch, LPKEYARRAY lpKeyArray,
	UINT uCacheFlags)
{
	FatalAppExit(0, "midiOutCacheDrumPatches() unsupported");
	return 0;
}

UINT	WINAPI
midiOutCachePatches(HMIDIOUT hMidiOut, UINT uBank, LPPATCHARRAY lpPatchArray,
	UINT uCacheFlags)
{
	FatalAppExit(0, "midiOutCachePatches() unsupported");
	return 0;
}

UINT	WINAPI
midiOutGetID(HMIDIOUT hMidiOut, LPUINT lpMidiOutDeviceID)
{
	UINT uMidiOutResult;
	NPMIDIOUT npMidiOut;

	/* get handle device ID */
	if (lpMidiOutDeviceID)
	{
		if ((npMidiOut = midiOutLock(hMidiOut, &uMidiOutResult)))
		{
			*lpMidiOutDeviceID = npMidiOut->uMidiOutDeviceID;
			midiOutUnlock(hMidiOut);
		}
		return (uMidiOutResult);
	}
	return (MMSYSERR_INVALPARAM);

}

DWORD	WINAPI
midiOutMessage(HMIDIOUT hMidiOut, UINT uMsg, DWORD dwParam1, DWORD dwParam2)
{
	UINT uMidiOutResult, uMidiOutDeviceID;
	DWORD dwMidiOutUser;

	if ( (uMidiOutResult = midiOutGetID(hMidiOut, &uMidiOutDeviceID)) &&
	     (uMidiOutResult = midiOutGetUser(hMidiOut, &dwMidiOutUser))) {
	
		return midiOutDeviceMessage(uMidiOutDeviceID, uMsg,
			dwMidiOutUser, dwParam1, dwParam2);
	}
	return (uMidiOutResult);

}

UINT	WINAPI
midiOutGetErrorText(UINT uError, LPSTR lpErrorText, UINT uErrorTextSize)
{
	UINT uMidiOutResult = MMSYSERR_BADERRNUM;

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
	if ((uError >= MIDIERR_BASE) &&
	    (uError <= MIDIERR_LASTERROR) ) {
	
		LoadString(GetModuleHandle("mmsystem.dll"),
			uError, lpErrorText, uErrorTextSize);
	}

	/* return get error text result */
	return (uMidiOutResult);

}
