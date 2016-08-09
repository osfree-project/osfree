/*!

   @file doswrite.c

   @brief DosWrite API implementation.

   (c) osFree Project 2010, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev <yuri.prokushev@gmail.com>


*/

#include "kal.h"

/*
  This is real implementation of DosWrite in Kernel Abstraction Layer
*/

/*!

   @brief Writes data to file

   @param hFile
   @param pBuffer
   @param cbWrite
   @param pcbActual

   @return

   API
     KalWrite

   Function logic
     Function calls KalWrite which is placed in KAL.DLL

*/

APIRET APIENTRY DosWrite(const HFILE hFile, const PVOID pBuffer, const ULONG cbWrite, ULONG * pcbActual)
{
  APIRET rc;
  log("%s\n", __FUNCTION__);
  log("hFile=%lx\n", hFile);
  log("pBuffer=%x\n", pBuffer);
  log("cbWrite=%x\n", cbWrite);
  rc = KalWrite(hFile, pBuffer, cbWrite, pcbActual);
  log("cbActual=%x\n", *pcbActual);
  return rc;
}
