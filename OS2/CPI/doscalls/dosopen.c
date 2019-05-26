#include "kal.h"


APIRET APIENTRY  DosOpenL(PCSZ     pszFileName,
                         PHFILE pHf,
                         PULONG pulAction,
                         LONGLONG  cbFile,
                         ULONG  ulAttribute,
                         ULONG  fsOpenFlags,
                         ULONG  fsOpenMode,
                         PEAOP2 peaop2)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("pszFileName=%s\n", pszFileName);
  log("cbFile=%lld\n", cbFile);
  log("ulAttribute=%lx\n", ulAttribute);
  log("fsOpenFlags=%lx\n", fsOpenFlags);
  log("fsOpenMode=%lx\n", fsOpenMode);
  rc = KalOpenL((PSZ)pszFileName,
                pHf,
                pulAction,
                cbFile,
                ulAttribute,
                fsOpenFlags,
                fsOpenMode,
                peaop2);
  log("hf=%lx\n", *pHf);
  log("ulAction=%lx\n", *pulAction);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
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
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  cbFileL.ulLo=cbFile;
  cbFileL.ulHi=0;
  rc = DosOpenL(pszFileName, pHf, pulAction, cbFileL, ulAttribute, fsOpenFlags, fsOpenMode, peaop2);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}
