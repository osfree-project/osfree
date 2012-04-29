#include "FreePM.hpp"

#include "F_def.hpp"
#include "F_config.hpp"
#include "F_globals.hpp"

#include "F_utils.hpp"

#include <stdio.h>
#include <time.h>

#include <sys/types.h>
#include <sys/timeb.h>

#include "debug.h"
#include <pmclient.h>

//DATETIME
time_t
getCurrentTime(void)
{
    struct _timeb timebuffer;

    _ftime(&timebuffer);
    _FreePM_current_time  = (long) timebuffer.time;
    //_FreePM_current_time.hundredths = (long) timebuffer.millitm * 1000;

    _FreePM_current_dtime = (double)timebuffer.time   +
       (double) timebuffer.millitm / 1000.0;
    return _FreePM_curtime = _FreePM_current_time;
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
