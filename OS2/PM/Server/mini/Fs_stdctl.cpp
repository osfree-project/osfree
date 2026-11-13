/*!
 * @file Fs_stdctl.cpp
 * @brief Standard window-class procedures (WC_FRAME, WC_TITLEBAR) and
 *        the per-window message dispatcher.
 *
 * Fs_Wnd::DispatchMsg switches on the window class name and calls the
 * matching per-class handler; Fs_Wnd::DefWindowProc is the leaf
 * fallback.
 *
 * The class names are still strings ("WC_FRAME", "WC_TITLEBAR"); the
 * real OS/2 PM classes are atoms (0xFFFFxxxx). Switching to atoms is
 * planned; until then, NWinCreateWindow and the client carry the
 * string form.
 *
 * No text yet: the title bar draws its background and the close cross,
 * not the title string (the font path is a separate step).
 * No mouse: WM_BUTTONDOWN / WM_MOUSEMOVE / WM_BUTTONUP are not handled
 * here yet (drag & drop and the close-button hit test are later).
 */
#include "Fs_wnd.hpp"
#include "F_debug.h"

#define INCL_GREALL
#include "pmddi.h"

#include <string.h>

// @todo Make via SV_CYTITLEBAR!
/*! @brief Title bar height in pels (fallback for SV_CYTITLEBAR). */
#define F_TITLEBAR_HEIGHT 20L

/*!
 * @brief Set the foreground line-bundle colour on a DC.
 *
 * gre2sdl interprets lColor as raw 0x00BBGGRR (see Os2ColorToRgb in
 * gre2sdl.cpp), not as a PM CLR_* logical index. Callers therefore
 * pass packed RGB values, not CLR_* constants.
 *
 * @param [in] hdc     Device context.
 * @param [in] lColor  Colour in 0x00BBGGRR.
 */
static void SrvSetColor(HDC hdc, LONG lColor)
{
    LINEBUNDLE lb;
    memset(&lb, 0, sizeof(lb));
    lb.lColor = lColor;
    GreDeviceSetAttributes(hdc, PRIM_LINE, LBB_COLOR, 0L, &lb);
}

/*!
 * @brief Fill the whole window rectangle with lColor.
 *
 * @param [in] hdc     Device context.
 * @param [in] cx      Width in pels.
 * @param [in] cy      Height in pels.
 * @param [in] lColor  Colour in 0x00BBGGRR.
 */
static void SrvFillWindow(HDC hdc, LONG cx, LONG cy, LONG lColor)
{
    POINTL    ptl00;
    BOXPARAMS box;

    SrvSetColor(hdc, lColor);

    ptl00.x = 0;
    ptl00.y = 0;
    GreSetCurrentPosition(hdc, &ptl00);

    box.ptl.x   = cx;
    box.ptl.y   = cy;
    box.sizl.cx = 0;
    box.sizl.cy = 0;
    GreBoxInterior(hdc, &box);
}

/*!
 * @brief Outline the whole window rectangle with lColor.
 *
 * @param [in] hdc     Device context.
 * @param [in] cx      Width in pels.
 * @param [in] cy      Height in pels.
 * @param [in] lColor  Colour in 0x00BBGGRR.
 */
static void SrvFrameWindow(HDC hdc, LONG cx, LONG cy, LONG lColor)
{
    POINTL    ptl00;
    BOXPARAMS box;

    SrvSetColor(hdc, lColor);

    ptl00.x = 0;
    ptl00.y = 0;
    GreSetCurrentPosition(hdc, &ptl00);

    box.ptl.x   = cx;
    box.ptl.y   = cy;
    box.sizl.cx = 0;
    box.sizl.cy = 0;
    GreBoxBoundary(hdc, &box);
}

/*!
 * @brief Draw one straight line, colour already set by the caller.
 *
 * @param [in] hdc  Device context.
 * @param [in] x0   Start X.
 * @param [in] y0   Start Y.
 * @param [in] x1   End X.
 * @param [in] y1   End Y.
 */
static void SrvDrawLine(HDC hdc, LONG x0, LONG y0, LONG x1, LONG y1)
{
    POINTL a[2];

    a[0].x = x0; a[0].y = y0;
    a[1].x = x1; a[1].y = y1;
    GrePolyLine(hdc, a, 2);
}

