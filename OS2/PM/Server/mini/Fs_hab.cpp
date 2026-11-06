/*!
 * @file Fs_hab.cpp
 * @brief Fs_Hab and Fs_HabList implementation.
 */
#include <time.h>
#include "Fs_hab.hpp"
#include "F_debug.h"

/* GpiPsDestroyByHab lives in gpi_ps.h (server side). */
#define GPI_SERVER
#include "gpi_ps.h"

/* ======================================================================== */
/*  Fs_Hab                                                                  */
/* ======================================================================== */

Fs_Hab::Fs_Hab()
    : m_bUsed(FALSE),
      m_iHab(0),
      m_idClient(0),
      m_tid(0),
      m_lastError(0)
{
}

BOOL Fs_Hab::Init(ULONG idClient, ULONG tid, ULONG iHab)
{
    if (m_bUsed) return FALSE;
    if (idClient == 0 || tid == 0 || iHab == 0) return FALSE;

    m_bUsed     = TRUE;
    m_iHab      = iHab;
    m_idClient  = idClient;
    m_tid       = tid;
    m_lastError = 0;
    return TRUE;
}

void Fs_Hab::Shutdown()
{
    m_bUsed     = FALSE;
    m_iHab      = 0;
    m_idClient  = 0;
    m_tid       = 0;
    m_lastError = 0;
}

/* ======================================================================== */
/*  Fs_HabList                                                              */
/* ======================================================================== */

Fs_HabList::Fs_HabList()
    : m_ulNextHab(1),          /* skip 0: 0 = invalid HAB */
      m_ulMaxHabs(F_MAX_HABS)
{
    m_hmtx = NULL;
    if (DosCreateMutexSem(NULL, &m_hmtx, 0, FALSE) != 0) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[HABLIST] ERROR: DosCreateMutexSem failed");
        m_hmtx = NULL;
    }
}

Fs_HabList::~Fs_HabList()
{
    if (m_hmtx) {
        DosCloseMutexSem(m_hmtx);
        m_hmtx = NULL;
    }
}

void Fs_HabList::Lock()
{
    if (m_hmtx) DosRequestMutexSem(m_hmtx, SEM_INDEFINITE_WAIT);
}

void Fs_HabList::Unlock()
{
    if (m_hmtx) DosReleaseMutexSem(m_hmtx);
}

void Fs_HabList::SetMax(ULONG ulMaxHabs)
{
    if (ulMaxHabs > (ULONG)F_MAX_HABS)
        ulMaxHabs = (ULONG)F_MAX_HABS;

    Lock();
    m_ulMaxHabs = ulMaxHabs;
    Unlock();

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
               "[HABLIST] SetMax(%lu)", (unsigned long)ulMaxHabs);
}

Fs_Hab *Fs_HabList::Create(ULONG idClient, ULONG tid, ULONG *pNewIHab)
{
    Fs_Hab *pFound = NULL;
    ULONG   iHab   = 0;
    int     i, nUsed = 0;

    if (pNewIHab) *pNewIHab = 0;
    if (idClient == 0 || tid == 0) return NULL;

    Lock();

    /* Count used slots first: refuse before consuming an iHab
       from the monotonic counter. */
    for (i = 0; i < F_MAX_HABS; i++)
        if (m_aHabs[i].IsUsed()) nUsed++;

    if (nUsed >= (int)m_ulMaxHabs) {
        Unlock();
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[HABLIST] limit reached (%d/%lu)",
                   nUsed, (unsigned long)m_ulMaxHabs);
        return NULL;
    }

    for (i = 0; i < F_MAX_HABS; i++) {
        if (m_aHabs[i].IsUsed()) continue;

        iHab = m_ulNextHab++;
        if (iHab == 0) iHab = m_ulNextHab++;   /* skip 0 */

        if (m_aHabs[i].Init(idClient, tid, iHab)) {
            pFound = &m_aHabs[i];
            if (pNewIHab) *pNewIHab = iHab;
            break;
        }
    }
    Unlock();

    if (!pFound) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[HABLIST] ERROR: table full (%d slots)", F_MAX_HABS);
        return NULL;
    }
    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
               "[HABLIST] Create idClient=%lu tid=%lu -> iHab=%lu",
               (unsigned long)idClient, (unsigned long)tid,
               (unsigned long)iHab);
    return pFound;
}

