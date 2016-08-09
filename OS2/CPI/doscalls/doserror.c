#include "kal.h"

APIRET APIENTRY DosError(ULONG errorno)
{
  log("%s\n", __FUNCTION__);
  log("errorno=%lx\n", errorno);
  return KalError(errorno);
}
