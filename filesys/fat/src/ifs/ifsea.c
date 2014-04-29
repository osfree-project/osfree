#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <dos.h>

#define INCL_DOS
#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_DOSERRORS

#include "os2.h"
#include "portable.h"
#include "fat32ifs.h"


PRIVATE PFEA   FindEA(PFEALIST pFeal, PSZ pszName, USHORT usMaxName);
PRIVATE USHORT usReadEAS(PVOLINFO pVolInfo, ULONG ulDirCluster, PSZ pszFileName, PFEALIST * ppFEAL, BOOL fCreate);
PRIVATE USHORT usWriteEAS(PVOLINFO pVolInfo, ULONG ulDirCluster, PSZ pszFileName, PFEALIST pFEAL);
PRIVATE USHORT GetEASName(PVOLINFO pVolInfo, ULONG ulDirCluster, PSZ pszFileName, PSZ * pszEASName);


/************************************************************************
*
************************************************************************/
USHORT usModifyEAS(PVOLINFO pVolInfo, ULONG ulDirCluster, PSZ pszFileName, PEAOP pEAOP)
{
USHORT rc;
PFEALIST pTarFeal;
PFEALIST pSrcFeal;
PBYTE    pSrcMax;
PFEA pSrcFea;
PFEA pTarFea;

   if (f32Parms.fMessageActive & LOG_EAS)
      Message("usModifyEAS for %s", pszFileName);


   /*
      Do not allow ea's file files with no filename (root)
   */
   if (!strlen(pszFileName))
      return 284;

   rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pEAOP, sizeof (EAOP));
   if (rc)
      {
      Message("Protection violation in usModifyEAS (1) at %lX", pEAOP);
      return rc;
      }

   pSrcFeal = pEAOP->fpFEAList;
   if (pSrcFeal->cbList > MAX_EA_SIZE)
      return ERROR_EA_LIST_TOO_LONG;

   rc = MY_PROBEBUF(PB_OPREAD, (PBYTE)pSrcFeal, (USHORT)pSrcFeal->cbList);
   if (rc)
      {
      Message("Protection violation in usModifyEAS (2) at %lX", pSrcFeal);
      return rc;
      }

   if (pSrcFeal->cbList <= sizeof (ULONG))
      return 0;

   rc = usReadEAS(pVolInfo, ulDirCluster, pszFileName, &pTarFeal, TRUE);
   if (rc)
      return rc;

   if (f32Parms.fMessageActive & LOG_EAS)
      Message("cbList before = %lu", pTarFeal->cbList);


   pSrcMax = (PBYTE)pSrcFeal + pSrcFeal->cbList;
   pSrcFea = pSrcFeal->list;
   while ((PBYTE)pSrcFea + sizeof (FEA) < pSrcMax)
      {
      PBYTE pName;
      USHORT usNewSize = sizeof (FEA) + (USHORT)pSrcFea->cbName + 1 + pSrcFea->cbValue;

      if ((PBYTE)pSrcFea + usNewSize > pSrcMax)
         {
         pEAOP->oError = (PBYTE)pSrcFea - (PBYTE)pEAOP;
         rc = ERROR_EA_LIST_INCONSISTENT;
         goto usStoreEASExit;
         }
      pName  = (PBYTE)(pSrcFea + 1);

      rc = FSH_CHECKEANAME(0x0001, pSrcFea->cbName, pName);
      if (rc && pSrcFea->cbValue)
         {
         pEAOP->oError = (PBYTE)pSrcFea - (PBYTE)pEAOP;
         goto usStoreEASExit;
         }

      if (!pSrcFea->cbValue || !pSrcFea->cbName)
         usNewSize = 0;
      else
         usNewSize = sizeof (FEA) + (USHORT)pSrcFea->cbName + 1 + pSrcFea->cbValue;

      pTarFea = FindEA(pTarFeal, pName, pSrcFea->cbName);
      if (!pTarFea)
         {
         pTarFea = (PFEA)((PBYTE)pTarFeal + pTarFeal->cbList);
         if (MAX_EA_SIZE - pTarFeal->cbList < (ULONG)usNewSize)
            {
            rc = ERROR_EAS_DIDNT_FIT;
            goto usStoreEASExit;
            }
         memcpy(pTarFea, pSrcFea, usNewSize);
         pTarFeal->cbList += usNewSize;

         if (f32Parms.fMessageActive & LOG_EAS)
            Message("Inserting EA '%s' (%u,%u)", pName,
               pSrcFea->cbName, pSrcFea->cbValue);
         }
      else
         {
         USHORT usOldSize  = sizeof (FEA) + (USHORT)pTarFea->cbName + 1 + pTarFea->cbValue;
         USHORT usMoveSize = (USHORT)pTarFeal->cbList -
            ((PBYTE)pTarFea - (PBYTE)pTarFeal);
         usMoveSize -= usOldSize;

         if (usOldSize < usNewSize)
            {
            if (MAX_EA_SIZE - pTarFeal->cbList < (ULONG)(usNewSize - usOldSize))
               {
               rc = ERROR_EAS_DIDNT_FIT;
               goto usStoreEASExit;
               }
            }
         memmove((PBYTE)pTarFea + usNewSize,
                 (PBYTE)pTarFea + usOldSize, usMoveSize);
         memcpy(pTarFea, pSrcFea, usNewSize);
         pTarFeal->cbList -= usOldSize;
         pTarFeal->cbList += usNewSize;

         if (f32Parms.fMessageActive & LOG_EAS)
            Message("Updating EA '%s' (%u,%u)", pName,
               pSrcFea->cbName, pSrcFea->cbValue);
         }

      usNewSize = sizeof (FEA) + (USHORT)pSrcFea->cbName + 1 + pSrcFea->cbValue;
      pSrcFea = (PFEA)((PBYTE)pSrcFea + usNewSize);
      }

   if (f32Parms.fMessageActive & LOG_EAS)
      Message("cbList after = %lu", pTarFeal->cbList);

   if (pTarFeal->cbList > 4)
      rc = usWriteEAS(pVolInfo, ulDirCluster, pszFileName, pTarFeal);
   else
      rc = usDeleteEAS(pVolInfo, ulDirCluster, pszFileName);

