//#define  INCL_DOSPROCESS
//#include <os2.h>

#include "kal.h"

APIRET APIENTRY DosExecPgm(PCHAR pObjname,
                           LONG cbObjname,
                           ULONG execFlag,
                           PCSZ  pArg,
                           PCSZ  pEnv,
                           PRESULTCODES pRes,
                           PCSZ  pName)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("cbObjname=%lx\n", cbObjname);
  log("execFlag=%lx\n", execFlag);
  log("pArg=%lx\n", pArg);
  log("pEnv=%lx\n", pEnv);
  log("pRes=%lx\n", pRes);
  log("pName=%s\n", pName);
  rc = KalExecPgm(pObjname,
                  cbObjname,
                  execFlag,
                  (PSZ)pArg,
                  (PSZ)pEnv,
                  pRes,
                  (PSZ)pName);
  log("pObjname=%s\n", pObjname);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}
