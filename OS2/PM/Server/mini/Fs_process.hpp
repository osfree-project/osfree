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

class Fs_ConnectionList;

class Fs_Process {
public:
    Fs_Process();

    BOOL  Init(ULONG idClient);
    void  Shutdown();

    ULONG GetIdClient() const { return m_idClient; }
    BOOL  IsUsed()      const { return m_bUsed; }

    /* Threads (thread+connection pairs). */
    Fs_Thread *AllocThread(Fs_ConnectionList *pConns, void *pRecvobj);
    void       FreeThread(Fs_Thread *pThread);
    Fs_Thread *FindThreadByTid(ULONG tid);
    int        CountActiveThreads() const;

private:
    BOOL          m_bUsed;
    ULONG         m_idClient;
    Fs_ThreadList m_threads;
};

/*! @brief Registry of Fs_Process slots, serialized by an OS/2 mutex. */
class Fs_ProcessList {
public:
    Fs_ProcessList();
    ~Fs_ProcessList();

    Fs_Process *Create(Fs_ConnectionList *pConns, void *pRecvobj,
                       ULONG *pIdClient);

    Fs_Process *FindByIdClient(ULONG idClient);

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
