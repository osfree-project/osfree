/*! @file init.cpp
    @brief PMCLIENT.DLL — per-thread session + transport.

    Exports Fc_SesConnect / Fc_SesDisconnect for per-thread session
    lifecycle, and Fc_Ses* for the data path. The transport handle for
    each thread lives in aConns[tid] and is not exposed.

    Contract:
    - Fc_SesConnect must be called by each thread before any Fc_Ses*
      call from that thread; it opens the transport, sends
      F_CMD_CLIENT_CONNECT, reads F_CLIENTCONFIG into g_clientConfig,
      and records aConns[tid] = { obj, SLOT_ACTIVE }.
    - Fc_SesDisconnect closes the transport for the calling thread and
      clears its slot. It does not send F_CMD_CLIENT_EXIT — that is
      sent separately by the caller.
    - Fc_Ses* return (APIRET)-1 if the calling thread has no active
      slot, otherwise they forward to the backend and, on transport
      error, mark the slot BROKEN and start a per-slot reconnect.

    g_idClient and g_clientConfig are process-wide, filled from the
    first successful CONNECT reply.

    Backend DLL is loaded on the first Fc_SesConnect of the process;
    the backend name is taken from PM_COMM_BACKEND, then fpm.ini
    (backend=), then the default "PMPIPE".
*/

#define INCL_GPI

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
#include "pmclient.h"

/* ---- Backend function pointers (resolved at load) ----
   Backend EXPORTS (pmtcp/pmpipe/pmshm): ord 1..7 are the client
   entries, 8..11 are server-side and not resolved here.
     ord 1 = Fc_TrnConnect         -> pfnFc_TrnConnect
     ord 2 = Fc_TrnDisconnect      -> pfnFc_TrnDisconnect
     ord 3 = Fc_TrnPostCommand     -> pfnFc_TrnPostCommand
     ord 4 = Fc_TrnPostData        -> pfnFc_TrnPostData
     ord 5 = Fc_TrnSendCommand     -> pfnFc_TrnSendCommand
     ord 6 = Fc_TrnSendCommandData -> pfnFc_TrnSendCommandData
     ord 7 = Fc_TrnGetData         -> pfnFc_TrnGetData */
#if defined(__OS2__) || defined(__OS2V2__)
/*! @def PMC_CB
    @brief Calling convention for backend function pointers (OS/2). */
#define PMC_CB APIENTRY
#else
/*! @def PMC_CB
    @brief Calling convention for backend function pointers (Win32).

    __stdcall has the same binary convention as __estdcall; needed
    here because __declspec(__pragma(...)) is a function modifier and
    cannot appear in pointer declarators. */
#define PMC_CB __stdcall
#endif

/*! @brief Backend export Fc_TrnConnect (ord 1). */
static APIRET (PMC_CB *pfnFc_TrnConnect)(char *, PULONG)                        = 0;
/*! @brief Backend export Fc_TrnDisconnect (ord 2). */
static APIRET (PMC_CB *pfnFc_TrnDisconnect)(void *)                             = 0;
/*! @brief Backend export Fc_TrnPostCommand (ord 3). */
static APIRET (PMC_CB *pfnFc_TrnPostCommand)(void *, int, int)                  = 0;
/*! @brief Backend export Fc_TrnPostData (ord 4). */
static APIRET (PMC_CB *pfnFc_TrnPostData)(void *, PVOID, ULONG)                 = 0;
/*! @brief Backend export Fc_TrnSendCommand (ord 5). */
static APIRET (PMC_CB *pfnFc_TrnSendCommand)(void *, int, int)                  = 0;
/*! @brief Backend export Fc_TrnSendCommandData (ord 6). */
static APIRET (PMC_CB *pfnFc_TrnSendCommandData)(void *, int, int, PVOID, ULONG) = 0;
/*! @brief Backend export Fc_TrnGetData (ord 7). */
static APIRET (PMC_CB *pfnFc_TrnGetData)(void *, PVOID, PULONG, ULONG)          = 0;

/*! @brief Backend module handle; 0 until LoadBackend succeeds. */
static HMODULE hmodBE = 0;

/*! @brief Resolve one backend export by ordinal.

    DosQueryProcAddr takes PFN*, but the pfn* variables here are
    APIENTRY-typed function pointers. Wrapped once to avoid casting
    at every call site.

    @param [in]  hmod  Backend module handle.
    @param [in]  ord   Export ordinal (1..7).
    @param [out] ppfn  Receives the resolved address.
    @return 0 on success, non-zero on failure.
    @retval 0  Address resolved and stored in *ppfn.
    @retval !0 DosQueryProcAddr failed; *ppfn is 0. */
static APIRET ResolveProc(HMODULE hmod, ULONG ord, void **ppfn)
{
    PFN    pfn = 0;
    APIRET rc  = DosQueryProcAddr(hmod, ord, NULL, &pfn);
    *ppfn = (void *)pfn;
    return rc;
}

/*! @brief Process-wide client configuration object (fpm.ini). */
class F_ClientConfig FPM_config;

/* ======================================================================== */
/*  Per-thread session table                                                */
/* ======================================================================== */

/*! @def FREEPM_MAX_THREADS
    @brief Maximum concurrent client threads with their own connection. */
#define FREEPM_MAX_THREADS 256

