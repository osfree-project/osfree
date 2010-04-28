#define INCL_DOSQUEUES        /* Queue values */

#define INCL_DOSERRORS        /* DOS Error values */

#include <os2.h>

#include <stdio.h>



int main(VOID) {



HFILE    ReadHandle     = NULLHANDLE; /* Read handle of pipe */

HFILE    WriteHandle    = NULLHANDLE; /* Write handle of pipe */

HFILE    NewReadHandle  = (HFILE) -1; /* Duplicate read handle */

HFILE    NewWriteHandle = (HFILE) 10; /* Duplicate write handle */

ULONG    PipeSize       = 42;         /* Size of pipe */

APIRET   rc             = NO_ERROR;   /* API return code */



rc = DosCreatePipe ( &ReadHandle, &WriteHandle, PipeSize );

if (rc != NO_ERROR) {

   printf("DosCreatePipe error: return code = %u\n", rc);

   return 1;

}

      /* Duplicate Read Handle of Pipe - use next available handle */



rc = DosDupHandle ( ReadHandle, &NewReadHandle );

if (rc != NO_ERROR) {

   printf("DosDupHandle error: return code = %u\n", rc);

   return 1;

}

      /* Duplicate Write Handle of Pipe - use handle 10 */



rc = DosDupHandle ( ReadHandle, &NewWriteHandle );

if (rc != NO_ERROR) {

   printf("DosDupHandle error: return code = %u\n", rc);

   return 1;

}



printf("Handles are: Read:%u  Write:%u  NewRead:%u  NewWrite:%u\n",

            ReadHandle, WriteHandle, NewReadHandle, NewWriteHandle);



return NO_ERROR;

}
