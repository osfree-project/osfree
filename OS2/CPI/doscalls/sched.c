#include <os2.h>

APIRET __cdecl  KalSleep(ULONG msec);

APIRET APIENTRY DosSleep(ULONG msec)
{
  return KalSleep(msec);
}
