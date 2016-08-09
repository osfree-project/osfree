/*!

   @file dosfsctl.c

   @brief DosFSCtl API implementation.

   (c) osFree Project 2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author


*/
#include "kal.h"

APIRET APIENTRY  DosFSCtl(PVOID pData,
                             ULONG cbData,
                             PULONG pcbData,
                             PVOID pParms,
                             ULONG cbParms,
                             PULONG pcbParms,
                             ULONG function,
                             PCSZ  pszRoute,
                             HFILE hFile,
                             ULONG method)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("pData=%lx\n", pData);
  log("cbData=%lx\n", cbData);
  log("pParms=%lx\n", pParms);
  log("cbParms=%lx\n", cbParms);
  log("function=%lx\n", function);
  log("pszRoute=%s\n", pszRoute);
  log("hFile=%s\n", hFile);
  log("method=%lx\n", method);
  rc = KalFSCtl(pData, cbData, pcbData,
                pParms, cbParms, pcbParms,
                function, (PSZ)pszRoute, hFile, method);
  log("*pcbData=%lx\n", *pcbData);
  log("*pcbParms=%lx\n", *pcbParms);
  return rc;
}
