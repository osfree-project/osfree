
/*
 *@@sourcefile datetime.h:
 *      header file for datetime.c. See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #include <os2.h>
 *@@include #include "helpers\datetime.h"
 */

/*      Copyright (C) 1997-2000 Ulrich M”ller.
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

#ifndef DATETIME_HEADER_INCLUDED
    #define DATETIME_HEADER_INCLUDED

#if __cplusplus
extern "C" {
#endif

    ULONG dtGetULongTime(VOID);

    int dtCreateFileTimeStamp(PSZ pszTimeStamp,
                                FDATE* pfdate,
                                FTIME* pftime);

    int dtCreateDosTimeStamp(PSZ pszTimeStamp,
                             DATETIME* pdt);

    ULONG dtDayOfWeek(ULONG day,
                      ULONG mon,
                      ULONG yr);

    int dtIsLeapYear(unsigned yr);

    unsigned dtMonths2Days(unsigned month);

    long dtYears2Days(unsigned yr);

    long dtDate2Scalar(unsigned yr,
                       unsigned mo,
                       unsigned day);

    void dtScalar2Date(long scalar,
                       unsigned *pyr,
                       unsigned *pmo,
                       unsigned *pday);

    BOOL dtIsValidDate(LONG day,
                       LONG month,
                       ULONG year);
#if __cplusplus
}
#endif

#endif