usStoreEASExit:
   freeseg(pTarFeal);

   if (f32Parms.fMessageActive & LOG_EAS)
      Message("usModifyEAS for %s returned %d",
         pszFileName, rc);

   return rc;
}

/************************************************************************
*
************************************************************************/
USHORT usGetEASize(PVOLINFO pVolInfo, ULONG ulDirCluster, PSZ pszFileName, PULONG pulSize)
{
PSZ pszEAName;
USHORT rc;
DIRENTRY DirEntry;
ULONG    ulCluster;

   if (f32Parms.fMessageActive & LOG_EAS)
      Message("usGetEASSize for %s", pszFileName);

   *pulSize = sizeof (ULONG);

   rc = GetEASName(pVolInfo, ulDirCluster, pszFileName, &pszEAName);
   if (rc)
      return rc;

   ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszEAName, &DirEntry, NULL);
   if (ulCluster == FAT_EOF || !ulCluster)
      {
      rc = 0;
      goto usGetEASizeExit;
      }
   *pulSize = DirEntry.ulFileSize;

   rc = 0;

usGetEASizeExit:
   free(pszEAName);

   if (f32Parms.fMessageActive & LOG_EAS)
      Message("usGetEASize for %s returned %d (%u bytes large)",
         pszFileName, rc, *pulSize);

   return rc;
}

