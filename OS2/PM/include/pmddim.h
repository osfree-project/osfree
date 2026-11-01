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
   #define NGreDisjointLines 0x00004016L
   #define GreDisjointLines(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),0L,NGreDisjointLines)
   #define NGreGetCurrentPosition 0x00004017L
   #define GreGetCurrentPosition(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreGetCurrentPosition)
   #define NGreSetCurrentPosition 0x00004018L
   #define GreSetCurrentPosition(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreSetCurrentPosition)
   #define NGrePolyLine 0x00004019L
   #define GrePolyLine(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),0L,NGrePolyLine)
   #define NGreDrawLinesInPath 0x0000401AL
   #define GreDrawLinesInPath(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PPATH)(b),(ULONG)(PCURVE)(c),(ULONG)(LONG)(d),0L,NGreDrawLinesInPath)
   #define NGrePolyShortLine 0x0000401BL
   #define GrePolyShortLine(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PSHORTLINE)(b),0L,NGrePolyShortLine)
#endif

#ifdef INCL_GRE_POLYGON
   #define NGrePolygonSet 0x00004056L
   #define GrePolygonSet(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(b),(ULONG)(c),(ULONG)(PPOLYGON)(d),(ULONG)(e),0L,NGrePolygonSet)
   #define NGreDrawRLE 0x00004058L
   #define GreDrawRLE(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRLEHDR)(b),0L,NGreDrawRLE)
   #define NGreDevicePolygonSet 0x00004059L
   #define GreDevicePolygonSet(a,b) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOLYSET)(b),0L,NGreDevicePolygonSet)
   #define NGrePolygon3d 0x0000405CL
   #define GrePolygon3d(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOLYSINFO)(b),(ULONG)(ULONG)(c),0L,NGrePolygon3d)
#endif

#ifdef INCL_GRE_ARCS
   #define NGreGetArcParameters 0x00004000L
   #define GreGetArcParameters(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PARCPARAMS)(b),0L,NGreGetArcParameters)
   #define NGreSetArcParameters 0x00004001L
   #define GreSetArcParameters(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PARCPARAMS)(b),0L,NGreSetArcParameters)
   #define NGreArc 0x00004002L
   #define GreArc(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreArc)
   #define NGrePartialArc 0x00004003L
   #define GrePartialArc(a,b,c,d,e) (INT) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(FIXED)(c),(ULONG)(FIXED)(d),(ULONG)(FIXED)(e),0L,NGrePartialArc)
   #define NGreFullArcInterior 0x00004004L
   #define GreFullArcInterior(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(FIXED)(b),0L,NGreFullArcInterior)
   #define NGreFullArcBoundary 0x00004005L
   #define GreFullArcBoundary(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(FIXED)(b),0L,NGreFullArcBoundary)
   #define NGreFullArcBoth 0x00004006L
   #define GreFullArcBoth(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(FIXED)(b),0L,NGreFullArcBoth)
   #define NGreBoxInterior 0x00004007L
   #define GreBoxInterior(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PBOXPARAMS)(b),0L,NGreBoxInterior)
   #define NGreBoxBoundary 0x00004008L
   #define GreBoxBoundary(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PBOXPARAMS)(b),0L,NGreBoxBoundary)
   #define NGreBoxBoth 0x00004009L
   #define GreBoxBoth(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PBOXPARAMS)(b),0L,NGreBoxBoth)
   #define NGreBoxExclusive 0x00004010L
   #define GreBoxExclusive(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PBOXPARAMS)(b),(ULONG)(c),0L,NGreBoxExclusive)
   #define NGrePolyFillet 0x0000400AL
   #define GrePolyFillet(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),0L,NGrePolyFillet)
   #define NGrePolyFilletSharp 0x0000400BL
   #define GrePolyFilletSharp(a,b,c,d) (INT) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),(ULONG)(PFIXED)(d),0L,NGrePolyFilletSharp)
   #define NGrePolySpline 0x0000400CL
   #define GrePolySpline(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),0L,NGrePolySpline)
   #define NGreOval 0x0000400DL
   #define GreOval(a,b,c,d,e) (INT) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(FIXED)(c),(ULONG)(FIXED)(d),(ULONG)(e),0L,NGreOval)
   #define NGreCookWholePath 0x0000400EL
   #define GreCookWholePath(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPATH)(b),0L,NGreCookWholePath)
   #define NGreCookPathCurves 0x0000400FL
   #define GreCookPathCurves(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPATH)(b),(ULONG)(PPIPELINEINFO)(c),0L,NGreCookPathCurves)
   #define NGreRenderPath 0x00004011L
   #define GreRenderPath(a,b,c,d,e,f,g,h) (BOOL) Gre32Entry10((ULONG)(HDC)(a),(ULONG)(PPATH)(b),(ULONG)(PCURVE)(c),(ULONG)(LONG)(d),(ULONG)(LONG)(e),(ULONG)(LONG)(f),(ULONG)(PSHORTLINE)(g),(ULONG)(LONG)(h),0L,NGreRenderPath)
#endif

#ifdef INCL_GRE_REGIONS
   #define NGreGetRegionBox 0x0000405DL
   #define GreGetRegionBox(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PRECTL)(c),0L,NGreGetRegionBox)
   #define NGreGetRegionRects 0x0000405EL
   #define GreGetRegionRects(a,b,c,d,e) (BOOL) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PRECTL)(c),(ULONG)(PRGNRECT)(d),(ULONG)(PRECTL)(e),0L,NGreGetRegionRects)
   #define NGreOffsetRegion 0x0000405FL
   #define GreOffsetRegion(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PPOINTL)(c),0L,NGreOffsetRegion)
   #define NGrePtInRegion 0x00004060L
   #define GrePtInRegion(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PPOINTL)(c),0L,NGrePtInRegion)
   #define NGreRectInRegion 0x00004061L
   #define GreRectInRegion(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PRECTL)(c),0L,NGreRectInRegion)
   #define NGreCreateRectRegion 0x00004062L
   #define GreCreateRectRegion(a,b,c) (HRGN) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),(ULONG)(LONG)(c),0L,NGreCreateRectRegion)
   #define NGreDestroyRegion 0x00004063L
   #define GreDestroyRegion(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HRGN)(b),0L,NGreDestroyRegion)
   #define NGreSetRectRegion 0x00004064L
   #define GreSetRectRegion(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PRECTL)(c),(ULONG)(LONG)(d),0L,NGreSetRectRegion)
   #define NGreCombineRegion 0x00004065L
   #define GreCombineRegion(a,b,c,d,e) (INT) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(HRGN)(c),(ULONG)(HRGN)(d),(ULONG)(ULONG)(e),0L,NGreCombineRegion)
   #define NGreCombineRectRegion 0x00004066L
   #define GreCombineRectRegion(a,b,c,d,e) (INT) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PRECTL)(c),(ULONG)(HRGN)(d),(ULONG)(ULONG)(e),0L,NGreCombineRectRegion)
   #define NGreCombineShortLineRegion 0x00004067L
   #define GreCombineShortLineRegion(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PSHORTLINE)(c),0L,NGreCombineShortLineRegion)
   #define NGreEqualRegion 0x00004068L
   #define GreEqualRegion(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(HRGN)(c),0L,NGreEqualRegion)
   #define NGrePaintRegion 0x00004069L
   #define GrePaintRegion(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HRGN)(b),0L,NGrePaintRegion)
   #define NGreSetRegionOwner 0x0000406AL
   #define GreSetRegionOwner(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(ULONG)(c),0L,NGreSetRegionOwner)
   #define NGreFrameRegion 0x0000406BL
   #define GreFrameRegion(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PSIZEL)(c),0L,NGreFrameRegion)
