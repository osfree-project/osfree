/*!
 * @file Fs_queue.cpp
 * @brief Fs_Queue implementation.
 */
#include <string.h>

#include "Fs_queue.hpp"
#include "F_debug.h"

/*!
 * @brief Allocate the queue buffer and mutex.
 *
 * If DosAllocMem or DosCreateMutexSem fails, the object is left in a
 * degenerate but safe state (m_pQueue == NULL or m_hmtx == NULL):
 * Add/Peek/Get return FALSE, Lock/Unlock become no-ops, the
 * destructor is a no-op.
 *
 * @param [in] lQueuesize  Capacity in messages; <= 0 selects
 *                         F_QUEUE_DEFAULT_SIZE.
 * @return Nothing (constructor).
 */
Fs_Queue::Fs_Queue(LONG lQueuesize)
    : m_pQueue(NULL), m_cbQueue(0), m_lStart(0), m_lLength(0), m_hmtx(NULL)
{
    PVOID pv = NULL;

    if (lQueuesize <= 0) lQueuesize = F_QUEUE_DEFAULT_SIZE;

    if (DosAllocMem(&pv,
                    (ULONG)lQueuesize * (ULONG)sizeof(QMSG),
                    PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[QUEUE] ERROR: DosAllocMem(%ld QMSG) failed",
                   (long)lQueuesize);
        return;
    }
    m_pQueue  = (PQMSG)pv;
    m_cbQueue = lQueuesize;

    if (DosCreateMutexSem(NULL, &m_hmtx, 0, FALSE) != 0) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[QUEUE] ERROR: DosCreateMutexSem failed");
        m_hmtx = NULL;
    }
}

/*!
 * @brief Free the queue buffer and close the mutex.
 * @return Nothing (destructor).
 */
Fs_Queue::~Fs_Queue()
{
    if (m_pQueue) {
        DosFreeMem(m_pQueue);
        m_pQueue  = NULL;
        m_cbQueue = 0;
    }
    if (m_hmtx) {
        DosCloseMutexSem(m_hmtx);
        m_hmtx = NULL;
    }
    m_lStart  = 0;
    m_lLength = 0;
}

/*!
 * @brief Acquire the queue mutex (no-op if it was not created).
 */
void Fs_Queue::Lock()
{
    if (m_hmtx) DosRequestMutexSem(m_hmtx, SEM_INDEFINITE_WAIT);
}

/*!
 * @brief Release the queue mutex (no-op if it was not created).
 */
void Fs_Queue::Unlock()
{
    if (m_hmtx) DosReleaseMutexSem(m_hmtx);
}

/*!
 * @brief Append one message (WinPostMsg).
 *
 * @param [in] pmsg  Message to copy into the queue.
 * @return TRUE on success, FALSE on full queue or invalid args.
 * @retval TRUE   Message queued.
 * @retval FALSE  pmsg == NULL, queue not allocated, or full.
 */
BOOL Fs_Queue::Add(PQMSG pmsg)
{
    BOOL ok = FALSE;

    if (!pmsg || !m_pQueue || m_cbQueue == 0) return FALSE;

    Lock();
    if (m_lLength < m_cbQueue) {
        LONG ind = (m_lStart + m_lLength) % m_cbQueue;
        m_pQueue[ind] = *pmsg;
        m_lLength++;
        ok = TRUE;
    }
    Unlock();

    return ok;
}

/*!
 * @brief Copy the first message without removing it (WinPeekMsg).
 *
 * @param [out] pmsg  Receives the first message.
 * @return TRUE on success, FALSE if the queue is empty or args invalid.
 * @retval TRUE   Message copied; queue unchanged.
 * @retval FALSE  Empty queue or pmsg == NULL.
 */
BOOL Fs_Queue::Peek(PQMSG pmsg)
{
    BOOL ok = FALSE;

    if (!pmsg || !m_pQueue || m_cbQueue == 0) return FALSE;

    Lock();
    if (m_lLength > 0) {
        *pmsg = m_pQueue[m_lStart];
        ok = TRUE;
    }
    Unlock();

    return ok;
}

/*!
 * @brief Remove and return the first message (WinGetMsg,
 *        WinPeekMsg with PM_REMOVE).
 *
 * @param [out] pmsg  Receives the first message.
 * @return TRUE on success, FALSE if the queue is empty or args invalid.
 * @retval TRUE   Message extracted.
 * @retval FALSE  Empty queue or pmsg == NULL.
 */
BOOL Fs_Queue::Get(PQMSG pmsg)
{
    BOOL ok = FALSE;

    if (!pmsg || !m_pQueue || m_cbQueue == 0) return FALSE;

    Lock();
    if (m_lLength > 0) {
        *pmsg    = m_pQueue[m_lStart];
        m_lStart = (m_lStart + 1) % m_cbQueue;
        m_lLength--;
        ok = TRUE;
    }
    Unlock();

    return ok;
}

/*!
 * @brief Number of messages currently queued.
 * @return Message count (0..capacity).
 */
int Fs_Queue::QueryStatus()
{
    int n;

    Lock();
    n = (int)m_lLength;
    Unlock();

    return n;
}

/*!
 * @brief Drop all queued messages (does not free the buffer).
 */
void Fs_Queue::Shutdown()
{
    Lock();
    m_lStart  = 0;
    m_lLength = 0;
    Unlock();
}
