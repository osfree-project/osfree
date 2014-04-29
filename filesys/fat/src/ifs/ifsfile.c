#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>

#define INCL_DOS
#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_DOSERRORS

#include "os2.h"
#include "portable.h"
#include "fat32ifs.h"

PRIVATE volatile PSHOPENINFO pGlobSH = NULL;

PRIVATE VOID ResetAllCurrents(POPENINFO pOI);

PRIVATE ULONG PositionToOffset(PVOLINFO pVolInfo, POPENINFO pOpenInfo, ULONG ulOffset);
PRIVATE USHORT NewSize(PVOLINFO pVolInfo,
    struct sffsi far * psffsi,      /* psffsi   */
    struct sffsd far * psffsd,      /* psffsd   */
    ULONG ulLen,
    USHORT usIOFlag);

/******************************************************************
*
******************************************************************/
int far pascal __loadds  FS_OPENCREATE(
    struct cdfsi far * pcdfsi,      /* pcdfsi       */
    void far * pcdfsd,     /* pcdfsd      */
    char far * pName,           /* pName        */
    unsigned short usCurDirEnd,     /* iCurDirEnd       */
    struct sffsi far * psffsi,      /* psffsi       */
    struct sffsd far * psffsd,      /* psffsd       */
    unsigned long ulOpenMode,       /* fhandflag/openmode   */
    unsigned short usOpenFlag,      /* openflag     */
    unsigned short far * pAction,   /* pAction      */
    unsigned short usAttr,      /* attr         */
    char far * pEABuf,          /* pEABuf       */
    unsigned short far * pfGenFlag  /* pfgenFlag        */
)
{
PVOLINFO pVolInfo;
ULONG    ulCluster;
ULONG    ulDirCluster;
PSZ      pszFile;
DIRENTRY DirEntry;
POPENINFO pOpenInfo = NULL;
USHORT   usIOMode;
USHORT rc;


   usIOMode = 0;
   if (ulOpenMode & OPEN_FLAGS_NO_CACHE)
      usIOMode |= DVIO_OPNCACHE;
   if (ulOpenMode & OPEN_FLAGS_WRITE_THROUGH)
      usIOMode |= DVIO_OPWRTHRU;

   if (f32Parms.fMessageActive & LOG_FS)
      {
      Message("FS_OPENCREATE for %s mode %lX, Flag %X, IOMode %X, selfsfn=%u",
         pName, ulOpenMode, usOpenFlag, usIOMode, psffsi->sfi_selfsfn);
      Message("              attribute %X, pEABuf %lX", usAttr, pEABuf);
      }

   pVolInfo = GetVolInfo(pcdfsi->cdi_hVPB);
   if (IsDriveLocked(pVolInfo))
      return ERROR_DRIVE_LOCKED;

   *pAction = 0;

   if (strlen(pName) > FAT32MAXPATH)
      {
      rc = ERROR_FILENAME_EXCED_RANGE;
      goto FS_OPENCREATEEXIT;
      }

   pOpenInfo = malloc(sizeof (OPENINFO));
   if (!pOpenInfo)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      goto FS_OPENCREATEEXIT;
      }
   memset(pOpenInfo, 0, sizeof (OPENINFO));
   *(POPENINFO *)psffsd = pOpenInfo;

   if ((ulOpenMode & OPEN_ACCESS_EXECUTE) == OPEN_ACCESS_EXECUTE)
      {
      ulOpenMode &= ~OPEN_ACCESS_EXECUTE;
      ulOpenMode |= OPEN_ACCESS_READONLY;
      }

   if (!(ulOpenMode & OPEN_FLAGS_DASD))
      {
      BYTE szLongName[ FAT32MAXPATH ];

      if( TranslateName(pVolInfo, 0L, pName, szLongName, TRANSLATE_SHORT_TO_LONG ))
         strcpy( szLongName, pName );

      pOpenInfo->pSHInfo = GetSH( szLongName, pOpenInfo);
      if (!pOpenInfo->pSHInfo)
         {
         rc = ERROR_TOO_MANY_OPEN_FILES;
         goto FS_OPENCREATEEXIT;
         }
      pOpenInfo->pSHInfo->sOpenCount++;
      if (pOpenInfo->pSHInfo->fLock)
         {
         rc = ERROR_ACCESS_DENIED;
         goto FS_OPENCREATEEXIT;
         }
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
         goto FS_OPENCREATEEXIT;
         }

#if 0
      if (f32Parms.fEAS)
         {
         if (IsEASFile(pszFile))
            {
            rc = ERROR_ACCESS_DENIED;
            goto FS_OPENCREATEEXIT;
            }
         }
#endif

      ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszFile, &DirEntry, NULL);
      if (pOpenInfo->pSHInfo->sOpenCount > 1)
         {
         if (pOpenInfo->pSHInfo->bAttr & FILE_DIRECTORY)
            {
            rc = ERROR_ACCESS_DENIED;
            goto FS_OPENCREATEEXIT;
            }

         if (f32Parms.fMessageActive & LOG_FS)
            Message("File has been previously opened!");
         ulCluster    = pOpenInfo->pSHInfo->ulStartCluster;
         DirEntry.bAttr = pOpenInfo->pSHInfo->bAttr;
         }

      if (ulCluster == FAT_EOF)
         {
         if (!(usOpenFlag & FILE_CREATE))
            {
            rc = ERROR_OPEN_FAILED;
            goto FS_OPENCREATEEXIT;
            }

         if (pVolInfo->fWriteProtected)
            {
            rc = ERROR_WRITE_PROTECT;
            goto FS_OPENCREATEEXIT;
            }

         }
      else
         {
         if (DirEntry.bAttr & FILE_DIRECTORY)
            {
            rc = ERROR_ACCESS_DENIED;
            goto FS_OPENCREATEEXIT;
            }

         if (!(usOpenFlag & (FILE_OPEN | FILE_TRUNCATE)))
            {
            rc = ERROR_OPEN_FAILED;
            goto FS_OPENCREATEEXIT;
            }

         if (DirEntry.bAttr & FILE_READONLY &&
           (ulOpenMode & OPEN_ACCESS_WRITEONLY ||
               ulOpenMode & OPEN_ACCESS_READWRITE))
            {
            if ((psffsi->sfi_type & STYPE_FCB) && (ulOpenMode & OPEN_ACCESS_READWRITE))
               {
               ulOpenMode &= ~(OPEN_ACCESS_WRITEONLY | OPEN_ACCESS_READWRITE);
               psffsi->sfi_mode &= ~(OPEN_ACCESS_WRITEONLY | OPEN_ACCESS_READWRITE);
               }
            else
               {
               rc = ERROR_ACCESS_DENIED;
               goto FS_OPENCREATEEXIT;
               }
            }
         }

      if (!pVolInfo->fDiskCleanOnMount &&
        (ulOpenMode & OPEN_ACCESS_WRITEONLY ||
         ulOpenMode & OPEN_ACCESS_READWRITE))
         {
         rc = ERROR_VOLUME_DIRTY;
         goto FS_OPENCREATEEXIT;
         }

      if (ulCluster == FAT_EOF)
         {
         memset(&DirEntry, 0, sizeof (DIRENTRY));
         DirEntry.bAttr = (BYTE)(usAttr & (FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_ARCHIVED));
         ulCluster = 0;

         if (psffsi->sfi_size > (ULONG)LONG_MAX)
            psffsi->sfi_size = (ULONG)LONG_MAX;

         if (psffsi->sfi_size > 0)
            {
            ULONG ulClustersNeeded = psffsi->sfi_size / pVolInfo->usClusterSize +
                  (psffsi->sfi_size % pVolInfo->usClusterSize ? 1:0);
            ulCluster = MakeFatChain(pVolInfo, FAT_EOF, ulClustersNeeded, NULL);
            if (ulCluster != FAT_EOF)
               {
               DirEntry.wCluster = LOUSHORT(ulCluster);
               DirEntry.wClusterHigh = HIUSHORT(ulCluster);
               DirEntry.ulFileSize = psffsi->sfi_size;
               }
            else
               {
               rc = ERROR_DISK_FULL;
               goto FS_OPENCREATEEXIT;
               }
            }

         rc = MakeDirEntry(pVolInfo, ulDirCluster, &DirEntry, pszFile);
         if (rc)
            goto FS_OPENCREATEEXIT;

         memcpy(&psffsi->sfi_ctime, &DirEntry.wCreateTime, sizeof (USHORT));
         memcpy(&psffsi->sfi_cdate, &DirEntry.wCreateDate, sizeof (USHORT));
         psffsi->sfi_atime = 0;
         memcpy(&psffsi->sfi_adate, &DirEntry.wAccessDate, sizeof (USHORT));
         memcpy(&psffsi->sfi_mtime, &DirEntry.wLastWriteTime, sizeof (USHORT));
         memcpy(&psffsi->sfi_mdate, &DirEntry.wLastWriteDate, sizeof (USHORT));

         pOpenInfo->pSHInfo->fMustCommit = TRUE;

         psffsi->sfi_tstamp = ST_SCREAT | ST_PCREAT | ST_SREAD | ST_PREAD | ST_SWRITE | ST_PWRITE;

         *pfGenFlag = 0;
         if (f32Parms.fEAS && pEABuf && pEABuf != MYNULL)
            {
            rc = usModifyEAS(pVolInfo, ulDirCluster, pszFile, (PEAOP)pEABuf);
            if (rc)
               goto FS_OPENCREATEEXIT;
            }
         *pAction   = FILE_CREATED;
         }
      else if (usOpenFlag & FILE_TRUNCATE)
         {
         DIRENTRY DirOld;

         if (pVolInfo->fWriteProtected)
            {
            rc = ERROR_WRITE_PROTECT;
            goto FS_OPENCREATEEXIT;
            }

         if ((DirEntry.bAttr & FILE_HIDDEN) && !(usAttr & FILE_HIDDEN))
            {
            rc = ERROR_ACCESS_DENIED;
            goto FS_OPENCREATEEXIT;
            }

         if ((DirEntry.bAttr & FILE_SYSTEM) && !(usAttr & FILE_SYSTEM))
            {
            rc = ERROR_ACCESS_DENIED;
            goto FS_OPENCREATEEXIT;
            }

         memcpy(&DirOld, &DirEntry, sizeof (DIRENTRY));

         DirEntry.bAttr = (BYTE)(usAttr & (FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_ARCHIVED));
         DirEntry.wCluster     = 0;
         DirEntry.wClusterHigh = 0;
         DirEntry.ulFileSize   = 0;
         DirEntry.fEAS = FILE_HAS_NO_EAS;

         rc = usDeleteEAS(pVolInfo, ulDirCluster, pszFile);
         if (rc)
            goto FS_OPENCREATEEXIT;
         DirOld.fEAS = FILE_HAS_NO_EAS;

         if (ulCluster)
            DeleteFatChain(pVolInfo, ulCluster);
         pOpenInfo->pSHInfo->ulLastCluster = FAT_EOF;
         ResetAllCurrents(pOpenInfo);
         ulCluster = 0;

         if (psffsi->sfi_size > (ULONG)LONG_MAX)
            psffsi->sfi_size = (ULONG)LONG_MAX;
         if (psffsi->sfi_size > 0)
            {
            ULONG ulClustersNeeded = psffsi->sfi_size / pVolInfo->usClusterSize +
                  (psffsi->sfi_size % pVolInfo->usClusterSize ? 1:0);
            ulCluster = MakeFatChain(pVolInfo, FAT_EOF, ulClustersNeeded, &pOpenInfo->pSHInfo->ulLastCluster);
            if (ulCluster != FAT_EOF)
               {
               DirEntry.wCluster = LOUSHORT(ulCluster);
               DirEntry.wClusterHigh = HIUSHORT(ulCluster);
               DirEntry.ulFileSize = psffsi->sfi_size;
               }
            else
               {
               ulCluster = 0;
               rc = ModifyDirectory(pVolInfo, ulDirCluster, MODIFY_DIR_UPDATE,
                  &DirOld, &DirEntry, NULL, usIOMode);
               if (!rc)
                  rc = ERROR_DISK_FULL;
               goto FS_OPENCREATEEXIT;
               }
            }
         rc = ModifyDirectory(pVolInfo, ulDirCluster, MODIFY_DIR_UPDATE,
            &DirOld, &DirEntry, NULL, usIOMode);
         if (rc)
            goto FS_OPENCREATEEXIT;

         memcpy(&psffsi->sfi_ctime, &DirEntry.wCreateTime, sizeof (USHORT));
         memcpy(&psffsi->sfi_cdate, &DirEntry.wCreateDate, sizeof (USHORT));
         psffsi->sfi_atime = 0;
         memcpy(&psffsi->sfi_adate, &DirEntry.wAccessDate, sizeof (USHORT));
         memcpy(&psffsi->sfi_mtime, &DirEntry.wLastWriteTime, sizeof (USHORT));
         memcpy(&psffsi->sfi_mdate, &DirEntry.wLastWriteDate, sizeof (USHORT));

         pOpenInfo->pSHInfo->fMustCommit = TRUE;

         psffsi->sfi_tstamp = ST_SWRITE | ST_PWRITE | ST_SREAD | ST_PREAD;
         *pfGenFlag = 0;
         if (f32Parms.fEAS && pEABuf && pEABuf != MYNULL)
            {
            rc = usModifyEAS(pVolInfo, ulDirCluster, pszFile, (PEAOP)pEABuf);
            if (rc)
               goto FS_OPENCREATEEXIT;
            }

         *pAction = FILE_TRUNCATED;
         }
      else
         {
         memcpy(&psffsi->sfi_ctime, &DirEntry.wCreateTime, sizeof (USHORT));
         memcpy(&psffsi->sfi_cdate, &DirEntry.wCreateDate, sizeof (USHORT));
         psffsi->sfi_atime = 0;
         memcpy(&psffsi->sfi_adate, &DirEntry.wAccessDate, sizeof (USHORT));
         memcpy(&psffsi->sfi_mtime, &DirEntry.wLastWriteTime, sizeof (USHORT));
         memcpy(&psffsi->sfi_mdate, &DirEntry.wLastWriteDate, sizeof (USHORT));

         psffsi->sfi_tstamp = 0;
         *pAction = FILE_EXISTED;
         *pfGenFlag = ( HAS_CRITICAL_EAS( DirEntry.fEAS ) ? 1 : 0);
         }

