#include <os2.h>

#include "dl.h"

APIRET APIENTRY DosExecPgm(PCHAR pObjname,
                           LONG cbObjname,
                           ULONG execFlag,
                           PCSZ  pArg,
                           PCSZ  pEnv,
                           PRESULTCODES pRes,
                           PCSZ  pName)
{
  return KalExecPgm(pObjname,
                    cbObjname,
                    execFlag,
                    pArg,
                    pEnv,
                    pRes,
                    pName);
}
