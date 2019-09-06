#include "kal.h"

typedef VOID (APIENTRY *PFNTHREAD)(const ULONG ul);

APIRET APIENTRY DosCreateThread(PTID ptid,
                                PFNTHREAD pfn,
                                ULONG param,
                                ULONG flag,
                                ULONG cbStack)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("ptid=%lx\n", ptid);
  log("param=%lx\n", param);
  log("flag=%lx\n", flag);
  log("cbStack=%lx\n", cbStack);
  rc = KalCreateThread(ptid, pfn, param, flag, cbStack);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY DosSuspendThread(TID tid)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("tid=%lu\n", tid);
  rc = KalSuspendThread(tid);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY DosResumeThread(TID tid)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("tid=%lu\n", tid);
  rc = KalResumeThread(tid);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}


APIRET APIENTRY DosWaitThread(PTID ptid,
                              ULONG option)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("option=%lu\n", option);
  rc = KalWaitThread(ptid, option);
  log("tid=%lu\n", *ptid);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}


APIRET APIENTRY DosKillThread(TID tid)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("tid=%lu\n", tid);
  rc = KalKillThread(tid);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}
