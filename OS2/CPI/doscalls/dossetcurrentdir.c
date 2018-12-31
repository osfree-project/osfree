#include "kal.h"


APIRET APIENTRY  DosSetCurrentDir(PCSZ  pszDir)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("pszDir=%s\n", pszDir);
  rc = KalSetCurrentDir((PSZ)pszDir);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}