/************************************************************************
*
************************************************************************/
USHORT usGetEAS(PVOLINFO pVolInfo, USHORT usLevel, ULONG ulDirCluster, PSZ pszFileName, PEAOP pEAOP)
{
USHORT rc;
PFEALIST pTarFeal;
PFEALIST pSrcFeal;
PFEA     pSrcFea;
PFEA     pTarFea;
PGEALIST pGeaList;
USHORT   usMaxSize;

   if (f32Parms.fMessageActive & LOG_EAS)
      Message("usGetEAS for %s Level %d", pszFileName, usLevel);

   /*
      Checking all the arguments
   */

   rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pEAOP, sizeof (EAOP));
   if (rc)
      {
      Message("Protection violation in usGetEAS (1) at %lX", pEAOP);
      return rc;
      }

   pTarFeal = pEAOP->fpFEAList;
   rc = MY_PROBEBUF(PB_OPREAD, (PBYTE)pTarFeal, sizeof (ULONG));
   if (rc)
      {
      Message("Protection violation in usGetEAS (2) at %lX", pTarFeal);
      return rc;
      }
   if (pTarFeal->cbList > MAX_EA_SIZE)
      usMaxSize = (USHORT)MAX_EA_SIZE;
   else
      usMaxSize = (USHORT)pTarFeal->cbList;

   if (usMaxSize < sizeof (ULONG))
      return ERROR_BUFFER_OVERFLOW;

   rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pTarFeal, (USHORT)usMaxSize);
   if (rc)
      return rc;

   if (usLevel == FIL_QUERYEASFROMLIST)
      {
      pGeaList = pEAOP->fpGEAList;
      rc = MY_PROBEBUF(PB_OPREAD, (PBYTE)pGeaList, sizeof (ULONG));
      if (rc)
         return rc;
      if (pGeaList->cbList > MAX_EA_SIZE)
         return ERROR_EA_LIST_TOO_LONG;
      rc = MY_PROBEBUF(PB_OPREAD, (PBYTE)pGeaList, (USHORT)pGeaList->cbList);
      if (rc)
         return rc;
      }
   else
      pGeaList = NULL;

   /*
      Initialize the FEALIST
   */
   memset(pTarFeal, 0, usMaxSize);
   pTarFeal->cbList = sizeof (ULONG);
   usMaxSize -= sizeof (ULONG);
   pTarFea = pTarFeal->list;

   /*
      Does the EA Exist?
   */

   rc = usReadEAS(pVolInfo, ulDirCluster, pszFileName, &pSrcFeal, FALSE);
   if (rc)
      goto usGetEASExit;

   /*
      If not, return
   */
   if (usLevel == FIL_QUERYEASFROMLIST)
      {
      PBYTE    pGeaMax;
      PGEA     pGea;

      pGeaMax = (PBYTE)pGeaList + pGeaList->cbList;
      pGea    = pGeaList->list;
      while ((PBYTE)pGea + sizeof (GEA) < pGeaMax)
         {
         USHORT usGeaSize = sizeof (GEA) + (USHORT)pGea->cbName;
         USHORT usFeaSize;

         if (pGea->szName + (USHORT)pGea->cbName > pGeaMax)
            {
            rc = ERROR_EA_LIST_INCONSISTENT;
            goto usGetEASExit;
            }

         pSrcFea = FindEA(pSrcFeal, pGea->szName, pGea->cbName);
         if (pSrcFea)
            {
            usFeaSize = sizeof (FEA) + (USHORT)pSrcFea->cbName + 1 + pSrcFea->cbValue;
            if (usFeaSize > usMaxSize)
               {
               rc = ERROR_BUFFER_OVERFLOW;
               pTarFeal->cbList = pSrcFeal->cbList;
               goto usGetEASExit;
               }
            if (f32Parms.fMessageActive & LOG_EAS)
               Message("Found %s", pSrcFea + 1);
            memcpy(pTarFea, pSrcFea, usFeaSize);
            }
         else
            {
            usFeaSize = sizeof (FEA) + (USHORT)pGea->cbName + 1;
            if (usFeaSize > usMaxSize)
               {
               rc = ERROR_BUFFER_OVERFLOW;
               if (pSrcFeal)
                  pTarFeal->cbList = pSrcFeal->cbList;
               else
                  pTarFeal->cbList = 4;
               goto usGetEASExit;
               }

            if (f32Parms.fMessageActive & LOG_EAS)
               Message("usGetEAS: %s not found!", pGea->szName);

            pTarFea->fEA = 0x00;
            pTarFea->cbName = pGea->cbName;
            pTarFea->cbValue = 0;
            strcpy((PBYTE)(pTarFea + 1), pGea->szName);
            }

         pTarFea = (PFEA)((PBYTE)pTarFea + usFeaSize);
         pTarFeal->cbList += usFeaSize;
         usMaxSize -= usFeaSize;

         pGea = (PGEA)((PBYTE)pGea + usGeaSize);
         }
      }
   else if (pSrcFeal)
      {
      PBYTE     pSrcMax = (PBYTE)pSrcFeal + pSrcFeal->cbList;
      pSrcFea = pSrcFeal->list;

      while ((PBYTE)pSrcFea + sizeof (FEA) < pSrcMax)
         {
         USHORT usFeaSize = sizeof (FEA) + (USHORT)pSrcFea->cbName + 1 + pSrcFea->cbValue;
         if (usFeaSize > usMaxSize)
            {
            rc = ERROR_BUFFER_OVERFLOW;
            pTarFeal->cbList = pSrcFeal->cbList;
            goto usGetEASExit;
            }
         if (f32Parms.fMessageActive & LOG_EAS)
            Message("Found %s (%u,%u)", pSrcFea + 1, (USHORT)pSrcFea->cbName, pSrcFea->cbValue);
         memcpy(pTarFea, pSrcFea, usFeaSize);
         pTarFea = (PFEA)((PBYTE)pTarFea + usFeaSize);
         pTarFeal->cbList += usFeaSize;
         pSrcFea = (PFEA)((PBYTE)pSrcFea + usFeaSize);
         usMaxSize -= usFeaSize;
         }
      }

   rc = 0;

