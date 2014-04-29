#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>

#define INCL_DOS
#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_DOSERRORS

#include "os2.h"
#include "portable.h"
#include "fat32ifs.h"

PUBLIC BYTE  pascal FS_NAME[]    ="FAT32";
PUBLIC ULONG pascal FS_ATTRIBUTE = FSA_LVL7;

PUBLIC ULONG     Device_Help = 0L;
PUBLIC PVOLINFO  pGlobVolInfo = NULL;
PUBLIC ULONG     ulCacheSectors=2048;
PUBLIC USHORT    usDefaultRASectors = 0xFFFF;
PUBLIC BYTE      szArguments[512] = "";
PUBLIC PGINFOSEG pGI = NULL;
PUBLIC PULONG    pGITicks = NULL;
PUBLIC F32PARMS  f32Parms = {0};

static BYTE szDiskLocked[]="The disk is in use or locked by another process.\r\n";
static BYTE rgValidChars[]="01234567890 ABCDEFGHIJKLMNOPQRSTUVWXYZ!#$%&'()-_@^`{}~";
static ULONG ulSemRWFat = 0;
static SEL sGlob = 0;
static SEL sLoc = 0;

static BYTE szBanner[]=
"FAT32.IFS version " FAT32_VERSION " " __DATE__ "\r\n"
"Made by Henk Kelder + Netlabs\r\n";

static ULONG GetFreeCluster(PVOLINFO pVolInfo);
static PDIRENTRY fSetLongName(PDIRENTRY pDir, PSZ pszName, BYTE bCheck);
static ULONG GetNextCluster2(PVOLINFO pVolInfo, ULONG ulCluster);
static ULONG SetNextCluster2(PVOLINFO pVolInfo, ULONG ulCluster, ULONG ulNext);
static PDIRENTRY CompactDir(PDIRENTRY pStart, USHORT usSize, USHORT usNeededEntries);
static USHORT GetFreeEntries(PDIRENTRY pDirBlock, USHORT usSize);
static VOID MarkFreeEntries(PDIRENTRY pDirBlock, USHORT usSize);
static USHORT GetFatAccess(PVOLINFO pVolInfo, PSZ pszName);
static VOID   ReleaseFat(PVOLINFO pVolInfo);
static USHORT RecoverChain(PVOLINFO pVolInfo, ULONG ulCluster, PBYTE pData, USHORT cbData);
static USHORT WriteFatSector(PVOLINFO pVolInfo, ULONG ulSector);
static USHORT ReadFatSector(PVOLINFO pVolInfo, ULONG ulSector);
static ULONG  GetVolDevice(PVOLINFO pVolInfo);
static USHORT SetFileSize(PVOLINFO pVolInfo, PFILESIZEDATA pFileSize);
static ULONG GetChainSize(PVOLINFO pVolInfo, ULONG ulCluster);
static VOID InitMessage(PSZ pszMessage);
static USHORT MakeChain(PVOLINFO pVolInfo, ULONG ulFirstCluster, ULONG ulSize);
static USHORT GetSetFileEAS(PVOLINFO pVolInfo, USHORT usFunc, PMARKFILEEASBUF pMark);
static USHORT DBCSStrlen( const PSZ pszStr );

/******************************************************************
*
******************************************************************/
int far pascal __loadds FS_ATTACH(unsigned short usFlag,     /* flag     */
                         char far * pDev,           /* pDev     */
                         void far * pvpfsd,         /* if remote drive
                                                 struct vpfsd far *
                                                   else if remote device
                                                    null ptr (0L)    */
                         void far * pdevfsd,            /* if remote drive
                                                   struct cdfsd far *
                                                   else
                                                    struct devfsd far * */
                         char far * pParm,          /* pParm    */
                         unsigned short far * pLen) /* pLen     */
{
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_ATTACH - NOT SUPPORTED");
   usFlag = usFlag;
   pDev = pDev;
   pvpfsd = pvpfsd;
   pdevfsd = pdevfsd;
   pParm = pParm;
   pLen = pLen;
   return ERROR_NOT_SUPPORTED;
}

/******************************************************************
*
******************************************************************/
int far pascal __loadds FS_COPY(
    unsigned short usMode,      /* copy mode    */
    struct cdfsi far * pcdfsi,      /* pcdfsi   */
    struct cdfsd far * pcdfsd,      /* pcdfsd   */
    char far * pSrc,            /* source name  */
    unsigned short usSrcCurDirEnd,      /* iSrcCurrDirEnd   */
    char far * pDst,            /* pDst     */
    unsigned short usDstCurDirEnd,      /* iDstCurrDirEnd   */
    unsigned short usNameType       /* nameType (flags) */
)
{
PVOLINFO pVolInfo;
ULONG ulSrcDirCluster;
ULONG ulDstDirCluster;
PSZ   pszSrcFile;
PSZ   pszDstFile;
DIRENTRY DirEntry;
DIRENTRY TarEntry;
ULONG    ulSrcCluster;
ULONG    ulDstCluster;
USHORT   rc, rc2;
POPENINFO pOpenInfo = NULL;
BYTE     szSrcLongName[ FAT32MAXPATH ];
BYTE     szDstLongName[ FAT32MAXPATH ];

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_COPY %s to %s, mode %d", pSrc, pDst, usMode);

   pVolInfo = GetVolInfo(pcdfsi->cdi_hVPB);
   pVolInfo->ulOpenFiles++;

   pOpenInfo = malloc(sizeof (OPENINFO));
   if (!pOpenInfo)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      goto FS_COPYEXIT;
      }
   memset(pOpenInfo, 0, sizeof (OPENINFO));

   /*
      Not on the same drive: cannot copy
   */
   if (*pSrc != *pDst)
      {
      rc = ERROR_CANNOT_COPY;
      goto FS_COPYEXIT;
      }

   if( TranslateName(pVolInfo, 0L, pSrc, szSrcLongName, TRANSLATE_SHORT_TO_LONG ))
      strcpy( szSrcLongName, pSrc );

   if( TranslateName(pVolInfo, 0L, pDst, szDstLongName, TRANSLATE_SHORT_TO_LONG ))
      strcpy( szDstLongName, pDst );

   if (!stricmp( szSrcLongName, szDstLongName ))
      {
      rc = ERROR_CANNOT_COPY;
      goto FS_COPYEXIT;
      }

   pOpenInfo->pSHInfo = GetSH( szDstLongName, pOpenInfo);
   if (!pOpenInfo->pSHInfo)
      {
      rc = ERROR_TOO_MANY_OPEN_FILES;
      goto FS_COPYEXIT;
      }
   pOpenInfo->pSHInfo->sOpenCount++;
   if (pOpenInfo->pSHInfo->sOpenCount > 1)
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_COPYEXIT;
      }
   pOpenInfo->pSHInfo->fLock = TRUE;

   usNameType = usNameType;

   if (IsDriveLocked(pVolInfo))
      {
      rc =ERROR_DRIVE_LOCKED;
      goto FS_COPYEXIT;
      }
   if (!pVolInfo->fDiskCleanOnMount)
      {
      rc = ERROR_VOLUME_DIRTY;
      goto FS_COPYEXIT;
      }
   if (pVolInfo->fWriteProtected)
      {
      rc = ERROR_WRITE_PROTECT;
      goto FS_COPYEXIT;
      }

   /*
      Check source
   */

   ulSrcDirCluster = FindDirCluster(pVolInfo,
      pcdfsi,
      pcdfsd,
      pSrc,
      usSrcCurDirEnd,
      RETURN_PARENT_DIR,
      &pszSrcFile);
   if (ulSrcDirCluster == FAT_EOF)
      {
      rc = ERROR_PATH_NOT_FOUND;
      goto FS_COPYEXIT;
      }

   ulSrcCluster = FindPathCluster(pVolInfo, ulSrcDirCluster, pszSrcFile, &DirEntry, NULL);
   if (ulSrcCluster == FAT_EOF)
      {
      rc = ERROR_FILE_NOT_FOUND;
      goto FS_COPYEXIT;
      }
   /*
      Do not allow directories to be copied
   */
   if (DirEntry.bAttr & FILE_DIRECTORY)
      {
      int iLen = strlen( szSrcLongName );

      if ( !strnicmp(szSrcLongName, szDstLongName, iLen ) &&
           ( szDstLongName[ iLen ] == '\\' ))
      {
        rc = ERROR_DIRECTORY;
        goto FS_COPYEXIT;
      }

      rc = ERROR_CANNOT_COPY;
      goto FS_COPYEXIT;
      }


   if (ulSrcCluster == pVolInfo->BootSect.bpb.RootDirStrtClus)
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_COPYEXIT;
      }

   /*
      Check destination
   */

   ulDstDirCluster = FindDirCluster(pVolInfo,
      pcdfsi,
      pcdfsd,
      pDst,
      usDstCurDirEnd,
      RETURN_PARENT_DIR,
      &pszDstFile);
   if (ulDstDirCluster == FAT_EOF)
      {
      rc = ERROR_PATH_NOT_FOUND;
      goto FS_COPYEXIT;
      }

   pszDstFile = strrchr( szDstLongName, '\\' ) + 1; /* To preserve long name */

   ulDstCluster = FindPathCluster(pVolInfo, ulDstDirCluster, pszDstFile, &TarEntry, NULL);
   if (ulDstCluster != FAT_EOF)
      {
      if (TarEntry.bAttr & FILE_DIRECTORY)
         {
         rc = ERROR_CANNOT_COPY;
         goto FS_COPYEXIT;
         }
      if (usMode == DCPY_APPEND)
         {
         rc = ERROR_CANNOT_COPY;
         goto FS_COPYEXIT;
         }

      /*
         Delete target
      */
      if (f32Parms.fEAS)
         {
         rc = usDeleteEAS(pVolInfo, ulDstDirCluster, pszDstFile);
         if (rc)
            goto FS_COPYEXIT;
#if 0
         if (TarEntry.fEAS == FILE_HAS_EAS || TarEntry.fEAS == FILE_HAS_CRITICAL_EAS)
            TarEntry.fEAS = FILE_HAS_NO_EAS;
#endif
         }

      rc = ModifyDirectory(pVolInfo, ulDstDirCluster, MODIFY_DIR_DELETE, &TarEntry, NULL, NULL, 0);
      if (rc)
         goto FS_COPYEXIT;

      if (!DeleteFatChain(pVolInfo, ulDstCluster))
         {
         rc = ERROR_FILE_NOT_FOUND;
         goto FS_COPYEXIT;
         }
      }

   /*
      Make new direntry
   */

   memcpy(&TarEntry, &DirEntry, sizeof TarEntry);
   TarEntry.wCluster = 0;
   TarEntry.wClusterHigh = 0;
   TarEntry.ulFileSize = 0L;
   rc = ModifyDirectory(pVolInfo, ulDstDirCluster, MODIFY_DIR_INSERT, NULL, &TarEntry, pszDstFile, 0);
   if (rc)
      goto FS_COPYEXIT;
   memcpy(DirEntry.bFileName, TarEntry.bFileName, 11);

   /*
      Do the copying
   */
   rc = CopyChain(pVolInfo, ulSrcCluster, &ulDstCluster);

   if (ulDstCluster != FAT_EOF)
      {
      DirEntry.wCluster = LOUSHORT(ulDstCluster);
      DirEntry.wClusterHigh = HIUSHORT(ulDstCluster);
      }
   else
      {
      DirEntry.wCluster = 0;
      DirEntry.wClusterHigh = 0;
      DirEntry.ulFileSize = 0L;
      }
   /*
      modify new entry
   */
   rc2 = ModifyDirectory(pVolInfo, ulDstDirCluster, MODIFY_DIR_UPDATE, &TarEntry, &DirEntry, NULL, 0);
   if (rc2 && !rc)
      rc = rc2;

   if (rc)
      goto FS_COPYEXIT;

   if (f32Parms.fEAS)
      {
      rc = usCopyEAS(pVolInfo, ulSrcDirCluster, pszSrcFile,
                               ulDstDirCluster, pszDstFile);
      }

FS_COPYEXIT:

   pVolInfo->ulOpenFiles--;

   if (pOpenInfo)
      {
      if (pOpenInfo->pSHInfo)
         ReleaseSH(pOpenInfo);
      else
         free(pOpenInfo);
      }

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_COPY returned %u", rc);
   return rc;
}

/******************************************************************
*
******************************************************************/
int far pascal  __loadds FS_DELETE(
    struct cdfsi far * pcdfsi,      /* pcdfsi   */
    struct cdfsd far * pcdfsd,      /* pcdfsd   */
    char far * pFile,           /* pFile    */
    unsigned short usCurDirEnd      /* iCurDirEnd   */
)
{
PVOLINFO pVolInfo;
ULONG    ulCluster;
ULONG    ulDirCluster;
PSZ      pszFile;
USHORT   rc;
DIRENTRY DirEntry;
POPENINFO pOpenInfo;
BYTE     szLongName[ FAT32MAXPATH ];

   if (f32Parms.fMessageActive & LOG_FS)
    Message("FS_DELETE for %s", pFile);

   pVolInfo = GetVolInfo(pcdfsi->cdi_hVPB);

   if (IsDriveLocked(pVolInfo))
      return ERROR_DRIVE_LOCKED;
   if (!pVolInfo->fDiskCleanOnMount)
      return ERROR_VOLUME_DIRTY;
   if (pVolInfo->fWriteProtected)
      return ERROR_WRITE_PROTECT;

   pOpenInfo = malloc(sizeof (OPENINFO));
   if (!pOpenInfo)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      goto FS_DELETEEXIT;
      }

   if( TranslateName(pVolInfo, 0L, pFile, szLongName, TRANSLATE_SHORT_TO_LONG ))
      strcpy( szLongName, pFile );

   pOpenInfo->pSHInfo = GetSH( szLongName, pOpenInfo);
   if (!pOpenInfo->pSHInfo)
      {
      rc = ERROR_TOO_MANY_OPEN_FILES;
      goto FS_DELETEEXIT;
      }
   pOpenInfo->pSHInfo->sOpenCount++;
   if (pOpenInfo->pSHInfo->sOpenCount > 1)
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_DELETEEXIT;
      }
   pOpenInfo->pSHInfo->fLock = TRUE;

   if (strlen(pFile) > FAT32MAXPATH)
      {
      rc = ERROR_FILENAME_EXCED_RANGE;
      goto FS_DELETEEXIT;
      }

   ulDirCluster = FindDirCluster(pVolInfo,
      pcdfsi,
      pcdfsd,
      pFile,
      usCurDirEnd,
      RETURN_PARENT_DIR,
      &pszFile);
   if (ulDirCluster == FAT_EOF)
      {
      rc = ERROR_PATH_NOT_FOUND;
      goto FS_DELETEEXIT;
      }

   ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszFile, &DirEntry, NULL);
   if (ulCluster == FAT_EOF)
      {
      rc = ERROR_FILE_NOT_FOUND;
      goto FS_DELETEEXIT;
      }


   if (DirEntry.bAttr & FILE_DIRECTORY)
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_DELETEEXIT;
      }

   if (DirEntry.bAttr & FILE_READONLY)
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_DELETEEXIT;
      }

   if (f32Parms.fEAS)
      {
      rc = usDeleteEAS(pVolInfo, ulDirCluster, pszFile);
      if (rc)
         goto FS_DELETEEXIT;
#if 0
      if (DirEntry.fEAS == FILE_HAS_EAS || DirEntry.fEAS == FILE_HAS_CRITICAL_EAS)
         DirEntry.fEAS = FILE_HAS_NO_EAS;
#endif
      }

   rc = ModifyDirectory(pVolInfo, ulDirCluster, MODIFY_DIR_DELETE, &DirEntry, NULL, NULL, 0);
   if (rc)
      goto FS_DELETEEXIT;

   if (ulCluster)
      DeleteFatChain(pVolInfo, ulCluster);
   rc = 0;


FS_DELETEEXIT:

   if (pOpenInfo)
      {
      if (pOpenInfo->pSHInfo)
         ReleaseSH(pOpenInfo);
      else
         free(pOpenInfo);
      }

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_DELETE returned %u", rc);

   return rc;
}

/******************************************************************
*
******************************************************************/
void far pascal  __loadds FS_EXIT(
    unsigned short usUid,       /* uid      */
    unsigned short usPid,       /* pid      */
    unsigned short usPdb        /* pdb      */
)
{
PVOLINFO pVolInfo = pGlobVolInfo;
PFINFO pFindInfo;
USHORT rc;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_EXIT for PID: %X, PDB %X",
         usPid, usPdb);

   while (pVolInfo)
      {
      rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pVolInfo, sizeof (VOLINFO));
      if (rc)
         {
         FatalMessage("FAT32: Protection VIOLATION (Volinfo) in FS_EXIT! (SYS%d)", rc);
         return;
         }

      if (pVolInfo->fLocked &&
          pVolInfo->ProcLocked.usPid == usPid &&
          pVolInfo->ProcLocked.usUid == usUid &&
          pVolInfo->ProcLocked.usPdb == usPdb)
         pVolInfo->fLocked = FALSE;

      pFindInfo = (PFINFO)pVolInfo->pFindInfo;
      while (pFindInfo)
         {
         if (f32Parms.fMessageActive & LOG_FUNCS)
            Message("Still findinfo's allocated");
         rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pFindInfo, sizeof (FINFO));
         if (rc)
            {
            FatalMessage("FAT32: Protection VIOLATION (FindInfo) in FS_EXIT! (SYS%d)", rc);
            Message("FAT32: Protection VIOLATION (FindInfo) in FS_EXIT! (SYS%d)", rc);
            return;
            }

         if (pFindInfo->ProcInfo.usPid == usPid &&
             pFindInfo->ProcInfo.usUid == usUid &&
             pFindInfo->ProcInfo.usPdb == usPdb)
            {
            if (f32Parms.fMessageActive & LOG_FUNCS)
               Message("Removing a FINDINFO");
            if (RemoveFindEntry(pVolInfo, pFindInfo))
               free(pFindInfo);
            pFindInfo = (PFINFO)pVolInfo->pFindInfo;
            }
         else
            pFindInfo = (PFINFO)pFindInfo->pNextEntry;
         }
      pVolInfo = (PVOLINFO)pVolInfo->pNextVolInfo;
      }

   return ;
}



/******************************************************************
*
******************************************************************/
int far pascal  __loadds FS_FLUSHBUF(
    unsigned short hVPB,        /* hVPB     */
    unsigned short usFlag       /* flag     */
)
{
PVOLINFO pVolInfo = GetVolInfo(hVPB);
USHORT rc;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FLUSHBUF, flag = %d", usFlag);

   if (pVolInfo->fWriteProtected)
      return 0;

   rc = usFlushVolume(pVolInfo, usFlag, TRUE, PRIO_URGENT);

   if (rc)
      return rc;

   if (!f32Parms.usDirtySectors) // vs
      goto FS_FLUSHEXIT;         //

   if (!UpdateFSInfo(pVolInfo))
      {
      rc = ERROR_SECTOR_NOT_FOUND;
      goto FS_FLUSHEXIT;
      }

   if (!MarkDiskStatus(pVolInfo, TRUE))
      {
      rc = ERROR_SECTOR_NOT_FOUND;
      goto FS_FLUSHEXIT;
      }
FS_FLUSHEXIT:

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FLUSHBUF returned %u", rc);
   return rc;
}


