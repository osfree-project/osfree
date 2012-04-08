/*  PMMERGE.DLL init/term: 
 *  (Load the communication backend)
 */

#define  INCL_DOSMISC
#define  INCL_DOSMODULEMGR
#include <os2.h>

#include <stdio.h>

APIRET APIENTRY (*InitServerConnection)(char *remotemachineName);
APIRET APIENTRY (*CloseServerConnection)(void);
APIRET APIENTRY (*F_SendCmdToServer)(int cmd, int data);
APIRET APIENTRY (*F_SendDataToServer)(void *data, int len);
APIRET APIENTRY (*F_RecvDataFromServer)(void *data, int *len, int maxlen);
APIRET APIENTRY (*F_SendGenCmdToServer)(int cmd, int par);
APIRET APIENTRY (*F_SendGenCmdDataToServer)(int cmd, int par, void *data, int datalen);
APIRET APIENTRY (*F_RecvCmdFromClient)(void *recvobj, int *ncmd, int *data);
void APIENTRY   (*fatal)(const char *message);

HMODULE   hmodBE = 0;

extern "C" APIRET APIENTRY __DLLstart_ (HMODULE hmod, ULONG flag);

extern "C" APIRET APIENTRY dll_initterm(HMODULE hmod, ULONG flag)
{
  PSZ       pszValue;
  char      LoadError[256];
  APIRET    rc;

  // call C startup init first
  rc = __DLLstart_(hmod, flag);

  if (!rc)
    return 0;

  if (flag)
  {
    // terminate
    if (hmodBE) DosFreeModule(hmodBE);
  }
  else
  {
    // init
    rc = DosScanEnv("PM_COMM_BACKEND", &pszValue);
    if (rc) pszValue = "PMPIPE";
    rc = DosLoadModule(LoadError, sizeof(LoadError), pszValue, &hmodBE);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 1, 0, (PFN *)&InitServerConnection);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 2, 0, (PFN *)&CloseServerConnection);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 3, 0, (PFN *)&F_SendCmdToServer);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 4, 0, (PFN *)&F_SendDataToServer);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 5, 0, (PFN *)&F_RecvDataFromServer);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 6, 0, (PFN *)&F_SendGenCmdToServer);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 7, 0, (PFN *)&F_SendGenCmdDataToServer);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 8, 0, (PFN *)&F_RecvCmdFromClient);

    if (rc)
     return 0;

    rc = DosQueryProcAddr(hmodBE, 9, 0, (PFN *)&fatal);

    if (rc)
      return 0;
  }

  return 1;
}
