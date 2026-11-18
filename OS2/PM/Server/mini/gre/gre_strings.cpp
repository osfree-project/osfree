/*!
 * @brief Dispatch for GreCharString — draw a character string from current position.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] cChars     Number of characters in the string.
 * @param [in] pchString  Pointer to the character string.
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order
 *                        WORD=NGreCharString.
 *
 * @return Integer (cHits) indicating correlation/error status.
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit (returned by display
 *                   drivers when the correlate flag is ON, and a hit is
 *                   detected).
 * @retval GPI_ERROR Error.
 *
 * @par EDM2 Reference
 * GreCharString (https://www.edm2.com/index.php/GreCharString):
 * "GreCharString draws a character string starting at the current (X,Y)
 * position. Upon completion, the current (X,Y) position is the start point
 * for the character cell immediately after the last character in the string."
 * "GreCharString must be supported by the presentation driver."
 * "The handling routine must provide full support for drawing characters
 * from an image font in CM_MODE1 when the character direction is
 * CHDIRN_LEFTRIGHT."
 * "For outline characters or characters in any other mode or direction,
 * the handling routine can dispatch the call to the graphics engine at
 * the address given for this call in the default dispatch table."
 * "GreCharString is called by the function GpiCharString."
 *
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |   None    |  Must  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_BASE_ERROR
 * - PMERR_COORDINATE_OVERFLOW
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_EXCEEDS_MAX_SEG_LENGTH
 * - PMERR_FONT_AND_MODE_MISMATCH
 * - PMERR_HDC_BUSY
 * - PMERR_HRGN_BUSY
 * - PMERR_HUGE_FONTS_NOT_SUPPORTED
 * - PMERR_INSUFFICIENT_MEMORY
 * - PMERR_INV_HDC
 * - PMERR_INV_IN_AREA
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_PATH_LIMIT_EXCEEDED
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
GRE_DRV_5 (GreDrvCharString)

/*!
 * @brief Dispatch for GreCharStringPos — draw a character string at a position.
 *
 * @param [in] hdc         Device context handle.
 * @param [in] pptlStart   Pointer to (X,Y) coordinates of start position.
 * @param [in] prclRect    Pointer to an opaque or clip rectangle defined as
 *                         a RECTL structure. This rectangle, which is in
 *                         world coordinates, is used as the clipping
 *                         rectangle or as the background for the string,
 *                         or both, depending on the value of flOptions.
 *                         When the CHS_OPAQUE flag is set, normal background
 *                         mix attributes are ignored and the rectangle is
 *                         drawn using overpaint and the character background
 *                         color attribute. When neither CHS_OPAQUE nor
 *                         CHS_CLIP are specified, this parameter is ignored.
 *                         Notice that points on the boundary of this
 *                         rectangle are considered to be inside the
 *                         rectangle.
 * @param [in] flOptions   Options flag. The following flags can be used in
 *                         combination:
 *                         - CHS_OPAQUE: background of characters is defined
 *                           by the rectangle prclRect. The rectangle is to
 *                           be shaded (with background color and overpaint)
 *                           before drawing.
 *                         - CHS_VECTOR: increment vector supplied (pAdx).
 *                           If 0, pAdx is ignored.
 *                         - CHS_LEAVEPOS: leave current position at the
 *                           start of the string.
 *                         - CHS_CLIP: clip string to rectangle.
 *                         - CHS_START_XY: start position of the string.
 *                           When set, the handling routine must draw the
 *                           string from the position indicated by pptlStart.
 *                           If this flag is not set, the current position
 *                           is used.
 *                         - CHS_ATTR_INFO: attributes to be used. When this
 *                           flag is set, pAttrs indicates the foreground and
 *                           background colors. Current attributes are
 *                           unchanged. If the flag is not set, the string
 *                           is drawn using the current character attributes.
 *                         - CHS_UNDERSCORE: underscore the characters.
 *                         - CHS_STRIKEOUT: overstrike the characters.
 * @param [in] cChars      Number of characters in the string.
 * @param [in] pchString   Pointer to the character string.
 * @param [in] pAdx        Pointer to an array of LONG integers, one element
 *                         for each character in the string. When CHS_VECTOR
 *                         is set, this array is used to set the spacing
 *                         between characters. Each element is the distance
 *                         in world coordinates from the bottom-left corner
 *                         of the corresponding character in the string to
 *                         the bottom-left corner of the next. The distance
 *                         is measured along the baseline for left-to-right
 *                         and right-to-left character directions, and along
 *                         the shear line for top-to-bottom and bottom-to-top
 *                         character directions. The final element is used to
 *                         reposition the current position, when necessary.
 * @param [in] pAttrs      Pointer to a CSP_INFO structure. This structure
 *                         contains the attributes to be used to draw the
 *                         string when the CHS_ATTR_INFO flag is set. These
 *                         do not alter the current character attributes.
 *                         The CSP_INFO structure is defined as:
 *                         - cSize:     number of bytes in structure
 *                         - lColor:    use foreground color
 *                         - lBackColor: use background color
 * @param [in] pInstance   Pointer to instance data.
 * @param [in] lFunction   High-order WORD=flags; low-order
 *                         WORD=NGreCharStringPos.
 *
 * @return Integer (cHits) indicating correlation/error status.
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit (returned by display
 *                   drivers when the correlate flag is ON, and a hit is
 *                   detected).
 * @retval GPI_ERROR Error.
 *
 * @par EDM2 Reference
 * GreCharStringPos (https://www.edm2.com/index.php/GreCharStringPos):
 * "GreCharStringPos draws a character string. The string can be drawn from
 * the current (X,Y) position or from a position specified."
 * "This function must be supported by the presentation driver."
 * "The handling routine must provide full support for drawing characters
 * from an image font in CM_MODE1 when the character direction is
 * CHDIRN_LEFTRIGHT."
 * "For outline characters or characters in any other mode or direction,
 * the handling routine can dispatch the call to the graphics engine at
 * the address given for this call in the default dispatch table."
 * "GreCharStringPos is called by the function GpiCharStringAt."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |   None    |  Must  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_BASE_ERROR
 * - PMERR_COORDINATE_OVERFLOW
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_EXCEEDS_MAX_SEG_LENGTH
 * - PMERR_FONT_AND_MODE_MISMATCH
 * - PMERR_HDC_BUSY
 * - PMERR_HRGN_BUSY
 * - PMERR_HUGE_FONTS_NOT_SUPPORTED
 * - PMERR_INSUFFICIENT_MEMORY
 * - PMERR_INV_HDC
 * - PMERR_INV_IN_AREA
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_PATH_LIMIT_EXCEEDED
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
GRE_DRV_10(GreCharStringPos)


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
GRE_STUB_7 (GreQueryCharOutline)

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
GRE_STUB_8 (GreQueryCharMetricsTable)

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
GRE_STUB_8 (GreRealizeString)

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
GRE_STUB_7 (GreQueryTabbedTextExtent)

GRE_STUB_10(GreTabbedCharStringAt)