#endif

#ifdef INCL_GRE_BITMAPS
   #define NGreDrawBits 0x00006022L
   #define GreDrawBits(a,b,c,d,e,f,g) (LONG) Gre32Entry9((ULONG)(HDC)(a),(ULONG)(PBYTE)(b),(ULONG)(PBITMAPINFO2)(c),(ULONG)(LONG)(d),(ULONG)(PPOINTL)(e),(ULONG)(LONG)(f),(ULONG)(ULONG)(g),0L,NGreDrawBits)
   #define NGreDeviceCreateBitmap 0x00006023L
   #define GreDeviceCreateBitmap(a,b,c,d,e) (HBITMAP) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(PBITMAPINFOHEADER2)(b),(ULONG)(ULONG)(c),(ULONG)(PBYTE)(d),(ULONG)(PBITMAPINFO2)(e),0L,NGreDeviceCreateBitmap)
   #define NGreDeviceDeleteBitmap 0x00004024L
   #define GreDeviceDeleteBitmap(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(HBITMAP)(b),(ULONG)(PDELETERETURN)(c),(ULONG)(ULONG)(d),0L,NGreDeviceDeleteBitmap)
   #define NGreDeviceSelectBitmap 0x00004025L
   #define GreDeviceSelectBitmap(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HBITMAP)(b),0L,NGreDeviceSelectBitmap)
   #define NGreBitblt 0x00006026L
   #define GreBitblt(a,b,c,d,e,f,g) (INT) Gre32Entry9((ULONG)(HDC)(a),(ULONG)(HDC)(b),(ULONG)(LONG)(c),(ULONG)(PBITBLTPARAMETERS)(d),(ULONG)(LONG)(e),(ULONG)(ULONG)(f),(ULONG)(PBITBLTATTRS)(g),0L,NGreBitblt)
   #define NGreGetPel 0x00006027L
   #define GreGetPel(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreGetPel)
   #define NGreSetPel 0x00004028L
   #define GreSetPel(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreSetPel)
   #define NGreImageData 0x00004029L
   #define GreImageData(a,b,c,d) (INT) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PBYTE)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),0L,NGreImageData)
   #define NGreScanLR 0x0000602AL
   #define GreScanLR(a,b,c,d,e,f) (INT) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PLONG)(e),(ULONG)(PLONG)(f),0L,NGreScanLR)
   #define NGreFloodFill 0x0000602BL
   #define GreFloodFill(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(LONG)(c),0L,NGreFloodFill)
   #define NGreSaveScreenBits 0x0000402CL
   #define GreSaveScreenBits(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreSaveScreenBits)
   #define NGreRestoreScreenBits 0x0000402DL
   #define GreRestoreScreenBits(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(LHANDLE)(b),(ULONG)(PRECTL)(c),(ULONG)(ULONG)(d),0L,NGreRestoreScreenBits)
   #define NGreDrawBorder 0x0000602EL
   #define GreDrawBorder(a,b,c,d,e,f,g) (BOOL) Gre32Entry9((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(COLOR)(e),(ULONG)(COLOR)(f),(ULONG)(ULONG)(g),0L,NGreDrawBorder)
   #define NGreDeviceSetCursor 0x0000402FL
   #define GreDeviceSetCursor(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(HBITMAP)(c),0L,NGreDeviceSetCursor)
   #define NGreGetBitmapBits 0x00006030L
   #define GreGetBitmapBits(a,b,c,d,e,f) (LONG) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(HBITMAP)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PBYTE)(e),(ULONG)(PBITMAPINFO2)(f),0L,NGreGetBitmapBits)
   #define NGreSetBitmapBits 0x00006031L
   #define GreSetBitmapBits(a,b,c,d,e,f) (LONG) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(HBITMAP)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PBYTE)(e),(ULONG)(PBITMAPINFO2)(f),0L,NGreSetBitmapBits)
#endif


#ifdef INCL_GRE_DCS
   #define NGreOpenDC 0x00000200L
   #define GreOpenDC(a,b,c,d,e) (HDC) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PSZ)(c),(ULONG)(LONG)(d),(ULONG)(PDEVOPENDATA)(e),0L,NGreOpenDC)
   #define NGreCloseDC 0x00004201L
   #define GreCloseDC(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreCloseDC)
#endif

#ifdef INCL_GRE_INKPATH
   #define NGreBeginInkPath 0x00004259L
   #define GreBeginInkPath(a,b,c) (BOOL) Gre32Entry5((HDC)(a),(LONG)(b),(ULONG)(c),0L,NGreBeginInkPath)
   #define NGreEndInkPath 0x0000425AL
   #define GreEndInkPath(a,b) (BOOL) Gre32Entry4((HDC)(a),(ULONG)(b),0L,NGreEndInkPath)
   #define NGreStrokeInkPath 0x0000425BL
   #define GreStrokeInkPath(a,b,c,d,e) (BOOL) Gre32Entry7((HDC)(a),(LONG)(b),(LONG)(c),(PPOINTL)(d),(ULONG)(e),0L,NGreStrokeInkPath)
#endif

#ifdef INCL_GRE_PALETTE
   #define NGreCreatePalette 0x00000240L
   #define GreCreatePalette(a,b,c,d) (HPAL) Gre32Entry6((ULONG)(ULONG)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(PULONG)(d),0L,NGreCreatePalette)
   #define NGreDeletePalette 0x00000241L
   #define GreDeletePalette(a) (ULONG) Gre32Entry3((ULONG)(HPAL)(a),0L,NGreDeletePalette)
   #define NGreSelectPalette 0x00004242L
   #define GreSelectPalette(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HPAL)(b),0L,NGreSelectPalette)
   #define NGreSetPaletteEntries 0x00000243L
   #define GreSetPaletteEntries(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HPAL)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(PULONG)(e),0L,NGreSetPaletteEntries)
   #define NGreAnimatePalette 0x00000244L
   #define GreAnimatePalette(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HPAL)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(PULONG)(e),0L,NGreAnimatePalette)
   #define NGreQueryPalette 0x00004245L
   #define GreQueryPalette(a) (ULONG) Gre32Entry3((ULONG)(HDC)(a),0L,NGreQueryPalette)
   #define NGreQueryPaletteInfo 0x00000246L
   #define GreQueryPaletteInfo(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HPAL)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(PULONG)(e),0L,NGreQueryPaletteInfo)
   #define NGreSetPaletteOwner 0x00000247L
   #define GreSetPaletteOwner(a,b) (BOOL) Gre32Entry4((ULONG)(HPAL)(a),(ULONG)(ULONG)(b),0L,NGreSetPaletteOwner)
   #define NGreResizePalette 0x00000248L
   #define GreResizePalette(a,b) (ULONG) Gre32Entry4((ULONG)(HPAL)(a),(ULONG)(ULONG)(b),0L,NGreResizePalette)
   #define NGreQueryDefaultPaletteInfo 0x00004249L
   #define GreQueryDefaultPaletteInfo(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(PULONG)(e),0L,NGreQueryDefaultPaletteInfo)
