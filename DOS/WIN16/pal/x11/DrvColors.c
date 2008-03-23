/*
	@(#)DrvColors.c	1.40
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

#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "windows.h"

#include "compat.h"
#include "Log.h"
#include "GdiObjects.h"
#include "DrvDC.h"
#include "DrvDP.h"
#include "Driver.h"

	/* enable new palette code */
#define	TWIN_PALETTE

#define xx_to_xxxx(x)	((unsigned short) (((x) * 0xffffUL) / 0xffUL))
#define xxxx_to_xx(x)	((unsigned char) (((x) * 0xffUL) / 0xffffUL))

/* exported inside the driver */
DWORD DrvMakePixel(COLORREF, LPDRIVERDC);
void DrvGetXColor(COLORREF,XColor *);

extern LPVOID WinCalloc( unsigned int units, unsigned int size );

Display *display;
static BOOL bPaletteDevice;

/* (Color) Device Colormap ************************************************* */

static int DeviceColormapSize = 0;
static Colormap DeviceColormap = (Colormap) 0;
static XColor *DeviceColormapValues = (XColor *) 0;
static unsigned long pixel0;
static unsigned long pixel_mask0, pixel_mask, pixel_masks[32];
static int pixel_numplanes;

static BOOL CreateDeviceColormap(Display *display, int screen,
	int minplanes, int maxplanes)
{
	int i, j;
	unsigned long *pixels, planes[32];

	/* pass 1: allocate colors from default colormap
	 * pass 2: create colormap & allocate colors from new colormap
	 */
	DeviceColormapSize = (XDefaultVisual(display, screen))->map_entries;
	for (i = 0; i < 2; i++)
	{
		/* pass 1: get default colormap
		 * pass 2: create custom colormap
		 */
		if (i == 0)
		{
			DeviceColormap = XDefaultColormap(display, screen);
		}
		else
		{
			DeviceColormap = XCreateColormap(display,
				XRootWindow(display, screen),
				XDefaultVisual(display, screen),
				AllocNone);
		}
		/* pass 1: save default colormap colors for pass 2
		 * pass 2: initialize with default colormap colors to minimize
		 *	   techni-color(tm) effect
		 */
		if (i == 0)
		{
			DeviceColormapValues = (XColor *)
				WinMalloc(DeviceColormapSize * sizeof(XColor));
			for (j = 0; j < DeviceColormapSize; j++)
			{
				DeviceColormapValues[j].pixel = j;
			}
			XQueryColors(display, DeviceColormap,
				DeviceColormapValues,
				DeviceColormapSize);
		}
		else
		{
			pixels = (unsigned long *) WinMalloc(DeviceColormapSize *
				sizeof(unsigned long));
			XAllocColorCells(display, DeviceColormap, True,
				planes, 0, pixels, DeviceColormapSize);
			XStoreColors(display, DeviceColormap,
				DeviceColormapValues,
				DeviceColormapSize);
			XFreeColors(display, DeviceColormap, pixels,
				DeviceColormapSize, 0L);
		}
		/* allocate as many color planes as possible */
		if ((minplanes < 1) || (maxplanes < 1))
			break;
		for (j = maxplanes; j >= minplanes; j--)
		{
			if (!XAllocColorCells(display, DeviceColormap, True,
				pixel_masks, j, &pixel0, 1))
			{
				continue;
			}
			pixel_numplanes = j;
			pixel_mask0 = ~0;
			for (pixel_mask = 0, j = 0; j < pixel_numplanes; j++)
			{
				pixel_mask |= pixel_masks[j];
				pixel_mask0 = min(pixel_mask0, pixel_masks[j]);
			}
			break;
		}
		if (j >= minplanes)
			break;
	}
	if (i == 2)
	{
		ERRSTR((LF_WARNING, 
			"%s: Unable to alloc %d to %d planes.\n",
			"CreateDeviceColormap",
			minplanes, maxplanes));
		return (FALSE);
	}

	/* success */
	return (TRUE);

}

/* (Color) Default Palette ************************************************* */

#define DefaultPaletteSize\
	(sizeof(DefaultPalette)/sizeof(DefaultPalette[0]))

#define DefaultPaletteLoStaticUse\
	(DefaultPaletteSize/2)

#define DefaultPaletteHiStaticUse\
	(DefaultPaletteSize - DefaultPaletteLoStaticUse)

#define DefaultPaletteLoNoStaticUse\
	(1)

#define DefaultPaletteHiNoStaticUse\
	(1)

#define DefaultPaletteLoStaticIndex\
	(0)

#define DefaultPaletteHiStaticIndex\
	(DefaultPaletteSize - DefaultPaletteHiStaticUse)

#define DefaultPaletteLoNoStaticIndex\
	(0)

#define DefaultPaletteHiNoStaticIndex\
	(DefaultPaletteSize - DefaultPaletteHiNoStaticUse)

static PALETTEENTRY DefaultPalette[] =
{
	/* first 10 system palette entries */
	{ 0x00, 0x00, 0x00, 0 },		/* black */
	{ 0x80, 0x00, 0x00, 0 },		/* dark red */
	{ 0x00, 0x80, 0x00, 0 },		/* dark green */
	{ 0x80, 0x80, 0x00, 0 },		/* dark yellow */
	{ 0x00, 0x00, 0x80, 0 },		/* dark blue */
	{ 0x80, 0x00, 0x80, 0 },		/* dark magenta */
	{ 0x00, 0x80, 0x80, 0 },		/* dark cyan */
	{ 0xc0, 0xc0, 0xc0, 0 },		/* light gray */
	{ 0xc0, 0xdc, 0xc0, 0 },		/* money green */
	{ 0xa6, 0xca, 0xf0, 0 },		/* sky blue */
	/* last 10 system palette entries */
	{ 0xff, 0xfb, 0xf0, 0 },		/* cream */
	{ 0xa0, 0xa0, 0xa4, 0 },		/* light gray */
	{ 0x80, 0x80, 0x80, 0 },		/* medium gray */
	{ 0xff, 0x00, 0x00, 0 },		/* red */
	{ 0x00, 0xff, 0x00, 0 },		/* green */
	{ 0xff, 0xff, 0x00, 0 },		/* yellow */
	{ 0x00, 0x00, 0xff, 0 },		/* blue */
	{ 0xff, 0x00, 0xff, 0 },		/* magenta */
	{ 0x00, 0xff, 0xff, 0 },		/* cyan */
	{ 0xff, 0xff, 0xff, 0 },		/* white */
};

static HPALETTE CreateDefaultPalette()
{
	union
	{
		LOGPALETTE LogPalette;
		PALETTEENTRY pad[sizeof(LOGPALETTE)/sizeof(PALETTEENTRY)
			+ DefaultPaletteSize];
	} u;
	int i;

	/* create logical palette from default palette */
	u.LogPalette.palVersion = 0x0300;
	u.LogPalette.palNumEntries = DefaultPaletteSize;
	for (i = 0; i < DefaultPaletteSize; i++)
		u.LogPalette.palPalEntry[i] = DefaultPalette[i];
	return (CreatePalette(&u.LogPalette));

}

/* (Color) System Palette ************************************************** */

static int SystemPaletteSize = 0;
static int SystemPaletteLoStaticUse = 0;
static int SystemPaletteHiStaticUse = 0;
static int SystemPaletteLoNoStaticUse = 0;
static int SystemPaletteHiNoStaticUse = 0;
static int SystemPaletteLoStaticIndex = 0;
static int SystemPaletteHiStaticIndex = 0;
static int SystemPaletteLoNoStaticIndex = 0;
static int SystemPaletteHiNoStaticIndex = 0;

static LPPALETTEENTRY SystemPalette = (LPPALETTEENTRY) 0;
static LPDWORD SystemPaletteMapper = (LPDWORD) 0;
static LPINT SystemPalettePixelUse = (LPINT) 0;

