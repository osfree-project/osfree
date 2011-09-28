/*  osFree FSH32 file system helpers
 *
 *
 *
 */

#ifndef  __FSH_H__
#define  __FSH_H__

#include <gcc_os2def.h>
#include <fsd.h>

FSRET FSENTRY FSH_ADDSHARE(char *pName,
                             USHORT mode,
                             USHORT hVPB,
                             ULONG *phShare);

FSRET FSENTRY FSH_CALLDRIVER(void *pPkt,
                               USHORT hVPB,
                               USHORT fControl);

FSRET FSENTRY FSH_CANONICALIZE(char *pPathName,
                                 USHORT cbPathBuf,
                                 char *pPathBuf,
                                 USHORT *pFl);

FSRET FSENTRY FSH_CHECKEANAME(USHORT iLevel,
                                ULONG cbEAName,
                                char *szEAN);

FSRET FSENTRY FSH_CRITERROR(USHORT cbMessage,
                              char *pMessage,
                              USHORT nSubs,
                              char *pSubs,
                              USHORT fAll);

FSRET FSENTRY FSH_DEVIOCTL(USHORT flag,
                             ULONG hDev,
                             USHORT sfn,
                             USHORT cat,
                             USHORT func,
                             char *pParm,
                             USHORT cbParm,
                             char *pData,
                             USHORT cbData);

FSRET FSENTRY FSH_DOVOLIO(USHORT operation,
                            USHORT fAllowed,
                            USHORT hVPB,
                            char *pData,
                            USHORT *pcSec,
                            ULONG iSec);


FSRET FSENTRY FSH_DOVOLIO2(ULONG hDev,
                             USHORT sfn,
                             USHORT cat,
                             USHORT func,
                             char *pParm,
                             USHORT cbParm,
                             char *pData
                             USHORT cbData);

FSRET FSENTRY FSH_FINDCHAR(USHORT nChars,
                             char *pChars,
                             char **ppStr);

FSRET FSENTRY FSH_FINDDUPHVPB(USHOR hVPB,
                                USHORT *phVPB);

FSRET FSENTRY FSH_FORCENOSWAP(USHORT sel);

FSRET FSENTRY FSH_GETPRIORITY(void);

FSRET FSENTRY FSH_GETOVERLAPBUF(USHORT hVPB,
    ULONG iSec,
    ULONG iSec2,     // (?)
    ULONG *pisecBuf,
    char **ppBuf);

void  FSENTRY  FSH_GETVOLPARM(USHORT hVPB,
                               struct vpfsi **ppVPBfsi,
                               struct vpfsd **ppVPBfsd);

void  FSENTRY  FSH_INTERR(char *pMsg,
                           USHORT cbMsg);

void  FSENTRY  FSH_IOBOOST(void);

FSRET FSENTRY FSH_IOSEMCLEAR(HEV pSem);

FSRET FSENTRY FSH_ISCURDIRPREFIX(char *pMsg);

void  FSENTRY FSH_LOADCHAR(char **ppStr,
                             USHORT *pChar);

FSRET FSENTRY FSH_NAMEFROMSFN(USHORT sfn,
                                char *pName,
                                USHORT *pcbName);

void  FSENTRY FSH_PREVCHAR(char *pBeg,
                             char **ppStr);

FSRET FSENTRY FSH_PROBEBUF(USHOR operation,
                             char *pData,
                             USHORT cbData);

FSRET FSENTRY FSH_QSYSINFO(USHORT index,
                             char *pData,
                             USHORT cbData);

FSRET FSENTRY FSH_REGISTERPERFCTRS(void *pDataBlk,
                                     void * pTextBlk,
                                     USHORT fsFlags);

void  FSENTRY FSH_REMOVESHARE(ULONG hShare);

FSRET FSENTRY FSH_SEGALLOC(USHORT flags,
                             ULONG cbSeg,
                             USHORT *pSel);

FSRET FSENTRY FSH_SEGFREE(USHORT sel);

FSRET FSENTRY FSH_SEGREALLOC(USHORT sel,
                               ULONG cbSeg);

FSRET FSENTRY FSH_SEMCLEAR(void *pSem);

FSRET FSENTRY FSH_SEMREQUEST(void *pSem,
                               ULONG cmsTimeout);

FSRET FSENTRY FSH_SEMSET(void *pSem);

FSRET FSENTRY FSH_SEMSETWAIT(void *pSem,
                               ULONG cmsTimeout);

FSRET FSENTRY FSH_SEMWAIT(void *pSem,
                            ULONG cmsTimeout);

FSRET FSENTRY FSH_SETVOLUME(USHORT hVPB,
                              USHORT fControl);

FSRET FSENTRY FSH_STORECHAR(USHORT chDBCS,
                              char **ppStr);

FSRET FSENTRY FSH_UPPERCASE(char *szPathName,
                              USHORT cbPathBuf,
                              char *pPathBuf);

FSRET FSENTRY FSH_WILDMATCH(char *pPat,
                              char *pStr);

void  FSENTRY FSH_YIELD(void);

#endif
