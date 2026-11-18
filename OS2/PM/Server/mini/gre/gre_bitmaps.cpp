/*!
 * @brief Dispatch for GreDrawBits — draw a rectangle of bits.
 *
 * @param [in] hdc          Device context handle.
 * @param [in] pBitmap      Pointer to the pel data of the bitmap. This data
 *                          is stored in the order that the coordinates appear
 *                          on a display screen: the pel in the lower-left
 *                          corner is the first in the bitmap. Pels are
 *                          scanned to the right, and upward, from that
 *                          position. The bits of the first pel are stored
 *                          beginning with the most significant bits of the
 *                          first byte. The data for pels in each scan line
 *                          is packed together tightly. However, all scan
 *                          lines are padded at the end so that each one
 *                          begins on a ULONG boundary. That is, three bytes
 *                          of pel data will hold one 24-bit pel, three 8-bit
 *                          pels, six 4-bit pels, or twenty-four 1-bit pels.
 *                          If those three bytes are the only pel data for
 *                          that scan line, one more byte of zeros would be
 *                          required to pad the line to a ULONG boundary.
 * @param [in] pInfo        Pointer to either a BITMAPINFO or a BITMAPINFO2
 *                          structure defining the new bitmap.
 * @param [in] cPoints      Number of (X, Y) pairs in paptlPoint. This count
 *                          must be set to 4. The operation is determined by
 *                          comparing the sizes of the two rectangles:
 *                          - Target < Source: compress the source rectangle
 *                            into the target rectangle. The flOptions flags
 *                            determine how eliminated rows and columns should
 *                            be handled.
 *                          - Target = Source: copy between equal rectangles.
 *                          - Target > Source: stretch the source rectangle
 *                            into the target rectangle.
 * @param [in] paptlPoint   Pointer to an array of (X, Y) coordinate pairs
 *                          defining the target and source rectangles. The
 *                          coordinates define the bottom-left and top-right
 *                          corners of the target and source rectangles:
 *                          (xTgtBL, yTgtBL), (xTgtTR, yTgtTR),
 *                          (xSrcBL, ySrcBL), (xSrcTR, ySrcTR).
 *                          When BBO_TARGWORLD is set, the target rectangle
 *                          is inclusive at all boundaries. The source is
 *                          noninclusive. When BBO_TARGWORLD is not set, the
 *                          rectangles are noninclusive.
 * @param [in] lRop         Raster operation code. The low-order byte
 *                          represents a mix value in the range hex 00 -
 *                          hex FF. In addition to the raster operation
 *                          values defined at the API, the presentation driver
 *                          must support ROP_GRAY (hex 000080CA). This value
 *                          is used to shade text for menu items that are not
 *                          currently selectable.
 * @param [in] flOptions    Option flags specifying treatment of eliminated
 *                          lines and columns:
 *                          - BBO_OR:          stretch/compress, ORing eliminated rows/columns
 *                          - BBO_AND:         stretch/compress, ANDing eliminated rows/columns
 *                          - BBO_IGNORE:      stretch/compress, ignoring eliminated rows/columns
 *                          - BBO_TARGWORLD:   target rectangle in world coordinates
 *                          - BBO_COLORS:      source color map as indexes into destination
 *                          - BBO_PAL_COLORS:  color values are palette indexes
 *                          - BBO_NO_COLOR_INFO: no color information with source
 *                          - BBO_DITHER:      enhanced stretching algorithm
 *                          - BLTMODE_SRC_BITMAP: hdcSrc is a bitmap handle
 *                          - BLTMODE_ATTRS_PRES: pBattrs parameter is present
 * @param [in] pInstance    Pointer to instance data.
 * @param [in] lFunction    High-order WORD=flags; low-order
 *                          WORD=NGreDrawBits.
 *
 * @return Integer (cHits) indicating correlation/error status.
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit (returned by the display
 *                   driver when the correlation flag is ON, and a hit is
 *                   detected).
 * @retval GPI_ERROR Error.
 *
 * @par EDM2 Reference
 * GreDrawBits (https://www.edm2.com/index.php/GreDrawBits):
 * "GreDrawBits draws a rectangle of bits."
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
 * - PMERR_HDC_BUSY
 * - PMERR_INCORRECT_DC_TYPE
 * - PMERR_INV_BITBLT_MIX
 * - PMERR_INV_BITBLT_STYLE
 * - PMERR_INV_COORDINATE
 * - PMERR_INV_HDC
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_INV_RECT
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 *
 */
GRE_DRV_9 (GreDrawBits)


