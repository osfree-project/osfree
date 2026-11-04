/*!
 * @file os2win.h
 * @brief OS/2 API compatibility header for Win32 builds.
 *
 * Provides the subset of OS/2 PM, GPI, and DOS types, constants, and
 * function declarations needed by the PMGPI test programs. Only used
 * for Win32 builds; OS/2 builds use the real SDK header.
 *
 * Do not include <windows.h> here: it defines some of the same names
 * (BOOL, HWND, HDC, ...) and the redefinition causes Watcom error
 * E125. Instead, declare everything locally.
 */
#ifndef OS2_COMPAT_H_INCLUDED
#define OS2_COMPAT_H_INCLUDED

#if defined(__OS2__) || defined(__OS2V2__)
#error os2.h compatibility header must not be used on OS/2 builds
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================ */
/* Base types                                                    */
/* ============================================================ */

typedef unsigned long  APIRET;     /*!< OS/2 return code. */
typedef unsigned long  ULONG;      /*!< 32-bit unsigned. */
typedef long           LONG;       /*!< 32-bit signed. */
typedef int            BOOL;       /*!< Boolean. */
typedef unsigned short USHORT;     /*!< 16-bit unsigned. */
typedef short          SHORT;      /*!< 16-bit signed. */
typedef unsigned char  BYTE;       /*!< 8-bit unsigned. */
typedef unsigned char *PBYTE;      /*!< Pointer to BYTE. */
typedef char          *PCH;        /*!< Pointer to CHAR. */
typedef char          *PSTR8;      /*!< Pointer to 8-bit string. */
typedef char           CHAR;       /*!< 8-bit character. */
typedef void          *PVOID;      /*!< Generic pointer. */
typedef long          *PLONG;      /*!< Pointer to LONG. */
typedef unsigned long *PULONG;     /*!< Pointer to ULONG. */
typedef unsigned short FIXED;      /*!< Fixed-point value. */
typedef FIXED         *PFIXED;     /*!< Pointer to FIXED. */
typedef unsigned int   UINT;       /*!< Unsigned int. */
typedef int            INT;        /*!< Signed int. */

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL ((void*)0)
#endif

/* ============================================================ */
/* PM handle types                                               */
/* ============================================================ */

typedef void *HAB;       /*!< Anchor block handle. */
typedef void *HMQ;       /*!< Message queue handle. */
typedef void *HPS;       /*!< Presentation space handle. */
typedef void *HWND;      /*!< Window handle. */
typedef void *HDC;       /*!< Device context handle. */
typedef void *HBITMAP;   /*!< Bitmap handle. */
typedef void *HRGN;      /*!< Region handle. */
typedef void *HPAL;      /*!< Palette handle. */
typedef void *HMF;       /*!< Metafile handle. */
typedef void *HMODULE;   /*!< Module handle. */

/* ============================================================ */
/* Geometry structures                                           */
/* ============================================================ */

typedef struct _POINTL { LONG x; LONG y; } POINTL, *PPOINTL;                 /*!< Point. */
typedef struct _SIZEL  { LONG cx; LONG cy; } SIZEL, *PSIZEL;                 /*!< Size. */
typedef struct _RECTL  { LONG xLeft; LONG yBottom;
                         LONG xRight; LONG yTop; } RECTL, *PRECTL;           /*!< Rectangle. */
typedef struct _SIZEF  { FIXED cx; FIXED cy; } SIZEF, *PSIZEF;               /*!< Fixed size. */
typedef struct _GRADIENTL { LONG x; LONG y; } GRADIENTL, *PGRADIENTL;        /*!< Gradient. */
typedef struct _MATRIXLF {
    FIXED fxM11; FIXED fxM12; LONG lM13;
    FIXED fxM21; FIXED fxM22; LONG lM23;
    LONG lM31; LONG lM32; LONG lM33;
} MATRIXLF, *PMATRIXLF;                                                       /*!< Transform matrix. */

/* ============================================================ */
/* GPI bundle and params structures                              */
/* ============================================================ */