/* ======================================================================== */
/*  Fs_Wnd::DispatchMsg                                                     */
/* ======================================================================== */

/*!
 * @brief Dispatch one message to the per-class handler.
 *
 * Switches on m_szClass and calls FrameWndProc / TitleBarWndProc.
 * Unknown classes fall through to DefWindowProc.
 *
 * @param [in] pOwner  Owning registry (passed through to handlers;
 *                     needed for DestroyTree on WM_CLOSE).
 * @param [in] msg     Message id.
 * @param [in] mp1     First message parameter.
 * @param [in] mp2     Second message parameter.
 * @return Message result (MRESULT).
 * @retval 0  Message not handled, or handled with a zero result.
 */
MRESULT Fs_Wnd::DispatchMsg(Fs_WndList *pOwner, ULONG msg,
                            MPARAM mp1, MPARAM mp2)
{
    int ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[WND] DispatchMsg(hwnd=%lu, class=%s, msg=0x%lx)",
                         (unsigned long)m_iHwnd, m_szClass,
                         (unsigned long)msg);

    if (strcmp(m_szClass, "WC_FRAME") == 0) {
        MRESULT mres = FrameWndProc(pOwner, msg, mp1, mp2);
        F_DbgCtxExitf(ctx, "0x%lx (frame)", (unsigned long)mres);
        return mres;
    }
    if (strcmp(m_szClass, "WC_TITLEBAR") == 0) {
        MRESULT mres = TitleBarWndProc(pOwner, msg, mp1, mp2);
        F_DbgCtxExitf(ctx, "0x%lx (titlebar)", (unsigned long)mres);
        return mres;
    }

    {
        MRESULT mres = DefWindowProc(pOwner, msg, mp1, mp2);
        F_DbgCtxExitf(ctx, "0x%lx (default)", (unsigned long)mres);
        return mres;
    }
}

/* ======================================================================== */
/*  Fs_Wnd::FrameWndProc                                                    */
/* ======================================================================== */

/*!
 * @brief Frame (WC_FRAME) window procedure.
 *
 * WM_PAINT  outlines the window rectangle (frame border). The client
 *           area and title bar are separate child windows and paint
 *           themselves.
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
MRESULT Fs_Wnd::FrameWndProc(Fs_WndList *pOwner, ULONG msg,
                             MPARAM mp1, MPARAM mp2)
{
    int ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[WND] FrameWndProc(hwnd=%lu, msg=0x%lx)",
                         (unsigned long)m_iHwnd, (unsigned long)msg);

    switch (msg) {

    case WM_PAINT: {
        HDC hdc = EnsureDC();
        if (hdc == NULLHANDLE) {
            F_DbgCtxExitf(ctx, "0 (no DC)");
            return 0;
        }
        SrvFrameWindow(hdc, m_cx, m_cy, 0x00808080L);
        F_DbgCtxExitf(ctx, "0 (painted %ldx%ld)", (long)m_cx, (long)m_cy);
        return 0;
    }

    case WM_SIZE: {
        Fs_Wnd *pTitle  = pOwner
                        ? pOwner->WindowFromID(m_iHwnd, FID_TITLEBAR)
                        : NULL;
        Fs_Wnd *pClient = pOwner
                        ? pOwner->WindowFromID(m_iHwnd, FID_CLIENT)
                        : NULL;
        LONG    tbH     = F_TITLEBAR_HEIGHT;

        /* Safety: never make the title bar taller than the frame. */
        if (tbH > m_cy) tbH = m_cy;

        if (pTitle) {
            pTitle->SetRect(m_x, m_y + m_cy - tbH, m_cx, tbH);
            pTitle->ResetDC();
        }
        if (pClient) {
            pClient->SetRect(m_x, m_y, m_cx, m_cy - tbH);
            pClient->ResetDC();
        }

        F_DbgCtxExitf(ctx, "0 (WM_SIZE: %ldx%ld, tbH=%ld)",
                      (long)m_cx, (long)m_cy, (long)tbH);
        return 0;
    }

    case WM_CLOSE:
        if (pOwner) pOwner->DestroyTree(m_iHwnd);
        F_DbgCtxExitf(ctx, "0 (WM_CLOSE)");
        return 0;

    default:
        break;
    }

    {
        MRESULT mres = DefWindowProc(pOwner, msg, mp1, mp2);
        F_DbgCtxExitf(ctx, "0x%lx (default)", (unsigned long)mres);
        return mres;
    }
}

