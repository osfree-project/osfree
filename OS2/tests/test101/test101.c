#define INCL_DOSSEMAPHORES   /* DOS semaphore values */

#define INCL_DOSERRORS       /* DOS error values */

#include <os2.h>

#include <stdio.h>



int main(VOID) {

 HMUX       hmuxFromCreate  = NULLHANDLE;   /* Handle returned by Create */

 HMUX       hmuxFromOpen    = NULLHANDLE;   /* Handle returned by Open */

 HEV        hev[2]          = {0};          /* Event semaphores */

 SEMRECORD  apsr[2]         = {{0}};        /* Semaphore records */

 APIRET     rc              = NO_ERROR;     /* Return code */

 ULONG      ulSem           = 0;



rc = DosCreateEventSem("\\SEM32\\E0", &hev[0], 0, FALSE);

apsr[0].hsemCur = (HSEM) hev[0],

apsr[0].ulUser = 0;



rc = DosCreateEventSem("\\SEM32\\E1", &hev[1], 0, FALSE);

apsr[1].hsemCur = (HSEM) hev[1],

apsr[1].ulUser = 0;



rc = DosCreateMuxWaitSem("\\SEM32\\MUXWAIT1", &hmuxFromCreate, 2L,

                         apsr, DCMW_WAIT_ALL);

if (rc != NO_ERROR) {

   printf("DosCreateMuxWaitSem error:  return code = %u\n", rc);

   return 1;

} else {

   printf ("DosCreateMuxWaitSem returns handle = 0x%x\n", hmuxFromCreate);

}

      /*******************************************************************/

      /* The following call would normally be done from another program. */

      /*******************************************************************/



rc = DosOpenMuxWaitSem("\\SEM32\\MUXWAIT1",

                        &hmuxFromOpen);     /* Handle returned */

if (rc != NO_ERROR) {

  printf("DosOpenMuxWaitSem error:  return code = %u\n", rc);

  return 1;

} else {

   printf ("DosOpenMuxWaitSem   returns handle = 0x%x\n", hmuxFromOpen);

}



return NO_ERROR;

}

