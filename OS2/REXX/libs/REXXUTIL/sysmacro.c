/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  Contains the following functions:
*      SysAddRexxMacro
*      SysReorderRexxMacro
*      SysDropRexxMacro
*      SysQueryRexxMacro
*      SysClearRexxMacroSpace
*      SysSaveRexxMacroSpace
*      SysLoadRexxMacroSpace
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <rexxdefs.h>  // rexxsaa.h include in this header

RexxFunctionHandler SysAddRexxMacro;
RexxFunctionHandler SysReorderRexxMacro;
RexxFunctionHandler SysDropRexxMacro;
RexxFunctionHandler SysQueryRexxMacro;
RexxFunctionHandler SysClearRexxMacroSpace;
RexxFunctionHandler SysSaveRexxMacroSpace;
RexxFunctionHandler SysLoadRexxMacroSpace;


/*************************************************************************
* Function:  SysAddRexxMacro                                             *
*                                                                        *
* Syntax:    result = SysAddRexxMacro(name, file, <order>)               *
*                                                                        *
* Params:    name   - loaded name of the macro file                      *
*            file   - file containing the loaded macro                   *
*            order  - Either 'B'efore or 'A'fter.  The default is 'B'    *
*                                                                        *
* Return:    return code from RexxAddMacro                               *
*************************************************************************/

unsigned long SysAddRexxMacro(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long position;              /* added position             */

    if (numargs < 2 || numargs > 3 ||    /* wrong number?              */
        !RXVALIDSTRING(args[0]) ||       /* first is omitted           */
        !RXVALIDSTRING(args[1]))         /* second is omitted          */
      return INVALID_ROUTINE;            /* raise error condition      */

    position = RXMACRO_SEARCH_BEFORE;    /* set default search position*/

    if (numargs == 3) {                  /* have an option?            */
        if (RXZEROLENSTRING(args[2])) return INVALID_ROUTINE;
        else if (toupper(args[2].strptr[0]) == 'B') position = RXMACRO_SEARCH_BEFORE;
        else if (toupper(args[2].strptr[0]) == 'A') position = RXMACRO_SEARCH_AFTER;
        else return INVALID_ROUTINE;   /* raise an error             */
    }
    /* try to add the macro       */

    RETVAL(RexxAddMacro(args[0].strptr, args[1].strptr, position))
}

/*************************************************************************
* Function:  SysReorderRexxMacro                                         *
*                                                                        *
* Syntax:    result = SysReorderRexxMacro(name, order)                   *
*                                                                        *
* Params:    name   - loaded name of the macro file                      *
*            order  - Either 'B'efore or 'A'fter.                        *
*                                                                        *
* Return:    return code from RexxReorderMacro                           *
*************************************************************************/

unsigned long SysReorderRexxMacro(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long position;                /* added position             */

    if (numargs != 2 || !RXVALIDSTRING(args[0]) || RXZEROLENSTRING(args[1]))
        return INVALID_ROUTINE;            /* raise error condition      */

    /* 'B'efore?                  */
    if (toupper(args[1].strptr[0]) == 'B') position = RXMACRO_SEARCH_BEFORE;
    else if (toupper(args[1].strptr[0]) == 'A') position = RXMACRO_SEARCH_AFTER;
    else return INVALID_ROUTINE;

    /* try to add the macro       */
    RETVAL(RexxReorderMacro(args[0].strptr, position))
}

/*************************************************************************
* Function:  SysDropRexxMacro                                            *
*                                                                        *
* Syntax:    result = SysDropRexxMacro(name)                             *
*                                                                        *
* Params:    name   - name of the macro space function                   *
*                                                                        *
* Return:    return code from RexxDropMacro                              *
*************************************************************************/

unsigned long SysDropRexxMacro(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    if (numargs != 1) return INVALID_ROUTINE;

    RETVAL(RexxDropMacro(args[0].strptr))  /* try to drop the macro   */
}

/*************************************************************************
* Function:  SysQueryRexxMacro                                           *
*                                                                        *
* Syntax:    result = SysQueryRexxMacro(name)                            *
*                                                                        *
* Params:    name   - name of the macro space function                   *
*                                                                        *
* Return:    position of the macro ('B' or 'A'), returns null for errors.*
*************************************************************************/

unsigned long SysQueryRexxMacro(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned short position;    /* returned position */

    if (numargs != 1) return INVALID_ROUTINE;

    /* query the macro position   */
    if (RexxQueryMacro(args[0].strptr, &position)) retstr->strlength = 0;
    else {
        /* return a 'B'               */
        if (position == RXMACRO_SEARCH_BEFORE) retstr->strptr[0] = 'B';
        else retstr->strptr[0] = 'A'; /* must be 'A'after  */

        retstr->strlength = 1; /* returning one character    */
    }

    return VALID_ROUTINE;      /* good completion            */
}

/*************************************************************************
* Function:  SysClearRexxMacroSpace                                      *
*                                                                        *
* Syntax:    result = SysClearRexxMacroSpace()                           *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    return code from RexxClearMacroSpace()                      *
*************************************************************************/

unsigned long SysClearRexxMacroSpace(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    if (numargs) return INVALID_ROUTINE;

    /* clear the macro space      */
    RETVAL(RexxClearMacroSpace())
}

/*************************************************************************
* Function:  SysSaveRexxMacroSpace                                       *
*                                                                        *
* Syntax:    result = SysSaveRexxMacroSpace(file)                        *
*                                                                        *
* Params:    file   - name of the saved macro space file                 *
*                                                                        *
* Return:    return code from RexxSaveMacroSpace()                       *
*************************************************************************/

unsigned long SysSaveRexxMacroSpace(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    if (numargs != 1) return INVALID_ROUTINE;

    /* save the macro space      */
    RETVAL(RexxSaveMacroSpace(0, NULL, args[0].strptr))
}

/*************************************************************************
* Function:  SysLoadRexxMacroSpace                                       *
*                                                                        *
* Syntax:    result = SysLoadRexxMacroSpace(file)                        *
*                                                                        *
* Params:    file   - name of the saved macro space file                 *
*                                                                        *
* Return:    return code from RexxLoadMacroSpace()                       *
*************************************************************************/

unsigned long SysLoadRexxMacroSpace(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    if (numargs != 1) return INVALID_ROUTINE; /* raise error condition */

    /* load the macro space      */
    RETVAL(RexxLoadMacroSpace(0, NULL, args[0].strptr))
}

