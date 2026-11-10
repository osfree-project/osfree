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

/*!
 * @def SERVER_RETURN
 * @brief Close the debug context and return from the current function.
 * @param [in] ctx  Context level from F_DbgCtxEnterf.
 * @param [in] fmt  printf-style format for the exit message.
 * @param [in] ...  Arguments for the format string.
 */
#define SERVER_RETURN(ctx, fmt, ...) \
    do { \
        F_DbgCtxExitf(ctx, fmt, ##__VA_ARGS__); \
        return; \
    } while (0)

/* ======================================================================== */
/*  Construction                                                            */
/* ======================================================================== */

/*!
 * @brief Default constructor. Wire callbacks are installed by SetWire.
 */
Fs_Server::Fs_Server()
    : m_pfnRecv(NULL),
      m_pfnSend(NULL),
      m_fShutdown(FALSE),
      m_fEverHadClient(FALSE)
{
}

/*!
 * @brief Destructor. Registries clean themselves up.
 */
Fs_Server::~Fs_Server()
{
}

/*!
 * @brief Install the transport recv/send callbacks.
 * @param [in] pfnRecv  Read callback (PMTCP backend).
 * @param [in] pfnSend  Write callback (PMTCP backend).
 */
void Fs_Server::SetWire(Fs_PfnRecv pfnRecv, Fs_PfnSend pfnSend)
{
    m_pfnRecv = pfnRecv;
    m_pfnSend = pfnSend;
}

/*!
 * @brief Apply the runtime configuration and propagate to subobjects.
 * @param [in] cfg  Configuration read from fpmsrv.ini.
 */
void Fs_Server::SetConfig(const F_ServerConfig &cfg)
{
    m_cfg = cfg;
    m_habs.SetMax(cfg.ulMaxHabs);
}

/* ======================================================================== */
/*  Monitor thread                                                          */
/* ======================================================================== */

#if defined(__OS2__) || defined(__OS2V2__)
/*!
 * @brief OS/2 thread entry for the monitor loop.
 * @param [in] param  Fs_Server* (this).
 */
static void MonitorThreadProc(void *param)
{
    ((Fs_Server *)param)->MonitorLoop();
}
#else
/*!
 * @brief Win32 thread entry for the monitor loop.
 * @param [in] param  Fs_Server* (this).
 * @return Always 0.
 * @retval 0 Thread finished.
 */
static unsigned long __stdcall MonitorThreadProc(void *param)
{
    ((Fs_Server *)param)->MonitorLoop();
    return 0;
}
#endif

/*!
 * @brief Start the monitor thread (once per process).
 */
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

/*!
 * @brief Monitor thread main loop.
 *
 * Wakes every m_cfg.ulMonitorTick ms, expires idle / detached
 * sessions via ExpireSessions(), and shuts the server down when
 * the last process has gone.
 */
void Fs_Server::MonitorLoop()
{
    while (!m_fShutdown) {
        DosSleep(m_cfg.ulMonitorTick);
        if (m_fShutdown) break;

        ExpireSessions(time(NULL));

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
/*  Session timeout                                                         */
/* ======================================================================== */

/*!
 * @brief Expire idle / detached sessions.
 *
 * Two timeouts are applied (spec2 §5.2):
 *
 *   ACTIVE -- inactive timeout -- DETACHED
 *   DETACHED -- grace timeout -- FREE (via TeardownThread)
 *
 * ACTIVE -> DETACHED mirrors a transport disconnect: the slot keeps
 * its HAB and thread alive so a later F_CMD_CLIENT_RECONNECT can
 * resume. DETACHED -> FREE does the full teardown: HAB destroyed
 * (cascading through GpiPsDestroyByHab), thread slot and connection
 * freed, and if this was the last thread of the process, Fs_Process
 * destroyed together with any orphan HABs.
 *
 * ulInactive == 0 disables the inactive timeout; ulGrace == 0
 * disables the grace timeout (detached sessions never expire).
 *
 * @param [in] now  Current time (from time(NULL)).
 */
void Fs_Server::ExpireSessions(time_t now)
{
    int i;

    for (i = 0; i < F_MAX_CLIENTS; i++) {
        Fs_Connection *pConn = m_conns.GetByIndex(i);

        if (!pConn || pConn->IsFree()) continue;

        if (pConn->IsActive() &&
            pConn->IsExpiredInactive(now, m_cfg.ulInactive)) {
            F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_INFO,
                       "[SERVER] EXPIRE: recvobj=%p ACTIVE -> DETACHED "
                       "(inactive)", pConn->GetRecvobj());
            pConn->Detach(now);
            continue;
        }

        if (pConn->IsDetached() &&
            pConn->IsExpiredGrace(now, m_cfg.ulGrace)) {
            F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_INFO,
                       "[SERVER] EXPIRE: connection DETACHED -> FREE "
                       "(grace elapsed)");
            TeardownThread(pConn->GetThread(), "EXPIRE");
        }
    }
}

/*!
 * @brief Tear down one thread: destroy its HAB, free slot + connection.
 *
 * Idempotent with respect to the process: if this was the last
 * active thread of the process, Fs_Process and any remaining HABs
 * are destroyed too. Called from HandleClientExit (EXIT command) and
 * from ExpireSessions (grace expiry).
 *
 * @param [in] pThread  Thread slot to tear down.
 * @param [in] reason   Short tag for log messages ("EXIT" / "EXPIRE").
 */
void Fs_Server::TeardownThread(Fs_Thread *pThread, const char *reason)
{
    Fs_Process *pProc;
    ULONG       idClient;
    ULONG       iHab;

    if (!pThread) return;

    pProc    = pThread->GetProcess();
    idClient = pProc ? pProc->GetIdClient() : 0;
    iHab     = pThread->GetIHab();

    if (pProc && iHab != 0) {
        m_habs.Destroy(idClient, pThread->GetTid(), iHab);
        pThread->ClearHab();
    }
    if (pProc) pProc->FreeThread(pThread);

    if (pProc && pProc->CountActiveThreads() == 0) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                   "[SERVER] %s: last thread of idClient=%lu gone, "
                   "destroying process", reason, (unsigned long)idClient);
        m_procs.Destroy(idClient);
        m_habs.DestroyAll(idClient);
    }
}

