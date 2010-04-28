#define INCL_DOSQUEUES   /* DOS Queue values */

#define INCL_DOSPROCESS  /* DOS thread and process values */

#define INCL_DOSERRORS   /* DOS error values */

#include <os2.h>

#include <stdio.h>

#include <string.h>



int main(VOID) {



   PSZ         szQueueName  = "\\QUEUES\\SPECIAL.QUE";

   HQUEUE      hqSpecialQue = NULLHANDLE; /* Queue handle                   */

   PSZ         DataBuffer   = "";         /* Data buffer for queue data     */

   REQUESTDATA Request      = {0};        /* Reques */

   PID         pidOwner     = 0;

   ULONG       ulDataLen    = 0;          /* Length of data returned        */

   BYTE        ElemPrty     = 0;          /* Priority of element (returned) */

   APIRET      rc           = NO_ERROR;   /* Return code                    */



   rc = DosCreateQueue(&hqSpecialQue,    /* Queue handle                    */

             QUE_FIFO |                  /* First-In First-Out order        */

             QUE_CONVERT_ADDRESS,        /* Convert 16-bit addresses to 32  */

             szQueueName);               /* Name of the queue to create     */

   if (rc!= NO_ERROR) {

      printf ("DosCreateQueue error: return code = %u\n", rc);

      return 1;

   }



   rc = DosOpenQueue (&pidOwner,         /* PID of queue owner              */

                      &hqSpecialQue,     /* Handle for created queue        */

                      szQueueName);      /* Name of the queue to open       */

   if (rc!= NO_ERROR) {

      printf ("DosOpenQueue error: return code = %u\n", rc);

      return 1;

   }



   DataBuffer = "To be, or not to be.  That is the question...";

   rc = DosWriteQueue (hqSpecialQue,        /* Queue to write to           */

                       12345L,              /* Request data                */

                       sizeof(DataBuffer),  /* Length of data to write     */

                       DataBuffer,          /* Pointer to data             */

                       0L);              /* Priority (not applicable here) */

   if (rc!= NO_ERROR) {

      printf ("DosWriteQueue error: return code = %u\n", rc);

      return 1;

   }



   DataBuffer = "";                  /* Clear the DataBuffer */

   Request.pid = pidOwner;           /* process ID for the DosReadQueue */



   rc = DosReadQueue (hqSpecialQue,          /* Queue to read from          */

                      &Request,              /* Request data from write     */

                      &ulDataLen,            /* Length of data returned     */

                      (PVOID) &DataBuffer,   /* The data                    */

                      0L,                    /* Remove first element        */

                      DCWW_WAIT,             /* Wait for available data     */

                      &ElemPrty,             /* Priority of data (returned) */

                      0L);             /* Semaphore to use when not waiting */

   if (rc!= NO_ERROR) {

      printf ("DosReadQueue error: return code = %u\n", rc);

      return 1;

   } else {

      printf ("DosReadQueue returns: '%s'\n", DataBuffer);

      printf ("      (Request data = %u)\n", Request.ulData);

   }



   rc = DosCloseQueue(hqSpecialQue);         /* Close the queue */

   if (rc!= NO_ERROR) {

      printf ("DosCloseQueue error: return code = %u\n", rc);

      return 1;

   }



   return NO_ERROR;

}
