/* gpi_platform.h */
#ifndef GPI_PLATFORM_H
#define GPI_PLATFORM_H

//#include <stddef.h> /* Для NULL */

#if defined(__OS2__) || defined(__OS2V2__)

  #include <os2.h>
  #define EXTERN

#elif defined(__WIN32__) || defined(_WIN32)


#ifdef __cplusplus
      extern "C" {
#endif

  /* ---------- Базовые типы ---------- */
  typedef unsigned long DWORD;
  typedef unsigned int  UINT;
  typedef unsigned long ULONG;
  typedef long          LONG;
  typedef unsigned short USHORT;
  typedef short         SHORT;
  typedef unsigned char BYTE;
  typedef char          CHAR;
  typedef void         *PVOID;
  typedef char         *PCH;
  typedef unsigned char *PBYTE;
  typedef LONG         *PLONG;
  typedef ULONG        *PULONG;
  typedef unsigned short FIXED;
  typedef FIXED        *PFIXED;
  typedef char         *PSTR8;
  typedef unsigned int  SIZE_T;
  typedef unsigned long APIRET;
  typedef int           BOOL;
  typedef unsigned long ULONG32;
  typedef long          LONG32;
  typedef unsigned long MPARAM;
  typedef unsigned long MRESULT;

  /* OS/2-compatible pointer to a generic procedure (for DosQueryProcAddr). */
  typedef void (*PFN)(void);

  #ifndef TRUE
  #define TRUE 1
  #endif
  #ifndef FALSE
  #define FALSE 0
  #endif
  #ifndef NULL
  #define NULL ((void*)0)
  #endif

  /* ---------- Хэндлы ---------- */
  typedef void *HAB;
  typedef void *HDC;
  typedef void *HPS;
  typedef void *HWND;
  typedef void *HBITMAP;
  typedef void *HRGN;
  typedef void *HPAL;
  typedef void *HMF;
  typedef void *HMODULE;
  typedef void *HANDLE;
  typedef void *HMQ;

  /* ---------- Строковые и указательные типы ---------- */
  typedef const char   *PCSZ;
  typedef char         *PSZ;
  typedef void         *PBUNDLE;
  typedef HRGN         *PHRGN;

  /* ---------- Конвенции вызова ---------- */
  /* Custom calling convention: __stdcall stack cleanup, no name
     decoration. Prevents _GpiSetColor@8 / _PmcPostCommand@8, so the
     DLL exports match the .def entries and the client resolves the
     PMCLIENT.LIB imports. Same pragma as F_platform.hpp. */
    #pragma aux __estdcall "*" \
               parm routine [] \
               value struct struct caller [] \
               modify [eax ecx edx]

    #undef EXPENTRY
    #define EXPENTRY __declspec(__pragma("__estdcall"))

  #ifndef APIENTRY
    #define APIENTRY   __declspec(__pragma("__estdcall"))
  #endif
  #ifndef APIENTRY16
    #define APIENTRY16 far pascal
  #endif

  #define EXTERN extern "C" 

  /* ---------- Коды ошибок GPI ---------- */
  #ifndef GPI_ERROR
    #define GPI_ERROR (-1L)
  #endif
  #ifndef GPI_OK
    #define GPI_OK 1L
  #endif

  /* ---------- Константы DosAllocMem ---------- */
  #ifndef PAG_COMMIT
    #define PAG_COMMIT     0x00010000
  #endif
  #ifndef PAG_READ
    #define PAG_READ       0x00000001
  #endif
  #ifndef PAG_WRITE
    #define PAG_WRITE      0x00000004
  #endif

  /* ---------- Константы BITMAPINFOHEADER2 ---------- */
  #ifndef BCA_UNCOMP
    #define BCA_UNCOMP          0L
  #endif
  #ifndef BRU_METRIC
    #define BRU_METRIC          0L
  #endif
  #ifndef BRA_BOTTOMUP
    #define BRA_BOTTOMUP        0L
  #endif
  #ifndef BRH_NOTHALFTONED
    #define BRH_NOTHALFTONED    0L
  #endif
  #ifndef BCE_RGB
    #define BCE_RGB             0L
  #endif

  /* ---------- Константы Win32 API ---------- */
  #ifndef MEM_COMMIT
    #define MEM_COMMIT     0x1000
  #endif
  #ifndef MEM_RELEASE
    #define MEM_RELEASE    0x8000
  #endif
  #ifndef PAGE_READWRITE
    #define PAGE_READWRITE 0x04
  #endif

  /* ---------- Win32 API ---------- */
  __declspec(dllimport) PVOID __stdcall VirtualAlloc(PVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
  __declspec(dllimport) DWORD __stdcall VirtualFree(PVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
  __declspec(dllimport) HMODULE __stdcall GetModuleHandleA(const char *lpModuleName);
  __declspec(dllimport) PVOID __stdcall GetProcAddress(HMODULE hModule, const char *lpProcName);
  __declspec(dllimport) void __stdcall OutputDebugStringA(const char *lpOutputString);

  /* ---------- Эмуляция OS/2 API ---------- */
  static inline APIRET DosAllocMem(PVOID *ppv, ULONG cb, ULONG flags) {
      (void)flags;
      *ppv = VirtualAlloc(NULL, cb, MEM_COMMIT, PAGE_READWRITE);
      return (*ppv != NULL) ? 0 : 1;
  }
  static inline APIRET DosFreeMem(PVOID pv) {
      return VirtualFree(pv, 0, MEM_RELEASE) ? 0 : 1;
  }
  static inline APIRET DosQueryModuleHandle(const char *pszName, HMODULE *phmod) {
      *phmod = GetModuleHandleA(pszName);
      return (*phmod != NULL) ? 0 : 1;
  }
  static inline APIRET DosQueryProcAddr(HMODULE hmod, ULONG ord, const char *pszName, PFN *ppfn) {
      (void)pszName;
      *ppfn = (PFN)GetProcAddress(hmod, (const char *)(ULONG)ord);
      return (*ppfn != NULL) ? 0 : 1;
  }

  /* ---------- Структуры ---------- */
  typedef struct _POINTL { LONG x; LONG y; } POINTL, *PPOINTL;
  typedef struct _SIZEL { LONG cx; LONG cy; } SIZEL, *PSIZEL;
  typedef struct _SIZEF { FIXED cx; FIXED cy; } SIZEF, *PSIZEF;
  typedef struct _RECTL { LONG xLeft; LONG yBottom; LONG xRight; LONG yTop; } RECTL, *PRECTL;
  typedef struct _MATRIXLF { FIXED fxM11; FIXED fxM12; LONG lM13; FIXED fxM21; FIXED fxM22; LONG lM23; LONG lM31; LONG lM32; LONG lM33; } MATRIXLF, *PMATRIXLF;
  typedef struct _ARCPARAMS { LONG lP; LONG lQ; LONG lR; LONG lS; } ARCPARAMS, *PARCPARAMS;
  typedef struct _GRADIENTL { LONG x; LONG y; } GRADIENTL, *PGRADIENTL;

  typedef struct _RGB { BYTE bBlue; BYTE bGreen; BYTE bRed; BYTE fcOptions; } RGB, *PRGB;
  typedef RGB RGB2, *PRGB2;

  typedef struct _BITMAPINFOHEADER {
      ULONG cbFix, cx, cy; USHORT cPlanes, cBitCount;
  } BITMAPINFOHEADER, *PBITMAPINFOHEADER;

  typedef struct _BITMAPINFOHEADER2 {
      ULONG cbFix, cx, cy; USHORT cPlanes, cBitCount;
      ULONG ulCompression, cbImage, cxResolution, cyResolution, cclrUsed, cclrImportant;
      USHORT usUnits, usReserved, usRecording, usRendering;
      ULONG cSize1, cSize2, ulColorEncoding, ulIdentifier;
  } BITMAPINFOHEADER2, *PBITMAPINFOHEADER2;

  /* Плоские: поля заголовка доступны напрямую (pInfo->cbFix) */
  typedef struct _BITMAPINFO {
      ULONG cbFix, cx, cy; USHORT cPlanes, cBitCount; RGB argbColor[1];
  } BITMAPINFO, *PBITMAPINFO;

  typedef struct _BITMAPINFO2 {
      ULONG cbFix, cx, cy; USHORT cPlanes, cBitCount;
      ULONG ulCompression, cbImage, cxResolution, cyResolution, cclrUsed, cclrImportant;
      USHORT usUnits, usReserved, usRecording, usRendering;
      ULONG cSize1, cSize2, ulColorEncoding, ulIdentifier;
      RGB2 argbColor[1];
  } BITMAPINFO2, *PBITMAPINFO2;

  typedef struct _FATTRS { USHORT usRecordLength, fsSelection; LONG lMatch; CHAR szFacename[32]; USHORT idRegistry, usCodePage; LONG lMaxBaselineExt, lAveCharWidth; } FATTRS, *PFATTRS;
  typedef struct _FONTMETRICS { CHAR szFamilyname[32], szFacename[32]; USHORT usCodepage, usWeight, usWidth; } FONTMETRICS, *PFONTMETRICS;
  typedef struct _KERNINGPAIRS { SHORT sFirstChar, sSecondChar; LONG lKerningAmount; } KERNINGPAIRS, *PKERNINGPAIRS;
  typedef struct _FACENAMEDESC { USHORT usSize, usWeightClass, usWidthClass, usReserved; ULONG flOptions; } FACENAMEDESC, *PFACENAMEDESC;
  typedef struct _RASTERIZERCAPS { ULONG cb, ulAvailFlags, ulEnabledFlags; } RASTERIZERCAPS, *PRASTERIZERCAPS;
  typedef struct _POLYGON { ULONG ulPoints; PPOINTL aPointl; } POLYGON, *PPOLYGON;
  typedef struct _POLYSET { ULONG ulPolys; POLYGON aPolygon[1]; } POLYSET, *PPOLYSET;
  typedef struct _RGNRECT { ULONG ircStart, crc, crcReturned, ulDirection; } RGNRECT, *PRGNRECT;
  typedef CHAR FFDESCS[2][32];
  typedef FFDESCS *PFFDESCS;
  typedef struct _QMSG { HWND hwnd; ULONG msg; MPARAM mp1, mp2; ULONG time; POINTL ptl; ULONG ulReserved; } QMSG, *PQMSG;

  /* ==================================================================
     Прототипы Gpi* (для gpi16*.c). Пропущены GpiBeginElement /
     GpiQueryElementType / GpiElement: они объявлены в pmgpi16.c с
     PSZ (non-const), дубликат с PCSZ вызвал бы конфликт типов.
     ================================================================== */

  /* GPICONTROL */
  HPS   APIENTRY GpiCreatePS(HAB hab, HDC hdc, PSIZEL psizlSize, ULONG flOptions);
  BOOL  APIENTRY GpiDestroyPS(HPS hps);
  BOOL  APIENTRY GpiAssociate(HPS hps, HDC hdc);
  BOOL  APIENTRY GpiRestorePS(HPS hps, LONG lPSid);
  LONG  APIENTRY GpiSavePS(HPS hps);
  BOOL  APIENTRY GpiErase(HPS hps);
  HDC   APIENTRY GpiQueryDevice(HPS hps);
  BOOL  APIENTRY GpiResetPS(HPS hps, ULONG flOptions);
  BOOL  APIENTRY GpiSetPS(HPS hps, PSIZEL psizlsize, ULONG flOptions);
  ULONG APIENTRY GpiQueryPS(HPS hps, PSIZEL psizlSize);
  LONG  APIENTRY GpiErrorSegmentData(HPS hps, PLONG plSegment, PLONG plContext);
  LONG  APIENTRY GpiQueryDrawControl(HPS hps, LONG lControl);
  BOOL  APIENTRY GpiSetDrawControl(HPS hps, LONG lControl, LONG lValue);
  LONG  APIENTRY GpiQueryDrawingMode(HPS hps);
  BOOL  APIENTRY GpiSetDrawingMode(HPS hps, LONG lMode);
  LONG  APIENTRY GpiQueryStopDraw(HPS hps);
  BOOL  APIENTRY GpiSetStopDraw(HPS hps, LONG lValue);

  /* GPICORRELATION */
  LONG  APIENTRY GpiCorrelateChain(HPS hps, LONG lType, PPOINTL pptlPick, LONG lMaxHits, LONG lMaxDepth, PLONG pl2);
  BOOL  APIENTRY GpiQueryTag(HPS hps, PLONG plTag);
  BOOL  APIENTRY GpiSetTag(HPS hps, LONG lTag);
  BOOL  APIENTRY GpiQueryPickApertureSize(HPS hps, PSIZEL psizlSize);
  BOOL  APIENTRY GpiSetPickApertureSize(HPS hps, LONG lOptions, PSIZEL psizlSize);
  BOOL  APIENTRY GpiQueryPickAperturePosition(HPS hps, PPOINTL pptlPoint);
  BOOL  APIENTRY GpiSetPickAperturePosition(HPS hps, PPOINTL pptlPick);
  BOOL  APIENTRY GpiQueryBoundaryData(HPS hps, PRECTL prclBoundary);
  BOOL  APIENTRY GpiResetBoundaryData(HPS hps);
  LONG  APIENTRY GpiCorrelateFrom(HPS hps, LONG lFirstSegment, LONG lLastSegment, LONG lType, PPOINTL pptlPick, LONG lMaxHits, LONG lMaxDepth, PLONG plSegTag);
  LONG  APIENTRY GpiCorrelateSegment(HPS hps, LONG lSegment, LONG lType, PPOINTL pptlPick, LONG lMaxHits, LONG lMaxDepth, PLONG alSegTag);

  /* GPISEGMENTS */
  BOOL  APIENTRY GpiOpenSegment(HPS hps, LONG lSegment);
  BOOL  APIENTRY GpiCloseSegment(HPS hps);
  BOOL  APIENTRY GpiDeleteSegment(HPS hps, LONG lSegid);
  LONG  APIENTRY GpiQueryInitialSegmentAttrs(HPS hps, LONG lAttribute);
  BOOL  APIENTRY GpiSetInitialSegmentAttrs(HPS hps, LONG lAttribute, LONG lValue);
  LONG  APIENTRY GpiQuerySegmentAttrs(HPS hps, LONG lSegid, LONG lAttribute);
  BOOL  APIENTRY GpiSetSegmentAttrs(HPS hps, LONG lSegid, LONG lAttribute, LONG lValue);
  LONG  APIENTRY GpiQuerySegmentPriority(HPS hps, LONG lRefSegid, LONG lOrder);
  BOOL  APIENTRY GpiSetSegmentPriority(HPS hps, LONG lSegid, LONG lRefSegid, LONG lOrder);
  BOOL  APIENTRY GpiDeleteSegments(HPS hps, LONG lFirstSegment, LONG lLastSegment);
  LONG  APIENTRY GpiQuerySegmentNames(HPS hps, LONG lFirstSegid, LONG lLastSegid, LONG lMax, PLONG alSegids);
  LONG  APIENTRY GpiGetData(HPS hps, LONG lSegid, PLONG plOffset, LONG lFormat, LONG lLength, PBYTE pbData);
  LONG  APIENTRY GpiPutData(HPS hps, LONG lFormat, PLONG plCount, PBYTE pbData);
  BOOL  APIENTRY GpiDrawChain(HPS hps);
  BOOL  APIENTRY GpiDrawFrom(HPS hps, LONG lFirstSegment, LONG lLastSegment);
  BOOL  APIENTRY GpiDrawSegment(HPS hps, LONG lSegment);
  BOOL  APIENTRY GpiDrawDynamics(HPS hps);
  BOOL  APIENTRY GpiRemoveDynamics(HPS hps, LONG lFirstSegid, LONG lLastSegid);

  /* GPISEGEDITING */
  LONG  APIENTRY GpiQueryEditMode(HPS hps);
  BOOL  APIENTRY GpiSetEditMode(HPS hps, LONG lMode);
  LONG  APIENTRY GpiQueryElementPointer(HPS hps);
  BOOL  APIENTRY GpiSetElementPointer(HPS hps, LONG lElement);
  BOOL  APIENTRY GpiOffsetElementPointer(HPS hps, LONG loffset);
  BOOL  APIENTRY GpiDeleteElement(HPS hps);
  BOOL  APIENTRY GpiDeleteElementRange(HPS hps, LONG lFirstElement, LONG lLastElement);
  BOOL  APIENTRY GpiLabel(HPS hps, LONG lLabel);
  BOOL  APIENTRY GpiDeleteElementsBetweenLabels(HPS hps, LONG lFirstLabel, LONG lLastLabel);
  BOOL  APIENTRY GpiSetElementPointerAtLabel(HPS hps, LONG lLabel);
  LONG  APIENTRY GpiQueryElement(HPS hps, LONG lOff, LONG lMaxLength, PBYTE pbData);
  BOOL  APIENTRY GpiEndElement(HPS hps);

  /* GPITRANSFORMS */
  BOOL  APIENTRY GpiQuerySegmentTransformMatrix(HPS hps, LONG lSegid, LONG lCount, PMATRIXLF pmatlfArray);
  BOOL  APIENTRY GpiSetSegmentTransformMatrix(HPS hps, LONG lSegid, LONG lCount, PMATRIXLF pmatlfarray, LONG lOptions);
  BOOL  APIENTRY GpiConvert(HPS hps, LONG lSrc, LONG lTarg, LONG lCount, PPOINTL aptlPoints);
  BOOL  APIENTRY GpiConvertWithMatrix(HPS hps, LONG lCountp, PPOINTL aptlPoints, LONG lCount, PMATRIXLF pmatlfArray);
  BOOL  APIENTRY GpiQueryModelTransformMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray);
  BOOL  APIENTRY GpiSetModelTransformMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray, LONG lOptions);
  LONG  APIENTRY GpiCallSegmentMatrix(HPS hps, LONG lSegment, LONG lCount, PMATRIXLF pmatlfArray, LONG lOptions);
  BOOL  APIENTRY GpiQueryDefaultViewMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray);
  BOOL  APIENTRY GpiSetDefaultViewMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfarray, LONG lOptions);
  BOOL  APIENTRY GpiQueryPageViewport(HPS hps, PRECTL prclViewport);
  BOOL  APIENTRY GpiSetPageViewport(HPS hps, PRECTL prclViewport);
  BOOL  APIENTRY GpiQueryViewingTransformMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray);
  BOOL  APIENTRY GpiSetViewingTransformMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray, LONG lOptions);
  BOOL  APIENTRY GpiTranslate(HPS hps, PMATRIXLF pmatlf, LONG lOptions, PPOINTL pptl);
  BOOL  APIENTRY GpiScale(HPS hps, PMATRIXLF pmatlf, LONG lOptions, PFIXED afx, PPOINTL pptl);
  BOOL  APIENTRY GpiRotate(HPS hps, PMATRIXLF pmatlf, LONG lOptions, FIXED fxAngle, PPOINTL pptl);
  BOOL  APIENTRY GpiSetGraphicsField(HPS hps, PRECTL prclField);
  BOOL  APIENTRY GpiQueryGraphicsField(HPS hps, PRECTL prclField);
  BOOL  APIENTRY GpiSetViewingLimits(HPS hps, PRECTL prclLimits);
  BOOL  APIENTRY GpiQueryViewingLimits(HPS hps, PRECTL prclLimits);

  /* GPIPATHS */
  BOOL  APIENTRY GpiBeginPath(HPS hps, LONG lPath);
  BOOL  APIENTRY GpiEndPath(HPS hps);
  BOOL  APIENTRY GpiCloseFigure(HPS hps);
  BOOL  APIENTRY GpiModifyPath(HPS hps, LONG lPath, LONG lMode);
  LONG  APIENTRY GpiFillPath(HPS hps, LONG lPath, LONG lOptions);
  BOOL  APIENTRY GpiSetClipPath(HPS hps, LONG lPath, LONG lOptions);
  LONG  APIENTRY GpiOutlinePath(HPS hps, LONG lPath, LONG lOptions);
  HRGN  APIENTRY GpiPathToRegion(HPS hps, LONG lPath, LONG lOptions);
  LONG  APIENTRY GpiStrokePath(HPS hps, LONG lPath, ULONG flOptions);

  /* GPILOGCOLORTABLE */
  BOOL  APIENTRY GpiCreateLogColorTable(HPS hps, ULONG flOptions, LONG lFormat, LONG lStart, LONG lCount, PLONG alTable);
  BOOL  APIENTRY GpiQueryColorData(HPS hps, LONG lCount, PLONG alArray);
  LONG  APIENTRY GpiQueryLogColorTable(HPS hps, ULONG flOptions, LONG lStart, LONG lCount, PLONG alArray);
  LONG  APIENTRY GpiQueryRealColors(HPS hps, ULONG flOptions, LONG lStart, LONG lCount, PLONG alColors);
  LONG  APIENTRY GpiQueryNearestColor(HPS hps, ULONG flOptions, LONG lRgbIn);
  LONG  APIENTRY GpiQueryColorIndex(HPS hps, ULONG flOptions, LONG lRgbColor);
  LONG  APIENTRY GpiQueryRGBColor(HPS hps, ULONG flOptions, LONG lColorIndex);
  HPAL  APIENTRY GpiCreatePalette(HAB hab, ULONG flOptions, ULONG ulFormat, ULONG ulCount, PULONG aulTable);
  BOOL  APIENTRY GpiDeletePalette(HPAL hpal);
  HPAL  APIENTRY GpiSelectPalette(HPS hps, HPAL hpal);
  LONG  APIENTRY GpiAnimatePalette(HPAL hpal, ULONG ulFormat, ULONG ulStart, ULONG ulCount, PULONG aulTable);
  BOOL  APIENTRY GpiSetPaletteEntries(HPAL hpal, ULONG ulFormat, ULONG ulStart, ULONG ulCount, PULONG aulTable);
  HPAL  APIENTRY GpiQueryPalette(HPS hps);
  LONG  APIENTRY GpiQueryPaletteInfo(HPAL hpal, HPS hps, ULONG flOptions, ULONG ulStart, ULONG ulCount, PULONG aulArray);
  ULONG APIENTRY GpiResizePalette(HPAL hpal, ULONG newsize);
  LONG  APIENTRY GpiQueryNearestPaletteIndex(HPAL hpal, ULONG color);

  /* GPIPRIMITIVES */
  BOOL  APIENTRY GpiSetColor(HPS hps, LONG lColor);
  LONG  APIENTRY GpiQueryColor(HPS hps);
  LONG  APIENTRY GpiBox(HPS hps, LONG lControl, PPOINTL pptlPoint, LONG lHRound, LONG lVRound);
  BOOL  APIENTRY GpiMove(HPS hps, PPOINTL pptlPoint);
  LONG  APIENTRY GpiLine(HPS hps, PPOINTL pptlEndPoint);
  LONG  APIENTRY GpiPolyLine(HPS hps, LONG lCount, PPOINTL aptlPoints);
  LONG  APIENTRY GpiPolyLineDisjoint(HPS hps, LONG lCount, PPOINTL aptlPoints);
  BOOL  APIENTRY GpiSetPattern(HPS hps, LONG lPatternSymbol);
  LONG  APIENTRY GpiQueryPattern(HPS hps);
  BOOL  APIENTRY GpiBeginArea(HPS hps, ULONG flOptions);
  LONG  APIENTRY GpiEndArea(HPS hps);
  LONG  APIENTRY GpiCharString(HPS hps, LONG lCount, PCH pchString);
  LONG  APIENTRY GpiCharStringAt(HPS hps, PPOINTL pptlPoint, LONG lCount, PCH pchString);
  BOOL  APIENTRY GpiSetAttrMode(HPS hps, LONG lMode);
  LONG  APIENTRY GpiQueryAttrMode(HPS hps);
  BOOL  APIENTRY GpiSetAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask, ULONG flDefMask, PBUNDLE ppbunAttrs);
  LONG  APIENTRY GpiQueryAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask, PBUNDLE ppbunAttrs);
  BOOL  APIENTRY GpiSetBackColor(HPS hps, LONG lColor);
  LONG  APIENTRY GpiQueryBackColor(HPS hps);
  BOOL  APIENTRY GpiSetMix(HPS hps, LONG lMixMode);
  LONG  APIENTRY GpiQueryMix(HPS hps);
  BOOL  APIENTRY GpiSetBackMix(HPS hps, LONG lMixMode);
  LONG  APIENTRY GpiQueryBackMix(HPS hps);
  BOOL  APIENTRY GpiSetLineType(HPS hps, LONG lLineType);
  LONG  APIENTRY GpiQueryLineType(HPS hps);
  BOOL  APIENTRY GpiSetLineWidth(HPS hps, FIXED fxLineWidth);
  FIXED APIENTRY GpiQueryLineWidth(HPS hps);
  BOOL  APIENTRY GpiSetLineWidthGeom(HPS hps, LONG lLineWidth);
  LONG  APIENTRY GpiQueryLineWidthGeom(HPS hps);
  BOOL  APIENTRY GpiSetLineEnd(HPS hps, LONG lLineEnd);
  LONG  APIENTRY GpiQueryLineEnd(HPS hps);
  BOOL  APIENTRY GpiSetLineJoin(HPS hps, LONG lLineJoin);
  LONG  APIENTRY GpiQueryLineJoin(HPS hps);
  BOOL  APIENTRY GpiSetCurrentPosition(HPS hps, PPOINTL pptlPoint);
  BOOL  APIENTRY GpiQueryCurrentPosition(HPS hps, PPOINTL pptlPoint);
  BOOL  APIENTRY GpiSetArcParams(HPS hps, PARCPARAMS parcpArcParams);
  BOOL  APIENTRY GpiQueryArcParams(HPS hps, PARCPARAMS parcpArcParams);
  LONG  APIENTRY GpiPointArc(HPS hps, PPOINTL pptl2);
  LONG  APIENTRY GpiFullArc(HPS hps, LONG lControl, FIXED fxMultiplier);
  LONG  APIENTRY GpiPartialArc(HPS hps, PPOINTL pptlCenter, FIXED fxMultiplier, FIXED fxStartAngle, FIXED fxSweepAngle);
  LONG  APIENTRY GpiOval(HPS hps, PPOINTL aptlPoints, FIXED fxStartAngle, FIXED fxSweepAngle, ULONG ulOptions);
  LONG  APIENTRY GpiPolyFillet(HPS hps, LONG lCount, PPOINTL aptlPoints);
  LONG  APIENTRY GpiPolySpline(HPS hps, LONG lCount, PPOINTL aptlPoints);
  LONG  APIENTRY GpiPolyFilletSharp(HPS hps, LONG lCount, PPOINTL aptlPoints, PFIXED afxPoints);
  BOOL  APIENTRY GpiSetPatternSet(HPS hps, LONG lSet);
  LONG  APIENTRY GpiQueryPatternSet(HPS hps);
  BOOL  APIENTRY GpiSetPatternRefPoint(HPS hps, PPOINTL pptlRefPoint);
  BOOL  APIENTRY GpiQueryPatternRefPoint(HPS hps, PPOINTL pptlRefPoint);
  BOOL  APIENTRY GpiQueryCharStringPos(HPS hps, ULONG flOptions, LONG lCount, PCH pchString, PLONG alXincrements, PPOINTL aptlPositions);
  BOOL  APIENTRY GpiQueryCharStringPosAt(HPS hps, PPOINTL pptlStart, ULONG flOptions, LONG lCount, PCH pchString, PLONG alXincrements, PPOINTL aptlPositions);
  LONG  APIENTRY GpiQueryTabbedTextExtent(HPS hps, LONG lCount, PCH pchString, LONG lTabCount, PULONG puTabStops);
  BOOL  APIENTRY GpiQueryTextBox(HPS hps, LONG lCount1, PCH pchString, LONG lCount2, PPOINTL aptlPoints);
  BOOL  APIENTRY GpiQueryDefCharBox(HPS hps, PSIZEL psizlSize);
  BOOL  APIENTRY GpiSetCharSet(HPS hps, LONG llcid);
  LONG  APIENTRY GpiQueryCharSet(HPS hps);
  BOOL  APIENTRY GpiSetCharBox(HPS hps, PSIZEF psizfxBox);
  BOOL  APIENTRY GpiQueryCharBox(HPS hps, PSIZEF psizfxSize);
  BOOL  APIENTRY GpiSetCharAngle(HPS hps, PGRADIENTL pgradlAngle);
  BOOL  APIENTRY GpiQueryCharAngle(HPS hps, PGRADIENTL pgradlAngle);
  BOOL  APIENTRY GpiSetCharShear(HPS hps, PPOINTL pptlAngle);
  BOOL  APIENTRY GpiQueryCharShear(HPS hps, PPOINTL pptlShear);
  BOOL  APIENTRY GpiSetCharDirection(HPS hps, LONG lDirection);
  LONG  APIENTRY GpiQueryCharDirection(HPS hps);
  BOOL  APIENTRY GpiSetCharMode(HPS hps, LONG lMode);
  LONG  APIENTRY GpiQueryCharMode(HPS hps);
  BOOL  APIENTRY GpiSetTextAlignment(HPS hps, LONG lHoriz, LONG lVert);
  BOOL  APIENTRY GpiQueryTextAlignment(HPS hps, PLONG plHoriz, PLONG plVert);
  LONG  APIENTRY GpiCharStringPos(HPS hps, PRECTL prclRect, ULONG flOptions, LONG lCount, PCH pchString, PLONG alAdx);
  LONG  APIENTRY GpiTabbedCharStringAt(HPS hps, PPOINTL pPtStart, PRECTL prclRect, ULONG flOptions, LONG lCount, PCH pchString, LONG lTabCount, PULONG puTabStops, LONG lTabOrigin);
  LONG  APIENTRY GpiCharStringPosAt(HPS hps, PPOINTL pptlStart, PRECTL prclRect, ULONG flOptions, LONG lCount, PCH pchString, PLONG alAdx);
  BOOL  APIENTRY GpiSetCharExtra(HPS hps, FIXED Extra);
  BOOL  APIENTRY GpiSetCharBreakExtra(HPS hps, FIXED BreakExtra);
  BOOL  APIENTRY GpiQueryCharExtra(HPS hps, PFIXED Extra);
  BOOL  APIENTRY GpiQueryCharBreakExtra(HPS hps, PFIXED BreakExtra);
  LONG  APIENTRY GpiMarker(HPS hps, PPOINTL pptlPoint);
  LONG  APIENTRY GpiPolyMarker(HPS hps, LONG lCount, PPOINTL aptlPoints);
  BOOL  APIENTRY GpiSetMarker(HPS hps, LONG lSymbol);
  BOOL  APIENTRY GpiSetMarkerBox(HPS hps, PSIZEF psizfxSize);
  BOOL  APIENTRY GpiSetMarkerSet(HPS hps, LONG lSet);
  LONG  APIENTRY GpiQueryMarker(HPS hps);
  BOOL  APIENTRY GpiQueryMarkerBox(HPS hps, PSIZEF psizfxSize);
  LONG  APIENTRY GpiQueryMarkerSet(HPS hps);
  LONG  APIENTRY GpiImage(HPS hps, LONG lFormat, PSIZEL psizlImageSize, LONG lLength, PBYTE pbData);
  BOOL  APIENTRY GpiPop(HPS hps, LONG lCount);
  LONG  APIENTRY GpiPtVisible(HPS hps, PPOINTL pptlPoint);
  LONG  APIENTRY GpiRectVisible(HPS hps, PRECTL prclRectangle);
  BOOL  APIENTRY GpiComment(HPS hps, LONG lLength, PBYTE pbData);

  /* GPILCIDS */
  BOOL  APIENTRY GpiQueryRasterizerCaps(PRASTERIZERCAPS pCaps);
  LONG  APIENTRY GpiCreateLogFont(HPS hps, PSTR8 pName, LONG lLcid, PFATTRS pfatAttrs);
  BOOL  APIENTRY GpiDeleteSetId(HPS hps, LONG lLcid);
  BOOL  APIENTRY GpiLoadFonts(HAB hab, PCSZ pszFilename);
  BOOL  APIENTRY GpiUnloadFonts(HAB hab, PCSZ pszFilename);
  LONG  APIENTRY GpiQueryFonts(HPS hps, ULONG flOptions, PCSZ pszFacename, PLONG plReqFonts, LONG lMetricsLength, PFONTMETRICS afmMetrics);
  BOOL  APIENTRY GpiQueryFontMetrics(HPS hps, LONG lMetricsLength, PFONTMETRICS pfmMetrics);
  LONG  APIENTRY GpiQueryKerningPairs(HPS hps, LONG lCount, PKERNINGPAIRS akrnprData);
  BOOL  APIENTRY GpiQueryWidthTable(HPS hps, LONG lFirstChar, LONG lCount, PLONG alData);
  LONG  APIENTRY GpiQueryNumberSetIds(HPS hps);
  BOOL  APIENTRY GpiQuerySetIds(HPS hps, LONG lCount, PLONG alTypes, PSTR8 aNames, PLONG allcids);
  ULONG APIENTRY GpiQueryFaceString(HPS hps, PCSZ FamilyName, PFACENAMEDESC attrs, LONG length, PCSZ CompoundFaceName);
  BOOL  APIENTRY GpiQueryLogicalFont(HPS hps, LONG lcid, PSTR8 name, PFATTRS attrs, LONG length);
  ULONG APIENTRY GpiQueryFontAction(HAB anchor, ULONG options);
  BOOL  APIENTRY GpiLoadPublicFonts(HAB hab, PCSZ pszFilename);
  BOOL  APIENTRY GpiUnloadPublicFonts(HAB hab, PCSZ pszFilename);
  BOOL  APIENTRY GpiSetCp(HPS hps, ULONG ulCodePage);
  ULONG APIENTRY GpiQueryCp(HPS hps);
  LONG  APIENTRY GpiQueryFullFontFileDescs(HAB hab, PCSZ pszFilename, PLONG plCount, PVOID pNames, PLONG plNamesBuffLength);
  ULONG APIENTRY GpiSetFontMappingFlags(HPS hps, ULONG flags);
  ULONG APIENTRY GpiQueryFontMappingFlags(HPS hps);
  LONG  APIENTRY GpiQueryFontFileDescriptions(HAB hab, PCSZ pszFilename, PLONG plCount, PFFDESCS affdescsNames);

  /* GPIBITMAPS */
  LONG    APIENTRY GpiBitBlt(HPS hpsTarget, HPS hpsSource, LONG lCount, PPOINTL aptlPoints, LONG lRop, ULONG flOptions);
  BOOL    APIENTRY GpiDeleteBitmap(HBITMAP hbm);
  HBITMAP APIENTRY GpiLoadBitmap(HPS hps, HMODULE Resource, ULONG idBitmap, LONG lWidth, LONG lHeight);
  HBITMAP APIENTRY GpiSetBitmap(HPS hps, HBITMAP hbm);
  LONG    APIENTRY GpiWCBitBlt(HPS hpsTarget, HBITMAP hbmSource, LONG lCount, PPOINTL aptlPoints, LONG lRop, ULONG flOptions);
  HBITMAP APIENTRY GpiCreateBitmap(HPS hps, PBITMAPINFOHEADER2 pbmpNew, ULONG flOptions, PBYTE pbInitData, PBITMAPINFO2 pbmiInfoTable);
  LONG    APIENTRY GpiSetBitmapBits(HPS hps, LONG lScanStart, LONG lScans, PBYTE pbBuffer, PBITMAPINFO2 pbmiInfoTable);
  BOOL    APIENTRY GpiSetBitmapDimension(HBITMAP hbm, PSIZEL psizlBitmapDimension);
  BOOL    APIENTRY GpiSetBitmapId(HPS hps, HBITMAP hbm, LONG lLcid);
  LONG    APIENTRY GpiQueryBitmapBits(HPS hps, LONG lScanStart, LONG lScans, PBYTE pbBuffer, PBITMAPINFO2 pbmiInfoTable);
  BOOL    APIENTRY GpiQueryBitmapDimension(HBITMAP hbm, PSIZEL psizlBitmapDimension);
  HBITMAP APIENTRY GpiQueryBitmapHandle(HPS hps, LONG lLcid);
  BOOL    APIENTRY GpiQueryBitmapParameters(HBITMAP hbm, PBITMAPINFOHEADER pbmpData);
  BOOL    APIENTRY GpiQueryBitmapInfoHeader(HBITMAP hbm, PBITMAPINFOHEADER2 pbmpData);
  BOOL    APIENTRY GpiQueryDeviceBitmapFormats(HPS hps, LONG lCount, PLONG alArray);
  LONG    APIENTRY GpiSetPel(HPS hps, PPOINTL pptlPoint);
  LONG    APIENTRY GpiQueryPel(HPS hps, PPOINTL pptlPoint);
  LONG    APIENTRY GpiFloodFill(HPS hps, LONG lOptions, LONG lColor);
  LONG    APIENTRY GpiDrawBits(HPS hps, PVOID pBits, PBITMAPINFO2 pbmiInfoTable, LONG lCount, PPOINTL aptlPoints, LONG lRop, ULONG flOptions);

  /* GPIREGIONS */
  LONG  APIENTRY GpiCombineRegion(HPS hps, HRGN hrgnDest, HRGN hrgnSrc1, HRGN hrgnSrc2, LONG lMode);
  HRGN  APIENTRY GpiCreateRegion(HPS hps, LONG lCount, PRECTL arclRectangles);
  BOOL  APIENTRY GpiDestroyRegion(HPS hps, HRGN hrgn);
  LONG  APIENTRY GpiEqualRegion(HPS hps, HRGN hrgnSrc1, HRGN hrgnSrc2);
  BOOL  APIENTRY GpiOffsetRegion(HPS hps, HRGN hrgn, PPOINTL pptlOffset);
  LONG  APIENTRY GpiPaintRegion(HPS hps, HRGN hrgn);
  LONG  APIENTRY GpiFrameRegion(HPS hps, HRGN hrgn, PSIZEL thickness);
  LONG  APIENTRY GpiPtInRegion(HPS hps, HRGN hrgn, PPOINTL pptlPoint);
  LONG  APIENTRY GpiQueryRegionBox(HPS hps, HRGN hrgn, PRECTL prclBound);
  BOOL  APIENTRY GpiQueryRegionRects(HPS hps, HRGN hrgn, PRECTL prclBound, PRGNRECT prgnrcControl, PRECTL prclRect);
  LONG  APIENTRY GpiRectInRegion(HPS hps, HRGN hrgn, PRECTL prclRect);
  BOOL  APIENTRY GpiSetRegion(HPS hps, HRGN hrgn, LONG lcount, PRECTL arclRectangles);
  LONG  APIENTRY GpiSetClipRegion(HPS hps, HRGN hrgn, PHRGN phrgnOld);
  HRGN  APIENTRY GpiQueryClipRegion(HPS hps);
  LONG  APIENTRY GpiQueryClipBox(HPS hps, PRECTL prclBound);
  LONG  APIENTRY GpiExcludeClipRectangle(HPS hps, PRECTL prclRectangle);
  LONG  APIENTRY GpiIntersectClipRectangle(HPS hps, PRECTL prclRectangle);
  LONG  APIENTRY GpiOffsetClipRegion(HPS hps, PPOINTL pptlPoint);
  HRGN  APIENTRY GpiCreateRoundRectRegion(HPS hps, PPOINTL pptlPoint, LONG lHround, LONG lVRound);
  HRGN  APIENTRY GpiCreateEllipticRegion(HPS hps, PRECTL prclRect);

  /* GPIMETAFILES */
  BOOL  APIENTRY GpiResumePlay(HPS hps);
  BOOL  APIENTRY GpiSuspendPlay(HPS hps);
  HMF   APIENTRY GpiCopyMetaFile(HMF hmf);
  BOOL  APIENTRY GpiDeleteMetaFile(HMF hmf);
  HMF   APIENTRY GpiLoadMetaFile(HAB hab, PCSZ pszFilename);
  LONG  APIENTRY GpiPlayMetaFile(HPS hps, HMF hmf, LONG lCount1, PLONG alOptarray, PLONG plSegCount, LONG lCount2, PCSZ pszDesc);
  BOOL  APIENTRY GpiQueryMetaFileBits(HMF hmf, LONG lOffset, LONG lLength, PBYTE pbData);
  LONG  APIENTRY GpiQueryMetaFileLength(HMF hmf);
  BOOL  APIENTRY GpiSaveMetaFile(HMF hmf, PCSZ pszFilename);
  BOOL  APIENTRY GpiSetMetaFileBits(HMF hmf, LONG lOffset, LONG lLength, PBYTE pbBuffer);

  /* GPIDEFAULTS */
  BOOL  APIENTRY GpiQueryDefArcParams(HPS hps, PARCPARAMS parcpArcParams);
  BOOL  APIENTRY GpiQueryDefAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask, PBUNDLE ppbunAttrs);
  BOOL  APIENTRY GpiQueryDefTag(HPS hps, PLONG plTag);
  BOOL  APIENTRY GpiQueryDefViewingLimits(HPS hps, PRECTL prclLimits);
  BOOL  APIENTRY GpiSetDefArcParams(HPS hps, PARCPARAMS parcpArcParams);
  BOOL  APIENTRY GpiSetDefAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask, PBUNDLE ppbunAttrs);
  BOOL  APIENTRY GpiSetDefTag(HPS hps, LONG lTag);
  BOOL  APIENTRY GpiSetDefViewingLimits(HPS hps, PRECTL prclLimits);

  /* GPIPOLYGON */
  LONG  APIENTRY GpiPolygons(HPS hps, ULONG ulCount, PPOLYGON paplgn, ULONG flOptions, ULONG flModel);
  HRGN  APIENTRY GpiCreatePolygonRegion(HPS hps, ULONG ulCount, PPOLYGON paplgn, ULONG flOptions);

  /* GPIINK */
  BOOL  APIENTRY GpiBeginInkPath(HPS hps, LONG lPath, ULONG flOptions);
  BOOL  APIENTRY GpiEndInkPath(HPS hps, ULONG flOptions);
  LONG  APIENTRY GpiStrokeInkPath(HPS hps, LONG lPath, LONG lCount, PPOINTL aptlPoints, ULONG flOptions);



  /* Server-side placeholder entry points from pmgpi.c
     ("fix prototype !!!"). Real prototypes are unknown yet;
     each is a no-op stub in gpi_manual.cpp. Declaring them here
     gives them extern "C" linkage so .def EXPORTS matches. */
  ULONG APIENTRY GpiSetDCOwner(void);
  ULONG APIENTRY GpiSetBitmapOwner(void);
  ULONG APIENTRY GpiSetRegionOwner(void);
  ULONG APIENTRY GpiInstallIFIFont(void);
  ULONG APIENTRY GpiOpenMetafile(void);
  ULONG APIENTRY GpiCloseMetafile(void);
  ULONG APIENTRY GpiConvPSH(void);
  ULONG APIENTRY GpiMtAssociate(void);
  ULONG APIENTRY GpiMtDisassociate(void);
  ULONG APIENTRY GpiEnableYInversion(void);
  ULONG APIENTRY GpiQueryYInversion(void);
  ULONG APIENTRY GpiSaveMetaFile2(void);
  ULONG APIENTRY GpiAllocateDCData(void);
  ULONG APIENTRY GpiQueryDCData(void);

  /* 32-bit mixed-case Dev* entry points (uppercase DEV* are 16-bit
     thunks in pmgpi16.cpp; these are the plain 32-bit stubs). */
  ULONG APIENTRY DevCloseDC(void);
  ULONG APIENTRY DevEscape(void);
  ULONG APIENTRY DevQueryCaps(void);
  ULONG APIENTRY DevQueryDeviceNames(void);
  ULONG APIENTRY DevQueryHardcopyCaps(void);
  ULONG APIENTRY DevPostDeviceModes(void);
  ULONG APIENTRY DevOpenDC(void);

