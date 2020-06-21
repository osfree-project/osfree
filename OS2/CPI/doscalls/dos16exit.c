#include "kal.h"

VOID APIENTRY16 DOS16EXIT(const USHORT action, const USHORT result)
{
  DosExit((ULONG)action, (ULONG)result);
}
