#include "kal.h"

APIRET16 APIENTRY16 DOS16QVERIFY(PUSHORT VerifySetting)
{
  APIRET rc;
  BOOL32 fUserVerify;

  rc=DosQueryVerify(&fUserVerify);
  *VerifySetting=fUserVerify;

  return rc;
}
