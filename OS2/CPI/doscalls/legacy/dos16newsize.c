#include "kal.h"

USHORT APIENTRY16   DOS16NEWSIZE(HFILE hf, ULONG size)
{
  return DosSetFileSize(hf, size);
}

