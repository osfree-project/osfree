#include "Fs_line.hpp"
#include "Fs_path.hpp"
#include <string.h>

/*!
 * @brief Construct with no owning path.
 * @return Nothing (constructor).
 * @retval (void)  No return value.
 */
Fs_Line::Fs_Line() : m_pPath(NULL), m_hdc(0) {}

/*!
 * @brief Destroy.
 * @return Nothing (destructor).
 * @retval (void)  No return value.
 */
Fs_Line::~Fs_Line() { Shutdown(); }

/*!
 * @brief Reset state.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Line::Init() {}

/*!
 * @brief Alias for Init.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Line::Shutdown() { Init(); }

/*!
 * @brief Emit a line segment into m_pPath.
 * @param [in] a  Start point.
 * @param [in] b  End point.
 * @return GPI_OK.
 * @retval GPI_OK  Always.
 */
LONG Fs_Line::EmitLine(const POINTL &a, const POINTL &b)
{
    if (m_pPath) m_pPath->AddLine(a, b);
    return GPI_OK;
}

/*!
 * @brief Tessellate a cubic Bezier into line segments.
 * @param [in] p0   Start point.
 * @param [in] p1   First control point.
 * @param [in] p2   Second control point.
 * @param [in] p3   End point.
 * @param [in] segments  Number of line segments.
 * @return GPI_OK.
 * @retval GPI_OK  Always.
 * @note De Casteljau evaluation of the cubic at parameter t = i/segments.
 *       The final point (t = 1) is exactly p3.
 */
LONG Fs_Line::Bezier(const POINTL &p0, const POINTL &p1,
                     const POINTL &p2, const POINTL &p3, int segments)
{
    POINTL prev = p0;
    for (int i = 1; i <= segments; i++) {
        double t = (double)i / segments;
        double u = 1.0 - t;
        POINTL cur;
        cur.x = (LONG)(u*u*u*p0.x + 3*u*u*t*p1.x + 3*u*t*t*p2.x + t*t*t*p3.x);
        cur.y = (LONG)(u*u*u*p0.y + 3*u*u*t*p1.y + 3*u*t*t*p2.y + t*t*t*p3.y);
        EmitLine(prev, cur);
        prev = cur;
    }
    return GPI_OK;
}

/*!
 * @brief Draw an RLE shape (format iType = 0).
 * @param [in] pRLEHDR  Pointer to RLEHDR.
 * @return cHits code.
 * @retval GPI_OK    Accepted.
 * @retval GPI_HITS  Correlation hit.
 * @retval GPI_ERROR pRLEHDR is NULL.
 *
 * @par EDM2 Reference
 * GreDrawRLE — see header.
 *
 * @warning RLE decoding into SCANDATA is not implemented.  The call is
 *          accepted and returns GPI_OK.  Real decoding is a gap.
 */
LONG Fs_Line::DrawRLE(const RLEHDR *pRLEHDR)
{
    if (!pRLEHDR) return GPI_ERROR;
    /* Format iType = 0: pRLE points to an array of POINTL.  The array
     * starts with (X, Y) where Y is the Y-coordinate and X is the number
     * of runs on the line, followed by (X_start, X_end) pairs per run.
     * Decoding into SCANDATA and calling the mandatory GrePolyScanline is
     * the documented engine path — TODO when SOFTDRAW integration lands. */
    return GPI_OK;
}

/*!
 * @brief Draw a set of closed polygons.
 * @param [in] flModel    POLYGON_INCL / POLYGON_EXCL.
 * @param [in] flOptions  POLYGON_ALTERNATE / POLYGON_WINDING,
 *                        POLYGON_BOUNDARY / POLYGON_NOBOUNDARY.
 * @param [in] paPolygon  Array of POLYGON structures.
 * @param [in] cPolygons  Number of polygons.
 * @return cHits code.
 * @retval GPI_OK    Drawn.
 * @retval GPI_HITS  Correlation hit.
 * @retval GPI_ERROR NULL array with cPolygons != 0.
 *
 * @par EDM2 Reference
 * GrePolygonSet — see header.
 */
LONG Fs_Line::PolygonSet(LONG flModel, ULONG flOptions,
                         const POLYGON *paPolygon, LONG cPolygons)
{
    (void)flModel; (void)flOptions;

    /* Per EDM2: cPolygons == 0 — no action, return Successful. */
    if (cPolygons == 0) return GPI_OK;
    if (!paPolygon) return GPI_ERROR;

    /* Per EDM2: GrePolygonSet is not valid when COM_AREA or COM_PATH is set.
     * That check is the caller's responsibility (the area/path state);
     * the engine's outer dispatcher can pass COM_* flags here if needed. */

    for (LONG i = 0; i < cPolygons; i++) {
        const POLYGON &poly = paPolygon[i];
        if (poly.ulPoints < 2) continue;

        /* Decompose polygon into line segments.  Per EDM2 the polygon is
         * automatically closed by drawing a line from the last vertex to
         * the first; the current position for the first polygon is the
         * first point, but for subsequent polygons all points are given
         * explicitly.  We emit explicit segments for every polygon. */
        for (ULONG j = 0; j + 1 < poly.ulPoints; j++)
            EmitLine(poly.aPointl[j], poly.aPointl[j + 1]);
        /* Auto-close if the last vertex differs from the first. */
        if (poly.ulPoints > 2)
            EmitLine(poly.aPointl[poly.ulPoints - 1], poly.aPointl[0]);
    }
    return GPI_OK;
}

