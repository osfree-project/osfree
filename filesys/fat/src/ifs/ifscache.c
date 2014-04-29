#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <dos.h>

#define INCL_DOS
#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_DOSERRORS

#include "os2.h"
#include "portable.h"
#include "fat32ifs.h"

#define NOT_USED    0xFFFFFFFF
#define MAX_SECTORS 4096
#define PAGE_SIZE   4096
#define MAX_SLOTS   0x4000
#define FREE_SLOT   0xFFFF
#define SECTORS_PER_RQ  ( PAGE_SIZE / SECTOR_SIZE )

PRIVATE volatile USHORT  usOldestEntry = 0xFFFF;
PRIVATE volatile USHORT  usNewestEntry = 0xFFFF;
PRIVATE volatile USHORT  usWaitCount = 0;
PRIVATE volatile USHORT  usRQInUse = 0;
PRIVATE ULONG    LinCacheAddr = 0;
PRIVATE SEL      rgCacheSel[(MAX_SECTORS / 128) + 1] = {0};
PRIVATE USHORT   usSelCount = 1;
PRIVATE USHORT   usPhysCount = 0;
PRIVATE USHORT   usRQCount = MAX_RQS;

PRIVATE CACHEBASE   pCacheBase[MAX_SECTORS] = {0};
PRIVATE CACHEBASE2  pCacheBase2[MAX_SECTORS] = {0};
PRIVATE BOOL        rgfDirty[MAX_SECTORS] = {0};
PRIVATE USHORT      rgSlot[MAX_SLOTS] = {0};
PRIVATE RQLIST      rgRQ[MAX_RQS] = {0};

PRIVATE BOOL   IsSectorInCache(PVOLINFO pVolInfo, ULONG ulSector, PBYTE bSector);
PRIVATE BOOL   fStoreSector(PVOLINFO pVolInfo, ULONG ulSector, PBYTE pbSector, BOOL fDirty);
PRIVATE PVOID  GetAddress(ULONG ulEntry);
PRIVATE PVOID  GetPhysAddr(PRQLIST pRQ, ULONG ulEntry);
PRIVATE BOOL   fFindSector(ULONG ulSector, BYTE bDrive, PUSHORT pusIndex);
PRIVATE USHORT WriteCacheSector(PVOLINFO pVolInfo, USHORT usCBIndex, BOOL fSetTime);
PRIVATE VOID   UpdateChain(USHORT usCBIndex);
PRIVATE VOID   vGetSectorFromCache(USHORT usCBIndex, PBYTE pbSector);
PRIVATE VOID   vReplaceSectorInCache(USHORT usCBIndex, PBYTE pbSector, BOOL fDirty);
PRIVATE VOID   LockBuffer(PCACHEBASE pBase);
PRIVATE VOID   UnlockBuffer(PCACHEBASE pBase);
PRIVATE PRQLIST GetRequestList(PVOLINFO pVolInfo, BOOL fWait);
PRIVATE BOOL   fAddToRLH(PRQLIST pRQ, USHORT usCBIndex, BYTE bPriority);
PRIVATE VOID   vCallStrategy(PVOLINFO pVolInfo, PRQLIST pRQ);
PRIVATE VOID __loadds  rlhNotify(VOID);
PRIVATE VOID __loadds  rhNotify(VOID);
PRIVATE VOID   vCheckRequest(PREQUEST pRequest);
PRIVATE USHORT usEmergencyFlush(VOID);

/******************************************************************
*
******************************************************************/
BOOL InitCache(ULONG ulSectors)
{
static BOOL fInitDone = FALSE;
ULONG ulLinCacheAddr;
PCACHEBASE pBase;
PCACHEBASE2 pBase2;
USHORT rc;
USHORT usIndex;
ULONG ulSize;
PVOID p;
PAGELIST PageList;
ULONG    linPageList;

   if (fInitDone)
      return FALSE;
   fInitDone = TRUE;

   if (!ulSectors || f32Parms.usCacheSize)
      return FALSE;

   if (ulSectors > MAX_SECTORS)
      ulSectors = MAX_SECTORS;
   Message("Allocating cache space for %ld sectors", ulSectors);

   f32Parms.usCacheSize = 0;

   /* Allocate enough selectors */

   usSelCount = (USHORT)((ulSectors * SECTOR_SIZE + 65535L ) / 65536L);

   rc = DevHelp_AllocGDTSelector(rgCacheSel, usSelCount);
   if (rc)
      {
      FatalMessage("FAT32: AllocGDTSelector failed, rc = %d", rc);
      return FALSE;
      }

   /* Allocate lineair memory */

   ulSize = ulSectors * (ULONG)sizeof (CACHE);
   ulLinCacheAddr = linalloc(ulSize, f32Parms.fHighMem, f32Parms.fHighMem);
   if (ulLinCacheAddr == 0xFFFFFFFF)
   {
      /* If tried to use high memory, try to use low memory */
      if( f32Parms.fHighMem )
      {
          f32Parms.fHighMem = FALSE;
          ulLinCacheAddr = linalloc(ulSize, f32Parms.fHighMem, f32Parms.fHighMem);
      }

      if( ulLinCacheAddr == 0xFFFFFFFF )
          return FALSE;
   }

   /* Fill the selectors */

   LinCacheAddr = ulLinCacheAddr;
   for (usIndex = 0; usIndex < usSelCount; usIndex++)
      {
      ULONG ulBlockSize = 0x10000;
      if (ulBlockSize > ulSize)
         ulBlockSize = ulSize;
      rc = DevHelp_LinToGDTSelector(rgCacheSel[usIndex],
         LinCacheAddr + (ULONG)usIndex * 0x10000, ulBlockSize);
      if (rc)
         {
         FatalMessage("FAT32: LinToGDTSelector (%d) failed, rc = %d", usIndex, rc);
         return FALSE;
         }
      ulSize -= 0x10000;
      }

   f32Parms.usCacheSize = (USHORT)ulSectors;
   f32Parms.usDirtyThreshold =
      f32Parms.usCacheSize - (f32Parms.usCacheSize / 20);

   /* initiate request lists */

   p = &PageList;
   rc = DevHelp_VirtToLin(SELECTOROF(p), OFFSETOF(p), &linPageList);
   if (rc)
      FatalMessage("FAT32:VirtToLin for pagelist failed, rc = %d", rc);

   usRQCount = (USHORT)((ULONG)MAX_RQS * ulSectors / MAX_SECTORS);
   if (usRQCount < MAX_RQS)
      usRQCount++;
   for (usIndex = 0; usIndex < usRQCount; usIndex++)
      {
      USHORT usIndex2;

      rgRQ[usIndex].usNr = usIndex;

      /* 65536 bytes */
      ulSize = ( ULONG )MAXRQENTRIES * PAGE_SIZE;

      rc = DevHelp_AllocGDTSelector(&rgRQ[usIndex].Sel, 1);
      if (rc)
         {
         FatalMessage("FAT32: AllocGDTSelector for RLH failed, rc = %d", rc);
         return FALSE;
         }

      rgRQ[usIndex].ulLin = linalloc(ulSize, f32Parms.fHighMem, f32Parms.fHighMem);
      if (rgRQ[usIndex].ulLin == 0xFFFFFFFF)
         {
            /* If tried to use high memory, try to use low memory */
            if( f32Parms.fHighMem )
            {
               f32Parms.fHighMem = FALSE;
               rgRQ[usIndex].ulLin = linalloc(ulSize, f32Parms.fHighMem, f32Parms.fHighMem);
            }

            if( rgRQ[usIndex].ulLin == 0xFFFFFFFF )
            {
               FatalMessage("FAT32: linAlloc for RLH failed");
               return FALSE;
            }
         }

      rc = DevHelp_LinToGDTSelector(rgRQ[usIndex].Sel,
         rgRQ[usIndex].ulLin, ulSize);
      if (rc)
         {
         FatalMessage("FAT32: LinToGDTSelector for RLH failed, rc = %d", rc);
         return FALSE;
         }

      usPhysCount = (USHORT)(ulSize / PAGE_SIZE) + (ulSize % PAGE_SIZE ? 1: 0);

      for (usIndex2 = 0; usIndex2 < usPhysCount; usIndex2++)
         {
         ULONG ulBlockSize = PAGE_SIZE;
         if (ulBlockSize > ulSize)
            ulBlockSize = ulSize;

         rc = DevHelp_LinToPageList(rgRQ[usIndex].ulLin + (ULONG)usIndex2 * PAGE_SIZE,
            ulBlockSize,
            linPageList,
            &ulBlockSize);
         if (rc)
            {
            FatalMessage("FAT32: LinToPageList for RLH (%d) failed, rc = %d", usIndex2, rc);
            return FALSE;
            }
         if (ulBlockSize != 1)
            {
            FatalMessage("FAT32: LinToPageList for RLH (%d) did not fill 1 entry", rc);
            return FALSE;
            }

         rgRQ[usIndex].rgPhys[usIndex2] = PageList.PhysAddr;
         ulSize -= PAGE_SIZE;
         }
      }

   pBase = pCacheBase;
   pBase2 = pCacheBase2;
   for (usIndex = 0; usIndex < f32Parms.usCacheSize; usIndex++)
      {
      pBase->ulSector = NOT_USED;
      pBase->bDrive = 0xFF;
      pBase2->usOlder = 0xFFFF;
      pBase2->usNewer = 0xFFFF;
      if (usIndex + 1 < f32Parms.usCacheSize)
         pBase->usNext = usIndex + 1;
      else
         pBase->usNext = FREE_SLOT;
      pBase++;
      pBase2++;
      }
   memset(rgSlot, 0xFF, sizeof rgSlot);
   memset(rgfDirty, FALSE, sizeof rgfDirty);

   f32Parms.usCacheUsed = 0L;

   /*
      Making sure all segments are available during interrupts
   */
   p = (PVOID)InitCache;
   rc = FSH_FORCENOSWAP(SELECTOROF(p));
   if (rc)
      FatalMessage("FAT32: FSH_FORCENOSWAP on CODE Segment failed, rc=%u", rc);

   p = (PVOID)pCacheBase;
   rc = FSH_FORCENOSWAP(SELECTOROF(p));
   if (rc)
      FatalMessage("FAT32:FSH_FORCENOSWAP on pCacheBase Segment failed, rc=%u", rc);

   p = (PVOID)&LinCacheAddr;
   rc = FSH_FORCENOSWAP(SELECTOROF(p));
   if (rc)
      FatalMessage("FAT32:FSH_FORCENOSWAP on DATA Segment failed, rc=%u", rc);

   p = (PVOID)rgRQ;
   rc = FSH_FORCENOSWAP(SELECTOROF(p));
   if (rc)
      FatalMessage("FAT32:FSH_FORCENOSWAP on rgRQ Segment failed, rc=%u", rc);

   return TRUE;
}


