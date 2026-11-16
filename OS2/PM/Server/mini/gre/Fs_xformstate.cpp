#include "Fs_xformstate.hpp"
#include <string.h>

#define XFORM_DATA_SIZE (sizeof(XFORM) * 4 + sizeof(ULONG) + sizeof(SIZEL) * 2 + \
                         sizeof(RECTL) * 3 + sizeof(BOOL))

/*!
 * @brief Construct with identity matrices and PU_PELS units.
 * @return Nothing (constructor).
 * @retval (void)  No return value.
 */
Fs_XformState::Fs_XformState()
    : m_ulUnits(PU_PELS), m_bViewingLimitsSet(FALSE),
      m_pSaveStack(NULL), m_ulSaveCount(0), m_ulSaveAlloc(0)
{
    m_sizlPage.cx = m_sizlPage.cy = 0;
    m_sizlDevice.cx = m_sizlDevice.cy = 0;
    memset(&m_rclPageViewport, 0, sizeof(RECTL));
    memset(&m_rclGraphicsField, 0, sizeof(RECTL));
    memset(&m_rclViewingLimits, 0, sizeof(RECTL));
    SetIdentity(m_model);
    SetIdentity(m_globalView);
    SetIdentity(m_windowViewport);
    SetIdentity(m_device);
}

/*!
 * @brief Destroy and release save stack.
 * @return Nothing (destructor).
 * @retval (void)  No return value.
 */
Fs_XformState::~Fs_XformState() { Shutdown(); }

/*!
 * @brief Reset to initial state.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_XformState::Init()
{
    m_ulUnits = PU_PELS;
    m_bViewingLimitsSet = FALSE;
    m_sizlPage.cx = m_sizlPage.cy = 0;
    m_sizlDevice.cx = m_sizlDevice.cy = 0;
    memset(&m_rclPageViewport, 0, sizeof(RECTL));
    memset(&m_rclGraphicsField, 0, sizeof(RECTL));
    memset(&m_rclViewingLimits, 0, sizeof(RECTL));
    SetIdentity(m_model);
    SetIdentity(m_globalView);
    SetIdentity(m_windowViewport);
    SetIdentity(m_device);
    if (m_pSaveStack) { DosFreeMem((PVOID)m_pSaveStack); m_pSaveStack = NULL; }
    m_ulSaveCount = m_ulSaveAlloc = 0;
}

/*!
 * @brief Alias for Init.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_XformState::Shutdown() { Init(); }

/*!
 * @brief Write identity matrix into x.
 * @param [out] x  Matrix to set.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_XformState::SetIdentity(XFORM &x) const
{
    x.fxM11 = MAKEFIXED(1, 0); x.fxM12 = 0;
    x.fxM21 = 0; x.fxM22 = MAKEFIXED(1, 0);
    x.lM41  = 0; x.lM42  = 0;
}

/*!
 * @brief Multiply dst = a * b.
 * @param [out] dst  Result.
 * @param [in]  a    Left.
 * @param [in]  b    Right.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_XformState::Multiply(XFORM &dst, const XFORM &a, const XFORM &b) const
{
    dst.fxM11 = (FIXED)(((LONG)a.fxM11 * b.fxM11 + (LONG)a.fxM21 * b.fxM12) >> 16);
    dst.fxM12 = (FIXED)(((LONG)a.fxM12 * b.fxM11 + (LONG)a.fxM22 * b.fxM12) >> 16);
    dst.fxM21 = (FIXED)(((LONG)a.fxM11 * b.fxM21 + (LONG)a.fxM21 * b.fxM22) >> 16);
    dst.fxM22 = (FIXED)(((LONG)a.fxM12 * b.fxM21 + (LONG)a.fxM22 * b.fxM22) >> 16);
    dst.lM41  = a.lM41 * b.fxM11 + a.lM42 * b.fxM21 + b.lM41;
    dst.lM42  = a.lM41 * b.fxM12 + a.lM42 * b.fxM22 + b.lM42;
}

/*!
 * @brief Get current page units and page extent.
 * @param [out] pExtent  Receives page dimensions.
 * @return Page units constant, or GPI_ERROR.
 * @retval GPI_ERROR  pExtent is NULL.
 * @retval PU_*       Current page units.
 *
 * @par EDM2 Reference
 * GreGetPageUnits — see header.
 */
