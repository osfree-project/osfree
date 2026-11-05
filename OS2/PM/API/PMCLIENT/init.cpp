/*! @file init.cpp
    @brief PMCLIENT.DLL init/term.

    Loads the communication backend, establishes the process-wide
    connection to the FreePM server, performs the client handshake
    (F_CMD_CLIENT_CONNECT), and runs a keepalive thread that sends
    F_CMD_CLIENT_PING. On unload, sends F_CMD_CLIENT_EXIT.

    Reconnect-on-link-loss: when a transport error is detected, a
    background thread retries _InitServerConnection + RECONNECT(idClient)
    until F_CLIENTCONFIG.ulGrace seconds have elapsed. On giving up, the
    session is considered lost.

    Shutdown: PMTCP's DllMain(DETACH) calls back into PMC_ShutdownCallback
    (registered via _InitServerConnection), which sends F_CMD_CLIENT_EXIT
    over the still-live socket. atexit is NOT used: on OS/2 and Win32
    atexit inside a DLL runs after the OS has already started unloading
    dependent DLLs (PMTCP), so pfnSendCmdToServer would target freed code.
*/

#if !defined(INCL_GPI)
#define INCL_GPI
#endif

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <process.h>

#include "F_platform.h"
#include "Fc_config.hpp"
#include "F_config.h"
#include "F_cmd.h"
#include "F_debug.h"

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

static APIRET (PMC_CB *pfnInitServerConnection)(PCSZ, PULONG, void *) = 0;
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

/*! @brief Process-wide connection to the FreePM server. Protected by
    g_hmtxState. */
static ULONG client_obj = 0;

/* ---- Client session state ---- */
static F_CLIENTCONFIG g_clientConfig;    /* idClient, ulInactive, ulGrace, flConfig */
static volatile BOOL  g_fKeepaliveRun = FALSE;
static volatile BOOL  g_fConnected    = FALSE;
static volatile BOOL  g_fReconnecting = FALSE;
static volatile BOOL  g_fTerminated   = FALSE;

/*! @brief Protects client_obj, g_fConnected, g_fReconnecting and
    g_clientConfig against concurrent access from public wrappers,
    the keepalive thread, the reconnect thread, and the shutdown
    callback. Never hold while calling any pfn* (I/O may block). */
static HMTX g_hmtxState = NULL;

static void PMC_ServerTerm(void);
static void APIENTRY PMC_ShutdownCallback(void);

/* ======================================================================== */
/*  State lock                                                              */
/* ======================================================================== */

static void LockState(void)
{
    if (g_hmtxState) DosRequestMutexSem(g_hmtxState, SEM_INDEFINITE_WAIT);
}

static void UnlockState(void)
{
    if (g_hmtxState) DosReleaseMutexSem(g_hmtxState);
}

/* ======================================================================== */
/*  Server startup / detection                                              */
/* ======================================================================== */

/*    Called when the local server is not running.
    @return 0 on success, non-zero on failure. */
static int ExecuteFreePMServer(void)
{
    RESULTCODES rcode;
    char        szFail[256];
    APIRET      rc;

    F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
               "PMC: ExecuteFreePMServer (PMSERVER.EXE)");
    rc = DosExecPgm(szFail, sizeof(szFail), EXEC_BACKGROUND,
                    (PSZ)"PMSERVER.EXE", NULL, &rcode, (PSZ)"PMSERVER.EXE");
    if (rc != 0) {
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: DosExecPgm failed: 0x%x", (int)rc);
        return -1;
    }
    return 0;
}

static int LocalServerRunning(void)
{
    HMTX     hmtx = NULL;
    APIRET   rc = DosOpenMutexSem((PSZ)FREEPM_MUTEX_NAME, &hmtx);
    if (rc == 0) DosCloseMutexSem(hmtx);
    F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
               "PMC: LocalServerRunning mutex rc=%u", rc);
    return (rc == 0);
}

/* ======================================================================== */
/*  Client handshake (CONNECT)                                              */
/* ======================================================================== */

