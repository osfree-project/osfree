/*!

   @file doswrite.c

   @brief DosWrite API implementation.

   (c) osFree Project 2010, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev <yuri.prokushev@gmail.com>


*/

#include <os2.h>

/*
  This is real implementation of DosWrite in Kernel Abstraction Layer
*/
APIRET __cdecl KalWrite(const HFILE hFile, const PVOID pBuffer, const ULONG cbWrite, ULONG * pcbActual);

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
  return KalWrite(hFile, pBuffer, cbWrite, pcbActual);
}
