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


/*!
 * @brief Dispatch for GreGetArcParameters — retrieve the current arc parameters.
 *
 * @param [in]  hdc         Device context handle.
 * @param [out] pArcParms   Pointer to an ARCPARAMS structure that receives
 *                          the current arc parameters.
 * @param [in]  pInstance   Pointer to instance data.
 * @param [in]  lFunction   High-order WORD=flags; low-order
 *                          WORD=NGreGetArcParameters.
 *
 * @return BOOLEAN success indicator.
 * @retval TRUE   Successful.
 * @retval FALSE  Error.
 *
 * @par EDM2 Reference
 * GreGetArcParameters (https://www.edm2.com/index.php/GreGetArcParameters):
 * "GreGetArcParameters stores the current arc parameters in the buffer
 * addressed by pArcParms."
 *
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_HDC_BUSY
 * - PMERR_INV_HDC
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
static BOOL __cdecl GetArcParameters(HDC hdc, PARCPARAMS pArcParms,
                                             PVOID pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetArc().QueryArcParameters(pArcParms);
}

/*!
 * @brief Dispatch for GreSetArcParameters — set the current arc parameters.
 *
 * @param [in] hdc         Device context handle.
 * @param [in] pArcParms   Pointer to an ARCPARAMS structure containing the
 *                         new arc parameters.
 * @param [in] pInstance   Pointer to instance data.
 * @param [in] lFunction   High-order WORD=flags; low-order
 *                         WORD=NGreSetArcParameters.
 *
 * @return BOOLEAN success indicator.
 * @retval TRUE   Successful.
 * @retval FALSE  Error.
 *
 * @par EDM2 Reference
 * GreSetArcParameters (https://www.edm2.com/index.php/GreSetArcParameters):
 * "GreSetArcParameters sets the arc parameters."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_HDC_BUSY
 * - PMERR_INV_HDC
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
static BOOL __cdecl SetArcParameters(HDC hdc, PARCPARAMS pArcParms,
                                             PVOID pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetArc().SetArcParameters((const ARCPARAMS *)pArcParms);
}

/*!
 * @brief Dispatch for GreArc — draw an arc through three points.
 *
 * @param [in] hdc         Device context handle.
 * @param [in] paptlPoint  Pointer to an array of POINTL structures giving
 *                         the mid- and end-points of the arc. If the
 *                         mid-point is coincident with the start or end
 *                         point, a straight line is drawn from the start
 *                         point to the end point.
 * @param [in] pInstance   Pointer to instance data.
 * @param [in] lFunction   High-order WORD=flags; low-order WORD=NGreArc.
 *
 * @return Integer (cHits) indicating correlation/error status.
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit (returned by display
 *                   drivers when the correlation flag is ON, and a hit
 *                   is detected).
 * @retval GPI_ERROR Error.
 *
 * @par EDM2 Reference
 * GreArc (https://www.edm2.com/index.php/GreArc):
 * "GreArc draws an arc through three points. These are the current
 * position and the two points specified in the data structure. Upon
 * completion, the current position is the third point of the arc."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_BASE_ERROR
 * - PMERR_BITMAP_NOT_SELECTED
 * - PMERR_COORDINATE_OVERFLOW
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_HDC_BUSY
 * - PMERR_INV_COLOR_DATA
 * - PMERR_INV_COLOR_INDEX
 * - PMERR_INV_COORD_SPACE
 * - PMERR_INV_HDC
 * - PMERR_INV_IN_AREA
 * - PMERR_INV_IN_PATH
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_INV_MATRIX_ELEMENT
 * - PMERR_INV_PICK_APERTURE_POSN
 * - PMERR_INV_RECT
 * - PMERR_PATH_LIMIT_EXCEEDED
 * - PMERR_PATH_UNKNOWN
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 * @par Remarks
 * "If GreArc is used within a path definition or an area definition to
 * continue a figure following a GreBoxxxx or GreFullArcxxx function, the
 * error PMERR_INV_NESTED_FIGURES is posted. This is because the GreBoxxxx
 * and GreFullArcxxx functions generate a closed figure within an area or
 * path definition."
 *
 */
static LONG __cdecl Arc(HDC hdc, PPOINTL paptlPoint,
                                PVOID pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return GPI_ERROR;
    return (ULONG)pDc->GetArc().Arc(paptlPoint);
}

