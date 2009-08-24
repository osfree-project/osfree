/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  Contains helper functions functions:
*       SysBootDrive
*       SysDriveInfo
*       SysDriveMap
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
/* http://www.oorexx.org/license.html                          */
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
#include <string.h>
#include <ctype.h>

#include <rexxdefs.h>  // rexxsaa.h include in this header

RexxFunctionHandler SysBootDrive;
RexxFunctionHandler SysDriveInfo;
RexxFunctionHandler SysDriveMap;


/*************************************************************************
* Function:  SysBootDrive                                                *
*                                                                        *
* Syntax:    call SysBootDrive                                           *
*                                                                        *
* Return:    boot drive - 'C', 'D', 'E', etc.                            *
*                                                                        *
*************************************************************************/

unsigned long SysBootDrive(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long BootDrive[1];                  /* Major version number       */

    if (numargs != 0) return INVALID_ROUTINE;

    DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE, BootDrive, sizeof(BootDrive));

    retstr->strlength =  sprintf(retstr->strptr, "%c:", (char)(BootDrive[0]+64));

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysDriveInfo                                                *
*                                                                        *
* Syntax:    call SysDriveInfo drive                                     *
*                                                                        *
* Params:    drive - 'C', 'D', 'E', etc.                                 *
*                                                                        *
* Return:    disk free total label                                       *
*************************************************************************/

unsigned long SysDriveInfo(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    char *arg;                         /* Temp var for holding args  */
    unsigned char     buf[256];        /* Give DosQFSInfo 256 bytes  */

    unsigned long     drivenum;        /* Drive number (A=1, B=2,    */
                                       /* etc)                       */
    FSALLOCATE FSInfoBuf;              /* Structure required by      */
                                       /* DosQfsInfo                 */
    long      rc;                      /* OS/2 return code           */

                                       /* validate arguments         */
    if (numargs != 1 || args[0].strlength > 2 ||
        args[0].strlength == 0) return INVALID_ROUTINE;

                                       /* validate the arg           */
    arg = args[0].strptr;              /* get argument pointer       */
                                       /* drive letter?              */

    if ((strlen(arg) == 2 && arg[1] != ':') ||
          !isalpha(arg[0])) return INVALID_ROUTINE;
    else drivenum = toupper(arg[0])-'A'+1;  /* Get the drive number       */

                                       /* query the disk             */
    DosError(0);                       /* Disable hard-error pop-up  */
                                       /* Get the drive info         */

    rc = DosQueryFSInfo(drivenum, 2, buf, sizeof(buf));
    DosQueryFSInfo(drivenum, 1, (char *)&FSInfoBuf, sizeof(FSInfoBuf));

    if (rc == 0 || rc == 125) {
        sprintf(retstr->strptr, "%c:  %-12lu %-12lu %-13s", (drivenum+'A'-1),
            FSInfoBuf.cSectorUnit * FSInfoBuf.cbSector *FSInfoBuf.cUnitAvail,
            FSInfoBuf.cSectorUnit * FSInfoBuf.cbSector *FSInfoBuf.cUnit, &buf[5]);
                                       /* create return string       */
        retstr->strlength = strlen(retstr->strptr);
    } else retstr->strlength = 0;             /* return null string         */

    DosError(1);                         /* Enable hard-error pop-up   */

    return VALID_ROUTINE;                /* no error on call           */

}


/*************************************************************************
* Function:  SysDriveMap                                                 *
*                                                                        *
* Syntax:    call SysDriveMap [drive] [,mode]                            *
*                                                                        *
* Params:    drive - 'C', 'D', 'E', etc.  The drive to start the search  *
*                     with.                                              *
*            mode  - Any of the following:  'FREE', 'USED', 'DETACHED',  *
*                                           'LOCAL', 'REMOTE'            *
*                                                                        *
* Return:    'A: B: C: D: ...'                                           *
*************************************************************************/

