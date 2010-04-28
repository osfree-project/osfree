#define INCL_DOSPROCESS       /* DOS process values (for DosSleep) */

#define INCL_DOSEXCEPTIONS    /* DOS exception values */

#define INCL_ERRORS           /* DOS error values     */

#include <os2.h>

#include <stdio.h>



ULONG _System MyTermHandler( PEXCEPTIONREPORTRECORD       p1,

                             PEXCEPTIONREGISTRATIONRECORD p2,

                             PCONTEXTRECORD               p3,

                             PVOID                        pv );



int main (VOID)

{

  EXCEPTIONREGISTRATIONRECORD  RegRec  = {0}; /* Exception Registration Record */

  APIRET                       rc      = NO_ERROR;   /* Return code*/



  /* Add MyTermHandler to this thread's chain of exception handlers */



  RegRec.ExceptionHandler = (ERR)MyTermHandler;

  rc = DosSetExceptionHandler( &RegRec );

  if (rc != NO_ERROR) {

     printf("DosSetExceptionHandler error: return code = %u\n",rc);

     return 1;

  }



  printf("Terminate this program using Ctrl-C or Ctrl-Break.\n");



  rc = DosSleep(60000L);   /* Give user plenty of time to comply... */

  rc = DosUnsetExceptionHandler( &RegRec );

  if (rc != NO_ERROR) {

     printf("DosUnsetExceptionHandler error: return code = %u\n",rc);

     return 1;

  }

  return NO_ERROR;

}

/***************************************************************************/

ULONG _System MyTermHandler( PEXCEPTIONREPORTRECORD       p1,

                             PEXCEPTIONREGISTRATIONRECORD p2,

                             PCONTEXTRECORD               p3,

                             PVOID                        pv )

{

  ULONG       ulCount = 0;          /* Count of API calls  */

  APIRET      rc      = NO_ERROR;   /* Return code         */



  printf("*** MyTermHandler entered: ExceptionNum = %x\n",p1->ExceptionNum);



   rc = DosSetSignalExceptionFocus( SIG_UNSETFOCUS, &ulCount);

   if (rc != NO_ERROR) {

      printf("DosSetSignalExceptionFocus error: return code = %u\n", rc);

      return 1;

   }

        /**************************************/

        /* Handle XCPT_SIGNAL exceptions here */

        /**************************************/



   rc = DosSetSignalExceptionFocus( SIG_SETFOCUS, &ulCount);

   if (rc != NO_ERROR) {

      printf("DosSetSignalExceptionFocus error: return code = %u\n", rc);

      return 1;

   }



  return XCPT_CONTINUE_SEARCH;          /* Exception not resolved... */

}
