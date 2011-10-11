#include <os2.h>

#include "dl.h"

//APIRET __cdecl   KalOpenL(PCSZ     pszFileName,
//                         PHFILE pHf,
//                         PULONG pulAction,
//                         LONGLONG  cbFile,
//                         ULONG  ulAttribute,
//                         ULONG  fsOpenFlags,
//                         ULONG  fsOpenMode,
//                         PEAOP2 peaop2);

APIRET APIENTRY  DosOpenL(PCSZ     pszFileName,
                         PHFILE pHf,
                         PULONG pulAction,
                         LONGLONG  cbFile,
                         ULONG  ulAttribute,
                         ULONG  fsOpenFlags,
                         ULONG  fsOpenMode,
                         PEAOP2 peaop2)
{
  return KalOpenL(pszFileName,
                  pHf,
                  pulAction,
                  cbFile,
                  ulAttribute,
                  fsOpenFlags,
                  fsOpenMode,
                  peaop2);
}

APIRET APIENTRY  DosOpen(PCSZ     pszFileName,
                         PHFILE pHf,
                         PULONG pulAction,
                         ULONG  cbFile,
                         ULONG  ulAttribute,
                         ULONG  fsOpenFlags,
                         ULONG  fsOpenMode,
                         PEAOP2 peaop2)
{
  LONGLONG cbFileL;
  cbFileL.ulLo=cbFile;
  cbFileL.ulHi=0;
  return DosOpenL(pszFileName, pHf, pulAction, cbFileL, ulAttribute, fsOpenFlags, fsOpenMode, peaop2);
}
