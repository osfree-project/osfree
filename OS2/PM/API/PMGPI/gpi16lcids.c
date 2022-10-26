/*!
   @file

   @ingroup pm

   @brief Presentation Manager 16-bit Graphics Programming Interface

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

*/

#ifndef GPI16LCIDS_INCLUDED
#define GPI16LCIDS_INCLUDED

LONG APIENTRY16 GPICREATELOGFONT(HPS a, PSTR8 b, LONG c, PFATTRS d)
{
#ifdef BIT32
	return GpiCreateLogFont(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIDELETESETID(HPS a, LONG b)
{
#ifdef BIT32
	return GpiDeleteSetId(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPILOADFONTS(HAB a, PSZ b)
{
#ifdef BIT32
	return GpiLoadFonts(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

USHORT APIENTRY16 GPIQUERYCP(HPS a)
{
#ifdef BIT32
	return GpiQueryCp(a);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYFONTFILEDESCRIPTIONS(HAB a, PSZ b, PLONG c, PFFDESCS d)
{
#ifdef BIT32
	return GpiQueryFontFileDescriptions(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYFONTS(HPS a, ULONG b, PSZ c, PLONG d, LONG e, PFONTMETRICS f)
{
#ifdef BIT32
	return GpiQueryFonts(a, b, c, d, e, f);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYFONTMETRICS(HPS a, LONG b, PFONTMETRICS c)
{
#ifdef BIT32
	return GpiQueryFontMetrics(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYKERNINGPAIRS(HPS a, LONG b, PKERNINGPAIRS c)
{
#ifdef BIT32
	return GpiQueryKerningPairs(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYNUMBERSETIDS(HPS a)
{
#ifdef BIT32
	return GpiQueryNumberSetIds(a);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYSETIDS(HPS a, LONG b, PLONG c, PSTR8 d, PLONG e)
{
#ifdef BIT32
	return GpiQuerySetIds(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYWIDTHTABLE(HPS a, LONG b, LONG c, PLONG d)
{
#ifdef BIT32
	return GpiQueryWidthTable(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETCP(HPS a, USHORT b)
{
#ifdef BIT32
	return GpiSetCp(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIUNLOADFONTS(HAB a, PSZ b)
{
#ifdef BIT32
	return GpiUnloadFonts(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#if APILEVEL > 1
BOOL APIENTRY16 GPILOADPUBLICFONTS(HAB a, PCSZ b)
{
#ifdef BIT32
	return GpiLoadPublicFonts(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIUNLOADPUBLICFONTS(HAB a, PCSZ b)
{
#ifdef BIT32
	return GpiUnloadPublicFonts(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}
#endif

#endif
