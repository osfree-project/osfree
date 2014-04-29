#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <dos.h>

#define INCL_DOS
#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_DOSERRORS

#include "os2.h"
#include "portable.h"
#include "fat32ifs.h"

#define LOGBUF_SIZE 8192

static BYTE szLogBuf[LOGBUF_SIZE] = "";

static BOOL fData = FALSE;
static BYTE szLost[]="(information lost)\r\n";

static BOOL fWriteLogging(PSZ pszMessage);

VOID cdecl Message(PSZ pszMessage, ...)
{
static BYTE szMessage[512];
va_list va;
PROCINFO Proc;
ULONG ulmSecs = *pGITicks;

   if (!f32Parms.fMessageActive)
      return;

   va_start(va, pszMessage);

   GetProcInfo(&Proc, sizeof Proc);
   memset(szMessage, 0, sizeof szMessage);
   sprintf(szMessage, "P:%X U:%X D:%X T:%u:%u ",
      Proc.usPid,
      Proc.usUid,
      Proc.usPdb,
      (USHORT)((ulmSecs / 1000) % 60),
      (USHORT)(ulmSecs % 1000));

   vsprintf(szMessage + strlen(szMessage), pszMessage, va);
   fWriteLogging(szMessage);
}

BOOL fWriteLogging(PSZ pszMessage)
{
USHORT usNeeded;
USHORT usSize;
PSZ    p1, p2;

   usNeeded = strlen(pszMessage) + 2;
   usSize = strlen(szLogBuf);
   if (usSize + usNeeded >= LOGBUF_SIZE)
      {
      usNeeded += strlen(szLost);
      p2 = szLogBuf;
      while (usSize + usNeeded >= LOGBUF_SIZE)
         {
         p1 = strchr(p2, '\n');
         if (p1)
            {
            usSize -= ((p1 - p2) + 1);
            p2 = p1 + 1;
            }
         else
            return TRUE;
         }

      memcpy(szLogBuf, szLost, strlen(szLost));
      memmove(szLogBuf + strlen(szLost), p2, strlen(p2) + 1);
      }

   strcat(szLogBuf + usSize, pszMessage);
   strcat(szLogBuf + usSize, "\r\n");

   fData = TRUE;
   DevHelp_ProcRun((ULONG)GetLogBuffer, &usSize);
   return TRUE;
}

USHORT GetLogBuffer(PBYTE pData, USHORT cbData, ULONG ulTimeOut)
{
USHORT rc;

   if (f32Parms.fInShutDown)
      return ERROR_ALREADY_SHUTDOWN;

   rc = MY_PROBEBUF(PB_OPWRITE, pData, cbData);
   if (rc)
      {
      Message("FAT32: Protection VIOLATION in GetLogBuffer! (SYS%d)", rc);
      return rc;
      }

   _disable();
   while (!fData && !f32Parms.fInShutDown)
      {
      rc = DevHelp_ProcBlock((ULONG)GetLogBuffer, ulTimeOut, WAIT_IS_INTERRUPTABLE);
      if (rc)
         {
         _enable();
         return rc;
         }
      _disable();
      }
   _enable();

   if (f32Parms.fInShutDown)
      return ERROR_ALREADY_SHUTDOWN;

   strncpy(pData, szLogBuf, cbData);
   if (strlen(szLogBuf) > cbData)
      {
      memmove(szLogBuf, szLogBuf + cbData, strlen(szLogBuf + cbData) + 1);
      rc = ERROR_BUFFER_OVERFLOW;
      }
   else
      {
      *szLogBuf = 0;
      fData = FALSE;
      rc = 0;
      }

   return rc;
}


