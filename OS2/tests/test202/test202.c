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

   CHAR     outmsg[256]            = "";         /* Output message buffer */

   CHAR     inmsg[256]             = "";         /* Input message buffer */

   HFILE    PipeHandle             = NULLHANDLE; /* Pipe handle */

   PIPEINFO PipeBuffer[4]          = {{0}};

   struct   _AVAILDATA  BytesAvail = {0};

   UCHAR    Buffer[200]            = {0};

   ULONG    bytes                  = 0;

   ULONG    Action                 = 0;

   PIPESEMSTATE infobuf[3]         = {{0}};



   rc = DosOpen("\\PIPE\\EXAMPLE", &PipeHandle, &Action, 0, 0, FILE_OPEN,

                OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE |

                OPEN_FLAGS_FAIL_ON_ERROR, NULL);

   if (rc != NO_ERROR) {

      printf("DosOpen error: error code = %u\n", rc);

      return 1;

   }  else printf("Connected to Pipe.\n");



   printf("Enter message to send to PIPEHOST: ");



   fflush(NULL);   /* Force above printf to output device */

   gets(outmsg);



   rc = DosTransactNPipe(PipeHandle,          /* Handle of duplex pipe */

                         outmsg,              /* Output message buffer */

                         strlen(outmsg),      /* Size of output message */

                         inmsg,               /* Input message buffer */

                         sizeof(inmsg),       /* SIze of input buffer */

                         &bytes);             /* Number of bytes read */

   if (rc != NO_ERROR) {

      printf("DosTransactNPipe error: error code = %u\n", rc);

      return 1;

   }



   printf("\nMessage received from PIPEHOST: %s\n\n", inmsg);



   printf("...Disconnected\n");

   return NO_ERROR;

}
