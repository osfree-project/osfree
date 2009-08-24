/********************************************************************

	@(#)DrvTime.c	1.5 Driver Functions to convert times 
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

********************************************************************/
 
#include "windows.h"
#include "Log.h"
#include <time.h>
#include <string.h>

#include "DrvCommon.h"
#include "Driver.h"

#include "platform.h"

DWORD DrvTimeTab(void);
static DWORD DrvTimeDoNothing(LPARAM, LPARAM, LPVOID);

/*
 * Windows defines three different types of times:
 *
 * -- FILETIME is a 64-bit word containing the number of 100-nanosecond
 *        intervals since 12:00 am 1 Jan 1601.
 *
 * -- SYSTEMTIME is a struct similar to the ANSI struct tm containing
 *        a description of a date and time, with resolution down to
 *        milliseconds.
 *
 * -- DOS date and time, a pair of packed short ints.
 *
 * We must convert between these, plus convert between file time and
 * a "local" file time, since file time always uses UCT.
 */

/* Global variables */

/*
 * ltimeOffset: the number of seconds between local time and system time.
 * System time is always in UCT; local time is based on the current time
 * zone.  ltimeOffset = timeLocal - timeSystem
 */
static time_t ltimeOffset;

/*
 * tzInfo: a TIME_ZONE_INFORMATION structure containing the current
 * time zone information, as passed in SetTimeZoneInformation.  We don't
 * do anything with any of this.  The Bias member should always be calculated
 * from ltimeOffset (timeSystem = timeLocal + bias ==>
 * bias = timeSystem - timeLocal ==> bias = -ltimeOffset / 60).
 */
static TIME_ZONE_INFORMATION tzInfo;


/*
 *  TIME_RATIO: The ratio between units in FILETIME type and units
 *  in time_t.  Currently 10^7.  [And won't change unless Microsoft
 *  completely changes the Win32 spec for FILETIME.]
 *  TIME_RATIO_DIV1: First portion of TIME_RATIO to use when dividing.
 *  TIME_RATIO_DIV2: Second portion of TIME_RATIO to use when dividing.
 *
 *  The following relations must hold for these routines to function
 *  properly:
 *
 *		TIME_RATIO = TIME_RATIO_DIV1 * TIME_RATIO_DIV2
 *		TIME_RATIO < 2^32
 *		TIME_RATIO_DIV1 < 2^16
 *		TIME_RATIO_DIV2 < 2^16
 *
 *  If these constraints cannot be met, the algorithm in 
 *  div_64bit_time() will have to be changed.
 */
#define TIME_RATIO       (10000000)
#define TIME_RATIO_DIV1  (10000)
#define TIME_RATIO_DIV2  (1000)

/*
 *  Using only 32-bit integer arithmetic, implement the division
 *  of a 64-bit integer by TIME_RATIO.  (Subject to all of the
 *  constraints listed with the definition of TIME_RATIO.)
 */
