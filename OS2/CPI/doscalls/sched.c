#include "kal.h"

APIRET APIENTRY DosSleep(ULONG msec)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("msec=%lu\n", msec);
  rc = KalSleep(msec);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}
