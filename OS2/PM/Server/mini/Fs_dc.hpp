/*!
 * @file Fs_dc.hpp
 * @brief One server-side device context + the server-wide DC registry.
 *
 * Fs_Dc is the server-side mirror of a presentation device context
 * (HDC): it owns the target pixel buffer (m_pTarget, usually
 * pVBuffmem), the geometry, the current foreground color, and the
 * current position. The m_iHdc field is an opaque index into the
 * Fs_DcList; on the wire it is passed as (HDC)m_iHdc.
 *
 * Fs_DcList holds F_MAX_DCS slots, keyed by a monotonic HDC (never
 * reused). Owned by Fs_Server (peer to Fs_ProcessList,
 * Fs_ConnectionList, Fs_HabList, Fs_WndList).
 *
 * Cascade: on HAB destroy, Fs_DcList::DestroyByHab(iHab) is called
 * from Fs_Server::TeardownThread, before windows and the HAB itself
 * go away.
 */
#ifndef FS_DC_HPP
#define FS_DC_HPP

#include "F_platform.h"
#include "Fs_config.hpp"

/*!
 * @brief One server-side device context slot.
 *
 * Holds geometry and pixel buffer for the target, plus minimal
 * per-DC rendering state (current color and current position) that
 * the GRE tier-1 implementation (E5) will read/write.
 */
class Fs_Dc {
public:
    Fs_Dc();

    /*!
     * @brief Install a new DC into a FREE slot.
     * @param [in] iHdc     Server-assigned HDC (monotonic, non-zero).
     * @param [in] iHwnd    Owning window id.
     * @param [in] iHab     Owning HAB id.
     * @param [in] pTarget  Target pixel buffer (may be NULL).
     * @param [in] width    Buffer width in pixels.
     * @param [in] height   Buffer height in pixels.
     * @param [in] bpp      Bytes per pixel.
     * @return TRUE on success, FALSE if slot is used or iHdc == 0.
     * @retval TRUE   Slot populated.
     * @retval FALSE  Slot already used or iHdc == 0.
     */
    BOOL Init(ULONG iHdc, ULONG iHwnd, ULONG iHab,
              PBYTE pTarget, LONG width, LONG height, LONG bpp);

    /*! @brief Release the slot (state -> FREE, all fields cleared). */
    void Shutdown();

    /* ---- Accessors ---- */
    BOOL   IsUsed()     const { return m_bUsed; }
    ULONG  GetHdc()     const { return m_iHdc; }
    ULONG  GetHwnd()    const { return m_iHwnd; }
    ULONG  GetHab()     const { return m_iHab; }
    PBYTE  GetTarget()  const { return m_pTarget; }
    LONG   GetWidth()   const { return m_width; }
    LONG   GetHeight()  const { return m_height; }
    LONG   GetBpp()     const { return m_bpp; }
    LONG   GetColor()   const { return m_color; }
    POINTL GetCurPos()  const { return m_curPos; }

    /* ---- Mutators (used by GRE tier-1 in E5) ---- */
    void SetTarget(PBYTE p)                { m_pTarget = p; }
    void SetColor(LONG c)                  { m_color = c; }
    void SetCurPos(const POINTL &pt)       { m_curPos = pt; }

private:
    BOOL   m_bUsed;
    ULONG  m_iHdc;
    ULONG  m_iHwnd;
    ULONG  m_iHab;
    PBYTE  m_pTarget;
    LONG   m_width;
    LONG   m_height;
    LONG   m_bpp;
    LONG   m_color;
    POINTL m_curPos;
};

/*!
 * @brief Registry of Fs_Dc slots, serialized by an OS/2 mutex.
 *
 * HDC is a monotonic id (never reused). The array slot itself may be
 * reused after Destroy, but the old HDC value will never be handed
 * out again.
 */
class Fs_DcList {
public:
    Fs_DcList();
    ~Fs_DcList();

    /*!
     * @brief Allocate a new DC slot.
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
    Fs_Dc *Create(ULONG iHwnd, ULONG iHab,
                  PBYTE pTarget, LONG width, LONG height, LONG bpp,
                  ULONG *pNewHdc);

    /*!
     * @brief Find a DC by HDC.
     * @param [in] iHdc  HDC value.
     * @return Slot pointer, or NULL if not found.
     * @retval !NULL  Matching slot.
     * @retval NULL   No such DC.
     */
    Fs_Dc *Find(ULONG iHdc);

    /*!
     * @brief Destroy one DC slot.
     * @param [in] iHdc  HDC value.
     * @return TRUE if found and destroyed.
     * @retval TRUE   Slot freed.
     * @retval FALSE  No such DC.
     */
    BOOL Destroy(ULONG iHdc);

    /*!
     * @brief Destroy all DCs owned by a given HAB.
     *
     * Called from Fs_Server::TeardownThread before windows and the
     * HAB itself are destroyed.
     *
     * @param [in] iHab  Owning HAB id (0 = no-op).
     * @return Number of DCs destroyed.
     * @retval 0    No DCs owned by iHab, or iHab == 0 (no-op).
     * @retval >0   Count of freed slots.
     */
    int DestroyByHab(ULONG iHab);

    /*!
     * @brief Count DCs owned by a given HAB.
     * @param [in] iHab  Owning HAB id.
     * @return Number of matching DCs.
     * @retval 0    No DCs owned by iHab, or iHab == 0.
     * @retval >0   Count.
     */
    int CountByHab(ULONG iHab);

    /*! @brief Count all active DCs in the registry.
        @return Number of used slots.
        @retval >=0  Count. */
    int CountActive();

private:
    void Lock();
    void Unlock();

    Fs_Dc m_aDcs[F_MAX_DCS];
    ULONG m_ulNextHdc;
    HMTX  m_hmtx;
};

#endif /* FS_DC_HPP */
