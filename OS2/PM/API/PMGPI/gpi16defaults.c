/*!
   @file

   @ingroup pm

   @brief Presentation Manager 16-bit Graphics Programming Interface

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

*/

#ifndef GPI16DEFAULTS_INCLUDED
#define GPI16DEFAULTS_INCLUDED

BOOL APIENTRY16 GPIQUERYDEFARCPARAMS(HPS a, PARCPARAMS b)
{
#ifdef BIT32
	return GpiQueryDefArcParams(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYDEFATTRS(HPS a, LONG b, ULONG c, PBUNDLE d)
{
#ifdef BIT32
	return GpiQueryDefAttrs(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYDEFTAG(HPS a, PLONG b)
{
#ifdef BIT32
	return GpiQueryDefTag(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYDEFVIEWINGLIMITS(HPS a, PRECTL b)
{
#ifdef BIT32
	return GpiQueryDefViewingLimits(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETDEFARCPARAMS(HPS a, PARCPARAMS b)
{
#ifdef BIT32
	return GpiSetDefArcParams(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETDEFATTRS(HPS a, LONG b, ULONG c, PBUNDLE d)
{
#ifdef BIT32
	return GpiSetDefAttrs(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETDEFTAG(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetDefTag(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETDEFVIEWINGLIMITS(HPS a, PRECTL b)
{
#ifdef BIT32
	return GpiSetDefViewingLimits(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#endif
