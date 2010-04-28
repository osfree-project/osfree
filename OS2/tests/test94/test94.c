#define INCL_DOSPROCESS     /* Process and thread values */

#define INCL_DOSERRORS      /* DOS error values          */

#include <os2.h>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>



int main(USHORT argc, PCHAR argv[] ) {



   APIRET      rc          = NO_ERROR; /* Return code                        */

   TID         tidToKill   = 0;        /* Kill this thread                   */



   if ( argc < 2 ) {

      printf("kthread error:  Need to pass TID of thread to kill.\n");

      return 1;

   } else {

      tidToKill = (TID) atoi ( argv[1] );

   } /* endif */



   rc = DosKillThread ( tidToKill );   /* Kill specified thread */



   if (rc != NO_ERROR) {

      printf("DosKillThread error: return code = %u\n", rc);

      return 1;

   } else {

      printf ("DosKillThread complete.\n");

   } /* endif */



   return NO_ERROR;

}
