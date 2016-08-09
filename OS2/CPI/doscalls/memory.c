#include "kal.h"


APIRET APIENTRY  DosAllocMem(PPVOID ppb,
                             ULONG cb,
                             ULONG flag)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("cb=%lx\n", cb);
  log("flag=%lx\n", flag);
  rc = KalAllocMem(ppb, cb, flag);
  log("pb=%lx\n", *ppb);
  return rc;
}

APIRET APIENTRY  DosFreeMem(PVOID pb)
{
  log("%s\n", __FUNCTION__);
  log("pb=%lx\n", pb);
  return KalFreeMem(pb);
}

APIRET APIENTRY  DosSetMem(PVOID pb,
                           ULONG cb,
                           ULONG flag)
{
  log("%s\n", __FUNCTION__);
  log("pb=%lx\n", pb);
  log("cb=%lx\n", cb);
  log("flag=%lx\n", flag);
  return KalSetMem(pb, cb, flag);
}

APIRET APIENTRY  DosQueryMem(PVOID pb,
                             PULONG pcb,
                             PULONG pFlag)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("pb=%lx\n", pb);
  rc = KalQueryMem(pb, pcb, pFlag);
  log("cb=%lx\n", *pcb);
  log("flag=%lx\n", *pFlag);
  return rc;
}

APIRET APIENTRY  DosAllocSharedMem(PPVOID ppb,
                                   PCSZ  pszName,
                                   ULONG cb,
                                   ULONG flag)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("pszName=%s\n", pszName);
  log("cb=%lx\n", cb);
  log("flag=%lx\n", flag);
  rc = KalAllocSharedMem(ppb, (PSZ)pszName, cb, flag);
  log("pb=%lx\n", *ppb);
  return rc;
}

APIRET APIENTRY  DosGetNamedSharedMem(PPVOID ppb,
                                      PCSZ  pszName,
                                      ULONG flag)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("pszName=%s\n", pszName);
  log("flag=%lx\n", flag);
  rc = KalGetNamedSharedMem(ppb, (PSZ)pszName, flag);
  log("pb=%lx\n", *ppb);
  return rc;
}


APIRET APIENTRY  DosGetSharedMem(PVOID pb,
                                 ULONG flag)
{
  log("%s\n", __FUNCTION__);
  log("pb=%lx\n", pb);
  log("flag=%lx\n", flag);
  return KalGetSharedMem(pb, flag);
}

APIRET APIENTRY  DosGiveSharedMem(PVOID pb,
                                  PID pid,
                                  ULONG flag)
{
  log("%s\n", __FUNCTION__);
  log("pb=%lx\n", pb);
  log("pid=%lx\n", pid);
  log("flag=%lx\n", flag);
  return KalGiveSharedMem(pb, pid, flag);
}
