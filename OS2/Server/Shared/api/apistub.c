/*
 *  Some minimal API
 *  functions stubs.
 */

#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define INCL_MISC
#include <os2.h>

#include <apistub.h>
#include <io.h>

#include <string.h>

PVOID entry_Table[] =
{0, 0,
 &api_DosWrite,
 &api_DosFSCtl,
 &api_DosExit,
 &api_DosQuerySysInfo};


APIRET APIENTRY
api_DosWrite(HFILE hFile, PVOID pBuffer,
         ULONG cbWrite, PULONG pcbActual)
{

  char buf[0x100];

  if (hFile != 1) return ERROR_INVALID_HANDLE;
  memcpy(buf, pBuffer, cbWrite+1);
  buf[cbWrite] = '\0';

  io_printf("%s", buf);

  return NO_ERROR;
}

APIRET APIENTRY
api_DosFSCtl(PVOID pData, ULONG cbData,
         PULONG pcbData, PVOID pParms,
         ULONG cbParms, PULONG pcbParms,
         ULONG xfunction, PCSZ pszRoute,
         HFILE hFile, ULONG method)
{
  return NO_ERROR;
}

VOID APIENTRY
api_DosExit(ULONG action, ULONG result)
{
}

APIRET APIENTRY
api_DosQuerySysInfo(ULONG iStart, ULONG iLast,
                PVOID pBuf, ULONG cbBuf)
{
  return NO_ERROR;
}