/*!
 * @brief Dispatch for GrePartialArc — draw a partial arc from the current position.
 *
 * @param [in] hdc          Device context handle.
 * @param [in] pptlCenter   Pointer to (X,Y) coordinates for the center of the arc.
 *                          If COM_TRANSFORM is not set, the function expects
 *                          the center of the arc position to be in screen
 *                          coordinates.
 * @param [in] fxMultiplier Multiplier defining the size of the full arc in
 *                          relation to an arc drawn with the current arc
 *                          parameters. The multiplier is a fixed-point value.
 *                          The high-order WORD contains the integer portion;
 *                          the low-order WORD contains the fractional portion.
 *                          A value of 64KB gives a multiplier of 1. There is
 *                          an implementation limit of 255 for this value,
 *                          which must not be negative.
 * @param [in] fxStart      Start angle that defines the starting point on the
 *                          curve. Start and sweep angles are measured
 *                          counterclockwise from the X-axis at the center of
 *                          the arc before the arc parameters are applied.
 *                          The angles are specified as doubleword values in
 *                          fixed-point format. A value of 6553 gives an angle
 *                          of 1°. Both angles must be positive. An angle
 *                          greater than 360° is also valid.
 * @param [in] fxSweep      Sweep angle that defines the extent of the curve
 *                          to be drawn. Same format as fxStart.
 * @param [in] pInstance    Pointer to instance data.
 * @param [in] lFunction    High-order WORD=flags; low-order
 *                          WORD=NGrePartialArc.
 *
 * @return Integer (cHits) indicating correlation/error status.
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit (returned by display
 *                   drivers when the correlation flag is ON, and a hit
 *                   is detected).
 * @retval GPI_ERROR Error.
 *
 * @par EDM2 Reference
 * GrePartialArc (https://www.edm2.com/index.php/GrePartialArc):
 * "GrePartialArc draws a straight line from the current position to the
 * starting point of a partial arc and draws the specified partial arc."
 *
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_BASE_ERROR
 * - PMERR_BITMAP_NOT_SELECTED
 * - PMERR_COORDINATE_OVERFLOW
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_HDC_BUSY
 * - PMERR_INV_ANGLE_PARM
 * - PMERR_INV_COLOR_DATA
 * - PMERR_INV_COLOR_INDEX
 * - PMERR_INV_COORD_SPACE
 * - PMERR_INV_HDC
 * - PMERR_INV_IN_AREA
 * - PMERR_INV_IN_PATH
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_INV_MULTIPLIER
 * - PMERR_INV_NESTED_FIGURES
 * - PMERR_INV_PICK_APERTURE_POSN
 * - PMERR_INV_RECT
 * - PMERR_PATH_LIMIT_EXCEEDED
 * - PMERR_PATH_UNKNOWN
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 * @par Remarks
 * "Upon completion, the current (X,Y) position is the end of the partial
 * arc. The dimensions of the full arc are defined as a multiplier, which
 * is applied to the current arc parameters. The partial arc that is drawn
 * is the section of the full arc that is enclosed by the specified start
 * and sweep angles."
 * "If GrePartialArc is used within a path or an area definition to
 * continue a figure following a GreBoxxxx or GreFullArcxxx function, the
 * error PMERR_INV_NESTED_FIGURES is posted. This error occurs because
 * GreBoxxxx and GreFullArcxxx generate a closed figure within an area or
 * path definition."
 *
 */
static LONG __cdecl PartialArc(HDC hdc, PPOINTL pptlCenter,
                                       FIXED fxMultiplier, FIXED fxStart,
                                       FIXED fxSweep, PVOID pInstance,
                                       ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return GPI_ERROR;
    return (ULONG)pDc->GetArc().PartialArc(pptlCenter,
                                            fxMultiplier,
                                            fxStart, fxSweep);
}

