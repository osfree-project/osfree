/*  Thunking (dummies in our implementation)
 *
 */

#include <os2.h>

APIRET APIENTRY DosFlatToSel(ULONG x)
{
  return x;
}

APIRET APIENTRY DosSelToFlat(ULONG x)
{
  return x;
}