/*!
 * @brief Dispatch for GreBitblt — perform a bit-block transfer.
 *
 * @param [in] hdc         Device context handle (destination DC).
 * @param [in] hdcSrc      Handle to the source DC or bitmap. When lRop does
 *                         not require a source, hdcSrc is passed as NULL.
 *                         The handling routine then copies the current
 *                         pattern to the currently selected bitmap or device.
 * @param [in] cPoints     Number of (X,Y) pairs in paptlPoint. The count
 *                         can be 2, 3, or 4:
 *                         - cPoints=2: a raster operation (as determined by
 *                           lRop) on the destination rectangle.
 *                         - cPoints=3: a copy between two rectangles of the
 *                           same size. Only the bottom-left corner is given
 *                           for the source rectangle.
 *                         - cPoints=4: an operation is determined by
 *                           comparing the sizes of the two rectangles:
 *                           * Target < Source: compress the source rectangle
 *                             into the target rectangle.
 *                           * Target = Source: copy between equal rectangles.
 *                           * Target > Source: stretch the source rectangle
 *                             into the target rectangle.
 * @param [in] paptlPoint  Pointer to an array of (X,Y) coordinate pairs
 *                         that define the target and source rectangles. The
 *                         coordinates define the bottom-left and top-right
 *                         corners of the target and source rectangles:
 *                         (xTgtBL, yTgtBL), (xTgtTR, yTgtTR),
 *                         (xSrcBL, ySrcBL), (xSrcTR, ySrcTR).
 *                         When BBO_TARGWORLD is not set, the rectangles are
 *                         noninclusive. When BBO_TARGWORLD is set, the
 *                         target rectangle is inclusive at all boundaries.
 *                         The source is noninclusive.
 * @param [in] lRop        Raster operation code. The low-order byte
 *                         represents a mix value in the range hex 00-hex FF.
 *                         In addition to the ROP values defined at the API,
 *                         the presentation driver must support ROP_GRAY
 *                         (000080CAH). This value is used to shade the text
 *                         for menu items that are not currently selectable.
 * @param [in] flOptions   Specifies treatment of eliminated lines and
 *                         columns when compression is done. Option flag
 *                         values are:
 *                         - BBO_OR:            stretch/compress, ORing eliminated rows/columns
 *                         - BBO_AND:           stretch/compress, ANDing eliminated rows/columns
 *                         - BBO_IGNORE:        stretch/compress, ignoring eliminated rows/columns
 *                         - BBO_TARGWORLD:     target rectangle in world coordinates
 *                         - BBO_NO_COLOR_INFO: no color information with source
 *                         - BLTMODE_SRC_BITMAP: hdcSrc is a bitmap handle
 *                         - BLTMODE_ATTRS_PRES: pBattrs parameter is present
 * @param [in] pBattrs     Pointer to a BITBLTATTRS structure containing the
 *                         attributes:
 *                         - cSize:    size of this structure
 *                         - lColor:   foreground color of source
 *                         - lBackColor: background color of source
 *                         The color values are used in conversions between
 *                         monochrome and color data.
 * @param [in] pInstance   Pointer to instance data.
 * @param [in] lFunction   High-order WORD=flags; low-order
 *                         WORD=NGreBitblt.
 *
 * @return Integer (cHits) indicating correlation/error status.
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit (returned by display
 *                   drivers when the correlate flag is ON, and a hit is
 *                   detected).
 * @retval GPI_ERROR Error.
 *
 * @par EDM2 Reference
 * GreBitblt (https://www.edm2.com/index.php/GreBitblt):
 * "GreBitBlt modifies bit-map data at a target rectangle in the current
 * device context (DC). The modification can copy a rectangle of data from
 * a specified source DC to the target or perform a raster operation on
 * the target."
 * "GreBitblt is called by the function GpiBitBlt and is used to modify
 * bit-map data within a target rectangle of the current device context."
 *
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    No     |  Must  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_BASE_ERROR
 * - PMERR_BITMAP_IS_SELECTED
 * - PMERR_BITMAP_NOT_SELECTED
 * - PMERR_COORDINATE_OVERFLOW
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_EXCEEDS_MAX_SEG_LENGTH
 * - PMERR_HBITMAP_BUSY
 * - PMERR_HDC_BUSY
 * - PMERR_HUGE_FONTS_NOT_SUPPORTED
 * - PMERR_INCOMPATIBLE_BITMAP
 * - PMERR_INCORRECT_DC_TYPE
 * - PMERR_INSUFFICIENT_MEMORY
 * - PMERR_INV_BACKGROUND_COL_ATTR
 * - PMERR_INV_BITMAP_DIMENSION
 * - PMERR_INV_BITBLT_MIX
 * - PMERR_INV_BITBLT_STYLE
 * - PMERR_INV_COLOR_ATTR
 * - PMERR_INV_COLOR_DATA
 * - PMERR_INV_COLOR_FORMAT
 * - PMERR_INV_COLOR_INDEX
 * - PMERR_INV_COLOR_OPTIONS
 * - PMERR_INV_COLOR_START_INDEX
 * - PMERR_INV_COORD_SPACE
 * - PMERR_INV_COORDINATE
 * - PMERR_INV_DC_DATA
 * - PMERR_INV_DC_TYPE
 * - PMERR_INV_DRIVER_NAME
 * - PMERR_INV_HBITMAP
 * - PMERR_INV_HDC
 * - PMERR_INV_ID
 * - PMERR_INV_IN_AREA
 * - PMERR_INV_IN_PATH
 * - PMERR_INV_INFO_TABLE
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_INV_PATTERN_SET_ATTR
 * - PMERR_INV_PATTERN_SET_FONT
 * - PMERR_INV_PICK_APERTURE_POSN
 * - PMERR_INV_SCAN_START
 * - PMERR_INV_USAGE_PARM
 * - PMERR_UNSUPPORTED_ATTR_VALUE
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 * @par Remarks
 * "The device contexts can be memory DCs with bit maps selected or DCs
 * belonging to devices that support raster operations."
 * "When copying bits from a color bit map to a monochrome bit map or
 * device, only those pels that are in the source background color are
 * copied to the target as background color. All other pels are copied to
 * the target as foreground color."
 * "When the target and source rectangles are in the same DC, no
 * information is lost from the source if the rectangles overlap."
 * "When the target is expressed in world coordinates (that is, the
 * BBO_TARGWORLD flag is set in flOptions), the coordinates must be
 * transformed device coordinates. The bits are transferred to an upright
 * rectangle in device space, regardless of any rotational elements that
 * might have been present in the transforms."
 * "The attribute structure identified by the pBattrs parameter defines the
 * bit-map foreground and background colors. If pBattrs is NULL, the
 * handling routine uses the current foreground and background colors."
 * "When the mix specified by lRop requires both source and pattern, a
 * 3-way operation is performed by using the current pattern in the target
 * device context. If pattern mixing is not required, a 2-way operation is
 * done."
 *
 */
