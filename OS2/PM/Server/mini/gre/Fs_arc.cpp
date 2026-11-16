#include "Fs_arc.hpp"
#include "Fs_path.hpp"
#include <string.h>
#include <math.h>

/*!
 * @brief Construct with zeroed ARCPARAMS.
 * @return Nothing (constructor).
 * @retval (void)  No return value.
 */
Fs_Arc::Fs_Arc() : m_pPath(NULL)
{
    memset(&m_arcParms, 0, sizeof(ARCPARAMS));
}

/*!
 * @brief Destroy.
 * @return Nothing (destructor).
 * @retval (void)  No return value.
 */
Fs_Arc::~Fs_Arc() { Shutdown(); }

/*!
 * @brief Reset ARCPARAMS to zero.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Arc::Init() { memset(&m_arcParms, 0, sizeof(ARCPARAMS)); }

/*!
 * @brief Alias for Init.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Arc::Shutdown() { Init(); }

/*!
 * @brief Set the current arc parameters.
 * @param [in] pArcParms  Pointer to ARCPARAMS.
 * @return Success indicator.
 * @retval TRUE   Stored.
 * @retval FALSE  pArcParms is NULL.
 *
 * @par EDM2 Reference
 * GreSetArcParameters — see header.
 */
BOOL Fs_Arc::SetArcParameters(const ARCPARAMS *pArcParms)
{
    if (!pArcParms) return FALSE;
    m_arcParms = *pArcParms;
    return TRUE;
}

/*!
 * @brief Query the current arc parameters.
 * @param [out] pArcParms  Receives ARCPARAMS.
 * @return Success indicator.
 * @retval TRUE   Copied.
 * @retval FALSE  pArcParms is NULL.
 */
BOOL Fs_Arc::QueryArcParameters(ARCPARAMS *pArcParms) const
{
    if (!pArcParms) return FALSE;
    *pArcParms = m_arcParms;
    return TRUE;
}

/*!
 * @brief Emit a line segment into m_pPath.
 * @param [in] a  Start point.
 * @param [in] b  End point.
 * @return GPI_OK.
 * @retval GPI_OK  Always.
 */
LONG Fs_Arc::EmitLine(const POINTL &a, const POINTL &b)
{
    if (m_pPath) m_pPath->AddLine(a, b);
    return GPI_OK;
}

/*!
 * @brief Tessellate an elliptical arc into line segments.
 * @param [in] center   Center.
 * @param [in] rx       X radius.
 * @param [in] ry       Y radius.
 * @param [in] fxStart  Start angle (FIXED).
 * @param [in] fxSweep  Sweep angle (FIXED).
 * @return GPI_OK.
 * @retval GPI_OK  Always.
 * @note 64 segments.  Uses cos/sin on the FIXED-to-radians conversion.
 */
LONG Fs_Arc::EmitArc(const POINTL &center, LONG rx, LONG ry,
                     FIXED fxStart, FIXED fxSweep)
{
    const int SEGMENTS = 64;
    LONG lStart = (LONG)fxStart;
    LONG lSweep = (LONG)fxSweep;
    POINTL prev;
    for (int i = 0; i <= SEGMENTS; i++) {
        LONG t = lStart + (lSweep * i) / SEGMENTS;
        double rad = (double)t / 65536.0;
        POINTL cur;
        cur.x = center.x + (LONG)((double)rx * cos(rad));
        cur.y = center.y + (LONG)((double)ry * sin(rad));
        if (i > 0) EmitLine(prev, cur);
        prev = cur;
    }
    return GPI_OK;
}

/*!
 * @brief Draw an arc through three points.
 * @param [in] paptlPoint  Array of 3 POINTL.
 * @return cHits code.
 * @retval GPI_OK    Drawn (two segments).
 * @retval GPI_HITS  Correlation hit.
 * @retval GPI_ERROR paptlPoint is NULL.
 *
 * @par EDM2 Reference
 * GreArc — see header.
 *
 * @warning Approximated by two straight segments; full three-point
 *          circle math is not implemented.
 */
LONG Fs_Arc::Arc(const POINTL *paptlPoint)
{
    if (!paptlPoint) return GPI_ERROR;
    POINTL start;
    if (m_pPath && m_pPath->LineCount() > 0)
        start = m_pPath->Lines()[m_pPath->LineCount() - 1].ptlC;
    else
        start = paptlPoint[0];
    EmitLine(start, paptlPoint[0]);
    EmitLine(paptlPoint[0], paptlPoint[1]);
    return GPI_OK;
}

/*!
 * @brief Draw a partial arc.
 * @param [in] pptlCenter   Center of arc.
 * @param [in] fxMultiplier FIXED multiplier.
 * @param [in] fxStart      FIXED start angle.
 * @param [in] fxSweep      FIXED sweep angle.
 * @return cHits code.
 * @retval GPI_OK / GPI_HITS / GPI_ERROR.
 *
 * @par EDM2 Reference
 * GrePartialArc — see header.
 */
