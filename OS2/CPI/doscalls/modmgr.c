/*  Module manager
 *
 */

#define  INCL_DOSMODULEMGR
#include <os2.h>

APIRET __cdecl   KalLoadModule(PCSZ  pszName,
                               ULONG cbName,
                               PCSZ  pszModname,
                               PHMODULE phmod);

APIRET __cdecl   KalQueryModuleHandle(const PSZ pszModname,
                                      PHMODULE phmod);
                               
APIRET __cdecl   KalQueryModuleName(HMODULE hmod,
                                    ULONG cbName,
                                    PCHAR pch);

APIRET __cdecl   KalQueryProcAddr(HMODULE hmod,
                                  ULONG ordinal,
                                  const PSZ pszName,
                                  PFN   *ppfn);

APIRET APIENTRY  DosLoadModule(PCSZ  pszName,
                               ULONG cbName,
                               PCSZ  pszModname,
                               PHMODULE phmod)
{
  return KalLoadModule(pszName, cbName, pszModname, phmod);
}

APIRET APIENTRY DosQueryModuleHandle(const PSZ pszModname, PHMODULE phmod)
{
  return KalQueryModuleHandle(pszModname, phmod);
}

APIRET APIENTRY DosQueryModuleName(HMODULE hmod, ULONG cbName, PCHAR pch)
{
  return KalQueryModuleName(hmod, cbName, pch);
}

APIRET APIENTRY DosQueryProcAddr(HMODULE hmod, ULONG ordinal, const PSZ pszName, PFN *  ppfn)
{
  return KalQueryProcAddr(hmod, ordinal, pszName, ppfn);
}
