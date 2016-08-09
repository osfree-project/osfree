#include "kal.h"

APIRET APIENTRY DosProtectSetFileLocks(HFILE hFile,
                                       PFILELOCK pflUnlock,
                                       PFILELOCK pflLock,
                                       ULONG timeout, ULONG flags,
                                       FHLOCK fhFileHandleLockID)
{
  FILELOCKL flUnlockL;
  FILELOCKL flLockL;
  APIRET rc;

  log("%s\n", __FUNCTION__);
  log("hFile=%lx\n", hFile);
  log("timeout=%lx\n", timeout);
  log("flags=%lx\n", flags);
  log("fhLockID=%lx\n", fhFileHandleLockID);

  if ((pflUnlock==NULL) | (pflLock==NULL))
  {
    return ERROR_INVALID_PARAMETER;
  }

  flUnlockL.lOffset.ulLo=pflUnlock->lOffset;
  flUnlockL.lOffset.ulHi=0;
  flUnlockL.lRange.ulLo=pflUnlock->lRange;
  flUnlockL.lRange.ulHi=0;
  flLockL.lOffset.ulLo=pflLock->lOffset;
  flLockL.lOffset.ulHi=0;
  flLockL.lRange.ulLo=pflLock->lRange;
  flLockL.lRange.ulHi=0;

  rc = DosProtectSetFileLocksL(hFile, &flUnlockL, &flLockL, timeout, flags, fhFileHandleLockID);
  log("flUnlock=%lx\n", *pflUnlock);
  log("flLock=%lx\n", *pflLock);
  return rc;
}
