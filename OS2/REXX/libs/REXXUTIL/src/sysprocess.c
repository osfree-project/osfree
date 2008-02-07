/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  Contains the following functions:
*      SysProcessType
*      SysSetPriority
*      SysQueryExtLIBPATH
*      SysSetExtLIBPATH
*      SysDumpVariables
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

#define INCL_DOSPROCESS
#define INCL_DOSMISC

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>
#include <sys\types.h>
#include <fcntl.h>
#include <io.h>

#include <alloca.h>

#include <rexxdefs.h>  // rexxsaa.h include in this header

RexxFunctionHandler SysProcessType;
RexxFunctionHandler SysSetPriority;
RexxFunctionHandler SysQueryExtLIBPATH;
RexxFunctionHandler SysSetExtLIBPATH;
RexxFunctionHandler SysDumpVariables;
RexxFunctionHandler SysLoadLibrary; // Undocumented returns INVALID_ROUTINE
RexxFunctionHandler SysDropLibrary; // Undocumented returns INVALID_ROUTINE


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

unsigned long SysProcessType(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    PPIB   ppib = NULL;

    if (numargs != 0) return INVALID_ROUTINE;

    DosGetInfoBlocks(NULL, &ppib);

    RETVAL(ppib->pib_ultype)
}


/*************************************************************************
* Function:  SysSetPriority                                              *
*                                                                        *
* Syntax:    result = SysSetPriority(Class, delta)                       *
*                                                                        *
* Params: Class - 0 No change (leave as-is).                             *
*                 1 Idle-time priority.                                  *
*                 2 Regular priority.                                    *
*                 3 Time-critical priority.                              *
*                 4 Foreground server (fixed high) priority.             *
*        delta  - value must fall within the range of -31 to +31         *
*                                                                        *
* Return: SysSetPriority returns the DosSetPriority API return code.     *
*                 0   Success.                                           *
*                 303 Invalid process ID.                                *
*                 304 Invalid priority delta.                            *
*                 305 Not descendant.                                    *
*                 307 Invalid priority class.                            *
*                 308 Invalid scope.                                     *
*                 309 Invalid thread ID.                                 *
*************************************************************************/

unsigned long SysSetPriority(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long Class;
    long delta;

    // requires 2 args
    if (numargs != 2) return INVALID_ROUTINE;

    if (!string2ulong(args[0].strptr, &Class)) return INVALID_ROUTINE;
    if (!string2long(args[1].strptr, &delta)) return INVALID_ROUTINE;

    RETVAL(DosSetPriority(PRTYS_THREAD, Class, delta, 0L))
}


/*************************************************************************
* Function:  SysQueryExtLIBPATH                                          *
*                                                                        *
* Params: B Returns the current BEGINLIBPATH.                            *
*         E Returns the current ENDLIBPATH.                              *
*                                                                        *
* Return: SysQueryExtLIBPATH returns the value of the specified extended *
*         LIBPATH. This is a string value which uses the same format as  *
*         a path environment variable (i.e. path names separated by      *
*         semicolons).                                                   *
**************************************************************************/

