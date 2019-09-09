/*    
	Sound.c	2.8
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

#include "Log.h"

int WINAPI
OpenSound(void)
{
    APISTR((LF_APISTUB,"OpenSound: \n"));
	return TRUE;
}

void WINAPI
CloseSound(void)
{
    APISTR((LF_APISTUB,"CloseSound()\n"));
}

int WINAPI
SetVoiceAccent(int nVoice, int nTempo, int nVolume, int fnMode, int nPitch)
{
    APISTR((LF_APISTUB,"SetVoiceAccent(int=%d,int=%d,int=%d,int=%d,int=%d)\n", 
	nVoice, nTempo, nVolume, fnMode, nPitch));
    return TRUE;
}

int WINAPI
SetVoiceNote(int voice, int value, int length, int cdots)
{
    APISTR((LF_APISTUB,"SetVoiceNote(int=%d,int=%d,int=%d,int=%d)\n",
	voice, value, length, cdots));
    return TRUE;
}

int WINAPI
SetVoiceQueueSize(int nVoice, int cbQueue)
{
    APISTR((LF_APISTUB,"SetVoiceQueueSize(int=%d,int=%d)\n",
	nVoice, cbQueue));
    return TRUE;
}

int WINAPI
StartSound(void)
{
    APISTR((LF_APISTUB,"StartSound()\n"));
    return TRUE;
}

int WINAPI
StopSound(void)
{
    APISTR((LF_APISTUB,"StopSound()\n"));
    return TRUE;
}

int WINAPI
WaitSoundState(int fnState)
{
    APISTR((LF_APISTUB,"WaitSoundState(int=%d)\n",fnState));
    return TRUE;
}
