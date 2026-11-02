/* F_platform.hpp */
/* Platform abstraction for PMCLIENT.
   Same PMCLIENT sources build for OS/2 and Win32.
   Only the macros and helpers below are platform-specific. */

#ifndef PMC_PLATFORM_HPP
#define PMC_PLATFORM_HPP

#if defined(__OS2__) || defined(__OS2V2__)

  #define INCL_DOS
  #define INCL_DOSERRORS
  #include <os2.h>

  typedef HMODULE  PMC_HMODULE;
  typedef HMTX     PMC_HMTX;

  #define PMC_ERR_OPEN_FAILED   ERROR_OPEN_FAILED
  #define PMC_ERR_BROKEN_PIPE   ERROR_BROKEN_PIPE

  /* Load a DLL by name. Returns 0 on success. */
  static inline APIRET PMC_LoadModule(const char *pszName, PMC_HMODULE *phmod)
  {
      return DosLoadModule(NULL, 0, (PSZ)pszName, phmod);
  }

  /* Resolve an ordinal in a loaded DLL. Returns 0 on success. */
  static inline APIRET PMC_QueryOrdinal(PMC_HMODULE hmod, ULONG ulOrdinal, void **ppfn)
  {
      PFN    pfn = 0;
      APIRET rc  = DosQueryProcAddr(hmod, ulOrdinal, 0, &pfn);
      *ppfn = (void *)pfn;
      return rc;
  }

  #define PMC_FreeModule(hmod)   DosFreeModule(hmod)

  /* Read an environment variable. On success returns 0 and sets
     *ppszValue to a pointer valid for the process lifetime. */
  static inline APIRET PMC_ScanEnv(const char *pszName, char **ppszValue)
  {
      return DosScanEnv((PSZ)pszName, (PSZ *)ppszValue);
  }

  #define PMC_Sleep(ms)          DosSleep(ms)

  /* Try to open a named system mutex. Returns 0 on success. */
  static inline APIRET PMC_MutexOpen(const char *pszName, PMC_HMTX *phmtx)
  {
      return DosOpenMutexSem((PSZ)pszName, phmtx);
  }

  /* Create a named system mutex. Returns 0 on success. */
  static inline APIRET PMC_CreateMutex(const char *pszName, PMC_HMTX *phmtx)
  {
      return DosCreateMutexSem((PSZ)pszName, phmtx, 0, FALSE);
  }

  #define PMC_MutexClose(h)      DosCloseMutexSem(h)

  /* Start PMSERVER.EXE detached. Returns 0 on success. */
  static inline APIRET PMC_StartProcess(const char *pszExe)
  {
      RESULTCODES rc;
      char        szFail[CCHMAXPATH];
      return DosExecPgm(szFail, sizeof(szFail), EXEC_BACKGROUND,
                        (PSZ)pszExe, NULL, &rc, (PSZ)pszExe);
  }

  #define PMC_HAS_MUTEX_CHECK  1

