#define INCL_DOSPROCESS

#define INCL_DOSERRORS

#include <os2.h>

#include <stdio.h>



int main(VOID)

{

   PTIB   ptib = NULL;          /* Thread information block structure  */

   PPIB   ppib = NULL;          /* Process information block structure */

   APIRET rc   = NO_ERROR;      /* Return code                         */



   rc = DosSetPriority (PRTYS_THREAD,        /* Change a single thread */

                        PRTYC_TIMECRITICAL,  /* Time critical class    */

                        15L,                 /* Increase by 15         */

                        0L);                 /* Assume current thread  */

   if (rc != NO_ERROR) {

      printf ("DosSetPriority error : rc = %u\n", rc);

      return 1;

   } else {

      rc = DosGetInfoBlocks(&ptib, &ppib);



      if (rc != NO_ERROR) {

         printf ("DosGetInfoBlocks error : rc = %u\n", rc);

         return 1;

      } else {

         printf("Priority Class = %d\n",

               ((ptib->tib_ptib2->tib2_ulpri) >> 8) & 0x00FF);

         printf("Priority Level = %d\n",

               ((ptib->tib_ptib2->tib2_ulpri) & 0x001F) );

      } /* endif */

   }



   return NO_ERROR;

}

