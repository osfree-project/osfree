#include "kal.h"

APIRET APIENTRY DosSleep(ULONG msec)
{
  log("%s\n", __FUNCTION__);
  log("msec=%lu\n", msec);
  return KalSleep(msec);
}
