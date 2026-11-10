/*!
 * @file Fs_server.cpp
 * @brief Fs_Server implementation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if defined(__OS2__) || defined(__OS2V2__)
  #include <process.h>
#endif

#include "Fs_server.hpp"
#include "F_debug.h"
#include "dispatch_win.hpp"

extern ULONG DispatchGpi(HPS hps, ULONG ulFnCode, PBYTE pArgs, ULONG cbArgs,
                         PULONG pcbPayload);

#define SERVER_RETURN(ctx, fmt, ...) \
    do { \
        F_DbgCtxExitf(ctx, fmt, ##__VA_ARGS__); \
        return; \
    } while (0)

/* ======================================================================== */
/*  Construction                                                            */
/* ======================================================================== */

Fs_Server::Fs_Server()
    : m_pfnRecv(NULL),
      m_pfnSend(NULL),
      m_fShutdown(FALSE),
      m_fEverHadClient(FALSE)
{
}

Fs_Server::~Fs_Server()
{
}

void Fs_Server::SetWire(Fs_PfnRecv pfnRecv, Fs_PfnSend pfnSend)
{
    m_pfnRecv = pfnRecv;
    m_pfnSend = pfnSend;
}

void Fs_Server::SetConfig(const F_ServerConfig &cfg)
{
    m_cfg = cfg;
}

/* ======================================================================== */
/*  Monitor thread                                                          */
/* ======================================================================== */

#if defined(__OS2__) || defined(__OS2V2__)
static void MonitorThreadProc(void *param)
{
    ((Fs_Server *)param)->MonitorLoop();
}
#else
static unsigned long __stdcall MonitorThreadProc(void *param)
{
    ((Fs_Server *)param)->MonitorLoop();
    return 0;
}
#endif

void Fs_Server::StartMonitor()
{
    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_INFO,
               "[SERVER] starting monitor thread (tick=%lu ms)",
               (unsigned long)m_cfg.ulMonitorTick);

#if defined(__OS2__) || defined(__OS2V2__)
    _beginthread(MonitorThreadProc, NULL, 65536, this);
#else
    CreateThread(NULL, 0, MonitorThreadProc, this, 0, NULL);
#endif
}

void Fs_Server::MonitorLoop()
{
    while (!m_fShutdown) {
        DosSleep(m_cfg.ulMonitorTick);
        if (m_fShutdown) break;

        time_t now = time(NULL);
        m_procs.Expire(now, m_cfg.ulInactive, m_cfg.ulGrace);

        int nActive = m_procs.CountActive();

        if (m_fEverHadClient && nActive == 0) {
            F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_INFO,
                       "[SERVER] all sessions gone, shutting down");
            m_fShutdown = TRUE;
            exit(0);
        }
    }
}

/* ======================================================================== */
/*  Response helpers                                                        */
/* ======================================================================== */

void Fs_Server::SendHeader(void *pRecvobj, ULONG ulStatus, ULONG ulResult,
                           ULONG cbPayload, ULONG ulLastError)
{
    struct {
        ULONG ulStatus;
        ULONG ulResult;
        ULONG cbPayload;
        ULONG ulLastError;
    } resp;

    resp.ulStatus    = ulStatus;
    resp.ulResult    = ulResult;
    resp.cbPayload   = cbPayload;
    resp.ulLastError = ulLastError;

    if (m_pfnSend) m_pfnSend(pRecvobj, &resp, sizeof(resp));
}

void Fs_Server::SendClientConfig(void *pRecvobj, Fs_ClientConnection *pThread)
{
    F_CLIENTCONFIG cfg;

    cfg.idClient    = pThread ? pThread->GetIdClient() : 0;
    cfg.ulInactive  = m_cfg.ulInactive;
    cfg.ulGrace     = m_cfg.ulGrace;
    cfg.flConfig    = (m_cfg.fAcceptLogs ? F_CFG_WANT_LOGS : 0)
                    | F_CFG_PING_REQUIRED;

    if (m_pfnSend) m_pfnSend(pRecvobj, &cfg, sizeof(cfg));

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
               "[SERVER] sent F_CLIENTCONFIG to %p (idClient=%lu)",
               pRecvobj, (unsigned long)cfg.idClient);
}

/* ======================================================================== */
/*  Client lifecycle                                                        */
/* ======================================================================== */

