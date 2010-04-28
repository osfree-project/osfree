#define INCL_DOSQUEUES   /* DOS Queue values */

#define INCL_DOSPROCESS  /* DOS thread and process values */

#define INCL_DOSERRORS   /* DOS error values */

#include <os2.h>

#include <stdio.h>

#include <string.h>



int main(VOID) {



   PSZ         szQueueName  = "\\QUEUES\\AnyQueueNameHere";

   HQUEUE      hqAnyQueue   = NULLHANDLE; /* Queue handle                   */

   PSZ         DataBuffer   = "";         /* Data buffer for queue data     */

   REQUESTDATA Request      = {0};        /* Request                        */

   ULONG       ulDataLen    = 0,          /* Length of data returned        */

               ulElemCode   = 0;          /* Element code (input/output)    */

   BYTE        ElemPrty     = 0;          /* Priority of element (returned) */

   APIRET      rc           = NO_ERROR;   /* Return code                    */

   rc = DosCreateQueue(&hqAnyQueue,      /* Queue handle                    */

             QUE_FIFO |                  /* First-In First-Out order        */

             QUE_CONVERT_ADDRESS,        /* Convert 16-bit addresses to 32  */

             szQueueName);               /* Name of the queue to create     */

   if (rc!= NO_ERROR) {

      printf ("DosCreateQueue error: return code = %u\n", rc);

      return 1;

   }



   DataBuffer = "Start of the data... Middle of data...  Data ends.";

   rc = DosWriteQueue (hqAnyQueue,          /* Queue to write to           */

                       87654321L,           /* Request data                */

                       sizeof(DataBuffer),  /* Length of data to write     */

                       DataBuffer,          /* Pointer to data             */

                       0L);              /* Priority (not applicable here) */

   if (rc!= NO_ERROR) {

      printf ("DosWriteQueue error: return code = %u\n", rc);

      return 1;

   }



   DataBuffer = "";                          /* Clear the DataBuffer        */

   rc = DosPeekQueue (hqAnyQueue,            /* Handle of queue             */

                      &Request,              /* Request data for element    */

                      &ulDataLen,            /* Length of data returned     */

                      (PVOID) &DataBuffer,   /* Data returned               */

                      &ulElemCode,           /* Input: 0 reads next element

                                                Output:  peeked element id  */

                      DCWW_WAIT,             /* Wait for data               */

                      &ElemPrty,             /* Priority of element         */

                      0L);                   /* Semaphore (not used here)   */

  if (rc != NO_ERROR) {

     printf ("DosPeekQueue error : return code = %u\n", rc);

     return 1;

  } else {

     printf ("DosPeekQueue returns: '%s'\n", DataBuffer);

     printf ("  (Request data = %u)\n", Request.ulData);

  }



  rc = DosCloseQueue(hqAnyQueue);         /* Close the queue */

  if (rc!= NO_ERROR) {

     printf ("DosCloseQueue error: return code = %u\n", rc);

     return 1;

  }



  return NO_ERROR;

}
