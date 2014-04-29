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

static USHORT FillDirEntry(PVOLINFO pVolInfo, PBYTE * ppData, PUSHORT pcbData, PFINDINFO pFindInfo, USHORT usLevel);
static BOOL GetCluster(PVOLINFO pVolInfo, PFINDINFO pFindInfo, USHORT usClusterIndex);

/******************************************************************
*
******************************************************************/
int far pascal __loadds FS_FINDCLOSE(struct fsfsi far * pfsfsi,
                            struct fsfsd far * pfsfsd)
{
PVOLINFO pVolInfo;
PFINDINFO pFindInfo = (PFINDINFO)pfsfsd;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FINDCLOSE");

   pVolInfo = GetVolInfo(pfsfsi->fsi_hVPB);

   if (pFindInfo->pInfo)
      {
      if (RemoveFindEntry(pVolInfo, pFindInfo->pInfo))
         free(pFindInfo->pInfo);
      pFindInfo->pInfo = NULL;
      }

   return 0;
}

/******************************************************************
*
******************************************************************/
int far pascal __loadds FS_FINDFIRST(struct cdfsi far * pcdfsi,      /* pcdfsi   */
                            struct cdfsd far * pcdfsd,      /* pcdfsd   */
                            char far * pName,           /* pName    */
                            unsigned short usCurDirEnd,     /* iCurDirEnd   */
                            unsigned short usAttr,      /* attr     */
                            struct fsfsi far * pfsfsi,      /* pfsfsi   */
                            struct fsfsd far * pfsfsd,      /* pfsfsd   */
                            char far * pData,           /* pData    */
                            unsigned short cbData,      /* cbData   */
                            unsigned short far * pcMatch,   /* pcMatch  */
                            unsigned short usLevel,     /* level    */
                            unsigned short usFlags)     /* flags    */
{
PVOLINFO pVolInfo;
PFINDINFO pFindInfo = (PFINDINFO)pfsfsd;
USHORT rc;
USHORT usIndex;
USHORT usNeededLen;
USHORT usNumClusters;
ULONG  ulCluster;
ULONG  ulDirCluster;
PSZ    pSearch;
PFINFO pNext;
ULONG ulNeededSpace;
USHORT usEntriesWanted;
EAOP   EAOP;
PROCINFO ProcInfo;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FINDFIRST for %s attr %X, Level %d, cbData %u, MaxEntries %u", pName, usAttr, usLevel, cbData, *pcMatch);

   usEntriesWanted = *pcMatch;
   *pcMatch  = 0;

   if (strlen(pName) > FAT32MAXPATH)
      {
      rc = ERROR_FILENAME_EXCED_RANGE;
      goto FS_FINDFIRSTEXIT;
      }

   memset(pfsfsd, 0, sizeof (struct fsfsd));

   pVolInfo = GetVolInfo(pfsfsi->fsi_hVPB);
   if (IsDriveLocked(pVolInfo))
      {
      rc = ERROR_DRIVE_LOCKED;
      goto FS_FINDFIRSTEXIT;
      }

   switch (usLevel)
      {
      case FIL_STANDARD         :
         usNeededLen = sizeof (FILEFNDBUF) - CCHMAXPATHCOMP;
         break;
      case FIL_QUERYEASIZE      :
         usNeededLen = sizeof (FILEFNDBUF2) - CCHMAXPATHCOMP;
         break;
      case FIL_QUERYEASFROMLIST :
         usNeededLen = sizeof (EAOP) + sizeof (FILEFNDBUF3) + sizeof (ULONG);
         break;
      default                   :
         rc = ERROR_NOT_SUPPORTED;
         goto FS_FINDFIRSTEXIT;
      }

   if (usFlags == FF_GETPOS)
      usNeededLen += sizeof (ULONG);

   if (cbData < usNeededLen)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      goto FS_FINDFIRSTEXIT;
      }

   rc = MY_PROBEBUF(PB_OPWRITE, pData, cbData);
   if (rc)
      {
      Message("FAT32: Protection VIOLATION in FS_FINDFIRST! (SYS%d)", rc);
      return rc;
      }

   if (usLevel == FIL_QUERYEASFROMLIST)
      {
      memcpy(&EAOP, pData, sizeof (EAOP));
      rc = MY_PROBEBUF(PB_OPREAD,
         (PBYTE)EAOP.fpGEAList,
         (USHORT)EAOP.fpGEAList->cbList);
      if (rc)
         goto FS_FINDFIRSTEXIT;
      }
   memset(pData, 0, cbData);

   usNumClusters = 0;
   ulDirCluster = FindDirCluster(pVolInfo,
      pcdfsi,
      pcdfsd,
      pName,
      usCurDirEnd,
      RETURN_PARENT_DIR,
      &pSearch);


   if (ulDirCluster == FAT_EOF)
      {
      rc = ERROR_PATH_NOT_FOUND;
      goto FS_FINDFIRSTEXIT;
      }

   ulCluster = ulDirCluster;
   while (ulCluster && ulCluster != FAT_EOF)
      {
      usNumClusters++;
      ulCluster = GetNextCluster( pVolInfo, ulCluster);
      }

   ulNeededSpace = sizeof (FINFO) + (usNumClusters - 1) * sizeof (ULONG);
   ulNeededSpace += pVolInfo->usClusterSize;

   GetProcInfo(&ProcInfo, sizeof ProcInfo);


   pFindInfo->pInfo = (PFINFO)malloc((size_t)ulNeededSpace);
   if (!pFindInfo->pInfo)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      goto FS_FINDFIRSTEXIT;
      }

   memset(pFindInfo->pInfo, 0, (size_t)ulNeededSpace);
   if (!pVolInfo->pFindInfo)
      pVolInfo->pFindInfo = pFindInfo->pInfo;
   else
      {
      pNext = (PFINFO)pVolInfo->pFindInfo;
      while (pNext->pNextEntry)
         pNext = (PFINFO)pNext->pNextEntry;
      pNext->pNextEntry = pFindInfo->pInfo;
      }

   memcpy(&pFindInfo->pInfo->EAOP, &EAOP, sizeof (EAOP));
   pFindInfo->usEntriesPerCluster = pVolInfo->usClusterSize / sizeof (DIRENTRY);
   pFindInfo->usClusterIndex = 0;
   pFindInfo->pInfo->rgClusters[0] = ulDirCluster;
   pFindInfo->usTotalClusters = usNumClusters;
   pFindInfo->pInfo->pDirEntries =
      (PDIRENTRY)(&pFindInfo->pInfo->rgClusters[usNumClusters]);

   if (f32Parms.fMessageActive & LOG_FIND)
      Message("pInfo at %lX, pDirEntries at %lX",
         pFindInfo->pInfo, pFindInfo->pInfo->pDirEntries);

   pFindInfo->pInfo->pNextEntry = NULL;
   memcpy(&pFindInfo->pInfo->ProcInfo, &ProcInfo, sizeof (PROCINFO));

   strcpy(pFindInfo->pInfo->szSearch, pSearch);
   FSH_UPPERCASE(pFindInfo->pInfo->szSearch, sizeof pFindInfo->pInfo->szSearch, pFindInfo->pInfo->szSearch);

   pFindInfo->ulMaxEntry   = ((ULONG)pVolInfo->usClusterSize / sizeof (DIRENTRY)) * usNumClusters;
   if (!GetCluster(pVolInfo, pFindInfo, 0))
      {
      rc = ERROR_SYS_INTERNAL;
      goto FS_FINDFIRSTEXIT;
      }

   pFindInfo->ulCurEntry = 0;


   if (usAttr & 0x0040)
      {
      pFindInfo->fLongNames = TRUE;
      usAttr &= ~0x0040;
      }
   else
      pFindInfo->fLongNames = FALSE;

   pFindInfo->bMustAttr = (BYTE)(usAttr >> 8);
   usAttr |= (FILE_READONLY | FILE_ARCHIVED);
   usAttr &= (FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_DIRECTORY | FILE_ARCHIVED);
   pFindInfo->bAttr = (BYTE)~usAttr;

   if (usLevel == FIL_QUERYEASFROMLIST)
      {
      memcpy(pData, &pFindInfo->pInfo->EAOP, sizeof (EAOP));
      pData += sizeof (EAOP);
      cbData -= sizeof (EAOP);
      }

   rc = 0;
   for (usIndex = 0; usIndex < usEntriesWanted; usIndex++)
      {
      PULONG pulOrdinal;

      if (usFlags == FF_GETPOS)
         {
         if (cbData < sizeof (ULONG))
            {
            rc = ERROR_BUFFER_OVERFLOW;
            break;
            }
         pulOrdinal = (PULONG)pData;
         pData += sizeof (ULONG);
         cbData -= sizeof (ULONG);
         }

      rc = FillDirEntry(pVolInfo, &pData, &cbData, pFindInfo, usLevel);
      if (!rc || (rc == ERROR_EAS_DIDNT_FIT && usIndex == 0))
         {
         if (usFlags == FF_GETPOS)
            *pulOrdinal = pFindInfo->ulCurEntry - 1;
         }
      if (rc)
         break;
      }


   if ((rc == ERROR_NO_MORE_FILES ||
        rc == ERROR_BUFFER_OVERFLOW ||
        rc == ERROR_EAS_DIDNT_FIT)
       && usIndex > 0)
      rc = 0;

   if (rc == ERROR_EAS_DIDNT_FIT && usIndex == 0)
      usIndex = 1;

   *pcMatch = usIndex;