/*! @def SLOT_FREE
    @brief aConns[] slot is unused. */
#define SLOT_FREE          0
/*! @def SLOT_ACTIVE
    @brief aConns[] slot has a live transport + CONNECT. */
#define SLOT_ACTIVE        1
/*! @def SLOT_BROKEN
    @brief aConns[] slot lost its transport; reconnect in progress. */
#define SLOT_BROKEN        2

/*! @brief One per-thread connection slot. */
typedef struct _ConnSlot {
    ULONG  tid;           /*!< OS/2 thread id (DosGetInfoBlocks). */
    ULONG  obj;           /*!< Opaque transport handle from backend. */
    int    state;         /*!< SLOT_FREE / SLOT_ACTIVE / SLOT_BROKEN. */
    int    reconnecting;  /*!< Non-zero if reconnect thread is running. */
    time_t lastActivity;  /*!< Last successful send/recv, seconds. */
} ConnSlot;

/*! @brief Per-thread connection table, protected by g_hmtxState. */
static ConnSlot aConns[FREEPM_MAX_THREADS];

/* ======================================================================== */
/*  Process-wide state                                                      */
/* ======================================================================== */

/*! @brief Client configuration, sent by server after CONNECT.
    Filled from the first CONNECT reply; subsequent threads share it.
    Protected by g_hmtxState. */
static F_CLIENTCONFIG g_clientConfig;

/*! @brief Process-wide idClient, filled from the first CONNECT reply. */
static ULONG g_idClient = 0;

/*! @brief Non-zero while keepalive thread should keep running. */
static volatile BOOL g_fKeepaliveRun = FALSE;

/*! @brief Non-zero once shutdown is in progress. */
static volatile BOOL g_fTerminated = FALSE;

/*! @brief Non-zero once PMC_ServerInit has completed successfully. */
static volatile BOOL g_fInitialized = FALSE;

/*! @brief Protects aConns, g_idClient, g_clientConfig, g_fInitialized
    and backend pointers against concurrent access from public wrappers,
    keepalive thread, reconnect threads, and PMC_ServerInit. Never held
    while calling any pfnFc_Trn* (I/O may block). */
static HMTX g_hmtxState = NULL;

/*! @brief Spawn a per-slot reconnect thread for a BROKEN slot.
    @param [in] tid  Thread id whose slot is BROKEN. */
static void StartReconnect(ULONG tid);

/*! @brief Mark a slot BROKEN and (if idle) start its reconnect thread.
    @param [in] tid  Thread id whose transport failed. */
static void OnTransportError(ULONG tid);

/*! @brief Stop the keepalive thread (asks it to exit at next tick). */
static void StopKeepalive(void);

/* ======================================================================== */
/*  State lock                                                              */
/* ======================================================================== */

/*! @brief Acquire g_hmtxState; no-op if mutex is not yet created. */
static void LockState(void)
{
    if (g_hmtxState) DosRequestMutexSem(g_hmtxState, SEM_INDEFINITE_WAIT);
}

/*! @brief Release g_hmtxState; no-op if mutex is not yet created. */
static void UnlockState(void)
{
    if (g_hmtxState) DosReleaseMutexSem(g_hmtxState);
}

/*! @brief Create the state mutex (unnamed, process-local) exactly once.

    Unnamed mutexes cannot collide on a name, so the create call
    always succeeds and no name lookup is needed.

    Race on first use: two threads can both pass the NULL check and
    both create a mutex. Only one handle ends up in g_hmtxState; the
    other is closed. Whichever handle wins, all subsequent Lock/Unlock
    calls pair up correctly. No error-code checks are used, so the
    code does not depend on ERROR_ALREADY_EXISTS being #defined. */
static void EnsureStateMutex(void)
{
    HMTX   h = NULL;
    APIRET rc;

    if (g_hmtxState != NULL) return;

    rc = DosCreateMutexSem(NULL, &h, 0, FALSE);
    if (rc != 0) {
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: DosCreateMutexSem failed rc=0x%x", (int)rc);
        return;
    }
    if (g_hmtxState == NULL) {
        g_hmtxState = h;
    } else {
        DosCloseMutexSem(h);
    }
}

/* ======================================================================== */
/*  Thread id and slot helpers                                              */
/* ======================================================================== */

/*! @brief Return the OS/2 thread id of the calling thread.
    @return Thread id. */
static ULONG QueryThreadTid(void)
{
    PTIB ptib;
    PPIB ppib;
    DosGetInfoBlocks(&ptib, &ppib);
    return (ULONG)ptib->tib_ptib2->tib2_ultid;
}

/*! @brief Find the slot for a given thread id.
    Called under g_hmtxState.
    @param [in] tid  Thread id.
    @return Slot pointer, or NULL if not found.
    @retval !NULL  Slot is allocated and matches tid.
    @retval NULL   No slot with that tid. */
static ConnSlot *SlotFind(ULONG tid)
{
    int i;
    for (i = 0; i < FREEPM_MAX_THREADS; i++)
        if (aConns[i].state != SLOT_FREE && aConns[i].tid == tid)
            return &aConns[i];
    return NULL;
}

/*! @brief Allocate a free slot and initialise it for the given tid.
    Called under g_hmtxState. Sets obj to 0; caller fills it on success.
    @param [in] tid  Thread id.
    @return Slot pointer, or NULL if the table is full.
    @retval !NULL  Slot allocated and initialised.
    @retval NULL   No free slot (FREEPM_MAX_THREADS exhausted). */
