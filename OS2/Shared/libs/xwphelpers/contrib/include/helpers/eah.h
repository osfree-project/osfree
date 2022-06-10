
/*
 *@@sourcefile eah.h:
 *      header file for eah.c. See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #include <os2.h>
 *@@include #include "helpers\eah.h"
 */

/*
 *      Most of the code in this file dealing with Extended Attributes
 *      is based on code (w) by Chris Hanson (cph@zurich.ai.mit.edu).
 *      Copyright (c) 1995 Massachusetts Institute of Technology.
 *      The original code is available as EALIB.ZIP at Hobbes.
 *
 *      From that code is the following copyright notice:
 *
 *      This material was developed by the Scheme project at the Massachusetts
 *      Institute of Technology, Department of Electrical Engineering and
 *      Computer Science.  Permission to copy this software, to redistribute
 *      it, and to use it for any purpose is granted, subject to the following
 *      restrictions and understandings.
 *
 *      1. Any copy made of this software must include this copyright notice
 *      in full.
 *
 *      2. Users of this software agree to make their best efforts (a) to
 *      return to the MIT Scheme project any improvements or extensions that
 *      they make, so that these may be included in future releases; and (b)
 *      to inform MIT of noteworthy uses of this software.
 *
 *      3. All materials developed as a consequence of the use of this
 *      software shall duly acknowledge such use, in accordance with the usual
 *      standards of acknowledging credit in academic research.
 *
 *      4. MIT has made no warrantee or representation that the operation of
 *      this software will be error-free, and MIT is under no obligation to
 *      provide any services, by way of maintenance, update, or otherwise.
 *
 *      5. In conjunction with products arising from the use of this material,
 *      there shall be no use of the name of the Massachusetts Institute of
 *      Technology nor of any adaptation thereof in any advertising,
 *      promotional, or sales literature without prior written consent from
 *      MIT in each case.
 *
 *      This file Copyright (C) 1997-2000 Ulrich M”ller,
 *                                        Massachusetts Institute of Technology.
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

#ifndef EAS_HEADER_INCLUDED
    #define EAS_HEADER_INCLUDED

    /*
     *@@ EABINDING:
     *      generic data structure used for all
     *      the EA functions.
     *
     *      usValueLength gives you the length of *pszValue,
     *      which is _not_ necessarily zero-terminated.
     *
     *      The format of pszValue depends on the EA type.
     *      This pointer points to the "raw" EA data.
     *
     *      The first USHORT gives you the EA type (EAT_* flag),
     *      on which the other bytes depend.
     *
     *      Most EA types have their size in the second USHORT.
     *      All EA formats are described in detail in CPREF.
     */

    typedef struct _EABINDING
    {
        BYTE    bFlags;
                // EA flags, as in FEA2.fEA. This is either
                // FFEA_NEEDEA (for critical EAs) or 0.
        BYTE    bNameLength;        // length of (*pszName)
        USHORT  usValueLength;      // length of (*pszValue)
        PSZ     pszName;            // EA name (e.g. ".CLASSINFO")
        PSZ     pszValue;           // complete raw EA contents.
                                    // The first USHORT gives you the type,
                                    // the second USHORT normally the size.
    } EABINDING, *PEABINDING;

    /*
     *@@ EALIST:
     *      list structure returned by eaPathReadAll and
     *      eaHFileReadAll.
     */

    typedef struct _EALIST
    {
        PEABINDING      peab;
        struct _EALIST  *next;
    } EALIST, *PEALIST;

    void eaFreeBinding(PEABINDING peab);

    void eaFreeList(PEALIST list);

    /* ******************************************************************
     *
     *   Read-EA functions
     *
     ********************************************************************/

    ULONG eaHFileQueryTotalSize(HFILE hFile);

    ULONG eaPathQueryTotalSize(const char *pcszPath);

    PEALIST eaHFileReadAll(HFILE hfile);

    PEALIST eaPathReadAll(const char *pcszPath);

    PEABINDING eaHFileReadOneByIndex(HFILE hfile, ULONG index);

    PEABINDING eaPathReadOneByIndex(const char *pcszPath, ULONG index);

    PEABINDING eaHFileReadOneByName(HFILE hfile, const char *pcszEAName);

    PEABINDING eaPathReadOneByName(const char *pcszPath, const char *pcszEAName);

    /* ******************************************************************
     *
     *   Write-EA functions
     *
     ********************************************************************/

    APIRET eaHFileWriteAll(HFILE hfile, PEALIST list);

    APIRET eaPathWriteAll(const char *pcszPath, PEALIST list);

    APIRET eaHFileWriteOne(HFILE hfile, PEABINDING peab);

    APIRET eaPathWriteOne(const char *pcszPath, PEABINDING peab);

    APIRET eaPathDeleteOne(const char *pcszPath, const char *pcszEAName);

    /********************************************************************
     *
     *   Translation funcs
     *
     ********************************************************************/

    USHORT eaQueryEAType(PEABINDING peab);

    PSZ eaCreatePSZFromBinding(PEABINDING peab);

    PEABINDING eaCreateBindingFromPSZ(const char *pcszEAName,
                                      const char *pcszInput);

    USHORT eaQueryMVCount(PEABINDING peab,
                          PUSHORT pusCodepage,
                          PUSHORT pusEAType);

    PSZ eaQueryMVItem(PEABINDING peab,
                      USHORT usIndex,
                      PUSHORT pusEAType,
                      PUSHORT pusCodepage,
                      PUSHORT pusDataLength);

    PSZ eaCreatePSZFromMVBinding(PEABINDING peab,
                                 const char *pcszSeparator,
                                 PUSHORT pusCodepage);

    PEABINDING eaCreateMVBindingFromPSZ(const char *pcszEAName,
                                        const char *pcszInput,
                                        const char *pcszSeparator,
                                        USHORT usCodepage);

    /* ******************************************************************
     *
     *   Direct plain-string EA handling
     *
     ********************************************************************/

    typedef struct _EAMVMT
    {
        USHORT         usType;
        USHORT         usCodepage;
        USHORT         usEntries;
        USHORT         usEntryType;
        USHORT         usEntryLen;
        CHAR           chEntry[1];
    } EAMVMT, *PEAMVMT;

    typedef struct _EASVST
    {
        USHORT         usType;
        USHORT         usEntryLen;
        CHAR           chEntry[1];
    } EASVST, *PEASVST;

    #define MAX(a,b)   (a > b ? a : b)
    #define MAX(a,b)   (a > b ? a : b)
    #define NEXTSTR(s) (s+strlen(s) + 1)

    APIRET eahWriteStringEA(HFILE hfile,
                            PSZ pszEaName,
                            PSZ pszEaValue);

    APIRET eahReadStringEA(PSZ pszFileName,
                           PSZ pszEaName,
                           PSZ pszBuffer,
                           PULONG pulBuflen);
#endif

#if __cplusplus
}
#endif