usGetEASExit:

   if (pSrcFeal)
      freeseg(pSrcFeal);

   if (f32Parms.fMessageActive & LOG_EAS)
      Message("usGetEAS for %s returned %d (%lu bytes in EAS)",
         pszFileName, rc, pTarFeal->cbList);

   return rc;
}

/************************************************************************
*
************************************************************************/
USHORT usCopyEAS(PVOLINFO pVolInfo, ULONG ulSrcDirCluster, PSZ pszSrcFile, ULONG ulTarDirCluster, PSZ pszTarFile)
{
USHORT rc;
ULONG ulSrcCluster, ulTarCluster;
PSZ   pszSrcEAName = NULL,
      pszTarEAName = NULL;
DIRENTRY SrcEntry, TarEntry;

   rc = GetEASName(pVolInfo, ulSrcDirCluster, pszSrcFile, &pszSrcEAName);
   if (rc)
      goto usCopyEASExit;
   rc = GetEASName(pVolInfo, ulTarDirCluster, pszTarFile, &pszTarEAName);
   if (rc)
      goto usCopyEASExit;

   ulSrcCluster = FindPathCluster(pVolInfo, ulSrcDirCluster, pszSrcEAName, &SrcEntry, NULL);
   ulTarCluster = FindPathCluster(pVolInfo, ulTarDirCluster, pszTarEAName, &TarEntry, NULL);
   if (ulTarCluster != FAT_EOF)
      {
      rc = ModifyDirectory(pVolInfo, ulTarDirCluster, MODIFY_DIR_DELETE, &TarEntry, NULL, NULL, 0);
      if (rc)
         goto usCopyEASExit;
      DeleteFatChain(pVolInfo, ulTarCluster);
      }

   if (ulSrcCluster == FAT_EOF)
      goto usCopyEASExit;

   rc = CopyChain(pVolInfo, ulSrcCluster, &ulTarCluster);
   if (rc)
      goto usCopyEASExit;

   SrcEntry.wCluster = LOUSHORT(ulTarCluster);
   SrcEntry.wClusterHigh = HIUSHORT(ulTarCluster);

   /*
      Make new direntry
   */
   rc = ModifyDirectory(pVolInfo, ulTarDirCluster, MODIFY_DIR_INSERT, NULL, &SrcEntry, pszTarEAName, 0);


usCopyEASExit:
   if (pszSrcEAName)
      free(pszSrcEAName);
   if (pszTarEAName)
      free(pszTarEAName);

   if (f32Parms.fMessageActive & LOG_EAS)
      Message("usCopyEAS for returned %d", rc);

   return rc;
}

/************************************************************************
*
************************************************************************/
USHORT usMoveEAS(PVOLINFO pVolInfo, ULONG ulSrcDirCluster, PSZ pszSrcFile, ULONG ulTarDirCluster, PSZ pszTarFile)
{
USHORT rc;
ULONG ulSrcCluster, ulTarCluster;
PSZ   pszSrcEAName = NULL,
      pszTarEAName = NULL;
DIRENTRY SrcEntry, TarEntry;

   rc = GetEASName(pVolInfo, ulSrcDirCluster, pszSrcFile, &pszSrcEAName);
   if (rc)
      goto usMoveEASExit;
   rc = GetEASName(pVolInfo, ulTarDirCluster, pszTarFile, &pszTarEAName);
   if (rc)
      goto usMoveEASExit;


   ulSrcCluster = FindPathCluster(pVolInfo, ulSrcDirCluster, pszSrcEAName, &SrcEntry, NULL);
   ulTarCluster = FindPathCluster(pVolInfo, ulTarDirCluster, pszTarEAName, &TarEntry, NULL);
   if (ulTarCluster != FAT_EOF && ulTarCluster != ulSrcCluster)
      {
      rc = ModifyDirectory(pVolInfo, ulTarDirCluster, MODIFY_DIR_DELETE, &TarEntry, NULL, NULL, 0);
      if (rc)
         goto usMoveEASExit;
      DeleteFatChain(pVolInfo, ulTarCluster);
      }

   if (ulSrcCluster == FAT_EOF)
      goto usMoveEASExit;

   if (ulSrcDirCluster == ulTarDirCluster)
      {
      memmove(&TarEntry, &SrcEntry, sizeof TarEntry);
      rc = ModifyDirectory(pVolInfo, ulSrcDirCluster,
         MODIFY_DIR_RENAME, &SrcEntry, &TarEntry, pszTarEAName, 0);
      goto usMoveEASExit;
      }

   rc = ModifyDirectory(pVolInfo, ulSrcDirCluster, MODIFY_DIR_DELETE, &SrcEntry, NULL, NULL, 0);
   if (rc)
      goto usMoveEASExit;

   rc = ModifyDirectory(pVolInfo, ulTarDirCluster, MODIFY_DIR_INSERT, NULL, &SrcEntry, pszTarEAName, 0);

usMoveEASExit:
   if (pszSrcEAName)
      free(pszSrcEAName);
   if (pszTarEAName)
      free(pszTarEAName);

   if (f32Parms.fMessageActive & LOG_EAS)
      Message("usMoveEAS for returned %d", rc);

   return rc;
}

