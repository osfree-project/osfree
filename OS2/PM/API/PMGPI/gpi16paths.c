/*!
   @file

   @ingroup pm

   @brief Presentation Manager 16-bit Graphics Programming Interface

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

*/

#ifndef GPI16PATHS_INCLUDED
#define GPI16PATHS_INCLUDED

BOOL APIENTRY16 GPIBEGINPATH(HPS a, LONG b)
{
#ifdef BIT32
	return GpiBeginPath(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPICLOSEFIGURE(HPS hps)
{
#ifdef BIT32
	return GpiCloseFigure(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIENDPATH(HPS hps)
{
#ifdef BIT32
	return GpiEndPath(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIFILLPATH(HPS a, LONG b, LONG c)
{
#ifdef BIT32
	return GpiFillPath(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIMODIFYPATH(HPS a, LONG b, LONG c)
{
#ifdef BIT32
	return GpiModifyPath(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIOUTLINEPATH(HPS a, LONG b, LONG c)
{
#ifdef BIT32
	return GpiOutlinePath(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETCLIPPATH(HPS a, LONG b, LONG c)
{
#ifdef BIT32
	return GpiSetClipPath(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPISTROKEPATH(HPS a, LONG b, ULONG c)
{
#ifdef BIT32
	return GpiStrokePath(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#endif