LONG Fs_XformState::GetPageUnits(SIZEL *pExtent) const
{
    if (!pExtent) return GPI_ERROR;
    *pExtent = m_sizlPage;
    return (LONG)m_ulUnits;
}

/*!
 * @brief Set page units and page extent.
 * @param [in] ulUnits  PU_* units.
 * @param [in] lWidth   Page width.
 * @param [in] lHeight  Page height.
 * @return TRUE.
 * @retval TRUE  Always.
 *
 * @par EDM2 Reference
 * GreSetPageUnits — see header.
 */
BOOL Fs_XformState::SetPageUnits(ULONG ulUnits, LONG lWidth, LONG lHeight)
{
    if (ulUnits == PU_ARBITRARY && (lWidth == 0 || lHeight == 0)) {
        if (lWidth == 0 && lHeight == 0) {
            lWidth = m_sizlDevice.cx;
            lHeight = m_sizlDevice.cy;
        } else if (lWidth == 0) {
            lWidth = (m_sizlDevice.cy > 0)
                         ? (lHeight * m_sizlDevice.cx / m_sizlDevice.cy)
                         : lHeight;
        } else {
            lHeight = (m_sizlDevice.cx > 0)
                          ? (lWidth * m_sizlDevice.cy / m_sizlDevice.cx)
                          : lWidth;
        }
    }
    m_ulUnits = ulUnits;
    m_sizlPage.cx = lWidth;
    m_sizlPage.cy = lHeight;
    return TRUE;
}

/*!
 * @brief Query model transform matrix.
 * @param [out] p  Receives the matrix.
 * @return Success indicator.
 * @retval TRUE   Copied.
 * @retval FALSE  p is NULL.
 */
BOOL Fs_XformState::GetModelXform(XFORM *p) const
{ if (!p) return FALSE; *p = m_model; return TRUE; }

/*!
 * @brief Set model transform matrix.
 * @param [in] p          Pointer to XFORM.
 * @param [in] flOptions  SX_* option.
 * @return Success indicator.
 * @retval TRUE   Applied.
 * @retval FALSE  Invalid option.
 */
BOOL Fs_XformState::SetModelXform(const XFORM *p, LONG flOptions)
{
    if (!p && flOptions != SX_UNITY) return FALSE;
    switch (flOptions) {
    case SX_UNITY:      SetIdentity(m_model); break;
    case SX_OVERWRITE:  m_model = *p; break;
    case SX_CAT_BEFORE: { XFORM r; Multiply(r, *p, m_model); m_model = r; } break;
    case SX_CAT_AFTER:  { XFORM r; Multiply(r, m_model, *p); m_model = r; } break;
    default: return FALSE;
    }
    return TRUE;
}

/*!
 * @brief Query window/viewport transform matrix.
 * @param [out] p  Receives the matrix.
 * @return Success indicator.
 * @retval TRUE   Copied.
 * @retval FALSE  p is NULL.
 */
BOOL Fs_XformState::GetWindowViewportXform(XFORM *p) const
{ if (!p) return FALSE; *p = m_windowViewport; return TRUE; }

/*!
 * @brief Set window/viewport transform matrix.
 * @param [in] p          Pointer to XFORM.
 * @param [in] flOptions  SX_* option.
 * @return Success indicator.
 * @retval TRUE   Applied.
 * @retval FALSE  Invalid option.
 */
BOOL Fs_XformState::SetWindowViewportXform(const XFORM *p, LONG flOptions)
{
    if (!p && flOptions != SX_UNITY) return FALSE;
    switch (flOptions) {
    case SX_UNITY:      SetIdentity(m_windowViewport); break;
    case SX_OVERWRITE:  m_windowViewport = *p; break;
    case SX_CAT_BEFORE: { XFORM r; Multiply(r, *p, m_windowViewport); m_windowViewport = r; } break;
    case SX_CAT_AFTER:  { XFORM r; Multiply(r, m_windowViewport, *p); m_windowViewport = r; } break;
    default: return FALSE;
    }
    return TRUE;
}