static void
div_64bit_time(FILETIME *in, FILETIME *out)
{
    unsigned long x[5];    /* Holds four 32-bit coefficents plus remainder */
    unsigned long y[5];
    unsigned long z[5];
    int i;

    /*
     *  Handle special case of upper 32 bits being 0.  The extra
     *  test is minimal if this isn't the case, but there are
     *  significant time savings if they -are- zero.
     */
    if (in->dwHighDateTime == 0)
    {
	out->dwHighDateTime = 0;
	out->dwLowDateTime = in->dwLowDateTime / TIME_RATIO;
        return;
    }

    /*
     *  First put input value into "normalized" form of
     *      x[0] * 2^48 + x[1] * 2^32 + x[2] * 2^16 + x[3]
     *
     *  where each x[i] is in the range 0 <= x[i] < 2^16
     */
    x[0] = (in->dwHighDateTime >> 16) & 0x0000ffff;
    x[1] = in->dwHighDateTime & 0x0000ffff;
    x[2] = (in->dwLowDateTime >> 16) & 0x0000ffff;
    x[3] = in->dwLowDateTime & 0x0000ffff;
    x[4] = 0;   /* Used for spillover of final remainder */

    /*
     *  Now we take advantage of the fact that 10^7 = 10^4 * 10^3,
     *  and that both factors are less than 2^16.  We do the overall
     *  division in two steps, first calculating Y = X / 10000, then
     *  calculating Z = Y / 1000.  By doing this, we guarantee that
     *  the remainder from any division is less than 2^16.  Since we
     *  also have each coefficient x[i] under 2^16, when we carry
     *  the remainder of x[i]/divisor into x[i+1], the sum
     *
     *      x[i+1] = x[i+1] + ((x[i] % divisor) << 16)
     *
     *  is guaranteed to be less than 2^32, so we don't have to worry
     *  about overflow.  We are also guaranteed that the quotient at
     *  each step is also under 2^16, so we don't need to re-normalize
     *  the results after either pass.
     */
    for (i=0; i<4; i++)
    {
	y[i] = x[i] / TIME_RATIO_DIV1;
	x[i+1] += (x[i] % TIME_RATIO_DIV1) << 16;
	/*
	 *  Note --- the remainder from the last pass is not really
	 *  used, but for simplicity, it is still calculated, and put
	 *  into the extra element of the array.  If we optimize this
	 *  by unrolling the short loop, the final step could be
	 *  removed, but the time savings is probably minimal.
	 */
    }
    for (i=0; i<4; i++)
    {
	z[i] = y[i] / TIME_RATIO_DIV2;
	y[i+1] += (y[i] % TIME_RATIO_DIV2) << 16;
    }

    /*
     *  Now translate the value back to the FILETIME representation.
     *  Again, we know that each coefficient is less than 2^16.
     */
    out->dwHighDateTime = (z[0] << 16) + z[1];
    out->dwLowDateTime = (z[2] << 16) + z[3];

    return;
}

/*
 * TWIN_DrvFiletime2UnixTime: Convert a FILETIME struct to a time_t
 *
 * Since a FILETIME is the number of 100-nanoseconds since 1 Jan 1601 and a
 * time_t is the number of (whole) seconds since 1 Jan 1970, this shouldn't
 * be _too_ hard.  TIMEOFFSETH and TIMEOFFSETL give the FILETIME equivalent 
 * for 0 in time_t.
 */

#define TIMEOFFSETH ((DWORD)0x019DB1DE)
#define TIMEOFFSETL ((DWORD)0xD53E8000)

time_t 
TWIN_DrvFiletime2UnixTime(LPFILETIME lpft)
{
    FILETIME ftTemp1;
    FILETIME ftTemp2;
    time_t retval;

    /*
     *  If the time is before zero-time for time_t, return 0.
     */
    if ((lpft->dwHighDateTime < TIMEOFFSETH) ||
        ((lpft->dwHighDateTime == TIMEOFFSETH) &&
         (lpft->dwLowDateTime <= TIMEOFFSETL)))
    {
	return (time_t)0;
    }

    /*
     *  Subtract the offset to make this time relative to 1 Jan 1970.
     */
    ftTemp1.dwHighDateTime = lpft->dwHighDateTime - TIMEOFFSETH;
    if (lpft->dwLowDateTime < TIMEOFFSETL)
        ftTemp1.dwHighDateTime--;
    ftTemp1.dwLowDateTime = lpft->dwLowDateTime - TIMEOFFSETL;

    /*
     *  Divide it by the ratio of 10^7.
     */
    div_64bit_time(&ftTemp1, &ftTemp2);

    /*
     *  If the size of time_t is less than a 64-bit value, then we
     *  must check to make sure the calculated value is in our range.
     *  If the high DWORD is zero, then just return the lower DWORD.
     *  However, if the high DWORD is non-zero, we are out of range,
     *  and should return our largest possible value.
     */
    if (sizeof(time_t) < 8)
    {
        if (ftTemp2.dwHighDateTime == 0)
	    return((time_t)ftTemp2.dwLowDateTime);
	else
	    return((time_t)-1);
    }

    /*
     *  At this point, we know time_t is at least 64 bits, so we
     *  want to return exactly what we got.  But in order to get
     *  the calculations to compile without warnings/errors on
     *  platforms where time_t is only 32 bits, we must do the
     *  shift of the high DWORD in two steps of 16 bits each.
     */
    retval = ((time_t)ftTemp2.dwHighDateTime) << 16;
    retval = retval << 16;
    retval += (time_t)ftTemp2.dwLowDateTime;
    return retval;
}

