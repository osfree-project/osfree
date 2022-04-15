/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  Contains the following functions:
*      SysStemSort
*      SysStemDelet
*      SysStemInsert
*      SysStemCopy
*
*  Michael Greene, December 2007
*
*  SysStemSort from regutil 1.26 by Patrick TJ McPhee - see license concerning
*    this code below
*
*  The rest is from OOREXX source with few changes
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

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <alloca.h>

#include <rexxdefs.h>  // rexxsaa.h include in this header


/*********************************************************************/
/* Defines used by SysStemSort -- must match values in okstem.hpp    */
/*********************************************************************/

#define SORT_CASESENSITIVE 0
#define SORT_CASEIGNORE    1

#define SORT_ASCENDING     0
#define SORT_DECENDING     1

RexxFunctionHandler SysStemDelete;
RexxFunctionHandler SysStemInsert;
RexxFunctionHandler SysStemCopy;
RexxFunctionHandler SysStemSort;


/*************************************************************************
* Function:  SysStemDelete                                               *
*                                                                        *
* Syntax:    result = SysStemDelete(stem, startitem [,itemcount])        *
*                                                                        *
* Params:    stem - name of stem where item will be deleted              *
*            startitem - index of item to delete                         *
*            itemcount - number of items to delete if more than 1        *
*                                                                        *
* Return:    0 - delete was successful                                   *
*            -1 - delete failed                                          *
*************************************************************************/

unsigned long SysStemDelete(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long   rc;
    char            szStemName[255];
    char            *pszStemIdx;
    char            szValue[255];
    SHVBLOCK        shvb;
    unsigned long   ulIdx;
    unsigned long   ulFirst;
    unsigned long   ulItems = 1;
    unsigned long   ulCount;
    bool            fOk = TRUE;

//    char buffer[256];

    if ( (numargs < 2) || (numargs > 3) || /* validate arguments       */
        !RXVALIDSTRING(args[0]) || !RXVALIDSTRING(args[1]) ||
        ((numargs == 3) && !RXVALIDSTRING(args[2])) )
      return INVALID_ROUTINE;

//    sprintf(buffer,"SysStemDelete: %s %s\n", args[0].strptr, args[1].strptr);
//    logmessage(buffer);

    /* remember stem name */
    memset(szStemName, 0, sizeof(szStemName));
    strcpy(szStemName, args[0].strptr);

    if (szStemName[args[0].strlength-1] != '.') szStemName[args[0].strlength] = '.';
    pszStemIdx = &(szStemName[strlen(szStemName)]);

    /* get item index to be deleted */
    if (sscanf(args[1].strptr, "%ld", &ulFirst) != 1) return INVALID_ROUTINE;

    /* get number of items to delete */
    if (numargs == 3) {
        if (sscanf(args[2].strptr, "%ld", &ulItems) != 1) return INVALID_ROUTINE;
        if (ulItems == 0) return INVALID_ROUTINE;
    } /* endif */

    /* retrieve the number of elements in stem */
    strcpy(pszStemIdx, "0");
    shvb.shvnext = NULL;
    shvb.shvname.strptr = szStemName;
    shvb.shvname.strlength = strlen((const char *)szStemName);
    shvb.shvvalue.strptr = szValue;
    shvb.shvvalue.strlength = sizeof(szValue);
    shvb.shvnamelen = shvb.shvname.strlength;
    shvb.shvvaluelen = shvb.shvvalue.strlength;
    shvb.shvcode = RXSHV_SYFET;
    shvb.shvret = 0;

    if (RexxVariablePool(&shvb) == RXSHV_OK) {
        /* index retrieved fine */
        if (sscanf(shvb.shvvalue.strptr, "%ld", &ulCount) != 1) return INVALID_ROUTINE;

        /* check wether supplied index and count is within limits */
        if (ulFirst + ulItems - 1 > ulCount) return INVALID_ROUTINE;

        /* now copy the remaining indices up front */
        for (ulIdx = ulFirst; ulIdx + ulItems <= ulCount; ulIdx++) {

            /* get element to relocate */
            sprintf(pszStemIdx, "%ld", ulIdx + ulItems);
            shvb.shvnext = NULL;
            shvb.shvname.strptr = szStemName;
            shvb.shvname.strlength = strlen((const char *)szStemName);
            shvb.shvvalue.strptr = NULL;
            shvb.shvvalue.strlength = 0;
            shvb.shvnamelen = shvb.shvname.strlength;
            shvb.shvvaluelen = shvb.shvvalue.strlength;
            shvb.shvcode = RXSHV_SYFET;
            shvb.shvret = 0;

            if (RexxVariablePool(&shvb) == RXSHV_OK) {
                sprintf(pszStemIdx, "%ld", ulIdx);
                shvb.shvnext = NULL;
                shvb.shvname.strptr = szStemName;
                shvb.shvname.strlength = strlen((const char *)szStemName);
                shvb.shvnamelen = shvb.shvname.strlength;
                shvb.shvvaluelen = shvb.shvvalue.strlength;
                shvb.shvcode = RXSHV_SYSET;
                shvb.shvret = 0;

                rc = RexxVariablePool(&shvb);

                if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV)) fOk = FALSE;

                /* free memory allocated by REXX */
                DosFreeMem(shvb.shvvalue.strptr);
            } else fOk = FALSE;

            if (!fOk) break;
        } /* endfor */

        if (fOk) {
            /* now delete the items at the end */
            for (ulIdx = ulCount - ulItems + 1; ulIdx <= ulCount; ulIdx++) {
                sprintf(pszStemIdx, "%ld", ulIdx);
                shvb.shvnext = NULL;
                shvb.shvname.strptr = szStemName;
                shvb.shvname.strlength = strlen((const char *)szStemName);
                shvb.shvvalue.strptr = NULL;
                shvb.shvvalue.strlength = 0;
                shvb.shvnamelen = shvb.shvname.strlength;
                shvb.shvvaluelen = shvb.shvvalue.strlength;
                shvb.shvcode = RXSHV_SYDRO;
                shvb.shvret = 0;

                if (RexxVariablePool(&shvb) != RXSHV_OK) {
                    fOk = FALSE;
                    break;
                } /* endif */
            } /* endfor */
        } /* endif */

        if (fOk) {
            /* set the new number of items in the stem array */
            strcpy(pszStemIdx, "0");
            sprintf(szValue, "%ld", ulCount - ulItems);
            shvb.shvnext = NULL;
            shvb.shvname.strptr = szStemName;
            shvb.shvname.strlength = strlen((const char *)szStemName);
            shvb.shvvalue.strptr = szValue;
            shvb.shvvalue.strlength = strlen(szValue);
            shvb.shvnamelen = shvb.shvname.strlength;
            shvb.shvvaluelen = shvb.shvvalue.strlength;
            shvb.shvcode = RXSHV_SYSET;
            shvb.shvret = 0;

            rc = RexxVariablePool(&shvb);

            if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV)) fOk = FALSE;
        } /* endif */

    } else fOk = FALSE;


    if (fOk) RETVAL(0)
    else RETVAL(-1)
}


