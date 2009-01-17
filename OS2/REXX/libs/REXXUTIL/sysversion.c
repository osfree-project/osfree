/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  Contains helper functions functions:
*       SysOS2Ver
*       SysVersion
*       SysUtilVersion
*
*  Michael Greene, December 2007
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

#define INCL_DOSMISC

#include <os2.h>
#include <stdio.h>
#include <string.h>

#include <rexxdefs.h>  // rexxsaa.h include in this header

RexxFunctionHandler SysOS2Ver;
RexxFunctionHandler SysVersion;
RexxFunctionHandler SysUtilVersion;


/*************************************************************************
* Function:  SysOS2Ver                                                   *
*                                                                        *
* Syntax:    call SysOS2Ver                                              *
*                                                                        *
* Return:    eCS-OS/2 Version                                            *
*************************************************************************/

unsigned long SysOS2Ver(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long Versions[2];                  /* Major version number       */

    if (numargs != 0) return INVALID_ROUTINE;

    DosQuerySysInfo(QSV_VERSION_MAJOR, QSV_VERSION_MINOR,
                        Versions, sizeof(Versions));

    retstr->strlength = sprintf(retstr->strptr,
                                "OS/2 %lu.%02lu",
                                Versions[0]/10, Versions[1]);
    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysVersion                                                  *
*                                                                        *
* Syntax:    Say  SysVersion                                             *
*                                                                        *
* Return:    Operating System and Version                                *
*************************************************************************/

unsigned long SysVersion(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    /* this is only an alias for SysOS2Ver */
    return SysOS2Ver(name, numargs, args, queuename, retstr);
}


/*************************************************************************
* Function:  SysUtilVersion                                              *
*                                                                        *
* Syntax:    Say  SysUtilVersion                                         *
*                                                                        *
* Return:    RexxUtil Library Version                                    *
*************************************************************************/

unsigned long SysUtilVersion(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    if (numargs != 0) return INVALID_ROUTINE;

    retstr->strlength = sprintf(retstr->strptr, "%d.%d",
                                    LIBVERSION_MAJOR, LIBVERSION_MINOR);

    return VALID_ROUTINE;
}

