#include "kal.h"

USHORT APIENTRY16   DOSFREEMODULE(HMODULE hmod)
{
  return DosFreeModule(hmod);
}
