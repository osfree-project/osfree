/*!
 * @file Fs_wnd.cpp
 * @brief Fs_Wnd and Fs_WndList implementation.
 */
#include "Fs_wnd.hpp"
#include "F_debug.h"

#define INCL_GREALL
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
      m_iParent(0), m_lZOrder(0), m_ulId(0),
      m_pCtlData(NULL), m_cbCtlData(0),
      m_pPresParams(NULL), m_cbPresParams(0)
{
    m_szTitle[0] = '\0';
    m_szClass[0] = '\0';
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
    m_szClass[0] = '\0';
    m_iParent  = 0;
    m_lZOrder  = 0;
    m_ulId     = 0;
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
    m_szClass[0] = '\0';
    m_iParent  = 0;
    m_lZOrder  = 0;
    m_ulId     = 0;
}

/*!
 * @brief Return the window DC, opening it on first use.
 *
 * Opens a display DC via GreOpenDC(OD_DIRECT) and sets it up with the
 * window origin and full-window clip via GreSetupDC. Idempotent: the
 * cached HDC is returned on subsequent calls.
 *
 * @return HDC on success, NULLHANDLE on failure.
 * @retval NULLHANDLE   GreOpenDC failed or driver rejected the DC.
 * @retval !=NULLHANDLE Valid HDC (existing or freshly opened).
 */
HDC Fs_Wnd::EnsureDC()
{
    HDC hdc;
    int ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[WND] EnsureDC(hwnd=%lu)",
                         (unsigned long)m_iHwnd);

    hdc = m_hdc;
    if (hdc != NULLHANDLE) {
        F_DbgCtxExitf(ctx, "HDC=%lu (existing)", (unsigned long)hdc);
        return hdc;
    }

    {
        PSZ pdop[2] = { (PSZ)"DISPLAY", (PSZ)"GRE2SDL" };
        hdc = GreOpenDC(NULLHANDLE, OD_DIRECT, (PSZ)"*", 2,
                        (PDEVOPENDATA)pdop);
    }
    if (hdc == NULLHANDLE || (ULONG)hdc == (ULONG)-1) {
        F_DbgCtxExitf(ctx, "NULLHANDLE (GreOpenDC failed, hdc=0x%lx)",
                      (unsigned long)hdc);
        return NULLHANDLE;
    }

    m_hdc = hdc;

    {
        RECTL rcl;
        rcl.xLeft   = 0;
        rcl.yBottom = 0;
        rcl.xRight  = m_cx;
        rcl.yTop    = m_cy;
        GreSetupDC(hdc, 0,
                   (LONG)m_x, (LONG)m_y,
                   &rcl,
                   SETUPDC_VISRGN | SETUPDC_ORIGIN);
    }

    F_DbgCtxExitf(ctx, "HDC=%lu (new)", (unsigned long)hdc);
    return hdc;
}

/*!
 * @brief Re-apply DC origin and clip from the current rectangle.
 *
 * No-op if the DC has not been opened yet (EnsureDC will set it up
 * on first use). Used by WM_SIZE to keep the DC in sync after a child
 * window has been repositioned.
 */
void Fs_Wnd::ResetDC()
{
    RECTL rcl;

    if (m_hdc == NULLHANDLE) return;

    rcl.xLeft   = 0;
    rcl.yBottom = 0;
    rcl.xRight  = m_cx;
    rcl.yTop    = m_cy;
    GreSetupDC(m_hdc, 0,
               (LONG)m_x, (LONG)m_y,
               &rcl,
               SETUPDC_VISRGN | SETUPDC_ORIGIN);
}

void Fs_Wnd::SetTitle(const char *t)
{
    if (!t) { m_szTitle[0] = '\0'; return; }
    strncpy(m_szTitle, t, sizeof(m_szTitle) - 1);
    m_szTitle[sizeof(m_szTitle) - 1] = '\0';
}

/*!
 * @brief Store the window class name.
 *
 * The caller (NWinCreateWindow) rejects class names longer than the
 * buffer; this setter only truncates defensively.
 *
 * @param [in] c  NUL-terminated class name (NULL clears the field).
 */
