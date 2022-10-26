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

HMF APIENTRY16 GPICOPYMETAFILE(HMF hmf)
{
#ifdef BIT32
	return GpiCopyMetaFile(hmf);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIDELETEMETAFILE(HMF hmf)
{
#ifdef BIT32
	return GpiDeleteMetaFile(hmf);
#else
	return unimplemented(__FUNCTION__);
#endif
}

HMF APIENTRY16 GPILOADMETAFILE(HAB a, PSZ b)
{
#ifdef BIT32
	return GpiLoadMetaFile(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIPLAYMETAFILE(HPS a, HMF b, LONG c, PLONG d, PLONG e, LONG f, PSZ g)
{
#ifdef BIT32
	return GpiPlayMetaFile(a, b, c, d, e, f, g);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYMETAFILEBITS(HMF a, LONG b, LONG c, PBYTE d)
{
#ifdef BIT32
	return GpiQueryMetaFileBits(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYMETAFILELENGTH(HMF hmf)
{
#ifdef BIT32
	return GpiQueryMetaFileLength(hmf);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISAVEMETAFILE(HMF hmf, PSZ b)
{
#ifdef BIT32
	return GpiSaveMetaFile(hmf, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETMETAFILEBITS(HMF a, LONG b, LONG c, PBYTE d)
{
#ifdef BIT32
	return GpiSetMetaFileBits(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#if APILEVEL > 1
BOOL APIENTRY16 GPISUSPENDPLAY(HPS hps)
{
#ifdef BIT32
	return GpiSuspendPlay(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIRESUMEPLAY(HPS hps)
{
#ifdef BIT32
	return GpiResumePlay(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}
#endif

#endif

