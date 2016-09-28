#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define INCL_BASE

#include <os2.h>
#include <uconv.h>

#include "portable.h"
#include "fat32def.h"

#define MAX_TRANS_TABLE     0x100
#define ARRAY_TRANS_TABLE   ( 0x10000 / MAX_TRANS_TABLE )

#define INDEX_OF_START      MAX_TRANS_TABLE
#define INDEX_OF_FIRSTINFO  MAX_TRANS_TABLE
#define INDEX_OF_LCASECONV  ( MAX_TRANS_TABLE + 1 )
#define INDEX_OF_END        INDEX_OF_LCASECONV
#define EXTRA_ELEMENT       ( INDEX_OF_END - INDEX_OF_START + 1 )

#define RETURN_PARENT_DIR 0xFFFF

#define MODIFY_DIR_INSERT 0
#define MODIFY_DIR_DELETE 1
#define MODIFY_DIR_UPDATE 2
#define MODIFY_DIR_RENAME 3

#define LONGNAME_OFF         0
#define LONGNAME_OK          1
#define LONGNAME_MAKE_UNIQUE 2
#define LONGNAME_ERROR       3

//#define min(a,b)  (((a) < (b)) ? (a) : (b))

extern F32PARMS f32Parms;

#define ARRAY_COUNT_PAGE    4
#define MAX_ARRAY_PAGE      ( 0x100 / ARRAY_COUNT_PAGE )

#define ARRAY_COUNT_UNICODE 256
#define MAX_ARRAY_UNICODE   (( USHORT )( 0x10000L / ARRAY_COUNT_UNICODE ))

HMODULE hModConv = 0;
HMODULE hModUni  = 0;

int (* CALLCONV pUniCreateUconvObject)(UniChar * code_set, UconvObject * uobj);
int (* CALLCONV pUniUconvToUcs)(
             UconvObject uobj,         /* I  - Uconv object handle         */
             void    * * inbuf,        /* IO - Input buffer                */
             size_t    * inbytes,      /* IO - Input buffer size (bytes)   */
             UniChar * * outbuf,       /* IO - Output buffer size          */
             size_t    * outchars,     /* IO - Output size (chars)         */
             size_t    * subst  );     /* IO - Substitution count          */
int (* CALLCONV pUniUconvFromUcs)(
             UconvObject uobj,
             UniChar * * inbuf,
             size_t    * inchars,
             void    * * outbuf,
             size_t    * outbytes,
             size_t    * subst  );
int (* CALLCONV pUniMapCpToUcsCp)( ULONG ulCp, UniChar *ucsCp, size_t n );
UniChar (* CALLCONV pUniTolower )( UniChar uin );
int (* CALLCONV pUniQueryUconvObject )
    (UconvObject uobj, uconv_attribute_t *attr, size_t size, char first[256], char other[256], udcrange_t udcrange[32]);

static ULONG     ulNewCP = 0;

static BYTE rgValidChars[]="01234567890 ABCDEFGHIJKLMNOPQRSTUVWXYZ!#$%&'()-_@^`{}~";

typedef struct _UniPage
{
USHORT usCode[256];
} UNIPAGE, *PUNIPAGE;

static PUSHORT  rgUnicode[ ARRAY_COUNT_UNICODE ] = { NULL, };

static UCHAR rgFirstInfo[ 256 ] = { 0 , };
static PUNIPAGE rgPage[ ARRAY_COUNT_PAGE ] = { NULL, };

static UCHAR rgLCase[ 256 ] =
{   0,
    1,   2,   3,   4,   5,   6,   7,   8,   9,  10,
   11,  12,  13,  14,  15,  16,  17,  18,  19,  20,
   21,  22,  23,  24,  25,  26,  27,  28,  29,  30,
   31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
   41,  42,  43,  44,  45,  46,  47,  48,  49,  50,
   51,  52,  53,  54,  55,  56,  57,  58,  59,  60,
   61,  62,  63,  64,
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
   91,  92,  93,  94,  95,  96,  97,  98,  99, 100,
  101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
  111, 112, 113, 114, 115, 116, 117, 118, 119, 120,
  121, 122, 123, 124, 125, 126, 127, 128, 129, 130,
  131, 132, 133, 134, 135, 136, 137, 138, 139, 140,
  141, 142, 143, 144, 145, 146, 147, 148, 149, 150,
  151, 152, 153, 154, 155, 156, 157, 158, 159, 160,
  161, 162, 163, 164, 165, 166, 167, 168, 169, 170,
  171, 172, 173, 174, 175, 176, 177, 178, 179, 180,
  181, 182, 183, 184, 185, 186, 187, 188, 189, 190,
  191, 192, 193, 194, 195, 196, 197, 198, 199, 200,
  201, 202, 203, 204, 205, 206, 207, 208, 209, 210,
  211, 212, 213, 214, 215, 216, 217, 218, 219, 220,
  221, 222, 223, 224, 225, 226, 227, 228, 229, 230,
  231, 232, 233, 234, 235, 236, 237, 238, 239, 240,
  241, 242, 243, 244, 245, 246, 247, 248, 249, 250,
  251, 252, 253, 254, 255
};

