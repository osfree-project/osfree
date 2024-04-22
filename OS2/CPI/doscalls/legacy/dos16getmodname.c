#include "kal.h"

USHORT APIENTRY16   DOS16GETMODNAME(HMODULE hmod, USHORT cbBuf, char * _Seg16 szBuf)
{
  return DosQueryModuleName(hmod, cbBuf, szBuf);
}
