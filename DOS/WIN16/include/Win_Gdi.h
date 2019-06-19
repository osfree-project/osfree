/*  Win_Gdi.h	1.40
    Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

*/
 

#ifndef Win_Gdi__h
#define Win_Gdi__h

/* Gdi typedefs ****************************************************/

#if defined(STRICT) && defined(LONGHANDLES)
typedef void NEAR* HGDIOBJ;
#else
DECLARE_HANDLE(HGDIOBJ);
#endif


/* DC management ***************************************************/
#ifndef NOAPIPROTO

HDC     WINAPI CreateDC(LPCSTR, LPCSTR, LPCSTR, const void FAR*);
HDC     WINAPI CreateIC(LPCSTR, LPCSTR, LPCSTR, const void FAR*);
HDC     WINAPI CreateCompatibleDC(HDC);

BOOL    WINAPI DeleteDC(HDC);

DWORD   WINAPI GetDCOrg(HDC);

int     WINAPI SaveDC(HDC);
BOOL    WINAPI RestoreDC(HDC, int);

int     WINAPI SetEnvironment(LPCSTR, const void FAR*, UINT);
int     WINAPI GetEnvironment(LPCSTR, void FAR*, UINT);

int     WINAPI MulDiv(int, int, int);

/* Drawing bounds accumulation APIs */
UINT    WINAPI SetBoundsRect(HDC hDC, const RECT FAR* lprcBounds, UINT flags);
UINT    WINAPI GetBoundsRect(HDC hDC, RECT FAR* lprcBounds, UINT flags);

#endif      /* NOAPIPROTO */

/* Flags for drawing bounds accumulation API's */
#define DCB_RESET       0x0001
#define DCB_ACCUMULATE  0x0002
#define DCB_DIRTY   DCB_ACCUMULATE
#define DCB_SET     (DCB_RESET | DCB_ACCUMULATE)
#define DCB_ENABLE      0x0004
#define DCB_DISABLE     0x0008


/* Device capabilities *********************************************/

#ifndef NOAPIPROTO

int WINAPI GetDeviceCaps(HDC, int);

#endif      /* NOAPIPROTO */


/* Device Parameters for GetDeviceCaps() */
#define DRIVERVERSION 0
#define TECHNOLOGY    2
#define HORZSIZE      4
#define VERTSIZE      6
#define HORZRES       8
#define VERTRES       10
#define BITSPIXEL     12
#define PLANES        14
#define NUMBRUSHES    16
#define NUMPENS       18
#define NUMMARKERS    20
#define NUMFONTS      22
#define NUMCOLORS     24
#define PDEVICESIZE   26
#define CURVECAPS     28
#define LINECAPS      30
#define POLYGONALCAPS 32
#define TEXTCAPS      34
#define CLIPCAPS      36
#define RASTERCAPS    38
#define ASPECTX       40
#define ASPECTY       42
#define ASPECTXY      44

#define LOGPIXELSX    88
#define LOGPIXELSY    90

#define SIZEPALETTE  104
#define NUMRESERVED  106
#define COLORRES     108

/* Printing related DeviceCaps */

#define PHYSICALWIDTH	110
#define PHYSICALHEIGHT	111
#define PHYSICALOFFSETX	112
#define PHYSICALOFFSETY	113
#define SCALINGFACTORX	114
#define SCALINGFACTORY	115

/* Display driver specific */

#define VREFRESH	116
#define DESKTOPVERTRES	117
#define DESKTOPHORZRES	118
#define BLTALIGNMENT	119

#ifndef NOGDICAPMASKS

/* GetDeviceCaps() return value masks */

/* TECHNOLOGY */
#define DT_PLOTTER          0
#define DT_RASDISPLAY       1
#define DT_RASPRINTER       2
#define DT_RASCAMERA        3
#define DT_CHARSTREAM       4
#define DT_METAFILE         5
#define DT_DISPFILE         6

/* CURVECAPS */
#define CC_NONE             0x0000
#define CC_CIRCLES          0x0001
#define CC_PIE              0x0002
#define CC_CHORD            0x0004
#define CC_ELLIPSES         0x0008
#define CC_WIDE             0x0010
#define CC_STYLED           0x0020
#define CC_WIDESTYLED       0x0040
#define CC_INTERIORS        0x0080
#define CC_ROUNDRECT        0x0100

/* LINECAPS */
#define LC_NONE             0x0000
#define LC_POLYLINE         0x0002
#define LC_MARKER           0x0004
#define LC_POLYMARKER       0x0008
#define LC_WIDE             0x0010
#define LC_STYLED           0x0020
#define LC_WIDESTYLED       0x0040
#define LC_INTERIORS        0x0080

/* POLYGONALCAPS */
#define PC_NONE             0x0000
#define PC_POLYGON          0x0001
#define PC_RECTANGLE        0x0002
#define PC_WINDPOLYGON      0x0004
#define PC_TRAPEZOID	    PC_WINDPOLYGON
#define PC_SCANLINE         0x0008
#define PC_WIDE             0x0010
#define PC_STYLED           0x0020
#define PC_WIDESTYLED       0x0040
#define PC_INTERIORS        0x0080
#define PC_POLYPOLYGON      0x0100	/* (WIN32) */
#define PC_PATHS            0x0200	/* (WIN32) */

/* TEXTCAPS */
#define TC_OP_CHARACTER     0x00000001
#define TC_OP_STROKE        0x00000002
#define TC_CP_STROKE        0x00000004
#define TC_CR_90            0x00000008
#define TC_CR_ANY           0x00000010
#define TC_SF_X_YINDEP      0x00000020
#define TC_SA_DOUBLE        0x00000040
#define TC_SA_INTEGER       0x00000080
#define TC_SA_CONTIN        0x00000100
#define TC_EA_DOUBLE        0x00000200
#define TC_IA_ABLE          0x00000400
#define TC_UA_ABLE          0x00000800
#define TC_SO_ABLE          0x00001000
#define TC_RA_ABLE          0x00002000
#define TC_VA_ABLE          0x00004000
#define TC_RESERVED         0x00008000
#define TC_SCROLLBLT	    0x00010000		/* (WIN32) */

/* CLIPCAPS */
#define CP_NONE             0x0000
#define CP_RECTANGLE        0x0001
#define CP_REGION           0x0002

/* RASTERCAPS */
#define RC_NONE
#define RC_BITBLT           0x0001
#define RC_BANDING          0x0002
#define RC_SCALING          0x0004
#define RC_BITMAP64         0x0008
#define RC_GDI20_OUTPUT     0x0010
#define RC_GDI20_STATE      0x0020
#define RC_SAVEBITMAP       0x0040
#define RC_DI_BITMAP        0x0080
#define RC_PALETTE          0x0100
#define RC_DIBTODEV         0x0200
#define RC_BIGFONT          0x0400
#define RC_STRETCHBLT       0x0800
#define RC_FLOODFILL        0x1000
#define RC_STRETCHDIB       0x2000
#define RC_OP_DX_OUTPUT     0x4000
#define RC_DEVBITS          0x8000

#endif  /* NOGDICAPMASKS */


/* Coordinate transformation management ****************************/

#ifndef NOAPIPROTO

int     WINAPI SetMapMode(HDC, int);
int     WINAPI GetMapMode(HDC);

DWORD   WINAPI SetWindowOrg(HDC, int, int);
DWORD   WINAPI GetWindowOrg(HDC);

DWORD   WINAPI SetWindowExt(HDC, int, int);
DWORD   WINAPI GetWindowExt(HDC);

DWORD   WINAPI OffsetWindowOrg(HDC, int, int);
DWORD   WINAPI ScaleWindowExt(HDC, int, int, int, int);

DWORD   WINAPI SetViewportOrg(HDC, int, int);
DWORD   WINAPI GetViewportOrg(HDC);

DWORD   WINAPI SetViewportExt(HDC, int, int);
DWORD   WINAPI GetViewportExt(HDC);

DWORD   WINAPI OffsetViewportOrg(HDC, int, int);
DWORD   WINAPI ScaleViewportExt(HDC, int, int, int, int);

BOOL    WINAPI SetWindowOrgEx(HDC, int, int, POINT FAR*);
BOOL    WINAPI GetWindowOrgEx(HDC, POINT FAR*);

BOOL    WINAPI SetWindowExtEx(HDC, int, int, SIZE FAR*);
BOOL    WINAPI GetWindowExtEx(HDC, SIZE FAR*);

BOOL    WINAPI OffsetWindowOrgEx(HDC, int, int, POINT FAR*);
BOOL    WINAPI ScaleWindowExtEx(HDC, int, int, int, int, SIZE FAR*);

BOOL    WINAPI SetViewportExtEx(HDC, int, int, SIZE FAR*);
BOOL    WINAPI GetViewportExtEx(HDC, SIZE FAR*);

BOOL    WINAPI SetViewportOrgEx(HDC, int, int, POINT FAR*);
BOOL    WINAPI GetViewportOrgEx(HDC, POINT FAR*);

BOOL    WINAPI OffsetViewportOrgEx(HDC, int, int, POINT FAR*);
BOOL    WINAPI ScaleViewportExtEx(HDC, int, int, int, int, SIZE FAR*);

BOOL    WINAPI DPtoLP(HDC, POINT FAR*, int);
BOOL    WINAPI LPtoDP(HDC, POINT FAR*, int);

#endif      /* NOAPIPROTO */


/* Map modes */
#define MM_TEXT         1
#define MM_LOMETRIC     2
#define MM_HIMETRIC     3
#define MM_LOENGLISH        4
#define MM_HIENGLISH        5
#define MM_TWIPS        6
#define MM_ISOTROPIC        7
#define MM_ANISOTROPIC      8

/* Coordinate Modes */
#define ABSOLUTE    1
#define RELATIVE    2



/* Color management ************************************************/
typedef DWORD COLORREF;

#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)(g)<<8))|(((DWORD)(BYTE)(b))<<16)))

#define GetRValue(rgb)      ((BYTE)(rgb))
#define GetGValue(rgb)      ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)      ((BYTE)((rgb)>>16))

#define WM_SYSCOLORCHANGE   0x0015

#ifndef NOAPIPROTO
COLORREF WINAPI GetNearestColor(HDC, COLORREF);
#endif      /* NOAPIPROTO */


#ifndef NOCOLOR

#ifndef NOAPIPROTO

COLORREF WINAPI GetSysColor(int);
void    WINAPI SetSysColors(int, const int FAR*, const COLORREF FAR*);

#endif      /* NOAPIPROTO */