/*!
 * @brief Dispatch for GreFullArcInterior — draw a filled full arc.
 *
 * @param [in] hdc           Device context handle.
 * @param [in] fxMultiplier  Multiplier defining the size of the required arc
 *                           in relation to an arc drawn with the current arc
 *                           parameters. The multiplier is a fixed-point value.
 *                           The high-order WORD contains the integer portion;
 *                           the low-order WORD contains the fractional portion.
 *                           A value of 64KB gives a multiplier of 1. The
 *                           implementation limit of the multiplier is 255.
 *                           This value must be positive or 0.
 * @param [in] pInstance     Pointer to instance data.
 * @param [in] lFunction     High-order WORD=flags; low-order
 *                           WORD=NGreFullArcInterior.
 *
 * @return Integer (cHits) indicating correlation/error status.
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit (returned by display
 *                   drivers when the correlation flag is ON, and a hit
 *                   is detected).
 * @retval GPI_ERROR Error.
 *
 * @par EDM2 Reference
 * GreFullArcInterior (https://www.edm2.com/index.php/GreFullArcInterior):
 * "GreFullArcInterior draws a filled, full arc by using the current pattern
 * attributes with its center at the current (X,Y) position. The dimensions
 * of the arc are defined by a multiplier that is applied to the current arc
 * parameters. GreFullArcxxx functions do not affect the current position.
 * The arc boundary is not drawn."
 * "When the COM_PATH or the COM_AREA flag is set, this function must raise
 * an error."
 * "When correlating, the handling routine must return a hit when the pick
 * aperture intersects the interior, or is completely within the interior,
 * even when the mix is LEAVEALONE."
 *
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_ALREADY_IN_AREA
 * - PMERR_BASE_ERROR
 * - PMERR_BITMAP_NOT_SELECTED
 * - PMERR_COORDINATE_OVERFLOW
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_EXCEEDS_MAX_SEG_LENGTH
 * - PMERR_HDC_BUSY
 * - PMERR_HRGN_BUSY
 * - PMERR_HUGE_FONTS_NOT_SUPPORTED
 * - PMERR_INSUFFICIENT_MEMORY
 * - PMERR_INV_AREA_CONTROL
 * - PMERR_INV_BACKGROUND_COL_ATTR
 * - PMERR_INV_BACKGROUND_MIX_ATTR
 * - PMERR_INV_CHAR_DIRECTION_ATTR
 * - PMERR_INV_CHAR_MODE_ATTR
 * - PMERR_INV_CODEPAGE
 * - PMERR_INV_COLOR_ATTR
 * - PMERR_INV_COLOR_DATA
 * - PMERR_INV_COLOR_INDEX
 * - PMERR_INV_COORD_SPACE
 * - PMERR_INV_COORDINATE
 * - PMERR_INV_HDC
 * - PMERR_INV_HRGN
 * - PMERR_INV_IN_AREA
 * - PMERR_INV_IN_PATH
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_INV_LINE_TYPE_ATTR
 * - PMERR_INV_MIX_ATTR
 * - PMERR_INV_MULTIPLIER
 * - PMERR_INV_NESTED_FIGURES
 * - PMERR_INV_PATTERN_REF_PT_ATTR
 * - PMERR_INV_PATTERN_SET_ATTR
 * - PMERR_INV_PATTERN_SET_FONT
 * - PMERR_INV_PICK_APERTURE_POSN
 * - PMERR_INV_RECT
 * - PMERR_INV_REGION_CONTROL
 * - PMERR_NOT_IN_AREA
 * - PMERR_NOT_IN_PATH
 * - PMERR_PATH_LIMIT_EXCEEDED
 * - PMERR_PATH_UNKNOWN
 * - PMERR_REGION_IS_CLIP_REGION
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
static LONG __cdecl FullArcInterior(HDC hdc, FIXED fxAngle,
                                            PVOID pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return GPI_ERROR;
    return pDc->GetArc().FullArcInterior(fxAngle);
}


/*!
 * @brief Dispatch for GreFullArcInterior — draw a filled full arc.
 *
 * @param [in] hdc           Device context handle.
 * @param [in] fxMultiplier  Multiplier defining the size of the required arc
 *                           in relation to an arc drawn with the current arc
 *                           parameters. The multiplier is a fixed-point value.
 *                           The high-order WORD contains the integer portion;
 *                           the low-order WORD contains the fractional portion.
 *                           A value of 64KB gives a multiplier of 1. The
 *                           implementation limit of the multiplier is 255.
 *                           This value must be positive or 0.
 * @param [in] pInstance     Pointer to instance data.
 * @param [in] lFunction     High-order WORD=flags; low-order
 *                           WORD=NGreFullArcInterior.
 *
 * @return Integer (cHits) indicating correlation/error status.
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit (returned by display
 *                   drivers when the correlation flag is ON, and a hit
 *                   is detected).
 * @retval GPI_ERROR Error.
 *
 * @par EDM2 Reference
 * GreFullArcInterior (https://www.edm2.com/index.php/GreFullArcInterior):
 * "GreFullArcInterior draws a filled, full arc by using the current pattern
 * attributes with its center at the current (X,Y) position. The dimensions
 * of the arc are defined by a multiplier that is applied to the current arc
 * parameters. GreFullArcxxx functions do not affect the current position.
 * The arc boundary is not drawn."
 * "When the COM_PATH or the COM_AREA flag is set, this function must raise
 * an error."
 * "When correlating, the handling routine must return a hit when the pick
 * aperture intersects the interior, or is completely within the interior,
 * even when the mix is LEAVEALONE."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_ALREADY_IN_AREA
 * - PMERR_BASE_ERROR
 * - PMERR_BITMAP_NOT_SELECTED
 * - PMERR_COORDINATE_OVERFLOW
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_EXCEEDS_MAX_SEG_LENGTH
 * - PMERR_HDC_BUSY
 * - PMERR_HRGN_BUSY
 * - PMERR_HUGE_FONTS_NOT_SUPPORTED
 * - PMERR_INSUFFICIENT_MEMORY
 * - PMERR_INV_AREA_CONTROL
 * - PMERR_INV_BACKGROUND_COL_ATTR
 * - PMERR_INV_BACKGROUND_MIX_ATTR
 * - PMERR_INV_CHAR_DIRECTION_ATTR
 * - PMERR_INV_CHAR_MODE_ATTR
 * - PMERR_INV_CODEPAGE
 * - PMERR_INV_COLOR_ATTR
 * - PMERR_INV_COLOR_DATA
 * - PMERR_INV_COLOR_INDEX
 * - PMERR_INV_COORD_SPACE
 * - PMERR_INV_COORDINATE
 * - PMERR_INV_HDC
 * - PMERR_INV_HRGN
 * - PMERR_INV_IN_AREA
 * - PMERR_INV_IN_PATH
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_INV_LINE_TYPE_ATTR
 * - PMERR_INV_MIX_ATTR
 * - PMERR_INV_MULTIPLIER
 * - PMERR_INV_NESTED_FIGURES
 * - PMERR_INV_PATTERN_REF_PT_ATTR
 * - PMERR_INV_PATTERN_SET_ATTR
 * - PMERR_INV_PATTERN_SET_FONT
 * - PMERR_INV_PICK_APERTURE_POSN
 * - PMERR_INV_RECT
 * - PMERR_INV_REGION_CONTROL
 * - PMERR_NOT_IN_AREA
 * - PMERR_NOT_IN_PATH
 * - PMERR_PATH_LIMIT_EXCEEDED
 * - PMERR_PATH_UNKNOWN
 * - PMERR_REGION_IS_CLIP_REGION
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
static LONG __cdecl FullArcBoundary(HDC hdc, FIXED fxAngle,
                                            PVOID pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return GPI_ERROR;
    return pDc->GetArc().FullArcBoundary(fxAngle);
}

/*!
 * @brief Dispatch for GreFullArcBoth — draw and fill a full arc.
 *
 * @param [in] hdc           Device context handle.
 * @param [in] fxMultiplier  Multiplier defining the size of the required arc
 *                           in relation to an arc drawn with the current arc
 *                           parameters. The multiplier is a fixed-point value.
 *                           The high-order WORD contains the integer portion;
 *                           the low-order WORD contains the fractional portion.
 *                           A value of 64KB gives a multiplier of 1. The
 *                           implementation limit of the multiplier is 255.
 *                           This value must not be negative.
 * @param [in] pInstance     Pointer to instance data.
 * @param [in] lFunction     High-order WORD=flags; low-order
 *                           WORD=NGreFullArcBoth.
 *
 * @return Integer (cHits) indicating correlation/error status.
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit (returned by display
 *                   drivers when the correlation flag is ON, and a hit
 *                   is detected).
 * @retval GPI_ERROR Error.
 *
 * @par EDM2 Reference
 * GreFullArcBoth (https://www.edm2.com/index.php/GreFullArcBoth):
 * "GreFullArcBoth draws and fills a full arc centered on the current
 * (X,Y) position. The current line attributes apply to the boundary line,
 * and the current pattern attributes apply to the interior. The dimensions
 * of the arc are defined as a multiplier that is applied to the current arc
 * parameters. GreFullArcBoth does not affect the current position."
 * "When the COM_PATH or the COM_AREA flag is set, this function must raise
 * an error."
 * "When correlating, the handling routine must return a hit when the pick
 * aperture intersects the boundary, or is completely within the interior,
 * even when the mix is LEAVEALONE."
 *
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_ALREADY_IN_AREA
 * - PMERR_BASE_ERROR
 * - PMERR_BITMAP_NOT_SELECTED
 * - PMERR_COORDINATE_OVERFLOW
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_EXCEEDS_MAX_SEG_LENGTH
 * - PMERR_HDC_BUSY
 * - PMERR_HRGN_BUSY
 * - PMERR_HUGE_FONTS_NOT_SUPPORTED
 * - PMERR_INSUFFICIENT_MEMORY
 * - PMERR_INV_AREA_CONTROL
 * - PMERR_INV_BACKGROUND_COL_ATTR
 * - PMERR_INV_BACKGROUND_MIX_ATTR
 * - PMERR_INV_CHAR_DIRECTION_ATTR
 * - PMERR_INV_CHAR_MODE_ATTR
 * - PMERR_INV_CODEPAGE
 * - PMERR_INV_COLOR_ATTR
 * - PMERR_INV_COLOR_DATA
 * - PMERR_INV_COLOR_INDEX
 * - PMERR_INV_COORD_SPACE
 * - PMERR_INV_COORDINATE
 * - PMERR_INV_HDC
 * - PMERR_INV_HRGN
 * - PMERR_INV_IN_AREA
 * - PMERR_INV_IN_PATH
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_INV_LINE_TYPE_ATTR
 * - PMERR_INV_MIX_ATTR
 * - PMERR_INV_MULTIPLIER
 * - PMERR_INV_NESTED_FIGURES
 * - PMERR_INV_PATTERN_REF_PT_ATTR
 * - PMERR_INV_PATTERN_SET_ATTR
 * - PMERR_INV_PATTERN_SET_FONT
 * - PMERR_INV_PICK_APERTURE_POSN
 * - PMERR_INV_RECT
 * - PMERR_INV_REGION_CONTROL
 * - PMERR_NOT_IN_AREA
 * - PMERR_NOT_IN_PATH
 * - PMERR_PATH_LIMIT_EXCEEDED
 * - PMERR_PATH_UNKNOWN
 * - PMERR_REGION_IS_CLIP_REGION
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
static LONG __cdecl FullArcBoth(HDC hdc, FIXED fxAngle,
                                        PVOID pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return GPI_ERROR;
    return pDc->GetArc().FullArcBoth(fxAngle);
}


/*!
 * @brief Dispatch for GreBoxInterior — draw a filled rectangular box.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] pBox       Pointer to a BOXDATA structure giving the opposite
 *                        corner and the X/Y diameters for corner rounding.
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order
 *                        WORD=NGreBoxInterior.
 *
 * @return Integer (cHits) indicating correlation/error status.
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit (returned by display
 *                   drivers when the correlation flag is ON, and a hit
 *                   is detected).
 * @retval GPI_ERROR Error.
 *
 * @par EDM2 Reference
 * GreBoxInterior (https://www.edm2.com/index.php/GreBoxInterior):
 * "GreBoxInterior draws a rectangular box with one corner at the current
 * (X,Y) position and the opposite corner at the specified (X,Y) position.
 * The current (X,Y) position does not change. When this function occurs
 * within an area or path definition, it generates a closed figure.
 * GreBoxInterior must not occur within any other figure definition."
 *
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * @todo: NO INFO!
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 * @par Remarks
 * "The sides of the box (before transformation) are parallel to the X-axis
 * and Y-axis. The corners of the box can be rounded by means of quarter
 * ellipses of the specified diameters. When the value of either diameter
 * is 0, no rounding occurs. When the value of either diameter exceeds the
 * length of the corresponding side, that length is used as the diameter
 * instead. When the value of the diameters are equal to the value of the
 * sides, the corners are rounded with a quarter circle."
 * "If the current position is (x0, y0), the box is drawn from the current
 * position in a counterclockwise direction. This is significant when, for
 * example, the area mode is BA_WINDING."
 * "When correlating, the handling routine records a hit when the pick
 * aperture intersects, or is completely within, the interior (even if the
 * mix used for the fill operation is LEAVEALONE)."
 *
 * @note The EDM2 page declares the pBox parameter as PPOINTL while
 *       describing it as a BOXDATA structure; pmddim.h declares it as
 *       PBOXPARAMS. All three types share the same layout (opposite
 *       corner POINTL + two LONG rounding diameters), so gre.cpp casts
 *       to const BOXDATA * directly. The error list above is taken from
 *       pmddim.h because the EDM2 page for GreBoxInterior omits the
 *       Possible Errors Detected section.
 */
