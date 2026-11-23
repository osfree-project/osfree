#ifndef FS_DC_HPP
#define FS_DC_HPP

#include "Fs_region.hpp"
#include "Fs_path.hpp"
#include "Fs_xformstate.hpp"
#include "Fs_area.hpp"
#include "Fs_arc.hpp"
#include "Fs_line.hpp"
#include "Fs_palette.hpp"

/*!
 * @class Fs_Dc
 * @brief Server-side device context aggregator.
 *
 * Owns the target pixel buffer, geometry, per-DC rendering state
 * (foreground color, current position, driver instance), the clip /
 * visible regions, the DC region, and all per-DC subsystem state
 * (path, transforms, area, arc, line, palette).
 *
 * @par EDM2 Reference
 * Clip and region entry points (INCL_GRE_CLIP, INCL_GRE_REGIONS).
 * CloseDC is mandatory; "deletes any visible regions and clip regions
 * selected into the DC".
 */
class Fs_Dc {
public:
    /*!
     * @brief Construct an unused DC slot.
     * @return Nothing (constructor).
     * @retval (void)  No return value.
     */
    Fs_Dc();

    /*!
     * @brief Destroy and Shutdown if still used.
     * @return Nothing (destructor).
     * @retval (void)  No return value.
     */
    ~Fs_Dc();

    /*!
     * @brief Install a fresh DC into this slot.
     * @param [in] iHdc     Server-assigned HDC (non-zero).
     * @param [in] iHwnd    Owning window id.
     * @param [in] iHab     Owning HAB id.
     * @param [in] pTarget  Target pixel buffer (may be NULL).
     * @param [in] width    Buffer width in pixels.
     * @param [in] height   Buffer height in pixels.
     * @param [in] bpp      Bytes per pixel.
     * @return Success indicator.
     * @retval TRUE   Slot populated.
     * @retval FALSE  Slot already used or iHdc == 0.
     *
     * @note Binds all sub-objects:
     *   - Fs_Path::SetHdc(iHdc)  — so Flush() can pass a real HDC to
     *                              GreDrawLinesInPath;
     *   - Fs_Area::SetPath(&m_Path);
     *   - Fs_Arc::SetPath(&m_Path);
     *   - Fs_Line::SetPath(&m_Path), Fs_Line::SetHdc(iHdc).
     */
    BOOL Init(ULONG iHdc, ULONG iHwnd, ULONG iHab,
              PBYTE pTarget, LONG width, LONG height, LONG bpp);

    /*!
     * @brief Release the slot and free clip regions.
     * @return Nothing.
     * @retval (void)  No return value.
     *
     * @par EDM2 Reference
     * GreCloseDC: "deletes any visible regions and clip regions selected
     * into the DC."
     *
     * @note Also frees the save-stack regions, then Shutdown()s every
     *       sub-object.  Called via g_greDcs.Destroy() from DestroyDc()
     *       in gre.cpp; that is the only correct teardown path.
     */
    void Shutdown();

    /* ---- Accessors ---- */

    /*!
     * @brief Used-slot indicator.
     * @return Flag.
     * @retval TRUE   Slot in use.
     * @retval FALSE  Slot free.
     */
    BOOL IsUsed() const { return m_bUsed; }

    /*!
     * @brief HDC value.
     * @return HDC.
     * @retval !=0  Valid HDC.
     * @retval 0    Unused slot.
     */
    ULONG GetHdc() const { return m_iHdc; }

    /*!
     * @brief Owning window id.
     * @return Window id.
     * @retval >=0  Window id.
     */
    ULONG GetHwnd() const { return m_iHwnd; }

    /*!
     * @brief Owning HAB id.
     * @return HAB id.
     * @retval >=0  HAB id.
     */
    ULONG GetHab() const { return m_iHab; }

    /*!
     * @brief Target pixel buffer.
     * @return Pointer.
     * @retval NULL     No target.
     * @retval non-NULL Target pointer.
     */
    PBYTE GetTarget() const { return m_pTarget; }

    /*!
     * @brief Buffer width.
     * @return Width in pixels.
     * @retval >=0  Width.
     */
    LONG GetWidth() const { return m_width; }

