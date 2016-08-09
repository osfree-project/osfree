/*  Thunking (dummies in our implementation)
 *
 */

#include "kal.h"

APIRET APIENTRY DosFlatToSel(ULONG x)
{
  log("%s\n", __FUNCTION__);
  log("addr=%lx\n", x);
  return x;
}

APIRET APIENTRY DosSelToFlat(ULONG x)
{
  log("%s\n", __FUNCTION__);
  log("addr=%lx\n", x);
  return x;
}