/******************************************************************
*
******************************************************************/
int far pascal  __loadds FS_FSCTL(
    union argdat far * pArgDat,     /* pArgdat  */
    unsigned short usArgType,       /* iArgType */
    unsigned short usFunc,      /* func     */
    char far * pParm,           /* pParm    */
    unsigned short cbParm,      /* lenParm  */
    unsigned short far * pcbParm,   /* plenParmOut  */
    char far * pData,           /* pData    */
    unsigned short cbData,      /* lenData  */
    unsigned short far * pcbData    /* plenDataOut  */
)
{
USHORT rc;
POPENINFO pOpenInfo;


   if (usFunc != FAT32_GETLOGDATA && f32Parms.fMessageActive & LOG_FS)
      Message("FS_FSCTL, Func = %Xh", usFunc);

   rc = 0;
   if (pData && pData != MYNULL)
      {
      if (cbData)
         {
         rc = MY_PROBEBUF(PB_OPWRITE, pData, cbData);
         if (rc)
            {
            Message("Protection VIOLATION in Data of FS_FSCTL!");
            return rc;
            }
         }
      if (pcbData)
         {
         rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pcbData, sizeof (USHORT));
         if (rc)
            pcbData = NULL;
         }
      if (pcbData)
         *pcbData = cbData;
      }
   else
      {
      cbData = 0;
      pcbData = NULL;
      }

   if (pParm && pParm != MYNULL)
      {
      if (cbParm)
         {
         rc = MY_PROBEBUF(PB_OPREAD, pParm, cbParm);
         if (rc)
            {
            Message("Protection VIOLATION in Parm of FS_FSCTL!");
            return rc;
            }
         }
      if (pcbParm)
         {
         rc = MY_PROBEBUF(PB_OPREAD, (PBYTE)pcbParm, sizeof (USHORT));
         if (rc)
            pcbParm = NULL;
         }
      if (pcbParm)
         *pcbParm = cbParm;
      }
   else
      {
      cbParm = 0;
      pcbParm = NULL;
      }


   switch (usFunc)
      {
      case FSCTL_FUNC_NEW_INFO:
         if (cbData > 15)
            {
            strcpy(pData, "Unknown error");
            if (pcbData)
               *pcbData = strlen(pData) + 1;
            rc = 0;
            }
         else
            {
            if (pcbData)
               *pcbData = 15;
            rc = ERROR_BUFFER_OVERFLOW;
            }
         break;

      case FSCTL_FUNC_EASIZE:
         {
         PEASIZEBUF pEA = (PEASIZEBUF)pData;
         if (pcbData)
            *pcbData = sizeof (EASIZEBUF);
         if (cbData < sizeof (EASIZEBUF))
            {
            rc = ERROR_BUFFER_OVERFLOW;
            goto FS_FSCTLEXIT;
            }
         if (f32Parms.fEAS)
            {
            pEA->cbMaxEASize = MAX_EA_SIZE - sizeof (ULONG) - sizeof (FEA);
            pEA->cbMaxEAListSize = MAX_EA_SIZE;
            }
         else
            {
            pEA->cbMaxEASize = 0;
            pEA->cbMaxEAListSize = 0;
            }
         rc = 0;
         break;
         }

      case FSCTL_DAEMON_QUERY:
         rc = ERROR_INVALID_FUNCTION;
         goto FS_FSCTLEXIT;
#if 0
         {
         PFSDDAEMON pDaemon = (PFSDDAEMON)pData;
         if (cbData != sizeof (PFSDDAEMON))
            {
            rc = ERROR_BUFFER_OVERFLOW;
            goto FS_FSCTLEXIT;
            }
         memset(pDaemon, 0, sizeof (PFSDDAEMON));
         pDaemon->usNumThreads = 1;
         pDaemon->tdThrds[0].usFunc = FAT32_DOLW;
         pDaemon->tdThrds[0].usStackSize = 2028;
         pDaemon->tdThrds[0].ulPriorityClass = PRTYC_IDLETIME;
         pDaemon->tdThrds[0].lPriorityLevel = 0;
         rc = 0;
         }
         break;
#endif

      case FAT32_SECTORIO:
         if (usArgType != 1)
            {
            rc = ERROR_INVALID_FUNCTION;
            goto FS_FSCTLEXIT;
            }

         if (!(pArgDat->sf.psffsi->sfi_mode & OPEN_FLAGS_DASD))
            {
            rc = ERROR_INVALID_FUNCTION;
            goto FS_FSCTLEXIT;
            }
         if (cbParm < sizeof (ULONG))
            {
            rc = ERROR_INSUFFICIENT_BUFFER;
            goto FS_FSCTLEXIT;
            }
         if (*(PULONG)pParm != 0xDEADFACE)
            {
            rc = ERROR_INVALID_PARAMETER;
            goto FS_FSCTLEXIT;
            }
         pOpenInfo = GetOpenInfo(pArgDat->sf.psffsd);
         pOpenInfo->fSectorMode = TRUE;
         rc = 0;
         break;

      case FAT32_GETLOGDATA:
         if (f32Parms.fInShutDown)
            {
            rc = ERROR_ALREADY_SHUTDOWN;
            goto FS_FSCTLEXIT;
            }
         rc = GetLogBuffer(pData, cbData, *(PULONG)pParm);
         break;

      case FAT32_SETMESSAGE:
         if (cbParm < sizeof (USHORT))
            {
            rc = ERROR_INSUFFICIENT_BUFFER;
            goto FS_FSCTLEXIT;
            }
         f32Parms.fMessageActive = *(BOOL *)pParm;
         rc = 0;
         break;

      case FAT32_STARTLW:
         if (f32Parms.fLW || ( !f32Parms.usCacheSize && !f32Parms.fForceLoad ))
            {
            rc = ERROR_INVALID_FUNCTION;
            goto FS_FSCTLEXIT;
            }

         f32Parms.fLW = TRUE;
         Message("Lazy writing is ON");
         rc = 0;
         break;

      case FAT32_STOPLW:
         if (!f32Parms.fLW)
            {
            rc = ERROR_INVALID_FUNCTION;
            goto FS_FSCTLEXIT;
            }

         f32Parms.fLW = FALSE;
         Message("Lazy writing is OFF");
         rc = usFlushAll();
         break;

      case FAT32_DOLW:
         if (cbParm < sizeof (LWOPTS))
            {
            rc = ERROR_INSUFFICIENT_BUFFER;
            goto FS_FSCTLEXIT;
            }
         DoLW(NULL, (PLWOPTS)pParm);
         rc = 0;
         break;

      case FAT32_EMERGTHREAD:
         if (cbParm < sizeof (LWOPTS))
            {
            rc = ERROR_INSUFFICIENT_BUFFER;
            goto FS_FSCTLEXIT;
            }
         DoEmergencyFlush((PLWOPTS)pParm);
         rc = 0;
         break;

      case FAT32_SETPARMS:
         if (cbParm > sizeof (F32PARMS))
            {
            rc = ERROR_INSUFFICIENT_BUFFER;
            goto FS_FSCTLEXIT;
            }
         f32Parms.ulDiskIdle       = ((PF32PARMS)pParm)->ulDiskIdle;
         f32Parms.ulBufferIdle     = ((PF32PARMS)pParm)->ulBufferIdle;
         f32Parms.ulMaxAge         = ((PF32PARMS)pParm)->ulMaxAge;
         f32Parms.fMessageActive   = ((PF32PARMS)pParm)->fMessageActive;
         f32Parms.ulCurCP          = ((PF32PARMS)pParm)->ulCurCP;
         f32Parms.fForceLoad       = ((PF32PARMS)pParm)->fForceLoad;

         /*
            Codepage is changed only by FAT32_SETTRANSTABLE,
            so we don't worry about not modifying DBCS lead byte info here.
         */

         rc = 0;
         break;

      case FAT32_GETPARMS:
         if (pcbData)
            *pcbData = sizeof f32Parms;
         if (cbData < sizeof (F32PARMS))
            return ERROR_BUFFER_OVERFLOW;
         memcpy(pData, &f32Parms, sizeof (F32PARMS));
         rc = 0;
         break;

      case FAT32_SETTRANSTABLE:
         if (!cbParm)
            f32Parms.fTranslateNames = FALSE;
         else
            {
#if 0
            if (cbParm != 512)
               {
               rc = ERROR_INSUFFICIENT_BUFFER;
               goto FS_FSCTLEXIT;
               }
#endif
            if( !TranslateInit(pParm, cbParm))
                {
                rc = ERROR_INVALID_PARAMETER;
                goto FS_FSCTLEXIT;
                }
            }
         rc = 0;
         break;

      case FAT32_WIN2OS:
         Translate2OS2((PUSHORT)pParm, pData, cbData);
         rc = 0;
         break;

      case FAT32_QUERYSHORTNAME:
         {
         char szShortPath[ FAT32MAXPATH ] = { 0, };
         PVOLINFO pVolInfo = pGlobVolInfo;
         BYTE     bDrive;

         bDrive = *pParm;
         if (bDrive >= 'a' && bDrive <= 'z')
            bDrive -= ('a' - 'A');
         bDrive -= 'A';
         while (pVolInfo)
            {
            if (pVolInfo->bDrive == bDrive)
               break;
            pVolInfo = (PVOLINFO)pVolInfo->pNextVolInfo;
            }
         if (pVolInfo)
         {
            TranslateName(pVolInfo, 0L, (PSZ)pParm, szShortPath, TRANSLATE_LONG_TO_SHORT);
            if( strlen( szShortPath ) >= cbData )
            {
                rc = ERROR_BUFFER_OVERFLOW;
                goto FS_FSCTLEXIT;
            }

            strcpy((PSZ)pData, szShortPath );
         }

         rc = 0;
         break;
         }

      case FAT32_GETCASECONVERSION:
        if( cbData < 256 )
        {
            rc = ERROR_BUFFER_OVERFLOW;
            goto FS_FSCTLEXIT;
        }

        GetCaseConversion( pData );

        rc = 0;
        break;

      case FAT32_GETFIRSTINFO:
        if( cbData < sizeof( UCHAR ) * 256 )
        {
            rc = ERROR_BUFFER_OVERFLOW;
            goto FS_FSCTLEXIT;
        }

        GetFirstInfo(( PBOOL )pData );
        rc = 0;
        break;

      default :
         rc = ERROR_INVALID_FUNCTION;
         break;
      }

FS_FSCTLEXIT:

   if (usFunc != FAT32_GETLOGDATA && f32Parms.fMessageActive & LOG_FS)
      Message("FS_FSCTL returned %u", rc);
   return rc;
}

/******************************************************************
*
******************************************************************/
int far pascal __loadds FS_FSINFO(
    unsigned short usFlag,      /* flag     */
    unsigned short hVBP,        /* hVPB     */
    char far * pData,           /* pData    */
    unsigned short cbData,      /* cbData   */
    unsigned short usLevel      /* level    */
)
{
PVOLINFO pVolInfo;
USHORT rc;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FSINFO, Flag = %d, Level = %d", usFlag, usLevel);

   pVolInfo = GetVolInfo(hVBP);

   if (IsDriveLocked(pVolInfo))
      return ERROR_DRIVE_LOCKED;

   if (pVolInfo->fFormatInProgress)
      return ERROR_BUSY_DRIVE;

   rc = MY_PROBEBUF(PB_OPWRITE, pData, cbData);
   if (rc)
      {
      Message("Protection VIOLATION in FS_FSINFO!");
      return rc;
      }

   if (usFlag == INFO_RETRIEVE)
      {
      switch (usLevel)
         {
         case FSIL_ALLOC:
            {
            PFSALLOCATE pAlloc = (PFSALLOCATE)pData;

            Message("FSIL_ALLOC");
            if (cbData < sizeof (FSALLOCATE))
               {
               rc = ERROR_BUFFER_OVERFLOW;
               goto FS_FSINFOEXIT;
               }
#ifdef OLD_SOURCE
            pAlloc->ulReserved  = 0L;
#endif
            pAlloc->cbSector = pVolInfo->BootSect.bpb.BytesPerSector;

            if (IsDosSession()) /* Dos Session */
               {
               ULONG ulTotalSectors = pVolInfo->BootSect.bpb.SectorsPerCluster * pVolInfo->ulTotalClusters;
               ULONG ulFreeSectors  = pVolInfo->BootSect.bpb.SectorsPerCluster * pVolInfo->pBootFSInfo->ulFreeClusters;

               if (ulTotalSectors > 32L * 65526L)
                  pAlloc->cSectorUnit = 64;
               else if (ulTotalSectors > 16L * 65526L)
                  pAlloc->cSectorUnit = 32;
               else if (ulTotalSectors > 8L * 65526L)
                  pAlloc->cSectorUnit = 16;
               else
                  pAlloc->cSectorUnit = 8;

               if ((ULONG)pVolInfo->BootSect.bpb.SectorsPerCluster > pAlloc->cSectorUnit)
                  pAlloc->cSectorUnit = (USHORT)pVolInfo->BootSect.bpb.SectorsPerCluster;

               pAlloc->cUnit = min(65526L, ulTotalSectors / pAlloc->cSectorUnit);
               pAlloc->cUnitAvail = min(65526L, ulFreeSectors / pAlloc->cSectorUnit);

               if (f32Parms.fMessageActive & LOG_FUNCS)
                  Message("DOS Free space: sc: %lu tc: %lu fc: %lu",
                     pAlloc->cSectorUnit, pAlloc->cUnit, pAlloc->cUnitAvail);
               }
            else
               {
               pAlloc->cSectorUnit = pVolInfo->BootSect.bpb.SectorsPerCluster;
               pAlloc->cUnit = pVolInfo->ulTotalClusters;
               pAlloc->cUnitAvail = pVolInfo->pBootFSInfo->ulFreeClusters;
               }
            rc = 0;
            break;
            }
         case FSIL_VOLSER:
            {
            PFSINFO pInfo = (PFSINFO)pData;
            USHORT usSize;
            Message("FSIL_VOLSER");
            if (cbData < sizeof (FSINFO))
               {
               rc = ERROR_BUFFER_OVERFLOW;
               goto FS_FSINFOEXIT;
               }

#ifndef __WATCOMC__
            // no such field in Toolkit or Watcom headers
            pInfo->ulVSN = pVolInfo->BootSect.ulVolSerial;
#else
            // low word (aka fdateCreation)
            *((PUSHORT)(pInfo))     = (pVolInfo->BootSect.ulVolSerial & 0xffff);
            // high word (aka ftimeCreation)
            *((PUSHORT)(pInfo) + 1) = (pVolInfo->BootSect.ulVolSerial >> 16);
#endif

            memset(pInfo->vol.szVolLabel, 0, sizeof pInfo->vol.szVolLabel);

            usSize = 11;
            rc = fGetSetVolLabel(pVolInfo, usFlag, pInfo->vol.szVolLabel, &usSize);
            pInfo->vol.cch = (BYTE)usSize;
            break;
            }
         default :
            rc = ERROR_BAD_COMMAND;
            break;
         }
      }
   else if (usFlag == INFO_SET)
      {
      switch (usLevel)
         {
         case FSIL_VOLSER:
            {
            PVOLUMELABEL pVol = (PVOLUMELABEL)pData;
            USHORT usSize;
            if (cbData < 1)
               {
               rc = ERROR_INSUFFICIENT_BUFFER;
               goto FS_FSINFOEXIT;
               }

            if (pVol->cch < (BYTE)12)
               {
               Message("FSIL_VOLSER < 12 , fGetSetVolLabel in");
               usSize = (USHORT) pVol->cch;
               Message("fGetSetVolLabel out, rc=%u", rc);
               rc = fGetSetVolLabel(pVolInfo, usFlag, pVol->szVolLabel, &usSize);
               }
            else
               {
               usSize = 11;
               Message("FSIL_VOLSER, too long, fGetSetVolLabel in");
               rc = fGetSetVolLabel(pVolInfo, usFlag, pVol->szVolLabel, &usSize);
               Message("fGetSetVolLabel out, rc=%lu", rc);
               if (!rc)
                  rc = ERROR_LABEL_TOO_LONG;
               }
            break;
            }

         default :
            rc = ERROR_BAD_COMMAND;
            break;
         }
      }
   else
      rc = ERROR_BAD_COMMAND;

FS_FSINFOEXIT:

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_FSINFO returned %u", rc);
   return rc;
}

USHORT fGetSetVolLabel(PVOLINFO pVolInfo, USHORT usFlag, PSZ pszVolLabel, PUSHORT pusSize)
{
struct vpfsi far * pvpfsi;
struct vpfsd far * pvpfsd;
PDIRENTRY pDirStart, pDir, pDirEnd;
ULONG ulCluster;
DIRENTRY DirEntry;
BOOL     fFound;
USHORT   rc;
BYTE     bAttr = FILE_VOLID | FILE_ARCHIVED;
USHORT   usIndex;
PBOOTSECT pBootSect;

   pDir = NULL;

   pDirStart = (PDIRENTRY)malloc(pVolInfo->usClusterSize);
   if (!pDirStart)
      return ERROR_NOT_ENOUGH_MEMORY;

   fFound = FALSE;
   ulCluster = pVolInfo->BootSect.bpb.RootDirStrtClus;
   while (!fFound && ulCluster != FAT_EOF)
      {
      ReadCluster(pVolInfo, ulCluster, pDirStart, 0);
      pDir    = pDirStart;
      pDirEnd = (PDIRENTRY)((PBYTE)pDirStart + pVolInfo->usClusterSize);
      while (pDir < pDirEnd)
         {
         if ((pDir->bAttr & 0x0F) == FILE_VOLID && pDir->bFileName[0] != DELETED_ENTRY)
            {
            fFound = TRUE;
            memcpy(&DirEntry, pDir, sizeof (DIRENTRY));
            break;
            }
         pDir++;
         }
      if (!fFound)
         {
         ulCluster = GetNextCluster(pVolInfo, ulCluster);
         if (!ulCluster)
            ulCluster = FAT_EOF;
         }
      }


   if (usFlag == INFO_RETRIEVE)
      {
      free(pDirStart);
      if (!fFound)
         {
         memset(pszVolLabel, 0, *pusSize);
         *pusSize = 0;
         return 0;
         }
      *pusSize = 11;
      memcpy(pszVolLabel, DirEntry.bFileName, 11);
      while (*pusSize > 0 && pszVolLabel[(*pusSize)-1] == 0x20)
         {
         (*pusSize)--;
         pszVolLabel[*pusSize] = 0;
         }
      return 0;
      }

   strupr(pszVolLabel);
   for (usIndex = 0; usIndex < *pusSize; usIndex++)
      {
      if (!strchr(rgValidChars, pszVolLabel[usIndex]))
         {
         free(pDirStart);
         return ERROR_INVALID_NAME;
         }
      }

   memset(&DirEntry, 0, sizeof DirEntry);
   memset(DirEntry.bFileName, 0x20, 11);
   memcpy(DirEntry.bFileName, pszVolLabel, min(11, *pusSize));
   DirEntry.bAttr = bAttr;

   if (fFound)
      {
      memcpy(pDir, &DirEntry, sizeof (DIRENTRY));
      rc = WriteCluster(pVolInfo, ulCluster, (PBYTE)pDirStart, DVIO_OPWRTHRU);
      }
   else
      {
      rc = ModifyDirectory(pVolInfo, pVolInfo->BootSect.bpb.RootDirStrtClus,
         MODIFY_DIR_INSERT, NULL, &DirEntry, pszVolLabel, DVIO_OPWRTHRU);
      }
   if (rc)
      {
      free(pDirStart);
      return rc;
      }

   rc = ReadSector(pVolInfo, 0L, 1, (PBYTE)pDirStart, DVIO_OPNCACHE);
   if (!rc)
      {
      pBootSect = (PBOOTSECT)pDirStart;
      memcpy(pBootSect->VolumeLabel, DirEntry.bFileName, 11);
      rc = WriteSector(pVolInfo, 0L, 1, (PBYTE)pBootSect, DVIO_OPWRTHRU | DVIO_OPNCACHE);
      }
   free(pDirStart);

   if (!rc)
      {
      memcpy(pVolInfo->BootSect.VolumeLabel, DirEntry.bFileName, 11);
      rc = FSH_GETVOLPARM(pVolInfo->hVBP, &pvpfsi, &pvpfsd);
      if (!rc)
         memcpy(pvpfsi->vpi_text, DirEntry.bFileName, 11);
      }

   return rc;

}

