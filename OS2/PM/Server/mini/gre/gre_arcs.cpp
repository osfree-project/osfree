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
static BOOL __cdecl GreGetArcParameters(HDC hdc, PARCPARAMS pArcParms,
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
static BOOL __cdecl GreSetArcParameters(HDC hdc, PARCPARAMS pArcParms,
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
 * "This function can be hooked by the presentation driver."
 *
 * @par Simulation support
 * "This function is simulated by a handling routine in the graphics engine."
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
static LONG __cdecl GreArc(HDC hdc, PPOINTL paptlPoint,
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
static LONG __cdecl GrePartialArc(HDC hdc, PPOINTL pptlCenter,
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
static LONG __cdecl GreFullArcInterior(HDC hdc, FIXED fxAngle,
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
static LONG __cdecl GreFullArcBoundary(HDC hdc, FIXED fxAngle,
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
static LONG __cdecl GreFullArcBoth(HDC hdc, FIXED fxAngle,
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
static LONG __cdecl GreBoxInterior(HDC hdc, PPOINTL pBox,
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
static LONG __cdecl GreBoxBoundary(HDC hdc, PPOINTL pBox,
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
static LONG __cdecl GreBoxBoth(HDC hdc, PBOXPARAMS pBox,
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
static LONG __cdecl GrePolyFillet(HDC hdc, PPOINTL paptlPoint,
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
static LONG __cdecl GrePolyFilletSharp(HDC hdc, PPOINTL paptlPoint,
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

   /**
    * @brief Draw a spline.
    *
    * @param[in] hdc        Device context handle.
    * @param[in] paptlPoint Pointer to an array of POINTL structures defining
    *                       the control points of the spline.
    * @param[in] cPoints    Number of (X,Y) pairs.
    *
    * @return Integer (cHits) indicating correlation/error status.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_HITS  Successful with correlation hit.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BASE_ERROR
    *       - PMERR_BITMAP_NOT_SELECTED
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_IN_AREA
    *       - PMERR_NOT_IN_PATH
    *       - PMERR_PATH_LIMIT_EXCEEDED
    */
   #define NGrePolySpline 0x0000400CL
   #define GrePolySpline(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),0L,NGrePolySpline)

   /**
    * @brief Draw an oval.
    *
    * @param[in] hdc       Device context handle.
    * @param[in] pptl      Pointer to a POINTL structure.
    * @param[in] fx        FIXED value.
    * @param[in] fy        FIXED value.
    * @param[in] cOptions  Option flags.
    *
    * @return Integer (cHits) indicating correlation/error status.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_HITS  Successful with correlation hit.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BASE_ERROR
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_COORD_SPACE
    *       - PMERR_INV_HDC
    *       - PMERR_INV_IN_AREA
    *       - PMERR_INV_IN_PATH
    *       - PMERR_PATH_LIMIT_EXCEEDED
    */
   #define NGreOval 0x0000400DL
   #define GreOval(a,b,c,d,e) (INT) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(FIXED)(c),(ULONG)(FIXED)(d),(ULONG)(e),0L,NGreOval)

   /* GreCookWholePath — no verifiable error list found; left undocumented. */
   #define NGreCookWholePath 0x0000400EL
   #define GreCookWholePath(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPATH)(b),0L,NGreCookWholePath)

   /* GreCookPathCurves — no verifiable error list found; left undocumented. */
   #define NGreCookPathCurves 0x0000400FL
   #define GreCookPathCurves(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPATH)(b),(ULONG)(PPIPELINEINFO)(c),0L,NGreCookPathCurves)

   /**
    * @brief Render a path.
    *
    * @param[in] hdc       Device context handle.
    * @param[in] pPath     Pointer to a PATH structure.
    * @param[in] pCurve    Pointer to a CURVE structure.
    * @param[in] cLines    Number of lines.
    * @param[in] cCurves   Number of curves.
    * @param[in] cPoints   Number of points.
    * @param[in] pShortLine Pointer to a SHORTLINE structure.
    * @param[in] lOptions  Option flags.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BASE_ERROR
    *       - PMERR_PATH_LIMIT_EXCEEDED
    *       - PMERR_HDC_BUSY
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_INV_LENGTH_OR_COUNT
    *       - PMERR_INV_HDC
    *       - PMERR_INV_COORD_SPACE
    */
   #define NGreRenderPath 0x00004011L
   #define GreRenderPath(a,b,c,d,e,f,g,h) (BOOL) Gre32Entry10((ULONG)(HDC)(a),(ULONG)(PPATH)(b),(ULONG)(PCURVE)(c),(ULONG)(LONG)(d),(ULONG)(LONG)(e),(ULONG)(LONG)(f),(ULONG)(PSHORTLINE)(g),(ULONG)(LONG)(h),0L,NGreRenderPath)
