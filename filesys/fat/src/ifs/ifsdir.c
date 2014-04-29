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

int far pascal __loadds  FS_CHDIR(
    unsigned short usFlag,      /* flag     */
    struct cdfsi far * pcdfsi,      /* pcdfsi   */
    struct cdfsd far * pcdfsd,      /* pcdfsd   */
    char far * pDir,            /* pDir     */
    unsigned short usCurDirEnd      /* iCurDirEnd   */
)
{
PVOLINFO pVolInfo;
POPENINFO pOpenInfo = NULL;
ULONG ulCluster;
PSZ   pszFile;
USHORT rc;
BYTE     szDirLongName[ FAT32MAXPATH ];

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_CHDIR, flag %u", usFlag);

   switch (usFlag)
      {
      case CD_VERIFY   :
         pDir = pcdfsi->cdi_curdir;
         usCurDirEnd = 0xFFFF;

      case CD_EXPLICIT :
         if (f32Parms.fMessageActive & LOG_FS)
            Message("CHDIR to %s", pDir);
         if (strlen(pDir) > FAT32MAXPATH)
            {
            rc = ERROR_FILENAME_EXCED_RANGE;
            goto FS_CHDIREXIT;
            }

         pVolInfo = GetVolInfo(pcdfsi->cdi_hVPB);
         if (IsDriveLocked(pVolInfo))
            {
            rc = ERROR_DRIVE_LOCKED;
            goto FS_CHDIREXIT;
            }

         if( usFlag == CD_EXPLICIT )
         {
            pOpenInfo = malloc(sizeof (OPENINFO));
            if (!pOpenInfo)
            {
                rc = ERROR_NOT_ENOUGH_MEMORY;
                goto FS_CHDIREXIT;
            }
            memset(pOpenInfo, 0, sizeof (OPENINFO));

            if( TranslateName(pVolInfo, 0L, pDir, szDirLongName, TRANSLATE_SHORT_TO_LONG ))
               strcpy( szDirLongName, pDir );

            pOpenInfo->pSHInfo = GetSH( szDirLongName, pOpenInfo);
            if (!pOpenInfo->pSHInfo)
            {
                rc = ERROR_TOO_MANY_OPEN_FILES;
                goto FS_CHDIREXIT;
            }
            pOpenInfo->pSHInfo->sOpenCount++;
            if (pOpenInfo->pSHInfo->fLock)
            {
                rc = ERROR_ACCESS_DENIED;
                goto FS_CHDIREXIT;
            }
         }

         ulCluster = FindDirCluster(pVolInfo,
            pcdfsi,
            pcdfsd,
            pDir,
            usCurDirEnd,
            FILE_DIRECTORY,
            &pszFile);


         if (ulCluster == FAT_EOF || *pszFile)
            {
            rc = ERROR_PATH_NOT_FOUND;
            goto FS_CHDIREXIT;
            }

         if (ulCluster == pVolInfo->BootSect.bpb.RootDirStrtClus)
            {
            rc = 0;
            goto FS_CHDIREXIT;
            }

         *(PULONG)pcdfsd = ulCluster;


         if( usFlag == CD_EXPLICIT )
         {
            pOpenInfo->pSHInfo->bAttr = FILE_DIRECTORY;
            *((PULONG)pcdfsd + 1 ) = ( ULONG )pOpenInfo;
         }
         rc = 0;
         break;

      case CD_FREE     :
         pOpenInfo = ( POPENINFO )*(( PULONG )pcdfsd + 1 );
         ReleaseSH( pOpenInfo );
         pOpenInfo = NULL;
         rc = 0;
         break;
      default :
         rc = ERROR_INVALID_FUNCTION;
         break;
      }

FS_CHDIREXIT:

   if (rc && pOpenInfo)
      {
      if (pOpenInfo->pSHInfo)
         ReleaseSH(pOpenInfo);
      else
         free(pOpenInfo);
      }

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_CHDIR returned %u", rc);
   return rc;
}

