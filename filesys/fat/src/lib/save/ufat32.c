/*****************************************
MSG_552, Errors
MSG_1375,VOLUMELABEL
MSG_1243,VOLSERIAL
MSG_1339 (Errors found)
MSG_1314 (Lost extended attributes)
MSG_1359,LostClusterSpace
MSG_1361,TotalDiskSpace
MSG_1363, HiddenSpace, hiddenFileCount
MSG_1364, DirSpace, DirCount
MSG_1365, UserSpace, UserFileCount
MSG_1819, ExtendedAttributes, 0
MSG_1368, FreeSpace, 0
MSG_1304, ClusterSize
MSG_1305, TotalClusters
MSG_1306, AvailableClusters
******************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <signal.h>
#include <io.h>
#include <fcntl.h>
#include <stdarg.h>
#include <conio.h>

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>
#include "portable.h"
#include "fat32def.h"

#define BLOCK_SIZE  0x4000
#define MAX_MESSAGE 2048
#define TYPE_LONG    0
#define TYPE_STRING  1
#define TYPE_PERC    2
#define TYPE_LONG2   3
#define TYPE_DOUBLE  4
#define TYPE_DOUBLE2 5

#define FAT_ERROR 0xFFFFFFFF
#define MAX_LOST_CHAINS 1024

typedef struct _DiskInfo
{
ULONG filesys_id;
ULONG sectors_per_cluster;
ULONG total_clusters;
ULONG avail_clusters;
WORD  bytes_per_sector;
} DISKINFO, *PDISKINFO;


typedef struct _CDInfo
{
BYTE        szDrive[3];
HFILE       hDisk;
BOOTSECT    BootSect;
BOOTFSINFO  FSInfo;
ULONG       ulStartOfData;
ULONG       ulActiveFatStart;
USHORT      usClusterSize;
ULONG       ulTotalClusters;
ULONG       ulCurFATSector;
BYTE        pbFATSector[512];
BOOL        fDetailed;
BOOL        fPM;
BOOL        fFix;
BOOL        fAutoRecover;
BOOL        fFatOk;
BOOL        fCleanOnBoot;
ULONG       ulFreeClusters;
ULONG       ulUserClusters;
ULONG       ulHiddenClusters;
ULONG       ulDirClusters;
ULONG       ulEAClusters;
ULONG       ulLostClusters;
ULONG       ulBadClusters;
ULONG       ulRecoveredClusters;
ULONG       ulRecoveredFiles;
ULONG       ulUserFiles;
ULONG       ulTotalDirs;
ULONG       ulHiddenFiles;
ULONG       ulErrorCount;
ULONG       ulTotalChains;
ULONG       ulFragmentedChains;
BYTE _huge * pFatBits;
DISKINFO    DiskInfo;
USHORT      usLostChains;
ULONG       rgulLost[MAX_LOST_CHAINS];
ULONG       rgulSize[MAX_LOST_CHAINS];

} CDINFO, *PCDINFO;


PRIVATE USHORT ChkDskMain(PCDINFO pCD);
PRIVATE USHORT ReadSector(PCDINFO pCD, ULONG ulSector, USHORT nSectors, PBYTE pbSector);
PRIVATE USHORT CheckFats(PCDINFO pCD);
PRIVATE USHORT CheckFiles(PCDINFO pCD);
PRIVATE USHORT CheckFreeSpace(PCDINFO pCD);
PRIVATE USHORT CheckDir(PCDINFO pCD, ULONG ulDirCluster, PSZ pszPath, ULONG ulParentDirCluster);
PRIVATE BOOL   ReadFATSector(PCDINFO pCD, ULONG ulSector);
PRIVATE ULONG  GetNextCluster(PCDINFO pCD, ULONG ulCluster);
PRIVATE USHORT SetNextCluster(PCDINFO pCD, ULONG ulCluster, ULONG ulNextCluster);
PRIVATE USHORT ReadCluster(PCDINFO pDrive, ULONG ulCluster, PBYTE pbCluster);
PRIVATE ULONG GetClusterCount(PCDINFO pCD, ULONG ulCluster, PSZ pszFile);
PRIVATE BOOL   MarkCluster(PCDINFO pCD, ULONG ulCluster, PSZ pszFile);
PRIVATE PSZ    MakeName(PDIRENTRY pDir, PSZ pszName, USHORT usMax);
PRIVATE BOOL   fGetLongName(PDIRENTRY pDir, PSZ pszName, USHORT wMax);
PRIVATE BOOL   ClusterInUse(PCDINFO pCD, ULONG ulCluster);
PRIVATE PSZ    GetOS2Error(USHORT rc);
PRIVATE INT cdecl iShowMessage(PCDINFO pCD, USHORT usNr, USHORT usNumFields, ...);
PRIVATE BOOL RecoverChain(PCDINFO pCD, ULONG ulCluster);
PRIVATE BOOL LostToFile(PCDINFO pCD, ULONG ulCluster, ULONG ulSize);
PRIVATE BOOL ClusterInChain(PCDINFO pCD, ULONG ulStart, ULONG ulCluster);

static F32PARMS  f32Parms;


int pascal CHKDSK(INT iArgc, PSZ rgArgv[], PSZ rgEnv[])
{
INT iArg;
HFILE hFile;
USHORT rc = 0;
PCDINFO pCD;
USHORT usAction;
BYTE   bSector[512];
ULONG  ulDeadFace = 0xDEADFACE;
USHORT usParmSize;
USHORT usDataSize;

   DosError(1); /* Enable hard errors */

   pCD = (PCDINFO)malloc(sizeof(CDINFO));
   if (!pCD)
      return ERROR_NOT_ENOUGH_MEMORY;
   memset(pCD, 0, sizeof (CDINFO));

   printf("\n(UFAT32.DLL version %s compiled on " __DATE__ ")\n", FAT32_VERSION);

   for (iArg = 1; iArg < iArgc; iArg++)
      {
      strupr(rgArgv[iArg]);
      if (rgArgv[iArg][0] == '/' || rgArgv[iArg][0] == '-')
         {
         switch (rgArgv[iArg][1])
            {
            case 'V':
               pCD->fDetailed = 2;
               if (rgArgv[iArg][2] == ':' && rgArgv[iArg][3] == '1')
                  pCD->fDetailed = 1;
               if (rgArgv[iArg][2] == ':' && rgArgv[iArg][3] == '2')
                  pCD->fDetailed = 2;
               break;
            case 'P':
               pCD->fPM = TRUE;
               break;
            case 'F':
               pCD->fFix = TRUE;
               break;
            case 'C':
               pCD->fAutoRecover = TRUE;
               break;
            default :
               iShowMessage(pCD, 543, 1, TYPE_STRING, rgArgv[iArg]);
               exit(543);
            }
         }
      else if (!strlen(pCD->szDrive))
         strncpy(pCD->szDrive, rgArgv[iArg], 2);
      }

   usDataSize = sizeof f32Parms;
   rc = DosFSCtl(
      (PVOID)&f32Parms, usDataSize, &usDataSize,
      NULL, 0, &usParmSize,
      FAT32_GETPARMS, "FAT32", -1, FSCTL_FSDNAME, 0L);
   if (rc)
      {
      printf("DosFSCtl, FAT32_GETPARMS failed, rc = %d\n", rc);
      DosExit(EXIT_PROCESS, 1);
      }
   if (strcmp(f32Parms.szVersion, FAT32_VERSION))
      {
      printf("ERROR: FAT32 version (%s) differs from UFAT32.DLL version (%s)\n", f32Parms.szVersion, FAT32_VERSION);
      DosExit(EXIT_PROCESS, 1);
      }


   if (!strlen(pCD->szDrive))
      {
      USHORT usDisk;
      ULONG  ulDrives;
      rc = DosQCurDisk(&usDisk, &ulDrives);
      pCD->szDrive[0] = (BYTE)(usDisk + '@');
      pCD->szDrive[1] = ':';
      }
   rc = DosOpen(pCD->szDrive,
      &hFile,
      &usAction,                          /* action taken */
      0L,                                 /* new size     */
      0,                                 /* attributes   */
      OPEN_ACTION_OPEN_IF_EXISTS,         /* open flags   */
      OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE | OPEN_FLAGS_DASD |
      OPEN_FLAGS_WRITE_THROUGH, // | OPEN_FLAGS_NO_CACHE,
      0L);
   if (rc)
      {
      if (rc == ERROR_DRIVE_LOCKED)
         iShowMessage(pCD, rc, 0);
      else
         printf("%s\n", GetOS2Error(rc));
      DosExit(1, 1);
      }
   usParmSize = sizeof ulDeadFace;
   rc = DosFSCtl(NULL, 0, 0,
                 (PBYTE)&ulDeadFace, usParmSize, &usParmSize,
                 FAT32_SECTORIO,
                 NULL,
                 hFile,
                 FSCTL_HANDLE,
                 0L);
   if (rc)
      {
      printf("DosFSCtl (SectorIO) failed:\n%s\n", GetOS2Error(rc));
      exit(1);
      }

   rc = DosDevIOCtl2((PVOID)&pCD->fCleanOnBoot, sizeof pCD->fCleanOnBoot,
      NULL, 0,
      FAT32_GETVOLCLEAN, IOCTL_FAT32, hFile);
   if (pCD->fAutoRecover && pCD->fCleanOnBoot)
      pCD->fAutoRecover = FALSE;

   if (pCD->fFix)
      {
      rc = DosDevIOCtl(NULL, NULL, DSK_LOCKDRIVE, IOCTL_DISK, hFile);
      if (rc)
         {
         if (rc == ERROR_DRIVE_LOCKED)
            iShowMessage(pCD, rc, 0);
         else
            printf("%s\n", GetOS2Error(rc));
         DosExit(1, 1);
         }
      }

   rc = DosQFSInfo(pCD->szDrive[0] - '@', FSIL_ALLOC,
      (PBYTE)&pCD->DiskInfo, sizeof (DISKINFO));
   if (rc)
      {
      fprintf(stderr, "DosQFSInfo failed, %s\n", GetOS2Error(rc));
      DosExit(1, 1);
      }
   pCD->hDisk = hFile;

   rc = ReadSector(pCD, 0, 1, bSector);
   if (rc)
      {
      printf("Error: Cannot read boot sector\n%s\n", GetOS2Error(rc));
      return rc;
      }
   memcpy(&pCD->BootSect, bSector, sizeof (BOOTSECT));

   rc = ReadSector(pCD, pCD->BootSect.bpb.FSinfoSec, 1, bSector);
   if (rc)
      {
      printf("Error: Cannot read FSInfo sector\n%s\n", GetOS2Error(rc));
      return rc;
      }

   rc = ChkDskMain(pCD);

   if (pCD->fFix)
      {
      rc = DosDevIOCtl(NULL, NULL, DSK_UNLOCKDRIVE, IOCTL_DISK, hFile);
      if (rc)
         {
         printf("The drive cannot be unlocked. SYS%4.4u\n", rc);
         DosExit(1, 1);
         }
      }
   DosClose(hFile);
   free(pCD);

   DosExit(1, rc);
   return rc;
}

