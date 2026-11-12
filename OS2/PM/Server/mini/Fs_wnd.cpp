/*!
 * @file Fs_wnd.cpp
 * @brief Fs_Wnd and Fs_WndList implementation.
 */
#include "Fs_wnd.hpp"
#include "F_debug.h"

#define INCL_GRE_DCS
#include "pmddi.h"

#include <string.h>

/* ======================================================================== */
/*  Fs_Wnd                                                                  */
/* ======================================================================== */

/*!
 * @brief Default constructor. Slot starts unused, all fields zeroed.
 * @return Nothing (constructor).
 */
Fs_Wnd::Fs_Wnd()
    : m_bUsed(FALSE), m_iHwnd(0), m_iHab(0), /* m_hPS(NULL),*/ m_hdc(NULL),
      m_x(0), m_y(0), m_cx(0), m_cy(0),
      m_bVisible(FALSE), m_flStyle(0),
      m_pCtlData(NULL), m_cbCtlData(0),
      m_pPresParams(NULL), m_cbPresParams(0)
{
    m_szTitle[0] = '\0';
}

/*!
 * @brief Install a new window into a FREE slot.
 *
 * @param [in] iHwnd  Server-assigned HWND (monotonic, non-zero).
 * @param [in] iHab   Owning HAB id (0 allowed for the desktop window).
 * @return TRUE on success, FALSE if the slot is used or iHwnd is 0.
 * @retval TRUE   Slot populated.
 * @retval FALSE  Slot already used or iHwnd == 0.
 */
BOOL Fs_Wnd::Init(ULONG iHwnd, ULONG iHab)
{
    if (m_bUsed)  return FALSE;
    if (iHwnd == 0) return FALSE;

    m_bUsed  = TRUE;
    m_iHwnd  = iHwnd;
    m_iHab   = iHab;
//    m_hPS    = NULL;
    m_hdc    = NULL;
    m_x = m_y = m_cx = m_cy = 0;
    m_bVisible = FALSE;
    m_flStyle  = 0;
    m_szTitle[0] = '\0';
    m_pCtlData = NULL;    m_cbCtlData    = 0;
    m_pPresParams = NULL; m_cbPresParams = 0;
    return TRUE;
}

/*!
 * @brief Release the slot (state -> FREE, cached HPS and HDC cleared).
 */
void Fs_Wnd::Shutdown()
{
    if (m_pCtlData)    { DosFreeMem(m_pCtlData);    m_pCtlData    = NULL; }
    if (m_pPresParams) { DosFreeMem(m_pPresParams); m_pPresParams = NULL; }
    m_cbCtlData    = 0;
    m_cbPresParams = 0;

    if (m_hdc != NULLHANDLE) {
        GreCloseDC(m_hdc);
        m_hdc = NULLHANDLE;
    }

    m_bUsed = FALSE;
    m_iHwnd = 0;
//    m_hPS   = NULL;
    m_iHab  = 0;
    m_x = m_y = m_cx = m_cy = 0;
    m_bVisible = FALSE;
    m_flStyle  = 0;
    m_szTitle[0] = '\0';
}


/* ======================================================================== */
/*  Fs_WndList                                                              */
/* ======================================================================== */

/*!
 * @brief Create the window registry and its OS/2 mutex.
 *
 * The monotonic HWND counter starts at 1 (0 = invalid HWND). On mutex
 * creation failure m_hmtx is left NULL and Lock/Unlock become no-ops,
 * so callers continue to work (without inter-thread protection in
 * that degenerate case).
 *
 * @return Nothing (constructor).
 */
Fs_WndList::Fs_WndList()
    : m_ulNextHwnd(1)          /* skip 0: 0 = invalid HWND */
{
    m_hmtx = NULL;
    if (DosCreateMutexSem(NULL, &m_hmtx, 0, FALSE) != 0) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[WNDLIST] ERROR: DosCreateMutexSem failed");
        m_hmtx = NULL;
    }
}

/*!
 * @brief Close the registry mutex.
 * @return Nothing (destructor).
 */
