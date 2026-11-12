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

   /* GreDevicePolygonSet — no verifiable error list found; left undocumented. */
   #define NGreDevicePolygonSet 0x00004059L
   #define GreDevicePolygonSet(a,b) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOLYSET)(b),0L,NGreDevicePolygonSet)

   /* GrePolygon3d — no verifiable error list found; left undocumented. */
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

   /* GreOval — no verifiable error list found; left undocumented. */
   #define NGreOval 0x0000400DL
   #define GreOval(a,b,c,d,e) (INT) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(FIXED)(c),(ULONG)(FIXED)(d),(ULONG)(e),0L,NGreOval)

   /* GreCookWholePath — no verifiable error list found; left undocumented. */
   #define NGreCookWholePath 0x0000400EL
   #define GreCookWholePath(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPATH)(b),0L,NGreCookWholePath)

   /* GreCookPathCurves — no verifiable error list found; left undocumented. */
   #define NGreCookPathCurves 0x0000400FL
   #define GreCookPathCurves(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPATH)(b),(ULONG)(PPIPELINEINFO)(c),0L,NGreCookPathCurves)

   /* GreRenderPath — no verifiable error list found; left undocumented. */
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

   /* GreFrameRegion — no verifiable error list found; left undocumented. */
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

   /* GreDeviceCreateBitmap — no verifiable error list found; left undocumented. */
   #define NGreDeviceCreateBitmap 0x00006023L
   #define GreDeviceCreateBitmap(a,b,c,d,e) (HBITMAP) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(PBITMAPINFOHEADER2)(b),(ULONG)(ULONG)(c),(ULONG)(PBYTE)(d),(ULONG)(PBITMAPINFO2)(e),0L,NGreDeviceCreateBitmap)

   /* GreDeviceDeleteBitmap — no verifiable error list found; left undocumented. */
   #define NGreDeviceDeleteBitmap 0x00004024L
   #define GreDeviceDeleteBitmap(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(HBITMAP)(b),(ULONG)(PDELETERETURN)(c),(ULONG)(ULONG)(d),0L,NGreDeviceDeleteBitmap)

   /* GreDeviceSelectBitmap — no verifiable error list found; left undocumented. */
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

   /* GreImageData — no verifiable error list found; left undocumented. */
   #define NGreImageData 0x00004029L
   #define GreImageData(a,b,c,d) (INT) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PBYTE)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),0L,NGreImageData)

   /* GreScanLR — no verifiable error list found; left undocumented. */
   #define NGreScanLR 0x0000602AL
   #define GreScanLR(a,b,c,d,e,f) (INT) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PLONG)(e),(ULONG)(PLONG)(f),0L,NGreScanLR)

   /* GreFloodFill — no verifiable error list found; left undocumented. */
   #define NGreFloodFill 0x0000602BL
   #define GreFloodFill(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(LONG)(c),0L,NGreFloodFill)

   /* GreSaveScreenBits — no verifiable error list found; left undocumented. */
   #define NGreSaveScreenBits 0x0000402CL
   #define GreSaveScreenBits(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreSaveScreenBits)

   /* GreRestoreScreenBits — no verifiable error list found; left undocumented. */
   #define NGreRestoreScreenBits 0x0000402DL
   #define GreRestoreScreenBits(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(LHANDLE)(b),(ULONG)(PRECTL)(c),(ULONG)(ULONG)(d),0L,NGreRestoreScreenBits)

   /* GreDrawBorder — no verifiable error list found; left undocumented. */
   #define NGreDrawBorder 0x0000602EL
   #define GreDrawBorder(a,b,c,d,e,f,g) (BOOL) Gre32Entry9((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(COLOR)(e),(ULONG)(COLOR)(f),(ULONG)(ULONG)(g),0L,NGreDrawBorder)

   /* GreDeviceSetCursor — no verifiable error list found; left undocumented. */
   #define NGreDeviceSetCursor 0x0000402FL
   #define GreDeviceSetCursor(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(HBITMAP)(c),0L,NGreDeviceSetCursor)

   /* GreGetBitmapBits — no verifiable error list found; left undocumented. */
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
   /* GreOpenDC — no verifiable error list found; left undocumented. */
   #define NGreOpenDC 0x00000200L
   #define GreOpenDC(a,b,c,d,e) (HDC) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PSZ)(c),(ULONG)(LONG)(d),(ULONG)(PDEVOPENDATA)(e),0L,NGreOpenDC)

   /* GreCloseDC — no verifiable error list found; left undocumented. */
   #define NGreCloseDC 0x00004201L
   #define GreCloseDC(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreCloseDC)
#endif

