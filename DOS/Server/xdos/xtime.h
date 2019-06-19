/*************************************************************************
*
*       @(#)xtime.h	1.3 1/28/96 22:40:10 /users/sccs/src/win/xdos/s.xtime.h
*	dos timer function definitions
*       Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.

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

**************************************************************************/

#ifndef xtime__h
#define xtime__h

/*	INT_1A's functions	*/

#define	GET_TICKS	0
#define	GET_TIME	2
#define SET_TIME	3
#define	GET_DATE	4
#define SET_DATE	5

#define format_date(calendar) ((calendar->tm_year-80)<<9)+((calendar->tm_mon+1)<<5)+calendar->tm_mday;
#define format_time(calendar) (calendar->tm_hour<<11)+(calendar->tm_min<<5)+calendar->tm_sec/2;


#define  MICRO 1000 
#endif /* xtime__h */
