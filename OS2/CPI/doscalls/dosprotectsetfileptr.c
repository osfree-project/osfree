#include "kal.h"

APIRET APIENTRY  DosProtectSetFilePtr(HFILE hFile,
                                      LONG ib,
                                      ULONG method,
                                      PULONG ibActual,
                                      FHLOCK fhFileHandleLockID)
{
  LONGLONG ibL;
  LONGLONG ibActualL;
  APIRET rc;

  log("%s\n", __FUNCTION__);
  log("hFile=%lx\n", hFile);
  log("method=%lx\n", method);
  log("fhLockID=%lx\n", fhFileHandleLockID);

  if (ibActual==NULL)
  {
    return ERROR_INVALID_PARAMETER;
  };
  ibL.ulLo=ib;
  ibL.ulHi=0;
  rc=DosProtectSetFilePtrL(hFile,
                           ibL,
                           method,
                           &ibActualL,
                           fhFileHandleLockID);

  if (ibActualL.ulHi!=0)
  {
    rc=ERROR_SEEK_ON_DEVICE;
  }

  *ibActual=ibActualL.ulLo;

  log("ibActual=%lx\n", *ibActual);
  return rc;
}