#define Cluster2Sector( pVolInfo, ulCluster ) \
    (( ULONG )(( pVolInfo )->ulStartOfData + \
     (( ULONG )( ulCluster ) - 2) * ( pVolInfo )->BootSect.bpb.SectorsPerCluster ))

#define Sector2Cluster( pVolInfo, ulSector ) \
    (( ULONG )((( ULONG )( ulSector ) - ( pVolInfo )->ulStartOfData ) / \
     ( pVolInfo )->BootSect.bpb.SectorsPerCluster + 2 ))

/******************************************************************
*
******************************************************************/
USHORT ReadSector(PVOLINFO pVolInfo, ULONG ulSector, USHORT nSectors, PCHAR pbData, USHORT usIOMode)
{
INT rc;
USHORT usSectors;
PSZ p;
USHORT usIndex;
PBYTE  pbSectors;
BOOL fFromCache;
BOOL fSectorInCache;
USHORT usCBIndex;

   if (ulSector + nSectors - 1 >= pVolInfo->BootSect.bpb.BigTotalSectors)
      {
      FatalMessage("FAT32: ERROR: ReadSector: Sector %ld does not exist on disk %c:",
         ulSector + nSectors - 1, pVolInfo->bDrive + 'A');
      return ERROR_SECTOR_NOT_FOUND;
      }

   f32Parms.ulTotalReads += nSectors;

   /*
      See if all sectors are in cache
   */
   fFromCache = TRUE;
   p = pbData;
   for (usIndex = 0; usIndex < nSectors; usIndex++)
      {
      if (!IsSectorInCache(pVolInfo, ulSector + usIndex, p))
         {
         fFromCache = FALSE;
         break;
         }
      p += SECTOR_SIZE;
      }
   /*
      if all sectors were found in cache, we are done
   */

   if (fFromCache)
      return 0;

#if 0
   if (f32Parms.fMessageActive & LOG_CACHE)
      {
      if (ulSector > pVolInfo->ulStartOfData)
         Message("Cluster %lu not found in cache!",
            (ulSector - pVolInfo->ulStartOfData) / pVolInfo->BootSect.bpb.SectorsPerCluster + 2);
      }
#endif
   pbSectors = NULL;
   if (( ulSector >= pVolInfo->ulStartOfData ) &&
       !(usIOMode & DVIO_OPNCACHE) && nSectors < pVolInfo->usRASectors)
      {
      usSectors = pVolInfo->usRASectors;
      
     if (ulSector + usSectors > pVolInfo->BootSect.bpb.BigTotalSectors) // ?
         usSectors = (USHORT)(pVolInfo->BootSect.bpb.BigTotalSectors - ulSector); // ?

      pbSectors = malloc(usSectors * SECTOR_SIZE);
      }

   if (!pbSectors)
      {
      pbSectors = pbData;
      usSectors = nSectors;
      }

   /* check bad cluster */
   if( ulSector >= pVolInfo->ulStartOfData )
   {
        ULONG ulStartCluster = Sector2Cluster( pVolInfo, ulSector );
        ULONG ulEndCluster = Sector2Cluster( pVolInfo, ulSector + usSectors - 1 );
        ULONG ulNextCluster = 0;
        ULONG ulCluster;

        for( ulCluster = ulStartCluster; ulCluster <= ulEndCluster; ulCluster++ )
        {
            ulNextCluster = GetNextCluster( pVolInfo, ulCluster );
            if( ulNextCluster == FAT_BAD_CLUSTER )
                break;
        }

        if( ulNextCluster == FAT_BAD_CLUSTER )
        {
            usSectors = ( ulStartCluster != ulCluster ) ?
                ( min(( USHORT )( Cluster2Sector( pVolInfo, ulCluster ) - ulSector ), usSectors )) : 0;
        }
   }

   usIOMode &= ~DVIO_OPWRITE;
   pVolInfo->ulLastDiskTime = GetCurTime();
   usWaitCount++;
   rc = FSH_DOVOLIO(DVIO_OPREAD | usIOMode, DVIO_ALLACK, pVolInfo->hVBP, pbSectors, &usSectors, ulSector);
   usWaitCount--;

   if (rc)
      {
      CritMessage("FAT32: ReadSector of sector %ld (%d sectors) failed, rc = %u",
         ulSector, usSectors, rc);
      Message("ERROR: ReadSector of sector %ld (%d sectors) failed, rc = %u",
         ulSector, usSectors, rc);
      }

   /*
      Store sector only in cache if we should
   */
   if (!rc)
      {
      p = pbSectors;
      for (usIndex = 0; usIndex < usSectors; usIndex++)
         {
         /*
            Was sector already in cache?
         */
         fSectorInCache = fFindSector(ulSector + usIndex,
            pVolInfo->bDrive,
            &usCBIndex);

         switch (fSectorInCache)
            {
            /*
               No, it wasn't. Store it if needed.
            */
            case FALSE :
               if (!(usIOMode & DVIO_OPNCACHE))
                  fStoreSector(pVolInfo, ulSector + usIndex, p, FALSE);
               break;
            case TRUE  :
            /*
               Yes it was. Get it if it was dirty since then it is different
               from version on disk.
            */
               if (rgfDirty[usCBIndex])
                  vGetSectorFromCache(usCBIndex, p);
               UnlockBuffer(pCacheBase + usCBIndex);
               break;
            }
         p += SECTOR_SIZE;
         }
      }

   if (!rc && pbSectors != pbData)
      {
      f32Parms.ulTotalRA += usSectors > nSectors ? (usSectors - nSectors) : 0;
      memcpy(pbData, pbSectors, min( usSectors, nSectors ) * SECTOR_SIZE);
      }
   if (pbSectors != pbData)
      free(pbSectors);

   return rc;
}