USHORT ReadSector(PCDINFO pCD, ULONG ulSector, USHORT nSectors, PBYTE pbSector)
{
USHORT rc;
ULONG  ulNewOffset;
USHORT usBytesRead;

   rc = DosChgFilePtr(pCD->hDisk, ulSector, SEEK_SET, &ulNewOffset);
   if (rc)
      return rc;
   if (ulNewOffset != ulSector)
      return ERROR_SECTOR_NOT_FOUND;
   rc = DosRead(pCD->hDisk, pbSector, nSectors, &usBytesRead);
   if (rc)
      return rc;
   if (usBytesRead != nSectors)
      return ERROR_SECTOR_NOT_FOUND;
   return 0;
}

USHORT ChkDskMain(PCDINFO pCD)
{
USHORT rc;
ULONG  ulBytes;
USHORT usBlocks;
BYTE   szString[12];
PSZ    p;

   memset(szString, 0, sizeof szString);
   strncpy(szString, pCD->BootSect.VolumeLabel, 11);
   p = szString + strlen(szString);
   while (p > szString && *(p-1) == ' ')
      p--;
   *p = 0;
   iShowMessage(pCD, 1375, 1, TYPE_STRING, szString);

   sprintf(szString, "%4.4X-%4.4X",
      HIUSHORT(pCD->BootSect.ulVolSerial), LOUSHORT(pCD->BootSect.ulVolSerial));
   iShowMessage(pCD, 1243, 1, TYPE_STRING, szString);
   
   if (pCD->BootSect.bpb.MediaDescriptor != 0xF8)
      {
      printf("The media descriptor is incorrect\n");
      pCD->ulErrorCount++;
      }

   /*
      Some preparations
   */
   pCD->ulCurFATSector = 0xFFFFFFFF;
   pCD->ulActiveFatStart =  pCD->BootSect.bpb.ReservedSectors;
   if (pCD->BootSect.bpb.ExtFlags & 0x0080)
      pCD->ulActiveFatStart +=
         pCD->BootSect.bpb.BigSectorsPerFat * (pCD->BootSect.bpb.ExtFlags & 0x000F);

   pCD->ulStartOfData    = pCD->BootSect.bpb.ReservedSectors +
     pCD->BootSect.bpb.BigSectorsPerFat * pCD->BootSect.bpb.NumberOfFATs;

   pCD->usClusterSize = pCD->BootSect.bpb.BytesPerSector * pCD->BootSect.bpb.SectorsPerCluster;
   pCD->ulTotalClusters = (pCD->BootSect.bpb.BigTotalSectors - pCD->ulStartOfData) / pCD->BootSect.bpb.SectorsPerCluster;

   ulBytes = pCD->ulTotalClusters / 8 +
      (pCD->ulTotalClusters % 8 ? 1:0);
   usBlocks = (USHORT)(ulBytes / 4096 +
            (ulBytes % 4096 ? 1:0));

   pCD->pFatBits = halloc(usBlocks,4096);
   if (!pCD->pFatBits)
      {
      printf("Not enough memory for FATBITS\n");
      return ERROR_NOT_ENOUGH_MEMORY;
      }


   rc = CheckFats(pCD);
   if (rc)
      {
      printf("The copies of the FATs do not match.\n");
      printf("Please run SCANDISK to correct this problem.\n");
      return rc;
      }
   rc = CheckFiles(pCD);
   rc = CheckFreeSpace(pCD);

   printf("\n\n");

   if (pCD->DiskInfo.total_clusters != pCD->ulTotalClusters)
      printf("Total clusters mismatch!\n");

   if (pCD->DiskInfo.avail_clusters != pCD->ulFreeClusters)
      {
      printf("The stored free disk space is incorrect.\n");
      printf("(%lu free allocation units are reported while %lu free units are detected.)\n",
         pCD->DiskInfo.avail_clusters, pCD->ulFreeClusters);
      if (pCD->fFix)
         {
         ULONG ulFreeBlocks;
         rc = DosDevIOCtl2(&ulFreeBlocks, sizeof ulFreeBlocks,
            NULL, 0,
            FAT32_GETFREESPACE, IOCTL_FAT32, pCD->hDisk);
         if (!rc)
            printf("The correct free space is set to %lu allocation units.\n",
               ulFreeBlocks);
         else
            {
            printf("Setting correct free space failed, rc = %u.\n", rc);
            pCD->ulErrorCount++;
            }
         }
      else
         pCD->ulErrorCount++;
      }

   iShowMessage(pCD, 1361, 1,
      TYPE_DOUBLE, (DOUBLE)pCD->ulTotalClusters * pCD->usClusterSize);
   if (pCD->ulBadClusters)
      iShowMessage(pCD, 1362, 1,
         TYPE_DOUBLE, (DOUBLE)pCD->ulBadClusters * pCD->usClusterSize);
   iShowMessage(pCD, 1363, 2,
      TYPE_DOUBLE, (DOUBLE)pCD->ulHiddenClusters * pCD->usClusterSize,
      TYPE_LONG, pCD->ulHiddenFiles);
   iShowMessage(pCD, 1364, 2,
      TYPE_DOUBLE, (DOUBLE)pCD->ulDirClusters * pCD->usClusterSize,
      TYPE_LONG, pCD->ulTotalDirs);
   iShowMessage(pCD, 1819, 1,
      TYPE_DOUBLE, (DOUBLE)pCD->ulEAClusters * pCD->usClusterSize);
   iShowMessage(pCD, 1365, 2, 
      TYPE_DOUBLE, (DOUBLE)pCD->ulUserClusters * pCD->usClusterSize,
      TYPE_LONG, pCD->ulUserFiles);

   if (pCD->ulRecoveredClusters)
      iShowMessage(pCD, 1365, 2, 
         TYPE_DOUBLE, (DOUBLE)pCD->ulRecoveredClusters * pCD->usClusterSize,
         TYPE_LONG, pCD->ulRecoveredFiles);

   if (pCD->ulLostClusters)
      iShowMessage(pCD, 1359, 1,
         TYPE_DOUBLE, (DOUBLE)pCD->ulLostClusters * pCD->usClusterSize);


   iShowMessage(pCD, 1368, 2,
      TYPE_DOUBLE, (DOUBLE)pCD->ulFreeClusters * pCD->usClusterSize,
      TYPE_LONG, 0L);

   printf("\n");


   iShowMessage(pCD, 1304, 1,
      TYPE_LONG, (ULONG)pCD->usClusterSize);

   iShowMessage(pCD, 1305, 1,
      TYPE_LONG, pCD->ulTotalClusters);

   iShowMessage(pCD, 1306, 1,
      TYPE_LONG, pCD->ulFreeClusters);

   if (pCD->ulTotalChains > 0)
      printf("\n%u%% of the files and directories are fragmented.\n",
         (USHORT)(pCD->ulFragmentedChains * 100 / pCD->ulTotalChains));

   if (pCD->ulErrorCount)
      {
      printf("\n");
      if (!pCD->fFix)
         iShowMessage(pCD, 1339, 0);
      else
         printf("Still errors found on disk. Please run Windows 95 ScanDisk!\n");
      }
   else if (pCD->fFix)
      DosDevIOCtl(NULL, NULL, FAT32_MARKVOLCLEAN, IOCTL_FAT32, pCD->hDisk);

   return 0;
}