LONG Fs_Arc::PartialArc(const POINTL *pptlCenter, FIXED fxMultiplier,
                        FIXED fxStart, FIXED fxSweep)
{
    if (!pptlCenter) return GPI_ERROR;
    LONG rx = (LONG)(((LONG)m_arcParms.lP * (LONG)fxMultiplier) >> 16);
    LONG ry = (LONG)(((LONG)m_arcParms.lQ * (LONG)fxMultiplier) >> 16);
    return EmitArc(*pptlCenter, rx, ry, fxStart, fxSweep);
}

/*!
 * @brief Draw a filled full arc.
 * @param [in] fxMultiplier  FIXED multiplier.
 * @return cHits code.
 * @retval GPI_OK / GPI_HITS / GPI_ERROR.
 *
 * @par EDM2 Reference
 * GreFullArcInterior — see header.
 */
LONG Fs_Arc::FullArcInterior(FIXED fxMultiplier)
{
    POINTL c = { 0, 0 };
    LONG rx = (LONG)(((LONG)m_arcParms.lP * (LONG)fxMultiplier) >> 16);
    LONG ry = (LONG)(((LONG)m_arcParms.lQ * (LONG)fxMultiplier) >> 16);
    return EmitArc(c, rx, ry, 0, 0x20000 /* 2*pi in FIXED */);
}

/*!
 * @brief Draw a full arc boundary.
 * @param [in] fxMultiplier  FIXED multiplier.
 * @return cHits code.
 * @retval GPI_OK / GPI_HITS / GPI_ERROR.
 */
LONG Fs_Arc::FullArcBoundary(FIXED fxMultiplier)
{ return FullArcInterior(fxMultiplier); }

/*!
 * @brief Draw a full arc (both).
 * @param [in] fxMultiplier  FIXED multiplier.
 * @return cHits code.
 * @retval GPI_OK / GPI_HITS / GPI_ERROR.
 */
LONG Fs_Arc::FullArcBoth(FIXED fxMultiplier)
{ return FullArcInterior(fxMultiplier); }

/*!
 * @brief Draw a rectangular box (interior).
 * @param [in] pBox  Pointer to BOXPARAMS.
 * @return cHits code.
 * @retval GPI_OK / GPI_HITS / GPI_ERROR.
 *
 * @par EDM2 Reference
 * GreBoxInterior — see header.
 */
LONG Fs_Arc::BoxInterior(const BOXPARAMS *pBox)
{
    if (!pBox) return GPI_ERROR;
    POINTL p[5];
    p[0] = pBox->ptl;
    p[1].x = pBox->ptl.x + pBox->sizl.cx; p[1].y = pBox->ptl.y;
    p[2].x = pBox->ptl.x + pBox->sizl.cx; p[2].y = pBox->ptl.y + pBox->sizl.cy;
    p[3].x = pBox->ptl.x;                 p[3].y = pBox->ptl.y + pBox->sizl.cy;
    p[4] = p[0];
    for (int i = 0; i < 4; i++) EmitLine(p[i], p[i + 1]);
    return GPI_OK;
}

/*!
 * @brief Draw a rectangular box (boundary).
 * @param [in] pBox  Pointer to BOXPARAMS.
 * @return cHits code.
 * @retval GPI_OK / GPI_HITS / GPI_ERROR.
 */
LONG Fs_Arc::BoxBoundary(const BOXPARAMS *pBox)
{ return BoxInterior(pBox); }

/*!
 * @brief Draw a rectangular box (both).
 * @param [in] pBox  Pointer to BOXPARAMS.
 * @return cHits code.
 * @retval GPI_OK / GPI_HITS / GPI_ERROR.
 */
LONG Fs_Arc::BoxBoth(const BOXPARAMS *pBox)
{ return BoxInterior(pBox); }

/*!
 * @brief Draw a poly-fillet.
 * @param [in] paptlPoint  Array of points.
 * @param [in] cPoints     Number of points.
 * @return cHits code.
 * @retval GPI_OK / GPI_HITS / GPI_ERROR.
 *
 * @par EDM2 Reference
 * GrePolyFillet — see header.
 */
LONG Fs_Arc::PolyFillet(const POINTL *paptlPoint, LONG cPoints)
{
    if (!paptlPoint || cPoints == 0) return GPI_OK;
    for (LONG i = 0; i + 1 < cPoints; i++)
        EmitLine(paptlPoint[i], paptlPoint[i + 1]);
    return GPI_OK;
}

/*!
 * @brief Draw a sharp poly-fillet.
 * @param [in] paptlPoint  Array of points.
 * @param [in] cPoints     Number of points.
 * @param [in] pfxSharp    Array of sharpness values (unused).
 * @return cHits code.
 * @retval GPI_OK / GPI_HITS / GPI_ERROR.
 *
 * @par EDM2 Reference
 * GrePolyFilletSharp — see header.
 */
LONG Fs_Arc::PolyFilletSharp(const POINTL *paptlPoint, LONG cPoints,
                             const FIXED *pfxSharp)
{
    (void)pfxSharp;
    return PolyFillet(paptlPoint, cPoints);
}
