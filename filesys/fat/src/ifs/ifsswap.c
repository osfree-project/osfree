#include <stdio.h>
#include <stddef.h>
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

VOID vCallStrategy2(PVOLINFO pVolInfo, RQLIST far *pRQ);
ULONG PositionToOffset(PVOLINFO pVolInfo, POPENINFO pOpenInfo, ULONG ulOffset);
int GetBlockNum(PVOLINFO pVolInfo, POPENINFO pOpenInfo, ULONG ulOffset, PULONG pulBlkNo);

#define PSIZE           4096
#define MAXPGREQ	8

RQLIST pgreq;

//static ULONG ulSemRWSwap = 0;


/******************************************************************
*
******************************************************************/
int far pascal _loadds FS_OPENPAGEFILE (
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
   int             rc;
   USHORT          dummyAction, dummyFlag;
   struct cdfsi    dummyCds;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_OPENPAGEFILE  pName=%s OpenMode=%x, OpenFlag=%x Attr=%x",
              pName, OpenMode, OpenFlag, Attr);

   _asm push es;

   /* Keep track of volume with swap-space.  We can't allow this volume
    * to be quiesced.
    */
   ////page_hVPB = psffsi->sfi_hVPB;

   /* pathlookup needs the hVPB in the current directory structure
    * to figure out where to start.  conjure up a cds with just the
    * needed information.
    */
   dummyCds.cdi_hVPB = psffsi->sfi_hVPB;

   /* do a regular open or create
    */
   rc = FS_OPENCREATE(&dummyCds, NULL, pName, -1,
                      psffsi, psffsd, OpenMode, OpenFlag,
                      &dummyAction, Attr, NULL, &dummyFlag);

   if (rc == 0)
   {
       /* set return information:
        *   pageio requests require physical addresses;
        *   maximum request is 16 pages;
        */
       *pFlags = PGIO_VADDR; //PGIO_PADDR;
       *pcMaxReq = 0; //MAXPGREQ;
   }

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_OPENPAGEFILE returned %u\n", rc);

   _asm pop es;

   return rc;
}


/******************************************************************
*
******************************************************************/
int far pascal _loadds FS_ALLOCATEPAGESPACE(
    struct sffsi far *psffsi,       /* ptr to fs independent SFT */
    struct sffsd far *psffsd,       /* ptr to fs dependent SFT   */
    unsigned long     lSize,        /* new size          */
    unsigned long     lWantContig   /* contiguous chunk size     */
)
{
   int rc;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_ALLOCATEPAGESPACE  size=%lu contig=%lu", lSize, lWantContig);

   _asm push es;

   rc = FS_NEWSIZE (psffsi, psffsd, lSize, 0x10);

   if (rc == 0)
       psffsi->sfi_size = lSize;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_ALLOCATEPAGESPACE returned %u\n", rc);

   _asm pop es;

   return rc;
}

/******************************************************************
*
******************************************************************/
int far pascal _loadds FS_DOPAGEIO(
    struct sffsi far *         psffsi,      /* ptr to fs independent SFT    */
    struct sffsd far *         psffsd,      /* ptr to fs dependent SFT      */
    struct PageCmdHeader far * pPageCmdList /* ptr to list of page commands */
)
{
   POPENINFO       pOpenInfo;
   PVOLINFO        pVolInfo;
   ULONG           ulStartCluster;
   ULONG           fsbno;  /* starting block number to read/write */
   struct PageCmd  *pgcmd; /* pointer to current command in list */
   Req_List_Header *rlhp;  /* pointer to request list header */
   Req_Header      *rhp;   /* pointer to request header */
   PB_Read_Write   *rwp;   /* pointer to request */
   int i, rc = NO_ERROR;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_DOPAGEIO\n");

   _asm push es;

   //FSH_SEMREQUEST(&ulSemRWSwap, TO_INFINITE);

   pVolInfo = GetVolInfo(psffsi->sfi_hVPB); 
   pOpenInfo = GetOpenInfo(psffsd);
   //ulStartCluster = pOpenInfo->pSHInfo->ulStartCluster;

   memset(&pgreq, 0, sizeof(pgreq));

   rlhp = &pgreq.rlh;
   rlhp->Count = pPageCmdList->OpCount;
   rlhp->Notify_Address = 0; //(PVOID)rlhNotify;
   rlhp->Request_Control = 0; //(pPageCmdList->InFlags & PGIO_FI_ORDER)
                            //? RLH_Notify_Done | RLH_Exe_Req_Seq
                            //: RLH_Notify_Done;
   rlhp->Block_Dev_Unit = pVolInfo->bUnit;

   for (i = 0, pgcmd = pPageCmdList->PageCmdList;
        i < pPageCmdList->OpCount;
        i++, pgcmd++)
      {
      /* Fill in request header.
       * These fields are set to zero by memset, above:
       *	rhp->Req_Control
       *	rhp->Status
       *	rwp->RW_Flags
       */
       rhp = &pgreq.rgReq[i].pb.RqHdr;
       rhp->Length = sizeof(REQUEST);
       //rhp->Req_Control = RH_NOTIFY_ERROR; // | RH_NOTIFY_DONE;
       rhp->Old_Command = PB_REQ_LIST;
       rhp->Command_Code = pgcmd->Cmd;
       rhp->Head_Offset = (ULONG)rhp - (ULONG)rlhp;
       rhp->Priority = pgcmd->Priority;
       rhp->Hint_Pointer = -1;
       /* Fill in read/write request.
        */
       rwp = &pgreq.rgReq[i].pb;
       rc = GetBlockNum(pVolInfo, pOpenInfo, pgcmd->FileOffset, &fsbno);
       if (rc)
          {
          /* request is not valid, return error */
          goto FS_DOPAGEIO_EXIT;
          }
       rwp->Start_Block = fsbno;
       rwp->Block_Count = PSIZE >> 9;
       rwp->SG_Desc_Count = 1;

       /* Fill in the scatter/gather descriptor
        */
       pgreq.rgReq[i].sg.BufferPtr = (void *)pgcmd->Addr;
       pgreq.rgReq[i].sg.BufferSize = PSIZE;
       }

   /* Length in last request must be set to terminal value.
    */
   rhp->Length = RH_LAST_REQ;

   vCallStrategy2(pVolInfo, &pgreq);

   /* Check for errors and update status info in the command list.
    * Set return value to error code from first failing command.
    */
   rc = 0;
   for (i = 0; i < pPageCmdList->OpCount; i++)
      {
      pgcmd = &pPageCmdList->PageCmdList[i];
      rhp = &pgreq.rgReq[i].pb.RqHdr;

      pgcmd->Status = rhp->Status;
      pgcmd->Error = rhp->Error_Code;
      if ((rc == 0) && (pgcmd->Error != 0))
         rc = pgcmd->Error;
      }

FS_DOPAGEIO_EXIT:
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_DOPAGEIO returned %u\n", rc);

   //FSH_SEMCLEAR(&ulSemRWSwap);

   _asm pop  es;

   return rc;
}

