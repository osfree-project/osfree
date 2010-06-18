/*
 *
 *
 */
#include <gcc_os2def.h>

APIRET CDECL
api_DosRead(HFILE hFile, PVOID pBuffer,
            ULONG cbRead, PULONG pcbActual);

APIRET CDECL
api_DosWrite
(HFILE hFile, PVOID pBuffer,
         ULONG cbWrite, PULONG pcbActual);

APIRET CDECL
api_DosFSCtl(PVOID pData, ULONG cbData,
         PULONG pcbData, PVOID pParms,
         ULONG cbParms, PULONG pcbParms,
         ULONG xfunction, PCSZ pszRoute,
         HFILE hFile, ULONG method);

VOID CDECL
api_DosExit(ULONG action, ULONG result);

APIRET CDECL
api_DosQueryCurrentDisk(PULONG pdisknum,
                        PULONG plogical);

APIRET CDECL
api_DosQueryCurrentDir(ULONG disknum, BYTE * pBuf, PULONG pcbBuf);

APIRET CDECL
api_DosQuerySysInfo(ULONG iStart, ULONG iLast,
                PVOID pBuf, ULONG cbBuf);

