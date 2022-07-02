#include <os2.h>
#include "unimpl.h"


// To configure Windows or OS/2 version define one of bellow defines
//#define PMOS2
//#define PMWIN

// To configure 32-bit version define bellow define
#define BIT32

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
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIOPENMETAFILE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPICLOSEMETAFILE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 DEVSTDOPEN(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPICONVPSH(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!!
USHORT APIENTRY16 SEGSGWOPENSEGMENTWINDOW(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTENDREADREQUEST(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTGETDESCRIPTION(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTGETCODEPAGE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTGETLCT(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTGETGDDINFO(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 FMTCONVERTGOCAPOLY(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTGETFIRSTFONT(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 SEGSGWNEWPARTDATA(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTSTARTREADREQUEST(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTGETFIRSTGRAPHICSDATA(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTGETNEXTFONT(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 MTGETNEXTGRAPHICSDATA(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!!
USHORT APIENTRY16 GPIACCESSMETAFILE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIMTASSOCIATE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIMTDISASSOCIATE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 SEGSGWNEXTORDERF(void)
{
  return unimplemented(__FUNCTION__);
}






// fix prototype !!!
USHORT APIENTRY16 GPIPATHTOREGION(void)
{
  return unimplemented(__FUNCTION__);
}

//BOOL    WINAPI FloodFill(HDC, int, int, COLORREF);
// fix prototype !!!
USHORT APIENTRY16 GPIFLOODFILL(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIDRAWBITS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYBITMAPINFOHEADER(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYLOGICALFONT(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYFACESTRING(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYFONTACTION(void)
{
  return unimplemented(__FUNCTION__);
}

//HPALETTE WINAPI CreatePalette(const LOGPALETTE FAR*);
// fix prototype !!!
USHORT APIENTRY16 GPICREATEPALETTE(void)
{
  return unimplemented(__FUNCTION__);
}

// UINT    WINAPI RealizePalette(HDC);
// fix prototype !!!
USHORT APIENTRY16 GPIDELETEPALETTE(void)
{
  return unimplemented(__FUNCTION__);
}

//HPALETTE WINAPI SelectPalette(HDC, HPALETTE, BOOL);
// fix prototype !!!
USHORT APIENTRY16 GPISELECTPALETTE(void)
{
  return unimplemented(__FUNCTION__);
}

//void    WINAPI AnimatePalette(HPALETTE, UINT, UINT, const PALETTEENTRY FAR*);
// fix prototype !!!
USHORT APIENTRY16 GPIANIMATEPALETTE(void)
{
  return unimplemented(__FUNCTION__);
}

// UINT    WINAPI SetPaletteEntries(HPALETTE, UINT, UINT, const PALETTEENTRY FAR*);
// fix prototype !!!
USHORT APIENTRY16 GPISETPALETTEENTRIES(void)
{
  return unimplemented(__FUNCTION__);
}

// UINT    WINAPI GetPaletteEntries(HPALETTE, UINT, UINT, PALETTEENTRY FAR*);
// fix prototype !!!
USHORT APIENTRY16 GPIQUERYPALETTE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYPALETTEINFO(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYCHAREXTRA(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETCHAREXTRA(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYCHARBREAKEXTRA(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETCHARBREAKEXTRA(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIFRAMEREGION(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!!
USHORT APIENTRY16 GPISETDCOWNER(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETBITMAPOWNER(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETREGIONOWNER(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIINSTALLIFIFONT(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 DSPINITSYSTEMDRIVERNAME(void)
{
  return unimplemented(__FUNCTION__);
}