FS_FINDFIRSTEXIT:

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FINDFIRST returned %d (%d entries)",
         rc, *pcMatch);

   if (rc && rc != ERROR_EAS_DIDNT_FIT)
      {
      FS_FINDCLOSE(pfsfsi, pfsfsd);
      }

   return rc;
}


/******************************************************************
*
******************************************************************/
int far pascal __loadds FS_FINDFROMNAME(
    struct fsfsi far * pfsfsi,      /* pfsfsi   */
    struct fsfsd far * pfsfsd,      /* pfsfsd   */
    char far * pData,           /* pData    */
    unsigned short cbData,      /* cbData   */
    unsigned short far * pcMatch,   /* pcMatch  */
    unsigned short usLevel,     /* level    */
    unsigned long ulPosition,       /* position */
    char far * pName,           /* pName    */
    unsigned short usFlags      /* flags    */
)
{
PFINDINFO pFindInfo = (PFINDINFO)pfsfsd;

   pName = pName;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FINDFROMNAME, curpos = %lu, requested %lu",
         pFindInfo->ulCurEntry, ulPosition);

   pFindInfo->ulCurEntry = ulPosition + 1;

   return FS_FINDNEXT(pfsfsi, pfsfsd, pData, cbData, pcMatch, usLevel, usFlags);
}