static LONG __cdecl BoxInterior(HDC hdc, PPOINTL pBox,
                                       PVOID pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return GPI_ERROR;
    return pDc->GetArc().BoxInterior(pDc, (const BOXDATA *)pBox);
}


/*!
 * @brief Dispatch for GreBoxBoundary — draw a rectangular box boundary.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] pBox       Pointer to a BOXDATA structure giving the opposite
 *                        corner and the X/Y diameters for corner rounding.
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order
 *                        WORD=NGreBoxBoundary.
 *
 * @return Integer (cHits) indicating correlation/error status.
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit (returned by display
 *                   drivers when the correlation flag is ON, and a hit
 *                   is detected).
 * @retval GPI_ERROR Error.
 *
 * @par EDM2 Reference
 * GreBoxBoundary (https://www.edm2.com/index.php/GreBoxBoundary):
 * "GreBoxBoundary draws a rectangular box with one corner at the current
 * (X,Y) position and the opposite corner at the specified (X,Y) position.
 * The current (X,Y) position does not change."
 *
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_BASE_ERROR
 * - PMERR_BITMAP_NOT_SELECTED
 * - PMERR_COORDINATE_OVERFLOW
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_HDC_BUSY
 * - PMERR_INV_BOX_ROUNDING_PARM
 * - PMERR_INV_COLOR_DATA
 * - PMERR_INV_COLOR_INDEX
 * - PMERR_INV_COORD_SPACE
 * - PMERR_INV_HDC
 * - PMERR_INV_IN_AREA
 * - PMERR_INV_IN_PATH
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_INV_NESTED_FIGURES
 * - PMERR_INV_PICK_APERTURE_POSN
 * - PMERR_INV_RECT
 * - PMERR_NOT_IN_PATH
 * - PMERR_PATH_LIMIT_EXCEEDED
 * - PMERR_PATH_UNKNOWN
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 * @par Remarks
 * "The sides of the box (before transformation) are parallel to the X-axis
 * and the Y-axis. The corners of the box can be rounded by means of quarter
 * ellipses of the specified diameters. When the value of either diameter
 * is 0, no rounding occurs. When the value of either diameter exceeds the
 * length of the corresponding side, that length is used as the diameter
 * instead. When the value of the diameters are equal to the value of the
 * sides, the corners are rounded with a quarter circle."
 * "If the current position is (x0, y0), the box is drawn from the current
 * position in a counterclockwise direction."
 * "When correlating, the handling routine records a hit when the pick
 * aperture intersects the boundary."
 *
 * @note The EDM2 page declares the pBox parameter as PPOINTL while
 *       describing it as a BOXDATA structure; pmddim.h declares it as
 *       PBOXPARAMS. All three types share the same layout (opposite
 *       corner POINTL + two LONG rounding diameters), so gre.cpp casts
 *       to const BOXDATA * directly.
 */