/******************************************************************
*
******************************************************************/
USHORT WriteSector(PVOLINFO pVolInfo, ULONG ulSector, USHORT nSectors, PCHAR pbData, USHORT usIOMode)
{
INT rc;
USHORT usSectors = nSectors;
PSZ p;
USHORT usIndex;
BOOL   fDirty;
BOOL   fSectorInCache;
USHORT usCBIndex;

   if (pVolInfo->fWriteProtected)
      return ERROR_WRITE_PROTECT;

   if (pVolInfo->fDiskClean)
      MarkDiskStatus(pVolInfo, FALSE);

   if (ulSector + nSectors - 1 >= pVolInfo->BootSect.bpb.BigTotalSectors)
      {
      FatalMessage("FAT32: ERROR: WriteSector: Sector %ld does not exist on disk %c:",
         ulSector + nSectors - 1, pVolInfo->bDrive + 'A');
      return ERROR_SECTOR_NOT_FOUND;
      }

   fDirty = TRUE;
   rc = 0;
   if (!f32Parms.fLW || (usIOMode & DVIO_OPWRTHRU) || (usIOMode & DVIO_OPNCACHE))
      {
      if (f32Parms.fLW && f32Parms.fMessageActive & LOG_CACHE)
         Message("WriteSector: Writing sector thru");
      pVolInfo->ulLastDiskTime = GetCurTime();
      usWaitCount++;
      rc = FSH_DOVOLIO(DVIO_OPWRITE | usIOMode, DVIO_ALLACK, pVolInfo->hVBP, pbData, &usSectors, ulSector);
      usWaitCount--;
      if (rc && rc != ERROR_WRITE_PROTECT)
         CritMessage("FAT32: ERROR: WriteSector sector %ld (%d sectors) failed, rc = %u",
            ulSector, nSectors, rc);
      fDirty = FALSE;
      }

   if (!rc)
      {
      p = pbData;
      for (usIndex = 0; usIndex < nSectors; usIndex++)
         {
         fSectorInCache =
            fFindSector(ulSector + usIndex, pVolInfo->bDrive, &usCBIndex);
         switch (fSectorInCache)
            {
            case FALSE :
               if (!(usIOMode & DVIO_OPNCACHE))
                  fStoreSector(pVolInfo, ulSector + usIndex, p, fDirty);
               break;
            case TRUE  :
               vReplaceSectorInCache(usCBIndex, p, fDirty);
               UnlockBuffer(pCacheBase + usCBIndex);
               break;
            }
         p += SECTOR_SIZE;
         }
      }

   return rc;
}



/******************************************************************
*
******************************************************************/
BOOL IsSectorInCache(PVOLINFO pVolInfo, ULONG ulSector, PBYTE pbSector)
{
USHORT usIndex;

   if (!fFindSector(ulSector, pVolInfo->bDrive, &usIndex))
      return FALSE;
   f32Parms.ulTotalHits++;
   vGetSectorFromCache(usIndex, pbSector);
   UnlockBuffer(pCacheBase + usIndex);
   return TRUE;
}

/******************************************************************
*
******************************************************************/
VOID vGetSectorFromCache(USHORT usCBIndex, PBYTE pbSector)
{
PCACHEBASE pBase;
PCACHE pCache;

   pBase = pCacheBase + usCBIndex;

   pCache = GetAddress(usCBIndex);
   memcpy(pbSector, pCache->bSector, SECTOR_SIZE);

   pBase->ulAccessTime = GetCurTime();
   UpdateChain(usCBIndex);

   return;
}

/******************************************************************
*
******************************************************************/
VOID UpdateChain(USHORT usCBIndex)
{
PCACHEBASE2 pBase2;

   /*
      Is entry already the newest ?
   */
   if (usNewestEntry == usCBIndex)
      return;

   pBase2 = pCacheBase2 + usCBIndex;
   /*
      Remove entry from current position in chain
   */
   if (pBase2->usOlder != 0xFFFF)
      pCacheBase2[pBase2->usOlder].usNewer = pBase2->usNewer;
   if (pBase2->usNewer != 0xFFFF)
      pCacheBase2[pBase2->usNewer].usOlder = pBase2->usOlder;

   /*
      Update the oldest if this entry was the oldest
   */

   if (usOldestEntry == 0xFFFF)
      usOldestEntry = usCBIndex;
   else if (usOldestEntry == usCBIndex)
      usOldestEntry = pBase2->usNewer;

   /*
      Update base itself
   */
   pBase2->usOlder = usNewestEntry;
   if (usNewestEntry != 0xFFFF)
      pCacheBase2[usNewestEntry].usNewer = usCBIndex;
   usNewestEntry = usCBIndex;
   pBase2->usNewer = 0xFFFF;
}