/*
 *  TWIN_DrvUnixTime2Filetime: convert a time_t to a FILETIME struct
 *
 *  We multiply by 10^7, and then add the offset representing zero-time
 *  in the Unix world.  The values involved are 64-bit quantities, but
 *  we have to do this with 32-bit math.
 */
void 
TWIN_DrvUnixTime2Filetime(time_t t, LPFILETIME lpft)
{
    unsigned long x1, x2;
    unsigned long y1, y2;
    unsigned long m1, m2, m3, m4;
    unsigned long z1, z2, z3, z4;

    /*
     *  First do the multiplication.  We perform Z = X * Y, where X is
     *  the passed in time value, and Y is 10^7.  To handle the potential
     *  overflow, we split X and Y up into x1,x2,y1,y2, all less than 2^16,
     *  as follows:
     *
     *      X = x1 * 2^16 + x2;   Y = y1 * 2^16 + y2;
     *
     *  Then we can express the product as this:
     *
     *      Z = X * Y = x1*y1*2^32 + x1*y2*2^16 + x2*y1*2^16 + x2*y2
     *
     *  or
     *
     *	    Z = m1 * 2^32 + m2 * 2^16 + m3 * 2^16 + m4
     */
    x1 = (((DWORD)t) >> 16) & 0x0ffff;
    x2 = ((DWORD)t) & 0x0ffff;
    y1 = (TIME_RATIO >> 16) & 0x0ffff;
    y2 = TIME_RATIO & 0x0ffff;
  
    m1 = x1 * y1;    /* Part of Z[63..32]  */
    m2 = x1 * y2;    /* Part of Z[47..16]  */
    m3 = x2 * y1;    /* Part of Z[47..16]  */
    m4 = x2 * y2;    /* Part of Z[31..0]   */

    /*
     *  Now that we have the components of Z, start adding together various
     *  parts.  We know that m1 cannot overflow, so add in the Z[47..32]
     *  portions of m2 and m3, zeroing out the upper half of both when
     *  we are done.
     */
    m1 = m1 + ((m2 >> 16) & 0x0ffff) + ((m3 >> 16) & 0x0ffff);
    m2 = m2 & 0x0ffff;
    m3 = m3 & 0x0ffff;

    /*
     *  Now we take the three parts that make up Z[31..16], and add
     *  them together in m2.  This cannot overflow either, since all
     *  three components are less than 2^16.  We are done with m3
     *  at this point, and the high word of m4 can be set to zero.
     */
    m2 = m2 + m3 + ((m4 >> 16) & 0x0ffff);
    m4 = m4 & 0x0ffff;

    /*
     *  Now split m2 into its two halves, and add them back into
     *  m1 (the high word) and m2 (the low word), shifted appropriately.
     *  Neither m1 nor m4 can overflow at this point.  We then have
     *  Z represented as m1 * 2^32 + m4.
     */
    m1 = m1 + ((m2 >> 16) & 0x0ffff);
    m4 = m4 + ((m2 & 0x0ffff) << 16);

    /*
     *  That takes care of the multiplication part.  Now do the addition
     *  of the offset.  This requires us to split Z into this form first:
     *
     *      Z = z1*2^48 + z2*2^32 + z3*2^16 + z4
     *
     *  And a similar representation of the offset in m1..m4, and then
     *  adding the various components together, with carries.
     */
    z1 = ((m1 >> 16) & 0x0ffff);
    z2 = m1 & 0x0ffff;
    z3 = ((m4 >> 16) & 0x0ffff);
    z4 = m4 & 0x0ffff;
    m1 = ((TIMEOFFSETH >> 16) & 0x0ffff);
    m2 = TIMEOFFSETH & 0x0ffff;
    m3 = ((TIMEOFFSETL >> 16) & 0x0ffff);
    m4 = TIMEOFFSETL & 0x0ffff;

    z4 = z4 + m4;
    z3 = z3 + m3 + ((z4 >> 16) & 0x0ffff);
    z2 = z2 + m2 + ((z3 >> 16) & 0x0ffff);
    z1 = z1 + m1 + ((z2 >> 16) & 0x0ffff);
    z4 = z4 & 0x0ffff;   /* Clear the previously carried portions */
    z3 = z3 & 0x0ffff;
    z2 = z2 & 0x0ffff;
    z1 = z1 & 0x0ffff;

    /*
     *  We are done, so build the final answer from these components.
     */
    lpft->dwHighDateTime = (z1 << 16) + z2;
    lpft->dwLowDateTime = (z3 << 16) + z4;
    return;
}

