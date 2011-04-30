/* MonCalls */

#define  INCL_DOSMONITORS
#include <os2.h>


USHORT __pascal DOSMONCLOSE(const HMONITOR hmon)
{
  return DosMonClose(hmon);
}


USHORT __pascal DOSMONOPEN(const PSZ a, HMONITOR * hmon)
{
  return DosMonOpen(a, hmon);
}


USHORT __pascal DOSMONREAD(const PBYTE a, const USHORT b, const PBYTE c, const PUSHORT d)
{
  return DosMonRead(a, b, c, d);
}


USHORT __pascal DOSMONREG(const HMONITOR hmon, const PBYTE b, const PBYTE c, const USHORT d, const USHORT e)
{
  return DosMonReg(hmon, b, c, d, e);
}


USHORT __pascal DOSMONWRITE(const PBYTE a, const PBYTE b, const USHORT c)
{
  return DosMonWrite(a, b, c);
}
