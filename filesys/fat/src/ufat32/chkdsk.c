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
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#include <os2.h>
#include "portable.h"
#include "fat32def.h"

PRIVATE ULONG ChkDskMain(PCDINFO pCD);
PRIVATE ULONG MarkVolume(PCDINFO pCD, BOOL fClean);
PRIVATE ULONG ReadSector(PCDINFO pCD, ULONG ulSector, USHORT nSectors, PBYTE pbSector);
PRIVATE ULONG CheckFats(PCDINFO pCD);
PRIVATE ULONG CheckFiles(PCDINFO pCD);
PRIVATE ULONG CheckFreeSpace(PCDINFO pCD);
PRIVATE ULONG CheckDir(PCDINFO pCD, ULONG ulDirCluster, PSZ pszPath, ULONG ulParentDirCluster);
PRIVATE BOOL   ReadFATSector(PCDINFO pCD, ULONG ulSector);
PRIVATE ULONG  GetNextCluster(PCDINFO pCD, ULONG ulCluster, BOOL fAllowBad);
PRIVATE ULONG SetNextCluster(PCDINFO pCD, ULONG ulCluster, ULONG ulNextCluster);
PRIVATE ULONG ReadCluster(PCDINFO pDrive, ULONG ulCluster, PBYTE pbCluster);
PRIVATE ULONG GetClusterCount(PCDINFO pCD, ULONG ulCluster, PSZ pszFile);
PRIVATE BOOL   MarkCluster(PCDINFO pCD, ULONG ulCluster, PSZ pszFile);
PRIVATE PSZ    MakeName(PDIRENTRY pDir, PSZ pszName, USHORT usMax);
PRIVATE ULONG fGetVolLabel(PCDINFO pCD, PSZ pszVolLabel);
PRIVATE BOOL   fGetLongName(PDIRENTRY pDir, PSZ pszName, USHORT wMax);
PRIVATE BOOL   ClusterInUse(PCDINFO pCD, ULONG ulCluster);
PRIVATE BOOL RecoverChain(PCDINFO pCD, ULONG ulCluster);
PRIVATE BOOL LostToFile(PCDINFO pCD, ULONG ulCluster, ULONG ulSize);
PRIVATE BOOL ClusterInChain(PCDINFO pCD, ULONG ulStart, ULONG ulCluster);
PRIVATE BOOL OutputToFile(VOID);

INT cdecl iShowMessage(PCDINFO pCD, USHORT usNr, USHORT usNumFields, ...);
PSZ       GetOS2Error(USHORT rc);

static F32PARMS  f32Parms;
static BOOL fToFile;

#if 1  /* by OAX */
static UCHAR rgFirstInfo[ 256 ] = { 0, };
static UCHAR rgLCase[ 256 ] = { 0, };

VOID TranslateInitDBCSEnv( VOID )
{
   ULONG ulDataSize;
   ULONG ulParamSize;

   DosFSCtl( rgFirstInfo, sizeof( rgFirstInfo ), &ulDataSize,
      NULL, 0, &ulParamSize,
      FAT32_GETFIRSTINFO, "FAT32", -1, FSCTL_FSDNAME );
}

BOOL IsDBCSLead( UCHAR uch)
{
    return ( rgFirstInfo[ uch ] == 2 );
}

VOID CaseConversionInit( VOID )
{
   ULONG ulDataSize;
   ULONG ulParamSize;

   DosFSCtl( rgLCase, sizeof( rgLCase ), &ulDataSize,
      NULL, 0, &ulParamSize,
      FAT32_GETCASECONVERSION, "FAT32", -1, FSCTL_FSDNAME );
}

/* Get the last-character. (sbcs/dbcs) */
int lastchar(const char *string)
{
    UCHAR *s;
    unsigned int c = 0;
    int i, len = strlen(string);
    s = (UCHAR *)string;
    for(i = 0; i < len; i++)
    {
        c = *(s + i);
        if(IsDBCSLead(( UCHAR )c))
        {
            c = (c << 8) + ( unsigned int )*(s + i + 1);
            i++;
        }
    }
    return c;
}

#endif /* by OAX */