/************************************************************************
*
************************************************************************/
USHORT MarkFileEAS(PVOLINFO pVolInfo, ULONG ulDirCluster, PSZ pszFileName, BYTE fEAS)
{
ULONG ulCluster;
DIRENTRY OldEntry, NewEntry;
USHORT rc;


   ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszFileName, &OldEntry, NULL);
   if (ulCluster == FAT_EOF)
      {
      CritMessage("FAT32: MarkfileEAS : %s not found!", pszFileName);
      return ERROR_FILE_NOT_FOUND;
      }
   memcpy(&NewEntry, &OldEntry, sizeof (DIRENTRY));
   if( HAS_OLD_EAS( NewEntry.fEAS ))
        NewEntry.fEAS = FILE_HAS_NO_EAS;
   NewEntry.fEAS = ( BYTE )(( NewEntry.fEAS & FILE_HAS_WINNT_EXT ) | fEAS );

   if (!memcmp(&NewEntry, &OldEntry, sizeof (DIRENTRY)))
      return 0;

   rc = ModifyDirectory(pVolInfo, ulDirCluster,
      MODIFY_DIR_UPDATE, &OldEntry, &NewEntry, NULL, 0);

   return rc;
}

/************************************************************************
*
************************************************************************/
USHORT usReadEAS(PVOLINFO pVolInfo, ULONG ulDirCluster, PSZ pszFileName, PFEALIST * ppFEAL, BOOL fCreate)
{
PFEALIST pFEAL;
ULONG ulCluster;
PBYTE  pszEAName;
PBYTE pRead;
USHORT rc;
USHORT usClustersUsed;

   *ppFEAL = NULL;

   rc = GetEASName(pVolInfo, ulDirCluster, pszFileName, &pszEAName);
   if (rc)
      return rc;

   ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszEAName, NULL, NULL);
   free(pszEAName);

   if ((ulCluster && ulCluster != FAT_EOF) || fCreate)
      {
      pFEAL = gdtAlloc(MAX_EA_SIZE, FALSE);
      if (!pFEAL)
         return ERROR_NOT_ENOUGH_MEMORY;
      memset(pFEAL, 0, (size_t) MAX_EA_SIZE);
      pFEAL->cbList = sizeof (ULONG);
      }
   else
      pFEAL = NULL;

   if (!ulCluster || ulCluster == FAT_EOF)
      {
      *ppFEAL = pFEAL;
      return 0;
      }

   pRead = (PBYTE)pFEAL;
   if (f32Parms.fMessageActive & LOG_EAS)
      Message("usReadEAS: Reading (1) cluster %lu", ulCluster);

   rc = ReadCluster(pVolInfo, ulCluster, pRead, 0);
   if (rc)
      {
      freeseg(pFEAL);
      return rc;
      }
   if (pFEAL->cbList > MAX_EA_SIZE)
      {
      freeseg(pFEAL);
      return ERROR_EAS_DIDNT_FIT;
      }

   usClustersUsed = (USHORT)(pFEAL->cbList / pVolInfo->usClusterSize);
   if (pFEAL->cbList % pVolInfo->usClusterSize)
      usClustersUsed++;

   /*
      vreemd: zonder deze Messages lijkt deze routine mis te gaan.
      Optimalisatie?
   */
   if (f32Parms.fMessageActive & LOG_EAS)
      Message("usReadEAS: %u clusters used", usClustersUsed);

   usClustersUsed--;
   pRead += pVolInfo->usClusterSize;

   while (usClustersUsed)
      {
      ulCluster = GetNextCluster(pVolInfo, ulCluster);
      if (!ulCluster)
         ulCluster = FAT_EOF;
      if (ulCluster == FAT_EOF)
         {
         freeseg(pFEAL);
         return ERROR_EA_FILE_CORRUPT;
         }
   /*
      vreemd: zonder deze Messages lijkt deze routine mis te gaan.
      Optimalisatie?
   */
      if (f32Parms.fMessageActive & LOG_EAS)
         Message("usReadEAS: Reading (2) cluster %lu", ulCluster);

      rc = ReadCluster(pVolInfo, ulCluster, pRead, 0);
      if (rc)
         {
         freeseg(pFEAL);
         return rc;
         }
      usClustersUsed--;
      pRead += pVolInfo->usClusterSize;
      }
   *ppFEAL = pFEAL;

   return 0;
}

