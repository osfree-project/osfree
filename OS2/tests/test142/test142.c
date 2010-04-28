#define INCL_DOSSEMAPHORES   /* DOS semaphore values */

#define INCL_DOSERRORS       /* DOS error values */

#include <os2.h>

#include <stdio.h>



int main(VOID) {

 HMUX       hmuxHandAny  = NULLHANDLE;   /* Muxwaithandle */

 HEV        hev[2]       = {0};          /* Event semaphores */

 SEMRECORD  apsr[2]      = {{0}};        /* Semaphore records */

 SEMRECORD  semrecQuery[2] = {{0}};        /* Pointer from query */

 ULONG      cQueryRec    = 2;            /* Number of records found by query */

 ULONG      fQueryFlags  = 0;            /* Attribute flags returned by query */

 APIRET     rc           = NO_ERROR;     /* Return code */

 ULONG      ulLoop       = 0;            /* Loop count */

 ULONG      ulSem        = 0;



for (ulLoop = 0; ulLoop < 2; ulLoop++) {

    rc = DosCreateEventSem((PSZ) NULL, &hev[ulLoop], 0, FALSE);

    if (rc != NO_ERROR) {

      printf("DosCreateEventSem error:  return code = %u\n", rc);

      return 1;

    }

    apsr[ulLoop].hsemCur = (HSEM) hev[ulLoop],

    apsr[ulLoop].ulUser = 0;

} /* endfor */



rc = DosCreateMuxWaitSem((PSZ) NULL, &hmuxHandAny, 2L, apsr, DCMW_WAIT_ANY);

if (rc != NO_ERROR) {

   printf("DosCreateMuxWaitSem error:  return code = %u\n", rc);

   return 1;

}

           /* Query information about the MuxWait semaphore */



rc = DosQueryMuxWaitSem(hmuxHandAny,    /* Semaphore handle */

                        &cQueryRec,     /* Number of records */

                        (PSEMRECORD) semrecQuery,  /* Pointer to Semrecords */

                        &fQueryFlags);  /* Flags returned */

if (rc != NO_ERROR) {

  printf("DosQueryMuxWaitSem error:  return code = %u\n", rc);

  return 1;

} else {

  printf("DosQueryMuxWaitSem found %u semaphore records\n", cQueryRec);

} /* endif */



return NO_ERROR;

}
