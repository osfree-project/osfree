#include "kal.h"

APIRET16 APIENTRY16 DOS16MKDIR(PSZ DirName, ULONG Reserved)
{
  return DosCreateDir(DirName, NULL);
}