/*!
 * @brief Handle F_CMD_CLIENT_CONNECT.
 *
 * The client sends par=0 on the first CONNECT of the process, and
 * par=idClient on every subsequent thread's CONNECT. This mirrors
 * PMCLIENT's per-process model:
 *
 *   par == 0        -> allocate a fresh Fs_Process (idClient) and its
 *                      first Fs_ClientConnection, reply with F_CLIENTCONFIG.
 *   par != 0        -> locate the existing Fs_Process by idClient,
 *                      allocate an additional Fs_ClientConnection for
 *                      this recvobj, reply with F_CLIENTCONFIG.
 *
 * A recvobj may only carry one thread slot per process; duplicates
 * on the same recvobj are rejected.
 *
 * @param [in] pRecvobj  Transport connection handle.
 * @param [in] idClient  Client-supplied idClient (0 = new process).
 */
void Fs_Server::HandleClientConnect(void *pRecvobj, ULONG idClient)
{
    int ctx;
    ULONG idNew = 0;
    Fs_Process *pProc;
    Fs_ClientConnection *pThread;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleClientConnect(recvobj=%p, idClient=%lu)",
                         pRecvobj, (unsigned long)idClient);

    /* Subsequent thread of an existing process. */
    if (idClient != 0) {
        pProc = m_procs.FindByIdClient(idClient);
        if (!pProc) {
            F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_WARNING,
                       "[SERVER] CONNECT: idClient=%lu not found, rejecting",
                       (unsigned long)idClient);
            SendClientConfig(pRecvobj, NULL);
            SERVER_RETURN(ctx, "rejected (idClient=%lu not found)",
                          (unsigned long)idClient);
        }
        if (pProc->FindThreadByConn(pRecvobj) != NULL) {
            F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_WARNING,
                       "[SERVER] CONNECT: recvobj=%p already attached to "
                       "idClient=%lu", pRecvobj, (unsigned long)idClient);
            SendClientConfig(pRecvobj, NULL);
            SERVER_RETURN(ctx, "rejected (recvobj already attached)");
        }
        pThread = pProc->AllocThread(pRecvobj);
        if (!pThread) {
            F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                       "[SERVER] CONNECT: idClient=%lu no thread slot",
                       (unsigned long)idClient);
            SendClientConfig(pRecvobj, NULL);
            SERVER_RETURN(ctx, "rejected (no thread slot)");
        }
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_INFO,
                   "[SERVER] CONNECT: idClient=%lu new thread recvobj=%p",
                   (unsigned long)idClient, pRecvobj);
        m_fEverHadClient = TRUE;
        pThread->Touch();
        SendClientConfig(pRecvobj, pThread);
        SERVER_RETURN(ctx, "idClient=%lu (new thread)",
                      (unsigned long)idClient);
    }

    /* First CONNECT of a new process. */
    if (m_procs.FindByConn(pRecvobj) != NULL) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_WARNING,
                   "[SERVER] CONNECT: recvobj=%p already in a process, "
                   "rejecting", pRecvobj);
        SendClientConfig(pRecvobj, NULL);
        SERVER_RETURN(ctx, "rejected (already has session)");
    }

    pProc = m_procs.Create(pRecvobj, &idNew);
    if (!pProc) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[SERVER] CONNECT: table full, rejecting %p", pRecvobj);
        SendClientConfig(pRecvobj, NULL);
        SERVER_RETURN(ctx, "rejected (table full)");
    }

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_INFO,
               "[SERVER] CONNECT: new process idClient=%lu recvobj=%p",
               (unsigned long)idNew, pRecvobj);
    m_fEverHadClient = TRUE;

    pThread = pProc->FindThreadByConn(pRecvobj);
    if (pThread) pThread->Touch();
    SendClientConfig(pRecvobj, pThread);
    SERVER_RETURN(ctx, "new idClient=%lu", (unsigned long)idNew);
}

/*!
 * @brief Handle F_CMD_CLIENT_RECONNECT.
 *
 * Wire: cmd=RECONNECT, data=idClient, payload={ ULONG tid }.
 *
 * The client reconnects from a fresh recvobj after the previous one
 * has been closed or dropped. The server must have already received
 * the disconnect callback for the old recvobj (Fs_ClientConnection is
 * in StateDetached) before RECONNECT arrives. The resumed thread keeps
 * its HAB (Fs_Hab) alive across the transition.
 *
 * @param [in] pRecvobj  New transport handle.
 * @param [in] idClient  Client-supplied idClient.
 */
