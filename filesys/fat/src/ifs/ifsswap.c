#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#define INCL_DOS
#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_DOSERRORS

#include "os2.h"
#include "portable.h"
#include "fat32ifs.h"

/******************************************************************
*
******************************************************************/
int far pascal __loadds  FS_OPENPAGEFILE (
    unsigned long far *pFlags,      /* pointer to Flags           */
    unsigned long far *pcMaxReq,    /* max # of reqs packed in list   */
    char far *         pName,       /* name of paging file        */
    struct sffsi far * psffsi,      /* ptr to fs independent SFT      */
    struct sffsd far * psffsd,      /* ptr to fs dependent SFT        */
    unsigned short     OpenMode,    /* sharing, ...           */
    unsigned short     OpenFlag,    /* open flag for action       */
    unsigned short     Attr,        /* file attribute             */
    unsigned long      Reserved     /* reserved, must be zero         */
)
{
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_OPENPAGEFILE - NOT SUPPORTED");
   return ERROR_NOT_SUPPORTED;

   pFlags = pFlags;
   pcMaxReq = pcMaxReq;
   pName = pName;
   psffsi = psffsi;
   psffsd = psffsd;
   OpenMode = OpenMode;
   OpenFlag = OpenFlag;
   Attr = Attr;
   Reserved = Reserved;
}


/******************************************************************
*
******************************************************************/
int far pascal __loadds  FS_ALLOCATEPAGESPACE(
    struct sffsi far *psffsi,       /* ptr to fs independent SFT */
    struct sffsd far *psffsd,       /* ptr to fs dependent SFT   */
    unsigned long     lSize,        /* new size          */
    unsigned long     lWantContig   /* contiguous chunk size     */
)
{
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_ALLOCATEPAGESPACE - NOT SUPPORTED");
   return ERROR_NOT_SUPPORTED;

   psffsi = psffsi;
   psffsd = psffsd;
   lSize = lSize;
   lWantContig = lWantContig;
}

/******************************************************************
*
******************************************************************/
int far pascal __loadds  FS_DOPAGEIO(
    struct sffsi far *         psffsi,      /* ptr to fs independent SFT    */
    struct sffsd far *         psffsd,      /* ptr to fs dependent SFT      */
    struct PageCmdHeader far * pPageCmdList /* ptr to list of page commands */
)
{
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_DOPAGEIO - NOT SUPPORTED");
   return ERROR_NOT_SUPPORTED;

   psffsi = psffsi;
   psffsd = psffsd;
   pPageCmdList = pPageCmdList;
}

/******************************************************************
*
******************************************************************/
int far pascal __loadds  FS_SETSWAP(
    struct sffsi far * psffsi,      /* psffsi   */
    struct sffsd far * psffsd       /* psffsd   */
)
{

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_SETSWAP - NOT SUPPORTED");
   return ERROR_NOT_SUPPORTED;

   psffsi = psffsi;
   psffsd = psffsd;
}