static LONG __cdecl BoxBoundary(HDC hdc, PPOINTL pBox,
                                       PVOID pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return GPI_ERROR;
    return pDc->GetArc().BoxBoundary(pDc, (const BOXDATA *)pBox);
}

/*!
 * @brief Dispatch for GreBoxBoth — draw and fill a rectangular box.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] pBox       Pointer to a BOXPARAMS structure giving the opposite
 *                        corner and the X/Y diameters for corner rounding.
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order
 *                        WORD=NGreBoxBoth.
 *
 * @return Integer (cHits) indicating correlation/error status.
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit (returned by display
 *                   drivers when the correlation flag is ON, and a hit
 *                   is detected).
 * @retval GPI_ERROR Error.
 *
 * @par EDM2 Reference
 * GreBoxBoth (https://www.edm2.com/index.php/GreBoxBoth):
 * "GreBoxBoth draws and fills a rectangular box with one corner at the
 * current (X,Y) position and the opposite corner at the specified (X,Y)
 * position. The current (X,Y) position does not change. When this function
 * occurs within an area or path definition, it generates a closed figure.
 * GreBoxBoth must not occur within any other figure definition."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_ALREADY_IN_AREA
 * - PMERR_BASE_ERROR
 * - PMERR_BITMAP_NOT_SELECTED
 * - PMERR_COORDINATE_OVERFLOW
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_EXCEEDS_MAX_SEG_LENGTH
 * - PMERR_HDC_BUSY
 * - PMERR_HRGN_BUSY
 * - PMERR_HUGE_FONTS_NOT_SUPPORTED
 * - PMERR_INSUFFICIENT_MEMORY
 * - PMERR_INV_AREA_CONTROL
 * - PMERR_INV_BACKGROUND_COL_ATTR
 * - PMERR_INV_BACKGROUND_MIX_ATTR
 * - PMERR_INV_BOX_ROUNDING_PARM
 * - PMERR_INV_CHAR_DIRECTION_ATTR
 * - PMERR_INV_CHAR_MODE_ATTR
 * - PMERR_INV_CODEPAGE
 * - PMERR_INV_COLOR_ATTR
 * - PMERR_INV_COLOR_DATA
 * - PMERR_INV_COLOR_INDEX
 * - PMERR_INV_COORD_SPACE
 * - PMERR_INV_COORDINATE
 * - PMERR_INV_HDC
 * - PMERR_INV_HRGN
 * - PMERR_INV_IN_AREA
 * - PMERR_INV_IN_PATH
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_INV_LINE_TYPE_ATTR
 * - PMERR_INV_MIX_ATTR
 * - PMERR_INV_NESTED_FIGURES
 * - PMERR_INV_PATTERN_REF_PT_ATTR
 * - PMERR_INV_PATTERN_SET_ATTR
 * - PMERR_INV_PATTERN_SET_FONT
 * - PMERR_INV_PICK_APERTURE_POSN
 * - PMERR_INV_RECT
 * - PMERR_INV_REGION_CONTROL
 * - PMERR_NOT_IN_AREA
 * - PMERR_NOT_IN_PATH
 * - PMERR_PATH_LIMIT_EXCEEDED
 * - PMERR_PATH_UNKNOWN
 * - PMERR_REGION_IS_CLIP_REGION
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 * @par Remarks
 * "The sides of the box (before transformation) are parallel to the X-axis
 * and the Y-axis. The corners of the box can be rounded by means of quarter
 * ellipses of the specified diameters. When the value of either diameter
 * is 0, no rounding occurs. When the value of either diameter exceeds the
 * length of the corresponding side, that length is used as the diameter
 * instead. When the value of the diameters are equal to the value of the
 * sides, the corners are rounded with a quarter circle."
 * "If the current position is (x0, y0), the box is drawn from the current
 * position in a counterclockwise direction."
 * "When correlating, the handling routine records a hit when the pick
 * aperture intersects the boundary or interior, or is completely within
 * the interior (even if the mix used for the fill operation is LEAVEALONE)."
 *
 * @note Unlike GreBoxInterior and GreBoxBoundary (where EDM2 incorrectly
 *       declares pBox as PPOINTL while describing BOXDATA), GreBoxBoth
 *       correctly declares pBox as PBOXPARAMS in both EDM2 and pmddim.h.
 */
