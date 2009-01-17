/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  Contains the following functions:
*      SysCreateMutexSem
*      SysOpenMutexSem
*      SysReleaseMutexSem
*      SysCloseMutexSem
*      SysRequestMutexSem
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

#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS

#include <os2.h>
#include <stdio.h>
#include <string.h>

#include <rexxdefs.h>  // rexxsaa.h include in this header

RexxFunctionHandler SysCreateMutexSem;
RexxFunctionHandler SysOpenMutexSem;
RexxFunctionHandler SysReleaseMutexSem;
RexxFunctionHandler SysCloseMutexSem;
RexxFunctionHandler SysRequestMutexSem;


/*************************************************************************
* Function:  SysCreateMutexSem                                           *
*                                                                        *
* Syntax:    handle = SysCreateMutexSem(<name>)                          *
*                                                                        *
* Params:    name  - optional name for a mutex semaphore                 *
*                                                                        *
* Return:    handle - token used as a mutex handle for                   *
*                     SysRequestMutexSem, SysReleaseMutexSem,            *
*                     SysCloseMutexSem, and SysOpenEventSem              *
*            '' - Empty string in case of any error                      *
*************************************************************************/

unsigned long SysCreateMutexSem(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;

    HMTX handle = NULL;           /* mutex handle */

    if (numargs > 1) return INVALID_ROUTINE;

    /* request for named sem      */
    if (numargs == 1) {

        rc = DosCreateMutexSem(args[0].strptr, &handle, DC_SEM_SHARED, 1);

        /* may already be created try to open it */
        if (rc != NO_ERROR) {
            rc = DosOpenMutexSem(args[0].strptr, &handle);
        }

    } else {                                /* unnamed semaphore */
        rc = DosCreateMutexSem(NULL, &handle, DC_SEM_SHARED, 1);
    }

    if (!handle && rc != NO_ERROR) retstr->strlength = 0;  /* return null string */
    else {                                       /* format the result  */
        retstr->strlength = sprintf(retstr->strptr, "%lu", handle);
    }

    return VALID_ROUTINE;                /* good completion            */
}


/*************************************************************************
* Function:  SysOpenMutexSem                                             *
*                                                                        *
* Syntax:    result = SysOpenMutexSem(name)                              *
*                                                                        *
* Params:    name - name of the mutex semaphore                          *
*                                                                        *
* Return:    result - handle to the mutex                                *
*************************************************************************/

unsigned long SysOpenMutexSem(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;

    HMTX handle = NULL;           /* mutex handle */

    if (numargs != 1) return INVALID_ROUTINE; /* Only one argument accepted */

    if (!string2ulong(args[0].strptr, &handle)) return INVALID_ROUTINE;

                                       /* get a binary handle        */
                                       /* try to open it             */
    rc = DosOpenMutexSem(NULL, &handle);

    retstr->strlength = sprintf(retstr->strptr, "%lu", rc);

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysReleaseMutexSem                                          *
*                                                                        *
* Syntax:    result = SysReleaseMutexSem(handle)                         *
*                                                                        *
* Params:    handle - token returned from SysCreateMutexSem              *
*                                                                        *
* Return:    result - return code from ReleaseMutex                      *
*************************************************************************/

unsigned long SysReleaseMutexSem(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;

    HMTX handle = NULL;           /* mutex handle */

    if (numargs != 1) return INVALID_ROUTINE; /* Only one argument accepted */

    if (!string2ulong(args[0].strptr, &handle)) return INVALID_ROUTINE;

    rc = DosReleaseMutexSem(handle);

    retstr->strlength = sprintf(retstr->strptr, "%lu", rc);

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysCloseMutexSem                                            *
*                                                                        *
* Syntax:    result = SysCloseMutexSem(handle)                           *
*                                                                        *
* Params:    handle - token returned from SysCreateMutexSem              *
*                                                                        *
* Return:    result - return code from CloseHandle                       *
*************************************************************************/

unsigned long SysCloseMutexSem(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;

    HMTX handle = NULL;           /* mutex handle */

    if (numargs != 1) return INVALID_ROUTINE; /* Only one argument accepted */

    if (!string2ulong(args[0].strptr, &handle)) return INVALID_ROUTINE;

    rc = DosCloseMutexSem(handle);

    retstr->strlength = sprintf(retstr->strptr, "%lu", rc);

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysRequestMutexSem                                          *
*                                                                        *
* Syntax:    result = SysRequestMutexSem(handle, <timeout>)              *
*                                                                        *
* Params:    handle - token returned from SysCreateMutexSem              *
*                                                                        *
* Return:    result - return code from WaitForSingleObject               *
*************************************************************************/

unsigned long SysRequestMutexSem(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;
    unsigned long timeout = SEM_INDEFINITE_WAIT; /* timeout value default */

    HMTX handle = NULL;           /* mutex handle */


    if (numargs < 1 || numargs > 2 || !RXVALIDSTRING(args[0]))
        return INVALID_ROUTINE;

    if (numargs == 2) {
        if (!string2ulong(args[1].strptr, &timeout)) return INVALID_ROUTINE;
    }

    if (!string2ulong(args[0].strptr, &handle)) return INVALID_ROUTINE;

    rc = DosRequestMutexSem(handle, timeout);

    retstr->strlength = sprintf(retstr->strptr, "%lu", rc);

    return VALID_ROUTINE;
}

