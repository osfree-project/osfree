#include "Fs_dc.hpp"
#include "F_debug.h"
#include <string.h>

/*!
 * @brief Construct an unused DC slot.
 * @return Nothing (constructor).
 * @retval (void)  No return value.
 */
Fs_Dc::Fs_Dc()
    : m_bUsed(FALSE), m_iHdc(0), m_iHwnd(0), m_iHab(0),
      m_pTarget(NULL), m_width(0), m_height(0), m_bpp(0), m_color(0),
      m_pClipRgn(NULL), m_pVisibleRgn(NULL),
      m_lComplexity(RGN_NULL), m_fDirty(FALSE),
      m_pSaveStack(NULL), m_ulSaveCount(0), m_ulSaveAlloc(0)
{
    m_curPos.x = m_curPos.y = 0;
    m_pInstance = NULL;
    m_rclDevice.xLeft = m_rclDevice.xRight = 0;
    m_rclDevice.yBottom = m_rclDevice.yTop = 0;
}

/*!
 * @brief Destroy and Shutdown if still used.
 * @return Nothing (destructor).
 * @retval (void)  No return value.
 */
Fs_Dc::~Fs_Dc()
{
    if (m_bUsed) Shutdown();
}

/*!
 * @brief Install a fresh DC into this slot.
 * @param [in] iHdc     Server-assigned HDC.
 * @param [in] iHwnd    Owning window id.
 * @param [in] iHab     Owning HAB id.
 * @param [in] pTarget  Target pixel buffer.
 * @param [in] width    Buffer width.
 * @param [in] height   Buffer height.
 * @param [in] bpp      Bytes per pixel.
 * @return Success indicator.
 * @retval TRUE   Populated.
 * @retval FALSE  Slot already used or iHdc == 0.
 */
BOOL Fs_Dc::Init(ULONG iHdc, ULONG iHwnd, ULONG iHab,
                 PBYTE pTarget, LONG width, LONG height, LONG bpp)
{
    if (m_bUsed)   return FALSE;
    if (iHdc == 0) return FALSE;

    m_bUsed  = TRUE;
    m_iHdc   = iHdc;
    m_iHwnd  = iHwnd;
    m_iHab   = iHab;
    m_pTarget = pTarget;
    m_width  = width;
    m_height = height;
    m_bpp    = bpp;
    m_color  = 0;
    m_curPos.x = m_curPos.y = 0;

    m_pClipRgn    = NULL;
    m_pVisibleRgn = NULL;
    m_dcRgn.Clear();
    m_rclDevice.xLeft   = 0;
    m_rclDevice.yBottom = 0;
    m_rclDevice.xRight  = width;
    m_rclDevice.yTop    = height;
    m_lComplexity = RGN_RECT;
    m_fDirty      = FALSE;

    m_Path.SetHdc(iHdc);
    m_Path.Init();
    m_Xform.Init();
    m_Area.Init();   m_Area.SetPath(&m_Path);
    m_Arc.Init();    m_Arc.SetPath(&m_Path);
    m_Line.Init();   m_Line.SetPath(&m_Path); m_Line.SetHdc(iHdc);
    m_Palette.Init();

    m_pSaveStack  = NULL;
    m_ulSaveCount = 0;
    m_ulSaveAlloc = 0;

    RecalcDCRegion();
    return TRUE;
}

/*!
 * @brief Release the slot and free clip regions.
 * @return Nothing.
 * @retval (void)  No return value.
 *
 * @par EDM2 Reference
 * GreCloseDC: "deletes any visible regions and clip regions selected into
 * the DC."
 */
