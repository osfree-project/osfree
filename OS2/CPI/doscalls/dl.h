/*  DL.DLL dynamic loader virtual library
 *
 */

#ifndef __DL_H__
#define __DL_H__

#include <os2def.h>

/* DL.DLL functions      */
APIRET __cdecl  DlOpen(PSZ name, PULONG handle);
APIRET __cdecl  DlRoute(ULONG handle, PSZ name, ...);

/* libkal.s.so handle */
#define KAL_HANDLE 0

/* KAL functions */
#define KalAllocMem(ppb, cb, flags)                  DlRoute(0, "KalAllocMem", ppb, cb, flags)
#define KalAllocSharedMem(ppb, pszName, cb, flags)   DlRoute(0, "KalAllocSharedMem", ppb, pszName, cb, flags)
#define KalClose(handle)                             DlRoute(0, "KalClose", handle)
#define KalCreateDir(pszDirName, peaop2)             DlRoute(0, "KalCreateDir", pszDirName, peaop2)
#define KalDelete(pszFileName)                       DlRoute(0, "KalDelete", pszFileName)
#define KalDeleteDir(pszDirName)                     DlRoute(0, "KalDeleteDir", pszDirName)
#define KalDupHandle(hFile, phFile2)                 DlRoute(0, "KalDupHandle",hFile, phFile2)
#define KalError(error)                              DlRoute(0, "KalError", error)
#define KalExecPgm(pObjname, cbObjname, execFlag, \
           pArg, pEnv, pRes, pName)                  DlRoute(0, "KalExecPgm", pObjname, cbObjname, execFlag, \
                                                             pArg, pEnv, pRes, pName)
#define KalExit(action, result)                      DlRoute(0, "KalExit", action, result)
#define KalFSCtl(pData, cbData, pcbData, pParms, \
             cbParms, pcbParms, function, pszRoute, \
             hFile, method)                          DlRoute(0, "KalFSCtl", pData, cbData, pcbData, pParms, \
                                                             cbParms, pcbParms, function, pszRoute, hFile, method)
#define KalFindClose(hDir)                           DlRoute(0, "KalFindClose", hDir)
#define KalFindFirst(pszFileSpec, phDir, \
             flAttribute, pFindBuf, cbBuf, \
             pcFileNames, ulInfolevel)               DlRoute(0, "KalFindFirst", pszFileSpec, phDir, flAttribute, \
                                                             pFindBuf, cbBuf, pcFileNames, ulInfolevel)
#define KalFindNext(hDir, pFindBuf, cbBuf, pcFileNames) DlRoute(0, "KalFindNext", hDir, pFindBuf, cbBuf, pcFileNames)
#define KalForceDelete(pszFileName)                  DlRoute(0, "KalForceDelete", pszFileName)
#define KalFreeMem(pb)                               DlRoute(0, "KalFreeMem", pb)
#define KalGetInfoBlocks(pptib, pppib)               DlRoute(0, "KalGetInfoBlocks", pptib, pppib)
#define KalLoadModule(pszName, cbName, pszModname, phmod) DlRoute(0, "KalLoadModule", pszName, cbName, pszModname, phmod)
#define KalLogWrite(s)                               DlRoute(0, "KalLogWrite", s)
#define KalMove(pszOld, pszNew)                      DlRoute(0, "KalMove", pszOld, pszNew)
#define KalOpenL(pszFileName, phFile, pulAction, \
         cbFile, ulAttribute, fsOpenFlags, \
         fsOpenMode, peaop2)                         DlRoute(0, "KalOpenL", pszFileName, phFile, pulAction, cbFile, ulAttribute, \
                                                             fsOpenFlags, fsOpenMode, peaop2)
#define KalQueryAppType(pszName, pFlags)             DlRoute(0, "KalQueryAppType", pszName, pFlags)
#define KalQueryCp(cb, arCP, pcCP)                   DlRoute(0, "KalQueryCp", cb, arCP, pcCP)
#define KalQueryCurrentDir(disknum, pBuf, pcbBuf)    DlRoute(0, "KalQueryCurrentDir", disknum, pBuf, pcbBuf)
#define KalQueryCurrentDisk(pdisknum, plogical)      DlRoute(0, "KalQueryCurrentDisk", pdisknum, plogical)
#define KalQueryDBCSEnv(cb, pcc, pBuf)               DlRoute(0, "KalQueryDBCSEnv", cb, pcc, pBuf)
#define KalQueryFHState(hFile, pMode)                DlRoute(0, "KalQueryFHState", hFile, pMode)
#define KalQueryFileInfo(hf, ulInfoLevel, \
                         pInfo, cbInfoBuf)           DlRoute(0, "KalQueryFileInfo", hf, ulInfoLevel, pInfo, cbInfoBuf)
#define KalQueryHType(handle, pType, pAttr)          DlRoute(0, "KalQueryHType", handle, pType, pAttr)
#define KalQueryMem(pb, pcb, pflags)                 DlRoute(0, "KalQueryMem", pb, pcb, pflags)
#define KalQueryModuleHandle(pszModname, phmod)      DlRoute(0, "KalQueryModuleHandle", pszModname, phmod)
#define KalQueryModuleName(hmod, cbBuf, pBuf)        DlRoute(0, "KalQueryModuleName", hmod, cbBuf, pBuf)
#define KalQueryPathInfo(pszPathName, ulInfoLevel, \
                         pInfo, cbInfoBuf)           DlRoute(0, "KalQueryPathInfo", pszPathName, ulInfoLevel, pInfo, cbInfoBuf)
#define KalQueryProcAddr(hmod, ordinal, pszName, ppfn) DlRoute(0, "KalQueryProcAddr", hmod, ordinal, pszName, ppfn)
#define KalQueryProcType(hmod, ordinal, pszName, pulProcType) DlRoute(0, "KalQueryProcType", hmod, ordinal, pszName, pulProcType)
#define KalRead(hFile, pBuffer, cbRead, pcbRead)     DlRoute(0, "KalRead", hFile, pBuffer, cbRead, pcbRead)
#define KalResetBuffer(handle)                       DlRoute(0, "KalResetBuffer", handle)
#define KalSetCurrentDir(pszDir)                     DlRoute(0, "KalSetCurrentDir", pszDir)
#define KalSetDefaultDisk(disknum)                   DlRoute(0, "KalSetDefaultDisk", disknum)
#define KalSetFHState(hFile, pMode)                  DlRoute(0, "KalSetFHState", hFile, pMode)
#define KalSetFilePtrL(handle, ib, method, ibActual) DlRoute(0, "KalSetFilePtrL", handle, ib, method, ibActual)
#define KalSetFileSizeL(hFile, cbSize)               DlRoute(0, "KalSetFileSizeL", hFile, cbSize)
#define KalSetMaxFH(cFH)                             DlRoute(0, "KalSetMaxFH", cFH)
#define KalSetMem(pb, cb, flags)                     DlRoute(0, "KalSetMem", pb, cb, flags)
#define KalSetRelMaxFH(pcbReqCount, pcbCurMaxFH)     DlRoute(0, "KalSetRelMaxFH", pcbReqCount, pcbCurMaxFH)
#define KalSleep(ms)                                 DlRoute(0, "KalSleep",ms)
#define KalWrite(hFile, pBuffer, cbWrite, pcbActual) DlRoute(0, "KalWrite", hFile, pBuffer, cbWrite, pcbActual)

#endif
