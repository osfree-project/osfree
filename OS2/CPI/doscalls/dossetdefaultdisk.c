#include <os2.h>

#include "dl.h"

//APIRET __cdecl KalSetDefaultDisk(ULONG disknum);

APIRET APIENTRY DosSetDefaultDisk(ULONG disknum)
{
  return KalSetDefaultDisk(disknum);
}