/* tm_to_systemtime: change an ANSI struct tm into a Windows SYSTEMTIME */
static void tm_to_systemtime(struct tm *ptm, LPSYSTEMTIME lpst)
{
  lpst->wYear = ptm->tm_year + 1900;
  lpst->wMonth = ptm->tm_mon + 1;
  lpst->wDayOfWeek = ptm->tm_wday;
  lpst->wDay = ptm->tm_mday;
  lpst->wHour = ptm->tm_hour;
  lpst->wMinute = ptm->tm_min;
  lpst->wSecond = ptm->tm_sec;
  lpst->wMilliseconds = 0;
}

/* systemtime_to_tm: change a Windows SYSTEMTIME into an ANSI struct tm */
static void systemtime_to_tm(LPSYSTEMTIME lpst, struct tm *ptm)
{
  ptm->tm_year = lpst->wYear - 1900;
  ptm->tm_mon = lpst->wMonth - 1;
  ptm->tm_wday = lpst->wDayOfWeek;
  ptm->tm_mday = lpst->wDay;
  ptm->tm_hour = lpst->wHour;
  ptm->tm_min = lpst->wMinute;
  ptm->tm_sec = lpst->wSecond;
  ptm->tm_isdst = -1;
}

/* FileTimeToSystemTime: Convert a FILETIME to a SYSTEMTIME */
static DWORD DrvFile2System(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
  LPFILETIME lpft = (LPFILETIME)dwParam1;
  LPSYSTEMTIME lpst = (LPSYSTEMTIME)dwParam2;
  time_t t;
  struct tm *ptm;

  t = TWIN_DrvFiletime2UnixTime(lpft);
  ptm = gmtime(&t);
  tm_to_systemtime(ptm, lpst);
  return (DWORD)TRUE;
}

/* SystemTimetoFileTime: Convert a SYSTEMTIME to a FILETIME */
static DWORD 
DrvSystem2File(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
  LPSYSTEMTIME lpst = (LPSYSTEMTIME)dwParam1;
  LPFILETIME lpft = (LPFILETIME)dwParam2;
  time_t t;
  struct tm stm;
  
  systemtime_to_tm(lpst, &stm);
  if ((t = mktime(&stm)) == -1)
    return (DWORD)FALSE;
  TWIN_DrvUnixTime2Filetime(t, lpft);
  return (DWORD)TRUE;
}
  
static DWORD
DrvFile2Local(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
  LPFILETIME lpft = (LPFILETIME)dwParam1;
  LPFILETIME lpftLocal = (LPFILETIME)dwParam2;
  time_t t;
  t = TWIN_DrvFiletime2UnixTime(lpft);
  t += ltimeOffset;
  TWIN_DrvUnixTime2Filetime(t, lpftLocal);
  return (DWORD)TRUE;
}

static DWORD 
DrvLocal2File(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
  LPFILETIME lpftLocal = (LPFILETIME)dwParam1;
  LPFILETIME lpft = (LPFILETIME)dwParam2;
  
  time_t t;
  t = TWIN_DrvFiletime2UnixTime(lpftLocal);
  t -= ltimeOffset;
  TWIN_DrvUnixTime2Filetime(t, lpft);
  return (DWORD)TRUE;
}