/* ======================================================================== */
/*  Response helpers                                                        */
/* ======================================================================== */

/*!
 * @brief Send the 16-byte FPM_RESP_HEADER.
 * @param [in] pRecvobj   Transport handle.
 * @param [in] ulStatus   0 = ok, non-zero = transport error.
 * @param [in] ulResult   Function result value.
 * @param [in] cbPayload  Size of the payload that follows (0 = none).
 * @param [in] ulLastError Per-connection PMERR_* (currently always 0).
 */
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

/*!
 * @brief Send F_CLIENTCONFIG in reply to CONNECT / RECONNECT.
 * @param [in] pRecvobj  Transport handle.
 * @param [in] pThread   Thread slot (idClient source), or NULL on reject.
 */
void Fs_Server::SendClientConfig(void *pRecvobj, Fs_Thread *pThread)
{
    F_CLIENTCONFIG cfg;

    cfg.idClient   = (pThread && pThread->GetProcess())
                     ? pThread->GetProcess()->GetIdClient() : 0;
    cfg.ulInactive = m_cfg.ulInactive;
    cfg.ulGrace    = m_cfg.ulGrace;
    cfg.flConfig   = (m_cfg.fAcceptLogs ? F_CFG_WANT_LOGS : 0)
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
 * par == 0        -> allocate a fresh Fs_Process (idClient) and its
 *                    first Fs_Thread + Fs_Connection pair.
 * par != 0        -> locate the existing Fs_Process by idClient and
 *                    allocate an additional Fs_Thread + Fs_Connection.
 *
 * A recvobj may only carry one thread slot per process; duplicates on
 * the same recvobj are rejected.
 *
 * @param [in] pRecvobj  Transport connection handle.
 * @param [in] idClient  Client-supplied idClient (0 = new process).
 */
void Fs_Server::HandleClientConnect(void *pRecvobj, ULONG idClient)
{
    int ctx;
    ULONG idNew = 0;
    Fs_Process *pProc;
    Fs_Thread *pThread;
    Fs_Connection *pExisting;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleClientConnect(recvobj=%p, idClient=%lu)",
                         pRecvobj, (unsigned long)idClient);

    pExisting = m_conns.FindByRecvobj(pRecvobj);
    if (pExisting != NULL) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_WARNING,
                   "[SERVER] CONNECT: recvobj=%p already attached, rejecting",
                   pRecvobj);
        SendClientConfig(pRecvobj, NULL);
        SERVER_RETURN(ctx, "rejected (recvobj already attached)");
    }

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
        pThread = pProc->AllocThread(&m_conns, pRecvobj);
        if (!pThread) {
            F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                       "[SERVER] CONNECT: idClient=%lu no thread/conn slot",
                       (unsigned long)idClient);
            SendClientConfig(pRecvobj, NULL);
            SERVER_RETURN(ctx, "rejected (no thread/conn slot)");
        }
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_INFO,
                   "[SERVER] CONNECT: idClient=%lu new thread recvobj=%p",
                   (unsigned long)idClient, pRecvobj);
        m_fEverHadClient = TRUE;
        pThread->GetConn()->Touch();
        SendClientConfig(pRecvobj, pThread);
        SERVER_RETURN(ctx, "idClient=%lu (new thread)",
                      (unsigned long)idClient);
    }

    pProc = m_procs.Create(&m_conns, pRecvobj, &idNew);
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

    {
        Fs_Connection *pConn = m_conns.FindByRecvobj(pRecvobj);
        pThread = pConn ? pConn->GetThread() : NULL;
    }
    if (pThread) pThread->GetConn()->Touch();
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
 * the disconnect callback for the old recvobj (Fs_Connection is in
 * StateDetached) before RECONNECT arrives. The resumed thread keeps
 * its HAB (Fs_Hab) alive across the transition.
 *
 * @param [in] pRecvobj  New transport handle.
 * @param [in] idClient  Client-supplied idClient.
 */
