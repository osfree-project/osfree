#include "kal.h"

USHORT APIENTRY16  DOS16CONNECTNMPIPE(HPIPE handle)
{
  return DosConnectNPipe(handle);
}
