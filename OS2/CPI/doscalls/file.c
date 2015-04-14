#include <os2.h>

#include "dl.h"

//APIRET __cdecl  KalClose(HFILE hFile);

//APIRET __cdecl  KalQueryHType(HFILE hFile,
//                              PULONG pType,
//                              PULONG pAttr);

//APIRET __cdecl  KalResetBuffer(HFILE);

//APIRET __cdecl  KalDupHandle(HFILE hFile,
//                             PHFILE pHfile);

//APIRET __cdecl  KalDelete(PCSZ  pszFile);

//APIRET __cdecl  KalForceDelete(PCSZ  pszFile);

//APIRET __cdecl  KalDeleteDir(PCSZ  pszDir);

//APIRET __cdecl  KalCreateDir(PCSZ  pszDirName,
//                             PEAOP2 peaop2);

//APIRET __cdecl  KalSetRelMaxFH(PLONG pcbReqCount,
//                               PULONG pcbCurMaxFH);

//APIRET __cdecl  KalSetMaxFH(ULONG cFH);

//APIRET __cdecl  KalFindFirst(PCSZ   pszFileSpec,
//                             PHDIR  phdir,
//                             ULONG  flAttribute,
//                             PVOID  pfindbuf,
//                             ULONG  cbBuf,
//                             PULONG pcFileNames,
//                             ULONG  ulInfoLevel);

//APIRET __cdecl  KalFindNext(HDIR   hDir,
//                            PVOID  pfindbuf,
//                            ULONG  cbfindbuf,
//                            PULONG pcFilenames);

//APIRET __cdecl  KalFindClose(HDIR hDir);

//APIRET __cdecl  KalQueryFHState(HFILE hFile,
//                                PULONG pMode);

//APIRET __cdecl  KalSetFHState(HFILE hFile,
//                              ULONG mode);

//APIRET __cdecl  KalQueryFileInfo(HFILE hf,
//                                 ULONG ulInfoLevel,
//                                 PVOID pInfo,
//                                 ULONG cbInfoBuf);

//APIRET __cdecl  KalQueryPathInfo(PCSZ  pszPathName,
//                                 ULONG ulInfoLevel,
//                                 PVOID pInfoBuf,
//                                 ULONG cbInfoBuf);

//APIRET __cdecl  KalSetFileSizeL(HFILE hFile,
//                                LONGLONG cbSize);

#if 0
// Implementation of kernel-independed functions via kernel-depended functions

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
  return KalClose (hFile);
}

APIRET APIENTRY  DosQueryHType(HFILE hFile,
                               PULONG pType,
                               PULONG pAttr)
{
  return KalQueryHType(hFile, pType, pAttr);
}

APIRET APIENTRY DosResetBuffer(HFILE hFile)
{
  return KalResetBuffer(hFile);
}

APIRET APIENTRY  DosDupHandle(HFILE hFile,
                              PHFILE pHfile)
{
  return KalDupHandle(hFile, pHfile);
}

APIRET APIENTRY  DosDelete(PCSZ  pszFile)
{
  return KalDelete(pszFile);
}

APIRET APIENTRY  DosForceDelete(PCSZ  pszFile)
{
  return KalForceDelete(pszFile);
}

APIRET APIENTRY  DosDeleteDir(PCSZ  pszDir)
{
  return KalDeleteDir(pszDir);
}

APIRET APIENTRY  DosCreateDir(PCSZ  pszDirName,
                              PEAOP2 peaop2)
{
  return KalCreateDir(pszDirName, peaop2);
}

APIRET APIENTRY  DosSetRelMaxFH(PLONG pcbReqCount,
                                PULONG pcbCurMaxFH)
{
  return KalSetRelMaxFH(pcbReqCount, pcbCurMaxFH);
}

APIRET APIENTRY  DosSetMaxFH(ULONG cFH)
{
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
  return KalFindFirst(pszFileSpec, phdir, flAttribute,
                      pfindbuf, cbBuf, pcFileNames, ulInfoLevel);
}

APIRET APIENTRY  DosFindNext(HDIR   hDir,
                             PVOID  pfindbuf,
                             ULONG  cbfindbuf,
                             PULONG pcFilenames)
{
  return KalFindNext(hDir, pfindbuf, cbfindbuf, pcFilenames);
}

APIRET APIENTRY  DosFindClose(HDIR hDir)
{
  return KalFindClose(hDir);
}

APIRET APIENTRY  DosQueryFHState(HFILE hFile,
                                 PULONG pMode)
{
  return KalQueryFHState(hFile, pMode);
}

APIRET APIENTRY  DosSetFHState(HFILE hFile,
                               ULONG mode)
{
  return KalSetFHState(hFile, mode);
}

APIRET APIENTRY  DosQueryFileInfo(HFILE hf,
                                  ULONG ulInfoLevel,
                                  PVOID pInfo,
                                  ULONG cbInfoBuf)
{
  return KalQueryFileInfo(hf, ulInfoLevel, pInfo, cbInfoBuf);
}

APIRET APIENTRY  DosQueryPathInfo(PCSZ   pszPathName,
                                  ULONG ulInfoLevel,
                                  PVOID pInfo,
                                  ULONG cbInfoBuf)
{
  return KalQueryPathInfo(pszPathName, ulInfoLevel, pInfo, cbInfoBuf);
}

APIRET APIENTRY  DosSetFileSizeL(HFILE hFile,
                                 LONGLONG cbSize)
{
  return KalSetFileSizeL(hFile, cbSize);
}
