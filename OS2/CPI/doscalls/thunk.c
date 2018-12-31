/*  Thunking (dummies in our implementation)
 *
 */

#include "kal.h"

APIRET APIENTRY DosFlatToSel(ULONG x)
{
  log("%s enter\n", __FUNCTION__);
  log("addr=%lx\n", x);
  log("%s exit => %lx\n", __FUNCTION__, x);
  return x;
}

APIRET APIENTRY DosSelToFlat(ULONG x)
{
  log("%s enter\n", __FUNCTION__);
  log("addr=%lx\n", x);
  log("%s exit => %lx\n", __FUNCTION__, x);
  return x;
}
