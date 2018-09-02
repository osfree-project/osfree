#include "kal.h"

APIRET16 APIENTRY16 DOS16GETVERSION(PUSHORT VersionWord)
{
  ULONG aulBuffer[2];
  APIRET rc;

  rc = DosQuerySysInfo(QSV_VERSION_MAJOR, QSV_VERSION_MAJOR,
		     (void *)aulBuffer, 2*sizeof(ULONG));

  *VersionWord=aulBuffer[0]*256+aulBuffer[1];
  return rc;
}
