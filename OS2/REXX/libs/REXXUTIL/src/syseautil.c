/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  Contains the following functions:
*      SysGetEA
*      SysPutEA
*      SysQueryEAList
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

#define INCL_DOSMEMMGR
#define INCL_ERRORS

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <string.h>

#include <rexxdefs.h>  // rexxsaa.h include in this header

#define  EABUFFER       20000

RexxFunctionHandler SysGetEA;
RexxFunctionHandler SysPutEA;
RexxFunctionHandler SysQueryEAList;


/*************************************************************************
* Function:  SysGetEA                                                    *
*                                                                        *
* Syntax:    call SysGetEA file, EA_name, variable                       *
*                                                                        *
* Params:    file - file containing EA.                                  *
*            EA_name - name of EA to be retrieved                        *
*            name of variable EA is placed in                            *
*                                                                        *
* Return:    Return code from DosQueryFileInfo.                          *
*************************************************************************/

unsigned long SysGetEA(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    long          rc;                      /* Ret code                   */
    unsigned char geabuff[300];            /* buffer for GEA             */
    unsigned long act;                     /* open action                */
    void          *fealist;                /* fealist buffer             */
    EAOP2         eaop;                    /* eaop structure             */
    PGEA2         pgea;                    /* pgea structure             */
    PFEA2         pfea;                    /* pfea structure             */
    HFILE         handle;                  /* file handle                */
    RXSTEMDATA    ldp;                     /* stem data                  */

    if (numargs != 3 || !RXVALIDSTRING(args[0]) ||
            !RXVALIDSTRING(args[1]) || !RXVALIDSTRING(args[2]))
            return INVALID_ROUTINE;

    ldp.count = 0;                       /* get the stem variable name */
    strcpy(ldp.varname, args[2].strptr);

    ldp.stemlen = args[2].strlength;
    strupr(ldp.varname);                 /* uppercase the name         */

    if (rc = DosOpen(args[0].strptr, &handle, &act,
            0L, 0, OPEN_ACTION_OPEN_IF_EXISTS,
            OPEN_ACCESS_READONLY + OPEN_SHARE_DENYREADWRITE +
            OPEN_FLAGS_FAIL_ON_ERROR + OPEN_FLAGS_WRITE_THROUGH,
            NULL)) {
        RETVAL(rc)
    }                                    /* get the file status info   */

    if (DosAllocMem((PPVOID)&fealist, 0x00010000L, AllocFlag)) {
        BUILDRXSTRING(retstr, ERROR_NOMEM);
        return VALID_ROUTINE;
    }
                                         /* FEA and GEA lists          */
    eaop.fpGEA2List = (PGEA2LIST)geabuff;
    eaop.fpFEA2List = (PFEA2LIST)fealist;
    eaop.oError = 0;                     /* no error occurred yet      */
    pgea = &eaop.fpGEA2List->list[0];    /* point to first GEA         */
    eaop.fpGEA2List->cbList = sizeof(ULONG) + sizeof(GEA2) +
        args[1].strlength;
    eaop.fpFEA2List->cbList = (ULONG)0xffff;

                                         /* fill in the EA name length */
    pgea->cbName = (BYTE)args[1].strlength;
    strcpy(pgea->szName, args[1].strptr);/* fill in the name           */
    pgea->oNextEntryOffset = 0;          /* fill in the next offset    */
                                         /* read the extended attribute*/
    rc = DosQueryFileInfo(handle, 3, (PSZ)&eaop, sizeof(EAOP2));
    DosClose(handle);                    /* close the file             */

    if (eaop.fpFEA2List->cbList <= sizeof(ULONG))
      rc = ERROR_EAS_NOT_SUPPORTED;      /* this is error also         */

    sprintf(retstr->strptr, "%d", rc);   /* format return code         */
    retstr->strlength = strlen(retstr->strptr);

    if (rc) {                            /* failure?                   */
        DosFreeMem(fealist);               /* error, get out             */
        return VALID_ROUTINE;
    }

    pfea = &(eaop.fpFEA2List->list[0]);  /* point to the first FEA     */
    ldp.shvb.shvnext = NULL;
    ldp.shvb.shvname.strptr = ldp.varname;
    ldp.shvb.shvname.strlength = ldp.stemlen;
    ldp.shvb.shvnamelen = ldp.stemlen;
    ldp.shvb.shvvalue.strptr = ((PSZ)pfea->szName+(pfea->cbName+1));
    ldp.shvb.shvvalue.strlength = pfea->cbValue;
    ldp.shvb.shvvaluelen = ldp.shvb.shvvalue.strlength;
    ldp.shvb.shvcode = RXSHV_SET;
    ldp.shvb.shvret = 0;
    if (RexxVariablePool(&ldp.shvb) == RXSHV_BADN) {
        DosFreeMem(fealist);               /* free our buffer            */
        return INVALID_ROUTINE;            /* error on non-zero          */
    }

    DosFreeMem(fealist);                 /* free our buffer            */

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysPutEA                                                    *
*                                                                        *
* Syntax:    call SysPutEA file, EA_name, value                          *
*                                                                        *
* Params:    file - file containing EA.                                  *
*            EA_name - name of EA to be written                          *
*            new value for the EA                                        *
*                                                                        *
* Return:    Return code from DosQueryFileInfo.                          *
*************************************************************************/

unsigned long SysPutEA(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;                      /* Ret code                   */
    unsigned long act;                     /* open action                */
    void          *fealist;                /* fealist buffer             */
    EAOP2         eaop;                    /* eaop structure             */
    PFEA2         pfea;                    /* pfea structure             */
    HFILE         handle;                  /* file handle                */



    if (numargs != 3 || !RXVALIDSTRING(args[0]) || !RXVALIDSTRING(args[1]))
            return INVALID_ROUTINE;

    if (rc = DosOpen2(args[0].strptr, &handle, &act,
                      0L, 0, OPEN_ACTION_OPEN_IF_EXISTS,
                      OPEN_ACCESS_READWRITE + OPEN_SHARE_DENYWRITE +
                      OPEN_FLAGS_FAIL_ON_ERROR + OPEN_FLAGS_WRITE_THROUGH,
                      NULL)) {
        RETVAL(rc)
    }

    if (DosAllocMem((PPVOID)&fealist, 0x00010000L, AllocFlag)) {
        BUILDRXSTRING(retstr, ERROR_NOMEM);
        return VALID_ROUTINE;
    }

    eaop.fpFEA2List = (PFEA2LIST)fealist;/* Set memory for the FEA     */
    eaop.fpGEA2List = NULL;              /* GEA is unused              */
    pfea = &eaop.fpFEA2List->list[0];    /* point to first FEA         */
    pfea->fEA = '\0';                    /* set the flags              */
                                         /* Size of FEA name field     */
    pfea->cbName = (BYTE)args[1].strlength;
                                         /* Size of Value for this one */
    pfea->cbValue = (SHORT)args[2].strlength;
                                         /* Set the name of this FEA   */
    strcpy((PSZ)pfea->szName, args[1].strptr);
                                         /* Set the EA value           */
    memcpy((PSZ)pfea->szName+(pfea->cbName+1), args[2].strptr,
        args[2].strlength);
    pfea->oNextEntryOffset = 0;          /* no next entry              */
    eaop.fpFEA2List->cbList =            /* Set the total size var     */
        sizeof(ULONG) + sizeof(FEA2) + pfea->cbName + pfea->cbValue;

                                         /* set the file info          */
    rc = DosSetFileInfo(handle, 2, (PSZ)&eaop, sizeof(EAOP2));
    DosClose(handle);                    /* Close the File             */
    DosFreeMem(fealist);                 /* Free the memory            */

    RETVAL(rc)
}


/*************************************************************************
* Function:  SysQueryEAList                                              *
*                                                                        *
* Syntax:    call SysQueryEAList  file, stem                             *
*                                                                        *
* Params:    file - queried file or directory                            *
*            stem - name of stem to receive class names                  *
*                                                                        *
* Return:    Error code                                                  *
*                                                                        *
*************************************************************************/

/*
 * NOTE: It is very important that you know I **hate** working with EAs
 * and this might not be nice but it works. I am so tried of messing with
 * this function - have ball fixing it. Mike
 */

unsigned long SysQueryEAList(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    int  countEA = 0;

    unsigned long rc;
    unsigned long act;
    unsigned long numberEA;
    unsigned long offset   = 0;

    char szStemName[256] = {0};
    char fileName[512]   = {0};
    char *pszStemIdx     = NULL;
    char buffer[EABUFFER];
    char eanum[5];

    FEA2         *ptrBuffer;
    FILESTATUS3  status;
    HFILE        handle;

    /* Only two arguments accepted */
    if (numargs != 2 || !RXVALIDSTRING(args[0]) ||
            !RXVALIDSTRING(args[1]) || args[0].strlength > 255)
            return INVALID_ROUTINE;

    /* remember stem name */
    strcpy(szStemName, args[1].strptr);
    strupr(szStemName);
    strcpy(fileName, args[0].strptr);

    // check for '.' and if not there make it so
    if (szStemName[args[1].strlength-1] != '.')
        szStemName[args[1].strlength] = '.';

    // pointer to the index part of stem
    pszStemIdx = &(szStemName[strlen(szStemName)]);

    // query to find if it is a file or directory
    if(DosQueryPathInfo(fileName,
                        FIL_STANDARD,
                        (void *)&status,
                        sizeof(status))) return INVALID_ROUTINE;

    // if it is a file open it
    if(!(status.attrFile&FILE_DIRECTORY)) {
        if(DosOpen(fileName, &handle, &act, 0L, 0,
                   FILE_OPEN,
                   OPEN_ACCESS_READONLY + OPEN_SHARE_DENYWRITE + OPEN_FLAGS_FAIL_ON_ERROR,
                   NULL)) return INVALID_ROUTINE;
    }

    numberEA = (ULONG)-1;

    rc = DosEnumAttribute(ENUMEA_REFTYPE_PATH,
                          fileName, 1,
                          (PVOID)&buffer, EABUFFER,
                          (ULONG *)&numberEA, ENUMEA_LEVEL_NO_VALUE);

    if(rc) RETVAL(0)

    // if it is a file - close it
    if (!(status.attrFile&FILE_DIRECTORY)) DosClose(handle);

    ptrBuffer = (FEA2 *)&buffer;   // set pointer to statrt of buffer

    if(numberEA != 0) {                   // if there are EAs found
        do {
            ptrBuffer = (PDENA2)((char *)ptrBuffer + offset);

            itoa(++countEA, pszStemIdx, 10);

            rc = SetRexxVariable(szStemName, ptrBuffer->szName);

            offset = ptrBuffer->oNextEntryOffset;

        } while ( ptrBuffer->oNextEntryOffset != 0 );
    } // end of handling EAs

    // setup the 0 index with number of classes
    strcpy(pszStemIdx, "0");
    sprintf(eanum, "%d", numberEA);

    rc = SetRexxVariable(szStemName, eanum);

    RETVAL(1);
}