static DWORD
DrvGetSysTime(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
  LPSYSTEMTIME lpst = (LPSYSTEMTIME)dwParam1;
  time_t t;
  struct tm *ptm;

  t = time(NULL);
  ptm = gmtime(&t);
  tm_to_systemtime(ptm, lpst);

  return (DWORD)TRUE;
}

static DWORD
DrvGetLocTime(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
  LPSYSTEMTIME lpst = (LPSYSTEMTIME)dwParam1;
  time_t t;
  struct tm *ptm;

  t = time(NULL);
  t += ltimeOffset;
  ptm = gmtime(&t);
  tm_to_systemtime(ptm, lpst);

  return (DWORD)TRUE;
}

static DWORD
DrvSetSysTime(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
  LPSYSTEMTIME lpst = (LPSYSTEMTIME)dwParam1;
  time_t t, now, local;
  struct tm stm;

#ifdef TWIN_HASSETTIME
  struct timeval tv;
#endif
  systemtime_to_tm(lpst, &stm);
  t = mktime(&stm);
  if (t == -1)
    return (DWORD)FALSE;
  
  now = time(NULL);
  local = now + ltimeOffset;

#ifdef TWIN_HASSETTIME
  tv.tv_usec = 0;
  tv.tv_sec = t;
  if (settimeofday(&tv, NULL) < 0)
    return (DWORD)FALSE;
#else 
  if (stime(&t) == -1)
    return (DWORD)FALSE;
#endif
  ltimeOffset = local - t;
  return (DWORD)TRUE;
}

static DWORD
DrvSetLocTime(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
  LPSYSTEMTIME lpst = (LPSYSTEMTIME)dwParam1;
  time_t t, now;
  struct tm stm;

  systemtime_to_tm(lpst, &stm);
  now = time(NULL);
  t = mktime(&stm);
  if (t == -1)
    return (DWORD)FALSE;
  ltimeOffset = t - now;
  return (DWORD)TRUE;
}

/* Time Zone information functions
 * TODO: Anything involving this data -- maybe? */

static DWORD
DrvSetTZI(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
  LPTIME_ZONE_INFORMATION lptzi = (LPTIME_ZONE_INFORMATION)dwParam1;
  tzInfo = *lptzi;
  ltimeOffset = -tzInfo.Bias * 60;
  return (DWORD)TRUE;
}

static DWORD
DrvGetTZI(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
  LPTIME_ZONE_INFORMATION lptzi = (LPTIME_ZONE_INFORMATION)dwParam1;
  *lptzi = tzInfo;
  lptzi->Bias = -ltimeOffset / 60;
  return TRUE;
}

/* DOS date and time
 * In 1980, this probably made sense...
 * The date and time for an MS-DOS file are packed into two 16-bit words.
 * The job of these functions is to pack and unpack those values. */

static void tm_to_dos_time(struct tm *ptm, WORD *pwDOSDate, WORD *pwDOSTime)
{
  *pwDOSDate = ((((ptm->tm_year - 80) & 0x7F) << 9) +
		(((ptm->tm_mon + 1) & 0xF) << 5) +
		(((ptm->tm_mday) & 0x1F)));
  *pwDOSTime = ((((ptm->tm_hour) & 0x1F) << 11) +
		(((ptm->tm_min) & 0x3F) << 5) +
		(((ptm->tm_sec / 2) & 0x1F)));
}

static void 
dos_time_to_tm(WORD wDOSDate, WORD wDOSTime, struct tm *ptm)
{
    ptm->tm_mday = (wDOSDate & 0x1F);
    ptm->tm_mon = ((wDOSDate >> 5) & 0xF) - 1;
    if (ptm->tm_mon < 0)
	ptm->tm_mon = 0;
    ptm->tm_year = ((wDOSDate >> 9) & 0x7F) + 80;
    ptm->tm_sec = (wDOSTime & 0x1F) * 2;
    ptm->tm_min = ((wDOSTime >> 5) & 0x3F);
    ptm->tm_hour = ((wDOSTime >> 11) & 0x1F);
    ptm->tm_isdst = 0;
    ptm->tm_wday = 0;
    ptm->tm_yday = 0;
}

