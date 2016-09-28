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

/******************************************************************
*
******************************************************************/
int far pascal _loadds FS_FILEATTRIBUTE(
    unsigned short usFlag,      /* flag     */
    struct cdfsi far * pcdfsi,      /* pcdfsi   */
    struct cdfsd far * pcdfsd,      /* pcdfsd   */
    char far * pName,           /* pName    */
    unsigned short usCurDirEnd,     /* iCurDirEnd   */
    unsigned short far * pAttr  /* pAttr    */
)
{
PVOLINFO pVolInfo;
ULONG ulCluster;
ULONG ulDirCluster;
PSZ   pszFile;
DIRENTRY DirEntry;
DIRENTRY DirNew;
USHORT rc;

   _asm push es;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FILEATTRIBUTE, Flag = %X for %s", usFlag, pName);

   pVolInfo = GetVolInfo(pcdfsi->cdi_hVPB);
   if (IsDriveLocked(pVolInfo))
      {
      rc = ERROR_DRIVE_LOCKED;
      goto FS_FILEATTRIBUTEEXIT;
      }

   if (strlen(pName) > FAT32MAXPATH)
      {
      rc = ERROR_FILENAME_EXCED_RANGE;
      goto FS_FILEATTRIBUTEEXIT;
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
      goto FS_FILEATTRIBUTEEXIT;
      }
   ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszFile, &DirEntry, NULL);
   if (ulCluster == FAT_EOF)
      {
      rc = ERROR_FILE_NOT_FOUND;
      goto FS_FILEATTRIBUTEEXIT;
      }

   switch (usFlag)
      {
      case FA_RETRIEVE :
         *pAttr = DirEntry.bAttr;
         rc = 0;
         break;

      case FA_SET     :
         {
         USHORT usMask;

         if (!pVolInfo->fDiskCleanOnMount)
            {
            rc = ERROR_VOLUME_DIRTY;
            goto FS_FILEATTRIBUTEEXIT;
            }

         if (pVolInfo->fWriteProtected)
            {
            rc = ERROR_WRITE_PROTECT;
            goto FS_FILEATTRIBUTEEXIT;
            }

         usMask = ~(FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_ARCHIVED);
         if (*pAttr & usMask)
            {
            rc = ERROR_ACCESS_DENIED;
            goto FS_FILEATTRIBUTEEXIT;
            }

         memcpy(&DirNew, &DirEntry, sizeof (DIRENTRY));

         if (DirNew.bAttr & FILE_DIRECTORY)
            DirNew.bAttr = (BYTE)(*pAttr | FILE_DIRECTORY);
         else
            DirNew.bAttr = (BYTE)*pAttr;
         rc = ModifyDirectory(pVolInfo, ulDirCluster, MODIFY_DIR_UPDATE,
            &DirEntry, &DirNew, NULL, 0);
         break;
         }
      default:
         rc = ERROR_INVALID_FUNCTION;
         break;
      }

FS_FILEATTRIBUTEEXIT:

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FILEATTRIBUTE returned %d", rc);

   _asm pop es;

   return rc;
}


