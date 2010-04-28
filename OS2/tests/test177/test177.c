#define INCL_DOSFILEMGR      /* File Manager values */

#define INCL_DOSERRORS       /* DOS Error values */

#include <os2.h>

#include <stdio.h>



int main(VOID){



  ULONG    CurMaxFH      = 0;          /* Current count of handles         */

  LONG     ReqCount      = 0;          /* Number to adjust file handles    */

  APIRET   rc            = NO_ERROR;   /* Return code                      */



  rc = DosSetRelMaxFH(&ReqCount,     /* Using 0 here will return the       */

                      &CurMaxFH);    /* current number of file handles     */



  if (rc != NO_ERROR) {

     printf("DosSetRelMaxFH error: return code = %u\n", rc);

     return 1;

  } else {

     printf("Maximum number of file handles is %u.\n", CurMaxFH);

  }



  rc = DosSetMaxFH(110L);            /* Set maximum file handles to 110  */



  if (rc != NO_ERROR) {

     printf("DosSetMaxFH error: return code = %u\n", rc);

     return 1;

  }



  ReqCount = -5L;                    /* Want 5 less file handles         */



  rc = DosSetRelMaxFH(&ReqCount,&CurMaxFH);     /* Change handle maximum */



  if (rc != NO_ERROR) {

     printf("DosSetRelMaxFH error: return code = %u\n", rc);

     return 1;

  } else {

     printf("Maximum number of file handles is now %u.\n", CurMaxFH);

  }



  return NO_ERROR;

}