#define COLOR_SCROLLBAR        0
#define COLOR_BACKGROUND       1
#define COLOR_ACTIVECAPTION    2
#define COLOR_INACTIVECAPTION  3
#define COLOR_MENU             4
#define COLOR_WINDOW           5
#define COLOR_WINDOWFRAME      6
#define COLOR_MENUTEXT         7
#define COLOR_WINDOWTEXT       8
#define COLOR_CAPTIONTEXT      9
#define COLOR_ACTIVEBORDER    10
#define COLOR_INACTIVEBORDER  11
#define COLOR_APPWORKSPACE    12
#define COLOR_HIGHLIGHT       13
#define COLOR_HIGHLIGHTTEXT   14
#define COLOR_BTNFACE         15
#define COLOR_BTNSHADOW       16
#define COLOR_GRAYTEXT        17
#define COLOR_BTNTEXT         18
#define COLOR_INACTIVECAPTIONTEXT 19
#define COLOR_BTNHIGHLIGHT        20

#endif  /* NOCOLOR */



/* GDI object management *******************************************/
#ifndef NOGDIOBJ

#ifdef STRICT
typedef int (CALLBACK* GOBJENUMPROC)(void FAR*, LPARAM);
#else
typedef FARPROC GOBJENUMPROC;
#endif

#ifndef NOAPIPROTO

HGDIOBJ WINAPI GetStockObject(int);

BOOL    WINAPI IsGDIObject(HGDIOBJ);

BOOL    WINAPI DeleteObject(HGDIOBJ);
HGDIOBJ WINAPI SelectObject(HDC, HGDIOBJ);
BOOL    WINAPI UnrealizeObject(HGDIOBJ);
BOOL	WINAPI GetDCOrgEx(HDC,LPPOINT);
int     WINAPI GetObject(HGDIOBJ, int, void FAR*);
DWORD	WINAPI GetObjectType(HGDIOBJ);

#if defined(TWIN32)
/*
 *  For now, without Unicode support, just make GetObjectA an
 *  alias for GetObject.  Do the real thing when we implement
 *  Unicode support.
 */
#define GetObjectA GetObject
#endif /* TWIN32 */

#ifdef STRICT
int     WINAPI EnumObjects(HDC, int, GOBJENUMPROC, LPARAM);
#else
int     WINAPI EnumObjects(HDC, int, GOBJENUMPROC, LPSTR);
#endif

#endif      /* NOAPIPROTO */

/* Object types for EnumObjects() */
#define OBJ_PEN          1
#define OBJ_BRUSH        2
#ifdef TWIN32
#define OBJ_DC           3
#define OBJ_METADC       4
#define OBJ_PAL          5
#define OBJ_FONT         6	/* Warning: conflicts with printer def. */
#define OBJ_BITMAP       7
#define OBJ_REGION       8
#define OBJ_METAFILE     9
#define OBJ_MEMDC       10
#define OBJ_EXTPEN      11
#define OBJ_ENHMETADC   12
#define OBJ_ENHMETAFILE 13
#endif /* TWIN32 */

#endif      /* NOGDIOBJ */


/* Pen support *****************************************************/
#ifndef NOGDIOBJ

/* Logical Pen */
typedef struct tagLOGPEN
{
    UINT    lopnStyle;
    POINT   lopnWidth;
    COLORREF lopnColor;
} LOGPEN;
typedef LOGPEN*       PLOGPEN;
typedef LOGPEN NEAR* NPLOGPEN;
typedef LOGPEN FAR*  LPLOGPEN;


#ifndef NOAPIPROTO

HPEN    WINAPI CreatePen(int, int, COLORREF);
HPEN    WINAPI CreatePenIndirect(LOGPEN FAR*);

#endif      /* NOAPIPROTO */


/* Pen Styles */
#define PS_SOLID	0
#define PS_DASH		1
#define PS_DOT		2
#define PS_DASHDOT	3
#define PS_DASHDOTDOT	4
#define PS_NULL		5
#define PS_INSIDEFRAME	6

/* Stock pens for use with GetStockObject(); */
#define WHITE_PEN       6
#define BLACK_PEN       7
#define NULL_PEN        8


#endif      /* NOGDIOBJ */


/* Brush support ***************************************************/

#ifndef NOGDIOBJ

/* Logical Brush (or Pattern) */
typedef struct tagLOGBRUSH
{
    UINT     lbStyle;
    COLORREF lbColor;
    int      lbHatch;
} LOGBRUSH;
typedef LOGBRUSH*       PLOGBRUSH;
typedef LOGBRUSH NEAR* NPLOGBRUSH;
typedef LOGBRUSH FAR*  LPLOGBRUSH;

typedef LOGBRUSH        PATTERN;
typedef PATTERN*       PPATTERN;
typedef PATTERN NEAR* NPPATTERN;
typedef PATTERN FAR*  LPPATTERN;

#ifndef NOAPIPROTO

HBRUSH  WINAPI CreateSolidBrush(COLORREF);
HBRUSH  WINAPI CreateHatchBrush(int, COLORREF);
HBRUSH  WINAPI CreatePatternBrush(HBITMAP);
HBRUSH  WINAPI CreateDIBPatternBrush(HGLOBAL, UINT);
HBRUSH  WINAPI CreateBrushIndirect(LOGBRUSH FAR*);

DWORD   WINAPI SetBrushOrg(HDC, int, int);
DWORD   WINAPI GetBrushOrg(HDC);

BOOL    WINAPI GetBrushOrgEx(HDC, POINT FAR*);

#endif      /* NOAPIPROTO */

/* Brush Styles */
#define BS_SOLID        0
#define BS_NULL         1
#define BS_HOLLOW       BS_NULL
#define BS_HATCHED      2
#define BS_PATTERN      3
#define BS_INDEXED      4
#define BS_DIBPATTERN       5
#define BS_DIBPATTERNPT	6		/* WIN32 */

/* Hatch Styles */
#define HS_HORIZONTAL       0
#define HS_VERTICAL         1
#define HS_FDIAGONAL        2
#define HS_BDIAGONAL        3
#define HS_CROSS            4
#define HS_DIAGCROSS        5

/* Stock brushes for use with GetStockObject() */
#define WHITE_BRUSH     0
#define LTGRAY_BRUSH    1
#define GRAY_BRUSH      2
#define DKGRAY_BRUSH    3
#define BLACK_BRUSH     4
#define NULL_BRUSH      5
#define HOLLOW_BRUSH    NULL_BRUSH

#endif      /* NOGDIOBJ */

/* (WIN32) GDI Brush ******************************************************* */

#ifndef NOGDIOBJ

#ifndef NOAPIPROTO

HBRUSH	WINAPI CreateDIBPatternBrushPt(CONST VOID *, UINT);
BOOL	WINAPI FixBrushOrgEx();
HBRUSH	WINAPI GetSysColorBrush(int);
BOOL	WINAPI SetBrushOrgEx(HDC, int, int, LPPOINT);

#endif	/* NOAPIPROTO */

#endif	/* NOGDIOBJ */

/* Region management ***********************************************/

#ifndef NOAPIPROTO

HRGN    WINAPI CreateRectRgn(int, int, int, int);
HRGN    WINAPI CreateRectRgnIndirect(const RECT FAR*);
HRGN    WINAPI CreateEllipticRgnIndirect(const RECT FAR*);
HRGN    WINAPI CreateEllipticRgn(int, int, int, int);
HRGN    WINAPI CreatePolygonRgn(const POINT FAR*, int, int);
HRGN    WINAPI CreatePolyPolygonRgn(const POINT FAR*, const int FAR*, int, int);
HRGN    WINAPI CreateRoundRectRgn(int, int, int, int, int, int);

void    WINAPI SetRectRgn(HRGN, int, int, int, int);

int     WINAPI CombineRgn(HRGN, HRGN, HRGN, int);

BOOL    WINAPI EqualRgn(HRGN, HRGN);
int     WINAPI OffsetRgn(HRGN, int, int);

int     WINAPI GetRgnBox(HRGN, RECT FAR*);

BOOL    WINAPI RectInRegion(HRGN, const RECT FAR*);
BOOL    WINAPI PtInRegion(HRGN, int, int);

#endif      /* NOAPIPROTO */


/* Region type flags */
#define ERROR           0
#define NULLREGION      1
#define SIMPLEREGION        2
#define COMPLEXREGION       3

/* CombineRgn() command values */
#define RGN_AND         1
#define RGN_OR          2
#define RGN_XOR         3
#define RGN_DIFF        4
#define RGN_COPY        5

/* Color palette management ****************************************/

#define PALETTERGB(r,g,b)   (0x02000000L | RGB(r,g,b))
#define PALETTEINDEX(i)     ((COLORREF)(0x01000000L | (DWORD)(WORD)(i)))

typedef struct tagPALETTEENTRY
{
    BYTE    peRed;
    BYTE    peGreen;
    BYTE    peBlue;
    BYTE    peFlags;
} PALETTEENTRY;
typedef PALETTEENTRY FAR* LPPALETTEENTRY;

/* Logical Palette */
typedef struct tagLOGPALETTE
{
    WORD    palVersion;
    WORD    palNumEntries;
    PALETTEENTRY palPalEntry[1];
} LOGPALETTE;
typedef LOGPALETTE*       PLOGPALETTE;
typedef LOGPALETTE NEAR* NPLOGPALETTE;
typedef LOGPALETTE FAR*  LPLOGPALETTE;


#ifndef NOAPIPROTO

HPALETTE WINAPI CreatePalette(const LOGPALETTE FAR*);

HPALETTE WINAPI SelectPalette(HDC, HPALETTE, BOOL);

UINT    WINAPI RealizePalette(HDC);

int     WINAPI UpdateColors(HDC);
BOOL	WINAPI AnimatePalette(HPALETTE, UINT, UINT, const PALETTEENTRY FAR*);

UINT    WINAPI SetPaletteEntries(HPALETTE, UINT, UINT, const PALETTEENTRY FAR*);
UINT    WINAPI GetPaletteEntries(HPALETTE, UINT, UINT, PALETTEENTRY FAR*);

UINT    WINAPI GetNearestPaletteIndex(HPALETTE, COLORREF);

BOOL    WINAPI ResizePalette(HPALETTE, UINT);

UINT    WINAPI GetSystemPaletteEntries(HDC, UINT, UINT, PALETTEENTRY FAR*);

UINT    WINAPI GetSystemPaletteUse(HDC);
UINT    WINAPI SetSystemPaletteUse(HDC, UINT);

#endif      /* NOAPIPROTO */


/* Palette entry flags */
#define PC_RESERVED 0x01    /* palette index used for animation */
#define PC_EXPLICIT 0x02    /* palette index is explicit to device */
#define PC_NOCOLLAPSE   0x04    /* do not match color to system palette */

/* Get/SetSystemPaletteUse() values */
#define SYSPAL_ERROR	0
#define SYSPAL_STATIC   1
#define SYSPAL_NOSTATIC 2

/* Palette window messages */
#define WM_QUERYNEWPALETTE  0x030F
#define WM_PALETTEISCHANGING 0x0310
#define WM_PALETTECHANGED   0x0311



/* Clipping support ************************************************/

#ifndef	NOAPIPROTO

