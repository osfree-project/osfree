 #define INCL_DOSPROCESS      /* Process values */

 #define INCL_DOSERRORS       /* Error values */

 #include <os2.h>

 #include <stdio.h>



 int main(VOID)

   {

   APIRET  rc = NO_ERROR;    /* Return code */



    rc = DosEnterCritSec();



    if (rc != NO_ERROR) {

        printf("DosEnterCritSec error: return code = %u\n",rc);

        return 1;

    }

              /***********************************************/

              /* Add critical section code here.  While this */

              /* code is running, all other threads are      */

              /* stopped.                                    */

              /***********************************************/



    rc =  DosExitCritSec();



    if (rc != NO_ERROR) {

        printf("DosExitCritSec error: return code = %u\n",rc);

        return 1;

    }



    return NO_ERROR;

    }