/******************************************************************
*
******************************************************************/
int far pascal __loadds FS_INIT(
    char far * pszParm,         /* szParm   */
    unsigned long pDevHlp,      /* pDevHlp  */
    unsigned long far * pMiniFSD    /* pMiniFSD */
)
{
BOOL fSilent = FALSE;
PSZ  p;

   pMiniFSD = pMiniFSD;

   memset(&f32Parms, 0, sizeof f32Parms);

   f32Parms.fLW = FALSE;
   f32Parms.ulDiskIdle = 1000;
   f32Parms.ulBufferIdle = 500;
   f32Parms.ulMaxAge = 5000;
   f32Parms.usSegmentsAllocated = 0;
   strcpy(f32Parms.szVersion, FAT32_VERSION);

   Device_Help = pDevHlp;
   if (pszParm)
      {
      strncpy(szArguments, pszParm, sizeof szArguments);
      strlwr( szArguments );

      p = strstr(szArguments, "/monitor");
      if( !p )
        p = strstr( szArguments, "-monitor");
      if( p )
         f32Parms.fMessageActive = LOG_FS;

      p = strstr(szArguments, "/q");
      if( !p )
         p = strstr( szArguments, "-q");
      if( p )
         fSilent = TRUE;

      /*
         Get size of cache
      */

      p = strstr(szArguments, "/cache:");
      if (!p)
         p = strstr(szArguments, "-cache:");
      if (p)
         {
         p += 7;
         ulCacheSectors = atol(p) * 2;
         }
      else
         {
         InitMessage("FAT32: Using default cache size of 1024 Kb.\r\n");
         }

      /*
         Get RA Sectors
      */
      p = strstr(szArguments, "/rasectors:");
      if (!p)
         p = strstr(szArguments, "-rasectors:");
      if (p)
         {
         p +=11;
         usDefaultRASectors = atoi(p);
         }
      /*
         Get EA Settings
      */
      p = strstr(szArguments, "/eas");
      if (!p)
         p = strstr(szArguments, "-eas");
      if (p)
         f32Parms.fEAS = TRUE;

      p = strstr( szArguments, "/h");
      if( !p )
        p = strstr( szArguments, "-h");
      if( p )
         f32Parms.fHighMem = TRUE;
      }

      p = strstr( szArguments, "/calcfree");
      if( !p )
         p = strstr( szArguments, "-calcfree");
      if( p )
         f32Parms.fCalcFree = TRUE;

#if 1
   if (!DosGetInfoSeg(&sGlob, &sLoc))
#else
   if (!DevHelp_GetDOSVar(DHGETDOSV_SYSINFOSEG, 0, (PPVOID)&sGlob))
#endif
      pGI = MAKEPGINFOSEG(sGlob);
   else
      {
      InitMessage("FAT32: Unable to acquire Global Infoseg!\r\n");
      return 1;
      }
   pGITicks = &pGI->msecs;
   if (!fSilent)
      InitMessage(szBanner);

   if (!ulCacheSectors)
      InitMessage("FAT32: Warning CACHE size is zero!\r\n");

   return 0;
}

VOID InitMessage(PSZ pszMessage)
{
USHORT usWritten;

   DosWrite(1, pszMessage, strlen(pszMessage), &usWritten);

}

/******************************************************************
*
******************************************************************/
int far pascal __loadds FS_IOCTL(
    struct sffsi far * psffsi,      /* psffsi   */
    struct sffsd far * psffsd,      /* psffsd   */
    unsigned short usCat,       /* cat      */
    unsigned short usFunc,      /* func     */
    char far * pParm,           /* pParm    */
    unsigned short cbParm,      /* lenParm  */
    unsigned far * pcbParm,     /* pParmLenInOut */
    char far * pData,           /* pData    */
    unsigned short cbData,      /* lenData  */
    unsigned far * pcbData      /* pDataLenInOut */
)
{
USHORT rc;
PVOLINFO pVolInfo;
ULONG hDEV;
PBIOSPARAMETERBLOCK pBPB;

   psffsd = psffsd;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_IOCTL, Cat %Xh, Func %Xh, File# %u",
         usCat, usFunc, psffsi->sfi_selfsfn);

   pVolInfo = GetVolInfo(psffsi->sfi_hVPB);

   rc = 0;

   if (pData && pData != MYNULL)
      {
      if (cbData > 0)
         {
         rc = MY_PROBEBUF(PB_OPWRITE, pData, cbData);
         if (rc)
            {
            Message("Protection VIOLATION in data of FS_IOCTL!");
            return rc;
            }
         }

      if (pcbData)
         {
         rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pcbData, sizeof (unsigned));
         if (rc)
            pcbData = NULL;
         }

      if (pcbData)
         *pcbData = cbData;
      }
   else
      {
      cbData = 0;
      pcbData = NULL;
      }

   if (pParm && pParm != MYNULL)
      {
      if (cbParm > 0)
         {
         rc = MY_PROBEBUF(PB_OPREAD, pParm, cbParm);
         if (rc)
            {
            Message("Protection VIOLATION in parm of FS_IOCTL, address %lX, len %u!",
               pParm, cbParm);
            return rc;
            }
         }
      if (pcbParm)
         {
         rc = MY_PROBEBUF(PB_OPREAD, (PBYTE)pcbParm, sizeof (unsigned));
         if (rc)
            pcbParm = NULL;
         }
      if (pcbParm)
         *pcbParm = cbParm;
      }
   else
      {
      cbParm = 0;
      pcbParm = NULL;
      }

   switch (usCat)
      {
      case IOCTL_DISK    :
         Message("pVolInfo->fLocked=%u, pVolInfo->ulOpenFiles=%lx", pVolInfo->fLocked, pVolInfo->ulOpenFiles);
         switch (usFunc)
            {
            case DSK_LOCKDRIVE:
               if (pVolInfo->fLocked)
                  {
                  Message("Drive locked, pVolInfo->fLocked=%lu", pVolInfo->fLocked);
                  rc = ERROR_DRIVE_LOCKED;
                  goto FS_IOCTLEXIT;
                  }
               if (pVolInfo->ulOpenFiles > 1L)
                  {
                  Message("Cannot lock, %lu open files", pVolInfo->ulOpenFiles);
                  rc = ERROR_DRIVE_LOCKED;
                  goto FS_IOCTLEXIT;
                  }

               hDEV = GetVolDevice(pVolInfo);
               rc = FSH_DOVOLIO2(hDEV, psffsi->sfi_selfsfn,
                  usCat, usFunc, pParm, cbParm, pData, cbData);

               if (!rc) {
                 pVolInfo->fLocked = TRUE;

                 if (pcbData) {
                   *pcbData = cbData;
                 }

                 if (pcbParm) {
                   *pcbParm = cbParm;
                 }
               }

               GetProcInfo(&pVolInfo->ProcLocked, sizeof (PROCINFO));
               break;

            case DSK_UNLOCKDRIVE:
               hDEV = GetVolDevice(pVolInfo);
               rc = FSH_DOVOLIO2(hDEV, psffsi->sfi_selfsfn,
                  usCat, usFunc, pParm, cbParm, pData, cbData);

               if (!rc) {

                 if (pcbData) {
                   *pcbData = cbData;
                 }

                 if (pcbParm) {
                   *pcbParm = cbParm;
                 }

                 if (pVolInfo->fLocked)
                    {
                    pVolInfo->fLocked = FALSE;
                    //rc = 0;
                    }
                 //else
                 //   rc = ERROR_INVALID_PARAMETER;
               }
               break;

            case DSK_BLOCKREMOVABLE :
               //if (pcbData)
               //   *pcbData = sizeof (BYTE);

               hDEV = GetVolDevice(pVolInfo);
               rc = FSH_DOVOLIO2(hDEV, psffsi->sfi_selfsfn,
                  usCat, usFunc, pParm, cbParm, pData, cbData);

               if (!rc) {

                 if (pcbData) {
                   *pcbData = cbData;
                 }

                 if (pcbParm) {
                   *pcbParm = cbParm;
                 }
               }

               break;
/*
            case DSK_QUERYMEDIASENSE :
               hDEV = GetVolDevice(pVolInfo);
               rc = FSH_DOVOLIO2(hDEV, psffsi->sfi_selfsfn,
                  usCat, usFunc, pParm, cbParm, pData, cbData);

               if (rc == ERROR_SECTOR_NOT_FOUND)
                  rc = ERROR_NOT_DOS_DISK;
               else if (rc)
                  rc = 0;                                            

                  if (pData)
                     *pData = 0;

               if (!rc) {

                  if (pcbData) {
                     *pcbData = cbData;
                  }

                  if (pcbParm) {
                     *pcbParm = cbParm;
                  }
               }

               break;
 */
            case 0x66 : /* DSK_GETLOCKSTATUS */
               if (pcbData)
                  *pcbData = sizeof (USHORT);
               if (cbData < 2)
                  {
                  rc = ERROR_BUFFER_OVERFLOW;
                  goto FS_IOCTLEXIT;
                  }
               if (pVolInfo->fLocked)
                  *(PWORD)pData = 0x0005;
               else
                  *(PWORD)pData = 0x0006;

               Message("DSK_GETLOCKSTATUS, rc=%u", rc);
               rc = 0;
               break;

            case DSK_GETDEVICEPARAMS :
               if (pcbData)
                  *pcbData = sizeof (BIOSPARAMETERBLOCK);
               hDEV = GetVolDevice(pVolInfo);
               rc = FSH_DOVOLIO2(hDEV, psffsi->sfi_selfsfn,
                  usCat, usFunc, pParm, cbParm, pData, cbData);
               if (!rc) {

                  if (pcbData) {
                    *pcbData = sizeof (BIOSPARAMETERBLOCK);
                  }

                  if (pcbParm) {
                    *pcbParm = cbParm;
                  }

                  pBPB = (PBIOSPARAMETERBLOCK)pData;

                  pBPB->bSectorsPerCluster = pVolInfo->BootSect.bpb.SectorsPerCluster;
                  pBPB->usReservedSectors = pVolInfo->BootSect.bpb.ReservedSectors;
                  pBPB->cFATs = pVolInfo->BootSect.bpb.NumberOfFATs;
                  pBPB->cRootEntries = (USHORT)(GetChainSize(pVolInfo, pVolInfo->BootSect.bpb.RootDirStrtClus) * 128);
                  if (pVolInfo->BootSect.bpb.BigSectorsPerFat < 0x10000L)
                     pBPB->usSectorsPerFAT = (USHORT)pVolInfo->BootSect.bpb.BigSectorsPerFat;
                  else
                     pBPB->usSectorsPerFAT = 0xFFFF; // ??? vs!
               }

               break;

            default  :
               hDEV = GetVolDevice(pVolInfo);
               rc = FSH_DOVOLIO2(hDEV, psffsi->sfi_selfsfn,
                  usCat, usFunc, pParm, cbParm, pData, cbData);

                if (!rc) {
                 if (pcbData) {
                   *pcbData = cbData;
                 }
  
                 if (pcbParm) {
                   *pcbParm = cbParm;
                 }
               }

               //Message("usFunc=default, cat=%u, func=%u rc=%u", usCat, usFunc, rc);
               break;
            }
         break;

      case IOCTL_FAT32 :
         switch (usFunc)
            {
            case FAT32_SETRASECTORS:
               if (cbParm < sizeof (USHORT))
                  {
                  rc = ERROR_INSUFFICIENT_BUFFER;
                  goto FS_IOCTLEXIT;
                  }
               pVolInfo->usRASectors = *(PUSHORT)pParm;
#if 1
               if (pVolInfo->usRASectors > MAX_RASECTORS)
                  pVolInfo->usRASectors = MAX_RASECTORS;
#else
               if (pVolInfo->usRASectors > (pVolInfo->usClusterSize / 512 ) * 4)
                  pVolInfo->usRASectors = (pVolInfo->usClusterSize / 512 ) * 4;
#endif
               *(PUSHORT)pParm = pVolInfo->usRASectors;
               Message("usRASectors changed to %u", pVolInfo->usRASectors);
               rc = 0;
               break;

            case FAT32_QUERYRASECTORS:
               if (cbData < sizeof (USHORT))
                  {
                  rc = ERROR_BUFFER_OVERFLOW;
                  goto FS_IOCTLEXIT;
                  }
               *(PUSHORT)pData = pVolInfo->usRASectors;
               rc = 0;
               break;

            case FAT32_GETVOLCLEAN :
               if (cbData < sizeof (BOOL))
                  {
                  rc = ERROR_BUFFER_OVERFLOW;
                  goto FS_IOCTLEXIT;
                  }
               *(PBOOL)pData = pVolInfo->fDiskCleanOnMount;
               if (cbData >= sizeof (BOOL) * 2)
                  *(PBOOL)(pData + 2) = GetDiskStatus(pVolInfo);
               rc = 0;
               break;

            case FAT32_MARKVOLCLEAN:
               Message("IOCTL:Marking volume clean");
               pVolInfo->fDiskCleanOnMount = TRUE;
               rc = 0;
               break;

            case FAT32_FORCEVOLCLEAN:
               if (cbParm < sizeof (BOOL))
                  {
                  rc = ERROR_INSUFFICIENT_BUFFER;
                  goto FS_IOCTLEXIT;
                  }
               Message("IOCTL:Forcing volume clean");
               pVolInfo->fDiskCleanOnMount = *(PBOOL)pParm;
               if (!MarkDiskStatus(pVolInfo, pVolInfo->fDiskCleanOnMount))
                  {
                  rc = ERROR_SECTOR_NOT_FOUND;
                  goto FS_IOCTLEXIT;
                  }
               *(PBOOL)pParm = GetDiskStatus(pVolInfo);
               rc = 0;
               break;

            case FAT32_RECOVERCHAIN :
               if (!pVolInfo->fLocked)
                  {
                  rc = ERROR_BAD_COMMAND;
                  goto FS_IOCTLEXIT;
                  }
               if (cbParm < sizeof (ULONG))
                  {
                  rc = ERROR_INSUFFICIENT_BUFFER;
                  goto FS_IOCTLEXIT;
                  }
               rc = RecoverChain(pVolInfo, *(PULONG)pParm, pData, cbData);
               break;

            case FAT32_DELETECHAIN  :
               if (!pVolInfo->fLocked)
                  {
                  rc = ERROR_BAD_COMMAND;
                  goto FS_IOCTLEXIT;
                  }
               if (cbParm < sizeof (ULONG))
                  {
                  rc = ERROR_INSUFFICIENT_BUFFER;
                  goto FS_IOCTLEXIT;
                  }
               DeleteFatChain(pVolInfo, *(PULONG)pParm);
               rc = 0;
               break;

            case FAT32_GETFREESPACE :
               if (!pVolInfo->fLocked)
                  {
                  rc = ERROR_BAD_COMMAND;
                  goto FS_IOCTLEXIT;
                  }
               if (cbData < sizeof (ULONG))
                  {
                  rc = ERROR_BUFFER_OVERFLOW;
                  goto FS_IOCTLEXIT;
                  }

               *(PULONG)pData = GetFreeSpace(pVolInfo);
               rc = 0;
               break;

            case FAT32_SETFILESIZE:
               if (!pVolInfo->fLocked)
                  {
                  rc = ERROR_BAD_COMMAND;
                  goto FS_IOCTLEXIT;
                  }
               if (cbParm < sizeof (FILESIZEDATA))
                  {
                  rc = ERROR_INSUFFICIENT_BUFFER;
                  goto FS_IOCTLEXIT;
                  }

               rc = SetFileSize(pVolInfo, (PFILESIZEDATA)pParm);
               break;
            case FAT32_QUERYEAS:
            case FAT32_SETEAS  :
               if (usFunc == FAT32_SETEAS && !pVolInfo->fLocked)
                  {
                  rc = ERROR_BAD_COMMAND;
                  goto FS_IOCTLEXIT;
                  }
               if (cbParm < sizeof (MARKFILEEASBUF))
                  {
                  rc = ERROR_INSUFFICIENT_BUFFER;
                  goto FS_IOCTLEXIT;
                  }
               rc = GetSetFileEAS(pVolInfo, usFunc, (PMARKFILEEASBUF)pParm);
               break;

            case FAT32_SETCLUSTER:
               if (cbParm < sizeof (SETCLUSTERDATA))
                  rc = ERROR_INSUFFICIENT_BUFFER;
               else
                  {
                  PSETCLUSTERDATA pSet = (PSETCLUSTERDATA)pParm;

                  if (SetNextCluster(pVolInfo, pSet->ulCluster,
                     pSet->ulNextCluster) != pSet->ulNextCluster)
                     rc = ERROR_SECTOR_NOT_FOUND;
                  else
                     rc = 0;
                  }
               break;
            case FAT32_READCLUSTER:
               {
               ULONG ulCluster;
               if (cbParm < sizeof(ULONG))
                  {
                  rc = ERROR_INSUFFICIENT_BUFFER;
                  goto FS_IOCTLEXIT;
                  }
               if ((USHORT)cbData < pVolInfo->usClusterSize)
                  {
                  rc = ERROR_BUFFER_OVERFLOW;
                  goto FS_IOCTLEXIT;
                  }
               ulCluster = *(PULONG)pParm;
               rc = ReadCluster(pVolInfo, ulCluster, pData, 0);
               break;
               }
            case FAT32_READSECTOR:
               {
               PREADSECTORDATA pRSD;
               if (cbParm < sizeof(READSECTORDATA))
                  {
                  rc = ERROR_INSUFFICIENT_BUFFER;
                  goto FS_IOCTLEXIT;
                  }
               pRSD = (PREADSECTORDATA)pParm;
               if ((USHORT)cbData < pRSD->nSectors * SECTOR_SIZE)
                  {
                  rc = ERROR_BUFFER_OVERFLOW;
                  goto FS_IOCTLEXIT;
                  }
               rc = ReadSector(pVolInfo, pRSD->ulSector, pRSD->nSectors, pData, 0);
               break;
               }
            case FAT32_WRITECLUSTER:
               {
               ULONG ulCluster;
               if (cbParm < sizeof(ULONG))
                  {
                  rc = ERROR_INSUFFICIENT_BUFFER;
                  goto FS_IOCTLEXIT;
                  }
               if ((USHORT)cbData < pVolInfo->usClusterSize)
                  {
                  rc = ERROR_INSUFFICIENT_BUFFER;
                  goto FS_IOCTLEXIT;
                  }
               ulCluster = *(PULONG)pParm;
               rc = WriteCluster(pVolInfo, ulCluster, pData, 0);
               break;
               }
            case FAT32_WRITESECTOR:
               {
               PWRITESECTORDATA pWSD;
               if (cbParm < sizeof(WRITESECTORDATA))
                  {
                  rc = ERROR_INSUFFICIENT_BUFFER;
                  goto FS_IOCTLEXIT;
                  }
               pWSD = (PWRITESECTORDATA)pParm;
               if ((USHORT)cbData < pWSD->nSectors * SECTOR_SIZE)
                  {
                  rc = ERROR_INSUFFICIENT_BUFFER;
                  goto FS_IOCTLEXIT;
                  }
               rc = WriteSector(pVolInfo, pWSD->ulSector, pWSD->nSectors, pData, 0);
               break;
               }
            default :
               rc = ERROR_BAD_COMMAND;
               break;
            }
         break;

      default:
         hDEV = GetVolDevice(pVolInfo);
         rc = FSH_DOVOLIO2(hDEV, psffsi->sfi_selfsfn,
            usCat, usFunc, pParm, cbParm, pData, cbData);

         if (!rc) {
           if (pcbData) {
             *pcbData = cbData;
           }

           if (pcbParm) {
             *pcbParm = cbParm;
           }
         }

         break;
      }