int far pascal __loadds  FS_MKDIR(
    struct cdfsi far * pcdfsi,      /* pcdfsi   */
    struct cdfsd far * pcdfsd,      /* pcdfsd   */
    char far * pName,           /* pName    */
    unsigned short usCurDirEnd,     /* iCurDirEnd   */
    char far * pEABuf,          /* pEABuf   */
    unsigned short usFlags      /* flags    */
)
{
PVOLINFO pVolInfo;
ULONG    ulCluster;
ULONG    ulDirCluster;
PSZ      pszFile;
DIRENTRY DirEntry;
PDIRENTRY pDir;
USHORT   rc;
PBYTE    pbCluster;

   usFlags = usFlags;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_MKDIR - %s", pName);

   pVolInfo = GetVolInfo(pcdfsi->cdi_hVPB);
   if (IsDriveLocked(pVolInfo))
      return ERROR_DRIVE_LOCKED;
   if (!pVolInfo->fDiskCleanOnMount)
      return ERROR_VOLUME_DIRTY;
   if (pVolInfo->fWriteProtected)
      return ERROR_WRITE_PROTECT;

   if (strlen(pName) > FAT32MAXPATH)
      return ERROR_FILENAME_EXCED_RANGE;

   ulDirCluster = FindDirCluster(pVolInfo,
      pcdfsi,
      pcdfsd,
      pName,
      usCurDirEnd,
      RETURN_PARENT_DIR,
      &pszFile);
   if (ulDirCluster == FAT_EOF)
      {
      rc = ERROR_PATH_NOT_FOUND;
      goto FS_MKDIREXIT;
      }

   ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszFile, &DirEntry, NULL);
   if (ulCluster != FAT_EOF)
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_MKDIREXIT;
      }

   ulCluster = SetNextCluster( pVolInfo, FAT_ASSIGN_NEW, FAT_EOF);
   if (ulCluster == FAT_EOF)
      {
      rc = ERROR_DISK_FULL;
      goto FS_MKDIREXIT;
      }

   pbCluster = malloc(pVolInfo->usClusterSize);
   if (!pbCluster)
      {
      SetNextCluster( pVolInfo, ulCluster, 0L);
      rc = ERROR_NOT_ENOUGH_MEMORY;
      goto FS_MKDIREXIT;
      }

   memset(pbCluster, 0, pVolInfo->usClusterSize);

   pDir = (PDIRENTRY)pbCluster;

   pDir->wCluster = LOUSHORT(ulCluster);
   pDir->wClusterHigh = HIUSHORT(ulCluster);
   pDir->bAttr = FILE_DIRECTORY;

   rc = MakeDirEntry(pVolInfo, ulDirCluster, (PDIRENTRY)pbCluster, pszFile);
   if (rc)
      {
      free(pbCluster);
      goto FS_MKDIREXIT;
      }
   memset(pDir->bFileName, 0x20, 11);
   memcpy(pDir->bFileName, ".", 1);

   memcpy(pDir + 1, pDir, sizeof (DIRENTRY));
   pDir++;

   memcpy(pDir->bFileName, "..", 2);
   if (ulDirCluster == pVolInfo->BootSect.bpb.RootDirStrtClus)
      {
      pDir->wCluster = 0;
      pDir->wClusterHigh = 0;
      }
   else
      {
      pDir->wCluster = LOUSHORT(ulDirCluster);
      pDir->wClusterHigh = HIUSHORT(ulDirCluster);
      }
   pDir->bAttr = FILE_DIRECTORY;

   rc = WriteCluster( pVolInfo, ulCluster, pbCluster, DVIO_OPWRTHRU);
   free(pbCluster);
   if (rc)
      goto FS_MKDIREXIT;

   if (f32Parms.fEAS && pEABuf && pEABuf != MYNULL)
      rc = usModifyEAS(pVolInfo, ulDirCluster, pszFile, (PEAOP)pEABuf);

FS_MKDIREXIT:
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_MKDIR returned %u", rc);
   return rc;
}