static int PMC_ClientConnect(void)
{
    APIRET rc;
    LONG   len = 0;
    int    ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: ClientConnect");

    F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
               "PMC: sending F_CMD_CLIENT_CONNECT");

    rc = pfnSendCmdToServer(client_obj, F_CMD_CLIENT_CONNECT, 0);
    if (rc != 0) {
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: F_CMD_CLIENT_CONNECT send failed rc=%u", rc);
        F_DbgCtxExitf(ctx, "send failed rc=%u", rc);
        return -1;
    }

    memset(&g_clientConfig, 0, sizeof(g_clientConfig));
    rc = pfnRecvDataFromServer(client_obj, &g_clientConfig, &len,
                               sizeof(g_clientConfig));
    if (rc != 0 || len != (LONG)sizeof(g_clientConfig)) {
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: CONNECT reply read failed rc=%u, len=%d",
                   rc, (int)len);
        F_DbgCtxExitf(ctx, "reply read failed rc=%u, len=%d", rc, (int)len);
        return -1;
    }

    if (g_clientConfig.idClient == 0) {
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: CONNECT rejected by server (idClient=0)");
        F_DbgCtxExitf(ctx, "rejected (idClient=0)");
        return -1;
    }

    F_DbgCtxExitf(ctx,
                  "ok idClient=%lu, ulInactive=%lu, ulGrace=%lu, flConfig=0x%lx",
                  (unsigned long)g_clientConfig.idClient,
                  (unsigned long)g_clientConfig.ulInactive,
                  (unsigned long)g_clientConfig.ulGrace,
                  (unsigned long)g_clientConfig.flConfig);
    return 0;
}

/* ======================================================================== */
/*  Transport error handling / reconnect                                    */
/* ======================================================================== */

static void StartReconnect(void);

/*! @brief Called from any public wrapper or keepalive when a
    transport-level failure is detected. Flips g_fConnected to FALSE
    and, if no reconnect is in progress, starts the reconnect thread. */
static void OnTransportError(void)
{
    BOOL start = FALSE;

    LockState();
    if (g_fConnected && !g_fReconnecting && !g_fTerminated) {
        g_fConnected = FALSE;
        start = TRUE;
    }
    UnlockState();

    if (start) {
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: transport error, starting reconnect");
        StartReconnect();
    }
}

/*! @brief Reconnect thread: retries _InitServerConnection + RECONNECT
    until ulGrace seconds have elapsed or the process is terminating.

    @param [in] param  idClient as ULONG cast to void*. */
#if defined(__OS2__) || defined(__OS2V2__)
static void ReconnectThreadProc(void *param)
#else
static unsigned long __stdcall ReconnectThreadProc(void *param)
#endif
{
    ULONG  idClient = (ULONG)param;
    ULONG  grace;
    ULONG  deadline;
    ULONG  wait;
    APIRET rc;
    ULONG  newobj;
    LONG   len;
    int    ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: ReconnectThreadProc(idClient=%lu)",
                         (unsigned long)idClient);

    LockState();
    grace = g_clientConfig.ulGrace;
    UnlockState();
    if (grace == 0) grace = 5;
    wait = (grace / 3) * 1000;
    if (wait < 1000) wait = 1000;

    deadline = (ULONG)time(NULL) + grace;

    while (!g_fTerminated && (ULONG)time(NULL) < deadline) {
        DosSleep(wait);
        if (g_fTerminated) break;

        newobj = 0;
        rc = pfnInitServerConnection(NULL, &newobj,
                                     (void *)PMC_ShutdownCallback);
        if (rc == 0 && newobj != 0) {
            APIRET rc2 = pfnSendCmdToServer(newobj, F_CMD_CLIENT_RECONNECT,
                                            idClient);
            if (rc2 == 0) {
                F_CLIENTCONFIG cfg;
                len = 0;
                rc2 = pfnRecvDataFromServer(newobj, &cfg, &len, sizeof(cfg));
                if (rc2 == 0 && len == (LONG)sizeof(cfg) &&
                    cfg.idClient == idClient) {
                    LockState();
                    client_obj     = newobj;
                    g_clientConfig = cfg;
                    g_fConnected   = TRUE;
                    g_fReconnecting = FALSE;
                    UnlockState();
                    F_DbgCtxExitf(ctx, "reconnect ok idClient=%lu",
                                  (unsigned long)idClient);
#if !(defined(__OS2__) || defined(__OS2V2__))
                    return 0;
#else
                    return;
#endif
                }
            }
            pfnCloseServerConnection();
        }
    }

    LockState();
    g_fConnected   = FALSE;
    g_fReconnecting = FALSE;
    client_obj     = 0;
    UnlockState();
    F_DbgCtxExitf(ctx, "grace expired");

