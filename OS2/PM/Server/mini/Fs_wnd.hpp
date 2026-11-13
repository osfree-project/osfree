/*!
 * @file Fs_wnd.hpp
 * @brief One server-side window + the server-wide window registry.
 *
 * Fs_Wnd is the minimal per-window record needed to hand out a PS for
 * a window (HWND -> HAB back-ref -> cached HPS). It also carries the
 * server-side window procedure: DispatchMsg dispatches a message to
 * the per-class handler, DefWindowProc is the leaf fallback.
 *
 * Fs_WndList holds F_MAX_WINDOWS slots, keyed by a monotonic HWND
 * (never reused, like idClient / iHab). Owned by Fs_Server (peer to
 * Fs_ProcessList, Fs_ConnectionList, Fs_HabList).
 *
 * Cascade: on HAB destroy, Fs_WndList::DestroyByHab(iHab) is called from
 * Fs_Server::TeardownThread, before the HAB itself goes away.
 */
#ifndef FS_WND_HPP
#define FS_WND_HPP

#define INCL_BASE
#define INCL_PM
#include "F_platform.h"
#include "Fs_config.hpp"

class Fs_WndList;

/*!
 * @brief One server-side window slot.
 *
 * Keyed by m_iHwnd (monotonic, unique). Holds a back-ref to the owning
 * HAB (m_iHab), the HDC created by WinOpenWindowDC for this window,
 * geometry, visibility, style, title, parent / Z-order / class / id,
 * and the ctlData/presParams blobs captured at creation time. The PS
 * belongs to the client.
 */
class Fs_Wnd {
public:
    Fs_Wnd();

    /*!
     * @brief Install into a FREE slot.
     * @param [in] iHwnd  Server-assigned HWND (monotonic).
     * @param [in] iHab   Owning HAB id.
     * @return TRUE on success, FALSE if slot is used or args invalid.
     * @retval TRUE   Slot populated.
     * @retval FALSE  Slot already used or iHwnd == 0.
     */
    BOOL Init(ULONG iHwnd, ULONG iHab);

    /*! @brief Release the slot (state -> FREE, HDC and blobs cleared). */
    void Shutdown();

    /* ---- Accessors ---- */
    BOOL  IsUsed()  const { return m_bUsed; }
    ULONG GetHwnd() const { return m_iHwnd; }
    ULONG GetHab()  const { return m_iHab;  }
//    HPS   GetPS()   const { return m_hPS;   }
    HDC   GetDC()   const { return m_hdc;   }

    /*! @brief Cache the HPS for this window (E3). */
//    void  SetPS(HPS hps)  { m_hPS = hps; }
    /*! @brief Clear the cached HPS reference (does not release it). */
//    void  ClearPS()       { m_hPS = NULL; }

    /*! @brief Cache the HDC for this window (created by WinOpenWindowDC). */
    void  SetDC(HDC hdc)  { m_hdc = hdc; }
    /*! @brief Clear the cached HDC reference. */
    void  ClearDC()       { m_hdc = NULL; }

    /*!
     * @brief Return the window DC, opening it on first use.
     *
     * Idempotent: returns the cached HDC if one is already attached.
     * On first call opens a display DC via GreOpenDC and sets it up
     * with the window origin and full-window clip via GreSetupDC.
     *
     * @return HDC on success, NULLHANDLE on failure.
     * @retval NULLHANDLE  GreOpenDC failed or driver rejected the DC.
     * @retval !=NULLHANDLE Valid HDC (existing or freshly opened).
     */
    HDC EnsureDC();

    /*!
     * @brief Re-apply DC origin and clip from the current rectangle.
     *
     * No-op if the DC has not been opened yet (EnsureDC will set it
     * up on first use). Used by WM_SIZE to keep the DC in sync after
     * a child window has been repositioned.
     */
    void ResetDC();

    LONG  GetX() const { return m_x; }
    LONG  GetY() const { return m_y; }
    LONG  GetCx() const { return m_cx; }
    LONG  GetCy() const { return m_cy; }
    BOOL  IsVisible() const { return m_bVisible; }
    ULONG GetStyle()  const { return m_flStyle; }
    const char *GetTitle() const { return m_szTitle; }

    void SetRect(LONG x, LONG y, LONG cx, LONG cy)
    { m_x = x; m_y = y; m_cx = cx; m_cy = cy; }
    void SetPos(LONG x, LONG y) { m_x = x; m_y = y; }
    void SetSize(LONG cx, LONG cy) { m_cx = cx; m_cy = cy; }
    void SetVisible(BOOL v) { m_bVisible = v; }
    void SetStyle(ULONG s)  { m_flStyle = s; }
    void SetTitle(const char *t);