#if 0
#define WAIT_THRESHOLD
#endif
/******************************************************************
*
******************************************************************/
BOOL fStoreSector(PVOLINFO pVolInfo, ULONG ulSector, PBYTE pbSector, BOOL fDirty)
{
USHORT usSlot;
USHORT usCBIndex;
PCACHEBASE pBase;
PCACHEBASE2 pBase2;
USHORT usCount;

   if (!f32Parms.usCacheSize)
      return FALSE;

   pBase = NULL;
   if (f32Parms.usCacheUsed < f32Parms.usCacheSize)
      {
      usCBIndex = f32Parms.usCacheUsed;
      f32Parms.usCacheUsed++;

      pBase = pCacheBase + usCBIndex;
      LockBuffer(pBase);

      usSlot = (USHORT)(ulSector % MAX_SLOTS);
      pBase->usNext = rgSlot[usSlot];
      rgSlot[usSlot] = usCBIndex;

      pBase->ulSector = ulSector;
      pBase->bDrive = pVolInfo->bDrive;

      vReplaceSectorInCache(usCBIndex, pbSector, fDirty);
      UnlockBuffer(pBase);
      }
   else
      {
      PCACHEBASE pWork;
      USHORT usIndex;

      if (usOldestEntry == 0xFFFF)
         FatalMessage("FAT32: No Oldest entry found!");

      if (usRQInUse < usRQCount &&
#ifdef WAIT_THRESHOLD
          f32Parms.usDirtySectors >= f32Parms.usDirtyThreshold)
#else
          f32Parms.usDirtySectors - f32Parms.usPendingFlush > f32Parms.usDirtyThreshold)
#endif
         {
         if (f32Parms.fMessageActive & LOG_CACHE ||
             f32Parms.fMessageActive & LOG_WAIT)
            Message("ProcRun on EmergencyFlush");

         usWaitCount++;
         DevHelp_ProcRun((ULONG)DoEmergencyFlush, &usCount);
         DevHelp_TCYield();
         usWaitCount--;

         if (f32Parms.fMessageActive & LOG_CACHE ||
             f32Parms.fMessageActive & LOG_WAIT)
            Message("continuing after ProcRun...");

#ifdef WAIT_THRESHOLD
        if (f32Parms.fMessageActive & LOG_CACHE ||
            f32Parms.fMessageActive & LOG_WAIT)
            Message("waiting for dirty sectors to be less than threshold...");

        _disable();
        while( f32Parms.usDirtySectors >= f32Parms.usDirtyThreshold )
        {
             DevHelp_ProcBlock((ULONG)&f32Parms.usDirtySectors, 1000L, 1);
            _disable();
        }
        _enable();
#endif
         }

      /*
         find the oldest non dirty, not locked, not pending sector in cache
      */

fStoreRetry:

      usCBIndex = 0xFFFF;
      if (f32Parms.usDirtySectors < f32Parms.usCacheSize)
         {
         usCBIndex = usOldestEntry;
         while (usCBIndex != 0xFFFF)
            {
            if (!rgfDirty[usCBIndex])
               {
               pBase = pCacheBase + usCBIndex;
               if (!pBase->fLocked)
                  {
                  LockBuffer(pBase);
                  if (!rgfDirty[usCBIndex])
                     break;
                  UnlockBuffer(pBase);
                  }
               }
            pBase2 = pCacheBase2 + usCBIndex;
            usCBIndex = pBase2->usNewer;
            }
         }

      /*
         no available sectors and at least one pending requestlist
         lets wait for at least one requestlist to finish
      */
      if (usCBIndex == 0xFFFF && usRQInUse)
         {
         USHORT usInUse = usRQInUse;
         if (f32Parms.fMessageActive & LOG_CACHE ||
               f32Parms.fMessageActive & LOG_WAIT)
            Message("All Sectors Dirty, waiting for a requestlist for finish (%u pending flush)", f32Parms.usPendingFlush);

         usWaitCount++;
         _disable();
         while (usRQInUse == usInUse)
            {
            DevHelp_ProcBlock((ULONG)GetRequestList, 1000L, 1);
            _disable();
            }
         _enable();
         usWaitCount--;

         if (f32Parms.fMessageActive & LOG_CACHE ||
               f32Parms.fMessageActive & LOG_WAIT)
            Message("Done waiting, still %u lists in use", usRQInUse);

         goto fStoreRetry;
         }

      if (usCBIndex == 0xFFFF)
         {
         /*
            No non-dirty sector was found, thus:
            find oldest not locked, not pending entry
         */
         usCBIndex = usOldestEntry;
         usCount = 0;
            while (usCBIndex != 0xFFFF)
            {
            pBase = pCacheBase + usCBIndex;
            if (pBase->fLocked)
               usCount++;
            if (!pBase->fLocked)
               {
               LockBuffer(pBase);
               break;
               }
            pBase2 = pCacheBase2 + usCBIndex;
            usCBIndex = pBase2->usNewer;
            }
         }

      if (usCBIndex == 0xFFFF)
         {
         BOOL fMsg = f32Parms.fMessageActive;
         CritMessage("FAT32:No free sectors in cache! (run MONITOR now!)");
         Message("ERROR: fStoreSector - No sectors available!");
         Message("       %u sectors are locked", usCount);
         Message("       %u sectors are pending flush", f32Parms.usPendingFlush);
         Message("       %u sectors are dirty", f32Parms.usDirtySectors);
         f32Parms.fMessageActive = fMsg;
         Yield();
         goto fStoreRetry;
         }

      /*
         Write old entry if needed
      */

      if (rgfDirty[usCBIndex])
         {
         if (f32Parms.fMessageActive & LOG_CACHE ||
            f32Parms.fMessageActive & LOG_WAIT)
            Message("fStoreSector: PROBLEM: Oldest is dirty! (%u, %u)",
               f32Parms.usDirtySectors, f32Parms.usPendingFlush);
         WriteCacheSector(NULL, usCBIndex, TRUE);
         }

      /*
         Remove entry from slot chain
      */

      usSlot = (USHORT)(pBase->ulSector % MAX_SLOTS);
      usIndex = rgSlot[usSlot];
      if (usIndex == usCBIndex)
         rgSlot[usSlot] = pBase->usNext;
      else
         {
         pWork = NULL;
         while (usIndex != FREE_SLOT)
            {
            pWork = pCacheBase + usIndex;
            if (pWork->usNext == usCBIndex)
               break;
            usIndex = pWork->usNext;
            }
         if (usIndex == FREE_SLOT)
            FatalMessage("FAT32: Store: Oldest entry not found in slot chain!");
         pWork->usNext = pBase->usNext;
         }

      usSlot = (USHORT)(ulSector % MAX_SLOTS);
      pBase->usNext = rgSlot[usSlot];
      rgSlot[usSlot] = usCBIndex;

      pBase->ulSector = ulSector;
      pBase->bDrive = pVolInfo->bDrive;
      vReplaceSectorInCache(usCBIndex, pbSector, fDirty);
      UnlockBuffer(pBase);
      }

   return TRUE;
}

/******************************************************************
*
******************************************************************/
VOID vReplaceSectorInCache(USHORT usCBIndex, PBYTE pbSector, BOOL fDirty)
{
PCACHEBASE pBase;
PCACHE     pCache;
#ifdef WAIT_THRESHOLD
USHORT     usCount;
#endif

      pBase = pCacheBase + usCBIndex;

      if (!rgfDirty[usCBIndex] && fDirty)
         {
         f32Parms.usDirtySectors++;
         pBase->ulCreateTime = GetCurTime();
         }
      else if (rgfDirty[usCBIndex] && !fDirty)
         {
         f32Parms.usDirtySectors--;
#ifdef WAIT_THRESHOLD
         DevHelp_ProcRun(( ULONG )&f32Parms.usDirtySectors, &usCount );
#endif
         }
      rgfDirty[usCBIndex] = fDirty;
      pCache = GetAddress(usCBIndex);
      memcpy(pCache->bSector, pbSector, SECTOR_SIZE);
      pBase->ulAccessTime = GetCurTime();
      UpdateChain(usCBIndex);
      return;
}
/******************************************************************
*
******************************************************************/
PVOID GetAddress(ULONG ulEntry)
{
ULONG ulOffset;
USHORT usSel;

   ulOffset = ulEntry * sizeof (CACHE);
   usSel = (USHORT)(ulOffset / 0x10000);

   ulOffset = ulOffset % 0x10000;

   return (PVOID)MAKEP(rgCacheSel[usSel], (USHORT)ulOffset);
}

