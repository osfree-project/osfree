#include "kal.h"

USHORT APIENTRY16    DOS16SETMAXFH(USHORT usNumHandles)
{
  return DosSetMaxFH(usNumHandles);
}
