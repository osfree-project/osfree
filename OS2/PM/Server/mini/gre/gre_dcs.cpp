/*!
 * @brief GreOpenDC — create an output device context (DC).
 *
 * @param [in] hdc        Device context handle. When ulType is OD_MEMORY,
 *                        the new DC is compatible with bitmaps that are
 *                        used with this device context. If this is NULL,
 *                        compatibility with the display is assumed.
 * @param [in] ulType     Type of DC being opened:
 *                        - OD_QUEUED: DC for a hardcopy device for which
 *                          output is to be queued by the spooler
 *                        - OD_DIRECT: DC for a device for which output is
 *                          not to be queued by the spooler
 *                        - OD_INFO: similar to OD_DIRECT except that it is
 *                          only used to retrieve information such as font
 *                          metrics. Drawing can be done to a presentation
 *                          space associated with such a DC but no output
 *                          medium is updated
 *                        - OD_MEMORY: the new DC is a memory DC used to
 *                          contain a bitmap. The hdc parameter identifies
 *                          the device with which the bitmap is to be
 *                          compatible
 * @param [in] pszToken   This parameter is ignored by the graphics engine.
 * @param [in] cData      Number of elements in the data structure.
 * @param [in] pszData    Pointer to a DEVOPENDATA structure:
 *                        - pszLogAddress: pointer to logical address
 *                          (e.g., LPT1Q)
 *                        - pszDriverName: pointer to presentation driver
 *                          name (e.g., LASERJET)
 *                        - pdriv: pointer to a DRIVDATA structure
 *                        - pszDataType: pointer to data type of queued file
 *                        - pszComment: pointer to description of the file
 *                        - pszQueueProcName: pointer to queue processor name
 *                        - pszQueueProcParams: pointer to queue processor
 *                          parameters
 *                        - pszSpoolerParams: pointer to spooler parameters
 *                        - pszNetworkParams: pointer to networking parameters
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order
 *                        WORD=NGreOpenDC.
 *
 * @return Handle of the new device context (HDC), or DEV_ERROR if an error
 *         occurs.
 * @retval HDC        Handle to the newly created device context.
 * @retval DEV_ERROR  Error occurred during DC creation.
 *
 * @par EDM2 Reference
 * GreOpenDC (https://www.edm2.com/index.php/GreOpenDC):
 * "GreOpenDC creates an output device context (DC)."
 * "The new device context inherits the current code page of the process
 * that created it."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    No     |   No   |   No    |
 *
 * @par Possible Errors
 * When an error is detected, the graphics engine calls WinSetErrorInfo to
 * post the condition. Reasons for failure of this function include:
 * - PMERR_BASE_ERROR
 * - PMERR_BITMAP_IS_SELECTED
 * - PMERR_COORDINATE_OVERFLOW
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_EXCEEDS_MAX_SEG_LENGTH
 * - PMERR_HBITMAP_BUSY
 * - PMERR_HDC_BUSY
 * - PMERR_HUGE_FONTS_NOT_SUPPORTED
 * - PMERR_INSUFFICIENT_MEMORY
 * - PMERR_INV_BACKGROUND_COL_ATTR
 * - PMERR_INV_BACKGROUND_MIX_ATTR
 * - PMERR_INV_BITMAP_DIMENSION
 * - PMERR_INV_CHAR_DIRECTION_ATTR
 * - PMERR_INV_CHAR_MODE_ATTR
 * - PMERR_INV_CODEPAGE
 * - PMERR_INV_COLOR_ATTR
 * - PMERR_INV_COORD_SPACE
 * - PMERR_INV_COORDINATE
 * - PMERR_INV_DC_DATA
 * - PMERR_INV_DC_TYPE
 * - PMERR_INV_DRIVER_NAME
 * - PMERR_INV_HBITMAP
 * - PMERR_INV_HDC
 * - PMERR_INV_HRGN
 * - PMERR_INV_ID
 * - PMERR_INV_IN_AREA
 * - PMERR_INV_IN_PATH
 * - PMERR_INV_INFO_TABLE
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_INV_LINE_TYPE_ATTR
 * - PMERR_INV_MIX_ATTR
 * - PMERR_INV_PATTERN_REF_PT_ATTR
 * - PMERR_INV_PATTERN_SET_ATTR
 * - PMERR_INV_PATTERN_SET_FONT
 * - PMERR_INV_RECT
 * - PMERR_INV_REGION_CONTROL
 * - PMERR_INV_SCAN_START
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 * @par Remarks
 * "Subsequent calls to DosSetCp do not alter the code page of an existing
 * DC."
 * "Default VIO and KBD code pages are always in the last code page set by
 * any application process."
 *
 */