static ConnSlot *SlotAlloc(ULONG tid)
{
    int i;
    for (i = 0; i < FREEPM_MAX_THREADS; i++) {
        if (aConns[i].state == SLOT_FREE) {
            aConns[i].tid          = tid;
            aConns[i].obj          = 0;
            aConns[i].state        = SLOT_ACTIVE;
            aConns[i].reconnecting = 0;
            aConns[i].lastActivity = time(NULL);
            return &aConns[i];
        }
    }
    return NULL;
}

/* ======================================================================== */
/*  Server startup / detection                                              */
/* ======================================================================== */

/*! @brief Start PMSERVER.EXE when the local server is not running.

    Called once, from PMC_ServerInit, when LocalServerRunning()
    indicates the server mutex is not held. Uses DosExecPgm with
    EXEC_BACKGROUND so the server runs detached.

    @return 0 on success, -1 on failure.
    @retval 0   DosExecPgm accepted the launch request.
    @retval -1  DosExecPgm failed. */
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

/*! @brief Check whether the local FreePM server is running.
    @return 1 if the server mutex is held (server up), 0 otherwise.
    @retval 1  Server mutex present.
    @retval 0  Server not running. */
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
/*  Backend loading                                                         */
/* ======================================================================== */

/*! @brief Load the backend and resolve ordinals 1..7.

    Called once from PMC_ServerInit, under g_hmtxState. On resolve
    failure the module is unloaded and hmodBE cleared, so a later
    retry performs a fresh load.

    Backend name precedence: PM_COMM_BACKEND env, fpm.ini backend=,
    default PMPIPE.

    @return 0 on success, -1 on failure.
    @retval 0   Backend loaded and all 7 ordinals resolved.
    @retval -1  DosLoadModule or any DosQueryProcAddr failed. */
static int LoadBackend(void)
{
    PSZ    pszValue = NULL;
    char   szBackend[256];
    APIRET rc;
    int    ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: LoadBackend");

    FPM_config.Read("fpm.ini");
    F_DbgSetLevels(FPM_config.debugOptions);

    if (DosScanEnv((PSZ)"PM_COMM_BACKEND", &pszValue) == 0 && pszValue != NULL)
        strncpy(szBackend, pszValue, sizeof(szBackend) - 1);
    else if (FPM_config.BackendName[0])
        strncpy(szBackend, FPM_config.BackendName, sizeof(szBackend) - 1);
    else
#if defined(__OS2__) || defined(__OS2V2__)
    strcpy(szBackend, "PMPIPE");
#else
    strcpy(szBackend, "PMTCP");
#endif
    szBackend[sizeof(szBackend) - 1] = 0;
    F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
               "PMC: Backend selected = %s", szBackend);

    rc = DosLoadModule(NULL, 0, szBackend, &hmodBE);
    if (rc) {
        hmodBE = 0;
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: DosLoadModule failed: 0x%x", (int)rc);
        F_DbgCtxExitf(ctx, "DosLoadModule failed rc=0x%x", (int)rc);
        return -1;
    }

    if (ResolveProc(hmodBE, 1, (void **)&pfnFc_TrnConnect)         ||
        ResolveProc(hmodBE, 2, (void **)&pfnFc_TrnDisconnect)      ||
        ResolveProc(hmodBE, 3, (void **)&pfnFc_TrnPostCommand)     ||
        ResolveProc(hmodBE, 4, (void **)&pfnFc_TrnPostData)        ||
        ResolveProc(hmodBE, 5, (void **)&pfnFc_TrnSendCommand)     ||
        ResolveProc(hmodBE, 6, (void **)&pfnFc_TrnSendCommandData) ||
        ResolveProc(hmodBE, 7, (void **)&pfnFc_TrnGetData))
    {
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: ResolveProc failed");
        DosFreeModule(hmodBE);
        hmodBE = 0;
        F_DbgCtxExitf(ctx, "ResolveProc failed");
        return -1;
    }

    F_DbgCtxExitf(ctx, "OK");
    return 0;
}

/* ======================================================================== */
/*  Session init (once per process)                                         */
/* ======================================================================== */

/*! @brief Process-wide init: state mutex, backend load, local server
    startup if needed.

    Called from Fc_SesConnect on first use. Idempotent — second call
    returns 1 immediately.

    @return 1 on success, 0 on failure.
    @retval 1  Init completed (or was already completed).
    @retval 0  Mutex, backend, or local-server startup failed. */
static int PMC_ServerInit(void)
{
    int startRc;
    int ctx;

    if (g_fInitialized) return 1;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: ServerInit");

    EnsureStateMutex();
    if (g_hmtxState == NULL) {
        F_DbgCtxExitf(ctx, "EnsureStateMutex failed");
        return 0;
    }

    LockState();
    if (g_fInitialized) {
        UnlockState();
        F_DbgCtxExitf(ctx, "already initialized");
        return 1;
    }

    if (LoadBackend() != 0) {
        UnlockState();
        F_DbgCtxExitf(ctx, "LoadBackend failed");
        return 0;
    }

    if (!LocalServerRunning()) {
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                   "PMC: Local server not running, attempting to start...");
        startRc = ExecuteFreePMServer();
        if (startRc == 0) {
            /* Hold the state mutex across the settle delay: otherwise
               other threads racing into PMC_ServerInit also see
               LocalServerRunning()==0 and launch extra PMSERVER copies. */
            DosSleep(1000);
        } else {
            F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                       "PMC: Failed to start PMSERVER.EXE.");
        }
    }

    g_fInitialized = TRUE;
    UnlockState();

    F_DbgCtxExitf(ctx, "Success");
    return 1;
}

