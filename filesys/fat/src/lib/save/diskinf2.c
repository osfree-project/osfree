#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>
#include "portable.h"
#include "fat32.h"


static APIRET AbsRead(PDRIVEINFO pDrive, ULONG ulSector, USHORT nSectors, PBYTE pvData);
static APIRET ReadSectors(PDRIVEINFO pDrive, USHORT usHead, USHORT usCylinder, USHORT usSector, USHORT nSectors, PVOID pvData);
static BOOL   ReadFATSector(PDRIVEINFO pDrive, ULONG ulSector);
static BOOL   ReadCluster(PDRIVEINFO pDrive, ULONG ulCluster);
static BOOL   fGetLongName(PDIRENTRY pDir, PSZ pszName, USHORT wMax);
PSZ   MakeName(PDIRENTRY pDir, PSZ pszName, USHORT usMax);
VOID vDumpDirEntry(PDIRENTRY pDir);
ULONG GetClusterCount(PDRIVEINFO pDrive, ULONG ulCluster);

IMPORT BOOL ClusterInUse(PDRIVEINFO pDrive, ULONG ulCluster);
                                        

BYTE szMaxPath[512];
ULONG ulTotalClusters = 0;



BOOL DumpDirectory(PDRIVEINFO pDrive, ULONG ulDirCluster, PSZ pszPath)
{
static BYTE szLongName[512];
int iIndex;
PDIRENTRY pDir;
PBYTE pbCluster;
PBYTE pbPath;
USHORT usClusters;
ULONG ulCluster;
ULONG ulBytesNeeded;
BYTE _huge * p;
PDIRENTRY pEnd;
BYTE bCheckSum, bCheck;
ULONG ulClustersNeeded;
ULONG ulClustersUsed;
ULONG ulEntries;
static BYTE szShortName[13];


   if (!ulDirCluster)
      {
      printf("ERROR: Cluster for %s is 0!\n", pszPath);
      return TRUE;
      }

   if (ulDirCluster == pDrive->bpb.RootDirStrtClus)
      {
      PULONG pulCluster;
      ReadFATSector(pDrive, 0);
      pulCluster = (PULONG)pDrive->pbFATSector;
      printf("MEDIA BYTES in FAT: %8.8lX\n",
         *pulCluster);

      pulCluster = (PULONG)pDrive->pbFATSector + 1;
      printf("DiskStatus: %8.8lX\n",
         *pulCluster);
//      vDumpSector(pDrive->pbFATSector);
      }

   //pDrive->ulTotalDirs++;

   pbPath = malloc(512);
   usClusters = GetClusterCount(pDrive, ulDirCluster);
   ulTotalClusters += usClusters;

   if (fDetailed > 2)
      printf("\n\nDirectory of %s (%u clusters)\n\n", pszPath, usClusters);

   ulBytesNeeded = (ULONG)pDrive->bpb.SectorsPerCluster * (ULONG)pDrive->bpb.BytesPerSector * usClusters;
   pbCluster = calloc(usClusters, pDrive->bpb.SectorsPerCluster * pDrive->bpb.BytesPerSector);
   if (!pbCluster)
      {
      printf("ERROR:Not enough memory!\n");
      return FALSE;
      }

   ulCluster = ulDirCluster;
   p = pbCluster;
   while (ulCluster != FAT_EOF)
      {
      ReadCluster(pDrive, ulCluster);
      memcpy(p, pDrive->pbCluster, pDrive->bpb.SectorsPerCluster * pDrive->bpb.BytesPerSector);
      ulCluster = GetNextCluster(pDrive, ulCluster);
      p += pDrive->bpb.SectorsPerCluster * pDrive->bpb.BytesPerSector;
      }

   memset(szLongName, 0, sizeof szLongName);
   pDir = (PDIRENTRY)pbCluster;
   pEnd = (PDIRENTRY)(pbCluster + ulBytesNeeded - sizeof (DIRENTRY));
   ulEntries = 0;
   bCheck = 0;
   while (pDir <= pEnd)
      {
      if (fDetailed > 3)
         {
         ULONG ulOffset = (PBYTE)pDir - pbCluster;
         if (ulOffset && !(ulOffset % 4096))
            printf("-------- NEXT CLUSTER ----------\n");
         }
      if (pDir->bFileName[0] && pDir->bFileName[0] != 0xE5)
         {
         if (pDir->bAttr == FILE_LONGNAME)
            {
            if (fDetailed > 3)
               {
               printf("(longname)\n");
               vDumpDirEntry(pDir);
               }
            if (strlen(szLongName) && bCheck != pDir->bReserved)
               {
               printf("A lost long filename was found: %s\n",
                  szLongName);
               memset(szLongName, 0, sizeof szLongName);
               }
            bCheck = pDir->bReserved;
            fGetLongName(pDir, szLongName, sizeof szLongName);
            }
         else 
            {
            bCheckSum = 0;
            for (iIndex = 0; iIndex < 11; iIndex++)
               {
               if (bCheckSum & 0x01)
                  {
                  bCheckSum >>=1;
                  bCheckSum |= 0x80;
                  }
               else
                  bCheckSum >>=1;
               bCheckSum += pDir->bFileName[iIndex];
               }
            if (strlen(szLongName) && bCheck != bCheckSum)
               {
               printf("The longname %s does not belong to %s\\%s\n",
                  szLongName, pszPath, MakeName(pDir, szShortName, sizeof szShortName));
               memset(szLongName, 0, sizeof szLongName);
               }

            if (fDetailed > 2)
               {
               printf("%-8.8s.%-3.3s %2.2X ",
                  pDir->bFileName,
                  pDir->bExtention,
                  pDir->bAttr);
               if (pDir->bAttr & FILE_DIRECTORY)
                  printf("<DIR>      ");
               else
                  printf("%10lu ", pDir->ulFileSize);

               printf("%8.8lX ", MAKEP(pDir->wClusterHigh, pDir->wCluster));
               if (pDir->fEAS)
                  printf("%2.2X ", pDir->fEAS);
               else
                  printf("   ");

               printf("%s\n", szLongName);
               }
            if (!(pDir->bAttr & FILE_DIRECTORY))
               {

               ulClustersNeeded = pDir->ulFileSize / pDrive->usClusterSize +
                  (pDir->ulFileSize % pDrive->usClusterSize ? 1:0);
               ulClustersUsed = GetClusterCount(pDrive,(ULONG)pDir->wClusterHigh * 0x10000 + pDir->wCluster);
               ulTotalClusters += ulClustersUsed;
               if (ulClustersNeeded != ulClustersUsed)
                  {
                  printf("File allocation error detected for %s\\%s\n",
                     pszPath, MakeName(pDir, szShortName, sizeof szShortName));
                  printf("%lu clusters needed, %lu clusters allocated\n",
                     ulClustersNeeded, ulClustersUsed);
                  }
               }
            memset(szLongName, 0, sizeof szLongName);
            }
         ulEntries++;
         }
      else if (fDetailed > 3)
         {
         if (!pDir->bFileName[0])
            printf("(never used entry)\n");
         else
            printf("(deleted entry)\n");
         }

      pDir++; 
      }
   if (fDetailed > 2)
      printf("%ld files\n", ulEntries);

   bCheck = 0;
   pDir = (PDIRENTRY)pbCluster;
   memset(szLongName, 0, sizeof szLongName);
   while (pDir <= pEnd)
      {
      if (pDir->bFileName[0] && pDir->bFileName[0] != 0xE5)
         {
         if (pDir->bAttr == FILE_LONGNAME)
            {
            if (strlen(szLongName) && bCheck != pDir->bReserved)
               memset(szLongName, 0, sizeof szLongName);
            fGetLongName(pDir, szLongName, sizeof szLongName);
            }
         else if (pDir->bAttr & FILE_DIRECTORY &&
                  !(pDir->bAttr & FILE_VOLID) &&
                  pDir->bFileName[0] != '.')
            {
            bCheckSum = 0;
            for (iIndex = 0; iIndex < 11; iIndex++)
               {
               if (bCheckSum & 0x01)
                  {
                  bCheckSum >>=1;
                  bCheckSum |= 0x80;
                  }
               else
                  bCheckSum >>=1;
               bCheckSum += pDir->bFileName[iIndex];
               }
            if (strlen(szLongName) && bCheck != bCheckSum)
               memset(szLongName, 0, sizeof szLongName);
            strcpy(pbPath, pszPath);
            strcat(pbPath, "\\");
            if (strlen(szLongName))
               strcat(pbPath, szLongName);
            else
               {
               MakeName(pDir, szLongName, sizeof szLongName);
               strcat(pbPath, szLongName);
               }

            memset(szLongName, 0, sizeof szLongName);
            DumpDirectory(pDrive,
               (ULONG)pDir->wClusterHigh * 0x10000 + pDir->wCluster,
               pbPath);
            }
         }
      pDir++; 
      }

   free(pbCluster);
   free(pbPath);

   return 0;
}

