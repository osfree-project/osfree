#include "kal.h"

USHORT APIENTRY16  DOS16WAITNMPIPE(CHAR * _Seg16 FileName, ULONG TimeOut)
{
  return DosWaitNPipe(FileName, TimeOut);
}
