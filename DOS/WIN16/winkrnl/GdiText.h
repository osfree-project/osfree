/*
	@(#)GdiText.h	2.6
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
 
#include "ObjEngine.h"

/* Font information structure */
typedef struct tagFONTINFO
  {
    OBJHEAD	ObjHead;	/* generic object header */
    LOGFONT	LogFont;	/* requested LOGFONT */
    int		nDevHeight;	/* physical height of LRF */
    LPNEWTEXTMETRIC lpNTM;	/* textmetric of the last realized font (LRF) */
    LPVOID	lpCharWidths;	/* char widths array of the LRF */
    LPDWORD     lpCachedFont;   /* cached LRF -- pointer to DRVFONTDATA we */
				/* get from DrvRealizeFont (see DrvText.c), */
				/* where the first word is uiFontType */
  } FONTINFO;

typedef FONTINFO *LPFONTINFO;

typedef LPFONTINFO HFONT32;
#define	GETHFONT32	GETFONTINFO
#define	GETHFONT16(h32)	(HFONT)((h32)->ObjHead.hObj)

#define	ASSERT_HFONT(hFont32,hFont,err) { \
		if (!((hFont32) = GETFONTINFO((hFont)))) { \
		    ERRSTR((LF_ERROR,"***ERROR*** bad Font %x\n",(hFont))); \
		    return (err); \
		}}

