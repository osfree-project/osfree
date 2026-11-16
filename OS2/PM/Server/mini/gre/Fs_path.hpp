#ifndef FS_PATH_HPP
#define FS_PATH_HPP

#define INCL_BASE
#define INCL_PM
#include "F_platform.h"

class Fs_Region;

/*!
 * @class Fs_Path
 * @brief One path under construction or being rendered for one DC.
 *
 * Holds an array of LINE structures (from PMDDI.H).  Storage uses
 * DosAllocMem/DosFreeMem only.  Path id is always 1 per EDM2.
 *
 * @par EDM2 Reference
 * "Path functions (INCL_GRE_PATHS)" category — all entry points are
 * simulated by a handling routine in the graphics engine.
 */
class Fs_Path {
public:
    /*!
     * @brief Construct an empty path.
     * @return Nothing (constructor).
     * @retval (void)  No return value.
     */
    Fs_Path();

    /*!
     * @brief Destroy the path and release storage.
     * @return Nothing (destructor).
     * @retval (void)  No return value.
     */
    ~Fs_Path();

    /*!
     * @brief Bind the path to its owning DC.
     * @param [in] hdc  Owning device context handle.
     * @return Nothing.
     * @retval (void)  No return value.
     * @note Required so that Flush() can pass a real HDC to
     *       GreDrawLinesInPath instead of 0.
     */
    void SetHdc(ULONG hdc) { m_hdc = hdc; }

    /*!
     * @brief Reset to initial state (no path, no save stack).
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void Init();

    /*!
     * @brief Alias for Init (symmetry with other Fs_*).
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void Shutdown();

    /*!
     * @brief Begin a path definition.
     * @param [in] idPath  Path identifier; must be 1.
     * @return Success indicator.
     * @retval TRUE   Path started.
     * @retval FALSE  idPath != 1, or a path is already open.
     *
     * @par EDM2 Reference
     * GreBeginPath (https://www.edm2.com/index.php/GreBeginPath):
     * "GreBeginPath identifies the start of a sequence of figures that
     * define a path."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL BeginPath(LONG idPath);

    /*!
     * @brief End a path definition.
     * @param [in] flOptions  Reserved (must be 0).
     * @return Success indicator.
     * @retval TRUE  Always (valid outside a path and has no effect).
     *
     * @par EDM2 Reference
     * GreEndPath (https://www.edm2.com/index.php/GreEndPath):
     * "This function is valid outside a path definition but has no effect."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL EndPath(ULONG flOptions);

    /*!
     * @brief Close the current figure.
     * @return Success indicator.
     * @retval TRUE   Figure closed (or no figure was open).
     * @retval FALSE  No path is open.
     *
     * @par EDM2 Reference
     * GreCloseFigure (https://www.edm2.com/index.php/GreCloseFigure):
     * "GreCloseFigure closes a figure within a path definition by drawing
     * a line from the current (X,Y) position to the start point of the
     * figure."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL CloseFigure();

    /*!
     * @brief Fill the interior of the closed figures in the path.
     * @param [in] flOptions  FPATH_ALTERNATE / FPATH_WINDING / FPATH_INCL /
     *                        FPATH_EXCL.
     * @return cHits code.
     * @retval GPI_OK    Success.
     * @retval GPI_HITS  Success with correlation hit.
     * @retval GPI_ERROR Error from GreDrawLinesInPath.
     *
     * @par EDM2 Reference
     * GreFillPath (https://www.edm2.com/index.php/GreFillPath):
     * "GreFillPath fills the interior of the closed figure defined in the
     * path by using the current pattern attributes."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG FillPath(ULONG flOptions);

    /*!
     * @brief Outline the path.
     * @param [in] flOptions  OPTH_NO_CLIPPING_REQD / OPTH_QRY_PATH_POINTS.
     * @return cHits code, or point count when OPTH_QRY_PATH_POINTS is set.
     * @retval GPI_OK    Success.
     * @retval GPI_HITS  Success with correlation hit.
     * @retval GPI_ERROR Error.
     * @retval >=0       Point count (OPTH_QRY_PATH_POINTS).
     *
     * @par EDM2 Reference
     * GreOutlinePath (https://www.edm2.com/index.php/GreOutlinePath):
     * "OPTH_NO_CLIPPING_REQD: The graphics engine will outline the path
     * without clipping and line attributes set. This output will be sent
     * to the presentation driver via GreDrawLinesInPath for each subpath."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG OutlinePath(ULONG flOptions);

    /*!
     * @brief Modify a path (currently MPATH_STROKE only).
     * @param [in] cmdMode  MPATH_STROKE.
     * @return Success indicator.
     * @retval TRUE   Accepted (path left unchanged).
     * @retval FALSE  Invalid cmdMode or no path.
     *
     * @par EDM2 Reference
     * GreModifyPath (https://www.edm2.com/index.php/GreModifyPath):
     * "MPATH_STROKE: This replaces the original path using the current
     * geometric wide line attribute."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     *
     * @warning Envelope construction is not implemented; the path is left
     *          unchanged.  Call returns TRUE.
     */
    BOOL ModifyPath(LONG cmdMode);

