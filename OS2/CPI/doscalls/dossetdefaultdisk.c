#include "kal.h"


APIRET APIENTRY DosSetDefaultDisk(ULONG disknum)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("disknum=%lu\n", disknum);
  rc = KalSetDefaultDisk(disknum);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}
