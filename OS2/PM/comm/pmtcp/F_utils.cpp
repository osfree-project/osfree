/*! @file F_utils.cpp
    @brief PMTCP utility helpers.

    Process and thread helpers shared with the PMPIPE backend.
*/

#include <stdio.h>

#include "FreePM.hpp"

#include <sys/types.h>
#include "F_def.hpp"

#include <pmclient.h>

/*! @brief Debug printer, implemented in F_debug.cpp.

    @param [in] format  printf-style format string.
    @param [in] ...     Arguments for the format string. */
extern "C" void APIENTRY _db_print(const char *format,...);
/*! @brief Current debug verbosity level. */
extern "C" APIRET _FreePM_db_level;
/*! @brief Per-section debug levels. */
extern "C" APIRET _FreePM_debugLevels[MAX_DEBUG_SECTIONS];

/*! @brief Raise the file-handle limit by ReqCount.

    @param [in] ReqCount  Number of extra handles to request.
    @return The new maximum handle count. */
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

/*! @brief Return the process type code from the OS/2 PIB.

    The returned value is one of:
      0 - Full screen protect-mode session
      1 - Requires real mode. DOS emulation.
      2 - VIO windowable protect-mode session
      3 - Presentation Manager protect-mode session
      4 - Detached protect-mode process.

    @return Process type code, or 1 on error.
    @retval 0..4  Process type code.
    @retval 1     DosGetInfoBlocks failed (also coincides with the
                  "requires real mode" type code). */
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
    return prtype;
}