static BOOL CreateSystemPaletteMapper()
{
	int i;

	/* create system palette mapper once */
	if (SystemPaletteMapper)
		return (TRUE);

	/* create system palette mapper */
	if (!(SystemPaletteMapper = (LPDWORD)
		WinMalloc(SystemPaletteSize * sizeof(DWORD))))
	{
		ERRSTR((LF_WARNING, "%s: Unable to create color table.\n",
			"CreateSystemPaletteMapper"));
		return (FALSE);
	}

	/* initialize system palette mapper */
	for (i = 0; i < SystemPaletteSize; i++)
	{
		SystemPaletteMapper[i] = pixel0 | (i * pixel_mask0);
	}
	return (TRUE);

}

static BOOL CreateSystemPalettePixelUse()
{
	int i;

	/* create system palette pixel use counter once */
	if (SystemPalettePixelUse)
		return (TRUE);

	/* create system palette pixel use counter */
	if (!(SystemPalettePixelUse = (LPINT)
		WinMalloc(SystemPaletteSize * sizeof(INT))))
	{
		ERRSTR((LF_WARNING,"%s: Unable to create color palette.\n",
			"CreateSystemPalettePixelUse"));
		return (FALSE);
	}

	/* initialize system palette pixel use counter */
	for (i = 0; i < SystemPaletteSize; i++)
	{
		SystemPalettePixelUse[i] = -1;
	}
	return (TRUE);

}

static BOOL CreateSystemPalette()
{
	int i, j,
	    SystemPaletteLoIndex0, SystemPaletteLoIndex1,
	    SystemPaletteHiIndex0, SystemPaletteHiIndex1;
	XColor xc;

	/* compute system palette index limits */
	SystemPaletteSize = 1 << pixel_numplanes;
	SystemPaletteLoStaticUse = DefaultPaletteLoStaticUse;
	SystemPaletteHiStaticUse = DefaultPaletteHiStaticUse;
	SystemPaletteLoNoStaticUse = DefaultPaletteLoNoStaticUse;
	SystemPaletteHiNoStaticUse = DefaultPaletteHiNoStaticUse;
	SystemPaletteLoStaticIndex = DefaultPaletteLoStaticIndex;
	SystemPaletteHiStaticIndex = SystemPaletteSize
		- DefaultPaletteHiStaticUse;
	SystemPaletteLoNoStaticIndex = DefaultPaletteLoNoStaticIndex;
	SystemPaletteHiNoStaticIndex = SystemPaletteSize
		- DefaultPaletteHiNoStaticUse;

	/* create system palette */
	if (!(SystemPalette = (LPPALETTEENTRY) WinMalloc(SystemPaletteSize *
		sizeof(PALETTEENTRY))))
	{
		ERRSTR((LF_WARNING,"%s: Unable to create system color palette.\n",
			"CreateSystemPalette"));
		return (FALSE);
	}

	/* initialize system palette */
	for (i = 0, j = pixel0; i < SystemPaletteSize; i++, j += pixel_mask0)
	{
		SystemPalette[i].peRed =
			xxxx_to_xx(DeviceColormapValues[j].red);
		SystemPalette[i].peGreen =
			xxxx_to_xx(DeviceColormapValues[j].green);
		SystemPalette[i].peBlue =
			xxxx_to_xx(DeviceColormapValues[j].blue);
		SystemPalette[i].peFlags = 0;
	}

	/* create system palette mapper */
	if (!CreateSystemPaletteMapper())
		return (FALSE);

	/* create system palette pixel use counter */
	if (!CreateSystemPalettePixelUse())
		return (FALSE);

	/* get system palette index limits */
	SystemPaletteLoIndex0 = SystemPaletteLoStaticIndex;
	SystemPaletteLoIndex1 = SystemPaletteLoStaticIndex
		+ SystemPaletteLoStaticUse;
	SystemPaletteHiIndex0 = SystemPaletteHiStaticIndex;
	SystemPaletteHiIndex1 = SystemPaletteHiStaticIndex
		+ SystemPaletteHiStaticUse;

	/* set system palette low static colors */
	for (i = SystemPaletteLoIndex0; i < SystemPaletteLoIndex1; i++)
	{
		/* low static colors cannot use system palette high half */
		if (i >= SystemPaletteSize / 2)
		{
			continue;
		}
		/* set system palette color */
		SystemPalette[i] =
			DefaultPalette[DefaultPaletteLoStaticIndex + i];
		/* set hardware color */
		xc.pixel = SystemPaletteMapper[i];
		xc.red = xx_to_xxxx(SystemPalette[i].peRed);
		xc.green = xx_to_xxxx(SystemPalette[i].peGreen);
		xc.blue = xx_to_xxxx(SystemPalette[i].peBlue);
		xc.flags = DoRed | DoGreen | DoBlue;
		XStoreColor(display, DeviceColormap, &xc);
	}

	/* set system palette unused colors */
	for (i = SystemPaletteLoIndex1; i < SystemPaletteHiIndex0; i++)
	{
		float rainbow;
		BYTE r, g, b;
		/* set system palette color */
		rainbow = i - SystemPaletteLoIndex1 + 1.0;
		rainbow /= SystemPaletteHiIndex0 - SystemPaletteLoIndex1 + 1.0;
		r = (BYTE) (((long)(rainbow * 0x00000100L)) % 0x0100L);
		g = (BYTE) (((long)(rainbow * 0x00010000L)) % 0x0100L);
		b = (BYTE) (((long)(rainbow * 0x01000000L)) % 0x0100L);
		SystemPalette[i].peRed = r;
		SystemPalette[i].peGreen = g;
		SystemPalette[i].peBlue = b;
		/* set hardware color */
		xc.pixel = SystemPaletteMapper[i];
		xc.red = xx_to_xxxx(SystemPalette[i].peRed);
		xc.green = xx_to_xxxx(SystemPalette[i].peGreen);
		xc.blue = xx_to_xxxx(SystemPalette[i].peBlue);
		xc.flags = DoRed | DoGreen | DoBlue;
		XStoreColor(display, DeviceColormap, &xc);
	}

	/* set system palette high static colors */
	for (i = SystemPaletteHiIndex0; i < SystemPaletteHiIndex1; i++)
	{
		/* high static colors cannot use system palette low half */
		if (i < SystemPaletteSize / 2)
		{
			continue;
		}
		/* set system palette color */
		SystemPalette[i] =
			DefaultPalette[DefaultPaletteHiStaticIndex + i
				- SystemPaletteHiIndex0];
		/* set hardware color */
		xc.pixel = SystemPaletteMapper[i];
		xc.red = xx_to_xxxx(SystemPalette[i].peRed);
		xc.green = xx_to_xxxx(SystemPalette[i].peGreen);
		xc.blue = xx_to_xxxx(SystemPalette[i].peBlue);
		xc.flags = DoRed | DoGreen | DoBlue;
		XStoreColor(display, DeviceColormap, &xc);
	}

	/* success */
	return (TRUE);

}

