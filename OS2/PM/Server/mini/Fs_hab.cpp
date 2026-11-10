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

/*!
 * @brief Default constructor. Slot starts unused, all fields zeroed.
 * @return Nothing (constructor).
 */
Fs_Hab::Fs_Hab()
    : m_bUsed(FALSE),
      m_iHab(0),
      m_idClient(0),
      m_tid(0),
      m_lastError(0)
{
}

/*!
 * @brief Install a new HAB into a FREE slot.
 *
 * Requires all three identifiers to be non-zero: idClient (owning
 * session), tid (client thread id, unique within the process), and
 * iHab (server-assigned handle, monotonic).
 *
 * @param [in] idClient  Owning session.
 * @param [in] tid       Client thread id.
 * @param [in] iHab      Server-assigned HAB id.
 * @return TRUE on success, FALSE if the slot is already used or any
 *         argument is zero.
 * @retval TRUE   Slot populated.
 * @retval FALSE  Slot already used or argument invalid.
 */
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

/*!
 * @brief Release the slot (state -> FREE, all fields cleared).
 */
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

/*!
 * @brief Create the HAB registry and its OS/2 mutex.
 *
 * The monotonic HAB counter starts at 1 (0 = invalid HAB). On mutex
 * creation failure m_hmtx is left NULL and Lock/Unlock become no-ops,
 * so callers continue to work (without inter-thread protection in
 * that degenerate case).
 *
 * @return Nothing (constructor).
 */
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

/*!
 * @brief Close the registry mutex.
 * @return Nothing (destructor).
 */
Fs_HabList::~Fs_HabList()
{
    if (m_hmtx) {
        DosCloseMutexSem(m_hmtx);
        m_hmtx = NULL;
    }
}

/*!
 * @brief Acquire the registry mutex (no-op if it was not created).
 */
void Fs_HabList::Lock()
{
    if (m_hmtx) DosRequestMutexSem(m_hmtx, SEM_INDEFINITE_WAIT);
}

/*!
 * @brief Release the registry mutex (no-op if it was not created).
 */
void Fs_HabList::Unlock()
{
    if (m_hmtx) DosReleaseMutexSem(m_hmtx);
}

/*!
 * @brief Set the runtime HAB limit (clamped to F_MAX_HABS).
 *
 * Not applied retroactively: existing HABs stay; new Create() calls
 * honour the new limit.
 *
 * @param [in] ulMaxHabs  New limit, 0..F_MAX_HABS.
 */
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

/*!
 * @brief Allocate a new HAB for the given session and thread.
 *
 * Both idClient and tid must be non-zero. If the runtime limit
 * (m_ulMaxHabs) has been reached, returns NULL without consuming an
 * iHab from the monotonic counter.
 *
 * @param [in]  idClient  Owning session.
 * @param [in]  tid       Client thread id.
 * @param [out] pNewIHab  Receives the freshly assigned HAB id.
 * @return Slot pointer, or NULL on failure.
 * @retval !NULL  HAB allocated and registered.
 * @retval NULL   Table full, runtime limit reached, or invalid args.
 */
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

/*!
 * @brief Find a HAB by its server-assigned id.
 * @param [in] iHab  HAB id.
 * @return Slot pointer, or NULL if not found or iHab is 0.
 * @retval !NULL  Matching slot.
 * @retval NULL   No such HAB.
 */
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

/*!
 * @brief Find a HAB by (session, thread) identity.
 *
 * Used by NWinInitialize (to detect a thread that already called
 * WinInitialize) and by NWinTerminate (to locate the caller's HAB
 * from the client-supplied TID).
 *
 * @param [in] idClient  Owning session.
 * @param [in] tid       Client thread id.
 * @return Slot pointer, or NULL if not found.
 * @retval !NULL  Matching slot.
 * @retval NULL   No such (idClient, tid) pair.
 */
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

/*!
 * @brief Destroy one HAB, verifying idClient, tid and iHab.
 *
 * Mirrors OS/2's rule that the anchor block is identified by the
 * identity of the calling thread: idClient (process), tid (thread
 * within process) and iHab must all agree. If any part does not
 * match, the slot is left untouched and the call fails.
 *
 * The check-and-Shutdown is performed under one lock acquisition;
 * GpiPsDestroyByHab runs outside the lock because it may be slow.
 *
 * @param [in] idClient  Owning session.
 * @param [in] tid       Client thread id.
 * @param [in] iHab      HAB id supplied by the client.
 * @return TRUE if a matching HAB was found and destroyed.
 * @retval TRUE   Slot matched and freed.
 * @retval FALSE  No matching slot.
 */
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

/*!
 * @brief Destroy all HABs belonging to a session.
 *
 * Used on CLIENT_EXIT and on grace expiry (reconnect window closed).
 * Cascades through GpiPsDestroyByHab for each HAB.
 *
 * @param [in] idClient  Owning session (0 = no-op).
 * @return Number of HABs destroyed.
 * @retval 0    idClient == 0 (no-op), or no HABs owned by idClient.
 * @retval >0   Count of freed slots.
 */
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

/*!
 * @brief Count HABs owned by a given session.
 * @param [in] idClient  Owning session.
 * @return Number of matching HABs.
 * @retval 0    idClient == 0, or no HABs owned by idClient.
 * @retval >0   Count.
 */
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

/*!
 * @brief Count all active HABs in the registry.
 * @return Number of used slots.
 * @retval >=0  Count.
 */
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
