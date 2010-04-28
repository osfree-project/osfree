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



   fflush(NULL);  /* Flush printf buffer */

   gets(message);



   rc = DosWrite(PipeHandle, message, strlen(message), &bytes);

   if (rc != NO_ERROR) {

      printf("DosWrite error: error code = %u\n", rc);

      return 1;

   }



   rc = DosQueryNPipeSemState((HSEM) hev, infobuf, sizeof(PIPESEMSTATE)*3);

   if (rc != NO_ERROR) {

     printf("DosQueryNPipeSemState error: return code = %u\n",rc);

     return 1;

   }



   printf("Status  Flag   Key   Avail\n------ ------ ------ ------\n");

   for (i=0; i<3; i++)

     printf("%6u %6u %6u %6u\n", infobuf[i].fStatus,

            infobuf[i].fFlag, infobuf[i].usKey, infobuf[i].usAvail);



   rc = DosRead(PipeHandle, message, sizeof(message), &bytes);

   if (rc != NO_ERROR) {

      printf("DosRead error: error code = %u\n", rc);

      return 1;

   }



   printf("\nMessage received from PIPEHOST: %s\n\n", message);



   rc = DosCloseEventSem(hev);

   /* Should check if (rc != NO_ERROR) here... */



   rc = DosClose(PipeHandle);

   /* Should check if (rc != NO_ERROR) here... */



   printf("...Disconnected\n");

   return NO_ERROR;

}