/*! @brief Arc parameters (P, Q, R, S). */
typedef struct _ARCPARAMS {
    LONG lP;
    LONG lQ;
    LONG lR;
    LONG lS;
} ARCPARAMS, *PARCPARAMS;

/*! @brief Line bundle. */
typedef struct _LINEBUNDLE {
    LONG   lColor;
    LONG   lBackColor;
    USHORT usMixMode;
    USHORT usBackMixMode;
    FIXED  fxWidth;
    LONG   lGeomWidth;
    USHORT usType;
    USHORT usEnd;
    USHORT usJoin;
    USHORT usReserved;
} LINEBUNDLE, *PLINEBUNDLE;

/*! @brief Character bundle. */
typedef struct _CHARBUNDLE {
    LONG   lColor;
    LONG   lBackColor;
    USHORT usMixMode;
    USHORT usBackMixMode;
    USHORT usSet;
    USHORT usPrecision;
    SIZEF  sizfxCell;
    POINTL ptlAngle;
    POINTL ptlShear;
    USHORT usDirection;
    USHORT usTextAlign;
    FIXED  fxExtra;
    FIXED  fxBreakExtra;
} CHARBUNDLE, *PCHARBUNDLE;

/*! @brief Marker bundle. */
typedef struct _MARKERBUNDLE {
    LONG   lColor;
    LONG   lBackColor;
    USHORT usMixMode;
    USHORT usBackMixMode;
    USHORT usSet;
    USHORT usSymbol;
    SIZEF  sizfxCell;
} MARKERBUNDLE, *PMARKERBUNDLE;

/*! @brief Area bundle. */
typedef struct _AREABUNDLE {
    LONG   lColor;
    LONG   lBackColor;
    USHORT usMixMode;
    USHORT usBackMixMode;
    USHORT usSet;
    USHORT usSymbol;
    POINTL ptlRefPoint;
} AREABUNDLE, *PAREABUNDLE;

/*! @brief Generic bundle pointer (for GpiSetAttrs). */
typedef void *PBUNDLE;

/* ============================================================ */
/* Constants                                                     */
/* ============================================================ */

#define HWND_DESKTOP ((HWND)1)   /*!< Desktop window handle. */

#define GPI_ERROR 0L             /*!< GPI failure return. */
#define GPI_OK     1L            /*!< GPI success return. */
#define GPI_ALTERROR (-1L)       /*!< GPI alternate error return. */

/* Default color table indices */
#define CLR_BLACK      (-1L)     /*!< Black. */
#define CLR_BACKGROUND    0L     /*!< Background. */
#define CLR_BLUE          1L     /*!< Blue. */
#define CLR_RED           2L     /*!< Red. */
#define CLR_PINK          3L     /*!< Pink. */
#define CLR_GREEN         4L     /*!< Green. */
#define CLR_CYAN          5L     /*!< Cyan. */
#define CLR_YELLOW        6L     /*!< Yellow. */
#define CLR_NEUTRAL       7L     /*!< Neutral. */
#define CLR_WHITE         0L     /*!< White (alias). */

/* Box draw control options */
#define DRO_FILL        1L       /*!< Fill only. */
#define DRO_OUTLINE     2L       /*!< Outline only. */
#define DRO_OUTLINEFILL 3L       /*!< Outline and fill. */

/* Drawing modes */
#define DM_ERROR            0L   /*!< Invalid mode. */
#define DM_DRAW             1L   /*!< Draw immediately. */
#define DM_RETAIN           2L   /*!< Retain only. */
#define DM_DRAWANDRETAIN    3L   /*!< Draw and retain. */

/* Segment attributes (ATTR_*) */
#define ATTR_DETECTABLE         1L
#define ATTR_VISIBLE            2L
#define ATTR_CHAINED            6L
#define ATTR_DYNAMIC            8L
#define ATTR_FASTCHAIN          9L
#define ATTR_PROP_DETECTABLE   10L
#define ATTR_PROP_VISIBLE      11L
#define ATTR_OFF                0L
#define ATTR_ON                 1L

/* Segment priority (GpiSet/QuerySegmentPriority) */
#define LOWER_PRI            (-1L)
#define HIGHER_PRI              1L

