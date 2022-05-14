#define INCL_DOSPROCESS     /* Process and thread values */

#define INCL_DOSERRORS      /* DOS error values          */

#include <os2.h>

#include <stdio.h>



void _System CntThreadProc(ULONG LoopMax);  /* Count Thread */



int main(VOID) {

   TID         tidCntThread = 0;     /* ID returned for newly created thread */

   PFNTHREAD   pfnCntThread = &CntThreadProc; /* Address of thread program   */

   ULONG       ulThreadParm = 100;        /* Parameter to thread routine     */

   APIRET      rc           = NO_ERROR;   /* Return code                     */



   rc = DosCreateThread(&tidCntThread,  /* Thread ID (returned by function)  */

                        pfnCntThread,   /* Address of thread program         */

                        ulThreadParm,   /* Parameter passed to ThreadProc    */

                        CREATE_READY |  /* Thread is ready when created      */

                        STACK_SPARSE,   /* Do not pre-commit stack pages     */

                        8192L);         /* Stack size, rounded to page bdy   */

   if (rc != NO_ERROR) {

      printf("DosCreateThread error: return code = %u\n", rc);

      return 1;

   }



   rc = DosSleep (1000);  /* Sleep for a second to allow thread to run a bit */



   rc = DosSuspendThread (tidCntThread);

   if (rc != NO_ERROR) {

      printf("DosSuspendThread error: return code = %u\n", rc);

      return 1;

   }



   rc = DosSleep (5000);  /* Sleep 5 seconds before resuming the thread     */



   rc = DosResumeThread (tidCntThread);

   if (rc != NO_ERROR) {

      printf("DosResumeThread error: return code = %u\n", rc);

      return 1;

   }



   rc = DosWaitThread (&tidCntThread, DCWW_WAIT);

   if (rc != NO_ERROR) {

      printf ("DosWaitThread error : return code = %u\n", rc);

   }

   printf ("Thread has completed!\n");



   return NO_ERROR;

}



void _System CntThreadProc(ULONG LoopMax )  /* Count thread */

{

   ULONG i = 0;                 /* Loop index */



   for (i=0;i < LoopMax;i++ ) {

      printf ("%d\n", i);

   }



   return;

}

