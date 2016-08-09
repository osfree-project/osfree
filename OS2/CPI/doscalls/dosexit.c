#include "kal.h"

VOID APIENTRY DosExit(const ULONG action, const ULONG result)
{
  log("%s\n", __FUNCTION__);
  log("action=%lx\n", action);
  log("result=%lx\n", result);
  KalExit(action, result);
}