int     WINAPI ExtSelectClipRgn(HDC, HRGN, int);
int     WINAPI SelectClipRgn(HDC, HRGN);
int     WINAPI GetClipBox(HDC, RECT FAR*);
int	WINAPI GetClipRgn(HDC, HRGN);
HRGN	WINAPI GetClipRgn16(HDC);
HGDIOBJ WINAPI GetCurrentObject(HDC, UINT);

int     WINAPI IntersectClipRect(HDC, int, int, int, int);
int     WINAPI OffsetClipRgn(HDC, int, int);
int     WINAPI ExcludeClipRect(HDC, int, int, int, int);

BOOL    WINAPI PtVisible(HDC, int, int);
BOOL    WINAPI RectVisible(HDC, const RECT FAR*);

#endif	/* NOAPIPROTO */

/* Drawing support *************************************************/

#ifdef STRICT
typedef void (CALLBACK* LINEDDAPROC)(int, int, LPARAM);
#else
typedef FARPROC LINEDDAPROC;
#endif


#ifndef NOAPIPROTO  /* Prototypes */

DWORD   WINAPI MoveTo(HDC, int, int);
DWORD   WINAPI GetCurrentPosition(HDC);

BOOL    WINAPI MoveToEx(HDC, int, int, POINT FAR*);
BOOL    WINAPI GetCurrentPositionEx(HDC, POINT FAR*);

BOOL    WINAPI LineTo(HDC, int, int);
BOOL    WINAPI Polyline(HDC, const POINT FAR*, int);

void    WINAPI LineDDA(int, int, int, int, LINEDDAPROC, LPARAM);

BOOL    WINAPI Rectangle(HDC, int, int, int, int);
BOOL    WINAPI RoundRect(HDC, int, int, int, int, int, int);

BOOL    WINAPI Ellipse(HDC, int, int, int, int);
BOOL    WINAPI Arc(HDC, int, int, int, int, int, int, int, int);
BOOL    WINAPI Chord(HDC, int, int, int, int, int, int, int, int);
BOOL    WINAPI Pie(HDC, int, int, int, int, int, int, int, int);

BOOL    WINAPI Polygon(HDC, const POINT FAR*, int);
BOOL    WINAPI PolyPolygon(HDC, const POINT FAR*, int FAR*, int);

int     WINAPI SetPolyFillMode(HDC, int);
int     WINAPI GetPolyFillMode(HDC);

BOOL    WINAPI FloodFill(HDC, int, int, COLORREF);
BOOL    WINAPI ExtFloodFill(HDC, int, int, COLORREF, UINT);

BOOL    WINAPI FillRgn(HDC, HRGN, HBRUSH);
BOOL    WINAPI FrameRgn(HDC, HRGN, HBRUSH, int, int);
BOOL    WINAPI InvertRgn(HDC, HRGN);
BOOL    WINAPI PaintRgn(HDC, HRGN);

int     WINAPI FillRect(HDC, const RECT FAR*, HBRUSH);
int     WINAPI FrameRect(HDC, const RECT FAR*, HBRUSH);
void    WINAPI InvertRect(HDC, const RECT FAR*);

void    WINAPI DrawFocusRect(HDC, const RECT FAR*);

#endif              /* NOAPIPROTO */


/* PolyFill Modes */
#define ALTERNATE   1
#define WINDING     2

/* ExtFloodFill style flags */
#define  FLOODFILLBORDER   0
#define  FLOODFILLSURFACE  1




/* Text support *************************************************/

#ifdef STRICT
typedef BOOL (CALLBACK* GRAYSTRINGPROC)(HDC, LPARAM, int);
#else
typedef FARPROC GRAYSTRINGPROC;
#endif


#ifndef NOAPIPROTO  /* Prototypes */

BOOL    WINAPI TextOut(HDC, int, int, LPCSTR, int);
LONG    WINAPI TabbedTextOut(HDC, int, int, LPCSTR, int, int, int FAR*, int);
BOOL    WINAPI ExtTextOut(HDC, int, int, UINT, const RECT FAR*, LPCSTR, UINT, int FAR*);

#ifndef NODRAWTEXT
int     WINAPI DrawText(HDC, LPCSTR, int, RECT FAR*, UINT);
#endif  /* NODRAWTEXT */

DWORD   WINAPI GetTextExtent(HDC, LPCSTR, int);
DWORD   WINAPI GetTabbedTextExtent(HDC, LPCSTR, int, int, int FAR*);

BOOL    WINAPI GetTextExtentPoint(HDC, LPCSTR, int, SIZE FAR*);
#define GetTextExtentPoint32 GetTextExtentPoint

BOOL    WINAPI GrayString(HDC, HBRUSH, GRAYSTRINGPROC, LPARAM, int, int, int, int, int);

BOOL    WINAPI GetCharWidth(HDC, UINT, UINT, int FAR*);

COLORREF WINAPI SetTextColor(HDC, COLORREF);
COLORREF WINAPI GetTextColor(HDC);

COLORREF WINAPI SetBkColor(HDC, COLORREF);
COLORREF WINAPI GetBkColor(HDC);

int     WINAPI SetBkMode(HDC, int);
int     WINAPI GetBkMode(HDC);

UINT    WINAPI SetTextAlign(HDC, UINT);
UINT    WINAPI GetTextAlign(HDC);

int     WINAPI SetTextCharacterExtra(HDC, int);
int     WINAPI GetTextCharacterExtra(HDC);

int     WINAPI SetTextJustification(HDC, int, int);

#endif              /* NOAPIPROTO */


#define ETO_GRAYED  0x0001
#define ETO_OPAQUE  0x0002
#define ETO_CLIPPED 0x0004

/* Background Modes */
#if defined TRANSPARENT
#undef TRANSPARENT
#endif
#define TRANSPARENT     1
#define OPAQUE          2

/* Text Alignment Options */
#define TA_NOUPDATECP            0x0000
#define TA_UPDATECP          0x0001
#define TA_LEFT              0x0000
#define TA_RIGHT             0x0002
#define TA_CENTER            0x0006
#define TA_TOP               0x0000
#define TA_BOTTOM            0x0008
#define TA_BASELINE          0x0018

#define VTA_BASELINE TA_BASELINE
#define VTA_LEFT     TA_BOTTOM
#define VTA_RIGHT    TA_TOP
#define VTA_CENTER   TA_CENTER
#define VTA_BOTTOM   TA_RIGHT
#define VTA_TOP      TA_LEFT

/* DrawText() Format Flags */
#ifndef NODRAWTEXT

#define DT_TOP          0x0000
#define DT_LEFT         0x0000
#define DT_CENTER       0x0001
#define DT_RIGHT        0x0002
#define DT_VCENTER      0x0004
#define DT_BOTTOM       0x0008
#define DT_WORDBREAK        0x0010
#define DT_SINGLELINE       0x0020
#define DT_EXPANDTABS       0x0040
#define DT_TABSTOP      0x0080
#define DT_NOCLIP       0x0100
#define DT_EXTERNALLEADING  0x0200
#define DT_CALCRECT     0x0400
#define DT_NOPREFIX     0x0800
#define DT_INTERNAL     0x1000

#endif  /* NODRAWTEXT */




/* Font support *************************************************/
#ifndef NOGDIOBJ

/* Logical Font */
#define LF_FACESIZE     32
typedef struct tagLOGFONT
{
    int     lfHeight;
    int     lfWidth;
    int     lfEscapement;
    int     lfOrientation;
    int     lfWeight;
    BYTE    lfItalic;
    BYTE    lfUnderline;
    BYTE    lfStrikeOut;
    BYTE    lfCharSet;
    BYTE    lfOutPrecision;
    BYTE    lfClipPrecision;
    BYTE    lfQuality;
    BYTE    lfPitchAndFamily;
    char    lfFaceName[LF_FACESIZE];
} LOGFONT;
typedef LOGFONT*       PLOGFONT;
typedef LOGFONT NEAR* NPLOGFONT;
typedef LOGFONT FAR*  LPLOGFONT;


#ifndef NOAPIPROTO  /* Prototypes */

HFONT   WINAPI CreateFont(int, int, int, int, int, BYTE, BYTE, BYTE, BYTE, BYTE, BYTE, BYTE, BYTE, LPCSTR);
HFONT   WINAPI CreateFontIndirect(const LOGFONT FAR*);

DWORD   WINAPI SetMapperFlags(HDC, DWORD);

int     WINAPI AddFontResource(LPCSTR);
BOOL    WINAPI RemoveFontResource(LPCSTR);

int     WINAPI GetTextFace(HDC, int, LPSTR);

DWORD   WINAPI GetAspectRatioFilter(HDC);

BOOL    WINAPI GetAspectRatioFilterEx(HDC, SIZE FAR*);

#endif              /* NOAPIPROTO */


/* weight values */
#define FW_DONTCARE     0
#define FW_THIN         100
#define FW_EXTRALIGHT       200
#define FW_LIGHT        300
#define FW_NORMAL       400
#define FW_MEDIUM       500
#define FW_SEMIBOLD     600
#define FW_BOLD         700
#define FW_EXTRABOLD        800
#define FW_HEAVY        900

#define FW_ULTRALIGHT       FW_EXTRALIGHT
#define FW_REGULAR      FW_NORMAL
#define FW_DEMIBOLD     FW_SEMIBOLD
#define FW_ULTRABOLD        FW_EXTRABOLD
#define FW_BLACK        FW_HEAVY

/* CharSet values */
#define ANSI_CHARSET        0
#define DEFAULT_CHARSET     1
#define SYMBOL_CHARSET      2
#define SHIFTJIS_CHARSET    128
#define HANGEUL_CHARSET     129
#define CHINESEBIG5_CHARSET 136
#define OEM_CHARSET         255

/* OutPrecision values */
#define OUT_DEFAULT_PRECIS      0
#define OUT_STRING_PRECIS       1
#define OUT_CHARACTER_PRECIS    2
#define OUT_STROKE_PRECIS       3
#define OUT_TT_PRECIS           4
#define OUT_DEVICE_PRECIS       5
#define OUT_RASTER_PRECIS       6
#define OUT_TT_ONLY_PRECIS      7

/* ClipPrecision values */
#define CLIP_DEFAULT_PRECIS     0x00
#define CLIP_CHARACTER_PRECIS   0x01
#define CLIP_STROKE_PRECIS      0x02
#define CLIP_MASK               0x0F
#define CLIP_LH_ANGLES          0x10
#define CLIP_TT_ALWAYS          0x20
#define CLIP_EMBEDDED           0x80

/* Quality values */
#define DEFAULT_QUALITY     0
#define DRAFT_QUALITY       1
#define PROOF_QUALITY       2

/* PitchAndFamily pitch values (low 4 bits) */
#define DEFAULT_PITCH       0x00
#define FIXED_PITCH         0x01
#define VARIABLE_PITCH      0x02

