#include "kal.h"

APIRET APIENTRY DosError(ULONG errorno)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("errorno=%lx\n", errorno);
  rc = KalError(errorno);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}
