/*  filesystem helpers library
 *
 */

#include <fsh.h>
 
FSRET FSENTRY FSH_ADDSHARE(char *pName,
                             USHORT mode,
                             USHORT hVPB,
                             ULONG *phShare)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_CALLDRIVER(void *pPkt,
                               USHORT hVPB,
                               USHORT fControl)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_CANONICALIZE(char *pPathName,
                                 USHORT cbPathBuf,
                                 char *pPathBuf,
                                 USHORT *pFl)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_CHECKEANAME(USHORT iLevel,
                                ULONG cbEAName,
                                char *szEAN)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_CRITERROR(USHORT cbMessage,
                              char *pMessage,
                              USHORT nSubs,
                              char *pSubs,
                              USHORT fAll)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_DEVIOCTL(USHORT flag,
                             ULONG hDev,
                             USHORT sfn,
                             USHORT cat,
                             USHORT func,
                             char *pParm,
                             USHORT cbParm,
                             char *pData,
                             USHORT cbData)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_DOVOLIO(USHORT operation,
                            USHORT fAllowed,
                            USHORT hVPB,
                            char *pData,
                            USHORT *pcSec,
                            ULONG iSec)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_DOVOLIO2(ULONG hDev,
                             USHORT sfn,
                             USHORT cat,
                             USHORT func,
                             char *pParm,
                             USHORT cbParm,
                             char *pData
                             USHORT cbData)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_FINDCHAR(USHORT nChars,
                             char *pChars,
                             char **ppStr)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_FINDDUPHVPB(USHOR hVPB,
                                USHORT *phVPB)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_FORCENOSWAP(USHORT sel)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_GETPRIORITY(void)
{
  return ERROR_NOT_SUPPORTED;
}

void  FSENTRY  FSH_GETVOLPARM(USHORT hVPB,
                               struct vpfsi **ppVPBfsi,
                               struct vpfsd **ppVPBfsd)
{
  return ERROR_NOT_SUPPORTED;
}

void  FSENTRY  FSH_INTERR(char *pMsg,
                           USHORT cbMsg)
{
  return ERROR_NOT_SUPPORTED;
}

void  FSENTRY  FSH_IOBOOST(void)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_IOSEMCLEAR(HEV pSem)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_ISCURDIRPREFIX(char *pMsg)
{
  return ERROR_NOT_SUPPORTED;
}

void  FSENTRY FSH_LOADCHAR(char **ppStr,
                             USHORT *pChar)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_NAMEFROMSFN(USHORT sfn,
                                char *pName,
                                USHORT *pcbName)
{
  return ERROR_NOT_SUPPORTED;
}

void  FSENTRY FSH_PREVCHAR(char *pBeg,
                             char **ppStr)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_PROBEBUF(USHOR operation,
                             char *pData,
                             USHORT cbData)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_QSYSINFO(USHORT index,
                             char *pData,
                             USHORT cbData)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_REGISTERPERFCTRS(void *pDataBlk,
                                     void * pTextBlk,
                                     USHORT fsFlags)
{
  return ERROR_NOT_SUPPORTED;
}

void  FSENTRY FSH_REMOVESHARE(ULONG hShare)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_SEGALLOC(USHORT flags,
                             ULONG cbSeg,
                             USHORT *pSel)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_SEGFREE(USHORT sel)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_SEGREALLOC(USHORT sel,
                               ULONG cbSeg)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_SEMCLEAR(void *pSem)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_SEMREQUEST(void *pSem,
                               ULONG cmsTimeout)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_SEMSET(void *pSem)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_SEMSETWAIT(void *pSem,
                               ULONG cmsTimeout)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_SEMWAIT(void *pSem,
                            ULONG cmsTimeout)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_SETVOLUME(USHORT hVPB,
                              USHORT fControl)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_STORECHAR(USHORT chDBCS,
                              char **ppStr)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_UPPERCASE(char *szPathName,
                              USHORT cbPathBuf,
                              char *pPathBuf)
{
  return ERROR_NOT_SUPPORTED;
}

FSRET FSENTRY FSH_WILDMATCH(char *pPat,
                              char *pStr)
{
  return ERROR_NOT_SUPPORTED;
}

void  FSENTRY FSH_YIELD(void)
{
  return ERROR_NOT_SUPPORTED;
}