/* Draw control */
#define DCTL_ERASE      1L
#define DCTL_DISPLAY    2L
#define DCTL_BOUNDARY   3L
#define DCTL_DYNAMIC    4L
#define DCTL_CORRELATE  5L
#define DCTL_OFF        0L
#define DCTL_ON         1L

/* Stop draw */
#define SDW_OFF         0L
#define SDW_ON          1L

/* Edit mode */
#define SEGEM_INSERT    1L
#define SEGEM_REPLACE   2L

/* Primitive types */
#define PRIM_LINE       1L
#define PRIM_CHAR       2L
#define PRIM_MARKER     3L
#define PRIM_AREA       4L
#define PRIM_IMAGE      5L

/* Line bundle attribute masks */
#define LBB_COLOR          0x0001L
#define LBB_BACK_COLOR     0x0002L
#define LBB_MIX_MODE       0x0004L
#define LBB_BACK_MIX_MODE  0x0008L
#define LBB_WIDTH          0x0010L
#define LBB_GEOM_WIDTH     0x0020L
#define LBB_TYPE           0x0040L
#define LBB_END            0x0080L
#define LBB_JOIN           0x0100L

/* Character bundle attribute masks */
#define CBB_COLOR          0x0001L
#define CBB_BACK_COLOR     0x0002L
#define CBB_MIX_MODE       0x0004L
#define CBB_BACK_MIX_MODE  0x0008L
#define CBB_SET            0x0010L
#define CBB_MODE           0x0020L
#define CBB_BOX            0x0040L
#define CBB_ANGLE          0x0080L
#define CBB_SHEAR          0x0100L
#define CBB_DIRECTION      0x0200L
#define CBB_TEXT_ALIGN     0x0400L
#define CBB_EXTRA          0x0800L
#define CBB_BREAK_EXTRA    0x1000L

/* Marker bundle attribute masks */
#define MBB_COLOR          0x0001L
#define MBB_BACK_COLOR     0x0002L
#define MBB_MIX_MODE       0x0004L
#define MBB_BACK_MIX_MODE  0x0008L
#define MBB_SET            0x0010L
#define MBB_SYMBOL         0x0020L
#define MBB_BOX            0x0040L

/* Area bundle attribute masks */
#define ABB_COLOR          0x0001L
#define ABB_BACK_COLOR     0x0002L
#define ABB_MIX_MODE       0x0004L
#define ABB_BACK_MIX_MODE  0x0008L
#define ABB_SET            0x0010L
#define ABB_SYMBOL         0x0020L
#define ABB_REF_POINT      0x0040L

/* Path constants */
#define MPATH_STROKE    6L
#define FPATH_ALTERNATE 0L
#define FPATH_WINDING   2L
#define FPATH_INCL      0L
#define FPATH_EXCL      8L
#define SCP_ALTERNATE   0L
#define SCP_WINDING     2L
#define SCP_AND         4L
#define SCP_RESET       0L
#define SCP_INCL        0L
#define SCP_EXCL        8L

/* ============================================================ */
/* Calling conventions                                           */
/* ============================================================ */
#pragma aux __estdcall "*" \
           parm routine [] \
           value struct struct caller [] \
           modify [eax ecx edx]

#define APIENTRY __declspec(__pragma("__estdcall"))
#define EXPENTRY __declspec(__pragma("__estdcall"))

/* ============================================================ */
/* String types                                                  */
/* ============================================================ */
typedef const char *PCSZ;   /*!< Pointer to constant string. */
typedef char       *PSZ;    /*!< Pointer to mutable string. */

/* ============================================================ */
/* PM function declarations (implemented in win_compat.cpp)      */
/* ============================================================ */

HAB  APIENTRY WinInitialize(ULONG flOptions);
HMQ  APIENTRY WinCreateMsgQueue(HAB hab, LONG cMsg);
BOOL APIENTRY WinDestroyMsgQueue(HMQ hmq);
BOOL APIENTRY WinTerminate(HAB hab);
HPS  APIENTRY WinGetScreenPS(HWND hwndDesktop);
BOOL APIENTRY WinReleasePS(HPS hps);

