#include "kal.h"


APIRET APIENTRY  DosSetFilePtrL(HFILE hFile,
                                LONGLONG ib,
                                ULONG method,
                                PLONGLONG pibActual)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("hFile=%lx\n", hFile);
  log("ib=%lld\n", ib);
  log("method=%lx\n", method);
  rc = KalSetFilePtrL(hFile, ib, method, (ULONGLONG *)pibActual);
  log("ibActual=%lld\n", *pibActual);
  return rc;
}


APIRET APIENTRY  DosSetFilePtr(HFILE hFile,
                               LONG ib,
                               ULONG method,
                               PULONG pibActual)
{
  LONGLONG ibL;
  ULONGLONG ibActualL;
  APIRET rc;

  log("%s\n", __FUNCTION__);
  log("hFile=%lx\n", hFile);
  log("ib=%ld\n", ib);
  log("method=%lx\n", method);

  if (pibActual==NULL)
  {
    return ERROR_INVALID_PARAMETER;
  }

  ibL.ulLo=ib;
  ibL.ulHi=0;

  rc=DosSetFilePtrL(hFile,
                    ibL,
                    method,
                    (PLONGLONG)&ibActualL);

  *pibActual=ibActualL.ulLo;

  //if (ibActualL.ulHi)
  //  rc = ERROR_SEEK_ON_DEVICE;

  log("ibActual=%lld\n", *pibActual);
  return rc;
}