/*************************************************************************
* Function:  SysStemInsert                                               *
*                                                                        *
* Syntax:    result = SysStemInsert(stem, position, value)               *
*                                                                        *
* Params:    stem - name of stem where item will be inserted             *
*            position - index where new item will be inserted            *
*            value - new item value                                      *
*                                                                        *
* Return:    0 - insert was successful                                   *
*            -1 - insert failed                                          *
*************************************************************************/

unsigned long SysStemInsert(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long   rc;
    char            szStemName[255];
    char            *pszStemIdx;
    char            szValue[255];
    SHVBLOCK        shvb;
    unsigned long   ulIdx;
    unsigned long   ulPosition;
    unsigned long   ulCount;
    bool            fOk = TRUE;

//    char buffer[256];

    if ( (numargs != 3) ||  /* validate arguments       */
        !RXVALIDSTRING(args[0]) || !RXVALIDSTRING(args[1]) ||
        RXNULLSTRING(args[2]) )
      return INVALID_ROUTINE;

//    sprintf(buffer, "SysStemInsert: %s  %s  %s\n", args[0].strptr, args[1].strptr, args[2].strptr);
//    logmessage(buffer);

    /* remember stem name */
    memset(szStemName, 0, sizeof(szStemName));
    strcpy(szStemName, args[0].strptr);

    if (szStemName[args[0].strlength-1] != '.') szStemName[args[0].strlength] = '.';
    pszStemIdx = &(szStemName[strlen(szStemName)]);

    /* get new item index */
    if (sscanf(args[1].strptr, "%ld", &ulPosition) != 1) return INVALID_ROUTINE;

    /* retrieve the number of elements in stem */
    strcpy(pszStemIdx, "0");
    shvb.shvnext = NULL;
    shvb.shvname.strptr = szStemName;
    shvb.shvname.strlength = strlen((const char *)szStemName);
    shvb.shvvalue.strptr = szValue;
    shvb.shvvalue.strlength = sizeof(szValue);
    shvb.shvnamelen = shvb.shvname.strlength;
    shvb.shvvaluelen = shvb.shvvalue.strlength;
    shvb.shvcode = RXSHV_SYFET;
    shvb.shvret = 0;
    if (RexxVariablePool(&shvb) == RXSHV_OK) {
        /* index retrieved fine */
        if (sscanf(shvb.shvvalue.strptr, "%ld", &ulCount) != 1) return INVALID_ROUTINE;

        /* check wether new position is within limits */
        if ((ulPosition == 0) || (ulPosition > ulCount + 1)) return INVALID_ROUTINE;

        /* make room for new item by moving all items to the end */
        for (ulIdx = ulCount; ulIdx >= ulPosition; ulIdx--) {
            /* get element to relocate */
            sprintf(pszStemIdx, "%ld", ulIdx);
            shvb.shvnext = NULL;
            shvb.shvname.strptr = szStemName;
            shvb.shvname.strlength = strlen((const char *)szStemName);
            shvb.shvvalue.strptr = NULL;
            shvb.shvvalue.strlength = 0;
            shvb.shvnamelen = shvb.shvname.strlength;
            shvb.shvvaluelen = shvb.shvvalue.strlength;
            shvb.shvcode = RXSHV_SYFET;
            shvb.shvret = 0;

            if (RexxVariablePool(&shvb) == RXSHV_OK) {
                sprintf(pszStemIdx, "%ld", ulIdx + 1);
                shvb.shvnext = NULL;
                shvb.shvname.strptr = szStemName;
                shvb.shvname.strlength = strlen((const char *)szStemName);
                shvb.shvnamelen = shvb.shvname.strlength;
                shvb.shvvaluelen = shvb.shvvalue.strlength;
                shvb.shvcode = RXSHV_SYSET;
                shvb.shvret = 0;

                rc = RexxVariablePool(&shvb);

                if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV)) fOk = FALSE;

                /* free memory allocated by REXX */
                DosFreeMem(shvb.shvvalue.strptr);
            } else fOk = FALSE;

            if (!fOk) break;
        } /* endfor */

        if (fOk) {
            /* set the new item value */
            sprintf(pszStemIdx, "%ld", ulPosition);
            shvb.shvnext = NULL;
            shvb.shvname.strptr = szStemName;
            shvb.shvname.strlength = strlen((const char *)szStemName);
            shvb.shvvalue.strptr = args[2].strptr;
            shvb.shvvalue.strlength = args[2].strlength;
            shvb.shvnamelen = shvb.shvname.strlength;
            shvb.shvvaluelen = shvb.shvvalue.strlength;
            shvb.shvcode = RXSHV_SYSET;
            shvb.shvret = 0;

            rc = RexxVariablePool(&shvb);

            if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV)) fOk = FALSE;
        } /* endif */

        if (fOk) {
            /* set the new number of items in the stem array */
            strcpy(pszStemIdx, "0");
            sprintf(szValue, "%ld", ulCount + 1);
            shvb.shvnext = NULL;
            shvb.shvname.strptr = szStemName;
            shvb.shvname.strlength = strlen((const char *)szStemName);
            shvb.shvvalue.strptr = szValue;
            shvb.shvvalue.strlength = strlen(szValue);
            shvb.shvnamelen = shvb.shvname.strlength;
            shvb.shvvaluelen = shvb.shvvalue.strlength;
            shvb.shvcode = RXSHV_SYSET;
            shvb.shvret = 0;

            rc = RexxVariablePool(&shvb);

            if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV)) fOk = FALSE;
        } /* endif */
    } else fOk = FALSE;

    if (fOk) RETVAL(0)
    else RETVAL(-1)
}


