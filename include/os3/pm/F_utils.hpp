/*
 $Id: F_utils.hpp,v 1.1.1.2 2002/09/05 17:55:49 evgen2 Exp $
*/
/* F_utils.hpp */
/* ver 0.00 22.08.2002 */

#ifndef FREEPM_UTILS
 #define FREEPM_UTILS

#include <sys/time.h>

/**** FreePM extension/utilities **********************/
/*
 The following process' type codes are available:

     0         Full screen protect-mode session
     1         Requires real mode. Dos emulation.
     2         VIO windowable protect-mode session
     3         Presentation Manager protect-mode session
     4         Detached protect-mode process.
*/
int QueryProcessType(void);
extern "C" int QueryThreadOrdinal(int &tid);


time_t getCurrentTime(void);

int OS2SetRelMaxFH(int ReqCount);

#endif
   /* FREEPM_UTILS */
