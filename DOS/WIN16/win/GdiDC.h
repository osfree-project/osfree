/********************************************************************

	@(#)GdiDC.h	2.32    GDI DC data definitions.
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
 
#ifndef GdiDC__h
#define GdiDC__h

#include "ObjEngine.h"

#define MAX_DEVICE_NAME		32	

typedef struct tagDCINFO *LPDCINFO;
typedef	LPDCINFO HDC32;

typedef struct tagLSDS_PARAMS *LPLSDS_PARAMS;

typedef DWORD (*GDIPROC)(WORD,HDC32,DWORD,LPLSDS_PARAMS);

typedef struct tagLSDENTRY
{
    char		Name[MAX_DEVICE_NAME];
    GDIPROC		OutputProc;
    GDIPROC		MapModeProc;
    GDIPROC		BoundsRectProc;
} LSDENTRY;

typedef struct tagDCINFO
{
    OBJHEAD	ObjHead;	/* generic object header */
    HWND	hWnd;		/* owning window 		*/

    DWORD	dwDCXFlags;	/* DCX_ flags of GetDCEx API */
				/* plus internal DC type flags */

    COLORREF	TextColor;	/* GetTextColor/SetTextColor */
    COLORREF	BackColor;	/* GetBkColor/SetBkColor     */
    int		BackMode;	/* GetBkMode/SetBkMode       */
    int		PolyFillMode;   /* GetPolyFillMode/Set...    */
    int		ROP2;		/* GetROP2/SetROP2           */
    int		StretchBltMode; /* Get/SetStretchBltMode     */
    int		RelAbs;		/* GetRelAbs/SetRelAbs       */

    UINT	TextAlign;	/* GetTextAlign/SetTextAlign */
    int		nBreakExtra;	/* extra space to pad on textout */
    int		nBreakCount;	/* number of padding characters  */
    int		nErrorTerm;	/* accumulated rounding error for align */
    int		nCharExtra;	/* inter-character padding       */
    LPINT       lpExtraSpace;   /* extra space array created/removed in SetTextJustify (MiD 10/17/95) */

    DWORD	dwMapperFlags;	/* SetMapperFlags value	*/

    int		MapMode;	/* MM_TEXT...		     */
    int		FOx,FOy;	/* final dc origin	     */
    int		DOx,DOy;	/* dc origin		     */
    int		WOx,WOy;	/* Window Origin x,y 	     */
    int		WEx,WEy;	/* Window Extent x,y 	     */
    int		VOx,VOy;	/* Viewport Origin x,y	     */
    int		VEx,VEy;	/* Viewport Extent x,y	     */

    UINT	BoundsFlag;	/* accumulating or not...    */
    RECT	BoundsRect;	/* GetBoundsRect 	     */
    
    HBRUSH	hBrush;
    POINT	ptBrushOrg;	/* origin to realize brushes	*/

    HFONT	hFont;
    LPVOID      lpCharWidths;   /* this can be LPINT or LPABC */
    LPNEWTEXTMETRIC lpNTM;      /* ptr to NEWTEXTMETRIC for this font */
    UINT        uiFontType;     /* RASTER/DEVICE/TRUETYPE_FONTTYPE */
    int         nDevWidth;      /* width in device units */
    int         nDevHeight;     /* height in device units */

    HPEN	hPen;
    HBITMAP	hBitmap;

    HPALETTE	hPalette;
    BOOL	fPalBack;	/* foreground/background palette */
    LPUINT	lpSystemPaletteIndex;

    struct tagDCINFO *lpSavedDC;	/* most recent saved DC */
    int nSavedID;			/* saved DC identifier */

    LSDENTRY	*lpLSDEntry;	/* points to the device description */
    GDIPROC	Output[3];	/* the three magic poniters */

    DWORD	dwInvalid;	/* invalid mask */

    LPVOID  	lpDrvData;	/* driver-specific portion */
    /* WIN32 */
    int		nArcDirection;	/* arc direction */
    int		nGraphicsMode;	/* graphics mode */
    double	eM11, eM12;	/* world transform matrix */
    double	eM21, eM22;
    double	eDx, eDy;
    FLOAT	eMiterLimit;	/* miter limit */
    int		nPathState;	/* path state (0=none,1=opened,2=closed) */
    int		nPathSize;	/* path size */
    LPBYTE	lpPathTypes;	/* path types */
    LPPOINT	lpPathPoints;	/* path points */
    COLORADJUSTMENT caHalftone;	/* HALFTONE stretch mode color adjustment */
} DCINFO;

typedef struct tagDCCACHE
{
    BOOL fBusy;
    DCINFO *lpDC;
} DCCACHE;

extern DCCACHE *TWIN_DCCache;
extern int TWIN_DCCacheSize;

#define	GETHDC32	GETDCINFO
#define	GETHDC16(h32)	(HDC)((h32)->ObjHead.hObj)

#define	ASSERT_HDC(hDC32,hDC,err) { \
		if (!((hDC32) = GETDCINFO((hDC)))) { \
		    ERRSTR((LF_ERROR,"***ERROR*** bad DC %x\n",(hDC))); \
		    return (err); \
		}}

#define	DC_OUTPUT(func,hDC32,dw,lp) (((hDC32)->Output[0]) \
				((func),(HDC32)(hDC32), \
				(DWORD)(dw),(LPLSDS_PARAMS)(lp)))
