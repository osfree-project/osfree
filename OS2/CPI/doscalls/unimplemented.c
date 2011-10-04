#include <strnlen.h>

#define INCL_DOSNMPIPES
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDATETIME
#define INCL_DOSEXCEPTIONS
#include <os2.h>

#include <stdio.h>
#include <stdarg.h>

APIRET __cdecl KalLogWrite(PSZ s);

void log(const char *fmt, ...)
{
  va_list arg_ptr;
  char buf[1024];

  va_start(arg_ptr, fmt);
  vsprintf(buf, fmt, arg_ptr);
  va_end(arg_ptr);

  KalLogWrite(buf);
}

APIRET unimplemented(char *func)
{
  log("%s is not yet implemented!\n", func);
  return 0;
}


//APIRET APIENTRY  DosForceDelete(PCSZ  pszFile)


APIRET APIENTRY DosKillThread(TID tid)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosQueryRASInfo(ULONG Index, PPVOID Addr)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosDumpProcess(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosSuppressPopUps(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         THK32ALLOCMEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         THK32FREEMEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         THK32ALLOCSTACK(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosOpen2Compt(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosQProcStatus(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosICallBack(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosRetForward(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIMakeNmPipe(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosICallNmPipe(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIConnectNmPipe(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIDisconnectNmPipe(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIPeekNmPipe(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIQNmPipeInfo(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIQNmPHandState(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosISetNmPHandState(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosITransactNmPipe(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIWaitNmPipe(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosISetNmPipeSem(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIQNmPipeSemState(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIRawReadNmPipe(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIRawWriteNmPipe(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosSetTraceInfo(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosLogMode(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosLogEntry(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosGetLogBuffer(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosLogRegister(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosLogRead(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosFindFromName(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosOPLockRelease(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosOPLockWait(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosICopy(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIQAppType(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosOPLockShutdown(void)
{
  return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY  DosSetMaxFH(ULONG cFH)


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


//APIRET APIENTRY  DosSetFHState(HFILE hFile,
//                               ULONG mode)


APIRET APIENTRY  DosSetFSInfo(ULONG disknum,
                              ULONG infolevel,
                              PVOID pBuf,
                              ULONG cbBuf)
{
  return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY  DosQueryPathInfo(PCSZ    pszPathName,
//                                  ULONG ulInfoLevel,
//                                  PVOID pInfoBuf,
//                                  ULONG cbInfoBuf)


//APIRET APIENTRY  DosQueryHType(HFILE hFile,
//                               PULONG pType,
//                               PULONG pAttr)


APIRET APIENTRY  DosQueryVerify(PBOOL32 pBool)

{
  return unimplemented(__FUNCTION__);
}

//APIRET APIENTRY  DosDeleteDir(PCSZ  pszDir)


//APIRET APIENTRY  DosScanEnv(PCSZ  pszName,
//                            PCSZ  *ppszValue)


//APIRET APIENTRY  DosSearchPath(ULONG flag,
//                               PCSZ  pszPathOrName,
//                               PCSZ  pszFilename,
//                               PBYTE pBuf,
//                               ULONG cbBuf)


//APIRET APIENTRY DosSleep(ULONG msec)


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


APIRET APIENTRY DosResumeThread(TID tid)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosSuspendThread(TID tid)
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


//APIRET APIENTRY  DosResetBuffer(HFILE hFile)


//APIRET APIENTRY  DosClose(HFILE hFile)


//APIRET APIENTRY  DosDelete(PCSZ  pszFile)


//APIRET APIENTRY  DosDupHandle(HFILE hFile,
//                              PHFILE pHfile)


//APIRET APIENTRY  DosFindClose(HDIR hDir)


//APIRET APIENTRY  DosFindFirst(PCSZ     pszFileSpec,
//                              PHDIR  phdir,
//                              ULONG  flAttribute,
//                              PVOID  pfindbuf,
//                              ULONG  cbBuf,
//                              PULONG pcFileNames,
//                              ULONG  ulInfoLevel)


//APIRET APIENTRY  DosFindNext(HDIR   hDir,
//                             PVOID  pfindbuf,
//                             ULONG  cbfindbuf,
//                             PULONG pcFilenames)


APIRET APIENTRY  DosFSAttach(PCSZ  pszDevice,
                             PCSZ  pszFilesystem,
                             PVOID pData,
                             ULONG cbData,
                             ULONG flag)
{
  return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY  DosCreateDir(PCSZ  pszDirName,
//                              PEAOP2 peaop2)


//APIRET APIENTRY  DosMove(PCSZ  pszOld,
//                         PCSZ  pszNew)


//APIRET APIENTRY  DosQueryFHState(HFILE hFile,
//                                 PULONG pMode)


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


//APIRET APIENTRY  DosQueryFileInfo(HFILE hf,
//                                  ULONG ulInfoLevel,
//                                  PVOID pInfo,
//                                  ULONG cbInfoBuf)


APIRET APIENTRY DosWaitChild(ULONG action,
                             ULONG option,
                             PRESULTCODES pres,
                             PPID ppid,
                             PID pid)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosDevIOCtl(HFILE hDevice, ULONG category, ULONG xfunction, PVOID pParams, ULONG cbParmLenMax, PULONG pcbParmLen, PVOID pData, ULONG cbDataLenMax, PULONG pcbDataLen)
{
  return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY  DosFSCtl(PVOID pData,
//                          ULONG cbData,
//                          PULONG pcbData,
//                          PVOID pParms,
//                          ULONG cbParms,
//                          PULONG pcbParms,
//                          ULONG function,
//                          PCSZ  pszRoute,
//                          HFILE hFile,
//                          ULONG method)


APIRET APIENTRY DosBeep(ULONG freq,
                        ULONG dur)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY DosPhysicalDisk(ULONG xfunction, PVOID pBuf, ULONG cbBuf, PVOID pParams, ULONG cbParams)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosSetCp(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosSetProcessCp(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosStopTimer(ULONG tmr)
{
  return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY      DosQueryCp(void)


APIRET APIENTRY   DosSetDateTime(PDATETIME pdt)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      Thk32AllocBlock(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      Thk32FreeBlock(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      Thk32R3DS(void)
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


//APIRET APIENTRY  DosAllocMem(PPVOID ppb,
//                             ULONG cb,
//                             ULONG flag)

//APIRET APIENTRY  DosAllocSharedMem(PPVOID ppb,
//                                   PCSZ  pszName,
//                                   ULONG cb,
//                                   ULONG flag)


APIRET APIENTRY  DosGetNamedSharedMem(PPVOID ppb,
                                      PCSZ  pszName,
                                      ULONG flag)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosGetSharedMem(PVOID pb,
                                 ULONG flag)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosGiveSharedMem(PVOID pb,
                                  PID pid,
                                  ULONG flag)
{
  return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY  DosFreeMem(PVOID pb)


//APIRET APIENTRY  DosSetMem(PVOID pb,
//                           ULONG cb,
//                           ULONG flag)


//APIRET APIENTRY  DosQueryMem(PVOID pb,
//                             PULONG pcb,
//                             PULONG pFlag)


APIRET  APIENTRY DosQueryMemState(PVOID pb,
                                  PULONG cb,
                                  PULONG pFlag)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosOpenVDD(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosRequestVDD(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosCloseVDD(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosCreateThread(PTID ptid,
                                PFNTHREAD pfn,
                                ULONG param,
                                ULONG flag,
                                ULONG cbStack)
{
  return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY DosGetInfoBlocks(PTIB *pptib,
//                                 PPIB *pppib)


APIRET APIENTRY      DosAllocProtSeg(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosAllocShrProtSeg(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosAllocProtHuge(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosDynamicTrace(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosDebug(PVOID pdbgbuf)
{
  return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY  DosLoadModule(PCSZ  pszName,
//                               ULONG cbName,
//                               PCSZ  pszModname,
//                               PHMODULE phmod);


//APIRET APIENTRY DosQueryModuleHandle(const PSZ pszModname, PHMODULE phmod);


//APIRET APIENTRY DosQueryModuleName(HMODULE hmod, ULONG cbName, PCHAR pch);


//APIRET APIENTRY DosQueryProcAddr(HMODULE hmod, ULONG ordinal, const PSZ pszName, PFN *  ppfn);


APIRET APIENTRY DosFreeModule(HMODULE hmod)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosQueryAppType(PCSZ pszName,
                                     PULONG pFlags)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosCreateEventSem(PCSZ pszName, PHEV phev, ULONG flAttr, BOOL32 fState)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosOpenEventSem(PCSZ pszName, PHEV phev)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosCloseEventSem(HEV hev)
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


APIRET APIENTRY DosWaitThread(PTID ptid,
                              ULONG option)
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


APIRET APIENTRY      DosSetExceptionHandler(PEXCEPTIONREGISTRATIONRECORD pERegRec)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosUnsetExceptionHandler(PEXCEPTIONREGISTRATIONRECORD pERegRec)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosRaiseException(PEXCEPTIONREPORTRECORD pexcept)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosUnwindException(PEXCEPTIONREGISTRATIONRECORD phandler,
                                        PVOID pTargetIP,
                                        PEXCEPTIONREPORTRECORD pERepRec)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosQueryPageUsage(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosQueryModFromCS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosQueryModFromEIP(HMODULE const *  phMod, ULONG const *  pObjNum, ULONG BuffLen, PCHAR pBuff, ULONG const *  pOffset, ULONG Address)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosFPDataArea(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosTmrQueryFreq(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosTmrQueryTime(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosAliasPerfCtrs(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosConfigPerf(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosDeconPerf(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosRegisterPerfCtrs(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosQuerySysState(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosFlatCS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosFlatDS(void)
{
  return unimplemented(__FUNCTION__);
}


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


APIRET APIENTRY      DosQueryDOSProperty(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosSetDOSProperty(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosSetSignalExceptionFocus(BOOL32 flag,
                                                PULONG pulTimes)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      DosSendSignalException(PID apid,
                                            ULONG exception)
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


//APIRET APIENTRY  DosSetRelMaxFH(PLONG pcbReqCount,
//                                PULONG pcbCurMaxFH)




//APIRET APIENTRY      DosIQueryMessageCP(void)


APIRET APIENTRY      DosCaseMap(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosGetCollate(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosGetCtryInfo(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosGetDBCSEv(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosQueryCtryInfo(void)
{
  return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY      DosQueryDBCSEnv(void)


APIRET APIENTRY      DosMapCase(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosQueryCollate(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosMakeNmPipe(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosQNmPipeInfo(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosConnectNmPipe(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosDisconnectNmPipe(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosQNmPHandState(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosSetNmPHandState(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosPeekNmPipe(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosWaitNmPipe(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosTransactNmPipe(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosCallNmPipe(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosRawReadNmPipe(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosRawWriteNmPipe(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosSetNmPipeSem(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosQNmPipeSemState(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      StartLazyWriter(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      _QueInst_Data(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY  DosShutdown(ULONG ulReserved)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosICacheModule(void)
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


APIRET APIENTRY      DosTIB(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIODelayCnt(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      LogOpen(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      LogClose(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      LogAddEntries(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      LogGetEntries(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      LogSetState(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      LogSetName(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      LogQueryState(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosOpenChangeNotify(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosResetChangeNotify(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosCloseChangeNotify(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosForceSystemDump(void)
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


APIRET APIENTRY      PTDA_LANMAN_SEC(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_PID(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      SAS_SEL(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      TCB_OPCOOKIE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      TCB_OPFLAGS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      TCB_NEWFLAGS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      TCB_USER_ID(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      TCB_PROC_ID(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      TCB_FSHARING(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      TCB_SRVATTRIB(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      TCB_ALLOWED(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      TCB_PRTCB(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      TCB_NUMBER(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      TCB_THISSFT(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      TCB_THISCDS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      TKOPTDA(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_CRITSEC(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_HOLDSIGCNT(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_PPTDAPARENT(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_PGDATA(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_HANDLE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_MODULE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_LDTHANDLE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_CODEPAGE_TAG(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_JFN_LENGTH(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_JFN_PTABLE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_JFN_FLG_PTR(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_EXTERR_LOCUS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_EXTERR(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_EXTERR_ACTION(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_EXTERR_CLASS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_PPID(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_PROCTYPE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_CURRTCB(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_CURRTSD(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      PTDA_SIGNATURE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32EXITLIST(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32ALLOCPROTECTEDMEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32ALIASMEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32ALLOCMEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32ALLOCSHAREDMEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32GETNAMEDSHAREDMEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32GETSHAREDMEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32GIVESHAREDMEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32FREEMEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32SETMEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32QUERYMEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32QUERYMEMSTATE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32OPENVDD(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32REQUESTVDD(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32CLOSEVDD(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32CREATETHREAD(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32DYNAMICTRACE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32DEBUG(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32QUERYPROCADDR(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32CREATEEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32OPENEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32CLOSEEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32RESETEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32POSTEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32WAITEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32QUERYEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32CREATEMUTEXSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32OPENMUTEXSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32CLOSEMUTEXSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32REQUESTMUTEXSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32RELEASEMUTEXSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32QUERYMUTEXSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32CREATEMUXWAITSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32OPENMUXWAITSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32CLOSEMUXWAITSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32WAITMUXWAITSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32ADDMUXWAITSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32DELETEMUXWAITSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32QUERYMUXWAITSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32QUERYSYSINFO(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32WAITTHREAD(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32GETRESOURCE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32FREERESOURCE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32EXCEPTIONCALLBACK(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32QUERYPAGEUSAGE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosExceptionCallBack(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      TI32ASYNCTIMER(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      TI32STARTTIMER(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosR3ExceptionDispatcher(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosLibiDisp(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosLibiDisp16(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosLibiDisp32(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosR3ExitAddr(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32IREAD(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32IWRITE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32TMRQUERYFREQ(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32TMRQUERYTIME(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32ALIASPERFCTRS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32CONFIGUREPERF(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32DECONPERF(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32REGISTERPERFCTRS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32QUERYSYSSTATE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosISetFileInfo(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosISetPathInfo(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32QUERYDOSPROPERTY(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32SETDOSPROPERTY(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIFindNext(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32PROFILE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32QUERYRESOURCESIZE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosQueryResourceSize(HMODULE hmod, ULONG idt, ULONG idn, PULONG pulsize)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32SETSIGNALEXCEPTIONFOCUS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32SENDSIGNALEXCEPTION(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32STARTTIMER(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32STOPTIMER(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32ASYNCTIMER(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32INITIALIZEPORTHOLE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosInitializePorthole(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32QUERYHEADERINFO(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosQueryHeaderInfo(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32QUERYPROCTYPE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DosQueryProcType(HMODULE hmod, ULONG ordinal, const PSZ pszName, PULONG pulproctype)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32IEXITMUSTCOMPLETE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosISigDispatch(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32ICACHEMODULE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32DLLTERM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosDllTermDisp(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32IRAISEEXCEPTION(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIRaiseException(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32ACKNOWLEDGESIGNALEXC(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosPM16SemChk(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIQueryFHState(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosISetFHState(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosLDTSel(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosR3Frestor(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIFindFirst(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32SETFILELOCKS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32CANCELLOCKREQUEST(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32IMONREAD(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32IMONWRITE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosMonOpen(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosMonClose(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosMonRead(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosMonWrite(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosMonReg(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      T32KILLTHREAD(void)
{
  return unimplemented(__FUNCTION__);
}


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
                                 PVOID pBuffer,
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


APIRET APIENTRY      DosSysCtl(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosQueryThreadContext(TID atid,
                                           ULONG level,
                                           PCONTEXTRECORD pcxt)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosSGQueryTopmost(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosPerfSysCall(void)
{
  return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY DosOpenL(PCSZ  pszFileName,
//                         PHFILE phf,
//                         PULONG pulAction,
//                         LONGLONG cbFile,
//                         ULONG ulAttribute,
//                         ULONG fsOpenFlags,
//                         ULONG fsOpenMode,
//                         PEAOP2 peaop2)


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


//APIRET APIENTRY  DosSetFilePtrL(HFILE hFile,
//                                LONGLONG ib,
//                                ULONG method,
//                                PLONGLONG ibActual)


//APIRET APIENTRY  DosSetFileSizeL(HFILE hFile,
//                                 LONGLONG cbSize)


APIRET APIENTRY      DosSystemService(void)
{
  return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY  DosListIOL(LONG CmdMODE,
//                            LONG NumEntries,
//                            PLISTIOL pListIO)

APIRET  APIENTRY        DosCreateThread2(PTHREADCREATE ptc)
{
  return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY DosEditName(ULONG metalevel,
//                            PCSZ pszSource,
//                            PSZ pszEdit,
//                            PSZ pszTarget,
//                            ULONG cbTarget)


APIRET APIENTRY DosExitCritSec(VOID)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY         THK32FREESTACK(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32Profile(void)
{
  return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY         Dos32TrueGetMessage(void)


APIRET APIENTRY         Dos32IQueryMessageCP(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32GetProcessorStatus(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32SetProcessorStatus(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32TestPSD(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32R3ExitAddr(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32IRead(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32IWrite(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32QueryResourceSize(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32QueryProcType(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32IProtectWrite(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32IProtectSetFileInfo(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32IProtectSetFHState(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32IProtectQueryFHState(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32IProtectRead(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosLDRDirtyWorker(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32IGetInfoBlocks(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32IGETINFOBLOCKS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32GETPROCESSORSTATUS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32SETPROCESSORSTATUS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32TESTPSD(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32QUERYTHREADAFFINITY(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32SETTHREADAFFINITY(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32ReadQueue(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32PurgeQueue(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32CloseQueue(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32QueryQueue(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32PeekQueue(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32WriteQueue(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32OpenQueue(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32CreateQueue(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32StartSession(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32SelectSession(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32SetSession(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32StopSession(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosRegisterNotification(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         AnsiInject(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         AnsiKeydef(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         AnsiInterp(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         THK32ALIASMEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         THK32FREEALIAS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         THK32ALLOCVARLEN(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         THK32HANDLEBOUNDARY(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         THK32HANDLESTRING(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         THK32DEALLOC(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         THK32XHNDLR(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32SetExtLIBPATH(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32QueryExtLIBPATH(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32QueryThreadContext(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32FORCESYSTEMDUMP(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32QUERYABIOSSUPPORT(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32QUERYMODFROMEIP(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32ACKNOWLEDGESIGNALEXCEPTION(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32DUMPPROCESS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32SUPPRESSPOPUPS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32IPROTECTWRITE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32PROTECTSETFILELOCKS(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32IPROTECTREAD(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32PMPOSTEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32PMWAITEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32PMREQUESTMUTEXSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32PMWAITMUXWAITSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32PM16SEMCHK(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32ALLOCTHREADLOCALMEMORY(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32FREETHREADLOCALMEMORY(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32SETEXTLIBPATH(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32QUERYEXTLIBPATH(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32PM16SEMRST(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32VERIFYPIDTID(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32SYSCTL(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32QUERYTHREADCONTEXT(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32PERFSYSCALL(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32LISTIO(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32PMR3WAITEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32PMR3POSTEVENTSEM(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32CREATETHREAD2(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32PROTECTSETFILELOCKSL(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32IPROTECTSETFILEPTRL(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32SETFILELOCKSL(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32CANCELLOCKREQUESTL(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32ISETFILEPTRL(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32LISTIOL(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32ISETFILEPTR(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         T32IPROTECTSETFILEPTR(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32CreateRamSem(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY                Dos32CloseRamSem(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32OpenRamSem(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32RequestRamSem(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32ReleaseRamSem(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32QueryProcessInfo(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32QueryThreadInfo(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32FindFromName(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosIProtectSetFileInfo(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosIProtectRead(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosIProtectWrite(void)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY         OS2BaseInit(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         QhKeybdHandle(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         QhMouseHandle(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         SMQueueRamSem(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         SMArray(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         SMPIDArray(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         SMInitialized(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         SMArraySize(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         THK32ALLOCBLOCK(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         THK32FREEBLOCK(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         THK32R3DS(void)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY         DosIOpenL(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosIProtectOpenL(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosISetFileSizeL(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosIProtectSetFileSizeL(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         THK32STRLEN(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         THK32_UNITHUNK(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32CreateSpinLock(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32AcquireSpinLock(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32ReleaseSpinLock(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32FreeSpinLock(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32PMR3WaitEventSem(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32PMR3PostEventSem(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32QueryThreadAffinity(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32SetThreadAffinity(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosPMSemWait(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         DosPMMuxSemWait(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         HT32_STARTUP(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         HT32_CLEANUP(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Mon32MonRead(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Mon32MonWrite(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32PMPostEventSem(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32PMWaitEventSem(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32PMRequestMutexSem(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY         Dos32PMWaitMuxWaitSem(void)
{
  return unimplemented(__FUNCTION__);
}


