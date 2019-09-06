#include "kal.h"

APIRET APIENTRY  DosProtectSetFileSize(HFILE hFile,
                                       ULONG cbSize,
                                       FHLOCK fhFileHandleLockID)
{
  LONGLONG cbSizeL;
  APIRET rc;

  log("%s enter\n", __FUNCTION__);
  log("hFile=%lx\n", hFile);
  log("cbSize=%lx\n", cbSize);
  log("hLockId=%lx\n", fhFileHandleLockID);

  cbSizeL.ulLo=cbSize;
  cbSizeL.ulHi=0;

  rc = DosProtectSetFileSizeL(hFile,
                              cbSizeL,
                              fhFileHandleLockID);

  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}