/* ============================================================ */
/* GPI: PS lifecycle                                             */
/* ============================================================ */

HPS   APIENTRY GpiCreatePS(HAB hab, HDC hdc, PSIZEL psizlSize, ULONG flOptions);
BOOL  APIENTRY GpiDestroyPS(HPS hps);
BOOL  APIENTRY GpiAssociate(HPS hps, HDC hdc);
HDC   APIENTRY GpiQueryDevice(HPS hps);
BOOL  APIENTRY GpiErase(HPS hps);
BOOL  APIENTRY GpiResetPS(HPS hps, ULONG flOptions);
BOOL  APIENTRY GpiSetPS(HPS hps, PSIZEL psizlsize, ULONG flOptions);
ULONG APIENTRY GpiQueryPS(HPS hps, PSIZEL psizlSize);
LONG  APIENTRY GpiSavePS(HPS hps);
BOOL  APIENTRY GpiRestorePS(HPS hps, LONG lPSid);

/* ============================================================ */
/* GPI: Drawing state                                            */
/* ============================================================ */

BOOL  APIENTRY GpiSetDrawingMode(HPS hps, LONG lMode);
LONG  APIENTRY GpiQueryDrawingMode(HPS hps);
BOOL  APIENTRY GpiSetDrawControl(HPS hps, LONG lControl, LONG lValue);
LONG  APIENTRY GpiQueryDrawControl(HPS hps, LONG lControl);
BOOL  APIENTRY GpiSetStopDraw(HPS hps, LONG lValue);
LONG  APIENTRY GpiQueryStopDraw(HPS hps);
BOOL  APIENTRY GpiSetEditMode(HPS hps, LONG lMode);
LONG  APIENTRY GpiQueryEditMode(HPS hps);
BOOL  APIENTRY GpiSetInitialSegmentAttrs(HPS hps, LONG lAttribute, LONG lValue);
LONG  APIENTRY GpiQueryInitialSegmentAttrs(HPS hps, LONG lAttribute);

/* ============================================================ */
/* GPI: Primitives                                               */
/* ============================================================ */

BOOL  APIENTRY GpiSetColor(HPS hps, LONG lColor);
LONG  APIENTRY GpiQueryColor(HPS hps);
BOOL  APIENTRY GpiMove(HPS hps, PPOINTL pptlPoint);
BOOL  APIENTRY GpiSetCurrentPosition(HPS hps, PPOINTL pptlPoint);
BOOL  APIENTRY GpiQueryCurrentPosition(HPS hps, PPOINTL pptlPoint);
LONG  APIENTRY GpiLine(HPS hps, PPOINTL pptlEndPoint);
LONG  APIENTRY GpiPolyLine(HPS hps, LONG lCount, PPOINTL aptlPoints);
LONG  APIENTRY GpiBox(HPS hps, LONG lControl, PPOINTL pptlPoint,
                      LONG lHRound, LONG lVRound);
LONG  APIENTRY GpiMarker(HPS hps, PPOINTL pptlPoint);
LONG  APIENTRY GpiPolyMarker(HPS hps, LONG lCount, PPOINTL aptlPoints);
LONG  APIENTRY GpiCharString(HPS hps, LONG lCount, PCH pchString);
LONG  APIENTRY GpiCharStringAt(HPS hps, PPOINTL pptlPoint,
                                LONG lCount, PCH pchString);
BOOL  APIENTRY GpiSetArcParams(HPS hps, PARCPARAMS parcpArcParams);
BOOL  APIENTRY GpiSetAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask,
                            ULONG flDefMask, PBUNDLE ppbunAttrs);

/* ============================================================ */
/* GPI: Attribute queries                                        */
/* ============================================================ */

LONG  APIENTRY GpiQueryLineType(HPS hps);
LONG  APIENTRY GpiQueryLineWidthGeom(HPS hps);
LONG  APIENTRY GpiQueryMix(HPS hps);

/* ============================================================ */
/* GPI: Segments                                                 */
/* ============================================================ */

