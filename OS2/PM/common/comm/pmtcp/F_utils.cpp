/*! @file F_utils.cpp
    @brief Process and thread helpers. Cross-platform: OS/2 and Win32.
*/

#include <stdio.h>

#if defined(__OS2__) || defined(__OS2V2__)
#define INCL_DOS
#define INCL_DOSERRORS      
#define INCL_PM
#include <os2.h>
#else
#include "F_tcp.hpp"
#endif

#include "F_debug.h"

#if defined(__OS2__) || defined(__OS2V2__)

/*! @brief Raise the file-handle limit by ReqCount.
    @param [in] ReqCount  Number of extra handles to request.
    @return The new maximum handle count. */
int OS2SetRelMaxFH(int ReqCount)
{
  ULONG CurMaxFH;
  int rc;
  rc = DosSetRelMaxFH((PLONG)&ReqCount, &CurMaxFH);
  if (rc) {
    F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_ERROR,
               "OS2SetRelMaxFH: rc =%d", rc);
  }
  return (int)CurMaxFH;
}

/*! @brief Return the process type code from the OS/2 PIB.
    @return Process type code, or 1 on error.
    @retval 0..4  Process type code.
    @retval 1     DosGetInfoBlocks failed. */
int QueryProcessType(void)
{
    PTIB   ptib = NULL;
    PPIB   ppib = NULL;
    APIRET rc   = NO_ERROR;

    rc = DosGetInfoBlocks(&ptib, &ppib);
    if (rc != NO_ERROR)
    {
        printf ("DosGetInfoBlocks error : rc = %u\n", rc);
        return 1;
    }
    return ppib->pib_ultype;
}

#else /* Win32 */

/*! @brief Stub for OS/2 function on Win32.
    @param [in] ReqCount  Ignored.
    @return 0.
    @retval 0 Always. */
int OS2SetRelMaxFH(int ReqCount)
{
    (void)ReqCount;
    return 0;
}

/*! @brief Stub for OS/2 function on Win32.
    @return 3 (pretend PM session).
    @retval 3 Always. */
int QueryProcessType(void)
{
    return 3;
}

#endif
