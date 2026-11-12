/*!
 * @file Fs_queue.cpp
 * @brief Fs_Queue implementation.
 */
#include <string.h>

#include "Fs_queue.hpp"
#include "F_debug.h"

/*!
 * @brief Allocate the buffer, mutex and event semaphore.
 * @param [in] iHab  Owning anchor block id.
 * @param [in] hmq   Server-assigned HMQ.
 * @return Nothing (constructor).
 */
Fs_Queue::Fs_Queue(ULONG iHab, ULONG hmq)
    : m_hHab(iHab), m_hmq(hmq),
      m_pQueue(NULL), m_cbQueue(0), m_lLength(0),
      m_hmtx(NULL), m_hevMsg(NULL)
{
    if (DosCreateMutexSem(NULL, &m_hmtx, 0, FALSE) != 0) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[QUEUE] ERROR: DosCreateMutexSem failed");
        m_hmtx = NULL;
    }
    if (DosCreateEventSem(NULL, &m_hevMsg, 0, FALSE) != 0) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[QUEUE] ERROR: DosCreateEventSem failed");
        m_hevMsg = NULL;
    }
}

/*!
 * @brief Free the buffer, close the mutex and the semaphore.
 * @return Nothing (destructor).
 */
Fs_Queue::~Fs_Queue()
{
    if (m_pQueue) { DosFreeMem(m_pQueue);       m_pQueue = NULL; }
    if (m_hmtx)   { DosCloseMutexSem(m_hmtx);   m_hmtx   = NULL; }
    if (m_hevMsg) { DosCloseEventSem(m_hevMsg); m_hevMsg = NULL; }
    m_cbQueue = 0;
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
 * @brief Grow the buffer; returns FALSE on OOM.
 * @return TRUE on success, FALSE on allocation failure.
 * @retval TRUE   Buffer reallocated (or already large enough).
 * @retval FALSE  DosAllocMem failed.
 */
BOOL Fs_Queue::Grow()
{
    PVOID pvNew = NULL;
    LONG  cbNew = m_cbQueue ? m_cbQueue * 2 : F_QUEUE_INITIAL_SIZE;

    if (DosAllocMem(&pvNew, (ULONG)cbNew * (ULONG)sizeof(QMSG),
                    PAG_COMMIT | PAG_READ | PAG_WRITE) != 0)
        return FALSE;
    if (m_pQueue && m_lLength > 0)
        memcpy(pvNew, m_pQueue, (ULONG)m_lLength * (ULONG)sizeof(QMSG));
    if (m_pQueue) DosFreeMem(m_pQueue);
    m_pQueue  = (PQMSG)pvNew;
    m_cbQueue = cbNew;
    return TRUE;
}

/*!
 * @brief Append one message (WinPostMsg).
 * @param [in] pmsg  Message to copy into the queue.
 * @return TRUE on success, FALSE on failure.
 * @retval TRUE   Message queued; a waiting Get is woken.
 * @retval FALSE  pmsg == NULL, queue resources lost, or OOM.
 */
BOOL Fs_Queue::Add(PQMSG pmsg)
{
    BOOL ok = FALSE;

    if (!pmsg) return FALSE;

    Lock();
    if (m_lLength >= m_cbQueue) {
        if (!Grow()) {
            Unlock();
            return FALSE;
        }
    }
    if (m_pQueue) {
        m_pQueue[m_lLength++] = *pmsg;
        ok = TRUE;
    }
    Unlock();

    if (ok && m_hevMsg) {
        /* Signal after Unlock: the message is already visible to a
         * consumer that wakes and re-checks under the lock. */
        DosPostEventSem(m_hevMsg);
    }

    return ok;
}

/*!
 * @brief Copy the oldest message without removing it (WinPeekMsg).
 * @param [out] pmsg  Receives the oldest message.
 * @return TRUE on success, FALSE on empty queue or bad argument.
 * @retval TRUE   Message copied.
 * @retval FALSE  Queue empty or pmsg == NULL.
 */
BOOL Fs_Queue::Peek(PQMSG pmsg)
{
    BOOL ok = FALSE;

    if (!pmsg) return FALSE;

    Lock();
    if (m_pQueue && m_lLength > 0) {
        *pmsg = m_pQueue[0];
        ok = TRUE;
    }
    Unlock();

    return ok;
}

/*!
 * @brief Remove and return the oldest message (WinGetMsg).
 *
 * Blocks until a message is available. The consumer resets the
 * event semaphore under the lock when the last message is drained;
 * a producer's PostEventSem after the matching Unlock cannot be lost
 * because it happens after this reset.
 *
 * @param [out] pmsg  Receives the oldest message.
 * @return TRUE on success, FALSE on failure.
 * @retval TRUE   Message extracted.
 * @retval FALSE  pmsg == NULL or event semaphore not available.
 */
BOOL Fs_Queue::Get(PQMSG pmsg)
{
    if (!pmsg) return FALSE;

    for (;;) {
        Lock();
        if (m_pQueue && m_lLength > 0) {
            *pmsg = m_pQueue[0];
            if (m_lLength > 1)
                memmove(&m_pQueue[0], &m_pQueue[1],
                        (ULONG)(m_lLength - 1) * (ULONG)sizeof(QMSG));
            m_lLength--;
            if (m_lLength == 0 && m_hevMsg)
                DosResetEventSem(m_hevMsg, NULL);
            Unlock();
            return TRUE;
        }
        Unlock();

        if (!m_hevMsg) return FALSE;

        if (DosWaitEventSem(m_hevMsg, SEM_INDEFINITE_WAIT) != 0)
            return FALSE;
    }
}

/*!
 * @brief Number of messages currently queued
 *        (WinQueryQueueStatus).
 * @return Message count in the range 0..m_cbQueue.
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
 * @brief Drop all queued messages (WinDestroyMsgQueue pre-step).
 *
 * Does not free the buffer; the buffer is released by the destructor.
 */
void Fs_Queue::Shutdown()
{
    Lock();
    m_lLength = 0;
    if (m_hevMsg) DosResetEventSem(m_hevMsg, NULL);
    Unlock();
}