static BOOL CreateUnusedSystemPalette(UINT uSystemPaletteUse)
{
	int i,
	    SystemPaletteLoIndex0, SystemPaletteLoIndex1,
	    SystemPaletteHiIndex0, SystemPaletteHiIndex1;

	/* get system palette index limits */
	switch (uSystemPaletteUse)
	{
	case SYSPAL_STATIC:
		SystemPaletteLoIndex0 = SystemPaletteLoStaticIndex;
		SystemPaletteLoIndex1 = SystemPaletteLoStaticIndex
			+ SystemPaletteLoStaticUse;
		SystemPaletteHiIndex0 = SystemPaletteHiStaticIndex;
		SystemPaletteHiIndex1 = SystemPaletteHiStaticIndex
			+ SystemPaletteHiStaticUse;
		break;
	case SYSPAL_NOSTATIC:
		SystemPaletteLoIndex0 = SystemPaletteLoNoStaticIndex;
		SystemPaletteLoIndex1 = SystemPaletteLoNoStaticIndex
			+ SystemPaletteLoNoStaticUse;
		SystemPaletteHiIndex0 = SystemPaletteHiNoStaticIndex;
		SystemPaletteHiIndex1 = SystemPaletteHiNoStaticIndex
			+ SystemPaletteHiNoStaticUse;
		break;
	default:
		ERRSTR((LF_WARNING,"%s: Invalid system palette use (%d).\n",
			"CreateUnusedSystemPalette",
			uSystemPaletteUse));
		return (FALSE);
	}

	/* create system palette index usage counter */
	if (!SystemPalettePixelUse
	 && !(SystemPalettePixelUse = (LPINT) WinMalloc(SystemPaletteSize *
		sizeof(INT))))
	{
		ERRSTR((LF_WARNING, 
			"%s: Unable to allocate palette usage counter.\n",
			"CreateUnusedSystemPalette"));
		return (FALSE);
	}

	/* mark unavailable system palette index */
	for (i = 0; i < SystemPaletteLoIndex0; i++)
	{
		if (i >= SystemPaletteSize / 2)
			continue;
		SystemPalettePixelUse[i] = -1;
	}

	/* mark static system palette index */
	for (i = SystemPaletteLoIndex0; i < SystemPaletteLoIndex1; i++)
	{
		if (i >= SystemPaletteSize / 2)
			continue;
		SystemPalettePixelUse[i] = 1;
	}

	/* mark unused system palette index */
	for (i = SystemPaletteLoIndex1; i < SystemPaletteHiIndex0; i++)
	{
#if 0
		SystemPalettePixelUse[i] = 0;
#else
		/* HACK:  This ought to be zero to mark palette entry as
		 * unused.  Instead, we mark as used so that we can color
		 * match with the rainbow colors which we set up in
		 * CreateSystemPalette().  This hack exists only to make
		 * the splash bitmap look nice and pretty on palette devices.
		 * Alternatively, we could inform the splash screen writer
		 * that LoadBitmap() on a 256-color bitmap will always
		 * create a 16-color bitmap under Windows (even on a 256-color
		 * palette device).
		 */
		SystemPalettePixelUse[i] = 1;
#endif
	}

	/* mark static system palette index */
	for (i = SystemPaletteHiIndex0; i < SystemPaletteHiIndex1; i++)
	{
		if (i < SystemPaletteSize / 2)
			continue;
		SystemPalettePixelUse[i] = 1;
	}

	/* mark unavailable system palette index */
	for (i = SystemPaletteHiIndex1; i < SystemPaletteSize; i++)
	{
		if (i < SystemPaletteSize / 2)
			continue;
		SystemPalettePixelUse[i] = -1;
	}

	/* success */
	return (TRUE);

}

static UINT UnusedSystemPaletteIndex()
{
	UINT u;

	/* find unused system palette index */
	for (u = 0; u < SystemPaletteSize; u++)
		if (SystemPalettePixelUse[u] == 0)
			break;
	return (u);

}

static UINT NearestSystemPaletteIndex(LPPALETTEENTRY pe)
{
	UINT u, v;
	LONG delta, delta0, r0, g0, b0;
	XColor xc;

	/* match exact color */
	for (u = 0; u < SystemPaletteSize; u++)
	{
		if (SystemPalettePixelUse[u] <= 0)
			continue;
		if (SystemPalette[u].peRed != pe->peRed)
			continue;
		if (SystemPalette[u].peGreen != pe->peGreen)
			continue;
		if (SystemPalette[u].peBlue != pe->peBlue)
			continue;
		SystemPalettePixelUse[u]++;
		return (u);
	}

	/* match unused system palette index */
	if ((u = UnusedSystemPaletteIndex()) < SystemPaletteSize)
	{
		/* set hardware color */
		xc.pixel = SystemPaletteMapper[u];
		xc.red = xx_to_xxxx(pe->peRed);
		xc.green = xx_to_xxxx(pe->peGreen);
		xc.blue = xx_to_xxxx(pe->peBlue);
		xc.flags = DoRed | DoGreen | DoBlue;
		XStoreColor(display, DeviceColormap, &xc);
		/* set system palette color */
		SystemPalette[u] = *pe;
		SystemPalette[u].peFlags = 0;
		SystemPalettePixelUse[u]++;
		return (u);
	}

	/* match nearest color */
	for (u = v = 0, delta = 0x30000L; v < SystemPaletteSize; v++)
	{
		if (SystemPalettePixelUse[v] <= 0)
			continue;
		r0 = ((LONG) SystemPalette[v].peRed) - ((LONG) pe->peRed);
		g0 = ((LONG) SystemPalette[v].peGreen) - ((LONG) pe->peGreen);
		b0 = ((LONG) SystemPalette[v].peBlue) - ((LONG) pe->peBlue);
		delta0 = (r0 * r0) + (g0 * g0) + (b0 * b0);
		if (delta > delta0)
		{
			u = v;
			delta = delta0;
		}
	}
	SystemPalettePixelUse[u]++;
	return (u);

}

static UINT AllocSystemPaletteIndex(LPPALETTEENTRY pe)
{
	UINT u;
	XColor xc;

	/* PC_RESERVED (after AllocSystemPaletteIndex call) */
	if (pe->peFlags == (PC_RESERVED | PC_EXPLICIT))
	{
		u = (pe->peRed & 0x00ff) | ((pe->peGreen << 8) & 0xff00);
		if ((u < SystemPaletteSize)
		 && (SystemPalette[u].peFlags == (PC_RESERVED | PC_EXPLICIT)))
		{
			return (u);
		}
	}

	/* PC_RESERVED (before AllocSystemPaletteIndex call) */
	if (pe->peFlags == PC_RESERVED)
	{
		if ((u = UnusedSystemPaletteIndex()) < SystemPaletteSize)
		{
			/* set hardware color */
			xc.pixel = SystemPaletteMapper[u];
			xc.red = xx_to_xxxx(pe->peRed);
			xc.green = xx_to_xxxx(pe->peGreen);
			xc.blue = xx_to_xxxx(pe->peBlue);
			xc.flags = DoRed | DoGreen | DoBlue;
			XStoreColor(display, DeviceColormap, &xc);
			/* set system palette color */
			SystemPalette[u] = *pe;
			SystemPalette[u].peFlags |= PC_EXPLICIT;
			SystemPalettePixelUse[u] = -1;
			/* get system palette index */
			pe->peRed = (u & 0xff);
			pe->peGreen = (u & 0xff00) >> 8;
			pe->peFlags |= PC_EXPLICIT;
			return (u);
		}
		return (0);
	}

	/* PC_EXPLICIT */
	if (pe->peFlags == PC_EXPLICIT)
	{
		u = pe->peRed | (pe->peGreen << 8);
		if (u >= SystemPaletteSize)
		{
			ERRSTR((LF_WARNING, "AllocSystemPaletteIndex:"
				" PC_EXPLICIT - hardware palette index"
				" [%4x] exceeds hardware palette size [%4x]\n",
				u, SystemPaletteSize));
			u = 0;
		}
		return (u);
	}

	/* PC_NOCOLLAPSE */
	if (pe->peFlags == PC_NOCOLLAPSE)
	{
		if ((u = UnusedSystemPaletteIndex()) < SystemPaletteSize)
		{
			/* set hardware color */
			xc.pixel = SystemPaletteMapper[u];
			xc.red = xx_to_xxxx(pe->peRed);
			xc.green = xx_to_xxxx(pe->peGreen);
			xc.blue = xx_to_xxxx(pe->peBlue);
			xc.flags = DoRed | DoGreen | DoBlue;
			XStoreColor(display, DeviceColormap, &xc);
			/* set system palette color */
			SystemPalette[u] = *pe;
			SystemPalette[u].peFlags = 0;
			SystemPalettePixelUse[u]++;
			return (u);
		}
	}

	/* default */
	if ((u = NearestSystemPaletteIndex(pe)) < SystemPaletteSize)
	{
		SystemPalettePixelUse[u]++;
	}
	return (u);

}

