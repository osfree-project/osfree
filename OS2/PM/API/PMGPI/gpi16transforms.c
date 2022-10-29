/*!
   @file

   @ingroup pm

   @brief Presentation Manager 16-bit Graphics Programming Interface

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

*/

#ifndef GPI16TRANSFORMS_INCLUDED
#define GPI16TRANSFORMS_INCLUDED

BOOL APIENTRY16 GPICONVERT(HPS a, LONG b, LONG c, LONG d, PPOINTL e)
{
#ifdef BIT32
	return GpiConvert(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPICALLSEGMENTMATRIX(HPS a, LONG b, LONG c, PMATRIXLF d, LONG e)
{
#ifdef BIT32
	return GpiCallSegmentMatrix(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYDEFAULTVIEWMATRIX(HPS a, LONG b, PMATRIXLF c)
{
#ifdef BIT32
	return GpiQueryDefaultViewMatrix(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYGRAPHICSFIELD(HPS a, PRECTL b)
{
#ifdef BIT32
	return GpiQueryGraphicsField(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYMODELTRANSFORMMATRIX(HPS a, LONG b, PMATRIXLF c)
{
#ifdef BIT32
	return GpiQueryModelTransformMatrix(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYPAGEVIEWPORT(HPS a, PRECTL b)
{
#ifdef BIT32
	return GpiQueryPageViewport(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYSEGMENTTRANSFORMMATRIX(HPS a, LONG b, LONG c, PMATRIXLF d)
{
#ifdef BIT32
	return GpiQuerySegmentTransformMatrix(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYVIEWINGTRANSFORMMATRIX(HPS a, LONG b, PMATRIXLF c)
{
#ifdef BIT32
	return GpiQueryViewingTransformMatrix(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYVIEWINGLIMITS(HPS a, PRECTL b)
{
#ifdef BIT32
	return GpiQueryViewingLimits(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIROTATE(HPS a, PMATRIXLF b, LONG c, FIXED d, PPOINTL e)
{
#ifdef BIT32
	return GpiRotate(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISCALE(HPS a, PMATRIXLF b, LONG c, PFIXED d, PPOINTL e)
{
#ifdef BIT32
	return GpiScale(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETDEFAULTVIEWMATRIX(HPS a, LONG b, PMATRIXLF c, LONG d)
{
#ifdef BIT32
	return GpiSetDefaultViewMatrix(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETGRAPHICSFIELD(HPS a, PRECTL b)
{
#ifdef BIT32
	return GpiSetGraphicsField(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETMODELTRANSFORMMATRIX(HPS a, LONG b, PMATRIXLF c, LONG d)
{
#ifdef BIT32
	return GpiSetModelTransformMatrix(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETPAGEVIEWPORT(HPS a, PRECTL b)
{
#ifdef BIT32
	return GpiSetPageViewport(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETSEGMENTTRANSFORMMATRIX(HPS a, LONG b, LONG c, PMATRIXLF d, LONG e)
{
#ifdef BIT32
	return GpiSetSegmentTransformMatrix(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETVIEWINGLIMITS(HPS a, PRECTL b)
{
#ifdef BIT32
	return GpiSetViewingLimits(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETVIEWINGTRANSFORMMATRIX(HPS a, LONG b, PMATRIXLF c, LONG d)
{
#ifdef BIT32
	return GpiSetViewingTransformMatrix(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPITRANSLATE(HPS a, PMATRIXLF b, LONG c, PPOINTL d)
{
#ifdef BIT32
	return GpiTranslate(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#if APILEVEL > 1

BOOL APIENTRY16 GPICONVERTWITHMATRIX(HPS a, LONG b, PPOINTL c, LONG d, PMATRIXLF e)
{
#ifdef BIT32
	return GpiConvertWithMatrix(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#endif

#endif
