/*!
 * @file Fs_client.cpp
 * @brief Fs_ClientConnection and Fs_ClientList implementation.
 */
#include <time.h>
#include "Fs_client.hpp"

extern void log(const char *fmt, ...);

/* ======================================================================== */
/*  Fs_ClientConnection                                                     */
/* ======================================================================== */

Fs_ClientConnection::Fs_ClientConnection()
    : m_state(StateFree),
      m_idClient(0),
      m_pRecvobj(NULL),
      m_timeLastActivity(0),
      m_timeDetached(0)
{
}

BOOL Fs_ClientConnection::Init(void *pRecvobj, ULONG idClient)
{
    if (m_state != StateFree) return FALSE;
    if (pRecvobj == NULL)     return FALSE;

    m_state            = StateActive;
    m_idClient         = idClient;
    m_pRecvobj         = pRecvobj;
    m_timeLastActivity = time(NULL);
    m_timeDetached     = 0;
    return TRUE;
}

void Fs_ClientConnection::Shutdown()
{
    if (m_state == StateFree) return;

    log("[CLIENT %lu] shutdown (recvobj=%p, prev state=%d)\n",
        (unsigned long)m_idClient, m_pRecvobj, (int)m_state);

    m_state            = StateFree;
    m_idClient         = 0;
    m_pRecvobj         = NULL;
    m_timeLastActivity = 0;
    m_timeDetached     = 0;
}

void Fs_ClientConnection::Touch()
{
    if (m_state != StateActive) return;
    m_timeLastActivity = time(NULL);
}

BOOL Fs_ClientConnection::IsExpiredInactive(time_t now, ULONG ulTimeout) const
{
    if (m_state != StateActive) return FALSE;
    if (ulTimeout == 0)         return FALSE;   /* 0 = never expire */
    return (now - m_timeLastActivity) > (time_t)ulTimeout;
}

BOOL Fs_ClientConnection::IsExpiredGrace(time_t now, ULONG ulTimeout) const
{
    if (m_state != StateDetached) return FALSE;
    if (ulTimeout == 0)           return FALSE;
    return (now - m_timeDetached) > (time_t)ulTimeout;
}

void Fs_ClientConnection::ToDetached(time_t now)
{
    if (m_state != StateActive) return;

    log("[CLIENT %lu] ACTIVE -> DETACHED (recvobj=%p)\n",
        (unsigned long)m_idClient, m_pRecvobj);

    m_state        = StateDetached;
    m_pRecvobj     = NULL;
    m_timeDetached = now;
}

void Fs_ClientConnection::ToActive(void *pRecvobj, time_t now)
{
    if (m_state != StateDetached) return;
    if (pRecvobj == NULL)         return;

    log("[CLIENT %lu] DETACHED -> ACTIVE (recvobj=%p)\n",
        (unsigned long)m_idClient, pRecvobj);

    m_state            = StateActive;
    m_pRecvobj         = pRecvobj;
    m_timeLastActivity = now;
    m_timeDetached     = 0;
}

/* ======================================================================== */
/*  Fs_ClientList                                                           */
/* ======================================================================== */

Fs_ClientList::Fs_ClientList()
    : m_ulNextId(1)
{
    m_hmtx = NULL;
    if (DosCreateMutexSem(NULL, &m_hmtx, 0, FALSE) != 0) {
        log("[CLIENTLIST] ERROR: DosCreateMutexSem failed\n");
        m_hmtx = NULL;
    }
}

Fs_ClientList::~Fs_ClientList()
{
    if (m_hmtx) {
        DosCloseMutexSem(m_hmtx);
        m_hmtx = NULL;
    }
}

void Fs_ClientList::Lock()
{
    if (m_hmtx) DosRequestMutexSem(m_hmtx, SEM_INDEFINITE_WAIT);
}

void Fs_ClientList::Unlock()
{
    if (m_hmtx) DosReleaseMutexSem(m_hmtx);
}