void Fs_Server::HandleClientReconnect(void *pRecvobj, ULONG idClient)
{
    int ctx;
    Fs_Process *pProc;
    Fs_ClientConnection *pThread;
    ULONG tid = 0;
    int   len = 0;
    int   rc;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleClientReconnect(idClient=%lu, "
                         "recvobj=%p)", (unsigned long)idClient, pRecvobj);

    if (idClient == 0) {
        SendClientConfig(pRecvobj, NULL);
        SERVER_RETURN(ctx, "rejected (idClient=0)");
    }

    if (!m_pfnRecv) {
        SendClientConfig(pRecvobj, NULL);
        SERVER_RETURN(ctx, "rejected (no wire)");
    }

    /* Read the {tid} payload. Must come before any reject so the
       wire stream stays in sync with the client's PostData. */
    rc = m_pfnRecv(pRecvobj, &tid, &len, sizeof(tid));
    if (rc != 0 || len != (int)sizeof(tid) || tid == 0) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[SERVER] RECONNECT: bad tid payload "
                   "(rc=%d, len=%d, tid=%lu)",
                   rc, len, (unsigned long)tid);
        SendClientConfig(pRecvobj, NULL);
        SERVER_RETURN(ctx, "rejected (bad tid payload)");
    }

    pProc = m_procs.FindByIdClient(idClient);
    if (!pProc) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_WARNING,
                   "[SERVER] RECONNECT: idClient=%lu not found",
                   (unsigned long)idClient);
        SendClientConfig(pRecvobj, NULL);
        SERVER_RETURN(ctx, "rejected (idClient=%lu not found)",
                      (unsigned long)idClient);
    }

    pThread = pProc->FindThreadByTid(tid);
    if (!pThread) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_WARNING,
                   "[SERVER] RECONNECT: tid=%lu not in idClient=%lu",
                   (unsigned long)tid, (unsigned long)idClient);
        SendClientConfig(pRecvobj, NULL);
        SERVER_RETURN(ctx, "rejected (tid=%lu not found)",
                      (unsigned long)tid);
    }

    if (!pThread->IsDetached()) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_WARNING,
                   "[SERVER] RECONNECT: tid=%lu not DETACHED "
                   "(idClient=%lu)", (unsigned long)tid,
                   (unsigned long)idClient);
        SendClientConfig(pRecvobj, NULL);
        SERVER_RETURN(ctx, "rejected (tid=%lu not DETACHED)",
                      (unsigned long)tid);
    }

    pThread->ToActive(pRecvobj, time(NULL));
    m_fEverHadClient = TRUE;

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_INFO,
               "[SERVER] RECONNECT: idClient=%lu tid=%lu resumed",
               (unsigned long)idClient, (unsigned long)tid);
    SendClientConfig(pRecvobj, pThread);
    SERVER_RETURN(ctx, "resumed idClient=%lu tid=%lu",
                  (unsigned long)idClient, (unsigned long)tid);
}

/*!
 * @brief Handle F_CMD_CLIENT_EXIT.
 *
 * Per-thread shutdown: detach this thread's HAB (cascading through
 * GpiPsDestroyByHab), free the thread slot, and destroy the whole
 * process only if this was its last thread. This mirrors the
 * per-thread model of PMCLIENT: each Fc_SesDisconnect on the client
 * closes one thread's session; the process lives as long as any of
 * its threads is still connected.
 *
 * @param [in] pThread  Thread slot identified by recvobj.
 */
void Fs_Server::HandleClientExit(Fs_ClientConnection *pThread)
{
    int ctx;
    ULONG idClient;
    ULONG iHab;
    Fs_Process *pProc;

    if (!pThread) return;

    idClient = pThread->GetIdClient();
    iHab     = pThread->GetIHab();
    pProc    = m_procs.FindByIdClient(idClient);

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleClientExit(idClient=%lu, iHab=%lu)",
                         (unsigned long)idClient, (unsigned long)iHab);

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_INFO,
               "[SERVER] EXIT: idClient=%lu, thread iHab=%lu",
               (unsigned long)idClient, (unsigned long)iHab);

    /* Tear down this thread's HAB (and any PS attached to it). */
    if (pProc && iHab != 0) {
        pProc->DetachHab(iHab);
    }

    /* Free this thread's connection slot. */
    pThread->Shutdown();

    /* Destroy the whole process only when its last thread is gone. */
    if (pProc && pProc->CountActiveThreads() == 0) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                   "[SERVER] EXIT: last thread of idClient=%lu gone, "
                   "destroying process", (unsigned long)idClient);
        m_procs.Destroy(idClient);
    }

    SERVER_RETURN(ctx, "done");
}