#elif defined(__WIN32__) || defined(_WIN32)

  /* ================================================================== */
  /*  Hide Windows internals from the rest of the project               */
  /* ================================================================== */

  /* Suppress sub-headers we do not need. _WINGDI_ in particular keeps
   * XFORM, PXFORM, RECT, POINT etc. out of the way. */
  #define _WINGDI_
  #define _WINUSER_
  #define _WINCON_
  #define _DDEHEADER_INCLUDED_
  #define _PRSHT_H_
  #define _INC_DDEMLH
  #define _WINSPOOL_
  #define __oleidl_h__
  #define COM_NO_WINDOWS_H
  #define _OLE2_H_
  #define _INC_COMMDLG
  #define _IMM_

  /* RECTL/RECT on Windows use {left,top,right,bottom}; OS/2 uses
   * {xLeft,yBottom,xRight,yTop}. Rename the Windows tag and both
   * typedefs before <windows.h> so the names are free afterwards,
   * then drop the macros and define the OS/2 layout below. */
  #define _RECTL   WIN_RECTL_TAG
  #define RECTL    WIN_RECTL
  #define PRECTL   PWIN_RECTL
  #include <windows.h>
  #undef _RECTL
  #undef RECTL
  #undef PRECTL

  /* ================================================================== */
  /*  OS/2-compatible types                                             */
  /* ================================================================== */

  /* LONG, ULONG, USHORT, UCHAR, BYTE, BOOL, SHORT, CHAR, POINTL,
   * SIZEL, POINTS, HDC, HANDLE, PVOID — provided by windef.h/winnt.h
   * with the same layout and field names as on OS/2. Not redefined. */

  #ifndef APIRET
    typedef unsigned long APIRET;
  #endif

  typedef char         *PCH;
  typedef char         *PSZ;
  typedef const char   *PCSZ;

  typedef LONG          FIXED;
  typedef ULONG         COLOR;

  typedef void         *HAB;
  typedef void         *HMQ;
  typedef void         *HPS;

  typedef struct _SIZEF     { FIXED cx; FIXED cy; } SIZEF, *PSIZEF;
  typedef struct _ARCPARAMS { LONG lP, lQ, lR, lS; } ARCPARAMS, *PARCPARAMS;

  /* OS/2 layout. Windows versions of the tag and typedefs were
   * renamed to WIN_RECTL_TAG / WIN_RECTL / PWIN_RECTL above. */
  typedef struct _RECTL {
      LONG xLeft;
      LONG yBottom;
      LONG xRight;
      LONG yTop;
  } RECTL, *PRECTL;

  /* GPI foreground bundles (layout identical to <pmgpi.h>). */
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

  typedef struct _MARKERBUNDLE {
      LONG   lColor;
      LONG   lBackColor;
      USHORT usMixMode;
      USHORT usBackMixMode;
      USHORT usSet;
      USHORT usSymbol;
      SIZEF  sizfxCell;
  } MARKERBUNDLE, *PMARKERBUNDLE;

  typedef struct _AREABUNDLE {
      LONG   lColor;
      LONG   lBackColor;
      USHORT usMixMode;
      USHORT usBackMixMode;
      USHORT usSet;
      USHORT usSymbol;
      POINTL ptlRefPoint;
  } AREABUNDLE, *PAREABUNDLE;

  typedef void *PBUNDLE;

  /* Primitive selector codes. */
  #define PRIM_LINE      1L
  #define PRIM_CHAR      2L
  #define PRIM_MARKER    3L
  #define PRIM_AREA      4L
  #define PRIM_IMAGE     5L

  /* Bundle field masks. */
  #define LBB_COLOR          0x0001L
  #define LBB_BACK_COLOR     0x0002L
  #define LBB_MIX_MODE       0x0004L
  #define LBB_BACK_MIX_MODE  0x0008L
  #define LBB_WIDTH          0x0010L
  #define LBB_GEOM_WIDTH     0x0020L
  #define LBB_TYPE           0x0040L
  #define LBB_END            0x0080L
  #define LBB_JOIN           0x0100L

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

  #define MBB_COLOR          0x0001L
  #define MBB_BACK_COLOR     0x0002L
  #define MBB_MIX_MODE       0x0004L
  #define MBB_BACK_MIX_MODE  0x0008L
  #define MBB_SET            0x0010L
  #define MBB_SYMBOL         0x0020L
  #define MBB_BOX            0x0040L

  #define ABB_COLOR          0x0001L
  #define ABB_BACK_COLOR     0x0002L
  #define ABB_MIX_MODE       0x0004L
  #define ABB_BACK_MIX_MODE  0x0008L
  #define ABB_SET            0x0010L
  #define ABB_SYMBOL         0x0020L
  #define ABB_REF_POINT      0x0040L

  /* BeginArea and character-string flags. */
  #define BA_NOBOUNDARY      0L
  #define BA_BOUNDARY        1L
  #define BA_ALTERNATE       0L
  #define BA_WINDING         2L

  #define CHS_CLIP           0x0010L

  /* ================================================================== */
  /*  Platform helpers used by pmserver.cpp                             */
  /* ================================================================== */

  typedef HMODULE  PMC_HMODULE;
  typedef HANDLE   PMC_HMTX;

  #define PMC_ERR_OPEN_FAILED   (-1L)
  #define PMC_ERR_BROKEN_PIPE   (-2L)

  #ifndef APIENTRY
    #define APIENTRY __stdcall
  #endif

  #define MAKEINTRESOURCEA( x )   ((LPSTR)(ULONG_PTR)(WORD)(x))

  static inline APIRET PMC_LoadModule(const char *pszName, PMC_HMODULE *phmod)
  {
      *phmod = LoadLibraryA(pszName);
      return (*phmod != NULL) ? 0 : -1;
  }

  static inline APIRET PMC_QueryOrdinal(PMC_HMODULE hmod, ULONG ulOrdinal,
                                        void **ppfn)
  {
      FARPROC pfn = GetProcAddress(hmod, MAKEINTRESOURCEA(ulOrdinal));
      *ppfn = (void *)pfn;
      return (pfn != NULL) ? 0 : -1;
  }

  #define PMC_FreeModule(hmod)   FreeLibrary(hmod)

  static inline APIRET PMC_ScanEnv(const char *pszName, char **ppszValue)
  {
      static char szBuf[1024];
      DWORD dwLen = GetEnvironmentVariableA(pszName, szBuf, sizeof(szBuf));
      if (dwLen == 0 || dwLen >= sizeof(szBuf)) {
          *ppszValue = NULL;
          return -1;
      }
      *ppszValue = szBuf;
      return 0;
  }

  #define PMC_Sleep(ms)          Sleep(ms)

  static inline APIRET PMC_MutexOpen(const char *pszName, PMC_HMTX *phmtx)
  {
      *phmtx = OpenMutexA(SYNCHRONIZE, FALSE, pszName);
      return (*phmtx != NULL) ? 0 : -1;
  }

  static inline APIRET PMC_CreateMutex(const char *pszName, PMC_HMTX *phmtx)
  {
      *phmtx = CreateMutexA(NULL, FALSE, pszName);
      return (*phmtx != NULL) ? 0 : -1;
  }

  #define PMC_MutexClose(h)      CloseHandle(h)

  static inline APIRET PMC_StartProcess(const char *pszExe)
  {
      STARTUPINFOA        si;
      PROCESS_INFORMATION pi;
      char                szCmd[1024];

      ZeroMemory(&si, sizeof(si));
      si.cb = sizeof(si);
      ZeroMemory(&pi, sizeof(pi));

      lstrcpynA(szCmd, pszExe, sizeof(szCmd));
      if (!CreateProcessA(NULL, szCmd, NULL, NULL, FALSE,
                          DETACHED_PROCESS, NULL, NULL, &si, &pi))
          return -1;

      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      return 0;
  }

  #define PMC_HAS_MUTEX_CHECK  1

  /* BITMAPINFO2 / RGB2 needed by <pmord.h>; not present in the
   * Windows SDK after _WINGDI_ is suppressed. */
  typedef struct _RGB { BYTE bBlue; BYTE bGreen; BYTE bRed; BYTE fcOptions; }
      RGB, *PRGB;
  typedef RGB RGB2, *PRGB2;

  typedef struct _BITMAPINFO2 {
      ULONG cbFix, cx, cy;
      USHORT cPlanes, cBitCount;
      ULONG ulCompression, cbImage, cxResolution, cyResolution,
            cclrUsed, cclrImportant;
      USHORT usUnits, usReserved, usRecording, usRendering;
      ULONG cSize1, cSize2, ulColorEncoding, ulIdentifier;
      RGB2 argbColor[1];
  } BITMAPINFO2, *PBITMAPINFO2;

#else
  #error Unsupported platform for PMCLIENT
#endif

/* Fallback for GPI constants not provided by the platform headers. */
#ifndef GPI_ERROR
  #define GPI_ERROR (0L)
#endif
#ifndef GPI_OK
  #define GPI_OK    (1L)
#endif

#endif /* PMC_PLATFORM_HPP */
