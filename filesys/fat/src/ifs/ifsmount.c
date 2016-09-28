#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define INCL_DOS
#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_DOSERRORS

#include "os2.h"
#include "portable.h"
#include "fat32ifs.h"


static BOOL RemoveVolume(PVOLINFO pVolInfo);
static USHORT CheckWriteProtect(PVOLINFO);
PRIVATE BOOL IsFAT32(PBOOTSECT pBoot);

int far pascal _loadds FS_MOUNT(unsigned short usFlag,      /* flag     */
                        struct vpfsi far * pvpfsi,      /* pvpfsi   */
                        struct vpfsd far * pvpfsd,      /* pvpfsd   */
                        unsigned short hVBP,        /* hVPB     */
                        char far *  pBoot       /* pBoot    */)
{
PBOOTSECT pSect;
PVOLINFO  pVolInfo;
PVOLINFO  pNext, pPrev;
USHORT usVolCount;
USHORT hDupVBP;
USHORT rc;
P_DriverCaps pDevCaps;
P_VolChars   pVolChars;

   _asm push es;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_MOUNT for %c (%d):, flag = %d",
         pvpfsi->vpi_drive + 'A',
         pvpfsi->vpi_unit,
         usFlag);

   switch (usFlag)
      {
      case MOUNT_MOUNT  :

        pSect = (PBOOTSECT)pBoot;
/*
        if (!IsFAT32((BOOTSECT far *)pBoot))
           {
           rc = ERROR_VOLUME_NOT_MOUNTED;
           goto FS_MOUNT_EXIT;
           }
 */
        if (FSH_FINDDUPHVPB(hVBP, &hDupVBP))
           hDupVBP = 0;

        if (memicmp(pSect->FileSystem, "FAT32", 5))
            {
            rc = ERROR_VOLUME_NOT_MOUNTED;
            goto FS_MOUNT_EXIT;
            }

        if (pSect->bpb.BytesPerSector != SECTOR_SIZE)
            {
            rc = ERROR_VOLUME_NOT_MOUNTED;
            goto FS_MOUNT_EXIT;
            }
/*
        if(( ULONG )pSect->bpb.BytesPerSector * pSect->bpb.SectorsPerCluster > MAX_CLUSTER_SIZE )
            {
            rc = ERROR_VOLUME_NOT_MOUNTED;
            goto FS_MOUNT_EXIT;
            }
 */
    if (!hDupVBP)   /* initial mounting of the volume */
//        if(( ULONG )pSect->bpb.BytesPerSector * pSect->bpb.SectorsPerCluster > MAX_CLUSTER_SIZE )
        {
        pVolInfo = gdtAlloc(STORAGE_NEEDED, FALSE);

        if (!pVolInfo)
            {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto FS_MOUNT_EXIT;
            }
        rc = FSH_FORCENOSWAP(SELECTOROF(pVolInfo));
        if (rc)
            {
            FatalMessage("FSH_FORCENOSWAP on VOLINFO Segment failed, rc=%u", rc);
            rc = ERROR_GEN_FAILURE;
            goto FS_MOUNT_EXIT;
            }

        *((PVOLINFO *)(pvpfsd->vpd_work)) = pVolInfo;
        memset(pVolInfo, 0, (size_t)STORAGE_NEEDED);
        pVolInfo->pNextVolInfo = NULL;

//        pvpfsi->vpi_vid    = pSect->ulVolSerial;
//        pvpfsi->vpi_bsize  = pSect->bpb.BytesPerSector;
//        pvpfsi->vpi_totsec = pSect->bpb.BigTotalSectors;
//        pvpfsi->vpi_trksec = pSect->bpb.SectorsPerTrack;
//        pvpfsi->vpi_nhead  = pSect->bpb.Heads;
//        memset(pvpfsi->vpi_text, 0, sizeof pvpfsi->vpi_text);
//        memcpy(pvpfsi->vpi_text, pSect->VolumeLabel, sizeof pSect->VolumeLabel);

            if (!pGlobVolInfo)
               {
               pGlobVolInfo = pVolInfo;
               }
            else
               {
               pNext = pGlobVolInfo;
               while(pNext->pNextVolInfo)
                  {
                  pNext = pNext->pNextVolInfo;
                  }
               pNext->pNextVolInfo = pVolInfo;
               }
                pvpfsi->vpi_vid    = pSect->ulVolSerial;
                pvpfsi->vpi_bsize  = pSect->bpb.BytesPerSector;
                pvpfsi->vpi_totsec = pSect->bpb.BigTotalSectors;
                pvpfsi->vpi_trksec = pSect->bpb.SectorsPerTrack;
                pvpfsi->vpi_nhead  = pSect->bpb.Heads;
                memset(pvpfsi->vpi_text, 0, sizeof pvpfsi->vpi_text);
                memcpy(pvpfsi->vpi_text, pSect->VolumeLabel, sizeof pSect->VolumeLabel);
            }
         else  /* remount of volume */
            {
            FSH_GETVOLPARM(hDupVBP,&pvpfsi,&pvpfsd);    /* Get the volume dependent/independent structure from the original volume block */
            pVolInfo = *((PVOLINFO *)(pvpfsd->vpd_work));  /* Get the pointer to the FAT32 Volume structure from the original block */
            hVBP = hDupVBP;                          /* indicate that the old duplicate will become the remaining block */
                                            /* since the new VPB will be discarded if there already is an old one according to IFS.INF */
//            rc = ERROR_NOT_ENOUGH_MEMORY;
//            goto FS_MOUNT_EXIT;
            }

         InitCache(ulCacheSectors);

         /* continue mount in both cases:
            for a first time mount it's an initializaton
            for the n-th remount it's a reinitialization of the old VPB
         */
         pVolInfo->hVBP = hVBP;
         memcpy(&pVolInfo->BootSect, pSect, sizeof (BOOTSECT));

         pVolInfo->ulActiveFatStart = pSect->bpb.ReservedSectors;
         if (pSect->bpb.ExtFlags & 0x0080)
            pVolInfo->ulActiveFatStart +=
               pSect->bpb.BigSectorsPerFat * (pSect->bpb.ExtFlags & 0x000F);
         pVolInfo->ulStartOfData    = pSect->bpb.ReservedSectors +
               pSect->bpb.BigSectorsPerFat * pSect->bpb.NumberOfFATs;

         pVolInfo->pBootFSInfo = (PBOOTFSINFO)(pVolInfo + 1);
         pVolInfo->pbFatSector = (PBYTE)(pVolInfo->pBootFSInfo + 1);
         pVolInfo->ulCurFatSector = -1L;
         pVolInfo->usClusterSize = pSect->bpb.BytesPerSector * pSect->bpb.SectorsPerCluster;
         pVolInfo->ulTotalClusters = (pSect->bpb.BigTotalSectors - pVolInfo->ulStartOfData) / pSect->bpb.SectorsPerCluster;

//         pVolInfo->hVBP = hVBP;
//         pVolInfo->hDupVBP = hDupVBP;
         pVolInfo->bDrive = pvpfsi->vpi_drive;
         pVolInfo->bUnit  = pvpfsi->vpi_unit;
         pVolInfo->pNextVolInfo = NULL;

         pVolInfo->fFormatInProgress = FALSE;

         if (usDefaultRASectors == 0xFFFF)
            pVolInfo->usRASectors = (pVolInfo->usClusterSize / SECTOR_SIZE ) * 2;
         else
            pVolInfo->usRASectors = usDefaultRASectors;

#if 1
         if( pVolInfo->usRASectors > MAX_RASECTORS )
            pVolInfo->usRASectors = MAX_RASECTORS;
#else
         if (pVolInfo->usRASectors > (pVolInfo->usClusterSize / SECTOR_SIZE) * 4)
            pVolInfo->usRASectors = (pVolInfo->usClusterSize / SECTOR_SIZE ) * 4;
#endif

         if (pSect->bpb.FSinfoSec != 0xFFFF)
            {
            ReadSector(pVolInfo, pSect->bpb.FSinfoSec, 1, pVolInfo->pbFatSector, DVIO_OPNCACHE);
            memcpy(pVolInfo->pBootFSInfo, pVolInfo->pbFatSector + FSINFO_OFFSET, sizeof (BOOTFSINFO));
            }
         else
            memset(pVolInfo->pBootFSInfo, 0, sizeof (BOOTFSINFO));

/*         *((PVOLINFO *)(pvpfsd->vpd_work)) = pVolInfo;

         if (!pGlobVolInfo)
            {
            pGlobVolInfo = pVolInfo;
            usVolCount = 1;
            }
         else
            {
            pNext = pGlobVolInfo;
            usVolCount = 1;
            if (pNext->bDrive == pvpfsi->vpi_drive && !pVolInfo->hDupVBP)
               pVolInfo->hDupVBP = pNext->hVBP;
            while (pNext->pNextVolInfo)
               {
               pNext = (PVOLINFO)pNext->pNextVolInfo;
               if (pNext->bDrive == pvpfsi->vpi_drive && !pVolInfo->hDupVBP)
                  pVolInfo->hDupVBP = pNext->hVBP;
               usVolCount++;
               }
            pNext->pNextVolInfo = pVolInfo;
            usVolCount++;
            }
         if (f32Parms.fMessageActive & LOG_FS)
            Message("%u Volumes mounted!", usVolCount);
 */

         rc = CheckWriteProtect(pVolInfo);
         if (rc && rc != ERROR_WRITE_PROTECT)
            {
            if (f32Parms.fMessageActive & LOG_FS)
              Message("Cannot access drive, rc = %u", rc);
            goto FS_MOUNT_EXIT;
            }
         if (rc == ERROR_WRITE_PROTECT)
            pVolInfo->fWriteProtected = TRUE;

         pVolInfo->fDiskCleanOnMount = pVolInfo->fDiskClean = GetDiskStatus(pVolInfo);
         if (!pVolInfo->fDiskCleanOnMount && f32Parms.fMessageActive & LOG_FS)
            Message("DISK IS DIRTY!");
         if (pVolInfo->fWriteProtected)
            pVolInfo->fDiskCleanOnMount = TRUE;

         if (!pVolInfo->hDupVBP &&
                (f32Parms.fCalcFree ||
                 pVolInfo->pBootFSInfo->ulFreeClusters == 0xFFFFFFFF ||
               /*!pVolInfo->fDiskClean ||*/
                 pVolInfo->BootSect.bpb.FSinfoSec == 0xFFFF))
            GetFreeSpace(pVolInfo);

         pDevCaps  = pvpfsi->vpi_pDCS;
         pVolChars = pvpfsi->vpi_pVCS;

         if (f32Parms.fMessageActive & LOG_FS)
         {
            if (pDevCaps->Capabilities & GDC_DD_Read2)
               Message("Read2 supported");
            if (pDevCaps->Capabilities & GDC_DD_DMA_Word)
               Message("DMA on word alligned buffers supported");
            if (pDevCaps->Capabilities & GDC_DD_DMA_Byte)
               Message("DMA on byte alligned buffers supported");
            if (pDevCaps->Capabilities & GDC_DD_Mirror)
               Message("Disk Mirroring supported");
            if (pDevCaps->Capabilities & GDC_DD_Duplex)
               Message("Disk Duplexing supported");
            if (pDevCaps->Capabilities & GDC_DD_No_Block)
               Message("Strategy2 does not block");
            if (pDevCaps->Capabilities & GDC_DD_16M)
               Message(">16M supported");
         }

         if (pDevCaps->Strategy2)
         {
            if (f32Parms.fMessageActive & LOG_FS)
            {
               Message("Strategy2   address at %lX", pDevCaps->Strategy2);
               Message("ChgPriority address at %lX", pDevCaps->ChgPriority);
            }

            pVolInfo->pfnStrategy = (STRATFUNC)pDevCaps->Strategy2;
            pVolInfo->pfnPriority = (STRATFUNC)pDevCaps->ChgPriority;
         }

         rc = 0;
         break;

      case MOUNT_ACCEPT :

         if (FSH_FINDDUPHVPB(hVBP, &hDupVBP))
           hDupVBP = 0;

         if (pvpfsi->vpi_bsize != SECTOR_SIZE)
            {
            rc = ERROR_VOLUME_NOT_MOUNTED;
            goto FS_MOUNT_EXIT;
            }

         pVolInfo = gdtAlloc(STORAGE_NEEDED, FALSE);
         if (!pVolInfo)
            {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto FS_MOUNT_EXIT;
            }
         rc = FSH_FORCENOSWAP(SELECTOROF(pVolInfo));
         if (rc)
            FatalMessage("FSH_FORCENOSWAP on VOLINFO Segment failed, rc=%u", rc);

         memset(pVolInfo, 0, (size_t)STORAGE_NEEDED);

         //InitCache(ulCacheSectors);

         memset(&pVolInfo->BootSect, 0, sizeof (BOOTSECT));
         pVolInfo->BootSect.bpb.BigTotalSectors = pvpfsi->vpi_totsec;
         pVolInfo->BootSect.bpb.BytesPerSector  = pvpfsi->vpi_bsize;
         pVolInfo->fWriteProtected   = FALSE;
         pVolInfo->fDiskCleanOnMount = FALSE;

         pVolInfo->hVBP = hVBP;
         pVolInfo->hDupVBP = hDupVBP;
         pVolInfo->bDrive = pvpfsi->vpi_drive;
         pVolInfo->bUnit  = pvpfsi->vpi_unit;
         pVolInfo->pNextVolInfo = NULL;

         // the volume is being formatted
         pVolInfo->fFormatInProgress = TRUE;

         // fake values assuming sector == cluster
         pVolInfo->usClusterSize   = pvpfsi->vpi_bsize;
         pVolInfo->ulTotalClusters = pvpfsi->vpi_totsec;

         pVolInfo->usRASectors = usDefaultRASectors;

         // undefined
         pVolInfo->ulStartOfData = 0;

         *((PVOLINFO *)(pvpfsd->vpd_work)) = pVolInfo; //

         if (!pGlobVolInfo)
            {
            pGlobVolInfo = pVolInfo;
            usVolCount = 1;
            }
         else
            {
            pNext = pGlobVolInfo;
            usVolCount = 1;
            if (pNext->bDrive == pvpfsi->vpi_drive && !pVolInfo->hDupVBP)
               pVolInfo->hDupVBP = pNext->hVBP;
            while (pNext->pNextVolInfo)
               {
               pNext = (PVOLINFO)pNext->pNextVolInfo;
               if (pNext->bDrive == pvpfsi->vpi_drive && !pVolInfo->hDupVBP)
                  pVolInfo->hDupVBP = pNext->hVBP;
               usVolCount++;
               }
            pNext->pNextVolInfo = pVolInfo;
            usVolCount++;
            }
         if (f32Parms.fMessageActive & LOG_FS)
            Message("%u Volumes mounted!", usVolCount);

         pDevCaps  = pvpfsi->vpi_pDCS;
         pVolChars = pvpfsi->vpi_pVCS;

         if (f32Parms.fMessageActive & LOG_FS)
         {
            if (pDevCaps->Capabilities & GDC_DD_Read2)
               Message("Read2 supported");
            if (pDevCaps->Capabilities & GDC_DD_DMA_Word)
               Message("DMA on word alligned buffers supported");
            if (pDevCaps->Capabilities & GDC_DD_DMA_Byte)
               Message("DMA on byte alligned buffers supported");
            if (pDevCaps->Capabilities & GDC_DD_Mirror)
               Message("Disk Mirroring supported");
            if (pDevCaps->Capabilities & GDC_DD_Duplex)
               Message("Disk Duplexing supported");
            if (pDevCaps->Capabilities & GDC_DD_No_Block)
               Message("Strategy2 does not block");
            if (pDevCaps->Capabilities & GDC_DD_16M)
               Message(">16M supported");
         }

         if (pDevCaps->Strategy2)
         {
            if (f32Parms.fMessageActive & LOG_FS)
            {
               Message("Strategy2   address at %lX", pDevCaps->Strategy2);
               Message("ChgPriority address at %lX", pDevCaps->ChgPriority);
            }

            pVolInfo->pfnStrategy = (STRATFUNC)pDevCaps->Strategy2;
            pVolInfo->pfnPriority = (STRATFUNC)pDevCaps->ChgPriority;
         }

         rc = 0;
         break;

      case MOUNT_VOL_REMOVED:
      case MOUNT_RELEASE:
         pVolInfo = GetVolInfo(hVBP);

         if (!pVolInfo)
            {
            if (f32Parms.fMessageActive & LOG_FS)
                Message("pVolInfo == 0\n");
            
            rc = ERROR_VOLUME_NOT_MOUNTED;
            goto FS_MOUNT_EXIT;
            }

         //if (FSH_FINDDUPHVPB(hVBP, &hDupVBP))
           //hDupVBP = 0;

         if (!pVolInfo->hDupVBP)
         //if (!hDupVBP)
           {
            if (f32Parms.fMessageActive & LOG_FS)
                Message("hDupVBP == 0\n");

            usFlushVolume( pVolInfo, FLUSH_DISCARD, TRUE, PRIO_URGENT );

            //if (f32Parms.usDirtySectors) // vs
            UpdateFSInfo(pVolInfo);  //

            MarkDiskStatus(pVolInfo, TRUE);
           }
/*
         if (!pVolInfo->hDupVBP)
         {
            usFlushVolume( pVolInfo, FLUSH_DISCARD, TRUE, PRIO_URGENT );

            if (f32Parms.usDirtySectors) // vs
                UpdateFSInfo(pVolInfo);  //

            MarkDiskStatus(pVolInfo, TRUE);
         }
 */

/*         // delete pVolInfo from the list
         if (pGlobVolInfo)
            {
            pNext = pPrev = pGlobVolInfo;

            // search for pVolInfo in the list
            while (pNext != pVolInfo)
               {
               pPrev = pNext;
               pNext = (PVOLINFO)pNext->pNextVolInfo;
               }

            // found
            if (pNext == pVolInfo)
               {
               if (pPrev == pVolInfo) // the very 1st list item
                  pGlobVolInfo = NULL;
               else
                  {
                  // delete it
                  pNext = pNext->pNextVolInfo;
                  pPrev->pNextVolInfo = pNext;
                  }
               usVolCount--;
               }
            } */

         RemoveVolume(pVolInfo);
         freeseg(pVolInfo);
         rc = NO_ERROR;
         break;

      default :
         rc = ERROR_NOT_SUPPORTED;
         break;
      }