/*!
 * @brief Query global viewing transform matrix.
 * @param [out] p  Receives the matrix.
 * @return Success indicator.
 * @retval TRUE   Copied.
 * @retval FALSE  p is NULL.
 */
BOOL Fs_XformState::GetGlobalViewingXform(XFORM *p) const
{ if (!p) return FALSE; *p = m_globalView; return TRUE; }

/*!
 * @brief Set global viewing transform matrix.
 * @param [in] p          Pointer to XFORM.
 * @param [in] flOptions  SX_* option.
 * @return Success indicator.
 * @retval TRUE   Applied.
 * @retval FALSE  Invalid option.
 */
BOOL Fs_XformState::SetGlobalViewingXform(const XFORM *p, LONG flOptions)
{
    if (!p && flOptions != SX_UNITY) return FALSE;
    switch (flOptions) {
    case SX_UNITY:      SetIdentity(m_globalView); break;
    case SX_OVERWRITE:  m_globalView = *p; break;
    case SX_CAT_BEFORE: { XFORM r; Multiply(r, *p, m_globalView); m_globalView = r; } break;
    case SX_CAT_AFTER:  { XFORM r; Multiply(r, m_globalView, *p); m_globalView = r; } break;
    default: return FALSE;
    }
    return TRUE;
}

/*!
 * @brief Query page viewport rectangle.
 * @param [out] p  Receives the viewport.
 * @return Success indicator.
 * @retval TRUE   Copied.
 * @retval FALSE  p is NULL.
 */
BOOL Fs_XformState::GetPageViewport(RECTL *p) const
{ if (!p) return FALSE; *p = m_rclPageViewport; return TRUE; }

/*!
 * @brief Set page viewport rectangle.
 * @param [in] p  Pointer to viewport.
 * @return Success indicator.
 * @retval TRUE   Set.
 * @retval FALSE  NULL or degenerate.
 */
BOOL Fs_XformState::SetPageViewport(const RECTL *p)
{
    if (!p) return FALSE;
    if (p->xLeft >= p->xRight || p->yBottom >= p->yTop) return FALSE;
    m_rclPageViewport = *p;
    return TRUE;
}

/*!
 * @brief Query graphics field rectangle.
 * @param [out] p  Receives the field.
 * @return Success indicator.
 * @retval TRUE   Copied.
 * @retval FALSE  p is NULL.
 */
BOOL Fs_XformState::GetGraphicsField(RECTL *p) const
{ if (!p) return FALSE; *p = m_rclGraphicsField; return TRUE; }

/*!
 * @brief Set graphics field rectangle.
 * @param [in] p  Pointer to graphics field.
 * @return Success indicator.
 * @retval TRUE   Set.
 * @retval FALSE  NULL or degenerate.
 */
BOOL Fs_XformState::SetGraphicsField(const RECTL *p)
{
    if (!p) return FALSE;
    if (p->xLeft >= p->xRight || p->yBottom >= p->yTop) return FALSE;
    m_rclGraphicsField = *p;
    return TRUE;
}

/*!
 * @brief Query viewing limits rectangle.
 * @param [out] p  Receives the limits.
 * @return Success indicator.
 * @retval TRUE   Copied (infinite bounds when unset).
 * @retval FALSE  p is NULL.
 */
BOOL Fs_XformState::GetViewingLimits(RECTL *p) const
{
    if (!p) return FALSE;
    if (!m_bViewingLimitsSet) {
        p->xLeft = 0x7FFFFFFF; p->yBottom = 0x7FFFFFFF;
        p->xRight = -0x7FFFFFFF; p->yTop = -0x7FFFFFFF;
    } else {
        *p = m_rclViewingLimits;
    }
    return TRUE;
}

/*!
 * @brief Set viewing limits rectangle.
 * @param [in] p  Pointer to limits.
 * @return Success indicator.
 * @retval TRUE   Set.
 * @retval FALSE  p is NULL.
 */
BOOL Fs_XformState::SetViewingLimits(const RECTL *p)
{
    if (!p) return FALSE;
    m_rclViewingLimits = *p;
    m_bViewingLimitsSet = TRUE;
    return TRUE;
}

