#define INCL_BASE
#define INCL_ERRORS
#include <os2.h>

APIRET APIENTRY DosCancelLockRequest(HFILE hFile,
                                     PFILELOCK pflLock)
{
  FILELOCKL flLockL;

  if (pflLock==NULL)
  {
    return ERROR_INVALID_PARAMETER;
  }

  flLockL.lOffset.ulLo=pflLock->lOffset;
  flLockL.lOffset.ulHi=0;
  flLockL.lRange.ulLo=pflLock->lRange;
  flLockL.lRange.ulHi=0;

  return DosCancelLockRequestL(hFile,
                               &flLockL);
};
