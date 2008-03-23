/*
 
 @(#)midMessage.c	1.2  - multimedia driver midMessage() function

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

static UINT XXXX_midiInGetNumDevs()
{
	return (0);
}

static UINT XXXX_midiInGetDevCaps(UINT uDeviceID,
	LPMIDIINCAPS lpMidiInCaps, DWORD dwMidiInCapsSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_midiInOpen(UINT uDeviceID, LPDWORD lpUser,
	LPMIDIOPENDESC lpMidiInOpenDesc, DWORD dwMidiInOpenFlag)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_midiInClose(UINT uDeviceID, DWORD dwUser)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_midiInAddBuffer(UINT uDeviceID, DWORD dwUser,
	LPMIDIHDR lpMidiInHdr, DWORD dwMidiInHdrSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_midiInReset(UINT uDeviceID, DWORD dwUser)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_midiInStart(UINT uDeviceID, DWORD dwUser)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_midiInStop(UINT uDeviceID, DWORD dwUser)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_opt_midiInPrepare(UINT uDeviceID, DWORD dwUser,
	LPMIDIHDR lpMidiInHdr, DWORD dwMidiInHdrSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

static UINT XXXX_opt_midiInUnprepare(UINT uDeviceID, DWORD dwUser,
	LPMIDIHDR lpMidiInHdr, DWORD dwMidiInHdrSize)
{
	return (MMSYSERR_NOTSUPPORTED);
}

/* [XxXxXxXx] END ********************************************************** */

DWORD
midMessage(UINT uDeviceID, UINT uMsg,
	DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
	LPMIDIINCAPS lpMidiInCaps;
	DWORD dwMidiInCapsSize;
	LPMIDIOPENDESC lpMidiInOpenDesc;
	DWORD dwMidiInOpenFlag;
	LPMIDIHDR lpMidiInHdr;
	DWORD dwMidiInHdrSize;

	switch (uMsg)
	{
	case MIDM_GETNUMDEVS:
		return XXXX_midiInGetNumDevs();
	case MIDM_GETDEVCAPS:
		lpMidiInCaps = (LPMIDIINCAPS) dwParam1;
		dwMidiInCapsSize = (DWORD) dwParam2;
		return XXXX_midiInGetDevCaps(uDeviceID,
			lpMidiInCaps, dwMidiInCapsSize);
	case MIDM_OPEN:
		lpMidiInOpenDesc = (LPMIDIOPENDESC) dwParam1;
		dwMidiInOpenFlag = (DWORD) dwParam2;
		return XXXX_midiInOpen(uDeviceID, (LPDWORD) dwUser,
			lpMidiInOpenDesc, dwMidiInOpenFlag);
	case MIDM_CLOSE:
		return XXXX_midiInClose(uDeviceID, dwUser);
	case MIDM_ADDBUFFER:
		lpMidiInHdr = (LPMIDIHDR) dwParam1;
		dwMidiInHdrSize = (DWORD) dwParam2;
		return XXXX_midiInAddBuffer(uDeviceID, dwUser,
			lpMidiInHdr, dwMidiInHdrSize);
	case MIDM_RESET:
		return XXXX_midiInReset(uDeviceID, dwUser);
	case MIDM_START:
		return XXXX_midiInStart(uDeviceID, dwUser);
	case MIDM_STOP:
		return XXXX_midiInStop(uDeviceID, dwUser);
	case MIDM_PREPARE:					/* OPTIONAL */
		lpMidiInHdr = (LPMIDIHDR) dwParam1;
		dwMidiInHdrSize = (DWORD) dwParam2;
		return XXXX_opt_midiInPrepare(uDeviceID, dwUser,
			lpMidiInHdr, dwMidiInHdrSize);
	case MIDM_UNPREPARE:					/* OPTIONAL */
		lpMidiInHdr = (LPMIDIHDR) dwParam1;
		dwMidiInHdrSize = (DWORD) dwParam2;
		return XXXX_opt_midiInUnprepare(uDeviceID, dwUser,
			lpMidiInHdr, dwMidiInHdrSize);
	}

	return (MMSYSERR_NOTSUPPORTED);

}

