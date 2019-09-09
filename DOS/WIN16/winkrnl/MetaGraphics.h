/*
	@(#)MetaGraphics.h	2.8
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
 
#define META_STRUCT	((LPINTERNALMETAFILE)(hDC32->lpDrvData))
#define META_RECORD(x)	(DWORD)(META_STRUCT->lpfRecorder(META_STRUCT, (x)))

/*	Metafile Record Sizes (in words) */
#define MFRS_MOVETO	5
#define MFRS_POLY	4	/* POLYLINE, POLYGON; plus POINT list */
#define MFRS_RECTANGLE	7
#define MFRS_ROUNDRECT	9
#define MFRS_ARC	11	/* also CHORD, PIE */
#define MFRS_POLYPOLY	4	/* plus Count list, POINT list */
#define MFRS_FLOODFILL	7
#define MFRS_EXTFLOODFILL	8
#define MFRS_TEXTOUT	6	/* plus CHAR list */
#define MFRS_EXTTEXTOUT	7	/* plus CHAR list, RECT, DX list */
#define MFRS_BITBLT	10
#define MFRS_SETPIXEL	7
#define MFRS_SELECTOBJECT	4
#define MFRS_DELETEOBJECT	4
#define MFRS_REALIZEPALETTE	3
#define MFRS_SAVEDC	3
#define MFRS_RESTOREDC	4
#define MFRS_PATBLT	9
