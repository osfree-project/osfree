/*
	@(#)DrvDP.h	1.14
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

#ifndef DrvDP__h
#define DrvDP__h

	/* enable new palette code */
#define TWIN_PALETTE

/* display specific information */
typedef struct {
	Display	*display;
	int     screen;
	BOOL	bConnected;

	BOOL 	 HardwareEnabled;
	BOOL 	 SwappedButtons;
	int	 CodePage;

	DWORD	 LastClickTime;
	DWORD 	 DoubleClickTime;
	DWORD	 ServerTime;

	RECT	 rcCursorClip;
	BOOL	 fCursorClipped;

#ifdef TWIN_PALETTE
	/* uSystemPaletteUse
	 *	 - SYSPAL_STATIC (20 static colors in system palette)
	 *	 - SYSPAL_NOSTATIC (2 static colors in system palette)
	 * uSystemPaletteUseCount
	 *	 - use count for each system palette entry
	 *	   (decremented to zero during palette initialization)
	 *	   (incremented by one during palette realization)
	 * hSystemPalette
	 *	 - Windows system palette
	 * hOldSystemPalette
	 *	 - previous Windows system palette (used by UpdateColors())
	 * SystemPalette
	 *	 - X system palette
	 */
	UINT     uSystemPaletteUse;
	LPUINT	 uSystemPaletteUseCount;
	HPALETTE hSystemPalette;
	HPALETTE hOldSystemPalette;
	Colormap SystemPalette;
#else
	Colormap color_map;
#endif

	DWORD	 pixel_base;		/* start of pixel space */
	DWORD	 pixel_mask;		/* pixel bit mask to use */
	int	 nNumPixels;		/* number of X pixels used by us */

	int	 BitsPixel;		/* bits per pixel in ZPixmap image */
	int	 Depth;			/* DefaultDepth of the screen */
	int	 BitOrder;		/* MSBFirst/LSBFirst */
	int	 ByteOrder;		/* image byte order */

	UINT	 ErrorCode;
	LONG	 KeyBoardType;
	LONG	 KeyBoardSubType;
	LONG	 KeyBoardFuncKeys;

	Atom	wm_protocols;		
	Atom	wm_delete_window;

	Atom	ipc_protocols;	

	Atom	XFormatAtom;
	Atom	XDataAtom;
	Atom	XCutBuffer;

	Atom	atmColormapProperty;

	XContext client_hwnd;
	XContext window_style;
	XContext window_ex_style;
} PRIVATEDISPLAY;

PRIVATEDISPLAY *GETDP();

#define	ASSERT_CONNECTION(r)	{ PRIVATEDISPLAY *dp = GETDP(); \
				  if (!dp->bConnected) return (r) ;}

#endif /* DrvDP__h */
