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

  char buf[0x100];

  void exe_end(void);


//APIRET APIENTRY
ULONG __attribute((__cdecl__)) 
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

//VOID APIENTRY
void __attribute((__cdecl__)) 
api_DosExit(ULONG action, ULONG result)
{
  //io_printf("action=%d, result=%d\n", action, result);
  exe_end();
}


ULONG __attribute((__cdecl__)) 
api_DosQueryCurrentDisk(PULONG pdisknum,
                        PULONG plogical)
{
  
}