/******************************************************************
*
******************************************************************/
PVOID GetPhysAddr(PRQLIST pRQ, ULONG ulEntry)
{
   return (PVOID)(pRQ->rgPhys[ulEntry]);
}

/******************************************************************
*
******************************************************************/
BOOL fFindSector(ULONG ulSector, BYTE bDrive, PUSHORT pusIndex)
{
PCACHEBASE pBase;
USHORT    usCBIndex;

   if (!f32Parms.usCacheUsed)
      return FALSE;

   usCBIndex = rgSlot[(USHORT)(ulSector % MAX_SLOTS)];
   while (usCBIndex != FREE_SLOT)
      {
      pBase = pCacheBase + usCBIndex;
      if (pBase->ulSector == ulSector && pBase->bDrive == bDrive)
         {
         LockBuffer(pBase);
         if (pBase->ulSector == ulSector && pBase->bDrive == bDrive)
            {
            *pusIndex = usCBIndex;
            return TRUE;
            }
         UnlockBuffer(pBase);
         usCBIndex = rgSlot[(USHORT)(ulSector % MAX_SLOTS)];
         continue;
         }
      usCBIndex = pBase->usNext;
      }
   return FALSE;
}


/******************************************************************
*
******************************************************************/
USHORT WriteCacheSector(PVOLINFO pVolInfo, USHORT usCBIndex, BOOL fSetTime)
{
static BOOL fLocked = FALSE;
static BYTE bSector[SECTOR_SIZE] = "";
PCACHEBASE pBase;
USHORT   rc;
USHORT   usSectors;
USHORT   usCount;
PCACHE   pCache;

   pBase = pCacheBase + usCBIndex;
   if (!rgfDirty[usCBIndex])
   {
      if( pBase->fDiscard )
      {
         pBase->bDrive = 0xFF;
         pBase->fDiscard = OFF;
      }

      return 0;
   }

   if (!pVolInfo)
      pVolInfo = pGlobVolInfo;

   while (pVolInfo)
      {
      if (pVolInfo->bDrive == (BYTE)pBase->bDrive)
         {
         pCache = GetAddress(usCBIndex);
         usSectors = 1;
         usWaitCount++;

         _disable();
         while (fLocked)
            {
            if (f32Parms.fMessageActive & LOG_WAIT)
               Message("Waiting for WriteCacheSector!");
            DevHelp_ProcBlock((ULONG)WriteCacheSector, 1000, 1);
            _disable();
            }
         _enable();

         usWaitCount--;
         fLocked = TRUE;

         memcpy(bSector, pCache->bSector, SECTOR_SIZE);
         rc = FSH_DOVOLIO(DVIO_OPWRITE, DVIO_ALLACK,
            pVolInfo->hVBP, bSector, &usSectors, pBase->ulSector);

         fLocked = FALSE;
         DevHelp_ProcRun((ULONG)WriteCacheSector, &usCount);

         if (!rc || rc == ERROR_WRITE_PROTECT)
            {
            if( pBase->fDiscard )
            {
               pBase->bDrive = 0xFF;
               pBase->fDiscard = OFF;
            }

            rgfDirty[usCBIndex] = FALSE;
            f32Parms.usDirtySectors--;
#ifdef WAIT_THRESHOLD
            DevHelp_ProcRun(( ULONG )&f32Parms.usDirtySectors, &usCount );
#endif

            if (fSetTime)
               pVolInfo->ulLastDiskTime = GetCurTime();
            }
         else
            CritMessage("FAT32: Error %u in WriteCacheSector", rc);
         return rc;
         }
      pVolInfo = (PVOLINFO)pVolInfo->pNextVolInfo;
      }

   FatalMessage("FAT32: WriteCacheSector: VOLINFO not found!");
   return 1;
}


/******************************************************************
*
******************************************************************/
VOID DoEmergencyFlush(PLWOPTS pOptions)
{
#if 0
USHORT usIndex;
#endif
USHORT rc;

   while (!f32Parms.fInShutDown && !pOptions->fTerminate)
      {
      _disable();
      rc = WAIT_TIMED_OUT;
      while (!f32Parms.fInShutDown && !pOptions->fTerminate && rc == WAIT_TIMED_OUT)
         {
         rc = DevHelp_ProcBlock((ULONG)DoEmergencyFlush, 5000L, 1);
         _disable();
         }
      _enable();
      if (f32Parms.fInShutDown || pOptions->fTerminate)
         break;
#if 0
      for (usIndex = 0; usIndex < usRQCount; usIndex++)
         {
         if (!rgRQ[usIndex].fBusy)
            break;
         }
      if (usIndex == usRQCount)
         continue;
#endif

      usEmergencyFlush();
      FSH_IOBOOST();
      }
}

/******************************************************************
*
******************************************************************/
USHORT usFlushAll(VOID)
{
PVOLINFO pVolInfo = pGlobVolInfo;
USHORT rc;

   while (pVolInfo)
      {
      rc = usFlushVolume(pVolInfo, FLUSH_DISCARD, TRUE, PRIO_URGENT);
      pVolInfo = (PVOLINFO)pVolInfo->pNextVolInfo;
      }
   return 0;
}

/******************************************************************
*
******************************************************************/
VOID DoLW(PVOLINFO pVolInfo, PLWOPTS pOptions)
{
BYTE bPriority;
LONG lWait;

   Message("DoLW started");

   lWait = f32Parms.ulDiskIdle;
   _disable();
   while (!f32Parms.fInShutDown && !pOptions->fTerminate)
      {
      DevHelp_ProcBlock((ULONG)DoLW, lWait, 1);
      Yield();
      if ((f32Parms.usDirtySectors == f32Parms.usPendingFlush) || usWaitCount)
         {
         lWait = f32Parms.ulDiskIdle;
         _disable();
         continue;
         }
      lWait = 5;

      if (f32Parms.fLW)
         {
         switch (pOptions->bLWPrio)
            {
            case 1:
               bPriority = PRIO_LAZY_WRITE;
               break;

            case 2:
               bPriority = PRIO_BACKGROUND_USER;
               break;

            case 3:
               bPriority = PRIO_FOREGROUND_USER;
               break;

            default:
               bPriority = PRIO_PAGER_HIGH;
               break;
            }

         pVolInfo = pGlobVolInfo;
         while (pVolInfo)
            {
            ULONG ulTime = GetCurTime();

            if (ulTime < pVolInfo->ulLastDiskTime ||
                pVolInfo->ulLastDiskTime + f32Parms.ulDiskIdle <= ulTime)

               usFlushVolume(pVolInfo, FLUSH_RETAIN, FALSE, bPriority);
            pVolInfo = (PVOLINFO)pVolInfo->pNextVolInfo;
            }
         }
      _disable();
      }
   _enable();

   f32Parms.fLW = FALSE;
   usFlushAll();
   Message("DoLW Stopped, Lazy writing also");
}

