#ifndef __OS3_KAL_H__
#define __OS3_KAL_H__

#ifdef __cplusplus
  extern "C" {
#endif

#define  INCL_DOSPROCESS
#define  INCL_DOS
#define  INCL_BASE
#include <os2.h>

void log(const char *fmt, ...);

APIRET __cdecl
KalOpenL (PSZ pszFileName,
          HFILE *phFile,
	  ULONG *pulAction,
	  LONGLONG cbFile,
	  ULONG ulAttribute,
	  ULONG fsOpenFlags,
	  ULONG fsOpenMode,
	  PEAOP2 peaop2);

APIRET __cdecl
KalFSCtl (PVOID pData,
          ULONG cbData,
	  PULONG pcbData,
	  PVOID pParms,
	  ULONG cbParms,
	  PULONG pcbParms,
	  ULONG function,
	  PSZ pszRoute,
	  HFILE hFile,
	  ULONG method);

APIRET __cdecl
KalRead (HFILE hFile, PVOID pBuffer,
            ULONG cbRead, PULONG pcbActual);

APIRET __cdecl
KalWrite (HFILE hFile, PVOID pBuffer,
              ULONG cbWrite, PULONG pcbActual);

APIRET __cdecl
KalLogWrite (PSZ s);

VOID __cdecl
KalExit(ULONG action, ULONG result);

APIRET __cdecl
KalQueryCurrentDisk(PULONG pdisknum,
                        PULONG plogical);

APIRET __cdecl
KalSetCurrentDir(PSZ pszDir);

APIRET __cdecl
KalSetDefaultDisk(ULONG disknum);

APIRET __cdecl
KalQueryCurrentDir(ULONG disknum,
                       PBYTE pBuf,
                       PULONG pcbBuf);

APIRET __cdecl
KalQueryProcAddr(ULONG hmod,
                     ULONG ordinal,
                     const PSZ  pszName,
                     void  **ppfn);

APIRET __cdecl
KalQueryModuleHandle(const char *pszModname,
                     unsigned long *phmod);

APIRET __cdecl
KalQueryModuleName(unsigned long hmod, unsigned long cbBuf, char *pBuf);

APIRET __cdecl
KalLoadModule(PSZ pszName,
                  ULONG cbName,
                  char const *pszModname,
                  PULONG phmod);

APIRET __cdecl
KalQueryProcType(HMODULE hmod,
                 ULONG ordinal,
		 PSZ pszName,
		 PULONG pulProcType);

APIRET __cdecl
KalQueryAppType(PSZ pszName,
                PULONG pFlags);

APIRET __cdecl
KalExecPgm(char *pObjname,
           long cbObjname,
           unsigned long execFlag,
           char *pArg,
           char *pEnv,
           struct _RESULTCODES *pRes,
           char *pName);

APIRET __cdecl
KalError(ULONG error);

APIRET __cdecl
KalAllocMem(PVOID *ppb,
            ULONG cb,
	    ULONG flags);

APIRET __cdecl
KalFreeMem(PVOID pb);

APIRET __cdecl
KalSetMem(PVOID pb,
          ULONG cb,
	  ULONG flags);

APIRET __cdecl
KalQueryMem(PVOID  pb,
            PULONG pcb,
	    PULONG pflags);

APIRET __cdecl
KalAllocSharedMem(PPVOID ppb,
                  PSZ    pszName,
		  ULONG  cb,
		  ULONG  flags);

APIRET __cdecl
KalGetSharedMem(PVOID pb,
                ULONG flag);

APIRET __cdecl
KalGetNamedSharedMem(PPVOID ppb,
                     PSZ pszName,
                     ULONG flag);

APIRET __cdecl
KalGiveSharedMem(PVOID pb,
                 PID pid,
                 ULONG flag);

APIRET __cdecl
KalResetBuffer(HFILE handle);

APIRET __cdecl
KalSetFilePtrL(HFILE handle,
               LONGLONG ib,
	       ULONG method,
	       PULONGLONG ibActual);

APIRET __cdecl
KalClose(HFILE handle);

APIRET __cdecl
KalQueryHType(HFILE handle,
              PULONG pType,
	      PULONG pAttr);

APIRET __cdecl
KalQueryDBCSEnv(ULONG cb,
                COUNTRYCODE *pcc,
		PBYTE pBuf);

APIRET __cdecl
KalQueryCp(ULONG cb,
           PULONG arCP,
	   PULONG pcCP);

APIRET __cdecl
KalGetInfoBlocks(PTIB *ptib, PPIB *ppib);

APIRET __cdecl
KalSetMaxFH(ULONG cFH);

APIRET __cdecl
KalSetRelMaxFH(PLONG pcbReqCount, PULONG pcbCurMaxFH);

APIRET __cdecl
KalSleep(ULONG ms);

APIRET __cdecl
KalDupHandle(HFILE hFile, HFILE *phFile2);

APIRET __cdecl
KalDelete(PSZ pszFileName);

APIRET __cdecl
KalForceDelete(PSZ pszFileName);

APIRET __cdecl
KalDeleteDir(PSZ pszDirName);

APIRET __cdecl
KalCreateDir(PSZ pszDirName, PEAOP2 peaop2);

APIRET __cdecl
KalFindFirst(char  *pszFileSpec,
             HDIR  *phDir,
             ULONG flAttribute,
             PVOID pFindBuf,
             ULONG cbBuf,
             ULONG *pcFileNames,
             ULONG ulInfolevel);

APIRET __cdecl
KalFindNext(HDIR  hDir,
            PVOID pFindBuf,
            ULONG cbBuf,
            ULONG *pcFileNames);

APIRET __cdecl
KalFindClose(HDIR hDir);

APIRET __cdecl
KalQueryFHState(HFILE hFile,
                PULONG pMode);

APIRET __cdecl
KalSetFHState(HFILE hFile,
              ULONG pMode);

APIRET __cdecl
KalQueryFileInfo(HFILE hf,
                 ULONG ulInfoLevel,
                 char *pInfo,
                 ULONG cbInfoBuf);

APIRET __cdecl
KalQueryPathInfo(PSZ pszPathName,
                 ULONG ulInfoLevel,
                 PVOID pInfo,
                 ULONG cbInfoBuf);

APIRET __cdecl
KalSetFileSizeL(HFILE hFile,
                LONGLONG cbSize);
APIRET __cdecl
KalMove(PSZ pszOld, PSZ pszNew);

APIRET __cdecl
KalOpenEventSem(PSZ pszName,
                PHEV phev);

APIRET __cdecl
KalCloseEventSem(HEV hev);

APIRET __cdecl
KalCreateEventSem(PSZ pszName,
                  PHEV phev,
                  ULONG flags,
                  BOOL32 fState);

APIRET __cdecl
KalCreateThread(PTID ptid,
                PFNTHREAD pfn,
                ULONG param,
                ULONG flag,
                ULONG cbStack);

APIRET __cdecl
KalSuspendThread(TID tid);

APIRET __cdecl
KalResumeThread(TID tid);

APIRET __cdecl
KalWaitThread(PTID ptid, ULONG option);

APIRET __cdecl
KalKillThread(TID tid);

#ifdef __cplusplus
  }
#endif

#endif
