#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <io.h>
#include <signal.h>

#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_DOSERRORS
#define INCL_DOS
#include <os2.h>
#include "portable.h"
#include "fat32def.h"


int main(int iArgc, PSZ pszArgv[])
{
ULONG ulDataSize, ulParmSize;
F32PARMS f32Parms;
APIRET rc;

   ulDataSize = sizeof f32Parms;
   while (!kbhit())
      {
      rc = DosFSCtl(
         (PVOID)&f32Parms, ulDataSize, &ulDataSize,
         NULL, 0, &ulParmSize,
         FAT32_GETPARMS, "FAT32", -1, FSCTL_FSDNAME);

      if (rc)
         {
         printf("DosFSCtl, FAT32_GETPARMS failed, rc = %d\n", rc);
         DosExit(EXIT_PROCESS, 1);
         }
      printf("%4u %4u\b\b\b\b\b\b\b\b\b",
         f32Parms.usDirtySectors, f32Parms.usPendingFlush);
      fflush(stdout);

      DosSleep(100);
      }
   return 0;
}

