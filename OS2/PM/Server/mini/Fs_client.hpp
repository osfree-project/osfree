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
    enum State {
        StateFree     = 0,
        StateActive   = 1,
        StateDetached = 2
    };

    Fs_ClientConnection();

    BOOL  Init(void *pRecvobj, ULONG idClient);
    void  Shutdown();
    void  Touch();
    BOOL  IsExpiredInactive(time_t now, ULONG ulTimeout) const;
    BOOL  IsExpiredGrace   (time_t now, ULONG ulTimeout) const;
    void  ToDetached(time_t now);
    void  ToActive(void *pRecvobj, time_t now);

    /* Attach/detach Hab (NWinInitialize / NWinTerminate). */
    void  AttachHab(ULONG tid, ULONG iHab);
    void  DetachHab();

    /* Accessors */
    ULONG  GetIdClient()        const { return m_idClient; }
    void  *GetRecvobj()         const { return m_pRecvobj; }
    State  GetState()           const { return m_state; }
    BOOL   IsFree()             const { return m_state == StateFree; }
    BOOL   IsActive()           const { return m_state == StateActive; }
    BOOL   IsDetached()         const { return m_state == StateDetached; }
    ULONG  GetTid()             const { return m_tid; }
    ULONG  GetIHab()            const { return m_iHab; }
    time_t GetLastActivity()    const { return m_timeLastActivity; }
    time_t GetTimeDetached()    const { return m_timeDetached; }

private:
    State   m_state;
    ULONG   m_idClient;
    void   *m_pRecvobj;
    ULONG   m_tid;
    ULONG   m_iHab;
    time_t  m_timeLastActivity;
    time_t  m_timeDetached;
};


#endif /* FS_CLIENT_HPP */