/* (Color) Non-Palette Device ********************************************** */

static DWORD dwRMask, dwGMask, dwBMask;
static DWORD dwRMapper[256], dwGMapper[256], dwBMapper[256];

/* -------- */

/* standard static system colors */

#ifdef 0
static PALETTEENTRY peSystemPalette[] =
{
	/* first 10 system palette entries */
	{ 0x00, 0x00, 0x00, 0 },		/* black */
	{ 0x80, 0x00, 0x00, 0 },		/* dark red */
	{ 0x00, 0x80, 0x00, 0 },		/* dark green */
	{ 0x80, 0x80, 0x00, 0 },		/* dark yellow */
	{ 0x00, 0x00, 0x80, 0 },		/* dark blue */
	{ 0x80, 0x00, 0x80, 0 },		/* dark magenta */
	{ 0x00, 0x80, 0x80, 0 },		/* dark cyan */
	{ 0xc0, 0xc0, 0xc0, 0 },		/* light gray */
	{ 0xc0, 0xdc, 0xc0, 0 },		/* money green */
	{ 0xa6, 0xca, 0xf0, 0 },		/* sky blue */
	/* last 10 system palette entries */
	{ 0xff, 0xfb, 0xf0, 0 },		/* cream */
	{ 0xa0, 0xa0, 0xa4, 0 },		/* light gray */
	{ 0x80, 0x80, 0x80, 0 },		/* medium gray */
	{ 0xff, 0x00, 0x00, 0 },		/* red */
	{ 0x00, 0xff, 0x00, 0 },		/* green */
	{ 0xff, 0xff, 0x00, 0 },		/* yellow */
	{ 0x00, 0x00, 0xff, 0 },		/* blue */
	{ 0xff, 0x00, 0xff, 0 },		/* magenta */
	{ 0x00, 0xff, 0xff, 0 },		/* cyan */
	{ 0xff, 0xff, 0xff, 0 },		/* white */
};
#endif

/* system palette index to hardware palette index translation table */

DWORD DrvColorsInit(LPARAM,LPARAM,LPVOID);
DWORD DrvColorsTab(void);

static DWORD DrvSystemPaletteUse(LPARAM, LPARAM, LPVOID);
static DWORD DrvGetSystemPalette(LPARAM, LPARAM, LPVOID);
static DWORD DrvSetSystemPalette(LPARAM, LPARAM, LPVOID);
static DWORD DrvAnimatePalette(LPARAM, LPARAM, LPVOID);
static DWORD DrvUpdateColors(LPARAM, LPARAM, LPVOID);
static DWORD DrvColorMakePixel(LPARAM, LPARAM, LPVOID);
static DWORD DrvColorGetColorRef(LPARAM, LPARAM, LPVOID);
static DWORD DrvColorsDoNothing(LPARAM,LPARAM,LPVOID);
static DWORD DrvColorsDoDefault(LPARAM,LPARAM,LPVOID);

static DWORD DrvGetNearestColor(LPARAM, LPARAM, LPVOID);
static DWORD DrvFillRGBTable(LPARAM, LPARAM, LPVOID);
static DWORD DrvInitColors(UINT);
static COLORREF DrvGetColorRef(DWORD);

RGBQUAD RgbColors_128[128] = {
	{ 0x00, 0x00, 0x00, 0 },
	{ 0x00, 0x00, 0x80, 0 },
	{ 0x00, 0x80, 0x00, 0 },
	{ 0x00, 0x80, 0x80, 0 },
	{ 0x80, 0x00, 0x00, 0 },
	{ 0x80, 0x00, 0x80, 0 },
	{ 0x80, 0x80, 0x00, 0 },
	{ 0xc0, 0xc0, 0xc0, 0 },
	{ 0xc0, 0xdc, 0xc0, 0 },
	{ 0xf0, 0xca, 0xa6, 0 },

	{ 0x50, 0x50, 0x50, 0 },
	{ 0x60, 0x60, 0x60, 0 },
	{ 0x70, 0x70, 0x70, 0 },
	{ 0x90, 0x90, 0x90, 0 },
	{ 0xa0, 0xa0, 0xa0, 0 },
	{ 0xb0, 0xb0, 0xb0, 0 },
	{ 0xd0, 0xd0, 0xd0, 0 },
	{ 0xe0, 0xe0, 0xe0, 0 },

	{ 0xc0, 0x00, 0x00, 0 },
	{ 0x00, 0x40, 0x00, 0 },
	{ 0x40, 0x80, 0x00, 0 },
	{ 0xc0, 0x80, 0x00, 0 },
	{ 0xff, 0x80, 0x00, 0 },
	{ 0x00, 0xc0, 0x00, 0 },
	{ 0x40, 0xc0, 0x00, 0 },
	{ 0x80, 0xc0, 0x00, 0 },
	{ 0xc0, 0xc0, 0x00, 0 },
	{ 0xff, 0xc0, 0x00, 0 },
	{ 0x40, 0xff, 0x00, 0 },
	{ 0x80, 0xff, 0x00, 0 },
	{ 0xc0, 0xff, 0x00, 0 },
	{ 0x40, 0x00, 0x40, 0 },
	{ 0x80, 0x00, 0x40, 0 },
	{ 0xc0, 0x00, 0x40, 0 },
	{ 0xff, 0x00, 0x40, 0 },
	{ 0x00, 0x40, 0x40, 0 },
	{ 0x40, 0x40, 0x40, 0 },
	{ 0x80, 0x40, 0x40, 0 },
	{ 0xc0, 0x40, 0x40, 0 },
	{ 0xff, 0x40, 0x40, 0 },
	{ 0x00, 0x80, 0x40, 0 },
	{ 0x40, 0x80, 0x40, 0 },
	{ 0x80, 0x80, 0x40, 0 },
	{ 0xc0, 0x80, 0x40, 0 },
	{ 0xff, 0x80, 0x40, 0 },
	{ 0x00, 0xc0, 0x40, 0 },
	{ 0x40, 0xc0, 0x40, 0 },
	{ 0x80, 0xc0, 0x40, 0 },
	{ 0xc0, 0xc0, 0x40, 0 },
	{ 0xff, 0xc0, 0x40, 0 },
	{ 0xc0, 0xff, 0x40, 0 },
	{ 0xff, 0xff, 0x40, 0 },
	{ 0x40, 0x00, 0x80, 0 },
	{ 0xc0, 0x00, 0x80, 0 },
	{ 0xff, 0x00, 0x80, 0 },
	{ 0x00, 0x40, 0x80, 0 },
	{ 0x40, 0x40, 0x80, 0 },
	{ 0x80, 0x40, 0x80, 0 },
	{ 0xc0, 0x40, 0x80, 0 },
	{ 0xff, 0x40, 0x80, 0 },
	{ 0x40, 0x80, 0x80, 0 },
	{ 0xc0, 0x80, 0x80, 0 },
	{ 0xff, 0x80, 0x80, 0 },
	{ 0x00, 0xc0, 0x80, 0 },
	{ 0x40, 0xc0, 0x80, 0 },
	{ 0x80, 0xc0, 0x80, 0 },
	{ 0xc0, 0xc0, 0x80, 0 },
	{ 0xff, 0xc0, 0x80, 0 },
	{ 0x00, 0xff, 0x80, 0 },
	{ 0x40, 0xff, 0x80, 0 },
	{ 0x80, 0xff, 0x80, 0 },
	{ 0xc0, 0xff, 0x80, 0 },
	{ 0xff, 0xff, 0x80, 0 },
	{ 0x00, 0x00, 0xc0, 0 },
	{ 0x40, 0x00, 0xc0, 0 },
	{ 0x80, 0x00, 0xc0, 0 },
	{ 0xc0, 0x00, 0xc0, 0 },
	{ 0xff, 0x00, 0xc0, 0 },
	{ 0x00, 0x40, 0xc0, 0 },
	{ 0x40, 0x40, 0xc0, 0 },
	{ 0x80, 0x40, 0xc0, 0 },
	{ 0xc0, 0x40, 0xc0, 0 },
	{ 0xff, 0x40, 0xc0, 0 },
	{ 0x00, 0x80, 0xc0, 0 },
	{ 0x40, 0x80, 0xc0, 0 },
	{ 0x80, 0x80, 0xc0, 0 },
	{ 0xc0, 0x80, 0xc0, 0 },
	{ 0xff, 0x80, 0xc0, 0 },
	{ 0x00, 0xc0, 0xc0, 0 },
	{ 0x40, 0xc0, 0xc0, 0 },
	{ 0x80, 0xc0, 0xc0, 0 },
	{ 0xff, 0xc0, 0xc0, 0 },
	{ 0x00, 0xff, 0xc0, 0 },
	{ 0x40, 0xff, 0xc0, 0 },
	{ 0x80, 0xff, 0xc0, 0 },
	{ 0xc0, 0xff, 0xc0, 0 },
	{ 0xff, 0xff, 0xc0, 0 },
	{ 0x40, 0x00, 0xff, 0 },
	{ 0x80, 0x00, 0xff, 0 },
	{ 0xc0, 0x00, 0xff, 0 },
	{ 0x00, 0x40, 0xff, 0 },
	{ 0x40, 0x40, 0xff, 0 },
	{ 0x80, 0x40, 0xff, 0 },
	{ 0xc0, 0x40, 0xff, 0 },
	{ 0xff, 0x40, 0xff, 0 },
	{ 0x00, 0x80, 0xff, 0 },
	{ 0x40, 0x80, 0xff, 0 },
	{ 0x80, 0x80, 0xff, 0 },
	{ 0xc0, 0x80, 0xff, 0 },
	{ 0xff, 0x80, 0xff, 0 },
	{ 0x00, 0xc0, 0xff, 0 },
	{ 0x40, 0xc0, 0xff, 0 },
	{ 0x80, 0xc0, 0xff, 0 },
	{ 0xc0, 0xc0, 0xff, 0 },
	{ 0xff, 0xc0, 0xff, 0 },
	{ 0x40, 0xff, 0xff, 0 },
	{ 0x80, 0xff, 0xff, 0 },
	{ 0xc0, 0xff, 0xff, 0 },

	{ 0xf0, 0xfb, 0xff, 0 },
	{ 0xa4, 0xa0, 0xa0, 0 },
	{ 0x80, 0x80, 0x80, 0 },
	{ 0x00, 0x00, 0xff, 0 },
	{ 0x00, 0xff, 0x00, 0 },
	{ 0x00, 0xff, 0xff, 0 },
	{ 0xff, 0x00, 0x00, 0 },
	{ 0xff, 0x00, 0xff, 0 },
	{ 0xff, 0xff, 0x00, 0 },
	{ 0xff, 0xff, 0xff, 0 }
};

