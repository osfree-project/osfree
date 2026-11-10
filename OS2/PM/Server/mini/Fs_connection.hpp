/*!
 * @file Fs_connection.hpp
 * @brief One transport connection + the server-wide connection registry.
 *
 * Fs_Connection is the server-side mirror of a PMCLIENT per-thread
 * transport slot: it owns the recvobj (F_Link*), tracks Active/Detached
 * state and last-activity / grace timestamps. A Free slot has no
 * recvobj. Back-ref to the owning Fs_Thread (m_pThread) lets dispatchers
 * go recvobj -> Fs_Connection -> Fs_Thread -> Fs_Process without any
 * secondary lookup.
 *
 * Fs_ConnectionList holds all connections (F_MAX_CLIENTS slots), keyed
 * by recvobj. Owned by Fs_Server (peer to Fs_ProcessList and Fs_HabList).
 */
#ifndef FS_CONNECTION_HPP
#define FS_CONNECTION_HPP

#include <time.h>
#include "F_platform.h"
#include "Fs_config.hpp"

class Fs_Thread;

class Fs_Connection {
public:
    enum State {
        StateFree     = 0,
        StateActive   = 1,
        StateDetached = 2
    };

    Fs_Connection()
        : m_state(StateFree), m_pRecvobj(NULL),
          m_timeLastActivity(0), m_timeDetached(0),
          m_pThread(NULL) {}

    void Attach(void *pRecvobj)
    {
        m_state            = StateActive;
        m_pRecvobj         = pRecvobj;
        m_timeLastActivity = time(NULL);
        m_timeDetached     = 0;
    }

    void Detach(time_t now)
    {
        if (m_state != StateActive) return;
        m_state        = StateDetached;
        m_pRecvobj     = NULL;
        m_timeDetached = now;
    }

    void Shutdown()
    {
        m_state            = StateFree;
        m_pRecvobj         = NULL;
        m_timeLastActivity = 0;
        m_timeDetached     = 0;
        m_pThread          = NULL;
    }

    void Touch()
    {
        if (m_state == StateActive) m_timeLastActivity = time(NULL);
    }

    BOOL IsFree()     const { return m_state == StateFree; }
    BOOL IsActive()   const { return m_state == StateActive; }
    BOOL IsDetached() const { return m_state == StateDetached; }

    BOOL IsExpiredInactive(time_t now, ULONG ulTimeout) const
    {
        if (m_state != StateActive) return FALSE;
        if (ulTimeout == 0)         return FALSE;
        return (now - m_timeLastActivity) > (time_t)ulTimeout;
    }

    BOOL IsExpiredGrace(time_t now, ULONG ulTimeout) const
    {
        if (m_state != StateDetached) return FALSE;
        if (ulTimeout == 0)           return FALSE;
        return (now - m_timeDetached) > (time_t)ulTimeout;
    }

    void  *GetRecvobj()       const { return m_pRecvobj; }
    time_t GetLastActivity()  const { return m_timeLastActivity; }
    time_t GetTimeDetached()  const { return m_timeDetached; }
    State  GetState()         const { return m_state; }

    void       SetThread(Fs_Thread *pThread) { m_pThread = pThread; }
    Fs_Thread *GetThread()                   const { return m_pThread; }

private:
    State      m_state;
    void      *m_pRecvobj;
    time_t     m_timeLastActivity;
    time_t     m_timeDetached;
    Fs_Thread *m_pThread;
};

class Fs_ConnectionList {
public:
    Fs_ConnectionList() : m_hmtx(NULL)
    {
        if (DosCreateMutexSem(NULL, &m_hmtx, 0, FALSE) != 0)
            m_hmtx = NULL;
    }
    ~Fs_ConnectionList()
    {
        if (m_hmtx) { DosCloseMutexSem(m_hmtx); m_hmtx = NULL; }
    }

    Fs_Connection *Alloc(void *pRecvobj)
    {
        int i;
        if (!pRecvobj) return NULL;
        Lock();
        for (i = 0; i < F_MAX_CLIENTS; i++) {
            if (m_aConns[i].IsFree()) {
                m_aConns[i].Attach(pRecvobj);
                Unlock();
                return &m_aConns[i];
            }
        }
        Unlock();
        return NULL;
    }

    Fs_Connection *FindByRecvobj(void *pRecvobj)
    {
        int i;
        if (!pRecvobj) return NULL;
        for (i = 0; i < F_MAX_CLIENTS; i++) {
            if (m_aConns[i].IsActive() &&
                m_aConns[i].GetRecvobj() == pRecvobj)
                return &m_aConns[i];
        }
        return NULL;
    }

    int CountActive() const
    {
        int i, n = 0;
        for (i = 0; i < F_MAX_CLIENTS; i++)
            if (m_aConns[i].IsActive()) n++;
        return n;
    }

private:
    void Lock()   { if (m_hmtx) DosRequestMutexSem(m_hmtx, SEM_INDEFINITE_WAIT); }
    void Unlock() { if (m_hmtx) DosReleaseMutexSem(m_hmtx); }

    Fs_Connection m_aConns[F_MAX_CLIENTS];
    HMTX          m_hmtx;
};

#endif /* FS_CONNECTION_HPP */
