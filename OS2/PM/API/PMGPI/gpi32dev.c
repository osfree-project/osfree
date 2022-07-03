#ifndef GPI32DEV_INCLUDED
#define GPI32DEV_INCLUDED

HMF    APIENTRY DevCloseDC(HDC hdc)
{
  return unimplemented(__FUNCTION__);
}

HDC    APIENTRY DevOpenDC(HAB hab,LONG a,PCSZ b,LONG c,PDEVOPENDATA d,HDC hdc)
{
	#include "devopendc.c"
}

LONG   APIENTRY DevPostDeviceModes(HAB hab,PDRIVDATA a,PCSZ b,PCSZ c,PCSZ d,ULONG e)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY DevQueryCaps(HDC hdc,LONG a,LONG b,PLONG c)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY DevEscape(HDC hdc,LONG a,LONG b,PBYTE c,PLONG d,PBYTE e)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY DevPostEscape(PCSZ a,PCSZ b,PCSZ c,PCSZ d,ULONG e,ULONG f,PBYTE g,ULONG h,PBYTE i)
{
  return unimplemented(__FUNCTION__);
}

BOOL   APIENTRY DevQueryDeviceNames(HAB hab,PCSZ a,PLONG b,PSTR32 c,PSTR64 d,PLONG e,PSTR16 f)
{
  return unimplemented(__FUNCTION__);
}

LONG   APIENTRY DevQueryHardcopyCaps(HDC hdc,LONG a,LONG b,PHCINFO c)
{
  return unimplemented(__FUNCTION__);
}

#endif
