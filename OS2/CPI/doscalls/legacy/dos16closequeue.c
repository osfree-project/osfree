#include "kal.h"

USHORT APIENTRY16       DOS16CLOSEQUEUE(HQUEUE hq)
{
  return DosCloseQueue(hq);
}
