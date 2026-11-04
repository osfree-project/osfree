/*!
 * @file Fs_server.hpp
 * @brief Top-level server object: sessions, wire, monitor.
 *
 * Fs_Server owns:
 *   - Fs_ClientList (per-connection sessions)
 *   - wire recv/send function pointers from PMTCP/PMPIPE
 *   - runtime config (Fs_ServerConfig)
 *   - the monitor thread that expires dead sessions and terminates
 *     the process when all sessions are gone.
 *
 * All command dispatch (pm_handler callback) routes through
 * HandleCommand().
 */
#ifndef FS_SERVER_HPP
#define FS_SERVER_HPP

#include <time.h>
#include "F_platform.h"
#include "Fs_config.hpp"
#include "Fs_client.hpp"
#include "F_cmd.h"

/* Wire function types.
 *
 * APIENTRY cannot be used in a function-pointer typedef: on Win32 it
 * expands to __declspec(__pragma("__estdcall")), which Watcom rejects
 * with E671 in this position. Use the plain calling-convention keyword
 * instead. The underlying ABI is identical (stdcall-style on Win32,
 * _System on OS/2), so the pointer is binary-compatible with the
 * PMTCP/PMPIPE exports.
 */
#if defined(__OS2__) || defined(__OS2V2__)
  #define FS_CALLCONV   _System
#elif defined(__WIN32__) || defined(_WIN32)
  #define FS_CALLCONV   __stdcall
#endif

typedef APIRET (FS_CALLCONV *Fs_PfnRecv)(void *pRecvobj, void *pvData,
                                         int *pcbLen, int cbMax);
typedef APIRET (FS_CALLCONV *Fs_PfnSend)(void *pRecvobj, void *pvData,
                                         int cbLen);

/*!
 * @brief Top-level server object. One instance (global) per process.
 */
class Fs_Server {
public:
    Fs_Server();
    ~Fs_Server();

    /*! Install PMTCP/PMPIPE wire functions before StartMonitor. */
    void SetWire(Fs_PfnRecv pfnRecv, Fs_PfnSend pfnSend);

    /*! Install runtime config (defaults are from Fs_ServerConfig). */
    void SetConfig(const Fs_ServerConfig &cfg);

    /*! Spawn the monitor thread. */
    void StartMonitor();

    /*! Main command entry point (called by pm_handler). */
    void HandleCommand(void *pRecvobj, int ncmd, int data, int threadNum);

    /*! @brief Called by PMTCP's per-connection thread when the socket
        is gone (recv failed or returned EOF). Transitions the session
        from ACTIVE to DETACHED so a legitimate RECONNECT can resume it
        without waiting for ulInactive. No reply is sent. */
    void HandleDisconnect(void *pRecvobj);

    /*! Monitor loop body (called by monitor thread). */
    void MonitorLoop();

    /*! Active + DETACHED session count (for tests / logs). */
    int  ActiveCount() const { return m_clients.CountActive(); }

private:
    /* Client lifecycle */
    void HandleClientConnect(void *pRecvobj);
    void HandleClientReconnect(void *pRecvobj, ULONG idClient);
    void HandleClientExit(Fs_ClientConnection *pConn);
    void HandleClientPing(Fs_ClientConnection *pConn);

    /* Dispatch */
    void HandleGpiCall(void *pRecvobj, ULONG fnCode, HPS hps);
    void HandleCoreCommand(Fs_ClientConnection *pConn, int ncmd, int data);

    /* Response helpers */
    void SendClientConfig(void *pRecvobj, Fs_ClientConnection *pConn);
    void SendHeader(void *pRecvobj, ULONG ulStatus, ULONG ulResult,
                    ULONG cbPayload, ULONG ulLastError);

    /* Thread entry wrapper */
    static void MonitorThreadFn(void *param);

    Fs_ClientList    m_clients;
    Fs_ServerConfig  m_cfg;
    Fs_PfnRecv       m_pfnRecv;
    Fs_PfnSend       m_pfnSend;
    volatile BOOL    m_fShutdown;
    volatile BOOL    m_fEverHadClient;
};

#endif /* FS_SERVER_HPP */
