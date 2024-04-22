#include "kal.h"

USHORT APIENTRY16  DOS16DISCONNECTNMPIPE(HPIPE handle)
{
  return DosDisConnectNPipe(handle);
}