#ifdef INCL_GRE_INKPATH
   /* GreBeginInkPath — no verifiable error list found; left undocumented. */
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
   /* GreCreatePalette — no verifiable error list found; left undocumented. */
   #define NGreCreatePalette 0x00000240L
   #define GreCreatePalette(a,b,c,d) (HPAL) Gre32Entry6((ULONG)(ULONG)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(PULONG)(d),0L,NGreCreatePalette)

   /* GreDeletePalette — no verifiable error list found; left undocumented. */
   #define NGreDeletePalette 0x00000241L
   #define GreDeletePalette(a) (ULONG) Gre32Entry3((ULONG)(HPAL)(a),0L,NGreDeletePalette)

   /* GreSelectPalette — no verifiable error list found; left undocumented. */
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

   /* GreSetColorCursor — no verifiable error list found; left undocumented. */
   #define NGreSetColorCursor 0x00004032L
   #define GreSetColorCursor(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTERINFO)(b),0L,NGreSetColorCursor)
#endif

#ifdef INCL_GRE_SCANS
   /* GrePolyScanline — no verifiable error list found; left undocumented. */
   #define NGrePolyScanline 0x0000401CL
   #define GrePolyScanline(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PSCANDATA)(b),0L,NGrePolyScanline)
#endif

#ifdef INCL_GRE_STRINGS
   /**
    * @brief Output a character string.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] lCount Count.
    * @param[in] pch  Pointer to character string.
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

   /* GreCharStringPos — no verifiable error list found; left undocumented. */
   #define NGreCharStringPos 0x00007036L
   #define GreCharStringPos(a,b,c,d,e,f,g,h) (INT) Gre32Entry10((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(PRECTL)(c),(ULONG)(ULONG)(d),(ULONG)(LONG)(e),(ULONG)(PCH)(f),(ULONG)(PLONG)(g),(ULONG)(PCSP_INFO)(h),0L,NGreCharStringPos)

   /**
    * @brief Query the text box.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] lCount Count.
    * @param[in] pch  Pointer to character string.
    * @param[in] lOptions Options.
    * @param[in] pptl Pointer to output POINTL array.
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

   /* GreQueryCharPositions — no verifiable error list found; left undocumented. */
   #define NGreQueryCharPositions 0x00005038L
   #define GreQueryCharPositions(a,b,c,d,e,f,g) (ULONG) Gre32Entry9((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(ULONG)(c),(ULONG)(LONG)(d),(ULONG)(PCH)(e),(ULONG)(PLONG)(f),(ULONG)(PPOINTL)(g),0L,NGreQueryCharPositions)

   /* GreQueryWidthTable — no verifiable error list found; left undocumented. */
   #define NGreQueryWidthTable 0x00005039L
   #define GreQueryWidthTable(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(LONG)(c),(ULONG)(PLONG)(d),0L,NGreQueryWidthTable)

   /* GreQueryCharOutline — no verifiable error list found; left undocumented. */
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
    * @param[in] hdc  Device context handle.
    * @param[in] pptl Pointer to a POINTL array.
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
   /* GreCharRect — no verifiable error list found; left undocumented. */
   #define NGreCharRect 0x0000403BL
   #define GreCharRect(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(VioPresentationSpace *)(b),(ULONG)(LPGridRectRef)(c),0L,NGreCharRect)

   /* GreCharStr — no verifiable error list found; left undocumented. */
   #define NGreCharStr 0x0000403CL
   #define GreCharStr(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(VioPresentationSpace *)(b),(ULONG)(LPGridStringRef)(c),0L,NGreCharStr)

   /* GreScrollRect — no verifiable error list found; left undocumented. */
   #define NGreScrollRect 0x0000403DL
   #define GreScrollRect(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(VioPresentationSpace *)(b),(ULONG)(LPScrollRectRef)(c),0L,NGreScrollRect)

   /* GreUpdateCursor — no verifiable error list found; left undocumented. */
   #define NGreUpdateCursor 0x0000403EL
   #define GreUpdateCursor(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(VioPresentationSpace *)(b),0L,NGreUpdateCursor)
#endif