static LONG __cdecl BoxBoth(HDC hdc, PBOXPARAMS pBox,
                                   PVOID pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return GPI_ERROR;
    return pDc->GetArc().BoxBoth(pDc, (const BOXPARAMS *)pBox);
}


/**
 * @brief 
 *
 * @param[in] 
 * @param[in] 
 * @param[in] 
 *
 * @return 
 *
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit.
 * @retval GPI_ERROR Error.
 *
 * @note No info!!
 */


/*!
 * @brief Dispatch for GrePolyFillet — draw a fillet on connected lines.
 *
 * @param [in] hdc         Device context handle.
 * @param [in] paptlPoint  Pointer to an array of cPoints (X,Y) pairs
 *                         containing the coordinates of the end points
 *                         for the lines. If COM_TRANSFORM is not set,
 *                         the function expects the points to be in screen
 *                         coordinates.
 * @param [in] cPoints     Number of coordinate pairs. If this value is
 *                         passed as 0, the handling routine does nothing
 *                         and returns Successful.
 * @param [in] pInstance   Pointer to instance data.
 * @param [in] lFunction   High-order WORD=flags; low-order
 *                         WORD=NGrePolyFillet.
 *
 * @return Integer (cHits) indicating correlation/error status.
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit (returned by display
 *                   drivers when the correlation flag is ON, and a hit
 *                   is detected).
 * @retval GPI_ERROR Error.
 *
 * @par EDM2 Reference
 * GrePolyFillet (https://www.edm2.com/index.php/GrePolyFillet):
 * "GrePolyFillet draws a fillet on a series of connected lines with the
 * first line starting at the current (X,Y) position. Upon completion, the
 * current (X,Y) position is set to the last point in the series."
 *
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_BASE_ERROR
 * - PMERR_BITMAP_NOT_SELECTED
 * - PMERR_COORDINATE_OVERFLOW
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_HDC_BUSY
 * - PMERR_INV_COLOR_DATA
 * - PMERR_INV_COLOR_INDEX
 * - PMERR_INV_COORD_SPACE
 * - PMERR_INV_HDC
 * - PMERR_INV_IN_AREA
 * - PMERR_INV_IN_PATH
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_INV_PICK_APERTURE_POSN
 * - PMERR_INV_RECT
 * - PMERR_PATH_LIMIT_EXCEEDED
 * - PMERR_PATH_UNKNOWN
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 * @par Remarks
 * "The shape of the fillet is controlled by a set of coordinates for a
 * series of two or more connected lines. The fillet is tangential to the
 * start point of the first line and to the end point of the last line.
 * When more than two sets of coordinates are supplied, the fillet passes
 * through the mid-points of the intermediate lines. An individual fillet
 * always lies within the area bounded by the start, end, and control points."
 *
 */
