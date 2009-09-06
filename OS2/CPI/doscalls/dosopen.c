#include <os2.h>

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