/*************************************************************************
* Function:  SysStemCopy                                                 *
*                                                                        *
* Syntax:    result = SysStemCopy(fromstem, tostem, from, to, count      *
*                                 [,insert])                             *
*                                                                        *
* Params:    fromstem - name of source stem                              *
*            tostem - - name of target stem                              *
*            from  - first index in source stem to copy                  *
*            to - position where items are copied/inserted in target stem*
*            count - number of items to copy/insert                      *
*            insert - 'I' to indicate insert instead of 'O' overwrite    *
*                                                                        *
* Return:    0 - stem copy was successful                                *
*            -1 - stem copy failed                                       *
*************************************************************************/

unsigned long SysStemCopy(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;
    char          szFromStemName[255];
    char          szToStemName[255];
    char          *pszFromStemIdx;
    char          *pszToStemIdx;
    char          szValue[255];
    SHVBLOCK      shvb;
    unsigned long ulIdx;
    unsigned long ulToCount;
    unsigned long ulFromCount;
    unsigned long ulFrom = 1;
    unsigned long ulTo = 1;
    unsigned long ulCopyCount = 0;
    bool          fInsert = FALSE;
    bool          fOk = TRUE;

//    char buffer[256];

    if ( (numargs < 2) || (numargs > 6) ||  /* validate arguments      */
        !RXVALIDSTRING(args[0]) || !RXVALIDSTRING(args[1]) ||
        ((numargs == 6) && !RXVALIDSTRING(args[5])) )
      return INVALID_ROUTINE;

//    sprintf(buffer,"SysStemCopy: %s %s %s\n", args[0].strptr, args[1].strptr, args[2].strptr);
//    logmessage(buffer);

    /* remember stem names */
    memset(szFromStemName, 0, sizeof(szFromStemName));
    strcpy(szFromStemName, args[0].strptr);

    if (szFromStemName[args[0].strlength-1] != '.')
        szFromStemName[args[0].strlength] = '.';

    pszFromStemIdx = &(szFromStemName[strlen(szFromStemName)]);

    memset(szToStemName, 0, sizeof(szToStemName));
    strcpy(szToStemName, args[1].strptr);

    if (szToStemName[args[1].strlength-1] != '.')
        szToStemName[args[1].strlength] = '.';

    pszToStemIdx = &(szToStemName[strlen(szToStemName)]);

    /* get from item index */
    if ((numargs >= 3) && RXVALIDSTRING(args[2]))
        if (sscanf(args[2].strptr, "%ld", &ulFrom) != 1)
            return INVALID_ROUTINE;

    /* get to item index */
    if ((numargs >= 4) && RXVALIDSTRING(args[3]))
        if (sscanf(args[3].strptr, "%ld", &ulTo) != 1)
            return INVALID_ROUTINE;

    /* get copy count */
    if ((numargs >= 5) && RXVALIDSTRING(args[4]))
        if (sscanf(args[4].strptr, "%ld", &ulCopyCount) != 1)
            return INVALID_ROUTINE;

    /* get copy type */
    if (numargs >= 6) {
        switch (args[5].strptr[0]) {

        case 'I':
        case 'i':
            fInsert = TRUE;
            break;

        case 'O':
        case 'o':
            fInsert = FALSE;
            break;

        default:
            return INVALID_ROUTINE;
        } /* endswitch */
    } /* endif */

    /* retrieve the number of elements in stems */
    strcpy(pszFromStemIdx, "0");
    shvb.shvnext = NULL;
    shvb.shvname.strptr = szFromStemName;
    shvb.shvname.strlength = strlen((const char *)szFromStemName);
    shvb.shvvalue.strptr = szValue;
    shvb.shvvalue.strlength = sizeof(szValue);
    shvb.shvnamelen = shvb.shvname.strlength;
    shvb.shvvaluelen = shvb.shvvalue.strlength;
    shvb.shvcode = RXSHV_SYFET;
    shvb.shvret = 0;

    if (RexxVariablePool(&shvb) == RXSHV_OK) {
        /* index retrieved fine */
        if (sscanf(shvb.shvvalue.strptr, "%ld", &ulFromCount) != 1)
            return INVALID_ROUTINE;

        if ((ulCopyCount > (ulFromCount - ulFrom + 1)) || (ulFromCount == 0))
            return INVALID_ROUTINE;
    } else fOk = FALSE;

    if (fOk) {
        strcpy(pszToStemIdx, "0");
        shvb.shvnext = NULL;
        shvb.shvname.strptr = szToStemName;
        shvb.shvname.strlength = strlen((const char *)szToStemName);
        shvb.shvvalue.strptr = szValue;
        shvb.shvvalue.strlength = sizeof(szValue);
        shvb.shvnamelen = shvb.shvname.strlength;
        shvb.shvvaluelen = shvb.shvvalue.strlength;
        shvb.shvcode = RXSHV_SYFET;
        shvb.shvret = 0;
        rc = RexxVariablePool(&shvb);

        if (rc == RXSHV_OK) {
            /* index retrieved fine */
            if (sscanf(shvb.shvvalue.strptr, "%ld", &ulToCount) != 1)
                return INVALID_ROUTINE;
        } else {
            if (rc == RXSHV_NEWV) {
                /* tostem.0 is not set, we assume empty target stem */
                ulToCount = 0;
            } else fOk = FALSE;
        } /* endif */

        if (fOk) {
            if (ulTo > ulToCount + 1) return INVALID_ROUTINE;
        } /* endif */
    } /* endif */

    /* set copy count to number of items in source stem if not already set */
    if (ulCopyCount == 0) ulCopyCount = ulFromCount - ulFrom + 1;

    if (fOk && fInsert) {
        /* if we are about to insert the items we have to make room */
        for (ulIdx = ulToCount; ulIdx >= ulTo; ulIdx--) {
            /* get element to relocate */
            sprintf(pszToStemIdx, "%ld", ulIdx);
            shvb.shvnext = NULL;
            shvb.shvname.strptr = szToStemName;
            shvb.shvname.strlength = strlen((const char *)szToStemName);
            shvb.shvvalue.strptr = NULL;
            shvb.shvvalue.strlength = 0;
            shvb.shvnamelen = shvb.shvname.strlength;
            shvb.shvvaluelen = shvb.shvvalue.strlength;
            shvb.shvcode = RXSHV_SYFET;
            shvb.shvret = 0;

            if (RexxVariablePool(&shvb) == RXSHV_OK) {
                sprintf(pszToStemIdx, "%ld", ulIdx + ulCopyCount);
                shvb.shvnext = NULL;
                shvb.shvname.strptr = szToStemName;
                shvb.shvname.strlength = strlen((const char *)szToStemName);
                shvb.shvnamelen = shvb.shvname.strlength;
                shvb.shvvaluelen = shvb.shvvalue.strlength;
                shvb.shvcode = RXSHV_SYSET;
                shvb.shvret = 0;
                rc = RexxVariablePool(&shvb);
                if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV)) fOk = FALSE;

                /* free memory allocated by REXX */
                DosFreeMem(shvb.shvvalue.strptr);
            } else fOk = FALSE;

            if (!fOk) break;

        } /* endfor */

        if (fOk) {
            /* set the new count for the target stem */
            strcpy(pszToStemIdx, "0");
            ulToCount += ulCopyCount;
            sprintf(szValue, "%ld", ulToCount);
            shvb.shvnext = NULL;
            shvb.shvname.strptr = szToStemName;
            shvb.shvname.strlength = strlen((const char *)szToStemName);
            shvb.shvvalue.strptr = szValue;
            shvb.shvvalue.strlength = strlen(szValue);
            shvb.shvnamelen = shvb.shvname.strlength;
            shvb.shvvaluelen = shvb.shvvalue.strlength;
            shvb.shvcode = RXSHV_SYSET;
            shvb.shvret = 0;
            rc = RexxVariablePool(&shvb);
            if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV)) fOk = FALSE;
        } /* endif */
    } /* endif */

    if (fOk) {
        /* now do the actual copying from the source to target */
        for (ulIdx = 0; ulIdx < ulCopyCount; ulIdx++) {
            /* get element to copy */
            sprintf(pszFromStemIdx, "%ld", ulFrom + ulIdx);
            shvb.shvnext = NULL;
            shvb.shvname.strptr = szFromStemName;
            shvb.shvname.strlength = strlen((const char *)szFromStemName);
            shvb.shvvalue.strptr = NULL;
            shvb.shvvalue.strlength = 0;
            shvb.shvnamelen = shvb.shvname.strlength;
            shvb.shvvaluelen = shvb.shvvalue.strlength;
            shvb.shvcode = RXSHV_SYFET;
            shvb.shvret = 0;

            if (RexxVariablePool(&shvb) == RXSHV_OK) {
                sprintf(pszToStemIdx, "%ld", ulTo + ulIdx);
                shvb.shvnext = NULL;
                shvb.shvname.strptr = szToStemName;
                shvb.shvname.strlength = strlen((const char *)szToStemName);
                shvb.shvnamelen = shvb.shvname.strlength;
                shvb.shvvaluelen = shvb.shvvalue.strlength;
                shvb.shvcode = RXSHV_SYSET;
                shvb.shvret = 0;
                rc = RexxVariablePool(&shvb);

                if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV)) fOk = FALSE;

                /* free memory allocated by REXX */
                DosFreeMem(shvb.shvvalue.strptr);
            } else fOk = FALSE;

            if (!fOk) break;
        } /* endfor */
    } /* endif */

    if (fOk && (ulTo + ulCopyCount - 1 > ulToCount)) {
        /* set the new count for the target stem */
        strcpy(pszToStemIdx, "0");
        ulToCount = ulTo + ulCopyCount - 1;
        sprintf(szValue, "%ld", ulToCount);
        shvb.shvnext = NULL;
        shvb.shvname.strptr = szToStemName;
        shvb.shvname.strlength = strlen((const char *)szToStemName);
        shvb.shvvalue.strptr = szValue;
        shvb.shvvalue.strlength = strlen(szValue);
        shvb.shvnamelen = shvb.shvname.strlength;
        shvb.shvvaluelen = shvb.shvvalue.strlength;
        shvb.shvcode = RXSHV_SYSET;
        shvb.shvret = 0;
        rc = RexxVariablePool(&shvb);

        if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV)) fOk = FALSE;
    } /* endif */

    if (fOk) RETVAL(0)
    else RETVAL(-1)
}


