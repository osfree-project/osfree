#include "kal.h"

USHORT APIENTRY16    DOS16ERROR(USHORT flag)
{
  return DosError(flag);
}
