/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  Contains the following functions:
*      SysQueryProcessCodePage
*      SysGetCollate
*      SysMapCase
*      SysNationalLanguageCompare
*      SysSetProcessCodePage
*
*  Michael Greene, January 2008
*
------------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or                 */
/* without modification, are permitted provided that the following            */
/* conditions are met:                                                        */
/*                                                                            */
/* Redistributions of source code must retain the above copyright             */
/* notice, this list of conditions and the following disclaimer.              */
/* Redistributions in binary form must reproduce the above copyright          */
/* notice, this list of conditions and the following disclaimer in            */
/* the documentation and/or other materials provided with the distribution.   */
/*                                                                            */
/* Neither the name of Rexx Language Association nor the names                */
/* of its contributors may be used to endorse or promote products             */
/* derived from this software without specific prior written permission.      */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/******************************************************************************/

#define INCL_DOSNLS

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rexxdefs.h>  // rexxsaa.h include in this header

RexxFunctionHandler SysQueryProcessCodePage;
RexxFunctionHandler SysGetCollate;
RexxFunctionHandler SysMapCase;
RexxFunctionHandler SysNationalLanguageCompare;
RexxFunctionHandler SysSetProcessCodePage;


/*************************************************************************
* Function:  SysProcessType                                              *
*                                                                        *
* Params:    None                                                        *
*                                                                        *
* Return:    0 Full screen protect mode session.                         *
*            1 Requires real mode.                                       *
*            2 VIO windowable protect mode session.                      *
*            3 Presentation Manager protect mode session.                *
*            4 Detached protect mode process.                            *
*                                                                        *
* OS/2 1.x had a now obsolete process type 1 (real mode). Actual OS/2    *
* session types are 0 (full screen, text mode), 2 (VIO, text window),    *
* 3 (PM), or 4 (DETACHed).                                               *
*                                                                        *
*************************************************************************/

unsigned long SysQueryProcessCodePage(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long aulCpList[8]  = {0};                /* Code page list        */
    unsigned long ulBufSize     = 8 * sizeof(ULONG);  /* Size of output list   */
    unsigned long ulListSize    = 0;                  /* Size of list returned */

    if (numargs != 0) return INVALID_ROUTINE;

    DosQueryCp(ulBufSize, aulCpList, &ulListSize);

    RETVAL(aulCpList[0])
}


/*************************************************************************
* Function:  SysGetCollate                                               *
*                                                                        *
* Params:    SysGetCollate country, codepage                             *
*                                                                        *
* country -  The three-digit country code. A value of 0 indicates the    *
*            currently-active country code; this is the default.         *
* codepage - The codepage number. A value of 0 indicates the currently-  *
*            active codepage; this is the default.                       *
*                                                                        *
* Return:    SysGetCollate returns a 256-character string containing     *
*            the collating sequence table.                               *
*                                                                        *
*                                                                        *
*************************************************************************/