PSZ MakeName(PDIRENTRY pDir, PSZ pszName, USHORT usMax)
{
PSZ p;
BYTE szExtention[4];

   memset(pszName, 0, usMax);
   strncpy(pszName, pDir->bFileName, 8);
   p = pszName + strlen(pszName);
   while (p > pszName && *(p-1) == 0x20)
      p--;
   *p = 0;

   memset(szExtention, 0, sizeof szExtention);
   strncpy(szExtention, pDir->bExtention, 3);
   p = szExtention + strlen(szExtention);
   while (p > szExtention && *(p-1) == 0x20)
      p--;
   *p = 0;
   if (strlen(szExtention))
      {
      strcat(pszName, ".");
      strcat(pszName, szExtention);
      }
   return pszName;
}

VOID MarkCluster(PDRIVEINFO pDrive, ULONG ulCluster)
{
ULONG ulOffset;
USHORT usShift;
BYTE bMask;

   if (ClusterInUse(pDrive, ulCluster))
      {
      printf("Cluster %lu is crosslinked\n", ulCluster);
      }

   ulCluster -= 2;
   ulOffset = ulCluster / 8;
   usShift = (USHORT)(ulCluster % 8);
   bMask = (BYTE)(0x80 >> usShift);
   pDrive->pFatBits[ulOffset] |= bMask;
}

