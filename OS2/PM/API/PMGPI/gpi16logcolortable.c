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

// fix prototype !!!
USHORT APIENTRY16 GPICREATELOGCOLORTABLE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYCOLORDATA(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYCOLORINDEX(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYLOGCOLORTABLE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#ifdef PMWIN
DWORD FAR PASCAL GetNearestColor(HDC hdc, DWORD a);
#endif
#ifdef PMOS2
// fix prototype !!!
USHORT APIENTRY16 GPIQUERYNEARESTCOLOR(HPS hps, USHORT a, SHORT b)
#endif
{
#ifdef BIT32
	return GpiQueryNearestColor(hps,a,b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYREALCOLORS(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYRGBCOLOR(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIREALIZECOLORTABLE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIUNREALIZECOLORTABLE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#endif
