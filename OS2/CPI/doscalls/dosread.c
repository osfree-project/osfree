/*   (c) osFree project,
 *   by valerius, 2010, Jun 18
 */

#include "kal.h"


APIRET APIENTRY  DosRead(HFILE hFile,
                         PVOID pBuffer,
                         ULONG cbRead,
                         PULONG pcbActual)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("hFile=%lx\n", hFile);
  log("pBuffer=%x\n", pBuffer);
  log("cbRead=%x\n", cbRead);
  rc = KalRead(hFile, pBuffer, cbRead, pcbActual);
  log("cbActual=%x\n", *pcbActual);
  return rc;
}
