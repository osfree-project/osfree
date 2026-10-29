/* gpi_platform.h */
#ifndef GPI_PLATFORM_H
#define GPI_PLATFORM_H

#if defined(__OS2__) || defined(__OS2V2__)

  #include <os2.h>

#elif defined(__WIN32__) || defined(_WIN32)

  /* Базовые типы (без <windows.h>!) */
  typedef unsigned long DWORD;
  typedef unsigned int  UINT;
  typedef unsigned long ULONG;
  typedef long          LONG;
  typedef unsigned short USHORT;
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

  /* Хэндлы */
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

  /* Конвенции вызова */
  #ifndef APIENTRY
    #define APIENTRY __stdcall
  #endif
  #ifndef APIENTRY16
    #define APIENTRY16 __stdcall
  #endif

  /* Коды ошибок GPI */
  #ifndef GPI_ERROR
    #define GPI_ERROR (-1L)
  #endif
  #ifndef GPI_OK
    #define GPI_OK 1L
  #endif

  /* Константы выделения памяти */
  #define MEM_COMMIT     0x1000
  #define MEM_RELEASE    0x8000
  #define PAGE_READWRITE 0x04

  /* Объявления Win32 API (вместо <windows.h>) */
  __declspec(dllimport) PVOID __stdcall VirtualAlloc(PVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
  __declspec(dllimport) DWORD __stdcall VirtualFree(PVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
  __declspec(dllimport) HMODULE __stdcall GetModuleHandleA(const char *lpModuleName);
  __declspec(dllimport) PVOID __stdcall GetProcAddress(HMODULE hModule, const char *lpProcName);
  __declspec(dllimport) void __stdcall OutputDebugStringA(const char *lpOutputString);

  /* Эмуляция OS/2 API */
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
  static inline APIRET DosQueryProcAddr(HMODULE hmod, ULONG ord, const char *pszName, PVOID *ppfn) {
      (void)pszName;
      *ppfn = GetProcAddress(hmod, (const char *)(ULONG)ord);
      return (*ppfn != NULL) ? 0 : 1;
  }
  static inline APIRET DosLogWrite(const char *pszMsg) {
      OutputDebugStringA(pszMsg);
      return 0;
  }

  /* Структуры GPI (идентичны OS/2) */
  typedef struct _POINTL { LONG x; LONG y; } POINTL, *PPOINTL;
  typedef struct _SIZEL { LONG cx; LONG cy; } SIZEL, *PSIZEL;
  typedef struct _SIZEF { FIXED cx; FIXED cy; } SIZEF, *PSIZEF;
  typedef struct _RECTL { LONG xLeft; LONG yBottom; LONG xRight; LONG yTop; } RECTL, *PRECTL;
  typedef struct _MATRIXLF { FIXED fxM11; FIXED fxM12; LONG lM13; FIXED fxM21; FIXED fxM22; LONG lM23; LONG lM31; LONG lM32; LONG lM33; } MATRIXLF, *PMATRIXLF;
  typedef struct _ARCPARAMS { LONG lP; LONG lQ; LONG lR; LONG lS; } ARCPARAMS, *PARCPARAMS;
  typedef struct _GRADIENTL { LONG x; LONG y; } GRADIENTL, *PGRADIENTL;

  /* Структуры битмапов */
  typedef struct _RGB { BYTE bBlue; BYTE bGreen; BYTE bRed; BYTE fcOptions; } RGB, *PRGB;
  typedef RGB RGB2, *PRGB2;

  typedef struct _BITMAPINFOHEADER { ULONG cbFix; ULONG cx; ULONG cy; USHORT cPlanes; USHORT cBitCount; } BITMAPINFOHEADER, *PBITMAPINFOHEADER;
  typedef struct _BITMAPINFOHEADER2 { ULONG cbFix; ULONG cx; ULONG cy; USHORT cPlanes; USHORT cBitCount; ULONG ulCompression; ULONG cbImage; ULONG cxResolution; ULONG cyResolution; ULONG cclrUsed; ULONG cclrImportant; USHORT usUnits; USHORT usReserved; USHORT usRecording; USHORT usRendering; ULONG cSize1; ULONG cSize2; ULONG ulColorEncoding; ULONG ulIdentifier; } BITMAPINFOHEADER2, *PBITMAPINFOHEADER2;

  typedef struct _BITMAPINFO { BITMAPINFOHEADER bmpHeader; RGB argbColor[1]; } BITMAPINFO, *PBITMAPINFO;
  typedef struct _BITMAPINFO2 { BITMAPINFOHEADER2 bmpHeader; RGB2 argbColor[1]; } BITMAPINFO2, *PBITMAPINFO2;

  /* Прочие структуры */
  typedef struct _FATTRS { USHORT usRecordLength; USHORT fsSelection; LONG lMatch; CHAR szFacename[32]; USHORT idRegistry; USHORT usCodePage; LONG lMaxBaselineExt; LONG lAveCharWidth; } FATTRS, *PFATTRS;
  typedef struct _FONTMETRICS { CHAR szFamilyname[32]; CHAR szFacename[32]; USHORT usCodepage; USHORT usWeight; USHORT usWidth; } FONTMETRICS, *PFONTMETRICS;
  typedef struct _KERNINGPAIRS { short sFirstChar; short sSecondChar; LONG lKerningAmount; } KERNINGPAIRS, *PKERNINGPAIRS;
  typedef struct _FACENAMEDESC { USHORT usSize; USHORT usWeightClass; USHORT usWidthClass; USHORT usReserved; ULONG flOptions; } FACENAMEDESC, *PFACENAMEDESC;
  typedef struct _RASTERIZERCAPS { ULONG cb; ULONG ulAvailFlags; ULONG ulEnabledFlags; } RASTERIZERCAPS, *PRASTERIZERCAPS;
  typedef struct _POLYGON { ULONG ulPoints; PPOINTL aPointl; } POLYGON, *PPOLYGON;
  typedef struct _POLYSET { ULONG ulPolys; POLYGON aPolygon[1]; } POLYSET, *PPOLYSET;
  typedef struct _RGNRECT { ULONG ircStart; ULONG crc; ULONG crcReturned; ULONG ulDirection; } RGNRECT, *PRGNRECT;
  typedef CHAR FFDESCS[2][32];
  typedef FFDESCS *PFFDESCS;

#else
  #error Unsupported platform for PMGPI
#endif

#endif /* GPI_PLATFORM_H */
