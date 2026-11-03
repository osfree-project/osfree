/*! @file init.cpp
    @brief PMCLIENT.DLL init/term.

    Loads the communication backend, establishes the process-wide
    connection to the FreePM server, performs the client handshake
    (F_CMD_CLIENT_CONNECT), and runs a keepalive thread that sends
    F_CMD_CLIENT_PING. On unload, sends F_CMD_CLIENT_EXIT.

    Reconnect-on-link-loss is NOT implemented yet (deferred).
*/

#if !defined(INCL_GPI)
#define INCL_GPI
#endif

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#if defined(__OS2__) || defined(__OS2V2__)
  #include <process.h>
#endif
#include "F_platform.h"
#include "Fc_config.hpp"
#include "F_config.hpp"
#include "F_cmd.h"

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
    PMC_Dbg("PMC: %s failed: 0x%x\n", func, err);
}
#else
#define PMC_Dbg(...) do {} while(0)
#define PMC_DbgError(func, err) do {} while(0)
#endif

/* ---- Backend function pointers (resolved at load) ----
   On OS/2 APIENTRY (_System) is a real type modifier and is valid in
   pointer types. On Win32 APIENTRY expands to __declspec(__pragma(...)),
   a function modifier that Watcom C rejects inside pointer declarators.
   Function pointers here are static and never exported, so __stdcall
   (same binary convention as __estdcall) is sufficient on Win32. */
#if defined(__OS2__) || defined(__OS2V2__)
  #define PMC_CB APIENTRY
#else
  #define PMC_CB __stdcall
#endif

static APIRET (PMC_CB *pfnInitServerConnection)(PCSZ, PULONG) = 0;
static APIRET (PMC_CB *pfnCloseServerConnection)(void) = 0;
static APIRET (PMC_CB *pfnSendCmdToServer)(ULONG, ULONG, ULONG) = 0;
static APIRET (PMC_CB *pfnSendDataToServer)(ULONG, PVOID, LONG) = 0;
static APIRET (PMC_CB *pfnRecvDataFromServer)(ULONG, PVOID, PLONG, LONG) = 0;
static APIRET (PMC_CB *pfnSendGenCmdToServer)(ULONG, ULONG, ULONG) = 0;
static APIRET (PMC_CB *pfnSendGenCmdDataToServer)(ULONG, ULONG, ULONG, PVOID, LONG) = 0;

#undef PMC_CB

static HMODULE hmodBE = 0;
/* DosQueryProcAddr takes PFN*, callers hold APIENTRY fn pointers.
   Wrap once here instead of casting at every call site. */
static APIRET ResolveProc(HMODULE hmod, ULONG ord, void **ppfn)
{
    PFN    pfn = 0;
    APIRET rc  = DosQueryProcAddr(hmod, ord, NULL, &pfn);
    *ppfn = (void *)pfn;
    return rc;
}
class F_ClientConfig FPM_config;

/*! @brief Process-wide connection to the FreePM server. */
static ULONG client_obj = 0;

/* ---- Client session state ---- */
static F_CLIENTCONFIG g_clientConfig;    /* idClient, ulInactive, ulGrace, flConfig */
static volatile BOOL  g_fKeepaliveRun = FALSE;
static volatile BOOL  g_fConnected    = FALSE;

/* ======================================================================== */
/*  Server startup / detection                                              */
/* ======================================================================== */

    Called when the local server is not running.
    @return 0 on success, non-zero on failure. */
static int ExecuteFreePMServer(void)
{
    RESULTCODES rcode;
    char        szFail[256];
    APIRET      rc;

    PMC_Dbg("PMC: ExecuteFreePMServer (PMSERVER.EXE)\n");
    rc = DosExecPgm(szFail, sizeof(szFail), EXEC_BACKGROUND,
                    (PSZ)"PMSERVER.EXE", NULL, &rcode, (PSZ)"PMSERVER.EXE");
    if (rc != 0) { PMC_DbgError("DosExecPgm", (int)rc); return -1; }
    return 0;
}

static int LocalServerRunning(void)
{
    HMTX     hmtx = NULL;
    APIRET   rc = DosOpenMutexSem((PSZ)FREEPM_MUTEX_NAME, &hmtx);
    if (rc == 0) DosCloseMutexSem(hmtx);
    PMC_Dbg("PMC: LocalServerRunning mutex rc=%u\n", rc);
    return (rc == 0);
}

/* ======================================================================== */
/*  Client handshake (CONNECT)                                              */
/* ======================================================================== */

