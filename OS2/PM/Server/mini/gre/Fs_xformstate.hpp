#ifndef FS_XFORMSTATE_HPP
#define FS_XFORMSTATE_HPP

#define INCL_BASE
#define INCL_PM
#include "F_platform.h"

/*!
 * @class Fs_XformState
 * @brief Viewing pipeline state for one DC.
 *
 * Holds four transform matrices (model, global viewing, window/viewport,
 * device), page units, page extent, device extent, page viewport,
 * graphics field, viewing limits, and per-level save stack.
 *
 * @par EDM2 Reference
 * "Transform functions (INCL_GRE_XFORMS)" category — all entry points are
 * simulated by a handling routine in the graphics engine.
 * GreGetPageUnits, GreSetPageUnits, GreGetModelXform, GreSetModelXform,
 * GreGetWindowViewportXform, GreSetWindowViewportXform,
 * GreGetGlobalViewingXform, GreSetGlobalViewingXform, GreGetPageViewport,
 * GreSetPageViewport, GreGetGraphicsField, GreSetGraphicsField,
 * GreGetViewingLimits, GreSetViewingLimits, GreQueryViewportSize,
 * GreConvert, GreConvertWithMatrix, GreMultiplyXforms, GreSaveXform,
 * GreRestoreXform, GreSaveXformData, GreRestoreXformData.
 */
class Fs_XformState {
public:
    /*!
     * @brief Construct with identity matrices and PU_PELS units.
     * @return Nothing (constructor).
     * @retval (void)  No return value.
     */
    Fs_XformState();

    /*!
     * @brief Destroy and release save stack.
     * @return Nothing (destructor).
     * @retval (void)  No return value.
     */
    ~Fs_XformState();

