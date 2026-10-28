/*! @file init.cpp
    @brief PMCLIENT.DLL entry point and client transport wrappers.

    PMCLIENT.DLL is the client transport facade over the communication
    backend (PMPIPE.DLL by default). On DLL load it reads fpm.ini,
    selects the backend, loads it, and establishes the single
    process-wide connection to the FreePM server. On unload it closes
    the connection and unloads the backend.

    The handle of the server connection is internal to PMCLIENT and is
    not exported. All HABs and all threads in the process share it
    through the Pmc* wrappers below.

    Naming and types follow OS/2 CP API and SAA CPI conventions:
    Pmc<Verb><Noun>, MixedCase, canonical OS/2 types (ULONG, LONG,
    PVOID, PLONG).
*/

// libc includes
#include <string.h>
#include <stdio.h>
// FreePM includes
#include "Fc_config.hpp"
#include "F_config.hpp"
// OS/2 includes
#define  INCL_DOSMISC
#define  INCL_DOSMODULEMGR
#define  INCL_DOSSEMAPHORES
#define  INCL_DOSPROCESS
#include <os2.h>

/*! @brief Init backend entry point. */
static APIRET (APIENTRY *pfnInitServerConnection)(PCSZ pcszServerName, PULONG pulObject) = 0;
/*! @brief Close backend entry point. */
static APIRET (APIENTRY *pfnCloseServerConnection)(void) = 0;
/*! @brief Post-command backend entry point. */
static APIRET (APIENTRY *pfnSendCmdToServer)(ULONG ulObject, ULONG ulCommand, ULONG ulData) = 0;
/*! @brief Send-data backend entry point. */
static APIRET (APIENTRY *pfnSendDataToServer)(ULONG ulObject, PVOID pvData, LONG cbData) = 0;
/*! @brief Get-data backend entry point. */
static APIRET (APIENTRY *pfnRecvDataFromServer)(ULONG ulObject, PVOID pvBuffer, PLONG pcbReceived, LONG cbBuffer) = 0;
/*! @brief Send-command (sync) backend entry point. */
static APIRET (APIENTRY *pfnSendGenCmdToServer)(ULONG ulObject, ULONG ulCommand, ULONG ulParameter) = 0;
/*! @brief Send-command-with-data (sync) backend entry point. */
static APIRET (APIENTRY *pfnSendGenCmdDataToServer)(ULONG ulObject, ULONG ulCommand, ULONG ulParameter, PVOID pvData, LONG cbData) = 0;

/*! @brief Handle of the loaded communication backend. */
static HMODULE hmodBE = 0;

/*! @brief Client configuration, read once at DLL load. */
class F_ClientConfig FPM_config;

/*! @brief Process-wide connection to the FreePM server.

    Internal to PMCLIENT; not exported. Filled once in _DLL_InitTerm,
    used by all transport wrappers below on behalf of all HABs and all
    threads in this process. */
static ULONG client_obj = 0;

/*! @brief Start the local FreePM server as a detached process.

    Launches PMSERVER.EXE in the background. Returns immediately;
    the caller is expected to give the server time to create its
    named mutex before attempting to connect. */
void ExecuteFreePMServer(void)
{
  RESULTCODES rc;
  char szFailName[CCHMAXPATH];

  DosExecPgm(szFailName, sizeof(szFailName), EXEC_BACKGROUND,
             "PMSERVER.EXE", NULL, &rc, "PMSERVER.EXE");
}

/*! @brief Test whether a local FreePM server is already running.

    The server creates a named mutex at startup. If the mutex can be
    opened, the server is running; otherwise it is not.

    @return 1 if a local server is running, 0 otherwise.
    @retval 1  A local server is running.
    @retval 0  No local server. */
static int LocalServerRunning(void)
{
  HMTX   hmtx = NULLHANDLE;
  APIRET rc;

  rc = DosOpenMutexSem(FREEPM_MUTEX_NAME, &hmtx);
  if (!rc)
    DosCloseMutexSem(hmtx);

  return (rc == 0);
}

/*! @brief Post a command with a 32-bit argument to the server.

    Asynchronous: the command is written to the transport and the
    function returns immediately. No server response is awaited.

    @param [in] ulCommand  Command code (F_CMD_*).
    @param [in] ulData     32-bit argument.
    @return 0 on success, non-zero on transport error.
    @retval 0     On success.
    @retval !=0   Transport error. */
extern "C" APIRET APIENTRY PmcPostCommand(ULONG ulCommand, ULONG ulData)
{
  if (!pfnSendCmdToServer) return (APIRET)-1;
  return pfnSendCmdToServer(client_obj, ulCommand, ulData);
}

/*! @brief Send raw data to the server.

    Asynchronous: the buffer is written to the transport and the
    function returns immediately. Typically used to ship a payload
    after a command posted via PmcPostCommand.

    @param [in] pvData  Pointer to the payload.
    @param [in] cbData  Payload size in bytes.
    @return 0 on success, non-zero on transport error.
    @retval 0     On success.
    @retval !=0   Transport error. */
extern "C" APIRET APIENTRY PmcSendData(PVOID pvData, LONG cbData)
{
  if (!pfnSendDataToServer) return (APIRET)-1;
  return pfnSendDataToServer(client_obj, pvData, cbData);
}

/*! @brief Get raw data from the server.

    Blocking: reads the next block from the transport into pvBuffer
    and stores the number of bytes actually received in *pcbReceived.
    Equivalent to WinGetMsg in the OS/2 PM message-queue model.

    @param [out] pvBuffer     Destination buffer.
    @param [out] pcbReceived  Receives the number of bytes received.
    @param [in]  cbBuffer     Size of the destination buffer.
    @return 0 on success, non-zero on transport error.
    @retval 0     On success.
    @retval !=0   Transport error. */