FS_MOUNT_EXIT:
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_MOUNT returned %u\n", rc);

   _asm pop  es;

   return rc;
}

static USHORT CheckWriteProtect(PVOLINFO pVolInfo)
{
USHORT rc;
USHORT usSectors = 1;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("CheckWriteProtect");

   rc = FSH_DOVOLIO(DVIO_OPREAD, DVIO_ALLACK, pVolInfo->hVBP, pVolInfo->pbFatSector, &usSectors, 1L);
   if (!rc)
      {
      usSectors = 1;
      rc = FSH_DOVOLIO(DVIO_OPWRITE, DVIO_ALLACK, pVolInfo->hVBP, pVolInfo->pbFatSector, &usSectors, 1L);
      }

   return rc;
}

static BOOL RemoveVolume(PVOLINFO pVolInfo)
{
PVOLINFO pNext;
USHORT rc;

   if (pGlobVolInfo == pVolInfo)
      {
      pGlobVolInfo = pVolInfo->pNextVolInfo;
      return TRUE;
      }

   pNext = (PVOLINFO)pGlobVolInfo;
   while (pNext)
      {
      rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pNext, sizeof (VOLINFO));
      if (rc)
         {
         FatalMessage("FAT32: Protection VIOLATION in RemoveVolume!\n");
         return FALSE;
         }

      if (pNext->pNextVolInfo == pVolInfo)
         {
         pNext->pNextVolInfo = pVolInfo->pNextVolInfo;
         return TRUE;
         }
      pNext = (PVOLINFO)pNext->pNextVolInfo;
      }
   return FALSE;
}