Fs_ClientConnection *Fs_ClientList::FindByRecvobj(void *pRecvobj)
{
    int i;
    Fs_ClientConnection *pFound = NULL;

    if (pRecvobj == NULL) return NULL;

    Lock();
    for (i = 0; i < F_MAX_CLIENTS; i++) {
        if (m_aClients[i].IsActive() &&
            m_aClients[i].GetRecvobj() == pRecvobj) {
            pFound = &m_aClients[i];
            break;
        }
    }
    Unlock();
    return pFound;
}

Fs_ClientConnection *Fs_ClientList::FindByIdClient(ULONG idClient)
{
    int i;
    Fs_ClientConnection *pFound = NULL;

    if (idClient == 0) return NULL;

    Lock();
    for (i = 0; i < F_MAX_CLIENTS; i++) {
        if ((m_aClients[i].IsActive() || m_aClients[i].IsDetached()) &&
            m_aClients[i].GetIdClient() == idClient) {
            pFound = &m_aClients[i];
            break;
        }
    }
    Unlock();
    return pFound;
}

Fs_ClientConnection *Fs_ClientList::FindFree()
{
    int i;
    Fs_ClientConnection *pFound = NULL;

    Lock();
    for (i = 0; i < F_MAX_CLIENTS; i++) {
        if (m_aClients[i].IsFree()) {
            pFound = &m_aClients[i];
            break;
        }
    }
    Unlock();
    return pFound;
}

Fs_ClientConnection *Fs_ClientList::GetOrCreate(void *pRecvobj,
                                                ULONG *pNewIdClient)
{
    Fs_ClientConnection *pConn = NULL;
    ULONG               idNew  = 0;
    int                 i;

    if (pNewIdClient) *pNewIdClient = 0;
    if (pRecvobj == NULL) return NULL;

    Lock();

    /* 1. Existing ACTIVE slot for this recvobj? */
    for (i = 0; i < F_MAX_CLIENTS; i++) {
        if (m_aClients[i].IsActive() &&
            m_aClients[i].GetRecvobj() == pRecvobj) {
            pConn = &m_aClients[i];
            break;
        }
    }

    /* 2. Otherwise grab the first FREE slot. */
    if (pConn == NULL) {
        for (i = 0; i < F_MAX_CLIENTS; i++) {
            if (m_aClients[i].IsFree()) {
                idNew = m_ulNextId++;
                if (idNew == 0) idNew = m_ulNextId++;  /* skip 0 */
                if (m_aClients[i].Init(pRecvobj, idNew)) {
                    pConn = &m_aClients[i];
                    if (pNewIdClient) *pNewIdClient = idNew;
                }
                break;
            }
        }
    }

    Unlock();

    if (pConn == NULL) {
        log("[CLIENTLIST] ERROR: table full (%d slots)\n", F_MAX_CLIENTS);
    }
    return pConn;
}

int Fs_ClientList::Expire(time_t now, ULONG ulInactive, ULONG ulGrace)
{
    int i;
    int nChanged = 0;

    Lock();
    for (i = 0; i < F_MAX_CLIENTS; i++) {
        Fs_ClientConnection *p = &m_aClients[i];

        if (p->IsActive() && p->IsExpiredInactive(now, ulInactive)) {
            p->ToDetached(now);
            nChanged++;
            continue;
        }
        if (p->IsDetached() && p->IsExpiredGrace(now, ulGrace)) {
            log("[CLIENT %lu] grace timeout expired\n",
                (unsigned long)p->GetIdClient());
            p->Shutdown();
            nChanged++;
        }
    }
    Unlock();
    return nChanged;
}

int Fs_ClientList::CountActive() const
{
    int i, n = 0;
    for (i = 0; i < F_MAX_CLIENTS; i++) {
        if (m_aClients[i].IsActive() || m_aClients[i].IsDetached())
            n++;
    }
    return n;
}

int Fs_ClientList::CountFree() const
{
    int i, n = 0;
    for (i = 0; i < F_MAX_CLIENTS; i++) {
        if (m_aClients[i].IsFree())
            n++;
    }
    return n;
}