void Fs_Server::HandleClientReconnect(void *pRecvobj, ULONG idClient)
{
    int ctx;
    Fs_Process *pProc;
    Fs_Thread *pThread;
    Fs_Connection *pConn;
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

    pConn = pThread->GetConn();
    if (!pConn || !pConn->IsDetached()) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_WARNING,
                   "[SERVER] RECONNECT: tid=%lu not DETACHED "
                   "(idClient=%lu)", (unsigned long)tid,
                   (unsigned long)idClient);
        SendClientConfig(pRecvobj, NULL);
        SERVER_RETURN(ctx, "rejected (tid=%lu not DETACHED)",
                      (unsigned long)tid);
    }

    pConn->Attach(pRecvobj);
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
 * Per-thread shutdown: destroy this thread's HAB (cascading through
 * GpiPsDestroyByHab), free the thread slot and its connection, and
 * destroy the whole process only if this was its last thread.
 *
 * @param [in] pThread  Thread slot identified by recvobj.
 */
void Fs_Server::HandleClientExit(Fs_Thread *pThread)
{
    int ctx;
    ULONG idClient;
    ULONG iHab;

    if (!pThread) return;

    idClient = pThread->GetProcess()
               ? pThread->GetProcess()->GetIdClient() : 0;
    iHab     = pThread->GetIHab();

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleClientExit(idClient=%lu, iHab=%lu)",
                         (unsigned long)idClient, (unsigned long)iHab);

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_INFO,
               "[SERVER] EXIT: idClient=%lu, thread iHab=%lu",
               (unsigned long)idClient, (unsigned long)iHab);

    TeardownThread(pThread, "EXIT");
    SERVER_RETURN(ctx, "done");
}