RGBQUAD RgbColors_64[64] = {
	{ 0x00, 0x00, 0x00, 0 },
	{ 0x00, 0x00, 0x80, 0 },
	{ 0x00, 0x80, 0x00, 0 },
	{ 0x00, 0x80, 0x80, 0 },
	{ 0x80, 0x00, 0x00, 0 },
	{ 0x80, 0x00, 0x80, 0 },
	{ 0x80, 0x80, 0x00, 0 },
	{ 0xc0, 0xc0, 0xc0, 0 },
	{ 0xc0, 0xdc, 0xc0, 0 },
	{ 0xf0, 0xca, 0xa6, 0 },

	{ 0x40, 0x40, 0x40, 0 },
	{ 0x80, 0x40, 0x00, 0 },
	{ 0xc0, 0x40, 0x00, 0 },
	{ 0xff, 0x40, 0x00, 0 },
	{ 0x00, 0xc0, 0x00, 0 },
	{ 0x80, 0xc0, 0x00, 0 },
	{ 0xc0, 0xc0, 0x00, 0 },
	{ 0xff, 0xc0, 0x00, 0 },
	{ 0x80, 0x80, 0x40, 0 },
	{ 0xc0, 0x80, 0x40, 0 },
	{ 0xff, 0x80, 0x40, 0 },
	{ 0x00, 0x40, 0x80, 0 },
	{ 0xc0, 0x40, 0x80, 0 },
	{ 0xff, 0x40, 0x80, 0 },
	{ 0x00, 0xc0, 0x80, 0 },
	{ 0x80, 0xc0, 0x80, 0 },
	{ 0xc0, 0xc0, 0x80, 0 },
	{ 0xff, 0xc0, 0x80, 0 },
	{ 0x00, 0xff, 0x80, 0 },
	{ 0x80, 0xff, 0x80, 0 },
	{ 0xc0, 0xff, 0x80, 0 },
	{ 0xff, 0xff, 0x80, 0 },
	{ 0x00, 0x00, 0xc0, 0 },
	{ 0x80, 0x00, 0xc0, 0 },
	{ 0xc0, 0x00, 0xc0, 0 },
	{ 0xff, 0x00, 0xc0, 0 },
	{ 0x00, 0x80, 0xc0, 0 },
	{ 0x80, 0x80, 0xc0, 0 },
	{ 0xc0, 0x80, 0xc0, 0 },
	{ 0xff, 0x80, 0xc0, 0 },
	{ 0x00, 0xff, 0xc0, 0 },
	{ 0x80, 0xff, 0xc0, 0 },
	{ 0xc0, 0xff, 0xc0, 0 },
	{ 0xff, 0xff, 0xc0, 0 },
	{ 0x40, 0x00, 0xff, 0 },
	{ 0x80, 0x00, 0xff, 0 },
	{ 0xc0, 0x00, 0xff, 0 },
	{ 0x00, 0xc0, 0xff, 0 },
	{ 0x80, 0xc0, 0xff, 0 },
	{ 0xc0, 0xc0, 0xff, 0 },
	{ 0xff, 0xc0, 0xff, 0 },
	{ 0x40, 0xff, 0xff, 0 },
	{ 0x80, 0xff, 0xff, 0 },
	{ 0xc0, 0xff, 0xff, 0 },

	{ 0xf0, 0xfb, 0xff, 0 },
	{ 0xa4, 0xa0, 0xa0, 0 },
	{ 0x80, 0x80, 0x80, 0 },
	{ 0x00, 0x00, 0xff, 0 },
	{ 0x00, 0xff, 0x00, 0 },
	{ 0x00, 0xff, 0xff, 0 },
	{ 0xff, 0x00, 0x00, 0 },
	{ 0xff, 0x00, 0xff, 0 },
	{ 0xff, 0xff, 0x00, 0 },
	{ 0xff, 0xff, 0xff, 0 }
};

