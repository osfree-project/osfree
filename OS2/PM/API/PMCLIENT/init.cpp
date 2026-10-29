/*! @file init.cpp
    @brief PMCLIENT.DLL init/term.

    Loads the communication backend, establishes the process-wide
    connection to the FreePM server, and exports the Pmc* transport API.
*/

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "F_platform.hpp"
#include "Fc_config.hpp"
#include "F_config.hpp"

/*! @def PMC_DEBUG
    @brief Enable (1) or disable (0) debug logging. */
#ifndef PMC_DEBUG
#define PMC_DEBUG 1
#endif

#if PMC_DEBUG
/*! @brief Print a debug message to stderr and OutputDebugString.

    @param [in] fmt  printf-style format string.
    @param [in] ...  Arguments for the format string. */
static void PMC_Dbg(const char *fmt, ...)
{
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
#if defined(__WIN32__) || defined(_WIN32)
    OutputDebugStringA(buf);
#endif
    fprintf(stderr, "%s", buf);
    fflush(stderr);
}

/*! @brief Print a debug message followed by a human-readable OS error.

    @param [in] func  Name of the failed function.
    @param [in] err   OS error code (GetLastError or APIRET). */
static void PMC_DbgError(const char *func, int err)
{
    char errBuf[256] = {0};
#if defined(__WIN32__) || defined(_WIN32)
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   errBuf, sizeof(errBuf), NULL);
    size_t len = strlen(errBuf);
    while(len > 0 && (errBuf[len-1] == '\n' || errBuf[len-1] == '\r')) {
        errBuf[len-1] = '\0';
        len--;
    }
#else
    snprintf(errBuf, sizeof(errBuf), "OS/2 error %d", err);
#endif
    PMC_Dbg("PMC: %s failed: 0x%x (%s)\n", func, err, errBuf);
}
#else
#define PMC_Dbg(...) do {} while(0)
#define PMC_DbgError(func, err) do {} while(0)
#endif

/*! @brief Init backend entry point. */
static APIRET (APIENTRY *pfnInitServerConnection)(PCSZ pcszServerName, PULONG pulObject) = 0;
/*! @brief Close backend entry point. */
static APIRET (APIENTRY *pfnCloseServerConnection)(void) = 0;
/*! @brief Post command backend entry point. */
static APIRET (APIENTRY *pfnSendCmdToServer)(ULONG ulObject, ULONG ulCommand, ULONG ulData) = 0;
/*! @brief Send data backend entry point. */
static APIRET (APIENTRY *pfnSendDataToServer)(ULONG ulObject, PVOID pvData, LONG cbData) = 0;
/*! @brief Get data backend entry point. */
static APIRET (APIENTRY *pfnRecvDataFromServer)(ULONG ulObject, PVOID pvBuffer, PLONG pcbReceived, LONG cbBuffer) = 0;
/*! @brief Send generic command backend entry point. */
static APIRET (APIENTRY *pfnSendGenCmdToServer)(ULONG ulObject, ULONG ulCommand, ULONG ulParameter) = 0;
/*! @brief Send generic command with data backend entry point. */
static APIRET (APIENTRY *pfnSendGenCmdDataToServer)(ULONG ulObject, ULONG ulCommand, ULONG ulParameter, PVOID pvData, LONG cbData) = 0;

/*! @brief Handle of the loaded communication backend. */
static PMC_HMODULE hmodBE = 0;

/*! @brief Client configuration, read once at DLL load. */
class F_ClientConfig FPM_config;

/*! @brief Process-wide connection to the FreePM server. */
static ULONG client_obj = 0;

/* ------------------------------------------------------------------ */
/* Server startup / detection                                          */
/* ------------------------------------------------------------------ */

/*! @brief Start PMSERVER.EXE as a detached process.

    Called when the local server is not running.
    @return 0 on success, non-zero on failure. */
static int ExecuteFreePMServer(void)
{
    PMC_Dbg("PMC: ExecuteFreePMServer (PMSERVER.EXE)\n");
    APIRET rc = PMC_StartProcess("PMSERVER.EXE");
    if (rc != 0) {
        PMC_DbgError("PMC_StartProcess", (int)rc);
        return -1;
    }
    return 0;
}

/*! @brief Check if a local FreePM server is already running.

    Opens the named mutex; if successful, the server is running.

    @return 1 if the server is running, 0 otherwise. */