/* PitchAndFamily family values (high 4 bits) */
#define FF_DONTCARE         0x00
#define FF_ROMAN            0x10
#define FF_SWISS            0x20
#define FF_MODERN           0x30
#define FF_SCRIPT           0x40
#define FF_DECORATIVE       0x50

/* Stock fonts for use with GetStockObject() */
#define OEM_FIXED_FONT		10
#define ANSI_FIXED_FONT		11
#define ANSI_VAR_FONT		12
#define SYSTEM_FONT		13
#define DEVICE_DEFAULT_FONT	14
#define DEFAULT_PALETTE		15
#define SYSTEM_FIXED_FONT	16

/* Stock fonts for use with GetStockObject() (WINVER >= 0x0400) */

#define DEFAULT_GUI_FONT	17

#if (WINVER >= 0x0400)
#define STOCK_LAST		17
#else
#define STOCK_LAST		16
#endif

/* misc font */

#define ASPECT_FILTERING	0x00000001L

#define WM_FONTCHANGE		0x001D

#endif  /* NOGDIOBJ */

/* Font metric support ******************************************/

#ifndef NOTEXTMETRIC

#if 0
typedef struct tagTEXTMETRIC
{
    int     tmHeight;
    int     tmAscent;
    int     tmDescent;
    int     tmInternalLeading;
    int     tmExternalLeading;
    int     tmAveCharWidth;
    int     tmMaxCharWidth;
    int     tmWeight;
    BYTE    tmItalic;
    BYTE    tmUnderlined;
    BYTE    tmStruckOut;
    BYTE    tmFirstChar;
    BYTE    tmLastChar;
    BYTE    tmDefaultChar;
    BYTE    tmBreakChar;
    BYTE    tmPitchAndFamily;
    BYTE    tmCharSet;
    int     tmOverhang;
    int     tmDigitizedAspectX;
    int     tmDigitizedAspectY;
} TEXTMETRIC;
typedef TEXTMETRIC*       PTEXTMETRIC;
typedef TEXTMETRIC NEAR* NPTEXTMETRIC;
typedef TEXTMETRIC FAR*  LPTEXTMETRIC;
#endif	/* 0 */

typedef struct tagTEXTMETRICA
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    BYTE        tmFirstChar;
    BYTE        tmLastChar;
    BYTE        tmDefaultChar;
    BYTE        tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
} TEXTMETRICA;
typedef TEXTMETRICA		*PTEXTMETRICA;
typedef TEXTMETRICA NEAR	*NPTEXTMETRICA;
typedef TEXTMETRICA FAR		*LPTEXTMETRICA;

typedef struct tagTEXTMETRICW
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    WCHAR       tmFirstChar;
    WCHAR       tmLastChar;
    WCHAR       tmDefaultChar;
    WCHAR       tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
} TEXTMETRICW;
typedef TEXTMETRICW		*PTEXTMETRICW;
typedef TEXTMETRICW NEAR	*NPTEXTMETRICW;
typedef TEXTMETRICW FAR		*LPTEXTMETRICW;

#ifdef UNICODE
typedef TEXTMETRICW	TEXTMETRIC;
typedef PTEXTMETRICW	PTEXTMETRIC;
typedef NPTEXTMETRICW	NPTEXTMETRIC;
typedef LPTEXTMETRICW	LPTEXTMETRIC;
#else
typedef TEXTMETRICA	TEXTMETRIC;
typedef PTEXTMETRICA	PTEXTMETRIC;
typedef NPTEXTMETRICA	NPTEXTMETRIC;
typedef LPTEXTMETRICA	LPTEXTMETRIC;
#endif	/* UNICODE */

#ifndef NOAPIPROTO  /* Prototypes */

BOOL    WINAPI GetTextMetrics(HDC, TEXTMETRIC FAR*);

#endif              /* NOAPIPROTO */


/* tmPitchAndFamily values */
#define TMPF_FIXED_PITCH    0x01
#define TMPF_VECTOR     0x02
#define TMPF_DEVICE     0x08
#define TMPF_TRUETYPE       0x04


#endif      /* NOTEXTMETRIC */



/* Font scalable font metric support ****************************/
#ifndef NOTEXTMETRIC
#ifndef NOSCALABLEFONT

typedef struct tagPANOSE
{
    BYTE    bFamilyType;
    BYTE    bSerifStyle;
    BYTE    bWeight;
    BYTE    bProportion;
    BYTE    bContrast;
    BYTE    bStrokeVariation;
    BYTE    bArmStyle;
    BYTE    bLetterform;
    BYTE    bMidline;
    BYTE    bXHeight;
} PANOSE, FAR* LPPANOSE;

typedef struct tagOUTLINETEXTMETRIC
{
    UINT    otmSize;
    TEXTMETRIC otmTextMetrics;
    BYTE    otmFiller;
    PANOSE  otmPanoseNumber;
    UINT    otmfsSelection;
    UINT    otmfsType;
    int     otmsCharSlopeRise;
    int     otmsCharSlopeRun;
    int     otmItalicAngle;
    UINT    otmEMSquare;
    int     otmAscent;
    int     otmDescent;
    UINT    otmLineGap;
    UINT    otmsCapEmHeight;
    UINT    otmsXHeight;
    RECT    otmrcFontBox;
    int     otmMacAscent;
    int     otmMacDescent;
    UINT    otmMacLineGap;
    UINT    otmusMinimumPPEM;
    POINT   otmptSubscriptSize;
    POINT   otmptSubscriptOffset;
    POINT   otmptSuperscriptSize;
    POINT   otmptSuperscriptOffset;
    UINT    otmsStrikeoutSize;
    int     otmsStrikeoutPosition;
    int     otmsUnderscorePosition;
    int     otmsUnderscoreSize;
    PSTR    otmpFamilyName;
    PSTR    otmpFaceName;
    PSTR    otmpStyleName;
    PSTR    otmpFullName;
} OUTLINETEXTMETRIC, FAR* LPOUTLINETEXTMETRIC;


#ifndef NOAPIPROTO  /* Prototypes */

WORD    WINAPI GetOutlineTextMetrics(HDC, UINT, OUTLINETEXTMETRIC FAR*);

#endif              /* NOAPIPROTO */


#endif      /* NOSCALABLEFONT */
#endif      /* NOTEXTMETRIC */


/* Font new font metric support *********************************/
#ifndef NOTEXTMETRIC
#ifndef NOGDIOBJ

#if 0

/* NOTE: NEWTEXTMETRIC is the same as TEXTMETRIC plus 4 new fields */
typedef struct tagNEWTEXTMETRIC
{
    int     tmHeight;
    int     tmAscent;
    int     tmDescent;
    int     tmInternalLeading;
    int     tmExternalLeading;
    int     tmAveCharWidth;
    int     tmMaxCharWidth;
    int     tmWeight;
    BYTE    tmItalic;
    BYTE    tmUnderlined;
    BYTE    tmStruckOut;
    BYTE    tmFirstChar;
    BYTE    tmLastChar;
    BYTE    tmDefaultChar;
    BYTE    tmBreakChar;
    BYTE    tmPitchAndFamily;
    BYTE    tmCharSet;
    int     tmOverhang;
    int     tmDigitizedAspectX;
    int     tmDigitizedAspectY;
    DWORD   ntmFlags;
    UINT    ntmSizeEM;
    UINT    ntmCellHeight;
    UINT    ntmAvgWidth;
} NEWTEXTMETRIC;
typedef NEWTEXTMETRIC*       PNEWTEXTMETRIC;
typedef NEWTEXTMETRIC NEAR* NPNEWTEXTMETRIC;
typedef NEWTEXTMETRIC FAR*  LPNEWTEXTMETRIC;

#endif /* 0 */

#ifndef UNICODE_ONLY
typedef struct tagNEWTEXTMETRICA
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    BYTE        tmFirstChar;
    BYTE        tmLastChar;
    BYTE        tmDefaultChar;
    BYTE        tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
    DWORD   ntmFlags;
    UINT    ntmSizeEM;
    UINT    ntmCellHeight;
    UINT    ntmAvgWidth;
} NEWTEXTMETRICA, *PNEWTEXTMETRICA, *NPNEWTEXTMETRICA, *LPNEWTEXTMETRICA;
#endif /* !UNICODE_ONLY */
#ifndef ANSI_ONLY
typedef struct tagNEWTEXTMETRICW
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    WCHAR       tmFirstChar;
    WCHAR       tmLastChar;
    WCHAR       tmDefaultChar;
    WCHAR       tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
    DWORD   ntmFlags;
    UINT    ntmSizeEM;
    UINT    ntmCellHeight;
    UINT    ntmAvgWidth;
} NEWTEXTMETRICW, *PNEWTEXTMETRICW, *NPNEWTEXTMETRICW, *LPNEWTEXTMETRICW;
#endif /* !ANSI_ONLY */
#ifdef UNICODE
typedef NEWTEXTMETRICW NEWTEXTMETRIC;
typedef PNEWTEXTMETRICW PNEWTEXTMETRIC;
typedef NPNEWTEXTMETRICW NPNEWTEXTMETRIC;
typedef LPNEWTEXTMETRICW LPNEWTEXTMETRIC;
#else
typedef NEWTEXTMETRICA NEWTEXTMETRIC;
typedef PNEWTEXTMETRICA PNEWTEXTMETRIC;
typedef NPNEWTEXTMETRICA NPNEWTEXTMETRIC;
typedef LPNEWTEXTMETRICA LPNEWTEXTMETRIC;
#endif /* UNICODE */

#define LF_FULLFACESIZE     64

/* Structure passed to FONTENUMPROC */
typedef struct tagENUMLOGFONT
{
    LOGFONT elfLogFont;
    char    elfFullName[LF_FULLFACESIZE];
    char    elfStyle[LF_FACESIZE];
} ENUMLOGFONT, FAR* LPENUMLOGFONT;


#ifndef NOAPIPROTO  /* Prototypes */

#endif              /* NOAPIPROTO */

/* ntmFlags field flags */
#define NTM_REGULAR 0x00000040L
#define NTM_BOLD    0x00000020L
#define NTM_ITALIC  0x00000001L

#endif      /* NOGDIOBJ */
#endif      /* NOTEXTMETRIC */


/* Font enumeration support *************************************/
#ifndef NOTEXTMETRIC

#ifndef NOGDIOBJ

#ifdef STRICT

typedef int (CALLBACK* FONTENUMPROC)(const ENUMLOGFONT FAR*, const NEWTEXTMETRIC FAR*, int, LPARAM);
#ifdef TWIN32
typedef FONTENUMPROC OLDFONTENUMPROC;
#else
typedef int (CALLBACK* OLDFONTENUMPROC)(const LOGFONT FAR*, const TEXTMETRIC FAR*, int, LPARAM);
#endif /* TWIN32 */

#else   /* STRICT */

typedef FARPROC OLDFONTENUMPROC;
typedef FARPROC FONTENUMPROC;

