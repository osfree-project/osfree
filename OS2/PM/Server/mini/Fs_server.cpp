/*!
 * @file Fs_server.cpp
 * @brief Fs_Server implementation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if defined(__OS2__) || defined(__OS2V2__)
  #include <process.h>       /* _beginthread */
#endif

#include "Fs_server.hpp"
#include "F_debug.h"

/* DispatchGpi is declared in dispatch.cpp. */
extern ULONG DispatchGpi(HPS hps, ULONG ulFnCode, PBYTE pArgs, ULONG cbArgs,
                         PULONG pcbPayload);

/*!
 * @def SERVER_RETURN
 * @brief Close the caller's debug context and return from the function.
 *
 * @param [in] ctx  Context level returned by F_DbgCtxEnterf.
 * @param [in] fmt  printf-style format for the exit reason.
 * @param [in] ...  Format arguments.
 */
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

void Fs_Server::SetConfig(const Fs_ServerConfig &cfg)
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
        m_clients.Expire(now, m_cfg.ulInactive, m_cfg.ulGrace);

        int nActive = m_clients.CountActive();

        /* Exit only after we've had at least one client and the count
           has dropped to zero. This lets the server sit idle before
           any client connects, and cleanly shuts down after the last
           client disconnects or times out. */
        if (m_fEverHadClient && nActive == 0) {
            F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_INFO,
                       "[SERVER] all sessions gone, shutting down");
            m_fShutdown = TRUE;
            /* Freeze state, then exit. exit(0) runs atexit handlers
               (closes the server presence mutex) and terminates any
               remaining PMTCP threads via the OS. */
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

void Fs_Server::SendClientConfig(void *pRecvobj, Fs_ClientConnection *pConn)
{
    F_CLIENTCONFIG cfg;

    cfg.idClient    = pConn ? pConn->GetIdClient() : 0;
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

void Fs_Server::HandleClientConnect(void *pRecvobj)
{
    int ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleClientConnect(recvobj=%p)", pRecvobj);

    /* Strict CONNECT: a recvobj may carry at most one session. If it
       already has an Active session, reject. A client that lost its
       transport must use RECONNECT (after the session transitions to
       DETACHED via the disconnect notification), not a second CONNECT
       on the same connection. */
    Fs_ClientConnection *pExisting = m_clients.FindByRecvobj(pRecvobj);
    if (pExisting) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_WARNING,
                   "[SERVER] CONNECT: recvobj=%p already has session "
                   "idClient=%lu, rejecting",
                   pRecvobj, (unsigned long)pExisting->GetIdClient());
        SendClientConfig(pRecvobj, NULL);   /* idClient=0 */
        SERVER_RETURN(ctx, "rejected (already has session idClient=%lu)",
                      (unsigned long)pExisting->GetIdClient());
    }

    ULONG idNew = 0;
    Fs_ClientConnection *pConn = m_clients.GetOrCreate(pRecvobj, &idNew);
    if (!pConn) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[SERVER] CONNECT: table full, rejecting %p", pRecvobj);
        SendClientConfig(pRecvobj, NULL);
        SERVER_RETURN(ctx, "rejected (table full)");
    }
    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_INFO,
               "[SERVER] CONNECT: new session idClient=%lu recvobj=%p",
               (unsigned long)idNew, pRecvobj);
    m_fEverHadClient = TRUE;
    pConn->Touch();
    SendClientConfig(pRecvobj, pConn);
    SERVER_RETURN(ctx, "new idClient=%lu", (unsigned long)idNew);
}

void Fs_Server::HandleClientReconnect(void *pRecvobj, ULONG idClient)
{
    int ctx;
    Fs_ClientConnection *pConn;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleClientReconnect(idClient=%lu, recvobj=%p)",
                         (unsigned long)idClient, pRecvobj);

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_INFO,
               "[SERVER] RECONNECT: idClient=%lu recvobj=%p",
               (unsigned long)idClient, pRecvobj);

    if (idClient == 0) {
        SendClientConfig(pRecvobj, NULL);
        SERVER_RETURN(ctx, "rejected (idClient=0)");
    }

    pConn = m_clients.FindByIdClient(idClient);
    if (!pConn) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_WARNING,
                   "[SERVER] RECONNECT: idClient=%lu not found, rejecting",
                   (unsigned long)idClient);
        SendClientConfig(pRecvobj, NULL);
        SERVER_RETURN(ctx, "rejected (not found)");
    }

    if (pConn->IsActive()) {
        /* Session is still ACTIVE. RECONNECT is only valid for a
           DETACHED session (after the old socket is known dead). A
           live session must not be taken over. */
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_WARNING,
                   "[SERVER] RECONNECT: idClient=%lu still ACTIVE, rejecting",
                   (unsigned long)idClient);
        SendClientConfig(pRecvobj, NULL);
        SERVER_RETURN(ctx, "rejected (still ACTIVE)");
    }

    pConn->ToActive(pRecvobj, time(NULL));
    m_fEverHadClient = TRUE;
    SendClientConfig(pRecvobj, pConn);
    SERVER_RETURN(ctx, "resumed idClient=%lu", (unsigned long)idClient);
}

