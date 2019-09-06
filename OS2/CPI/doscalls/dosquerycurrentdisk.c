#include "kal.h"


APIRET APIENTRY  DosQueryCurrentDisk(PULONG pdisknum,
                                     PULONG plogical)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  rc = KalQueryCurrentDisk(pdisknum, plogical);
  log("disknum=%lu\n", *pdisknum);
  log("logical=%lu\n", *plogical);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}