/************************************************************************
*
************************************************************************/
USHORT usDeleteEAS(PVOLINFO pVolInfo, ULONG ulDirCluster, PSZ pszFileName)
{
PSZ pszEAName;
USHORT rc;
DIRENTRY DirEntry;
ULONG    ulCluster;

   rc = GetEASName(pVolInfo, ulDirCluster, pszFileName, &pszEAName);
   if (rc)
      return rc;

   ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszEAName, &DirEntry, NULL);
   if (ulCluster == FAT_EOF)
      {
      rc = 0;
      goto usDeleteEASExit;
      }
   rc = ModifyDirectory(pVolInfo, ulDirCluster, MODIFY_DIR_DELETE, &DirEntry, NULL, NULL, 0);
   if (rc)
      goto usDeleteEASExit;

   if (ulCluster)
      DeleteFatChain(pVolInfo, ulCluster);

   rc = MarkFileEAS(pVolInfo, ulDirCluster, pszFileName, FILE_HAS_NO_EAS);

usDeleteEASExit:
   free(pszEAName);

   if (f32Parms.fMessageActive & LOG_EAS)
      Message("usDeleteEAS for %s returned %d",
         pszFileName, rc);

   return rc;
}

/************************************************************************
*
************************************************************************/
USHORT usWriteEAS(PVOLINFO pVolInfo, ULONG ulDirCluster, PSZ pszFileName, PFEALIST pFEAL)
{
ULONG ulCluster, ulNextCluster;
PBYTE  pszEAName;
PBYTE pWrite;
USHORT rc;
USHORT usClustersNeeded;
DIRENTRY DirEntry;
DIRENTRY DirNew;
BOOL     fCritical;
PFEA     pFea, pFeaEnd;

   if (pFEAL->cbList > MAX_EA_SIZE)
      return ERROR_EA_LIST_TOO_LONG;

   rc = GetEASName(pVolInfo, ulDirCluster, pszFileName, &pszEAName);
   if (rc)
      return rc;

   usClustersNeeded = (USHORT)pFEAL->cbList / pVolInfo->usClusterSize;
   if (pFEAL->cbList % pVolInfo->usClusterSize)
      usClustersNeeded++;

   ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszEAName, &DirEntry, NULL);
   if (!ulCluster || ulCluster == FAT_EOF)
      {
      BOOL fNew = FALSE;

      if (ulCluster == FAT_EOF)
         {
         fNew = TRUE;
         memset(&DirNew, 0, sizeof DirNew);
         }
      else
         memcpy(&DirNew, &DirEntry, sizeof DirEntry);

      DirNew.bAttr  = FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY;
      DirNew.ulFileSize = pFEAL->cbList;


      ulCluster = MakeFatChain(pVolInfo, FAT_EOF, (ULONG)usClustersNeeded, NULL);
      if (ulCluster == FAT_EOF)
         {
         free(pszEAName);
         return ERROR_DISK_FULL;
         }

      DirNew.wCluster = LOUSHORT(ulCluster);
      DirNew.wClusterHigh = HIUSHORT(ulCluster);

      if (fNew)
         rc = MakeDirEntry(pVolInfo, ulDirCluster, &DirNew, pszEAName);
      else
         rc = ModifyDirectory(pVolInfo, ulDirCluster,
            MODIFY_DIR_UPDATE, &DirEntry, &DirNew, NULL, 0);
      if (rc)
         {
         free(pszEAName);
         return rc;
         }
      }
   else
      {
      memcpy(&DirNew, &DirEntry, sizeof (DIRENTRY));
      DirNew.ulFileSize = pFEAL->cbList;
      rc = ModifyDirectory(pVolInfo, ulDirCluster, MODIFY_DIR_UPDATE,
           &DirEntry, &DirNew, NULL, 0);
      if (rc)
         {
         free(pszEAName);
         return rc;
         }
      }

   free(pszEAName);

   pWrite = (PBYTE)pFEAL;
   ulNextCluster = FAT_EOF;
   while (usClustersNeeded)
      {
      ulNextCluster = GetNextCluster(pVolInfo, ulCluster);
      if (!ulNextCluster)
         ulNextCluster = FAT_EOF;
      rc = WriteCluster(pVolInfo, ulCluster, pWrite, 0);
      if (rc)
         return rc;
      usClustersNeeded --;
      pWrite += pVolInfo->usClusterSize;

      if (usClustersNeeded)
         {
         if (ulNextCluster == FAT_EOF)
            ulCluster = MakeFatChain(pVolInfo, ulCluster, (ULONG)usClustersNeeded, NULL);
         else
            ulCluster = ulNextCluster;
         if (ulCluster == FAT_EOF)
            return ERROR_DISK_FULL;
         }
      }
   if (ulNextCluster != FAT_EOF)
      {
      SetNextCluster(pVolInfo, ulCluster, FAT_EOF);
      DeleteFatChain(pVolInfo, ulNextCluster);
      }

   pFea = pFEAL->list;
   pFeaEnd = (PFEA)((PBYTE)pFEAL + pFEAL->cbList);
   fCritical = FALSE;
   while (pFea < pFeaEnd)
      {
      if (pFea->fEA & FEA_NEEDEA)
         fCritical = TRUE;
      pFea = (PFEA)((PBYTE)pFea + sizeof (FEA) + (USHORT)pFea->cbName + 1 + pFea->cbValue);
      }


   if (fCritical)
      rc = MarkFileEAS(pVolInfo, ulDirCluster, pszFileName, FILE_HAS_CRITICAL_EAS);
   else
      rc = MarkFileEAS(pVolInfo, ulDirCluster, pszFileName, FILE_HAS_EAS);

   return rc;
}


