#define INCL_DOSPROCESS     /* Process and thread values */

#define INCL_DOSEXCEPTIONS  /* DOS exception values      */

#define INCL_DOSERRORS      /* DOS error values          */

#include <os2.h>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>



int main(USHORT argc, CHAR *argv[] ) {



   APIRET      rc          = NO_ERROR; /* Return code                        */

   PID         pidToIntr   = 0;        /* Interrupt this process             */



   if ( argc < 2 ) {

      printf("sendsig error:  Need to pass PID of thread to interrupt.\n");

      return 1;

   } else {

      pidToIntr = (PID) atoi ( argv[1] );

   } /* endif */



   rc = DosSendSignalException ( pidToIntr,            /* Process to interrupt */

                                 XCPT_SIGNAL_INTR );   /* Send this signal     */



   if (rc != NO_ERROR) {

      printf("DosSendSignalException error: return code = %u\n", rc);

      return 1;

   } else {

      printf ("DosSendSignalException complete.\n");

   } /* endif */



   return NO_ERROR;

}
