#define INCL_DOSPROCESS    /* DOS Process and thread values */

#define INCL_DOSERRORS     /* DOS Error values */

#include <os2.h>

#include <stdio.h>



int main(VOID) {



   APIRET   rc = NO_ERROR;     /* Return code                       */



   rc = DosBeep(1440L,         /* Beep frequency, in hertz          */

                1000L);        /* Duration of beep, in milliseconds */



   if (rc != NO_ERROR) {

      printf("DosBeep error: return code = %u\n", rc);

      return 1;

   }



   return NO_ERROR;

}