void Fs_Dc::Shutdown()
{
    if (m_pClipRgn) {
        m_pClipRgn->SetClipSelected(FALSE);
        delete m_pClipRgn;
        m_pClipRgn = NULL;
    }
    if (m_pVisibleRgn) {
        delete m_pVisibleRgn;
        m_pVisibleRgn = NULL;
    }
    for (ULONG i = 0; i < m_ulSaveCount; i++)
        if (m_pSaveStack[i].pClipRgn) delete m_pSaveStack[i].pClipRgn;
    if (m_pSaveStack) { DosFreeMem((PVOID)m_pSaveStack); m_pSaveStack = NULL; }
    m_ulSaveCount = m_ulSaveAlloc = 0;
    m_dcRgn.Clear();

    m_Path.Shutdown();
    m_Xform.Shutdown();
    m_Area.Shutdown();
    m_Arc.Shutdown();
    m_Line.Shutdown();
    m_Palette.Shutdown();

    m_bUsed = FALSE;
    m_iHdc  = 0;
    m_iHwnd = 0;
    m_iHab  = 0;
    m_pTarget = NULL;
    m_width = m_height = m_bpp = 0;
    m_color = 0;
    m_curPos.x = m_curPos.y = 0;
    m_rclDevice.xLeft = m_rclDevice.xRight = 0;
    m_rclDevice.yBottom = m_rclDevice.yTop = 0;
    m_lComplexity = RGN_NULL;
    m_fDirty = FALSE;
    m_pInstance = NULL;
}

/*!
 * @brief Detach the clip region without destroying it.
 * @return Previously selected region.
 * @retval NULL     None was selected.
 * @retval non-NULL Caller takes ownership.
 */
Fs_Region *Fs_Dc::ReleaseClipRgn()
{
    Fs_Region *p = m_pClipRgn;
    m_pClipRgn = NULL;
    return p;
}

/*!
 * @brief Recompute DC region = device ∩ visible ∩ clip.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Dc::RecalcDCRegion()
{
    m_dcRgn.Clear();
    m_dcRgn.AddRect(m_rclDevice);
    if (m_pVisibleRgn) m_dcRgn.IntersectRegion(*m_pVisibleRgn);
    if (m_pClipRgn)    m_dcRgn.IntersectRegion(*m_pClipRgn);
    RECTL b;
    m_lComplexity = m_dcRgn.UpdateBounds(b);
}

/*!
 * @brief DC region bounding rectangle and complexity.
 * @param [out] out  Receives bounding rectangle.
 * @return Complexity code.
 * @retval RGN_NULL / RGN_RECT / RGN_COMPLEX.
 */
LONG Fs_Dc::GetClipBox(RECTL &out) const
{
    return m_dcRgn.UpdateBounds(out);
}

/*!
 * @brief Enumerate DC region rectangles.
 * @param [in]     prclBound  Bounding filter (NULL = all).
 * @param [in,out] pControl   RGNRECT.
 * @param [out]    parclRect  Output buffer (NULL = count only).
 * @return Success indicator.
 * @retval TRUE   Completed.
 * @retval FALSE  pControl is NULL.
 */
BOOL Fs_Dc::EnumClipRects(const RECTL *prclBound, PRGNRECT pControl,
                          PRECTL parclRect) const
{
    return m_dcRgn.EnumRects(prclBound, pControl, parclRect);
}

/*!
 * @brief Point visibility test.
 * @param [in] pt  Point.
 * @return Visibility code.
 * @retval PVIS_VISIBLE / PVIS_INVISIBLE.
 */
LONG Fs_Dc::PtVisible(const POINTL &pt) const
{
    return m_dcRgn.Contains(pt) ? PVIS_VISIBLE : PVIS_INVISIBLE;
}

/*!
 * @brief Rectangle visibility test.
 * @param [in] rc  Rectangle.
 * @return Visibility code.
 * @retval RVIS_VISIBLE / RVIS_PARTIAL / RVIS_INVISIBLE.
 */
LONG Fs_Dc::RectVisible(const RECTL &rc) const
{
    RECTL nr = rc;
    Fs_Region::NormalizeRect(nr);
    Fs_Region orig;
    orig.AddRect(nr);
    Fs_Region rem = orig;
    rem.SubtractRegion(m_dcRgn);
    if (rem.IsEmpty()) return RVIS_VISIBLE;
    if (rem.Count() == orig.Count() &&
        memcmp(rem.Data(), orig.Data(), rem.Count() * sizeof(RECTL)) == 0)
        return RVIS_INVISIBLE;
    return RVIS_PARTIAL;
}

/*!
 * @brief Offset the clip region.
 * @param [in] dx  X delta.
 * @param [in] dy  Y delta.
 * @return Complexity of the new DC region.
 * @retval RGN_NULL / RGN_RECT / RGN_COMPLEX.
 */
