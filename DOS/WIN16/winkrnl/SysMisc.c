/*    
	SysMisc.c	1.12
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

#include "Log.h"
#include "kerndef.h"
#include "Kernel.h"
#include "Driver.h"

/* exported stuff */
BOOL TWIN_GetTimerMsg(HWND, HTASK, LPMSG, UINT);
int TWIN_GetTimeout(int);

/* internal routines */
static UINT SetTimer2(HWND, UINT, UINT, TIMERPROC, BOOL);
static BOOL KillTimer2(HWND, UINT, BOOL);

UINT WINAPI
GetDoubleClickTime(void)
{
    UINT uiRet;

    APISTR((LF_APICALL,"GetDoubleClickTime()\n"));

    uiRet = (UINT)DRVCALL_EVENTS(PEH_DBLCLKTIME,0,0,0);

    APISTR((LF_APIRET,"GetDoubleClickTime: return UINT %x\n",uiRet));

    return uiRet;
}

void WINAPI
SetDoubleClickTime(UINT uInterval)
{

    APISTR((LF_APICALL,"SetDoubleClickTime(UINT=%x)\n",uInterval));
    (void)DRVCALL_EVENTS(PEH_DBLCLKTIME,1,uInterval,0);
    APISTR((LF_APIRET,"SetDoubleClickTime: returns void\n"));
}

BOOL WINAPI
SwapMouseButton(BOOL fSwap)
{
    BOOL rc;
    LOGSTR((LF_APICALL,"SwapMouseButton(BOOL=%x)\n",fSwap));
    rc =  (BOOL)DRVCALL_EVENTS(PEH_MOUSEBUTTON,fSwap,0,0);
    LOGSTR((LF_APIRET,"SwapMouseButton: returns BOOL %x\n",rc));
    return rc;
}

typedef struct tagSYNCTIMER
{
    HTASK	hTask;
    HWND	hWnd;
    UINT	uID;
    UINT	uTimeout;
    UINT	uFlags;
    int		nWaitTime;
    TIMERPROC	lpfnTimerProc;
} SYNCTIMER;
typedef SYNCTIMER *LPSYNCTIMER;

#define	STF_SYSTEM	0x0001

#define	SYNC_TIMERS	48
static SYNCTIMER SyncTimers[SYNC_TIMERS];

UINT WINAPI
SetTimer(HWND hWnd, UINT idTimer, UINT uTimeout, TIMERPROC tmprc)
{
    UINT rc;
    APISTR((LF_APICALL,"SetTimer(HWND=%x,UINT=%x,UINT=%x,TIMERPROC=%x)\n",
	hWnd,idTimer,uTimeout,tmprc));
    rc = SetTimer2(hWnd,idTimer,uTimeout,tmprc,FALSE);
    APISTR((LF_APIRET,"SetTimer: returns UINT %x\n",rc));
    return rc;
}

UINT WINAPI
SetSystemTimer(HWND hWnd, UINT idTimer, UINT uTimeout, TIMERPROC tmprc)
{
    UINT rc;
    APISTR((LF_APICALL,"SetSystemTimer(HWND=%x,UINT=%x,UINT=%x,TIMERPROC=%x)\n",
	hWnd,idTimer,uTimeout,tmprc));
    rc = SetTimer2(hWnd,idTimer,uTimeout,tmprc,TRUE);
    APISTR((LF_APIRET,"SetSystemTimer: returns UINT %x\n",rc));
    return rc;
}

static int nMaxTimer = 0;

static UINT
SetTimer2(HWND hWnd, UINT idTimer, UINT uTime,
		TIMERPROC tmprc, BOOL bSystem)
{
    LPSYNCTIMER lpSyncTimer;
    HTASK hTask;
    int n;
    UINT uTimeout = (uTime)?uTime:20;

    if (hWnd != 0 && IsWindow(hWnd))
	hTask = GetWindowTask(hWnd);
    else
	hTask = GetCurrentTask();

    for (n = 0; n < SYNC_TIMERS; n++)
	if (SyncTimers[n].hTask == 0) {
	    lpSyncTimer = &SyncTimers[n];
	    lpSyncTimer->hTask = hTask;
	    lpSyncTimer->hWnd = hWnd;
	    lpSyncTimer->uID = idTimer;
	    lpSyncTimer->uTimeout = uTimeout;
	    lpSyncTimer->uFlags = (bSystem)?STF_SYSTEM:0;
	    lpSyncTimer->nWaitTime = (int)uTimeout;
	    lpSyncTimer->lpfnTimerProc = tmprc;

	    if (hWnd == 0)
		lpSyncTimer->uID = n + 1;
	    if (n > nMaxTimer)
		nMaxTimer = n;
	    return (lpSyncTimer->uID) ? lpSyncTimer->uID : 1;
	}

    return 0;
}