FS_IOCTLEXIT:
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_IOCTL returned %u", rc);
   return rc;

}


USHORT GetSetFileEAS(PVOLINFO pVolInfo, USHORT usFunc, PMARKFILEEASBUF pMark)
{
ULONG ulDirCluster;
PSZ   pszFile;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("GetSetFileEAS");

   ulDirCluster = FindDirCluster(pVolInfo,
      NULL,
      NULL,
      pMark->szFileName,
      0xFFFF,
      RETURN_PARENT_DIR,
      &pszFile);
   if (ulDirCluster == FAT_EOF)
      return ERROR_PATH_NOT_FOUND;

   if (usFunc == FAT32_QUERYEAS)
      {
      ULONG ulCluster;
      DIRENTRY DirEntry;

      ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszFile, &DirEntry, NULL);
      if (ulCluster == FAT_EOF)
         return ERROR_FILE_NOT_FOUND;
      pMark->fEAS = DirEntry.fEAS;
      return 0;
      }

   return MarkFileEAS(pVolInfo, ulDirCluster, pszFile, pMark->fEAS);
}

USHORT RecoverChain(PVOLINFO pVolInfo, ULONG ulCluster, PBYTE pData, USHORT cbData)
{
DIRENTRY DirEntry;
BYTE     szFileName[14];
USHORT   usNr;

   memset(&DirEntry, 0, sizeof (DIRENTRY));

   memcpy(DirEntry.bFileName, "FILE0000CHK", 11);
   strcpy(szFileName, "FILE0000.CHK");
   for (usNr = 0; usNr < 9999; usNr++)
      {
      USHORT iPos = 7;
      USHORT usNum = usNr;

         while (usNum)
            {
            szFileName[iPos] = (BYTE)((usNum % 10) + '0');
            usNum /= 10;
            iPos--;
            }
         if (FindPathCluster(pVolInfo, pVolInfo->BootSect.bpb.RootDirStrtClus,
            szFileName, NULL, NULL) == FAT_EOF)
            break;
      }
   if (usNr == 9999)
      return ERROR_FILE_EXISTS;
   memcpy(DirEntry.bFileName, szFileName, 8);
   if (pData)
      strncpy(pData, szFileName, cbData);

   DirEntry.wCluster = LOUSHORT(ulCluster);
   DirEntry.wClusterHigh = HIUSHORT(ulCluster);
   while (ulCluster != FAT_EOF)
      {
      ULONG ulNextCluster;
      DirEntry.ulFileSize += pVolInfo->usClusterSize;
      ulNextCluster = GetNextCluster(pVolInfo, ulCluster);
      if (!ulNextCluster)
         {
         SetNextCluster(pVolInfo, ulCluster, FAT_EOF);
         ulCluster = FAT_EOF;
         }
      else
         ulCluster = ulNextCluster;
      }

   return MakeDirEntry(pVolInfo,
      pVolInfo->BootSect.bpb.RootDirStrtClus,
      &DirEntry, szFileName);
}

USHORT SetFileSize(PVOLINFO pVolInfo, PFILESIZEDATA pFileSize)
{
ULONG ulDirCluster;
PSZ   pszFile;
ULONG ulCluster;
DIRENTRY DirEntry;
DIRENTRY DirNew;
ULONG ulClustersNeeded;
ULONG ulClustersUsed;
USHORT rc;

   ulDirCluster = FindDirCluster(pVolInfo,
      NULL,
      NULL,
      pFileSize->szFileName,
      0xFFFF,
      RETURN_PARENT_DIR,
      &pszFile);

   if (ulDirCluster == FAT_EOF)
      return ERROR_PATH_NOT_FOUND;

   ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszFile, &DirEntry, NULL);
   if (ulCluster == FAT_EOF)
      return ERROR_FILE_NOT_FOUND;
   if (!ulCluster)
      pFileSize->ulFileSize = 0L;

   ulClustersNeeded = pFileSize->ulFileSize / pVolInfo->usClusterSize;
   if (pFileSize->ulFileSize % pVolInfo->usClusterSize)
      ulClustersNeeded++;

   if (pFileSize->ulFileSize > 0 )
      {
      ulClustersUsed = 1;
      while (ulClustersUsed < ulClustersNeeded)
         {
         ULONG ulNextCluster = GetNextCluster(pVolInfo, ulCluster);
         if (!ulNextCluster)
            break;
         ulCluster = ulNextCluster;
         if (ulCluster == FAT_EOF)
            break;
         ulClustersUsed++;
         }
      if (ulCluster == FAT_EOF)
         pFileSize->ulFileSize = ulClustersUsed * pVolInfo->usClusterSize;
      else
         SetNextCluster(pVolInfo, ulCluster, FAT_EOF);
      }

   memcpy(&DirNew, &DirEntry, sizeof (DIRENTRY));
   DirNew.ulFileSize = pFileSize->ulFileSize;

   if (!pFileSize->ulFileSize)
      {
      DirNew.wCluster = 0;
      DirNew.wClusterHigh = 0;
      }


   rc = ModifyDirectory(pVolInfo, ulDirCluster, MODIFY_DIR_UPDATE,
      &DirEntry, &DirNew, NULL, 0);

   return rc;
}


/******************************************************************
*
******************************************************************/
int far pascal  __loadds FS_MOVE(
    struct cdfsi far * pcdfsi,      /* pcdfsi   */
    struct cdfsd far * pcdfsd,      /* pcdfsd   */
    char far * pSrc,            /* pSrc     */
    unsigned short usSrcCurDirEnd,      /* iSrcCurDirEnd*/
    char far * pDst,            /* pDst     */
    unsigned short usDstCurDirEnd,      /* iDstCurDirEnd*/
    unsigned short usFlags      /* flags    */
)
{
PVOLINFO pVolInfo;
ULONG ulSrcDirCluster;
ULONG ulDstDirCluster;
PSZ   pszSrcFile;
PSZ   pszDstFile;
DIRENTRY DirEntry;
ULONG    ulCluster;
ULONG    ulTarCluster;
USHORT   rc;
POPENINFO pOISrc = NULL;
POPENINFO pOIDst = NULL;
BYTE     szSrcLongName[ FAT32MAXPATH ];
BYTE     szDstLongName[ FAT32MAXPATH ];

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_MOVE %s to %s", pSrc, pDst);

   pVolInfo = GetVolInfo(pcdfsi->cdi_hVPB);
   if (IsDriveLocked(pVolInfo))
      {
      rc = ERROR_DRIVE_LOCKED;
      goto FS_MOVEEXIT;
      }
   if (!pVolInfo->fDiskCleanOnMount)
      {
      rc = ERROR_VOLUME_DIRTY;
      goto FS_MOVEEXIT;
      }
   if (pVolInfo->fWriteProtected)
      {
      rc = ERROR_WRITE_PROTECT;
      goto FS_MOVEEXIT;
      }

   if( TranslateName(pVolInfo, 0L, pSrc, szSrcLongName, TRANSLATE_SHORT_TO_LONG ))
      strcpy( szSrcLongName, pSrc );

   if( TranslateName(pVolInfo, 0L, pDst, szDstLongName, TRANSLATE_SHORT_TO_LONG ))
      strcpy( szDstLongName, pDst );

   pOISrc = malloc(sizeof (OPENINFO));
   if (!pOISrc)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      goto FS_MOVEEXIT;
      }

   pOISrc->pSHInfo = GetSH( szSrcLongName, pOISrc);
   if (!pOISrc->pSHInfo)
      {
      rc = ERROR_TOO_MANY_OPEN_FILES;
      goto FS_MOVEEXIT;
      }
   pOISrc->pSHInfo->sOpenCount++;
   if (pOISrc->pSHInfo->sOpenCount > 1)
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_MOVEEXIT;
      }
   pOISrc->pSHInfo->fLock = TRUE;

   if( stricmp( szSrcLongName, szDstLongName ))
   {
        pOIDst = malloc(sizeof (OPENINFO));
        if (!pOIDst)
        {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto FS_MOVEEXIT;
        }

        pOIDst->pSHInfo = GetSH( szDstLongName, pOIDst);
        if (!pOIDst->pSHInfo)
        {
            rc = ERROR_TOO_MANY_OPEN_FILES;
            goto FS_MOVEEXIT;
        }
        pOIDst->pSHInfo->sOpenCount++;
        if (pOIDst->pSHInfo->sOpenCount > 1)
        {
            rc = ERROR_ACCESS_DENIED;
            goto FS_MOVEEXIT;
        }
        pOIDst->pSHInfo->fLock = TRUE;
   }


   /*
      Check destination
   */
   ulDstDirCluster = FindDirCluster(pVolInfo,
      pcdfsi,
      pcdfsd,
      pDst,
      usDstCurDirEnd,
      RETURN_PARENT_DIR,
      &pszDstFile);
   if (ulDstDirCluster == FAT_EOF)
      {
      rc = ERROR_PATH_NOT_FOUND;
      goto FS_MOVEEXIT;
      }

   ulTarCluster = FindPathCluster(pVolInfo, ulDstDirCluster, pszDstFile, &DirEntry, NULL);

   /*
      Check source
   */
   ulSrcDirCluster = FindDirCluster(pVolInfo,
      pcdfsi,
      pcdfsd,
      pSrc,
      usSrcCurDirEnd,
      RETURN_PARENT_DIR,
      &pszSrcFile);
   if (ulSrcDirCluster == FAT_EOF)
      {
      rc = ERROR_PATH_NOT_FOUND;
      goto FS_MOVEEXIT;
      }

   ulCluster = FindPathCluster(pVolInfo, ulSrcDirCluster, pszSrcFile, &DirEntry, NULL);
   if (ulCluster == FAT_EOF)
      {
      rc = ERROR_FILE_NOT_FOUND;
      goto FS_MOVEEXIT;
      }

   if (!(ulTarCluster == FAT_EOF || ulTarCluster == ulCluster))
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_MOVEEXIT;
      }

   if (ulCluster == pVolInfo->BootSect.bpb.RootDirStrtClus)
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_MOVEEXIT;
      }

#if 0
   if (DirEntry.bAttr & FILE_READONLY)
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_MOVEEXIT;
      }
#endif

   if (DirEntry.bAttr & FILE_DIRECTORY)
      {
#if 1
      int iLen = strlen( szSrcLongName );

      if ( !strnicmp(szSrcLongName, szDstLongName, iLen ) &&
           ( szDstLongName[ iLen ] == '\\' ))
      {
        rc = ERROR_CIRCULARITY_REQUESTED;
        goto FS_MOVEEXIT;
      }

      ReleaseSH( pOISrc );
      pOISrc = NULL;

      rc = MY_ISCURDIRPREFIX( szSrcLongName );
      if( rc )
        goto FS_MOVEEXIT;

      pOISrc = malloc(sizeof (OPENINFO));
      if (!pOISrc)
      {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto FS_MOVEEXIT;
      }

      pOISrc->pSHInfo = GetSH( szSrcLongName, pOISrc);
      if (!pOISrc->pSHInfo)
      {
        rc = ERROR_TOO_MANY_OPEN_FILES;
        goto FS_MOVEEXIT;
      }

      pOISrc->pSHInfo->sOpenCount++;
      if (pOISrc->pSHInfo->sOpenCount > 1)
      {
          rc = ERROR_ACCESS_DENIED;
        goto FS_MOVEEXIT;
      }

      pOISrc->pSHInfo->fLock = TRUE;
#else
      BYTE szName[FAT32MAXPATH];
      rc = FSH_ISCURDIRPREFIX(pSrc);
      if (rc)
         goto FS_MOVEEXIT;
      rc = TranslateName(pVolInfo, 0L, pSrc, szName, TRANSLATE_AUTO);
      if (rc)
         goto FS_MOVEEXIT;
      rc = FSH_ISCURDIRPREFIX(szName);
      if (rc)
         goto FS_MOVEEXIT;
#endif
      }

   /*
        rename EA file
   */

   if (f32Parms.fEAS)
      {
      rc = usMoveEAS(pVolInfo, ulSrcDirCluster, pszSrcFile,
                               ulDstDirCluster, pszDstFile);
      if( rc )
         goto FS_MOVEEXIT;
      }

   if (ulSrcDirCluster == ulDstDirCluster)
      {
      DIRENTRY DirOld;

      memcpy(&DirOld, &DirEntry, sizeof DirEntry);

      rc = ModifyDirectory(pVolInfo, ulSrcDirCluster,
         MODIFY_DIR_RENAME, &DirOld, &DirEntry, pszDstFile, 0);
      goto FS_MOVEEXIT;
      }

   /*
      First delete old
   */

   rc = ModifyDirectory(pVolInfo, ulSrcDirCluster, MODIFY_DIR_DELETE, &DirEntry, NULL, NULL, 0);
   if (rc)
      goto FS_MOVEEXIT;

   /*
      Then insert new
   */

   rc = ModifyDirectory(pVolInfo, ulDstDirCluster, MODIFY_DIR_INSERT, NULL, &DirEntry, pszDstFile, 0);
   if (rc)
      goto FS_MOVEEXIT;

   /*
      If a directory was moved, the .. entry in the dir itself must
      be modified as well.
   */
   if (DirEntry.bAttr & FILE_DIRECTORY)
      {
      ULONG ulTmp;
      ulTmp = FindPathCluster(pVolInfo, ulCluster, "..", &DirEntry, NULL);
      if (ulTmp != FAT_EOF)
         {
         DIRENTRY DirNew;

         memcpy(&DirNew, &DirEntry, sizeof (DIRENTRY));
         DirNew.wCluster = LOUSHORT(ulDstDirCluster);
         DirNew.wClusterHigh = HIUSHORT(ulDstDirCluster);

         rc = ModifyDirectory(pVolInfo, ulCluster,
            MODIFY_DIR_UPDATE, &DirEntry, &DirNew, NULL, 0);
         if (rc)
            goto FS_MOVEEXIT;
         }
      else
         {
         Message("FS_MOVE: .. entry of moved directory not found!");
         rc = ERROR_PATH_NOT_FOUND;
         goto FS_MOVEEXIT;
         }
      }

FS_MOVEEXIT:

   if (pOISrc)
      {
      if (pOISrc->pSHInfo)
         ReleaseSH(pOISrc);
      else
         free(pOISrc);
      }

   if (pOIDst)
      {
      if (pOIDst->pSHInfo)
         ReleaseSH(pOIDst);
      else
         free(pOIDst);
      }

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_MOVE returned %d", rc);

   return rc;

   usFlags = usFlags;
}

/******************************************************************
*
******************************************************************/
int far pascal  __loadds FS_PROCESSNAME(
    char far *  pNameBuf        /* pNameBuf */
)
{
#if 1
     USHORT usUniCh;
     USHORT usLen;
     char   far *p;

     if (f32Parms.fMessageActive & LOG_FS)
        Message("FS_PROCESSNAME for %s", pNameBuf);

     for( p = pNameBuf; *p; p += usLen )
     {
        usLen = Translate2Win( p, &usUniCh, 1 );
        if( usUniCh == 0xFFFD )
        {
            *p = '_';
            if( usLen == 2 )
                p[ 1 ] = '_';
        }
     }

     if (f32Parms.fMessageActive & LOG_FS)
        Message(" FS_PROCESSNAME returned filename: %s", pNameBuf);
#endif
   return 0;
}


/******************************************************************
*
******************************************************************/
int far pascal  __loadds FS_SHUTDOWN(
    unsigned short usType,      /* usType   */
    unsigned long    ulReserved /* ulReserved   */
)
{
PVOLINFO pVolInfo;
USHORT rc = 0;

   ulReserved = ulReserved;

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_SHUTDOWN, Type = %d", usType);
   f32Parms.fInShutDown = TRUE;
   f32Parms.fLW = FALSE;

   if (usType == SD_BEGIN)
      {
      pVolInfo = pGlobVolInfo;
      while (pVolInfo)
         {
         if (!pVolInfo->fWriteProtected)
            {
            rc = usFlushVolume(pVolInfo, FLUSH_DISCARD, TRUE, PRIO_URGENT);
            if (rc)
               goto FS_SHUTDOWNEXIT;

            if (!f32Parms.usDirtySectors)
               goto FS_SHUTDOWNEXIT;

            if (!UpdateFSInfo(pVolInfo))
               {
               rc = ERROR_SECTOR_NOT_FOUND;
               goto FS_SHUTDOWNEXIT;
               }
            if (!MarkDiskStatus(pVolInfo, TRUE))
               {
               rc = ERROR_SECTOR_NOT_FOUND;
               goto FS_SHUTDOWNEXIT;
               }
            }

         pVolInfo = (PVOLINFO)pVolInfo->pNextVolInfo;
         }
      }
   else /* usType == SD_COMPLETE */
      TranslateFreeBuffer();

FS_SHUTDOWNEXIT:

   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_SHUTDOWN returned %d", rc);
   return rc;
}

/******************************************************************
*
******************************************************************/
int far pascal __loadds  FS_VERIFYUNCNAME(
    unsigned short usFlag,      /* flag     */
    char far *  pName       /* pName    */
)
{
   if (f32Parms.fMessageActive & LOG_FS)
      Message("FS_VERIFYUNCNAME - NOT SUPPORTED");
   usFlag = usFlag;
   pName = pName;
   return ERROR_NOT_SUPPORTED;
}



/******************************************************************
*
******************************************************************/
void * gdtAlloc(ULONG tSize, BOOL fSwap)
{
INT rc;
USHORT usSel;
PVOID pRet;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("gdtAlloc for %lu bytes", tSize);

   if (fSwap)
      rc = FSH_SEGALLOC(SA_FRING0|SA_FSWAP, tSize, &usSel);
   else
      rc = FSH_SEGALLOC(SA_FRING0, tSize, &usSel);
   if (rc)
      {
      Message("FSH_SEGALLOC (gdtAlloc %ld bytes) failed, rc = %d",
         tSize, rc);
      return NULL;
      }

   pRet = NULL;
   pRet = MAKEP(usSel, 0);

   f32Parms.usSegmentsAllocated++;
   return pRet;
}

/******************************************************************
*
******************************************************************/
void * ldtAlloc(ULONG tSize)
{
INT rc;
USHORT usSel;
PVOID pRet;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("ldtAlloc for %lu bytes", tSize);

   rc = FSH_SEGALLOC(SA_FLDT | SA_FSWAP| SA_FRING3, tSize, &usSel);
   if (rc)
      {
      Message("FSH_SEGALLOC (ldtAlloc %ld bytes) failed, rc = %d",
         tSize, rc);
      return gdtAlloc(tSize, TRUE);
      }

   pRet = NULL;
   pRet = MAKEP(usSel, 0);
   f32Parms.usSegmentsAllocated++;
   return pRet;
}


/******************************************************************
*
******************************************************************/
ULONG linalloc(ULONG tSize, BOOL fHighMem, BOOL fIgnore)
{
USHORT rc;
ULONG ulFlags;
ULONG ulAddress;
ULONG ulReserved;
PVOID pv = &ulReserved;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("linAlloc");

   ulFlags = VMDHA_FIXED;
   if( fHighMem )
      ulFlags |= VMDHA_USEHIGHMEM;

   rc = DevHelp_VMAlloc( ulFlags,
                tSize,
                -1,
                &ulAddress,
                (VOID **)&pv);
   if (rc)
      {
      if (f32Parms.fMessageActive & LOG_FUNCS)
         Message("ERROR: linalloc failed, rc = %d", rc);

      if( !fIgnore )
        CritMessage("linalloc failed, rc = %d", rc);

      return 0xFFFFFFFF;
      }
   return ulAddress;
}