static HDC __cdecl GreOpenDC(HDC hdcComp, ULONG lType,
                                    PSZ   pszToken, LONG lCount,
                                    PDEVOPENDATA pdop, PVOID pInstance,
                                    ULONG fnCode)
{
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE] GreOpenDC(ulType=0x%lx, fn=0x%lx)",
                             (unsigned long)lType, (unsigned long)fnCode);
    ULONG newHdc;

    (void)hdcComp; (void)pszToken; (void)lCount; (void)pdop;
    (void)pInstance;

    if (!g_bDriverLoaded && !LoadGre2Sdl()) {
        F_DbgCtxExitf(ctx, "GPI_ERROR (driver load failed)");
        return DEV_ERROR;
    }
    newHdc = NewDc(0, lType);
    if (newHdc == (ULONG)-1) {
        F_DbgCtxExitf(ctx, "GPI_ERROR (NewDc failed)");
        return DEV_ERROR;
    }
    F_DbgCtxExitf(ctx, "hdc=%lu", (unsigned long)newHdc);
    return newHdc;
}

/*!
 * @brief GreCloseDC - closes a device context.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order WORD=NGreCloseDC.
 *
 * @return Boolean success indicator.
 * @retval TRUE   Successful.
 * @retval FALSE  Error.
 *
 * @par EDM2 Reference
 * GreCloseDC (https://www.edm2.com/index.php/GreCloseDC):
 * "GreCloseDC closes a device context."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    No     |   No   |   No    |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_BASE_ERROR
 * - PMERR_BITMAP_IS_SELECTED
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_EXCEEDS_MAX_SEG_LENGTH
 * - PMERR_HBITMAP_BUSY
 * - PMERR_HDC_BUSY
 * - PMERR_INSUFFICIENT_MEMORY
 * - PMERR_INV_BITMAP_DIMENSION
 * - PMERR_INV_CODEPAGE
 * - PMERR_INV_COORDINATE
 * - PMERR_INV_DC_TYPE
 * - PMERR_INV_HBITMAP
 * - PMERR_INV_HDC
 * - PMERR_INV_HRGN
 * - PMERR_INV_ID
 * - PMERR_INV_IN_AREA
 * - PMERR_INV_IN_PATH
 * - PMERR_INV_INFO_TABLE
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_INV_RECT
 * - PMERR_INV_REGION_CONTROL
 * - PMERR_INV_SCAN_START
 *
 * Refer to the "Error Explanations" section of the Presentation Manager
 * Programming Reference for further explanation.
 *
 * @par Remarks
 * "If the device context is a memory DC that has a bit map selected, the
 * engine deselects the bit map before destroying the DC."
 * "This function deletes any visible regions and clip regions selected into
 * the DC."
 *
 */
static BOOL __cdecl GreCloseDC(HDC hdc, PVOID pInstance, ULONG fnCode)
{
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE] GreCloseDC(hdc=%lu, fn=0x%lx)",
                             (unsigned long)hdc, fnCode);
    BOOL ok;
    (void)pInstance;
    ok = DestroyDc(hdc);
    F_DbgCtxExitf(ctx, "%s", (ok ? "TRUE" : "FALSE"));
    return ok;
}