    /*!
     * @brief Reset to initial state.
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
     * @brief Get current page units and page extent.
     * @param [out] pExtent  Receives page dimensions in current units.
     * @return Page units constant, or GPI_ERROR.
     * @retval GPI_ERROR  pExtent is NULL.
     * @retval PU_*       Current page units (PU_PELS, PU_LOMETRIC, ...).
     *
     * @par EDM2 Reference
     * GreGetPageUnits (https://www.edm2.com/index.php/GreGetPageUnits):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG GetPageUnits(SIZEL *pExtent) const;

    /*!
     * @brief Set page units and page extent.
     * @param [in] ulUnits  PU_* units.
     * @param [in] lWidth   Page width (0 for PU_ARBITRARY: derive from extent).
     * @param [in] lHeight  Page height.
     * @return Success indicator.
     * @retval TRUE   Set.
     * @retval FALSE  Never (accepted for any ulUnits).
     *
     * @par EDM2 Reference
     * GreSetPageUnits (https://www.edm2.com/index.php/GreSetPageUnits):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL SetPageUnits(ULONG ulUnits, LONG lWidth, LONG lHeight);

    /*!
     * @brief Query model transform matrix.
     * @param [out] paXformData  Receives the 6 XFORM elements.
     * @return Success indicator.
     * @retval TRUE   Copied.
     * @retval FALSE  paXformData is NULL.
     *
     * @par EDM2 Reference
     * GreGetModelXform (https://www.edm2.com/index.php/GreGetModelXform):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL GetModelXform(XFORM *paXformData) const;

    /*!
     * @brief Set model transform matrix.
     * @param [in] paXformData  Pointer to XFORM (ignored when SX_UNITY).
     * @param [in] flOptions    SX_UNITY / SX_OVERWRITE / SX_CAT_BEFORE /
     *                          SX_CAT_AFTER.
     * @return Success indicator.
     * @retval TRUE   Applied.
     * @retval FALSE  NULL matrix with non-unity option, or unknown option.
     *
     * @par EDM2 Reference
     * GreSetModelXform (https://www.edm2.com/index.php/GreSetModelXform):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL SetModelXform(const XFORM *paXformData, LONG flOptions);

    /*!
     * @brief Query window/viewport transform matrix.
     * @param [out] paXformData  Receives the 6 XFORM elements.
     * @return Success indicator.
     * @retval TRUE   Copied.
     * @retval FALSE  paXformData is NULL.
     *
     * @par EDM2 Reference
     * GreGetWindowViewportXform
     * (https://www.edm2.com/index.php/GreGetWindowViewportXform):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL GetWindowViewportXform(XFORM *paXformData) const;

    /*!
     * @brief Set window/viewport transform matrix.
     * @param [in] paXformData  Pointer to XFORM.
     * @param [in] flOptions    SX_* option.
     * @return Success indicator.
     * @retval TRUE   Applied.
     * @retval FALSE  Invalid option.
     *
     * @par EDM2 Reference
     * GreSetWindowViewportXform
     * (https://www.edm2.com/index.php/GreSetWindowViewportXform):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL SetWindowViewportXform(const XFORM *paXformData, LONG flOptions);

    /*!
     * @brief Query global viewing transform matrix.
     * @param [out] paXformData  Receives the 6 XFORM elements.
     * @return Success indicator.
     * @retval TRUE   Copied.
     * @retval FALSE  paXformData is NULL.
     *
     * @par EDM2 Reference
     * GreGetGlobalViewingXform
     * (https://www.edm2.com/index.php/GreGetGlobalViewingXform):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL GetGlobalViewingXform(XFORM *paXformData) const;

    /*!
     * @brief Set global viewing transform matrix.
     * @param [in] paXformData  Pointer to XFORM.
     * @param [in] flOptions    SX_* option.
     * @return Success indicator.
     * @retval TRUE   Applied.
     * @retval FALSE  Invalid option.
     *
     * @par EDM2 Reference
     * GreSetGlobalViewingXform
     * (https://www.edm2.com/index.php/GreSetGlobalViewingXform):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL SetGlobalViewingXform(const XFORM *paXformData, LONG flOptions);

    /*!
     * @brief Query page viewport rectangle.
     * @param [out] prclViewport  Receives the viewport.
     * @return Success indicator.
     * @retval TRUE   Copied.
     * @retval FALSE  prclViewport is NULL.
     *
     * @par EDM2 Reference
     * GreGetPageViewport (https://www.edm2.com/index.php/GreGetPageViewport):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL GetPageViewport(RECTL *prclViewport) const;

    /*!
     * @brief Set page viewport rectangle.
     * @param [in] prclViewport  Pointer to viewport rectangle.
     * @return Success indicator.
     * @retval TRUE   Set.
     * @retval FALSE  NULL, or degenerate (xLeft>=xRight or yBottom>=yTop).
     *
     * @par EDM2 Reference
     * GreSetPageViewport (https://www.edm2.com/index.php/GreSetPageViewport):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL SetPageViewport(const RECTL *prclViewport);

    /*!
     * @brief Query graphics field rectangle.
     * @param [out] prclField  Receives the graphics field.
     * @return Success indicator.
     * @retval TRUE   Copied.
     * @retval FALSE  prclField is NULL.
     *
     * @par EDM2 Reference
     * GreGetGraphicsField
     * (https://www.edm2.com/index.php/GreGetGraphicsField):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL GetGraphicsField(RECTL *prclField) const;

    /*!
     * @brief Set graphics field rectangle.
     * @param [in] prclField  Pointer to graphics field.
     * @return Success indicator.
     * @retval TRUE   Set.
     * @retval FALSE  NULL or degenerate.
     *
     * @par EDM2 Reference
     * GreSetGraphicsField
     * (https://www.edm2.com/index.php/GreSetGraphicsField):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL SetGraphicsField(const RECTL *prclField);

    /*!
     * @brief Query viewing limits rectangle.
     * @param [out] prclLimits  Receives the limits.
     * @return Success indicator.
     * @retval TRUE   Copied (unset state returns infinite bounds).
     * @retval FALSE  prclLimits is NULL.
     *
     * @par EDM2 Reference
     * GreGetViewingLimits
     * (https://www.edm2.com/index.php/GreGetViewingLimits):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL GetViewingLimits(RECTL *prclLimits) const;

    /*!
     * @brief Set viewing limits rectangle.
     * @param [in] prclLimits  Pointer to viewing limits.
     * @return Success indicator.
     * @retval TRUE   Set.
     * @retval FALSE  prclLimits is NULL.
     *
     * @par EDM2 Reference
     * GreSetViewingLimits
     * (https://www.edm2.com/index.php/GreSetViewingLimits):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL SetViewingLimits(const RECTL *prclLimits);

    /*!
     * @brief Query viewport size.
     * @param [in]  ulType     Reserved (must be 0).
     * @param [out] psizlSize  Receives SIZEL of page viewport.
     * @return Success indicator.
     * @retval TRUE   Copied.
     * @retval FALSE  psizlSize is NULL.
     *
     * @par EDM2 Reference
     * GreQueryViewportSize (simulated transform function).
     */
    BOOL QueryViewportSize(ULONG ulType, SIZEL *psizlSize) const;

