 #define INCL_DOSSEMAPHORES   /* Semaphore values */

 #define INCL_DOSERRORS       /* DOS error values */

 #include <os2.h>

 #include <stdio.h>



 int main(VOID) {



 PSZ     szSemName  = "\\SEM32\\MYTOOL\\EVENTSEM\\1"; /* Semaphore name    */

 HEV     hevEvent      = 0;                   /* Event semaphore handle    */

 APIRET  rc            = NO_ERROR;            /* Return code               */



    rc = DosCreateEventSem(szSemName,      /* Name of semaphore to create  */

                           &hevEvent,      /* Handle of semaphore returned */

                           DC_SEM_SHARED,  /* Shared semaphore             */

                           FALSE);         /* Don't want it POSTed yet     */



    if (rc != NO_ERROR) {

        printf("DosCreateEventSem error: return code = %u\n", rc);

        return 1;       }



          /* OOPS... we changed our mind, POST the semaphore */



    rc = DosPostEventSem(hevEvent);



    if (rc != NO_ERROR) {

        printf("DosPostEventSem error: return code = %u\n", rc);

        return 1;

    }



return NO_ERROR;

}