#ifdef INCL_GRE_PATHS
   /* GreBeginArea — no verifiable error list found; left undocumented. */
   #define NGreBeginArea 0x00004046L
   #define GreBeginArea(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreBeginArea)

   /* GreEndArea — no verifiable error list found; left undocumented. */
   #define NGreEndArea 0x00004047L
   #define GreEndArea(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreEndArea)

   /* GreBeginPath — no verifiable error list found; left undocumented. */
   #define NGreBeginPath 0x00004048L
   #define GreBeginPath(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PHID)(b),0L,NGreBeginPath)

   /* GreEndPath — no verifiable error list found; left undocumented. */
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
   /* GreGetClipBox — no verifiable error list found; left undocumented. */
   #define NGreGetClipBox 0x0000406EL
   #define GreGetClipBox(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreGetClipBox)

   /* GreGetClipRects — no verifiable error list found; left undocumented. */
   #define NGreGetClipRects 0x0000406FL
   #define GreGetClipRects(a,b,c,d) (INT) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),(ULONG)(PRGNRECT)(c),(ULONG)(PRECTL)(d),0L,NGreGetClipRects)

   /* GreOffsetClipRegion — no verifiable error list found; left undocumented. */
   #define NGreOffsetClipRegion 0x00004070L
   #define GreOffsetClipRegion(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreOffsetClipRegion)

   /* GrePtVisible — no verifiable error list found; left undocumented. */
   #define NGrePtVisible 0x00004071L
   #define GrePtVisible(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGrePtVisible)

   /* GreRectVisible — no verifiable error list found; left undocumented. */
   #define NGreRectVisible 0x00004072L
   #define GreRectVisible(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreRectVisible)

   /* GreQueryClipRegion — no verifiable error list found; left undocumented. */
   #define NGreQueryClipRegion 0x00004073L
   #define GreQueryClipRegion(a) (HRGN) Gre32Entry3((ULONG)(HDC)(a),0L,NGreQueryClipRegion)

   /* GreSelectClipRegion — no verifiable error list found; left undocumented. */
   #define NGreSelectClipRegion 0x00004074L
   #define GreSelectClipRegion(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PHRGN)(c),0L,NGreSelectClipRegion)

   /* GreIntersectClipRectangle — no verifiable error list found; left undocumented. */
   #define NGreIntersectClipRectangle 0x00004075L
   #define GreIntersectClipRectangle(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreIntersectClipRectangle)

   /* GreExcludeClipRectangle — no verifiable error list found; left undocumented. */
   #define NGreExcludeClipRectangle 0x00004076L
   #define GreExcludeClipRectangle(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreExcludeClipRectangle)

   /* GreSetXformRect — no verifiable error list found; left undocumented. */
   #define NGreSetXformRect 0x00004077L
   #define GreSetXformRect(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreSetXformRect)

   /* GreSaveRegion — no verifiable error list found; left undocumented. */
   #define NGreSaveRegion 0x0000407BL
   #define GreSaveRegion(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreSaveRegion)

   /* GreRestoreRegion — no verifiable error list found; left undocumented. */
   #define NGreRestoreRegion 0x0000407CL
   #define GreRestoreRegion(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreRestoreRegion)

   /* GreClipPathCurves — no verifiable error list found; left undocumented. */
   #define NGreClipPathCurves 0x0000407DL
   #define GreClipPathCurves(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPATH)(b),(ULONG)(PPIPELINEINFO)(c),0L,NGreClipPathCurves)

   /* GreSelectPathRegion — no verifiable error list found; left undocumented. */
   #define NGreSelectPathRegion 0x0000407EL
   #define GreSelectPathRegion(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HRGN)(b),0L,NGreSelectPathRegion)

   /* GreRegionSelectBitmap — no verifiable error list found; left undocumented. */
   #define NGreRegionSelectBitmap 0x0000407FL
   #define GreRegionSelectBitmap(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HBITMAP)(b),0L,NGreRegionSelectBitmap)

   /* GreCopyClipRegion — no verifiable error list found; left undocumented. */
   #define NGreCopyClipRegion 0x00004080L
   #define GreCopyClipRegion(a,b,c,d) (INT) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PRECTL)(c),(ULONG)(ULONG)(d),0L,NGreCopyClipRegion)

   /* GreSetupDC — no verifiable error list found; left undocumented. */
   #define NGreSetupDC 0x00004081L
   #define GreSetupDC(a,b,c,d,e,f) (BOOL) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PRECTL)(e),(ULONG)(ULONG)(f),0L,NGreSetupDC)
#endif