/* ======================================================================== */
/*  Client handshake (CONNECT)                                              */
/* ======================================================================== */

/*! @brief Send F_CMD_CLIENT_CONNECT and read F_CLIENTCONFIG.

    Sends the CONNECT command with par=0 (per-thread connect; PMCLIENT
    does not carry clientid over the wire for this command), then
    blocks on a full F_CLIENTCONFIG reply.

    Reply is 16 bytes { idClient, ulInactive, ulGrace, flConfig }.
    Rejects the connection if idClient == 0.

    F_CLIENTCONFIG is process-wide: it is filled once, from the first
    successful CONNECT in this process. Subsequent CONNECTs read their
    reply into a local struct and verify only that idClient matches
    g_idClient; other fields (ulInactive, ulGrace, flConfig) are
    ignored, since the process-wide config is already fixed.

    @param [in] obj  This thread's transport handle.
    @return 0 on success, -1 on failure.
    @retval 0   CONNECT accepted; on first call, g_clientConfig and
                g_idClient filled.
    @retval -1  Transport error, short reply, rejected by server, or
                idClient mismatch with the existing g_idClient. */
static int PMC_ClientConnect(ULONG obj)
{
    APIRET         rc;
    ULONG          len = 0;
    F_CLIENTCONFIG cfg;
    ULONG          send_idc;
    int            ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: ClientConnect(obj=0x%lx)", (unsigned long)obj);

    /* Serialize CONNECT end-to-end. The state mutex is held across
       the post + read because both send_idc and g_idClient must stay
       consistent: two threads racing here would otherwise both send
       idClient=0 and the server would create two processes. */
    LockState();

    send_idc = g_idClient;

    F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
               "PMC: sending F_CMD_CLIENT_CONNECT (idClient=%lu)",
               (unsigned long)send_idc);

    rc = pfnFc_TrnPostCommand((void *)obj, F_CMD_CLIENT_CONNECT, (int)send_idc);
    if (rc != 0) {
        UnlockState();
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: F_CMD_CLIENT_CONNECT send failed rc=%u", rc);
        F_DbgCtxExitf(ctx, "send failed rc=%u", rc);
        return -1;
    }

    memset(&cfg, 0, sizeof(cfg));
    rc = pfnFc_TrnGetData((void *)obj, &cfg, &len, sizeof(cfg));
    if (rc != 0 || len != (ULONG)sizeof(cfg)) {
        UnlockState();
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: CONNECT reply read failed rc=%u, len=%u",
                   rc, (unsigned)len);
        F_DbgCtxExitf(ctx, "reply read failed rc=%u, len=%u",
                      rc, (unsigned)len);
        return -1;
    }

    if (cfg.idClient == 0) {
        UnlockState();
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: CONNECT rejected by server (idClient=0)");
        F_DbgCtxExitf(ctx, "rejected (idClient=0)");
        return -1;
    }

    if (g_idClient == 0) {
        g_clientConfig = cfg;
        g_idClient     = cfg.idClient;
        UnlockState();
        F_DbgCtxExitf(ctx,
                      "ok first, idClient=%lu, ulInactive=%lu, "
                      "ulGrace=%lu, flConfig=0x%lx",
                      (unsigned long)g_clientConfig.idClient,
                      (unsigned long)g_clientConfig.ulInactive,
                      (unsigned long)g_clientConfig.ulGrace,
                      (unsigned long)g_clientConfig.flConfig);
        return 0;
    }

    if (cfg.idClient != g_idClient) {
        ULONG expected = g_idClient;
        UnlockState();
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: CONNECT reply idClient=%lu mismatch g_idClient=%lu",
                   (unsigned long)cfg.idClient, (unsigned long)expected);
        F_DbgCtxExitf(ctx, "idClient mismatch");
        return -1;
    }

    UnlockState();
    F_DbgCtxExitf(ctx, "ok idClient=%lu", (unsigned long)cfg.idClient);
    return 0;
}

/* ======================================================================== */
/*  Transport error handling / reconnect                                    */
/* ======================================================================== */

/*! @brief Called from any public wrapper or keepalive when a
    transport-level failure is detected on a specific slot.

    Marks the slot BROKEN and, if no reconnect is already in progress
    for it and shutdown has not begun, starts the reconnect thread for
    that slot.

    @param [in] tid  Thread id whose transport failed. */
static void OnTransportError(ULONG tid)
{
    BOOL start = FALSE;

    LockState();
    {
        ConnSlot *p = SlotFind(tid);
        if (p != NULL && p->state == SLOT_ACTIVE &&
            !p->reconnecting && !g_fTerminated) {
            p->state        = SLOT_BROKEN;
            p->reconnecting = 1;
            start = TRUE;
        }
    }
    UnlockState();

    if (start) {
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: transport error on tid=%lu, starting reconnect",
                   (unsigned long)tid);
        StartReconnect(tid);
    }
}

