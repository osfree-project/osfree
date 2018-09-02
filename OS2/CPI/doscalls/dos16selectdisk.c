#include "kal.h"

APIRET16 APIENTRY16 DOS16SELECTDISK(USHORT Drive)
{
  return DosSetDefaultDisk(Drive);
}
