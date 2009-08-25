/*
 *  Some minimal API
 *  functions stubs.
 */

#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define INCL_DOSMISC
#include <gcc_os2def.h>

#include "../apistub.h"
#include <io.h>

#include <string.h>

APIRET APIENTRY
api_DosWrite(HFILE hFile, PVOID pBuffer,
         ULONG cbWrite, PULONG pcbActual)
{

  char buf[0x100];

  if (hFile != 1) return 6/*ERROR_INVALID_HANDLE*/;
  memcpy(buf, pBuffer, cbWrite+1);
  buf[cbWrite] = '\0';

  io_printf("%s", buf);

  return 0/*NO_ERROR*/;
}

VOID APIENTRY
api_DosExit(ULONG action, ULONG result)
{
  io_printf("action=%d, result=%d\n", action, result);
//  exit;
}

