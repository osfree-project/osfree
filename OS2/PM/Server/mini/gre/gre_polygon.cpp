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