RGBQUAD RgbColors_32[32] = {
	{ 0x00, 0x00, 0x00, 0 },
	{ 0x00, 0x00, 0x80, 0 },
	{ 0x00, 0x80, 0x00, 0 },
	{ 0x00, 0x80, 0x80, 0 },
	{ 0x80, 0x00, 0x00, 0 },
	{ 0x80, 0x00, 0x80, 0 },
	{ 0x80, 0x80, 0x00, 0 },
	{ 0xc0, 0xc0, 0xc0, 0 },
	{ 0xc0, 0xdc, 0xc0, 0 },
	{ 0xf0, 0xca, 0xa6, 0 },

	{ 0xff, 0x80, 0x00, 0 },
	{ 0x80, 0xff, 0x00, 0 },
	{ 0xff, 0x00, 0x80, 0 },
	{ 0xff, 0x80, 0x80, 0 },
	{ 0x00, 0xff, 0x80, 0 },
	{ 0x80, 0xff, 0x80, 0 },
	{ 0xff, 0xff, 0x80, 0 },
	{ 0x80, 0x00, 0xff, 0 },
	{ 0x00, 0x80, 0xff, 0 },
	{ 0x80, 0x80, 0xff, 0 },
	{ 0xff, 0x80, 0xff, 0 },
	{ 0x80, 0xff, 0xff, 0 },

	{ 0x40, 0x40, 0x40, 0 },
	{ 0xf0, 0xfb, 0xff, 0 },
	{ 0x80, 0x80, 0x80, 0 },
	{ 0x00, 0x00, 0xff, 0 },
	{ 0x00, 0xff, 0x00, 0 },
	{ 0x00, 0xff, 0xff, 0 },
	{ 0xff, 0x00, 0x00, 0 },
	{ 0xff, 0x00, 0xff, 0 },
	{ 0xff, 0xff, 0x00, 0 },
	{ 0xff, 0xff, 0xff, 0 }
};

RGBQUAD RgbColors_16[16] = {
	{ 0x00, 0x00, 0x00, 0 },
	{ 0x00, 0x00, 0x80, 0 },
	{ 0x00, 0x80, 0x00, 0 },
	{ 0x00, 0x80, 0x80, 0 },
	{ 0x80, 0x00, 0x00, 0 },
	{ 0x80, 0x00, 0x80, 0 },
	{ 0x80, 0x80, 0x00, 0 },
	{ 0xc0, 0xc0, 0xc0, 0 },

	{ 0x80, 0x80, 0x80, 0 },
	{ 0x00, 0x00, 0xff, 0 },
	{ 0x00, 0xff, 0x00, 0 },
	{ 0x00, 0xff, 0xff, 0 },
	{ 0xff, 0x00, 0x00, 0 },
	{ 0xff, 0x00, 0xff, 0 },
	{ 0xff, 0xff, 0x00, 0 },
	{ 0xff, 0xff, 0xff, 0 }
};

typedef struct tagXRGB
{
    WORD	red,green,blue,pad;
} XRGB, *LPXRGB;

typedef struct tagCOLORMAPPROPERTY
{
    LONG	nRefCount;
    LONG	nNumColors;
    DWORD 	nBasePixel;
    DWORD 	nPlaneMask;
    XRGB	xrgb[1];
} COLORMAPPROPERTY, *LPCOLORMAPPROPERTY;

#define		COLOR_STEP	0x400

#define		itoc(x)		(((int)(x) * ((int)0xFFFF)) / ((int)0x00FF))
#define		ctoi(x)		(((int)(x) * ((int)0x00FF)) / ((int)0xFFFF))

static BOOL	fTrueColor = FALSE;

static TWINDRVSUBPROC DrvColorsEntryTab[] = {
	DrvColorsInit,
	DrvColorsDoNothing, /* GETCAPS */
	DrvColorsDoNothing, /* EVENTS */
	DrvColorsDoDefault, /* PLH_DEFAULTPALETTE */
	DrvSystemPaletteUse,
	DrvGetSystemPalette,
	DrvSetSystemPalette,
	DrvAnimatePalette,
	DrvUpdateColors,
	DrvColorMakePixel,
	DrvGetNearestColor,
	DrvColorGetColorRef,
	DrvFillRGBTable,
	DrvColorsDoDefault, /*PLH_MAKEPALENTRY */
	DrvColorsDoDefault /* PLH_GETPIXELINFO */
};

DWORD
DrvColorsInit(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	DWORD dwReturn;

	if (dwParam2)
	{
		dwReturn = DrvInitColors((UINT)dwParam1);
		return dwReturn;
	}
	else
		return 0L; 
}

static DWORD 
DrvSystemPaletteUse(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpParams)
{
	LPDRIVERDC lpDriverDC = (LPDRIVERDC)dwParam1;
	UINT uSystemPaletteUse = (UINT)dwParam2;
	BOOL bSetSystemPaletteUse = (BOOL)lpParams;
	UINT uOldSystemPaletteUse;

	/* verify palette device */
	if (!bPaletteDevice)
	{
		ERRSTR((LF_WARNING, "DrvSystemPaletteUse:"
			" Non-palette device.\n"));
		SetLastErrorEx(1, 0);
		return (DWORD)(SYSPAL_ERROR);
	}

	/* get system palette use */
	uOldSystemPaletteUse = lpDriverDC->dp->uSystemPaletteUse;

	/* set system palette use */
	if (bSetSystemPaletteUse)
	{
		switch (uSystemPaletteUse)
		{
		case SYSPAL_STATIC:
			lpDriverDC->dp->uSystemPaletteUse = uSystemPaletteUse;
			break;
		case SYSPAL_NOSTATIC:
			lpDriverDC->dp->uSystemPaletteUse = uSystemPaletteUse;
			break;
		default:
			ERRSTR((LF_WARNING, "DrvSystemPaletteUse:"
				" Unknown system palette use %d\n",
				uSystemPaletteUse));
			uOldSystemPaletteUse = SYSPAL_ERROR;
			break;
		}
	}

	/* return old system palette use */
	return (DWORD)(uOldSystemPaletteUse);

}

static DWORD 
DrvGetSystemPalette(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpParams)
{
	/* (non-palette) */
	if (!bPaletteDevice)
		return (DWORD)(CreateDefaultPalette());

	/* (palette) */
	return (DWORD)(CreateDefaultPalette());


}

static DWORD 
DrvSetSystemPalette(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpParams)
{
	LPDRIVERDC lpDriverDC = (LPDRIVERDC)dwParam1;	
	BOOL bInitSystemPalette = (BOOL)dwParam2;
	LPPALETTEENTRY lpPaletteEntry = (LPPALETTEENTRY)lpParams;

	/* verify palette device */
	if (!bPaletteDevice)
	{
		static UINT Pixel;
		if (bInitSystemPalette)
		{
		  Pixel = 0;
		}
		else
		{
	          Pixel++;
		}
		return Pixel;
	}

	/* initialize system palette */
	if (bInitSystemPalette
	 && !CreateUnusedSystemPalette(lpDriverDC->dp->uSystemPaletteUse))
	{
		ERRSTR((LF_WARNING, "DrvSetSystemPalette:"
			" Initializing palette.\n"));
		return (DWORD)(0);
	}

	/* allocate system palette index */
	return (DWORD)AllocSystemPaletteIndex(lpPaletteEntry);

}

static DWORD
DrvAnimatePalette(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpParams)
{
        UINT uAnimatePaletteIndex = (UINT)dwParam2;
	LPPALETTEENTRY lpAnimatePaletteEntry = (LPPALETTEENTRY)lpParams;
	XColor xc;

	if (uAnimatePaletteIndex >= SystemPaletteSize)
	{
		ERRSTR((LF_WARNING, "DrvAnimatePalette:"
			" Animate palette index [%04x]"
			" >= system palette size [%04x].\n",
			uAnimatePaletteIndex, SystemPaletteSize));
		return (DWORD)(FALSE);
	}

	if (!lpAnimatePaletteEntry)
	{
		ERRSTR((LF_WARNING, "DrvAnimatePalette:"
			" NULL animate palette entry.\n"));
		return (DWORD)(FALSE);
	}

	if (SystemPalettePixelUse[uAnimatePaletteIndex] != -1)
	{
/*
		ERRSTR((LF_WARNING, "DrvAnimatePalette:"
			" Attempt to animate unreserved index [%04x]\n",
			uAnimatePaletteIndex));
		return (DWORD)(FALSE);
*/
	}

	/* set hardware color */
	xc.pixel = SystemPaletteMapper[uAnimatePaletteIndex];
	xc.red = xx_to_xxxx(lpAnimatePaletteEntry->peRed);
	xc.green = xx_to_xxxx(lpAnimatePaletteEntry->peGreen);
	xc.blue = xx_to_xxxx(lpAnimatePaletteEntry->peBlue);
	xc.flags = DoRed | DoGreen | DoBlue;
	XStoreColor(display, DeviceColormap, &xc);
	return (DWORD)(TRUE);

}