static int LocalServerRunning(void)
{
    PMC_HMTX hmtx = NULL;
    APIRET   rc;

    rc = PMC_MutexOpen(FREEPM_MUTEX_NAME, &hmtx);
    if (rc == 0)
        PMC_MutexClose(hmtx);

    PMC_Dbg("PMC: LocalServerRunning mutex rc=%u\n", rc);
    return (rc == 0);
}

/* ------------------------------------------------------------------ */
/* Exported transport wrappers                                         */
/* ------------------------------------------------------------------ */

/*! @brief Post a command with a 32-bit argument to the server.

    Asynchronous: returns immediately after sending.

    @param [in] ulCommand  Command code.
    @param [in] ulData     32-bit argument.
    @return 0 on success, non-zero on transport error. */
extern "C" APIRET EXPENTRY PmcPostCommand(ULONG ulCommand, ULONG ulData)
{
  PMC_Dbg("PMC: PmcPostCommand(0x%x, 0x%x) IN\n", ulCommand, ulData);
  if (!pfnSendCmdToServer) {
      PMC_Dbg("PMC: PmcPostCommand ERROR: pfnSendCmdToServer is NULL!\n");
      return (APIRET)-1;
  }
  APIRET rc = pfnSendCmdToServer(client_obj, ulCommand, ulData);
  PMC_Dbg("PMC: PmcPostCommand OUT -> %u\n", rc);
  return rc;
}

/*! @brief Send raw data to the server.

    Asynchronous: returns immediately after sending.

    @param [in] pvData  Pointer to the payload.
    @param [in] cbData  Payload size in bytes.
    @return 0 on success, non-zero on transport error. */
extern "C" APIRET EXPENTRY PmcSendData(PVOID pvData, LONG cbData)
{
  PMC_Dbg("PMC: PmcSendData(%p, %d) IN\n", pvData, cbData);
  if (!pfnSendDataToServer) {
      PMC_Dbg("PMC: PmcSendData ERROR: pfnSendDataToServer is NULL!\n");
      return (APIRET)-1;
  }
  APIRET rc = pfnSendDataToServer(client_obj, pvData, cbData);
  PMC_Dbg("PMC: PmcSendData OUT -> %u\n", rc);
  return rc;
}

/*! @brief Get raw data from the server.

    Blocking: reads the next block from the transport.

    @param [out] pvBuffer     Destination buffer.
    @param [out] pcbReceived  Receives the number of bytes received.
    @param [in]  cbBuffer     Size of the destination buffer.
    @return 0 on success, non-zero on transport error. */
extern "C" APIRET EXPENTRY PmcGetData(PVOID pvBuffer, PLONG pcbReceived, LONG cbBuffer)
{
  PMC_Dbg("PMC: PmcGetData(%p, %p, %d) IN\n", pvBuffer, pcbReceived, cbBuffer);
  if (!pfnRecvDataFromServer) {
      PMC_Dbg("PMC: PmcGetData ERROR: pfnRecvDataFromServer is NULL!\n");
      return (APIRET)-1;
  }
  APIRET rc = pfnRecvDataFromServer(client_obj, pvBuffer, pcbReceived, cbBuffer);
  PMC_Dbg("PMC: PmcGetData OUT -> %u (received %d)\n", rc, (pcbReceived ? *pcbReceived : 0));
  return rc;
}

/*! @brief Send a command with a 32-bit parameter and wait for reply.

    Synchronous: blocks until the server returns a 32-bit reply.

    @param [in] ulCommand    Command code.
    @param [in] ulParameter  32-bit parameter.
    @return Server reply code, or a transport error code. */
extern "C" APIRET EXPENTRY PmcSendCommand(ULONG ulCommand, ULONG ulParameter)
{
  PMC_Dbg("PMC: PmcSendCommand(0x%x, 0x%x) IN\n", ulCommand, ulParameter);
  if (!pfnSendGenCmdToServer) {
      PMC_Dbg("PMC: PmcSendCommand ERROR: pfnSendGenCmdToServer is NULL!\n");
      return (APIRET)-1;
  }
  APIRET rc = pfnSendGenCmdToServer(client_obj, ulCommand, ulParameter);
  PMC_Dbg("PMC: PmcSendCommand OUT -> %u\n", rc);
  return rc;
}

