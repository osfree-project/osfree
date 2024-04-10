#include "kal.h"

USHORT APIENTRY16   DOS16MAKEPIPE(HFILE * _Seg16 ReadHandle, HFILE * _Seg16 WriteHandle,
                                  USHORT PipeSize)
{
  return DosCreatePipe(ReadHandle, WriteHandle, PipeSize);
}
