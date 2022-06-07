
/*
 *@@sourcefile datetime.c:
 *      contains various date and time helper functions.
 *      Some functions in here are OS/2-specific, others
 *      are plain C code.
 *
 *      Functions marked with (C) Ray Gardner are from
 *      "scaldate.c":
 *          scalar date routines    --    public domain by Ray Gardner
 *          These will work over the range 1/01/01 thru 14699/12/31
 *
 *      Usage: All OS/2 programs.
 *
 *      Function prefixes (new with V0.81):
 *      --  dat*   date/time helper functions
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\datetime.h"
 *@@added V0.9.0 [umoeller]
 */

/*
 *      This file Copyright (C) 1997-2000 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSMISC
#include <os2.h>

#include <stdio.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\datetime.h"

#pragma hdrstop

/*
 *@@category: Helpers\C helpers\Date/time helpers
 *      See datetime.c.
 */

/*******************************************************************
 *                                                                 *
 *  Private declarations                                           *
 *                                                                 *
 ******************************************************************/

const char  *pcszFormatTimestamp = "%4u%02u%02u%02u%02u%02u%";

ULONG G_ulDateScalarFirstCalled = 0;

/*
 *@@ dtGetULongTime:
 *      this returns the current time as a ULONG value (in milliseconds).
 *      Useful for stopping how much time the machine has spent in
 *      a certain function. To do this, simply call this function twice,
 *      and subtract the two values, which will give you the execution
 *      time in milliseconds.
 *
 *      A ULONG can hold a max value of 4'294'967'295.
 *      So this overflows after 49.71... days.
 *
 *@@changed V0.9.7 (2000-12-08) [umoeller]: replaced, now using DosQuerySysInfo(QSV_MS_COUNT)
 */

ULONG dtGetULongTime(VOID)
{
    ULONG ulTimeNow;
    DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT,
                    &ulTimeNow,
                    sizeof(ulTimeNow));
    return ulTimeNow;
}

/*
 *@@ dtCreateFileTimeStamp:
 *      this creates a time stamp string in pszTimeStamp
 *      from the given FDATE and FTIME structures (which
 *      are, for example, used in the FILESTATUS3 structure
 *      returned by DosQueryPathInfo).
 *
 *      The time stamp string is exactly 15 bytes in length
 *      (including the terminating null byte) and has the
 *      following format:
 +          YYYYMMDDhhmmss
 *      (being year, mondth, day, hours, minutes, seconds).
 *      Your buffer must be large enough for that, this is
 *      not checked.
 *
 *      This time stamp can be used to compare two dates
 *      simply by calling strcmp.
 *
 *      Note that since FTIME only has a two-seconds resolution,
 *      the seconds part of the time stamp will have that too.
 *
 *      This returns the string length (excluding the null
 *      terminator), which should be 14.
 *
 *@@added V0.9.0 [umoeller]
 */

int dtCreateFileTimeStamp(PSZ pszTimeStamp,      // out: time stamp
                          FDATE* pfdate,         // in: date
                          FTIME* pftime)         // in: time
{
    return sprintf(pszTimeStamp,
                   pcszFormatTimestamp,
                   pfdate->year + 1980,
                   pfdate->month,
                   pfdate->day,
                   pftime->hours,
                   pftime->minutes,
                   pftime->twosecs * 2);
}

/*
 *@@ dtCreateDosTimeStamp:
 *      just like dtCreateFileTimeStamp,
 *      except that this takes a DATETIME
 *      structure as input. The time stamp
 *      string is exactly the same.
 *
 *@@added V0.9.0 [umoeller]
 */

int dtCreateDosTimeStamp(PSZ pszTimeStamp,
                         DATETIME* pdt)
{
    return sprintf(pszTimeStamp,
                   pcszFormatTimestamp,
                   pdt->year,
                   pdt->month,
                   pdt->day,
                   pdt->hours,
                   pdt->minutes,
                   pdt->seconds);
}

/*
**
**
**   day:    day of month
**   mon:    month (1-12)
**   yr:     year
**
**
*/

/*
 *@@ dtDayOfWeek:
 *      returns an integer that represents the day of
 *      the week for the date passed as parameters.
 *
 *      Returns 0-6 where 0 is sunday.
 *
 *@@added V0.9.7 (2000-12-05) [umoeller]
 */