/*! @brief Per-slot reconnect thread: retries backend connect +
    RECONNECT until ulGrace seconds have elapsed or the process is
    terminating.

    Wire: cmd=RECONNECT, data=idClient, payload={ ULONG tid }.
    The tid identifies the client thread whose slot went BROKEN; it is
    also the key under which NWinInitialize registered a HAB on the
    server side. RECONNECT does not touch the HAB.

    On success the slot's obj, state and lastActivity are refreshed;
    on give-up, the reconnecting flag is cleared and the slot remains
    BROKEN (the next keepalive tick may try again).

    @param [in] param  Thread id (tid) as ULONG cast to void*. */
static void ReconnectThreadProc(void *param)
{
    ULONG    tid = (ULONG)param;
    ULONG    grace;
    ULONG    deadline;
    ULONG    wait;
    APIRET   rc;
    ULONG    newobj;
    ULONG    len;
    F_CLIENTCONFIG cfg;
    int      ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: ReconnectThreadProc(tid=%lu)",
                         (unsigned long)tid);

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
        rc = pfnFc_TrnConnect(NULL, &newobj);
        if (rc == 0 && newobj != 0) {
            rc = pfnFc_TrnPostCommand((void *)newobj,
                                      F_CMD_CLIENT_RECONNECT, g_idClient);
            if (rc == 0) {
                rc = pfnFc_TrnPostData((void *)newobj, &tid,
                                       (ULONG)sizeof(tid));
            }
            if (rc == 0) {
                memset(&cfg, 0, sizeof(cfg));
                len = 0;
                rc = pfnFc_TrnGetData((void *)newobj, &cfg, &len,
                                      sizeof(cfg));
                if (rc == 0 && len == (ULONG)sizeof(cfg) &&
                    cfg.idClient == g_idClient) {
                    LockState();
                    {
                        ConnSlot *p = SlotFind(tid);
                        if (p != NULL && p->state == SLOT_BROKEN) {
                            p->obj          = newobj;
                            p->state        = SLOT_ACTIVE;
                            p->reconnecting = 0;
                            p->lastActivity = time(NULL);
                            newobj = 0;
                        }
                    }
                    UnlockState();
                    if (newobj == 0) {
                        F_DbgCtxExitf(ctx, "reconnect ok tid=%lu",
                                      (unsigned long)tid);
                        return;
                    }
                }
            }
            if (newobj != 0)
                pfnFc_TrnDisconnect((void *)newobj);
        }
    }

    LockState();
    {
        ConnSlot *p = SlotFind(tid);
        if (p != NULL && p->state == SLOT_BROKEN)
            p->reconnecting = 0;
    }
    UnlockState();
    F_DbgCtxExitf(ctx, "grace expired tid=%lu", (unsigned long)tid);
}

/*! @brief Spawn a reconnect thread for one slot.
    @param [in] tid  Thread id whose slot is BROKEN. */
static void StartReconnect(ULONG tid)
{
#if defined(__OS2__) || defined(__OS2V2__)
    int thrc = _beginthread(ReconnectThreadProc, NULL, 65536, (void *)tid);
#else
    int thrc = (int)_beginthread(ReconnectThreadProc, 65536, (void *)tid);
#endif
    if (thrc == -1) {
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: _beginthread for reconnect failed tid=%lu",
                   (unsigned long)tid);
        LockState();
        { ConnSlot *p = SlotFind(tid); if (p != NULL) p->reconnecting = 0; }
        UnlockState();
    }
}

/* ======================================================================== */
/*  Keepalive thread                                                        */
/* ======================================================================== */

/*! @brief Keepalive thread: every ulInactive/2 seconds (floor 5 s),
    iterates aConns[] and sends F_CMD_CLIENT_PING on each ACTIVE slot.

    On PING failure the slot is marked BROKEN via OnTransportError,
    which starts a reconnect thread for it. Thread exits when
    g_fKeepaliveRun is cleared or g_fTerminated is set.

    @param [in] param  Unused. */
static void KeepaliveProc(void *param)
{
    (void)param;

    ULONG pingSec = g_clientConfig.ulInactive / 2;
    if (pingSec < 5) pingSec = 5;

    F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
               "PMC: keepalive thread started (ping every %lu sec)",
               (unsigned long)pingSec);

    while (g_fKeepaliveRun && !g_fTerminated) {
        int i;

        DosSleep(pingSec * 1000);
        if (!g_fKeepaliveRun || g_fTerminated) break;

        for (i = 0; i < FREEPM_MAX_THREADS; i++) {
            ULONG obj    = 0;
            ULONG tid    = 0;
            ULONG idc    = 0;
            BOOL  active = FALSE;

            LockState();
            if (aConns[i].state == SLOT_ACTIVE) {
                obj    = aConns[i].obj;
                tid    = aConns[i].tid;
                idc    = g_idClient;
                active = TRUE;
            }
            UnlockState();

            if (active && obj != 0) {
                if (pfnFc_TrnPostCommand((void *)obj,
                                         F_CMD_CLIENT_PING, idc) != 0) {
                    F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                               "PMC: PING failed on tid=%lu",
                               (unsigned long)tid);
                    OnTransportError(tid);
                }
            }
        }
    }

    F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
               "PMC: keepalive thread exiting");
}