#endif  /* !STRICT */

#ifndef NOAPIPROTO  /* Prototypes */
int     WINAPI EnumFonts(HDC, LPCSTR, OLDFONTENUMPROC, LPARAM);
int     WINAPI EnumFontFamilies(HDC, LPCSTR, FONTENUMPROC, LPARAM);
#endif  /* NOAPIPROTO */


/* EnumFonts font type values */
#define RASTER_FONTTYPE     0x0001
#define DEVICE_FONTTYPE     0X0002
#define TRUETYPE_FONTTYPE   0x0004

#endif  /* NOGDIOBJ */

#endif  /* NOTEXTMETRIC */



/* Font glyph support *******************************************/
#ifndef NOSCALABLEFONT

typedef struct tagGLYPHMETRICS
{
    UINT    gmBlackBoxX;
    UINT    gmBlackBoxY;
    POINT   gmptGlyphOrigin;
    int     gmCellIncX;
    int     gmCellIncY;
} GLYPHMETRICS, FAR* LPGLYPHMETRICS;

typedef struct tagFIXED
{
    UINT    fract;
    int     value;
} FIXED, FAR* LPFIXED;

typedef struct tagMAT2
{
    FIXED  eM11;
    FIXED  eM12;
    FIXED  eM21;
    FIXED  eM22;
} MAT2, FAR* LPMAT2;

typedef struct tagPOINTFX
{
    FIXED x;
    FIXED y;
} POINTFX, FAR* LPPOINTFX;

typedef struct tagTTPOLYCURVE
{
    UINT    wType;
    UINT    cpfx;
    POINTFX apfx[1];
} TTPOLYCURVE, FAR* LPTTPOLYCURVE;

typedef struct tagTTPOLYGONHEADER
{
    DWORD   cb;
    DWORD   dwType;
    POINTFX pfxStart;
} TTPOLYGONHEADER, FAR* LPTTPOLYGONHEADER;

typedef struct tagABC
{
    int   abcA;
    UINT  abcB;
    int   abcC;
} ABC;
typedef ABC FAR* LPABC;

typedef struct _ABCFLOAT
{
    FLOAT abcfA;
    FLOAT abcfB;
    FLOAT abcfC;
} ABCFLOAT;
typedef ABCFLOAT	*PABCFLOAT;
typedef ABCFLOAT NEAR	*NPABCFLOAT;
typedef ABCFLOAT FAR	*LPABCFLOAT;

typedef struct tagKERNINGPAIR
{
    WORD wFirst;
    WORD wSecond;
    int  iKernAmount;
} KERNINGPAIR, FAR* LPKERNINGPAIR;

typedef struct tagRASTERIZER_STATUS
{
    int   nSize;
    int   wFlags;
    int   nLanguageID;
} RASTERIZER_STATUS;
typedef RASTERIZER_STATUS FAR* LPRASTERIZER_STATUS;


#ifndef NOAPIPROTO  /* Prototypes */

DWORD   WINAPI GetFontData(HDC, DWORD, DWORD, void FAR*, DWORD);
BOOL    WINAPI CreateScalableFontResource(UINT, LPCSTR, LPCSTR, LPCSTR);

DWORD   WINAPI GetGlyphOutline(HDC, UINT, UINT, GLYPHMETRICS FAR*, DWORD, void FAR*, const MAT2 FAR*);

BOOL    WINAPI GetCharABCWidths(HDC, UINT, UINT, ABC FAR*);

int     WINAPI GetKerningPairs(HDC, int, KERNINGPAIR FAR*);

BOOL    WINAPI GetRasterizerCaps(RASTERIZER_STATUS FAR*, int);

#endif              /* NOAPIPROTO */


/* GetGlyphOutline constants */
#define GGO_METRICS        0
#define GGO_BITMAP         1
#define GGO_NATIVE         2

#define TT_POLYGON_TYPE   24

#define TT_PRIM_LINE       1
#define TT_PRIM_QSPLINE    2

/* bits defined in wFlags of RASTERIZER_STATUS */
#define TT_AVAILABLE    0x0001
#define TT_ENABLED  0x0002


#endif      /* NOSCALABLEFONT */



/* Bitmap management ********************************************/
#ifndef NOBITMAP

typedef struct tagBITMAP
{
    int     bmType;
    int     bmWidth;
    int     bmHeight;
    int     bmWidthBytes;
    BYTE    bmPlanes;
    BYTE    bmBitsPixel;
    void FAR* bmBits;
} BITMAP;
typedef BITMAP*       PBITMAP;
typedef BITMAP NEAR* NPBITMAP;
typedef BITMAP FAR*  LPBITMAP;

/* Bitmap Header structures */
typedef struct tagRGBTRIPLE
{
    BYTE    rgbtBlue;
    BYTE    rgbtGreen;
    BYTE    rgbtRed;
} RGBTRIPLE;
typedef RGBTRIPLE FAR* LPRGBTRIPLE;

typedef struct tagRGBQUAD
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} RGBQUAD;
typedef RGBQUAD FAR* LPRGBQUAD;

/* structures for defining DIBs */
typedef struct tagBITMAPCOREHEADER
{
    DWORD   bcSize;
    short   bcWidth;
    short   bcHeight;
    WORD    bcPlanes;
    WORD    bcBitCount;
} BITMAPCOREHEADER;
typedef BITMAPCOREHEADER*      PBITMAPCOREHEADER;
typedef BITMAPCOREHEADER FAR* LPBITMAPCOREHEADER;

typedef struct tagBITMAPINFOHEADER
{
    DWORD   biSize;
    LONG    biWidth;
    LONG    biHeight;
    WORD    biPlanes;
    WORD    biBitCount;
    DWORD   biCompression;
    DWORD   biSizeImage;
    LONG    biXPelsPerMeter;
    LONG    biYPelsPerMeter;
    DWORD   biClrUsed;
    DWORD   biClrImportant;
} BITMAPINFOHEADER;
typedef BITMAPINFOHEADER*      PBITMAPINFOHEADER;
typedef BITMAPINFOHEADER FAR* LPBITMAPINFOHEADER;

typedef struct tagBITMAPINFO
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD      bmiColors[1];
} BITMAPINFO;
typedef BITMAPINFO*     PBITMAPINFO;
typedef BITMAPINFO FAR* LPBITMAPINFO;

typedef struct tagBITMAPCOREINFO
{
    BITMAPCOREHEADER bmciHeader;
    RGBTRIPLE        bmciColors[1];
} BITMAPCOREINFO;
typedef BITMAPCOREINFO*      PBITMAPCOREINFO;
typedef BITMAPCOREINFO FAR* LPBITMAPCOREINFO;

typedef struct tagBITMAPFILEHEADER
{
    WORD    bfType;
    DWORD   bfSize;
    WLS_UINT    bfReserved1;
    WLS_UINT    bfReserved2;
    DWORD   bfOffBits;
} BITMAPFILEHEADER;
typedef BITMAPFILEHEADER*      PBITMAPFILEHEADER;
typedef BITMAPFILEHEADER FAR* LPBITMAPFILEHEADER;


#ifndef NOAPIPROTO  /* Prototypes */

HBITMAP WINAPI CreateBitmap(int, int, UINT, UINT, const void FAR*);
HBITMAP WINAPI CreateBitmapIndirect(BITMAP FAR* );
HBITMAP WINAPI CreateCompatibleBitmap(HDC, int, int);
HBITMAP WINAPI CreateDiscardableBitmap(HDC, int, int);
#ifdef TWIN32
HBITMAP WINAPI CreateDIBitmap(HDC, const BITMAPINFOHEADER *,DWORD, const void *, const BITMAPINFO *,UINT);
#else
HBITMAP WINAPI CreateDIBitmap(HDC, BITMAPINFOHEADER FAR*, DWORD, const void FAR*, BITMAPINFO FAR*, UINT);
#endif

HBITMAP WINAPI LoadBitmap(HINSTANCE, LPCSTR);

BOOL    WINAPI BitBlt(HDC, int, int, int, int, HDC, int, int, DWORD);

BOOL    WINAPI PatBlt(HDC, int, int, int, int, DWORD);

BOOL    WINAPI StretchBlt(HDC, int, int, int, int, HDC, int, int, int, int, DWORD);
int     WINAPI StretchDIBits(HDC, int, int, int, int, int,
                        int, int, int, const void FAR*, LPBITMAPINFO, UINT, DWORD);

COLORREF WINAPI SetPixel(HDC, int, int, COLORREF);
COLORREF WINAPI GetPixel(HDC, int, int);

int     WINAPI SetStretchBltMode(HDC, int);
int     WINAPI GetStretchBltMode(HDC);

DWORD   WINAPI SetBitmapDimension(HBITMAP, int, int);
DWORD   WINAPI GetBitmapDimension(HBITMAP);

BOOL    WINAPI SetBitmapDimensionEx(HBITMAP, int, int, SIZE FAR*);
BOOL    WINAPI GetBitmapDimensionEx(HBITMAP, SIZE FAR*);

int     WINAPI SetROP2(HDC, int);
int     WINAPI GetROP2(HDC);

LONG    WINAPI SetBitmapBits(HBITMAP, DWORD, const void FAR*);
LONG    WINAPI GetBitmapBits(HBITMAP, LONG, void FAR*);

int     WINAPI SetDIBits(HDC, HBITMAP, UINT, UINT, const void FAR*, BITMAPINFO FAR*, UINT);
int     WINAPI GetDIBits(HDC, HBITMAP, UINT, UINT, void FAR*, BITMAPINFO FAR*, UINT);

int     WINAPI SetDIBitsToDevice(HDC, int, int, int, int, int, int, UINT, UINT,
                    void FAR*, BITMAPINFO FAR*, UINT);

#endif              /* NOAPIPROTO */


/* constants for the biCompression field */
#define BI_RGB      0L
#define BI_RLE8     1L
#define BI_RLE4     2L
#define BI_BITFIELDS 3L		/* (WIN32) */

/* DIB color table identifiers */
#define DIB_RGB_COLORS  0
#define DIB_PAL_COLORS  1

/* constants for CreateDIBitmap */
#define CBM_INIT        0x00000004L
#define CBM_CREATEDIB   0x00000022L	/* (WIN32) */

/* StretchBlt() Modes */
#define BLACKONWHITE    1
#define WHITEONBLACK    2
#define COLORONCOLOR    3
#define HALFTONE	4		/* (WIN32) */

/* new StretchBlt() Modes (simpler names) */
#define STRETCH_ANDSCANS        BLACKONWHITE
#define STRETCH_ORSCANS         WHITEONBLACK
#define STRETCH_DELETESCANS     COLORONCOLOR
#define STRETCH_HALFTONE	HALFTONE

#endif      /* NOBITMAP */

/* Bitmap raster op support *************************************/

#ifndef NORASTEROPS

/* Binary raster ops */

