/*!
   @file

   @ingroup pm

   @brief Presentation Manager 16-bit Graphics Programming Interface

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

*/

#ifndef GPI16SEGMENTS_INCLUDED
#define GPI16SEGMENTS_INCLUDED

BOOL APIENTRY16 GPIOPENSEGMENT(HPS a, LONG b)
{
#ifdef BIT32
	return GpiOpenSegment(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPICLOSESEGMENT(HPS hps)
{
#ifdef BIT32
	return GpiCloseSegment(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIDELETESEGMENT(HPS a, LONG b)
{
#ifdef BIT32
	return GpiDeleteSegment(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIDELETESEGMENTS(HPS a, LONG b, LONG c)
{
#ifdef BIT32
	return GpiDeleteSegments(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYSEGMENTNAMES(HPS a, LONG b, LONG c, LONG d, PLONG e)
{
#ifdef BIT32
	return GpiQuerySegmentNames(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETINITIALSEGMENTATTRS(HPS a, LONG b, LONG c)
{
#ifdef BIT32
	return GpiSetInitialSegmentAttrs(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYINITIALSEGMENTATTRS(HPS a, LONG b)
{
#ifdef BIT32
	return GpiQueryInitialSegmentAttrs(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETSEGMENTATTRS(HPS a, LONG b, LONG c, LONG d)
{
#ifdef BIT32
	return GpiSetSegmentAttrs(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYSEGMENTATTRS(HPS a, LONG b, LONG c)
{
#ifdef BIT32
	return GpiQuerySegmentAttrs(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETSEGMENTPRIORITY(HPS a, LONG b, LONG c, LONG d)
{
#ifdef BIT32
	return GpiSetSegmentPriority(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYSEGMENTPRIORITY(HPS a, LONG b, LONG c)
{
#ifdef BIT32
	return GpiQuerySegmentPriority(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIGETDATA(HPS a, LONG b, PLONG c, LONG d, LONG e, PBYTE f)
{
#ifdef BIT32
	return GpiGetData(a, b, c, d, e, f);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIPUTDATA(HPS a, LONG b, PLONG c, PBYTE d)
{
#ifdef BIT32
	return GpiPutData(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIDRAWCHAIN(HPS hps)
{
#ifdef BIT32
	return GpiDrawChain(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIDRAWFROM(HPS a, LONG b, LONG c)
{
#ifdef BIT32
	return GpiDrawFrom(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIDRAWSEGMENT(HPS a, LONG b)
{
#ifdef BIT32
	return GpiDrawSegment(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIREMOVEDYNAMICS(HPS a, LONG b, LONG c)
{
#ifdef BIT32
	return GpiRemoveDynamics(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIDRAWDYNAMICS(HPS hps)
{
#ifdef BIT32
	return GpiDrawDynamics(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#endif
