/*!
 * @file Fs_dc.cpp
 * @brief Fs_Dc and Fs_DcList implementation.
 */
#include "Fs_dc.hpp"
#include "F_debug.h"

/* ======================================================================== */
/*  Fs_Dc                                                                   */
/* ======================================================================== */

/*!
 * @brief Default constructor. Slot starts unused, all fields zeroed.
 * @return Nothing (constructor).
 */
Fs_Dc::Fs_Dc()
    : m_bUsed(FALSE),
      m_iHdc(0), m_iHwnd(0), m_iHab(0),
      m_pTarget(NULL),
      m_width(0), m_height(0), m_bpp(0),
      m_color(0)
{
    m_curPos.x = 0;
    m_curPos.y = 0;
    m_pInstance = NULL;
}

/*!
 * @brief Install a new DC into a FREE slot.
 *
 * @param [in] iHdc     Server-assigned HDC (monotonic, non-zero).
 * @param [in] iHwnd    Owning window id.
 * @param [in] iHab     Owning HAB id.
 * @param [in] pTarget  Target pixel buffer (may be NULL).
 * @param [in] width    Buffer width in pixels.
 * @param [in] height   Buffer height in pixels.
 * @param [in] bpp      Bytes per pixel.
 * @return TRUE on success, FALSE if the slot is used or iHdc is 0.
 * @retval TRUE   Slot populated.
 * @retval FALSE  Slot already used or iHdc == 0.
 */
BOOL Fs_Dc::Init(ULONG iHdc, ULONG iHwnd, ULONG iHab,
                 PBYTE pTarget, LONG width, LONG height, LONG bpp)
{
    if (m_bUsed)    return FALSE;
    if (iHdc == 0)  return FALSE;

    m_bUsed   = TRUE;
    m_iHdc    = iHdc;
    m_iHwnd   = iHwnd;
    m_iHab    = iHab;
    m_pTarget = pTarget;
    m_width   = width;
    m_height  = height;
    m_bpp     = bpp;
    m_color   = 0;
    m_curPos.x = 0;
    m_curPos.y = 0;
    return TRUE;
}

/*!
 * @brief Release the slot (state -> FREE, all fields cleared).
 */
void Fs_Dc::Shutdown()
{
    m_bUsed   = FALSE;
    m_iHdc    = 0;
    m_iHwnd   = 0;
    m_iHab    = 0;
    m_pTarget = NULL;
    m_width   = 0;
    m_height  = 0;
    m_bpp     = 0;
    m_color   = 0;
    m_curPos.x = 0;
    m_curPos.y = 0;
}

/* ======================================================================== */
/*  Fs_DcList                                                               */
/* ======================================================================== */

/*!
 * @brief Create the DC registry and its OS/2 mutex.
 *
 * The monotonic HDC counter starts at 1 (0 = invalid HDC). On mutex
 * creation failure m_hmtx is left NULL and Lock/Unlock become no-ops,
 * so callers continue to work (without inter-thread protection in
 * that degenerate case).
 *
 * @return Nothing (constructor).
 */
Fs_DcList::Fs_DcList()
    : m_ulNextHdc(1)           /* skip 0: 0 = invalid HDC */
{
    m_hmtx = NULL;
    if (DosCreateMutexSem(NULL, &m_hmtx, 0, FALSE) != 0) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[DCLIST] ERROR: DosCreateMutexSem failed");
        m_hmtx = NULL;
    }
}

/*!
 * @brief Close the registry mutex.
 * @return Nothing (destructor).
 */
Fs_DcList::~Fs_DcList()
{
    if (m_hmtx) {
        DosCloseMutexSem(m_hmtx);
        m_hmtx = NULL;
    }
}

/*!
 * @brief Acquire the registry mutex (no-op if it was not created).
 */
void Fs_DcList::Lock()
{
    if (m_hmtx) DosRequestMutexSem(m_hmtx, SEM_INDEFINITE_WAIT);
}

/*!
 * @brief Release the registry mutex (no-op if it was not created).
 */
void Fs_DcList::Unlock()
{
    if (m_hmtx) DosReleaseMutexSem(m_hmtx);
}

/*!
 * @brief Allocate a new DC slot.
 *
 * @param [in]  iHwnd     Owning window id.
 * @param [in]  iHab      Owning HAB id.
 * @param [in]  pTarget   Target pixel buffer (may be NULL).
 * @param [in]  width     Buffer width in pixels.
 * @param [in]  height    Buffer height in pixels.
 * @param [in]  bpp       Bytes per pixel.
 * @param [out] pNewHdc   Receives the freshly assigned HDC.
 * @return Slot pointer, or NULL on failure.
 * @retval !NULL  DC allocated.
 * @retval NULL   Table full.
 */
