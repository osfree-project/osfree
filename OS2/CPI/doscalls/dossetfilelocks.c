#include "kal.h"

APIRET APIENTRY DosSetFileLocks(HFILE hFile,
                                PFILELOCK pflUnlock,
                                PFILELOCK pflLock,
                                ULONG timeout,
                                ULONG flags)
{
  FILELOCKL flUnlockL;
  FILELOCKL flLockL;
  APIRET rc;

  log("%s enter\n", __FUNCTION__);
  log("hFile=%lx\n", hFile);
  log("timeout=%lx\n", timeout);
  log("flags=%lx\n", flags);

  if ((pflUnlock==NULL) | (pflLock==NULL))
  {
    rc = ERROR_INVALID_PARAMETER;
    goto DOSSETFILELOCKS_EXIT;
  }

  flUnlockL.lOffset.ulLo=pflUnlock->lOffset;
  flUnlockL.lOffset.ulHi=0;
  flUnlockL.lRange.ulLo=pflUnlock->lRange;
  flUnlockL.lRange.ulHi=0;
  flLockL.lOffset.ulLo=pflLock->lOffset;
  flLockL.lOffset.ulHi=0;
  flLockL.lRange.ulLo=pflLock->lRange;
  flLockL.lRange.ulHi=0;

  rc = DosSetFileLocksL(hFile, &flUnlockL, &flLockL, timeout, flags);

DOSSETFILELOCKS_EXIT:
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}