/*************************************************************************
* Function:  SysStemSort                                                 *
*                                                                        *
* Syntax:    result = SysStemSort(stem, order, type, start, end,         *
*                                 firstcol, lastcol)                     *
*                                                                        *
* Params:    stem - name of stem to sort                                 *
*            order - 'A' or 'D' for sort order                           *
*            type - 'C', 'I', 'N' for comparision type                   *
*            start - first index to sort                                 *
*            end - last index to sort                                    *
*            firstcol - first column to use as sort key                  *
*            lastcol - last column to use as sort key                    *
*                                                                        *
* Return:    0 - sort was successful                                     *
*            -1 - sort failed                                            *
*************************************************************************/
/* Stem and variable manipulation functions for regutil
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is regutil.
 *
 * The Initial Developer of the Original Code is Patrick TJ McPhee.
 * Portions created by Patrick McPhee are Copyright © 1998, 2001
 * Patrick TJ McPhee. All Rights Reserved.
 *
 * Contributors:
 *
 * $Header: C:/ptjm/rexx/regutil/RCS/regstem.c 1.21 2004/06/16 00:15:06 ptjm Rel $
 */

/* return codes. 22 is rc for invalid call */
#define NOMEMORY 5
#define BADARGS 22
#define BADGENERAL 40

#define INITPTR 1000

