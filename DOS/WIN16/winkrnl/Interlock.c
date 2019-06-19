/*    
	Interlock.c	1.3
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

/* (WIN32) Interlocked Arithmetic ****************************************** */

/*
 * WARNING: These functions are provided as functional, but not thread-safe!
 */

LONG WINAPI
InterlockedIncrement(LPLONG lpAddend)
{
    return ++(*lpAddend);
}

LONG WINAPI
InterlockedDecrement(LPLONG lpAddend)
{
    return --(*lpAddend);
}

LONG WINAPI
InterlockedExchange(LPLONG lpAddend, LONG Value)
{
    LONG OldValue = *lpAddend;

    *lpAddend = Value;

    return OldValue;
}

LONG WINAPI
InterlockedExchangeAdd(LPLONG lpAddend, LONG Value)
{
    LONG OldValue = *lpAddend;

    *lpAddend += Value;

    return OldValue;
}

PVOID WINAPI
InterlockedCompareExchange(PVOID *lpAddend, PVOID Value, PVOID Compare)
{
    PVOID OldValue = *lpAddend;

    if ( *lpAddend == Compare )
	*lpAddend = Value;

    return OldValue;
}

/* (WIN32) Synchronization ************************************************* */

DWORD	WINAPI
WaitForSingleObject(HANDLE hObject, DWORD dwTimeOut)
{
	APISTR((LF_API, "WaitForSingleObject:"
		" (API) hObject %x dwTimeOut %ld\n",
		hObject, dwTimeOut));
	return (WAIT_OBJECT_0);
}

DWORD	WINAPI
WaitForSingleObjectEx(HANDLE hObject, DWORD dwTimeOut, BOOL bAlertable)
{
	APISTR((LF_API, "WaitForSingleObject:"
		" (API) hObject %x dwTimeOut %ld bAlertable %s\n",
		hObject, dwTimeOut, bAlertable ? "TRUE" : "FALSE"));
	return (WAIT_OBJECT_0);
}

DWORD	WINAPI
WaitForMultipleObjects(DWORD dwCount, CONST HANDLE *lpHandles,
	BOOL bWaitAll, DWORD dwMilliseconds)
{
	APISTR((LF_API, "WaitForMultipleObjects:"
		" (API) dwCount %ld lpHandles %p\n",
		dwCount, (void *) lpHandles));
	return (WAIT_OBJECT_0);
}

DWORD	WINAPI
WaitForMultipleObjectsEx(DWORD dwCount, CONST HANDLE *lpHandles,
	BOOL bWaitAll, DWORD dwMilliseconds, BOOL bAlertable)
{
	APISTR((LF_API, "WaitForMultipleObjectsEx:"
		" (API) dwCount %ld lpHandles %p\n",
		dwCount, (void *) lpHandles));
	return (WAIT_OBJECT_0);
}

/* (WIN32) Mutex *********************************************************** */

HANDLE	WINAPI
CreateMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
	BOOL bInitialOwner,
	LPCSTR lpName)
{
	APISTR((LF_API, "CreateMutex: (API) %s\n", lpName));
	return ((HANDLE) 0);
}

HANDLE	WINAPI
OpenMutex(DWORD dwDesiredAccess,
	BOOL bInheritHandle,
	LPCSTR lpName)
{
	APISTR((LF_API, "OpenMutex: (API) %s\n", lpName));
	return ((HANDLE) 0);
}

BOOL	WINAPI
ReleaseMutex(HANDLE hMutex)
{
	APISTR((LF_API, "ReleaseMutex: (API) hMutex %x\n", hMutex));
	return (TRUE);
}

/* (WIN32) Semaphore ******************************************************* */

HANDLE	WINAPI
CreateSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
	LONG lInitialCount,
	LONG lMaximumCount,
	LPCSTR lpName)
{
	APISTR((LF_API, "CreateSemaphore: (API) %s\n", lpName));
	return ((HANDLE) 0);
}

HANDLE	WINAPI
OpenSemaphore(DWORD dwDesiredAccess,
	BOOL bInheritHandle,
	LPCSTR lpName)
{
	APISTR((LF_API, "OpenSemaphore: (API) %s\n", lpName));
	return ((HANDLE) 0);
}

BOOL	WINAPI
ReleaseSemaphore(HANDLE hSemaphore,
	LONG lpReleaseCount, LPLONG lpPreviousCount)
{
	APISTR((LF_API, "ReleaseSemaphore: (API) hSemaphore %x\n",
		hSemaphore));
	return (TRUE);
}

/* (WIN32) Event *********************************************************** */

HANDLE	WINAPI
CreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
	BOOL bManualReset,
	BOOL bInitialState,
	LPCSTR lpName)
{
	APISTR((LF_API, "CreateEvent: (API) %s\n", lpName));
	return ((HANDLE) 0);
}

BOOL	WINAPI
ResetEvent(HANDLE hEvent)
{
	APISTR((LF_API, "ResetEvent: (API) hEvent %x\n", hEvent));
	return (TRUE);
}

BOOL	WINAPI
SetEvent(HANDLE hEvent)
{
	APISTR((LF_API, "SetEvent: (API) hEvent %x\n", hEvent));
	return (TRUE);
}

BOOL	WINAPI
PulseEvent(HANDLE hEvent)
{
	APISTR((LF_API, "PulseEvent: (API) hEvent %x\n", hEvent));
	return (TRUE);
}

/* (WIN32) Message Synchronization ***************************************** */

DWORD	WINAPI
MsgWaitForMultipleObjects(DWORD dwCount,
	LPHANDLE lpHandles,
	BOOL bWaitAll,
	DWORD dwMilliseconds,
	DWORD dwWakeMask)
{
	APISTR((LF_API, "MsgWaitForMultipleObjects: (API)\n"));
	return (WAIT_OBJECT_0);
}