    /*!
     * @brief Buffer height.
     * @return Height in pixels.
     * @retval >=0  Height.
     */
    LONG GetHeight() const { return m_height; }

    /*!
     * @brief Bytes per pixel.
     * @return BPP.
     * @retval >=0  Bytes per pixel.
     */
    LONG GetBpp() const { return m_bpp; }

    /*!
     * @brief Current foreground color.
     * @return Color value.
     * @retval any  Current color.
     */
    LONG GetColor() const { return m_color; }

    /*!
     * @brief Current position.
     * @return POINTL.
     * @retval (x,y)  Current position.
     */
    POINTL GetCurPos() const { return m_curPos; }

    /*!
     * @brief Driver instance pointer.
     * @return Pointer.
     * @retval NULL     Not set.
     * @retval non-NULL Driver data.
     */
    PVOID GetInstance() const { return m_pInstance; }

    /* ---- Mutators ---- */

    /*!
     * @brief Set target pixel buffer.
     * @param [in] p  Pointer.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void SetTarget(PBYTE p) { m_pTarget = p; }

    /*!
     * @brief Set foreground color.
     * @param [in] c  Color value.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void SetColor(LONG c) { m_color = c; }

    /*!
     * @brief Set current position.
     * @param [in] pt  New position.
     * @return Nothing.
     * @retval (void)  No return value.
     * @note Also forwards to Fs_Path so path segment origin tracks
     *       GreSetCurrentPosition, per EDM2 GreCloseFigure contract.
     */
    void SetCurPos(const POINTL &pt)
    { m_curPos = pt; m_Path.SetCurrentPosition(pt); }

    /*!
     * @brief Set owning HAB.
     * @param [in] iHab  HAB id.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void SetHab(ULONG iHab) { m_iHab = iHab; }

    /*!
     * @brief Set driver instance pointer.
     * @param [in] p  Pointer.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void SetInstance(PVOID p) { m_pInstance = p; }

    /* ---- Sub-object access ---- */

    /*!
     * @brief DC region (device ∩ visible ∩ clip).
     * @return Reference.
     * @retval (ref)  Always valid.
     */
    Fs_Region &GetRegion() { return m_dcRgn; }

    /*!
     * @brief Path.
     * @return Reference.
     * @retval (ref)  Always valid.
     */
    Fs_Path &GetPath() { return m_Path; }

    /*!
     * @brief Transform state.
     * @return Reference.
     * @retval (ref)  Always valid.
     */
    Fs_XformState &GetXform() { return m_Xform; }

    /*!
     * @brief Area state.
     * @return Reference.
     * @retval (ref)  Always valid.
     */
    Fs_Area &GetArea() { return m_Area; }

    /*!
     * @brief Arc state.
     * @return Reference.
     * @retval (ref)  Always valid.
     */
    Fs_Arc &GetArc() { return m_Arc; }

    /*!
     * @brief Line state.
     * @return Reference.
     * @retval (ref)  Always valid.
     */
    Fs_Line &GetLine() { return m_Line; }

    /*!
     * @brief Palette state.
     * @return Reference.
     * @retval (ref)  Always valid.
     */
    Fs_Palette &GetPalette() { return m_Palette; }

    /* ---- Clip region state ---- */

    /*!
     * @brief Current clip region.
     * @return Pointer.
     * @retval NULL     No clip region selected.
     * @retval non-NULL Selected clip region.
     *
     * @par EDM2 Reference
     * GreQueryClipRegion (https://www.edm2.com/index.php/GreQueryClipRegion).
     */
    Fs_Region *GetClipRgn() const { return m_pClipRgn; }

    /*!
     * @brief Visible region.
     * @return Pointer.
     * @retval NULL     No visible region.
     * @retval non-NULL Visible region.
     */
    Fs_Region *GetVisibleRgn() const { return m_pVisibleRgn; }

    /*!
     * @brief Current clip region (alias of GetClipRgn).
     * @return Pointer.
     * @retval NULL     None.
     * @retval non-NULL Selected clip region.
     */
    Fs_Region *QueryClipRegion() const { return m_pClipRgn; }

