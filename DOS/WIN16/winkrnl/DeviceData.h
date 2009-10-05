/********************************************************************

	@(#)DeviceData.h	2.32    Device data definitions.
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


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

********************************************************************/
 

#ifndef DeviceData__h
#define DeviceData__h

#include "windows.h"
#include "GdiDC.h"

#define	LSD_INIT 		0

#define	LSD_CREATEDC		1
#define	LSD_DELETEDC		2
#define	LSD_SAVEDC		3
#define	LSD_RESTOREDC		4

#define	LSD_SETBOUNDSRECT	5
#define	LSD_GETBOUNDSRECT	6

#define	LSD_GETDEVICECAPS	7
#define	LSD_SETDA		8
#define	LSD_GETDA		9

#define	LSD_GETNEARCLR		10

#define	LSD_SELECTOBJECT	11
#define	LSD_ENUMOBJECTS		12
#define	LSD_SELECTPALETTE	13
#define	LSD_REALIZEPALETTE	14

#define	LSD_SELECTCLIPRGN	15
#define	LSD_INTERSECTCLIPRECT	16
#define	LSD_OFFSETCLIPRGN	17
#define	LSD_EXCLUDECLIPRECT	18
#define	LSD_GETCLIPBOX		19
#define	LSD_GETCLIPRGN		20

#define	LSD_MOVETO		21
#define	LSD_LINETO		22
#define	LSD_POLYLINE		23
#define	LSD_RECTANGLE		24
#define	LSD_ROUNDRECT		25
#define	LSD_ELLIPSE		26
#define	LSD_ARC			27
#define	LSD_CHORD		28
#define	LSD_PIE			29
#define	LSD_POLYGON		30
#define	LSD_POLYPOLYGON		31
#define	LSD_FLOODFILL		32
#define	LSD_EXTFLOODFILL	33

#define	LSD_FILLRGN		34
#define	LSD_FRAMERGN		35
#define	LSD_INVERTRGN		36
#define	LSD_PAINTRGN		37

#define	LSD_TEXTOUT		38
#define	LSD_EXTTEXTOUT		39
#define	LSD_GETCHARWIDTH	40
#define	LSD_GETASPECTRATIO	41
#define	LSD_GETTEXTMETRICS	42
#define	LSD_ENUMFONTS		43
#define	LSD_GETTEXTFACE		44

#define	LSD_BITBLT		45
#define	LSD_PATBLT		46
#define	LSD_STRETCHBLT		47
#define	LSD_STRETCHDIBITS	48
#define	LSD_SETPIXEL		49
#define	LSD_GETPIXEL		50
#define	LSD_SETDIBTODEVICE	51
#define	LSD_SCROLLDC		52

#define	LSD_STARTDOC		53
#define	LSD_STARTPAGE		54
#define	LSD_ENDPAGE		55
#define	LSD_ENDDOC		56
#define	LSD_ABORTDOC		57
#define	LSD_SETABORTPROC	58
#define	LSD_QUERYABORT		59
#define	LSD_ESCAPE		60

#define LSD_MASKBLT		61
#define LSD_PLGBLT		62
#define LSD_SYSTEMPALETTEUSE	63
#define LSD_GETSYSTEMPALETTE	64
#define LSD_UPDATECOLORS	65
#define LSD_GETGLYPHOUTLINE	66

/* sub-functions for SET_DA */

