
/*
 *@@sourcefile nls.h:
 *      header file for nls.c. See notes there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #define INCL_DOSDATETIME
 *@@include #include <os2.h>
 *@@include #include "helpers\nls.h"
 */

/*
 *      Copyright (C) 1997-2001 Ulrich M”ller.
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

#if __cplusplus
extern "C" {
#endif

#ifndef NLS_HEADER_INCLUDED
    #define NLS_HEADER_INCLUDED

    #include "simples.h"
            // V0.9.19 (2002-06-13) [umoeller]

    /* ******************************************************************
     *
     *   DBCS support
     *
     ********************************************************************/

    #define TYPE_SBCS           0x0000
    #define TYPE_DBCS_1ST       0x0001
    #define TYPE_DBCS_2ND       0x0002

    ULONG XWPENTRY nlsQueryCodepage(VOID);

    BOOL XWPENTRY nlsDBCS(VOID);

    ULONG XWPENTRY nlsQueryDBCSChar(PCSZ pcszString,
                                    ULONG ulOfs);

    PSZ XWPENTRY nlschr(PCSZ p, char c);

    PSZ XWPENTRY nlsrchr(PCSZ p, char c);

    /* ******************************************************************
     *
     *   Country-dependent formatting
     *
     ********************************************************************/

    #ifdef OS2_INCLUDED

        /*
         *@@ COUNTRYSETTINGS:
         *      structure used for returning country settings
         *      with nlsQueryCountrySettings.
         */

        typedef struct _COUNTRYSETTINGS
        {
            ULONG   ulDateFormat,
                            // date format:
                            // --  0   mm.dd.yyyy  (English)
                            // --  1   dd.mm.yyyy  (e.g. German)
                            // --  2   yyyy.mm.dd  (Japanese)
                            // --  3   yyyy.dd.mm
                    ulTimeFormat;
                            // time format:
                            // --  0   12-hour clock
                            // --  >0  24-hour clock
            CHAR    cDateSep,
                            // date separator (e.g. '/')
                    cTimeSep,
                            // time separator (e.g. ':')
                    cDecimal,
                            // decimal separator (e.g. '.')
                    cThousands;
                            // thousands separator (e.g. ',')
        } COUNTRYSETTINGS, *PCOUNTRYSETTINGS;

        /*
         *@@ COUNTRYAMPM:
         *
         *@@added V1.0.1 (2003-01-17) [umoeller]
         */

        typedef struct _COUNTRYAMPM
        {
            CHAR            sz2359[10],
                            sz1159[10];
        } COUNTRYAMPM, *PCOUNTRYAMPM;

        /*
         *@@ COUNTRYSETTINGS:
         *      second structure to finally get the AM/PM
         *      stuff right too. I suspect people have used
         *      this structure in XCenter DLLs and such so
         *      I cannot easily change it... hence a second
         *      one.
         *
         *@@changed V1.0.1 (2003-01-17) [umoeller]
         */

        typedef struct _COUNTRYSETTINGS2
        {
            COUNTRYSETTINGS cs;
            COUNTRYAMPM     ampm;
        } COUNTRYSETTINGS2, *PCOUNTRYSETTINGS2;

        VOID XWPENTRY nlsQueryCountrySettings(PCOUNTRYSETTINGS2 pcs);

        PSZ XWPENTRY nlsThousandsULong(PSZ pszTarget, ULONG ul, CHAR cThousands);
        typedef PSZ XWPENTRY NLSTHOUSANDSULONG(PSZ pszTarget, ULONG ul, CHAR cThousands);
        typedef NLSTHOUSANDSULONG *PNLSTHOUSANDSULONG;

        PSZ XWPENTRY nlsThousandsDouble(PSZ pszTarget, double dbl, CHAR cThousands);

        PSZ XWPENTRY nlsVariableDouble(PSZ pszTarget,
                                       double dbl,
                                       PCSZ pszUnits,
                                       CHAR cThousands);

        VOID XWPENTRY nlsDate(const COUNTRYSETTINGS2 *pcs2,
                              PSZ pszDate,
                              USHORT year,
                              BYTE month,
                              BYTE day);

        VOID XWPENTRY nlsTime(const COUNTRYSETTINGS2 *pcs,
                              PSZ pszTime,
                              BYTE hours,
                              BYTE minutes,
                              BYTE seconds);

        VOID XWPENTRY nlsFileDate(PSZ pszBuf,
                                  const FDATE *pfDate,
                                  const COUNTRYSETTINGS2 *pcs);

        VOID XWPENTRY nlsFileTime(PSZ pszBuf,
                                  const FTIME *pfTime,
                                  const COUNTRYSETTINGS2 *pcs);

        VOID XWPENTRY nlsDateTime(PSZ pszDate,
                                  PSZ pszTime,
                                  const DATETIME *pDateTime,
                                  ULONG ulDateFormat,
                                  CHAR cDateSep,
                                  ULONG ulTimeFormat,
                                  CHAR cTimeSep);
        typedef VOID XWPENTRY NLSDATETIME(PSZ pszDate,
                                          PSZ pszTime,
                                          const DATETIME *pDateTime,
                                          ULONG ulDateFormat,
                                          CHAR cDateSep,
                                          ULONG ulTimeFormat,
                                          CHAR cTimeSep);
        typedef NLSDATETIME *PNLSDATETIME;

        VOID XWPENTRY nlsDateTime2(PSZ pszDate,
                                   PSZ pszTime,
                                   const DATETIME *pDateTime,
                                   const COUNTRYSETTINGS2 *pcs2);
        typedef VOID XWPENTRY NLSDATETIME2(PSZ pszDate,
                                           PSZ pszTime,
                                           const DATETIME *pDateTime,
                                           const COUNTRYSETTINGS2 *pcs2);
        typedef NLSDATETIME2 *PNLSDATETIME2;

    #endif

    ULONG nlsUpper(PSZ psz);

#endif

#if __cplusplus
}
#endif