ULONG dtDayOfWeek(ULONG day,
                  ULONG mon,    // 1-12
                  ULONG yr)
{
    int dow;

    if (mon <= 2)
    {
          mon += 12;
          yr -= 1;
    }
    dow = (   day
            + mon * 2
            + ((mon + 1) * 6) / 10
            + yr
            + yr / 4
            - yr / 100
            + yr / 400
            + 2);
    dow = dow % 7;
    return ((dow ? dow : 7) - 1);
}

/*
 *@@ dtIsLeapYear:
 *      returns TRUE if yr is a leap year.
 *
 *      (W) Ray Gardner. Public domain.
 */

int dtIsLeapYear(unsigned yr)
{
   return (    (yr % 400 == 0)
            || (    (yr % 4 == 0)
                 && (yr % 100 != 0)
               )
          );
}

/*
 *@@ dtMonths2Days:
 *      returns the no. of days for the beginning
 *      of "month" (starting from 1).
 *
 *      For example, if you pass 1 (for january),
 *      you get 0 because there's no days at jan 1st
 *      yet.
 *
 *      If you pass 2 (for february), you get 31.
 *
 *      If you pass 3 (for march), you get 61.
 *
 *      This is useful for computing a day index
 *      for a given month/day pair. Pass the month
 *      in here and add (day-1); for march 3rd,
 *      you then get 63.
 *
 *      (W) Ray Gardner. Public domain.
 */

unsigned dtMonths2Days(unsigned month)
{
    return (month * 3057 - 3007) / 100;
}

/*
 *@@ dtYears2Days:
 *      converts a year to the no. of days passed.
 *
 *      (W) Ray Gardner. Public domain.
 */

long dtYears2Days (unsigned yr)
{
   return (   yr * 365L
            + yr / 4
            - yr / 100
            + yr / 400);
}

/*
 *@@ dtDate2Scalar:
 *      returns a scalar (i.e. the no. of days) for
 *      the given date.
 *
 *      (W) Ray Gardner. Public domain.
 */

long dtDate2Scalar(unsigned yr,     // in: year     (e.g. 1999)
                   unsigned mo,     // in: month    (1-12)
                   unsigned day)    // in: day      (1-31)
{
   long scalar;
   scalar = day + dtMonths2Days(mo);
   if ( mo > 2 )                         /* adjust if past February */
      scalar -= dtIsLeapYear(yr) ? 1 : 2;
   yr--;
   scalar += dtYears2Days(yr);
   return scalar;
}

/*
 *@@ dtScalar2Date:
 *
 *
 *      (W) Ray Gardner. Public domain.
 */

void dtScalar2Date(long scalar,     // in: date scalar
                   unsigned *pyr,   // out: year    (e.g. 1999)
                   unsigned *pmo,   // out: month   (1-12)
                   unsigned *pday)  // out: day     (1-31)
{
   unsigned n;                /* compute inverse of dtYears2Days() */

   for ( n = (unsigned)((scalar * 400L) / 146097); dtYears2Days(n) < scalar;)
      n++;                          /* 146097 == dtYears2Days(400) */
   *pyr = n;
   n = (unsigned)(scalar - dtYears2Days(n-1));
   if ( n > 59 )    /* adjust if past February */
   {
      n += 2;
      if ( dtIsLeapYear(*pyr) )
         n -= n > 62 ? 1 : 2;
   }
   *pmo = (n * 100 + 3007) / 3057;  /* inverse of dtMonths2Days() */
   *pday = n - dtMonths2Days(*pmo);
}

/*
 *@@ dtIsValidDate:
 *      returns TRUE if the given date is valid.
 *
 *@@added V0.9.7 (2000-12-05) [umoeller]
 */

BOOL dtIsValidDate(LONG day,      // in: day (1-31)
                   LONG month,    // in: month (1-12)
                   ULONG year)    // in: year (e.g. 1999)
{
    if (day > 0)
    {
        switch( month )
        {
            case 1  :
            case 3  :
            case 5  :
            case 7  :
            case 8  :
            case 10 :
            case 12 :
                if (day <= 31)
                    return TRUE;
            break;

            case 4  :
            case 6  :
            case 9  :
            case 11 :
                if (day <= 30)
                    return TRUE;
            break;

            case 2 :
                if (day < 29)
                    return TRUE;
                else
                    if (day == 29)
                        if (dtIsLeapYear(year))
                            return TRUE;
        }
    }

    return FALSE;
}