LONG Fs_Dc::OffsetClipRegion(LONG dx, LONG dy)
{
    if (m_pClipRgn) m_pClipRgn->Offset(dx, dy);
    RecalcDCRegion();
    RECTL b;
    return m_dcRgn.UpdateBounds(b);
}

/*!
 * @brief Select a clip region.
 * @param [in]  pNew   New clip region (NULL = clear).
 * @param [out] ppOld  Receives previous region (caller owns) when non-NULL;
 *                     otherwise old is deleted.
 * @return Complexity of the new DC region.
 * @retval RGN_NULL / RGN_RECT / RGN_COMPLEX.
 */
LONG Fs_Dc::SelectClipRegion(Fs_Region *pNew, Fs_Region **ppOld)
{
    if (m_pClipRgn) {
        m_pClipRgn->SetClipSelected(FALSE);
        if (ppOld) *ppOld = m_pClipRgn;
        else       delete m_pClipRgn;
    } else if (ppOld) {
        *ppOld = NULL;
    }
    m_pClipRgn = pNew;
    if (m_pClipRgn) m_pClipRgn->SetClipSelected(TRUE);
    RecalcDCRegion();
    RECTL b;
    return m_dcRgn.UpdateBounds(b);
}

/*!
 * @brief Intersect the clip region with a rectangle.
 * @param [in] rc  Rectangle.
 * @return Complexity of the new DC region.
 * @retval RGN_NULL / RGN_RECT / RGN_COMPLEX.
 */
LONG Fs_Dc::IntersectClipRectangle(const RECTL &rc)
{
    RECTL nr = rc;
    Fs_Region::NormalizeRect(nr);
    if (!m_pClipRgn) {
        m_pClipRgn = new Fs_Region(nr);
        if (m_pClipRgn) m_pClipRgn->SetClipSelected(TRUE);
    } else {
        m_pClipRgn->IntersectRect(nr);
    }
    RecalcDCRegion();
    RECTL b;
    return m_dcRgn.UpdateBounds(b);
}

/*!
 * @brief Exclude a rectangle from the clip region.
 * @param [in] rc  Rectangle.
 * @return Complexity of the new DC region.
 * @retval RGN_NULL / RGN_RECT / RGN_COMPLEX.
 */
LONG Fs_Dc::ExcludeClipRectangle(const RECTL &rc)
{
    RECTL nr = rc;
    Fs_Region::NormalizeRect(nr);
    if (!m_pClipRgn) {
        m_pClipRgn = new Fs_Region(m_rclDevice);
        if (m_pClipRgn) m_pClipRgn->SetClipSelected(TRUE);
    }
    if (m_pClipRgn) m_pClipRgn->SubtractRect(nr);
    RecalcDCRegion();
    RECTL b;
    return m_dcRgn.UpdateBounds(b);
}

/*!
 * @brief Intersect DC region with a device rectangle.
 * @param [in] rc  Rectangle.
 * @return Complexity of the new DC region.
 * @retval RGN_NULL / RGN_RECT / RGN_COMPLEX.
 */
LONG Fs_Dc::SetXformRect(const RECTL &rc)
{
    return IntersectClipRectangle(rc);
}

/*!
 * @brief Copy visible / clip / DC region into destination.
 * @param [in] dst        Destination (cleared first).
 * @param [in] flOptions  COPYCRGN_* selector.
 * @return Complexity of the DC region.
 * @retval RGN_ERROR                          Unknown flOptions.
 * @retval RGN_NULL / RGN_RECT / RGN_COMPLEX  DC complexity.
 */
LONG Fs_Dc::CopyClipRegion(Fs_Region &dst, ULONG flOptions) const
{
    const Fs_Region *pSrc = NULL;
    switch (flOptions) {
    case COPYCRGN_ALLINTERSECT:
    case COPYCRGN_CLIPRGN:
        pSrc = &m_dcRgn;
        break;
    case COPYCRGN_VISRGN:
        pSrc = m_pVisibleRgn;
        break;
    default:
        return RGN_ERROR;
    }
    dst.Clear();
    if (pSrc) dst.AddRegion(*pSrc);
    RECTL b;
    return m_dcRgn.UpdateBounds(b);
}

