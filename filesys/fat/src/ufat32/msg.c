#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "fat32c.h"

PSZ GetOS2Error(USHORT rc)
{
static BYTE szErrorBuf[MAX_MESSAGE] = "";
static BYTE szErrNo[12] = "";
ULONG rc2;
ULONG ulReplySize;

   memset(szErrorBuf, 0, sizeof(szErrorBuf));
   if (rc)
      {
      sprintf(szErrNo, "SYS%4.4u: ", rc);
      rc2 = DosGetMessage(NULL, 0, szErrorBuf, sizeof(szErrorBuf),
                          rc, "OSO001.MSG", &ulReplySize);
      switch (rc2)
         {
         case NO_ERROR:
            strcat(szErrorBuf, "\n");
            break;
         case ERROR_FILE_NOT_FOUND :
            sprintf(szErrorBuf, "SYS%04u (Message file not found!)", rc);
            break;
         default:
            sprintf(szErrorBuf, "SYS%04u (Error %d while retrieving message text!)", rc, rc2);
            break;
         }
      DosGetMessage(NULL,
         0,
         szErrorBuf + strlen(szErrorBuf),
         sizeof(szErrorBuf) - strlen(szErrorBuf),
         rc,
         "OSO001H.MSG",
         &ulReplySize);
      }

   if (memicmp(szErrorBuf, "SYS", 3))
      {
      memmove(szErrorBuf + strlen(szErrNo), szErrorBuf, strlen(szErrorBuf) + 1);
      memcpy(szErrorBuf, szErrNo, strlen(szErrNo));
      }
   return szErrorBuf;
}

INT cdecl iShowMessage(PCDINFO pCD, USHORT usNr, USHORT usNumFields, ...)
{
static BYTE szErrNo[12] = "";
static BYTE szMessage[MAX_MESSAGE];
static BYTE szOut[MAX_MESSAGE];
static BYTE rgNum[9][50];
ULONG ulReplySize;
ULONG rc;
PSZ    rgPSZ[9];
va_list va;
USHORT usIndex;
PSZ    pszMess;

   va_start(va, usNumFields);

   memset(szMessage, 0, sizeof(szMessage));
   rc = DosGetMessage(NULL, 0, szMessage, sizeof(szMessage),
                          usNr, "OSO001.MSG", &ulReplySize);
   switch (rc)
      {
      case NO_ERROR:
         break;
      case ERROR_FILE_NOT_FOUND :
         sprintf(szMessage, "SYS%04u (Message file not found!)", usNr);
         break;
      default:
         sprintf(szMessage, "SYS%04u (Error %d while retrieving message text!)", usNr, rc);
         break;
      }
   if (rc)
      {
      printf("%s\n", szMessage);
      return 0;
      }

   pszMess = szMessage;

   memset(szOut, 0, sizeof(szOut));
   if (pCD && pCD->fPM)
      sprintf(szOut, "MSG_%u", usNr);

   memset(rgNum, 0, sizeof(rgNum));
   for (usIndex = 0; usIndex < usNumFields; usIndex++)
      {
      USHORT usType = va_arg(va, USHORT);
      if (usType == TYPE_PERC)
         {
         sprintf(rgNum[usIndex], "%3u", va_arg(va, USHORT));
         rgPSZ[usIndex] = rgNum[usIndex];
         }
      else if (usType == TYPE_LONG || usType == TYPE_LONG2)
         {
         if (!usIndex && usType == TYPE_LONG)
            sprintf(rgNum[usIndex], "%12lu", va_arg(va, ULONG));
         else
            sprintf(rgNum[usIndex], "%lu", va_arg(va, ULONG));
         rgPSZ[usIndex] = rgNum[usIndex];
         }
      else if (usType == TYPE_DOUBLE || usType == TYPE_DOUBLE2)
         {
         PSZ p;
         if (!usIndex && usType == TYPE_DOUBLE)
            sprintf(rgNum[usIndex], "%12.0lf", va_arg(va, double));
         else
            sprintf(rgNum[usIndex], "%lf", va_arg(va, double));
         p = strchr(rgNum[usIndex], '.');
         if (p)
            *p = 0;
         rgPSZ[usIndex] = rgNum[usIndex];
         }
      else if (usType == TYPE_STRING)
         {
         rgPSZ[usIndex] = va_arg(va, PSZ);
         }
      if (pCD && pCD->fPM)
         {
         PSZ p;
         strcat(szOut, ", ");
         p = rgPSZ[usIndex];
         while (*p == 0x20)
            p++;
         strcat(szOut, p);
         }
      }

   va_end( va );

   if (pCD && pCD->fPM)
      {
      strcat(szOut, "\n");
/*
      printf(szOut);
      return usNumFields;
*/
      }

   rc = DosInsertMessage(rgPSZ,
      usNumFields,
      pszMess,
      strlen(pszMess),
      szOut + strlen(szOut),
      sizeof(szOut) - strlen(szOut),
      &ulReplySize);
   printf("%s", szOut);

   return usNumFields;
}
