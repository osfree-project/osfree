/* 
 @(#)auxMessage.c	1.2  multimedia driver auxMessage() function

  
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

static UINT XXXX_auxGetNumDevs()
{
	return (0);
}

static UINT XXXX_auxGetDevCaps(UINT uDeviceID,
	LPAUXCAPS lpAuxCaps, DWORD dwAuxCapsSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_opt_auxGetVolume(UINT uDeviceID,
	LPDWORD lpAuxVolume)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_opt_auxSetVolume(UINT uDeviceID,
	DWORD dwAuxVolume)
{
	return (MMSYSERR_NOTSUPPORTED);
}

/* [XxXxXxXx] END ********************************************************** */

DWORD
auxMessage(UINT uDeviceID, UINT uMsg,
	DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
	LPAUXCAPS lpAuxCaps;
	DWORD dwAuxCapsSize;
	LPDWORD lpAuxVolume;
	DWORD dwAuxVolume;
	WORD wAuxVolumeL, wAuxVolumeR;

	switch (uMsg)
	{
	case AUXDM_GETNUMDEVS:
		return XXXX_auxGetNumDevs();
	case AUXDM_GETDEVCAPS:
		lpAuxCaps = (LPAUXCAPS) dwParam1;
		dwAuxCapsSize = (DWORD) dwParam2;
		return XXXX_auxGetDevCaps(uDeviceID,
			lpAuxCaps, dwAuxCapsSize);
	case AUXDM_GETVOLUME:					/* OPTIONAL */
		lpAuxVolume = (LPDWORD) dwParam1;
		return XXXX_opt_auxGetVolume(uDeviceID,
			lpAuxVolume);
	case AUXDM_SETVOLUME:					/* OPTIONAL */
		dwAuxVolume = (DWORD) dwParam1;
		wAuxVolumeL = LOWORD(dwAuxVolume);
		wAuxVolumeR = HIWORD(dwAuxVolume);
		return XXXX_opt_auxSetVolume(uDeviceID,
			dwAuxVolume);
	}

	return (MMSYSERR_NOTSUPPORTED);

}

