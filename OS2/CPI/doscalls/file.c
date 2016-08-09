#include "kal.h"

#if 0
// Implementation of kernel-independent functions via kernel-dependent functions

APIRET APIENTRY DosCancelLockRequest(HFILE,PFILELOCK);
APIRET APIENTRY DosCreateDir(PCSZ,PEAOP2);
APIRET APIENTRY DosDelete(PCSZ);
APIRET APIENTRY DosDeleteDir(PCSZ);
APIRET APIENTRY DosDupHandle(HFILE,PHFILE);
APIRET APIENTRY DosEnumAttribute(ULONG,PVOID,ULONG,PVOID,ULONG,PULONG,ULONG);
APIRET APIENTRY DosForceDelete(PCSZ);
APIRET APIENTRY DosFSAttach(PCSZ,PCSZ,PVOID,ULONG,ULONG);
APIRET APIENTRY DosMove(PCSZ,PCSZ);
APIRET APIENTRY DosProtectClose(HFILE,FHLOCK);
APIRET APIENTRY DosProtectEnumAttribute(ULONG,PVOID,ULONG,PVOID,ULONG,PULONG,ULONG,FHLOCK);
APIRET APIENTRY DosProtectOpen(PCSZ,PHFILE,PULONG,ULONG,ULONG,ULONG,ULONG,PEAOP2,PFHLOCK);
APIRET APIENTRY DosProtectQueryFHState(HFILE,PULONG,FHLOCK);
APIRET APIENTRY DosProtectQueryFileInfo(HFILE,ULONG,PVOID,ULONG,FHLOCK);
APIRET APIENTRY DosProtectRead(HFILE,PVOID,ULONG,PULONG,FHLOCK);
APIRET APIENTRY DosProtectSetFHState(HFILE,ULONG,FHLOCK);
APIRET APIENTRY DosProtectSetFileInfo(HFILE,ULONG,PVOID,ULONG,FHLOCK);
APIRET APIENTRY DosProtectSetFileLocks(HFILE,PFILELOCK,PFILELOCK,ULONG,ULONG,FHLOCK);
APIRET APIENTRY DosProtectSetFilePtr(HFILE,LONG,ULONG,PULONG,FHLOCK);
APIRET APIENTRY DosProtectSetFileSize(HFILE,ULONG,FHLOCK);
APIRET APIENTRY DosProtectWrite(HFILE,ULONG,ULONG,PULONG,FHLOCK);
APIRET APIENTRY DosQueryFHState(HFILE,PULONG);
APIRET APIENTRY DosQueryFileInfo(HFILE,ULONG,PVOID,ULONG);
APIRET APIENTRY DosQueryFSAttach(PCSZ,ULONG,ULONG,PFSQBUFFER2,PULONG);
APIRET APIENTRY DosQueryFSInfo(ULONG,ULONG,PVOID,ULONG);
APIRET APIENTRY DosQueryHType(HFILE,PULONG,PULONG);
APIRET APIENTRY DosQueryPathInfo(PCSZ,ULONG,PVOID,ULONG);
APIRET APIENTRY DosQueryVerify(BOOL32*);
APIRET APIENTRY DosResetBuffer(HFILE);
APIRET APIENTRY DosSetFHState(HFILE,ULONG);
APIRET APIENTRY DosSetFileInfo(HFILE,ULONG,PVOID,ULONG);
APIRET APIENTRY DosSetFileLocks(HFILE,PFILELOCK,PFILELOCK,ULONG,ULONG);
APIRET APIENTRY DosSetFilePtr(HFILE,LONG,ULONG,PULONG);
APIRET APIENTRY DosSetFileSize(HFILE,ULONG);
APIRET APIENTRY DosSetFSInfo(ULONG,ULONG,PVOID,ULONG);
APIRET APIENTRY DosSetMaxFH(ULONG);
APIRET APIENTRY DosSetPathInfo(PCSZ,ULONG,PVOID,ULONG,ULONG);
APIRET APIENTRY DosSetRelMaxFH(PLONG,PULONG);
APIRET APIENTRY DosSetVerify(BOOL32);
APIRET APIENTRY DosShutdown(ULONG);
#endif

APIRET APIENTRY  DosClose(HFILE hFile)
{
  log("%s\n", __FUNCTION__);
  log("hfile=%lx\n", hFile);
  return KalClose (hFile);
}

APIRET APIENTRY  DosQueryHType(HFILE hFile,
                               PULONG pType,
                               PULONG pAttr)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("hfile=%lx\n", hFile);
  rc = KalQueryHType(hFile, pType, pAttr);
  log("type=%lx\n", *pType);
  log("attr=%lx\n", *pAttr);
  return rc;
}

APIRET APIENTRY DosResetBuffer(HFILE hFile)
{
  log("%s\n", __FUNCTION__);
  log("hfile=%lx\n", hFile);
  return KalResetBuffer(hFile);
}

APIRET APIENTRY  DosDupHandle(HFILE hFile,
                              PHFILE pHfile)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("hfile=%lx\n", hFile);
  rc = KalDupHandle(hFile, pHfile);
  log("hfile2=%lx\n", *pHfile);
  return rc;
}