#if 0
      rc = FSH_UPPERCASE(pName, sizeof pOpenInfo->pSHInfo->szFileName, pOpenInfo->pSHInfo->szFileName);
      if (rc || !strlen(pOpenInfo->pSHInfo->szFileName))
         {
         Message("OpenCreate: FSH_UPPERCASE failed!, rc = %d", rc);
         strncpy(pOpenInfo->pSHInfo->szFileName, pName, sizeof pOpenInfo->pSHInfo->szFileName);
         rc = 0;
         }
#endif

      if (pOpenInfo->pSHInfo->sOpenCount == 1)
         {
         pOpenInfo->pSHInfo->ulLastCluster = GetLastCluster(pVolInfo, ulCluster);
         pOpenInfo->pSHInfo->bAttr = DirEntry.bAttr;
         }

      pOpenInfo->pSHInfo->ulDirCluster = ulDirCluster;
      pOpenInfo->pSHInfo->ulStartCluster = ulCluster;
      if (ulCluster)
         pOpenInfo->ulCurCluster = ulCluster;
      else
         pOpenInfo->ulCurCluster = FAT_EOF;

      psffsi->sfi_size = DirEntry.ulFileSize;
      psffsi->sfi_DOSattr = DirEntry.bAttr;
      }
   else /* OPEN_FLAGS_DASD */
      {
      if (!usOpenFlag)
         {
         rc = ERROR_FILE_EXISTS;
         goto FS_OPENCREATEEXIT;
         }

      if (usOpenFlag != FILE_OPEN)
         {
         rc = ERROR_ACCESS_DENIED;
         goto FS_OPENCREATEEXIT;
         }

      psffsi->sfi_size = pVolInfo->BootSect.bpb.BigTotalSectors * SECTOR_SIZE;
      psffsi->sfi_ctime = 0;
      psffsi->sfi_cdate = 0;
      psffsi->sfi_atime = 0;
      psffsi->sfi_adate = 0;
      psffsi->sfi_mtime = 0;
      psffsi->sfi_mdate = 0;

      *pAction = FILE_EXISTED;
      }

   psffsi->sfi_position = 0L;
   psffsi->sfi_type &= ~STYPE_FCB;
   psffsi->sfi_mode = ulOpenMode;
   pVolInfo->ulOpenFiles++;

   rc = 0;

FS_OPENCREATEEXIT:

   if (rc && pOpenInfo)
      {
      if (pOpenInfo->pSHInfo)
         ReleaseSH(pOpenInfo);
      else
         free(pOpenInfo);
      }

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_OPENCREATE returned %u (Action = %u, OI=%lX)", rc, *pAction, pOpenInfo);
   return rc;
}

/******************************************************************
* GetSH
******************************************************************/
PSHOPENINFO GetSH(PSZ pszFileName, POPENINFO pOI)
{
PSHOPENINFO pSH;

   pSH = pGlobSH;
   while (pSH)
      {
      if (!stricmp(pSH->szFileName, pszFileName))
         break;
      pSH = (PSHOPENINFO)pSH->pNext;
      }

   if (!pSH)
      {
      pSH = malloc(sizeof (SHOPENINFO));
      if (!pSH)
         return NULL;
      memset(pSH, 0, sizeof (SHOPENINFO));
      strcpy(pSH->szFileName, pszFileName);

      pSH->pNext = (PVOID)pGlobSH;
      pGlobSH = pSH;
      }

   pOI->pNext = pSH->pChild;
   pSH->pChild = pOI;

   return pSH;
}

/******************************************************************
*  Release shared info
******************************************************************/
BOOL ReleaseSH(POPENINFO pOI)
{
PSHOPENINFO pSH2;
PSHOPENINFO pSH = pOI->pSHInfo;
POPENINFO pOI2;
USHORT rc;

   /*
      Remove the openinfo from the chain
   */

   if ((POPENINFO)pSH->pChild == pOI)
      pSH->pChild = pOI->pNext;
   else
      {
      pOI2 = pSH->pChild;
      while (pOI2)
         {
         rc = MY_PROBEBUF(PB_OPREAD, (PBYTE)pOI2, sizeof (OPENINFO));
         if (rc)
            {
            CritMessage("FAT32: Protection VIOLATION (OpenInfo) in ReleaseSH! (SYS%d)", rc);
            Message("FAT32: Protection VIOLATION (OpenInfo) in ReleaseSH! (SYS%d)", rc);
            return FALSE;
            }
         if ((POPENINFO)pOI2->pNext == pOI)
            {
            pOI2->pNext = pOI->pNext;
            break;
            }
         pOI2 = (POPENINFO)pOI2->pNext;
         }
      if (!pOI2)
         {
         CritMessage("FAT32: ReleaseSH: Error cannot find OI for %s!", pSH->szFileName);
         Message("FAT32: ReleaseSH: Error cannot find OI for %s!", pSH->szFileName);
         }
      }
   free(pOI);

   /*
      Now release the SHOPENINFO if needed
   */

   pSH->sOpenCount--;
   if (pSH->sOpenCount > 0)
      return TRUE;

   if (pGlobSH == pSH)
      pGlobSH = pSH->pNext;
   else
      {
      pSH2 = pGlobSH;
      while (pSH2)
         {
         rc = MY_PROBEBUF(PB_OPREAD, (PBYTE)pSH2, sizeof (SHOPENINFO));
         if (rc)
            {
            CritMessage("FAT32: Protection VIOLATION (SHOpenInfo) in ReleaseSH! (SYS%d)", rc);
            Message("FAT32: Protection VIOLATION (SHOpenInfo) in ReleaseSH! (SYS%d)", rc);
            return FALSE;
            }
         if ((PSHOPENINFO)pSH2->pNext == pSH)
            {
            pSH2->pNext = pSH->pNext;
            break;
            }
         pSH2 = (PSHOPENINFO)pSH2->pNext;
         }
      if (!pSH2)
         {
         CritMessage("FAT32: ReleaseSH: Error cannot find SH for %s!", pSH->szFileName);
         Message("FAT32: ReleaseSH: Error cannot find SH for %s!", pSH->szFileName);
         }
      }

   free(pSH);
   return TRUE;
}