/******************************************************************
*
******************************************************************/
int far pascal __loadds FS_FINDNEXT(
    struct fsfsi far * pfsfsi,      /* pfsfsi   */
    struct fsfsd far * pfsfsd,      /* pfsfsd   */
    char far * pData,           /* pData    */
    unsigned short cbData,      /* cbData   */
    unsigned short far * pcMatch,   /* pcMatch  */
    unsigned short usLevel,     /* level    */
    unsigned short usFlags      /* flag     */
)
{
PVOLINFO pVolInfo;
PFINDINFO pFindInfo = (PFINDINFO)pfsfsd;
USHORT rc;
USHORT usIndex;
USHORT usNeededLen;
USHORT usEntriesWanted;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FINDNEXT, level %u, cbData %u, MaxEntries %u", usLevel, cbData, *pcMatch);

   usEntriesWanted = *pcMatch;
   *pcMatch = 0;

   pVolInfo = GetVolInfo(pfsfsi->fsi_hVPB);
   if (IsDriveLocked(pVolInfo))
      {
      rc = ERROR_DRIVE_LOCKED;
      goto FS_FINDNEXTEXIT;
      }

   switch (usLevel)
      {
      case FIL_STANDARD         :
         usNeededLen = sizeof (FILEFNDBUF) - CCHMAXPATHCOMP;
         break;
      case FIL_QUERYEASIZE      :
         usNeededLen = sizeof (FILEFNDBUF2) - CCHMAXPATHCOMP;
         break;
      case FIL_QUERYEASFROMLIST :
         usNeededLen = sizeof (EAOP) + sizeof (FILEFNDBUF3) + sizeof (ULONG);
         break;
      default                   :
         rc = ERROR_NOT_SUPPORTED;
         goto FS_FINDNEXTEXIT;
      }

   if (usFlags == FF_GETPOS)
      usNeededLen += sizeof (ULONG);

   if (cbData < usNeededLen)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      goto FS_FINDNEXTEXIT;
      }

   rc = MY_PROBEBUF(PB_OPWRITE, pData, cbData);
   if (rc)
      {
      Message("FAT32: Protection VIOLATION in FS_FINDNEXT!");
      return rc;
      }

   if (usLevel == FIL_QUERYEASFROMLIST)
      {
      memcpy(&pFindInfo->pInfo->EAOP, pData, sizeof (EAOP));
      rc = MY_PROBEBUF(PB_OPREAD,
         (PBYTE)pFindInfo->pInfo->EAOP.fpGEAList,
         (USHORT)pFindInfo->pInfo->EAOP.fpGEAList->cbList);
      if (rc)
         goto FS_FINDNEXTEXIT;
      }

   memset(pData, 0, cbData);

   if (usLevel == FIL_QUERYEASFROMLIST)
      {
      memcpy(pData, &pFindInfo->pInfo->EAOP, sizeof (EAOP));
      pData += sizeof (EAOP);
      cbData -= sizeof (EAOP);
      }

   rc = 0;
   for (usIndex = 0; usIndex < usEntriesWanted; usIndex++)
      {
      PULONG pulOrdinal = NULL;

      if (usFlags == FF_GETPOS)
         {
         if (cbData < sizeof (ULONG))
            {
            rc = ERROR_BUFFER_OVERFLOW;
            break;
            }

         pulOrdinal = (PULONG)pData;
         pData += sizeof (ULONG);
         cbData -= sizeof (ULONG);
         }

      rc = FillDirEntry(pVolInfo, &pData, &cbData, pFindInfo, usLevel);
      if (!rc || (rc == ERROR_EAS_DIDNT_FIT && usIndex == 0))
         {
         if (usFlags == FF_GETPOS)
            *pulOrdinal = pFindInfo->ulCurEntry - 1;
         }
      if (rc)
         break;
      }

   if ((rc == ERROR_NO_MORE_FILES ||
        rc == ERROR_BUFFER_OVERFLOW ||
        rc == ERROR_EAS_DIDNT_FIT)
      && usIndex > 0)
      rc = 0;
   if (rc == ERROR_EAS_DIDNT_FIT && usIndex == 0)
      usIndex = 1;

   *pcMatch = usIndex;


