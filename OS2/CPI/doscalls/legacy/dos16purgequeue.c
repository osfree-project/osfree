#include "kal.h"

USHORT APIENTRY16       DOS16PURGEQUEUE(HQUEUE hq)
{
  return DosPurgeQueue(hq);
}
