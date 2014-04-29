#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_DOS


#include <os2.h>
#include "portable.h"

#include "fat32.h"


BYTE szText[]="\
³ ³³³ ³\n\
³ ³³³ À Partition type (the number the specify after /P to get this\n\
³ ³³³                   partition type handled by PARTFILT)\n\
³ ³³ÀÄÄ H = Hidden partition\n\
³ ³ÀÄÄÄ A = Active / B = Bootable via bootmanager\n\
³ ÀÄÄÄÄ P = Primary / L = Logical (extended)\n\
ÀÄÄÄÄÄÄ Seq # to be used in the OPTIONAL /M argument for PARTFILT.\n\n";



static APIRET ReadSectors(HFILE hDisk, USHORT usHead, USHORT usCylinder, USHORT usSector, USHORT nSectors, PVOID pvData);
static APIRET ScanDisks(WORD wDiskNum);
static BOOL   GetDPMB(HFILE hDisk, PDEVICEPARAMETERBLOCK pDPMB);
static BOOL   GetMBR(HFILE hDisk, PMBR pMBR, USHORT usHead, USHORT usCylinder, USHORT usSector);
static VOID   vListMBR(HFILE hDisk, PMBR pMBR,  PDEVICEPARAMETERBLOCK pdpmb, WORD wDiskNum);
static APIRET PrepareDrive(PDRIVEINFO pDrive);
VOID vDumpSector(PBYTE pbSector);

/****************************************************************
*
****************************************************************/
APIRET InitProg(void)
{
APIRET rc;
WORD   wDiskCount;
WORD   wDisk;
USHORT usDrive;

   rc = DosPhysicalDisk(INFO_COUNT_PARTITIONABLE_DISKS,
      &wDiskCount,
      sizeof wDiskCount,
      NULL,
      0);

   if (rc)
      {
      printf("DosPhysicalDisk returned %ld\n", rc);
      return rc;
      }
   printf("There are %d disks\n", wDiskCount);

   for (wDisk = 1; wDisk <= wDiskCount; wDisk++)
      ScanDisks(wDisk);

   printf(szText);

   printf("%d FAT32 partitions found!\n", usDriveCount);
   if (!memcmp(rgfFakePart, rgfFakeComp, 256))
      {
      printf("WARNING: /P xx not specified.\n");
      printf("         Only 'normal' partitions are assigned a partition sequence number!\n");
      }

   if (usDriveCount > 0 && fDetailed > 1)
      {
      for (usDrive = 0; usDrive < usDriveCount; usDrive++)
         {
         printf("\n");
         if (PrepareDrive(&rgDrives[usDrive]))
            return rc;
         }
      }

   return 0;
}



/****************************************************************
*
****************************************************************/
APIRET ScanDisks(WORD wDiskNum)
{
DEVICEPARAMETERBLOCK dpmb;
APIRET rc;
HFILE  hDisk;
MBR    mbr;

   printf("=== Scanning physical disk %d.===\n", wDiskNum);

   rc  = OpenDisk(wDiskNum, &hDisk);
   if (rc)
      return rc;

   if (GetDPMB(hDisk, &dpmb))
      {
      if (fDetailed > 1)
         printf("There are %d cylinders, %d heads and %d sectors/track\n",
            dpmb.cCylinders,
            dpmb.cHeads,
            dpmb.cSectorsPerTrack);
      }

   if (GetMBR(hDisk, &mbr, 0, 0, 1))
      vListMBR(hDisk, &mbr, &dpmb, wDiskNum);

   CloseDisk(hDisk);

   return 0;
}

APIRET OpenDisk(WORD wDiskNum, PHFILE phFile)
{
APIRET rc;
BYTE   szDisk[5];
WORD   wHandle;

   sprintf(szDisk, "%d:", wDiskNum);
   rc = DosPhysicalDisk(INFO_GETIOCTLHANDLE,
      &wHandle,
      sizeof wHandle,
      szDisk,
      strlen(szDisk)+1);
   if (rc)
      {
      printf("Unable to obtain an IOCTL handle, SYS%4.4u!\n", rc);
      return rc;
      }
   *phFile = wHandle;
   return 0;
}

