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
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("hfile=%lx\n", hFile);
  rc = KalClose (hFile);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY  DosQueryHType(HFILE hFile,
                               PULONG pType,
                               PULONG pAttr)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("hfile=%lx\n", hFile);
  rc = KalQueryHType(hFile, pType, pAttr);
  log("type=%lx\n", *pType);
  log("attr=%lx\n", *pAttr);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY DosResetBuffer(HFILE hFile)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("hfile=%lx\n", hFile);
  rc = KalResetBuffer(hFile);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY  DosDupHandle(HFILE hFile,
                              PHFILE pHfile)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("hfile=%lx\n", hFile);
  rc = KalDupHandle(hFile, pHfile);
  log("hfile2=%lx\n", *pHfile);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY  DosDelete(PCSZ  pszFile)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("pszFile=%s\n", pszFile);
  rc = KalDelete((PSZ)pszFile);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY  DosForceDelete(PCSZ  pszFile)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("pszFile=%s\n", pszFile);
  rc = KalForceDelete((PSZ)pszFile);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY  DosDeleteDir(PCSZ  pszDir)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("pszDir=%s\n", pszDir);
  rc = KalDeleteDir((PSZ)pszDir);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY  DosCreateDir(PCSZ  pszDirName,
                              PEAOP2 peaop2)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("pszDirName=%s\n", pszDirName);
  rc = KalCreateDir((PSZ)pszDirName, peaop2);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY  DosSetRelMaxFH(PLONG pcbReqCount,
                                PULONG pcbCurMaxFH)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("cbReqCount=%lx\n", *pcbReqCount);
  log("cbCurMaxFH=%lx\n", *pcbCurMaxFH);
  rc = KalSetRelMaxFH(pcbReqCount, pcbCurMaxFH);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY  DosSetMaxFH(ULONG cFH)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("cFH=%lx\n", cFH);
  rc = KalSetMaxFH(cFH);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
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
  log("%s enter\n", __FUNCTION__);
  log("pszFileSpec=%s\n", pszFileSpec);
  log("flAttribute=%lx\n", flAttribute);
  log("cbBuf=%lx\n", cbBuf);
  log("ulInfoLevel=%lx\n", ulInfoLevel);
  rc = KalFindFirst((PSZ)pszFileSpec, phdir, flAttribute,
                      pfindbuf, cbBuf, pcFileNames, ulInfoLevel);
  log("hdir=%lx\n", *phdir);
  log("cFileNames=%lx\n", *pcFileNames);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY  DosFindNext(HDIR   hDir,
                             PVOID  pfindbuf,
                             ULONG  cbfindbuf,
                             PULONG pcFilenames)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("hdir=%lx\n", hDir);
  log("cbfindbuf=%lx\n", cbfindbuf);
  rc = KalFindNext(hDir, pfindbuf, cbfindbuf, pcFilenames);
  log("cFilenames=%lx\n", *pcFilenames);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY  DosFindClose(HDIR hDir)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("hdir=%lx\n", hDir);
  rc = KalFindClose(hDir);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY  DosQueryFHState(HFILE hFile,
                                 PULONG pMode)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("hFile=%lx\n", hFile);
  rc = KalQueryFHState(hFile, pMode);
  log("mode=%lx\n", *pMode);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY  DosSetFHState(HFILE hFile,
                               ULONG mode)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("hFile=%lx\n", hFile);
  log("mode=%lx\n", mode);
  rc = KalSetFHState(hFile, mode);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY  DosQueryFileInfo(HFILE hf,
                                  ULONG ulInfoLevel,
                                  PVOID pInfo,
                                  ULONG cbInfoBuf)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("hf=%lx\n", hf);
  log("ulInfoLevel=%lu\n", ulInfoLevel);
  log("cbInfoBuf=%lu\n", cbInfoBuf);
  rc = KalQueryFileInfo(hf, ulInfoLevel, pInfo, cbInfoBuf);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY  DosQueryPathInfo(PCSZ  pszPathName,
                                  ULONG ulInfoLevel,
                                  PVOID pInfo,
                                  ULONG cbInfoBuf)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("pszPathName=%s\n", pszPathName);
  log("ulInfoLevel=%lu\n", ulInfoLevel);
  log("cbInfoBuf=%lu\n", cbInfoBuf);
  rc = KalQueryPathInfo((PSZ)pszPathName, ulInfoLevel, pInfo, cbInfoBuf);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY  DosSetFileSizeL(HFILE hFile,
                                 LONGLONG cbSize)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("hFile=%lx\n", hFile);
  log("cbSize=%lld\n", cbSize);
  rc = KalSetFileSizeL(hFile, cbSize);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}
