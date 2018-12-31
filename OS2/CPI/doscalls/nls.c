#include "kal.h"


APIRET APIENTRY      DosQueryCp(ULONG   cb,
                                PULONG  arCP,
                                PULONG  pcCP)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("cb=%lu\n", cb);
  rc = KalQueryCp(cb, arCP, pcCP);
  log("arCP=%lu\n", arCP);
  log("pcCP=%lu\n", pcCP);
  log("*arCP=%lu\n", *arCP);
  log("*pcCP=%lu\n", *pcCP);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY      DosQueryDBCSEnv(ULONG cb,
                                     COUNTRYCODE *pcc,
                                     PBYTE pBuf)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("cb=%lu\n", cb);
  log("pcc=%lu\n", pcc);
  rc = KalQueryDBCSEnv(cb, pcc, pBuf);
  log("*pcc=%lu\n", *pcc);
  log("pBuf=%lu\n", pBuf);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

/*
ULONG           cb;    //  The length, in bytes, of the string of binary values to be case-mapped.
PCOUNTRYCODE    pcc;   //  Pointer to the COUNTRYCODE structure in which the country code and code page are identified.
PCHAR           pch;   //  The string of binary characters to be case-mapped.
*/
APIRET DosMapCase(ULONG cb, PCOUNTRYCODE pcc, PCHAR pch)
{
  APIRET rc = NO_ERROR;
  log("%s enter\n", __FUNCTION__);
  log("cb=%lu\n", cb);
  log("pcc=%lu\n", pcc);
  log("pch=%lu\n", pch);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

/*
ULONG           cb;    //  The length, in bytes, of the data area (pch) provided by the caller.
PCOUNTRYCODE    pcc;   //  Pointer to the COUNTRYCODE structure in which the country code and code page are identified.
PCHAR           pch;   //  The data area where the collating sequence table is returned.
PULONG          pcch;  //  The length, in bytes, of the collating sequence table returned.
*/
APIRET DosQueryCollate(ULONG cb, PCOUNTRYCODE pcc, PCHAR pch, PULONG pcch)
{
  APIRET rc = NO_ERROR;
  log("%s enter\n", __FUNCTION__);
  log("cb=%lu\n", cb);
  log("pcc=%lu\n", pcc);
  log("pch=%lu\n", pch);
  log("pcch=%lu\n", pcch);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;  
}