VOID ResetAllCurrents(POPENINFO pOI)
{
PSHOPENINFO pSH = pOI->pSHInfo;

   pOI = (POPENINFO)pSH->pChild;
   while (pOI)
      {
      pOI->ulCurCluster = FAT_EOF;
      pOI = (POPENINFO)pOI->pNext;
      }
}

/******************************************************************
*
******************************************************************/
int far pascal __loadds  FS_CLOSE(
    unsigned short usType,      /* close type   */
    unsigned short IOFlag,      /* IOflag   */
    struct sffsi far * psffsi,      /* psffsi   */
    struct sffsd far * psffsd       /* psffsd   */
)
{
POPENINFO pOpenInfo = GetOpenInfo(psffsd);
PVOLINFO pVolInfo = GetVolInfo(psffsi->sfi_hVPB);
USHORT  rc = 0;

   if (f32Parms.fMessageActive & LOG_FS)
      {
      if (psffsi->sfi_mode & OPEN_FLAGS_DASD)
         Message("FS_CLOSE (DASD) type %u:", usType);
      else
         Message("FS_CLOSE of %s, type = %u OI=%lX",
            pOpenInfo->pSHInfo->szFileName,
            usType,
            pOpenInfo);
      }

   if (IsDriveLocked(pVolInfo))
      return ERROR_DRIVE_LOCKED;

   if (usType == FS_CL_FORSYS)
      {
      if (!pVolInfo->ulOpenFiles)
         {
         Message("FAT32 - FS_CLOSE: Error openfile count would become negative!");
         CritMessage("FAT32 - FS_CLOSE: Error openfile count would become negative!");
         }
      else
         pVolInfo->ulOpenFiles--;
      }

   if (psffsi->sfi_mode & OPEN_FLAGS_DASD)
      {
      rc = 0;
      goto FS_CLOSEEXIT;
      }

   if (pOpenInfo->pSHInfo->fMustCommit && !pVolInfo->fWriteProtected)
      {
      if (usType != FS_CL_ORDINARY || IOFlag & DVIO_OPWRTHRU)
         rc = FS_COMMIT(FS_COMMIT_ONE, IOFlag, psffsi, psffsd);
      }

   if (usType == FS_CL_FORSYS)
      ReleaseSH(pOpenInfo);

FS_CLOSEEXIT:
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_CLOSE returned %u", rc);
   return rc;
}

/******************************************************************
*
******************************************************************/
int far pascal __loadds  FS_READ(
    struct sffsi far * psffsi,      /* psffsi   */
    struct sffsd far * psffsd,      /* psffsd   */
    char far * pData,           /* pData    */
    unsigned short far * pLen,  /* pLen     */
    unsigned short usIOFlag     /* IOflag   */
)
{
USHORT rc;
PVOLINFO pVolInfo;
POPENINFO pOpenInfo = GetOpenInfo(psffsd);
USHORT usBytesRead;
USHORT usBytesToRead;
PBYTE  pbCluster;
ULONG  ulClusterSector;
USHORT usClusterOffset;
USHORT usBytesPerCluster;


   usBytesToRead = *pLen;
   usBytesRead = 0;
   *pLen = 0;
   pbCluster = NULL;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_READ, %u bytes at offset %ld",
         usBytesToRead, psffsi->sfi_position);

   pVolInfo = GetVolInfo(psffsi->sfi_hVPB);
   if (IsDriveLocked(pVolInfo))
   {
      rc = ERROR_DRIVE_LOCKED;
      goto FS_READEXIT;
   }

   if (!((psffsi->sfi_mode & 0xFUL) == OPEN_ACCESS_READONLY) &&
       !((psffsi->sfi_mode & 0xFUL) == OPEN_ACCESS_READWRITE))
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_READEXIT;
      }

   if (!usBytesToRead)
      {
      rc = NO_ERROR;
      goto FS_READEXIT;
      }

   pbCluster = malloc(pVolInfo->usClusterSize);
   if (!pbCluster)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      goto FS_READEXIT;
      }

    if (psffsi->sfi_mode & OPEN_FLAGS_DASD)
    {
        ULONG   ulSector;
        USHORT  usSectorOffset;
        USHORT  usRemaining;
        USHORT  usBytesPerSector;
        USHORT  usNumSectors;

        char far *pBufPosition = pData;

        usBytesPerSector    = pVolInfo->BootSect.bpb.BytesPerSector;
        usBytesRead         = 0;

        if (pOpenInfo->fSectorMode)
        {
            USHORT usTotNumBytes;

            if (usBytesToRead > (USHRT_MAX/usBytesPerSector))
            {
                rc = ERROR_TRANSFER_TOO_LONG;
                goto FS_READEXIT;
            }

            usTotNumBytes = usBytesToRead * usBytesPerSector;

            rc = MY_PROBEBUF(PB_OPWRITE, pBufPosition, usTotNumBytes);
            if (rc)
            {
                Message("Protection VIOLATION in FS_READ! (SYS%d)", rc);
                goto FS_READEXIT;
            }

            /*
                for sector mode, offsets are actually sectors
                and Number of Bytes to read are actually
                Number of sectors to read
            */
            ulSector        = psffsi->sfi_position;
            usSectorOffset  = 0;
            usRemaining     = 0;
            usNumSectors    = usBytesToRead;
            if (ulSector > (pVolInfo->BootSect.bpb.BigTotalSectors - (ULONG)usNumSectors))
            {
                usNumSectors    = (USHORT)(pVolInfo->BootSect.bpb.BigTotalSectors - ulSector);
            }
            usBytesToRead   = usNumSectors;
        }
        else
        {
            rc = MY_PROBEBUF(PB_OPWRITE, pData, usBytesToRead);
            if (rc)
            {
                Message("Protection VIOLATION in FS_READ! (SYS%d)", rc);
                goto FS_READEXIT;
            }

            ulSector        = (ULONG)(psffsi->sfi_position / (ULONG)usBytesPerSector);
            usSectorOffset  = (USHORT)(psffsi->sfi_position % (ULONG)usBytesPerSector);
            usRemaining     = (usSectorOffset + usBytesToRead) % usBytesPerSector;
            usNumSectors    = (usBytesToRead - usRemaining)/usBytesPerSector;

            if (ulSector > (pVolInfo->BootSect.bpb.BigTotalSectors - (ULONG)usNumSectors - (ULONG)(usRemaining ? 1 : 0)))
            {
                usNumSectors    = (USHORT)(pVolInfo->BootSect.bpb.BigTotalSectors - ulSector);
                usRemaining     = 0;
            }
            usBytesToRead   = usNumSectors*usBytesPerSector;
        }

        if (ulSector >= pVolInfo->BootSect.bpb.BigTotalSectors)
        {
            rc = ERROR_SECTOR_NOT_FOUND;
            goto FS_READEXIT;
        }

        if (usSectorOffset)
        {
            rc = ReadSector(pVolInfo, ulSector, 1, pbCluster, usIOFlag);
            if (rc)
            {
                goto FS_READEXIT;
            }
            memcpy(pBufPosition,pbCluster + usSectorOffset,usBytesPerSector-usSectorOffset);
            pBufPosition            += (usBytesPerSector - usSectorOffset);
            psffsi->sfi_position    += (usBytesPerSector - usSectorOffset);
            usBytesRead             += (usBytesPerSector - usSectorOffset);
        }

        if (usNumSectors)
        {
            rc = ReadSector(pVolInfo, ulSector, usNumSectors, pBufPosition, usIOFlag);
            if (rc)
            {
                goto FS_READEXIT;
            }
            pBufPosition            += (ULONG)((ULONG)usNumSectors*(ULONG)usBytesPerSector);
            psffsi->sfi_position    += usBytesToRead;
            usBytesRead             += usBytesToRead;
        }

        if (usRemaining)
        {
            rc = ReadSector(pVolInfo, ulSector, 1, pbCluster, usIOFlag);
            if (rc)
            {
                goto FS_READEXIT;
            }
            memcpy(pBufPosition,pbCluster,usRemaining);
            pBufPosition            += usRemaining;
            psffsi->sfi_position    += usRemaining;
            usBytesRead             += usRemaining;
        }

        *pLen                       = usBytesRead;
        rc = NO_ERROR;
    }
    else
    {
        char far *pBufPosition = pData;

        rc = MY_PROBEBUF(PB_OPWRITE, pData, usBytesToRead);
        if (rc)
        {
            Message("Protection VIOLATION in FS_READ! (SYS%d)", rc);
            goto FS_READEXIT;
        }

        pOpenInfo->pSHInfo->fMustCommit = TRUE;
        if (pOpenInfo->ulCurCluster == FAT_EOF)
            pOpenInfo->ulCurCluster = PositionToOffset(pVolInfo, pOpenInfo, psffsi->sfi_position);

        /*
            First, handle the first part that does not align on a cluster border
        */
        usBytesPerCluster = pVolInfo->usClusterSize;
        usClusterOffset   = (USHORT)(psffsi->sfi_position % usBytesPerCluster); /* get remainder */
        if
            (
                (pOpenInfo->ulCurCluster != FAT_EOF) &&
                (psffsi->sfi_position < psffsi->sfi_size) &&
                (usBytesToRead) &&
                (usClusterOffset)
            )
        {
            USHORT  usCurrBytesToRead;
            USHORT  usSectorsToRead;
            USHORT  usSectorsPerCluster;

            usSectorsPerCluster = pVolInfo->BootSect.bpb.SectorsPerCluster;
            usSectorsToRead = usSectorsPerCluster;

            /* compute the number of bytes
                to the cluster end or
                as much as fits into the user buffer
                or how much remains to be read until file end
               whatever is the smallest
            */
            usCurrBytesToRead   = (usBytesPerCluster - usClusterOffset);
            usCurrBytesToRead   = min(usCurrBytesToRead,usBytesToRead);
            usCurrBytesToRead   = (USHORT)min((ULONG)usCurrBytesToRead,psffsi->sfi_size-psffsi->sfi_position);
            if (usCurrBytesToRead)
            {
                ulClusterSector = pVolInfo->ulStartOfData + (pOpenInfo->ulCurCluster-2)*usSectorsPerCluster;

                rc = ReadSector(pVolInfo, ulClusterSector,usSectorsToRead,pbCluster, usIOFlag);
                if (rc)
                {
                    goto FS_READEXIT;
                }
                memcpy(pBufPosition, pbCluster + usClusterOffset, usCurrBytesToRead);

                pBufPosition            += usCurrBytesToRead;
                psffsi->sfi_position    += usCurrBytesToRead;
                usBytesRead             += usCurrBytesToRead;
                usBytesToRead           -= usCurrBytesToRead;
            }

            if ((usClusterOffset + usCurrBytesToRead) >= usBytesPerCluster)
            {
                pOpenInfo->ulCurCluster     = GetNextCluster(pVolInfo, pOpenInfo->ulCurCluster);
                if (!pOpenInfo->ulCurCluster)
                {
                    pOpenInfo->ulCurCluster = FAT_EOF;
                }
            }
        }


        /*
            Second, handle the part that aligns on a cluster border and is a multiple of the cluster size
        */
        if
            (
                (pOpenInfo->ulCurCluster != FAT_EOF) &&
                (psffsi->sfi_position < psffsi->sfi_size) &&
                (usBytesToRead)
            )
        {
            ULONG   ulCurrCluster       = pOpenInfo->ulCurCluster;
            ULONG   ulNextCluster       = ulCurrCluster;
            USHORT  usCurrBytesToRead   = 0;
            USHORT  usSectorsPerCluster = pVolInfo->BootSect.bpb.SectorsPerCluster;
            USHORT  usClustersToProcess = 0;
            USHORT  usAdjacentClusters  = 1;

            usCurrBytesToRead           = (USHORT)min((ULONG)usBytesToRead,psffsi->sfi_size - psffsi->sfi_position);

            usClustersToProcess         = usCurrBytesToRead / usBytesPerCluster; /* get the number of full clusters */

            while (usClustersToProcess && (ulCurrCluster != FAT_EOF))
            {
                ulNextCluster       = GetNextCluster(pVolInfo, ulCurrCluster);
                if (!ulNextCluster)
                {
                    ulNextCluster = FAT_EOF;
                }

                usClustersToProcess -= 1;

                if  (
                        (ulNextCluster != FAT_EOF) &&
                        (ulNextCluster == (ulCurrCluster+1)) &&
                        (usClustersToProcess)
                    )
                {
                    usAdjacentClusters  += 1;
                }
                else
                {
                    USHORT usCurrBytesToRead = usAdjacentClusters * usBytesPerCluster;
                    USHORT usSectorsToRead = usAdjacentClusters * usSectorsPerCluster;

                    ulClusterSector = pVolInfo->ulStartOfData + (pOpenInfo->ulCurCluster-2)*usSectorsPerCluster;
#if 0
                    /*
                        The following code is fast, but is not compatible
                        with OBJ_ANY attribute
                    */
                    rc = ReadSector(pVolInfo, ulClusterSector,usSectorsToRead,pBufPosition, usIOFlag);
                    if (rc)
                    {
                        goto FS_READEXIT;
                    }
                    pBufPosition                    += usCurrBytesToRead;
                    psffsi->sfi_position            += usCurrBytesToRead;
                    usBytesRead                     += usCurrBytesToRead;
                    usBytesToRead                   -= usCurrBytesToRead;
                    usAdjacentClusters              = 1;
                    pOpenInfo->ulCurCluster         = ulNextCluster;
#else
                    while( usAdjacentClusters )
                    {
                        rc = ReadSector(pVolInfo, ulClusterSector,usSectorsPerCluster,pbCluster, usIOFlag);
                        if (rc)
                        {
                            goto FS_READEXIT;
                        }

                        memcpy( pBufPosition, pbCluster, usBytesPerCluster );

                        pBufPosition                += usBytesPerCluster;
                        ulClusterSector             += usSectorsPerCluster;
                        usAdjacentClusters--;
                    }

                    psffsi->sfi_position            += usCurrBytesToRead;
                    usBytesRead                     += usCurrBytesToRead;
                    usBytesToRead                   -= usCurrBytesToRead;
                    usAdjacentClusters              = 1;
                    pOpenInfo->ulCurCluster         = ulNextCluster;

#endif
                }
                ulCurrCluster       = ulNextCluster;
            }
        }

        /*
            Third, handle the part that aligns on a cluster border but does not make up a complete cluster
        */
        if
            (
                (pOpenInfo->ulCurCluster != FAT_EOF) &&
                (psffsi->sfi_position < psffsi->sfi_size) &&
                (usBytesToRead)
            )
        {
            USHORT usCurrBytesToRead;
            USHORT usSectorsToRead;
            USHORT usSectorsPerCluster;

            usSectorsToRead = pVolInfo->BootSect.bpb.SectorsPerCluster;
            usSectorsPerCluster = usSectorsToRead;

            usCurrBytesToRead = (USHORT)min((ULONG)usBytesToRead,psffsi->sfi_size - psffsi->sfi_position);
            if (usCurrBytesToRead)
            {
                ulClusterSector = pVolInfo->ulStartOfData + (pOpenInfo->ulCurCluster-2)*usSectorsPerCluster;
                rc = ReadSector(pVolInfo, ulClusterSector,usSectorsToRead,pbCluster, usIOFlag);
                if (rc)
                {
                    goto FS_READEXIT;
                }
                memcpy(pBufPosition,pbCluster,usCurrBytesToRead);

                psffsi->sfi_position    += usCurrBytesToRead;
                usBytesRead             += usCurrBytesToRead;
                usBytesToRead           -= usCurrBytesToRead;
            }
        }

        *pLen = usBytesRead;
        psffsi->sfi_tstamp  |= ST_SREAD | ST_PREAD;
        rc = NO_ERROR;
    }

