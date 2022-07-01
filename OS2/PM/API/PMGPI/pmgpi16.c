#include <os2.h>
#include "unimpl.h"


// To configure Windows or OS/2 version define one of bellow defines
//#define PMOS2
//#define PMWIN

// To configure 32-bit version define bellow define
#define BIT32

#if !defined(PMWIN) && !defined(PMOS2)
#	define PMOS2
#endif

#if define(PMWIN) && defined(BIT32)
#	error 32-bit Windows API not supported (yet?)
#endif

/*

Presentation Manager GPI and Windows GDI comparation

						GPI													GDI
BOOL APIENTRY GpiErase(HPS);
HDC  APIENTRY GpiQueryDevice(HPS);
BOOL APIENTRY GpiRestorePS(HPS,LONG); 							BOOL FAR PASCAL RestoreDC(HDC, short);
LONG APIENTRY GpiSavePS(HPS);									SHORT FAR PASCAL SaveDC(HDC);
LONG  APIENTRY GpiErrorSegmentData(HPS,PLONG,PLONG);
LONG  APIENTRY GpiQueryDrawControl(HPS,LONG);
LONG  APIENTRY GpiQueryDrawingMode(HPS);
ULONG APIENTRY GpiQueryPS(HPS,PSIZEL);
LONG  APIENTRY GpiQueryStopDraw(HPS);
BOOL  APIENTRY GpiResetPS(HPS,ULONG);
BOOL  APIENTRY GpiSetDrawControl(HPS,LONG,LONG);
BOOL  APIENTRY GpiSetDrawingMode(HPS,LONG);
BOOL  APIENTRY GpiSetPS(HPS,PSIZEL,ULONG);
BOOL  APIENTRY GpiSetStopDraw(HPS,LONG);
LONG  APIENTRY GpiCorrelateChain(HPS,LONG,PPOINTL,LONG,LONG,PLONG);
LONG  APIENTRY GpiCorrelateFrom(HPS,LONG,LONG,LONG,PPOINTL,LONG,LONG,PLONG);
LONG  APIENTRY GpiCorrelateSegment(HPS,LONG,LONG,PPOINTL,LONG,LONG,PLONG);
BOOL  APIENTRY GpiQueryBoundaryData(HPS,PRECTL);
BOOL  APIENTRY GpiQueryPickAperturePosition(HPS,PPOINTL);
BOOL  APIENTRY GpiQueryPickApertureSize(HPS,PSIZEL);
BOOL  APIENTRY GpiQueryTag(HPS,PLONG);
BOOL  APIENTRY GpiResetBoundaryData(HPS);
BOOL  APIENTRY GpiSetPickAperturePosition(HPS,PPOINTL);
BOOL  APIENTRY GpiSetPickApertureSize(HPS,LONG,PSIZEL);
BOOL  APIENTRY GpiSetTag(HPS,LONG);
BOOL APIENTRY GpiCloseSegment(HPS);
BOOL APIENTRY GpiDeleteSegment(HPS,LONG);
BOOL APIENTRY GpiDeleteSegments(HPS,LONG,LONG);
BOOL APIENTRY GpiDrawChain(HPS);
BOOL APIENTRY GpiDrawDynamics(HPS);
BOOL APIENTRY GpiDrawFrom(HPS,LONG,LONG);
BOOL APIENTRY GpiDrawSegment(HPS,LONG);
LONG APIENTRY GpiGetData(HPS,LONG,PLONG,LONG,LONG,PBYTE);
BOOL APIENTRY GpiOpenSegment(HPS,LONG);
LONG APIENTRY GpiPutData(HPS,LONG,PLONG,PBYTE);
LONG APIENTRY GpiQueryInitialSegmentAttrs(HPS,LONG);
LONG APIENTRY GpiQuerySegmentAttrs(HPS,LONG,LONG);
LONG APIENTRY GpiQuerySegmentNames(HPS,LONG,LONG,LONG,PLONG);
LONG APIENTRY GpiQuerySegmentPriority(HPS,LONG,LONG);
BOOL APIENTRY GpiRemoveDynamics(HPS,LONG,LONG);
BOOL APIENTRY GpiSetInitialSegmentAttrs(HPS,LONG,LONG);
BOOL APIENTRY GpiSetSegmentAttrs(HPS,LONG,LONG,LONG);
BOOL APIENTRY GpiSetSegmentPriority(HPS,LONG,LONG,LONG);
BOOL  APIENTRY GpiBeginElement(HPS,LONG,PSZ);
BOOL  APIENTRY GpiDeleteElement(HPS);
BOOL  APIENTRY GpiDeleteElementRange(HPS,LONG,LONG);
BOOL  APIENTRY GpiDeleteElementsBetweenLabels(HPS,LONG,LONG);
BOOL  APIENTRY GpiEndElement(HPS);
LONG  APIENTRY GpiElement(HPS,LONG,PSZ,LONG,PBYTE);
BOOL  APIENTRY GpiLabel(HPS,LONG);
BOOL  APIENTRY GpiOffsetElementPointer(HPS,LONG);
LONG  APIENTRY GpiQueryEditMode(HPS);
LONG  APIENTRY GpiQueryElement(HPS,LONG,LONG,PBYTE);
LONG  APIENTRY GpiQueryElementPointer(HPS);
LONG  APIENTRY GpiQueryElementType(HPS,PLONG,LONG,PSZ);
BOOL  APIENTRY GpiSetEditMode(HPS,LONG);
BOOL  APIENTRY GpiSetElementPointer(HPS,LONG);
BOOL  APIENTRY GpiSetElementPointerAtLabel(HPS,LONG);
BOOL APIENTRY GpiConvert(HPS,LONG,LONG,LONG,PPOINTL);
LONG APIENTRY GpiCallSegmentMatrix(HPS,LONG,LONG,PMATRIXLF,LONG);
BOOL APIENTRY GpiQueryDefaultViewMatrix(HPS,LONG,PMATRIXLF);
BOOL APIENTRY GpiQueryGraphicsField(HPS,PRECTL);
BOOL APIENTRY GpiQueryModelTransformMatrix(HPS,LONG,PMATRIXLF);
BOOL APIENTRY GpiQueryPageViewport(HPS,PRECTL);
BOOL APIENTRY GpiQuerySegmentTransformMatrix(HPS,LONG,LONG,PMATRIXLF);
BOOL APIENTRY GpiQueryViewingTransformMatrix(HPS,LONG,PMATRIXLF);
BOOL APIENTRY GpiQueryViewingLimits(HPS,PRECTL);
BOOL APIENTRY GpiRotate(HPS,PMATRIXLF,LONG,FIXED,PPOINTL);
BOOL APIENTRY GpiScale(HPS,PMATRIXLF,LONG,PFIXED,PPOINTL);
BOOL APIENTRY GpiSetDefaultViewMatrix(HPS,LONG,PMATRIXLF,LONG);
BOOL APIENTRY GpiSetGraphicsField(HPS,PRECTL);
BOOL APIENTRY GpiSetModelTransformMatrix(HPS,LONG,PMATRIXLF,LONG);
BOOL APIENTRY GpiSetPageViewport(HPS,PRECTL);
BOOL APIENTRY GpiSetSegmentTransformMatrix(HPS,LONG,LONG,PMATRIXLF,LONG);
BOOL APIENTRY GpiSetViewingLimits(HPS,PRECTL);
BOOL APIENTRY GpiSetViewingTransformMatrix(HPS,LONG,PMATRIXLF,LONG);
BOOL APIENTRY GpiTranslate(HPS,PMATRIXLF,LONG,PPOINTL);
BOOL APIENTRY GpiBeginPath(HPS,LONG);
BOOL APIENTRY GpiCloseFigure(HPS);
BOOL APIENTRY GpiEndPath(HPS);
LONG APIENTRY GpiFillPath(HPS,LONG,LONG);
BOOL APIENTRY GpiModifyPath(HPS,LONG,LONG);
LONG APIENTRY GpiOutlinePath(HPS,LONG,LONG);
BOOL APIENTRY GpiSetClipPath(HPS,LONG,LONG);
LONG APIENTRY GpiStrokePath(HPS,LONG,ULONG);
BOOL  APIENTRY GpiCreateLogColorTable(HPS,ULONG,LONG,LONG,LONG,PLONG);
BOOL  APIENTRY GpiQueryColorData(HPS,LONG,PLONG);
LONG  APIENTRY GpiQueryColorIndex(HPS,ULONG,LONG);
LONG  APIENTRY GpiQueryLogColorTable(HPS,ULONG,LONG,LONG,PLONG);
LONG  APIENTRY GpiQueryNearestColor(HPS,ULONG,LONG);			DWORD FAR PASCAL GetNearestColor(HDC, DWORD);
LONG  APIENTRY GpiQueryRealColors(HPS,ULONG,LONG,LONG,PLONG);
LONG  APIENTRY GpiQueryRGBColor(HPS,ULONG,LONG);
BOOL  APIENTRY GpiRealizeColorTable(HPS);
BOOL  APIENTRY GpiUnrealizeColorTable(HPS);
BOOL  APIENTRY GpiBeginArea(HPS,ULONG);
LONG  APIENTRY GpiBox(HPS,LONG,PPOINTL,LONG,LONG);
LONG  APIENTRY GpiCharString(HPS,LONG,PCH);
LONG  APIENTRY GpiCharStringAt(HPS,PPOINTL,LONG,PCH);
LONG  APIENTRY GpiEndArea(HPS);
LONG  APIENTRY GpiLine(HPS,PPOINTL);
BOOL  APIENTRY GpiMove(HPS,PPOINTL);
LONG  APIENTRY GpiPolyLine(HPS,LONG,PPOINTL);
LONG  APIENTRY GpiQueryColor(HPS);
LONG  APIENTRY GpiQueryPattern(HPS);
BOOL  APIENTRY GpiSetColor(HPS,LONG);							DWORD FAR PASCAL SetTextColor(HDC, DWORD);
BOOL  APIENTRY GpiSetPattern(HPS,LONG);
LONG  APIENTRY GpiCharStringPos(HPS,PRECTL,ULONG,LONG,PCH,PLONG);
LONG  APIENTRY GpiCharStringPosAt(HPS,PPOINTL,PRECTL,ULONG,LONG,PCH,PLONG);
BOOL  APIENTRY GpiComment(HPS,LONG,PBYTE);
LONG  APIENTRY GpiFullArc(HPS,LONG,FIXED);
LONG  APIENTRY GpiImage(HPS,LONG,PSIZEL,LONG,PBYTE);
LONG  APIENTRY GpiMarker(HPS,PPOINTL);
LONG  APIENTRY GpiPartialArc(HPS,PPOINTL,FIXED,FIXED,FIXED);
LONG  APIENTRY GpiPointArc(HPS,PPOINTL);
LONG  APIENTRY GpiPolyFillet(HPS,LONG,PPOINTL);
LONG  APIENTRY GpiPolyFilletSharp(HPS,LONG,PPOINTL,PFIXED);
LONG  APIENTRY GpiPolyMarker(HPS,LONG,PPOINTL);
LONG  APIENTRY GpiPolySpline(HPS,LONG,PPOINTL);
BOOL  APIENTRY GpiPop(HPS,LONG);
LONG  APIENTRY GpiPtVisible(HPS,PPOINTL);
BOOL  APIENTRY GpiQueryArcParams(HPS,PARCPARAMS);
LONG  APIENTRY GpiQueryAttrMode(HPS);
LONG  APIENTRY GpiQueryAttrs(HPS,LONG,ULONG,PBUNDLE);
LONG  APIENTRY GpiQueryBackColor(HPS);							DWORD FAR PASCAL GetBkColor(HDC);
LONG  APIENTRY GpiQueryBackMix(HPS);
BOOL  APIENTRY GpiQueryCharAngle(HPS,PGRADIENTL);
BOOL  APIENTRY GpiQueryCharBox(HPS,PSIZEF);
LONG  APIENTRY GpiQueryCharDirection(HPS);
LONG  APIENTRY GpiQueryCharMode(HPS);
LONG  APIENTRY GpiQueryCharSet(HPS);
BOOL  APIENTRY GpiQueryCharShear(HPS,PPOINTL);
BOOL  APIENTRY GpiQueryCharStringPos(HPS,ULONG,LONG,PCH,PLONG,PPOINTL);
BOOL  APIENTRY GpiQueryCharStringPosAt(HPS,PPOINTL,ULONG,LONG,PCH,PLONG,PPOINTL);
BOOL  APIENTRY GpiQueryCurrentPosition(HPS,PPOINTL);
BOOL  APIENTRY GpiQueryDefCharBox(HPS,PSIZEL);
LONG  APIENTRY GpiQueryLineEnd(HPS);
LONG  APIENTRY GpiQueryLineJoin(HPS);
LONG  APIENTRY GpiQueryLineType(HPS);
FIXED APIENTRY GpiQueryLineWidth(HPS);
LONG  APIENTRY GpiQueryLineWidthGeom(HPS);
LONG  APIENTRY GpiQueryMarker(HPS);
BOOL  APIENTRY GpiQueryMarkerBox(HPS,PSIZEF);
LONG  APIENTRY GpiQueryMarkerSet(HPS);
LONG  APIENTRY GpiQueryMix(HPS);
BOOL  APIENTRY GpiQueryPatternRefPoint(HPS,PPOINTL);
LONG  APIENTRY GpiQueryPatternSet(HPS);
BOOL  APIENTRY GpiQueryTextBox(HPS,LONG,PCH,LONG,PPOINTL);
LONG  APIENTRY GpiRectVisible(HPS,PRECTL);
BOOL  APIENTRY GpiSetArcParams(HPS,PARCPARAMS);
BOOL  APIENTRY GpiSetAttrMode(HPS,LONG);
BOOL  APIENTRY GpiSetAttrs(HPS,LONG,ULONG,ULONG,PBUNDLE);
BOOL  APIENTRY GpiSetBackColor(HPS,LONG);						DWORD FAR PASCAL SetBkColor(HDC, DWORD);
BOOL  APIENTRY GpiSetBackMix(HPS,LONG);
BOOL  APIENTRY GpiSetCharAngle(HPS,PGRADIENTL);
BOOL  APIENTRY GpiSetCharBox(HPS,PSIZEF);
BOOL  APIENTRY GpiSetCharDirection(HPS,LONG);
BOOL  APIENTRY GpiSetCharMode(HPS,LONG);
BOOL  APIENTRY GpiSetCharSet(HPS,LONG);
BOOL  APIENTRY GpiSetCharShear(HPS,PPOINTL);
BOOL  APIENTRY GpiSetCurrentPosition(HPS,PPOINTL);
BOOL  APIENTRY GpiSetLineEnd(HPS,LONG);
BOOL  APIENTRY GpiSetLineJoin(HPS,LONG);
BOOL  APIENTRY GpiSetLineType(HPS,LONG);
BOOL  APIENTRY GpiSetLineWidth(HPS,FIXED);
BOOL  APIENTRY GpiSetLineWidthGeom(HPS,LONG);
BOOL  APIENTRY GpiSetMarker(HPS,LONG);
BOOL  APIENTRY GpiSetMarkerBox(HPS,PSIZEF);
BOOL  APIENTRY GpiSetMarkerSet(HPS,LONG);
BOOL  APIENTRY GpiSetMix(HPS,LONG);
BOOL  APIENTRY GpiSetPatternRefPoint(HPS,PPOINTL);
BOOL  APIENTRY GpiSetPatternSet(HPS,LONG);
LONG   APIENTRY GpiCreateLogFont(HPS,PSTR8,LONG,PFATTRS);
BOOL   APIENTRY GpiDeleteSetId(HPS,LONG);
BOOL   APIENTRY GpiLoadFonts(HAB,PSZ);
USHORT APIENTRY GpiQueryCp(HPS);
LONG   APIENTRY GpiQueryFontFileDescriptions(HAB,PSZ,PLONG,PFFDESCS);
LONG   APIENTRY GpiQueryFonts(HPS,ULONG,PSZ,PLONG,LONG,PFONTMETRICS);
BOOL   APIENTRY GpiQueryFontMetrics(HPS,LONG,PFONTMETRICS);
LONG   APIENTRY GpiQueryKerningPairs(HPS,LONG,PKERNINGPAIRS);
LONG   APIENTRY GpiQueryNumberSetIds(HPS);
BOOL   APIENTRY GpiQuerySetIds(HPS,LONG,PLONG,PSTR8,PLONG);
BOOL   APIENTRY GpiQueryWidthTable(HPS,LONG,LONG,PLONG);
BOOL   APIENTRY GpiSetCp(HPS,USHORT);
BOOL   APIENTRY GpiUnloadFonts(HAB,PSZ);
LONG    APIENTRY GpiBitBlt(HPS,HPS,LONG,PPOINTL,LONG,ULONG);
BOOL    APIENTRY GpiDeleteBitmap(HBITMAP);
HBITMAP APIENTRY GpiLoadBitmap(HPS,HMODULE,USHORT,LONG,LONG);
HBITMAP APIENTRY GpiSetBitmap(HPS,HBITMAP);
LONG    APIENTRY GpiWCBitBlt(HPS,HBITMAP,LONG,PPOINTL,LONG,ULONG);
HBITMAP APIENTRY GpiCreateBitmap(HPS,PBITMAPINFOHEADER,ULONG,PBYTE,PBITMAPINFO);
LONG    APIENTRY GpiQueryBitmapBits(HPS,LONG,LONG,PBYTE,PBITMAPINFO);
BOOL    APIENTRY GpiQueryBitmapDimension(HBITMAP,PSIZEL);
HBITMAP APIENTRY GpiQueryBitmapHandle(HPS,LONG);
BOOL    APIENTRY GpiQueryBitmapParameters(HBITMAP,PBITMAPINFOHEADER);
BOOL    APIENTRY GpiQueryDeviceBitmapFormats(HPS,LONG,PLONG);
LONG    APIENTRY GpiQueryPel(HPS,PPOINTL);
LONG    APIENTRY GpiSetBitmapBits(HPS,LONG,LONG,PBYTE,PBITMAPINFO);
BOOL    APIENTRY GpiSetBitmapDimension(HBITMAP,PSIZEL);
BOOL    APIENTRY GpiSetBitmapId(HPS,HBITMAP,LONG);
LONG    APIENTRY GpiSetPel(HPS,PPOINTL);
LONG  APIENTRY GpiCombineRegion(HPS,HRGN,HRGN,HRGN,LONG);	int     WINAPI CombineRgn(HRGN, HRGN, HRGN, int);
HRGN  APIENTRY GpiCreateRegion(HPS,LONG,PRECTL);			HRGN    WINAPI CreateRectRgn(int, int, int, int);
BOOL  APIENTRY GpiDestroyRegion(HPS,HRGN);
LONG  APIENTRY GpiEqualRegion(HPS,HRGN,HRGN);				BOOL    WINAPI EqualRgn(HRGN, HRGN);
LONG  APIENTRY GpiExcludeClipRectangle(HPS,PRECTL);
LONG  APIENTRY GpiIntersectClipRectangle(HPS,PRECTL);
LONG  APIENTRY GpiOffsetClipRegion(HPS,PPOINTL);
BOOL  APIENTRY GpiOffsetRegion(HPS,HRGN,PPOINTL);			int     WINAPI OffsetRgn(HRGN, int, int);
LONG  APIENTRY GpiPaintRegion(HPS,HRGN);
LONG  APIENTRY GpiPtInRegion(HPS,HRGN,PPOINTL);				BOOL    WINAPI PtInRegion(HRGN, int, int);
LONG  APIENTRY GpiQueryRegionBox(HPS,HRGN,PRECTL);			int     WINAPI GetRgnBox(HRGN, RECT FAR*);
BOOL  APIENTRY GpiQueryRegionRects(HPS,HRGN,PRECTL,PRGNRECT,PRECTL);
LONG  APIENTRY GpiRectInRegion(HPS,HRGN,PRECTL);			BOOL    WINAPI RectInRegion(HRGN, const RECT FAR*);
BOOL  APIENTRY GpiSetRegion(HPS,HRGN,LONG,PRECTL);
LONG  APIENTRY GpiSetClipRegion(HPS,HRGN,PHRGN);
HRGN  APIENTRY GpiQueryClipRegion(HPS);
LONG  APIENTRY GpiQueryClipBox(HPS,PRECTL);
HMF   APIENTRY GpiCopyMetaFile(HMF);						HANDLE FAR PASCAL  CopyMetaFile(HANDLE, LPSTR);
BOOL  APIENTRY GpiDeleteMetaFile(HMF);						BOOL FAR PASCAL  DeleteMetaFile(HANDLE);
HMF   APIENTRY GpiLoadMetaFile(HAB,PSZ);					HANDLE FAR PASCAL  GetMetaFile(LPSTR);
LONG  APIENTRY GpiPlayMetaFile(HPS,HMF,LONG,PLONG,PLONG,LONG,PSZ);	BOOL FAR PASCAL  PlayMetaFile(HDC, HANDLE);
BOOL  APIENTRY GpiQueryMetaFileBits(HMF,LONG,LONG,PBYTE);
LONG  APIENTRY GpiQueryMetaFileLength(HMF);
BOOL  APIENTRY GpiSaveMetaFile(HMF,PSZ);
BOOL  APIENTRY GpiSetMetaFileBits(HMF,LONG,LONG,PBYTE);
BOOL  APIENTRY GpiQueryDefArcParams(HPS,PARCPARAMS);
BOOL  APIENTRY GpiQueryDefAttrs(HPS,LONG,ULONG,PBUNDLE);
BOOL  APIENTRY GpiQueryDefTag(HPS,PLONG);
BOOL  APIENTRY GpiQueryDefViewingLimits(HPS,PRECTL);
BOOL  APIENTRY GpiSetDefArcParams(HPS,PARCPARAMS);
BOOL  APIENTRY GpiSetDefAttrs(HPS,LONG,ULONG,PBUNDLE);
BOOL  APIENTRY GpiSetDefTag(HPS,LONG);
BOOL  APIENTRY GpiSetDefViewingLimits(HPS,PRECTL);
*/

