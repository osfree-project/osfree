#define INCL_DOSSPINLOCK

#include "kal.h"

USHORT APIENTRY16     DOS16FREESPINLOCK(HSPINLOCK Handle)
{
  return DosFreeSpinLock(Handle);
}
