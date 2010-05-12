#include <os2.h>

APIRET __cdecl KalExecPgm(PCHAR pObjname,
                           LONG cbObjname,
                           ULONG execFlag,
                           PCSZ  pArg,
                           PCSZ  pEnv,
                           PRESULTCODES pRes,
                           PCSZ  pName);

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
