/*!
 * @file Fs_process.cpp
 * @brief Fs_Process and Fs_ProcessList implementation.
 */
#include <time.h>
#include "Fs_process.hpp"
#include "Fs_connection.hpp"
#include "F_debug.h"

/* ======================================================================== */
/*  Fs_Process                                                              */
/* ======================================================================== */

Fs_Process::Fs_Process()
    : m_bUsed(FALSE), m_idClient(0)
{
}

BOOL Fs_Process::Init(ULONG idClient)
{
    if (m_bUsed)       return FALSE;
    if (idClient == 0) return FALSE;
    m_bUsed    = TRUE;
    m_idClient = idClient;
    return TRUE;
}

void Fs_Process::Shutdown()
{
    if (!m_bUsed) return;
    m_threads.Shutdown();
    m_bUsed    = FALSE;
    m_idClient = 0;
}

Fs_Thread *Fs_Process::AllocThread(Fs_ConnectionList *pConns, void *pRecvobj)
{
    Fs_Connection *pConn;
    Fs_Thread     *pThread;

    if (!m_bUsed || !pConns || !pRecvobj) return NULL;

    pConn = pConns->Alloc(pRecvobj);
    if (!pConn) return NULL;

    pThread = m_threads.Alloc(this, pConn);
    if (!pThread) { pConn->Shutdown(); return NULL; }

    pConn->SetThread(pThread);
    return pThread;
}

void Fs_Process::FreeThread(Fs_Thread *pThread)
{
    Fs_Connection *pConn;
    if (!pThread) return;
    pConn = pThread->GetConn();
    if (pConn) pConn->Shutdown();
    pThread->Shutdown();
}

Fs_Thread *Fs_Process::FindThreadByTid(ULONG tid)
{
    return m_threads.FindByTid(tid);
}

int Fs_Process::CountActiveThreads() const
{
    return m_threads.CountActive();
}

/* ======================================================================== */
/*  Fs_ProcessList                                                          */
/* ======================================================================== */

Fs_ProcessList::Fs_ProcessList()
    : m_ulNextId(1)
{
    m_hmtx = NULL;
    if (DosCreateMutexSem(NULL, &m_hmtx, 0, FALSE) != 0) {
        F_DbgPrint(F_DBG_SEC_FS_CLIENT, F_DBG_LVL_ERROR,
                   "[PROCLIST] ERROR: DosCreateMutexSem failed");
        m_hmtx = NULL;
    }
}

Fs_ProcessList::~Fs_ProcessList()
{
    if (m_hmtx) { DosCloseMutexSem(m_hmtx); m_hmtx = NULL; }
}

void Fs_ProcessList::Lock()
{
    if (m_hmtx) DosRequestMutexSem(m_hmtx, SEM_INDEFINITE_WAIT);
}

void Fs_ProcessList::Unlock()
{
    if (m_hmtx) DosReleaseMutexSem(m_hmtx);
}

Fs_Process *Fs_ProcessList::Create(Fs_ConnectionList *pConns,
                                    void *pRecvobj, ULONG *pIdClient)
{
    Fs_Process *pProc = NULL;
    ULONG       idNew = 0;
    int         i;
    int         nUsed = 0;

    if (pIdClient) *pIdClient = 0;
    if (pRecvobj == NULL) return NULL;
    if (pConns   == NULL) return NULL;

    Lock();
    for (i = 0; i < F_MAX_CLIENTS; i++)
        if (m_aProcs[i].IsUsed()) nUsed++;

    if (nUsed >= F_MAX_CLIENTS) {
        Unlock();
        F_DbgPrint(F_DBG_SEC_FS_CLIENT, F_DBG_LVL_ERROR,
                   "[PROCLIST] limit reached (%d/%d)", nUsed, F_MAX_CLIENTS);
        return NULL;
    }

    for (i = 0; i < F_MAX_CLIENTS; i++) {
        if (!m_aProcs[i].IsUsed()) {
            idNew = m_ulNextId++;
            if (idNew == 0) idNew = m_ulNextId++;
            if (m_aProcs[i].Init(idNew)) {
                pProc = &m_aProcs[i];
                if (pIdClient) *pIdClient = idNew;
            }
            break;
        }
    }

    if (pProc && pProc->AllocThread(pConns, pRecvobj) == NULL) {
        pProc->Shutdown();
        pProc = NULL;
        if (pIdClient) *pIdClient = 0;
    }

    Unlock();
    return pProc;
}

Fs_Process *Fs_ProcessList::FindByIdClient(ULONG idClient)
{
    int i;
    Fs_Process *pFound = NULL;
    if (idClient == 0) return NULL;
    Lock();
    for (i = 0; i < F_MAX_CLIENTS; i++) {
        if (m_aProcs[i].IsUsed() && m_aProcs[i].GetIdClient() == idClient) {
            pFound = &m_aProcs[i];
            break;
        }
    }
    Unlock();
    return pFound;
}

void Fs_ProcessList::Destroy(ULONG idClient)
{
    int i;
    Fs_Process *pFound = NULL;
    if (idClient == 0) return;

    Lock();
    for (i = 0; i < F_MAX_CLIENTS; i++) {
        if (m_aProcs[i].IsUsed() && m_aProcs[i].GetIdClient() == idClient) {
            pFound = &m_aProcs[i];
            break;
        }
    }
    Unlock();

    if (pFound) pFound->Shutdown();

    F_DbgPrint(F_DBG_SEC_FS_CLIENT, F_DBG_LVL_VERBOSE,
               "[PROCLIST] Destroy idClient=%lu", (unsigned long)idClient);
}

int Fs_ProcessList::Expire(time_t now, ULONG ulInactive, ULONG ulGrace)
{
    (void)now; (void)ulInactive; (void)ulGrace;
    return 0;
}

int Fs_ProcessList::CountActive() const
{
    int i, n = 0;
    for (i = 0; i < F_MAX_CLIENTS; i++)
        if (m_aProcs[i].IsUsed()) n++;
    return n;
}