/******************************************************************
*
******************************************************************/
void freeseg(void *p)
{
USHORT usSel;
USHORT rc;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("freeseg");

   if (!p)
      return;

   rc = MY_PROBEBUF(PB_OPREAD, p, 1);
   if (rc)
      {
      CritMessage("FAT32: freeseg: protection violation!");
      Message("ERROR: freeseg: protection violation");
      return;
      }

   usSel = SELECTOROF(p);
   rc = FSH_SEGFREE(usSel);
   if (rc)
      {
      CritMessage("FAT32: FSH_SEGFREE failed for selector %X", usSel);
      Message("ERROR: FSH_SEGFREE failed");
      }
   else
      f32Parms.usSegmentsAllocated--;
}



/******************************************************************
*
******************************************************************/
USHORT ReadCluster(PVOLINFO pVolInfo, ULONG ulCluster, PVOID pbCluster, USHORT usIOMode)
{
ULONG ulSector;
USHORT rc;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("ReadCluster %lu", ulCluster);

   if (ulCluster < 2 || ulCluster >= pVolInfo->ulTotalClusters + 2)
      {
      CritMessage("ERROR: Cluster %lu does not exist on disk %c:",
         ulCluster, pVolInfo->bDrive + 'A');
      Message("ERROR: Cluster %lu (%lX) does not exist on disk %c:",
         ulCluster, ulCluster, pVolInfo->bDrive + 'A');
      return ERROR_SECTOR_NOT_FOUND;
      }

   //Message("ulStartOfData=%lu, ulCluster=%lu, SectorsPerCluster=%u",
   //        pVolInfo->ulStartOfData, ulCluster, pVolInfo->BootSect.bpb.SectorsPerCluster);

   ulSector = pVolInfo->ulStartOfData +
      (ulCluster - 2) * pVolInfo->BootSect.bpb.SectorsPerCluster;

   rc = ReadSector(pVolInfo, ulSector,
      pVolInfo->BootSect.bpb.SectorsPerCluster,
      pbCluster, usIOMode);
   if (rc)
      {
      Message("ReadCluster: Cluster %lu failed!", ulCluster);
      return rc;
      }

   return 0;
}

/******************************************************************
*
******************************************************************/
USHORT WriteCluster(PVOLINFO pVolInfo, ULONG ulCluster, PVOID pbCluster, USHORT usIOMode)
{
ULONG ulSector;
USHORT rc;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("WriteCluster");

   if (ulCluster < 2 || ulCluster >= pVolInfo->ulTotalClusters + 2)
      {
      CritMessage("ERROR: Cluster %ld does not exist on disk %c:",
         ulCluster, pVolInfo->bDrive + 'A');
      Message("ERROR: Cluster %ld does not exist on disk %c:",
         ulCluster, pVolInfo->bDrive + 'A');
      return ERROR_SECTOR_NOT_FOUND;
      }

   ulSector = pVolInfo->ulStartOfData +
      (ulCluster - 2) * pVolInfo->BootSect.bpb.SectorsPerCluster;

   rc = WriteSector(pVolInfo, ulSector,
      pVolInfo->BootSect.bpb.SectorsPerCluster,
      pbCluster, usIOMode);
   if (rc)
      {
      Message("WriteCluster: Cluster %lu failed!", ulCluster);
      return rc;
      }

   return 0;
}

/******************************************************************
*
******************************************************************/
USHORT ReadFatSector(PVOLINFO pVolInfo, ULONG ulSector)
{
USHORT rc;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("ReadFatSector");

   if (pVolInfo->ulCurFatSector == ulSector)
      return 0;

   if (ulSector >= pVolInfo->BootSect.bpb.BigSectorsPerFat)
      {
      CritMessage("ERROR: ReadFatSector: Sector %lu too high", ulSector);
      Message("ERROR: ReadFatSector: Sector %lu too high", ulSector);
      return ERROR_SECTOR_NOT_FOUND;
      }

   rc = ReadSector(pVolInfo, pVolInfo->ulActiveFatStart + ulSector, 1,
      pVolInfo->pbFatSector, 0);
   if (rc)
      return rc;

   pVolInfo->ulCurFatSector = ulSector;

   return 0;
}

/******************************************************************
*
******************************************************************/
USHORT WriteFatSector(PVOLINFO pVolInfo, ULONG ulSector)
{
USHORT usFat;
USHORT rc;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("WriteFatSector");

   if (pVolInfo->ulCurFatSector != ulSector)
      {
      CritMessage("FAT32: WriteFatSector: Sectors do not match!");
      Message("ERROR: WriteFatSector: Sectors do not match!");
      return ERROR_SECTOR_NOT_FOUND;
      }

   if (ulSector >= pVolInfo->BootSect.bpb.BigSectorsPerFat)
      {
      CritMessage("ERROR: WriteFatSector: Sector %ld too high", ulSector);
      Message("ERROR: WriteFatSector: Sector %ld too high", ulSector);
      return ERROR_SECTOR_NOT_FOUND;
      }

   for (usFat = 0; usFat < pVolInfo->BootSect.bpb.NumberOfFATs; usFat++)
      {
      rc = WriteSector(pVolInfo, pVolInfo->ulActiveFatStart + ulSector, 1,
         pVolInfo->pbFatSector, 0);
      if (rc)
         return rc;

      if (pVolInfo->BootSect.bpb.ExtFlags & 0x0080)
         break;
      ulSector += pVolInfo->BootSect.bpb.BigSectorsPerFat;
      }

   return 0;
}

/******************************************************************
*
******************************************************************/
ULONG GetNextCluster(PVOLINFO pVolInfo, ULONG ulCluster)
{

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("GetNextCluster for %lu", ulCluster);

   if (!GetFatAccess(pVolInfo, "GetNextCluster"))
      {
      ulCluster = GetNextCluster2(pVolInfo, ulCluster);
      ReleaseFat(pVolInfo);
      return ulCluster;
      }
   else
      return FAT_EOF;
}

/******************************************************************
*
******************************************************************/
ULONG GetNextCluster2(PVOLINFO pVolInfo, ULONG ulCluster)
{
PULONG pulCluster;

   if (ReadFatSector(pVolInfo, ulCluster / 128))
      {
      Message("GetNextCluster for %ld failed", ulCluster);
      return FAT_EOF;
      }

   pulCluster = (PULONG)pVolInfo->pbFatSector + (ulCluster % 128);

   ulCluster = *pulCluster & FAT_EOF;
   if (ulCluster >= FAT_EOF2 && ulCluster <= FAT_EOF)
      return FAT_EOF;
   return ulCluster;
}

/******************************************************************
*
******************************************************************/
ULONG GetFreeSpace(PVOLINFO pVolInfo)
{
ULONG ulSector;
ULONG ulCluster;
ULONG ulTotalFree;
PULONG pulCluster;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("GetFreeSpace");

   if (GetFatAccess(pVolInfo, "GetFreeSpace"))
      return 0L;

   ulTotalFree = 0;
   for (ulCluster = 2; ulCluster < pVolInfo->ulTotalClusters + 2; ulCluster++)
      {
      ulSector = ulCluster / 128;
      if (ulSector != pVolInfo->ulCurFatSector)
         ReadFatSector(pVolInfo, ulSector);
      pulCluster = (PULONG)pVolInfo->pbFatSector + (ulCluster % 128);
      if ((*pulCluster & FAT_EOF) == 0)
         ulTotalFree++;
      }

   if (pVolInfo->pBootFSInfo->ulFreeClusters != ulTotalFree)
      {
      pVolInfo->pBootFSInfo->ulFreeClusters = ulTotalFree;
      /* UpdateFSInfo(pVolInfo); */
      }

   ReleaseFat(pVolInfo);
   return ulTotalFree;

}
/******************************************************************
*
******************************************************************/
ULONG MakeFatChain(PVOLINFO pVolInfo, ULONG ulPrevCluster, ULONG ulClustersRequested, PULONG pulLast)
{
ULONG  ulCluster;
ULONG  ulFirstCluster;
ULONG  ulStartCluster;
ULONG  ulLargestChain;
ULONG  ulLargestSize;
ULONG  ulReturn;
ULONG  ulSector;
PULONG pulCluster;
BOOL   fStartAt2;
BOOL   fContiguous;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("MakeFatChain, %lu clusters", ulClustersRequested);

   if (!ulClustersRequested)
      return FAT_EOF;

   if (GetFatAccess(pVolInfo, "MakeFatChain"))
      return FAT_EOF;

   if (pVolInfo->pBootFSInfo->ulFreeClusters < ulClustersRequested)
      {
      ReleaseFat(pVolInfo);
      return FAT_EOF;
      }

   ulReturn = FAT_EOF;
   fContiguous = TRUE;
   for (;;)
      {
      ulLargestChain = FAT_EOF;
      ulLargestSize = 0;

      ulFirstCluster = pVolInfo->pBootFSInfo->ulNextFreeCluster + 1;
      if (ulFirstCluster < 2 || ulFirstCluster >= pVolInfo->ulTotalClusters + 2)
         {
         fStartAt2 = TRUE;
         ulFirstCluster = 2;
         ulStartCluster = pVolInfo->ulTotalClusters + 3;
         }
      else
         {
         ulStartCluster = ulFirstCluster;
         fStartAt2 = FALSE;
         }

      for (;;)
         {
#ifdef CALL_YIELD
         Yield();
#endif
         /*
            Find first free cluster
         */
         while (ulFirstCluster < pVolInfo->ulTotalClusters + 2)
            {
            ulSector = ulFirstCluster / 128;
            pulCluster = (PULONG)pVolInfo->pbFatSector + (ulFirstCluster % 128);
            if (ulSector != pVolInfo->ulCurFatSector)
               ReadFatSector(pVolInfo, ulSector);
            if (!(*pulCluster))
               break;
            ulFirstCluster++;
            }

         if (fStartAt2 && ulFirstCluster >= ulStartCluster)
            break;

         if (ulFirstCluster >= pVolInfo->ulTotalClusters + 2)
            {
            if (fStartAt2)
               break;
            if (f32Parms.fMessageActive & LOG_FUNCS)
               Message("No contiguous block found, restarting at cluster 2");
            ulFirstCluster = 2;
            fStartAt2 = TRUE;
            continue;
            }


         /*
            Check if chain is long enough
         */

         for (ulCluster = ulFirstCluster ;
                  ulCluster < ulFirstCluster + ulClustersRequested &&
                  ulCluster < pVolInfo->ulTotalClusters + 2;
                        ulCluster++)
            {
            ulSector = ulCluster / 128;
            pulCluster = (PULONG)pVolInfo->pbFatSector + (ulCluster % 128);
            if (ulSector != pVolInfo->ulCurFatSector)
               ReadFatSector(pVolInfo, ulSector);
            if (*pulCluster)
               break;
            }

         if (ulCluster != ulFirstCluster + ulClustersRequested)
            {
            /*
               Keep the largests chain found
            */
            if (ulCluster - ulFirstCluster > ulLargestSize)
               {
               ulLargestChain = ulFirstCluster;
               ulLargestSize  = ulCluster - ulFirstCluster;
               }
            ulFirstCluster = ulCluster;
            continue;
            }

         /*
            Chain found long enough
         */
         if (ulReturn == FAT_EOF)
            ulReturn = ulFirstCluster;

         if (MakeChain(pVolInfo, ulFirstCluster, ulClustersRequested))
            goto MakeFatChain_Error;

         if (ulPrevCluster != FAT_EOF)
            {
            if (SetNextCluster2(pVolInfo, ulPrevCluster, ulFirstCluster) == FAT_EOF)
               goto MakeFatChain_Error;
            }

         ReleaseFat(pVolInfo);
         if (f32Parms.fMessageActive & LOG_FUNCS)
            {
            if (fContiguous)
               Message("Contiguous chain returned, first = %lu", ulReturn);
            else
               Message("NON Contiguous chain returned, first = %lu", ulReturn);
            }
         if (pulLast)
            *pulLast = ulFirstCluster + ulClustersRequested - 1;
         return ulReturn;
         }

      /*
         We get here only if no free chain long enough was found!
      */
      if (f32Parms.fMessageActive & LOG_FUNCS)
         Message("No contiguous block found, largest found is %lu clusters", ulLargestSize);
      fContiguous = FALSE;

      if (ulLargestChain != FAT_EOF)
         {
         ulFirstCluster = ulLargestChain;
         if (ulReturn == FAT_EOF)
            ulReturn = ulFirstCluster;

         if (MakeChain(pVolInfo, ulFirstCluster, ulLargestSize))
            goto MakeFatChain_Error;

         if (ulPrevCluster != FAT_EOF)
            {
            if (SetNextCluster2(pVolInfo, ulPrevCluster, ulFirstCluster) == FAT_EOF)
               goto MakeFatChain_Error;
            }

         ulPrevCluster        = ulFirstCluster + ulLargestSize - 1;
         ulClustersRequested -= ulLargestSize;
         }
      else
         break;
      }

MakeFatChain_Error:

   ReleaseFat(pVolInfo);
   if (ulReturn != FAT_EOF)
      DeleteFatChain(pVolInfo, ulReturn);

   return FAT_EOF;
}

/******************************************************************
*
******************************************************************/
USHORT MakeChain(PVOLINFO pVolInfo, ULONG ulFirstCluster, ULONG ulSize)
{
ULONG ulSector;
ULONG ulLastCluster;
PULONG pulCluster;
ULONG  ulCluster;
USHORT rc;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("MakeChain");

   ulLastCluster = ulFirstCluster + ulSize - 1;

   ulSector = ulFirstCluster / 128;
   if (ulSector != pVolInfo->ulCurFatSector)
      ReadFatSector(pVolInfo, ulSector);

   for (ulCluster = ulFirstCluster; ulCluster < ulLastCluster; ulCluster++)
      {
      ulSector = ulCluster / 128;
      if (ulSector != pVolInfo->ulCurFatSector)
         {
         rc = WriteFatSector(pVolInfo, pVolInfo->ulCurFatSector);
         if (rc)
            return rc;
         ReadFatSector(pVolInfo, ulSector);
         }
      pulCluster = (PULONG)pVolInfo->pbFatSector + (ulCluster % 128);
      if (*pulCluster)
         {
         CritMessage("FAT32:MakeChain:Cluster %lu is not free!", ulCluster);
         Message("ERROR:MakeChain:Cluster %lu is not free!", ulCluster);
         return ERROR_SECTOR_NOT_FOUND;
         }
      *pulCluster = ulCluster + 1;
      }

   ulSector = ulCluster / 128;
   if (ulSector != pVolInfo->ulCurFatSector)
      {
      rc = WriteFatSector(pVolInfo, pVolInfo->ulCurFatSector);
      if (rc)
         return rc;
      ReadFatSector(pVolInfo, ulSector);
      }
   pulCluster = (PULONG)pVolInfo->pbFatSector + (ulCluster % 128);
   if (*pulCluster)
      {
      CritMessage("FAT32:MakeChain:Cluster %lu is not free!", ulCluster);
      Message("ERROR:MakeChain:Cluster %lu is not free!", ulCluster);
      return ERROR_SECTOR_NOT_FOUND;
      }

   *pulCluster = FAT_EOF;
   rc = WriteFatSector(pVolInfo, pVolInfo->ulCurFatSector);
   if (rc)
      return rc;

   pVolInfo->pBootFSInfo->ulNextFreeCluster = ulCluster;
   pVolInfo->pBootFSInfo->ulFreeClusters   -= ulSize;

   return 0;
}

/******************************************************************
*
******************************************************************/
BOOL UpdateFSInfo(PVOLINFO pVolInfo)
{
static BYTE bSector[SECTOR_SIZE] = "";

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("UpdateFSInfo");

   if (pVolInfo->fFormatInProgress)
      return FALSE;

   if (pVolInfo->fWriteProtected)
      return TRUE;

   if (pVolInfo->BootSect.bpb.FSinfoSec == 0xFFFF)
      return TRUE;

   if (!ReadSector(pVolInfo, pVolInfo->BootSect.bpb.FSinfoSec, 1, bSector, DVIO_OPNCACHE))
      {
      memcpy(bSector + FSINFO_OFFSET, pVolInfo->pBootFSInfo, sizeof (BOOTFSINFO));
      if (!WriteSector(pVolInfo, pVolInfo->BootSect.bpb.FSinfoSec, 1, bSector, DVIO_OPNCACHE | DVIO_OPWRTHRU))
         return TRUE;
      }
   CritMessage("UpdateFSInfo for %c: failed!", pVolInfo->bDrive + 'A');
   Message("ERROR: UpdateFSInfo for %c: failed!", pVolInfo->bDrive + 'A');

   return FALSE;
}

/******************************************************************
*
******************************************************************/
BOOL MarkDiskStatus(PVOLINFO pVolInfo, BOOL fClean)
{
static BYTE bSector[SECTOR_SIZE] = "";
PULONG pulStatus;
ULONG ulSector;
USHORT usFat;
PBYTE pbSector;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("MarkDiskStatus, %d", fClean);

   if (!pVolInfo->fDiskCleanOnMount && fClean)
      return TRUE;
   if (pVolInfo->fWriteProtected)
      return TRUE;

   if (pVolInfo->ulCurFatSector != 0)
      {
      if (ReadSector(pVolInfo, pVolInfo->ulActiveFatStart, 1,
         bSector, DVIO_OPNCACHE))
         return FALSE;
      pbSector = bSector;
      pulStatus = (PULONG)bSector + 1;
      }
   else
      {
      pulStatus = (PULONG)pVolInfo->pbFatSector + 1;
      pbSector = pVolInfo->pbFatSector;
      }

   if (fClean)
      (*pulStatus) |= FAT32_CLEAN_SHUTDOWN;
   else
      (*pulStatus) &= ~FAT32_CLEAN_SHUTDOWN;

   /*
      Trick, set fDiskClean to FALSE, so WriteSector
      won't set is back to dirty again
   */
   pVolInfo->fDiskClean = FALSE;

   ulSector = 0L;
   for (usFat = 0; usFat < pVolInfo->BootSect.bpb.NumberOfFATs; usFat++)
      {
      if (WriteSector(pVolInfo, pVolInfo->ulActiveFatStart + ulSector, 1,
         pbSector, DVIO_OPWRTHRU | DVIO_OPNCACHE))
         {
         ReleaseFat(pVolInfo);
         return FALSE;
         }
      if (pVolInfo->BootSect.bpb.ExtFlags & 0x0080)
         break;
      ulSector += pVolInfo->BootSect.bpb.BigSectorsPerFat;
      }

   pVolInfo->fDiskClean = fClean;

   return TRUE;

}

/******************************************************************
*
******************************************************************/
BOOL GetDiskStatus(PVOLINFO pVolInfo)
{
PULONG pulStatus;
BOOL   fStatus;

   if (GetFatAccess(pVolInfo, "GetDiskStatus"))
      return FALSE;
   if (ReadFatSector(pVolInfo, 0L))
      {
      ReleaseFat(pVolInfo);
      return FALSE;
      }
   pulStatus = (PULONG)pVolInfo->pbFatSector + 1;

   if (*pulStatus & FAT32_CLEAN_SHUTDOWN)
      fStatus = TRUE;
   else
      fStatus = FALSE;
   ReleaseFat(pVolInfo);

   return fStatus;
}

/******************************************************************
*
******************************************************************/
ULONG SetNextCluster(PVOLINFO pVolInfo, ULONG ulCluster, ULONG ulNext)
{
   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("SetNextCluster");


   if (GetFatAccess(pVolInfo, "SetNextCluster"))
      return FAT_EOF;

   ulCluster = SetNextCluster2(pVolInfo, ulCluster, ulNext);
   ReleaseFat(pVolInfo);
   return ulCluster;
}