USHORT CheckFats(PCDINFO pCD)
{
PBYTE pSector;
USHORT nFat;
ULONG ulSector;
USHORT rc;
USHORT usPerc = 0xFFFF;
BOOL   fDiff;
ULONG  ulCluster;
USHORT usIndex;
PULONG pulCluster;
USHORT fRetco;

//   printf("Each fat contains %lu sectors\n",
//      pCD->BootSect.bpb.BigSectorsPerFat);

   printf("CHKDSK is checking fats :    ");

   if (pCD->BootSect.bpb.ExtFlags & 0x0080)
      {
      printf("There is only one active FAT.\n");
      return 0;
      }


   pSector = halloc(pCD->BootSect.bpb.NumberOfFATs, BLOCK_SIZE);
   if (!pSector)
      return ERROR_NOT_ENOUGH_MEMORY;


   fDiff = FALSE;
   ulCluster = 0L;
   fRetco = 0;
   for (ulSector = 0; ulSector < pCD->BootSect.bpb.BigSectorsPerFat; ulSector+=32)
      {
      USHORT usNew  = (USHORT)(ulSector * 100 / pCD->BootSect.bpb.BigSectorsPerFat);
      USHORT nSectors;


      if (!pCD->fPM && usNew != usPerc)
         {
         printf("\b\b\b\b%3u%%", usNew);
         usPerc = usNew;
         }

      nSectors = BLOCK_SIZE / 512;
      if ((ULONG)nSectors > pCD->BootSect.bpb.BigSectorsPerFat - ulSector)
         nSectors = (USHORT)(pCD->BootSect.bpb.BigSectorsPerFat - ulSector);


      for (nFat = 0; nFat < pCD->BootSect.bpb.NumberOfFATs; nFat++)
         {
         rc = ReadSector(pCD,
            pCD->ulActiveFatStart + (nFat * pCD->BootSect.bpb.BigSectorsPerFat) + ulSector,
               nSectors, pSector + nFat * BLOCK_SIZE);
         }
      for (nFat = 0; nFat < (USHORT)(pCD->BootSect.bpb.NumberOfFATs - 1); nFat++)
         {
         if (memcmp(pSector + nFat * BLOCK_SIZE,
                    pSector + nFat * BLOCK_SIZE + BLOCK_SIZE,
                    nSectors * 512))
            fDiff = TRUE;
         }

      pulCluster = (PULONG)pSector;
      if (!ulSector)
         {
         pulCluster += 2;
         ulCluster = 2;
         usIndex = 2;
         }
      else
         usIndex = 0;
      for (; ulCluster < pCD->ulTotalClusters + 2 && usIndex < nSectors * 128; usIndex++)
         {
         if ((*pulCluster & FAT_EOF) >= pCD->ulTotalClusters + 2)
            {
            ULONG ulVal = *pulCluster & FAT_EOF;
            if (!(ulVal >= FAT_BAD_CLUSTER && ulVal <= FAT_EOF))
               {
               printf("FAT Entry for cluster %lu contains an invalid value.\n",
                  ulCluster);
               fRetco = 1;
               }
            }
         pulCluster++;
         ulCluster++;
         }
      

      }
   printf("\b\b\b\b");
   if (fDiff)
      {
      printf("\n");
      iShowMessage(pCD, 1374, 1, TYPE_STRING, pCD->szDrive);
      pCD->ulErrorCount++;
      pCD->fFatOk = FALSE;
      fRetco = 1;
      }
   else
      {
      printf("Ok.   \n");
      pCD->fFatOk = TRUE;
      }
   
   free(pSector);
   return fRetco;
}