GRE_DRV_9 (GreBitblt)

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
GRE_DRV_8 (GreScanLR)

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
GRE_DRV_5 (GreFloodFill)

/*!
 * @brief Dispatch for GreSaveScreenBits — save a rectangle of screen bits.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] prclRect   Pointer to a screen rectangle defined in screen
 *                        coordinates.
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order
 *                        WORD=NGreSaveScreenBits.
 *
 * @return Handle to the saved bits (hsbBits), or 0 to indicate that the
 *         bits were not saved or that an error occurred.
 * @retval hsbBits  Handle to the saved bits.
 * @retval 0        Bits were not saved or error occurred.
 *
 * @par EDM2 Reference
 * GreSaveScreenBits (https://www.edm2.com/index.php/GreSaveScreenBits):
 * "GreSaveScreenBits saves a rectangle of screen bits."
 * "It is permissible to implement this function by returning 0 to indicate
 * that the bits were not saved and, therefore, must be saved by the
 * calling routine."
 * "This function lets the user-interface routines improve the performance
 * of dialog boxes."
 *
 * @par Support
 * | Simulated | Hooked  | Chained |
 * |-----------|---------|---------|
 * | Yes(otr)  |Must(dsp)|   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_INV_HDC
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
GRE_DRV_4 (GreSaveScreenBits)

/*!
 * @brief Dispatch for GreRestoreScreenBits — restore saved screen bits.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] hsbBits    Handle to the saved screen bits to be restored.
 * @param [in] prclRect   Pointer to a screen rectangle defined in screen
 *                        coordinates.
 * @param [in] flOptions  Option flags:
 *                        - RSB_FREE:    free the saved bits handle
 *                        - RSB_RESTORE: restore the bits to the screen
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order
 *                        WORD=NGreRestoreScreenBits.
 *
 * @return BOOLEAN success indicator.
 * @retval TRUE   Successful.
 * @retval FALSE  Error.
 *
 * @par EDM2 Reference
 * GreRestoreScreenBits (https://www.edm2.com/index.php/GreRestoreScreenBits):
 * "GreRestoreScreenBits restores a rectangle of bits to a screen rectangle
 * and can also free the handle of the saved bits."
 *
 *
 * @par Support
 * | Simulated | Hooked  | Chained |
 * |-----------|---------|---------|
 * | Yes(oth)  |Must(dsp)|   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_BASE_ERROR
 * - PMERR_BITMAP_IS_SELECTED
 * - PMERR_BITMAP_NOT_SELECTED
 * - PMERR_COORDINATE_OVERFLOW
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_EXCEEDS_MAX_SEG_LENGTH
 * - PMERR_HBITMAP_BUSY
 * - PMERR_HDC_BUSY
 * - PMERR_HUGE_FONTS_NOT_SUPPORTED
 * - PMERR_INCOMPATIBLE_BITMAP
 * - PMERR_INCORRECT_DC_TYPE
 * - PMERR_INSUFFICIENT_MEMORY
 * - PMERR_INV_BACKGROUND_COL_ATTR
 * - PMERR_INV_BACKGROUND_MIX_ATTR
 * - PMERR_INV_BITBLT_MIX
 * - PMERR_INV_BITBLT_STYLE
 * - PMERR_INV_BITMAP_DIMENSION
 * - PMERR_INV_CHAR_DIRECTION_ATTR
 * - PMERR_INV_CHAR_MODE_ATTR
 * - PMERR_INV_CHAR_SET_ATTR
 * - PMERR_INV_CHAR_SHEAR_ATTR
 * - PMERR_INV_CODEPAGE
 * - PMERR_INV_COLOR_ATTR
 * - PMERR_INV_COLOR_DATA
 * - PMERR_INV_COLOR_FORMAT
 * - PMERR_INV_COLOR_INDEX
 * - PMERR_INV_COLOR_OPTIONS
 * - PMERR_INV_COLOR_START_INDEX
 * - PMERR_INV_COORD_SPACE
 * - PMERR_INV_COORDINATE
 * - PMERR_INV_DC_DATA
 * - PMERR_INV_DC_TYPE
 * - PMERR_INV_DRIVER_NAME
 * - PMERR_INV_GEOM_LINE_WIDTH_ATTR
 * - PMERR_INV_HBITMAP
 * - PMERR_INV_HDC
 * - PMERR_INV_HRGN
 * - PMERR_INV_ID
 * - PMERR_INV_IN_AREA
 * - PMERR_INV_IN_PATH
 * - PMERR_INV_INFO_TABLE
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_INV_LINE_END_ATTR
 * - PMERR_INV_LINE_JOIN_ATTR
 * - PMERR_INV_LINE_TYPE_ATTR
 * - PMERR_INV_LINE_WIDTH_ATTR
 * - PMERR_INV_MARKER_SET_ATTR
 * - PMERR_INV_MARKER_SYMBOL_ATTR
 * - PMERR_INV_MIX_ATTR
 * - PMERR_INV_PATTERN_REF_PT_ATTR
 * - PMERR_INV_PATTERN_SET_ATTR
 * - PMERR_INV_PATTERN_SET_FONT
 * - PMERR_INV_PICK_APERTURE_POSN
 * - PMERR_INV_PRIMITIVE_TYPE
 * - PMERR_INV_RECT
 * - PMERR_INV_REGION_CONTROL
 * - PMERR_INV_SCAN_START
 * - PMERR_INV_SETID
 * - PMERR_INV_USAGE_PARM
 * - PMERR_REALIZE_NOT_SUPPORTED
 * - PMERR_UNSUPPORTED_ATTR
 * - PMERR_UNSUPPORTED_ATTR_VALUE
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 * @par Remarks
 * "Clipping is done on the restored bits, as necessary."
 *
 */
