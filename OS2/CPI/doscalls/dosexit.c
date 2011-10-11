#include <os2.h>

#include "dl.h"

VOID APIENTRY DosExit(const ULONG action, const ULONG result)
{
  KalExit(action, result);
}