static DWORD
DrvDos2File(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
  WORD wDOSDate = (WORD)dwParam1;
  WORD wDOSTime = (WORD)dwParam2;
  LPFILETIME lpft = (LPFILETIME)lpStruct;
  struct tm stm;
  time_t t;

  dos_time_to_tm(wDOSDate, wDOSTime, &stm);
  t = mktime(&stm);
  if (t == -1)
    return (DWORD)FALSE;
  TWIN_DrvUnixTime2Filetime(t, lpft);
  return (DWORD)TRUE;
}

static DWORD
DrvFile2Dos(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
  LPFILETIME lpft = (LPFILETIME)dwParam1;
  LPWORD lpwDOSDate = (LPWORD)dwParam2;
  LPWORD lpwDOSTime = (LPWORD)lpStruct;
  struct tm *ptm;
  time_t t;

  t = TWIN_DrvFiletime2UnixTime(lpft);
  ptm = gmtime(&t);
  tm_to_dos_time(ptm, lpwDOSDate, lpwDOSTime);
  return (DWORD)TRUE;
}

static DWORD
DrvSys2TZILoc(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
  LPTIME_ZONE_INFORMATION lpTimeZoneInformation = 
		(LPTIME_ZONE_INFORMATION)dwParam1;
  LPSYSTEMTIME lpUCT = (LPSYSTEMTIME)dwParam2;
  LPSYSTEMTIME lpst = (LPSYSTEMTIME)lpStruct;
  FILETIME ft;
  time_t t;
  SystemTimeToFileTime(lpUCT, &ft);
  t = TWIN_DrvFiletime2UnixTime(&ft);
  t -= lpTimeZoneInformation->Bias * 60;
  TWIN_DrvUnixTime2Filetime(t, &ft);
  FileTimeToSystemTime(&ft, lpst);
  return (DWORD)TRUE;
}

static TWINDRVSUBPROC DrvTimeEntryTab[] = {
	DrvTimeDoNothing,
	DrvTimeDoNothing,
	DrvTimeDoNothing,
	DrvFile2System,
	DrvSystem2File,
	DrvFile2Local,
	DrvLocal2File,
	DrvGetSysTime,
	DrvSetSysTime,
	DrvGetLocTime,
	DrvSetLocTime,
	DrvGetTZI,
	DrvSetTZI,
	DrvDos2File,
	DrvFile2Dos,
	DrvSys2TZILoc
};
	
	

DWORD
DrvTimeTab(void)
{
#if defined(TWIN_RUNTIME_DRVTAB)
        DrvTimeEntryTab[0] = DrvTimeDoNothing;
        DrvTimeEntryTab[1] = DrvTimeDoNothing;
        DrvTimeEntryTab[2] = DrvTimeDoNothing;
        DrvTimeEntryTab[3] = DrvFile2System;
        DrvTimeEntryTab[4] = DrvSystem2File;
        DrvTimeEntryTab[5] = DrvFile2Local;
        DrvTimeEntryTab[6] = DrvLocal2File;
        DrvTimeEntryTab[7] = DrvGetSysTime;
        DrvTimeEntryTab[8] = DrvSetSysTime;
        DrvTimeEntryTab[9] = DrvGetLocTime;
        DrvTimeEntryTab[10] = DrvSetLocTime;
        DrvTimeEntryTab[11] = DrvGetTZI;
        DrvTimeEntryTab[12] = DrvSetTZI;
        DrvTimeEntryTab[13] = DrvDos2File;
        DrvTimeEntryTab[14] = DrvFile2Dos;
        DrvTimeEntryTab[15] = DrvSys2TZILoc;
#endif
	return (DWORD)DrvTimeEntryTab;
}

static DWORD
DrvTimeDoNothing(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return 0L;
}