#define	DC_MM_OUTPUT(func,hDC32,dw,lp) (((hDC32)->Output[1]) \
				((func),(hDC32),(dw),(lp)))
#define	DC_BR_OUTPUT(func,hDC32,dw,lp) (((hDC32)->Output[2]) \
				((func),(hDC32),(dw),(lp)))

/* GDI DC ****************************************************************** */

typedef DCINFO	DC;
typedef DC	*PDC;
typedef DC NEAR	*NPDC;
typedef DC FAR	*LPDC;

#define ASSERT_DC(lpDC, hDC, error)\
	ASSERT_HDC(lpDC, hDC, error)

#define LOCK_DC(hDC)\
	GETDCINFO(hDC)

#define UNLOCK_DC(hDC)\
	RELEASEDCINFO(hDC)

/* Our internal DC types */
#define	DCX_CACHE_DC	0x10000000
#define	DCX_PARENT_DC	0x20000000
#define	DCX_CLASS_DC	0x40000000
#define	DCX_OWN_DC	0x80000000
#define	DCX_DISPLAY_DC	0x01000000
#define	DCX_COMPATIBLE_DC 0x02000000
#define DCX_PRINTER_DC	0x04000000
#define	DCX_METAFILE_DC	0x08000000

#define	DCXDEVICE_MASK	(DCX_DISPLAY_DC|DCX_PRINTER_DC|DCX_METAFILE_DC)

/* client area clipping for non-WS_CLIPSIBLINGS */
#define	DCX_CLIENTCLIP	0x00800000

/* The WIN32 extensions to the GetDCEx API interface follow: */
#define DCX_DEFAULTCLIP		0x00000000L	/* placeholder */
#define DCX_NORESETATTRS	0x00000004L
#define DCX_EXCLUDEUPDATE	0x00000100L
#define DCX_INTERSECTUPDATE	0x00000200L
#define DCX_NORECOMPUTE		0x00100000L
#define DCX_VALIDATE		0x00200000L

/* Masks for attribute invalidation */
#define IM_POLYFILLMODEMASK	0x00000001L
#define IM_TEXTCOLORMASK	0x00000002L
#define IM_BKCOLORMASK		0x00000004L
#define IM_BKMODEMASK		0x00000008L
#define IM_ROP2MASK		0x00000080L
#define IM_STRETCHMODEMASK	0x00000100L
#define IM_BRUSHCOLORMASK	0x00000200L
#define IM_BRUSHFILLMASK	0x00000400L
#define IM_BRUSHDEPTHMASK	0x00000800L
#define IM_PENCOLORMASK		0x00001000L
#define IM_PENSTYLEMASK		0x00002000L
#define IM_PENWIDTHMASK		0x00004000L
#define IM_FONTMASK		0x00008000L
#define IM_FONTSCALEMASK	0x00010000L
#define IM_TEXTMETRICMASK	0x00020000L
#define IM_SRCBKCOLORMASK	0x00100000L

/* If a new bitmask is added, add it also to IM_INVALID */

#define IM_INVALID		0x0013FF8FL	/* All of the above */

#define IM_TEXTMASK	(IM_TEXTCOLORMASK|IM_BKCOLORMASK|IM_BKMODEMASK)
#define IM_BRUSHMASK	(IM_BRUSHCOLORMASK|IM_BRUSHFILLMASK|IM_BRUSHDEPTHMASK| \
				IM_BKMODEMASK|IM_BKCOLORMASK)
#define IM_PENMASK	(IM_PENCOLORMASK|IM_PENSTYLEMASK|IM_PENWIDTHMASK| \
				IM_BKMODEMASK)

#define PALETTE_RGB	0x02000000L
#define PALETTE_INDEX	0x01000000L
#define PALETTE_MASK	(PALETTE_RGB|PALETTE_INDEX)

#define	BFP_NULL	0	/* null brush	*/
#define BFP_BITMAP 	1	/* private is a bitmap (stipple)  */
#define BFP_PIXEL 	2	/* private is a pixel color       */
#define BFP_PIXMAP 	3	/* private is a pixmap (tile)     */
#define BFP_UNASSIGNED 	4	/* private is to be determined	  */

#define	DM_DEFAULT	0x00000001	/* for dithering routine */
#define	DM_MONOCHROME	0x00000004

/* Exported functions */

BOOL  GdiInitDC(void);
HDC32 GdiCreateHDC(BOOL bCache);
void  GdiInitDisplayDC(HDC32);

/* these are internal DC functions */

BOOL LEtoDE(HDC32,LPSIZE);
BOOL DEtoLE(HDC32,LPSIZE);

int MM_LEtoDE_X(HDC32, int);
int MM_LEtoDE_Y(HDC32, int);
int MM_DEtoLE_X(HDC32, int);
int MM_DEtoLE_Y(HDC32, int);

int MM0_LPtoDP_X(HDC32, int, int);
int MM0_LPtoDP_Y(HDC32, int, int);
int MM0_DPtoLP_X(HDC32, int, int);
int MM0_DPtoLP_Y(HDC32, int, int);
int MM_LPtoDP_X(HDC32, int, int);
int MM_LPtoDP_Y(HDC32, int, int);
int MM_DPtoLP_X(HDC32, int, int);
int MM_DPtoLP_Y(HDC32, int, int);

#endif