FS_FINDNEXTEXIT:

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FINDNEXT returned %d (%d entries)",
         rc, *pcMatch);
   return rc;
}


/******************************************************************
*
******************************************************************/
USHORT FillDirEntry(PVOLINFO pVolInfo, PBYTE * ppData, PUSHORT pcbData, PFINDINFO pFindInfo, USHORT usLevel)
{
BYTE szLongName[FAT32MAXPATHCOMP];
BYTE szUpperName[FAT32MAXPATHCOMP];
BYTE szShortName[14];
PBYTE pStart = *ppData;
USHORT rc;
DIRENTRY _huge * pDir;
BYTE bCheck1;
USHORT usClusterIndex;


   memset(szLongName, 0, sizeof szLongName);
   pDir = &pFindInfo->pInfo->pDirEntries[pFindInfo->ulCurEntry % pFindInfo->usEntriesPerCluster];
   bCheck1 = 0;
   while (pFindInfo->ulCurEntry < pFindInfo->ulMaxEntry)
      {
      memset(szShortName, 0, sizeof(szShortName)); // vs

      usClusterIndex = (USHORT)(pFindInfo->ulCurEntry / pFindInfo->usEntriesPerCluster);
      if (usClusterIndex != pFindInfo->usClusterIndex)
         {
         if (!GetCluster(pVolInfo, pFindInfo, usClusterIndex))
            return ERROR_SYS_INTERNAL;
         pDir = &pFindInfo->pInfo->pDirEntries[pFindInfo->ulCurEntry % pFindInfo->usEntriesPerCluster];
         }

      if (pDir->bFileName[0] && pDir->bFileName[0] != DELETED_ENTRY)
         {
         if (pDir->bAttr == FILE_LONGNAME)
            {
            fGetLongName(pDir, szLongName, sizeof szLongName, &bCheck1);
            }
         else if ((pDir->bAttr & FILE_VOLID) != FILE_VOLID)
            {
            if (!(pDir->bAttr & pFindInfo->bAttr))
               {
               BYTE bCheck2 = GetVFATCheckSum(pDir);
               MakeName(pDir, szShortName, sizeof szShortName);
               FSH_UPPERCASE(szShortName, sizeof szShortName, szShortName);

               rc = 0;

               if (f32Parms.fEAS && bCheck2 == bCheck1 && strlen(szLongName))
                  if (IsEASFile(szLongName))
                     rc = 1;

               if (f32Parms.fMessageActive & LOG_FIND)
                  {
                  if (bCheck2 != bCheck1 && strlen(szLongName))
                     Message("Invalid LFN entry found: %s", szLongName);
                  }

               if (bCheck2 != bCheck1 ||
                  !strlen(szLongName))
               {
                  strcpy(szLongName, szShortName);

                  /* support for the FAT32 variation of WinNT family */
                  if( HAS_WINNT_EXT( pDir->fEAS ))
                  {
                        PBYTE pDot = strchr( szLongName, '.' );;

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
               }

               if (f32Parms.fEAS && IsEASFile(szLongName))
                  rc = 1;

               strcpy(szUpperName, szLongName);
               FSH_UPPERCASE(szUpperName, sizeof szUpperName, szUpperName);

               if( !pFindInfo->fLongNames )
                  strcpy( szLongName, szShortName );

               /*
                  Check for MUST HAVE attributes
               */
               if (!rc && pFindInfo->bMustAttr)
                  {
                  if ((pDir->bAttr & pFindInfo->bMustAttr) != pFindInfo->bMustAttr)
                     rc = 1;
                  }

               if (!rc && strlen(pFindInfo->pInfo->szSearch))
                  {
                  rc = FSH_WILDMATCH(pFindInfo->pInfo->szSearch, szUpperName);
                  if (rc && stricmp(szShortName, szUpperName))
                     rc = FSH_WILDMATCH(pFindInfo->pInfo->szSearch, szShortName);
                  }
               if (!rc && f32Parms.fMessageActive & LOG_FIND)
                  Message("%lu : %s, %s", pFindInfo->ulCurEntry, szLongName, szShortName );

               if (!rc && usLevel == FIL_STANDARD)
                  {
                  PFILEFNDBUF pfFind = (PFILEFNDBUF)*ppData;

                  if (*pcbData < sizeof (FILEFNDBUF) - CCHMAXPATHCOMP + strlen(szLongName) + 1)
                     return ERROR_BUFFER_OVERFLOW;

                  pfFind->fdateCreation = pDir->wCreateDate;
                  pfFind->ftimeCreation = pDir->wCreateTime;
                  pfFind->fdateLastAccess = pDir->wAccessDate;
                  pfFind->fdateLastWrite = pDir->wLastWriteDate;
                  pfFind->ftimeLastWrite = pDir->wLastWriteTime;
                  pfFind->cbFile = pDir->ulFileSize;
                  pfFind->cbFileAlloc =
                     (pfFind->cbFile / pVolInfo->usClusterSize) * pVolInfo->usClusterSize +
                     (pfFind->cbFile % pVolInfo->usClusterSize ? pVolInfo->usClusterSize : 0);

                  pfFind->attrFile = (USHORT)pDir->bAttr;
                  pfFind->cchName = (BYTE)strlen(szLongName);
                  strcpy(pfFind->achName, szLongName);
                  *ppData = pfFind->achName + pfFind->cchName + 1;
                  (*pcbData) -= *ppData - pStart;
                  pFindInfo->ulCurEntry++;
                  return 0;
                  }
               else if (!rc && usLevel == FIL_QUERYEASIZE)
                  {
                  PFILEFNDBUF2 pfFind = (PFILEFNDBUF2)*ppData;

                  if (*pcbData < sizeof (FILEFNDBUF2) - CCHMAXPATHCOMP + strlen(szLongName) + 1)
                     return ERROR_BUFFER_OVERFLOW;

                  pfFind->fdateCreation = pDir->wCreateDate;
                  pfFind->ftimeCreation = pDir->wCreateTime;
                  pfFind->fdateLastAccess = pDir->wAccessDate;
                  pfFind->fdateLastWrite = pDir->wLastWriteDate;
                  pfFind->ftimeLastWrite = pDir->wLastWriteTime;
                  pfFind->cbFile = pDir->ulFileSize;
                  pfFind->cbFileAlloc =
                     (pfFind->cbFile / pVolInfo->usClusterSize)  +
                     (pfFind->cbFile % pVolInfo->usClusterSize ? 1 : 0);
                  if (!f32Parms.fEAS || !HAS_EAS( pDir->fEAS ))
                     pfFind->cbList = sizeof pfFind->cbList;
                  else
                     {
                     rc = usGetEASize(pVolInfo, pFindInfo->pInfo->rgClusters[0],
                        szLongName, &pfFind->cbList);
                     if (rc)
                        pfFind->cbList = 4;
                     rc = 0;
                     }
                  pfFind->attrFile = (USHORT)pDir->bAttr;
                  pfFind->cchName = (BYTE)strlen(szLongName);
                  strcpy(pfFind->achName, szLongName);
                  *ppData = pfFind->achName + pfFind->cchName + 1;
                  (*pcbData) -= *ppData - pStart;
                  pFindInfo->ulCurEntry++;
                  return 0;
                  }
               else if (!rc && usLevel == FIL_QUERYEASFROMLIST)
                  {
                  PFILEFNDBUF3 pfFind = (PFILEFNDBUF3)*ppData;
                  ULONG ulFeaSize;

                  if (*pcbData < sizeof (FILEFNDBUF3) + sizeof (ULONG) + strlen(szLongName) + 2)
                     return ERROR_BUFFER_OVERFLOW;

                  pfFind->fdateCreation = pDir->wCreateDate;
                  pfFind->ftimeCreation = pDir->wCreateTime;
                  pfFind->fdateLastAccess = pDir->wAccessDate;
                  pfFind->fdateLastWrite = pDir->wLastWriteDate;
                  pfFind->ftimeLastWrite = pDir->wLastWriteTime;
                  pfFind->cbFile = pDir->ulFileSize;
                  pfFind->cbFileAlloc =
                     (pfFind->cbFile / pVolInfo->usClusterSize) * pVolInfo->usClusterSize +
                     (pfFind->cbFile % pVolInfo->usClusterSize ? pVolInfo->usClusterSize : 0);
                  pfFind->attrFile = (USHORT)pDir->bAttr;
                  *ppData = (PBYTE)(pfFind + 1);
                  (*pcbData) -= *ppData - pStart;

                  if (f32Parms.fEAS && HAS_EAS( pDir->fEAS ))
                     {
                     pFindInfo->pInfo->EAOP.fpFEAList = (PFEALIST)*ppData;
                     pFindInfo->pInfo->EAOP.fpFEAList->cbList =
                        *pcbData - (strlen(szLongName) + 2);

                     rc = usGetEAS(pVolInfo, FIL_QUERYEASFROMLIST,
                        pFindInfo->pInfo->rgClusters[0],
                        szLongName, &pFindInfo->pInfo->EAOP);
                     if (rc && rc != ERROR_BUFFER_OVERFLOW)
                        return rc;
                     if (rc)
                        {
                        rc = ERROR_EAS_DIDNT_FIT;
                        ulFeaSize = sizeof (ULONG);
                        }
                     else
                        ulFeaSize = pFindInfo->pInfo->EAOP.fpFEAList->cbList;
                     }
                  else
                     {
                     pFindInfo->pInfo->EAOP.fpFEAList = (PFEALIST)*ppData;
                     pFindInfo->pInfo->EAOP.fpFEAList->cbList =
                        *pcbData - (strlen(szLongName) + 2);

                     rc = usGetEmptyEAS(szLongName,&pFindInfo->pInfo->EAOP);

                     if (rc && (rc != ERROR_EAS_DIDNT_FIT))
                        return rc;
                     else if (rc == ERROR_EAS_DIDNT_FIT)
                        ulFeaSize = sizeof(pFindInfo->pInfo->EAOP.fpFEAList->cbList);
                     else
                        ulFeaSize = pFindInfo->pInfo->EAOP.fpFEAList->cbList;
                     }
                  (*ppData) += ulFeaSize;
                  (*pcbData) -= ulFeaSize;

                  /*
                     Length and longname
                  */

                  *(*ppData)++ = (BYTE)strlen(szLongName);
                  (*pcbData)--;
                  strcpy(*ppData, szLongName);

                  (*ppData) += strlen(szLongName) + 1;
                  (*pcbData) -= (strlen(szLongName) + 1);

                  pFindInfo->ulCurEntry++;
                  return rc;
                  }
               }
            memset(szLongName, 0, sizeof szLongName);
            }
         }
      pFindInfo->ulCurEntry++;
      pDir++;
      }
   return ERROR_NO_MORE_FILES;
}

VOID MakeName(PDIRENTRY pDir, PSZ pszName, USHORT usMax)
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
}


