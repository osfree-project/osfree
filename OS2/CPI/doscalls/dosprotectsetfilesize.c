#include "kal.h"

APIRET APIENTRY  DosProtectSetFileSize(HFILE hFile,
                                       ULONG cbSize,
                                       FHLOCK fhFileHandleLockID)
{
  LONGLONG cbSizeL;

  log("%s\n", __FUNCTION__);
  log("hFile=%lx\n", hFile);
  log("cbSize=%lx\n", cbSize);
  log("hLockId=%lx\n", fhFileHandleLockID);

  cbSizeL.ulLo=cbSize;
  cbSizeL.ulHi=0;

  return DosProtectSetFileSizeL(hFile,
                                cbSizeL,
                                fhFileHandleLockID);
}
