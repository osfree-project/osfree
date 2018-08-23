#include "kal.h"

VOID _Far16 _Pascal DOS16EXIT(const USHORT action, const USHORT result)
{
  DosExit((ULONG)action, (ULONG)result);
}