/*!
 * @brief Query viewport size.
 * @param [in]  ulType     Reserved.
 * @param [out] psizlSize  Receives SIZEL.
 * @return Success indicator.
 * @retval TRUE   Copied.
 * @retval FALSE  psizlSize is NULL.
 */
BOOL Fs_XformState::QueryViewportSize(ULONG ulType, SIZEL *psizlSize) const
{
    (void)ulType;
    if (!psizlSize) return FALSE;
    psizlSize->cx = m_rclPageViewport.xRight - m_rclPageViewport.xLeft;
    psizlSize->cy = m_rclPageViewport.yTop    - m_rclPageViewport.yBottom;
    return TRUE;
}

/*!
 * @brief Compose transform between two coordinate spaces.
 * @param [out] xform  Resulting XFORM.
 * @param [in]  lSrc   Source CVTC_*.
 * @param [in]  lDst   Destination CVTC_*.
 * @return Nothing.
 * @retval (void)  No return value.
 * @note Inverse path (lSrc > lDst) uses standard 2x2 inversion; singular
 *       matrices fall back to identity.
 */
void Fs_XformState::BuildTransform(XFORM &xform, LONG lSrc, LONG lDst) const
{
    SetIdentity(xform);
    LONG lFrom = lSrc, lTo = lDst;
    if (lFrom > lTo) { LONG t = lFrom; lFrom = lTo; lTo = t; }

    for (LONG s = lFrom; s < lTo; s++) {
        XFORM step;
        switch (s) {
        case CVTC_WORLD:                          step = m_model;          break;
        case CVTC_MODEL:                          step = m_globalView;     break;
        case CVTC_DEFAULTPAGE: case CVTC_PAGE:    step = m_windowViewport; break;
        case CVTC_DEVICE:                         step = m_device;         break;
        default:                                  SetIdentity(step);       break;
        }
        XFORM r;
        Multiply(r, xform, step);
        xform = r;
    }

    if (lSrc > lDst) {
        FIXED det = (FIXED)(((LONG)xform.fxM11 * xform.fxM22 -
                             (LONG)xform.fxM12 * xform.fxM21) >> 16);
        if (det == 0) { SetIdentity(xform); return; }
        XFORM inv;
        inv.fxM11 = (FIXED)(((LONG)xform.fxM22 << 16) / det);
        inv.fxM12 = (FIXED)(-(((LONG)xform.fxM12 << 16) / det));
        inv.fxM21 = (FIXED)(-(((LONG)xform.fxM21 << 16) / det));
        inv.fxM22 = (FIXED)(((LONG)xform.fxM11 << 16) / det);
        inv.lM41  = -((LONG)inv.fxM11 * xform.lM41 + (LONG)inv.fxM21 * xform.lM42);
        inv.lM42  = -((LONG)inv.fxM12 * xform.lM41 + (LONG)inv.fxM22 * xform.lM42);
        xform = inv;
    }
}

/*!
 * @brief Convert coordinates between spaces.
 * @param [in]     lSrc        Source space.
 * @param [in]     lDst        Destination space.
 * @param [in,out] paptlPoint  Array of POINTL.
 * @param [in]     cPoints     Number of points.
 * @return Success indicator.
 * @retval TRUE   Converted.
 * @retval FALSE  NULL or cPoints < 0.
 *
 * @par EDM2 Reference
 * GreConvert — see header.
 */
BOOL Fs_XformState::Convert(LONG lSrc, LONG lDst, PPOINTL paptlPoint,
                             LONG cPoints) const
{
    if (!paptlPoint || cPoints < 0) return FALSE;
    XFORM x;
    BuildTransform(x, lSrc, lDst);
    for (LONG i = 0; i < cPoints; i++) {
        LONG xr = ((LONG)x.fxM11 * paptlPoint[i].x +
                   (LONG)x.fxM21 * paptlPoint[i].y) + x.lM41;
        LONG yr = ((LONG)x.fxM12 * paptlPoint[i].x +
                   (LONG)x.fxM22 * paptlPoint[i].y) + x.lM42;
        paptlPoint[i].x = xr;
        paptlPoint[i].y = yr;
    }
    return TRUE;
}

