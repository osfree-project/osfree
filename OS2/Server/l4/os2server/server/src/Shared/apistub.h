/*
 *
 *
 */
#include <gcc_os2def.h>

unsigned long __attribute__((__cdecl__)) //APIRET APIENTRY
api_DosWrite
(HFILE hFile, PVOID pBuffer,
         ULONG cbWrite, PULONG pcbActual);

APIRET APIENTRY
api_DosFSCtl(PVOID pData, ULONG cbData,
         PULONG pcbData, PVOID pParms,
         ULONG cbParms, PULONG pcbParms,
         ULONG xfunction, PCSZ pszRoute,
         HFILE hFile, ULONG method);

void __attribute__((__cdecl__))
api_DosExit(ULONG action, ULONG result);

unsigned long __attribute__((__cdecl__))
api_DosQueryCurrentDisk(PULONG pdisknum,
                        PULONG plogical);

unsigned long __attribute__((__cdecl__))
api_DosQueryCurrentDir(ULONG disknum, BYTE * pBuf, PULONG pcbBuf);

APIRET APIENTRY
api_DosQuerySysInfo(ULONG iStart, ULONG iLast,
                PVOID pBuf, ULONG cbBuf);