#endif

#ifdef INCL_WINPOINTERS
   #define NGreDeviceSetPointer 0x0000403FL
   #define GreDeviceSetPointer(a,b) (LONG) Gre32Entry4((a), (PHWSETPTRIN)(b), 0L, NGreDeviceSetPointer)
   #define NGreSetColorCursor 0x00004032L
   #define GreSetColorCursor(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTERINFO)(b),0L,NGreSetColorCursor)
#endif

#ifdef INCL_GRE_SCANS
   #define NGrePolyScanline 0x0000401CL
   #define GrePolyScanline(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PSCANDATA)(b),0L,NGrePolyScanline)
#endif

#ifdef INCL_GRE_STRINGS
   #define NGreCharString 0x00005035L
   #define GreCharString(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PCH)(c),0L,NGreCharString)
   #define NGreCharStringPos 0x00007036L
   #define GreCharStringPos(a,b,c,d,e,f,g,h) (INT) Gre32Entry10((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(PRECTL)(c),(ULONG)(ULONG)(d),(ULONG)(LONG)(e),(ULONG)(PCH)(f),(ULONG)(PLONG)(g),(ULONG)(PCSP_INFO)(h),0L,NGreCharStringPos)
   #define NGreQueryTextBox 0x00005037L
   #define GreQueryTextBox(a,b,c,d,e) (BOOL) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PCH)(c),(ULONG)(LONG)(d),(ULONG)(PPOINTL)(e),0L,NGreQueryTextBox)
   #define NGreQueryCharPositions 0x00005038L
   #define GreQueryCharPositions(a,b,c,d,e,f,g) (ULONG) Gre32Entry9((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(ULONG)(c),(ULONG)(LONG)(d),(ULONG)(PCH)(e),(ULONG)(PLONG)(f),(ULONG)(PPOINTL)(g),0L,NGreQueryCharPositions)
   #define NGreQueryWidthTable 0x00005039L
   #define GreQueryWidthTable(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(LONG)(c),(ULONG)(PLONG)(d),0L,NGreQueryWidthTable)
   #define NGreQueryCharOutline 0x00004256L
   #define GreQueryCharOutline(a,b,c,d,e) (LONG)Gre32Entry7((HDC)(a),(ULONG)(b),(ULONG)(PBYTE)(c),(ULONG)(d),(ULONG)(e),0L,NGreQueryCharOutline)
   #define NGreQueryCharMetricsTable 0x00004258L
   #define GreQueryCharMetricsTable(a,b,c,d,e,f) (BOOL)Gre32Entry8((HDC)(a),(ULONG)(b),(ULONG)(c),(ULONG)(PCHARMETRICS)(d),(ULONG)(e),(ULONG)(f),0L,NGreQueryCharMetricsTable)
   #define NGreRealizeString 0x00004257L
   #define GreRealizeString(a,b,c,d,e,f) (LONG)Gre32Entry8((HDC)(a),(ULONG)(b),(ULONG)(c),(ULONG)(d),(ULONG)(e),(ULONG)(f),0L,NGreRealizeString)
   #define NGreQueryTabbedTextExtent 0x00004269L
   #define GreQueryTabbedTextExtent(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(a),(ULONG)(b),(ULONG)(c),(ULONG)(d), (ULONG)(e), 0L, NGreQueryTabbedTextExtent)
   #define NGreTabbedCharStringAt 0x0000426AL
   #define GreTabbedCharStringAt(a,b,c,d,e,f,g,h) (ULONG) Gre32Entry10((ULONG)(a),(ULONG)(PPOINTL)(b),(ULONG)(PRECTL)(c),(ULONG)(d), (ULONG)(e),(ULONG)(PCH)(f),(ULONG)(PLONG)(g),(ULONG)(PULONG)(h), 0L, NGreTabbedCharStringAt)
#endif

#ifdef INCL_GRE_MARKERS
   #define NGrePolyMarker 0x0000403AL
   #define GrePolyMarker(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),0L,NGrePolyMarker)
#endif

#ifdef INCL_AVIOP
   #define NGreCharRect 0x0000403BL
   #define GreCharRect(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(VioPresentationSpace *)(b),(ULONG)(LPGridRectRef)(c),0L,NGreCharRect)
   #define NGreCharStr 0x0000403CL
   #define GreCharStr(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(VioPresentationSpace *)(b),(ULONG)(LPGridStringRef)(c),0L,NGreCharStr)
   #define NGreScrollRect 0x0000403DL
   #define GreScrollRect(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(VioPresentationSpace *)(b),(ULONG)(LPScrollRectRef)(c),0L,NGreScrollRect)
   #define NGreUpdateCursor 0x0000403EL
   #define GreUpdateCursor(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(VioPresentationSpace *)(b),0L,NGreUpdateCursor)
#endif

#ifdef INCL_GRE_PATHS
   #define NGreBeginArea 0x00004046L
   #define GreBeginArea(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreBeginArea)
   #define NGreEndArea 0x00004047L
   #define GreEndArea(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreEndArea)
   #define NGreBeginPath 0x00004048L
   #define GreBeginPath(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PHID)(b),0L,NGreBeginPath)
   #define NGreEndPath 0x00004049L
   #define GreEndPath(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreEndPath)
   #define NGreCloseFigure 0x0000404AL
   #define GreCloseFigure(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreCloseFigure)
   #define NGreFillPath 0x0000404BL
   #define GreFillPath(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PHID)(b),(ULONG)(ULONG)(c),0L,NGreFillPath)
   #define NGreOutlinePath 0x0000404CL
   #define GreOutlinePath(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PHID)(b),(ULONG)(ULONG)(c),0L,NGreOutlinePath)
   #define NGreModifyPath 0x0000404DL
   #define GreModifyPath(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PHID)(b),(ULONG)(ULONG)(c),0L,NGreModifyPath)
   #define NGreStrokePath 0x0000404EL
   #define GreStrokePath(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PHID)(b),(ULONG)(ULONG)(c),0L,NGreStrokePath)
   #define NGreSelectClipPath 0x0000404FL
   #define GreSelectClipPath(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PHID)(b),(ULONG)(ULONG)(c),0L,NGreSelectClipPath)
   #define NGreSavePath 0x00004050L
   #define GreSavePath(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreSavePath)
   #define NGreRestorePath 0x00004051L
   #define GreRestorePath(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreRestorePath)
   #define NGreClip1DPath 0x00004052L
   #define GreClip1DPath(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPATH)(b),(ULONG)(PPIPELINEINFO)(c),0L,NGreClip1DPath)
   #define NGreDrawRawPath 0x00004053L
   #define GreDrawRawPath(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPATH)(b),0L,NGreDrawRawPath)
   #define NGreDrawCookedPath 0x00004054L
   #define GreDrawCookedPath(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PPATH)(b),(ULONG)(PCURVE)(c),(ULONG)(LONG)(d),0L,NGreDrawCookedPath)
   #define NGreAreaSetAttributes 0x00006055L
   #define GreAreaSetAttributes(a,b,c,d,e) (BOOL) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(PBUNDLE)(e),0L,NGreAreaSetAttributes)
   #define NGrePathToRegion 0x00004057L
   #define GrePathToRegion(a,b,c) (HRGN) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PHID)(b),(ULONG)(ULONG)(c),0L,NGrePathToRegion)
#endif