#ifdef INCL_GRE_XFORMS
   /* GreGetPageUnits — no verifiable error list found; left undocumented. */
   #define NGreGetPageUnits 0x00004083L
   #define GreGetPageUnits(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PULONG)(b),0L,NGreGetPageUnits)

   /* GreSetPageUnits — no verifiable error list found; left undocumented. */
   #define NGreSetPageUnits 0x00004084L
   #define GreSetPageUnits(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),0L,NGreSetPageUnits)

   /* GreGetModelXform — no verifiable error list found; left undocumented. */
   #define NGreGetModelXform 0x00004085L
   #define GreGetModelXform(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),0L,NGreGetModelXform)

   /* GreSetModelXform — no verifiable error list found; left undocumented. */
   #define NGreSetModelXform 0x00004086L
   #define GreSetModelXform(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),(ULONG)(ULONG)(c),0L,NGreSetModelXform)

   /* GreGetWindowViewportXform — no verifiable error list found; left undocumented. */
   #define NGreGetWindowViewportXform 0x00004087L
   #define GreGetWindowViewportXform(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),0L,NGreGetWindowViewportXform)

   /* GreSetWindowViewportXform — no verifiable error list found; left undocumented. */
   #define NGreSetWindowViewportXform 0x00004088L
   #define GreSetWindowViewportXform(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),(ULONG)(ULONG)(c),0L,NGreSetWindowViewportXform)

   /* GreGetGlobalViewingXform — no verifiable error list found; left undocumented. */
   #define NGreGetGlobalViewingXform 0x00004089L
   #define GreGetGlobalViewingXform(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),0L,NGreGetGlobalViewingXform)

   /* GreSetGlobalViewingXform — no verifiable error list found; left undocumented. */
   #define NGreSetGlobalViewingXform 0x0000408AL
   #define GreSetGlobalViewingXform(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),(ULONG)(ULONG)(c),0L,NGreSetGlobalViewingXform)

   /* GreSaveXformData — no verifiable error list found; left undocumented. */
   #define NGreSaveXformData 0x0000408BL
   #define GreSaveXformData(a,b,c) (LONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PBYTE)(c),0L,NGreSaveXformData)

   /* GreRestoreXformData — no verifiable error list found; left undocumented. */
   #define NGreRestoreXformData 0x0000408CL
   #define GreRestoreXformData(a,b,c) (LONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PBYTE)(c),0L,NGreRestoreXformData)

   /* GreGetPageViewport — no verifiable error list found; left undocumented. */
   #define NGreGetPageViewport 0x0000408DL
   #define GreGetPageViewport(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreGetPageViewport)

   /* GreSetPageViewport — no verifiable error list found; left undocumented. */
   #define NGreSetPageViewport 0x0000408EL
   #define GreSetPageViewport(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),(ULONG)(ULONG)(c),0L,NGreSetPageViewport)

   /* GreGetGraphicsField — no verifiable error list found; left undocumented. */
   #define NGreGetGraphicsField 0x00004091L
   #define GreGetGraphicsField(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreGetGraphicsField)

   /* GreSetGraphicsField — no verifiable error list found; left undocumented. */
   #define NGreSetGraphicsField 0x00004092L
   #define GreSetGraphicsField(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreSetGraphicsField)

   /* GreGetViewingLimits — no verifiable error list found; left undocumented. */
   #define NGreGetViewingLimits 0x00004093L
   #define GreGetViewingLimits(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreGetViewingLimits)

   /* GreSetViewingLimits — no verifiable error list found; left undocumented. */
   #define NGreSetViewingLimits 0x00004094L
   #define GreSetViewingLimits(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreSetViewingLimits)

   /* GreQueryViewportSize — no verifiable error list found; left undocumented. */
   #define NGreQueryViewportSize 0x00004095L
   #define GreQueryViewportSize(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),(ULONG)(PVIEWPORTSIZE)(d),0L,NGreQueryViewportSize)

   /* GreConvert — no verifiable error list found; left undocumented. */
   #define NGreConvert 0x00004096L
   #define GreConvert(a,b,c,d,e) (BOOL) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(LONG)(c),(ULONG)(PPOINTL)(d),(ULONG)(LONG)(e),0L,NGreConvert)

   /* GreConvertPath — no verifiable error list found; left undocumented. */
   #define NGreConvertPath 0x00004097L
   #define GreConvertPath(a,b,c,d,e,f) (BOOL) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(PPATH)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PCURVE)(e),(ULONG)(LONG)(f),0L,NGreConvertPath)

   /* GreSaveXform — no verifiable error list found; left undocumented. */
   #define NGreSaveXform 0x00004098L
   #define GreSaveXform(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreSaveXform)

   /* GreRestoreXform — no verifiable error list found; left undocumented. */
   #define NGreRestoreXform 0x00004099L
   #define GreRestoreXform(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreRestoreXform)

   /* GreMultiplyXforms — no verifiable error list found; left undocumented. */
   #define NGreMultiplyXforms 0x0000409AL
   #define GreMultiplyXforms(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),(ULONG)(PXFORM)(c),(ULONG)(ULONG)(d),0L,NGreMultiplyXforms)

   /* GreConvertWithMatrix — no verifiable error list found; left undocumented. */
   #define NGreConvertWithMatrix 0x0000409BL
   #define GreConvertWithMatrix(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),(ULONG)(PXFORM)(d),0L,NGreConvertWithMatrix)
#endif