#if 0
#define WAIT_PENDINGFLUSH
#endif

/******************************************************************
*
******************************************************************/
USHORT usFlushVolume(PVOLINFO pVolInfo, USHORT usFlag, BOOL fFlushAll, BYTE bPriority)
{
USHORT usCBIndex = 0;
PCACHEBASE pBase;
PCACHEBASE2 pBase2;
USHORT usCount;
ULONG  ulCurTime = GetCurTime();
PRQLIST pRQ;

   if (!f32Parms.usCacheUsed )
      return 0;

   if( usFlag == FLUSH_DISCARD )
   {
      /* Wait to flush pending dirty sectors completely */
#ifndef WAIT_PENDINGFLUSH
      _disable();
      while ( usRQInUse )
      {
         DevHelp_ProcBlock((ULONG)GetRequestList, 1000L, 1);
         _disable();
      }
      _enable();
#else
      _disable();
      while( f32Parms.usPendingFlush )
      {
         DevHelp_ProcBlock((ULONG)&f32Parms.usPendingFlush, 1000L, 1);
         _disable();
      }
      _enable();
#endif
   }
   else if( f32Parms.usDirtySectors == f32Parms.usPendingFlush )
      return 0;

   usCount = 0;

   if (!fFlushAll)
      {
      pRQ = NULL;
      usCBIndex = usOldestEntry;
      while (usCBIndex != 0xFFFF && !usWaitCount)
         {
         if (pRQ && pRQ->rlh.Count >= MAXRQENTRIES)
            break;

         pBase2 = pCacheBase2 + usCBIndex;
         pBase  = pCacheBase + usCBIndex;

         if (pBase->bDrive == pVolInfo->bDrive &&
             (ulCurTime - pBase->ulCreateTime >= f32Parms.ulMaxAge ||
              ulCurTime - pBase->ulAccessTime >= f32Parms.ulBufferIdle))
            {
            if( rgfDirty[ usCBIndex ] )
               {
               if (!pBase->fLocked && !pBase->fFlushPending)
                  {
                  if (!pRQ)
                     pRQ = GetRequestList(pVolInfo, TRUE);

                  LockBuffer(pBase);
                  if( usFlag == FLUSH_DISCARD )
                     pBase->fDiscard = SET;
                  if (!pRQ || !fAddToRLH(pRQ, usCBIndex, bPriority))
                     {
                     WriteCacheSector(pVolInfo, usCBIndex, FALSE);
                     UnlockBuffer(pBase);
                     }
                  usCount++;
                  }
               }
            else if( usFlag == FLUSH_DISCARD )
               {
                  LockBuffer( pBase );
                  pBase->bDrive = 0xFF;
                  pBase->fDiscard = OFF;
                  UnlockBuffer( pBase );
               }
            }

         usCBIndex = pBase2->usNewer;
         }
      if (f32Parms.fMessageActive & LOG_CACHE && usCount > 0)
         Message("%u sectors LAZY flushed, still %u dirty", usCount, f32Parms.usDirtySectors);

      if (pRQ)
         vCallStrategy(pVolInfo, pRQ);
      return 0;
      }

   if (f32Parms.fMessageActive & LOG_CACHE)
      Message("usFlushVolume ALL");

   pRQ = GetRequestList(pVolInfo, TRUE);
   usCBIndex = 0;
   for (usCBIndex = 0; ( f32Parms.usDirtySectors || usFlag == FLUSH_DISCARD ) &&
         usCBIndex < f32Parms.usCacheUsed; usCBIndex++)
      {
      pBase = pCacheBase + usCBIndex;
      if (pVolInfo->bDrive == (BYTE)pBase->bDrive)
         {
         if( rgfDirty[usCBIndex] )
            {
            if (pRQ && pRQ->rlh.Count >= MAXRQENTRIES)
               {
               vCallStrategy(pVolInfo, pRQ);
               pRQ = GetRequestList(pVolInfo, TRUE);
               }

            LockBuffer(pBase);
            if( usFlag == FLUSH_DISCARD )
               pBase->fDiscard = SET;
            if (!pRQ || !fAddToRLH(pRQ, usCBIndex, PRIO_URGENT))
               {
               WriteCacheSector(pVolInfo, usCBIndex, FALSE);
               UnlockBuffer(pBase);
               }
            usCount++;
            }
         else if( usFlag == FLUSH_DISCARD )
            {
               LockBuffer( pBase );
               pBase->bDrive = 0xFF;
               pBase->fDiscard = OFF;
               UnlockBuffer( pBase );
            }
         }
      }
   vCallStrategy(pVolInfo, pRQ);

   if (f32Parms.fMessageActive & LOG_CACHE)
      Message("%u sectors flushed, still %u dirty", usCount, f32Parms.usDirtySectors);

   return 0;
}

/******************************************************************
*
******************************************************************/
USHORT usEmergencyFlush(VOID)
{
USHORT usCBIndex = 0;
PCACHEBASE pBase;
PCACHEBASE2 pBase2;
USHORT usCount;
PVOLINFO pVolInfo;
PRQLIST pRQ;

   if (!f32Parms.usCacheUsed || (f32Parms.usDirtySectors == f32Parms.usPendingFlush))
      return 0;

   /*
      find the volinfo for the oldest entry
   */
   pVolInfo = NULL;
   usCBIndex = usOldestEntry;
   while (usCBIndex != 0xFFFF)
      {
      pBase2 = pCacheBase2 + usCBIndex;
      pBase  = pCacheBase + usCBIndex;
      if (rgfDirty[usCBIndex] && !pBase->fFlushPending)
         {
         pVolInfo = pGlobVolInfo;
         while (pVolInfo)
            {
            if ((BYTE)pBase->bDrive == pVolInfo->bDrive)
               break;
            pVolInfo = (PVOLINFO)(pVolInfo->pNextVolInfo);
            }
         if (!pVolInfo)
            //FatalMessage("FAT32: Drive not found in emergency flush!");
            return 0; //
         break;
         }
      usCBIndex = pBase2->usNewer;
      }

   if (usCBIndex == 0xFFFF)
      return 0;

   if (!pVolInfo)
      return 0;

   pRQ = GetRequestList(pVolInfo, TRUE);
   if (!pRQ)
      return 0;

   usCount = 0;
   while (usCBIndex != 0xFFFF && usCount < MAXRQENTRIES)
      {
      pBase2 = pCacheBase2 + usCBIndex;
      pBase  = pCacheBase + usCBIndex;
      if (rgfDirty[usCBIndex] &&
         pVolInfo->bDrive == (BYTE)pBase->bDrive &&
         !pBase->fLocked && !pBase->fFlushPending)
         {
         LockBuffer(pBase);
         fAddToRLH(pRQ, usCBIndex, PRIO_URGENT);
         usCount++;
         }
      usCBIndex = pBase2->usNewer;
      }

   pVolInfo->ulLastDiskTime = GetCurTime();

   if (f32Parms.fMessageActive & LOG_CACHE)
      Message("usEmergencyFlush: %u sectors flushed, still %u dirty", usCount, f32Parms.usDirtySectors);

   vCallStrategy(pVolInfo, pRQ);

   return 0;
}

