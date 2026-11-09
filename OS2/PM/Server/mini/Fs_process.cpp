/*!
 * @file Fs_process.cpp
 * @brief Fs_Process and Fs_ProcessList implementation.
 */
#include <time.h>
#include "Fs_process.hpp"
#include "F_debug.h"

#define GPI_SERVER
#include "gpi_ps.h"   /* GpiPsDestroyByHab */

/* ======================================================================== */
/*  Fs_Process                                                              */
/* ======================================================================== */

Fs_Process::Fs_Process()
    : m_bUsed(FALSE), m_idClient(0), m_ulNextHab(1)
{
}

BOOL Fs_Process::Init(ULONG idClient)
{
    if (m_bUsed)       return FALSE;
    if (idClient == 0) return FALSE;
    m_bUsed     = TRUE;
    m_idClient  = idClient;
    m_ulNextHab = 1;
    return TRUE;
}

void Fs_Process::Shutdown()
{
    int i;

    if (!m_bUsed) return;

    for (i = 0; i < F_MAX_THREADS; i++) {
        if (!m_aThreads[i].IsFree())
            m_aThreads[i].Shutdown();
    }
    DestroyAllHabs();

    m_bUsed     = FALSE;
    m_idClient  = 0;
    m_ulNextHab = 1;
}

Fs_ClientConnection *Fs_Process::AllocThread(void *pRecvobj)
{
    int i;

    if (!m_bUsed)         return NULL;
    if (pRecvobj == NULL) return NULL;

    for (i = 0; i < F_MAX_THREADS; i++) {
        if (m_aThreads[i].IsFree()) {
            if (m_aThreads[i].Init(pRecvobj, m_idClient))
                return &m_aThreads[i];
            return NULL;
        }
    }
    return NULL;
}

Fs_ClientConnection *Fs_Process::FindThreadByConn(void *pRecvobj)
{
    int i;

    if (pRecvobj == NULL) return NULL;
    for (i = 0; i < F_MAX_THREADS; i++) {
        if (!m_aThreads[i].IsFree() &&
            m_aThreads[i].GetRecvobj() == pRecvobj)
            return &m_aThreads[i];
    }
    return NULL;
}

Fs_ClientConnection *Fs_Process::FindThreadByTid(ULONG tid)
{
    int i;

    if (tid == 0) return NULL;
    for (i = 0; i < F_MAX_THREADS; i++) {
        if (!m_aThreads[i].IsFree() &&
            m_aThreads[i].GetTid() == tid)
            return &m_aThreads[i];
    }
    return NULL;
}

int Fs_Process::CountActiveThreads() const
{
    int i, n = 0;
    for (i = 0; i < F_MAX_THREADS; i++)
        if (!m_aThreads[i].IsFree()) n++;
    return n;
}

Fs_Hab *Fs_Process::AttachHab(Fs_ClientConnection *pThread, ULONG tid,
                              ULONG *pIHab)
{
    int     i;
    ULONG   iHab;
    Fs_Hab *pHab = NULL;

    if (pIHab) *pIHab = 0;
    if (!m_bUsed)        return NULL;
    if (pThread == NULL) return NULL;
    if (tid == 0)        return NULL;
    if (pThread->GetIHab() != 0) return NULL;   /* already attached */

    for (i = 0; i < F_MAX_HABS; i++) {
        if (!m_aHabs[i].IsUsed()) {
            iHab = m_ulNextHab++;
            if (iHab == 0) iHab = m_ulNextHab++;
            if (m_aHabs[i].Init(m_idClient, tid, iHab)) {
                pHab = &m_aHabs[i];
                pThread->AttachHab(tid, iHab);
                if (pIHab) *pIHab = iHab;
                break;
            }
        }
    }
    return pHab;
}