/*!
 * @brief Initialize DC region from visible region and origin.
 * @param [in] pVis        New visible region (ownership transferred).
 * @param [in] xOrg        DC origin X (screen coordinates).
 * @param [in] yOrg        DC origin Y (screen coordinates).
 * @param [in] prclBounds  Device bounds (may be NULL).
 * @param [in] flOptions   SETUPDC_* flags.
 * @return TRUE.
 * @retval TRUE  Always.
 *
 * @par EDM2 Reference
 * GreSetupDC — see header.
 */
BOOL Fs_Dc::SetupDC(Fs_Region *pVis, LONG xOrg, LONG yOrg,
                    const RECTL *prclBounds, ULONG flOptions)
{
    if (flOptions & SETUPDC_VISRGN) {
        if (m_pVisibleRgn && m_pVisibleRgn != pVis) delete m_pVisibleRgn;
        m_pVisibleRgn = pVis;
    }

    if (flOptions & SETUPDC_ORIGIN) {
        if (prclBounds) {
            m_rclDevice.xLeft   = prclBounds->xLeft   + xOrg;
            m_rclDevice.yBottom = prclBounds->yBottom + yOrg;
            m_rclDevice.xRight  = prclBounds->xRight  + xOrg;
            m_rclDevice.yTop    = prclBounds->yTop    + yOrg;
        }
        /* Per EDM2: visible region coordinates are screen-based; the engine
         * adjusts the origin and passes device-space bounds.  Translate the
         * visible region by the same offset to keep the two in sync. */
        if (m_pVisibleRgn) m_pVisibleRgn->Offset(xOrg, yOrg);
    }

    if (flOptions & SETUPDC_CLEANDC) m_fDirty = FALSE;

    RecalcDCRegion();
    return TRUE;
}

/*!
 * @brief Select or reset the clip path.
 * @param [in] idPath     Path id (0 = clear; 1 = merge current path).
 * @param [in] flOptions  SCP_AND / SCP_RESET.
 * @return Success indicator.
 * @retval TRUE   Applied.
 * @retval FALSE  idPath not 0 or 1, or path→region conversion failed.
 *
 * @par EDM2 Reference
 * GreSelectClipPath — see header.
 */
BOOL Fs_Dc::SelectClipPath(ULONG idPath, ULONG flOptions)
{
    if (idPath == 0) {
        if (m_pClipRgn) {
            m_pClipRgn->SetClipSelected(FALSE);
            delete m_pClipRgn;
            m_pClipRgn = NULL;
        }
        RecalcDCRegion();
        return TRUE;
    }

    if (idPath != 1) return FALSE;

    Fs_Region pathRgn;
    if (!m_Path.ToRegion(pathRgn)) return FALSE;

    if (flOptions & SCP_AND) {
        if (!m_pClipRgn) {
            m_pClipRgn = new Fs_Region();
            m_pClipRgn->SetClipSelected(TRUE);
        }
        m_pClipRgn->IntersectRegion(pathRgn);
    } else {
        if (m_pClipRgn) {
            m_pClipRgn->SetClipSelected(FALSE);
            delete m_pClipRgn;
        }
        m_pClipRgn = new Fs_Region(pathRgn);
        m_pClipRgn->SetClipSelected(TRUE);
    }

    /* Path is consumed once selected as clip. */
    m_Path.EndPath(0);
    RecalcDCRegion();
    return TRUE;
}

/*!
 * @brief Merge a path-derived region into the clip region.
 * @param [in] pPath  Path-derived region.
 * @return Success indicator.
 * @retval TRUE   Merged.
 * @retval FALSE  pPath is NULL or allocation failure.
 */
BOOL Fs_Dc::SelectPathRegion(Fs_Region *pPath)
{
    if (!pPath) return FALSE;
    if (!m_pClipRgn) {
        m_pClipRgn = new Fs_Region();
        if (!m_pClipRgn) return FALSE;
        m_pClipRgn->SetClipSelected(TRUE);
    }
    m_pClipRgn->AddRegion(*pPath);
    RecalcDCRegion();
    return TRUE;
}

