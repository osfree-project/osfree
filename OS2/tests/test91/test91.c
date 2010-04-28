#define INCL_DOSMEMMGR   /* Include DOS Memory Management APIs */

#define INCL_DOSERRORS   /* DOS error values                   */

#include <os2.h>

#include <stdio.h>

#include <string.h>



int main (USHORT argc, PCHAR argv[])

{

   PVOID  pvShrObject = NULL;      /* Pointer to shared memory object      */

   ULONG  ulObjSize   = 0;         /* Size of memory object                */

   PID    pidSharer   = 0;         /* Process ID for partner               */

   APIRET rc          = NO_ERROR;  /* Return code                          */



   /************************************************************************/

   /* This example assumes that pidSharer can be initialized here with     */

   /* the process ID for the process to which we are giving the memory.    */

   /* Normally this would be passed to this program via inter-process      */

   /* communication (IPC) - a pipe, a queue, a semaphore, the file system. */

   /************************************************************************/



   ulObjSize = 4100;          /* Will be rounded to a page boundary - 8192 */



   rc = DosAllocSharedMem(&pvShrObject,   /* Pointer to object pointer     */

                          NULL,           /* Unnamed memory                */

                          ulObjSize,      /* Desired size of object        */

                          OBJ_GIVEABLE | PAG_COMMIT | PAG_WRITE | PAG_READ );

   if (rc != NO_ERROR) {

      printf("DosAllocSharedMem error:  return code = %u\n",rc);

      return 1;

   }



   rc = DosGiveSharedMem(pvShrObject,             /* Object pointer     */

                         pidSharer,               /* Process to give to */

                         PAG_WRITE | PAG_READ);   /* Memory attributes  */

   if (rc != NO_ERROR) {

      printf("DosGetSharedMem error: return code = %u\n", rc);

      return 1;

   }



   return NO_ERROR;

}
