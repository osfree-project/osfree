#include <strnlen.h>

#define INCL_DOSNLS
#define INCL_DOSMVDM
#define INCL_DOSNMPIPES
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDATETIME
#define INCL_DOSEXCEPTIONS
#define INCL_DOSSPINLOCK
#define INCL_DOSQUEUES
#define INCL_DOSSESMGR
#include <os2.h>

#include <stdio.h>
#include <stdarg.h>

//#include "kal.h"

APIRET __cdecl KalLogWrite(PSZ s);

typedef struct _QMRESULT{
    USHORT seg;
    USHORT htme;
    char name[256];
} QMRESULT;

typedef QMRESULT *PQMRESULT;

void log(const char *fmt, ...)
{
  va_list arg_ptr;
  char buf[1024];

  va_start(arg_ptr, fmt);
  vsprintf(buf, fmt, arg_ptr);
  va_end(arg_ptr);

  KalLogWrite (buf);
}

APIRET unimplemented(char *func)
{
  log("%s is not yet implemented!\n", func);
  return 0;
}


APIRET APIENTRY DosQueryRASInfo(ULONG Index, PPVOID Addr)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosDumpProcess(ULONG Flag, ULONG Drive, PID Pid)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosSuppressPopUps(ULONG Flag,
                                       ULONG Drive)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         THK32ALLOCMEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         THK32FREEMEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         THK32ALLOCSTACK(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosOpen2Compt(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosSetTraceInfo(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosLogMode(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosLogEntry(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosGetLogBuffer(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosLogRegister(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosLogRead(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      Dos32FindFromName(HDIR hDir, PVOID pfindbuf, ULONG cbBuf,
                                       PULONG pcFileNames, ULONG ulPosition, PVOID pszFileSpec)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosOPLockRelease(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosOPLockWait(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosOPLockShutdown(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosSetVerify(BOOL32 bFlag)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosErrClass(ULONG code,
                             PULONG pClass,
                             PULONG pAction,
                             PULONG pLocus)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosCreateVDM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosSetFileInfo(HFILE hf,
                                ULONG ulInfoLevel,
                                PVOID pInfoBuf,
                                ULONG cbInfoBuf)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosSetPathInfo(PCSZ    pszPathName,
                                ULONG ulInfoLevel,
                                PVOID pInfoBuf,
                                ULONG cbInfoBuf,
                                ULONG flOptions)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosSetFSInfo(ULONG disknum,
                              ULONG infolevel,
                              PVOID pBuf,
                              ULONG cbBuf)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosQueryVerify(PBOOL32 pBool)

{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY   DosGetDateTime(PDATETIME pdt)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosDevConfig(PSZ pdevinfo, ULONG item)
{
  log("pdevinfo=%lx\n", pdevinfo);
  log("item=%lx\n", item);
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosEnterCritSec(VOID)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosKillProcess(ULONG action,
                               PID pid)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosSetPriority(ULONG scope,
                                ULONG ulClass,
                                LONG  delta,
                                ULONG PorTid)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosCreatePipe(PHFILE phfRead,
                               PHFILE phfWrite,
                               ULONG cb)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosCallNPipe(PCSZ  pszName,
                              PVOID pInbuf,
                              ULONG cbIn,
                              PVOID pOutbuf,
                              ULONG cbOut,
                              PULONG pcbActual,
                              ULONG msec)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosConnectNPipe(HPIPE hpipe)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosDisConnectNPipe(HPIPE hpipe)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosCreateNPipe(PCSZ  pszName,
                                PHPIPE pHpipe,
                                ULONG openmode,
                                ULONG pipemode,
                                ULONG cbInbuf,
                                ULONG cbOutbuf,
                                ULONG msec)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosPeekNPipe(HPIPE hpipe,
                              PVOID pBuf,
                              ULONG cbBuf,
                              PULONG pcbActual,
                              PAVAILDATA pAvail,
                              PULONG pState)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosQueryNPHState(HPIPE hpipe,
                                  PULONG pState)
{
  return unimplemented(__FUNCTION__);
}


APIRET  APIENTRY  DosRawReadNPipe(HPIPE hPipe,
                                  PVOID pBuffer,
                                  ULONG cbRead,
                                  PULONG pcbActual)
{
  return unimplemented(__FUNCTION__);
}


APIRET  APIENTRY  DosRawWriteNPipe(HPIPE hPipe,
                                   PVOID pBuffer,
                                   ULONG cbWrite,
                                   PULONG pcbActual)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosQueryNPipeInfo(HPIPE hpipe,
                                   ULONG infolevel,
                                   PVOID pBuf,
                                   ULONG cbBuf)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosQueryNPipeSemState(HSEM hsem,
                                       PPIPESEMSTATE pnpss,
                                       ULONG cbBuf)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosSetNPHState(HPIPE hpipe,
                                ULONG state)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosSetNPipeSem(HPIPE hpipe,
                                HSEM hsem,
                                ULONG key)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosTransactNPipe(HPIPE hpipe,
                                  PVOID pOutbuf,
                                  ULONG cbOut,
                                  PVOID pInbuf,
                                  ULONG cbIn,
                                  PULONG pcbRead)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosWaitNPipe(PCSZ  pszName,
                              ULONG msec)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosFSAttach(PCSZ  pszDevice,
                             PCSZ  pszFilesystem,
                             PVOID pData,
                             ULONG cbData,
                             ULONG flag)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosQueryFSAttach(PCSZ     pszDeviceName,
                                  ULONG  ulOrdinal,
                                  ULONG  ulFSAInfoLevel,
                                  PFSQBUFFER2 pfsqb,
                                  PULONG pcbBuffLength)
{
  log("pszDeviceName=%s\n", pszDeviceName);
  log("ulOrdinal=%lu\n", ulOrdinal);
  log("ulFSAInfoLevel=%lu\n", ulFSAInfoLevel);
  log("pfsqb=%lx\n", pfsqb);
  log("pcbBuffLength=%lx\n", pcbBuffLength);
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosQueryFSInfo(ULONG disknum,
                                ULONG infolevel,
                                PVOID pBuf,
                                ULONG cbBuf)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosWaitChild(ULONG action,
                             ULONG option,
                             PRESULTCODES pres,
                             PPID ppid,
                             PID pid)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosDevIOCtl(HFILE hDevice, ULONG category, ULONG xfunction,
                            PVOID pParams, ULONG cbParmLenMax, PULONG pcbParmLen,
                            PVOID pData, ULONG cbDataLenMax, PULONG pcbDataLen)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosBeep(ULONG freq,
                        ULONG dur)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY DosPhysicalDisk(ULONG xfunction, PVOID pBuf, ULONG cbBuf, PVOID pParams, ULONG cbParams)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosSetCp(ULONG cp, ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosSetProcessCp(ULONG cp)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosStopTimer(ULONG tmr)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY   DosSetDateTime(PDATETIME pdt)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      Thk32AllocBlock(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      Thk32FreeBlock(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      THK32R3DS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosExitList(ULONG ordercode,
                            PFNEXITLIST pfn)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosAllocProtectedMem(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET  APIENTRY DosAliasMem(PVOID pb,
                             ULONG cb,
                             PPVOID ppbAlias,
                             ULONG fl)
{
  return unimplemented(__FUNCTION__);
}


APIRET  APIENTRY DosQueryMemState(PVOID pb,
                                  PULONG cb,
                                  PULONG pFlag)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosOpenVDD(PCSZ pszVDD, PHVDD phvdd)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      DosRequestVDD(HVDD  hvdd,
                                   SGID  sgid,
                                   ULONG cmd,
                                   ULONG cbInput,
                                   PVOID pInput,
                                   ULONG cbOutput,
                                   PVOID pOutput)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosCloseVDD(HVDD hvdd)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosDebug(PVOID pdbgbuf)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosResetEventSem(HEV hev, PULONG pulPostCt)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosPostEventSem(HEV hev)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosWaitEventSem(HEV hev, ULONG ulTimeout)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosQueryEventSem(HEV hev, PULONG pulPostCt)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosCreateMutexSem(PCSZ pszName, PHMTX phmtx, ULONG flAttr, BOOL32 fState)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosOpenMutexSem(PCSZ pszName, PHMTX phmtx)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosCloseMutexSem(HMTX hmtx)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosRequestMutexSem(HMTX hmtx, ULONG ulTimeout)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosReleaseMutexSem(HMTX hmtx)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosQueryMutexSem(HMTX hmtx, PPID ppid, PTID ptid, PULONG pulCount)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosCreateMuxWaitSem(PCSZ pszName, PHMUX phmux, ULONG cSemRec, PSEMRECORD pSemRec, ULONG flAttr)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosOpenMuxWaitSem(PCSZ pszName, PHMUX phmux)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosCloseMuxWaitSem(HMUX hmux)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosWaitMuxWaitSem(HMUX hmux, ULONG ulTimeout, PULONG pulUser)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosAddMuxWaitSem(HMUX hmux, PSEMRECORD pSemRec)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosDeleteMuxWaitSem(HMUX hmux, HSEM hSem)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosQueryMuxWaitSem(HMUX hmux, PULONG pcSemRec, PSEMRECORD pSemRec, PULONG pflAttr)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosSubSetMem(PVOID pbBase,
                              ULONG flag,
                              ULONG cb)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosSubAllocMem(PVOID pbBase,
                                PPVOID ppb,
                                ULONG cb)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosSubFreeMem(PVOID pbBase,
                               PVOID pb,
                               ULONG cb)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosSubUnsetMem(PVOID pbBase)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosAsyncTimer(ULONG msec, HSEM hsem, PHTIMER phtimer)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosStartTimer(ULONG msec, HSEM hsem, PHTIMER phtimer)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosGetResource(HMODULE hmod, ULONG idType, ULONG idName, PPVOID ppb)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosFreeResource(PVOID pb)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosQueryPageUsage(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosQueryModFromCS(SEL sel, QMRESULT *qmresult)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosQueryModFromEIP(HMODULE const *phMod, ULONG const *pObjNum,
                                   ULONG BuffLen, PCHAR pBuff,
                                   ULONG const *pOffset, ULONG Address)
{
  return unimplemented(__FUNCTION__);
}

ULONG DosFPDataArea;

APIRET APIENTRY      DosTmrQueryFreq(PULONG pulTmrFreq)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosTmrQueryTime(PQWORD pqwTmrTime)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosAliasPerfCtrs(ULONG p1, ULONG p2, PBYTE* p3, PULONG p4)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosConfigPerf(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosDeconPerf(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosRegisterPerfCtrs(PBYTE pbTextBlk, ULONG flFlags)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosQuerySysState(ULONG EntityList, ULONG EntityLevel,
                                      PID pid, TID tid, PVOID pDataBuf, ULONG cbBuf)
{
  return unimplemented(__FUNCTION__);
}


ULONG DosFlatCS;


ULONG DosFlatDS;


APIRET APIENTRY DosQueryABIOSSupport(ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosEnumAttribute(ULONG  ulRefType,
                                  PVOID  pvFile,
                                  ULONG  ulEntry,
                                  PVOID  pvBuf,
                                  ULONG  cbBuf,
                                  PULONG pulCount,
                                  ULONG  ulInfoLevel)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosQueryDOSProperty(SGID sgid, PCSZ pszName, ULONG cb, PCSZ pszBuf)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosSetDOSProperty(SGID sgid, PCSZ pszName, ULONG cb, PCSZ pszBuf)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosEnterMustComplete(PULONG pulNesting)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosExitMustComplete(PULONG pulNesting)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosQueryCtryInfo(ULONG cb, PCOUNTRYCODE pcc, PCOUNTRYINFO pci, PULONG pcbActual)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosDynamicTrace(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      StartLazyWriter(void)
{
  return unimplemented(__FUNCTION__);
}


ULONG QueInst_Data;


APIRET APIENTRY  DosShutdown(ULONG ulReserved)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosReplaceModule(const PSZ pszOldModule, const PSZ pszNewModule, const PSZ pszBackupModule)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosAcknowledgeSignalException(ULONG ulSignalNum)
{
  return unimplemented(__FUNCTION__);
}


ULONG DosTIB;


ULONG DosIODelayCnt;


// fix prototype !!! (undoc)
APIRET APIENTRY      LogOpen(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      LogClose(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      LogAddEntries(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      LogGetEntries(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      LogSetState(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      LogSetName(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      LogQueryState(void)
{
  return unimplemented(__FUNCTION__);
}


typedef PBYTE PCNPATH;


APIRET APIENTRY      DosOpenChangeNotify(PCNPATH PathBuf,
                                      ULONG LogSize,
                                      PHDIR hdir,
                                      ULONG ulReserved)
{
  return unimplemented(__FUNCTION__);
}


typedef PBYTE PCNINFO;


APIRET APIENTRY      DosResetChangeNotify(PCNINFO LogBuf,
                                       ULONG BufferSize,
                                       PULONG LogCount,
                                       HDIR hdir)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosCloseChangeNotify(HDIR hdir)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosForceSystemDump(ULONG Reserved)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosAllocThreadLocalMemory(ULONG cb, PULONG *p)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosFreeThreadLocalMemory(ULONG *p)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosVerifyPidTid(PID Pid, TID Tid)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32EXITLIST(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32ALLOCPROTECTEDMEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32ALIASMEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32ALLOCMEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32ALLOCSHAREDMEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32GETNAMEDSHAREDMEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32GETSHAREDMEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32GIVESHAREDMEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32FREEMEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32SETMEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32QUERYMEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32QUERYMEMSTATE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32OPENVDD(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32REQUESTVDD(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32CLOSEVDD(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32CREATETHREAD(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32DYNAMICTRACE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32DEBUG(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32QUERYPROCADDR(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32CREATEEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32OPENEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32CLOSEEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32RESETEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32POSTEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32WAITEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32QUERYEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32CREATEMUTEXSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32OPENMUTEXSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32CLOSEMUTEXSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32REQUESTMUTEXSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32RELEASEMUTEXSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32QUERYMUTEXSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32CREATEMUXWAITSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32OPENMUXWAITSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32CLOSEMUXWAITSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32WAITMUXWAITSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32ADDMUXWAITSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32DELETEMUXWAITSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32QUERYMUXWAITSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32QUERYSYSINFO(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32WAITTHREAD(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32GETRESOURCE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32FREERESOURCE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32EXCEPTIONCALLBACK(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32QUERYPAGEUSAGE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosExceptionCallBack(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      TI32ASYNCTIMER(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      TI32STARTTIMER(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosR3ExceptionDispatcher(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosLibiDisp(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosLibiDisp16(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosLibiDisp32(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosR3ExitAddr(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32IREAD(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32IWRITE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32TMRQUERYFREQ(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32TMRQUERYTIME(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32ALIASPERFCTRS(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32CONFIGUREPERF(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32DECONPERF(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32REGISTERPERFCTRS(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32QUERYSYSSTATE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32QUERYDOSPROPERTY(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32SETDOSPROPERTY(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32PROFILE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32QUERYRESOURCESIZE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosQueryResourceSize(HMODULE hmod, ULONG idt,
                                     ULONG idn, PULONG pulsize)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32SETSIGNALEXCEPTIONFOCUS(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32SENDSIGNALEXCEPTION(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32STARTTIMER(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32STOPTIMER(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32ASYNCTIMER(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32INITIALIZEPORTHOLE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosInitializePorthole(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32QUERYHEADERINFO(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosQueryHeaderInfo(HMODULE hmod, ULONG ulIndex, PVOID pvBuffer,
                                        ULONG cbBuffer, ULONG ulSubFunction)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32QUERYPROCTYPE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32IEXITMUSTCOMPLETE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32ICACHEMODULE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32DLLTERM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosDllTermDisp(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32IRAISEEXCEPTION(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32ACKNOWLEDGESIGNALEXC(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosPM16SemChk(void)
{
  return unimplemented(__FUNCTION__);
}


// LDT selector value
SEL DosLDTSel;
#pragma aux DosLDTSel "*"


// fix prototype !!! (undoc)
APIRET APIENTRY      DosR3Frestor(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32SETFILELOCKS(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32CANCELLOCKREQUEST(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32IMONREAD(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32IMONWRITE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      T32KILLTHREAD(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      TQUERYRASINFO(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosProtectEnumAttribute(ULONG ulRefType,
                                         PVOID pvFile,
                                         ULONG ulEntry,
                                         PVOID pvBuf,
                                         ULONG cbBuf,
                                         PULONG pulCount,
                                         ULONG ulInfoLevel,
                                         FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosProtectClose(HFILE hFile,
                                 FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosProtectRead(HFILE hFile,
                                PVOID pBuffer,
                                ULONG cbRead,
                                PULONG pcbActual,
                                FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosProtectWrite(HFILE hFile,
                                 ULONG pBuffer,
                                 ULONG cbWrite,
                                 PULONG pcbActual,
                                 FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosProtectSetFileInfo(HFILE hf,
                                       ULONG ulInfoLevel,
                                       PVOID pInfoBuf,
                                       ULONG cbInfoBuf,
                                       FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosProtectSetFHState(HFILE hFile,
                                      ULONG mode,
                                      FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosProtectQueryFHState(HFILE hFile,
                                        PULONG pMode,
                                        FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosProtectQueryFileInfo(HFILE hf,
                                         ULONG ulInfoLevel,
                                         PVOID pInfo,
                                         ULONG cbInfoBuf,
                                         FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosCloseMessageFile(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosSetExtLIBPATH(const PSZ pszExtLIBPATH, ULONG flags)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosQueryExtLIBPATH(const PSZ pszExtLIBPATH, ULONG flags)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosSysCtl(ULONG entry, PULONG data)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosQueryThreadContext(TID atid,
                                           ULONG level,
                                           PCONTEXTRECORD pcxt)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY      DosSGQueryTopmost(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosPerfSysCall(ULONG ulCommand, ULONG ulParm1, ULONG ulParm2, ULONG ulParm3)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosProtectOpenL(PCSZ  pszFileName,
                                 PHFILE phf,
                                 PULONG pulAction,
                                 LONGLONG cbFile,
                                 ULONG ulAttribute,
                                 ULONG fsOpenFlags,
                                 ULONG fsOpenMode,
                                 PEAOP2 peaop2,
                                 PFHLOCK pfhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosProtectSetFileLocksL(HFILE hFile,
                                        PFILELOCKL pflUnlock,
                                        PFILELOCKL pflLock,
                                        ULONG timeout, ULONG flags,
                                        FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosProtectSetFilePtrL(HFILE hFile,
                                       LONGLONG ib,
                                       ULONG method,
                                       PLONGLONG ibActual,
                                       FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY   DosProtectSetFileSizeL(HFILE hFile,
                                         LONGLONG cbSize,
                                         FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosSetFileLocksL(HFILE hFile,
                                 PFILELOCKL pflUnlock,
                                 PFILELOCKL pflLock,
                                 ULONG timeout,
                                 ULONG flags)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosCancelLockRequestL(HFILE hFile,
                                      PFILELOCKL pflLock)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosSystemService(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET  APIENTRY        DosCreateThread2(PTHREADCREATE ptc)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosExitCritSec(VOID)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         THK32FREESTACK(void)
{
  return unimplemented(__FUNCTION__);
}

typedef struct {
   ULONG sl_vaddr; /* start of VA segment to profile */
   ULONG sl_size;  /* length of VA segment */
   ULONG sl_mode;  /* !=0 use PRF_VA* flags, */
                   /* =0, simple count */
} PRFSLOT;

      #define PRF_PROCESS_MT 0      /* profile proc+threads */
      #define PRF_PROCESS_ST 1      /* profile proc only */
      #define PRF_KERNEL     2      /* profile kernel */
      #define PRF_VADETAIL   0      /* create detailed page counters */
      #define PRF_VAHIT      4      /* create hit table */
      #define PRF_VATOTAL    8      /* create total count for VA only */
      #define PRF_FLGBITS    0x40   /*  has a flgbits structure (?) */
      #define PRF_WRAP       0x80   /* don't use: if hit table full, wrap */
                                    /* there is a bug in kernel, which */
                                    /* prevents this from correct working! */
      /* status bits, don't ever set these (won't work, not masked, bug!) */
      #define PRFS_RUNNING   0x100  /* profiling is active */
      #define PRFS_THRDS     0x200  /* also profiling threads */
      #define PRFS_HITOVFL   0x800  /* overflow in hit buffer */
      #define PRFS_HEADER    0x1000 /* internally used */


typedef struct {
   PRFSLOT *cm_slots;              /* Virtual address slots */
   USHORT   cm_nslots;             /* # of VA slots < 256 (!) */
   USHORT   cm_flags;              /* command */
   ULONG    cm_bufsz;              /* reserve # of bytes for buffers */
                                    /* e.g. for hit buffer or detailed */
                                    /* counters */
   USHORT   cm_timval;             /* timer resolution */
                                    /* if 0, use default == 1000 */
   /* valid if PRF_FLAGBITS set */
   PUCHAR   cm_flgbits;            /* vector of flag bits (?) */
   UCHAR    cm_nflgs;              /* # of flag bits >= 2 if present */
} PRFCMD; /* 19 bytes */

      #define PRF_RET_GLOBAL   0 /* return global data */
                                 /* set us_thrdno for specific thread */
                                 /* us_buf = struct PRFRET0 */
      #define PRF_RET_VASLOTS  1 /* return VA slot data (PRFRET1) */
      #define PRF_RET_VAHITS   2 /* return hit table (PRFRET2) */
      #define PRF_RET_VADETAIL 3 /* return detailed counters (PRFRET3) */
                                 /* specify us_vaddr */
typedef struct {
   UCHAR us_cmd;                /* command */
   USHORT us_thrdno;            /* thread requested for cmd=0 */
   ULONG us_vaddr;              /* VA for cmd=3*/
   ULONG us_bufsz;              /* length of return buffer */
   VOID *us_buf;                /* return buffer */
} PRFRET; /* 15 bytes */

typedef struct {
   USHORT r0_flags;             /* profile flags */
                                 /* see PRF_* defines */
   USHORT r0_shift;             /* shift factor */
                                 /* 2^N = length of a segment for */
                                 /* detailed counters */
   ULONG  r0_idle;              /* count if process is idle */
   ULONG  r0_vm86;              /* count if process is in VM mode */
   ULONG  r0_kernel;            /* count if process is in kernel */
   ULONG  r0_shrmem;            /* count if process is in shr mem */
   ULONG  r0_unknown;           /* count if process is elsewhere */
   ULONG  r0_nhitbufs;          /* # of dwords in hitbufs */
   ULONG  r0_hitbufcnt;         /* # of entries in hit table */
   ULONG  r0_reserved1;         /* internally used */
   ULONG  r0_reserved2;         /* internally used */
   USHORT r0_timval;            /* timer resolution */
   UCHAR  r0_errcnt;            /* error count */
   USHORT r0_nstruc1;           /* # of add structures 1 (?) */
   USHORT r0_nstruc2;           /* # of add structures 2 (?) */
} PRFRET0;

typedef struct {
   ULONG va_vaddr;              /* virtual address of segment */
   ULONG va_size;               /* length of segment */
   ULONG va_flags;              /* == 8, va_cnt is valid */
   ULONG va_reserved;           /* internally used */
   ULONG va_cnt;                /* profile count */
} PRFVA;

typedef struct {
   UCHAR r1_nslots;             /* # of slots (bug: prevents */
                                 /* correct # if #slots >255) */
   PRFVA r1_slots[1];           /* slots */
} PRFRET1;

typedef struct {
   ULONG r2_nhits;              /* # of entries in table */
   ULONG r2_hits[1];            /* hit table */
} PRFRET2;

typedef struct {
   ULONG r3_size;               /* size of segment */
   ULONG r3_ncnts;              /* # of entries in table */
   ULONG r3_cnts[1];            /* counters */
} PRFRET3;

APIRET APIENTRY         DosProfile(ULONG func, PID pid, PRFCMD *profcmd, PRFRET *profret)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32GetProcessorStatus(ULONG procid, PULONG status)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32SetProcessorStatus(ULONG procid, ULONG status)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32TestPSD(PCSZ arg)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32R3ExitAddr(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32IRead(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32IWrite(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32IProtectWrite(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32IProtectSetFileInfo(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32IProtectSetFHState(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32IProtectQueryFHState(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32IProtectRead(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         DosLDRDirtyWorker(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32IGetInfoBlocks(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32IGETINFOBLOCKS(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32GETPROCESSORSTATUS(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32SETPROCESSORSTATUS(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32TESTPSD(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32QUERYTHREADAFFINITY(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32SETTHREADAFFINITY(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosReadQueue(HQUEUE hq, PREQUESTDATA pRequest, PULONG pcbData,
         PPVOID ppbuf, ULONG element, BOOL32 wait, PBYTE ppriority, HEV hsem)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosPurgeQueue(HQUEUE hq)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosCloseQueue(HQUEUE hq)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosQueryQueue(HQUEUE hq, PULONG pcbEntries)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosPeekQueue(HQUEUE hq, PREQUESTDATA pRequest, PULONG pcbData,
         PPVOID ppbuf, PULONG element, BOOL32 nowait, PBYTE ppriority, HEV hse)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosWriteQueue(HQUEUE hq, ULONG request, ULONG cbData,
         PVOID pbData, ULONG priority)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosOpenQueue(PPID ppid, PHQUEUE phq, PCSZ pszName)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosCreateQueue(PHQUEUE phq, ULONG priority, PCSZ pszName)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosStartSession(PSTARTDATA psd, PULONG pidSession, PPID ppid)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosSelectSession(ULONG idSession)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosSetSession(ULONG idSession, PSTATUSDATA psd)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosStopSession(ULONG scope, ULONG idSession)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosRegisterNotification(PVOID data)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         AnsiInject(void) // ???
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         AnsiKeydef(void) // ???
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         AnsiInterp(void) // ???
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         THK32ALIASMEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         THK32FREEALIAS(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         THK32ALLOCVARLEN(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         THK32HANDLEBOUNDARY(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         THK32HANDLESTRING(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         THK32DEALLOC(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         THK32XHNDLR(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32FORCESYSTEMDUMP(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32QUERYABIOSSUPPORT(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32QUERYMODFROMEIP(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32ACKNOWLEDGESIGNALEXCEPTION(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32DUMPPROCESS(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32SUPPRESSPOPUPS(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32IPROTECTWRITE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32PROTECTSETFILELOCKS(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32IPROTECTREAD(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32PMPOSTEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32PMWAITEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32PMREQUESTMUTEXSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32PMWAITMUXWAITSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32PM16SEMCHK(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32ALLOCTHREADLOCALMEMORY(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32FREETHREADLOCALMEMORY(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32SETEXTLIBPATH(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32QUERYEXTLIBPATH(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32PM16SEMRST(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32VERIFYPIDTID(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32SYSCTL(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32QUERYTHREADCONTEXT(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32PERFSYSCALL(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32LISTIO(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32PMR3WAITEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32PMR3POSTEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32CREATETHREAD2(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32PROTECTSETFILELOCKSL(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32IPROTECTSETFILEPTRL(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32SETFILELOCKSL(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32CANCELLOCKREQUESTL(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32ISETFILEPTRL(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32LISTIOL(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32ISETFILEPTR(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         T32IPROTECTSETFILEPTR(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32CreateRamSem(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY                Dos32CloseRamSem(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32OpenRamSem(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32RequestRamSem(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32ReleaseRamSem(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32QueryProcessInfo(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32QueryThreadInfo(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         OS2BaseInit(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         THK32ALLOCBLOCK(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         THK32FREEBLOCK(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         THK32STRLEN(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         THK32_UNITHUNK(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosCreateSpinLock(PHSPINLOCK pHandle)
{
  return unimplemented(__FUNCTION__);
}


VOID APIENTRY         DosAcquireSpinLock(HSPINLOCK Handle)
{
  unimplemented(__FUNCTION__);
}


VOID APIENTRY         DosReleaseSpinLock(HSPINLOCK Handle)
{
  unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosFreeSpinLock(HSPINLOCK Handle)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32PMR3WaitEventSem(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32PMR3PostEventSem(void)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY         DosQueryThreadAffinity(ULONG scope, PMPAFFINITY pAffinityMask)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosSetThreadAffinity(PMPAFFINITY pAffinityMask)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         DosPMSemWait(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         DosPMMuxSemWait(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         HT32_STARTUP(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         HT32_CLEANUP(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Mon32MonRead(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Mon32MonWrite(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32PMPostEventSem(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32PMWaitEventSem(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32PMRequestMutexSem(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
APIRET APIENTRY         Dos32PMWaitMuxWaitSem(void)
{
  return unimplemented(__FUNCTION__);
}
