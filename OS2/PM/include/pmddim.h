#ifdef __cplusplus
      extern "C" {
#endif

#define DDIM_INCLUDED

ULONG APIENTRY Gre32Entry2(ULONG,ULONG);
ULONG APIENTRY Gre32Entry3(ULONG,ULONG,ULONG);
ULONG APIENTRY Gre32Entry4(ULONG,ULONG,ULONG,ULONG);
ULONG APIENTRY Gre32Entry5(ULONG,ULONG,ULONG,ULONG,ULONG);
ULONG APIENTRY Gre32Entry6(ULONG,ULONG,ULONG,ULONG,ULONG,ULONG);
ULONG APIENTRY Gre32Entry7(ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG);
ULONG APIENTRY Gre32Entry8(ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG);
ULONG APIENTRY Gre32Entry9(ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG);
ULONG APIENTRY Gre32Entry10(ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG);

#ifdef INCL_GRE_LINES
   /**
    * @brief Draw a series of disjoint lines.
    *
    * @param[in] hdc        Device context handle.
    * @param[in] paptlPoint Pointer to an array of POINTL structures that
    *                       define the end points of the lines.
    * @param[in] cPoints    Number of (X,Y) pairs in the point array.
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
    */
   #define NGreDisjointLines 0x00004016L
   #define GreDisjointLines(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),0L,NGreDisjointLines)

   /**
    * @brief Retrieve the current (X,Y) position.
    *
    * @param[in]  hdc          Device context handle.
    * @param[out] pptlPosition Pointer to a POINTL structure that receives the
    *                          current position.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreGetCurrentPosition 0x00004017L
   #define GreGetCurrentPosition(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreGetCurrentPosition)

   /**
    * @brief Set the current (X,Y) position.
    *
    * @param[in] hdc          Device context handle.
    * @param[in] pptlPosition Pointer to a POINTL structure containing the new
    *                         current position.
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
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_COORD_SPACE
    *       - PMERR_INV_HDC
    */
   #define NGreSetCurrentPosition 0x00004018L
   #define GreSetCurrentPosition(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreSetCurrentPosition)

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
    */
   #define NGrePolyLine 0x00004019L
   #define GrePolyLine(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),0L,NGrePolyLine)

   /**
    * @brief Draw a sequence of lines from a linked structure.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] pPath  Pointer to a PATH structure containing the line data.
    * @param[in] pCurve Pointer to a CURVE structure.
    * @param[in] cLines Number of lines to draw.
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
    *       - PMERR_BITMAP_NOT_SELECTED
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_INV_IN_PATH
    */
   #define NGreDrawLinesInPath 0x0000401AL
   #define GreDrawLinesInPath(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PPATH)(b),(ULONG)(PCURVE)(c),(ULONG)(LONG)(d),0L,NGreDrawLinesInPath)

   /**
    * @brief Draw a series of short lines.
    *
    * @param[in] hdc Device context handle.
    * @param[in] psl Pointer to the first SHORTLINE structure in the linked list.
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
    *       - PMERR_BITMAP_NOT_SELECTED
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_COLOR_DATA
    */
   #define NGrePolyShortLine 0x0000401BL
   #define GrePolyShortLine(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PSHORTLINE)(b),0L,NGrePolyShortLine)
#endif

#ifdef INCL_GRE_POLYGON
   /**
    * @brief Draw a set of closed polygons.
    *
    * @param[in] hdc       Device context handle.
    * @param[in] flModel   Model flags.
    * @param[in] flOptions Option flags (e.g. POLYGON_INCL).
    * @param[in] paPolygon Pointer to an array of POLYGON structures.
    * @param[in] cPolygons Number of POLYGON structures in the array.
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
    *       - PMERR_INV_IN_AREA
    *       - PMERR_INV_IN_PATH
    */
   #define NGrePolygonSet 0x00004056L
   #define GrePolygonSet(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(b),(ULONG)(c),(ULONG)(PPOLYGON)(d),(ULONG)(e),0L,NGrePolygonSet)

   /**
    * @brief Draw a run-length-encoded (RLE) bitmap.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] prleh Pointer to an RLEHDR structure describing the RLE data.
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
    *       - PMERR_BITMAP_NOT_SELECTED
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_COLOR_DATA
    */
   #define NGreDrawRLE 0x00004058L
   #define GreDrawRLE(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRLEHDR)(b),0L,NGreDrawRLE)

   /**
    * @brief Draw a device-dependent polygon set.
    *
    * @param[in] hdc       Device context handle.
    * @param[in] pPolyset  Pointer to a POLYSET structure.
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
    *       - PMERR_INV_IN_AREA
    *       - PMERR_INV_IN_PATH
    */
   #define NGreDevicePolygonSet 0x00004059L
   #define GreDevicePolygonSet(a,b) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOLYSET)(b),0L,NGreDevicePolygonSet)

   /**
    * @brief Draw a 3-D polygon.
    *
    * @param[in] hdc         Device context handle.
    * @param[in] pPolysinfo  Pointer to a POLYSINFO structure.
    * @param[in] cOptions    Option flags.
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
    *       - PMERR_INV_IN_AREA
    */
   #define NGrePolygon3d 0x0000405CL
   #define GrePolygon3d(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOLYSINFO)(b),(ULONG)(ULONG)(c),0L,NGrePolygon3d)
#endif

#ifdef INCL_GRE_ARCS
   /**
    * @brief Retrieve the current arc parameters.
    *
    * @param[in]  hdc       Device context handle.
    * @param[out] pArcParms Pointer to an ARCPARAMS structure that receives the
    *                       current arc parameters.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    */
   #define NGreGetArcParameters 0x00004000L
   #define GreGetArcParameters(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PARCPARAMS)(b),0L,NGreGetArcParameters)

   /**
    * @brief Set the current arc parameters.
    *
    * @param[in] hdc       Device context handle.
    * @param[in] pArcParms Pointer to an ARCPARAMS structure containing the new
    *                      arc parameters.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    */
   #define NGreSetArcParameters 0x00004001L
   #define GreSetArcParameters(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PARCPARAMS)(b),0L,NGreSetArcParameters)

   /**
    * @brief Draw an arc through three points.
    *
    * @param[in] hdc        Device context handle.
    * @param[in] paptlPoint Pointer to an array of POINTL structures giving
    *                       the mid- and end-points of the arc.
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
    *       - PMERR_INV_COLOR_DATA
    *       - PMERR_INV_COLOR_INDEX
    *       - PMERR_INV_COORD_SPACE
    *       - PMERR_INV_HDC
    *       - PMERR_INV_IN_AREA
    *       - PMERR_INV_IN_PATH
    *       - PMERR_INV_LENGTH_OR_COUNT
    *       - PMERR_INV_NESTED_FIGURES
    *       - PMERR_INV_PICK_APERTURE_POSN
    *       - PMERR_INV_RECT
    *       - PMERR_PATH_LIMIT_EXCEEDED
    *       - PMERR_PATH_UNKNOWN
    */
   #define NGreArc 0x00004002L
   #define GreArc(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreArc)

   /**
    * @brief Draw a partial arc.
    *
    * @param[in] hdc          Device context handle.
    * @param[in] pptlCenter   Pointer to the centre of the arc.
    * @param[in] fxMultiplier Multiplier for the arc’s axes.
    * @param[in] fxStart      Start angle of the arc.
    * @param[in] fxSweep      Sweep angle of the arc.
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
    *       - PMERR_INV_NESTED_FIGURES
    */
   #define NGrePartialArc 0x00004003L
   #define GrePartialArc(a,b,c,d,e) (INT) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(FIXED)(c),(ULONG)(FIXED)(d),(ULONG)(FIXED)(e),0L,NGrePartialArc)

   /**
    * @brief Draw a full arc (interior only).
    *
    * @param[in] hdc     Device context handle.
    * @param[in] fxAngle Rotation angle of the arc.
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
    *       - PMERR_ALREADY_IN_AREA
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
    */
   #define NGreFullArcInterior 0x00004004L
   #define GreFullArcInterior(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(FIXED)(b),0L,NGreFullArcInterior)

   /**
    * @brief Draw a full arc (boundary only).
    *
    * @param[in] hdc     Device context handle.
    * @param[in] fxAngle Rotation angle of the arc.
    *
    * @return Integer (cHits) indicating correlation/error status.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_HITS  Successful with correlation hit.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the errors
    *       documented for GreFullArcInterior.
    */
   #define NGreFullArcBoundary 0x00004005L
   #define GreFullArcBoundary(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(FIXED)(b),0L,NGreFullArcBoundary)

   /**
    * @brief Draw a full arc (both interior and boundary).
    *
    * @param[in] hdc     Device context handle.
    * @param[in] fxAngle Rotation angle of the arc.
    *
    * @return Integer (cHits) indicating correlation/error status.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_HITS  Successful with correlation hit.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the errors
    *       documented for GreFullArcInterior.
    */
   #define NGreFullArcBoth 0x00004006L
   #define GreFullArcBoth(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(FIXED)(b),0L,NGreFullArcBoth)

   /**
    * @brief Draw a rectangular box (interior only).
    *
    * @param[in] hdc  Device context handle.
    * @param[in] pBox Pointer to a BOXPARAMS structure defining the box.
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
    *       - PMERR_INV_BOX_ROUNDING_PARM
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
    */
   #define NGreBoxInterior 0x00004007L
   #define GreBoxInterior(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PBOXPARAMS)(b),0L,NGreBoxInterior)

   /**
    * @brief Draw a rectangular box (boundary only).
    *
    * @param[in] hdc  Device context handle.
    * @param[in] pBox Pointer to a BOXPARAMS structure defining the box.
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
    *       - PMERR_INV_BOX_ROUNDING_PARM
    *       - PMERR_INV_COLOR_DATA
    *       - PMERR_INV_COLOR_INDEX
    *       - PMERR_INV_COORD_SPACE
    *       - PMERR_INV_HDC
    *       - PMERR_INV_IN_AREA
    *       - PMERR_INV_IN_PATH
    *       - PMERR_INV_LENGTH_OR_COUNT
    *       - PMERR_INV_NESTED_FIGURES
    *       - PMERR_INV_PICK_APERTURE_POSN
    *       - PMERR_INV_RECT
    *       - PMERR_NOT_IN_PATH
    *       - PMERR_PATH_LIMIT_EXCEEDED
    *       - PMERR_PATH_UNKNOWN
    */
   #define NGreBoxBoundary 0x00004008L
   #define GreBoxBoundary(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PBOXPARAMS)(b),0L,NGreBoxBoundary)

   /**
    * @brief Draw a rectangular box (both interior and boundary).
    *
    * @param[in] hdc  Device context handle.
    * @param[in] pBox Pointer to a BOXPARAMS structure defining the box.
    *
    * @return Integer (cHits) indicating correlation/error status.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_HITS  Successful with correlation hit.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the errors
    *       documented for GreBoxBoundary.
    */
   #define NGreBoxBoth 0x00004009L
   #define GreBoxBoth(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PBOXPARAMS)(b),0L,NGreBoxBoth)

   /**
    * @brief Draw a rectangular box with exclusive-OR behaviour.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] pBox     Pointer to a BOXPARAMS structure defining the box.
    * @param[in] cOptions Option flags.
    *
    * @return Integer (cHits) indicating correlation/error status.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_HITS  Successful with correlation hit.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the errors
    *       documented for GreBoxBoundary.
    */
   #define NGreBoxExclusive 0x00004010L
   #define GreBoxExclusive(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PBOXPARAMS)(b),(ULONG)(c),0L,NGreBoxExclusive)

   /**
    * @brief Draw a fillet on a series of connected lines.
    *
    * @param[in] hdc        Device context handle.
    * @param[in] paptlPoint Pointer to an array of POINTL structures that
    *                       define the end points of the lines.
    * @param[in] cPoints    Number of (X,Y) pairs in the point array.
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
    */
   #define NGrePolyFillet 0x0000400AL
   #define GrePolyFillet(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),0L,NGrePolyFillet)

   /**
    * @brief Draw a sharp fillet on a series of connected lines.
    *
    * @param[in] hdc        Device context handle.
    * @param[in] paptlPoint Pointer to an array of POINTL structures.
    * @param[in] cPoints    Number of (X,Y) pairs.
    * @param[in] pSharp     Pointer to a FIXED value that controls the
    *                       sharpness of the fillet.
    *
    * @return Integer (cHits) indicating correlation/error status.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_HITS  Successful with correlation hit.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the errors
    *       documented for GrePolyFillet.
    */
   #define NGrePolyFilletSharp 0x0000400BL
   #define GrePolyFilletSharp(a,b,c,d) (INT) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),(ULONG)(PFIXED)(d),0L,NGrePolyFilletSharp)

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
#endif

#ifdef INCL_GRE_REGIONS
   /**
    * @brief Get the bounding box of a region.
    *
    * @param[in]  hdc   Device context handle.
    * @param[in]  hrgn  Region handle.
    * @param[out] prcl  Pointer to a RECTL structure that receives the box.
    *
    * @return Integer status code.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HRGN_BUSY
    *       - PMERR_INV_HRGN
    */
   #define NGreGetRegionBox 0x0000405DL
   #define GreGetRegionBox(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PRECTL)(c),0L,NGreGetRegionBox)

   /**
    * @brief Get the rectangles of a region.
    *
    * @param[in]  hdc      Device context handle.
    * @param[in]  hrgn     Region handle.
    * @param[in]  prcl     Pointer to a RECTL structure.
    * @param[in]  prgnrc   Pointer to a RGNRECT structure.
    * @param[out] prclOut  Pointer to a RECTL structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HRGN_BUSY
    *       - PMERR_INV_COORDINATE
    */
   #define NGreGetRegionRects 0x0000405EL
   #define GreGetRegionRects(a,b,c,d,e) (BOOL) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PRECTL)(c),(ULONG)(PRGNRECT)(d),(ULONG)(PRECTL)(e),0L,NGreGetRegionRects)

   /**
    * @brief Offset a region.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] hrgn  Region handle.
    * @param[in] pptl  Pointer to a POINTL structure giving the offset.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_HRGN_BUSY
    */
   #define NGreOffsetRegion 0x0000405FL
   #define GreOffsetRegion(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PPOINTL)(c),0L,NGreOffsetRegion)

   /**
    * @brief Test if a point is in a region.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] hrgn  Region handle.
    * @param[in] pptl  Pointer to a POINTL structure.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HRGN_BUSY
    *       - PMERR_INV_COORDINATE
    */
   #define NGrePtInRegion 0x00004060L
   #define GrePtInRegion(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PPOINTL)(c),0L,NGrePtInRegion)

   /**
    * @brief Test if a rectangle is in a region.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] hrgn  Region handle.
    * @param[in] prcl  Pointer to a RECTL structure.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HRGN_BUSY
    *       - PMERR_INV_COORDINATE
    */
   #define NGreRectInRegion 0x00004061L
   #define GreRectInRegion(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PRECTL)(c),0L,NGreRectInRegion)

   /**
    * @brief Create a rectangular region.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] prcl  Pointer to a RECTL structure.
    * @param[in] lMode Creation mode.
    *
    * @return HRGN handle, or RGN_ERROR on error.
    *
    * @retval RGN_ERROR  Error.
    *
    * @note If RGN_ERROR is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_COORDINATE
    *       - PMERR_INV_RECT
    *       - PMERR_INV_LENGTH_OR_COUNT
    */
   #define NGreCreateRectRegion 0x00004062L
   #define GreCreateRectRegion(a,b,c) (HRGN) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),(ULONG)(LONG)(c),0L,NGreCreateRectRegion)

   /**
    * @brief Destroy a region.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] hrgn  Region handle.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HRGN_BUSY
    *       - PMERR_INV_HRGN
    */
   #define NGreDestroyRegion 0x00004063L
   #define GreDestroyRegion(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HRGN)(b),0L,NGreDestroyRegion)

   /**
    * @brief Set a rectangular region.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] hrgn  Region handle.
    * @param[in] prcl  Pointer to a RECTL structure.
    * @param[in] lMode Mode.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HRGN_BUSY
    *       - PMERR_INV_COORDINATE
    *       - PMERR_INV_RECT
    */
   #define NGreSetRectRegion 0x00004064L
   #define GreSetRectRegion(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PRECTL)(c),(ULONG)(LONG)(d),0L,NGreSetRectRegion)

   /**
    * @brief Combine two regions.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] hrgnD  Destination region.
    * @param[in] hrgnS1 Source region 1.
    * @param[in] hrgnS2 Source region 2.
    * @param[in] lMode  Combination mode.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HRGN_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_COORDINATE
    *       - PMERR_INV_HRGN
    *       - PMERR_INV_LENGTH_OR_COUNT
    */
   #define NGreCombineRegion 0x00004065L
   #define GreCombineRegion(a,b,c,d,e) (INT) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(HRGN)(c),(ULONG)(HRGN)(d),(ULONG)(ULONG)(e),0L,NGreCombineRegion)

   /**
    * @brief Combine a region with a rectangle.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] hrgnD  Destination region.
    * @param[in] prcl   Pointer to a RECTL structure.
    * @param[in] hrgnS  Source region.
    * @param[in] lMode  Combination mode.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the errors
    *       documented for GreCombineRegion.
    */
   #define NGreCombineRectRegion 0x00004066L
   #define GreCombineRectRegion(a,b,c,d,e) (INT) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PRECTL)(c),(ULONG)(HRGN)(d),(ULONG)(ULONG)(e),0L,NGreCombineRectRegion)

   /**
    * @brief Combine a region with a short line.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] hrgn Region handle.
    * @param[in] psl  Pointer to a SHORTLINE structure.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HRGN_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_HRGN
    */
   #define NGreCombineShortLineRegion 0x00004067L
   #define GreCombineShortLineRegion(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PSHORTLINE)(c),0L,NGreCombineShortLineRegion)

   /**
    * @brief Test if two regions are equal.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] hrgn1 Region 1.
    * @param[in] hrgn2 Region 2.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the errors
    *       documented for GreCombineRegion.
    */
   #define NGreEqualRegion 0x00004068L
   #define GreEqualRegion(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(HRGN)(c),0L,NGreEqualRegion)

   /**
    * @brief Paint a region.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] hrgn Region handle.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_HRGN_BUSY
    *       - PMERR_INV_HDC
    *       - PMERR_INV_HRGN
    *       - PMERR_INV_IN_AREA
    *       - PMERR_INV_IN_PATH
    *       - PMERR_REGION_IS_CLIP_REGION
    */
   #define NGrePaintRegion 0x00004069L
   #define GrePaintRegion(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HRGN)(b),0L,NGrePaintRegion)

   /**
    * @brief Set the owner of a region.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] hrgn  Region handle.
    * @param[in] lOwner Owner value.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HRGN_BUSY
    *       - PMERR_INV_HRGN
    */
   #define NGreSetRegionOwner 0x0000406AL
   #define GreSetRegionOwner(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(ULONG)(c),0L,NGreSetRegionOwner)

   /**
    * @brief Frame a region.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] hrgn  Region handle.
    * @param[in] pszl  Pointer to a SIZEL structure.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_HRGN_BUSY
    *       - PMERR_INV_HRGN
    *       - PMERR_REGION_IS_CLIP_REGION
    */
   #define NGreFrameRegion 0x0000406BL
   #define GreFrameRegion(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PSIZEL)(c),0L,NGreFrameRegion)
#endif

#ifdef INCL_GRE_BITMAPS
   /**
    * @brief Draw bits from a source bitmap to a destination DC.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] pBits  Pointer to source bitmap data.
    * @param[in] pBmi   Pointer to a BITMAPINFO2 structure.
    * @param[in] lSrcX  Source X coordinate.
    * @param[in] pptl   Pointer to a POINTL structure.
    * @param[in] lDestX Destination X coordinate.
    * @param[in] lDestY Destination Y coordinate.
    *
    * @return LONG value indicating success or an error code.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BASE_ERROR
    *       - PMERR_BITMAP_NOT_SELECTED
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_BITBLT_MIX
    *       - PMERR_INV_BITBLT_STYLE
    *       - PMERR_INV_COLOR_DATA
    *       - PMERR_INV_COORDINATE
    *       - PMERR_INV_HDC
    *       - PMERR_INV_LENGTH_OR_COUNT
    */
   #define NGreDrawBits 0x00006022L
   #define GreDrawBits(a,b,c,d,e,f,g) (LONG) Gre32Entry9((ULONG)(HDC)(a),(ULONG)(PBYTE)(b),(ULONG)(PBITMAPINFO2)(c),(ULONG)(LONG)(d),(ULONG)(PPOINTL)(e),(ULONG)(LONG)(f),(ULONG)(ULONG)(g),0L,NGreDrawBits)

   /**
    * @brief Create a device bitmap.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] pBmih    Pointer to a BITMAPINFOHEADER2 structure.
    * @param[in] cOptions Options.
    * @param[in] pBits    Pointer to bitmap data.
    * @param[in] pBmi     Pointer to a BITMAPINFO2 structure.
    *
    * @return HBITMAP handle, or NULL on error.
    *
    * @retval NULL  Error.
    *
    * @note If NULL is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_BITMAP_DIMENSION
    *       - PMERR_INV_HDC
    */
   #define NGreDeviceCreateBitmap 0x00006023L
   #define GreDeviceCreateBitmap(a,b,c,d,e) (HBITMAP) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(PBITMAPINFOHEADER2)(b),(ULONG)(ULONG)(c),(ULONG)(PBYTE)(d),(ULONG)(PBITMAPINFO2)(e),0L,NGreDeviceCreateBitmap)

   /**
    * @brief Delete a device bitmap.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] hbm      Bitmap handle.
    * @param[in] pDelete  Pointer to a DELETERETURN structure.
    * @param[in] cOptions Options.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BITMAP_IS_SELECTED
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HBITMAP_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    */
   #define NGreDeviceDeleteBitmap 0x00004024L
   #define GreDeviceDeleteBitmap(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(HBITMAP)(b),(ULONG)(PDELETERETURN)(c),(ULONG)(ULONG)(d),0L,NGreDeviceDeleteBitmap)

   /**
    * @brief Select a device bitmap into a DC.
    *
    * @param[in] hdc Device context handle.
    * @param[in] hbm Bitmap handle.
    *
    * @return ULONG previous bitmap handle, or NULL on error.
    *
    * @retval NULL  Error.
    *
    * @note If NULL is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreDeviceSelectBitmap 0x00004025L
   #define GreDeviceSelectBitmap(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HBITMAP)(b),0L,NGreDeviceSelectBitmap)

   /**
    * @brief Perform a bit-block transfer.
    *
    * @param[in] hdcSrc Source DC.
    * @param[in] hdcDest Destination DC.
    * @param[in] lCount  Count.
    * @param[in] pParams Pointer to BITBLTPARAMETERS.
    * @param[in] lSrcX   Source X.
    * @param[in] lDestX  Destination X.
    * @param[in] pAttrs  Pointer to BITBLTATTRS.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BASE_ERROR
    *       - PMERR_BITMAP_IS_SELECTED
    *       - PMERR_BITMAP_NOT_SELECTED
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    */
   #define NGreBitblt 0x00006026L
   #define GreBitblt(a,b,c,d,e,f,g) (INT) Gre32Entry9((ULONG)(HDC)(a),(ULONG)(HDC)(b),(ULONG)(LONG)(c),(ULONG)(PBITBLTPARAMETERS)(d),(ULONG)(LONG)(e),(ULONG)(ULONG)(f),(ULONG)(PBITBLTATTRS)(g),0L,NGreBitblt)

   /**
    * @brief Get a pel value.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] pptl Pointer to a POINTL structure.
    *
    * @return LONG pel value, or -1 on error.
    *
    * @retval -1  Error.
    *
    * @note If -1 is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BITMAP_NOT_SELECTED
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    */
   #define NGreGetPel 0x00006027L
   #define GreGetPel(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreGetPel)

   /**
    * @brief Set a pel value.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] pptl Pointer to a POINTL structure.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BITMAP_NOT_SELECTED
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    */
   #define NGreSetPel 0x00004028L
   #define GreSetPel(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreSetPel)

   /**
    * @brief Draw a single row of image data (one bit per pel).
    *
    * @param[in] hdc   Device context handle.
    * @param[in] pData Pointer to data string.
    * @param[in] cBits Number of bits in row (maximum is 2040).
    * @param[in] offRow Row number relative to the current y-position.
    *
    * @return LONG integer (cHits) indicating correlation/error status.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_HITS  Successful with correlation hit.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BITMAP_NOT_SELECTED
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_COLOR_DATA
    *       - PMERR_INV_COLOR_INDEX
    *       - PMERR_INV_COORD_SPACE
    *       - PMERR_INV_COORDINATE
    *       - PMERR_INV_HDC
    *       - PMERR_INV_IMAGE_DATA_LENGTH
    *       - PMERR_INV_IN_AREA
    *       - PMERR_INV_IN_PATH
    *       - PMERR_INV_LENGTH_OR_COUNT
    *       - PMERR_INV_PICK_APERTURE_POSN
    *       - PMERR_INV_RECT
    *       - PMERR_INV_REGION_CONTROL
    *       - PMERR_INV_SCAN_START
    */
   #define NGreImageData 0x00004029L
   #define GreImageData(a,b,c,d) (INT) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PBYTE)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),0L,NGreImageData)

   /**
    * @brief Scan a line.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] pptl Pointer to a POINTL structure.
    * @param[in] lX   X coordinate.
    * @param[in] lY   Y coordinate.
    * @param[in] plX  Pointer to X result.
    * @param[in] plY  Pointer to Y result.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BASE_ERROR
    *       - PMERR_BITMAP_NOT_SELECTED
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_EXCEEDS_MAX_SEG_LENGTH
    *       - PMERR_HBITMAP_BUSY
    *       - PMERR_HDC_BUSY
    *       - PMERR_HUGE_FONTS_NOT_SUPPORTED
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_BACKGROUND_COL_ATTR
    *       - PMERR_INV_COLOR_ATTR
    *       - PMERR_INV_COLOR_DATA
    *       - PMERR_INV_COLOR_FORMAT
    */
   #define NGreScanLR 0x0000602AL
   #define GreScanLR(a,b,c,d,e,f) (INT) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PLONG)(e),(ULONG)(PLONG)(f),0L,NGreScanLR)

   /**
    * @brief Flood fill an area.
    *
    * @param[in] hdc Device context handle.
    * @param[in] lX  X coordinate.
    * @param[in] lY  Y coordinate.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_COORD_SPACE
    *       - PMERR_INV_HDC
    *       - PMERR_INV_HRGN
    *       - PMERR_INV_IN_PATH
    *       - PMERR_INV_LENGTH_OR_COUNT
    *       - PMERR_INV_RECT
    */
   #define NGreFloodFill 0x0000602BL
   #define GreFloodFill(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(LONG)(c),0L,NGreFloodFill)

   /**
    * @brief Save screen bits.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] prcl Pointer to a RECTL structure.
    *
    * @return ULONG handle, or 0 on error.
    *
    * @retval 0  Error.
    *
    * @note If 0 is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreSaveScreenBits 0x0000402CL
   #define GreSaveScreenBits(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreSaveScreenBits)

   /**
    * @brief Restore screen bits.
    *
    * @param[in] hdc     Device context handle.
    * @param[in] hsbBits Handle to saved bits.
    * @param[in] prcl    Pointer to a RECTL structure.
    * @param[in] lOptions Option flags.
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
    *       - PMERR_BITMAP_IS_SELECTED
    *       - PMERR_BITMAP_NOT_SELECTED
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_EXCEEDS_MAX_SEG_LENGTH
    *       - PMERR_HBITMAP_BUSY
    *       - PMERR_HDC_BUSY
    *       - PMERR_HUGE_FONTS_NOT_SUPPORTED
    *       - PMERR_INCOMPATIBLE_BITMAP
    *       - PMERR_INCORRECT_DC_TYPE
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_BACKGROUND_COL_ATTR
    *       - PMERR_INV_BACKGROUND_MIX_ATTR
    *       - PMERR_INV_BITBLT_MIX
    *       - PMERR_INV_BITBLT_STYLE
    *       - PMERR_INV_BITMAP_DIMENSION
    *       - PMERR_INV_CHAR_DIRECTION_ATTR
    *       - PMERR_INV_CHAR_MODE_ATTR
    *       - PMERR_INV_CHAR_SET_ATTR
    *       - PMERR_INV_CHAR_SHEAR_ATTR
    *       - PMERR_INV_CODEPAGE
    *       - PMERR_INV_COLOR_ATTR
    *       - PMERR_INV_COLOR_DATA
    *       - PMERR_INV_COLOR_FORMAT
    *       - PMERR_INV_COLOR_INDEX
    *       - PMERR_INV_COLOR_OPTIONS
    *       - PMERR_INV_COLOR_START_INDEX
    *       - PMERR_INV_COORD_SPACE
    *       - PMERR_INV_COORDINATE
    *       - PMERR_INV_DC_DATA
    *       - PMERR_INV_DC_TYPE
    *       - PMERR_INV_DRIVER_NAME
    *       - PMERR_INV_GEOM_LINE_WIDTH_ATTR
    *       - PMERR_INV_HBITMAP
    *       - PMERR_INV_HDC
    *       - PMERR_INV_HRGN
    *       - PMERR_INV_ID
    *       - PMERR_INV_IN_AREA
    *       - PMERR_INV_IN_PATH
    *       - PMERR_INV_INFO_TABLE
    *       - PMERR_INV_LENGTH_OR_COUNT
    *       - PMERR_INV_LINE_END_ATTR
    *       - PMERR_INV_LINE_JOIN_ATTR
    *       - PMERR_INV_LINE_TYPE_ATTR
    *       - PMERR_INV_LINE_WIDTH_ATTR
    *       - PMERR_INV_MARKER_SET_ATTR
    *       - PMERR_INV_MARKER_SYMBOL_ATTR
    *       - PMERR_INV_MIX_ATTR
    *       - PMERR_INV_PATTERN_REF_PT_ATTR
    *       - PMERR_INV_PATTERN_SET_ATTR
    */
   #define NGreRestoreScreenBits 0x0000402DL
   #define GreRestoreScreenBits(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(LHANDLE)(b),(ULONG)(PRECTL)(c),(ULONG)(ULONG)(d),0L,NGreRestoreScreenBits)

   /**
    * @brief Draw a border.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] prcl   Pointer to a RECTL structure.
    * @param[in] lThick Thickness.
    * @param[in] lType  Type.
    * @param[in] color  Color.
    * @param[in] clrBk  Background color.
    * @param[in] cOptions Options.
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
    *       - PMERR_INV_IN_AREA
    *       - PMERR_INV_IN_PATH
    *       - PMERR_HDC_BUSY
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_BITMAP_NOT_SELECTED
    */
   #define NGreDrawBorder 0x0000602EL
   #define GreDrawBorder(a,b,c,d,e,f,g) (BOOL) Gre32Entry9((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(COLOR)(e),(ULONG)(COLOR)(f),(ULONG)(ULONG)(g),0L,NGreDrawBorder)

   /**
    * @brief Set the cursor for a device.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] pptl Pointer to a POINTL structure.
    * @param[in] hbm  Bitmap handle.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_COORDINATE
    *       - PMERR_INV_CURSOR_BITMAP
    *       - PMERR_INV_HDC
    */
   #define NGreDeviceSetCursor 0x0000402FL
   #define GreDeviceSetCursor(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(HBITMAP)(c),0L,NGreDeviceSetCursor)

   /**
    * @brief Get bitmap bits.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] hbm    Bitmap handle.
    * @param[in] lSrcX  Source X.
    * @param[in] lSrcY  Source Y.
    * @param[in] pBits  Pointer to output buffer.
    * @param[in] pBmi   Pointer to a BITMAPINFO2 structure.
    *
    * @return LONG value indicating success or an error code.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BITMAP_NOT_SELECTED
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    */
   #define NGreGetBitmapBits 0x00006030L
   #define GreGetBitmapBits(a,b,c,d,e,f) (LONG) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(HBITMAP)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PBYTE)(e),(ULONG)(PBITMAPINFO2)(f),0L,NGreGetBitmapBits)

   /**
    * @brief Set bitmap bits.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] hbm    Bitmap handle.
    * @param[in] lSrcX  Source X.
    * @param[in] lSrcY  Source Y.
    * @param[in] pBits  Pointer to input data.
    * @param[in] pBmi   Pointer to a BITMAPINFO2 structure.
    *
    * @return LONG value indicating success or an error code.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BITMAP_NOT_SELECTED
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INCORRECT_DC_TYPE
    *       - PMERR_INV_HBITMAP
    */
   #define NGreSetBitmapBits 0x00006031L
   #define GreSetBitmapBits(a,b,c,d,e,f) (LONG) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(HBITMAP)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PBYTE)(e),(ULONG)(PBITMAPINFO2)(f),0L,NGreSetBitmapBits)
#endif


#ifdef INCL_GRE_DCS
   /**
    * @brief Open a device context.
    *
    * @param[in] hdc       Device context handle.
    * @param[in] lOptions  Options.
    * @param[in] pszDevice Device name.
    * @param[in] lMode     Mode.
    * @param[in] pData     Pointer to DEVOPENDATA.
    *
    * @return HDC handle, or DEV_ERROR on error.
    *
    * @retval DEV_ERROR  Error.
    *
    * @note If DEV_ERROR is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BASE_ERROR
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    */
   #define NGreOpenDC 0x00000200L
   #define GreOpenDC(a,b,c,d,e) (HDC) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PSZ)(c),(ULONG)(LONG)(d),(ULONG)(PDEVOPENDATA)(e),0L,NGreOpenDC)

   /**
    * @brief Close a device context.
    *
    * @param[in] hdc Device context handle.
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
    *       - PMERR_BITMAP_IS_SELECTED
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_CODEPAGE
    *       - PMERR_INV_HDC
    *       - PMERR_INV_IN_AREA
    *       - PMERR_INV_IN_PATH
    *       - PMERR_INV_LENGTH_OR_COUNT
    */
   #define NGreCloseDC 0x00004201L
   #define GreCloseDC(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreCloseDC)
#endif

#ifdef INCL_GRE_INKPATH
   /**
    * @brief Begin an ink path.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] lMode    Mode.
    * @param[in] lOptions Options.
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
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_END_PATH_OPTIONS
    *       - PMERR_INV_HDC
    *       - PMERR_NOT_IN_PATH
    *       - PMERR_PATH_LIMIT_EXCEEDED
    */
   #define NGreBeginInkPath 0x00004259L
   #define GreBeginInkPath(a,b,c) (BOOL) Gre32Entry5((HDC)(a),(LONG)(b),(ULONG)(c),0L,NGreBeginInkPath)

   /* GreEndInkPath — no verifiable error list found; left undocumented. */
   #define NGreEndInkPath 0x0000425AL
   #define GreEndInkPath(a,b) (BOOL) Gre32Entry4((HDC)(a),(ULONG)(b),0L,NGreEndInkPath)

   /* GreStrokeInkPath — no verifiable error list found; left undocumented. */
   #define NGreStrokeInkPath 0x0000425BL
   #define GreStrokeInkPath(a,b,c,d,e) (BOOL) Gre32Entry7((HDC)(a),(LONG)(b),(LONG)(c),(PPOINTL)(d),(ULONG)(e),0L,NGreStrokeInkPath)
#endif

#ifdef INCL_GRE_PALETTE
   /**
    * @brief Create a palette.
    *
    * @param[in] lFormat   Format.
    * @param[in] lOptions  Options.
    * @param[in] cEntries  Number of entries.
    * @param[in] pEntries  Pointer to palette entries.
    *
    * @return HPAL handle, or NULL on error.
    *
    * @retval NULL  Error.
    *
    * @note If NULL is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_LENGTH_OR_COUNT
    */
   #define NGreCreatePalette 0x00000240L
   #define GreCreatePalette(a,b,c,d) (HPAL) Gre32Entry6((ULONG)(ULONG)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(PULONG)(d),0L,NGreCreatePalette)

   /* GreDeletePalette — no verifiable error list found; left undocumented. */
   #define NGreDeletePalette 0x00000241L
   #define GreDeletePalette(a) (ULONG) Gre32Entry3((ULONG)(HPAL)(a),0L,NGreDeletePalette)

   /**
    * @brief Select a palette into a DC.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] hpal Palette handle.
    *
    * @return ULONG previous palette handle, or NULL on error.
    *
    * @retval NULL  Error.
    *
    * @note If NULL is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_HDC
    *       - PMERR_PALETTE_BUSY
    */
   #define NGreSelectPalette 0x00004242L
   #define GreSelectPalette(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HPAL)(b),0L,NGreSelectPalette)

   /* GreSetPaletteEntries — no verifiable error list found; left undocumented. */
   #define NGreSetPaletteEntries 0x00000243L
   #define GreSetPaletteEntries(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HPAL)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(PULONG)(e),0L,NGreSetPaletteEntries)

   /* GreAnimatePalette — no verifiable error list found; left undocumented. */
   #define NGreAnimatePalette 0x00000244L
   #define GreAnimatePalette(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HPAL)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(PULONG)(e),0L,NGreAnimatePalette)

   /* GreQueryPalette — no verifiable error list found; left undocumented. */
   #define NGreQueryPalette 0x00004245L
   #define GreQueryPalette(a) (ULONG) Gre32Entry3((ULONG)(HDC)(a),0L,NGreQueryPalette)

   /* GreQueryPaletteInfo — no verifiable error list found; left undocumented. */
   #define NGreQueryPaletteInfo 0x00000246L
   #define GreQueryPaletteInfo(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HPAL)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(PULONG)(e),0L,NGreQueryPaletteInfo)

   /* GreSetPaletteOwner — no verifiable error list found; left undocumented. */
   #define NGreSetPaletteOwner 0x00000247L
   #define GreSetPaletteOwner(a,b) (BOOL) Gre32Entry4((ULONG)(HPAL)(a),(ULONG)(ULONG)(b),0L,NGreSetPaletteOwner)

   /* GreResizePalette — no verifiable error list found; left undocumented. */
   #define NGreResizePalette 0x00000248L
   #define GreResizePalette(a,b) (ULONG) Gre32Entry4((ULONG)(HPAL)(a),(ULONG)(ULONG)(b),0L,NGreResizePalette)

   /* GreQueryDefaultPaletteInfo — no verifiable error list found; left undocumented. */
   #define NGreQueryDefaultPaletteInfo 0x00004249L
   #define GreQueryDefaultPaletteInfo(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(PULONG)(e),0L,NGreQueryDefaultPaletteInfo)
#endif

#ifdef INCL_WINPOINTERS
   /* GreDeviceSetPointer — no verifiable error list found; left undocumented. */
   #define NGreDeviceSetPointer 0x0000403FL
   #define GreDeviceSetPointer(a,b) (LONG) Gre32Entry4((a), (PHWSETPTRIN)(b), 0L, NGreDeviceSetPointer)

   /**
    * @brief Set a color cursor.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] pInfo Pointer to a POINTERINFO structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_COORDINATE
    */
   #define NGreSetColorCursor 0x00004032L
   #define GreSetColorCursor(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTERINFO)(b),0L,NGreSetColorCursor)
#endif

#ifdef INCL_GRE_SCANS
   /**
    * @brief Draw a poly-scanline.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] pScan Pointer to a SCANDATA structure.
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
    *       - PMERR_BITMAP_NOT_SELECTED
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    */
   #define NGrePolyScanline 0x0000401CL
   #define GrePolyScanline(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PSCANDATA)(b),0L,NGrePolyScanline)
#endif

#ifdef INCL_GRE_STRINGS
   /**
    * @brief Output a character string.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] lCount Count.
    * @param[in] pch    Pointer to character string.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BASE_ERROR
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_EXCEEDS_MAX_SEG_LENGTH
    *       - PMERR_FONT_AND_MODE_MISMATCH
    *       - PMERR_PATH_LIMIT_EXCEEDED
    */
   #define NGreCharString 0x00005035L
   #define GreCharString(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PCH)(c),0L,NGreCharString)

   /**
    * @brief Output a character string at a position.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] pptl   Pointer to a POINTL structure.
    * @param[in] prcl   Pointer to a RECTL structure.
    * @param[in] lOptions Options.
    * @param[in] lCount Count.
    * @param[in] pch    Pointer to character string.
    * @param[in] plX    Pointer to X array.
    * @param[in] pInfo  Pointer to CSP_INFO.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BASE_ERROR
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_EXCEEDS_MAX_SEG_LENGTH
    *       - PMERR_FONT_AND_MODE_MISMATCH
    */
   #define NGreCharStringPos 0x00007036L
   #define GreCharStringPos(a,b,c,d,e,f,g,h) (INT) Gre32Entry10((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(PRECTL)(c),(ULONG)(ULONG)(d),(ULONG)(LONG)(e),(ULONG)(PCH)(f),(ULONG)(PLONG)(g),(ULONG)(PCSP_INFO)(h),0L,NGreCharStringPos)

   /**
    * @brief Query the text box.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] lCount   Count.
    * @param[in] pch      Pointer to character string.
    * @param[in] lOptions Options.
    * @param[in] pptl     Pointer to output POINTL array.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    */
   #define NGreQueryTextBox 0x00005037L
   #define GreQueryTextBox(a,b,c,d,e) (BOOL) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PCH)(c),(ULONG)(LONG)(d),(ULONG)(PPOINTL)(e),0L,NGreQueryTextBox)

   /**
    * @brief Query character positions.
    *
    * @param[in] hdc        Device context handle.
    * @param[in] pptlStart  Pointer to starting position.
    * @param[in] lOptions   Option flags.
    * @param[in] cChars     Number of bytes in string.
    * @param[in] pchString  Pointer to character string.
    * @param[in] pAdx       Pointer to increment array.
    * @param[in] paptXY     Pointer to output positions.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_INV_MATRIX_ELEMENT
    *       - PMERR_INV_SETID
    *       - PMERR_INV_CHAR_MODE_ATTR
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_LENGTH_OR_COUNT
    *       - PMERR_INV_CODEPAGE
    *       - PMERR_EXCEEDS_MAX_SEG_LENGTH
    *       - PMERR_INV_HDC
    *       - PMERR_INV_CHAR_ANGLE_ATTR
    *       - PMERR_INV_TRANSFORM_TYPE
    */
   #define NGreQueryCharPositions 0x00005038L
   #define GreQueryCharPositions(a,b,c,d,e,f,g) (ULONG) Gre32Entry9((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(ULONG)(c),(ULONG)(LONG)(d),(ULONG)(PCH)(e),(ULONG)(PLONG)(f),(ULONG)(PPOINTL)(g),0L,NGreQueryCharPositions)

   /**
    * @brief Query the width table.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] lStart   Start index.
    * @param[in] cEntries Number of entries.
    * @param[in] plWidths Pointer to output width array.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_EXCEEDS_MAX_SEG_LENGTH
    *       - PMERR_HDC_BUSY
    */
   #define NGreQueryWidthTable 0x00005039L
   #define GreQueryWidthTable(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(LONG)(c),(ULONG)(PLONG)(d),0L,NGreQueryWidthTable)

   /**
    * @brief Query character outline.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] lCode Code.
    * @param[in] pData Pointer to output data.
    * @param[in] lOptions Options.
    * @param[in] lCount Count.
    *
    * @return LONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BASE_ERROR
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_IN_AREA
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_EXCEEDS_MAX_SEG_LENGTH
    */
   #define NGreQueryCharOutline 0x00004256L
   #define GreQueryCharOutline(a,b,c,d,e) (LONG)Gre32Entry7((HDC)(a),(ULONG)(b),(ULONG)(PBYTE)(c),(ULONG)(d),(ULONG)(e),0L,NGreQueryCharOutline)

   /* GreQueryCharMetricsTable — no verifiable error list found; left undocumented. */
   #define NGreQueryCharMetricsTable 0x00004258L
   #define GreQueryCharMetricsTable(a,b,c,d,e,f) (BOOL)Gre32Entry8((HDC)(a),(ULONG)(b),(ULONG)(c),(ULONG)(PCHARMETRICS)(d),(ULONG)(e),(ULONG)(f),0L,NGreQueryCharMetricsTable)

   /* GreRealizeString — no verifiable error list found; left undocumented. */
   #define NGreRealizeString 0x00004257L
   #define GreRealizeString(a,b,c,d,e,f) (LONG)Gre32Entry8((HDC)(a),(ULONG)(b),(ULONG)(c),(ULONG)(d),(ULONG)(e),(ULONG)(f),0L,NGreRealizeString)

   /* GreQueryTabbedTextExtent — no verifiable error list found; left undocumented. */
   #define NGreQueryTabbedTextExtent 0x00004269L
   #define GreQueryTabbedTextExtent(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(a),(ULONG)(b),(ULONG)(c),(ULONG)(d), (ULONG)(e), 0L, NGreQueryTabbedTextExtent)

   /* GreTabbedCharStringAt — no verifiable error list found; left undocumented. */
   #define NGreTabbedCharStringAt 0x0000426AL
   #define GreTabbedCharStringAt(a,b,c,d,e,f,g,h) (ULONG) Gre32Entry10((ULONG)(a),(ULONG)(PPOINTL)(b),(ULONG)(PRECTL)(c),(ULONG)(d), (ULONG)(e),(ULONG)(PCH)(f),(ULONG)(PLONG)(g),(ULONG)(PULONG)(h), 0L, NGreTabbedCharStringAt)
#endif

#ifdef INCL_GRE_MARKERS
   /**
    * @brief Draw a poly-marker.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] pptl   Pointer to a POINTL array.
    * @param[in] lCount Count.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BASE_ERROR
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_EXCEEDS_MAX_SEG_LENGTH
    *       - PMERR_HDC_BUSY
    *       - PMERR_HRGN_BUSY
    *       - PMERR_HUGE_FONTS_NOT_SUPPORTED
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_MARKER_SYMBOL_ATTR
    *       - PMERR_PATH_LIMIT_EXCEEDED
    */
   #define NGrePolyMarker 0x0000403AL
   #define GrePolyMarker(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),0L,NGrePolyMarker)
#endif

#ifdef INCL_AVIOP
   /**
    * @brief Output characters in a rectangle.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] pPs  Presentation space.
    * @param[in] pRect Pointer to a grid rectangle reference.
    *
    * @return ULONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreCharRect 0x0000403BL
   #define GreCharRect(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(VioPresentationSpace *)(b),(ULONG)(LPGridRectRef)(c),0L,NGreCharRect)

   /**
    * @brief Draw a string of character cells from the LVB to the device context.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] pPs   Presentation space.
    * @param[in] pStr  Pointer to a grid string reference.
    *
    * @return LONG value indicating success or an error code.
    *
    * @retval NO_ERROR  Successful.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreCharStr 0x0000403CL
   #define GreCharStr(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(VioPresentationSpace *)(b),(ULONG)(LPGridStringRef)(c),0L,NGreCharStr)

   /**
    * @brief Scroll a rectangle.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] pPs   Presentation space.
    * @param[in] pRect Pointer to a scroll rectangle reference.
    *
    * @return LONG value indicating success or an error code.
    *
    * @retval NO_ERROR  Successful.
    * @retval CE_INVALID_PRESENTATION_SPACE  Error.
    *
    * @note If CE_INVALID_PRESENTATION_SPACE is returned, the handling routine
    *       must call WinSetErrorInfo.
    */
   #define NGreScrollRect 0x0000403DL
   #define GreScrollRect(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(VioPresentationSpace *)(b),(ULONG)(LPScrollRectRef)(c),0L,NGreScrollRect)

   /* GreUpdateCursor — no verifiable error list found; left undocumented. */
   #define NGreUpdateCursor 0x0000403EL
   #define GreUpdateCursor(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(VioPresentationSpace *)(b),0L,NGreUpdateCursor)
#endif

#ifdef INCL_GRE_PATHS
   /**
    * @brief Begin an area.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] lOptions Options.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_ALREADY_IN_AREA
    *       - PMERR_BASE_ERROR
    *       - PMERR_PATH_LIMIT_EXCEEDED
    *       - PMERR_INV_IN_PATH
    *       - PMERR_HDC_BUSY
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    */
   #define NGreBeginArea 0x00004046L
   #define GreBeginArea(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreBeginArea)

   /**
    * @brief End an area.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] lOptions Options.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BASE_ERROR
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_IN_AREA
    *       - PMERR_NOT_IN_PATH
    *       - PMERR_PATH_LIMIT_EXCEEDED
    */
   #define NGreEndArea 0x00004047L
   #define GreEndArea(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreEndArea)

   /**
    * @brief Begin a path.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] pHid Pointer to a HID structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_ALREADY_IN_PATH
    *       - PMERR_BASE_ERROR
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    */
   #define NGreBeginPath 0x00004048L
   #define GreBeginPath(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PHID)(b),0L,NGreBeginPath)

   /**
    * @brief End a path.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] lOptions Options.
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
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_END_PATH_OPTIONS
    *       - PMERR_NOT_IN_PATH
    *       - PMERR_PATH_LIMIT_EXCEEDED
    */
   #define NGreEndPath 0x00004049L
   #define GreEndPath(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreEndPath)

   /* GreCloseFigure — no verifiable error list found; left undocumented. */
   #define NGreCloseFigure 0x0000404AL
   #define GreCloseFigure(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreCloseFigure)

   /* GreFillPath — no verifiable error list found; left undocumented. */
   #define NGreFillPath 0x0000404BL
   #define GreFillPath(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PHID)(b),(ULONG)(ULONG)(c),0L,NGreFillPath)

   /* GreOutlinePath — no verifiable error list found; left undocumented. */
   #define NGreOutlinePath 0x0000404CL
   #define GreOutlinePath(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PHID)(b),(ULONG)(ULONG)(c),0L,NGreOutlinePath)

   /* GreModifyPath — no verifiable error list found; left undocumented. */
   #define NGreModifyPath 0x0000404DL
   #define GreModifyPath(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PHID)(b),(ULONG)(ULONG)(c),0L,NGreModifyPath)

   /* GreStrokePath — no verifiable error list found; left undocumented. */
   #define NGreStrokePath 0x0000404EL
   #define GreStrokePath(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PHID)(b),(ULONG)(ULONG)(c),0L,NGreStrokePath)

   /* GreSelectClipPath — no verifiable error list found; left undocumented. */
   #define NGreSelectClipPath 0x0000404FL
   #define GreSelectClipPath(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PHID)(b),(ULONG)(ULONG)(c),0L,NGreSelectClipPath)

   /* GreSavePath — no verifiable error list found; left undocumented. */
   #define NGreSavePath 0x00004050L
   #define GreSavePath(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreSavePath)

   /* GreRestorePath — no verifiable error list found; left undocumented. */
   #define NGreRestorePath 0x00004051L
   #define GreRestorePath(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreRestorePath)

   /* GreClip1DPath — no verifiable error list found; left undocumented. */
   #define NGreClip1DPath 0x00004052L
   #define GreClip1DPath(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPATH)(b),(ULONG)(PPIPELINEINFO)(c),0L,NGreClip1DPath)

   /* GreDrawRawPath — no verifiable error list found; left undocumented. */
   #define NGreDrawRawPath 0x00004053L
   #define GreDrawRawPath(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPATH)(b),0L,NGreDrawRawPath)

   /* GreDrawCookedPath — no verifiable error list found; left undocumented. */
   #define NGreDrawCookedPath 0x00004054L
   #define GreDrawCookedPath(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PPATH)(b),(ULONG)(PCURVE)(c),(ULONG)(LONG)(d),0L,NGreDrawCookedPath)

   /* GreAreaSetAttributes — no verifiable error list found; left undocumented. */
   #define NGreAreaSetAttributes 0x00006055L
   #define GreAreaSetAttributes(a,b,c,d,e) (BOOL) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(PBUNDLE)(e),0L,NGreAreaSetAttributes)

   /* GrePathToRegion — no verifiable error list found; left undocumented. */
   #define NGrePathToRegion 0x00004057L
   #define GrePathToRegion(a,b,c) (HRGN) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PHID)(b),(ULONG)(ULONG)(c),0L,NGrePathToRegion)
#endif


#ifdef INCL_GRE_CLIP
   /**
    * @brief Get the clip box.
    *
    * @param[in]  hdc  Device context handle.
    * @param[out] prcl Pointer to a RECTL structure.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_COORD_SPACE
    */
   #define NGreGetClipBox 0x0000406EL
   #define GreGetClipBox(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreGetClipBox)

   /**
    * @brief Get the clip rectangles.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] prcl     Pointer to a RECTL structure.
    * @param[in] prgnrc   Pointer to a RGNRECT structure.
    * @param[in] prclOut  Pointer to a RECTL structure.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_COORDINATE
    *       - PMERR_INV_HDC
    *       - PMERR_INV_RECT
    *       - PMERR_INV_REGION_CONTROL
    */
   #define NGreGetClipRects 0x0000406FL
   #define GreGetClipRects(a,b,c,d) (INT) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),(ULONG)(PRGNRECT)(c),(ULONG)(PRECTL)(d),0L,NGreGetClipRects)

   /**
    * @brief Offset the clip region.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] pptl Pointer to a POINTL structure.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_COORD_OFFSET
    */
   #define NGreOffsetClipRegion 0x00004070L
   #define GreOffsetClipRegion(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreOffsetClipRegion)

   /**
    * @brief Test if a point is visible.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] pptl Pointer to a POINTL structure.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_COORD_SPACE
    */
   #define NGrePtVisible 0x00004071L
   #define GrePtVisible(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGrePtVisible)

   /**
    * @brief Test if a rectangle is visible.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] prcl Pointer to a RECTL structure.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_COORD_SPACE
    */
   #define NGreRectVisible 0x00004072L
   #define GreRectVisible(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreRectVisible)

   /**
    * @brief Query the clip region.
    *
    * @param[in] hdc Device context handle.
    *
    * @return HRGN handle, or NULL on error.
    *
    * @retval NULL  Error.
    *
    * @note If NULL is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    */
   #define NGreQueryClipRegion 0x00004073L
   #define GreQueryClipRegion(a) (HRGN) Gre32Entry3((ULONG)(HDC)(a),0L,NGreQueryClipRegion)

   /**
    * @brief Select a clip region.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] hrgn  Region handle.
    * @param[in] phrgn Pointer to a HRGN.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    */
   #define NGreSelectClipRegion 0x00004074L
   #define GreSelectClipRegion(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PHRGN)(c),0L,NGreSelectClipRegion)

   /**
    * @brief Intersect the clip with a rectangle.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] prcl Pointer to a RECTL structure.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    */
   #define NGreIntersectClipRectangle 0x00004075L
   #define GreIntersectClipRectangle(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreIntersectClipRectangle)

   /**
    * @brief Exclude a rectangle from the clip.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] prcl Pointer to a RECTL structure.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    */
   #define NGreExcludeClipRectangle 0x00004076L
   #define GreExcludeClipRectangle(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreExcludeClipRectangle)

   /**
    * @brief Set an XForm rectangle.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] prcl Pointer to a RECTL structure.
    *
    * @return ULONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_COORDINATE
    */
   #define NGreSetXformRect 0x00004077L
   #define GreSetXformRect(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreSetXformRect)

   /**
    * @brief Save the region.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] lMode Mode.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_COORDINATE
    *       - PMERR_INV_HDC
    *       - PMERR_INV_HRGN
    *       - PMERR_INV_RECT
    */
   #define NGreSaveRegion 0x0000407BL
   #define GreSaveRegion(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreSaveRegion)

   /**
    * @brief Restore the region.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] lMode Mode.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_COORDINATE
    */
   #define NGreRestoreRegion 0x0000407CL
   #define GreRestoreRegion(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreRestoreRegion)

   /**
    * @brief Clip path curves.
    *
    * @param[in] hdc       Device context handle.
    * @param[in] pPath     Pointer to a PATH structure.
    * @param[in] pPipeline Pointer to a PIPELINEINFO structure.
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
    *       - PMERR_INV_IN_AREA
    *       - PMERR_PATH_LIMIT_EXCEEDED
    *       - PMERR_INV_IN_PATH
    *       - PMERR_PATH_UNKNOWN
    *       - PMERR_HDC_BUSY
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_BITMAP_NOT_SELECTED
    *       - PMERR_INSUFFICIENT_MEMORY
    */
   #define NGreClipPathCurves 0x0000407DL
   #define GreClipPathCurves(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPATH)(b),(ULONG)(PPIPELINEINFO)(c),0L,NGreClipPathCurves)

   /**
    * @brief Select a path region.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] hrgn Region handle.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_HRGN_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_COORDINATE
    */
   #define NGreSelectPathRegion 0x0000407EL
   #define GreSelectPathRegion(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HRGN)(b),0L,NGreSelectPathRegion)

   /**
    * @brief Select a bitmap for a region.
    *
    * @param[in] hdc Device context handle.
    * @param[in] hbm Bitmap handle.
    *
    * @return ULONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_COORDINATE
    */
   #define NGreRegionSelectBitmap 0x0000407FL
   #define GreRegionSelectBitmap(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HBITMAP)(b),0L,NGreRegionSelectBitmap)

   /**
    * @brief Copy the clip region.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] hrgn     Region handle.
    * @param[in] prcl     Pointer to a RECTL structure.
    * @param[in] lOptions Options.
    *
    * @return Integer result.
    *
    * @retval GPI_OK    Successful.
    * @retval GPI_ERROR Error.
    *
    * @note If the function returns GPI_ERROR, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_HRGN_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_COORDINATE
    *       - PMERR_INV_HDC
    *       - PMERR_INV_HRGN
    *       - PMERR_REGION_IS_CLIP_REGION
    */
   #define NGreCopyClipRegion 0x00004080L
   #define GreCopyClipRegion(a,b,c,d) (INT) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PRECTL)(c),(ULONG)(ULONG)(d),0L,NGreCopyClipRegion)

   /**
    * @brief Set up a DC.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] hrgn     Region handle.
    * @param[in] lMode    Mode.
    * @param[in] lOptions Options.
    * @param[in] prcl     Pointer to a RECTL structure.
    * @param[in] lParam   Parameter.
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
   #define NGreSetupDC 0x00004081L
   #define GreSetupDC(a,b,c,d,e,f) (BOOL) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PRECTL)(e),(ULONG)(ULONG)(f),0L,NGreSetupDC)
#endif

#ifdef INCL_GRE_XFORMS
   /**
    * @brief Get page units.
    *
    * @param[in]  hdc      Device context handle.
    * @param[out] pulUnits Pointer to output units.
    *
    * @return LONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    */
   #define NGreGetPageUnits 0x00004083L
   #define GreGetPageUnits(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PULONG)(b),0L,NGreGetPageUnits)

   /**
    * @brief Set page units.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] lUnits Units.
    * @param[in] lX     X value.
    * @param[in] lY     Y value.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    */
   #define NGreSetPageUnits 0x00004084L
   #define GreSetPageUnits(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),0L,NGreSetPageUnits)

   /**
    * @brief Get the model XForm.
    *
    * @param[in]  hdc    Device context handle.
    * @param[out] pXform Pointer to an XFORM structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    */
   #define NGreGetModelXform 0x00004085L
   #define GreGetModelXform(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),0L,NGreGetModelXform)

   /**
    * @brief Set the model XForm.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] pXform   Pointer to an XFORM structure.
    * @param[in] lOptions Options.
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
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    */
   #define NGreSetModelXform 0x00004086L
   #define GreSetModelXform(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),(ULONG)(ULONG)(c),0L,NGreSetModelXform)

   /**
    * @brief Get the window/viewport XForm.
    *
    * @param[in]  hdc    Device context handle.
    * @param[out] pXform Pointer to an XFORM structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    */
   #define NGreGetWindowViewportXform 0x00004087L
   #define GreGetWindowViewportXform(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),0L,NGreGetWindowViewportXform)

   /**
    * @brief Set the window/viewport XForm.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] pXform   Pointer to an XFORM structure.
    * @param[in] lOptions Options.
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
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_HRGN_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_COORD_SPACE
    *       - PMERR_INV_COORDINATE
    */
   #define NGreSetWindowViewportXform 0x00004088L
   #define GreSetWindowViewportXform(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),(ULONG)(ULONG)(c),0L,NGreSetWindowViewportXform)

   /**
    * @brief Get the global viewing XForm.
    *
    * @param[in]  hdc    Device context handle.
    * @param[out] pXform Pointer to an XFORM structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    */
   #define NGreGetGlobalViewingXform 0x00004089L
   #define GreGetGlobalViewingXform(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),0L,NGreGetGlobalViewingXform)

   /**
    * @brief Set the global viewing XForm.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] pXform   Pointer to an XFORM structure.
    * @param[in] lOptions Options.
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
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    */
   #define NGreSetGlobalViewingXform 0x0000408AL
   #define GreSetGlobalViewingXform(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),(ULONG)(ULONG)(c),0L,NGreSetGlobalViewingXform)

   /**
    * @brief Save XForm data.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] lMode Mode.
    * @param[in] pData Pointer to data.
    *
    * @return LONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    *       - PMERR_INV_LENGTH_OR_COUNT
    */
   #define NGreSaveXformData 0x0000408BL
   #define GreSaveXformData(a,b,c) (LONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PBYTE)(c),0L,NGreSaveXformData)

   /**
    * @brief Restore XForm data.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] lMode Mode.
    * @param[in] pData Pointer to data.
    *
    * @return LONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BASE_ERROR
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    */
   #define NGreRestoreXformData 0x0000408CL
   #define GreRestoreXformData(a,b,c) (LONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PBYTE)(c),0L,NGreRestoreXformData)

   /**
    * @brief Get the page viewport.
    *
    * @param[in]  hdc  Device context handle.
    * @param[out] prcl Pointer to a RECTL structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    */
   #define NGreGetPageViewport 0x0000408DL
   #define GreGetPageViewport(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreGetPageViewport)

   /* GreSetPageViewport — no verifiable error list found; left undocumented. */
   #define NGreSetPageViewport 0x0000408EL
   #define GreSetPageViewport(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),(ULONG)(ULONG)(c),0L,NGreSetPageViewport)

   /* GreGetGraphicsField — no verifiable error list found; left undocumented. */
   #define NGreGetGraphicsField 0x00004091L
   #define GreGetGraphicsField(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreGetGraphicsField)

   /**
    * @brief Set the graphics field.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] prcl Pointer to a RECTL structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_COORDINATE
    *       - PMERR_INV_GRAPHICS_FIELD
    *       - PMERR_INV_HDC
    *       - PMERR_INV_IN_AREA
    */
   #define NGreSetGraphicsField 0x00004092L
   #define GreSetGraphicsField(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreSetGraphicsField)

   /**
    * @brief Get the viewing limits.
    *
    * @param[in]  hdc  Device context handle.
    * @param[out] prcl Pointer to a RECTL structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    */
   #define NGreGetViewingLimits 0x00004093L
   #define GreGetViewingLimits(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreGetViewingLimits)

   /**
    * @brief Set the viewing limits.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] prcl Pointer to a RECTL structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_COORDINATE
    *       - PMERR_INV_GRAPHICS_FIELD
    *       - PMERR_INV_HDC
    *       - PMERR_INV_IN_AREA
    */
   #define NGreSetViewingLimits 0x00004094L
   #define GreSetViewingLimits(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreSetViewingLimits)

   /* GreQueryViewportSize — no verifiable error list found; left undocumented. */
   #define NGreQueryViewportSize 0x00004095L
   #define GreQueryViewportSize(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),(ULONG)(PVIEWPORTSIZE)(d),0L,NGreQueryViewportSize)

   /**
    * @brief Convert coordinates.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] lSrcX Source X.
    * @param[in] lSrcY Source Y.
    * @param[in] pptl Pointer to a POINTL array.
    * @param[in] lCount Count.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_COORD_SPACE
    *       - PMERR_INV_HDC
    *       - PMERR_INV_LENGTH_OR_COUNT
    */
   #define NGreConvert 0x00004096L
   #define GreConvert(a,b,c,d,e) (BOOL) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(LONG)(c),(ULONG)(PPOINTL)(d),(ULONG)(LONG)(e),0L,NGreConvert)

   /* GreConvertPath — no verifiable error list found; left undocumented. */
   #define NGreConvertPath 0x00004097L
   #define GreConvertPath(a,b,c,d,e,f) (BOOL) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(PPATH)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PCURVE)(e),(ULONG)(LONG)(f),0L,NGreConvertPath)

   /**
    * @brief Save the XForm.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] lMode Mode.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_HDC
    */
   #define NGreSaveXform 0x00004098L
   #define GreSaveXform(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreSaveXform)

   /**
    * @brief Restore the XForm.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] lMode Mode.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    */
   #define NGreRestoreXform 0x00004099L
   #define GreRestoreXform(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreRestoreXform)

   /**
    * @brief Multiply two XForms.
    *
    * @param[in] hdc     Device context handle.
    * @param[in] pXform1 Pointer to the first XFORM.
    * @param[in] pXform2 Pointer to the second XFORM.
    * @param[in] lOptions Options.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_INV_MATRIX_ELEMENT
    *       - PMERR_INV_TRANSFORM_TYPE
    */
   #define NGreMultiplyXforms 0x0000409AL
   #define GreMultiplyXforms(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),(ULONG)(PXFORM)(c),(ULONG)(ULONG)(d),0L,NGreMultiplyXforms)

   /**
    * @brief Convert points using a matrix.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] pptl   Pointer to a POINTL array.
    * @param[in] lCount Count.
    * @param[in] pXform Pointer to an XFORM structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_INV_LENGTH_OR_COUNT
    */
   #define NGreConvertWithMatrix 0x0000409BL
   #define GreConvertWithMatrix(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),(ULONG)(PXFORM)(d),0L,NGreConvertWithMatrix)
#endif

#ifdef INCL_GRE_DEVMISC1
   /**
    * @brief Get device attributes.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] lAttr  Attribute.
    * @param[in] lOptions Options.
    * @param[in] pBundle Pointer to a BUNDLE structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreDeviceGetAttributes 0x0000609DL
   #define GreDeviceGetAttributes(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(PBUNDLE)(d),0L,NGreDeviceGetAttributes)

   /* GreDeviceSetAVIOFont2 — no verifiable error list found; left undocumented. */
   #define NGreDeviceSetAVIOFont2 0x0000409EL
   #define GreDeviceSetAVIOFont2(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PFATTRS)(b),(ULONG)(PFOCAFONT)(c),(ULONG)(LCID)(d),0L,NGreDeviceSetAVIOFont2)

   /**
    * @brief Get the pair kerning table.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] lCode Code.
    * @param[in] pKerning Pointer to a KERNINGPAIRS structure.
    *
    * @return ULONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_EXCEEDS_MAX_SEG_LENGTH
    *       - PMERR_HDC_BUSY
    */
   #define NGreGetPairKerningTable 0x000040A0L
   #define GreGetPairKerningTable(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PKERNINGPAIRS)(c),0L,NGreGetPairKerningTable)

   /* GreDeviceSetAVIOFont — no verifiable error list found; left undocumented. */
   #define NGreDeviceSetAVIOFont 0x000040A1L
   #define GreDeviceSetAVIOFont(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PFOCAFONT)(b),(ULONG)(LCID)(c),0L,NGreDeviceSetAVIOFont)

   /* GreDeviceSetAttributes — no verifiable error list found; left undocumented. */
   #define NGreDeviceSetAttributes 0x000060A2L
   #define GreDeviceSetAttributes(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(PBUNDLE)(e),0L,NGreDeviceSetAttributes)

   /* GreDeviceSetGlobalAttribute — no verifiable error list found; left undocumented. */
   #define NGreDeviceSetGlobalAttribute 0x000060A3L
   #define GreDeviceSetGlobalAttribute(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),0L,NGreDeviceSetGlobalAttribute)
#endif

#ifdef INCL_GRE_DEVMISC2
   /**
    * @brief Notify the driver of a clip change.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] prcl   Pointer to a RECTL structure.
    * @param[in] lMode  Mode.
    * @param[in] lOptions Options.
    *
    * @return ULONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_COORDINATE
    *       - PMERR_INV_HDC
    *       - PMERR_INV_RECT
    *       - PMERR_INV_REGION_CONTROL
    */
   #define NGreNotifyClipChange 0x000040A4L
   #define GreNotifyClipChange(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),(ULONG)(LONG)(c),(ULONG)(ULONG)(d),0L,NGreNotifyClipChange)

   /**
    * @brief Notify the driver of a transform change.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] lOptions Options.
    * @param[in] pData  Pointer to NOTIFYTRANSFORMDATA.
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
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_COORD_SPACE
    */
   #define NGreNotifyTransformChange 0x000040A5L
   #define GreNotifyTransformChange(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PNOTIFYTRANSFORMDATA)(c),0L,NGreNotifyTransformChange)

   /**
    * @brief Realize a font.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] lOptions Options.
    * @param[in] pAttrs Pointer to FATTRS.
    * @param[in] pFont  Pointer to FOCAFONT.
    *
    * @return ULONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_COORD_SPACE
    *       - PMERR_INV_HDC
    *       - PMERR_INV_LENGTH_OR_COUNT
    */
   #define NGreRealizeFont 0x000040A6L
   #define GreRealizeFont(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PFATTRS)(c),(ULONG)(PFOCAFONT)(d),0L,NGreRealizeFont)

   /**
    * @brief Erase the presentation space.
    *
    * @param[in] hdc Device context handle.
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
    */
   #define NGreErasePS 0x000040A7L
   #define GreErasePS(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreErasePS)

   /**
    * @brief Set the style ratio.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] pData Pointer to data.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreSetStyleRatio 0x000040A8L
   #define GreSetStyleRatio(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PBYTE)(b),0L,NGreSetStyleRatio)

   /**
    * @brief Query device font attributes.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] lCode Code.
    * @param[in] pMetrics Pointer to FONTMETRICS.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreDeviceQueryFontAttributes 0x000040A9L
   #define GreDeviceQueryFontAttributes(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PFONTMETRICS)(c),0L,NGreDeviceQueryFontAttributes)

   /**
    * @brief Query device fonts.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] lOptions Options.
    * @param[in] psz    Pointer to a string.
    * @param[in] pMetrics Pointer to FONTMETRICS.
    * @param[in] lCount Count.
    * @param[in] plCount Pointer to count.
    *
    * @return LONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_COORD_SPACE
    *       - PMERR_INV_HDC
    *       - PMERR_INV_LENGTH_OR_COUNT
    */
   #define NGreDeviceQueryFonts 0x000040AAL
   #define GreDeviceQueryFonts(a,b,c,d,e,f) (LONG) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PSZ)(c),(ULONG)(PFONTMETRICS)(d),(ULONG)(LONG)(e),(ULONG)(PLONG)(f),0L,NGreDeviceQueryFonts)

   /**
    * @brief Invalidate the visible region.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] lMode Mode.
    * @param[in] pBlock Pointer to a DC_BLOCK structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    */
   #define NGreDeviceInvalidateVisRegion 0x000040ABL
   #define GreDeviceInvalidateVisRegion(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PDC_BLOCK)(c),0L,NGreDeviceInvalidateVisRegion)

   /**
    * @brief Set font mapping flags.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] lFlags Flags.
    *
    * @return LONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_EXCEEDS_MAX_SEG_LENGTH
    *       - PMERR_INV_HDC
    */
   #define NGreSetFontMappingFlags 0x00004268L
   #define GreSetFontMappingFlags(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreSetFontMappingFlags)
#endif

#ifdef INCL_GRE_PICK
   /**
    * @brief Get the pick window.
    *
    * @param[in]  hdc  Device context handle.
    * @param[out] prcl Pointer to a RECTL structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreGetPickWindow 0x000040ACL
   #define GreGetPickWindow(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreGetPickWindow)

   /**
    * @brief Set the pick window.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] prcl Pointer to a RECTL structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_COORD_SPACE
    */
   #define NGreSetPickWindow 0x000040ADL
   #define GreSetPickWindow(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreSetPickWindow)
#endif

#ifdef INCL_GRE_DEVMISC3
   /**
    * @brief Reset bounds.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] lMode Mode.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreResetBounds 0x000040AEL
   #define GreResetBounds(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreResetBounds)

   /* GreGetBoundsData — no verifiable error list found; left undocumented. */
   #define NGreGetBoundsData 0x000040AFL
   #define GreGetBoundsData(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PRECTL)(c),0L,NGreGetBoundsData)

   /* GreAccumulateBounds — no verifiable error list found; left undocumented. */
   #define NGreAccumulateBounds 0x000040B0L
   #define GreAccumulateBounds(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreAccumulateBounds)

   /* GreGetExtraError — no verifiable error list found; left undocumented. */
   #define NGreGetExtraError 0x000040B1L
   #define GreGetExtraError(a) (FIXED) Gre32Entry3((ULONG)(HDC)(a),0L,NGreGetExtraError)

   /* GreSetExtraError — no verifiable error list found; left undocumented. */
   #define NGreSetExtraError 0x000040B2L
   #define GreSetExtraError(a,b) (VOID) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(FIXED)(b),0L,NGreSetExtraError)

   /**
    * @brief Get the code page.
    *
    * @param[in] hdc Device context handle.
    *
    * @return LONG code page.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    */
   #define NGreGetCodePage 0x000040B3L
   #define GreGetCodePage(a) (LONG) Gre32Entry3((ULONG)(HDC)(a),0L,NGreGetCodePage)

   /**
    * @brief Set the code page.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] lCodePage Code page.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_EXCEEDS_MAX_SEG_LENGTH
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_CODEPAGE
    */
   #define NGreSetCodePage 0x000040B4L
   #define GreSetCodePage(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreSetCodePage)

   /**
    * @brief Lock the device.
    *
    * @param[in] hdc Device context handle.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreLockDevice 0x000040B5L
   #define GreLockDevice(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreLockDevice)

   /**
    * @brief Unlock the device.
    *
    * @param[in] hdc Device context handle.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    */
   #define NGreUnlockDevice 0x000040B6L
   #define GreUnlockDevice(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreUnlockDevice)

   /**
    * @brief Notify the driver of device death.
    *
    * @param[in] hdc Device context handle.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreDeath 0x000040B7L
   #define GreDeath(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreDeath)

   /**
    * @brief Notify the driver of device resurrection.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] lMode Mode.
    * @param[in] pData Pointer to data.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreResurrection 0x000040B8L
   #define GreResurrection(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PBYTE)(c),0L,NGreResurrection)

   /**
    * @brief Get the DC origin.
    *
    * @param[in]  hdc  Device context handle.
    * @param[out] pptl Pointer to a POINTL structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreGetDCOrigin 0x000040BAL
   #define GreGetDCOrigin(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreGetDCOrigin)

   /**
    * @brief Set the DC origin.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] pptl Pointer to a POINTL structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreDeviceSetDCOrigin 0x000040BBL
   #define GreDeviceSetDCOrigin(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreDeviceSetDCOrigin)

   /**
    * @brief Get the line origin.
    *
    * @param[in]  hdc  Device context handle.
    * @param[out] pptl Pointer to a POINTL structure.
    *
    * @return LONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreGetLineOrigin 0x000040BCL
   #define GreGetLineOrigin(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreGetLineOrigin)

   /**
    * @brief Set the line origin.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] pptl Pointer to a POINTL structure.
    * @param[in] lStyle Style.
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
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_COORD_SPACE
    */
   #define NGreSetLineOrigin 0x000040BDL
   #define GreSetLineOrigin(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),0L,NGreSetLineOrigin)
#endif

#ifdef INCL_GRE_DEVMISC2
   /**
    * @brief Get the style ratio.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] pData Pointer to data.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreGetStyleRatio 0x000040BEL
   #define GreGetStyleRatio(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PBYTE)(b),0L,NGreGetStyleRatio)
#endif

#ifdef INCL_GRE_COLORTABLE
   /**
    * @brief Query color data.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] lStart Start index.
    * @param[in] plData Pointer to output data.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_INV_IN_AREA
    *       - PMERR_INV_IN_PATH
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    *       - PMERR_COL_TABLE_NOT_REALIZED
    */
   #define NGreQueryColorData 0x000060C3L
   #define GreQueryColorData(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PLONG)(c),0L,NGreQueryColorData)

   /**
    * @brief Query the logical color table.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] lStart Start index.
    * @param[in] lCount Count.
    * @param[in] lOptions Options.
    * @param[in] plArray Pointer to output array.
    *
    * @return LONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_COLOR_OPTIONS
    *       - PMERR_INV_COLOR_START_INDEX
    *       - PMERR_INV_HDC
    *       - PMERR_INV_LENGTH_OR_COUNT
    */
   #define NGreQueryLogColorTable 0x000060C4L
   #define GreQueryLogColorTable(a,b,c,d,e) (LONG) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PLONG)(e),0L,NGreQueryLogColorTable)

   /**
    * @brief Create a logical color table.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] lFormat  Format.
    * @param[in] lOptions Options.
    * @param[in] lStart   Start index.
    * @param[in] lCount   Count.
    * @param[in] plArray  Pointer to input array.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_COLOR_OPTIONS
    *       - PMERR_INV_COLOR_START_INDEX
    *       - PMERR_INV_LENGTH_OR_COUNT
    */
   #define NGreCreateLogColorTable 0x000060C5L
   #define GreCreateLogColorTable(a,b,c,d,e,f) (BOOL) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(LONG)(d),(ULONG)(LONG)(e),(ULONG)(PLONG)(f),0L,NGreCreateLogColorTable)

   /**
    * @brief Realize the color table.
    *
    * @param[in] hdc Device context handle.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COL_TABLE_NOT_REALIZABLE
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    *       - PMERR_INV_IN_AREA
    *       - PMERR_INV_IN_PATH
    *       - PMERR_REALIZE_NOT_SUPPORTED
    */
   #define NGreRealizeColorTable 0x000060C6L
   #define GreRealizeColorTable(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreRealizeColorTable)

   /**
    * @brief Unrealize the color table.
    *
    * @param[in] hdc Device context handle.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COL_TABLE_NOT_REALIZED
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    *       - PMERR_INV_IN_AREA
    */
   #define NGreUnrealizeColorTable 0x000060C7L
   #define GreUnrealizeColorTable(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreUnrealizeColorTable)

   /**
    * @brief Query real colors.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] lStart Start index.
    * @param[in] lCount Count.
    * @param[in] lOptions Options.
    * @param[in] plArray Pointer to output array.
    *
    * @return LONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_INV_IN_AREA
    *       - PMERR_INV_IN_PATH
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    *       - PMERR_COL_TABLE_NOT_REALIZED
    */
   #define NGreQueryRealColors 0x000040C8L
   #define GreQueryRealColors(a,b,c,d,e) (LONG) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PLONG)(e),0L,NGreQueryRealColors)

   /**
    * @brief Query the nearest color.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] lOptions Options.
    * @param[in] color Color.
    *
    * @return LONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_COLOR_OPTIONS
    *       - PMERR_INV_HDC
    *       - PMERR_INV_RGBCOLOR
    */
   #define NGreQueryNearestColor 0x000040C9L
   #define GreQueryNearestColor(a,b,c) (LONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(COLOR)(c),0L,NGreQueryNearestColor)

   /**
    * @brief Query the color index.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] lOptions Options.
    * @param[in] color  Color.
    *
    * @return LONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_COLOR_OPTIONS
    *       - PMERR_INV_HDC
    *       - PMERR_INV_RGBCOLOR
    */
   #define NGreQueryColorIndex 0x000060CAL
   #define GreQueryColorIndex(a,b,c) (LONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(COLOR)(c),0L,NGreQueryColorIndex)

   /**
    * @brief Query the RGB color.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] lOptions Options.
    * @param[in] color  Color.
    *
    * @return LONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_COLOR_DATA
    *       - PMERR_INV_COLOR_INDEX
    *       - PMERR_INV_COLOR_OPTIONS
    *       - PMERR_INV_HDC
    */
   #define NGreQueryRGBColor 0x000060CBL
   #define GreQueryRGBColor(a,b,c) (LONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(COLOR)(c),0L,NGreQueryRGBColor)
#endif

#ifdef INCL_GRE_DEVICE
   #ifndef NGreQueryDevFntMetrics
      /**
       * @brief Query device font metrics.
       *
       * @param[in] hdc    Device context handle.
       * @param[in] lCode  Code.
       * @param[in] lCount Count.
       *
       * @return ULONG result.
       *
       * @note If an error is returned, the handling routine must call
       *       WinSetErrorInfo. WinGetLastError may return one of the following
       *       errors:
       *       - PMERR_BASE_ERROR
       *       - PMERR_INSUFFICIENT_MEMORY
       *       - PMERR_INV_FONT_FILE_DATA
       */
      #define NGreQueryDevFntMetrics 0x000060CCL
      #define GreQueryDeviceFontMetrics(a,b,c) (ULONG)Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(LONG)(c),0L,NGreQueryDevFntMetrics)
   #endif
#endif

#ifdef INCL_GRE_PAX
   /* GreDeviceEnableYInversion — no verifiable error list found; left undocumented. */
   #define NGreDeviceEnableYInversion 0x000040CDL
   #define GreDeviceEnableYInversion(a,b) (ULONG) Gre32Entry4((ULONG)(a),(LONG)(b),0L,NGreDeviceEnableYInversion)
#endif

#ifdef INCL_GRE_DEVICE
   /**
    * @brief Query device bitmaps.
    *
    * @param[in] hdc     Device context handle.
    * @param[in] pFormat Pointer to a BITMAPFORMAT structure.
    * @param[in] lCount  Count.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_LENGTH_OR_COUNT
    */
   #define NGreQueryDeviceBitmaps 0x000040D0L
   #define GreQueryDeviceBitmaps(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PBITMAPFORMAT)(b),(ULONG)(LONG)(c),0L,NGreQueryDeviceBitmaps)

   /**
    * @brief Query device capabilities.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] lStart Start index.
    * @param[in] plData Pointer to output data.
    * @param[in] lCount Count.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_LENGTH_OR_COUNT
    *       - PMERR_INV_QUERY_ELEMENT_NO
    */
   #define NGreQueryDeviceCaps 0x000040D1L
   #define GreQueryDeviceCaps(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PLONG)(c),(ULONG)(LONG)(d),0L,NGreQueryDeviceCaps)

   /**
    * @brief Perform an escape.
    *
    * @param[in] hdc       Device context handle.
    * @param[in] lEscape   Escape code.
    * @param[in] lCount    Count.
    * @param[in] pInData   Pointer to input data.
    * @param[in] plOutCount Pointer to output count.
    * @param[in] pOutData  Pointer to output data.
    *
    * @return LONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_LENGTH_OR_COUNT
    */
   #define NGreEscape 0x000040D2L
   #define GreEscape(a,b,c,d,e,f) (LONG) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),(ULONG)(PBYTE)(d),(ULONG)(PLONG)(e),(ULONG)(PBYTE)(f),0L,NGreEscape)

   /**
    * @brief Query hardcopy capabilities.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] lStart Start index.
    * @param[in] lCount Count.
    * @param[in] pInfo  Pointer to a HCINFO structure.
    *
    * @return LONG result.
    *
    * @retval DQHC_ERROR  Error.
    *
    * @note If DQHC_ERROR is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_INV_LENGTH_OR_COUNT
    */
   #define NGreQueryHardcopyCaps 0x000040D3L
   #define GreQueryHardcopyCaps(a,b,c,d) (LONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(LONG)(c),(ULONG)(PHCINFO)(d),0L,NGreQueryHardcopyCaps)

   /**
    * @brief Query device resources.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] lType  Resource type.
    * @param[in] lIndex Resource index.
    *
    * @return PVOID pointer to the resource, or NULL on error.
    *
    * @retval NULL  Error.
    *
    * @note If NULL is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BASE_ERROR
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    */
   #define NGreQueryDevResource2 0x000040D4L
   #define GreQueryDevResource2(a,b,c) (PVOID) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),0L,NGreQueryDevResource2)

   /* GreUnloadPD — no verifiable error list found; left undocumented. */
   #define NGreUnloadPD 0x0000026EL
   #define GreUnloadPD(a,b) (PVOID) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreUnloadPD)
#endif

#ifdef INCL_GRE_SCREEN
   /**
    * @brief Open a screen change area.
    *
    * @param[in] hdc Device context handle.
    *
    * @return ULONG handle, or GPI_ERROR on error.
    *
    * @retval GPI_ERROR  Error.
    *
    * @note If GPI_ERROR is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_INV_HDC
    *       - PMERR_MEMORY_ALLOCATION_ERR
    */
   #define NGreOpenScreenChangeArea 0x00004012L
   #define GreOpenScreenChangeArea(a) (ULONG) Gre32Entry3((ULONG)(HDC)(a),0L,NGreOpenScreenChangeArea)

   /**
    * @brief Get a screen change area.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] lType Type.
    * @param[in] lIndex Index.
    *
    * @return ULONG handle, or 0 on error.
    *
    * @retval 0  Error.
    *
    * @note If 0 is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_INV_HDC
    *       - PMERR_NO_HANDLE
    */
   #define NGreGetScreenChangeArea 0x00004013L
   #define GreGetScreenChangeArea(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),0L,NGreGetScreenChangeArea)

   /**
    * @brief Close a screen change area.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] hsca Screen change area handle.
    *
    * @return ULONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_INV_HDC
    *       - PMERR_NO_HANDLE
    */
   #define NGreCloseScreenChangeArea 0x00004014L
   #define GreCloseScreenChangeArea(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreCloseScreenChangeArea)

   /**
    * @brief Get screen bits.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] lOptions Options.
    * @param[in] lCount   Count.
    * @param[in] lParam1  Parameter 1.
    * @param[in] lParam2  Parameter 2.
    *
    * @return ULONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_INV_DC_TYPE
    *       - PMERR_INV_FORMAT_CONTROL
    */
   #define NGreGetScreenBits 0x0000401DL
   #define GreGetScreenBits(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(ULONG)(e),0L,NGreGetScreenBits)

   /**
    * @brief Set screen bits.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] lOptions Options.
    * @param[in] lCount   Count.
    * @param[in] lParam   Parameter.
    *
    * @return ULONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BITMAP_NOT_SELECTED
    *       - PMERR_INCOMPAT_COLOR_FORMAT
    *       - PMERR_INV_DC_TYPE
    *       - PMERR_INV_IMAGE_DIMENSION
    *       - PMERR_INV_IN_AREA
    *       - PMERR_INV_IN_PATH
    *       - PMERR_INV_RECT
    */
   #define NGreSetScreenBits 0x0000401EL
   #define GreSetScreenBits(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),0L,NGreSetScreenBits)
#endif

#ifdef INCL_GRE_PALETTE
   /* GreDeviceCreatePalette — no verifiable error list found; left undocumented. */
   #define NGreDeviceCreatePalette 0x000040D5L
   #define GreDeviceCreatePalette(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPALETTEINFO)(b),(ULONG)(HDEVPAL)(c),0L,NGreDeviceCreatePalette)

   /* GreDeviceDeletePalette — no verifiable error list found; left undocumented. */
   #define NGreDeviceDeletePalette 0x000040D6L
   #define GreDeviceDeletePalette(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HDEVPAL)(b),0L,NGreDeviceDeletePalette)

   /* GreDeviceSetPaletteEntries — no verifiable error list found; left undocumented. */
   #define NGreDeviceSetPaletteEntries 0x000040D7L
   #define GreDeviceSetPaletteEntries(a,b,c,d,e,f) (ULONG) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(HDEVPAL)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(ULONG)(e),(ULONG)(PULONG)(f),0L,NGreDeviceSetPaletteEntries)

   /* GreDeviceAnimatePalette — no verifiable error list found; left undocumented. */
   #define NGreDeviceAnimatePalette 0x000040D8L
   #define GreDeviceAnimatePalette(a,b,c,d,e,f) (ULONG) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(HDEVPAL)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(ULONG)(e),(ULONG)(PULONG)(f),0L,NGreDeviceAnimatePalette)

   /* GreDeviceResizePalette — no verifiable error list found; left undocumented. */
   #define NGreDeviceResizePalette 0x000040D9L
   #define GreDeviceResizePalette(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HDEVPAL)(b),(ULONG)(ULONG)(c),0L,NGreDeviceResizePalette)

   /* GreRealizePalette — no verifiable error list found; left undocumented. */
   #define NGreRealizePalette 0x000040DAL
   #define GreRealizePalette(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PULONG)(c),0L,NGreRealizePalette)

   /* GreQueryHWPaletteInfo — no verifiable error list found; left undocumented. */
   #define NGreQueryHWPaletteInfo 0x000040DBL
   #define GreQueryHWPaletteInfo(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(PULONG)(d),0L,NGreQueryHWPaletteInfo)

   /* GreUpdateColors — no verifiable error list found; left undocumented. */
   #define NGreUpdateColors 0x000040DCL
   #define GreUpdateColors(a) (ULONG) Gre32Entry3((ULONG)(HDC)(a),0L,NGreUpdateColors)

   /* GreQueryPaletteRealization — no verifiable error list found; left undocumented. */
   #define NGreQueryPaletteRealization 0x000040DDL
   #define GreQueryPaletteRealization(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(PULONG)(d),0L,NGreQueryPaletteRealization)

   /* GreGetVisRects — no verifiable error list found; left undocumented. */
   #define NGreGetVisRects 0x000040DEL
   #define GreGetVisRects(a,b,c,d) (INT) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),(ULONG)(PRGNRECT)(c),(ULONG)(PRECTL)(d),0L,NGreGetVisRects)

   /* GreQueryNearestPaletteIndex — no verifiable error list found; left undocumented. */
   #define NGreQueryNearestPaletteIndex 0x00000267L
   #define GreQueryNearestPaletteIndex(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRGB2)(b),0L,NGreQueryNearestPaletteIndex)
#endif

#ifdef INCL_GRE_DCS
   /**
    * @brief Reset a device context.
    *
    * @param[in] hdc Device context handle.
    * @param[in] lOptions Options.
    *
    * @return ULONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BASE_ERROR
    *       - PMERR_INV_IN_AREA
    *       - PMERR_INV_IN_PATH
    *       - PMERR_HDC_BUSY
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_LENGTH_OR_COUNT
    *       - PMERR_INV_CODEPAGE
    *       - PMERR_EXCEEDS_MAX_SEG_LENGTH
    *       - PMERR_INV_RECT
    *       - PMERR_INV_COORDINATE
    *       - PMERR_INV_HDC
    *       - PMERR_INV_HRGN
    *       - PMERR_INV_REGION_CONTROL
    *       - PMERR_INV_HBITMAP
    *       - PMERR_INV_DC_TYPE
    *       - PMERR_INV_ID
    *       - PMERR_BITMAP_IS_SELECTED
    *       - PMERR_HBITMAP_BUSY
    *       - PMERR_INV_INFO_TABLE
    *       - PMERR_INV_BITMAP_DIMENSION
    *       - PMERR_INV_SCAN_START
    */
   #define NGreResetDC 0x00004202L
   #define GreResetDC(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreResetDC)

   /* GreResetDC2 — no verifiable error list found; left undocumented. */
   #define NGreResetDC2 0x0000426BL
   #define GreResetDC2(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreResetDC2)

   /**
    * @brief Get a handle.
    *
    * @param[in] hdc    Device context handle.
    * @param[in] lIndex Index.
    *
    * @return LONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    */
   #define NGreGetHandle 0x00004203L
   #define GreGetHandle(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreGetHandle)

   /**
    * @brief Set a handle.
    *
    * @param[in] hdc     Device context handle.
    * @param[in] lHandle Handle.
    * @param[in] lIndex  Index.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    */
   #define NGreSetHandle 0x00004204L
   #define GreSetHandle(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LHANDLE)(b),(ULONG)(ULONG)(c),0L,NGreSetHandle)

   /**
    * @brief Get process control.
    *
    * @param[in] hdc Device context handle.
    *
    * @return LONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    */
   #define NGreGetProcessControl 0x00004205L
   #define GreGetProcessControl(a) (LONG) Gre32Entry3((ULONG)(HDC)(a),0L,NGreGetProcessControl)

   /**
    * @brief Set process control.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] lFlags   Flags.
    * @param[in] lOptions Options.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    */
   #define NGreSetProcessControl 0x00004206L
   #define GreSetProcessControl(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),0L,NGreSetProcessControl)

   /**
    * @brief Save a device context.
    *
    * @param[in] hdc Device context handle.
    *
    * @return LONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BASE_ERROR
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_EXCEEDS_MAX_SEG_LENGTH
    *       - PMERR_HDC_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    */
   #define NGreSaveDC 0x00004207L
   #define GreSaveDC(a) (LONG) Gre32Entry3((ULONG)(HDC)(a),0L,NGreSaveDC)

   /**
    * @brief Restore a device context.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] lMode Mode.
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
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_EXCEEDS_MAX_SEG_LENGTH
    *       - PMERR_HDC_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_CODEPAGE
    */
   #define NGreRestoreDC 0x00004208L
   #define GreRestoreDC(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreRestoreDC)

   /* GreQueryDeviceNames — no verifiable error list found; left undocumented. */
   #define NGreQueryDeviceNames 0x00000209L
   #define GreQueryDeviceNames(a,b,c,d,e,f) (BOOL) Gre32Entry8((ULONG)(PSZ)(a),(ULONG)(PLONG)(b),(ULONG)(PSTR32)(c),(ULONG)(PSTR64)(d),(ULONG)(PLONG)(e),(ULONG)(PSTR16)(f),0L,NGreQueryDeviceNames)

   /* GreQueryEngineVersion — no verifiable error list found; left undocumented. */
   #define NGreQueryEngineVersion 0x0000020AL
   #define GreQueryEngineVersion() (LONG) Gre32Entry2(0L,NGreQueryEngineVersion)

   /**
    * @brief Set the DC owner.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] lOwner Owner value.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreSetDCOwner 0x0000420BL
   #define GreSetDCOwner(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreSetDCOwner)
#endif

#ifdef INCL_GRE_DEVSUPPORT
   /* GreQueryBitmapSelection — no verifiable error list found; left undocumented. */
   #define NGreQueryBitmapSelection 0x0000020CL
   #define GreQueryBitmapSelection(a) (HDC) Gre32Entry3((ULONG)(HBITMAP)(a),0L,NGreQueryBitmapSelection)

   /* GreGetBitmapInfoHeader — no verifiable error list found; left undocumented. */
   #define NGreGetBitmapInfoHeader 0x0000020DL
   #define GreGetBitmapInfoHeader(a,b) (BOOL) Gre32Entry4((ULONG)(HBITMAP)(a),(ULONG)(PBITMAPINFOHEADER2)(b),0L,NGreGetBitmapInfoHeader)

   /* GreConvertBitmapData — no verifiable error list found; left undocumented. */
   #define NGreConvertBitmapData 0x0000420EL
   #define GreConvertBitmapData(a,b,c,d,e,f,g) (ULONG) Gre32Entry9((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(PBITMAPINFO2)(d),(ULONG)(PBYTE)(e),(ULONG)(PBITMAPINFO2)(f),(ULONG)(PBYTE)(g),0L,NGreConvertBitmapData)

   /* GreInvalidateVisRegion — no verifiable error list found; left undocumented. */
   #define NGreInvalidateVisRegion 0x0000420FL
   #define GreInvalidateVisRegion(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PDC_BLOCK)(c),0L,NGreInvalidateVisRegion)

   /**
    * @brief Create a bitmap.
    *
    * @param[in] hdc      Device context handle.
    * @param[in] pBmih    Pointer to a BITMAPINFOHEADER2 structure.
    * @param[in] cOptions Options.
    * @param[in] pBits    Pointer to bitmap data.
    * @param[in] pBmi     Pointer to a BITMAPINFO2 structure.
    *
    * @return HBITMAP handle, or NULL on error.
    *
    * @retval NULL  Error.
    *
    * @note If NULL is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_LENGTH_OR_COUNT
    *       - PMERR_INV_HDC
    */
   #define NGreCreateBitmap 0x00004210L
   #define GreCreateBitmap(a,b,c,d,e) (HBITMAP) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(PBITMAPINFOHEADER2)(b),(ULONG)(ULONG)(c),(ULONG)(PBYTE)(d),(ULONG)(PBITMAPINFO2)(e),0L,NGreCreateBitmap)

   /**
    * @brief Delete a bitmap.
    *
    * @param[in] hbm Bitmap handle.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BITMAP_IS_SELECTED
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HBITMAP_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_HBITMAP
    */
   #define NGreDeleteBitmap 0x00000211L
   #define GreDeleteBitmap(a) (BOOL) Gre32Entry3((ULONG)(HBITMAP)(a),0L,NGreDeleteBitmap)

   /**
    * @brief Select a bitmap.
    *
    * @param[in] hdc Device context handle.
    * @param[in] hbm Bitmap handle.
    *
    * @return HBITMAP previous bitmap handle, or NULL on error.
    *
    * @retval NULL  Error.
    *
    * @note If NULL is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BITMAP_IS_SELECTED
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HBITMAP_BUSY
    *       - PMERR_HDC_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_BITMAP_DIMENSION
    *       - PMERR_INV_RECT
    */
   #define NGreSelectBitmap 0x00004212L
   #define GreSelectBitmap(a,b) (HBITMAP) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HBITMAP)(b),0L,NGreSelectBitmap)

   /**
    * @brief Get bitmap parameters.
    *
    * @param[in] hbm   Bitmap handle.
    * @param[in] pBmih Pointer to a BITMAPINFOHEADER structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BITMAP_IS_SELECTED
    *       - PMERR_HBITMAP_BUSY
    *       - PMERR_INV_HBITMAP
    */
   #define NGreGetBitmapParameters 0x00000213L
   #define GreGetBitmapParameters(a,b) (BOOL) Gre32Entry4((ULONG)(HBITMAP)(a),(ULONG)(PBITMAPINFOHEADER)(b),0L,NGreGetBitmapParameters)

   /**
    * @brief Get bitmap dimension.
    *
    * @param[in] hbm    Bitmap handle.
    * @param[in] pSizel Pointer to a SIZEL structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BITMAP_IS_SELECTED
    *       - PMERR_HBITMAP_BUSY
    *       - PMERR_INV_HBITMAP
    */
   #define NGreGetBitmapDimension 0x00000214L
   #define GreGetBitmapDimension(a,b) (BOOL) Gre32Entry4((ULONG)(HBITMAP)(a),(ULONG)(PSIZEL)(b),0L,NGreGetBitmapDimension)

   /**
    * @brief Set bitmap dimension.
    *
    * @param[in] hbm    Bitmap handle.
    * @param[in] pSizel Pointer to a SIZEL structure.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BITMAP_IS_SELECTED
    *       - PMERR_HBITMAP_BUSY
    *       - PMERR_INV_HBITMAP
    */
   #define NGreSetBitmapDimension 0x00000215L
   #define GreSetBitmapDimension(a,b) (BOOL) Gre32Entry4((ULONG)(HBITMAP)(a),(ULONG)(PSIZEL)(b),0L,NGreSetBitmapDimension)

   /* GreSetCursor — no verifiable error list found; left undocumented. */
   #define NGreSetCursor 0x00004216L
   #define GreSetCursor(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(HBITMAP)(c),0L,NGreSetCursor)

   /* GreGetAttributes — no verifiable error list found; left undocumented. */
   #define NGreGetAttributes 0x00004217L
   #define GreGetAttributes(a,b,c,d) (LONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(PBUNDLE)(d),0L,NGreGetAttributes)

   /* GreSetAttributes — no verifiable error list found; left undocumented. */
   #define NGreSetAttributes 0x00004218L
   #define GreSetAttributes(a,b,c,d,e) (BOOL) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(PBUNDLE)(e),0L,NGreSetAttributes)

   /* GreSetGlobalAttribute — no verifiable error list found; left undocumented. */
   #define NGreSetGlobalAttribute 0x00004219L
   #define GreSetGlobalAttribute(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),0L,NGreSetGlobalAttribute)

   /* GreSetBitmapOwner — no verifiable error list found; left undocumented. */
   #define NGreSetBitmapOwner 0x0000021AL
   #define GreSetBitmapOwner(a,b) (BOOL) Gre32Entry4((ULONG)(HBITMAP)(a),(ULONG)(ULONG)(b),0L,NGreSetBitmapOwner)

   /* GreGetDefaultAttributes — no verifiable error list found; left undocumented. */
   #define NGreGetDefaultAttributes 0x0000421BL
   #define GreGetDefaultAttributes(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(PBUNDLE)(d),0L,NGreGetDefaultAttributes)

   /* GreSetDefaultAttributes — no verifiable error list found; left undocumented. */
   #define NGreSetDefaultAttributes 0x0000421CL
   #define GreSetDefaultAttributes(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(PBUNDLE)(d),0L,NGreSetDefaultAttributes)

   /* GreGetDefaultArcParameters — no verifiable error list found; left undocumented. */
   #define NGreGetDefaultArcParameters 0x0000421DL
   #define GreGetDefaultArcParameters(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PARCPARAMS)(b),0L,NGreGetDefaultArcParameters)

   /* GreSetDefaultArcParameters — no verifiable error list found; left undocumented. */
   #define NGreSetDefaultArcParameters 0x0000421EL
   #define GreSetDefaultArcParameters(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PARCPARAMS)(b),0L,NGreSetDefaultArcParameters)

   /* GreGetDefaultViewingLimits — no verifiable error list found; left undocumented. */
   #define NGreGetDefaultViewingLimits 0x0000421FL
   #define GreGetDefaultViewingLimits(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreGetDefaultViewingLimits)

   /* GreSetDefaultViewingLimits — no verifiable error list found; left undocumented. */
   #define NGreSetDefaultViewingLimits 0x00004220L
   #define GreSetDefaultViewingLimits(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreSetDefaultViewingLimits)

   /* GreInitializeAttributes — no verifiable error list found; left undocumented. */
   #define NGreInitializeAttributes 0x00004221L
   #define GreInitializeAttributes(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreInitializeAttributes)
#endif

#ifdef INCL_GRE_SETID
   /**
    * @brief Delete a set ID.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] lcid  Locale ID.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_CODEPAGE
    *       - PMERR_SETID_NOT_FOUND
    */
   #define NGreDeleteSetId 0x00004222L
   #define GreDeleteSetId(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LCID)(b),0L,NGreDeleteSetId)

   /**
    * @brief Query the number of set IDs.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] lType Type.
    *
    * @return LONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    *       - PMERR_INV_SETID
    *       - PMERR_INV_SETID_TYPE
    */
   #define NGreQueryNumberSetIds 0x00004223L
   #define GreQueryNumberSetIds(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreQueryNumberSetIds)

   /**
    * @brief Query set IDs.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] lCount Count.
    * @param[in] plTypes Pointer to types.
    * @param[in] pStr   Pointer to string.
    * @param[in] plcid  Pointer to locale ID.
    * @param[in] lOptions Options.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    *       - PMERR_INV_LENGTH_OR_COUNT
    *       - PMERR_INV_SETID
    *       - PMERR_INV_SETID_TYPE
    */
   #define NGreQuerySetIds 0x00004224L
   #define GreQuerySetIds(a,b,c,d,e,f) (BOOL) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PLONG)(c),(ULONG)(PSTR8)(d),(ULONG)(PLCID)(e),(ULONG)(ULONG)(f),0L,NGreQuerySetIds)
#endif

#ifdef INCL_GRE_LCID
   /**
    * @brief Query a bitmap handle.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] lcid  Locale ID.
    *
    * @return HBITMAP handle, or NULL on error.
    *
    * @retval NULL  Error.
    *
    * @note If NULL is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BITMAP_NOT_SELECTED
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HDC
    *       - PMERR_INV_SETID
    */
   #define NGreQueryBitmapHandle 0x00000225L
   #define GreQueryBitmapHandle(a,b) (HBITMAP) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LCID)(b),0L,NGreQueryBitmapHandle)

   /**
    * @brief Set a bitmap ID.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] hbm   Bitmap handle.
    * @param[in] lcid  Locale ID.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BITMAP_IS_SELECTED
    *       - PMERR_HBITMAP_BUSY
    *       - PMERR_HDC_BUSY
    *       - PMERR_INV_HBITMAP
    *       - PMERR_INV_HDC
    */
   #define NGreSetBitmapID 0x00004226L
   #define GreSetBitmapID(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HBITMAP)(b),(ULONG)(LCID)(c),0L,NGreSetBitmapID)

   /**
    * @brief Copy DC load data.
    *
    * @param[in] hdc    Destination device context handle.
    * @param[in] lCmd   Command.
    * @param[in] hdcSrc Source device context handle.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_FONT_NOT_LOADED
    *       - PMERR_KERNING_NOT_SUPPORTED
    */
   #define NGreCopyDCLoadData 0x00004227L
   #define GreCopyDCLoadData(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(HDC)(c),0L,NGreCopyDCLoadData)
#endif

#ifdef INCL_GRE_SETID
   /* GreQuerySetIdUsage — no verifiable error list found; left undocumented. */
   #define NGreQuerySetIdUsage 0x00004228L
   #define GreQuerySetIdUsage(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LCID)(b),0L,NGreQuerySetIdUsage)
#endif

#ifdef INCL_GRE_FONTS
   /* GreQueryFullFontFileDescriptions — no verifiable error list found; left undocumented. */
   #define NGreQueryFullFontFileDescriptions 0x0000022BL
   #define GreQueryFullFontFileDescriptions(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(PSZ)(a),(ULONG)(PULONG)(b),(ULONG)(PVOID)(c),(ULONG)(PLONG)(d),0L,NGreQueryFullFontFileDescriptions)

   /* GreQueryLogicalFont — no verifiable error list found; left undocumented. */
   #define NGreQueryLogicalFont 0x0000422CL
   #define GreQueryLogicalFont(a,b,c,d,e) (BOOL) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(LCID)(b),(ULONG)(PSTR8)(c),(ULONG)(PFATTRS)(d),(ULONG)(LONG)(e),0L,NGreQueryLogicalFont)

   /* GreCreateLogicalFont — no verifiable error list found; left undocumented. */
   #define NGreCreateLogicalFont 0x0000422DL
   #define GreCreateLogicalFont(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(LCID)(b),(ULONG)(PSTR8)(c),(ULONG)(PFATTRS)(d),0L,NGreCreateLogicalFont)

   /**
    * @brief Load a font.
    *
    * @param[in] pszFilename Pointer to the font file name.
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
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_FONT_FILE_DATA
    */
   #define NGreLoadFont 0x0000022EL
   #define GreLoadFont(a) (BOOL) Gre32Entry3((ULONG)(PSZ)(a),0L,NGreLoadFont)

   /**
    * @brief Unload a font.
    *
    * @param[in] pszFilename Pointer to the font file name.
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
    *       - PMERR_FONT_FILE_NOT_LOADED
    */
   #define NGreUnloadFont 0x0000022FL
   #define GreUnloadFont(a) (BOOL) Gre32Entry3((ULONG)(PSZ)(a),0L,NGreUnloadFont)

   /**
    * @brief Query fonts.
    *
    * @param[in] hdc        Device context handle.
    * @param[in] lOptions   Options.
    * @param[in] pszFaceName Pointer to face name.
    * @param[in] pMetrics   Pointer to FONTMETRICS.
    * @param[in] lCount     Count.
    * @param[in] plCount    Pointer to count.
    *
    * @return LONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    */
   #define NGreQueryFonts 0x00004230L
   #define GreQueryFonts(a,b,c,d,e,f) (LONG) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PSZ)(c),(ULONG)(PFONTMETRICS)(d),(ULONG)(LONG)(e),(ULONG)(PLONG)(f),0L,NGreQueryFonts)

   /**
    * @brief Query font attributes.
    *
    * @param[in] hdc       Device context handle.
    * @param[in] lCode     Code.
    * @param[in] pMetrics  Pointer to FONTMETRICS.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_COORDINATE_OVERFLOW
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_HDC_BUSY
    */
   #define NGreQueryFontAttributes 0x00004231L
   #define GreQueryFontAttributes(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PFONTMETRICS)(c),0L,NGreQueryFontAttributes)

   /* GreInstallIFI — no verifiable error list found; left undocumented. */
   #define NGreInstallIFI 0x00000232L
   #define GreInstallIFI(a,b,c,d) (LONG) Gre32Entry6((ULONG)(ULONG)(a),(ULONG)(PSZ)(b),(ULONG)(PSZ)(c),(ULONG)(PSZ)(d),0L,NGreInstallIFI)

   /* GreLoadPublicFont — no verifiable error list found; left undocumented. */
   #define NGreLoadPublicFont 0x00000233L
   #define GreLoadPublicFont(a) (BOOL) Gre32Entry3((ULONG)(PSZ)(a),0L,NGreLoadPublicFont)

   /* GreUnLoadPublicFont — no verifiable error list found; left undocumented. */
   #define NGreUnLoadPublicFont 0x00000234L
   #define GreUnLoadPublicFont(a) (BOOL) Gre32Entry3((ULONG)(PSZ)(a),0L,NGreUnLoadPublicFont)

   /**
    * @brief Query the code page vector.
    *
    * @param[in] lCodePage Code page.
    *
    * @return ULONG result.
    *
    * @retval GPI_ERROR  Error.
    *
    * @note If GPI_ERROR is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_EXCEEDS_MAX_SEG_LENGTH
    *       - PMERR_INSUFFICIENT_MEMORY
    *       - PMERR_INV_CODEPAGE
    */
   #define NGreQueryCodePageVector 0x00000235L
   #define GreQueryCodePageVector(a) (ULONG) Gre32Entry3((ULONG)(ULONG)(a),0L,NGreQueryCodePageVector)

   /**
    * @brief Query font file descriptions.
    *
    * @param[in] pszFilename Pointer to the font file name.
    * @param[in] plCount     Pointer to count.
    * @param[in] pDescs      Pointer to FFDESCS.
    *
    * @return ULONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_BASE_ERROR
    *       - PMERR_INV_FONT_FILE_DATA
    */
   #define NGreQueryFontFileDescriptions 0x00000236L
   #define GreQueryFontFileDescriptions(a,b,c) (ULONG) Gre32Entry5((ULONG)(PSZ)(a),(ULONG)(PLONG)(b),(ULONG)(PFFDESCS)(c),0L,NGreQueryFontFileDescriptions)

   /* GreQueryFaceString — no verifiable error list found; left undocumented. */
   #define NGreQueryFaceString 0x00004237L
   #define GreQueryFaceString(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(PSZ)(b),(ULONG)(PFACENAMEDESC)(c),(ULONG)(ULONG)(d),(ULONG)(PSZ)(e),0L,NGreQueryFaceString)

   /* GreQueryFontAction — no verifiable error list found; left undocumented. */
   #define NGreQueryFontAction 0x00000238L
   #define GreQueryFontAction(a) (ULONG) Gre32Entry3((ULONG)(ULONG)(a),0L,NGreQueryFontAction)

   /* GreQueryCodePageObject — no verifiable error list found; left undocumented. */
   #define NGreQueryCodePageObject 0x00000255L
   #define GreQueryCodePageObject(a,b,c) (ULONG)Gre32Entry5((ULONG)(a),(ULONG)(PSZ)(b),(ULONG)(PCODEPAGEOBJECT *)(c),0L,NGreQueryCodePageObject)
#endif

#ifdef INCL_GRE_JOURNALING
   /* GreAccumulateJournalFile — no verifiable error list found; left undocumented. */
   #define NGreAccumulateJournalFile 0x00000239L
   #define GreAccumulateJournalFile(a) (ULONG) Gre32Entry3((ULONG)(ULONG)(a),0L,NGreAccumulateJournalFile)

   /* GreCreateJournalFile — no verifiable error list found; left undocumented. */
   #define NGreCreateJournalFile 0x0000023AL
   #define GreCreateJournalFile(a,b,c) (ULONG) Gre32Entry5((ULONG)(PSZ)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),0L,NGreCreateJournalFile)

   /* GreDeleteJournalFile — no verifiable error list found; left undocumented. */
   #define NGreDeleteJournalFile 0x0000023BL
   #define GreDeleteJournalFile(a) (ULONG) Gre32Entry3((ULONG)(LHANDLE)(a),0L,NGreDeleteJournalFile)

   /* GreStartJournalFile — no verifiable error list found; left undocumented. */
   #define NGreStartJournalFile 0x0000023CL
   #define GreStartJournalFile(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LHANDLE)(b),0L,NGreStartJournalFile)

   /* GreStopJournalFile — no verifiable error list found; left undocumented. */
   #define NGreStopJournalFile 0x0000023DL
   #define GreStopJournalFile(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LHANDLE)(b),0L,NGreStopJournalFile)

   /* GrePlayJournalFile — no verifiable error list found; left undocumented. */
   #define NGrePlayJournalFile 0x0000023EL
   #define GrePlayJournalFile(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LHANDLE)(b),0L,NGrePlayJournalFile)

   /* GreOpenJournalFile — no verifiable error list found; left undocumented. */
   #define NGreOpenJournalFile 0x0000023FL
   #define GreOpenJournalFile(a,b,c) (ULONG) Gre32Entry5((ULONG)(PSZ)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),0L,NGreOpenJournalFile)
#endif

#ifdef INCL_GRE_HFONT
   /* GreCreateFontHandle — no verifiable error list found; left undocumented. */
   #define NGreCreateFontHandle 0x00000250L
   #define GreCreateFontHandle(a,b,c) (HFONT) Gre32Entry5((ULONG)(ULONG)(a),(ULONG)(PSTR8)(b),(ULONG)(PFATTRS)(c),0L,NGreCreateFontHandle)

   /* GreQueryFontHandle — no verifiable error list found; left undocumented. */
   #define NGreQueryFontHandle 0x00000251L
   #define GreQueryFontHandle(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HFONT)(a),(ULONG)(PULONG)(b),(ULONG)(PSTR8)(c),(ULONG)(PFATTRS)(d),(ULONG)(ULONG)(e),0L,NGreQueryFontHandle)

   /* GreSelectFontHandle — no verifiable error list found; left undocumented. */
   #define NGreSelectFontHandle 0x00004252L
   #define GreSelectFontHandle(a,b) (HFONT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HFONT)(b),0L,NGreSelectFontHandle)

   /* GreDeleteFontHandle — no verifiable error list found; left undocumented. */
   #define NGreDeleteFontHandle 0x00000253L
   #define GreDeleteFontHandle(a) (ULONG) Gre32Entry3((ULONG)(HFONT)(a),0L,NGreDeleteFontHandle)

   /* GreSetFontHandleOwner — no verifiable error list found; left undocumented. */
   #define NGreSetFontHandleOwner 0x00000254L
   #define GreSetFontHandleOwner(a,b) (ULONG) Gre32Entry4((ULONG)(HFONT)(a),(ULONG)(ULONG)(b),0L,NGreSetFontHandleOwner)
#endif

#ifdef INCL_GRE_PAX
   /* GrePaxGetPolyFillMode — no verifiable error list found; left undocumented. */
   #define NGrePaxGetPolyFillMode  0x0000425CL
   #define GrePaxGetPolyFillMode(a) (ULONG) Gre32Entry3((ULONG)(a),0L,NGrePaxGetPolyFillMode)

   /* GrePaxSetPolyFillMode — no verifiable error list found; left undocumented. */
   #define NGrePaxSetPolyFillMode  0x0000425DL
   #define GrePaxSetPolyFillMode(a,b) (ULONG) Gre32Entry4((ULONG)(a),(ULONG)(b),0L,NGrePaxSetPolyFillMode)

   /* GreQueryMitreLimit — no verifiable error list found; left undocumented. */
   #define NGreQueryMitreLimit  0x0000425EL
   #define GreQueryMitreLimit(a) (FIXED) Gre32Entry3((ULONG)(a),0L,NGreQueryMitreLimit)

   /* GreSetMitreLimit — no verifiable error list found; left undocumented. */
   #define NGreSetMitreLimit  0x0000425FL
   #define GreSetMitreLimit(a,b) (FIXED) Gre32Entry4((ULONG)(a),(ULONG)(b),0L,NGreSetMitreLimit)

   /* GreEnableYInversion — no verifiable error list found; left undocumented. */
   #define NGreEnableYInversion 0x00004260L
   #define GreEnableYInversion(a,b) (ULONG) Gre32Entry4((ULONG)(a),(LONG)(b),0L,NGreEnableYInversion)

   /* GreQueryYInversion — no verifiable error list found; left undocumented. */
   #define NGreQueryYInversion 0x00004261L
   #define GreQueryYInversion(a) (ULONG) Gre32Entry3((ULONG)(a),0L,NGreQueryYInversion)

   /* GreQueryPath — no verifiable error list found; left undocumented. */
   #define NGreQueryPath  0x00004262L
   #define GreQueryPath(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(a),(ULONG)(b),(ULONG)(c),(ULONG)(d),0L,NGreQueryPath)

   /* GreAllocateDCData — no verifiable error list found; left undocumented. */
   #define NGreAllocateDCData  0x00004263L
   #define GreAllocateDCData(a,b) (ULONG) Gre32Entry4((ULONG)(a),(ULONG)(b),0L,NGreAllocateDCData)

   /* GreQueryDCData — no verifiable error list found; left undocumented. */
   #define NGreQueryDCData  0x00004264L
   #define GreQueryDCData(a) (ULONG) Gre32Entry3((ULONG)(a),0L,NGreQueryDCData)

   /* GreQueryRasterCaps — no verifiable error list found; left undocumented. */
   #define NGreQueryRasterCaps 0x00000265L
   #define GreQueryRasterCaps(a) (ULONG) Gre32Entry3((ULONG)(a), 0L, NGreQueryRasterCaps)

   /* GrePaxStrokeAndFillPath — no verifiable error list found; left undocumented. */
   #define NGrePaxStrokeAndFillPath 0x00004266L
   #define GrePaxStrokeAndFillPath(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(a),(ULONG)(b),(ULONG)(c),(ULONG)(d), 0L, NGrePaxStrokeAndFillPath)
#endif

#ifdef __cplusplus
      }
#endif