/*!
 * @brief Handle F_CMD_CLIENT_PING.
 *
 * Fire-and-forget touch. The thread is identified by recvobj (which
 * is the transport-level authority), not by the client-supplied
 * idClient. A mismatch is logged but the thread is still touched.
 *
 * @param [in] pThread   Thread slot identified by recvobj.
 * @param [in] idClient  Client-supplied idClient (informational).
 */
void Fs_Server::HandleClientPing(Fs_Thread *pThread, ULONG idClient)
{
    int ctx;

    if (!pThread) return;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleClientPing(idClient=%lu, "
                         "thread idClient=%lu)",
                         (unsigned long)idClient,
                         (unsigned long)(pThread->GetProcess()
                                         ? pThread->GetProcess()->GetIdClient()
                                         : 0));

    if (pThread->GetProcess() &&
        idClient != pThread->GetProcess()->GetIdClient()) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_WARNING,
                   "[SERVER] PING: idClient=%lu mismatch thread idClient=%lu "
                   "on recvobj=%p", (unsigned long)idClient,
                   (unsigned long)pThread->GetProcess()->GetIdClient(),
                   pThread->GetConn() ? pThread->GetConn()->GetRecvobj() : NULL);
    }

    if (pThread->GetConn()) pThread->GetConn()->Touch();
    SERVER_RETURN(ctx, "done");
}

/*!
 * @brief Handle a transport disconnect (PMTCP disc_cb).
 *
 * Marks the thread's connection as DETACHED so a subsequent
 * F_CMD_CLIENT_RECONNECT can resume it. Does not touch the HAB
 * (Fs_Hab survives across reconnect until ulGrace expires or EXIT).
 *
 * @param [in] pRecvobj  Transport handle of the dropped connection.
 */
void Fs_Server::HandleDisconnect(void *pRecvobj)
{
    int ctx;
    Fs_Connection *pConn;

    pConn = m_conns.FindByRecvobj(pRecvobj);

    if (!pConn) return;
    if (!pConn->IsActive()) return;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleDisconnect(recvobj=%p)", pRecvobj);

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_INFO,
               "[SERVER] DISCONNECT: recvobj=%p -> DETACHED", pRecvobj);
    pConn->Detach(time(NULL));
    SERVER_RETURN(ctx, "-> DETACHED");
}

/* ======================================================================== */
/*  GPI dispatch                                                            */
/* ======================================================================== */

/*!
 * @brief Handle a GPI command: read cbArgs + payload, dispatch, reply.
 *
 * Reads the 4-byte cbArgs header, then cbArgs bytes of payload.
 * Calls DispatchGpi with the decoded HPS (data) and the marshaled
 * arguments. Sends FPM_RESP_HEADER + optional payload.
 *
 * @param [in] pThread  Thread slot identified by recvobj.
 * @param [in] fnCode   NGpi* opcode (F_CMD_GET_OPCODE of the wire cmd).
 * @param [in] data     Primary handle (HPS).
 */