static DWORD
DrvUpdateColors(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpParams)
{
	/* verify palette device */
	if (!bPaletteDevice)
	{
		ERRSTR((LF_WARNING, "DrvUpdateColors:"
			" Non-palette device.\n"));
		SetLastErrorEx(1, 0);
		return (DWORD)(FALSE);
	}
	return (DWORD)(FALSE);
}

static COLORREF
DrvGetNearestColor(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	COLORREF cr = (COLORREF)dwParam1;
	BYTE r, g, b;
	DWORD dwPixel;
	PRIVATEDISPLAY *dp = GETDP();
	XColor xc;

	/* (non-palette device) */
	if (!bPaletteDevice)
	{
		r = GetRValue(cr);
		g = GetGValue(cr);
		b = GetBValue(cr);
		dwPixel = (dwRMapper[r] | dwGMapper[g] | dwBMapper[b]);
		r = 255 * (dwPixel & dwRMask) / dwRMask;
		g = 255 * (dwPixel & dwGMask) / dwGMask;
		b = 255 * (dwPixel & dwBMask) / dwBMask;
		return (RGB(r, g, b));
	}

	/* (palette device) */
	if (bPaletteDevice)
	{
		xc.red = itoc(GetRValue(cr));
		xc.green = itoc(GetGValue(cr));
		xc.blue = itoc(GetBValue(cr));
		if (XAllocColor(dp->display, dp->SystemPalette, &xc))
			XFreeColors(dp->display, dp->SystemPalette,
				&xc.pixel, 1, 0L);
		return RGB(ctoi(xc.red), ctoi(xc.green), ctoi(xc.blue));
	}

	return cr;
}

void
DrvGetXColor(COLORREF cr, XColor *xc)
{
    PRIVATEDISPLAY *dp = GETDP();

    if (fTrueColor) {
	xc->red   = itoc(GetRValue(cr)); 
	xc->green = itoc(GetGValue(cr)); 
	xc->blue   = itoc(GetBValue(cr)); 
	xc->flags = DoRed|DoGreen|DoBlue;
    }
    else {
	xc->red = itoc(GetRValue(cr));
	xc->green = itoc(GetGValue(cr));
	xc->blue = itoc(GetBValue(cr));
	if (XAllocColor(dp->display, dp->SystemPalette, xc))
		XFreeColors(dp->display, dp->SystemPalette, &xc->pixel, 1, 0L);
    }
}

/* 
 *	for a given pixel value
 *	lookup its colors and return colorref for it
 *	(note: if its in our table use that entry, otherwise
 *	return the query value...)
 */


static DWORD
DrvColorGetColorRef(LPARAM dwParam1, LPARAM dwParm2, LPVOID lpParams)
{
	return (DWORD)DrvGetColorRef((DWORD)dwParam1);
}

static COLORREF
DrvGetColorRef(DWORD dwPixel)
{
	BYTE r, g, b;
	XColor xc;

	/* (non-palette device) */
	if (!bPaletteDevice)
	{
		r = 255 * (dwPixel & dwRMask) / dwRMask;
		g = 255 * (dwPixel & dwGMask) / dwGMask;
		b = 255 * (dwPixel & dwBMask) / dwBMask;
	}

	/* (palette device) */
	if (bPaletteDevice)
	{
		xc.pixel = pixel0 | (dwPixel & pixel_mask);
		XQueryColor(display, DeviceColormap, &xc);
		r = xxxx_to_xx(xc.red);
		g = xxxx_to_xx(xc.green);
		b = xxxx_to_xx(xc.blue);
	}

	return (RGB(r, g, b));

}

static DWORD 
DrvFillRGBTable(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpParams)
{
    int nNumColors = (int)dwParam1;
    RGBQUAD *lprgb = (RGBQUAD *)lpParams;
    int i;
    COLORREF cr;

	if (nNumColors > SystemPaletteSize)
	{
		ERRSTR((LF_WARNING, "DrvFillRGBTable:"
			" nNumColors [%d] > SystemPaletteSize [%d]\n",
			nNumColors, SystemPaletteSize));
		return (DWORD)(FALSE);
	}

    for (i = 0; i < nNumColors; i++) {
	cr = DrvGetColorRef((DWORD)i);

	lprgb[i].rgbBlue = GetBValue(cr);
	lprgb[i].rgbGreen = GetGValue(cr);
	lprgb[i].rgbRed = GetRValue(cr);
	lprgb[i].rgbReserved = 0;
    }
    return (DWORD)TRUE;
}

/*
 *	given a colorref, return the pixel that goes with it 
 *	normalize to standard pixel values...
 *	try to match
 *	if none found, but free slot exists, then allocate it
 */

static WORD DrvGetNearestPaletteIndex(BYTE r, BYTE g, BYTE b,
	WORD wPaletteSize, LPPALETTEENTRY lpPaletteEntryTable)
{
	WORD w, wNearestPaletteIndex;
	long err, err0, r0, g0, b0;

	for (w = wNearestPaletteIndex = 0, err = 0x30000L;
	     w < wPaletteSize;
	     w++)
	{
		r0 = lpPaletteEntryTable[w].peRed;
		g0 = lpPaletteEntryTable[w].peGreen;
		b0 = lpPaletteEntryTable[w].peBlue;
		err0 = (r - r0) * (r - r0)
			+ (g - g0) * (g - g0)
			+ (b - b0) * (b - b0);
		if (err0 == 0)
			return (w);
		if (err0 < err)
		{
			err = err0;
			wNearestPaletteIndex = w;
		}
	}
	return (wNearestPaletteIndex);

}

static DWORD DrvMakePaletteIndexPixel(WORD wPaletteIndex,
	WORD wPaletteSize, LPWORD lpPaletteIndexTable)
{
	if (wPaletteIndex >= wPaletteSize)
	{
		ERRSTR((LF_WARNING, "DrvMakePaletteIndexPixel():"
			" Palette index [%d] >= palette size [d].\n",
			wPaletteIndex, wPaletteSize));
		wPaletteIndex = 0;
	}
	return (lpPaletteIndexTable[wPaletteIndex]);
}

static void DrvGetPaletteIndexRGB(WORD wPaletteIndex,
	WORD wPaletteSize, LPPALETTEENTRY lpPaletteEntryTable,
	BYTE *r, BYTE *g, BYTE *b)
{
	if (wPaletteIndex >= wPaletteSize)
	{
		ERRSTR((LF_WARNING, "DrvMakePaletteIndexRGB():"
			" Palette index [%d] >= palette size [d].\n",
			wPaletteIndex, wPaletteSize));
		wPaletteIndex = 0;
	}
	*r = lpPaletteEntryTable[wPaletteIndex].peRed;
	*g = lpPaletteEntryTable[wPaletteIndex].peGreen;
	*b = lpPaletteEntryTable[wPaletteIndex].peBlue;
}

