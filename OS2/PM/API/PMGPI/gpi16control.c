#ifndef GPI16CONTROL_INCLUDED
#define GPI16CONTROL_INCLUDED

HPS  APIENTRY16 GPICREATEPS(HAB hab,HDC hdc,PSIZEL psizlSize,ULONG flOptions);
HPS  APIENTRY16 GPICREATEPS(HAB hab,HDC hdc,PSIZEL psizlSize,ULONG flOptions)
{
#ifdef BIT32
	return GpiCreatePS(hab,hdc,psizlSize,flOptions);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIDESTROYPS(HPS hps);
BOOL APIENTRY16 GPIDESTROYPS(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

BOOL APIENTRY16 GPIASSOCIATE(HPS hps,HDC hdc);
BOOL APIENTRY16 GPIASSOCIATE(HPS hps,HDC hdc)
{
  return unimplemented(__FUNCTION__);
}

#ifdef PMWIN
BOOL FAR PASCAL RestoreDC(HDC, SHORT);
#endif
#ifdef PMOS2
// fix prototype !!!
USHORT APIENTRY16 GPIRESTOREPS(HPS hps,SHORT a)
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
USHORT APIENTRY16 GPISAVEPS(HPS hps)
#endif
{
#ifdef BIT32
	return GpiSavePS(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

//BOOL  APIENTRY GpiErase( HPS hps );
// fix prototype !!!
BOOL APIENTRY16 GPIERASE(HPS hps)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYDEVICE(void)
{
  return unimplemented(__FUNCTION__);
}

BOOL  APIENTRY16 GpiResetPS( HPS hps, ULONG flOptions )
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETPS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYPS(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIERRORSEGMENTDATA(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYDRAWCONTROL(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETDRAWCONTROL(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETDRAWINGMODE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYDRAWINGMODE(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPISETSTOPDRAW(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYSTOPDRAW(void)
{
  return unimplemented(__FUNCTION__);
}

#endif