/******************************************************************
*
******************************************************************/
ULONG SetNextCluster2(PVOLINFO pVolInfo, ULONG ulCluster, ULONG ulNext)
{
PULONG pulCluster;
BOOL fUpdateFSInfo;
ULONG ulReturn;
USHORT rc;

   ulReturn = ulNext;
   if (ulCluster == FAT_ASSIGN_NEW)
      {
      /*
         A new seperate CHAIN is started.
      */
      ulCluster = GetFreeCluster(pVolInfo);
      if (ulCluster == FAT_EOF)
         return FAT_EOF;
      ulReturn = ulCluster;
      ulNext = FAT_EOF;
      }

   else if (ulNext == FAT_ASSIGN_NEW)
      {
      /*
         An existing chain is extended
      */
      ulNext = SetNextCluster2(pVolInfo, FAT_ASSIGN_NEW, FAT_EOF);
      if (ulNext == FAT_EOF)
         return FAT_EOF;
      ulReturn = ulNext;
      }

   if (ReadFatSector(pVolInfo, ulCluster / 128))
      return FAT_EOF;

   fUpdateFSInfo = FALSE;
   pulCluster = (PULONG)pVolInfo->pbFatSector + (ulCluster % 128);
   if (*pulCluster && !ulNext)
      {
      fUpdateFSInfo = TRUE;
      pVolInfo->pBootFSInfo->ulFreeClusters++;
      }
   if (*pulCluster == 0 && ulNext)
      {
      fUpdateFSInfo = TRUE;
      pVolInfo->pBootFSInfo->ulNextFreeCluster = ulCluster;
      pVolInfo->pBootFSInfo->ulFreeClusters--;
      }

   *pulCluster = ulNext;

   rc = WriteFatSector(pVolInfo, ulCluster / 128);
   if (rc)
      return FAT_EOF;

/*
   if (fUpdateFSInfo)
      UpdateFSInfo(pVolInfo);
*/

   return ulReturn;
}


/******************************************************************
*
******************************************************************/
ULONG GetFreeCluster(PVOLINFO pVolInfo)
{
ULONG ulStartCluster;
ULONG ulCluster;
BOOL fStartAt2;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("GetFreeCluster");

   if (pVolInfo->pBootFSInfo->ulFreeClusters == 0L)
      return FAT_EOF;

   fStartAt2 = FALSE;
   ulCluster = pVolInfo->pBootFSInfo->ulNextFreeCluster + 1;
   if (!ulCluster || ulCluster >= pVolInfo->ulTotalClusters + 2)
      {
      fStartAt2 = TRUE;
      ulCluster = 2;
      ulStartCluster = pVolInfo->ulTotalClusters + 2;
      }
   else
      ulStartCluster = ulCluster;

   while (GetNextCluster2(pVolInfo, ulCluster))
      {
      ulCluster++;
      if (fStartAt2 && ulCluster >= ulStartCluster)
         return FAT_EOF;

      if (ulCluster >= pVolInfo->ulTotalClusters + 2)
         {
         if (!fStartAt2)
            {
            ulCluster = 2;
            fStartAt2 = TRUE;
            }
         else
            return FAT_EOF;
         }
      }
   return ulCluster;
}

/******************************************************************
*
******************************************************************/
PVOLINFO GetVolInfo(USHORT hVBP)
{
INT rc;
struct vpfsi far * pvpfsi;
struct vpfsd far * pvpfsd;
PVOLINFO pVolInfo;

   rc = FSH_GETVOLPARM(hVBP, &pvpfsi, &pvpfsd);
   if (rc)
      {
      FatalMessage("FAT32:GetVolInfo - Volinfo not found!");
      return pGlobVolInfo;
      }

   pVolInfo = *(PVOLINFO *)pvpfsd;
   rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pVolInfo, sizeof (VOLINFO));
   if (rc)
      {
      FatalMessage("FAT32: Error VOLINFO invalid in GetVolInfo (ProtViol)");
      return pGlobVolInfo;
      }
   pVolInfo->hVBP = hVBP;
   return pVolInfo;
}

ULONG GetVolDevice(PVOLINFO pVolInfo)
{
INT rc;
struct vpfsi far * pvpfsi;
struct vpfsd far * pvpfsd;

   rc = FSH_GETVOLPARM(pVolInfo->hVBP, &pvpfsi, &pvpfsd);
   if (rc)
      return 0L;
   return pvpfsi->vpi_hDEV;
}

/******************************************************************
*
******************************************************************/
ULONG FindDirCluster(PVOLINFO pVolInfo,
   struct cdfsi far * pcdfsi,       /* pcdfsi   */
   struct cdfsd far * pcdfsd,       /* pcdfsd   */
   PSZ pDir,
   USHORT usCurDirEnd,
   USHORT usAttrWanted,
   PSZ *pDirEnd)
{
BYTE   szDir[FAT32MAXPATH];
ULONG  ulCluster;
ULONG  ulCluster2;
DIRENTRY DirEntry;
PSZ    p;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("FindDirCluster for %s, CurDirEnd %u, AttrWanted %u", pDir, usCurDirEnd, usAttrWanted );

   if (pcdfsi &&
      (pcdfsi->cdi_flags & CDI_ISVALID) &&
      !(pcdfsi->cdi_flags & CDI_ISROOT) &&
      usCurDirEnd != 0xFFFF)
      {
      pDir += usCurDirEnd;
      ulCluster = *(PULONG)pcdfsd;
      }
   else
      {
      ulCluster = pVolInfo->BootSect.bpb.RootDirStrtClus;
      if (strlen(pDir) >= 2)
         {
         if (pDir[1] == ':')
            pDir += 2;
         }
      }

   if (*pDir == '\\')
      pDir++;

   p = strrchr(pDir, '\\');
   if (!p)
      p = pDir;
   memset(szDir, 0, sizeof szDir);
   memcpy(szDir, pDir, p - pDir);
   if (*p && p != pDir)
      pDir = p + 1;
   else
      pDir = p;

   if (pDirEnd)
      *pDirEnd = pDir;
   ulCluster = FindPathCluster(pVolInfo, ulCluster, szDir, &DirEntry, NULL);
   if (ulCluster == FAT_EOF)
      {
      if (f32Parms.fMessageActive & LOG_FUNCS)
         Message("FindDirCluster for '%s', not found", szDir);
      return FAT_EOF;
      }
   if (ulCluster != FAT_EOF && !(DirEntry.bAttr & FILE_DIRECTORY))
      {
      if (f32Parms.fMessageActive & LOG_FUNCS)
         Message("FindDirCluster for '%s', not a directory", szDir);
      return FAT_EOF;
      }

   if (*pDir)
      {
      if (usAttrWanted != RETURN_PARENT_DIR && !strpbrk(pDir, "?*"))
         {
         ulCluster2 = FindPathCluster(pVolInfo, ulCluster, pDir, &DirEntry, NULL);
         if (ulCluster2 != FAT_EOF && (DirEntry.bAttr & usAttrWanted) == usAttrWanted)
            {
            if (pDirEnd)
               *pDirEnd = pDir + strlen(pDir);
            return ulCluster2;
            }
         }
      }

   return ulCluster;
}

#define MODE_START  0
#define MODE_RETURN 1
#define MODE_SCAN   2

/******************************************************************
*
******************************************************************/
ULONG FindPathCluster(PVOLINFO pVolInfo, ULONG ulCluster, PSZ pszPath, PDIRENTRY pDirEntry, PSZ pszFullName)
{
BYTE szShortName[13];
PSZ  pszLongName;
PSZ  pszPart;
PSZ  p;
PDIRENTRY pDir;
PDIRENTRY pDirStart;
PDIRENTRY pDirEnd;
BOOL fFound;
USHORT usMode;
BYTE   bCheck;
PROCINFO ProcInfo;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("FindPathCluster for %s, dircluster %lu", pszPath, ulCluster);

   if (pDirEntry)
      {
      memset(pDirEntry, 0, sizeof (DIRENTRY));
      pDirEntry->bAttr = FILE_DIRECTORY;
      }
   if (pszFullName)
      {
      memset(pszFullName, 0, FAT32MAXPATH);
      if (ulCluster == pVolInfo->BootSect.bpb.RootDirStrtClus)
         {
         pszFullName[0] = (BYTE)(pVolInfo->bDrive + 'A');
         pszFullName[1] = ':';
         pszFullName[2] = '\\';
         }
      }

   if (strlen(pszPath) >= 2)
      {
      if (pszPath[1] == ':')
         pszPath += 2;
      }

   pDirStart = malloc(pVolInfo->usClusterSize);
   if (!pDirStart)
      {
      Message("FAT32: Not enough memory for cluster in FindPathCluster");
      return FAT_EOF;
      }
   pszLongName = malloc(FAT32MAXPATHCOMP * 2);
   if (!pszLongName)
      {
      Message("FAT32: Not enough memory for buffers in FindPathCluster");
      free(pDirStart);
      return FAT_EOF;
      }
   memset(pszLongName, 0, FAT32MAXPATHCOMP * 2);
   pszPart = pszLongName + FAT32MAXPATHCOMP;

   usMode = MODE_SCAN;
   GetProcInfo(&ProcInfo, sizeof ProcInfo);
   /*
      Allow EA files to be found!
   */
   if (ProcInfo.usPdb && f32Parms.fEAS && IsEASFile(pszPath))
      ProcInfo.usPdb = 0;

   while (usMode != MODE_RETURN && ulCluster != FAT_EOF)
      {
      usMode = MODE_SCAN;

      if (*pszPath == '\\')
         pszPath++;

      if (!strlen(pszPath))
         break;

      p = strchr(pszPath, '\\');
      if (!p)
         p = pszPath + strlen(pszPath);

      memset(pszPart, 0, FAT32MAXPATHCOMP);
      if (p - pszPath > FAT32MAXPATHCOMP - 1)
         {
         free(pDirStart);
         free(pszLongName);
         return FAT_EOF;
         }

      memcpy(pszPart, pszPath, p - pszPath);
      pszPath = p;

      memset(pszLongName, 0, FAT32MAXPATHCOMP);

      fFound = FALSE;
      while (usMode == MODE_SCAN && ulCluster != FAT_EOF)
         {
         ReadCluster(pVolInfo, ulCluster, pDirStart, 0);
         pDir    = pDirStart;
         pDirEnd = (PDIRENTRY)((PBYTE)pDirStart + pVolInfo->usClusterSize);

#ifdef CALL_YIELD
         Yield();
#endif

         while (usMode == MODE_SCAN && pDir < pDirEnd)
            {
            if (pDir->bAttr == FILE_LONGNAME)
               {
               fGetLongName(pDir, pszLongName, FAT32MAXPATHCOMP, &bCheck);
               }
            else if ((pDir->bAttr & 0x0F) != FILE_VOLID)
               {
               MakeName(pDir, szShortName, sizeof szShortName);
               FSH_UPPERCASE(szShortName, sizeof szShortName, szShortName);
               if (strlen(pszLongName) && bCheck != GetVFATCheckSum(pDir))
                  memset(pszLongName, 0, FAT32MAXPATHCOMP);

                /* support for the FAT32 variation of WinNT family */
                if( !*pszLongName && HAS_WINNT_EXT( pDir->fEAS ))
                {
                    PBYTE pDot;

                    MakeName( pDir, pszLongName, sizeof( pszLongName ));
                    pDot = strchr( pszLongName, '.' );

                    if( HAS_WINNT_EXT_NAME( pDir->fEAS )) /* name part is lower case */
                    {
                        if( pDot )
                            *pDot = 0;

                        strlwr( pszLongName );

                        if( pDot )
                            *pDot = '.';
                    }

                    if( pDot && HAS_WINNT_EXT_EXT( pDir->fEAS )) /* ext part is lower case */
                        strlwr( pDot + 1 );
                }

               if (!strlen(pszLongName))
                  strcpy(pszLongName, szShortName);

               if (( strlen(pszLongName) && !stricmp(pszPart, pszLongName)) ||
                   !stricmp( pszPart, szShortName ))
                  {
                    if( pszFullName )
                        strcat( pszFullName, pszLongName );
                    fFound = TRUE;
                  }

               if (fFound)
                  {
                  ulCluster = (ULONG)pDir->wClusterHigh * 0x10000L + pDir->wCluster;
                  if (strlen(pszPath))
                     {
                     if (pDir->bAttr & FILE_DIRECTORY)
                        {
                        if (pszFullName)
                           strcat(pszFullName, "\\");
                        usMode = MODE_START;
                        break;
                        }
                     ulCluster = FAT_EOF;
                     }
                  else
                     {
                     if (pDirEntry)
                        memcpy(pDirEntry, pDir, sizeof (DIRENTRY));
                     }
                  usMode = MODE_RETURN;
                  break;
                  }
               memset(pszLongName, 0, FAT32MAXPATHCOMP);
               }
            pDir++;
            }
         if (usMode != MODE_SCAN)
            break;
         ulCluster = GetNextCluster(pVolInfo, ulCluster);
         if (!ulCluster)
            ulCluster = FAT_EOF;
         }
      }
   free(pDirStart);
   free(pszLongName);
   if (f32Parms.fMessageActive & LOG_FUNCS)
      {
      if (ulCluster != FAT_EOF)
         Message("FindPathCluster for %s found cluster %ld", pszPath, ulCluster);
      else
         Message("FindPathCluster for %s returned EOF", pszPath);
      }
   return ulCluster;
}


USHORT TranslateName(PVOLINFO pVolInfo, ULONG ulDirCluster, PSZ pszPath, PSZ pszTarget, USHORT usTranslate)
{
BYTE szShortName[13];
PSZ  pszLongName;
PSZ  pszUpperName;
PSZ  pszUpperPart;
PSZ  pszPart;
PSZ  p;
/* PSZ  pTar = pszTarget; */
PDIRENTRY pDir;
PDIRENTRY pDirStart;
PDIRENTRY pDirEnd;
BOOL fFound;
USHORT usMode;
BYTE   bCheck;
ULONG  ulCluster;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("TranslateName: %s", pszPath);

   memset(pszTarget, 0, FAT32MAXPATH);
   if (strlen(pszPath) >= 2)
      {
      if (pszPath[1] == ':')
         {
         memcpy(pszTarget, pszPath, 2);
         pszTarget += 2;
         pszPath += 2;
         }
      }

   pDirStart = malloc(pVolInfo->usClusterSize);
   if (!pDirStart)
      {
      Message("FAT32: Not enough memory for cluster in TranslateName");
      return ERROR_NOT_ENOUGH_MEMORY;
      }
   pszLongName = malloc(FAT32MAXPATHCOMP * 4);
   if (!pszLongName)
      {
      Message("FAT32: Not enough memory for buffers in TranslateName");
      free(pDirStart);
      return ERROR_NOT_ENOUGH_MEMORY;
      }
   memset(pszLongName, 0, FAT32MAXPATHCOMP * 4);

   pszPart      = pszLongName + FAT32MAXPATHCOMP;
   pszUpperPart = pszPart + FAT32MAXPATHCOMP;
   pszUpperName = pszUpperPart + FAT32MAXPATHCOMP;

   if (usTranslate == TRANSLATE_AUTO)
      {
      if (IsDosSession())
         usTranslate = TRANSLATE_SHORT_TO_LONG;
      else
         usTranslate = TRANSLATE_LONG_TO_SHORT;
      }

   if (ulDirCluster)
      ulCluster = ulDirCluster;
   else
      ulCluster = pVolInfo->BootSect.bpb.RootDirStrtClus;

   usMode = MODE_SCAN;
   while (usMode != MODE_RETURN && ulCluster != FAT_EOF)
      {
      usMode = MODE_SCAN;

      if (*pszPath == '\\')
         *pszTarget++ = *pszPath++;

      if (!strlen(pszPath))
         break;

      p = strchr(pszPath, '\\');
      if (!p)
         p = pszPath + strlen(pszPath);

      if (p - pszPath > FAT32MAXPATHCOMP - 1)
         {
         free(pDirStart);
         free(pszLongName);
         return ERROR_BUFFER_OVERFLOW;
         }

      memset(pszPart, 0, FAT32MAXPATHCOMP);
      memcpy(pszPart, pszPath, p - pszPath);
      FSH_UPPERCASE(pszPart, FAT32MAXPATHCOMP, pszUpperPart);
      pszPath = p;

      memset(pszLongName, 0, FAT32MAXPATHCOMP);
      fFound = FALSE;
      while (usMode == MODE_SCAN && ulCluster != FAT_EOF)
         {
         ReadCluster(pVolInfo, ulCluster, pDirStart, 0);
         pDir    = pDirStart;
         pDirEnd = (PDIRENTRY)((PBYTE)pDirStart + pVolInfo->usClusterSize);

#ifdef CALL_YIELD
         Yield();
#endif

         while (usMode == MODE_SCAN && pDir < pDirEnd)
            {
            if (pDir->bAttr == FILE_LONGNAME)
               {
               fGetLongName(pDir, pszLongName, FAT32MAXPATHCOMP, &bCheck);
               }
            else if ((pDir->bAttr & 0x0F) != FILE_VOLID)
               {

               MakeName(pDir, szShortName, sizeof szShortName);
               FSH_UPPERCASE(szShortName, sizeof szShortName, szShortName);

               if (bCheck != GetVFATCheckSum(pDir))
                  memset(pszLongName, 0, FAT32MAXPATHCOMP);

                /* support for the FAT32 variation of WinNT family */
                if( !*pszLongName && HAS_WINNT_EXT( pDir->fEAS ))
                {
                    PBYTE pDot;

                    MakeName( pDir, pszLongName, sizeof( pszLongName ));
                    pDot = strchr( pszLongName, '.' );

                    if( HAS_WINNT_EXT_NAME( pDir->fEAS )) /* name part is lower case */
                    {
                        if( pDot )
                            *pDot = 0;

                        strlwr( pszLongName );

                        if( pDot )
                            *pDot = '.';
                    }

                    if( pDot && HAS_WINNT_EXT_EXT( pDir->fEAS )) /* ext part is lower case */
                        strlwr( pDot + 1 );
                }

               if (!strlen(pszLongName))
                  strcpy(pszLongName, szShortName);
               FSH_UPPERCASE(pszLongName, FAT32MAXPATHCOMP, pszUpperName);

               if (usTranslate == TRANSLATE_LONG_TO_SHORT) /* OS/2 session, translate to DOS */
                  {
                  if (!stricmp(pszUpperName, pszUpperPart) ||
                      !stricmp(szShortName,  pszUpperPart))
                     {
                     strcat(pszTarget, szShortName);
                     pszTarget += strlen(pszTarget);
                     fFound = TRUE;
                     }
                  }
               else /* translate from DOS to OS/2 */
                  {
                  if (!stricmp(szShortName,  pszUpperPart) ||
                      !stricmp(pszUpperName, pszUpperPart))
                     {
                     strcat(pszTarget, pszLongName);
                     pszTarget += strlen(pszTarget);
                     fFound = TRUE;
                     }
                  }

               if (fFound)
                  {
                  ulCluster = (ULONG)pDir->wClusterHigh * 0x10000L + pDir->wCluster;
                  if (strlen(pszPath))
                     {
                     if (pDir->bAttr & FILE_DIRECTORY)
                        {
                        usMode = MODE_START;
                        break;
                        }
                     ulCluster = FAT_EOF;
                     }
                  usMode = MODE_RETURN;
                  break;
                  }
               memset(pszLongName, 0, FAT32MAXPATHCOMP);
               }
            pDir++;
            }
         if (usMode != MODE_SCAN)
            break;
         ulCluster = GetNextCluster(pVolInfo, ulCluster);
         if (!ulCluster)
            ulCluster = FAT_EOF;
         if (ulCluster == FAT_EOF)
            strcat(pszTarget, pszPart);
         }
      }

   free(pDirStart);
   free(pszLongName);
   if (ulCluster == FAT_EOF)
      strcat(pszTarget, pszPath);
   return 0;
}


