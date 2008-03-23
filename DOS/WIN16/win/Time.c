/*    
	Time.c	1.6
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
#include "Driver.h"

/* FileTimeToSystemTime: Convert a FILETIME to a SYSTEMTIME */
BOOL WINAPI
FileTimeToSystemTime(LPFILETIME lpft, LPSYSTEMTIME lpst)
{
    return (BOOL)DRVCALL_TIME(PTMH_FT2ST,(LPARAM)lpft,(LPARAM)lpst,
				(LPVOID)NULL);
}

/* SystemTimetoFileTime: Convert a SYSTEMTIME to a FILETIME */
BOOL WINAPI
SystemTimeToFileTime(LPSYSTEMTIME lpst, LPFILETIME lpft)
{
    return (BOOL)DRVCALL_TIME(PTMH_ST2FT,(LPARAM)lpst,(LPARAM)lpft,
				(LPVOID)NULL);
}
  
BOOL WINAPI
FileTimeToLocalFileTime(LPFILETIME lpft, LPFILETIME lpftLocal)
{
    return (BOOL)DRVCALL_TIME(PTMH_FT2LFT,(LPARAM)lpft,(LPARAM)lpftLocal,
				(LPVOID)NULL);
}

BOOL WINAPI
LocalFileTimeToFileTime(LPFILETIME lpftLocal, LPFILETIME lpft)
{
    return (BOOL)DRVCALL_TIME(PTMH_LFT2FT,(LPARAM)lpftLocal,(LPARAM)lpft,
				(LPVOID)NULL);
}

void WINAPI
GetSystemTime(LPSYSTEMTIME lpst)
{
    DRVCALL_TIME(PTMH_GETSYS,(LPARAM)lpst, (LPARAM)0, (LPVOID)NULL);
}

void WINAPI
GetLocalTime(LPSYSTEMTIME lpst)
{
    DRVCALL_TIME(PTMH_GETLOC,(LPARAM)lpst, (LPARAM)0, (LPVOID)NULL);
}

BOOL WINAPI
SetSystemTime(LPSYSTEMTIME lpst)
{
    return (BOOL)DRVCALL_TIME(PTMH_SETSYS,(LPARAM)lpst,(LPARAM)0,(LPVOID)NULL);
}

BOOL WINAPI
SetLocalTime(LPSYSTEMTIME lpst)
{
    return (BOOL)DRVCALL_TIME(PTMH_SETLOC,(LPARAM)lpst,(LPARAM)0,(LPVOID)NULL);
}

BOOL WINAPI
SetTimeZoneInformation(LPTIME_ZONE_INFORMATION lptzi)
{
    return (BOOL)DRVCALL_TIME(PTMH_SETTZI,(LPARAM)lptzi,(LPARAM)0,(LPVOID)NULL);
}

BOOL WINAPI
GetTimeZoneInformation(LPTIME_ZONE_INFORMATION lptzi)
{
    return (BOOL)DRVCALL_TIME(PTMH_GETTZI,(LPARAM)lptzi,(LPARAM)0,(LPVOID)NULL);
}

BOOL WINAPI
DosDateTimeToFileTime(WORD wDOSDate, WORD wDOSTime, LPFILETIME lpft)
{
    return (BOOL)DRVCALL_TIME(PTMH_DOS2FT,(LPARAM)wDOSDate,(LPARAM)wDOSTime,
				(LPVOID)lpft);
}

BOOL WINAPI
FileTimeToDosDateTime(LPFILETIME lpft, LPWORD lpwDOSDate, LPWORD lpwDOSTime)
{
    return (BOOL)DRVCALL_TIME(PTMH_FT2DOS,(LPARAM)lpft,(LPARAM)lpwDOSTime,
				(LPVOID)lpwDOSTime);
}

LONG WINAPI
CompareFileTime(LPFILETIME lpft1, LPFILETIME lpft2)
{
  if (lpft1->dwHighDateTime > lpft2->dwHighDateTime)
    return 1;
  if (lpft1->dwHighDateTime < lpft2->dwHighDateTime)
    return -1;
  if (lpft1->dwLowDateTime > lpft2->dwLowDateTime)
    return 1;
  if (lpft1->dwLowDateTime < lpft2->dwLowDateTime)
    return -1;
  return 0;
}

/* We don't support system time adjustment, but the user can always turn it off... */
BOOL WINAPI
SetSystemTimeAdjustment(DWORD dwTimeAdjustment, BOOL bTimeAdjustmentDisabled)
{
  if (bTimeAdjustmentDisabled)
    return TRUE;
  return FALSE;
}

BOOL WINAPI
GetSystemTimeAdjustment(DWORD *lpTimeAdjustment, DWORD *lpTimeIncrement,
			LPBOOL lpTimeAdjustmentDisabled)
{
  *lpTimeAdjustmentDisabled = TRUE;
  return TRUE;
}

void WINAPI
GetSystemTimeAsFileTime(LPFILETIME lpft)
{
  SYSTEMTIME st;

  GetSystemTime(&st);
  SystemTimeToFileTime(&st, lpft);
}

BOOL WINAPI
SystemTimeToTzSpecificLocalTime(LPTIME_ZONE_INFORMATION lpTimeZoneInformation,
				     LPSYSTEMTIME lpUCT, LPSYSTEMTIME lpst)
{
    return (BOOL)DRVCALL_TIME(PTMH_SYS2TZILOC,(LPARAM)lpTimeZoneInformation,
				(LPARAM)lpUCT, (LPVOID)lpst);
}
