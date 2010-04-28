#define INCL_DOSQUEUES   /* DOS Queue values */

#define INCL_DOSPROCESS  /* DOS thread and process values */

#define INCL_DOSERRORS   /* DOS error values */

#include <os2.h>

#include <stdio.h>

#include <string.h>



int main(USHORT argc, PCHAR argv[]) {



   PSZ         szQueueName  = "\\QUEUES\\OF\\DATA\\WAITING\\FOR\\SERVICE";

   HQUEUE      hqSpecialQue = NULLHANDLE; /* Queue handle                   */

   REQUESTDATA Request      = {0};        /* Reques */

   PID         pidOwner     = 0;

   APIRET      rc           = NO_ERROR;   /* Return code                    */



   rc = DosCreateQueue(&hqSpecialQue,    /* Queue handle                    */

             QUE_FIFO |                  /* First-In First-Out order        */

             QUE_CONVERT_ADDRESS,        /* Convert 16-bit addresses to 32  */

             szQueueName);               /* Name of the queue to create     */



   if (rc!= NO_ERROR) {

      printf ("DosCreateQueue error: return code = %u\n", rc);

      return 1;       }



   rc = DosOpenQueue (&pidOwner,         /* PID of queue owner              */

                      &hqSpecialQue,     /* Handle for created queue        */

                      szQueueName);      /* Name of the queue to open       */



   if (rc!= NO_ERROR) {

      printf ("DosOpenQueue error: return code = %u\n", rc);

      return 1;       }



           /* Kill the queue owner (which is us) */



   rc = DosKillProcess(0, pidOwner);

   if (rc != NO_ERROR) {

      printf("DosKillProcess error: return code = %u\n", rc);

      return 1;

   }



   rc = DosSleep(45000L);    /* Dead code */



   return NO_ERROR;

}
