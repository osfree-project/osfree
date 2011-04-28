#include <os2.h>

APIRET __cdecl   KalAllocMem(PPVOID ppb,
                             ULONG cb,
                             ULONG flag);

APIRET __cdecl   KalFreeMem(PVOID pb);

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
