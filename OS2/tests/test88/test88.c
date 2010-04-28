#define INCL_DOSMEMMGR   /* Include DOS Memory Management APIs */

#define INCL_DOSERRORS   /* DOS error values                   */

#include <os2.h>

#include <stdio.h>

#include <string.h>



int main (VOID)

{

   PVOID  pvShrObject = NULL;      /* Pointer to shared memory object      */

   PSZ    pszMemName  = "\\SHAREMEM\\MYTOOL\\APPLICAT.DAT"; /* Object name */

   PVOID  pvAltObject = NULL;      /* Alternate pointer to shared memory   */

   APIRET rc          = NO_ERROR;  /* Return code                          */

   ULONG  ulObjSize   = 1024;   /* Size (system rounds to 4096 - page bdy) */



   rc = DosAllocSharedMem(&pvShrObject,   /* Pointer to object pointer     */

                          pszMemName,     /* Name for shared memory        */

                          ulObjSize,      /* Desired size of object        */

                          PAG_COMMIT |    /* Commit memory                 */

                          PAG_WRITE );    /* Allocate memory as read/write */

   if (rc != NO_ERROR) {

      printf("DosAllocSharedMem error:  return code = %u\n",rc);

      return 1;

   }



   strcpy(pvShrObject, "Write your shared application data here.");



      /* Get the address of the shared memory and reference it that way.

         (Done for illustrative purposes only, this is how another process

         would go about accessing the named shared memory.)            */



   rc = DosGetNamedSharedMem(&pvAltObject,  /* Pointer to pointer of object */

                             pszMemName,    /* Name of shared memory        */

                             PAG_READ);     /* Want read-only access        */

   if (rc != NO_ERROR) {

      printf("DosGetNamedSharedMem error:  return code = %u\n",rc);

      return 1;

   }



   printf("Shared data read was \"%s\"\n",pvAltObject);



   rc = DosFreeMem(pvShrObject);

   if (rc != NO_ERROR) {

      printf("DosFreeMem error:  return code = %u\n",rc);

      return 1;

   }



   return NO_ERROR;

}