static DWORD DrvMakeVGAPalettePixel(BYTE r, BYTE g, BYTE b)
{
/*
	return DrvMakePaletteIndexPixel(DrvGetNearestPaletteIndex(r, g, b,
		SystemPaletteSize, SystemPalette),
		SystemPaletteSize, SystemPaletteMapper);
*/
	PALETTEENTRY pe;

	/* make black pixel */
	if ((r == 0) && (g == 0) && (b == 0))
		return (SystemPaletteMapper[0]);

	/* make white pixel */
	if ((r == 255) && (g == 255) && (b == 255))
		return (SystemPaletteMapper[SystemPaletteSize - 1]);

	/* make other pixel */
	pe.peRed = r;
	pe.peGreen = g;
	pe.peBlue = b;
	pe.peFlags = 0;
	return SystemPaletteMapper[DrvGetNearestPaletteIndex(r, g, b,
		SystemPaletteSize, SystemPalette)];

}

static DWORD DrvMakeNonPalettePixel(BYTE r, BYTE g, BYTE b)
{
	return (dwRMapper[r] | dwGMapper[g] | dwBMapper[b]);
}

static DWORD 
DrvColorMakePixel(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpParams)
{
	return DrvMakePixel((COLORREF)dwParam1, (LPDRIVERDC)dwParam2);
}

DWORD
DrvMakePixel(COLORREF ColorRef, LPDRIVERDC lpDriverDC)
{
	BYTE r, g, b;
	WORD wPaletteIndex;
	DWORD dwPixel = ColorRef;

	/* convert PALETTERGB() color value to pixel value */
	if ((ColorRef & 0xff000000UL) == 0x02000000UL)
	{
		r = GetRValue(ColorRef);
		g = GetGValue(ColorRef);
		b = GetBValue(ColorRef);
		if (bPaletteDevice)
		{
			/* convert PALETTERGB() to PALETTEINDEX() */
			if (!lpDriverDC || !lpDriverDC->lpPaletteEntryTable)
			{
				/* use system palette */
				wPaletteIndex = DrvGetNearestPaletteIndex
					(r, g, b, SystemPaletteSize,
					SystemPalette);
			}
			else
			{
				/* use selected palette */
				wPaletteIndex = DrvGetNearestPaletteIndex
					(r, g, b, lpDriverDC->wPaletteSize,
					lpDriverDC->lpPaletteEntryTable);
			}
			ColorRef = PALETTEINDEX(wPaletteIndex);
		}
		else
		{
			/* convert PALETTERGB() to RGB() */
			ColorRef = RGB(r, g, b);
		}
	}

	/* convert PALETTEINDEX() color value to pixel value */
	if ((ColorRef & 0xff000000UL) == 0x01000000UL)
	{
		wPaletteIndex = (WORD) (ColorRef & 0x0000ffffUL);
		if (bPaletteDevice)
		{
			/* convert PALETTEINDEX() to pixel value */
			if (!lpDriverDC || !lpDriverDC->lpPaletteEntryTable)
			{
				/* use system palette */
/*
				dwPixel = DrvMakePaletteIndexPixel
					(wPaletteIndex, SystemPaletteSize,
					SystemPaletteMapper);
*/
dwPixel = 0;
			}
			else
			{
				/* use selected palette */
				dwPixel = DrvMakePaletteIndexPixel
					(wPaletteIndex,
					lpDriverDC->wPaletteSize,
					lpDriverDC->lpPaletteIndexTable);
			}
			/* return pixel value */
			return (dwPixel);
		}
		else
		{
			/* convert PALETTEINDEX() to RGB() */
			if (!lpDriverDC || !lpDriverDC->lpPaletteEntryTable)
			{
				/* use system palette */
				DrvGetPaletteIndexRGB(wPaletteIndex,
					SystemPaletteSize, SystemPalette,
					&r, &g, &b);
			}
			else
			{
				/* use selected palette */
				DrvGetPaletteIndexRGB(wPaletteIndex,
					lpDriverDC->wPaletteSize,
					lpDriverDC->lpPaletteEntryTable,
					&r, &g, &b);
			}
			ColorRef = RGB(r, g, b);
		}
	}

	/* convert RGB() color value to pixel value */
	if ((ColorRef & 0xff000000UL) == 0x00000000UL)
	{
		r = GetRValue(ColorRef);
		g = GetGValue(ColorRef);
		b = GetBValue(ColorRef);
		if (bPaletteDevice)
		{
			dwPixel = DrvMakeVGAPalettePixel(r, g, b);
		}
		else
		{
			dwPixel = DrvMakeNonPalettePixel(r, g, b);
		}
	}

	/* return pixel value */
	return (dwPixel);

}


static DWORD
DrvInitColors(UINT uiCompatibility)
{
	static BOOL bInitialized = FALSE;
	PRIVATEDISPLAY *dp = GETDP();
	Visual *visual;
	int i;

	/* initialize colors once */
	if (bInitialized)
		return (1L);
	bInitialized = TRUE;

	/* determine whether display is a palette or non-palette device */
	display = dp->display;
	visual = DefaultVisual(dp->display, dp->screen);
	switch (visual->class)
	{
	case StaticGray:
		bPaletteDevice = TRUE;
		break;
	case StaticColor:
		bPaletteDevice = TRUE;
		break;
	case GrayScale:
		bPaletteDevice = TRUE;
		break;
	case PseudoColor:
		bPaletteDevice = TRUE;
		break;
	case TrueColor:
		bPaletteDevice = FALSE;
		break;
	case DirectColor:
		ERRSTR((LF_WARNING, "DrvInitColors: DirectColor not supported\n"));
		return (0L);
	default:
		ERRSTR((LF_WARNING, "DrvInitColors: unknown visual class %d\n",
			visual->class));
		return (0L);
	}

	/* [NONPALETTE] initialize red/green/blue color component info */
	if (!bPaletteDevice)
	{
		dwRMask = visual->red_mask;
		dwGMask = visual->green_mask;
		dwBMask = visual->blue_mask;
		for (i = 0; i < 256; i++)
		{
			dwRMapper[i] = (i * dwRMask / 255) & dwRMask;
			dwGMapper[i] = (i * dwGMask / 255) & dwGMask;
			dwBMapper[i] = (i * dwBMask / 255) & dwBMask;
		}
		dp->pixel_mask = ~0L;
		return (1L);
	}

	/* [PALETTE] initialize palette index info */
	if (!CreateDeviceColormap(dp->display, dp->screen, 5, 8))
		return (0L);
	CreateSystemPalette();
	CreateUnusedSystemPalette(SYSPAL_STATIC);

	/* save colormap for create window code */
	dp->SystemPalette = DeviceColormap;

	/* save pixel mask for bitblt/raster-op code */
	dp->pixel_base = pixel0;
	dp->pixel_mask = pixel_mask;
	dp->nNumPixels = 1 << pixel_numplanes;

	/* success */
	return (1L);

}

DWORD
DrvColorsDoNothing(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return 1L;
}

DWORD
DrvColorsDoDefault(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return 0L;
}

DWORD
DrvColorsTab(void)
{
#if defined(TWIN_RUNTIME_DRVTAB)
	DrvColorsEntryTab[0] = DrvColorsInit;
	DrvColorsEntryTab[1] = DrvColorsDoNothing;
	DrvColorsEntryTab[2] = DrvColorsDoNothing;
	DrvColorsEntryTab[3] = DrvColorsDoDefault;
	DrvColorsEntryTab[4] = DrvSystemPaletteUse;
	DrvColorsEntryTab[5] = DrvGetSystemPalette;
	DrvColorsEntryTab[6] = DrvSetSystemPalette;
	DrvColorsEntryTab[7] = DrvAnimatePalette;
	DrvColorsEntryTab[8] = DrvUpdateColors;
	DrvColorsEntryTab[9] = DrvColorMakePixel;
	DrvColorsEntryTab[10] = DrvGetNearestColor;
	DrvColorsEntryTab[11] = DrvColorGetColorRef;
	DrvColorsEntryTab[12] = DrvFillRGBTable;
	DrvColorsEntryTab[13] = DrvColorsDoDefault;
	DrvColorsEntryTab[14] = DrvColorsDoDefault;
#endif

	return (DWORD)DrvColorsEntryTab;
}