    /*!
     * @brief Detach the clip region without destroying it.
     * @return Previously selected region.
     * @retval NULL     None was selected.
     * @retval non-NULL Caller takes ownership.
     */
    Fs_Region *ReleaseClipRgn();

    /*!
     * @brief Attach a clip region pointer (does not set clip-selected).
     * @param [in] p  Region (may be NULL).
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void SetClipRgn(Fs_Region *p) { m_pClipRgn = p; }

    /*!
     * @brief Attach a visible region pointer.
     * @param [in] p  Region (may be NULL).
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void SetVisibleRgn(Fs_Region *p) { m_pVisibleRgn = p; }

    /* ---- Clip operations ---- */

    /*!
     * @brief DC region bounding rectangle and complexity.
     * @param [out] out  Receives bounding rectangle.
     * @return Complexity code.
     * @retval RGN_NULL / RGN_RECT / RGN_COMPLEX.
     *
     * @par EDM2 Reference
     * GreGetClipBox (https://www.edm2.com/index.php/GreGetClipBox):
     * "The DC region is defined as the intersection of the visible region,
     * clip region, viewing limits, graphics field, and clip path."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG GetClipBox(RECTL &out) const;

    /*!
     * @brief Enumerate DC region rectangles.
     * @param [in]     prclBound  Optional bounding filter.
     * @param [in,out] pControl   RGNRECT.
     * @param [out]    parclRect  Output buffer (NULL = count only).
     * @return Success indicator.
     * @retval TRUE   Enumeration completed.
     * @retval FALSE  pControl is NULL.
     *
     * @par EDM2 Reference
     * GreGetClipRects (https://www.edm2.com/index.php/GreGetClipRects).
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL EnumClipRects(const RECTL *prclBound, PRGNRECT pControl,
                       PRECTL parclRect) const;

    /*!
     * @brief Point visibility test.
     * @param [in] pt  Point.
     * @return Visibility code.
     * @retval PVIS_VISIBLE    Visible.
     * @retval PVIS_INVISIBLE  Invisible.
     *
     * @par EDM2 Reference
     * GrePtVisible (https://www.edm2.com/index.php/GrePtVisible).
     */
    LONG PtVisible(const POINTL &pt) const;

    /*!
     * @brief Rectangle visibility test.
     * @param [in] rc  Rectangle.
     * @return Visibility code.
     * @retval RVIS_VISIBLE    Fully visible.
     * @retval RVIS_PARTIAL    Partially visible.
     * @retval RVIS_INVISIBLE  Fully invisible.
     *
     * @par EDM2 Reference
     * GreRectVisible (https://www.edm2.com/index.php/GreRectVisible).
     */
    LONG RectVisible(const RECTL &rc) const;

    /*!
     * @brief Offset the clip region.
     * @param [in] dx  X delta.
     * @param [in] dy  Y delta.
     * @return Complexity of the new DC region.
     * @retval RGN_NULL / RGN_RECT / RGN_COMPLEX.
     *
     * @par EDM2 Reference
     * GreOffsetClipRegion (https://www.edm2.com/index.php/GreOffsetClipRegion).
     */
    LONG OffsetClipRegion(LONG dx, LONG dy);

    /*!
     * @brief Select a clip region.
     * @param [in]  pNew   New clip region (NULL = clear).
     * @param [out] ppOld  Receives previous region (caller owns) when
     *                     non-NULL; otherwise old is deleted.
     * @return Complexity of the new DC region.
     * @retval RGN_NULL / RGN_RECT / RGN_COMPLEX.
     *
     * @par EDM2 Reference
     * GreSelectClipRegion (https://www.edm2.com/index.php/GreSelectClipRegion):
     * "passes the handle of the previously selected clip region back to
     * the calling routine at the location addressed by phrgnOld. If this
     * handle is not NULL, the calling routine is responsible for deleting
     * the previous clip region."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG SelectClipRegion(Fs_Region *pNew, Fs_Region **ppOld);

    /*!
     * @brief Intersect the clip region with a rectangle.
     * @param [in] rc  Rectangle.
     * @return Complexity of the new DC region.
     * @retval RGN_NULL / RGN_RECT / RGN_COMPLEX.
     *
     * @par EDM2 Reference
     * GreIntersectClipRectangle
     * (https://www.edm2.com/index.php/GreIntersectClipRectangle):
     * "When no clip region exists, GreIntersectClipRectangle must create one."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG IntersectClipRectangle(const RECTL &rc);

    /*!
     * @brief Exclude a rectangle from the clip region.
     * @param [in] rc  Rectangle.
     * @return Complexity of the new DC region.
     * @retval RGN_NULL / RGN_RECT / RGN_COMPLEX.
     *
     * @par EDM2 Reference
     * GreExcludeClipRectangle
     * (https://www.edm2.com/index.php/GreExcludeClipRectangle):
     * "creates a clip region when none exists."
     */
    LONG ExcludeClipRectangle(const RECTL &rc);

