/*!
 * @file Fs_client.cpp
 * @brief Fs_ClientConnection and Fs_ClientList implementation.
 */
#include <time.h>
#include "Fs_client.hpp"
#include "F_debug.h"

/* ======================================================================== */
/*  Fs_ClientConnection                                                     */
/* ======================================================================== */

Fs_ClientConnection::Fs_ClientConnection()
    : m_state(StateFree),
      m_idClient(0),
      m_pRecvobj(NULL),
      m_tid(0),
      m_iHab(0),
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
    m_tid              = 0;
    m_iHab             = 0;
    m_timeLastActivity = time(NULL);
    m_timeDetached     = 0;
    return TRUE;
}

void Fs_ClientConnection::Shutdown()
{
    int ctx;

    if (m_state == StateFree) return;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_CLIENT, F_DBG_LVL_VERBOSE,
                         "[CLIENT %lu] Shutdown(recvobj=%p, prev state=%d)",
                         (unsigned long)m_idClient, m_pRecvobj, (int)m_state);

    F_DbgPrint(F_DBG_SEC_FS_CLIENT, F_DBG_LVL_INFO,
               "[CLIENT %lu] shutdown (recvobj=%p, prev state=%d)",
               (unsigned long)m_idClient, m_pRecvobj, (int)m_state);

    m_state            = StateFree;
    m_idClient         = 0;
    m_pRecvobj         = NULL;
    m_tid              = 0;
    m_iHab             = 0;
    m_timeLastActivity = 0;
    m_timeDetached     = 0;

    F_DbgCtxExitf(ctx, "-> StateFree");
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
    int ctx;

    if (m_state != StateActive) return;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_CLIENT, F_DBG_LVL_VERBOSE,
                         "[CLIENT %lu] ToDetached(recvobj=%p)",
                         (unsigned long)m_idClient, m_pRecvobj);

    F_DbgPrint(F_DBG_SEC_FS_CLIENT, F_DBG_LVL_INFO,
               "[CLIENT %lu] ACTIVE -> DETACHED (recvobj=%p)",
               (unsigned long)m_idClient, m_pRecvobj);

    m_state        = StateDetached;
    m_pRecvobj     = NULL;
    m_timeDetached = now;

    F_DbgCtxExitf(ctx, "-> StateDetached");
}

void Fs_ClientConnection::ToActive(void *pRecvobj, time_t now)
{
    int ctx;

    if (m_state != StateDetached) return;
    if (pRecvobj == NULL)         return;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_CLIENT, F_DBG_LVL_VERBOSE,
                         "[CLIENT %lu] ToActive(recvobj=%p)",
                         (unsigned long)m_idClient, pRecvobj);

    F_DbgPrint(F_DBG_SEC_FS_CLIENT, F_DBG_LVL_INFO,
               "[CLIENT %lu] DETACHED -> ACTIVE (recvobj=%p)",
               (unsigned long)m_idClient, pRecvobj);

    m_state            = StateActive;
    m_pRecvobj         = pRecvobj;
    m_timeLastActivity = now;
    m_timeDetached     = 0;

    F_DbgCtxExitf(ctx, "-> StateActive");
}

void Fs_ClientConnection::AttachHab(ULONG tid, ULONG iHab)
{
    if (m_state != StateActive) return;
    m_tid  = tid;
    m_iHab = iHab;
}

void Fs_ClientConnection::DetachHab()
{
    m_tid  = 0;
    m_iHab = 0;
}

