#include "kal.h"

typedef VOID (APIENTRY *PFNTHREAD)(const ULONG ul);

APIRET APIENTRY DosCreateThread(PTID ptid,
                                PFNTHREAD pfn,
                                ULONG param,
                                ULONG flag,
                                ULONG cbStack)
{
  log("%s\n", __FUNCTION__);
  log("ptid=%lx\n", ptid);
  log("param=%lx\n", param);
  log("flag=%lx\n", flag);
  log("cbStack=%lx\n", cbStack);
  return KalCreateThread(ptid, pfn, param, flag, cbStack);
}

APIRET APIENTRY DosSuspendThread(TID tid)
{
  log("%s\n", __FUNCTION__);
  log("tid=%lu\n", tid);
  return KalSuspendThread(tid);
}

APIRET APIENTRY DosResumeThread(TID tid)
{
  log("%s\n", __FUNCTION__);
  log("tid=%lu\n", tid);
  return KalResumeThread(tid);
}


APIRET APIENTRY DosWaitThread(PTID ptid,
                              ULONG option)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("option=%lu\n", option);
  rc = KalWaitThread(ptid, option);
  log("tid=%lu\n", *ptid);
  return rc;
}


APIRET APIENTRY DosKillThread(TID tid)
{
  log("%s\n", __FUNCTION__);
  log("tid=%lu\n", tid);
  return KalKillThread(tid);
}
