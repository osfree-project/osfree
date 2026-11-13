/*!
 * @file Fs_queue.hpp
 * @brief Server-side message queue (HMQ backing store).
 *
 * One Fs_Queue per HAB that called WinCreateMsgQueue. The queue is
 * owned by the HAB (in PM the AAB holds a pointer to the MQ, and the
 * MQ carries a back-reference to its owner); Fs_Queue keeps both the
 * owning iHab and the assigned HMQ so NWinDestroyMsgQueue and future
 * NWinGetMsg callers can validate ownership.
 *
 * Semantics follow OS/2 PM version 3 or higher:
 *   - the lQueuesize argument to WinCreateMsgQueue is ignored and the
 *     queue grows dynamically (PMERR_HEAP_OUT_OF_MEMORY is the only
 *     full-queue condition);
 *   - WinPostMsg (Add) never blocks;
 *   - WinGetMsg (Get) blocks the calling thread until a message is
 *     available;
 *   - WinPeekMsg (Peek) never blocks.
 *
 * Thread safety is provided by an internal OS/2 mutex; the blocking
 * Get uses an OS/2 event semaphore to wait for producers.
 */
#ifndef FS_QUEUE_HPP
#define FS_QUEUE_HPP

#define INCL_BASE
#define INCL_PM
#include "F_platform.h"

/*! @brief Initial capacity in messages; the buffer doubles on Grow. */
#define F_QUEUE_INITIAL_SIZE 10

/*!
 * @brief One message queue backing an HMQ.
 *
 * Stores QMSG records in a linear buffer; the oldest message is at
 * index 0, and removing it shifts the tail down (memmove). This is
 * O(n) per removal but n is small: a message queue holds messages for
 * one thread, not a general work queue.
 */
class Fs_Queue {
public:
    /*!
     * @brief Allocate the buffer, mutex and event semaphore.
     *
     * If any of the three OS/2 resources fails to allocate, the
     * corresponding member is left NULL and the affected operations
     * return FALSE; the object remains safe to pass to
     * Add/Peek/Get/QueryStatus/Shutdown and to delete.
     *
     * @param [in] iHab  Owning anchor block id (from Fs_Hab::GetIHab).
     * @param [in] hmq   Server-assigned HMQ; stored for later
     *                   ownership checks.
     * @return Nothing (constructor).
     */
    Fs_Queue(ULONG iHab, ULONG hmq);

    /*! @brief Free the buffer, close the mutex and the semaphore. */
    ~Fs_Queue();

    /*!
     * @brief Append one message (WinPostMsg).
     *
     * Never blocks. Grows the internal buffer if needed. Returns
     * FALSE only when the message cannot be copied (NULL argument,
     * queue resources not allocated, or out of memory during Grow,
     * which corresponds to PMERR_HEAP_OUT_OF_MEMORY).
     *
     * @param [in] pmsg  Message to copy into the queue.
     * @return TRUE on success, FALSE on failure.
     * @retval TRUE   Message queued; a waiting Get is woken.
     * @retval FALSE  pmsg == NULL, queue resources lost, or OOM.
     */
    BOOL Add(PQMSG pmsg);

    /*!
     * @brief Copy the oldest message without removing it (WinPeekMsg).
     * @param [out] pmsg  Receives the oldest message.
     * @return TRUE on success, FALSE on empty queue or bad argument.
     * @retval TRUE   Message copied; queue unchanged.
     * @retval FALSE  Queue empty or pmsg == NULL.
     */
    BOOL Peek(PQMSG pmsg);

    /*!
     * @brief Remove and return the oldest message (WinGetMsg).
     *
     * Blocks the calling thread until a message is available. The
     * consumer resets the event semaphore under the lock when the
     * last message is drained; a producer's PostEventSem after the
     * matching Unlock cannot be lost because it happens after this
     * reset.
     *
     * @param [out] pmsg  Receives the oldest message.
     * @return TRUE on success, FALSE if pmsg == NULL or the wait
     *         failed (event semaphore not allocated).
     * @retval TRUE   Message extracted.
     * @retval FALSE  pmsg == NULL or no event semaphore.
     */
    BOOL Get(PQMSG pmsg);

    /*!
     * @brief Number of messages currently queued
     *        (WinQueryQueueStatus).
     * @return Message count in the range 0..current capacity.
     */
    int QueryStatus();

    /*! @brief Drop all queued messages; the buffer is not freed. */
    void Shutdown();

    /*! @brief Owning HAB id (set at construction, never changes). */
    ULONG GetHab() const { return m_hHab; }
    /*! @brief HMQ assigned to this queue (set at construction). */
    ULONG GetHmq() const { return m_hmq;  }

private:
    /*! @brief Grow the buffer; returns FALSE on OOM. */
    BOOL Grow();
    /*! @brief Acquire m_hmtx (no-op if not created). */
    void Lock();
    /*! @brief Release m_hmtx (no-op if not created). */
    void Unlock();

    ULONG  m_hHab;      /*!< Owning HAB; immutable.                       */
    ULONG  m_hmq;       /*!< HMQ assigned to this queue; immutable.       */
    PQMSG  m_pQueue;    /*!< Linear buffer of QMSG slots.                 */
    LONG   m_cbQueue;   /*!< Capacity in messages.                        */
    LONG   m_lLength;   /*!< Number of queued messages.                   */
    HMTX   m_hmtx;      /*!< Queue mutex; NULL if creation failed.        */
    HEV    m_hevMsg;    /*!< Message-available event sem; NULL on failure.*/
};

#endif /* FS_QUEUE_HPP */