USHORT CheckFiles(PCDINFO pCD)
{

   printf("CHKDSK is checking files and directories...\n");
   return CheckDir(pCD, pCD->BootSect.bpb.RootDirStrtClus, pCD->szDrive, 0L);
}

USHORT CheckFreeSpace(PCDINFO pCD)
{
ULONG ulCluster;
USHORT usPerc = 100;
BOOL fMsg = FALSE;

   iShowMessage(pCD, 564, 0);

   pCD->ulFreeClusters = 0;
   for (ulCluster = 0; ulCluster < pCD->ulTotalClusters; ulCluster++)
      {
      USHORT usNew  = (USHORT)(ulCluster * 100 / pCD->ulTotalClusters);
      ULONG ulNext = GetNextCluster(pCD, ulCluster + 2);

      if (!pCD->fPM && usNew != usPerc)
         {
         iShowMessage(pCD, 563, 1, TYPE_PERC, usNew);
         printf("\r");
         usPerc = usNew;
         }
      if (ulNext == FAT_BAD_CLUSTER)
         {
         pCD->ulBadClusters++;
         MarkCluster(pCD, ulCluster+2, "Bad sectors");
         }
      else if (!ulNext)
         {
         MarkCluster(pCD, ulCluster+2, "Free space");
         pCD->ulFreeClusters++;
         }
      else
         {
         if (!ClusterInUse(pCD, ulCluster+2))
            {
            if (!fMsg)
               {
               printf("\n");
               iShowMessage(pCD, 562, 1, TYPE_STRING, pCD->szDrive);
               iShowMessage(pCD, 563, 1, TYPE_PERC, usNew);
               printf("\r");
               fMsg = TRUE;
               }
            RecoverChain(pCD, ulCluster+2);
            }
         }
      }

   iShowMessage(pCD, 563, 1, TYPE_PERC, 100);
   printf("\n");

   if (pCD->usLostChains)
      {
      BYTE bChar;
      USHORT usIndex;
      USHORT rc;

      if (pCD->usLostChains >= MAX_LOST_CHAINS)
         iShowMessage(pCD, 548, 0);

      if (!pCD->fAutoRecover)
         iShowMessage(pCD, 1356, 2,
            TYPE_LONG2, pCD->ulLostClusters,
            TYPE_LONG2, (ULONG)pCD->usLostChains);

      for (;;)
         {
         if (!pCD->fAutoRecover)
            bChar = (BYTE)getch();
         else
            bChar = 'Y';
         if (bChar == 'Y' || bChar == 'y')
            {
            bChar = 'Y';
            break;
            }
         if (bChar == 'N' || bChar == 'n')
            {
            bChar = 'N';
            break;
            }
         }
      printf("\n");


      if (pCD->fFix)
         {
         for (usIndex = 0; usIndex < MAX_LOST_CHAINS &&
                           usIndex < pCD->usLostChains; usIndex++)
            {
            pCD->ulLostClusters -= pCD->rgulSize[usIndex];
            if (bChar == 'Y')
               LostToFile(pCD, pCD->rgulLost[usIndex], pCD->rgulSize[usIndex]);
            else
               {
               rc = DosDevIOCtl2(NULL, 0,
                  (PVOID)&pCD->rgulLost[usIndex], 4,
                  FAT32_DELETECHAIN, IOCTL_FAT32, pCD->hDisk);
               if (rc)
                  {
                  printf("CHKDSK was unable to delete a lost chain.\n");
                  pCD->ulErrorCount++;
                  }
               else
                  pCD->ulFreeClusters += pCD->rgulSize[usIndex];
               }
            }
         }
      }
   return 0;
}

BOOL ClusterInUse(PCDINFO pCD, ULONG ulCluster)
{
ULONG ulOffset;
USHORT usShift;
BYTE bMask;

   if (ulCluster >= pCD->ulTotalClusters + 2)
      {
      printf("An invalid cluster number %8.8lX was found.\n", ulCluster);
      return TRUE;
      }

   ulCluster -= 2;
   ulOffset = ulCluster / 8;
   usShift = (USHORT)(ulCluster % 8);
   bMask = (BYTE)(0x80 >> usShift);
   if (pCD->pFatBits[ulOffset] & bMask)
      return TRUE;
   else
      return FALSE;
}