/*!
 * @brief Handle F_CMD_CLIENT_PING.
 *
 * Fire-and-forget touch. The thread is identified by recvobj (which
 * is the transport-level authority), not by the client-supplied
 * idClient. The idClient is checked anyway: a mismatch means the
 * client thinks it is talking to a different session than the one
 * this recvobj is actually bound to, which is worth logging even
 * though we still touch the thread (recvobj is trusted).
 *
 * @param [in] pThread   Thread slot identified by recvobj.
 * @param [in] idClient  Client-supplied idClient (informational).
 */
void Fs_Server::HandleClientPing(Fs_ClientConnection *pThread, ULONG idClient)
{
    int ctx;

    if (!pThread) return;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleClientPing(idClient=%lu, "
                         "thread idClient=%lu)",
                         (unsigned long)idClient,
                         (unsigned long)pThread->GetIdClient());

    if (idClient != pThread->GetIdClient()) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_WARNING,
                   "[SERVER] PING: idClient=%lu mismatch thread idClient=%lu "
                   "on recvobj=%p", (unsigned long)idClient,
                   (unsigned long)pThread->GetIdClient(),
                   pThread->GetRecvobj());
    }

    pThread->Touch();
    SERVER_RETURN(ctx, "done");
}

void Fs_Server::HandleDisconnect(void *pRecvobj)
{
    int ctx;
    Fs_ClientConnection *pThread;

    pThread = m_procs.FindThreadByConn(pRecvobj);

    if (!pThread) return;
    if (!pThread->IsActive()) return;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleDisconnect(recvobj=%p, idClient=%lu)",
                         pRecvobj, (unsigned long)pThread->GetIdClient());

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_INFO,
               "[SERVER] DISCONNECT: idClient=%lu recvobj=%p -> DETACHED",
               (unsigned long)pThread->GetIdClient(), pRecvobj);
    pThread->ToDetached(time(NULL));
    SERVER_RETURN(ctx, "-> DETACHED");
}

/* ======================================================================== */
/*  GPI dispatch                                                            */
/* ======================================================================== */

void Fs_Server::HandleGpiCall(Fs_ClientConnection *pThread,
                              ULONG fnCode, ULONG data)
{
    void *pRecvobj = pThread->GetRecvobj();
    HPS   hps      = (HPS)data;
    ULONG cbArgs    = 0;
    ULONG cbPayload = 0;
    PVOID pv        = NULL;
    PBYTE pArgs     = NULL;
    int   rc, len;
    int   ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleGpiCall(fnCode=0x%lx, hps=%p)",
                         (unsigned long)fnCode, (void*)hps);

    if (!m_pfnRecv) {
        SERVER_RETURN(ctx, "no wire");
    }

    rc = m_pfnRecv(pRecvobj, &cbArgs, &len, sizeof(cbArgs));
    if (rc == 0 && len == (int)sizeof(cbArgs)) {
        if (cbArgs > 0) {
            if (DosAllocMem(&pv, cbArgs,
                            PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
                SendHeader(pRecvobj, 1, GPI_ERROR, 0, 0);
                SERVER_RETURN(ctx, "no memory (cbArgs=%lu)",
                              (unsigned long)cbArgs);
            }
            pArgs = (PBYTE)pv;
            rc = m_pfnRecv(pRecvobj, pArgs, &len, cbArgs);
        }

        ULONG ulResult = DispatchGpi(hps, fnCode, pArgs, cbArgs, &cbPayload);
        SendHeader(pRecvobj, 0, ulResult, cbPayload, 0);
        if (cbPayload > 0 && pArgs != NULL && cbPayload <= cbArgs && m_pfnSend)
            m_pfnSend(pRecvobj, pArgs, (int)cbPayload);

        if (pv) DosFreeMem(pv);
        SERVER_RETURN(ctx, "result=0x%lx", (unsigned long)ulResult);
    } else {
        SendHeader(pRecvobj, 1, GPI_ERROR, 0, 0);
        SERVER_RETURN(ctx, "recv header failed (rc=%d, len=%d)", rc, len);
    }
}

