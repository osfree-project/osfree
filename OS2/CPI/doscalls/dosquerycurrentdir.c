#include "kal.h"


APIRET APIENTRY  DosQueryCurrentDir(ULONG disknum,
                                    PBYTE pBuf,
                                    PULONG pcbBuf)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("disknum=%lu\n", disknum);
  rc = KalQueryCurrentDir(disknum,
                          pBuf,
                          pcbBuf);
  log("pBuf=%s\n", pBuf);
  log("pcbBuf=%lx\n", pcbBuf);
  log("*pcbBuf=%lu\n", *pcbBuf);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}
