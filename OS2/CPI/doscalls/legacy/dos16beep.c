#include "kal.h"

USHORT APIENTRY16   DOS16BEEP(USHORT freq, USHORT duration)
{
  return DosBeep(freq, duration);
}
