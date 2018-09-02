#include "kal.h"

APIRET16 APIENTRY16 DOS16SETVERIFY(USHORT VerifyFlag)
{
  return DosSetVerify(VerifyFlag);
}
