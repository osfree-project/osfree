/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  Contains the following functions:
*      SysCreateEventSem
*      SysOpenEventSem
*      SysCloseEventSem
*      SysResetEventSem
*      SysPostEventSem
*      SysWaitEventSem
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
#include <stdlib.h>
#include <string.h>

#include <rexxdefs.h>  // rexxsaa.h include in this header

RexxFunctionHandler SysCreateEventSem;
RexxFunctionHandler SysOpenEventSem;
RexxFunctionHandler SysCloseEventSem;
RexxFunctionHandler SysResetEventSem;
RexxFunctionHandler SysPostEventSem;
RexxFunctionHandler SysWaitEventSem;


/*************************************************************************
* Function:  SysCreateEventSem                                           *
*                                                                        *
* Syntax:    handle = SysCreateEventSem(<name>)                          *
*                                                                        *
* Params:    name  - optional name for a event semaphore                 *
*                                                                        *
* Return:    handle - token used as a event sem handle for               *
*                     SysPostEventSem, SysClearEventSem,                 *
*                     SysCloseEventSem, and SysOpenEventSem              *
*            '' - Empty string in case of any error                      *
*************************************************************************/

unsigned long SysCreateEventSem(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;

    HEV handle = NULL;           /* mutex handle */

    if (numargs > 1) return INVALID_ROUTINE;

    if (numargs == 1) {

        rc = DosCreateEventSem(args[0].strptr, &handle, DC_SEM_SHARED, 1);

        /* may already be created try to open it */
        if (rc != NO_ERROR) {
            rc = DosOpenEventSem(args[0].strptr, &handle);
        }

    } else {                                /* unnamed semaphore */
        rc = DosCreateEventSem(NULL, &handle, DC_SEM_SHARED, 1);
    }

    if (!handle && rc != NO_ERROR) retstr->strlength = 0;  /* return null string */
    else {                                       /* format the result  */
        retstr->strlength = sprintf(retstr->strptr, "%lu", handle);
    }

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysOpenEventSem                                             *
*                                                                        *
* Syntax:    result = SysOpenEventSem(name)                              *
*                                                                        *
* Params:    name - name of the event semaphore                          *
*                                                                        *
* Return:    result - return code from OpenEvent                         *
*************************************************************************/

unsigned long SysOpenEventSem(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;

    HEV handle = NULL;           /* mutex handle */

    if (numargs != 1) return INVALID_ROUTINE; /* Only one argument accepted */

    if (!string2ulong(args[0].strptr, &handle)) return INVALID_ROUTINE;

                                       /* get a binary handle        */
                                       /* try to open it             */
    rc = DosOpenEventSem(NULL, &handle);

    RETVAL(rc)
}


/*************************************************************************
* Function:  SysCloseEventSem                                            *
*                                                                        *
* Syntax:    result = SysCloseEventSem(handle)                           *
*                                                                        *
* Params:    handle - token returned from SysCreateEventSem              *
*                                                                        *
* Return:    result - return code from CloseHandle                       *
*************************************************************************/

unsigned long SysCloseEventSem(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;

    HEV handle = NULL;           /* mutex handle */

    if (numargs != 1) return INVALID_ROUTINE; /* Only one argument accepted */

    if (!string2ulong(args[0].strptr, &handle)) return INVALID_ROUTINE;

    rc = DosCloseEventSem(handle);

    RETVAL(rc)
}


/*************************************************************************
* Function:  SysResetEventSem                                            *
*                                                                        *
* Syntax:    result = SysResetEventSem(handle)                           *
*                                                                        *
* Params:    handle - token returned from SysCreateEventSem              *
*                                                                        *
* Return:    result - return code from ResetEvent                        *
*************************************************************************/

unsigned long SysResetEventSem(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;
    unsigned long PostCount = 0;

    HEV handle = NULL;           /* mutex handle */

    if (numargs != 1) return INVALID_ROUTINE; /* Only one argument accepted */

    if (!string2ulong(args[0].strptr, &handle)) return INVALID_ROUTINE;

    rc = DosResetEventSem(handle, &PostCount);

    RETVAL(rc)
}


/*************************************************************************
* Function:  SysPostEventSem                                             *
*                                                                        *
* Syntax:    result = SysPostEventSem(handle)                            *
*                                                                        *
* Params:    handle - token returned from SysCreateEventSem              *
*                                                                        *
* Return:    result - return code from SetEvent                          *
*************************************************************************/

unsigned long SysPostEventSem(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;

    HEV handle = NULL;           /* mutex handle */

    if (numargs != 1) return INVALID_ROUTINE; /* Only one argument accepted */

    if (!string2ulong(args[0].strptr, &handle)) return INVALID_ROUTINE;

    rc = DosPostEventSem(handle);

    RETVAL(rc)
}


/*************************************************************************
* Function:  SysWaitEventSem                                             *
*                                                                        *
* Syntax:    result = SysWaitEventSem(handle, <timeout>)                 *
*                                                                        *
* Params:    handle - token returned from SysWaitEventSem                *
*                                                                        *
* Return:    result - return code from WaitForSingleObject               *
*************************************************************************/

unsigned long SysWaitEventSem(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;
    unsigned long timeout = SEM_INDEFINITE_WAIT; /* timeout value default */

    HEV handle = NULL;           /* mutex handle */

    if (numargs < 1 || numargs > 2 || !RXVALIDSTRING(args[0]))
        return INVALID_ROUTINE;

    if (numargs == 2) {
        if (!string2ulong(args[1].strptr, &timeout)) return INVALID_ROUTINE;
    }

    if (!string2ulong(args[0].strptr, &handle)) return INVALID_ROUTINE;

    rc = DosWaitEventSem(handle, timeout);

    RETVAL(rc)
}

