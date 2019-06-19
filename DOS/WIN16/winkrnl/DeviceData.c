/*    
	DeviceData.c	2.24
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

#include "windows.h"

#include "GdiDC.h"
#include "DeviceData.h"

/* exported routines */

/* output routine for display LSD */
DWORD DCDisplayProc(WORD,HDC32,DWORD,LPLSDS_PARAMS);

/* output routine for printer LSD */
DWORD DCPrinterProc(WORD,HDC32,DWORD,LPLSDS_PARAMS);

/* output routine for metafile LSD */
DWORD DCMetaProc(WORD,HDC32,DWORD,LPLSDS_PARAMS);

/* output routine for mapping mode transformations */
DWORD DCMapModeProc(WORD,HDC32,DWORD,LPLSDS_PARAMS);

/* output routine for bounding rectangle transformations */
DWORD DCBoundsRectProc(WORD,HDC32,DWORD,LPLSDS_PARAMS);

/* LSD jump tables are now static inside this module */

static GDIPROC DCDisplayTable[] =
{
    lsd_display_init,		/* LSD_INIT 		0 */
    lsd_display_createdc,	/* LSD_CREATEDC/IC	1 */
    lsd_display_deletedc,	/* LSD_DELETEDC		2 */
    lsd_display_savedc,		/* LSD_SAVEDC		3 */
    lsd_display_restoredc,	/* LSD_RESTOREDC	4 */
    lsd_setboundsrect,		/* LSD_SETBOUNDSRECT	5 */
    lsd_getboundsrect,		/* LSD_GETBOUNDSRECT	6 */
    lsd_display_getdevcaps,	/* LSD_GETDEVICECAPS	7 */
    lsd_setda,			/* LSD_SETDA		8 */
    lsd_getda,			/* LSD_GETDA		9 */
    lsd_display_getnearclr,	/* LSD_GETNEARCLR	10 */
    lsd_display_selectobject,	/* LSD_SELECTOBJECT	11 */
    lsd_display_enumobjects,	/* LSD_ENUMOBJECTS	12 */
    lsd_display_selectpal,	/* LSD_SELECTPALETTE	13 */
    lsd_display_realizepal,	/* LSD_REALIZEPALETTE	14 */
#ifdef	LATER
    /* palette manager functions such as UpdateColors etc. may have */
    /* to be added here to be device-specific */
#endif
    lsd_display_selectcliprgn,	/* LSD_SELECTCLIPRGN	15 */
    lsd_excludecliprect,	/* LSD_INTERSECTCLIPRECT 16 */
    lsd_offsetcliprgn,		/* LSD_OFFSETCLIPRGN	17 */
    lsd_excludecliprect,	/* LSD_EXCLUDECLIPRECT	18 */
    lsd_display_getclipbox,	/* LSD_GETCLIPBOX	19 */
    lsd_display_getcliprgn,	/* LSD_GETCLIPRGN	20 */
    lsd_display_moveto,		/* LSD_MOVETO		21 */
    lsd_display_lineto,		/* LSD_LINETO		22 */
    lsd_display_polyline,	/* LSD_POLYLINE		23 */
    lsd_display_rectangle,	/* LSD_RECTANGLE	24 */
    lsd_display_roundrect,	/* LSD_ROUNDRECT	25 */
    lsd_display_ellipse,	/* LSD_ELLIPSE		26 */
    lsd_display_arc,		/* LSD_ARC		27 */
    lsd_display_arc,		/* LSD_CHORD		28 */
    lsd_display_arc,		/* LSD_PIE		29 */
    lsd_display_polygon,	/* LSD_POLYGON		30 */
    lsd_display_polypolygon,	/* LSD_POLYPOLYGON	31 */
    lsd_display_extfloodfill,	/* LSD_FLOODFILL	32 */
    lsd_display_extfloodfill,	/* LSD_EXTFLOODFILL	33 */
    lsd_paintrgn,		/* LSD_FILLRGN		34 */
    lsd_framergn,		/* LSD_FRAMERGN		35 */
    lsd_paintrgn,		/* LSD_INVERTRGN	36 */
    lsd_paintrgn,		/* LSD_PAINTRGN		37 */
    lsd_display_textout,	/* LSD_TEXTOUT		38 */
    lsd_display_exttextout,	/* LSD_EXTTEXTOUT	39 */
    lsd_display_getcharwidth,	/* LSD_GETCHARWIDTH	40 */
    lsd_display_getaspectratio,	/* LSD_GETASPECTRATIO	41 */
    lsd_display_gettextmetrics,	/* LSD_GETTEXTMETRICS	42 */
    lsd_display_enumfonts,	/* LSD_ENUMFONTS	43 */
    lsd_display_gettextface,	/* LSD_GETTEXTFACE	44 */
#ifdef	LATER
    /* there is a bunch of TT functions to be added here */
#endif
    lsd_display_stretchblt,	/* LSD_BITBLT		45 */
    lsd_display_stretchblt,	/* LSD_PATBLT		46 */
    lsd_display_stretchblt,	/* LSD_STRETCHBLT	47 */
    lsd_display_stretchdibits,	/* LSD_STRETCHDIBITS	48 */
    lsd_display_setpixel,	/* LSD_SETPIXEL		49 */
    lsd_display_getpixel,	/* LSD_GETPIXEL		50 */
    lsd_display_setdibtodevice,	/* LSD_SETDIBTODEVICE	51 */
    lsd_display_scrolldc,	/* LSD_SCROLLDC		52 */
    lsd_noop,			/* LSD_STARTDOC		53 */
    lsd_noop,			/* LSD_STARTPAGE	54 */
    lsd_noop,			/* LSD_ENDPAGE		55 */
    lsd_noop,			/* LSD_ENDDOC		56 */
    lsd_noop,			/* LSD_ABORTDOC		57 */
    lsd_noop,			/* LSD_SETABORTPROC	58 */
    lsd_noop,			/* LSD_QUERYABORT	59 */
    lsd_display_escape,		/* LSD_ESCAPE		60 */
    lsd_display_worldblt,	/* LSD_MASKBLT		61 */
    lsd_display_worldblt,	/* LSD_PLGBLT		62 */
    lsd_display_systempaletteuse, /* LSD_SYSTEMPALETTEUSE	63 */
    lsd_display_getsystempalette, /* LSD_GETSYSTEMPALETTE	64 */
    lsd_display_updatecolors,	/* LSD_UPDATECOLORS	65 */
    lsd_display_getglyphoutline, /* LSD_GETGLYPHOUTLINE	66 */
};

