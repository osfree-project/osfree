#include "kal.h"

USHORT APIENTRY16   DOS16GETMODHANDLE(char * _Seg16 pszName, HMODULE * _Seg16 phmod)
{
  return DosQueryModuleHandle(pszName, phmod);
}