    /*!
     * @brief Intersect DC region with a device rectangle.
     * @param [in] rc  Rectangle.
     * @return Complexity of the new DC region.
     * @retval RGN_NULL / RGN_RECT / RGN_COMPLEX.
     *
     * @par EDM2 Reference
     * GreSetXformRect (https://www.edm2.com/index.php/GreSetXformRect).
     */
    LONG SetXformRect(const RECTL &rc);

    /*!
     * @brief Copy visible / clip / DC region into destination.
     * @param [in] dst        Destination (cleared first).
     * @param [in] flOptions  COPYCRGN_ALLINTERSECT / VISRGN / CLIPRGN.
     * @return Complexity of the DC region.
     * @retval RGN_ERROR                          Unknown flOptions.
     * @retval RGN_NULL / RGN_RECT / RGN_COMPLEX  DC complexity.
     *
     * @par EDM2 Reference
     * GreCopyClipRegion (https://www.edm2.com/index.php/GreCopyClipRegion).
     */
    LONG CopyClipRegion(Fs_Region &dst, ULONG flOptions) const;

    /*!
     * @brief Initialize DC region from visible region and origin.
     * @param [in] pVis        New visible region (ownership transferred).
     * @param [in] xOrg        DC origin X (screen coordinates).
     * @param [in] yOrg        DC origin Y (screen coordinates).
     * @param [in] prclBounds  Device bounds (may be NULL).
     * @param [in] flOptions   SETUPDC_* flags.
     * @return Success indicator.
     * @retval TRUE   Always.
     * @retval FALSE  Never.
     *
     * @par EDM2 Reference
     * GreSetupDC (https://www.edm2.com/index.php/GreSetupDC):
     * "SETUPDC_VISRGN: When this bit is set, the old visible region is
     * replaced with the new one. The presentation driver must copy the data."
     * "SETUPDC_ORIGIN: ... The graphics engine adjusts the origin in the
     * graphics engine and then calls GreSetupDC."
     * "SETUPDC_CLEANDC: ... the simulation marks the visible regions as valid
     * and calls GreNotifyClipChange in the presentation driver."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL SetupDC(Fs_Region *pVis, LONG xOrg, LONG yOrg,
                 const RECTL *prclBounds, ULONG flOptions);

    /*!
     * @brief Select or reset the clip path.
     * @param [in] idPath     Path id (0 = clear; 1 = merge current path).
     * @param [in] flOptions  SCP_AND / SCP_RESET.
     * @return Success indicator.
     * @retval TRUE   Applied.
     * @retval FALSE  idPath not 0 or 1, or path→region conversion failed.
     *
     * @par EDM2 Reference
     * GreSelectClipPath (https://www.edm2.com/index.php/GreSelectClipPath):
     * "When this is passed as 0, it indicates "no clipping". Otherwise, the
     * new clip path is intersected with the existing clip path."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL SelectClipPath(ULONG idPath, ULONG flOptions);

    /*!
     * @brief Merge a path-derived region into the clip region.
     * @param [in] pPath  Path-derived region.
     * @return Success indicator.
     * @retval TRUE   Merged.
     * @retval FALSE  pPath is NULL or allocation failure.
     *
     * @par EDM2 Reference
     * GreSelectPathRegion (https://www.edm2.com/index.php/GreSelectPathRegion).
     */
    BOOL SelectPathRegion(Fs_Region *pPath);