void Fs_Server::HandleClientPing(Fs_ClientConnection *pConn)
{
    int ctx;

    if (!pConn) return;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleClientPing(idClient=%lu)",
                         (unsigned long)pConn->GetIdClient());
    pConn->Touch();   /* already done in HandleCommand, but safe */
    SERVER_RETURN(ctx, "done");
}

void Fs_Server::HandleClientExit(Fs_ClientConnection *pConn)
{
    int ctx;

    if (!pConn) return;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleClientExit(idClient=%lu)",
                         (unsigned long)pConn->GetIdClient());
    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_INFO,
               "[SERVER] EXIT: idClient=%lu",
               (unsigned long)pConn->GetIdClient());
    pConn->Shutdown();
    SERVER_RETURN(ctx, "done");
}

void Fs_Server::HandleDisconnect(void *pRecvobj)
{
    int ctx;
    Fs_ClientConnection *pConn;

    pConn = m_clients.FindByRecvobj(pRecvobj);

    if (!pConn) return;
    if (!pConn->IsActive()) return;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleDisconnect(recvobj=%p, idClient=%lu)",
                         pRecvobj, (unsigned long)pConn->GetIdClient());

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_INFO,
               "[SERVER] DISCONNECT: idClient=%lu recvobj=%p -> DETACHED",
               (unsigned long)pConn->GetIdClient(), pRecvobj);
    pConn->ToDetached(time(NULL));
    SERVER_RETURN(ctx, "-> DETACHED");
}

/* ======================================================================== */
/*  GPI dispatch                                                            */
/* ======================================================================== */

void Fs_Server::HandleGpiCall(void *pRecvobj, ULONG fnCode, HPS hps)
{
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

void Fs_Server::HandleCoreCommand(Fs_ClientConnection *pConn, int ncmd, int data)
{
    void *pRecvobj = pConn->GetRecvobj();
    char buf[256];
    int  rc, len;
    int  ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleCoreCommand(ncmd=0x%x, data=0x%x, idClient=%lu)",
                         ncmd, data, (unsigned long)pConn->GetIdClient());

    switch (ncmd) {

        case F_CMD_GET_IHAB: {
            /* Stub for HAB step; returns {1, idClient} for compatibility. */
            int inf[2];
            inf[0] = 1;
            inf[1] = (int)pConn->GetIdClient();
            if (m_pfnSend) m_pfnSend(pRecvobj, inf, sizeof(inf));
            SERVER_RETURN(ctx, "GET_IHAB -> {1,%lu}",
                          (unsigned long)pConn->GetIdClient());
        }

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
                           (unsigned long)pConn->GetIdClient(), buf);
            }
            SERVER_RETURN(ctx, "DB_PRINT");
        }

        case F_CMD_FATAL_COMMON: {
            if (m_pfnRecv) {
                m_pfnRecv(pRecvobj, buf, &len, sizeof(buf));
                F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_FATAL,
                           "FATAL[id=%lu]: %s",
                           (unsigned long)pConn->GetIdClient(), buf);
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
    Fs_ClientConnection *pConn;

    (void)threadNum;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[SERVER] HandleCommand(ncmd=0x%x, data=0x%x, recvobj=%p)",
                         ncmd, data, pRecvobj);

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
               "[SERVER] IN  -> recvobj=%p, ncmd=0x%x, data=0x%x",
               pRecvobj, ncmd, data);

    /* Lifecycle commands (do not require an existing session) */
    switch (ncmd) {
    case F_CMD_CLIENT_CONNECT:
        HandleClientConnect(pRecvobj);
        SERVER_RETURN(ctx, "CONNECT done");
    case F_CMD_CLIENT_RECONNECT:
        HandleClientReconnect(pRecvobj, (ULONG)data);
        SERVER_RETURN(ctx, "RECONNECT done");
    }

    pConn = m_clients.FindByRecvobj(pRecvobj);

    /* Every remaining command requires an existing session. */
    if (!pConn) {
        ULONG err = (ULONG)-1;
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[SERVER] ERROR: no session for recvobj=%p, cmd 0x%x, "
                   "returning error", pRecvobj, ncmd);
        if (m_pfnSend) m_pfnSend(pRecvobj, &err, sizeof(err));
        SERVER_RETURN(ctx, "no session");
    }
    pConn->Touch();

    if (ncmd == F_CMD_CLIENT_EXIT) {
        HandleClientExit(pConn);
        SERVER_RETURN(ctx, "EXIT done");
    }
    if (ncmd == F_CMD_CLIENT_PING) {
        HandleClientPing(pConn);
        SERVER_RETURN(ctx, "PING done");
    }
    if (F_CMD_GET_CLASS((ULONG)ncmd) == F_CMD_CLASS_GPI) {
        HandleGpiCall(pRecvobj, F_CMD_GET_OPCODE((ULONG)ncmd), (HPS)data);
        SERVER_RETURN(ctx, "GPI call done");
    }
    HandleCoreCommand(pConn, ncmd, data);
    SERVER_RETURN(ctx, "CORE command done");
}

#undef SERVER_RETURN