/******************************************************************
*
******************************************************************/
VOID LockBuffer(PCACHEBASE pBase)
{
   _disable();
   usWaitCount++;
   while (pBase->fLocked)
      {
      if (f32Parms.fMessageActive & LOG_WAIT)
         Message("Waiting on locked buffer...");
      DevHelp_ProcBlock((ULONG)pBase, 1000, 1);
      _disable();
      if (!pBase->fLocked && f32Parms.fMessageActive & LOG_WAIT)
         Message("Continuing after wait for locked buffer");
      }
   usWaitCount--;
   pBase->fLocked = SET;
   _enable();
}

/******************************************************************
*
******************************************************************/
VOID UnlockBuffer(PCACHEBASE pBase)
{
USHORT usCount;

   pBase->fLocked = OFF;
   DevHelp_ProcRun((ULONG)pBase, &usCount);
}

/******************************************************************
*
******************************************************************/
PRQLIST GetRequestList(PVOLINFO pVolInfo, BOOL fWait)
{
PRQLIST pRQ;
USHORT  usIndex;

   if (!pVolInfo->pfnStrategy)
      return NULL;

   pRQ = NULL;

   usWaitCount++;
   _disable();
   while (!pRQ)
      {
      for (usIndex = 0; usIndex < usRQCount; usIndex++)
         {
         if (!rgRQ[usIndex].fBusy)
            {
            pRQ = &rgRQ[usIndex];
            pRQ->usNr = usIndex;
            break;
            }
         }

      if (!pRQ)
         {
         if (fWait)
            {
            if (f32Parms.fMessageActive & LOG_WAIT)
               Message("Waiting for a requestlist to become available");
            DevHelp_ProcBlock((ULONG)GetRequestList, 1000L, 1);
            _disable();
            }
         else
            {
            _enable();
            if (f32Parms.fMessageActive & LOG_CACHE)
               Message("No RQ Buffers free!");
            return NULL;
            }
         }
      }
   _enable();
   usWaitCount--;

   memset(&pRQ->rlh, 0, sizeof pRQ->rlh);
   memset(pRQ->rgReq, 0, sizeof pRQ->rgReq);
   pRQ->fBusy = TRUE;
   usRQInUse++;
   pRQ->rlh.Count = 0;
   pRQ->rlh.Notify_Address  = (PVOID)rlhNotify;
   pRQ->rlh.Request_Control = RLH_Notify_Done;
   pRQ->rlh.Block_Dev_Unit  = pVolInfo->bUnit;
   return pRQ;
}

/******************************************************************
*
******************************************************************/
BOOL fAddToRLH(PRQLIST pRQin, USHORT usCBIndex, BYTE bPriority)
{
_segment rqBase;
RQLIST  _based(rqBase) * pRQ;
RLH     _based(rqBase) * pRLH;
PB      _based(rqBase) * pPB;
SG      _based(rqBase) * pSG;
PCACHEBASE pBase;
USHORT usEntry;
PBYTE pTar;
PCACHE pCache;

   if (!pRQin)
      return FALSE;

   rqBase = SELECTOROF(pRQin);
   pRQ = (RQLIST _based(rqBase) *)OFFSETOF(pRQin);

   pBase = pCacheBase + usCBIndex;

   /* Find a free entry to add a sector */
   for( usEntry = 0; usEntry < ( USHORT )pRQin->rlh.Count; usEntry++ )
   {
      pPB = (PB _based(rqBase) *)&pRQ->rgReq[ usEntry ].pb;
      if( pPB->Block_Count < SECTORS_PER_RQ &&
          pPB->Start_Block + pPB->Block_Count == pBase->ulSector )
        break;
   }

   if( usEntry == ( USHORT )pRQin->rlh.Count && pRQin->rlh.Count == MAXRQENTRIES )
      return FALSE;

   pRLH = (RLH _based(rqBase) *)&pRQ->rlh;
   pPB  = (PB _based(rqBase) *)&pRQ->rgReq[usEntry].pb;
   pSG  = (SG _based(rqBase) *)&pRQ->rgReq[usEntry].sg;

   pRQ->rgReq[usEntry].rgCBIndex[ pPB->Block_Count ] = usCBIndex;

if( usEntry == ( USHORT )pRQ->rlh.Count )
{
   pPB->RqHdr.Length        = RH_LAST_REQ;
   pPB->RqHdr.Old_Command   = PB_REQ_LIST;
   pPB->RqHdr.Command_Code  = PB_WRITE_X;
   pPB->RqHdr.Head_Offset   = 0;
/*   pPB->RqHdr.Req_Control   = RH_NOTIFY_ERROR | RH_NOTIFY_DONE; */
   pPB->RqHdr.Req_Control   = RH_NOTIFY_ERROR;
   pPB->RqHdr.Priority      = bPriority;
   pPB->RqHdr.Status        = RH_NOT_QUEUED;
   pPB->RqHdr.Error_Code    = 0;
   pPB->RqHdr.Notify_Address = (PVOID)rhNotify;
   pPB->RqHdr.Hint_Pointer   = 0xFFFFFFFF;

   pPB->Start_Block    = pBase->ulSector;
   pPB->Block_Count    = 1;
   pPB->Blocks_Xferred = 0;
   pPB->RW_Flags       = RW_Cache_Req;
   pPB->SG_Desc_Count  = 1;

   pSG->BufferPtr = GetPhysAddr(pRQ, pRQ->rlh.Count);
   pSG->BufferSize = SECTOR_SIZE;

   pRQ->rlh.Count++;
}
else
{
   pPB->Block_Count++;
   pSG->BufferSize += SECTOR_SIZE;
}

   pTar = MAKEP(pRQ->Sel, usEntry * PAGE_SIZE + ( pPB->Block_Count - 1 ) * SECTOR_SIZE);
   pCache = GetAddress(usCBIndex);
   memcpy(pTar, pCache->bSector, SECTOR_SIZE);

   pBase->fFlushPending = SET;
   f32Parms.usPendingFlush++;

   UnlockBuffer(pBase);

   return TRUE;
}

#pragma optimize("g", off)
/******************************************************************
*
******************************************************************/
VOID vCallStrategy(PVOLINFO pVolInfo, PRQLIST pRQin)
{
STRATFUNC pfnStrategy;
USHORT usSeg;
USHORT usOff;
ULONG  ulIndex;

_segment rqBase;
RQLIST _based(rqBase) * pRQ;
RLH    _based(rqBase) * pRLH;
PB     _based(rqBase) * pPB;

   if (!pRQin)
      return;

   rqBase = SELECTOROF(pRQin);
   pRQ = (RQLIST _based(rqBase) *)OFFSETOF(pRQin);

   if (f32Parms.fMessageActive & LOG_CACHE && pRQ->rlh.Count)
      Message("vCallStrategy drive %c:, %lu sectors, RQ %u",
         pRQ->rlh.Block_Dev_Unit + 'A',
         pRQ->rlh.Count,
         pRQ->usNr);

   if (!pRQ->rlh.Count)
      {
      USHORT usCount;

      pRQ->fBusy = FALSE;
      usRQInUse--;
      DevHelp_ProcRun((ULONG)GetRequestList, &usCount);
      return;
      }

   pRLH = (RLH _based(rqBase) *)&pRQ->rlh;
   if (pRQ->rlh.Count == 1)
      pRQ->rlh.Lst_Status |= RLH_Single_Req;

   for (ulIndex = 0; ulIndex < pRQ->rlh.Count; ulIndex++)
      {
      pPB = (PB _based(rqBase) *)&pRQ->rgReq[ulIndex].pb;

      if (ulIndex + 1 < pRQ->rlh.Count)
         pPB->RqHdr.Length = sizeof (REQUEST);
      pPB->RqHdr.Head_Offset   = (USHORT)pPB - (USHORT)pRLH;
      }

   usSeg = rqBase;
   usOff = (USHORT)pRQ + offsetof(RQLIST, rlh);

   pfnStrategy = pVolInfo->pfnStrategy;
   _asm push  es; // vs
   //_asm push  bx; //

   _asm mov es, usSeg;
   _asm mov bx, usOff;
   (*pfnStrategy)();

   //_asm pop   bx; // vs
   _asm pop   es; //
}

