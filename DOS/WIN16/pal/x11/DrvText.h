/********************************************************************

	@(#)DrvText.h	2.15   Private definitions for text subsustem	
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

*************************************************************************/
 
#include <X11/Xlib.h>

/****************************************************************************
 * Driver Dependent Font Declaration
 * Per Fontface information
 *     	Each of Bold/Normal Italic/Normal have a font list
 *	If height is 0, then this is scalable...
 *	We should add underline/strikethrough and height of bar
 *	
 *	
 ****************************************************************************/
typedef struct tagDRVFONTDATA {
    	UINT		uiFontType;	
	int   		fmheight;	/* what is the height */
	char           *fmstring;	/* what string to load for this font */
	char	       *format;		  /* string to get format            */
	int		aspectx,aspecty;  /* font aspect of font, 75 vs. 100 */
	int		internalleading;  /* point vs. ascent + descent      */ 

	/* display device specific corollaries 				     */
    	Font		Font;		/* device dependent font     */
    	XFontStruct    *finfo;		/* device dependent fontinfo */

	/* windows specific corollaries 				     */
    	LOGFONT		LogFont;	/* the logfont that got us here      */
	NEWTEXTMETRIC  *ntm;		/* new text metrics saved data       */
	struct tagFontInfo    *fp;	/* pointer to base */
	struct tagDRVFONTDATA *fmnext;	/* the next one...    */
} DRVFONTDATA, *LPDRVFONTDATA;

/****************************************************************************
 * Driver Dependent Font Declaration
 * Fontface information
 ****************************************************************************/
typedef struct tagFontInfo {
	struct tagFontInfo *fontnext;
	int		    fontflag;	 /* constant/proportional, 	*/
					 /* serif/sans serif,		*/
					 /* aliased, scalable  		*/
    	UINT		    uiFontType;	
	int		    fontminmax;	 /* DWORD(minimum,maximum) size */
	char 		   *fontface;	 /* the fontface for this group */
	DRVFONTDATA	   *fontinfo[4]; /* one each for bold italics ... */
} DPYFONTINFO;

/* font mapper flags, defined in  [windows]        */
/*				  fontmapper=value */
/*				  low byte is error to enable scaling */