#define R2_BLACK            1
#define R2_NOTMERGEPEN      2
#define R2_MASKNOTPEN       3
#define R2_NOTCOPYPEN       4
#define R2_MASKPENNOT       5
#define R2_NOT              6
#define R2_XORPEN           7
#define R2_NOTMASKPEN       8
#define R2_MASKPEN          9
#define R2_NOTXORPEN        10
#define R2_NOP              11
#define R2_MERGENOTPEN      12
#define R2_COPYPEN          13
#define R2_MERGEPENNOT      14
#define R2_MERGEPEN         15
#define R2_WHITE            16

/* Ternary raster operations */

#define SRCCOPY             0x00CC0020L
#define SRCPAINT            0x00EE0086L
#define SRCAND              0x008800C6L
#define SRCINVERT           0x00660046L
#define SRCERASE            0x00440328L
#define NOTSRCCOPY          0x00330008L
#define NOTSRCERASE         0x001100A6L
#define MERGECOPY           0x00C000CAL
#define MERGEPAINT          0x00BB0226L
#define PATCOPY             0x00F00021L
#define PATPAINT            0x00FB0A09L
#define PATINVERT           0x005A0049L
#define DSTINVERT           0x00550009L
#define BLACKNESS           0x00000042L
#define WHITENESS           0x00FF0062L

#endif      /* NORASTEROPS */

/* Metafile management ******************************************/

#ifndef NOMETAFILE

DECLARE_HANDLE(HMETAFILE);

/* Clipboard Metafile Picture Structure */
typedef struct tagMETAFILEPICT {
    int     mm;
    int     xExt;
    int     yExt;
    HMETAFILE hMF;
} METAFILEPICT;
typedef METAFILEPICT FAR* LPMETAFILEPICT;

typedef struct tagMETAHEADER {
    UINT    mtType;
    UINT    mtHeaderSize;
    UINT    mtVersion;
    DWORD   mtSize;
    UINT    mtNoObjects;
    DWORD   mtMaxRecord;
    UINT    mtNoParameters;
} METAHEADER;

typedef struct tagHANDLETABLE {
    HGDIOBJ objectHandle[1];
} HANDLETABLE;
typedef HANDLETABLE*      PHANDLETABLE;
typedef HANDLETABLE FAR* LPHANDLETABLE;

typedef struct tagMETARECORD {
    DWORD   rdSize;
    UINT    rdFunction;
    UINT    rdParm[1];
} METARECORD;
typedef METARECORD*      PMETARECORD;
typedef METARECORD FAR* LPMETARECORD;

#ifdef STRICT
typedef int (CALLBACK* MFENUMPROC)(HDC, HANDLETABLE FAR*, METARECORD FAR*, int, LPARAM);
#else
typedef FARPROC MFENUMPROC;
#endif

#ifndef NOAPIPROTO  /* Prototypes */

HDC     WINAPI CreateMetaFile(LPCSTR);
HMETAFILE WINAPI CloseMetaFile(HDC);

HMETAFILE WINAPI GetMetaFile(LPCSTR);
BOOL      WINAPI DeleteMetaFile(HMETAFILE);
HMETAFILE WINAPI CopyMetaFile(HMETAFILE, LPCSTR);

BOOL    WINAPI PlayMetaFile(HDC, HMETAFILE);

HGLOBAL WINAPI GetMetaFileBits(HMETAFILE);
UINT WINAPI GetMetaFileBitsEx(HMETAFILE, UINT, LPVOID);

HMETAFILE WINAPI SetMetaFileBits(HGLOBAL);
HMETAFILE WINAPI SetMetaFileBitsBetter(HGLOBAL);
HMETAFILE WINAPI SetMetaFileBitsEx(UINT, LPBYTE);

void    WINAPI PlayMetaFileRecord(HDC, HANDLETABLE FAR*, METARECORD FAR*, UINT);

BOOL    WINAPI EnumMetaFile(HDC, HMETAFILE, MFENUMPROC, LPARAM);

#endif              /* NOAPIPROTO */

/* Metafile Functions */
#define META_SETBKCOLOR          0x0201
#define META_SETBKMODE           0x0102
#define META_SETMAPMODE          0x0103
#define META_SETROP2             0x0104
#define META_SETRELABS           0x0105
#define META_SETPOLYFILLMODE         0x0106
#define META_SETSTRETCHBLTMODE       0x0107
#define META_SETTEXTCHAREXTRA        0x0108
#define META_SETTEXTCOLOR        0x0209
#define META_SETTEXTJUSTIFICATION    0x020A
#define META_SETWINDOWORG        0x020B
#define META_SETWINDOWEXT        0x020C
#define META_SETVIEWPORTORG      0x020D
#define META_SETVIEWPORTEXT      0x020E
#define META_OFFSETWINDOWORG         0x020F
#define META_SCALEWINDOWEXT      0x0410
#define META_OFFSETVIEWPORTORG       0x0211
#define META_SCALEVIEWPORTEXT        0x0412
#define META_LINETO          0x0213
#define META_MOVETO          0x0214
#define META_EXCLUDECLIPRECT         0x0415
#define META_INTERSECTCLIPRECT       0x0416
#define META_ARC             0x0817
#define META_ELLIPSE             0x0418
#define META_FLOODFILL           0x0419
#define META_PIE             0x081A
#define META_RECTANGLE           0x041B
#define META_ROUNDRECT           0x061C
#define META_PATBLT          0x061D
#define META_SAVEDC          0x001E
#define META_SETPIXEL            0x041F
#define META_OFFSETCLIPRGN       0x0220
#define META_TEXTOUT             0x0521
#define META_BITBLT          0x0922
#define META_STRETCHBLT          0x0B23
#define META_POLYGON             0x0324
#define META_POLYLINE            0x0325
#define META_ESCAPE          0x0626
#define META_RESTOREDC           0x0127
#define META_FILLREGION          0x0228
#define META_FRAMEREGION         0x0429
#define META_INVERTREGION        0x012A
#define META_PAINTREGION         0x012B
#define META_SELECTCLIPREGION        0x012C
#define META_SELECTOBJECT        0x012D
#define META_SETTEXTALIGN        0x012E
#define META_DRAWTEXT            0x062F

#define META_CHORD           0x0830
#define META_SETMAPPERFLAGS      0x0231
#define META_EXTTEXTOUT          0x0a32
#define META_SETDIBTODEV         0x0d33
#define META_SELECTPALETTE       0x0234
#define META_REALIZEPALETTE      0x0035
#define META_ANIMATEPALETTE      0x0436
#define META_SETPALENTRIES       0x0037
#define META_POLYPOLYGON         0x0538
#define META_RESIZEPALETTE       0x0139

#define META_DIBBITBLT           0x0940
#define META_DIBSTRETCHBLT       0x0b41
#define META_DIBCREATEPATTERNBRUSH   0x0142
#define META_STRETCHDIB          0x0f43

#define META_EXTFLOODFILL        0x0548

#define META_RESETDC             0x014C
#define META_STARTDOC            0x014D
#define META_STARTPAGE           0x004F
#define META_ENDPAGE             0x0050
#define META_ABORTDOC            0x0052
#define META_ENDDOC          0x005E

#define META_DELETEOBJECT        0x01f0

#define META_CREATEPALETTE       0x00f7
#define META_CREATEBRUSH         0x00F8
#define META_CREATEPATTERNBRUSH      0x01F9
#define META_CREATEPENINDIRECT       0x02FA
#define META_CREATEFONTINDIRECT      0x02FB
#define META_CREATEBRUSHINDIRECT     0x02FC
#define META_CREATEBITMAPINDIRECT    0x02FD
#define META_CREATEBITMAP        0x06FE
#define META_CREATEREGION        0x06FF

#endif      /* NOMETAFILE */

/* Enhanced metafile management *********************************/

#ifndef NOMETAFILE

DECLARE_HANDLE(HENHMETAFILE);

typedef struct tagENHMETARECORD {
	DWORD	iType;
	DWORD	nSize;
	DWORD	dParm[1];
} ENHMETARECORD;
typedef ENHMETARECORD		*PENHMETARECORD;
typedef ENHMETARECORD NEAR	*NPENHMETARECORD;
typedef ENHMETARECORD FAR	*LPENHMETARECORD;

typedef struct tagENHMETAHEADER {
	DWORD	iType;
	DWORD	nSize;
	RECTL	rclBounds;
	RECTL	rclFrame;
	DWORD	dSignature;
	DWORD	nVersion;
	DWORD	nBytes;
	DWORD	nRecords;
	WORD	nHandles;
	WORD	sReserved;
	DWORD	nDescription;
	DWORD	offDescription;
	DWORD	nPalEntries;
	SIZEL	szlDevice;
	SIZEL	szlMillimeters;
} ENHMETAHEADER;
typedef ENHMETAHEADER		*PENHMETAHEADER;
typedef ENHMETAHEADER NEAR	*NPENHMETAHEADER;
typedef ENHMETAHEADER FAR	*LPENHMETAHEADER;

#ifndef NOAPIPROTO

HENHMETAFILE WINAPI CloseEnhMetaFile(HDC);
HDC WINAPI CreateEnhMetaFile(HDC, LPTSTR, LPRECT, LPTSTR);
BOOL WINAPI GdiComment(HDC, UINT, CONST BYTE *);
BOOL WINAPI PlayEnhMetaFile(HDC, HENHMETAFILE, CONST RECT *);

#endif	/* NOAPIPROTO */

#endif	/* NOMETAFILE */

/* Printing management ******************************************/

#ifdef STRICT
typedef BOOL (CALLBACK* ABORTPROC)(HDC, int);
#else
typedef FARPROC ABORTPROC;
#endif

typedef struct {
    int     cbSize;
    LPCSTR  lpszDocName;
    LPCSTR  lpszOutput;
#if (WINVER >= 0x0400)
    LPCSTR   lpszDatatype;
    DWORD    fwType;
#endif /* WINVER */
}   DOCINFO;
typedef DOCINFO FAR* LPDOCINFO;

#ifndef NOAPIPROTO  /* Prototypes */

int     WINAPI StartDoc(HDC, DOCINFO FAR*);
int     WINAPI StartPage(HDC);
int     WINAPI EndPage(HDC);
int     WINAPI EndDoc(HDC);
int     WINAPI AbortDoc(HDC);
int     WINAPI SetAbortProc(HDC, ABORTPROC);
HANDLE  WINAPI SpoolFile(LPSTR, LPSTR, LPSTR, LPSTR);
BOOL    WINAPI QueryAbort(HDC, int);

#endif              /* NOAPIPROTO */

/* Spooler Error Codes */
#define SP_NOTREPORTED           0x4000
#define SP_ERROR             (-1)
#define SP_APPABORT          (-2)
#define SP_USERABORT             (-3)
#define SP_OUTOFDISK             (-4)
#define SP_OUTOFMEMORY           (-5)