/* make a null-terminated string from a rexx string */
#define rxstrdup(y, x) do { \
    y = (char *)alloca(RXSTRLEN(x)+1); \
    memcpy(y,RXSTRPTR(x),RXSTRLEN(x)); \
    y[RXSTRLEN(x)] = 0; \
    } while (0)

/* datatype: re-sizeable array of characters */
struct caalloc {
   struct caalloc * N;
   int alloc, used;
   char data[sizeof(int)];
};

typedef struct {
    int count;
    int ptr_alloc;
    PRXSTRING array;
    struct caalloc * chars;
} chararray;

// stemsort helpers from regutil 1.26
static int rxstrcasecmp(const PRXSTRING l, const PRXSTRING r);
static int rxstrcasecmpbackwards(const PRXSTRING l, const PRXSTRING r);
static int rxstrcmp(const PRXSTRING l, const PRXSTRING r);
static int rxstrcmpbackwards(const PRXSTRING l, const PRXSTRING r);
static int casecmp(const unsigned char * l, const unsigned char * r, const int len);
static int getstemtail(PRXSTRING varname, const int ind, chararray * const values);
static int setastem(PRXSTRING varname, const chararray * const values);
static int cha_addstr(chararray *ca, const char * str, int len);
static chararray * new_chararray(void);
static void delete_chararray(chararray * );


