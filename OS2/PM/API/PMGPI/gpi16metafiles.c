/*!
   @file

   @ingroup pm

   @brief Presentation Manager 16-bit Graphics Programming Interface

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

*/

#ifndef GPI16METAFILES_INCLUDED
#define GPI16METAFILES_INCLUDED

// fix prototype !!!
USHORT APIENTRY16 GPICOPYMETAFILE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIDELETEMETAFILE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPILOADMETAFILE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIPLAYMETAFILE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYMETAFILEBITS(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYMETAFILELENGTH(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPISAVEMETAFILE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPISETMETAFILEBITS(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#if APILEVEL > 1
// fix prototype !!!
USHORT APIENTRY16 GPISUSPENDPLAY(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIRESUMEPLAY(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}
#endif

#endif

