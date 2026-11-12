/*!
 * @file Fs_hab.hpp
 * @brief Per-thread anchor blocks (HABs) and their container.
 *
 * One Fs_Hab per WinInitialize() call on the client. Keyed by a
 * monotonically increasing m_iHab (never reused); linked to the
 * owning session through m_idClient. HABs own per-thread state
 * (last error, message queue) but do NOT own PS, WND
 * or DC — those live in their own global registries and only
 * back-reference the HAB via m_hHab.
 *
 * On WinTerminate(iHab) / CLIENT_EXIT / grace expiry, the HAB is
 * destroyed and its per-thread resources are released. PS and WND
 * with m_hHab == iHab are force-destroyed too (robustness against
 * clients that crash without calling GpiDestroyPS).
 *
 * All public methods take the internal mutex; callers do not need
 * Lock/Unlock unless they need a multi-call critical section
 * (not required by the current Fs_Server).
 */
#ifndef FS_HAB_HPP
#define FS_HAB_HPP

#define INCL_PM
#include "F_platform.h"
#include "Fs_config.hpp"
#include "Fs_queue.hpp"

/* Phase 4 forward declaration. Not implemented yet. */
class Fs_ClassInfo;

/*!
 * @brief One per-thread anchor block slot.
 *
 * Keyed by the pair (@c m_idClient, @c m_tid): @c m_idClient identifies
 * the session (process), @c m_tid identifies the client thread within
 * that process. TIDs are unique only within a process, so both parts
 * are required to distinguish two HABs that happen to share a @c m_tid
 * in different sessions.
 */
class Fs_Hab {
public:
    Fs_Hab();

    /*!
     * @brief Install a new HAB into a FREE slot.
     * @param [in] idClient  Owning session.
     * @param [in] tid       Client thread id (unique within the process;
     *                       must be non-zero).
     * @param [in] iHab      Server-assigned HAB id.
     * @return TRUE on success, FALSE if the slot is used or any
     *         argument is zero.
     */
    BOOL  Init(ULONG idClient, ULONG tid, ULONG iHab);

    /*! @brief Release the slot (state -> FREE, all fields cleared). */
    void  Shutdown();

    /* ---- Accessors ---- */
    BOOL   IsUsed()       const { return m_bUsed; }
    ULONG  GetIHab()      const { return m_iHab; }
    ULONG  GetIdClient()  const { return m_idClient; }
    ULONG  GetTid()       const { return m_tid; }
    int    GetLastError() const { return m_lastError; }

    /*! @brief Set the per-thread last error (PMERR_*). */
    void   SetLastError(int err) { m_lastError = err; }

    /*!
     * @brief Owning message queue attached to this HAB, or NULL.
     * @return Queue pointer, or NULL if WinCreateMsgQueue was not
     *         called for this HAB.
     */
    Fs_Queue *GetQueue() const  { return m_pQueue; }

    /*!
     * @brief Attach a message queue (NWinCreateMsgQueue).
     *        Ownership transfers to this HAB.
     * @param [in] pQueue  Newly allocated queue.
     */
    void      SetQueue(Fs_Queue *pQueue) { m_pQueue = pQueue; }

    /*!
     * @brief Detach a message queue without deleting it
     *        (NWinDestroyMsgQueue after the caller has already
     *        deleted it).
     */
    void      ClearQueue()      { m_pQueue = NULL; }

private:
    BOOL     m_bUsed;
    ULONG    m_iHab;        /*!< Monotonic, unique, never reused. */
    ULONG    m_idClient;    /*!< Owning session (stable across reconnect). */
    ULONG    m_tid;         /*!< Client thread id (unique within process). */
    int      m_lastError;   /*!< Per-thread last error (PMERR_*). */

    Fs_Queue *m_pQueue;     /*!< Owning message queue; NULL if none. */
    /* Phase 4: Fs_ClassInfo *m_pClassList; */
};