int chkdsk(int iArgc, char *rgArgv[], char *rgEnv[])
{
INT iArg;
HFILE hFile;
ULONG rc = 0;
PCDINFO pCD;
ULONG  ulAction;
BYTE   bSector[512];
ULONG  ulDeadFace = 0xDEADFACE;
ULONG  ulParmSize;
ULONG  ulDataSize;
ULONG  cbDataLen;

   DosError(1); /* Enable hard errors */

   TranslateInitDBCSEnv();

   CaseConversionInit();

   fToFile = OutputToFile();

   pCD = (PCDINFO)malloc(sizeof(CDINFO));
   if (!pCD)
      return ERROR_NOT_ENOUGH_MEMORY;
   memset(pCD, 0, sizeof (CDINFO));

   printf("(UFAT32.DLL version %s compiled on " __DATE__ ")\n", FAT32_VERSION);

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

   ulDataSize = sizeof(f32Parms);
   rc = DosFSCtl(
      (PVOID)&f32Parms, ulDataSize, &ulDataSize,
      NULL, 0, &ulParmSize,
      FAT32_GETPARMS, "FAT32", -1, FSCTL_FSDNAME);
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
      ULONG ulDisk;
      ULONG  ulDrives;
      rc = DosQueryCurrentDisk(&ulDisk, &ulDrives);
      pCD->szDrive[0] = (BYTE)(ulDisk + '@');
      pCD->szDrive[1] = ':';
      }
   //printf("000\n");
   //printf("DosOpen pCD->szDrive=%s\n", pCD->szDrive);
   rc = DosOpen(pCD->szDrive,
      &hFile,
      &ulAction,                         /* action taken */
      0L,                                /* new size     */
      0,                                 /* attributes   */
      OPEN_ACTION_OPEN_IF_EXISTS,        /* open flags   */
      OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE | OPEN_FLAGS_DASD |
      OPEN_FLAGS_WRITE_THROUGH,         /* OPEN_FLAGS_NO_CACHE , */
      0L);
   //printf("rc=%lu\n", rc);
   //printf("001\n");
   if (rc)
      {
      if (rc == ERROR_DRIVE_LOCKED)
         iShowMessage(pCD, rc, 0);
      else
         printf("%s\n", GetOS2Error(rc));
      DosExit(EXIT_PROCESS, 1);
      }
   ulParmSize = sizeof(ulDeadFace);
   //printf("002\n");
   rc = DosFSCtl(NULL, 0, 0,
                 (PBYTE)&ulDeadFace, ulParmSize, &ulParmSize,
                 FAT32_SECTORIO,
                 NULL,
                 hFile,
                 FSCTL_HANDLE);
   if (rc)
      {
      printf("DosFSCtl (SectorIO) failed:\n%s\n", GetOS2Error(rc));
      exit(1);
      }
   //printf("003\n");
   rc = DosDevIOCtl(hFile, IOCTL_FAT32, FAT32_GETVOLCLEAN,
                    NULL, 0, NULL,
                    (PVOID)&pCD->fCleanOnBoot, sizeof(pCD->fCleanOnBoot), &cbDataLen);
   //rc = DosDevIOCtl2((PVOID)&pCD->fCleanOnBoot, sizeof(pCD->fCleanOnBoot),
   //   NULL, 0,
   //   FAT32_GETVOLCLEAN, IOCTL_FAT32, hFile);
   //printf("DosDevIOCtl, rc=%lu\n", rc);
   if (pCD->fAutoRecover && pCD->fCleanOnBoot)
      pCD->fAutoRecover = FALSE;

   if (pCD->fFix)
      {
      //printf("004\n");
      rc = DosDevIOCtl(hFile, IOCTL_DISK, DSK_LOCKDRIVE,
                       NULL, 0, NULL,
                       NULL, 0, NULL);
      //rc = DosDevIOCtl(NULL, NULL, DSK_LOCKDRIVE, IOCTL_DISK, hFile);
      if (rc)
         {
         if (rc == ERROR_DRIVE_LOCKED)
            iShowMessage(pCD, rc, 0);
         else
            printf("%s\n", GetOS2Error(rc));
         DosExit(EXIT_PROCESS, 1);
         }
      }
   //printf("005\n");

   rc = DosQueryFSInfo(pCD->szDrive[0] - '@', FSIL_ALLOC,
      (PBYTE)&pCD->DiskInfo, sizeof (DISKINFO));
   if (rc)
      {
      fprintf(stderr, "DosQFSInfo failed, %s\n", GetOS2Error(rc));
      DosExit(EXIT_PROCESS, 1);
      }
   pCD->hDisk = hFile;

   //printf("006\n");
   rc = ReadSector(pCD, 0, 1, bSector);
   if (rc)
      {
      printf("Error: Cannot read boot sector: %s\n", GetOS2Error(rc));
      return rc;
      }
   memcpy(&pCD->BootSect, bSector, sizeof (BOOTSECT));

   //printf("007\n");
   rc = ReadSector(pCD, pCD->BootSect.bpb.FSinfoSec, 1, bSector);
   if (rc)
      {
      printf("Error: Cannot read FSInfo sector\n%s\n", GetOS2Error(rc));
      return rc;
      }

   //printf("008\n");
   rc = ChkDskMain(pCD);
   //printf("009\n");

   if (pCD->fFix)
      {
      //printf("0090\n");
      rc = DosDevIOCtl(hFile, IOCTL_DISK, DSK_UNLOCKDRIVE,
                       NULL, 0, NULL,
                       NULL, 0, NULL);
      //rc = DosDevIOCtl(NULL, NULL, DSK_UNLOCKDRIVE, IOCTL_DISK, hFile);
      if (rc)
         {
         printf("The drive cannot be unlocked. SYS%4.4u\n", rc);
         DosExit(EXIT_PROCESS, 1);
         }
      }
   DosClose(hFile);
   free(pCD);

   //printf("010\n");

   DosExit(EXIT_PROCESS, rc);
   return rc;

   rgEnv = rgEnv;
}