ULONG ReadSector(PCDINFO pCD, ULONG ulSector, USHORT nSectors, PBYTE pbSector);
ULONG ReadCluster(PCDINFO pCD, ULONG ulCluster, PVOID pbCluster);
ULONG WriteSector(PCDINFO pCD, ULONG ulSector, USHORT nSectors, PBYTE pbSector);
ULONG WriteCluster(PCDINFO pCD, ULONG ulCluster, PVOID pbCluster);
ULONG ReadFatSector(PCDINFO pCD, ULONG ulSector);
ULONG WriteFatSector(PCDINFO pCD, ULONG ulSector);
ULONG ReadSector(PCDINFO pCD, ULONG ulSector, USHORT nSectors, PBYTE pbSector);
ULONG GetNextCluster(PCDINFO pCD, ULONG ulCluster);
ULONG GetNextCluster2(PCDINFO pCD, ULONG ulCluster);
BOOL  GetDiskStatus(PCDINFO pCD);
ULONG GetFreeSpace(PCDINFO pCD);
BOOL  MarkDiskStatus(PCDINFO pCD, BOOL fClean);
ULONG FindDirCluster(PCDINFO pCD, PSZ pDir, USHORT usCurDirEnd, USHORT usAttrWanted, PSZ *pDirEnd);
ULONG FindPathCluster(PCDINFO pCD, ULONG ulCluster, PSZ pszPath, PDIRENTRY pDirEntry, PSZ pszFullName);
USHORT ModifyDirectory(PCDINFO pCD, ULONG ulDirCluster, USHORT usMode, PDIRENTRY pOld, PDIRENTRY pNew, PSZ pszLongName);
USHORT MarkFileEAS(PCDINFO pCD, ULONG ulDirCluster, PSZ pszFileName, BYTE fEAS);
USHORT GetSetFileEAS(PCDINFO pCD, USHORT usFunc, PMARKFILEEASBUF pMark);
BOOL fGetLongName(PDIRENTRY pDir, PSZ pszName, USHORT wMax, PBYTE pbCheck);
USHORT QueryUni2NLS( USHORT usPage, USHORT usChar );
BYTE GetVFATCheckSum(PDIRENTRY pDir);
USHORT MakeShortName(PCDINFO pCD, ULONG ulDirCluster, PSZ pszLongName, PSZ pszShortName);
USHORT DBCSStrlen( const PSZ pszStr );
BOOL IsDBCSLead( UCHAR uch );
VOID Translate2OS2(PUSHORT pusUni, PSZ pszName, USHORT usLen);
PDIRENTRY fSetLongName(PDIRENTRY pDir, PSZ pszLongName, BYTE bCheck);
USHORT Translate2Win(PSZ pszName, PUSHORT pusUni, USHORT usLen);
USHORT QueryNLS2Uni( USHORT usCode );
ULONG SetNextCluster(PCDINFO pCD, ULONG ulCluster, ULONG ulNext);
ULONG SetNextCluster2(PCDINFO pCD, ULONG ulCluster, ULONG ulNext);
VOID MakeName(PDIRENTRY pDir, PSZ pszName, USHORT usMax);
USHORT GetFreeEntries(PDIRENTRY pDirBlock, USHORT usSize);
PDIRENTRY CompactDir(PDIRENTRY pStart, USHORT usSize, USHORT usEntriesNeeded);
VOID MarkFreeEntries(PDIRENTRY pDirBlock, USHORT usSize);
ULONG GetFreeCluster(PCDINFO pCD);
USHORT SetFileSize(PCDINFO pCD, PFILESIZEDATA pFileSize);
USHORT RecoverChain2(PCDINFO pCD, ULONG ulCluster, PBYTE pData, USHORT cbData);
USHORT MakeDirEntry(PCDINFO pCD, ULONG ulDirCluster, PDIRENTRY pNew, PSZ pszName);
BOOL DeleteFatChain(PCDINFO pCD, ULONG ulCluster);
VOID TranslateAllocBuffer( VOID );
BOOL TranslateInit(PVOID16 rgTrans[], USHORT usSize);
VOID GetFirstInfo( PBOOL pFirstInfo );
VOID SetUni2NLS( USHORT usPage, USHORT usChar, USHORT usCode );
BOOL LoadTranslateTable(VOID);
VOID GetCaseConversion( PUCHAR pCase );
BOOL UpdateFSInfo(PCDINFO pCD);

/******************************************************************
*
******************************************************************/
BOOL GetDiskStatus(PCDINFO pCD)
{
   PULONG pulStatus;
   BOOL   fStatus;

   if (ReadFatSector(pCD, 0L))
      return FALSE;

   pulStatus = (PULONG)pCD->pbFATSector + 1;

   if (*pulStatus & FAT32_CLEAN_SHUTDOWN)
      fStatus = TRUE;
   else
      fStatus = FALSE;

   return fStatus;
}

/******************************************************************
*
******************************************************************/
ULONG ReadFatSector(PCDINFO pCD, ULONG ulSector)
{
   ULONG rc;

   if (pCD->ulCurFATSector == ulSector)
      return 0;

   if (ulSector >= pCD->BootSect.bpb.BigSectorsPerFat)
      return ERROR_SECTOR_NOT_FOUND;

   rc = ReadSector(pCD, pCD->ulActiveFatStart + ulSector, 1,
      pCD->pbFATSector);
   if (rc)
      return rc;

   pCD->ulCurFATSector = ulSector;

   return 0;
}

/******************************************************************
*
******************************************************************/
ULONG WriteFatSector(PCDINFO pCD, ULONG ulSector)
{
   USHORT usFat;
   ULONG rc;

   //if (f32Parms.fMessageActive & LOG_FUNCS)
   //   Message("WriteFatSector");

   if (pCD->ulCurFATSector != ulSector)
      //{
      //CritMessage("FAT32: WriteFatSector: Sectors do not match!");
      //Message("ERROR: WriteFatSector: Sectors do not match!");
      return ERROR_SECTOR_NOT_FOUND;
      //}

   if (ulSector >= pCD->BootSect.bpb.BigSectorsPerFat)
      //{
      //CritMessage("ERROR: WriteFatSector: Sector %ld too high", ulSector);
      //Message("ERROR: WriteFatSector: Sector %ld too high", ulSector);
      return ERROR_SECTOR_NOT_FOUND;
      //}

   for (usFat = 0; usFat < pCD->BootSect.bpb.NumberOfFATs; usFat++)
      {
      rc = WriteSector(pCD, pCD->ulActiveFatStart + ulSector, 1,
         pCD->pbFATSector);

      if (rc)
         return rc;

      if (pCD->BootSect.bpb.ExtFlags & 0x0080)
         break;

      ulSector += pCD->BootSect.bpb.BigSectorsPerFat;
      }

   return 0;
}

/******************************************************************
*
******************************************************************/
ULONG GetNextCluster(PCDINFO pCD, ULONG ulCluster)
{
      ulCluster = GetNextCluster2(pCD, ulCluster);
      return ulCluster;
}

/******************************************************************
*
******************************************************************/
ULONG GetNextCluster2(PCDINFO pCD, ULONG ulCluster)
{
   PULONG pulCluster;

   if (ReadFatSector(pCD, ulCluster / 128))
      return FAT_EOF;

   pulCluster = (PULONG)pCD->pbFATSector + (ulCluster % 128);

   ulCluster = *pulCluster & FAT_EOF;
   if (ulCluster >= FAT_EOF2 && ulCluster <= FAT_EOF)
      return FAT_EOF;
   return ulCluster;
}

/******************************************************************
*
******************************************************************/
ULONG GetFreeSpace(PCDINFO pCD)
{
   ULONG ulSector;
   ULONG ulCluster;
   ULONG ulTotalFree;
   PULONG pulCluster;

   ulTotalFree = 0;
   for (ulCluster = 2; ulCluster < pCD->ulTotalClusters + 2; ulCluster++)
      {
      ulSector = ulCluster / 128;
      if (ulSector != pCD->ulCurFATSector)
         ReadFatSector(pCD, ulSector);
      pulCluster = (PULONG)pCD->pbFATSector + (ulCluster % 128);
      if ((*pulCluster & FAT_EOF) == 0)
         ulTotalFree++;
      }

   if (pCD->FSInfo.ulFreeClusters != ulTotalFree)
      {
      pCD->FSInfo.ulFreeClusters = ulTotalFree;
      UpdateFSInfo(pCD); ////
      }

   return ulTotalFree;
}

