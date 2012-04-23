/*  PMMERGE.DLL init/term: 
 *  (Load the communication backend)
 */

// libc includes
#include <string.h>
#include <stdio.h>
// FreePM includes
#include "FreePM.hpp"
#include "FreePM_err.hpp"
#include "Fc_config.hpp"
#include "F_def.hpp"
// OS/2 includes
#define  INCL_DOSMISC
#define  INCL_DOSMODULEMGR
#include <os2.h>

extern "C" APIRET client_obj = 0;
extern "C" APIRET APIENTRY (*InitServerConnection)(char *remotemachineName, ULONG *obj) = 0;
extern "C" APIRET APIENTRY (*CloseServerConnection)(void) = 0;
extern "C" APIRET APIENTRY (*startServerThreads)(void *handl) = 0;
extern "C" APIRET APIENTRY (*F_SendCmdToServer)(ULONG obj, int cmd, int data) = 0;
extern "C" APIRET APIENTRY (*F_SendDataToServer)(ULONG obj, void *data, int len) = 0;
extern "C" APIRET APIENTRY (*F_RecvDataFromServer)(ULONG obj, void *data, int *len, int maxlen) = 0;
extern "C" APIRET APIENTRY (*F_SendGenCmdToServer)(ULONG obj, int cmd, int par) = 0;
extern "C" APIRET APIENTRY (*F_SendGenCmdDataToServer)(ULONG obj, int cmd, int par, void *data, int datalen) = 0;
extern "C" APIRET APIENTRY (*F_RecvCmdFromClient)(ULONG obj, int *ncmd, int *data) = 0;
extern "C" APIRET APIENTRY (*F_RecvDataFromClient)(ULONG obj, void *sqmsg, int *l, int size) = 0;
extern "C" APIRET APIENTRY (*F_SendDataToClient)(ULONG obj, void *data, int len) = 0;
//extern "C" void APIENTRY   (*fatal)(const char *message) = 0;
//extern "C" void APIENTRY   (*db_print)(const char *format,...) = 0;

void APIENTRY _fatal(const char *message);
extern "C" void   APIENTRY _db_print(const char *format,...);
extern "C" APIRET _FreePM_db_level;
extern "C" APIRET _FreePM_debugLevels[MAX_DEBUG_SECTIONS];

HMODULE   hmodBE = 0;
ULONG *addr = 0;

class F_ClientConfig FPM_config;

/* Name of external machine for pipes like \\MACHINE\PIPE\SQDR   */
static char *ExternMachine = NULL;

extern "C" APIRET APIENTRY __DLLstart_ (HMODULE hmod, ULONG flag);

void ExecuteFreePMServer(void)
{
  //char Buffer[CCHMAXPATH];
  char     LoadErr[256];
  HMODULE  hmod;
  //PTIB     ptib;
  //PPIB     ppib;
  //char     *p, *q;

  //DosGetInfoBlocks(&ptib, &ppib);

  // find last backslash in the file path
  //for (p = strstr(ppib->pib_pchcmd, "\\"), q = p; p; q = p + 1, p = strstr(q, "\\")) ;
  //printf("\"%s\"\n", q);

  DosLoadModule(LoadErr, sizeof(LoadErr), "PMSRV", &hmod);
/*
  // check our command line
  if (strcasecmp(q, "fmshell.exe")) // "pmshell.exe"
  {
    // we're executing in a context other than pmshell.exe
    printf("lalala\n");
    char CmdLineBuf[2048];
    char *CmdLine;
    char *P;
    RESULTCODES ResultCodes;
    char ApplierEXE[] = "fmshell.exe";

    CmdLine = CmdLineBuf;

    if ((((ULONG)CmdLine+1024)&0xFFFF) < 1024)
        CmdLine += 1024;

    P = strcpy(CmdLine, ApplierEXE)+strlen(ApplierEXE)+1;
    *P = '"';
    P++;
    P = strcpy(P, Buffer)+strlen(Buffer);
    *P = '"';
    P++;
    *P = 0;
    P++;
    *P = 0;

    DosExecPgm(NULL, 0, EXEC_ASYNC, CmdLine, NULL, &ResultCodes, ApplierEXE);
  }
 */
}

extern "C" APIRET APIENTRY InitSrvConn(char *remotemachineName, ULONG *obj)
{
  int  rc;

  /* First  let's look for FreePM.ini and read it if any */
  FPM_config.Read("fpm.ini");

  if (remotemachineName)
  {
    strcpy(FPM_config.ExternMachineName, remotemachineName);
    ExternMachine = &FPM_config.ExternMachineName[0];
  } else {
    /* test for FreePM's semaphore  at local machine */

    HMTX    FREEPM_hmtx     = NULLHANDLE; /* Mutex semaphore handle */

    rc = DosOpenMutexSem(FREEPM_MUTEX_NAME,    /* Semaphore name */
                         &FREEPM_hmtx);        /* Handle returned */

    DosCloseMutexSem(FREEPM_hmtx);

    if (rc)
      /* FreePM server is not running at local machine, let's look for FreePM.ini */
      ExternMachine = &FPM_config.ExternMachineName[0];
    else
      ExternMachine = NULL;
  }

  /* init connection to FreePM server */
  rc = InitServerConnection(ExternMachine, obj);

  if (rc && ExternMachine)
  {
     ExecuteFreePMServer();
     DosSleep(1000);
     rc = InitServerConnection(NULL, obj);
  }

  return rc;
}

extern "C" APIRET APIENTRY CloseSrvConn (void)
{
  return CloseServerConnection();
}

extern "C" APIRET APIENTRY dll_initterm (HMODULE hmod, ULONG flag)
{
  PSZ       pszValue;
  char      LoadError[256];
  APIRET    rc;

  // call C startup init first
  rc = __DLLstart_(hmod, flag);

  //dummy_print = &dummy;

  if (!rc)
    return 0;

  if (flag)
  {
    // terminate
    if (hmodBE) DosFreeModule(hmodBE);
  }
  else
  {
    //fatal    = &_fatal;
    //db_print = &_db_print;

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

    rc = DosQueryProcAddr(hmodBE, 3, 0, (PFN *)&startServerThreads);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 4, 0, (PFN *)&F_SendCmdToServer);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 5, 0, (PFN *)&F_SendDataToServer);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 6, 0, (PFN *)&F_RecvDataFromServer);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 7, 0, (PFN *)&F_SendGenCmdToServer);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 8, 0, (PFN *)&F_SendGenCmdDataToServer);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 9, 0, (PFN *)&F_RecvCmdFromClient);

    if (rc)
     return 0;

    rc = DosQueryProcAddr(hmodBE, 10, 0, (PFN *)&F_RecvDataFromClient);

    if (rc)
     return 0;

    rc = DosQueryProcAddr(hmodBE, 11, 0, (PFN *)&F_SendDataToClient);

    if (rc)
     return 0;
/*
    rc = DosQueryProcAddr(hmodBE, 12, 0, (PFN *)&fatal);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 13, 0, (PFN *)&db_print);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 14, 0, (PFN *)&FreePM_db_level);
    //FreePM_db_level = *addr;

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 15, 0, (PFN *)&FreePM_debugLevels);

    if (rc)
      return 0;
 */
    //memset(FreePM_debugLevels, 0, sizeof(FreePM_debugLevels));
  }

  return 1;
}