/*! @brief Start keepalive thread, once per process.
    @note Called from Fc_SesConnect under g_hmtxState. */
static void StartKeepalive(void)
{
    if (g_fKeepaliveRun) return;
    g_fKeepaliveRun = TRUE;
#if defined(__OS2__) || defined(__OS2V2__)
    _beginthread(KeepaliveProc, NULL, 65536, NULL);
#else
    _beginthread(KeepaliveProc, 65536, NULL);
#endif
}

/*! @brief Stop keepalive thread.

    The thread will notice within pingSec and exit; it is not joined
    (DLL shutdown should not block for that long). */
static void StopKeepalive(void)
{
    if (!g_fKeepaliveRun) return;
    g_fKeepaliveRun = FALSE;
}

/* ======================================================================== */
/*  Exported session wrappers                                               */
/* ======================================================================== */

/*! @brief Open this thread's session + send F_CMD_CLIENT_CONNECT.

    - Lazy backend load (once per process, under g_hmtxState).
    - If address == NULL and no local server is running, start
      PMSERVER.EXE and wait ~1 s for it to come up.
    - pfnFc_TrnConnect(address, &obj).
    - CONNECT + read F_CLIENTCONFIG via PMC_ClientConnect.
    - aConns[tid] = { obj, ACTIVE }.

    A thread may call Fc_SesConnect at most once per its lifetime.
    If aConns[tid] already exists (in any state — SLOT_ACTIVE or
    SLOT_BROKEN), the call returns (APIRET)-1 without touching the
    transport. Recovery from a broken slot is the sole responsibility
    of the per-slot reconnect thread started by OnTransportError.

    @param [in] address  Backend-specific address, or NULL for default.
                         TcpLink: "host" or "host:port"; PipeLink /
                         ShmLink: NULL selects the default name.
    @return 0 on success, -1 on failure.
    @retval 0   Transport open, CONNECT accepted, slot registered.
    @retval -1  Slot already in use for this tid, or init / connect /
                handshake / slot allocation failed. */
extern "C" APIRET EXPENTRY Fc_SesConnect(char *address)
{
    ULONG     tid;
    ULONG     obj = 0;
    APIRET    rc;
    ConnSlot *p;
    int       ctx;

    tid = QueryThreadTid();

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: Fc_SesConnect(tid=%lu, addr=%s)",
                         (unsigned long)tid, address ? address : "NULL");

    if (!PMC_ServerInit()) {
        F_DbgCtxExitf(ctx, "PMC_ServerInit failed");
        return (APIRET)-1;
    }

    LockState();
    p = SlotFind(tid);
    if (p != NULL) {
        int state = p->state;
        UnlockState();
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: Fc_SesConnect(tid=%lu) slot already in use "
                   "(state=%d)", (unsigned long)tid, state);
        F_DbgCtxExitf(ctx, "slot already in use (state=%d)", state);
        return (APIRET)-1;
    }
    UnlockState();

    rc = pfnFc_TrnConnect(address, &obj);
    if (rc != 0 || obj == 0) {
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: pfnFc_TrnConnect rc=0x%x", (unsigned)rc);
        F_DbgCtxExitf(ctx, "Fc_TrnConnect rc=0x%x", (unsigned)rc);
        return (APIRET)-1;
    }

    if (PMC_ClientConnect(obj) != 0) {
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: CONNECT failed, aborting init");
        pfnFc_TrnDisconnect((void *)obj);
        F_DbgCtxExitf(ctx, "CONNECT failed");
        return (APIRET)-1;
    }

    LockState();
    p = SlotFind(tid);
    if (p != NULL) {
        UnlockState();
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: Fc_SesConnect(tid=%lu) slot appeared during "
                   "connect; dropping new transport",
                   (unsigned long)tid);
        pfnFc_TrnDisconnect((void *)obj);
        F_DbgCtxExitf(ctx, "slot appeared during connect");
        return (APIRET)-1;
    }
    p = SlotAlloc(tid);
    if (p == NULL) {
        UnlockState();
        F_DbgPrint(F_DBG_SEC_PMCLIENT, F_DBG_LVL_ERROR,
                   "PMC: slot table full (tid=%lu)", (unsigned long)tid);
        pfnFc_TrnDisconnect((void *)obj);
        F_DbgCtxExitf(ctx, "slot table full");
        return (APIRET)-1;
    }
    p->obj          = obj;
    p->state        = SLOT_ACTIVE;
    p->reconnecting = 0;
    p->lastActivity = time(NULL);
    if (g_clientConfig.flConfig & F_CFG_PING_REQUIRED)
        StartKeepalive();
    UnlockState();

    F_DbgCtxExitf(ctx, "ok tid=%lu obj=0x%lx idClient=%lu",
                  (unsigned long)tid, (unsigned long)obj,
                  (unsigned long)g_idClient);
    return 0;
}

/*! @brief Close this thread's session.

    Looks up aConns[tid], clears the slot, and calls backend
    Fc_TrnDisconnect(obj). Does NOT send F_CMD_CLIENT_EXIT — the caller
    (WinTerminate in PMMERGE) sends it explicitly via Fc_SesPostCommand
    before calling Fc_SesDisconnect, keeping session-level tear-down
    (EXIT) separate from transport-level tear-down (close).

    No-op if the thread has no active transport.

    @return 0 always.
    @retval 0  Slot cleared (or was already free). */
