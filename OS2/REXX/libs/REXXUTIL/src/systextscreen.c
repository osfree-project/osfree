/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  Contains helper functions functions:
*       SysTextScreenSize
*       SysTextScreenRead
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

#define INCL_DOSMEMMGR
#define INCL_VIO

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rexxdefs.h>  // rexxsaa.h include in this header

RexxFunctionHandler SysTextScreenSize;
RexxFunctionHandler SysTextScreenRead;


/*************************************************************************
* Function:  SysTextScreenSize                                           *
*                                                                        *
* Syntax:    call SysTextScreenSize                                      *
*                                                                        *
* Return:    Size of screen in row and columns returned as:  row, col    *
*************************************************************************/

unsigned long SysTextScreenSize(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    VIOMODEINFO  ModeData;

    if (numargs != 0)  return INVALID_ROUTINE;

    ModeData.cb = sizeof(ModeData);
    VioGetMode(&ModeData, (HVIO) 0);

    retstr->strlength = sprintf(retstr->strptr, "%d %d",
                                (int)ModeData.row,(int)ModeData.col);

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysTextScreenRead                                           *
*                                                                        *
* Syntax:    call SysTextScreenRead row, col [,len]                      *
*                                                                        *
* Params:    row - Horizontal row on the screen to start reading from.   *
*                   The row at the top of the screen is 0.               *
*            col - Vertical column on the screen to start reading from.  *
*                   The column at the left of the screen is 0.           *
*            len - The number of characters to read.  The default is the *
*                   rest of the screen.                                  *
*                                                                        *
* Return:    Characters read from text screen.                           *
*************************************************************************/

unsigned long SysTextScreenRead(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    long  row;
    long  col;
    long  len = 8160;
    char  temp[8160];

    if (numargs < 2 || numargs > 3 || !RXVALIDSTRING(args[0]) ||
        !RXVALIDSTRING(args[1]) || !string2long(args[0].strptr, &row) ||
        row < 0 || !string2long(args[1].strptr, &col) || col < 0)
            return INVALID_ROUTINE;

    if (numargs == 3) {
        if (!RXVALIDSTRING(args[2]) || !string2long(args[2].strptr, &len) ||
            len < 0) return INVALID_ROUTINE;
    }

    VioReadCharStr( temp, (PUSHORT)&len, row, col, (HVIO) 0);

    if (len > retstr->strlength)
        if (DosAllocMem((PPVOID)&retstr->strptr, len, AllocFlag)) RETVAL(3)

    memcpy(retstr->strptr, temp, len);
    retstr->strlength = len;

    return VALID_ROUTINE;
}


