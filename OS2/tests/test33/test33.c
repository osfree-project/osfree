#define INCL_DOSMEMMGR   /* Include DOS Memory Management APIs */
#define INCL_DOSERRORS   /* DOS error values                   */
#include <os2.h>
#include <stdio.h>
#include <string.h>

int main (VOID)
{
   PVOID  MyObject    = NULL;     /* Pointer to memory object            */
   ULONG  ulObjSize   = 0;        /* Size of memory object (in bytes)    */
   ULONG  ulMemFlags  = 0;        /* Attribute flags for the object      */
   ULONG  ulMemSize   = 0;        /* Size of memory region for DosSetMem */
   APIRET rc          = NO_ERROR; /* Return code                         */

   ulObjSize = 2000;          /* Will be rounded to a page boundary - 4096 */

   rc = DosAllocMem(&MyObject,     /* Pointer to memory object pointer     */
                    ulObjSize,     /* Size of object to be allocated       */
                    PAG_WRITE );   /* Allocate memory as read/writeable    */

   if (rc != NO_ERROR) {
      printf("DosAllocMem error:  return code = %u\n",rc);
      return 1;
   }

       /* Object can't be used until it is COMMITTED.  Since this was
          not done at DosAllocMem time, do it now.                    */

   rc = DosSetMem(MyObject,                    /* Pointer to object      */
                  ulObjSize,                   /* Size of area to change */
                  PAG_DEFAULT | PAG_COMMIT );  /* Commit the object      */

   if (rc != NO_ERROR) {
      printf("DosSetMem error:  return code = %u\n",rc);
      rc = DosFreeMem(MyObject);
      /* If omitted, OS/2 frees it at termination */
      return 1;
   } else { printf("DosSetMem: complete\n"); }

   strcpy(MyObject, "The memory object has just been used.");

              /* Check COMMIT status of the memory object. */

   ulMemSize = ulObjSize;
   rc = DosQueryMem(MyObject, &ulMemSize, &ulMemFlags);

   if (rc == NO_ERROR) {

      if (ulMemFlags & PAG_COMMIT) {
         printf("   Page containing MyObject is now committed.\n");
      } else {
         printf("Error: Page containing MyObject has not"
                 "been committed.\n");
      } /* endif */
   } else {
      printf("DosQueryMem error: return code = %u\n",rc);
   } /* endif */

   rc = DosFreeMem(MyObject);

   if (rc != NO_ERROR) {
      printf("DosFreeMem error: return code = %u\n",rc);
      return 1;
   }

   return NO_ERROR;
}