/*!
 * @brief Convert coordinates using explicit matrix.
 * @param [in,out] paptlPoint  Array of POINTL.
 * @param [in]     cPoints     Number of points.
 * @param [in]     pxform      Transform matrix.
 * @return Success indicator.
 * @retval TRUE   Converted.
 * @retval FALSE  NULL or cPoints < 0.
 *
 * @par EDM2 Reference
 * GreConvertWithMatrix — see header.
 */
BOOL Fs_XformState::ConvertWithMatrix(PPOINTL paptlPoint, LONG cPoints,
                                       const XFORM *pxform) const
{
    if (!paptlPoint || cPoints < 0 || !pxform) return FALSE;
    for (LONG i = 0; i < cPoints; i++) {
        LONG x = ((LONG)pxform->fxM11 * paptlPoint[i].x +
                  (LONG)pxform->fxM21 * paptlPoint[i].y) + pxform->lM41;
        LONG y = ((LONG)pxform->fxM12 * paptlPoint[i].x +
                  (LONG)pxform->fxM22 * paptlPoint[i].y) + pxform->lM42;
        paptlPoint[i].x = x;
        paptlPoint[i].y = y;
    }
    return TRUE;
}

/*!
 * @brief Multiply transform matrix by another.
 * @param [in,out] paXform        In/out.
 * @param [in]     paNewXformData Multiplier.
 * @param [in]     lMode          SX_* mode.
 * @return Success indicator.
 * @retval TRUE   Multiplied.
 * @retval FALSE  NULL or unknown mode.
 *
 * @par EDM2 Reference
 * GreMultiplyXforms — see header.
 */
BOOL Fs_XformState::MultiplyXforms(XFORM *paXform, const XFORM *paNewXformData,
                                    LONG lMode) const
{
    if (!paXform || !paNewXformData) return FALSE;
    XFORM r;
    switch (lMode) {
    case SX_CAT_BEFORE: Multiply(r, *paNewXformData, *paXform); break;
    case SX_CAT_AFTER:  Multiply(r, *paXform, *paNewXformData); break;
    case SX_OVERWRITE:  r = *paNewXformData; break;
    default: return FALSE;
    }
    *paXform = r;
    return TRUE;
}

/*!
 * @brief Save transform state.
 * @param [in] cSave  Save level (1 = init, N>=2 = push).
 * @return Success indicator.
 * @retval TRUE   Saved.
 * @retval FALSE  Negative level or alloc failure.
 *
 * @par EDM2 Reference
 * GreSaveXform — see header.
 */
BOOL Fs_XformState::SaveXform(LONG cSave)
{
    if (cSave < 0) return FALSE;
    if (cSave == 1) { Init(); return TRUE; }

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
    s.model            = m_model;
    s.globalView       = m_globalView;
    s.windowViewport   = m_windowViewport;
    s.device           = m_device;
    s.ulUnits          = m_ulUnits;
    s.sizlPage         = m_sizlPage;
    s.sizlDevice       = m_sizlDevice;
    s.rclPageViewport  = m_rclPageViewport;
    s.rclGraphicsField = m_rclGraphicsField;
    s.rclViewingLimits = m_rclViewingLimits;
    s.bViewingLimitsSet = m_bViewingLimitsSet;
    m_ulSaveCount++;
    return TRUE;
}

/*!
 * @brief Restore transform state.
 * @param [in] cSave  -1/0/1 = reset; N>=2 = restore level N.
 * @return Success indicator.
 * @retval TRUE   Restored.
 * @retval FALSE  Level out of range.
 *
 * @par EDM2 Reference
 * GreRestoreXform — see header.
 */
BOOL Fs_XformState::RestoreXform(LONG cSave)
{
    if (cSave <= 0 || cSave == 1) { Init(); return TRUE; }
    ULONG idx = (ULONG)cSave - 2;
    if (idx >= m_ulSaveCount) return FALSE;
    SaveState &s = m_pSaveStack[idx];
    m_model            = s.model;
    m_globalView       = s.globalView;
    m_windowViewport   = s.windowViewport;
    m_device           = s.device;
    m_ulUnits          = s.ulUnits;
    m_sizlPage         = s.sizlPage;
    m_sizlDevice       = s.sizlDevice;
    m_rclPageViewport  = s.rclPageViewport;
    m_rclGraphicsField = s.rclGraphicsField;
    m_rclViewingLimits = s.rclViewingLimits;
    m_bViewingLimitsSet = s.bViewingLimitsSet;
    m_ulSaveCount = idx + 1;
    return TRUE;
}

