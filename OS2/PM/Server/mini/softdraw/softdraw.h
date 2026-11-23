#ifndef SOFTDRAW_H
#define SOFTDRAW_H

#include <os2.h>
#include <memory.h>

// --- Коды возврата ---
#define RC_SUCCESS 0
#define RC_ERROR   1

// --- Флаги для ulBltFlags (примеры, основанные на документации) ---
#define BF_SOURCE_TRANSPARENT   0x00000001
#define BF_PATTERN_TRANSPARENT  0x00000002
#define BF_APPLY_BACK_ROP       0x00000004
#define BF_SRC_INVERT           0x00000008
#define BF_PAT_INVERT           0x00000010
#define BF_DST_INVERT           0x00000020
#define BF_SOURCE_LINEAR        0x00000040
#define BF_PATTERN_LINEAR       0x00000080

// --- Флаги для ulType в BMAPINFO ---
#define BMAP_VIDEO_MEMORY       0x00000001
#define BMAP_SYSTEM_MEMORY      0x00000002
#define BMAP_VERTICAL_SCAN      0x00000004
#define BMAP_FONT_GLYPHS        0x00000008

// --- Структура BMAPINFO (описание битмапа) ---
typedef struct _BMAPINFO {
    ULONG  ulLength;
    ULONG  ulType;
    ULONG  ulBytesPerLine;
    ULONG  ulWidth;
    ULONG  ulHeight;
    ULONG  ulBitCount;
    PBYTE  pBits;
    ULONG  ulRsvd[4];
} BMAPINFO, *PBMAPINFO;

// --- Структура BITBLTINFO (полная, согласно документации) ---
typedef struct _BITBLTINFO {
    ULONG      ulLength;
    ULONG      ulBltFlags;
    ULONG      cBlits;
    ULONG      ulROP;
    ULONG      ulMonoBackROP;
    ULONG      ulSrcFGColor;
    ULONG      ulSrcBGColor;
    ULONG      ulPatFGColor;
    ULONG      ulPatBGColor;
    PBYTE      abColors;
    PBMAPINFO  pSrcBmapInfo;
    PBMAPINFO  pDstBmapInfo;
    PBMAPINFO  pPatBmapInfo;
    PPOINTL    aptlSrcOrg;
    PPOINTL    aptlPatOrg;
    PBLTRECT   abrDst;
    PRECTL     prclSrcBounds;
    PRECTL     prclDstBounds;
} BITBLTINFO, *PBITBLTINFO;

// --- Структура LINEINFO (полная, согласно документации) ---
typedef struct _LINEINFO {
    ULONG      ulLength;
    ULONG      ulType;
    ULONG      ulFlags;
    ULONG      ulFGColor;
    ULONG      ulBGColor;
    ULONG      usForeROP;
    ULONG      usBackROP;
    ULONG      ulStyleMask;
    ULONG      ulStyleStep;
    ULONG      ulStyleValue;
    ULONG      cLines;
    PBMAPINFO  pDstBmapInfo;
    POINTL     ptlStart;
    POINTL     ptlEnd;
    POINTL     ptlClipStart;
    POINTL     ptlClipEnd;
    ULONG      lClipStartError;
    // Поля для массива линий (alphLinePack) опущены для краткости,
    // так как обычно обрабатывается cLines и ptlStart/ptlEnd.
} LINEINFO, *PLINEINFO;

// --- Прототипы экспортируемых функций ---
ULONG _System SDEntry(PVOID pInput, PVOID pOutput);
ULONG _System SDBitBlt(PBITBLTINFO pInput, PVOID pOutput);
ULONG _System SDLine(PLINEINFO pInput, PVOID pOutput);

// --- Вспомогательные функции ---
ULONG ApplyROP3(ULONG rop, ULONG src, ULONG pat, ULONG dst);
VOID DrawLineBresenham(PLINEINFO pLine);

#endif // SOFTDRAW_H