/*! @brief Close this thread's session.

    Sends F_CMD_CLIENT_EXIT (best-effort, fire-and-forget) so the
    server tears down this thread's HAB and slot immediately, then
    calls backend Fc_TrnDisconnect(obj). No reply is read — EXIT is
    fire-and-forget by protocol; the transport close is what actually
    unblocks Fs_ClientThreadProc on the server side.

    Sending EXIT here (instead of relying on the transport-close
    detection) matters because the fallback path is DETACHED +
    ulGrace wait: a HAB that should be freed right now would instead
    linger for ulGrace seconds (30 s by default) waiting for a
    RECONNECT that never comes.

    No-op if the thread has no active transport.

    @return 0 always.
    @retval 0  Slot cleared (or was already free). */
extern "C" APIRET EXPENTRY Fc_SesDisconnect(void)
{
    ULONG     tid;
    ConnSlot *p;
    ULONG     obj = 0;
    BOOL      was_active = FALSE;
    int       ctx;

    tid = QueryThreadTid();

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: Fc_SesDisconnect(tid=%lu)",
                         (unsigned long)tid);

    LockState();
    p = SlotFind(tid);
    if (p != NULL) {
        if (p->state == SLOT_ACTIVE) {
            obj        = p->obj;
            was_active = TRUE;
        }
        p->state        = SLOT_FREE;
        p->obj          = 0;
        p->reconnecting = 0;
    }
    UnlockState();

    if (!was_active || obj == 0) {
        F_DbgCtxExitf(ctx, "no active slot");
        return 0;
    }

    /* Best-effort EXIT. Server finds the thread by recvobj, so
       g_idClient is informational; send 0 if not set (should not
       happen on the was_active path, but be defensive). */
    pfnFc_TrnPostCommand((void *)obj, F_CMD_CLIENT_EXIT, (int)g_idClient);

    pfnFc_TrnDisconnect((void *)obj);
    F_DbgCtxExitf(ctx, "ok obj=0x%lx", (unsigned long)obj);
    return 0;
}

/* ======================================================================== */
/*  Exported session data-path wrappers                                     */
/* ======================================================================== */

/*! @brief Async write of an 8-byte header (cmd + par) on this thread.

    Looks up aConns[QueryThreadTid()]. On transport error marks the
    slot BROKEN via OnTransportError.

    @param [in] ulCommand  Command code.
    @param [in] ulData     32-bit argument.
    @return 0 on success, non-zero on transport error (or no slot).
    @retval 0    Sent.
    @retval !=0  Not connected, or Send failed. */
extern "C" APIRET EXPENTRY Fc_SesPostCommand(ULONG ulCommand, ULONG ulData)
{
    ULONG  tid;
    ULONG  obj = 0;
    APIRET rc;
    int    ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: Fc_SesPostCommand(cmd=0x%lx, data=0x%lx)",
                         (unsigned long)ulCommand, (unsigned long)ulData);

    tid = QueryThreadTid();
    LockState();
    {
        ConnSlot *p = SlotFind(tid);
        if (p != NULL && p->state == SLOT_ACTIVE) {
            obj = p->obj;
            p->lastActivity = time(NULL);
        }
    }
    UnlockState();
    if (obj == 0) {
        F_DbgCtxExitf(ctx, "not connected (tid=%lu)", (unsigned long)tid);
        return (APIRET)-1;
    }

    rc = pfnFc_TrnPostCommand((void *)obj, (int)ulCommand, (int)ulData);
    if (rc != 0) {
        F_DbgCtxExitf(ctx, "rc=0x%lx, transport error", (unsigned long)rc);
        OnTransportError(tid);
        return rc;
    }
    F_DbgCtxExitf(ctx, "rc=0");
    return rc;
}

/*! @brief Async write of raw payload on this thread.

    Looks up aConns[QueryThreadTid()]. On transport error marks the
    slot BROKEN via OnTransportError.

    @param [in] pvData  Payload. Must not be NULL when cbData > 0.
    @param [in] cbData  Payload size in bytes.
    @return 0 on success, non-zero on transport error (or no slot).
    @retval 0    Sent.
    @retval !=0  Not connected, or Send failed. */
extern "C" APIRET EXPENTRY Fc_SesPostData(PVOID pvData, LONG cbData)
{
    ULONG  tid;
    ULONG  obj = 0;
    APIRET rc;
    int    ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: Fc_SesPostData(data=%p, cbData=%ld)",
                         pvData, (long)cbData);

    tid = QueryThreadTid();
    LockState();
    {
        ConnSlot *p = SlotFind(tid);
        if (p != NULL && p->state == SLOT_ACTIVE) {
            obj = p->obj;
            p->lastActivity = time(NULL);
        }
    }
    UnlockState();
    if (obj == 0) {
        F_DbgCtxExitf(ctx, "not connected (tid=%lu)", (unsigned long)tid);
        return (APIRET)-1;
    }

    rc = pfnFc_TrnPostData((void *)obj, pvData, (ULONG)cbData);
    if (rc != 0) {
        F_DbgCtxExitf(ctx, "rc=0x%lx, transport error", (unsigned long)rc);
        OnTransportError(tid);
        return rc;
    }
    F_DbgCtxExitf(ctx, "rc=0");
    return rc;
}

