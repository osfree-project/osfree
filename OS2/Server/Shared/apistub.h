/*
 *
 *
 */
#include <os2.h>

APIRET APIENTRY
api_DosWrite(HFILE hFile, PVOID pBuffer,
         ULONG cbWrite, PULONG pcbActual);

APIRET APIENTRY
api_DosFSCtl(PVOID pData, ULONG cbData,
         PULONG pcbData, PVOID pParms,
         ULONG cbParms, PULONG pcbParms,
         ULONG xfunction, PCSZ pszRoute,
         HFILE hFile, ULONG method);

VOID APIENTRY
api_DosExit(ULONG action, ULONG result);

APIRET APIENTRY
api_DosQuerySysInfo(ULONG iStart, ULONG iLast,
                PVOID pBuf, ULONG cbBuf);