unsigned long SysStemSort(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    char          stemName[255];
    size_t        first     = 1;
    size_t        last      = ULONG_MAX;
    size_t        firstCol  = 0;
    size_t        lastCol   = ULONG_MAX;
    int           sortType  = SORT_CASESENSITIVE;
    int           sortOrder = SORT_ASCENDING;

    // regutil varables
    chararray * ca;
    int start       = 0;
    int count       = 0;
    int width       = 0;
    register int i;
    int (*cmpfn)(const PRXSTRING l, const PRXSTRING r);

//    char buffer[256];

    // quick check valid number of args
    if ( (numargs < 1) || (numargs > 7) || !RXVALIDSTRING(args[0]))
        return INVALID_ROUTINE;

//    sprintf(buffer,"SysStemSort: %s %s %s\n", args[0].strptr, args[1].strptr, args[2].strptr);
//    logmessage(buffer);

    /* remember stem name */
    memset(stemName, 0, sizeof(stemName));
    strcpy(stemName, args[0].strptr);

    // check for trailing '.' and add if not provided
    if (stemName[args[0].strlength-1] != '.') stemName[args[0].strlength] = '.';

    /* check other parameters */

    // [ORDER option] Decode option of order as Ascending or Decending
    if ((numargs >= 2) && RXVALIDSTRING(args[1])) {  /* sort order  */

        switch (args[1].strptr[0]) {

        case 'A':
        case 'a':
            sortOrder = SORT_ASCENDING;
            break;

        case 'D':
        case 'd':
            sortOrder = SORT_DECENDING;
            break;

        default:
            return INVALID_ROUTINE;
        } /* endswitch */
    } /* endif */

    // [TYPE option] Decode option as Case sensitive or insensitive
    if ((numargs >= 3) && RXVALIDSTRING(args[2])) {  /* sort type */

        switch (args[2].strptr[0]) {

        case 'C':
        case 'c':
            sortType = SORT_CASESENSITIVE;
            break;

        case 'I':
        case 'i':
            sortType = SORT_CASEIGNORE;
            break;

        default:
            return INVALID_ROUTINE;
        } /* endswitch */
    } /* endif */

    // scan the [START option] into size_t first
    if ((numargs >= 4) && RXVALIDSTRING(args[3])) { /* first element to sort */
        if (sscanf(args[3].strptr, "%ld", &first) != 1) return INVALID_ROUTINE;
        if (first == 0) return INVALID_ROUTINE; // can't use 0 index
    } /* endif */

    // scan the [END option] into size_t last
    if ((numargs >= 5) && RXVALIDSTRING(args[4])) { /* last element to sort */
        if (sscanf(args[4].strptr, "%ld", &last) != 1) return INVALID_ROUTINE;
        if (last < first) return INVALID_ROUTINE;
    } /* endif */

    // scan the [FIRSTCOL option] into size_t firstCol then subtract 1
    if ((numargs >= 6) && RXVALIDSTRING(args[5])) { /* first column to sort */
        if (sscanf(args[5].strptr, "%ld", &firstCol) != 1) return INVALID_ROUTINE;
        firstCol--;
    } /* endif */

    // scan the [LASTCOL option] into size_t lastCol then subtract 1
    if ((numargs == 7) && RXVALIDSTRING(args[6])) { /* last column to sort */
        if (sscanf(args[6].strptr, "%ld", &lastCol) != 1) return INVALID_ROUTINE;
        lastCol--;
        if (lastCol < firstCol) RETVAL(-7) //return INVALID_ROUTINE;
    } /* endif */


    // *** here is where the stemsort OREXX sources moves into regutil source
    start = (first - 1); //TMP
    if(start < 0) return INVALID_ROUTINE;

    count = last - first;
    if (count < 0) count = 0;  //RETVAL(-9) //return INVALID_ROUTINE;

    width = lastCol - firstCol;

    ca = new_chararray();
    if (!ca) return NOMEMORY;

    getstemtail(args, 1, ca);

    if (!count) {
        count = ca->count - start;
        if (count < 0) count = 0;
    }

    if (sortType && sortOrder) cmpfn = rxstrcasecmpbackwards;
    else if (sortType) cmpfn = rxstrcasecmp;
    else if (sortOrder) cmpfn = rxstrcmpbackwards;
    else cmpfn = rxstrcmp;

    if (!firstCol && !width)
      qsort(ca->array+start, count, sizeof(*ca->array), (int(*)(const void *, const void *))cmpfn);
    else {

        struct {
            RXSTRING se;
            RXSTRING orig;
        } *aa;

        aa = malloc(sizeof(*aa) * count);



        if (!aa) {
            delete_chararray(ca);
            return NOMEMORY;
        }

        for (i = 0; i < count; i++) {
            aa[i].orig = ca->array[i+start];
            aa[i].se.strptr = aa[i].orig.strptr + firstCol;
            aa[i].se.strlength = aa[i].orig.strlength - firstCol;

            if (width && (int)aa[i].se.strlength > width) aa[i].se.strlength = width;

            else if ((int)aa[i].se.strlength < 0) aa[i].se.strlength = 0;
      }

      qsort(aa, count, sizeof(*aa), (int(*)(const void *, const void *))cmpfn);

      for (i = 0; i < count; i++) {
         ca->array[i+start] = aa[i].orig;
      }

      free(aa);
   }

    setastem(args, ca);
    delete_chararray(ca);

    RETVAL(0)
}


static int rxstrcasecmpbackwards(const PRXSTRING l, const PRXSTRING r)
{
    return rxstrcasecmp(r, l);
}


static int rxstrcmpbackwards(const PRXSTRING l, const PRXSTRING r)
{
    return rxstrcmp(r, l);
}