/******************************************************************
*
******************************************************************/
int far pascal _loadds FS_SETSWAP(
    struct sffsi far * psffsi,      /* psffsi   */
    struct sffsd far * psffsd       /* psffsd   */
)
{
   int rc = NO_ERROR;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_SETSWAP\n");

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_SETSWAP returned %u\n", rc);

   return rc;
}

/******************************************************************
*
******************************************************************/
int GetBlockNum(PVOLINFO pVolInfo, POPENINFO pOpenInfo, ULONG ulOffset, PULONG pulBlkNo)
{
   ULONG ulCluster;
   //ULONG ulClusterSize = pVolInfo->usClusterSize;
   //int i;

   // get cluster at the ulOffset offset
   /* for (i = 0, ulCluster = ulStartCluster;
        i < ulOffset / ulClusterSize + 1;
        i++, ulCluster = GetNextCluster(pVolInfo, ulCluster))
      {
      if (ulCluster == FAT_EOF)
          return ERROR_SECTOR_NOT_FOUND;
      } */

   // get cluster number from file offset
   ulCluster = PositionToOffset(pVolInfo, pOpenInfo, ulOffset);

   if (ulCluster == FAT_EOF)
       return ERROR_SECTOR_NOT_FOUND;

   *pulBlkNo = pVolInfo->BootSect.bpb.HiddenSectors + pVolInfo->ulStartOfData +
      (ulCluster - 2) * pVolInfo->BootSect.bpb.SectorsPerCluster;

   return NO_ERROR;
}

/******************************************************************
*
******************************************************************/
VOID vCallStrategy2(PVOLINFO pVolInfo, RQLIST far *pRQ)
{
   STRATFUNC pfnStrategy;
   USHORT usSeg;
   USHORT usOff;
   ULONG  ulIndex;
   RLH *pRLH;
   PB  *pPB;

   if (!pRQ)
      return;

   _asm push es; // vs
   _asm push bx; //

   if (f32Parms.fMessageActive & LOG_FS && pRQ->rlh.Count)
      Message("vCallStrategy2 drive %c:, %lu sectors, RQ %u",
         pRQ->rlh.Block_Dev_Unit + 'A',
         pRQ->rlh.Count,
         pRQ->usNr);

   pRLH = (RLH *)&pRQ->rlh;

   if (pRQ->rlh.Count == 1)
      pRQ->rlh.Lst_Status |= RLH_Single_Req;

   for (ulIndex = 0; ulIndex < pRQ->rlh.Count; ulIndex++)
      {
      pPB = (PB *)&pRQ->rgReq[ulIndex].pb;

      if (ulIndex + 1 < pRQ->rlh.Count)
         pPB->RqHdr.Length = sizeof (REQUEST);

      pPB->RqHdr.Head_Offset   = (USHORT)pPB - (USHORT)pRLH;
      }

   usSeg = SELECTOROF(pRQ);
   usOff = (USHORT)pRQ + offsetof(RQLIST, rlh);

   pfnStrategy = pVolInfo->pfnStrategy;

   _asm mov es, usSeg;
   _asm mov bx, usOff;

   (*pfnStrategy)();

   _asm pop   bx; // vs
   _asm pop   es; //
}
