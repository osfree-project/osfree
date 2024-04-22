#include "kal.h"

USHORT APIENTRY16   DOS16KILLPROCESS(USHORT ActionCode, PID pid)
{
  return DosKillProcess(ActionCode, pid);
}
