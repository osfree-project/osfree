#define INCL_BSE
#define INCL_VIO
#define INCL_DOSERRORS
#include <os2.h>

USHORT APIENTRY VioWrtTTY(const PCHAR Str, const USHORT Count, const HVIO Handle)
{
  ULONG ulActual;
  ULONG rc;

  // Check input parameters
  if (Str==NULL) return ERROR_VIO_INVALID_PARMS;

  if (Handle==0)
  {
    rc=DosWrite(1, Str, Count, &ulActual);
  } else {
    rc=ERROR_VIO_INVALID_HANDLE;
  }

  return rc;
}