static LONG __cdecl PolyFillet(HDC hdc, PPOINTL paptlPoint,
                                       LONG cPoints, PVOID pInstance,
                                       ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return GPI_ERROR;
    return pDc->GetArc().PolyFillet((const POINTL *)paptlPoint,
                                            cPoints);
}

/*!
 * @brief Dispatch for GrePolyFilletSharp — draw a sequence of sharp fillets.
 *
 * @param [in] hdc         Device context handle.
 * @param [in] paptlPoint  Pointer to an array of cPoints (X,Y) pairs where
 *                         each group contains two pairs of (X,Y) coordinates
 *                         for the control and end points of the fillets.
 *                         If COM_TRANSFORM is not set, the function expects
 *                         the points to be in screen coordinates.
 * @param [in] cPoints     Number of coordinate pairs. When this is passed
 *                         as 0, it returns Successful.
 * @param [in] pfxSharp    Pointer to an array of sharpness values with one
 *                         element for each fillet. Sharpness is a fixed-point
 *                         value. The high-order WORD contains the integer
 *                         portion; the low-order WORD contains the fractional
 *                         portion. A value of 64KB gives a sharpness of 1.
 * @param [in] pInstance   Pointer to instance data.
 * @param [in] lFunction   High-order WORD=flags; low-order
 *                         WORD=NGrePolyFilletSharp.
 *
 * @return Integer (cHits) indicating correlation/error status.
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit (returned by display
 *                   drivers when the correlation flag is ON, and a hit
 *                   is detected).
 * @retval GPI_ERROR Error.
 *
 * @par EDM2 Reference
 * GrePolyFilletSharp (https://www.edm2.com/index.php/GrePolyFilletSharp):
 * "GrePolyFilletSharp draws a sequence of one or more sharp fillets
 * starting at the current (X,Y) position."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_BASE_ERROR
 * - PMERR_BITMAP_NOT_SELECTED
 * - PMERR_COORDINATE_OVERFLOW
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_HDC_BUSY
 * - PMERR_INV_COLOR_DATA
 * - PMERR_INV_COLOR_INDEX
 * - PMERR_INV_COORD_SPACE
 * - PMERR_INV_HDC
 * - PMERR_INV_IN_AREA
 * - PMERR_INV_IN_PATH
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_INV_PICK_APERTURE_POSN
 * - PMERR_INV_RECT
 * - PMERR_PATH_LIMIT_EXCEEDED
 * - PMERR_PATH_UNKNOWN
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 * @par Remarks
 * "As each fillet is drawn, the end point for the fillet becomes the start
 * point for the next fillet. Upon completion, the current (X,Y) position
 * is the end point of the last fillet. Each fillet is controlled by a set
 * of coordinates for two connected lines and by a sharpness value. The
 * fillet is tangential to the start point of the first line and to the
 * end point of the second line."
 * "The sharpness value is determined from: Sharpness = WD/DB, where W is
 * the mid-point of the line joining the end points, B is the control point
 * above the top of the fillet, D is the point where the fillet intersects
 * the line WC."
 *
 */