BOOL Fs_Process::DetachHab(ULONG iHab)
{
    int     i;
    BOOL    found = FALSE;

    if (!m_bUsed)  return FALSE;
    if (iHab == 0) return FALSE;

    for (i = 0; i < F_MAX_HABS; i++) {
        if (m_aHabs[i].IsUsed() && m_aHabs[i].GetIHab() == iHab) {
            ULONG tid = m_aHabs[i].GetTid();
            m_aHabs[i].Shutdown();
            {
                Fs_ClientConnection *pThread = FindThreadByTid(tid);
                if (pThread) pThread->DetachHab();
            }
            found = TRUE;
            break;
        }
    }

    if (found) GpiPsDestroyByHab(iHab);
    return found;
}

Fs_Hab *Fs_Process::FindHabByIHab(ULONG iHab)
{
    int i;
    if (iHab == 0) return NULL;
    for (i = 0; i < F_MAX_HABS; i++) {
        if (m_aHabs[i].IsUsed() && m_aHabs[i].GetIHab() == iHab)
            return &m_aHabs[i];
    }
    return NULL;
}

void Fs_Process::DestroyAllHabs()
{
    int i;
    for (i = 0; i < F_MAX_HABS; i++) {
        if (m_aHabs[i].IsUsed()) {
            ULONG iHab = m_aHabs[i].GetIHab();
            m_aHabs[i].Shutdown();
            GpiPsDestroyByHab(iHab);
        }
    }
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
    if (m_hmtx) {
        DosCloseMutexSem(m_hmtx);
        m_hmtx = NULL;
    }
}

void Fs_ProcessList::Lock()
{
    if (m_hmtx) DosRequestMutexSem(m_hmtx, SEM_INDEFINITE_WAIT);
}

void Fs_ProcessList::Unlock()
{
    if (m_hmtx) DosReleaseMutexSem(m_hmtx);
}

Fs_Process *Fs_ProcessList::Create(void *pRecvobj, ULONG *pIdClient)
{
    Fs_Process *pProc = NULL;
    ULONG       idNew = 0;
    int         i;
    int         nUsed = 0;

    if (pIdClient) *pIdClient = 0;
    if (pRecvobj == NULL) return NULL;

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

    if (pProc && pProc->AllocThread(pRecvobj) == NULL) {
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

Fs_Process *Fs_ProcessList::FindByConn(void *pRecvobj)
{
    int i;
    Fs_Process *pFound = NULL;
    if (pRecvobj == NULL) return NULL;
    Lock();
    for (i = 0; i < F_MAX_CLIENTS; i++) {
        if (m_aProcs[i].IsUsed() &&
            m_aProcs[i].FindThreadByConn(pRecvobj) != NULL) {
            pFound = &m_aProcs[i];
            break;
        }
    }
    Unlock();
    return pFound;
}

Fs_ClientConnection *Fs_ProcessList::FindThreadByConn(void *pRecvobj)
{
    int i;
    Fs_ClientConnection *pFound = NULL;
    if (pRecvobj == NULL) return NULL;
    Lock();
    for (i = 0; i < F_MAX_CLIENTS; i++) {
        if (m_aProcs[i].IsUsed()) {
            Fs_ClientConnection *pThread =
                m_aProcs[i].FindThreadByConn(pRecvobj);
            if (pThread) { pFound = pThread; break; }
        }
    }
    Unlock();
    return pFound;
}

Fs_Hab *Fs_ProcessList::FindHabByIHab(ULONG iHab)
{
    int i;
    Fs_Hab *pFound = NULL;
    if (iHab == 0) return NULL;
    Lock();
    for (i = 0; i < F_MAX_CLIENTS; i++) {
        if (m_aProcs[i].IsUsed()) {
            Fs_Hab *pHab = m_aProcs[i].FindHabByIHab(iHab);
            if (pHab) { pFound = pHab; break; }
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
    /* Timeouts belong to the reconnect/grace phase; not needed for the
       session-level MVP. Kept as a no-op to satisfy the Fs_Server
       MonitorLoop contract. */
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