/******************************************************************
*
******************************************************************/
USHORT GetProcInfo(PPROCINFO pProcInfo, USHORT usSize)
{
USHORT rc;

   memset(pProcInfo, 0xFF, usSize);
   rc = FSH_QSYSINFO(2, (PBYTE)pProcInfo, 6);
   if (rc)
      FatalMessage("FAT32: GetProcInfo failed, rc = %d", rc);
   return rc;
}

/******************************************************************
*
******************************************************************/
BOOL RemoveFindEntry(PVOLINFO pVolInfo, PFINFO pFindInfo)
{
PFINFO pNext;
USHORT rc;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("RemoveFindEntry");

   if (pVolInfo->pFindInfo == pFindInfo)
      {
      pVolInfo->pFindInfo = pFindInfo->pNextEntry;
      return TRUE;
      }

   pNext = (PFINFO)pVolInfo->pFindInfo;
   while (pNext)
      {
      rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pNext, sizeof (FINDINFO));
      if (rc)
         {
         CritMessage("FAT32: Protection VIOLATION in RemoveFindEntry! (SYS%d)", rc);
         Message("FAT32: Protection VIOLATION in RemoveFindEntry! (SYS%d)", rc);
         return FALSE;
         }

      if (pNext->pNextEntry == pFindInfo)
         {
         pNext->pNextEntry = pFindInfo->pNextEntry;
         return TRUE;
         }
      pNext = (PFINFO)pNext->pNextEntry;
      }
   CritMessage("FAT32: RemoveFindEntry: Entry not found!");
   Message("ERROR: RemoveFindEntry: Entry not found!");
   return FALSE;
}

/******************************************************************
*
******************************************************************/
#ifdef __WATCOM
_WCRTLINK int sprintf(char * pszBuffer, const char * pszFormat, ...)
#else
int cdecl sprintf(char * pszBuffer, const char *pszFormat, ...)
#endif
{
va_list va;

   va_start(va, pszFormat);
   return vsprintf(pszBuffer, pszFormat, va);
}

/******************************************************************
*
******************************************************************/
USHORT cdecl CritMessage(PSZ pszMessage, ...)
{
static BYTE szMessage[512];
va_list va;

   memset(szMessage, 0, sizeof szMessage);
   va_start(va, pszMessage);
   vsprintf(szMessage, pszMessage, va);
   strcat(szMessage, "\r\n");
   return FSH_CRITERROR(strlen(szMessage) + 1, szMessage, 0, "", CE_ALLABORT | CE_ALLRETRY);
}

VOID cdecl FatalMessage(PSZ pszMessage, ...)
{
static BYTE szMessage[512];
va_list va;

   memset(szMessage, 0, sizeof szMessage);
   va_start(va, pszMessage);
   vsprintf(szMessage, pszMessage, va);
   strcat(szMessage, "\r\n");
   FSH_INTERR(szMessage, strlen(szMessage) + 1);
}

VOID InternalError(PSZ pszMessage)
{
   DevHelp_InternalError(pszMessage, strlen(pszMessage) + 1);
}

/******************************************************************
*
******************************************************************/
#ifdef __WATCOM
_WCRTLINK int vsprintf(char * pszBuffer, const char * pszFormat, va_list va)
#else
int cdecl vsprintf(char * pszBuffer, const char * pszFormat, va_list va)
#endif
{
PSZ p;
BOOL fLong = FALSE;
ULONG ulValue;
USHORT usValue;
PSZ   pszValue;
BYTE  bToken;


   for (;;)
      {
      p = strchr(pszFormat, '%');
      if (!p)
         break;
      strncpy(pszBuffer, pszFormat, p - pszFormat);
      pszBuffer[p - pszFormat] = 0;
      pszBuffer += strlen(pszBuffer);
      pszFormat = p;
      p++;
      fLong = FALSE;
      if (*p == 'l')
         {
         fLong = TRUE;
         p++;
         }
      bToken = *p;
      if (*p)
         p++;

      switch (bToken)
         {
         case '%':
            strcpy(pszBuffer, "%");
            break;
         case 'd':
         case 'u':
            if (fLong)
               ulValue = va_arg(va, unsigned long);
            else
               {
               usValue = va_arg(va, unsigned short);
               ulValue = usValue;
               }
            if (bToken == 'u')
               ultoa(ulValue, pszBuffer, 10);
            else
               ltoa(ulValue, pszBuffer, 10);
            break;
         case 'x':
         case 'X':
            if (fLong)
               ulValue = va_arg(va, unsigned long);
            else
               {
               usValue = va_arg(va, unsigned short);
               ulValue = usValue;
               }
            ultoa(ulValue, pszBuffer, 16);
            if (bToken == 'X')
               strupr(pszBuffer);
            break;
         case 's':
            pszValue = va_arg(va, char *);
            if (pszValue)
               {
               if (MY_PROBEBUF(PB_OPREAD, pszValue, 1))
                  strcpy(pszBuffer, "(bad address)");
               else
                  strcpy(pszBuffer, pszValue);
               }
            else
               strcpy(pszBuffer, "(null)");
            break;
         case 'c':
            pszBuffer[0] = (char)va_arg(va, USHORT);
            pszBuffer[1] = 0;
            break;
         default :
            strncpy(pszBuffer, pszFormat, p - pszFormat);
            pszBuffer[p - pszFormat] = 0;
            break;
         }
      pszBuffer += strlen(pszBuffer);
      pszFormat = p;
      }
   strcpy(pszBuffer, pszFormat);
   return 1;
}

/******************************************************************
*
******************************************************************/
BYTE GetVFATCheckSum(PDIRENTRY pDir)
{
BYTE bCheck;
INT  iIndex;

   bCheck = 0;
   for (iIndex = 0; iIndex < 11; iIndex++)
      {
      if (bCheck & 0x01)
         {
         bCheck >>=1;
         bCheck |= 0x80;
         }
      else
         bCheck >>=1;
      bCheck += pDir->bFileName[iIndex];
      }

   return bCheck;

}

/******************************************************************
*
******************************************************************/
PDIRENTRY fSetLongName(PDIRENTRY pDir, PSZ pszLongName, BYTE bCheck)
{
USHORT usNeededEntries;
PDIRENTRY pRet;
BYTE bCurEntry;
PLNENTRY pLN;
USHORT usIndex;
USHORT uniName[13];
USHORT uniEnd[13];
PUSHORT p;

   if (!pszLongName || !strlen(pszLongName))
      return pDir;

#if 0
   usNeededEntries = strlen(pszLongName) / 13 +
      (strlen(pszLongName) % 13 ? 1 : 0);
#else
   usNeededEntries = ( DBCSStrlen( pszLongName ) + 12 ) / 13;
#endif

   if (!usNeededEntries)
      return pDir;

   pDir += (usNeededEntries - 1);
   pRet = pDir + 1;
   pLN = (PLNENTRY)pDir;

   bCurEntry = 1;
   while (*pszLongName)
      {
#if 0
      USHORT usLen;
#endif
      pLN->bNumber = bCurEntry;
      if (DBCSStrlen(pszLongName) <= 13)
         pLN->bNumber += 0x40;
      pLN->wCluster = 0L;
      pLN->bAttr = FILE_LONGNAME;
      pLN->bReserved = 0;
      pLN->bVFATCheckSum = bCheck;

#if 0
      usLen = strlen(pszLongName);
      if (usLen > 13)
         usLen = 13;
#endif

      memset(uniEnd, 0xFF, sizeof uniEnd);
      memset(uniName, 0, sizeof uniName);

      pszLongName += Translate2Win(pszLongName, uniName, 13);

      p = uniName;
      for (usIndex = 0; usIndex < 5; usIndex ++)
         {
         pLN->usChar1[usIndex] = *p;
         if (*p == 0)
            p = uniEnd;
         p++;
         }

      for (usIndex = 0; usIndex < 6; usIndex ++)
         {
         pLN->usChar2[usIndex] = *p;
         if (*p == 0)
            p = uniEnd;
         p++;
         }

      for (usIndex = 0; usIndex < 2; usIndex ++)
         {
         pLN->usChar3[usIndex] = *p;
         if (*p == 0)
            p = uniEnd;
         p++;
         }

      pLN--;
      bCurEntry++;
      }

   return pRet;
}


/******************************************************************
*
******************************************************************/
USHORT MakeDirEntry(PVOLINFO pVolInfo, ULONG ulDirCluster, PDIRENTRY pNew, PSZ pszName)
{
   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("MakeDirEntry %s", pszName);

   if (pGI)
      {
      pNew->wLastWriteDate.year = pGI->year - 1980;
      pNew->wLastWriteDate.month = pGI->month;
      pNew->wLastWriteDate.day = pGI->day;
      pNew->wLastWriteTime.hours = pGI->hour;
      pNew->wLastWriteTime.minutes = pGI->minutes;
      pNew->wLastWriteTime.twosecs = pGI->seconds / 2;

      pNew->wCreateDate = pNew->wLastWriteDate;
      pNew->wCreateTime = pNew->wLastWriteTime;
      pNew->wAccessDate = pNew->wLastWriteDate;
      }

   return ModifyDirectory(pVolInfo, ulDirCluster, MODIFY_DIR_INSERT,
      NULL, pNew, pszName, 0);
}



/******************************************************************
*
******************************************************************/
USHORT MakeShortName(PVOLINFO pVolInfo, ULONG ulDirCluster, PSZ pszLongName, PSZ pszShortName)
{
USHORT usLongName;
PSZ pLastDot;
PSZ pFirstDot;
PSZ p;
USHORT usIndex;
BYTE szShortName[12];
PSZ  pszUpper;
USHORT rc;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("MakeShortName for %s, dircluster %lu",
         pszLongName, ulDirCluster);

   usLongName = LONGNAME_OFF;
   memset(szShortName, 0x20, 11);
   szShortName[11] = 0;

   /*
      Uppercase the longname
   */
   usIndex = strlen(pszLongName) + 5;
   pszUpper = malloc(usIndex);
   if (!pszUpper)
      return LONGNAME_ERROR;

   rc = FSH_UPPERCASE(pszLongName, usIndex, pszUpper);
   if (rc)
      {
      free(pszUpper);
      return LONGNAME_ERROR;
      }

   /* Skip all leading dots */
   p = pszUpper;
   while (*p == '.')
      p++;

   pLastDot  = strrchr(p, '.');
   pFirstDot = strchr(p, '.');

   if (!pLastDot)
      pLastDot = pszUpper + strlen(pszUpper);
   if (!pFirstDot)
      pFirstDot = pLastDot;

   /*
      Is the name a valid 8.3 name ?
   */
   if ((!strcmp(pszLongName, pszUpper) || IsDosSession()) &&
      pFirstDot == pLastDot &&
      pLastDot - pszUpper <= 8 &&
      strlen(pLastDot) <= 4)
      {
      PSZ p = pszUpper;

      if (*p != '.')
         {
         while (*p)
            {
            if (*p < 128 && !strchr(rgValidChars, *p) && *p != '.')
               break;
            p++;
            }
         }

      if (!(*p))
         {
         memset(szShortName, 0x20, sizeof szShortName);
         szShortName[11] = 0;
         memcpy(szShortName, pszUpper, pLastDot - pszUpper);
         if (*pLastDot)
            memcpy(szShortName + 8, pLastDot + 1, strlen(pLastDot + 1));

         memcpy(pszShortName, szShortName, 11);
         free(pszUpper);
         return usLongName;
         }
      }

#if 0
   if (IsDosSession())
      {
      free(pszUpper);
      return LONGNAME_ERROR;
      }
#endif

   usLongName = LONGNAME_OK;

   if (pLastDot - pszUpper > 8)
      usLongName = LONGNAME_MAKE_UNIQUE;

   szShortName[11] = 0;


   usIndex = 0;
   p = pszUpper;
   while (usIndex < 11)
      {
      if (!(*p))
         break;

      if (usIndex == 8 && p <= pLastDot)
         {
         if (p < pLastDot)
            usLongName = LONGNAME_MAKE_UNIQUE;
         if (*pLastDot)
            p = pLastDot + 1;
         else
            break;
         }

      while (*p == 0x20)
         {
         usLongName = LONGNAME_MAKE_UNIQUE;
         p++;
         }
      if (!(*p))
         break;

      if (*p >= 128)
         {
         szShortName[usIndex++] = *p;
         }
      else if (*p == '.')
         {
         /*
            Skip all dots, if multiple dots are in the
            name create an unique name
         */
         if (p == pLastDot)
            usIndex = 8;
         else
            usLongName = LONGNAME_MAKE_UNIQUE;
         }
      else if (strchr(rgValidChars, *p))
         szShortName[usIndex++] = *p;
      else
         {
         szShortName[usIndex++] = '_';
         }
      p++;
      }
   if (strlen(p))
      usLongName = LONGNAME_MAKE_UNIQUE;

   free(pszUpper);

   p = szShortName;
   for( usIndex = 0; usIndex < 8; usIndex++ )
      if( IsDBCSLead( p[ usIndex ]))
         usIndex++;

   if( usIndex > 8 )
      p[ 7 ] = 0x20;

   p = szShortName + 8;
   for( usIndex = 0; usIndex < 3; usIndex++ )
      if( IsDBCSLead( p[ usIndex ]))
         usIndex++;

   if( usIndex > 3 )
      p[ 2 ] = 0x20;

   if (usLongName == LONGNAME_MAKE_UNIQUE)
      {
      USHORT usNum;
      BYTE   szFileName[25];
      BYTE   szNumber[18];
      ULONG ulCluster;
      PSZ p;
      USHORT usPos1, usPos2;

      for (usPos1 = 8; usPos1 > 0; usPos1--)
         if (szShortName[usPos1 - 1] != 0x20)
            break;

      for (usNum = 1; usNum < 32000; usNum++)
         {
         memset(szFileName, 0, sizeof szFileName);
         memcpy(szFileName, szShortName, 8);

         /*
            Find last blank in filename before dot.
         */

         memset(szNumber, 0, sizeof szNumber);
         itoa(usNum, szNumber, 10);

         usPos2 = 7 - (strlen(szNumber));
         if (usPos1 && usPos1 < usPos2)
            usPos2 = usPos1;

         for( usIndex = 0; usIndex < usPos2; usIndex++ )
            if( IsDBCSLead( szShortName[ usIndex ]))
               usIndex++;

         if( usIndex > usPos2 )
            usPos2--;

         strcpy(szFileName + usPos2, "~");
         strcat(szFileName, szNumber);

         if (memcmp(szShortName + 8, "   ", 3))
            {
            strcat(szFileName, ".");
            memcpy(szFileName + strlen(szFileName), szShortName + 8, 3);
            p = szFileName + strlen(szFileName);
            while (p > szFileName && *(p-1) == 0x20)
               p--;
            *p = 0;
            }
         ulCluster = FindPathCluster(pVolInfo, ulDirCluster, szFileName, NULL, NULL);
         if (ulCluster == FAT_EOF)
            break;
         }
      if (usNum < 32000)
         {
         p = strchr(szFileName, '.');
#if 0
         if (p && p - szFileName < 8)
            memcpy(szShortName, szFileName, p - szFileName);
         else
            memccpy(szShortName, szFileName, 0, 8 );
         }
#else
         if( !p )
            p = szFileName + strlen( szFileName );

         memcpy(szShortName, szFileName, p - szFileName);
         memset( szShortName + ( p - szFileName ), 0x20, 8 - ( p - szFileName ));
#endif
         }
      else
         return LONGNAME_ERROR;
      }
   memcpy(pszShortName, szShortName, 11);
   return usLongName;
}


/******************************************************************
*
******************************************************************/
BOOL DeleteFatChain(PVOLINFO pVolInfo, ULONG ulCluster)
{
ULONG ulNextCluster;
ULONG ulSector;
PULONG pulCluster;
ULONG ulClustersFreed;
USHORT rc;

   if (!ulCluster)
      return TRUE;

   if (ulCluster >= 2 && ulCluster < pVolInfo->ulTotalClusters + 2)
      {
      if (f32Parms.fMessageActive  & LOG_FUNCS)
         Message("DeleteFatChain for cluster %lu", ulCluster);
      }
   else
      {
      Message("DeleteFatChain for invalid cluster %lu (ERROR)", ulCluster);
      return FALSE;
      }

   if (GetFatAccess(pVolInfo, "DeleteFatChain"))
      return FALSE;

   ulSector = ulCluster / 128;
   ReadFatSector(pVolInfo, ulSector);
   ulClustersFreed = 0;
   while (!(ulCluster >= FAT_EOF2 && ulCluster <= FAT_EOF))
      {
#ifdef CALL_YIELD
      Yield();
#endif

      if (!ulCluster || ulCluster == FAT_BAD_CLUSTER)
         {
         Message("DeleteFatChain: Bad Chain (Cluster %lu)",
            ulCluster);
         break;
         }
      ulSector = ulCluster / 128;
      if (ulSector != pVolInfo->ulCurFatSector)
         {
         rc = WriteFatSector(pVolInfo, pVolInfo->ulCurFatSector);
         if (rc)
            {
            ReleaseFat(pVolInfo);
            return FALSE;
            }
         ReadFatSector(pVolInfo, ulSector);
         }
      pulCluster = (PULONG)pVolInfo->pbFatSector + (ulCluster % 128);

      ulNextCluster = *pulCluster;
      *pulCluster = 0L;
      ulClustersFreed++;
      ulCluster = ulNextCluster;
      }
   rc = WriteFatSector(pVolInfo, pVolInfo->ulCurFatSector);
   if (rc)
      {
      ReleaseFat(pVolInfo);
      return FALSE;
      }

   pVolInfo->pBootFSInfo->ulFreeClusters += ulClustersFreed;
/*   UpdateFSInfo(pVolInfo);*/

   ReleaseFat(pVolInfo);

   return TRUE;
}

ULONG SeekToCluster(PVOLINFO pVolInfo, ULONG ulCluster, ULONG ulPosition)
{
ULONG  ulSector;
PULONG pulCluster;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("SeekToCluster");

   if (GetFatAccess(pVolInfo, "SeekToCluster"))
      return FAT_EOF;

   while (ulCluster != FAT_EOF && ulPosition >= (ULONG)pVolInfo->usClusterSize)
      {
      ulSector = ulCluster / 128;

      if (ulSector != pVolInfo->ulCurFatSector)
         ReadFatSector(pVolInfo, ulSector);

      pulCluster = (PULONG)pVolInfo->pbFatSector + (ulCluster % 128);

      ulCluster = *pulCluster & FAT_EOF;
      if (ulCluster >= FAT_EOF2 && ulCluster <= FAT_EOF)
         ulCluster = FAT_EOF;

      ulPosition -= pVolInfo->usClusterSize;
      }
   ReleaseFat(pVolInfo);

   return ulCluster;

}

ULONG GetLastCluster(PVOLINFO pVolInfo, ULONG ulCluster)
{
ULONG  ulSector;
PULONG pulCluster;
ULONG  ulReturn;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("GetLastCluster");

   if (!ulCluster)
      return FAT_EOF;

   if (GetFatAccess(pVolInfo, "GetLastCluster"))
      return FAT_EOF;

   ulReturn = ulCluster;
   while (ulCluster != FAT_EOF)
      {
      ulReturn = ulCluster;
      ulSector = ulCluster / 128;

      if (ulSector != pVolInfo->ulCurFatSector)
         ReadFatSector(pVolInfo, ulSector);

      pulCluster = (PULONG)pVolInfo->pbFatSector + (ulCluster % 128);

      ulCluster = *pulCluster & FAT_EOF;
      if (ulCluster >= FAT_EOF2 && ulCluster <= FAT_EOF)
         break;
      }
   ReleaseFat(pVolInfo);
   return ulReturn;
}

