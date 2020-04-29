/*  Thunking (dummies in our implementation)
 *
 */

#include "kal.h"

APIRET APIENTRY DosFlatToSel(ULONG addr)
{
  USHORT sel;
  USHORT offs;

  log("%s enter\n", __FUNCTION__);
  log("addr=%lx\n", addr);

  if (! addr)
  {
    addr = 0;
    goto flat_to_sel_exit;
  }

  sel = addr >> 16;
  sel = (sel << 3) | 7;
  offs = addr & 0xffff;

  addr = (sel << 16) | offs;

flat_to_sel_exit:
  log("%s exit => %lx\n", __FUNCTION__, addr);
  return addr;
}

APIRET APIENTRY DosSelToFlat(ULONG addr)
{
  USHORT sel;
  USHORT offs;

  log("%s enter\n", __FUNCTION__);
  log("addr=%lx\n", addr);

  sel = addr >> 19;
  offs = addr & 0xffff;

  if (! sel)
  {
    addr = 0;
    goto sel_to_flat_exit;
  }

  addr = (sel << 16) | offs;

sel_to_flat_exit:
  log("%s exit => %lx\n", __FUNCTION__, addr);
  return addr;
}
