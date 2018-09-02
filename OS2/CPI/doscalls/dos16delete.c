#include "kal.h"

APIRET16 APIENTRY16 DOS16DELETE(PSZ FileName, ULONG Reserved)
{
  return DosDelete(FileName);
}