void Fs_Server::HandleGpiCall(Fs_Thread *pThread,
                              ULONG fnCode, ULONG data)
{
    void *pRecvobj = pThread->GetConn()->GetRecvobj();
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

        ULONG ulResult = DispatchGpi(pRecvobj, fnCode, data,
                                      pArgs, cbArgs, &cbPayload);
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

/*!
 * @brief Handle a Win* command.
 *
 * HAB lifecycle (fnCode 0x0001..0x000F) is sync: no cbArgs is read
 * and the reply is a bare 4-byte ulResult (matching Fc_TrnSendCommand
 * in Fc_trn.cpp). All other WIN domains use the async path: cbArgs +
 * payload on the wire, FPM_RESP_HEADER + optional payload on reply.
 *
 * Routing by fnCode range selects one of the DispatchWin* methods.
 * Each dispatcher is private and shares the uniform signature
 * (pRecvobj, fnCode, data, pArgs, cbArgs, pcbPayload).
 *
 * @param [in] pThread  Thread slot identified by recvobj.
 * @param [in] fnCode   NWin* opcode (0x0001..0x01BF).
 * @param [in] data     Primary handle (HAB/HWND/HMQ/HPS/…).
 */
void Fs_Server::HandleWinCall(Fs_Thread *pThread,
                              ULONG fnCode, ULONG data)
{
    void *pRecvobj = pThread->GetConn()->GetRecvobj();
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
                         (unsigned long)(pThread->GetProcess()
                                         ? pThread->GetProcess()->GetIdClient()
                                         : 0));

    if (!m_pfnRecv) {
        SERVER_RETURN(ctx, "no wire");
    }

    /* HAB lifecycle (0x0001-0x000F): sync, no cbArgs, 4-byte reply. */
    if (fnCode >= NWinInitialize && fnCode <= 0x000F) {
        ulResult = DispatchWinHab(pRecvobj, fnCode, data,
                                   NULL, 0, &cbPayload);
        if (m_pfnSend)
            m_pfnSend(pRecvobj, &ulResult, sizeof(ulResult));
        SERVER_RETURN(ctx, "result=0x%lx (sync HAB)", (unsigned long)ulResult);
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

    if (fnCode >= 0x0001 && fnCode <= 0x000F)
        ulResult = DispatchWinHab  (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x0010 && fnCode <= 0x001F)
        ulResult = DispatchWinMq   (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x0020 && fnCode <= 0x005F)
        ulResult = DispatchWinWnd  (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x0060 && fnCode <= 0x007F)
        ulResult = DispatchWinPs   (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x0080 && fnCode <= 0x009F)
        ulResult = DispatchWinCls  (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x00A0 && fnCode <= 0x00BF)
        ulResult = DispatchWinDlg  (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x00C0 && fnCode <= 0x00DF)
        ulResult = DispatchWinPtr  (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x00E0 && fnCode <= 0x00E7)
        ulResult = DispatchWinClip (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x00E8 && fnCode <= 0x00EF)
        ulResult = DispatchWinAtom (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x00F0 && fnCode <= 0x00FF)
        ulResult = DispatchWinStr  (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x0100 && fnCode <= 0x010F)
        ulResult = DispatchWinErr  (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x0110 && fnCode <= 0x011F)
        ulResult = DispatchWinSys  (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x0120 && fnCode <= 0x013F)
        ulResult = DispatchWinRect (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x0140 && fnCode <= 0x014F)
        ulResult = DispatchWinInp  (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x0150 && fnCode <= 0x015F)
        ulResult = DispatchWinDraw (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x0160 && fnCode <= 0x016F)
        ulResult = DispatchWinFrame(pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x0170 && fnCode <= 0x017F)
        ulResult = DispatchWinSync (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x0180 && fnCode <= 0x018F)
        ulResult = DispatchWinMisc (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x0190 && fnCode <= 0x019F)
        ulResult = DispatchWin32   (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x01A0 && fnCode <= 0x01AF)
        ulResult = DispatchWinMisc (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else if (fnCode >= 0x01B0 && fnCode <= 0x01BF)
        ulResult = DispatchWinDax  (pRecvobj, fnCode, data, pArgs, cbArgs, &cbPayload);
    else {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_WARNING,
                   "[SERVER] HandleWinCall: fnCode=0x%lx out of range",
                   (unsigned long)fnCode);
        ulResult = 0;
    }

    SendHeader(pRecvobj, 0, ulResult, cbPayload, 0);
    if (cbPayload > 0 && pArgs != NULL && cbPayload <= cbArgs && m_pfnSend)
        m_pfnSend(pRecvobj, pArgs, (int)cbPayload);

    if (pv) DosFreeMem(pv);
    SERVER_RETURN(ctx, "result=0x%lx", (unsigned long)ulResult);
}

/*!
 * @brief Handle a CORE-class command (session mgmt, debug, HPS stubs).
 *
 * Covers F_CMD_GET_HPS, F_CMD_RELEASE_HPS, F_CMD_DB_PRINT and
 * F_CMD_FATAL_COMMON. Any other CORE command is echoed back so the
 * client's Fc_SesSendCommand returns without blocking.
 *
 * @param [in] pThread  Thread slot identified by recvobj.
 * @param [in] ncmd     Wire command code (F_CMD_CORE(opcode)).
 * @param [in] data     Primary handle or 32-bit argument.
 */
void Fs_Server::HandleCoreCommand(Fs_Thread *pThread, ULONG ncmd, ULONG data)
{
    void *pRecvobj = pThread->GetConn()->GetRecvobj();
    char buf[256];
    int  rc, len;
    int  ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleCoreCommand(ncmd=0x%lx, data=0x%lx, "
                         "idClient=%lu)",
                         (unsigned long)ncmd, (unsigned long)data,
                         (unsigned long)(pThread->GetProcess()
                                         ? pThread->GetProcess()->GetIdClient()
                                         : 0));

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
                           (unsigned long)pThread->GetProcess()->GetIdClient(),
                           buf);
            }
            SERVER_RETURN(ctx, "DB_PRINT");
        }

        case F_CMD_FATAL_COMMON: {
            if (m_pfnRecv) {
                m_pfnRecv(pRecvobj, buf, &len, sizeof(buf));
                F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_FATAL,
                           "FATAL[id=%lu]: %s",
                           (unsigned long)pThread->GetProcess()->GetIdClient(),
                           buf);
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

/*!
 * @brief Main command entry (called from PMTCP handler).
 *
 * Routes CONNECT/RECONNECT directly (they do not require an existing
 * session). All other commands first locate the calling thread's
 * Fs_Thread via m_conns.FindByRecvobj; if not found, replies with
 * (ULONG)-1. Dispatches by CORE / GPI / WIN class.
 *
 * @param [in] pRecvobj   Transport connection handle.
 * @param [in] ncmd       Full wire command code (class + opcode).
 * @param [in] data       Primary handle (HAB/HWND/HPS/idClient/…).
 * @param [in] threadNum  PMTCP thread index (unused).
 */
void Fs_Server::HandleCommand(void *pRecvobj, int ncmd, int data, int threadNum)
{
    int                  ctx;
    Fs_Connection       *pConn;
    Fs_Thread           *pThread;

    (void)threadNum;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleCommand(ncmd=0x%x, data=0x%x, "
                         "recvobj=%p)", ncmd, data, pRecvobj);

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
               "[SERVER] IN  -> recvobj=%p, ncmd=0x%x, data=0x%x",
               pRecvobj, ncmd, data);

    switch (ncmd) {
    case F_CMD_CLIENT_CONNECT:
        HandleClientConnect(pRecvobj, (ULONG)data);
        SERVER_RETURN(ctx, "CONNECT done");
    case F_CMD_CLIENT_RECONNECT:
        HandleClientReconnect(pRecvobj, (ULONG)data);
        SERVER_RETURN(ctx, "RECONNECT done");
    }

    pConn   = m_conns.FindByRecvobj(pRecvobj);
    pThread = pConn ? pConn->GetThread() : NULL;

    if (!pThread) {
        ULONG err = (ULONG)-1;
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[SERVER] ERROR: no session for recvobj=%p, cmd 0x%x, "
                   "returning error", pRecvobj, ncmd);
        if (m_pfnSend) m_pfnSend(pRecvobj, &err, sizeof(err));
        SERVER_RETURN(ctx, "no session");
    }
    pConn->Touch();

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
