/*!
   @file

   @ingroup pm

   @brief Presentation Manager 16-bit Graphics Programming Interface

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

*/

#ifndef GPI16DEV_INCLUDED
#define GPI16DEV_INCLUDED

// fix prototype !!!
USHORT APIENTRY16 DEVOPENDC(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
#	include "devopendc.c"
#endif
}

// fix prototype !!!
USHORT APIENTRY16 DEVCLOSEDC(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 DEVPOSTDEVICEMODES(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 DEVESCAPE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 DEVQUERYHARDCOPYCAPS(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 DEVQUERYCAPS(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#if APILEVEL > 1

// fix prototype !!!
USHORT APIENTRY16 DEVQUERYDEVICENAMES(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT   APIENTRY16 DEVPOSTESCAPE(PCSZ a,PCSZ b,PCSZ c,PCSZ d,ULONG e,ULONG f,PBYTE g,ULONG h,PBYTE i)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#endif

#endif
