#include <os2.h>

APIRET APIENTRY  DosProtectOpen(PCSZ  pszFileName,
                                PHFILE phf,
                                PULONG pulAction,
                                ULONG cbFile,
                                ULONG ulAttribute,
                                ULONG fsOpenFlags,
                                ULONG fsOpenMode,
                                PEAOP2 peaop2,
                                PFHLOCK pfhFileHandleLockID)
{
  LONGLONG cbFileL;

  cbFileL.ulLo=cbFile;
  cbFileL.ulHi=0;

  return DosProtectOpenL(pszFileName, phf,
                                 pulAction,
                                 cbFileL,
                                 ulAttribute,
                                 fsOpenFlags,
                                 fsOpenMode,
                                 peaop2,
                                 pfhFileHandleLockID);
}
