#include "kal.h"

APIRET16 APIENTRY16 DOS16CHDIR(PSZ DirName, ULONG Reserved)
{
  return DosSetCurrentDir(DirName);
}