#ifdef __cplusplus
  }
#endif

#else
  #error Unsupported platform for PMGPI
#endif


#ifdef __cplusplus
      extern "C" {
#endif

#ifdef __cplusplus
BOOL APIENTRY GpiBeginElement(HPS, LONG, PCSZ);
#else
BOOL APIENTRY GpiBeginElement(HPS, LONG, PSZ);
#endif

LONG APIENTRY GpiQueryEditMode(HPS);
BOOL APIENTRY GpiSetElementPointer(HPS, LONG);
LONG APIENTRY GpiQueryElementPointer(HPS);
BOOL APIENTRY GpiOffsetElementPointer(HPS, LONG);
BOOL APIENTRY GpiDeleteElement(HPS);
BOOL APIENTRY GpiDeleteElementRange(HPS, LONG, LONG);
BOOL APIENTRY GpiLabel(HPS, LONG);
BOOL APIENTRY GpiSetElementPointerAtLabel(HPS, LONG);
BOOL APIENTRY GpiDeleteElementsBetweenLabels(HPS, LONG, LONG);

#ifdef __cplusplus
LONG APIENTRY GpiQueryElementType(HPS, PLONG, LONG, PCSZ);
#else
LONG APIENTRY GpiQueryElementType(HPS, PLONG, LONG, PSZ);
#endif

LONG APIENTRY GpiQueryElement(HPS, LONG, LONG, PBYTE);

#ifdef __cplusplus
LONG APIENTRY GpiElement(HPS, LONG, PCSZ, LONG, PBYTE);
#else
LONG APIENTRY GpiElement(HPS, LONG, PSZ, LONG, PBYTE);
#endif

BOOL APIENTRY GpiEndElement(HPS);

BOOL APIENTRY GpiQueryArcParams(HPS, PARCPARAMS);


#ifdef __cplusplus
  }
#endif


#endif /* GPI_PLATFORM_H */