FS_READEXIT:

    if( pbCluster )
        free( pbCluster );

    if (f32Parms.fMessageActive & LOG_FS)
        Message("FS_READ returned %u (%u bytes read)", rc, *pLen);
    return rc;
}

/******************************************************************
*
******************************************************************/
int far pascal __loadds  FS_WRITE(
    struct sffsi far * psffsi,      /* psffsi   */
    struct sffsd far * psffsd,      /* psffsd   */
    char far * pData,           /* pData    */
    unsigned short far * pLen,  /* pLen     */
    unsigned short usIOFlag     /* IOflag   */
)
{
USHORT rc;
PVOLINFO pVolInfo;
POPENINFO pOpenInfo = GetOpenInfo(psffsd);
USHORT usBytesWritten;
USHORT usBytesToWrite;
PBYTE  pbCluster;
ULONG  ulClusterSector;
USHORT usClusterOffset;
USHORT usBytesPerCluster;

   usBytesToWrite = *pLen;
   usBytesWritten = 0;
   *pLen = 0;
   pbCluster = NULL;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_WRITE, %u bytes at offset %ld, ioflag %X, size = %lu",
      usBytesToWrite, psffsi->sfi_position, usIOFlag, psffsi->sfi_size);

   pVolInfo = GetVolInfo(psffsi->sfi_hVPB);
   if (IsDriveLocked(pVolInfo))
   {
      rc = ERROR_DRIVE_LOCKED;
      goto FS_WRITEEXIT;
   }


   if (pVolInfo->fWriteProtected)
   {
      rc = ERROR_WRITE_PROTECT;
      goto FS_WRITEEXIT;
   }

   if (!((psffsi->sfi_mode & 0xFUL) == OPEN_ACCESS_WRITEONLY) &&
       !((psffsi->sfi_mode & 0xFUL) == OPEN_ACCESS_READWRITE))
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_WRITEEXIT;
      }

   if (!usBytesToWrite)
      {
      rc = NO_ERROR;
      goto FS_WRITEEXIT;
      }

   pbCluster = malloc(pVolInfo->usClusterSize);
   if (!pbCluster)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      goto FS_WRITEEXIT;
      }

    if (psffsi->sfi_mode & OPEN_FLAGS_DASD)
    {
        ULONG   ulSector;
        USHORT  usSectorOffset;
        USHORT  usRemaining;
        USHORT  usBytesPerSector;
        USHORT  usNumSectors;

        char far *pBufPosition = pData;

        usBytesPerSector    = pVolInfo->BootSect.bpb.BytesPerSector;
        usBytesWritten      = 0;

        if (pOpenInfo->fSectorMode)
        {
            USHORT usTotNumBytes;

            if (usBytesToWrite > (USHRT_MAX/usBytesPerSector))
            {
                rc = ERROR_TRANSFER_TOO_LONG;
                goto FS_WRITEEXIT;
            }

            usTotNumBytes = usBytesToWrite * usBytesPerSector;

            rc = MY_PROBEBUF(PB_OPWRITE, pBufPosition, usTotNumBytes);
            if (rc)
            {
                Message("Protection VIOLATION in FS_READ! (SYS%d)", rc);
                goto FS_WRITEEXIT;
            }

            /*
                for sector mode, offsets are actually sectors
                and Number of Bytes to write are actually
                Number of sectors to write
            */
            ulSector        = psffsi->sfi_position;
            usSectorOffset  = 0;
            usRemaining     = 0;
            usNumSectors    = usBytesToWrite;
            if (ulSector > (pVolInfo->BootSect.bpb.BigTotalSectors - (ULONG)usNumSectors))
            {
                usNumSectors    = (USHORT)(pVolInfo->BootSect.bpb.BigTotalSectors - ulSector);
            }
            usBytesToWrite   = usNumSectors;
        }
        else
        {
            rc = MY_PROBEBUF(PB_OPWRITE, pData, usBytesToWrite);
            if (rc)
            {
                Message("Protection VIOLATION in FS_READ! (SYS%d)", rc);
                goto FS_WRITEEXIT;
            }

            ulSector        = (ULONG)(psffsi->sfi_position / (ULONG)usBytesPerSector);
            usSectorOffset  = (USHORT)(psffsi->sfi_position % (ULONG)usBytesPerSector);
            usRemaining     = (usSectorOffset + usBytesToWrite) % usBytesPerSector;
            usNumSectors    = (usBytesToWrite - usRemaining)/usBytesPerSector;

            if (ulSector > (pVolInfo->BootSect.bpb.BigTotalSectors - (ULONG)usNumSectors - (ULONG)(usRemaining ? 1 : 0)))
            {
                usNumSectors    = (USHORT)(pVolInfo->BootSect.bpb.BigTotalSectors - ulSector);
                usRemaining     = 0;
            }
            usBytesToWrite   = usNumSectors*usBytesPerSector;
        }

        if (ulSector >= pVolInfo->BootSect.bpb.BigTotalSectors)
        {
            rc = ERROR_SECTOR_NOT_FOUND;
            goto FS_WRITEEXIT;
        }

        if (ulSector > (pVolInfo->BootSect.bpb.BigTotalSectors - (ULONG)usNumSectors - (ULONG)(usRemaining ? 1 : 0)))
        {
            rc = ERROR_SECTOR_NOT_FOUND;
            goto FS_WRITEEXIT;
        }

        if (usSectorOffset)
        {
            rc = ReadSector(pVolInfo, ulSector, 1, pbCluster, usIOFlag);
            if (rc)
            {
                goto FS_WRITEEXIT;
            }
            memcpy(pbCluster + usSectorOffset, pBufPosition, usBytesPerSector-usSectorOffset);
            rc = WriteSector(pVolInfo, ulSector, 1, pbCluster, usIOFlag);
            if (rc)
            {
                goto FS_WRITEEXIT;
            }

            pBufPosition            += (usBytesPerSector - usSectorOffset);
            psffsi->sfi_position    += (usBytesPerSector - usSectorOffset);
            usBytesWritten          += (usBytesPerSector - usSectorOffset);
        }

        if (usNumSectors)
        {
            rc = WriteSector(pVolInfo, ulSector, usNumSectors, pBufPosition, usIOFlag);
            if (rc)
            {
                goto FS_WRITEEXIT;
            }
            pBufPosition            += (ULONG)((ULONG)usNumSectors*(ULONG)usBytesPerSector);
            psffsi->sfi_position    += usBytesToWrite;
            usBytesWritten          += usBytesToWrite;
        }

        if (usRemaining)
        {
            rc = ReadSector(pVolInfo, ulSector, 1, pbCluster, usIOFlag);
            if (rc)
            {
                goto FS_WRITEEXIT;
            }
            memcpy(pbCluster, pBufPosition, usRemaining);
            rc = WriteSector(pVolInfo, ulSector, 1, pbCluster, usIOFlag);
            if (rc)
            {
                goto FS_WRITEEXIT;
            }

            pBufPosition            += usRemaining;
            psffsi->sfi_position    += usRemaining;
            usBytesWritten          += usRemaining;
        }

        *pLen                       = usBytesWritten;
        rc = NO_ERROR;
    }
    else
    {
        char far *pBufPosition = pData;

        /*
            No writes if file is larger than 7FFFFFFF (= 2Gb)
        */
        if (psffsi->sfi_position >= LONG_MAX)
        {
            rc = NO_ERROR;
            goto FS_WRITEEXIT;
        }

        rc = MY_PROBEBUF(PB_OPREAD, pData, usBytesToWrite);
        if (rc)
        {
            Message("Protection VIOLATION in FS_WRITE! (SYS%d)", rc);
            goto FS_WRITEEXIT;
        }

        pOpenInfo->pSHInfo->fMustCommit = TRUE;


        if ((ULONG)LONG_MAX - psffsi->sfi_position < (ULONG)usBytesToWrite)
            usBytesToWrite = (USHORT)((ULONG)LONG_MAX - psffsi->sfi_position);

        if (psffsi->sfi_position + usBytesToWrite > psffsi->sfi_size)
        {
            ULONG ulLast = FAT_EOF;

            if (
                    pOpenInfo->ulCurCluster == FAT_EOF &&
                    psffsi->sfi_position == psffsi->sfi_size &&
                    !(psffsi->sfi_size % pVolInfo->usClusterSize)
                )
                ulLast = pOpenInfo->pSHInfo->ulLastCluster;

            rc = NewSize(pVolInfo, psffsi, psffsd,
            psffsi->sfi_position + usBytesToWrite, usIOFlag);
            if (rc)
                goto FS_WRITEEXIT;

            if (ulLast != FAT_EOF)
            {
                pOpenInfo->ulCurCluster = GetNextCluster(pVolInfo, ulLast);
                if (!pOpenInfo->ulCurCluster)
                    pOpenInfo->ulCurCluster = FAT_EOF;

                if (pOpenInfo->ulCurCluster == FAT_EOF)
                {
                    Message("FS_WRITE (INIT) No next cluster available!");
                    CritMessage("FAT32: FS_WRITE (INIT) No next cluster available!");
                }
            }
        }

        if (pOpenInfo->ulCurCluster == FAT_EOF)
            pOpenInfo->ulCurCluster = PositionToOffset(pVolInfo, pOpenInfo, psffsi->sfi_position);

        if (pOpenInfo->ulCurCluster == FAT_EOF)
        {
            Message("FS_WRITE (INIT2) No next cluster available!");
            CritMessage("FAT32: FS_WRITE (INIT2) No next cluster available!");
            rc = ERROR_INVALID_HANDLE;
            goto FS_WRITEEXIT;
        }

        /*
            First, handle the first part that does not align on a cluster border
        */
        usBytesPerCluster = pVolInfo->usClusterSize;
        usClusterOffset     = (USHORT)(psffsi->sfi_position % usBytesPerCluster);
        if
            (
                (pOpenInfo->ulCurCluster != FAT_EOF) &&
                (psffsi->sfi_position < psffsi->sfi_size) &&
                (usBytesToWrite) &&
                (usClusterOffset)
            )
        {
            USHORT  usCurrBytesToWrite;
            USHORT  usSectorsToWrite;
            USHORT  usSectorsPerCluster;

            usSectorsPerCluster = pVolInfo->BootSect.bpb.SectorsPerCluster;
            usSectorsToWrite    = usSectorsPerCluster;

            /* compute the number of bytes
                to the cluster end or
                as much as fits into the user buffer
                or how much remains to be read until file end
               whatever is the smallest
            */
            usCurrBytesToWrite  = (usBytesPerCluster - usClusterOffset);
            usCurrBytesToWrite  = min(usCurrBytesToWrite,usBytesToWrite);
            usCurrBytesToWrite  = (USHORT)min((ULONG)usCurrBytesToWrite,psffsi->sfi_size-psffsi->sfi_position);
            if (usCurrBytesToWrite)
            {
                ulClusterSector = pVolInfo->ulStartOfData + (pOpenInfo->ulCurCluster-2)*usSectorsPerCluster;

                rc = ReadSector(pVolInfo, ulClusterSector,usSectorsToWrite,pbCluster, usIOFlag);
                if (rc)
                {
                    goto FS_WRITEEXIT;
                }
                memcpy(pbCluster + usClusterOffset, pBufPosition, usCurrBytesToWrite);

                rc = WriteSector(pVolInfo, ulClusterSector,usSectorsToWrite,pbCluster, usIOFlag);
                if (rc)
                {
                    goto FS_WRITEEXIT;
                }

                pBufPosition            += usCurrBytesToWrite;
                psffsi->sfi_position    += usCurrBytesToWrite;
                usBytesWritten          += usCurrBytesToWrite;
                usBytesToWrite          -= usCurrBytesToWrite;
            }


            if ((usClusterOffset + usCurrBytesToWrite) >= usBytesPerCluster)
            {
                pOpenInfo->ulCurCluster     = GetNextCluster(pVolInfo, pOpenInfo->ulCurCluster);
                if (!pOpenInfo->ulCurCluster)
                {
                    pOpenInfo->ulCurCluster = FAT_EOF;
                }
            }
        }

        /*
            Second, handle the part that aligns on a cluster border and is a multiple of the cluster size
        */
        if
            (
                (pOpenInfo->ulCurCluster != FAT_EOF) &&
                (psffsi->sfi_position < psffsi->sfi_size) &&
                (usBytesToWrite)
            )
        {
            ULONG   ulCurrCluster       = pOpenInfo->ulCurCluster;
            ULONG   ulNextCluster       = ulCurrCluster;
            USHORT  usCurrBytesToWrite  = 0;
            USHORT  usSectorsPerCluster = pVolInfo->BootSect.bpb.SectorsPerCluster;
            USHORT  usClustersToProcess = 0;
            USHORT  usAdjacentClusters  = 1;

            usCurrBytesToWrite          = (USHORT)min((ULONG)usBytesToWrite,psffsi->sfi_size - psffsi->sfi_position);

            usClustersToProcess         = usCurrBytesToWrite / usBytesPerCluster; /* get the number of full clusters */

            while (usClustersToProcess && (ulCurrCluster != FAT_EOF))
            {
                ulNextCluster       = GetNextCluster(pVolInfo, ulCurrCluster);
                if (!ulNextCluster)
                {
                    ulNextCluster = FAT_EOF;
                }

                usClustersToProcess -= 1;

                if  (
                        (ulNextCluster != FAT_EOF) &&
                        (ulNextCluster == (ulCurrCluster+1)) &&
                        (usClustersToProcess) &&
                        (usBytesToWrite >= (usAdjacentClusters+1) * usBytesPerCluster )
                    )
                {
                    usAdjacentClusters  += 1;
                }
                else
                {
                    USHORT usCurrBytesToWrite = usAdjacentClusters * usBytesPerCluster;
                    USHORT usSectorsToWrite = usAdjacentClusters * usSectorsPerCluster;

                    ulClusterSector = pVolInfo->ulStartOfData + (pOpenInfo->ulCurCluster-2)*usSectorsPerCluster;
#if 0
                    /*
                        The following code is fast, but is not compatible
                        with OBJ_ANY attribute
                    */
                    rc = WriteSector(pVolInfo, ulClusterSector,usSectorsToWrite,pBufPosition, usIOFlag);
                    if (rc)
                    {
                        goto FS_WRITEEXIT;
                    }

                    pBufPosition                    += usCurrBytesToWrite;
                    psffsi->sfi_position            += usCurrBytesToWrite;
                    usBytesWritten                  += usCurrBytesToWrite;
                    usBytesToWrite                  -= usCurrBytesToWrite;
                    usAdjacentClusters              = 1;
                    pOpenInfo->ulCurCluster         = ulNextCluster;
#else
                    while( usAdjacentClusters )
                    {
                        memcpy(pbCluster,pBufPosition,usBytesPerCluster);

                        rc = WriteSector(pVolInfo,ulClusterSector,usSectorsPerCluster,pbCluster,usIOFlag);
                        if (rc)
                        {
                            goto FS_WRITEEXIT;
                        }

                        pBufPosition                += usBytesPerCluster;
                        ulClusterSector             += usSectorsPerCluster;
                        usAdjacentClusters--;
                    }

                    psffsi->sfi_position            += usCurrBytesToWrite;
                    usBytesWritten                  += usCurrBytesToWrite;
                    usBytesToWrite                  -= usCurrBytesToWrite;
                    usAdjacentClusters              = 1;
                    pOpenInfo->ulCurCluster         = ulNextCluster;
#endif
                }
                ulCurrCluster       = ulNextCluster;
            }
        }


        /*
            Third, handle the part that aligns on a cluster border but does not make up a complete cluster
        */
        if
            (
                (pOpenInfo->ulCurCluster != FAT_EOF) &&
                (psffsi->sfi_position < psffsi->sfi_size) &&
                (usBytesToWrite)
            )
        {
            USHORT usCurrBytesToWrite;
            USHORT usSectorsToWrite;
            USHORT usSectorsPerCluster;

            usSectorsToWrite = pVolInfo->BootSect.bpb.SectorsPerCluster;
            usSectorsPerCluster = usSectorsToWrite;

            usCurrBytesToWrite = (USHORT)min((ULONG)usBytesToWrite,psffsi->sfi_size - psffsi->sfi_position);
            if (usCurrBytesToWrite)
            {
                ulClusterSector = pVolInfo->ulStartOfData + (pOpenInfo->ulCurCluster-2)*usSectorsPerCluster;
                rc = ReadSector(pVolInfo, ulClusterSector,usSectorsToWrite,pbCluster, usIOFlag);
                if (rc)
                {
                    goto FS_WRITEEXIT;
                }
                memcpy(pbCluster, pBufPosition, usCurrBytesToWrite);

                rc = WriteSector(pVolInfo, ulClusterSector,usSectorsToWrite,pbCluster, usIOFlag);
                if (rc)
                {
                    goto FS_WRITEEXIT;
                }

                psffsi->sfi_position    += usCurrBytesToWrite;
                usBytesWritten          += usCurrBytesToWrite;
                usBytesToWrite          -= usCurrBytesToWrite;
            }
        }

        *pLen = usBytesWritten;
        if (usBytesWritten)
        {
            psffsi->sfi_tstamp |= ST_SWRITE | ST_PWRITE;
            pOpenInfo->fCommitAttr = TRUE;
            pOpenInfo->pSHInfo->bAttr |= FILE_ARCHIVED;
            psffsi->sfi_DOSattr |= FILE_ARCHIVED;
        }


        if (usIOFlag & DVIO_OPWRTHRU)
            rc = FS_COMMIT(FS_COMMIT_ONE, usIOFlag, psffsi, psffsd);
        else
            rc = NO_ERROR;
    }