APIRET CloseDisk(HFILE hFile)
{
APIRET rc;
WORD   wHandle = hFile;

   rc = DosPhysicalDisk(INFO_FREEIOCTLHANDLE,
      NULL,
      0,
      &wHandle,
      sizeof wHandle);
   if (rc)
      {
      printf("Unable to close an IOCTL handle, rc = %d!\n", rc);
      return rc;
      }
   return 0;
}
/****************************************************************
*
****************************************************************/
VOID vListMBR(HFILE hDisk, PMBR pMBR,  PDEVICEPARAMETERBLOCK pdpmb, WORD wDiskNum)
{
int  iIndex;
PSZ  pszPartType;
BYTE szUnknown[50];
PPARTITIONENTRY pPart;
static iCount = 0;
static iLevel = 0;
BOOL fKnown;
BOOL fExtended;
BYTE bActive;
BYTE bType;

      iLevel++;
      for (iIndex = 0 ; iIndex < 4 ; iIndex++)
         {
         fKnown = FALSE;
         fExtended = FALSE;
         pPart = &pMBR->partition[iIndex];
         if (rgfFakePart[pPart->cSystem])
            fKnown = TRUE;
         switch (pPart->cSystem & ~PARTITION_HIDDEN)
            {
            case PARTITION_ENTRY_UNUSED   :
               pszPartType = NULL;
               break;
            case PARTITION_FAT_12         :
               fKnown = TRUE;
               pszPartType = "FAT12";
               break;
            case PARTITION_XENIX_1        : 
               pszPartType = "XENIX_1";
               break;
            case PARTITION_XENIX_2        :
               pszPartType = "XENIX_2";
               break;
            case PARTITION_FAT_16         :
               fKnown = TRUE;
               pszPartType = "FAT16";
               break;
            case PARTITION_EXTENDED       :
               fExtended = TRUE;
               if (fDetailed)
                  pszPartType = "EXTENDED (05)";
               else
                  pszPartType = NULL;
               break;
            case PARTITION_HUGE           :
               fKnown = TRUE;
               pszPartType = "HUGE";
               break;
            case PARTITION_IFS            :
               fKnown = TRUE;
               pszPartType = "IFS";
               break;
            case PARTITION_BOOTMANAGER    :
               pszPartType = "BOOTMANAGER";
               break;
            case PARTITION_FAT32          :
            case PARTITION_FAT32_XINT13   :
               if ((pPart->cSystem & ~PARTITION_HIDDEN) == PARTITION_FAT32)
                  pszPartType = "FAT32";
               else
                  pszPartType = "FAT32 (int13)";

               rgDrives[usDriveCount].bPartType = pPart->cSystem;
               rgDrives[usDriveCount].DiskNum = wDiskNum;
               rgDrives[usDriveCount].nHeads = pdpmb->cHeads;
               rgDrives[usDriveCount].nCylinders = pdpmb->cCylinders;
               rgDrives[usDriveCount].nSectorsPerTrack = pdpmb->cSectorsPerTrack;
               rgDrives[usDriveCount].StartHead = pPart->Start.Head;
               rgDrives[usDriveCount].StartCylinder = pPart->Start.Cylinders256 * 256 + pPart->Start.Cylinders;
               rgDrives[usDriveCount].StartSector = pPart->Start.Sectors;

               rgDrives[usDriveCount].EndHead = pPart->End.Head;
               rgDrives[usDriveCount].EndCylinder = pPart->End.Cylinders256 * 256 + pPart->End.Cylinders;
               rgDrives[usDriveCount].EndSector = pPart->End.Sectors;
               usDriveCount++;
               break;
            case PARTITION_XINT13         :
               pszPartType = "XINT13";
               break;
            case PARTITION_XINT13_EXTENDED:
               fExtended = TRUE;
               if (fDetailed)
                  pszPartType = "EXTENDED (0F)";
               else
                  pszPartType = NULL;
               break;
            case PARTITION_PREP           :
               pszPartType = "PREP";
               break;
            case PARTITION_UNIX           :
               pszPartType = "UNIX";
               break;
            case VALID_NTFT               :
               pszPartType = "NTFT";
               break;
            case PARTITION_LINUX:
               pszPartType = "LINUX";
               break;
            default:
               sprintf(szUnknown, "UNKNOWN %X", pPart->cSystem);
               pszPartType = szUnknown;
               break;
            }

         if (pszPartType)
            {
            if (fDetailed)
               printf("%*.*s", iLevel-1, iLevel-1, "               ");
            if (!fExtended)
               {
               if (fKnown)
                  {
                  printf("%d:", iCount);
                  iCount++;
                  }
               else
                  printf("%c:", '-');
               }
            else
               printf(" :");

            bActive = ' ';
            if (iLevel == 1 && (pPart->iBoot & 0x80))
               bActive = 'A';
            else if (pPart->iBoot & 0x80)
               bActive = 'B';
            if (!fExtended)
               {
               if (iLevel == 1)
                  bType = 'P';
               else
                  bType = 'L';
               }
            else
               bType = 'E';

            printf("%c%c%c %2.2X %-15.15s",
               bType,
               bActive,
               (pPart->cSystem & PARTITION_HIDDEN ? 'H' : ' '),
               pPart->cSystem,
               pszPartType);

            printf("Strt:H:%6d C:%4d S:%4d",
               pPart->Start.Head,
               pPart->Start.Cylinders256 * 256 + pPart->Start.Cylinders,
               pPart->Start.Sectors);
            printf(" End:H:%6d C:%4d S:%4d\n",
               pPart->End.Head,
               pPart->End.Cylinders256 * 256 + pPart->End.Cylinders,
               pPart->End.Sectors);
            }
         }

      for (iIndex = 0 ; iIndex < 4 ; iIndex++)
         {
         pPart = &pMBR->partition[iIndex];
         if (pPart->cSystem == PARTITION_EXTENDED ||
             pPart->cSystem == PARTITION_XINT13_EXTENDED)
            {
            MBR mbr2;
            if (fDetailed)
               printf(" (extended partition)\n");
            if (GetMBR(hDisk, &mbr2,
               pPart->Start.Head,
               pPart->Start.Cylinders256 * 256 + pPart->Start.Cylinders,
               pPart->Start.Sectors))
               vListMBR(hDisk, &mbr2, pdpmb, wDiskNum);
            }
         }

      iLevel--;
}

