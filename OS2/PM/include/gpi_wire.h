/*!
 * @file gpi_wire.h
 * @brief Wire codes for the PMGPI client-server protocol.
 */
#ifndef GPI_WIRE_H_INCLUDED
#define GPI_WIRE_H_INCLUDED

#include "F_platform.h"

#define NGpiCreatePS                     0x00000001L
#define NGpiDestroyPS                    0x00000002L
#define NGpiAssociate                    0x00000003L
#define NGpiRestorePS                    0x00000004L
#define NGpiSavePS                       0x00000005L
#define NGpiErase                        0x00000006L
#define NGpiQueryDevice                  0x00000007L
#define NGpiResetPS                      0x00000008L
#define NGpiSetPS                        0x00000009L
#define NGpiQueryPS                      0x0000000AL
#define NGpiErrorSegmentData             0x0000000BL
#define NGpiQueryDrawControl             0x0000000CL
#define NGpiSetDrawControl               0x0000000DL
#define NGpiQueryDrawingMode             0x0000000EL
#define NGpiSetDrawingMode               0x0000000FL
#define NGpiQueryStopDraw                0x00000010L
#define NGpiSetStopDraw                  0x00000011L
#define NGpiCorrelateChain               0x00000012L
#define NGpiQueryTag                     0x00000013L
#define NGpiSetTag                       0x00000014L
#define NGpiQueryPickApertureSize        0x00000015L
#define NGpiSetPickApertureSize          0x00000016L
#define NGpiQueryPickAperturePosition    0x00000017L
#define NGpiSetPickAperturePosition      0x00000018L
#define NGpiQueryBoundaryData            0x00000019L
#define NGpiResetBoundaryData            0x0000001AL
#define NGpiCorrelateFrom                0x0000001BL
#define NGpiCorrelateSegment             0x0000001CL
#define NGpiOpenSegment                  0x0000001DL
#define NGpiCloseSegment                 0x0000001EL
#define NGpiDeleteSegment                0x0000001FL
#define NGpiQueryInitialSegmentAttrs     0x00000020L
#define NGpiSetInitialSegmentAttrs       0x00000021L
#define NGpiQuerySegmentAttrs            0x00000022L
#define NGpiSetSegmentAttrs              0x00000023L
#define NGpiQuerySegmentPriority         0x00000024L
#define NGpiSetSegmentPriority           0x00000025L
#define NGpiDeleteSegments               0x00000026L
#define NGpiQuerySegmentNames            0x00000027L
#define NGpiGetData                      0x00000028L
#define NGpiPutData                      0x00000029L
#define NGpiDrawChain                    0x0000002AL
#define NGpiDrawFrom                     0x0000002BL
#define NGpiDrawSegment                  0x0000002CL
#define NGpiDrawDynamics                 0x0000002DL
#define NGpiRemoveDynamics               0x0000002EL
#define NGpiBeginElement                 0x0000002FL
#define NGpiEndElement                   0x00000030L
#define NGpiLabel                        0x00000031L
#define NGpiElement                      0x00000032L
#define NGpiQueryElement                 0x00000033L
#define NGpiDeleteElement                0x00000034L
#define NGpiDeleteElementRange           0x00000035L
#define NGpiDeleteElementsBetweenLabels  0x00000036L
#define NGpiQueryEditMode                0x00000037L
#define NGpiSetEditMode                  0x00000038L
#define NGpiQueryElementPointer          0x00000039L
#define NGpiSetElementPointer            0x0000003AL
#define NGpiOffsetElementPointer         0x0000003BL
#define NGpiQueryElementType             0x0000003CL
#define NGpiSetElementPointerAtLabel     0x0000003DL
#define NGpiQuerySegmentTransformMatrix  0x0000003EL
#define NGpiSetSegmentTransformMatrix    0x0000003FL
#define NGpiConvert                      0x00000040L
#define NGpiConvertWithMatrix            0x00000041L
#define NGpiQueryModelTransformMatrix    0x00000042L
#define NGpiSetModelTransformMatrix      0x00000043L
#define NGpiCallSegmentMatrix            0x00000044L
#define NGpiQueryDefaultViewMatrix       0x00000045L
#define NGpiSetDefaultViewMatrix         0x00000046L
#define NGpiQueryPageViewport            0x00000047L
#define NGpiSetPageViewport              0x00000048L
#define NGpiQueryViewingTransformMatrix  0x00000049L
#define NGpiSetViewingTransformMatrix    0x0000004AL
#define NGpiTranslate                    0x0000004BL
#define NGpiScale                        0x0000004CL
#define NGpiRotate                       0x0000004DL
#define NGpiSetGraphicsField             0x0000004EL
#define NGpiQueryGraphicsField           0x0000004FL
#define NGpiSetViewingLimits             0x00000050L
#define NGpiQueryViewingLimits           0x00000051L
#define NGpiBeginPath                    0x00000052L
#define NGpiEndPath                      0x00000053L
#define NGpiCloseFigure                  0x00000054L
#define NGpiModifyPath                   0x00000055L
#define NGpiFillPath                     0x00000056L
#define NGpiSetClipPath                  0x00000057L
#define NGpiOutlinePath                  0x00000058L
#define NGpiPathToRegion                 0x00000059L
#define NGpiStrokePath                   0x0000005AL
#define NGpiCreateLogColorTable          0x0000005BL
#define NGpiQueryColorData               0x0000005CL
#define NGpiQueryLogColorTable           0x0000005DL
#define NGpiQueryRealColors              0x0000005EL
#define NGpiQueryNearestColor            0x0000005FL
#define NGpiQueryColorIndex              0x00000060L
#define NGpiQueryRGBColor                0x00000061L
#define NGpiCreatePalette                0x00000062L
#define NGpiDeletePalette                0x00000063L
#define NGpiSelectPalette                0x00000064L
#define NGpiAnimatePalette               0x00000065L
#define NGpiSetPaletteEntries            0x00000066L
#define NGpiQueryPalette                 0x00000067L
#define NGpiQueryPaletteInfo             0x00000068L
#define NGpiResizePalette                0x00000069L
#define NGpiQueryNearestPaletteIndex     0x0000006AL
#define NGpiSetColor                     0x0000006BL
#define NGpiQueryColor                   0x0000006CL
#define NGpiBox                          0x0000006DL
#define NGpiMove                         0x0000006EL
#define NGpiLine                         0x0000006FL
#define NGpiPolyLine                     0x00000070L
#define NGpiPolyLineDisjoint             0x00000071L
#define NGpiSetPattern                   0x00000072L
#define NGpiQueryPattern                 0x00000073L
#define NGpiBeginArea                    0x00000074L
#define NGpiEndArea                      0x00000075L
#define NGpiCharString                   0x00000076L
#define NGpiCharStringAt                 0x00000077L
#define NGpiSetAttrMode                  0x00000078L
#define NGpiQueryAttrMode                0x00000079L
#define NGpiSetAttrs                     0x0000007AL
#define NGpiQueryAttrs                   0x0000007BL
#define NGpiSetBackColor                 0x0000007CL
#define NGpiQueryBackColor               0x0000007DL
#define NGpiSetMix                       0x0000007EL
#define NGpiQueryMix                     0x0000007FL
#define NGpiSetBackMix                   0x00000080L
#define NGpiQueryBackMix                 0x00000081L
#define NGpiSetLineType                  0x00000082L
#define NGpiQueryLineType                0x00000083L
#define NGpiSetLineWidth                 0x00000084L
#define NGpiQueryLineWidth               0x00000085L
#define NGpiSetLineWidthGeom             0x00000086L
#define NGpiQueryLineWidthGeom           0x00000087L
#define NGpiSetLineEnd                   0x00000088L
#define NGpiQueryLineEnd                 0x00000089L
#define NGpiSetLineJoin                  0x0000008AL
#define NGpiQueryLineJoin                0x0000008BL
#define NGpiSetCurrentPosition           0x0000008CL
#define NGpiQueryCurrentPosition         0x0000008DL
#define NGpiSetArcParams                 0x0000008EL
#define NGpiQueryArcParams               0x0000008FL
#define NGpiPointArc                     0x00000090L
#define NGpiFullArc                      0x00000091L
#define NGpiPartialArc                   0x00000092L
#define NGpiOval                         0x00000093L
#define NGpiPolyFillet                   0x00000094L
#define NGpiPolySpline                   0x00000095L
#define NGpiPolyFilletSharp              0x00000096L
#define NGpiSetPatternSet                0x00000097L
#define NGpiQueryPatternSet              0x00000098L
#define NGpiSetPatternRefPoint           0x00000099L
#define NGpiQueryPatternRefPoint         0x0000009AL
#define NGpiQueryCharStringPos           0x0000009BL
#define NGpiQueryCharStringPosAt         0x0000009CL
#define NGpiQueryTabbedTextExtent        0x0000009DL
#define NGpiQueryTextBox                 0x0000009EL
#define NGpiQueryDefCharBox              0x0000009FL
#define NGpiSetCharSet                   0x000000A0L
#define NGpiQueryCharSet                 0x000000A1L
#define NGpiSetCharBox                   0x000000A2L
#define NGpiQueryCharBox                 0x000000A3L
#define NGpiSetCharAngle                 0x000000A4L
#define NGpiQueryCharAngle               0x000000A5L
#define NGpiSetCharShear                 0x000000A6L
#define NGpiQueryCharShear               0x000000A7L
#define NGpiSetCharDirection             0x000000A8L
#define NGpiQueryCharDirection           0x000000A9L
#define NGpiSetCharMode                  0x000000AAL
#define NGpiQueryCharMode                0x000000ABL
#define NGpiSetTextAlignment             0x000000ACL
#define NGpiQueryTextAlignment           0x000000ADL
#define NGpiCharStringPos                0x000000AEL
#define NGpiTabbedCharStringAt           0x000000AFL
#define NGpiCharStringPosAt              0x000000B0L
#define NGpiSetCharExtra                 0x000000B1L
#define NGpiSetCharBreakExtra            0x000000B2L
#define NGpiQueryCharExtra               0x000000B3L
#define NGpiQueryCharBreakExtra          0x000000B4L
#define NGpiMarker                       0x000000B5L
#define NGpiPolyMarker                   0x000000B6L
#define NGpiSetMarker                    0x000000B7L
#define NGpiSetMarkerBox                 0x000000B8L
#define NGpiSetMarkerSet                 0x000000B9L
#define NGpiQueryMarker                  0x000000BAL
#define NGpiQueryMarkerBox               0x000000BBL
#define NGpiQueryMarkerSet               0x000000BCL
#define NGpiImage                        0x000000BDL
#define NGpiPop                          0x000000BEL
#define NGpiPtVisible                    0x000000BFL
#define NGpiRectVisible                  0x000000C0L
#define NGpiComment                      0x000000C1L
#define NGpiQueryRasterizerCaps          0x000000C2L
#define NGpiCreateLogFont                0x000000C3L
#define NGpiDeleteSetId                  0x000000C4L
#define NGpiLoadFonts                    0x000000C5L
#define NGpiUnloadFonts                  0x000000C6L
#define NGpiQueryFonts                   0x000000C7L
#define NGpiQueryFontMetrics             0x000000C8L
#define NGpiQueryKerningPairs            0x000000C9L
#define NGpiQueryWidthTable              0x000000CAL
#define NGpiQueryNumberSetIds            0x000000CBL
#define NGpiQuerySetIds                  0x000000CCL
#define NGpiQueryFaceString              0x000000CDL
#define NGpiQueryLogicalFont             0x000000CEL
#define NGpiQueryFontAction              0x000000CFL
#define NGpiLoadPublicFonts              0x000000D0L
#define NGpiUnloadPublicFonts            0x000000D1L
#define NGpiSetCp                        0x000000D2L
#define NGpiQueryCp                      0x000000D3L
#define NGpiQueryFullFontFileDescs       0x000000D4L
#define NGpiSetFontMappingFlags          0x000000D5L
#define NGpiQueryFontMappingFlags        0x000000D6L
#define NGpiQueryFontFileDescriptions    0x000000D7L
#define NGpiBitBlt                       0x000000D8L
#define NGpiDeleteBitmap                 0x000000D9L
#define NGpiLoadBitmap                   0x000000DAL
#define NGpiSetBitmap                    0x000000DBL
#define NGpiWCBitBlt                     0x000000DCL
#define NGpiCreateBitmap                 0x000000DDL
#define NGpiSetBitmapBits                0x000000DEL
#define NGpiSetBitmapDimension           0x000000DFL
#define NGpiSetBitmapId                  0x000000E0L
#define NGpiQueryBitmapBits              0x000000E1L
#define NGpiQueryBitmapDimension         0x000000E2L
#define NGpiQueryBitmapHandle            0x000000E3L
#define NGpiQueryBitmapParameters        0x000000E4L
#define NGpiQueryBitmapInfoHeader        0x000000E5L
#define NGpiQueryDeviceBitmapFormats     0x000000E6L
#define NGpiSetPel                       0x000000E7L
#define NGpiQueryPel                     0x000000E8L
#define NGpiFloodFill                    0x000000E9L
#define NGpiDrawBits                     0x000000EAL
#define NGpiCombineRegion                0x000000EBL
#define NGpiCreateRegion                 0x000000ECL
#define NGpiDestroyRegion                0x000000EDL
#define NGpiEqualRegion                  0x000000EEL
#define NGpiOffsetRegion                 0x000000EFL
#define NGpiPaintRegion                  0x000000F0L
#define NGpiFrameRegion                  0x000000F1L
#define NGpiPtInRegion                   0x000000F2L
#define NGpiQueryRegionBox               0x000000F3L
#define NGpiQueryRegionRects             0x000000F4L
#define NGpiRectInRegion                 0x000000F5L
#define NGpiSetRegion                    0x000000F6L
#define NGpiSetClipRegion                0x000000F7L
#define NGpiQueryClipRegion              0x000000F8L
#define NGpiQueryClipBox                 0x000000F9L
#define NGpiExcludeClipRectangle         0x000000FAL
#define NGpiIntersectClipRectangle       0x000000FBL
#define NGpiOffsetClipRegion             0x000000FCL
#define NGpiCreateRoundRectRegion        0x000000FDL
#define NGpiCreateEllipticRegion         0x000000FEL
#define NGpiResumePlay                   0x000000FFL
#define NGpiSuspendPlay                  0x00000100L
#define NGpiCopyMetaFile                 0x00000101L
#define NGpiDeleteMetaFile               0x00000102L
#define NGpiLoadMetaFile                 0x00000103L
#define NGpiPlayMetaFile                 0x00000104L
#define NGpiQueryMetaFileBits            0x00000105L
#define NGpiQueryMetaFileLength          0x00000106L
#define NGpiSaveMetaFile                 0x00000107L
#define NGpiSetMetaFileBits              0x00000108L
#define NGpiQueryDefArcParams            0x00000109L
#define NGpiQueryDefAttrs                0x0000010AL
#define NGpiQueryDefTag                  0x0000010BL
#define NGpiQueryDefViewingLimits        0x0000010CL
#define NGpiSetDefArcParams              0x0000010DL
#define NGpiSetDefAttrs                  0x0000010EL
#define NGpiSetDefTag                    0x0000010FL
#define NGpiSetDefViewingLimits          0x00000110L
#define NGpiPolygons                     0x00000111L
#define NGpiCreatePolygonRegion          0x00000112L
#define NGpiBeginInkPath                 0x00000113L
#define NGpiEndInkPath                   0x00000114L
#define NGpiStrokeInkPath                0x00000115L
/* Additional server-side placeholder entry points from pmgpi.c.
   Signatures are the current server "fix prototype" forms; they are
   intentionally manual stubs on the client until real prototypes exist. */
