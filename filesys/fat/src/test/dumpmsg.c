#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>

#define MAX_MESSAGE 2048

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

#include "portable.h"

PSZ GetOS2Error(USHORT rc);

int main(INT iArgc, PSZ Argv[])
{
USHORT rc;

   for (rc = 1; rc < 5000; rc++)
      {
      PSZ p = GetOS2Error(rc);
      if (p)
         printf("%s", p);
      }
   return 0;
}

PSZ GetOS2Error(USHORT rc)
{
static BYTE szErrorBuf[MAX_MESSAGE] = "";
static BYTE szErrNo[12] = "";
USHORT rc2;
USHORT usReplySize;

   memset(szErrorBuf, 0, sizeof szErrorBuf);
   if (rc)
      {
      sprintf(szErrNo, "SYS%4.4u: ", rc);
      rc2 = DosGetMessage(NULL, 0, szErrorBuf, sizeof(szErrorBuf),
                          rc, "OSO001.MSG", &usReplySize);
      switch (rc2)
         {
         case NO_ERROR:
            break;
         case ERROR_FILE_NOT_FOUND :
            sprintf(szErrorBuf, "SYS%04u (Message file not found!)", rc);
            break;
         default:
            return NULL;
            sprintf(szErrorBuf, "SYS%04u (Error %d while retrieving message text!)", rc, rc2);
            break;
         }
      }

   if (memicmp(szErrorBuf, "SYS", 3))
      {
      memmove(szErrorBuf + strlen(szErrNo), szErrorBuf, strlen(szErrorBuf) + 1);
      memcpy(szErrorBuf, szErrNo, strlen(szErrNo));
      }
   return szErrorBuf;
}

