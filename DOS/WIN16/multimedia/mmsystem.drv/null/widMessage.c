/* 
 
 	@(#)widMessage.c	1.2  multimedia driver widMessage() function
  
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

static UINT XXXX_waveInGetNumDevs()
{
	return (0);
}

static UINT XXXX_waveInGetDevCaps(UINT uDeviceID,
	LPWAVEINCAPS lpWaveInCaps, DWORD dwWaveInCapsSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_waveInOpen(UINT uDeviceID, LPDWORD lpUser,
	LPWAVEOPENDESC lpWaveInOpenDesc, DWORD dwWaveInOpenFlag)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_waveInClose(UINT uDeviceID, DWORD dwUser)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_waveInAddBuffer(UINT uDeviceID, DWORD dwUser,
	LPWAVEHDR lpWaveInHdr, DWORD dwWaveInHdrSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_waveInReset(UINT uDeviceID, DWORD dwUser)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_waveInStart(UINT uDeviceID, DWORD dwUser)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_waveInStop(UINT uDeviceID, DWORD dwUser)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_waveInGetPos(UINT uDeviceID, DWORD dwUser,
	LPMMTIME lpMmTime, DWORD dwMmTimeSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_opt_waveInPrepare(UINT uDeviceID, DWORD dwUser,
	LPWAVEHDR lpWaveInHdr, DWORD dwWaveInHdrSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_opt_waveInUnprepare(UINT uDeviceID, DWORD dwUser,
	LPWAVEHDR lpWaveInHdr, DWORD dwWaveInHdrSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

/* [XxXxXxXx] END ********************************************************** */

DWORD
widMessage(UINT uDeviceID, UINT uMsg,
	DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
	LPWAVEINCAPS lpWaveInCaps;
	DWORD dwWaveInCapsSize;
	LPWAVEOPENDESC lpWaveInOpenDesc;
	DWORD dwWaveInOpenFlag;
	LPWAVEHDR lpWaveInHdr;
	DWORD dwWaveInHdrSize;
	LPMMTIME lpMmTime;
	DWORD dwMmTimeSize;

	switch (uMsg)
	{
	case WIDM_GETNUMDEVS:
		return XXXX_waveInGetNumDevs();
	case WIDM_GETDEVCAPS:
		lpWaveInCaps = (LPWAVEINCAPS) dwParam1;
		dwWaveInCapsSize = (DWORD) dwParam2;
		return XXXX_waveInGetDevCaps(uDeviceID,
			lpWaveInCaps, dwWaveInCapsSize);
	case WIDM_OPEN:
		lpWaveInOpenDesc = (LPWAVEOPENDESC) dwParam1;
		dwWaveInOpenFlag = (DWORD) dwParam2;
		return XXXX_waveInOpen(uDeviceID, (LPDWORD) dwUser,
			lpWaveInOpenDesc, dwWaveInOpenFlag);
	case WIDM_CLOSE:
		return XXXX_waveInClose(uDeviceID, dwUser);
	case WIDM_ADDBUFFER:
		lpWaveInHdr = (LPWAVEHDR) dwParam1;
		dwWaveInHdrSize = (DWORD) dwParam2;
		return XXXX_waveInAddBuffer(uDeviceID, dwUser,
			lpWaveInHdr, dwWaveInHdrSize);
	case WIDM_RESET:
		return XXXX_waveInReset(uDeviceID, dwUser);
	case WIDM_START:
		return XXXX_waveInStart(uDeviceID, dwUser);
	case WIDM_STOP:
		return XXXX_waveInStop(uDeviceID, dwUser);
	case WIDM_GETPOS:
		lpMmTime = (LPMMTIME) dwParam1;
		dwMmTimeSize = (DWORD) dwParam2;
		return XXXX_waveInGetPos(uDeviceID, dwUser,
			lpMmTime, dwMmTimeSize);
	case WIDM_PREPARE:					/* OPTIONAL */
		lpWaveInHdr = (LPWAVEHDR) dwParam1;
		dwWaveInHdrSize = (DWORD) dwParam2;
		return XXXX_opt_waveInPrepare(uDeviceID, dwUser,
			lpWaveInHdr, dwWaveInHdrSize);
	case WIDM_UNPREPARE:					/* OPTIONAL */
		lpWaveInHdr = (LPWAVEHDR) dwParam1;
		dwWaveInHdrSize = (DWORD) dwParam2;
		return XXXX_opt_waveInUnprepare(uDeviceID, dwUser,
			lpWaveInHdr, dwWaveInHdrSize);
	}

	return (MMSYSERR_NOTSUPPORTED);

}

