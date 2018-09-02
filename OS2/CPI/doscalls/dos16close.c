#include "kal.h"

APIRET16 APIENTRY16 DOS16CLOSE(USHORT FileHandle)
{
  return DosClose(FileHandle);
}