#define LSD_DA_MAPMODE		0
#define LSD_DA_WINDOWORG	1
#define LSD_DA_WINDOWEXT	2
#define LSD_DA_OFFSETWINDOWORG	3
#define LSD_DA_SCALEWINDOWEXT	4
#define LSD_DA_VIEWPORTORG	5
#define LSD_DA_VIEWPORTEXT	6
#define LSD_DA_OFFSETVIEWPORTORG 7
#define LSD_DA_SCALEVIEWPORTEXT	8
#define LSD_DA_BRUSHORG		9
#define LSD_DA_POLYFILLMODE	10
#define LSD_DA_TEXTCOLOR	11
#define LSD_DA_BKCOLOR		12
#define LSD_DA_BKMODE		13
#define LSD_DA_TEXTALIGN	14
#define LSD_DA_TEXTCHAREXTRA	15
#define LSD_DA_TEXTJUST		16
#define LSD_DA_MAPPERFLAGS	17
#define LSD_DA_STRETCHMODE	18
#define LSD_DA_ROP2		19
#define	LSD_DA_RELABS		20
#define	LSD_DA_DCORG		21

#define LSD_DA_SYSPALUSE	22
#define LSD_DA_GRAPHICSMODE	22		/* (WIN32) */
#define LSD_DA_WORLDTRANSFORM	23		/* (WIN32) */
#define LSD_DA_ARCDIRECTION	24		/* (WIN32) */
#define LSD_DA_COLORADJUSTMENT	24		/* (WIN32) */

/* modifiers for lsd_ functions - dwParam */

#define	LSDM_GET		0L
#define	LSDM_SET		1L

/* structures */

typedef struct
   {
   UINT				uChar;
   UINT				fuFormat;
   LPGLYPHMETRICS	lpgm;
   DWORD			cbBuffer;
   LPVOID			lpvBuffer;
   CONST MAT2*		lpmat2;
   } LSDE_GETGLYPHOUTLINE;
   
typedef struct tagNEWLOGFONT
   {
   LOGFONT  lf;
   BYTE     lfFullName[2 * LF_FACESIZE];
   BYTE     lfStyle[LF_FACESIZE];
   } NEWLOGFONT, *LPNEWLOGFONT;

typedef struct {
    DWORD    dwInvalidMask;
    COLORREF TextColor;
    COLORREF BackColor;
    COLORREF SrcDCBkColor;
    int	     BackMode;
    int	     PolyFillMode;
    int	     ROP2;
    int	     StretchMode;
    WORD     BrushFlag;
    COLORREF BrushColor;
    LPVOID   BrushPrivate;
    UINT     PenStyle;
    int	     PenWidth;
    int	     PenHeight;
    COLORREF PenColor;
} LSDE_VALIDATE;

typedef struct {
    LPVOID lpimagedata;
    int xSrc,ySrc;
    int xDest,yDest;
    int cx,cy;
} LSDE_PUTIMAGEDATA;

typedef struct {
	int	xDest;
	int	yDest;
	int	nWidthDest;
	int	nHeightDest;
	HDC32	hSrcDC32;
	int	xSrc;
	int	ySrc;
	int	nWidthSrc;
	int	nHeightSrc;
	UINT	startline;
	UINT	numlines;
	const	void *lpvBits;
	LPBITMAPINFO lpbmi;
	UINT	fuColorUse;
	DWORD	dwRop;
} LSDE_STRETCHDATA;

typedef struct {
	POINT	ptDestXY[3];
	LPDC	lpSrcDC;
	int	nSrcX, nSrcY;
	int	nSrcWidth, nSrcHeight;
	HBITMAP	hMaskBitmap;
	int	nMaskX, nMaskY;
	DWORD	dwRop;
} LSDE_WORLDBLT;
typedef LSDE_WORLDBLT		*PLSDE_WORLDBLT;
typedef LSDE_WORLDBLT NEAR	*NPLSDE_WORLDBLT;
typedef LSDE_WORLDBLT FAR	*LPLSDE_WORLDBLT;

typedef struct {
	UINT	uPaletteIndex;
	COLORREF crPaletteColor;
} LSDE_COLOR;
typedef LSDE_COLOR		*PLSDE_COLOR;
typedef LSDE_COLOR NEAR		*NPLSDE_COLOR;
typedef LSDE_COLOR FAR		*LPLSDE_COLOR;