void Fs_Wnd::SetClass(const char *c)
{
    if (!c) { m_szClass[0] = '\0'; return; }
    strncpy(m_szClass, c, sizeof(m_szClass) - 1);
    m_szClass[sizeof(m_szClass) - 1] = '\0';
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
 * @brief Find a child window of parent by id.
 *
 * Per OS/2 PM WinWindowFromID: returns the window whose parent is
 * hwndParent and whose id equals id. If several siblings share the
 * same id, behaviour is unspecified in PM; here the first slot in
 * registration order is returned.
 *
 * @param [in] hwndParent  Parent HWND.
 * @param [in] id          Window id (e.g. FID_TITLEBAR / FID_CLIENT).
 * @return Slot pointer, or NULL if not found.
 * @retval !NULL  Matching slot.
 * @retval NULL   No child of hwndParent with this id.
 */
Fs_Wnd *Fs_WndList::WindowFromID(ULONG hwndParent, ULONG id)
{
    Fs_Wnd *pFound = NULL;
    int     i;

    if (hwndParent == 0) return NULL;

    Lock();
    for (i = 0; i < F_MAX_WINDOWS; i++) {
        if (m_aWnds[i].IsUsed() &&
            m_aWnds[i].GetParent() == hwndParent &&
            m_aWnds[i].GetId()     == id) {
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
 * @brief Place hwnd in the sibling Z-order.
 *
 * Sibling set = all used windows sharing hwnd's parent (hwnd itself
 * excluded). The set is sorted by current Z, and hwnd is reinserted:
 *
 *   HWND_TOP     — first element (topmost sibling).
 *   HWND_BOTTOM  — last element (bottom-most sibling).
 *   otherwise    — immediately after hwndInsertBehind (the named
 *                  sibling stays above hwnd; any siblings that were
 *                  below hwndInsertBehind end up below hwnd).
 *
 * If hwndInsertBehind is not HWND_TOP, not HWND_BOTTOM, and not a
 * sibling of hwnd, the call is a no-op.
 *
 * Ranks are renumbered 1..N in the new top-to-bottom order.
 *
 * @param [in] hwnd              Window to move (0 = no-op).
 * @param [in] hwndInsertBehind  HWND_TOP, HWND_BOTTOM, or a sibling.
 */
void Fs_WndList::PlaceWindow(ULONG hwnd, ULONG hwndInsertBehind)
{
    int   hwndSlot = -1;
    ULONG parent;
    int   sibSlot[F_MAX_WINDOWS];
    LONG  sibZ[F_MAX_WINDOWS];
    int   nSib = 0;
    int   pos;
    int   i, j;
    int   ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[WNDLIST] PlaceWindow(hwnd=%lu, behind=%lu)",
                         (unsigned long)hwnd,
                         (unsigned long)hwndInsertBehind);

    if (hwnd == 0) {
        F_DbgCtxExitf(ctx, "no-op (hwnd=0)");
        return;
    }

    Lock();

    /* Locate hwnd's slot and its parent. */
    for (i = 0; i < F_MAX_WINDOWS; i++) {
        if (m_aWnds[i].IsUsed() && m_aWnds[i].GetHwnd() == hwnd) {
            hwndSlot = i;
            break;
        }
    }
    if (hwndSlot < 0) {
        Unlock();
        F_DbgCtxExitf(ctx, "no-op (hwnd not found)");
        return;
    }
    parent = m_aWnds[hwndSlot].GetParent();

    /* Collect siblings (same parent, not hwnd). */
    for (i = 0; i < F_MAX_WINDOWS; i++) {
        if (!m_aWnds[i].IsUsed())          continue;
        if (i == hwndSlot)                 continue;
        if (m_aWnds[i].GetParent() != parent) continue;
        sibSlot[nSib] = i;
        sibZ[nSib]    = m_aWnds[i].GetZOrder();
        nSib++;
    }

    /* Sort siblings by Z, descending (topmost first). */
    for (i = 0; i < nSib - 1; i++) {
        for (j = 0; j < nSib - 1 - i; j++) {
            if (sibZ[j] < sibZ[j + 1]) {
                int  ts = sibSlot[j]; sibSlot[j] = sibSlot[j + 1]; sibSlot[j + 1] = ts;
                LONG tz = sibZ[j];    sibZ[j]    = sibZ[j + 1];    sibZ[j + 1]    = tz;
            }
        }
    }

    /* Decide the insertion position (0 = before all siblings). */
    if (hwndInsertBehind == (ULONG)HWND_TOP) {
        pos = 0;
    } else if (hwndInsertBehind == (ULONG)HWND_BOTTOM) {
        pos = nSib;
    } else {
        pos = -1;
        for (i = 0; i < nSib; i++) {
            if (m_aWnds[sibSlot[i]].GetHwnd() == hwndInsertBehind) {
                pos = i + 1;   /* sibling stays above; hwnd goes below it */
                break;
            }
        }
        if (pos < 0) {
            Unlock();
            F_DbgCtxExitf(ctx, "no-op (behind=%lu not a sibling)",
                          (unsigned long)hwndInsertBehind);
            return;
        }
    }

    /* Renumber: top-to-bottom ranks N..1.  Siblings before pos keep
     * their relative order above hwnd; siblings from pos onwards end
     * up below hwnd. */
    for (i = 0; i < pos; i++)
        m_aWnds[sibSlot[i]].SetZOrder((LONG)(nSib + 1 - i));
    m_aWnds[hwndSlot].SetZOrder((LONG)(nSib + 1 - pos));
    for (i = pos; i < nSib; i++)
        m_aWnds[sibSlot[i]].SetZOrder((LONG)(nSib - i));

    Unlock();

    F_DbgCtxExitf(ctx, "pos=%d of %d sibling(s)", pos, nSib);
}

/*!
 * @brief Destroy hwnd and all descendants (m_iParent chain).
 *
 * The subtree is collected under one lock acquisition, then destroyed
 * outside the lock, children before parent. This matches OS/2 PM:
 * every window is destroyed before its parent.
 *
 * @param [in] hwnd  Window handle (0 = no-op).
 */
void Fs_WndList::DestroyTree(ULONG hwnd)
{
    ULONG stack[F_MAX_WINDOWS];
    ULONG toDestroy[F_MAX_WINDOWS];
    int   nStack   = 0;
    int   nDestroy = 0;
    int   i;
    int   ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[WNDLIST] DestroyTree(hwnd=%lu)",
                         (unsigned long)hwnd);

    if (hwnd == 0) {
        F_DbgCtxExitf(ctx, "no-op (hwnd=0)");
        return;
    }

    Lock();
    stack[nStack++] = hwnd;
    while (nStack > 0) {
        ULONG cur = stack[--nStack];
        if (nDestroy < F_MAX_WINDOWS)
            toDestroy[nDestroy++] = cur;
        for (i = 0; i < F_MAX_WINDOWS; i++) {
            if (m_aWnds[i].IsUsed() &&
                m_aWnds[i].GetParent() == cur &&
                nStack < F_MAX_WINDOWS) {
                stack[nStack++] = m_aWnds[i].GetHwnd();
            }
        }
    }
    Unlock();

    /* Reverse order: children before parent. */
    for (i = nDestroy - 1; i >= 0; i--)
        Destroy(toDestroy[i]);

    F_DbgCtxExitf(ctx, "%d window(s) destroyed", nDestroy);
}

/*!
 * @brief Route one message to its target window.
 *
 * Finds the slot by hwnd and delegates to Fs_Wnd::DispatchMsg.
 * Unknown hwnd is accepted and returns 0.
 *
 * @param [in] hwnd  Target window handle. May be 0 or unknown.
 * @param [in] msg   Message id.
 * @param [in] mp1   First message parameter.
 * @param [in] mp2   Second message parameter.
 * @return Message result (MRESULT).
 * @retval 0  hwnd unknown, or the handler returned 0.
 */
MRESULT Fs_WndList::DispatchMsg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    Fs_Wnd *pWnd;
    MRESULT mres;
    int     ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[WNDLIST] DispatchMsg(hwnd=%lu, msg=0x%lx)",
                         (unsigned long)hwnd, (unsigned long)msg);

    pWnd = Find((ULONG)hwnd);
    if (!pWnd) {
        F_DbgCtxExitf(ctx, "0 (hwnd not found)");
        return 0;
    }

    mres = pWnd->DispatchMsg(this, msg, mp1, mp2);

    F_DbgCtxExitf(ctx, "0x%lx (class=%s)",
                  (unsigned long)mres, pWnd->GetClass());
    return mres;
}
