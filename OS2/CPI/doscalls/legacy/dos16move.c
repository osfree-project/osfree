#include "kal.h"

USHORT APIENTRY16   DOS16MOVE(char * _Seg16 old, char * _Seg16 new, ULONG reserved)
{
  return DosMove(old, new);
}