/* compare two strings case-sensitively */
static int rxstrcmp(const PRXSTRING l, const PRXSTRING r)
{
    register int len = min(l->strlength, r->strlength),
                           d = l->strlength - r->strlength,
                           c = memcmp(l->strptr, r->strptr, len);

    return c ? c : d;
}


/* compare two strings case-insensitively */
static int rxstrcasecmp(const PRXSTRING l, const PRXSTRING r)
{
    register int len = min(l->strlength, r->strlength),
                           d = l->strlength - r->strlength,
                           c = casecmp(l->strptr, r->strptr, len);

    return (len && c) ? c : d;
}


/* case-insensitive comparison of two regions of memory */
static int casecmp(const unsigned char * l, const unsigned char * r, const int len)
{
    register int i;
    register int c;

    for (i = c = 0; !c && i < len; i++) c = toupper(l[i]) - toupper(r[i]);

    return c;
}


/* given a stem name, retrieve the values associated with that stem, starting
 * at index ind (1-based) and stick them in a chararray. I'm doing this by
 * retrieving one value at a time and adding it to the array using cha_addstr. */
static int getstemtail(PRXSTRING varname, const int ind, chararray * const values)
{
    SHVBLOCK stem;
    SHVBLOCK *vars;

    register int i;
    register int count;
    register int vallen;

    int namelen = varname->strlength + 12;

    char * origname;
    char valbuf[11];
    char *nameptr;
    char *valptr = NULL;

    if (!values) return -1;

    values->count = 0;

    if (varname->strptr[varname->strlength-1] == '.') {
        rxstrdup(origname, *varname);
    } else {
        origname = alloca(varname->strlength+2);
        memcpy(origname, varname->strptr, varname->strlength);
        origname[varname->strlength] = '.';
        origname[varname->strlength+1] = 0;
    }
    strupr(origname);

    /* first, find out how many there are */
    nameptr = alloca(namelen);
    memset(&stem, 0, sizeof(stem));

    /* set up the .0 member */
    stem.shvname.strptr = nameptr;
    stem.shvname.strlength = sprintf(nameptr, "%s%d", origname, 0);
    stem.shvvalue.strptr = valbuf;
    stem.shvvalue.strlength = stem.shvvaluelen = sizeof(valbuf);
    stem.shvcode = RXSHV_SYFET;

    RexxVariablePool(&stem);

    /* there is no stem, so there is nothing to return */
    if (stem.shvret & RXSHV_NEWV) return 0;

    valbuf[stem.shvvalue.strlength] = 0;
    count = atoi(valbuf) - ind + 1;

    /* likewise, if there is nothing to return, there is nothing
     * to return */
    if (count <= 0) return 0;

    vallen = 500;
    vars = malloc(count*(sizeof(*vars)+namelen+vallen));

    if (!vars) {
        stem.shvname.strptr = nameptr;
        stem.shvcode = RXSHV_SYFET;
        vallen = 10000;
        valptr = malloc(vallen);
        stem.shvvalue.strptr = valptr;

        for (i = 0; i < count; i++) {
            stem.shvname.strlength = sprintf(nameptr, "%s%d", origname, i+ind);
            stem.shvvaluelen = stem.shvvalue.strlength = vallen;
            RexxVariablePool(&stem);
            if (stem.shvret & RXSHV_TRUNC) {
                vallen = stem.shvvaluelen;
                valptr = realloc(valptr, vallen);
                stem.shvret = 0;
                stem.shvvalue.strptr = valptr;
                i--;
            } else {
                cha_addstr(values, stem.shvvalue.strptr, stem.shvvalue.strlength);
            }
        }
    } else {
        valptr = (char *)(vars+count) + namelen*count;
        for (i = 0; i < count; i++) {
            nameptr = (char *)(vars+count) + namelen*i;
            vars[i].shvname.strptr = nameptr;
            vars[i].shvcode = RXSHV_SYFET;
            vars[i].shvvalue.strptr = valptr+i*vallen;
            vars[i].shvvaluelen = vallen;
            vars[i].shvret = 0;
            vars[i].shvname.strlength = sprintf(nameptr, "%s%d", origname, i+ind);
            vars[i].shvvaluelen = vars[i].shvvalue.strlength = vallen;
            vars[i].shvnext = vars+i+1;
        } vars[i-1].shvnext = NULL;
        valptr = NULL;

        RexxVariablePool(vars);

        /* add strings to array. If any string is truncated, knock up the
         * buffer and try it again */
        for (i = 0; i < count; i++) {
            if (vars[i].shvret & RXSHV_TRUNC) {
                vars[i].shvnext = NULL;

                /* re-allocate only if we haven't already made the buffer big
                 * enough */
                if (vallen < vars[i].shvvaluelen) {
                    vallen = vars[i].shvvaluelen;
                    valptr = realloc(valptr, vallen);
                }

                vars[i].shvvalue.strptr = valptr;
                vars[i].shvvaluelen = vallen;
                RexxVariablePool(vars+i);
            }

            cha_addstr(values, vars[i].shvvalue.strptr, vars[i].shvvalue.strlength);
        }
        free(vars);
    }

    if (valptr) free(valptr);

    return 0;
}


/* given an array of strings and the name of a stem variable, set the stem
 * variable to match the strings */
