#include <os2.h>

/*
ULONG APIENTRY GpiQueryFontMappingFlags(HPS);
BOOL  APIENTRY GpiQueryRasterizerCaps(PRASTERIZERCAPS);
ULONG APIENTRY GpiSetFontMappingFlags(HPS,ULONG);
LONG   APIENTRY GpiQueryNearestPaletteIndex(HPAL,ULONG);
ULONG  APIENTRY GpiResizePalette(HPAL,ULONG);
LONG   APIENTRY GpiOval(HPS,PPOINTL,FIXED,FIXED,ULONG);
LONG   APIENTRY GpiQueryTabbedTextExtent(HPS,LONG,PCH,LONG,PULONG);
LONG   APIENTRY GpiTabbedCharStringAt(HPS,PPOINTL,PRECTL,ULONG,LONG,PCH,LONG,PULONG,LONG);
HRGN   APIENTRY GpiCreateEllipticRegion(HPS hps, PRECTL prclRect);
HRGN   APIENTRY GpiCreateRoundRectRegion(HPS hps, PPOINTL pptlPt, LONG lHround, LONG lVRound);
*/

BOOL   APIENTRY GpiAssociate(HPS hps, HDC hdc)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiBeginArea(HPS hps, ULONG flOptions)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiBeginElement(void)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiBeginPath(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

LONG    APIENTRY GpiBitBlt(HPS hps,HPS,LONG,PPOINTL,LONG,ULONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiBox(HPS hps, LONG lControl, PPOINTL pptlPoint, LONG lHRound, LONG lVRound)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiCallSegmentMatrix(HPS hps,LONG,LONG,PMATRIXLF,LONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiCharString(HPS hps, LONG lCount, PCH pchString)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiCharStringAt(HPS hps, PPOINTL pptlPoint, LONG lCount, PCH pchString)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiCloseFigure(HPS)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiCloseSegment(HPS)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiCombineRegion(HPS hps, HRGN hrgnDest, HRGN hrgnSr1, HRGN hrgnSr2, LONG lMode)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiComment(HPS hps,LONG,PBYTE)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiConvert(HPS hps,LONG,LONG,LONG,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

HMF    APIENTRY GpiCopyMetaFile(HMF)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiCorrelateChain(HPS hps,LONG,PPOINTL,LONG,LONG,PLONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiCorrelateFrom(HPS hps,LONG,LONG,LONG,PPOINTL,LONG,LONG,PLONG)
{
  return unimplemented(__FUNCTION__);
}

LONG  APIENTRY GpiCreateLogFont(HPS hps,PSTR8,LONG,PFATTRS)
{
  return unimplemented(__FUNCTION__);
}

HPS    APIENTRY GpiCreatePS(HAB,HDC,PSIZEL,ULONG)
{
  return unimplemented(__FUNCTION__);
}

HRGN   APIENTRY GpiCreateRegion(HPS hps, LONG lCount, PRECTL arclRectangles)
{
  return unimplemented(__FUNCTION__);
}

BOOL    APIENTRY GpiDeleteBitmap(HBITMAP)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiDeleteElement(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiDeleteElementRange(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiDeleteElementsBetweenLabels(void)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDeleteMetaFile(HMF)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDeleteSegment(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDeleteSegments(HPS hps,LONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL  APIENTRY GpiDeleteSetId(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDestroyPS(HPS)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDrawChain(HPS)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDrawDynamics(HPS)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDrawFrom(HPS hps,LONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDrawSegment(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiElement(void)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiEndArea(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiEndElement(void)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiEndPath(HPS)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiEqualRegion(HPS hps, HRGN hrgnSrc1, HRGN hrgnSrc2)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiErase(HPS)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiErrorSegmentData(void)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiExcludeClipRectangle(HPS hps, PRECTL prclRectangle)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiFillPath(HPS hps,LONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiFullArc(HPS hps,LONG,FIXED)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiGetData(HPS hps,LONG,PLONG,LONG,LONG,PBYTE)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiImage(HPS hps,LONG,PSIZEL,LONG,PBYTE)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiIntersectClipRectangle(HPS hps, PRECTL prclRectangle)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiLabel(void)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiLine(HPS hps, PPOINTL pptlEndPoint)
{
  return unimplemented(__FUNCTION__);
}

BOOL  APIENTRY GpiLoadFonts(HAB,PCSZ)
{
  return unimplemented(__FUNCTION__);
}

HMF    APIENTRY GpiLoadMetaFile(HAB,PCSZ)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiMarker(HPS hps,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiModifyPath(HPS hps,LONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiMove(HPS hps, PPOINTL pptlPoint)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiOffsetClipRegion(HPS hps, PPOINTL pptlPoint)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiOffsetElementPointer(void)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiOffsetRegion(HPS hps, HRGN Hrgn, PPOINTL pptlOffset)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiOpenSegment(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPaintRegion(HPS hps, HRGN hrgn)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPlayMetaFile(HPS hps,HMF,LONG,PLONG,PLONG,LONG,PCSZ)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPointArc(HPS hps,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPolyFillet(HPS hps,LONG,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPolyFilletSharp(HPS hps,LONG,PPOINTL,PFIXED)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPolyLine(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPolyMarker(HPS hps,LONG,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPolySpline(HPS hps,LONG,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiPop(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPtInRegion(HPS hps, HRGN hrgn, PPOINTL pptlPoint)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPtVisible(HPS hps,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPutData(HPS hps,LONG,PLONG,PBYTE)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiQueryArcParams(void)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryAttrMode(HPS)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryBackColor(HPS)
{
  return unimplemented(__FUNCTION__)
}

LONG   APIENTRY GpiQueryBackMix(HPS)
{
  return unimplemented(__FUNCTION__);
}

BOOL    APIENTRY GpiQueryBitmapDimension(HBITMAP,PSIZEL)
{
  return unimplemented(__FUNCTION__);
}

HBITMAP APIENTRY GpiQueryBitmapHandle(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryBoundaryData(HPS hps,PRECTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryCharAngle(HPS hps,PGRADIENTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryCharBox(HPS hps,PSIZEF)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryCharDirection(HPS)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryCharMode(HPS)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryCharSet(HPS)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryCharShear(HPS hps,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryClipBox(HPS hps, PRECTL prclBound)
{
  return unimplemented(__FUNCTION__);
}

HRGN   APIENTRY GpiQueryClipRegion(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryColor(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryColorData(HPS hps,LONG,PLONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryColorIndex(HPS hps,ULONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

ULONG APIENTRY GpiQueryCp(HPS)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryCurrentPosition(HPS hps,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryDefCharBox(HPS hps,PSIZEL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryDefaultViewMatrix(HPS hps,LONG,PMATRIXLF)
{
  return unimplemented(__FUNCTION__);
}

HDC    APIENTRY GpiQueryDevice(HPS)
{
  return unimplemented(__FUNCTION__);
}

BOOL    APIENTRY GpiQueryDeviceBitmapFormats(HPS hps,LONG,PLONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryDrawControl(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryDrawingMode(HPS)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiQueryEditMode(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiQueryElement(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiQueryElementPointer(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiQueryElementType(void)
{
  return unimplemented(__FUNCTION__);
}

LONG  APIENTRY GpiQueryFontFileDescriptions(HAB,PCSZ,PLONG,PFFDESCS)
{
  return unimplemented(__FUNCTION__);
}

BOOL  APIENTRY GpiQueryFontMetrics(HPS hps,LONG,PFONTMETRICS)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryGraphicsField(HPS hps,PRECTL)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryInitialSegmentAttrs(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

LONG  APIENTRY GpiQueryKerningPairs(HPS hps,LONG,PKERNINGPAIRS)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryLineEnd(HPS)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryLineJoin(HPS)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryLineType(HPS)
{
  return unimplemented(__FUNCTION__);
}

FIXED  APIENTRY GpiQueryLineWidth(HPS)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryLineWidthGeom(HPS)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryMarker(HPS)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryMarkerBox(HPS hps,PSIZEF)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryMarkerSet(HPS)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryMetaFileBits(HMF,LONG,LONG,PBYTE)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryMetaFileLength(HMF)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryMix(HPS)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryModelTransformMatrix(HPS hps,LONG,PMATRIXLF)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryNearestColor(HPS hps,ULONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

LONG  APIENTRY GpiQueryNumberSetIds(HPS)
{
  return unimplemented(__FUNCTION__);
}

ULONG  APIENTRY GpiQueryPS(HPS hps,PSIZEL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryPageViewport(HPS hps,PRECTL)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryPattern(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryPatternRefPoint(HPS hps,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryPatternSet(HPS)
{
  return unimplemented(__FUNCTION__);
}

LONG    APIENTRY GpiQueryPel(HPS hps,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryPickAperturePosition(HPS hps,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryPickApertureSize(HPS hps,PSIZEL)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryRGBColor(HPS hps,ULONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryRealColors(HPS hps,ULONG,LONG,LONG,PLONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryRegionBox(HPS hps, HRGN hrgn, PRECTL prclBound)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQuerySegmentAttrs(HPS hps,LONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQuerySegmentNames(HPS hps,LONG,LONG,LONG,PLONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQuerySegmentPriority(HPS hps,LONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQuerySegmentTransformMatrix(HPS hps,LONG,LONG,PMATRIXLF)
{
  return unimplemented(__FUNCTION__);
}

BOOL  APIENTRY GpiQuerySetIds(HPS hps,LONG,PLONG,PSTR8,PLONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryStopDraw(HPS)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryTag(HPS hps,PLONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryTextBox(HPS hps,LONG,PCH,LONG,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryViewingLimits(HPS hps,PRECTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryViewingTransformMatrix(HPS hps,LONG,PMATRIXLF)
{
  return unimplemented(__FUNCTION__);
}

BOOL  APIENTRY GpiQueryWidthTable(HPS hps,LONG,LONG,PLONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiRectInRegion(HPS hps, HRGN hrgn, PRECTL prclRect)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiRectVisible(HPS hps,PRECTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiRemoveDynamics(HPS hps,LONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiResetBoundaryData(HPS)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiResetPS(HPS hps,ULONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiRestorePS(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSaveMetaFile(HMF,PCSZ)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiSavePS(HPS)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetArcParams(HPS hps,PARCPARAMS)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetAttrMode(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetBackColor(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetBackMix(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

HBITMAP APIENTRY GpiSetBitmap(HPS hps,HBITMAP)
{
  return unimplemented(__FUNCTION__);
}

BOOL    APIENTRY GpiSetBitmapDimension(HBITMAP,PSIZEL)
{
  return unimplemented(__FUNCTION__);
}

BOOL    APIENTRY GpiSetBitmapId(HPS hps,HBITMAP,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetCharAngle(HPS hps,PGRADIENTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetCharBox(HPS hps,PSIZEF)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetCharDirection(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetCharMode(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetCharSet(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetCharShear(HPS hps,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetClipPath(HPS hps,LONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiSetClipRegion(HPS hps, HRGN hrgn, PHRGN phrgnOld)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetColor(HPS hps, LONG lColor)
{
  return unimplemented(__FUNCTION__);
}

BOOL  APIENTRY GpiSetCp(HPS hps,ULONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetCurrentPosition(HPS hps,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetDefaultViewMatrix(HPS hps,LONG,PMATRIXLF,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetDrawControl(HPS hps,LONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetDrawingMode(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiSetEditMode(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiSetElementPointer(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiSetElementPointerAtLabel(void)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetGraphicsField(HPS hps,PRECTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetInitialSegmentAttrs(HPS hps,LONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetLineEnd(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetLineJoin(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetLineType(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetLineWidth(HPS hps,FIXED)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetLineWidthGeom(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetMarker(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetMarkerBox(HPS hps,PSIZEF)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetMarkerSet(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetMetaFileBits(HMF,LONG,LONG,PBYTE)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetMix(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetModelTransformMatrix(HPS hps,LONG,PMATRIXLF,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetPS(HPS hps,PSIZEL,ULONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetPageViewport(HPS hps,PRECTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetPattern(HPS hps, LONG lPatternSymbol)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetPatternRefPoint(HPS hps,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetPatternSet(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

LONG    APIENTRY GpiSetPel(HPS hps,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetPickAperturePosition(HPS hps,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetRegion(HPS hps, HRGN hrgn, LONG lcount, PRECTL arclRectangles)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetSegmentAttrs(HPS hps,LONG,LONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetSegmentPriority(HPS hps,LONG,LONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetPageViewport(HPS hps,PRECTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetStopDraw(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetTag(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetViewingLimits(HPS hps,PRECTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetViewingTransformMatrix(HPS hps,LONG,PMATRIXLF,LONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiStrokePath(HPS hps,LONG,ULONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL  APIENTRY GpiUnloadFonts(HAB,PCSZ)
{
  return unimplemented(__FUNCTION__);
}

LONG    APIENTRY GpiWCBitBlt(HPS hps,HBITMAP,LONG,PPOINTL,LONG,ULONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPolyLineDisjoint(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
  return unimplemented(__FUNCTION__);
}

HRGN   APIENTRY GpiPathToRegion(HPS hps,LONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

LONG    APIENTRY GpiFloodFill(HPS hps,LONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSuspendPlay(HPS)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiResumePlay(HPS)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiOutlinePath(HPS hps,LONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiTranslate(HPS hps,PMATRIXLF,LONG,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiScale(HPS hps,PMATRIXLF,LONG,PFIXED,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiRotate(HPS hps,PMATRIXLF,LONG,FIXED,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryDefArcParams(HPS hps,PARCPARAMS)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryDefTag(HPS hps,PLONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryDefViewingLimits(HPS hps,PRECTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetDefArcParams(HPS hps,PARCPARAMS)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetDefTag(HPS hps,LONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetDefViewingLimits(HPS hps,PRECTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL    APIENTRY GpiQueryBitmapParameters(HBITMAP,PBITMAPINFOHEADER)
{
  return unimplemented(__FUNCTION__);
}

BOOL  APIENTRY GpiQueryLogicalFont(HPS hps,LONG,PSTR8,PFATTRS,LONG)
{
  return unimplemented(__FUNCTION__);
}

ULONG APIENTRY GpiQueryFaceString(HPS hps,PCSZ,PFACENAMEDESC,LONG,PCSZ)
{
  return unimplemented(__FUNCTION__);
}

ULONG APIENTRY GpiQueryFontAction(HAB,ULONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDeletePalette(HPAL)
{
  return unimplemented(__FUNCTION__);
}

HPAL   APIENTRY GpiSelectPalette(HPS hps,HPAL)
{
  return unimplemented(__FUNCTION__);
}

HPAL   APIENTRY GpiQueryPalette(HPS)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiCharStringPos(HPS hps,PRECTL,ULONG,LONG,PCH,PLONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiCharStringPosAt(HPS hps,PPOINTL,PRECTL,ULONG,LONG,PCH,PLONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiCorrelateSegment(HPS hps,LONG,LONG,PPOINTL,LONG,LONG,PLONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryAttrs(HPS hps,LONG,ULONG,PBUNDLE)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryCharStringPos(HPS hps,ULONG,LONG,PCH,PLONG,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryCharStringPosAt(HPS hps,PPOINTL,ULONG,LONG,PCH,PLONG,PPOINTL)
{
  return unimplemented(__FUNCTION__);
}

LONG  APIENTRY GpiQueryFonts(HPS hps,ULONG,PCSZ,PLONG,LONG,PFONTMETRICS)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryRegionRects(HPS hps, HRGN hrgn, PRECTL prclBound,
                   PRGNRECT prgnrcControl, PRECTL prclRect)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetAttrs(HPS hps,LONG,ULONG,ULONG,PBUNDLE)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetPickApertureSize(HPS hps,LONG,PSIZEL)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryDefAttrs(HPS hps,LONG,ULONG,PBUNDLE)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetDefAttrs(HPS hps,LONG,ULONG,PBUNDLE)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiCreateLogColorTable(HPS hps,ULONG,LONG,LONG,LONG,PLONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryLogColorTable(HPS hps,ULONG,LONG,LONG,PLONG)
{
  return unimplemented(__FUNCTION__);
}

HPAL   APIENTRY GpiCreatePalette(HAB,ULONG,ULONG,ULONG,PULONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiAnimatePalette(HPAL,ULONG,ULONG,ULONG,PULONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetPaletteEntries(HPAL,ULONG,ULONG,ULONG,PULONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryPaletteInfo(HPAL,HPS,ULONG,ULONG,ULONG,PULONG)
{
  return unimplemented(__FUNCTION__);
}

HBITMAP APIENTRY GpiCreateBitmap(HPS hps,PBITMAPINFOHEADER2,ULONG,PBYTE,PBITMAPINFO2)
{
  return unimplemented(__FUNCTION__);
}

LONG    APIENTRY GpiQueryBitmapBits(HPS hps,LONG,LONG,PBYTE,PBITMAPINFO2)
{
  return unimplemented(__FUNCTION__);
}

BOOL    APIENTRY GpiQueryBitmapInfoHeader(HBITMAP,PBITMAPINFOHEADER2)
{
  return unimplemented(__FUNCTION__);
}

LONG    APIENTRY GpiSetBitmapBits(HPS hps,LONG,LONG,PBYTE,PBITMAPINFO2)
{
  return unimplemented(__FUNCTION__);
}

LONG    APIENTRY GpiDrawBits(HPS hps,PVOID,PBITMAPINFO2,LONG,PPOINTL,LONG,ULONG)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY DevCloseDC(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY DevEscape(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY DevQueryCaps(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY DevQueryDeviceNames(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY DevQueryHardcopyCaps(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY DevPostDeviceModes(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY DevOpenDC(void)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDestroyRegion(HPS hps, HRGN hrgn)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPartialArc(HPS hps,PPOINTL,FIXED,FIXED,FIXED)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryCharExtra(HPS hps,PFIXED)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetCharExtra(HPS hps,FIXED)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryCharBreakExtra(HPS hps,PFIXED)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetCharBreakExtra(HPS hps,FIXED)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiFrameRegion(HPS hps, HRGN hrgn, PSIZEL thickness)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiConvertWithMatrix(HPS hps,LONG,PPOINTL,LONG,PMATRIXLF)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiSetDCOwner(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiSetBitmapOwner(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiSetRegionOwner(void)
{
  return unimplemented(__FUNCTION__);
}

BOOL  APIENTRY GpiLoadPublicFonts(HAB,PCSZ)
{
  return unimplemented(__FUNCTION__);
}

BOOL  APIENTRY GpiUnloadPublicFonts(HAB,PCSZ)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiInstallIFIFont(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc625(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc626(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiOpenMetafile(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiCloseMetafile(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiConvPSH(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc630(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc631(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc632(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc633(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc634(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc635(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc636(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc637(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc638(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc639(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc640(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc641(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc642(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc643(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiMtAssociate(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiMtDisassociate(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc646(void)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryTextAlignment(HPS hps,PLONG,PLONG)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetTextAlignment(HPS hps,LONG,LONG)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPolygons(HPS hps, ULONG ulCount, PPOLYGON paplgn, ULONG flOpts, ULONG flMdl)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc651(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc652(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc653(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY DspDefaultResolution(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY DevQueryDisplayResolutions(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY DspInitSystemDriverName(void)
{
  return unimplemented(__FUNCTION__);
}

LONG  APIENTRY GpiQueryFullFontFileDescs(HAB,PCSZ,PLONG,PVOID,PLONG)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc658(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc659(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc660(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc661(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc662(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc663(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiAllocateDCData(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiQueryDCData(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc666(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc667(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc668(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc669(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GPIBEGININKPATH(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GPIENDINKPATH(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GPISTROKEINKPATH(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY PaxGetPolyFillMode(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY PaxSetPolyFillMode(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY PaxBeginPath(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY PaxCloseFigure(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY PaxEndPath(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY PaxFillPath(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY PaxFlattenPath(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY PaxPathToRegion(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY PaxStrokeAndFillPath(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY PaxStrokePath(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY PaxWidenPath(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY PaxArc(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY PaxChord(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY PaxEllipse(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY PaxLineDDA(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY PaxPie(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY PaxPolyBezier(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY PaxPolyBezierAt(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc722(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiEnableYInversion(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc724(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc725(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiQueryYInversion(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc727(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY GpiSaveMetaFile2(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY DEVPOSTESCAPE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
ULONG APIENTRY undoc730(void)
{
  return unimplemented(__FUNCTION__);
}