#if !(defined(__OS2__) || defined(__OS2V2__))
    return 0;
#endif
}

static void StartReconnect(void)
{
    ULONG  idClient;

    LockState();
    if (g_fReconnecting || g_fTerminated || g_clientConfig.idClient == 0) {
        UnlockState();
        return;
    }
    g_fReconnecting = TRUE;
    idClient = g_clientConfig.idClient;
    UnlockState();

#if defined(__OS2__) || defined(__OS2V2__)
    {
        int thrc = _beginthread(ReconnectThreadProc, NULL, 65536,
                                (void *)idClient);
        if (thrc == -1) {
            F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                       "PMC: _beginthread for reconnect failed");
            LockState();
            g_fReconnecting = FALSE;
            UnlockState();
        }
    }
#else
    if (CreateThread(NULL, 0, ReconnectThreadProc,
                     (void *)idClient, 0, NULL) == NULL) {
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: CreateThread for reconnect failed");
        LockState();
        g_fReconnecting = FALSE;
        UnlockState();
    }
#endif
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

    F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
               "PMC: keepalive thread started (ping every %lu sec)",
               (unsigned long)pingSec);

    while (g_fKeepaliveRun) {
        ULONG obj = 0;
        ULONG idc = 0;
        BOOL  connected = FALSE;

        DosSleep(pingSec * 1000);
        if (!g_fKeepaliveRun) break;

        LockState();
        if (g_fConnected) {
            obj = client_obj;
            idc = g_clientConfig.idClient;
            connected = TRUE;
        }
        UnlockState();
        if (!connected || obj == 0) continue;

        if (pfnSendCmdToServer(obj, F_CMD_CLIENT_PING, idc) != 0) {
            OnTransportError();
        }
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
    APIRET rc;
    ULONG  obj;
    int    ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: PmcPostCommand(cmd=0x%lx, data=0x%lx)",
                         (unsigned long)ulCommand, (unsigned long)ulData);

    if (!pfnSendCmdToServer) {
        F_DbgCtxExitf(ctx, "no backend");
        return (APIRET)-1;
    }
    LockState();
    obj = client_obj;
    UnlockState();
    if (obj == 0) {
        F_DbgCtxExitf(ctx, "not connected");
        return (APIRET)-1;
    }

    rc = pfnSendCmdToServer(obj, ulCommand, ulData);
    if (rc != 0) OnTransportError();
    F_DbgCtxExitf(ctx, "rc=0x%lx", (unsigned long)rc);
    return rc;
}

extern "C" APIRET EXPENTRY PmcSendData(PVOID pvData, LONG cbData)
{
    APIRET rc;
    ULONG  obj;
    int    ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: PmcSendData(data=%p, cbData=%ld)",
                         pvData, (long)cbData);

    if (!pfnSendDataToServer) {
        F_DbgCtxExitf(ctx, "no backend");
        return (APIRET)-1;
    }
    LockState();
    obj = client_obj;
    UnlockState();
    if (obj == 0) {
        F_DbgCtxExitf(ctx, "not connected");
        return (APIRET)-1;
    }

    rc = pfnSendDataToServer(obj, pvData, cbData);
    if (rc != 0) OnTransportError();
    F_DbgCtxExitf(ctx, "rc=0x%lx", (unsigned long)rc);
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
    APIRET rc;
    ULONG  obj;
    int    ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: PmcGetData(buffer=%p, cbBuffer=%ld)",
                         pvBuffer, (long)cbBuffer);

    if (!pfnRecvDataFromServer) {
        F_DbgCtxExitf(ctx, "no backend");
        return (APIRET)-1;
    }
    LockState();
    obj = client_obj;
    UnlockState();
    if (obj == 0) {
        F_DbgCtxExitf(ctx, "not connected");
        return (APIRET)-1;
    }

    rc = pfnRecvDataFromServer(obj, pvBuffer, pcbReceived, cbBuffer);
    if (rc != 0) {
        OnTransportError();
        F_DbgCtxExitf(ctx, "Recv failed rc=0x%lx", (unsigned long)rc);
        return rc;
    }
    F_DbgCtxExitf(ctx, "rc=0, received=%ld", (long)*pcbReceived);
    return rc;
}