Fs_WndList::~Fs_WndList()
{
    if (m_hmtx) {
        DosCloseMutexSem(m_hmtx);
        m_hmtx = NULL;
    }
}

/*!
 * @brief Acquire the registry mutex (no-op if it was not created).
 */
void Fs_WndList::Lock()
{
    if (m_hmtx) DosRequestMutexSem(m_hmtx, SEM_INDEFINITE_WAIT);
}

/*!
 * @brief Release the registry mutex (no-op if it was not created).
 */
void Fs_WndList::Unlock()
{
    if (m_hmtx) DosReleaseMutexSem(m_hmtx);
}

/*!
 * @brief Allocate a new window slot.
 *
 * @param [in]  iHab      Owning HAB id.
 * @param [out] pNewHwnd  Receives the freshly assigned HWND.
 * @return Slot pointer, or NULL on failure.
 * @retval !NULL  Window allocated.
 * @retval NULL   Table full or invalid args.
 */
Fs_Wnd *Fs_WndList::Create(ULONG iHab, ULONG *pNewHwnd)
{
    Fs_Wnd *pFound = NULL;
    ULONG   iHwnd  = 0;
    int     i, nUsed = 0;

    if (pNewHwnd) *pNewHwnd = 0;

    Lock();

    for (i = 0; i < F_MAX_WINDOWS; i++)
        if (m_aWnds[i].IsUsed()) nUsed++;

    if (nUsed >= F_MAX_WINDOWS) {
        Unlock();
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[WNDLIST] limit reached (%d/%d)", nUsed, F_MAX_WINDOWS);
        return NULL;
    }

    for (i = 0; i < F_MAX_WINDOWS; i++) {
        if (m_aWnds[i].IsUsed()) continue;

        iHwnd = m_ulNextHwnd++;
        if (iHwnd == 0) iHwnd = m_ulNextHwnd++;   /* skip 0 */

        if (m_aWnds[i].Init(iHwnd, iHab)) {
            pFound = &m_aWnds[i];
            if (pNewHwnd) *pNewHwnd = iHwnd;
            break;
        }
    }
    Unlock();

    if (!pFound) {
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[WNDLIST] ERROR: table full (%d slots)", F_MAX_WINDOWS);
        return NULL;
    }

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
               "[WNDLIST] Create iHab=%lu -> hwnd=%lu",
               (unsigned long)iHab, (unsigned long)iHwnd);
    return pFound;
}

/*!
 * @brief Find a window by HWND.
 *
 * @param [in] iHwnd  Window id.
 * @return Slot pointer, or NULL if not found.
 * @retval !NULL  Matching slot.
 * @retval NULL   No such window.
 */
Fs_Wnd *Fs_WndList::Find(ULONG iHwnd)
{
    Fs_Wnd *pFound = NULL;
    int     i;

    if (iHwnd == 0) return NULL;

    Lock();
    for (i = 0; i < F_MAX_WINDOWS; i++) {
        if (m_aWnds[i].IsUsed() && m_aWnds[i].GetHwnd() == iHwnd) {
            pFound = &m_aWnds[i];
            break;
        }
    }
    Unlock();
    return pFound;
}

/*!
 * @brief Destroy one window slot (does not release HPS; caller must).
 *
 * @param [in] iHwnd  Window id.
 * @return TRUE if found and destroyed.
 * @retval TRUE   Slot freed.
 * @retval FALSE  No such window.
 */
BOOL Fs_WndList::Destroy(ULONG iHwnd)
{
    Fs_Wnd *pWnd = NULL;
    int     i;

    if (iHwnd == 0) return FALSE;

    Lock();
    for (i = 0; i < F_MAX_WINDOWS; i++) {
        if (m_aWnds[i].IsUsed() && m_aWnds[i].GetHwnd() == iHwnd) {
            pWnd = &m_aWnds[i];
            pWnd->Shutdown();
            break;
        }
    }
    Unlock();

    if (!pWnd) return FALSE;

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
               "[WNDLIST] Destroy hwnd=%lu", (unsigned long)iHwnd);
    return TRUE;
}

