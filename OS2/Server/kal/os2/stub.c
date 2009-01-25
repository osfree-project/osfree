//
//
//
//

#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define INCL_MISC
#include <os2.h>

APIRET APIENTRY
DosWrite(HFILE hFile, PVOID pBuffer,
         ULONG cbWrite, PULONG pcbActual)
{
  return NO_ERROR;
}

APIRET APIENTRY
DosFSCtl(PVOID pData, ULONG cbData,
         PULONG pcbData, PVOID pParms,
         ULONG cbParms, PULONG pcbParms,
         ULONG xfunction, PCSZ pszRoute,
         HFILE hFile, ULONG method)
{
  return NO_ERROR;
}

VOID APIENTRY
DosExit(ULONG action, ULONG result)
{
}

APIRET APIENTRY
DosQuerySysInfo(ULONG iStart, ULONG iLast,
                PVOID pBuf, ULONG cbBuf)
{
   return NO_ERROR;
}

//  return DosWrite(hfile, pBuf, cbMsg, &ulActual);
