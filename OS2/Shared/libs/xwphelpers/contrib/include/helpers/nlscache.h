
/*
 *@@sourcefile nlscache.h:
 *      header file for nlscache.c. See notes there.
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

#ifndef NLSCACHE_HEADER_INCLUDED
    #define NLSCACHE_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   NLS strings
     *
     ********************************************************************/

    /*
     *@@ STRINGENTITY:
     *
     *@@added V0.9.16 (2001-09-29) [umoeller]
     */

    typedef struct _STRINGENTITY
    {
        PCSZ    pcszEntity;
        PCSZ    *ppcszString;
    } STRINGENTITY, *PSTRINGENTITY;

    typedef const struct _STRINGENTITY *PCSTRINGENTITY;

    VOID XWPENTRY nlsInitStrings(HAB hab,
                                 HMODULE hmod,
                                 PCSTRINGENTITY paEntities,
                                 ULONG cEntities);

    #ifdef XSTRING_HEADER_INCLUDED
        ULONG XWPENTRY nlsReplaceEntities(PXSTRING pstr);
    #endif

    VOID XWPENTRY nlsLoadString(ULONG ulID,
                                PSZ *ppsz,
                                PULONG pulLength);

    PCSZ XWPENTRY nlsGetString(ULONG ulStringID);


#endif

#if __cplusplus
}
#endif

