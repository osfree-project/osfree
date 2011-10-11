#include <os2.h>

#include "dl.h"

APIRET APIENTRY DosError(ULONG errorno)
{
  return KalError(errorno);
}
