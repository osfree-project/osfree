/*    
	MultiMedia.c	2.4
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
#include <string.h>

#include "windows.h"
/*
#include "mmsystem.h"
*/
#define __MMSYSTEM_H__ 
#include "Willows.h"
#include "Log.h"

UINT WINAPI
timeBeginPeriod(UINT uPeriod)
{
    APISTR((LF_APISTUB,"timeBeginPeriod(UINT=%x)\n",uPeriod));
    return 0;
}


UINT WINAPI
timeEndPeriod(UINT uPeriod)
{
    APISTR((LF_APISTUB,"timeEndPeriod(UINT=%x)\n",uPeriod));
    return 0;
}

UINT WINAPI
timeGetDevCaps(LPTIMECAPS lpTimeCaps, UINT uSize)
{
    APISTR((LF_APISTUB,"timeGetDevCaps(LPTIMECAPS=%p,UINT=%d)\n",uSize));
    lpTimeCaps->wPeriodMin = 1000;	/* milliseconds */
    lpTimeCaps->wPeriodMax = 65535;	/* milliseconds */
    return 0;
}

UINT WINAPI
timeGetSystemTime(LPMMTIME lpTime, UINT uSize)
{
    APISTR((LF_APISTUB,"timeGetSystemTime(LPMMTIME=%p,UINT=%x)\n",uSize));
    lpTime->wType = TIME_MS;
    lpTime->u.ms = GetCurrentTime();
    return 0;
}

DWORD WINAPI
timeGetTime()
{
    DWORD	ctime;
    APISTR((LF_APICALL,"timeGetTime()\n"));
    ctime =  GetCurrentTime();
    APISTR((LF_APIRET,"timeGetTime: returns DWORD %lx\n",ctime));
    return ctime;
}

typedef struct tagMMTIMER
{
	LPTIMECALLBACK	lpCallBack;
	DWORD		dwUser;
	UINT		uFlags;
} MMTIMER;
static MMTIMER mmTimers[16];

static void CALLBACK
mmTimerCallback(HWND hWnd, UINT msg, UINT idTimer, DWORD dwTime)
{
    if (mmTimers[idTimer-1].lpCallBack == 0)
	return;
    mmTimers[idTimer-1].lpCallBack(idTimer,msg,mmTimers[idTimer-1].dwUser,
					0L,0L);
    if (mmTimers[idTimer-1].uFlags == TIME_ONESHOT)
	timeKillEvent(idTimer);
}

UINT WINAPI
timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpFunction,
		DWORD dwUser, UINT uFlags)
{
    UINT uIDTimer;

    APISTR((LF_APICALL,
	"timeSetEvent(UINT=%x,UINT=%x,LPPROC=%p,DWORD=%x,UINT=%x\n",
	uDelay,uResolution,lpFunction,dwUser,uFlags));

    uIDTimer = SetTimer((HWND)0,0,uDelay,(TIMERPROC)mmTimerCallback);

    if (uIDTimer == 0) {
    	APISTR((LF_APIFAIL,"timeSetEvent: returns UINT %x\n",0));
	return 0;
    }

    mmTimers[uIDTimer-1].lpCallBack = lpFunction;
    mmTimers[uIDTimer-1].dwUser = dwUser;
    mmTimers[uIDTimer-1].uFlags = uFlags; 

    APISTR((LF_APIRET,"timeSetEvent: returns UINT %x\n",uIDTimer));
    return uIDTimer;
}

UINT WINAPI
timeKillEvent(UINT uTimerID)
{
    APISTR((LF_APICALL,"timeKillEvent(UINT=%x)\n",uTimerID));

    if (uTimerID == 0 || uTimerID >= 16) {
    	APISTR((LF_APIFAIL,"timeKillEvent: returns UINT %x\n",TIMERR_NOCANDO));
	return TIMERR_NOCANDO;
    }

    KillTimer((HWND)0,uTimerID);
    memset((LPSTR)&mmTimers[uTimerID-1],'\0',sizeof(MMTIMER));
    APISTR((LF_APIRET,"timeKillEvent: returns UINT %x\n",0));
    return 0;
}


BOOL WINAPI
sndPlaySound(LPCSTR lpSoundName, UINT Options)
{
    APISTR((LF_APISTUB,"sndPlaySound(LPCSTR=%s,UINT=%x)\n",
	lpSoundName?lpSoundName:"NULL", Options));
    return TRUE;
}

