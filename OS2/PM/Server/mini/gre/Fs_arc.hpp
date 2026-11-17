#ifndef FS_ARC_HPP
#define FS_ARC_HPP

#define INCL_BASE
#include "F_platform.h"

#define INCL_GREALL
#define INCL_GRE_PATHS
#define INCL_DDIPATHS
#include "pmddi.h"

class Fs_Path;
class Fs_Dc;

/*!
 * @class Fs_Arc
 * @brief Arc/fillet/box state for one DC.
 *
 * Holds the current ARCPARAMS.  All drawing functions tessellate into
 * line segments and emit them into the owning Fs_Path via AddLine.
 *
 * @par EDM2 Reference
 * "Arc functions (INCL_GRE_ARCS)" category — all entry points are
 * simulated by a handling routine in the graphics engine.
 */
class Fs_Arc {
public:
    /*!
     * @brief Construct with zeroed ARCPARAMS.
     * @return Nothing (constructor).
     * @retval (void)  No return value.
     */
    Fs_Arc();

    /*!
     * @brief Destroy.
     * @return Nothing (destructor).
     * @retval (void)  No return value.
     */
    ~Fs_Arc();

    /*!
     * @brief Reset ARCPARAMS to zero.
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
     * @brief Set the current arc parameters.
     * @param [in] pArcParms  Pointer to ARCPARAMS (lP, lQ, lR, lS).
     * @return Success indicator.
     * @retval TRUE   Stored.
     * @retval FALSE  pArcParms is NULL.
     *
     * @par EDM2 Reference
     * GreSetArcParameters
     * (https://www.edm2.com/index.php/GreSetArcParameters):
     * "GreSetArcParameters sets the arc parameters."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
//    BOOL SetArcParameters(const ARCPARAMS *pArcParms);

    /*!
     * @brief Query the current arc parameters.
     * @param [out] pArcParms  Receives ARCPARAMS.
     * @return Success indicator.
     * @retval TRUE   Copied.
     * @retval FALSE  pArcParms is NULL.
     */
//    BOOL QueryArcParameters(ARCPARAMS *pArcParms) const;

    /*!
     * @brief Draw an arc through three points.
     * @param [in] paptlPoint  Array of 3 POINTL (start, mid, end).
     * @return cHits code.
     * @retval GPI_OK    Drawn.
     * @retval GPI_HITS  Correlation hit.
     * @retval GPI_ERROR paptlPoint is NULL.
     *
     * @par EDM2 Reference
     * GreArc (https://www.edm2.com/index.php/GreArc):
     * "GreArc draws an arc through three points."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     *
     * @warning Approximated by two straight segments; full three-point
     *          circle math is not implemented.
     */
    LONG Arc(const POINTL *paptlPoint);

    /*!
     * @brief Draw a partial arc.
     * @param [in] pptlCenter   Center of arc.
     * @param [in] fxMultiplier FIXED multiplier (64K = 1).
     * @param [in] fxStart      FIXED start angle (6553 = 1°).
     * @param [in] fxSweep      FIXED sweep angle.
     * @return cHits code.
     * @retval GPI_OK    Drawn.
     * @retval GPI_HITS  Correlation hit.
     * @retval GPI_ERROR pptlCenter is NULL.
     *
     * @par EDM2 Reference
     * GrePartialArc (https://www.edm2.com/index.php/GrePartialArc):
     * "GrePartialArc draws a straight line from the current position to the
     * starting point of a partial arc and draws the specified partial arc."
     * "A value of 6553 gives an angle of 1°. Both angles must be positive."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG PartialArc(const POINTL *pptlCenter, FIXED fxMultiplier,
                    FIXED fxStart, FIXED fxSweep);

    /*!
     * @brief Draw a filled full arc (interior only).
     * @param [in] fxMultiplier  FIXED multiplier.
     * @return cHits code.
     * @retval GPI_OK    Drawn.
     * @retval GPI_HITS  Correlation hit.
     * @retval GPI_ERROR Error.
     *
     * @par EDM2 Reference
     * GreFullArcInterior (simulated arc function):
     * "Draws a filled full arc, using the current pattern attributes, with
     * its center at the current (x,y) position."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG FullArcInterior(FIXED fxMultiplier);

    /*!
     * @brief Draw a full arc boundary.
     * @param [in] fxMultiplier  FIXED multiplier.
     * @return cHits code.
     * @retval GPI_OK / GPI_HITS / GPI_ERROR.
     *
     * @par EDM2 Reference
     * GreFullArcBoundary (simulated arc function).
     */
    LONG FullArcBoundary(FIXED fxMultiplier);