FS_WRITEEXIT:
   if( pbCluster )
      free( pbCluster );

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_WRITE returned %u (%u bytes written)", rc, *pLen);
   return rc;
}

/******************************************************************
* Positition to offset
******************************************************************/
ULONG PositionToOffset(PVOLINFO pVolInfo, POPENINFO pOpenInfo, ULONG ulOffset)
{
ULONG ulCurCluster;


   ulCurCluster = pOpenInfo->pSHInfo->ulStartCluster;
   if (!ulCurCluster)
      return FAT_EOF;

   if (ulOffset < pVolInfo->usClusterSize)
      return ulCurCluster;

   return SeekToCluster(pVolInfo, ulCurCluster, ulOffset);
}

/******************************************************************
*
******************************************************************/
int far pascal __loadds  FS_CANCELLOCKREQUEST(
    struct sffsi far * psffsi,      /* psffsi   */
    struct sffsd far * psffsd,      /* psffsd   */
    void far * pLockRang            /* pLockRang    */
)
{
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_CANCELLOCKREQUEST - NOT SUPPORTED");
   return ERROR_NOT_SUPPORTED;

   psffsi = psffsi;
   psffsd = psffsd;
   pLockRang = pLockRang;
}


/******************************************************************
*
******************************************************************/
int far pascal __loadds  FS_CHGFILEPTR(
    struct sffsi far * psffsi,      /* psffsi   */
    struct sffsd far * psffsd,      /* psffsd   */
    long lOffset,           /* offset   */
    unsigned short usType,      /* type     */
    unsigned short IOFlag       /* IOflag   */
)
{
PVOLINFO pVolInfo;
POPENINFO pOpenInfo = GetOpenInfo(psffsd);
LONG  lNewOffset = 0;
USHORT rc;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_CHGFILEPTR, Mode %d - offset %ld, current offset=%lu",
      usType, lOffset, psffsi->sfi_position);

   pVolInfo = GetVolInfo(psffsi->sfi_hVPB);
   if (IsDriveLocked(pVolInfo))
      return ERROR_DRIVE_LOCKED;

   switch (usType)
      {
      case CFP_RELBEGIN :
         if (lOffset < 0)
            {
            rc = ERROR_NEGATIVE_SEEK;
            goto FS_CHGFILEPTREXIT;
            }
         lNewOffset = lOffset;
         break;
      case CFP_RELCUR  :
         lNewOffset = psffsi->sfi_position + lOffset;
         break;
      case CFP_RELEND   :
         lNewOffset = psffsi->sfi_size + lOffset;
         break;
      }
   if (!IsDosSession() && lNewOffset < 0)
      {
      rc = ERROR_NEGATIVE_SEEK;
      goto FS_CHGFILEPTREXIT;
      }

   if (psffsi->sfi_position != (ULONG)lNewOffset)
      {
      psffsi->sfi_position = (ULONG)lNewOffset;
      pOpenInfo->ulCurCluster = FAT_EOF;
      }
   rc = 0;