USHORT CopyChain(PVOLINFO pVolInfo, ULONG ulCluster, PULONG pulNew)
{
USHORT rc;
ULONG  ulNext;
ULONG  ulNext2;
ULONG  ulCount;
PBYTE  pbCluster;
ULONG  tStart = GetCurTime();


   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("CopyChain, cluster %lu", ulCluster);

   if (!ulCluster)
      {
      *pulNew = 0L;
      return 0;
      }

   *pulNew = FAT_EOF;

   pbCluster = malloc(pVolInfo->usClusterSize);
   if (!pbCluster)
      return ERROR_NOT_ENOUGH_MEMORY;

   ulCount = GetChainSize(pVolInfo, ulCluster);
   if (!ulCount)
      return 1340;

   *pulNew = MakeFatChain(pVolInfo, FAT_EOF, ulCount, NULL);
   if (*pulNew == FAT_EOF)
      {
      free(pbCluster);
      return ERROR_DISK_FULL;
      }

   ulNext = *pulNew;
   rc = 0;
   while (ulCluster != FAT_EOF && ulNext != FAT_EOF)
      {
      ULONG tNow = GetCurTime();
      if (tStart + 3 > tNow)
         {
         Yield();
         tStart = tNow;
         }

      rc = ReadCluster(pVolInfo, ulCluster, pbCluster, DVIO_OPNCACHE);
      if (rc)
         break;
      rc = WriteCluster(pVolInfo, ulNext, pbCluster, 0);
      if (rc)
         break;
      ulCluster = GetNextCluster(pVolInfo, ulCluster);
      if (!ulCluster)
         ulCluster = FAT_EOF;
      if (ulCluster == FAT_EOF)
         break;
      ulNext2 = GetNextCluster(pVolInfo, ulNext);
      if (ulNext2 == FAT_EOF)
         ulNext2 = SetNextCluster(pVolInfo, ulNext, FAT_ASSIGN_NEW);
      ulNext = ulNext2;
      }
   if (ulCluster != FAT_EOF)
      rc = ERROR_DISK_FULL;

   free(pbCluster);
   if (rc)
      {
      DeleteFatChain(pVolInfo, *pulNew);
      *pulNew = FAT_EOF;
      }
   return rc;

}

ULONG GetChainSize(PVOLINFO pVolInfo, ULONG ulCluster)
{
ULONG ulCount;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("GetChainSize");

   if (GetFatAccess(pVolInfo, "GetChainCount"))
      return 0L;

   ulCount = 0;
   while (ulCluster && ulCluster != FAT_EOF)
      {
      ulCount++;
      ulCluster = GetNextCluster2(pVolInfo, ulCluster);
      }
   ReleaseFat(pVolInfo);
   return ulCount;
}


USHORT ModifyDirectory(PVOLINFO pVolInfo, ULONG ulDirCluster, USHORT usMode, PDIRENTRY pOld, PDIRENTRY pNew, PSZ pszLongName, USHORT usIOMode)
{
PDIRENTRY pDirectory;
PDIRENTRY pDir2;
PDIRENTRY pWork, pWork2;
PDIRENTRY pMax;
USHORT    usEntriesNeeded;
USHORT    usFreeEntries;
DIRENTRY  DirNew;
ULONG     ulCluster;
ULONG     ulPrevCluster;
ULONG     ulNextCluster;
PDIRENTRY pLNStart;
USHORT    rc;
USHORT    usClusterCount;
BOOL      fNewCluster;

   if (f32Parms.fMessageActive & LOG_FUNCS)
      Message("ModifyDirectory DirCluster %ld, Mode = %d",
      ulDirCluster, usMode);


   if (usMode == MODIFY_DIR_RENAME ||
       usMode == MODIFY_DIR_INSERT)
      {
      if (!pNew || !pszLongName)
         {
         Message("Modify directory: Invalid parameters 1");
         return ERROR_INVALID_PARAMETER;
         }

      memcpy(&DirNew, pNew, sizeof (DIRENTRY));
      if ((pNew->bAttr & 0x0F) != FILE_VOLID)
         {
         rc = MakeShortName(pVolInfo, ulDirCluster, pszLongName, DirNew.bFileName);
         if (rc == LONGNAME_ERROR)
            {
            Message("Modify directory: Longname error");
            return ERROR_FILE_EXISTS;
            }
         memcpy(pNew, &DirNew, sizeof (DIRENTRY));

         if (rc == LONGNAME_OFF)
            pszLongName = NULL;
         }
      else
         pszLongName = NULL;

      usEntriesNeeded = 1;
      if (pszLongName)
#if 0
         usEntriesNeeded += strlen(pszLongName) / 13 +
            (strlen(pszLongName) % 13 ? 1 : 0);
#else
         usEntriesNeeded += ( DBCSStrlen( pszLongName ) + 12 ) / 13;
#endif
      }

   if (usMode == MODIFY_DIR_RENAME ||
       usMode == MODIFY_DIR_DELETE ||
       usMode == MODIFY_DIR_UPDATE)
      {
      if (!pOld)
         {
         Message("Modify directory: Invalid parameter 2 ");
         return ERROR_INVALID_PARAMETER;
         }
      }

   pDirectory = (PDIRENTRY)malloc(2 * pVolInfo->usClusterSize);
   if (!pDirectory)
      {
      Message("Modify directory: Not enough memory");
      return ERROR_NOT_ENOUGH_MEMORY;
      }

   memset(pDirectory, 0, pVolInfo->usClusterSize);
   pDir2 =(PDIRENTRY)((PBYTE)pDirectory + pVolInfo->usClusterSize);
   memset(pDir2, 0, pVolInfo->usClusterSize);
   pMax = (PDIRENTRY)((PBYTE)pDirectory + pVolInfo->usClusterSize * 2);

   ulCluster = ulDirCluster;
   pLNStart = NULL;
   ulPrevCluster = FAT_EOF;
   usClusterCount = 0;
   fNewCluster = FALSE;
   while (ulCluster != FAT_EOF)
      {
#ifdef CALL_YIELD
      Yield();
#endif

      usClusterCount++;
      if (!fNewCluster)
         {
         rc = ReadCluster(pVolInfo, ulCluster, pDir2, usIOMode);
         if (rc)
            {
            free(pDirectory);
            return rc;
            }
         }
      else
         {
         memset(pDir2, 0, pVolInfo->usClusterSize);
         fNewCluster = FALSE;
         }

      switch (usMode)
         {
         case MODIFY_DIR_RENAME :
         case MODIFY_DIR_UPDATE :
         case MODIFY_DIR_DELETE :

            /*
               Find old entry
            */

            pWork = pDir2;
            while (pWork != pMax)
               {
               if (pWork->bFileName[0] && pWork->bFileName[0] != DELETED_ENTRY)
                  {
                  if (pWork->bAttr == FILE_LONGNAME)
                     {
                     if (!pLNStart)
                        pLNStart = pWork;
                     }
                  else if ((pWork->bAttr & 0x0F) != FILE_VOLID)
                     {
                     if (!memcmp(pWork->bFileName, pOld->bFileName, 11) &&
                         pWork->wCluster     == pOld->wCluster &&
                         pWork->wClusterHigh == pOld->wClusterHigh)
                        {
                        if (!pLNStart)
                           pLNStart = pWork;
                        break;
                        }
                     pLNStart = NULL;
                     }
                  else
                     pLNStart = NULL;
                  }
               else
                  pLNStart = NULL;
               pWork++;
               }

            if (pWork != pMax)
               {
               switch (usMode)
                  {
                  case MODIFY_DIR_UPDATE:
                     if (f32Parms.fMessageActive & LOG_FUNCS)
                        Message(" Updating cluster");
                     memcpy(pWork, pNew, sizeof (DIRENTRY));
                     rc = WriteCluster(pVolInfo, ulCluster, pDir2, usIOMode);
                     if (rc)
                        {
                        free(pDirectory);
                        return rc;
                        }
                     ulCluster = FAT_EOF;
                     break;

                  case MODIFY_DIR_DELETE:
                  case MODIFY_DIR_RENAME:
                     if (f32Parms.fMessageActive & LOG_FUNCS)
                        Message(" Removing entry from cluster");
                     pWork2 = pLNStart;
                     while (pWork2 < pWork)
                        {
                        if (f32Parms.fMessageActive & LOG_FUNCS)
                           Message("Deleting Longname entry.");
                        pWork2->bFileName[0] = DELETED_ENTRY;
                        pWork2++;
                        }
                     pWork->bFileName[0] = DELETED_ENTRY;

                     /*
                        Write previous cluster if LN start lies there
                     */
                     if (ulPrevCluster != FAT_EOF &&
                        pLNStart < pDir2)
                        {
                        rc = WriteCluster(pVolInfo, ulPrevCluster, pDirectory, usIOMode);
                        if (rc)
                           {
                           free(pDirectory);
                           return rc;
                           }
                        }

                     /*
                        Write current cluster
                     */
                     rc = WriteCluster(pVolInfo, ulCluster, pDir2, usIOMode);
                     if (rc)
                        {
                        free(pDirectory);
                        return rc;
                        }

                     if (usMode == MODIFY_DIR_DELETE)
                        ulCluster = FAT_EOF;
                     else
                        {
                        usMode = MODIFY_DIR_INSERT;
                        ulCluster = ulDirCluster;
                        ulPrevCluster = FAT_EOF;
                        usClusterCount = 0;
                        pLNStart = NULL;
                        continue;
                        }
                     break;
                  }
               }

            break;

         case MODIFY_DIR_INSERT:
            if (ulPrevCluster != FAT_EOF && GetFreeEntries(pDirectory, pVolInfo->usClusterSize * 2) >= usEntriesNeeded)
               {
               BYTE bCheck = GetVFATCheckSum(&DirNew);

               if (f32Parms.fMessageActive & LOG_FUNCS)
                  Message(" Inserting entry into 2 clusters");

               pWork = CompactDir(pDirectory, pVolInfo->usClusterSize * 2, usEntriesNeeded);
               pWork = fSetLongName(pWork, pszLongName, bCheck);
               memcpy(pWork, &DirNew, sizeof (DIRENTRY));

               rc = WriteCluster(pVolInfo, ulPrevCluster, pDirectory, usIOMode);
               if (rc)
                  {
                  free(pDirectory);
                  return rc;
                  }

               rc = WriteCluster(pVolInfo, ulCluster, pDir2, usIOMode);
               if (rc)
                  {
                  free(pDirectory);
                  return rc;
                  }
               ulCluster = FAT_EOF;
               break;
               }

            usFreeEntries = GetFreeEntries(pDir2, pVolInfo->usClusterSize);
            if (usFreeEntries >= usEntriesNeeded)
               {
               BYTE bCheck = GetVFATCheckSum(&DirNew);

               if (f32Parms.fMessageActive & LOG_FUNCS)
                  Message(" Inserting entry into 1 cluster");

               pWork = CompactDir(pDir2, pVolInfo->usClusterSize, usEntriesNeeded);
               pWork = fSetLongName(pWork, pszLongName, bCheck);
               memcpy(pWork, &DirNew, sizeof (DIRENTRY));
               rc = WriteCluster(pVolInfo, ulCluster, pDir2, usIOMode);
               if (rc)
                  {
                  free(pDirectory);
                  return rc;
                  }
               ulCluster = FAT_EOF;
               break;
               }
            else if (usFreeEntries > 0)
               {
               MarkFreeEntries(pDir2, pVolInfo->usClusterSize);
               rc = WriteCluster(pVolInfo, ulCluster, pDir2, usIOMode);
               if (rc)
                  {
                  free(pDirectory);
                  return rc;
                  }
               }

            break;
         }

      if (ulCluster != FAT_EOF)
         {
         ulPrevCluster = ulCluster;
         memmove(pDirectory, pDir2, pVolInfo->usClusterSize);
         if (pLNStart)
            pLNStart = (PDIRENTRY)((PBYTE)pLNStart - pVolInfo->usClusterSize);


         ulNextCluster = GetNextCluster(pVolInfo, ulCluster);
         if (!ulNextCluster)
            ulNextCluster = FAT_EOF;
         if (ulNextCluster == FAT_EOF)
            {

            if (usMode == MODIFY_DIR_UPDATE ||
                usMode == MODIFY_DIR_DELETE ||
                usMode == MODIFY_DIR_RENAME)
               {
               free(pDirectory);
               return ERROR_FILE_NOT_FOUND;
               }

            ulNextCluster = SetNextCluster(pVolInfo, ulCluster, FAT_ASSIGN_NEW);
            if (ulNextCluster == FAT_EOF)
               {
               free(pDirectory);
               Message("Modify Directory: Disk Full!");
               return ERROR_DISK_FULL;
               }
            fNewCluster = TRUE;
            }
         ulCluster = ulNextCluster;
         }
      }

   free(pDirectory);
   return 0;
}

VOID MarkFreeEntries(PDIRENTRY pDirBlock, USHORT usSize)
{
PDIRENTRY pMax;

   pMax = (PDIRENTRY)((PBYTE)pDirBlock + usSize);
   while (pDirBlock != pMax)
      {
      if (!pDirBlock->bFileName[0])
         pDirBlock->bFileName[0] = DELETED_ENTRY;
      pDirBlock++;
      }
}

USHORT GetFreeEntries(PDIRENTRY pDirBlock, USHORT usSize)
{
USHORT usCount;
PDIRENTRY pMax;
BOOL bLoop;

   pMax = (PDIRENTRY)((PBYTE)pDirBlock + usSize);
   usCount = 0;
   bLoop = pMax == pDirBlock;
   while (( pDirBlock != pMax ) || bLoop )
      {
      if (!pDirBlock->bFileName[0] || pDirBlock->bFileName[0] == DELETED_ENTRY)
         usCount++;
      bLoop = FALSE;
      pDirBlock++;
      }

   return usCount;
}

PDIRENTRY CompactDir(PDIRENTRY pStart, USHORT usSize, USHORT usEntriesNeeded)
{
PDIRENTRY pTar, pMax, pFirstFree;
USHORT usFreeEntries;
BOOL bLoop;


   pMax = (PDIRENTRY)((PBYTE)pStart + usSize);
   bLoop = pMax == pStart;
   pFirstFree = pMax;
   usFreeEntries = 0;
   while (( pFirstFree != pStart ) || bLoop )
      {
      if (!(pFirstFree-1)->bFileName[0])
         usFreeEntries++;
      else
         break;
      bLoop = FALSE;
      pFirstFree--;
      }

   if ((( pFirstFree == pStart ) && !bLoop ) || (pFirstFree - 1)->bAttr != FILE_LONGNAME)
      if (usFreeEntries >= usEntriesNeeded)
         return pFirstFree;

   /*
      Leaving longname entries at the end
   */
   while ((( pFirstFree != pStart ) || bLoop ) && (pFirstFree - 1)->bAttr == FILE_LONGNAME)
   {
      bLoop = FALSE;
      pFirstFree--;
   }

   usFreeEntries = 0;
   pTar = pStart;
   while ((( pStart != pFirstFree ) || bLoop ) && usFreeEntries < usEntriesNeeded)
      {
      if (pStart->bFileName[0] && pStart->bFileName[0] != DELETED_ENTRY)
         {
         if (pTar != pStart)
            *pTar = *pStart;
         pTar++;
         }
      else
         usFreeEntries++;

      bLoop = FALSE;
      pStart++;
      }
   if (pTar != pStart)
      {
#if 1
      USHORT usEntries = 0;
      PDIRENTRY p;

      for( p = pStart; ( p != pFirstFree ) /*|| bLoop */; p++ )
        {
            /*bLoop = FALSE;*/
            usEntries++;
        }
      memmove(pTar, pStart, usEntries * sizeof (DIRENTRY));
#else
      memmove(pTar, pStart, (pFirstFree - pStart) * sizeof (DIRENTRY));
#endif
      pFirstFree -= usFreeEntries;
      memset(pFirstFree, DELETED_ENTRY, usFreeEntries * sizeof (DIRENTRY));
      }

   return pFirstFree;
}

USHORT SemRequest(void far * hSem, ULONG ulTimeOut, PSZ pszText)
{
USHORT rc;

   if (ulTimeOut == TO_INFINITE)
      ulTimeOut = 60000; /* 1 minute */
   do
      {
      rc = FSH_SEMREQUEST(hSem, ulTimeOut);
      if (rc == ERROR_SEM_TIMEOUT)
         {
         Message("ERROR: Timeout on semaphore for %s", pszText);
         rc = CritMessage("FAT32: Timeout on semaphore for %s", pszText);
         if (rc != CE_RETRETRY)
            rc = ERROR_SEM_TIMEOUT;
         else
            rc = ERROR_INTERRUPT;
         }
      } while (rc == ERROR_INTERRUPT);

   return rc;
}

BOOL IsDosSession(VOID)
{
PROCINFO pr;

   GetProcInfo(&pr, sizeof pr);
   if (pr.usPdb)
      return TRUE;
   return FALSE;
}

BOOL IsDriveLocked(PVOLINFO pVolInfo)
{
PROCINFO ProcInfo;
USHORT   rc;

   for (;;)
      {
      if (!pVolInfo->fLocked)
         return FALSE;
      GetProcInfo(&ProcInfo, sizeof ProcInfo);
      if (!memcmp(&pVolInfo->ProcLocked, &ProcInfo, sizeof (PROCINFO)))
         return FALSE;

      rc = FSH_CRITERROR(strlen(szDiskLocked) + 1, szDiskLocked, 0, "",
         CE_ALLFAIL | CE_ALLABORT | CE_ALLRETRY);
      if (rc != CE_RETRETRY)
         return TRUE;
      }
}

USHORT GetFatAccess(PVOLINFO pVolInfo, PSZ pszName)
{
USHORT rc;

   pVolInfo = pVolInfo;

   rc = SemRequest(&ulSemRWFat, TO_INFINITE, pszName);
   if (rc)
      {
      Message("ERROR: SemRequest GetFatAccess Failed, rc = %d!", rc);
      CritMessage("FAT32: SemRequest GetFatAccess Failed, rc = %d!", rc);
      Message("GetFatAccess Failed for %s, rc = %d", pszName, rc);
      return rc;
      }
   return 0;
}

VOID ReleaseFat(PVOLINFO pVolInfo)
{
   pVolInfo = pVolInfo;
   FSH_SEMCLEAR(&ulSemRWFat);
}

VOID Yield(void)
{
static PBYTE pYieldFlag = NULL;

   if (!pYieldFlag)
      DevHelp_GetDOSVar(DHGETDOSV_YIELDFLAG, 0, (PPVOID)&pYieldFlag);
   if (*pYieldFlag)
      DevHelp_Yield();
}

USHORT MY_PROBEBUF(USHORT usOperation, char far * pData, USHORT cbData)
{
   return DevHelp_VerifyAccess(SELECTOROF(pData),
      cbData,
      OFFSETOF(pData),
      (UCHAR)usOperation);
}

USHORT DBCSStrlen( const PSZ pszStr )
{
   USHORT usLen;
   USHORT usIndex;
   USHORT usRet;

   usLen = strlen( pszStr );
   usRet = 0;
   for( usIndex = 0; usIndex < usLen; usIndex++ )
      {
         if( IsDBCSLead( pszStr[ usIndex ]))
            usIndex++;

         usRet++;
      }

   return usRet;
}