    /*!
     * @brief Notify that a new bitmap was selected into the DC.
     * @return Success indicator.
     * @retval TRUE   Always.
     * @retval FALSE  Never.
     *
     * @par EDM2 Reference
     * GreRegionSelectBitmap
     * (https://www.edm2.com/index.php/GreRegionSelectBitmap):
     * "removes the old visible region and informs the presentation driver
     * that the DC region must be recalculated."
     */
    BOOL RegionSelectBitmap();

    /*!
     * @brief Notify a clip-change event.
     * @param [in] idClipPath  NCC_CLEANDC clears the dirty flag.
     * @return Nothing.
     * @retval (void)  No return value.
     *
     * @par EDM2 Reference
     * GreNotifyClipChange (https://www.edm2.com/index.php/GreNotifyClipChange):
     * "For display devices, when idClipPath is passed as NCC_CLEANDC, the
     * handling routine should clear the HDC_IS_DIRTY flag."
     *
     * @warning Terminal engine handler; driver hooks chain INTO this via
     *          the default dispatch table.  It does not chain further.
     */
    void NotifyClipChange(ULONG idClipPath);

    /*!
     * @brief Query HDC_IS_DIRTY.
     * @return Flag.
     * @retval TRUE   Dirty.
     * @retval FALSE  Clean.
     */
    BOOL IsDirty() const { return m_fDirty; }

    /*!
     * @brief Push current clip state onto the save stack.
     * @return Success indicator.
     * @retval TRUE   Pushed.
     * @retval FALSE  DosAllocMem failure.
     *
     * @par EDM2 Reference
     * GreSaveRegion (https://www.edm2.com/index.php/GreSaveRegion):
     * "GreSaveRegion is called during SaveDC and OpenDC to allow the
     * simulations to save their local data structures."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL SaveRegionState();

    /*!
     * @brief Restore clip state from a save level.
     * @param [in] cSaveLevel  -1/0/1 = reset; N>=2 = restore level N.
     * @return Success indicator.
     * @retval TRUE   Restored.
     * @retval FALSE  Level out of range.
     *
     * @par EDM2 Reference
     * GreRestoreRegion (https://www.edm2.com/index.php/GreRestoreRegion):
     * "When this DC is closed, GreRestoreRegion is called with a count of 0
     * to free its local data."
     */
    BOOL RestoreRegionState(LONG cSaveLevel);

    /*!
     * @brief Reset clip state to post-OpenDC initial.
     * @return Success indicator.
     * @retval TRUE   Always.
     * @retval FALSE  Never.
     *
     * @par EDM2 Reference
     * GreSaveRegion (level 1 initialization) / GreRestoreRegion (level 0).
     */
    BOOL InitRegionState();

private:
    /*!
     * @brief Recompute DC region = device ∩ visible ∩ clip.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void RecalcDCRegion();

    BOOL    m_bUsed;         /*!< Slot in use. */
    ULONG   m_iHdc;          /*!< Server-assigned HDC. */
    ULONG   m_iHwnd;         /*!< Owning window id. */
    ULONG   m_iHab;          /*!< Owning HAB id. */
    PBYTE   m_pTarget;       /*!< Target pixel buffer. */
    LONG    m_width;         /*!< Buffer width. */
    LONG    m_height;        /*!< Buffer height. */
    LONG    m_bpp;           /*!< Bytes per pixel. */
    LONG    m_color;         /*!< Current foreground color. */
    POINTL  m_curPos;        /*!< Current position. */
    PVOID   m_pInstance;     /*!< Driver instance pointer. */

    Fs_Region *m_pClipRgn;   /*!< Selected clip region (owned). */
    Fs_Region *m_pVisibleRgn;/*!< Visible region (owned). */
    Fs_Region  m_dcRgn;      /*!< DC region = device ∩ visible ∩ clip. */
    RECTL      m_rclDevice;  /*!< Device rectangle. */
    LONG       m_lComplexity;/*!< Cached DC complexity. */
    BOOL       m_fDirty;     /*!< HDC_IS_DIRTY flag. */

