#include "kal.h"

APIRET APIENTRY DosCreateEventSem(PCSZ pszName, PHEV phev, ULONG flAttr, BOOL32 fState)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("pszName=%s\n", pszName);
  log("flAttr=%lu\n", flAttr);
  log("fState=%lu\n", fState);
  rc = KalCreateEventSem((PSZ)pszName, phev, flAttr, fState);
  log("hev=%lu\n", *phev);
  return rc;
}


APIRET APIENTRY DosOpenEventSem(PCSZ pszName, PHEV phev)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("pszName=%s\n", pszName);
  rc = KalOpenEventSem((PSZ)pszName, phev);
  log("hev=%lu\n", *phev);
  return rc;
}


APIRET APIENTRY DosCloseEventSem(HEV hev)
{
  log("%s\n", __FUNCTION__);
  log("hev=%lu\n", hev);
  return KalCloseEventSem(hev);
}