FS_CHGFILEPTREXIT:
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_CHGFILEPTR returned %u", rc);
   return rc;

   IOFlag = IOFlag;
}


/******************************************************************
*
******************************************************************/
int far pascal __loadds  FS_COMMIT(
    unsigned short usType,      /* commit type  */
    unsigned short usIOFlag,        /* IOflag   */
    struct sffsi far * psffsi,      /* psffsi   */
    struct sffsd far * psffsd       /* psffsd   */
)
{
USHORT rc;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_COMMIT, type %d", usType);

   switch (usType)
      {
      case FS_COMMIT_ONE:
         {
         PVOLINFO pVolInfo = GetVolInfo(psffsi->sfi_hVPB);
         POPENINFO pOpenInfo = GetOpenInfo(psffsd);
         PSZ  pszFile;
         DIRENTRY DirEntry;
         DIRENTRY DirOld;
         ULONG  ulCluster;

         if (IsDriveLocked(pVolInfo))
            return ERROR_DRIVE_LOCKED;

         if (psffsi->sfi_mode & OPEN_FLAGS_DASD)
            return ERROR_NOT_SUPPORTED;

         if (!pOpenInfo->pSHInfo->fMustCommit)
            {
            rc = 0;
            goto FS_COMMITEXIT;
            }

         pszFile = strrchr(pOpenInfo->pSHInfo->szFileName, '\\');
         if (!pszFile)
            {
            Message("FS_COMMIT, cannot find \\ in '%s'", pOpenInfo->pSHInfo->szFileName);
            CritMessage("FAT32:FS_COMMIT, cannot find \\ in '%s'!", pOpenInfo->pSHInfo->szFileName);
            return 1;
            }
         pszFile++;

         ulCluster = FindPathCluster(pVolInfo, pOpenInfo->pSHInfo->ulDirCluster, pszFile, &DirOld, NULL);
         if (ulCluster == FAT_EOF)
            {
            rc = ERROR_FILE_NOT_FOUND;
            goto FS_COMMITEXIT;
            }

         memcpy(&DirEntry, &DirOld, sizeof (DIRENTRY));
         memcpy(&DirEntry.wCreateTime,    &psffsi->sfi_ctime, sizeof (USHORT));
         memcpy(&DirEntry.wCreateDate,    &psffsi->sfi_cdate, sizeof (USHORT));
         memcpy(&DirEntry.wAccessDate,    &psffsi->sfi_adate, sizeof (USHORT));
         memcpy(&DirEntry.wLastWriteTime, &psffsi->sfi_mtime, sizeof (USHORT));
         memcpy(&DirEntry.wLastWriteDate, &psffsi->sfi_mdate, sizeof (USHORT));

         DirEntry.ulFileSize  = psffsi->sfi_size;
         if (pOpenInfo->fCommitAttr || psffsi->sfi_DOSattr != pOpenInfo->pSHInfo->bAttr)
            {
            DirEntry.bAttr = pOpenInfo->pSHInfo->bAttr = psffsi->sfi_DOSattr;
            pOpenInfo->fCommitAttr = FALSE;
            }

         if (pOpenInfo->pSHInfo->ulStartCluster)
            {
            DirEntry.wCluster     = LOUSHORT(pOpenInfo->pSHInfo->ulStartCluster);
            DirEntry.wClusterHigh = HIUSHORT(pOpenInfo->pSHInfo->ulStartCluster);
            }
         else
            {
            DirEntry.wCluster = 0;
            DirEntry.wClusterHigh = 0;
            }

         rc = ModifyDirectory(pVolInfo, pOpenInfo->pSHInfo->ulDirCluster, MODIFY_DIR_UPDATE,
            &DirOld, &DirEntry, NULL, usIOFlag);
         if (!rc)
            psffsi->sfi_tstamp = 0;
         goto FS_COMMITEXIT;
         }
      }
   rc = ERROR_NOT_SUPPORTED;

FS_COMMITEXIT:

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_COMMIT returned %u", rc);
   return rc;
}