/******************************************************************
*
******************************************************************/
int far pascal _loadds FS_PATHINFO(
    unsigned short usFlag,      /* flag     */
    struct cdfsi far * pcdfsi,      /* pcdfsi   */
    struct cdfsd far * pcdfsd,      /* pcdfsd   */
    char far * pName,           /* pName    */
    unsigned short usCurDirEnd,     /* iCurDirEnd   */
    unsigned short usLevel,     /* level    */
    char far * pData,           /* pData    */
    unsigned short cbData       /* cbData   */
)
{
PVOLINFO pVolInfo;
ULONG ulCluster;
ULONG ulDirCluster;
PSZ   pszFile;
DIRENTRY DirEntry;
USHORT usNeededSize;
USHORT rc;

   _asm push es;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_PATHINFO Flag = %d, Level = %d called for %s, cbData = %u",
          usFlag, usLevel, pName, cbData);

   pVolInfo = GetVolInfo(pcdfsi->cdi_hVPB);
   if (IsDriveLocked(pVolInfo))
      {
      rc = ERROR_DRIVE_LOCKED;
      goto FS_PATHINFOEXIT;
      }

   if (strlen(pName) > FAT32MAXPATH)
      {
      rc = ERROR_FILENAME_EXCED_RANGE;
      goto FS_PATHINFOEXIT;
      }

   if (usLevel != FIL_NAMEISVALID)
      {
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
         goto FS_PATHINFOEXIT;
         }
      }

   if (usFlag == PI_RETRIEVE)
      {
      BYTE szFullName[FAT32MAXPATH];

      if (usLevel != FIL_NAMEISVALID)
         {
         ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszFile, &DirEntry, NULL);
         if (ulCluster == FAT_EOF)
            {
            rc = ERROR_FILE_NOT_FOUND;
            goto FS_PATHINFOEXIT;
            }

         if( TranslateName( pVolInfo, 0L, pName, szFullName, TRANSLATE_SHORT_TO_LONG ))
            strcpy( szFullName, pName );
         }

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
         case FIL_NAMEISVALID:
            rc = 0;
            goto FS_PATHINFOEXIT;
         case 7:
            usNeededSize = strlen(szFullName) + 1;
            break;
         default                   :
            rc = ERROR_INVALID_LEVEL;
            goto FS_PATHINFOEXIT;
         }

      if (cbData < usNeededSize)
         {
         rc = ERROR_BUFFER_OVERFLOW;
         goto FS_PATHINFOEXIT;
         }

      rc = MY_PROBEBUF(PB_OPWRITE, pData, cbData);
      if (rc)
         {
         Message("Protection VIOLATION in FS_PATHINFO!\n");
         goto FS_PATHINFOEXIT;
         }

      switch (usLevel)
         {
         case FIL_STANDARD         :
            {
            PFILESTATUS pfStatus = (PFILESTATUS)pData;

            memset(pfStatus, 0, sizeof (FILESTATUS));
            pfStatus->fdateCreation = DirEntry.wCreateDate;
            pfStatus->ftimeCreation = DirEntry.wCreateTime;
            pfStatus->fdateLastAccess = DirEntry.wAccessDate;
            pfStatus->fdateLastWrite = DirEntry.wLastWriteDate;
            pfStatus->ftimeLastWrite = DirEntry.wLastWriteTime;

            if (!(DirEntry.bAttr & FILE_DIRECTORY))
               {
               pfStatus->cbFile = DirEntry.ulFileSize;
               pfStatus->cbFileAlloc =
                  (pfStatus->cbFile / pVolInfo->usClusterSize) * pVolInfo->usClusterSize +
                  (pfStatus->cbFile % pVolInfo->usClusterSize ? pVolInfo->usClusterSize : 0);
               }

            pfStatus->attrFile = (USHORT)DirEntry.bAttr;
            rc = 0;
            break;
            }

         case FIL_QUERYEASIZE      :
            {
            PFILESTATUS2 pfStatus = (PFILESTATUS2)pData;

            memset(pfStatus, 0, sizeof (FILESTATUS2));

            pfStatus->fdateCreation = DirEntry.wCreateDate;
            pfStatus->ftimeCreation = DirEntry.wCreateTime;
            pfStatus->fdateLastAccess = DirEntry.wAccessDate;
            pfStatus->fdateLastWrite = DirEntry.wLastWriteDate;
            pfStatus->ftimeLastWrite = DirEntry.wLastWriteTime;
            if (!(DirEntry.bAttr & FILE_DIRECTORY))
               {
               pfStatus->cbFile = DirEntry.ulFileSize;
               pfStatus->cbFileAlloc =
                  (pfStatus->cbFile / pVolInfo->usClusterSize) * pVolInfo->usClusterSize +
                  (pfStatus->cbFile % pVolInfo->usClusterSize ? pVolInfo->usClusterSize : 0);
               }

            pfStatus->attrFile = (USHORT)DirEntry.bAttr;
            if (!f32Parms.fEAS)
               {
               pfStatus->cbList = sizeof pfStatus->cbList;
               rc = 0;
               }
            else
               {
               rc = usGetEASize(pVolInfo, ulDirCluster, pszFile, &pfStatus->cbList);
               }
            break;
            }

         case FIL_QUERYEASFROMLIST:
            {
            PEAOP pEA = (PEAOP)pData;
            PFEALIST pFEA = pEA->fpFEAList;
            rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pFEA, sizeof pFEA->cbList);
            if (rc)
               {
               Message("Protection VIOLATION in FS_PATHINFO!\n");
               goto FS_PATHINFOEXIT;
               }
            rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pFEA, (USHORT)pFEA->cbList);
            if (rc)
               {
               Message("Protection VIOLATION in FS_PILEINFO!\n");
               goto FS_PATHINFOEXIT;
               }
            if (!f32Parms.fEAS)
               {
               rc = usGetEmptyEAS(pszFile,pEA);
               }
            else
               {
               rc = usGetEAS(pVolInfo, usLevel, ulDirCluster, pszFile, pEA);
               }

            break;
            }

         case 4:
            {
            PEAOP pEA = (PEAOP)pData;
            PFEALIST pFEA = pEA->fpFEAList;
            rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pFEA, sizeof pFEA->cbList);
            if (rc)
               {
               Message("Protection VIOLATION in FS_PATHINFO!\n");
               goto FS_PATHINFOEXIT;
               }
            rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pFEA, (USHORT)pFEA->cbList);
            if (rc)
               {
               Message("Protection VIOLATION in FS_PATHINFO!\n");
               goto FS_PATHINFOEXIT;
               }
            if (!f32Parms.fEAS)
               {
               memset(pFEA, 0, (USHORT)pFEA->cbList);
               pFEA->cbList = sizeof pFEA->cbList;
               rc = 0;
               }
            else
               {
               rc = usGetEAS(pVolInfo, usLevel, ulDirCluster, pszFile, pEA);
               }

            break;
            }

         case FIL_NAMEISVALID :
            rc = 0;
            break;

         case 7:
            strcpy(pData, szFullName);
            rc = 0;
            break;

         default :
            rc = ERROR_INVALID_LEVEL;
            break;
         }
      goto FS_PATHINFOEXIT;
      }


   if (usFlag & PI_SET)
      {
      if (!pVolInfo->fDiskCleanOnMount)
         {
         rc = ERROR_VOLUME_DIRTY;
         goto FS_PATHINFOEXIT;
         }
      if (pVolInfo->fWriteProtected)
         {
         rc = ERROR_WRITE_PROTECT;
         goto FS_PATHINFOEXIT;
         }

      rc = MY_PROBEBUF(PB_OPREAD, pData, cbData);
      if (rc)
         {
         Message("Protection VIOLATION in FS_PATHINFO!\n");
         goto FS_PATHINFOEXIT;
         }

      if (usLevel == FIL_STANDARD || usLevel == FIL_QUERYEASIZE)
         {
         ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszFile, &DirEntry, NULL);
         if (ulCluster == FAT_EOF)
            {
            rc = ERROR_FILE_NOT_FOUND;
            goto FS_PATHINFOEXIT;
            }
         }

      switch (usLevel)
         {
         case FIL_STANDARD:
            {
            PFILESTATUS pfStatus = (PFILESTATUS)pData;
            USHORT usMask;
            DIRENTRY DirNew;

            if (cbData < sizeof (FILESTATUS))
               {
               rc = ERROR_INSUFFICIENT_BUFFER;
               goto FS_PATHINFOEXIT;
               }

            usMask = FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_ARCHIVED;
            if (DirEntry.bAttr & FILE_DIRECTORY)
               usMask |= FILE_DIRECTORY;
            usMask = ~usMask;

            if (pfStatus->attrFile & usMask)
               {
               if (f32Parms.fMessageActive & LOG_FS)
                  Message("Trying to set invalid attr bits: %X", pfStatus->attrFile);
               rc = ERROR_ACCESS_DENIED;
               goto FS_PATHINFOEXIT;
               }

            memcpy(&DirNew, &DirEntry, sizeof (DIRENTRY));

            usMask = 0;
            if (memcmp(&pfStatus->fdateCreation, &usMask, sizeof usMask) ||
                memcmp(&pfStatus->ftimeCreation, &usMask, sizeof usMask))
               {
               DirNew.wCreateDate = pfStatus->fdateCreation;
               DirNew.wCreateTime = pfStatus->ftimeCreation;
               }

            if (memcmp(&pfStatus->fdateLastWrite, &usMask, sizeof usMask) ||
                memcmp(&pfStatus->ftimeLastWrite, &usMask, sizeof usMask))
               {
               DirNew.wLastWriteDate = pfStatus->fdateLastWrite;
               DirNew.wLastWriteTime = pfStatus->ftimeLastWrite;
               }

            if (memcmp(&pfStatus->fdateLastAccess, &usMask, sizeof usMask))
               {
               DirNew.wAccessDate = pfStatus->fdateLastAccess;
               }

            if (DirNew.bAttr & FILE_DIRECTORY)
               DirNew.bAttr = (BYTE)(pfStatus->attrFile | FILE_DIRECTORY);
            else
               DirNew.bAttr = (BYTE)pfStatus->attrFile;

            rc = ModifyDirectory(pVolInfo, ulDirCluster, MODIFY_DIR_UPDATE,
               &DirEntry, &DirNew, NULL, 0);
            break;
            }

         case FIL_QUERYEASIZE      :
            if (!f32Parms.fEAS)
               rc = 0;
            else
               {
#if 0
               DIRENTRY DirNew;
#endif
               rc = usModifyEAS(pVolInfo, ulDirCluster, pszFile, (PEAOP)pData);
               if (rc)
                  goto FS_PATHINFOEXIT;
#if 0
               memcpy(&DirNew, &DirEntry, sizeof (DIRENTRY));
               DirNew.wLastWriteDate.year = pGI->year - 1980;
               DirNew.wLastWriteDate.month = pGI->month;
               DirNew.wLastWriteDate.day = pGI->day;
               DirNew.wLastWriteTime.hours = pGI->hour;
               DirNew.wLastWriteTime.minutes = pGI->minutes;
               DirNew.wLastWriteTime.twosecs = pGI->seconds / 2;
               rc = ModifyDirectory(pVolInfo, ulDirCluster, MODIFY_DIR_UPDATE,
                  &DirEntry, &DirNew, NULL, 0);
#endif
               }
            break;

         default          :
            rc = ERROR_INVALID_LEVEL;
            break;
         }
      }
   else
      rc = ERROR_INVALID_FUNCTION;

FS_PATHINFOEXIT:

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_PATHINFO returned %u", rc);

   _asm pop es;

   return rc;
}