#ifdef INCL_GRE_DEVMISC1
   /* GreDeviceGetAttributes — no verifiable error list found; left undocumented. */
   #define NGreDeviceGetAttributes 0x0000609DL
   #define GreDeviceGetAttributes(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(PBUNDLE)(d),0L,NGreDeviceGetAttributes)

   /* GreDeviceSetAVIOFont2 — no verifiable error list found; left undocumented. */
   #define NGreDeviceSetAVIOFont2 0x0000409EL
   #define GreDeviceSetAVIOFont2(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PFATTRS)(b),(ULONG)(PFOCAFONT)(c),(ULONG)(LCID)(d),0L,NGreDeviceSetAVIOFont2)

   /* GreGetPairKerningTable — no verifiable error list found; left undocumented. */
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
   /* GreNotifyClipChange — no verifiable error list found; left undocumented. */
   #define NGreNotifyClipChange 0x000040A4L
   #define GreNotifyClipChange(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),(ULONG)(LONG)(c),(ULONG)(ULONG)(d),0L,NGreNotifyClipChange)

   /* GreNotifyTransformChange — no verifiable error list found; left undocumented. */
   #define NGreNotifyTransformChange 0x000040A5L
   #define GreNotifyTransformChange(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PNOTIFYTRANSFORMDATA)(c),0L,NGreNotifyTransformChange)

   /* GreRealizeFont — no verifiable error list found; left undocumented. */
   #define NGreRealizeFont 0x000040A6L
   #define GreRealizeFont(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PFATTRS)(c),(ULONG)(PFOCAFONT)(d),0L,NGreRealizeFont)

   /* GreErasePS — no verifiable error list found; left undocumented. */
   #define NGreErasePS 0x000040A7L
   #define GreErasePS(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreErasePS)

   /* GreSetStyleRatio — no verifiable error list found; left undocumented. */
   #define NGreSetStyleRatio 0x000040A8L
   #define GreSetStyleRatio(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PBYTE)(b),0L,NGreSetStyleRatio)

   /* GreDeviceQueryFontAttributes — no verifiable error list found; left undocumented. */
   #define NGreDeviceQueryFontAttributes 0x000040A9L
   #define GreDeviceQueryFontAttributes(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PFONTMETRICS)(c),0L,NGreDeviceQueryFontAttributes)

   /* GreDeviceQueryFonts — no verifiable error list found; left undocumented. */
   #define NGreDeviceQueryFonts 0x000040AAL
   #define GreDeviceQueryFonts(a,b,c,d,e,f) (LONG) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PSZ)(c),(ULONG)(PFONTMETRICS)(d),(ULONG)(LONG)(e),(ULONG)(PLONG)(f),0L,NGreDeviceQueryFonts)

   /* GreDeviceInvalidateVisRegion — no verifiable error list found; left undocumented. */
   #define NGreDeviceInvalidateVisRegion 0x000040ABL
   #define GreDeviceInvalidateVisRegion(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PDC_BLOCK)(c),0L,NGreDeviceInvalidateVisRegion)

   /* GreSetFontMappingFlags — no verifiable error list found; left undocumented. */
   #define NGreSetFontMappingFlags 0x00004268L
   #define GreSetFontMappingFlags(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreSetFontMappingFlags)
#endif

#ifdef INCL_GRE_PICK
   /* GreGetPickWindow — no verifiable error list found; left undocumented. */
   #define NGreGetPickWindow 0x000040ACL
   #define GreGetPickWindow(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreGetPickWindow)

   /* GreSetPickWindow — no verifiable error list found; left undocumented. */
   #define NGreSetPickWindow 0x000040ADL
   #define GreSetPickWindow(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreSetPickWindow)
#endif