    /*!
     * @brief Convert coordinates between spaces.
     * @param [in]     lSrc        Source space (CVTC_*).
     * @param [in]     lDst        Destination space.
     * @param [in,out] paptlPoint  Array of POINTL.
     * @param [in]     cPoints     Number of points.
     * @return Success indicator.
     * @retval TRUE   Converted.
     * @retval FALSE  paptlPoint NULL or cPoints < 0.
     *
     * @par EDM2 Reference
     * GreConvert (https://www.edm2.com/index.php/GreConvert):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL Convert(LONG lSrc, LONG lDst, PPOINTL paptlPoint, LONG cPoints) const;

    /*!
     * @brief Convert coordinates using an explicit matrix.
     * @param [in,out] paptlPoint  Array of POINTL.
     * @param [in]     cPoints     Number of points.
     * @param [in]     pxform      Transform matrix to apply.
     * @return Success indicator.
     * @retval TRUE   Converted.
     * @retval FALSE  NULL argument or cPoints < 0.
     *
     * @par EDM2 Reference
     * GreConvertWithMatrix
     * (https://www.edm2.com/index.php/GreConvertWithMatrix):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL ConvertWithMatrix(PPOINTL paptlPoint, LONG cPoints,
                           const XFORM *pxform) const;

    /*!
     * @brief Multiply transform matrix by another.
     * @param [in,out] paXform        In: original; out: result.
     * @param [in]     paNewXformData Multiplier.
     * @param [in]     lMode          SX_CAT_BEFORE / SX_CAT_AFTER / SX_OVERWRITE.
     * @return Success indicator.
     * @retval TRUE   Multiplied.
     * @retval FALSE  NULL arguments or unknown mode.
     *
     * @par EDM2 Reference
     * GreMultiplyXforms
     * (https://www.edm2.com/index.php/GreMultiplyXforms):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL MultiplyXforms(XFORM *paXform, const XFORM *paNewXformData,
                        LONG lMode) const;

    /*!
     * @brief Save transform state (called during OpenDC / SaveDC).
     * @param [in] cSave  Save level (1 = init, N>=2 = push).
     * @return Success indicator.
     * @retval TRUE   Saved.
     * @retval FALSE  Negative level or allocation failure.
     *
     * @par EDM2 Reference
     * GreSaveXform (https://www.edm2.com/index.php/GreSaveXform):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL SaveXform(LONG cSave);

    /*!
     * @brief Restore transform state (called during CloseDC / RestoreDC).
     * @param [in] cSave  -1/0/1 = reset; N>=2 = restore level N.
     * @return Success indicator.
     * @retval TRUE   Restored.
     * @retval FALSE  Level out of range.
     *
     * @par EDM2 Reference
     * GreRestoreXform (https://www.edm2.com/index.php/GreRestoreXform):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL RestoreXform(LONG cSave);

    /*!
     * @brief Save transform state to caller buffer.
     * @param [in]  ulSize   Buffer size, or 0 to query required size.
     * @param [out] pBuffer  Caller buffer (NULL when ulSize == 0).
     * @return Required size (when ulSize == 0), GPI_OK, or GPI_ERROR.
     * @retval 0           ulSize == 0 (size query).
     * @retval GPI_OK      Success.
     * @retval GPI_ERROR   Buffer too small or NULL.
     *
     * @par EDM2 Reference
     * GreSaveXformData
     * (https://www.edm2.com/index.php/GreSaveXformData):
     * "This function is simulated by a handling routine in the graphics engine."
     *
     * @note Serializes 4 matrices + ulUnits + sizlPage + sizlDevice +
     *       3 rectangles + bViewingLimitsSet.
     */
    ULONG SaveXformData(ULONG ulSize, PBYTE pBuffer) const;

    /*!
     * @brief Restore transform state from caller buffer.
     * @param [in] ulSize   Buffer size.
     * @param [in] pBuffer  Caller buffer.
     * @return Success indicator.
     * @retval TRUE   Restored.
     * @retval FALSE  Buffer too small or NULL.
     *
     * @par EDM2 Reference
     * GreRestoreXformData
     * (https://www.edm2.com/index.php/GreRestoreXformData):
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL RestoreXformData(ULONG ulSize, const BYTE *pBuffer);

private:
    struct SaveState {
        XFORM  model, globalView, windowViewport, device;
        ULONG  ulUnits;
        SIZEL  sizlPage, sizlDevice;
        RECTL  rclPageViewport, rclGraphicsField, rclViewingLimits;
        BOOL   bViewingLimitsSet;
    };

    /*!
     * @brief Write identity matrix into x.
     * @param [out] x  Matrix to set.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void SetIdentity(XFORM &x) const;

    /*!
     * @brief Multiply dst = a * b (FIXED 16.16).
     * @param [out] dst  Result.
     * @param [in]  a    Left.
     * @param [in]  b    Right.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void Multiply(XFORM &dst, const XFORM &a, const XFORM &b) const;

    /*!
     * @brief Compose transform between two coordinate spaces.
     * @param [out] xform  Resulting XFORM.
     * @param [in]  lSrc   Source CVTC_*.
     * @param [in]  lDst   Destination CVTC_*.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void BuildTransform(XFORM &xform, LONG lSrc, LONG lDst) const;

    XFORM  m_model;
    XFORM  m_globalView;
    XFORM  m_windowViewport;
    XFORM  m_device;

    ULONG  m_ulUnits;
    SIZEL  m_sizlPage;
    SIZEL  m_sizlDevice;

    RECTL  m_rclPageViewport;
    RECTL  m_rclGraphicsField;
    RECTL  m_rclViewingLimits;
    BOOL   m_bViewingLimitsSet;

    SaveState *m_pSaveStack;
    ULONG      m_ulSaveCount;
    ULONG      m_ulSaveAlloc;
};

#endif /* FS_XFORMSTATE_HPP */