void Fs_Server::HandleWinCall(Fs_ClientConnection *pThread,
                              ULONG fnCode, ULONG data)
{
    void *pRecvobj = pThread->GetRecvobj();
    ULONG cbArgs    = 0;
    ULONG cbPayload = 0;
    PVOID pv        = NULL;
    PBYTE pArgs     = NULL;
    ULONG ulResult  = 0;
    int   rc, len;
    int   ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleWinCall(fnCode=0x%lx, data=0x%lx, "
                         "idClient=%lu)",
                         (unsigned long)fnCode, (unsigned long)data,
                         (unsigned long)pThread->GetIdClient());

    if (!m_pfnRecv) {
        SERVER_RETURN(ctx, "no wire");
    }

    /* HAB lifecycle (0x0001-0x000F): sync-only, no cbArgs, no payload.
       Reply = 4-byte ulResult, per Fc_TrnSendCommand's contract
       (Fc_trn.cpp: PostCommand + GetData(4)). */
    if (fnCode >= NWinInitialize && fnCode <= 0x000F) {
        ulResult = DispatchWinHab(&m_procs, pRecvobj, fnCode, data,
                                   NULL, 0, &cbPayload);
        if (m_pfnSend)
            m_pfnSend(pRecvobj, &ulResult, sizeof(ulResult));
        SERVER_RETURN(ctx, "result=0x%lx (sync HAB)",
                      (unsigned long)ulResult);
    }

    rc = m_pfnRecv(pRecvobj, &cbArgs, &len, sizeof(cbArgs));
    if (rc != 0 || len != (int)sizeof(cbArgs)) {
        SendHeader(pRecvobj, 1, GPI_ERROR, 0, 0);
        SERVER_RETURN(ctx, "recv header failed (rc=%d, len=%d)", rc, len);
    }
    if (cbArgs > 0) {
        if (DosAllocMem(&pv, cbArgs,
                        PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
            SendHeader(pRecvobj, 1, GPI_ERROR, 0, 0);
            SERVER_RETURN(ctx, "no memory (cbArgs=%lu)",
                          (unsigned long)cbArgs);
        }
        pArgs = (PBYTE)pv;
        rc = m_pfnRecv(pRecvobj, pArgs, &len, cbArgs);
        if (rc != 0 || len != (int)cbArgs) {
            DosFreeMem(pv);
            SendHeader(pRecvobj, 1, GPI_ERROR, 0, 0);
            SERVER_RETURN(ctx, "recv payload failed (rc=%d, len=%d)", rc, len);
        }
    }

    /* Phase 2: only HAB (0x0001-0x000F). Other domains are stubs = 0.
       (HAB already handled above on the sync path.) */
    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_WARNING,
               "[SERVER] HandleWinCall: fnCode=0x%lx not implemented",
               (unsigned long)fnCode);
    ulResult = 0;

    SendHeader(pRecvobj, 0, ulResult, cbPayload, 0);
    if (cbPayload > 0 && pArgs != NULL && cbPayload <= cbArgs && m_pfnSend)
        m_pfnSend(pRecvobj, pArgs, (int)cbPayload);

    if (pv) DosFreeMem(pv);
    SERVER_RETURN(ctx, "result=0x%lx", (unsigned long)ulResult);
}