/************************************************************************
*
************************************************************************/
PFEA FindEA(PFEALIST pFeal, PSZ pszName, USHORT usMaxName)
{
PFEA pFea;
PBYTE pMax;

   if (!pFeal)
      return NULL;

   pFea = pFeal->list;
   pMax = (PBYTE)pFeal + pFeal->cbList;

   while ((PBYTE)pFea + sizeof (FEA) < pMax)
      {
      PBYTE pName, pValue;

      pName  = (PBYTE)(pFea + 1);
      if (pName >= pMax)
         return NULL;
      pValue = pName + (USHORT)pFea->cbName + 1;
      if (pValue + pFea->cbValue > pMax)
         return NULL;
#if 0
      if (f32Parms.fMessageActive & LOG_EAS)
         Message("FindEA: '%s'", pName);
#endif
      if (pFea->cbName == (BYTE)usMaxName && !memicmp(pName, pszName, usMaxName))
         return pFea;

      pFea = (PFEA)((PBYTE)pFea + sizeof (FEA) + (USHORT)pFea->cbName + 1 + pFea->cbValue);
      }
   return NULL;
}

USHORT GetEASName(PVOLINFO pVolInfo, ULONG ulDirCluster, PSZ pszFileName, PSZ * pszEASName)
{
   if (strlen(pszFileName) > FAT32MAXPATH - 4)
      return ERROR_FILENAME_EXCED_RANGE;

   *pszEASName = malloc(FAT32MAXPATH);
   if (!(*pszEASName))
      return ERROR_NOT_ENOUGH_MEMORY;

   if( TranslateName( pVolInfo, ulDirCluster, pszFileName, *pszEASName, TRANSLATE_SHORT_TO_LONG ))
       strcpy(*pszEASName, pszFileName);

   strcat(*pszEASName, EA_EXTENTION);
   return 0;
}

