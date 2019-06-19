/*    
	int_1a.c	1.12
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

/*******************************************
**
**	Bios Interrupt 0x1a (26) time if day service
**
**	Dos 21h timer services are for the most part exactly the same
**	as those of the bios functions. Dos 21 functions should call the
**	bios 1a functions, if possible.
**
********************************************/

#include "platform.h"

#include <stdlib.h>

#include <time.h>


#include "xdos.h"
#include "BinTypes.h"
#include "Log.h"
#include "xtime.h"
#include "dosdefn.h"
#include "xdosproc.h"

#define wsim_word(a,b,c)
#define wsim_byte(a,b,c)
int time_of_day_service(int, unsigned long *, unsigned long *, unsigned long *);

void
int_1a(int nNumber, register ENV *envp)
{
	int func;
	int result;

	func = HIBYTE(LOWORD(envp->reg.ax));

	result = time_of_day_service(func, &(envp->reg.ax),
			&(envp->reg.cx), &(envp->reg.dx));
	envp->reg.flags = (result >> 16) & 0xffff;
}

/************************************************************************
**	long_to_BCD
**
**	Converts the given integer to a BCD (binary coded decimal) value,
**	and returns it. Net result is that it turns decimal 1989 into
**	hex 0x1989. No checks for errors.
*/
unsigned long
long_to_BCD( unsigned long invalue )
{
	register unsigned long	value = invalue, answer = 0;
	register int		bits = 0;
	
	while (value != 0) {
		answer += (value % 10) << bits;
		value /= 10;
		bits += 4;
	}
	return answer;	
}
/************************************************************************
**	BCD_to_long
**
**	Converts the given BCD value to an integer and returns it. Turns
**	hex 0x1989 into decimal 1989.
*/
unsigned long
BCD_to_long( unsigned long invalue )
{
	register unsigned long	value = invalue, answer = 0;
	register int		decplace = 1;
	
	while ( value ) {
		answer += (value & 0x000f) * decplace;
		value = value >> 4;
		decplace *= 10;
	}
	return answer;
}


