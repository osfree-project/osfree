// 16-bit prototypes
#define  INCL_DOSMONITORS
#include <os2.h>

/* MonCalls */

// 32-bit prototypes (EMXWRAP/SUB32 style)
USHORT APIENTRY Dos32MonClose(const HMONITOR hmon);
USHORT APIENTRY Dos32MonOpen(const PSZ a, HMONITOR * hmon);
USHORT APIENTRY Dos32MonRead(const PBYTE a, const USHORT b, const PBYTE c, const PUSHORT d);
USHORT APIENTRY Dos32MonReg(const HMONITOR hmon, const PBYTE b, const PBYTE c, const USHORT d, const USHORT e);
USHORT APIENTRY Dos32MonWrite(const PBYTE a, const PBYTE b, const USHORT c);


USHORT Dos32MonClose(const HMONITOR hmon)
{
  return 0;
}


USHORT Dos32MonOpen(const PSZ a, HMONITOR * hmon)
{
  return 0;
}


USHORT Dos32MonRead(const PBYTE a, const USHORT b, const PBYTE c, const PUSHORT d)
{
  return 0;
}


USHORT Dos32MonReg(const HMONITOR hmon, const PBYTE b, const PBYTE c, const USHORT d, const USHORT e)
{
  return 0;
}


USHORT Dos32MonWrite(const PBYTE a, const PBYTE b, const USHORT c)
{
  return 0;
}
