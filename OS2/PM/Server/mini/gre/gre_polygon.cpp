/*!
 * @brief GrePolygonSet — draw a set of closed polygons.
 * @param [in] hdc        Device context handle.
 * @param [in] flModel    POLYGON_INCL / POLYGON_EXCL.
 * @param [in] flOptions  POLYGON_ALTERNATE / POLYGON_WINDING,
 *                        POLYGON_BOUNDARY / POLYGON_NOBOUNDARY.
 * @param [in] paPolygon  Pointer to an array of POLYGON structures.
 * @param [in] cPolygons  Number of POLYGON structures.
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order WORD=NGrePolygonSet.
 * @return cHits code.
 * @retval GPI_OK    Drawn.
 * @retval GPI_HITS  Correlation hit.
 * @retval GPI_ERROR Invalid DC or NULL array with cPolygons != 0.
 *
 * @par EDM2 Reference
 * GrePolygonSet (https://www.edm2.com/index.php/GrePolygonSet):
 * "GrePolygonSet draws a set of closed polygons."
 * "The polygons are filled using the current AREABUNDLE structure values.
 * For the first polygon, the current position is the first point. For all
 * subsequent polygons, all points that define the polygon are given
 * explicitly. The polygons are automatically closed, if necessary, by
 * drawing a line from the last vertex to the first. Notice that polygons
 * can overlap, if needed."
 * "GrePolygonSet is not valid when the COM_AREA or COM_PATH flag is set."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |  Can   |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_BASE_ERROR
 * - PMERR_COORDINATE_OVERFLOW
 * - PMERR_INV_IN_AREA
 * - PMERR_INV_IN_PATH
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 */
static LONG __cdecl GrePolygonSet(HDC hdc, ULONG flModel,
                                  ULONG flOptions, PPOLYGON paPolygon,
                                  ULONG cPolygons, PVOID pInstance,
                                  ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)GPI_ERROR;
    return (ULONG)pDc->GetLine().PolygonSet((LONG)flModel, flOptions,
                                             (const POLYGON *)paPolygon,
                                             (LONG)cPolygons);
}


/*!
 * @brief Dispatch for GreDrawRLE — draw a run-length-encoded shape.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] pRLEHDR    Pointer to run-length-encoding data header (RLEHDR) structure.
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order WORD=NGreDrawRLE.
 *
 * @return Integer (cHits) indicating correlation/error status.
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit (returned by display
 *                   drivers when the correlation flag is ON, and a hit
 *                   is detected).
 * @retval GPI_ERROR Error.
 *
 * @par EDM2 Reference
 * GreDrawRLE (https://www.edm2.com/index.php/GreDrawRLE):
 * "GreDrawRLE draws a run-length-encoded shape. If the call is passed
 * back to the graphics engine, it calls the GrePolyScanline entry point."
 *
 * @par Remarks
 * "GreDrawRLE is called by the area fill code in the graphics engine.
 * The calling function uses the driver's
 * CAPS_ADDITIONAL_GRAPHICS/CAPS_CLIP_FILLS field, which is queried
 * during the first instance of DevOpenDC. If it is set, the data
 * delivered to GreDrawRLE has already been clipped and the COM_PRECLIP
 * flag will be set in the flag's WORD of the function number. All
 * coordinates are passed as screen coordinates."
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
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 */
static ULONG __cdecl GreDrawRLE(HDC hdc, PRLEHDR pRLEHDR,
                                    PVOID pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)GPI_ERROR;
    return (ULONG)pDc->GetLine().DrawRLE((const RLEHDR *)pRLEHDR);
}


/**
 * @brief Draw a device-dependent polygon set.
 *
 * @param[in] a
 * @param[in] b
 *
 * @return 
 *
 * @retval 
 *
 * @note Not found any information yet and not found any driver used it?
 */
GRE_DRV_4 (GreDevicePolygonSet)
#define GreDevicePolygonSet32 GreDevicePolygonSet

/**
 * @brief Draw a 3-D polygon.
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
GRE_DRV_5 (GrePolygon3d)


