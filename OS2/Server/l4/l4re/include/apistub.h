/*
 *
 *
 */

#ifndef __APISTUB_H__
#define __APISTUB_H__

#include <l4/os3/gcc_os2def.h>
#include <l4/os3/processmgr.h>

APIRET CDECL
kalRead(HFILE hFile, PVOID pBuffer,
            ULONG cbRead, PULONG pcbRead);

APIRET CDECL
kalWrite(HFILE hFile, PVOID pBuffer,
         ULONG cbWrite, PULONG pcbActual);

APIRET CDECL
kalLogWrite (PSZ s);

APIRET CDECL
kalFSCtl(PVOID pData, ULONG cbData,
         PULONG pcbData, PVOID pParms,
         ULONG cbParms, PULONG pcbParms,
         ULONG function, PSZ pszRoute,
         HFILE hFile, ULONG method);

VOID CDECL
kalExit(ULONG action, ULONG result);

APIRET CDECL
kalQueryCurrentDisk(PULONG pdisknum,
                        PULONG plogical);

APIRET CDECL
kalSetCurrentDir(PSZ pszDir);

APIRET CDECL
kalSetDefaultDisk(ULONG disknum);

APIRET CDECL
kalQueryCurrentDir(ULONG disknum, BYTE * pBuf, PULONG pcbBuf);

APIRET CDECL
kalLoadModule(PSZ pszName,
                  ULONG cbName,
                  char const *pszModname,
                  PULONG phmod);

APIRET CDECL
kalQueryProcAddr(ULONG hmod,
                     ULONG ordinal,
                     const PSZ  pszName,
                     void  **ppfn);

APIRET CDECL
kalQueryProcType(HMODULE hmod,
                 ULONG ordinal,
		 PSZ pszName,
		 PULONG pulProcType);

APIRET CDECL
kalQueryAppType(PSZ pszName,
                PULONG pFlags);

APIRET CDECL
kalExecPgm(char *pObjname,
           long cbObjname,
           unsigned long execFlag,
           char *pArg,
           char *pEnv,
           struct _RESULTCODES *pRes,
           char *pName);

APIRET CDECL
kalError(ULONG error);

APIRET CDECL
kalAllocMem(PVOID *ppb,
            ULONG cb,
	    ULONG flags);

APIRET CDECL
kalFreeMem(PVOID pb);

APIRET CDECL
kalResetBuffer(HFILE handle);

APIRET CDECL
kalSetFilePtrL(HFILE handle,
               LONGLONG ib,
	       ULONG method,
	       PULONGLONG ibActual);
	      
APIRET CDECL
kalClose(HFILE handle);

APIRET CDECL
kalSetMaxFH(ULONG cFH);

APIRET CDECL
kalSetRelMaxFH(PLONG pcbReqCount, PULONG pcbCurMaxFH);

APIRET CDECL
kalFindFirst(char  *pszFileSpec,
             HDIR  *phDir,
             ULONG flAttribute,
             PVOID pFindBuf,
             ULONG cbBuf,
             ULONG *pcFileNames,
             ULONG ulInfolevel);

APIRET CDECL
kalFindNext(HDIR  hDir,
            PVOID pFindBuf,
            ULONG cbBuf,
            ULONG *pcFileNames);

APIRET CDECL
kalFindClose(HDIR hDir);

APIRET CDECL
kalQueryFHState(HFILE hFile,
                PULONG pMode);

APIRET CDECL
kalSetFHState(HFILE hFile,
              ULONG pMode);

APIRET CDECL
kalQueryFileInfo(HFILE hf,
                 ULONG ulInfoLevel,
                 char *pInfo,
                 ULONG cbInfoBuf);

APIRET CDECL
kalQueryPathInfo(PSZ pszPathName,
                 ULONG ulInfoLevel,
                 PVOID pInfo,
                 ULONG cbInfoBuf);

APIRET CDECL
kalSetFileSizeL(HFILE hFile,
                long long cbSize);

APIRET CDECL
kalAllocSharedMem(PPVOID ppb,
                  PSZ    pszName,
		  ULONG  cb,
		  ULONG  flags);

APIRET CDECL
kalCreateDir(PSZ pszDirName, PEAOP2 peaop2);

APIRET CDECL
kalDupHandle(HFILE hFile, HFILE *phFile2);

APIRET CDECL
kalDelete(PSZ pszFileName);

APIRET CDECL
kalForceDelete(PSZ pszFileName);

APIRET CDECL
kalDeleteDir(PSZ pszDirName);

APIRET CDECL
kalGetInfoBlocks(PTIB *pptib, PPIB *pppib);

APIRET CDECL
kalMove(PSZ pszOld, PSZ pszNew);

APIRET CDECL
kalOpenL (PSZ pszFileName,
          HFILE *phFile,
	  ULONG *pulAction,
	  LONGLONG cbFile,
	  ULONG ulAttribute,
	  ULONG fsOpenFlags,
	  ULONG fsOpenMode,
	  PEAOP2 peaop2);
	  
APIRET CDECL
kalQueryHType(HFILE handle,
              PULONG pType,
	      PULONG pAttr);

APIRET CDECL
kalQueryDBCSEnv(ULONG cb,
                COUNTRYCODE *pcc,
		char *pBuf);

APIRET CDECL
kalQueryCp(ULONG cb,
           PULONG arCP,
	   PULONG pcCP);

APIRET CDECL
kalQueryMem(PVOID  pb,
            PULONG pcb,
	    PULONG pflags);

APIRET CDECL
kalQueryModuleName(unsigned long hmod, unsigned long cbBuf, char *pBuf);

APIRET CDECL
kalQueryModuleHandle(const char *pszModname,
                     unsigned long *phmod);


APIRET CDECL
kalSleep(ULONG ms);

APIRET CDECL
kalSetMem(PVOID pb,
          ULONG cb,
	  ULONG flags);

#endif