#ifdef INCL_GRE_DEVMISC3
   /* GreResetBounds — no verifiable error list found; left undocumented. */
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

   /* GreGetCodePage — no verifiable error list found; left undocumented. */
   #define NGreGetCodePage 0x000040B3L
   #define GreGetCodePage(a) (LONG) Gre32Entry3((ULONG)(HDC)(a),0L,NGreGetCodePage)

   /* GreSetCodePage — no verifiable error list found; left undocumented. */
   #define NGreSetCodePage 0x000040B4L
   #define GreSetCodePage(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreSetCodePage)

   /* GreLockDevice — no verifiable error list found; left undocumented. */
   #define NGreLockDevice 0x000040B5L
   #define GreLockDevice(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreLockDevice)

   /* GreUnlockDevice — no verifiable error list found; left undocumented. */
   #define NGreUnlockDevice 0x000040B6L
   #define GreUnlockDevice(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreUnlockDevice)

   /* GreDeath — no verifiable error list found; left undocumented. */
   #define NGreDeath 0x000040B7L
   #define GreDeath(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreDeath)

   /* GreResurrection — no verifiable error list found; left undocumented. */
   #define NGreResurrection 0x000040B8L
   #define GreResurrection(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PBYTE)(c),0L,NGreResurrection)

   /* GreGetDCOrigin — no verifiable error list found; left undocumented. */
   #define NGreGetDCOrigin 0x000040BAL
   #define GreGetDCOrigin(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreGetDCOrigin)

   /* GreDeviceSetDCOrigin — no verifiable error list found; left undocumented. */
   #define NGreDeviceSetDCOrigin 0x000040BBL
   #define GreDeviceSetDCOrigin(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreDeviceSetDCOrigin)

   /* GreGetLineOrigin — no verifiable error list found; left undocumented. */
   #define NGreGetLineOrigin 0x000040BCL
   #define GreGetLineOrigin(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreGetLineOrigin)

   /* GreSetLineOrigin — no verifiable error list found; left undocumented. */
   #define NGreSetLineOrigin 0x000040BDL
   #define GreSetLineOrigin(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),0L,NGreSetLineOrigin)
#endif

#ifdef INCL_GRE_DEVMISC2
   /* GreGetStyleRatio — no verifiable error list found; left undocumented. */
   #define NGreGetStyleRatio 0x000040BEL
   #define GreGetStyleRatio(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PBYTE)(b),0L,NGreGetStyleRatio)
#endif

#ifdef INCL_GRE_COLORTABLE
   /* GreQueryColorData — no verifiable error list found; left undocumented. */
   #define NGreQueryColorData 0x000060C3L
   #define GreQueryColorData(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PLONG)(c),0L,NGreQueryColorData)

   /* GreQueryLogColorTable — no verifiable error list found; left undocumented. */
   #define NGreQueryLogColorTable 0x000060C4L
   #define GreQueryLogColorTable(a,b,c,d,e) (LONG) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PLONG)(e),0L,NGreQueryLogColorTable)

   /* GreCreateLogColorTable — no verifiable error list found; left undocumented. */
   #define NGreCreateLogColorTable 0x000060C5L
   #define GreCreateLogColorTable(a,b,c,d,e,f) (BOOL) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(LONG)(d),(ULONG)(LONG)(e),(ULONG)(PLONG)(f),0L,NGreCreateLogColorTable)

   /* GreRealizeColorTable — no verifiable error list found; left undocumented. */
   #define NGreRealizeColorTable 0x000060C6L
   #define GreRealizeColorTable(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreRealizeColorTable)

   /* GreUnrealizeColorTable — no verifiable error list found; left undocumented. */
   #define NGreUnrealizeColorTable 0x000060C7L
   #define GreUnrealizeColorTable(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreUnrealizeColorTable)

   /* GreQueryRealColors — no verifiable error list found; left undocumented. */
   #define NGreQueryRealColors 0x000040C8L
   #define GreQueryRealColors(a,b,c,d,e) (LONG) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PLONG)(e),0L,NGreQueryRealColors)

   /* GreQueryNearestColor — no verifiable error list found; left undocumented. */
   #define NGreQueryNearestColor 0x000040C9L
   #define GreQueryNearestColor(a,b,c) (LONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(COLOR)(c),0L,NGreQueryNearestColor)

   /* GreQueryColorIndex — no verifiable error list found; left undocumented. */
   #define NGreQueryColorIndex 0x000060CAL
   #define GreQueryColorIndex(a,b,c) (LONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(COLOR)(c),0L,NGreQueryColorIndex)

   /* GreQueryRGBColor — no verifiable error list found; left undocumented. */
   #define NGreQueryRGBColor 0x000060CBL
   #define GreQueryRGBColor(a,b,c) (LONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(COLOR)(c),0L,NGreQueryRGBColor)
#endif

