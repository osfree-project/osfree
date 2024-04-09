#include "kal.h"

USHORT APIENTRY16      DOS16COPY(char * _Seg16 pszSrc, char * _Seg16 pszDst,
                                 USHORT opMode, ULONG reserved)
{
  return DosCopy(pszSrc, pszDst, opMode);
}
