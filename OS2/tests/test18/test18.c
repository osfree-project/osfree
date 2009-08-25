 #define INCL_DOSSEMAPHORES   /* Semaphore values */
 #define INCL_DOSDATETIME     /* Timer support    */
 #define INCL_DOSERRORS       /* DOS error values */
 #include <os2.h>
 #include <stdio.h>

 int main(VOID) {

 PSZ     szSemName  = "\\SEM32\\TIMER\\THREAD1\\EVENT1"; /* Semaphore name */
 HEV     hevEvent1     = 0;                   /* Event semaphore handle    */
 HTIMER  htimerEvent1  = 0;                   /* Timer handle              */
 APIRET  rc            = NO_ERROR;            /* Return code               */

    rc = DosCreateEventSem(szSemName,      /* Name of semaphore to create  */
                           &hevEvent1,     /* Handle of semaphore returned */
                           DC_SEM_SHARED,  /* Shared semaphore             */
                           FALSE);         /* Semaphore is in RESET state  */
    if (rc != NO_ERROR) {
        printf("DosCreateEventSem error: return code = %u\n", rc);
        return 1;       }

    rc = DosAsyncTimer(7000L,              /* 7 second interval            */
                       (HSEM) hevEvent1,   /* Semaphore to post            */
                       &htimerEvent1);     /* Timer handler (returned)     */
    if (rc != NO_ERROR) {
        printf("DosAsyncTimer error: return code = %u\n", rc);
        return 1;
    } else {
        printf("Timer will expire in about 7 seconds...\n");
    }

           /* ... add your other processing here... */

    rc = DosWaitEventSem(hevEvent1,            /* Wait for AsyncTimer event */
                 (ULONG) SEM_INDEFINITE_WAIT); /* As long as it takes       */
    if (rc != NO_ERROR) {
        printf("DosWaitEventSem error: return code = %u\n", rc);
        return 1;
    }

    rc = DosCloseEventSem(hevEvent1);      /* Get rid of semaphore       */
    if (rc != NO_ERROR) {
        printf("DosCloseEventSem error: return code = %u", rc);
        return 1;
    }

return NO_ERROR;
}