BOOL fGetLongName(PDIRENTRY pDir, PSZ pszName, USHORT wMax, PBYTE pbCheck)
{
BYTE szLongName[30] = "";
USHORT uniName[15] = {0};
USHORT wNameSize;
USHORT usIndex;
PLNENTRY pLN = (PLNENTRY)pDir;

   memset(szLongName, 0, sizeof szLongName);
   memset(uniName, 0, sizeof uniName);

   wNameSize = 0;
   if (pLN->bVFATCheckSum != *pbCheck)
      {
      memset(pszName, 0, wMax);
      *pbCheck = pLN->bVFATCheckSum;
      }

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

   Translate2OS2(uniName, szLongName, sizeof szLongName);

   wNameSize = strlen( szLongName );
   if (strlen(pszName) + wNameSize > wMax)
      return FALSE;

   memmove(pszName + wNameSize, pszName, strlen(pszName) + 1);
   memcpy(pszName, szLongName, wNameSize);
   return TRUE;
}



/******************************************************************
*
******************************************************************/
int far pascal __loadds FS_FINDNOTIFYCLOSE( unsigned short usHandle)
{
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FINDNOTIFYCLOSE - NOT SUPPORTED");
   return ERROR_NOT_SUPPORTED;

   usHandle = usHandle;
}

