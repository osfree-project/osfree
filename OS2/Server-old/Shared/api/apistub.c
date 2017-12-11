/*
 *  Some minimal API
 *  functions stubs.
 */

#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define INCL_DOSMISC
#include <os2.h>

#include <apistub.h>
#include <io.h>

#include <string.h>
#include <stdlib.h>

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

VOID APIENTRY
api_DosExit(ULONG action, ULONG result)
{
  io_printf("action=%d, result=%d\n", action, result);
  exit;
}
