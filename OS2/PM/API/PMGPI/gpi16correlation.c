/*!
   @file

   @ingroup pm

   @brief Presentation Manager 16-bit Graphics Programming Interface

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

*/

#ifndef GPI16CORRELATION_INCLUDED
#define GPI16CORRELATION_INCLUDED

LONG APIENTRY16 GPICORRELATECHAIN(HPS a, LONG b, PPOINTL c, LONG d, LONG e, PLONG f)
{
#ifdef BIT32
	return GpiCorrelateChain(a, b, c, d, e, f);
#else
	return unimplemented(__FUNCTION__);
#endif
}


LONG APIENTRY16 GPICORRELATEFROM(HPS a, LONG b, LONG c, LONG d, PPOINTL e, LONG f, LONG g, PLONG h)
{
#ifdef BIT32
	return GpiCorrelateFrom(a, b, c, d, e, f, g, h);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPICORRELATESEGMENT(HPS a, LONG b, LONG c, PPOINTL d, LONG e, LONG f, PLONG g)
{
#ifdef BIT32
	return GpiCorrelateSegment(a, b, c, d, e, f, g);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIRESETBOUNDARYDATA(HPS a)
{
#ifdef BIT32
	return GpiResetBoundaryData(a);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYBOUNDARYDATA(HPS a, PRECTL b)
{
#ifdef BIT32
	return GpiQueryBoundaryData(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETPICKAPERTURESIZE(HPS a, LONG b, PSIZEL c)
{
#ifdef BIT32
	return GpiSetPickApertureSize(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYPICKAPERTURESIZE(HPS a, PSIZEL b)
{
#ifdef BIT32
	return GpiQueryPickApertureSize(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETPICKAPERTUREPOSITION(HPS a, PPOINTL b)
{
#ifdef BIT32
	return GpiSetPickAperturePosition(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYPICKAPERTUREPOSITION(HPS a, PPOINTL b)
{
#ifdef BIT32
	return GpiQueryPickAperturePosition(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETTAG(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetTag(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYTAG(HPS a, PLONG b)
{
#ifdef BIT32
	return GpiQueryTag(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#endif
