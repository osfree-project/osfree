/*    
	dos_date.c	1.9
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

#include <string.h>

#include "xdos.h"
#include "xtime.h"
#include "dosdefn.h"
#include "xdosproc.h"

unsigned long long_to_BCD(unsigned long);
unsigned long BCD_to_long(unsigned long);
int time_of_day_service(int, unsigned long *, unsigned long *, unsigned long *);
void
time_date(unsigned short mode, unsigned long *ax,
		unsigned long *cx,unsigned long *dx)
{
	unsigned long tcx, tdx;
	long retcode;

	if (mode == SET_DATE) {
		tcx = (long_to_BCD(*cx / 100) << 8) + long_to_BCD(*cx % 100);
		tdx = (long_to_BCD(*dx >> 8) << 8) + long_to_BCD(*dx & 0x00ff);
	}
	retcode = time_of_day_service(mode, ax, &tcx, &tdx);
	switch (mode) {
	case GET_DATE:
		*cx = BCD_to_long((tcx >> 8) & 0xff) * 100 + 
		      BCD_to_long(tcx & 0x00ff);
		*dx = (BCD_to_long((tdx >> 8) & 0xff) << 8) + 
		      BCD_to_long(tdx & 0x00ff);
		break;
	case SET_DATE:
		break;
	case GET_TIME:
		*cx = (BCD_to_long((tcx >> 8) & 0xff) << 8)	/* h/min */
			+ BCD_to_long(tcx & 0x00ff);
		*dx = (BCD_to_long((tdx >> 8) & 0xff) << 8)	/* sec/hundrs*/
			+ BCD_to_long(tdx & 0x00ff);
		break;
	case SET_TIME:
		*cx = (long_to_BCD((tcx >> 8) & 0xff) << 8) 	/* h/min */
			+ long_to_BCD(tcx & 0xff);
		*dx = (long_to_BCD((tdx >> 8) & 0xff) << 8) 	/* sec/hundrs*/
			+ long_to_BCD(tdx & 0xff);
		break;
	}
	
	if (retcode)
		*ax |= 0xff00;		/* set ah on - error */
	else
		*ax &= ~0xff00;		/* no error, ah = 0 */
}

#define DATE_LONG 0x1
#define DATE_US   0x2

int
TWIN_GetCurrentDate(char * buffer, unsigned int mode)
{
  unsigned long ax;		/* day of week */
  unsigned long cx;		/* year */
  unsigned long dx;		/* month in hi byte, day in low byte */
  char month[3], date[3], year[3], year_temp[5];

  time_date(GET_DATE, &ax, &cx, &dx);

  if ( !(mode & DATE_LONG) && mode & DATE_US)
  {
    if ( buffer == NULL )
    {
     return 0;
    }

    sprintf(year_temp, "%ld", cx);
    /* convert year from XXXX to XX format */
    year[0] = year_temp[2];	
    year[1] = year_temp[3];

    sprintf(month, "%ld", ((dx >> 8) & 0xff));
    sprintf(date, "%ld", (dx & 0x00ff));

    strcpy(buffer, month);
    if (buffer[1] == '\0')
    { /* make "8" into "08" */
      buffer[1] = buffer[0];
      buffer[0] = '0';
    }
    buffer[2] = '/';
    strcpy(&buffer[3], date);
    if (buffer[4] == '\0')
    {
      buffer[4] = buffer[3];
      buffer[3] = '0';
    }
    buffer[5] = '/';
    strcpy(&buffer[6], year);
    if (buffer[7] == '\0' && buffer[6] != '\0')
    {
      buffer[7] = buffer[6];
      buffer[6] = '0';
    }
    if (buffer[6] == '\0' && buffer[7] == '\0')
    { /* year "00" */
      buffer[6] = buffer[7] = '0';
    }
    buffer[8] = '\0';
  }
  return 1;
}