/* ======================================================================== */
/*  Fs_Wnd::TitleBarWndProc                                                 */
/* ======================================================================== */

/*!
 * @brief Title bar (WC_TITLEBAR) window procedure.
 *
 * WM_PAINT  fills the background and draws the close cross at the
 *           right end. No title text yet — the font path is a
 *           separate step.
 * All other messages fall through to DefWindowProc.
 *
 * @param [in] pOwner  Owning registry.
 * @param [in] msg     Message id.
 * @param [in] mp1     First message parameter.
 * @param [in] mp2     Second message parameter.
 * @return Message result (MRESULT).
 * @retval 0  Handled with a zero result, or not handled.
 */
MRESULT Fs_Wnd::TitleBarWndProc(Fs_WndList *pOwner, ULONG msg,
                                MPARAM mp1, MPARAM mp2)
{
    int ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[WND] TitleBarWndProc(hwnd=%lu, msg=0x%lx)",
                         (unsigned long)m_iHwnd, (unsigned long)msg);

    switch (msg) {

    case WM_PAINT: {
        HDC   hdc;
        LONG  cx, cy, s, x0, y0, x1, y1;

        hdc = EnsureDC();
        if (hdc == NULLHANDLE) {
            F_DbgCtxExitf(ctx, "0 (no DC)");
            return 0;
        }
        cx = m_cx;
        cy = m_cy;

        /* Background. */
        SrvFillWindow(hdc, cx, cy, 0x00C08040L);

        /* Close cross at the right end of the bar. Square box of side
         * s; right/top/bottom margin 2 pels. */
        s = cy - 2 * 2L;
        if (s < 4) s = 4;
        x0 = cx - s - 2L;
        if (x0 < 2L) x0 = 2L;
        y0 = 2L;
        x1 = x0 + s;
        y1 = y0 + s;

        SrvSetColor(hdc, 0x00FFFFFFL);
        SrvDrawLine(hdc, x0, y0, x1, y1);
        SrvDrawLine(hdc, x1, y0, x0, y1);

        F_DbgCtxExitf(ctx, "0 (painted %ldx%ld, cross %ld..%ld,%ld..%ld)",
                      (long)cx, (long)cy,
                      (long)x0, (long)x1, (long)y0, (long)y1);
        return 0;
    }

    default:
        break;
    }

    {
        MRESULT mres = DefWindowProc(pOwner, msg, mp1, mp2);
        F_DbgCtxExitf(ctx, "0x%lx (default)", (unsigned long)mres);
        return mres;
    }
}

/* ======================================================================== */
/*  Fs_Wnd::DefWindowProc                                                   */
/* ======================================================================== */

/*!
 * @brief Server-side default window procedure (leaf).
 *
 * Analogue of PMWIN's WinDefWindowProc for the server side. Every
 * window in PM shares certain fundamental behaviour defined by the
 * default window procedure; FreePM houses that behaviour here. This is
 * a leaf: it does not dispatch into any per-class handler.
 *
 * Handles the small set of messages the server can meaningfully act
 * on today:
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
MRESULT Fs_Wnd::DefWindowProc(Fs_WndList *pOwner, ULONG msg,
                              MPARAM mp1, MPARAM mp2)
{
    int ctx;

    (void)mp1;
    (void)mp2;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_FS_SERVER, F_DBG_LVL_VERBOSE,
                         "[WND] DefWindowProc(hwnd=%lu, msg=0x%lx)",
                         (unsigned long)m_iHwnd, (unsigned long)msg);

    switch (msg) {

    case WM_CLOSE:
        if (pOwner) pOwner->DestroyTree(m_iHwnd);
        F_DbgCtxExitf(ctx, "0 (WM_CLOSE)");
        return 0;

    default:
        F_DbgCtxExitf(ctx, "0 (unhandled)");
        return 0;
    }
}
