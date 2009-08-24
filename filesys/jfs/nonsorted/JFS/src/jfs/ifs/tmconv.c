/* $Id: tmconv.c,v 1.1.1.1 2003/05/21 13:38:48 pasha Exp $ */

static char *SCCSID = "@(#)1.5  4/12/99 11:01:12 src/jfs/ifs/tmconv.c, sysjfs, w45.fs32, 990417.1";
/*
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or 
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software 
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
  */
/***	convtime.c - map internal to external times and vice-versa
 */



#define INCL_NOPMAPI
#define INCL_DOSERRORS
#include <os2.h>
#include <jfs_os2.h>

/*              ___________________________
 *              | hours | minutes | seconds|
 *              | 5 bit |  6 bits | 5 bits+|
 *              |__________________________|
 *              ___________________________
 *              | years*|  months |  days  |
 *              | 7 bit |  4 bits | 5 bits |
 *              |__________________________|
 *      (*) years from 1980
 *      (+) 2 seconds units
 */
typedef struct DOSTIME {
    unsigned int seconds2:5;
    unsigned int minutes:6;
    unsigned int hours:5;
} DOSTIME;

typedef struct DOSDATE {
    unsigned int days:5;
    unsigned int months:4;
    unsigned int years:7;
} DOSDATE;

typedef DOSTIME *PDOSTIME;
typedef DOSDATE *PDOSDATE;

typedef uint32 TIME;


#define _70_to_80_bias	0x012CEA600L
#define SECS_IN_HALFDAY 0x0A8C0L
#define FOURYEARS	(3*365+366)

uint16 YrTab[] = { 366, 365, 365, 365 };

uint16 MonTab[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
uint16 MonTabLeap[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

void Net_To_Dos_Date (TIME time, PDOSDATE pdd, PDOSTIME pdt)
{
    time -= _70_to_80_bias;

    {
	uint32 secs = time % (SECS_IN_HALFDAY * 2);
	uint16 usTmp;
	uint16 usTmp2;

	/* pdt->hours = (uint16) (secs / (60 * 60)); */
#ifdef DEBUG
	if ((secs / 2) > 0x0000ffff) {
		dprintf ("Net_To_Dos_Date: secs too big\n");
	}
#endif
	pdt->hours = usTmp = ((uint16) (secs / 2)) / 1800;

	/* usTmp = (uint16) (secs % (60 * 60)); */
	usTmp = (uint16) (secs - (uint32) usTmp * (uint32) (60 * 60));

	pdt->minutes = usTmp2 = usTmp / 60;

	/* pdt->seconds2 = (usTmp % 60) / 2 */
	pdt->seconds2 = (usTmp - (usTmp2 * 60)) / 2;
    }

    {
	uint32 days = time / (SECS_IN_HALFDAY * 2);
	uint16 i, j;
	uint16 usDays;

#ifdef DEBUG
	if ((days / FOURYEARS) * 4 > 0x0000ffff) {
		dprintf ("Net_To_Dos_Date: days too big\n");
	}
#endif
	pdd->years = (i = (uint16) (days / FOURYEARS)) * 4;

	/* usDays = (uint16) (days % FOURYEARS); */
	usDays = (uint16) (days - (uint32) i * (uint32) FOURYEARS);

	/*
	 * Since usDays must be between 0 and FOURYEARS (sum of days in 4
	 * years), then we shouldn't need to do the mod in the loop below.
	 */
	i = 0;
	while (usDays >= YrTab[i]) {
	    usDays -= YrTab[i];

	    /* i = (i + 1) % 4; */
	    i++;
#ifdef DEBUG
	    if (i > 3) {
		dprintf ("Net_To_Dos_Date: going beyond YrTab\n");
	    }
#endif
	}

	/*  (days) = offset of days into year
	 *  (i) = offset into 4-year period
	 */

	pdd->years += i;

	j = 0;
	if (i == 0)
	    while (usDays >= MonTabLeap[j])
		usDays -= MonTabLeap[j++];
	else
	    while (usDays >= MonTab[j])
		usDays -= MonTab[j++];

	/*  (usDays) = offset of days into month
	 *  (j) = month of year - 1
	 */

	pdd->months = j + 1;
	pdd->days = usDays + 1;
    }
}

TIME  Dos_To_Net_Date (DOSDATE dd, DOSTIME dt)
{
    int32 time;
    uint16 i;

    time = (dd.years / 4) * FOURYEARS +     /*	days in four-year blocks */
	   (dd.years % 4) * 365;	    /*	days in normal years */

    /*	If we're past the leap year in the four-year block or if
     *	we're past February then account for leap day
     */
    if ((dd.years % 4) != 0 || dd.months > 2)
	time++;

    /*	(time) is the number of days to the beginning of this year
     */
    for (i = 1; i < dd.months; i++)
	time += MonTab[i - 1];

    /*	(time) is the number of days to the beginning of this month
     */

    time += dd.days - 1;
    return ((time * 24 + dt.hours) * 60 + dt.minutes) * 60 + dt.seconds2 * 2 + _70_to_80_bias;
}

int32	Validate_Dos_Date_Time(DOSDATE dd, DOSTIME dt)
{
	/* Both values zero, okay */
	if ((*(USHORT *)&dd == 0) && (*(USHORT *)&dt == 0))
		return 0;

	if ((dd.days > 366) || (dd.months < 1) || (dd.months > 12) ||
	    (dt.seconds2 > 29) || (dt.minutes > 59) || (dt.hours > 23))
		return ERROR_INVALID_PARAMETER;

	return 0;
}