USHORT CheckDir(PCDINFO pCD, ULONG ulDirCluster, PSZ pszPath, ULONG ulParentDirCluster)
{
static BYTE szLongName[512] = "";
static BYTE szShortName[13] = "";
static MARKFILEEASBUF Mark;

int iIndex;
DIRENTRY _huge * pDir;
DIRENTRY _huge * pEnd;
PBYTE pbCluster;
PBYTE pbPath;
ULONG ulCluster;
ULONG ulClusters;
ULONG ulBytesNeeded;
BYTE _huge * p;
BYTE bCheckSum, bCheck;
ULONG ulClustersNeeded;
ULONG ulClustersUsed;
ULONG ulEntries;
PBYTE pEA;
USHORT rc;

   if (!ulDirCluster)
      {
      printf("ERROR: Cluster for %s is 0!\n", pszPath);
      return TRUE;
      }

   pCD->ulTotalDirs++;

   pbPath = malloc(512);
   strcpy(pbPath, "Directory ");
   strcat(pbPath, pszPath);
   ulClusters = GetClusterCount(pCD, ulDirCluster, pbPath);

   pCD->ulDirClusters += ulClusters;

   if (pCD->fDetailed == 2)
      printf("\n\nDirectory of %s (%lu clusters)\n\n", pszPath, ulClusters);

   ulBytesNeeded = (ULONG)pCD->BootSect.bpb.SectorsPerCluster * (ULONG)pCD->BootSect.bpb.BytesPerSector * ulClusters;
   pbCluster = halloc(ulClusters, pCD->BootSect.bpb.SectorsPerCluster * pCD->BootSect.bpb.BytesPerSector);
   if (!pbCluster)
      {
      printf("ERROR:Directory %s is too large ! (Not enough memory!)\n", pszPath);
      return ERROR_NOT_ENOUGH_MEMORY;
      }

   ulCluster = ulDirCluster;
   p = pbCluster;
   while (ulCluster != FAT_EOF)
      {
      ReadCluster(pCD, ulCluster, p);
      ulCluster = GetNextCluster(pCD, ulCluster);
      if (!ulCluster)
         ulCluster = FAT_EOF;
      p += pCD->BootSect.bpb.SectorsPerCluster * pCD->BootSect.bpb.BytesPerSector;
      }

   memset(szLongName, 0, sizeof szLongName);
   pDir = (DIRENTRY _huge *)pbCluster;
   pEnd = (DIRENTRY _huge *)(p - sizeof (DIRENTRY));
   ulEntries = 0;
   bCheck = 0;
   while (pDir <= pEnd)
      {
      if (pDir->bFileName[0] && pDir->bFileName[0] != 0xE5)
         {
         if (pDir->bAttr == FILE_LONGNAME)
            {
            if (strlen(szLongName) && bCheck != pDir->bReserved)
               {
               printf("A lost long filename was found: %s\n",
                  szLongName);
               pCD->ulErrorCount++;
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
               pCD->ulErrorCount++;
               printf("The longname %s does not belong to %s\\%s\n",
                  szLongName, pszPath, MakeName(pDir, szShortName, sizeof szShortName));
               memset(szLongName, 0, sizeof szLongName);
               }

            if (pCD->fDetailed == 2)
               {
               printf("%-13.13s", MakeName(pDir, szShortName, sizeof szShortName));
               if (pDir->bAttr & FILE_DIRECTORY)
                  printf("<DIR>      ");
               else
                  printf("%10lu ", pDir->ulFileSize);

//               printf("%8.8lX ", MAKEP(pDir->wClusterHigh, pDir->wCluster));

               printf("%s ", szLongName);
               }

            /*
               Construct full path
            */
            strcpy(pbPath, pszPath);
            if (pbPath[strlen(pbPath) - 1] != '\\')
               strcat(pbPath, "\\");
            if (strlen(szLongName))
               strcat(pbPath, szLongName);
            else
               {
               MakeName(pDir, szLongName, sizeof szLongName);
               strcat(pbPath, szLongName);
               }


            if (pDir->fEAS)
               {
               FILESTATUS fStat;
               
               strcpy(Mark.szFileName, pbPath);
               strcat(Mark.szFileName, EA_EXTENTION);
               rc = DosQPathInfo(Mark.szFileName, FIL_STANDARD, (PBYTE)&fStat, sizeof fStat, 0L);
               if (rc)
                  {
                  printf("%s is marked having EAs, but the EA file (%s) is not found. (SYS%4.4u)\n", pbPath, Mark.szFileName, rc);
                  if (pCD->fFix)
                     {
                     strcpy(Mark.szFileName, pbPath);
                     Mark.fEAS = FILE_HAS_NO_EAS;
                     rc = DosDevIOCtl2(NULL, 0,
                        (PVOID)&Mark, sizeof Mark,
                        FAT32_SETEAS, IOCTL_FAT32, pCD->hDisk);
                     if (!rc)
                        printf("This has been corrected.\n");
                     else
                        printf("SYS%4.4u: Unable to correct problem.\n", rc);
                     }
                  }
               else if (!fStat.cbFile)
                  {
                  printf("%s is marked having EAs, but the EA file (%s) is empty.\n", pbPath, Mark.szFileName);
                  if (pCD->fFix)
                     {
                     unlink(Mark.szFileName);
                     strcpy(Mark.szFileName, pbPath);
                     Mark.fEAS = FILE_HAS_NO_EAS;
                     rc = DosDevIOCtl2(NULL, 0,
                        (PVOID)&Mark, sizeof Mark,
                        FAT32_SETEAS, IOCTL_FAT32, pCD->hDisk);
                     if (!rc)
                        printf("This has been corrected.\n");
                     else
                        printf("SYS%4.4u: Unable to correct problem.\n", rc);
                     }
                  }
               }

            if (!(pDir->bAttr & FILE_DIRECTORY))
               {
               ulClustersNeeded = pDir->ulFileSize / pCD->usClusterSize +
                  (pDir->ulFileSize % pCD->usClusterSize ? 1:0);
               ulClustersUsed = GetClusterCount(pCD,(ULONG)pDir->wClusterHigh * 0x10000 + pDir->wCluster, pbPath);
               pEA = strstr(pbPath, EA_EXTENTION);
               if (pEA && pDir->ulFileSize)
                  {
                  USHORT rc;

                  pCD->ulEAClusters += ulClustersUsed;

                  memset(&Mark, 0, sizeof Mark);
                  memcpy(Mark.szFileName, pbPath, pEA - pbPath);

                  rc = DosDevIOCtl2(NULL, 0,
                     (PVOID)&Mark, sizeof Mark,
                     FAT32_QUERYEAS, IOCTL_FAT32, pCD->hDisk);
                  if (rc == 2 || rc == 3)
                     {
                     printf("A lost Extended attribute was found (for %s)\n",
                        Mark.szFileName);
                     if (pCD->fFix)
                        {
                        rc = DosSetFileMode(pbPath, FILE_NORMAL, 0L);
                        if (!rc)
                           rc = DosMove(pbPath, Mark.szFileName, 0L);
                        if (!rc)
                           printf("This attribute has been converted to a file \n(%s).\n", Mark.szFileName);
                        else
                           printf("SYS%4.4u: Cannot convert %s\n",
                              rc, pbPath);
                        }
                     }
                  else if (rc)
                     {
                     printf("SYS%4.4u occured while retrieving EA flag for %s.\n", rc, Mark.szFileName);
                     }
                  else
                     {
                     if (Mark.fEAS == FILE_HAS_NO_EAS)
                        {
                        printf("EAs detected for %s, but it is not marked having EAs.\n", Mark.szFileName);
                        if (pCD->fFix)
                           {
                           Mark.fEAS = FILE_HAS_EAS;
                           rc = DosDevIOCtl2(NULL, 0,
                              (PVOID)&Mark, sizeof Mark,
                              FAT32_SETEAS, IOCTL_FAT32, pCD->hDisk);
                           if (!rc)
                              printf("This has been corrected.\n");
                           else
                              printf("SYS%4.4u: Unable to correct problem.\n", rc);
                           }
                        }
                     }
                  }
               else if (pDir->bAttr & FILE_HIDDEN)
                  {                 
                  pCD->ulHiddenClusters += ulClustersUsed;
                  pCD->ulHiddenFiles++;
                  }
               else
                  {
                  pCD->ulUserClusters += ulClustersUsed;
                  pCD->ulUserFiles++;
                  }
               if (ulClustersNeeded != ulClustersUsed)
                  {
                  if (!pCD->fFix)
                     {
                     printf("File allocation error detected for %s\\%s\n",
                        pszPath, MakeName(pDir, szShortName, sizeof szShortName));
                     pCD->ulErrorCount++;
                     }
                  else
                     {
                     FILESIZEDATA fs;
                     USHORT rc;

                     memset(&fs, 0, sizeof fs);
                     strcpy(fs.szFileName, pszPath);
                     if (fs.szFileName[strlen(pszPath) - 1] != '\\')
                        strcat(fs.szFileName, "\\");
                     strcat(fs.szFileName, MakeName(pDir, szShortName, sizeof szShortName));
                     fs.ulFileSize = ulClustersUsed * pCD->usClusterSize;

                     rc = DosDevIOCtl2(NULL, 0,
                        (PVOID)&fs, sizeof fs,
                        FAT32_SETFILESIZE, IOCTL_FAT32, pCD->hDisk);
                     if (rc)
                        {
                        printf("File allocation error detected for %s.\n",
                           fs.szFileName);
                        printf("CHKDSK was unable to correct the filesize. SYS%4.4u.\n", rc);
                        pCD->ulErrorCount++;
                        }
                     else
                        iShowMessage(pCD, 560, 1,
                           TYPE_STRING, fs.szFileName);
                     }

                  }
               }
            if (pCD->fDetailed == 2)
               printf("\n");


            memset(szLongName, 0, sizeof szLongName);
            }
         ulEntries++;
         }
      pDir++;

      }
   if (pCD->fDetailed == 2)
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
            bCheck = pDir->bReserved;
            fGetLongName(pDir, szLongName, sizeof szLongName);
            }
         else if (pDir->bAttr & FILE_DIRECTORY &&
                  !(pDir->bAttr & FILE_VOLID))
            {
            ulCluster = (ULONG)pDir->wClusterHigh * 0x10000 + pDir->wCluster;
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

            if (!memicmp(pDir->bFileName,      ".          ", 11)) 
               {
               if (ulCluster != ulDirCluster)
                  printf(". entry in %s is incorrect!\n", pszPath); 
               }
            else if (!memicmp(pDir->bFileName, "..         ", 11))
               {
               if (ulCluster != ulParentDirCluster)
                  printf(".. entry in %s is incorrect! (%lX %lX)\n",
                     pszPath, ulCluster, ulParentDirCluster); 
               }
            else
               {
               /*
                  Construct full path
               */
               strcpy(pbPath, pszPath);
               if (pbPath[strlen(pbPath) - 1] != '\\')
                  strcat(pbPath, "\\");
               if (strlen(szLongName))
                  strcat(pbPath, szLongName);
               else
                  {
                  MakeName(pDir, szLongName, sizeof szLongName);
                  strcat(pbPath, szLongName);
                  }

               memset(szLongName, 0, sizeof szLongName);
               CheckDir(pCD,
                  (ULONG)pDir->wClusterHigh * 0x10000 + pDir->wCluster,
                  pbPath, (ulDirCluster == pCD->BootSect.bpb.RootDirStrtClus ? 0 : ulDirCluster));
               }
            }
         }
      pDir++; 
      }

   free(pbCluster);
   free(pbPath);

   return 0;
}


