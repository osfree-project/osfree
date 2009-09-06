#define INCL_BASE
#define INCL_ERRORS
#include <os2.h>

APIRET APIENTRY  DosProtectSetFilePtr(HFILE hFile,
                                      LONG ib,
                                      ULONG method,
                                      PULONG ibActual,
                                      FHLOCK fhFileHandleLockID)
{
  LONGLONG ibL;
  LONGLONG ibActualL;
  APIRET rc;

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

  return rc;
}