#ifdef INCL_GRE_CLIP
   #define NGreGetClipBox 0x0000406EL
   #define GreGetClipBox(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreGetClipBox)
   #define NGreGetClipRects 0x0000406FL
   #define GreGetClipRects(a,b,c,d) (INT) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),(ULONG)(PRGNRECT)(c),(ULONG)(PRECTL)(d),0L,NGreGetClipRects)
   #define NGreOffsetClipRegion 0x00004070L
   #define GreOffsetClipRegion(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreOffsetClipRegion)
   #define NGrePtVisible 0x00004071L
   #define GrePtVisible(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGrePtVisible)
   #define NGreRectVisible 0x00004072L
   #define GreRectVisible(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreRectVisible)
   #define NGreQueryClipRegion 0x00004073L
   #define GreQueryClipRegion(a) (HRGN) Gre32Entry3((ULONG)(HDC)(a),0L,NGreQueryClipRegion)
   #define NGreSelectClipRegion 0x00004074L
   #define GreSelectClipRegion(a,b,c) (INT) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PHRGN)(c),0L,NGreSelectClipRegion)
   #define NGreIntersectClipRectangle 0x00004075L
   #define GreIntersectClipRectangle(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreIntersectClipRectangle)
   #define NGreExcludeClipRectangle 0x00004076L
   #define GreExcludeClipRectangle(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreExcludeClipRectangle)
   #define NGreSetXformRect 0x00004077L
   #define GreSetXformRect(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreSetXformRect)
   #define NGreSaveRegion 0x0000407BL
   #define GreSaveRegion(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreSaveRegion)
   #define NGreRestoreRegion 0x0000407CL
   #define GreRestoreRegion(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreRestoreRegion)
   #define NGreClipPathCurves 0x0000407DL
   #define GreClipPathCurves(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPATH)(b),(ULONG)(PPIPELINEINFO)(c),0L,NGreClipPathCurves)
   #define NGreSelectPathRegion 0x0000407EL
   #define GreSelectPathRegion(a,b) (INT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HRGN)(b),0L,NGreSelectPathRegion)
   #define NGreRegionSelectBitmap 0x0000407FL
   #define GreRegionSelectBitmap(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HBITMAP)(b),0L,NGreRegionSelectBitmap)
   #define NGreCopyClipRegion 0x00004080L
   #define GreCopyClipRegion(a,b,c,d) (INT) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(PRECTL)(c),(ULONG)(ULONG)(d),0L,NGreCopyClipRegion)
   #define NGreSetupDC 0x00004081L
   #define GreSetupDC(a,b,c,d,e,f) (BOOL) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(HRGN)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PRECTL)(e),(ULONG)(ULONG)(f),0L,NGreSetupDC)
#endif

#ifdef INCL_GRE_XFORMS
   #define NGreGetPageUnits 0x00004083L
   #define GreGetPageUnits(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PULONG)(b),0L,NGreGetPageUnits)
   #define NGreSetPageUnits 0x00004084L
   #define GreSetPageUnits(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),0L,NGreSetPageUnits)
   #define NGreGetModelXform 0x00004085L
   #define GreGetModelXform(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),0L,NGreGetModelXform)
   #define NGreSetModelXform 0x00004086L
   #define GreSetModelXform(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),(ULONG)(ULONG)(c),0L,NGreSetModelXform)
   #define NGreGetWindowViewportXform 0x00004087L
   #define GreGetWindowViewportXform(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),0L,NGreGetWindowViewportXform)
   #define NGreSetWindowViewportXform 0x00004088L
   #define GreSetWindowViewportXform(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),(ULONG)(ULONG)(c),0L,NGreSetWindowViewportXform)
   #define NGreGetGlobalViewingXform 0x00004089L
   #define GreGetGlobalViewingXform(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),0L,NGreGetGlobalViewingXform)
   #define NGreSetGlobalViewingXform 0x0000408AL
   #define GreSetGlobalViewingXform(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),(ULONG)(ULONG)(c),0L,NGreSetGlobalViewingXform)
   #define NGreSaveXformData 0x0000408BL
   #define GreSaveXformData(a,b,c) (LONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PBYTE)(c),0L,NGreSaveXformData)
   #define NGreRestoreXformData 0x0000408CL
   #define GreRestoreXformData(a,b,c) (LONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PBYTE)(c),0L,NGreRestoreXformData)
   #define NGreGetPageViewport 0x0000408DL
   #define GreGetPageViewport(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreGetPageViewport)
   #define NGreSetPageViewport 0x0000408EL
   #define GreSetPageViewport(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),(ULONG)(ULONG)(c),0L,NGreSetPageViewport)
   #define NGreGetGraphicsField 0x00004091L
   #define GreGetGraphicsField(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreGetGraphicsField)
   #define NGreSetGraphicsField 0x00004092L
   #define GreSetGraphicsField(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreSetGraphicsField)
   #define NGreGetViewingLimits 0x00004093L
   #define GreGetViewingLimits(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreGetViewingLimits)
   #define NGreSetViewingLimits 0x00004094L
   #define GreSetViewingLimits(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreSetViewingLimits)
   #define NGreQueryViewportSize 0x00004095L
   #define GreQueryViewportSize(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),(ULONG)(PVIEWPORTSIZE)(d),0L,NGreQueryViewportSize)
   #define NGreConvert 0x00004096L
   #define GreConvert(a,b,c,d,e) (BOOL) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(LONG)(c),(ULONG)(PPOINTL)(d),(ULONG)(LONG)(e),0L,NGreConvert)
   #define NGreConvertPath 0x00004097L
   #define GreConvertPath(a,b,c,d,e,f) (BOOL) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(PPATH)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PCURVE)(e),(ULONG)(LONG)(f),0L,NGreConvertPath)
   #define NGreSaveXform 0x00004098L
   #define GreSaveXform(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreSaveXform)
   #define NGreRestoreXform 0x00004099L
   #define GreRestoreXform(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreRestoreXform)
   #define NGreMultiplyXforms 0x0000409AL
   #define GreMultiplyXforms(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PXFORM)(b),(ULONG)(PXFORM)(c),(ULONG)(ULONG)(d),0L,NGreMultiplyXforms)
   #define NGreConvertWithMatrix 0x0000409BL
   #define GreConvertWithMatrix(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),(ULONG)(PXFORM)(d),0L,NGreConvertWithMatrix)
#endif

#ifdef INCL_GRE_DEVMISC1
   #define NGreDeviceGetAttributes 0x0000609DL
   #define GreDeviceGetAttributes(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(PBUNDLE)(d),0L,NGreDeviceGetAttributes)
   #define NGreDeviceSetAVIOFont2 0x0000409EL
   #define GreDeviceSetAVIOFont2(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PFATTRS)(b),(ULONG)(PFOCAFONT)(c),(ULONG)(LCID)(d),0L,NGreDeviceSetAVIOFont2)
   #define NGreGetPairKerningTable 0x000040A0L
   #define GreGetPairKerningTable(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PKERNINGPAIRS)(c),0L,NGreGetPairKerningTable)
   #define NGreDeviceSetAVIOFont 0x000040A1L
   #define GreDeviceSetAVIOFont(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PFOCAFONT)(b),(ULONG)(LCID)(c),0L,NGreDeviceSetAVIOFont)
   #define NGreDeviceSetAttributes 0x000060A2L
   #define GreDeviceSetAttributes(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(PBUNDLE)(e),0L,NGreDeviceSetAttributes)
   #define NGreDeviceSetGlobalAttribute 0x000060A3L
   #define GreDeviceSetGlobalAttribute(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),0L,NGreDeviceSetGlobalAttribute)