int far pascal __loadds FS_RMDIR(
    struct cdfsi far * pcdfsi,      /* pcdfsi   */
    struct cdfsd far * pcdfsd,      /* pcdfsd   */
    char far * pName,           /* pName    */
    unsigned short usCurDirEnd      /* iCurDirEnd   */
)
{
PVOLINFO pVolInfo;
ULONG    ulCluster;
ULONG    ulNextCluster;
ULONG    ulDirCluster;
PSZ      pszFile;
DIRENTRY DirEntry;
PDIRENTRY pDir;
PDIRENTRY pWork, pMax;
USHORT   rc;
USHORT   usFileCount;
BYTE     szLongName[ FAT32MAXPATH ];

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_RMDIR %s", pName);

   pVolInfo = GetVolInfo(pcdfsi->cdi_hVPB);
   if (IsDriveLocked(pVolInfo))
      return ERROR_DRIVE_LOCKED;
   if (!pVolInfo->fDiskCleanOnMount)
      return ERROR_VOLUME_DIRTY;
   if (pVolInfo->fWriteProtected)
      return ERROR_WRITE_PROTECT;
   if (strlen(pName) > FAT32MAXPATH)
      return ERROR_FILENAME_EXCED_RANGE;

#if 1
   if( TranslateName(pVolInfo, 0L, pName, szLongName, TRANSLATE_SHORT_TO_LONG ))
      strcpy( szLongName, pName );

   rc = MY_ISCURDIRPREFIX( szLongName );
   if( rc )
     goto FS_RMDIREXIT;
#else
   rc = FSH_ISCURDIRPREFIX(pName);
   if (rc)
      goto FS_RMDIREXIT;
   rc = TranslateName(pVolInfo, 0L, pName, szName, TRANSLATE_AUTO);
   if (rc)
      goto FS_RMDIREXIT;
   rc = FSH_ISCURDIRPREFIX(szName);
   if (rc)
      goto FS_RMDIREXIT;
#endif

   ulDirCluster = FindDirCluster(pVolInfo,
      pcdfsi,
      pcdfsd,
      pName,
      usCurDirEnd,
      RETURN_PARENT_DIR,
      &pszFile);
   if (ulDirCluster == FAT_EOF)
      {
      rc = ERROR_PATH_NOT_FOUND;
      goto FS_RMDIREXIT;
      }

   ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszFile, &DirEntry, NULL);
   if (ulCluster == FAT_EOF || !(DirEntry.bAttr & FILE_DIRECTORY))
      {
      rc = ERROR_PATH_NOT_FOUND;
      goto FS_RMDIREXIT;
      }

   if (DirEntry.bAttr & FILE_READONLY)
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_RMDIREXIT;
      }

   pDir = malloc(pVolInfo->usClusterSize);
   if (!pDir)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      goto FS_RMDIREXIT;
      }


   ulNextCluster = ulCluster;
   usFileCount = 0;
   while (ulNextCluster != FAT_EOF)
      {
      rc = ReadCluster( pVolInfo, ulNextCluster, pDir, 0);
      if (rc)
         {
         free(pDir);
         goto FS_RMDIREXIT;
         }

      pWork = pDir;
      pMax = (PDIRENTRY)((PBYTE)pDir + pVolInfo->usClusterSize);
      while (pWork < pMax)
         {
         if (pWork->bFileName[0] && pWork->bFileName[0] != DELETED_ENTRY &&
             pWork->bAttr != FILE_LONGNAME)
            {
            if (memcmp(pWork->bFileName, ".       ", 8) &&
                memcmp(pWork->bFileName, "..      ", 8))
               usFileCount++;
            }
         pWork++;
         }
      ulNextCluster = GetNextCluster( pVolInfo, ulNextCluster);
      if (!ulNextCluster)
         ulNextCluster = FAT_EOF;
      }
   free(pDir);
   if (usFileCount)
      {
      Message("Cannot RMDIR, contains %u files", usFileCount);
      rc = ERROR_ACCESS_DENIED;
      goto FS_RMDIREXIT;
      }

   if (f32Parms.fEAS)
      {
      rc = usDeleteEAS(pVolInfo, ulDirCluster, pszFile);
      if (rc)
         goto FS_RMDIREXIT;
#if 0
      if (DirEntry.fEAS == FILE_HAS_EAS || DirEntry.fEAS == FILE_HAS_CRITICAL_EAS)
         DirEntry.fEAS = FILE_HAS_NO_EAS;
#endif
      }

   rc = ModifyDirectory(pVolInfo, ulDirCluster, MODIFY_DIR_DELETE,
      &DirEntry, NULL, NULL, DVIO_OPWRTHRU);
   if (rc)
      goto FS_RMDIREXIT;

   DeleteFatChain(pVolInfo, ulCluster);

FS_RMDIREXIT:
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_RMDIR returned %u", rc);
   return rc;
}