Fs_Dc *Fs_DcList::Create(ULONG iHwnd, ULONG iHab,
                         PBYTE pTarget, LONG width, LONG height, LONG bpp,
                         ULONG *pNewHdc)
{
    Fs_Dc *pFound = NULL;
    ULONG  iHdc   = 0;
    int    i, nUsed = 0;

    if (pNewHdc) *pNewHdc = 0;

    Lock();

    for (i = 0; i < F_MAX_DCS; i++)
        if (m_aDcs[i].IsUsed()) nUsed++;

    if (nUsed >= F_MAX_DCS) {
        Unlock();
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[DCLIST] limit reached (%d/%d)", nUsed, F_MAX_DCS);
        return NULL;
    }

    for (i = 0; i < F_MAX_DCS; i++) {
        if (m_aDcs[i].IsUsed()) continue;

        iHdc = m_ulNextHdc++;
        if (iHdc == 0) iHdc = m_ulNextHdc++;   /* skip 0 */

        if (m_aDcs[i].Init(iHdc, iHwnd, iHab,
                           pTarget, width, height, bpp)) {
            pFound = &m_aDcs[i];
            if (pNewHdc) *pNewHdc = iHdc;
            break;
        }
    }
    Unlock();

    if (!pFound) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[DCLIST] ERROR: table full (%d slots)", F_MAX_DCS);
        return NULL;
    }

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
               "[DCLIST] Create hwnd=%lu iHab=%lu -> hdc=%lu",
               (unsigned long)iHwnd, (unsigned long)iHab,
               (unsigned long)iHdc);
    return pFound;
}

/*!
 * @brief Find a DC by HDC.
 *
 * @param [in] iHdc  HDC value.
 * @return Slot pointer, or NULL if not found.
 * @retval !NULL  Matching slot.
 * @retval NULL   No such DC.
 */
Fs_Dc *Fs_DcList::Find(ULONG iHdc)
{
    Fs_Dc *pFound = NULL;
    int    i;

    if (iHdc == 0) return NULL;

    Lock();
    for (i = 0; i < F_MAX_DCS; i++) {
        if (m_aDcs[i].IsUsed() && m_aDcs[i].GetHdc() == iHdc) {
            pFound = &m_aDcs[i];
            break;
        }
    }
    Unlock();
    return pFound;
}

/*!
 * @brief Destroy one DC slot.
 *
 * @param [in] iHdc  HDC value.
 * @return TRUE if found and destroyed.
 * @retval TRUE   Slot freed.
 * @retval FALSE  No such DC.
 */
BOOL Fs_DcList::Destroy(ULONG iHdc)
{
    Fs_Dc *pDc = NULL;
    int    i;

    if (iHdc == 0) return FALSE;

    Lock();
    for (i = 0; i < F_MAX_DCS; i++) {
        if (m_aDcs[i].IsUsed() && m_aDcs[i].GetHdc() == iHdc) {
            pDc = &m_aDcs[i];
            pDc->Shutdown();
            break;
        }
    }
    Unlock();

    if (!pDc) return FALSE;

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
               "[DCLIST] Destroy hdc=%lu", (unsigned long)iHdc);
    return TRUE;
}

/*!
 * @brief Destroy all DCs owned by a given HAB.
 *
 * @param [in] iHab  Owning HAB id (0 = no-op).
 * @return Number of DCs destroyed.
 * @retval 0    No DCs owned by iHab, or iHab == 0 (no-op).
 * @retval >0   Count of freed slots.
 */
int Fs_DcList::DestroyByHab(ULONG iHab)
{
    int n = 0;
    int i;

    if (iHab == 0) return 0;

    Lock();
    for (i = 0; i < F_MAX_DCS; i++) {
        if (m_aDcs[i].IsUsed() && m_aDcs[i].GetHab() == iHab) {
            m_aDcs[i].Shutdown();
            n++;
        }
    }
    Unlock();

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
               "[DCLIST] DestroyByHab iHab=%lu -> %d DCs",
               (unsigned long)iHab, n);
    return n;
}

/*!
 * @brief Count DCs owned by a given HAB.
 *
 * @param [in] iHab  Owning HAB id.
 * @return Number of matching DCs.
 * @retval 0    No DCs owned by iHab, or iHab == 0.
 * @retval >0   Count.
 */
int Fs_DcList::CountByHab(ULONG iHab)
{
    int n = 0;
    int i;

    if (iHab == 0) return 0;

    Lock();
    for (i = 0; i < F_MAX_DCS; i++) {
        if (m_aDcs[i].IsUsed() && m_aDcs[i].GetHab() == iHab)
            n++;
    }
    Unlock();
    return n;
}

/*!
 * @brief Count all active DCs in the registry.
 *
 * @return Number of used slots.
 * @retval >=0  Count.
 */
int Fs_DcList::CountActive()
{
    int n = 0;
    int i;

    Lock();
    for (i = 0; i < F_MAX_DCS; i++)
        if (m_aDcs[i].IsUsed()) n++;
    Unlock();
    return n;
}