#endif

#ifdef INCL_GRE_DEVMISC2
   #define NGreNotifyClipChange 0x000040A4L
   #define GreNotifyClipChange(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),(ULONG)(LONG)(c),(ULONG)(ULONG)(d),0L,NGreNotifyClipChange)
   #define NGreNotifyTransformChange 0x000040A5L
   #define GreNotifyTransformChange(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PNOTIFYTRANSFORMDATA)(c),0L,NGreNotifyTransformChange)
   #define NGreRealizeFont 0x000040A6L
   #define GreRealizeFont(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PFATTRS)(c),(ULONG)(PFOCAFONT)(d),0L,NGreRealizeFont)
   #define NGreErasePS 0x000040A7L
   #define GreErasePS(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreErasePS)
   #define NGreSetStyleRatio 0x000040A8L
   #define GreSetStyleRatio(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PBYTE)(b),0L,NGreSetStyleRatio)
   #define NGreDeviceQueryFontAttributes 0x000040A9L
   #define GreDeviceQueryFontAttributes(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PFONTMETRICS)(c),0L,NGreDeviceQueryFontAttributes)
   #define NGreDeviceQueryFonts 0x000040AAL
   #define GreDeviceQueryFonts(a,b,c,d,e,f) (LONG) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PSZ)(c),(ULONG)(PFONTMETRICS)(d),(ULONG)(LONG)(e),(ULONG)(PLONG)(f),0L,NGreDeviceQueryFonts)
   #define NGreDeviceInvalidateVisRegion 0x000040ABL
   #define GreDeviceInvalidateVisRegion(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PDC_BLOCK)(c),0L,NGreDeviceInvalidateVisRegion)
   #define NGreSetFontMappingFlags 0x00004268L
   #define GreSetFontMappingFlags(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreSetFontMappingFlags)
#endif

#ifdef INCL_GRE_PICK
   #define NGreGetPickWindow 0x000040ACL
   #define GreGetPickWindow(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreGetPickWindow)
   #define NGreSetPickWindow 0x000040ADL
   #define GreSetPickWindow(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreSetPickWindow)
#endif

#ifdef INCL_GRE_DEVMISC3
   #define NGreResetBounds 0x000040AEL
   #define GreResetBounds(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreResetBounds)
   #define NGreGetBoundsData 0x000040AFL
   #define GreGetBoundsData(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PRECTL)(c),0L,NGreGetBoundsData)
   #define NGreAccumulateBounds 0x000040B0L
   #define GreAccumulateBounds(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreAccumulateBounds)
   #define NGreGetExtraError 0x000040B1L
   #define GreGetExtraError(a) (FIXED) Gre32Entry3((ULONG)(HDC)(a),0L,NGreGetExtraError)
   #define NGreSetExtraError 0x000040B2L
   #define GreSetExtraError(a,b) (VOID) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(FIXED)(b),0L,NGreSetExtraError)
   #define NGreGetCodePage 0x000040B3L
   #define GreGetCodePage(a) (LONG) Gre32Entry3((ULONG)(HDC)(a),0L,NGreGetCodePage)
   #define NGreSetCodePage 0x000040B4L
   #define GreSetCodePage(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreSetCodePage)
   #define NGreLockDevice 0x000040B5L
   #define GreLockDevice(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreLockDevice)
   #define NGreUnlockDevice 0x000040B6L
   #define GreUnlockDevice(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreUnlockDevice)
   #define NGreDeath 0x000040B7L
   #define GreDeath(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreDeath)
   #define NGreResurrection 0x000040B8L
   #define GreResurrection(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PBYTE)(c),0L,NGreResurrection)
   #define NGreGetDCOrigin 0x000040BAL
   #define GreGetDCOrigin(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreGetDCOrigin)
   #define NGreDeviceSetDCOrigin 0x000040BBL
   #define GreDeviceSetDCOrigin(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreDeviceSetDCOrigin)
   #define NGreGetLineOrigin 0x000040BCL
   #define GreGetLineOrigin(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),0L,NGreGetLineOrigin)
   #define NGreSetLineOrigin 0x000040BDL
   #define GreSetLineOrigin(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(LONG)(c),0L,NGreSetLineOrigin)
#endif

#ifdef INCL_GRE_DEVMISC2
   #define NGreGetStyleRatio 0x000040BEL
   #define GreGetStyleRatio(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PBYTE)(b),0L,NGreGetStyleRatio)
#endif

#ifdef INCL_GRE_COLORTABLE
   #define NGreQueryColorData 0x000060C3L
   #define GreQueryColorData(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PLONG)(c),0L,NGreQueryColorData)
   #define NGreQueryLogColorTable 0x000060C4L
   #define GreQueryLogColorTable(a,b,c,d,e) (LONG) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PLONG)(e),0L,NGreQueryLogColorTable)
   #define NGreCreateLogColorTable 0x000060C5L
   #define GreCreateLogColorTable(a,b,c,d,e,f) (BOOL) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(LONG)(d),(ULONG)(LONG)(e),(ULONG)(PLONG)(f),0L,NGreCreateLogColorTable)
   #define NGreRealizeColorTable 0x000060C6L
   #define GreRealizeColorTable(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreRealizeColorTable)
   #define NGreUnrealizeColorTable 0x000060C7L
   #define GreUnrealizeColorTable(a) (BOOL) Gre32Entry3((ULONG)(HDC)(a),0L,NGreUnrealizeColorTable)
   #define NGreQueryRealColors 0x000040C8L
   #define GreQueryRealColors(a,b,c,d,e) (LONG) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),(ULONG)(LONG)(d),(ULONG)(PLONG)(e),0L,NGreQueryRealColors)
   #define NGreQueryNearestColor 0x000040C9L
   #define GreQueryNearestColor(a,b,c) (LONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(COLOR)(c),0L,NGreQueryNearestColor)
   #define NGreQueryColorIndex 0x000060CAL
   #define GreQueryColorIndex(a,b,c) (LONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(COLOR)(c),0L,NGreQueryColorIndex)
   #define NGreQueryRGBColor 0x000060CBL
   #define GreQueryRGBColor(a,b,c) (LONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(COLOR)(c),0L,NGreQueryRGBColor)
#endif

#ifdef INCL_GRE_DEVICE
   #ifndef NGreQueryDevFntMetrics
      #define NGreQueryDevFntMetrics 0x000060CCL
      #define GreQueryDeviceFontMetrics(a,b,c) (ULONG)Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(LONG)(c),0L,NGreQueryDevFntMetrics)
   #endif
#endif

#ifdef INCL_GRE_PAX
   #define NGreDeviceEnableYInversion 0x000040CDL
   #define GreDeviceEnableYInversion(a,b) (ULONG) Gre32Entry4((ULONG)(a),(LONG)(b),0L,NGreDeviceEnableYInversion)
#endif