unsigned long SysGetCollate(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long SeqLen;
    unsigned char achColSeq[256] = {0};

    // the default values, if not user provided, are 0
    COUNTRYCODE UserInfo = {0,0};

    if (numargs > 2) return INVALID_ROUTINE;

    /* Request information about current country */
    if ((numargs == 1) || (numargs == 2)) {
        if (args[0].strptr != 0 || args[0].strptr != NULL)
            string2ulong(args[0].strptr, &UserInfo.country);
    }

    /* ... and current code page                 */
    if (numargs == 2) {
        if (args[1].strptr != 0 || args[1].strptr != NULL)
            string2ulong(args[1].strptr, &UserInfo.codepage);
    }

    DosQueryCollate(sizeof(achColSeq), &UserInfo, achColSeq, &SeqLen);

    memcpy( retstr->strptr, achColSeq, SeqLen);
    retstr->strlength = SeqLen;

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysMapCase                                                  *
*                                                                        *
* Params:    SysMapCase  string , country , codepage                     *
*                                                                        *
* string  -  String to be converted to uppercase                         *
* country -  The three-digit country code. A value of 0 indicates the    *
*            currently-active country code; this is the default.         *
* codepage - The codepage number. A value of 0 indicates the currently-  *
*            active codepage; this is the default.                       *
*                                                                        *
* Return:    Returns the uppercased form of the specified string         *
*                                                                        *
*************************************************************************/

unsigned long SysMapCase(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    // the default values, if not user provided, are 0
    COUNTRYCODE UserInfo = {0,0};

    if ((numargs < 1) || (numargs > 3)) return INVALID_ROUTINE;

    /* Request information about current country */
    if ((numargs == 2) || (numargs == 3)) {
        if (args[1].strptr != 0 || args[1].strptr != NULL)
            string2ulong(args[1].strptr, &UserInfo.country);
    }

    /* ... and current code page                 */
    if (numargs == 3) {
        if (args[2].strptr != 0 || args[2].strptr != NULL)
            string2ulong(args[2].strptr, &UserInfo.codepage);
    }

    DosMapCase(args[0].strlength, &UserInfo, args[0].strptr);

    memcpy( retstr->strptr, args[0].strptr, args[0].strlength);
    retstr->strlength = args[0].strlength;

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysNationalLanguageCompare                                  *
*                                                                        *
* Params:    SysNationalLanguageCompare string1, string2,                *
*                                             country, codepage          *
*                                                                        *
* string1  - The first string to compare                                 *
* string2  - The second string to compare                                *
* country -  The three-digit country code. A value of 0 indicates the    *
*            currently-active country code; this is the default.         *
* codepage - The codepage number. A value of 0 indicates the currently-  *
*            active codepage; this is the default.                       *
*                                                                        *
* Return:     0 string1 and string2 are equal.                           *
*             1 string1 has a greater value than string2.                *
*            -1 string2 has a greater value than string1.                *
*                                                                        *
*************************************************************************/

unsigned long SysNationalLanguageCompare(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    int  result = 0;
    int  longstring;

    unsigned long SeqLen;
    unsigned char achColSeq[256] = {0};
    unsigned char *inString;
    unsigned char *outString;

    // the default values
    COUNTRYCODE UserInfo = {0,0};

    if (numargs < 2 || numargs > 4 || !RXVALIDSTRING(args[0]) ||
           !RXVALIDSTRING(args[1])) return INVALID_ROUTINE;

    if (numargs >= 3 && RXVALIDSTRING(args[2]))
            string2ulong(args[2].strptr, &UserInfo.country);

    if (numargs == 4) {
            string2ulong(args[3].strptr, &UserInfo.codepage);
    }

    if ( DosQueryCollate(sizeof(achColSeq), &UserInfo, achColSeq, &SeqLen) )
                             return INVALID_ROUTINE;

    longstring = min(args[0].strlength, args[1].strlength);

    // strings passed in for comparison
    inString  = args[0].strptr;
    outString = args[1].strptr;

    // while result !0 and haven't worked through string
    while (!result && longstring) {
        longstring--;
        result = achColSeq[*inString++] - achColSeq[*outString++];
    }

    if (!result) result = (int)args[0].strlength - (int)args[1].strlength;

    if (result < 0) result = -1;
    else if (result > 0) result = 1;

    RETVAL(result)
}


/*************************************************************************
* Function:  SysSetProcessCodePage                                       *
*                                                                        *
* Params:    SysSetProcessCodePage codepage                              *
*                                                                        *
* codepage - The codepage number.                                        *
*                                                                        *
* Return:     0 Success.                                                 *
*           472 Invalid code page.                                       *
*                                                                        *
*************************************************************************/

unsigned long SysSetProcessCodePage(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long codepage;

    if (numargs !=1) return INVALID_ROUTINE;

    string2ulong(args[0].strptr, &codepage);

    RETVAL(DosSetProcessCp(codepage))
}


