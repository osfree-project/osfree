#include "kal.h"


APIRET16 APIENTRY16 DOS16CHDIR(char * _Seg16 DirName, ULONG Reserved)
{
  return DosSetCurrentDir(DirName);
}
