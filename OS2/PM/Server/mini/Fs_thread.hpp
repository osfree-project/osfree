/*!
 * @file Fs_thread.hpp
 * @brief One client thread + the per-process thread registry.
 *
 * Fs_Thread mirrors a client-side OS thread: it holds the client's TID
 * (reported in NWinInitialize), the HAB id currently attached to it,
 * and two back-refs — to the owning Fs_Process and to this thread's
 * Fs_Connection (which lives in Fs_Server::m_conns, not here).
 *
 * Fs_ThreadList holds F_MAX_THREADS slots inside one Fs_Process. It
 * does not own connections: Alloc/Free coordinate with a caller-supplied
 * Fs_ConnectionList so the thread<->connection pair stays in sync.
 */
#ifndef FS_THREAD_HPP
#define FS_THREAD_HPP

#include "F_platform.h"
#include "Fs_config.hpp"
#include "Fs_connection.hpp"

class Fs_Process;

class Fs_Thread {
public:
    Fs_Thread()
        : m_bUsed(FALSE), m_tid(0), m_iHab(0),
          m_pProcess(NULL), m_pConn(NULL) {}

    void Init(Fs_Process *pProcess, Fs_Connection *pConn)
    {
        m_bUsed    = TRUE;
        m_tid      = 0;
        m_iHab     = 0;
        m_pProcess = pProcess;
        m_pConn    = pConn;
    }

    void Shutdown()
    {
        m_bUsed    = FALSE;
        m_tid      = 0;
        m_iHab     = 0;
        m_pProcess = NULL;
        m_pConn    = NULL;
    }

    void AttachHab(ULONG tid, ULONG iHab) { m_tid = tid; m_iHab = iHab; }
    void DetachHab()                      { m_tid = 0;   m_iHab = 0;   }

    BOOL  IsUsed() const { return m_bUsed; }
    ULONG GetTid() const { return m_tid; }
    ULONG GetIHab() const { return m_iHab; }

    Fs_Process    *GetProcess() const { return m_pProcess; }
    Fs_Connection *GetConn()    const { return m_pConn; }

    void SetProcess(Fs_Process *pProcess) { m_pProcess = pProcess; }
    void SetConn(Fs_Connection *pConn)    { m_pConn = pConn; }

private:
    BOOL           m_bUsed;
    ULONG          m_tid;
    ULONG          m_iHab;
    Fs_Process    *m_pProcess;
    Fs_Connection *m_pConn;
};

class Fs_ThreadList {
public:
    Fs_ThreadList() {}

    Fs_Thread *Alloc(Fs_Process *pProcess, Fs_Connection *pConn)
    {
        int i;
        for (i = 0; i < F_MAX_THREADS; i++) {
            if (!m_aThreads[i].IsUsed()) {
                m_aThreads[i].Init(pProcess, pConn);
                return &m_aThreads[i];
            }
        }
        return NULL;
    }

    void Shutdown()
    {
        int i;
        for (i = 0; i < F_MAX_THREADS; i++) {
            if (m_aThreads[i].IsUsed()) {
                Fs_Connection *pConn = m_aThreads[i].GetConn();
                if (pConn) pConn->Shutdown();
                m_aThreads[i].Shutdown();
            }
        }
    }

    Fs_Thread *FindByTid(ULONG tid)
    {
        int i;
        if (tid == 0) return NULL;
        for (i = 0; i < F_MAX_THREADS; i++) {
            if (m_aThreads[i].IsUsed() && m_aThreads[i].GetTid() == tid)
                return &m_aThreads[i];
        }
        return NULL;
    }

    Fs_Thread *FindByConn(void *pRecvobj)
    {
        int i;
        if (!pRecvobj) return NULL;
        for (i = 0; i < F_MAX_THREADS; i++) {
            if (m_aThreads[i].IsUsed()) {
                Fs_Connection *pConn = m_aThreads[i].GetConn();
                if (pConn && pConn->GetRecvobj() == pRecvobj)
                    return &m_aThreads[i];
            }
        }
        return NULL;
    }

    int CountActive() const
    {
        int i, n = 0;
        for (i = 0; i < F_MAX_THREADS; i++)
            if (m_aThreads[i].IsUsed()) n++;
        return n;
    }

private:
    Fs_Thread m_aThreads[F_MAX_THREADS];
};

#endif /* FS_THREAD_HPP */
