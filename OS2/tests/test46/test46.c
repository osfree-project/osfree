#define INCL_BASE

#define INCL_DOSSEMAPHORES

#define INCL_DOSNMPIPES

#include <os2.h>

#include <stdio.h>

#include <string.h>



int main(VOID) {

CHAR     PipeName[256] = "\\PIPE\\EXAMPLE" ;     /* Pipe name */

HPIPE    PipeHandle    = NULLHANDLE;             /* Pipe handle */

HEV      hev           = NULLHANDLE;             /* Semaphore handle */

ULONG    ulBytes       = 0;                      /* Bytes read or written */

CHAR     message[256]  = "";                     /* Input/Output buffer */

APIRET   rc            = NO_ERROR;               /* Return code */



   rc = DosCreateNPipe(PipeName,                /* Name of pipe to create */

                       &PipeHandle,             /* Handle returned for pipe */

                       NP_ACCESS_DUPLEX,        /* Duplex pipe */

                       NP_WAIT |

                       NP_TYPE_MESSAGE |

                       NP_READMODE_MESSAGE |

                       NP_WMESG |               /* Write messages */

                       NP_RMESG |               /* Read messages */

                       0x01,                    /* Unique instance of pipe */

                       sizeof(message),         /* Output buffer size */

                       sizeof(message),         /* Input buffer size */

                       0L);                     /* Use default time-out */

   if (rc != NO_ERROR) {

     printf("DosCreateNPipe error: return code = %u\n",rc);

     return 1;

   }



   rc = DosCreateEventSem("\\SEM32\\PIPE\\EXAMPLE", &hev, 0L, 0L);

   /* Should check if (rc != NO_ERROR)  here... This semaphore is not

      always used.                                                     */



   rc = DosSetNPipeSem(PipeHandle,       /* Handle for pipe */

                       (HSEM) hev,       /* Handle of semaphore */

                       1L);              /* Used to distinguish among events */

   if (rc != NO_ERROR) {

     printf("DosSetNPipeSem error: return code = %u\n",rc);

     return 1;

   }



   printf("Waiting for connection to pipe %s...\n",PipeName);



   rc = DosConnectNPipe(PipeHandle);

   if (rc != NO_ERROR) {

     printf("DosConnectNPipe error: return code = %u\n",rc);

     return 1;

   }



   printf("\nCONNECTED\nWaiting for a message...\n");

   rc = DosRead(PipeHandle,            /* Handle of pipe */

                message,               /* Buffer for message read */

                sizeof(message),       /* Buffer size */

                &ulBytes);             /* Number of bytes actually read */

   if (rc != NO_ERROR) {

     printf("DosRead error: return code = %u\n",rc);

     return 1;

   }



   printf("\n\nMessage received was: %s\n\n", message);



   strcpy(message, "Thank you for your message!");

   rc = DosWrite(PipeHandle,           /* Handle of pipe */

                 message,              /* Buffer containing message to write */

                 strlen(message),      /* Length of message */

                 &ulBytes);            /* Number of bytes actually written */

   if (rc != NO_ERROR) {

     printf("DosWrite error: return code = %u\n",rc);

     return 1;

   }



   rc = DosCloseEventSem(hev);

   /* Should check if (rc != NO_ERROR)  here... */



   rc = DosDisConnectNPipe(PipeHandle);

   /* Should check if (rc != NO_ERROR)  here... */



   return NO_ERROR;

}
