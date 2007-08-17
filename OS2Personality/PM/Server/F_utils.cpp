/* 
 $Id: F_utils.cpp,v 1.5 2002/11/18 13:24:53 evgen2 Exp $ 
*/
/* F_utils.cpp */
/* ver 0.00 22.08.2002       */

#include <stdio.h>
#include <time.h>
#include "F_def.hpp"
#include "F_config.hpp"
#include "F_globals.hpp"

#include "FreePM.hpp"
#include "F_utils.hpp"
#define F_INCL_DOSPROCESS
   #include "F_OS2.hpp"

#include <sys/types.h>
#include <sys/timeb.h>


time_t
getCurrentTime(void)
{
    struct _timeb timebuffer;

    _ftime(&timebuffer);
    _FreePM_current_time.tv_sec  = (long) timebuffer.time;
    _FreePM_current_time.tv_usec = (long) timebuffer.millitm * 1000;

    _FreePM_current_dtime = (double)timebuffer.time   +
       (double) timebuffer.millitm / 1000.0;
    return _FreePM_curtime = timebuffer.time;
}



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

int QueryThreadOrdinal(int &tid)
{   PTIB   ptib = NULL;          /* Thread information block structure  */
    PPIB   ppib = NULL;          /* Process information block structure */
    APIRET rc   = NO_ERROR;      /* Return code                         */
    PTIB2 pt2;
    int ordinal;

    rc = DosGetInfoBlocks(&ptib, &ppib);
    if (rc != NO_ERROR)
    {  printf ("DosGetInfoBlocks error : rc = %u\n", rc);
          return 1;
    }
    ordinal = ptib->tib_ordinal;
    pt2 = ptib->tib_ptib2;
    tid = pt2->tib2_ultid;
    return ordinal;
}