#ifdef	macintosh
static GDIPROC DCPrinterTable[] =
{
    lsd_printer_init,		/* LSD_INIT 		0 */
    lsd_printer_createdc,	/* LSD_CREATEDC/IC	1 */
    lsd_printer_deletedc,	/* LSD_DELETEDC		2 */
    lsd_printer_savedc,		/* LSD_SAVEDC		3 */
    lsd_printer_restoredc,	/* LSD_RESTOREDC	4 */
    lsd_setboundsrect,		/* LSD_SETBOUNDSRECT	5 */
    lsd_getboundsrect,		/* LSD_GETBOUNDSRECT	6 */
    lsd_display_getdevcaps,		/* LSD_GETDEVICECAPS		7 CMA - Changed this */
    lsd_setda,					/* LSD_SETDA				8 */
    lsd_getda,					/* LSD_GETDA				9 */
    lsd_display_getnearclr,		/* LSD_GETNEARCLR			10 CMA - Changed this */
    lsd_display_selectobject,	/* LSD_SELECTOBJECT			11 CMA - Changed this */
    lsd_display_enumobjects,	/* LSD_ENUMOBJECTS			12 CMA - Changed this */
    lsd_display_selectpal,		/* LSD_SELECTPALETTE		13 CMA - Changed this */
    lsd_display_realizepal,		/* LSD_REALIZEPALETTE		14 CMA - Changed this */
#ifdef	LATER
    /* palette manager functions such as UpdateColors etc. may have */
    /* to be added here to be device-specific */
#endif
    lsd_display_selectcliprgn,	/* LSD_SELECTCLIPRGN		15 CMA - Changed this */
    lsd_excludecliprect,	/* LSD_INTERSECTCLIPRECT 16 */
    lsd_offsetcliprgn,		/* LSD_OFFSETCLIPRGN	17 */
    lsd_excludecliprect,	/* LSD_EXCLUDECLIPRECT	18 */
    lsd_display_getclipbox,		/* LSD_GETCLIPBOX			19 CMA - Changed this */
    lsd_display_getcliprgn,		/* LSD_GETCLIPRGN			20 CMA - Changed this */
    lsd_display_moveto,			/* LSD_MOVETO				21 CMA - Changed this */
    lsd_display_lineto,			/* LSD_LINETO				22 CMA - Changed this */
    lsd_display_polyline,		/* LSD_POLYLINE				23 CMA - Changed this */
    lsd_display_rectangle,		/* LSD_RECTANGLE			24 CMA - Changed this */
    lsd_display_roundrect,		/* LSD_ROUNDRECT			25 CMA - Changed this */
    lsd_display_ellipse,		/* LSD_ELLIPSE				26 CMA - Changed this */
    lsd_display_arc,			/* LSD_ARC					27 CMA - Changed this */
    lsd_display_arc,			/* LSD_CHORD				28 CMA - Changed this */
    lsd_display_arc,			/* LSD_PIE					29 CMA - Changed this */
    lsd_display_polygon,		/* LSD_POLYGON				30 CMA - Changed this */
    lsd_display_polypolygon,	/* LSD_POLYPOLYGON			31 CMA - Changed this */
    lsd_display_extfloodfill,	/* LSD_FLOODFILL			32 CMA - Changed this */
    lsd_display_extfloodfill,	/* LSD_EXTFLOODFILL			33 CMA - Changed this */
    lsd_paintrgn,		/* LSD_FILLRGN		34 */
    lsd_framergn,		/* LSD_FRAMERGN		35 */
    lsd_paintrgn,		/* LSD_INVERTRGN	36 */
    lsd_paintrgn,		/* LSD_PAINTRGN		37 */
    lsd_display_textout,		/* LSD_TEXTOUT				38 CMA - Changed this */
    lsd_display_exttextout,		/* LSD_EXTTEXTOUT			39 CMA - Changed this */
    lsd_display_getcharwidth,	/* LSD_GETCHARWIDTH			40 CMA - Changed this */
    lsd_display_getaspectratio,	/* LSD_GETASPECTRATIO		41 CMA - Changed this */
    lsd_display_gettextmetrics,	/* LSD_GETTEXTMETRICS		42 CMA - Changed this */
    lsd_display_enumfonts,		/* LSD_ENUMFONTS			43 CMA - Changed this */
    lsd_display_gettextface,	/* LSD_GETTEXTFACE			44 CMA - Changed this */
#ifdef	LATER
    /* there is a bunch of TT functions to be added here */
#endif
    lsd_display_stretchblt,		/* LSD_BITBLT				45 CMA - Changed this */
    lsd_display_stretchblt,		/* LSD_PATBLT				46 CMA - Changed this */
    lsd_display_stretchblt,		/* LSD_STRETCHBLT			47 CMA - Changed this */
    lsd_display_stretchdibits,	/* LSD_STRETCHDIBITS		48 CMA - Changed this */
    lsd_display_setpixel,		/* LSD_SETPIXEL				49 CMA - Changed this */
    lsd_display_getpixel,		/* LSD_GETPIXEL				50 CMA - Changed this */
    lsd_display_setdibtodevice,	/* LSD_SETDIBTODEVICE		51 CMA - Changed this */
    lsd_display_scrolldc,		/* LSD_SCROLLDC				52 CMA - Changed this */
    lsd_printer_startdoc,	/* LSD_STARTDOC		53 */
    lsd_printer_startpage,	/* LSD_STARTPAGE	54 */
    lsd_printer_endpage,	/* LSD_ENDPAGE		55 */
    lsd_printer_enddoc,		/* LSD_ENDDOC		56 */
    lsd_printer_abortdoc,	/* LSD_ABORTDOC		57 */
    lsd_printer_setabortproc,	/* LSD_SETABORTPROC	58 */
    lsd_printer_queryabort,	/* LSD_QUERYABORT	59 */
    lsd_display_escape,			/* LSD_ESCAPE				60 CMA - Changed this */
    lsd_display_worldblt,		/* LSD_MASKBLT				61 CMA - Changed this */
    lsd_display_worldblt,		/* LSD_PLGBLT				62 CMA - Changed this */
    lsd_display_systempaletteuse, /* LSD_SYSTEMPALETTEUSE	63 CMA - Changed this */
    lsd_display_getsystempalette, /* LSD_GETSYSTEMPALETTE	64 CMA - Changed this */
    lsd_display_updatecolors,	/* LSD_UPDATECOLORS			65 CMA - Changed this */
    lsd_display_getglyphoutline, /* LSD_GETGLYPHOUTLINE	66 */
};
#else	/* macintosh */
#if !defined(TWIN_EMPTY_LSD_PRINTER)
static GDIPROC DCPrinterTable[] =
{
    lsd_printer_init,		/* LSD_INIT 		0 */
    lsd_printer_createdc,	/* LSD_CREATEDC/IC	1 */
    lsd_printer_deletedc,	/* LSD_DELETEDC		2 */
    lsd_printer_savedc,		/* LSD_SAVEDC		3 */
    lsd_printer_restoredc,	/* LSD_RESTOREDC	4 */
    lsd_setboundsrect,		/* LSD_SETBOUNDSRECT	5 */
    lsd_getboundsrect,		/* LSD_GETBOUNDSRECT	6 */
    lsd_printer_getdevcaps,	/* LSD_GETDEVICECAPS	7 */
    lsd_setda,			/* LSD_SETDA		8 */
    lsd_getda,			/* LSD_GETDA		9 */
    lsd_printer_getnearclr,	/* LSD_GETNEARCLR	10 */
    lsd_printer_selectobject,	/* LSD_SELECTOBJECT	11 */
    lsd_printer_enumobjects,	/* LSD_ENUMOBJECTS	12 */
    lsd_printer_selectpal,	/* LSD_SELECTPALETTE	13 */
    lsd_printer_realizepal,	/* LSD_REALIZEPALETTE	14 */
#ifdef	LATER
    /* palette manager functions such as UpdateColors etc. may have */
    /* to be added here to be device-specific */
#endif
    lsd_printer_selectcliprgn,	/* LSD_SELECTCLIPRGN	15 */
    lsd_excludecliprect,	/* LSD_INTERSECTCLIPRECT 16 */
    lsd_offsetcliprgn,		/* LSD_OFFSETCLIPRGN	17 */
    lsd_excludecliprect,	/* LSD_EXCLUDECLIPRECT	18 */
    lsd_printer_getclipbox,	/* LSD_GETCLIPBOX	19 */
    lsd_printer_getcliprgn,	/* LSD_GETCLIPRGN	20 */
    lsd_printer_moveto,		/* LSD_MOVETO		21 */
    lsd_printer_output,		/* LSD_LINETO		22 */
    lsd_printer_output,		/* LSD_POLYLINE		23 */
    lsd_printer_output,		/* LSD_RECTANGLE	24 */
    lsd_printer_output,		/* LSD_ROUNDRECT	25 */
    lsd_printer_output,		/* LSD_ELLIPSE		26 */
    lsd_printer_output,		/* LSD_ARC		27 */
    lsd_printer_output,		/* LSD_CHORD		28 */
    lsd_printer_output,		/* LSD_PIE		29 */
    lsd_printer_output,		/* LSD_POLYGON		30 */
    lsd_printer_output,		/* LSD_POLYPOLYGON	31 */
    lsd_printer_extfloodfill,	/* LSD_FLOODFILL	32 */
    lsd_printer_extfloodfill,	/* LSD_EXTFLOODFILL	33 */
    lsd_paintrgn,		/* LSD_FILLRGN		34 */
    lsd_framergn,		/* LSD_FRAMERGN		35 */
    lsd_paintrgn,		/* LSD_INVERTRGN	36 */
    lsd_paintrgn,		/* LSD_PAINTRGN		37 */
    lsd_printer_textout,	/* LSD_TEXTOUT		38 */
    lsd_printer_exttextout,	/* LSD_EXTTEXTOUT	39 */
    lsd_printer_getcharwidth,	/* LSD_GETCHARWIDTH	40 */
    lsd_printer_getaspectratio,	/* LSD_GETASPECTRATIO	41 */
    lsd_printer_gettextmetrics,	/* LSD_GETTEXTMETRICS	42 */
    lsd_printer_enumfonts,	/* LSD_ENUMFONTS	43 */
    lsd_printer_gettextface,	/* LSD_GETTEXTFACE	44 */
#ifdef	LATER
    /* there is a bunch of TT functions to be added here */
#endif
    lsd_printer_stretchblt,	/* LSD_BITBLT		45 */
    lsd_printer_stretchblt,	/* LSD_PATBLT		46 */
    lsd_printer_stretchblt,	/* LSD_STRETCHBLT	47 */
    lsd_printer_stretchdibits,	/* LSD_STRETCHDIBITS	48 */
    lsd_printer_pixel,		/* LSD_SETPIXEL		49 */
    lsd_printer_pixel,		/* LSD_GETPIXEL		50 */
    lsd_printer_setdibtodevice,	/* LSD_SETDIBTODEVICE	51 */
    lsd_printer_scrolldc,	/* LSD_SCROLLDC		52 */
    lsd_printer_startdoc,	/* LSD_STARTDOC		53 */
    lsd_printer_startpage,	/* LSD_STARTPAGE	54 */
    lsd_printer_endpage,	/* LSD_ENDPAGE		55 */
    lsd_printer_enddoc,		/* LSD_ENDDOC		56 */
    lsd_printer_abortdoc,	/* LSD_ABORTDOC		57 */
    lsd_printer_setabortproc,	/* LSD_SETABORTPROC	58 */
    lsd_printer_queryabort,	/* LSD_QUERYABORT	59 */
    lsd_printer_escape,		/* LSD_ESCAPE		60 */
    lsd_printer_worldblt,	/* LSD_MASKBLT		61 */
    lsd_printer_worldblt,	/* LSD_PLGBLT		62 */
    lsd_printer_systempaletteuse, /* LSD_SYSTEMPALETTEUSE	63 */
    lsd_printer_getsystempalette, /* LSD_GETSYSTEMPALETTE	64 */
    lsd_printer_updatecolors,	/* LSD_UPDATECOLORS	65 */
    lsd_noop,			/* LSD_GETGLYPHOUTLINE	66 */
};
#else /* empty lsd_printer */
static GDIPROC DCPrinterTable[] =
{
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
    lsd_noop,
};
#endif  /* TWIN_EMPTY_LSD_PRINTER */
#endif	/* else macintosh */

