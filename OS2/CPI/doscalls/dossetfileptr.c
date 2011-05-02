#define INCL_BASE
#define INCL_ERRORS
#include <os2.h>

APIRET __cdecl   KalSetFilePtrL(HFILE hFile,
                                LONGLONG ib,
                                ULONG method,
                                PLONGLONG ibActual);

APIRET APIENTRY  DosSetFilePtrL(HFILE hFile,
                                LONGLONG ib,
                                ULONG method,
                                PLONGLONG ibActual)
{
  return KalSetFilePtrL(hFile, ib, method, ibActual);
}


APIRET APIENTRY  DosSetFilePtr(HFILE hFile,
                               LONG ib,
                               ULONG method,
                               PULONG ibActual)
{
  LONGLONG ibL;
  LONGLONG ibActualL;
  APIRET rc;

  if (ibActual==NULL)
  {
    return ERROR_INVALID_PARAMETER;
  }

  ibL.ulLo=ib;
  ibL.ulHi=0;

  rc=DosSetFilePtrL(hFile,
                    ibL,
                    method,
                    &ibActualL);

  if (ibActualL.ulHi!=0)
  {
    rc=ERROR_SEEK_ON_DEVICE;
  }

  *ibActual=ibActualL.ulLo;

  return rc;
}
