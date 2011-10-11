#include <os2.h>

#include "dl.h"

//APIRET __cdecl KalQueryCurrentDisk(PULONG pdisknum,
//                                   PULONG plogical);

APIRET APIENTRY  DosQueryCurrentDisk(PULONG pdisknum,
                                     PULONG plogical)
{
  return KalQueryCurrentDisk(pdisknum, plogical);
}
