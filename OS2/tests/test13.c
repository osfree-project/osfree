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
  EXCEPTIONREGISTRATIONRECORD  RegRec  = {0};
  APIRET                       rc      = NO_ERROR;   /* Return code */

  /* Add MyTermHandler to this thread's chain of exception handlers */

  RegRec.ExceptionHandler = (ERR)MyTermHandler;
  rc = DosSetExceptionHandler( &RegRec );
  if (rc != NO_ERROR) {
     printf("DosSetExceptionHandler error: return code = %u\n",rc);
     return 1;
  }

  printf("Stop this program using Ctrl-C or Ctrl-Break.\n");

  rc = DosSleep(10000L);   /* Give user 10 seconds... */

  rc = DosUnsetExceptionHandler( &RegRec );
  if (rc != NO_ERROR) {
     printf("DosUnsetExceptionHandler error: return code = %u\n",rc);
     return 1;
  }

  printf("Program ends.\n");
  return NO_ERROR;
}

/**********************************************************************/
ULONG _System MyTermHandler( PEXCEPTIONREPORTRECORD       p1,
                             PEXCEPTIONREGISTRATIONRECORD p2,
                             PCONTEXTRECORD               p3,
                             PVOID                        pv )
{
  APIRET   rc  = NO_ERROR;   /* Return code */

  printf("*** MyTermHandler entered ***\n");

  if (p1->ExceptionNum == XCPT_SIGNAL) {
        rc = DosAcknowledgeSignalException ( XCPT_SIGNAL_BREAK );
        if (rc != NO_ERROR) {
          printf("DosAcknowledgeSignalException error: return code = %u\n", rc);
          return 1;
        } else {
          printf("DosAcknowledgeSignalException done. Program resumes.\n");
          return XCPT_CONTINUE_EXECUTION;  /* Continue execution */
        } /* endif */
  }

  return XCPT_CONTINUE_SEARCH;     /* Exception not resolved. */
}

