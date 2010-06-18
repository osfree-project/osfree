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
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

  char buf[0x100];

  void exe_end(void);

APIRET CDECL
api_DosRead(HFILE hFile, PVOID pBuffer,
            ULONG cbRead, PULONG pcbActual)
{
  int  nread = 1;
  int  total = 0;
  char *buf = pBuffer;

  if (hFile)
  {
    io_printf("DosRead not implemented for hFile != stdin!\n");
    return 6; //ERROR_INVALID_HANDLE
  }

  do
  {
    nread = read(hFile, buf + nread - 1, cbRead - nread + 1);
    if (nread == -1)
    {
      switch (errno)
      {
        // @todo: more accurate error handling
        default:
          return 232; //ERROR_NO_DATA
      }
    }
    total += nread;
  } while (buf[total - 1] == EOF || buf[total - 1] != '\n');
  
  *pcbActual = total;
  //io_printf("hFile=%d, pBuffer=%x, cbRead=%u, cbActual=%u\n", hFile, pBuffer, cbRead, *pcbActual);

  return 0; // NO_ERROR
}

APIRET CDECL
api_DosWrite(HFILE hFile, PVOID pBuffer,
         ULONG cbWrite, PULONG pcbActual)
{
//  unsigned long esp;
//  asm ("pop %[esp]\n"::[esp] "m" (esp));
//  io_printf("hfile=%x", esp);
//  asm ("pop %[esp]\n"::[esp] "m" (esp));
//  io_printf("pbuffer=%x", esp);
//  asm ("pop %[esp]\n"::[esp] "m" (esp));
//  io_printf("cbwrite=%x", esp);
//  asm ("pop %[esp]\n"::[esp] "m" (esp));
//  io_printf("pulactual=%x", esp);

  io_printf("hFile=%x, pBuffer=%x, cbWrite=%x, pcbActial=%x\n", hFile, pBuffer, cbWrite, pcbActual);

  if (hFile != 1) return 6/*ERROR_INVALID_HANDLE*/;
  memcpy(buf, pBuffer, cbWrite+1);
  buf[cbWrite] = '\0';

  io_printf("%s\n", buf);

  return 0/*NO_ERROR*/;
}

VOID CDECL
api_DosExit(ULONG action, ULONG result)
{
  //io_printf("action=%d, result=%d\n", action, result);
  exe_end();
}


APIRET CDECL
api_DosQueryCurrentDisk(PULONG pdisknum,
                        PULONG plogical)
{

}


APIRET CDECL
api_DosQueryCurrentDir(ULONG disknum,
                       PBYTE pBuf,
                       PULONG pcbBuf)
{

}
