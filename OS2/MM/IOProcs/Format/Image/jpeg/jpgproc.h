/*
 * Copyright (c) Chris Wohlgemuth 2002
 * All rights reserved.
 *
 * http://www.geocities.com/SiliconValley/Sector/5785/
 * http://www.os2world.com/cdwriting
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The authors name may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/********************** START OF SPECIFICATIONS *************************/
/*                                                                      */
/* COPYRIGHT:                                                           */
/*              Chris Wohlgemuth 2002                                   */
/*                        All Rights Reserved                           */
/*                                                                      */
/* ABSTRACT: This file is the include/header file for use with the      */
/*           JPG IOProc                                                 */
/*                                                                      */
/*********************** END OF SPECIFICATIONS **************************/
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <os2medef.h>
#include <mmioos2.h>
#include "gbm.h"

#pragma pack(2)

static char     pszJPGExt  [] = "JPG";

typedef RGB FAR *PRGB;


/****************************************
 * IOProc information structure, used for every file opened
 *    by this IOProc
 ****************************************/
typedef struct _JPGFILESTATUS
{
  GBM gbm;
  int ft;  /* GBM index for the proc to use */
  int fHandleGBM;

  PBYTE   lpRGBBuf;               /* 24-bit RGB Buf for translated data */
  ULONG   ulRGBTotalBytes;        /* Length of 24-bit RGBBuf       */

  LONG    lImgBytePos;            /* Current pos in RGB buf        */

  ULONG   ulImgTotalBytes;
  BOOL    bSetHeader;             /* TRUE if header set in WRITE mode*/

  MMIMAGEHEADER   mmImgHdr;       /* Standard image header         */
} JPGFILESTATUS;
typedef JPGFILESTATUS FAR *PJPGFILESTATUS;

#define     MMOTION_HEADER_SIZE         sizeof (MMOTIONHEADER)
#define     ONE_BLOCK                   6L
#define     FOURCC_JPG                 mmioFOURCC('J', 'P', 'E', 'G')

/* RC file defines */

#define     HEX_FOURCC_JPG               0x4745504aL

#define     MMOTION_IOPROC_NAME_TABLE   7000
#define     MMOTION_NLS_CHARSET_INFO    7500

#ifndef BITT_NONE
#define BITT_NONE 0
#define BI_NONE   0
#endif

/**********************
 * Function Declarations
 **********************/
LONG EXPENTRY IOProc_Entry (PVOID  pmmioStr, USHORT usMsg,
                                   LONG   lParam1, LONG lParam2);


LONG GetFormatString (LONG lSearchId,
                      PSZ  pszFormatString,
                      LONG lBytes);

LONG GetFormatStringLength (LONG  lSearchId,
                            PLONG plNameLength);

ULONG APIENTRY GetNLSData (PULONG, PULONG);

#define DEBUG

#ifdef DEBUG
void writeLog(const char* chrFormat, ...)
{
  char logNameLocal[CCHMAXPATH];
  FILE *fHandle;

  sprintf(logNameLocal,"%s\\jpgio.log",getenv("LOGFILES"));
  fHandle=fopen(logNameLocal,"a");
  if(fHandle) {
    va_list arg_ptr;
    void *tb;

    va_start (arg_ptr, chrFormat);
    vfprintf(fHandle, chrFormat, arg_ptr);
    va_end (arg_ptr);
    fclose(fHandle);
  }
}
#endif

#pragma pack()
