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

/*!
 * @brief Default constructor. Slot starts unused, idClient zeroed.
 * @return Nothing (constructor).
 */
Fs_Process::Fs_Process()
    : m_bUsed(FALSE), m_idClient(0)
{
}

/*!
 * @brief Install a new process into a FREE slot.
 *
 * @param [in] idClient  Monotonic client id (non-zero).
 * @return TRUE on success, FALSE if slot used or idClient == 0.
 * @retval TRUE   Slot populated.
 * @retval FALSE  Slot already used or idClient == 0.
 */
BOOL Fs_Process::Init(ULONG idClient)
{
    if (m_bUsed)       return FALSE;
    if (idClient == 0) return FALSE;
    m_bUsed    = TRUE;
    m_idClient = idClient;
    return TRUE;
}

/*!
 * @brief Release all threads of this process and free the slot.
 */
void Fs_Process::Shutdown()
{
    if (!m_bUsed) return;
    m_threads.Shutdown();
    m_bUsed    = FALSE;
    m_idClient = 0;
}

/*!
 * @brief Allocate a thread slot + connection for the given recvobj.
 *
 * Pairs Fs_Thread with Fs_Connection: the connection is allocated in
 * the caller-supplied Fs_ConnectionList, then the thread slot is
 * allocated in m_threads, and the two back-refs are cross-linked.
 *
 * @param [in] pConns    Server-wide connection registry.
 * @param [in] pRecvobj  Transport handle of the calling thread.
 * @return Thread slot, or NULL on failure.
 * @retval !NULL  Thread + connection allocated.
 * @retval NULL   Process unused, args invalid, or table full.
 */
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

/*!
 * @brief Free a thread slot and its connection.
 *
 * @param [in] pThread  Thread slot to free (no-op if NULL).
 */
void Fs_Process::FreeThread(Fs_Thread *pThread)
{
    Fs_Connection *pConn;
    if (!pThread) return;
    pConn = pThread->GetConn();
    if (pConn) pConn->Shutdown();
    pThread->Shutdown();
}

/*!
 * @brief Find a thread slot by client TID.
 *
 * @param [in] tid  Client thread id.
 * @return Thread slot, or NULL if not found.
 * @retval !NULL  Matching slot.
 * @retval NULL   No such tid.
 */
Fs_Thread *Fs_Process::FindThreadByTid(ULONG tid)
{
    return m_threads.FindByTid(tid);
}

/*!
 * @brief Count active thread slots of this process.
 *
 * @return Number of used slots.
 * @retval >=0  Count.
 */
int Fs_Process::CountActiveThreads() const
{
    return m_threads.CountActive();
}

/* ======================================================================== */
/*  Fs_ProcessList                                                          */
/* ======================================================================== */

/*!
 * @brief Create the process registry and its OS/2 mutex.
 *
 * The monotonic idClient counter starts at 1 (0 = invalid idClient).
 * On mutex creation failure m_hmtx is left NULL and Lock/Unlock become
 * no-ops.
 *
 * @return Nothing (constructor).
 */
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

/*!
 * @brief Close the registry mutex.
 * @return Nothing (destructor).
 */
Fs_ProcessList::~Fs_ProcessList()
{
    if (m_hmtx) { DosCloseMutexSem(m_hmtx); m_hmtx = NULL; }
}

/*!
 * @brief Acquire the registry mutex (no-op if it was not created).
 */
void Fs_ProcessList::Lock()
{
    if (m_hmtx) DosRequestMutexSem(m_hmtx, SEM_INDEFINITE_WAIT);
}

/*!
 * @brief Release the registry mutex (no-op if it was not created).
 */
void Fs_ProcessList::Unlock()
{
    if (m_hmtx) DosReleaseMutexSem(m_hmtx);
}

/*!
 * @brief Create a new process and allocate its first thread.
 *
 * @param [in]  pConns     Server-wide connection registry.
 * @param [in]  pRecvobj   Transport handle of the first thread.
 * @param [out] pIdClient  Receives the assigned idClient.
 * @return Process slot, or NULL on failure.
 * @retval !NULL  Process + first thread allocated.
 * @retval NULL   Args invalid, table full, or thread/conn slot full.
 */
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

/*!
 * @brief Find a process by idClient.
 *
 * @param [in] idClient  Client id (non-zero).
 * @return Process slot, or NULL if not found.
 * @retval !NULL  Matching slot.
 * @retval NULL   No such idClient.
 */
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

/*!
 * @brief Destroy a process slot (all threads released by Shutdown).
 *
 * @param [in] idClient  Client id (0 = no-op).
 */
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

/*!
 * @brief Count active process slots.
 *
 * @return Number of used slots.
 * @retval >=0  Count.
 */
int Fs_ProcessList::CountActive() const
{
    int i, n = 0;
    for (i = 0; i < F_MAX_CLIENTS; i++)
        if (m_aProcs[i].IsUsed()) n++;
    return n;
}
