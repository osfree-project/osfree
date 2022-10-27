/*!
   @file

   @ingroup pm

   @brief Presentation Manager 16-bit Graphics Programming Interface

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

*/

#ifndef GPI16SEGEDITING_INCLUDED
#define GPI16SEGEDITING_INCLUDED

BOOL APIENTRY16 GPISETEDITMODE(HPS a, LONG b, PSZ c)
{
#ifdef BIT32
	return GpiBeginElement(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYEDITMODE(HPS hps)
{
#ifdef BIT32
	return GpiQueryEditMode(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETELEMENTPOINTER(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetElementPointer(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYELEMENTPOINTER(HPS hps)
{
#ifdef BIT32
	return GpiQueryElementPointer(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIOFFSETELEMENTPOINTER(HPS a, LONG b)
{
#ifdef BIT32
	return GpiOffsetElementPointer(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIDELETEELEMENT(HPS hps)
{
#ifdef BIT32
	return GpiDeleteElement(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIDELETEELEMENTRANGE(HPS a, LONG b, LONG c)
{
#ifdef BIT32
	return GpiDeleteElementRange(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPILABEL(HPS a, LONG b)
{
#ifdef BIT32
	return GpiLabel(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETELEMENTPOINTERATLABEL(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetElementPointerAtLabel(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIDELETEELEMENTSBETWEENLABELS(HPS a, LONG b, LONG c)
{
#ifdef BIT32
	return GpiDeleteElementsBetweenLabels(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYELEMENTTYPE(HPS a, PLONG b, LONG c, PSZ d)
{
#ifdef BIT32
	return GpiQueryElementType(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYELEMENT(HPS a, LONG b, LONG c, PBYTE d)
{
#ifdef BIT32
	return GpiQueryElement(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIELEMENT(HPS a, LONG b, PSZ c, LONG d, PBYTE e)
{
#ifdef BIT32
	return GpiElement(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIBEGINELEMENT(HPS a, LONG b, PSZ c)
{
#ifdef BIT32
	return GpiBeginElement(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIENDELEMENT(HPS hps)
{
#ifdef BIT32
	return GpiEndElement(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#endif