#define PR_JOBSTATUS             0x0000

/* GDI Escape management ****************************************/

#ifndef NOAPIPROTO  /* Prototypes */

int     WINAPI Escape(HDC, int, int, LPCSTR, void FAR*);

#endif              /* NOAPIPROTO */

/* GDI Escapes */
#define NEWFRAME             1
#define ABORTDOC             2
#define NEXTBAND             3
#define SETCOLORTABLE        4
#define GETCOLORTABLE        5
#define FLUSHOUTPUT          6
#define DRAFTMODE            7
#define QUERYESCSUPPORT      8
#define SETABORTPROC         9
#define STARTDOC             10
#define ENDDOC               11
#define GETPHYSPAGESIZE      12
#define GETPRINTINGOFFSET    13
#define GETSCALINGFACTOR     14
#define MFCOMMENT            15
#define GETPENWIDTH          16
#define SETCOPYCOUNT         17
#define SELECTPAPERSOURCE    18
#define DEVICEDATA           19
#define PASSTHROUGH          19
#define GETTECHNOLGY         20
#define GETTECHNOLOGY        20
#define SETLINECAP           21
#define SETLINEJOIN          22
#define SETMITERLIMIT        23
#define BANDINFO             24
#define DRAWPATTERNRECT      25
#define GETVECTORPENSIZE     26
#define GETVECTORBRUSHSIZE   27
#define ENABLEDUPLEX         28
#define GETSETPAPERBINS      29
#define GETSETPRINTORIENT    30
#define ENUMPAPERBINS        31
#define SETDIBSCALING        32
#define EPSPRINTING          33
#define ENUMPAPERMETRICS     34
#define GETSETPAPERMETRICS   35
#define POSTSCRIPT_DATA      37
#define POSTSCRIPT_IGNORE    38
#define MOUSETRAILS          39

#define GETEXTENDEDTEXTMETRICS  256
#define GETEXTENTTABLE          257
#define GETPAIRKERNTABLE        258
#define GETTRACKKERNTABLE       259
#define EXTTEXTOUT              512
#define GETFACENAME             513
#define ENABLERELATIVEWIDTHS    768
#define ENABLEPAIRKERNING       769
#define SETKERNTRACK            770
#define SETALLJUSTVALUES        771
#define SETCHARSET              772

#define STRETCHBLT              2048

#define GETSETSCREENPARAMS      3072

#define BEGIN_PATH              4096
#define CLIP_TO_PATH            4097
#define END_PATH                4098
#define EXT_DEVICE_CAPS         4099
#define RESTORE_CTM             4100
#define SAVE_CTM                4101
#define SET_ARC_DIRECTION       4102
#define SET_BACKGROUND_COLOR    4103
#define SET_POLY_MODE           4104
#define SET_SCREEN_ANGLE        4105
#define SET_SPREAD              4106
#define TRANSFORM_CTM           4107
#define SET_CLIP_BOX            4108
#define SET_BOUNDS              4109

/* (WIN32) GDI Pixel Graphics ********************************************** */

#ifndef NOAPIPROTO	/* Prototypes */

BOOL	WINAPI SetPixelV(HDC, int, int, COLORREF);

#endif			/* NOAPIPROTO */

/* (WIN32) GDI Path Graphics *********************************************** */

#define CONST		const

	/* PolyDraw types */
#define PT_MOVETO	0x01
#define PT_LINETO	0x02
#define PT_BEZIERTO	0x03
#define PT_CLOSEFIGURE	0x80

#ifndef	NOAPIPROTO	/* Prototypes */

BOOL	WINAPI AngleArc(HDC, int, int, DWORD, FLOAT, FLOAT);
BOOL	WINAPI ArcTo(HDC, int, int, int, int, int, int, int, int);
BOOL	WINAPI PolyBezier(HDC, CONST POINT *, DWORD);
BOOL	WINAPI PolyBezierTo(HDC, CONST POINT *, DWORD);
BOOL	WINAPI PolyDraw(HDC, CONST POINT *, CONST BYTE *, int);
BOOL	WINAPI PolylineTo(HDC, CONST POINT *, DWORD);
BOOL	WINAPI PolyPolyline(HDC, CONST POINT *, CONST DWORD *, DWORD);

#endif			/* NOAPIPROTO */

/* (WIN32) GDI Path ******************************************************** */

#ifndef NOAPIPROTO	/* Prototypes */

BOOL	WINAPI AbortPath(HDC);
BOOL	WINAPI BeginPath(HDC);
BOOL	WINAPI CloseFigure(HDC);
BOOL	WINAPI EndPath(HDC);
BOOL	WINAPI FillPath(HDC);
BOOL	WINAPI FlattenPath(HDC);
int	WINAPI GetPath(HDC, LPPOINT, LPBYTE, int);
HRGN	WINAPI PathToRegion(HDC);
BOOL	WINAPI StrokeAndFillPath(HDC);
BOOL	WINAPI StrokePath(HDC);
BOOL	WINAPI WidenPath(HDC);

#endif			/* NOAPIPROTO */

/* (WIN32) GDI Pen ********************************************************* */

#define PS_USERSTYLE		7
#define PS_ALTERNATE		8
#define PS_STYLE_MASK		0x0000000F

#define PS_ENDCAP_ROUND		0x00000000
#define PS_ENDCAP_SQUARE	0x00000100
#define PS_ENDCAP_FLAT		0x00000200
#define PS_ENDCAP_MASK		0x00000F00

#define PS_JOIN_ROUND		0x00000000
#define PS_JOIN_BEVEL		0x00001000
#define PS_JOIN_MITER		0x00002000
#define PS_JOIN_MASK		0x0000F000

#define PS_COSMETIC		0x00000000
#define PS_GEOMETRIC		0x00010000
#define PS_TYPE_MASK		0x000F0000

typedef struct tagEXTLOGPEN {
	UINT	elpPenStyle;
	UINT	elpWidth;
	UINT	elpBrushStyle;
	COLORREF elpColor;
	LONG	elpHatch;
	DWORD	elpNumEntries;
	DWORD	elpStyleEntry[1];
} EXTLOGPEN;
typedef EXTLOGPEN	*PEXTLOGPEN;
typedef EXTLOGPEN NEAR	*NPEXTLOGPEN;
typedef EXTLOGPEN FAR	*LPEXTLOGPEN;

#ifndef NOAPIPROTO	/* Prototypes */

HPEN	WINAPI ExtCreatePen(DWORD, DWORD, LPLOGBRUSH, DWORD, LPDWORD);

#endif			/* NOAPIPROTO */

/* (WIN32) GDI Bitmap ****************************************************** */

#ifndef NOBITMAP

#define BMF_TOPDOWN	1
#define BMF_DIB		2

typedef struct tagDIBSECTION {
	BITMAP			dsBm;
	BITMAPINFOHEADER	dsBmih;
	DWORD			dsBitfields[3];
	HANDLE			dshSection;
	DWORD			dsOffset;
} DIBSECTION;
typedef DIBSECTION	*PDIBSECTION;
typedef DIBSECTION NEAR	*NPDIBSECTION;
typedef DIBSECTION FAR	*LPDIBSECTION;

#ifndef NOAPIPROTO

HBITMAP WINAPI CreateDIBSection(HDC, CONST BITMAPINFO *, UINT, VOID *, HANDLE,
	DWORD);
UINT	WINAPI GetDIBColorTable(HDC, UINT, UINT, RGBQUAD *);
UINT	WINAPI SetDIBColorTable(HDC, UINT, UINT, CONST RGBQUAD *);

#endif	/* NOAPIPROTO */

#endif	/* NOBITMAP */

/* (WIN32) GDI BitBlt ****************************************************** */

#ifndef NORASTEROPS

#define MAKEROP4(fg,bg)		((((bg) << 8) & 0xff000000L) | (fg))

#endif	/* NORASTEROPS */

#ifndef NOAPIPROTO

BOOL	WINAPI MaskBlt(HDC, int, int, int, int, HDC, int, int,
	HBITMAP, int, int, DWORD);
BOOL	WINAPI PlgBlt(HDC, LPPOINT, HDC, int, int, int, int,
	HBITMAP, int, int);

#endif

/* (WIN32) GDI Graphics Mode *********************************************** */

#define GM_COMPATIBLE	0x01		/* compatible with 3.1 */
#define GM_ADVANCED	0x02		/* allow world transformations */

#ifndef NOAPIPROTO	/* Prototypes */

int	WINAPI GetGraphicsMode(HDC);
int	WINAPI SetGraphicsMode(HDC, int);

#endif			/* NOAPIPROTO */

/* (WIN32) GDI World Coordinate Transform ********************************** */

#define MWT_IDENTITY		0x00000001UL
#define MWT_LEFTMULTIPLY	0x00000002UL
#define MWT_RIGHTMULTIPLY	0x00000004UL

typedef struct tagXFORM {
	FLOAT	eM11;
	FLOAT	eM12;
	FLOAT	eM21;
	FLOAT	eM22;
	FLOAT	eDx;
	FLOAT	eDy;
} XFORM;
typedef XFORM		*PXFORM;
typedef XFORM NEAR	*NPXFORM;
typedef XFORM FAR	*LPXFORM;

#ifndef NOAPIPROTO	/* Prototypes */

BOOL	WINAPI CombineTransform(LPXFORM, LPXFORM, LPXFORM);
BOOL	WINAPI GetWorldTransform(HDC, LPXFORM);
BOOL	WINAPI ModifyWorldTransform(HDC, LPXFORM, DWORD);
BOOL	WINAPI SetWorldTransform(HDC, LPXFORM);

#endif			/* NOAPIPROTO */

/* (WIN32) GDI Region ****************************************************** */

#ifndef NOGDIOBJ

#define	RDH_RECTANGLES	1

typedef struct _RGNDATAHEADER {
	DWORD		dwSize;
	DWORD		iType;
	DWORD		nCount;
	DWORD		nRgnSize;
	RECT		rcBound;
} RGNDATAHEADER;
typedef RGNDATAHEADER		*PRGNDATAHEADER;
typedef RGNDATAHEADER NEAR	*NPRGNDATAHEADER;
typedef RGNDATAHEADER FAR	*LPRGNDATAHEADER;

typedef struct _RGNDATA {
	RGNDATAHEADER	rdh;
	char		Buffer[1];
} RGNDATA;
typedef RGNDATA		*PRGNDATA;
typedef RGNDATA NEAR	*NPRGNDATA;
typedef RGNDATA FAR	*LPRGNDATA;

#ifndef NOAPIPROTO

HRGN	WINAPI ExtCreateRegion(CONST XFORM *, DWORD, CONST RGNDATA *);
DWORD	WINAPI GetRegionData(HRGN, DWORD, LPRGNDATA);
BOOL	WINAPI SelectClipPath(HDC, int);

#endif	/* NOAPIPROTO */

#endif	/* NOGDIOBJ */