static GDIPROC DCMetaTable[] =
{
    lsd_meta_init,		/* LSD_INIT 		0 */
    lsd_meta_createdc,		/* LSD_CREATEDC/IC	1 */
    lsd_meta_deletedc,		/* LSD_DELETEDC		2 */
    lsd_meta_savedc,		/* LSD_SAVEDC		3 */
    lsd_meta_restoredc,		/* LSD_RESTOREDC	4 */
    lsd_noop,			/* LSD_SETBOUNDSRECT	5 */
    lsd_noop,			/* LSD_GETBOUNDSRECT	6 */
    lsd_noop,			/* LSD_GETDEVICECAPS	7 */
    lsd_meta_setda,		/* LSD_SETDA		8 */
    lsd_noop,			/* LSD_GETDA		9 */
    lsd_noop,			/* LSD_GETNEARCLR	10 */
    lsd_meta_selectobject,	/* LSD_SELECTOBJECT	11 */
    lsd_noop,			/* LSD_ENUMOBJECTS	12 */
    lsd_meta_selectobject,	/* LSD_SELECTPALETTE	13 */
    lsd_meta_realizepal,	/* LSD_REALIZEPALETTE	14 */
#ifdef	LATER
    /* palette manager functions such as UpdateColors etc. may have */
    /* to be added here to be device-specific */
#endif
    lsd_meta_selectcliprgn,	/* LSD_SELECTCLIPRGN	15 */
    lsd_meta_rectangle,		/* LSD_INTERSECTCLIPRECT 16 */
    lsd_meta_offsetcliprgn,	/* LSD_OFFSETCLIPRGN	17 */
    lsd_meta_rectangle,		/* LSD_EXCLUDECLIPRECT	18 */
    lsd_noop,			/* LSD_GETCLIPBOX	19 */
    lsd_noop,			/* LSD_GETCLIPRGN	20 */
    lsd_meta_moveto,		/* LSD_MOVETO		21 */
    lsd_meta_moveto,		/* LSD_LINETO		22 */
    lsd_meta_poly,		/* LSD_POLYLINE		23 - shared */
    lsd_meta_rectangle,		/* LSD_RECTANGLE	24 */
    lsd_meta_roundrect,		/* LSD_ROUNDRECT	25 */
    lsd_meta_rectangle,		/* LSD_ELLIPSE		26 */
    lsd_meta_arc,		/* LSD_ARC		27 */
    lsd_meta_arc,		/* LSD_CHORD		28 */
    lsd_meta_arc,		/* LSD_PIE		29 */
    lsd_meta_poly,		/* LSD_POLYGON		30 - shared */
    lsd_meta_polypolygon,	/* LSD_POLYPOLYGON	31 */
    lsd_meta_floodfill,		/* LSD_FLOODFILL	32 */
    lsd_meta_extfloodfill,	/* LSD_EXTFLOODFILL	33 */
    lsd_meta_fillrgn,		/* LSD_FILLRGN		34 */
    lsd_meta_framergn,		/* LSD_FRAMERGN		35 */
    lsd_meta_invertrgn,		/* LSD_INVERTRGN	36 */
    lsd_meta_paintrgn,		/* LSD_PAINTRGN		37 */
    lsd_meta_textout,		/* LSD_TEXTOUT		38 */
    lsd_meta_exttextout,	/* LSD_EXTTEXTOUT	39 */
    lsd_noop,			/* LSD_GETCHARWIDTH	40 */
    lsd_noop,			/* LSD_GETASPECTRATIO	41 */
    lsd_noop,			/* LSD_GETTEXTMETRICS	42 */
    lsd_noop,			/* LSD_ENUMFONTS	43 */
    lsd_noop,			/* LSD_GETTEXTFACE	44 */
#ifdef	LATE4
    /* there is a bunch of TT functions to be added here */
#endif
    lsd_meta_bitblt,		/* LSD_BITBLT		45 */
    lsd_meta_patblt,		/* LSD_PATBLT		46 */
    lsd_meta_stretchblt,	/* LSD_STRETCHBLT	47 */
    lsd_meta_stretchdibits,	/* LSD_STRETCHDIBITS	48 */
    lsd_meta_setpixel,		/* LSD_SETPIXEL		49 */
    lsd_noop,			/* LSD_GETPIXEL		50 */
    lsd_meta_setdibtodevice,	/* LSD_SETDIBTODEVICE	51 */
    lsd_noop,			/* LSD_SCROLLDC		52 */
    lsd_meta_startdoc,		/* LSD_STARTDOC		53 */
    lsd_meta_startpage,		/* LSD_STARTPAGE	54 */
    lsd_meta_endpage,		/* LSD_ENDPAGE		55 */
    lsd_meta_enddoc,		/* LSD_ENDDOC		56 */
    lsd_meta_abortdoc,		/* LSD_ABORTDOC		57 */
    lsd_noop,			/* LSD_SETABORTPROC	58 */
    lsd_noop,			/* LSD_QUERYABORT	59 */
    lsd_meta_escape,		/* LSD_ESCAPE		60 */
    lsd_noop,			/* LSD_MASKBLT		61 */
    lsd_noop,			/* LSD_PLGBLT		62 */
    lsd_noop,			/* LSD_SYSTEMPALETTEUSE	63 */
    lsd_noop,			/* LSD_GETSYSTEMPALETTE	64 */
    lsd_noop,			/* LSD_UPDATECOLORS	65 */
    lsd_noop,			/* LSD_GETGLYPHOUTLINE	66 */
};