/****************************************************************
*
****************************************************************/
BOOL GetDPMB(HFILE hDisk, PDEVICEPARAMETERBLOCK pDPMB)
{
BYTE   bParam;
ULONG  ulParmSize;
ULONG  ulDataSize;
APIRET rc;

   bParam = 0;
   ulParmSize = sizeof bParam; 
   ulDataSize  = sizeof (DEVICEPARAMETERBLOCK);

   rc = DosDevIOCtl(hDisk,
      IOCTL_PHYSICALDISK,
      PDSK_GETPHYSDEVICEPARAMS,
      (PVOID)&bParam, ulParmSize, &ulParmSize,
      (PVOID)pDPMB, ulDataSize, &ulDataSize);

   if (rc)
      {
      printf("GetDPMB: DosDevIOCtl failed, SYS%4.4u\n", rc);
      exit(1);
      }
   return TRUE;
}



/**************************************************************
*
**************************************************************/
BOOL GetMBR(HFILE hDisk, PMBR pMBR, USHORT usHead, USHORT usCylinder, USHORT usSector)
{
static BYTE   bSector[SECTOR_SIZE];
APIRET        rc;

   rc = ReadSectors(hDisk, usHead, usCylinder, usSector, 1, bSector);
   if (rc)
      return FALSE;

   memcpy(pMBR, bSector + MBRTABLEOFFSET, sizeof (MBR));

   return TRUE;
}