BOOL IsEASFile(PSZ pszFileName)
{
USHORT usExtLen = strlen(EA_EXTENTION);

   if (strlen(pszFileName) > usExtLen)
      {
      if (!stricmp(pszFileName + (strlen(pszFileName) - usExtLen), EA_EXTENTION))
         return TRUE;
      }
   return FALSE;
}

USHORT usGetEmptyEAS(PSZ pszFileName, PEAOP pEAOP)
{
   USHORT rc;

   PFEALIST pTarFeal;
   USHORT usMaxSize;
   PFEA     pCurrFea;

   PGEALIST pGeaList;
   PGEA     pCurrGea;

   ULONG    ulGeaSize;
   ULONG    ulFeaSize;
   ULONG    ulCurrFeaLen;
   ULONG    ulCurrGeaLen;

   if (f32Parms.fMessageActive & LOG_EAS)
      Message("usGetEmptyEAS for %s with pEAOP %lX", pszFileName,pEAOP);

   /*
      Checking all the arguments
   */

   rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pEAOP, sizeof (EAOP));
   if (rc)
      {
      Message("Protection violation in usGetEmptyEAS (1) at %lX", pEAOP);
      return rc;
      }

   pTarFeal = pEAOP->fpFEAList;
   rc = MY_PROBEBUF(PB_OPREAD, (PBYTE)pTarFeal, sizeof (ULONG));
   if (rc)
      {
      Message("Protection violation in usGetEmptyEAS (2) at %lX", pTarFeal);
      return rc;
      }
   if (pTarFeal->cbList > MAX_EA_SIZE)
      usMaxSize = (USHORT)MAX_EA_SIZE;
   else
      usMaxSize = (USHORT)pTarFeal->cbList;

   if (usMaxSize < sizeof (ULONG))
      return ERROR_BUFFER_OVERFLOW;

   rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pTarFeal, (USHORT)usMaxSize);
   if (rc)
      return rc;

   pGeaList = pEAOP->fpGEAList;
   rc = MY_PROBEBUF(PB_OPREAD, (PBYTE)pGeaList, sizeof (ULONG));
   if (rc)
      return rc;
   if (pGeaList->cbList > MAX_EA_SIZE)
      return ERROR_EA_LIST_TOO_LONG;
   rc = MY_PROBEBUF(PB_OPREAD, (PBYTE)pGeaList, (USHORT)pGeaList->cbList);
   if (rc)
      return rc;

   ulFeaSize = sizeof(pTarFeal->cbList);
   ulGeaSize = sizeof(pGeaList->cbList);

   pCurrGea = pGeaList->list;
   pCurrFea = pTarFeal->list;
   while(ulGeaSize < pGeaList->cbList)
      {
      ulFeaSize += sizeof(FEA) + pCurrGea->cbName + 1;
      ulCurrGeaLen = sizeof(GEA) + pCurrGea->cbName;
      pCurrGea = (PGEA)((PBYTE)pCurrGea + ulCurrGeaLen);
      ulGeaSize += ulCurrGeaLen;
      }

   if (ulFeaSize > usMaxSize)
      {
      /* this is what HPFS.IFS returns */
      /* when a file does not have any EAs */
      pTarFeal->cbList = 0xEF;
      rc = ERROR_EAS_DIDNT_FIT;
      }
   else
      {
       /* since we DO copy something to */
       /* FEALIST, we have to set the complete */
       /* size of the resulting FEALIST in the */
       /* length field */
       pTarFeal->cbList = ulFeaSize;
       ulGeaSize = sizeof(pGeaList->cbList);
       pCurrGea = pGeaList->list;
       pCurrFea = pTarFeal->list;
       /* copy the EA names requested to the FEA area */
       /* even if any values cannot be returned       */
       while (ulGeaSize < pGeaList->cbList)
          {
          pCurrFea->fEA     = 0;
          strcpy((PBYTE)(pCurrFea+1),pCurrGea->szName);
          pCurrFea->cbName  = (BYTE)strlen(pCurrGea->szName);
          pCurrFea->cbValue = 0;

          ulCurrFeaLen = sizeof(FEA) + pCurrFea->cbName + 1;
          pCurrFea = (PFEA)((PBYTE)pCurrFea + ulCurrFeaLen);

          ulCurrGeaLen = sizeof(GEA) + pCurrGea->cbName;
          pCurrGea = (PGEA)((PBYTE)pCurrGea + ulCurrGeaLen);
          ulGeaSize += ulCurrGeaLen;
          }
       rc = 0;
       }
   return rc;
}