/*!
 * @brief Container of all HAB slots, with an OS/2 mutex.
 *
 * Slots are keyed by the triple (@c idClient, @c tid, @c iHab): @c idClient
 * identifies the session (process), @c tid identifies the client thread
 * within that process, and @c iHab is the server-assigned anchor-block id.
 * TIDs are unique only within a process, so the first two parts are both
 * required to locate a HAB unambiguously; the third part is verified on
 * destroy to mirror OS/2's rule that the anchor block is identified by
 * the identity of the calling thread.
 *
 * Concurrency: the current transport (PMTCP) delivers all commands of one
 * client connection through a single server thread, so calls from one
 * session never run concurrently with themselves. The design anticipates
 * a future model in which each client thread has its own server thread:
 * every mutating operation (Create, Destroy, DestroyAll) runs atomically
 * under m_hmtx, and Destroy verifies (idClient, tid, iHab) in a single
 * pass so concurrent NWinTerminate calls — or MonitorLoop's grace sweep —
 * cannot race.
 *
 * All public methods take the lock internally. The Lock/Unlock pair is
 * provided for callers that need to inspect several slots atomically;
 * Fs_Server does not currently use it.
 */
class Fs_HabList {
public:
    Fs_HabList();
    ~Fs_HabList();

    /*!
     * @brief Allocate a new HAB for the given session and thread.
     *
     * Creation honours the runtime limit set by SetMax(). Both
     * @c idClient and @c tid must be non-zero.
     *
     * @param [in]  idClient   Owning session.
     * @param [in]  tid        Client thread id (unique within the process).
     * @param [out] pNewIHab   Receives the freshly assigned HAB id.
     * @return Slot pointer, or NULL if the table is full, the runtime
     *         limit is reached, or an argument is zero.
     */
    Fs_Hab *Create(ULONG idClient, ULONG tid, ULONG *pNewIHab);

    /*! @brief Find a HAB by its id. @return Slot, or NULL. */
    Fs_Hab *FindByIHab(ULONG iHab);

    /*!
     * @brief Find a HAB by (session, thread) identity.
     *
     * Used by NWinInitialize (to detect a thread that already called
     * WinInitialize) and by NWinTerminate (to locate the caller's
     * HAB from the client-supplied TID).
     *
     * @param [in] idClient  Owning session.
     * @param [in] tid       Client thread id.
     * @return Slot, or NULL if no match.
     */
    Fs_Hab *FindByTid(ULONG idClient, ULONG tid);

    /*!
     * @brief Destroy one HAB, verifying that idClient, tid and iHab all
     *        refer to the same slot.
     *
     * Mirrors OS/2's rule that the anchor block is identified by the
     * identity of the calling thread: idClient (process), tid (thread
     * within process) and iHab must agree. If any part does not match,
     * the slot is left untouched and the call fails.
     *
     * The check-and-Shutdown is performed under one lock acquisition;
     * GpiPsDestroyByHab runs outside the lock because it may be slow.
     *
     * @param [in] idClient  Owning session.
     * @param [in] tid       Client thread id.
     * @param [in] iHab      HAB id supplied by the client.
     * @return TRUE if a matching HAB was found and destroyed, FALSE otherwise.
     */
    BOOL    Destroy(ULONG idClient, ULONG tid, ULONG iHab);

    /*!
     * @brief Destroy all HABs belonging to a session.
     *
     * Used on CLIENT_EXIT and on grace expiry (reconnect window
     * closed). Cascades through GpiPsDestroyByHab for each HAB.
     * In the future per-thread-server model this remains the correct
     * fallback for a lost session (all threads of the process are gone).
     *
     * @return Number of HABs destroyed.
     */
    int     DestroyAll(ULONG idClient);

    /*! @brief Number of HABs owned by the given session. */
    int     CountByClient(ULONG idClient);

    /*! @brief Total number of active HABs. */
    int     CountActive();

    /*! @brief Table capacity (F_MAX_HABS). */
    int     Capacity() const { return F_MAX_HABS; }

    /*!
     * @brief Set the runtime HAB limit (clamped to [0, F_MAX_HABS]).
     * Called from Fs_Server::SetConfig. Not applied retroactively:
     * existing HABs stay; new Create() calls honour the new limit.
     */
    void    SetMax(ULONG ulMaxHabs);

    /*! @brief Current runtime limit. */
    ULONG   GetMax() const { return m_ulMaxHabs; }

private:
    void Lock();
    void Unlock();

    Fs_Hab  m_aHabs[F_MAX_HABS];
    ULONG   m_ulNextHab;
    ULONG   m_ulMaxHabs;
    HMTX    m_hmtx;
};

#endif /* FS_HAB_HPP */
