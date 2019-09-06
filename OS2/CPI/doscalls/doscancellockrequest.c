#include "kal.h"

APIRET APIENTRY DosCancelLockRequest(HFILE hFile,
                                     PFILELOCK pflLock)
{
  FILELOCKL flLockL;
  APIRET rc;

  log("%s enter\n", __FUNCTION__);
  log("hFile=%lx\n", hFile);

  if (pflLock==NULL)
  {
    return ERROR_INVALID_PARAMETER;
  }

  flLockL.lOffset.ulLo=pflLock->lOffset;
  flLockL.lOffset.ulHi=0;
  flLockL.lRange.ulLo=pflLock->lRange;
  flLockL.lRange.ulHi=0;

  rc = DosCancelLockRequestL(hFile,
                             &flLockL);
  log("flLock=%lx\n", *pflLock);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
};
