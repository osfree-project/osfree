#include "kal.h"

APIRET16 APIENTRY16 DOS16QVERIFY(USHORT * _Seg16 verify)
{
  APIRET rc;
  BOOL32 fUserVerify;

  rc=DosQueryVerify(&fUserVerify);
  *verify=fUserVerify;

  return rc;
}
