#include <strnlen.h>
#define INCL_DOSNMPIPES
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDATETIME
#define INCL_DOSEXCEPTIONS
#include <os2.h>

APIRET APIENTRY      DosICreateThread(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIExecPgm(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }


APIRET APIENTRY      DosPtrace(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosISysSemClear(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY      DosISemRequest(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosISysSemSet(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY      DosISemWait(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosSGSwitch(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY      DosSGSwitchMe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIRead(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIWrite(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosQTraceInfo(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosICanonicalize(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosSetFgnd(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosSwapTaskInit(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosReadPhys(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosSGSwitchProc2(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosISetRelMaxFH(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIDevIOCtl(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosForceDelete(PCSZ  pszFile)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }


APIRET APIENTRY DosKillThread(TID tid)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosQueryRASInfo(ULONG Index, PPVOID Addr)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosDumpProcess(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosSuppressPopUps(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosOpen2Compt(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosGetSTDA(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosSGSwitchProc(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIRamSemWake(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIRamSemRequest(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosISetCP(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosGlobalSeg(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosProfile(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosQProcStatus(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosICallBack(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosRetForward(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIMakeNmPipe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosICallNmPipe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIConnectNmPipe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIDisconnectNmPipe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIPeekNmPipe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIQNmPipeInfo(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIQNmPHandState(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosISetNmPHandState(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosITransactNmPipe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIWaitNmPipe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosISetNmPipeSem(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIQNmPipeSemState(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIRawReadNmPipe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIRawWriteNmPipe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosSetTraceInfo(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosLogMode(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosLogEntry(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosGetLogBuffer(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosLogRegister(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosLogRead(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosFindFromName(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosOPLockRelease(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosOPLockWait(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosICopy(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIQAppType(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosOPLockShutdown(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosSetMaxFH(ULONG cFH)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosSetVerify(BOOL32 bFlag)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosErrClass(ULONG code,
                             PULONG pClass,
                             PULONG pAction,
                             PULONG pLocus)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }


APIRET APIENTRY      DosCreateVDM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosSetFileInfo(HFILE hf,
                                ULONG ulInfoLevel,
                                PVOID pInfoBuf,
                                ULONG cbInfoBuf)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosSetPathInfo(PCSZ    pszPathName,
                                ULONG ulInfoLevel,
                                PVOID pInfoBuf,
                                ULONG cbInfoBuf,
                                ULONG flOptions)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosSetFHState(HFILE hFile,
                               ULONG mode)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosSetFSInfo(ULONG disknum,
                              ULONG infolevel,
                              PVOID pBuf,
                              ULONG cbBuf)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosQueryPathInfo(PCSZ    pszPathName,
                                  ULONG ulInfoLevel,
                                  PVOID pInfoBuf,
                                  ULONG cbInfoBuf)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosQueryHType(HFILE hFile,
                               PULONG pType,
                               PULONG pAttr)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosQueryVerify(PBOOL32 pBool)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosDeleteDir(PCSZ  pszDir)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosScanEnv(PCSZ  pszName,
                            PCSZ  *ppszValue)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosSearchPath(ULONG flag,
                               PCSZ  pszPathOrName,
                               PCSZ  pszFilename,
                               PBYTE pBuf,
                               ULONG cbBuf)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosSleep(ULONG msec)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY   DosGetDateTime(PDATETIME pdt)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosDevConfig(PSZ pdevinfo, ULONG item)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosEnterCritSec(VOID)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }


APIRET APIENTRY DosKillProcess(ULONG action,
                               PID pid)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosSetPriority(ULONG scope,
                                ULONG ulClass,
                                LONG  delta,
                                ULONG PorTid)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosResumeThread(TID tid)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosSuspendThread(TID tid)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosCreatePipe(PHFILE phfRead,
                               PHFILE phfWrite,
                               ULONG cb)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosCallNPipe(PCSZ  pszName,
                              PVOID pInbuf,
                              ULONG cbIn,
                              PVOID pOutbuf,
                              ULONG cbOut,
                              PULONG pcbActual,
                              ULONG msec)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosConnectNPipe(HPIPE hpipe)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosDisConnectNPipe(HPIPE hpipe)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosCreateNPipe(PCSZ  pszName,
                                PHPIPE pHpipe,
                                ULONG openmode,
                                ULONG pipemode,
                                ULONG cbInbuf,
                                ULONG cbOutbuf,
                                ULONG msec)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosPeekNPipe(HPIPE hpipe,
                              PVOID pBuf,
                              ULONG cbBuf,
                              PULONG pcbActual,
                              PAVAILDATA pAvail,
                              PULONG pState)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosQueryNPHState(HPIPE hpipe,
                                  PULONG pState)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET  APIENTRY  DosRawReadNPipe(HPIPE hPipe,
                                  PVOID pBuffer,
                                  ULONG cbRead,
                                  PULONG pcbActual)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET  APIENTRY  DosRawWriteNPipe(HPIPE hPipe,
                                   PVOID pBuffer,
                                   ULONG cbWrite,
                                   PULONG pcbActual)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosQueryNPipeInfo(HPIPE hpipe,
                                   ULONG infolevel,
                                   PVOID pBuf,
                                   ULONG cbBuf)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosQueryNPipeSemState(HSEM hsem,
                                       PPIPESEMSTATE pnpss,
                                       ULONG cbBuf)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosSetNPHState(HPIPE hpipe,
                                ULONG state)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosSetNPipeSem(HPIPE hpipe,
                                HSEM hsem,
                                ULONG key)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosTransactNPipe(HPIPE hpipe,
                                  PVOID pOutbuf,
                                  ULONG cbOut,
                                  PVOID pInbuf,
                                  ULONG cbIn,
                                  PULONG pcbRead)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosWaitNPipe(PCSZ  pszName,
                              ULONG msec)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosResetBuffer(HFILE hFile)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }



APIRET APIENTRY  DosClose(HFILE hFile)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosDelete(PCSZ  pszFile)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosDupHandle(HFILE hFile,
                              PHFILE pHfile)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosFindClose(HDIR hDir)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosFindFirst(PCSZ     pszFileSpec,
                              PHDIR  phdir,
                              ULONG  flAttribute,
                              PVOID  pfindbuf,
                              ULONG  cbBuf,
                              PULONG pcFileNames,
                              ULONG  ulInfoLevel)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosFindNext(HDIR   hDir,
                             PVOID  pfindbuf,
                             ULONG  cbfindbuf,
                             PULONG pcFilenames)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosFSAttach(PCSZ  pszDevice,
                             PCSZ  pszFilesystem,
                             PVOID pData,
                             ULONG cbData,
                             ULONG flag)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosCreateDir(PCSZ  pszDirName,
                              PEAOP2 peaop2)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosMove(PCSZ  pszOld,
                         PCSZ  pszNew)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }





APIRET APIENTRY  DosQueryFHState(HFILE hFile,
                                 PULONG pMode)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosQueryFSAttach(PCSZ     pszDeviceName,
                                  ULONG  ulOrdinal,
                                  ULONG  ulFSAInfoLevel,
                                  PFSQBUFFER2 pfsqb,
                                  PULONG pcbBuffLength)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosQueryFSInfo(ULONG disknum,
                                ULONG infolevel,
                                PVOID pBuf,
                                ULONG cbBuf)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosQueryFileInfo(HFILE hf,
                                  ULONG ulInfoLevel,
                                  PVOID pInfo,
                                  ULONG cbInfoBuf)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosWaitChild(ULONG action,
                             ULONG option,
                             PRESULTCODES pres,
                             PPID ppid,
                             PID pid)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosDevIOCtl(HFILE hDevice, ULONG category, ULONG xfunction, PVOID pParams, ULONG cbParmLenMax, PULONG pcbParmLen, PVOID pData, ULONG cbDataLenMax, PULONG pcbDataLen)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY  DosFSCtl(PVOID pData,
                          ULONG cbData,
                          PULONG pcbData,
                          PVOID pParms,
                          ULONG cbParms,
                          PULONG pcbParms,
                          ULONG function,
                          PCSZ  pszRoute,
                          HFILE hFile,
                          ULONG method)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosBeep(ULONG freq,
                        ULONG dur)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosPhysicalDisk(ULONG xfunction, PVOID pBuf, ULONG cbBuf, PVOID pParams, ULONG cbParams)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY      DosSetCp(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosSetProcessCp(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosStopTimer(ULONG tmr)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosQueryCp(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY   DosSetDateTime(PDATETIME pdt)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      Thk32AllocBlock(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      Thk32FreeBlock(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      Thk32R3DS(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosExitList(ULONG ordercode,
                            PFNEXITLIST pfn)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosAllocProtectedMem(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET  APIENTRY DosAliasMem(PVOID pb,
                             ULONG cb,
                             PPVOID ppbAlias,
                             ULONG fl)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosAllocMem(PPVOID ppb,
                             ULONG cb,
                             ULONG flag)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosAllocSharedMem(PPVOID ppb,
                                   PCSZ  pszName,
                                   ULONG cb,
                                   ULONG flag)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosGetNamedSharedMem(PPVOID ppb,
                                      PCSZ  pszName,
                                      ULONG flag)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosGetSharedMem(PVOID pb,
                                 ULONG flag)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosGiveSharedMem(PVOID pb,
                                  PID pid,
                                  ULONG flag)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosFreeMem(PVOID pb)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosSetMem(PVOID pb,
                           ULONG cb,
                           ULONG flag)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosQueryMem(PVOID pb,
                             PULONG pcb,
                             PULONG pFlag)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET  APIENTRY DosQueryMemState(PVOID pb,
                                  PULONG cb,
                                  PULONG pFlag)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosOpenVDD(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosRequestVDD(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosCloseVDD(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosCreateThread(PTID ptid,
                                PFNTHREAD pfn,
                                ULONG param,
                                ULONG flag,
                                ULONG cbStack)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosGetInfoBlocks(PTIB *pptib,
                                 PPIB *pppib)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosAllocProtSeg(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosAllocShrProtSeg(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosAllocProtHuge(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosDynamicTrace(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosDebug(PVOID pdbgbuf)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosLoadModule(PCSZ  pszName,
                               ULONG cbName,
                               PCSZ  pszModname,
                               PHMODULE phmod)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosQueryModuleHandle(const PSZ pszModname, PHMODULE phmod)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosQueryModuleName(HMODULE hmod, ULONG cbName, PCHAR pch)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosQueryProcAddr(HMODULE hmod, ULONG ordinal, const PSZ pszName, PFN *  ppfn)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosFreeModule(HMODULE hmod)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY      DosQueryAppType(PCSZ pszName,
                                     PULONG pFlags)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosCreateEventSem(PCSZ pszName, PHEV phev, ULONG flAttr, BOOL32 fState)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosOpenEventSem(PCSZ pszName, PHEV phev)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosCloseEventSem(HEV hev)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosResetEventSem(HEV hev, PULONG pulPostCt)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosPostEventSem(HEV hev)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosWaitEventSem(HEV hev, ULONG ulTimeout)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosQueryEventSem(HEV hev, PULONG pulPostCt)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosCreateMutexSem(PCSZ pszName, PHMTX phmtx, ULONG flAttr, BOOL32 fState)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosOpenMutexSem(PCSZ pszName, PHMTX phmtx)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosCloseMutexSem(HMTX hmtx)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosRequestMutexSem(HMTX hmtx, ULONG ulTimeout)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosReleaseMutexSem(HMTX hmtx)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosQueryMutexSem(HMTX hmtx, PPID ppid, PTID ptid, PULONG pulCount)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosCreateMuxWaitSem(PCSZ pszName, PHMUX phmux, ULONG cSemRec, PSEMRECORD pSemRec, ULONG flAttr)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosOpenMuxWaitSem(PCSZ pszName, PHMUX phmux)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosCloseMuxWaitSem(HMUX hmux)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosWaitMuxWaitSem(HMUX hmux, ULONG ulTimeout, PULONG pulUser)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosAddMuxWaitSem(HMUX hmux, PSEMRECORD pSemRec)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosDeleteMuxWaitSem(HMUX hmux, HSEM hSem)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosQueryMuxWaitSem(HMUX hmux, PULONG pcSemRec, PSEMRECORD pSemRec, PULONG pflAttr)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY  DosSubSetMem(PVOID pbBase,
                              ULONG flag,
                              ULONG cb)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY  DosSubAllocMem(PVOID pbBase,
                                PPVOID ppb,
                                ULONG cb)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY  DosSubFreeMem(PVOID pbBase,
                               PVOID pb,
                               ULONG cb)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY  DosSubUnsetMem(PVOID pbBase)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}


APIRET APIENTRY DosWaitThread(PTID ptid,
                              ULONG option)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY      DosAsyncTimer(ULONG msec, HSEM hsem, PHTIMER phtimer)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY      DosStartTimer(ULONG msec, HSEM hsem, PHTIMER phtimer)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosGetResource(HMODULE hmod, ULONG idType, ULONG idName, PPVOID ppb)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosFreeResource(PVOID pb)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY      DosSetExceptionHandler(PEXCEPTIONREGISTRATIONRECORD pERegRec)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY      DosUnsetExceptionHandler(PEXCEPTIONREGISTRATIONRECORD pERegRec)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosRaiseException(PEXCEPTIONREPORTRECORD pexcept)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosUnwindException(PEXCEPTIONREGISTRATIONRECORD phandler,
                                        PVOID pTargetIP,
                                        PEXCEPTIONREPORTRECORD pERepRec)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosQueryPageUsage(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosQueryModFromCS(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosQueryModFromEIP(HMODULE const *  phMod, ULONG const *  pObjNum, ULONG BuffLen, PCHAR pBuff, ULONG const *  pOffset, ULONG Address)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY      DosFPDataArea(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosTmrQueryFreq(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosTmrQueryTime(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosAliasPerfCtrs(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosConfigPerf(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosDeconPerf(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosRegisterPerfCtrs(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosQuerySysState(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosFlatCS(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosFlatDS(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosQueryABIOSSupport(ULONG reserved)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY  DosEnumAttribute(ULONG  ulRefType,
                                  PVOID  pvFile,
                                  ULONG  ulEntry,
                                  PVOID  pvBuf,
                                  ULONG  cbBuf,
                                  PULONG pulCount,
                                  ULONG  ulInfoLevel)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosQueryDOSProperty(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosSetDOSProperty(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }


APIRET APIENTRY      DosSetSignalExceptionFocus(BOOL32 flag,
                                                PULONG pulTimes)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosSendSignalException(PID apid,
                                            ULONG exception)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosEnterMustComplete(PULONG pulNesting)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosExitMustComplete(PULONG pulNesting)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosSetRelMaxFH(PLONG pcbReqCount,
                                PULONG pcbCurMaxFH)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }





APIRET APIENTRY      DosIQueryMessageCP(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosCaseMap(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosGetCollate(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosGetCtryInfo(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosGetDBCSEv(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosQueryCtryInfo(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosQueryDBCSEnv(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosMapCase(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosQueryCollate(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosMakeNmPipe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosQNmPipeInfo(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosConnectNmPipe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosDisconnectNmPipe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosQNmPHandState(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosSetNmPHandState(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosPeekNmPipe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosWaitNmPipe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosTransactNmPipe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosCallNmPipe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosRawReadNmPipe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosRawWriteNmPipe(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosSetNmPipeSem(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosQNmPipeSemState(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      StartLazyWriter(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      _QueInst_Data(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosShutdown(ULONG ulReserved)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosICacheModule(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosReplaceModule(const PSZ pszOldModule, const PSZ pszNewModule, const PSZ pszBackupModule)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY      DosAcknowledgeSignalException(ULONG ulSignalNum)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosTIB(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }


APIRET APIENTRY      DosIODelayCnt(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }


APIRET APIENTRY      LogOpen(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      LogClose(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      LogAddEntries(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      LogGetEntries(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      LogSetState(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      LogSetName(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      LogQueryState(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosOpenChangeNotify(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosResetChangeNotify(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosCloseChangeNotify(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosForceSystemDump(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosAllocThreadLocalMemory(ULONG cb, PULONG *p)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosFreeThreadLocalMemory(ULONG *p)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosVerifyPidTid(PID Pid, TID Tid)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY      PTDA_LANMAN_SEC(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_PID(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      SAS_SEL(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      TCB_OPCOOKIE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      TCB_OPFLAGS(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      TCB_NEWFLAGS(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      TCB_USER_ID(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      TCB_PROC_ID(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      TCB_FSHARING(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      TCB_SRVATTRIB(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      TCB_ALLOWED(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      TCB_PRTCB(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      TCB_NUMBER(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      TCB_THISSFT(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      TCB_THISCDS(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      TKOPTDA(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_CRITSEC(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_HOLDSIGCNT(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_PPTDAPARENT(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_PGDATA(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_HANDLE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_MODULE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_LDTHANDLE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_CODEPAGE_TAG(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_JFN_LENGTH(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_JFN_PTABLE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_JFN_FLG_PTR(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_EXTERR_LOCUS(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_EXTERR(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_EXTERR_ACTION(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_EXTERR_CLASS(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_PPID(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_PROCTYPE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY      PTDA_CURRTCB(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_CURRTSD(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      PTDA_SIGNATURE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32EXITLIST(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32ALLOCPROTECTEDMEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32ALIASMEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32ALLOCMEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32ALLOCSHAREDMEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32GETNAMEDSHAREDMEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32GETSHAREDMEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32GIVESHAREDMEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32FREEMEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32SETMEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32QUERYMEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32QUERYMEMSTATE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32OPENVDD(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32REQUESTVDD(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32CLOSEVDD(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32CREATETHREAD(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32DYNAMICTRACE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32DEBUG(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32QUERYPROCADDR(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32CREATEEVENTSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32OPENEVENTSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32CLOSEEVENTSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32RESETEVENTSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32POSTEVENTSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32WAITEVENTSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32QUERYEVENTSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32CREATEMUTEXSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32OPENMUTEXSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32CLOSEMUTEXSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32REQUESTMUTEXSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32RELEASEMUTEXSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32QUERYMUTEXSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32CREATEMUXWAITSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32OPENMUXWAITSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32CLOSEMUXWAITSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32WAITMUXWAITSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32ADDMUXWAITSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32DELETEMUXWAITSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32QUERYMUXWAITSEM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32QUERYSYSINFO(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32WAITTHREAD(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32GETRESOURCE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32FREERESOURCE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32EXCEPTIONCALLBACK(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32QUERYPAGEUSAGE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosExceptionCallBack(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      TI32ASYNCTIMER(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      TI32STARTTIMER(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosR3ExceptionDispatcher(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosLibiDisp(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosLibiDisp16(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosLibiDisp32(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosR3ExitAddr(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }


APIRET APIENTRY      T32IREAD(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32IWRITE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32TMRQUERYFREQ(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32TMRQUERYTIME(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32ALIASPERFCTRS(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32CONFIGUREPERF(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32DECONPERF(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32REGISTERPERFCTRS(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32QUERYSYSSTATE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosISetFileInfo(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosISetPathInfo(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32QUERYDOSPROPERTY(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32SETDOSPROPERTY(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIFindNext(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32PROFILE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32QUERYRESOURCESIZE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosQueryResourceSize(HMODULE hmod, ULONG idt, ULONG idn, PULONG pulsize)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY      T32SETSIGNALEXCEPTIONFOCUS(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32SENDSIGNALEXCEPTION(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32STARTTIMER(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32STOPTIMER(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32ASYNCTIMER(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32INITIALIZEPORTHOLE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosInitializePorthole(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32QUERYHEADERINFO(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosQueryHeaderInfo(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32QUERYPROCTYPE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosQueryProcType(HMODULE hmod, ULONG ordinal, const PSZ pszName, PULONG pulproctype)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY      T32IEXITMUSTCOMPLETE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosISigDispatch(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32ICACHEMODULE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32DLLTERM(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosDllTermDisp(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32IRAISEEXCEPTION(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIRaiseException(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32ACKNOWLEDGESIGNALEXC(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosPM16SemChk(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIQueryFHState(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosISetFHState(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosLDTSel(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosR3Frestor(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosIFindFirst(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32SETFILELOCKS(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32CANCELLOCKREQUEST(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32IMONREAD(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32IMONWRITE(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosMonOpen(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosMonClose(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosMonRead(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosMonWrite(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosMonReg(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      T32KILLTHREAD(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      TQUERYRASINFO(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }


APIRET APIENTRY  DosProtectEnumAttribute(ULONG ulRefType,
                                         PVOID pvFile,
                                         ULONG ulEntry,
                                         PVOID pvBuf,
                                         ULONG cbBuf,
                                         PULONG pulCount,
                                         ULONG ulInfoLevel,
                                         FHLOCK fhFileHandleLockID)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }


APIRET APIENTRY  DosProtectClose(HFILE hFile,
                                 FHLOCK fhFileHandleLockID)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }


APIRET APIENTRY  DosProtectRead(HFILE hFile,
                                PVOID pBuffer,
                                ULONG cbRead,
                                PULONG pcbActual,
                                FHLOCK fhFileHandleLockID)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosProtectWrite(HFILE hFile,
                                 PVOID pBuffer,
                                 ULONG cbWrite,
                                 PULONG pcbActual,
                                 FHLOCK fhFileHandleLockID)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosProtectSetFileInfo(HFILE hf,
                                       ULONG ulInfoLevel,
                                       PVOID pInfoBuf,
                                       ULONG cbInfoBuf,
                                       FHLOCK fhFileHandleLockID)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosProtectSetFHState(HFILE hFile,
                                      ULONG mode,
                                      FHLOCK fhFileHandleLockID)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosProtectQueryFHState(HFILE hFile,
                                        PULONG pMode,
                                        FHLOCK fhFileHandleLockID)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosProtectQueryFileInfo(HFILE hf,
                                         ULONG ulInfoLevel,
                                         PVOID pInfo,
                                         ULONG cbInfoBuf,
                                         FHLOCK fhFileHandleLockID)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosCloseMessageFile(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosSetExtLIBPATH(const PSZ pszExtLIBPATH, ULONG flags)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}

APIRET APIENTRY DosQueryExtLIBPATH(const PSZ pszExtLIBPATH, ULONG flags)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosSysCtl(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosQueryThreadContext(TID atid,
                                           ULONG level,
                                           PCONTEXTRECORD pcxt)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosSGQueryTopmost(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY      DosPerfSysCall(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosOpenL(PCSZ  pszFileName,
                         PHFILE phf,
                         PULONG pulAction,
                         LONGLONG cbFile,
                         ULONG ulAttribute,
                         ULONG fsOpenFlags,
                         ULONG fsOpenMode,
                         PEAOP2 peaop2)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

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
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosProtectSetFileLocksL(HFILE hFile,
                                        PFILELOCKL pflUnlock,
                                        PFILELOCKL pflLock,
                                        ULONG timeout, ULONG flags,
                                        FHLOCK fhFileHandleLockID)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosProtectSetFilePtrL(HFILE hFile,
                                       LONGLONG ib,
                                       ULONG method,
                                       PLONGLONG ibActual,
                                       FHLOCK fhFileHandleLockID)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY   DosProtectSetFileSizeL(HFILE hFile,
                                         LONGLONG cbSize,
                                         FHLOCK fhFileHandleLockID)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosSetFileLocksL(HFILE hFile,
                                 PFILELOCKL pflUnlock,
                                 PFILELOCKL pflLock,
                                 ULONG timeout,
                                 ULONG flags)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosCancelLockRequestL(HFILE hFile,
                                      PFILELOCKL pflLock)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosSetFilePtrL(HFILE hFile,
                                LONGLONG ib,
                                ULONG method,
                                PLONGLONG ibActual)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosSetFileSizeL(HFILE hFile,
                                 LONGLONG cbSize)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }


APIRET APIENTRY      DosSystemService(void)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY  DosListIOL(LONG CmdMODE,
                            LONG NumEntries,
                            PLISTIOL pListIO)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}


APIRET  APIENTRY        DosCreateThread2(PTHREADCREATE ptc)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosEditName(ULONG metalevel,
                            PCSZ pszSource,
                            PSZ pszEdit,
                            PSZ pszTarget,
                            ULONG cbTarget)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0; }

APIRET APIENTRY DosExitCritSec(VOID)
{
  ULONG ulActual;
  PCSZ pcszMsg =__FUNCTION__" not implemented yet\n";
  DosWrite(1, pcszMsg, strnlen(pcszMsg, 250), &ulActual);
  return 0;
}
