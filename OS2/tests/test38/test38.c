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



   printf("Enter message to send to PIPEHOST: ");



   fflush(NULL);   /* Make above printf show on display */

   gets(outmsg);



   rc = DosCallNPipe("\\PIPE\\EXAMPLE",   /* Name of duplex pipe */

                      outmsg,             /* Output message buffer */

                      strlen(outmsg),     /* Size of output message */

                      inmsg,              /* Input message buffer */

                      sizeof(inmsg),      /* Size of input buffer */

                      &bytes,             /* Number of bytes read */

                      30000L);            /* Wait 30 seconds for pipe */

   if (rc != NO_ERROR) {

      printf("DosCallNPipe error: error code = %u\n", rc);

      return 1;

   } else {

      printf("\nMessage received from PIPEHOST: %s\n\n", inmsg);

   } /* endif */



   return NO_ERROR;

}