GDIPROC DCMapModeTable[] =
{
    lsd_mm_passthru,		/* LSD_INIT 		0 */
    lsd_mm_passthru,		/* LSD_CREATEDC/IC	1 */
    lsd_mm_passthru,		/* LSD_DELETEDC		2 */
    lsd_mm_passthru,		/* LSD_SAVEDC		3 */
    lsd_mm_passthru,		/* LSD_RESTOREDC	4 */
    lsd_mm_setboundsrect,	/* LSD_SETBOUNDSRECT	5 */
    lsd_mm_getboundsrect,	/* LSD_GETBOUNDSRECT	6 */
    lsd_mm_passthru,		/* LSD_GETDEVICECAPS	7 */
    lsd_mm_passthru,		/* LSD_SETDA		8 */
    lsd_mm_passthru,		/* LSD_GETDA		9 */
    lsd_mm_passthru,		/* LSD_GETNEARCLR	10 */
    lsd_mm_passthru,		/* LSD_SELECTOBJECT	11 */
    lsd_mm_passthru,		/* LSD_ENUMOBJECTS	12 */
    lsd_mm_passthru,		/* LSD_SELECTPALETTE	13 */
    lsd_mm_passthru,		/* LSD_REALIZEPALETTE	14 */
#ifdef	LATER
    /* palette manager functions such as UpdateColors etc. may have */
    /* to be added here to be device-specific */
#endif
    lsd_mm_passthru,		/* LSD_SELECTCLIPRGN	15 */
    lsd_mm_passthru,		/* LSD_INTERSECTCLIPRECT 16 */
    lsd_mm_passthru,		/* LSD_OFFSETCLIPRGN	17 */
    lsd_mm_passthru,		/* LSD_EXCLUDECLIPRECT	18 */
    lsd_mm_getclipbox,		/* LSD_GETCLIPBOX	19 */
    lsd_mm_passthru,		/* LSD_GETCLIPRGN	20 */
    lsd_mm_moveto,		/* LSD_MOVETO		21 */
    lsd_mm_lineto,		/* LSD_LINETO		22 */
    lsd_mm_poly,		/* LSD_POLYLINE		23 */
    lsd_mm_rectangle,		/* LSD_RECTANGLE	24 */
    lsd_mm_roundrect,		/* LSD_ROUNDRECT	25 */
    lsd_mm_rectangle,		/* LSD_ELLIPSE		26 */
    lsd_mm_arc,			/* LSD_ARC		27 */
    lsd_mm_arc,			/* LSD_CHORD		28 */
    lsd_mm_arc,			/* LSD_PIE		29 */
    lsd_mm_poly,		/* LSD_POLYGON		30 */
    lsd_mm_poly,		/* LSD_POLYPOLYGON	31 */
    lsd_mm_extfloodfill,	/* LSD_FLOODFILL	32 */
    lsd_mm_extfloodfill,	/* LSD_EXTFLOODFILL	33 */
    lsd_mm_passthru,		/* LSD_FILLRGN		34 */
    lsd_mm_passthru,		/* LSD_FRAMERGN		35 */
    lsd_mm_passthru,		/* LSD_INVERTRGN	36 */
    lsd_mm_passthru,		/* LSD_PAINTRGN		37 */
    lsd_mm_exttextout,		/* LSD_TEXTOUT		38 */
    lsd_mm_exttextout,		/* LSD_EXTTEXTOUT	39 */
    lsd_mm_getcharwidth,	/* LSD_GETCHARWIDTH	40 */
    lsd_mm_passthru,		/* LSD_GETASPECTRATIO	41 */
    lsd_mm_gettextmetrics,	/* LSD_GETTEXTMETRICS	42 */
    lsd_mm_passthru,		/* LSD_ENUMFONTS	43 */
    lsd_mm_passthru,		/* LSD_GETTEXTFACE	44 */
#ifdef	LATER
    /* there is a bunch of TT functions to be added here */
#endif
    lsd_mm_stretchblt,		/* LSD_BITBLT		45 */
    lsd_mm_stretchblt,		/* LSD_PATBLT		46 */
    lsd_mm_stretchblt,		/* LSD_STRETCHBLT	47 */
    lsd_mm_stretchblt,		/* LSD_STRETCHDIBITS	48 */
    lsd_mm_point,		/* LSD_SETPIXEL		49 */
    lsd_mm_point,		/* LSD_GETPIXEL		50 */
    lsd_mm_setdibtodevice,	/* LSD_SETDIBTODEVICE	51 */
    lsd_mm_scrolldc,		/* LSD_SCROLLDC		52 */
    lsd_mm_passthru,		/* LSD_STARTDOC		53 */
    lsd_mm_passthru,		/* LSD_STARTPAGE	54 */
    lsd_mm_passthru,		/* LSD_ENDPAGE		55 */
    lsd_mm_passthru,		/* LSD_ENDDOC		56 */
    lsd_mm_passthru,		/* LSD_ABORTDOC		57 */
    lsd_mm_passthru,		/* LSD_SETABORTPROC	58 */
    lsd_mm_passthru,		/* LSD_QUERYABORT	59 */
    lsd_mm_passthru,		/* LSD_ESCAPE		60 */
    lsd_mm_worldblt,		/* LSD_MASKBLT		61 */
    lsd_mm_worldblt,		/* LSD_PLGBLT		62 */
    lsd_mm_passthru,		/* LSD_SYSTEMPALETTEUSE	63 */
    lsd_mm_passthru,		/* LSD_GETSYSTEMPALETTE	64 */
    lsd_mm_passthru,		/* LSD_UPDATECOLORS	65 */
    lsd_mm_passthru,		/* LSD_GETGLYPHOUTLINE	66 */

};

