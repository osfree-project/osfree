/*!
 * @file Fs_queue.hpp
 * @brief Server-side message queue (HMQ backing store).
 *
 * One Fs_Queue per client thread that calls WinCreateMsgQueue. Stores
 * QMSG records in a circular buffer. Thread-safe through an OS/2
 * mutex; all public methods take the lock internally.
 *
 * Semantics follow OS/2 PM:
 *   - WinPostMsg (Add) never blocks and returns FALSE if the queue
 *     is full. Old messages are NOT overwritten.
 *   - WinGetMsg / WinPeekMsg(PM_REMOVE) (Get) extract the first
 *     message in FIFO order.
 *   - WinPeekMsg (Peek) returns a copy of the first message without
 *     removing it.
 *
 * The queue is a plain container. HMQ allocation, per-thread binding
 * and WinDispatchMsg (Program Inversion) live in dispatch_win_mq.cpp.
 */
#ifndef FS_QUEUE_HPP
#define FS_QUEUE_HPP

#include "F_platform.h"

/*! @brief Default queue capacity when WinCreateMsgQueue receives
    cMsg == 0 (recommended by PM; the exact number is system-defined). */
#define F_QUEUE_DEFAULT_SIZE 64

/*!
 * @brief One message queue backing an HMQ.
 *
 * QMSG records are stored in a circular buffer of m_cbQueue slots.
 * m_lStart is the index of the oldest message, m_lLength is the
 * number of messages currently queued.
 */
class Fs_Queue {
public:
    /*!
     * @brief Allocate the queue buffer and mutex.
     * @param [in] lQueuesize  Capacity in messages; <= 0 selects
     *                         F_QUEUE_DEFAULT_SIZE.
     */
    Fs_Queue(LONG lQueuesize);

    /*! @brief Free the queue buffer and close the mutex. */
    ~Fs_Queue();

    /*!
     * @brief Append one message (WinPostMsg).
     *
     * Does not block. Fails if the queue is full; the oldest message
     * is NOT overwritten (PM returns FALSE from WinPostMsg in this
     * case).
     *
     * @param [in] pmsg  Message to copy into the queue.
     * @return TRUE on success, FALSE on full queue or invalid args.
     * @retval TRUE   Message queued.
     * @retval FALSE  pmsg == NULL, queue not allocated, or full.
     */
    BOOL Add(PQMSG pmsg);

    /*!
     * @brief Copy the first message without removing it (WinPeekMsg).
     * @param [out] pmsg  Receives the first message.
     * @return TRUE on success, FALSE if the queue is empty or args invalid.
     * @retval TRUE   Message copied; queue unchanged.
     * @retval FALSE  Empty queue or pmsg == NULL.
     */
    BOOL Peek(PQMSG pmsg);

    /*!
     * @brief Remove and return the first message (WinGetMsg,
     *        WinPeekMsg with PM_REMOVE).
     * @param [out] pmsg  Receives the first message.
     * @return TRUE on success, FALSE if the queue is empty or args invalid.
     * @retval TRUE   Message extracted.
     * @retval FALSE  Empty queue or pmsg == NULL.
     */
    BOOL Get(PQMSG pmsg);

    /*!
     * @brief Number of messages currently queued.
     * @return Message count (0..capacity).
     */
    int QueryStatus();

    /*! @brief Drop all queued messages (does not free the buffer). */
    void Shutdown();

private:
    void Lock();
    void Unlock();

    PQMSG  m_pQueue;    /*!< Circular buffer of m_cbQueue QMSG slots. */
    LONG   m_cbQueue;   /*!< Capacity in messages.                    */
    LONG   m_lStart;    /*!< Index of the oldest message.             */
    LONG   m_lLength;   /*!< Number of queued messages.               */
    HMTX   m_hmtx;      /*!< Registry mutex; NULL if creation failed. */
};

#endif /* FS_QUEUE_HPP */