#ifdef INCL_GRE_DEVICE
   #define NGreQueryDeviceBitmaps 0x000040D0L
   #define GreQueryDeviceBitmaps(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PBITMAPFORMAT)(b),(ULONG)(LONG)(c),0L,NGreQueryDeviceBitmaps)
   #define NGreQueryDeviceCaps 0x000040D1L
   #define GreQueryDeviceCaps(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PLONG)(c),(ULONG)(LONG)(d),0L,NGreQueryDeviceCaps)
   #define NGreEscape 0x000040D2L
   #define GreEscape(a,b,c,d,e,f) (LONG) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),(ULONG)(PBYTE)(d),(ULONG)(PLONG)(e),(ULONG)(PBYTE)(f),0L,NGreEscape)
   #define NGreQueryHardcopyCaps 0x000040D3L
   #define GreQueryHardcopyCaps(a,b,c,d) (LONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(LONG)(c),(ULONG)(PHCINFO)(d),0L,NGreQueryHardcopyCaps)
   #define NGreQueryDevResource2 0x000040D4L
   #define GreQueryDevResource2(a,b,c) (PVOID) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),0L,NGreQueryDevResource2)
   #define NGreUnloadPD 0x0000026EL
   #define GreUnloadPD(a,b) (PVOID) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreUnloadPD)
#endif

#ifdef INCL_GRE_SCREEN
   #define NGreOpenScreenChangeArea 0x00004012L
   #define GreOpenScreenChangeArea(a) (ULONG) Gre32Entry3((ULONG)(HDC)(a),0L,NGreOpenScreenChangeArea)
   #define NGreGetScreenChangeArea 0x00004013L
   #define GreGetScreenChangeArea(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),0L,NGreGetScreenChangeArea)
   #define NGreCloseScreenChangeArea 0x00004014L
   #define GreCloseScreenChangeArea(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreCloseScreenChangeArea)
   #define NGreGetScreenBits 0x0000401DL
   #define GreGetScreenBits(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(ULONG)(e),0L,NGreGetScreenBits)
   #define NGreSetScreenBits 0x0000401EL
   #define GreSetScreenBits(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),0L,NGreSetScreenBits)
#endif

#ifdef INCL_GRE_PALETTE
   #define NGreDeviceCreatePalette 0x000040D5L
   #define GreDeviceCreatePalette(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPALETTEINFO)(b),(ULONG)(HDEVPAL)(c),0L,NGreDeviceCreatePalette)
   #define NGreDeviceDeletePalette 0x000040D6L
   #define GreDeviceDeletePalette(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HDEVPAL)(b),0L,NGreDeviceDeletePalette)
   #define NGreDeviceSetPaletteEntries 0x000040D7L
   #define GreDeviceSetPaletteEntries(a,b,c,d,e,f) (ULONG) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(HDEVPAL)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(ULONG)(e),(ULONG)(PULONG)(f),0L,NGreDeviceSetPaletteEntries)
   #define NGreDeviceAnimatePalette 0x000040D8L
   #define GreDeviceAnimatePalette(a,b,c,d,e,f) (ULONG) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(HDEVPAL)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(ULONG)(e),(ULONG)(PULONG)(f),0L,NGreDeviceAnimatePalette)
   #define NGreDeviceResizePalette 0x000040D9L
   #define GreDeviceResizePalette(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HDEVPAL)(b),(ULONG)(ULONG)(c),0L,NGreDeviceResizePalette)
   #define NGreRealizePalette 0x000040DAL
   #define GreRealizePalette(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PULONG)(c),0L,NGreRealizePalette)
   #define NGreQueryHWPaletteInfo 0x000040DBL
   #define GreQueryHWPaletteInfo(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(PULONG)(d),0L,NGreQueryHWPaletteInfo)
   #define NGreUpdateColors 0x000040DCL
   #define GreUpdateColors(a) (ULONG) Gre32Entry3((ULONG)(HDC)(a),0L,NGreUpdateColors)
   #define NGreQueryPaletteRealization 0x000040DDL
   #define GreQueryPaletteRealization(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(PULONG)(d),0L,NGreQueryPaletteRealization)
   #define NGreGetVisRects 0x000040DEL
   #define GreGetVisRects(a,b,c,d) (INT) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),(ULONG)(PRGNRECT)(c),(ULONG)(PRECTL)(d),0L,NGreGetVisRects)
   #define NGreQueryNearestPaletteIndex 0x00000267L
   #define GreQueryNearestPaletteIndex(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRGB2)(b),0L,NGreQueryNearestPaletteIndex)
#endif

#ifdef INCL_GRE_DCS
   #define NGreResetDC 0x00004202L
   #define GreResetDC(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreResetDC)
   #define NGreResetDC2 0x0000426BL
   #define GreResetDC2(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreResetDC2)
   #define NGreGetHandle 0x00004203L
   #define GreGetHandle(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreGetHandle)
   #define NGreSetHandle 0x00004204L
   #define GreSetHandle(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LHANDLE)(b),(ULONG)(ULONG)(c),0L,NGreSetHandle)
   #define NGreGetProcessControl 0x00004205L
   #define GreGetProcessControl(a) (LONG) Gre32Entry3((ULONG)(HDC)(a),0L,NGreGetProcessControl)
   #define NGreSetProcessControl 0x00004206L
   #define GreSetProcessControl(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),0L,NGreSetProcessControl)
   #define NGreSaveDC 0x00004207L
   #define GreSaveDC(a) (LONG) Gre32Entry3((ULONG)(HDC)(a),0L,NGreSaveDC)
   #define NGreRestoreDC 0x00004208L
   #define GreRestoreDC(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LONG)(b),0L,NGreRestoreDC)
   #define NGreQueryDeviceNames 0x00000209L
   #define GreQueryDeviceNames(a,b,c,d,e,f) (BOOL) Gre32Entry8((ULONG)(PSZ)(a),(ULONG)(PLONG)(b),(ULONG)(PSTR32)(c),(ULONG)(PSTR64)(d),(ULONG)(PLONG)(e),(ULONG)(PSTR16)(f),0L,NGreQueryDeviceNames)
   #define NGreQueryEngineVersion 0x0000020AL
   #define GreQueryEngineVersion() (LONG) Gre32Entry2(0L,NGreQueryEngineVersion)
   #define NGreSetDCOwner 0x0000420BL
   #define GreSetDCOwner(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreSetDCOwner)
#endif

