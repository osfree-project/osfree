#include <os2.h>

APIRET __cdecl KalSetDefaultDisk(ULONG disknum);

APIRET APIENTRY DosSetDefaultDisk(ULONG disknum)
{
  return KalSetDefaultDisk(disknum);
}