    /*!
     * @brief Store the window class name.
     *
     * The caller (NWinCreateWindow) rejects class names longer than
     * the buffer; this setter only truncates defensively.
     *
     * @param [in] c  NUL-terminated class name (NULL clears the field).
     */
    void SetClass(const char *c);

    /* ---- Parent / Z-order / class / id ---- */
    ULONG GetParent() const { return m_iParent; }
    LONG  GetZOrder() const { return m_lZOrder; }
    const char *GetClass() const { return m_szClass; }
    ULONG GetId()     const { return m_ulId;   }

    void SetParent(ULONG p) { m_iParent = p; }
    void SetZOrder(LONG z)  { m_lZOrder = z; }
    void SetId(ULONG id)    { m_ulId = id;   }

    void SetCtlData(PBYTE p, ULONG cb)
    { if (m_pCtlData) DosFreeMem(m_pCtlData); m_pCtlData = p; m_cbCtlData = cb; }
    void SetPresParams(PBYTE p, ULONG cb)
    { if (m_pPresParams) DosFreeMem(m_pPresParams); m_pPresParams = p; m_cbPresParams = cb; }

    PBYTE GetCtlData()     const { return m_pCtlData;    }
    ULONG GetCbCtlData()   const { return m_cbCtlData;   }
    PBYTE GetPresParams()  const { return m_pPresParams; }
    ULONG GetCbPresParams()const { return m_cbPresParams;}

    /* ---- Server-side window procedure ---- */

    /*!
     * @brief Dispatch one message to the per-class handler.
     *
     * Switches on m_szClass and calls FrameWndProc / TitleBarWndProc.
     * Unknown classes fall through to DefWindowProc.
     *
     * @param [in] pOwner  Owning registry (passed through to handlers;
     *                     needed for DestroyTree on WM_CLOSE and for
     *                     WM_SIZE child recalculation).
     * @param [in] msg     Message id.
     * @param [in] mp1     First message parameter.
     * @param [in] mp2     Second message parameter.
     * @return Message result (MRESULT).
     * @retval 0  Message not handled, or handled with a zero result.
     */
    MRESULT DispatchMsg(Fs_WndList *pOwner, ULONG msg,
                        MPARAM mp1, MPARAM mp2);

    /*!
     * @brief Frame (WC_FRAME) window procedure.
     *
     * WM_PAINT  draws the frame boundary around the client area.
     * WM_SIZE   repositions FID_TITLEBAR (top strip) and FID_CLIENT
     *           (remaining area) via pOwner->WindowFromID, then
     *           calls ResetDC on each so the DC origin follows.
     * WM_CLOSE  destroys the window tree via pOwner->DestroyTree.
     * All other messages fall through to DefWindowProc.
     *
     * @param [in] pOwner  Owning registry.
     * @param [in] msg     Message id.
     * @param [in] mp1     First message parameter.
     * @param [in] mp2     Second message parameter.
     * @return Message result (MRESULT).
     * @retval 0  Handled with a zero result, or not handled.
     */
    MRESULT FrameWndProc(Fs_WndList *pOwner, ULONG msg,
                         MPARAM mp1, MPARAM mp2);

    /*!
     * @brief Title bar (WC_TITLEBAR) window procedure.
     *
     * WM_PAINT  fills the background and draws the close cross.
     * All other messages fall through to DefWindowProc.
     *
     * @param [in] pOwner  Owning registry.
     * @param [in] msg     Message id.
     * @param [in] mp1     First message parameter.
     * @param [in] mp2     Second message parameter.
     * @return Message result (MRESULT).
     * @retval 0  Handled with a zero result, or not handled.
     */
    MRESULT TitleBarWndProc(Fs_WndList *pOwner, ULONG msg,
                            MPARAM mp1, MPARAM mp2);

    /*!
     * @brief Server-side default window procedure (leaf).
     *
     * Analogue of PMWIN's WinDefWindowProc for the server side. Every
     * window in PM shares certain fundamental behaviour defined by the
     * default window procedure; FreePM houses that behaviour here.
     * This is a leaf: it does not dispatch into any per-class handler.
     *
     * Handles the small set of messages the server can meaningfully
     * act on today:
     *
     *   WM_CLOSE  destroys the target window tree via
     *             pOwner->DestroyTree and returns 0.
     *
     * All other messages are accepted as "not handled" and return 0.
     *
     * @param [in] pOwner  Owning registry.
     * @param [in] msg     Message id.
     * @param [in] mp1     First message parameter.
     * @param [in] mp2     Second message parameter.
     * @return Message result (MRESULT).
     * @retval 0  Message not handled, or handled with a zero result.
     */
    MRESULT DefWindowProc(Fs_WndList *pOwner, ULONG msg,
                          MPARAM mp1, MPARAM mp2);

private:
    BOOL  m_bUsed;
    ULONG m_iHwnd;
    ULONG m_iHab;
//    HPS   m_hPS;
    HDC   m_hdc;

