#define INCL_DOSNLS     /* National Language Support values */

#define INCL_DOSERRORS  /* DOS error values                 */

#include <os2.h>

#include <stdio.h>



int main(VOID) {

   ULONG  aulCpList[8]  = {0},                /* Code page list        */

          ulBufSize     = 8 * sizeof(ULONG),  /* Size of output list   */

          ulListSize    = 0,                  /* Size of list returned */

          indx          = 0;                  /* Loop index            */

   APIRET rc            = NO_ERROR;           /* Return code           */



   rc = DosQueryCp(ulBufSize,      /* Length of output code page list  */

                   aulCpList,      /* List of code pages               */

                   &ulListSize);   /* Length of list returned          */



   if (rc != NO_ERROR) {

      printf("DosQueryCp error: return code = %u\n",rc);

      return 1;

   } else {

      for (indx=0; indx < ulListSize/sizeof(ULONG); indx++)

         printf ("aulCpList[%u] = %u\n", indx, aulCpList[indx]);

   }



   return NO_ERROR;

}