ULONG GetClusterCount(PDRIVEINFO pDrive, ULONG ulCluster)
{
ULONG ulCount;
ULONG ulNextCluster;

   ulCount = 0;
   if (!ulCluster)
      return ulCount;

   if (ulCluster >= pDrive->ulTotalClusters + 2)
      {
      printf("ERROR: Cluster %ld is too high!\n", ulCluster);
      return ulCount;
      }

   ulNextCluster = ulCluster;
   while (ulNextCluster && ulNextCluster != FAT_EOF)
      {
      MarkCluster(pDrive, ulNextCluster);
      ulNextCluster = GetNextCluster(pDrive, ulNextCluster);
      if (ulNextCluster != FAT_EOF && ulNextCluster >= pDrive->ulTotalClusters + 2)
         {
         printf("ERROR: FAT Appears damaged!! (FAT Sector %ld contains %8.8lX)\n",
            pDrive->ulCurFATSector, ulNextCluster);
         return ulCount;
         }
      ulCount++;
      }
   if (ulNextCluster != FAT_EOF)
      printf("ERROR: Chain started at cluster %ld was not terminated properly!\n", ulCluster);

   return ulCount;
}

APIRET AbsRead(PDRIVEINFO pDrive, ULONG ulSector, USHORT nSectors, PBYTE pvData)
{
USHORT usHead, usSector, usCylinder;

   ulSector += pDrive->ulRelStartSector - 1;

   usSector = ulSector % pDrive->nSectorsPerTrack;
   ulSector = (ulSector - usSector) / pDrive->nSectorsPerTrack;
   usHead = ulSector % pDrive->nHeads;
   usCylinder = (ulSector - usHead) / pDrive->nHeads;

   usSector++;

   /*
      Cannot read accross heads/tracks
   */
   if (usSector + nSectors > pDrive->nSectorsPerTrack + 1)
      {
      APIRET rc;
      USHORT nSectors2;

         nSectors2 = (pDrive->nSectorsPerTrack - usSector) + 1;
         rc = ReadSectors(pDrive,
            usHead, usCylinder, usSector, nSectors2, pvData);
         if (rc)
            return rc;

         pvData += nSectors2 * pDrive->bpb.BytesPerSector;
         usSector = 1;
         usHead++;
         if (usHead >= pDrive->nHeads)
            {
            usHead = 0;
            usCylinder++;
            }
         nSectors2 = nSectors - nSectors2;
         rc = ReadSectors(pDrive,
            usHead, usCylinder, usSector, nSectors2, pvData);
         if (rc)
            return rc;


         return 0;
      }

   return ReadSectors(pDrive,
      usHead, usCylinder, usSector, nSectors, pvData);
}