/************************************************************************
**	time_of_day_service
**
**	This function is set up exactly as the bios 1a functions are. 
**	Regs->AH holds the function number to be run.
**
**	RETURNS:	The dos 21 funcitons that call this function need
**			to know if it worked or not, (set time and date) 
**			This function returns 0 if they worked, or -1 if
**			they did not.  (with set functions disabled, allways
**			will return 0)
**
**	All set timer/date functions are either ignored or initiate a error
**	message on the screen depending on the RELEASE #define value.
**	The alarm functions are treated the same way
**
**	Get Current clock count - 18.2 per second, 0 every midnight
**		Input:	AH	00
**		Output:	CX	high order part of clock count
**			DX	low order part of clock count
**			AL	0 if timer has not passed 24 hour period
**				Norton Programers guide, pg 223  
**				says that midnight flag is cleared on read
**				Initially timer value picked up in i1ah_open()
**				and held in 
**	Set Current clock count		<< NOT IMPLEMENTED >>
**		Input:	AH	01
**			CX	high order part of clock count
**			DX	low order part of clock count
**		Output:	None
**	Read current clock count	Bios 1ah:2, Dos 21h:2ch
**		Input:	AH	2
**		Output:	CH	Hours
**			CL	Minutes
**			DH	Seconds
**			DL	Hundredths	Dos 21h:2ch needs
**						Bios 1ah:02h ignores?
**	Set Real time clock		<< NOT IMPLEMENTED >>
**		Input:	AH	3
**			CH	Hours
**			CL	Minutes
**			DH	Seconds
**			DL	1 if daylight saving time, 0 standard
**		Output: none
**	Read Date from real time clock
**		Input:	AH	4
**		Output:	DL	Day
**			DH	Month   (1 = jan ?? .. on unix, 0 = jan)
**			CL	Year
**			CH	Century (19 or 20)
**			AL	Day of Week	(for dos 21h:2ah)
**	Set Date in real time clock	<< NOT IMPLEMENTED >>
**		Input:	AH	5
**			DL	Day
**			DH	Month
**			CL	Year
**			CH	Century (19 or 20)
**		Output: none
**	Set Alarm			<< NOT IMPLEMENTED >>
**		Input:	AH	6		Place address for alarm
**			CH	Hours		in interrupt 4A location
**			CL	Minutes
**			DH	Seconds
**	Reset Alarm			<< NOT IMPLEMENTED >>
**		Input:	AH	7
**
*/
int
time_of_day_service( int fun, 
		     unsigned long *ax, unsigned long *cx, unsigned long *dx)
{
	register long	secs_since_midnight;
	register long	tics_since_midnight;
	static int	midnight_flag = 0;	/* set 1st cross 00:00:00*/
	static long boot_secs_since_mid;
	int		retflags = 0;
	struct tm	*tm=0, *localtime();
	struct timeval	tp;
#ifndef NETWARE
	struct timezone tzp;
#else
	int tzp[2];
#endif

	if (fun == GET_TICKS || fun == GET_TIME || fun == GET_DATE)
	{	
	gettimeofday(&tp, &tzp);	/* secs since 1/1/70 GMT*/
	tm = localtime((time_t *)(&tp.tv_sec));	/* fmt to year/mon/...	*/
	}
	switch(fun)
	{
		case GET_TICKS:
			secs_since_midnight = tm->tm_sec +
					60 * (tm->tm_min +
					(60 * tm->tm_hour));
			/* PC increments tic 0x1800B0 times in a 24 hour  */
			/* period ratio = 19663/1080 max temp value is	  */
			/* 0x651b5980 					  */

			tics_since_midnight = (secs_since_midnight * 19633)
					    / 1080;
			/* Add in micro sec's converted */
			/* to 1/18.2 sec tics here   	*/
			tics_since_midnight += (tp.tv_usec * 182) / 10000000L;
			*dx = tics_since_midnight & 0xffffL;
			wsim_word(BIOS_DATA, TIMER_LOW, *dx);
			*cx = (tics_since_midnight >> 16) & 0xffffL;
			wsim_word(BIOS_DATA, TIMER_HIGH, *cx);

			/* Midnight flag (al) only set the first time 	*/
			/* midnight is crossed according to norton	*/
			/* programmers guide, pg 223			*/

			/* setonly once, and never again... */
			if(boot_secs_since_mid == 0) {
				boot_secs_since_mid = tm->tm_sec
				     + 60 * (tm->tm_min 
				     + (60 * tm->tm_hour));
				boot_secs_since_mid++;
			}
			if (secs_since_midnight < boot_secs_since_mid
			    && !midnight_flag)
			{
				midnight_flag = 1;
				/* *ax = (*ax & 0xff00) | 1; */
				*ax = (*ax & 0xff00);
			}
			else
				*ax &= 0xff00L;	/* al - 0 midnight flag */ 
			wsim_byte(BIOS_DATA, TIMER_OFL, 
				(unsigned char)(*ax & 0xff));
			break;	
		case 1:			/* set current tic		*/
			retflags = CARRY_FLAG;
			break;
		case GET_TIME:
			*cx = (long_to_BCD((long)tm->tm_hour) << 8) + 
				long_to_BCD((long)tm->tm_min);
			/* Note that dos_21:2C wants a 1/100 sec value	*/ 
			/* in DL. Bios_1A:2 does not specify what is in	*/ 
			/* DL. For Generic reasons we stuff hundredth's	*/
			/* value into DL in both cases			*/
			*dx = (long_to_BCD((long)tm->tm_sec) << 8) + 
				long_to_BCD((long)tp.tv_usec/10000);
			break;	
		case 3:			/* write real time clock	*/
			retflags = CARRY_FLAG;
			break;
		case GET_DATE:
			*cx = (long_to_BCD((long)(tm->tm_year / 100) + 19) << 8) 
				   + long_to_BCD((long)tm->tm_year % 100);

			/* UNIX: jan = month 0 - DOS: jan = month 1 */

			*dx = (long_to_BCD((long)(tm->tm_mon+1)) << 8) + 
				long_to_BCD((long)tm->tm_mday);
			/* Day of week, dos 21h:2ah..	*/
			/* Not required for bios 1ah:04h*/
			*ax = (*ax & 0xff00) | tm->tm_wday;
			break;
		case 5:			/* write real time date		*/
			retflags = CARRY_FLAG;
			break;
		case 6:			/* set alarm			*/
			retflags = CARRY_FLAG;
			break;
		case 7:			/* reset alarm			*/
			retflags = CARRY_FLAG;
			break;
	}
	return retflags;		/* 0 ok, -1 problems..		*/
}