/*!
 * @brief Save transform state to caller buffer.
 * @param [in]  ulSize   Buffer size, or 0 to query required size.
 * @param [out] pBuffer  Caller buffer.
 * @return Required size, GPI_OK, or GPI_ERROR.
 * @retval 0           ulSize == 0.
 * @retval GPI_OK      Success.
 * @retval GPI_ERROR   Buffer too small or NULL.
 *
 * @par EDM2 Reference
 * GreSaveXformData — see header.
 */
ULONG Fs_XformState::SaveXformData(ULONG ulSize, PBYTE pBuffer) const
{
    if (ulSize == 0) return XFORM_DATA_SIZE;
    if (ulSize < XFORM_DATA_SIZE || !pBuffer) return GPI_ERROR;
    BYTE *p = pBuffer;
    memcpy(p, &m_model, sizeof(XFORM));          p += sizeof(XFORM);
    memcpy(p, &m_globalView, sizeof(XFORM));     p += sizeof(XFORM);
    memcpy(p, &m_windowViewport, sizeof(XFORM)); p += sizeof(XFORM);
    memcpy(p, &m_device, sizeof(XFORM));         p += sizeof(XFORM);
    memcpy(p, &m_ulUnits, sizeof(ULONG));        p += sizeof(ULONG);
    memcpy(p, &m_sizlPage, sizeof(SIZEL));       p += sizeof(SIZEL);
    memcpy(p, &m_sizlDevice, sizeof(SIZEL));     p += sizeof(SIZEL);
    memcpy(p, &m_rclPageViewport, sizeof(RECTL)); p += sizeof(RECTL);
    memcpy(p, &m_rclGraphicsField, sizeof(RECTL)); p += sizeof(RECTL);
    memcpy(p, &m_rclViewingLimits, sizeof(RECTL)); p += sizeof(RECTL);
    memcpy(p, &m_bViewingLimitsSet, sizeof(BOOL));
    return GPI_OK;
}

/*!
 * @brief Restore transform state from caller buffer.
 * @param [in] ulSize   Buffer size.
 * @param [in] pBuffer  Caller buffer.
 * @return Success indicator.
 * @retval TRUE   Restored.
 * @retval FALSE  Buffer too small or NULL.
 *
 * @par EDM2 Reference
 * GreRestoreXformData — see header.
 */
BOOL Fs_XformState::RestoreXformData(ULONG ulSize, const BYTE *pBuffer)
{
    if (ulSize < XFORM_DATA_SIZE || !pBuffer) return FALSE;
    const BYTE *p = pBuffer;
    memcpy(&m_model, p, sizeof(XFORM));          p += sizeof(XFORM);
    memcpy(&m_globalView, p, sizeof(XFORM));     p += sizeof(XFORM);
    memcpy(&m_windowViewport, p, sizeof(XFORM)); p += sizeof(XFORM);
    memcpy(&m_device, p, sizeof(XFORM));         p += sizeof(XFORM);
    memcpy(&m_ulUnits, p, sizeof(ULONG));        p += sizeof(ULONG);
    memcpy(&m_sizlPage, p, sizeof(SIZEL));       p += sizeof(SIZEL);
    memcpy(&m_sizlDevice, p, sizeof(SIZEL));     p += sizeof(SIZEL);
    memcpy(&m_rclPageViewport, p, sizeof(RECTL)); p += sizeof(RECTL);
    memcpy(&m_rclGraphicsField, p, sizeof(RECTL)); p += sizeof(RECTL);
    memcpy(&m_rclViewingLimits, p, sizeof(RECTL)); p += sizeof(RECTL);
    memcpy(&m_bViewingLimitsSet, p, sizeof(BOOL));
    return TRUE;
}
