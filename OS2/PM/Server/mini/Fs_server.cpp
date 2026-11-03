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

extern void log(const char *fmt, ...);

/* DispatchGpi is declared in dispatch.cpp. */
extern ULONG DispatchGpi(HPS hps, ULONG ulFnCode, PBYTE pArgs, ULONG cbArgs,
                         PULONG pcbPayload);

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
    log("[SERVER] starting monitor thread (tick=%lu ms)\n",
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
            log("[SERVER] all sessions gone, shutting down\n");
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

    log("[SERVER] sent F_CLIENTCONFIG to %p (idClient=%lu)\n",
        pRecvobj, (unsigned long)cfg.idClient);
}

/* ======================================================================== */
/*  Client lifecycle                                                        */
/* ======================================================================== */

void Fs_Server::HandleClientConnect(void *pRecvobj)
{
    ULONG idNew = 0;
    Fs_ClientConnection *pConn = m_clients.GetOrCreate(pRecvobj, &idNew);

    if (!pConn) {
        log("[SERVER] CONNECT: table full, rejecting %p\n", pRecvobj);
        SendClientConfig(pRecvobj, NULL);   /* idClient=0 signals error */
        return;
    }

    if (idNew != 0) {
        log("[SERVER] CONNECT: new session idClient=%lu recvobj=%p\n",
            (unsigned long)idNew, pRecvobj);
        m_fEverHadClient = TRUE;
    } else {
        log("[SERVER] CONNECT: existing session idClient=%lu recvobj=%p\n",
            (unsigned long)pConn->GetIdClient(), pRecvobj);
    }
    pConn->Touch();
    SendClientConfig(pRecvobj, pConn);
}

void Fs_Server::HandleClientReconnect(void *pRecvobj, ULONG idClient)
{
    Fs_ClientConnection *pConn;

    log("[SERVER] RECONNECT: idClient=%lu recvobj=%p\n",
        (unsigned long)idClient, pRecvobj);

    if (idClient == 0) {
        SendClientConfig(pRecvobj, NULL);
        return;
    }

    pConn = m_clients.FindByIdClient(idClient);
    if (!pConn) {
        log("[SERVER] RECONNECT: idClient=%lu not found, rejecting\n",
            (unsigned long)idClient);
        SendClientConfig(pRecvobj, NULL);
        return;
    }

    if (pConn->IsActive() && pConn->GetRecvobj() != pRecvobj) {
        /* Old connection still alive. In a proper system we'd have to
           decide: refuse the new one, or kick the old one. For now,
           refuse the new one — the old client may still be live. */
        log("[SERVER] RECONNECT: idClient=%lu still ACTIVE on old recvobj=%p, "
            "refusing new recvobj=%p\n",
            (unsigned long)idClient, pConn->GetRecvobj(), pRecvobj);
        SendClientConfig(pRecvobj, NULL);
        return;
    }

    pConn->ToActive(pRecvobj, time(NULL));
    m_fEverHadClient = TRUE;
    SendClientConfig(pRecvobj, pConn);
}

void Fs_Server::HandleClientPing(Fs_ClientConnection *pConn)
{
    if (!pConn) return;
    pConn->Touch();   /* already done in HandleCommand, but safe */
}

