#define INCL_DOSSEMAPHORES   /* DOS semaphore values */
#define INCL_DOSERRORS       /* DOS error values */
#include <os2.h>
#include <stdio.h>

int main(VOID) {
 HMUX      hmuxHandAny = NULLHANDLE;       /* Muxwaithandle */
 HEV       hevA[2]      = {0};             /* Event semaphores */
 SEMRECORD apsrA[2]     = {{0}};           /* Semaphore records */
 HEV       hevB[2]      = {0};             /* Event semaphores */
 SEMRECORD apsrB[2]     = {{0}};           /* Semaphore records */
 APIRET    rc          = NO_ERROR;         /* Return code */
 ULONG     ulLoop      = 0;                /* Loop count */

for (ulLoop = 0; ulLoop < 2; ulLoop++) {
    rc = DosCreateEventSem((PSZ) NULL,
                           &hevA[ulLoop],
                           0,
                           FALSE);
    if (rc != NO_ERROR) {
      printf("DosCreateEventSem error:  return code = %u\n", rc);
      return 1;
    }
    apsrA[ulLoop].hsemCur = (HSEM) hevA[ulLoop],
    apsrA[ulLoop].ulUser = 0;

    rc = DosCreateEventSem((PSZ) NULL,
                           &hevB[ulLoop],
                           0,
                           FALSE);
    if (rc != NO_ERROR) {
      printf("DosCreateEventSem error:  return code = %u\n", rc);
      return 1;
    }
    apsrB[ulLoop].hsemCur = (HSEM) hevB[ulLoop],
    apsrB[ulLoop].ulUser = 0;
} /* endfor */

rc = DosCreateMuxWaitSem((PSZ) NULL,
                         &hmuxHandAny,
                         2L,             /* Number of semaphores in list */
                         apsrA,          /* Semaphore list */
                         DCMW_WAIT_ANY); /* Wait for any semaphore */
    if (rc != NO_ERROR) {
      printf("DosCreateMuxWaitSem error:  return code = %u\n", rc);
      return 1;
    }

rc = DosAddMuxWaitSem(hmuxHandAny,  apsrB);
    if (rc != NO_ERROR) {
      printf("DosAddMuxWaitSem error:  return code = %u\n", rc);
      return 1;
    }

return NO_ERROR;
}