void Fs_Server::HandleCoreCommand(Fs_ClientConnection *pThread,
                                  ULONG ncmd, ULONG data)
{
    void *pRecvobj = pThread->GetRecvobj();
    char buf[256];
    int  rc, len;
    int  ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleCoreCommand(ncmd=0x%lx, data=0x%lx, "
                         "idClient=%lu)",
                         (unsigned long)ncmd, (unsigned long)data,
                         (unsigned long)pThread->GetIdClient());

    switch (ncmd) {

        case F_CMD_GET_HPS: {
            HPS hps = (HPS)1;
            F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                       "[SERVER] GET_HPS(hwnd=0x%x) -> hps=%p (stub)",
                       data, (void*)hps);
            if (m_pfnSend) m_pfnSend(pRecvobj, &hps, sizeof(HPS));
            SERVER_RETURN(ctx, "GET_HPS -> hps=%p (stub)", (void*)hps);
        }

        case F_CMD_RELEASE_HPS: {
            int rc1 = 0;
            F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                       "[SERVER] RELEASE_HPS(hps=0x%x) -> 0", data);
            if (m_pfnSend) m_pfnSend(pRecvobj, &rc1, sizeof(int));
            SERVER_RETURN(ctx, "RELEASE_HPS -> 0");
        }

        case F_CMD_DB_PRINT: {
            if (m_pfnRecv && m_cfg.fAcceptLogs) {
                m_pfnRecv(pRecvobj, buf, &len, sizeof(buf));
                F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_INFO,
                           "DEBUG[id=%lu]: %s",
                           (unsigned long)pThread->GetIdClient(), buf);
            }
            SERVER_RETURN(ctx, "DB_PRINT");
        }

        case F_CMD_FATAL_COMMON: {
            if (m_pfnRecv) {
                m_pfnRecv(pRecvobj, buf, &len, sizeof(buf));
                F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_FATAL,
                           "FATAL[id=%lu]: %s",
                           (unsigned long)pThread->GetIdClient(), buf);
            }
            SERVER_RETURN(ctx, "FATAL_COMMON");
        }

        default: {
            unsigned long reply = (unsigned long)ncmd;
            if (m_pfnSend) m_pfnSend(pRecvobj, &reply, sizeof(reply));
            SERVER_RETURN(ctx, "unknown cmd 0x%x -> echoed", ncmd);
        }
    }
}

/* ======================================================================== */
/*  Main command entry                                                      */
/* ======================================================================== */

void Fs_Server::HandleCommand(void *pRecvobj, int ncmd, int data, int threadNum)
{
    int                  ctx;
    Fs_ClientConnection *pThread;

    (void)threadNum;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleCommand(ncmd=0x%x, data=0x%x, "
                         "recvobj=%p)", ncmd, data, pRecvobj);

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
               "[SERVER] IN  -> recvobj=%p, ncmd=0x%x, data=0x%x",
               pRecvobj, ncmd, data);

    /* Lifecycle commands (do not require an existing session) */
    switch (ncmd) {
    case F_CMD_CLIENT_CONNECT:
        HandleClientConnect(pRecvobj, (ULONG)data);
        SERVER_RETURN(ctx, "CONNECT done");
    case F_CMD_CLIENT_RECONNECT:
        HandleClientReconnect(pRecvobj, (ULONG)data);
        SERVER_RETURN(ctx, "RECONNECT done");
    }

    pThread = m_procs.FindThreadByConn(pRecvobj);

    /* Every remaining command requires an existing session. */
    if (!pThread) {
        ULONG err = (ULONG)-1;
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[SERVER] ERROR: no session for recvobj=%p, cmd 0x%x, "
                   "returning error", pRecvobj, ncmd);
        if (m_pfnSend) m_pfnSend(pRecvobj, &err, sizeof(err));
        SERVER_RETURN(ctx, "no session");
    }
    pThread->Touch();

    if (ncmd == F_CMD_CLIENT_EXIT) {
        HandleClientExit(pThread);
        SERVER_RETURN(ctx, "EXIT done");
    }
    if (ncmd == F_CMD_CLIENT_PING) {
        HandleClientPing(pThread, (ULONG)data);
        SERVER_RETURN(ctx, "PING done");
    }
    if (F_CMD_GET_CLASS((ULONG)ncmd) == F_CMD_CLASS_GPI) {
        HandleGpiCall(pThread, F_CMD_GET_OPCODE((ULONG)ncmd), (ULONG)data);
        SERVER_RETURN(ctx, "GPI call done");
    }
    if (F_CMD_GET_CLASS((ULONG)ncmd) == F_CMD_CLASS_WIN) {
        HandleWinCall(pThread, F_CMD_GET_OPCODE((ULONG)ncmd), (ULONG)data);
        SERVER_RETURN(ctx, "WIN call done");
    }
    HandleCoreCommand(pThread, (ULONG)ncmd, (ULONG)data);
    SERVER_RETURN(ctx, "CORE command done");
}

#undef SERVER_RETURN
