#ifndef GPI32DEV_INCLUDED
#define GPI32DEV_INCLUDED

#define INCL_GREALL
#include <pmddi.h>

/*
  Documentation: http://www.edm2.com/index.php/GreCloseDC
*/
HMF    APIENTRY DevCloseDC(HDC hdc)
{
   return GreCloseDC(hdc);
}

/*
  Documentation: http://www.edm2.com/index.php/GreOpenDC
*/

HDC    APIENTRY DevOpenDC(HAB hab,LONG a,PCSZ b,LONG c,PDEVOPENDATA d,HDC hdc)
{
   return GreOpenDC(hdc, a, b, c, d);
}

LONG   APIENTRY DevPostDeviceModes(HAB hab,PDRIVDATA a,PCSZ b,PCSZ c,PCSZ d,ULONG e)
{
  return unimplemented(__FUNCTION__);
}

/*
  Documentation: http://www.edm2.com/index.php/GreQueryDeviceCaps
*/

BOOL   APIENTRY DevQueryCaps(HDC hdc,LONG a,LONG b,PLONG c)
{
  return GreQueryDeviceCaps(hdc, a, b, c);
}

/*
  Documentation: http://www.edm2.com/index.php/GreEscape
*/
LONG   APIENTRY DevEscape(HDC hdc,LONG a,LONG b,PBYTE c,PLONG d,PBYTE e)
{
  return GreEscape(hdc, a, b, c, d, e);
}

LONG   APIENTRY DevPostEscape(PCSZ a,PCSZ b,PCSZ c,PCSZ d,ULONG e,ULONG f,PBYTE g,ULONG h,PBYTE i)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY DevQueryDeviceNames(HAB hab,PCSZ a,PLONG b,PSTR32 c,PSTR64 d,PLONG e,PSTR16 f)
{
  return unimplemented(__FUNCTION__);
}

/*
  Documentation: http://www.edm2.com/index.php/GreQueryHardcopyCaps
*/
LONG   APIENTRY DevQueryHardcopyCaps(HDC hdc,LONG a,LONG b,PHCINFO c)
{
  return GreQueryHardcopyCaps(hdc,a,b,c);
}

#endif