/**************************************************************
*
**************************************************************/
APIRET ReadSectors(HFILE hDisk, USHORT usHead, USHORT usCylinder, USHORT usSector, USHORT nSectors, PVOID pvData)
{
DEVICEPARAMETERBLOCK dpmb;
ULONG         ulParmSize;
ULONG         ulDataSize;
PTRACKLAYOUT  pTrackLayout;
USHORT        usIndex;
APIRET        rc;

   GetDPMB(hDisk, &dpmb);

//   printf("Reading H:%d C:%d S:%d (%d sectors)\n",
//      usHead, usCylinder, usSector, nSectors);

   ulParmSize = sizeof (TRACKLAYOUT) + (dpmb.cSectorsPerTrack - 1) * 4;
   pTrackLayout = malloc(ulParmSize);
   if (!pTrackLayout)
      {
      printf("Not enough memory for Tracklayout!\n");
      return FALSE;
      }

   memset(pTrackLayout, 0, ulParmSize);

   pTrackLayout->bCommand      = (usSector == 1 ? 0x01 : 0x00);
   pTrackLayout->usHead        = usHead;
   pTrackLayout->usCylinder    = usCylinder;
   pTrackLayout->usFirstSector = usSector - 1;
   pTrackLayout->cSectors      = nSectors;

   for (usIndex = 0; usIndex < dpmb.cSectorsPerTrack; usIndex ++)
      {
      pTrackLayout->TrackTable[usIndex].usSectorNumber = usIndex + 1;
      pTrackLayout->TrackTable[usIndex].usSectorSize = SECTOR_SIZE;
      }

   ulDataSize = SECTOR_SIZE * nSectors;

   rc = DosDevIOCtl(hDisk, IOCTL_PHYSICALDISK, PDSK_READPHYSTRACK,
      (PVOID)pTrackLayout, ulParmSize, &ulParmSize,
      pvData, ulDataSize, &ulDataSize);

   if (rc)
      printf("ReadSectors: DosDevIOCtl failed, SYS%4.4u\n", rc);

   free(pTrackLayout);

   return rc;

}