typedef struct {
	LPPOINT  lpPoints;
	LPINT	lpCounts;
	int	nCount;
	int nTotalCount;
} LSDE_POLYPOLY;

typedef struct {
	int nXStart,nYStart;
	COLORREF cr;
	UINT	fuFillType;
} LSDE_FLOODFILL;

typedef struct {
	HRGN	hRgn;
	HBRUSH	hBrush;
	int	nWidth,nHeight;
	DWORD	dwROP;
} LSDE_PAINTRGN;

typedef struct {
	int	x;
	int	y;
	LPSTR	lpStr;
	int	nCnt;
	UINT	uiFlags;
	LPRECT	lpRect;
	LPINT	lpDX;
	/* these are the DC attributes that affect TextOut/ExtTextOut */
	UINT	TextAlign;
	int	nBreakExtra;
	int	nBreakCount;
	int	nErrorTerm;
	int	nCharExtra;
        LPINT   lpExtraSpace;  
        char    chBreak;       /* break character */
} LSDE_TEXT;

typedef struct {
	LPCSTR lpszDriver;
	LPCSTR lpszDevice;
	LPCSTR lpszOutput;
	LPVOID lpvInitData;
} LSDE_CREATEDATA;

typedef struct {
	int cbInput;
	LPCSTR lpszInData;
	LPVOID lpvOutData;
} LSDE_ESCAPEDATA;

typedef struct {
	int xSrc,ySrc;
	int xDest,yDest;
	int nWidth,nHeight;
} LSDE_SCROLLDC;

typedef struct {
	int fnObjectType;
	GOBJENUMPROC goenmprc;
	LPARAM  lParam;
} LSDE_ENUMOBJ;

typedef struct {
	HDC32 hDC32;
	LPSTR lpszFamily;
	FONTENUMPROC fntenmprc;
	LPARAM lParam;
	LPNEWLOGFONT lplfCache; /* MiD 10/09/95 */
	LPNEWTEXTMETRIC lpntmCache;
	DWORD dwCacheSize;
} LSDE_ENUMFONTS;

typedef struct {
	FONTENUMPROC fntenmprc;
	LPLOGFONT lpLogFont;
	LPNEWTEXTMETRIC lpntm;
	int FontType;
	LPARAM lParam;
} LSDS_ENUMFONTSCALLBACK;

typedef struct {
        DWORD     lpMagic;   /* pointer to cached font or NULL if it's a new font */
        LOGFONT   LogFont; /* logfont to realize */
} LSDE_REALIZEFONT;

typedef DWORD (CALLBACK* LSDENUMFONTSPROC)(LPVOID, LSDS_ENUMFONTSCALLBACK *);

typedef struct tagLSDS_PARAMS {
    LSDE_VALIDATE		lsde_validate;
    union {
	int			mode;
	SIZE			size;
	POINT			point;
	RECT			rect;
	COLORREF		colorref;
	POINT			arc[4];
	LOGFONT			logfont;
	ABORTPROC		lpfnabort;
	HRGN			region;
	LPDOCINFO		lpdocinfo;
        LPTEXTMETRIC		lpmetric;
	LSDE_PUTIMAGEDATA	imagedata;
	LSDE_STRETCHDATA	stretchdata;
	LSDE_POLYPOLY		polypoly;
	LSDE_FLOODFILL		floodfill;
	LSDE_PAINTRGN		paintrgn;
	LSDE_TEXT		text;
	LSDE_CREATEDATA		create;
	LSDE_ESCAPEDATA		escape;
	LSDE_SCROLLDC		scrolldc;
	LSDE_ENUMOBJ		enumobj;
	LSDE_ENUMFONTS		enumfonts;
        LSDE_REALIZEFONT        realizefont;
	LSDE_WORLDBLT		worldblt;
	LSDE_COLOR		color;
	LSDE_GETGLYPHOUTLINE	getglyphoutline;
    } lsde;
} LSDS_PARAMS;