BOOL WINAPI
KillTimer(HWND hWnd, UINT uID)
{
    BOOL rc;
    APISTR((LF_APICALL,"KillTimer(HWND=%x,UINT=%x)\n",hWnd,uID));
    rc = KillTimer2(hWnd,uID,FALSE);
    APISTR((LF_APIRET,"KillTimer: returns BOOL %x\n",rc));
    return rc;
}

BOOL WINAPI
UserKillSystemTimer(HWND hWnd, UINT uID)
{
    BOOL rc;
    APISTR((LF_APICALL,"UserKillSystemTimer(HWND=%x,UINT=%x)\n",hWnd,uID));
    rc = KillTimer2(hWnd,uID,TRUE);
    APISTR((LF_APIRET,"UserKillSystemTimer: returns BOOL %x\n",rc));
    return rc;
}

static BOOL
KillTimer2(HWND hWnd, UINT uID, BOOL bSystem)
{
    HTASK hTask;
    int n;

    if (hWnd != 0 && IsWindow(hWnd))
	hTask = GetWindowTask(hWnd);
    else
	hTask = GetCurrentTask();

    for (n = 0; n < SYNC_TIMERS; n++)
	if (SyncTimers[n].hTask == hTask) {
	    if (SyncTimers[n].hWnd == hWnd && SyncTimers[n].uID == uID) {
		SyncTimers[n].hTask = 0;
		if (nMaxTimer && n == nMaxTimer) {
		    nMaxTimer--;
		    while (nMaxTimer && SyncTimers[nMaxTimer].hTask == 0)
			nMaxTimer--;
		}
		return TRUE;
	    }
	}

    return FALSE;
}

BOOL
TWIN_GetTimerMsg(HWND hWnd, HTASK hTask, LPMSG lpMsg, UINT uFlg)
{
    static int nOffset = 0;
    int n,nIndex;
    LPSYNCTIMER lpSyncTimer;
    BOOL bSystem;

    for (n = 0; n <= nMaxTimer; n++) {
	nIndex = (n+nOffset) % (nMaxTimer+1);
	if (SyncTimers[nIndex].hTask == 0)
	    continue;
	lpSyncTimer = &SyncTimers[nIndex];
	if (lpSyncTimer->hTask == hTask && lpSyncTimer->nWaitTime <= 0) {
	    if (hWnd == 0 || lpSyncTimer->hWnd == hWnd) {
		bSystem = (lpSyncTimer->uFlags & STF_SYSTEM)?TRUE:FALSE;
		memset((LPSTR)lpMsg,'\0',sizeof(MSG));
		lpMsg->hwnd = lpSyncTimer->hWnd;
		lpMsg->message = (bSystem)?WM_SYSTIMER:WM_TIMER;
		lpMsg->wParam = (WPARAM)lpSyncTimer->uID;
		lpMsg->lParam = (LPARAM)lpSyncTimer->lpfnTimerProc;
		if (uFlg & PM_REMOVE)
		    lpSyncTimer->nWaitTime = (int)lpSyncTimer->uTimeout;
		nOffset = ++nOffset % (nMaxTimer+1);
		return TRUE;
	    }
	}
    }
    QueueClearFlags(hTask, QFTIMER);
    nOffset = ++nOffset % (nMaxTimer+1);
    return FALSE;
}

int
TWIN_GetTimeout(int nTimeout)
{
    int n;
    int nMinTimeout = 0;
    LPSYNCTIMER lpSyncTimer;

    for (n = 0; n < SYNC_TIMERS; n++) {
	lpSyncTimer = &SyncTimers[n];
	if (lpSyncTimer->hTask == 0)
	    continue;
#ifdef	LATER
	/* sol.exe timers get killed on menu select if we test for 0 here */
	if (lpSyncTimer->nWaitTime == 0)
	    continue;
#endif

	lpSyncTimer->nWaitTime -= nTimeout;

	if (lpSyncTimer->nWaitTime <= 0) {
	    QueueSetFlags(lpSyncTimer->hTask,QFTIMER);
	    lpSyncTimer->nWaitTime = 0;
	    nMinTimeout = -1;
	    continue;
	}

	if (nMinTimeout == 0 || lpSyncTimer->nWaitTime < nMinTimeout)
	    nMinTimeout = lpSyncTimer->nWaitTime;

    }

    return nMinTimeout;
}

DWORD WINAPI
GetCurrentTime()
{
    DWORD rc;
    APISTR((LF_APICALL,"GetCurrentTime()\n"));
    rc = DRVCALL_EVENTS(PEH_SYSTIME,0,0,0);
    APISTR((LF_APIRET,"GetCurrentTime: returns DWORD %x\n",rc));
    return rc;
}

DWORD WINAPI
GetTickCount()
{
    DWORD rc;
    APISTR((LF_APICALL,"GetTickCount()\n"));
    rc =  DRVCALL_EVENTS(PEH_SYSTIME,0,0,0);
    APISTR((LF_APIRET,"GetTickCount: returns DWORD %x\n",rc));
    return rc;
}
