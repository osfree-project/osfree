#ifndef FS_LINE_HPP
#define FS_LINE_HPP

#define INCL_BASE
#define INCL_PM
#include "F_platform.h"

class Fs_Path;

/*!
 * @class Fs_Line
 * @brief Line / polygon / spline / RLE state for one DC.
 *
 * Bezier splines are tessellated into line segments and emitted into
 * Fs_Path.  Polygons are decomposed into line segments.  DrawRLE is the
 * documented RLE entry point; PolyScanline is a mandatory driver routine
 * that the engine calls (not simulates).
 *
 * @par EDM2 Reference
 * "Line functions (INCL_GRE_LINE)" category.
 * GreDrawRLE, GrePolygonSet, GrePolySpline (all simulated);
 * GrePolyScanline (mandatory, Simulation support: None).
 */
class Fs_Line {
public:
    /*!
     * @brief Construct with no owning path.
     * @return Nothing (constructor).
     * @retval (void)  No return value.
     */
    Fs_Line();

    /*!
     * @brief Destroy.
     * @return Nothing (destructor).
     * @retval (void)  No return value.
     */
    ~Fs_Line();

    /*!
     * @brief Reset state.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void Init();

    /*!
     * @brief Alias for Init.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void Shutdown();

    /*!
     * @brief Bind to the owning path (for line emission).
     * @param [in] pPath  Owning Fs_Path.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void SetPath(Fs_Path *pPath) { m_pPath = pPath; }

    /*!
     * @brief Bind to the owning DC (reserved for forwarders).
     * @param [in] hdc  Owning device context handle.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void SetHdc(ULONG hdc) { m_hdc = hdc; }

    /*!
     * @brief Draw an RLE shape (format iType = 0).
     * @param [in] pRLEHDR  Pointer to RLEHDR (lType, brectlBounds, pRLE).
     * @return cHits code.
     * @retval GPI_OK    Drawn.
     * @retval GPI_HITS  Correlation hit.
     * @retval GPI_ERROR pRLEHDR is NULL.
     *
     * @par EDM2 Reference
     * GreDrawRLE (https://www.edm2.com/index.php/GreDrawRLE):
     * "GreDrawRLE draws a run-length-encoded shape. If the call is passed
     * back to the graphics engine, it calls the GrePolyScanline entry point.
     * This function can be hooked by the presentation driver."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     *
     * @warning RLE decoding into SCANDATA is not implemented; the call is
     *          accepted and returns GPI_OK.  Real decoding is a documented
     *          gap and belongs to a future SOFTDRAW integration.
     */
    LONG DrawRLE(const RLEHDR *pRLEHDR);

    /*!
     * @brief Draw a set of closed polygons.
     * @param [in] flModel    POLYGON_INCL / POLYGON_EXCL.
     * @param [in] flOptions  POLYGON_ALTERNATE / POLYGON_WINDING,
     *                        POLYGON_BOUNDARY / POLYGON_NOBOUNDARY.
     * @param [in] paPolygon  Array of POLYGON structures.
     * @param [in] cPolygons  Number of polygons (0 returns GPI_OK).
     * @return cHits code.
     * @retval GPI_OK    Drawn.
     * @retval GPI_HITS  Correlation hit.
     * @retval GPI_ERROR NULL array with cPolygons != 0.
     *
     * @par EDM2 Reference
     * GrePolygonSet (https://www.edm2.com/index.php/GrePolygonSet):
     * "GrePolygonSet draws a set of closed polygons. This function can be
     * hooked by the presentation driver."
     * "The polygons are filled using the current AREABUNDLE structure values.
     * For the first polygon, the current position is the first point. For all
     * subsequent polygons, all points that define the polygon are given
     * explicitly. The polygons are automatically closed, if necessary, by
     * drawing a line from the last vertex to the first. Notice that polygons
     * can overlap, if needed."
     * "GrePolygonSet is not valid when the COM_AREA or COM_PATH flag is set."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG PolygonSet(LONG flModel, ULONG flOptions,
                    const POLYGON *paPolygon, LONG cPolygons);

    /*!
     * @brief Draw one or more Bezier splines.
     * @param [in] paptlPoint  Array of coordinate pairs (3 pairs per spline).
     * @param [in] cPoints     Number of coordinate pairs (0 returns GPI_OK).
     * @return cHits code.
     * @retval GPI_OK    Drawn.
     * @retval GPI_HITS  Correlation hit.
     * @retval GPI_ERROR NULL array with cPoints != 0.
     *
     * @par EDM2 Reference
     * GrePolySpline (http://www.edm2.com/index.php?title=GrePolySpline&oldid=69318):
     * "GrePolySpline draws a sequence of one or more Bezier splines starting
     * at the current (X,Y) position. As each spline is drawn, the specified
     * end point for the spline becomes the start point for the next spline.
     * Upon completion, the current (X,Y) position is the end point of the
     * last spline."
     * "The shape of each spline is controlled by a set of coordinates for
     * three connected lines. The spline starts at the current position and
     * ends at the end point of the third line. The end points of the first
     * and second lines are used as control points. An individual spline
     * always lies within the area bounded by the start, end, and control
     * points."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG PolySpline(const POINTL *paptlPoint, LONG cPoints);

    /*!
     * @brief Forward an area fill to the driver's GrePolyScanline.
     * @param [in] pScanData  Pointer to SCANDATA.
     * @return cHits code.
     * @retval GPI_OK    Forwarded (or accepted).
     * @retval GPI_HITS  Correlation hit.
     * @retval GPI_ERROR pScanData is NULL.
     *
     * @par EDM2 Reference
     * GrePolyScanline (http://www.edm2.com/index.php?title=GrePolyScanline&oldid=70434):
     * "GrePolyScanline fills an area lying between polyshortline pairs by
     * using the current pattern attribute. This function must be supported by
     * all presentation drivers except those that hook the GreDrawRLE,
     * GreEndArea, and GreFillPath functions. All function calls to
     * GrePolyScanline come from the GreDrawRLE, GreEndArea, or GreFillPath
     * handling routines in the graphics engine."
     *
     * @par Simulation support
     * "None. This function is mandatory for all drivers."
     *
     * @note This method is a thin forwarder.  GrePolyScanline is mandatory
     *       for the driver; the engine is not supposed to simulate it.  The
     *       engine's DrawRLE / EndArea / FillPath paths are the documented
     *       callers.  If the driver hooks those three, the driver owns the
     *       SCANDATA traversal entirely.
     */
    LONG PolyScanline(const SCANDATA *pScanData);

private:
    /*!
     * @brief Emit a line segment into m_pPath.
     * @param [in] a  Start point.
     * @param [in] b  End point.
     * @return GPI_OK.
     * @retval GPI_OK  Always.
     */
    LONG EmitLine(const POINTL &a, const POINTL &b);

    /*!
     * @brief Tessellate a cubic Bezier into line segments.
     * @param [in] p0   Start point.
     * @param [in] p1   First control point.
     * @param [in] p2   Second control point.
     * @param [in] p3   End point.
     * @param [in] segments  Number of line segments (32 recommended).
     * @return GPI_OK.
     * @retval GPI_OK  Always.
     */
    LONG Bezier(const POINTL &p0, const POINTL &p1,
                const POINTL &p2, const POINTL &p3, int segments);

    Fs_Path *m_pPath;   /*!< Owning path (may be NULL). */
    ULONG    m_hdc;     /*!< Owning DC handle (reserved). */
};

#endif /* FS_LINE_HPP */
