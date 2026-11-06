/*!
 * @file Fs_client.hpp
 * @brief Per-connection client slots and the client list container.
 *
 * One Fs_ClientConnection per TCP connection (recvobj). Each slot has
 * one of three states:
 *
 *   StateFree      - slot available
 *   StateActive    - client is connected and sending commands
 *   StateDetached  - client stopped responding; slot is held for a
 *                    possible F_CMD_CLIENT_RECONNECT(idClient) until the
 *                    grace timeout expires
 *
 * Fs_ClientList serializes access with an OS/2 mutex. Every public
 * method takes the lock internally; callers do not need Lock/Unlock
 * unless they need a multi-call critical section (not required by the
 * current Fs_Server).
 *
 * PS/HAB are not tracked here yet; that comes with the HAB step.
 */
#ifndef FS_CLIENT_HPP
#define FS_CLIENT_HPP

#include "F_platform.h"
#include "Fs_config.hpp"

/*!
 * @brief One client connection slot.
 */
class Fs_ClientConnection {
public:
    /*! @brief Slot state. */
    enum State {
        StateFree     = 0,
        StateActive   = 1,
        StateDetached = 2
    };

    Fs_ClientConnection();

    /*! @brief Install a new ACTIVE connection into a FREE slot. */
    BOOL  Init(void *pRecvobj, ULONG idClient);

    /*! @brief Release the slot (state -> FREE, all fields cleared). */
    void  Shutdown();

    /*! @brief Refresh last-activity timestamp (ACTIVE only). */
    void  Touch();

    /*! @brief True if ACTIVE and now - lastActivity > ulTimeout. */
    BOOL  IsExpiredInactive(time_t now, ULONG ulTimeout) const;

    /*! @brief True if DETACHED and now - timeDetached > ulTimeout. */
    BOOL  IsExpiredGrace   (time_t now, ULONG ulTimeout) const;

    /*! @brief ACTIVE -> DETACHED (recvobj dropped, clientId retained). */
    void  ToDetached(time_t now);

    /*! @brief DETACHED -> ACTIVE (reconnect succeeded). */
    void  ToActive(void *pRecvobj, time_t now);

    /* ---- Accessors ---- */
    ULONG  GetIdClient()        const { return m_idClient; }
    void  *GetRecvobj()         const { return m_pRecvobj; }
    State  GetState()           const { return m_state; }
    BOOL   IsFree()             const { return m_state == StateFree; }
    BOOL   IsActive()           const { return m_state == StateActive; }
    BOOL   IsDetached()         const { return m_state == StateDetached; }
    time_t GetLastActivity()    const { return m_timeLastActivity; }
    time_t GetTimeDetached()    const { return m_timeDetached; }

private:
    State   m_state;
    ULONG   m_idClient;
    void   *m_pRecvobj;
    time_t  m_timeLastActivity;
    time_t  m_timeDetached;
};

/*!
 * @brief Container of all client slots, with an OS/2 mutex.
 *
 * All public methods take the lock internally. The Lock/Unlock pair is
 * provided for callers that need to inspect several slots atomically;
 * Fs_Server does not currently use it.
 */
class Fs_ClientList {
public:
    Fs_ClientList();
    ~Fs_ClientList();

    /* Manual critical section, if ever needed. */
    void Lock();
    void Unlock();

    /*!
     * @brief Find the slot currently bound to pRecvobj (ACTIVE only).
     * @return Slot pointer, or NULL if none.
     */
    Fs_ClientConnection *FindByRecvobj(void *pRecvobj);

    /*!
     * @brief Find the slot holding idClient (ACTIVE or DETACHED).
     * @return Slot pointer, or NULL if none.
     */
    Fs_ClientConnection *FindByIdClient(ULONG idClient);

    /*!
     * @brief Find a FREE slot without creating anything.
     * @return Slot pointer, or NULL if the table is full.
     */
    Fs_ClientConnection *FindFree();

    /*!
     * @brief Return the ACTIVE slot for pRecvobj, creating one if needed.
     *
     * On success, *pNewIdClient receives the freshly assigned idClient
     * when a new slot was created; on the "found existing" path
     * *pNewIdClient is set to 0 (caller can distinguish: 0 means
     * "already had a session").
     *
     * Creation honours the runtime limit set by SetMax(); a reconnect
     * of an existing recvobj is never blocked by the limit.
     *
     * @return Slot pointer, or NULL if the table is full or the
     *         runtime limit is reached.
     */
    Fs_ClientConnection *GetOrCreate(void *pRecvobj, ULONG *pNewIdClient);

    /*!
     * @brief Sweep the table applying both timeouts.
     *
     *   ACTIVE   + IsExpiredInactive(now, ulInactive) -> ToDetached(now)
     *   DETACHED + IsExpiredGrace(now, ulGrace)       -> Shutdown()
     *
     * @return Number of slots whose state changed (detached + freed).
     */
    int Expire(time_t now, ULONG ulInactive, ULONG ulGrace);

    /*! @brief ACTIVE + DETACHED count. */
    int CountActive() const;

    /*! @brief FREE count. */
    int CountFree() const;

    /*! @brief Table capacity (F_MAX_CLIENTS). */
    int Capacity() const { return F_MAX_CLIENTS; }

    /*!
     * @brief Set the runtime client limit (clamped to [0, F_MAX_CLIENTS]).
     *
     * Called from Fs_Server::SetConfig. Not applied retroactively:
     * existing sessions stay; new GetOrCreate() calls honour the new
     * limit.
     */
    void  SetMax(ULONG ulMaxClients);

    /*! @brief Current runtime limit. */
    ULONG GetMax() const { return m_ulMaxClients; }

private:
    Fs_ClientConnection m_aClients[F_MAX_CLIENTS];
    ULONG               m_ulNextId;
    ULONG               m_ulMaxClients;
    HMTX                m_hmtx;
};

#endif /* FS_CLIENT_HPP */
