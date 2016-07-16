#include <os2.h>

#include "dl.h"

APIRET APIENTRY DosCreateEventSem(PCSZ pszName, PHEV phev, ULONG flAttr, BOOL32 fState)
{
  return KalCreateEventSem(pszName, phev, flAttr, fState);
}


APIRET APIENTRY DosOpenEventSem(PCSZ pszName, PHEV phev)
{
  return KalOpenEventSem(pszName, phev);
}


APIRET APIENTRY DosCloseEventSem(HEV hev)
{
  return KalCloseEventSem(hev);
}