/*! @brief Send a command with a 32-bit parameter and wait for reply.

    Synchronous: blocks until the server returns a 32-bit reply.

    @param [in] ulCommand    Command code.
    @param [in] ulParameter  32-bit parameter.
    @return Server reply code, or (APIRET)-1 on transport error. */
extern "C" APIRET EXPENTRY PmcSendCommand(ULONG ulCommand, ULONG ulParameter)
{
    APIRET rc;
    ULONG  obj;
    int    ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: PmcSendCommand(cmd=0x%lx, par=0x%lx)",
                         (unsigned long)ulCommand, (unsigned long)ulParameter);

    if (!pfnSendGenCmdToServer) {
        F_DbgCtxExitf(ctx, "no backend");
        return (APIRET)-1;
    }
    LockState();
    obj = client_obj;
    UnlockState();
    if (obj == 0) {
        F_DbgCtxExitf(ctx, "not connected");
        return (APIRET)-1;
    }

    rc = pfnSendGenCmdToServer(obj, ulCommand, ulParameter);
    if (rc == (APIRET)-1) OnTransportError();
    F_DbgCtxExitf(ctx, "rc=0x%lx", (unsigned long)rc);
    return rc;
}

/*! @brief Send a command with a payload and wait for reply.

    Synchronous: blocks until the server returns a 32-bit reply.

    @param [in] ulCommand    Command code.
    @param [in] ulParameter  32-bit parameter.
    @param [in] pvData       Pointer to the payload.
    @param [in] cbData       Payload size in bytes.
    @return Server reply code, or (APIRET)-1 on transport error. */
extern "C" APIRET EXPENTRY PmcSendCommandData(ULONG ulCommand, ULONG ulParameter,
                                              PVOID pvData, LONG cbData)
{
    APIRET rc;
    ULONG  obj;
    int    ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: PmcSendCommandData(cmd=0x%lx, par=0x%lx, data=%p, cbData=%ld)",
                         (unsigned long)ulCommand, (unsigned long)ulParameter,
                         pvData, (long)cbData);

    if (!pfnSendGenCmdDataToServer) {
        F_DbgCtxExitf(ctx, "no backend");
        return (APIRET)-1;
    }
    LockState();
    obj = client_obj;
    UnlockState();
    if (obj == 0) {
        F_DbgCtxExitf(ctx, "not connected");
        return (APIRET)-1;
    }

    rc = pfnSendGenCmdDataToServer(obj, ulCommand, ulParameter, pvData, cbData);
    if (rc == (APIRET)-1) OnTransportError();
    F_DbgCtxExitf(ctx, "rc=0x%lx", (unsigned long)rc);
    return rc;
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
    int    ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: ServerInit");

    if (DosCreateMutexSem(NULL, &g_hmtxState, 0, FALSE) != 0) {
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: DosCreateMutexSem failed");
        g_hmtxState = NULL;
        F_DbgCtxExitf(ctx, "DosCreateMutexSem failed");
        return 0;
    }

    FPM_config.Read("fpm.ini");
    F_DbgSetLevels(FPM_config.debugOptions);

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
    F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
               "PMC: Backend selected = %s", szBackend);

    rc = DosLoadModule(NULL, 0, szBackend, &hmodBE);
    if (rc) {
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: DosLoadModule failed: 0x%x", (int)rc);
        F_DbgCtxExitf(ctx, "DosLoadModule failed rc=0x%x", (int)rc);
        return 0;
    }

    if (ResolveProc(hmodBE, 1, (void **)&pfnInitServerConnection)) {
        F_DbgCtxExitf(ctx, "ResolveProc(1) failed");
        return 0;
    }
    if (ResolveProc(hmodBE, 2, (void **)&pfnCloseServerConnection)) {
        F_DbgCtxExitf(ctx, "ResolveProc(2) failed");
        return 0;
    }
    if (ResolveProc(hmodBE, 4, (void **)&pfnSendCmdToServer)) {
        F_DbgCtxExitf(ctx, "ResolveProc(4) failed");
        return 0;
    }
    if (ResolveProc(hmodBE, 5, (void **)&pfnSendDataToServer)) {
        F_DbgCtxExitf(ctx, "ResolveProc(5) failed");
        return 0;
    }
    if (ResolveProc(hmodBE, 6, (void **)&pfnRecvDataFromServer)) {
        F_DbgCtxExitf(ctx, "ResolveProc(6) failed");
        return 0;
    }
    if (ResolveProc(hmodBE, 7, (void **)&pfnSendGenCmdToServer)) {
        F_DbgCtxExitf(ctx, "ResolveProc(7) failed");
        return 0;
    }
    if (ResolveProc(hmodBE, 8, (void **)&pfnSendGenCmdDataToServer)) {
        F_DbgCtxExitf(ctx, "ResolveProc(8) failed");
        return 0;
    }

    if (FPM_config.ExternMachineName[0])
        serverName = &FPM_config.ExternMachineName[0];
    else
        serverName = NULL;

    if (serverName == NULL && !LocalServerRunning())
    {
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                   "PMC: Local server not running, attempting to start...");
        startRc = ExecuteFreePMServer();
        if (startRc == 0) {
            DosSleep(1000);
        } else {
            F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                       "PMC: Failed to start PMSERVER.EXE.");
        }
    }

    rc = pfnInitServerConnection(serverName, &client_obj,
                                 (void *)PMC_ShutdownCallback);
    if (rc) {
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: pfnInitServerConnection rc=0x%x", rc);
        F_DbgCtxExitf(ctx, "InitServerConnection rc=0x%x", rc);
        return 0;
    }

    /* NEW: client handshake */
    if (PMC_ClientConnect() != 0) {
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: CONNECT failed, aborting init");
        pfnCloseServerConnection();
        client_obj = 0;
        F_DbgCtxExitf(ctx, "CONNECT failed");
        return 0;
    }

    g_fConnected = TRUE;

    /* NEW: start keepalive */
    if (g_clientConfig.flConfig & F_CFG_PING_REQUIRED)
        StartKeepalive();

    /* NOTE: no atexit(PMC_ServerTerm) here. On OS/2 and Win32, atexit
       callbacks inside a DLL run AFTER the loader has started
       unloading dependent DLLs (LIFO: PMTCP before PMCLIENT), so
       pfnSendCmdToServer would target freed code. Instead, PMTCP
       invokes PMC_ShutdownCallback from its own DllMain(DETACH),
       when both DLLs are still alive. */

    F_DbgCtxExitf(ctx, "Success (idClient=%lu)",
                  (unsigned long)g_clientConfig.idClient);
    return 1;
}

