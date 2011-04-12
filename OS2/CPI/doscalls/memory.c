#include <os2.h>

APIRET APIENTRY  DosAllocMem(PPVOID ppb,
                             ULONG cb,
                             ULONG flag)
{
  return KalAllocMem(ppb, cb, flag);
}

APIRET APIENTRY  DosFreeMem(PVOID pb)
{
  return KalFreeMem(pb);
}
