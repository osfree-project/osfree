/*  osFree FSH32 file system helpers
 *
 *
 *
 */

#include <gcc_os2def.h>

#define FSHRET    int
#define FSHENTRY  _System

FSHRET FSHENTRY FSH_ADDSHARE(char *pName,
                             USHORT mode,
                             USHORT hVPB,
                             ULONG *phShare);

FSHRET FSHENTRY FSH_CALLDRIVER(void *pPkt,
                               USHORT hVPB,
                               USHORT fControl);

FSHRET FSHENTRY FSH_CANONICALIZE(char *pPathName,
                                 USHORT cbPathBuf,
                                 char *pPathBuf,
                                 USHORT *pFl);

FSHRET FSHENTRY FSH_CHECKEANAME(USHORT iLevel,
                                ULONG cbEAName,
                                char *szEAN);

FSHRET FSHENTRY FSH_CRITERROR(USHORT cbMessage,
                              char *pMessage,
                              USHORT nSubs,
                              char *pSubs,
                              USHORT fAll);

FSHRET FSHENTRY FSH_DEVIOCTL(USHORT flag,
                             ULONG hDev,
                             USHORT sfn,
                             USHORT cat,
                             USHORT func,
                             char *pParm,
                             USHORT cbParm,
                             char *pData,
                             USHORT cbData);

FSHRET FSHENTRY FSH_DOVOLIO(USHORT operation,
                            USHORT fAllowed,
                            USHORT hVPB,
                            char *pData,
                            USHORT *pcSec,
                            ULONG iSec);


FSHRET FSHENTRY FSH_DOVOLIO2(ULONG hDev,
                             USHORT sfn,
                             USHORT cat,
                             USHORT func,
                             char *pParm,
                             USHORT cbParm,
                             char *pData
                             USHORT cbData);

FSHRET FSHENTRY FSH_FINDCHAR(USHORT nChars,
                             char *pChars,
                             char **ppStr);

FSHRET FSHENTRY FSH_FINDDUPHVPB(USHOR hVPB,
                                USHORT *phVPB);

FSHRET FSHENTRY FSH_FORCENOSWAP(USHORT sel);

FSHRET FSHENTRY FSH_GETPRIORITY(void);

void  FSHENTRY  FSH_GETVOLPARM(USHORT hVPB,
                               struct vpfsi **ppVPBfsi,
                               struct vpfsd **ppVPBfsd);

void  FSHENTRY  FSH_INTERR(char *pMsg,
                           USHORT cbMsg);

void  FSHENTRY  FSH_IOBOOST(void);

FSHRET FSHENTRY FSH_IOSEMCLEAR(HEV pSem);

FSHRET FSHENTRY FSH_ISCURDIRPREFIX(char *pMsg);

void   FSHENTRY FSH_LOADCHAR(char **ppStr,
                             USHORT *pChar);

FSHRET FSHENTRY FSH_NAMEFROMSFN(USHORT sfn,
                                char *pName,
                                USHORT *pcbName);

void   FSHENTRY FSH_PREVCHAR(char *pBeg,
                             char **ppStr);

FSHRET FSHENTRY FSH_PROBEBUF(USHOR operation,
                             char *pData,
                             USHORT cbData);

FSHRET FSHENTRY FSH_QSYSINFO(USHORT index,
                             char *pData,
                             USHORT cbData);

FSHRET FSHENTRY FSH_REGISTERPERFCTRS(void *pDataBlk,
                                     void * pTextBlk,
                                     USHORT fsFlags);

void   FSHENTRY FSH_REMOVESHARE(ULONG hShare);

FSHRET FSHENTRY FSH_SEGALLOC(USHORT flags,
                             ULONG cbSeg,
                             USHORT *pSel);

FSHRET FSHENTRY FSH_SEGFREE(USHORT sel);

FSHRET FSHENTRY FSH_SEGREALLOC(USHORT sel,
                               ULONG cbSeg);

FSHRET FSHENTRY FSH_SEMCLEAR(void *pSem);

FSHRET FSHENTRY FSH_SEMREQUEST(void *pSem,
                               ULONG cmsTimeout);

FSHRET FSHENTRY FSH_SEMSET(void *pSem);

FSHRET FSHENTRY FSH_SEMSETWAIT(void *pSem,
                               ULONG cmsTimeout);

FSHRET FSHENTRY FSH_SEMWAIT(void *pSem,
                            ULONG cmsTimeout);

FSHRET FSHENTRY FSH_SETVOLUME(USHORT hVPB,
                              USHORT fControl);

FSHRET FSHENTRY FSH_STORECHAR(USHORT chDBCS,
                              char **ppStr);

FSHRET FSHENTRY FSH_UPPERCASE(char *szPathName,
                              USHORT cbPathBuf,
                              char *pPathBuf);

FSHRET FSHENTRY FSH_WILDMATCH(char *pPat,
                              char *pStr);

void   FSHENTRY FSH_YIELD(void);