Fs_Hab *Fs_HabList::FindByIHab(ULONG iHab)
{
    Fs_Hab *pFound = NULL;
    int     i;

    if (iHab == 0) return NULL;

    Lock();
    for (i = 0; i < F_MAX_HABS; i++) {
        if (m_aHabs[i].IsUsed() && m_aHabs[i].GetIHab() == iHab) {
            pFound = &m_aHabs[i];
            break;
        }
    }
    Unlock();
    return pFound;
}

Fs_Hab *Fs_HabList::FindByTid(ULONG idClient, ULONG tid)
{
    Fs_Hab *pFound = NULL;
    int     i;

    if (idClient == 0 || tid == 0) return NULL;

    Lock();
    for (i = 0; i < F_MAX_HABS; i++) {
        if (m_aHabs[i].IsUsed() &&
            m_aHabs[i].GetIdClient() == idClient &&
            m_aHabs[i].GetTid()      == tid) {
            pFound = &m_aHabs[i];
            break;
        }
    }
    Unlock();
    return pFound;
}

BOOL Fs_HabList::Destroy(ULONG idClient, ULONG tid, ULONG iHab)
{
    Fs_Hab *pHab = NULL;
    int     i, nPs;

    if (idClient == 0 || tid == 0 || iHab == 0) return FALSE;

    /* Atomic check-and-free: (idClient, tid, iHab) must all match
       the same slot. GpiPsDestroyByHab runs outside the lock because
       it may be slow; once the slot is free, no finder can reach it. */
    Lock();
    for (i = 0; i < F_MAX_HABS; i++) {
        if (m_aHabs[i].IsUsed() &&
            m_aHabs[i].GetIdClient() == idClient &&
            m_aHabs[i].GetTid()      == tid &&
            m_aHabs[i].GetIHab()     == iHab) {
            pHab = &m_aHabs[i];
            pHab->Shutdown();
            break;
        }
    }
    Unlock();

    if (!pHab) return FALSE;

    nPs = GpiPsDestroyByHab(iHab);

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
               "[HABLIST] Destroy idClient=%lu tid=%lu iHab=%lu (forced %d PS)",
               (unsigned long)idClient, (unsigned long)tid,
               (unsigned long)iHab, nPs);
    return TRUE;
}

int Fs_HabList::DestroyAll(ULONG idClient)
{
    int n = 0;
    int i;

    if (idClient == 0) return 0;

    Lock();
    for (i = 0; i < F_MAX_HABS; i++) {
        if (m_aHabs[i].IsUsed() && m_aHabs[i].GetIdClient() == idClient) {
            ULONG iHab = m_aHabs[i].GetIHab();
            Unlock();

            /* Cascade outside the lock: GpiPsDestroyByHab may be slow. */
            GpiPsDestroyByHab(iHab);

            Lock();
            m_aHabs[i].Shutdown();
            n++;
        }
    }
    Unlock();

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
               "[HABLIST] DestroyAll idClient=%lu -> %d HABs",
               (unsigned long)idClient, n);
    return n;
}

int Fs_HabList::CountByClient(ULONG idClient)
{
    int n = 0;
    int i;

    if (idClient == 0) return 0;

    Lock();
    for (i = 0; i < F_MAX_HABS; i++) {
        if (m_aHabs[i].IsUsed() && m_aHabs[i].GetIdClient() == idClient)
            n++;
    }
    Unlock();
    return n;
}

int Fs_HabList::CountActive()
{
    int n = 0;
    int i;

    Lock();
    for (i = 0; i < F_MAX_HABS; i++) {
        if (m_aHabs[i].IsUsed()) n++;
    }
    Unlock();
    return n;
}
