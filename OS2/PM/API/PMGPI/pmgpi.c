/*!
   @file

   @ingroup pm

   @brief Presentation Manager 32-bit Graphics Programming Interface

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

*/

#define INCL_DEV
#define INCL_GREALL
#include <os2.h>
#include "pmddi.h"
#include "unimpl.h"

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

#include "gpi32dev.c"
#include "gpi32lines.c"

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

BOOL   APIENTRY GpiBeginPath(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

LONG    APIENTRY GpiBitBlt(HPS hps,HPS hps2,LONG a,PPOINTL b,LONG c,ULONG d)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiBox(HPS hps, LONG lControl, PPOINTL pptlPoint, LONG lHRound, LONG lVRound)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiCallSegmentMatrix(HPS hps,LONG a,LONG b,PMATRIXLF c,LONG d)
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

BOOL   APIENTRY GpiCloseFigure(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiCloseSegment(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiCombineRegion(HPS hps, HRGN hrgnDest, HRGN hrgnSr1, HRGN hrgnSr2, LONG lMode)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiComment(HPS hps,LONG a,PBYTE b)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiConvert(HPS hps,LONG a,LONG b,LONG c,PPOINTL d)
{
  return unimplemented(__FUNCTION__);
}

HMF    APIENTRY GpiCopyMetaFile(HMF hmf)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiCorrelateChain(HPS hps,LONG a,PPOINTL b, LONG c,LONG d,PLONG e)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiCorrelateFrom(HPS hps,LONG a,LONG b ,LONG c,PPOINTL d,LONG e ,LONG f,PLONG g)
{
  return unimplemented(__FUNCTION__);
}

LONG  APIENTRY GpiCreateLogFont(HPS hps,PSTR8 a,LONG b,PFATTRS c)
{
  return unimplemented(__FUNCTION__);
}

HPS    APIENTRY GpiCreatePS(HAB hab,HDC hdc,PSIZEL a,ULONG b)
{
  return unimplemented(__FUNCTION__);
}

HRGN   APIENTRY GpiCreateRegion(HPS hps, LONG lCount, PRECTL arclRectangles)
{
  return unimplemented(__FUNCTION__);
}

BOOL    APIENTRY GpiDeleteBitmap(HBITMAP hbm)
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

BOOL   APIENTRY GpiDeleteMetaFile(HMF hmf)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDeleteSegment(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDeleteSegments(HPS hps,LONG a,LONG b)
{
  return unimplemented(__FUNCTION__);
}

BOOL  APIENTRY GpiDeleteSetId(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDestroyPS(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDrawChain(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDrawDynamics(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDrawFrom(HPS hps,LONG a,LONG b)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDrawSegment(HPS hps,LONG a)
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

BOOL   APIENTRY GpiEndPath(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiEqualRegion(HPS hps, HRGN hrgnSrc1, HRGN hrgnSrc2)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiErase(HPS hps)
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

LONG   APIENTRY GpiFillPath(HPS hps,LONG a,LONG b)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiFullArc(HPS hps,LONG a,FIXED b)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiGetData(HPS hps,LONG a,PLONG b,LONG c,LONG d,PBYTE e)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiImage(HPS hps,LONG a,PSIZEL b,LONG c,PBYTE d)
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

BOOL  APIENTRY GpiLoadFonts(HAB hab,PCSZ pcsz)
{
  return unimplemented(__FUNCTION__);
}

HMF    APIENTRY GpiLoadMetaFile(HAB hab,PCSZ pcsz)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiMarker(HPS hps,PPOINTL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiModifyPath(HPS hps,LONG a,LONG b)
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

BOOL   APIENTRY GpiOpenSegment(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPaintRegion(HPS hps, HRGN hrgn)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPlayMetaFile(HPS hps,HMF hmf,LONG a,PLONG b,PLONG c,LONG d,PCSZ e)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPointArc(HPS hps,PPOINTL a)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPolyFillet(HPS hps,LONG a,PPOINTL b)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPolyFilletSharp(HPS hps,LONG a,PPOINTL b,PFIXED c)
{
  return unimplemented(__FUNCTION__);
}


LONG   APIENTRY GpiPolyMarker(HPS hps,LONG a,PPOINTL b)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPolySpline(HPS hps,LONG a,PPOINTL b)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiPop(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPtInRegion(HPS hps, HRGN hrgn, PPOINTL pptlPoint)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPtVisible(HPS hps,PPOINTL a)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPutData(HPS hps,LONG a,PLONG b,PBYTE c)
{
  return unimplemented(__FUNCTION__);
}

BOOL APIENTRY GpiQueryArcParams(HPS hps, PARCPARAMS parcpArcParams)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryAttrMode(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryBackColor(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryBackMix(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

BOOL    APIENTRY GpiQueryBitmapDimension(HBITMAP hbm,PSIZEL a)
{
  return unimplemented(__FUNCTION__);
}

HBITMAP APIENTRY GpiQueryBitmapHandle(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryBoundaryData(HPS hps,PRECTL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryCharAngle(HPS hps,PGRADIENTL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryCharBox(HPS hps,PSIZEF a)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryCharDirection(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryCharMode(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryCharSet(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryCharShear(HPS hps,PPOINTL a)
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

BOOL   APIENTRY GpiQueryColorData(HPS hps,LONG a,PLONG b)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryColorIndex(HPS hps,ULONG a,LONG b)
{
  return unimplemented(__FUNCTION__);
}

ULONG APIENTRY GpiQueryCp(HPS hps)
{
  return unimplemented(__FUNCTION__);
}


BOOL   APIENTRY GpiQueryDefCharBox(HPS hps,PSIZEL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryDefaultViewMatrix(HPS hps,LONG a,PMATRIXLF b)
{
  return unimplemented(__FUNCTION__);
}

HDC    APIENTRY GpiQueryDevice(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

BOOL    APIENTRY GpiQueryDeviceBitmapFormats(HPS hps,LONG a,PLONG b)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryDrawControl(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryDrawingMode(HPS hps)
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

LONG  APIENTRY GpiQueryFontFileDescriptions(HAB hab,PCSZ a,PLONG b,PFFDESCS c)
{
  return unimplemented(__FUNCTION__);
}

BOOL  APIENTRY GpiQueryFontMetrics(HPS hps,LONG a,PFONTMETRICS b)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryGraphicsField(HPS hps,PRECTL a)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryInitialSegmentAttrs(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

LONG  APIENTRY GpiQueryKerningPairs(HPS hps,LONG a,PKERNINGPAIRS b)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryLineEnd(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryLineJoin(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryLineType(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

FIXED  APIENTRY GpiQueryLineWidth(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryLineWidthGeom(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryMarker(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryMarkerBox(HPS hps,PSIZEF a)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryMarkerSet(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryMetaFileBits(HMF hmf,LONG a,LONG b,PBYTE c)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryMetaFileLength(HMF hmf)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryMix(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryModelTransformMatrix(HPS hps,LONG a,PMATRIXLF b)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryNearestColor(HPS hps,ULONG a,LONG b)
{
  return unimplemented(__FUNCTION__);
}

LONG  APIENTRY GpiQueryNumberSetIds(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

ULONG  APIENTRY GpiQueryPS(HPS hps,PSIZEL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryPageViewport(HPS hps,PRECTL a)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryPattern(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryPatternRefPoint(HPS hps,PPOINTL a)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryPatternSet(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

LONG    APIENTRY GpiQueryPel(HPS hps,PPOINTL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryPickAperturePosition(HPS hps,PPOINTL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryPickApertureSize(HPS hps,PSIZEL a)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryRGBColor(HPS hps,ULONG a,LONG b)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryRealColors(HPS hps,ULONG a,LONG b,LONG c,PLONG d)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryRegionBox(HPS hps, HRGN hrgn, PRECTL prclBound)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQuerySegmentAttrs(HPS hps,LONG a,LONG b)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQuerySegmentNames(HPS hps,LONG a,LONG b,LONG c,PLONG d)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQuerySegmentPriority(HPS hps,LONG a,LONG b)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQuerySegmentTransformMatrix(HPS hps,LONG a,LONG b,PMATRIXLF c)
{
  return unimplemented(__FUNCTION__);
}

BOOL  APIENTRY GpiQuerySetIds(HPS hps,LONG a,PLONG b,PSTR8 c,PLONG d)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryStopDraw(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryTag(HPS hps,PLONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryTextBox(HPS hps,LONG a,PCH b,LONG c,PPOINTL d)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryViewingLimits(HPS hps,PRECTL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryViewingTransformMatrix(HPS hps,LONG a,PMATRIXLF b)
{
  return unimplemented(__FUNCTION__);
}

BOOL  APIENTRY GpiQueryWidthTable(HPS hps,LONG a,LONG b,PLONG c)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiRectInRegion(HPS hps, HRGN hrgn, PRECTL prclRect)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiRectVisible(HPS hps,PRECTL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiRemoveDynamics(HPS hps,LONG a,LONG b)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiResetBoundaryData(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiResetPS(HPS hps,ULONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiRestorePS(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSaveMetaFile(HMF hmf,PCSZ pcsz)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiSavePS(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetArcParams(HPS hps,PARCPARAMS a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetAttrMode(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetBackColor(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetBackMix(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

HBITMAP APIENTRY GpiSetBitmap(HPS hps,HBITMAP a)
{
  return unimplemented(__FUNCTION__);
}

BOOL    APIENTRY GpiSetBitmapDimension(HBITMAP hbm,PSIZEL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL    APIENTRY GpiSetBitmapId(HPS hps,HBITMAP a,LONG b)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetCharAngle(HPS hps,PGRADIENTL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetCharBox(HPS hps,PSIZEF a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetCharDirection(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetCharMode(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetCharSet(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetCharShear(HPS hps,PPOINTL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetClipPath(HPS hps,LONG a,LONG b)
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

BOOL  APIENTRY GpiSetCp(HPS hps,ULONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetDefaultViewMatrix(HPS hps,LONG a,PMATRIXLF b,LONG c)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetDrawControl(HPS hps,LONG a,LONG b)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetDrawingMode(HPS hps,LONG a)
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

BOOL   APIENTRY GpiSetGraphicsField(HPS hps,PRECTL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetInitialSegmentAttrs(HPS hps,LONG a,LONG b)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetLineEnd(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetLineJoin(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetLineType(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetLineWidth(HPS hps,FIXED a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetLineWidthGeom(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetMarker(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetMarkerBox(HPS hps,PSIZEF a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetMarkerSet(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetMetaFileBits(HMF hmf,LONG a,LONG b,PBYTE c)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetMix(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetModelTransformMatrix(HPS hps,LONG a,PMATRIXLF b,LONG c)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetPS(HPS hps,PSIZEL a,ULONG b)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetPageViewport(HPS hps,PRECTL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetPattern(HPS hps, LONG lPatternSymbol)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetPatternRefPoint(HPS hps,PPOINTL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetPatternSet(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

LONG    APIENTRY GpiSetPel(HPS hps,PPOINTL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetPickAperturePosition(HPS hps,PPOINTL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetRegion(HPS hps, HRGN hrgn, LONG lcount, PRECTL arclRectangles)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetSegmentAttrs(HPS hps,LONG a,LONG b,LONG c)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetSegmentPriority(HPS hps,LONG a,LONG b,LONG c)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetStopDraw(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetTag(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetViewingLimits(HPS hps,PRECTL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetViewingTransformMatrix(HPS hps,LONG a,PMATRIXLF b,LONG c)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiStrokePath(HPS hps,LONG a,ULONG b)
{
  return unimplemented(__FUNCTION__);
}

BOOL  APIENTRY GpiUnloadFonts(HAB hab,PCSZ a)
{
  return unimplemented(__FUNCTION__);
}

LONG    APIENTRY GpiWCBitBlt(HPS hps,HBITMAP a,LONG f,PPOINTL b,LONG c,ULONG d)
{
  return unimplemented(__FUNCTION__);
}


HRGN   APIENTRY GpiPathToRegion(HPS hps,LONG a,LONG b)
{
  return unimplemented(__FUNCTION__);
}

LONG    APIENTRY GpiFloodFill(HPS hps, LONG a,LONG b)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSuspendPlay(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiResumePlay(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiOutlinePath(HPS hps,LONG a,LONG b)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiTranslate(HPS hps,PMATRIXLF a,LONG b,PPOINTL c)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiScale(HPS hps,PMATRIXLF a,LONG b,PFIXED c,PPOINTL d)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiRotate(HPS hps,PMATRIXLF a,LONG b,FIXED c,PPOINTL d)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryDefArcParams(HPS hps,PARCPARAMS a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryDefTag(HPS hps,PLONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryDefViewingLimits(HPS hps,PRECTL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetDefArcParams(HPS hps,PARCPARAMS a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetDefTag(HPS hps,LONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetDefViewingLimits(HPS hps,PRECTL a)
{
  return unimplemented(__FUNCTION__);
}

BOOL    APIENTRY GpiQueryBitmapParameters(HBITMAP hbm,PBITMAPINFOHEADER a)
{
  return unimplemented(__FUNCTION__);
}

BOOL  APIENTRY GpiQueryLogicalFont(HPS hps,LONG a,PSTR8 b,PFATTRS c,LONG d)
{
  return unimplemented(__FUNCTION__);
}

ULONG APIENTRY GpiQueryFaceString(HPS hps,PCSZ a,PFACENAMEDESC b,LONG c,PCSZ d)
{
  return unimplemented(__FUNCTION__);
}

ULONG APIENTRY GpiQueryFontAction(HAB hab,ULONG a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDeletePalette(HPAL hpal)
{
  return unimplemented(__FUNCTION__);
}

HPAL   APIENTRY GpiSelectPalette(HPS hps,HPAL hpal)
{
  return unimplemented(__FUNCTION__);
}

HPAL   APIENTRY GpiQueryPalette(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiCharStringPos(HPS hps,PRECTL a,ULONG b,LONG c,PCH d,PLONG e)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiCharStringPosAt(HPS hps,PPOINTL a,PRECTL b,ULONG c,LONG d,PCH e,PLONG f)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiCorrelateSegment(HPS hps,LONG a,LONG b,PPOINTL c,LONG d,LONG e,PLONG f)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryAttrs(HPS hps,LONG a,ULONG b,PBUNDLE c)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryCharStringPos(HPS hps,ULONG a,LONG b,PCH c,PLONG d,PPOINTL e)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryCharStringPosAt(HPS hps,PPOINTL a,ULONG b,LONG c,PCH d,PLONG e,PPOINTL f)
{
  return unimplemented(__FUNCTION__);
}

LONG  APIENTRY GpiQueryFonts(HPS hps,ULONG a,PCSZ b,PLONG c,LONG d,PFONTMETRICS e)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryRegionRects(HPS hps, HRGN hrgn, PRECTL prclBound,
                   PRGNRECT prgnrcControl, PRECTL prclRect)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetAttrs(HPS hps,LONG a,ULONG b,ULONG c,PBUNDLE d)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetPickApertureSize(HPS hps,LONG a,PSIZEL b)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryDefAttrs(HPS hps,LONG a,ULONG b ,PBUNDLE c)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetDefAttrs(HPS hps,LONG a,ULONG b ,PBUNDLE c)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiCreateLogColorTable(HPS hps,ULONG a,LONG b,LONG c,LONG d,PLONG e)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryLogColorTable(HPS hps,ULONG a,LONG b,LONG c,PLONG d)
{
  return unimplemented(__FUNCTION__);
}

HPAL   APIENTRY GpiCreatePalette(HAB hab,ULONG a,ULONG b,ULONG c,PULONG d)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiAnimatePalette(HPAL hpal,ULONG a,ULONG b,ULONG c,PULONG d)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetPaletteEntries(HPAL hpal,ULONG a,ULONG b,ULONG c,PULONG d)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiQueryPaletteInfo(HPAL hpal,HPS hps,ULONG a,ULONG b,ULONG c,PULONG d)
{
  return unimplemented(__FUNCTION__);
}

HBITMAP APIENTRY GpiCreateBitmap(HPS hps,PBITMAPINFOHEADER2 a,ULONG b,PBYTE c,PBITMAPINFO2 d)
{
  return unimplemented(__FUNCTION__);
}

LONG    APIENTRY GpiQueryBitmapBits(HPS hps,LONG a,LONG b,PBYTE c,PBITMAPINFO2 d)
{
  return unimplemented(__FUNCTION__);
}

BOOL    APIENTRY GpiQueryBitmapInfoHeader(HBITMAP hbm,PBITMAPINFOHEADER2 a)
{
  return unimplemented(__FUNCTION__);
}

LONG    APIENTRY GpiSetBitmapBits(HPS hps,LONG a,LONG b,PBYTE c,PBITMAPINFO2 d)
{
  return unimplemented(__FUNCTION__);
}

LONG    APIENTRY GpiDrawBits(HPS hps,PVOID a,PBITMAPINFO2 b,LONG c,PPOINTL d,LONG e,ULONG f)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiDestroyRegion(HPS hps, HRGN hrgn)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiPartialArc(HPS hps,PPOINTL a,FIXED b,FIXED c,FIXED d)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryCharExtra(HPS hps,PFIXED a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetCharExtra(HPS hps,FIXED a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiQueryCharBreakExtra(HPS hps,PFIXED a)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetCharBreakExtra(HPS hps,FIXED a)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY GpiFrameRegion(HPS hps, HRGN hrgn, PSIZEL thickness)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiConvertWithMatrix(HPS hps,LONG a,PPOINTL b,LONG c,PMATRIXLF d)
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

BOOL  APIENTRY GpiLoadPublicFonts(HAB hab ,PCSZ a)
{
  return unimplemented(__FUNCTION__);
}

BOOL  APIENTRY GpiUnloadPublicFonts(HAB hab,PCSZ a)
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

BOOL   APIENTRY GpiQueryTextAlignment(HPS hps,PLONG a, PLONG b)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY GpiSetTextAlignment(HPS hps,LONG a,LONG b)
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

LONG  APIENTRY GpiQueryFullFontFileDescs(HAB hab,PCSZ a,PLONG b,PVOID c,PLONG d)
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
ULONG APIENTRY undoc730(void)
{
  return unimplemented(__FUNCTION__);
}