DWORD lsd_noop(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_setboundsrect(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_getboundsrect(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_setda(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_getda(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_excludecliprect(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_offsetcliprgn(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_paintrgn(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_framergn(WORD,HDC32,DWORD,LPLSDS_PARAMS);

DWORD lsd_display_init(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_createdc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_deletedc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_savedc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_restoredc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_getdevcaps(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_getnearclr(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_selectobject(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_enumobjects(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_selectpal(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_realizepal(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_selectcliprgn(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_getclipbox(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_getcliprgn(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_moveto(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_lineto(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_polyline(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_rectangle(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_roundrect(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_ellipse(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_arc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_polygon(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_polypolygon(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_extfloodfill(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_textout(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_exttextout(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_getcharwidth(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_getaspectratio(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_gettextmetrics(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_enumfonts(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_gettextface(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_stretchblt(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_stretchdibits(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_scrolldc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_setpixel(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_getpixel(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_setdibtodevice(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_escape(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_worldblt(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_systempaletteuse(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_getsystempalette(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_updatecolors(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_display_getglyphoutline(WORD,HDC32,DWORD,LPLSDS_PARAMS);

DWORD lsd_printer_init(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_createdc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_deletedc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_savedc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_restoredc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_getdevcaps(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_getnearclr(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_selectobject(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_enumobjects(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_selectpal(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_realizepal(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_selectcliprgn(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_getclipbox(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_getcliprgn(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_moveto(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_output(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_extfloodfill(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_textout(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_exttextout(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_getcharwidth(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_getaspectratio(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_gettextmetrics(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_enumfonts(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_gettextface(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_stretchblt(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_stretchdibits(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_pixel(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_setdibtodevice(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_scrolldc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_startdoc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_startpage(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_endpage(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_enddoc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_abortdoc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_setabortproc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_queryabort(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_escape(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_worldblt(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_systempaletteuse(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_getsystempalette(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_printer_updatecolors(WORD,HDC32,DWORD,LPLSDS_PARAMS);

DWORD lsd_meta_init(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_createdc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_deletedc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_savedc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_restoredc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_setda(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_selectobject(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_selectpal(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_realizepal(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_selectcliprgn(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_intersectcliprect(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_offsetcliprgn(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_excludecliprect(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_moveto(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_lineto(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_poly(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_rectangle(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_roundrect(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_ellipse(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_arc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_poly(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_polypolygon(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_floodfill(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_extfloodfill(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_fillrgn(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_framergn(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_invertrgn(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_paintrgn(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_textout(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_exttextout(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_bitblt(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_patblt(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_stretchblt(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_stretchdibits(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_setpixel(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_setdibtodevice(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_startdoc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_startpage(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_endpage(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_enddoc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_abortdoc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_meta_escape(WORD,HDC32,DWORD,LPLSDS_PARAMS);

DWORD lsd_mm_passthru(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_mm_setboundsrect(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_mm_getboundsrect(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_mm_getclipbox(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_mm_rectangle(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_mm_point(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_mm_moveto(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_mm_lineto(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_mm_poly(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_mm_roundrect(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_mm_arc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_mm_extfloodfill(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_mm_exttextout(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_mm_getcharwidth(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_mm_gettextmetrics(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_mm_stretchblt(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_mm_setdibtodevice(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_mm_scrolldc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_mm_worldblt(WORD,HDC32,DWORD,LPLSDS_PARAMS);

DWORD lsd_br_passthru(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_br_rectangle(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_br_lineto(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_br_poly(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_br_polyline(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_br_polypolygon(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_br_framergn(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_br_paintrgn(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_br_extfloodfill(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_br_exttextout(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_br_textout(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_br_stretchblt(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_br_stretchdibits(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_br_setdibtodevice(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_br_setpixel(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_br_scrolldc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
DWORD lsd_br_worldblt(WORD,HDC32,DWORD,LPLSDS_PARAMS);

#endif
