/*    
	Threads.c	1.3
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
#include "windowsx.h"
#include "Log.h"

/* (WIN32) Threads ********************************************************* */

BOOL WINAPI EnumThreadWindows(DWORD dwThreadId, WNDENUMPROC lpfn, 
			      LPARAM lParam)
{
    return EnumTaskWindows((HTASK) dwThreadId, lpfn, lParam);
}

DWORD WINAPI GetCurrentProcessId(void)
{
    return (DWORD) GetCurrentTask();
}

DWORD WINAPI GetWindowThreadProcessId(HWND hWnd, LPDWORD lpdwProcessId)
{
    HTASK hTask;
    
    hTask = GetWindowTask(hWnd);
    if (lpdwProcessId)
	*lpdwProcessId = (DWORD) hTask;
    
    return (DWORD) hTask;
}

BOOL WINAPI PostThreadMessage(DWORD idThread, UINT Msg, 
			      WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

/* (WIN32) Thread Local Storage ******************************************** */

static DWORD dwTlsIndex = 0;
static LPVOID TlsData[512];

DWORD	WINAPI
TlsAlloc(VOID)
{
	APISTR((LF_API, "TlsAlloc: (API)\n"));
	if (dwTlsIndex < sizeof(TlsData) / sizeof(TlsData[0]))
	{
		TlsData[dwTlsIndex] = NULL;
		return (dwTlsIndex++);
	}
	return (0xFFFFFFFFUL);
}

BOOL	WINAPI
TlsFree(DWORD dwTlsIndex)
{
	APISTR((LF_APISTUB, "TlsFree(DWORD=%ld)\n", dwTlsIndex));
	return (TRUE);
}

LPVOID	WINAPI
TlsGetValue(DWORD dwTlsIndex)
{
	APISTR((LF_API, "TlsGetValue: (API) dwTlsIndex %ld\n", dwTlsIndex));
	if (dwTlsIndex < sizeof(TlsData) / sizeof(TlsData[0]))
	{
		LOGSTR((LF_LOG, "TlsGetValue: (LOG) [%ld] = %p\n",
			dwTlsIndex, TlsData[dwTlsIndex]));
		SetLastError(NO_ERROR);
		return (TlsData[dwTlsIndex]);
	}
	SetLastErrorEx(1, 0);
	return (NULL);
}

BOOL	WINAPI
TlsSetValue(DWORD dwTlsIndex, LPVOID lpTlsValue)
{
	APISTR((LF_API, "TlsSetValue: (API) dwTlsIndex %ld lpTlsValue %p\n",
		dwTlsIndex, lpTlsValue));
	if (dwTlsIndex < sizeof(TlsData) / sizeof(TlsData[0]))
	{
		LOGSTR((LF_LOG, "TlsSetValue: (LOG) [%ld] = %p\n",
			dwTlsIndex, lpTlsValue));
		TlsData[dwTlsIndex] = lpTlsValue;
		return (TRUE);
	}
	return (FALSE);
}

/*************************************************************

GetThreadPriority()
{
	APISTR((LF_APISTUB, "GetThreadPriority()\n"));
}

SetThreadPriority()
{
	APISTR((LF_APISTUB, "SetThreadPriority()\n"));
}

ResumeThread()
{
	APISTR((LF_APISTUB, "ResumeThread()\n"));
}

void *GetCurrentThread()
{
	APISTR((LF_APISTUB, "GetCurrentThread()\n"));
	return (0);
}

DWORD GetProcessVersion(UINT x)
{
	APISTR((LF_APISTUB, "GetProcessVersion()\n"));
}

LCID WINAPI
GetThreadLocale(void)
{
	APISTR((LF_APISTUB, "GetThreadLocale()\n")):
}

SuspendThread()
{
	APISTR((LF_APISTUB, "SuspendThread()\n")):
}

**************************************************************************/