/*!
 * @brief Notify that a new bitmap was selected into the DC.
 * @return TRUE.
 * @retval TRUE  Always.
 */
BOOL Fs_Dc::RegionSelectBitmap()
{
    if (m_pVisibleRgn) {
        delete m_pVisibleRgn;
        m_pVisibleRgn = NULL;
    }
    m_dcRgn.Clear();
    m_rclDevice.xLeft = m_rclDevice.xRight = 0;
    m_rclDevice.yBottom = m_rclDevice.yTop = 0;
    return TRUE;
}

/*!
 * @brief Notify a clip-change event.
 * @param [in] idClipPath  NCC_CLEANDC clears the dirty flag.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Dc::NotifyClipChange(ULONG idClipPath)
{
    if (idClipPath == NCC_CLEANDC) m_fDirty = FALSE;
}

/*!
 * @brief Push current clip state onto the save stack.
 * @return Success indicator.
 * @retval TRUE   Pushed.
 * @retval FALSE  DosAllocMem failure.
 */
BOOL Fs_Dc::SaveRegionState()
{
    if (m_ulSaveCount >= m_ulSaveAlloc) {
        ULONG cNew = m_ulSaveAlloc ? m_ulSaveAlloc * 2 : 4;
        PVOID pv = NULL;
        if (DosAllocMem(&pv, cNew * sizeof(SaveState),
                        PAG_READ | PAG_WRITE | PAG_COMMIT) != 0) return FALSE;
        if (m_pSaveStack)
            memcpy(pv, m_pSaveStack, m_ulSaveCount * sizeof(SaveState));
        if (m_pSaveStack) DosFreeMem((PVOID)m_pSaveStack);
        m_pSaveStack  = (SaveState *)pv;
        m_ulSaveAlloc = cNew;
    }

    SaveState &s = m_pSaveStack[m_ulSaveCount];
    s.pClipRgn    = m_pClipRgn ? m_pClipRgn->Clone() : NULL;
    s.rclDevice   = m_rclDevice;
    s.lComplexity = m_lComplexity;
    s.fDirty      = m_fDirty;
    m_ulSaveCount++;
    return TRUE;
}

/*!
 * @brief Restore clip state from a save level.
 * @param [in] cSaveLevel  -1/0/1 = reset; N>=2 = restore level N.
 * @return Success indicator.
 * @retval TRUE   Restored.
 * @retval FALSE  Level out of range.
 */
BOOL Fs_Dc::RestoreRegionState(LONG cSaveLevel)
{
    if (cSaveLevel <= 0 || cSaveLevel == 1) return InitRegionState();

    ULONG targetIdx = (ULONG)cSaveLevel - 2;
    if (targetIdx >= m_ulSaveCount) return FALSE;

    Fs_Region *pRestored  = m_pSaveStack[targetIdx].pClipRgn;
    RECTL      rDev       = m_pSaveStack[targetIdx].rclDevice;
    LONG       lC         = m_pSaveStack[targetIdx].lComplexity;
    BOOL       f          = m_pSaveStack[targetIdx].fDirty;
    m_pSaveStack[targetIdx].pClipRgn = NULL;

    for (ULONG i = targetIdx + 1; i < m_ulSaveCount; i++)
        if (m_pSaveStack[i].pClipRgn) delete m_pSaveStack[i].pClipRgn;
    m_ulSaveCount = targetIdx + 1;

    if (m_pClipRgn) {
        m_pClipRgn->SetClipSelected(FALSE);
        delete m_pClipRgn;
    }
    m_pClipRgn = pRestored;
    if (m_pClipRgn) m_pClipRgn->SetClipSelected(TRUE);

    m_rclDevice   = rDev;
    m_lComplexity = lC;
    m_fDirty      = f;

    RecalcDCRegion();
    return TRUE;
}

/*!
 * @brief Reset clip state to post-OpenDC initial.
 * @return TRUE.
 * @retval TRUE  Always.
 */
