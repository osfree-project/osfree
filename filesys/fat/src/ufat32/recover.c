#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <ctype.h>

#define INCL_BASE
#define INCL_DOSDEVIOCTL
#define INCL_LONGLONG
#include <os2.h>

#include "portable.h"
#include "fat32def.h"

BOOL  GetDiskStatus(PCDINFO pCD);
ULONG ReadSector(PCDINFO pCD, ULONG ulSector, USHORT nSectors, PBYTE pbSector);
INT cdecl iShowMessage(PCDINFO pCD, USHORT usNr, USHORT usNumFields, ...);
PSZ       GetOS2Error(USHORT rc);
ULONG FindPathCluster(PCDINFO pCD, ULONG ulCluster, PSZ pszPath, PDIRENTRY pDirEntry, PSZ pszFullName);
USHORT RecoverChain2(PCDINFO pCD, ULONG ulCluster, PBYTE pData, USHORT cbData);
BOOL LoadTranslateTable(VOID);

BOOL DoRecover(PCDINFO pCD, char *pszFilename);
int recover_thread(int argc, char *argv[]);
void remount_media (HFILE hDevice);

#define STACKSIZE 0x10000

int recover_thread(int argc, char *argv[])
{
   PCDINFO pCD;
   HFILE  hFile;
   ULONG  ulAction;
   BYTE   bSector[512];
   char   szTarget[0x8000];
   ULONG  ulBytes;
   USHORT usBlocks;
   APIRET  rc;
   int i;

   LoadTranslateTable();

   pCD = (PCDINFO)malloc(sizeof(CDINFO));

   if (!pCD)
      return ERROR_NOT_ENOUGH_MEMORY;

   memset(pCD, 0, sizeof (CDINFO));

   if (!strlen(pCD->szDrive))
      strncpy(pCD->szDrive, argv[1], 2);

   if (!strlen(pCD->szDrive))
      {
      ULONG ulDisk;
      ULONG  ulDrives;
      rc = DosQueryCurrentDisk(&ulDisk, &ulDrives);
      pCD->szDrive[0] = (BYTE)(ulDisk + '@');
      pCD->szDrive[1] = ':';
      }

   pCD->szDrive[0] = toupper(pCD->szDrive[0]);

   rc = DosOpenL(pCD->szDrive,
      &hFile,
      &ulAction,                         /* action taken */
      0LL,                               /* new size     */
      0,                                 /* attributes   */
      OPEN_ACTION_OPEN_IF_EXISTS,        /* open flags   */
      OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE | OPEN_FLAGS_DASD |
      OPEN_FLAGS_WRITE_THROUGH,         /* OPEN_FLAGS_NO_CACHE , */
      0L);
   if (rc)
      {
      if (rc == ERROR_DRIVE_LOCKED)
         iShowMessage(pCD, rc, 0);
      else
         printf("%s\n", GetOS2Error(rc));
      DosExit(EXIT_PROCESS, 1);
      }

   pCD->fCleanOnBoot = GetDiskStatus(pCD);

   if (pCD->fAutoRecover && pCD->fCleanOnBoot)
      pCD->fAutoRecover = FALSE;

   //if (pCD->fFix)
      //{
      rc = DosDevIOCtl(hFile, IOCTL_DISK, DSK_LOCKDRIVE,
                       NULL, 0, NULL,
                       NULL, 0, NULL);
      if (rc)
         {
         if (rc == ERROR_DRIVE_LOCKED)
            iShowMessage(pCD, rc, 0);
         else
            printf("%s\n", GetOS2Error(rc));
         DosExit(EXIT_PROCESS, 1);
         }
      //}

   rc = DosQueryFSInfo(pCD->szDrive[0] - '@', FSIL_ALLOC,
      (PBYTE)&pCD->DiskInfo, sizeof (DISKINFO));
   if (rc)
      {
      fprintf(stderr, "DosQueryFSInfo failed, %s\n", GetOS2Error(rc));
      DosExit(EXIT_PROCESS, 1);
      }
   pCD->hDisk = hFile;

   rc = ReadSector(pCD, 0, 1, bSector);
   if (rc)
      {
      printf("Error: Cannot read boot sector: %s\n", GetOS2Error(rc));
      return rc;
      }
   memcpy(&pCD->BootSect, bSector, sizeof (BOOTSECT));

   rc = ReadSector(pCD, pCD->BootSect.bpb.FSinfoSec, 1, bSector);
   if (rc)
      {
      printf("Error: Cannot read FSInfo sector\n%s\n", GetOS2Error(rc));
      return rc;
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

   //pCD->pFatBits = halloc(usBlocks,4096);
   pCD->pFatBits = calloc(usBlocks,4096);
   if (!pCD->pFatBits)
      {
      printf("Not enough memory for FATBITS\n");
      return ERROR_NOT_ENOUGH_MEMORY;
      }

   memset(szTarget, 0, sizeof(szTarget));

   for (i = 1; i < argc; i++)
      {
      //printf("%s\n", argv[i]);
      DosEditName(1, argv[i], "*", szTarget, sizeof(szTarget));
      printf("%s\n", szTarget);
      DoRecover(pCD, argv[i]);
      }   

   remount_media(hFile);
   //if (pCD->fFix)
      //{
      rc = DosDevIOCtl(hFile, IOCTL_DISK, DSK_UNLOCKDRIVE,
                       NULL, 0, NULL,
                       NULL, 0, NULL);
      if (rc)
         {
         printf("The drive cannot be unlocked. SYS%4.4u\n", rc);
         DosExit(EXIT_PROCESS, 1);
         }
      //}
   DosClose(hFile);
   free(pCD);

   return 0;
}

int recover(int argc, char *argv[], char *envp[])
{
  char *stack;
  APIRET rc;

  // Here we're switching stack, because the original
  // recover.com stack is too small.

  // allocate stack
  rc = DosAllocMem((void **)&stack, 
                   STACKSIZE, 
                   PAG_READ | PAG_WRITE | 
                   PAG_COMMIT | OBJ_TILE);

  if (rc)
    return rc;

  // call recover_thread on new stack
  _asm {
    mov eax, esp
    mov edx, stack
    mov ecx, argv
    add edx, STACKSIZE - 4
    mov esp, edx
    push eax
    push ecx
    mov ecx, argc
    push ecx
    call recover_thread
    add esp, 8
    pop esp
  }

  // deallocate new stack
  DosFreeMem(stack);

  return 0;
}

BOOL DoRecover(PCDINFO pCD, char *pszFilename)
{
   ULONG ulCluster = pCD->BootSect.bpb.RootDirStrtClus;
   BYTE  szRecovered[CCHMAXPATH];
   DIRENTRY DirEntry;
   APIRET rc;

   ulCluster = FindPathCluster(pCD, ulCluster, pszFilename, &DirEntry, NULL);

   //printf("ulCluster=%lx\n", ulCluster);

   if (ulCluster == FAT_EOF)
      return FALSE;

   memset(szRecovered, 0, sizeof(szRecovered));
   rc = RecoverChain2(pCD, ulCluster, szRecovered, sizeof(szRecovered));

   //printf("rc=%lu\n", rc);

   if (rc)
      {
      pCD->ulErrorCount++;
      printf("RECOVER was unable to recover a lost chain. SYS%4.4u\n", rc);
      return FALSE;
      }

   return TRUE;
}
