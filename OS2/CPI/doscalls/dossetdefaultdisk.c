#include "kal.h"


APIRET APIENTRY DosSetDefaultDisk(ULONG disknum)
{
  log("%s\n", __FUNCTION__);
  log("disknum=%lu\n", disknum);
  return KalSetDefaultDisk(disknum);
}