#ifdef INCL_GRE_DEVSUPPORT
   #define NGreQueryBitmapSelection 0x0000020CL
   #define GreQueryBitmapSelection(a) (HDC) Gre32Entry3((ULONG)(HBITMAP)(a),0L,NGreQueryBitmapSelection)
   #define NGreGetBitmapInfoHeader 0x0000020DL
   #define GreGetBitmapInfoHeader(a,b) (BOOL) Gre32Entry4((ULONG)(HBITMAP)(a),(ULONG)(PBITMAPINFOHEADER2)(b),0L,NGreGetBitmapInfoHeader)
   #define NGreConvertBitmapData 0x0000420EL
   #define GreConvertBitmapData(a,b,c,d,e,f,g) (ULONG) Gre32Entry9((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(PBITMAPINFO2)(d),(ULONG)(PBYTE)(e),(ULONG)(PBITMAPINFO2)(f),(ULONG)(PBYTE)(g),0L,NGreConvertBitmapData)
   #define NGreInvalidateVisRegion 0x0000420FL
   #define GreInvalidateVisRegion(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PDC_BLOCK)(c),0L,NGreInvalidateVisRegion)
   #define NGreCreateBitmap 0x00004210L
   #define GreCreateBitmap(a,b,c,d,e) (HBITMAP) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(PBITMAPINFOHEADER2)(b),(ULONG)(ULONG)(c),(ULONG)(PBYTE)(d),(ULONG)(PBITMAPINFO2)(e),0L,NGreCreateBitmap)
   #define NGreDeleteBitmap 0x00000211L
   #define GreDeleteBitmap(a) (BOOL) Gre32Entry3((ULONG)(HBITMAP)(a),0L,NGreDeleteBitmap)
   #define NGreSelectBitmap 0x00004212L
   #define GreSelectBitmap(a,b) (HBITMAP) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HBITMAP)(b),0L,NGreSelectBitmap)
   #define NGreGetBitmapParameters 0x00000213L
   #define GreGetBitmapParameters(a,b) (BOOL) Gre32Entry4((ULONG)(HBITMAP)(a),(ULONG)(PBITMAPINFOHEADER)(b),0L,NGreGetBitmapParameters)
   #define NGreGetBitmapDimension 0x00000214L
   #define GreGetBitmapDimension(a,b) (BOOL) Gre32Entry4((ULONG)(HBITMAP)(a),(ULONG)(PSIZEL)(b),0L,NGreGetBitmapDimension)
   #define NGreSetBitmapDimension 0x00000215L
   #define GreSetBitmapDimension(a,b) (BOOL) Gre32Entry4((ULONG)(HBITMAP)(a),(ULONG)(PSIZEL)(b),0L,NGreSetBitmapDimension)
   #define NGreSetCursor 0x00004216L
   #define GreSetCursor(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(PPOINTL)(b),(ULONG)(HBITMAP)(c),0L,NGreSetCursor)
   #define NGreGetAttributes 0x00004217L
   #define GreGetAttributes(a,b,c,d) (LONG) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(PBUNDLE)(d),0L,NGreGetAttributes)
   #define NGreSetAttributes 0x00004218L
   #define GreSetAttributes(a,b,c,d,e) (BOOL) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),(ULONG)(PBUNDLE)(e),0L,NGreSetAttributes)
   #define NGreSetGlobalAttribute 0x00004219L
   #define GreSetGlobalAttribute(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(ULONG)(d),0L,NGreSetGlobalAttribute)
   #define NGreSetBitmapOwner 0x0000021AL
   #define GreSetBitmapOwner(a,b) (BOOL) Gre32Entry4((ULONG)(HBITMAP)(a),(ULONG)(ULONG)(b),0L,NGreSetBitmapOwner)
   #define NGreGetDefaultAttributes 0x0000421BL
   #define GreGetDefaultAttributes(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(PBUNDLE)(d),0L,NGreGetDefaultAttributes)
   #define NGreSetDefaultAttributes 0x0000421CL
   #define GreSetDefaultAttributes(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(ULONG)(c),(ULONG)(PBUNDLE)(d),0L,NGreSetDefaultAttributes)
   #define NGreGetDefaultArcParameters 0x0000421DL
   #define GreGetDefaultArcParameters(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PARCPARAMS)(b),0L,NGreGetDefaultArcParameters)
   #define NGreSetDefaultArcParameters 0x0000421EL
   #define GreSetDefaultArcParameters(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PARCPARAMS)(b),0L,NGreSetDefaultArcParameters)
   #define NGreGetDefaultViewingLimits 0x0000421FL
   #define GreGetDefaultViewingLimits(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreGetDefaultViewingLimits)
   #define NGreSetDefaultViewingLimits 0x00004220L
   #define GreSetDefaultViewingLimits(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(PRECTL)(b),0L,NGreSetDefaultViewingLimits)
   #define NGreInitializeAttributes 0x00004221L
   #define GreInitializeAttributes(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreInitializeAttributes)
#endif

#ifdef INCL_GRE_SETID
   #define NGreDeleteSetId 0x00004222L
   #define GreDeleteSetId(a,b) (BOOL) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LCID)(b),0L,NGreDeleteSetId)
   #define NGreQueryNumberSetIds 0x00004223L
   #define GreQueryNumberSetIds(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(ULONG)(b),0L,NGreQueryNumberSetIds)
   #define NGreQuerySetIds 0x00004224L
   #define GreQuerySetIds(a,b,c,d,e,f) (BOOL) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PLONG)(c),(ULONG)(PSTR8)(d),(ULONG)(PLCID)(e),(ULONG)(ULONG)(f),0L,NGreQuerySetIds)
#endif

#ifdef INCL_GRE_LCID
   #define NGreQueryBitmapHandle 0x00000225L
   #define GreQueryBitmapHandle(a,b) (HBITMAP) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LCID)(b),0L,NGreQueryBitmapHandle)
   #define NGreSetBitmapID 0x00004226L
   #define GreSetBitmapID(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(HBITMAP)(b),(ULONG)(LCID)(c),0L,NGreSetBitmapID)
   #define NGreCopyDCLoadData 0x00004227L
   #define GreCopyDCLoadData(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(HDC)(c),0L,NGreCopyDCLoadData)
#endif

#ifdef INCL_GRE_SETID
   #define NGreQuerySetIdUsage 0x00004228L
   #define GreQuerySetIdUsage(a,b) (LONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LCID)(b),0L,NGreQuerySetIdUsage)
#endif

#ifdef INCL_GRE_FONTS
   #define NGreQueryFullFontFileDescriptions 0x0000022BL
   #define GreQueryFullFontFileDescriptions(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(PSZ)(a),(ULONG)(PULONG)(b),(ULONG)(PVOID)(c),(ULONG)(PLONG)(d),0L,NGreQueryFullFontFileDescriptions)
   #define NGreQueryLogicalFont 0x0000422CL
   #define GreQueryLogicalFont(a,b,c,d,e) (BOOL) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(LCID)(b),(ULONG)(PSTR8)(c),(ULONG)(PFATTRS)(d),(ULONG)(LONG)(e),0L,NGreQueryLogicalFont)
   #define NGreCreateLogicalFont 0x0000422DL
   #define GreCreateLogicalFont(a,b,c,d) (BOOL) Gre32Entry6((ULONG)(HDC)(a),(ULONG)(LCID)(b),(ULONG)(PSTR8)(c),(ULONG)(PFATTRS)(d),0L,NGreCreateLogicalFont)
   #define NGreLoadFont 0x0000022EL
   #define GreLoadFont(a) (BOOL) Gre32Entry3((ULONG)(PSZ)(a),0L,NGreLoadFont)
   #define NGreUnloadFont 0x0000022FL
   #define GreUnloadFont(a) (BOOL) Gre32Entry3((ULONG)(PSZ)(a),0L,NGreUnloadFont)
   #define NGreQueryFonts 0x00004230L
   #define GreQueryFonts(a,b,c,d,e,f) (LONG) Gre32Entry8((ULONG)(HDC)(a),(ULONG)(ULONG)(b),(ULONG)(PSZ)(c),(ULONG)(PFONTMETRICS)(d),(ULONG)(LONG)(e),(ULONG)(PLONG)(f),0L,NGreQueryFonts)
   #define NGreQueryFontAttributes 0x00004231L
   #define GreQueryFontAttributes(a,b,c) (BOOL) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(LONG)(b),(ULONG)(PFONTMETRICS)(c),0L,NGreQueryFontAttributes)
   #define NGreInstallIFI 0x00000232L
   #define GreInstallIFI(a,b,c,d) (LONG) Gre32Entry6((ULONG)(ULONG)(a),(ULONG)(PSZ)(b),(ULONG)(PSZ)(c),(ULONG)(PSZ)(d),0L,NGreInstallIFI)
   #define NGreLoadPublicFont 0x00000233L
   #define GreLoadPublicFont(a) (BOOL) Gre32Entry3((ULONG)(PSZ)(a),0L,NGreLoadPublicFont)
   #define NGreUnLoadPublicFont 0x00000234L
   #define GreUnLoadPublicFont(a) (BOOL) Gre32Entry3((ULONG)(PSZ)(a),0L,NGreUnLoadPublicFont)
   #define NGreQueryCodePageVector 0x00000235L
   #define GreQueryCodePageVector(a) (ULONG) Gre32Entry3((ULONG)(ULONG)(a),0L,NGreQueryCodePageVector)
   #define NGreQueryFontFileDescriptions 0x00000236L
   #define GreQueryFontFileDescriptions(a,b,c) (ULONG) Gre32Entry5((ULONG)(PSZ)(a),(ULONG)(PLONG)(b),(ULONG)(PFFDESCS)(c),0L,NGreQueryFontFileDescriptions)
   #define NGreQueryFaceString 0x00004237L
   #define GreQueryFaceString(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HDC)(a),(ULONG)(PSZ)(b),(ULONG)(PFACENAMEDESC)(c),(ULONG)(ULONG)(d),(ULONG)(PSZ)(e),0L,NGreQueryFaceString)
   #define NGreQueryFontAction 0x00000238L
   #define GreQueryFontAction(a) (ULONG) Gre32Entry3((ULONG)(ULONG)(a),0L,NGreQueryFontAction)
   #define NGreQueryCodePageObject 0x00000255L
   #define GreQueryCodePageObject(a,b,c) (ULONG)Gre32Entry5((ULONG)(a),(ULONG)(PSZ)(b),(ULONG)(PCODEPAGEOBJECT *)(c),0L,NGreQueryCodePageObject)