static LONG __cdecl PolyFilletSharp(HDC hdc, PPOINTL paptlPoint,
                                            LONG cPoints, PFIXED pfxSharp,
                                            PVOID pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return GPI_ERROR;
    return pDc->GetArc().PolyFilletSharp((const POINTL *)paptlPoint,
                                                 cPoints,
                                                 (const FIXED *)pfxSharp);
}

/*!
 * @brief Dispatch for GrePolySpline — draw a sequence of Bezier splines.
 *
 * @param [in] hdc         Device context handle.
 * @param [in] paptlPoint  Pointer to an array of elements in which each group
 *                         contains three pairs of (X,Y) coordinates for the
 *                         control and end points for the splines. If
 *                         COM_TRANSFORM is not set, the function expects the
 *                         points to be in screen coordinates.
 * @param [in] cPoints     Number of coordinate pairs in the array. When this
 *                         is passed as 0, it returns Successful.
 * @param [in] pInstance   Pointer to instance data.
 * @param [in] lFunction   High-order WORD=flags; low-order
 *                         WORD=NGrePolySpline.
 *
 * @return Integer (cHits) indicating correlation/error status.
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit (returned by display
 *                   drivers when the correlation flag is ON, and a hit
 *                   is detected).
 * @retval GPI_ERROR Error.
 *
 * @par EDM2 Reference
 * GrePolySpline (https://www.edm2.com/index.php/GrePolySpline):
 * "GrePolySpline draws a sequence of one or more Bezier splines starting
 * at the current (X,Y) position. As each spline is drawn, the specified
 * end point for the spline becomes the start point for the next spline.
 * Upon completion, the current (X,Y) position is the end point of the
 * last spline."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_BASE_ERROR
 * - PMERR_BITMAP_NOT_SELECTED
 * - PMERR_COORDINATE_OVERFLOW
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_HDC_BUSY
 * - PMERR_INV_COLOR_DATA
 * - PMERR_INV_COLOR_INDEX
 * - PMERR_INV_COORD_SPACE
 * - PMERR_INV_HDC
 * - PMERR_INV_IN_AREA
 * - PMERR_INV_IN_PATH
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_INV_PICK_APERTURE_POSN
 * - PMERR_INV_RECT
 * - PMERR_PATH_LIMIT_EXCEEDED
 * - PMERR_PATH_UNKNOWN
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 * @par Remarks
 * "The shape of each spline is controlled by a set of coordinates for
 * three connected lines. The spline starts at the current position and
 * ends at the end point of the third line. The end points of the first
 * and second lines are used as control points. An individual spline
 * always lies within the area bounded by the start, end, and control
 * points."
 *
 */
static LONG __cdecl PolySpline(HDC hdc, PPOINTL paptlPoint,
                                       LONG cPoints, PVOID pInstance,
                                       ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return GPI_ERROR;
    return pDc->GetLine().PolySpline((const POINTL *)paptlPoint,
                                             cPoints);
}

/**
 * @brief
 *
 * @param[in] a
 * @param[in] b
 * @param[in] c
 *
 * @return 
 *
 * @retval 
 *
 * @note Not found any information yet and not found any driver used it?
 */
GRE_DRV_7 (Oval)

/**
 * @brief
 *
 * @param[in] a
 * @param[in] b
 * @param[in] c
 *
 * @return 
 *
 * @retval 
 *
 * @note Not found any information yet and not found any driver used it?
 */
GRE_DRV_4 (CookWholePath)

/**
 * @brief
 *
 * @param[in] a
 * @param[in] b
 * @param[in] c
 *
 * @return 
 *
 * @retval 
 *
 * @note Not found any information yet and not found any driver used it?
 */
GRE_DRV_5 (CookPathCurves)

/**
 * @brief
 *
 * @param[in] a
 * @param[in] b
 * @param[in] c
 *
 * @return 
 *
 * @retval 
 *
 * @note Not found any information yet and not found any driver used it?
 */
GRE_DRV_10(RenderPath)

/**
 * @brief
 *
 * @param[in] a
 * @param[in] b
 * @param[in] c
 *
 * @return 
 *
 * @retval 
 *
 * @note Not found any information yet and not found any driver used it?
 */
GRE_DRV_5 (BoxExclusive)
