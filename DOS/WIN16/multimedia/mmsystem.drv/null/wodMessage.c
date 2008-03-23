/* 
 
    @(#)wodMessage.c	1.2  multimedia driver wodMessage() function
  
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

static UINT XXXX_waveOutGetNumDevs()
{
	return (0);
}

static UINT XXXX_waveOutGetDevCaps(UINT uDeviceID,
	LPWAVEOUTCAPS lpWaveOutCaps, DWORD dwWaveOutCapsSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_waveOutOpen(UINT uDeviceID, LPDWORD lpUser,
	LPWAVEOPENDESC lpWaveOutOpenDesc, DWORD dwWaveOutOpenFlag)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_waveOutClose(UINT uDeviceID, DWORD dwUser)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_waveOutWrite(UINT uDeviceID, DWORD dwUser,
	LPWAVEHDR lpWaveOutHdr, DWORD dwWaveOutHdrSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_waveOutReset(UINT uDeviceID, DWORD dwUser)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_waveOutPause(UINT uDeviceID, DWORD dwUser)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_waveOutRestart(UINT uDeviceID, DWORD dwUser)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_waveOutBreakLoop(UINT uDeviceID, DWORD dwUser)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_waveOutGetPos(UINT uDeviceID, DWORD dwUser,
	LPMMTIME lpMmTime, DWORD dwMmTimeSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_opt_waveOutPrepare(UINT uDeviceID, DWORD dwUser,
	LPWAVEHDR lpWaveOutHdr, DWORD dwWaveOutHdrSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_opt_waveOutUnprepare(UINT uDeviceID, DWORD dwUser,
	LPWAVEHDR lpWaveOutHdr, DWORD dwWaveOutHdrSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_opt_waveOutGetVolume(UINT uDeviceID,
	LPDWORD lpWaveOutVolume)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_opt_waveOutSetVolume(UINT uDeviceID,
	DWORD dwWaveOutVolume)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_opt_waveOutGetPitch(UINT uDeviceID, DWORD dwUser,
	LPDWORD lpWaveOutPitch)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_opt_waveOutSetPitch(UINT uDeviceID, DWORD dwUser,
	DWORD dwWaveOutPitch)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_opt_waveOutGetPlaybackRate(UINT uDeviceID, DWORD dwUser,
	LPDWORD lpWaveOutPlaybackRate)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_opt_waveOutSetPlaybackRate(UINT uDeviceID, DWORD dwUser,
	DWORD dwWaveOutPlaybackRate)
{
	return (MMSYSERR_NOTSUPPORTED);
}

/* [XxXxXxXx] END ********************************************************** */

DWORD
wodMessage(UINT uDeviceID, UINT uMsg,
	DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
	LPWAVEOUTCAPS lpWaveOutCaps;
	DWORD dwWaveOutCapsSize;
	LPWAVEOPENDESC lpWaveOutOpenDesc;
	DWORD dwWaveOutOpenFlag;
	LPWAVEHDR lpWaveOutHdr;
	DWORD dwWaveOutHdrSize;
	LPMMTIME lpMmTime;
	DWORD dwMmTimeSize;
	LPDWORD lpWaveOutVolume;
	DWORD dwWaveOutVolume;
	LPDWORD lpWaveOutPitch;
	DWORD dwWaveOutPitch;
	LPDWORD lpWaveOutPlaybackRate;
	DWORD dwWaveOutPlaybackRate;

	switch (uMsg)
	{
	case WODM_GETNUMDEVS:
		return XXXX_waveOutGetNumDevs();
	case WODM_GETDEVCAPS:
		lpWaveOutCaps = (LPWAVEOUTCAPS) dwParam1;
		dwWaveOutCapsSize = (DWORD) dwParam2;
		return XXXX_waveOutGetDevCaps(uDeviceID,
			lpWaveOutCaps, dwWaveOutCapsSize);
	case WODM_OPEN:
		lpWaveOutOpenDesc = (LPWAVEOPENDESC) dwParam1;
		dwWaveOutOpenFlag = (DWORD) dwParam2;
		return XXXX_waveOutOpen(uDeviceID, (LPDWORD) dwUser,
			lpWaveOutOpenDesc, dwWaveOutOpenFlag);
	case WODM_CLOSE:
		return XXXX_waveOutClose(uDeviceID, dwUser);
	case WODM_WRITE:
		lpWaveOutHdr = (LPWAVEHDR) dwParam1;
		dwWaveOutHdrSize = (DWORD) dwParam2;
		return XXXX_waveOutWrite(uDeviceID, dwUser,
			lpWaveOutHdr, dwWaveOutHdrSize);
	case WODM_RESET:
		return XXXX_waveOutReset(uDeviceID, dwUser);
	case WODM_PAUSE:
		return XXXX_waveOutPause(uDeviceID, dwUser);
	case WODM_RESTART:
		return XXXX_waveOutRestart(uDeviceID, dwUser);
	case WODM_BREAKLOOP:
		return XXXX_waveOutBreakLoop(uDeviceID, dwUser);
	case WODM_GETPOS:
		lpMmTime = (LPMMTIME) dwParam1;
		dwMmTimeSize = (DWORD) dwParam2;
		return XXXX_waveOutGetPos(uDeviceID, dwUser,
			lpMmTime, dwMmTimeSize);
	case WODM_PREPARE:					/* OPTIONAL */
		lpWaveOutHdr = (LPWAVEHDR) dwParam1;
		dwWaveOutHdrSize = (DWORD) dwParam2;
		return XXXX_opt_waveOutPrepare(uDeviceID, dwUser,
			lpWaveOutHdr, dwWaveOutHdrSize);
	case WODM_UNPREPARE:					/* OPTIONAL */
		lpWaveOutHdr = (LPWAVEHDR) dwParam1;
		dwWaveOutHdrSize = (DWORD) dwParam2;
		return XXXX_opt_waveOutUnprepare(uDeviceID, dwUser,
			lpWaveOutHdr, dwWaveOutHdrSize);
	case WODM_GETVOLUME:					/* OPTIONAL */
		lpWaveOutVolume = (LPDWORD) dwParam1;
		return XXXX_opt_waveOutGetVolume(uDeviceID,
			lpWaveOutVolume);
	case WODM_SETVOLUME:					/* OPTIONAL */
		dwWaveOutVolume = (DWORD) dwParam1;
		return XXXX_opt_waveOutSetVolume(uDeviceID,
			dwWaveOutVolume);
	case WODM_GETPITCH:					/* OPTIONAL */
		lpWaveOutPitch = (LPDWORD) dwParam1;
		return XXXX_opt_waveOutGetPitch(uDeviceID, dwUser,
			lpWaveOutPitch);
	case WODM_SETPITCH:					/* OPTIONAL */
		dwWaveOutPitch = (DWORD) dwParam1;
		return XXXX_opt_waveOutSetPitch(uDeviceID, dwUser,
			dwWaveOutPitch);
	case WODM_GETPLAYBACKRATE:				/* OPTIONAL */
		lpWaveOutPlaybackRate = (LPDWORD) dwParam1;
		return XXXX_opt_waveOutGetPlaybackRate(uDeviceID, dwUser,
			lpWaveOutPlaybackRate);
	case WODM_SETPLAYBACKRATE:				/* OPTIONAL */
		dwWaveOutPlaybackRate = (DWORD) dwParam1;
		return XXXX_opt_waveOutSetPlaybackRate(uDeviceID, dwUser,
			dwWaveOutPlaybackRate);
	}

	return (MMSYSERR_NOTSUPPORTED);

}

