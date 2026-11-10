/*!
 * @file Fs_server.hpp
 * @brief Top-level server object: processes, connections, HABs, wire, monitor.
 */
#ifndef FS_SERVER_HPP
#define FS_SERVER_HPP

#include <time.h>
#include "F_platform.h"
#include "Fs_config.hpp"
#include "Fs_process.hpp"
#include "Fs_connection.hpp"
#include "Fs_thread.hpp"
#include "Fs_hab.hpp"
#include "F_cmd.h"
#include "win_wire.h"

#if defined(__OS2__) || defined(__OS2V2__)
  #define FS_CALLCONV   _System
#elif defined(__WIN32__) || defined(_WIN32)
  #define FS_CALLCONV   __stdcall
#endif

/*! @brief Transport recv callback: read pvData up to cbMax bytes,
    store actual length in *pcbLen. */
typedef APIRET (FS_CALLCONV *Fs_PfnRecv)(void *pRecvobj, void *pvData,
                                         int *pcbLen, int cbMax);
/*! @brief Transport send callback: write cbLen bytes from pvData. */
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

    Fs_ProcessList    *GetProcs() { return &m_procs; }
    Fs_ConnectionList *GetConns() { return &m_conns; }
    Fs_HabList        *GetHabs()  { return &m_habs;  }

private:
    /* ---- Client lifecycle ---- */
    void HandleClientConnect(void *pRecvobj, ULONG idClient);
    void HandleClientReconnect(void *pRecvobj, ULONG idClient);
    void HandleClientExit(Fs_Thread *pThread);
    void HandleClientPing(Fs_Thread *pThread, ULONG idClient);

    /* ---- Top-level dispatch by class ---- */
    void HandleGpiCall(Fs_Thread *pThread, ULONG fnCode, ULONG data);
    void HandleWinCall(Fs_Thread *pThread, ULONG fnCode, ULONG data);
    void HandleCoreCommand(Fs_Thread *pThread, ULONG ncmd, ULONG data);

    /* ---- Per-class dispatchers (uniform signature) ---- */
    ULONG DispatchGpi     (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinHab  (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinMq   (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinWnd  (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinPs   (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinCls  (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinDlg  (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinPtr  (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinClip (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinAtom (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinStr  (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinErr  (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinSys  (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinRect (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinInp  (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinDraw (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinFrame(void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinSync (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinMisc (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWin32   (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
    ULONG DispatchWinDax  (void *pRecvobj, ULONG fnCode, ULONG data,
                           PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);

    /* ---- Response helpers ---- */
    void SendClientConfig(void *pRecvobj, Fs_Thread *pThread);
    void SendHeader(void *pRecvobj, ULONG ulStatus, ULONG ulResult,
                    ULONG cbPayload, ULONG ulLastError);

    /* Thread entry wrapper */
    static void MonitorThreadFn(void *param);

    Fs_PfnRecv        m_pfnRecv;
    Fs_PfnSend        m_pfnSend;
    volatile BOOL     m_fShutdown;
    volatile BOOL     m_fEverHadClient;
    F_ServerConfig    m_cfg;
    Fs_ProcessList    m_procs;
    Fs_ConnectionList m_conns;
    Fs_HabList        m_habs;
};

#endif /* FS_SERVER_HPP */
