#include "kal.h"

APIRET APIENTRY  DosSetFileSize(HFILE hFile,
                                ULONG cbSize)
{
  LONGLONG cbSizeL;
  APIRET rc;

  log("%s enter\n", __FUNCTION__);
  log("hFile=%lx\n", hFile);
  log("cbSize=%lx\n", cbSize);

  cbSizeL.ulLo=cbSize;
  cbSizeL.ulHi=0;

  rc = DosSetFileSizeL(hFile,
                       cbSizeL);

  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}
