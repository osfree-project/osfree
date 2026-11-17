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
static LONG __cdecl GrePolyLine(HDC hdc, PPOINTL paptlPoint, LONG cPoints, PVOID pInstance, ULONG lFunction)
{
  //@todo implement described logic
}