GRE_DRV_6 (GreRestoreScreenBits)

/*!
 * @brief Dispatch for GreDeviceSetCursor — set the cursor bitmap.
 *
 * @param [in] hdc          Device context handle.
 * @param [in] pptlHotSpot  Pointer to a POINTL structure giving the hot
 *                          spot coordinates within the cursor bitmap:
 *                          - x: X-position of the hotspot within the cursor
 *                            bitmap
 *                          - y: Y-position of the hotspot within the cursor
 *                            bitmap
 * @param [in] hbm          Bitmap handle used for the cursor image. If hbm
 *                          is NULL, the cursor has no shape and its image
 *                          is removed from the display screen.
 * @param [in] pInstance    Pointer to instance data.
 * @param [in] lFunction    High-order WORD=flags; low-order
 *                          WORD=NGreDeviceSetCursor.
 *
 * @return BOOLEAN success indicator.
 * @retval TRUE   Successful.
 * @retval FALSE  Error.
 *
 * @par EDM2 Reference
 * GreDeviceSetCursor (https://www.edm2.com/index.php/GreDeviceSetCursor):
 * "GreDeviceSetCursor sets the cursor bit map that defines the cursor
 * shape."
 * "GreDeviceSetCursor is subject to all clipping."
 * "The handling routine takes the previous cursor bit map and replaces it
 * with the one indicated by hbm."
 *
 * @par Support
 * | Simulated | Hooked  | Chained |
 * |-----------|---------|---------|
 * | Yes(oth)  |Must(dsp)|  Can    |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_INV_COORDINATE
 * - PMERR_INV_CURSOR_BITMAP
 * - PMERR_INV_HDC
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
GRE_DRV_5 (GreDeviceSetCursor)
