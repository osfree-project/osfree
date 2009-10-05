/*    
	dos_timer.c	1.7
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

#include "platform.h"

#define		_POSIX_SOURCE	1
#include	<time.h>


#define	dysize(y)	\
	(((y) % 4) ? 365 : (((y) % 100) ? 366 : (((y) % 400) ? 365 : 366)))

static int  dmsize[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


/* Perform converse of gmtime */
static long
time_gm(
	register int sec,		/* 0 - 59 */
	register int min,		/* 0 - 59 */
	register int hour,		/* 0 - 23 */
	register int mday,		/* 1 - 31 */
	register int mon,		/* 0 - 11 */
	register int year		/* year - 1900 */
       )
{
	register long   result;
	int		i;

	if (sec < 0 || sec > 59
	    || min < 0 || min > 59
	    || hour < 0 || hour > 23
	    || mday < 1 || mday > 31
	    || mon < 0 || mon > 11)
		return -1L;

	result = 0L;
	year += 1900;
	for (i = 1970; i < year; i++)
		result += dysize(i);
	if (dysize(year) == 366 && mon >= 3)
		result++;
	while (mon--)
		result += dmsize[mon];
	result += mday - 1;
	result = 24 * result + hour;
	result = 60 * result + min;
	result = 60 * result + sec;

	return result;
}


/* Performs converse of localtime(3) */
static long
time_local(
	   register int sec,		/* 0 - 59 */
	   register int min,		/* 0 - 59 */
	   register int hour,		/* 0 - 23 */
	   register int mday,		/* 1 - 31 */
	   register int mon,		/* 0 - 11 */
	   register int year		/* year - 1900 */
          )
{
	long		result = 0;
	struct tm	*tm, *localtime();
	long		zonefix;

	tm = localtime(&result);	/* Epoch (1/1/70) adjusted for GMT */

	result = time_gm(sec, min, hour, mday, mon, year);
	zonefix = (((long)(tm->tm_hour * 60) + tm->tm_min) * 60) + tm->tm_sec;
	if (tm->tm_year < 70) {		/* In Western hemisphere? */
		result += (24 * 60 * 60) - zonefix;
	}
	else {
		result -= zonefix;
	}
	tm = localtime(&result);	/* Break down current time ... */
	if (tm->tm_isdst)		/* To check if it is during DST */
		result -= 60 * 60;
	return result;
}


unsigned long
seconds( unsigned int access_date, unsigned int access_time )
{
	unsigned int	year;
	unsigned int	month;
	unsigned int	day;
	unsigned int	hour;
	unsigned int	minute;
	unsigned int	second;

	/* unformat dos date and time */

	year   = ((access_date >> 9) & 0x7f) + 1980;
	month  = ((access_date >> 5) & 0x0f) - 1;
	day    =   access_date & 0x1f;
	hour   =  (access_time >> 11) & 0x1f;
	minute =  (access_time >> 5) & 0x3f;
	second =   access_time & 0x1f;

	/* compute number of seconds since jan 1, 1970 */

	return ( time_local(second, minute, hour, day, month, year - 1900) );
}