    LONG  m_x, m_y, m_cx, m_cy;
    BOOL  m_bVisible;
    ULONG m_flStyle;
    char  m_szTitle[64];

    /*! @brief Parent HWND (0 = none; HWND_DESKTOP for top-level frames). */
    ULONG m_iParent;

    /*! @brief Z-order rank among siblings. Higher = nearer the top. */
    LONG  m_lZOrder;

    /*! @brief Window class name (NUL-terminated, at most 31 chars). */
    char  m_szClass[32];

    /*! @brief Window id (FID_TITLEBAR / FID_CLIENT / user id). */
    ULONG m_ulId;

    PBYTE m_pCtlData;    ULONG m_cbCtlData;
    PBYTE m_pPresParams; ULONG m_cbPresParams;
};

/*!
 * @brief Registry of Fs_Wnd slots, serialized by an OS/2 mutex.
 *
 * HWND is a monotonic id (never reused). The array slot itself may be
 * reused after Destroy, but the old HWND value will never be handed
 * out again.
 */
class Fs_WndList {
public:
    Fs_WndList();
    ~Fs_WndList();

    /*!
     * @brief Allocate a new window slot.
     * @param [in]  iHab      Owning HAB id.
     * @param [out] pNewHwnd  Receives the freshly assigned HWND.
     * @return Slot pointer, or NULL on failure.
     * @retval !NULL  Window allocated.
     * @retval NULL   Table full or invalid args.
     */
    Fs_Wnd *Create(ULONG iHab, ULONG *pNewHwnd);

    /*!
     * @brief Find a window by HWND.
     * @param [in] iHwnd  Window id.
     * @return Slot pointer, or NULL if not found.
     * @retval !NULL  Matching slot.
     * @retval NULL   No such window.
     */
    Fs_Wnd *Find(ULONG iHwnd);

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
    Fs_Wnd *WindowFromID(ULONG hwndParent, ULONG id);

    /*!
     * @brief Destroy one window slot; closes its HDC if one was opened.
     * @param [in] iHwnd  Window id.
     * @return TRUE if found and destroyed.
     * @retval TRUE   Slot freed.
     * @retval FALSE  No such window.
     */
    BOOL Destroy(ULONG iHwnd);

    /*!
     * @brief Destroy all windows owned by a given HAB.
     *
     * Called from Fs_Server::TeardownThread before the HAB is destroyed.
     *
     * @param [in] iHab  Owning HAB id (0 = no-op).
     * @return Number of windows destroyed.
     * @retval >=0  Count of freed slots.
     */
    int DestroyByHab(ULONG iHab);

    /*!
     * @brief Count windows owned by a given HAB.
     *
     * @param [in] iHab  Owning HAB id.
     * @return Number of matching windows.
     * @retval >=0  Count.
     */
    int CountByHab(ULONG iHab);

    /*! @brief Count all active windows in the registry.
        @return Number of used slots.
        @retval >=0  Count. */
    int CountActive();

    /*!
     * @brief Place hwnd in the sibling Z-order.
     *
     * Per OS/2 PM (WinCreateWindow / WinSetWindowPos):
     *   HWND_TOP     — place hwnd on top of all its siblings.
     *   HWND_BOTTOM  — place hwnd behind all its siblings.
     *   otherwise    — hwndInsertBehind must be a sibling of hwnd;
     *                  hwnd is placed immediately behind it
     *                  (the named sibling stays above hwnd).
     *
     * If hwndInsertBehind is not a valid sibling of hwnd, the call
     * is a no-op (PM returns PMERR_INVALID_HWND).
     *
     * @param [in] hwnd              Window to move (0 = no-op).
     * @param [in] hwndInsertBehind  HWND_TOP, HWND_BOTTOM, or a sibling.
     */
    void PlaceWindow(ULONG hwnd, ULONG hwndInsertBehind);

    /*!
     * @brief Destroy hwnd and all descendants (m_iParent chain).
     *
     * The subtree is collected under one lock acquisition, then
     * destroyed outside the lock, children before parent (matches OS/2
     * PM: every window is destroyed before its parent).
     *
     * @param [in] hwnd  Window handle (0 = no-op).
     */
    void DestroyTree(ULONG hwnd);

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
    MRESULT DispatchMsg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

private:
    void Lock();
    void Unlock();

    Fs_Wnd m_aWnds[F_MAX_WINDOWS];
    ULONG  m_ulNextHwnd;
    HMTX   m_hmtx;
};

#endif /* FS_WND_HPP */
