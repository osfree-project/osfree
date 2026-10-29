/*!
 * @file gpi_manual.c
 * @brief Hand-written client-side PMGPI proxies (stubs).
 */
#define INCL_GPI
#include "gpi_platform.h"
#include "gpi_entry.h"


/*!
 * @brief GpiCreatePS (stub).
 * @param [in] hab HAB.
 * @param [in] hdc HDC.
 * @param [in] psizlSize PSIZEL.
 * @param [in] flOptions ULONG.
 * @return Handle, or GPI_ERROR.
 */
HPS APIENTRY GpiCreatePS(HAB hab, HDC hdc, PSIZEL psizlSize, ULONG flOptions)
{
    return (HPS)GPI_ERROR;
}

/*!
 * @brief GpiQueryPS (stub).
 * @param [in] hps HPS.
 * @param [out] psizlSize PSIZEL.
 * @return Implementation-defined value.
 */
ULONG APIENTRY GpiQueryPS(HPS hps, PSIZEL psizlSize)
{
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief GpiErrorSegmentData (stub).
 * @param [in] hps HPS.
 * @param [in] plSegment PLONG.
 * @param [in] plContext PLONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiErrorSegmentData(HPS hps, PLONG plSegment, PLONG plContext)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiCorrelateChain (stub).
 * @param [in] hps HPS.
 * @param [in] lType LONG.
 * @param [in] pptlPick PPOINTL.
 * @param [in] lMaxHits LONG.
 * @param [in] lMaxDepth LONG.
 * @param [in] pl2 PLONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiCorrelateChain(HPS hps, LONG lType, PPOINTL pptlPick, LONG lMaxHits, LONG lMaxDepth, PLONG pl2)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryTag (stub).
 * @param [in] hps HPS.
 * @param [out] plTag PLONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryTag(HPS hps, PLONG plTag)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryPickApertureSize (stub).
 * @param [in] hps HPS.
 * @param [out] psizlSize PSIZEL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryPickApertureSize(HPS hps, PSIZEL psizlSize)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryPickAperturePosition (stub).
 * @param [in] hps HPS.
 * @param [out] pptlPoint PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryPickAperturePosition(HPS hps, PPOINTL pptlPoint)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryBoundaryData (stub).
 * @param [in] hps HPS.
 * @param [out] prclBoundary PRECTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryBoundaryData(HPS hps, PRECTL prclBoundary)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiCorrelateFrom (stub).
 * @param [in] hps HPS.
 * @param [in] lFirstSegment LONG.
 * @param [in] lLastSegment LONG.
 * @param [in] lType LONG.
 * @param [in] pptlPick PPOINTL.
 * @param [in] lMaxHits LONG.
 * @param [in] lMaxDepth LONG.
 * @param [in] plSegTag PLONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiCorrelateFrom(HPS hps, LONG lFirstSegment, LONG lLastSegment, LONG lType, PPOINTL pptlPick, LONG lMaxHits, LONG lMaxDepth, PLONG plSegTag)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiCorrelateSegment (stub).
 * @param [in] hps HPS.
 * @param [in] lSegment LONG.
 * @param [in] lType LONG.
 * @param [in] pptlPick PPOINTL.
 * @param [in] lMaxHits LONG.
 * @param [in] lMaxDepth LONG.
 * @param [in] alSegTag PLONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiCorrelateSegment(HPS hps, LONG lSegment, LONG lType, PPOINTL pptlPick, LONG lMaxHits, LONG lMaxDepth, PLONG alSegTag)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQuerySegmentNames (stub).
 * @param [in] hps HPS.
 * @param [in] lFirstSegid LONG.
 * @param [in] lLastSegid LONG.
 * @param [in] lMax LONG.
 * @param [out] alSegids PLONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQuerySegmentNames(HPS hps, LONG lFirstSegid, LONG lLastSegid, LONG lMax, PLONG alSegids)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiGetData (stub).
 * @param [in] hps HPS.
 * @param [in] lSegid LONG.
 * @param [out] plOffset PLONG.
 * @param [in] lFormat LONG.
 * @param [in] lLength LONG.
 * @param [out] pbData PBYTE.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiGetData(HPS hps, LONG lSegid, PLONG plOffset, LONG lFormat, LONG lLength, PBYTE pbData)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiPutData (stub).
 * @param [in] hps HPS.
 * @param [in] lFormat LONG.
 * @param [in] plCount PLONG.
 * @param [in] pbData PBYTE.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiPutData(HPS hps, LONG lFormat, PLONG plCount, PBYTE pbData)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiBeginElement (stub).
 * @param [in] hps HPS.
 * @param [in] lType LONG.
 * @param [in] pszDesc PCSZ.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiBeginElement(HPS hps, LONG lType, PCSZ pszDesc)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiElement (stub).
 * @param [in] hps HPS.
 * @param [in] lType LONG.
 * @param [in] pszDesc PCSZ.
 * @param [in] lLength LONG.
 * @param [in] pbData PBYTE.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiElement(HPS hps, LONG lType, PCSZ pszDesc, LONG lLength, PBYTE pbData)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryElement (stub).
 * @param [in] hps HPS.
 * @param [in] lOff LONG.
 * @param [in] lMaxLength LONG.
 * @param [out] pbData PBYTE.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryElement(HPS hps, LONG lOff, LONG lMaxLength, PBYTE pbData)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryElementType (stub).
 * @param [in] hps HPS.
 * @param [out] plType PLONG.
 * @param [in] lLength LONG.
 * @param [in] pszData PCSZ.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryElementType(HPS hps, PLONG plType, LONG lLength, PCSZ pszData)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQuerySegmentTransformMatrix (stub).
 * @param [in] hps HPS.
 * @param [in] lSegid LONG.
 * @param [in] lCount LONG.
 * @param [out] pmatlfArray PMATRIXLF.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQuerySegmentTransformMatrix(HPS hps, LONG lSegid, LONG lCount, PMATRIXLF pmatlfArray)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiSetSegmentTransformMatrix (stub).
 * @param [in] hps HPS.
 * @param [in] lSegid LONG.
 * @param [in] lCount LONG.
 * @param [in] pmatlfarray PMATRIXLF.
 * @param [in] lOptions LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetSegmentTransformMatrix(HPS hps, LONG lSegid, LONG lCount, PMATRIXLF pmatlfarray, LONG lOptions)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiConvert (stub).
 * @param [in] hps HPS.
 * @param [in] lSrc LONG.
 * @param [in] lTarg LONG.
 * @param [in] lCount LONG.
 * @param [in] aptlPoints PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiConvert(HPS hps, LONG lSrc, LONG lTarg, LONG lCount, PPOINTL aptlPoints)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiConvertWithMatrix (stub).
 * @param [in] hps HPS.
 * @param [in] lCountp LONG.
 * @param [in] aptlPoints PPOINTL.
 * @param [in] lCount LONG.
 * @param [in] pmatlfArray PMATRIXLF.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiConvertWithMatrix(HPS hps, LONG lCountp, PPOINTL aptlPoints, LONG lCount, PMATRIXLF pmatlfArray)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryModelTransformMatrix (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [out] pmatlfArray PMATRIXLF.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryModelTransformMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiSetModelTransformMatrix (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [in] pmatlfArray PMATRIXLF.
 * @param [in] lOptions LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetModelTransformMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray, LONG lOptions)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiCallSegmentMatrix (stub).
 * @param [in] hps HPS.
 * @param [in] lSegment LONG.
 * @param [in] lCount LONG.
 * @param [in] pmatlfArray PMATRIXLF.
 * @param [in] lOptions LONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiCallSegmentMatrix(HPS hps, LONG lSegment, LONG lCount, PMATRIXLF pmatlfArray, LONG lOptions)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryDefaultViewMatrix (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [out] pmatlfArray PMATRIXLF.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryDefaultViewMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiSetDefaultViewMatrix (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [in] pmatlfarray PMATRIXLF.
 * @param [in] lOptions LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetDefaultViewMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfarray, LONG lOptions)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryPageViewport (stub).
 * @param [in] hps HPS.
 * @param [out] prclViewport PRECTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryPageViewport(HPS hps, PRECTL prclViewport)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryViewingTransformMatrix (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [out] pmatlfArray PMATRIXLF.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryViewingTransformMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiSetViewingTransformMatrix (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [in] pmatlfArray PMATRIXLF.
 * @param [in] lOptions LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetViewingTransformMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray, LONG lOptions)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiTranslate (stub).
 * @param [in] p1 HPS.
 * @param [in] p2 PMATRIXLF.
 * @param [in] p3 LONG.
 * @param [in] p4 PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiTranslate(HPS p1, PMATRIXLF p2, LONG p3, PPOINTL p4)
{
    (void)p1;
    (void)p2;
    (void)p3;
    (void)p4;
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiScale (stub).
 * @param [in] p1 HPS.
 * @param [in] p2 PMATRIXLF.
 * @param [in] p3 LONG.
 * @param [in] p4 PFIXED.
 * @param [in] p5 PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiScale(HPS p1, PMATRIXLF p2, LONG p3, PFIXED p4, PPOINTL p5)
{
    (void)p1;
    (void)p2;
    (void)p3;
    (void)p4;
    (void)p5;
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiRotate (stub).
 * @param [in] p1 HPS.
 * @param [in] p2 PMATRIXLF.
 * @param [in] p3 LONG.
 * @param [in] p4 FIXED.
 * @param [in] p5 PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiRotate(HPS p1, PMATRIXLF p2, LONG p3, FIXED p4, PPOINTL p5)
{
    (void)p1;
    (void)p2;
    (void)p3;
    (void)p4;
    (void)p5;
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryGraphicsField (stub).
 * @param [in] hps HPS.
 * @param [out] prclField PRECTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryGraphicsField(HPS hps, PRECTL prclField)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryViewingLimits (stub).
 * @param [in] hps HPS.
 * @param [out] prclLimits PRECTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryViewingLimits(HPS hps, PRECTL prclLimits)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiCreateLogColorTable (stub).
 * @param [in] hps HPS.
 * @param [in] flOptions ULONG.
 * @param [in] lFormat LONG.
 * @param [in] lStart LONG.
 * @param [in] lCount LONG.
 * @param [in] alTable PLONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiCreateLogColorTable(HPS hps, ULONG flOptions, LONG lFormat, LONG lStart, LONG lCount, PLONG alTable)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryColorData (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [out] alArray PLONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryColorData(HPS hps, LONG lCount, PLONG alArray)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryLogColorTable (stub).
 * @param [in] hps HPS.
 * @param [in] flOptions ULONG.
 * @param [in] lStart LONG.
 * @param [in] lCount LONG.
 * @param [out] alArray PLONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryLogColorTable(HPS hps, ULONG flOptions, LONG lStart, LONG lCount, PLONG alArray)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryRealColors (stub).
 * @param [in] hps HPS.
 * @param [in] flOptions ULONG.
 * @param [in] lStart LONG.
 * @param [in] lCount LONG.
 * @param [out] alColors PLONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryRealColors(HPS hps, ULONG flOptions, LONG lStart, LONG lCount, PLONG alColors)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiCreatePalette (stub).
 * @param [in] hab HAB.
 * @param [in] flOptions ULONG.
 * @param [in] ulFormat ULONG.
 * @param [in] ulCount ULONG.
 * @param [in] aulTable PULONG.
 * @return Handle, or GPI_ERROR.
 */
HPAL APIENTRY GpiCreatePalette(HAB hab, ULONG flOptions, ULONG ulFormat, ULONG ulCount, PULONG aulTable)
{
    return (HPAL)GPI_ERROR;
}

/*!
 * @brief GpiDeletePalette (stub).
 * @param [in] hpal HPAL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiDeletePalette(HPAL hpal)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiAnimatePalette (stub).
 * @param [in] hpal HPAL.
 * @param [in] ulFormat ULONG.
 * @param [in] ulStart ULONG.
 * @param [in] ulCount ULONG.
 * @param [in] aulTable PULONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiAnimatePalette(HPAL hpal, ULONG ulFormat, ULONG ulStart, ULONG ulCount, PULONG aulTable)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiSetPaletteEntries (stub).
 * @param [in] hpal HPAL.
 * @param [in] ulFormat ULONG.
 * @param [in] ulStart ULONG.
 * @param [in] ulCount ULONG.
 * @param [in] aulTable PULONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetPaletteEntries(HPAL hpal, ULONG ulFormat, ULONG ulStart, ULONG ulCount, PULONG aulTable)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryPaletteInfo (stub).
 * @param [in] hpal HPAL.
 * @param [in] hps HPS.
 * @param [in] flOptions ULONG.
 * @param [in] ulStart ULONG.
 * @param [in] ulCount ULONG.
 * @param [out] aulArray PULONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryPaletteInfo(HPAL hpal, HPS hps, ULONG flOptions, ULONG ulStart, ULONG ulCount, PULONG aulArray)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiResizePalette (stub).
 * @param [in] hpal HPAL.
 * @param [in] newsize ULONG.
 * @return Implementation-defined value.
 */
ULONG APIENTRY GpiResizePalette(HPAL hpal, ULONG newsize)
{
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryNearestPaletteIndex (stub).
 * @param [in] hpal HPAL.
 * @param [in] color ULONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryNearestPaletteIndex(HPAL hpal, ULONG color)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiPolyLine (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [in] aptlPoints PPOINTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiPolyLine(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiPolyLineDisjoint (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [in] aptlPoints PPOINTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiPolyLineDisjoint(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiCharString (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [in] pchString PCH.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiCharString(HPS hps, LONG lCount, PCH pchString)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiCharStringAt (stub).
 * @param [in] hps HPS.
 * @param [in] pptlPoint PPOINTL.
 * @param [in] lCount LONG.
 * @param [in] pchString PCH.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiCharStringAt(HPS hps, PPOINTL pptlPoint, LONG lCount, PCH pchString)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiSetAttrs (stub).
 * @param [in] hps HPS.
 * @param [in] lPrimType LONG.
 * @param [in] flAttrMask ULONG.
 * @param [in] flDefMask ULONG.
 * @param [in] ppbunAttrs PBUNDLE.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask, ULONG flDefMask, PBUNDLE ppbunAttrs)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryAttrs (stub).
 * @param [in] hps HPS.
 * @param [in] lPrimType LONG.
 * @param [in] flAttrMask ULONG.
 * @param [in] ppbunAttrs PBUNDLE.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask, PBUNDLE ppbunAttrs)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryCurrentPosition (stub).
 * @param [in] hps HPS.
 * @param [out] pptlPoint PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryCurrentPosition(HPS hps, PPOINTL pptlPoint)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryArcParams (stub).
 * @param [in] hps HPS.
 * @param [out] parcpArcParams PARCPARAMS.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryArcParams(HPS hps, PARCPARAMS parcpArcParams)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiOval (stub).
 * @param [in] hps HPS.
 * @param [in] aptlPoints PPOINTL.
 * @param [in] fxStartAngle FIXED.
 * @param [in] fxSweepAngle FIXED.
 * @param [in] ulOptions ULONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiOval(HPS hps, PPOINTL aptlPoints, FIXED fxStartAngle, FIXED fxSweepAngle, ULONG ulOptions)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiPolyFillet (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [in] aptlPoints PPOINTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiPolyFillet(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiPolySpline (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [in] aptlPoints PPOINTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiPolySpline(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiPolyFilletSharp (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [in] aptlPoints PPOINTL.
 * @param [in] afxPoints PFIXED.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiPolyFilletSharp(HPS hps, LONG lCount, PPOINTL aptlPoints, PFIXED afxPoints)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryPatternRefPoint (stub).
 * @param [in] hps HPS.
 * @param [out] pptlRefPoint PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryPatternRefPoint(HPS hps, PPOINTL pptlRefPoint)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryCharStringPos (stub).
 * @param [in] hps HPS.
 * @param [in] flOptions ULONG.
 * @param [in] lCount LONG.
 * @param [out] pchString PCH.
 * @param [out] alXincrements PLONG.
 * @param [out] aptlPositions PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryCharStringPos(HPS hps, ULONG flOptions, LONG lCount, PCH pchString, PLONG alXincrements, PPOINTL aptlPositions)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryCharStringPosAt (stub).
 * @param [in] hps HPS.
 * @param [out] pptlStart PPOINTL.
 * @param [in] flOptions ULONG.
 * @param [in] lCount LONG.
 * @param [out] pchString PCH.
 * @param [out] alXincrements PLONG.
 * @param [out] aptlPositions PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryCharStringPosAt(HPS hps, PPOINTL pptlStart, ULONG flOptions, LONG lCount, PCH pchString, PLONG alXincrements, PPOINTL aptlPositions)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryTabbedTextExtent (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [out] pchString PCH.
 * @param [in] lTabCount LONG.
 * @param [out] puTabStops PULONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryTabbedTextExtent(HPS hps, LONG lCount, PCH pchString, LONG lTabCount, PULONG puTabStops)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryTextBox (stub).
 * @param [in] hps HPS.
 * @param [in] lCount1 LONG.
 * @param [out] pchString PCH.
 * @param [in] lCount2 LONG.
 * @param [out] aptlPoints PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryTextBox(HPS hps, LONG lCount1, PCH pchString, LONG lCount2, PPOINTL aptlPoints)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryDefCharBox (stub).
 * @param [in] hps HPS.
 * @param [out] psizlSize PSIZEL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryDefCharBox(HPS hps, PSIZEL psizlSize)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryCharBox (stub).
 * @param [in] hps HPS.
 * @param [out] psizfxSize PSIZEF.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryCharBox(HPS hps, PSIZEF psizfxSize)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryCharAngle (stub).
 * @param [in] hps HPS.
 * @param [out] pgradlAngle PGRADIENTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryCharAngle(HPS hps, PGRADIENTL pgradlAngle)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryCharShear (stub).
 * @param [in] hps HPS.
 * @param [out] pptlShear PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryCharShear(HPS hps, PPOINTL pptlShear)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryTextAlignment (stub).
 * @param [in] hps HPS.
 * @param [out] plHoriz PLONG.
 * @param [out] plVert PLONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryTextAlignment(HPS hps, PLONG plHoriz, PLONG plVert)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiCharStringPos (stub).
 * @param [in] hps HPS.
 * @param [in] prclRect PRECTL.
 * @param [in] flOptions ULONG.
 * @param [in] lCount LONG.
 * @param [in] pchString PCH.
 * @param [in] alAdx PLONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiCharStringPos(HPS hps, PRECTL prclRect, ULONG flOptions, LONG lCount, PCH pchString, PLONG alAdx)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiTabbedCharStringAt (stub).
 * @param [in] hps HPS.
 * @param [in] pPtStart PPOINTL.
 * @param [in] prclRect PRECTL.
 * @param [in] flOptions ULONG.
 * @param [in] lCount LONG.
 * @param [in] pchString PCH.
 * @param [in] lTabCount LONG.
 * @param [in] puTabStops PULONG.
 * @param [in] lTabOrigin LONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiTabbedCharStringAt(HPS hps, PPOINTL pPtStart, PRECTL prclRect, ULONG flOptions, LONG lCount, PCH pchString, LONG lTabCount, PULONG puTabStops, LONG lTabOrigin)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiCharStringPosAt (stub).
 * @param [in] hps HPS.
 * @param [in] pptlStart PPOINTL.
 * @param [in] prclRect PRECTL.
 * @param [in] flOptions ULONG.
 * @param [in] lCount LONG.
 * @param [in] pchString PCH.
 * @param [in] alAdx PLONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiCharStringPosAt(HPS hps, PPOINTL pptlStart, PRECTL prclRect, ULONG flOptions, LONG lCount, PCH pchString, PLONG alAdx)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryCharExtra (stub).
 * @param [in] hps HPS.
 * @param [in] Extra PFIXED.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryCharExtra(HPS hps, PFIXED Extra)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryCharBreakExtra (stub).
 * @param [in] hps HPS.
 * @param [in] BreakExtra PFIXED.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryCharBreakExtra(HPS hps, PFIXED BreakExtra)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiPolyMarker (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [in] aptlPoints PPOINTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiPolyMarker(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryMarkerBox (stub).
 * @param [in] hps HPS.
 * @param [out] psizfxSize PSIZEF.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryMarkerBox(HPS hps, PSIZEF psizfxSize)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiImage (stub).
 * @param [in] hps HPS.
 * @param [in] lFormat LONG.
 * @param [in] psizlImageSize PSIZEL.
 * @param [in] lLength LONG.
 * @param [in] pbData PBYTE.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiImage(HPS hps, LONG lFormat, PSIZEL psizlImageSize, LONG lLength, PBYTE pbData)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiComment (stub).
 * @param [in] hps HPS.
 * @param [in] lLength LONG.
 * @param [in] pbData PBYTE.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiComment(HPS hps, LONG lLength, PBYTE pbData)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryRasterizerCaps (stub).
 * @param [in] p1 PRASTERIZERCAPS.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryRasterizerCaps(PRASTERIZERCAPS p1)
{
    (void)p1;
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiCreateLogFont (stub).
 * @param [in] hps HPS.
 * @param [in] pName PSTR8.
 * @param [in] lLcid LONG.
 * @param [in] pfatAttrs PFATTRS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiCreateLogFont(HPS hps, PSTR8 pName, LONG lLcid, PFATTRS pfatAttrs)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiLoadFonts (stub).
 * @param [in] hab HAB.
 * @param [in] pszFilename PCSZ.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiLoadFonts(HAB hab, PCSZ pszFilename)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiUnloadFonts (stub).
 * @param [in] hab HAB.
 * @param [in] pszFilename PCSZ.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiUnloadFonts(HAB hab, PCSZ pszFilename)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryFonts (stub).
 * @param [in] hps HPS.
 * @param [in] flOptions ULONG.
 * @param [in] pszFacename PCSZ.
 * @param [out] plReqFonts PLONG.
 * @param [in] lMetricsLength LONG.
 * @param [out] afmMetrics PFONTMETRICS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryFonts(HPS hps, ULONG flOptions, PCSZ pszFacename, PLONG plReqFonts, LONG lMetricsLength, PFONTMETRICS afmMetrics)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryFontMetrics (stub).
 * @param [in] hps HPS.
 * @param [in] lMetricsLength LONG.
 * @param [out] pfmMetrics PFONTMETRICS.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryFontMetrics(HPS hps, LONG lMetricsLength, PFONTMETRICS pfmMetrics)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryKerningPairs (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [out] akrnprData PKERNINGPAIRS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryKerningPairs(HPS hps, LONG lCount, PKERNINGPAIRS akrnprData)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryWidthTable (stub).
 * @param [in] hps HPS.
 * @param [in] lFirstChar LONG.
 * @param [in] lCount LONG.
 * @param [out] alData PLONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryWidthTable(HPS hps, LONG lFirstChar, LONG lCount, PLONG alData)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQuerySetIds (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [out] alTypes PLONG.
 * @param [out] aNames PSTR8.
 * @param [out] allcids PLONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQuerySetIds(HPS hps, LONG lCount, PLONG alTypes, PSTR8 aNames, PLONG allcids)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryFaceString (stub).
 * @param [in] PS HPS.
 * @param [in] FamilyName PCSZ.
 * @param [in] attrs PFACENAMEDESC.
 * @param [in] length LONG.
 * @param [in] CompoundFaceName PCSZ.
 * @return Implementation-defined value.
 */
ULONG APIENTRY GpiQueryFaceString(HPS PS, PCSZ FamilyName, PFACENAMEDESC attrs, LONG length, PCSZ CompoundFaceName)
{
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryLogicalFont (stub).
 * @param [in] PS HPS.
 * @param [in] lcid LONG.
 * @param [out] name PSTR8.
 * @param [out] attrs PFATTRS.
 * @param [in] length LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryLogicalFont(HPS PS, LONG lcid, PSTR8 name, PFATTRS attrs, LONG length)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryFontAction (stub).
 * @param [in] anchor HAB.
 * @param [in] options ULONG.
 * @return Implementation-defined value.
 */
ULONG APIENTRY GpiQueryFontAction(HAB anchor, ULONG options)
{
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief GpiLoadPublicFonts (stub).
 * @param [in] p1 HAB.
 * @param [in] p2 PCSZ.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiLoadPublicFonts(HAB p1, PCSZ p2)
{
    (void)p1;
    (void)p2;
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiUnloadPublicFonts (stub).
 * @param [in] p1 HAB.
 * @param [in] p2 PCSZ.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiUnloadPublicFonts(HAB p1, PCSZ p2)
{
    (void)p1;
    (void)p2;
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryFullFontFileDescs (stub).
 * @param [in] hab HAB.
 * @param [in] pszFilename PCSZ.
 * @param [out] plCount PLONG.
 * @param [out] pNames PVOID.
 * @param [out] plNamesBuffLength PLONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryFullFontFileDescs(HAB hab, PCSZ pszFilename, PLONG plCount, PVOID pNames, PLONG plNamesBuffLength)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryFontFileDescriptions (stub).
 * @param [in] hab HAB.
 * @param [in] pszFilename PCSZ.
 * @param [out] plCount PLONG.
 * @param [in] affdescsNames PFFDESCS.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryFontFileDescriptions(HAB hab, PCSZ pszFilename, PLONG plCount, PFFDESCS affdescsNames)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiBitBlt (stub).
 * @param [in] hpsTarget HPS.
 * @param [in] hpsSource HPS.
 * @param [in] lCount LONG.
 * @param [in] aptlPoints PPOINTL.
 * @param [in] lRop LONG.
 * @param [in] flOptions ULONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiBitBlt(HPS hpsTarget, HPS hpsSource, LONG lCount, PPOINTL aptlPoints, LONG lRop, ULONG flOptions)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiDeleteBitmap (stub).
 * @param [in] hbm HBITMAP.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiDeleteBitmap(HBITMAP hbm)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiWCBitBlt (stub).
 * @param [in] hpsTarget HPS.
 * @param [in] hbmSource HBITMAP.
 * @param [in] lCount LONG.
 * @param [in] aptlPoints PPOINTL.
 * @param [in] lRop LONG.
 * @param [in] flOptions ULONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiWCBitBlt(HPS hpsTarget, HBITMAP hbmSource, LONG lCount, PPOINTL aptlPoints, LONG lRop, ULONG flOptions)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiCreateBitmap (stub).
 * @param [in] hps HPS.
 * @param [in] pbmpNew PBITMAPINFOHEADER2.
 * @param [in] flOptions ULONG.
 * @param [in] pbInitData PBYTE.
 * @param [in] pbmiInfoTable PBITMAPINFO2.
 * @return Handle, or GPI_ERROR.
 */
HBITMAP APIENTRY GpiCreateBitmap(HPS hps, PBITMAPINFOHEADER2 pbmpNew, ULONG flOptions, PBYTE pbInitData, PBITMAPINFO2 pbmiInfoTable)
{
    return (HBITMAP)GPI_ERROR;
}

/*!
 * @brief GpiSetBitmapBits (stub).
 * @param [in] hps HPS.
 * @param [in] lScanStart LONG.
 * @param [in] lScans LONG.
 * @param [in] pbBuffer PBYTE.
 * @param [in] pbmiInfoTable PBITMAPINFO2.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiSetBitmapBits(HPS hps, LONG lScanStart, LONG lScans, PBYTE pbBuffer, PBITMAPINFO2 pbmiInfoTable)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiSetBitmapDimension (stub).
 * @param [in] hbm HBITMAP.
 * @param [in] psizlBitmapDimension PSIZEL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetBitmapDimension(HBITMAP hbm, PSIZEL psizlBitmapDimension)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryBitmapBits (stub).
 * @param [in] hps HPS.
 * @param [in] lScanStart LONG.
 * @param [in] lScans LONG.
 * @param [out] pbBuffer PBYTE.
 * @param [in] pbmiInfoTable PBITMAPINFO2.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryBitmapBits(HPS hps, LONG lScanStart, LONG lScans, PBYTE pbBuffer, PBITMAPINFO2 pbmiInfoTable)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryBitmapDimension (stub).
 * @param [in] hbm HBITMAP.
 * @param [out] psizlBitmapDimension PSIZEL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryBitmapDimension(HBITMAP hbm, PSIZEL psizlBitmapDimension)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryBitmapParameters (stub).
 * @param [in] hbm HBITMAP.
 * @param [in] pbmpData PBITMAPINFOHEADER.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryBitmapParameters(HBITMAP hbm, PBITMAPINFOHEADER pbmpData)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryBitmapInfoHeader (stub).
 * @param [in] hbm HBITMAP.
 * @param [in] pbmpData PBITMAPINFOHEADER2.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryBitmapInfoHeader(HBITMAP hbm, PBITMAPINFOHEADER2 pbmpData)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryDeviceBitmapFormats (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [out] alArray PLONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryDeviceBitmapFormats(HPS hps, LONG lCount, PLONG alArray)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryPel (stub).
 * @param [in] hps HPS.
 * @param [out] pptlPoint PPOINTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryPel(HPS hps, PPOINTL pptlPoint)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiDrawBits (stub).
 * @param [in] hps HPS.
 * @param [in] pBits PVOID.
 * @param [in] pbmiInfoTable PBITMAPINFO2.
 * @param [in] lCount LONG.
 * @param [in] aptlPoints PPOINTL.
 * @param [in] lRop LONG.
 * @param [in] flOptions ULONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiDrawBits(HPS hps, PVOID pBits, PBITMAPINFO2 pbmiInfoTable, LONG lCount, PPOINTL aptlPoints, LONG lRop, ULONG flOptions)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiCreateRegion (stub).
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [in] arclRectangles PRECTL.
 * @return Handle, or GPI_ERROR.
 */
HRGN APIENTRY GpiCreateRegion(HPS hps, LONG lCount, PRECTL arclRectangles)
{
    return (HRGN)GPI_ERROR;
}

/*!
 * @brief GpiQueryRegionBox (stub).
 * @param [in] hps HPS.
 * @param [in] hrgn HRGN.
 * @param [out] prclBound PRECTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryRegionBox(HPS hps, HRGN hrgn, PRECTL prclBound)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryRegionRects (stub).
 * @param [in] hps HPS.
 * @param [in] hrgn HRGN.
 * @param [out] prclBound PRECTL.
 * @param [in] prgnrcControl PRGNRECT.
 * @param [out] prclRect PRECTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryRegionRects(HPS hps, HRGN hrgn, PRECTL prclBound, PRGNRECT prgnrcControl, PRECTL prclRect)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiSetRegion (stub).
 * @param [in] hps HPS.
 * @param [in] hrgn HRGN.
 * @param [in] lcount LONG.
 * @param [in] arclRectangles PRECTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetRegion(HPS hps, HRGN hrgn, LONG lcount, PRECTL arclRectangles)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiSetClipRegion (stub).
 * @param [in] hps HPS.
 * @param [in] hrgn HRGN.
 * @param [in] phrgnOld PHRGN.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiSetClipRegion(HPS hps, HRGN hrgn, PHRGN phrgnOld)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryClipBox (stub).
 * @param [in] hps HPS.
 * @param [out] prclBound PRECTL.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryClipBox(HPS hps, PRECTL prclBound)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiCopyMetaFile (stub).
 * @param [in] hmf HMF.
 * @return Handle, or GPI_ERROR.
 */
HMF APIENTRY GpiCopyMetaFile(HMF hmf)
{
    return (HMF)GPI_ERROR;
}

/*!
 * @brief GpiDeleteMetaFile (stub).
 * @param [in] hmf HMF.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiDeleteMetaFile(HMF hmf)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiLoadMetaFile (stub).
 * @param [in] hab HAB.
 * @param [in] pszFilename PCSZ.
 * @return Handle, or GPI_ERROR.
 */
HMF APIENTRY GpiLoadMetaFile(HAB hab, PCSZ pszFilename)
{
    return (HMF)GPI_ERROR;
}

/*!
 * @brief GpiPlayMetaFile (stub).
 * @param [in] hps HPS.
 * @param [in] hmf HMF.
 * @param [in] lCount1 LONG.
 * @param [in] alOptarray PLONG.
 * @param [in] plSegCount PLONG.
 * @param [in] lCount2 LONG.
 * @param [in] pszDesc PCSZ.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiPlayMetaFile(HPS hps, HMF hmf, LONG lCount1, PLONG alOptarray, PLONG plSegCount, LONG lCount2, PCSZ pszDesc)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryMetaFileBits (stub).
 * @param [in] hmf HMF.
 * @param [in] lOffset LONG.
 * @param [in] lLength LONG.
 * @param [out] pbData PBYTE.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryMetaFileBits(HMF hmf, LONG lOffset, LONG lLength, PBYTE pbData)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryMetaFileLength (stub).
 * @param [in] hmf HMF.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryMetaFileLength(HMF hmf)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiSaveMetaFile (stub).
 * @param [in] hmf HMF.
 * @param [in] pszFilename PCSZ.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSaveMetaFile(HMF hmf, PCSZ pszFilename)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiSetMetaFileBits (stub).
 * @param [in] hmf HMF.
 * @param [in] lOffset LONG.
 * @param [in] lLength LONG.
 * @param [in] pbBuffer PBYTE.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetMetaFileBits(HMF hmf, LONG lOffset, LONG lLength, PBYTE pbBuffer)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryDefArcParams (stub).
 * @param [in] hps HPS.
 * @param [out] parcpArcParams PARCPARAMS.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryDefArcParams(HPS hps, PARCPARAMS parcpArcParams)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryDefAttrs (stub).
 * @param [in] hps HPS.
 * @param [in] lPrimType LONG.
 * @param [in] flAttrMask ULONG.
 * @param [in] ppbunAttrs PBUNDLE.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryDefAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask, PBUNDLE ppbunAttrs)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryDefTag (stub).
 * @param [in] hps HPS.
 * @param [out] plTag PLONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryDefTag(HPS hps, PLONG plTag)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiQueryDefViewingLimits (stub).
 * @param [in] hps HPS.
 * @param [out] prclLimits PRECTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryDefViewingLimits(HPS hps, PRECTL prclLimits)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiSetDefAttrs (stub).
 * @param [in] hps HPS.
 * @param [in] lPrimType LONG.
 * @param [in] flAttrMask ULONG.
 * @param [in] ppbunAttrs PBUNDLE.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetDefAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask, PBUNDLE ppbunAttrs)
{
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief GpiPolygons (stub).
 * @param [in] hps HPS.
 * @param [in] ulCount ULONG.
 * @param [in] paplgn PPOLYGON.
 * @param [in] flOptions ULONG.
 * @param [in] flModel ULONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiPolygons(HPS hps, ULONG ulCount, PPOLYGON paplgn, ULONG flOptions, ULONG flModel)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiCreatePolygonRegion (stub).
 * @param [in] hps HPS.
 * @param [in] ulCount ULONG.
 * @param [in] paplgn PPOLYGON.
 * @param [in] flOptions ULONG.
 * @return Handle, or GPI_ERROR.
 */
HRGN APIENTRY GpiCreatePolygonRegion(HPS hps, ULONG ulCount, PPOLYGON paplgn, ULONG flOptions)
{
    return (HRGN)GPI_ERROR;
}

/*!
 * @brief GpiStrokeInkPath (stub).
 * @param [in] hps HPS.
 * @param [in] lPath LONG.
 * @param [in] lCount LONG.
 * @param [in] aptlPoints PPOINTL.
 * @param [in] flOptions ULONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiStrokeInkPath(HPS hps, LONG lPath, LONG lCount, PPOINTL aptlPoints, ULONG flOptions)
{
    return (LONG)GPI_ERROR;
}

/*!
 * @brief GpiSetDCOwner (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
ULONG APIENTRY GpiSetDCOwner(void)
{
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief GpiSetBitmapOwner (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
ULONG APIENTRY GpiSetBitmapOwner(void)
{
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief GpiSetRegionOwner (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
ULONG APIENTRY GpiSetRegionOwner(void)
{
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief GpiInstallIFIFont (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
ULONG APIENTRY GpiInstallIFIFont(void)
{
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief GpiOpenMetafile (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
ULONG APIENTRY GpiOpenMetafile(void)
{
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief GpiCloseMetafile (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
ULONG APIENTRY GpiCloseMetafile(void)
{
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief GpiConvPSH (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
ULONG APIENTRY GpiConvPSH(void)
{
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief GpiMtAssociate (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
ULONG APIENTRY GpiMtAssociate(void)
{
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief GpiMtDisassociate (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
ULONG APIENTRY GpiMtDisassociate(void)
{
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief GpiEnableYInversion (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
ULONG APIENTRY GpiEnableYInversion(void)
{
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryYInversion (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
ULONG APIENTRY GpiQueryYInversion(void)
{
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief GpiSaveMetaFile2 (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
ULONG APIENTRY GpiSaveMetaFile2(void)
{
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief GpiAllocateDCData (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
ULONG APIENTRY GpiAllocateDCData(void)
{
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief GpiQueryDCData (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
ULONG APIENTRY GpiQueryDCData(void)
{
    return (ULONG)GPI_ERROR;
}

/* ------------------------------------------------------------------
 * Placeholder stubs for entry points that exist in the server's
 * pmgpi.c only as "fix prototype !!!" placeholders (undocNNN, Dsp*,
 * Dev*, GPI*INKPATH, Pax*). They are listed in pmgpi.def for
 * PMMERGE ABI compatibility but have no real prototype yet, so they
 * are NOT proxied through the F_CMD_GPI_DISPATCH wire protocol.
 * Each stub returns GPI_ERROR until a real prototype exists; at that
 * point it can be moved to gpi_proxies.c / wire codes.
 * ------------------------------------------------------------------ */
#define FPM_UNDOC_STUB(name) EXTERN ULONG APIENTRY name(void) { return (ULONG)GPI_ERROR; }

FPM_UNDOC_STUB(undoc625)
FPM_UNDOC_STUB(undoc626)
FPM_UNDOC_STUB(undoc630)
FPM_UNDOC_STUB(undoc631)
FPM_UNDOC_STUB(undoc632)
FPM_UNDOC_STUB(undoc633)
FPM_UNDOC_STUB(undoc634)
FPM_UNDOC_STUB(undoc635)
FPM_UNDOC_STUB(undoc636)
FPM_UNDOC_STUB(undoc637)
FPM_UNDOC_STUB(undoc638)
FPM_UNDOC_STUB(undoc639)
FPM_UNDOC_STUB(undoc640)
FPM_UNDOC_STUB(undoc641)
FPM_UNDOC_STUB(undoc642)
FPM_UNDOC_STUB(undoc643)
FPM_UNDOC_STUB(undoc646)
FPM_UNDOC_STUB(undoc651)
FPM_UNDOC_STUB(undoc652)
FPM_UNDOC_STUB(undoc653)
FPM_UNDOC_STUB(undoc658)
FPM_UNDOC_STUB(undoc659)
FPM_UNDOC_STUB(undoc660)
FPM_UNDOC_STUB(undoc661)
FPM_UNDOC_STUB(undoc662)
FPM_UNDOC_STUB(undoc663)
FPM_UNDOC_STUB(undoc666)
FPM_UNDOC_STUB(undoc667)
FPM_UNDOC_STUB(undoc668)
FPM_UNDOC_STUB(undoc669)
FPM_UNDOC_STUB(undoc722)
FPM_UNDOC_STUB(undoc724)
FPM_UNDOC_STUB(undoc725)
FPM_UNDOC_STUB(undoc727)
FPM_UNDOC_STUB(undoc730)

FPM_UNDOC_STUB(DspDefaultResolution)
FPM_UNDOC_STUB(DevQueryDisplayResolutions)
FPM_UNDOC_STUB(DspInitSystemDriverName)

FPM_UNDOC_STUB(GPIBEGININKPATH)
FPM_UNDOC_STUB(GPIENDINKPATH)
FPM_UNDOC_STUB(GPISTROKEINKPATH)

FPM_UNDOC_STUB(PaxGetPolyFillMode)
FPM_UNDOC_STUB(PaxSetPolyFillMode)
FPM_UNDOC_STUB(PaxBeginPath)
FPM_UNDOC_STUB(PaxCloseFigure)
FPM_UNDOC_STUB(PaxEndPath)
FPM_UNDOC_STUB(PaxFillPath)
FPM_UNDOC_STUB(PaxFlattenPath)
FPM_UNDOC_STUB(PaxPathToRegion)
FPM_UNDOC_STUB(PaxStrokeAndFillPath)
FPM_UNDOC_STUB(PaxStrokePath)
FPM_UNDOC_STUB(PaxWidenPath)
FPM_UNDOC_STUB(PaxArc)
FPM_UNDOC_STUB(PaxChord)
FPM_UNDOC_STUB(PaxEllipse)
FPM_UNDOC_STUB(PaxLineDDA)
FPM_UNDOC_STUB(PaxPie)
FPM_UNDOC_STUB(PaxPolyBezier)
FPM_UNDOC_STUB(PaxPolyBezierAt)

FPM_UNDOC_STUB(DevCloseDC)
FPM_UNDOC_STUB(DevEscape)
FPM_UNDOC_STUB(DevQueryCaps)
FPM_UNDOC_STUB(DevQueryDeviceNames)
FPM_UNDOC_STUB(DevQueryHardcopyCaps)
FPM_UNDOC_STUB(DevPostDeviceModes)
FPM_UNDOC_STUB(DevOpenDC)

#undef FPM_UNDOC_STUB