BOOL Fs_Dc::InitRegionState()
{
    if (m_pClipRgn) {
        m_pClipRgn->SetClipSelected(FALSE);
        delete m_pClipRgn;
        m_pClipRgn = NULL;
    }
    if (m_pVisibleRgn) {
        delete m_pVisibleRgn;
        m_pVisibleRgn = NULL;
    }
    for (ULONG i = 0; i < m_ulSaveCount; i++)
        if (m_pSaveStack[i].pClipRgn) delete m_pSaveStack[i].pClipRgn;
    m_ulSaveCount = 0;

    m_rclDevice.xLeft   = 0;
    m_rclDevice.yBottom = 0;
    m_rclDevice.xRight  = m_width;
    m_rclDevice.yTop    = m_height;
    m_fDirty = FALSE;

    RecalcDCRegion();
    return TRUE;
}

/* ==================================================================== */
/*  Fs_DcList                                                           */
/* ==================================================================== */

/*!
 * @brief Create the DC registry and its mutex.
 * @return Nothing (constructor).
 * @retval (void)  No return value.
 */
Fs_DcList::Fs_DcList() : m_ulNextHdc(1)
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
 * @retval (void)  No return value.
 */
Fs_DcList::~Fs_DcList()
{
    if (m_hmtx) { DosCloseMutexSem(m_hmtx); m_hmtx = NULL; }
}

/*!
 * @brief Acquire the registry mutex.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_DcList::Lock()
{ if (m_hmtx) DosRequestMutexSem(m_hmtx, SEM_INDEFINITE_WAIT); }

/*!
 * @brief Release the registry mutex.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_DcList::Unlock()
{ if (m_hmtx) DosReleaseMutexSem(m_hmtx); }

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
Fs_Dc *Fs_DcList::Create(ULONG iHwnd, ULONG iHab,
                         PBYTE pTarget, LONG width, LONG height, LONG bpp,
                         ULONG *pNewHdc)
{
    Fs_Dc *pFound = NULL;
    ULONG  iHdc = 0;
    int    i, nUsed = 0;

    if (pNewHdc) *pNewHdc = 0;
    Lock();
    for (i = 0; i < F_MAX_DCS; i++) if (m_aDcs[i].IsUsed()) nUsed++;
    if (nUsed >= F_MAX_DCS) {
        Unlock();
        F_DbgPrint(F_DBG_SEC_FS_SERVER, F_DBG_LVL_ERROR,
                   "[DCLIST] limit reached (%d/%d)", nUsed, F_MAX_DCS);
        return NULL;
    }
    for (i = 0; i < F_MAX_DCS; i++) {
        if (m_aDcs[i].IsUsed()) continue;
        iHdc = m_ulNextHdc++;
        if (iHdc == 0) iHdc = m_ulNextHdc++;
        if (m_aDcs[i].Init(iHdc, iHwnd, iHab, pTarget, width, height, bpp)) {
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
               (unsigned long)iHwnd, (unsigned long)iHab, (unsigned long)iHdc);
    return pFound;
}

/*!
 * @brief Find a DC by HDC.
 * @param [in] iHdc  HDC value.
 * @return Slot pointer.
 * @retval NULL     Not found.
 * @retval non-NULL Matching slot.
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
 * @param [in] iHdc  HDC value.
 * @return Success indicator.
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
 * @param [in] iHab  Owning HAB id.
 * @return Count of freed slots.
 * @retval 0    No DCs owned by iHab, or iHab == 0.
 * @retval >0   Count.
 */
int Fs_DcList::DestroyByHab(ULONG iHab)
{
    int n = 0, i;
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
 * @param [in] iHab  Owning HAB id.
 * @return Count.
 * @retval 0    No DCs, or iHab == 0.
 * @retval >0   Count.
 */
int Fs_DcList::CountByHab(ULONG iHab)
{
    int n = 0, i;
    if (iHab == 0) return 0;
    Lock();
    for (i = 0; i < F_MAX_DCS; i++)
        if (m_aDcs[i].IsUsed() && m_aDcs[i].GetHab() == iHab) n++;
    Unlock();
    return n;
}

/*!
 * @brief Count active DCs.
 * @return Count.
 * @retval >=0  Number of used slots.
 */
int Fs_DcList::CountActive()
{
    int n = 0, i;
    Lock();
    for (i = 0; i < F_MAX_DCS; i++) if (m_aDcs[i].IsUsed()) n++;
    Unlock();
    return n;
}