    Fs_Path       m_Path;    /*!< Path under construction. */
    Fs_XformState m_Xform;   /*!< Transform state. */
    Fs_Area       m_Area;    /*!< Area bracket state. */
    Fs_Arc        m_Arc;     /*!< Arc state. */
    Fs_Line       m_Line;    /*!< Line state. */
    Fs_Palette    m_Palette; /*!< Palette state. */

    struct SaveState {
        Fs_Region *pClipRgn;   /*!< Deep copy of clip region at save time. */
        RECTL      rclDevice;  /*!< Device rect at save time. */
        LONG       lComplexity;/*!< Complexity at save time. */
        BOOL       fDirty;     /*!< Dirty flag at save time. */
    };
    SaveState *m_pSaveStack;   /*!< Save stack (DosAllocMem). */
    ULONG      m_ulSaveCount;  /*!< Used entries. */
    ULONG      m_ulSaveAlloc;  /*!< Allocated capacity. */
};

/*!
 * @class Fs_DcList
 * @brief Registry of Fs_Dc slots, serialized by an OS/2 mutex.
 *
 * HDC is a monotonic id (never reused).  The array slot itself may be
 * reused after Destroy, but the old HDC value will never be handed out
 * again.
 */
class Fs_DcList {
public:
    /*!
     * @brief Create the DC registry and its mutex.
     * @return Nothing (constructor).
     * @retval (void)  No return value.
     */
    Fs_DcList();

    /*!
     * @brief Close the registry mutex.
     * @return Nothing (destructor).
     * @retval (void)  No return value.
     */
    ~Fs_DcList();

    /*!
     * @brief Allocate a new DC slot.
     * @param [in]  iHwnd     Owning window id.
     * @param [in]  iHab      Owning HAB id.
     * @param [in]  pTarget   Target pixel buffer.
     * @param [in]  width     Buffer width.
     * @param [in]  height    Buffer height.
     * @param [in]  bpp       Bytes per pixel.
     * @param [out] pNewHdc   Receives the new HDC.
     * @return Slot pointer.
     * @retval NULL     Table full.
     * @retval non-NULL Allocated DC.
     */
    Fs_Dc *Create(ULONG iHwnd, ULONG iHab,
                  PBYTE pTarget, LONG width, LONG height, LONG bpp,
                  ULONG *pNewHdc);

    /*!
     * @brief Find a DC by HDC.
     * @param [in] iHdc  HDC value.
     * @return Slot pointer.
     * @retval NULL     Not found.
     * @retval non-NULL Matching slot.
     */
    Fs_Dc *Find(ULONG iHdc);

    /*!
     * @brief Destroy one DC slot.
     * @param [in] iHdc  HDC value.
     * @return Success indicator.
     * @retval TRUE   Slot freed.
     * @retval FALSE  No such DC.
     */
    BOOL Destroy(ULONG iHdc);

    /*!
     * @brief Destroy all DCs owned by a given HAB.
     * @param [in] iHab  Owning HAB id.
     * @return Count of freed slots.
     * @retval 0    No DCs owned by iHab, or iHab == 0.
     * @retval >0   Count.
     */
    int DestroyByHab(ULONG iHab);

    /*!
     * @brief Count DCs owned by a given HAB.
     * @param [in] iHab  Owning HAB id.
     * @return Count.
     * @retval 0    No DCs, or iHab == 0.
     * @retval >0   Count.
     */
    int CountByHab(ULONG iHab);

    /*!
     * @brief Count active DCs.
     * @return Count.
     * @retval >=0  Number of used slots.
     */
    int CountActive();

private:
    /*!
     * @brief Acquire the registry mutex (no-op if not created).
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void Lock();

    /*!
     * @brief Release the registry mutex (no-op if not created).
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void Unlock();

    Fs_Dc m_aDcs[F_MAX_DCS]; /*!< Fixed slot array. */
    ULONG m_ulNextHdc;       /*!< Monotonic HDC counter (starts at 1). */
    HMTX  m_hmtx;            /*!< Registry mutex (may be NULL). */
};

#endif /* FS_DC_HPP */
