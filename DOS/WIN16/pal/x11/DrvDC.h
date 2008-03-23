/*
	@(#)DrvDC.h	2.6
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

#include <X11/Xlib.h>
#include "DrvDP.h"	/* PRIVATEDISPLAY */
#include "DrvImages.h"	/* DRVIMAGEDATA */
#include "DrvText.h"	/* DRVFONTDATA */

typedef struct tagDRIVERDC
{
    GC		gc;		/* graphics context 		*/
    Pixmap	Drawable;	/* drawable (win or pixmap)	*/
    PRIVATEDISPLAY  *dp;	/* pointer to display/screen 	*/
    DWORD	dwDCFlags;	/* DCX flags			*/

    POINT	cpt;		/* current MoveTo point		*/

    int		BkFillMode;	/* OPAQUE/TRASPARENT		     */
    int		RopFunction;	/* logical operation on GC	*/
    int		StretchMode;	/* SetStretchBltMode		*/

    LONG	TextColorPixel;	/* Pixel for TextColor		*/
    LONG	PenColorPixel;	/* Pixel for PenColor		*/
    LONG	BrushColorPixel;/* Pixel for BrushColor		*/
    LONG 	ForeColorPixel;	/* Last set foreground color pixel */
    LONG	BackColorPixel;	/* Pixel for BkColor		*/

    int		FillStyle;	/* Last set fill style		*/
    int		BrushFillStyle;	/* FillSolid/FillOpaqueStippled, etc */

    int		BrushFlag;	/* BFP_XXX flag			*/
    DWORD	BrushXOrigin;
    LPDRVIMAGEDATA lpDrvImage;  /* currently selected bitmap	*/

    int		PenStyle;	/* PS_XXX			*/

    LPDRVFONTDATA lpFontData;	/* driver font data ptr */

    POINT	cpClipOrigin;	/* clipping origin (DCOrg)	*/
    Region	ClipRegion;	/* clipping region	*/
    Region 	UpdateRegion;	/* update region 	*/
    Region 	VisibleRegion;	/* visible region 	*/

    /* palette index translation table (from logical to hardware palette) */
    WORD	wPaletteSize;
    LPWORD	lpPaletteIndexTable;
    LPPALETTEENTRY lpPaletteEntryTable;

} DRIVERDC;

typedef DRIVERDC *LPDRIVERDC;