static int PMC_ClientConnect(void)
{
    APIRET rc;
    LONG   len = 0;

    PMC_Dbg("PMC: sending F_CMD_CLIENT_CONNECT\n");

    rc = pfnSendCmdToServer(client_obj, F_CMD_CLIENT_CONNECT, 0);
    if (rc != 0) {
        PMC_Dbg("PMC: F_CMD_CLIENT_CONNECT send failed rc=%u\n", rc);
        return -1;
    }

    memset(&g_clientConfig, 0, sizeof(g_clientConfig));
    rc = pfnRecvDataFromServer(client_obj, &g_clientConfig, &len,
                               sizeof(g_clientConfig));
    if (rc != 0 || len != (LONG)sizeof(g_clientConfig)) {
        PMC_Dbg("PMC: CONNECT reply read failed rc=%u, len=%d\n",
                rc, (int)len);
        return -1;
    }

    if (g_clientConfig.idClient == 0) {
        PMC_Dbg("PMC: CONNECT rejected by server (idClient=0)\n");
        return -1;
    }

    PMC_Dbg("PMC: CONNECT ok, idClient=%lu, ulInactive=%lu, ulGrace=%lu, "
            "flConfig=0x%lx\n",
            (unsigned long)g_clientConfig.idClient,
            (unsigned long)g_clientConfig.ulInactive,
            (unsigned long)g_clientConfig.ulGrace,
            (unsigned long)g_clientConfig.flConfig);
    return 0;
}

/* ======================================================================== */
/*  Keepalive thread                                                        */
/* ======================================================================== */

#if defined(__OS2__) || defined(__OS2V2__)
static void KeepaliveProc(void *param)
#else
static unsigned long __stdcall KeepaliveProc(void *param)
#endif
{
    (void)param;

    /* Send a ping every ulInactive/2 seconds. Floor: 5 s. */
    ULONG pingSec = g_clientConfig.ulInactive / 2;
    if (pingSec < 5) pingSec = 5;

    PMC_Dbg("PMC: keepalive thread started (ping every %lu sec)\n",
            (unsigned long)pingSec);

    while (g_fKeepaliveRun) {
        DosSleep(pingSec * 1000);
        if (!g_fKeepaliveRun) break;
        if (!g_fConnected)     continue;

        pfnSendCmdToServer(client_obj, F_CMD_CLIENT_PING,
                           g_clientConfig.idClient);
    }

#if !(defined(__OS2__) || defined(__OS2V2__))
    return 0;
#endif
}

static void StartKeepalive(void)
{
    g_fKeepaliveRun = TRUE;
#if defined(__OS2__) || defined(__OS2V2__)
    _beginthread(KeepaliveProc, NULL, 65536, NULL);
#else
    CreateThread(NULL, 0, KeepaliveProc, NULL, 0, NULL);
#endif
}

static void StopKeepalive(void)
{
    if (!g_fKeepaliveRun) return;
    g_fKeepaliveRun = FALSE;
    /* Thread will notice within pingSec; may take up to pingSec/2 sec.
       We don't join it because DLL unload shouldn't block long. */
}

/* ======================================================================== */
/*  Exported transport wrappers                                             */
/* ======================================================================== */

extern "C" APIRET EXPENTRY PmcPostCommand(ULONG ulCommand, ULONG ulData)
{
    if (!pfnSendCmdToServer) return (APIRET)-1;
    return pfnSendCmdToServer(client_obj, ulCommand, ulData);
}

extern "C" APIRET EXPENTRY PmcSendData(PVOID pvData, LONG cbData)
{
    if (!pfnSendDataToServer) return (APIRET)-1;
    return pfnSendDataToServer(client_obj, pvData, cbData);
}

/*! @brief Get raw data from the server.

    Blocking: reads the next block from the transport.

    @param [out] pvBuffer     Destination buffer.
    @param [out] pcbReceived  Receives the number of bytes received.
    @param [in]  cbBuffer     Size of the destination buffer.
    @return 0 on success, non-zero on transport error. */
extern "C" APIRET EXPENTRY PmcGetData(PVOID pvBuffer, PLONG pcbReceived, LONG cbBuffer)
{
    if (!pfnRecvDataFromServer) return (APIRET)-1;
    return pfnRecvDataFromServer(client_obj, pvBuffer, pcbReceived, cbBuffer);
}

/*! @brief Send a command with a 32-bit parameter and wait for reply.

    Synchronous: blocks until the server returns a 32-bit reply.

    @param [in] ulCommand    Command code.
    @param [in] ulParameter  32-bit parameter.
    @return Server reply code, or a transport error code. */