#ifdef INCL_GRE_DEVICE
   #ifndef NGreQueryDevFntMetrics
      /* GreQueryDeviceFontMetrics — no verifiable error list found; left undocumented. */
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
   /* GreQueryDeviceBitmaps — no verifiable error list found; left undocumented. */
   #define NGreQueryDeviceBitmaps 0x000040D0L
   #define GreQueryDeviceBitmaps(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PBITMAPFORMAT)(b),(ULONG)(LONG)(c),0L,NGreQueryDeviceBitmaps)

   /* GreQueryDeviceCaps — no verifiable error list found; left undocumented. */
   #define NGreQueryDeviceCaps 0x000040D1L
   #define GreQueryDeviceCaps(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PLONG)(c),(ULONG)(LONG)(d),0L,NGreQueryDeviceCaps)

   /* GreEscape — no verifiable error list found; left undocumented. */
   #define NGreEscape 0x000040D2L
   #define GreEscape(a,b,c,d,e,f) (LONG) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),(ULONG)(PBYTE)(d),(ULONG)(PLONG)(e),(ULONG)(PBYTE)(f),0L,NGreEscape)

   /* GreQueryHardcopyCaps — no verifiable error list found; left undocumented. */
   #define NGreQueryHardcopyCaps 0x000040D3L
   #define GreQueryHardcopyCaps(a,b,c,d) (LONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(LONG)(c),(ULONG)(PHCINFO)(d),0L,NGreQueryHardcopyCaps)

   /* GreQueryDevResource2 — no verifiable error list found; left undocumented. */
   #define NGreQueryDevResource2 0x000040D4L
   #define GreQueryDevResource2(a,b,c) (PVOID) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),0L,NGreQueryDevResource2)

   /* GreUnloadPD — no verifiable error list found; left undocumented. */
   #define NGreUnloadPD 0x0000026EL
   #define GreUnloadPD(a,b) (PVOID) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreUnloadPD)
#endif

#ifdef INCL_GRE_SCREEN
   /* GreOpenScreenChangeArea — no verifiable error list found; left undocumented. */
   #define NGreOpenScreenChangeArea 0x00004012L
   #define GreOpenScreenChangeArea(a) (ULONG) Gre32Entry3((ULONG)(HDC)(a),0L,NGreOpenScreenChangeArea)

   /* GreGetScreenChangeArea — no verifiable error list found; left undocumented. */
   #define NGreGetScreenChangeArea 0x00004013L
   #define GreGetScreenChangeArea(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),0L,NGreGetScreenChangeArea)

   /* GreCloseScreenChangeArea — no verifiable error list found; left undocumented. */
   #define NGreCloseScreenChangeArea 0x00004014L
   #define GreCloseScreenChangeArea(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreCloseScreenChangeArea)

   /* GreGetScreenBits — no verifiable error list found; left undocumented. */
   #define NGreGetScreenBits 0x0000401DL
   #define GreGetScreenBits(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(ULONG)(e),0L,NGreGetScreenBits)

   /* GreSetScreenBits — no verifiable error list found; left undocumented. */
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
   /* GreResetDC — no verifiable error list found; left undocumented. */
   #define NGreResetDC 0x00004202L
   #define GreResetDC(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreResetDC)

   /* GreResetDC2 — no verifiable error list found; left undocumented. */
   #define NGreResetDC2 0x0000426BL
   #define GreResetDC2(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreResetDC2)

   /* GreGetHandle — no verifiable error list found; left undocumented. */
   #define NGreGetHandle 0x00004203L
   #define GreGetHandle(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreGetHandle)

   /* GreSetHandle — no verifiable error list found; left undocumented. */
   #define NGreSetHandle 0x00004204L
   #define GreSetHandle(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LHANDLE)(b),(ULONG)(ULONG)(c),0L,NGreSetHandle)

   /* GreGetProcessControl — no verifiable error list found; left undocumented. */
   #define NGreGetProcessControl 0x00004205L
   #define GreGetProcessControl(a) (LONG) Gre32Entry3((ULONG)(HDC)(a),0L,NGreGetProcessControl)

   /* GreSetProcessControl — no verifiable error list found; left undocumented. */
   #define NGreSetProcessControl 0x00004206L
   #define GreSetProcessControl(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),0L,NGreSetProcessControl)

   /* GreSaveDC — no verifiable error list found; left undocumented. */
   #define NGreSaveDC 0x00004207L
   #define GreSaveDC(a) (LONG) Gre32Entry3((ULONG)(HDC)(a),0L,NGreSaveDC)

   /* GreRestoreDC — no verifiable error list found; left undocumented. */
   #define NGreRestoreDC 0x00004208L
   #define GreRestoreDC(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreRestoreDC)

   /* GreQueryDeviceNames — no verifiable error list found; left undocumented. */
   #define NGreQueryDeviceNames 0x00000209L
   #define GreQueryDeviceNames(a,b,c,d,e,f) (BOOL) Gre32Entry8((ULONG)(PSZ)(a),(ULONG)(PLONG)(b),(ULONG)(PSTR32)(c),(ULONG)(PSTR64)(d),(ULONG)(PLONG)(e),(ULONG)(PSTR16)(f),0L,NGreQueryDeviceNames)

   /* GreQueryEngineVersion — no verifiable error list found; left undocumented. */
   #define NGreQueryEngineVersion 0x0000020AL
   #define GreQueryEngineVersion() (LONG) Gre32Entry2(0L,NGreQueryEngineVersion)

   /* GreSetDCOwner — no verifiable error list found; left undocumented. */
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

   /* GreCreateBitmap — no verifiable error list found; left undocumented. */
   #define NGreCreateBitmap 0x00004210L
   #define GreCreateBitmap(a,b,c,d,e) (HBITMAP) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(PBITMAPINFOHEADER2)(b),(ULONG)(ULONG)(c),(ULONG)(PBYTE)(d),(ULONG)(PBITMAPINFO2)(e),0L,NGreCreateBitmap)

   /* GreDeleteBitmap — no verifiable error list found; left undocumented. */
   #define NGreDeleteBitmap 0x00000211L
   #define GreDeleteBitmap(a) (BOOL) Gre32Entry3((ULONG)(HBITMAP)(a),0L,NGreDeleteBitmap)

   /* GreSelectBitmap — no verifiable error list found; left undocumented. */
   #define NGreSelectBitmap 0x00004212L
   #define GreSelectBitmap(a,b) (HBITMAP) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HBITMAP)(b),0L,NGreSelectBitmap)

   /* GreGetBitmapParameters — no verifiable error list found; left undocumented. */
   #define NGreGetBitmapParameters 0x00000213L
   #define GreGetBitmapParameters(a,b) (BOOL) Gre32Entry4((ULONG)(HBITMAP)(a),(ULONG)(PBITMAPINFOHEADER)(b),0L,NGreGetBitmapParameters)

   /* GreGetBitmapDimension — no verifiable error list found; left undocumented. */
   #define NGreGetBitmapDimension 0x00000214L
   #define GreGetBitmapDimension(a,b) (BOOL) Gre32Entry4((ULONG)(HBITMAP)(a),(ULONG)(PSIZEL)(b),0L,NGreGetBitmapDimension)

   /* GreSetBitmapDimension — no verifiable error list found; left undocumented. */
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
   /* GreDeleteSetId — no verifiable error list found; left undocumented. */
   #define NGreDeleteSetId 0x00004222L
   #define GreDeleteSetId(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LCID)(b),0L,NGreDeleteSetId)

   /* GreQueryNumberSetIds — no verifiable error list found; left undocumented. */
   #define NGreQueryNumberSetIds 0x00004223L
   #define GreQueryNumberSetIds(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreQueryNumberSetIds)

   /* GreQuerySetIds — no verifiable error list found; left undocumented. */
   #define NGreQuerySetIds 0x00004224L
   #define GreQuerySetIds(a,b,c,d,e,f) (BOOL) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PLONG)(c),(ULONG)(PSTR8)(d),(ULONG)(PLCID)(e),(ULONG)(ULONG)(f),0L,NGreQuerySetIds)
