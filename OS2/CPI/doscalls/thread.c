#define  INCL_DOSPROCESS
#include <os2.h>

#include "dl.h"

typedef VOID (APIENTRY *PFNTHREAD)(const ULONG ul);

APIRET APIENTRY DosCreateThread(PTID ptid,
                                PFNTHREAD pfn,
                                ULONG param,
                                ULONG flag,
                                ULONG cbStack)
{
  return KalCreateThread(ptid, pfn, param, flag, cbStack);
}

APIRET APIENTRY DosSuspendThread(TID tid)
{
  return KalSuspendThread(tid);
}

APIRET APIENTRY DosResumeThread(TID tid)
{
  return KalResumeThread(tid);
}


APIRET APIENTRY DosWaitThread(PTID ptid,
                              ULONG option)
{
  return KalWaitThread(ptid, option);
}


APIRET APIENTRY DosKillThread(TID tid)
{
  return KalKillThread(tid);
}