/*! @brief Send a command with a payload and wait for reply.

    Synchronous: blocks until the server returns a 32-bit reply.

    @param [in] ulCommand    Command code.
    @param [in] ulParameter  32-bit parameter.
    @param [in] pvData       Pointer to the payload.
    @param [in] cbData       Payload size in bytes.
    @return Server reply code, or a transport error code. */
extern "C" APIRET EXPENTRY PmcSendCommandData(ULONG ulCommand, ULONG ulParameter,
                                              PVOID pvData, LONG cbData)
{
  PMC_Dbg("PMC: PmcSendCommandData(0x%x, 0x%x, %p, %d) IN\n", ulCommand, ulParameter, pvData, cbData);
  if (!pfnSendGenCmdDataToServer) {
      PMC_Dbg("PMC: PmcSendCommandData ERROR: pfnSendGenCmdDataToServer is NULL!\n");
      return (APIRET)-1;
  }
  APIRET rc = pfnSendGenCmdDataToServer(client_obj, ulCommand, ulParameter, pvData, cbData);
  PMC_Dbg("PMC: PmcSendCommandData OUT -> %u\n", rc);
  return rc;
}

/* ------------------------------------------------------------------ */
/* Common init/term body                                               */
/* ------------------------------------------------------------------ */

/*! @brief Load the backend, resolve ordinals, and connect to the server.

    Called once from DllMain (Win32) or _DLL_InitTerm (OS/2).

    @return 1 on success, 0 on failure.
    @retval 1  Success.
    @retval 0  Failure. */
static int PMC_ServerInit(void)
{
    PSZ    pszValue = NULL;
    char   szBackend[256];
    char  *serverName;
    APIRET rc;
    int startRc;

    PMC_Dbg("PMC: PMC_ServerInit started\n");

    FPM_config.Read("fpm.ini");
    PMC_Dbg("PMC: fpm.ini read\n");

    /* Backend selection: env overrides config, config overrides default. */
    if (PMC_ScanEnv("PM_COMM_BACKEND", &pszValue) == 0 && pszValue != NULL)
        strncpy(szBackend, pszValue, sizeof(szBackend) - 1);
    else if (FPM_config.BackendName[0])
        strncpy(szBackend, FPM_config.BackendName, sizeof(szBackend) - 1);
    else {
#if defined(__WIN32__) || defined(_WIN32)
        strcpy(szBackend, "PMTCP");
#else
        strcpy(szBackend, "PMPIPE");
#endif
    }
    szBackend[sizeof(szBackend) - 1] = 0;
    
    PMC_Dbg("PMC: Backend selected = %s\n", szBackend);

    rc = PMC_LoadModule(szBackend, &hmodBE);
    if (rc) {
#if defined(__WIN32__) || defined(_WIN32)
        PMC_DbgError("PMC_LoadModule", GetLastError());
#else
        PMC_DbgError("PMC_LoadModule", (int)rc);
#endif
        return 0;
    }
    PMC_Dbg("PMC: PMC_LoadModule rc=0\n");

    if (PMC_QueryOrdinal(hmodBE, 1, (void **)&pfnInitServerConnection)) { PMC_Dbg("PMC: Missing ord 1 (InitServerConnection)\n"); return 0; }
    if (PMC_QueryOrdinal(hmodBE, 2, (void **)&pfnCloseServerConnection)) { PMC_Dbg("PMC: Missing ord 2 (CloseServerConnection)\n"); return 0; }
    if (PMC_QueryOrdinal(hmodBE, 4, (void **)&pfnSendCmdToServer)) { PMC_Dbg("PMC: Missing ord 4 (SendCmdToServer)\n"); return 0; }
    if (PMC_QueryOrdinal(hmodBE, 5, (void **)&pfnSendDataToServer)) { PMC_Dbg("PMC: Missing ord 5 (SendDataToServer)\n"); return 0; }
    if (PMC_QueryOrdinal(hmodBE, 6, (void **)&pfnRecvDataFromServer)) { PMC_Dbg("PMC: Missing ord 6 (RecvDataFromServer)\n"); return 0; }
    if (PMC_QueryOrdinal(hmodBE, 7, (void **)&pfnSendGenCmdToServer)) { PMC_Dbg("PMC: Missing ord 7 (SendGenCmdToServer)\n"); return 0; }
    if (PMC_QueryOrdinal(hmodBE, 8, (void **)&pfnSendGenCmdDataToServer)) { PMC_Dbg("PMC: Missing ord 8 (SendGenCmdDataToServer)\n"); return 0; }
    
    PMC_Dbg("PMC: All ordinals resolved\n");

    /* Establish the single process-wide connection. */
    if (FPM_config.ExternMachineName[0])
        serverName = &FPM_config.ExternMachineName[0];
    else
        serverName = NULL;

    PMC_Dbg("PMC: serverName = %s\n", serverName ? serverName : "NULL (local)");

    if (serverName == NULL && !LocalServerRunning())
    {
        PMC_Dbg("PMC: Local server not running, attempting to start...\n");
        startRc = ExecuteFreePMServer();
        if (startRc != 0) {
            PMC_Dbg("PMC: Failed to start PMSERVER.EXE.\n");
        } else {
            PMC_Dbg("PMC: PMSERVER.EXE launched, waiting 1000ms...\n");
            PMC_Sleep(1000);
        }
    }

    PMC_Dbg("PMC: Calling pfnInitServerConnection\n");
    rc = pfnInitServerConnection(serverName, &client_obj);
    if (rc) {
        PMC_Dbg("PMC: pfnInitServerConnection returned rc=0x%x\n", rc);
        if (rc == 101) {
            PMC_Dbg("PMC: This usually means HAND_SHAKE_ERROR (protocol mismatch or server crashed during handshake).\n");
        }
#if defined(__WIN32__) || defined(_WIN32)
        if (rc == 10061) {
            PMC_Dbg("PMC: This usually means WSAECONNREFUSED (PMSERVER.EXE is not listening on port 9876).\n");
        }
#endif
        return 0;
    }

    PMC_Dbg("PMC: ServerInit completed successfully\n");
    return 1;
}