#endif

#ifdef INCL_GRE_JOURNALING
   #define NGreAccumulateJournalFile 0x00000239L
   #define GreAccumulateJournalFile(a) (ULONG) Gre32Entry3((ULONG)(ULONG)(a),0L,NGreAccumulateJournalFile)
   #define NGreCreateJournalFile 0x0000023AL
   #define GreCreateJournalFile(a,b,c) (ULONG) Gre32Entry5((ULONG)(PSZ)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),0L,NGreCreateJournalFile)
   #define NGreDeleteJournalFile 0x0000023BL
   #define GreDeleteJournalFile(a) (ULONG) Gre32Entry3((ULONG)(LHANDLE)(a),0L,NGreDeleteJournalFile)
   #define NGreStartJournalFile 0x0000023CL
   #define GreStartJournalFile(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LHANDLE)(b),0L,NGreStartJournalFile)
   #define NGreStopJournalFile 0x0000023DL
   #define GreStopJournalFile(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LHANDLE)(b),0L,NGreStopJournalFile)
   #define NGrePlayJournalFile 0x0000023EL
   #define GrePlayJournalFile(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(LHANDLE)(b),0L,NGrePlayJournalFile)
   #define NGreOpenJournalFile 0x0000023FL
   #define GreOpenJournalFile(a,b,c) (ULONG) Gre32Entry5((ULONG)(PSZ)(a),(ULONG)(ULONG)(b),(ULONG)(LONG)(c),0L,NGreOpenJournalFile)
#endif

#ifdef INCL_GRE_HFONT
   #define NGreCreateFontHandle 0x00000250L
   #define GreCreateFontHandle(a,b,c) (HFONT) Gre32Entry5((ULONG)(ULONG)(a),(ULONG)(PSTR8)(b),(ULONG)(PFATTRS)(c),0L,NGreCreateFontHandle)
   #define NGreQueryFontHandle 0x00000251L
   #define GreQueryFontHandle(a,b,c,d,e) (ULONG) Gre32Entry7((ULONG)(HFONT)(a),(ULONG)(PULONG)(b),(ULONG)(PSTR8)(c),(ULONG)(PFATTRS)(d),(ULONG)(ULONG)(e),0L,NGreQueryFontHandle)
   #define NGreSelectFontHandle 0x00004252L
   #define GreSelectFontHandle(a,b) (HFONT) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(HFONT)(b),0L,NGreSelectFontHandle)
   #define NGreDeleteFontHandle 0x00000253L
   #define GreDeleteFontHandle(a) (ULONG) Gre32Entry3((ULONG)(HFONT)(a),0L,NGreDeleteFontHandle)
   #define NGreSetFontHandleOwner 0x00000254L
   #define GreSetFontHandleOwner(a,b) (ULONG) Gre32Entry4((ULONG)(HFONT)(a),(ULONG)(ULONG)(b),0L,NGreSetFontHandleOwner)
#endif

#ifdef INCL_GRE_PAX
   #define NGrePaxGetPolyFillMode  0x0000425CL
   #define GrePaxGetPolyFillMode(a) (ULONG) Gre32Entry3((ULONG)(a),0L,NGrePaxGetPolyFillMode)
   #define NGrePaxSetPolyFillMode  0x0000425DL
   #define GrePaxSetPolyFillMode(a,b) (ULONG) Gre32Entry4((ULONG)(a),(ULONG)(b),0L,NGrePaxSetPolyFillMode)
   #define NGreQueryMitreLimit  0x0000425EL
   #define GreQueryMitreLimit(a) (FIXED) Gre32Entry3((ULONG)(a),0L,NGreQueryMitreLimit)
   #define NGreSetMitreLimit  0x0000425FL
   #define GreSetMitreLimit(a,b) (FIXED) Gre32Entry4((ULONG)(a),(ULONG)(b),0L,NGreSetMitreLimit)
   #define NGreEnableYInversion 0x00004260L
   #define GreEnableYInversion(a,b) (ULONG) Gre32Entry4((ULONG)(a),(LONG)(b),0L,NGreEnableYInversion)
   #define NGreQueryYInversion 0x00004261L
   #define GreQueryYInversion(a) (ULONG) Gre32Entry3((ULONG)(a),0L,NGreQueryYInversion)
   #define NGreQueryPath  0x00004262L
   #define GreQueryPath(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(a),(ULONG)(b),(ULONG)(c),(ULONG)(d),0L,NGreQueryPath)
   #define NGreAllocateDCData  0x00004263L
   #define GreAllocateDCData(a,b) (ULONG) Gre32Entry4((ULONG)(a),(ULONG)(b),0L,NGreAllocateDCData)
   #define NGreQueryDCData  0x00004264L
   #define GreQueryDCData(a) (ULONG) Gre32Entry3((ULONG)(a),0L,NGreQueryDCData)
   #define NGreQueryRasterCaps 0x00000265L
   #define GreQueryRasterCaps(a) (ULONG) Gre32Entry3((ULONG)(a), 0L, NGreQueryRasterCaps)
   #define NGrePaxStrokeAndFillPath 0x00004266L
   #define GrePaxStrokeAndFillPath(a,b,c,d) (ULONG) Gre32Entry6((ULONG)(a),(ULONG)(b),(ULONG)(c),(ULONG)(d), 0L, NGrePaxStrokeAndFillPath)
#endif

#ifdef __cplusplus
      }
#endif