unsigned long SysDriveMap(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    char     temp[MAX];                  /* Entire drive map built here*/
    char     tmpstr[MAX];                /* Single drive entries built */
                                         /* here                       */
    char     buf[256];                   /* Give DosQFSInfo 256 bytes  */
    char     DeviceName[3];              /* Device name or drive letter*/
                                         /* string                     */
    unsigned long    CurDrive;           /* Current drive              */
    unsigned long    DriveMap;           /* Drive map                  */
    unsigned long    Ordinal;            /* Ordinal of entry in name   */
                                         /* list                       */
    unsigned long    FSAInfoLevel;       /* Type of attached FSD data  */
                                         /* required                   */
    unsigned long    DataBufferLen;      /* Buffer length              */
    unsigned long    dnum;               /* Disk num variable          */
    unsigned long    start = 3;          /* Initial disk num           */
    unsigned long    Mode = USED;        /* Query mode USED, FREE,     */
                                         /* LOCAL, etc                 */
    FSQBUFFER2 DataBuffer;               /* Returned data buffer       */
    long     rc;                         /* OS/2 return codes          */

    Ordinal = (unsigned long )0;
    FSAInfoLevel = (unsigned long )1;

    temp[0] = '\0';

    if (numargs > 2) return INVALID_ROUTINE;

    if (numargs >= 1 && args[0].strptr) {
        if ((strlen(args[0].strptr) == 2 &&
            args[0].strptr[1] != ':') ||
            strlen(args[0].strptr) > 2 ||
            strlen(args[0].strptr) == 0 ||
            !isalpha(args[0].strptr[0]))
            return INVALID_ROUTINE;
        start = toupper(args[0].strptr[0])-'A'+1;
    }
                                         /* check the mode             */
    if (numargs == 2 && args[1].strlength != 0) {

        if (!stricmp(args[1].strptr, "FREE")) Mode = FREE;
        else if (!stricmp(args[1].strptr, "USED")) Mode = USED;
        else if (!stricmp(args[1].strptr, "DETACHED")) Mode = DETACHED;
        else if (!stricmp(args[1].strptr, "REMOTE")) Mode = REMOTE;
        else if (!stricmp(args[1].strptr, "LOCAL")) Mode = LOCAL;
        else return INVALID_ROUTINE;
    }
                                         /* perform the query          */
    DosError(0);                         /* Disable Hard-Error Popups  */

    DosQueryCurrentDisk(&CurDrive, &DriveMap);

    DriveMap>>=start-1;                  /* Shift to the first drive   */

    temp[0] = '\0';                      /* Clear temporary buffer     */

    for (dnum = start; dnum <= 26; dnum++) {
        if (!(DriveMap&(unsigned long)1) && Mode == FREE) {
            sprintf(tmpstr, "%c: ", dnum+'A'-1);
            strcat(temp, tmpstr);
        }
                                         /* Hey, we have a used drive  */
        else if ((DriveMap&(unsigned long)1) && Mode == USED) {
            sprintf(tmpstr, "%c: ", dnum+'A'-1);
            strcat(temp, tmpstr);
        }

        else if (DriveMap&(unsigned long)1) {      /* Check specific drive info  */
            sprintf(DeviceName, "%c:", dnum+'A'-1);
            DataBufferLen = sizeof DataBuffer;
            DosQueryFSAttach(DeviceName, Ordinal, FSAInfoLevel, &DataBuffer, &DataBufferLen);
            rc = DosQueryFSInfo(dnum, 2, buf, sizeof(buf));

            if (rc == 67 && DataBuffer.iType == 4 && Mode == DETACHED) {
                                         /* Hey, we have a detached    */
                                         /* drive                      */
                sprintf(tmpstr, "%c: ", dnum+'A'-1);
                strcat(temp, tmpstr);
            } else if (DataBuffer.iType == 4 && Mode == REMOTE) {
                sprintf(tmpstr, "%c: ", dnum+'A'-1);
                strcat(temp, tmpstr);
            } else if (DataBuffer.iType == 3 && Mode == LOCAL) {
                sprintf(tmpstr, "%c: ", dnum+'A'-1);
                strcat(temp, tmpstr);
            }
        }

        DriveMap>>=1;                      /* Shift to the next drive    */
    }

    BUILDRXSTRING(retstr, temp);         /* pass back result           */

    if (retstr->strlength) retstr->strlength--;

    DosError(1);                         /* Enable Hard-Error Popups   */

    return VALID_ROUTINE;                /* no error on call           */
}

