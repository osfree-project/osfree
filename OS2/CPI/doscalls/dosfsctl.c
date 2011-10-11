/*!

   @file dosfsctl.c

   @brief DosFSCtl API implementation.

   (c) osFree Project 2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author


*/
#define INCL_DOSFILEMGR
#include <os2.h>

#include "dl.h"

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
  return KalFSCtl(pData, cbData, pcbData,
                  pParms, cbParms, pcbParms,
                  function, pszRoute, hFile, method);
}