/******************************************************************
*
******************************************************************/
BOOL MarkDiskStatus(PCDINFO pCD, BOOL fClean)
{
   static BYTE bSector[SECTOR_SIZE] = "";
   PULONG pulStatus;
   ULONG ulSector;
   USHORT usFat;
   PBYTE pbSector;

   //if (!pVolInfo->fDiskCleanOnMount && fClean)
   //   return TRUE;
   //if (pVolInfo->fWriteProtected)
   //   return TRUE;

   if (pCD->ulCurFATSector != 0)
      {
      if (ReadSector(pCD, pCD->ulActiveFatStart, 1, bSector))
         return FALSE;
      pbSector = bSector;
      pulStatus = (PULONG)bSector + 1;
      }
   else
      {
      pulStatus = (PULONG)pCD->pbFATSector + 1;
      pbSector = pCD->pbFATSector;
      }

   if (fClean)
      (*pulStatus) |= FAT32_CLEAN_SHUTDOWN;
   else
      (*pulStatus) &= ~FAT32_CLEAN_SHUTDOWN;

   /*
      Trick, set fDiskClean to FALSE, so WriteSector
      won't set is back to dirty again
   */
   //pVolInfo->fDiskClean = FALSE;

   ulSector = 0L;
   for (usFat = 0; usFat < pCD->BootSect.bpb.NumberOfFATs; usFat++)
      {
      if (WriteSector(pCD, pCD->ulActiveFatStart + ulSector, 1, pbSector))
         return FALSE;
      if (pCD->BootSect.bpb.ExtFlags & 0x0080)
         break;
      ulSector += pCD->BootSect.bpb.BigSectorsPerFat;
      }

   //pVolInfo->fDiskClean = fClean;

   return TRUE;
}

/******************************************************************
*
******************************************************************/
ULONG FindDirCluster(PCDINFO pCD,
   //struct cdfsi far * pcdfsi,       /* pcdfsi   */
   //struct cdfsd far * pcdfsd,       /* pcdfsd   */
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

   //if (f32Parms.fMessageActive & LOG_FUNCS)
   //   Message("FindDirCluster for %s, CurDirEnd %u, AttrWanted %u", pDir, usCurDirEnd, usAttrWanted );

   //if (pcdfsi &&
   //   (pcdfsi->cdi_flags & CDI_ISVALID) &&
   //   !(pcdfsi->cdi_flags & CDI_ISROOT) &&
   //   usCurDirEnd != 0xFFFF)
   //   {
   //   pDir += usCurDirEnd;
   //   ulCluster = *(PULONG)pcdfsd;
   //   }
   //else
   //   {
      ulCluster = pCD->BootSect.bpb.RootDirStrtClus;
      if (strlen(pDir) >= 2)
         {
         if (pDir[1] == ':')
            pDir += 2;
         }
   //   }

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
   ulCluster = FindPathCluster(pCD, ulCluster, szDir, &DirEntry, NULL);
   if (ulCluster == FAT_EOF)
      //{
      //if (f32Parms.fMessageActive & LOG_FUNCS)
      //   Message("FindDirCluster for '%s', not found", szDir);
      return FAT_EOF;
      //}
   if (ulCluster != FAT_EOF && !(DirEntry.bAttr & FILE_DIRECTORY))
      //{
      //if (f32Parms.fMessageActive & LOG_FUNCS)
      //   Message("FindDirCluster for '%s', not a directory", szDir);
      return FAT_EOF;
      //}

   if (*pDir)
      {
      if (usAttrWanted != RETURN_PARENT_DIR && !strpbrk(pDir, "?*"))
         {
         ulCluster2 = FindPathCluster(pCD, ulCluster, pDir, &DirEntry, NULL);
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
ULONG FindPathCluster(PCDINFO pCD, ULONG ulCluster, PSZ pszPath, PDIRENTRY pDirEntry, PSZ pszFullName)
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
//PROCINFO ProcInfo;

   //if (f32Parms.fMessageActive & LOG_FUNCS)
   //   Message("FindPathCluster for %s, dircluster %lu", pszPath, ulCluster);

   if (pDirEntry)
      {
      memset(pDirEntry, 0, sizeof (DIRENTRY));
      pDirEntry->bAttr = FILE_DIRECTORY;
      }
   if (pszFullName)
      {
      memset(pszFullName, 0, FAT32MAXPATH);
      if (ulCluster == pCD->BootSect.bpb.RootDirStrtClus)
         {
         pszFullName[0] = (BYTE)(pCD->szDrive[0] + 'A');
         pszFullName[1] = ':';
         pszFullName[2] = '\\';
         }
      }

   if (strlen(pszPath) >= 2)
      {
      if (pszPath[1] == ':')
         pszPath += 2;
      }

   pDirStart = malloc(pCD->usClusterSize);
   if (!pDirStart)
      //{
      //Message("FAT32: Not enough memory for cluster in FindPathCluster");
      return FAT_EOF;
      //}
   pszLongName = malloc(FAT32MAXPATHCOMP * 2);
   if (!pszLongName)
      {
      //Message("FAT32: Not enough memory for buffers in FindPathCluster");
      free(pDirStart);
      return FAT_EOF;
      }
   memset(pszLongName, 0, FAT32MAXPATHCOMP * 2);
   pszPart = pszLongName + FAT32MAXPATHCOMP;

   usMode = MODE_SCAN;
   //GetProcInfo(&ProcInfo, sizeof ProcInfo);
   /*
      Allow EA files to be found!
   */
   //if (ProcInfo.usPdb && f32Parms.fEAS && IsEASFile(pszPath))
   //   ProcInfo.usPdb = 0;

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
         ReadCluster(pCD, ulCluster, pDirStart);
         pDir    = pDirStart;
         pDirEnd = (PDIRENTRY)((PBYTE)pDirStart + pCD->usClusterSize);

#ifdef CALL_YIELD
         //Yield();
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
               //FSH_UPPERCASE(szShortName, sizeof szShortName, szShortName);
               strupr(szShortName); // !!! @todo DBCS/Unicode
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
         ulCluster = GetNextCluster(pCD, ulCluster);
         if (!ulCluster)
            ulCluster = FAT_EOF;
         }
      }
   free(pDirStart);
   free(pszLongName);
   //if (f32Parms.fMessageActive & LOG_FUNCS)
   //   {
   //   if (ulCluster != FAT_EOF)
   //      Message("FindPathCluster for %s found cluster %ld", pszPath, ulCluster);
   //   else
   //      Message("FindPathCluster for %s returned EOF", pszPath);
   //   }
   return ulCluster;
}

/******************************************************************
*
******************************************************************/
USHORT GetSetFileEAS(PCDINFO pCD, USHORT usFunc, PMARKFILEEASBUF pMark)
{
   ULONG ulDirCluster;
   PSZ   pszFile;

   ulDirCluster = FindDirCluster(pCD,
      //NULL,
      //NULL,
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

      ulCluster = FindPathCluster(pCD, ulDirCluster, pszFile, &DirEntry, NULL);
      if (ulCluster == FAT_EOF)
         return ERROR_FILE_NOT_FOUND;
      pMark->fEAS = DirEntry.fEAS;
      return 0;
      }

   return MarkFileEAS(pCD, ulDirCluster, pszFile, pMark->fEAS);
}