/**************************************************************
*
**************************************************************/
APIRET ReadSectors(PDRIVEINFO pDrive, USHORT usHead, USHORT usCylinder, USHORT usSector, USHORT nSectors, PVOID pvData)
{
ULONG         ulParmSize;
ULONG         ulDataSize;
APIRET        rc;

//   printf("Reading H:%d C:%d S:%d (%d sectors)\n",
//      usHead, usCylinder, usSector, nSectors);

   ulParmSize = pDrive->cTrackSize;
   pDrive->pTrack->bCommand      = (usSector == 1 ? 0x01 : 0x00);
   pDrive->pTrack->usHead        = usHead;
   pDrive->pTrack->usCylinder    = usCylinder;
   pDrive->pTrack->usFirstSector = usSector - 1;
   pDrive->pTrack->cSectors      = nSectors;

   ulDataSize = SECTOR_SIZE * nSectors;

   rc = DosDevIOCtl(pDrive->hDisk, IOCTL_PHYSICALDISK, PDSK_READPHYSTRACK,
      (PVOID)pDrive->pTrack, ulParmSize, &ulParmSize,
      pvData, ulDataSize, &ulDataSize);

   if (rc)
      printf("ReadSectors: DosDevIOCtl for sector failed, SYS%4.4u\n",
         rc);

   return rc;
}

BOOL ReadFATSector(PDRIVEINFO pDrive, ULONG ulSector)
{
APIRET rc;

   if (pDrive->ulCurFATSector == ulSector)
      return TRUE;

//   printf("Reading fat sector %ld : ", ulSector);
   rc = AbsRead(pDrive, pDrive->ulStartOfFAT + ulSector, 1,
      (PBYTE)pDrive->pbFATSector);
   if (rc)
      return FALSE;

   pDrive->ulCurFATSector = ulSector;

   return TRUE;
}

ULONG GetNextCluster(PDRIVEINFO pDrive, ULONG ulCluster)
{
PULONG pulCluster;

   if (!ReadFATSector(pDrive, ulCluster / 128))
      {
      printf("GetNextCluster for cluster %8.8lX failed\n",
         ulCluster);
      return FAT_EOF;
      }

   pulCluster = (PULONG)pDrive->pbFATSector + (ulCluster % 128);
   ulCluster = *pulCluster & FAT_EOF;
   if (ulCluster >= FAT_EOF2 && ulCluster <= FAT_EOF)
      return FAT_EOF;

   return ulCluster;

}

BOOL ReadCluster(PDRIVEINFO pDrive, ULONG ulCluster)
{
ULONG ulSector;

   if (ulCluster == pDrive->ulCurCluster)
      return TRUE;

   ulSector = pDrive->bpb.ReservedSectors +
      pDrive->bpb.BigSectorsPerFat * pDrive->bpb.NumberOfFATs;
   ulSector += (ulCluster - 2) * pDrive->bpb.SectorsPerCluster;

//   printf("Reading cluster %ld   ", ulCluster);

   if (!AbsRead(pDrive, ulSector, pDrive->bpb.SectorsPerCluster,
      pDrive->pbCluster))
      return FALSE;

   pDrive->ulCurCluster = ulCluster;
   return TRUE;
}

BOOL fGetLongName(PDIRENTRY pDir, PSZ pszName, USHORT wMax)
{
BYTE szLongName[15];
USHORT wNameSize;
USHORT usIndex;
PLNENTRY pLN = (PLNENTRY)pDir;

   memset(szLongName, 0, sizeof szLongName);
   wNameSize = 0;

   for (usIndex = 0; usIndex < 5; usIndex ++)
      {
      if (pLN->usChar1[usIndex] != 0xFFFF)
         szLongName[wNameSize++] = (BYTE)pLN->usChar1[usIndex];
      }
   for (usIndex = 0; usIndex < 6; usIndex ++)
      {
      if (pLN->usChar2[usIndex] != 0xFFFF)
         szLongName[wNameSize++] = (BYTE)pLN->usChar2[usIndex];
      }
   for (usIndex = 0; usIndex < 2; usIndex ++)
      {
      if (pLN->usChar3[usIndex] != 0xFFFF)
         szLongName[wNameSize++] = (BYTE)pLN->usChar3[usIndex];
      }
   if (strlen(pszName) + wNameSize > wMax)
      return FALSE;

   memmove(pszName + wNameSize, pszName, strlen(pszName) + 1);
   memcpy(pszName, szLongName, wNameSize);
   return TRUE;
}

VOID vDumpDirEntry(PDIRENTRY pDir)
{
PSZ p;
USHORT usIndex;
USHORT usStart;



   for (usStart = 0; usStart < sizeof (DIRENTRY); usStart += 16)
      {
      p = (PSZ)pDir + usStart;
      for (usIndex = 0; usIndex < 16; usIndex++, p++)
         {
         printf("%2.2X ", *p);
         }

      p = (PSZ)pDir + usStart;
      for (usIndex = 0; usIndex < 16; usIndex++, p++)
         {
         if (*p > 32)
            printf("%c", *p);
         else
            printf(".");
         }
      printf("\n");
      }

}
