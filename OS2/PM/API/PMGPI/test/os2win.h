/*! @file os2.h
    @brief OS/2 API compatibility header for Win32 builds.

    Provides the subset of OS/2 PM, GPI, and DOS types, constants, and
    function declarations needed by the PMGPI test programs. Only used
    for Win32 builds; OS/2 builds use the real SDK header.

    Do not include <windows.h> here: it defines some of the same names
    (BOOL, HWND, HDC, ...) and the redefinition causes Watcom error
    E125. Instead, declare everything locally. */

#ifndef OS2_COMPAT_H_INCLUDED
#define OS2_COMPAT_H_INCLUDED

#if defined(__OS2__) || defined(__OS2V2__)
#error os2.h compatibility header must not be used on OS/2 builds
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ---------- Base types ---------- */
typedef unsigned long  APIRET;     /*!< OS/2 return code. */
typedef unsigned long  ULONG;      /*!< 32-bit unsigned. */
typedef long           LONG;       /*!< 32-bit signed. */
typedef int            BOOL;       /*!< Boolean. */
typedef unsigned short USHORT;     /*!< 16-bit unsigned. */
typedef short          SHORT;      /*!< 16-bit signed. */
typedef unsigned char  BYTE;       /*!< 8-bit unsigned. */
typedef char           CHAR;       /*!< 8-bit character. */
typedef void          *PVOID;      /*!< Generic pointer. */
typedef long          *PLONG;      /*!< Pointer to LONG. */
typedef unsigned long *PULONG;     /*!< Pointer to ULONG. */
typedef unsigned short FIXED;      /*!< Fixed-point value. */
typedef FIXED         *PFIXED;     /*!< Pointer to FIXED. */

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL ((void*)0)
#endif

/* ---------- PM handle types ---------- */
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

/* ---------- Constants ---------- */
#define HWND_DESKTOP ((HWND)1)   /*!< Desktop window handle. */

#define GPI_ERROR (-1L)          /*!< GPI failure return. */
#define GPI_OK     1L            /*!< GPI success return. */

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

/* Draw control options */
#define DRO_FILL        1L       /*!< Fill only. */
#define DRO_OUTLINE     2L       /*!< Outline only. */
#define DRO_OUTLINEFILL 3L       /*!< Outline and fill. */

/* ---------- Structures ---------- */
typedef struct _POINTL { LONG x; LONG y; } POINTL, *PPOINTL;                 /*!< Point. */
typedef struct _SIZEL  { LONG cx; LONG cy; } SIZEL, *PSIZEL;                 /*!< Size. */
typedef struct _RECTL  { LONG xLeft; LONG yBottom;
                         LONG xRight; LONG yTop; } RECTL, *PRECTL;           /*!< Rectangle. */

/* ---------- Calling conventions ---------- */
/* Same custom calling convention as gpi_platform.h (Win32 branch):
   stdcall stack cleanup, caller cleanup, no name decoration. */
#pragma aux __estdcall "*" \
           parm routine [] \
           value struct struct caller [] \
           modify [eax ecx edx]

#define APIENTRY __declspec(__pragma("__estdcall"))
#define EXPENTRY __declspec(__pragma("__estdcall"))

/* ---------- PM function declarations (implemented in win_compat.c) ---------- */
HAB  APIENTRY WinInitialize(ULONG flOptions);
HMQ  APIENTRY WinCreateMsgQueue(HAB hab, LONG cMsg);
BOOL APIENTRY WinDestroyMsgQueue(HMQ hmq);
BOOL APIENTRY WinTerminate(HAB hab);
HPS  APIENTRY WinGetScreenPS(HWND hwndDesktop);
BOOL APIENTRY WinReleasePS(HPS hps);

/* ---------- GPI function declarations (from PMGPI client library) ---------- */
BOOL APIENTRY GpiSetColor(HPS hps, LONG lColor);
BOOL APIENTRY GpiMove(HPS hps, PPOINTL pptlPoint);
LONG APIENTRY GpiLine(HPS hps, PPOINTL pptlEndPoint);
LONG APIENTRY GpiBox(HPS hps, LONG lControl, PPOINTL pptlPoint,
                     LONG lHRound, LONG lVRound);

#ifdef __cplusplus
}
#endif

#endif /* OS2_COMPAT_H_INCLUDED */