BOOL OutputToFile(VOID)
{
ULONG rc;
ULONG ulType;
ULONG ulAttr;

   rc = DosQueryHType(fileno(stdout), &ulType, &ulAttr);
   switch (ulType & 0x000F)
      {
      case 0:
         return TRUE;
      case 1:
      case 2:
         return FALSE;
/*
      default:
         return FALSE;
*/
      }
   return FALSE;
}

ULONG ReadCluster(PCDINFO pCD, ULONG ulCluster, PBYTE pbCluster)
{
   ULONG dummy1 = 0, dummy2 = 0;
   ULONG ulDataSize = pCD->usClusterSize;
   ULONG rc;

   //printf("pCD->usClusterSize=%lu\n", ulDataSize);   
   //printf("ulCluster=%lu\n", ulCluster);   
//   return DosDevIOCtl2(
//     (PVOID)pbCluster, pCD->usClusterSize,
//      (PVOID)&ulCluster, sizeof(ulCluster),
//      FAT32_READCLUSTER, IOCTL_FAT32, pCD->hDisk);
   //printf("240\n");
   rc = DosDevIOCtl(pCD->hDisk, IOCTL_FAT32, FAT32_READCLUSTER,
                      (PVOID)&ulCluster, sizeof(ulCluster), &dummy1,
                      (PVOID)pbCluster, ulDataSize, &dummy2);
   //printf("241\n");
   return rc;
}

ULONG ReadSector(PCDINFO pCD, ULONG ulSector, USHORT nSectors, PBYTE pbSector)
{
READSECTORDATA rsd;
ULONG ulDataSize;
ULONG dummy1 = 0, dummy2 = 0;
ULONG rc;

   rsd.ulSector = ulSector;
   rsd.nSectors = nSectors;

   ulDataSize = nSectors * SECTOR_SIZE;

//   return DosDevIOCtl2(
//      (PVOID)pbSector, usDataSize,
//      (PVOID)&rsd, sizeof(rsd),
//      FAT32_READSECTOR, IOCTL_FAT32, pCD->hDisk);
   return DosDevIOCtl(pCD->hDisk, IOCTL_FAT32, FAT32_READSECTOR,
                      (PVOID)&rsd, sizeof(rsd), &dummy1,
                      (PVOID)pbSector, ulDataSize, &dummy2);
}

ULONG ChkDskMain(PCDINFO pCD)
{
ULONG  rc;
ULONG  ulBytes;
USHORT usBlocks;
BYTE   szString[12];
PSZ    p;
ULONG  dummy = 0;

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

   //pCD->pFatBits = halloc(usBlocks,4096);
   pCD->pFatBits = calloc(usBlocks,4096);
   if (!pCD->pFatBits)
      {
      printf("Not enough memory for FATBITS\n");
      return ERROR_NOT_ENOUGH_MEMORY;
      }
   //printf("100\n");

   memset(szString, 0, sizeof(szString));
#if 0
   strncpy(szString, pCD->BootSect.VolumeLabel, 11);
#else
   //printf("101\n");
   fGetVolLabel( pCD, szString );
   //printf("102\n");
#endif
   p = szString + strlen(szString);
   while (p > szString && *(p-1) == ' ')
      p--;
   *p = 0;
   if( p > szString )
      iShowMessage(pCD, 1375, 1, TYPE_STRING, szString);

   sprintf(szString, "%4.4X-%4.4X",
      HIUSHORT(pCD->BootSect.ulVolSerial), LOUSHORT(pCD->BootSect.ulVolSerial));
   iShowMessage(pCD, 1243, 1, TYPE_STRING, szString);
   //printf("103\n");
   if (pCD->BootSect.bpb.MediaDescriptor != 0xF8)
      {
      printf("The media descriptor is incorrect\n");
      pCD->ulErrorCount++;
      }
   //printf("104\n");

   rc = CheckFats(pCD);
   //printf("105\n");
   if (rc)
      {
      printf("The copies of the FATs do not match.\n");
      printf("Please run SCANDISK to correct this problem.\n");
      return rc;
      }
   //printf("106\n");
   rc = CheckFiles(pCD);
   //printf("107\n");
   rc = CheckFreeSpace(pCD);
   //printf("108\n");


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
         //printf("109\n");
         rc = DosDevIOCtl(pCD->hDisk, IOCTL_FAT32, FAT32_GETFREESPACE,
                          NULL, 0, NULL,
                          &ulFreeBlocks, sizeof(ulFreeBlocks), &dummy);
         //rc = DosDevIOCtl2(&ulFreeBlocks, sizeof(ulFreeBlocks),
         //   NULL, 0,
         //   FAT32_GETFREESPACE, IOCTL_FAT32, pCD->hDisk);
         //printf("110\n");
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

   //printf("111\n");
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

   //printf("112\n");
   if (pCD->ulRecoveredClusters)
      iShowMessage(pCD, 1365, 2,
         TYPE_DOUBLE, (DOUBLE)pCD->ulRecoveredClusters * pCD->usClusterSize,
         TYPE_LONG, pCD->ulRecoveredFiles);

   //printf("113\n");
   if (pCD->ulLostClusters)
      iShowMessage(pCD, 1359, 1,
         TYPE_DOUBLE, (DOUBLE)pCD->ulLostClusters * pCD->usClusterSize);


   //printf("114\n");
   iShowMessage(pCD, 1368, 2,
      TYPE_DOUBLE, (DOUBLE)pCD->ulFreeClusters * pCD->usClusterSize,
      TYPE_LONG, 0L);

   printf("\n");


   //printf("115\n");
   iShowMessage(pCD, 1304, 1,
      TYPE_LONG, (ULONG)pCD->usClusterSize);

   //printf("116\n");
   iShowMessage(pCD, 1305, 1,
      TYPE_LONG, pCD->ulTotalClusters);

   //printf("117\n");
   iShowMessage(pCD, 1306, 1,
      TYPE_LONG, pCD->ulFreeClusters);

   //printf("118\n");
   if (pCD->ulTotalChains > 0)
      printf("\n%u%% of the files and directories are fragmented.\n",
         (USHORT)(pCD->ulFragmentedChains * 100 / pCD->ulTotalChains));

   //printf("119\n");
   if (pCD->ulErrorCount)
      {
      printf("\n");
      if (!pCD->fFix)
         iShowMessage(pCD, 1339, 0);
      else
         printf("Still errors found on disk. Please run Windows 95 ScanDisk!\n");
      }
   else if (pCD->fFix)
      //DosDevIOCtl(NULL, NULL, FAT32_MARKVOLCLEAN, IOCTL_FAT32, pCD->hDisk);
      DosDevIOCtl(pCD->hDisk, IOCTL_FAT32, FAT32_MARKVOLCLEAN, 
                  NULL, 0, NULL, NULL, 0, NULL);

   //printf("120\n");
   return 0;
}