extern "C" APIRET EXPENTRY PmcSendCommand(ULONG ulCommand, ULONG ulParameter)
{
    if (!pfnSendGenCmdToServer) return (APIRET)-1;
    return pfnSendGenCmdToServer(client_obj, ulCommand, ulParameter);
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
    if (!pfnSendGenCmdDataToServer) return (APIRET)-1;
    return pfnSendGenCmdDataToServer(client_obj, ulCommand, ulParameter,
                                     pvData, cbData);
}

/* ======================================================================== */
/*  Server init/term                                                        */
/* ======================================================================== */

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
    int    startRc;

    PMC_Dbg("PMC: PMC_ServerInit started\n");

    FPM_config.Read("fpm.ini");

    if (DosScanEnv((PSZ)"PM_COMM_BACKEND", &pszValue) == 0 && pszValue != NULL)
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

    rc = DosLoadModule(NULL, 0, szBackend, &hmodBE);
    if (rc) { PMC_DbgError("DosLoadModule", (int)rc); return 0; }

    if (ResolveProc(hmodBE, 1, (void **)&pfnInitServerConnection))   return 0;
    if (ResolveProc(hmodBE, 2, (void **)&pfnCloseServerConnection))  return 0;
    if (ResolveProc(hmodBE, 4, (void **)&pfnSendCmdToServer))        return 0;
    if (ResolveProc(hmodBE, 5, (void **)&pfnSendDataToServer))       return 0;
    if (ResolveProc(hmodBE, 6, (void **)&pfnRecvDataFromServer))     return 0;
    if (ResolveProc(hmodBE, 7, (void **)&pfnSendGenCmdToServer))     return 0;
    if (ResolveProc(hmodBE, 8, (void **)&pfnSendGenCmdDataToServer)) return 0;

    if (FPM_config.ExternMachineName[0])
        serverName = &FPM_config.ExternMachineName[0];
    else
        serverName = NULL;

    if (serverName == NULL && !LocalServerRunning())
    {
        PMC_Dbg("PMC: Local server not running, attempting to start...\n");
        startRc = ExecuteFreePMServer();
        if (startRc == 0) {
            DosSleep(1000);
        } else {
            PMC_Dbg("PMC: Failed to start PMSERVER.EXE.\n");
        }
    }

    rc = pfnInitServerConnection(serverName, &client_obj);
    if (rc) {
        PMC_Dbg("PMC: pfnInitServerConnection rc=0x%x\n", rc);
        return 0;
    }

    /* NEW: client handshake */
    if (PMC_ClientConnect() != 0) {
        PMC_Dbg("PMC: CONNECT failed, aborting init\n");
        pfnCloseServerConnection();
        client_obj = 0;
        return 0;
    }

    g_fConnected = TRUE;

    /* NEW: start keepalive */
    if (g_clientConfig.flConfig & F_CFG_PING_REQUIRED)
        StartKeepalive();

    PMC_Dbg("PMC: ServerInit completed\n");
    return 1;
}

static void PMC_ServerTerm(void)
{
    PMC_Dbg("PMC: PMC_ServerTerm started\n");

    /* NEW: stop keepalive, send EXIT */
    StopKeepalive();

    if (g_fConnected && pfnSendCmdToServer) {
        PMC_Dbg("PMC: sending F_CMD_CLIENT_EXIT\n");
        pfnSendCmdToServer(client_obj, F_CMD_CLIENT_EXIT,
                           g_clientConfig.idClient);
        g_fConnected = FALSE;
    }

    if (pfnCloseServerConnection) {
        PMC_Dbg("PMC: calling pfnCloseServerConnection\n");
        pfnCloseServerConnection();
    }
    client_obj = 0;

    if (hmodBE) {
        DosFreeModule(hmodBE);
        hmodBE = 0;
    }
    PMC_Dbg("PMC: ServerTerm completed\n");
}

/* ======================================================================== */
/*  Platform entry points                                                   */
/* ======================================================================== */

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
    if (flag) { PMC_ServerTerm(); return 1; }
    return PMC_ServerInit() ? 1 : 0;
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
    (void)hInstance; (void)lpReserved;
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (!PMC_ServerInit()) { ExitProcess(1); return FALSE; }
        return TRUE;
    }
    if (fdwReason == DLL_PROCESS_DETACH) {
        PMC_ServerTerm();
    }
    return TRUE;
}
#endif