static BOOL IsFAT32(BOOTSECT far *pSect)
{
/*
   check for FAT32 according to the Microsoft FAT32 specification
*/
    PBPB  pbpb;
    ULONG FATSz;
    ULONG TotSec;
    ULONG RootDirSectors;
    ULONG NonDataSec;
    ULONG DataSec;
    ULONG CountOfClusters;

    if (!pSect)
    {
        return FALSE;
    } /* endif */

    pbpb = &pSect->bpb;

    if (!pbpb->BytesPerSector)
    {
      return FALSE;
    }

    if (pbpb->BytesPerSector != SECTOR_SIZE)
    {
        return FALSE;
    }

    if(( ULONG )pSect->bpb.BytesPerSector * pSect->bpb.SectorsPerCluster > MAX_CLUSTER_SIZE )
    {
        return FALSE;
    }

    RootDirSectors = ((pbpb->RootDirEntries * 32UL) + (pbpb->BytesPerSector-1UL)) / pbpb->BytesPerSector;

    if (pbpb->SectorsPerFat)
    {
        FATSz = pbpb->SectorsPerFat;
    }
    else
    {
        FATSz = pbpb->BigSectorsPerFat;
    } /* endif */

    if (pbpb->TotalSectors)
    {
        TotSec = pbpb->TotalSectors;
    }
    else
    {
        TotSec = pbpb->BigTotalSectors;
    } /* endif */

    NonDataSec =      pbpb->ReservedSectors
                    +  (pbpb->NumberOfFATs * FATSz)
                    +  RootDirSectors;

    if (TotSec < NonDataSec)
    {
        return FALSE;
    } /* endif */

    DataSec = TotSec - NonDataSec;
    CountOfClusters = DataSec / pbpb->SectorsPerCluster;

    if ((CountOfClusters >= 65525UL) && !memicmp(pSect->FileSystem, "FAT32", 5))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    } /* endif */
}
