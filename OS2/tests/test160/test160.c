 #define INCL_DOSSEMAPHORES   /* Semaphore values */

 #define INCL_DOSDATETIME     /* Timer support    */

 #define INCL_DOSERRORS       /* DOS error values */

 #include <os2.h>

 #include <stdio.h>



 int main(VOID) {



 HEV     hevEvent1     = 0;                   /* Event semaphore handle    */

 HTIMER  htimerEvent1  = 0;                   /* Timer handle              */

 APIRET  rc            = NO_ERROR;            /* Return code               */

 ULONG   ulPostCount   = 0;                   /* Semaphore post count      */

 ULONG   i             = 0;                   /* A loop index              */



    rc = DosCreateEventSem(NULL,           /* Unnamed semaphore            */

                           &hevEvent1,     /* Handle of semaphore returned */

                           DC_SEM_SHARED,  /* Indicate a shared semaphore  */

                           FALSE);         /* Put in RESET state           */

    if (rc != NO_ERROR) {

        printf("DosCreateEventSem error: return code = %u\n", rc);

        return 1;

    }



    rc = DosStartTimer(2000L,              /* 2 second interval            */

                       (HSEM) hevEvent1,   /* Semaphore to post            */

                       &htimerEvent1);     /* Timer handler (returned)     */

    if (rc != NO_ERROR) {

        printf("DosStartTimer error: return code = %u\n", rc);

        return 1;

    }



    for (i = 1 ; i < 6 ; i++) {



      rc = DosWaitEventSem(hevEvent1,15000L); /* Wait 15 seconds for timer */

      if (rc != NO_ERROR) {

          printf("DosWaitEventSem error: return code = %u\n", rc);

          return 1;

      }



      rc = DosResetEventSem(hevEvent1,       /* Reset the semaphore         */

                            &ulPostCount);   /* And get count (should be 1) */

      if (rc != NO_ERROR) {

          printf("DosWaitEventSem error: return code = %u\n", rc);

          return 1;

      }



      printf("Iteration %u: ulPostCount = %u\n", i, ulPostCount);



    } /* for loop */

    rc = DosStopTimer(htimerEvent1);       /* Stop the timer             */

    if (rc != NO_ERROR) {

        printf("DosCloseEventSem error: return code = %u\n", rc);

        return 1;

    }



    rc = DosCloseEventSem(hevEvent1);      /* Get rid of semaphore       */

    if (rc != NO_ERROR) {

        printf("DosCloseEventSem error: return code = %u\n", rc);

        return 1;

    }



 return NO_ERROR;

}