/*! @brief Get raw data from the server on this thread.

    Blocking: reads the next block from the transport.

    @param [out] pvBuffer     Destination buffer.
    @param [out] pcbReceived  Receives the number of bytes received.
    @param [in]  cbBuffer     Size of the destination buffer.
    @return 0 on success, non-zero on transport error (or no slot).
    @retval 0    Data read.
    @retval !=0  Not connected, or Recv failed. */
extern "C" APIRET EXPENTRY Fc_SesGetData(PVOID pvBuffer, PLONG pcbReceived,
                                         LONG cbBuffer)
{
    ULONG  tid;
    ULONG  obj = 0;
    ULONG  ul  = 0;
    APIRET rc;
    int    ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: Fc_SesGetData(buffer=%p, cbBuffer=%ld)",
                         pvBuffer, (long)cbBuffer);

    tid = QueryThreadTid();
    LockState();
    {
        ConnSlot *p = SlotFind(tid);
        if (p != NULL && p->state == SLOT_ACTIVE) {
            obj = p->obj;
            p->lastActivity = time(NULL);
        }
    }
    UnlockState();
    if (obj == 0) {
        F_DbgCtxExitf(ctx, "not connected (tid=%lu)", (unsigned long)tid);
        return (APIRET)-1;
    }

    rc = pfnFc_TrnGetData((void *)obj, pvBuffer, &ul, (ULONG)cbBuffer);
    if (pcbReceived) *pcbReceived = (LONG)ul;
    if (rc != 0) {
        F_DbgCtxExitf(ctx, "rc=0x%lx, transport error", (unsigned long)rc);
        OnTransportError(tid);
        return rc;
    }
    F_DbgCtxExitf(ctx, "rc=0, received=%lu", (unsigned long)ul);
    return rc;
}

/*! @brief Send a command with a 32-bit parameter and wait for reply
    on this thread.

    Synchronous: blocks until the server returns a 32-bit reply.

    @param [in] ulCommand    Command code.
    @param [in] ulParameter  32-bit parameter.
    @return Server reply code, or (APIRET)-1 on transport error.
    @retval -1       Not connected, or transport error.
    @retval retcode  Server reply code. */
extern "C" APIRET EXPENTRY Fc_SesSendCommand(ULONG ulCommand, ULONG ulParameter)
{
    ULONG  tid;
    ULONG  obj = 0;
    APIRET rc;
    int    ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: Fc_SesSendCommand(cmd=0x%lx, par=0x%lx)",
                         (unsigned long)ulCommand, (unsigned long)ulParameter);

    tid = QueryThreadTid();
    LockState();
    {
        ConnSlot *p = SlotFind(tid);
        if (p != NULL && p->state == SLOT_ACTIVE) {
            obj = p->obj;
            p->lastActivity = time(NULL);
        }
    }
    UnlockState();
    if (obj == 0) {
        F_DbgCtxExitf(ctx, "not connected (tid=%lu)", (unsigned long)tid);
        return (APIRET)-1;
    }

    rc = pfnFc_TrnSendCommand((void *)obj, (int)ulCommand, (int)ulParameter);
    if (rc == (APIRET)-1) {
        F_DbgCtxExitf(ctx, "transport error");
        OnTransportError(tid);
        return rc;
    }
    F_DbgCtxExitf(ctx, "rc=0x%lx", (unsigned long)rc);
    return rc;
}

/*! @brief Send a command with a payload and wait for reply on this
    thread.

    Synchronous: blocks until the server returns a 32-bit reply.

    @param [in] ulCommand    Command code.
    @param [in] ulParameter  32-bit parameter.
    @param [in] pvData       Pointer to the payload.
    @param [in] cbData       Payload size in bytes.
    @return Server reply code, or (APIRET)-1 on transport error.
    @retval -1       Not connected, or transport error.
    @retval retcode  Server reply code. */
extern "C" APIRET EXPENTRY Fc_SesSendCommandData(ULONG ulCommand, ULONG ulParameter,
                                                 PVOID pvData, LONG cbData)
{
    ULONG  tid;
    ULONG  obj = 0;
    APIRET rc;
    int    ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: Fc_SesSendCommandData(cmd=0x%lx, par=0x%lx, "
                         "data=%p, cbData=%ld)",
                         (unsigned long)ulCommand, (unsigned long)ulParameter,
                         pvData, (long)cbData);

    tid = QueryThreadTid();
    LockState();
    {
        ConnSlot *p = SlotFind(tid);
        if (p != NULL && p->state == SLOT_ACTIVE) {
            obj = p->obj;
            p->lastActivity = time(NULL);
        }
    }
    UnlockState();
    if (obj == 0) {
        F_DbgCtxExitf(ctx, "not connected (tid=%lu)", (unsigned long)tid);
        return (APIRET)-1;
    }

    rc = pfnFc_TrnSendCommandData((void *)obj, (int)ulCommand,
                                  (int)ulParameter, pvData, (ULONG)cbData);
    if (rc == (APIRET)-1) {
        F_DbgCtxExitf(ctx, "transport error");
        OnTransportError(tid);
        return rc;
    }
    F_DbgCtxExitf(ctx, "rc=0x%lx", (unsigned long)rc);
    return rc;
}
