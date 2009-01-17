/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  Various helpers and defines used by several modules.
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

#include <stdbool.h>
#include <rexxsaa.h>

#define LIBVERSION_MAJOR  2
#define LIBVERSION_MINOR  10

/*********************************************************************/
/* Numeric Return calls                                              */
/*********************************************************************/

#define  INVALID_ROUTINE 40            /* Raise Rexx error           */
#define  VALID_ROUTINE    0            /* Successful completion      */

/*********************************************************************/
/* Numeric Error Return Strings                                      */
/*********************************************************************/

#define  NO_UTIL_ERROR    "0"          /* No error whatsoever        */
#define  ERROR_NOMEM      "2"          /* Insufficient memory        */
#define  ERROR_FILEOPEN   "3"          /* Error opening text file    */

/*********************************************************************/
/*  Various definitions used by various functions.                   */
/*********************************************************************/

#define  MAX_DIGITS     9          /* maximum digits in numeric arg  */
#define  MAX            256        /* temporary buffer length        */
#define  IBUF_LEN       4096       /* Input buffer length            */
#define  STEMNAMLEN     256
#define  TMPBUFF        256

/* for DosAllocMem   */
#define  AllocFlag      PAG_COMMIT | PAG_READ | PAG_WRITE | OBJ_ANY

#define MAX_LINE_LEN   4096

/*********************************************************************/
/*  Defines used by SysDriveMap                                      */
/*********************************************************************/

#define  USED           0
#define  FREE           1
#define  DETACHED       2
#define  REMOTE         3
#define  LOCAL          4

/*********************************************************************/
/* Defines uses by SysTree                                           */
/*********************************************************************/

#define  FIRST_TIME     0x0001
#define  RECURSE        0x0002
#define  DO_DIRS        0x0004
#define  DO_FILES       0x0008
#define  NAME_ONLY      0x0010
#define  EDITABLE_TIME  0x0020
#define  LONG_TIME      0x0040
#define  RXIGNORE       2              /* Ignore attributes entirely */
#define  AllAtts        FILE_NORMAL | FILE_READONLY | FILE_HIDDEN | \
FILE_SYSTEM | FILE_DIRECTORY | FILE_ARCHIVED

/*********************************************************************/
/* Alpha Numeric Return Strings                                      */
/*********************************************************************/

#define  ERROR_RETSTR   "ERROR:"

/*********************************************************************/
/* Some useful macros                                                */
/*********************************************************************/

#define BUILDRXSTRING(t, s) (t)->strlength=strlen(strcpy((t)->strptr, (s)));


#define BUILDRXSTRING2(t, f, s) { \
        (t)->strlength=sprintf((t)->strptr, (f), (s)); \
        return VALID_ROUTINE; \
    }

#define RETVAL(retc) { \
        retstr->strlength = strlen(itoa(retc, retstr->strptr,10)); \
        return VALID_ROUTINE; \
    }

#define WININIT(a,b) {\
        b = WinInitialize(0); \
        if (b == NULLHANDLE ) { \
            b = WinQueryAnchorBlock(HWND_DESKTOP);  \
            a = FALSE; \
        } \
    }

#define WINTERM(a,b) if ( a ) WinTerminate( b );

/*********************************************************************/
/* RxStemData                                                        */
/*   Structure which describes as generic                            */
/*   stem variable.                                                  */
/*********************************************************************/

typedef struct RxStemData {
    SHVBLOCK      shvb;              /* Request block for RxVar      */
    char          ibuf[IBUF_LEN];    /* Input buffer                 */
    char          varname[MAX];      /* Buffer for the variable name */
    char          stemname[MAX];     /* Buffer for the variable name */
    unsigned long stemlen;           /* Length of stem.              */
    unsigned long vlen;              /* Length of variable value     */
    unsigned long j;                 /* Temp counter                 */
    unsigned long tlong;             /* Temp counter                 */
    unsigned long count;             /* Number of elements processed */
} RXSTEMDATA;


bool string2long(char *string, long *number);
bool string2ulong(char *string, unsigned long *number);

unsigned long SetRexxVariable(char *name, char *value);

void logmessage( char *entry);


