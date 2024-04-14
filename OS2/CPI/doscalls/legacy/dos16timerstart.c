#include "kal.h"

USHORT APIENTRY16   DOS16TIMERSTART(ULONG interval, HSEM sem, HTIMER * _Seg16 timer)
{
  return DosStartTimer(interval, sem, timer);
}
