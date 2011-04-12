#include <os2.h>

typedef struct _COUNTRYCODE {
  ULONG country;
  ULONG codepage;
} COUNTRYCODE, *PCOUNTRYCODE;

APIRET APIENTRY      DosQueryCp(ULONG   cb,
                                PULONG  arCP,
                                PULONG  pcCP)
{
  return KalQueryCp(cb, arCP, pcCP);
}

APIRET APIENTRY      DosQueryDBCSEnv(ULONG cb,
                                     COUNTRYCODE *pcc,
                                     PBYTE pBuf)
{
  return KalQueryDBCSEnv(cb, pcc, pBuf);
}

