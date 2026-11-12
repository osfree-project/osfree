/*!
 * @file Fs_wnd.hpp
 * @brief One server-side window + the server-wide window registry.
 *
 * Fs_Wnd is the minimal per-window record needed to hand out a PS for
 * a window (HWND -> HAB back-ref -> cached HPS). It does not yet carry
 * geometry, parent/child links, WndProc, message queue, or class — those
 * come with Phase 5.
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

#include "F_platform.h"
#include "Fs_config.hpp"

/*!
 * @brief One server-side window slot.
 *
 * Keyed by m_iHwnd (monotonic, unique). Holds a back-ref to the owning
 * HAB (m_iHab), the HDC created by WinOpenWindowDC for this window,
 * geometry, visibility, style, title, and the ctlData/presParams
 * blobs captured at creation time. The PS belongs to the client.
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

    void SetCtlData(PBYTE p, ULONG cb)
    { if (m_pCtlData) DosFreeMem(m_pCtlData); m_pCtlData = p; m_cbCtlData = cb; }
    void SetPresParams(PBYTE p, ULONG cb)
    { if (m_pPresParams) DosFreeMem(m_pPresParams); m_pPresParams = p; m_cbPresParams = cb; }

    PBYTE GetCtlData()     const { return m_pCtlData;    }
    ULONG GetCbCtlData()   const { return m_cbCtlData;   }
    PBYTE GetPresParams()  const { return m_pPresParams; }
    ULONG GetCbPresParams()const { return m_cbPresParams;}

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
     * @brief Server-side default window procedure.
     *
     * Analogue of PMWIN's WinDefWindowProc for the server side: every
     * window in PM shares certain fundamental behaviour defined by the
     * default window procedure, and FreePM houses that behaviour here
     * (until per-class Server*WndProc implementations appear in
     * Phase 5 / D3, at which point DefWindowProc will dispatch into
     * them for standard classes and stay the generic fallback for
     * custom classes).
     *
     * Called from NWinDispatchMsg in place of a real client window
     * procedure. Handles the small set of messages the server can
     * meaningfully act on today (WM_CLOSE > Destroy(hwnd)); all other
     * messages are accepted as "not handled" and return 0.
     *
     * @param [in] hwnd  Target window handle. May be 0 or unknown.
     * @param [in] msg   Message id.
     * @param [in] mp1   First message parameter.
     * @param [in] mp2   Second message parameter.
     * @return Message result (MRESULT).
     * @retval 0  Message not handled, or handled with a zero result.
     */
    MRESULT DefWindowProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

private:
    void Lock();
    void Unlock();

    Fs_Wnd m_aWnds[F_MAX_WINDOWS];
    ULONG  m_ulNextHwnd;
    HMTX   m_hmtx;
};

#endif /* FS_WND_HPP */
