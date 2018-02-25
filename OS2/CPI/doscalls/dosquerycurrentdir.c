#include "kal.h"


APIRET APIENTRY  DosQueryCurrentDir(ULONG disknum,
                                    PBYTE pBuf,
                                    PULONG pcbBuf)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("disknum=%lu\n", disknum);
  rc = KalQueryCurrentDir(disknum,
                          pBuf,
                          pcbBuf);
  log("pBuf=%s\n", pBuf);
  log("pcbBuf=%lx\n", pcbBuf);
  log("*pcbBuf=%lu\n", *pcbBuf);
  return rc;
}