APIRET APIENTRY  DosDelete(PCSZ  pszFile)
{
  log("%s\n", __FUNCTION__);
  log("pszFile=%s\n", pszFile);
  return KalDelete((PSZ)pszFile);
}

APIRET APIENTRY  DosForceDelete(PCSZ  pszFile)
{
  log("%s\n", __FUNCTION__);
  log("pszFile=%s\n", pszFile);
  return KalForceDelete((PSZ)pszFile);
}

APIRET APIENTRY  DosDeleteDir(PCSZ  pszDir)
{
  log("%s\n", __FUNCTION__);
  log("pszDir=%s\n", pszDir);
  return KalDeleteDir((PSZ)pszDir);
}

APIRET APIENTRY  DosCreateDir(PCSZ  pszDirName,
                              PEAOP2 peaop2)
{
  log("%s\n", __FUNCTION__);
  log("pszDirName=%s\n", pszDirName);
  return KalCreateDir((PSZ)pszDirName, peaop2);
}

APIRET APIENTRY  DosSetRelMaxFH(PLONG pcbReqCount,
                                PULONG pcbCurMaxFH)
{
  log("%s\n", __FUNCTION__);
  log("cbReqCount=%lx\n", *pcbReqCount);
  log("cbCurMaxFH=%lx\n", *pcbCurMaxFH);
  return KalSetRelMaxFH(pcbReqCount, pcbCurMaxFH);
}

APIRET APIENTRY  DosSetMaxFH(ULONG cFH)
{
  log("%s\n", __FUNCTION__);
  log("cFH=%lx\n", cFH);
  return KalSetMaxFH(cFH);
}


APIRET APIENTRY  DosFindFirst(PCSZ   pszFileSpec,
                              PHDIR  phdir,
                              ULONG  flAttribute,
                              PVOID  pfindbuf,
                              ULONG  cbBuf,
                              PULONG pcFileNames,
                              ULONG  ulInfoLevel)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("pszFileSpec=%s\n", pszFileSpec);
  log("flAttribute=%lx\n", flAttribute);
  log("cbBuf=%lx\n", cbBuf);
  log("ulInfoLevel=%lx\n", ulInfoLevel);
  rc = KalFindFirst((PSZ)pszFileSpec, phdir, flAttribute,
                      pfindbuf, cbBuf, pcFileNames, ulInfoLevel);
  log("hdir=%lx\n", *phdir);
  log("cFileNames=%lx\n", *pcFileNames);
  return rc;
}

APIRET APIENTRY  DosFindNext(HDIR   hDir,
                             PVOID  pfindbuf,
                             ULONG  cbfindbuf,
                             PULONG pcFilenames)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("hdir=%lx\n", hDir);
  log("cbfindbuf=%lx\n", cbfindbuf);
  rc = KalFindNext(hDir, pfindbuf, cbfindbuf, pcFilenames);
  log("cFilenames=%lx\n", *pcFilenames);
  return rc;
}

APIRET APIENTRY  DosFindClose(HDIR hDir)
{
  log("%s\n", __FUNCTION__);
  log("hdir=%lx\n", hDir);
  return KalFindClose(hDir);
}

APIRET APIENTRY  DosQueryFHState(HFILE hFile,
                                 PULONG pMode)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("hFile=%lx\n", hFile);
  rc = KalQueryFHState(hFile, pMode);
  log("mode=%lx\n", *pMode);
  return rc;
}

APIRET APIENTRY  DosSetFHState(HFILE hFile,
                               ULONG mode)
{
  log("%s\n", __FUNCTION__);
  log("hFile=%lx\n", hFile);
  log("mode=%lx\n", mode);
  return KalSetFHState(hFile, mode);
}

APIRET APIENTRY  DosQueryFileInfo(HFILE hf,
                                  ULONG ulInfoLevel,
                                  PVOID pInfo,
                                  ULONG cbInfoBuf)
{
  log("%s\n", __FUNCTION__);
  log("hf=%lx\n", hf);
  log("ulInfoLevel=%lu\n", ulInfoLevel);
  log("cbInfoBuf=%lu\n", cbInfoBuf);
  return KalQueryFileInfo(hf, ulInfoLevel, pInfo, cbInfoBuf);
}

APIRET APIENTRY  DosQueryPathInfo(PCSZ   pszPathName,
                                  ULONG ulInfoLevel,
                                  PVOID pInfo,
                                  ULONG cbInfoBuf)
{
  log("%s\n", __FUNCTION__);
  log("pszPathName=%lx\n", pszPathName);
  log("ulInfoLevel=%lu\n", ulInfoLevel);
  log("cbInfoBuf=%lu\n", cbInfoBuf);
  return KalQueryPathInfo((PSZ)pszPathName, ulInfoLevel, pInfo, cbInfoBuf);
}

APIRET APIENTRY  DosSetFileSizeL(HFILE hFile,
                                 LONGLONG cbSize)
{
  log("%s\n", __FUNCTION__);
  log("hFile=%lx\n", hFile);
  log("cbSize=%lld\n", cbSize);
  return KalSetFileSizeL(hFile, cbSize);
}