BOOL  APIENTRY GpiOpenSegment(HPS hps, LONG lSegment);
BOOL  APIENTRY GpiCloseSegment(HPS hps);
BOOL  APIENTRY GpiDrawSegment(HPS hps, LONG lSegment);
BOOL  APIENTRY GpiDeleteSegment(HPS hps, LONG lSegid);
BOOL  APIENTRY GpiDeleteSegments(HPS hps, LONG lFirstSegment, LONG lLastSegment);
LONG  APIENTRY GpiQuerySegmentNames(HPS hps, LONG lFirstSegid, LONG lLastSegid,
                                     LONG lMax, PLONG alSegids);
LONG  APIENTRY GpiQuerySegmentAttrs(HPS hps, LONG lSegid, LONG lAttribute);
BOOL  APIENTRY GpiSetSegmentAttrs(HPS hps, LONG lSegid, LONG lAttribute, LONG lValue);
LONG  APIENTRY GpiGetData(HPS hps, LONG lSegid, PLONG plOffset, LONG lFormat,
                           LONG lLength, PBYTE pbData);
LONG  APIENTRY GpiPutData(HPS hps, LONG lFormat, PLONG plCount, PBYTE pbData);
BOOL  APIENTRY GpiDrawChain(HPS hps);
BOOL  APIENTRY GpiDrawFrom(HPS hps, LONG lFirstSegment, LONG lLastSegment);
BOOL  APIENTRY GpiDrawDynamics(HPS hps);
BOOL  APIENTRY GpiRemoveDynamics(HPS hps, LONG lFirstSegid, LONG lLastSegid);
LONG  APIENTRY GpiQuerySegmentPriority(HPS hps, LONG lRefSegid, LONG lOrder);
BOOL  APIENTRY GpiSetSegmentPriority(HPS hps, LONG lSegid, LONG lRefSegid, LONG lOrder);

/* ============================================================ */
/* GPI: Element editing                                          */
/* ============================================================ */

BOOL  APIENTRY GpiBeginElement(HPS hps, LONG lType, PCSZ pszDesc);
BOOL  APIENTRY GpiEndElement(HPS hps);
LONG  APIENTRY GpiElement(HPS hps, LONG lType, PCSZ pszDesc,
                           LONG lLength, PBYTE pbData);
BOOL  APIENTRY GpiLabel(HPS hps, LONG lLabel);
LONG  APIENTRY GpiQueryElement(HPS hps, LONG lOff, LONG lMaxLength, PBYTE pbData);
LONG  APIENTRY GpiQueryElementType(HPS hps, PLONG plType, LONG lLength, PCSZ pszData);
BOOL  APIENTRY GpiSetElementPointer(HPS hps, LONG lElement);
LONG  APIENTRY GpiQueryElementPointer(HPS hps);
BOOL  APIENTRY GpiOffsetElementPointer(HPS hps, LONG lOffset);
BOOL  APIENTRY GpiSetElementPointerAtLabel(HPS hps, LONG lLabel);
BOOL  APIENTRY GpiDeleteElement(HPS hps);
BOOL  APIENTRY GpiDeleteElementRange(HPS hps, LONG lFirstElement, LONG lLastElement);
BOOL  APIENTRY GpiDeleteElementsBetweenLabels(HPS hps, LONG lFirstLabel, LONG lLastLabel);

/* ============================================================ */
/* GPI: Paths                                                    */
/* ============================================================ */

BOOL  APIENTRY GpiBeginPath(HPS hps, LONG lPath);
BOOL  APIENTRY GpiEndPath(HPS hps);
BOOL  APIENTRY GpiCloseFigure(HPS hps);
BOOL  APIENTRY GpiModifyPath(HPS hps, LONG lPath, LONG lMode);
LONG  APIENTRY GpiFillPath(HPS hps, LONG lPath, LONG lOptions);
LONG  APIENTRY GpiOutlinePath(HPS hps, LONG lPath, LONG lOptions);
BOOL  APIENTRY GpiSetClipPath(HPS hps, LONG lPath, LONG lOptions);
LONG  APIENTRY GpiStrokePath(HPS hps, LONG lPath, ULONG flOptions);

#ifdef __cplusplus
}
#endif

#endif /* OS2_COMPAT_H_INCLUDED */
