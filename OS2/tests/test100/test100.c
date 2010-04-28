#define INCL_DOSSEMAPHORES      /* Semaphore values */

#define INCL_DOSERRORS          /* DOS Error values */

#include <os2.h>

#include <stdio.h>



int main(VOID) {

HMTX    hmtx     = NULLHANDLE; /* Mutex semaphore handle */

PID     pidOwner = 0;          /* PID of current mutex semaphore owner */

TID     tidOwner = 0;          /* TID of current mutex semaphore owner */

ULONG   ulCount  = 0;          /* Request count for the semaphore */

APIRET  rc       = NO_ERROR;   /* Return code */



   rc = DosCreateMutexSem("\\SEM32\\MUTEX1",      /* Semaphore name */

                          &hmtx, 0, FALSE);       /* Handle returned */

   if (rc != NO_ERROR) {

      printf("DosOpenMutexSem error: return code = %u\n", rc);

      return 1;

    }

        /* This would normally be done by another unit of work */

   rc = DosOpenMutexSem("\\SEM32\\MUTEX1",      /* Semaphore name */

                        &hmtx);                 /* Handle returned */

   if (rc != NO_ERROR) {

      printf("DosOpenMutexSem error: return code = %u\n", rc);

      return 1;

    }



   rc = DosRequestMutexSem(hmtx,                     /* Handle of semaphore */

                           (ULONG) SEM_INDEFINITE_WAIT);  /* Timeout (none) */

   if (rc != NO_ERROR) {

      printf("DosRequestMutexSem error: return code = %u\n", rc);

      return 1;

   }



   rc = DosQueryMutexSem(hmtx,         /* Handle of semaphore */

                         &pidOwner,    /* Process ID of owner */

                         &tidOwner,    /* Thread ID of owner */

                         &ulCount);    /* Count */

   if (rc != NO_ERROR) {

      printf("DosQueryMutexSem error: return code = %u\n", rc);

      return 1;

   } else  {

      printf("Semaphore owned by PID %u, TID %u.", pidOwner, tidOwner);

      printf("  Request count is %u.\n", ulCount);

   } /* endif */



   rc = DosReleaseMutexSem(hmtx);        /* Relinquish ownership */

   if (rc != NO_ERROR) {

      printf("DosReleaseMutexSem error: return code = %u\n", rc);

      return 1;

    }



   rc = DosCloseMutexSem(hmtx);          /* Close mutex semaphore */

   if (rc != NO_ERROR) {

      printf("DosCloseMutexSem error: return code = %u\n", rc);

      return 1;

   }



return NO_ERROR;

}