extern "C" APIRET APIENTRY PmcGetData(PVOID pvBuffer, PLONG pcbReceived, LONG cbBuffer)
{
  if (!pfnRecvDataFromServer) return (APIRET)-1;
  return pfnRecvDataFromServer(client_obj, pvBuffer, pcbReceived, cbBuffer);
}

/*! @brief Send a command with a 32-bit parameter and wait for reply.

    Synchronous: the command is written to the transport, then the
    function blocks until the server returns a 32-bit reply and
    returns that reply. Equivalent to WinSendMsg in the OS/2 PM
    message-queue model.

    @param [in] ulCommand    Command code (F_CMD_*).
    @param [in] ulParameter  32-bit parameter.
    @return Server reply code, or a transport error code.
    @retval 0     Server-reported success.
    @retval !=0   Server-reported error or transport error. */
extern "C" APIRET APIENTRY PmcSendCommand(ULONG ulCommand, ULONG ulParameter)
{
  if (!pfnSendGenCmdToServer) return (APIRET)-1;
  return pfnSendGenCmdToServer(client_obj, ulCommand, ulParameter);
}

/*! @brief Send a command with a payload and wait for reply.

    Synchronous: writes the command, parameter and payload to the
    transport, then blocks until the server returns a 32-bit reply
    and returns that reply.

    @param [in] ulCommand    Command code (F_CMD_*).
    @param [in] ulParameter  32-bit parameter.
    @param [in] pvData       Pointer to the payload.
    @param [in] cbData       Payload size in bytes.
    @return Server reply code, or a transport error code.
    @retval 0     Server-reported success.
    @retval !=0   Server-reported error or transport error. */
extern "C" APIRET APIENTRY PmcSendCommandData(ULONG ulCommand, ULONG ulParameter,
                                              PVOID pvData, LONG cbData)
{
  if (!pfnSendGenCmdDataToServer) return (APIRET)-1;
  return pfnSendGenCmdDataToServer(client_obj, ulCommand, ulParameter, pvData, cbData);
}

/*! @brief OS/2 DLL entry point.

    On load (flag == 0):
      - reads fpm.ini into FPM_config,
      - selects backend: PM_COMM_BACKEND env, else fpm.ini "backend=",
        else "PMPIPE",
      - loads the backend and resolves client transport ordinals,
      - establishes the single process-wide connection to the FreePM
        server (auto-starting PMSERVER.EXE locally if needed).

    On unload (flag == 1): closes the connection, unloads the backend.

    If the backend cannot be loaded, the ordinals cannot be resolved,
    or the connection to the server cannot be established, PM is not
    available in this process and PMCLIENT is not loaded.

    @param [in] modHandle  DLL module handle (unused).
    @param [in] flag       0 = load, non-zero = unload.
    @return 1 on success, 0 on failure.
    @retval 1  Success; PM is available in this process.
    @retval 0  Failure; PM is not available. */
unsigned long _System _DLL_InitTerm(unsigned long modHandle, unsigned long flag)
{
  PSZ       pszValue;
  char      LoadError[256];
  char      szBackend[CCHMAXPATH];
  char     *serverName;
  APIRET    rc;

  (void)modHandle;

  if (flag)
  {
    /* terminate */
    if (pfnCloseServerConnection)
      pfnCloseServerConnection();
    client_obj = 0;
    if (hmodBE) DosFreeModule(hmodBE);
    hmodBE = 0;
    return 1;
  }

  /* init */
  FPM_config.Read("fpm.ini");

  /* Backend selection: env overrides config, config overrides default. */
  if (!DosScanEnv("PM_COMM_BACKEND", &pszValue))
    strcpy(szBackend, pszValue);
  else if (FPM_config.BackendName[0])
    strcpy(szBackend, FPM_config.BackendName);
  else
    strcpy(szBackend, "PMPIPE");

  rc = DosLoadModule(LoadError, sizeof(LoadError), szBackend, &hmodBE);
  if (rc) return 0;

  rc = DosQueryProcAddr(hmodBE, 1, 0, (PFN *)&pfnInitServerConnection);
  if (rc) return 0;

  rc = DosQueryProcAddr(hmodBE, 2, 0, (PFN *)&pfnCloseServerConnection);
  if (rc) return 0;

  rc = DosQueryProcAddr(hmodBE, 4, 0, (PFN *)&pfnSendCmdToServer);
  if (rc) return 0;

  rc = DosQueryProcAddr(hmodBE, 5, 0, (PFN *)&pfnSendDataToServer);
  if (rc) return 0;

  rc = DosQueryProcAddr(hmodBE, 6, 0, (PFN *)&pfnRecvDataFromServer);
  if (rc) return 0;

  rc = DosQueryProcAddr(hmodBE, 7, 0, (PFN *)&pfnSendGenCmdToServer);
  if (rc) return 0;

  rc = DosQueryProcAddr(hmodBE, 8, 0, (PFN *)&pfnSendGenCmdDataToServer);
  if (rc) return 0;

  /* Establish the single process-wide connection. */
  if (FPM_config.ExternMachineName[0])
    serverName = &FPM_config.ExternMachineName[0];
  else
    serverName = NULL;

  if (serverName == NULL && !LocalServerRunning())
  {
    ExecuteFreePMServer();
    DosSleep(1000);
  }

  rc = pfnInitServerConnection(serverName, &client_obj);
  if (rc) return 0;

  return 1;
}
