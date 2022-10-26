/*!
   @file

   @ingroup pm

   @brief Presentation Manager 16-bit Graphics Programming Interface

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

*/

#ifndef GPI16LOGCOLORTABLE_INCLUDED
#define GPI16LOGCOLORTABLE_INCLUDED

BOOL APIENTRY16 GPICREATELOGCOLORTABLE(HPS a, ULONG b, LONG c, LONG d, LONG e, PLONG f)
{
#ifdef BIT32
	return GpiCreateLogColorTable(a, b, c, d, e, f);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYCOLORDATA(HPS a, LONG b, PLONG c)
{
#ifdef BIT32
	return GpiQueryColorData(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYCOLORINDEX(HPS a, ULONG b, LONG c)
{
#ifdef BIT32
	return GpiQueryColorIndex(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYLOGCOLORTABLE(HPS a, ULONG b, LONG c, LONG d, PLONG e)
{
#ifdef BIT32
	return GpiQueryLogColorTable(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#ifdef PMWIN
DWORD FAR PASCAL GetNearestColor(HDC hdc, DWORD a);
#endif
#ifdef PMOS2
LONG APIENTRY16 GPIQUERYNEARESTCOLOR(HPS hps, ULONG a, LONG b)
#endif
{
#ifdef BIT32
	return GpiQueryNearestColor(hps,a,b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYREALCOLORS(HPS a, ULONG b, LONG c, LONG d, PLONG e)
{
#ifdef BIT32
	return GpiQueryRealColors(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYRGBCOLOR(HPS a, ULONG b, LONG c)
{
#ifdef BIT32
	return GpiQueryRGBColor(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// Only 16-bit version
BOOL APIENTRY16 GPIREALIZECOLORTABLE(HPS hps)
{
	return unimplemented(__FUNCTION__);
}

// Only 16-bit version
BOOL APIENTRY16 GPIUNREALIZECOLORTABLE(HPS hps)
{
	return unimplemented(__FUNCTION__);
}

#endif
