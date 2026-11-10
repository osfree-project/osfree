/*!
 * @file Fs_process.hpp
 * @brief One client process (= one idClient) + the process registry.
 */
#ifndef FS_PROCESS_HPP
#define FS_PROCESS_HPP

#include <time.h>
#include "F_platform.h"
#include "Fs_config.hpp"
#include "Fs_thread.hpp"
#include "Fs_hab.hpp"

class Fs_ConnectionList;

class Fs_Process {
public:
    Fs_Process();

    BOOL  Init(ULONG idClient);
    void  Shutdown();

    ULONG GetIdClient() const { return m_idClient; }
    BOOL  IsUsed()      const { return m_bUsed; }

    /* Threads (pairs thread+connection) */
    Fs_Thread *AllocThread(Fs_ConnectionList *pConns, void *pRecvobj);
    void       FreeThread(Fs_Thread *pThread);
    Fs_Thread *FindThreadByConn(void *pRecvobj);
    Fs_Thread *FindThreadByTid(ULONG tid);
    int        CountActiveThreads() const;

    /* Habs (still local to the process; will move to Fs_HabList in step 2) */
    Fs_Hab *AttachHab(Fs_Thread *pThread, ULONG tid, ULONG *pIHab);
    BOOL    DetachHab(ULONG iHab);
    Fs_Hab *FindHabByIHab(ULONG iHab);
    void    DestroyAllHabs();

private:
    BOOL        m_bUsed;
    ULONG       m_idClient;
    Fs_ThreadList m_threads;
    Fs_Hab      m_aHabs[F_MAX_HABS];
    ULONG       m_ulNextHab;
};

/*! @brief Registry of Fs_Process slots, serialized by an OS/2 mutex. */
class Fs_ProcessList {
public:
    Fs_ProcessList();
    ~Fs_ProcessList();

    Fs_Process          *Create(Fs_ConnectionList *pConns, void *pRecvobj,
                                ULONG *pIdClient);

    Fs_Process          *FindByIdClient(ULONG idClient);
    Fs_Process          *FindByConn(void *pRecvobj);
    Fs_Thread           *FindThreadByConn(void *pRecvobj);
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
