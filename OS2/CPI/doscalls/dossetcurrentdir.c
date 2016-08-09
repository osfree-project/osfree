#include "kal.h"


APIRET APIENTRY  DosSetCurrentDir(PCSZ  pszDir)
{
  log("%s\n", __FUNCTION__);
  log("pszDir=%s\n", pszDir);
  return KalSetCurrentDir((PSZ)pszDir);
}
