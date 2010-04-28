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

   PIPEINFO PipeBuffer[4]          = {{0}};

   struct   _AVAILDATA  BytesAvail = {0};

   UCHAR    Buffer[200]            = {0};

   ULONG    bytes                  = 0;

   ULONG    Action                 = 0;

   ULONG    PipeState              = 0;

   ULONG    HandType               = 0;

   ULONG    FlagWord               = 0;

   ULONG    BytesRead              = 0;



   rc = DosOpen("\\PIPE\\EXAMPLE", &PipeHandle, &Action, 0, 0, FILE_OPEN,

                OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE |

                OPEN_FLAGS_FAIL_ON_ERROR, NULL);

   if (rc != NO_ERROR) {

      printf("DosOpen error: error code = %u\n", rc);

      return 1;

   }  else printf("Connected to pipe.\n");

   rc = DosQueryHType(PipeHandle, &HandType, &FlagWord);

   if (rc != NO_ERROR) {

      printf("DosQueryHType error: error code = %u\n", rc);

      return 1;

   }  else printf("Handle type value is %u\n", HandType);



   rc = DosPeekNPipe(PipeHandle, Buffer, sizeof(Buffer),

                     &BytesRead, &BytesAvail, &PipeState);

   if (rc != NO_ERROR) {

      printf("DosPeekNPipe error: error code = %u\n", rc);

      return 1;

   }  else printf("Pipe status value is %u\n\n", PipeState);



   printf("Enter message to send to PIPEHOST: ");



   fflush(NULL);   /* Flush above printf out to display */

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



   printf("\nMessage received from PIPEHOST: %s\n\n", message);



   rc = DosClose(PipeHandle);

   /* Should check if (rc != NO_ERROR) here... */



   printf("...Disconnected\n");

   return NO_ERROR;

}

