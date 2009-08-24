/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  Contains helper functions functions:
*       SysLoadFuncs
*       SysLoadFuncs
*
*  Michael Greene, December 2007
*
*  17 Jan 2008 MKG:
*   Had to change returns of SysLoadFuncs and SysDropFuncs to match old
*   DLL to keep old stuff working
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

#include <stdlib.h>
#include <string.h>

#include <rexxdefs.h>  // rexxsaa.h include in this header
#include <rexxfuncs.h>

RexxFunctionHandler SysLoadFuncs;
RexxFunctionHandler SysDropFuncs;


/*************************************************************************
* Function:  SysLoadFuncs                                                *
*                                                                        *
* Syntax:    call SysLoadFuncs [option]                                  *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    null string                                                 *
*************************************************************************/

unsigned long SysLoadFuncs(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    int i;
    int FuncCnt;    /* Num of functions  */

    unsigned long rc;

    if (numargs > 0) return INVALID_ROUTINE;

    FuncCnt = sizeof(RxFncTable)/sizeof(char *);

    for (i = 1; i < FuncCnt; i++)
        rc = RexxRegisterFunctionDll(RxFncTable[i],
                                     RxFncTable[0],
                                     RxFncTable[i]);

    // I am sure it would be nice to have a real return value here but
    // the old dll returns a NULL string. I found that any new ideas break
    // existing stuff -- ie eCSMT
//    if(rc != RXFUNC_OK || rc != RXFUNC_DEFINED) RETVAL(1)

    retstr->strlength = 0;
    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysLoadFuncs                                                *
*                                                                        *
* Syntax:    call SysLoadFuncs [option]                                  *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    null string                                                 *
*************************************************************************/

unsigned long SysDropFuncs(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    int i;
    int FuncCnt;       /* Num of functions  */

    if (numargs > 0) return INVALID_ROUTINE;

    FuncCnt = sizeof(RxFncTable)/sizeof(char *);

    for (i = 1; i < FuncCnt; i++) {
        RexxDeregisterFunction(RxFncTable[i]);
    }

    return VALID_ROUTINE;
}


