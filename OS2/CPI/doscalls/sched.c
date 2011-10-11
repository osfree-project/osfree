#include <os2.h>

#include "dl.h"

APIRET APIENTRY DosSleep(ULONG msec)
{
  return KalSleep(msec);
}
