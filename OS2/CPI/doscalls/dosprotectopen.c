#include "kal.h"

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
  APIRET rc;

  log("%s\n", __FUNCTION__);
  log("pszFileName=%s\n", *pszFileName);
  log("cbFile=%lx\n", cbFile);
  log("ulAttribute=%lx\n", ulAttribute);
  log("fsOpenFlags=%lx\n", fsOpenFlags);
  log("fsOpenMode=%lx\n", fsOpenMode);

  cbFileL.ulLo=cbFile;
  cbFileL.ulHi=0;

  rc = DosProtectOpenL(pszFileName, phf,
                       pulAction,
                       cbFileL,
                       ulAttribute,
                       fsOpenFlags,
                       fsOpenMode,
                       peaop2,
                       pfhFileHandleLockID);
  log("hf=%lx\n", *phf);
  log("ulAction=%lx\n", *pulAction);
  log("fhLockID=%lx\n", *pfhFileHandleLockID);
  return rc;
}