    /*!
     * @brief Draw a full arc (both interior and boundary).
     * @param [in] fxMultiplier  FIXED multiplier.
     * @return cHits code.
     * @retval GPI_OK / GPI_HITS / GPI_ERROR.
     *
     * @par EDM2 Reference
     * GreFullArcBoth (simulated arc function).
     */
    LONG FullArcBoth(FIXED fxMultiplier);

    /*!
     * @brief Draw a rectangular box (interior only).
     * @param [in] pBox  Pointer to BOXDATA.
     * @return cHits code.
     * @retval GPI_OK / GPI_HITS / GPI_ERROR.
     *
     * @par EDM2 Reference
     * GreBoxInterior (https://www.edm2.com/index.php/GreBoxInterior):
     * "Draws a rectangular box with one corner at the current (X,Y) position
     * and the opposite corner at the specified (X,Y) position."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG BoxInterior(Fs_Dc *pDc, const BOXDATA *pBox);

    /*!
     * @brief Draw a rectangular box (boundary only).
     * @param [in] pBox  Pointer to BOXDATA.
     * @return cHits code.
     * @retval GPI_OK / GPI_HITS / GPI_ERROR.
     *
     * @par EDM2 Reference
     * GreBoxBoundary (https://www.edm2.com/index.php/GreBoxBoundary).
     */
    LONG BoxBoundary(Fs_Dc *pDc, const BOXDATA *pBox);

    /*!
     * @brief Draw a rectangular box (interior and boundary).
     * @param [in] pBox  Pointer to BOXPARAMS.
     * @return cHits code.
     * @retval GPI_OK / GPI_HITS / GPI_ERROR.
     *
     * @par EDM2 Reference
     * GreBoxBoth (simulated arc function).
     */
    LONG BoxBoth(Fs_Dc *pDc, const BOXPARAMS *pBox);

    /*!
     * @brief Draw a poly-fillet.
     * @param [in] paptlPoint  Array of points.
     * @param [in] cPoints     Number of points (0 returns GPI_OK).
     * @return cHits code.
     * @retval GPI_OK / GPI_HITS / GPI_ERROR.
     *
     * @par EDM2 Reference
     * GrePolyFillet (https://www.edm2.com/index.php/GrePolyFillet):
     * "draws a sequence of one or more fillets starting at the current
     * (X,Y) position."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG PolyFillet(const POINTL *paptlPoint, LONG cPoints);

    /*!
     * @brief Draw a sharp poly-fillet.
     * @param [in] paptlPoint  Array of points.
     * @param [in] cPoints     Number of points.
     * @param [in] pfxSharp    Array of sharpness values (FIXED, 64K = 1).
     * @return cHits code.
     * @retval GPI_OK / GPI_HITS / GPI_ERROR.
     *
     * @par EDM2 Reference
     * GrePolyFilletSharp
     * (https://www.edm2.com/index.php/GrePolyFilletSharp):
     * "An array of sharpness values with one element for each fillet.
     *  \"Sharpness\" is defined as a fixed-point value."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG PolyFilletSharp(const POINTL *paptlPoint, LONG cPoints,
                         const FIXED *pfxSharp);

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
     * @brief Tessellate an elliptical arc into line segments.
     * @param [in] center   Center.
     * @param [in] rx       X radius.
     * @param [in] ry       Y radius.
     * @param [in] fxStart  Start angle (FIXED).
     * @param [in] fxSweep  Sweep angle (FIXED).
     * @return GPI_OK.
     * @retval GPI_OK  Always.
     */
    LONG EmitArc(const POINTL &center, LONG rx, LONG ry,
                 FIXED fxStart, FIXED fxSweep);

//    ARCPARAMS m_arcParms;   /*!< Current arc parameters. */
    Fs_Path  *m_pPath;      /*!< Owning path (may be NULL). */
};

#endif /* FS_ARC_HPP */