#endif

#ifdef INCL_GRE_LCID
   /* GreQueryBitmapHandle — no verifiable error list found; left undocumented. */
   #define NGreQueryBitmapHandle 0x00000225L
   #define GreQueryBitmapHandle(a,b) (HBITMAP) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LCID)(b),0L,NGreQueryBitmapHandle)

   /* GreSetBitmapID — no verifiable error list found; left undocumented. */
   #define NGreSetBitmapID 0x00004226L
   #define GreSetBitmapID(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HBITMAP)(b),(ULONG)(LCID)(c),0L,NGreSetBitmapID)

   /* GreCopyDCLoadData — no verifiable error list found; left undocumented. */
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

   /* GreLoadFont — no verifiable error list found; left undocumented. */
   #define NGreLoadFont 0x0000022EL
   #define GreLoadFont(a) (BOOL) Gre32Entry3((ULONG)(PSZ)(a),0L,NGreLoadFont)

   /* GreUnloadFont — no verifiable error list found; left undocumented. */
   #define NGreUnloadFont 0x0000022FL
   #define GreUnloadFont(a) (BOOL) Gre32Entry3((ULONG)(PSZ)(a),0L,NGreUnloadFont)

   /* GreQueryFonts — no verifiable error list found; left undocumented. */
   #define NGreQueryFonts 0x00004230L
   #define GreQueryFonts(a,b,c,d,e,f) (LONG) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PSZ)(c),(ULONG)(PFONTMETRICS)(d),(ULONG)(LONG)(e),(ULONG)(PLONG)(f),0L,NGreQueryFonts)

   /* GreQueryFontAttributes — no verifiable error list found; left undocumented. */
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

   /* GreQueryCodePageVector — no verifiable error list found; left undocumented. */
   #define NGreQueryCodePageVector 0x00000235L
   #define GreQueryCodePageVector(a) (ULONG) Gre32Entry3((ULONG)(ULONG)(a),0L,NGreQueryCodePageVector)

   /* GreQueryFontFileDescriptions — no verifiable error list found; left undocumented. */
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
