#define INCL_DOSQUEUES   /* DOS Queue values */

#define INCL_DOSERRORS   /* DOS Error values */

#include <os2.h>

#include <stdio.h>

#include <string.h>



#define QUE_NAME "\\QUEUES\\PANDAWRITE\\LOCALQUEUE"



int main(VOID) {



   HQUEUE QueueHandle = NULLHANDLE;  /* Queue handle                   */

   CHAR   *DataBuffer = "";          /* Data to write to queue         */

   ULONG  ulNumElems  = 0L;          /* Number of elements on queue    */

   APIRET rc          = NO_ERROR;    /* Return code                    */



   rc = DosCreateQueue(&QueueHandle, /* Queue handle                   */

           QUE_LIFO |                /* Last In, First Out ordering    */

           QUE_CONVERT_ADDRESS,      /* Do 16-bit to 32-bit conversion */

           QUE_NAME);                /* Name of the queue              */

   if (rc!= NO_ERROR) {

      printf ("DosCreateQueue error: return code = %u\n", rc);

      return 1;

   }



   DataBuffer = "Element 1 of 2";

   rc = DosWriteQueue (QueueHandle, 100L, sizeof(DataBuffer),

                       (PVOID)DataBuffer, 0L);

   if (rc!= NO_ERROR) {

      printf ("DosWriteQueue error: return code = %u\n", rc);

      return 1;

   }



   rc = DosQueryQueue (QueueHandle, &ulNumElems);

   if (rc != NO_ERROR) {

      printf ("DosQueryQueue error: return code = %u\n", rc);

      return 1;

   } else { printf ("DosQueryQueue: %u elements\n", ulNumElems); }



   DataBuffer = "Element 2 of 2";

   rc = DosWriteQueue (QueueHandle, 200L, sizeof(DataBuffer),

                       (PVOID)DataBuffer, 0L);

   if (rc!= NO_ERROR) {

      printf ("DosWriteQueue error: return code = %u\n", rc);

      return 1;

   }



   rc = DosQueryQueue (QueueHandle, &ulNumElems);

   if (rc != NO_ERROR) {

      printf ("DosQueryQueue error: return code = %u\n", rc);

      return 1;

   } else { printf ("DosQueryQueue: %u elements\n", ulNumElems); }



   rc = DosPurgeQueue (QueueHandle);

   if (rc != NO_ERROR) {

      printf ("DosPurgeQueue error: return code = %u\n", rc);

      return 1;        }

   rc = DosQueryQueue (QueueHandle, &ulNumElems);

   if (rc != NO_ERROR) {

      printf ("DosQueryQueue error: return code = %u\n", rc);

      return 1;

   } else { printf ("DosQueryQueue: %u elements\n", ulNumElems); }



   rc = DosCloseQueue(QueueHandle);      /* Close the queue */

   if (rc!= NO_ERROR) {

      printf ("DosCloseQueue error: return code = %u\n", rc);

      return 1;       }



   return NO_ERROR;

}
