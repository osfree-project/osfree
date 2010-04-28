#define  INCL_DOSPROCESS

#define  INCL_DOSERRORS

#include <os2.h>

#include <stdio.h>          /* Needed for printf */



PULONG   pulThreadDWords = NULL;     /* Pointer to thread DWORDs returned */

APIRET   rc              = NO_ERROR; /* Return code */



int main(VOID) {



         /* Allocate 3 DWORDs of local thread memory */



  rc = DosAllocThreadLocalMemory(3,                   /* Number of DWORDs */

                                 &pulThreadDWords);   /* Address returned */



  if (rc != NO_ERROR) {

    printf("DosAllocThreadLocalMemory error: return code = %u\n", rc);

    return 1;

  }



          /* ... Use the thread-local memory ... */



  rc = DosFreeThreadLocalMemory(pulThreadDWords);     /* Free the DWORDs */



  if (rc != NO_ERROR) {

    printf("DosFreeThreadLocalMemory error: return code = %u\n", rc);

    return 1;

  }



  return NO_ERROR;

}

