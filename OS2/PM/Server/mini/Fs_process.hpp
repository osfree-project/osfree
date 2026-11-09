/*!
 * @file Fs_process.hpp
 * @brief Per-process container: threads + Habs + registry.
 *
 * Mirrors the client side: PMCLIENT keeps g_idClient (process-wide)
 * and aConns[tid] (per-thread). The server mirrors this as Fs_Process
 * (idClient) containing one Fs_ClientConnection per client thread
 * (keyed by recvobj) and one Fs_Hab per WinInitialize.
 */
#ifndef FS_PROCESS_HPP
#define FS_PROCESS_HPP

#include <time.h>
#include "F_platform.h"
#include "Fs_config.hpp"
#include "Fs_client.hpp"
#include "Fs_hab.hpp"

/*! @brief One client process (one idClient). */
class Fs_Process {
public:
    Fs_Process();

    /*! @brief Install into a FREE slot. */
    BOOL  Init(ULONG idClient);

    /*! @brief Release all threads and Habs. */
    void  Shutdown();

    ULONG GetIdClient() const { return m_idClient; }
    BOOL  IsUsed()      const { return m_bUsed; }

    /* ---- Threads (one Fs_ClientConnection per recvobj) ---- */
    Fs_ClientConnection *AllocThread(void *pRecvobj);
    Fs_ClientConnection *FindThreadByConn(void *pRecvobj);
    Fs_ClientConnection *FindThreadByTid(ULONG tid);
    int  CountActiveThreads() const;

    /* ---- Habs (one Fs_Hab per WinInitialize) ---- */
    Fs_Hab *AttachHab(Fs_ClientConnection *pThread, ULONG tid, ULONG *pIHab);
    BOOL    DetachHab(ULONG iHab);
    Fs_Hab *FindHabByIHab(ULONG iHab);
    void    DestroyAllHabs();

private:
    BOOL                m_bUsed;
    ULONG               m_idClient;
    Fs_ClientConnection m_aThreads[F_MAX_THREADS];
    Fs_Hab              m_aHabs[F_MAX_HABS];
    ULONG               m_ulNextHab;
};

/*! @brief Registry of Fs_Process slots, serialized by an OS/2 mutex. */
class Fs_ProcessList {
public:
    Fs_ProcessList();
    ~Fs_ProcessList();

    /*! @brief Create a new process and allocate its first thread. */
    Fs_Process          *Create(void *pRecvobj, ULONG *pIdClient);

    Fs_Process          *FindByIdClient(ULONG idClient);
    Fs_Process          *FindByConn(void *pRecvobj);
    Fs_ClientConnection *FindThreadByConn(void *pRecvobj);
    Fs_Hab              *FindHabByIHab(ULONG iHab);

    void  Destroy(ULONG idClient);
    int   Expire(time_t now, ULONG ulInactive, ULONG ulGrace);
    int   CountActive() const;

private:
    void  Lock();
    void  Unlock();

    Fs_Process m_aProcs[F_MAX_CLIENTS];
    ULONG      m_ulNextId;
    HMTX       m_hmtx;
};

#endif /* FS_PROCESS_HPP */
