/* F_utils.cpp */

#include <stdio.h>
//#include <time.h>

#include "FreePM.hpp"

#include <sys/types.h>
//#include <sys/timeb.h>
#include "F_def.hpp"

#include <pmclient.h>

extern "C" void APIENTRY _db_print(const char *format,...);
extern "C" APIRET _FreePM_db_level;
extern "C" APIRET _FreePM_debugLevels[MAX_DEBUG_SECTIONS];

/* добавить процессу ReqCount файловых хендлов, возвращает текущее число хендлов (новое) */
int OS2SetRelMaxFH(int ReqCount)
{ ULONG    CurMaxFH;
  int rc;
  rc = DosSetRelMaxFH((PLONG)&ReqCount,     /* Using 0 here will return the       */
                           &CurMaxFH);    /* current number of file handles     */
  if(rc)
  {   debug(99, 0) ("OS2SetRelMaxFH: rc =%d, %s\n", rc, GetOS2ErrorMessage(rc));
  }
  return (int)CurMaxFH;
}

int QueryProcessType(void)
{
    PTIB   ptib = NULL;          /* Thread information block structure  */
    PPIB   ppib = NULL;          /* Process information block structure */
    APIRET rc   = NO_ERROR;      /* Return code                         */
    int prtype;

    rc = DosGetInfoBlocks(&ptib, &ppib);
    if (rc != NO_ERROR)
    {  printf ("DosGetInfoBlocks error : rc = %u\n", rc);
          return 1;
    }

    prtype = ppib->pib_ultype;
/*
  pib_ultype (ULONG)
     Process' type code.

     The following process' type codes are available:

     0         Full screen protect-mode session
     1         Requires real mode. Dos emulation.
     2         VIO windowable protect-mode session
     3         Presentation Manager protect-mode session
     4         Detached protect-mode process.

*/
    return prtype;
}