/*! @brief Shutdown callback invoked from PMTCP's DllMain(DETACH).

    At this point PMTCP and PMCLIENT are both fully loaded, so
    pfnSendCmdToServer goes through live code and the socket is still
    open. Sends F_CMD_CLIENT_EXIT; PMTCP then closes the socket in the
    continuation of its own DllMain(DETACH). */
static void APIENTRY PMC_ShutdownCallback(void)
{
    APIRET rc;
    ULONG  obj;
    ULONG  idc;

    F_DbgSetShuttingDown(1);

    if (g_fTerminated) return;
    g_fTerminated = TRUE;

    F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
               "PMC: PMC_ShutdownCallback (from PMTCP DETACH)");

    StopKeepalive();

    LockState();
    obj = client_obj;
    idc = g_clientConfig.idClient;
    if (obj != 0 && g_fConnected && pfnSendCmdToServer) {
        UnlockState();
        rc = pfnSendCmdToServer(obj, F_CMD_CLIENT_EXIT, idc);
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                   "PMC: EXIT sent (rc=0x%lx)", (unsigned long)rc);
        LockState();
        g_fConnected = FALSE;
    }
    client_obj      = 0;
    g_fReconnecting = FALSE;
    UnlockState();
}

/*! @brief Fallback term, invoked from DllMain(DLL_PROCESS_DETACH) /
    _DLL_InitTerm(unload) if PMTCP's callback did not run (e.g.
    abnormal termination where PMTCP was unloaded without invoking
    the callback). Performs state-only cleanup; never calls pfn*. */
static void PMC_ServerTerm(void)
{
    F_DbgSetShuttingDown(1);

    if (g_fTerminated) return;
    g_fTerminated = TRUE;

    F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
               "PMC: PMC_ServerTerm started");

    StopKeepalive();

    if (g_hmtxState) {
        DosCloseMutexSem(g_hmtxState);
        g_hmtxState = NULL;
    }

    client_obj = 0;
    hmodBE = 0;
    F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
               "PMC: ServerTerm completed");
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