/******************************************************************
*
******************************************************************/
int far pascal __loadds  FS_FILELOCKS(
    struct sffsi far * psffsi,      /* psffsi   */
    struct sffsd far * psffsd,      /* psffsd   */
    void far * pUnlockRange,            /* pUnLockRange */
    void far * pLockRange,          /* pLockRange   */
    unsigned long ulTimeOut,        /* timeout  */
    unsigned long   ulFlags /* flags    */
)
{
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FILELOCKS");
   return ERROR_NOT_SUPPORTED;

   psffsi = psffsi;
   psffsd = psffsd;
   pUnlockRange = pUnlockRange;
   pLockRange = pLockRange;
   ulTimeOut = ulTimeOut;
   ulFlags = ulFlags;
}

/******************************************************************
*
******************************************************************/
int far pascal __loadds  FS_NEWSIZE(
    struct sffsi far * psffsi,      /* psffsi   */
    struct sffsd far * psffsd,      /* psffsd   */
    unsigned long ulLen,        /* len      */
    unsigned short usIOFlag     /* IOflag   */
)
{
PVOLINFO pVolInfo;
POPENINFO pOpenInfo = GetOpenInfo(psffsd);
USHORT rc;

   pOpenInfo->pSHInfo->fMustCommit = TRUE;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_NEWSIZE newsize = %lu", ulLen);

   if (psffsi->sfi_mode & OPEN_FLAGS_DASD)
      return ERROR_NOT_SUPPORTED;

   pVolInfo = GetVolInfo(psffsi->sfi_hVPB);
   if (IsDriveLocked(pVolInfo))
      return ERROR_DRIVE_LOCKED;
   if (pVolInfo->fWriteProtected)
      return ERROR_WRITE_PROTECT;

   rc = NewSize(pVolInfo, psffsi, psffsd, ulLen, usIOFlag);
   if (!rc)
      psffsi->sfi_tstamp |= ST_SWRITE | ST_PWRITE;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_NEWSIZE returned %u", rc);

   return rc;
}

/******************************************************************
*
******************************************************************/
USHORT NewSize(PVOLINFO pVolInfo,
    struct sffsi far * psffsi,      /* psffsi   */
    struct sffsd far * psffsd,      /* psffsd   */
    ULONG ulLen,
    USHORT usIOFlag)
{
POPENINFO pOpenInfo = GetOpenInfo(psffsd);
ULONG ulClustersNeeded;
ULONG ulClusterCount;
ULONG ulCluster, ulNextCluster;


   if (ulLen == psffsi->sfi_size)
      return 0;

   if (!ulLen)
      {
      if (pOpenInfo->pSHInfo->ulStartCluster)
         {
         DeleteFatChain(pVolInfo, pOpenInfo->pSHInfo->ulStartCluster);
         pOpenInfo->pSHInfo->ulStartCluster = 0L;
         pOpenInfo->pSHInfo->ulLastCluster = FAT_EOF;
         }

      ResetAllCurrents(pOpenInfo);
      psffsi->sfi_size = ulLen;
      if (usIOFlag & DVIO_OPWRTHRU)
         return FS_COMMIT(FS_COMMIT_ONE, usIOFlag, psffsi, psffsd);

      return 0;
      }

   /*
      Calculate number of needed clusters
   */
   ulClustersNeeded = ulLen / pVolInfo->usClusterSize;
   if (ulLen % pVolInfo->usClusterSize)
      ulClustersNeeded ++;

   /*
      if file didn't have any clusters
   */

   if (!pOpenInfo->pSHInfo->ulStartCluster)
      {
      ulCluster = MakeFatChain(pVolInfo, FAT_EOF, ulClustersNeeded, &pOpenInfo->pSHInfo->ulLastCluster);
      if (ulCluster == FAT_EOF)
         return ERROR_DISK_FULL;
      pOpenInfo->pSHInfo->ulStartCluster = ulCluster;
      }

   /*
      If newsize < current size
   */

   else if (ulLen < psffsi->sfi_size)
      {
      if (!(ulLen % pVolInfo->usClusterSize))
         ulCluster = PositionToOffset(pVolInfo, pOpenInfo, ulLen - 1);
      else
         ulCluster = PositionToOffset(pVolInfo, pOpenInfo, ulLen);

      if (ulCluster == FAT_EOF)
         return ERROR_SECTOR_NOT_FOUND;

      ulNextCluster = GetNextCluster(pVolInfo, ulCluster);
      if (ulNextCluster != FAT_EOF)
         {
         SetNextCluster( pVolInfo, ulCluster, FAT_EOF);
         DeleteFatChain(pVolInfo, ulNextCluster);
         }
      pOpenInfo->pSHInfo->ulLastCluster = ulCluster;

      ResetAllCurrents(pOpenInfo);
      }
   else
      {
      /*
         If newsize > current size
      */

      ulCluster = pOpenInfo->pSHInfo->ulLastCluster;
      if (ulCluster == FAT_EOF)
         {
         CritMessage("FAT32: Lastcluster empty in NewSize!");
         Message("FAT32: Lastcluster empty in NewSize!");
         return ERROR_SECTOR_NOT_FOUND;
         }

      ulClusterCount = psffsi->sfi_size / pVolInfo->usClusterSize;
      if (psffsi->sfi_size % pVolInfo->usClusterSize)
         ulClusterCount ++;

      if (ulClustersNeeded > ulClusterCount)
         {
         ulNextCluster = MakeFatChain(pVolInfo, ulCluster, ulClustersNeeded - ulClusterCount, &pOpenInfo->pSHInfo->ulLastCluster);
         if (ulNextCluster == FAT_EOF)
            return ERROR_DISK_FULL;
         }
      }

   psffsi->sfi_size = ulLen;

   if (usIOFlag & DVIO_OPWRTHRU)
      return FS_COMMIT(FS_COMMIT_ONE, usIOFlag, psffsi, psffsd);

   return 0;
}