unsigned long SysQueryExtLIBPATH(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;
    unsigned long flags;

    // requires 1 arg
    if (numargs != 1) return INVALID_ROUTINE;

    // handle flags passed
    switch (args[0].strptr[0]) {

    case 'b':
    case 'B':
        flags = BEGIN_LIBPATH;
        break;

    case 'e':
    case 'E':
        flags = END_LIBPATH;
        break;

    default:
        return INVALID_ROUTINE;
    }

    rc = DosQueryExtLIBPATH(retstr->strptr, flags);

    if(rc != 0) RETVAL(rc)

    retstr->strlength = strlen(retstr->strptr);

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysSetExtLIBPATH  path flags                                *
*                                                                        *
* Params: path   list of paths or %BEGINLIBPATH% / %ENDLIBPATH%          *
*                if none then flag path will be cleared                  *
*                                                                        *
*         flags  B Returns the current BEGINLIBPATH.                     *
*                E Returns the current ENDLIBPATH.                       *
*                                                                        *
* Return:   0 Success.                                                   *
*           8 Not enough memory.                                         *
*          87 Invalid parameter.                                         *
**************************************************************************/

unsigned long SysSetExtLIBPATH(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long flags;

    char extLib[1024] = {0};        // DosSetExtLIBPATH - final value of
                                    // the extended LIBPATH after substitution
                                    // may not exceed 1024

    // requires 2 args
    if (numargs != 2) return INVALID_ROUTINE;

    // handle flags passed first
    switch (args[1].strptr[0]) {

    case 'b':
    case 'B':
        flags = BEGIN_LIBPATH;
        break;

    case 'e':
    case 'E':
        flags = END_LIBPATH;
        break;

    default:
        return INVALID_ROUTINE;
    }

    if (args[0].strptr != NULL) sprintf(extLib, "%s", args[0].strptr);

    RETVAL(DosSetExtLIBPATH(extLib, flags))
}


/*************************************************************************
* Function:  SysDumpVariables                                            *
*                                                                        *
* Syntax:    result = SysDumpVariables([filename])                       *
*                                                                        *
* Params:    filename - name of the file where variables are appended to *
*                       (dump is written to stdout if omitted)           *
*                                                                        *
* Return:    0 - dump completed OK                                       *
*           -1 - failure during dump                                     *
*************************************************************************/

unsigned long SysDumpVariables(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    int       outFile;
    SHVBLOCK  shvb;
    bool      fCloseFile = FALSE;
    char     *buffer = NULL;             /* ENG: write result file to  */
    char     *current, *end;             /* memory first, much faster! */
    size_t    buffer_size = 10240;       /* buffer, realloc'd if needed*/
    size_t    new_size;                  /* real new size              */
    unsigned long rc;                        /* Ret code                   */


    if ((numargs > 1) || ((numargs > 0) && !RXVALIDSTRING(args[0])))
        return INVALID_ROUTINE;          /* raise error condition      */

    if (numargs > 0) {
        /* open output file for append */
        outFile = open(args[0].strptr, O_WRONLY | O_APPEND | O_CREAT,
                     S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IWOTH | S_IROTH);

        if(outFile < 0) return INVALID_ROUTINE; /* raise error condition      */

        fCloseFile = TRUE;

    } else outFile = STDOUT_FILENO;

                                         /* write results to memory    */
                                         /* first and then in one step */
                                         /* to disk                    */
    buffer = (char*) calloc(buffer_size,1);
    if (buffer == NULL) return INVALID_ROUTINE; /* raise error condition      */
    current = buffer;
    end = current + buffer_size;

    do {
        /* prepare request block */
        shvb.shvnext = NULL;
        shvb.shvname.strptr = NULL;      /* let REXX allocate the memory */
        shvb.shvname.strlength = 0;
        shvb.shvnamelen = 0;
        shvb.shvvalue.strptr = NULL;     /* let REXX allocate the memory */
        shvb.shvvalue.strlength = 0;
        shvb.shvvaluelen = 0;
        shvb.shvcode = RXSHV_NEXTV;
        shvb.shvret = 0;

        rc = RexxVariablePool(&shvb);

        if (rc == RXSHV_OK) {
             new_size = 5 + 9 + 3 + shvb.shvname.strlength + shvb.shvvalue.strlength;
                                     /* if buffer is not big enough, */
                                     /* reallocate                   */
            if (current + new_size >= end) {
                int offset = current - buffer;
                buffer_size *= 2;
                /* if new buffer too small, use the minimal fitting size */
                if (buffer_size - offset < new_size) {
                    buffer_size = new_size + offset;
                }
                buffer = realloc(buffer,buffer_size);
                current = buffer + offset;
                end = buffer + buffer_size;
            }
            sprintf(current, "Name=");
            current += 5;
            memcpy(current, shvb.shvname.strptr, shvb.shvname.strlength);
            current += shvb.shvname.strlength;
            sprintf(current, ", Value='");
            current += 9;
            memcpy(current, shvb.shvvalue.strptr, shvb.shvvalue.strlength);
            current += shvb.shvvalue.strlength;
            sprintf(current, "'\r\n");
            current += 3;

            /* free memory allocated by REXX */
            DosFreeMem(shvb.shvname.strptr);
            DosFreeMem(shvb.shvvalue.strptr);

            /* leave loop if this was the last var */
            if (shvb.shvret & RXSHV_LVAR) break;
        }
    } while (rc == RXSHV_OK);

    write(outFile, buffer, (current - buffer));

    free(buffer);

    if (fCloseFile) close(outFile);

    /*  if (rc != RXSHV_OK)  */
    if (rc != RXSHV_LVAR) RETVAL(-1)
    else RETVAL(0)
}


/* These are undocumented so I just return INVALID_ROUTINE
   until I find something more on both
*/
unsigned long SysLoadLibrary(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    if (numargs < 1 || numargs > 2 || !RXVALIDSTRING(args[0]))
            return INVALID_ROUTINE;

    return VALID_ROUTINE;
}


unsigned long SysDropLibrary(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{

    return INVALID_ROUTINE;
}