/*!
 * @brief Destroy all windows owned by a given HAB.
 *
 * Called from Fs_Server::TeardownThread before the HAB is destroyed.
 *
 * @param [in] iHab  Owning HAB id (0 = no-op).
 * @return Number of windows destroyed.
 * @retval 0    No windows owned by iHab, or iHab == 0 (no-op).
 * @retval >0   Count of freed slots.
 */
int Fs_WndList::DestroyByHab(ULONG iHab)
{
    int n = 0;
    int i;

    if (iHab == 0) return 0;

    Lock();
    for (i = 0; i < F_MAX_WINDOWS; i++) {
        if (m_aWnds[i].IsUsed() && m_aWnds[i].GetHab() == iHab) {
            m_aWnds[i].Shutdown();
            n++;
        }
    }
    Unlock();

    F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
               "[WNDLIST] DestroyByHab iHab=%lu -> %d windows",
               (unsigned long)iHab, n);
    return n;
}

/*!
 * @brief Count windows owned by a given HAB.
 *
 * @param [in] iHab  Owning HAB id.
 * @return Number of matching windows.
 * @retval 0    No windows owned by iHab, or iHab == 0.
 * @retval >0   Count.
 */
int Fs_WndList::CountByHab(ULONG iHab)
{
    int n = 0;
    int i;

    if (iHab == 0) return 0;

    Lock();
    for (i = 0; i < F_MAX_WINDOWS; i++) {
        if (m_aWnds[i].IsUsed() && m_aWnds[i].GetHab() == iHab)
            n++;
    }
    Unlock();
    return n;
}

/*!
 * @brief Count all active windows in the registry.
 *
 * @return Number of used slots.
 * @retval >=0  Count.
 */
int Fs_WndList::CountActive()
{
    int n = 0;
    int i;

    Lock();
    for (i = 0; i < F_MAX_WINDOWS; i++)
        if (m_aWnds[i].IsUsed()) n++;
    Unlock();
    return n;
}

/*!
 * @brief Server-side default window procedure.
 *
 * The analogue of PMWIN's WinDefWindowProc on the server side. Every
 * window in PM shares certain fundamental behaviour defined by the
 * default window procedure; FreePM houses that behaviour here. As
 * per-class Server*WndProc implementations appear (Phase 5 / D3),
 * this function will dispatch into them for standard classes and stay
 * the generic fallback for custom classes.
 *
 * Handles the small set of messages the server can meaningfully act
 * on today:
 *
 *   WM_CLOSE  destroys the target window via the WndList and returns 0.
 *
 * All other messages are accepted as "not handled" and return 0.
 *
 * @param [in] hwnd  Target window handle. 0 or an unknown value is
 *                   tolerated (returns 0).
 * @param [in] msg   Message id.
 * @param [in] mp1   First message parameter (currently unused).
 * @param [in] mp2   Second message parameter (currently unused).
 * @return Message result (MRESULT).
 * @retval 0  Message not handled, or handled with a zero result.
 */
MRESULT Fs_WndList::DefWindowProc(HWND hwnd, ULONG msg,
                                   MPARAM mp1, MPARAM mp2)
{
    int ctx;

    (void)mp1;
    (void)mp2;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[WNDLIST] DefWindowProc(hwnd=%lu, msg=0x%lx)",
                         (unsigned long)hwnd, (unsigned long)msg);

    switch (msg) {

    case WM_CLOSE:
        if (Destroy((ULONG)hwnd))
            F_DbgCtxExitf(ctx, "0 (WM_CLOSE: window destroyed)");
        else
            F_DbgCtxExitf(ctx, "0 (WM_CLOSE: hwnd not found)");
        return 0;

    default:
        F_DbgCtxExitf(ctx, "0 (unhandled)");
        return 0;
    }
}

void Fs_Wnd::SetTitle(const char *t)
{
    if (!t) { m_szTitle[0] = '\0'; return; }
    strncpy(m_szTitle, t, sizeof(m_szTitle) - 1);
    m_szTitle[sizeof(m_szTitle) - 1] = '\0';
}
