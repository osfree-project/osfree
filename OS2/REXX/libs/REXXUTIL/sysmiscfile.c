/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  Contains the following functions:
*      SysMkDir
*      SysRmDir
*      SysTempFileName
*      SysAddFileHandle
*      SysSetFileHandle
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
#define INCL_DOSFILEMGR

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include <rexxdefs.h>  // rexxsaa.h include in this header

static void GetUniqueFileName(char *Template, char Filler, char *file);

RexxFunctionHandler SysWildCard;
RexxFunctionHandler SysMkDir;
RexxFunctionHandler SysRmDir;
RexxFunctionHandler SysTempFileName;
RexxFunctionHandler SysAddFileHandle;
RexxFunctionHandler SysSetFileHandle;


/*************************************************************************
* Function:  SysMkDir                                                    *
*                                                                        *
* Syntax:    call SysMkDir source, wildcard                              *
*                                                                        *
* Params:    source - Filename string to be transformed.                 *
*            wildcard - Wildcard pattern used to transform the           *
*                       source string.                                   *
*                                                                        *
* Return:    returns the transformed string. If an error occurs,         *
*            a null string ('') is returned.                             *
*************************************************************************/

unsigned long SysWildCard(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;                        /* Return code */

    char *buffer = NULL;

    /* If no args, then its an  incorrect call */
    if (numargs != 2) return INVALID_ROUTINE;

    buffer = alloca(_MAX_FNAME + _MAX_EXT);

    rc = DosEditName(1, args[0].strptr, args[1].strptr,
                     buffer, (_MAX_FNAME + _MAX_EXT));

    /* result is return code      */
    retstr->strlength = sprintf(retstr->strptr, "%s", buffer);

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysMkDir                                                    *
*                                                                        *
* Syntax:    call SysMkDir dir                                           *
*                                                                        *
* Params:    dir - Directory to be created.                              *
*                                                                        *
* Return:    NO_UTIL_ERROR                                               *
*            Return code from DosCreateDir()                             *
*************************************************************************/

unsigned long SysMkDir(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;                        /* Return code */

    /* If no args, then its an  incorrect call */
    if (numargs != 1) return INVALID_ROUTINE;

    /* make the directory         */
    rc = DosCreateDir(args[0].strptr, 0L);

    /* result is return code      */
    retstr->strlength = sprintf(retstr->strptr, "%d", rc);

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysRmDir                                                    *
*                                                                        *
* Syntax:    call SysRmDir dir                                           *
*                                                                        *
* Params:    dir - Directory to be removed.                              *
*                                                                        *
* Return:    NO_UTIL_ERROR                                               *
*            Return code from DosDeleteDir()                             *
*************************************************************************/

unsigned long SysRmDir(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;                  /* Return code */

    /* If no args, then its an  incorrect call */
    if (numargs != 1) return INVALID_ROUTINE;

    /* remove the directory */
    rc = DosDeleteDir(args[0].strptr);

    /* result is return code      */
    retstr->strlength = sprintf(retstr->strptr, "%d", rc);

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysTempFileName                                             *
*                                                                        *
* Syntax:    call SysTempFileName template [,filler]                     *
*                                                                        *
* Params:    template - Description of filespec desired.  For example:   *
*                        C:\TEMP\FILE.???                                *
*            filler   - A character which when found in template will be *
*                        replaced with random digits until a unique file *
*                        or directory is found.  The default character   *
*                        is '?'.                                         *
*                                                                        *
* Return:    other - Unique file/directory name.                         *
*            ''    - No more files exist given specified template.       *
*************************************************************************/

unsigned long SysTempFileName(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    char filler;                       /* filler character           */

    /* validate arguments */
    if (numargs < 1 || numargs > 2 || !RXVALIDSTRING(args[0]) ||
        args[0].strlength > 512) return INVALID_ROUTINE;

    /* get filler character       */
    if (numargs == 2 && !RXNULLSTRING(args[1])) {
        if (args[1].strlength != 1) return INVALID_ROUTINE; /* must be one character      */
        filler = args[1].strptr[0];
    } else filler = '?';

    /* get the file id            */
    GetUniqueFileName(args[0].strptr, filler, retstr->strptr);

    retstr->strlength = strlen(retstr->strptr);

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysAddFileHandle -Adds to the number of file handles        *
*                              available to the current process.         *
*                                                                        *
* Syntax:    call SysSearchPath handles                                  *
*                                                                        *
* Params:    handles - The desired number of additional file handles     *
*            A positive number increases and a negative number should    *
*            decrease, but the system may defer or disregard a request   *
*            to decrease the maximum number of file handles for the      *
*            current process (DosSetRelMaxFH docs). I have yet to see    *
*            the number decreased. A 0 or no options passed will return  *
*            the current number of file handles.                         *
*                                                                        *
* Return:    Returns the number of file handles now available.           *
*                                                                        *
*************************************************************************/

unsigned long SysAddFileHandle(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    long ReqCount = 0;
    unsigned long CurMaxFH = 0;

    // have right number of args?
    if (numargs > 1) return INVALID_ROUTINE;

    if (numargs != 0) {
        // convert arg string to long
        if (!string2long(args[0].strptr, &ReqCount)) return INVALID_ROUTINE;
    }

    DosSetRelMaxFH(&ReqCount, &CurMaxFH);

    retstr->strlength = sprintf(retstr->strptr, "%d", CurMaxFH);

    return VALID_ROUTINE;                /* no error on call            */
}


/*************************************************************************
* Function:  SysSetFileHandle                                            *
*                                                                        *
*                                                                        *
* Syntax:    call SysSearchPath handles                                  *
*                                                                        *
* Params:    handles - Sets the maximum number of file handles available *
*            to the current process.                                     *
*                                                                        *
* Return:    SysSetFileHandle returns the DosSetMaxFH API return code.   *
*                                                                        *
*************************************************************************/

unsigned long SysSetFileHandle(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;
    unsigned long ReqCount = 0;

    // have right number of args?
    if (numargs != 1) return INVALID_ROUTINE;

    // convert arg string to long
    if (!string2ulong(args[0].strptr, &ReqCount)) return INVALID_ROUTINE;

    rc = DosSetMaxFH(ReqCount);

    /* result is return code      */
    retstr->strlength = sprintf(retstr->strptr, "%d", rc);

    return VALID_ROUTINE;                /* no error on call            */
}


/****************************************************************
* Function: GetUniqueFileName(Template, Filler, file)           *
*                                                               *
* Purpose:  This function returns a unique temporary file name  *
*           given a template and a filler character.            *
*                                                               *
* Params:   CHAR* Template - The template.  Must contain at     *
*                            least one or more filler chars.    *
*                                                               *
*                            Example:  "C:\TEMP\FILE????.DAT    *
*                                                               *
*           CHAR Filler    - The character in the Template to   *
*                            be replaced with numbers.  For     *
*                            the above example, the filler char *
*                            would be '?'.                      *
*           CHAR* file     - file name produced (output)        *
*                                                               *
* Used By:  SysTempFileName()                                   *
****************************************************************/

static void GetUniqueFileName(char *Template, char Filler, char *file)
{
    char numstr[6];
    bool Unique = FALSE;

    unsigned long rc;             /* return code for OS/2 call    */
    unsigned long x;              /* loop index                   */
    unsigned long i;              /*                              */
    unsigned long j = 0;          /* number of filler chars found */
    unsigned long num;            /* temporary random number      */
    unsigned long start;          /* first random number          */
    unsigned long max = 1;        /* maximum random number        */

    unsigned long SearchCount;    /* count of files to search     */
                                  /* for.                         */
    HDIR DirHandle;               /* handle for found file        */
    int  seed;                    /* to get current time          */

    FILEFINDBUF3 finfo;           /* OS2 File Find Utility        */
                                  /* Structure                    */
    DATETIME DT;                  /* The date and time structure  */

    /** Determine number of filler characters *                   */

    for (x = 0; Template[x] != 0; x++)
        if (Template[x] == Filler) {
            max = max *10;
            j++;
        }

    /** Return NULL string if less than 1 or greater than 4 *           */

    if (j == 0 || j > 5) {
        Unique = TRUE;
        strcpy(file, "");
    }

    /** Get a random number in the appropriate range                    */

    DosGetDateTime((PDATETIME)&DT);      /* Get the time               */
    seed = DT.hours*60 + DT.minutes;     /* convert to hundreths       */
    seed = seed*60 + DT.seconds;
    seed = seed*100 + DT.hundredths;
    srand((INT)seed);                    /* Set random number seed     */
    num = (ULONG)rand()%max;
    start = num;

    /** Do until a unique name is found                                 */

    while (!Unique) {

        /** Generate string which represents the number                  */

        switch (j) {

        case 1 :
            sprintf(numstr, "%01u", num);
            break;

        case 2 :
            sprintf(numstr, "%02u", num);
            break;

        case 3 :
            sprintf(numstr, "%03u", num);
            break;

        case 4 :
            sprintf(numstr, "%04u", num);
            break;
        case 5 :
            sprintf(numstr, "%05u", num);
            break;
        }

        /** Subsitute filler characters with numeric string **/

        i = 0;

        for (x = 0; Template[x] != 0; x++)
            if (Template[x] == Filler) file[x] = numstr[i++];
            else file[x] = Template[x];

        file[x] = '\0';

        /** See if the file exists */

        DosError(0);  /* Disable Hard-Error popups  */

        DirHandle = 0xFFFF;
        SearchCount = 1;

        rc = DosFindFirst(file,
                          &DirHandle,
                          AllAtts,
                          (PVOID)&finfo,
                          (ULONG)sizeof(finfo),
                          (PULONG)&SearchCount,
                          FIL_STANDARD);

        if (rc == 2 || rc == 18) Unique = TRUE;

        DosFindClose(DirHandle);

        DosError(1);  /* Enable Hard-Error popups   */

        /** Make sure we are not wasting our time **/

        num = (num+1)%max;

        if (num == start && !Unique) {
            Unique = TRUE;
            strcpy(file, "");
        }
    }
}

