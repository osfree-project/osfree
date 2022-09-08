#include "kal.h"

APIRET16 APIENTRY16 DOS16GETVERSION(USHORT * _Seg16 version)
{
  ULONG aulBuffer[2];
  APIRET16 rc;

  rc = DosQuerySysInfo(QSV_VERSION_MAJOR, QSV_VERSION_MAJOR,
		     (void *)aulBuffer, 2*sizeof(ULONG));

  *version=aulBuffer[0]*256+aulBuffer[1];
  return rc;
}
