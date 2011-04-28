/*!

   @file sub32.c

   @brief emx_revision API implementation.

   (c) osFree Project, <http://www.osFree.org>
   for licence see legal file in same directory as this source

   @author Yuri Prokushev <prokushev@freemail.ru>


*/
#include <os2.h>
#include <rexxsaa.h>

#pragma aux EMX_REVISION "*"

// We use revision number 70 because latest EMXWRAP revision is 60.
// It is clear we never will have newer revision so we use next number here.
static const char revision[] = "70";

ULONG EMX_REVISION (PCSZ pszName,
                    LONG lArgC,
                    const RXSTRING *rxArgV,
                    PCSZ pszQueueName,
                    PRXSTRING prxRetStr)
{
  int i;

  if (lArgC != 0)
    return 1;
  for (i = 0; revision[i] != 0; ++i)
    prxRetStr->strptr[i] = revision[i];
  prxRetStr->strlength = i;
  return 0;
}
