#define INCL_DOSQUEUES        /* Queue values */

#define INCL_DOSERRORS        /* DOS Error values */

#include <os2.h>

#include <stdio.h>



int main(VOID) {

HFILE    ReadHandle  = 0;          /* Read handle of pipe */

HFILE    WriteHandle = 0;          /* Write handle of pipe */

ULONG    PipeSize    = 4096;       /* Size of pipe */

APIRET   rc          = NO_ERROR;   /* API return code */



rc = DosCreatePipe(&ReadHandle, &WriteHandle, PipeSize);



if (rc != NO_ERROR) {

   printf("DosCreatePipe error: return code = %u\n", rc);

   return 1;

}



rc = DosClose(ReadHandle);

if (rc != NO_ERROR) {

   printf("DosClose error: return code = %u\n", rc);

   return 1;

}



rc = DosClose(WriteHandle);

if (rc != NO_ERROR) {

   printf("DosClose error: return code = %u\n", rc);

   return 1;

}



return NO_ERROR;

}
