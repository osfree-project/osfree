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

  printf("You will get a System Error popup for the exception.\n");



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

  APIRET  rc = NO_ERROR;



  printf("*** MyTermHandler entered: ExceptionNum = %x\n",p1->ExceptionNum);



  rc = DosUnsetExceptionHandler( p2 );  /* Stop recursive entry to handler */

  rc = DosUnwindException( p2,          /* Exception Registration Record */

                           (PVOID) 0,   /* BAD ADDRESS!!! */

                           p1);         /* Exception Report Record */

  if (rc != NO_ERROR) {

     printf("DosUnwindException error:  return code = %u\n", rc);

  }



  return XCPT_CONTINUE_SEARCH;

}