ULONG MarkVolume(PCDINFO pCD, BOOL fClean)
{
ULONG rc;
ULONG dummy = 0;

   rc = DosDevIOCtl(pCD->hDisk,
          IOCTL_FAT32,
          FAT32_FORCEVOLCLEAN,
          (PVOID)&fClean, sizeof(fClean), &dummy,
          NULL, 0, NULL);
//   rc = DosDevIOCtl2( NULL, 0,
//      (PVOID)&fClean, sizeof(fClean),
//      FAT32_FORCEVOLCLEAN,
//      IOCTL_FAT32,
//      pCD->hDisk);

   return rc;
}

ULONG CheckFats(PCDINFO pCD)
{
PBYTE pSector;
USHORT nFat;
ULONG ulSector;
ULONG  rc;
USHORT usPerc = 0xFFFF;
BOOL   fDiff;
ULONG  ulCluster;
USHORT usIndex;
PULONG pulCluster;
USHORT fRetco;

/*
//   printf("Each fat contains %lu sectors\n",
//      pCD->BootSect.bpb.BigSectorsPerFat);
*/
   printf("CHKDSK is checking fats :    ");

   if (pCD->BootSect.bpb.ExtFlags & 0x0080)
      {
      printf("There is only one active FAT.\n");
      return 0;
      }


   //pSector = halloc(pCD->BootSect.bpb.NumberOfFATs, BLOCK_SIZE);
   pSector = calloc(pCD->BootSect.bpb.NumberOfFATs, BLOCK_SIZE);
   if (!pSector)
      return ERROR_NOT_ENOUGH_MEMORY;


   fDiff = FALSE;
   ulCluster = 0L;
   fRetco = 0;
   for (ulSector = 0; ulSector < pCD->BootSect.bpb.BigSectorsPerFat; ulSector+=32)
      {
      USHORT usNew  = (USHORT)(ulSector * 100 / pCD->BootSect.bpb.BigSectorsPerFat);
      USHORT nSectors;


      if (!pCD->fPM && !fToFile && usNew != usPerc)
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

   if (!pCD->fPM && !fToFile)
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


ULONG CheckFiles(PCDINFO pCD)
{

   printf("CHKDSK is checking files and directories...\n");
   return CheckDir(pCD, pCD->BootSect.bpb.RootDirStrtClus, pCD->szDrive, 0L);
}

ULONG CheckFreeSpace(PCDINFO pCD)
{
ULONG ulCluster;
USHORT usPerc = 100;
BOOL fMsg = FALSE;
ULONG dummy = 0;

   iShowMessage(pCD, 564, 0);

   pCD->ulFreeClusters = 0;
   for (ulCluster = 0; ulCluster < pCD->ulTotalClusters; ulCluster++)
      {
      USHORT usNew  = (USHORT)(ulCluster * 100 / pCD->ulTotalClusters);
      ULONG ulNext = GetNextCluster(pCD, ulCluster + 2, TRUE);

      if (!pCD->fPM && !fToFile && usNew != usPerc)
         {
         iShowMessage(pCD, 563, 1, TYPE_PERC, usNew);
         printf("\r");
         usPerc = usNew;
         }
      /* bad cluster ? */
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

   if (!pCD->fPM && !fToFile)
      iShowMessage(pCD, 563, 1, TYPE_PERC, 100);
   printf("\n");

   if (pCD->usLostChains)
      {
      BYTE bChar;
      USHORT usIndex;
      ULONG rc;

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
               rc = DosDevIOCtl(pCD->hDisk, IOCTL_FAT32, FAT32_DELETECHAIN, 
                            (PVOID)&pCD->rgulLost[usIndex], 4, &dummy,
                            NULL, 0, NULL);
               //rc = DosDevIOCtl2(NULL, 0,
               //   (PVOID)&pCD->rgulLost[usIndex], 4,
               //   FAT32_DELETECHAIN, IOCTL_FAT32, pCD->hDisk);
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


ULONG CheckDir(PCDINFO pCD, ULONG ulDirCluster, PSZ pszPath, ULONG ulParentDirCluster)
{
static BYTE szLongName[512] = "";
static BYTE szShortName[13] = "";
static MARKFILEEASBUF Mark;

int iIndex;
//DIRENTRY _huge * pDir;
//DIRENTRY _huge * pEnd;
DIRENTRY * pDir;
DIRENTRY * pEnd;
PBYTE pbCluster;
PBYTE pbPath;
ULONG ulCluster;
ULONG ulClusters;
ULONG ulBytesNeeded;
//BYTE _huge * p;
BYTE * p;
BYTE bCheckSum, bCheck;
ULONG ulClustersNeeded;
ULONG ulClustersUsed;
ULONG ulEntries;
PBYTE pEA;
ULONG rc;
ULONG dummy = 0;

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
   //pbCluster = halloc(ulClusters, pCD->BootSect.bpb.SectorsPerCluster * pCD->BootSect.bpb.BytesPerSector);
   pbCluster = calloc(ulClusters, pCD->BootSect.bpb.SectorsPerCluster * pCD->BootSect.bpb.BytesPerSector);
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
      ulCluster = GetNextCluster(pCD, ulCluster, FALSE);
      if (!ulCluster)
         ulCluster = FAT_EOF;
      p += pCD->BootSect.bpb.SectorsPerCluster * pCD->BootSect.bpb.BytesPerSector;
      }

   memset(szLongName, 0, sizeof(szLongName));
   //pDir = (DIRENTRY _huge *)pbCluster;
   //pEnd = (DIRENTRY _huge *)(p - sizeof (DIRENTRY));
   pDir = (DIRENTRY *)pbCluster;
   pEnd = (DIRENTRY *)(p - sizeof (DIRENTRY));
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
               memset(szLongName, 0, sizeof(szLongName));
               }
            bCheck = pDir->bReserved;
            fGetLongName(pDir, szLongName, sizeof(szLongName));
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
                  szLongName, pszPath, MakeName(pDir, szShortName, sizeof(szShortName)));
               memset(szLongName, 0, sizeof(szLongName));
               }

            /* support for the FAT32 variation of WinNT family */
            if( !*szLongName && HAS_WINNT_EXT( pDir->fEAS ))
            {
                PBYTE pDot;

                MakeName( pDir, szLongName, sizeof( szLongName ));
                pDot = strchr( szLongName, '.' );

                if( HAS_WINNT_EXT_NAME( pDir->fEAS )) /* name part is lower case */
                {
                    if( pDot )
                        *pDot = 0;

                        strlwr( szLongName );

                        if( pDot )
                        *pDot = '.';
                }

                if( pDot && HAS_WINNT_EXT_EXT( pDir->fEAS )) /* ext part is lower case */
                    strlwr( pDot + 1 );
            }

            if (pCD->fDetailed == 2)
               {
               printf("%-13.13s", MakeName(pDir, szShortName, sizeof(szShortName)));
               if (pDir->bAttr & FILE_DIRECTORY)
                  printf("<DIR>      ");
               else
                  printf("%10lu ", pDir->ulFileSize);

/*               printf("%8.8lX ", MAKEP(pDir->wClusterHigh, pDir->wCluster));*/

               printf("%s ", szLongName);
               }

            /*
               Construct full path
            */
            strcpy(pbPath, pszPath);
            if (lastchar(pbPath) != '\\')
               strcat(pbPath, "\\");
            if (strlen(szLongName))
               strcat(pbPath, szLongName);
            else
               {
               MakeName(pDir, szLongName, sizeof(szLongName));
               strcat(pbPath, szLongName);
               }

            if( f32Parms.fEAS && HAS_OLD_EAS( pDir->fEAS ))
            {
                printf("%s has old EA mark byte(0x%0X).\n", pbPath, pDir->fEAS );
                if (pCD->fFix)
                {
                     strcpy(Mark.szFileName, pbPath);
                     Mark.fEAS = ( BYTE )( pDir->fEAS == FILE_HAS_OLD_EAS ? FILE_HAS_EAS : FILE_HAS_CRITICAL_EAS );
                     rc = DosDevIOCtl(pCD->hDisk, IOCTL_FAT32, FAT32_SETEAS,
                                     (PVOID)&Mark, sizeof(Mark), &dummy,
                                     NULL, 0, NULL);
                     //rc = DosDevIOCtl2(NULL, 0,
                     //                  (PVOID)&Mark, sizeof(Mark),
                     //                  FAT32_SETEAS, IOCTL_FAT32, pCD->hDisk);
                     if (!rc)
                        printf("This has been corrected.\n");
                     else
                        printf("SYS%4.4u: Unable to correct problem.\n", rc);
                }
            }

#if 1
            if( f32Parms.fEAS && pDir->fEAS && !HAS_WINNT_EXT( pDir->fEAS ) && !HAS_EAS( pDir->fEAS ))
            {
                printf("%s has unknown EA mark byte(0x%0X).\n", pbPath, pDir->fEAS );
            }
#endif

#if 0
            if( f32Parms.fEAS && HAS_EAS( pDir->fEAS ))
            {
                printf("%s has EA byte(0x%0X).\n", pbPath, pDir->fEAS );
            }
#endif

            if (f32Parms.fEAS && HAS_EAS( pDir->fEAS ))
               {
               FILESTATUS fStat;

               strcpy(Mark.szFileName, pbPath);
               strcat(Mark.szFileName, EA_EXTENTION);
               rc = DosQueryPathInfo(Mark.szFileName, FIL_STANDARD, (PBYTE)&fStat, sizeof(fStat));
               //rc = DosQPathInfo(Mark.szFileName, FIL_STANDARD, (PBYTE)&fStat, sizeof(fStat), 0L);
               if (rc)
                  {
                  printf("%s is marked having EAs, but the EA file (%s) is not found. (SYS%4.4u)\n", pbPath, Mark.szFileName, rc);
                  if (pCD->fFix)
                     {
                     strcpy(Mark.szFileName, pbPath);
                     Mark.fEAS = FILE_HAS_NO_EAS;
                     rc = DosDevIOCtl(pCD->hDisk, IOCTL_FAT32, FAT32_SETEAS, 
                                   (PVOID)&Mark, sizeof(Mark), &dummy,
                                   NULL, 0, NULL);
                     //rc = DosDevIOCtl2(NULL, 0,
                     //   (PVOID)&Mark, sizeof(Mark),
                     //   FAT32_SETEAS, IOCTL_FAT32, pCD->hDisk);
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
                     rc = DosDevIOCtl(pCD->hDisk, IOCTL_FAT32, FAT32_SETEAS, 
                            (PVOID)&Mark, sizeof(Mark), &dummy,
                            NULL, 0, NULL);
                     //rc = DosDevIOCtl2(NULL, 0,
                     //   (PVOID)&Mark, sizeof(Mark),
                     //   FAT32_SETEAS, IOCTL_FAT32, pCD->hDisk);
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
               if (f32Parms.fEAS && pEA && pDir->ulFileSize)
                  {
                  ULONG rc;
                  FILESTATUS3 fs;

                  pCD->ulEAClusters += ulClustersUsed;

                  memset(&Mark, 0, sizeof(Mark));
                  memcpy(Mark.szFileName, pbPath, pEA - pbPath);

                  rc = DosDevIOCtl(pCD->hDisk, IOCTL_FAT32, FAT32_QUERYEAS, 
                                   (PVOID)&Mark, sizeof(Mark), &dummy,
                                   NULL, 0, NULL);
                  //rc = DosDevIOCtl2(NULL, 0,
                  //   (PVOID)&Mark, sizeof(Mark),
                  //   FAT32_QUERYEAS, IOCTL_FAT32, pCD->hDisk);
                  if (rc == 2 || rc == 3)
                     {
                     printf("A lost Extended attribute was found (for %s)\n",
                        Mark.szFileName);
                     if (pCD->fFix)
                        {
                        strcat(Mark.szFileName, ".EA");
                        rc = MarkVolume(pCD, TRUE);
                        if (!rc)
                        {
                           DosQueryPathInfo(pbPath, FIL_STANDARD, &fs ,sizeof(fs));
			   fs.attrFile = FILE_NORMAL;
                           rc = DosSetPathInfo(pbPath, FIL_STANDARD, &fs, sizeof(fs), 0);
                           //rc = DosSetFileMode(pbPath, FILE_NORMAL, 0L);
                        }
                        if (!rc)
                           //rc = DosMove(pbPath, Mark.szFileName, 0L);
                           rc = DosMove(pbPath, Mark.szFileName);
                        if (!rc)
                           printf("This attribute has been converted to a file \n(%s).\n", Mark.szFileName);
                        else
                           {
                           printf("SYS%4.4u: Cannot convert %s\n",
                              rc, pbPath);
                           pCD->ulErrorCount++;
                           }
                        MarkVolume(pCD, pCD->fCleanOnBoot);
                        }
                     }
                  else if (rc)
                     {
                     printf("SYS%4.4u occured while retrieving EA flag for %s.\n", rc, Mark.szFileName);
                     }
                  else
                     {
                     if ( !HAS_EAS( Mark.fEAS ))
                        {
                        printf("EAs detected for %s, but it is not marked having EAs.\n", Mark.szFileName);
                        if (pCD->fFix)
                           {
                           Mark.fEAS = FILE_HAS_EAS;
                           rc = DosDevIOCtl(pCD->hDisk, IOCTL_FAT32, FAT32_SETEAS, 
                                        (PVOID)&Mark, sizeof(Mark), &dummy,
                                        NULL, 0, NULL);
                           //rc = DosDevIOCtl2(NULL, 0,
                           //   (PVOID)&Mark, sizeof(Mark),
                           //   FAT32_SETEAS, IOCTL_FAT32, pCD->hDisk);
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
#if 0
                        pszPath, MakeName(pDir, szShortName, sizeof(szShortName)));
#else
                        pszPath, szLongName);
#endif
                     pCD->ulErrorCount++;
                     }
                  else
                     {
                     FILESIZEDATA fs;
                     ULONG rc;

                     memset(&fs, 0, sizeof(fs));
                     strcpy(fs.szFileName, pszPath);
                     if (lastchar(fs.szFileName) != '\\')
                        strcat(fs.szFileName, "\\");
                     strcat(fs.szFileName, MakeName(pDir, szShortName, sizeof(szShortName)));
                     fs.ulFileSize = ulClustersUsed * pCD->usClusterSize;
                     rc = DosDevIOCtl(pCD->hDisk, IOCTL_FAT32, FAT32_SETFILESIZE, 
                               (PVOID)&fs, sizeof(fs), &dummy,
                               NULL, 0, NULL);
                     //rc = DosDevIOCtl2(NULL, 0, 
                     //   (PVOID)&fs, sizeof(fs),
                     //   FAT32_SETFILESIZE, IOCTL_FAT32, pCD->hDisk);
                     strcpy( strrchr( fs.szFileName, '\\' ) + 1, szLongName );
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


           memset(szLongName, 0, sizeof(szLongName));
            }
         ulEntries++;
         }
      pDir++;

      }
   if (pCD->fDetailed == 2)
      printf("%ld files\n", ulEntries);

   bCheck = 0;
   pDir = (PDIRENTRY)pbCluster;
   memset(szLongName, 0, sizeof(szLongName));
   while (pDir <= pEnd)
      {
      if (pDir->bFileName[0] && pDir->bFileName[0] != 0xE5)
         {
         if (pDir->bAttr == FILE_LONGNAME)
            {
            if (strlen(szLongName) && bCheck != pDir->bReserved)
               memset(szLongName, 0, sizeof(szLongName));
            bCheck = pDir->bReserved;
            fGetLongName(pDir, szLongName, sizeof(szLongName));
            }
         else
            {
            if (pDir->bAttr & FILE_DIRECTORY &&
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
                  {
                  printf("Non matching longname %s for %-11.11s\n",
                     szLongName, pDir->bFileName);
                  memset(szLongName, 0, sizeof(szLongName));
                  }

               /* support for the FAT32 variation of WinNT family */
               if( !*szLongName && HAS_WINNT_EXT( pDir->fEAS ))
               {
                    PBYTE pDot;

                    MakeName( pDir, szLongName, sizeof( szLongName ));
                    pDot = strchr( szLongName, '.' );

                    if( HAS_WINNT_EXT_NAME( pDir->fEAS )) /* name part is lower case */
                    {
                        if( pDot )
                            *pDot = 0;

                        strlwr( szLongName );

                        if( pDot )
                            *pDot = '.';
                    }

                    if( pDot && HAS_WINNT_EXT_EXT( pDir->fEAS )) /* ext part is lower case */
                        strlwr( pDot + 1 );
               }

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
                  if (lastchar(pbPath) != '\\')
                     strcat(pbPath, "\\");
                  if (strlen(szLongName))
                     strcat(pbPath, szLongName);
                  else
                     {
                     MakeName(pDir, szLongName, sizeof(szLongName));
                     strcat(pbPath, szLongName);
                     }

                  memset(szLongName, 0, sizeof(szLongName));
                  CheckDir(pCD,
                     (ULONG)pDir->wClusterHigh * 0x10000 + pDir->wCluster,
                     pbPath, (ulDirCluster == pCD->BootSect.bpb.RootDirStrtClus ? 0L : ulDirCluster));
                  }
               }
            memset(szLongName, 0, sizeof(szLongName));
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
      ulNextCluster = GetNextCluster(pCD, ulCluster, FALSE);
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

   memset(szExtention, 0, sizeof(szExtention));
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

ULONG fGetVolLabel( PCDINFO pCD, PSZ pszVolLabel )
{
PDIRENTRY pDirStart, pDir, pDirEnd;
ULONG ulCluster;
DIRENTRY DirEntry;
BOOL     fFound;

   pDir = NULL;

   //printf("200\n");

   pDirStart = malloc(pCD->usClusterSize);
   if (!pDirStart)
      return ERROR_NOT_ENOUGH_MEMORY;
   //printf("210\n");

   fFound = FALSE;
   ulCluster = pCD->BootSect.bpb.RootDirStrtClus;
   while (!fFound && ulCluster != FAT_EOF)
      {
      //printf("211\n");
      ReadCluster(pCD, ulCluster, (PBYTE)pDirStart);
      //printf("212\n");
      pDir = pDirStart;
      pDirEnd = (PDIRENTRY)((PBYTE)pDirStart + pCD->usClusterSize);
      //printf("213\n");
      while (pDir < pDirEnd)
         {
         if ((pDir->bAttr & 0x0F) == FILE_VOLID && pDir->bFileName[0] != DELETED_ENTRY)
            {
            fFound = TRUE;
            memcpy(&DirEntry, pDir, sizeof (DIRENTRY));
            //printf("xx\n");
            break;
            }
         pDir++;
         }
      if (!fFound)
         {
         //printf("220\n");
         ulCluster = GetNextCluster(pCD, ulCluster, FALSE);
         if (!ulCluster)
            ulCluster = FAT_EOF;
         }
      }
   free(pDirStart);
   if (!fFound)
      memset(pszVolLabel, 0, 11);
   else
      memcpy(pszVolLabel, DirEntry.bFileName, 11);
   //printf("230\n");

   return 0;
}

BOOL fGetLongName(PDIRENTRY pDir, PSZ pszName, USHORT wMax)
{
static BYTE szLongName[30] = "";
static USHORT uniName[15] = {0};
USHORT wNameSize;
USHORT usIndex;
ULONG ulDataSize;
ULONG ulParmSize;
PLNENTRY pLN = (PLNENTRY)pDir;

   memset(szLongName, 0, sizeof(szLongName));
   memset(uniName, 0, sizeof(uniName));

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

   ulDataSize = sizeof(szLongName);
   ulParmSize = sizeof(uniName);
   //DosFSCtl(szLongName, usDataSize, &usDataSize,
   //      (PVOID)uniName, usParmSize, &usParmSize,
   //      FAT32_WIN2OS,
   //      "FAT32",
   //      -1,
   //      FSCTL_FSDNAME,
   //      0);
   DosFSCtl(szLongName,  ulDataSize, &ulDataSize,
         (PVOID)uniName, ulParmSize, &ulParmSize,
         FAT32_WIN2OS,
         "FAT32",
         -1,
         FSCTL_FSDNAME);

   wNameSize = strlen( szLongName );

   if (strlen(pszName) + wNameSize > wMax)
      return FALSE;

   memmove(pszName + wNameSize, pszName, strlen(pszName) + 1);
   memcpy(pszName, szLongName, wNameSize);
   return TRUE;
}



ULONG GetNextCluster(PCDINFO pCD, ULONG ulCluster, BOOL fAllowBad)
{
PULONG pulCluster;
ULONG  ulSector;
ULONG  ulRet;

   //printf("300\n");
   ulSector = ulCluster / 128;
   if (!ReadFATSector(pCD, ulSector))
      return FAT_EOF;
   //printf("301\n");

   pulCluster = (PULONG)pCD->pbFATSector + ulCluster % 128;

   ulRet = *pulCluster & FAT_EOF;
   if (ulRet >= FAT_EOF2 && ulRet <= FAT_EOF)
      return FAT_EOF;

   //printf("302\n");

   if (ulRet == FAT_BAD_CLUSTER && fAllowBad)
      return ulRet;

   //printf("303\n");

   if (ulRet >= pCD->ulTotalClusters  + 2)
      {
      printf("Error: Next cluster for %lu = %8.8lX\n",
         ulCluster, *pulCluster);
      return FAT_EOF;
      }
   //printf("304\n");

   return ulRet;
}

ULONG SetNextCluster(PCDINFO pCD, ULONG ulCluster, ULONG ulNextCluster)
{
SETCLUSTERDATA SetCluster;
ULONG rc;
PULONG pulCluster;
ULONG  ulSector;
ULONG  dummy = 0;

   SetCluster.ulCluster = ulCluster;
   SetCluster.ulNextCluster = ulNextCluster;


   rc = DosDevIOCtl(pCD->hDisk, IOCTL_FAT32, FAT32_SETCLUSTER, 
          (PVOID)&SetCluster, sizeof(SetCluster), &dummy,
          NULL, 0, NULL);
   //rc = DosDevIOCtl2(NULL, 0,
   //   (PVOID)&SetCluster, sizeof(SetCluster),
   //   FAT32_SETCLUSTER, IOCTL_FAT32, pCD->hDisk);
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
ULONG rc;

   if (pCD->ulCurFATSector == ulSector)
      return TRUE;

   rc = ReadSector(pCD, pCD->ulActiveFatStart + ulSector, 1,
      (PBYTE)pCD->pbFATSector);
   if (rc)
      return FALSE;

   pCD->ulCurFATSector = ulSector;

   return TRUE;
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
            ulNext = GetNextCluster(pCD, ulNext, FALSE);
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
      ulStart = GetNextCluster(pCD, ulStart, FALSE);
      }
   return FALSE;
}

BOOL LostToFile(PCDINFO pCD, ULONG ulCluster, ULONG ulSize)
{
BYTE   szRecovered[CCHMAXPATH];
ULONG  rc, dummy1 = 0, dummy2 = 0;

   memset(szRecovered, 0, sizeof(szRecovered));
   rc = DosDevIOCtl(pCD->hDisk, IOCTL_FAT32, FAT32_RECOVERCHAIN, 
              (PVOID)&ulCluster, 4, &dummy1,
              szRecovered, sizeof(szRecovered), &dummy2);
   //rc = DosDevIOCtl2(szRecovered, sizeof(szRecovered),
   //   (PVOID)&ulCluster, 4,
   //   FAT32_RECOVERCHAIN, IOCTL_FAT32, pCD->hDisk);
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