ULONG GetClusterCount(PCDINFO pCD, ULONG ulCluster, PSZ pszFile)
{
ULONG ulCount;
ULONG ulNextCluster;
BOOL  fCont = TRUE;
BOOL  fShown = FALSE;

   ulCount = 0;
   if (!ulCluster)
      return ulCount;

   if (ulCluster  > pCD->ulTotalClusters + 2)
      {
      printf("Invalid start of clusterchain %lX found for %s\n",
         ulCluster, pszFile);
      return 0;
      }

   while (ulCluster != FAT_EOF)
      {
      ulNextCluster = GetNextCluster(pCD, ulCluster);
      if (!MarkCluster(pCD, ulCluster, pszFile))
         return ulCount;
      ulCount++;

      if (!ulNextCluster)
         {
         if (pCD->fFix)
            {
            printf("CHKDSK found an improperly terminated cluster chain for %s ", pszFile);
            if (SetNextCluster(pCD, ulCluster, FAT_EOF))
               {
               printf(", but was unable to fix it.\n");
               pCD->ulErrorCount++;
               }
            else
               printf(" and corrected the problem.\n");
            }
         else
            {
            printf("A bad terminated cluster chain was found for %s\n", pszFile);
            pCD->ulErrorCount++;
            }
         ulNextCluster = FAT_EOF;
         }

      if (ulNextCluster != FAT_EOF && ulNextCluster != ulCluster + 1)
         {
         if (pCD->fDetailed)
            {
            if (!fShown)
               {
               printf("%s is fragmented\n", pszFile);
               fShown = TRUE;
               }
            }
         fCont = FALSE;
         }
      ulCluster = ulNextCluster;
      }
   pCD->ulTotalChains++;
   if (!fCont)
      pCD->ulFragmentedChains++;
   return ulCount;
}