#define NGpiSetDCOwner                   0x00000116L
#define NGpiSetBitmapOwner               0x00000117L
#define NGpiSetRegionOwner               0x00000118L
#define NGpiInstallIFIFont               0x00000119L
#define NGpiOpenMetafile                 0x0000011AL
#define NGpiCloseMetafile                0x0000011BL
#define NGpiConvPSH                      0x0000011CL
#define NGpiMtAssociate                  0x0000011DL
#define NGpiMtDisassociate               0x0000011EL
#define NGpiEnableYInversion             0x0000011FL
#define NGpiQueryYInversion              0x00000120L
#define NGpiSaveMetaFile2                0x00000121L
#define NGpiAllocateDCData               0x00000122L
#define NGpiQueryDCData                  0x00000123L
#define NDevOpenDC                       0x00000124L

#ifdef __cplusplus
extern "C" {
#endif

/* ======================================================================== */
/*  Wire argument structures (shared by client and server)                  */
/* ======================================================================== */

/*!
 * @struct _GpiCreatePS_Args
 * @brief Wire arguments for NGpiCreatePS.
 */
typedef struct _GpiCreatePS_Args {
    HAB   hab;        /*!< Anchor block.                    */
    HDC   hdc;        /*!< Device context.                  */
    SIZEL psizlSize;  /*!< Presentation space size.         */
    ULONG flOptions;  /*!< PS_UNITS / PS_FORMAT / PS_NORESET. */
} GpiCreatePS_Args, *PGpiCreatePS_Args;

/*!
 * @struct _GpiSetPS_Args
 * @brief Wire arguments for NGpiSetPS.
 */
typedef struct _GpiSetPS_Args {
    SIZEL psizlSize;  /*!< Presentation space size.         */
    ULONG flOptions;  /*!< PS_UNITS / PS_FORMAT / PS_NORESET. */
} GpiSetPS_Args, *PGpiSetPS_Args;

/*!
 * @struct _GpiBox_Args
 * @brief Wire arguments for NGpiBox.
 */
typedef struct _GpiBox_Args {
    LONG   lControl;  /*!< DRO_* control flags.      */
    POINTL ptl;       /*!< Opposite corner.          */
    LONG   lHRound;   /*!< Horizontal corner radius. */
    LONG   lVRound;   /*!< Vertical corner radius.   */
} GpiBox_Args, *PGpiBox_Args;

/*!
 * @struct _GpiErrorSegmentData_Args
 * @brief Wire arguments for NGpiErrorSegmentData.
 */
typedef struct _GpiErrorSegmentData_Args {
    LONG lSegment;   /*!< Error segment identifier. */
    LONG lContext;   /*!< Error context.            */
} GpiErrorSegmentData_Args, *PGpiErrorSegmentData_Args;

/*!
 * @struct _GpiCorrelateChain_Args
 * @brief Wire arguments for NGpiCorrelateChain.
 */
typedef struct _GpiCorrelateChain_Args {
    LONG   lType;      /*!< Correlation type (COR_*).            */
    POINTL ptlPick;    /*!< Pick point.                          */
    LONG   lMaxHits;   /*!< Maximum number of hits.              */
    LONG   lMaxDepth;  /*!< Maximum depth.                       */
    LONG   alHits[1];  /*!< Hit array (variable length).         */
} GpiCorrelateChain_Args, *PGpiCorrelateChain_Args;

/*!
 * @struct _GpiCorrelateFrom_Args
 * @brief Wire arguments for NGpiCorrelateFrom.
 */
typedef struct _GpiCorrelateFrom_Args {
    LONG   lFirstSegment;  /*!< First segment identifier.        */
    LONG   lLastSegment;   /*!< Last segment identifier.         */
    LONG   lType;          /*!< Correlation type (COR_*).        */
    POINTL ptlPick;        /*!< Pick point.                      */
    LONG   lMaxHits;       /*!< Maximum number of hits.          */
    LONG   lMaxDepth;      /*!< Maximum depth.                   */
    LONG   alSegTag[1];    /*!< Segment/tag array (variable).    */
} GpiCorrelateFrom_Args, *PGpiCorrelateFrom_Args;

/*!
 * @struct _GpiCorrelateSegment_Args
 * @brief Wire arguments for NGpiCorrelateSegment.
 */
typedef struct _GpiCorrelateSegment_Args {
    LONG   lSegment;       /*!< Segment identifier.              */
    LONG   lType;          /*!< Correlation type (COR_*).        */
    POINTL ptlPick;        /*!< Pick point.                      */
    LONG   lMaxHits;       /*!< Maximum number of hits.          */
    LONG   lMaxDepth;      /*!< Maximum depth.                   */
    LONG   alSegTag[1];    /*!< Segment/tag array (variable).    */
} GpiCorrelateSegment_Args, *PGpiCorrelateSegment_Args;

/*!
 * @struct _GpiQuerySegmentNames_Args
 * @brief Wire arguments for NGpiQuerySegmentNames.
 */
typedef struct _GpiQuerySegmentNames_Args {
    LONG lFirstSegid;   /*!< First segment in range.          */
    LONG lLastSegid;    /*!< Last segment in range.           */
    LONG lMax;          /*!< Maximum number of names to return. */
    LONG alSegids[1];   /*!< Segment id array (variable).     */
} GpiQuerySegmentNames_Args, *PGpiQuerySegmentNames_Args;

/*!
 * @struct _GpiGetData_Args
 * @brief Wire arguments for NGpiGetData.
 */
typedef struct _GpiGetData_Args {
    LONG lSegid;        /*!< Segment identifier.              */
    LONG lOffset;       /*!< Offset within the segment.       */
    LONG lFormat;       /*!< Data format (DFORM_*).           */
    LONG lLength;       /*!< Requested data length.           */
    BYTE abData[1];     /*!< Data buffer (variable).          */
} GpiGetData_Args, *PGpiGetData_Args;

/*!
 * @struct _GpiPutData_Args
 * @brief Wire arguments for NGpiPutData.
 */
typedef struct _GpiPutData_Args {
    LONG lFormat;       /*!< Data format (DFORM_*).           */
    LONG lCount;        /*!< Number of data bytes.            */
    BYTE abData[1];     /*!< Data buffer (variable).          */
} GpiPutData_Args, *PGpiPutData_Args;

/*!
 * @struct _GpiBeginElement_Args
 * @brief Wire arguments for NGpiBeginElement.
 */
typedef struct _GpiBeginElement_Args {
    LONG lType;         /*!< Element type.                    */
    char szDesc[256];   /*!< Element description (NUL-terminated). */
} GpiBeginElement_Args, *PGpiBeginElement_Args;

/*!
 * @struct _GpiElement_Args
 * @brief Wire arguments for NGpiElement.
 */
typedef struct _GpiElement_Args {
    LONG lType;         /*!< Element type.                    */
    char szDesc[256];   /*!< Element description.             */
    LONG lLength;       /*!< Element data length.             */
    BYTE abData[1];     /*!< Element data (variable).         */
} GpiElement_Args, *PGpiElement_Args;

/*!
 * @struct _GpiQueryElement_Args
 * @brief Wire arguments for NGpiQueryElement.
 */
typedef struct _GpiQueryElement_Args {
    LONG lOff;          /*!< Offset within the element.       */
    LONG lMaxLength;    /*!< Maximum bytes to read.           */
    BYTE abData[1];     /*!< Output buffer (variable).        */
} GpiQueryElement_Args, *PGpiQueryElement_Args;

/*!
 * @struct _GpiQueryElementType_Args
 * @brief Wire arguments for NGpiQueryElementType.
 */
typedef struct _GpiQueryElementType_Args {
    LONG lType;         /*!< Element type (output).           */
    LONG lLength;       /*!< Description buffer length.       */
    char szData[256];   /*!< Description data (NUL-terminated). */
} GpiQueryElementType_Args, *PGpiQueryElementType_Args;

/*!
 * @struct _GpiQuerySegmentTransformMatrix_Args
 * @brief Wire arguments for NGpiQuerySegmentTransformMatrix.
 */
typedef struct _GpiQuerySegmentTransformMatrix_Args {
    LONG lSegid;         /*!< Segment identifier.              */
    LONG lCount;         /*!< Number of matrix elements.       */
    MATRIXLF aMatlf[1];  /*!< Matrix array (variable).         */
} GpiQuerySegmentTransformMatrix_Args, *PGpiQuerySegmentTransformMatrix_Args;

/*!
 * @struct _GpiSetSegmentTransformMatrix_Args
 * @brief Wire arguments for NGpiSetSegmentTransformMatrix.
 */
typedef struct _GpiSetSegmentTransformMatrix_Args {
    LONG lSegid;         /*!< Segment identifier.              */
    LONG lCount;         /*!< Number of matrix elements.       */
    LONG lOptions;       /*!< Transform options.               */
    MATRIXLF aMatlf[1];  /*!< Matrix array (variable).         */
} GpiSetSegmentTransformMatrix_Args, *PGpiSetSegmentTransformMatrix_Args;

/*!
 * @struct _GpiConvert_Args
 * @brief Wire arguments for NGpiConvert.
 */
typedef struct _GpiConvert_Args {
    LONG lSrc;           /*!< Source coordinate space.         */
    LONG lTarg;          /*!< Target coordinate space.         */
    LONG lCount;         /*!< Number of points.                */
    POINTL aptl[1];      /*!< Point array (variable).          */
} GpiConvert_Args, *PGpiConvert_Args;

/*!
 * @struct _GpiConvertWithMatrix_Args
 * @brief Wire arguments for NGpiConvertWithMatrix.
 */
typedef struct _GpiConvertWithMatrix_Args {
    LONG lCountp;        /*!< Number of points.                */
    LONG lCount;         /*!< Number of matrix elements.       */
    MATRIXLF matlf;      /*!< Transformation matrix.           */
    POINTL aptl[1];      /*!< Point array (variable).          */
} GpiConvertWithMatrix_Args, *PGpiConvertWithMatrix_Args;

/*!
 * @struct _GpiQueryModelTransformMatrix_Args
 * @brief Wire arguments for NGpiQueryModelTransformMatrix.
 */
typedef struct _GpiQueryModelTransformMatrix_Args {
    LONG lCount;         /*!< Number of matrix elements.       */
    MATRIXLF aMatlf[1];  /*!< Matrix array (variable).         */
} GpiQueryModelTransformMatrix_Args, *PGpiQueryModelTransformMatrix_Args;

/*!
 * @struct _GpiSetModelTransformMatrix_Args
 * @brief Wire arguments for NGpiSetModelTransformMatrix.
 */
typedef struct _GpiSetModelTransformMatrix_Args {
    LONG lCount;         /*!< Number of matrix elements.       */
    LONG lOptions;       /*!< Transform options.               */
    MATRIXLF aMatlf[1];  /*!< Matrix array (variable).         */
} GpiSetModelTransformMatrix_Args, *PGpiSetModelTransformMatrix_Args;

/*!
 * @struct _GpiCallSegmentMatrix_Args
 * @brief Wire arguments for NGpiCallSegmentMatrix.
 */
typedef struct _GpiCallSegmentMatrix_Args {
    LONG lSegment;       /*!< Segment identifier.              */
    LONG lCount;         /*!< Number of matrix elements.       */
    LONG lOptions;       /*!< Transform options.               */
    MATRIXLF aMatlf[1];  /*!< Matrix array (variable).         */
} GpiCallSegmentMatrix_Args, *PGpiCallSegmentMatrix_Args;

/*!
 * @struct _GpiQueryDefaultViewMatrix_Args
 * @brief Wire arguments for NGpiQueryDefaultViewMatrix.
 */
typedef struct _GpiQueryDefaultViewMatrix_Args {
    LONG lCount;         /*!< Number of matrix elements.       */
    MATRIXLF aMatlf[1];  /*!< Matrix array (variable).         */
} GpiQueryDefaultViewMatrix_Args, *PGpiQueryDefaultViewMatrix_Args;

/*!
 * @struct _GpiSetDefaultViewMatrix_Args
 * @brief Wire arguments for NGpiSetDefaultViewMatrix.
 */
typedef struct _GpiSetDefaultViewMatrix_Args {
    LONG lCount;         /*!< Number of matrix elements.       */
    LONG lOptions;       /*!< Transform options.               */
    MATRIXLF aMatlf[1];  /*!< Matrix array (variable).         */
} GpiSetDefaultViewMatrix_Args, *PGpiSetDefaultViewMatrix_Args;

/*!
 * @struct _GpiQueryViewingTransformMatrix_Args
 * @brief Wire arguments for NGpiQueryViewingTransformMatrix.
 */
typedef struct _GpiQueryViewingTransformMatrix_Args {
    LONG lCount;         /*!< Number of matrix elements.       */
    MATRIXLF aMatlf[1];  /*!< Matrix array (variable).         */
} GpiQueryViewingTransformMatrix_Args, *PGpiQueryViewingTransformMatrix_Args;

/*!
 * @struct _GpiSetViewingTransformMatrix_Args
 * @brief Wire arguments for NGpiSetViewingTransformMatrix.
 */
typedef struct _GpiSetViewingTransformMatrix_Args {
    LONG lCount;         /*!< Number of matrix elements.       */
    LONG lOptions;       /*!< Transform options.               */
    MATRIXLF aMatlf[1];  /*!< Matrix array (variable).         */
} GpiSetViewingTransformMatrix_Args, *PGpiSetViewingTransformMatrix_Args;

/*!
 * @struct _GpiTranslate_Args
 * @brief Wire arguments for NGpiTranslate.
 */
typedef struct _GpiTranslate_Args {
    MATRIXLF matlf;      /*!< Input/output matrix.             */
    LONG lOptions;       /*!< Transform options.               */
    POINTL ptl;          /*!< Translation vector.              */
} GpiTranslate_Args, *PGpiTranslate_Args;

/*!
 * @struct _GpiScale_Args
 * @brief Wire arguments for NGpiScale.
 */
typedef struct _GpiScale_Args {
    MATRIXLF matlf;      /*!< Input/output matrix.             */
    LONG lOptions;       /*!< Transform options.               */
    FIXED afx[2];        /*!< Scaling factors (x, y).          */
    POINTL ptl;          /*!< Scaling origin.                  */
} GpiScale_Args, *PGpiScale_Args;

/*!
 * @struct _GpiRotate_Args
 * @brief Wire arguments for NGpiRotate.
 */
typedef struct _GpiRotate_Args {
    MATRIXLF matlf;      /*!< Input/output matrix.             */
    LONG lOptions;       /*!< Transform options.               */
    FIXED fxAngle;       /*!< Rotation angle (FIXED).          */
    POINTL ptl;          /*!< Rotation center.                 */
} GpiRotate_Args, *PGpiRotate_Args;

/*!
 * @struct _GpiCreateLogColorTable_Args
 * @brief Wire arguments for NGpiCreateLogColorTable.
 */
typedef struct _GpiCreateLogColorTable_Args {
    ULONG flOptions;     /*!< LCOL_* options.                  */
    LONG  lFormat;       /*!< Color table format (LCOLF_*).    */
    LONG  lStart;        /*!< First color index.               */
    LONG  lCount;        /*!< Number of entries.               */
    LONG  alTable[1];    /*!< Color table array (variable).    */
} GpiCreateLogColorTable_Args, *PGpiCreateLogColorTable_Args;

/*!
 * @struct _GpiQueryColorData_Args
 * @brief Wire arguments for NGpiQueryColorData.
 */
typedef struct _GpiQueryColorData_Args {
    LONG lCount;         /*!< Number of requested values.      */
    LONG aData[1];       /*!< Output data array (variable).    */
} GpiQueryColorData_Args, *PGpiQueryColorData_Args;

/*!
 * @struct _GpiQueryLogColorTable_Args
 * @brief Wire arguments for NGpiQueryLogColorTable.
 */
typedef struct _GpiQueryLogColorTable_Args {
    ULONG flOptions;     /*!< LCOL_* options.                  */
    LONG  lStart;        /*!< First color index.               */
    LONG  lCount;        /*!< Number of entries.               */
    LONG  aData[1];      /*!< Output color array (variable).   */
} GpiQueryLogColorTable_Args, *PGpiQueryLogColorTable_Args;

/*!
 * @struct _GpiQueryRealColors_Args
 * @brief Wire arguments for NGpiQueryRealColors.
 */
typedef struct _GpiQueryRealColors_Args {
    ULONG flOptions;     /*!< LCOL_* options.                  */
    LONG  lStart;        /*!< First color index.               */
    LONG  lCount;        /*!< Number of entries.               */
    LONG  aColors[1];    /*!< Output RGB array (variable).     */
} GpiQueryRealColors_Args, *PGpiQueryRealColors_Args;

/*!
 * @struct _GpiCreatePalette_Args
 * @brief Wire arguments for NGpiCreatePalette.
 */
typedef struct _GpiCreatePalette_Args {
    ULONG flOptions;     /*!< Palette creation options.        */
    ULONG ulFormat;      /*!< Entry format (LCOLF_*).          */
    ULONG ulCount;       /*!< Number of entries.               */
    ULONG aTable[1];     /*!< Entry table (variable).          */
} GpiCreatePalette_Args, *PGpiCreatePalette_Args;

/*!
 * @struct _GpiAnimatePalette_Args
 * @brief Wire arguments for NGpiAnimatePalette.
 */
typedef struct _GpiAnimatePalette_Args {
    ULONG ulFormat;      /*!< Entry format (LCOLF_*).          */
    ULONG ulStart;       /*!< First entry.                     */
    ULONG ulCount;       /*!< Number of entries.               */
    ULONG aTable[1];     /*!< Entry table (variable).          */
} GpiAnimatePalette_Args, *PGpiAnimatePalette_Args;

/*!
 * @struct _GpiSetPaletteEntries_Args
 * @brief Wire arguments for NGpiSetPaletteEntries.
 */
typedef struct _GpiSetPaletteEntries_Args {
    ULONG ulFormat;      /*!< Entry format (LCOLF_*).          */
    ULONG ulStart;       /*!< First entry.                     */
    ULONG ulCount;       /*!< Number of entries.               */
    ULONG aTable[1];     /*!< Entry table (variable).          */
} GpiSetPaletteEntries_Args, *PGpiSetPaletteEntries_Args;

/*!
 * @struct _GpiQueryPaletteInfo_Args
 * @brief Wire arguments for NGpiQueryPaletteInfo.
 */
typedef struct _GpiQueryPaletteInfo_Args {
    ULONG flOptions;     /*!< Query options.                   */
    ULONG ulStart;       /*!< First entry.                     */
    ULONG ulCount;       /*!< Number of entries.               */
    ULONG aData[1];      /*!< Output array (variable).         */
} GpiQueryPaletteInfo_Args, *PGpiQueryPaletteInfo_Args;

/*!
 * @struct _GpiPolyLine_Args
 * @brief Wire arguments for NGpiPolyLine (also used by PolyMarker, PolySpline,
 *        PolyFillet, PolyLineDisjoint, StrokeInkPath).
 */
typedef struct _GpiPolyLine_Args {
    LONG   lCount;       /*!< Number of points.                */
    POINTL aptl[1];      /*!< Point array (variable).          */
} GpiPolyLine_Args, *PGpiPolyLine_Args;

/*!
 * @struct _GpiCharString_Args
 * @brief Wire arguments for NGpiCharString.
 */
typedef struct _GpiCharString_Args {
    LONG lCount;         /*!< Number of characters.            */
    char szString[1];    /*!< Character data (variable).       */
} GpiCharString_Args, *PGpiCharString_Args;

/*!
 * @struct _GpiCharStringAt_Args
 * @brief Wire arguments for NGpiCharStringAt.
 */
typedef struct _GpiCharStringAt_Args {
    POINTL ptl;          /*!< Starting position.               */
    LONG lCount;         /*!< Number of characters.            */
    char szString[1];    /*!< Character data (variable).       */
} GpiCharStringAt_Args, *PGpiCharStringAt_Args;

/*!
 * @struct _GpiSetAttrs_Args
 * @brief Wire arguments for NGpiSetAttrs.
 */
typedef struct _GpiSetAttrs_Args {
    LONG  lPrimType;     /*!< Primitive type (PRIM_*).         */
    ULONG flAttrMask;    /*!< Attribute mask (LBB_/CBB_/...).  */
    ULONG flDefMask;     /*!< Default mask.                    */
    BYTE  abBundle[1];   /*!< Attribute bundle (variable).     */
} GpiSetAttrs_Args, *PGpiSetAttrs_Args;

/*!
 * @struct _GpiQueryAttrs_Args
 * @brief Wire arguments for NGpiQueryAttrs.
 */
typedef struct _GpiQueryAttrs_Args {
    LONG  lPrimType;     /*!< Primitive type (PRIM_*).         */
    ULONG flAttrMask;    /*!< Attribute mask.                  */
    BYTE  abBundle[1];   /*!< Output bundle (variable).        */
} GpiQueryAttrs_Args, *PGpiQueryAttrs_Args;

/*!
 * @struct _GpiOval_Args
 * @brief Wire arguments for NGpiOval.
 */
typedef struct _GpiOval_Args {
    POINTL aptl[2];      /*!< Center and radii (x, y).         */
    FIXED fxStart;       /*!< Start angle.                     */
    FIXED fxSweep;       /*!< Sweep angle.                     */
    ULONG ulOptions;     /*!< Drawing options.                 */
} GpiOval_Args, *PGpiOval_Args;

/*!
 * @struct _GpiPolyFilletSharp_Args
 * @brief Wire arguments for NGpiPolyFilletSharp.
 */
typedef struct _GpiPolyFilletSharp_Args {
    LONG   lCount;       /*!< Number of points.                */
    POINTL aptl[1];      /*!< Point array (variable).          */
    FIXED  afx[1];       /*!< Sharpness array (variable).      */
} GpiPolyFilletSharp_Args, *PGpiPolyFilletSharp_Args;

/*!
 * @struct _GpiQueryCharStringPos_Args
 * @brief Wire arguments for NGpiQueryCharStringPos.
 */
typedef struct _GpiQueryCharStringPos_Args {
    ULONG flOptions;     /*!< Options (CHS_*).                 */
    LONG lCount;         /*!< Number of characters.            */
    char szString[1];    /*!< Character data (variable).       */
    LONG aXinc[1];       /*!< X-increment array (variable).    */
    POINTL aptl[1];      /*!< Position array (variable).       */
} GpiQueryCharStringPos_Args, *PGpiQueryCharStringPos_Args;

/*!
 * @struct _GpiQueryCharStringPosAt_Args
 * @brief Wire arguments for NGpiQueryCharStringPosAt.
 */
typedef struct _GpiQueryCharStringPosAt_Args {
    POINTL ptlStart;     /*!< Starting position.               */
    ULONG flOptions;     /*!< Options (CHS_*).                 */
    LONG lCount;         /*!< Number of characters.            */
    char szString[1];    /*!< Character data (variable).       */
    LONG aXinc[1];       /*!< X-increment array (variable).    */
    POINTL aptl[1];      /*!< Position array (variable).       */
} GpiQueryCharStringPosAt_Args, *PGpiQueryCharStringPosAt_Args;

/*!
 * @struct _GpiQueryTabbedTextExtent_Args
 * @brief Wire arguments for NGpiQueryTabbedTextExtent.
 */
typedef struct _GpiQueryTabbedTextExtent_Args {
    LONG lCount;         /*!< Number of characters.            */
    LONG lTabCount;      /*!< Number of tab stops.             */
    char szString[1];    /*!< Character data (variable).       */
    ULONG aTabStops[1];  /*!< Tab stop array (variable).       */
} GpiQueryTabbedTextExtent_Args, *PGpiQueryTabbedTextExtent_Args;

/*!
 * @struct _GpiQueryTextBox_Args
 * @brief Wire arguments for NGpiQueryTextBox.
 */
typedef struct _GpiQueryTextBox_Args {
    LONG lCount1;        /*!< Number of characters.            */
    LONG lCount2;        /*!< Number of output points.         */
    char szString[1];    /*!< Character data (variable).       */
    POINTL aptl[1];      /*!< Output point array (variable).   */
} GpiQueryTextBox_Args, *PGpiQueryTextBox_Args;

/*!
 * @struct _GpiQueryTextAlignment_Args
 * @brief Wire arguments for NGpiQueryTextAlignment.
 */
typedef struct _GpiQueryTextAlignment_Args {
    LONG lHoriz;         /*!< Horizontal alignment (output).   */
    LONG lVert;          /*!< Vertical alignment (output).     */
} GpiQueryTextAlignment_Args, *PGpiQueryTextAlignment_Args;

/*!
 * @struct _GpiCharStringPos_Args
 * @brief Wire arguments for NGpiCharStringPos.
 */
typedef struct _GpiCharStringPos_Args {
    RECTL prclRect;      /*!< Character box.                   */
    ULONG flOptions;     /*!< Options (CHS_*).                 */
    LONG lCount;         /*!< Number of characters.            */
    char szString[1];    /*!< Character data (variable).       */
    LONG aAdx[1];        /*!< Advance-width array (variable).  */
} GpiCharStringPos_Args, *PGpiCharStringPos_Args;

/*!
 * @struct _GpiTabbedCharStringAt_Args
 * @brief Wire arguments for NGpiTabbedCharStringAt.
 */
typedef struct _GpiTabbedCharStringAt_Args {
    POINTL ptStart;      /*!< Starting position.               */
    RECTL prclRect;      /*!< Character box.                   */
    ULONG flOptions;     /*!< Options (CHS_*).                 */
    LONG lCount;         /*!< Number of characters.            */
    LONG lTabCount;      /*!< Number of tab stops.             */
    LONG lTabOrigin;     /*!< Tab origin.                      */
    char szString[1];    /*!< Character data (variable).       */
    ULONG aTabStops[1];  /*!< Tab stop array (variable).       */
} GpiTabbedCharStringAt_Args, *PGpiTabbedCharStringAt_Args;

/*!
 * @struct _GpiCharStringPosAt_Args
 * @brief Wire arguments for NGpiCharStringPosAt.
 */
typedef struct _GpiCharStringPosAt_Args {
    POINTL ptlStart;     /*!< Starting position.               */
    RECTL prclRect;      /*!< Character box.                   */
    ULONG flOptions;     /*!< Options (CHS_*).                 */
    LONG lCount;         /*!< Number of characters.            */
    char szString[1];    /*!< Character data (variable).       */
    LONG aAdx[1];        /*!< Advance-width array (variable).  */
} GpiCharStringPosAt_Args, *PGpiCharStringPosAt_Args;

/*!
 * @struct _GpiImage_Args
 * @brief Wire arguments for NGpiImage.
 */
typedef struct _GpiImage_Args {
    LONG lFormat;        /*!< Image format.                    */
    SIZEL size;          /*!< Image size.                      */
    LONG lLength;        /*!< Image data length.               */
    BYTE abData[1];      /*!< Image data (variable).           */
} GpiImage_Args, *PGpiImage_Args;

/*!
 * @struct _GpiComment_Args
 * @brief Wire arguments for NGpiComment.
 */
typedef struct _GpiComment_Args {
    LONG lLength;        /*!< Comment data length.             */
    BYTE abData[1];      /*!< Comment data (variable).         */
} GpiComment_Args, *PGpiComment_Args;

/*!
 * @struct _GpiCreateLogFont_Args
 * @brief Wire arguments for NGpiCreateLogFont.
 */
typedef struct _GpiCreateLogFont_Args {
    LONG lLcid;          /*!< Logical font id.                 */
    char szName[256];    /*!< Font name (NUL-terminated).      */
    FATTRS attrs;        /*!< Font attributes.                 */
} GpiCreateLogFont_Args, *PGpiCreateLogFont_Args;

/*!
 * @struct _GpiLoadFonts_Args
 * @brief Wire arguments for NGpiLoadFonts / NGpiUnloadFonts /
 *        NGpiLoadPublicFonts / NGpiUnloadPublicFonts.
 */
typedef struct _GpiLoadFonts_Args {
    char szFilename[256]; /*!< Font file name (NUL-terminated). */
} GpiLoadFonts_Args, *PGpiLoadFonts_Args;

/*!
 * @struct _GpiQueryFonts_Args
 * @brief Wire arguments for NGpiQueryFonts.
 */
typedef struct _GpiQueryFonts_Args {
    ULONG flOptions;     /*!< Query options.                   */
    LONG lReqFonts;      /*!< Number of requested fonts.       */
    LONG lMetricsLength; /*!< Size of one metrics record.      */
    char szFacename[256];/*!< Face name (NUL-terminated).      */
    BYTE abMetrics[1];   /*!< Output metrics (variable).       */
} GpiQueryFonts_Args, *PGpiQueryFonts_Args;

/*!
 * @struct _GpiQueryFontMetrics_Args
 * @brief Wire arguments for NGpiQueryFontMetrics.
 */
typedef struct _GpiQueryFontMetrics_Args {
    LONG lMetricsLength; /*!< Size of one metrics record.      */
    BYTE abMetrics[1];   /*!< Output metrics (variable).       */
} GpiQueryFontMetrics_Args, *PGpiQueryFontMetrics_Args;

/*!
 * @struct _GpiQueryKerningPairs_Args
 * @brief Wire arguments for NGpiQueryKerningPairs.
 */
typedef struct _GpiQueryKerningPairs_Args {
    LONG lCount;         /*!< Number of kerning pairs.         */
    KERNINGPAIRS aData[1]; /*!< Output array (variable).       */
} GpiQueryKerningPairs_Args, *PGpiQueryKerningPairs_Args;

/*!
 * @struct _GpiQueryWidthTable_Args
 * @brief Wire arguments for NGpiQueryWidthTable.
 */
typedef struct _GpiQueryWidthTable_Args {
    LONG lFirstChar;     /*!< First character code.            */
    LONG lCount;         /*!< Number of characters.            */
    LONG aData[1];       /*!< Output width array (variable).   */
} GpiQueryWidthTable_Args, *PGpiQueryWidthTable_Args;

/*!
 * @struct _GpiQuerySetIds_Args
 * @brief Wire arguments for NGpiQuerySetIds.
 */
typedef struct _GpiQuerySetIds_Args {
    LONG lCount;         /*!< Number of entries.               */
    BYTE abData[1];      /*!< Interleaved types/lcids/names (variable). */
} GpiQuerySetIds_Args, *PGpiQuerySetIds_Args;

/*!
 * @struct _GpiQueryFaceString_Args
 * @brief Wire arguments for NGpiQueryFaceString.
 */
typedef struct _GpiQueryFaceString_Args {
    FACENAMEDESC desc;   /*!< Face name descriptor.            */
    LONG length;         /*!< Output buffer length.            */
    char szFamily[256];  /*!< Family name (NUL-terminated).    */
    char szCompound[256];/*!< Compound face name (NUL-terminated). */
} GpiQueryFaceString_Args, *PGpiQueryFaceString_Args;

/*!
 * @struct _GpiQueryLogicalFont_Args
 * @brief Wire arguments for NGpiQueryLogicalFont.
 */
typedef struct _GpiQueryLogicalFont_Args {
    LONG lcid;           /*!< Logical font id.                 */
    LONG length;         /*!< Output buffer length.            */
    char szName[256];    /*!< Font name (NUL-terminated).      */
    FATTRS attrs;        /*!< Output font attributes.          */
} GpiQueryLogicalFont_Args, *PGpiQueryLogicalFont_Args;

/*!
 * @struct _GpiQueryFullFontFileDescs_Args
 * @brief Wire arguments for NGpiQueryFullFontFileDescs.
 */
typedef struct _GpiQueryFullFontFileDescs_Args {
    LONG lCount;         /*!< Number of requested descriptors. */
    LONG lNamesBuffLength; /*!< Names buffer length.           */
    char szFilename[256];/*!< Font file name (NUL-terminated). */
    BYTE abNames[1];     /*!< Output names buffer (variable).  */
} GpiQueryFullFontFileDescs_Args, *PGpiQueryFullFontFileDescs_Args;

/*!
 * @struct _GpiQueryFontFileDescriptions_Args
 * @brief Wire arguments for NGpiQueryFontFileDescriptions.
 */
typedef struct _GpiQueryFontFileDescriptions_Args {
    LONG lCount;         /*!< Number of requested descriptions. */
    char szFilename[256];/*!< Font file name (NUL-terminated). */
    BYTE abDescs[1];     /*!< Output descriptions (variable).  */
} GpiQueryFontFileDescriptions_Args, *PGpiQueryFontFileDescriptions_Args;

/*!
 * @struct _GpiBitBlt_Args
 * @brief Wire arguments for NGpiBitBlt.
 */
typedef struct _GpiBitBlt_Args {
    HPS    hpsSource;    /*!< Source PS.                       */
    LONG   lCount;       /*!< Number of points.                */
    LONG   lRop;         /*!< Raster operation code.           */
    ULONG  flOptions;    /*!< BitBlt options.                  */
    POINTL aptl[1];      /*!< Point array (variable).          */
} GpiBitBlt_Args, *PGpiBitBlt_Args;

/*!
 * @struct _GpiWCBitBlt_Args
 * @brief Wire arguments for NGpiWCBitBlt.
 */
typedef struct _GpiWCBitBlt_Args {
    HBITMAP hbmSource;   /*!< Source bitmap.                   */
    LONG    lCount;      /*!< Number of points.                */
    LONG    lRop;        /*!< Raster operation code.           */
    ULONG   flOptions;   /*!< BitBlt options.                  */
    POINTL  aptl[1];     /*!< Point array (variable).          */
} GpiWCBitBlt_Args, *PGpiWCBitBlt_Args;

/*!
 * @struct _GpiCreateBitmap_Args
 * @brief Wire arguments for NGpiCreateBitmap.
 */
typedef struct _GpiCreateBitmap_Args {
    BITMAPINFOHEADER2 hdr; /*!< Bitmap header.                 */
    ULONG             flOptions;   /*!< Creation options.     */
    ULONG             cbInitData;  /*!< Size of init data.    */
    ULONG             cbInfo;      /*!< Size of info table.   */
    BYTE              abData[1];   /*!< Init data + info (variable). */
} GpiCreateBitmap_Args, *PGpiCreateBitmap_Args;

/*!
 * @struct _GpiSetBitmapBits_Args
 * @brief Wire arguments for NGpiSetBitmapBits.
 */
typedef struct _GpiSetBitmapBits_Args {
    LONG  lScanStart;    /*!< First scan line.                 */
    LONG  lScans;        /*!< Number of scan lines.            */
    ULONG cbBuffer;      /*!< Size of the bits buffer.         */
    ULONG cbInfo;        /*!< Size of the info table.          */
    BYTE  abData[1];     /*!< Bits + info (variable).          */
} GpiSetBitmapBits_Args, *PGpiSetBitmapBits_Args;

/*!
 * @struct _GpiSetBitmapDimension_Args
 * @brief Wire arguments for NGpiSetBitmapDimension.
 */
typedef struct _GpiSetBitmapDimension_Args {
    HBITMAP hbm;         /*!< Target bitmap.                   */
    SIZEL   size;        /*!< New dimensions.                  */
} GpiSetBitmapDimension_Args, *PGpiSetBitmapDimension_Args;

/*!
 * @struct _GpiQueryBitmapBits_Args
 * @brief Wire arguments for NGpiQueryBitmapBits.
 */
typedef struct _GpiQueryBitmapBits_Args {
    LONG  lScanStart;    /*!< First scan line.                 */
    LONG  lScans;        /*!< Number of scan lines.            */
    ULONG cbBuffer;      /*!< Size of the output buffer.       */
    ULONG cbInfo;        /*!< Size of the info table.          */
    BYTE  abData[1];     /*!< Output bits + info (variable).   */
} GpiQueryBitmapBits_Args, *PGpiQueryBitmapBits_Args;

/*!
 * @struct _GpiQueryBitmapDimension_Args
 * @brief Wire arguments for NGpiQueryBitmapDimension.
 */
typedef struct _GpiQueryBitmapDimension_Args {
    HBITMAP hbm;         /*!< Target bitmap.                   */
    SIZEL   size;        /*!< Output dimensions.               */
} GpiQueryBitmapDimension_Args, *PGpiQueryBitmapDimension_Args;

/*!
 * @struct _GpiQueryBitmapParameters_Args
 * @brief Wire arguments for NGpiQueryBitmapParameters.
 */
typedef struct _GpiQueryBitmapParameters_Args {
    HBITMAP hbm;         /*!< Target bitmap.                   */
    BITMAPINFOHEADER hdr;/*!< Output header.                   */
} GpiQueryBitmapParameters_Args, *PGpiQueryBitmapParameters_Args;

/*!
 * @struct _GpiQueryBitmapInfoHeader_Args
 * @brief Wire arguments for NGpiQueryBitmapInfoHeader.
 */
typedef struct _GpiQueryBitmapInfoHeader_Args {
    HBITMAP hbm;         /*!< Target bitmap.                   */
    BITMAPINFOHEADER2 hdr; /*!< Output header.                 */
} GpiQueryBitmapInfoHeader_Args, *PGpiQueryBitmapInfoHeader_Args;

/*!
 * @struct _GpiQueryDeviceBitmapFormats_Args
 * @brief Wire arguments for NGpiQueryDeviceBitmapFormats.
 */
typedef struct _GpiQueryDeviceBitmapFormats_Args {
    LONG lCount;         /*!< Number of entries.               */
    LONG aData[1];       /*!< Output format array (variable).  */
} GpiQueryDeviceBitmapFormats_Args, *PGpiQueryDeviceBitmapFormats_Args;

/*!
 * @struct _GpiDrawBits_Args
 * @brief Wire arguments for NGpiDrawBits.
 */
typedef struct _GpiDrawBits_Args {
    LONG   lCount;       /*!< Number of points.                */
    LONG   lRop;         /*!< Raster operation code.           */
    ULONG  flOptions;    /*!< Drawing options.                 */
    ULONG  cbInfo;       /*!< Size of the info table.          */
    ULONG  cbBits;       /*!< Size of the bits buffer.         */
    POINTL aptl[1];      /*!< Point array (variable).          */
} GpiDrawBits_Args, *PGpiDrawBits_Args;

/*!
 * @struct _GpiCreateRegion_Args
 * @brief Wire arguments for NGpiCreateRegion.
 */
typedef struct _GpiCreateRegion_Args {
    LONG  lCount;        /*!< Number of rectangles.            */
    RECTL arcl[1];       /*!< Rectangle array (variable).      */
} GpiCreateRegion_Args, *PGpiCreateRegion_Args;

/*!
 * @struct _GpiQueryRegionBox_Args
 * @brief Wire arguments for NGpiQueryRegionBox.
 */
typedef struct _GpiQueryRegionBox_Args {
    HRGN  hrgn;          /*!< Region handle.                   */
    RECTL rclBound;      /*!< Output bounding box.             */
} GpiQueryRegionBox_Args, *PGpiQueryRegionBox_Args;

/*!
 * @struct _GpiQueryRegionRects_Args
 * @brief Wire arguments for NGpiQueryRegionRects.
 */
typedef struct _GpiQueryRegionRects_Args {
    HRGN    hrgn;        /*!< Region handle.                   */
    RECTL   rclBound;    /*!< Bounding box.                    */
    RGNRECT rgnrcControl;/*!< Query control block.             */
    RECTL   arclRect[1]; /*!< Output rectangle array (variable). */
} GpiQueryRegionRects_Args, *PGpiQueryRegionRects_Args;

/*!
 * @struct _GpiSetRegion_Args
 * @brief Wire arguments for NGpiSetRegion.
 */
typedef struct _GpiSetRegion_Args {
    HRGN  hrgn;          /*!< Region handle.                   */
    LONG  lCount;        /*!< Number of rectangles.            */
    RECTL arcl[1];       /*!< Rectangle array (variable).      */
} GpiSetRegion_Args, *PGpiSetRegion_Args;

/*!
 * @struct _GpiSetClipRegion_Args
 * @brief Wire arguments for NGpiSetClipRegion.
 */
typedef struct _GpiSetClipRegion_Args {
    HRGN hrgn;           /*!< New clip region.                 */
    HRGN hrgnOld;        /*!< Previous clip region (output).   */
} GpiSetClipRegion_Args, *PGpiSetClipRegion_Args;

/*!
 * @struct _GpiLoadMetaFile_Args
 * @brief Wire arguments for NGpiLoadMetaFile.
 */
typedef struct _GpiLoadMetaFile_Args {
    char szFilename[256];/*!< Metafile name (NUL-terminated).  */
} GpiLoadMetaFile_Args, *PGpiLoadMetaFile_Args;

/*!
 * @struct _GpiPlayMetaFile_Args
 * @brief Wire arguments for NGpiPlayMetaFile.
 */
typedef struct _GpiPlayMetaFile_Args {
    HMF  hmf;            /*!< Metafile handle.                 */
    LONG lCount1;        /*!< Number of option entries.        */
    LONG lCount2;        /*!< Description length.              */
    LONG lSegCount;      /*!< Segment count (output).          */
    LONG aOptarray[1];   /*!< Option array (variable).         */
    char szDesc[256];    /*!< Description (NUL-terminated).    */
} GpiPlayMetaFile_Args, *PGpiPlayMetaFile_Args;

/*!
 * @struct _GpiQueryMetaFileBits_Args
 * @brief Wire arguments for NGpiQueryMetaFileBits.
 */
typedef struct _GpiQueryMetaFileBits_Args {
    HMF  hmf;            /*!< Metafile handle.                 */
    LONG lOffset;        /*!< Byte offset.                     */
    LONG lLength;        /*!< Number of bytes.                 */
    BYTE abData[1];      /*!< Output data (variable).          */
} GpiQueryMetaFileBits_Args, *PGpiQueryMetaFileBits_Args;

/*!
 * @struct _GpiSaveMetaFile_Args
 * @brief Wire arguments for NGpiSaveMetaFile.
 */
typedef struct _GpiSaveMetaFile_Args {
    HMF  hmf;            /*!< Metafile handle.                 */
    char szFilename[256];/*!< Target file name (NUL-terminated). */
} GpiSaveMetaFile_Args, *PGpiSaveMetaFile_Args;

/*!
 * @struct _GpiSetMetaFileBits_Args
 * @brief Wire arguments for NGpiSetMetaFileBits.
 */
typedef struct _GpiSetMetaFileBits_Args {
    HMF  hmf;            /*!< Metafile handle.                 */
    LONG lOffset;        /*!< Byte offset.                     */
    LONG lLength;        /*!< Number of bytes.                 */
    BYTE abData[1];      /*!< Data buffer (variable).          */
} GpiSetMetaFileBits_Args, *PGpiSetMetaFileBits_Args;

/*!
 * @struct _GpiQueryDefAttrs_Args
 * @brief Wire arguments for NGpiQueryDefAttrs.
 */
typedef struct _GpiQueryDefAttrs_Args {
    LONG  lPrimType;     /*!< Primitive type (PRIM_*).         */
    ULONG flAttrMask;    /*!< Attribute mask.                  */
    BYTE  abBundle[1];   /*!< Output bundle (variable).        */
} GpiQueryDefAttrs_Args, *PGpiQueryDefAttrs_Args;

/*!
 * @struct _GpiSetDefAttrs_Args
 * @brief Wire arguments for NGpiSetDefAttrs.
 */
typedef struct _GpiSetDefAttrs_Args {
    LONG  lPrimType;     /*!< Primitive type (PRIM_*).         */
    ULONG flAttrMask;    /*!< Attribute mask.                  */
    BYTE  abBundle[1];   /*!< Input bundle (variable).         */
} GpiSetDefAttrs_Args, *PGpiSetDefAttrs_Args;

/*!
 * @struct _GpiPolygons_Args
 * @brief Wire arguments for NGpiPolygons.
 */
typedef struct _GpiPolygons_Args {
    ULONG ulCount;       /*!< Number of polygons.              */
    ULONG flOptions;     /*!< Drawing options.                 */
    ULONG flModel;       /*!< Fill model (POLYGON_*).          */
    ULONG ulTotalPoints; /*!< Total number of points.          */
    BYTE  abData[1];     /*!< Interleaved (LONG n; POINTL[n]) (variable). */
} GpiPolygons_Args, *PGpiPolygons_Args;

/*!
 * @struct _GpiCreatePolygonRegion_Args
 * @brief Wire arguments for NGpiCreatePolygonRegion.
 */
typedef struct _GpiCreatePolygonRegion_Args {
    ULONG ulCount;       /*!< Number of polygons.              */
    ULONG flOptions;     /*!< Region options.                  */
    ULONG ulTotalPoints; /*!< Total number of points.          */
    BYTE  abData[1];     /*!< Interleaved polygon data (variable). */
} GpiCreatePolygonRegion_Args, *PGpiCreatePolygonRegion_Args;

/*!
 * @struct _GpiStrokeInkPath_Args
 * @brief Wire arguments for NGpiStrokeInkPath.
 */
typedef struct _GpiStrokeInkPath_Args {
    LONG   lPath;        /*!< Ink path id.                     */
    LONG   lCount;       /*!< Number of points.                */
    ULONG  flOptions;    /*!< Stroke options.                  */
    POINTL aptl[1];      /*!< Point array (variable).          */
} GpiStrokeInkPath_Args, *PGpiStrokeInkPath_Args;

/*!
 * @struct _GpiDevOpenDC_Args
 * @brief Wire arguments for NDevOpenDC.
 *
 * The fixed part is followed by variable-length NUL-terminated strings:
 * pszToken, then lCount strings from the pdop array.
 */
typedef struct _GpiDevOpenDC_Args {
    HAB   hab;        /*!< Anchor block. */
    LONG  lType;      /*!< OD_DIRECT, OD_MEMORY, etc. */
    LONG  lCount;     /*!< Number of strings in pdop. */
    HDC   hdcComp;    /*!< Compatibility DC. */
} GpiDevOpenDC_Args, *PGpiDevOpenDC_Args;

#ifdef __cplusplus
}
#endif

#endif
