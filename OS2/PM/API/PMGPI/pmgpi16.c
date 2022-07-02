/*!
   @file

   @ingroup pm

   @brief Presentation Manager 16-bit Graphics Programming Interface

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

*/

/*
 * This is 16-bit part of PM GPI. If it compilation target is not i386 or higher then
 * will be build pure 16-bit PM GPI. If it compilation target is i386 or higher
 * then will be build 16-32 forwarders to 32-bit part of PM GPI.
 * Build tested only by WCC386 compiler.
 */

// OS/2 includes must be from OS/2 2.1 or higher for correct building.
#include <os2.h>

// Private defines
#include "unimpl.h"


// To configure Windows or OS/2 version define one of bellow defines
#ifdef __OS2__
#	define PMOS2
#else
#	define PMWIN
#endif

// To configure 32-bit version define bellow define
#ifdef __386__
#	define BIT32
#endif

// To configure supported API level define it as bellow
//
// APILEVEL   OS/2     Windows
//    1       1.10      1.01
//    2       1.20      1.02
//    3       1.30
//    4       2.00
//    5

#define APILEVEL 2

#if !defined(PMWIN) && !defined(PMOS2)
#	define PMOS2
#endif

#if define(PMWIN) && defined(BIT32)
#	error 32-bit Windows API not supported (yet?)
#endif

#if APILEVEL > 2
#	error This APILEVEL not supported (yet?)
#endif

// Here we define APIENTRY16 APIRET16 and so on macros
#ifndef BIT32
#endif

#include "gpi16dev.c"
#include "gpi16control.c"
#include "gpi16correlation.c"
#include "gpi16segments.c"
#include "gpi16segediting.c"
#include "gpi16transforms.c"
#include "gpi16paths.c"
#include "gpi16logcolortable.c"
#include "gpi16primitives.c"
#include "gpi16lcids.c"
#include "gpi16bitmaps.c"
#include "gpi16regions.c"
#include "gpi16metafiles.c"
#include "gpi16defaults.c"

// fix prototype !!!
USHORT APIENTRY16 GPIVECTORSYMBOL(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIOPENMETAFILE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPICLOSEMETAFILE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 DEVSTDOPEN(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPICONVPSH(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}


// fix prototype !!!
USHORT APIENTRY16 SEGSGWOPENSEGMENTWINDOW(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 MTENDREADREQUEST(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 MTGETDESCRIPTION(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 MTGETCODEPAGE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 MTGETLCT(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 MTGETGDDINFO(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 FMTCONVERTGOCAPOLY(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 MTGETFIRSTFONT(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 SEGSGWNEWPARTDATA(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 MTSTARTREADREQUEST(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 MTGETFIRSTGRAPHICSDATA(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 MTGETNEXTFONT(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 MTGETNEXTGRAPHICSDATA(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}


// fix prototype !!!
USHORT APIENTRY16 GPIACCESSMETAFILE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIMTASSOCIATE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIMTDISASSOCIATE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 SEGSGWNEXTORDERF(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIPATHTOREGION(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

//BOOL    WINAPI FloodFill(HDC, int, int, COLORREF);
// fix prototype !!!
USHORT APIENTRY16 GPIFLOODFILL(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIDRAWBITS(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYBITMAPINFOHEADER(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYLOGICALFONT(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYFACESTRING(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYFONTACTION(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

//HPALETTE WINAPI CreatePalette(const LOGPALETTE FAR*);
// fix prototype !!!
USHORT APIENTRY16 GPICREATEPALETTE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// UINT    WINAPI RealizePalette(HDC);
// fix prototype !!!
USHORT APIENTRY16 GPIDELETEPALETTE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

//HPALETTE WINAPI SelectPalette(HDC, HPALETTE, BOOL);
// fix prototype !!!
USHORT APIENTRY16 GPISELECTPALETTE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

//void    WINAPI AnimatePalette(HPALETTE, UINT, UINT, const PALETTEENTRY FAR*);
// fix prototype !!!
USHORT APIENTRY16 GPIANIMATEPALETTE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// UINT    WINAPI SetPaletteEntries(HPALETTE, UINT, UINT, const PALETTEENTRY FAR*);
// fix prototype !!!
USHORT APIENTRY16 GPISETPALETTEENTRIES(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// UINT    WINAPI GetPaletteEntries(HPALETTE, UINT, UINT, PALETTEENTRY FAR*);
// fix prototype !!!
USHORT APIENTRY16 GPIQUERYPALETTE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYPALETTEINFO(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYCHAREXTRA(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPISETCHAREXTRA(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYCHARBREAKEXTRA(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPISETCHARBREAKEXTRA(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIFRAMEREGION(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}


// fix prototype !!!
USHORT APIENTRY16 GPISETDCOWNER(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPISETBITMAPOWNER(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPISETREGIONOWNER(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIINSTALLIFIFONT(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 DSPINITSYSTEMDRIVERNAME(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}
