/*
	@(#)GdiObjects.h	1.18
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

*/
 
#ifndef GdiObjects__h
#define GdiObjects__h

#include "ObjEngine.h"
#include "Resources.h"
#include "kerndef.h"

/* ----- GDI objects -----*/
typedef struct tagGDIOBJ
  {
    OBJHEAD     ObjHead;
    BOOL fIsRealized;
  } GDIOBJ, *LPGDIOBJ; 

/*----- Palette information structure -----*/
typedef struct tagPALENTRYINFO 
  {
    COLORREF	crPalColor;
    DWORD	dwPixel;
  } PALENTRYINFO, *LPPALENTRYINFO;

typedef struct tagPALETTEINFO {
    OBJHEAD	ObjHead;
    BOOL	fIsRealized;
    /* next 2 lines are obsolete - use WIN32 implementation */
/*  UINT	uiNumEntries;		*/
/*  LPPALENTRYINFO lpPalEntries;	*/
    /* (WIN32) */
    LPLOGPALETTE	lpLogPalette;
} PALETTEINFO;

typedef PALETTEINFO *LPPALETTEINFO;
typedef LPPALETTEINFO HPALETTE32;
#define	GETHPALETTE32	GETPALETTEINFO
#define	GETHPALETTE16(h32) (HPALETTE)((h32)->ObjHead.hObj)

#define	ASSERT_HPALETTE(hPalette32,hPalette,err) { \
		if (!((hPalette32) = GETHPALETTE32((hPalette)))) { \
		   ERRSTR((LF_ERROR,"***ERROR*** bad Palette %x\n",(hPalette))); \
		   return (err); \
		}}

/* GDI Logical Palette ***************************************************** */

typedef PALETTEINFO PALETTE;
typedef PALETTE		*PPALETTE;
typedef PALETTE NEAR	*NPPALETTE;
typedef PALETTE FAR	*LPPALETTE;

#define ASSERT_PALETTE(lpPalette, hPalette, error)\
	ASSERT_HPALETTE(lpPalette, hPalette, error)

#define LOCK_PALETTE(hPalette)\
	GETPALETTEINFO(hPalette)

#define UNLOCK_PALETTE(hPalette)\
	RELEASEPALETTEINFO(hPalette)

/*----- Brush information structure -----*/
typedef struct tagBRUSHINFO
  {
    OBJHEAD	ObjHead;		/* generic object header */
    BOOL	fIsRealized;
    LPVOID	lpPrivate;		/* driver-specific data  */
    DWORD	dwFlags;		/* object flags --- subfields: */
					/* Flags & 0x0000ffff = type of brush */
					/* Flags & 0xffff0000 =              */
					/*      depth<<16 | DCX_ flags       */
    LOGBRUSH	lpBrush;		/* logical brush structure */
  } BRUSHINFO;
 
typedef BRUSHINFO *LPBRUSHINFO;
typedef LPBRUSHINFO HBRUSH32;
#define	GETHBRUSH32	GETBRUSHINFO
#define	GETHBRUSH16(h32) (HBRUSH)((h32)->ObjHead.hObj)

#define	ASSERT_HBRUSH(hBrush32,hBrush,err) { \
		if (!((hBrush32) = GETHBRUSH32((hBrusht)))) { \
		    ERRSTR((LF_ERROR,"***ERROR*** bad Brush %x\n",(hBrush))); \
		    return (err); \
		}}

/*----- Pen information structure -----*/
typedef struct tagPENINFO
  {
    OBJHEAD	ObjHead;		/* generic object header */
    BOOL	fIsRealized;
    LOGPEN	lpPen;			/* logical pen structure */
    LPEXTLOGPEN	lpExtPen;		/* (WIN32) extended pen structure */
  } PENINFO;

typedef PENINFO *LPPENINFO;
typedef LPPENINFO HPEN32;
#define	GETHPEN32	GETPENINFO
#define	GETHPEN16(h32)	(HPEN)((h32)->ObjHead.hObj)

#define	ASSERT_HPEN(hPen32,hPen,err) { \
		if (!((hPen32) = GETHPEN32((hPen)))) { \
		    ERRSTR((LF_ERROR,"***ERROR*** bad Pen %x\n",(hPen))); \
		    return (err); \
		}}

/*----- Bitmap information structure -----*/
typedef struct {
    OBJHEAD	ObjHead;		/* generic object header 	*/
    BOOL	fIsRealized;

    int		ImageWidth;		/* bit width of a raster line	*/
    int		ImageHeight;		/* bit height of an image	*/
    int		WidthBytes;		/* byte width of a raster line	*/
    int		LinePad;		/* raster line bit alignment	*/
    BYTE	ImagePlanes;
    BYTE	ImageDepth;
    int		BitsPixel;
    BOOL	fMono;			/* set for a mono image	*/
    NAMEINFO	*rcsinfo;		/* points to loaded resource 	*/
    DWORD	dimension; 		/* dimension of bitmap 		*/
    LPVOID	lpDrvData;		/* driver-specific portion	*/
    DIBSECTION	ds;			/* (WIN32) DIBSECTION */
    UINT	dsUsage;		/* (WIN32) DIBSECTION */
    LPBITMAPINFO lpdsBmi;		/* (WIN32) DIBSECTION */
} IMAGEINFO;
typedef IMAGEINFO *LPIMAGEINFO;

/*----- Icon information structure -----*/
typedef struct tagTWIN_ICONINFO {
	OBJHEAD	  ObjHead;		/* generic object header 	*/
	HBITMAP   hXORImage;		/* handle of color bitmap */
	HBITMAP   hANDMask;		/* handle of monochrome bitmask */
	HBITMAP   hMonoBmp;		/* handle of monochrome XOR image */
	LPNAMEINFO rcsinfo;		/* points to loaded resource 	*/
} TWIN_ICONINFO;
typedef TWIN_ICONINFO *LPTWIN_ICONINFO;

/*----- Metafile information structure  ------*/
typedef struct tagMETAFILEINFO {
    OBJHEAD	ObjHead;		/* generic object header     */
    LPSTR	lpMetaObj;		/* Metafile object in memory */
} METAFILEINFO;
typedef METAFILEINFO *LPMETAFILEINFO;


/*----- Cursor information structure  ------*/
typedef struct {
	OBJHEAD		ObjHead;	/* generic object header 	*/
	LPVOID		lpCurData;	/* ptr to driver cursor		*/
	LPNAMEINFO	rcsinfo;	/* points to loaded resource	*/
} CURSORINFO;
typedef CURSORINFO	*LPCURSORINFO;

#endif /* GdiObjects__h */
