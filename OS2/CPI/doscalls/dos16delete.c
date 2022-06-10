#include "kal.h"

APIRET16 APIENTRY16 DOS16DELETE(char * _Seg16 pszFilename, ULONG reserved)
{
  return DosDelete(pszFilename);
}

