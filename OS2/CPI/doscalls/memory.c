#include <os2.h>

#include "dl.h"

//APIRET __cdecl   KalAllocMem(PPVOID ppb,
//                             ULONG cb,
//                             ULONG flag);

//APIRET __cdecl   KalFreeMem(PVOID pb);

//APIRET __cdecl   KalSetMem(PVOID pb,
//                           ULONG cb,
//                           ULONG flag);

//APIRET __cdecl   KalQueryMem(PVOID pb,
//                             PULONG pcb,
//                             PULONG pFlag);

//APIRET __cdecl   KalAllocSharedMem(PPVOID ppb,
//                                   PCSZ  pszName,
//                                   ULONG cb,
//                                   ULONG flag);

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

APIRET APIENTRY  DosSetMem(PVOID pb,
                           ULONG cb,
                           ULONG flag)
{
  return KalSetMem(pb, cb, flag);
}

APIRET APIENTRY  DosQueryMem(PVOID pb,
                             PULONG pcb,
                             PULONG pFlag)
{
  return KalQueryMem(pb, pcb, pFlag);
}

APIRET APIENTRY  DosAllocSharedMem(PPVOID ppb,
                                   PCSZ  pszName,
                                   ULONG cb,
                                   ULONG flag)
{
  return KalAllocSharedMem(ppb, pszName, cb, flag);
}
