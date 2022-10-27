/*!
   @file

   @ingroup pm

   @brief Presentation Manager 16-bit Graphics Programming Interface

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

*/

#ifndef GPI16REGIONS_INCLUDED
#define GPI16REGIONS_INCLUDED

LONG APIENTRY16 GPICOMBINEREGION(HPS a, HRGN b, HRGN c, HRGN d, LONG e)
{
#ifdef BIT32
	return GpiCombineRegion(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

HRGN APIENTRY16 GPICREATEREGION(HPS a, LONG b, PRECTL c)
{
#ifdef BIT32
	return GpiCreateRegion(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIDESTROYREGION(HPS a, HRGN b)
{
#ifdef BIT32
	return GpiDestroyRegion(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIEQUALREGION(HPS a, HRGN b, HRGN c)
{
#ifdef BIT32
	return GpiEqualRegion(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIEXCLUDECLIPRECTANGLE(HPS a, PRECTL b)
{
#ifdef BIT32
	return GpiExcludeClipRectangle(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIINTERSECTCLIPRECTANGLE(HPS a, PRECTL b)
{
#ifdef BIT32
	return GpiIntersectClipRectangle(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIOFFSETCLIPREGION(HPS a, PPOINTL b)
{
#ifdef BIT32
	return GpiOffsetClipRegion(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIOFFSETREGION(HPS a, HRGN b, PPOINTL c)
{
#ifdef BIT32
	return GpiOffsetRegion(a, b ,c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

//BOOL    WINAPI Polygon(HDC, const POINT FAR*, int);
LONG APIENTRY16 GPIPAINTREGION(HPS a, HRGN b)
{
#ifdef BIT32
	return GpiPaintRegion(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIPTINREGION(HPS a, HRGN b, PPOINTL c)
{
#ifdef BIT32
	return GpiPtInRegion(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYREGIONBOX(HPS a, HRGN b, PRECTL c)
{
#ifdef BIT32
	return GpiQueryRegionBox(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYREGIONRECTS(HPS a, HRGN b, PRECTL c, PRGNRECT d, PRECTL e)
{
#ifdef BIT32
	return GpiQueryRegionRects(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIRECTINREGION(HPS a, HRGN b, PRECTL c)
{
#ifdef BIT32
	return GpiRectInRegion(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETREGION(HPS a, HRGN b, LONG c, PRECTL d)
{
#ifdef BIT32
	return GpiSetRegion(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

//int     WINAPI SelectClipRgn(HDC, HRGN);
LONG APIENTRY16 GPISETCLIPREGION(HPS a, HRGN b, PHRGN c)
{
#ifdef BIT32
	return GpiSetClipRegion(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

HRGN APIENTRY16 GPIQUERYCLIPREGION(HPS hps)
{
#ifdef BIT32
	return GpiQueryClipRegion(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

//int     WINAPI GetClipBox(HDC, RECT FAR*);
LONG APIENTRY16 GPIQUERYCLIPBOX(HPS a, PRECTL b)
{
#ifdef BIT32
	return GpiQueryClipBox(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#endif