BOOL MarkCluster(PCDINFO pCD, ULONG ulCluster, PSZ pszFile)
{
ULONG ulOffset;
USHORT usShift;
BYTE bMask;

   if (ClusterInUse(pCD, ulCluster))
      {
      iShowMessage(pCD, 1343, 2,
         TYPE_STRING, pszFile,
         TYPE_LONG, ulCluster);
      pCD->ulErrorCount++;
      return FALSE;
      }

   ulCluster -= 2;
   ulOffset = ulCluster / 8;
   usShift = (USHORT)(ulCluster % 8);
   bMask = (BYTE)(0x80 >> usShift);
   pCD->pFatBits[ulOffset] |= bMask;
   return TRUE;
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

BOOL fGetLongName(PDIRENTRY pDir, PSZ pszName, USHORT wMax)
{
static BYTE szLongName[15] = "";
static USHORT uniName[15] = {0};
USHORT wNameSize;
USHORT usIndex;
USHORT usDataSize;
USHORT usParmSize;
PLNENTRY pLN = (PLNENTRY)pDir;

   memset(szLongName, 0, sizeof szLongName);
   memset(uniName, 0, sizeof uniName);

   wNameSize = 0;
   for (usIndex = 0; usIndex < 5; usIndex ++)
      {
      if (pLN->usChar1[usIndex] != 0xFFFF)
         uniName[wNameSize++] = pLN->usChar1[usIndex];
      }
   for (usIndex = 0; usIndex < 6; usIndex ++)
      {
      if (pLN->usChar2[usIndex] != 0xFFFF)
         uniName[wNameSize++] = pLN->usChar2[usIndex];
      }
   for (usIndex = 0; usIndex < 2; usIndex ++)
      {
      if (pLN->usChar3[usIndex] != 0xFFFF)
         uniName[wNameSize++] = pLN->usChar3[usIndex];
      }

   if (strlen(pszName) + wNameSize > wMax)
      return FALSE;

   usDataSize = sizeof szLongName;
   usParmSize = sizeof uniName;
   DosFSCtl(szLongName, usDataSize, &usDataSize,
         (PVOID)uniName, usParmSize, &usParmSize,
         FAT32_WIN2OS,
         "FAT32",
         -1,
         FSCTL_FSDNAME,
         0L);

   memmove(pszName + wNameSize, pszName, strlen(pszName) + 1);
   memcpy(pszName, szLongName, wNameSize);
   return TRUE;
}

USHORT ReadCluster(PCDINFO pCD, ULONG ulCluster, PBYTE pbCluster)
{
ULONG ulSector;
USHORT rc;

   ulSector = pCD->ulStartOfData +
      (ulCluster - 2) * pCD->BootSect.bpb.SectorsPerCluster;

   rc = ReadSector(pCD, ulSector, pCD->BootSect.bpb.SectorsPerCluster,
      pbCluster);
   if (rc)
      return rc;

   return 0;
}


ULONG GetNextCluster(PCDINFO pCD, ULONG ulCluster)
{
PULONG pulCluster;
ULONG  ulSector;
ULONG  ulRet;
   
   ulSector = ulCluster / 128;
   if (!ReadFATSector(pCD, ulSector))
      return FAT_EOF;

   pulCluster = (PULONG)pCD->pbFATSector + ulCluster % 128;

   ulRet = *pulCluster & FAT_EOF;
   if (ulRet >= FAT_EOF2 && ulRet <= FAT_EOF)
      return FAT_EOF;

   if (ulRet >= pCD->ulTotalClusters  + 2)
      {
      printf("Error: Next cluster for %lu = %8.8lX\n",
         ulCluster, *pulCluster);
      return FAT_EOF;
      }

   return ulRet;
}

USHORT SetNextCluster(PCDINFO pCD, ULONG ulCluster, ULONG ulNextCluster)
{
SETCLUSTERDATA SetCluster;
USHORT rc;
PULONG pulCluster;
ULONG  ulSector;
ULONG  ulRet;


   SetCluster.ulCluster = ulCluster;
   SetCluster.ulNextCluster = ulNextCluster;


   rc = DosDevIOCtl2(NULL, 0,
      (PVOID)&SetCluster, sizeof SetCluster,
      FAT32_SETCLUSTER, IOCTL_FAT32, pCD->hDisk);
   if (rc)
      return rc;

   ulSector = ulCluster / 128;
   if (!ReadFATSector(pCD, ulSector))
      return ERROR_SECTOR_NOT_FOUND;

   pulCluster = (PULONG)pCD->pbFATSector + ulCluster % 128;
   *pulCluster = ulNextCluster;
   return 0;
}

BOOL ReadFATSector(PCDINFO pCD, ULONG ulSector)
{
USHORT rc;

   if (pCD->ulCurFATSector == ulSector)
      return TRUE;

   rc = ReadSector(pCD, pCD->ulActiveFatStart + ulSector, 1,
      (PBYTE)pCD->pbFATSector);
   if (rc)
      return FALSE;

   pCD->ulCurFATSector = ulSector;

   return TRUE;
}

int pascal FORMAT(INT iArgc, PSZ rgArgv[], PSZ rgEnv[])
{
   printf("\nThis function is not supported\n");
   return ERROR_NOT_SUPPORTED;
}

int pascal RECOVER(INT iArgc, PSZ rgArgv[], PSZ rgEnv[])
{
   printf("\nThis function is not supported\n");
   return ERROR_NOT_SUPPORTED;
}

int pascal SYS(INT iArgc, PSZ rgArgv[], PSZ rgEnv[])
{
   printf("\nThis function is not supported\n");
   return ERROR_NOT_SUPPORTED;
}

PSZ GetOS2Error(USHORT rc)
{
static BYTE szErrorBuf[MAX_MESSAGE] = "";
static BYTE szErrNo[12] = "";
USHORT rc2;
USHORT usReplySize;

   memset(szErrorBuf, 0, sizeof szErrorBuf);
   if (rc)
      {
      sprintf(szErrNo, "SYS%4.4u: ", rc);
      rc2 = DosGetMessage(NULL, 0, szErrorBuf, sizeof(szErrorBuf),
                          rc, "OSO001.MSG", &usReplySize);
      switch (rc2)
         {
         case NO_ERROR:
            strcat(szErrorBuf, "\n");
            break;
         case ERROR_FILE_NOT_FOUND :
            sprintf(szErrorBuf, "SYS%04u (Message file not found!)", rc);
            break;
         default:
            sprintf(szErrorBuf, "SYS%04u (Error %d while retrieving message text!)", rc, rc2);
            break;
         }
      DosGetMessage(NULL,
         0,
         szErrorBuf + strlen(szErrorBuf),
         sizeof(szErrorBuf) - strlen(szErrorBuf),
         rc,
         "OSO001H.MSG",
         &usReplySize);
      }

   if (memicmp(szErrorBuf, "SYS", 3))
      {
      memmove(szErrorBuf + strlen(szErrNo), szErrorBuf, strlen(szErrorBuf) + 1);
      memcpy(szErrorBuf, szErrNo, strlen(szErrNo));
      }
   return szErrorBuf;
}


void * cdecl malloc(size_t tSize)
{
USHORT rc;
SEL Sel;

   rc = DosAllocSeg(tSize, &Sel, 0);
   if (rc)
      return NULL;
   return MAKEP(Sel, 0);
}

void cdecl free(void * pntr)
{
SEL Sel;

   Sel = SELECTOROF(pntr);
   DosFreeSeg(Sel);
}

void _huge * cdecl halloc(long num, size_t tSize)
{
USHORT rc;
SEL Sel;
ULONG ulTotalSize;
USHORT cSegs;
USHORT cPartial;
LONG lIndex;
BYTE _huge * pRet;
BYTE _huge * pWork;


   ulTotalSize = num * tSize;
   cSegs = (USHORT)(ulTotalSize / 0x10000);
   cPartial = (USHORT)(ulTotalSize % 0x10000);


   rc = DosAllocHuge(cSegs, cPartial, &Sel, 0, 0);
   if (rc)
      return NULL;

   pRet = MAKEP(Sel, 0);

   pWork = pRet;
   for (lIndex = 0; lIndex < num; lIndex++)
      {
      memset(pWork, 0, tSize);
      pWork += tSize;
      }

   return pRet;
}

INT cdecl iShowMessage(PCDINFO pCD, USHORT usNr, USHORT usNumFields, ...)
{
static BYTE szErrNo[12] = "";
static BYTE szMessage[MAX_MESSAGE];
static BYTE szOut[MAX_MESSAGE];
static BYTE rgNum[9][50];
USHORT usReplySize;
USHORT rc;
PSZ    rgPSZ[9];
va_list va;
USHORT usIndex;
PSZ    pszMess;

   va_start(va, usNumFields);

   memset(szMessage, 0, sizeof szMessage);
   rc = DosGetMessage(NULL, 0, szMessage, sizeof(szMessage),
                          usNr, "OSO001.MSG", &usReplySize);
   switch (rc)
      {
      case NO_ERROR:
         break;
      case ERROR_FILE_NOT_FOUND :
         sprintf(szMessage, "SYS%04u (Message file not found!)", usNr);
         break;
      default:
         sprintf(szMessage, "SYS%04u (Error %d while retrieving message text!)", usNr, rc);
         break;
      }
   if (rc)
      {
      printf("%s\n", szMessage);
      return 0;
      }

   pszMess = szMessage;

   memset(szOut, 0, sizeof szOut);
   if (pCD->fPM)
      sprintf(szOut, "MSG_%u", usNr);

   memset(rgNum, 0, sizeof rgNum);
   for (usIndex = 0; usIndex < usNumFields; usIndex++)
      {
      USHORT usType = va_arg(va, USHORT);
      if (usType == TYPE_PERC)
         {
         sprintf(rgNum[usIndex], "%3u", va_arg(va, USHORT));
         rgPSZ[usIndex] = rgNum[usIndex];
         }
      else if (usType == TYPE_LONG || usType == TYPE_LONG2)
         {
         if (!usIndex && usType == TYPE_LONG)
            sprintf(rgNum[usIndex], "%12lu", va_arg(va, ULONG));
         else
            sprintf(rgNum[usIndex], "%lu", va_arg(va, ULONG));
         rgPSZ[usIndex] = rgNum[usIndex];
         }
      else if (usType == TYPE_DOUBLE || usType == TYPE_DOUBLE2)
         {
         PSZ p;
         if (!usIndex && usType == TYPE_DOUBLE)
            sprintf(rgNum[usIndex], "%12.0lf", va_arg(va, DOUBLE));
         else
            sprintf(rgNum[usIndex], "%lf", va_arg(va, DOUBLE));
         p = strchr(rgNum[usIndex], '.');
         if (p)
            *p = 0;
         rgPSZ[usIndex] = rgNum[usIndex];
         }
      else if (usType == TYPE_STRING)
         {
         rgPSZ[usIndex] = va_arg(va, PSZ);
         }
      if (pCD->fPM)
         {
         PSZ p;
         strcat(szOut, ", ");
         p = rgPSZ[usIndex];
         while (*p == 0x20)
            p++;
         strcat(szOut, p);
         }
      }

   if (pCD->fPM)
      {
      strcat(szOut, "\n");
//      printf(szOut);
//      return usNumFields;
      }

   rc = DosInsMessage(rgPSZ,
      usNumFields,
      pszMess,
      strlen(pszMess),
      szOut + strlen(szOut),
      sizeof szOut - strlen(szOut),
      &usReplySize);
   printf("%s", szOut);

   return usNumFields;
}


BOOL RecoverChain(PCDINFO pCD, ULONG ulCluster)
{
ULONG  ulSize;
USHORT usIndex;


   if (!pCD->fFix)
      {
      pCD->ulErrorCount++;
      pCD->ulLostClusters ++;
      return TRUE;
      }


   for (usIndex = 0; usIndex < pCD->usLostChains &&
                     usIndex < MAX_LOST_CHAINS; usIndex++)
      {
      if (ClusterInChain(pCD, ulCluster, pCD->rgulLost[usIndex]))
         {
         ULONG ulNext = ulCluster;
         while (ulNext != pCD->rgulLost[usIndex])
            {
            MarkCluster(pCD, ulNext, "Lost cluster");
            pCD->rgulSize[usIndex]++;
            pCD->ulLostClusters++;
            ulNext = GetNextCluster(pCD, ulNext);
            }
         pCD->rgulLost[usIndex] = ulCluster;
         return TRUE;
         }
      }

   ulSize = GetClusterCount(pCD, ulCluster, "Lost data");
   if (pCD->usLostChains < MAX_LOST_CHAINS)
      {
      pCD->rgulLost[pCD->usLostChains] = ulCluster;
      pCD->rgulSize[pCD->usLostChains] = ulSize;
      }
   pCD->ulLostClusters += ulSize;
   pCD->usLostChains++;
   return TRUE;

}

BOOL ClusterInChain(PCDINFO pCD, ULONG ulStart, ULONG ulCluster)
{
   while (ulStart && ulStart != FAT_EOF)
      {
      if (ulStart == ulCluster)
         return TRUE;
      ulStart = GetNextCluster(pCD, ulStart);
      }
   return FALSE;
}

BOOL LostToFile(PCDINFO pCD, ULONG ulCluster, ULONG ulSize)
{
BYTE   szRecovered[CCHMAXPATH];
USHORT rc;

   memset(szRecovered, 0, sizeof szRecovered);
   rc = DosDevIOCtl2(szRecovered, sizeof szRecovered, 
      (PVOID)&ulCluster, 4,
      FAT32_RECOVERCHAIN, IOCTL_FAT32, pCD->hDisk);
   if (rc)
      {
      pCD->ulErrorCount++;
      printf("CHKDSK was unable to recover a lost chain. SYS%4.4u\n", rc);
      return FALSE;
      }
   pCD->ulRecoveredClusters += ulSize;
   pCD->ulRecoveredFiles++;

   iShowMessage(pCD, 574, 1, TYPE_STRING, szRecovered);
   return TRUE;
}