    /*!
     * @brief Convert path to envelope of wide line and fill it.
     * @param [in] flOptions  Reserved (must be 0).
     * @return cHits code.
     * @retval GPI_OK    Success.
     * @retval GPI_HITS  Success with correlation hit.
     * @retval GPI_ERROR Error.
     *
     * @par EDM2 Reference
     * GreStrokePath (https://www.edm2.com/index.php/GreStrokePath):
     * "This function converts a path to the envelope of a wide line by
     * using the current geometric line attribute. The converted path is
     * filled by using winding mode and the current area attributes, and is
     * then drawn."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     *
     * @warning Envelope construction is not implemented; path filled as-is.
     */
    LONG StrokePath(ULONG flOptions);

    /*!
     * @brief Save path state (from OpenDC / SaveDC).
     * @param [in] cSave  Save level (1 = init, N>=2 = push).
     * @return Success indicator.
     * @retval TRUE   Saved.
     * @retval FALSE  Negative cSave or allocation failure.
     *
     * @par EDM2 Reference
     * GreSavePath (https://www.edm2.com/index.php/GreSavePath):
     * "When a new DC is created, GreSavePath is called with a count of 1
     * to initialize its local data."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL SavePath(LONG cSave);

    /*!
     * @brief Restore path state (from CloseDC / RestoreDC).
     * @param [in] cSave  -1/0/1 = reset; N>=2 = restore level N.
     * @return Success indicator.
     * @retval TRUE   Restored.
     * @retval FALSE  Level out of range.
     *
     * @par EDM2 Reference
     * GreRestorePath (https://www.edm2.com/index.php/GreRestorePath):
     * "When a DC is closed, GreRestorePath is called with a count of 0 to
     * free its local data."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL RestorePath(LONG cSave);

    /*!
     * @brief Add a line segment to the path.
     * @param [in] ptlStart  Start point (device coordinates).
     * @param [in] ptlEnd    End point (device coordinates).
     * @return Nothing.
     * @retval (void)  No return value.
     * @note If this is the first line of a figure, m_ptlFigureStart is set
     *       to ptlStart.  Current position is updated to ptlEnd.
     */
    void AddLine(const POINTL &ptlStart, const POINTL &ptlEnd);

    /*!
     * @brief Set current position (called by GreSetCurrentPosition).
     * @param [in] ptl  New current position.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void SetCurrentPosition(const POINTL &ptl) { m_ptlCurrent = ptl; }

    /*!
     * @brief Set figure start point.
     * @param [in] ptl  New figure start.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void SetFigureStart(const POINTL &ptl) { m_ptlFigureStart = ptl; }

    /*!
     * @brief Query whether a path is currently open.
     * @return Flag.
     * @retval TRUE   Open.
     * @retval FALSE  Closed.
     */
    BOOL IsOpen() const { return m_bOpen; }

    /*!
     * @brief Number of line segments in the path.
     * @return Count.
     * @retval >=0  Number of lines.
     */
    ULONG LineCount() const { return m_ulLineCount; }

    /*!
     * @brief Pointer to first LINE structure.
     * @return Pointer.
     * @retval NULL     Empty.
     * @retval non-NULL First LINE.
     */
    PLINE Lines() const { return (PLINE)m_pLines; }

    /*!
     * @brief Convert the current path to a region (scanline tessellation).
     * @param [out] out  Receives the resulting region.
     * @return Success indicator.
     * @retval TRUE   Converted.
     * @retval FALSE  Allocation failure.
     * @note Rectangular approximation: each line is converted to a thin
     *       axis-aligned rectangle.  Used for GreSelectClipPath(idPath!=0).
     */
    BOOL ToRegion(Fs_Region &out) const;

private:
    struct SaveState {
        BOOL   bOpen;
        PLINE  pLines;
        ULONG  ulLineCount;
        ULONG  ulAlloc;
        POINTL ptlFigureStart;
        POINTL ptlCurrent;
    };

    /*!
     * @brief Ensure capacity for at least cNeeded LINE entries.
     * @param [in] cNeeded  Minimum required capacity.
     * @return Nothing.
     * @retval (void)  No return value.
     * @note Uses DosAllocMem + memcpy + DosFreeMem to emulate realloc.
     */
    void EnsureCapacity(ULONG cNeeded);

    /*!
     * @brief Release the LINE array and reset counters.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void FreeLines();

    /*!
     * @brief Emit lines via GreDrawLinesInPath (real HDC, no error masking).
     * @param [in] flOptions  Flags passed to GreDrawLinesInPath.
     * @param [in] bFill      TRUE for fill, FALSE for outline.
     * @return Result of GreDrawLinesInPath (GPI_OK / GPI_HITS / GPI_ERROR).
     * @retval GPI_OK / GPI_HITS / GPI_ERROR
     *
     * @par EDM2 Reference
     * GreOutlinePath: "The graphics engine will outline the path without
     * clipping and line attributes set. This output will be sent to the
     * presentation driver via GreDrawLinesInPath for each subpath."
     */
    LONG Flush(ULONG flOptions, BOOL bFill);

    ULONG   m_hdc;
    BOOL    m_bOpen;
    LINE   *m_pLines;
    ULONG   m_ulLineCount;
    ULONG   m_ulAlloc;
    POINTL  m_ptlFigureStart;
    POINTL  m_ptlCurrent;

    SaveState *m_pSaveStack;
    ULONG      m_ulSaveCount;
    ULONG      m_ulSaveAlloc;
};

#endif /* FS_PATH_HPP */