/******************************************************************
*
******************************************************************/
int far pascal __loadds FS_FINDNOTIFYFIRST(
    struct cdfsi far * pcdfsi,      /* pcdfsi   */
    struct cdfsd far * pcdfsd,      /* pcdfsd   */
    char far * pName,           /* pName    */
    unsigned short usCurDirEnd,     /* iCurDirEnd   */
    unsigned short usAttr,      /* attr     */
    unsigned short far * pHandle,   /* pHandle  */
    char far * pData,           /* pData    */
    unsigned short cbData,      /* cbData   */
    unsigned short far * pcMatch,   /* pcMatch  */
    unsigned short usLevel,     /* level    */
    unsigned long   ulTimeOut   /* timeout  */
)
{
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FINDNOTIFYFIRST - NOT SUPPORTED");

   return ERROR_NOT_SUPPORTED;

   pcdfsi = pcdfsi;
   pcdfsd = pcdfsd;
   pName = pName;
   usCurDirEnd = usCurDirEnd;
   usAttr = usAttr;
   pHandle = pHandle;
   pData = pData;
   cbData = cbData;
   pcMatch = pcMatch;
   usLevel = usLevel;
   ulTimeOut =ulTimeOut;
}

/******************************************************************
*
******************************************************************/
int far pascal __loadds FS_FINDNOTIFYNEXT(
    unsigned short usHandle,        /* handle   */
    char far * pData,           /* pData    */
    unsigned short cbData,      /* cbData   */
    unsigned short far * pcMatch,   /* pcMatch  */
    unsigned short usInfoLevel,     /* infolevel    */
    unsigned long    ulTimeOut  /* timeout  */
)
{
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FINDNOTIFYNEXT - NOT SUPPORTED");

   return ERROR_NOT_SUPPORTED;

   usHandle = usHandle;
   pData = pData;
   cbData = cbData;
   pcMatch = pcMatch;
   usInfoLevel = usInfoLevel;
   ulTimeOut = ulTimeOut;
}

