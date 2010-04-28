#define INCL_DOSFILEMGR       /* DOS File Manager values */

#define INCL_DOSNMPIPES       /* DOS Named Pipes values */

#define INCL_DOSSEMAPHORES    /* DOS Semaphore values */

#define INCL_DOSERRORS        /* DOS Error values */

#include <os2.h>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>



int main(VOID) {

   APIRET   rc                     = NO_ERROR;   /* Return code */

   CHAR     message[256]           = "";         /* Message buffer */

   HFILE    PipeHandle             = NULLHANDLE; /* Pipe handle */

   HEV      hev                    = NULLHANDLE; /* Event semaphore handle */

   PIPEINFO PipeBuffer[4]          = {{0}};

   struct   _AVAILDATA  BytesAvail = {0};

   UCHAR    Buffer[200]            = {0};

   ULONG    bytes                  = 0;

   ULONG    Action                 = 0;

   ULONG    ulPostCt               = 0;

   int      i                      = 0;

   PIPESEMSTATE infobuf[3]         = {{0}};



   rc = DosOpen("\\PIPE\\EXAMPLE", &PipeHandle, &Action, 0, 0, FILE_OPEN,

                OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE |

                OPEN_FLAGS_FAIL_ON_ERROR, NULL);

   if (rc != NO_ERROR) {

      printf("DosOpen error: error code = %u\n", rc);

      return 1;

   }  else printf("Connected to Pipe.\n");



   rc = DosOpenEventSem("\\SEM32\\PIPE\\EXAMPLE", &hev);

   if (rc != NO_ERROR) {

      printf("DosOpenEventSem error: error code = %u\n", rc);

      return 1;

   }



   printf("Enter message to send to PIPEHOST: ");



   fflush(NULL);  /* Make printf appear on display */

   gets(message);



   rc = DosWrite(PipeHandle, message, strlen(message), &bytes);

   if (rc != NO_ERROR) {

      printf("DosWrite error: error code = %u\n", rc);

      return 1;

   }

   rc = DosRead(PipeHandle, message, sizeof(message), &bytes);

   if (rc != NO_ERROR) {

      printf("DosRead error: error code = %u\n", rc);

      return 1;

   }



   printf("\nMessage received from PHOST: %s\n\n", message);



   rc = DosQueryEventSem(hev, &ulPostCt);

   if (rc != NO_ERROR) {

      printf("DosQueryEventSem error: return code = %u\n", rc);

      return 1;

    } else printf("Current post count value is %u\n", ulPostCt);



   rc = DosCloseEventSem(hev);

   if (rc != NO_ERROR) {

      printf("DosCloseEventSem error: error code = %u\n", rc);

      return 1;

   }



   printf("...Disconnected\n");

   return NO_ERROR;

}
