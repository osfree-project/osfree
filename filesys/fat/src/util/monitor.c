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

char szBuffer[16348];

void CtrlCHandler(int signal);

F32PARMS f32Parms;

int main(INT iArgc, PSZ Argv[])
{
APIRET rc;
ULONG  ulTimeOut = 10L;
ULONG ulParmSize;
ULONG ulDataSize;
FILE *fLog;
BOOL fActive;
BOOL fSilent = FALSE;
BOOL fForce = FALSE;
INT  iArg;

   signal(SIGINT, CtrlCHandler);
   signal(SIGTERM, CtrlCHandler);
   signal(SIGBREAK, CtrlCHandler);

   fActive = 1;
   for (iArg = 1; iArg < iArgc; iArg++)
      {
      if (!stricmp(Argv[iArg], "off"))
         {
         fActive = FALSE;
         break;
         }
      if (isdigit(Argv[iArg][0]))
         fActive = atoi(Argv[iArg]);
      if (!stricmp(Argv[iArg], "/S"))
         fSilent = TRUE;
      if (!stricmp(Argv[iArg], "/F"))
         fForce = TRUE;
      }


   ulParmSize = sizeof fActive;
   rc = DosFSCtl(
      NULL, 0, &ulDataSize,
     (PVOID)&fActive, ulParmSize, &ulParmSize,
      FAT32_SETMESSAGE, "FAT32", -1, FSCTL_FSDNAME);

   if (!fActive)
      return 0;

   rc = DosSetPriority(PRTYS_PROCESS,
      PRTYC_FOREGROUNDSERVER, 0, 0);

   fLog = fopen("FAT32.LOG", "wb");
   for (;;)
      {
      if (kbhit())
         break;
      ulParmSize = sizeof ulTimeOut;
      ulDataSize = sizeof szBuffer;

      rc = DosFSCtl(
         (PVOID)szBuffer, ulDataSize, &ulDataSize,
         (PVOID)&ulTimeOut, ulParmSize, &ulParmSize,
         FAT32_GETLOGDATA, "FAT32", -1, FSCTL_FSDNAME);

      if (rc == ERROR_NOT_SUPPORTED)
         {
         printf("Not supported function\n");
         break;
         }
      if (!rc || rc == 111)
         {
         if (!fSilent)
            printf(szBuffer);
         if (fLog)
            {
            fputs(szBuffer, fLog);
            if (fForce)
               {
               INT iHandle;
               fflush(fLog);
               iHandle = dup(fileno(fLog));
               close(iHandle);
               }
            }
         }
      else if (rc != 0x8001)
         {
         printf("SYS%4.4d\n", rc);
         break;
         }
      }

   fActive = FALSE;
   ulParmSize = sizeof fActive;
   rc = DosFSCtl(
      NULL, 0, &ulDataSize,
     (PVOID)&fActive, ulParmSize, &ulParmSize,
      FAT32_SETMESSAGE, "FAT32", -1, FSCTL_FSDNAME);

   fclose(fLog);
   return 0;
}


void CtrlCHandler(int signal)
{
BOOL fActive;
ULONG ulParmSize;
ULONG ulDataSize;
APIRET rc;

   fActive = FALSE;
   ulParmSize = sizeof fActive;
   ulDataSize = 0;
   rc = DosFSCtl(
      NULL, 0, &ulDataSize,
     (PVOID)&fActive, ulParmSize, &ulParmSize,
      FAT32_SETMESSAGE, "FAT32", -1, FSCTL_FSDNAME);

   printf("\nprogram aborted! rc = %ld\n", rc);
   exit(1);

   signal = signal;
}