#include "gpi16dev.c"
#include "gpi16control.c"
#include "gpi16correlation.c"
#include "gpi16segments.c"
#include "gpi16segediting.c"
#include "gpi16transforms.c"
#include "gpi16paths.c"
#include "gpi16logcolortable.c"
#include "gpi16primitives.c"




// fix prototype !!!
USHORT APIENTRY16 GPISETBITMAPID(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYBITMAPHANDLE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPILOADFONTS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIUNLOADFONTS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPICREATELOGFONT(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIDELETESETID(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYNUMBERSETIDS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYSETIDS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYFONTS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYFONTMETRICS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYKERNINGPAIRS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYWIDTHTABLE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETCP(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYCP(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 DEVQUERYDEVICENAMES(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYFONTFILEDESCRIPTIONS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPICREATEBITMAP(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIDELETEBITMAP(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETBITMAP(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETBITMAPDIMENSION(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYBITMAPDIMENSION(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYDEVICEBITMAPFORMATS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYBITMAPPARAMETERS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETBITMAPBITS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYBITMAPBITS(void)
{
  return unimplemented(__FUNCTION__);
}

//BOOL    WINAPI BitBlt(HDC, int, int, int, int, HDC, int, int, DWORD);
// fix prototype !!!
USHORT APIENTRY16 GPIBITBLT(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETPEL(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYPEL(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPICREATEREGION(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETREGION(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIDESTROYREGION(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPICOMBINEREGION(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIEQUALREGION(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIOFFSETREGION(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIPTINREGION(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIRECTINREGION(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYREGIONBOX(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYREGIONRECTS(void)
{
  return unimplemented(__FUNCTION__);
}

//int     WINAPI SelectClipRgn(HDC, HRGN);
// fix prototype !!!
USHORT APIENTRY16 GPISETCLIPREGION(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYCLIPREGION(void)
{
  return unimplemented(__FUNCTION__);
}

//int     WINAPI GetClipBox(HDC, RECT FAR*);
// fix prototype !!!
USHORT APIENTRY16 GPIQUERYCLIPBOX(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIINTERSECTCLIPRECTANGLE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIEXCLUDECLIPRECTANGLE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIOFFSETCLIPREGION(void)
{
  return unimplemented(__FUNCTION__);
}

//BOOL    WINAPI Polygon(HDC, const POINT FAR*, int);
// fix prototype !!!
USHORT APIENTRY16 GPIPAINTREGION(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPILOADMETAFILE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPICOPYMETAFILE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIPLAYMETAFILE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISAVEMETAFILE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIDELETEMETAFILE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYMETAFILEBITS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETMETAFILEBITS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYMETAFILELENGTH(void)
{
  return unimplemented(__FUNCTION__);
}








// fix prototype !!!
USHORT APIENTRY16 GPIVECTORSYMBOL(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIOPENMETAFILE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPICLOSEMETAFILE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 DEVSTDOPEN(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPICONVPSH(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!!
USHORT APIENTRY16 SEGSGWOPENSEGMENTWINDOW(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTENDREADREQUEST(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTGETDESCRIPTION(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTGETCODEPAGE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTGETLCT(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTGETGDDINFO(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 FMTCONVERTGOCAPOLY(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTGETFIRSTFONT(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 SEGSGWNEWPARTDATA(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTSTARTREADREQUEST(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTGETFIRSTGRAPHICSDATA(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTGETNEXTFONT(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTGETNEXTGRAPHICSDATA(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPILOADPUBLICFONTS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIUNLOADPUBLICFONTS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIACCESSMETAFILE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIMTASSOCIATE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIMTDISASSOCIATE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIWCBITBLT(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 SEGSGWNEXTORDERF(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISUSPENDPLAY(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIRESUMEPLAY(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!!
USHORT APIENTRY16 GPISETDEFTAG(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYDEFTAG(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETDEFATTRS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYDEFATTRS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETDEFVIEWINGLIMITS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYDEFVIEWINGLIMITS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETDEFARCPARAMS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYDEFARCPARAMS(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!!
USHORT APIENTRY16 GPIPOLYLINEDISJOINT(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIPATHTOREGION(void)
{
  return unimplemented(__FUNCTION__);
}

//BOOL    WINAPI FloodFill(HDC, int, int, COLORREF);
// fix prototype !!!
USHORT APIENTRY16 GPIFLOODFILL(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIDRAWBITS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYBITMAPINFOHEADER(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYLOGICALFONT(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYFACESTRING(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYFONTACTION(void)
{
  return unimplemented(__FUNCTION__);
}

//HPALETTE WINAPI CreatePalette(const LOGPALETTE FAR*);
// fix prototype !!!
USHORT APIENTRY16 GPICREATEPALETTE(void)
{
  return unimplemented(__FUNCTION__);
}

// UINT    WINAPI RealizePalette(HDC);
// fix prototype !!!
USHORT APIENTRY16 GPIDELETEPALETTE(void)
{
  return unimplemented(__FUNCTION__);
}

//HPALETTE WINAPI SelectPalette(HDC, HPALETTE, BOOL);
// fix prototype !!!
USHORT APIENTRY16 GPISELECTPALETTE(void)
{
  return unimplemented(__FUNCTION__);
}

//void    WINAPI AnimatePalette(HPALETTE, UINT, UINT, const PALETTEENTRY FAR*);
// fix prototype !!!
USHORT APIENTRY16 GPIANIMATEPALETTE(void)
{
  return unimplemented(__FUNCTION__);
}

// UINT    WINAPI SetPaletteEntries(HPALETTE, UINT, UINT, const PALETTEENTRY FAR*);
// fix prototype !!!
USHORT APIENTRY16 GPISETPALETTEENTRIES(void)
{
  return unimplemented(__FUNCTION__);
}

// UINT    WINAPI GetPaletteEntries(HPALETTE, UINT, UINT, PALETTEENTRY FAR*);
// fix prototype !!!
USHORT APIENTRY16 GPIQUERYPALETTE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYPALETTEINFO(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYCHAREXTRA(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETCHAREXTRA(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYCHARBREAKEXTRA(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETCHARBREAKEXTRA(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIFRAMEREGION(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPICONVERTWITHMATRIX(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETDCOWNER(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETBITMAPOWNER(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETREGIONOWNER(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIINSTALLIFIFONT(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 DSPINITSYSTEMDRIVERNAME(void)
{
  return unimplemented(__FUNCTION__);
}

