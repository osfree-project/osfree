#include "kal.h"

USHORT APIENTRY16     DOS16EDITNAME(USHORT level, char * _Seg16 pszSrc,
                                    char * _Seg16 pszEditStr, void * _Seg16 pBuf,
                                    USHORT cbBuf)
{
  return DosEditName(level, pszSrc, pszEditStr, pBuf, cbBuf);
}
