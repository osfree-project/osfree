/*   (c) osFree project,
 *   by valerius, 2010, Jun 18
 */

#include <os2.h>

#include "dl.h"

/*
  This is real implementation of DosWrite in Kernel Abstraction Layer
*/
//APIRET __cdecl KalRead(const HFILE hFile,
//                       const PVOID pBuffer,
//                       const ULONG cbRead,
//                       PULONG pcbActual);

APIRET APIENTRY  DosRead(HFILE hFile,
                         PVOID pBuffer,
                         ULONG cbRead,
                         PULONG pcbActual)
{
  return KalRead(hFile, pBuffer, cbRead, pcbActual);
}
