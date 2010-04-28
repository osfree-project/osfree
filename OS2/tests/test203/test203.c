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

  EXCEPTIONREGISTRATIONRECORD RegRec  = {0};     /* Exception Registration Record */

  APIRET      rc      = NO_ERROR;   /* Return code                   */



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

  printf("*** MyTermHandler entered: ExceptionNum = %x\n",p1->ExceptionNum);



  switch( p1->ExceptionNum) {

   case XCPT_SIGNAL: {

      printf(" XCPT_SIGNAL");

      switch( p1->ExceptionInfo[0] ) {

        case XCPT_SIGNAL_INTR:     { printf("_INTR"); break; }

        case XCPT_SIGNAL_KILLPROC: { printf("_KILLPROC"); break; }

        case XCPT_SIGNAL_BREAK:    { printf("_BREAK"); break; }

        default:;

      }

      printf("\n"); break;

   }

   case XCPT_PROCESS_TERMINATE:       { printf(" Process Terminate \n"); break;}

   case XCPT_ASYNC_PROCESS_TERMINATE: { printf(" Async Process Term\n"); break;}

   case XCPT_UNWIND:                  { printf(" XCPT_UNWIND\n");        break;}

   default:;

  }



  return XCPT_CONTINUE_SEARCH;          /* Exception not resolved... */

}
