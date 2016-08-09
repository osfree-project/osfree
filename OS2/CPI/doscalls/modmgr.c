/*  Module manager
 *
 */

#include "kal.h"

APIRET APIENTRY  DosLoadModule(PSZ  pszName,
                               ULONG cbName,
                               PCSZ  pszModname,
                               PHMODULE phmod)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("cbName=%lu\n", cbName);
  log("pszModname=%s\n", pszModname);
  rc = KalLoadModule(pszName, cbName, pszModname, phmod);
  log("pszName=%s\n", pszName);
  log("hmod=%lx\n", *phmod);
  return rc;
}

APIRET APIENTRY DosQueryModuleHandle(const PCSZ pszModname, PHMODULE phmod)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("pszModname=%s\n", pszModname);
  rc = KalQueryModuleHandle(pszModname, phmod);
  log("hmod=%lx\n", *phmod);
  return rc;
}

APIRET APIENTRY DosQueryModuleName(HMODULE hmod, ULONG cbName, PCHAR pch)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("hmod=%lx\n", hmod);
  log("cbName=%lu\n", cbName);
  rc = KalQueryModuleName(hmod, cbName, pch);
  log("pch=%s\n", pch);
  return rc;
}

APIRET APIENTRY DosQueryProcAddr(HMODULE hmod, ULONG ordinal, const PCSZ pszName, PFN *  ppfn)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("hmod=%lx\n", hmod);
  log("ordinal=%lx\n", ordinal);
  log("pszName=%s\n", pszName);
  rc = KalQueryProcAddr(hmod, ordinal, (PSZ)pszName, (void **)ppfn);
  log("pfn=%lx\n", *ppfn);
  return rc;
}

APIRET APIENTRY DosQueryProcType(HMODULE hmod, ULONG ordinal, const PCSZ pszName, PULONG pulproctype)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("hmod=%lx\n", hmod);
  log("ordinal=%lx\n", ordinal);
  log("pszName=%s\n", pszName);
  rc = KalQueryProcType(hmod, ordinal, (PSZ)pszName, pulproctype);
  log("proctype=%lx\n", *pulproctype);
  return rc;
}

APIRET APIENTRY DosQueryAppType(PCSZ pszName,
                                PULONG pFlags)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("pszName=%s\n", pszName);
  rc = KalQueryAppType((PSZ)pszName, pFlags);
  log("flags=%lx\n", *pFlags);
  return rc;
}

APIRET APIENTRY DosFreeModule(HMODULE hmod)
{
  log("%s\n", __FUNCTION__);
  log("hmod=%lx\n", hmod);
  return 0;
}
