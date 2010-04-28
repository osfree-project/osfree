#define INCL_DOSSEMAPHORES   /* DOS semaphore values */

#define INCL_DOSERRORS       /* DOS error values */

#include <os2.h>

#include <stdio.h>



int main(VOID) {

 HMUX      hmuxHandAny = NULLHANDLE;      /* Muxwaithandle */

 HEV       hev[5]      = {0};             /* Event semaphores */

 SEMRECORD apsr[5]     = {{0}};           /* Semaphore records */

 APIRET    rc          = NO_ERROR;        /* Return code */

 ULONG     ulLoop      = 0;               /* Loop count */

 ULONG     ulSem       = 0;



for (ulLoop = 0; ulLoop < 5; ulLoop++) {

    rc = DosCreateEventSem((PSZ) NULL,

                           &hev[ulLoop],

                           0,

                           FALSE);

    if (rc != NO_ERROR) {

      printf("DosCreateEventSem error:  return code = %u\n", rc);

      return 1;

    }

    apsr[ulLoop].hsemCur = (HSEM) hev[ulLoop],

    apsr[ulLoop].ulUser = 0;

} /* endfor */



rc = DosCreateMuxWaitSem((PSZ) NULL,

                         &hmuxHandAny,

                         5L,              /* Number of semaphores in list */

                         apsr,            /* Semaphore list */

                         DCMW_WAIT_ANY);  /* Wait for any semaphore */

    if (rc != NO_ERROR) {

      printf("DosCreateMuxWaitSem error:  return code = %u\n", rc);

      return 1;

    }



rc = DosWaitMuxWaitSem(hmuxHandAny,

                       SEM_IMMEDIATE_RETURN,

                       &ulSem);           /* No semaphores have been posted, so

                                             we should see a timeout below...    */

    if (rc != ERROR_TIMEOUT) {

      printf("DosWaitMuxWaitSem error:  return code = %u\n", rc);

      return 1;

    }



rc = DosDeleteMuxWaitSem(hmuxHandAny, apsr[0].hsemCur);

    if (rc != NO_ERROR) {

      printf("DosDeleteMuxWaitSem error:  return code = %u\n", rc);

      return 1;

    }



rc = DosCloseMuxWaitSem(hmuxHandAny);

     if (rc != NO_ERROR) {

       printf("DosCloseMuxWaitSem error: return code = %u\n", rc);

       return 1;

     }



return NO_ERROR;

}