/******************************************************************
*
******************************************************************/
int far pascal __loadds  FS_FILEINFO(unsigned short usFlag,       /* flag     */
    struct sffsi far * psffsi,      /* psffsi   */
    struct sffsd far * psffsd,      /* psffsd   */
    unsigned short usLevel,     /* level    */
    char far * pData,           /* pData    */
    unsigned short cbData,      /* cbData   */
    unsigned short IOFlag       /* IOflag   */
)
{
PVOLINFO pVolInfo;
POPENINFO pOpenInfo = GetOpenInfo(psffsd);
USHORT usNeededSize;
USHORT rc;
PSZ  pszFile;



   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FILEINFO for %s, usFlag = %X, level %d",
         pOpenInfo->pSHInfo->szFileName,
         usFlag, usLevel);

   pVolInfo = GetVolInfo(psffsi->sfi_hVPB);
   if (psffsi->sfi_mode & OPEN_FLAGS_DASD)
      return ERROR_NOT_SUPPORTED;
   if (IsDriveLocked(pVolInfo))
      return ERROR_DRIVE_LOCKED;

   pszFile = strrchr(pOpenInfo->pSHInfo->szFileName, '\\');
   if (!pszFile)
      {
      Message("FS_FILEINFO, cannot find \\!");
      CritMessage("FAT32:FS_FILEINFO, cannot find \\!");
      return 1;
      }
   pszFile++;


   psffsd = psffsd;
   IOFlag = IOFlag;

   if (usFlag == FI_RETRIEVE)
      {
      switch (usLevel)
         {
         case FIL_STANDARD         :
            usNeededSize = sizeof (FILESTATUS);
            break;
         case FIL_QUERYEASIZE      :
            usNeededSize = sizeof (FILESTATUS2);
            break;
         case FIL_QUERYEASFROMLIST :
         case 4:
            usNeededSize = sizeof (EAOP);
            break;
         default                   :
            rc = ERROR_INVALID_LEVEL;
            goto FS_FILEINFOEXIT;
         }
      if (cbData < usNeededSize)
         {
         rc = ERROR_BUFFER_OVERFLOW;
         goto FS_FILEINFOEXIT;
         }

      rc = MY_PROBEBUF(PB_OPWRITE, pData, cbData);
      if (rc)
         {
         Message("Protection VIOLATION in FS_FILEINFO!\n");
         return rc;
         }

      if (usLevel == FIL_STANDARD || usLevel == FIL_QUERYEASIZE)
         {
         DIRENTRY DirEntry;
         ULONG ulCluster;

         ulCluster = FindPathCluster(pVolInfo, pOpenInfo->pSHInfo->ulDirCluster, pszFile, &DirEntry, NULL);
         if (ulCluster == FAT_EOF)
            {
            rc = ERROR_FILE_NOT_FOUND;
            goto FS_FILEINFOEXIT;
            }

         memcpy(&psffsi->sfi_ctime, &DirEntry.wCreateTime, sizeof (USHORT));
         memcpy(&psffsi->sfi_cdate, &DirEntry.wCreateDate, sizeof (USHORT));
         psffsi->sfi_atime = 0;
         memcpy(&psffsi->sfi_adate, &DirEntry.wAccessDate, sizeof (USHORT));
         memcpy(&psffsi->sfi_mtime, &DirEntry.wLastWriteTime, sizeof (USHORT));
         memcpy(&psffsi->sfi_mdate, &DirEntry.wLastWriteDate, sizeof (USHORT));
         psffsi->sfi_DOSattr = DirEntry.bAttr = pOpenInfo->pSHInfo->bAttr;
         }

      switch (usLevel)
         {
         case FIL_STANDARD         :
            {
            PFILESTATUS pfStatus = (PFILESTATUS)pData;
            memset(pfStatus, 0, sizeof (FILESTATUS));

            memcpy(&pfStatus->fdateCreation, &psffsi->sfi_cdate, sizeof (USHORT));
            memcpy(&pfStatus->ftimeCreation, &psffsi->sfi_ctime, sizeof (USHORT));
            memcpy(&pfStatus->fdateLastAccess, &psffsi->sfi_adate, sizeof (USHORT));
            memcpy(&pfStatus->fdateLastWrite, &psffsi->sfi_mdate, sizeof (USHORT));
            memcpy(&pfStatus->ftimeLastWrite, &psffsi->sfi_mtime, sizeof (USHORT));
            pfStatus->cbFile = psffsi->sfi_size;
            pfStatus->cbFileAlloc =
               (pfStatus->cbFile / pVolInfo->usClusterSize) * pVolInfo->usClusterSize +
               (pfStatus->cbFile % pVolInfo->usClusterSize ? pVolInfo->usClusterSize : 0);

            pfStatus->attrFile = psffsi->sfi_DOSattr = pOpenInfo->pSHInfo->bAttr;
            rc = 0;
            break;
            }
         case FIL_QUERYEASIZE      :
            {
            PFILESTATUS2 pfStatus = (PFILESTATUS2)pData;
            memset(pfStatus, 0, sizeof (FILESTATUS2));

            memcpy(&pfStatus->fdateCreation, &psffsi->sfi_cdate, sizeof (USHORT));
            memcpy(&pfStatus->ftimeCreation, &psffsi->sfi_ctime, sizeof (USHORT));
            memcpy(&pfStatus->fdateLastAccess, &psffsi->sfi_adate, sizeof (USHORT));
            memcpy(&pfStatus->fdateLastWrite, &psffsi->sfi_mdate, sizeof (USHORT));
            memcpy(&pfStatus->ftimeLastWrite, &psffsi->sfi_mtime, sizeof (USHORT));
            pfStatus->cbFile = psffsi->sfi_size;
            pfStatus->cbFileAlloc =
               (pfStatus->cbFile / pVolInfo->usClusterSize) * pVolInfo->usClusterSize +
               (pfStatus->cbFile % pVolInfo->usClusterSize ? pVolInfo->usClusterSize : 0);

            pfStatus->attrFile = psffsi->sfi_DOSattr = pOpenInfo->pSHInfo->bAttr;

            if (!f32Parms.fEAS)
               {
               pfStatus->cbList = sizeof pfStatus->cbList;
               rc = 0;
               }
            else
               rc = usGetEASize(pVolInfo, pOpenInfo->pSHInfo->ulDirCluster, pszFile, &pfStatus->cbList);
            break;
            }
         case FIL_QUERYEASFROMLIST:
            {
            PEAOP pEA = (PEAOP)pData;
            PFEALIST pFEA = pEA->fpFEAList;
            rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pFEA, sizeof pFEA->cbList);
            if (rc)
               {
               Message("FAT32: Protection VIOLATION in FS_FILEINFO!\n");
               return rc;
               }

            rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pFEA, (USHORT)pFEA->cbList);
            if (rc)
               {
               Message("FAT32: Protection VIOLATION in FS_FILEINFO!\n");
               return rc;
               }

            if (!f32Parms.fEAS)
               {
               rc = usGetEmptyEAS(pszFile,pEA);
               }
            else
               rc = usGetEAS(pVolInfo, usLevel, pOpenInfo->pSHInfo->ulDirCluster, pszFile, pEA);
            break;
            }

         case 4:
            {
            PEAOP pEA = (PEAOP)pData;
            PFEALIST pFEA = pEA->fpFEAList;
            rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pFEA, sizeof pFEA->cbList);
            if (rc)
               {
               Message("FAT32: Protection VIOLATION in FS_FILEINFO!\n");
               return rc;
               }

            rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pFEA, (USHORT)pFEA->cbList);
            if (rc)
               {
               Message("FAT32: Protection VIOLATION in FS_FILEINFO!\n");
               return rc;
               }
            if (!f32Parms.fEAS)
               {
               memset(pFEA, 0, (USHORT)pFEA->cbList);
               pFEA->cbList = sizeof pFEA->cbList;
               rc = 0;
               }
            else
               rc = usGetEAS(pVolInfo, usLevel, pOpenInfo->pSHInfo->ulDirCluster, pszFile, pEA);
            break;
            }
         default :
            rc = ERROR_INVALID_LEVEL;
            break;
         }
      goto FS_FILEINFOEXIT;
      }


   if (usFlag & FI_SET)
      {
      rc = MY_PROBEBUF(PB_OPREAD, pData, cbData);
      if (rc)
         {
         Message("FAT32: Protection VIOLATION in FS_FILEINFO!\n");
         return rc;
         }

      if (!(psffsi->sfi_mode & OPEN_ACCESS_WRITEONLY) &&
         !(psffsi->sfi_mode & OPEN_ACCESS_READWRITE))
         {
         rc = ERROR_ACCESS_DENIED;
         goto FS_FILEINFOEXIT;
         }

      switch (usLevel)
         {
         case FIL_STANDARD:
            {
            USHORT usMask;
            PFILESTATUS pfStatus = (PFILESTATUS)pData;

            if (cbData < sizeof (FILESTATUS))
               {
               rc = ERROR_INSUFFICIENT_BUFFER;
               goto FS_FILEINFOEXIT;
               }

            usMask = ~(FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_ARCHIVED);
            if (pfStatus->attrFile & usMask)
               {
               rc = ERROR_ACCESS_DENIED;
               goto FS_FILEINFOEXIT;
               }

            usMask = 0;
            if (memcmp(&pfStatus->fdateCreation, &usMask, sizeof usMask) ||
                memcmp(&pfStatus->ftimeCreation, &usMask, sizeof usMask))
               {
               psffsi->sfi_tstamp &= ~ST_SCREAT;
               psffsi->sfi_tstamp |= ST_PCREAT;
               memcpy(&psffsi->sfi_ctime, &pfStatus->ftimeCreation, sizeof (USHORT));
               memcpy(&psffsi->sfi_cdate, &pfStatus->fdateCreation, sizeof (USHORT));
               pOpenInfo->pSHInfo->fMustCommit = TRUE;
               }

            if (memcmp(&pfStatus->fdateLastWrite, &usMask, sizeof usMask) ||
                memcmp(&pfStatus->ftimeLastWrite, &usMask, sizeof usMask))
               {
               psffsi->sfi_tstamp &= ~ST_SWRITE;
               psffsi->sfi_tstamp |= ST_PWRITE;
               memcpy(&psffsi->sfi_mdate, &pfStatus->fdateLastWrite, sizeof (USHORT));
               memcpy(&psffsi->sfi_mtime, &pfStatus->ftimeLastWrite, sizeof (USHORT));
               pOpenInfo->pSHInfo->fMustCommit = TRUE;
               }

            if (memcmp(&pfStatus->fdateLastAccess, &usMask, sizeof usMask))
               {
               psffsi->sfi_tstamp &= ~ST_SREAD;
               psffsi->sfi_tstamp |= ST_PREAD;
               memcpy(&psffsi->sfi_adate, &pfStatus->fdateLastAccess, sizeof (USHORT));
               psffsi->sfi_atime = 0;
               pOpenInfo->pSHInfo->fMustCommit = TRUE;
               }
            if (psffsi->sfi_DOSattr       != (BYTE)pfStatus->attrFile ||
                pOpenInfo->pSHInfo->bAttr != (BYTE)pfStatus->attrFile)
               {
               psffsi->sfi_DOSattr = (BYTE)pfStatus->attrFile;
               pOpenInfo->pSHInfo->bAttr = (BYTE)pfStatus->attrFile;
               pOpenInfo->fCommitAttr = TRUE;
               pOpenInfo->pSHInfo->fMustCommit = TRUE;
               }

            if (IOFlag & DVIO_OPWRTHRU)
               rc = FS_COMMIT(FS_COMMIT_ONE, IOFlag, psffsi, psffsd);
            else
               rc = 0;
            break;
            }

         case FIL_QUERYEASIZE      :
            if (!f32Parms.fEAS)
               rc = 0;
            else
               {
               rc = usModifyEAS(pVolInfo, pOpenInfo->pSHInfo->ulDirCluster,
                  pszFile, (PEAOP)pData);
               psffsi->sfi_tstamp = ST_SWRITE | ST_PWRITE;
               }
            break;

         default          :
            rc = ERROR_INVALID_LEVEL;
            break;
         }
      }
   else
      rc = ERROR_INVALID_FUNCTION;

FS_FILEINFOEXIT:
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FILEINFO returned %u", rc);
   return rc;
}

/******************************************************************
*
******************************************************************/
int far pascal __loadds  FS_FILEIO(
    struct sffsi far * psffsi,      /* psffsi   */
    struct sffsd far * psffsd,      /* psffsd   */
    char far * cbCmdList,           /* cbCmdList    */
    unsigned short pCmdLen,     /* pCmdLen  */
    unsigned short far * poError,   /* poError  */
    unsigned short IOFlag       /* IOflag   */
)
{
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FILEIO - NOT SUPPORTED");
   return ERROR_NOT_SUPPORTED;

   psffsi = psffsi;
   psffsd = psffsd;
   cbCmdList = cbCmdList;
   pCmdLen = pCmdLen;
   poError = poError;
   IOFlag = IOFlag;
}


/******************************************************************
*
******************************************************************/
int far pascal __loadds  FS_NMPIPE(
    struct sffsi far * psffsi,      /* psffsi   */
    struct sffsd far * psffsd,      /* psffsd   */
    unsigned short usOpType,        /* OpType   */
    union npoper far * pOpRec,      /* pOpRec   */
    char far * pData,           /* pData    */
    char far *  pName       /* pName    */
)
{
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_NMPIPE - NOT SUPPORTED");
   return ERROR_NOT_SUPPORTED;

   psffsi = psffsi;
   psffsd = psffsd;
   usOpType = usOpType;
   pOpRec = pOpRec;
   pData = pData;
   pName = pName;
}

POPENINFO GetOpenInfo(struct sffsd far * psffsd)
{
POPENINFO pOpenInfo = *(POPENINFO *)psffsd;
USHORT rc;

   rc = MY_PROBEBUF(PB_OPREAD, (PBYTE)pOpenInfo, sizeof (OPENINFO));
   if (rc)
      {
      CritMessage("FAT32: Protection VIOLATION (OpenInfo) in GetOpenInfo! (SYS%d)", rc);
      Message("FAT32: Protection VIOLATION (OpenInfo) in GetOpenInfo!(SYS%d)", rc);
      return NULL;
      }
   return pOpenInfo;
}

USHORT MY_ISCURDIRPREFIX( PSZ pszName )
{
PSHOPENINFO pSH;
int iLength = strlen( pszName );

   pSH = pGlobSH;
   while (pSH)
      {
      if ( !strnicmp(pSH->szFileName, pszName, iLength ) &&
           (( pSH->szFileName[ iLength ] == '\\' ) || ( pSH->szFileName[ iLength ] == '\0' )))
         return ERROR_CURRENT_DIRECTORY;

      pSH = (PSHOPENINFO)pSH->pNext;
      }

   return 0;
}
