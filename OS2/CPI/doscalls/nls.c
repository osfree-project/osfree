#define  INCL_BASE
#include <os2.h>

#include "dl.h"

//typedef struct _COUNTRYCODE {
//  ULONG country;
//  ULONG codepage;
//} COUNTRYCODE, *PCOUNTRYCODE;

//APIRET __cdecl       KalQueryCp(ULONG   cb,
//                                PULONG  arCP,
//                                PULONG  pcCP);

//APIRET __cdecl       KalQueryDBCSEnv(ULONG cb,
//                                     COUNTRYCODE *pcc,
//                                     PBYTE pBuf);

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

/*
ULONG           cb;    //  The length, in bytes, of the string of binary values to be case-mapped.
PCOUNTRYCODE    pcc;   //  Pointer to the COUNTRYCODE structure in which the country code and code page are identified.
PCHAR           pch;   //  The string of binary characters to be case-mapped.
*/
APIRET DosMapCase(ULONG cb, PCOUNTRYCODE pcc, PCHAR pch)
{
  return NO_ERROR;
}

/*
ULONG           cb;    //  The length, in bytes, of the data area (pch) provided by the caller.
PCOUNTRYCODE    pcc;   //  Pointer to the COUNTRYCODE structure in which the country code and code page are identified.
PCHAR           pch;   //  The data area where the collating sequence table is returned.
PULONG          pcch;  //  The length, in bytes, of the collating sequence table returned.
*/
APIRET DosQueryCollate(ULONG cb, PCOUNTRYCODE pcc, PCHAR pch, PULONG pcch)
{
  return NO_ERROR;  
}
