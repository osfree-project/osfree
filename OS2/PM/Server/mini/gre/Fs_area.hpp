#ifndef FS_AREA_HPP
#define FS_AREA_HPP

#define INCL_BASE
#include "F_platform.h"

class Fs_Path;

/*!
 * @class Fs_Area
 * @brief Area-bracket state for one DC.
 *
 * An area is a sequence of drawing functions between GreBeginArea and
 * GreEndArea. All boundaries of the area are considered part of the
 * interior and are filled when the bracket closes.
 *
 * @par EDM2 Reference
 * "Area functions": GreBeginArea, GreEndArea, GreAreaSetAttributes —
 * all simulated by a handling routine in the graphics engine.
 */
class Fs_Area {
public:
    /*!
     * @brief Construct an empty area state.
     * @return Nothing (constructor).
     * @retval (void)  No return value.
     */
    Fs_Area();

    /*!
     * @brief Destroy and release attribute copy.
     * @return Nothing (destructor).
     * @retval (void)  No return value.
     */
    ~Fs_Area();

    /*!
     * @brief Reset to initial state (no area open, no stored attrs).
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
     * @brief Bind to the owning path (for fill on End).
     * @param [in] pPath  Owning Fs_Path.
     * @return Nothing.
     * @retval (void)  No return value.
     * @note Required for End() to call FillPath with the correct DC HDC.
     */
    void SetPath(Fs_Path *pPath) { m_pPath = pPath; }

    /*!
     * @brief Begin an area bracket.
     * @param [in] flOptions  BA_NOBOUNDARY / BA_BOUNDARY,
     *                        BA_ALTERNATE / BA_WINDING, BA_EXCL.
     * @return Success indicator.
     * @retval TRUE   Area bracket opened.
     * @retval FALSE  Area bracket already open.
     *
     * @par EDM2 Reference
     * GreBeginArea (https://www.edm2.com/index.php/GreBeginArea):
     * "GreBeginArea indicates the beginning of a set of drawing functions
     * that define the boundary of an area. All of the boundaries of the area
     * are considered to be part of the interior and are filled."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL Begin(ULONG flOptions);

    /*!
     * @brief End an area bracket and fill it.
     * @param [in] flCancel  EA_DRAW (default) or EA_CANCEL.
     * @return cHits code.
     * @retval GPI_OK    Filled successfully, or cancelled/no-op.
     * @retval GPI_HITS  Filled with correlation hit.
     * @retval GPI_ERROR Error (no path bound, no bracket, or fill failed).
     *
     * @par EDM2 Reference
     * GreEndArea (https://www.edm2.com/index.php/GreEndArea):
     * "If the final (X,Y) position is not the same as the starting position
     * of the last figure in the area, the handling routine must close the
     * figure by drawing a line from the current position to the start of
     * the final figure."
     * "If the flOptions bit BA_EXCL (specified on the GreBeginArea call) was
     * set on and the driver is calling the engine with fill path for
     * simulation, the fill path flOptions parameter must have the FPATH_EXCL
     * bit set."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG End(ULONG flCancel);

    /*!
     * @brief Save area attributes for later fill.
     * @param [in] lPrimType   PRIM_LINE / PRIM_CHAR / PRIM_MARKER /
     *                         PRIM_AREA / PRIM_IMAGE.
     * @param [in] flDefsMask  Attributes to reset to default.
     * @param [in] flAttrsMask Attributes to modify.
     * @param [in] pAttrs      Pointer to the fixed-format bundle record.
     * @return Success indicator.
     * @retval TRUE   Attributes stored (or no-op when mask == 0).
     * @retval FALSE  Allocation failure or unknown primitive type.
     *
     * @par EDM2 Reference
     * GreAreaSetAttributes
     * (https://www.edm2.com/index.php/GreAreaSetAttributes):
     * "The handling routine in the graphics engine does nothing. Its purpose
     * is to provide an entry in the dispatch table that can be hooked by
     * presentation drivers. This function must be hooked by presentation
     * drivers that perform their own area or path simulations."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     *
     * @warning The engine copies sizeof(bundle) bytes. The caller must
     *          provide a buffer large enough for the highest offset
     *          attribute referenced (per EDM2). For partial bundles the
     *          caller should pad the buffer to the full bundle size.
     */
    BOOL SetAttributes(LONG lPrimType, ULONG flDefsMask,
                       ULONG flAttrsMask, PVOID pAttrs);

    /*!
     * @brief Query whether an area bracket is open.
     * @return Flag.
     * @retval TRUE   Open.
     * @retval FALSE  Closed.
     */
    BOOL IsOpen() const { return m_bOpen; }

private:
    /*!
     * @brief Release the stored attribute copy.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void FreeAttrs();

    Fs_Path *m_pPath;          /*!< Owning path (for fill on End). */
    BOOL     m_bOpen;          /*!< Area bracket currently open. */
    ULONG    m_ulBeginFlags;   /*!< BA_* flags from GreBeginArea. */
    LONG     m_lPrimType;      /*!< PRIM_* bundle type from SetAttributes. */
    ULONG    m_ulDefsMask;     /*!< flDefsMask from SetAttributes. */
    ULONG    m_ulAttrsMask;    /*!< flAttrsMask from SetAttributes. */
    BYTE    *m_pAttrs;         /*!< Copy of the bundle (DosAllocMem). */
    ULONG    m_ulAttrsSize;    /*!< Size of m_pAttrs in bytes. */
};

#endif /* FS_AREA_HPP */