/******************************************************************
*
******************************************************************/
VOID __loadds rlhNotify(VOID)
{
PRQLIST pRQ;
USHORT usCount;
WORD ESReg, BXReg;
INT iStatus, iErrorStatus;
ULONG ulIndex, ulIndex2;
PREQUEST pRequest;

   _asm push es; // vs
   //_asm push bx; //

   _asm mov ESReg, es
   _asm mov BXReg, bx

   pRQ = MAKEP(ESReg, BXReg - offsetof(RQLIST, rlh));

   iStatus      = pRQ->rlh.Lst_Status & 0x0F;
   iErrorStatus = pRQ->rlh.Lst_Status & 0xF0;

   if (iStatus == RLH_All_Req_Done)
      {
      switch (iErrorStatus)
         {
         case RLH_No_Error  :
         case RLH_Rec_Error :
            pRequest = pRQ->rgReq;
            for (ulIndex = 0; ulIndex < pRQ->rlh.Count; ulIndex++)
               {
               if (pRequest->pb.RqHdr.Status & 0x0F != RH_DONE)
                  FatalMessage("FAT32: rlhNotify received with not all sectors being done!");

         for( ulIndex2 = 0; ulIndex2 < pRequest->pb.Blocks_Xferred; ulIndex2++ )
            {
               PCACHEBASE pBase = pCacheBase + pRequest->rgCBIndex[ ulIndex2 ];

               if (pBase->ulSector >= pRequest->pb.Start_Block &&
                   pBase->ulSector < pRequest->pb.Start_Block + pRequest->pb.Blocks_Xferred )
                  {
                  if (rgfDirty[pRequest->rgCBIndex[ ulIndex2 ]])
                     {
                     if( pBase->fDiscard )
                        {
                        pBase->bDrive = 0xFF;
                        pBase->fDiscard = OFF;
                        }

                     rgfDirty[pRequest->rgCBIndex[ ulIndex2 ]] = FALSE;
                     f32Parms.usDirtySectors--;
#ifdef WAIT_THRESHOLD
                     DevHelp_ProcRun(( ULONG )&f32Parms.usDirtySectors, &usCount );
#endif
                     }
                  if (pBase->fFlushPending)
                     {
                     f32Parms.usPendingFlush--;
#ifdef WAIT_PENDINGFLUSH
                     DevHelp_ProcRun((ULONG)&f32Parms.usPendingFlush, &usCount );
#endif
                     pBase->fFlushPending = OFF;
                     }
                  }
            }

               pRequest++;
               }
            break;

         default :
            CritMessage("FAT32: Fatal WRITE Error %X in rlhNotify!", iErrorStatus);
            break;
         }

      pRQ->fBusy = FALSE;
      usRQInUse--;
      DevHelp_ProcRun((ULONG)GetRequestList, &usCount);
      return;
      }

   switch (iErrorStatus)
      {
      case RLH_No_Error  :
      case RLH_Rec_Error :
         return;

      default            :
         CritMessage("FAT32: Fatal WRITE Error %X in rlhNotify!", iErrorStatus);
         pRQ->fBusy = FALSE;
         usRQInUse--;
         DevHelp_ProcRun((ULONG)GetRequestList, &usCount);
         break;
      }

   //_asm pop  bx; // vs
   _asm pop  es; //

   return;
}


/******************************************************************
*
******************************************************************/
VOID __loadds rhNotify(VOID)
{
WORD ESReg, BXReg;

   _asm push es // vs
   //_asm push bx //

   _asm mov ESReg, es
   _asm mov BXReg, bx

   vCheckRequest(MAKEP(ESReg, BXReg));

   //_asm pop  bx; // vs
   _asm pop  es; //
}
#pragma optimize("", on)

/******************************************************************
*
******************************************************************/
VOID vCheckRequest(PREQUEST pRequest)
{
PCACHEBASE pBase;
INT status;
INT err_status;
INT err_code;
#ifdef WAIT_THRESHOLD
USHORT usCount;
#endif
ULONG ulIndex;

for( ulIndex = 0; ulIndex < pRequest->pb.Blocks_Xferred; ulIndex++ )
   {
   if (pRequest->rgCBIndex[ ulIndex ] > f32Parms.usCacheUsed)
      InternalError("FAT32: usCBIndex is wrong in vCheckRequest!");

   pBase = pCacheBase + pRequest->rgCBIndex[ ulIndex ];

   if (pBase->ulSector < pRequest->pb.Start_Block ||
       pBase->ulSector >= pRequest->pb.Start_Block + pRequest->pb.Blocks_Xferred )
      {
      CritMessage("FAT32: Sectors do not match in rhNotify!");
      return;
      }

   status         = pRequest->pb.RqHdr.Status & 0x0F;
   err_status     = pRequest->pb.RqHdr.Status & 0xF0;
   err_code       = pRequest->pb.RqHdr.Error_Code;

   if (status == RH_DONE)
      {
      switch (err_status)
         {
         case RH_NO_ERROR    :
         case RH_RECOV_ERROR :
            if( pBase->fDiscard )
            {
               pBase->bDrive = 0xFF;
               pBase->fDiscard = OFF;
            }

            rgfDirty[pRequest->rgCBIndex[ ulIndex ]] = FALSE;
            f32Parms.usDirtySectors--;
#ifdef WAIT_THRESHOLD
            DevHelp_ProcRun(( ULONG )&f32Parms.usDirtySectors, &usCount );
#endif
            break;
         default             :
            CritMessage("FAT32: Error %X in rhNotify!", err_code);
            break;
         }

      if (pBase->fFlushPending)
         {
         f32Parms.usPendingFlush--;
#ifdef WAIT_PENDINGFLUSH
         DevHelp_ProcRun((ULONG)&f32Parms.usPendingFlush, &usCount );
#endif
         pBase->fFlushPending = OFF;
         }
      continue;
      }

   switch (err_status)
      {
      /*
         In this case we should be notified later of the real status of the request.
      */
      case RH_NO_ERROR    :
      case RH_RECOV_ERROR :
         break;
      /*
         In this case there is no more chance ...
      */
      default :
         if (pBase->fFlushPending)
            {
            f32Parms.usPendingFlush--;
#ifdef WAIT_PENDINGFLUSH
            DevHelp_ProcRun((ULONG)&f32Parms.usPendingFlush, &usCount );
#endif
            pBase->fFlushPending = OFF;
            }

         CritMessage("FAT32: Error %X in rhNotify!", err_code);
         break;
      }
   }
}