/* (WIN32) GDI Math ******************************************************** */

#ifndef NOGDI

typedef long		LONGLONG;
typedef unsigned long	DWORDLONG;

#ifndef NOAPIPROTO

LONGLONG	WINAPI Int32x32To64(LONG, LONG);
DWORDLONG	WINAPI Int64ShllMod32(DWORDLONG, DWORD);
LONGLONG	WINAPI Int64ShraMod32(LONGLONG, DWORD);
DWORDLONG	WINAPI Int64ShrlMod32(DWORDLONG, DWORD);
DWORDLONG	WINAPI UInt32x32To64(DWORD, DWORD);

#endif	/* NOAPIPROTO */

#endif	/* NOGDI */

/* (WIN32) GDI DC Attributes *********************************************** */

#define	AD_COUNTERCLOCKWISE	1
#define	AD_CLOCKWISE		2

#ifndef	NOAPIPROTO	/* Prototypes */

int	WINAPI GetArcDirection(HDC);
int	WINAPI SetArcDirection(HDC, int);
BOOL	WINAPI GetMiterLimit(HDC, PFLOAT);
BOOL	WINAPI SetMiterLimit(HDC, FLOAT, PFLOAT);

#endif			/* NOAPIPROTO */

/* (WIN32) GDI Halftone Palette ******************************************** */

#ifndef NOGDI

#define CA_NEGATIVE	0x0001
#define CA_LOG_FILTER	0x0002

#define ILLUMINANT_DEVICE_DEFAULT	0x0000
#define ILLUMINANT_A			0x0001
#define ILLUMINANT_B			0x0002
#define ILLUMINANT_C			0x0003
#define ILLUMINANT_D50			0x0004
#define ILLUMINANT_D55			0x0005
#define ILLUMINANT_D65			0x0006
#define ILLUMINANT_D75			0x0007
#define ILLUMINANT_F2			0x0008
#define ILLUMINANT_MAX_INDEX		ILLUMINANT_F2
#define ILLUMINANT_TUNGSTEN		ILLUMINANT_A
#define ILLUMINANT_DAYLIGHT		ILLUMINANT_C
#define ILLUMINANT_FLUORESCENT		ILLUMINANT_F2
#define ILLUMINANT_NTSC			ILLUMINANT_C

typedef struct tagCOLORADJUSTMENT {
	WORD	caSize;
	WORD	caFlags;
	WORD	caIlluminantIndex;
	WORD	caRedGamma;
	WORD	caGreenGamma;
	WORD	caBlueGamma;
	WORD	caReferenceBlack;
	WORD	caReferenceWhite;
	SHORT	caContrast;
	SHORT	caBrightness;
	SHORT	caColorfulness;
	SHORT	caRedGreenTint;
} COLORADJUSTMENT;
typedef COLORADJUSTMENT		*PCOLORADJUSTMENT;
typedef COLORADJUSTMENT NEAR	*NPCOLORADJUSTMENT;
typedef COLORADJUSTMENT FAR	*LPCOLORADJUSTMENT;

#ifndef NOAPIPROTO

HPALETTE WINAPI CreateHalftonePalette(HDC);

BOOL	WINAPI GetColorAdjustment(HDC, LPCOLORADJUSTMENT);
BOOL	WINAPI SetColorAdjustment(HDC, CONST COLORADJUSTMENT *);

#endif	/* NOAPIPROTO */

#endif	/* NOGDI */

/* GDI Error *************************************************************** */

#define GDI_ERROR	(0xFFFFFFFFL)
#define CLR_INVALID	(0xFFFFFFFFL)

/* DEVMODE ***************************************************************** */

#define CCHDEVICENAME	32
#define CCHFORMNAME	32

typedef struct _devicemodeA
{
    BYTE	dmDeviceName[CCHDEVICENAME];
    WORD	dmSpecVersion;
    WORD	dmDriverVersion;
    WORD	dmSize;
    WORD	dmDriverExtra;
    DWORD	dmFields;
    short	dmOrientation;
    short	dmPaperSize;
    short	dmPaperLength;
    short	dmPaperWidth;
    short	dmScale;
    short	dmCopies;
    short	dmDefaultSource;
    short	dmPrintQuality;
    short	dmColor;
    short	dmDuplex;
    short	dmYResolution;
    short	dmTTOption;
    short	dmCollate;
    BYTE	dmFormName[CCHFORMNAME];
    WORD	dmLogPixels;
    DWORD	dmBitsPerPel;
    DWORD	dmPelsWidth;
    DWORD	dmPelsHeight;
    DWORD	dmDisplayFlags;
    DWORD	dmDisplayFrequency;
#if (WINVER >= 0x0400)
    DWORD	dmICMMethod;
    DWORD	dmICMIntent;
    DWORD	dmMediaType;
    DWORD	dmDitherType;
    DWORD	dmReserved1;
    DWORD	dmReserved2;
#endif	/* WINVER */
} DEVMODEA;
typedef DEVMODEA	*PDEVMODEA;
typedef DEVMODEA NEAR	*NPDEVMODEA;
typedef DEVMODEA FAR	*LPDEVMODEA;

typedef struct _devicemodeW
{
    WCHAR	dmDeviceName[CCHDEVICENAME];
    WORD	dmSpecVersion;
    WORD	dmDriverVersion;
    WORD	dmSize;
    WORD	dmDriverExtra;
    DWORD	dmFields;
    short	dmOrientation;
    short	dmPaperSize;
    short	dmPaperLength;
    short	dmPaperWidth;
    short	dmScale;
    short	dmCopies;
    short	dmDefaultSource;
    short	dmPrintQuality;
    short	dmColor;
    short	dmDuplex;
    short	dmYResolution;
    short	dmTTOption;
    short	dmCollate;
    WCHAR	dmFormName[CCHFORMNAME];
    WORD	dmLogPixels;
    DWORD	dmBitsPerPel;
    DWORD	dmPelsWidth;
    DWORD	dmPelsHeight;
    DWORD	dmDisplayFlags;
    DWORD	dmDisplayFrequency;
#if (WINVER >= 0x0400)
    DWORD	dmICMMethod;
    DWORD	dmICMIntent;
    DWORD	dmMediaType;
    DWORD	dmDitherType;
    DWORD	dmReserved1;
    DWORD	dmReserved2;
#endif	/* WINVER */
} DEVMODEW;
typedef DEVMODEW	*PDEVMODEW;
typedef DEVMODEW NEAR	*NPDEVMODEW;
typedef DEVMODEW FAR	*LPDEVMODEW;

#ifdef UNICODE
typedef DEVMODEW	DEVMODE;
typedef PDEVMODEW	PDEVMODE;
typedef NPDEVMODEW	NPDEVMODE;
typedef LPDEVMODEW	LPDEVMODE;
#else
typedef DEVMODEA	DEVMODE;
typedef PDEVMODEA	PDEVMODE;
typedef NPDEVMODEA	NPDEVMODE;
typedef LPDEVMODEA	LPDEVMODE;
#endif	/* UNICODE */

#include "Win_NT.h"

/* Pixel format descriptor ************************************************* */

typedef struct tagPIXELFORMATDESCRIPTOR
{
	WORD	nSize;
	WORD	nVersion;
	DWORD	dwFlags;
	BYTE	iPixelType;
	BYTE	cColorBits;
	BYTE	cRedBits;
	BYTE	cRedShift;
	BYTE	cGreenBits;
	BYTE	cGreenShift;
	BYTE	cBlueBits;
	BYTE	cBlueShift;
	BYTE	cAlphaBits;
	BYTE	cAlphaShift;
	BYTE	cAccumBits;
	BYTE	cAccumRedBits;
	BYTE	cAccumGreenBits;
	BYTE	cAccumBlueBits;
	BYTE	cAccumAlphaBits;
	BYTE	cDepthBits;
	BYTE	cStencilBits;
	BYTE	cAuxBuffers;
	BYTE	iLayerType;
	BYTE	Reserved;
	DWORD	dwLayerMask;
	DWORD	dwVisibleMask;
	DWORD	dwDamageMask;
} PIXELFORMATDESCRIPTOR;
typedef PIXELFORMATDESCRIPTOR		*PPIXELFORMATDESCRIPTOR;
typedef PIXELFORMATDESCRIPTOR NEAR	*NPPIXELFORMATDESCRIPTOR;
typedef PIXELFORMATDESCRIPTOR FAR	*LPPIXELFORMATDESCRIPTOR;

/* pixel types */

#define PFD_TYPE_RGBA			0
#define PFD_TYPE_COLORINDEX		1

/* layer types */

#define PFD_MAIN_PLANE			0
#define PFD_OVERLAY_PLANE		1
#define PFD_UNDERLAY_PLANE		(-1)

/* PIXELFORMATDESCRIPTOR flags */

#define PFD_DOUBLEBUFFER		0x00000001
#define PFD_STEREO			0x00000002
#define PFD_DRAW_TO_WINDOW		0x00000004
#define PFD_DRAW_TO_BITMAP		0x00000008
#define PFD_SUPPORT_GDI			0x00000010
#define PFD_SUPPORT_OPENGL		0x00000020
#define PFD_GENERIC_FORMAT		0x00000040
#define PFD_NEED_PALETTE		0x00000080
#define PFD_NEED_SYSTEM_PALETTE		0x00000100
#define PFD_SWAP_EXCHANGE		0x00000200
#define PFD_SWAP_COPY			0x00000400
#define PFD_SWAP_LAYER_BUFFERS		0x00000800
#define PFD_GENERIC_ACCELERATED		0x00001000

/* PIXELFORMATDESCRIPTOR flags (ChoosePixelFormat()) */

#define PFD_DEPTH_DONTCARE		0x20000000
#define PFD_DOUBLEBUFFER_DONTCARE	0x40000000
#define PFD_STEREO_DONTCARE		0x80000000

/* OpenGL ****************************************************************** */

#ifndef	NOAPIPROTO

DECLARE_HANDLE(HGLRC);

BOOL	WINAPI wglCopyContext(HGLRC, HGLRC, UINT);
HGLRC	WINAPI wglCreateContext(HDC);
HGLRC	WINAPI wglCreateLayerContext(HDC, int);
BOOL	WINAPI wglDeleteContext(HGLRC);
HGLRC	WINAPI wglGetCurrentContext(VOID);
HDC	WINAPI wglGetCurrentDC(VOID);
PROC	WINAPI wglGetProcAddress(LPCSTR);
BOOL	WINAPI wglMakeCurrent(HDC, HGLRC);
BOOL	WINAPI wglShareLists(HGLRC, HGLRC);
BOOL	WINAPI wglUseFontBitmapsA(HDC, DWORD, DWORD, DWORD);
BOOL	WINAPI wglUseFontBitmapsW(HDC, DWORD, DWORD, DWORD);

#endif	/* NOAPIPROTO */

#endif      /* Win_Gdi__h */
