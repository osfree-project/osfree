/* MonCalls */

#define  INCL_DOSMONITORS
#include <os2.h>


USHORT __pascal Dos16MonClose(const HMONITOR hmon)
{
  return DosMonClose(hmon);
}


USHORT __pascal Dos16MonOpen(const PSZ a, HMONITOR * hmon)
{
  return DosMonOpen(a, hmon);
}


USHORT __pascal Dos16MonRead(const PBYTE a, const USHORT b, const PBYTE c, const PUSHORT d)
{
  return DosMonRead(a, b, c, d);
}


USHORT __pascal Dos16MonReg(const HMONITOR hmon, const PBYTE b, const PBYTE c, const USHORT d, const USHORT e)
{
  return DosMonReg(hmon, b, c, d, e);
}


USHORT __pascal Dos16MonWrite(const PBYTE a, const PBYTE b, const USHORT c)
{
  return 0(a, b, c);
}

USHORT DosMonClose(const HMONITOR hmon)
{
  return 0;
}


USHORT DosMonOpen(const PSZ a, HMONITOR * hmon)
{
  return 0;
}


USHORT DosMonRead(const PBYTE a, const USHORT b, const PBYTE c, const PUSHORT d)
{
  return 0;
}


USHORT DosMonReg(const HMONITOR hmon, const PBYTE b, const PBYTE c, const USHORT d, const USHORT e)
{
  return 0;
}


USHORT DosMonWrite(const PBYTE a, const PBYTE b, const USHORT c)
{
  return 0;
}
