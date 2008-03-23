/* 
 
    @(#)modMessage.c	1.2 - multimedia driver modMessage() function
  
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

static UINT XXXX_midiOutGetNumDevs()
{
	return (0);
}

static UINT XXXX_midiOutGetDevCaps(UINT uDeviceID,
	LPMIDIOUTCAPS lpMidiOutCaps, DWORD dwMidiOutCapsSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_midiOutOpen(UINT uDeviceID, LPDWORD lpUser,
	LPMIDIOPENDESC lpMidiOutOpenDesc, DWORD dwMidiOutOpenFlag)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_midiOutClose(UINT uDeviceID, DWORD dwUser)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_midiOutData(UINT uDeviceID, DWORD dwUser,
	DWORD dwMidiOutMsg)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_midiOutLongData(UINT uDeviceID, DWORD dwUser,
	LPMIDIHDR lpMidiOutHdr, DWORD dwMidiOutHdrSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_midiOutReset(UINT uDeviceID, DWORD dwUser)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_opt_midiOutPrepare(UINT uDeviceID, DWORD dwUser,
	LPMIDIHDR lpMidiOutHdr, DWORD dwMidiOutHdrSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_opt_midiOutUnprepare(UINT uDeviceID, DWORD dwUser,
	LPMIDIHDR lpMidiOutHdr, DWORD dwMidiOutHdrSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_opt_midiOutGetVolume(UINT uDeviceID,
	LPDWORD lpMidiOutVolume)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_opt_midiOutSetVolume(UINT uDeviceID,
	DWORD dwMidiOutVolume)
{
	return (MMSYSERR_NOTSUPPORTED);
}

/* [XxXxXxXx] END ********************************************************** */

DWORD
modMessage(UINT uDeviceID, UINT uMsg,
	DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
	LPMIDIOUTCAPS lpMidiOutCaps;
	DWORD dwMidiOutCapsSize;
	LPMIDIOPENDESC lpMidiOutOpenDesc;
	DWORD dwMidiOutOpenFlag;
	DWORD dwMidiOutMsg;
	LPMIDIHDR lpMidiOutHdr;
	DWORD dwMidiOutHdrSize;
	LPDWORD lpMidiOutVolume;
	DWORD dwMidiOutVolume;

	switch (uMsg)
	{
	case MODM_GETNUMDEVS:
		return XXXX_midiOutGetNumDevs();
	case MODM_GETDEVCAPS:
		lpMidiOutCaps = (LPMIDIOUTCAPS) dwParam1;
		dwMidiOutCapsSize = (DWORD) dwParam2;
		return XXXX_midiOutGetDevCaps(uDeviceID,
			lpMidiOutCaps, dwMidiOutCapsSize);
	case MODM_OPEN:
		lpMidiOutOpenDesc = (LPMIDIOPENDESC) dwParam1;
		dwMidiOutOpenFlag = (DWORD) dwParam2;
		return XXXX_midiOutOpen(uDeviceID, (LPDWORD) dwUser,
			lpMidiOutOpenDesc, dwMidiOutOpenFlag);
	case MODM_CLOSE:
		return XXXX_midiOutClose(uDeviceID, dwUser);
	case MODM_DATA:
		dwMidiOutMsg = (DWORD) dwParam1;
		return XXXX_midiOutData(uDeviceID, dwUser,
			dwMidiOutMsg);
	case MODM_LONGDATA:
		lpMidiOutHdr = (LPMIDIHDR) dwParam1;
		dwMidiOutHdrSize = (DWORD) dwParam2;
		return XXXX_midiOutLongData(uDeviceID, dwUser,
			lpMidiOutHdr, dwMidiOutHdrSize);
	case MODM_RESET:
		return XXXX_midiOutReset(uDeviceID, dwUser);
	case MODM_PREPARE:					/* OPTIONAL */
		lpMidiOutHdr = (LPMIDIHDR) dwParam1;
		dwMidiOutHdrSize = (DWORD) dwParam2;
		return XXXX_opt_midiOutPrepare(uDeviceID, dwUser,
			lpMidiOutHdr, dwMidiOutHdrSize);
	case MODM_UNPREPARE:					/* OPTIONAL */
		lpMidiOutHdr = (LPMIDIHDR) dwParam1;
		dwMidiOutHdrSize = (DWORD) dwParam2;
		return XXXX_opt_midiOutUnprepare(uDeviceID, dwUser,
			lpMidiOutHdr, dwMidiOutHdrSize);
	case MODM_GETVOLUME:					/* OPTIONAL */
		lpMidiOutVolume = (LPDWORD) dwParam1;
		return XXXX_opt_midiOutGetVolume(uDeviceID,
			lpMidiOutVolume);
	case MODM_SETVOLUME:					/* OPTIONAL */
		dwMidiOutVolume = (DWORD) dwParam1;
		return XXXX_opt_midiOutSetVolume(uDeviceID,
			dwMidiOutVolume);
	}

	return (MMSYSERR_NOTSUPPORTED);

}

