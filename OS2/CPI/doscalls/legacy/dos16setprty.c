#include "kal.h"

USHORT APIENTRY16   DOS16SETPRTY(USHORT scope, USHORT PriorityClass,
                                 SHORT PriorityDelta, USHORT id)
{
  return DosSetPriority(scope, PriorityClass, PriorityDelta, id);
}
