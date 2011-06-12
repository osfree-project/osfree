/*
 *
 *
 */

#ifndef __APISTUB_H__
#define __APISTUB_H__

#include <l4/os3/gcc_os2def.h>
#include <l4/os3/processmgr.h>

APIRET CDECL
KalRead(HFILE hFile, PVOID pBuffer,
            ULONG cbRead, PULONG pcbRead);

APIRET CDECL
KalWrite
(HFILE hFile, PVOID pBuffer,
         ULONG cbWrite, PULONG pcbActual);

APIRET CDECL
KalLogWrite (PSZ s);

APIRET CDECL
KalFSCtl(PVOID pData, ULONG cbData,
         PULONG pcbData, PVOID pParms,
         ULONG cbParms, PULONG pcbParms,
         ULONG function, PSZ pszRoute,
         HFILE hFile, ULONG method);

VOID CDECL
KalExit(ULONG action, ULONG result);

APIRET CDECL
KalQueryCurrentDisk(PULONG pdisknum,
                        PULONG plogical);

APIRET CDECL
KalSetDefaultDisk(ULONG disknum);

APIRET CDECL
KalQueryCurrentDir(ULONG disknum, BYTE * pBuf, PULONG pcbBuf);

APIRET CDECL
KalQuerySysInfo(ULONG iStart, ULONG iLast,
                PVOID pBuf, ULONG cbBuf);

APIRET CDECL
KalLoadModule(PSZ pszName,
                  ULONG cbName,
                  char const *pszModname,
                  PULONG phmod);

APIRET CDECL
KalQueryProcAddr(ULONG hmod,
                     ULONG ordinal,
                     const PSZ  pszName,
                     void  **ppfn);

APIRET CDECL
KalExecPgm(char *pObjname,
           long cbObjname,
           unsigned long execFlag,
           char *pArg,
           char *pEnv,
           struct _RESULTCODES *pRes,
           char *pName);

APIRET CDECL
KalError(ULONG error);

APIRET CDECL
KalAllocMem(PVOID *ppb,
            ULONG cb,
	    ULONG flags);

APIRET CDECL
KalFreeMem(PVOID pb);

APIRET CDECL
KalResetBuffer(HFILE handle);

APIRET CDECL
KalSetFilePtrL(HFILE handle,
               LONGLONG ib,
	       ULONG method,
	       PLONGLONG ibActual);
	      
APIRET CDECL
KalClose(HFILE handle);

APIRET CDECL
KalScanEnv(PSZ pszName,
           PPSZ ppszValue);

APIRET CDECL
KalSetMaxFH(ULONG cFH);

APIRET CDECL
KalSetRelMaxFH(PLONG pcbReqCount, PULONG pcbCurMaxFH);

#endif