/*! @brief Close the connection and unload the backend.

    Called once from DllMain (Win32) or _DLL_InitTerm (OS/2). */
static void PMC_ServerTerm(void)
{
    PMC_Dbg("PMC: PMC_ServerTerm started\n");
    if (pfnCloseServerConnection)
    {
        PMC_Dbg("PMC: Calling pfnCloseServerConnection\n");
        pfnCloseServerConnection();
    }
    client_obj = 0;
    if (hmodBE) 
    {
        PMC_Dbg("PMC: Freeing backend module\n");
        PMC_FreeModule(hmodBE);
    }
    hmodBE = 0;
    PMC_Dbg("PMC: ServerTerm completed\n");
}

/* ------------------------------------------------------------------ */
/* Platform entry points                                               */
/* ------------------------------------------------------------------ */

#if defined(__OS2__) || defined(__OS2V2__)

/*! @brief OS/2 DLL entry point.

    @param [in] modHandle  DLL module handle (unused).
    @param [in] flag       0 = load, non-zero = unload.
    @return 1 on success, 0 on failure.
    @retval 1  Success.
    @retval 0  Failure. */
unsigned long _System _DLL_InitTerm(unsigned long modHandle, unsigned long flag)
{
    (void)modHandle;
    PMC_Dbg("PMC: _DLL_InitTerm flag=%lu\n", flag);
    if (flag) { PMC_ServerTerm(); return 1; }
    if (!PMC_ServerInit()) {
        PMC_Dbg("PMC: _DLL_InitTerm returning 0 to abort load\n");
        return 0;
    }
    return 1;
}

#else /* Win32 */

/*! @brief Win32 DLL entry point.

    @param [in] hInstance   DLL instance handle (unused).
    @param [in] fdwReason   Reason for calling.
    @param [in] lpReserved  Reserved (unused).
    @return TRUE on success, FALSE on initialization failure.
    @retval TRUE   Success.
    @retval FALSE  Failure. */
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpReserved)
{
    (void)hInstance;
    (void)lpReserved;

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        PMC_Dbg("PMC: DllMain PROCESS_ATTACH\n");
        int ret = PMC_ServerInit();
        if (!ret) {
            PMC_Dbg("PMC: DllMain PMC_ServerInit returned 0, aborting DLL load.\n");
            /* WORKAROUND: Watcom CRT crashes when DllMain returns FALSE.
               Calling ExitProcess directly prevents the OS from trying to unwind
               the CRT state, which causes the EIP=0x00000000 trap. */
            ExitProcess(1);
            return FALSE;
        }
        return TRUE;
    }
    if (fdwReason == DLL_PROCESS_DETACH)
    {
        PMC_Dbg("PMC: DllMain PROCESS_DETACH\n");
        PMC_ServerTerm();
    }
    return TRUE;
}

#endif