void Fs_Server::HandleClientExit(Fs_ClientConnection *pConn)
{
    if (!pConn) return;
    log("[SERVER] EXIT: idClient=%lu\n", (unsigned long)pConn->GetIdClient());
    pConn->Shutdown();
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

    if (!m_pfnRecv) return;

    rc = m_pfnRecv(pRecvobj, &cbArgs, &len, sizeof(cbArgs));
    if (rc == 0 && len == (int)sizeof(cbArgs)) {
        if (cbArgs > 0) {
            if (DosAllocMem(&pv, cbArgs,
                            PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
                SendHeader(pRecvobj, 1, GPI_ERROR, 0, 0);
                return;
            }
            pArgs = (PBYTE)pv;
            rc = m_pfnRecv(pRecvobj, pArgs, &len, cbArgs);
        }

        ULONG ulResult = DispatchGpi(hps, fnCode, pArgs, cbArgs, &cbPayload);
        SendHeader(pRecvobj, 0, ulResult, cbPayload, 0);
        if (cbPayload > 0 && pArgs != NULL && cbPayload <= cbArgs && m_pfnSend)
            m_pfnSend(pRecvobj, pArgs, (int)cbPayload);

        if (pv) DosFreeMem(pv);
    } else {
        SendHeader(pRecvobj, 1, GPI_ERROR, 0, 0);
    }
}

void Fs_Server::HandleCoreCommand(Fs_ClientConnection *pConn, int ncmd, int data)
{
    void *pRecvobj = pConn->GetRecvobj();
    char buf[256];
    int  rc, len;

    switch (ncmd) {

        case F_CMD_GET_IHAB: {
            /* Stub for HAB step; returns {1, idClient} for compatibility. */
            int inf[2];
            inf[0] = 1;
            inf[1] = (int)pConn->GetIdClient();
            if (m_pfnSend) m_pfnSend(pRecvobj, inf, sizeof(inf));
            break;
        }

        case F_CMD_GET_HPS: {
            HPS hps = (HPS)1;
            log("[SERVER] GET_HPS(hwnd=0x%x) -> hps=%p (stub)\n",
                data, (void*)hps);
            if (m_pfnSend) m_pfnSend(pRecvobj, &hps, sizeof(HPS));
            break;
        }

        case F_CMD_RELEASE_HPS: {
            int rc1 = 0;
            log("[SERVER] RELEASE_HPS(hps=0x%x) -> 0\n", data);
            if (m_pfnSend) m_pfnSend(pRecvobj, &rc1, sizeof(int));
            break;
        }

        case F_CMD_DB_PRINT: {
            if (m_pfnRecv && m_cfg.fAcceptLogs) {
                m_pfnRecv(pRecvobj, buf, &len, sizeof(buf));
                log("DEBUG[id=%lu]: %s\n",
                    (unsigned long)pConn->GetIdClient(), buf);
            }
            break;
        }

        case F_CMD_FATAL_COMMON: {
            if (m_pfnRecv) {
                m_pfnRecv(pRecvobj, buf, &len, sizeof(buf));
                log("FATAL[id=%lu]: %s\n",
                    (unsigned long)pConn->GetIdClient(), buf);
            }
            break;
        }

        default: {
            unsigned long reply = (unsigned long)ncmd;
            if (m_pfnSend) m_pfnSend(pRecvobj, &reply, sizeof(reply));
            break;
        }
    }
}

/* ======================================================================== */
/*  Main command entry                                                      */
/* ======================================================================== */

void Fs_Server::HandleCommand(void *pRecvobj, int ncmd, int data, int threadNum)
{
    (void)threadNum;

    log("[SERVER] IN  -> recvobj=%p, ncmd=0x%x, data=0x%x\n",
        pRecvobj, ncmd, data);

    /* Lifecycle commands (do not require an existing session for CONNECT) */
    switch (ncmd) {
    case F_CMD_CLIENT_CONNECT:
        HandleClientConnect(pRecvobj);
        return;
    case F_CMD_CLIENT_RECONNECT:
        HandleClientReconnect(pRecvobj, (ULONG)data);
        return;
    }

    /* All other commands need an ACTIVE session. Fallback: auto-create
       (backward-compatible with a client that does not yet send CONNECT). */
    Fs_ClientConnection *pConn = m_clients.FindByRecvobj(pRecvobj);
    if (!pConn) {
        ULONG idNew = 0;
        pConn = m_clients.GetOrCreate(pRecvobj, &idNew);
        if (!pConn) {
            log("[SERVER] ERROR: no session for recvobj=%p, cmd 0x%x dropped\n",
                pRecvobj, ncmd);
            return;
        }
        if (idNew != 0) {
            log("[SERVER] auto-created session idClient=%lu (no CONNECT)\n",
                (unsigned long)idNew);
            m_fEverHadClient = TRUE;
        }
    }
    pConn->Touch();

    if (ncmd == F_CMD_CLIENT_EXIT) {
        HandleClientExit(pConn);
        return;
    }
    if (ncmd == F_CMD_CLIENT_PING) {
        HandleClientPing(pConn);
        return;
    }
    if (F_CMD_GET_CLASS((ULONG)ncmd) == F_CMD_CLASS_GPI) {
        HandleGpiCall(pRecvobj, F_CMD_GET_OPCODE((ULONG)ncmd), (HPS)data);
        return;
    }
    HandleCoreCommand(pConn, ncmd, data);
}
