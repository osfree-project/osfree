/*!
 * @file Fs_arc.cpp
 * @brief Arc, fillet and box primitives for one DC.
 *
 * All drawing is tessellated into line segments and emitted into the
 * owning Fs_Path.
 */

#include "Fs_arc.hpp"
#include "Fs_path.hpp"
#include "Fs_dc.hpp"

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
 */
void Fs_Arc::Init() { memset(&m_arcParms, 0, sizeof(ARCPARAMS)); }

/*!
 * @brief Alias for Init.
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
//BOOL Fs_Arc::QueryArcParameters(ARCPARAMS *pArcParms) const
//{
//    if (!pArcParms) return FALSE;
//    *pArcParms = m_arcParms;
//    return TRUE;
//}

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
 * @retval GPI_OK    Arc emitted.
 * @retval GPI_HITS  Correlation hit.
 * @retval GPI_ERROR pptlCenter is NULL.
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
 * @param [in] pDc   Device context; supplies the current (X,Y) position.
 * @param [in] pBox  Pointer to BOXDATA (ptlOpposite, lHRound, lVRound).
 * @return cHits code.
 * @retval GPI_OK    Figure emitted.
 * @retval GPI_HITS  Correlation hit.
 * @retval GPI_ERROR Invalid DC or pBox.
 *
 * @par EDM2 Reference
 * GreBoxInterior — see header.
 */
LONG Fs_Arc::BoxInterior(Fs_Dc *pDc, const BOXDATA *pBox)
{
    LONG rc = BoxBoundary(pDc, pBox);
    if (rc != GPI_OK) return rc;
    // TODO: fill the closed path via Fs_Area
    return GPI_OK;
}

/*! @brief Quarter turn in the FIXED units EmitArc expects (pi/2 * 65536). */
static const FIXED FX_QUARTER = 102944;

/*!
* @brief Draw a rectangular box boundary with optional rounded corners.
* @param [in] pDc   Device context; supplies the current (X,Y) position.
* @param [in] pBox  Pointer to BOXDATA (ptlOpposite, lHRound, lVRound).
* @return cHits code.
* @retval GPI_OK    Figure emitted.
* @retval GPI_HITS  Correlation hit.
* @retval GPI_ERROR pDc or pBox is NULL.
*
* @par EDM2 Reference
* GreBoxBoundary — first corner is the current position, opposite corner
* is pBox->ptlOpposite. Sides are parallel to axes. Corners rounded by
* BOXDATA.lHRound/lVRound (DIAMETERS). Drawn counterclockwise.
*/
LONG Fs_Arc::BoxBoundary(Fs_Dc *pDc, const BOXDATA *pBox)
{
    if (!pDc || !pBox) return GPI_ERROR;

    POINTL start = pDc->GetCurPos();

    LONG x0 = start.x, y0 = start.y;
    LONG x1 = pBox->ptlOpposite.x, y1 = pBox->ptlOpposite.y;

    if (x0 > x1) { LONG t = x0; x0 = x1; x1 = t; }
    if (y0 > y1) { LONG t = y0; y0 = y1; y1 = t; }

    LONG width  = x1 - x0;
    LONG height = y1 - y0;

    // BOXDATA.lHRound/lVRound are DIAMETERS; clamp to the side length
    LONG rx = pBox->lHRound / 2;
    LONG ry = pBox->lVRound / 2;

    if (rx > width  / 2) rx = width  / 2;
    if (ry > height / 2) ry = height / 2;

    // Either diameter 0 (or negative) disables rounding
    if (rx <= 0 || ry <= 0) {
        POINTL p[5] = {
            {x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}, {x0, y0}
        };
        for (int i = 0; i < 4; i++) EmitLine(p[i], p[i + 1]);
        return GPI_OK;
    }

    // Rounded rectangle counterclockwise: 4 edges + 4 quarter-ellipse
    // corners.  Each arc starts where the previous edge ended; the last
    // arc closes the figure back at (x0 + rx, y0).
    POINTL a, b, c;

    // Bottom edge, then bottom-right corner
    a.x = x0 + rx; a.y = y0;
    b.x = x1 - rx; b.y = y0;
    EmitLine(a, b);
    c.x = x1 - rx; c.y = y0 + ry;
    EmitArc(c, rx, ry, -FX_QUARTER, FX_QUARTER);

    // Right edge, then top-right corner
    a.x = x1;      a.y = y0 + ry;
    b.x = x1;      b.y = y1 - ry;
    EmitLine(a, b);
    c.x = x1 - rx; c.y = y1 - ry;
    EmitArc(c, rx, ry, 0, FX_QUARTER);

    // Top edge, then top-left corner
    a.x = x1 - rx; a.y = y1;
    b.x = x0 + rx; b.y = y1;
    EmitLine(a, b);
    c.x = x0 + rx; c.y = y1 - ry;
    EmitArc(c, rx, ry, FX_QUARTER, FX_QUARTER);

    // Left edge, then bottom-left corner (closes the figure)
    a.x = x0;      a.y = y1 - ry;
    b.x = x0;      b.y = y0 + ry;
    EmitLine(a, b);
    c.x = x0 + rx; c.y = y0 + ry;
    EmitArc(c, rx, ry, 2 * FX_QUARTER, FX_QUARTER);

    return GPI_OK;
}

/*!
* @brief Draw and fill a rectangular box (interior and boundary).
* @param [in] pDc   Device context; supplies the current (X,Y) position.
* @param [in] pBox  Pointer to BOXPARAMS (ptl, sizl.cx, sizl.cy).
* @return cHits code.
* @retval GPI_OK    Figure emitted.
* @retval GPI_HITS  Correlation hit.
* @retval GPI_ERROR Invalid DC or pBox.
*
* @par EDM2 Reference
* GreBoxBoth — pBox is PBOXPARAMS, but it is same as PBOXDATA.
*/
LONG Fs_Arc::BoxBoth(Fs_Dc *pDc, const BOXPARAMS *pBox)
{
    if (!pDc || !pBox) return GPI_ERROR;
    return BoxInterior(pDc, (const BOXDATA *)pBox);
}

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