BOOL GetCluster(PVOLINFO pVolInfo, PFINDINFO pFindInfo, USHORT usClusterIndex)
{
USHORT usIndex;

   if (usClusterIndex >= pFindInfo->usTotalClusters)
      return FALSE;

   if (!pFindInfo->pInfo->rgClusters[usClusterIndex])
      {
      for (usIndex = pFindInfo->usClusterIndex; usIndex < usClusterIndex; usIndex++)
         {
         pFindInfo->pInfo->rgClusters[usIndex + 1] =
            GetNextCluster( pVolInfo, pFindInfo->pInfo->rgClusters[usIndex]);

         if (!pFindInfo->pInfo->rgClusters[usIndex + 1])
            pFindInfo->pInfo->rgClusters[usIndex + 1] = FAT_EOF;

         if (pFindInfo->pInfo->rgClusters[usIndex + 1] == FAT_EOF)
            return FALSE;
         }
      }

   if (pFindInfo->pInfo->rgClusters[usClusterIndex] == FAT_EOF)
      return FALSE;

   if (ReadCluster( pVolInfo,
      pFindInfo->pInfo->rgClusters[usClusterIndex], pFindInfo->pInfo->pDirEntries, 0))
      return FALSE;

   pFindInfo->usClusterIndex = usClusterIndex;
   return TRUE;
}