GDIPROC DCBoundsRectTable[] =
{
    lsd_br_passthru,		/* LSD_INIT			0 */
    lsd_br_passthru,		/* LSD_CREATEDC/IC		1 */
    lsd_br_passthru,		/* LSD_DELETEDC			2 */
    lsd_br_passthru,		/* LSD_SAVEDC			3 */
    lsd_br_passthru,		/* LSD_RESTOREDC		4 */
    lsd_br_passthru,		/* LSD_SETBOUNDSRECT    5 */
    lsd_br_passthru,		/* LSD_GETBOUNDSRECT    6 */
    lsd_br_passthru,		/* LSD_GETDEVICECAPS    7 */
    lsd_br_passthru,		/* LSD_SETDA 			8 */
    lsd_br_passthru,		/* LSD_GETDA 			9 */
    lsd_br_passthru,		/* LSD_GETNEARCLR		10 */
    lsd_br_passthru,		/* LSD_SELECTOBJECT		11 */
    lsd_br_passthru,		/* LSD_ENUMOBJECTS		12 */
    lsd_br_passthru,		/* LSD_SELECTPALETTE    13 */
    lsd_br_passthru,		/* LSD_REALIZEPALETTE   14 */
#ifdef	LATER
    /* palette manager functions such as UpdateColors etc. may have */
    /* to be added here to be device-specific */
#endif
    lsd_br_passthru,		/* LSD_SELECTCLIPRGN    15 */
    lsd_br_passthru,		/* LSD_INTERSECTCLIPRECT 16 */
    lsd_br_passthru,		/* LSD_OFFSETCLIPRGN    17 */
    lsd_br_passthru,		/* LSD_EXCLUDECLIPRECT  18 */
    lsd_br_passthru,		/* LSD_GETCLIPBOX  19 */
    lsd_br_passthru,		/* LSD_GETCLIPRGN  20 */
    lsd_br_passthru,		/* LSD_MOVETO			21 */
    lsd_br_lineto,		/* LSD_LINETO			22 */
    lsd_br_polyline,		/* LSD_POLYLINE			23 */
    lsd_br_rectangle,		/* LSD_RECTANGLE		24 */
    lsd_br_rectangle,		/* LSD_ROUNDRECT	 <25> */
    lsd_br_rectangle,		/* LSD_ELLIPSE		 <26> */
    lsd_br_rectangle, 		/* LSD_ARC		 <27> */
    lsd_br_rectangle,		/* LSD_CHORD 		 <28> */
    lsd_br_rectangle, 		/* LSD_PIE		 <29> */
    lsd_br_polyline,		/* LSD_POLYGON		 <30> */
    lsd_br_polypolygon,    /* LSD_POLYPOLYGON		31 */
    lsd_br_extfloodfill,   /* LSD_FLOODFILL		32 */
    lsd_br_extfloodfill,   /* LSD_EXTFLOODFILL		33 */
    lsd_br_paintrgn,		/* LSD_FILLRGN			34 */
    lsd_br_framergn,		/* LSD_FRAMERGN			35 */
    lsd_br_paintrgn,		/* LSD_INVERTRGN		36 */
    lsd_br_paintrgn,		/* LSD_PAINTRGN			37 */
    lsd_br_textout,		/* LSD_TEXTOUT			38 */
    lsd_br_exttextout,		/* LSD_EXTTEXTOUT		39 */
    lsd_br_passthru,		/* LSD_GETCHARWIDTH		40 */
    lsd_br_passthru,		/* LSD_GETASPECTRATIO   41 */
    lsd_br_passthru,		/* LSD_GETTEXTMETRICS   42 */
    lsd_br_passthru,		/* LSD_ENUMFONTS		43 */
    lsd_br_passthru,		/* LSD_GETTEXTFACE		44 */
#ifdef	LATER
    /* there is a bunch of TT functions to be added here */
#endif
    lsd_br_stretchblt,		/* LSD_BITBLT			45 */
    lsd_br_stretchblt,		/* LSD_PATBLT			46 */
    lsd_br_stretchblt,		/* LSD_STRETCHBLT		47 */
    lsd_br_stretchblt,		/* LSD_STRETCHDIBITS		48 */
    lsd_br_setpixel,		/* LSD_SETPIXEL			49 */
    lsd_br_passthru,		/* LSD_GETPIXEL			50 */
    lsd_br_stretchblt,		/* LSD_SETDIBTODEVICE		51 */
    lsd_br_scrolldc,		/* LSD_SCROLLDC			52 */
    lsd_br_passthru,		/* LSD_STARTDOC			53 */
    lsd_br_passthru,		/* LSD_STARTPAGE		54 */
    lsd_br_passthru,		/* LSD_ENDPAGE			55 */
    lsd_br_passthru,		/* LSD_ENDDOC			56 */
    lsd_br_passthru,		/* LSD_ABORTDOC			57 */
    lsd_br_passthru,		/* LSD_SETABORTPROC		58 */
    lsd_br_passthru,		/* LSD_QUERYABORT		59 */
    lsd_br_passthru,		/* LSD_ESCAPE			60 */
    lsd_br_worldblt,		/* LSD_MASKBLT		61 */
    lsd_br_worldblt,		/* LSD_PLGBLT		62 */
    lsd_br_passthru,		/* LSD_SYSTEMPALETTEUSE	63 */
    lsd_br_passthru,		/* LSD_GETSYSTEMPALETTE	64 */
    lsd_br_passthru,		/* LSD_UPDATECOLORS	65 */
    lsd_br_passthru,		/* LSD_GETGLYPHOUTLINE	66 */
};

DWORD
DCDisplayProc(WORD wFunc, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return (DCDisplayTable[wFunc])(wFunc, hDC32, dwParam, lpStruct);
}

DWORD
DCPrinterProc(WORD wFunc, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return (DCPrinterTable[wFunc])(wFunc, hDC32, dwParam, lpStruct);
}

DWORD
DCMetaProc(WORD wFunc, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return (DCMetaTable[wFunc])(wFunc, hDC32, dwParam, lpStruct);
}

DWORD
DCMapModeProc(WORD wFunc, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return (DCMapModeTable[wFunc])(wFunc, hDC32, dwParam, lpStruct);
}

DWORD
DCBoundsRectProc(WORD wFunc, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return (DCBoundsRectTable[wFunc])(wFunc, hDC32, dwParam, lpStruct);
}

