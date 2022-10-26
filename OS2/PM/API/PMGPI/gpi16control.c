/*!
   @file

   @ingroup pm

   @brief Presentation Manager 16-bit Graphics Programming Interface

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

*/

#ifndef GPI16CONTROL_INCLUDED
#define GPI16CONTROL_INCLUDED

//HPS APIENTRY16 GPICREATEPS(HAB hab,HDC hdc,PSIZEL psizlSize,ULONG flOptions);
HPS APIENTRY16 GPICREATEPS(HAB hab,HDC hdc,PSIZEL psizlSize,ULONG flOptions)
{
#ifdef BIT32
	return GpiCreatePS(hab,hdc,psizlSize,flOptions);
#else
	return unimplemented(__FUNCTION__);
#endif
}

//BOOL APIENTRY16 GPIDESTROYPS(HPS hps);
BOOL APIENTRY16 GPIDESTROYPS(HPS hps)
{
#ifdef BIT32
	return GpiDestroyPS(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

//BOOL APIENTRY16 GPIASSOCIATE(HPS hps,HDC hdc);
BOOL APIENTRY16 GPIASSOCIATE(HPS hps,HDC hdc)
{
#ifdef BIT32
	return GpiAssociate(hps, hdc);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#ifdef PMWIN
BOOL FAR PASCAL RestoreDC(HDC, SHORT);
#endif
#ifdef PMOS2
BOOL APIENTRY16 GPIRESTOREPS(HPS hps, LONG a)
#endif
{
#ifdef BIT32
	return GpiRestorePS(hps, a);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#ifdef PMWIN
SHORT FAR PASCAL SaveDC(HDC hdc);
#endif
#ifdef PMOS2
LONG APIENTRY16 GPISAVEPS(HPS hps)
#endif
{
#ifdef BIT32
	return GpiSavePS(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIERASE(HPS hps)
{
#ifdef BIT32
	return GpiErase(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

HDC APIENTRY16 GPIQUERYDEVICE(HPS a)
{
#ifdef BIT32
	return GpiQueryDevice(a);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIRESETPS(HPS hps, ULONG flOptions)
{
#ifdef BIT32
	return GpiResetPS(hps, flOptions);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETPS(HPS a, PSIZEL b, ULONG c)
{
#ifdef BIT32
	return GpiSetPS(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

ULONG APIENTRY16 GPIQUERYPS(HPS a, PSIZEL b)
{
#ifdef BIT32
	return GpiQueryPS(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIERRORSEGMENTDATA(HPS a, PLONG b, PLONG c)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYDRAWCONTROL(HPS a, LONG b)
{
#ifdef BIT32
	return GpiQueryDrawControl(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETDRAWCONTROL(HPS a, LONG b, LONG c)
{
#ifdef BIT32
	return GpiSetDrawControl(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETDRAWINGMODE(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetDrawingMode(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYDRAWINGMODE(HPS a)
{
#ifdef BIT32
	return GpiQueryDrawingMode(a);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETSTOPDRAW(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetStopDraw(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYSTOPDRAW(HPS a)
{
#ifdef BIT32
	return GpiQueryStopDraw(a);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#endif