static int setastem(PRXSTRING varname, const chararray * const values)
{
    SHVBLOCK *astem;
    SHVBLOCK stem0[2];
    register int i;
    int namelen = varname->strlength + 12, orignamelen, count;
    char *namebuf;
    char *origname;
    char valbuf[11];
    char *nameptr;

    namebuf = malloc(namelen * (values->count+1));

    if (values && values->count) {
        count = values->count;
        astem = malloc(values->count*sizeof(*astem));
        memset(astem, 0, values->count * sizeof(*astem));
    } else {
       astem = NULL;
       count = 0;
    }

    memset(stem0, 0, sizeof(stem0));

    if (varname->strptr[varname->strlength-1] == '.') {
       rxstrdup(origname, *varname);
       orignamelen = varname->strlength;
    } else {
       origname = alloca(varname->strlength+2);
       memcpy(origname, varname->strptr, varname->strlength);
       origname[varname->strlength] = '.';
       origname[varname->strlength+1] = 0;
       orignamelen = varname->strlength+1;
    }
    strupr(origname);
    nameptr = namebuf;

    /* drop the stem */
    stem0[0].shvnext = stem0+1;
    stem0[0].shvname.strptr = origname;
    stem0[0].shvname.strlength = orignamelen;
    stem0[0].shvcode = RXSHV_DROPV;

    /* set up the .0 member */
    stem0[1].shvnext = astem;
    stem0[1].shvname.strptr = nameptr;
    stem0[1].shvname.strlength = sprintf(nameptr, "%s%d", origname, 0);
    stem0[1].shvvalue.strptr = valbuf;
    stem0[1].shvvalue.strlength = sprintf(valbuf, "%d", count);
    stem0[1].shvcode = RXSHV_SYSET;

    nameptr += namelen;

    for (i = 0; i < count; i++) {
        astem[i].shvnext = astem+i+1;
        astem[i].shvname.strptr = nameptr;
        astem[i].shvname.strlength = sprintf(nameptr, "%s%d", origname, i+1);
        astem[i].shvvalue = values->array[i];
        astem[i].shvcode = RXSHV_SYSET;
        nameptr += namelen;
    }

    if (i) astem[i-1].shvnext = NULL;

    RexxVariablePool(stem0);
    free(namebuf);
    if (astem) free(astem);

    return 0;
}


static chararray * new_chararray(void)
{
    chararray * ca;

    ca = malloc(sizeof(*ca));

    if (!ca)
        return NULL;

    /* allocate 1000 pointers by default. These point into one big buffer,
     * which we allocate separately from the points -- a chararray has four
     * mallocs, instead of one per pointer (on the other hand, we'll have
     * some big chunks of data to realloc ...) */
    ca->ptr_alloc = INITPTR;
    ca->array = malloc(INITPTR*sizeof(*ca->array));
    ca->chars = malloc(sizeof(*ca->chars) + INITPTR*_MAX_PATH);

    if (!ca->chars || !ca->array) {
        if (ca->chars) free(ca->chars);
        if (ca->array) free(ca->array);
        free(ca);
        ca = NULL;
    }

    ca->chars->alloc = INITPTR*_MAX_PATH;
    ca->chars->N = NULL;
    ca->count = 0;
    ca->chars->used = 0;

    return ca;
}


static void free_caalloc(struct caalloc * nd)
{
   if (!nd) return;
   free_caalloc(nd->N);
   free(nd);
}


static void delete_chararray(chararray * ca)
{
    if (!ca) return;

    if (ca->array) free(ca->array);
    free_caalloc(ca->chars);
    free(ca);
}


static int cha_addstr(chararray *ca, const char * str, int len)
{
    if (ca->count >= ca->ptr_alloc) {
        ca->ptr_alloc += INITPTR;
        ca->array = realloc(ca->array, ca->ptr_alloc*sizeof(*ca->array));
        if (ca->array == NULL) {
            ca->ptr_alloc = 0;
            ca->count = 0;
            return -1;
         }
    }

    /* there's not enough room in this buffer, so allocate a new one which
     * is big enough */
    if ((ca->chars->used+len+1) >= ca->chars->alloc) {
        struct caalloc * nd, *cd, *pd;

        /* keep the list of buffers ordered by free space -- this prevents some
         * needless memory allocation at the cost of a few cycles. This doesn't
         * seem to buy much, but reallocation is generally rare in any case. */
        for (cd = ca->chars, nd = cd->N, pd = NULL;
             nd && (nd->alloc - nd->used) > (cd->alloc - cd->used);
             pd = nd, nd = nd->N)
           ;

        if (pd) {
           ca->chars = cd->N;
           cd->N = pd->N;
           pd->N = cd;
        }

        /* maybe we now have enough space, so test again */
        if ((ca->chars->used+len+1) >= ca->chars->alloc) {
           nd = malloc(sizeof(*nd)+len+INITPTR*_MAX_PATH);

           if (!nd) {
              return -1;
           }

           nd->alloc = len+INITPTR*_MAX_PATH;
           nd->used = 0;
           nd->N = ca->chars;
           ca->chars = nd;
        }
     }


    ca->array[ca->count].strlength = len;
    ca->array[ca->count].strptr = ca->chars->data+ca->chars->used;
    memcpy(ca->array[ca->count++].strptr, str, len);
    ca->chars->used += len+1;
    ca->chars->data[ca->chars->used] = 0;

    return 0;
}