/*!
 * @brief Draw one or more Bezier splines.
 * @param [in] paptlPoint  Array of coordinate pairs (3 pairs per spline).
 * @param [in] cPoints     Number of coordinate pairs.
 * @return cHits code.
 * @retval GPI_OK    Drawn.
 * @retval GPI_HITS  Correlation hit.
 * @retval GPI_ERROR NULL array with cPoints != 0.
 *
 * @par EDM2 Reference
 * GrePolySpline — see header.
 */
LONG Fs_Line::PolySpline(const POINTL *paptlPoint, LONG cPoints)
{
    /* Per EDM2: cPoints == 0 — return Successful. */
    if (cPoints == 0) return GPI_OK;
    if (!paptlPoint) return GPI_ERROR;

    /* Starting point is the current position (line path's last point) or,
     * when no path is bound, the first coordinate pair as a fallback. */
    POINTL cur;
    if (m_pPath && m_pPath->LineCount() > 0)
        cur = m_pPath->Lines()[m_pPath->LineCount() - 1].ptlC;
    else
        cur = paptlPoint[0];

    /* Each group of 3 coordinate pairs defines one cubic Bezier:
     *   p1 = paptlPoint[i]     first control point
     *   p2 = paptlPoint[i+1]   second control point
     *   p3 = paptlPoint[i+2]   end point
     * The spline starts at cur and ends at p3.  Per EDM2 the end point of
     * each spline becomes the start point of the next. */
    for (LONG i = 0; i + 2 < cPoints; i += 3) {
        Bezier(cur, paptlPoint[i], paptlPoint[i + 1], paptlPoint[i + 2], 32);
        cur = paptlPoint[i + 2];
    }
    return GPI_OK;
}

/*!
 * @brief Forward an area fill to the driver's GrePolyScanline.
 * @param [in] pScanData  Pointer to SCANDATA.
 * @return cHits code.
 * @retval GPI_OK    Accepted.
 * @retval GPI_HITS  Correlation hit.
 * @retval GPI_ERROR pScanData is NULL.
 *
 * @par EDM2 Reference
 * GrePolyScanline — see header.
 *
 * @par Simulation support
 * "None. This function is mandatory for all drivers."
 *
 * @note This method is a thin forwarder.  The engine's DrawRLE / EndArea /
 *       FillPath paths are the documented callers.  If the driver hooks
 *       those three functions, the driver owns the SCANDATA traversal.
 */
LONG Fs_Line::PolyScanline(const SCANDATA *pScanData)
{
    if (!pScanData) return GPI_ERROR;
    /* Mandatory in the driver; not simulated by the engine.  Reaching this
     * point means the engine's DrawRLE/EndArea/FillPath simulation path
     * would normally be calling the driver's GrePolyScanline directly.
     * We accept here as a defensive fallback. */
    return GPI_OK;
}


/**
 * @brief Draw a polyline.
 *
 * @param[in] hdc        Device context handle.
 * @param[in] paptlPoint Pointer to an array of POINTL structures that
 *                       define the vertices of the polyline.
 * @param[in] cPoints    Number of (X,Y) pairs in the point array.
 *
 * @return Integer (cHits) indicating correlation/error status.
 *
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit.
 * @retval GPI_ERROR Error.
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    No     |  Must  |   Can   |
 * @note If the function returns GPI_ERROR, the handling routine must call
 *       WinSetErrorInfo. WinGetLastError may return one of the following
 *       errors:
 *       - PMERR_BASE_ERROR
 *       - PMERR_BITMAP_NOT_SELECTED
 *       - PMERR_COORDINATE_OVERFLOW
 *       - PMERR_DEV_FUNC_NOT_INSTALLED
 *       - PMERR_HDC_BUSY
 *       - PMERR_INV_COLOR_DATA
 *       - PMERR_INV_COLOR_INDEX
 *       - PMERR_INV_COORD_SPACE
 *       - PMERR_INV_HDC
 *       - PMERR_INV_IN_AREA
 *       - PMERR_INV_IN_PATH
 *       - PMERR_INV_LENGTH_OR_COUNT
 *       - PMERR_INV_PICK_APERTURE_POSN
 *       - PMERR_INV_RECT
 *       - PMERR_PATH_LIMIT_EXCEEDED
 *       - PMERR_PATH_UNKNOWN
 *
 * @par Remarks
 * "When COM_TRANSFORM is not set, the function expects the array of
 * points to be in screen coordinates."
 * "When the COM_AREA or COM_PATH flag is set, this function is part of
 * an area or path definition. In either case, the handling routine would
 * usually pass the call back to the graphics engine for processing by
 * the default handling routine. The call would not be passed back to the
 * graphics engine if the presentation driver had hooked all of the area
 * and path functions."
 * "When this function is used to draw a closed figure, the handling
 * routine must not draw the last point of the last (closure) line. The
 * handling routine can call back to the graphics engine to do any
 * necessary clipping."
 *
 * @note  When the clip region is more complex, the presentation driver handling
 *        routine can forward the call to the graphics engine (this function)
 *        using the pointer supplied 
 *        in the dispatch table when the presentation driver was enabled.
 *        The engine (we) will clip each line and return it to the presentation
 *        driver as a call to GreDrawLinesInPath. 
 */
static LONG __cdecl PolyLine(HDC hdc, PPOINTL paptlPoint, LONG cPoints, PVOID pInstance, ULONG lFunction)
{
  //@todo implement described logic
}