APIRET PrepareDrive(PDRIVEINFO pDrive)
{
APIRET rc;
BYTE bSector[SECTOR_SIZE];
BOOTSECT  bSect;
USHORT    usIndex;
ULONG     ulBytes;
USHORT    usBlocks;

   pDrive->ulRelStartSector =
      pDrive->StartCylinder * pDrive->nHeads * pDrive->nSectorsPerTrack +
      pDrive->StartHead * pDrive->nSectorsPerTrack +
      pDrive->StartSector;
   pDrive->ulRelEndSector =
      pDrive->EndCylinder * pDrive->nHeads * pDrive->nSectorsPerTrack +
      pDrive->EndHead * pDrive->nSectorsPerTrack +
      pDrive->EndSector;

   pDrive->cTrackSize = sizeof (TRACKLAYOUT) +
      (pDrive->nSectorsPerTrack - 1) * 4;

   pDrive->pTrack = malloc(pDrive->cTrackSize);
   if (!pDrive->pTrack)
      return 8;

   memset(pDrive->pTrack, 0, pDrive->cTrackSize);
   for (usIndex = 0; usIndex < pDrive->nSectorsPerTrack; usIndex ++)
      {
      pDrive->pTrack->TrackTable[usIndex].usSectorNumber = usIndex + 1;
      pDrive->pTrack->TrackTable[usIndex].usSectorSize = SECTOR_SIZE;
      }

   printf("Reading BOOT Sector\n");

   rc = OpenDisk(pDrive->DiskNum, &pDrive->hDisk);
   if (rc)
      return rc;

   rc = ReadSectors(pDrive->hDisk,
      pDrive->StartHead,
      pDrive->StartCylinder,
      pDrive->StartSector,
      1, bSector);

   if (rc)
      {
      CloseDisk(pDrive->hDisk);
      pDrive->hDisk = 0;
      return rc;
      }

   memcpy(&bSect, bSector, sizeof bSect);


   memcpy(&pDrive->bpb, &bSect.bpb, sizeof pDrive->bpb);
   pDrive->ulStartOfFAT = pDrive->bpb.ReservedSectors;

   pDrive->ulCurFATSector = -1L;
   pDrive->ulCurCluster = FAT_EOF;
   pDrive->usClusterSize = pDrive->bpb.SectorsPerCluster * pDrive->bpb.BytesPerSector;
   pDrive->ulStartOfData = pDrive->bpb.ReservedSectors +
               pDrive->bpb.BigSectorsPerFat * pDrive->bpb.NumberOfFATs;
   pDrive->ulTotalClusters = (pDrive->bpb.BigTotalSectors - pDrive->ulStartOfData) / pDrive->bpb.SectorsPerCluster;

   pDrive->pbCluster = malloc(pDrive->bpb.SectorsPerCluster * pDrive->bpb.BytesPerSector);
   if (!pDrive->pbCluster)
      return FALSE;

   ulBytes = pDrive->ulTotalClusters / 8 +
      (pDrive->ulTotalClusters % 8 ? 1:0);
   usBlocks = (USHORT)(ulBytes / 4096 +
             (ulBytes % 4096 ? 1:0));

   pDrive->pFatBits = calloc(usBlocks,4096);
   if (!pDrive->pFatBits)
      printf("Not enough memory for FATBITS\n");

   if (fDetailed > 1)
      {
      printf("BytesPerSector    : %u\n", bSect.bpb.BytesPerSector);
      printf("SectorsPerCluster : %u\n", bSect.bpb.SectorsPerCluster);
      printf("Reserved Sectors  : %u\n", bSect.bpb.ReservedSectors);
      printf("NumberOfFATs;     : %u\n", bSect.bpb.NumberOfFATs);    
      printf("RootDirEntries;   : %u\n", bSect.bpb.RootDirEntries);  
      printf("TotalSectors;     : %u\n", bSect.bpb.TotalSectors);    
      printf("MediaDescriptor;  : %u\n", bSect.bpb.MediaDescriptor); 
      printf("SectorsPerFat;    : %u\n", bSect.bpb.SectorsPerFat);   
      printf("SectorsPerTrack;  : %u\n", bSect.bpb.SectorsPerTrack); 
      printf("Heads;            : %u\n", bSect.bpb.Heads);           
      printf("HiddenSectors;    : %lu\n",bSect.bpb.HiddenSectors);   
      printf("BigTotalSectors;  : %lu\n",bSect.bpb.BigTotalSectors); 
      printf("BigSectorsPerFat; : %lu\n",bSect.bpb.BigSectorsPerFat);
      printf("ExtFlags;         : %u\n", bSect.bpb.ExtFlags);        
      printf("FS_Version;       : %u\n", bSect.bpb.FS_Version);      
      printf("RootDirStrtClus;  : %lu\n",bSect.bpb.RootDirStrtClus); 
      printf("FSinfoSec;        : %u\n", bSect.bpb.FSinfoSec);       
      printf("BkUpBootSec;      : %u\n", bSect.bpb.BkUpBootSec);     
      printf("bReserved[6];     : %-6.6s\n", bSect.bpb.bReserved);

      printf("Volume label      : '%-11.11s'\n", bSect.VolumeLabel);
      printf("FileSystem        : '%-8.8s'\n", bSect.FileSystem);
      printf("Total clusters    : %lu\n", pDrive->ulTotalClusters);

      if (fShowBootSector)
         vDumpSector((PBYTE)&bSect);
      }

   CloseDisk(pDrive->hDisk);
   pDrive->hDisk = 0;

   return 0;
}


VOID vDumpSector(PBYTE pbSector)
{
BYTE szText[17];
PSZ  p;
USHORT usIndex;

   memset(szText, 0, sizeof szText);

   for (usIndex = 0; usIndex < 512; usIndex++)
      {
      if (!(usIndex % 16))
         {
         printf("  %s\n", szText);
         memset(szText, 0, sizeof szText);
         p = szText;
         }
      printf("%2.2X ", pbSector[usIndex]);
      if (pbSector[usIndex] >= ' ')
         *p++ = pbSector[usIndex];
      else
         *p++ = '.';
      }
   printf("  %s\n", szText);

}
