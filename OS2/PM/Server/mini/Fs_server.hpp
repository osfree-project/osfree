/*!
 * @file Fs_server.hpp
 * @brief Top-level server object: processes, wire, monitor.
 */
#ifndef FS_SERVER_HPP
#define FS_SERVER_HPP

#include <time.h>
#include "F_platform.h"
#include "Fs_config.hpp"
#include "Fs_process.hpp"
#include "F_cmd.h"

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

    void SetWire(Fs_PfnRecv pfnRecv, Fs_PfnSend pfnSend);
    void SetConfig(const F_ServerConfig &cfg);
    void StartMonitor();
    void HandleCommand(void *pRecvobj, int ncmd, int data, int threadNum);
    void HandleDisconnect(void *pRecvobj);
    void MonitorLoop();
    int  ActiveCount() const { return m_procs.CountActive(); }

private:
    /* Client lifecycle */
    void HandleClientConnect(void *pRecvobj, ULONG idClient);
    void HandleClientReconnect(void *pRecvobj, ULONG idClient);
    void HandleClientExit(Fs_ClientConnection *pThread);
    void HandleClientPing(Fs_ClientConnection *pThread, ULONG idClient);

    /* Dispatch */
    void HandleGpiCall(Fs_ClientConnection *pThread, ULONG fnCode, ULONG data);
    void HandleWinCall(Fs_ClientConnection *pThread, ULONG fnCode, ULONG data);
    void HandleCoreCommand(Fs_ClientConnection *pThread, ULONG ncmd, ULONG data);

    /* Response helpers */
    void SendClientConfig(void *pRecvobj, Fs_ClientConnection *pThread);
    void SendHeader(void *pRecvobj, ULONG ulStatus, ULONG ulResult,
                    ULONG cbPayload, ULONG ulLastError);

    /* Thread entry wrapper */
    static void MonitorThreadFn(void *param);

    Fs_PfnRecv        m_pfnRecv;
    Fs_PfnSend        m_pfnSend;
    volatile BOOL     m_fShutdown;
    volatile BOOL     m_fEverHadClient;
    F_ServerConfig   m_cfg;
    Fs_ProcessList    m_procs;
};

#endif /* FS_SERVER_HPP */