/************************************************************************
*
************************************************************************/
USHORT MarkFileEAS(PCDINFO pCD, ULONG ulDirCluster, PSZ pszFileName, BYTE fEAS)
{
   ULONG ulCluster;
   DIRENTRY OldEntry, NewEntry;
   USHORT rc;

   ulCluster = FindPathCluster(pCD, ulDirCluster, pszFileName, &OldEntry, NULL);
   if (ulCluster == FAT_EOF)
      //{
      //CritMessage("FAT32: MarkfileEAS : %s not found!", pszFileName);
      return ERROR_FILE_NOT_FOUND;
      //}
   memcpy(&NewEntry, &OldEntry, sizeof (DIRENTRY));
   if( HAS_OLD_EAS( NewEntry.fEAS ))
        NewEntry.fEAS = FILE_HAS_NO_EAS;
   NewEntry.fEAS = ( BYTE )(( NewEntry.fEAS & FILE_HAS_WINNT_EXT ) | fEAS );

   if (!memcmp(&NewEntry, &OldEntry, sizeof (DIRENTRY)))
      return 0;

   rc = ModifyDirectory(pCD, ulDirCluster,
      MODIFY_DIR_UPDATE, &OldEntry, &NewEntry, NULL);

   return rc;
}

/************************************************************************
*
************************************************************************/
USHORT ModifyDirectory(PCDINFO pCD, ULONG ulDirCluster, USHORT usMode, PDIRENTRY pOld, PDIRENTRY pNew, PSZ pszLongName) //, USHORT usIOMode)
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

   //if (f32Parms.fMessageActive & LOG_FUNCS)
   //   Message("ModifyDirectory DirCluster %ld, Mode = %d",
   //   ulDirCluster, usMode);

   if (usMode == MODIFY_DIR_RENAME ||
       usMode == MODIFY_DIR_INSERT)
      {
      if (!pNew || !pszLongName)
         return ERROR_INVALID_PARAMETER;

      memcpy(&DirNew, pNew, sizeof (DIRENTRY));
      if ((pNew->bAttr & 0x0F) != FILE_VOLID)
         {
         rc = MakeShortName(pCD, ulDirCluster, pszLongName, DirNew.bFileName);
         if (rc == LONGNAME_ERROR)
            return ERROR_FILE_EXISTS;
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
         return ERROR_INVALID_PARAMETER;
      }

   pDirectory = (PDIRENTRY)malloc(2 * pCD->usClusterSize);
   if (!pDirectory)
      return ERROR_NOT_ENOUGH_MEMORY;

   memset(pDirectory, 0, pCD->usClusterSize);
   pDir2 =(PDIRENTRY)((PBYTE)pDirectory + pCD->usClusterSize);
   memset(pDir2, 0, pCD->usClusterSize);
   pMax = (PDIRENTRY)((PBYTE)pDirectory + pCD->usClusterSize * 2);

   ulCluster = ulDirCluster;
   pLNStart = NULL;
   ulPrevCluster = FAT_EOF;
   usClusterCount = 0;
   fNewCluster = FALSE;
   while (ulCluster != FAT_EOF)
      {
#ifdef CALL_YIELD
      //Yield();
#endif

      usClusterCount++;
      if (!fNewCluster)
         {
         rc = ReadCluster(pCD, ulCluster, pDir2); //, usIOMode);
         if (rc)
            {
            free(pDirectory);
            return rc;
            }
         }
      else
         {
         memset(pDir2, 0, pCD->usClusterSize);
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
                     //if (f32Parms.fMessageActive & LOG_FUNCS)
                     //   Message(" Updating cluster");
                     memcpy(pWork, pNew, sizeof (DIRENTRY));
                     rc = WriteCluster(pCD, ulCluster, pDir2); //, usIOMode);
                     if (rc)
                        {
                        free(pDirectory);
                        return rc;
                        }
                     ulCluster = FAT_EOF;
                     break;

                  case MODIFY_DIR_DELETE:
                  case MODIFY_DIR_RENAME:
                     //if (f32Parms.fMessageActive & LOG_FUNCS)
                     //   Message(" Removing entry from cluster");
                     pWork2 = pLNStart;
                     while (pWork2 < pWork)
                        {
                        //if (f32Parms.fMessageActive & LOG_FUNCS)
                        //   Message("Deleting Longname entry.");
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
                        rc = WriteCluster(pCD, ulPrevCluster, pDirectory); //, usIOMode);
                        if (rc)
                           {
                           free(pDirectory);
                           return rc;
                           }
                        }

                     /*
                        Write current cluster
                     */
                     rc = WriteCluster(pCD, ulCluster, pDir2); //, usIOMode);
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
            if (ulPrevCluster != FAT_EOF && GetFreeEntries(pDirectory, pCD->usClusterSize * 2) >= usEntriesNeeded)
               {
               BYTE bCheck = GetVFATCheckSum(&DirNew);

               //if (f32Parms.fMessageActive & LOG_FUNCS)
               //   Message(" Inserting entry into 2 clusters");

               pWork = (PDIRENTRY)CompactDir(pDirectory, pCD->usClusterSize * 2, usEntriesNeeded);
               pWork = (PDIRENTRY)fSetLongName(pWork, pszLongName, bCheck);
               memcpy(pWork, &DirNew, sizeof (DIRENTRY));

               rc = WriteCluster(pCD, ulPrevCluster, pDirectory); //, usIOMode);
               if (rc)
                  {
                  free(pDirectory);
                  return rc;
                  }

               rc = WriteCluster(pCD, ulCluster, pDir2); //, usIOMode);
               if (rc)
                  {
                  free(pDirectory);
                  return rc;
                  }
               ulCluster = FAT_EOF;
               break;
               }

            usFreeEntries = GetFreeEntries(pDir2, pCD->usClusterSize);
            if (usFreeEntries >= usEntriesNeeded)
               {
               BYTE bCheck = GetVFATCheckSum(&DirNew);

               //if (f32Parms.fMessageActive & LOG_FUNCS)
               //   Message(" Inserting entry into 1 cluster");

               pWork = (PDIRENTRY)CompactDir(pDir2, pCD->usClusterSize, usEntriesNeeded);
               pWork = (PDIRENTRY)fSetLongName(pWork, pszLongName, bCheck);
               memcpy(pWork, &DirNew, sizeof (DIRENTRY));
               rc = WriteCluster(pCD, ulCluster, pDir2); //, usIOMode);
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
               MarkFreeEntries(pDir2, pCD->usClusterSize);
               rc = WriteCluster(pCD, ulCluster, pDir2); //, usIOMode);
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
         memmove(pDirectory, pDir2, pCD->usClusterSize);
         if (pLNStart)
            pLNStart = (PDIRENTRY)((PBYTE)pLNStart - pCD->usClusterSize);


         ulNextCluster = GetNextCluster(pCD, ulCluster);
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

            ulNextCluster = SetNextCluster(pCD, ulCluster, FAT_ASSIGN_NEW);
            if (ulNextCluster == FAT_EOF)
               {
               free(pDirectory);
               //Message("Modify Directory: Disk Full!");
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

/************************************************************************
*
************************************************************************/
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

/************************************************************************
*
************************************************************************/
VOID Translate2OS2(PUSHORT pusUni, PSZ pszName, USHORT usLen)
{
   USHORT usPage;
   USHORT usChar;
   USHORT usCode;

   //if (!f32Parms.fTranslateNames)
   //   {
   //   while (*pusUni && usLen)
   //      {
   //      *pszName++ = (BYTE)*pusUni++;
   //      usLen--;
   //      }

   //   return;
   //   }

/*
   GetCurrentCodePage();
*/

   while (*pusUni && usLen)
      {
      usPage = ((*pusUni) >> 8) & 0x00FF;
      usChar = (*pusUni) & 0x00FF;

      usCode = QueryUni2NLS( usPage, usChar );
      *pszName++ = ( BYTE )( usCode & 0x00FF );
      if( usCode & 0xFF00 )
         {
         *pszName++ = ( BYTE )(( usCode >> 8 ) & 0x00FF );
         usLen--;
         }

      pusUni++;
      usLen--;
      }
}

/************************************************************************
*
************************************************************************/
USHORT QueryUni2NLS( USHORT usPage, USHORT usChar )
{
    return rgPage[ usPage / MAX_ARRAY_PAGE ][ usPage % MAX_ARRAY_PAGE ].usCode[ usChar ];
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
USHORT MakeShortName(PCDINFO pCD, ULONG ulDirCluster, PSZ pszLongName, PSZ pszShortName)
{
   USHORT usLongName;
   PSZ pLastDot;
   PSZ pFirstDot;
   PSZ p;
   USHORT usIndex;
   BYTE szShortName[12];
   PSZ  pszUpper;
   USHORT rc;

   //if (f32Parms.fMessageActive & LOG_FUNCS)
   //   Message("MakeShortName for %s, dircluster %lu",
   //      pszLongName, ulDirCluster);

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

   //rc = FSH_UPPERCASE(pszLongName, usIndex, pszUpper);
   strupr(pszLongName); // !!! @todo DBCS/Unicode
   //if (rc)
   //   {
   //   free(pszUpper);
   //   return LONGNAME_ERROR;
   //   }

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
   if ((!strcmp(pszLongName, pszUpper)) && // || IsDosSession()) &&
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
         ulCluster = FindPathCluster(pCD, ulDirCluster, szFileName, NULL, NULL);
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

/******************************************************************
*
******************************************************************/
USHORT Translate2Win(PSZ pszName, PUSHORT pusUni, USHORT usLen)
{
   USHORT usCode;
   USHORT usProcessedLen;

   usProcessedLen = 0;

   //if (!f32Parms.fTranslateNames)
   //   {
   //   while (*pszName && usLen)
   //      {
   //      *pusUni++ = (USHORT)*pszName++;
   //      usLen--;
   //      usProcessedLen++;
   //      }
   //   return usProcessedLen;
   //   }

/*
   GetCurrentCodePage();
*/

   while (*pszName && usLen)
      {
      usCode = *pszName++;
      if( IsDBCSLead(( UCHAR )usCode ))
         {
         usCode |= (( USHORT )*pszName++ << 8 ) & 0xFF00;
         usProcessedLen++;
         }

      *pusUni++ = QueryNLS2Uni( usCode );
      usLen--;
      usProcessedLen++;
      }

   return usProcessedLen;
}

/******************************************************************
*
******************************************************************/
USHORT QueryNLS2Uni( USHORT usCode )
{
    return rgUnicode[ usCode / MAX_ARRAY_UNICODE ][ usCode % MAX_ARRAY_UNICODE ];
}

/******************************************************************
*
******************************************************************/
ULONG SetNextCluster(PCDINFO pCD, ULONG ulCluster, ULONG ulNext)
{
   //if (f32Parms.fMessageActive & LOG_FUNCS)
   //   Message("SetNextCluster");

   //if (GetFatAccess(pVolInfo, "SetNextCluster"))
   //   return FAT_EOF;

   ulCluster = SetNextCluster2(pCD, ulCluster, ulNext);
   //ReleaseFat(pVolInfo);
   return ulCluster;
}

/******************************************************************
*
******************************************************************/
ULONG SetNextCluster2(PCDINFO pCD, ULONG ulCluster, ULONG ulNext)
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
      ulCluster = GetFreeCluster(pCD);
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
      ulNext = SetNextCluster2(pCD, FAT_ASSIGN_NEW, FAT_EOF);
      if (ulNext == FAT_EOF)
         return FAT_EOF;
      ulReturn = ulNext;
      }

   if (ReadFatSector(pCD, ulCluster / 128))
      return FAT_EOF;

   fUpdateFSInfo = FALSE;
   pulCluster = (PULONG)pCD->pbFATSector + (ulCluster % 128);
   if (*pulCluster && !ulNext)
      {
      fUpdateFSInfo = TRUE;
      pCD->FSInfo.ulFreeClusters++;
      }
   if (*pulCluster == 0 && ulNext)
      {
      fUpdateFSInfo = TRUE;
      pCD->FSInfo.ulNextFreeCluster = ulCluster;
      pCD->FSInfo.ulFreeClusters--;
      }

   *pulCluster = ulNext;

   rc = WriteFatSector(pCD, ulCluster / 128);
   if (rc)
      return FAT_EOF;

   if (fUpdateFSInfo) ////
      UpdateFSInfo(pCD);

   return ulReturn;
}

/******************************************************************
*
******************************************************************/
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

/******************************************************************
*
******************************************************************/
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

/******************************************************************
*
******************************************************************/
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

/******************************************************************
*
******************************************************************/
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

/******************************************************************
*
******************************************************************/
ULONG GetFreeCluster(PCDINFO pCD)
{
   ULONG ulStartCluster;
   ULONG ulCluster;
   BOOL fStartAt2;

   //if (f32Parms.fMessageActive & LOG_FUNCS)
   //   Message("GetFreeCluster");

   if (pCD->FSInfo.ulFreeClusters == 0L)
      return FAT_EOF;

   fStartAt2 = FALSE;
   ulCluster = pCD->FSInfo.ulNextFreeCluster + 1;
   if (!ulCluster || ulCluster >= pCD->ulTotalClusters + 2)
      {
      fStartAt2 = TRUE;
      ulCluster = 2;
      ulStartCluster = pCD->ulTotalClusters + 2;
      }
   else
      ulStartCluster = ulCluster;

   while (GetNextCluster2(pCD, ulCluster))
      {
      ulCluster++;
      if (fStartAt2 && ulCluster >= ulStartCluster)
         return FAT_EOF;

      if (ulCluster >= pCD->ulTotalClusters + 2)
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
USHORT SetFileSize(PCDINFO pCD, PFILESIZEDATA pFileSize)
{
   ULONG ulDirCluster;
   PSZ   pszFile;
   ULONG ulCluster;
   DIRENTRY DirEntry;
   DIRENTRY DirNew;
   ULONG ulClustersNeeded;
   ULONG ulClustersUsed;
   USHORT rc;

   ulDirCluster = FindDirCluster(pCD,
      //NULL,
      //NULL,
      pFileSize->szFileName,
      0xFFFF,
      RETURN_PARENT_DIR,
      &pszFile);

   if (ulDirCluster == FAT_EOF)
      return ERROR_PATH_NOT_FOUND;

   ulCluster = FindPathCluster(pCD, ulDirCluster, pszFile, &DirEntry, NULL);
   if (ulCluster == FAT_EOF)
      return ERROR_FILE_NOT_FOUND;
   if (!ulCluster)
      pFileSize->ulFileSize = 0L;

   ulClustersNeeded = pFileSize->ulFileSize / pCD->usClusterSize;
   if (pFileSize->ulFileSize % pCD->usClusterSize)
      ulClustersNeeded++;

   if (pFileSize->ulFileSize > 0 )
      {
      ulClustersUsed = 1;
      while (ulClustersUsed < ulClustersNeeded)
         {
         ULONG ulNextCluster = GetNextCluster(pCD, ulCluster);
         if (!ulNextCluster)
            break;
         ulCluster = ulNextCluster;
         if (ulCluster == FAT_EOF)
            break;
         ulClustersUsed++;
         }
      if (ulCluster == FAT_EOF)
         pFileSize->ulFileSize = ulClustersUsed * pCD->usClusterSize;
      else
         SetNextCluster(pCD, ulCluster, FAT_EOF);
      }

   memcpy(&DirNew, &DirEntry, sizeof (DIRENTRY));
   DirNew.ulFileSize = pFileSize->ulFileSize;

   if (!pFileSize->ulFileSize)
      {
      DirNew.wCluster = 0;
      DirNew.wClusterHigh = 0;
      }


   rc = ModifyDirectory(pCD, ulDirCluster, MODIFY_DIR_UPDATE,
      &DirEntry, &DirNew, NULL);

   return rc;
}

/******************************************************************
*
******************************************************************/
USHORT RecoverChain2(PCDINFO pCD, ULONG ulCluster, PBYTE pData, USHORT cbData)
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
         if (FindPathCluster(pCD, pCD->BootSect.bpb.RootDirStrtClus,
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
      DirEntry.ulFileSize += pCD->usClusterSize;
      ulNextCluster = GetNextCluster(pCD, ulCluster);
      if (!ulNextCluster)
         {
         SetNextCluster(pCD, ulCluster, FAT_EOF);
         ulCluster = FAT_EOF;
         }
      else
         ulCluster = ulNextCluster;
      }

   return MakeDirEntry(pCD,
      pCD->BootSect.bpb.RootDirStrtClus,
      &DirEntry, szFileName);
}

/******************************************************************
*
******************************************************************/
USHORT MakeDirEntry(PCDINFO pCD, ULONG ulDirCluster, PDIRENTRY pNew, PSZ pszName)
{
   DATETIME pdt;

   //if (f32Parms.fMessageActive & LOG_FUNCS)
   //   Message("MakeDirEntry %s", pszName);

   DosGetDateTime(&pdt);

   //if (pGI)
      //{
      //pNew->wLastWriteDate.year = pGI->year - 1980;
      //pNew->wLastWriteDate.month = pGI->month;
      //pNew->wLastWriteDate.day = pGI->day;
      //pNew->wLastWriteTime.hours = pGI->hour;
      //pNew->wLastWriteTime.minutes = pGI->minutes;
      //pNew->wLastWriteTime.twosecs = pGI->seconds / 2;

      pNew->wLastWriteDate.year = pdt.year;
      pNew->wLastWriteDate.month = pdt.month;
      pNew->wLastWriteDate.day = pdt.day;
      pNew->wLastWriteTime.hours = pdt.hours;
      pNew->wLastWriteTime.minutes = pdt.minutes;
      pNew->wLastWriteTime.twosecs = pdt.seconds / 2;

      pNew->wCreateDate = pNew->wLastWriteDate;
      pNew->wCreateTime = pNew->wLastWriteTime;
      pNew->wAccessDate = pNew->wLastWriteDate;
      //}

   return ModifyDirectory(pCD, ulDirCluster, MODIFY_DIR_INSERT,
      NULL, pNew, pszName);
}

/******************************************************************
*
******************************************************************/
BOOL DeleteFatChain(PCDINFO pCD, ULONG ulCluster)
{
   ULONG ulNextCluster;
   ULONG ulSector;
   PULONG pulCluster;
   ULONG ulClustersFreed;
   ULONG rc;

   if (!ulCluster)
      return TRUE;

   //if (ulCluster >= 2 && ulCluster < pCD->ulTotalClusters + 2)
   //   {
   //   if (f32Parms.fMessageActive  & LOG_FUNCS)
   //      Message("DeleteFatChain for cluster %lu", ulCluster);
   //   }
   //else
   //   {
   //   Message("DeleteFatChain for invalid cluster %lu (ERROR)", ulCluster);
   //   return FALSE;
   //   }

   //if (GetFatAccess(pVolInfo, "DeleteFatChain"))
   //   return FALSE;

   ulSector = ulCluster / 128;
   ReadFatSector(pCD, ulSector);
   ulClustersFreed = 0;
   while (!(ulCluster >= FAT_EOF2 && ulCluster <= FAT_EOF))
      {
#ifdef CALL_YIELD
      //Yield();
#endif

      if (!ulCluster || ulCluster == FAT_BAD_CLUSTER)
         {
         //Message("DeleteFatChain: Bad Chain (Cluster %lu)",
         //   ulCluster);
         break;
         }
      ulSector = ulCluster / 128;
      if (ulSector != pCD->ulCurFATSector)
         {
         rc = WriteFatSector(pCD, pCD->ulCurFATSector);
         if (rc)
            //{
            //ReleaseFat(pVolInfo);
            return FALSE;
            //}
         ReadFatSector(pCD, ulSector);
         }
      pulCluster = (PULONG)pCD->pbFATSector + (ulCluster % 128);

      ulNextCluster = *pulCluster;
      *pulCluster = 0L;
      ulClustersFreed++;
      ulCluster = ulNextCluster;
      }
   rc = WriteFatSector(pCD, pCD->ulCurFATSector);
   if (rc)
      //{
      //ReleaseFat(pVolInfo);
      return FALSE;
      //}

   pCD->FSInfo.ulFreeClusters += ulClustersFreed;
   UpdateFSInfo(pCD); ////

   //ReleaseFat(pVolInfo);

   return TRUE;
}

/******************************************************************
*
******************************************************************/
VOID GetFirstInfo( PBOOL pFirstInfo )
{
    memcpy( pFirstInfo, rgFirstInfo, sizeof( rgFirstInfo ));
}

/******************************************************************
*
******************************************************************/
BOOL TranslateInit(PVOID16 rgTrans[], USHORT usSize)
{
   ULONG  ulCode;
   USHORT usPage;
   USHORT usChar;
   INT    iIndex;

   PVOID16 *prgTrans = ( PVOID16 * )rgTrans;

   if( rgPage[ 0 ] == NULL )
      TranslateAllocBuffer();

   if (usSize != sizeof( PVOID ) * ( ARRAY_COUNT_UNICODE + 2 ) )
      return FALSE;

   for( iIndex = 0; iIndex < ARRAY_COUNT_UNICODE; iIndex++ )
      memcpy( rgUnicode[ iIndex ], (void *)(void _Far16 *)prgTrans[ iIndex ], sizeof( USHORT ) * MAX_ARRAY_UNICODE );

   for( iIndex = 0; iIndex < MAX_ARRAY_UNICODE; iIndex++ )
      rgFirstInfo[ iIndex ] = ( UCHAR )((( PUSHORT )( prgTrans[ ARRAY_COUNT_UNICODE ] ))[ iIndex ]);

   for( iIndex = 0; iIndex < MAX_ARRAY_UNICODE; iIndex++ )
      rgLCase[ iIndex ] = ( UCHAR )((( PUSHORT )( prgTrans[ ARRAY_COUNT_UNICODE + 1 ] ))[ iIndex ]);

   for( iIndex = 0; iIndex < ARRAY_COUNT_PAGE; iIndex++ )
      memset( rgPage[ iIndex ], '_', sizeof( UNIPAGE ) * MAX_ARRAY_PAGE );

   for (ulCode = 0; ulCode < 0x10000; ulCode++)
      {
      usPage = (QueryNLS2Uni(( USHORT )ulCode ) >> 8) & 0x00FF;
      usChar = QueryNLS2Uni(( USHORT )ulCode ) & 0x00FF;

      SetUni2NLS( usPage, usChar, ( USHORT )ulCode );
      }

   //f32Parms.fTranslateNames = TRUE;

   return TRUE;
}

/******************************************************************
*
******************************************************************/
VOID TranslateAllocBuffer( VOID )
{
   INT iIndex;

   for( iIndex = 0; iIndex < ARRAY_COUNT_PAGE; iIndex++ )
      rgPage[ iIndex ] = malloc( sizeof( UNIPAGE ) * MAX_ARRAY_PAGE );

   for( iIndex = 0; iIndex < ARRAY_COUNT_UNICODE; iIndex++ )
      rgUnicode[ iIndex ] = malloc( sizeof( USHORT ) * MAX_ARRAY_UNICODE );
}

/******************************************************************
*
******************************************************************/
BOOL LoadTranslateTable(VOID)
{
    APIRET rc;
    ULONG ulParmSize;
    BYTE   rgData[ 256 ];
    // Extra space for DBCS lead info and case conversion
    UniChar *rgTranslate[ MAX_TRANS_TABLE + EXTRA_ELEMENT ] = { NULL, };
    PBYTE  pChar;
    UniChar *pUni;
    UconvObject  uconv_object = NULL;
    INT iIndex;
    size_t bytes_left;
    size_t uni_chars_left;
    size_t num_subs;
    ULONG rgCP[3];
    ULONG cbCP;
    // Extra space for DBCS lead info and case conversion
    PVOID16 rgTransTable[ MAX_TRANS_TABLE + EXTRA_ELEMENT ] = { NULL, };
    char rgFirst[ 256 ];
    USHORT first, second;
    USHORT usCode;
    UniChar ucsCp[ 12 ];
    UniChar rgUniBuffer[ ARRAY_TRANS_TABLE ];

   rc = DosLoadModule(rgData, sizeof rgData, "UCONV.DLL", &hModConv);
   if (rc)
      {
      printf("FAT32: No NLS support found (%s does not load).\n", rgData);
      printf("FAT32: No UNICODE translate table loaded!\n");
      rc = TRUE;
      goto free_exit;
      }
   rc = DosQueryProcAddr(hModConv, 0L,
      "UniCreateUconvObject", (PFN *)&pUniCreateUconvObject);
   if (rc)
      {
      printf("FAT32: ERROR: Could not find address of UniCreateUconvObject.\n");
      rc = FALSE;
      goto free_exit;
      }
   rc = DosQueryProcAddr(hModConv, 0L,
      "UniUconvToUcs", (PFN *)&pUniUconvToUcs);
   if (rc)
      {
      printf("FAT32: ERROR: Could not find address of UniUconvToUcs.\n");
      rc = FALSE;
      goto free_exit;
      }

   rc = DosQueryProcAddr(hModConv, 0L,
      "UniUconvFromUcs", (PFN *)&pUniUconvFromUcs);
   if (rc)
      {
      printf("FAT32: ERROR: Could not find address of UniUconvFromUcs.\n");
      rc = FALSE;
      goto free_exit;
      }

   rc = DosQueryProcAddr(hModConv, 0L,
      "UniMapCpToUcsCp", (PFN *)&pUniMapCpToUcsCp);
   if (rc)
      {
      printf("FAT32: ERROR: Could not find address of UniMapCpToUcsCp.\n");
      rc = FALSE;
      goto free_exit;
      }

   rc = DosQueryProcAddr(hModConv, 0L,
      "UniQueryUconvObject", (PFN *)&pUniQueryUconvObject);
   if (rc)
      {
      printf("FAT32: ERROR: Could not find address of UniQueryUconvObject.\n");
      rc = FALSE;
      goto free_exit;
      }

   rc = DosLoadModule(rgData, sizeof rgData, "LIBUNI.DLL", &hModUni);
   if (rc)
      {
      printf("FAT32: No NLS support found (%s does not load).\n", rgData);
      printf("FAT32: No UNICODE translate table loaded!\n");
      rc = TRUE;
      goto free_exit;
      }

   rc = DosQueryProcAddr(hModUni, 0L,
      "UniTolower", (PFN *)&pUniTolower);
   if (rc)
      {
      printf("FAT32: ERROR: Could not find address of UniTolower.\n");
      rc = FALSE;
      goto free_exit;
      }

   if( ulNewCP )
        rgCP[ 0 ] = ulNewCP;
   else
        DosQueryCp(sizeof rgCP, rgCP, &cbCP);

   if (f32Parms.ulCurCP == rgCP[0])
   {
      rc = FALSE;
      goto free_exit;
   }

#if 0
   if (f32Parms.ulCurCP && !fSayYes)
      {
      BYTE chChar;
      printf("Loaded unicode translate table is for CP %lu\n", f32Parms.ulCurCP);
      printf("Current CP is %lu\n", rgCP[0]);
      printf("Would you like to reload the translate table for this CP [Y/N]? ");
      fflush(stdout);

      for (;;)
         {
         chChar = getch();
         switch (chChar)
            {
            case 'y':
            case 'Y':
               chChar = 'Y';
               break;
            case 'n':
            case 'N':
               chChar = 'N';
               break;
            default :
               DosBeep(660, 10);
               continue;
            }
         printf("%c\n", chChar);
         break;
         }
      if (chChar == 'N')
         {
         rc = FALSE;
         goto free_exit;
         }
      }
#endif

   rc = pUniMapCpToUcsCp( rgCP[ 0 ], ucsCp, sizeof( ucsCp ) / sizeof( UniChar ));
   if( rc != ULS_SUCCESS )
   {
        printf("FAT32: ERROR: UniMapCpToUcsCp error: return code = %u\n", rc );
        rc = FALSE;
        goto free_exit;
   }

   rc = pUniCreateUconvObject( ucsCp, &uconv_object);
   if (rc != ULS_SUCCESS)
      {
      printf("FAT32: ERROR: UniCreateUconvObject error: return code = %u\n", rc);
      rc = FALSE;
      goto free_exit;
      }

   rc = pUniQueryUconvObject( uconv_object, NULL, 0, rgFirst, NULL, NULL );
   if (rc != ULS_SUCCESS)
      {
      printf("FAT32: ERROR: UniQueryUConvObject error: return code = %u\n", rc);
      rc = FALSE;
      goto free_exit;
      }

   // Allocation for conversion, DBCS lead info and case conversion
   for( iIndex = 0; iIndex <= INDEX_OF_END ; iIndex ++ )
   {
        rgTransTable[ iIndex ] = rgTranslate[ iIndex ] = malloc( sizeof(USHORT ) * ARRAY_TRANS_TABLE );
        memset( rgTranslate[ iIndex ], 0, sizeof( USHORT ) * ARRAY_TRANS_TABLE );
   }

   // Initialize SBCS only for conversion and set DBCS lead info
   for( iIndex = 0; iIndex < ARRAY_TRANS_TABLE; iIndex++ )
   {
        rgData[ iIndex ] = ( rgFirst[ iIndex ] == 1 ) ? iIndex : 0;
        rgTranslate[ INDEX_OF_FIRSTINFO ][ iIndex ] = rgFirst[ iIndex ];
   }

   pChar = rgData;
   bytes_left = sizeof rgData;
   pUni = ( PVOID )rgTranslate[ 0 ];
   uni_chars_left = ARRAY_TRANS_TABLE;

   rc = pUniUconvToUcs(uconv_object,
      (PVOID *)&pChar,
      &bytes_left,
      &pUni,
      &uni_chars_left,
      &num_subs);

   if (rc != ULS_SUCCESS)
      {
      printf("FAT32: ERROR: UniUconvToUcs failed, rc = %u\n", rc);
      rc = FALSE;
      goto free_exit;
      }

   // Translate upper case to lower case
   for( iIndex = 0; iIndex < ARRAY_TRANS_TABLE; iIndex++ )
        rgUniBuffer[ iIndex ] = pUniTolower( rgTranslate[ 0 ][ iIndex ] );

   // Convert lower case in Unicode to codepage code
   pUni = ( PVOID )rgUniBuffer;
   uni_chars_left = ARRAY_TRANS_TABLE;
   pChar  = rgData;
   bytes_left = sizeof rgData;

   rc = pUniUconvFromUcs( uconv_object,
        &pUni,
        &uni_chars_left,
        ( PVOID * )&pChar,
        &bytes_left,
        &num_subs );

   if (rc != ULS_SUCCESS)
      {
      printf("FAT32: ERROR: UniUconvFromUcs failed, rc = %u\n", rc);
      rc = FALSE;
      goto free_exit;
      }

   // Store codepage code to transtable
   for( iIndex = 0; iIndex < ARRAY_TRANS_TABLE; iIndex++ )
        rgTranslate[ INDEX_OF_LCASECONV ][ iIndex ] = rgData[ iIndex ] ? rgData[ iIndex ] : iIndex;

   // Translate DBCS code to unicode
   for( first = 0; first < ARRAY_TRANS_TABLE; first++ )
   {
        if( rgFirst[ first ] == 2 )
        {
            for( second = 0; second < 0x100; second++ )
            {
                  usCode = first | (( second << 8 ) & 0xFF00 );

                  pChar  = ( PVOID )&usCode;
                  bytes_left = sizeof usCode;
                  pUni = ( PVOID )&rgTranslate[ second ][ first ];
                  uni_chars_left = 1;

                  rc = pUniUconvToUcs(uconv_object,
                     (PVOID *)&pChar,
                     &bytes_left,
                     &pUni,
                     &uni_chars_left,
                     &num_subs);

                  if (rc != ULS_SUCCESS)
                  {
                     printf("FAT32: ERROR: UniUconvToUcs failed, rc = %u\n", rc);
                     rc = FALSE;
                     goto free_exit;
                  }
            }
        }
   }

   ulParmSize = sizeof rgTransTable;
   //rc = DosFSCtl(NULL, 0, NULL,
   //            ( PVOID )rgTransTable, ulParmSize, &ulParmSize,
   //            FAT32_SETTRANSTABLE, "FAT32", -1, FSCTL_FSDNAME);
   if( !TranslateInit((PVOID)rgTransTable, ulParmSize))
      rc = ERROR_INVALID_PARAMETER;
   else
      rc = 0;
   
   if (rc)
      {
      printf("FAT32: ERROR: Unable to set translate table for current Codepage.\n");
      rc = FALSE;
      goto free_exit;
      }

   f32Parms.ulCurCP = rgCP[0];
   //if( !fSilent )
       printf("FAT32: Unicode translate table for CP %lu loaded.\n", rgCP[0]);
   rc = TRUE;
free_exit:

   for( iIndex = 0; iIndex <= INDEX_OF_END; iIndex++ )
      if( rgTranslate[ iIndex ])
        free( rgTranslate[ iIndex ]);

   if( hModConv )
        DosFreeModule( hModConv);

   if( hModUni )
        DosFreeModule( hModUni );

   return rc;
}

/******************************************************************
*
******************************************************************/
VOID SetUni2NLS( USHORT usPage, USHORT usChar, USHORT usCode )
{
    rgPage[ usPage / MAX_ARRAY_PAGE ][ usPage % MAX_ARRAY_PAGE ].usCode[ usChar ] = usCode;
}

/******************************************************************
*
******************************************************************/
VOID GetCaseConversion( PUCHAR pCase )
{
    memcpy( pCase, rgLCase, sizeof( rgLCase ));
}

/******************************************************************
*
******************************************************************/
BOOL UpdateFSInfo(PCDINFO pCD)
{
   static BYTE bSector[SECTOR_SIZE] = "";

   //if (f32Parms.fMessageActive & LOG_FUNCS)
   //   Message("UpdateFSInfo");

   //if (pCD->fFormatInProgress)
   //   return FALSE;

   //if (pCD->fWriteProtected)
   //   return TRUE;

   if (pCD->BootSect.bpb.FSinfoSec == 0xFFFF)
      return TRUE;

   if (!ReadSector(pCD, pCD->BootSect.bpb.FSinfoSec, 1, bSector))
      {
      memcpy(bSector + FSINFO_OFFSET, (void *)&pCD->FSInfo, sizeof (BOOTFSINFO));
      if (!WriteSector(pCD, pCD->BootSect.bpb.FSinfoSec, 1, bSector))
         return TRUE;
      }
   //CritMessage("UpdateFSInfo for %c: failed!", pVolInfo->bDrive + 'A');
   //Message("ERROR: UpdateFSInfo for %c: failed!", pVolInfo->bDrive + 'A');

   return FALSE;
}
